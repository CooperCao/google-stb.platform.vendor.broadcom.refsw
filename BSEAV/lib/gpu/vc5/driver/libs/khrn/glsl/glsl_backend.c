/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>
#include <setjmp.h>
#include <limits.h>

#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_binary_shader.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_qpu_instr.h"

const char *mov_excuse_strings[] = {
   "",
   "   # unfolded output       ",
   "   # mov present in IR     ",
   "   # move out of the way   ",
   "   # move flag B for a push",
   "   # move acc due to thrsw ",
   "   # copy on write         ",
   "   # can't write to r5     ",
   "   # flagify               ",
};

/*********************** HOW THIS STUFF ALL WORKS ****************************/
/* The register points to the node and the node to the register.             */
/* The last time at which a register was read is stored in reg_available.    */
/* The fit_* functions find a place for the instruction(s) and write into the*/
/*    instruction stream. They return the time of the instruction.           */
/* move_result moves something, leaving the state completely consistent so   */
/*    other stuff doesn't have to worry.                                     */

/* TODO: This is what we were implicitly doing all along, since backend errors
   are silently killed off, but that doesn't make it a great plan */
jmp_buf s_BackendErrorHandlerEnv;

static void backend_error(const char *description) {
   longjmp(s_BackendErrorHandlerEnv, 1);
}

static void translate_write(uint32_t *waddr_out, bool *is_magic, backend_reg reg)
{
   if (reg == REG_UNDECIDED) return;

   assert( IS_RRF(reg) || IS_MAGIC(reg) );

   *is_magic = !IS_RF(reg);

   if (IS_RF(reg))             *waddr_out = reg - REG_RF0;
   else if (IS_R(reg))         *waddr_out = reg - REG_R0_;
   else                        *waddr_out = reg - REG_MAGIC_BASE;
}

static v3d_qpu_in_source_t translate_mux(backend_reg reg, const backend_reg *reads) {
   if (reg == REG_UNDECIDED) return V3D_QPU_IN_SOURCE_R0;

   assert(IS_R(reg) || (IS_RF(reg) && (reg == reads[0] || reg == reads[1])));

   if (IS_R(reg))            return reg - REG_R0_;
   else if (reg == reads[0]) return V3D_QPU_IN_SOURCE_A;
   else                      return V3D_QPU_IN_SOURCE_B;
}

struct opcode_s {
   BackflowFlavour flavour;
   v3d_qpu_opcode_t opcode;
};

static const struct opcode_s opcodes[] = {
   { BACKFLOW_ADD,        V3D_QPU_OP_FADD    },
   { BACKFLOW_VFPACK,     V3D_QPU_OP_VFPACK  },
   { BACKFLOW_IADD,       V3D_QPU_OP_ADD     },
   { BACKFLOW_ISUB,       V3D_QPU_OP_SUB     },
   { BACKFLOW_SUB,        V3D_QPU_OP_FSUB    },
   { BACKFLOW_IMIN,       V3D_QPU_OP_MIN     },
   { BACKFLOW_IMAX,       V3D_QPU_OP_MAX     },
   { BACKFLOW_UMIN,       V3D_QPU_OP_UMIN    },
   { BACKFLOW_UMAX,       V3D_QPU_OP_UMAX    },
   { BACKFLOW_SHL,        V3D_QPU_OP_SHL     },
   { BACKFLOW_SHR,        V3D_QPU_OP_SHR     },
   { BACKFLOW_ASHR,       V3D_QPU_OP_ASR     },
   { BACKFLOW_ROR,        V3D_QPU_OP_ROR     },
   { BACKFLOW_MIN,        V3D_QPU_OP_FMIN    },
   { BACKFLOW_MAX,        V3D_QPU_OP_FMAX    },
   { BACKFLOW_AND,        V3D_QPU_OP_AND     },
   { BACKFLOW_OR,         V3D_QPU_OP_OR      },
   { BACKFLOW_XOR,        V3D_QPU_OP_XOR     },
   { BACKFLOW_NOT,        V3D_QPU_OP_NOT     },
   { BACKFLOW_INEG,       V3D_QPU_OP_NEG     },
   { BACKFLOW_SETMSF,     V3D_QPU_OP_SETMSF  },
   { BACKFLOW_SETREVF,    V3D_QPU_OP_SETREVF },
   { BACKFLOW_TIDX,       V3D_QPU_OP_TIDX    },
   { BACKFLOW_EIDX,       V3D_QPU_OP_EIDX    },
   { BACKFLOW_FL,         V3D_QPU_OP_INVALID },
   { BACKFLOW_FLN,        V3D_QPU_OP_INVALID },
   { BACKFLOW_FXCD,       V3D_QPU_OP_FXCD    },
   { BACKFLOW_XCD,        V3D_QPU_OP_XCD     },
   { BACKFLOW_FYCD,       V3D_QPU_OP_FYCD    },
   { BACKFLOW_YCD,        V3D_QPU_OP_YCD     },
   { BACKFLOW_MSF,        V3D_QPU_OP_MSF     },
   { BACKFLOW_REVF,       V3D_QPU_OP_REVF    },
#if V3D_VER_AT_LEAST(4,0,2,0)
   { BACKFLOW_IID,        V3D_QPU_OP_IID     },
   { BACKFLOW_SAMPID,     V3D_QPU_OP_SAMPID  },
   { BACKFLOW_PATCHID,    V3D_QPU_OP_PATCHID },
#endif
   { BACKFLOW_TMUWT,      V3D_QPU_OP_TMUWT   },
#if V3D_VER_AT_LEAST(4,0,2,0)
   { BACKFLOW_VPMWT,      V3D_QPU_OP_VPMWT   },
   { BACKFLOW_LDVPMV_IN,  V3D_QPU_OP_LDVPMV_IN  },
   { BACKFLOW_LDVPMV_OUT, V3D_QPU_OP_LDVPMV_OUT },
   { BACKFLOW_LDVPMD_IN,  V3D_QPU_OP_LDVPMD_IN  },
   { BACKFLOW_LDVPMD_OUT, V3D_QPU_OP_LDVPMD_OUT },
   { BACKFLOW_LDVPMP,     V3D_QPU_OP_LDVPMP     },
   { BACKFLOW_LDVPMG_IN,  V3D_QPU_OP_LDVPMG_IN  },
   { BACKFLOW_LDVPMG_OUT, V3D_QPU_OP_LDVPMG_OUT },
   { BACKFLOW_STVPMV,     V3D_QPU_OP_STVPMV     },
   { BACKFLOW_STVPMD,     V3D_QPU_OP_STVPMD     },
   { BACKFLOW_STVPMP,     V3D_QPU_OP_STVPMP     },
#else
   { BACKFLOW_VPMSETUP,   V3D_QPU_OP_VPMSETUP },
#endif
   { BACKFLOW_NEG,        V3D_QPU_OP_VFNEGMOV },
   { BACKFLOW_FCMP,       V3D_QPU_OP_FCMP     },
   { BACKFLOW_ROUND,      V3D_QPU_OP_FROUND   },
   { BACKFLOW_FTOIN,      V3D_QPU_OP_FTOIN    },
   { BACKFLOW_TRUNC,      V3D_QPU_OP_FTRUNC   },
   { BACKFLOW_FTOIZ,      V3D_QPU_OP_FTOIZ    },
   { BACKFLOW_FLOOR,      V3D_QPU_OP_FFLOOR   },
   { BACKFLOW_FTOUZ,      V3D_QPU_OP_FTOUZ    },
   { BACKFLOW_CEIL,       V3D_QPU_OP_FCEIL    },
   { BACKFLOW_FTOC,       V3D_QPU_OP_FTOC     },
   { BACKFLOW_FDX,        V3D_QPU_OP_FDX      },
   { BACKFLOW_FDY,        V3D_QPU_OP_FDY      },
   { BACKFLOW_ITOF,       V3D_QPU_OP_ITOF     },
   { BACKFLOW_CLZ,        V3D_QPU_OP_CLZ      },
   { BACKFLOW_UTOF,       V3D_QPU_OP_UTOF     },

   { BACKFLOW_IADD_M,     V3D_QPU_OP_ADD    },
   { BACKFLOW_ISUB_M,     V3D_QPU_OP_SUB    },
   { BACKFLOW_UMUL,       V3D_QPU_OP_UMUL24 },
   { BACKFLOW_SMUL,       V3D_QPU_OP_SMUL24 },
   { BACKFLOW_MULTOP,     V3D_QPU_OP_MULTOP },
   { BACKFLOW_FMOV,       V3D_QPU_OP_FMOV   },
   { BACKFLOW_MOV,        V3D_QPU_OP_MOV    },
   { BACKFLOW_MUL,        V3D_QPU_OP_FMUL   },

};

static inline v3d_qpu_opcode_t get_opcode(BackflowFlavour f) {
   assert(opcodes[f].flavour == f);
   return opcodes[f].opcode;
}

static v3d_qpu_setf_t convert_setf(uint32_t cond_setf) {
   return cs_is_push(cond_setf) || cs_is_updt(cond_setf) ? cond_setf : V3D_QPU_SETF_NONE;
}

static v3d_qpu_cond_t convert_cond(uint32_t cond_setf) {
   switch(cond_setf) {
      case COND_IFA:  return V3D_QPU_COND_IFA;
      case COND_IFB:  return V3D_QPU_COND_IFB;
      case COND_IFNA: return V3D_QPU_COND_IFNA;
      case COND_IFNB: return V3D_QPU_COND_IFNB;
      default:        return V3D_QPU_COND_ALWAYS;
   }
}

static void gen_wr(struct v3d_qpu_instr_write *wr, const GLSL_OP_T *op) {
   const static struct v3d_qpu_instr_write no_wr = { .addr = V3D_QPU_MAGIC_WADDR_NOP, .magic = true,
                                                     .setf = V3D_QPU_SETF_NONE,
                                                     .cond = V3D_QPU_COND_ALWAYS };
   if (!op->used) *wr = no_wr;
   else {
      translate_write(&wr->addr, &wr->magic, op->output);
      wr->setf = convert_setf(op->cond_setf);
      wr->cond = convert_cond(op->cond_setf);
   }
}

static inline void add_read(backend_reg *read, backend_reg reg) {
   assert(IS_RF(reg));
   assert(read[1] == REG_UNDECIDED || read[0] == reg || read[1] == reg);
   /* No need to record this, it's already here */
   if (read[0] == reg || read[1] == reg) return;

   if (read[0] == REG_UNDECIDED) read[0] = reg;
   else                          read[1] = reg;
}

static void instr_get_reads(const INSTR_T *ci, backend_reg *read) {
   read[0] = read[1] = REG_UNDECIDED;
   if (ci->a.used && IS_RF(ci->a.input_a)) add_read(read, ci->a.input_a);
   if (ci->a.used && IS_RF(ci->a.input_b)) add_read(read, ci->a.input_b);
   if (ci->m.used && IS_RF(ci->m.input_a)) add_read(read, ci->m.input_a);
   if (ci->m.used && IS_RF(ci->m.input_b)) add_read(read, ci->m.input_b);
}

static v3d_qpu_unpack_t get_v3d_unpack(SchedNodeUnpack u) {
   switch (u) {
      case UNPACK_ABS:        return V3D_QPU_UNPACK_32F_ABS;
      case UNPACK_NONE:       return V3D_QPU_UNPACK_NONE;
      case UNPACK_F16_A:      return V3D_QPU_UNPACK_16F_LOW_TO_32F;
      case UNPACK_F16_B:      return V3D_QPU_UNPACK_16F_HIGH_TO_32F;
      default: unreachable(); return V3D_QPU_UNPACK_INVALID;
   }
}

static void gen_op(struct v3d_qpu_op *out, const GLSL_OP_T *op_struct, const backend_reg *instr_reads) {
   const static struct v3d_qpu_op op_nop =
         { .a = { .unpack = V3D_QPU_UNPACK_NONE }, .b = { .unpack = V3D_QPU_UNPACK_NONE },
           .opcode = V3D_QPU_OP_NOP, .pack = V3D_QPU_PACK_NONE };
   const static v3d_qpu_opcode_t flag_op[2][2] = { { V3D_QPU_OP_VFLA, V3D_QPU_OP_VFLB },
                                                   { V3D_QPU_OP_VFLNA, V3D_QPU_OP_VFLNB } };

   if (!op_struct->used) *out = op_nop;
   else if (op_struct->op == BACKFLOW_FL || op_struct->op == BACKFLOW_FLN) {
      assert(op_struct->input_a == REG_FLAG_A || op_struct->input_a == REG_FLAG_B);
      out->opcode = flag_op[ (op_struct->op == BACKFLOW_FLN) ][ (op_struct->input_a == REG_FLAG_B) ];
      out->a.unpack = V3D_QPU_UNPACK_NONE;
      out->b.unpack = V3D_QPU_UNPACK_NONE;
      out->pack = V3D_QPU_PACK_NONE;
   } else {
      out->a.source = translate_mux(op_struct->input_a, instr_reads);
      out->b.source = translate_mux(op_struct->input_b, instr_reads);
      out->a.unpack = get_v3d_unpack(op_struct->unpack[0]);
      out->b.unpack = get_v3d_unpack(op_struct->unpack[1]);
      out->opcode = get_opcode(op_struct->op);
      out->pack = V3D_QPU_PACK_NONE;
   }
}

static void gen_sig(struct v3d_qpu_sig *sig, v3d_qpu_sigbits_t sigbits, backend_reg sig_waddr) {
   sig->sigbits = sigbits;

#if V3D_VER_AT_LEAST(4,0,2,0)
   assert(sig_waddr == REG_UNDECIDED || IS_RF(sig_waddr));
   sig->sig_reg = (sig_waddr != REG_UNDECIDED);
   sig->waddr   = sig_waddr - REG_RF0;
#endif
}

static uint64_t pack_instr(const struct v3d_qpu_instr *in) {
   uint64_t ret;
   bool ok = v3d_qpu_instr_try_pack(in, &ret);
   assert(ok);
   return ret;
}

static uint64_t generate(const INSTR_T *ci) {
   struct v3d_qpu_instr in = { .type = V3D_QPU_INSTR_TYPE_ALU };

   gen_wr(&in.add_wr, &ci->a);
   gen_wr(&in.mul_wr, &ci->m);

   backend_reg read[2];
   instr_get_reads(ci, read);

   uint32_t raddr[2] = {0, 0};
   for (int i=0; i<2; i++) {
      if (read[i] != REG_UNDECIDED) {
         assert(IS_RF(read[i]));
         raddr[i] = read[i] - REG_RF0;
      }
   }
   in.u.alu.raddr_a = raddr[0];
   in.u.alu.raddr_b = raddr[1];

   gen_op(&in.u.alu.add, &ci->a, read);
   gen_op(&in.u.alu.mul, &ci->m, read);

   gen_sig(&in.u.alu.sig, ci->sigbits, ci->sig_waddr);

   return pack_instr(&in);
}

void generate_all_instructions(const INSTR_T *instructions,
                               uint32_t instr_count,
                               GENERATED_SHADER_T *gen)
{
   uint32_t unif_count = 0;
   uint32_t varying_count = 0;

   for (unsigned i = 0; i < instr_count; i++) {
      const INSTR_T *ci = &instructions[i];

      gen->instructions[i] = generate(ci);

      /* uniform */
      if (ci->unif != UNIF_NONE) {
         if (unif_count >= MAX_UNIFORMS) backend_error("Too many uniforms");

         assert((ci->conflicts & CONFLICT_UNIF) != 0);
         gen->unifs[2*unif_count]   = (uint32_t)ci->unif;
         gen->unifs[2*unif_count+1] = (uint32_t)(ci->unif >> 32);
         unif_count++;
      }

      /* varying */
      if (ci->sigbits & V3D_QPU_SIG_LDVARY && ci->varying < VARYING_ID_HW_0) {
         assert(varying_count < MAX_VARYINGS);
         gen->varyings[varying_count] = ci->varying;
         varying_count++;
      }
   }

   gen->instruction_count = instr_count;
   gen->unif_count        = unif_count;
   gen->varying_count     = varying_count;
}

static bool conflicts_ok(uint32_t conflicts_a, uint32_t conflicts_b) {
   static const uint32_t peripherals = CONFLICT_TMU_W | CONFLICT_TMU_C | CONFLICT_TMU_R |
                                       CONFLICT_TLB_R | CONFLICT_TLB_W | CONFLICT_VPM | CONFLICT_SFU;
   static const uint32_t tlb_acc     = CONFLICT_TLB_R | CONFLICT_TLB_W | CONFLICT_POST_THRSW;
#if V3D_VER_AT_LEAST(3,4,0,0)
   static const uint32_t cond_reg_conflict = CONFLICT_TLB_R   | CONFLICT_TMU_R   | CONFLICT_VARY |
                                             CONFLICT_ADDFLAG | CONFLICT_MULFLAG | CONFLICT_SETFLAG;
#else
   static const uint32_t cond_reg_conflict = 0;
#endif

   uint32_t bad_accesses = conflicts_a;
   if (conflicts_a & tlb_acc)           bad_accesses |= tlb_acc;
   if (conflicts_a & cond_reg_conflict) bad_accesses |= cond_reg_conflict;
   if (conflicts_a & peripherals) {
      uint32_t p = peripherals;
      if (conflicts_a & CONFLICT_TMU_W) p &= ~CONFLICT_TMU_C;
      if (conflicts_a & CONFLICT_TMU_C) p &= ~CONFLICT_TMU_W;
#if V3D_HAS_TMU_VPM_SYNC
      if (conflicts_a & CONFLICT_VPM)   p &= ~CONFLICT_TMU_R;
      if (conflicts_a & CONFLICT_TMU_R) p &= ~CONFLICT_VPM;
#endif
      bad_accesses |= p;
   }

   return (conflicts_b & bad_accesses) == 0;
}

static bool can_add_sigbits(uint32_t a, uint32_t b) {
   uint32_t ignored;
   return (a & b) == 0 && v3d_qpu_try_encode_sigbits((a | b), &ignored);
}

static uint32_t sig_conflicts(uint32_t sigbits) {
   switch (sigbits)
   {
#if V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_QPU_SIG_WRTMUC: return CONFLICT_TMU_C | CONFLICT_UNIF;
#else
   case V3D_QPU_SIG_LDVPM:  return CONFLICT_VPM;
#endif
   case V3D_QPU_SIG_LDUNIF: return CONFLICT_UNIF;
   case V3D_QPU_SIG_LDTLBU: return CONFLICT_TLB_R | CONFLICT_UNIF;
   case V3D_QPU_SIG_LDTLB:  return CONFLICT_TLB_R;
   case V3D_QPU_SIG_LDTMU:  return CONFLICT_TMU_R;
   case V3D_QPU_SIG_LDVARY: return CONFLICT_VARY;
   default: return 0;
   }
}

static bool sig_uses_waddr(uint32_t sigbit) {
   switch (sigbit) {
#if V3D_VER_AT_LEAST(4,0,2,0)
      case V3D_QPU_SIG_LDTMU:
      case V3D_QPU_SIG_LDTLB:
      case V3D_QPU_SIG_LDTLBU: return true;
#endif
      default: return false;
   }
}

static uint32_t fit_sig(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t sigbits, uint64_t unif, backend_reg sig_waddr)
{
   uint32_t conflicts = sig_conflicts(sigbits);

   /* Advance current instruction until we find one which doesn't conflict */
   INSTR_T *ci;
   while (true) {
      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      ci = &sched->instructions[time];
      if (can_add_sigbits(ci->sigbits, sigbits) && conflicts_ok(ci->conflicts, conflicts))
         break;

      time++;
   }

   /* Fill in instruction */
   assert( (conflicts & CONFLICT_UNIF) || unif == UNIF_NONE);

   ci->sigbits   |= sigbits;
   ci->conflicts |= conflicts;
   if (unif != UNIF_NONE) ci->unif = unif;
   if (sig_uses_waddr(sigbits)) {
      assert(ci->sig_waddr == REG_UNDECIDED);
      ci->sig_waddr = sig_waddr;
   }

   return time;
}

static uint32_t get_output_conflicts(uint32_t output)
{
   if (IS_MAGIC(output))
   {
      switch (output)
      {
      case REG_MAGIC_TLBU:
      case REG_MAGIC_TLB:
         return CONFLICT_TLB_W;
      case REG_MAGIC_TMUA:
      case REG_MAGIC_TMUAU:
      case REG_MAGIC_TMUD:
         return CONFLICT_TMU_W;
#if V3D_VER_AT_LEAST(4,0,2,0)
      case REG_MAGIC_TMUC:
         return CONFLICT_TMU_W | CONFLICT_TMU_C;
      case REG_MAGIC_TMUS:
      case REG_MAGIC_TMUT:
      case REG_MAGIC_TMUR:
      case REG_MAGIC_TMUI:
      case REG_MAGIC_TMUB:
      case REG_MAGIC_TMUDREF:
      case REG_MAGIC_TMUOFF:
      case REG_MAGIC_TMUSCM:
      case REG_MAGIC_TMUSF:
      case REG_MAGIC_TMUSLOD:
         return CONFLICT_TMU_W;
#else
      case REG_MAGIC_VPM:
      case REG_MAGIC_VPMU:
         return CONFLICT_VPM;

      case REG_MAGIC_TMU:
      case REG_MAGIC_TMUL:
         return CONFLICT_TMU_W;
#endif
      case REG_MAGIC_RECIP:
      case REG_MAGIC_RSQRT:
      case REG_MAGIC_RSQRT2:
      case REG_MAGIC_EXP:
      case REG_MAGIC_LOG:
      case REG_MAGIC_SIN:
         return CONFLICT_SFU;
      }
   }

   return 0;
}

static inline bool IS_SFU(uint32_t output) {
   return output == REG_MAGIC_RECIP  ||
          output == REG_MAGIC_RSQRT  ||
          output == REG_MAGIC_RSQRT2 ||
          output == REG_MAGIC_EXP    ||
          output == REG_MAGIC_LOG    ||
          output == REG_MAGIC_SIN;
}

static uint32_t get_output_delay(uint32_t output) {
   if (IS_SFU(output)) return 2;
   else return 1;
}

static uint32_t get_cond_setf_conflicts(uint32_t cond_setf, SchedNodeType alu_type)
{
   uint32_t conflict = 0;
   assert(is_cond_setf(cond_setf));
   if (cs_is_none(cond_setf)) return 0;

   switch (alu_type)
   {
   case ALU_A: conflict |= CONFLICT_ADDFLAG; break;
   case ALU_M: conflict |= CONFLICT_MULFLAG; break;
   default: unreachable();
   }

   if (cs_is_push(cond_setf) || cs_is_updt(cond_setf)) conflict |= CONFLICT_SETFLAG;

   /* Special Case: there isn't encoding space for muf + an add thing */
   if (cs_is_updt(cond_setf) && alu_type == ALU_M) conflict |= CONFLICT_ADDFLAG;

   return conflict;
}

#if V3D_VER_AT_LEAST(4,0,2,0)
static bool is_vpm_load(BackflowFlavour f) {
   return f == BACKFLOW_LDVPMV_IN  || f == BACKFLOW_LDVPMD_IN  ||
          f == BACKFLOW_LDVPMV_OUT || f == BACKFLOW_LDVPMD_OUT ||
          f == BACKFLOW_LDVPMG_IN  || f == BACKFLOW_LDVPMG_OUT ||
          f == BACKFLOW_LDVPMP;
}
static bool is_vpm_store(BackflowFlavour f) {
   return f == BACKFLOW_STVPMV || f == BACKFLOW_STVPMD || f == BACKFLOW_STVPMP;
}
#endif

static uint32_t get_special_instruction_conflicts(BackflowFlavour op) {
   if (op == BACKFLOW_SETMSF || op == BACKFLOW_SETREVF) return CONFLICT_MSF;

   if (op == BACKFLOW_TMUWT) return CONFLICT_TMU_W | CONFLICT_TMU_R | CONFLICT_TMU_C;

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (is_vpm_load(op) || is_vpm_store(op))  return CONFLICT_VPM;
#else
   if (op == BACKFLOW_VPMSETUP) return CONFLICT_VPM;
#endif
   return 0;
}

static void read_register(GLSL_SCHEDULER_STATE_T *sched, backend_reg reg, uint32_t time) {
   assert(reg < countof(sched->reg));
   sched->reg[reg].available = gfx_umax(sched->reg[reg].available, time);
}

/* This functions like 'read_register' but cleans the register if needed  */
/* NB. Although we clean the register here, we may have been inserted before
 * another read, so the final read time of the register may be in the future. */
static void clean_register(GLSL_SCHEDULER_STATE_T *sched, backend_reg reg, uint32_t time) {
   assert(IS_RRF(reg) || IS_FLAG(reg));

   read_register(sched, reg, time);

   /* Since we didn't do this in dereference() we need to do it now. */
   if (sched->reg[reg].user != NULL && sched->reg[reg].user->remaining_dependents == 0)
      sched->reg[reg].user = NULL;
}

static void invalidate_register(GLSL_SCHEDULER_STATE_T *sched, backend_reg reg, uint32_t time)
{
   assert(reg < countof(sched->reg));

   assert(sched->reg[reg].user == NULL);
   assert(sched->reg[reg].available <= time);
   sched->reg[reg].available = time;
}

static void write_register(GLSL_SCHEDULER_STATE_T *sched, Backflow *node, backend_reg reg, uint32_t time)
{
   assert(reg < countof(sched->reg));

   /* node may be NULL, in which case we are writing throw-away data */
   if (node != NULL) node->reg = reg;
   sched->reg[reg].user = node;
   sched->reg[reg].written = time;

   assert(sched->reg[reg].available <= time);
   sched->reg[reg].available = time;
}

static inline bool no_read_clashes(const backend_reg *old_read, const backend_reg *new_read)
{
   int n_old_reads = (old_read[0] != REG_UNDECIDED) + (old_read[1] != REG_UNDECIDED);
   int n_new_reads = (new_read[0] != REG_UNDECIDED) + (new_read[1] != REG_UNDECIDED);
   if (new_read[0] == old_read[0] || new_read[0] == old_read[1]) n_new_reads--;
   if (new_read[1] == old_read[0] || new_read[1] == old_read[1]) n_new_reads--;

   return (n_old_reads + n_new_reads) < 3;
}

static inline bool can_place_alu(const INSTR_T *ci, SchedNodeType alu_type, uint32_t conflicts, const backend_reg *read)
{
   bool can_place = true;
   backend_reg instr_read[2];
   instr_get_reads(ci, instr_read);

   can_place &= ((alu_type == ALU_M) ? (!ci->m.used) : (!ci->a.used));
   can_place &= conflicts_ok(ci->conflicts, conflicts);
   can_place &= no_read_clashes(instr_read, read);

   return can_place;
}

static inline void set_alu(GLSL_OP_T *alu, BackflowFlavour op, SchedNodeUnpack ul, SchedNodeUnpack ur, backend_reg output, backend_reg input_a, backend_reg input_b, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   assert(!alu->used);

   alu->used = true;
   alu->op = op;
   alu->unpack[0] = ul;
   alu->unpack[1] = ur;
   alu->output = output;
   alu->input_a = input_a;
   alu->input_b = input_b;
   alu->cond_setf = cond_setf;
   alu->mov_excuse = mov_excuse;
}

uint32_t get_conflicts(uint32_t output, uint32_t cond_setf, SchedNodeType alu_type,
                       BackflowFlavour op, uint64_t unif)
{
   uint32_t conflicts[4];
   conflicts[0] = get_output_conflicts(output);
   conflicts[1] = get_cond_setf_conflicts(cond_setf, alu_type);
   conflicts[2] = get_special_instruction_conflicts(op);
   conflicts[3] = (unif != UNIF_NONE ? CONFLICT_UNIF : 0);
   for (int i=1; i<4; i++) {
      /* If individual conflict words aren't ok then instruction conflicts with itself */
      assert(conflicts_ok(conflicts[0], conflicts[i]));
      conflicts[0] |= conflicts[i];
   }
   return conflicts[0];
}

static uint32_t fit_alu(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, SchedNodeType alu_type,
                        BackflowFlavour op, SchedNodeUnpack ul, SchedNodeUnpack ur, backend_reg output,
                        backend_reg input_a, backend_reg input_b, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   bool mul = (alu_type == ALU_M);
   bool add = !mul;

   /* Determine conflicts */
   uint32_t conflicts = get_conflicts(output, cond_setf, alu_type, op, unif);

   /* See which regfiles we're reading from */
   backend_reg read[2] = { REG_UNDECIDED, REG_UNDECIDED };
   if (IS_RF(input_a)) add_read(read, input_a);
   if (IS_RF(input_b)) add_read(read, input_b);

   /* Advance current instruction until we find one which doesn't conflict */
   INSTR_T *ci;
   while (true) {
      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      ci = &sched->instructions[time];
      if ( can_place_alu(ci, alu_type, conflicts, read) ) break;

      if(ci->alt_mov_i) {
         /* Or in the conflicts on the instruction, which may have been updated
          * since the alt_mov_i was created. This can still lead to invalid code
          * because we don't know whether the alt_mov_i conflicts with those
          * updates. That information is now lost.
          */
         ci->alt_mov_i->conflicts |= ci->conflicts;

         bool suitable = can_place_alu(ci->alt_mov_i, alu_type, conflicts, read);

         if(suitable && add && ci->a.used && !ci->m.used && ci->alt_mov_i->m.used) {
            set_alu(&ci->m, ci->alt_mov_i->m.op, ci->alt_mov_i->m.unpack[0], ci->alt_mov_i->m.unpack[1],
                            ci->alt_mov_i->m.output, ci->alt_mov_i->m.input_a, ci->alt_mov_i->m.input_b,
                            ci->alt_mov_i->m.cond_setf, ci->alt_mov_i->m.mov_excuse);
            ci->a.used = false;
            ci->alt_mov_i = NULL;
            break;
         } else if(suitable && mul && ci->m.used && !ci->a.used && ci->alt_mov_i && ci->alt_mov_i->a.used) {
            set_alu(&ci->a, ci->alt_mov_i->a.op, ci->alt_mov_i->a.unpack[0], ci->alt_mov_i->a.unpack[1],
                            ci->alt_mov_i->a.output, ci->alt_mov_i->a.input_a, ci->alt_mov_i->a.input_b,
                            ci->alt_mov_i->a.cond_setf, ci->alt_mov_i->a.mov_excuse);
            ci->m.used = false;
            ci->alt_mov_i = NULL;
            break;
         }
      }

      time++;
   }

   /* Fill in instruction */
   assert( add || mul );
   if (add) set_alu(&ci->a, op, ul, ur, output, input_a, input_b, cond_setf, mov_excuse);
   else set_alu(&ci->m, op, ul, ur, output, input_a, input_b, cond_setf, mov_excuse);

   if (unif != UNIF_NONE) {
      assert(ci->unif == UNIF_NONE);
      ci->unif = unif;
   }
   assert(conflicts_ok(ci->conflicts, conflicts));
   ci->conflicts |= conflicts;

   return time;
}

static uint32_t fit_multi(GLSL_SCHEDULER_STATE_T *sched, uint32_t time,
                          BackflowFlavour op_a, BackflowFlavour op_m,
                          SchedNodeUnpack ul, SchedNodeUnpack ur,
                          backend_reg output, backend_reg input_a, backend_reg input_b, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   bool add = false, mul = false;
   bool add_rep = (op_m == BACKFLOW_MOV || op_m == BACKFLOW_FMOV);

   /* Determine conflicts for each lane */
   uint32_t conflicts_a = get_conflicts(output, cond_setf, ALU_A, op_a, unif);
   uint32_t conflicts_m = get_conflicts(output, cond_setf, ALU_M, op_m, unif);

   /* See which regfiles we're reading from */
   backend_reg read[2] = { REG_UNDECIDED, REG_UNDECIDED };
   if (IS_RF(input_a)) add_read(read, input_a);
   if (IS_RF(input_b)) add_read(read, input_b);

   /* Advance current instruction until we find one which doesn't conflict */
   INSTR_T *ci;
   while (true) {
      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      ci = &sched->instructions[time];
      if(can_place_alu(ci, ALU_M, conflicts_m, read)) mul = true;
      if(can_place_alu(ci, ALU_A, conflicts_a, read)) add = true;

      if( add || mul) break;

      time++;
   }

   if (unif != UNIF_NONE) {
      assert(ci->unif == UNIF_NONE);
      ci->unif = unif;
   }

   /* Fill in instruction */
   assert(add || mul);
   if (mul) {
      set_alu(&ci->m, op_m, ul, ur, output, input_a, input_b, cond_setf, mov_excuse);

      assert(conflicts_ok(ci->conflicts, conflicts_m));
      ci->conflicts |= conflicts_m;
      ci->alt_mov_i = NULL;

      if(add) {
         INSTR_T *alti = (INSTR_T*)malloc_fast(sizeof(INSTR_T));
         memset(alti, 0, sizeof(INSTR_T));

         ci->alt_mov_i = alti;
         set_alu(&alti->a, op_a, ul, add_rep ? ul : ur, output, input_a, add_rep ? input_a : input_b, cond_setf, mov_excuse);

         assert(alti->conflicts == 0);
         alti->conflicts = conflicts_a;
      }
   } else {
      set_alu(&ci->a, op_a, ul, add_rep ? ul : ur, output, input_a, add_rep ? input_a : input_b, cond_setf, mov_excuse);

      assert(conflicts_ok(ci->conflicts, conflicts_a));
      ci->conflicts |= conflicts_a;
      ci->alt_mov_i = NULL;
   }

   return time;
}

static uint32_t fit_mov(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, backend_reg output, backend_reg input, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   if (IS_FLAG(input))
      return fit_alu(sched, time, ALU_A, BACKFLOW_FLN, UNPACK_NONE, UNPACK_NONE, output, input, REG_UNDECIDED, unif, cond_setf, mov_excuse);
   else {
      assert(IS_RRF(input));
      return fit_multi(sched, time, BACKFLOW_OR, BACKFLOW_MOV, UNPACK_NONE, UNPACK_NONE, output, input, REG_UNDECIDED, unif, cond_setf, mov_excuse);
   }
}

/* No op may write rf* or read rf[0,2] on or after thrend */
static bool op_valid_for_thrend(const GLSL_OP_T *op) {
   if (!op->used) return true;

   if (IS_RF(op->output)) return false;
   if (op->input_a == REG_RF0 || op->input_a == REG_RF1 || op->input_a == REG_RF2) return false;
   if (op->input_b == REG_RF0 || op->input_b == REG_RF1 || op->input_b == REG_RF2) return false;
   return true;
}

static bool instr_valid_for_thrend(const INSTR_T *i) {
   uint32_t conflicts = V3D_VER_AT_LEAST(4,0,2,0) ? 0 : (CONFLICT_MSF | CONFLICT_VPM);
   return op_valid_for_thrend(&i->a)  && op_valid_for_thrend(&i->m)    &&
          !(i->conflicts & conflicts) && !(i->sigbits & V3D_QPU_SIG_LDVARY) &&
          i->sig_waddr == REG_UNDECIDED;
}

static bool can_add_thrsw(const GLSL_SCHEDULER_STATE_T *sched, uint32_t time, bool lthrsw, bool thrend)
{
   if (time < 3) return false;

   /* Check that this is not the delay slot of another thrsw */
   for (unsigned i=5; i>3; i--) {
      int instr = time > i ? time - i : 0;
      if (sched->instructions[instr].sigbits & V3D_QPU_SIG_THRSW) return false;
   }

   const INSTR_T *i = &sched->instructions[time-3];

   if (!can_add_sigbits(i[0].sigbits, V3D_QPU_SIG_THRSW)) return false;
   if (lthrsw && !can_add_sigbits(i[1].sigbits, V3D_QPU_SIG_THRSW)) return false;

   if (!conflicts_ok(i[3].conflicts, CONFLICT_POST_THRSW)) return false;

   if (thrend) {
      for (int j=0; j<3; j++)
         if (!instr_valid_for_thrend(&i[j])) return false;

#if !V3D_VER_AT_LEAST(4,0,2,0)
#  if !V3D_VER_AT_LEAST(3,3,0,0)
      if (i[0].conflicts & CONFLICT_UNIF) return false;
#  endif
      if (i[1].conflicts & CONFLICT_UNIF) return false;
      if (i[2].conflicts & CONFLICT_UNIF) return false;
#endif

      /* Z-writes are not allowed on the final instruction */
      /* Any z-write in the IR that may schedule last is required to load the
       * config on that instruction. (This is very likely to be the case because
       * z-writes * have to come before other writes anyway). If this weren't
       * the case then we'd have to determine the config here more cleverly */
      if ( (i[2].a.used && i[2].a.output == REG_MAGIC_TLBU) ||
           (i[2].m.used && i[2].m.output == REG_MAGIC_TLBU) )
      {
         uint8_t cfg = (i[2].unif >> 32) & 0xFF;
         if ((cfg & 0xF0) == 0x80) return false;
      }
   }
   return true;
}

/* Place thrsw. Returns the instruction which triggers the actual switch.
 * The first instruction of the new thread is the instruction after that */
static uint32_t fit_thrsw(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, bool thrend) {
   bool lthrsw = (sched->thrsw_remaining == 1 && sched->lthrsw);

   if (thrend) assert(sched->thrsw_remaining == 0);
   else {
      assert(sched->thrsw_remaining >= 1);
      sched->thrsw_remaining--;
   }

   while (true) {
      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      if (can_add_thrsw(sched, time, lthrsw, thrend)) break;

      time++;
   }

   sched->instructions[time-3].sigbits |= V3D_QPU_SIG_THRSW;
   if (lthrsw)
      sched->instructions[time-2].sigbits |= V3D_QPU_SIG_THRSW;

   uint32_t new_thread_start = time--;

   /* Work around GFXH-1369: Cannot access the TLB immediately after threadswitch */
   sched->instructions[new_thread_start].conflicts |= CONFLICT_POST_THRSW;

   /* Make sure writes to accumulators don't get scheduled *before* the thrsw */
   for (int i = 0; i < 6; i++)
      invalidate_register(sched, REG_R(i), new_thread_start);

   return time;
}

/* Insert code to calculate the varying value.
 * The dependencies determine where the "ldvary" may be placed. The return value
 * is the first instruction that can read from the fadd result.  */
static uint32_t fit_varying(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t output,
                            backend_reg temp_reg, backend_reg w_reg, uint32_t varying_id, VaryingType varying_type)
{
   assert((varying_type == VARYING_FLAT                                ) ||
          (varying_type == VARYING_LINE_COORD && w_reg == REG_UNDECIDED) ||
          (varying_type == VARYING_DEFAULT    && w_reg != REG_UNDECIDED)    );

   GLSL_OP_T v_ops[3][2] = {
      /* Default */
      { { true, BACKFLOW_MUL, { UNPACK_NONE, UNPACK_NONE }, temp_reg, REG_R3_,    w_reg, SETF_NONE},
        { true, BACKFLOW_ADD, { UNPACK_NONE, UNPACK_NONE }, output,   temp_reg, REG_R5_, SETF_NONE} },
      /* Line Coord */
      { { true, BACKFLOW_MOV, { UNPACK_NONE, UNPACK_NONE }, temp_reg, REG_R3_,   0,      SETF_NONE},
        { true, BACKFLOW_ADD, { UNPACK_NONE, UNPACK_NONE }, output,   temp_reg, REG_R5_, SETF_NONE} },
      /* Flat */
      { { false, 0,                                                                              },
        { true,  BACKFLOW_OR, { UNPACK_NONE, UNPACK_NONE }, output, REG_R5_, REG_R5_, SETF_NONE} } };
   GLSL_OP_T *v = v_ops[varying_type-1];

   uint32_t sig_conflict = sig_conflicts(V3D_QPU_SIG_LDVARY);
   uint32_t conflicts[2] = { 0, 0 };
   backend_reg read[2][2] = { { REG_UNDECIDED, REG_UNDECIDED }, { REG_UNDECIDED, REG_UNDECIDED} };
   for (int i=0; i<2; i++) {
      if (v[i].used) {
         conflicts[i] = get_output_conflicts(v[i].output);
         if (IS_RF(v[i].input_a)) add_read(read[i], v[i].input_a);
         if (IS_RF(v[i].input_b)) add_read(read[i], v[i].input_b);
      }
   }

   INSTR_T *ci;
   while (true) {
      if (time + 2 >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      ci = &sched->instructions[time];

      bool suitable = can_add_sigbits(ci[0].sigbits, V3D_QPU_SIG_LDVARY) &&
                      conflicts_ok(ci[0].conflicts, sig_conflict);
      if (v[0].used) suitable &= can_place_alu(&ci[1], ALU_M, conflicts[0], read[0]);
      if (v[1].used) suitable &= can_place_alu(&ci[2], ALU_A, conflicts[1], read[1]);
      if (suitable) break;

      time++;
   }

   ci[0].sigbits |= V3D_QPU_SIG_LDVARY;
   ci[0].varying = varying_id;
   ci[0].conflicts |= sig_conflict;

   for (int i=0; i<2; i++) {
      if (v[i].used) {
         GLSL_OP_T *dest = (i == 0) ? &ci[i+1].m : &ci[i+1].a;
         set_alu(dest, v[i].op, v[i].unpack[0], v[i].unpack[1], v[i].output, v[i].input_a, v[i].input_b,
                       v[i].cond_setf, v[i].mov_excuse);

         ci[i+1].conflicts |= conflicts[i];
      }
   }

   /* Update our temporary registers. Main code updates inputs/outputs */
   assert(sched->reg[REG_R3_].user == NULL);
   assert(sched->reg[REG_R5_].user == NULL);
   assert(sched->reg[temp_reg].user == NULL);
   write_register(sched, NULL, REG_R3_, time + 1);
   write_register(sched, NULL, REG_R5_, time + 2);
   write_register(sched, NULL, temp_reg, time + 2);
   read_register(sched, REG_R3_, time + 1);
   read_register(sched, REG_R5_, time + 2);
   read_register(sched, temp_reg, time + 2);

   return time + 1;
}

static uint32_t fit_imul32(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, backend_reg output, backend_reg left, backend_reg right)
{

   GLSL_OP_T v[2] = {
        { true, BACKFLOW_MULTOP, { UNPACK_NONE, UNPACK_NONE }, REG_MAGIC_NOP, left, right, SETF_NONE},
        { true, BACKFLOW_UMUL,   { UNPACK_NONE, UNPACK_NONE }, output,        left, right, SETF_NONE} };

   //conflicts = get_conflicts(output, cond_setf, alu_type, op, op1, op2, unif);
   uint32_t conflicts[2] = { 0, 0 };
   backend_reg read[2][2] = { { REG_UNDECIDED, REG_UNDECIDED }, { REG_UNDECIDED, REG_UNDECIDED} };
   for (int i=0; i<2; i++) {
      if (v[i].used) {
         conflicts[i] = get_output_conflicts(v[i].output);
         if (IS_RF(v[i].input_a)) add_read(read[i], v[i].input_a);
         if (IS_RF(v[i].input_b)) add_read(read[i], v[i].input_b);
      }
   }

   INSTR_T *ci;
   while (true) {
      if (time + 1 >= MAX_INSTRUCTIONS) backend_error("Too many instructions");

      ci = &sched->instructions[time];
      bool suitable = true;
      for (int i=0; i<2; i++)
         suitable &= can_place_alu(&ci[i], ALU_M, conflicts[i], read[i]);

      if (suitable) break;

      time++;
   }

   for (int i=0; i<2; i++) {
      if (v[i].used) {
         set_alu(&ci[i].m, v[i].op, v[i].unpack[0], v[i].unpack[1], v[i].output, v[i].input_a, v[i].input_b,
                           v[i].cond_setf, v[i].mov_excuse);

         ci[i].conflicts |= conflicts[i];
      }
   }

   /* Update our temporary registers. Main code updates inputs/outputs */
   read_register(sched, left,  time);
   read_register(sched, right, time);

   read_register(sched, left,     time + 1);
   read_register(sched, right,    time + 1);
   read_register(sched, REG_RTOP, time + 1);
   /* The final write of the output is taken care of below */

   return time + 1;
}

/* May or may not choose a register in r0-r4.
   If insist is true then it will always choose something.  */
static backend_reg choose_acc(const GLSL_SCHEDULER_STATE_T *sched, bool insist, uint32_t time)
{
   uint32_t oldest = ~0u;
   backend_reg oldest_r = REG_UNDECIDED;

   /* Choose a free register, failing that one that wasn't used recently */
   for (int i = 0; i < 5; i++) {
      if ( sched->reg[REG_R(i)].user == NULL ) return REG_R(i);

      if (sched->reg[REG_R(i)].written < oldest) {
         oldest = sched->reg[REG_R(i)].written;
         oldest_r = REG_R(i);
      }
   }
   assert(oldest_r != REG_UNDECIDED);
   if (oldest + 4 < time) return oldest_r;

   /* If we've insisted, then return the least recently used */
   if (insist) return oldest_r;
   else return REG_UNDECIDED;
}

static backend_reg choose_regfile(const GLSL_SCHEDULER_STATE_T *sched, uint32_t time)
{
   uint32_t best_time = ~0u;
   backend_reg best = REG_UNDECIDED;
   for (unsigned j = 0; j != sched->regfile_max; ++j)
   {
      // permute priority of regfile to avoid likelyhood of rf0-2 near thrend
      unsigned i = (j + 3) % sched->regfile_max;

      if (sched->register_blackout & (1ull << i)) continue;

      if (sched->reg[REG_RF(i)].user == NULL) {
         if (sched->reg[REG_RF(i)].available < best_time) {
            best_time = sched->reg[REG_RF(i)].available;
            best = REG_RF(i);
         }
      }
      if (best_time <= time) break;
   }

   if (best == REG_UNDECIDED) backend_error("No regfile space");

   return best;
}

/*
   Move a value in a register into a different register. We update the register
   tracking structures so that everything is consistent in the new state
*/
static void move_to_register(GLSL_SCHEDULER_STATE_T *sched, backend_reg reg,
                             backend_reg output, uint32_t mov_excuse)
{
   /* Validate input and output choices */
   assert(IS_RRF(reg) || IS_FLAG(reg));
   assert(sched->reg[reg].user != NULL);
   assert(IS_RRF(output));
   assert(sched->reg[output].user == NULL);
   assert(output != reg);

   Backflow *node = sched->reg[reg].user;
   assert(node->reg == reg);

   assert(node->remaining_dependents > 0);

   /* Place after the result has been written and the output reg is free */
   uint32_t mov_timestamp = sched->reg[reg].written;
   mov_timestamp = gfx_umax(mov_timestamp, sched->reg[output].available);
   mov_timestamp = fit_mov(sched, mov_timestamp, output, reg, UNIF_NONE, SETF_NONE, mov_excuse);
   uint32_t delay = get_output_delay(output);

   write_register(sched, node, output, mov_timestamp + delay);

   sched->reg[reg].user = NULL;
   read_register(sched, reg, mov_timestamp);
}

static void move_result(GLSL_SCHEDULER_STATE_T *sched, backend_reg reg, uint32_t mov_excuse)
{
   if (sched->reg[reg].user == NULL) return;    /* Nothing to move */
   uint32_t timestamp_guess = sched->reg[reg].written;
   move_to_register(sched, reg, choose_regfile(sched, timestamp_guess), mov_excuse);
}

static uint64_t node_get_unif(const Backflow *node)
{
   assert(node->unif_type >= 0 &&
          node->unif_type <= BACKEND_UNIFORM_LAST_ELEMENT);

   if (node->unif_type == BACKEND_UNIFORM_UNASSIGNED) {
      assert(node->unif == 0);
      return UNIF_NONE;
   }

   return UNIF_PAIR(node->unif_type, node->unif);
}

static uint32_t find_output_reg(const Backflow *node) {
   if (node->type == SIG) {
      assert(node->magic_write == REG_UNDECIDED);
      switch (node->u.sigbits) {
      case V3D_QPU_SIG_LDUNIF: return REG_R5_;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case V3D_QPU_SIG_LDTMU:  return REG_R4_;
      case V3D_QPU_SIG_LDTLB:  return REG_R3_;
      case V3D_QPU_SIG_LDTLBU: return REG_R3_;
      case V3D_QPU_SIG_LDVPM:  return REG_R3_;
#else
      case V3D_QPU_SIG_LDTMU:  return REG_UNDECIDED;
      case V3D_QPU_SIG_LDTLB:  return REG_UNDECIDED;
      case V3D_QPU_SIG_LDTLBU: return REG_UNDECIDED;
#endif
      default:            return REG_MAGIC_NOP; /* Other SIGS have no output */
      }
   }

   switch (node->magic_write) {
      /* Node genuinely has no output */
      case REG_MAGIC_NOP:  return REG_MAGIC_NOP;
      /* Output to the SFU comes back in r4 */
      case REG_MAGIC_RECIP:
      case REG_MAGIC_RSQRT:
      case REG_MAGIC_RSQRT2:
      case REG_MAGIC_LOG:
      case REG_MAGIC_EXP:
      case REG_MAGIC_SIN:  return REG_R4_;
      /* These are shader output so the results aren't returned */
      case REG_MAGIC_TMUA:
      case REG_MAGIC_TMUAU:
      case REG_MAGIC_TMUD:
#if V3D_VER_AT_LEAST(4,0,2,0)
      case REG_MAGIC_TMUC:
      case REG_MAGIC_TMUS:
      case REG_MAGIC_TMUT:
      case REG_MAGIC_TMUR:
      case REG_MAGIC_TMUI:
      case REG_MAGIC_TMUB:
      case REG_MAGIC_TMUDREF:
      case REG_MAGIC_TMUOFF:
      case REG_MAGIC_TMUSCM:
      case REG_MAGIC_TMUSF:
      case REG_MAGIC_TMUSLOD:
#else
      case REG_MAGIC_TMU:
      case REG_MAGIC_TMUL:
      case REG_MAGIC_VPM:
      case REG_MAGIC_VPMU:
#endif
      case REG_MAGIC_TLBU:
      case REG_MAGIC_TLB:
         return REG_MAGIC_NOP;
      default:
         break;
   }

   return REG_UNDECIDED;
}

static inline void validate_reg(const GLSL_SCHEDULER_STATE_T *sched, const Backflow *node) {
   if (node == NULL) return;

   assert(IS_RRF(node->reg) || IS_FLAG(node->reg));
   assert(sched->reg[node->reg].user == node);
   assert(node->remaining_dependents >= 1);
}

static void flagify(GLSL_SCHEDULER_STATE_T *sched, Backflow *node, bool require_a)
{
   if (node->reg != REG_FLAG_A && (node->reg != REG_FLAG_B || require_a)) {
      if (node->reg != REG_FLAG_B)
         move_result(sched, REG_FLAG_B, MOV_EXCUSE_FLAG_B_OUT_OF_THE_WAY);

      /* Going to clobber both flag registers so wait until after last read */
      uint32_t timestamp = sched->reg[REG_FLAG_A].available;
      timestamp = gfx_umax(timestamp, sched->reg[REG_FLAG_B].available);
      timestamp = gfx_umax(timestamp, sched->reg[node->reg].written);
      timestamp = fit_mov(sched, timestamp, REG_MAGIC_NOP, node->reg, UNIF_NONE, SETF_PUSHZ, MOV_EXCUSE_FLAGIFY);
      uint32_t delay = get_output_delay(REG_MAGIC_NOP);
      read_register(sched, node->reg, timestamp);
      sched->reg[node->reg].user = NULL;

      /* Push the a-flag down to b and write the result to a */
      write_register(sched, sched->reg[REG_FLAG_A].user, REG_FLAG_B, timestamp+delay);
      write_register(sched, node,                        REG_FLAG_A, timestamp+delay);
   }
}

static uint32_t dereference(const GLSL_SCHEDULER_STATE_T *sched, Backflow *node)
{
   assert(node->remaining_dependents >= 1);
   node->remaining_dependents--;

   /*
   Don't update sched->reg_user[reg] or sched->reg_available[reg] yet.

   reg_available holds the latest instruction to read from node, excluding the current one.
   (The current instruction isn't going to interfere with itself).
   We update reg_available once we know where the current instruction will get put.

   We don't update reg_user yet because we want to avoid this register from being allocated to
   something else (i.e. move_result) until we're really finished with it.
   */

   return node->reg;
}

static uint32_t specialise_cond(uint32_t cond_setf, uint32_t reg) {
   assert(IS_FLAG(reg));
   if (cond_setf == COND_IFFLAG)
      return reg == REG_FLAG_A ? COND_IFA  : COND_IFB;
   if (cond_setf == COND_IFNFLAG)
      return reg == REG_FLAG_A ? COND_IFNA : COND_IFNB;
   return cond_setf; /* Other conditions don't need specialising */
}

static bool can_use_both_lanes(SchedNodeType type, BackflowFlavour op) {
   return (type == ALU_A && (op == BACKFLOW_IADD   || op == BACKFLOW_ISUB))   ||
          (type == ALU_M && (op == BACKFLOW_IADD_M || op == BACKFLOW_ISUB_M)) ||
          (type == ALU_M && op == BACKFLOW_FMOV);
}

static const BackflowFlavour valid[3][2] = { { BACKFLOW_MIN,  BACKFLOW_FMOV   },
                                             { BACKFLOW_IADD, BACKFLOW_IADD_M },
                                             { BACKFLOW_ISUB, BACKFLOW_ISUB_M } };

static const BackflowFlavour *multi_get_ops(SchedNodeType t, BackflowFlavour op) {
   for (int i=0; i<3; i++)
      if (valid[i][ (t == ALU_M) ] == op) return valid[i];
   unreachable();
}

static void schedule_node(GLSL_SCHEDULER_STATE_T *sched, Backflow *node)
{
   backend_reg temp_reg = REG_UNDECIDED;
   backend_reg input_a  = REG_UNDECIDED;
   backend_reg input_b  = REG_UNDECIDED;
   backend_reg flag_reg = REG_UNDECIDED;

   /* Store copies of things. Some so we can modify, others just for kicks */
   backend_reg waddr = node->magic_write;
   uint32_t cond_setf = node->cond_setf;

   uint64_t unif = node_get_unif(node);
   /* If the result comes to a fixed register (sig, sfu, etc) fill in which */
   node->reg = find_output_reg(node);

   Backflow *flag_dep     = node->dependencies[0];  /* cond_setf determines usage */
   Backflow *output_dep   = node->dependencies[3];  /* NULL if not overwriting another node */
   Backflow *input_dep[2] = { node->dependencies[1], node->dependencies[2] };

#ifndef NDEBUG
   for (int i=0; i<BACKFLOW_DEP_COUNT; i++) validate_reg(sched, node->dependencies[i]);
#endif

   /* If we need flags as inputs(/outputs) sort them out first */
   if (flag_dep != NULL) {
      bool writes_flag_a = cs_is_updt(cond_setf);
      flagify(sched, flag_dep, writes_flag_a);
      flag_reg = dereference(sched, flag_dep);

      if (flag_dep->remaining_dependents != 0 && writes_flag_a) {
         move_result(sched, flag_reg, MOV_EXCUSE_COPY_ON_WRITE);
      }
      /* XXX We only do a copy on write above, not a move out of the way */

      /* If we need to specialise based on which flag, do that now */
      cond_setf = specialise_cond(cond_setf, flag_reg);
      assert(!writes_flag_a || flag_reg == REG_FLAG_A);
   }

   /* Now sort out the dependencies of our outputs, starting with flags */
   /* Push clobbers b-flag */
   if (cs_is_push(cond_setf)) {
      move_result(sched, REG_FLAG_B, MOV_EXCUSE_FLAG_B_OUT_OF_THE_WAY);
   }

   if (output_dep != NULL) {
      assert(cs_is_cond(cond_setf));
      assert(waddr == REG_UNDECIDED);
      assert(node->reg == REG_UNDECIDED);

      /* We can't write to r5 */
      uint32_t cur_reg = output_dep->reg;
      if (cur_reg == REG_R5_) move_result(sched, cur_reg, MOV_EXCUSE_R5_WRITE);
      if (IS_RF(cur_reg) && ((1ull << (cur_reg - REG_RF0)) & sched->register_blackout) != 0)
         move_result(sched, cur_reg, MOV_EXCUSE_COPY_ON_WRITE);

      node->reg = dereference(sched, output_dep);

      if (output_dep->remaining_dependents != 0)
         move_result(sched, node->reg, MOV_EXCUSE_COPY_ON_WRITE);

      assert(IS_RRF(node->reg));  /* Should be writing to a reg not a flag */
   }
   if (cs_is_updt(cond_setf) || cs_is_push(cond_setf)) {
      /* We can't currently write to both a register and a flag */
      assert(waddr == REG_UNDECIDED);
      assert(node->reg == REG_UNDECIDED);
      node->reg = REG_FLAG_A;
      waddr = REG_MAGIC_NOP;
   }

   /*
   Decrement reference count for these inputs. Don't retire them, lest we
   overwrite the register with a move later.
   */
   if (input_dep[0] != NULL) input_a = dereference(sched, input_dep[0]);
   if (input_dep[1] != NULL) input_b = dereference(sched, input_dep[1]);

   if (node->type == ALU_A && (node->u.alu.op == BACKFLOW_FL || node->u.alu.op == BACKFLOW_FLN)) {
      assert(input_a == REG_UNDECIDED && IS_FLAG(flag_reg));
      input_a = flag_reg;
      flag_reg = REG_UNDECIDED;
   }

   /*
   Special register allocation for VARYING.
   The boilerplate code requires a temporary register and also makes use of r3 and r5.

   Note that it *is* safe for the temporary and output registers to be the same. It's also
   safe to use r3 as the temporary register (though it makes stacking less efficient).
   So we're sloppy about making sure the same thing doesn't get allocated for two different
   purposes, but it won't actually break anything.
   */
   if (node->type == SPECIAL_VARYING) {
      move_result(sched, REG_R5_, MOV_EXCUSE_OUT_OF_THE_WAY);
      move_result(sched, REG_R3_, MOV_EXCUSE_OUT_OF_THE_WAY);
      temp_reg = choose_acc(sched, true, 0);   /* Insist on acc. Not sure why. Time doesn't matter when insisting */
      move_result(sched, temp_reg, MOV_EXCUSE_OUT_OF_THE_WAY);
   }

   /* Accumulators aren't preserved over a thrsw */
   if (node->type == SPECIAL_THRSW)
      for (int i = 0; i < 6; i++) move_result(sched, REG_R(i), MOV_EXCUSE_THRSW);

   /* First guess the final scheduling time ... */
   uint32_t timestamp = 0, delay;

   if (node->type == SPECIAL_VARYING) {
      timestamp = gfx_umax(timestamp, sched->reg[REG_R3_].available);
      timestamp = gfx_smax(timestamp, sched->reg[temp_reg].available-1);
      timestamp = gfx_smax(timestamp, sched->reg[REG_R5_].available-1);
   }
   if (node->type == SPECIAL_THRSW) {
      /* offset==1 because timestamp wants to be *after* the mov, not on the same line. */
      for (int i=0; i<6; i++) timestamp = gfx_umax(timestamp, sched->reg[REG_R(i)].available+1);
      timestamp = gfx_umax(timestamp, sched->reg[REG_RTOP].available + 1);
   }

   /* Wait until the inputs we need to read have been written */
   if (flag_reg  != REG_UNDECIDED) timestamp = gfx_umax(timestamp, sched->reg[flag_reg ].written);
   if (input_a   != REG_UNDECIDED) timestamp = gfx_umax(timestamp, sched->reg[input_a  ].written);
   if (input_b   != REG_UNDECIDED) timestamp = gfx_umax(timestamp, sched->reg[input_b  ].written);
   /* Wait until any register we'll write to has been read for the last time */
   if (node->reg != REG_MAGIC_NOP) timestamp = gfx_umax(timestamp, sched->reg[node->reg].available);
   if (cs_is_push(cond_setf)) timestamp = gfx_umax(timestamp, sched->reg[REG_FLAG_B].available);
   /* Wait for our iodependencies */
   BackflowChainNode *chain_node;
   LIST_FOR_EACH(chain_node, &node->io_dependencies, l)
      timestamp = gfx_umax(timestamp, chain_node->ptr->io_timestamp);

   /* Register allocation. If not needed then REG_MAGIC_NOP should already be set */
   if (node->reg == REG_UNDECIDED) {
      /* Try an accumulator first then fallback to regfile */
      if ( !(node->type == ALU_A && glsl_sched_node_requires_regfile(node->u.alu.op)) && node->type != SIG) {
         if (node->remaining_dependents == 0) node->reg = REG_MAGIC_NOP;
         else if (node->remaining_dependents == 1) node->reg = choose_acc(sched, false, timestamp);
      }
      if (node->reg == REG_UNDECIDED) node->reg = choose_regfile(sched, timestamp);
   }
   assert(node->reg != REG_UNDECIDED);
   /* If this node isn't a magic write, set waddr to the output. */
   if (waddr == REG_UNDECIDED) waddr = node->reg;

   if (node->reg != REG_MAGIC_NOP) {
      /* Insert movs if we're writing to something which is already in use */
      assert(IS_RRF(node->reg) || IS_FLAG(node->reg));
      if (sched->reg[node->reg].user && sched->reg[node->reg].user->remaining_dependents != 0)
         move_result(sched, node->reg, MOV_EXCUSE_OUT_OF_THE_WAY);
   }

   /* Wait until any register we'll write to has been read for the last time */
   if (node->reg != REG_MAGIC_NOP) timestamp = gfx_umax(timestamp, sched->reg[node->reg].available);

   switch (node->type)
   {
   case SIG:
      timestamp = fit_sig(sched, timestamp, node->u.sigbits, unif, node->reg);
      delay = 1;
      break;
   case SPECIAL_THRSW:
      timestamp = fit_thrsw(sched, timestamp, false);
      delay = 0;
      break;
   case SPECIAL_VARYING:
      /* For a varying input_a contains our w value */
      /* TODO: This puts the iotimestamp 1 instruction too late lest input_a should be marked free too early (see below) */
      /* TODO: There's no reason we can't load a unif here, except that we don't track conflicts for it */
      assert(node->unif == UNIF_NONE);
      timestamp = fit_varying(sched, timestamp, waddr, temp_reg, input_a, node->u.varying.row, node->u.varying.type);
      delay = 1 + get_output_delay(waddr);
      break;
   case SPECIAL_IMUL32:
      timestamp = fit_imul32(sched, timestamp, waddr, input_a, input_b);
      delay = get_output_delay(waddr);
      break;
   case ALU_A:
   case ALU_M:
      if (node->type == ALU_M && node->u.alu.op == BACKFLOW_MOV) {
         timestamp = fit_mov(sched, timestamp, waddr, input_a, unif, cond_setf, MOV_EXCUSE_IR);
      } else if (can_use_both_lanes(node->type, node->u.alu.op)) {
         uint32_t mov_excuse = (node->type == ALU_M && node->u.alu.op == BACKFLOW_FMOV) ? MOV_EXCUSE_IR : MOV_EXCUSE_NONE;
         const BackflowFlavour *ops = multi_get_ops(node->type, node->u.alu.op);
         timestamp = fit_multi(sched, timestamp, ops[0], ops[1],
                               node->u.alu.unpack[0], node->u.alu.unpack[1],
                               waddr, input_a, input_b, unif, cond_setf, mov_excuse);
      } else {
         timestamp = fit_alu(sched, timestamp, node->type,
                             node->u.alu.op, node->u.alu.unpack[0], node->u.alu.unpack[1],
                             waddr,
                             input_a, input_b, unif, cond_setf, MOV_EXCUSE_NONE);
      }
      delay = get_output_delay(waddr);
      break;
   case SPECIAL_VOID:
      /* timestamp = timestamp; */
      delay = 0;
      break;
   default:
      unreachable();
   }

   node->io_timestamp = timestamp + 1;  /* We must go *after* the actual timestamp */
   if (node->type == SPECIAL_VOID || node->type == SPECIAL_VARYING)
      node->io_timestamp--;

   /* Mark reads on all the registers and clean them if done with */
   if (flag_reg != REG_UNDECIDED) clean_register(sched, flag_reg, timestamp);
   if (input_a  != REG_UNDECIDED) clean_register(sched, input_a,  timestamp);
   if (input_b  != REG_UNDECIDED) clean_register(sched, input_b,  timestamp);

   if (cs_is_updt(cond_setf) || cs_is_push(cond_setf)) {
      /* TODO: how to generate instructions which both write somewhere and update flags? */
      assert(waddr == REG_MAGIC_NOP);
      assert(node->reg == REG_FLAG_A);
   }

   if (cs_is_push(cond_setf)) {
      /* Update flag B with the pushed value */
      assert(sched->reg[REG_FLAG_B].user == NULL);
      write_register(sched, sched->reg[REG_FLAG_A].user, REG_FLAG_B, timestamp+delay);
   }

   if (node->reg != REG_MAGIC_NOP)
   {
      /* Should write to somewhere "normal" that noone else is using */
      assert(IS_RRF(node->reg) || IS_FLAG(node->reg));
      assert(sched->reg[node->reg].user == NULL || sched->reg[node->reg].user == output_dep ||
                                                   sched->reg[node->reg].user == flag_dep);

      write_register(sched, node, node->reg, timestamp + delay);
      /* Sometimes a node has iodependencies only, e.g. an attrib */
      if (node->remaining_dependents == 0) sched->reg[node->reg].user = NULL;
   }
}

#ifdef KHRN_SHADER_STATS
#include <stdio.h>

static void print_shader_stats(const GLSL_SCHEDULER_STATE_T *sched, int n_instrs) {
   int a_valid = 0, a_nop = 0, a_mov = 0, m_valid = 0, m_nop = 0, m_mov = 0;
   int unifs = 0;
   bool tmu = false;
   int n_mov_excuses[MOV_EXCUSE_COUNT] = { 0,  };

   for (int i=0; i<n_instrs; i++) {
      if (sched->instructions[i].a.used) a_valid++;
      else a_nop++;
      if (sched->instructions[i].m.used) m_valid++;
      else m_nop++;

      if (sched->instructions[i].a.mov_excuse != MOV_EXCUSE_NONE) {
         n_mov_excuses[sched->instructions[i].a.mov_excuse] ++;
         a_mov++;
      }
      if (sched->instructions[i].m.mov_excuse != MOV_EXCUSE_NONE) {
         n_mov_excuses[sched->instructions[i].m.mov_excuse] ++;
         m_mov++;
      }

      if (sched->instructions[i].unif != UNIF_NONE) unifs++;
      if (sched->instructions[i].sigbits & V3D_QPU_SIG_LDTMU) tmu = true;
   }
   a_valid -= a_mov;
   m_valid -= m_mov;
   int reg_used = sched->regfile_max;
   for (int i=sched->regfile_max-1; i>=0; i--) {
      if (sched->reg[REG_RF(i)].available != 0) break;
      reg_used = i;
   }

#if V3D_HAS_RELAXED_THRSW
   tmu = true;
#endif
   printf("Shader stats:\n");
   printf("   Shader Type: %s%s, N threads: %d/%d\n",
          glsl_shader_flavour_name(sched->shader_flavour), sched->bin_mode ? " (bin)": "",
          sched->threadability, tmu ? 4 : 1);
   printf("   Total instructions: %d\n", n_instrs);
   printf("   Uniform reads:      %d\n", unifs);
   printf("   Registers used:     %d\n", reg_used);
   printf("                         a    m\n");
   printf("   Valid Instructions: %3d  %3d\n", a_valid, m_valid);
   printf("   Movs:               %3d  %3d\n", a_mov, m_mov);
   printf("   Nops:               %3d  %3d\n", a_nop, m_nop);
   printf("   Utilisation (%%):    %3d  %3d\n", (int)((100.0f*a_valid)/n_instrs), (int)((100.0f*m_valid)/n_instrs));
   printf("\n");
   printf("   Mov excuses:\n");
   for (int i=1; i<MOV_EXCUSE_COUNT; i++) {
      if (n_mov_excuses[i] > 0) {
         printf("  %s   %3d\n", mov_excuse_strings[i], n_mov_excuses[i]);
      }
   }
   printf("\n");
}
#endif

typedef struct {
   Backflow *nodes[MAX_STACK];
   int depth;
} SchedStack;

static void stack_push(SchedStack *stack, Backflow *d)
{
   assert(d->phase == PHASE_UNVISITED);
   if (stack->depth == MAX_STACK-1) backend_error("Stack overflow");

   d->phase = PHASE_STACKED;
   stack->depth++;
   stack->nodes[stack->depth] = d;
}

static Backflow *get_unvisited_dep(Backflow *node) {
   BackflowChainNode *chain_node;
   LIST_FOR_EACH(chain_node, &node->io_dependencies, l)
   {
      assert(chain_node->ptr->phase != PHASE_STACKED);
      if (chain_node->ptr->phase == PHASE_UNVISITED)
         return chain_node->ptr;
   }

   for (int i = 0; i < BACKFLOW_DEP_COUNT; i++)
   {
      if (node->dependencies[i] == NULL) continue;
      assert(node->dependencies[i]->phase != PHASE_STACKED);
      if (node->dependencies[i]->phase == PHASE_UNVISITED)
         return node->dependencies[i];
   }
   return NULL;
}

static bool all_deps_ready(const Backflow *node) {
   for (int i=0; i<BACKFLOW_DEP_COUNT; i++) {
      if (node->dependencies[i] != NULL && node->dependencies[i]->phase != PHASE_COMPLETE) return false;
   }
   BackflowChainNode *n;
   LIST_FOR_EACH(n, &node->io_dependencies, l) {
      if (n->ptr->phase != PHASE_COMPLETE) return false;
   }
   return true;
}

static void visit_recursive(SchedStack *stack, GLSL_SCHEDULER_STATE_T *sched, Backflow *root)
{
   if (root->phase == PHASE_COMPLETE) return;

   stack_push(stack, root);
   while(stack->depth >= 0) {
      /* Hunt for a dependency that we haven't visited yet. */
      Backflow *node = stack->nodes[stack->depth];
      Backflow *dep = get_unvisited_dep(node);
      if (dep != NULL) {
         stack_push(stack, dep);
         continue;               /* Recurse */
      }

      /* Already visited all dependencies so deal with this node. */
      schedule_node(sched, node);
      assert(node->phase == PHASE_STACKED);
      node->phase = PHASE_COMPLETE;
      stack->depth--;

      /* Determine how many regfile locations are in use */
      int threshold = sched->regfile_usable - ((sched->regfile_max * 14) / 64);

      int p = 0;
      for (unsigned i=0; i<sched->regfile_max; i++) {
         if (sched->reg[REG_RF(i)].user != NULL) p++;
      }

      /* If this exceeds a threshold (arbitrarily set) then search for
       * nodes that free up locations and schedule them next.           */
      if (p > threshold) {
         for (unsigned i=0; i<sched->regfile_max; i++) {
            if (sched->reg[REG_RF(i)].user) {
               Backflow *node = sched->reg[REG_RF(i)].user;
               /* Having only 1 dependent means that scheduling it will free the register */
               if (node->remaining_dependents != 1) continue;

               /* Search for the dependent */
               BackflowChainNode *n;
               for (n = node->data_dependents.head; n; n=n->l.next) {
                  /* n->ptr may not be needed for the shader, so check if anything will use it */
                  if (n->ptr->phase != PHASE_COMPLETE) break;
               }
               /* If the 1 remaining_dependent is a block output then there may be no node */
               if (n == NULL) continue;

               /* This is only helpful (or correct) if we don't have to schedule any other things first */
               if (n->ptr->phase == PHASE_UNVISITED && all_deps_ready(n->ptr)) stack_push(stack, n->ptr);
            }
         }
      }
   }
}

static void prescheduled(GLSL_SCHEDULER_STATE_T *sched, Backflow *node, uint32_t reg) {
   /* TODO: Decide which of these conditions means 'not used' */
   if (node != NULL && node->remaining_dependents > 0) {
      assert(sched->reg[reg].user == NULL);
      write_register(sched, node, reg, 0);

      /* Remove this from consideration. We don't need to schedule it */
      node->phase = PHASE_COMPLETE;
   }
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
static void break_alias(GLSL_SCHEDULER_STATE_T *sched) {
   if (sched->reg[REG_RF0].user != NULL && sched->reg[REG_RF1].user != NULL)
   {
      uint32_t time = fit_mov(sched, 0, REG_RF0, REG_RF0, UNIF_NONE,
                              SETF_NONE, MOV_EXCUSE_OUT_OF_THE_WAY);
      assert(time == 0);
   }
}
#endif

GENERATED_SHADER_T *glsl_backend(BackflowChain          *iodeps,
                                 ShaderFlavour          flavour,
                                 bool                   bin_mode,
                                 uint32_t               thrsw_count,
                                 uint32_t               threadability,
                                 BackflowPriorityQueue *sched_queue,
                                 RegList               *presched,
                                 RegList               *outputs,
                                 int                    branch_idx,
                                 int                    blackout,
                                 bool                   last,
                                 bool                   lthrsw)
{
   GENERATED_SHADER_T *gen = NULL;

   SchedStack *stack = glsl_safemem_malloc(sizeof(SchedStack));
   stack->depth = -1;

   GLSL_SCHEDULER_STATE_T *sched = glsl_safemem_calloc(1, sizeof(GLSL_SCHEDULER_STATE_T));

   if(setjmp(s_BackendErrorHandlerEnv) != 0) {
      /* It wouldn't leak if we didn't free this, but then again
         it's not a great plan to waste the memory each time */
      glsl_safemem_free(stack);
      glsl_safemem_free(sched);
      glsl_safemem_free(gen);
      return NULL;
   }

   sched->shader_flavour = flavour;
   sched->bin_mode       = bin_mode;
   sched->register_blackout = gfx_mask64(blackout);
   sched->regfile_max = get_max_regfile(threadability);
   sched->regfile_usable = sched->regfile_max - blackout;

   /* If data starts in the regfile record that now */
   for ( ; presched; presched = presched->next)
      prescheduled(sched, presched->node, presched->reg);

   /* Remove any register that we output to from the blackout */
   for (RegList *l = outputs; l!=NULL; l=l->next) {
      uint64_t reg_bit = (1ull << (l->reg - REG_RF0));
      sched->register_blackout &= ~reg_bit;
      sched->regfile_usable += 1;
   }

#if !V3D_VER_AT_LEAST(4,0,2,0)
   /* Break any potential aliasing between w and w_c immediately */
   break_alias(sched);
#endif

   sched->lthrsw          = lthrsw;
   sched->thrsw_remaining = thrsw_count;
   sched->threadability   = threadability;

   /* TLB access is not allowed in the first instruction */
   sched->instructions[0].conflicts = CONFLICT_POST_THRSW;

   while (true)
   {
      Backflow *d = glsl_backflow_priority_queue_pop(sched_queue);
      if (d == NULL) break;

      visit_recursive(stack, sched, d);
   }
   for (RegList *r = outputs; r; r=r->next) {
      visit_recursive(stack, sched, r->node);
   }
   for (BackflowChainNode *n=iodeps->head; n; n=n->l.next)
      visit_recursive(stack, sched, n->ptr);

   uint32_t instr_count = 0;
   if (!last) {
      /* Restore the complete blackout here. A single output value that is being
       * placed in multiple registers goes wrong otherwise. It will be moved out
       * of its first register to the second, and the first could be reused. */
      for (RegList *r = outputs; r; r=r->next) {
         uint64_t reg_bit = (1ull << (r->reg - REG_RF0));
         sched->register_blackout |= reg_bit;
      }

      /* Place the outputs in their assigned registers */
      for (RegList *r = outputs; r; r=r->next) {
         if (r->node->reg != r->reg) {
            if (sched->reg[r->reg].user != NULL) {
               move_result(sched, r->reg, MOV_EXCUSE_OUT_OF_THE_WAY);
            }
            move_to_register(sched, r->node->reg, r->reg, MOV_EXCUSE_OUTPUT);
         }
         /* The write is done one instruction before the data will be available */
         instr_count = gfx_smax(instr_count, sched->reg[r->reg].available);
      }

      if (branch_idx != -1) {
         RegList *r = outputs;
         for (int i=0; i<branch_idx; i++) r = r->next;
         flagify(sched, r->node, true);      /* Need this in flag a */
         instr_count = gfx_smax(instr_count, sched->reg[REG_FLAG_A].available);
      }
   }

   for (BackflowChainNode *n=iodeps->head; n; n=n->l.next)
      instr_count = gfx_umax(instr_count, n->ptr->io_timestamp);

   if (last)
      instr_count = fit_thrsw(sched, instr_count, true) + 1;

   /* ldvary has delayed effects that could affect the next block, so wait */
   if (instr_count > 0 && sched->instructions[instr_count-1].sigbits & V3D_QPU_SIG_LDVARY) instr_count++;

   assert(!last || !outputs);
   assert(instr_count <= MAX_INSTRUCTIONS);

#ifndef NDEBUG
   /* Ensure that all registers are free at the end, otherwise we've leaked */
   /* First clean up the outputs */
   for (RegList *r = outputs; r; r=r->next) {
      dereference(sched, r->node);
      if (instr_count > 0)
         clean_register(sched, r->node->reg, instr_count-1);
   }
   for (int i=0; i<countof(sched->reg); i++) assert(sched->reg[i].user == NULL);
#endif

#ifdef KHRN_SHADER_STATS
   print_shader_stats(sched, instr_count);
#endif

   gen = glsl_safemem_calloc(1, sizeof(GENERATED_SHADER_T));
   /* Fills in sched->gen with the actual code! */
   generate_all_instructions(sched->instructions, instr_count, gen);

   glsl_safemem_free(stack);
   glsl_safemem_free(sched);

   return gen;
}
