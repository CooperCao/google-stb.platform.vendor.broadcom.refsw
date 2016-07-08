/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_TRACE_H
#define GLSL_TRACE_H

#include <stdio.h>

#include "glsl_ast_print.h"

#define SLANG_TRACE 0
#define SLANG_TRACE_AST 0
#define SLANG_TRACE_AST_EXPRS_EVALUATED 0

#if defined(_DEBUG) && SLANG_TRACE

#define TRACE(a) \
   { \
      printf a; \
   }

#define TRACE_CONSTANT(t, v) \
   { \
      printf("constant: "); \
      glsl_print_compile_time_value(stdout, t, v); \
      printf("\n"); \
   }

#define TRACE_PHASE(phase) \
   { \
      printf("\n\n~~~~~ "); \
      printf phase; \
      printf(" ~~~~~\n\n"); \
   }


#else

#define TRACE(a) /**/
#define TRACE_CONSTANT(t, v) /**/
#define TRACE_PHASE(phase) /**/

#endif // defined(_DEBUG) && SLANG_TRACE



#if defined(_DEBUG) && SLANG_TRACE_AST

#ifdef SLANG_TRACE_AST_EXPRS_EVALUATED
#undef SLANG_TRACE_AST_EXPRS_EVALUATED
#define SLANG_TRACE_AST_EXPRS_EVALUATED true
#else
#define SLANG_TRACE_AST_EXPRS_EVALUATED false
#endif

#define TRACE_STATEMENT(s) \
   { \
      glsl_print_statement(stdout, s, SLANG_TRACE_AST_EXPRS_EVALUATED, 0, false); \
   }

#else

#define TRACE_STATEMENT(s) /**/

#endif // defined(_DEBUG) && SLANG_TRACE_AST

#endif // TRACE_H
