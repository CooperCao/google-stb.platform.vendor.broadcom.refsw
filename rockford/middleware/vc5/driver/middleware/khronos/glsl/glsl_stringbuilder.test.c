/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>
#include <string.h>

#include "glsl_common.h"
#include "glsl_fastmem.h"
#include "glsl_stringbuilder.h"

int main(int argc, char **argv)
{
   glsl_fastmem_init();

   // Initial value
   {
      StringBuilder* sb = glsl_sb_new();
      assert(!strcmp(glsl_sb_content(sb), ""));
   }

   // Append empty string
   {
      StringBuilder* sb = glsl_sb_new();
      glsl_sb_append(sb, "");
      assert(!strcmp(glsl_sb_content(sb), ""));
   }

   // Result of glsl_sb_content not modified
   {
      StringBuilder* sb = glsl_sb_new();
      glsl_sb_append(sb, "1234567890");
      const char* old_conent = glsl_sb_content(sb);
      glsl_sb_append(sb, ";");
      assert(!strcmp(old_conent, "1234567890"));
      assert(!strcmp(glsl_sb_content(sb), "1234567890;"));
   }

   // Create long string, one character at a time
   {
      int length;
      for (length = 0; length < 1000; length++) {
         StringBuilder* sb = glsl_sb_new();
         char expected[1024] = { 0 };
         int i;
         for (i = 0; i < length; i++) {
            char c = 'a' + i % 26;
            glsl_sb_append(sb, "%c", c);
            expected[i] = c;
         }
         assert(!strcmp(glsl_sb_content(sb), expected));
         vcos_unused_in_release(expected);

         // check that asprintf_fast can create this string
         assert(!strcmp(expected, asprintf_fast("%s", expected)));
      }
   }

   // Random appends
   {
      const char str[] = "1234567890abcdef";
      const int max_num_appends = 20;
      int run;
      for (run = 0; run < 1000; run++) {
         StringBuilder* sb = glsl_sb_new();
         char expected[sizeof(str) * 20];
         int i;
         int num_appends = rand() % max_num_appends;
         expected[0] = 0;
         for (i = 0; i < num_appends; i++) {
            char substr[sizeof(str)] = { 0 };
            strncat(substr, str, rand() % sizeof(str));
            glsl_sb_append(sb, "%s", substr);
            strcat(expected, substr);
         }
         assert(!strcmp(glsl_sb_content(sb), expected));
      }
   }

   glsl_fastmem_term();

   return 0;
}
