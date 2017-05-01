/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gfx_util_file.h"
#include "libs/util/demand.h"
#include "libs/util/assert_helpers.h"
#include "vcos_string.h"
#include <limits.h>
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

void *gfx_load_binary_file_range(size_t *size, const char *filename,
   uint64_t offset, uint64_t max_size)
{
   FILE *f = fopen(filename, "rb");
   demand_msg(f, "Couldn't open '%s' for reading!", filename);

   demand(fseek(f, 0, SEEK_END) == 0);
   long tell = ftell(f); // Note this may return a bogus value on 32-bit systems for >=2GB files...
   demand(tell >= 0);
   uint64_t file_size = tell;

   demand(offset <= file_size);
   uint64_t size_64 = file_size - offset;
   if (max_size < size_64)
      size_64 = max_size;
   demand(size_64 <= SIZE_MAX);
   *size = (size_t)size_64;

   void *mem = malloc(*size);
   demand(mem);

   demand(offset <= LONG_MAX);
   demand(fseek(f, (long)offset, SEEK_SET) == 0);

   size_t bytes_read = fread(mem, 1, *size, f);
   demand(bytes_read == *size);

   fclose(f);

   return mem;
}
