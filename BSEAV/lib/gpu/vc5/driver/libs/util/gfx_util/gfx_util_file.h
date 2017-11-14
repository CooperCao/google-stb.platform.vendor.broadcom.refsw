/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GFX_UTIL_FILE_H
#define GFX_UTIL_FILE_H

#include "libs/util/common.h"

#include <stddef.h>
#include <stdint.h>

EXTERN_C_BEGIN

extern void gfx_strip_extension(char *filename);

/* If filename ends with to_replace, replace it with replacement.
 * Otherwise, just append replacement to filename.
 * Free returned string with free(). */
extern char *gfx_replace_extension(const char *filename,
   const char *to_replace, const char *replacement);

/* Free returned string with free() */
extern char *gfx_replace_any_extension(const char *filename, const char *replacement);

/* Free returned pointer with free() */
extern void *gfx_load_binary_file_range(size_t *size, const char *filename,
   uint64_t offset, uint64_t max_size);

static inline void *gfx_load_binary_file(size_t *size, const char *filename)
{
   return gfx_load_binary_file_range(size, filename, 0, UINT64_MAX);
}

EXTERN_C_END

#endif
