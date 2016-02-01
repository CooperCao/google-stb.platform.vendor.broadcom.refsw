/* A Bison parser, made by GNU Bison 2.4.2.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

      Copyright (C) 1984, 1989-1990, 2000-2006, 2009-2010 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

#ifdef WIN32
#define snprintf sprintf_s
#endif

/* Copy the first part of user declarations.  */


   #include "middleware/khronos/glsl/glsl_common.h"

   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>

   #include "middleware/khronos/glsl/glsl_symbols.h"
   #include "middleware/khronos/glsl/glsl_ast.h"
   #include "middleware/khronos/glsl/glsl_dataflow.h"
   #include "middleware/khronos/glsl/glsl_trace.h"
   #include "middleware/khronos/glsl/glsl_errors.h"
   #include "middleware/khronos/glsl/glsl_intern.h"
   #include "middleware/khronos/glsl/glsl_globals.h"
   #include "middleware/khronos/glsl/glsl_builders.h"
   #include "middleware/khronos/glsl/glsl_extensions.h"

   #include "middleware/khronos/glsl/prepro/glsl_prepro_expand.h"
   #include "middleware/khronos/glsl/prepro/glsl_prepro_directive.h"

   extern TokenData pptoken;

   extern void ppunput(int c);
   extern int pplex(void);

   void yyerror(char *);

   static TokenSeq *seq;

   static bool fast;

   int yylex(void)
   {
      V3DTokenType type = 0;
      TokenData data;

      /*
         fast path bypassing preprocessor
      */

      if (fast) {
         bool newline = false;

         do {
            type = (V3DTokenType)pplex();
            data = pptoken;

            if (type == NEWLINE)
               newline = true;
         } while (type == WHITESPACE || type == NEWLINE);

         if (type == IDENTIFIER) {
            /*
               expand built-in macros
            */

            if (!strcmp(data.s, "__LINE__")) {
               type = PPNUMBERI;
               data.i = g_LineNumber;
            } else if (!strcmp(data.s, "__FILE__")) {
               type = PPNUMBERI;
               data.i = g_FileNumber;
            } else if (!strcmp(data.s, "__VERSION__")) {
               type = PPNUMBERI;
               data.i = 100;
            } else if (!strcmp(data.s, "GL_ES")) {
               type = PPNUMBERI;
               data.i = 1;
            } else if (!strcmp(data.s, "GL_FRAGMENT_PRECISION_HIGH")) {
               type = PPNUMBERI;
               data.i = 1;
            }
         }

         if (newline && type == HASH)
            fast = false;
         else
            glsl_directive_disallow_version();
      }

      /*
         slow path via preprocessor
      */

      if (!fast) {
         if (!seq)
            seq = glsl_expand(NULL, false);

         if (seq) {
            type = seq->token->type;
            data = seq->token->data;

            seq = seq->next;
         } else
            return 0;
      }

      /*
         detect uses of reserved keywords (anything from the list, or anything containing '__')
      */

      if ((type >= ASM && type <= USING) || (type == IDENTIFIER && strstr(data.s, "__")))
         glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL);

      if (type == UNKNOWN)
         glsl_compile_error(ERROR_UNKNOWN, 0, g_LineNumber, "unknown character");

      /*
         detokenize tokens which were meaningful to the preprocessor
      */

      if (type >= DEFINE && type <= DISABLE)
         type = IDENTIFIER;

      /*
         the idea here is to return CANDIDATE_TYPE_NAME if it could be a type,
         and otherwise to return IDENTIFIER
      */

      switch (type) {
      case IDENTIFIER:
      {
         Symbol *sym = glsl_symbol_table_lookup(g_SymbolTable, data.s, true);

         yylval.lookup.symbol = sym;
         yylval.lookup.name = data.s;

         if (sym)
            switch (sym->flavour) {
            case SYMBOL_TYPE:
               type = CANDIDATE_TYPE_NAME;
               break;
            case SYMBOL_VAR_INSTANCE:
            case SYMBOL_PARAM_INSTANCE:
            case SYMBOL_FUNCTION_INSTANCE:
               break;
            default:
               /*
                  we shouldn't see anything else in the main symbol table
               */
               UNREACHABLE();
               break;
            }
         break;
      }
      case PPNUMBERI:
         yylval.i = data.i;
         break;
      case PPNUMBERF:
         yylval.f = data.f;
         break;
      case PPNUMBERU:
         yylval.s = data.s;
         break;
      default:
         break;
      }

      return type;
   }

   // This allows yyparse to take an argument of type void* with the given name.
   #define YYPARSE_PARAM top_level_statement

   #define YYMALLOC yymalloc
   #define YYFREE yyfree

   void *yymalloc(size_t bytes)
   {
      return glsl_fastmem_malloc(bytes, true);
   }

   void yyfree(void *ptr)
   {
      UNUSED(ptr);
   }

   void enter_loop(void)
   {
      vcos_assert(g_LoopDepth < INT_MAX);
      g_LoopDepth++;
   }
   static void exit_loop(void)
   {
      vcos_assert(g_LoopDepth > 0);
      g_LoopDepth--;
   }
   static int in_loop(void)
   {
      return (g_LoopDepth > 0);
   }


   void glsl_init_parser(void)
   {
      g_DataflowSources = glsl_dataflow_sources_new();
      g_SymbolTable = glsl_symbol_table_populate(glsl_symbol_table_new());
      g_InGlobalScope = true;

      glsl_sb_invalidate(&g_StringBuilder);

      g_StructBuilderMembers = NULL;
      g_FunctionBuilderParams = NULL;

      g_NextAnonParam = 0;

      seq = NULL;

      fast = true;

      g_LoopDepth = 0;
   }

   static const char *generate_anon_param(void)
   {
      char c[16];

      snprintf(c, sizeof(c), "$$anon%d\n", g_NextAnonParam++);

      return glsl_intern(c, true);
   }



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 512,
     PPNUMBERI = 513,
     PPNUMBERF = 514,
     PPNUMBERU = 515,
     UNKNOWN = 516,
     HASH = 517,
     WHITESPACE = 518,
     NEWLINE = 519,
     LEFT_OP = 520,
     RIGHT_OP = 521,
     INC_OP = 522,
     DEC_OP = 523,
     LE_OP = 524,
     GE_OP = 525,
     EQ_OP = 526,
     NE_OP = 527,
     LOGICAL_AND_OP = 528,
     LOGICAL_OR_OP = 529,
     LOGICAL_XOR_OP = 530,
     MUL_ASSIGN = 531,
     DIV_ASSIGN = 532,
     ADD_ASSIGN = 533,
     MOD_ASSIGN = 534,
     LEFT_ASSIGN = 535,
     RIGHT_ASSIGN = 536,
     AND_ASSIGN = 537,
     XOR_ASSIGN = 538,
     OR_ASSIGN = 539,
     SUB_ASSIGN = 540,
     LEFT_PAREN = 541,
     RIGHT_PAREN = 542,
     LEFT_BRACKET = 543,
     RIGHT_BRACKET = 544,
     LEFT_BRACE = 545,
     RIGHT_BRACE = 546,
     DOT = 547,
     COMMA = 548,
     COLON = 549,
     EQUAL = 550,
     SEMICOLON = 551,
     BANG = 552,
     DASH = 553,
     TILDE = 554,
     PLUS = 555,
     STAR = 556,
     SLASH = 557,
     PERCENT = 558,
     LEFT_ANGLE = 559,
     RIGHT_ANGLE = 560,
     BITWISE_OR_OP = 561,
     BITWISE_XOR_OP = 562,
     BITWISE_AND_OP = 563,
     QUESTION = 564,
     INTRINSIC_TEXTURE_2D_BIAS = 565,
     INTRINSIC_TEXTURE_2D_LOD = 566,
     INTRINSIC_TEXTURE_CUBE_BIAS = 567,
     INTRINSIC_TEXTURE_CUBE_LOD = 568,
     INTRINSIC_RSQRT = 569,
     INTRINSIC_RCP = 570,
     INTRINSIC_LOG2 = 571,
     INTRINSIC_EXP2 = 572,
     INTRINSIC_CEIL = 573,
     INTRINSIC_FLOOR = 574,
     INTRINSIC_SIGN = 575,
     INTRINSIC_TRUNC = 576,
     INTRINSIC_NEAREST = 577,
     INTRINSIC_MIN = 578,
     INTRINSIC_MAX = 579,
     INTRINSIC_MINABS = 580,
     INTRINSIC_MAXABS = 581,
     DEFINE = 582,
     UNDEF = 583,
     IFDEF = 584,
     IFNDEF = 585,
     ELIF = 586,
     ENDIF = 587,
     _ERROR = 588,
     PRAGMA = 589,
     EXTENSION = 590,
     VERSION = 591,
     LINE = 592,
     ALL = 593,
     REQUIRE = 594,
     ENABLE = 595,
     WARN = 596,
     DISABLE = 597,
     ATTRIBUTE = 598,
     _CONST = 599,
     _BOOL = 600,
     _FLOAT = 601,
     _INT = 602,
     BREAK = 603,
     CONTINUE = 604,
     DO = 605,
     ELSE = 606,
     FOR = 607,
     IF = 608,
     DISCARD = 609,
     RETURN = 610,
     BVEC2 = 611,
     BVEC3 = 612,
     BVEC4 = 613,
     IVEC2 = 614,
     IVEC3 = 615,
     IVEC4 = 616,
     VEC2 = 617,
     VEC3 = 618,
     VEC4 = 619,
     _MAT2 = 620,
     MAT3 = 621,
     MAT4 = 622,
     _IN = 623,
     _OUT = 624,
     INOUT = 625,
     UNIFORM = 626,
     VARYING = 627,
     SAMPLER2D = 628,
     SAMPLEREXTERNAL = 629,
     SAMPLERCUBE = 630,
     STRUCT = 631,
     _VOID = 632,
     WHILE = 633,
     INVARIANT = 634,
     HIGH_PRECISION = 635,
     MEDIUM_PRECISION = 636,
     LOW_PRECISION = 637,
     PRECISION = 638,
     ASM = 639,
     CLASS = 640,
     UNION = 641,
     ENUM = 642,
     TYPEDEF = 643,
     TEMPLATE = 644,
     THIS = 645,
     PACKED = 646,
     GOTO = 647,
     SWITCH = 648,
     DEFAULT = 649,
     _INLINE = 650,
     NOINLINE = 651,
     VOLATILE = 652,
     PUBLIC = 653,
     STATIC = 654,
     EXTERN = 655,
     EXTERNAL = 656,
     INTERFACE = 657,
     FLAT = 658,
     _LONG = 659,
     _SHORT = 660,
     DOUBLE = 661,
     HALF = 662,
     _FIXED = 663,
     _UNSIGNED = 664,
     SUPERP = 665,
     _INPUT = 666,
     OUTPUT = 667,
     HVEC2 = 668,
     HVEC3 = 669,
     HVEC4 = 670,
     DVEC2 = 671,
     DVEC3 = 672,
     DVEC4 = 673,
     FVEC2 = 674,
     FVEC3 = 675,
     FVEC4 = 676,
     SAMPLER1D = 677,
     SAMPLER3D = 678,
     SAMPLER1DSHADOW = 679,
     SAMPLER2DSHADOW = 680,
     SAMPLER2DRECT = 681,
     SAMPLER3DRECT = 682,
     SAMPLER2DRECTSHADOW = 683,
     SIZEOF = 684,
     CAST = 685,
     NAMESPACE = 686,
     USING = 687,
     _TRUE = 688,
     _FALSE = 689,
     CANDIDATE_TYPE_NAME = 690
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{


   const_int i; const_float f; const char* s;
   struct { const char* name; Symbol* symbol; } lookup;
   SymbolType* type;
   Symbol* symbol;
   CallContext call_context;
   Expr* expr; ExprFlavour expr_flavour;
   Statement* statement;
   StatementChain* statement_chain;
   struct { Statement* a; Statement* b; } statement2;



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
        && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
    || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)      \
      do               \
   {               \
     YYSIZE_T yyi;            \
     for (yyi = 0; yyi < (Count); yyi++)   \
       (To)[yyi] = (From)[yyi];      \
   }               \
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)            \
    do                           \
      {                           \
   YYSIZE_T yynewbytes;                  \
   YYCOPY (&yyptr->Stack_alloc, Stack, yysize);         \
   Stack = &yyptr->Stack_alloc;               \
   yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
   yyptr += yynewbytes / sizeof (*yyptr);            \
      }                           \
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  72
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1856

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  182
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  86
/* YYNRULES -- Number of rules.  */
#define YYNRULES  250
/* YYNRULES -- Number of states.  */
#define YYNSTATES  388

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   691

#define YYTRANSLATE(YYX)                  \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,   139,   140,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   180,
     181,     2
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    13,    15,    19,
      21,    26,    28,    32,    35,    38,    40,    42,    45,    48,
      51,    53,    56,    60,    63,    65,    67,    69,    71,    73,
      75,    77,    79,    81,    83,    85,    87,    89,    91,    93,
      95,    97,    99,   101,   103,   105,   107,   109,   111,   113,
     115,   117,   119,   121,   123,   125,   127,   129,   131,   133,
     135,   137,   140,   143,   146,   149,   152,   155,   157,   161,
     165,   169,   171,   175,   179,   181,   185,   189,   191,   195,
     199,   203,   207,   209,   213,   217,   219,   223,   225,   229,
     231,   235,   237,   241,   243,   247,   249,   253,   255,   261,
     263,   267,   271,   273,   275,   277,   279,   281,   283,   285,
     287,   289,   291,   293,   297,   299,   302,   305,   310,   313,
     315,   317,   320,   324,   328,   335,   339,   346,   349,   355,
     358,   360,   361,   363,   365,   367,   369,   373,   380,   386,
     388,   391,   397,   402,   405,   407,   410,   412,   414,   416,
     419,   421,   423,   426,   428,   430,   432,   434,   436,   438,
     440,   442,   444,   446,   448,   450,   452,   454,   456,   458,
     460,   462,   464,   466,   468,   470,   472,   474,   475,   482,
     483,   489,   491,   494,   497,   499,   503,   510,   513,   519,
     521,   523,   525,   527,   529,   531,   533,   535,   537,   540,
     541,   546,   547,   550,   551,   554,   556,   558,   561,   565,
     567,   570,   572,   575,   581,   585,   587,   589,   594,   595,
     596,   604,   605,   606,   616,   617,   618,   630,   632,   635,
     637,   639,   644,   646,   647,   649,   650,   652,   653,   656,
     659,   662,   666,   669,   671,   674,   676,   678,   679,   683,
     685
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     263,     0,    -1,     3,    -1,   183,    -1,     5,    -1,     4,
      -1,   179,    -1,   180,    -1,    32,   210,    33,    -1,   184,
      -1,   185,    34,   186,    35,    -1,   187,    -1,   185,    38,
     267,    -1,   185,    13,    -1,   185,    14,    -1,   210,    -1,
     188,    -1,   190,    33,    -1,   189,    33,    -1,   191,   123,
      -1,   191,    -1,   191,   208,    -1,   190,    39,   208,    -1,
     192,    32,    -1,   194,    -1,   193,    -1,     3,    -1,    56,
      -1,    57,    -1,    58,    -1,    59,    -1,    60,    -1,    61,
      -1,    62,    -1,    63,    -1,    64,    -1,    65,    -1,    66,
      -1,    67,    -1,    68,    -1,    69,    -1,    70,    -1,    71,
      -1,    72,    -1,    92,    -1,    93,    -1,    91,    -1,   108,
      -1,   109,    -1,   110,    -1,   102,    -1,   103,    -1,   104,
      -1,   105,    -1,   106,    -1,   107,    -1,   111,    -1,   112,
      -1,   113,    -1,   181,    -1,   185,    -1,    13,   195,    -1,
      14,   195,    -1,    46,   195,    -1,    44,   195,    -1,    43,
     195,    -1,    45,   195,    -1,   195,    -1,   196,    47,   195,
      -1,   196,    48,   195,    -1,   196,    49,   195,    -1,   196,
      -1,   197,    46,   196,    -1,   197,    44,   196,    -1,   197,
      -1,   198,    11,   197,    -1,   198,    12,   197,    -1,   198,
      -1,   199,    50,   198,    -1,   199,    51,   198,    -1,   199,
      15,   198,    -1,   199,    16,   198,    -1,   199,    -1,   200,
      17,   199,    -1,   200,    18,   199,    -1,   200,    -1,   201,
      54,   200,    -1,   201,    -1,   202,    53,   201,    -1,   202,
      -1,   203,    52,   202,    -1,   203,    -1,   204,    19,   203,
      -1,   204,    -1,   205,    21,   204,    -1,   205,    -1,   206,
      20,   205,    -1,   206,    -1,   206,    55,   210,    40,   208,
      -1,   207,    -1,   195,    41,   208,    -1,   195,   209,   208,
      -1,    22,    -1,    23,    -1,    24,    -1,    31,    -1,    25,
      -1,    26,    -1,    27,    -1,    28,    -1,    29,    -1,    30,
      -1,   208,    -1,   210,    39,   208,    -1,   207,    -1,   213,
      42,    -1,   220,    42,    -1,   129,   226,   225,    42,    -1,
     214,    33,    -1,   216,    -1,   215,    -1,   216,   217,    -1,
     215,    39,   217,    -1,   222,   267,    32,    -1,   222,    34,
     211,    35,   267,    32,    -1,   218,   224,   267,    -1,   218,
     224,   267,    34,   211,    35,    -1,   218,   224,    -1,   218,
     224,    34,   211,    35,    -1,   223,   219,    -1,   219,    -1,
      -1,   114,    -1,   115,    -1,   116,    -1,   221,    -1,   220,
      39,   267,    -1,   220,    39,   267,    34,   211,    35,    -1,
     220,    39,   267,    41,   234,    -1,   222,    -1,   222,   267,
      -1,   222,   267,    34,   211,    35,    -1,   222,   267,    41,
     234,    -1,   125,   267,    -1,   224,    -1,   223,   224,    -1,
      90,    -1,    89,    -1,   118,    -1,   125,   118,    -1,   117,
      -1,   225,    -1,   226,   225,    -1,   123,    -1,    92,    -1,
      93,    -1,    91,    -1,   108,    -1,   109,    -1,   110,    -1,
     102,    -1,   103,    -1,   104,    -1,   105,    -1,   106,    -1,
     107,    -1,   111,    -1,   112,    -1,   113,    -1,   119,    -1,
     120,    -1,   121,    -1,   227,    -1,   181,    -1,   126,    -1,
     127,    -1,   128,    -1,    -1,   122,   228,   267,    36,   230,
      37,    -1,    -1,   122,   229,    36,   230,    37,    -1,   231,
      -1,   230,   231,    -1,   232,    42,    -1,   233,    -1,   232,
      39,   267,    -1,   232,    39,   267,    34,   211,    35,    -1,
     224,   267,    -1,   224,   267,    34,   211,    35,    -1,   208,
      -1,   212,    -1,   238,    -1,   237,    -1,   235,    -1,   246,
      -1,   247,    -1,   250,    -1,   262,    -1,    36,    37,    -1,
      -1,    36,   239,   245,    37,    -1,    -1,   241,   244,    -1,
      -1,   242,   237,    -1,   244,    -1,   237,    -1,    36,    37,
      -1,    36,   245,    37,    -1,   236,    -1,   245,   236,    -1,
      42,    -1,   210,    42,    -1,    99,    32,   210,    33,   248,
      -1,   240,    97,   240,    -1,   240,    -1,   210,    -1,   222,
     267,    41,   234,    -1,    -1,    -1,   124,    32,   251,   249,
      33,   252,   243,    -1,    -1,    -1,    96,   253,   240,   254,
     124,    32,   210,    33,    42,    -1,    -1,    -1,    98,    32,
     255,   259,    42,   260,    42,   261,    33,   256,   243,    -1,
     258,    -1,   226,   258,    -1,    92,    -1,    93,    -1,   257,
     267,    41,   234,    -1,   210,    -1,    -1,   249,    -1,    -1,
     210,    -1,    -1,    95,    42,    -1,    94,    42,    -1,   101,
      42,    -1,   101,   210,    42,    -1,   100,    42,    -1,   264,
      -1,   263,   264,    -1,   265,    -1,   212,    -1,    -1,   213,
     266,   244,    -1,     3,    -1,   181,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   508,   508,   512,   513,   514,   515,   516,   517,   521,
     522,   523,   524,   525,   526,   530,   534,   557,   558,   562,
     563,   567,   568,   572,   576,   577,   578,   583,   584,   585,
     586,   587,   588,   589,   590,   591,   592,   593,   594,   595,
     596,   597,   598,   599,   603,   604,   605,   606,   607,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   617,   618,
     622,   623,   624,   625,   626,   627,   628,   635,   636,   637,
     638,   642,   643,   644,   648,   649,   650,   654,   655,   656,
     657,   658,   662,   663,   664,   668,   669,   673,   674,   678,
     679,   683,   684,   688,   689,   693,   694,   698,   699,   703,
     704,   705,   709,   710,   711,   712,   713,   714,   715,   716,
     717,   718,   722,   723,   727,   731,   737,   740,   748,   755,
     756,   760,   761,   765,   773,   777,   778,   779,   780,   784,
     785,   789,   790,   791,   792,   797,   800,   804,   808,   816,
     820,   824,   828,   832,   840,   841,   845,   846,   847,   848,
     849,   853,   854,   858,   859,   860,   861,   862,   863,   864,
     865,   866,   867,   868,   869,   870,   871,   872,   873,   874,
     875,   887,   888,   889,   894,   895,   896,   902,   902,   903,
     903,   907,   908,   912,   916,   917,   918,   922,   923,   927,
     931,   935,   936,   942,   943,   944,   945,   946,   950,   951,
     951,   955,   955,   956,   956,   959,   960,   964,   965,   969,
     970,   974,   975,   979,   983,   984,   990,   991,   995,   995,
     995,   996,   996,   996,   997,   997,   997,  1001,  1002,  1006,
    1007,  1011,  1017,  1019,  1023,  1025,  1030,  1031,  1035,  1038,
    1041,  1042,  1043,  1049,  1055,  1064,  1065,  1069,  1069,  1074,
    1075
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "PPNUMBERI", "PPNUMBERF",
  "PPNUMBERU", "UNKNOWN", "HASH", "WHITESPACE", "NEWLINE", "LEFT_OP",
  "RIGHT_OP", "INC_OP", "DEC_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP",
  "LOGICAL_AND_OP", "LOGICAL_OR_OP", "LOGICAL_XOR_OP", "MUL_ASSIGN",
  "DIV_ASSIGN", "ADD_ASSIGN", "MOD_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN",
  "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN", "SUB_ASSIGN", "LEFT_PAREN",
  "RIGHT_PAREN", "LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_BRACE",
  "RIGHT_BRACE", "DOT", "COMMA", "COLON", "EQUAL", "SEMICOLON", "BANG",
  "DASH", "TILDE", "PLUS", "STAR", "SLASH", "PERCENT", "LEFT_ANGLE",
  "RIGHT_ANGLE", "BITWISE_OR_OP", "BITWISE_XOR_OP", "BITWISE_AND_OP",
  "QUESTION", "INTRINSIC_TEXTURE_2D_BIAS", "INTRINSIC_TEXTURE_2D_LOD",
  "INTRINSIC_TEXTURE_CUBE_BIAS", "INTRINSIC_TEXTURE_CUBE_LOD",
  "INTRINSIC_RSQRT", "INTRINSIC_RCP", "INTRINSIC_LOG2", "INTRINSIC_EXP2",
  "INTRINSIC_CEIL", "INTRINSIC_FLOOR", "INTRINSIC_SIGN", "INTRINSIC_TRUNC",
  "INTRINSIC_NEAREST", "INTRINSIC_MIN", "INTRINSIC_MAX",
  "INTRINSIC_MINABS", "INTRINSIC_MAXABS", "DEFINE", "UNDEF", "IFDEF",
  "IFNDEF", "ELIF", "ENDIF", "_ERROR", "PRAGMA", "EXTENSION", "VERSION",
  "LINE", "ALL", "REQUIRE", "ENABLE", "WARN", "DISABLE", "ATTRIBUTE",
  "_CONST", "_BOOL", "_FLOAT", "_INT", "BREAK", "CONTINUE", "DO", "ELSE",
  "FOR", "IF", "DISCARD", "RETURN", "BVEC2", "BVEC3", "BVEC4", "IVEC2",
  "IVEC3", "IVEC4", "VEC2", "VEC3", "VEC4", "_MAT2", "MAT3", "MAT4", "_IN",
  "_OUT", "INOUT", "UNIFORM", "VARYING", "SAMPLER2D", "SAMPLEREXTERNAL",
  "SAMPLERCUBE", "STRUCT", "_VOID", "WHILE", "INVARIANT", "HIGH_PRECISION",
  "MEDIUM_PRECISION", "LOW_PRECISION", "PRECISION", "ASM", "CLASS",
  "UNION", "ENUM", "TYPEDEF", "TEMPLATE", "THIS", "PACKED", "GOTO",
  "SWITCH", "DEFAULT", "_INLINE", "NOINLINE", "VOLATILE", "PUBLIC",
  "STATIC", "EXTERN", "EXTERNAL", "INTERFACE", "FLAT", "_LONG", "_SHORT",
  "DOUBLE", "HALF", "_FIXED", "_UNSIGNED", "SUPERP", "_INPUT", "OUTPUT",
  "HVEC2", "HVEC3", "HVEC4", "DVEC2", "DVEC3", "DVEC4", "FVEC2", "FVEC3",
  "FVEC4", "SAMPLER1D", "SAMPLER3D", "SAMPLER1DSHADOW", "SAMPLER2DSHADOW",
  "SAMPLER2DRECT", "SAMPLER3DRECT", "SAMPLER2DRECTSHADOW", "SIZEOF",
  "CAST", "NAMESPACE", "USING", "_TRUE", "_FALSE", "CANDIDATE_TYPE_NAME",
  "$accept", "variable_identifier", "primary_expression",
  "postfix_expression", "integer_expression", "function_call",
  "function_call_generic", "function_call_header_no_parameters",
  "function_call_header_with_parameters", "function_call_header",
  "function_identifier", "intrinsic_identifier", "constructor_identifier",
  "unary_expression", "multiplicative_expression", "additive_expression",
  "shift_expression", "relational_expression", "equality_expression",
  "and_expression", "exclusive_or_expression", "inclusive_or_expression",
  "logical_and_expression", "logical_xor_expression",
  "logical_or_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "declaration", "function_prototype",
  "function_declarator", "function_header_with_parameters",
  "function_header", "parameter_declaration",
  "type_and_parameter_qualifier", "parameter_qualifier",
  "init_declarator_list", "init_declarator", "fully_specified_type",
  "type_qualifier", "type_specifier", "type_specifier_no_prec",
  "precision_qualifier", "struct_specifier", "$@1", "$@2",
  "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator", "initializer",
  "declaration_statement", "statement", "simple_statement",
  "compound_statement", "$@3", "statement_with_scope", "$@4", "$@5",
  "statement_no_new_scope", "compound_statement_no_new_scope",
  "statement_list", "expression_statement", "selection_statement",
  "selection_rest_statement", "condition", "iteration_statement", "$@6",
  "$@7", "$@8", "$@9", "$@10", "$@11", "induction_type_specifier",
  "induction_type_specifier_no_prec", "for_init", "for_test", "for_loop",
  "jump_statement", "translation_unit", "external_declaration",
  "function_definition", "$@12", "identifier_or_typename", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   691,   512,   513,   514,   515,   516,   517,   518,
     519,   520,   521,   522,   523,   524,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,   564,   565,   566,   567,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   585,   586,   587,   588,
     589,   590,   591,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,   602,   603,   604,   605,   606,   607,   608,
     609,   610,   611,   612,   613,   614,   615,   616,   617,   618,
     619,   620,   621,   622,   623,   624,   625,   626,   627,   628,
     629,   630,   631,   632,   633,   634,   635,   636,   637,   638,
     639,   640,   641,   642,   643,   644,   645,   646,   647,   648,
     649,   650,   651,   652,   653,   654,   655,   656,   657,   658,
     659,   660,   661,   662,   663,   664,   665,   666,   667,   668,
     669,   670,   671,   672,   673,   674,   675,   676,   677,   678,
     679,   680,   681,   682,   683,   684,   685,   686,   687,   688,
     689,   690
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   182,   183,   184,   184,   184,   184,   184,   184,   185,
     185,   185,   185,   185,   185,   186,   187,   188,   188,   189,
     189,   190,   190,   191,   192,   192,   192,   193,   193,   193,
     193,   193,   193,   193,   193,   193,   193,   193,   193,   193,
     193,   193,   193,   193,   194,   194,   194,   194,   194,   194,
     194,   194,   194,   194,   194,   194,   194,   194,   194,   194,
     195,   195,   195,   195,   195,   195,   195,   196,   196,   196,
     196,   197,   197,   197,   198,   198,   198,   199,   199,   199,
     199,   199,   200,   200,   200,   201,   201,   202,   202,   203,
     203,   204,   204,   205,   205,   206,   206,   207,   207,   208,
     208,   208,   209,   209,   209,   209,   209,   209,   209,   209,
     209,   209,   210,   210,   211,   212,   212,   212,   213,   214,
     214,   215,   215,   216,   216,   217,   217,   217,   217,   218,
     218,   219,   219,   219,   219,   220,   220,   220,   220,   221,
     221,   221,   221,   221,   222,   222,   223,   223,   223,   223,
     223,   224,   224,   225,   225,   225,   225,   225,   225,   225,
     225,   225,   225,   225,   225,   225,   225,   225,   225,   225,
     225,   225,   225,   225,   226,   226,   226,   228,   227,   229,
     227,   230,   230,   231,   232,   232,   232,   233,   233,   234,
     235,   236,   236,   237,   237,   237,   237,   237,   238,   239,
     238,   241,   240,   242,   240,   243,   243,   244,   244,   245,
     245,   246,   246,   247,   248,   248,   249,   249,   251,   252,
     250,   253,   254,   250,   255,   256,   250,   257,   257,   258,
     258,   259,   259,   259,   260,   260,   261,   261,   262,   262,
     262,   262,   262,   263,   263,   264,   264,   266,   265,   267,
     267
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     3,     1,
       4,     1,     3,     2,     2,     1,     1,     2,     2,     2,
       1,     2,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     1,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     5,     1,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     2,     2,     4,     2,     1,
       1,     2,     3,     3,     6,     3,     6,     2,     5,     2,
       1,     0,     1,     1,     1,     1,     3,     6,     5,     1,
       2,     5,     4,     2,     1,     2,     1,     1,     1,     2,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     6,     0,
       5,     1,     2,     2,     1,     3,     6,     2,     5,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       4,     0,     2,     0,     2,     1,     1,     2,     3,     1,
       2,     1,     2,     5,     3,     1,     1,     4,     0,     0,
       7,     0,     0,     9,     0,     0,    11,     1,     2,     1,
       1,     4,     1,     0,     1,     0,     1,     0,     2,     2,
       2,     3,     2,     1,     2,     1,     1,     0,     3,     1,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   147,   146,   156,   154,   155,   160,   161,   162,   163,
     164,   165,   157,   158,   159,   166,   167,   168,   150,   148,
     169,   170,   171,   177,   153,     0,   174,   175,   176,     0,
     173,   246,   247,     0,   120,   131,     0,   135,   139,     0,
     144,   151,     0,   172,     0,   243,   245,     0,     0,   249,
     149,   250,   143,     0,   115,     0,   118,   131,   132,   133,
     134,     0,   121,     0,   130,   131,     0,   116,     0,   140,
     145,   152,     1,   244,     0,     0,     0,     0,   248,   122,
     127,   129,   136,     2,     5,     4,     0,     0,     0,     0,
       0,     0,     0,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      46,    44,    45,    50,    51,    52,    53,    54,    55,    47,
      48,    49,    56,    57,    58,     6,     7,    59,     3,     9,
      60,    11,    16,     0,     0,    20,     0,    25,    24,    67,
      71,    74,    77,    82,    85,    87,    89,    91,    93,    95,
      97,   114,     0,   123,     0,     0,     0,     0,     0,   181,
       0,   184,   117,   199,   207,   211,   156,   154,   155,     0,
       0,   221,     0,     0,     0,     0,   160,   161,   162,   163,
     164,   165,   157,   158,   159,   166,   167,   168,     0,   173,
      67,    99,   112,     0,   190,     0,   193,   209,   192,   191,
       0,   194,   195,   196,   197,     0,   125,     0,     0,    61,
      62,     0,    65,    64,    66,    63,    13,    14,     0,     0,
      18,    17,     0,    19,    21,    23,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   189,   142,
       0,   187,   180,   182,     0,   183,   198,     0,   239,   238,
     203,   224,     0,   242,   240,     0,   218,   102,   103,   104,
     106,   107,   108,   109,   110,   111,   105,     0,     0,     0,
     212,   208,   210,     0,     0,     0,   138,     8,     0,    15,
      12,    22,    68,    69,    70,    73,    72,    75,    76,    80,
      81,    78,    79,    83,    84,    86,    88,    90,    92,    94,
      96,     0,     0,   141,   178,     0,   185,     0,   222,     0,
       0,   233,     0,   241,     0,   100,   101,   113,   128,     0,
     137,    10,     0,   124,     0,     0,   200,     0,   202,   204,
     229,   230,   232,     0,     0,   227,     0,   203,   216,     0,
       0,   126,    98,   188,     0,     0,   229,   230,   228,     0,
     235,   215,   213,     0,   219,   186,     0,     0,   234,     0,
     203,     0,     0,     0,   231,   237,   214,   217,   206,   220,
     205,     0,   236,     0,   223,   225,     0,   226
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   128,   129,   130,   288,   131,   132,   133,   134,   135,
     136,   137,   138,   190,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   191,   192,   278,   193,   152,
     194,   195,    33,    34,    35,    62,    63,    64,    36,    37,
      38,    39,    40,    41,    42,    43,    47,    48,   158,   159,
     160,   161,   249,   196,   197,   198,   199,   257,   318,   319,
     320,   379,   380,   200,   201,   202,   362,   350,   203,   324,
     372,   260,   337,   321,   386,   344,   345,   346,   369,   383,
     204,    44,    45,    46,    55,    52
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -313
static const yytype_int16 yypact[] =
{
    1635,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,    36,  -313,    21,  -313,  -313,  -313,   -10,
    -313,  -313,    -8,    54,    58,   -15,    53,  -313,    26,  1675,
    -313,  -313,   888,  -313,   210,  -313,  -313,    30,    71,  -313,
    -313,  -313,  -313,   888,  -313,   102,  -313,   -49,  -313,  -313,
    -313,    22,  -313,  1675,  -313,     9,    30,  -313,  1515,    -2,
    -313,  -313,  -313,  -313,   106,  1675,   104,   337,  -313,  -313,
      28,  -313,    37,   115,  -313,  -313,  1515,  1515,  1515,  1515,
    1515,  1515,  1515,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
      23,  -313,  -313,   119,   -20,  1351,   116,  -313,  -313,  -313,
      79,    47,    11,     5,    96,    99,   101,   103,   137,   136,
      -4,  -313,   123,  -313,  1515,  1515,  1675,    30,  1551,  -313,
      66,  -313,  -313,   124,  -313,  -313,   128,   131,   134,   125,
     126,  -313,   138,   139,   130,  1433,   141,   142,   145,   146,
     147,   148,   150,   171,   173,   180,   181,   182,   183,   185,
     232,  -313,  -313,    67,  -313,    -8,  -313,  -313,  -313,  -313,
     464,  -313,  -313,  -313,  -313,  1515,   184,  1515,  1515,  -313,
    -313,    31,  -313,  -313,  -313,  -313,  -313,  -313,  1515,    30,
    -313,  -313,  1515,  -313,  -313,  -313,  1515,  1515,  1515,  1515,
    1515,  1515,  1515,  1515,  1515,  1515,  1515,  1515,  1515,  1515,
    1515,  1515,  1515,  1515,  1515,  1515,    30,   186,  -313,  -313,
    1595,   188,  -313,  -313,    30,  -313,  -313,   718,  -313,  -313,
     187,  -313,  1515,  -313,  -313,    73,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  1515,  1515,  1515,
    -313,  -313,  -313,   189,  1515,   192,  -313,  -313,   193,   190,
    -313,  -313,  -313,  -313,  -313,    79,    79,    47,    47,    11,
      11,    11,    11,     5,     5,    96,    99,   101,   103,   137,
     136,    81,   198,  -313,  -313,  1515,   197,   591,  -313,   102,
     972,  1225,    40,  -313,  1099,  -313,  -313,  -313,  -313,   199,
    -313,  -313,  1515,  -313,   201,  1515,  -313,    95,  -313,  -313,
     131,   134,   190,    42,    30,  -313,   191,   187,   190,    30,
     204,  -313,  -313,  -313,   205,   200,  -313,  -313,  -313,   203,
    1099,   149,  -313,   206,  -313,  -313,  1515,  1515,  -313,   207,
     187,  1515,   845,    49,  -313,  1515,  -313,  -313,  -313,  -313,
    -313,   208,   190,   209,  -313,  -313,   845,  -313
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,  -313,
    -313,  -313,  -313,   -43,  -100,   -95,  -150,   -93,     6,    12,
       7,    27,    25,    20,  -313,   -64,  -128,  -313,   -86,  -109,
       8,    10,  -313,  -313,  -313,   196,  -313,   211,  -313,  -313,
    -307,    24,   -25,   -27,   -28,  -313,  -313,  -313,   109,  -146,
    -313,  -313,  -202,  -313,  -195,  -309,  -313,  -313,  -312,  -313,
    -313,  -120,   -52,    13,  -313,  -313,  -313,   -85,  -313,  -313,
    -313,  -313,  -313,  -313,  -313,  -313,   -66,  -313,  -313,  -313,
    -313,  -313,   234,  -313,  -313,   -38
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -202
static const yytype_int16 yytable[] =
{
      69,    53,   211,    78,   151,   282,   286,   224,    31,    74,
      32,   339,   253,   221,    70,    71,   244,   349,  -119,   222,
     233,   234,   231,   232,    49,   139,    76,   248,    82,    49,
     153,    49,   154,    49,    54,   361,   216,   217,    80,   155,
       1,     2,   206,   209,   210,   247,   212,   213,   214,   215,
     157,   245,    31,   349,    32,   235,   236,   218,   376,    65,
      68,   219,   205,   378,   287,    58,    59,    60,    18,    19,
     279,   207,  -179,   347,     1,     2,    61,   378,   208,   279,
     248,    65,   381,   299,   300,   301,   302,    56,   279,   265,
     151,   229,    66,   230,   291,    67,   283,    57,   285,    58,
      59,    60,    18,    19,   253,   254,   279,    75,   255,   280,
      61,   139,   279,   237,   238,   323,    26,    27,    28,   251,
     279,   332,   282,    58,    59,    60,   226,   227,   228,   295,
     296,   157,   289,   157,   356,   357,   297,   298,    77,    50,
      50,   151,   156,   151,   303,   304,   162,   -26,   225,   325,
     326,   327,   220,   239,   240,   241,   242,   243,   246,   311,
     -46,   256,   139,   -44,   139,   374,   -45,   258,   259,   377,
     261,   262,   263,   -50,   -51,   329,   322,   -52,   -53,   -54,
     -55,   290,   -47,   292,   293,   294,   139,   139,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   139,
     139,   139,    51,   -48,   352,   -49,   334,    51,   312,    51,
      72,    51,   -56,   -57,   -58,   266,   316,   -59,   284,   355,
     151,   313,   315,  -201,   328,   157,   354,   330,   331,   279,
     333,   335,   366,   360,   351,   342,   353,   364,   348,   248,
     365,   139,   385,   248,   367,   305,   370,   371,   307,   375,
     384,   151,   306,    79,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   310,   250,   387,   338,   309,   308,
     317,   151,   139,   277,   348,   368,    81,   358,    73,     0,
     373,     0,     0,     0,     0,     0,     0,     0,     0,   382,
       0,     0,   139,   343,     0,     0,     0,     0,     0,     1,
       2,     3,     4,     5,     0,     0,   359,     0,     0,     0,
       0,   363,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,     0,     0,     0,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,    27,    28,    29,
      83,    84,    85,     0,     0,     0,     0,     0,     0,     0,
      86,    87,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    88,
       0,     0,     0,   163,   164,     0,     0,     0,     0,   165,
      89,    90,    91,    92,     0,     0,     0,     0,     0,     0,
       0,    30,     0,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     1,     2,   166,   167,
     168,   169,   170,   171,     0,   172,   173,   174,   175,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,   188,    25,    26,    27,    28,    29,    83,    84,    85,
       0,     0,     0,     0,     0,     0,     0,    86,    87,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    88,     0,     0,     0,
     163,   281,     0,     0,     0,     0,   165,    89,    90,    91,
      92,     0,     0,     0,     0,     0,   125,   126,   189,     0,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     1,     2,   166,   167,   168,   169,   170,
     171,     0,   172,   173,   174,   175,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,   188,    25,
      26,    27,    28,    29,    83,    84,    85,     0,     0,     0,
       0,     0,     0,     0,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,     0,     0,     0,   163,   336,     0,
       0,     0,     0,   165,    89,    90,    91,    92,     0,     0,
       0,     0,     0,   125,   126,   189,     0,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       1,     2,   166,   167,   168,   169,   170,   171,     0,   172,
     173,   174,   175,   176,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,     0,     0,     0,    18,    19,
      20,    21,    22,    23,    24,   188,    25,    26,    27,    28,
      29,    83,    84,    85,     0,     0,     0,     0,     0,     0,
       0,    86,    87,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      88,     0,     0,     0,   163,     0,     0,     0,     0,     0,
     165,    89,    90,    91,    92,     0,     0,     0,     0,     0,
     125,   126,   189,     0,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     1,     2,   166,
     167,   168,   169,   170,   171,     0,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,   188,    25,    26,    27,    28,    29,    83,    84,
      85,     0,     0,     0,     0,     0,     0,     0,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    88,     0,     0,
       0,    77,     0,     0,     0,     0,     0,   165,    89,    90,
      91,    92,     0,     0,     0,     0,     0,   125,   126,   189,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,     2,   166,   167,   168,   169,
     170,   171,     0,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   186,   187,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,   188,
      25,    26,    27,    28,    29,    83,    84,    85,     0,     3,
       4,     5,     0,     0,     0,    86,    87,     0,     0,     0,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,     0,     0,    88,     0,     0,    20,    21,    22,
      23,    24,     0,     0,   165,    89,    90,    91,    92,     0,
       0,     0,     0,     0,   125,   126,   189,     0,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     1,     2,   166,   167,   168,   169,   170,   171,    30,
     172,   173,   174,   175,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,     0,     0,     0,    18,
      19,    20,    21,    22,    23,    24,   188,    25,    26,    27,
      28,    29,    83,    84,    85,     0,     0,     0,     0,     0,
       0,     0,    86,    87,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    89,    90,    91,    92,     0,     0,     0,     0,
       0,   125,   126,   189,     0,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     1,     2,
     166,   167,   168,     0,     0,     0,     0,     0,     0,     0,
       0,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,     0,     0,     0,    18,    19,    20,    21,
      22,    23,    24,     0,    61,    26,    27,    28,    83,    84,
      85,     0,     0,     0,     0,     0,     0,     0,    86,    87,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    89,    90,
      91,    92,     0,     0,     0,     0,     0,     0,   125,   126,
     189,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   110,   340,   341,     0,
       0,     0,     0,     0,     0,     0,     0,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    26,    27,    28,    83,    84,    85,     0,     0,     0,
       0,     0,     0,     0,    86,    87,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    88,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    89,    90,    91,    92,     0,     0,
       0,     0,     0,     0,   125,   126,   127,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    83,    84,    85,     0,
       0,     0,   110,   111,   112,     0,    86,    87,     0,     0,
       0,     0,     0,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,    88,     0,     0,     0,     0,
       0,     0,     0,     0,   223,   264,    89,    90,    91,    92,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    83,    84,
      85,     0,     0,     0,   110,   111,   112,     0,    86,    87,
     125,   126,   127,     0,     0,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,    88,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    89,    90,
      91,    92,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   252,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   110,   111,   112,     0,
       0,     0,   125,   126,   127,     0,     0,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,     0,
       0,     0,   314,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     4,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,     0,     0,     0,     0,     0,
      20,    21,    22,    23,    24,     0,     0,    26,    27,    28,
       0,     0,     0,     0,     0,     0,     3,     4,     5,     0,
       0,     0,     0,     0,   125,   126,   127,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,    20,    21,    22,    23,    24,     0,
       0,    26,    27,    28,     1,     2,     3,     4,     5,     0,
       0,     0,    30,     0,     0,     0,     0,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,    18,    19,    20,    21,    22,    23,    24,     0,
      25,    26,    27,    28,    29,     0,     3,     4,     5,     0,
       0,     0,     0,     0,     0,     0,    30,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,     0,
       0,     0,     0,     0,    20,    21,    22,    23,    24,     0,
       0,    26,    27,    28,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    30
};

static const yytype_int16 yycheck[] =
{
      38,    29,    88,    55,    68,   200,   208,   135,     0,    47,
       0,   320,   158,    33,    39,    42,    20,   324,    33,    39,
      15,    16,    11,    12,     3,    68,    53,   155,    66,     3,
      32,     3,    34,     3,    42,   347,    13,    14,    63,    41,
      89,    90,    80,    86,    87,   154,    89,    90,    91,    92,
      75,    55,    44,   360,    44,    50,    51,    34,   370,    35,
      34,    38,    34,   372,    33,   114,   115,   116,   117,   118,
      39,    34,    36,    33,    89,    90,   125,   386,    41,    39,
     208,    57,    33,   233,   234,   235,   236,    33,    39,   175,
     154,    44,    39,    46,   222,    42,   205,    39,   207,   114,
     115,   116,   117,   118,   250,    39,    39,    36,    42,    42,
     125,   154,    39,    17,    18,    42,   126,   127,   128,   157,
      39,    40,   317,   114,   115,   116,    47,    48,    49,   229,
     230,   156,   218,   158,    92,    93,   231,   232,    36,   118,
     118,   205,    36,   207,   237,   238,    42,    32,    32,   277,
     278,   279,    33,    54,    53,    52,    19,    21,    35,   245,
      32,    37,   205,    32,   207,   367,    32,    42,    42,   371,
      32,    32,    42,    32,    32,   284,   262,    32,    32,    32,
      32,   219,    32,   226,   227,   228,   229,   230,   231,   232,
     233,   234,   235,   236,   237,   238,   239,   240,   241,   242,
     243,   244,   181,    32,   332,    32,   315,   181,   246,   181,
       0,   181,    32,    32,    32,    32,   254,    32,    34,   124,
     284,    35,    34,    36,    35,   250,   335,    35,    35,    39,
      32,    34,    32,    42,    35,   321,    35,    33,   324,   367,
      35,   284,    33,   371,    41,   239,    97,    41,   241,    42,
      42,   315,   240,    57,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,   244,   156,   386,   319,   243,   242,
     257,   335,   315,    41,   360,   360,    65,   343,    44,    -1,
     366,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   375,
      -1,    -1,   335,   321,    -1,    -1,    -1,    -1,    -1,    89,
      90,    91,    92,    93,    -1,    -1,   344,    -1,    -1,    -1,
      -1,   349,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,    -1,    -1,    -1,   117,   118,   119,
     120,   121,   122,   123,    -1,   125,   126,   127,   128,   129,
       3,     4,     5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,
      -1,    -1,    -1,    36,    37,    -1,    -1,    -1,    -1,    42,
      43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   181,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,    92,
      93,    94,    95,    96,    -1,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,    -1,    -1,    -1,   117,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,     3,     4,     5,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,    -1,
      36,    37,    -1,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,    -1,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    89,    90,    91,    92,    93,    94,    95,
      96,    -1,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,    -1,    -1,
      -1,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    36,    37,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,   179,   180,   181,    -1,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      89,    90,    91,    92,    93,    94,    95,    96,    -1,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
     129,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      32,    -1,    -1,    -1,    36,    -1,    -1,    -1,    -1,    -1,
      42,    43,    44,    45,    46,    -1,    -1,    -1,    -1,    -1,
     179,   180,   181,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,    91,
      92,    93,    94,    95,    96,    -1,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,    -1,    -1,    -1,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    36,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,   179,   180,   181,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    89,    90,    91,    92,    93,    94,
      95,    96,    -1,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,     3,     4,     5,    -1,    91,
      92,    93,    -1,    -1,    -1,    13,    14,    -1,    -1,    -1,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,    -1,    -1,    32,    -1,    -1,   119,   120,   121,
     122,   123,    -1,    -1,    42,    43,    44,    45,    46,    -1,
      -1,    -1,    -1,    -1,   179,   180,   181,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    89,    90,    91,    92,    93,    94,    95,    96,   181,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,    -1,    -1,    -1,   117,
     118,   119,   120,   121,   122,   123,   124,   125,   126,   127,
     128,   129,     3,     4,     5,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    43,    44,    45,    46,    -1,    -1,    -1,    -1,
      -1,   179,   180,   181,    -1,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    89,    90,
      91,    92,    93,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,    -1,    -1,    -1,   117,   118,   119,   120,
     121,   122,   123,    -1,   125,   126,   127,   128,     3,     4,
       5,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    13,    14,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,   179,   180,
     181,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   126,   127,   128,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    32,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    43,    44,    45,    46,    -1,    -1,
      -1,    -1,    -1,    -1,   179,   180,   181,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
      -1,    -1,    91,    92,    93,    -1,    13,    14,    -1,    -1,
      -1,    -1,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    32,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   123,    42,    43,    44,    45,    46,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,    -1,    -1,    91,    92,    93,    -1,    13,    14,
     179,   180,   181,    -1,    -1,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,    32,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    37,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
      -1,    -1,   179,   180,   181,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    91,    92,    93,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,    -1,    -1,    -1,    -1,    -1,
     119,   120,   121,   122,   123,    -1,    -1,   126,   127,   128,
      -1,    -1,    -1,    -1,    -1,    -1,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,   179,   180,   181,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,    -1,
      -1,   126,   127,   128,    89,    90,    91,    92,    93,    -1,
      -1,    -1,   181,    -1,    -1,    -1,    -1,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,   117,   118,   119,   120,   121,   122,   123,    -1,
     125,   126,   127,   128,   129,    -1,    91,    92,    93,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,    -1,
      -1,    -1,    -1,    -1,   119,   120,   121,   122,   123,    -1,
      -1,   126,   127,   128,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   181
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    89,    90,    91,    92,    93,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   117,   118,
     119,   120,   121,   122,   123,   125,   126,   127,   128,   129,
     181,   212,   213,   214,   215,   216,   220,   221,   222,   223,
     224,   225,   226,   227,   263,   264,   265,   228,   229,     3,
     118,   181,   267,   226,    42,   266,    33,    39,   114,   115,
     116,   125,   217,   218,   219,   223,    39,    42,    34,   267,
     224,   225,     0,   264,   267,    36,   225,    36,   244,   217,
     224,   219,   267,     3,     4,     5,    13,    14,    32,    43,
      44,    45,    46,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      91,    92,    93,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   179,   180,   181,   183,   184,
     185,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   211,    32,    34,    41,    36,   224,   230,   231,
     232,   233,    42,    36,    37,    42,    91,    92,    93,    94,
      95,    96,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   124,   181,
     195,   207,   208,   210,   212,   213,   235,   236,   237,   238,
     245,   246,   247,   250,   262,    34,   267,    34,    41,   195,
     195,   210,   195,   195,   195,   195,    13,    14,    34,    38,
      33,    33,    39,   123,   208,    32,    47,    48,    49,    44,
      46,    11,    12,    15,    16,    50,    51,    17,    18,    54,
      53,    52,    19,    21,    20,    55,    35,   211,   208,   234,
     230,   267,    37,   231,    39,    42,    37,   239,    42,    42,
     253,    32,    32,    42,    42,   210,    32,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    41,   209,    39,
      42,    37,   236,   211,    34,   211,   234,    33,   186,   210,
     267,   208,   195,   195,   195,   196,   196,   197,   197,   198,
     198,   198,   198,   199,   199,   200,   201,   202,   203,   204,
     205,   210,   267,    35,    37,    34,   267,   245,   240,   241,
     242,   255,   210,    42,   251,   208,   208,   208,    35,   211,
      35,    35,    40,    32,   211,    34,    37,   254,   244,   237,
      92,    93,   210,   226,   257,   258,   259,    33,   210,   222,
     249,    35,   208,    35,   211,   124,    92,    93,   258,   267,
      42,   240,   248,   267,    33,    35,    32,    41,   249,   260,
      97,    41,   252,   210,   234,    42,   240,   234,   237,   243,
     244,    33,   210,   261,    42,    33,   256,   243
};

#define yyerrok      (yyerrstatus = 0)
#define yyclearin   (yychar = YYEMPTY)
#define YYEMPTY      (-2)
#define YYEOF      0

#define YYACCEPT   goto yyacceptlab
#define YYABORT      goto yyabortlab
#define YYERROR      goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL      goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)               \
do                        \
  if (yychar == YYEMPTY && yylen == 1)            \
    {                        \
      yychar = (Token);                  \
      yylval = (Value);                  \
      yytoken = YYTRANSLATE (yychar);            \
      YYPOPSTACK (1);                  \
      goto yybackup;                  \
    }                        \
  else                        \
    {                        \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                     \
    }                        \
while (YYID (0))


#define YYTERROR   1
#define YYERRCODE   256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)            \
    do                           \
      if (YYID (N))                                                    \
   {                        \
     (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;   \
     (Current).first_column = YYRHSLOC (Rhs, 1).first_column;   \
     (Current).last_line    = YYRHSLOC (Rhs, N).last_line;      \
     (Current).last_column  = YYRHSLOC (Rhs, N).last_column;   \
   }                        \
      else                        \
   {                        \
     (Current).first_line   = (Current).last_line   =      \
       YYRHSLOC (Rhs, 0).last_line;            \
     (Current).first_column = (Current).last_column =      \
       YYRHSLOC (Rhs, 0).last_column;            \
   }                        \
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)         \
     fprintf (File, "%d.%d-%d.%d",         \
         (Loc).first_line, (Loc).first_column,   \
         (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)         \
do {                  \
  if (yydebug)               \
    YYFPRINTF Args;            \
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)           \
do {                             \
  if (yydebug)                          \
    {                             \
      YYFPRINTF (stderr, "%s ", Title);                 \
      yy_symbol_print (stderr,                    \
        Type, Value); \
      YYFPRINTF (stderr, "\n");                    \
    }                             \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
   break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)            \
do {                        \
  if (yydebug)                     \
    yy_stack_print ((Bottom), (Top));            \
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
        yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
             &(yyvsp[(yyi + 1) - (yynrhs)])
                          );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)      \
do {               \
  if (yydebug)            \
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef   YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
   switch (*++yyp)
     {
     case '\'':
     case ',':
       goto do_not_strip_quotes;

     case '\\':
       if (*++yyp != '\\')
         goto do_not_strip_quotes;
       /* Fall through.  */
     default:
       if (yyres)
         yyres[yyn] = *yyp;
       yyn++;
       break;

     case '"':
       if (yyres)
         yyres[yyn] = '\0';
       return yyn;
     }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
    constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
          + sizeof yyexpecting - 1
          + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
             * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
    YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
   if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
     {
       if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
         {
      yycount = 1;
      yysize = yysize0;
      yyformat[sizeof yyunexpected - 1] = '\0';
      break;
         }
       yyarg[yycount++] = yytname[yyx];
       yysize1 = yysize + yytnamerr (0, yytname[yyx]);
       yysize_overflow |= (yysize1 < yysize);
       yysize = yysize1;
       yyfmt = yystpcpy (yyfmt, yyprefix);
       yyprefix = yyor;
     }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
   return YYSIZE_MAXIMUM;

      if (yyresult)
   {
     /* Avoid sprintf, as that infringes on the user's name space.
        Don't have undefined behavior even if the translation
        produced a string with the wrong number of "%s"s.  */
     char *yyp = yyresult;
     int yyi = 0;
     while ((*yyp = *yyf) != '\0')
       {
         if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
      {
        yyp += yytnamerr (yyp, yyarg[yyi++]);
        yyf += 2;
      }
         else
      {
        yyp++;
        yyf++;
      }
       }
   }
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
   break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
   /* Give user a chance to reallocate the stack.  Use copies of
      these so that the &'s don't force the real ones into
      memory.  */
   YYSTYPE *yyvs1 = yyvs;
   yytype_int16 *yyss1 = yyss;

   /* Each stack pointer address is followed by the size of the
      data in use in that stack, in bytes.  This used to be a
      conditional around just the two extra args, but that might
      be undefined if yyoverflow is a macro.  */
   yyoverflow (YY_("memory exhausted"),
          &yyss1, yysize * sizeof (*yyssp),
          &yyvs1, yysize * sizeof (*yyvsp),
          &yystacksize);

   yyss = yyss1;
   yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
   goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
   yystacksize = YYMAXDEPTH;

      {
   yytype_int16 *yyss1 = yyss;
   union yyalloc *yyptr =
     (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
   if (! yyptr)
     goto yyexhaustedlab;
   YYSTACK_RELOCATE (yyss_alloc, yyss);
   YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
   if (yyss1 != yyssa)
     YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
        (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
   YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
   goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

    { (yyval.lookup) = (yyvsp[(1) - (1)].lookup); ;}
    break;

  case 3:

    { (yyval.expr) = glsl_expr_construct_instance((yyvsp[(1) - (1)].lookup).symbol); ;}
    break;

  case 4:

    { (yyval.expr) = glsl_expr_construct_value_float((yyvsp[(1) - (1)].f)); ;}
    break;

  case 5:

    { (yyval.expr) = glsl_expr_construct_value_int((yyvsp[(1) - (1)].i)); ;}
    break;

  case 6:

    { (yyval.expr) = glsl_expr_construct_value_bool(CONST_BOOL_TRUE); ;}
    break;

  case 7:

    { (yyval.expr) = glsl_expr_construct_value_bool(CONST_BOOL_FALSE); ;}
    break;

  case 8:

    { (yyval.expr) = (yyvsp[(2) - (3)].expr); ;}
    break;

  case 9:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 10:

    { (yyval.expr) = glsl_expr_construct_subscript((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr)); ;}
    break;

  case 11:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 12:

    { (yyval.expr) = glsl_expr_construct_field_selector((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].s)); ;}
    break;

  case 13:

    { (yyval.expr) = glsl_expr_construct_unary_op_arithmetic(EXPR_POST_INC, (yyvsp[(1) - (2)].expr)); ;}
    break;

  case 14:

    { (yyval.expr) = glsl_expr_construct_unary_op_arithmetic(EXPR_POST_DEC, (yyvsp[(1) - (2)].expr)); ;}
    break;

  case 15:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 16:

    {
                                 switch((yyvsp[(1) - (1)].call_context).flavour)
                                 {
                                    case CALL_CONTEXT_FUNCTION:
                                       (yyval.expr) = glsl_expr_construct_function_call((yyvsp[(1) - (1)].call_context).u.function.symbol, (yyvsp[(1) - (1)].call_context).args);
                                       break;
                                    case CALL_CONTEXT_PRIM_CONSTRUCTOR:
                                       (yyval.expr) = glsl_expr_construct_prim_constructor_call((yyvsp[(1) - (1)].call_context).u.prim_constructor.index, (yyvsp[(1) - (1)].call_context).args);
                                       break;
                                    case CALL_CONTEXT_TYPE_CONSTRUCTOR:
                                       (yyval.expr) = glsl_expr_construct_type_constructor_call((yyvsp[(1) - (1)].call_context).u.type_constructor.symbol->type, (yyvsp[(1) - (1)].call_context).args);
                                       break;
                                    case CALL_CONTEXT_INTRINSIC:
                                       (yyval.expr) = glsl_expr_construct_intrinsic((yyvsp[(1) - (1)].call_context).u.intrinsic.flavour, (yyvsp[(1) - (1)].call_context).args);
                                       break;
                                    default:
                                       UNREACHABLE();
                                       break;
                                 }
                              ;}
    break;

  case 17:

    { (yyval.call_context) = (yyvsp[(1) - (2)].call_context); ;}
    break;

  case 18:

    { (yyval.call_context) = (yyvsp[(1) - (2)].call_context); ;}
    break;

  case 19:

    { (yyval.call_context).args = glsl_expr_chain_create(); ;}
    break;

  case 20:

    { (yyval.call_context).args = glsl_expr_chain_create(); ;}
    break;

  case 21:

    { (yyval.call_context).args = glsl_expr_chain_append(glsl_expr_chain_create(), (yyvsp[(2) - (2)].expr)); ;}
    break;

  case 22:

    { (yyval.call_context).args = glsl_expr_chain_append((yyvsp[(1) - (3)].call_context).args, (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 23:

    { (yyval.call_context) = (yyvsp[(1) - (2)].call_context); ;}
    break;

  case 24:

    { (yyval.call_context) = (yyvsp[(1) - (1)].call_context); ;}
    break;

  case 25:

    { (yyval.call_context) = (yyvsp[(1) - (1)].call_context); ;}
    break;

  case 26:

    { (yyval.call_context).flavour = CALL_CONTEXT_FUNCTION; (yyval.call_context).u.function.symbol = (yyvsp[(1) - (1)].lookup).symbol; ;}
    break;

  case 27:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_TEXTURE_2D_BIAS; ;}
    break;

  case 28:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_TEXTURE_2D_LOD; ;}
    break;

  case 29:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_TEXTURE_CUBE_BIAS; ;}
    break;

  case 30:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_TEXTURE_CUBE_LOD; ;}
    break;

  case 31:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_RSQRT; ;}
    break;

  case 32:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_RCP; ;}
    break;

  case 33:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_LOG2; ;}
    break;

  case 34:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_EXP2; ;}
    break;

  case 35:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_CEIL; ;}
    break;

  case 36:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_FLOOR; ;}
    break;

  case 37:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_SIGN; ;}
    break;

  case 38:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_TRUNC; ;}
    break;

  case 39:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_NEAREST; ;}
    break;

  case 40:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_MIN; ;}
    break;

  case 41:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_MAX; ;}
    break;

  case 42:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_MINABS; ;}
    break;

  case 43:

    { (yyval.call_context).flavour = CALL_CONTEXT_INTRINSIC; (yyval.call_context).u.intrinsic.flavour = EXPR_INTRINSIC_MAXABS; ;}
    break;

  case 44:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_FLOAT; ;}
    break;

  case 45:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_INT; ;}
    break;

  case 46:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_BOOL; ;}
    break;

  case 47:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_VEC2; ;}
    break;

  case 48:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_VEC3; ;}
    break;

  case 49:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_VEC4; ;}
    break;

  case 50:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_BVEC2; ;}
    break;

  case 51:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_BVEC3; ;}
    break;

  case 52:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_BVEC4; ;}
    break;

  case 53:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_IVEC2; ;}
    break;

  case 54:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_IVEC3; ;}
    break;

  case 55:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_IVEC4; ;}
    break;

  case 56:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_MAT2; ;}
    break;

  case 57:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_MAT3; ;}
    break;

  case 58:

    { (yyval.call_context).flavour = CALL_CONTEXT_PRIM_CONSTRUCTOR; (yyval.call_context).u.prim_constructor.index = PRIM_MAT4; ;}
    break;

  case 59:

    { (yyval.call_context).flavour = CALL_CONTEXT_TYPE_CONSTRUCTOR; (yyval.call_context).u.type_constructor.symbol = (yyvsp[(1) - (1)].lookup).symbol; ;}
    break;

  case 60:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 61:

    { (yyval.expr) = glsl_expr_construct_unary_op_arithmetic(EXPR_PRE_INC, (yyvsp[(2) - (2)].expr)); ;}
    break;

  case 62:

    { (yyval.expr) = glsl_expr_construct_unary_op_arithmetic(EXPR_PRE_DEC, (yyvsp[(2) - (2)].expr)); ;}
    break;

  case 63:

    { (yyval.expr) = (yyvsp[(2) - (2)].expr); ;}
    break;

  case 64:

    { (yyval.expr) = glsl_expr_construct_unary_op_arithmetic(EXPR_ARITH_NEGATE, (yyvsp[(2) - (2)].expr)); ;}
    break;

  case 65:

    { (yyval.expr) = glsl_expr_construct_unary_op_logical(EXPR_LOGICAL_NOT, (yyvsp[(2) - (2)].expr)); ;}
    break;

  case 66:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 67:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 68:

    { (yyval.expr) = glsl_expr_construct_binary_op_arithmetic(EXPR_MUL, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 69:

    { (yyval.expr) = glsl_expr_construct_binary_op_arithmetic(EXPR_DIV, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 70:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 71:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 72:

    { (yyval.expr) = glsl_expr_construct_binary_op_arithmetic(EXPR_ADD, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 73:

    { (yyval.expr) = glsl_expr_construct_binary_op_arithmetic(EXPR_SUB, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 74:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 75:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 76:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 77:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 78:

    { (yyval.expr) = glsl_expr_construct_binary_op_relational(EXPR_LESS_THAN, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 79:

    { (yyval.expr) = glsl_expr_construct_binary_op_relational(EXPR_GREATER_THAN, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 80:

    { (yyval.expr) = glsl_expr_construct_binary_op_relational(EXPR_LESS_THAN_EQUAL, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 81:

    { (yyval.expr) = glsl_expr_construct_binary_op_relational(EXPR_GREATER_THAN_EQUAL, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 82:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 83:

    { (yyval.expr) = glsl_expr_construct_binary_op_equality(EXPR_EQUAL, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 84:

    { (yyval.expr) = glsl_expr_construct_binary_op_equality(EXPR_NOT_EQUAL, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 85:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 86:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 87:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 88:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 89:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 90:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 91:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 92:

    { (yyval.expr) = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_AND, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 93:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 94:

    { (yyval.expr) = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_XOR, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 95:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 96:

    { (yyval.expr) = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_OR, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 97:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 98:

    { (yyval.expr) = glsl_expr_construct_cond_op((yyvsp[(1) - (5)].expr), (yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].expr)); ;}
    break;

  case 99:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 100:

    { (yyval.expr) = glsl_expr_construct_assign_op((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 101:

    { (yyval.expr) = glsl_expr_construct_assign_op((yyvsp[(1) - (3)].expr), glsl_expr_construct_binary_op_arithmetic((yyvsp[(2) - (3)].expr_flavour), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr))); ;}
    break;

  case 102:

    { (yyval.expr_flavour) = EXPR_MUL; ;}
    break;

  case 103:

    { (yyval.expr_flavour) = EXPR_DIV; ;}
    break;

  case 104:

    { (yyval.expr_flavour) = EXPR_ADD; ;}
    break;

  case 105:

    { (yyval.expr_flavour) = EXPR_SUB; ;}
    break;

  case 106:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 107:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 108:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 109:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 110:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 111:

    { /* RESERVED */ glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, NULL); ;}
    break;

  case 112:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 113:

    { (yyval.expr) = glsl_expr_construct_sequence((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); ;}
    break;

  case 114:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 115:

    {
                                                                          if (!g_InGlobalScope)
                                                                             glsl_compile_error(ERROR_CUSTOM, 39, g_LineNumber, NULL);
                                                                          // Don't store these in the AST.
                                                                          (yyval.statement) = glsl_statement_construct_decl_list(glsl_statement_chain_create());
                                                                       ;}
    break;

  case 116:

    {
                                                                          (yyval.statement) = glsl_statement_construct_decl_list((yyvsp[(1) - (2)].statement_chain));
                                                                       ;}
    break;

  case 117:

    {
                                                                          // "Precision not supported" warning already issued.
                                                                          // Don't store these in the AST.
                                                                          (yyval.statement) = glsl_statement_construct_decl_list(glsl_statement_chain_create());
                                                                       ;}
    break;

  case 118:

    { glsl_build_function_type(); glsl_commit_singleton_function_declaration(g_FunctionBuilderName); (yyval.symbol) = g_LastInstance; ;}
    break;

  case 119:

    { /* Nothing to do here. */ ;}
    break;

  case 120:

    { /* Nothing to do here. */ ;}
    break;

  case 121:

    { /* Nothing to do here. */ ;}
    break;

  case 122:

    { /* Nothing to do here. */ ;}
    break;

  case 123:

    {
            g_FunctionBuilderReturnType = g_TypeBuilder;
            if (g_TypeQual != TYPE_QUAL_DEFAULT)
               glsl_compile_error(ERROR_CUSTOM, 40, g_LineNumber, NULL);
            g_FunctionBuilderName = (yyvsp[(2) - (3)].s);
            glsl_reinit_function_builder();
         ;}
    break;

  case 124:

    { glsl_compile_error(ERROR_SEMANTIC, 41, g_LineNumber, NULL); ;}
    break;

  case 125:

    { glsl_commit_singleton_function_param((yyvsp[(3) - (3)].s)); ;}
    break;

  case 126:

    { glsl_commit_array_function_param((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].expr)); ;}
    break;

  case 127:

    { glsl_commit_singleton_function_param(generate_anon_param()); ;}
    break;

  case 128:

    { glsl_commit_array_function_param(generate_anon_param(), (yyvsp[(4) - (5)].expr)); ;}
    break;

  case 129:

    { /* Nothing to do here. */ ;}
    break;

  case 130:

    { g_TypeQual = TYPE_QUAL_DEFAULT; ;}
    break;

  case 131:

    { g_ParamQual = PARAM_QUAL_DEFAULT; ;}
    break;

  case 132:

    { g_ParamQual = PARAM_QUAL_IN; ;}
    break;

  case 133:

    { g_ParamQual = PARAM_QUAL_OUT; ;}
    break;

  case 134:

    { g_ParamQual = PARAM_QUAL_INOUT; ;}
    break;

  case 135:

    {
                                                                                                            (yyval.statement_chain) = (yyvsp[(1) - (1)].statement_chain);
                                                                                                         ;}
    break;

  case 136:

    {
                                                                                                            glsl_commit_singleton_variable_instance((yyvsp[(3) - (3)].s), NULL);
                                                                                                            (yyval.statement_chain) = glsl_statement_chain_append((yyvsp[(1) - (3)].statement_chain), glsl_statement_construct_var_decl(g_LastInstance, NULL));
                                                                                                         ;}
    break;

  case 137:

    {
                                                                                                            glsl_commit_array_instance((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].expr));
                                                                                                            (yyval.statement_chain) = glsl_statement_chain_append((yyvsp[(1) - (6)].statement_chain), glsl_statement_construct_var_decl(g_LastInstance, NULL));
                                                                                                         ;}
    break;

  case 138:

    {
                                                                                                            glsl_commit_singleton_variable_instance((yyvsp[(3) - (5)].s), (yyvsp[(5) - (5)].expr));
                                                                                                            (yyval.statement_chain) = glsl_statement_chain_append((yyvsp[(1) - (5)].statement_chain), glsl_statement_construct_var_decl(g_LastInstance, (yyvsp[(5) - (5)].expr)));
                                                                                                         ;}
    break;

  case 139:

    {
                                                                                                      // This is to match struct declarations, but unfortunately it also admits rubbish like "int , x".
                                                                                                      (yyval.statement_chain) = glsl_statement_chain_create();
                                                                                                   ;}
    break;

  case 140:

    {
                                                                                                      glsl_commit_singleton_variable_instance((yyvsp[(2) - (2)].s), NULL);
                                                                                                      (yyval.statement_chain) = glsl_statement_chain_append(glsl_statement_chain_create(), glsl_statement_construct_var_decl(g_LastInstance, NULL));
                                                                                                   ;}
    break;

  case 141:

    {
                                                                                                      glsl_commit_array_instance((yyvsp[(2) - (5)].s), (yyvsp[(4) - (5)].expr));
                                                                                                      (yyval.statement_chain) = glsl_statement_chain_append(glsl_statement_chain_create(), glsl_statement_construct_var_decl(g_LastInstance, NULL));
                                                                                                   ;}
    break;

  case 142:

    {
                                                                                                      glsl_commit_singleton_variable_instance((yyvsp[(2) - (4)].s), (yyvsp[(4) - (4)].expr));
                                                                                                      (yyval.statement_chain) = glsl_statement_chain_append(glsl_statement_chain_create(), glsl_statement_construct_var_decl(g_LastInstance, (yyvsp[(4) - (4)].expr)));
                                                                                                   ;}
    break;

  case 143:

    {
                                                                                                      (yyval.statement_chain) = glsl_statement_chain_create();
                                                                                                   ;}
    break;

  case 144:

    { g_TypeQual = TYPE_QUAL_DEFAULT; ;}
    break;

  case 146:

    { g_TypeQual = TYPE_QUAL_CONST; ;}
    break;

  case 147:

    { g_TypeQual = TYPE_QUAL_ATTRIBUTE; ;}
    break;

  case 148:

    { g_TypeQual = TYPE_QUAL_VARYING; ;}
    break;

  case 149:

    { g_TypeQual = TYPE_QUAL_INVARIANT_VARYING; ;}
    break;

  case 150:

    { g_TypeQual = TYPE_QUAL_UNIFORM; ;}
    break;

  case 153:

    { g_TypeBuilder = &primitiveTypes[PRIM_VOID]; ;}
    break;

  case 154:

    { g_TypeBuilder = &primitiveTypes[PRIM_FLOAT]; ;}
    break;

  case 155:

    { g_TypeBuilder = &primitiveTypes[PRIM_INT]; ;}
    break;

  case 156:

    { g_TypeBuilder = &primitiveTypes[PRIM_BOOL]; ;}
    break;

  case 157:

    { g_TypeBuilder = &primitiveTypes[PRIM_VEC2]; ;}
    break;

  case 158:

    { g_TypeBuilder = &primitiveTypes[PRIM_VEC3]; ;}
    break;

  case 159:

    { g_TypeBuilder = &primitiveTypes[PRIM_VEC4]; ;}
    break;

  case 160:

    { g_TypeBuilder = &primitiveTypes[PRIM_BVEC2]; ;}
    break;

  case 161:

    { g_TypeBuilder = &primitiveTypes[PRIM_BVEC3]; ;}
    break;

  case 162:

    { g_TypeBuilder = &primitiveTypes[PRIM_BVEC4]; ;}
    break;

  case 163:

    { g_TypeBuilder = &primitiveTypes[PRIM_IVEC2]; ;}
    break;

  case 164:

    { g_TypeBuilder = &primitiveTypes[PRIM_IVEC3]; ;}
    break;

  case 165:

    { g_TypeBuilder = &primitiveTypes[PRIM_IVEC4]; ;}
    break;

  case 166:

    { g_TypeBuilder = &primitiveTypes[PRIM_MAT2]; ;}
    break;

  case 167:

    { g_TypeBuilder = &primitiveTypes[PRIM_MAT3]; ;}
    break;

  case 168:

    { g_TypeBuilder = &primitiveTypes[PRIM_MAT4]; ;}
    break;

  case 169:

    { g_TypeBuilder = &primitiveTypes[PRIM_SAMPLER2D]; ;}
    break;

  case 170:

    {
        switch (glsl_ext_status(GLSL_EXT_IMAGE_EXTERNAL)) {
        case GLSL_ENABLED_WARN:
            glsl_compile_error(WARNING, 2, g_LineNumber, NULL);
        case GLSL_ENABLED:
            g_TypeBuilder = &primitiveTypes[PRIM_SAMPLEREXTERNAL];
            break;
        case GLSL_DISABLED:
            glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, NULL);
            break;
        }
      ;}
    break;

  case 171:

    { g_TypeBuilder = &primitiveTypes[PRIM_SAMPLERCUBE]; ;}
    break;

  case 172:

    { /* g_TypeBuilder already set. */ ;}
    break;

  case 173:

    { g_TypeBuilder = (yyvsp[(1) - (1)].lookup).symbol->type; ;}
    break;

  case 174:

    {  ;}
    break;

  case 175:

    {  ;}
    break;

  case 176:

    {  ;}
    break;

  case 177:

    { glsl_reinit_struct_builder(); ;}
    break;

  case 178:

    { glsl_build_struct_type(); glsl_commit_struct_type((yyvsp[(3) - (6)].s)); ;}
    break;

  case 179:

    { glsl_reinit_struct_builder(); ;}
    break;

  case 180:

    { glsl_build_struct_type(); ;}
    break;

  case 185:

    { glsl_commit_singleton_struct_member((yyvsp[(3) - (3)].s)); ;}
    break;

  case 186:

    { glsl_commit_array_struct_member((yyvsp[(3) - (6)].s), (yyvsp[(5) - (6)].expr)); ;}
    break;

  case 187:

    { glsl_commit_singleton_struct_member((yyvsp[(2) - (2)].s)); ;}
    break;

  case 188:

    { glsl_commit_array_struct_member((yyvsp[(2) - (5)].s), (yyvsp[(4) - (5)].expr)); ;}
    break;

  case 189:

    { (yyval.expr) = (yyvsp[(1) - (1)].expr); ;}
    break;

  case 190:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 191:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 192:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 193:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 194:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 195:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 196:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 197:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 198:

    { (yyval.statement) = glsl_statement_construct_compound(glsl_statement_chain_create()); ;}
    break;

  case 199:

    { glsl_enter_scope(); ;}
    break;

  case 200:

    { glsl_exit_scope(); (yyval.statement) = glsl_statement_construct_compound((yyvsp[(3) - (4)].statement_chain)); ;}
    break;

  case 201:

    { glsl_enter_scope(); ;}
    break;

  case 202:

    { (yyval.statement) = (yyvsp[(2) - (2)].statement); glsl_exit_scope(); ;}
    break;

  case 203:

    { glsl_enter_scope(); ;}
    break;

  case 204:

    { (yyval.statement) = (yyvsp[(2) - (2)].statement); glsl_exit_scope(); ;}
    break;

  case 205:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 206:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 207:

    { (yyval.statement) = glsl_statement_construct_compound(glsl_statement_chain_create()); ;}
    break;

  case 208:

    { (yyval.statement) = glsl_statement_construct_compound((yyvsp[(2) - (3)].statement_chain)); ;}
    break;

  case 209:

    { (yyval.statement_chain) = glsl_statement_chain_append(glsl_statement_chain_create(), (yyvsp[(1) - (1)].statement)); ;}
    break;

  case 210:

    { (yyval.statement_chain) = glsl_statement_chain_append((yyvsp[(1) - (2)].statement_chain), (yyvsp[(2) - (2)].statement)); ;}
    break;

  case 211:

    { (yyval.statement) = glsl_statement_construct(STATEMENT_NULL); ;}
    break;

  case 212:

    { (yyval.statement) = glsl_statement_construct_expr((yyvsp[(1) - (2)].expr)); ;}
    break;

  case 213:

    { (yyval.statement) = glsl_statement_construct_selection((yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].statement2).a, (yyvsp[(5) - (5)].statement2).b); ;}
    break;

  case 214:

    { (yyval.statement2).a = (yyvsp[(1) - (3)].statement); (yyval.statement2).b = (yyvsp[(3) - (3)].statement); ;}
    break;

  case 215:

    { (yyval.statement2).a = (yyvsp[(1) - (1)].statement); (yyval.statement2).b = NULL; ;}
    break;

  case 216:

    { (yyval.statement) = glsl_statement_construct_expr((yyvsp[(1) - (1)].expr)); ;}
    break;

  case 217:

    { glsl_commit_singleton_variable_instance((yyvsp[(2) - (4)].s), (yyvsp[(4) - (4)].expr)); (yyval.statement) = glsl_statement_construct_var_decl(g_LastInstance, (yyvsp[(4) - (4)].expr)); ;}
    break;

  case 218:

    { glsl_enter_scope(); ;}
    break;

  case 219:

    { enter_loop(); ;}
    break;

  case 220:

    { exit_loop(); glsl_exit_scope(); (yyval.statement) = glsl_statement_construct_iterator_while((yyvsp[(4) - (7)].statement), (yyvsp[(7) - (7)].statement)); ;}
    break;

  case 221:

    { enter_loop(); ;}
    break;

  case 222:

    { exit_loop(); ;}
    break;

  case 223:

    { (yyval.statement) = glsl_statement_construct_iterator_do_while((yyvsp[(3) - (9)].statement), (yyvsp[(7) - (9)].expr)); ;}
    break;

  case 224:

    { glsl_enter_scope(); ;}
    break;

  case 225:

    { enter_loop(); ;}
    break;

  case 226:

    { exit_loop(); glsl_exit_scope(); (yyval.statement) = glsl_statement_construct_iterator_for((yyvsp[(4) - (11)].statement), (yyvsp[(6) - (11)].statement), (yyvsp[(8) - (11)].expr), (yyvsp[(11) - (11)].statement)); ;}
    break;

  case 229:

    { g_TypeBuilder = &primitiveTypes[PRIM_FLOAT]; ;}
    break;

  case 230:

    { g_TypeBuilder = &primitiveTypes[PRIM_INT]; ;}
    break;

  case 231:

    {
                                                                             g_TypeQual = TYPE_QUAL_LOOP_INDEX;

                                                                             glsl_commit_singleton_variable_instance((yyvsp[(2) - (4)].s), (yyvsp[(4) - (4)].expr));
                                                                             (yyval.statement) = glsl_statement_construct_var_decl(g_LastInstance, (yyvsp[(4) - (4)].expr));
                                                                          ;}
    break;

  case 232:

    { (yyval.statement) = glsl_statement_construct_expr((yyvsp[(1) - (1)].expr)); ;}
    break;

  case 233:

    { (yyval.statement) = NULL; ;}
    break;

  case 235:

    { (yyval.statement) = NULL; ;}
    break;

  case 237:

    { (yyval.expr) = NULL; ;}
    break;

  case 238:

    { if (!in_loop())
                                         glsl_compile_error(ERROR_CUSTOM, 29, g_LineNumber, NULL);
                                      (yyval.statement) = glsl_statement_construct(STATEMENT_CONTINUE); ;}
    break;

  case 239:

    { if (!in_loop())
                                         glsl_compile_error(ERROR_CUSTOM, 29, g_LineNumber, NULL);
                                      (yyval.statement) = glsl_statement_construct(STATEMENT_BREAK); ;}
    break;

  case 240:

    { (yyval.statement) = glsl_statement_construct(STATEMENT_RETURN); ;}
    break;

  case 241:

    { (yyval.statement) = glsl_statement_construct_return_expr((yyvsp[(2) - (3)].expr)); ;}
    break;

  case 242:

    { if (SHADER_FRAGMENT != g_ShaderFlavour) glsl_compile_error(ERROR_CUSTOM, 12, g_LineNumber, NULL); (yyval.statement) = glsl_statement_construct(STATEMENT_DISCARD); ;}
    break;

  case 243:

    {
                                                 StatementChain* chain = glsl_statement_chain_create();
                                                 glsl_statement_chain_append(chain, (yyvsp[(1) - (1)].statement));
                                                 (yyval.statement) = glsl_statement_construct_ast(chain);
                                                 *(Statement**)top_level_statement = (yyval.statement); // Save for calling function.
                                              ;}
    break;

  case 244:

    {
                                                 StatementChain* chain = (yyvsp[(1) - (2)].statement)->u.ast.decls;
                                                 glsl_statement_chain_append(chain, (yyvsp[(2) - (2)].statement));
                                                 (yyval.statement) = (yyvsp[(1) - (2)].statement);
                                                 *(Statement**)top_level_statement = (yyval.statement); // Save for calling function.
                                              ;}
    break;

  case 245:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 246:

    { (yyval.statement) = (yyvsp[(1) - (1)].statement); ;}
    break;

  case 247:

    { glsl_enter_scope(); g_InGlobalScope = false; glsl_instantiate_function_params(g_TypeBuilder); ;}
    break;

  case 248:

    { glsl_exit_scope(); g_InGlobalScope = true; (yyval.statement) = glsl_statement_construct_function_def((yyvsp[(1) - (3)].symbol), (yyvsp[(3) - (3)].statement)); glsl_insert_function_definition((yyval.statement)); ;}
    break;

  case 249:

    { (yyval.s) = (yyvsp[(1) - (1)].lookup).name; ;}
    break;

  case 250:

    { (yyval.s) = (yyvsp[(1) - (1)].lookup).name; ;}
    break;



      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
   YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
   if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
     {
       YYSIZE_T yyalloc = 2 * yysize;
       if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
         yyalloc = YYSTACK_ALLOC_MAXIMUM;
       if (yymsg != yymsgbuf)
         YYSTACK_FREE (yymsg);
       yymsg = (char *) YYSTACK_ALLOC (yyalloc);
       if (yymsg)
         yymsg_alloc = yyalloc;
       else
         {
      yymsg = yymsgbuf;
      yymsg_alloc = sizeof yymsgbuf;
         }
     }

   if (0 < yysize && yysize <= yymsg_alloc)
     {
       (void) yysyntax_error (yymsg, yystate, yychar);
       yyerror (yymsg);
     }
   else
     {
       yyerror (YY_("syntax error"));
       if (yysize != 0)
         goto yyexhaustedlab;
     }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
    error, discard it.  */

      if (yychar <= YYEOF)
   {
     /* Return failure if at end of input.  */
     if (yychar == YYEOF)
       YYABORT;
   }
      else
   {
     yydestruct ("Error: discarding",
            yytoken, &yylval);
     yychar = YYEMPTY;
   }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;   /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
   {
     yyn += YYTERROR;
     if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
       {
         yyn = yytable[yyn];
         if (0 < yyn)
      break;
       }
   }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
   YYABORT;


      yydestruct ("Error: popping",
        yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
       yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
        yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}
