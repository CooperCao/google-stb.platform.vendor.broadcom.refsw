/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "gfx_util_file.h"
#include "libs/util/demand.h"
#include "libs/util/assert_helpers.h"
#include "vcos_string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void gfx_strip_extension(char *filename)
{
   char *dot = strrchr(filename, '.');
   char *slash = strrchr(filename, '/');
   char *after_slash = slash ? (slash + 1) : filename;
   if (dot && (dot > after_slash))
      *dot = '\0';
}

char *gfx_replace_extension(const char *filename,
   const char *to_replace, const char *replacement)
{
   size_t orig_len = strlen(filename);
   size_t to_replace_len = strlen(to_replace);
   size_t replacement_len = strlen(replacement);

   char *new_filename;
   if ((orig_len >= to_replace_len) &&
      (strcmp(filename + orig_len - to_replace_len, to_replace) == 0))
   {
      /* filename ends with to_replace; just replace that with replacement */
      new_filename = malloc(orig_len - to_replace_len + replacement_len + 1);
      demand(new_filename);
      memcpy(new_filename, filename, orig_len - to_replace_len);
      strcpy(new_filename + orig_len - to_replace_len, replacement);
   }
   else
   {
      /* filename doesn't end with to_replace. Just append replacement. */
      new_filename = malloc(orig_len + replacement_len + 1);
      demand(new_filename);
      memcpy(new_filename, filename, orig_len);
      strcpy(new_filename + orig_len, replacement);
   }

   return new_filename;
}

char *gfx_replace_any_extension(const char *filename, const char *replacement)
{
   char *stem = strdup(filename);
   demand(stem);
   gfx_strip_extension(stem);

   char *stem_plus_replacement;
   demand(vcos_asprintf(&stem_plus_replacement, "%s%s", stem, replacement) >= 0);

   free(stem);

   return stem_plus_replacement;
}

void *gfx_load_binary_file(size_t *size, const char *filename)
{
   FILE *f;
   long tell;
   void *mem;
   size_t bytes_read;

   f = fopen(filename, "rb");
   demand_msg(f, "Couldn't open '%s' for reading!", filename);

   demand(fseek(f, 0, SEEK_END) == 0);
   tell = ftell(f);
   demand(tell > 0);
   *size = tell;
   demand(fseek(f, 0, SEEK_SET) == 0);

   mem = malloc(*size);
   demand(mem);

   bytes_read = fread(mem, 1, *size, f);
   demand(bytes_read == *size);

   fclose(f);

   return mem;
}
