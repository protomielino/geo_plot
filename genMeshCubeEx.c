#if 0
#include "raylib.h"
#include <math.h>

#define SPHERE_RADIUS 10.0f
#define SPHERE_RESOLUTION 256 // Risoluzione della sfera

// Funzione per ottenere il colore da una texture in base a coordinate sferiche
Color GetColorFromTexture(Image image, float longitude, float latitude) {
    // Calcola le coordinate del pixel
    int x = (int)((longitude + 180.0f) / 360.0f * image.width) % image.width;
    int y = (int)((latitude + 90.0f) / 180.0f * image.height) % image.height;

    return GetImageColor(image, x, y);
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Proiezione di un'immagine su una sfera");

    // Carica la texture
    Image image = LoadImage("gebco_08_rev_elev_21600x10800.png");

    // Imposta la telecamera
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 0.0f, 15.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;

    SetTargetFPS(60);

    DisableCursor();

    while (!WindowShouldClose())
    {
        UpdateCamera(&camera, CAMERA_FREE);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        // Disegna la sfera con la proiezione dell'immagine
        for (int i = 0; i < SPHERE_RESOLUTION; i++) {
            for (int j = 0; j < SPHERE_RESOLUTION; j++) {
                // Calcola la longitudine e latitudine
                float longitude = (float)i / (SPHERE_RESOLUTION - 1) * 360.0f - 180.0f;
                float latitude = (float)j / (SPHERE_RESOLUTION - 1) * 180.0f - 90.0f;

                // Calcola le coordinate 3D sulla sfera
                float x = SPHERE_RADIUS * cosf(DEG2RAD*latitude) * cosf(DEG2RAD*longitude);
                float y = SPHERE_RADIUS * sinf(DEG2RAD*latitude);
                float z = SPHERE_RADIUS * cosf(DEG2RAD*latitude) * sinf(DEG2RAD*longitude);

                // Ottieni il colore dalla texture
                Color color = GetColorFromTexture(image, longitude, latitude);

                // Disegna un punto sulla sfera
                DrawCube((Vector3){ x, -y, z }, 0.2f, 0.2f, 0.2f, color);
            }
        }

        EndMode3D();
        DrawText("Proiezione di un'immagine su una sfera", 10, 10, 20, DARKGRAY);
        EndDrawing();
    }

    UnloadImage(image);

    CloseWindow();

    return 0;
}
#else
#include <stdio.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

#include "geojson.h"

#include "maths.h"

Vector2 offset;
Vector2 scale;

const Vector3 up      = {  0,  1,  0 };
const Vector3 down    = {  0, -1,  0 };
const Vector3 right   = {  1,  0,  0 };
const Vector3 left    = { -1,  0,  0 };
const Vector3 front   = {  0,  0,  1 };
const Vector3 back    = {  0,  0, -1 };
Vector3 faceNormal[] = {
        up,
        down,
        right,
        left,
        front,
        back
};
int numNormals = sizeof(faceNormal)/sizeof(faceNormal[0]);

static Mesh GenMeshPlaneEx(Vector3 normal, float width, float length, float depth, int resX, int resY);
static Mesh GenMeshCubeEx(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshSphereEx(float radius, int resolution);

typedef struct
{
    float latitude;
    float longitude;
} Coordinate;

// Calculate latitude and longitude (in radians) from point on unit sphere
static Coordinate pointToCoordinate(Vector3 pointOnUnitSphere)
{
    float latitude = asinf(pointOnUnitSphere.y);
    float longitude = atan2f(pointOnUnitSphere.x, -pointOnUnitSphere.z);
    return (Coordinate){latitude, longitude};
}

// Calculate point on unit sphere given latitude and longitude (in radians)
static Vector3 coordinateToPoint(Coordinate coordinate)
{
    float y =  sinf(coordinate.latitude);
    float r =  cosf(coordinate.latitude);
    float x =  sinf(coordinate.longitude) * r;
    float z = -cosf(coordinate.longitude) * r;
    return (Vector3){x, y, z};
}

//float map(float input, float input_start, float input_end, float output_start, float output_end)
//{
//    float slope = 1.0 * (output_end - output_start) / (input_end - input_start);
////    float output = output_start + round(slope * (input - input_start));
//    /** CHECH output VARIABLE. Should be rounded? **/
//    float output = output_start + slope * (input - input_start);
//    return output;
//}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    Vector3 linePen = Vector3Zero();
    bool linePenFirst = true;
    int currentCountry = 113;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");

    // We generate a checked image for texturing
    Image image = {};
//    image = GenImageChecked(10, 10, 1, 1, BLACK, BLACK);
    image = LoadImage("Colour_8192x4096.jpg");
    Texture2D texture = LoadTextureFromImage(image);

    Model plane = { 0 };
    Model cube = { 0 };
    Model sphere = { 0 };
    plane = LoadModelFromMesh(GenMeshPlaneEx(up, 10, 10, 0.001f, 10, 10));
    cube = LoadModelFromMesh(GenMeshCubeEx(2, 3, 5, 2, 3, 5));
    sphere = LoadModelFromMesh(GenMeshSphereEx(20, 25));

    // Generated meshes could be exported as .obj files
    //ExportMesh(models.meshes[0], "plane.obj");

    // Set checked texture as default diffuse component for all models material
    plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    cube.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Define the camera to look into our 3d world
    Camera camera = {
            position:   { 0.0f, 20.0f, -30.0f },
            target:     { 0.0f, 0.0f, 0.0f },
            up:         { 0.0f, 1.0f, 0.0f },
            fovy:       45.0f,
            projection: 0
    };

    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };

    // GeoJSON stuff
    if (argc < 2) {
//        fprintf(stdout, "usage : ./parser <GEOJSON file>\n");
//        return 1;
    }

    char *fileName = "countries.geojson";
    geojson_t geojson = {};
    if (geojson_convert(&geojson, /*argv[1]*/fileName) < 0) {
        return 1;
    }

    //--------------------------------------------------------------------------------------

    DisableCursor();

    SetTargetFPS(30);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        if (IsKeyPressed('2')) {
            if (currentCountry < arrlen(geojson.m_feature)-1) {
                currentCountry ++;
            }
        }
        if (IsKeyPressed('1')) {
            if (currentCountry > 0) {
                currentCountry --;
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKGRAY);

            BeginMode3D(camera); {

                ///////////////////////////////////////////////////////////////////////////////////////
                //render geojson
                ///////////////////////////////////////////////////////////////////////////////////////

//                fprintf(stdout, "\n");

                size_t size_features = arrlen(geojson.m_feature);
                for (size_t idx_fet = 0; idx_fet < size_features; idx_fet++) {
//                for (size_t idx_fet = currentCountry; idx_fet < currentCountry+1; idx_fet++) {
                  feature_t feature = geojson.m_feature[idx_fet];

//                  printf("NAME: \"%s\" [%ld/%ld]\n", feature.m_name, idx_fet+1, size_features);

                  float m = map(idx_fet, 0, size_features-1, 0.0f, 360.0f);
                  Color countryColor = ColorFromHSV(m, 1.0f, 1.0f);

                  size_t size_geometry = arrlen(feature.m_geometry);
                  for (size_t idx_geo = 0; idx_geo < size_geometry; idx_geo++) {
                    geometry_t geometry = feature.m_geometry[idx_geo];

//                    printf("  GEOMETRY TYPE: \"%s\" [%ld/%ld]\n", geometry.m_type, idx_geo+1, size_geometry);

                    size_t size_pol = arrlen(geometry.m_polygons);
                    for (size_t idx_pol = 0; idx_pol < size_pol; idx_pol++) {
                      polygon_t polygon = geometry.m_polygons[idx_pol];
                      size_t size_crd = arrlen(polygon.m_coord);

                      if (size_crd == 0) {
                        continue;
                      }

                      /*std::vector<*/double *lat = NULL;
                      /*std::vector<*/double *lon = NULL;

                      for (size_t idx_crd = 0; idx_crd < size_crd; idx_crd++) {
                        arrput(lat, polygon.m_coord[idx_crd].y);
                        arrput(lon, -polygon.m_coord[idx_crd].x);
                      }

                      ///////////////////////////////////////////////////////////////////////////////////////
                      //render each polygon as a vector of vertices
                      ///////////////////////////////////////////////////////////////////////////////////////

                      if (strcmp(geometry.m_type, "Point") == 0) {
//                          printf("    [Point] [%ld/%ld/%ld]\n", idx_pol+1, size_pol, size_crd);

                          // since we are dealing with points idx_crd showld be 1
                          // so the index could be just 0, with no loop
                          for (int idx_crd = 0; idx_crd < size_crd; ++idx_crd) {
                              Coordinate c = { DEG2RAD*lat[idx_crd], DEG2RAD*lon[idx_crd] };
                              Vector3 point = coordinateToPoint(c);
                              point = Vector3Scale(point, 20.0f);
                              if (linePenFirst) {
                                  linePen = point;
                                  linePenFirst = false;
                              }

                              if (currentCountry == idx_fet) {
                                  DrawCube(point, 0.01, 0.01, 0.01, countryColor);
                              } else {
                                  DrawLine3D(linePen, point, countryColor);
                              }

                              linePen = point;
//                              printf("      %f, %f\n", lat[idx_crd], lon[idx_crd]);
                          }
                      }
                      else if (strcmp(geometry.m_type, "Polygon") == 0) {
//                          printf("    [Polygon] [%ld/%ld/%ld]\n", idx_pol+1, size_pol, size_crd);

                          for (int idx_crd = 0; idx_crd < size_crd; ++idx_crd) {
                              Coordinate c = { DEG2RAD*lat[idx_crd], DEG2RAD*lon[idx_crd] };
                              Vector3 point = coordinateToPoint(c);
                              point = Vector3Scale(point, 20.0f);
                              if (linePenFirst) {
                                  linePen = point;
                                  linePenFirst = false;
                              }

                              if (currentCountry == idx_fet) {
                                  DrawCube(point, 0.01, 0.01, 0.01, countryColor);
                              } else {
                                  DrawLine3D(linePen, point, countryColor);
                              }

                              linePen = point;
//                              printf("      %f, %f\n", lat[idx_crd], lon[idx_crd]);
                          }
                      }
                      else if (strcmp(geometry.m_type, "MultiPolygon") == 0) {
//                          printf("    [MultiPolygon] [%ld/%ld/%ld]\n", idx_pol+1, size_pol, size_crd);

                          for (int idx_crd = 0; idx_crd < size_crd; ++idx_crd) {
                              Coordinate c = { DEG2RAD*lat[idx_crd], DEG2RAD*lon[idx_crd] };
                              Vector3 point = coordinateToPoint(c);
                              point = Vector3Scale(point, 20.0f);
                              if (linePenFirst) {
                                  linePen = point;
                                  linePenFirst = false;
                              }

                              if (currentCountry == idx_fet) {
                                  DrawCube(point, 0.01, 0.01, 0.01, countryColor);
                              } else {
                                  DrawLine3D(linePen, point, countryColor);
                              }

                              linePen = point;
//                              printf("      %f, %f\n", lat[idx_crd], lon[idx_crd]);
                          }
                      }

                      arrfree(lat);
                      arrfree(lon);

                      linePenFirst = true;
                    }  //idx_pol
                  } //idx_geo
                } //idx_fet

//                position = (Vector3){ -10, 0, 0 };
//
//                DrawModel(plane, position, 1.0f, WHITE);
//                DrawModelWires(plane, position, 1.0f, RED);
//                position = Vector3Add(position, (Vector3){ 10, 0, 0 });
//
//                DrawModel(cube, position, 1.0f, WHITE);
//                DrawModelWires(cube, position, 1.001f, RED);
//                position = Vector3Add(position, (Vector3){ 10, 0, 0 });
//
                DrawModel(sphere, position, 1.0f, WHITE);
//                DrawModelWires(sphere, position, 1.001f, RED);

                DrawGrid(10, 1.0);

                Vector3 pos;
                pos = (Vector3){0, 0, 0};
                DrawCubeWires(pos, 2.01, 2.01, 2, MAGENTA);
                
                pos = (Vector3){2, 0, 0};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, MAROON);
                DrawLine3D(Vector3Zero(), pos, MAROON);
                pos = (Vector3){0, 2, 0};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, LIME);
                DrawLine3D(Vector3Zero(), pos, LIME);
                pos = (Vector3){0, 0, 2};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, DARKBLUE);
                DrawLine3D(Vector3Zero(), pos, DARKBLUE);

            } EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadImage(image);
    UnloadTexture(texture); // Unload texture

    // Unload models data (GPU VRAM)

    UnloadModel(plane);
    UnloadModel(cube);
    UnloadModel(sphere);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Generate plane mesh (with subdivisions) oriented by normal
static Mesh GenMeshPlaneEx(Vector3 normal, float width, float length, float depth, int resX, int resY)
{
    resX++;
    resY++;

    // Vertices definition
    int vertexCount = resX*resY; // vertices get reused for the faces

    normal = Vector3Normalize(normal);

    Vector3 axisA = { normal.y, normal.z, normal.x };
    Vector3 axisB = Vector3CrossProduct(normal, axisA);

    normal = Vector3Scale(normal, depth);

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int y = 0; y < resY; y++) {
        for (int x = 0; x < resX; x++) {
            int vertexIndex = x + y*resX;
            Vector2 t = { (float)x, (float)y };
            t.x /= (float)resX-1.0f;
            t.y /= (float)resY-1.0f;
            Vector3 pointA = Vector3Scale(axisA, (t.x - 0.5f)*width);
            Vector3 pointB = Vector3Scale(axisB, (t.y - 0.5f)*length);
            Vector3 point = Vector3Add(pointA, pointB);
            point = Vector3Add(point, Vector3Scale(normal, 0.5f));
            vertices[vertexIndex] = point;
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = Vector3Normalize(normal);

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resY; v++) {
        for (int u = 0; u < resX; u++) {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), -(float)v/(resY - 1)+1.0f };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resY - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++) {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i; // 0
        triangles[t++] = i + 1; // 1
        triangles[t++] = i + resX + 1; // 3

        triangles[t++] = i; // 0
        triangles[t++] = i + resX + 1; // 3
        triangles[t++] = i + resX; // 2
    }

    Mesh mesh = {};

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.vertices[3*i + 0] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.texcoords[2*i + 0] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++) {
        mesh.normals[3*i + 0] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++) {
        mesh.indices[i] = triangles[i];
    }

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshCubeEx(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = {};

    int vertexCount =
            (resX+1)*(resZ+1)*2+
            (resX+1)*(resY+1)*2+
            (resZ+1)*(resY+1)*2;
    int numFaces =
            resX*resZ*2+
            resX*resY*2+
            resZ*resY*2;
    int numTriangles = numFaces * 2;

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numTriangles;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    int cursorVertices = 0;
    int cursorTriangles = 0;
    int totVerts = 0;
    Mesh plane = {};
    for (int fi = 0; fi < numNormals; ++fi) {
        switch (fi) {
            case 0:
                plane = GenMeshPlaneEx(up, width, depth, length, resX, resZ);
                break;
            case 1:
                plane = GenMeshPlaneEx(down, width, depth, length, resX, resZ);
                break;
            case 2:
                plane = GenMeshPlaneEx(right, depth, length, width, resZ, resY);
                break;
            case 3:
                plane = GenMeshPlaneEx(left, depth, length, width, resZ, resY);
                break;
            case 4:
                plane = GenMeshPlaneEx(front, length, width, depth, resY, resX);
                break;
            case 5:
                plane = GenMeshPlaneEx(back, length, width, depth, resY, resX);
                break;
            default:
                break;
        }

        memcpy(&mesh.vertices[cursorVertices*3], plane.vertices, plane.vertexCount*3*sizeof(float));
        memcpy(&mesh.normals[cursorVertices*3], plane.normals, plane.vertexCount*3*sizeof(float));
        memcpy(&mesh.texcoords[cursorVertices*2], plane.texcoords, plane.vertexCount*2*sizeof(float));
        cursorVertices += plane.vertexCount;

        for (int i = cursorTriangles*3, j = 0; j < plane.triangleCount*3; ++i, j++) {
            mesh.indices[i] = plane.indices[j]+totVerts;
        }
        totVerts += plane.vertexCount;
        cursorTriangles += plane.triangleCount;

        UnloadMesh(plane);
    }

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshSphereEx(float radius, int resolution)
{
    Mesh cube = GenMeshCubeEx(1, 1, 1, resolution, resolution, resolution);

    Mesh mesh = {};

    mesh.vertexCount = cube.vertexCount;
    mesh.triangleCount = cube.triangleCount;

    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    Vector3 v = {};
    for (int i = 0; i < mesh.vertexCount; ++i) {
        v.x = cube.vertices[3*i + 0];
        v.y = cube.vertices[3*i + 1];
        v.z = cube.vertices[3*i + 2];

#if 0 
// normal ("pinched") projextion
        v = Vector3Normalize(v);
        v = Vector3Scale(v, radius);
#else
// uniform projextion
        float x2_ = v.x * v.x;
        float y2_ = v.y * v.y;
        float z2_ = v.z * v.z;
        float x_ = v.x * sqrt(1.0f - (y2_ + z2_) / 2.0f + (y2_ * z2_) / 3.0f);
        float y_ = v.y * sqrt(1.0f - (z2_ + x2_) / 2.0f + (z2_ * x2_) / 3.0f);
        float z_ = v.z * sqrt(1.0f - (x2_ + y2_) / 2.0f + (x2_ * y2_) / 3.0f);
        v = (Vector3){x_, y_, z_};
        v = Vector3Normalize(v);
        v = Vector3Scale(v, radius);
#endif

        mesh.vertices[3*i + 0] = v.x;
        mesh.vertices[3*i + 1] = v.y;
        mesh.vertices[3*i + 2] = v.z;
    }

    // Mesh texcoords array
    memcpy(mesh.texcoords, cube.texcoords, mesh.vertexCount*2*sizeof(float));
    for (int i = 0; i < mesh.vertexCount-1; ++i) {
        int vxi = i*3+0;
        int vyi = i*3+1;
        int vzi = i*3+2;

        int tui = i * 2 + 0;
        int tvi = i * 2 + 1;

        Vector3 v = {
                mesh.vertices[vxi],
                mesh.vertices[vyi],
                mesh.vertices[vzi]
        };

        v = Vector3Normalize(v);

        Coordinate coor = pointToCoordinate(v);

        float lon = map(coor.longitude, -M_PI,    M_PI, 0.0f, 1.0f);
        float lat = map(coor.latitude, M_PI_2, -M_PI_2,   0.0f, 1.0f);

        mesh.texcoords[tui] = -lon+1.0f;
        mesh.texcoords[tvi] = lat;
    }

    // Mesh normals array
    memcpy(mesh.normals, cube.normals, mesh.vertexCount*3*sizeof(float));

    // Mesh indices array initialization
    memcpy(mesh.indices, cube.indices, mesh.triangleCount*3*sizeof(unsigned short));

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    UnloadMesh(cube);

    return mesh;
}
#endif
