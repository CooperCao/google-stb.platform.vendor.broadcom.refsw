/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_COMMON_H
#define GLSL_COMMON_H

#include "../common/khrn_int_common.h"

// Enumerations.

typedef enum _QualFlavour
{
   QUAL_STORAGE,
   QUAL_AUXILIARY,
   QUAL_MEMORY,
   QUAL_LAYOUT,
   QUAL_PREC,
   QUAL_INTERP,
   QUAL_INVARIANT,
   QUAL_PRECISE,
   QUAL_FLAVOUR_LAST = QUAL_PRECISE
} QualFlavour;

typedef enum _StorageQualifier
{
   STORAGE_NONE,     /* TODO: Not sure if this should really exist */
   STORAGE_CONST,
   STORAGE_IN,
   STORAGE_OUT,
   STORAGE_INOUT,    /* TODO: Not sure if this should really exist */
   STORAGE_UNIFORM,
   STORAGE_BUFFER,
   STORAGE_SHARED
} StorageQualifier;

typedef enum _AuxiliaryQualifier
{
   AUXILIARY_CENTROID,
   AUXILIARY_PATCH,
   AUXILIARY_SAMPLE
} AuxiliaryQualifier;

typedef enum _MemoryQualifier
{
   MEMORY_NONE      = 0,
   MEMORY_COHERENT  = (1<<0),
   MEMORY_VOLATILE  = (1<<1),
   MEMORY_RESTRICT  = (1<<2),
   MEMORY_READONLY  = (1<<3),
   MEMORY_WRITEONLY = (1<<4)
} MemoryQualifier;

#define LAYOUT_SHARED       (1 << 0)
#define LAYOUT_PACKED       (1 << 1)
#define LAYOUT_STD140       (1 << 2)
#define LAYOUT_STD430       (1 << 3)
#define LAYOUT_ROW_MAJOR    (1 << 4)
#define LAYOUT_COLUMN_MAJOR (1 << 5)

typedef enum _Qualified
{
   NONE_QUALED    = 0,
   LOC_QUALED     = (1<<0),
   UNIF_QUALED    = (1<<1),
   BINDING_QUALED = (1<<2),
   FORMAT_QUALED  = (1<<3),
   OFFSET_QUALED  = (1<<4)
} Qualified;

typedef enum
{
   FMT_RGBA32F,
   FMT_RGBA16F,
   FMT_R32F,
   FMT_RGBA8,
   FMT_RGBA8_SNORM,
   FMT_RGBA32I,
   FMT_RGBA16I,
   FMT_RGBA8I,
   FMT_R32I,
   FMT_RGBA32UI,
   FMT_RGBA16UI,
   FMT_RGBA8UI,
   FMT_R32UI,
} FormatQualifier;

typedef struct _LayoutQualifier
{
   uint32_t qualified;

   int location;
   int binding;
   FormatQualifier format;
   int offset;
   uint32_t unif_bits;
} LayoutQualifier;

typedef enum _PrecisionQualifier {
   PREC_UNKNOWN,
   PREC_NONE,
   PREC_LOWP,
   PREC_MEDIUMP,
   PREC_HIGHP,
   PREC_MAXP
} PrecisionQualifier;

typedef enum _InterpolationQualifier
{
   INTERP_SMOOTH,
   INTERP_FLAT
} InterpolationQualifier;

typedef struct {
   QualFlavour flavour;

   union {
      StorageQualifier       storage;
      AuxiliaryQualifier     auxiliary;
      MemoryQualifier        memory;
      struct layout_id_list *layout;   /* Pointer to the list of identifiers */
      PrecisionQualifier     precision;
      InterpolationQualifier interp;
   } u;
} Qualifier;

typedef struct _QualListNode {
   Qualifier *q;
   struct _QualListNode *next;
} QualListNode;

typedef struct {
   QualListNode *head;
   QualListNode *tail;
} QualList;

/* A composite qualifier type that the lists get compressed down to */
/* TODO: Remove. This is an anachronism */
typedef enum _TypeQualifier
{
   TYPE_QUAL_NONE,
   TYPE_QUAL_CENTROID,
   TYPE_QUAL_FLAT,
   TYPE_QUAL_PATCH,
   TYPE_QUAL_SAMPLE
} TypeQualifier;

typedef enum _ParamQualifier
{
   PARAM_QUAL_IN,
   PARAM_QUAL_OUT,
   PARAM_QUAL_INOUT
} ParamQualifier;

typedef enum _ExprFlavour
{
   EXPR_VALUE,
   EXPR_INSTANCE,
   EXPR_SUBSCRIPT,
   EXPR_FUNCTION_CALL,
   EXPR_PRIM_CONSTRUCTOR_CALL,
   EXPR_COMPOUND_CONSTRUCTOR_CALL,
   EXPR_FIELD_SELECTOR,
   EXPR_SWIZZLE,
   EXPR_POST_INC,
   EXPR_POST_DEC,
   EXPR_PRE_INC,
   EXPR_PRE_DEC,
   EXPR_ARITH_NEGATE,
   EXPR_LOGICAL_NOT,
   EXPR_BITWISE_NOT,
   EXPR_MUL,
   EXPR_DIV,
   EXPR_REM,
   EXPR_ADD,
   EXPR_SUB,
   EXPR_LESS_THAN,
   EXPR_LESS_THAN_EQUAL,
   EXPR_GREATER_THAN,
   EXPR_GREATER_THAN_EQUAL,
   EXPR_EQUAL,
   EXPR_NOT_EQUAL,
   EXPR_LOGICAL_AND,
   EXPR_LOGICAL_XOR,
   EXPR_LOGICAL_OR,
   EXPR_CONDITIONAL,
   EXPR_ASSIGN,
   EXPR_BITWISE_AND,
   EXPR_BITWISE_XOR,
   EXPR_BITWISE_OR,
   EXPR_SHL,
   EXPR_SHR,
   EXPR_SEQUENCE,

   EXPR_ARRAY_LENGTH,

   // Language extensions.
   EXPR_INTRINSIC,

   EXPR_FLAVOUR_COUNT
} ExprFlavour;

typedef enum _StatementFlavour
{
   STATEMENT_AST,
   STATEMENT_DECL_LIST,
   STATEMENT_FUNCTION_DEF,
   STATEMENT_VAR_DECL,
   STATEMENT_STRUCT_DECL,
   STATEMENT_STRUCT_MEMBER_DECL,
   STATEMENT_COMPOUND,
   STATEMENT_EXPR,
   STATEMENT_SELECTION,
   STATEMENT_SWITCH,
   STATEMENT_ITERATOR_FOR,
   STATEMENT_ITERATOR_WHILE,
   STATEMENT_ITERATOR_DO_WHILE,
   STATEMENT_CONTINUE,
   STATEMENT_BREAK,
   STATEMENT_CASE,
   STATEMENT_DEFAULT,
   STATEMENT_DISCARD,
   STATEMENT_RETURN,
   STATEMENT_RETURN_EXPR,
   STATEMENT_PRECISION,
   STATEMENT_QUALIFIER_DEFAULT,
   STATEMENT_QUALIFIER_AUGMENT,
   STATEMENT_BARRIER,
   STATEMENT_NULL,
   STATEMENT_FLAVOUR_COUNT
} StatementFlavour;

//
// Forward declarations for symbols.h, ast.h, dataflow.h, const_types.h.
//

// Typedefs.
struct _Symbol;
typedef struct _Symbol Symbol;
struct _SymbolType;
typedef struct _SymbolType SymbolType;

struct _MEMBER_LAYOUT_T;
typedef struct _MEMBER_LAYOUT_T MEMBER_LAYOUT_T;

struct _Expr;
typedef struct _Expr Expr;
struct _Statement;
typedef struct _Statement Statement;

struct _CallContext;
typedef struct _CallContext CallContext;

struct _ExprChain;
typedef struct _ExprChain ExprChain;
struct _ExprChainNode;
typedef struct _ExprChainNode ExprChainNode;

struct _StatementChain;
typedef struct _StatementChain StatementChain;
struct _StatementChainNode;
typedef struct _StatementChainNode StatementChainNode;

typedef struct _Qualifiers
{
   bool invariant;
   StorageQualifier   sq;
   TypeQualifier      tq;
   LayoutQualifier   *lq;
   PrecisionQualifier pq;
   MemoryQualifier    mq;
} Qualifiers;


// Functions.

static inline bool glsl_is_row_major(LayoutQualifier *lq)
{
   if(!lq)
      return false;

   if((lq->qualified & UNIF_QUALED) && (lq->unif_bits & LAYOUT_ROW_MAJOR))
      return true;

   return false;
}

static inline bool glsl_layouts_equal(LayoutQualifier *lq1, LayoutQualifier *lq2)
{
   if(lq1 == lq2)
      return true;

   if(lq1->qualified != lq2->qualified)
      return false;

   if( (lq1->qualified & LOC_QUALED) && lq1->location!=lq2->location )
      return false;

   if( (lq1->qualified & UNIF_QUALED) && lq1->unif_bits!=lq2->unif_bits )
      return false;

   if( (lq1->qualified & BINDING_QUALED) && lq1->binding != lq2->binding )
      return false;

   if( (lq1->qualified & OFFSET_QUALED) && lq1->offset != lq2->offset  )
      return false;

   if( (lq1->qualified & FORMAT_QUALED) && lq1->format != lq2->format )
      return false;

   return true;
}

bool glsl_parsing_user_code(void);

#endif // COMMON_H
