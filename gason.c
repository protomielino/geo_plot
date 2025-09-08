#include <stdlib.h>
#include <stdbool.h>

#include "gason.h"

#define JSON_ZONE_SIZE 4096
#define JSON_STACK_SIZE 32

const char *jsonStrError(int err)
{
    switch (err) {
#define XX(no, str) \
    case JSON_##no: \
        return str;
        JSON_ERRNO_MAP(XX)
#undef XX
    default:
        return "unknown";
    }
}

JsonValue JsonValue_new1(double x)
//: fval(x)
{
    JsonValue this = {};
    this.fval = x;
    return this;
}
JsonValue JsonValue_new2(JsonTag tag/* = JSON_NULL*/, void *payload/* = NULL*/)
{
    JsonValue this = {};
    assert((uintptr_t)payload <= JSON_VALUE_PAYLOAD_MASK);
    this.ival = JSON_VALUE_NAN_MASK | ((uint64_t)tag << JSON_VALUE_TAG_SHIFT) | (uintptr_t)payload;
    return this;
}
bool JsonValue_isDouble(JsonValue *this) /*const*/
{
    return (int64_t)this->ival <= (int64_t)JSON_VALUE_NAN_MASK;
}
JsonTag JsonValue_getTag(JsonValue *this) /*const*/
{
    return JsonValue_isDouble(this) ? JSON_NUMBER : (JsonTag)((this->ival >> JSON_VALUE_TAG_SHIFT) & JSON_VALUE_TAG_MASK);
}
uint64_t JsonValue_getPayload(JsonValue *this) /*const*/
{
    assert(!JsonValue_isDouble(this));
    return this->ival & JSON_VALUE_PAYLOAD_MASK;
}
double JsonValue_toNumber(JsonValue *this) /*const*/
{
    assert(JsonValue_getTag(this) == JSON_NUMBER);
    return this->fval;
}
char* JsonValue_toString(JsonValue *this) /*const*/
{
    assert(JsonValue_getTag(this) == JSON_STRING);
    return (char *)JsonValue_getPayload(this);
}
JsonNode* JsonValue_toNode(JsonValue *this) /*const*/
{
    assert(JsonValue_getTag(this) == JSON_ARRAY || JsonValue_getTag(this) == JSON_OBJECT);
    return (JsonNode *)JsonValue_getPayload(this);
}

void* JsonAllocator_allocate(JsonAllocator *this, size_t size)
{
    size = (size + 7) & ~7;

    if (this->head && this->head->used + size <= JSON_ZONE_SIZE) {
        char *p = (char *)this->head + this->head->used;
        this->head->used += size;
        return p;
    }

    size_t allocSize = sizeof(Zone) + size;
    Zone *zone = (Zone *)malloc(allocSize <= JSON_ZONE_SIZE ? JSON_ZONE_SIZE : allocSize);
    if (zone == NULL)
        return NULL;
    zone->used = allocSize;
    if (allocSize <= JSON_ZONE_SIZE || this->head == NULL) {
        zone->next = this->head;
        this->head = zone;
    } else {
        zone->next = this->head->next;
        this->head->next = zone;
    }
    return (char *)zone + sizeof(Zone);
}

JsonAllocator JsonAllocator_ctor()
//: head(nullptr) {};
{
    JsonAllocator this = {};
    this.head = NULL;
    return this;
}

void JsonAllocator_dtor(JsonAllocator *this)
{
    JsonAllocator_deallocate(this);
}

void JsonAllocator_deallocate(JsonAllocator *this)
{
    while (this->head) {
        Zone *next = this->head->next;
        free(this->head);
        this->head = next;
    }
}

static inline bool _isspace(char c)
{
    return c == ' ' || (c >= '\t' && c <= '\r');
}

static inline bool _isdelim(char c)
{
    return c == ',' || c == ':' || c == ']' || c == '}' || _isspace(c) || !c;
}

static inline bool _isdigit(char c)
{
    return c >= '0' && c <= '9';
}

static inline bool _isxdigit(char c)
{
    return (c >= '0' && c <= '9') || ((c & ~' ') >= 'A' && (c & ~' ') <= 'F');
}

static inline int _char2int(char c)
{
    if (c <= '9')
        return c - '0';
    return (c & ~' ') - 'A' + 10;
}

static double _string2double(char *s, char **endptr)
{
    char ch = *s;
    if (ch == '-')
        ++s;

    double result = 0;
    while (_isdigit(*s))
        result = (result * 10) + (*s++ - '0');

    if (*s == '.') {
        ++s;

        double fraction = 1;
        while (_isdigit(*s)) {
            fraction *= 0.1;
            result += (*s++ - '0') * fraction;
        }
    }

    if (*s == 'e' || *s == 'E') {
        ++s;

        double base = 10;
        if (*s == '+')
            ++s;
        else if (*s == '-') {
            ++s;
            base = 0.1;
        }

        unsigned int exponent = 0;
        while (_isdigit(*s))
            exponent = (exponent * 10) + (*s++ - '0');

        double power = 1;
        for (; exponent; exponent >>= 1, base *= base)
            if (exponent & 1)
                power *= base;

        result *= power;
    }

    *endptr = s;
    return ch == '-' ? -result : result;
}

static inline JsonNode *_insertAfter(JsonNode *tail, JsonNode *node)
{
    if (!tail)
        return node->next = node;
    node->next = tail->next;
    tail->next = node;
    return node;
}

static inline JsonValue _listToValue(JsonTag tag, JsonNode *tail)
{
    if (tail) {
        JsonNode *head = tail->next;
        tail->next = NULL;
        return JsonValue_new2(tag, head);
    }
    return JsonValue_new2(tag, NULL);
}

int jsonParse(char *s, char **endptr, JsonValue *value, JsonAllocator /*&*/*allocator)
{
    JsonNode *tails[JSON_STACK_SIZE] = {};
    JsonTag tags[JSON_STACK_SIZE] = {};
    char *keys[JSON_STACK_SIZE] = {};
    JsonValue o = {};
    int pos = -1;
    bool separator = true;
    JsonNode *node = NULL;
    *endptr = s;

    while (*s) {
        while (_isspace(*s)) {
            ++s;
            if (!*s) break;
        }
        *endptr = s++;
        switch (**endptr) {
        case '-':
            if (!_isdigit(*s) && *s != '.') {
                *endptr = s;
                return JSON_BAD_NUMBER;
            }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            o = JsonValue_new1(_string2double(*endptr, &s));
            if (!_isdelim(*s)) {
                *endptr = s;
                return JSON_BAD_NUMBER;
            }
            break;
        case '"':
            o = JsonValue_new2(JSON_STRING, s);
            for (char *it = s; *s; ++it, ++s) {
                int c = *it = *s;
                if (c == '\\') {
                    c = *++s;
                    switch (c) {
                    case '\\':
                    case '"':
                    case '/':
                        *it = c;
                        break;
                    case 'b':
                        *it = '\b';
                        break;
                    case 'f':
                        *it = '\f';
                        break;
                    case 'n':
                        *it = '\n';
                        break;
                    case 'r':
                        *it = '\r';
                        break;
                    case 't':
                        *it = '\t';
                        break;
                    case 'u':
                        c = 0;
                        for (int i = 0; i < 4; ++i) {
                            if (_isxdigit(*++s)) {
                                c = c * 16 + _char2int(*s);
                            } else {
                                *endptr = s;
                                return JSON_BAD_STRING;
                            }
                        }
                        if (c < 0x80) {
                            *it = c;
                        } else if (c < 0x800) {
                            *it++ = 0xC0 | (c >> 6);
                            *it = 0x80 | (c & 0x3F);
                        } else {
                            *it++ = 0xE0 | (c >> 12);
                            *it++ = 0x80 | ((c >> 6) & 0x3F);
                            *it = 0x80 | (c & 0x3F);
                        }
                        break;
                    default:
                        *endptr = s;
                        return JSON_BAD_STRING;
                    }
                } else if ((unsigned int)c < ' ' || c == '\x7F') {
                    *endptr = s;
                    return JSON_BAD_STRING;
                } else if (c == '"') {
                    *it = 0;
                    ++s;
                    break;
                }
            }
            if (!_isdelim(*s)) {
                *endptr = s;
                return JSON_BAD_STRING;
            }
            break;
        case 't':
            if (!(s[0] == 'r' && s[1] == 'u' && s[2] == 'e' && _isdelim(s[3])))
                return JSON_BAD_IDENTIFIER;
            o = JsonValue_new2(JSON_TRUE, NULL);
            s += 3;
            break;
        case 'f':
            if (!(s[0] == 'a' && s[1] == 'l' && s[2] == 's' && s[3] == 'e' && _isdelim(s[4])))
                return JSON_BAD_IDENTIFIER;
            o = JsonValue_new2(JSON_FALSE, NULL);
            s += 4;
            break;
        case 'n':
            if (!(s[0] == 'u' && s[1] == 'l' && s[2] == 'l' && _isdelim(s[3])))
                return JSON_BAD_IDENTIFIER;
            o = JsonValue_new2(JSON_NULL, NULL);
            s += 3;
            break;
        case ']':
            if (pos == -1)
                return JSON_STACK_UNDERFLOW;
            if (tags[pos] != JSON_ARRAY)
                return JSON_MISMATCH_BRACKET;
            o = _listToValue(JSON_ARRAY, tails[pos--]);
            break;
        case '}':
            if (pos == -1)
                return JSON_STACK_UNDERFLOW;
            if (tags[pos] != JSON_OBJECT)
                return JSON_MISMATCH_BRACKET;
            if (keys[pos] != NULL)
                return JSON_UNEXPECTED_CHARACTER;
            o = _listToValue(JSON_OBJECT, tails[pos--]);
            break;
        case '[':
            if (++pos == JSON_STACK_SIZE)
                return JSON_STACK_OVERFLOW;
            tails[pos] = NULL;
            tags[pos] = JSON_ARRAY;
            keys[pos] = NULL;
            separator = true;
            continue;
        case '{':
            if (++pos == JSON_STACK_SIZE)
                return JSON_STACK_OVERFLOW;
            tails[pos] = NULL;
            tags[pos] = JSON_OBJECT;
            keys[pos] = NULL;
            separator = true;
            continue;
        case ':':
            if (separator || keys[pos] == NULL)
                return JSON_UNEXPECTED_CHARACTER;
            separator = true;
            continue;
        case ',':
            if (separator || keys[pos] != NULL)
                return JSON_UNEXPECTED_CHARACTER;
            separator = true;
            continue;
        case '\0':
            continue;
        default:
            return JSON_UNEXPECTED_CHARACTER;
        }

        separator = false;

        if (pos == -1) {
            *endptr = s;
            *value = o;
            return JSON_OK;
        }

        if (tags[pos] == JSON_OBJECT) {
            if (!keys[pos]) {
                if (JsonValue_getTag(&o) != JSON_STRING)
                    return JSON_UNQUOTED_KEY;
                keys[pos] = JsonValue_toString(&o);
                continue;
            }
            if ((node = (JsonNode *) JsonAllocator_allocate(allocator, sizeof(JsonNode))) == NULL)
                return JSON_ALLOCATION_FAILURE;
            tails[pos] = _insertAfter(tails[pos], node);
            tails[pos]->key = keys[pos];
            keys[pos] = NULL;
        } else {
            if ((node = (JsonNode *) JsonAllocator_allocate(allocator, sizeof(JsonNode) - sizeof(char *))) == NULL)
                return JSON_ALLOCATION_FAILURE;
            tails[pos] = _insertAfter(tails[pos], node);
        }
        tails[pos]->value = o;
    }
    return JSON_BREAKING_BAD;
}
