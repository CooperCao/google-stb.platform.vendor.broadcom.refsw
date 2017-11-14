/**
 *  process with: bison -v -d -o glsl_parser.c glsl_parser.y
 *  (-v for debugging, not needed for release).
 *  (-d to produce glsl_parser.h, NEEDED for release).
 */
%{
%}

%code requires {
   #include "glsl_common.h"
   #include "glsl_layout.h"
   #include "glsl_intrinsic_types.h"
   #include "glsl_symbols.h"

   typedef enum {
      CALL_CONTEXT_FUNCTION,
      CALL_CONTEXT_CONSTRUCTOR,
      CALL_CONTEXT_INTRINSIC
   } CallContextFlavour;

   struct CallContext {
      CallContextFlavour flavour;

      union {
         struct {
            Symbol *symbol;
         } function;

         struct {
            SymbolType *type;
         } constructor;

         struct {
            glsl_intrinsic_index_t flavour;
         } intrinsic;
      } u;
   };
}

%code provides {
   extern Statement *glsl_parse_ast(ShaderFlavour flavour, int version, int sourcec, const char * const *sourcev);
}

%code {
   #include <stdlib.h>
   #include <stdio.h>
   #include <string.h>
   #include <limits.h>
   #include <assert.h>

   #include "glsl_symbols.h"
   #include "glsl_errors.h"
   #include "glsl_intern.h"
   #include "glsl_globals.h"
   #include "glsl_builders.h"
   #include "glsl_extensions.h"
   #include "glsl_ast.h"
   #include "glsl_ast_visitor.h"
   #include "glsl_symbol_table.h"
   #include "glsl_primitive_types.auto.h"
   #include "glsl_intrinsic_lookup.auto.h"
   #include "glsl_layout.auto.h"
   #include "glsl_fastmem.h"  // for 'malloc_fast'

   #include "glsl_stdlib.auto.h"
   #include "glsl_unique_index_queue.h"

   #include "glsl_quals.h"

   #include "prepro/glsl_prepro_token.h"
   #include "prepro/glsl_prepro_expand.h"
   #include "prepro/glsl_prepro_directive.h"

   #include "../glxx/glxx_int_config.h"

   #define PS ((struct parse_state *)state)

   extern void glsl_init_preprocessor (int version);

   void glsl_init_lexer (int sourcec, const char * const *sourcev);
   void glsl_term_lexer (void);
   const char *glsl_keyword_to_string(TokenType token);

   struct parse_state {
      Statement *astp;
      bool in_struct;
      bool user_code;

      TokenSeq *seq;

      SymbolTable *symbol_table;

      DeclDefaultState dflt;

      PrecisionTable *precision_table;

      bool force_identifier;
   };

   int yyparse (void *state);

   static void find_stdlib_function_calls(Expr *e, void *data) {
      GLSL_UNIQUE_INDEX_QUEUE_T *q = data;
      if (e->flavour == EXPR_FUNCTION_CALL) {
         Symbol *function = e->u.function_call.function;
         if (glsl_stdlib_is_stdlib_function(function)) {
            glsl_unique_index_queue_add(q, glsl_stdlib_function_index(function));
         }
      }
   }

   Statement *glsl_parse_ast (ShaderFlavour flavour, int version, int sourcec, const char * const *sourcev)
   {
      struct parse_state state;   /* TODO: Contains lots of nastiness, but this beats global */

      glsl_ext_init();

      g_ShaderFlavour = flavour;
      g_ShaderVersion = version;
      g_InGlobalScope = true;

      state.symbol_table = NULL;    /* Allocated on first token, once version, extensions are decided */
      state.in_struct = false;
      state.force_identifier = false;
      state.seq = NULL;
      state.user_code = true;

      /* Set up the root precision table with default precisions */
      state.precision_table = glsl_prec_add_table(NULL);
      glsl_prec_set_defaults(state.precision_table, flavour);

      state.dflt.buffer_layout  = LAYOUT_SHARED | LAYOUT_COLUMN_MAJOR;
      state.dflt.uniform_layout = LAYOUT_SHARED | LAYOUT_COLUMN_MAJOR;
      /* T+G shaders can have default sizes for in/out arrays. 0 means no default *
       * (but one may be set via a layout qualifier)                              */
      state.dflt.input_size     = (flavour == SHADER_TESS_CONTROL || flavour == SHADER_TESS_EVALUATION) ? 32 : 0;
      state.dflt.output_size    = 0;

      for (int i=0; i<GLXX_CONFIG_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS; i++)
         state.dflt.atomic_offset[i] = 0;

      glsl_init_preprocessor(version);
      glsl_init_lexer(sourcec, sourcev);
      yyparse(&state);
      glsl_term_lexer();

      Statement *ast = state.astp;

      /* Now process the standard library functions that we've used */
      glsl_symbol_table_exit_scope(state.symbol_table);
      glsl_directive_reset_macros();
      int user_version = g_ShaderVersion;
      g_ShaderVersion  = GLSL_SHADER_VERSION(3, 20, 1);
      state.user_code = false;

      GLSL_UNIQUE_INDEX_QUEUE_T *stdlib_functions = glsl_unique_index_queue_alloc(GLSL_STDLIB_FUNCTION_COUNT);
      glsl_statement_accept_postfix(ast, stdlib_functions, NULL, find_stdlib_function_calls);

      StatementChain *ch = glsl_statement_chain_create();
      /* Gather the actual sources here, then run yyparse again */
      while(!glsl_unique_index_queue_empty(stdlib_functions)) {
         const char *srcp[GLSL_STDLIB_FUNCTION_COUNT+1];
         int srcc = 0;

         /* XXX We process the stdlib setup multiple times here. This is wasteful but should work */
         if(glsl_stdlib_setup != NULL) srcp[srcc++] = glsl_stdlib_setup;
         while (!glsl_unique_index_queue_empty(stdlib_functions))
            srcp[srcc++] = glsl_stdlib_function_bodies[glsl_unique_index_queue_remove(stdlib_functions)];

         glsl_init_lexer(srcc,srcp);
         yyparse(&state);
         glsl_term_lexer();

         glsl_statement_accept_postfix(state.astp, stdlib_functions, NULL, find_stdlib_function_calls);

         glsl_statement_chain_cat(ch, state.astp->u.ast.decls);
      }

      /* Validate the real AST after stdlib functions are resolved but before we  *
       * add the stdlib symbols to it, which use features that might not validate */
      glsl_ast_validate(ast, flavour, version);

      /* Hackily append the declaration to the ast decls chain */
      glsl_statement_chain_cat(ast->u.ast.decls, ch);

      g_ShaderVersion = user_version;
      // Try to trap uses of g_LineNumber when it doesn't contain good data.
      // TODO: Remove the whole thing.
      g_LineNumber = LINE_NUMBER_UNDEFINED;

      glsl_symbol_table_delete(state.symbol_table);

      return ast;
   }

   static void yyerror (void *state, const char *s)
   {
      if (strcmp(s, "syntax error") == 0) {
         // Catch syntax errors and redirect them.
         glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, NULL);
      } else {
         glsl_compile_error(ERROR_UNKNOWN, 0, g_LineNumber, "%s", s);
      }
   }

   static int yylex (YYSTYPE *yylvalp, void *state)
   {
      TokenType type = 0;
      TokenData data;

      do {
         if(!PS->seq) {
            PS->seq = glsl_expand(NULL, false);
            if(!PS->seq) return 0;
         }

         type = PS->seq->token->type;
         data = PS->seq->token->data;
         PS->seq = PS->seq->next;
      } while (type==WHITESPACE);

      if (type == INVALID_CHAR)
         glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "invalid characters: %s", data.s);

      if (PS->symbol_table == NULL) {
         PS->symbol_table = glsl_symbol_table_new();
         glsl_symbol_table_populate(PS->symbol_table, g_ShaderFlavour, g_ShaderVersion);
      }

      /* Detect uses of reserved keywords */
      if (KEYWORDS_BEGIN < type && type < KEYWORDS_END) {
         KeywordFlags keyword_flag, reserved_flag;
         switch(g_ShaderVersion) {
            case GLSL_SHADER_VERSION(1, 0, 1):
               keyword_flag = GLSL_ES_1_KEYWORD; reserved_flag = GLSL_ES_1_RESERVED;
               break;
            case GLSL_SHADER_VERSION(3, 0, 1):
               keyword_flag = GLSL_ES_3_KEYWORD; reserved_flag = GLSL_ES_3_RESERVED;
               break;
            case GLSL_SHADER_VERSION(3, 10, 1):
               keyword_flag = GLSL_ES_31_KEYWORD; reserved_flag = GLSL_ES_31_RESERVED;
               break;
            case GLSL_SHADER_VERSION(3, 20, 1):
               keyword_flag = GLSL_ES_32_KEYWORD; reserved_flag = GLSL_ES_32_RESERVED;
               break;
            default: unreachable();
         }

         if (data.flags & reserved_flag)
            glsl_compile_error(ERROR_LEXER_PARSER, 3, g_LineNumber, "%s", glsl_keyword_to_string(type));
         if (!(data.flags & keyword_flag)) {
            // it is not keyword in this GLSL version, convert it back to identifier
            data.s = glsl_keyword_to_string(type);
            type = IDENTIFIER;
         }
      }

      switch (type) {
      case IDENTIFIER:
      {
         Symbol *sym = glsl_symbol_table_lookup(PS->symbol_table, data.s);
         yylvalp->lookup.symbol = sym;
         yylvalp->lookup.name = data.s;
         /* If this names a type return TYPE_NAME unless identifier is forced */
         if (sym && sym->flavour == SYMBOL_TYPE) {
            if(!PS->force_identifier) type = TYPE_NAME;
         }
         break;
      }
      case INTRINSIC:
      {
         if(PS->user_code) {
            glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "invalid character");
         } else {
            /* lookup must always succeed, since this is in the stdlib */
            const glsl_intrinsic_data_t *lookup = glsl_intrinsic_lookup(data.s, strlen(data.s));
            yylvalp->intrinsic = lookup->index;
         }
         break;
      }
      case BOOLCONSTANT:
      case INTCONSTANT:
         yylvalp->v = data.v;
         break;
      case PPNUMBER:
         type = glsl_lex_ppnumber(data.s, &yylvalp->v);
         break;
      default:
         break;
      }

      return type;
   }

   #define YYMALLOC yymalloc
   #define YYFREE yyfree

   void *yymalloc (size_t bytes) { return malloc_fast(bytes); }
   void  yyfree   (void *ptr)    { ((void)ptr); }

   static void enter_scope(struct parse_state *state)
   {
      glsl_symbol_table_enter_scope(state->symbol_table);
      state->precision_table = glsl_prec_add_table( state->precision_table );
   }

   static void exit_scope(struct parse_state *state)
   {
      glsl_symbol_table_exit_scope(state->symbol_table);
      state->precision_table = glsl_prec_delete_table( state->precision_table );
   }

   static void validate_symbol_lookup(const Symbol *s, const char *name) {
      if(!s)
         glsl_compile_error(ERROR_LEXER_PARSER, 2, g_LineNumber, "%s", name);
   }

   static Statement *process_block_decl(struct parse_state *state, QualList *quals, const char *block_name,
                                        StatementChain *type_members, const char *instance_name, ExprChain *array_size)
   {
      Qualifiers q;
      qualifiers_from_list(&q, quals);
      if (q.sq == STORAGE_UNIFORM || q.sq == STORAGE_BUFFER) {
         uint32_t dflt = (q.sq == STORAGE_UNIFORM) ? state->dflt.uniform_layout : state->dflt.buffer_layout;
         if (!q.lq) q.lq = glsl_layout_create(NULL);
         q.lq->qualified |= UNIF_QUALED;
         q.lq->unif_bits = glsl_layout_combine_block_bits(dflt, q.lq->unif_bits);
      }

      SymbolType *type  = glsl_build_block_type(&q, block_name, type_members);
      if (array_size != NULL)
         type = glsl_build_array_type(type, array_size);

      Symbol *block_symbol = glsl_commit_block_type(state->symbol_table, &state->dflt, type, &q);

      if (instance_name)
         glsl_commit_var_instance(state->symbol_table, instance_name, type, &q, NULL);
      else
         glsl_commit_anonymous_block_members(state->symbol_table, block_symbol, q.mq);

      return glsl_statement_construct_var_decl(g_LineNumber, quals, type, block_symbol, NULL);
   }
}

// Create a reentrant, pure parser:
%define api.pure
%lex-param   { void *state }
%parse-param { void *state }
//%token-table       // for debugging: include token table:
%verbose             // for debugging: emit *.output file with parser states
%expect 1            // 1 shift/reduce conflict (if/else, default behaviour's fine)

%start translation_unit

// Yacc's value stack type.
%union {
   const char *s;
   const_value v;
   struct { const char *name; Symbol *symbol; } lookup;
   Symbol *symbol;
   SymbolList *symbol_list;
   struct CallContext call_context;
   struct { const char *name; SymbolType *type; } func_proto;
   ExprFlavour expr_flavour;
   Expr *expr;
   ExprChain *expr_chain;
   glsl_intrinsic_index_t intrinsic;
   struct { QualList *quals; SymbolType *type; Statement *s; } decl;
   struct { QualList *quals; SymbolType *type; StatementChain *ch; } decl_chain;
   Statement *statement;
   StatementChain *statement_chain;
   struct { Statement *t; Statement *f; } selection_rest;
   struct { Statement *cond_or_decl; Expr *iter; } for_rest;
   SymbolType *type;
   QualList  *qual_list;
   Qualifier *qualifier;
   StorageQualifier       storage_qual;
   AuxiliaryQualifier     auxiliary_qual;
   MemoryQualifier        memory_qual;
   PrecisionQualifier     prec_qual;
   InterpolationQualifier interp_qual;
   struct { const char *name; ExprChain *size; } declarator;
   struct { const char *name; ExprChain *size; Expr *init; } init_declarator;
   struct param *param;
   struct param_list *param_list;
   LayoutID *lq_id;
   LayoutIDList *lq_id_list;
   struct { QualList *quals; SymbolType *type; } fs_type;
}

%token KEYWORDS_BEGIN // mark; not a real token

// list of keywords from GLSL ES Specification 1.0.17 and GLSL ES Specification 3.00.4
// keywords "true" and "false" are lexed as BOOLCONSTANT
%token ACTIVE
%token ASM
%token ATOMIC_UINT
%token ATTRIBUTE
%token BOOL
%token BREAK
%token BUFFER
%token BVEC2
%token BVEC3
%token BVEC4
%token CASE
%token CAST
%token CENTROID
%token CLASS
%token COHERENT
%token COMMON
%token CONST
%token CONTINUE
%token DEFAULT
%token DISCARD
%token DMAT2
%token DMAT2X2
%token DMAT2X3
%token DMAT2X4
%token DMAT3
%token DMAT3X2
%token DMAT3X3
%token DMAT3X4
%token DMAT4
%token DMAT4X2
%token DMAT4X3
%token DMAT4X4
%token DO
%token DOUBLE
%token DVEC2
%token DVEC3
%token DVEC4
%token ELSE
%token ENUM
%token EXTERN
%token EXTERNAL
%token FILTER
%token FIXED
%token FLAT
%token FLOAT
%token FOR
%token FVEC2
%token FVEC3
%token FVEC4
%token GOTO
%token HALF
%token HIGH_PRECISION
%token HVEC2
%token HVEC3
%token HVEC4
%token IF
%token IIMAGE1D
%token IIMAGE1DARRAY
%token IIMAGE2D
%token IIMAGE2DARRAY
%token IIMAGE2DMS
%token IIMAGE2DMSARRAY
%token IIMAGE2DRECT
%token IIMAGE3D
%token IIMAGEBUFFER
%token IIMAGECUBE
%token IIMAGECUBEARRAY
%token IMAGE1D
%token IMAGE1DARRAY
%token IMAGE1DARRAYSHADOW
%token IMAGE1DSHADOW
%token IMAGE2D
%token IMAGE2DARRAY
%token IMAGE2DMS
%token IMAGE2DMSARRAY
%token IMAGE2DRECT
%token IMAGE2DARRAYSHADOW
%token IMAGE2DSHADOW
%token IMAGE3D
%token IMAGEBUFFER
%token IMAGECUBE
%token IMAGECUBEARRAY
%token IN
%token _INLINE
%token INOUT
%token INPUT
%token INT
%token INTERFACE
%token INVARIANT
%token ISAMPLER1D
%token ISAMPLER1DARRAY
%token ISAMPLER2D
%token ISAMPLER2DARRAY
%token ISAMPLER2DMS
%token ISAMPLER2DMSARRAY
%token ISAMPLER2DRECT
%token ISAMPLER3D
%token ISAMPLERBUFFER
%token ISAMPLERCUBE
%token ISAMPLERCUBEARRAY
%token IVEC2
%token IVEC3
%token IVEC4
%token LAYOUT
%token LONG
%token LOW_PRECISION
%token MAT2
%token MAT2X2
%token MAT2X3
%token MAT2X4
%token MAT3
%token MAT3X2
%token MAT3X3
%token MAT3X4
%token MAT4
%token MAT4X2
%token MAT4X3
%token MAT4X4
%token MEDIUM_PRECISION
%token NAMESPACE
%token NOINLINE
%token NOPERSPECTIVE
%token OUT
%token OUTPUT
%token PACKED
%token PARTITION
%token PATCH
%token PRECISE
%token PRECISION
%token PUBLIC
%token READONLY
%token RESOURCE
%token RESTRICT
%token RETURN
%token SAMPLE
%token SAMPLER1D
%token SAMPLER1DARRAY
%token SAMPLER1DARRAYSHADOW
%token SAMPLER1DSHADOW
%token SAMPLER2D
%token SAMPLER2DARRAY
%token SAMPLER2DARRAYSHADOW
%token SAMPLER2DMS
%token SAMPLER2DMSARRAY
%token SAMPLER2DRECT
%token SAMPLER2DRECTSHADOW
%token SAMPLER2DSHADOW
%token SAMPLER3D
%token SAMPLER3DRECT
%token SAMPLERBUFFER
%token SAMPLERCUBE
%token SAMPLERCUBEARRAY
%token SAMPLERCUBEARRAYSHADOW
%token SAMPLERCUBESHADOW
%token SHARED
%token SHORT
%token SIZEOF
%token SMOOTH
%token STATIC
%token STRUCT
%token SUBROUTINE
%token SUPERP
%token SWITCH
%token TEMPLATE
%token THIS
%token TYPEDEF
%token UIMAGE1D
%token UIMAGE1DARRAY
%token UIMAGE2D
%token UIMAGE2DARRAY
%token UIMAGE2DMS
%token UIMAGE2DMSARRAY
%token UIMAGE2DRECT
%token UIMAGE3D
%token UIMAGEBUFFER
%token UIMAGECUBE
%token UIMAGECUBEARRAY
%token UINT
%token UNIFORM
%token UNION
%token UNSIGNED
%token USAMPLER1D
%token USAMPLER1DARRAY
%token USAMPLER2D
%token USAMPLER2DARRAY
%token USAMPLER2DMS
%token USAMPLER2DMSARRAY
%token USAMPLER2DRECT
%token USAMPLER3D
%token USAMPLERBUFFER
%token USAMPLERCUBE
%token USAMPLERCUBEARRAY
%token USING
%token UVEC2
%token UVEC3
%token UVEC4
%token VARYING
%token VEC2
%token VEC3
%token VEC4
%token VOID
%token VOLATILE
%token WHILE
%token WRITEONLY
// end of specification keywords

// Extension keywords
%token SAMPLEREXTERNAL

// Preprocessor-only keywords
%token DEFINE
%token UNDEF
%token IFDEF
%token IFNDEF
%token ELIF
%token ENDIF
%token ERROR
%token PRAGMA
%token EXTENSION
%token VERSION
%token LINE
%token ALL
%token REQUIRE
%token ENABLE
%token WARN
%token DISABLE

%token KEYWORDS_END // mark; not a real token

%token IDENTIFIER TYPE_NAME FLOATCONSTANT INTCONSTANT UINTCONSTANT BOOLCONSTANT
// -- not parseable using a LR parser, we use IDENTIFIER instead: %token FIELD_SELECTION
%token LEFT_OP RIGHT_OP
%token INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP XOR_OP MUL_ASSIGN DIV_ASSIGN ADD_ASSIGN
%token MOD_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN XOR_ASSIGN OR_ASSIGN
%token SUB_ASSIGN
%token LEFT_PAREN RIGHT_PAREN LEFT_BRACKET RIGHT_BRACKET LEFT_BRACE RIGHT_BRACE DOT
%token COMMA COLON EQUAL SEMICOLON BANG DASH TILDE PLUS STAR SLASH PERCENT
%token LEFT_ANGLE RIGHT_ANGLE VERTICAL_BAR CARET AMPERSAND QUESTION

// intrinsics
%token BARRIER
%token INTRINSIC

%type<lookup> IDENTIFIER
%type<v> INTCONSTANT UINTCONSTANT FLOATCONSTANT BOOLCONSTANT

/* preprocessor tokens: */
%token HASH
%token WHITESPACE
%token NEWLINE
%token PPNUMBER
%token BACKSLASH
%token INVALID_CHAR

%type<lookup> TYPE_NAME
%type<lookup> variable_identifier
%type<lookup> identifier_or_typename

%type<call_context> function_identifier

%type<expr_chain> function_argument_list
%type<expr_chain> assignment_expression_list

%type<expr> primary_expression postfix_expression integer_expression
%type<expr> function_call
%type<expr> unary_expression
%type<expr> multiplicative_expression
%type<expr> additive_expression
%type<expr> shift_expression
%type<expr> relational_expression
%type<expr> equality_expression
%type<expr> and_expression
%type<expr> exclusive_or_expression
%type<expr> inclusive_or_expression
%type<expr> logical_and_expression
%type<expr> logical_xor_expression
%type<expr> logical_or_expression
%type<expr> conditional_expression
%type<expr> assignment_expression
%type<expr> initializer
%type<expr> expression
%type<expr> constant_expression

%type<expr_flavour> assignment_operator
%type<intrinsic> INTRINSIC

%type<func_proto> function_prototype

%type<decl> single_declaration
%type<init_declarator> init_declarator
%type<decl_chain> init_declarator_list
%type<symbol_list> identifier_list

%type<statement> declaration
%type<statement> declaration_statement
%type<statement> simple_statement
%type<statement> statement
%type<statement> statement_no_new_scope
%type<statement> compound_statement_no_new_scope
%type<statement_chain> statement_list switch_statement_list
%type<statement> expression_statement
%type<statement> selection_statement
%type<statement> iteration_statement
%type<statement> switch_statement
%type<statement> case_label
%type<statement> jump_statement
%type<statement> translation_unit external_declaration function_definition
%type<statement> statement_with_scope compound_statement_with_scope
%type<statement> for_init_statement
%type<statement> condition
%type<statement> conditionopt

%type<selection_rest> selection_rest_statement
%type<for_rest> for_rest_statement

%type<statement_chain> struct_declaration_list struct_declaration_list_inner struct_declarator_list
%type<statement> struct_declaration

%type<type> struct_specifier type_specifier type_specifier_nonarray
%type<expr_chain> array_specifier

%type<storage_qual>   storage_qualifier
%type<auxiliary_qual> auxiliary_qualifier
%type<memory_qual>    memory_qualifier
%type<interp_qual>    interpolation_qualifier
%type<prec_qual>      precision_qualifier
%type<lq_id>          layout_qualifier_id
%type<lq_id_list>     layout_qualifier_id_list
%type<lq_id_list>     layout_qualifier

%type<qualifier> single_type_qualifier
%type<qual_list> type_qualifier

%type<param> parameter_declaration
%type<param_list> parameter_declaration_list parameter_declaration_list_opt

%type<declarator> declarator

%type<fs_type> fully_specified_type

%type<v> unary_operator

%%

variable_identifier
   : IDENTIFIER
   ;

primary_expression
   : variable_identifier                { validate_symbol_lookup($1.symbol, $1.name);
                                          $$ = glsl_expr_construct_instance(g_LineNumber, $1.symbol); }
   | INTCONSTANT                        { $$ = glsl_expr_construct_const_value(g_LineNumber, PRIM_INT,  $1); }
   | UINTCONSTANT                       { $$ = glsl_expr_construct_const_value(g_LineNumber, PRIM_UINT, $1); }
   | FLOATCONSTANT                      { $$ = glsl_expr_construct_const_value(g_LineNumber, PRIM_FLOAT,$1); }
   | BOOLCONSTANT                       { $$ = glsl_expr_construct_const_value(g_LineNumber, PRIM_BOOL, $1); }
   | LEFT_PAREN expression RIGHT_PAREN  { $$ = $2; }
   ;

postfix_expression
   : primary_expression
   | postfix_expression LEFT_BRACKET integer_expression RIGHT_BRACKET
                                                      { $$ = glsl_expr_construct_subscript(g_LineNumber, $1, $3); }
   | function_call
// -- not parseable using an LR parser, we use IDENTIFIER instead:
// | postfix_expression DOT FIELD_SELECTION         { $$ = glsl_expr_construct_field_selector(g_LineNumber, $1, $3); }
/* We need to force the lexer to return IDENTIFIER in case struct/block members have the same names as types:
   ('force_identifier' is placed before DOT to ensure its semantic action is taken before IDENTIFIER is lexed as the lookahead token.) */
   | postfix_expression force_identifier DOT IDENTIFIER unforce_identifier { $$ = glsl_expr_construct_field_selector(g_LineNumber, $1, $4.name); }
   | postfix_expression INC_OP                                             { $$ = glsl_expr_construct_unary_op(EXPR_POST_INC, g_LineNumber, $1); }
   | postfix_expression DEC_OP                                             { $$ = glsl_expr_construct_unary_op(EXPR_POST_DEC, g_LineNumber, $1); }
   ;

integer_expression
   : expression
   ;

function_call
   : function_identifier LEFT_PAREN function_argument_list RIGHT_PAREN {
      switch($1.flavour) {
      case CALL_CONTEXT_FUNCTION:
         $$ = glsl_expr_construct_function_call   (g_LineNumber, $1.u.function.symbol,   $3);
         break;
      case CALL_CONTEXT_CONSTRUCTOR:
         $$ = glsl_expr_construct_constructor_call(g_LineNumber, $1.u.constructor.type,  $3);
         break;
      case CALL_CONTEXT_INTRINSIC:
         $$ = glsl_intrinsic_construct_expr       (g_LineNumber, $1.u.intrinsic.flavour, $3);
         break;
      default:
         unreachable();
         break;
      }
   }
   /* Here 'force_identifier' and 'unforce_identifier' are needed to avoid a
      conflict with the third derivation rule for 'postfix_expression' */
   | postfix_expression force_identifier DOT function_identifier unforce_identifier LEFT_PAREN RIGHT_PAREN {
       assert($4.flavour == CALL_CONTEXT_FUNCTION);
       $$ = glsl_expr_construct_method_call(g_LineNumber, $1, $4.u.function.symbol);
   }
   ;

force_identifier
   : { PS->force_identifier = true; }
   ;

unforce_identifier
   : { PS->force_identifier = false; }
   ;

function_argument_list
   : /* empty */                 { $$ = glsl_expr_chain_create(); }
   | VOID                        { $$ = glsl_expr_chain_create(); }
   | assignment_expression_list  { $$ = $1; }
   ;

assignment_expression_list
   : assignment_expression            { $$ = glsl_expr_chain_append(glsl_expr_chain_create(), $1); }
   | assignment_expression_list COMMA assignment_expression { $$ = glsl_expr_chain_append($1, $3); }
   ;

/* Grammar Note: Constructors look like functions, but lexical analysis recognized most of them as */
/* keywords. They are now recognized through type_specifier. */
function_identifier
   : type_specifier                                   { $$.flavour             = CALL_CONTEXT_CONSTRUCTOR;
                                                        $$.u.constructor.type  = $1;
                                                      }
   | IDENTIFIER  /* or FIELD_SELECTION */             { validate_symbol_lookup($1.symbol, $1.name);
                                                        $$.flavour             = CALL_CONTEXT_FUNCTION;
                                                        $$.u.function.symbol   = $1.symbol;
                                                      }
   | INTRINSIC   /* Extensions: */                    { $$.flavour             = CALL_CONTEXT_INTRINSIC;
                                                        $$.u.intrinsic.flavour = $1;
                                                      }
   ;

unary_expression
   : postfix_expression
   | INC_OP unary_expression                          { $$ = glsl_expr_construct_unary_op(EXPR_PRE_INC, g_LineNumber, $2); }
   | DEC_OP unary_expression                          { $$ = glsl_expr_construct_unary_op(EXPR_PRE_DEC, g_LineNumber, $2); }
   | unary_operator unary_expression                  { $$ = glsl_expr_construct_unary_op($1, g_LineNumber, $2); }
   /* Grammar Note: No traditional style type casts. */
   ;

unary_operator
   : PLUS                                             { $$ = EXPR_ADD; }
   | DASH                                             { $$ = EXPR_ARITH_NEGATE; }
   | BANG                                             { $$ = EXPR_LOGICAL_NOT; }
   | TILDE                                            { $$ = EXPR_BITWISE_NOT; }
   /* Grammar Note: No '*' or '&' unary ops. Pointers are not supported. */
   ;

multiplicative_expression
   : unary_expression
   | multiplicative_expression STAR unary_expression    { $$ = glsl_expr_construct_binary_op_arithmetic(EXPR_MUL, g_LineNumber, $1, $3); }
   | multiplicative_expression SLASH unary_expression   { $$ = glsl_expr_construct_binary_op_arithmetic(EXPR_DIV, g_LineNumber, $1, $3); }
   | multiplicative_expression PERCENT unary_expression { $$ = glsl_expr_construct_binary_op_arithmetic(EXPR_REM, g_LineNumber, $1, $3); }
   ;

additive_expression
   : multiplicative_expression
   | additive_expression PLUS multiplicative_expression { $$ = glsl_expr_construct_binary_op_arithmetic(EXPR_ADD, g_LineNumber, $1, $3); }
   | additive_expression DASH multiplicative_expression { $$ = glsl_expr_construct_binary_op_arithmetic(EXPR_SUB, g_LineNumber, $1, $3); }
   ;

shift_expression
   : additive_expression
   | shift_expression LEFT_OP additive_expression     { $$ = glsl_expr_construct_binary_op_shift(EXPR_SHL, g_LineNumber, $1, $3); }
   | shift_expression RIGHT_OP additive_expression    { $$ = glsl_expr_construct_binary_op_shift(EXPR_SHR, g_LineNumber, $1, $3); }
   ;

relational_expression
   : shift_expression
   | relational_expression LEFT_ANGLE shift_expression { $$ = glsl_expr_construct_binary_op_relational(EXPR_LESS_THAN, g_LineNumber, $1, $3); }
   | relational_expression RIGHT_ANGLE shift_expression { $$ = glsl_expr_construct_binary_op_relational(EXPR_GREATER_THAN, g_LineNumber, $1, $3); }
   | relational_expression LE_OP shift_expression { $$ = glsl_expr_construct_binary_op_relational(EXPR_LESS_THAN_EQUAL, g_LineNumber, $1, $3); }
   | relational_expression GE_OP shift_expression { $$ = glsl_expr_construct_binary_op_relational(EXPR_GREATER_THAN_EQUAL, g_LineNumber, $1, $3); }
   ;

equality_expression
   : relational_expression
   | equality_expression EQ_OP relational_expression { $$ = glsl_expr_construct_binary_op_equality(EXPR_EQUAL, g_LineNumber, $1, $3); }
   | equality_expression NE_OP relational_expression { $$ = glsl_expr_construct_binary_op_equality(EXPR_NOT_EQUAL, g_LineNumber, $1, $3); }
   ;

and_expression
   : equality_expression
   | and_expression AMPERSAND equality_expression     { $$ = glsl_expr_construct_binary_op_bitwise(EXPR_BITWISE_AND, g_LineNumber, $1, $3); }
   ;

exclusive_or_expression
   : and_expression
   | exclusive_or_expression CARET and_expression     { $$ = glsl_expr_construct_binary_op_bitwise(EXPR_BITWISE_XOR, g_LineNumber, $1, $3); }
   ;

inclusive_or_expression
   : exclusive_or_expression
   | inclusive_or_expression VERTICAL_BAR exclusive_or_expression { $$ = glsl_expr_construct_binary_op_bitwise(EXPR_BITWISE_OR, g_LineNumber, $1, $3); }
   ;

logical_and_expression
   : inclusive_or_expression
   | logical_and_expression AND_OP inclusive_or_expression { $$ = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_AND, g_LineNumber, $1, $3); }
   ;

logical_xor_expression
   : logical_and_expression
   | logical_xor_expression XOR_OP logical_and_expression { $$ = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_XOR, g_LineNumber, $1, $3); }
   ;

logical_or_expression
   : logical_xor_expression
   | logical_or_expression OR_OP logical_xor_expression { $$ = glsl_expr_construct_binary_op_logical(EXPR_LOGICAL_OR, g_LineNumber, $1, $3); }
   ;

conditional_expression
   : logical_or_expression
   | logical_or_expression QUESTION expression COLON assignment_expression { $$ = glsl_expr_construct_cond_op(g_LineNumber, $1, $3, $5); }
   ;

assignment_expression
   : conditional_expression
   | unary_expression assignment_operator assignment_expression
      {
         Expr *expr;
         switch ($2)
         {
            case EXPR_MUL:
            case EXPR_DIV:
            case EXPR_REM:
            case EXPR_ADD:
            case EXPR_SUB:
               expr = glsl_expr_construct_binary_op_arithmetic($2, g_LineNumber, $1, $3);
               break;
            case EXPR_SHL:
            case EXPR_SHR:
               expr = glsl_expr_construct_binary_op_shift($2, g_LineNumber, $1, $3);
               break;
            case EXPR_BITWISE_AND:
            case EXPR_BITWISE_XOR:
            case EXPR_BITWISE_OR:
               expr = glsl_expr_construct_binary_op_bitwise($2, g_LineNumber, $1, $3);
               break;
            case EXPR_ASSIGN:
               expr = $3;
               break;
            default:
               unreachable();
         }
         $$ = glsl_expr_construct_assign_op(g_LineNumber, $1, expr);
      }
   ;

assignment_operator
   : EQUAL         { $$ = EXPR_ASSIGN; }
   | MUL_ASSIGN    { $$ = EXPR_MUL; }
   | DIV_ASSIGN    { $$ = EXPR_DIV; }
   | MOD_ASSIGN    { $$ = EXPR_REM; }
   | ADD_ASSIGN    { $$ = EXPR_ADD; }
   | SUB_ASSIGN    { $$ = EXPR_SUB; }
   | LEFT_ASSIGN   { $$ = EXPR_SHL; }
   | RIGHT_ASSIGN  { $$ = EXPR_SHR; }
   | AND_ASSIGN    { $$ = EXPR_BITWISE_AND; }
   | XOR_ASSIGN    { $$ = EXPR_BITWISE_XOR; }
   | OR_ASSIGN     { $$ = EXPR_BITWISE_OR; }
   ;

expression
   : assignment_expression
   | expression COMMA assignment_expression { $$ = glsl_expr_construct_sequence(g_LineNumber, $1, $3); }
   ;

constant_expression
   : conditional_expression
   ;

declaration
   : function_prototype SEMICOLON           {  glsl_commit_singleton_function_declaration(PS->symbol_table, $1.name, $1.type, false, PS->user_code);
                                               // Don't store these in the AST.
                                               $$ = glsl_statement_construct(STATEMENT_NULL, g_LineNumber);
                                            }
   | init_declarator_list SEMICOLON         { $$ = glsl_statement_construct_decl_list(g_LineNumber, $1.ch); }
   | PRECISION precision_qualifier type_specifier SEMICOLON
                                            { /* update precision table entry */
                                               glsl_prec_modify_prec( PS->precision_table, $3, $2 );
                                               $$ = glsl_statement_construct_precision(g_LineNumber, $2, $3);
                                            }
   | type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE SEMICOLON
      { $$ = process_block_decl(PS, $1, $2.name, $4, NULL, NULL); }
   | type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE IDENTIFIER SEMICOLON
      { $$ = process_block_decl(PS, $1, $2.name, $4, $6.name, NULL); }
   | type_qualifier IDENTIFIER LEFT_BRACE struct_declaration_list RIGHT_BRACE IDENTIFIER array_specifier SEMICOLON
      { $$ = process_block_decl(PS, $1, $2.name, $4, $6.name, $7); }
   | type_qualifier SEMICOLON
      {
         $$ = glsl_statement_construct_qualifier_default(g_LineNumber, $1);
         qualifiers_process_default($1, PS->symbol_table, &PS->dflt);
      }
   | type_qualifier identifier_list SEMICOLON
      { $$ = glsl_statement_construct_qualifier_augment(g_LineNumber, $1, $2); }
   ;

identifier_list
   : IDENTIFIER
      {
         validate_symbol_lookup($1.symbol, $1.name);
         $$ = glsl_symbol_list_new();
         glsl_symbol_list_append($$, $1.symbol);
      }
   | identifier_list COMMA IDENTIFIER
      {
         validate_symbol_lookup($3.symbol, $3.name);
         glsl_symbol_list_append($1, $3.symbol);
      }
   ;

function_prototype
   : fully_specified_type IDENTIFIER LEFT_PAREN parameter_declaration_list_opt RIGHT_PAREN
      {
         $$.name = $2.name;
         $$.type = glsl_build_function_type($1.quals, $1.type, $4);
      }
   ;

parameter_declaration_list_opt
   : /* empty */ { $$ = NULL; }
   | parameter_declaration_list
   ;

parameter_declaration_list
   : parameter_declaration { $$ = malloc_fast(sizeof(ParamList)); $$->p = $1; $$->next = NULL; }
   | parameter_declaration_list COMMA parameter_declaration
      { ParamList *l = $1;
        while (l->next != NULL) l = l->next;
        l->next = malloc_fast(sizeof(ParamList));
        l = l->next;
        l->p = $3;
        l->next = NULL;
        $$ = $1;
      }
   ;

declarator
   : identifier_or_typename                 { $$.name = $1.name; $$.size = NULL; }
   | identifier_or_typename array_specifier { $$.name = $1.name; $$.size = $2; }
   ;

identifier_or_typename
   : IDENTIFIER
   | TYPE_NAME
   ;

parameter_declaration
   : fully_specified_type declarator
      { $$ = malloc_fast(sizeof(Param)); $$->name = $2.name; $$->type = $1.type; $$->quals = $1.quals; $$->size = $2.size; }
   | fully_specified_type
      { $$ = malloc_fast(sizeof(Param)); $$->name = NULL;    $$->type = $1.type; $$->quals = $1.quals; $$->size = NULL; }
   ;

init_declarator
   : declarator                   { $$.name = $1.name; $$.size = $1.size; $$.init = NULL; }
   | declarator EQUAL initializer { $$.name = $1.name; $$.size = $1.size; $$.init = $3; }
   ;

init_declarator_list
   : single_declaration
      {
         $$.quals       = $1.quals;
         $$.type        = $1.type;
         $$.ch = glsl_statement_chain_create();
         if ($1.s != NULL)
            $$.ch = glsl_statement_chain_append($$.ch, $1.s);
      }
   | init_declarator_list COMMA init_declarator
      {
         Symbol *symbol = glsl_commit_variable_instance(PS->symbol_table, PS->precision_table, &PS->dflt,
                                                        $1.quals, $1.type, $3.name, $3.size, $3.init);
         $$.quals       = $1.quals;
         $$.type        = $1.type;
         $$.ch          = glsl_statement_chain_append($1.ch,
                                                      glsl_statement_construct_var_decl(g_LineNumber, $1.quals, $1.type, symbol, $3.init));
      }
   ;

single_declaration
   : fully_specified_type     // This matches struct declarations, but also admits rubbish like "int , x".
      {
         $$.quals = $1.quals;
         $$.type  = $1.type;

         Symbol *symbol = glsl_commit_variable_instance(PS->symbol_table, PS->precision_table, &PS->dflt,
                                                        $1.quals, $1.type, NULL, NULL, NULL);
         assert(symbol == NULL);
         $$.s = NULL;
      }
   | fully_specified_type init_declarator
      {
         $$.quals = $1.quals;
         $$.type  = $1.type;

         Symbol *symbol = glsl_commit_variable_instance(PS->symbol_table, PS->precision_table, &PS->dflt,
                                                        $1.quals, $1.type, $2.name, $2.size, $2.init);
         $$.s           = glsl_statement_construct_var_decl(g_LineNumber, $1.quals, $1.type, symbol, $2.init);
      }
   /* Grammar Note: No 'enum', or 'typedef'. */
   ;

fully_specified_type
   : type_specifier                { $$.quals = NULL; $$.type = $1; }
   | type_qualifier type_specifier { $$.quals = $1;   $$.type = $2; }
   ;

invariant_qualifier
   : INVARIANT
   ;

interpolation_qualifier
   : SMOOTH        { $$ = INTERP_SMOOTH;        }
   | NOPERSPECTIVE { $$ = INTERP_NOPERSPECTIVE; }
   | FLAT          { $$ = INTERP_FLAT;          }
   ;

layout_qualifier
   : LAYOUT LEFT_PAREN layout_qualifier_id_list RIGHT_PAREN    { $$ = $3; }
   ;

layout_qualifier_id_list
   : layout_qualifier_id                                 { $$ = glsl_lq_id_list_new($1); }
   | layout_qualifier_id_list COMMA layout_qualifier_id  { $$ = glsl_lq_id_list_append($1, $3); }
   ;

layout_qualifier_id
   : IDENTIFIER
      {
         $$ = malloc_fast(sizeof(LayoutID));
         $$->flavour = LQ_FLAVOUR_PLAIN;
         const struct layout_data *d = glsl_layout_lookup($1.name, strlen($1.name));
         if (d == NULL)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layout \'%s\' not valid", $1.name);
         $$->id = d->lq;
      }
   | IDENTIFIER EQUAL INTCONSTANT
      {
         $$ = malloc_fast(sizeof(LayoutID));
         $$->flavour = LQ_FLAVOUR_INT;
         $$->argument = $3;
         const struct layout_data *d = glsl_layout_lookup($1.name, strlen($1.name));
         if (d == NULL)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layout \'%s\' not valid", $1.name);
         $$->id = d->lq;
      }
   | IDENTIFIER EQUAL UINTCONSTANT
      {
         $$ = malloc_fast(sizeof(LayoutID));
         $$->flavour = LQ_FLAVOUR_UINT;
         $$->argument = $3;
         const struct layout_data *d = glsl_layout_lookup($1.name, strlen($1.name));
         if (d == NULL)
            glsl_compile_error(ERROR_CUSTOM, 15, g_LineNumber, "layout \'%s\' not valid", $1.name);
         $$->id = d->lq;
      }
   | SHARED
      {
         $$ = malloc_fast(sizeof(LayoutID));
         $$->flavour = LQ_FLAVOUR_PLAIN;
         $$->id = LQ_SHARED;
      }
   ;

precise_qualifier
   : PRECISE
   ;

type_qualifier
   : single_type_qualifier                { $$ = qual_list_new($1); }
   | type_qualifier single_type_qualifier { $$ = qual_list_append($1, $2); }
   ;

single_type_qualifier
   : storage_qualifier        { $$ = new_qual_storage($1); }
   | auxiliary_qualifier      { $$ = new_qual_auxiliary($1); }
   | memory_qualifier         { $$ = new_qual_memory($1); }
   | layout_qualifier         { $$ = new_qual_layout($1); }
   | precision_qualifier      { $$ = new_qual_prec($1); }
   | interpolation_qualifier  { $$ = new_qual_interp($1); }
   | invariant_qualifier      { $$ = new_qual_invariant(); }
   | precise_qualifier        { $$ = new_qual_precise(); }
   ;

storage_qualifier
   : CONST           { $$ = STORAGE_CONST; }
   | IN              { $$ = STORAGE_IN; }
   | OUT             { $$ = STORAGE_OUT; }
   | INOUT           { $$ = STORAGE_INOUT; }
   | UNIFORM         { $$ = STORAGE_UNIFORM; }
   | BUFFER          { $$ = STORAGE_BUFFER; }
   | SHARED          { $$ = STORAGE_SHARED; }
   /* These became reserved keywords in GLSL_300ES. To handle GLSL_100ES, we deviate from the spec grammar: */
   | ATTRIBUTE
      {
         assert(g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1));

         if (g_ShaderFlavour != SHADER_VERTEX)
            glsl_compile_error(ERROR_LEXER_PARSER, 1, g_LineNumber, "\"attribute\" only valid in vertex shaders");
         else
            $$ = STORAGE_IN;
      }
   | VARYING
      {
         assert(g_ShaderVersion == GLSL_SHADER_VERSION(1,0,1));
         assert(g_ShaderFlavour == SHADER_VERTEX || g_ShaderFlavour == SHADER_FRAGMENT);

         if (g_ShaderFlavour == SHADER_VERTEX)
            $$ = STORAGE_OUT;
         else
            $$ = STORAGE_IN;
      }
   ;

auxiliary_qualifier
   : CENTROID  { $$ = AUXILIARY_CENTROID; }
   | PATCH     { $$ = AUXILIARY_PATCH; }
   | SAMPLE    { $$ = AUXILIARY_SAMPLE; }
   ;

memory_qualifier
   : COHERENT  { $$ = MEMORY_COHERENT; }
   | VOLATILE  { $$ = MEMORY_VOLATILE; }
   | RESTRICT  { $$ = MEMORY_RESTRICT; }
   | READONLY  { $$ = MEMORY_READONLY; }
   | WRITEONLY { $$ = MEMORY_WRITEONLY; }
   ;

type_specifier
   : type_specifier_nonarray
   | type_specifier_nonarray array_specifier { $$ = glsl_build_array_type($1, $2); }
   ;

array_specifier
   : LEFT_BRACKET RIGHT_BRACKET                                     { $$ = glsl_expr_chain_append(glsl_expr_chain_create(), NULL); }
   | LEFT_BRACKET constant_expression RIGHT_BRACKET                 { $$ = glsl_expr_chain_append(glsl_expr_chain_create(), $2); }
   | array_specifier LEFT_BRACKET RIGHT_BRACKET                     { $$ = glsl_expr_chain_append($1, NULL); }
   | array_specifier LEFT_BRACKET constant_expression RIGHT_BRACKET { $$ = glsl_expr_chain_append($1, $3); }
   ;

type_specifier_nonarray
   : VOID                    { $$ = &primitiveTypes[PRIM_VOID]; }
   | FLOAT                   { $$ = &primitiveTypes[PRIM_FLOAT]; }
   | INT                     { $$ = &primitiveTypes[PRIM_INT]; }
   | UINT                    { $$ = &primitiveTypes[PRIM_UINT]; }
   | BOOL                    { $$ = &primitiveTypes[PRIM_BOOL]; }
   | VEC2                    { $$ = &primitiveTypes[PRIM_VEC2]; }
   | VEC3                    { $$ = &primitiveTypes[PRIM_VEC3]; }
   | VEC4                    { $$ = &primitiveTypes[PRIM_VEC4]; }
   | BVEC2                   { $$ = &primitiveTypes[PRIM_BVEC2]; }
   | BVEC3                   { $$ = &primitiveTypes[PRIM_BVEC3]; }
   | BVEC4                   { $$ = &primitiveTypes[PRIM_BVEC4]; }
   | IVEC2                   { $$ = &primitiveTypes[PRIM_IVEC2]; }
   | IVEC3                   { $$ = &primitiveTypes[PRIM_IVEC3]; }
   | IVEC4                   { $$ = &primitiveTypes[PRIM_IVEC4]; }
   | UVEC2                   { $$ = &primitiveTypes[PRIM_UVEC2]; }
   | UVEC3                   { $$ = &primitiveTypes[PRIM_UVEC3]; }
   | UVEC4                   { $$ = &primitiveTypes[PRIM_UVEC4]; }
   | MAT2                    { $$ = &primitiveTypes[PRIM_MAT2]; }
   | MAT3                    { $$ = &primitiveTypes[PRIM_MAT3]; }
   | MAT4                    { $$ = &primitiveTypes[PRIM_MAT4]; }
   | MAT2X2                  { $$ = &primitiveTypes[PRIM_MAT2]; }
   | MAT2X3                  { $$ = &primitiveTypes[PRIM_MAT2X3]; }
   | MAT2X4                  { $$ = &primitiveTypes[PRIM_MAT2X4]; }
   | MAT3X2                  { $$ = &primitiveTypes[PRIM_MAT3X2]; }
   | MAT3X3                  { $$ = &primitiveTypes[PRIM_MAT3]; }
   | MAT3X4                  { $$ = &primitiveTypes[PRIM_MAT3X4]; }
   | MAT4X2                  { $$ = &primitiveTypes[PRIM_MAT4X2]; }
   | MAT4X3                  { $$ = &primitiveTypes[PRIM_MAT4X3]; }
   | MAT4X4                  { $$ = &primitiveTypes[PRIM_MAT4]; }
   | ATOMIC_UINT             { $$ = &primitiveTypes[PRIM_ATOMIC_UINT]; }
   | SAMPLER2D               { $$ = &primitiveTypes[PRIM_SAMPLER2D]; }
   | SAMPLER3D               { $$ = &primitiveTypes[PRIM_SAMPLER3D]; }
   | SAMPLERCUBE             { $$ = &primitiveTypes[PRIM_SAMPLERCUBE]; }
   | SAMPLER2DSHADOW         { $$ = &primitiveTypes[PRIM_SAMPLER2DSHADOW]; }
   | SAMPLERCUBESHADOW       { $$ = &primitiveTypes[PRIM_SAMPLERCUBESHADOW]; }
   | SAMPLER2DARRAY          { $$ = &primitiveTypes[PRIM_SAMPLER2DARRAY]; }
   | SAMPLER2DARRAYSHADOW    { $$ = &primitiveTypes[PRIM_SAMPLER2DARRAYSHADOW]; }
   | SAMPLERBUFFER           { $$ = &primitiveTypes[PRIM_SAMPLERBUFFER]; }
   | SAMPLERCUBEARRAY        { $$ = &primitiveTypes[PRIM_SAMPLERCUBEARRAY]; }
   | SAMPLERCUBEARRAYSHADOW  { $$ = &primitiveTypes[PRIM_SAMPLERCUBEARRAYSHADOW]; }
   | ISAMPLER2D              { $$ = &primitiveTypes[PRIM_ISAMPLER2D]; }
   | ISAMPLER3D              { $$ = &primitiveTypes[PRIM_ISAMPLER3D]; }
   | ISAMPLERCUBE            { $$ = &primitiveTypes[PRIM_ISAMPLERCUBE]; }
   | ISAMPLER2DARRAY         { $$ = &primitiveTypes[PRIM_ISAMPLER2DARRAY]; }
   | ISAMPLERBUFFER          { $$ = &primitiveTypes[PRIM_ISAMPLERBUFFER]; }
   | ISAMPLERCUBEARRAY       { $$ = &primitiveTypes[PRIM_ISAMPLERCUBEARRAY]; }
   | USAMPLER2D              { $$ = &primitiveTypes[PRIM_USAMPLER2D]; }
   | USAMPLER3D              { $$ = &primitiveTypes[PRIM_USAMPLER3D]; }
   | USAMPLERCUBE            { $$ = &primitiveTypes[PRIM_USAMPLERCUBE]; }
   | USAMPLER2DARRAY         { $$ = &primitiveTypes[PRIM_USAMPLER2DARRAY]; }
   | USAMPLERBUFFER          { $$ = &primitiveTypes[PRIM_USAMPLERBUFFER]; }
   | USAMPLERCUBEARRAY       { $$ = &primitiveTypes[PRIM_USAMPLERCUBEARRAY]; }
   | SAMPLER2DMS             { $$ = &primitiveTypes[PRIM_SAMPLER2DMS]; }
   | ISAMPLER2DMS            { $$ = &primitiveTypes[PRIM_ISAMPLER2DMS]; }
   | USAMPLER2DMS            { $$ = &primitiveTypes[PRIM_USAMPLER2DMS]; }
   | SAMPLER2DMSARRAY        { $$ = &primitiveTypes[PRIM_SAMPLER2DMSARRAY]; }
   | ISAMPLER2DMSARRAY       { $$ = &primitiveTypes[PRIM_ISAMPLER2DMSARRAY]; }
   | USAMPLER2DMSARRAY       { $$ = &primitiveTypes[PRIM_USAMPLER2DMSARRAY]; }
   | IMAGE2D                 { $$ = &primitiveTypes[PRIM_IMAGE2D]; }
   | IIMAGE2D                { $$ = &primitiveTypes[PRIM_IIMAGE2D]; }
   | UIMAGE2D                { $$ = &primitiveTypes[PRIM_UIMAGE2D]; }
   | IMAGE3D                 { $$ = &primitiveTypes[PRIM_IMAGE3D]; }
   | IIMAGE3D                { $$ = &primitiveTypes[PRIM_IIMAGE3D]; }
   | UIMAGE3D                { $$ = &primitiveTypes[PRIM_UIMAGE3D]; }
   | IMAGECUBE               { $$ = &primitiveTypes[PRIM_IMAGECUBE]; }
   | IIMAGECUBE              { $$ = &primitiveTypes[PRIM_IIMAGECUBE]; }
   | UIMAGECUBE              { $$ = &primitiveTypes[PRIM_UIMAGECUBE]; }
   | IMAGE2DARRAY            { $$ = &primitiveTypes[PRIM_IMAGE2DARRAY]; }
   | IIMAGE2DARRAY           { $$ = &primitiveTypes[PRIM_IIMAGE2DARRAY]; }
   | UIMAGE2DARRAY           { $$ = &primitiveTypes[PRIM_UIMAGE2DARRAY]; }
   | IMAGEBUFFER             { $$ = &primitiveTypes[PRIM_IMAGEBUFFER]; }
   | IIMAGEBUFFER            { $$ = &primitiveTypes[PRIM_IIMAGEBUFFER]; }
   | UIMAGEBUFFER            { $$ = &primitiveTypes[PRIM_UIMAGEBUFFER]; }
   | UIMAGECUBEARRAY         { $$ = &primitiveTypes[PRIM_UIMAGECUBEARRAY]; }
   | IMAGECUBEARRAY          { $$ = &primitiveTypes[PRIM_IMAGECUBEARRAY]; }
   | IIMAGECUBEARRAY         { $$ = &primitiveTypes[PRIM_IIMAGECUBEARRAY]; }
   | SAMPLEREXTERNAL         { $$ = &primitiveTypes[PRIM_SAMPLEREXTERNALOES]; }
   | SAMPLER1D               { $$ = &primitiveTypes[PRIM_SAMPLER1D]; }
   | SAMPLER1DARRAY          { $$ = &primitiveTypes[PRIM_SAMPLER1DARRAY]; }
   | ISAMPLER1D              { $$ = &primitiveTypes[PRIM_ISAMPLER1D]; }
   | ISAMPLER1DARRAY         { $$ = &primitiveTypes[PRIM_ISAMPLER1DARRAY]; }
   | USAMPLER1D              { $$ = &primitiveTypes[PRIM_USAMPLER1D]; }
   | USAMPLER1DARRAY         { $$ = &primitiveTypes[PRIM_USAMPLER1DARRAY]; }
   | struct_specifier
   | TYPE_NAME               { $$ = $1.symbol->type; }
   // Note that yylex returns token TYPE_NAME iff $1.symbol->flavour==SYMBOL_TYPE.
   // It's NOT context-free information whether an IDENTIFIER is the name of a
   // type or of something else (e.g. a variable).
   ;

precision_qualifier
   : HIGH_PRECISION        { $$ = PREC_HIGHP;   }
   | MEDIUM_PRECISION      { $$ = PREC_MEDIUMP; }
   | LOW_PRECISION         { $$ = PREC_LOWP;    }
   ;

struct_specifier
   : STRUCT identifier_or_typename LEFT_BRACE struct_declaration_list RIGHT_BRACE
         { $$ = glsl_build_struct_type($2.name, $4); glsl_commit_struct_type(PS->symbol_table, $$); }
   | STRUCT LEFT_BRACE struct_declaration_list RIGHT_BRACE
         { $$ = glsl_build_struct_type(NULL, $3); }
   ;

struct_declaration_list
   :  { if(PS->in_struct)
           glsl_compile_error(ERROR_CUSTOM, 17, g_LineNumber, NULL);
        PS->in_struct = true;
      }
      struct_declaration_list_inner
      { PS->in_struct = false;
        $$ = $2;
      }
   ;

struct_declaration_list_inner
   : struct_declaration                               { $$ = glsl_statement_chain_append(glsl_statement_chain_create(), $1); }
   | struct_declaration_list_inner struct_declaration { $$ = glsl_statement_chain_append($1, $2); }
   ;

struct_declaration
   : type_specifier struct_declarator_list SEMICOLON
      { $$ = glsl_statement_construct_struct_decl(g_LineNumber, $1, NULL, $2); }
   | type_qualifier type_specifier struct_declarator_list SEMICOLON
      { $$ = glsl_statement_construct_struct_decl(g_LineNumber, $2, $1, $3); }
   ;

struct_declarator_list
   : declarator                              { $$ = glsl_statement_chain_append(glsl_statement_chain_create(),
                                                            glsl_statement_construct_struct_member_decl(g_LineNumber, $1.name, $1.size)); }
   | struct_declarator_list COMMA declarator { $$ = glsl_statement_chain_append($1,
                                                            glsl_statement_construct_struct_member_decl(g_LineNumber, $3.name, $3.size)); }
   ;

initializer
   : assignment_expression
   ;

declaration_statement
   : declaration
   ;

statement
   : compound_statement_with_scope
   | simple_statement
   ;

statement_no_new_scope
   : compound_statement_no_new_scope
   | simple_statement
   ;

statement_with_scope
   : { enter_scope(state); } compound_statement_no_new_scope { exit_scope(state); $$ = $2; }
   | { enter_scope(state); } simple_statement                { exit_scope(state); $$ = $2; }
   ;

/* Grammar Note: labeled statements for SWITCH only; 'goto' is not supported. */

simple_statement
   : declaration_statement
   | expression_statement
   | selection_statement
   | switch_statement
   | case_label
   | iteration_statement
   | jump_statement
   | BARRIER SEMICOLON        { $$ = glsl_statement_construct(STATEMENT_BARRIER, g_LineNumber); }
   ;

compound_statement_with_scope
   : LEFT_BRACE RIGHT_BRACE                            { $$ = glsl_statement_construct_compound(g_LineNumber, glsl_statement_chain_create()); }
   | LEFT_BRACE { enter_scope(state); } statement_list RIGHT_BRACE { exit_scope(state); $$ = glsl_statement_construct_compound(g_LineNumber, $3); }
   ;

compound_statement_no_new_scope
   : LEFT_BRACE RIGHT_BRACE                { $$ = glsl_statement_construct_compound(g_LineNumber, glsl_statement_chain_create()); }
   | LEFT_BRACE statement_list RIGHT_BRACE { $$ = glsl_statement_construct_compound(g_LineNumber, $2); }
   ;

statement_list
   : statement                            { $$ = glsl_statement_chain_append(glsl_statement_chain_create(), $1); }
   | statement_list statement             { $$ = glsl_statement_chain_append($1, $2); }
   ;

expression_statement
   : SEMICOLON                                         { $$ = glsl_statement_construct(STATEMENT_NULL, g_LineNumber); }
   | expression SEMICOLON                              { $$ = glsl_statement_construct_expr(g_LineNumber, $1); }
   ;

selection_statement
   : IF LEFT_PAREN expression RIGHT_PAREN selection_rest_statement { $$ = glsl_statement_construct_selection(g_LineNumber, $3, $5.t, $5.f); }
   ;

selection_rest_statement
   : statement_with_scope ELSE statement_with_scope    { $$.t = $1; $$.f = $3; }
   | statement_with_scope                              { $$.t = $1; $$.f = NULL; }
   ;

condition
   : expression                                        { $$ = glsl_statement_construct_expr(g_LineNumber, $1); }
   | fully_specified_type IDENTIFIER EQUAL initializer
      {
         Symbol *symbol = glsl_commit_variable_instance(PS->symbol_table, PS->precision_table, &PS->dflt,
                                                        $1.quals, $1.type, $2.name, NULL, $4);
         $$ = glsl_statement_construct_var_decl(g_LineNumber, $1.quals, $1.type, symbol, $4);
      }
   ;

switch_statement
   : SWITCH LEFT_PAREN expression RIGHT_PAREN { enter_scope(state); } LEFT_BRACE switch_statement_list RIGHT_BRACE
                                              { exit_scope(state); $$ = glsl_statement_construct_switch(g_LineNumber, $3, $7); }
   ;

switch_statement_list
   : /* nothing */   { $$ = glsl_statement_chain_create(); }
   | statement_list
   ;

case_label
   : CASE expression COLON                             { $$ = glsl_statement_construct_case(g_LineNumber, $2); }
   | DEFAULT COLON                                     { $$ = glsl_statement_construct(STATEMENT_DEFAULT, g_LineNumber); }
   ;

iteration_statement
   : WHILE LEFT_PAREN { enter_scope(state); } condition RIGHT_PAREN statement_no_new_scope { exit_scope(state); $$ = glsl_statement_construct_iterator_while(g_LineNumber, $4, $6); }
   | DO statement_with_scope WHILE LEFT_PAREN expression RIGHT_PAREN SEMICOLON { $$ = glsl_statement_construct_iterator_do_while(g_LineNumber, $2, $5); }
   | FOR LEFT_PAREN { enter_scope(state); } for_init_statement for_rest_statement RIGHT_PAREN statement_no_new_scope { exit_scope(state); $$ = glsl_statement_construct_iterator_for(g_LineNumber, $4, $5.cond_or_decl, $5.iter, $7); }
   ;

for_init_statement
   : expression_statement
   | declaration_statement
   ;

conditionopt
   : condition
   | /* empty */                                   { $$ = NULL; }
   ;

for_rest_statement
   : conditionopt SEMICOLON                        { $$.cond_or_decl = $1; $$.iter = NULL; }
   | conditionopt SEMICOLON expression             { $$.cond_or_decl = $1; $$.iter = $3; }
   ;

jump_statement
   : CONTINUE SEMICOLON                            { $$ = glsl_statement_construct(STATEMENT_CONTINUE, g_LineNumber); }
   | BREAK SEMICOLON                               { $$ = glsl_statement_construct(STATEMENT_BREAK,    g_LineNumber); }
   | RETURN SEMICOLON                              { $$ = glsl_statement_construct(STATEMENT_RETURN,   g_LineNumber); }
   | RETURN expression SEMICOLON                   { $$ = glsl_statement_construct_return_expr(g_LineNumber, $2); }
   | DISCARD SEMICOLON /* Fragment shader only. */ { $$ = glsl_statement_construct(STATEMENT_DISCARD,  g_LineNumber); }
   /* Grammar Note: No 'goto'. Gotos are not supported. */
   ;

translation_unit
   : external_declaration                     {
                                                 StatementChain *chain = glsl_statement_chain_create();
                                                 glsl_statement_chain_append(chain, $1);
                                                 $$ = glsl_statement_construct_ast(g_LineNumber, chain);
                                                 PS->astp = $$; // Save for calling function.
                                              }
   | translation_unit external_declaration    {
                                                 glsl_statement_chain_append($1->u.ast.decls, $2);
                                                 $$ = $1;
                                                 PS->astp = $$; // Save for calling function.
                                              }
   ;

external_declaration
   : function_definition
   | declaration
   ;

function_definition
   : function_prototype
      {
         enter_scope(state);
         g_InGlobalScope = false;
         glsl_instantiate_function_params(PS->symbol_table, $1.type);
         if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1)) enter_scope(state);
      }
                        compound_statement_no_new_scope
      {
         if (g_ShaderVersion == GLSL_SHADER_VERSION(1, 0, 1)) exit_scope(state);
         exit_scope(state);
         g_InGlobalScope = true;
         Symbol *f = glsl_commit_singleton_function_declaration(PS->symbol_table, $1.name, $1.type, true, PS->user_code);
         $$ = glsl_statement_construct_function_def(g_LineNumber, f, $3);
         glsl_insert_function_definition($$);
      }
   ;

%%
