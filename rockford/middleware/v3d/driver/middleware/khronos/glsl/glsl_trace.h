/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_TRACE_H
#define GLSL_TRACE_H

#include <stdio.h>

#include "middleware/khronos/glsl/glsl_globals.h"
#include "middleware/khronos/glsl/glsl_ast_print.h"
#include "middleware/khronos/glsl/glsl_dataflow_print.h"

#define SLANG_TRACE 0
#define SLANG_TRACE_AST 0
#define SLANG_TRACE_AST_EXPRS_EVALUATED 0
#define SLANG_TRACE_DATAFLOW 1

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



#if defined(_DEBUG) && SLANG_TRACE_DATAFLOW

#define TRACE_DATAFLOW_VERT_FILE "Vert.dot"
#define TRACE_DATAFLOW_FRAG_FILE "Frag.dot"
#define TRACE_DATAFLOW_FROM_ROOT(flavour, root, pass) \
   { \
      FILE* f; \
      const char* fn; \
      fn = SHADER_VERTEX == flavour ? TRACE_DATAFLOW_VERT_FILE : TRACE_DATAFLOW_FRAG_FILE; \
      f = fopen(fn, "w"); \
      printf("dumping dataflow graph to <%s>...\n", fn); \
      glsl_print_dataflow_from_root(f, root, pass); \
      fclose(f); \
   }
#define TRACE_DATAFLOW_FROM_ROOTS(flavour, roots, order, pass) \
   { \
      FILE* f; \
      const char* fn; \
      fn = SHADER_VERTEX == flavour ? TRACE_DATAFLOW_VERT_FILE : TRACE_DATAFLOW_FRAG_FILE; \
      f = fopen(fn, "w"); \
      printf("dumping dataflow graph to <%s>...\n", fn); \
      glsl_print_dataflow_from_roots(f, roots, order, pass); \
      fclose(f); \
   }

#else

#define TRACE_DATAFLOW(flavour, root, pass) /**/
#define TRACE_DATAFLOW_FROM_ROOTS(flavour, roots, order, pass) /**/

#endif // defined(_DEBUG) && SLANG_TRACE_DATAFLOW

#endif // TRACE_H
