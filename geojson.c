#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "stb_ds.h"

#include "geojson.h"


const int SHIFT_WIDTH = 2;
bool DATA_NEWLINE = false;
bool OBJECT_NEWLINE = true;


/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_convert
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_convert(geojson_t *this, const char* file_name)
{
    size_t length = 0;
    FILE *f;

    f = fopen(file_name, "rb");
    if (!f) {
        fprintf(stdout, "cannot open %s\n", file_name);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = calloc(length, sizeof(char));
    if (buf[0] == '\0') {
        size_t nbr = fread(buf, 1, length, f);
        fprintf(stdout, "read %ld\n", nbr);
    }
    fclose(f);

    char *endptr;
    JsonValue value = {};
    JsonAllocator allocator = {};
    int rc = jsonParse(buf, &endptr, &value, &allocator);
    if (rc != JSON_OK) {
        fprintf(stdout, "invalid JSON format for %s\n", buf);
        return -1;
    }

    geojson_parse_root(this, value);
    geojson_dump_value(this, value, 0);

    JsonAllocator_deallocate(&allocator);

    free(buf);

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_parse_root
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_parse_root(geojson_t *this, JsonValue value)
{
    for (JsonNode *node = JsonValue_toNode(&value); node != NULL; node = node->next) {
        //JSON organized in hierarchical levels
        //level 0, root with objects: "type", "features"
        //FeatureCollection is not much more than an object that has "type": "FeatureCollection"
        //and then an array of Feature objects under the key "features".

        if (strcmp(node->key, "type") == 0) {
            assert(JsonValue_getTag(&node->value) == JSON_STRING);
            char *str = JsonValue_toString(&node->value);
            if (strcmp(str, "Feature") == 0) {
                //parse the root, contains only one "Feature"
                geojson_parse_feature(this, value);
            }
        }
        if (strcmp(node->key, "features") == 0) {
            assert(JsonValue_getTag(&node->value) == JSON_ARRAY);
            geojson_parse_features(this, node->value);
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_parse_features
//array of Feature objects under the key "features".
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_parse_features(geojson_t *this, JsonValue value)
{
    assert(JsonValue_getTag(&value) == JSON_ARRAY);
    size_t arr_size = 0; //size of array
    for (JsonNode *n_feat = JsonValue_toNode(&value); n_feat != NULL; n_feat = n_feat->next)
    {
        arr_size++;
    }
    fprintf(stdout, "features: %ld\n", arr_size);
    for (JsonNode *n_feat = JsonValue_toNode(&value); n_feat != NULL; n_feat = n_feat->next)
    {
        JsonValue object = n_feat->value;
        assert(JsonValue_getTag(&object) == JSON_OBJECT);
        geojson_parse_feature(this, object);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_parse_feature
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_parse_feature(geojson_t *this, JsonValue value)
{
    JsonValue object = value;
    assert(JsonValue_getTag(&object) == JSON_OBJECT);
    feature_t feature = {};

    //3 objects with keys:
    // "type",
    // "properties",
    // "geometry"
    //"type" has a string value "Feature"
    //"properties" has a list of objects
    //"geometry" has 2 objects:
    //key "type" with value string geometry type (e.g."Polygon") and
    //key "coordinates" an array

    for (JsonNode *obj = JsonValue_toNode(&object); obj != NULL; obj = obj->next) {
        if (strcmp(obj->key, "type") == 0) {
            assert(JsonValue_getTag(&obj->value) == JSON_STRING);
        }
        else if (strcmp(obj->key, "properties") == 0) {
            assert(JsonValue_getTag(&obj->value) == JSON_OBJECT);
            //parse properties
            for (JsonNode *prp = JsonValue_toNode(&obj->value); prp != NULL; prp = prp->next) {
                //geojson_dump_value(this, value, 0);
                //get name
                if (strcmp(prp->key, "NAME") == 0 || strcmp(prp->key, "name") == 0 || strcmp(prp->key, "ADMIN") == 0) {
                    assert(JsonValue_getTag(&prp->value) == JSON_STRING);
                    feature.m_name = strdup(JsonValue_toString(&prp->value));
//                    fprintf(stdout, "NAME: %s\n", feature.m_name);
                }
            }
        }
        else if (strcmp(obj->key, "geometry") == 0) {
            assert(JsonValue_getTag(&obj->value) == JSON_OBJECT);
            geojson_parse_geometry(this, obj->value, &feature);
        }
    }

    arrput(this->m_feature, feature);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_parse_geometry
//"geometry" has 2 objects:
//key "type" with value string geometry type (e.g."Polygon") and
//key "coordinates" an array
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_parse_geometry(geojson_t *this, JsonValue value, feature_t /*&*/*feature)
{
    assert(JsonValue_getTag(&value) == JSON_OBJECT);
    char *str_geometry_type = NULL; //"Polygon", "MultiPolygon", "Point"
    for (JsonNode *node = JsonValue_toNode(&value); node != NULL; node = node->next) {
        if (strcmp(node->key, "type") == 0) {
            assert(JsonValue_getTag(&node->value) == JSON_STRING);
            str_geometry_type = JsonValue_toString(&node->value);
        }
        else if (strcmp(node->key, "coordinates") == 0) {
            assert(JsonValue_getTag(&node->value) == JSON_ARRAY);

            if (strcmp(str_geometry_type, "Point") == 0) {
                /////////////////////////////////////////////////////////////////////////////////////////////////////
                //store geometry locally for points
                /////////////////////////////////////////////////////////////////////////////////////////////////////

                geometry_t geometry = {};
                geometry.m_type = strdup(str_geometry_type);

                polygon_t polygon = {};
                JsonValue arr_coord = node->value;
                double lon = JsonValue_toNumber(&JsonValue_toNode(&arr_coord)->value);;
                double lat = JsonValue_toNumber(&JsonValue_toNode(&arr_coord)->next->value);
                coord_t coord = { lon, lat };
                arrput(polygon.m_coord, coord);
                arrput(geometry.m_polygons, polygon);
                arrput(feature->m_geometry, geometry);
            }

            /////////////////////////////////////////////////////////////////////////////////////////////////////
            //store geometry in parse_coordinates() for polygons
            /////////////////////////////////////////////////////////////////////////////////////////////////////

            if (strcmp(str_geometry_type, "Polygon") == 0)
            {
                assert(JsonValue_getTag(&node->value) == JSON_ARRAY);
                geojson_parse_coordinates(this, node->value, str_geometry_type, feature);
            }
            if (strcmp(str_geometry_type, "MultiPolygon") == 0)
            {
                assert(JsonValue_getTag(&node->value) == JSON_ARRAY);
                geojson_parse_coordinates(this, node->value, str_geometry_type, feature);
            }
        }
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_parse_coordinates
//"parse_coordinates"
//for "Polygon"
//is an array of size 1 that contains another array and then an array of 2 numbers (lat, lon)
//for "MultiPolygon"
//is an array that contains another array of size 1, that contains another array,
//and then an array of 2 numbers (lat, lon)
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_parse_coordinates(geojson_t *this, JsonValue value, /*std::string*/char /*&*/*type, feature_t /*&*/*feature)
{
    assert(JsonValue_getTag(&value) == JSON_ARRAY);
    geometry_t geometry = {};
    geometry.m_type = strdup(type);
    if (strcmp(type, "Polygon") == 0) {
        for (JsonNode *node = JsonValue_toNode(&value); node != NULL; node = node->next) {
            JsonValue arr = node->value;
            assert(JsonValue_getTag(&arr) == JSON_ARRAY);

            polygon_t polygon = {};
            for (JsonNode *n = JsonValue_toNode(&arr); n != NULL; n = n->next) {
                JsonValue crd = n->value;
                assert(JsonValue_getTag(&crd) == JSON_ARRAY);
                double lon = JsonValue_toNumber(&(JsonValue_toNode(&crd)->value));
                double lat = JsonValue_toNumber(&(JsonValue_toNode(&crd)->next->value));
                coord_t coord = { lon, lat };
                arrput(polygon.m_coord, coord);
            }
            arrput(geometry.m_polygons, polygon);
        }
    }

    if (strcmp(type, "MultiPolygon") == 0){
        for (JsonNode *node = JsonValue_toNode(&value); node != NULL; node = node->next) {
            JsonValue arr = node->value;
            assert(JsonValue_getTag(&arr) == JSON_ARRAY);
            for (JsonNode *n = JsonValue_toNode(&arr); n != NULL; n = n->next) { // array of size 1
                JsonValue arr_crd = n->value;
                assert(JsonValue_getTag(&arr_crd) == JSON_ARRAY);
                polygon_t polygon = {};
                for (JsonNode *m = JsonValue_toNode(&arr_crd); m != NULL; m = m->next) {
                    JsonValue crd = m->value;
                    //geojson_dump_value(this, crd, 10);
                    assert(JsonValue_getTag(&crd) == JSON_ARRAY);
                    JsonNode *n = JsonValue_toNode(&crd);
                    double lon = JsonValue_toNumber(&n->value);
                    double lat = JsonValue_toNumber(&n->next->value);
                    coord_t coord = { lon, lat };
                    arrput(polygon.m_coord, coord);
                }
                arrput(geometry.m_polygons, polygon);
            }
        }
    }
    //store
    arrput(feature->m_geometry, geometry);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_dump_value
/////////////////////////////////////////////////////////////////////////////////////////////////////

int geojson_dump_value(geojson_t *this, JsonValue o, int indent)
{
    switch (JsonValue_getTag(&o))
    {
    case JSON_NUMBER:
        fprintf(stdout, "%f", JsonValue_toNumber(&o));
        break;
    case JSON_STRING:
        geojson_dump_string(this, JsonValue_toString(&o));
        break;
    case JSON_ARRAY:
        if (!JsonValue_toNode(&o))
        {
            fprintf(stdout, "[]");
            break;
        }
        fprintf(stdout, "[");
        if (DATA_NEWLINE)
            fprintf(stdout, "\n");

        for (JsonNode *i = JsonValue_toNode(&o); i != NULL; i = i->next) {
            if (DATA_NEWLINE)
                fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
            geojson_dump_value(this, i->value, indent + SHIFT_WIDTH);
            if (DATA_NEWLINE)
                fprintf(stdout, i->next ? ",\n" : "\n");
            else
                fprintf(stdout, i->next ? "," : "");
        }
        if (DATA_NEWLINE)
            fprintf(stdout, "%*s]", indent, "");
        else
            fprintf(stdout, "]");
        break;
    case JSON_OBJECT:
        if (!JsonValue_toNode(&o))
        {
            fprintf(stdout, "{}");
            break;
        }
        fprintf(stdout, "{\n");
        for (JsonNode *i = JsonValue_toNode(&o); i != NULL; i = i->next) {
            fprintf(stdout, "%*s", indent + SHIFT_WIDTH, "");
            geojson_dump_string(this, i->key);
            fprintf(stdout, ": ");
            geojson_dump_value(this, i->value, indent + SHIFT_WIDTH);
            fprintf(stdout, i->next ? ",\n" : "\n");
        }
        fprintf(stdout, "%*s}", indent, "");
        break;
    case JSON_TRUE:
        fprintf(stdout, "true");
        break;
    case JSON_FALSE:
        fprintf(stdout, "false");
        break;
    case JSON_NULL:
        fprintf(stdout, "null");
        break;
    }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_dump_string
/////////////////////////////////////////////////////////////////////////////////////////////////////

void geojson_dump_string(geojson_t *this, const char *s)
{
    fputc('"', stdout);
    while (*s)
    {
        int c = *s++;
        switch (c)
        {
        case '\b':
            fprintf(stdout, "\\b");
            break;
        case '\f':
            fprintf(stdout, "\\f");
            break;
        case '\n':
            fprintf(stdout, "\\n");
            break;
        case '\r':
            fprintf(stdout, "\\r");
            break;
        case '\t':
            fprintf(stdout, "\\t");
            break;
        case '\\':
            fprintf(stdout, "\\\\");
            break;
        case '"':
            fprintf(stdout, "\\\"");
            break;
        default:
            fputc(c, stdout);
        }
    }
    fprintf(stdout, "%s\"", s);
}
