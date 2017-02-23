/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/prepro/glsl_prepro_eval.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_directive.h"
#include "middleware/khronos/glsl/prepro/glsl_prepro_expand.h"

#include "middleware/khronos/glsl/glsl_errors.h"
#include "middleware/khronos/glsl/glsl_stack.h"

#include <string.h>
#include <stdlib.h>

extern int yyfile;
extern int yyline;

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

static TokenSeq *seq;

static void skip_white(void)
{
   while(seq && seq->token->type==WHITESPACE)
      seq = seq->next;
}

static void seq_next(void)
{
   seq = seq->next;
   skip_white();
}

static void check_seq(void)
{
   if (!seq)
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected end of line in constant expression");
}

static void expect_rparen(void)
{
   skip_white();
   check_seq();

   if (seq->token->type == RIGHT_PAREN)
      seq_next();
   else
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "mismatched parenthesis in constant expression");
}

static void expect_comma(void)
{
   skip_white();
   check_seq();

   if (seq->token->type == COMMA)
      seq_next();
   else
      glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "expected comma");
}

// eval==false is used for C-style short-circuiting: In this case undefined identifiers are allowed (if not needed due to short-circuiting), but we must do the full recursion to check syntax.
static int eval_1(bool eval);
static int eval_2(bool eval);
static int eval_3(bool eval);
static int eval_4(bool eval);
static int eval_5(bool eval);
static int eval_6(bool eval);
static int eval_7(bool eval);
static int eval_8(bool eval);
static int eval_9(bool eval);
static int eval_10(bool eval);
static int eval_11(bool eval);

static void parse_to_rparen(void)
{
   seq_next(); // consume LEFT_PAREN

   while(true)
   {
      check_seq();
      eval_11(false);

      if(seq->token->type == RIGHT_PAREN)
      {
         seq_next(); // consume RIGHT_PAREN
         return;
      }
      expect_comma();
   }
}

static int eval_1(bool eval)
{
   check_seq();

   switch (seq->token->type) {
   case IDENTIFIER:
   {
      if(eval)
      {
         seq = glsl_expand(glsl_remove_defined(seq), false);
         skip_white();
         check_seq();

         if(seq->token->type==IDENTIFIER && !glsl_macrolist_find(directive_macros, seq->token))
         {
            glsl_compile_error(ERROR_PREPROCESSOR, 7, g_LineNumber, NULL);
            return 0;
         }
         else
            return eval_11(eval);
      }
      else
      {
         // Skip identifier or macro.
         seq = seq->next; // consume identifier,
                          // no whitespace is allowed between macro name and parameter list

         if(seq && seq->token->type == LEFT_PAREN)
            parse_to_rparen(); // NOTE: Due to short-circuiting we accept things like "#if 1 || AAA()".

         return 0;
      }
   }
   case PPNUMBERI:
   {
      int res = (int)seq->token->data.i;

      seq_next();

      return res;
   }
   case PPNUMBERF:
   case PPNUMBERU:
      if(eval)
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "invalid integer constant");
      return 0;
   case PLUS:
      seq_next();

      return +eval_1(eval);
   case DASH:
      seq_next();

      return -eval_1(eval);
   case TILDE:
      seq_next();

      return ~eval_1(eval);
   case BANG:
      seq_next();

      return !eval_1(eval);
   case LEFT_PAREN:
   {
      int res;

      seq_next();

      res = eval_11(eval);

      expect_rparen();

      return res;
   }
   default:
      if(eval)
         glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "unexpected token in constant expression");
      return 0;
   }
}

static int eval_2(bool eval)
{
   int res = eval_1(eval);

   while (seq)
      switch (seq->token->type) {
      case STAR:
         seq_next();

         res *= eval_1(eval);
         break;
      case SLASH:
      {
         int rhs;

         seq_next();

         rhs = eval_1(eval);

         if (eval)
         {
            if (rhs)
               res /= rhs;
            else
               glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "divide by zero in constant expression");
         }
         break;
      }
      case PERCENT:
      {
         int rhs;

         seq_next();

         rhs = eval_1(eval);

         if (eval)
         {
            if (rhs)
               res %= rhs;
            else
               glsl_compile_error(ERROR_PREPROCESSOR, 1, g_LineNumber, "divide by zero in constant expression");
         }
         break;
      }
      default:
         return res;
      }

   return res;
}

static int eval_3(bool eval)
{
   int res = eval_2(eval);

   while (seq)
      switch (seq->token->type) {
      case PLUS:
         seq_next();

         res += eval_2(eval);
         break;
      case DASH:
         seq_next();

         res -= eval_2(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_4(bool eval)
{
   int res = eval_3(eval);
   int rhs;

   while (seq)
      switch (seq->token->type) {
      case LEFT_OP:
         seq_next();

         rhs = eval_3(eval);

         if (rhs < 32)
            res <<= rhs;
         else
            res = 0;
         break;
      case RIGHT_OP:
         seq_next();

         rhs = eval_3(eval);

         if (rhs < 32)
            res >>= rhs;
         else
            if (res < 0)
               res = -1;
            else
               res = 0;
         break;
      default:
         return res;
      }

   return res;
}

static int eval_5(bool eval)
{
   int res = eval_4(eval);

   while (seq)
      switch (seq->token->type) {
      case LEFT_ANGLE:
         seq_next();

         res = res < eval_4(eval);
         break;
      case RIGHT_ANGLE:
         seq_next();

         res = res > eval_4(eval);
         break;
      case LE_OP:
         seq_next();

         res = res <= eval_4(eval);
         break;
      case GE_OP:
         seq_next();

         res = res >= eval_4(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_6(bool eval)
{
   int res = eval_5(eval);

   while (seq)
      switch (seq->token->type) {
      case EQ_OP:
         seq_next();

         res = res == eval_5(eval);
         break;
      case NE_OP:
         seq_next();

         res = res != eval_5(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_7(bool eval)
{
   int res = eval_6(eval);

   while (seq)
      switch (seq->token->type) {
      case BITWISE_AND_OP:
         seq_next();

         res &= eval_6(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_8(bool eval)
{
   int res = eval_7(eval);

   while (seq)
      switch (seq->token->type) {
      case BITWISE_XOR_OP:
         seq_next();

         res ^= eval_7(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_9(bool eval)
{
   int res = eval_8(eval);

   while (seq)
      switch (seq->token->type) {
      case BITWISE_OR_OP:
         seq_next();

         res |= eval_8(eval);
         break;
      default:
         return res;
      }

   return res;
}

static int eval_10(bool eval)
{
   int res = eval_9(eval);

   while (seq)
      switch (seq->token->type) {
      case LOGICAL_AND_OP:
      {
         int rhs;

         seq_next();
         // C-style short-circuiting:
         rhs = eval_9(res ? eval : false);
         res = res ? rhs : 0 ;
         break;
      }
      default:
         return res;
      }

   return res;
}

static int eval_11(bool eval)
{
   int res = eval_10(eval);

   while (seq)
      switch (seq->token->type) {
      case LOGICAL_OR_OP:
      {
         int rhs;

         seq_next();
         // C-style short-circuiting:
         rhs = eval_10(res ? false : eval);
         res = res ? 1 : rhs ;
         break;
      }
      default:
         return res;
      }

   return res;
}

void glsl_eval_set_sequence(TokenSeq *_seq)
{
   seq = _seq;
}

bool glsl_eval_has_sequence(void)
{
   return seq != NULL;
}

int glsl_eval_evaluate(void)
{
   skip_white();
   return eval_11(true);
}
