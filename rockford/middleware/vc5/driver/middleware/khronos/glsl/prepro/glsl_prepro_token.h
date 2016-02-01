/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PREPRO_TOKEN_H
#define GLSL_PREPRO_TOKEN_H

#include "glsl_const_types.h"
#include "glsl_symbols.h"
#include "glsl_layout.h"   /* Should be included in parser.h. Stupid bison */
#include "glsl_parser.h"

#ifndef NDEBUG
#define TOKEN_DEBUG
#endif

typedef enum yytokentype TokenType;

typedef enum _KeywordFlags {
   PREPROC_KEYWORD      = (1 << 0),
   GLSL_ES_1_KEYWORD    = (1 << 1),
   GLSL_ES_1_RESERVED   = (1 << 2),
   GLSL_ES_3_KEYWORD    = (1 << 3),
   GLSL_ES_3_RESERVED   = (1 << 4),
   GLSL_ES_31_KEYWORD   = (1 << 5),
   GLSL_ES_31_RESERVED  = (1 << 6),
   GLSL_ES_32_KEYWORD   = (1 << 7),
   GLSL_ES_32_RESERVED  = (1 << 8)
} KeywordFlags;

typedef union _TokenData {
   const char *s;
   const_value v;
   KeywordFlags flags;
} TokenData;

typedef struct _Token {
   TokenType type;
   TokenData data;
} Token;

extern Token *glsl_token_construct(TokenType type, TokenData data);
extern Token *glsl_token_construct_identifier(const char *s);
extern Token *glsl_token_construct_intconst(int i);
extern Token *glsl_lex_ppnumber(Token *token);
extern bool glsl_token_equals(Token *t1, Token *t2);

static inline bool is_pp_identifier(Token *t)
{
   // all identifiers and keywords can be used as macro names (including 'define' itself for example)
   return (t->type == IDENTIFIER) || (KEYWORDS_BEGIN < t->type && t->type < KEYWORDS_END) || (t->type == BOOLCONSTANT);
}

static inline bool is_lparen(Token *t)
{
   return t->type == LEFT_PAREN;
}

static inline bool is_rparen(Token *t)
{
   return t->type == RIGHT_PAREN;
}

static inline bool is_comma(Token *t)
{
   return t->type == COMMA;
}

static inline bool is_newline(Token *t)
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
extern void glsl_tokenlist_dump(TokenList *list, const char *sep);
extern void glsl_tokenseq_dump(TokenSeq *seq, const char *sep);
#endif

#endif
