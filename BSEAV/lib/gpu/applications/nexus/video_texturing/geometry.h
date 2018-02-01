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
   void  ToggleWireframe();

private:
   void MakeFlatRect();
   void MakeIcosahedron();
   void MakeOctahedron();
   void MakeCube(Format360 format, uint32_t texW, uint32_t texH);
   void MakeSphere(uint32_t divisions);
   void FillBuffers(const std::vector<GLfloat> &vertexData, const std::vector<GLushort> &solidIndices,
                    const std::vector<GLuint> &wfIndices);
   void FillBuffers(const std::vector<GLfloat> &vertexData, const std::vector<GLushort> &solidIndices);

private:
   GLuint  m_vbos[3];  // 0 for vertexData, 1 for solid indices, 2 for wireframe indices
   GLsizei m_solidIndexCount;
   GLsizei m_wfIndexCount;
   float   m_modelYRotate;
   GLenum  m_solidPrimitiveType;
   GLenum  m_wfPrimitiveType;
   bool    m_wireframe;
};

} // namespace
