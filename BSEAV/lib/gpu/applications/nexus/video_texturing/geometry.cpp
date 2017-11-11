/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "geometry.h"
#include <cassert>

namespace video_texturing
{

#define BUFFER_OFFSET(i) ((char *)0 + (i))

// cube face location and orientation within cube video
struct CubeFace
{
   int posX; // horizontal position within grid
   int posY; // vertical position within grid
   int rot;  // face rotation (0=0, 1=90, 2=180, 3=270)
};

// cube video format definition
struct CubeFmt
{
   int      gridW;   // number of horizontal cube faces in 360 video grid
   int      gridH;   // number of vertical cube faces in 360 video grid
   CubeFace face[6]; // face locations and orientation (0=left, 1=back, 2=right, 3=front, 4=top, 5=bot)
};

Geometry::Geometry(bool is360, Format360 format, uint32_t texW, uint32_t texH) :
   m_indexCount(0), m_modelYRotate(0.0f)
{
   // Get some buffer ids
   glGenBuffers(2, m_vbos);

   if (!is360)
      MakeFlatRect();
   else
   {
      switch (format)
      {
      case FORMAT_ICOSAHEDRON : MakeIcosahedron(); break;
      case FORMAT_OCTAHEDRON  : MakeOctahedron();  break;
      default                 : MakeCube(format, texW, texH); break;
      }
   }

   // Determine the correct cube rotation about Y for each format
   switch (format)
   {
   case FORMAT_ICOSAHEDRON  :
   case FORMAT_OCTAHEDRON   :
   case FORMAT_FISHEYE      : m_modelYRotate = 180.0f;
                              break;
   case FORMAT_EAP          :
   case FORMAT_EQUIRECT     : m_modelYRotate = 90.0f;
                              break;
   case FORMAT_CUBE_32_0    :
   case FORMAT_CUBE_32_90   :
   case FORMAT_CUBE_32_270  :
   case FORMAT_CUBE_32_P270 :
   case FORMAT_CUBE_43_0    : m_modelYRotate = -90.0f;
                              break;
   default                  : assert(0);
   }
}

Geometry::~Geometry()
{
   glDeleteBuffers(2, m_vbos);
}

void Geometry::Bind(GLint posLocation, GLint texCoordLocation)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_vbos[0]);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbos[1]);

   if (posLocation != -1)
   {
      glVertexAttribPointer(posLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), BUFFER_OFFSET(0));
      glEnableVertexAttribArray(posLocation);
   }

   if (texCoordLocation != -1)
   {
      glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                            BUFFER_OFFSET(3 * sizeof(GLfloat)));
      glEnableVertexAttribArray(texCoordLocation);
   }
}

void Geometry::Draw()
{
   glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_SHORT, 0);
}

void Geometry::FillBuffers(const std::vector<GLfloat> &vertexData, const std::vector<GLushort> &indices)
{
   glBindBuffer(GL_ARRAY_BUFFER, m_vbos[0]);
   glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(vertexData[0]), vertexData.data(), GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbos[1]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

   m_indexCount = indices.size();
}

void Geometry::MakeFlatRect()
{
   std::vector<GLfloat> vertexData =
   {
      //          POSITION                        TEXCOORD
      1.000000f, -1.000000f, 0.000000f,     1.000000f, 1.000000f,
      1.000000f,  1.000000f, 0.000000f,     1.000000f, 0.000000f,
     -1.000000f, -1.000000f, 0.000000f,     0.000000f, 1.000000f,
     -1.000000f,  1.000000f, 0.000000f,     0.000000f, 0.000000f,
   };

   std::vector<GLushort> indices =
   {
      0, 2, 1,
      1, 2, 3,
   };

   FillBuffers(vertexData, indices);
}

void Geometry::MakeIcosahedron()
{
   std::vector<GLfloat> vertexData =
   {
      //          POSITION                    TEXCOORD
      -0.7236f,   0.4472f,   0.5258f,   0.09091f,   0.00000f,
      -0.7236f,   0.4472f,   0.5258f,   0.27273f,   0.00000f,
      -0.7236f,   0.4472f,   0.5258f,   0.45455f,   0.00000f,
      -0.7236f,   0.4472f,   0.5258f,   0.63636f,   0.00000f,
      -0.7236f,   0.4472f,   0.5258f,   0.81818f,   0.00000f,

       0.0000f,   1.0000f,   0.0000f,   0.00000f,   0.33333f,
      -0.7236f,   0.4472f,  -0.5258f,   0.18182f,   0.33333f,
      -0.8944f,  -0.4472f,   0.0000f,   0.36364f,   0.33333f,
      -0.2764f,  -0.4472f,   0.8507f,   0.54545f,   0.33333f,
       0.2764f,   0.4472f,   0.8507f,   0.72727f,   0.33333f,
       0.0000f,   1.0000f,   0.0000f,   0.90909f,   0.33333f,

       0.2764f,   0.4472f,  -0.8507f,   0.09091f,   0.66667f,
      -0.2764f,  -0.4472f,  -0.8507f,   0.27273f,   0.66667f,
       0.0000f,  -1.0000f,   0.0000f,   0.45455f,   0.66667f,
       0.7236f,  -0.4472f,   0.5258f,   0.63636f,   0.66667f,
       0.8944f,   0.4472f,   0.0000f,   0.81818f,   0.66667f,
       0.2764f,   0.4472f,  -0.8507f,   1.00000f,   0.66667f,

       0.7236f,  -0.4472f,  -0.5258f,   0.18182f,   1.00000f,
       0.7236f,  -0.4472f,  -0.5258f,   0.36364f,   1.00000f,
       0.7236f,  -0.4472f,  -0.5258f,   0.54545f,   1.00000f,
       0.7236f,  -0.4472f,  -0.5258f,   0.72727f,   1.00000f,
       0.7236f,  -0.4472f,  -0.5258f,   0.90909f,   1.00000f,
   };

   std::vector<GLushort> indices =
   {
      0,  5,  6,
      1,  6,  7,
      2,  7,  8,
      3,  8,  9,
      4,  9, 10,
      5, 11,  6,
      6, 12,  7,
      7, 13,  8,
      8, 14,  9,
      9, 15, 10,
      6, 11, 12,
      7, 12, 13,
      8, 13, 14,
      9, 14, 15,
      10, 15, 16,
      11, 17, 12,
      12, 18, 13,
      13, 19, 14,
      14, 20, 15,
      15, 21, 16,
   };

   FillBuffers(vertexData, indices);
}

void Geometry::MakeOctahedron()
{
   std::vector<GLfloat> vertexData =
   {
      //          POSITION                    TEXCOORD
       0.0000f,   0.0000f,   1.0000f,   0.8750f,   0.0000f,
       0.0000f,   0.0000f,   1.0000f,   0.6250f,   0.0000f,
       0.0000f,   0.0000f,   1.0000f,   0.3750f,   0.0000f,
       0.0000f,   0.0000f,   1.0000f,   0.1250f,   0.0000f,

       0.0000f,   1.0000f,   0.0000f,   1.0000f,   0.5000f,
      -1.0000f,   0.0000f,   0.0000f,   0.7500f,   0.5000f,
       0.0000f,  -1.0000f,   0.0000f,   0.5000f,   0.5000f,
       1.0000f,   0.0000f,   0.0000f,   0.2500f,   0.5000f,
       0.0000f,   1.0000f,   0.0000f,   0.0000f,   0.5000f,

       0.0000f,   0.0000f,  -1.0000f,   0.8750f,   1.0000f,
       0.0000f,   0.0000f,  -1.0000f,   0.6250f,   1.0000f,
       0.0000f,   0.0000f,  -1.0000f,   0.3750f,   1.0000f,
       0.0000f,   0.0000f,  -1.0000f,   0.1250f,   1.0000f,
   };

   std::vector<GLushort> indices =
   {
      0,  4,  5,
      1,  5,  6,
      2,  6,  7,
      3,  7,  8,
      4,  9,  5,
      5, 10,  6,
      6, 11,  7,
      7, 12,  8,
   };

   FillBuffers(vertexData, indices);
}

void Geometry::MakeCube(Format360 format, uint32_t texW, uint32_t texH)
{
   std::vector<GLfloat> vertexData =
   {
      //          POSITION                 TEXCOORD
      -1.000000f, -1.000000f, -1.000000f,  0.0f,  0.0f,
      -1.000000f,  1.000000f, -1.000000f,  0.0f,  1.0f,
       1.000000f,  1.000000f, -1.000000f,  1.0f,  1.0f,
       1.000000f, -1.000000f, -1.000000f,  1.0f,  0.0f,

      -1.000000f, -1.000000f,  1.000000f,  0.0f,  1.0f,
      -1.000000f,  1.000000f,  1.000000f,  1.0f,  1.0f,
      -1.000000f,  1.000000f, -1.000000f,  1.0f,  0.0f,
      -1.000000f, -1.000000f, -1.000000f,  0.0f,  0.0f,

       1.000000f, -1.000000f,  1.000000f,  1.0f,  1.0f,
       1.000000f,  1.000000f,  1.000000f,  1.0f,  0.0f,
      -1.000000f,  1.000000f,  1.000000f,  0.0f,  0.0f,
      -1.000000f, -1.000000f,  1.000000f,  0.0f,  1.0f,

       1.000000f, -1.000000f, -1.000000f,  0.0f,  1.0f,
       1.000000f,  1.000000f, -1.000000f,  1.0f,  1.0f,
       1.000000f,  1.000000f,  1.000000f,  1.0f,  0.0f,
       1.000000f, -1.000000f,  1.000000f,  0.0f,  0.0f,

       1.000000f,  1.000000f, -1.000000f,  1.0f,  0.0f,
      -1.000000f,  1.000000f, -1.000000f,  0.0f,  0.0f,
      -1.000000f,  1.000000f,  1.000000f,  0.0f,  1.0f,
       1.000000f,  1.000000f,  1.000000f,  1.0f,  1.0f,

      -1.000000f, -1.000000f, -1.000000f,  0.0f,  1.0f,
       1.000000f, -1.000000f, -1.000000f,  1.0f,  1.0f,
       1.000000f, -1.000000f,  1.000000f,  1.0f,  0.0f,
      -1.000000f, -1.000000f,  1.000000f,  0.0f,  0.0f,
   };

   std::vector<GLushort> indices =
   {
       0,  1,  2,
       3,  0,  2,
       4,  5,  6,
       7,  4,  6,
       8,  9, 10,
      11,  8, 10,
      12, 13, 14,
      15, 12, 14,
      16, 17, 18,
      19, 16, 18,
      20, 21, 22,
      23, 20, 22
   };

   if (format == FORMAT_CUBE_32_0 ||
       format == FORMAT_CUBE_32_90 ||
       format == FORMAT_CUBE_32_270 ||
       format == FORMAT_CUBE_32_P270 ||
       format == FORMAT_CUBE_43_0)
   {
      CubeFmt   fmt;
      CubeFace *face;
      GLfloat   tex[8];
      float     min[2], max[2], val;
      uint32_t  xIdx, yIdx, faceIdx;
      uint32_t  texSize[2] = { texW, texH };

      if (format == FORMAT_CUBE_32_0)
      {
         fmt.gridW = 3;         fmt.gridH = 2;
         fmt.face[0].posX = 1;  fmt.face[0].posY = 1;  fmt.face[0].rot = 0;
         fmt.face[1].posX = 2;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
         fmt.face[2].posX = 1;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
         fmt.face[3].posX = 0;  fmt.face[3].posY = 0;  fmt.face[3].rot = 0;
         fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 0;
         fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 0;
      }
      else if (format == FORMAT_CUBE_32_90)
      {
         fmt.gridW = 3;         fmt.gridH = 2;
         fmt.face[0].posX = 2;  fmt.face[0].posY = 0;  fmt.face[0].rot = 0;
         fmt.face[1].posX = 1;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
         fmt.face[2].posX = 0;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
         fmt.face[3].posX = 1;  fmt.face[3].posY = 1;  fmt.face[3].rot = 1;
         fmt.face[4].posX = 0;  fmt.face[4].posY = 1;  fmt.face[4].rot = 1;
         fmt.face[5].posX = 2;  fmt.face[5].posY = 1;  fmt.face[5].rot = 1;
      }
      else if (format == FORMAT_CUBE_32_270)
      {
         fmt.gridW = 3;         fmt.gridH = 2;
         fmt.face[0].posX = 2;  fmt.face[0].posY = 0;  fmt.face[0].rot = 0;
         fmt.face[1].posX = 1;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
         fmt.face[2].posX = 0;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
         fmt.face[3].posX = 1;  fmt.face[3].posY = 1;  fmt.face[3].rot = 3;
         fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 3;
         fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 3;
      }
      else if (format == FORMAT_CUBE_32_P270)
      {
         fmt.gridW = 3;         fmt.gridH = 2;
         fmt.face[0].posX = 1;  fmt.face[0].posY = 1;  fmt.face[0].rot = 3;
         fmt.face[1].posX = 2;  fmt.face[1].posY = 0;  fmt.face[1].rot = 0;
         fmt.face[2].posX = 1;  fmt.face[2].posY = 0;  fmt.face[2].rot = 0;
         fmt.face[3].posX = 0;  fmt.face[3].posY = 0;  fmt.face[3].rot = 0;
         fmt.face[4].posX = 2;  fmt.face[4].posY = 1;  fmt.face[4].rot = 0;
         fmt.face[5].posX = 0;  fmt.face[5].posY = 1;  fmt.face[5].rot = 0;
      }
      else if (format == FORMAT_CUBE_43_0)
      {
         fmt.gridW = 4;         fmt.gridH = 3;
         fmt.face[0].posX = 3;  fmt.face[0].posY = 1;  fmt.face[0].rot = 0;
         fmt.face[1].posX = 2;  fmt.face[1].posY = 1;  fmt.face[1].rot = 0;
         fmt.face[2].posX = 1;  fmt.face[2].posY = 1;  fmt.face[2].rot = 0;
         fmt.face[3].posX = 0;  fmt.face[3].posY = 1;  fmt.face[3].rot = 0;
         fmt.face[4].posX = 0;  fmt.face[4].posY = 0;  fmt.face[4].rot = 0;
         fmt.face[5].posX = 0;  fmt.face[5].posY = 2;  fmt.face[5].rot = 0;
      }

      // loop through each face
      for (faceIdx = 0; faceIdx < 6; faceIdx++)
      {
         face = &fmt.face[faceIdx];

         // get texture boundaries
         min[0] = (float)(face->posX + 0) / (float)fmt.gridW;
         max[0] = (float)(face->posX + 1) / (float)fmt.gridW;
         min[1] = (float)(face->posY + 0) / (float)fmt.gridH;
         max[1] = (float)(face->posY + 1) / (float)fmt.gridH;

         // match up face orientation to cube face
         if (face->rot == 0)
         {
            tex[0] = min[0];  tex[1] = max[1];
            tex[2] = min[0];  tex[3] = min[1];
            tex[4] = max[0];  tex[5] = min[1];
            tex[6] = max[0];  tex[7] = max[1];
         }
         else if (face->rot == 1)
         {
            tex[0] = max[0];  tex[1] = max[1];
            tex[2] = min[0];  tex[3] = max[1];
            tex[4] = min[0];  tex[5] = min[1];
            tex[6] = max[0];  tex[7] = min[1];
         }
         else if (face->rot == 2)
         {
            tex[0] = max[0];  tex[1] = min[1];
            tex[2] = max[0];  tex[3] = max[1];
            tex[4] = min[0];  tex[5] = max[1];
            tex[6] = min[0];  tex[7] = min[1];
         }
         else
         {
            tex[0] = min[0];  tex[1] = min[1];
            tex[2] = max[0];  tex[3] = min[1];
            tex[4] = max[0];  tex[5] = max[1];
            tex[6] = min[0];  tex[7] = max[1];
         }

         // round to nearest texel within face
         for (yIdx = 0; yIdx < 4; yIdx++)
         {
            for (xIdx = 0; xIdx < 2; xIdx++)
            {
               // round highest coordinate down, round lowest coordinate up
               val = tex[2 * yIdx + xIdx];
               if (val == max[xIdx])
                  val = (val * texSize[xIdx] - 0.5) / texSize[xIdx];
               else
                  val = (val * texSize[xIdx] + 0.5) / texSize[xIdx];

               // set texture coordinate within cube array
               vertexData[20 * faceIdx + 5 * yIdx + 3 + xIdx] = val;
            }
         }
      }
   }

   FillBuffers(vertexData, indices);
}

} // namespace
