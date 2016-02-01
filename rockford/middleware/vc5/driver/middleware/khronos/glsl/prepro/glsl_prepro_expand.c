/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  prepro
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_globals.h"

#include "prepro/glsl_prepro_expand.h"
#include "prepro/glsl_prepro_directive.h"

#include "glsl_errors.h"

#include <string.h>
#include <stdlib.h>

/*
   implementation of Dave Prosser's reference code for
   C-style macro expansion

   expand(TS)                 // recur, substitute, pushback, rescan
   {
      if TS is {} then
         return {};
      else if TS is T(HS) * TS' and T is in HS then
         return T(HS) * expand(TS');
      else if TS is T(HS) * TS' and T is a object-like macro then
         return expand(subst(ts(T),{},{},HS union {T},{}) * TS');
      else if TS is T(HS) * ( * TS' and T is a function-like macro then
         check TS' is actuals * )(HS') * TS'' and actuals are "correct for T"
         return expand(subst(ts(T),fp(T),actuals,(HS intersect HS') union {T},{}) * TS'');

      note TS must be T(HS) * TS'
      return T(HS) * expand(TS');
   }

   subst(IS, FP, AP, HS, OS)     // substitute args
   {
      if IS is {} then
         return hsadd(HS, OS);
      else if IS is T * IS' and T is FP[i] then
         return subst(IS',FP,AP,HS,OS * expand(select(i,AP)));

      note IS must be T(HS') * IS'
      return subst(IS',FP,AP,HS,OS * T(HS'));
   }

   hsadd(HS, TS)                 // add to token sequence's hide sets
   {
      if TS is {} then
         return {};
      note TS must be T(HS') * TS'
         return T(HS union HS') * hsadd(HS,TS');
   }

   The remaining support functions are:

   ts(T)          Given a macro-name token, ts returns the replacement token sequence from the macro's definition.

   fp(T)          Given a macro-name token, fp returns the (ordered) list of formal parameters from the macro's definition.

   select(i,TS)   Given a token sequence and an index i, select returns the i-th token sequence using the comma tokens
                  (not between nested parenthesis pairs) in the original token sequence as delimiters.
*/

/*
   hsadd(HS, TS)                 // add to token sequence's hide sets
   {
      if TS is {} then
         return {};
      note TS must be T(HS') * TS'
         return T(HS union HS') * hsadd(HS,TS');
   }

   checked
*/

static TokenSeq *hsadd(TokenList *hs, TokenSeq *ts)
{
   TokenSeq *s;

   for (s = ts; s; s = s->next)
      s->hide = glsl_tokenlist_union(s->hide, hs);

   return ts;
}

/*
   subst(IS, FP, AP, HS, OS)     // substitute args
   {
      if IS is {} then
         return hsadd(HS, OS);
      else if IS is T * IS' and T is FP[i] then
         return subst(IS',FP,AP,HS,OS * expand(select(i,AP)));

      note IS must be T(HS') * IS'
      return subst(IS',FP,AP,HS,OS * T(HS'));
   }

   checked
*/

static TokenSeq *subst(TokenSeq *is, TokenList *fp, TokenSeqList *ap, TokenList *hs, TokenSeq *os)
{
   TokenList *formal;
   TokenSeqList *actual;

   bool found;

   while (true) {
      if (is == NULL)
         return hsadd(hs, os);

      for (formal = fp, actual = ap, found = false;
            formal && actual && !found;
            formal = formal->next, actual = actual->next) {

         if (glsl_token_equals(is->token, formal->token)) {
            os = glsl_tokenseq_destructive_reverse(glsl_expand(actual->seq, true), os);
            is = is->next;

            found = true;
         }
      }

      if (!found) {
         /* The two lists should have been the same length */
         assert(formal == NULL && actual == NULL);

         os = glsl_tokenseq_construct(is->token, is->hide, os);
         is = is->next;
      }
   }
}

/*
   expand(TS)                 // recur, substitute, pushback, rescan
   {
      if TS is {} then
         return {};
      else if TS is T(HS) * TS' and T is in HS then
         return T(HS) * expand(TS');
      else if TS is T(HS) * TS' and T is a object-like macro then
         return expand(subst(ts(T),{},{},HS union {T},{}) * TS');
      else if TS is T(HS) * ( * TS' and T is a function-like macro then
         check TS' is actuals * )(HS') * TS'' and actuals are "correct for T"
         return expand(subst(ts(T),fp(T),actuals,(HS intersect HS') union {T},{}) * TS'');

      note TS must be T(HS) * TS'
      return T(HS) * expand(TS');
   }

   checked, applied manual tail recursion elimination
*/

TokenSeq *glsl_expand(TokenSeq *ts, bool recursive)
{
   TokenSeq *res = NULL;

   while (ts || !res) {
      if (!recursive && ts == NULL)
         ts = glsl_directive_next_token();

      if (ts == NULL)
         break;

      if (!glsl_tokenlist_contains(ts->hide, ts->token)) {
         Macro *m = glsl_macrolist_find(directive_macros, ts->token);

         if (m != NULL) {
            switch (m->type) {
            case MACRO_OBJECT:
               ts = glsl_tokenseq_destructive_reverse(subst(m->body, NULL, NULL, glsl_tokenlist_construct(ts->token, ts->hide), NULL), ts->next);
               continue;   // effective tail call
            case MACRO_FUNCTION:
               if (!recursive && ts->next == NULL)
                  ts->next = glsl_directive_next_token();

               if (ts->next && ts->next->token->type == WHITESPACE)
                  ts->next = ts->next->next;

               if (!recursive && ts->next == NULL)
                  ts->next = glsl_directive_next_token();

               if (ts->next && is_lparen(ts->next->token)) {
                  int formal_count = glsl_tokenlist_length(m->args);

                  int depth = 0;
                  int count = 0;

                  /* gather up actual parameters */

                  TokenSeq *tail = ts->next->next;
                  TokenSeq *curr = NULL;

                  TokenSeqList *actuals = NULL;

                  while (true) {
                     if (!recursive && tail == NULL)
                        tail = glsl_directive_next_token();

                     if (tail == NULL) {
                        glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "mismatched parenthesis in macro invocation");
                        break;
                     }

                     if (is_lparen(tail->token))
                        depth++;
                     if (is_rparen(tail->token))
                     {
                        if (depth == 0)
                           break;
                        else
                           depth--;
                     }
                     if (is_comma(tail->token))
                        if (depth == 0) {
                           actuals = glsl_tokenseqlist_construct(glsl_tokenseq_destructive_reverse(curr, NULL), actuals);
                           count++;

                           tail = tail->next;
                           curr = NULL;

                           continue;
                        }

                     curr = glsl_tokenseq_construct(tail->token, tail->hide, curr);

                     tail = tail->next;
                  }
                  if (tail == NULL) break;

                  if (count > 0 || formal_count == 1 || curr != NULL) {
                     actuals = glsl_tokenseqlist_construct(glsl_tokenseq_destructive_reverse(curr, NULL), actuals);
                     count++;
                  }

                  /* check against arity of macro */
                  if (count == formal_count) {
                     ts = glsl_tokenseq_destructive_reverse(subst(m->body, m->args, actuals, glsl_tokenlist_construct(ts->token, glsl_tokenlist_intersect(ts->hide, tail->hide)), NULL), tail->next);
                     continue;   // effective tail call
                  } else
                     glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "arity mismatch in macro invocation");
               }
               break;
            case MACRO_LINE:
               ts = glsl_tokenseq_construct(glsl_token_construct_intconst(g_LineNumber), NULL, ts->next);
               break;
            case MACRO_FILE:
               ts = glsl_tokenseq_construct(glsl_token_construct_intconst(g_FileNumber), NULL, ts->next);
               break;
            case MACRO_VERSION:
            {
               int version = GLSL_SHADER_VERSION_NUMBER(g_ShaderVersion);
               ts = glsl_tokenseq_construct(glsl_token_construct_intconst(version), NULL, ts->next);
               break;
            }
            default:
               UNREACHABLE();
               break;
            }
         }
      }

      res = glsl_tokenseq_construct(ts->token, ts->hide, res);
      ts = ts->next;
   }

   return glsl_tokenseq_destructive_reverse(res, NULL);
}
