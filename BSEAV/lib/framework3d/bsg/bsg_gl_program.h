/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GLPROGRAM_H__
#define __BSG_GLPROGRAM_H__

#include "bsg_glapi.h"

#include "bsg_common.h"
#include "bsg_no_copy.h"

#include <string>
#include <vector>
#include <map>

namespace bsg
{
// @cond
class GLProgram : public NoCopy
{
public:
   GLProgram();
   GLProgram(const std::string &vert, const std::string &frag, const std::vector<std::string> &defines);
   GLProgram(
      const std::string &vert,
      const std::string &frag,
      const std::string &tc,
      const std::string &te,
      const std::string &geom,
      const std::vector<std::string> &defines);
   ~GLProgram();

   void SetPrograms(const std::string &vert, const std::string &frag, const std::vector<std::string> &defines);
   void SetPrograms(
      const std::string &vert,
      const std::string &frag,
      const std::string &tc,
      const std::string &te,
      const std::string &geom,
      const std::vector<std::string> &defines);
   void Use();

   // TODO interface to query program properties

   GLint GetUniformLocation(const std::string &name) const;
   GLint GetAttribLocation(const std::string &name) const;

private:
   void Finish();
   void BuildLazySources();
   void SetActualPrograms(const std::string &vert, const std::string &frag);
   void HandleDefinesIfNeeded();

private:
   GLint m_vert;
   GLint m_frag;
   GLint m_tessc;
   GLint m_tesse;
   GLint m_geom;
   GLint m_prog;

   mutable std::map<std::string, GLint>  m_uniformLocs;
   mutable std::map<std::string, GLint>  m_attribLocs;
};
// @endcond
}

#endif
