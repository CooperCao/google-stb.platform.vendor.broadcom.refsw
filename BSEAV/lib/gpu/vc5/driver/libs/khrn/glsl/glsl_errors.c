/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"
#include "glsl_errors.h"

#include <stdio.h>
#include <stdarg.h>
#include "vcos_string.h"

#define BAD_ERROR_CODE "Invalid error code"

jmp_buf g_ErrorHandlerEnv;

static const char * const ErrorsCustom[] =
{
   //00000
   BAD_ERROR_CODE,
   //00001
   "Identifier is not a variable or parameter",
   //00002
   "Subscript must be integral type",
   //00003
   "Expression cannot be subscripted",
   //00004
   "Invalid interface block declaration",
   //00005
   "Invalid fragment shader interface",
   //00006
   "Out of memory",
   //00007
   "case and default labels may only appear inside switches",
   //00008
   "type of init-expression in switch statement must match the type of the case labels",
   //00009
   "Identifier is not a function name. Perhaps function is hidden by variable or type name?",
   //00010
   "const type qualifier cannot be used with out or inout parameter qualifier",
   //00011
   "No declared overload matches function call arguments",
   //00012
   "Discard statement can only be used in fragment shader",
   //00013
   "Shader without 'main' function",
   //00014
   "Declaring a symbol beginning with gl_",
   //00015
   "Invalid qualifier",
   //00016
   "Invalid argument",
   //00017
   "Nested struct not supported",
   //00018
   "Cannot instantiate void type",
   //00019
   "void type can only be used in empty formal parameter list",
   //00020
   "Use of opaque type in interface block declaration",
   //00021
   "Function call without function definition",
   //00022
   "Sampler type can only be used for uniform or function input parameter",
   //00023
   "Use of break or continue outside loop or switch",
   //00024
   "Function prototype not allowed within function body",
   //00025
   BAD_ERROR_CODE,
   //00026
   "Invalid switch statement",
   //00027
   "Members of array cannot be arrays",
   //00028
   "Incomplete array type",
};

static const char * const ErrorsPreprocessor[] =
{
   //P0000
   BAD_ERROR_CODE,
   //P0001
   "Preprocessor syntax error",
   //P0002
   "#error",
   //P0003
   "Bad #extension",
   //P0004
   "Use of undefined macro",
   //P0005
   "Invalid #version construct",
};

static const char * const ErrorsLexerParser[] =
{
   //L0000
   BAD_ERROR_CODE,
   //L0001
   "Syntax error",
   //L0002
   "Undefined identifier",
   //L0003
   "Use of reserved keywords",
   //L0004
   "Identifier too long",
   //L0005
   "Integer constant too long",
};

static const char * const ErrorsSemantic[] =
{
   //S0000
   BAD_ERROR_CODE,
   //S0001
   "Type mismatch in expression",
   //S0002
   "Invalid initialiser",
   //S0003
   "Invalid constructor",
   //S0004
   "Operator not supported for operand types",
   //S0005
   "Read expression is not an r-value",
   //S0006
   "Function definition inconsistent with declaration",
   //S0007
   "Interface variables must be declared at global scope",
   //S0008
   "Illegal data type for input or output",
   //S0009
   "Variable cannot be declared invariant",
   //S0010
   "Expression must be an integral constant expression",
   //S0011
   "Array size must be greater than zero",
   //S0012
   "Indexing with a constant expression out of bounds",
   //S0013
   "Redefinition of name in same scope",
   //S0014
   "Illegal field selector",
   //S0015
   "Target of assignment is not an l-value",
   //S0016
   "Precision used with type other than integer, floating point or opaque type",
   //S0017
   "Declaring a main function with the wrong signature or return type",
   //S0018
   "Static recursion present",
   //S0019
   "Overloading built-in functions not allowed.",
};

static const char * const ErrorsLinker[] =
{
   //L0000
   BAD_ERROR_CODE,
   //L0001
   "Type mismatch",
   //L0002
   "Out of memory",
   //L0003
   "Too many vertex input values",
   //L0004
   "Invalid output declaration",
   //L0005
   "Too many uniform values",
   //L0006
   "Invalid combination of stages",
   //L0007
   "Input variable not declared in the previous shader stage",
   //L0008
   "Shader language version mismatch",
   //L0009
   "Invalid Transform Feedback",
   //L0010
   "Program limit exceeded",
};

static const char * const Warnings[] =
{
   //W0000
   BAD_ERROR_CODE,
   //W0001
   "Unsupported extension",
   //W0002
   "Using extension",
   //W0003
   "feature not implemented",
   //W0004
   "Indexing with an integral constant expression greater than declared size",
};

static const char * const ErrorTypeStrings[] =
{
   "UNKNOWN", // ERROR_UNKNOWN
   "CUSTOM", // ERROR_CUSTOM
   "PREPROCESSOR", // ERROR_PREPROCESSOR
   "LEX/PARSE", // ERROR_LEXER_PARSER
   "SEMANTIC", // ERROR_SEMANTIC
   "LINK", // ERROR_LINKER
   "WARN" // WARNING
};

static const char * const * const ErrorStrings[] =
{
   NULL, // ERROR_UNKNOWN
   ErrorsCustom, // ERROR_CUSTOM
   ErrorsPreprocessor, // ERROR_PREPROCESSOR
   ErrorsLexerParser, // ERROR_LEXER_PARSER
   ErrorsSemantic, // ERROR_SEMANTIC
   ErrorsLinker, // ERROR_LINKER
   Warnings // WARNING
};

void glsl_compiler_exit() {
   longjmp(g_ErrorHandlerEnv, 1);
}


#define MAX_ERROR_LENGTH 1024
static char error_buffer[MAX_ERROR_LENGTH];
static int error_offset;

void glsl_compile_error_reset(void)
{
   error_buffer[0] = '\0';
   error_offset = 0;
}

const char *glsl_compile_error_get(void)
{
   return error_buffer;
}

void glsl_compile_verror(ErrorType e, int code, int line_num, const char *clarification, va_list ap)
{
   const char *kind = e == WARNING ? "WARNING" : "ERROR";

   error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset,
                                    "%s:%s-%d (line %d) %s", kind,
                                    ErrorTypeStrings[e], code,
                                    line_num, ErrorStrings[e][code]);

   if (clarification)
   {
      const char sep[] = " : ";

      error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset, "%s", sep);

      error_offset = VCOS_SAFE_VSPRINTF(error_buffer, error_offset, clarification, ap);
   }

   error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset, "\n");
}

void glsl_compile_error(ErrorType e, int code, int line_num, const char *clarification, ...)
{
   va_list argp;

   va_start(argp, clarification);
   glsl_compile_verror(e, code, line_num, clarification, argp);
   va_end(argp);

   if (e != WARNING) {
      glsl_compiler_exit();
   }
}
