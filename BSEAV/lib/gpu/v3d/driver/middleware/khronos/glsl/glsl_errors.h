/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_ERRORS_H
#define GLSL_ERRORS_H

#include "middleware/khronos/glsl/glsl_compiler.h"

extern int lineNumber;

typedef enum
{
   ERROR_UNKNOWN,
   ERROR_CUSTOM,
   ERROR_PREPROCESSOR,
   ERROR_LEXER_PARSER,
   ERROR_SEMANTIC,
   ERROR_LINKER,
   ERROR_OPTIMIZER,
   WARNING
} ErrorType;

// Fetches a standard error string based on type and code, but if clarification is supplied, prints that too.
void glsl_compile_error(ErrorType e, int code, int line_num, const char* clarification, ...);
void glsl_link_error(ErrorType e, int code, int line_num, const char* clarification, ...);

extern char* ErrorsCustom[];
extern char* ErrorsPreprocessor[];
extern char* ErrorsLexerParser[];
extern char* ErrorsSemantic[];
extern char* ErrorsLinker[];
extern char* ErrorsOptimizer[];
extern char* Warnings[];

extern char* ErrorTypeStrings[];
extern char** ErrorStrings[];

#endif // ERRORS_H
