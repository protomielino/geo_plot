#ifndef GEO_JSON_HH
#define GEO_JSON_HH

#ifdef WT_BUILDING
#include "Wt/WDllDefs.h"
#else
#ifndef WT_API
#define WT_API
#endif
#endif

#include <string.h>

#include "gason.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//coord_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
//public:
  double latitude;
  double longitude;
} coord_t;
//public:
//  coord_t(double x_, double y_) :
//    x(x_),
//    y(y_)
//  {
//  }

///////////////////////////////////////////////////////////////////////////////////////
//polygon_t
///////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
//public:
  /*std::vector<>*/coord_t *m_coord;
} polygon_t;
//public:
//  polygon_t()
//  {};

///////////////////////////////////////////////////////////////////////////////////////
//geometry_t
///////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
//public:
  /*std::string*/char *m_type; //"Polygon", "Point"
  /*std::vector<>*/polygon_t *m_polygons;
} geometry_t;
//public:
//  geometry_t()
//  {};

///////////////////////////////////////////////////////////////////////////////////////
//feature_t
///////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
//public:
  /*std::string*/char *m_name;
  /*std::vector<>*/geometry_t *m_geometry;
} feature_t;
//public:
//  feature_t()
//  {};


/////////////////////////////////////////////////////////////////////////////////////////////////////
//geojson_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
//public:
  //storage is a list of features
  /*std::vector<>*/feature_t *m_feature;
} WT_API geojson_t;
//public:
geojson_t geojson_ctor();
void geojson_dtor(geojson_t *this);
int geojson_convert(geojson_t *this, const char* file_name);

//private:
int geojson_parse_root(geojson_t *this, JsonValue value);
int geojson_parse_features(geojson_t *this, JsonValue value);
int geojson_parse_feature(geojson_t *this, JsonValue value);
int geojson_parse_geometry(geojson_t *this, JsonValue value, feature_t /*&*/*feature);
int geojson_parse_coordinates(geojson_t *this, JsonValue value, /*std::string*/char /*&*/*type, feature_t /*&*/*feature);
int geojson_dump_value(geojson_t *this, JsonValue value, int indent/* = 0*/);
void geojson_dump_string(geojson_t *this, const char *s);

#endif




