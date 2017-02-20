/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <setjmp.h>
#include "libs/util/common.h"

EXTERN_C_BEGIN

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
extern void glsl_compile_error(ErrorType e, int code, int line_num, const char *clarification, ...) ATTRIBUTE_FORMAT(printf, 4, 5);

extern const char *glsl_compile_error_get(void);

extern void glsl_compile_error_reset(void);

EXTERN_C_END
