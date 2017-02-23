/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_STRINGBUILDER_H
#define GLSL_STRINGBUILDER_H

#include "middleware/khronos/glsl/glsl_fastmem.h"

typedef struct
{
   char* buf;
   size_t len; // note that len does not include the NULL byte, so the NULL byte will be stored at buf[len].
   size_t capacity; // this of course does include the NULL byte.
} StringBuilder;

#ifndef SB_INIT_CAPACITY
#define SB_INIT_CAPACITY 128
#endif

// Invalidate the internal pointer
void glsl_sb_invalidate(StringBuilder* sb);

// If buf is NULL, allocates SB_INIT_CAPACITY bytes of space.
// Otherwise just sets length to zero and adds NULL terminator.
void glsl_sb_reset(StringBuilder* sb);

// The string builder will ensure that it has enough space to write n more chars, so ensure this is accurate!
// n does *not* include the NULL terminator.
void glsl_sb_append(StringBuilder* sb, const size_t n, const char* format, ...);

// Backs up n characters (but stopping at 0).
void glsl_sb_back_up(StringBuilder* sb, const size_t n);

// Copies the current string out of the buffer (using strdup_fast()).
// Does not free builder memory or reset its length.
char* glsl_sb_copy_out_fast(StringBuilder* sb);

#endif // STRINGBUILDER_H
