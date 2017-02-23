/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_STRINGBUILDER_H
#define GLSL_STRINGBUILDER_H

typedef struct _StringBuilder StringBuilder;

// Create new empty string builder
StringBuilder* glsl_sb_new(void);

// Append formatted text at the end
void glsl_sb_append(StringBuilder* sb, const char* format, ...);

// Return the content of the string builder
// Further calls to append will not change the returned value
const char* glsl_sb_content(StringBuilder* sb);

// Format string and return it
const char* asprintf_fast(const char* format, ...);

#endif // STRINGBUILDER_H
