%{
   // build me with "flex -L -o glsl_numbers.c --never-interactive glsl_numbers.l"
   // --never-interactive is to avoid isatty declaration issues with c++

   #include "libs/util/gfx_util/gfx_util.h"
   #include "glsl_numbers.h"

   #include <stdlib.h>
   #include <string.h>
   #include <assert.h>

   #define END_OF_INPUT -1
   typedef uint32_t YYSTYPE;

   /* drop warning level for the auto-generated code */
   #ifdef _MSC_VER
   #pragma warning(push, 1)
   #endif
%}

%option prefix="nn"
%option never-interactive
%option batch
%option noyywrap nounistd nounput noinput
%option bison-bridge
%option reentrant

DS       [0-9]+
EXP      [eE][-+]?{DS}
FRAC     {DS}"."{DS}|{DS}"."|"."{DS}
FS       [fF]

INTDEC   [1-9][0-9]*
INTOCT   "0"[0-7]*
INTHEX   "0"[xX][0-9a-fA-F]+
INTSUFF  [uU]


%%

{FRAC}{EXP}?{FS}?|{DS}{EXP}{FS}? {
                          // Cast from double and then copy bits.
                          *yylval = gfx_float_to_bits((float)atof(yytext));
                          return NUM_FLOAT;
                       }

{INTDEC}|{INTOCT}|{INTHEX} {
                              // Base of 0 does automatic dec/oct/hex recognition.
                              *yylval = (uint32_t)strtoul(yytext, NULL, 0);
                              return NUM_INT;
                           }

{INTDEC}{INTSUFF}|{INTOCT}{INTSUFF}|{INTHEX}{INTSUFF} {
                              // Base of 0 does automatic dec/oct/hex recognition.
                              *yylval = (uint32_t)strtoul(strtok(yytext, "uU"), NULL, 0);
                              return NUM_UINT;
                           }

.        { return NUM_INVALID; }
<<EOF>>  { return END_OF_INPUT; }

%%

int numlex(const char *s, uint32_t *value)
{
   yyscan_t lexer;
   nnlex_init(&lexer);

   YY_BUFFER_STATE yy_buf = nn_scan_string(s, lexer);

   int t = nnlex(value, lexer);
   assert(t != END_OF_INPUT);    /* There must have been some token */

   /* If there's any remaining string then the number is invalid */
   if(t != NUM_INVALID && nnlex(value, lexer) != END_OF_INPUT)
      t = NUM_INVALID;

   nn_delete_buffer(yy_buf, lexer);
   nnlex_destroy(lexer);

   return t;
}
