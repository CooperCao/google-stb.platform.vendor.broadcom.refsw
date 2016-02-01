
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

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

extern YYSTYPE yylval;
