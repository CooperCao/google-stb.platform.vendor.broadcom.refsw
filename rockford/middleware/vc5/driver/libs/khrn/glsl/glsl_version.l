%{
   // build me with "flex -L -o glsl_version.c glsl_version.l"

   #include <stdbool.h>

   #include "glsl_common.h"  /* XXX: For GLSL_SHADER_VERSION */

   int vvinput(char *buf, int buf_size, yyscan_t scanner);

   typedef struct {
      int                count;
      const char *const *values;
      const char        *cur_pos;
      int                index;
   } ShaderSource;

   typedef enum {
      PROFILE_NONE,
      PROFILE_ES,
      PROFILE_CORE,
      PROFILE_COMPATIBILITY
   } Profile;

   typedef union {
      int version;
      Profile profile;
   } YYSTYPE;

   #define YY_INPUT(buf,result,max_size) { result = vvinput(buf,(int)max_size, yyscanner); }

   #define HASH          0
   #define VERSION       1
   #define WHITESPACE    2
   #define NUMBER        3
   #define PROFILE       4
   #define NON_VERSION   5
   #define NEWLINE       6
%}

%option prefix="vv"
%option never-interactive
%option batch
%option nounistd nounput noinput noyywrap
%option reentrant
%option bison-bridge

%%

"#"              return HASH;
"version"        return VERSION;
[ \t]+           return WHITESPACE;
[0-9]+           { yylval->version = atoi(yytext); return NUMBER; }
"es"             { yylval->profile = PROFILE_ES;   return PROFILE; }
"core"           { yylval->profile = PROFILE_CORE; return PROFILE; }
"compatibility"  { yylval->profile = PROFILE_COMPATIBILITY; return PROFILE; }
[\r\n]           return NEWLINE;
.                return NON_VERSION;

%%

static int next(yyscan_t scanner, YYSTYPE *tok_data, bool skip_whitespace) {
   int tok;
   if (skip_whitespace) {
      do { tok = yylex(tok_data, scanner); } while (tok == WHITESPACE);
   } else tok = yylex(tok_data, scanner);
   return tok;
}

static bool parse_for_version(yyscan_t scanner, int *version) {
   int expected[] = { HASH, VERSION, WHITESPACE, NUMBER, WHITESPACE, PROFILE, NEWLINE };
   int default_version = GLSL_SHADER_VERSION(1, 0, 0);
   YYSTYPE tok_data;
   int v = 0;  /* Never used. All valid declarations set v */
   bool es = false;

   for (int i=0; i<7; i++) {
      /* Get the next token. Don't skip whitespace if we require some next */
      int tok = next(scanner, &tok_data, expected[i] != WHITESPACE);
      if (tok != expected[i]) {
         if (i < 2) {
            /* Turns out this wasn't a version declaration at all */
            *version = default_version;
            return true;
         }

         /* Is this a valid version declaration with no profile? */
         if (tok == NEWLINE && (i == 4 || i == 5)) break;

         /* Invalid version declaration */
         return false;
      }

      if (tok == NUMBER) v = tok_data.version;
      if (tok == PROFILE) es = (tok_data.profile == PROFILE_ES);
   }

   *version = GLSL_SHADER_VERSION(v / 100, v % 100, es ? 1 : 0);
   return true;
}

static const int supported_versions[] = {
   GLSL_SHADER_VERSION(1, 0, 0),
   GLSL_SHADER_VERSION(3, 0, 1),
#ifdef GLSL_310_SUPPORT
   GLSL_SHADER_VERSION(3, 10, 1),
#endif
#ifdef GLSL_320_SUPPORT
   GLSL_SHADER_VERSION(3, 20, 1),
#endif
};

static bool is_version_supported(int version) {
   int n_supported_versions = sizeof(supported_versions) / sizeof(int);
   for (int i=0; i<n_supported_versions; i++) {
      if (supported_versions[i] == version) return true;
   }
   return false;
}

/* The entry point for this file. Returns the version declared in sourcev. */
bool glsl_find_version(int sourcec, const char *const *sourcev, int *version)
{
   ShaderSource s = { .count   = sourcec,
                      .values  = sourcev,
                      .index   = 0,
                      .cur_pos = "" };

   yyscan_t scanner;
   vvlex_init_extra(&s, &scanner);
   bool valid = parse_for_version(scanner, version);
   vvlex_destroy(scanner);
   if (!valid) return false;

   if (!is_version_supported(*version)) return false;

   if (*version == GLSL_SHADER_VERSION(1, 0, 0))
      *version = GLSL_SHADER_VERSION(1, 0, 1);

   return true;
}

// Called by flex to get more input
// If we have reached the end of the source buffer, move to the next
int vvinput(char *buf, int buf_size, yyscan_t scanner)
{
   ShaderSource *s = vvget_extra(scanner);

   while(*s->cur_pos == '\0') {
      if(s->index >= s->count) return 0;

      s->cur_pos = s->values[s->index];
      s->index++;
   }

   // Copy data to flex's buffer until the end of current source buffer.
   // Flex will process this buffer and call vvinput again once it is done with it.
   int num = 0;
   while (*s->cur_pos != '\0' && num < buf_size)
   {
      buf[num++] = *(s->cur_pos++);
   }

   return num;
}
