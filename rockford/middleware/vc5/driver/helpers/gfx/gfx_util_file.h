/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_UTIL_FILE_H
#define GFX_UTIL_FILE_H

#include "vcos_types.h"

#include <stddef.h>

VCOS_EXTERN_C_BEGIN

extern void gfx_strip_extension(char *filename);

/* If filename ends with to_replace, replace it with replacement.
 * Otherwise, just append replacement to filename.
 * Free returned string with free(). */
extern char *gfx_replace_extension(const char *filename,
   const char *to_replace, const char *replacement);

/* Free returned string with free() */
extern char *gfx_replace_any_extension(const char *filename, const char *replacement);

/* Free returned pointer with free() */
extern void *gfx_load_binary_file(size_t *size, const char *filename);

VCOS_EXTERN_C_END

#endif
