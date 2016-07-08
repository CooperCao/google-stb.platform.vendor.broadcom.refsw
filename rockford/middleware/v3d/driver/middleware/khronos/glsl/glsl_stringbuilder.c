/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "middleware/khronos/glsl/glsl_fastmem.h"

#include "middleware/khronos/glsl/glsl_stringbuilder.h"

void glsl_sb_invalidate(StringBuilder* sb)
{
   sb->buf = NULL;
   sb->capacity = 0;
}

void glsl_sb_reset(StringBuilder* sb)
{
   sb->len = 0;

   if (sb->buf)
   {
      vcos_assert(sb->capacity >= SB_INIT_CAPACITY);
      sb->buf[0] = '\0';
      return;
   }
   else
   {
      sb->buf = (char *)malloc_fast(SB_INIT_CAPACITY);
      sb->capacity = SB_INIT_CAPACITY;
      sb->buf[0] = '\0';
   }
}

void glsl_sb_append(StringBuilder* sb, const size_t n, const char* format, ...)
{
   size_t c;
   va_list argp;

   if (sb->buf && sb->capacity)
   {
      size_t capacity_req = sb->len + n + 1;

      // Ensure the buffer is big enough.
      // Hopefully, though, this will never be necessary, as SB_INIT_CAPACITY should be big enough.
      if (capacity_req > sb->capacity)
      {
         size_t capacity_new = capacity_req * 2;
         char* buf_new;

         buf_new = (char *)malloc_fast((int32_t)capacity_new);

         khrn_memcpy(buf_new, sb->buf, sb->capacity);

         sb->capacity = capacity_new;
         sb->buf = buf_new;
      }

      vcos_assert(capacity_req <= sb->capacity);

      // Print the string.
      va_start(argp, format);
      c = vsnprintf(sb->buf + sb->len, sb->capacity, format, argp);
      va_end(argp);

      // Work out the new length.
      vcos_assert(c <= n);
      sb->len += c;
   }
}

void glsl_sb_back_up(StringBuilder* sb, const size_t n)
{
   sb->len = (sb->len >= n) ? sb->len - n : 0;
   sb->buf[sb->len] = '\0';
}

char* glsl_sb_copy_out_fast(StringBuilder* sb)
{
   return strdup_fast(sb->buf);
}
