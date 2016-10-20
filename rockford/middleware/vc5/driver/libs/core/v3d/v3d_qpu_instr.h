/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "libs/util/assert_helpers.h"
#include "v3d_addr.h"
#include "v3d_gen.h"
#include <stdbool.h>
#include <stdint.h>

VCOS_EXTERN_C_BEGIN

/** ALU instructions */

typedef enum
{
   V3D_QPU_IN_SOURCE_CLASS_REGULAR_ACC,
   V3D_QPU_IN_SOURCE_CLASS_R5,
   V3D_QPU_IN_SOURCE_CLASS_REGFILE,
   V3D_QPU_IN_SOURCE_CLASS_INVALID
} v3d_qpu_in_source_class_t;

static inline v3d_qpu_in_source_class_t v3d_qpu_classify_in_source(v3d_qpu_in_source_t source)
{
   switch (source)
   {
   case V3D_QPU_IN_SOURCE_R0:
   case V3D_QPU_IN_SOURCE_R1:
   case V3D_QPU_IN_SOURCE_R2:
   case V3D_QPU_IN_SOURCE_R3:
   case V3D_QPU_IN_SOURCE_R4:
      return V3D_QPU_IN_SOURCE_CLASS_REGULAR_ACC;
   case V3D_QPU_IN_SOURCE_R5:
      return V3D_QPU_IN_SOURCE_CLASS_R5;
   case V3D_QPU_IN_SOURCE_A:
   case V3D_QPU_IN_SOURCE_B:
      return V3D_QPU_IN_SOURCE_CLASS_REGFILE;
   default:
      unreachable();
      return V3D_QPU_IN_SOURCE_CLASS_INVALID;
   }
}

struct v3d_qpu_input
{
   v3d_qpu_in_source_t source;
   v3d_qpu_unpack_t unpack;
};

extern uint32_t v3d_qpu_op_num_inputs(v3d_qpu_opcode_t opcode);

/* 2-input ops only! */
extern bool v3d_qpu_op_is_commutative(v3d_qpu_opcode_t opcode);
extern bool v3d_qpu_op_is_mov_when_inputs_match(v3d_qpu_opcode_t opcode);

struct v3d_qpu_op
{
   struct v3d_qpu_input a, b;
   v3d_qpu_opcode_t opcode;
   v3d_qpu_pack_t pack;
};

struct v3d_qpu_sig
{
   bool small_imm;
   bool rotate;

   bool thrsw;
   bool ucb;

   bool ldvary;
   bool ldtlb;
   bool ldtlbu;
#if !V3D_HAS_LDVPM
   bool ldvpm;
#endif
   bool ldtmu;
#if V3D_HAS_NEW_TMU_CFG
   bool wrtmuc;
#endif
   bool ldunif;

#if V3D_HAS_SIG_TO_REG
   bool sig_reg;
   uint32_t waddr;
#endif
};

struct v3d_qpu_instr_alu
{
   uint32_t raddr_a;
   uint32_t raddr_b;

   struct v3d_qpu_op add, mul;

   struct v3d_qpu_sig sig;
};

extern bool v3d_qpu_alu_uses_source(struct v3d_qpu_instr_alu const* in, v3d_qpu_in_source_t src);

extern const uint32_t v3d_qpu_small_imms[48];

/** Other instruction types */

struct v3d_qpu_instr_branch
{
   uint32_t raddr_a;

   v3d_qpu_bcond_t cond;
   v3d_qpu_msfign_t msfign;

   v3d_qpu_bdest_t bdi;
   v3d_addr_t i_addr;

   bool ub;
   v3d_qpu_bdest_t bdu;

   bool ulr;
};

struct v3d_qpu_instr_ldi
{
   v3d_qpu_ldi_mode_t mode;
   uint32_t imm;
};

struct v3d_qpu_instr_bkpt
{
   bool halt_this;
   bool halt_other;
   bool raise_regular_int;
   bool raise_debug_int;
};

struct v3d_qpu_instr_debug
{
   uint32_t addr;
};

struct v3d_qpu_branch_params
{
   v3d_qpu_bcond_t cond;
   v3d_qpu_msfign_t msfign;

   v3d_qpu_bdest_t bdi;
   v3d_addr_t i_addr;

   bool ub;
   v3d_qpu_bdest_t bdu;
   v3d_addr_t u_addr;

   bool ulr;
};

static inline bool v3d_qpu_branch_instr_reads_unif(const struct v3d_qpu_instr_branch *in)
{
   return in->ub && (in->bdu == V3D_QPU_BDEST_ABS || in->bdu == V3D_QPU_BDEST_REL);
}

static inline void v3d_qpu_branch_params_from_instr(struct v3d_qpu_branch_params *p,
   const struct v3d_qpu_instr_branch *in, uint32_t unif)
{
   p->cond = in->cond;
   p->msfign = in->msfign;

   p->bdi = in->bdi;
   p->i_addr = in->i_addr;

   p->ub = in->ub;
   p->bdu = in->bdu;
   p->u_addr = unif & ~3; /* Bottom 2 bits ignored, as per the spec... */

   p->ulr = in->ulr;
}

static inline v3d_qpu_bdest_t v3d_qpu_ucb_dest(int32_t rel_addr)
{
   if (rel_addr == -1024)
      return V3D_QPU_BDEST_LINK_REG;
   else if (rel_addr == -1023)
      return V3D_QPU_BDEST_REGFILE;
   else
      return V3D_QPU_BDEST_REL;
}

static inline void v3d_qpu_branch_params_from_ucb(
   struct v3d_qpu_branch_params *p, uint32_t unif)
{
   V3D_UNIF_BRANCH_T ucb;
   v3d_unpack_unif_branch(&ucb, unif);

   p->cond = ucb.bcond;
   p->msfign = ucb.msfign;

   p->bdi = v3d_qpu_ucb_dest(ucb.rel_i_addr >> 3);
   p->i_addr = ucb.rel_i_addr;

   p->ub = true;
   p->bdu = v3d_qpu_ucb_dest(ucb.rel_u_addr >> 2);
   p->u_addr = ucb.rel_u_addr;

   p->ulr = ucb.ulr;
}

/** Common fields in instructions that do writes */

static inline bool v3d_qpu_magic_waddr_is_acc(uint32_t addr)
{
   return (addr >= V3D_QPU_MAGIC_WADDR_R0) && (addr <= V3D_QPU_MAGIC_WADDR_R5QUAD);
}

struct v3d_qpu_instr_write
{
   uint32_t addr;
   bool magic;

   v3d_qpu_setf_t setf;
   v3d_qpu_cond_t cond;
};

/** Putting it all together */

typedef enum
{
   V3D_QPU_INSTR_TYPE_ALU,
   V3D_QPU_INSTR_TYPE_BRANCH,
   V3D_QPU_INSTR_TYPE_LDI,
   V3D_QPU_INSTR_TYPE_BKPT,
   V3D_QPU_INSTR_TYPE_DEBUG,
   V3D_QPU_INSTR_TYPE_INVALID
} v3d_qpu_instr_type_t;

struct v3d_qpu_instr
{
   v3d_qpu_instr_type_t type;
   union
   {
      struct v3d_qpu_instr_alu alu;
      struct v3d_qpu_instr_branch branch;
      struct v3d_qpu_instr_ldi ldi;
      struct v3d_qpu_instr_bkpt bkpt;
      struct v3d_qpu_instr_debug debug;
   } u;
   struct v3d_qpu_instr_write add_wr, mul_wr; /* Valid iff v3d_qpu_instr_does_write(type) */
};

extern v3d_qpu_instr_type_t v3d_qpu_instr_get_type(uint64_t bits);
#define V3D_QPU_INSTR_ERR_SIZE 128
/* On error, false is returned. If err is not NULL, an error message will be
 * written to it. */
extern bool v3d_qpu_instr_try_unpack(struct v3d_qpu_instr *in, uint64_t bits,
   char err[V3D_QPU_INSTR_ERR_SIZE]);
extern void v3d_qpu_instr_unpack(struct v3d_qpu_instr *in, uint64_t bits);

extern bool v3d_qpu_instr_try_pack(struct v3d_qpu_instr const* in, uint64_t* bits);

extern uint64_t v3d_qpu_instr_unused_mask(const struct v3d_qpu_instr *in);

static inline bool v3d_qpu_instr_does_write(v3d_qpu_instr_type_t type)
{
   return
      (type == V3D_QPU_INSTR_TYPE_ALU) ||
      (type == V3D_QPU_INSTR_TYPE_LDI) ||
      (type == V3D_QPU_INSTR_TYPE_DEBUG);
}

/** Result type */

typedef enum
{
   V3D_QPU_RES_TYPE_32I, /* 32-bit integer (red "anything else" in the arch spec) */
   V3D_QPU_RES_TYPE_32F, /* 32-bit float (green in the arch spec) */
   V3D_QPU_RES_TYPE_2X16I, /* 2x16-bit integer (purple in the arch spec) */
   V3D_QPU_RES_TYPE_2X16F, /* 2x16-bit float (blue in the arch spec) */
   V3D_QPU_RES_TYPE_NOP, /* nop (grey in spec) */

   /* Hardware writes directly to register file, no packing/masking.
    * Only valid for register file writes (no magic). */
   V3D_QPU_RES_TYPE_DIRECT_RF_WRITE,

   V3D_QPU_RES_TYPE_INVALID
} v3d_qpu_res_type_t;

static inline bool v3d_qpu_res_type_is_2x16(v3d_qpu_res_type_t res_type)
{
   return (res_type == V3D_QPU_RES_TYPE_2X16I) ||
          (res_type == V3D_QPU_RES_TYPE_2X16F);
}

extern v3d_qpu_res_type_t v3d_qpu_res_type_from_opcode(v3d_qpu_opcode_t opcode);
extern void v3d_qpu_res_types_from_instr(
   v3d_qpu_res_type_t *add_type, v3d_qpu_res_type_t *mul_type,
   const struct v3d_qpu_instr *in);

VCOS_EXTERN_C_END
