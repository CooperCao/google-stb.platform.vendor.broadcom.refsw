/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_globals.h"
#include "glsl_errors.h"

#include "prepro/glsl_prepro_eval.h"
#include "prepro/glsl_prepro_directive.h"
#include "prepro/glsl_prepro_macro.h"
#include "prepro/glsl_prepro_expand.h"

#include <string.h>
#include <stdlib.h>

/*
   simple recursive-descent parser for preprocessor constant expressions

   0 (highest) parenthetical grouping     ( )                  NA
   1 unary                                defined + - ~ !      Right to Left
   2 multiplicative                       * / %                Left to Right
   3 additive                             + -                  Left to Right
   4 bit-wise shift                       << >>                Left to Right
   5 relational                           < > <= >=            Left to Right
   6 equality                             == !=                Left to Right
   7 bit-wise and                         &                    Left to Right
   8 bit-wise exclusive or                ^                    Left to Right
   9 bit-wise inclusive or                |                    Left to Right
   10 logical and                         &&                   Left to Right
   11 (lowest) logical inclusive or       ||                   Left to Right
*/

static TokenSeq *skip_white(TokenSeq *s)
{
   while(s && s->token->type==WHITESPACE)
      s = s->next;
   return s;
}

static TokenSeq *seq_next(TokenSeq *s)
{
   s = s->next;
   return skip_white(s);
}

static void check_seq(TokenSeq *s)
{
   if (!s)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected end of line in constant expression");
}

static TokenSeq *expect_rparen(TokenSeq *s)
{
   s = skip_white(s);
   check_seq(s);

   if (s->token->type == RIGHT_PAREN)
      s = seq_next(s);
   else
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "mismatched parenthesis in constant expression");

   return s;
}

static TokenSeq *expect_comma(TokenSeq *s)
{
   s = skip_white(s);
   check_seq(s);

   if (s->token->type == COMMA)
      s = seq_next(s);
   else
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected comma");

   return s;
}

// eval==false is used for C-style short-circuiting: In this case undefined identifiers are allowed (if not needed due to short-circuiting), but we must do the full recursion to check syntax.
static int eval_11(TokenSeq *s, TokenSeq **rem, bool eval);

static void parse_to_rparen(TokenSeq *s, TokenSeq **rem)
{
   s = seq_next(s); // consume LEFT_PAREN

   while(true)
   {
      check_seq(s);
      eval_11(s, &s, false);

      if(s->token->type == RIGHT_PAREN)
      {
         *rem = seq_next(s); // consume RIGHT_PAREN
         return;
      }
      s = expect_comma(s);
   }
}

static int eval_1(TokenSeq *s, TokenSeq **rem, bool eval)
{
   check_seq(s);

   switch (s->token->type) {
   case PPNUMBER:
   {
      s->token = glsl_lex_ppnumber(s->token);
      return eval_1(s, rem, eval);
   }
   case INTCONSTANT:
   case UINTCONSTANT:
   {
      int res = (int)s->token->data.v;
      *rem = seq_next(s);
      return res;
   }
   case BOOLCONSTANT:
   case FLOATCONSTANT:
      if(eval)
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "invalid integer constant");
      return 0;

   case IDENTIFIER:
   {
      if(eval)
      {
         s = glsl_expand(glsl_remove_defined(s), false);
         s = skip_white(s);
         check_seq(s);

         if(s->token->type==IDENTIFIER && !glsl_macrolist_find(directive_macros, s->token))
         {
            glsl_compile_error(ERROR_PREPROCESSOR, 4, g_LineNumber, NULL);
            return 0;
         }
         else {
            int res = eval_11(s, rem, eval);
            return res;
         }
      }
      else
      {
         // Skip identifier or macro.
         s = s->next; // consume identifier,
                      // no whitespace is allowed between macro name and parameter list

         if(s && s->token->type == LEFT_PAREN) {
            parse_to_rparen(s, rem); // NOTE: Due to short-circuiting we accept things like "#if 1 || AAA()".
         } else *rem = s;

         return 0;
      }
   }
   case PLUS:
      return +eval_1(seq_next(s), rem, eval);
   case DASH:
      return -eval_1(seq_next(s), rem, eval);
   case TILDE:
      return ~eval_1(seq_next(s), rem, eval);
   case BANG:
      return !eval_1(seq_next(s), rem, eval);
   case LEFT_PAREN:
   {
      int res = eval_11(seq_next(s), &s, eval);
      *rem = expect_rparen(s);

      return res;
   }
   default:
      if(eval)
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected token in constant expression");
      return 0;
   }
}

static bool is_prec_2(TokenType t) {
   return t == STAR || t == SLASH || t == PERCENT;
}

static int eval_2(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_1(s, &s, eval);

   while (s && is_prec_2(s->token->type)) {
      TokenType op = s->token->type;
      int rhs = eval_1(seq_next(s), &s, eval);

      if (eval) {
         if (op == STAR) res *= rhs;
         else {
            if (rhs == 0)
               glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "divide by zero in constant expression");

            if (op == SLASH)   res /= rhs;
            if (op == PERCENT) res %= rhs;
         }
      }
   }

   *rem = s;
   return res;
}

static bool is_prec_3(TokenType t) {
   return t == PLUS || t == DASH;
}

static int eval_3(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_2(s, &s, eval);

   while (s && is_prec_3(s->token->type)) {
      TokenType op = s->token->type;
      int rhs = eval_2(seq_next(s), &s, eval);

      if (op == PLUS) res += rhs;
      if (op == DASH) res -= rhs;
   }

   *rem = s;
   return res;
}

static bool is_prec_4(TokenType t) {
   return t == LEFT_OP || t == RIGHT_OP;
}

static int eval_4(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_3(s, &s, eval);

   while (s && is_prec_4(s->token->type)) {
      TokenType op = s->token->type;
      int rhs = eval_3(seq_next(s), &s, eval);

      switch (op) {
      case LEFT_OP:
         if (rhs < 32)
            res <<= rhs;
         else
            res = 0;
         break;
      case RIGHT_OP:
         if (rhs < 32)
            res >>= rhs;
         else
            if (res < 0)
               res = -1;
            else
               res = 0;
         break;
      default:
         unreachable();
      }
   }

   *rem = s;
   return res;
}

static bool is_prec_5(TokenType t) {
   return t == LEFT_ANGLE || t == RIGHT_ANGLE || t == LE_OP || t == GE_OP;
}

static int eval_5(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_4(s, &s, eval);

   while (s && is_prec_5(s->token->type)) {
      TokenType op = s->token->type;
      int rhs = eval_4(seq_next(s), &s, eval);

      switch (op) {
      case LEFT_ANGLE:  res = res <  rhs; break;
      case RIGHT_ANGLE: res = res >  rhs; break;
      case LE_OP:       res = res <= rhs; break;
      case GE_OP:       res = res >= rhs; break;
      default: unreachable(); break;
      }
   }

   *rem = s;
   return res;
}

static bool is_prec_6(TokenType t) {
   return t == EQ_OP || t == NE_OP;
}

static int eval_6(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_5(s, &s, eval);

   while (s && is_prec_6(s->token->type)) {
      TokenType op = s->token->type;
      int rhs = eval_5(seq_next(s), &s, eval);
      switch (op) {
      case EQ_OP: res = res == rhs; break;
      case NE_OP: res = res != rhs; break;
      default: unreachable();       break;
      }
   }

   *rem = s;
   return res;
}

static int eval_7(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_6(s, &s, eval);

   while (s && s->token->type == AMPERSAND) {
      res &= eval_6(seq_next(s), &s, eval);
   }

   *rem = s;
   return res;
}

static int eval_8(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_7(s, &s, eval);

   while (s && s->token->type == CARET) {
      res ^= eval_7(seq_next(s), &s, eval);
   }

   *rem = s;
   return res;
}

static int eval_9(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_8(s, &s, eval);

   while (s && s->token->type == VERTICAL_BAR) {
      res |= eval_8(seq_next(s), &s, eval);
   }

   *rem = s;
   return res;
}

static int eval_10(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_9(s, &s, eval);

   while (s && s->token->type == AND_OP) {
      // C-style short-circuiting:
      int rhs = eval_9(seq_next(s), &s, eval && res);
      res = res ? rhs : 0;
   }

   *rem = s;
   return res;
}

static int eval_11(TokenSeq *s, TokenSeq **rem, bool eval)
{
   int res = eval_10(s, &s, eval);

   while (s && s->token->type == OR_OP) {
      // C-style short-circuiting:
      int rhs = eval_10(seq_next(s), &s, eval && !res);
      res = res ? 1 : rhs ;
   }

   *rem = s;
   return res;
}

int glsl_eval_evaluate(TokenSeq *s, TokenSeq **rem) {
   int res = eval_11(skip_white(s), rem, true);
   return res;
}
