/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
