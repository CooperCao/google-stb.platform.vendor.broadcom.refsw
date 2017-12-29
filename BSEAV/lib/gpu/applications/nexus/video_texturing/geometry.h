/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#ifdef VC5
#include <GLES3/gl3.h>
#else // VC4
#include <GLES2/gl2.h>
#endif

#include "formats_360.h"

#include <vector>

namespace video_texturing
{

class Geometry
{
public:
   Geometry(bool is360, Format360 format, uint32_t texW, uint32_t texH);
   ~Geometry();

   void  Bind(GLint posLocation, GLint texCoordLocation);
   void  Draw();
   float GetModelYRotate() const { return m_modelYRotate; }

private:
   void MakeFlatRect();
   void MakeIcosahedron();
   void MakeOctahedron();
   void MakeCube(Format360 format, uint32_t texW, uint32_t texH);
   void FillBuffers(const std::vector<GLfloat> &vertexData, const std::vector<GLushort> &indices);

private:
   GLuint  m_vbos[2];  // 1 for vertexData, 1 for indices
   GLsizei m_indexCount;
   float   m_modelYRotate;
};

} // namespace
