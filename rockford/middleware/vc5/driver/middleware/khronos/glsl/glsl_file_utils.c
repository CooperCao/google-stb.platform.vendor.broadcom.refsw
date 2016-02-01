/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_file_utils.h"

const char *glsl_file_utils_basename(const char *filename) {
   const char *p;
   const char *ret;

   ret = filename;
   for(p = filename; *p; ++p) {
      if(*p == '/' || *p == '\\') {
         ret = p+1;
      }
   }
   return ret;
}
