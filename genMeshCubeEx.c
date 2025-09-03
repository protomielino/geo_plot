#include <stdio.h>
#include <string.h>

#include <raylib.h>
#define RAYMATH_IMPLEMENTATION
#include <raymath.h>

static Mesh GenMeshPlaneEx(Vector3 normal, float width, float length, float depth, int resX, int resY);
#if 0
static Mesh GenMeshPlaneUp(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshPlaneDown(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshPlaneLeft(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshPlaneRight(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshPlaneFront(float width, float length, float depth, int resX, int resY, int resZ);
static Mesh GenMeshPlaneBack(float width, float length, float depth, int resX, int resY, int resZ);
#endif
static Mesh GenMeshCubeEx(float width, float length, float depth, int resX, int resY, int resZ);

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
    Image checked = GenImageChecked(2, 2, 1, 1, RED, GREEN);
    Texture2D texture = LoadTextureFromImage(checked);
    UnloadImage(checked);

    Model models = { 0 };

    models = LoadModelFromMesh(GenMeshCubeEx(10, 10, 10, 10, 10, 10));

    // Generated meshes could be exported as .obj files
    //ExportMesh(models.meshes[0], "plane.obj");

    // Set checked texture as default diffuse component for all models material
    models.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // Define the camera to look into our 3d world
    Camera camera = {
            position:   { 5.0f, 5.0f, 5.0f },
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

                DrawModelWires(models, position, 1.0f, WHITE);
//                DrawModel(models, position, 1.0f, WHITE);
//                DrawGrid(10, 1.0);

                Vector3 pos;
                pos = (Vector3){0, 0, 0};
                DrawCubeWires(pos, 2, 2, 2, WHITE);
                pos = (Vector3){2, 0, 0};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, RED);
                DrawLine3D(Vector3Zero(), pos, RED);
                pos = (Vector3){0, 2, 0};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, GREEN);
                DrawLine3D(Vector3Zero(), pos, GREEN);
                pos = (Vector3){0, 0, 2};
                DrawCubeWires(pos, 0.1, 0.1, 0.1, BLUE);
                DrawLine3D(Vector3Zero(), pos, BLUE);

            } EndMode3D();

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texture); // Unload texture

    // Unload models data (GPU VRAM)

//    RL_FREE(models.meshes[0].vertices);
//    RL_FREE(models.meshes[0].normals);
//    RL_FREE(models.meshes[0].texcoords);
//    RL_FREE(models.meshes[0].indices);
//
//    UnloadModel(models);

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

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
//            plane = GenMeshPlaneUp(width, length, depth, resX, resY, resZ);
            plane = GenMeshPlaneEx(up, width, depth, length, resX, resZ);
            break;
        case 1:
//            plane = GenMeshPlaneDown(width, length, depth, resX, resY, resZ);
            plane = GenMeshPlaneEx(down, width, depth, length, resX, resZ);
            break;
        case 2:
//            plane = GenMeshPlaneRight(width, length, depth, resX, resY, resZ);
            plane = GenMeshPlaneEx(right, depth, length, width, resZ, resY);
            break;
        case 3:
//            plane = GenMeshPlaneLeft(width, length, depth, resX, resY, resZ);
            plane = GenMeshPlaneEx(left, depth, length, width, resZ, resY);
            break;
        case 4:
//            plane = GenMeshPlaneFront(width, length, depth, resX, resY, resZ);
            plane = GenMeshPlaneEx(front, length, width, depth, resY, resX);
            break;
        case 5:
//            plane = GenMeshPlaneBack(width, length, depth, resX, resY, resZ);
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
    }

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneEx(Vector3 normal, float width, float length, float depth, int resX, int resY)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;

    // Vertices definition
    int vertexCount = resX*resY; // vertices get reused for the faces

    normal = Vector3Normalize(normal);

    Vector3 axisA = { normal.y, normal.z, normal.x };
    Vector3 axisB = Vector3CrossProduct(normal, axisA);

    normal = Vector3Scale(normal, depth);

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int y = 0; y < resY; y++)
    {
        // [-length/2, length/2]
        for (int x = 0; x < resX; x++)
        {
            int vertexIndex = x + y*resX;
            Vector2 t = { x, y };
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
    for (int v = 0; v < resY; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resY - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resY - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i;
        triangles[t++] = i + 1;

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i + resX + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i + 0] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i + 0] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i + 0] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

#if 0
// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneUp(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float yPos = 0.0f;
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*depth;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + z*resX] = (Vector3){ xPos, yPos, zPos };
            vertices[x + z*resX] = Vector3Add(vertices[x + z*resX], Vector3Scale(up, length/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = up;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneDown(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float yPos = 0.0f;
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*depth;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + z*resX] = (Vector3){ xPos, yPos, zPos };
            vertices[x + z*resX] = Vector3Add(vertices[x + z*resX], Vector3Scale(down, length/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = down;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i;
        triangles[t++] = i + 1;

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i + resX + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneLeft(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resY*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float xPos = 0;
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*depth;
        for (int y = 0; y < resY; y++)
        {
            // [-width/2, width/2]
            float yPos = ((float)y/(resY - 1) - 0.5f)*length;
            vertices[y + z*resY] = (Vector3){ xPos, yPos, zPos };
            vertices[y + z*resY] = Vector3Add(vertices[y + z*resY], Vector3Scale(left, width/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = left;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resY; u++)
        {
            texcoords[u + v*resY] = (Vector2){ (float)u/(resY - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resY - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resY - 1);

        triangles[t++] = i + resY;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resY;
        triangles[t++] = i + resY + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneRight(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resY*resZ; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float xPos = 0;
    for (int z = 0; z < resZ; z++)
    {
        // [-length/2, length/2]
        float zPos = ((float)z/(resZ - 1) - 0.5f)*depth;
        for (int x = 0; x < resY; x++)
        {
            // [-width/2, width/2]
            float yPos = ((float)x/(resY - 1) - 0.5f)*length;
            vertices[x + z*resY] = (Vector3){ xPos, yPos, zPos };
            vertices[x + z*resY] = Vector3Add(vertices[x + z*resY], Vector3Scale(right, width/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = right;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resZ; v++)
    {
        for (int u = 0; u < resY; u++)
        {
            texcoords[u + v*resY] = (Vector2){ (float)u/(resY - 1), (float)v/(resZ - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resY - 1)*(resZ - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resY - 1);

        triangles[t++] = i + resY;
        triangles[t++] = i;
        triangles[t++] = i + 1;

        triangles[t++] = i + resY;
        triangles[t++] = i + 1;
        triangles[t++] = i + resY + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneFront(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resY; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float zPos = 0.0f;
    for (int y = 0; y < resY; y++)
    {
        // [-length/2, length/2]
        float yPos = ((float)y/(resY - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + y*resX] = (Vector3){ xPos, yPos, zPos };
            vertices[x + y*resX] = Vector3Add(vertices[x + y*resX], Vector3Scale(front, depth/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = front;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resY; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resY - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resY - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i;
        triangles[t++] = i + 1;

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i + resX + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}

// Generate plane mesh (with subdivisions)
static Mesh GenMeshPlaneBack(float width, float length, float depth, int resX, int resY, int resZ)
{
    Mesh mesh = { 0 };

    resX++;
    resY++;
    resZ++;

    // Vertices definition
    int vertexCount = resX*resY; // vertices get reused for the faces

    Vector3 *vertices = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    float zPos = 0.0f;
    for (int y = 0; y < resY; y++)
    {
        // [-length/2, length/2]
        float yPos = ((float)y/(resY - 1) - 0.5f)*length;
        for (int x = 0; x < resX; x++)
        {
            // [-width/2, width/2]
            float xPos = ((float)x/(resX - 1) - 0.5f)*width;
            vertices[x + y*resX] = (Vector3){ xPos, yPos, zPos };
            vertices[x + y*resX] = Vector3Add(vertices[x + y*resX], Vector3Scale(back, depth/2.0f));
        }
    }

    // Normals definition
    Vector3 *normals = (Vector3 *)RL_MALLOC(vertexCount*sizeof(Vector3));
    for (int n = 0; n < vertexCount; n++)
        normals[n] = back;

    // TexCoords definition
    Vector2 *texcoords = (Vector2 *)RL_MALLOC(vertexCount*sizeof(Vector2));
    for (int v = 0; v < resY; v++)
    {
        for (int u = 0; u < resX; u++)
        {
            texcoords[u + v*resX] = (Vector2){ (float)u/(resX - 1), (float)v/(resY - 1) };
        }
    }

    // Triangles definition (indices)
    int numFaces = (resX - 1)*(resY - 1);
    int *triangles = (int *)RL_MALLOC(numFaces*6*sizeof(int));
    int t = 0;
    for (int face = 0; face < numFaces; face++)
    {
        // Retrieve lower left corner from face ind
        int i = face + face/(resX - 1);

        triangles[t++] = i + resX;
        triangles[t++] = i + 1;
        triangles[t++] = i;

        triangles[t++] = i + resX;
        triangles[t++] = i + resX + 1;
        triangles[t++] = i + 1;
    }

    mesh.vertexCount = vertexCount;
    mesh.triangleCount = numFaces*2;
    mesh.vertices = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.texcoords = (float *)RL_MALLOC(mesh.vertexCount*2*sizeof(float));
    mesh.normals = (float *)RL_MALLOC(mesh.vertexCount*3*sizeof(float));
    mesh.indices = (unsigned short *)RL_MALLOC(mesh.triangleCount*3*sizeof(unsigned short));

    // Mesh vertices position array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.vertices[3*i] = vertices[i].x;
        mesh.vertices[3*i + 1] = vertices[i].y;
        mesh.vertices[3*i + 2] = vertices[i].z;
    }

    // Mesh texcoords array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.texcoords[2*i] = texcoords[i].x;
        mesh.texcoords[2*i + 1] = texcoords[i].y;
    }

    // Mesh normals array
    for (int i = 0; i < mesh.vertexCount; i++)
    {
        mesh.normals[3*i] = normals[i].x;
        mesh.normals[3*i + 1] = normals[i].y;
        mesh.normals[3*i + 2] = normals[i].z;
    }

    // Mesh indices array initialization
    for (int i = 0; i < mesh.triangleCount*3; i++)
        mesh.indices[i] = triangles[i];

    RL_FREE(vertices);
    RL_FREE(normals);
    RL_FREE(texcoords);
    RL_FREE(triangles);

    // Upload vertex data to GPU (static mesh)
    UploadMesh(&mesh, false);

    return mesh;
}
#endif
