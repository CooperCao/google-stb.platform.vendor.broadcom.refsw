/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_common.h"
#include "glsl_basic_block.h"
#include "glsl_basic_block_builder.h"
#include "glsl_basic_block_elim_dead.h"
#include "glsl_dataflow_builder.h"
#include "glsl_nast.h"
#include "glsl_symbols.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stdlib.auto.h"
#include "glsl_stackmem.h"

typedef struct {
   // The currently active basic block. Use basic_block_start and basic_block_end to set it.
   BasicBlock *current_basic_block;

   // The head of the list determines where return/break/continue statement jumps
   BasicBlockList *break_targets;
   BasicBlockList *continue_targets;
   BasicBlockList *return_targets;
   SymbolList *return_symbols; // temporary symbol to store function return value
} builder_context_t;

static void build_statement(builder_context_t *ctx, const NStmt *stmt);

// Start processing of given basic block.
// It should be new clean basic block.
static void basic_block_start(builder_context_t *ctx, BasicBlock *basic_block)
{
   assert(ctx->current_basic_block == NULL);
   assert(basic_block->branch_cond == NULL);
   assert(basic_block->branch_target == NULL);
   assert(basic_block->fallthrough_target == NULL);
   ctx->current_basic_block = basic_block;
}

// End processing of given basic block and set its successors.
// This is the only place where the successors are set.
// The basic block should not be further modified after this.
static void basic_block_end(builder_context_t *ctx, Dataflow *branch_cond, BasicBlock *branch_target, BasicBlock *fallthough_target)
{
   assert(ctx->current_basic_block->branch_cond == NULL);
   assert(ctx->current_basic_block->branch_target == NULL);
   assert(ctx->current_basic_block->fallthrough_target == NULL);
   ctx->current_basic_block->branch_cond = branch_cond;
   ctx->current_basic_block->branch_target = branch_target;
   ctx->current_basic_block->fallthrough_target = fallthough_target;
   ctx->current_basic_block = NULL;
}

static inline Dataflow **alloc_dataflow(size_t amount) {
   return glsl_stack_malloc(sizeof(Dataflow *), amount);
}

static inline void free_dataflow(Dataflow **dflow) {
   glsl_stack_free(dflow);
}

// Assign dataflow to lvalue if cond is true (the condition is used for dynamic array writes)
static void block_assignment(BasicBlock *block, Expr *lvalue, int offset, Dataflow **scalar_values, unsigned scalar_count, Dataflow *cond)
{
   switch (lvalue->flavour) {
   case EXPR_INSTANCE: {
      Symbol *symbol = lvalue->u.instance.symbol;

      assert(symbol->flavour == SYMBOL_VAR_INSTANCE   ||
             symbol->flavour == SYMBOL_PARAM_INSTANCE ||
             symbol->flavour == SYMBOL_TEMPORARY );

      if (offset == 0 && scalar_count == symbol->type->scalar_count && cond == NULL) {
         glsl_basic_block_set_scalar_values(block, symbol, scalar_values);
      } else {
         for (unsigned int i = 0; i < scalar_count; i++) {
            Dataflow *scalar_value = scalar_values[i];
            if (cond) {
               Dataflow *old_value = glsl_basic_block_get_scalar_value(block, symbol, offset + i);
               scalar_value = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond, scalar_value, old_value);
            }
            glsl_basic_block_set_scalar_value(block, symbol, offset + i, scalar_value);
         }
      }
      break;
   }
   case EXPR_SUBSCRIPT: {
      Dataflow *subscript;
      assert(lvalue->u.subscript.subscript->type->scalar_count == 1);
      glsl_expr_calculate_dataflow(block, &subscript, lvalue->u.subscript.subscript);
      assert(glsl_dataflow_is_integral_type(subscript));

      Expr *aggregate = lvalue->u.subscript.aggregate;
      unsigned int member_scalar_count;
      assert(aggregate->type->flavour == SYMBOL_PRIMITIVE_TYPE || aggregate->type->flavour == SYMBOL_ARRAY_TYPE);
      if (aggregate->type->flavour == SYMBOL_PRIMITIVE_TYPE)
         member_scalar_count = primitiveTypeSubscriptTypes[aggregate->type->u.primitive_type.index]->scalar_count;
      else
         member_scalar_count = aggregate->type->u.array_type.member_type->scalar_count;

      if (subscript->flavour == DATAFLOW_CONST) {
         unsigned int element_offset = subscript->u.constant.value * member_scalar_count;
         if (element_offset < aggregate->type->scalar_count)
            block_assignment(block, aggregate, offset + element_offset, scalar_values, scalar_count, cond);
      } else {
         unsigned int element_offset;
         unsigned int element_index;
         for (element_offset = 0, element_index = 0; element_offset < aggregate->type->scalar_count; element_offset += member_scalar_count, element_index++) {
            Dataflow *element_cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, subscript, glsl_dataflow_construct_const_value(subscript->type, element_index));
            if (cond)
               element_cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, element_cond);
            block_assignment(block, aggregate, offset + element_offset, scalar_values, scalar_count, element_cond);
         }
      }
      break;
   }
   case EXPR_FIELD_SELECTOR: {
      Expr *aggregate = lvalue->u.field_selector.aggregate;
      int field_offset = 0;

      assert(aggregate->type->flavour == SYMBOL_STRUCT_TYPE);
      for (int i = 0; i < lvalue->u.field_selector.field_no; i++)
         field_offset += aggregate->type->u.struct_type.member[i].type->scalar_count;

      block_assignment(block, aggregate, offset + field_offset, scalar_values, scalar_count, cond);
      break;
   }
   case EXPR_SWIZZLE: {
      Expr *aggregate = lvalue->u.swizzle.aggregate;
      assert(aggregate->type->scalar_count <= 4);
      assert(offset + scalar_count <= 4);
      for (unsigned int i = 0; i < scalar_count; i++) {
         unsigned char swizzle_slot = lvalue->u.swizzle.swizzle_slots[offset + i];
         assert(swizzle_slot != SWIZZLE_SLOT_UNUSED);
         block_assignment(block, aggregate, swizzle_slot, &scalar_values[i], 1, cond);
      }
      break;
   }

   default:
      unreachable();
      break;
   }
}

static void assign(BasicBlock *block, Expr *lvalue, Dataflow **scalar_values)
{
   if (in_addressable_memory(lvalue))
      buffer_store_expr_calculate_dataflow(block, lvalue, scalar_values);
   else
      block_assignment(block, lvalue, 0, scalar_values, lvalue->type->scalar_count, NULL);
}

static void build_statement_list(builder_context_t *ctx, NStmtList *stmts)
{
   if (stmts) {
      for (NStmtListNode *node = stmts->head; node; node = node->next)
         build_statement(ctx, node->v);
   }
}

static void build_var_decl_statement(builder_context_t *ctx, const NStmt *stmt)
{
   Symbol *var = stmt->u.var_decl.var;
   // If there's an initializer, and the variable is non-const,
   // save the dataflow graphs (for each scalar value in the initializer) in the variable.
   assert(!stmt->u.var_decl.initializer || var->flavour == SYMBOL_VAR_INSTANCE);
   if (stmt->u.var_decl.initializer && var->u.var_instance.storage_qual != STORAGE_CONST) {
      Dataflow **scalar_values = alloc_dataflow(var->type->scalar_count);
      glsl_expr_calculate_dataflow(ctx->current_basic_block, scalar_values, stmt->u.var_decl.initializer);
      glsl_basic_block_set_scalar_values(ctx->current_basic_block, var, scalar_values);
      free_dataflow(scalar_values);
   }
}

static void build_assign_statement(builder_context_t *ctx, const NStmt *stmt)
{
   unsigned int scalar_count = stmt->u.assign.rvalue->type->scalar_count;
   Dataflow **scalar_values = alloc_dataflow(scalar_count);
   glsl_expr_calculate_dataflow(ctx->current_basic_block, scalar_values, stmt->u.assign.rvalue);
   // lvalue is evaluated after rvalue (parts of it potentially multiple times)
   // but that is fine, because the lvalue should be side-effect free by now.
   assign(ctx->current_basic_block, stmt->u.assign.lvalue, scalar_values);
   free_dataflow(scalar_values);
}

static void build_selection_statement(builder_context_t *ctx, const NStmt *stmt)
{
   bool has_false = (stmt->u.selection.if_false && stmt->u.selection.if_false->head);
   BasicBlock *true_block = glsl_basic_block_construct();
   BasicBlock *false_block = has_false ? glsl_basic_block_construct() : NULL;
   BasicBlock *end_block = glsl_basic_block_construct();

   Dataflow *cond;
   glsl_expr_calculate_dataflow(ctx->current_basic_block, &cond, stmt->u.selection.cond);
   basic_block_end(ctx, cond, true_block, has_false ? false_block : end_block);

   basic_block_start(ctx, true_block);
   build_statement_list(ctx, stmt->u.selection.if_true);
   basic_block_end(ctx, NULL, NULL, end_block);

   if (has_false) {
      basic_block_start(ctx, false_block);
      build_statement_list(ctx, stmt->u.selection.if_false);
      basic_block_end(ctx, NULL, NULL, end_block);
   }

   basic_block_start(ctx, end_block);
}

static void build_switch_statement(builder_context_t *ctx, const NStmt *stmt)
{
   Symbol *switch_value_symbol = glsl_symbol_construct_temporary(stmt->u.switch_stmt.cond->type);
   BasicBlock *default_block = NULL;
   BasicBlock *end_block = glsl_basic_block_construct();

   {
      Dataflow *switch_value;
      glsl_expr_calculate_dataflow(ctx->current_basic_block, &switch_value, stmt->u.switch_stmt.cond);
      glsl_basic_block_set_scalar_value(ctx->current_basic_block, switch_value_symbol, 0, switch_value);
   }

   // If the switch statement is empty then we're done
   if (stmt->u.switch_stmt.statements->head == NULL) return;

   glsl_basic_block_list_add(&ctx->break_targets, end_block);

   Map *case_to_basic_block = glsl_map_new();

   // Create list of conditions which find the right case to branch to
   // The basic blocks for the cases are created here, but they are not populated yet
   for (NStmtListNode *node = stmt->u.switch_stmt.statements->head; node; node = node->next) {
      if(node->v->flavour == NSTMT_CASE) {
         Dataflow *switch_value = glsl_basic_block_get_scalar_value(ctx->current_basic_block, switch_value_symbol, 0);
         Dataflow *scalar_value;
         BasicBlock *case_block = glsl_basic_block_construct();
         BasicBlock *next_block = glsl_basic_block_construct();
         glsl_map_put(case_to_basic_block, node, case_block);
         glsl_expr_calculate_dataflow(ctx->current_basic_block, &scalar_value, node->v->u.case_stmt.expr);
         scalar_value = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, switch_value, scalar_value);
         basic_block_end(ctx, scalar_value, case_block, next_block);
         basic_block_start(ctx, next_block);
      } else if (node->v->flavour == NSTMT_DEFAULT) {
         default_block = glsl_basic_block_construct();
         glsl_map_put(case_to_basic_block, node, default_block);
      }
   }
   // If none of the cases was taken, branch to the default case (if we have one)
   basic_block_end(ctx, NULL, NULL, default_block ? default_block : end_block);

   /* Fill the basic blocks for cases. */
   /* It is not valid to have code before the first case statement */
   NStmtListNode *node = stmt->u.switch_stmt.statements->head;
   assert(node->v->flavour == NSTMT_CASE || node->v->flavour == NSTMT_DEFAULT);
   BasicBlock *case_block = glsl_map_get(case_to_basic_block, node);
   basic_block_start(ctx, case_block);

   for (node = node->next; node; node = node->next) {
      if(node->v->flavour == NSTMT_CASE || node->v->flavour == NSTMT_DEFAULT) {
         BasicBlock *case_block = glsl_map_get(case_to_basic_block, node);
         /* Terminate the previous case with a fallthrough and start the new one */
         basic_block_end(ctx, NULL, NULL, case_block);
         basic_block_start(ctx, case_block);
      } else {
         build_statement(ctx, node->v);
      }
   }
   // The last case falls through to the end
   basic_block_end(ctx, NULL, NULL, end_block);

   glsl_map_delete(case_to_basic_block);

   glsl_basic_block_list_pop(&ctx->break_targets);
   basic_block_start(ctx, end_block);
}

static void build_iterator_statement(builder_context_t *ctx, const NStmt *stmt)
{
   bool has_pre_cond  = (stmt->u.iterator.pre_cond_stmts  || stmt->u.iterator.pre_cond_expr);
   bool has_post_cond = (stmt->u.iterator.post_cond_stmts || stmt->u.iterator.post_cond_expr);
   BasicBlock *pre_cond_block  = has_pre_cond  ? glsl_basic_block_construct() : NULL;
   BasicBlock *post_cond_block = has_post_cond ? glsl_basic_block_construct() : NULL;
   BasicBlock *body_block      = glsl_basic_block_construct();
   BasicBlock *iter_block      = glsl_basic_block_construct();
   BasicBlock *end_block       = glsl_basic_block_construct();

   BasicBlock *loop_head = has_pre_cond  ? pre_cond_block  : body_block;
   BasicBlock *loop_next = has_post_cond ? post_cond_block : iter_block;

   glsl_basic_block_list_add(&ctx->break_targets,    end_block);
   glsl_basic_block_list_add(&ctx->continue_targets, loop_next);

   basic_block_end(ctx, NULL, NULL, loop_head);

   // Loop pre-condition
   if (has_pre_cond) {
      basic_block_start(ctx, pre_cond_block);
      build_statement_list(ctx, stmt->u.iterator.pre_cond_stmts);
      Dataflow *cond_scalar;
      if (stmt->u.iterator.pre_cond_expr) {
         glsl_expr_calculate_dataflow(ctx->current_basic_block, &cond_scalar, stmt->u.iterator.pre_cond_expr);
      } else {
         cond_scalar = glsl_dataflow_construct_const_bool(true);
      }
      basic_block_end(ctx, cond_scalar, body_block, end_block);
   }

   // Loop body
   basic_block_start(ctx, body_block);
   build_statement_list(ctx, stmt->u.iterator.body);
   basic_block_end(ctx, NULL, NULL, loop_next);

   // Loop post-condition
   if (has_post_cond) {
      basic_block_start(ctx, post_cond_block);
      build_statement_list(ctx, stmt->u.iterator.post_cond_stmts);
      Dataflow *cond_scalar;
      if (stmt->u.iterator.post_cond_expr) {
         glsl_expr_calculate_dataflow(ctx->current_basic_block, &cond_scalar, stmt->u.iterator.post_cond_expr);
      } else {
         cond_scalar = glsl_dataflow_construct_const_bool(true);
      }
      basic_block_end(ctx, cond_scalar, iter_block, end_block);
   }

   // Loop increment
   basic_block_start(ctx, iter_block);
   build_statement_list(ctx, stmt->u.iterator.increment);
   basic_block_end(ctx, NULL, NULL, loop_head);

   glsl_basic_block_list_pop(&ctx->break_targets);
   glsl_basic_block_list_pop(&ctx->continue_targets);

   basic_block_start(ctx, end_block);
}

static void build_function_call_statement(builder_context_t *ctx, const NStmt *stmt)
{
   Expr *lvalue = stmt->u.function_call.lvalue;
   Symbol *function = stmt->u.function_call.function;
   const NStmt *function_def = function->u.function_instance.function_norm_def;
   Symbol *return_value = glsl_symbol_construct_temporary(lvalue->type);

   // Evaluate and assign function arguments
   glsl_expr_calculate_function_call_args(ctx->current_basic_block, stmt->u.function_call.function, stmt->u.function_call.args);

   // Evaluate function body
   {
      BasicBlock *start_block = ctx->current_basic_block;
      BasicBlock *end_block = glsl_basic_block_construct();
      glsl_basic_block_list_add(&ctx->return_targets, end_block);
      glsl_symbol_list_append(ctx->return_symbols, return_value);

      build_statement_list(ctx, function_def->u.function_def.body);

      glsl_basic_block_list_pop(&ctx->return_targets);
      glsl_symbol_list_pop(ctx->return_symbols);
      basic_block_end(ctx, NULL, NULL, end_block);
      basic_block_start(ctx, end_block);

      // do not create extra basic block if we do not have to
      // Note this is required for some usage of intrinsics in the standard
      // library to work!
      if (start_block->fallthrough_target == end_block && (!start_block->branch_cond ||
         (start_block->branch_cond->flavour == DATAFLOW_CONST && start_block->branch_cond->u.constant.value == 0)))
      {
         glsl_basic_block_elim_dead(start_block);
         assert(!start_block->branch_cond && !start_block->branch_target);
         assert(start_block->fallthrough_target == end_block);
         glsl_basic_block_delete(end_block);
         start_block->fallthrough_target = NULL;
         ctx->current_basic_block = start_block;
      }
   }

   // Marshall output arguments
   ExprChainNode *node = stmt->u.function_call.args->first;
   for (unsigned i = 0; i < function->type->u.function_type.param_count; i++, node = node->next) {
      Symbol *formal = function->type->u.function_type.params[i];
      Expr *actual = node->expr;
      if (formal->u.param_instance.param_qual == PARAM_QUAL_OUT || formal->u.param_instance.param_qual == PARAM_QUAL_INOUT) {
         Dataflow **scalar_values = glsl_basic_block_get_scalar_values(ctx->current_basic_block, formal);
         assign(ctx->current_basic_block, actual, scalar_values);
      }
   }

   // Marshall return value
   Dataflow **scalar_values = glsl_basic_block_get_scalar_values(ctx->current_basic_block, return_value);
   assign(ctx->current_basic_block, lvalue, scalar_values);
}

static void build_expr_statement(builder_context_t *ctx, const NStmt *stmt)
{
   Dataflow **scalar_values = alloc_dataflow(stmt->u.expr.expr->type->scalar_count);
   glsl_expr_calculate_dataflow(ctx->current_basic_block, scalar_values, stmt->u.expr.expr);
   free_dataflow(scalar_values);
}

static void build_return_expr_statement(builder_context_t *ctx, const NStmt *stmt)
{
   Dataflow **scalar_values = alloc_dataflow(stmt->u.return_expr.expr->type->scalar_count);
   glsl_expr_calculate_dataflow(ctx->current_basic_block, scalar_values, stmt->u.return_expr.expr);
   glsl_basic_block_set_scalar_values(ctx->current_basic_block, ctx->return_symbols->tail->s, scalar_values);
   free_dataflow(scalar_values);

   BasicBlock *dead = glsl_basic_block_construct();
   basic_block_end(ctx, glsl_dataflow_construct_const_bool(false), dead, ctx->return_targets->v);
   basic_block_start(ctx, dead);
}

static void build_statement(builder_context_t *ctx, const NStmt *stmt)
{
   switch (stmt->flavour) {
   case NSTMT_FUNCTION_DEF:
      /* We only care about the definition of 'main' everything else is done at call time */
      if (strcmp(stmt->u.function_def.header->name, "main") == 0)
         build_statement_list(ctx, stmt->u.function_def.body);
      break;

   case NSTMT_VAR_DECL:
      build_var_decl_statement(ctx, stmt);
      break;

   case NSTMT_ASSIGN:
      build_assign_statement(ctx, stmt);
      break;

   case NSTMT_FUNCTION_CALL:
      build_function_call_statement(ctx, stmt);
      break;

   case NSTMT_EXPR:
      build_expr_statement(ctx, stmt);
      break;

   case NSTMT_SELECTION:
      build_selection_statement(ctx, stmt);
      break;

   case NSTMT_SWITCH:
      build_switch_statement(ctx, stmt);
      break;

   case NSTMT_ITERATOR:
      build_iterator_statement(ctx, stmt);
      break;

   case NSTMT_CONTINUE: {
      BasicBlock *dead = glsl_basic_block_construct();
      basic_block_end(ctx, glsl_dataflow_construct_const_bool(false), dead, ctx->continue_targets->v);
      basic_block_start(ctx, dead);
      break;
   }

   case NSTMT_RETURN: {
      BasicBlock *dead = glsl_basic_block_construct();
      basic_block_end(ctx, glsl_dataflow_construct_const_bool(false), dead, ctx->return_targets->v);
      basic_block_start(ctx, dead);
      break;
   }

   case NSTMT_BREAK: {
      BasicBlock *dead = glsl_basic_block_construct();
      basic_block_end(ctx, glsl_dataflow_construct_const_bool(false), dead, ctx->break_targets->v);
      basic_block_start(ctx, dead);
      break;
   }

   case NSTMT_DISCARD: {
      const Symbol *discard = glsl_stdlib_get_variable(GLSL_STDLIB_VAR__OUT__BOOL____DISCARD);
      glsl_basic_block_set_scalar_value(ctx->current_basic_block, discard, 0, glsl_dataflow_construct_const_bool(true));
      break;
   }
   case NSTMT_RETURN_EXPR:
      build_return_expr_statement(ctx, stmt);
      break;

   case NSTMT_BARRIER:
      ctx->current_basic_block->barrier = true;
      BasicBlock *next = glsl_basic_block_construct();
      basic_block_end(ctx, NULL, NULL, next);
      basic_block_start(ctx, next);
      break;

   case NSTMT_CASE:           /* Should be handled in the switch, and improper */
   case NSTMT_DEFAULT:        /* nesting has been rejected by AST validation   */
   case NSTMT_FLAVOUR_COUNT:
      unreachable();
      break;
   }
}

BasicBlock *glsl_basic_block_build(NStmtList *nast)
{
   builder_context_t ctx = { .return_symbols = glsl_symbol_list_new(),  };
   BasicBlock *entry_block = glsl_basic_block_construct();
   BasicBlock *end_block   = glsl_basic_block_construct();

   glsl_dataflow_reset_age();
   glsl_basic_block_list_add(&ctx.return_targets, end_block);

   basic_block_start(&ctx, entry_block);
   build_statement_list(&ctx, nast);
   basic_block_end(&ctx, NULL, NULL, end_block);

   glsl_basic_block_list_pop(&ctx.return_targets);
   assert(ctx.return_targets == NULL && ctx.break_targets == NULL && ctx.continue_targets == NULL);

   int df_stack_leaks = glsl_stack_cleanup();
   assert(df_stack_leaks == 0);

   return entry_block;
}
