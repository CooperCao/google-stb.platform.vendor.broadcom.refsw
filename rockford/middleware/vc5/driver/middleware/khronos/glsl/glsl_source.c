/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "helpers/snprintf.h"

#include "glsl_common.h" // For bool, strdup
#include "glsl_source.h"

#ifdef WIN32
__declspec(dllimport) void __stdcall OutputDebugStringA(const char * lpOutputString);
#  define debug_print(s) OutputDebugStringA(s)
#else
#  define debug_print(s) printf("%s",s)
#endif

static void format(const char *source, char *buffer)
{
   int         line = 2;
   const char  *head = source;
   char        c;
   char        *dest = buffer;

   *dest++ = '\n';
#if 0
   *dest++ = ' ';
   *dest++ = ' ';
   *dest++ = '1';
   *dest++ = ':';
   *dest++ = ' ';
#endif
   for (;;) {
      c = *head;
      ++head;
      if (c == 0) {
         *dest++ = c;
         debug_print(buffer);
         break;
      }

      if (c == '\n') {
#if 0
         int   x100  = line / 100;
         int   x10   = (line - (x100 * 100)) / 10;
         int   x     = line - (x100 * 100) - (x10 * 10);
         char  c100  = (x100 > 0) ? '0' + x100 : ' ';
         char  c10   = ((x100 > 0) || (x10 > 0)) ? '0' + x10 : ' ';
         char  c1    = '0' + x;
#endif
         if (dest != buffer) {
            *dest = 0;
            debug_print(buffer);
            dest = buffer;
         }
         ++line;
         *dest++ = '\n';
#if 0
         *dest++ = c100;
         *dest++ = c10;
         *dest++ = c1;
         *dest++ = ':';
         *dest++ = ' ';
#endif
         continue;
      }
      if (c == '\r')
         continue;
      *dest++ = c;
   }
   if (dest > buffer + 6) {
      *dest++ = '\n';
      *dest = 0;
      debug_print(buffer);
   } else if (dest != buffer) {
      dest = buffer;
      *dest++ = '\n';
      *dest = 0;
      debug_print(buffer);
   }
}

void glsl_shader_source_dump(const GLSL_SHADER_SOURCE_T *p)
{
   char buffer[10000];
   int i;
   debug_print("//\n");
   for (i = 0; i < p->sourcec; i++)
   {
      format(p->sourcev[i], buffer);
      snprintf(buffer, sizeof(buffer), "// ^ Shader %d [%d]:\n",
               p->name, i);
      debug_print(buffer);
      debug_print("// -----------------------------------------------------------------\n");
   }
}

void glsl_program_source_dump(const GLSL_PROGRAM_SOURCE_T *p)
{
   char buffer[10000];
   debug_print("//\n");

   for (unsigned i = 0; i < p->num_tf_varyings; i++)
   {
      snprintf(buffer, sizeof(buffer), "// TF Varying: %s\n", p->tf_varyings[i]);
      debug_print(buffer);
   }
}
