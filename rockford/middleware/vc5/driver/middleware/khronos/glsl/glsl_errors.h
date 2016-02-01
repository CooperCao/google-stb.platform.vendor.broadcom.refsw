/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_ERRORS_H
#define GLSL_ERRORS_H

#include <setjmp.h>
#include "vcos.h"

/* A buffer holding the call-stack to which we'll jump on error */
extern jmp_buf g_ErrorHandlerEnv;

typedef enum
{
	ERROR_UNKNOWN,
	ERROR_CUSTOM,
	ERROR_PREPROCESSOR,
	ERROR_LEXER_PARSER,
	ERROR_SEMANTIC,
	ERROR_LINKER,
   WARNING
} ErrorType;

// Fetches a standard error string based on type and code, but if clarification is supplied, prints that too.
extern void glsl_compile_error(ErrorType e, int code, int line_num, const char *clarification, ...) VCOS_FORMAT_ATTR_(printf, 4, 5);

extern const char *glsl_compile_error_get(void);

extern void glsl_compile_error_reset(void);

#endif // ERRORS_H
