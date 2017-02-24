/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/* This a basic memory manager for the GLSL compiler. It provides lightweight wrappers
   over the standard library functions, which have the following two properties:
   - Any memory allocated and not previously free'd will be free'd when glsl_cleanup_safe is called
   - Any failure in allocation will trigger a compiler error (Out of memory) and subsequent exit
*/

#include <stddef.h>
#include "libs/util/common.h"

EXTERN_C_BEGIN

#ifndef GLSL_MEMORY_DEBUG

void *glsl_safemem_malloc (size_t size);
void *glsl_safemem_calloc (size_t nmemb, size_t size);
void *glsl_safemem_realloc(void *data, size_t size);

#else

void *glsl_safemem_malloc_debug (size_t size, const char *file, int line);
void *glsl_safemem_calloc_debug (size_t nmemb, size_t size, const char *file, int line);
void *glsl_safemem_realloc_debug(void *data, size_t size, const char *file, int line);

#define glsl_safemem_malloc(size)        glsl_safemem_malloc_debug ((size),          __FILE__, __LINE__)
#define glsl_safemem_calloc(nmemb, size) glsl_safemem_calloc_debug ((nmemb), (size), __FILE__, __LINE__)
#define glsl_safemem_realloc(data, size) glsl_safemem_realloc_debug((data), (size),  __FILE__, __LINE__)

#endif

void  glsl_safemem_free   (void *data);
void  glsl_safemem_cleanup();
void  glsl_safemem_verify ();
void  glsl_safemem_dump   ();

EXTERN_C_END
