#include "world/BlockMeshBuilder.h"

std::vector<float> BlockMeshBuilder::createCubeVertices()
{
    return {
        // Each vertex is:
        // position x, y, z           texture u, v

        // BACK FACE (-Z)
        -0.5f,-0.5f,-0.5f,           0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,           1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,           1.0f, 1.0f,
         0.5f, 0.5f,-0.5f,           1.0f, 1.0f,
        -0.5f, 0.5f,-0.5f,           0.0f, 1.0f,
        -0.5f,-0.5f,-0.5f,           0.0f, 0.0f,

        // FRONT FACE (+Z)
        -0.5f,-0.5f, 0.5f,           0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,           1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,           1.0f, 1.0f,
         0.5f, 0.5f, 0.5f,           1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,           0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,           0.0f, 0.0f,

        // LEFT FACE (-X) — final orientation
        -0.5f, 0.5f, 0.5f,           1.0f, 1.0f,
        -0.5f, 0.5f,-0.5f,           0.0f, 1.0f,
        -0.5f,-0.5f,-0.5f,           0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,           0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,           1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,           1.0f, 1.0f,

        // RIGHT FACE (+X) — final orientation
         0.5f, 0.5f, 0.5f,           0.0f, 1.0f,
         0.5f, 0.5f,-0.5f,           1.0f, 1.0f,
         0.5f,-0.5f,-0.5f,           1.0f, 0.0f,
         0.5f,-0.5f,-0.5f,           1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,           0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,           0.0f, 1.0f,

         // BOTTOM FACE (-Y)
         -0.5f,-0.5f,-0.5f,           0.0f, 1.0f,
          0.5f,-0.5f,-0.5f,           1.0f, 1.0f,
          0.5f,-0.5f, 0.5f,           1.0f, 0.0f,
          0.5f,-0.5f, 0.5f,           1.0f, 0.0f,
         -0.5f,-0.5f, 0.5f,           0.0f, 0.0f,
         -0.5f,-0.5f,-0.5f,           0.0f, 1.0f,

         // TOP FACE (+Y)
         -0.5f, 0.5f,-0.5f,           0.0f, 1.0f,
          0.5f, 0.5f,-0.5f,           1.0f, 1.0f,
          0.5f, 0.5f, 0.5f,           1.0f, 0.0f,
          0.5f, 0.5f, 0.5f,           1.0f, 0.0f,
         -0.5f, 0.5f, 0.5f,           0.0f, 0.0f,
         -0.5f, 0.5f,-0.5f,           0.0f, 1.0f
    };
}