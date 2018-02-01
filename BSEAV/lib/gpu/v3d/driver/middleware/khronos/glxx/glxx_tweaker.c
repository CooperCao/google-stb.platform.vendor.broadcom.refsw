/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glxx/glxx_tweaker.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/include/GLES/gl.h"

void glxx_tweaker_init(TWEAK_STATE_T *ts, bool es2)
{
   ts->tweaks_disabled = khrn_options.disable_tweaks;
   ts->es2 = es2;
   ts->depthmask = true;
   ts->depthfunc = GL_LESS;
   ts->blend_enabled = false;
   ts->source_matches = false;
}

bool glxx_tweaker_update(TWEAK_STATE_T *ts)
{
   if (ts->tweaks_disabled)
      return false;

   if (ts->es2 && ts->source_matches && !ts->depthmask && ts->depthfunc == GL_EQUAL && ts->blend_enabled)
      return true;

   return false;
}

void glxx_tweaker_setdepthmask(TWEAK_STATE_T *ts, bool m)
{
   ts->depthmask = m;
}

void glxx_tweaker_setdepthfunc(TWEAK_STATE_T *ts, GLenum func)
{
   ts->depthfunc = func;
}

void glxx_tweaker_setblendenabled(TWEAK_STATE_T *ts, bool enabled)
{
   ts->blend_enabled = enabled;
}

void glxx_tweaker_setshadersource(TWEAK_STATE_T *ts, GLsizei count, const char *const*string, const GLint *length)
{
   if (ts->tweaks_disabled)
      return;

   ts->source_matches = false;

   if (count == 1 && length == NULL)
   {
      size_t len = 0;
      if (string && string[0])
         len = strlen(string[0]);

      if (len == 191)
      {
         uint32_t i;
         uint32_t sum = 0;

         /* Simple checksum */
         for (i = 0; i < len; i++)
            sum += string[0][i];

         if (sum == 0x38A7)
            ts->source_matches = true;
      }
   }
}
