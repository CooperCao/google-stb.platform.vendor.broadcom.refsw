/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

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
