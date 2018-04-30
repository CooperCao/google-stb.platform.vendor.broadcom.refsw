/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "libs/util/snprintf.h"

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
   for (;;) {
      c = *head;
      ++head;
      if (c == 0) {
         *dest++ = c;
         debug_print(buffer);
         break;
      }

      if (c == '\n') {
         if (dest != buffer) {
            *dest = 0;
            debug_print(buffer);
            dest = buffer;
         }
         ++line;
         *dest++ = '\n';
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
   debug_print("//\n");
   for (int i = 0; i < p->sourcec; i++) {
      format(p->sourcev[i], buffer);
      snprintf(buffer, sizeof(buffer), "// ^ Shader %d [%d]:\n",
               p->name, i);
      debug_print(buffer);
      debug_print("// -----------------------------------------------------------------\n");
   }
}
