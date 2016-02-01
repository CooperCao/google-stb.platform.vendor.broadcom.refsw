/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_globals.h"
#include "prepro/glsl_prepro_token.h"
#include "glsl_extensions.h"
#include "glsl_numbers.h"
#include "helpers/gfx/gfx_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

#ifdef TOKEN_DEBUG
static struct {
   int token;
   int tokenlist;
   int tokenseq;
   int tokenseqlist;

   int total;
} malloc_info;

void glsl_token_malloc_init()
{
   memset(&malloc_info, 0, sizeof(malloc_info));
}

void glsl_token_malloc_print()
{
   printf("token        = %d\n", malloc_info.token);
   printf("tokenlist    = %d\n", malloc_info.tokenlist);
   printf("tokenseq     = %d\n", malloc_info.tokenseq);
   printf("tokenseqlist = %d\n", malloc_info.tokenseqlist);
   printf("total        = %d\n", malloc_info.total);
}
#endif

static Token *token_alloc(void)   /* clean */
{
#ifdef TOKEN_DEBUG
   malloc_info.token += sizeof(Token);
   malloc_info.total += sizeof(Token);
#endif
   return (Token *)malloc_fast(sizeof(Token));
}

Token *glsl_token_construct(TokenType type, TokenData data)    // clean
{
   Token *token = token_alloc();

   token->type = type;

   if (KEYWORDS_BEGIN < type && type < KEYWORDS_END) {
      token->data = data;
   } else {
      switch (type) {
      case IDENTIFIER:
      case INTRINSIC:
      case PPNUMBER:
      case INTCONSTANT:
      case UINTCONSTANT:
      case BOOLCONSTANT:
      case FLOATCONSTANT:
         token->data = data;
         break;
      default:
         token->data.s = NULL;
      }
   }

   return token;
}

Token *glsl_token_construct_identifier(const char *s) {
   TokenData data = { .s = s };
   return glsl_token_construct(IDENTIFIER, data);
}

Token *glsl_token_construct_intconst(int i) {
   TokenData data = { .v = i};
   return glsl_token_construct(INTCONSTANT, data);
}

Token *glsl_lex_ppnumber(Token *token)
{
   assert(token->type == PPNUMBER);

   uint32_t value;
   int number_type = numlex(token->data.s, &value);
   if (number_type == NUM_INVALID)
      glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "invalid numeric constant \"%s\"", token->data.s);

   switch(number_type) {
      case NUM_INT:   token->type = INTCONSTANT; break;
      case NUM_UINT:  token->type = UINTCONSTANT; break;
      case NUM_FLOAT: token->type = FLOATCONSTANT; break;
      default: UNREACHABLE();
   }
   token->data.v = value;

   return token;
}

bool glsl_token_equals(Token *t1, Token *t2)   // clean
{
   if (t1->type != t2->type)
      return false;

   switch (t1->type) {
   case PPNUMBER:
   case IDENTIFIER:
   case INTRINSIC:
      return !strcmp(t1->data.s, t2->data.s);
   case INTCONSTANT:
   case UINTCONSTANT:
   case BOOLCONSTANT:
   case FLOATCONSTANT:
      return t1->data.v == t2->data.v;
   default:
      return true;
   }
}

static TokenList *glsl_tokenlist_alloc(void)    /* clean */
{
#ifdef TOKEN_DEBUG
   malloc_info.tokenlist += sizeof(TokenList);
   malloc_info.total += sizeof(TokenList);
#endif
   return (TokenList *)malloc_fast(sizeof(TokenList));
}

TokenList *glsl_tokenlist_construct(Token *token, TokenList *next)  // clean
{
   TokenList *ts = glsl_tokenlist_alloc();

   ts->token = token;
   ts->next = next;

   return ts;
}

TokenList *glsl_tokenlist_intersect(TokenList *hs0, TokenList *hs1)
{
   TokenList *hs = NULL;

   while (hs0) {
      if (glsl_tokenlist_contains(hs1, hs0->token))
         hs = glsl_tokenlist_construct(hs0->token, hs);

      hs0 = hs0->next;
   }

   return hs;
}

TokenList *glsl_tokenlist_union(TokenList *hs0, TokenList *hs1)
{
   TokenList *hs = hs1;

   while (hs0) {
      if (!glsl_tokenlist_contains(hs1, hs0->token))
         hs = glsl_tokenlist_construct(hs0->token, hs);

      hs0 = hs0->next;
   }

   return hs;
}

bool glsl_tokenlist_equals(TokenList *t1, TokenList *t2)
{
   while (t1) {
      if (!t2 || !glsl_token_equals(t1->token, t2->token))
         return false;

      t1 = t1->next;
      t2 = t2->next;
   }

   return t2 == NULL;
}

bool glsl_tokenlist_contains(TokenList *hs, Token *t)
{
   while (hs) {
      if (glsl_token_equals(hs->token, t))
         return true;

      hs = hs->next;
   }

   return false;
}

int glsl_tokenlist_length(TokenList *hl)
{
   int len = 0;

   while (hl) {
      len++;

      hl = hl->next;
   }

   return len;
}

static TokenSeq *glsl_tokenseq_alloc(void)
{
#ifdef TOKEN_DEBUG
   malloc_info.tokenseq += sizeof(TokenSeq);
   malloc_info.total += sizeof(TokenSeq);
#endif
   return (TokenSeq *)malloc_fast(sizeof(TokenSeq));
}

TokenSeq *glsl_tokenseq_construct(Token *token, TokenList *hide, TokenSeq *next)
{
   TokenSeq *ts = glsl_tokenseq_alloc();

   ts->token = token;
   ts->hide = hide;
   ts->next = next;

   return ts;
}

bool glsl_tokenseq_equals(TokenSeq *t1, TokenSeq *t2)
{
   while (t1) {
      if (!t2 || !glsl_token_equals(t1->token, t2->token))
         return false;

      t1 = t1->next;
      t2 = t2->next;
   }

   return t2 == NULL;
}

TokenSeq *glsl_tokenseq_destructive_reverse(TokenSeq *t, TokenSeq *p)
{
   while (t) {
      TokenSeq *n = t->next;
      t->next = p;
      p = t;
      t = n;
   }

   return p;
}

static TokenSeqList *tokenseqlist_alloc(void)
{
#ifdef TOKEN_DEBUG
   malloc_info.tokenseqlist += sizeof(TokenSeqList);
   malloc_info.total += sizeof(TokenSeqList);
#endif
   return (TokenSeqList *)malloc_fast(sizeof(TokenSeqList));
}

TokenSeqList *glsl_tokenseqlist_construct(TokenSeq *seq, TokenSeqList *next)
{
   TokenSeqList *tss = tokenseqlist_alloc();

   tss->seq = seq;
   tss->next = next;

   return tss;
}

#ifdef TOKEN_DEBUG
void glsl_token_dump(Token *token)
{
   switch (token->type) {
   case IDENTIFIER:
      printf("%s", token->data.s);
      break;
   case INTRINSIC:
      printf("$$%s", token->data.s);
      break;
   case INTCONSTANT:
   case UINTCONSTANT:
   case BOOLCONSTANT:
      printf("%d", token->data.v);
      break;
   case FLOATCONSTANT:
      printf("%f", gfx_float_from_bits(token->data.v));
      break;
   default:
      /* Note: If you want plaintext token names, enable %token-table in glsl_parser.y: */
      printf("token->type %d\n", token->type);
      break;
   }
}

void glsl_tokenlist_dump(TokenList *list, const char *sep)
{
   while (list) {
      glsl_token_dump(list->token);

      if (list->next)
         printf("%s", sep);

      list = list->next;
   }
}

void glsl_tokenseq_dump(TokenSeq *seq, const char *sep)
{
   while (seq) {
      glsl_token_dump(seq->token);

      if (seq->next)
         printf("%s", sep);

      seq = seq->next;
   }
}
#endif
