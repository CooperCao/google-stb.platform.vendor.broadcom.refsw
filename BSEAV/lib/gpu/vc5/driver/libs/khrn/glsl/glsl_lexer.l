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

   #define YYSTYPE TokenData

   static inline int handle_keyword_w_ext(TokenType token, TokenData *tok_data, KeywordFlags flags, enum glsl_ext ext);
   static inline int handle_keyword(TokenType token, TokenData *tok_data, KeywordFlags flags);
   static inline int handle_extension_keyword(TokenType token, TokenData *tok_data, enum glsl_ext ext);
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
%option noyywrap
%option bison-bridge

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

"define"                      return handle_keyword(DEFINE,    yylval, PREPROC_KEYWORD);
"undef"                       return handle_keyword(UNDEF,     yylval, PREPROC_KEYWORD);
"ifdef"                       return handle_keyword(IFDEF,     yylval, PREPROC_KEYWORD);
"ifndef"                      return handle_keyword(IFNDEF,    yylval, PREPROC_KEYWORD);
"elif"                        return handle_keyword(ELIF,      yylval, PREPROC_KEYWORD);
"endif"                       return handle_keyword(ENDIF,     yylval, PREPROC_KEYWORD);
"error"                       return handle_keyword(ERROR,     yylval, PREPROC_KEYWORD);
"pragma"                      return handle_keyword(PRAGMA,    yylval, PREPROC_KEYWORD);
"extension"                   return handle_keyword(EXTENSION, yylval, PREPROC_KEYWORD);
"version"                     return handle_keyword(VERSION,   yylval, PREPROC_KEYWORD);
"line"                        return handle_keyword(LINE,      yylval, PREPROC_KEYWORD);
"all"                         return handle_keyword(ALL,       yylval, PREPROC_KEYWORD);
"require"                     return handle_keyword(REQUIRE,   yylval, PREPROC_KEYWORD);
"enable"                      return handle_keyword(ENABLE,    yylval, PREPROC_KEYWORD);
"warn"                        return handle_keyword(WARN,      yylval, PREPROC_KEYWORD);
"disable"                     return handle_keyword(DISABLE,   yylval, PREPROC_KEYWORD);

   /*
      list of keywords from GLSL ES Specification 1.0.17 and GLSL ES Specification 3.00.4

      Note that it is ok to pass reserved keywords to the preprocessor.
      If they do not reach the parser, it is not an error.

      keywords "true" and "false" are lexed as BOOLCONSTANT
   */

"active"                      return handle_keyword(ACTIVE,               yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"asm"                         return handle_keyword(ASM,                  yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"atomic_uint"                 return handle_keyword(ATOMIC_UINT,          yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"attribute"                   return handle_keyword(ATTRIBUTE,            yylval, KW_FLAGS(KEYWORD,  RESERVED, RESERVED, RESERVED));
"bool"                        return handle_keyword(BOOL,                 yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"break"                       return handle_keyword(BREAK,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"buffer"                      return handle_keyword(BUFFER,               yylval, KW_FLAGS(NONE,     NONE,     KEYWORD,  KEYWORD));
"bvec2"                       return handle_keyword(BVEC2,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"bvec3"                       return handle_keyword(BVEC3,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"bvec4"                       return handle_keyword(BVEC4,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"case"                        return handle_keyword(CASE,                 yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"cast"                        return handle_keyword(CAST,                 yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"centroid"                    return handle_keyword(CENTROID,             yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"class"                       return handle_keyword(CLASS,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"coherent"                    return handle_keyword(COHERENT,             yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"common"                      return handle_keyword(COMMON,               yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"const"                       return handle_keyword(CONST,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"continue"                    return handle_keyword(CONTINUE,             yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"default"                     return handle_keyword(DEFAULT,              yylval, KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"discard"                     return handle_keyword(DISCARD,              yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"dmat2"                       return handle_keyword(DMAT2,                yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x2"                     return handle_keyword(DMAT2X2,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x3"                     return handle_keyword(DMAT2X3,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat2x4"                     return handle_keyword(DMAT2X4,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3"                       return handle_keyword(DMAT3,                yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x2"                     return handle_keyword(DMAT3X2,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x3"                     return handle_keyword(DMAT3X3,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat3x4"                     return handle_keyword(DMAT3X4,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4"                       return handle_keyword(DMAT4,                yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x2"                     return handle_keyword(DMAT4X2,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x3"                     return handle_keyword(DMAT4X3,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"dmat4x4"                     return handle_keyword(DMAT4X4,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"do"                          return handle_keyword(DO,                   yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"double"                      return handle_keyword(DOUBLE,               yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec2"                       return handle_keyword(DVEC2,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec3"                       return handle_keyword(DVEC3,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"dvec4"                       return handle_keyword(DVEC4,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"else"                        return handle_keyword(ELSE,                 yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"enum"                        return handle_keyword(ENUM,                 yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"extern"                      return handle_keyword(EXTERN,               yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"external"                    return handle_keyword(EXTERNAL,             yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"filter"                      return handle_keyword(FILTER,               yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"fixed"                       return handle_keyword(FIXED,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"flat"                        return handle_keyword(FLAT,                 yylval, KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"float"                       return handle_keyword(FLOAT,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"for"                         return handle_keyword(FOR,                  yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"fvec2"                       return handle_keyword(FVEC2,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"fvec3"                       return handle_keyword(FVEC3,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"fvec4"                       return handle_keyword(FVEC4,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"goto"                        return handle_keyword(GOTO,                 yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"half"                        return handle_keyword(HALF,                 yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"highp"                       return handle_keyword(HIGH_PRECISION,       yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"hvec2"                       return handle_keyword(HVEC2,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"hvec3"                       return handle_keyword(HVEC3,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"hvec4"                       return handle_keyword(HVEC4,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"if"                          return handle_keyword(IF,                   yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"iimage1D"                    return handle_keyword(IIMAGE1D,             yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"iimage1DArray"               return handle_keyword(IIMAGE1DARRAY,        yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"iimage2D"                    return handle_keyword(IIMAGE2D,             yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimage2DArray"               return handle_keyword(IIMAGE2DARRAY,        yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimage2DMS"                  return handle_keyword(IIMAGE2DMS,           yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage2DMSArray"             return handle_keyword(IIMAGE2DMSARRAY,      yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage2DRect"                return handle_keyword(IIMAGE2DRECT,         yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"iimage3D"                    return handle_keyword(IIMAGE3D,             yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimageBuffer"                return handle_keyword_w_ext(IIMAGEBUFFER,         yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"iimageCube"                  return handle_keyword(IIMAGECUBE,           yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"iimageCubeArray"             return handle_keyword_w_ext(IIMAGECUBEARRAY,      yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"image1D"                     return handle_keyword(IMAGE1D,              yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"image1DArray"                return handle_keyword(IMAGE1DARRAY,         yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"image1DArrayShadow"          return handle_keyword(IMAGE1DARRAYSHADOW,   yylval, KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image1DShadow"               return handle_keyword(IMAGE1DSHADOW,        yylval, KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image2D"                     return handle_keyword(IMAGE2D,              yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"image2DArray"                return handle_keyword(IMAGE2DARRAY,         yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"image2DMS"                   return handle_keyword(IMAGE2DMS,            yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DMSArray"              return handle_keyword(IMAGE2DMSARRAY,       yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DRect"                 return handle_keyword(IMAGE2DRECT,          yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"image2DArrayShadow"          return handle_keyword(IMAGE2DARRAYSHADOW,   yylval, KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image2DShadow"               return handle_keyword(IMAGE2DSHADOW,        yylval, KW_FLAGS(NONE,     RESERVED, NONE,     NONE));
"image3D"                     return handle_keyword(IMAGE3D,              yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"imageBuffer"                 return handle_keyword_w_ext(IMAGEBUFFER,          yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"imageCube"                   return handle_keyword(IMAGECUBE,            yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"imageCubeArray"              return handle_keyword_w_ext(IMAGECUBEARRAY,       yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"in"                          return handle_keyword(IN,                   yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"inline"                      return handle_keyword(_INLINE,              yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"inout"                       return handle_keyword(INOUT,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"input"                       return handle_keyword(INPUT,                yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"int"                         return handle_keyword(INT,                  yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"interface"                   return handle_keyword(INTERFACE,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"invariant"                   return handle_keyword(INVARIANT,            yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"isampler1D"                  return handle_keyword_w_ext(ISAMPLER1D,           yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"isampler1DArray"             return handle_keyword_w_ext(ISAMPLER1DARRAY,      yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"isampler2D"                  return handle_keyword(ISAMPLER2D,           yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isampler2DArray"             return handle_keyword(ISAMPLER2DARRAY,      yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isampler2DMS"                return handle_keyword(ISAMPLER2DMS,         yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"isampler2DMSArray"           return handle_keyword_w_ext(ISAMPLER2DMSARRAY,    yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_2DMSARRAY);
"isampler2DRect"              return handle_keyword(ISAMPLER2DRECT,       yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"isampler3D"                  return handle_keyword(ISAMPLER3D,           yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isamplerBuffer"              return handle_keyword_w_ext(ISAMPLERBUFFER,       yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"isamplerCube"                return handle_keyword(ISAMPLERCUBE,         yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"isamplerCubeArray"           return handle_keyword_w_ext(ISAMPLERCUBEARRAY,    yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"ivec2"                       return handle_keyword(IVEC2,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"ivec3"                       return handle_keyword(IVEC3,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"ivec4"                       return handle_keyword(IVEC4,                yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"layout"                      return handle_keyword(LAYOUT,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"long"                        return handle_keyword(LONG,                 yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"lowp"                        return handle_keyword(LOW_PRECISION,        yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat2"                        return handle_keyword(MAT2,                 yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat2x2"                      return handle_keyword(MAT2X2,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat2x3"                      return handle_keyword(MAT2X3,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat2x4"                      return handle_keyword(MAT2X4,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3"                        return handle_keyword(MAT3,                 yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat3x2"                      return handle_keyword(MAT3X2,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3x3"                      return handle_keyword(MAT3X3,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat3x4"                      return handle_keyword(MAT3X4,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4"                        return handle_keyword(MAT4,                 yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"mat4x2"                      return handle_keyword(MAT4X2,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4x3"                      return handle_keyword(MAT4X3,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mat4x4"                      return handle_keyword(MAT4X4,               yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"mediump"                     return handle_keyword(MEDIUM_PRECISION,     yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"namespace"                   return handle_keyword(NAMESPACE,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"noinline"                    return handle_keyword(NOINLINE,             yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"noperspective"               return handle_keyword(NOPERSPECTIVE,        yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"out"                         return handle_keyword(OUT,                  yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"output"                      return handle_keyword(OUTPUT,               yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"packed"                      return handle_keyword(PACKED,               yylval, KW_FLAGS(RESERVED, NONE,     NONE,     NONE));
"partition"                   return handle_keyword(PARTITION,            yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"patch"                       return handle_keyword_w_ext(PATCH,                yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TESSELLATION);
"precise"                     return handle_keyword_w_ext(PRECISE,              yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_GPU_SHADER5);
"precision"                   return handle_keyword(PRECISION,            yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"public"                      return handle_keyword(PUBLIC,               yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"readonly"                    return handle_keyword(READONLY,             yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"resource"                    return handle_keyword(RESOURCE,             yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"restrict"                    return handle_keyword(RESTRICT,             yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"return"                      return handle_keyword(RETURN,               yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"sample"                      return handle_keyword_w_ext(SAMPLE,               yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_MS_INTERPOLATION);
"sampler1D"                   return handle_keyword_w_ext(SAMPLER1D,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"sampler1DArray"              return handle_keyword_w_ext(SAMPLER1DARRAY,       yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"sampler1DArrayShadow"        return handle_keyword(SAMPLER1DARRAYSHADOW, yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"sampler1DShadow"             return handle_keyword(SAMPLER1DSHADOW,      yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2D"                   return handle_keyword(SAMPLER2D,            yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DArray"              return handle_keyword(SAMPLER2DARRAY,       yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DArrayShadow"        return handle_keyword(SAMPLER2DARRAYSHADOW, yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"sampler2DMS"                 return handle_keyword(SAMPLER2DMS,          yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"sampler2DMSArray"            return handle_keyword_w_ext(SAMPLER2DMSARRAY,     yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_2DMSARRAY);
"sampler2DRect"               return handle_keyword(SAMPLER2DRECT,        yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2DRectShadow"         return handle_keyword(SAMPLER2DRECTSHADOW,  yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sampler2DShadow"             return handle_keyword(SAMPLER2DSHADOW,      yylval, KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"sampler3D"                   return handle_keyword(SAMPLER3D,            yylval, KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"sampler3DRect"               return handle_keyword(SAMPLER3DRECT,        yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"samplerBuffer"               return handle_keyword_w_ext(SAMPLERBUFFER,        yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"samplerCube"                 return handle_keyword(SAMPLERCUBE,          yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"samplerCubeArray"            return handle_keyword_w_ext(SAMPLERCUBEARRAY,       yylval, KW_FLAGS(NONE, NONE, RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"samplerCubeArrayShadow"      return handle_keyword_w_ext(SAMPLERCUBEARRAYSHADOW, yylval, KW_FLAGS(NONE, NONE, RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"samplerCubeShadow"           return handle_keyword(SAMPLERCUBESHADOW, yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"shared"                      return handle_keyword(SHARED,            yylval, KW_FLAGS(NONE,     NONE,     KEYWORD,  KEYWORD));
"short"                       return handle_keyword(SHORT,             yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"sizeof"                      return handle_keyword(SIZEOF,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"smooth"                      return handle_keyword(SMOOTH,            yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"static"                      return handle_keyword(STATIC,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"struct"                      return handle_keyword(STRUCT,            yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"subroutine"                  return handle_keyword(SUBROUTINE,        yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"superp"                      return handle_keyword(SUPERP,            yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"switch"                      return handle_keyword(SWITCH,            yylval, KW_FLAGS(RESERVED, KEYWORD,  KEYWORD,  KEYWORD));
"template"                    return handle_keyword(TEMPLATE,          yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"this"                        return handle_keyword(THIS,              yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"typedef"                     return handle_keyword(TYPEDEF,           yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"uimage1D"                    return handle_keyword(UIMAGE1D,          yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"uimage1DArray"               return handle_keyword(UIMAGE1DARRAY,     yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"uimage2D"                    return handle_keyword(UIMAGE2D,          yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimage2DArray"               return handle_keyword(UIMAGE2DARRAY,     yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimage2DMS"                  return handle_keyword(UIMAGE2DMS,        yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage2DMSArray"             return handle_keyword(UIMAGE2DMSARRAY,   yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage2DRect"                return handle_keyword(UIMAGE2DRECT,      yylval, KW_FLAGS(NONE,     NONE,     RESERVED, RESERVED));
"uimage3D"                    return handle_keyword(UIMAGE3D,          yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimageBuffer"                return handle_keyword_w_ext(UIMAGEBUFFER,      yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"uimageCube"                  return handle_keyword(UIMAGECUBE,        yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"uimageCubeArray"             return handle_keyword_w_ext(UIMAGECUBEARRAY,   yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"uint"                        return handle_keyword(UINT,              yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uniform"                     return handle_keyword(UNIFORM,           yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"union"                       return handle_keyword(UNION,             yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"unsigned"                    return handle_keyword(UNSIGNED,          yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"usampler1D"                  return handle_keyword_w_ext(USAMPLER1D,        yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"usampler1DArray"             return handle_keyword_w_ext(USAMPLER1DARRAY,   yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED), GLSL_EXT_BRCM_TEXTURE_1D);
"usampler2D"                  return handle_keyword(USAMPLER2D,        yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usampler2DArray"             return handle_keyword(USAMPLER2DARRAY,   yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usampler2DMS"                return handle_keyword(USAMPLER2DMS,      yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));
"usampler2DMSArray"           return handle_keyword_w_ext(USAMPLER2DMSARRAY, yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_2DMSARRAY);
"usampler2DRect"              return handle_keyword(USAMPLER2DRECT,    yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, RESERVED));
"usampler3D"                  return handle_keyword(USAMPLER3D,        yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usamplerBuffer"              return handle_keyword_w_ext(USAMPLERBUFFER,    yylval, KW_FLAGS(NONE,     RESERVED, RESERVED, KEYWORD), GLSL_EXT_TEXTURE_BUFFER);
"usamplerCube"                return handle_keyword(USAMPLERCUBE,      yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"usamplerCubeArray"           return handle_keyword_w_ext(USAMPLERCUBEARRAY, yylval, KW_FLAGS(NONE,     NONE,     RESERVED, KEYWORD), GLSL_EXT_CUBE_MAP_ARRAY);
"using"                       return handle_keyword(USING,             yylval, KW_FLAGS(RESERVED, RESERVED, RESERVED, RESERVED));
"uvec2"                       return handle_keyword(UVEC2,             yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uvec3"                       return handle_keyword(UVEC3,             yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"uvec4"                       return handle_keyword(UVEC4,             yylval, KW_FLAGS(NONE,     KEYWORD,  KEYWORD,  KEYWORD));
"varying"                     return handle_keyword(VARYING,           yylval, KW_FLAGS(KEYWORD,  RESERVED, RESERVED, RESERVED));
"vec2"                        return handle_keyword(VEC2,              yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"vec3"                        return handle_keyword(VEC3,              yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"vec4"                        return handle_keyword(VEC4,              yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"void"                        return handle_keyword(VOID,              yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"volatile"                    return handle_keyword(VOLATILE,          yylval, KW_FLAGS(RESERVED, RESERVED, KEYWORD,  KEYWORD));
"while"                       return handle_keyword(WHILE,             yylval, KW_FLAGS(KEYWORD,  KEYWORD,  KEYWORD,  KEYWORD));
"writeonly"                   return handle_keyword(WRITEONLY,         yylval, KW_FLAGS(NONE,     RESERVED, KEYWORD,  KEYWORD));

   /*
      keywords for extensions
   */

"samplerExternalOES"          return handle_extension_keyword(SAMPLEREXTERNAL, yylval, GLSL_EXT_IMAGE_EXTERNAL);

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

"$$barrier"    { return BARRIER; }
"$$"{IDENT}    { yylval->s = glsl_intern(pptext+2, true); return INTRINSIC; }

   /*
      boolean constants, preprocessing number and identifier

      TRUE FALSE
      INTCONSTANT
      FLOATCONSTANT
      IDENTIFIER
   */

"true"                 { yylval->v = 1; return BOOLCONSTANT; }
"false"                { yylval->v = 0; return BOOLCONSTANT; }

{DOT}?{DIGIT}+({DIGIT}*{NONDIG}*{EXP}*{DOT}*)*  {
                                                   yylval->s = strdup_fast(pptext);
                                                   return PPNUMBER;
                                                }

{IDENT} {
           if (ppleng > 1024)
              glsl_compile_error(ERROR_LEXER_PARSER, 4, g_LineNumber, "%s", pptext);

           yylval->s = glsl_intern(pptext, true);
           return IDENTIFIER;
        }

   /* C++ comment */

"//".*   return WHITESPACE;

   /* C comment */

"/*"     BEGIN(comment);

<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>\n             g_LineNumber++;
<comment>"*"+"/"        {
                           BEGIN(INITIAL);
                           return WHITESPACE;
                        }
<comment><<EOF>>        glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "unexpected end of file in comment");

.                       { yylval->s = glsl_intern(pptext, true); return INVALID_CHAR; }

%%

static struct ShaderSource {
   int         count;
   const char *const *values;
   const char *cur_pos;
   int         index;
} s_ShaderSource;

static const char *s_keywordToString[KEYWORDS_END - KEYWORDS_BEGIN];

// When the lexer starts, set it to read the header.
// Note this prototype is copied into glsl_parser.y
void glsl_init_lexer(int sourcec, const char * const *sourcev)
{
   g_LineNumber =  LINE_NUMBER_UNDEFINED;
   g_FileNumber = -1;

   s_ShaderSource.count   = sourcec;
   s_ShaderSource.values  = sourcev;
   s_ShaderSource.index   =  0;
   s_ShaderSource.cur_pos = "";

   for(int i = 0; i < KEYWORDS_END - KEYWORDS_BEGIN; i++)
      s_keywordToString[i] = NULL;
}

void glsl_term_lexer(void)
{
   pplex_destroy();
}

static inline int handle_keyword_w_ext(TokenType token, TokenData *tok_data, KeywordFlags flags, enum glsl_ext ext) {
   /* Cache the string in case we need to de-tokenise later */
   if (s_keywordToString[token - KEYWORDS_BEGIN] == NULL)
      s_keywordToString[token - KEYWORDS_BEGIN] = glsl_intern(pptext, true);

   if (ext != GLSL_EXT_NOT_SUPPORTED && glsl_ext_status(ext) != GLSL_DISABLED) flags = KW_FLAGS(KEYWORD, KEYWORD, KEYWORD, KEYWORD);
   tok_data->flags = flags;
   return token;
}

static inline int handle_keyword(TokenType token, TokenData *tok_data, KeywordFlags flags) {
   return handle_keyword_w_ext(token, tok_data, flags, GLSL_EXT_NOT_SUPPORTED);
}

static inline int handle_extension_keyword(TokenType token, TokenData *tok_data, enum glsl_ext ext) {
   return handle_keyword_w_ext(token, tok_data, KW_FLAGS(NONE, NONE, NONE, NONE), ext);
}

const char *glsl_keyword_to_string(TokenType token)
{
   assert(KEYWORDS_BEGIN < token && token < KEYWORDS_END);
   assert(s_keywordToString[token - KEYWORDS_BEGIN] != NULL);
   return s_keywordToString[token - KEYWORDS_BEGIN];
}

// If we have reached the end of current source buffer, move to the next one. This may reset the line number.
static void ensure_source(struct ShaderSource *s)
{
   while(*s->cur_pos == '\0') {
      g_LineNumber = 1;

      if(s->index < s->count) {
         s->cur_pos = s->values[s->index++];
         g_FileNumber++;
      } else
         return;
   }
}

// Helper method to skip given character from the source.
static int consume(struct ShaderSource *s, char c)
{
   // ensure that we have data to read, this may reset the line number
   ensure_source(s);
   if (*s->cur_pos == c) {
      // skip the consumed character
      s->cur_pos++;
      return 1;
   }
   return 0;
}

// Called by flex to get more input
static int yyinput(char *buf, int buf_size)
{
   int num = 0;
   struct ShaderSource *s = &s_ShaderSource;

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
   while (consume(s, '\\')) {
      if (g_ShaderVersion < GLSL_SHADER_VERSION(3, 0, 1)) {
         // Line continuations are not supported before 3.0
         // #version should have been processed by now
         buf[num++] = '\\';
         break;
      } else if (consume(s, '\n')) {
         g_LineNumber++;
         if (consume(s, '\r') && g_LineNumber == 1)
            g_LineNumber++; // do again if line number was just reset
      } else if (consume(s, '\r')) {
         g_LineNumber++;
         if (consume(s, '\n') && g_LineNumber == 1)
            g_LineNumber++; // do again if line number was just reset
      } else {
         // the backslash did not start line continuation so pass it to the lexer
         buf[num++] = '\\';
         break;
      }
   }

   ensure_source(s);

   // Copy chunk of data to flex's buffer until we see something that needs to handled in this method
   // (end of current source buffer or line continuation).
   // Flex will process this buffer and call yyinput again once it is done with it.
   while (*s->cur_pos != '\0' && *s->cur_pos != '\\' && num < buf_size) {
      // Turn any \r, \n or pair into \n.
      // GLSL ES 1.0 & 3.0: [Lines] are terminated by carriage-return or line-feed. If both are used together, it will count as only a single line termination.
      char c = *(s->cur_pos++);
      if (c == '\n' || c == '\r') {
         char c2 = *s->cur_pos;
         if ((c == '\n' && c2 == '\r') || (c == '\r' && c2 == '\n'))
            s->cur_pos++; // skip the other one

         buf[num++] = '\n';
      } else {
         buf[num++] = c;
      }
   }

   return num;
}
