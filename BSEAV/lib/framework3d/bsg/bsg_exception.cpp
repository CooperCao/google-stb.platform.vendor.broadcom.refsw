/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_exception.h"

#include <iostream>

#include "bsg_glapi.h"

namespace bsg
{

static const char *GLErrString(GLenum err)
{
#define ERR_CASE_GL(X)  case X: return #X

   switch (err)
   {
   ERR_CASE_GL(GL_NO_ERROR);
   ERR_CASE_GL(GL_INVALID_ENUM);
   ERR_CASE_GL(GL_INVALID_VALUE);
   ERR_CASE_GL(GL_INVALID_OPERATION);
   ERR_CASE_GL(GL_OUT_OF_MEMORY);
   default: return "Unknown";
   }

#undef ERR_CASE_GL
}

void Oops(Exception x)
{
#ifndef NDEBUG
   std::cerr << "Exception : " << x.Message() << "\n";
#endif
   throw x;
}

void GLOops(const char *file, uint32_t line)
{
   GLenum   err = glGetError();

   if (err != GL_NO_ERROR)
      std::cerr << "GL Error at line " << line << " of file " << file << " (" << GLErrString(err) << ")\n";
}

}
