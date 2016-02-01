/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "glsl_fastmem.h"
#include "glsl_stringbuilder.h"

#include "helpers/snprintf.h"
#include "vcos_string.h"

struct _StringBuilder {
   char* buffer;
   int length;   // does not include the NULL byte
   int capacity; // does not include the NULL byte
};

static void glsl_sb_init(StringBuilder* sb)
{
   sb->buffer = "";
   sb->length = 0;
   sb->capacity = 0;
}

StringBuilder* glsl_sb_new(void)
{
   StringBuilder* sb = malloc_fast(sizeof(StringBuilder));
   glsl_sb_init(sb);
   return sb;
}

void glsl_sb_append(StringBuilder* sb, const char* format, ...)
{
   while (true) {
      int new_length = 0;

      // Print the string if we have writeable buffer
      // In particular, do not write to the constant ""
      if (sb->capacity > sb->length) {
         va_list argp;
         va_start(argp, format);
         new_length = vcos_safe_vsprintf(sb->buffer, sb->capacity + 1, sb->length, format, argp);
         va_end(argp);
         if (sb->length <= new_length && new_length <= sb->capacity) {
            sb->length = new_length;
            return;
         }
      }

      // Increase the buffer capacity and retry
      // 'new_length' may or may not contain the exact needed size depending on platform
      sb->capacity = vcos_max(new_length, vcos_max(sb->capacity * 2, 15));
      sb->buffer = memcpy(malloc_fast(sb->capacity + 1), sb->buffer, sb->length + 1);
   }
}

const char* glsl_sb_content(StringBuilder* sb)
{
   // Mark the buffer as full so that further calls to glsl_sb_append will
   // grow the buffer instead of modifying the value returned to the user.
   sb->capacity = sb->length;
   return sb->buffer;
}

const char* asprintf_fast(const char* format, ...)
{
   size_t size;
   for (size = 16; ; size *= 2) {
      char* buffer = malloc_fast(size);
      int length;
      va_list argp;
      va_start(argp, format);
      length = vsnprintf(buffer, size, format, argp);
      va_end(argp);
      if (0 <= length && (size_t)length < size)
         return buffer;
   }
}
