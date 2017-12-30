/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_backflow.h"
#include "glsl_backend_uniforms.h"

static inline Backflow *tr_nullary(BackflowFlavour flavour) {
   return create_node(flavour, UNPACK_NONE, SETF_NONE, NULL, NULL, NULL, NULL);
}

static inline Backflow *tr_uop(BackflowFlavour flavour, Backflow *operand) {
   return create_node(flavour, UNPACK_NONE, SETF_NONE, NULL, operand, NULL, NULL);
}

static inline Backflow *tr_binop(BackflowFlavour flavour, Backflow *left, Backflow *right) {
   return create_node(flavour, UNPACK_NONE, SETF_NONE, NULL, left, right, NULL);
}

static inline Backflow *tr_mov_to_reg(uint32_t reg, Backflow *operand) {
   Backflow *ret = tr_uop(BACKFLOW_MOV, operand);
   assert(ret->magic_write == REG_UNDECIDED);
   ret->magic_write = reg;
   return ret;
}

static inline Backflow *tr_uop_cond(BackflowFlavour f, uint32_t cond_setf, Backflow *flag, Backflow *operand) {
   return create_node(f, UNPACK_NONE, cond_setf, flag, operand, NULL, NULL);
}

static inline Backflow *tr_binop_push(BackflowFlavour f, uint32_t cond_setf, Backflow *l, Backflow *r) {
   return create_node(f, UNPACK_NONE, cond_setf, NULL, l, r, NULL);
}

static inline Backflow *tr_binop_cond(BackflowFlavour f, uint32_t cond_setf, Backflow *flags, Backflow *l, Backflow *r) {
   return create_node(f, UNPACK_NONE, cond_setf, flags, l, r, NULL);
}

static inline Backflow *tr_sig_io_offset(uint32_t sigbits, Backflow *dep, int io_offset) {
   Backflow *res = create_sig(sigbits);
   glsl_iodep_offset(res, dep, io_offset);
   return res;
}

static inline Backflow *tr_sig_io(uint32_t sigbits, Backflow *dep) {
   return tr_sig_io_offset(sigbits, dep, 0);
}

static inline Backflow *tr_uop_io(BackflowFlavour flavour, Backflow *param, Backflow *prev) {
   Backflow *result = tr_uop(flavour, param);
   glsl_iodep(result, prev);
   return result;
}

static inline Backflow *tr_binop_io(BackflowFlavour flavour, Backflow *l, Backflow *r, Backflow *prev) {
   Backflow *result = tr_binop(flavour, l, r);
   glsl_iodep(result, prev);
   return result;
}

static inline Backflow *tr_mov_to_reg_io(uint32_t reg, Backflow *param, Backflow *iodep) {
   Backflow *result = tr_mov_to_reg(reg, param);
   glsl_iodep(result, iodep);
   return result;
}

static inline Backflow *tr_typed_uniform(BackendUniformFlavour flavour, uint32_t value) {
   Backflow *result  = create_sig(V3D_QPU_SIG_LDUNIF);
   assert(flavour >= 0 && flavour <= BACKEND_UNIFORM_LAST_ELEMENT);
   result->unif_type = flavour;
   result->unif      = value;
   return result;
}

static inline Backflow *tr_special_uniform(BackendSpecialUniformFlavour flavour) {
   assert(flavour >= 0 || flavour <= BACKEND_SPECIAL_UNIFORM_LAST_ELEMENT);
   return tr_typed_uniform(BACKEND_UNIFORM_SPECIAL, flavour);
}

static inline Backflow *tr_unif(uint32_t i) {
   return tr_typed_uniform(BACKEND_UNIFORM_PLAIN, i);
}
static inline Backflow *tr_uniform_address(uint32_t row, uint32_t offset) {
   assert( (row >> 16) == 0 && (offset >> 16) == 0);
   return tr_typed_uniform(BACKEND_UNIFORM_ADDRESS, row | (offset << 16));
}
static inline Backflow *tr_buffer_unif(BackendUniformFlavour f, uint32_t index, uint32_t offset) {
   assert( (index >> 5) == 0 && (offset >> 27) == 0);
   return tr_typed_uniform(f, index | (offset << 5));
}

static inline Backflow *tr_const(uint32_t c) { return tr_typed_uniform(BACKEND_UNIFORM_LITERAL, c); }
static inline Backflow *tr_cfloat(float f)   { return tr_const(gfx_float_to_bits(f)); }

/* Define these static inline, so that unused functions for versioned
 * builds won't cause compiler warnings.                             */
static inline Backflow *mul(Backflow *a, Backflow *b)    { return tr_binop(BACKFLOW_MUL,  a, b); }
static inline Backflow *add(Backflow *a, Backflow *b)    { return tr_binop(BACKFLOW_ADD,  a, b); }
static inline Backflow *sub(Backflow *a, Backflow *b)    { return tr_binop(BACKFLOW_SUB,  a, b); }
static inline Backflow *bitand(Backflow *a, Backflow *b) { return tr_binop(BACKFLOW_AND,  a, b); }
static inline Backflow *bitor(Backflow *a, Backflow *b)  { return tr_binop(BACKFLOW_OR,   a, b); }
static inline Backflow *shl(Backflow *a, uint32_t b)     { return tr_binop(BACKFLOW_SHL,  a, tr_const(b)); }
static inline Backflow *shr(Backflow *a, uint32_t b)     { return tr_binop(BACKFLOW_SHR,  a, tr_const(b)); }
static inline Backflow *asr(Backflow *a, uint32_t b)     { return tr_binop(BACKFLOW_ASHR, a, tr_const(b)); }
static inline Backflow *imin(Backflow *a, Backflow *b)   { return tr_binop(BACKFLOW_MIN, a, b); }
static inline Backflow *imax(Backflow *a, Backflow *b)   { return tr_binop(BACKFLOW_MAX, a, b); }

static inline Backflow *recip(Backflow *x) { return tr_mov_to_reg(REG_MAGIC_RECIP, x); }
static inline Backflow *absf(Backflow *x)  { return create_node(BACKFLOW_FMOV, UNPACK_ABS, SETF_NONE, NULL, x, NULL, NULL); }
static inline Backflow *tr_sqrt(Backflow *operand) {
#if V3D_VER_AT_LEAST(3,3,0,0)
   return mul(operand, tr_mov_to_reg(REG_MAGIC_RSQRT2, operand));
#else
   return tr_mov_to_reg(REG_MAGIC_RECIP, tr_mov_to_reg(REG_MAGIC_RSQRT, operand));
#endif
}

/* Used by glsl_scheduler_4.c */
static inline Backflow *glsl_backflow_thrsw(void) { return tr_nullary(BACKFLOW_THREADSWITCH); }
static inline Backflow *glsl_backflow_tmuwt(void) { return tr_nullary(BACKFLOW_TMUWT); }
#if V3D_VER_AT_LEAST(4,0,2,0)
static inline Backflow *glsl_backflow_vpmwt(void) { return tr_nullary(BACKFLOW_VPMWT); }
#endif
static inline Backflow *glsl_backflow_dummy(void) { return tr_nullary(BACKFLOW_DUMMY); }


static inline bool is_unif(const Backflow *b) { return b->type == SIG && b->u.sigbits == V3D_QPU_SIG_LDUNIF; }

static inline bool is_plain_unif(const Backflow *b) { return is_unif(b) && b->unif_type == BACKEND_UNIFORM_PLAIN;       }
static inline bool is_const(const Backflow *b)      { return is_unif(b) && b->unif_type == BACKEND_UNIFORM_LITERAL;     }
static inline bool is_ubo_addr(const Backflow *b)   { return is_unif(b) && b->unif_type == BACKEND_UNIFORM_UBO_ADDRESS; }

static inline bool is_const_zero(const Backflow *b) { return (is_const(b) && b->unif == 0); }
