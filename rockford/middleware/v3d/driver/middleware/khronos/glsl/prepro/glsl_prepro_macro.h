/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_PREPRO_MACRO_H
#define GLSL_PREPRO_MACRO_H

#include "middleware/khronos/glsl/prepro/glsl_prepro_token.h"

#ifdef _DEBUG
#define MACRO_DEBUG        // compile dump routines
#endif

typedef enum {
   MACRO_UNDEF,
   MACRO_OBJECT,
   MACRO_FUNCTION,
   MACRO_LINE,
   MACRO_FILE
} MacroType;

typedef struct _Macro {
   MacroType type;

   Token *name;
   TokenList *args;
   TokenSeq *body;
} Macro;

extern Macro *glsl_macro_construct_undef(Token *name);
extern Macro *glsl_macro_construct_object(Token *name, TokenSeq *body);
extern Macro *glsl_macro_construct_function(Token *name, TokenList *args, TokenSeq *body);
extern Macro *glsl_macro_construct_line(void);
extern Macro *glsl_macro_construct_file(void);

extern bool glsl_macro_equals(Macro *m1, Macro *m2);

typedef struct _MacroList {
   Macro *macro;

   struct _MacroList *next;
} MacroList;

extern MacroList *glsl_macrolist_construct(Macro *macro, MacroList *next);
extern MacroList *glsl_macrolist_construct_initial(void);

extern Macro *glsl_macrolist_find(MacroList *list, Token *name);

extern bool glsl_is_defined(MacroList *list, Token *name);

extern TokenSeq *glsl_remove_defined(TokenSeq *seq);

#ifdef MACRO_DEBUG
extern void glsl_macro_dump(Macro *macro);
extern void glsl_macrolist_dump(MacroList *list);
#endif

#endif