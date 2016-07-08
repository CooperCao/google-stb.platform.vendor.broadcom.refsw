/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/prepro/glsl_prepro_macro.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_directive.h"
#include "middleware/khronos/glsl/glsl_extensions.h"

#include "middleware/khronos/glsl/glsl_errors.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef MACRO_DEBUG
static struct {
   int macro;
   int macrolist;

   int total;
} malloc_info;

void macro_malloc_init()
{
   memset(&malloc_info, 0, sizeof(malloc_info));
}

void macro_malloc_print()
{
   printf("macro        = %d\n", malloc_info.macro);
   printf("macrolist    = %d\n", malloc_info.macrolist);
   printf("total        = %d\n", malloc_info.total);
}
#endif

static Macro *macro_alloc(void)
{
#ifdef MACRO_DEBUG
   malloc_info.macro += sizeof(Macro);
   malloc_info.total += sizeof(Macro);
#endif
   return (Macro *)malloc_fast(sizeof(Macro));
}

Macro *glsl_macro_construct_undef(Token *name)
{
   Macro *macro = macro_alloc();

   macro->type = MACRO_UNDEF;
   macro->name = name;

   return macro;
}

Macro *glsl_macro_construct_object(Token *name, TokenSeq *body)
{
   Macro *macro = macro_alloc();

   macro->type = MACRO_OBJECT;
   macro->name = name;
   macro->args = NULL;
   macro->body = body;

   return macro;
}

Macro *glsl_macro_construct_function(Token *name, TokenList *args, TokenSeq *body)
{
   Macro *macro = macro_alloc();

   macro->type = MACRO_FUNCTION;
   macro->name = name;
   macro->args = args;
   macro->body = body;

   return macro;
}

static Macro *glsl_macro_construct_builtin(MacroType type, char *s)
{
   Macro *macro = macro_alloc();

   TokenData data;

   data.s = s;

   macro->type = type;
   macro->name = glsl_token_construct(IDENTIFIER, data);
   macro->args = NULL;
   macro->body = NULL;

   return macro;
}

Macro *glsl_macro_construct_line()
{
   return glsl_macro_construct_builtin(MACRO_LINE, "__LINE__");
}

Macro *glsl_macro_construct_file()
{
   return glsl_macro_construct_builtin(MACRO_FILE, "__FILE__");
}

bool glsl_macro_equals(Macro *m1, Macro *m2)
{
   vcos_assert(m1);
   vcos_assert(m2);

   return m1->type == m2->type &&
          glsl_token_equals(m1->name, m2->name, false) &&
          glsl_tokenlist_equals(m1->args, m2->args) &&
          glsl_tokenseq_equals(m1->body, m2->body);
}

bool glsl_macro_is_builtin(Macro *macro)
{
   if (macro->type == MACRO_LINE ||
      macro->type == MACRO_FILE)
      return true;

   if (macro->name->type == IDENTIFIER &&
      strlen(macro->name->data.s) >= 3 &&
      !strncmp(macro->name->data.s, "GL_", 3))
      return true;

   return false;
}

static MacroList *macrolist_alloc(void)
{
#ifdef MACRO_DEBUG
   malloc_info.macrolist += sizeof(MacroList);
   malloc_info.total += sizeof(MacroList);
#endif
   return (MacroList *)malloc_fast(sizeof(MacroList));
}

MacroList *glsl_macrolist_construct(Macro *macro, MacroList *next)
{
   MacroList *list = macrolist_alloc();

   list->macro = macro;
   list->next = next;

   return list;
}

MacroList *glsl_macrolist_construct_initial()
{
    MacroList *list = NULL;
    const char *ext;
    int i = 0;

    list = glsl_macrolist_construct(glsl_macro_construct_line(), list);
    list = glsl_macrolist_construct(glsl_macro_construct_file(), list);
    list = glsl_macrolist_construct(glsl_macro_construct_object(glsl_token_construct_identifier("__VERSION__"), glsl_tokenseq_construct(glsl_token_construct_ppnumberi(100), NULL, NULL)), list);
    list = glsl_macrolist_construct(glsl_macro_construct_object(glsl_token_construct_identifier("GL_ES"), glsl_tokenseq_construct(glsl_token_construct_ppnumberi(1), NULL, NULL)), list);
    list = glsl_macrolist_construct(glsl_macro_construct_object(glsl_token_construct_identifier("GL_FRAGMENT_PRECISION_HIGH"), glsl_tokenseq_construct(glsl_token_construct_ppnumberi(1), NULL, NULL)), list);

    /* GLSL ES 3.4-- predefine macros for supported extensions */
    while ((ext = glsl_ext_get_identifier(&i)) != NULL) {
        Token *id = glsl_token_construct_identifier(ext);
        Token *value = glsl_token_construct_ppnumberi(1);
        TokenSeq *body = glsl_tokenseq_construct(value, NULL, NULL);

        Macro *m = glsl_macro_construct_object(id, body);
        list = glsl_macrolist_construct(m, list);
    }

    return list;
}

Macro *glsl_macrolist_find(MacroList *list, Token *name)
{
   while (list) {
      if (glsl_token_equals(list->macro->name, name, false))
      {
         if (list->macro->type == MACRO_UNDEF)
            return NULL;
         else
            return list->macro;
      }

      list = list->next;
   }

   return NULL;
}

bool glsl_is_defined(MacroList *list, Token *name)
{
   return glsl_macrolist_find(list, name) != NULL;
}

TokenSeq *glsl_remove_defined(TokenSeq *seq)
{
   TokenSeq *res = NULL;

   while (seq) {
      if (seq->token->type == IDENTIFIER && !strcmp(seq->token->data.s, "defined")) {
         bool has_lparen;

         seq = seq->next;

         if (seq && seq->token->type == LEFT_PAREN) {
            seq = seq->next;
            has_lparen = true;
         } else
            has_lparen = false;

         if (seq && is_pp_identifier(seq->token)) {
            res = glsl_tokenseq_construct(glsl_token_construct_ppnumberi(glsl_is_defined(directive_macros, seq->token) ? 1 : 0), NULL, res);
            seq = seq->next;
         } else
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected identifier in constant expression");

         if (has_lparen)
         {
            if (seq && seq->token->type == RIGHT_PAREN)
               seq = seq->next;
            else
               glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "mismatched parenthesis in constant expression");
         }
      } else {
         res = glsl_tokenseq_construct(seq->token, NULL, res);
         seq = seq->next;
      }
   }

   return glsl_tokenseq_destructive_reverse(res, NULL);
}

#ifdef MACRO_DEBUG
void glsl_macro_dump(Macro *macro)
{
   glsl_token_dump(macro->name);

   if (macro->type == MACRO_FUNCTION) {
      printf("(");

      glsl_tokenlist_dump(macro->args, ", ");

      printf(")");
   }

   printf(" -> ");

   glsl_tokenseq_dump(macro->body, " ");

   printf("\n");
}

void glsl_macrolist_dump(MacroList *list)
{
   while (list) {
      glsl_macro_dump(list->macro);

      list++;
   }
}
#endif
