/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_PREPRO_TOKEN_H
#define GLSL_PREPRO_TOKEN_H

#include "middleware/khronos/glsl/glsl_const_types.h"
#include "middleware/khronos/glsl/glsl_symbols.h"
#include "middleware/khronos/glsl/y.tab.h"

#ifndef NDEBUG
#define TOKEN_DEBUG
#endif

typedef enum yytokentype V3DTokenType;

typedef struct _TokenData {
   const char *s;
   union {
      const_int i;
      const_float f;
   };
} TokenData;

typedef struct _Token {
   V3DTokenType type;
   TokenData data;
} Token;

extern Token *glsl_token_construct(V3DTokenType type, TokenData data);
extern Token *glsl_token_construct_identifier(const char *s);
extern Token *glsl_token_construct_ppnumberi(int i);

extern bool glsl_token_equals(Token *t1, Token *t2, bool subcmp);

static INLINE bool is_pp_identifier(Token *t)
{
   return t->type == IDENTIFIER || (t->type >= DEFINE && t->type <= _FALSE);
}

static INLINE bool is_lparen(Token *t)
{
   return t->type == LEFT_PAREN;
}

static INLINE bool is_rparen(Token *t)
{
   return t->type == RIGHT_PAREN;
}

static INLINE bool is_comma(Token *t)
{
   return t->type == COMMA;
}

static INLINE bool is_newline(Token *t)
{
   return t->type == NEWLINE;
}

/*
   sets of tokens implemented as linked lists

   used to implement 'hide sets' for recursion prevention
*/

typedef struct _TokenList {
   Token *token;

   struct _TokenList *next;
} TokenList;

extern TokenList *glsl_tokenlist_construct(Token *token, TokenList *next);
extern TokenList *glsl_tokenlist_intersect(TokenList *hs0, TokenList *hs1);
extern TokenList *glsl_tokenlist_union(TokenList *hs0, TokenList *hs1);

extern bool glsl_tokenlist_equals(TokenList *t1, TokenList *t2);

extern bool glsl_tokenlist_contains(TokenList *hs, Token *t);

extern int glsl_tokenlist_length(TokenList *hs);

/*
   sequences of tokens with associated hide sets implemented
   as linked lists
*/

typedef struct _TokenSeq {
   Token *token;

   struct _TokenList *hide;
   struct _TokenSeq *next;
} TokenSeq;

extern TokenSeq *glsl_tokenseq_construct(Token *token, TokenList *hide, TokenSeq *next);

extern bool glsl_tokenseq_equals(TokenSeq *t1, TokenSeq *t2);

extern TokenSeq *glsl_tokenseq_destructive_reverse(TokenSeq *t, TokenSeq *p);

typedef struct _TokenSeqList {
   TokenSeq *seq;

   struct _TokenSeqList *next;
} TokenSeqList;

extern TokenSeqList *glsl_tokenseqlist_construct(TokenSeq *seq, TokenSeqList *next);

#ifdef TOKEN_DEBUG
extern void glsl_token_malloc_init(void);
extern void glsl_token_malloc_print(void);

extern void glsl_token_dump(Token *token);
extern void glsl_tokenlist_dump(TokenList *list, char *sep);
extern void glsl_tokenseq_dump(TokenSeq *seq, char *sep);
#endif

#endif
