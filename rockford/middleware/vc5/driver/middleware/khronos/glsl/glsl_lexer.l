%{
   // build me with "flex -L -o glsl_lexer.c --never-interactive glsl_lexer.l"
   // and delete the bogus # line directive at the start
   // --never-interactive is to avoid isatty declaration issues with c++

   #include "glsl_common.h"
   #include "glsl_errors.h"
   #include "glsl_intern.h"
   #include "glsl_intrinsic.h"
   #include "glsl_mem_utils.h"
   #include "glsl_symbol_table.h"

   #include "prepro/glsl_prepro_directive.h"
   #include "prepro/glsl_prepro_token.h"

   #include "glsl_globals.h"
   #include "glsl_stdlib.auto.h"
   #include "glsl_extensions.h"

   #include <stdlib.h>
   #include <string.h>

   TokenData pptoken;

   static inline int handle_keyword(TokenType token, KeywordFlags flags);
   static inline int handle_extension_keyword(TokenType token, enum glsl_ext ext);
   static int yyinput(char *buf, int buf_size);

   #undef YY_BUF_SIZE
   #define YY_BUF_SIZE 256
   #define YY_INPUT(buf,result,max_size) { result = yyinput(buf,(int)max_size); }

   #define GLSL_ES_1_NONE 0
   #define GLSL_ES_3_NONE 0
   #define GLSL_ES_31_NONE 0
   #define GLSL_ES_32_NONE 0
   #define KW_FLAGS(ES1, ES3, ES31, ES32)  (GLSL_ES_1_ ## ES1 | GLSL_ES_3_ ## ES3 | GLSL_ES_31_ ## ES31 | GLSL_ES_32_ ## ES32)
%}

%option prefix="pp"
%option never-interactive
%option batch
%option nounistd
%option nounput
%option noinput

IDENT    [_a-zA-Z][_a-zA-Z0-9]*
	// reserved keywords (including identifiers that contain '__') are checked for in yylex() as implemented in glsl_parser.y
DIGIT    [0-9]
EXP      [eE][-+]
DOT      "."
NONDIG   [_a-zA-Z]

%x comment

%%
%{
   BEGIN INITIAL;
%}

   /*
      tokens for the preprocessor

      HASH
      WHITESPACE
      NEWLINE

      DEFINE
      UNDEF
      IFDEF
      IFNDEF
      ELIF
      ENDIF
      ERROR
      PRAGMA
      EXTENSION
      VERSION
      LINE

      ALL
      REQUIRE
      ENABLE
      WARN
      DISABLE

      PPNUMBER
      BACKSLASH
   */

"#"                           return HASH;
[ \t]+                        return WHITESPACE;
\n                            {
                                 g_LineNumber++;
                                 return NEWLINE;
                              }
\r                            ;
\\                            return BACKSLASH;

"define"                      return handle_keyword(DEFINE, PREPROC_KEYWORD);
"undef"                       return handle_keyword(UNDEF, PREPROC_KEYWORD);
"ifdef"                       return handle_keyword(IFDEF, PREPROC_KEYWORD);
"ifndef"                      return handle_keyword(IFNDEF, PREPROC_KEYWORD);
"elif"                        return handle_keyword(ELIF, PREPROC_KEYWORD);
"endif"                       return handle_keyword(ENDIF, PREPROC_KEYWORD);
"error"                       return handle_keyword(ERROR, PREPROC_KEYWORD);
"pragma"                      return handle_keyword(PRAGMA, PREPROC_KEYWORD);
"extension"                   return handle_keyword(EXTENSION, PREPROC_KEYWORD);
"version"                     return handle_keyword(VERSION, PREPROC_KEYWORD);
"line"                        return handle_keyword(LINE, PREPROC_KEYWORD);
"all"                         return handle_keyword(ALL, PREPROC_KEYWORD);
"require"                     return handle_keyword(REQUIRE, PREPROC_KEYWORD);
"enable"                      return handle_keyword(ENABLE, PREPROC_KEYWORD);
"warn"                        return handle_keyword(WARN, PREPROC_KEYWORD);
"disable"                     return handle_keyword(DISABLE, PREPROC_KEYWORD);

   /*
      list of keywords from GLSL ES Specification 1.0.17 and GLSL ES Specification 3.00.4

      Note that it is ok to pass reserved keywords to the preprocessor.
      If they do not reach the parser, it is not an error.

      keywords "true" and "false" are lexed as BOOLCONSTANT
   */

"active"                      return handle_keyword(ACTIVE,            KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"asm"                         return handle_keyword(ASM,               KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"atomic_uint"                 return handle_keyword(ATOMIC_UINT,       KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"attribute"                   return handle_keyword(ATTRIBUTE,         KW_FLAGS(KEYWORD,  RESERVED, RESERVED, RESERVED));
"bool"                        return handle_keyword(BOOL,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"break"                       return handle_keyword(BREAK,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"buffer"                      return handle_keyword(BUFFER,            KW_FLAGS(NONE,     NONE,     KEYWORD,  KEYWORD));
"bvec2"                       return handle_keyword(BVEC2,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"bvec3"                       return handle_keyword(BVEC3,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"bvec4"                       return handle_keyword(BVEC4,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"case"                        return handle_keyword(CASE,              KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"cast"                        return handle_keyword(CAST,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"centroid"                    return handle_keyword(CENTROID,          KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"class"                       return handle_keyword(CLASS,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"coherent"                    return handle_keyword(COHERENT,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"common"                      return handle_keyword(COMMON,            KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"const"                       return handle_keyword(CONST,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"continue"                    return handle_keyword(CONTINUE,          KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"default"                     return handle_keyword(DEFAULT,           KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"discard"                     return handle_keyword(DISCARD,           KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"dmat2"                       return handle_keyword(DMAT2,             KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x2"                     return handle_keyword(DMAT2X2,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x3"                     return handle_keyword(DMAT2X3,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x4"                     return handle_keyword(DMAT2X4,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3"                       return handle_keyword(DMAT3,             KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x2"                     return handle_keyword(DMAT3X2,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x3"                     return handle_keyword(DMAT3X3,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x4"                     return handle_keyword(DMAT3X4,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4"                       return handle_keyword(DMAT4,             KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x2"                     return handle_keyword(DMAT4X2,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x3"                     return handle_keyword(DMAT4X3,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x4"                     return handle_keyword(DMAT4X4,           KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"do"                          return handle_keyword(DO,                KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"double"                      return handle_keyword(DOUBLE,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec2"                       return handle_keyword(DVEC2,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec3"                       return handle_keyword(DVEC3,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec4"                       return handle_keyword(DVEC4,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"else"                        return handle_keyword(ELSE,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"enum"                        return handle_keyword(ENUM,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"extern"                      return handle_keyword(EXTERN,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"external"                    return handle_keyword(EXTERNAL,          KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"filter"                      return handle_keyword(FILTER,            KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"fixed"                       return handle_keyword(FIXED,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"flat"                        return handle_keyword(FLAT,              KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"float"                       return handle_keyword(FLOAT,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"for"                         return handle_keyword(FOR,               KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"fvec2"                       return handle_keyword(FVEC2,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"fvec3"                       return handle_keyword(FVEC3,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"fvec4"                       return handle_keyword(FVEC4,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"goto"                        return handle_keyword(GOTO,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"half"                        return handle_keyword(HALF,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"highp"                       return handle_keyword(HIGH_PRECISION,    KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"hvec2"                       return handle_keyword(HVEC2,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"hvec3"                       return handle_keyword(HVEC3,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"hvec4"                       return handle_keyword(HVEC4,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"if"                          return handle_keyword(IF,                KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"iimage1D"                    return handle_keyword(IIMAGE1D,          KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"iimage1DArray"               return handle_keyword(IIMAGE1DARRAY,     KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"iimage2D"                    return handle_keyword(IIMAGE2D,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimage2DArray"               return handle_keyword(IIMAGE2DARRAY,     KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimage2DMS"                  return handle_keyword(IIMAGE2DMS,        KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage2DMSArray"             return handle_keyword(IIMAGE2DMSARRAY,   KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage2DRect"                return handle_keyword(IIMAGE2DRECT,      KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage3D"                    return handle_keyword(IIMAGE3D,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimageBuffer"                return handle_keyword(IIMAGEBUFFER,      KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"iimageCube"                  return handle_keyword(IIMAGECUBE,        KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimageCubeArray"             return handle_keyword(IIMAGECUBEARRAY,   KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"image1D"                     return handle_keyword(IMAGE1D,           KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"image1DArray"                return handle_keyword(IMAGE1DARRAY,      KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"image1DArrayShadow"          return handle_keyword(IMAGE1DARRAYSHADOW, KW_FLAGS(NONE,    RESERVED, NONE,     NONE));
"image1DShadow"               return handle_keyword(IMAGE1DSHADOW,     KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image2D"                     return handle_keyword(IMAGE2D,           KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"image2DArray"                return handle_keyword(IMAGE2DARRAY,      KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"image2DMS"                   return handle_keyword(IMAGE2DMS,         KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DMSArray"              return handle_keyword(IMAGE2DMSARRAY,    KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DRect"                 return handle_keyword(IMAGE2DRECT,       KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DArrayShadow"          return handle_keyword(IMAGE2DARRAYSHADOW, KW_FLAGS(NONE,    RESERVED, NONE,     NONE));
"image2DShadow"               return handle_keyword(IMAGE2DSHADOW,     KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image3D"                     return handle_keyword(IMAGE3D,           KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"imageBuffer"                 return handle_keyword(IMAGEBUFFER,       KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"imageCube"                   return handle_keyword(IMAGECUBE,         KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"imageCubeArray"              return handle_keyword(IMAGECUBEARRAY,    KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"in"                          return handle_keyword(IN,                KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"inline"                      return handle_keyword(_INLINE,           KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"inout"                       return handle_keyword(INOUT,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"input"                       return handle_keyword(INPUT,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"int"                         return handle_keyword(INT,               KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"interface"                   return handle_keyword(INTERFACE,         KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"invariant"                   return handle_keyword(INVARIANT,         KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"isampler1D"                  return handle_keyword(ISAMPLER1D,        KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"isampler1DArray"             return handle_keyword(ISAMPLER1DARRAY,   KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"isampler2D"                  return handle_keyword(ISAMPLER2D,        KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isampler2DArray"             return handle_keyword(ISAMPLER2DARRAY,   KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isampler2DMS"                return handle_keyword(ISAMPLER2DMS,      KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"isampler2DMSArray"           return handle_keyword(ISAMPLER2DMSARRAY, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"isampler2DRect"              return handle_keyword(ISAMPLER2DRECT,    KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"isampler3D"                  return handle_keyword(ISAMPLER3D,        KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isamplerBuffer"              return handle_keyword(ISAMPLERBUFFER,    KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"isamplerCube"                return handle_keyword(ISAMPLERCUBE,      KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isamplerCubeArray"           return handle_keyword(ISAMPLERCUBEARRAY, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"ivec2"                       return handle_keyword(IVEC2,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"ivec3"                       return handle_keyword(IVEC3,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"ivec4"                       return handle_keyword(IVEC4,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"layout"                      return handle_keyword(LAYOUT,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"long"                        return handle_keyword(LONG,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"lowp"                        return handle_keyword(LOW_PRECISION,     KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat2"                        return handle_keyword(MAT2,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat2x2"                      return handle_keyword(MAT2X2,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat2x3"                      return handle_keyword(MAT2X3,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat2x4"                      return handle_keyword(MAT2X4,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3"                        return handle_keyword(MAT3,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat3x2"                      return handle_keyword(MAT3X2,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3x3"                      return handle_keyword(MAT3X3,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3x4"                      return handle_keyword(MAT3X4,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4"                        return handle_keyword(MAT4,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat4x2"                      return handle_keyword(MAT4X2,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4x3"                      return handle_keyword(MAT4X3,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4x4"                      return handle_keyword(MAT4X4,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mediump"                     return handle_keyword(MEDIUM_PRECISION,  KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"namespace"                   return handle_keyword(NAMESPACE,         KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"noinline"                    return handle_keyword(NOINLINE,          KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"noperspective"               return handle_keyword(NOPERSPECTIVE,     KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"out"                         return handle_keyword(OUT,               KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"output"                      return handle_keyword(OUTPUT,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"packed"                      return handle_keyword(PACKED,            KW_FLAGS(RESERVED, NONE,     NONE,     NONE));
"partition"                   return handle_keyword(PARTITION,         KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"patch"                       return handle_keyword(PATCH,             KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"precise"                     return handle_keyword(PRECISE,           KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"precision"                   return handle_keyword(PRECISION,         KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"public"                      return handle_keyword(PUBLIC,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"readonly"                    return handle_keyword(READONLY,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"resource"                    return handle_keyword(RESOURCE,          KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"restrict"                    return handle_keyword(RESTRICT,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"return"                      return handle_keyword(RETURN,            KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"sample"                      return handle_keyword(SAMPLE,            KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"sampler1D"                   return handle_keyword(SAMPLER1D,         KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler1DArray"              return handle_keyword(SAMPLER1DARRAY,    KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"sampler1DArrayShadow"        return handle_keyword(SAMPLER1DARRAYSHADOW, KW_FLAGS(NONE,  RESERVED, RESERVED, RESERVED));
"sampler1DShadow"             return handle_keyword(SAMPLER1DSHADOW,   KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2D"                   return handle_keyword(SAMPLER2D,         KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DArray"              return handle_keyword(SAMPLER2DARRAY,    KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DArrayShadow"        return handle_keyword(SAMPLER2DARRAYSHADOW, KW_FLAGS(NONE,  KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DMS"                 return handle_keyword(SAMPLER2DMS,       KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"sampler2DMSArray"            return handle_keyword(SAMPLER2DMSARRAY,  KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"sampler2DRect"               return handle_keyword(SAMPLER2DRECT,     KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2DRectShadow"         return handle_keyword(SAMPLER2DRECTSHADOW, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2DShadow"             return handle_keyword(SAMPLER2DSHADOW,   KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"sampler3D"                   return handle_keyword(SAMPLER3D,         KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"sampler3DRect"               return handle_keyword(SAMPLER3DRECT,     KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"samplerBuffer"               return handle_keyword(SAMPLERBUFFER,     KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"samplerCube"                 return handle_keyword(SAMPLERCUBE,       KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"samplerCubeArray"            return handle_keyword(SAMPLERCUBEARRAY,  KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"samplerCubeArrayShadow"      return handle_keyword(SAMPLERCUBEARRAYSHADOW, KW_FLAGS(NONE,NONE,     RESERVED, KEYWORD));
"samplerCubeShadow"           return handle_keyword(SAMPLERCUBESHADOW, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"shared"                      return handle_keyword(SHARED,            KW_FLAGS(NONE,     NONE,     KEYWORD,  KEYWORD));
"short"                       return handle_keyword(SHORT,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sizeof"                      return handle_keyword(SIZEOF,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"smooth"                      return handle_keyword(SMOOTH,            KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"static"                      return handle_keyword(STATIC,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"struct"                      return handle_keyword(STRUCT,            KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"subroutine"                  return handle_keyword(SUBROUTINE,        KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"superp"                      return handle_keyword(SUPERP,            KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"switch"                      return handle_keyword(SWITCH,            KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"template"                    return handle_keyword(TEMPLATE,          KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"this"                        return handle_keyword(THIS,              KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"typedef"                     return handle_keyword(TYPEDEF,           KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"uimage1D"                    return handle_keyword(UIMAGE1D,          KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"uimage1DArray"               return handle_keyword(UIMAGE1DARRAY,     KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"uimage2D"                    return handle_keyword(UIMAGE2D,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimage2DArray"               return handle_keyword(UIMAGE2DARRAY,     KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimage2DMS"                  return handle_keyword(UIMAGE2DMS,        KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage2DMSArray"             return handle_keyword(UIMAGE2DMSARRAY,   KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage2DRect"                return handle_keyword(UIMAGE2DRECT,      KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage3D"                    return handle_keyword(UIMAGE3D,          KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimageBuffer"                return handle_keyword(UIMAGEBUFFER,      KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"uimageCube"                  return handle_keyword(UIMAGECUBE,        KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimageCubeArray"             return handle_keyword(UIMAGECUBEARRAY,   KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"uint"                        return handle_keyword(UINT,              KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uniform"                     return handle_keyword(UNIFORM,           KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"union"                       return handle_keyword(UNION,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"unsigned"                    return handle_keyword(UNSIGNED,          KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"usampler1D"                  return handle_keyword(USAMPLER1D,        KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"usampler1DArray"             return handle_keyword(USAMPLER1DARRAY,   KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"usampler2D"                  return handle_keyword(USAMPLER2D,        KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usampler2DArray"             return handle_keyword(USAMPLER2DARRAY,   KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usampler2DMS"                return handle_keyword(USAMPLER2DMS,      KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"usampler2DMSArray"           return handle_keyword(USAMPLER2DMSARRAY, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"usampler2DRect"              return handle_keyword(USAMPLER2DRECT,    KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"usampler3D"                  return handle_keyword(USAMPLER3D,        KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usamplerBuffer"              return handle_keyword(USAMPLERBUFFER,    KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD));
"usamplerCube"                return handle_keyword(USAMPLERCUBE,      KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usamplerCubeArray"           return handle_keyword(USAMPLERCUBEARRAY, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD));
"using"                       return handle_keyword(USING,             KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"uvec2"                       return handle_keyword(UVEC2,             KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uvec3"                       return handle_keyword(UVEC3,             KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uvec4"                       return handle_keyword(UVEC4,             KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"varying"                     return handle_keyword(VARYING,           KW_FLAGS(KEYWORD,  RESERVED, RESERVED, RESERVED));
"vec2"                        return handle_keyword(VEC2,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"vec3"                        return handle_keyword(VEC3,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"vec4"                        return handle_keyword(VEC4,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"void"                        return handle_keyword(VOID,              KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"volatile"                    return handle_keyword(VOLATILE,          KW_FLAGS(RESERVED, RESERVED, KEYWORD,  KEYWORD));
"while"                       return handle_keyword(WHILE,             KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"writeonly"                   return handle_keyword(WRITEONLY,         KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));

   /*
      keywords for extensions
   */

"sampler1DBRCM"               return handle_extension_keyword(SAMPLER1D,       GLSL_EXT_BRCM_TEXTURE_1D);
"sampler1DArrayBRCM"          return handle_extension_keyword(SAMPLER1DARRAY,  GLSL_EXT_BRCM_TEXTURE_1D);
"isampler1DBRCM"              return handle_extension_keyword(ISAMPLER1D,      GLSL_EXT_BRCM_TEXTURE_1D);
"isampler1DArrayBRCM"         return handle_extension_keyword(ISAMPLER1DARRAY, GLSL_EXT_BRCM_TEXTURE_1D);
"usampler1DBRCM"              return handle_extension_keyword(USAMPLER1D,      GLSL_EXT_BRCM_TEXTURE_1D);
"usampler1DArrayBRCM"         return handle_extension_keyword(USAMPLER1DARRAY, GLSL_EXT_BRCM_TEXTURE_1D);
"samplerExternalOES"          return handle_extension_keyword(SAMPLEREXTERNAL, GLSL_EXT_IMAGE_EXTERNAL);

   /*
      tokens for the shading language itself (punctuators)

      LEFT_OP RIGHT_OP
      INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP
      LOGICAL_AND_OP LOGICAL_OR_OP LOGICAL_XOR_OP MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN
      MOD_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
      SUB_ASSIGN
      LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE DOT
      COMMA COLON EQUAL SEMICOLON BANG DASH TILDE PLUS STAR SLASH PERCENT
      LEFT_ANGLE RIGHT_ANGLE BITWISE_OR_OP BITWISE_XOR_OP BITWISE_AND_OP QUESTION
   */

"<<"           return LEFT_OP;
">>"           return RIGHT_OP;
"++"           return INC_OP;
"--"           return DEC_OP;
"<="           return LE_OP;
">="           return GE_OP;
"=="           return EQ_OP;
"!="           return NE_OP;
"&&"           return AND_OP;
"||"           return OR_OP;
"^^"           return XOR_OP;
"*="           return MUL_ASSIGN;
"/="           return DIV_ASSIGN;
"+="           return ADD_ASSIGN;
"%="           return MOD_ASSIGN;
"<<="          return LEFT_ASSIGN;
">>="          return RIGHT_ASSIGN;
"&="           return AND_ASSIGN;
"^="           return XOR_ASSIGN;
"|="           return OR_ASSIGN;
"-="           return SUB_ASSIGN;
"("            return LEFT_PAREN;
")"            return RIGHT_PAREN;
"["            return LEFT_BRACKET;
"]"            return RIGHT_BRACKET;
"{"            return LEFT_BRACE;
"}"            return RIGHT_BRACE;
"."            return DOT;
","            return COMMA;
":"            return COLON;
"="            return EQUAL;
";"            return SEMICOLON;
"!"            return BANG;
"-"            return DASH;
"~"            return TILDE;
"+"            return PLUS;
"*"            return STAR;
"/"            return SLASH;
"%"            return PERCENT;
"<"            return LEFT_ANGLE;
">"            return RIGHT_ANGLE;
"|"            return VERTICAL_BAR;
"^"            return CARET;
"&"            return AMPERSAND;
"?"            return QUESTION;

   /*
      intrinsics
   */

"$$"{IDENT}    { pptoken.s = glsl_intern(pptext+2, true); return INTRINSIC; }

   /*
      boolean constants, preprocessing number and identifier

      TRUE FALSE
      INTCONSTANT
      FLOATCONSTANT
      IDENTIFIER
   */

"true"                 { pptoken.v = 1; return BOOLCONSTANT; }
"false"                { pptoken.v = 0; return BOOLCONSTANT; }

{DOT}?{DIGIT}+({DIGIT}*{NONDIG}*{EXP}*{DOT}*)*  {
                                                   pptoken.s = strdup_fast(pptext);
                                                   return PPNUMBER;
                                                }

{IDENT} {
           if (ppleng > 1024)
              glsl_compile_error(ERROR_LEXER_PARSER, 4, g_LineNumber, "%s", pptext);

           pptoken.s = glsl_intern(pptext, true);
           return IDENTIFIER;
        }

   /*
      C++ comment
   */

"//".*   return WHITESPACE;

   /*
      C comment
   */

"/*"     BEGIN(comment);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             g_LineNumber++;
<comment>"*"+"/"        {
                           BEGIN(INITIAL);
                           return WHITESPACE;
                        }
<comment><<EOF>>        glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "unexpected end of file in comment");

.                       { pptoken.s = glsl_intern(pptext, true); return INVALID_CHAR; }

%%

typedef enum lexer_state_e {
   LEXER_STATE_NORMAL,
   LEXER_STATE_INCLUDE
} lexer_state_t;

static struct ShaderSource {
   int         Count;
   const char *const *Values;
   const char *CurrentPosition;
   int         Index;
} s_ShaderSource;
static lexer_state_t s_LexerState;
static int including_oldLineNumber;
static YY_BUFFER_STATE s_CurrentBufferState;
static const char* s_keywordToString[KEYWORDS_END - KEYWORDS_BEGIN];

// When the lexer starts, set it to read the header.
// Note this prototype is copied into glsl_parser.y
void glsl_init_lexer(int sourcec, const char * const *sourcev)
{
   g_LineNumber =  LINE_NUMBER_UNDEFINED;
   g_FileNumber = -1;

   s_ShaderSource.Count = sourcec;
   s_ShaderSource.Values = sourcev;

   s_ShaderSource.Index           =  0;
   s_ShaderSource.CurrentPosition = "";
   s_LexerState = LEXER_STATE_NORMAL;

   // Create buffer to use with YY_INPUT
   s_CurrentBufferState = pp_create_buffer(NULL, YY_BUF_SIZE - 2);
   pp_switch_to_buffer(s_CurrentBufferState);

   for(int i = 0; i < KEYWORDS_END - KEYWORDS_BEGIN; i++)
      s_keywordToString[i] = NULL;
}

void glsl_include(const char *s)
{
    /* We don't support including from inside an include */
    assert(s_LexerState == LEXER_STATE_NORMAL);
    pppush_buffer_state(s_CurrentBufferState);
    pp_scan_string(s);
    s_LexerState = LEXER_STATE_INCLUDE;
    including_oldLineNumber = g_LineNumber;
}

void glsl_term_lexer(void)
{
   pplex_destroy();
}

static inline int handle_keyword(TokenType token, KeywordFlags flags)
{
   // We might need to convert the keyword to identifier later
   if (s_keywordToString[token - KEYWORDS_BEGIN] == NULL) {
      s_keywordToString[token - KEYWORDS_BEGIN] = glsl_intern(pptext, true);
   }
   pptoken.flags = flags;
   return token;
}

static inline int handle_extension_keyword(TokenType token, enum glsl_ext ext)
{
   if (glsl_ext_status(ext) == GLSL_DISABLED) {
      pptoken.s = glsl_intern(pptext, true);
      return IDENTIFIER;
   } else {
      return handle_keyword(token, KW_FLAGS(KEYWORD, KEYWORD, KEYWORD, KEYWORD));
   }
}

const char* glsl_keyword_to_string(TokenType token)
{
   assert(KEYWORDS_BEGIN < token && token < KEYWORDS_END);
   assert(s_keywordToString[token - KEYWORDS_BEGIN] != NULL);
   return s_keywordToString[token - KEYWORDS_BEGIN];
}

// Called by lex when we reach EOF (include or root)
// Returning 1 causes the lexer to terminate; 0 causes it to try again.
// This allows you to switch buffers.
int ppwrap(void)
{
    if (s_LexerState == LEXER_STATE_INCLUDE) {
        pppop_buffer_state();
        s_LexerState = LEXER_STATE_NORMAL;
        g_LineNumber = including_oldLineNumber;
        return 0;
    }
    return 1; // EOF
}

bool glsl_lexer_in_user_code() {
   return s_LexerState == LEXER_STATE_NORMAL;
}

// If we have reached the end of current source buffer, move to the next one. This may reset the line number.
static void pp_ensure_source()
{
   while(*s_ShaderSource.CurrentPosition == '\0') {
      g_LineNumber = 1;

      if(s_ShaderSource.Index < s_ShaderSource.Count) {
         s_ShaderSource.CurrentPosition = s_ShaderSource.Values[s_ShaderSource.Index++];
         g_FileNumber++;
      } else
         return;
   }
}

// Helper method to skip given character from the source.
static int pp_consume(char c)
{
   // ensure that we have data to read, this may reset the line number
   pp_ensure_source();
   if (*s_ShaderSource.CurrentPosition == c) {
      // skip the consumed character
      s_ShaderSource.CurrentPosition++;
      return 1;
   }
   return 0;
}

// Called by flex to get more input
static int yyinput(char* buf, int buf_size)
{
   int num = 0;

   // Check whether there is unprocessed new-line buffered in flex.
   // Flex requires at least one lookahead character before it commits to token's action
   // so this happens reasonably frequently and it breaks our line numbering.
   if (YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0] == '\n' && &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[1] == buf) {
      // \r is not used for anything in the lexer, so use it to flush the buffer
      buf[num++] = '\r';
      return num;
   }

   // Consume all line continuations here so that the lexer does not even see them.
   // Note that single line continuation may be split among multiple source buffers.
   // Line number modification is fine here because the lexer has already processed
   // all previous (complete) tokens at this point (new-lines in particular).
   // GLSL ES 1.0.17: The line continuation character (\) is not part of the language.
   // GLSL ES 3.00.4: Backslash ('\'), used to indicate line continuation when immediately preceding a new-line.
   while (pp_consume('\\')) {
      if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 0, 1)) {
         // Line continuations are not supported before 3.0
         // #version should have been processed by now
         buf[num++] = '\\';
         break;
      } else if (pp_consume('\n')) {
         g_LineNumber++;
         if (pp_consume('\r') && g_LineNumber == 1)
            g_LineNumber++; // do again if line number was just reset
      } else if (pp_consume('\r')) {
         g_LineNumber++;
         if (pp_consume('\n') && g_LineNumber == 1)
            g_LineNumber++; // do again if line number was just reset
      } else {
         // the backslash did not start line continuation so pass it to the lexer
         buf[num++] = '\\';
         break;
      }
   }

   pp_ensure_source();

   // Copy chunk of data to flex's buffer until we see something that needs to handled in this method
   // (end of current source buffer or line continuation).
   // Flex will process this buffer and call yyinput again once it is done with it.
   while (*s_ShaderSource.CurrentPosition != '\0' && *s_ShaderSource.CurrentPosition != '\\' && num < buf_size) {
      // Turn any \r, \n or pair into \n.
      // GLSL ES 1.0 & 3.0: [Lines] are terminated by carriage-return or line-feed. If both are used together, it will count as only a single line termination.
      char c = *(s_ShaderSource.CurrentPosition++);
      if (c == '\n' || c == '\r') {
         char c2 = *s_ShaderSource.CurrentPosition;
         if ((c == '\n' && c2 == '\r') || (c == '\r' && c2 == '\n'))
            s_ShaderSource.CurrentPosition++; // skip the other one

         buf[num++] = '\n';
      } else {
         buf[num++] = c;
      }
   }

   return num;
}
