/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

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
   "Invalid use of both gl_FragColor and gl_FragData",
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
   "Argument cannot be used as out or inout parameter",
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
   "Texture offset must be a constant expression",
   //00026
   "Duplicate labels in switch statement list",
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
   "#extension if a required extension extension_name is not supported, or if all is specified",
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
   BAD_ERROR_CODE,
   //S0003
   BAD_ERROR_CODE,
   //S0004
   "Operator not supported for operand types",
   //S0005
   BAD_ERROR_CODE,
   //S0006
   BAD_ERROR_CODE,
   //S0007
   "Invalid constructor",
   //S0008
   BAD_ERROR_CODE,
   //S0009
   BAD_ERROR_CODE,
   //S0010
   BAD_ERROR_CODE,
   //S0011
   BAD_ERROR_CODE,
   //S0012
   BAD_ERROR_CODE,
   //S0013
   "Invalid initialiser",
   //S0014
   BAD_ERROR_CODE,
   //S0015
   "Expression must be an integral constant expression",
   //S0016
   BAD_ERROR_CODE,
   //S0017
   "Array size must be greater than zero",
   //S0018
   BAD_ERROR_CODE,
   //S0019
   BAD_ERROR_CODE,
   //S0020
   "Indexing with a constant expression out of bounds",
   //S0021
   BAD_ERROR_CODE,
   //S0022
   "Redefinition of name in same scope",
   //S0023
   BAD_ERROR_CODE,
   //S0024
   BAD_ERROR_CODE,
   //S0025
   BAD_ERROR_CODE,
   //S0026
   "Illegal field selector",
   //S0027
   "Target of assignment is not an l-value",
   //S0028
   "Precision used with type other than integer, floating point or opaque type",
   //S0029
   "Declaring a main function with the wrong signature or return type",
   //S0030
   "Static recursion present",
   //S0031
   "Overloading built-in functions not allowed.",
   //S0032
   "Use of float or int without a precision qualifier where the default precision is not defined",
   //S0033
   "Expression that does not have an intrinsic precision where the default precision is not defined",
   //S0034
   "Variable cannot be declared invariant",
   //S0035
   BAD_ERROR_CODE,
   //S0036
   BAD_ERROR_CODE,
   //S0037
   BAD_ERROR_CODE,
   //S0038
   BAD_ERROR_CODE,
   //S0039
   BAD_ERROR_CODE,
   //S0040
   BAD_ERROR_CODE,
   //S0041
   "Function return type is an array",
   //S0042
   "Return type of function definition must match return type of function declaration",
   //S0043
   "Parameter qualifiers of function definition must match parameter qualifiers of function declaration",
   //S0044
   BAD_ERROR_CODE,
   //S0045
   BAD_ERROR_CODE,
   //S0046
   BAD_ERROR_CODE,
   //S0047
   "Interface variables must be declared at global scope",
   //S0048
   "Illegal data type for vertex output or fragment input",
   //S0049
   "Illegal data type for vertex input (can only use float, floating-point vectors, matrices, signed and unsigned integers and integer vectors)",
   //S0050
   "Illegal data type for fragment output",
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
   "Too many vertex output values (varyings)",
   //L0005
   "Too many uniform values",
   //L0006
   "Too many fragment output values",
   //L0007
   "Fragment shader uses a varying that has not been declared in the vertex shader",
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

void glsl_compiler_exit()
{
   longjmp(g_ErrorHandlerEnv, 1);
}

/*
   glsl_compile_error

   Prints out a suitable message to the console. Exits compilation if it is an error (rather than a warning).

   (clarification, ...) must form a valid printf sequence
*/


#define MAX_ERROR_LENGTH 256
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

void glsl_compile_error(ErrorType e, int code, int line_num, const char *clarification, ...)
{
   va_list argp;
   const char *kind = e == WARNING ? "WARNING" : "ERROR";

   error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset,
                                    "%s:%s-%d (line %d) %s", kind,
                                    ErrorTypeStrings[e], code,
                                    line_num, ErrorStrings[e][code]);

   if (clarification)
   {
      const char sep[] = " : ";

      error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset, "%s", sep);

      va_start(argp, clarification);
      error_offset = VCOS_SAFE_VSPRINTF(error_buffer, error_offset, clarification, argp);
      va_end(argp);
   }

   error_offset = VCOS_SAFE_SPRINTF(error_buffer, error_offset, "\n");

   if (e != WARNING) {
      glsl_compiler_exit();
   }
}
