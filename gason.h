#ifndef GASON_H
#define GASON_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>

typedef enum
{
    JSON_NUMBER = 0,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL = 0xF
} JsonTag;

typedef struct JsonNode_s JsonNode;

#define JSON_VALUE_PAYLOAD_MASK 0x00007FFFFFFFFFFFULL
#define JSON_VALUE_NAN_MASK 0x7FF8000000000000ULL
#define JSON_VALUE_TAG_MASK 0xF
#define JSON_VALUE_TAG_SHIFT 47

typedef union
{
    uint64_t ival;
    double fval;
} JsonValue;

JsonValue JsonValue_new1(double x); //: fval(x)
JsonValue JsonValue_new2(JsonTag tag/* = JSON_NULL*/, void *payload/* = NULL*/);
bool JsonValue_isDouble(JsonValue *this); /*const*/
JsonTag JsonValue_getTag(JsonValue *this); /*const*/
uint64_t JsonValue_getPayload(JsonValue *this); /*const*/
double JsonValue_toNumber(JsonValue *this); /*const*/
char* JsonValue_toString(JsonValue *this); /*const*/
JsonNode* JsonValue_toNode(JsonValue *this); /*const*/

typedef struct JsonNode_s
{
    JsonValue value;
    JsonNode *next;
    char *key;
} JsonNode;

typedef struct
{
    JsonNode *p;
//
//    void operator++() {
//        p = p->next;
//    }
//    bool operator!=(const JsonIterator &x) /*const*/ {
//        return p != x.p;
//    }
//    JsonNode *operator*() /*const*/ {
//        return p;
//    }
//    JsonNode *operator->() /*const*/ {
//        return p;
//    }
} JsonIterator;

//inline JsonIterator begin(JsonValue o)
//{
//    return JsonIterator{o.toNode()};
//}
//inline JsonIterator end(JsonValue)
//{
//    return JsonIterator{NULL};
//}

#define JSON_ERRNO_MAP(XX)                           \
    XX(OK, "ok")                                     \
    XX(BAD_NUMBER, "bad number")                     \
    XX(BAD_STRING, "bad string")                     \
    XX(BAD_IDENTIFIER, "bad identifier")             \
    XX(STACK_OVERFLOW, "stack overflow")             \
    XX(STACK_UNDERFLOW, "stack underflow")           \
    XX(MISMATCH_BRACKET, "mismatch bracket")         \
    XX(UNEXPECTED_CHARACTER, "unexpected character") \
    XX(UNQUOTED_KEY, "unquoted key")                 \
    XX(BREAKING_BAD, "breaking bad")                 \
    XX(ALLOCATION_FAILURE, "allocation failure")

enum JsonErrno {
#define XX(no, str) JSON_##no,
    JSON_ERRNO_MAP(XX)
#undef XX
};

const char *jsonStrError(int err);

typedef struct Zone_s Zone;
typedef struct
{
    struct Zone_s {
        Zone *next;
        size_t used;
    } *head;

} JsonAllocator;
//public:
//JsonAllocator() : head(nullptr) {};
//JsonAllocator(const JsonAllocator &) = delete;
//JsonAllocator &operator=(const JsonAllocator &) = delete;
//JsonAllocator(JsonAllocator &&x) : head(x.head) {
//    x.head = nullptr;
//}
//JsonAllocator &operator=(JsonAllocator &&x) {
//    head = x.head;
//    x.head = nullptr;
//    return *this;
//}
//~JsonAllocator() {
//    deallocate();
//}
void* JsonAllocator_allocate(JsonAllocator *this, size_t size);
void JsonAllocator_deallocate(JsonAllocator *this);

int jsonParse(char *str, char **endptr, JsonValue *value, JsonAllocator /*&*/*allocator);

#endif /* GASON_H */
