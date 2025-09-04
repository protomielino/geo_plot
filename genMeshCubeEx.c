#include <stdio.h>
#include <string.h>

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

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

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - mesh generation");

    // We generate a checked image for texturing
    Image checked = GenImageChecked(512, 512, 256, 256, BLUE, YELLOW);
    Texture2D texture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    Model plane = { 0 };
    Model cube = { 0 };
    Model sphere = { 0 };
    plane = LoadModelFromMesh(GenMeshPlaneEx(up, 10, 10, 0.001f, 25, 25));
    cube = LoadModelFromMesh(GenMeshCubeEx(5, 5, 5, 25, 25, 25));
    sphere = LoadModelFromMesh(GenMeshSphereEx(3.5, 25));

    // Generated meshes could be exported as .obj files
    //ExportMesh(models.meshes[0], "plane.obj");

    // Set checked texture as default diffuse component for all models material
    plane.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    cube.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    sphere.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Define the camera to look into our 3d world
    Camera camera = {
            position:   { 10.0f, 10.0f, 10.0f },
            target:     { 0.0f, 0.0f, 0.0f },
            up:         { 0.0f, 1.0f, 0.0f },
            fovy:       45.0f,
            projection: 0
    };

    // Model drawing position
    Vector3 position = { 0.0f, 0.0f, 0.0f };

    DisableCursor();

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(BLACK);

            BeginMode3D(camera); {

                DrawModel(plane, position, 1.0f, WHITE);
                DrawModel(cube, position, 1.0f, WHITE);
                DrawModel(sphere, position, 1.0f, WHITE);
                DrawModelWires(plane, position, 1.0f, RED);
                DrawModelWires(cube, position, 1.001f, RED);
                DrawModelWires(sphere, position, 1.001f, RED);
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
        normals[n] = normal;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resY; v++) {
        for (int u = 0; u < resX; u++) {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resY - 1) };
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
    // Mesh normals array
    memcpy(mesh.normals, cube.normals, mesh.vertexCount*3*sizeof(float));
    // Mesh indices array initialization
    memcpy(mesh.indices, cube.indices, mesh.triangleCount*3*sizeof(unsigned short));

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    UnloadMesh(cube);

    return mesh;
}
