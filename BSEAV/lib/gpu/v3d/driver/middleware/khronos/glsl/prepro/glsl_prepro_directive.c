/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/prepro/glsl_prepro_eval.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_expand.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_directive.h"

#include "middleware/khronos/glsl/glsl_extensions.h"
#include "middleware/khronos/glsl/glsl_errors.h"

#include <string.h>
#include <stdio.h>

//#define DEBUG_PREPRO

extern TokenData pptoken;

extern int pplex(void);

MacroList *directive_macros;

static bool allow_directive;
static bool allow_extension;
static bool allow_version;

#define MAX_DEPTH 32

static struct {
   bool active;
   bool sticky;
   bool seen_else;
} if_stack[MAX_DEPTH];

static int depth;

void glsl_init_preprocessor(void)
{
   directive_macros = glsl_macrolist_construct_initial();

   allow_directive = true;
   allow_extension = false;
   allow_version = false;
   depth = 0;
}

void glsl_directive_reset_macros(void)
{
   directive_macros = NULL;
}

void glsl_directive_allow_version(void)
{
   allow_version = true;
}

void glsl_directive_allow_extension(void)
{
   allow_extension = true;
}

static void push(bool active)
{
   if (depth == MAX_DEPTH) {
      /*
         implementation-specific error
      */

      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "excessive nesting of #if directives");
   }

   if_stack[depth].active = active;
   if_stack[depth].sticky = active;
   if_stack[depth].seen_else = false;

#ifdef DEBUG_PREPRO
   printf("push: depth = %d, active = %s\n", depth, active ? "true" : "false");
#endif

   depth++;
}

static void pop(void)
{
   if (depth == 0)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected #endif");

   depth--;

#ifdef DEBUG_PREPRO
   printf("pop: depth = %d, active = %s, sticky = %s, seen_else = %s\n", depth, if_stack[depth].active ? "true" : "false", if_stack[depth].sticky ? "true" : "false", if_stack[depth].seen_else ? "true" : "false");
#endif
}

static bool is_active(int delta)
{
   int i;

   for (i = 0; i < depth - delta; i++)
      if (!if_stack[i].active)
         return false;

   return true;
}

static bool allow_else(void)
{
   return depth > 0 && !if_stack[depth - 1].seen_else;
}

static bool has_sticky(void)
{
   return if_stack[depth - 1].sticky;
}

static void seen_else(void)
{
   assert(depth > 0);

   if_stack[depth - 1].seen_else = true;
}

static void set_active(bool active)
{
   bool sticky = if_stack[depth - 1].sticky;

   assert(depth > 0);

   if_stack[depth - 1].active = !sticky && active;
   if_stack[depth - 1].sticky = sticky || active;
}

static V3DTokenType get_type(bool skip)
{
   V3DTokenType t;
   do {
      t = (V3DTokenType)pplex();
   } while (skip && t == WHITESPACE);
   return t;
}

static Token *get_token(bool skip)
{
   V3DTokenType t = get_type(skip);          // TODO: end of file handling

   return glsl_token_construct(t, pptoken);
}

static Token *get_identifier(void)
{
   Token *token = get_token(true);

   if (!is_pp_identifier(token))
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected identifier");

   return token;
}

static TokenList *get_identifier_list(void)
{
   TokenList *list = NULL;

   Token *token = get_token(true);

   if (!is_rparen(token)) {
      V3DTokenType type;

      if (!is_pp_identifier(token))
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected identifier");

      list = glsl_tokenlist_construct(token, list);

      for (type = get_type(true); type != RIGHT_PAREN; type = get_type(true)) {
         if (type != COMMA)
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected comma");

         token = get_token(true);

         if (!is_pp_identifier(token))
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected identifier");

         if (glsl_tokenlist_contains(list, token))
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "duplicate formal parameter in macro definition");

         list = glsl_tokenlist_construct(token, list);
      }
   }

   return list;
}

static TokenSeq *get_remainder(bool skip)
{
   TokenSeq *seq = NULL;
   Token *token;

   for (token = get_token(true); !is_newline(token); token = get_token(skip)) {
      // merge consequtive whitespaces into single whitespace token
      if (seq != NULL && seq->token->type == WHITESPACE && token->type == WHITESPACE)
         continue;
      seq = glsl_tokenseq_construct(token, NULL, seq);
   }

   seq = glsl_tokenseq_destructive_reverse(seq, NULL);

   return seq;
}

static void skip_remainder(void)
{
   while (get_type(true) != NEWLINE);           // TODO: end of file handling
}

static void check_no_remainder(void)
{
   if (get_type(true) != NEWLINE)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");
}

/*
   # define identifier replacement-list new-line
   # define identifier ( identifier-listopt ) replacement-list new-line
   # define identifier ( lparen ... ) replacement-list new-line
   # define identifier ( lparen identifier-list , ... ) replacement-list new-line

   function-like macro definitions may not have whitespace between the name and left parenthesis
*/

static void check_valid_name(const char *s)
{
   if (strlen(s) >= 3 && !strncmp(s, "GL_", 3))
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "reserved macro name");
}

static void parse_define(void)
{
   if (is_active(0)) {
      Token *name = get_identifier();

      Macro *prev = glsl_macrolist_find(directive_macros, name);
      Macro *curr = NULL;

      if (name->type == IDENTIFIER)
         check_valid_name(name->data.s);

      switch (get_type(false)) {
      case WHITESPACE:
         curr = glsl_macro_construct_object(name, get_remainder(false));
         break;
      case NEWLINE:
         curr = glsl_macro_construct_object(name, NULL);
         break;
      case LEFT_PAREN:
      {
         TokenList *args = get_identifier_list();
         TokenSeq *body = get_remainder(false);

         curr = glsl_macro_construct_function(name, args, body);
         break;
      }
      default:
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "malformed macro definition");
         break;
      }

      if (!prev || glsl_macro_equals(prev, curr))
         directive_macros = glsl_macrolist_construct(curr, directive_macros);
      else
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "inconsistent macro redefinition");
   } else
      skip_remainder();
}

/*
   # undef identifier new-line
*/

static void parse_undef(void)
{
   if (is_active(0)) {
      Token *name = get_identifier();

      /* Check that this doesn't undef a builtin */
      Macro *prev = glsl_macrolist_find(directive_macros, name);
      if (prev != NULL && glsl_macro_is_builtin(prev))
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "Cannot undef builtin: %s", name->data.s);

      directive_macros = glsl_macrolist_construct(glsl_macro_construct_undef(name), directive_macros);

      check_no_remainder();
   } else
      skip_remainder();
}

/*
   # if constant-expression new-line
*/

static void parse_if(void)
{
   if (is_active(0)) {
      TokenSeq *seq = get_remainder(true);

      g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

      glsl_eval_set_sequence(glsl_expand(glsl_remove_defined(seq), true));
      push(glsl_eval_evaluate());

      if (glsl_eval_has_sequence())
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

      g_LineNumber++;
   } else {
      skip_remainder();

      push(false);
   }
}

/*
   # ifdef identifier new-line
*/

static void parse_ifdef(void)
{
   if (is_active(0)) {
      push(glsl_is_defined(directive_macros, get_identifier()));

      check_no_remainder();
   } else {
      push(false);

      skip_remainder();
   }
}

/*
   # ifndef identifier new-line
*/

static void parse_ifndef(void)
{
   if (is_active(0)) {
      push(!glsl_is_defined(directive_macros, get_identifier()));

      check_no_remainder();
   } else {
      push(false);

      skip_remainder();
   }
}

/*
   # else new-line
*/

static void parse_helse(void)
{
   if (is_active(1)) {
      if (!allow_else())
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected #else");

      seen_else();

      if (has_sticky()) {
         set_active(false);
      } else {
         set_active(true);
      }

      check_no_remainder();
   } else
      skip_remainder();
}

/*
   # elif constant-expression new-line
*/

static void parse_helif(void)
{
   if (is_active(1)) {
      TokenSeq *seq = get_remainder(true);

      if (!allow_else())
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected #elif");

      if (has_sticky())
         set_active(false);
      else {
         g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

         glsl_eval_set_sequence(glsl_expand(glsl_remove_defined(seq), true));
         set_active(glsl_eval_evaluate());

         if (glsl_eval_has_sequence())
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

         g_LineNumber++;
      }
   } else
      skip_remainder();
}

/*
   # endif new-line
*/

static void parse_endif(void)
{
   pop();

   if (is_active(0))
      check_no_remainder();
   else
      skip_remainder();
}

static void parse_error(void)
{
   if (is_active(0)) {
      get_remainder(true);

      glsl_compile_error(ERROR_PREPROCESSOR, 2, g_LineNumber - 1, NULL);
   } else
      skip_remainder();
}

static void parse_pragma(void)
{
   skip_remainder();
}

static void expect_colon(void)
{
   V3DTokenType type = get_type(true);

   if (type != COLON)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected colon");
}

extern void glsl_include(const char *s);

static void parse_extension(void)
{
    Token *tok;
    enum glsl_ext ext = GLSL_EXT_NOT_SUPPORTED;
    const char *contents;
    bool enabled = false;

    if (!is_active(0)) {
        skip_remainder();
        return;
    }

    if (!allow_extension)
       glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "Invalid extension enable");

    tok = get_token(true);

    switch (tok->type) {
    case IDENTIFIER:
        ext = glsl_ext_lookup(tok->data.s);
        break;
    case ALL:
        ext = GLSL_EXT_ALL;
        break;
    default:
        glsl_compile_error(ERROR_PREPROCESSOR,
                1, g_LineNumber, "expected identifier");
    }

    expect_colon();

    switch (get_type(true)) {
    case REQUIRE:
        if (!glsl_ext_enable(ext, false)) {
            glsl_compile_error(ERROR_PREPROCESSOR, 3, g_LineNumber, NULL);
            break;
        }
        enabled = true;
        break;
    case ENABLE:
        if (ext == GLSL_EXT_ALL) {
            glsl_compile_error(ERROR_PREPROCESSOR, 3, g_LineNumber, NULL);
            break;
        }
        else if (!glsl_ext_enable(ext, false))
            glsl_compile_error(WARNING, 1, g_LineNumber, NULL);
        enabled = true;
        break;
    case WARN:
        if (ext == GLSL_EXT_ALL) {
            glsl_ext_enable(ext, true);
            glsl_compile_error(WARNING, 1, g_LineNumber, NULL);
        }
        else if (!glsl_ext_enable(ext, true))
            glsl_compile_error(WARNING, 1, g_LineNumber, NULL);
        enabled = true;
        break;
    case DISABLE:
        if (!glsl_ext_disable(ext))
            glsl_compile_error(WARNING, 1, g_LineNumber, NULL);
        break;
    default:
        glsl_compile_error(ERROR_PREPROCESSOR,
                1, g_LineNumber, "expected require, enable, disable or warn");
        break;
    }

    check_no_remainder();

    if (enabled) {
        contents = glsl_ext_contents(ext);
        if (contents) glsl_include(contents);
    }
}

static void parse_version(bool allow)
{
   if (is_active(0)) {
      Token *token = get_token(true);

      if (!allow)
         glsl_compile_error(ERROR_PREPROCESSOR, 5, g_LineNumber, NULL);

      if (token->type != PPNUMBERI || token->data.i != 100)
         glsl_compile_error(ERROR_CUSTOM, 28, g_LineNumber, NULL);

      check_no_remainder();
   } else
      skip_remainder();
}

static void parse_line(void)
{
   if (is_active(0)) {
      int line;

      TokenSeq *seq = get_remainder(true);

      g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

      glsl_eval_set_sequence(glsl_expand(glsl_remove_defined(seq), true));

      line = glsl_eval_evaluate();

      if (glsl_eval_has_sequence())
         g_FileNumber = glsl_eval_evaluate();

      if (glsl_eval_has_sequence())
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

#ifdef USE_SPECCED_BUT_STUPID_HASH_LINE
      g_LineNumber = line + 1;
#else
      g_LineNumber = line;
#endif
   } else
      skip_remainder();
}

Token *glsl_directive_next_token(void)
{
   while (true) {
      V3DTokenType type = get_type(true);

      if (type == 0) {
         if (depth)
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected end of file");

         return NULL;
      }

      // Eagerly expand __LINE__ to line number in effort to get accurate answer.
      // If __LINE__ is part of a macro, this code path will not be hit, and we will get the line number during macro expansion.
      if (type == IDENTIFIER && strcmp(pptoken.s, "__LINE__") == 0)
      {
         type = PPNUMBERI;
         pptoken.i = g_LineNumber;
      }

      if (allow_directive && type == HASH) {
         switch (get_type(true)) {                 // TODO: end of file handling
         case DEFINE:
            parse_define();
            break;
         case UNDEF:
            parse_undef();
            break;
         case IF:
            parse_if();
            break;
         case IFDEF:
            parse_ifdef();
            break;
         case IFNDEF:
            parse_ifndef();
            break;
         case ELSE:
            parse_helse();
            break;
         case ELIF:
            parse_helif();
            break;
         case ENDIF:
            parse_endif();
            break;
         case _ERROR:
            parse_error();
            break;
         case PRAGMA:
            parse_pragma();
            break;
         case EXTENSION:
            parse_extension();
            break;
         case VERSION:
            parse_version(allow_version);
            break;
         case LINE:
            parse_line();
            break;
         case NEWLINE:
            break;
         default:
            if (is_active(0))
               glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "invalid preprocessor directive");
            break;
         }

         allow_version = false;
      } else {
         if (type == NEWLINE)
            allow_directive = true;
         else {
            allow_directive = false;
            allow_extension = false;
            allow_version = false;

            if (is_active(0))
               return glsl_token_construct(type, pptoken);
         }
      }
   }
}
