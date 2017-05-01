/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"
#include "glsl_globals.h"

#include "prepro/glsl_prepro_eval.h"
#include "prepro/glsl_prepro_expand.h"
#include "prepro/glsl_prepro_directive.h"

#include "glsl_extensions.h"
#include "glsl_errors.h"
#include "glsl_symbol_table.h"

#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#define strcasecmp _stricmp
#endif

extern int pplex(TokenData *tok_data);

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

void glsl_init_preprocessor(int version)
{
   directive_macros = glsl_macrolist_construct_initial(version);

   allow_directive = true;
   allow_extension = true;
   allow_version = true;
   depth = 0;
}

void glsl_directive_reset_macros()
{
   directive_macros = NULL;
}

static void push(bool active)
{
   if (depth == MAX_DEPTH) {
      /* implementation-specific error */
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "excessive nesting of #if directives");
   }

   if_stack[depth].active = active;
   if_stack[depth].sticky = active;
   if_stack[depth].seen_else = false;

   depth++;
}

static void pop(void)
{
   if (depth == 0)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected #endif");

   depth--;
}

static bool is_active(int delta)
{
   for (int i = 0; i < depth - delta; i++)
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

static TokenType get(bool skip, TokenData *tok_data) {
   TokenType t;
   do {
      t = (TokenType)pplex(tok_data);
   } while (t == WHITESPACE && skip);
   return t;
}

static TokenType get_type(bool skip) {
   TokenData tok_data;
   return get(skip, &tok_data);
}

static Token *get_token(bool skip)
{
   TokenData tok_data;
   TokenType t = get(skip, &tok_data);
   return glsl_token_construct(t, tok_data);
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
      TokenType type;

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

   // skip initial whitespace:
   for ( token = get_token(true); !is_newline(token); token = get_token(skip)) {
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
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "reserved macro name: %s", s);
}

static void parse_define(void)
{
   Token *name = get_identifier();

   Macro *prev = glsl_macrolist_find(directive_macros, name);
   Macro *curr = NULL;

   if (name->type == IDENTIFIER) { check_valid_name(name->data.s); }

   Token *first = get_token(false);
   switch (first->type) {
   case WHITESPACE:
      curr = glsl_macro_construct_object(name, get_remainder(false)); // presence of whitespace matters, hence don't skip it
      break;
   case NEWLINE:
      curr = glsl_macro_construct_object(name, NULL);
      break;
   case LEFT_PAREN:
   {
      TokenList *args = get_identifier_list();
      TokenSeq *body = get_remainder(false); // presence of whitespace matters, hence don't skip it

      curr = glsl_macro_construct_function(name, args, body);
      break;
   }
   default:
      {
         TokenSeq *rem = get_remainder(false);
         rem = glsl_tokenseq_construct(first, NULL, rem);
         curr = glsl_macro_construct_object(name, rem);
         break;
      }
   }

   if (!prev || glsl_macro_equals(prev, curr))
      directive_macros = glsl_macrolist_construct(curr, directive_macros);
   else
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "inconsistent macro redefinition");
}

/* # undef identifier new-line */
static void parse_undef(void)
{
   Token *name = get_identifier();

   /* Check that this doesn't undef a builtin */
   Macro *prev = glsl_macrolist_find(directive_macros, name);
   if (prev != NULL && glsl_macro_is_builtin(prev))
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "Cannot undef builtin: %s", name->data.s);

   directive_macros = glsl_macrolist_construct(glsl_macro_construct_undef(name), directive_macros);

   check_no_remainder();
}

/* # if constant-expression new-line */
static void parse_if(void)
{
   if (is_active(0)) {
      TokenSeq *seq = get_remainder(true);
      TokenSeq *rem;

      g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

      push(glsl_eval_evaluate(glsl_expand(glsl_remove_defined(seq), true), &rem));

      if (rem)
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

      g_LineNumber++;
   } else {
      skip_remainder();

      push(false);
   }
}

/* # ifdef identifier new-line */
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

/* # ifndef identifier new-line */
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

/* # else new-line */
static void parse_else(void)
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

/* # elif constant-expression new-line */
static void parse_elif(void)
{
   if (is_active(1)) {
      TokenSeq *seq = get_remainder(true);

      if (!allow_else())
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected #elif");

      if (has_sticky())
         set_active(false);
      else {
         TokenSeq *rem;
         g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

         set_active(glsl_eval_evaluate(glsl_expand(glsl_remove_defined(seq), true), &rem));

         if (rem)
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

         g_LineNumber++;
      }
   } else
      skip_remainder();
}

/* # endif new-line */
static void parse_endif(void)
{
   pop();

   if (is_active(0))
      check_no_remainder();
   else
      skip_remainder();
}

static void parse_pragma(void)
{
   /* This implementation defines no pragmas, so ignore them all */
   skip_remainder();
}

static void expect_colon(void)
{
   TokenType type = get_type(true);

   if (type != COLON)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected colon");
}

static void parse_extension(void)
{
   if (!allow_extension)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "Invalid extension directive");

   Token *ext_tok = get_token(true);
   expect_colon();
   TokenType directive = get_type(true);
   check_no_remainder();

   if (directive != REQUIRE && directive != ENABLE && directive != WARN && directive != DISABLE)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected require, enable, disable or warn");

   enum glsl_ext ext = GLSL_EXT_NOT_SUPPORTED;
   switch (ext_tok->type) {
   case IDENTIFIER:
      ext = glsl_ext_lookup(ext_tok->data.s);
      break;
   case ALL:
      ext = GLSL_EXT_ALL;
      break;
   default:
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected identifier");
   }

   if (ext == GLSL_EXT_NOT_SUPPORTED) {
      if (directive == REQUIRE)
         glsl_compile_error(ERROR_PREPROCESSOR, 3, g_LineNumber, "Required extension %s not supported", ext_tok->data.s);
      else
         glsl_compile_error(WARNING, 1, g_LineNumber, "%s", ext_tok->data.s);
      return;
   }

   switch (directive) {
   case REQUIRE:
   case ENABLE:
      if (ext == GLSL_EXT_ALL) {
         glsl_compile_error(ERROR_PREPROCESSOR, 3, g_LineNumber, "'all' cannot be enabled or required");
         break;
      }
      glsl_ext_enable(ext, false);
      break;
    case WARN:
      glsl_ext_enable(ext, true);
      break;
    case DISABLE:
      glsl_ext_disable(ext);
      break;
    default:
      unreachable();
      break;
    }
}

static void parse_line(void)
{
   TokenSeq *seq = get_remainder(true);
   TokenSeq *rem;

   g_LineNumber--;         // compensate for the fact that we've now fetched the line feed

   int line = glsl_eval_evaluate(glsl_expand(glsl_remove_defined(seq), true), &rem);

   if (rem)
      g_FileNumber = glsl_eval_evaluate(rem, &rem);

   if (rem)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "garbage at end of preprocessor directive");

   g_LineNumber = line;
}

static bool is_conditional_directive(TokenType t) {
   return t == IF || t == IFDEF || t == IFNDEF || t == ELSE || t == ELIF || t == ENDIF;
}

static void parse_conditional_directive(TokenType t) {
   switch (t) {
   case IF:     parse_if();     return;
   case IFDEF:  parse_ifdef();  return;
   case IFNDEF: parse_ifndef(); return;
   case ELSE:   parse_else();   return;
   case ELIF:   parse_elif();   return;
   case ENDIF:  parse_endif();  return;
   default:
      unreachable();
      break;
   }
}

static void parse_directive()
{
   TokenType t = get_type(true);

   if (is_conditional_directive(t)) {
      parse_conditional_directive(t);
      return;
   }

   if (!is_active(0)) {
      skip_remainder();
      return;
   }

   switch (t) {
   case ERROR:
      glsl_compile_error(ERROR_PREPROCESSOR, 2, g_LineNumber, NULL);
      break;
   case DEFINE:
      parse_define();
      break;
   case UNDEF:
      parse_undef();
      break;
   case PRAGMA:
      parse_pragma();
      break;
   case EXTENSION:
      parse_extension();
      break;
   case VERSION:
      /* If declaring here is valid then we've already checked it, otherwise it's an error */
      if (!allow_version)
         glsl_compile_error(ERROR_PREPROCESSOR, 5, g_LineNumber, NULL);
      skip_remainder();
      break;
   case LINE:
      parse_line();
      break;
   case NEWLINE:
      break;
   default:
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "invalid preprocessor directive");
      break;
   }
}

Token *glsl_directive_next_token(void)
{
   while (true) {
      TokenData tok_data;
      TokenType type = get(true, &tok_data);

      if (type == 0) {
         if (depth)
            glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected end of file");

         return NULL;
      }

      // Eagerly expand __LINE__ to line number in effort to get accurate answer.
      // If __LINE__ is part of a macro, this code path will not be hit, and we will get the line number during macro expansion.
      if (type == IDENTIFIER && strcmp(tok_data.s, "__LINE__") == 0)
      {
         type = INTCONSTANT;
         tok_data.v = g_LineNumber;
      }

      if (allow_directive && type == HASH) {
         parse_directive();
         allow_version = false;
      } else {
         if (type == NEWLINE)
            allow_directive = true;
         else {
            allow_directive = false;
            allow_version   = false;

            /* We only disallow extensions when returning active tokens. It's debatable
             * whether inactive ones should count as 'non-preprocessor tokens' */
            if (is_active(0)) {
               allow_extension = false;
               return glsl_token_construct(type, tok_data);
            }
         }
      }
   }
}
