/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_qpu_instr.h"
#include "v3d_limits.h"

#include <inttypes.h>
#include <string.h>
#include "vcos_string.h"

const uint32_t v3d_qpu_small_imms[] = {
   // 0..15 -> 0..15
   0x00000000, 0x00000001, 0x00000002, 0x00000003,
   0x00000004, 0x00000005, 0x00000006, 0x00000007,
   0x00000008, 0x00000009, 0x0000000a, 0x0000000b,
   0x0000000c, 0x0000000d, 0x0000000e, 0x0000000f,
   // 16..31 -> -16..-1
   0xfffffff0, 0xfffffff1, 0xfffffff2, 0xfffffff3,
   0xfffffff4, 0xfffffff5, 0xfffffff6, 0xfffffff7,
   0xfffffff8, 0xfffffff9, 0xfffffffa, 0xfffffffb,
   0xfffffffc, 0xfffffffd, 0xfffffffe, 0xffffffff,
   // 32..39 -> Floating point 2^-8..2^-1
   0x3b800000, 0x3c000000, 0x3c800000, 0x3d000000,
   0x3d800000, 0x3e000000, 0x3e800000, 0x3f000000,
   // 40..47 -> Floating point 2^0..2^7
   0x3f800000, 0x40000000, 0x40800000, 0x41000000,
   0x41800000, 0x42000000, 0x42800000, 0x43000000,
   // 48..63 reserved
   };

v3d_qpu_magic_waddr_class_t v3d_qpu_classify_magic_waddr(uint32_t addr)
{
   switch (addr)
   {
   case V3D_QPU_MAGIC_WADDR_R0:
   case V3D_QPU_MAGIC_WADDR_R1:
   case V3D_QPU_MAGIC_WADDR_R2:
   case V3D_QPU_MAGIC_WADDR_R3:
   case V3D_QPU_MAGIC_WADDR_R4:
      return V3D_QPU_MAGIC_WADDR_CLASS_ACC;
   case V3D_QPU_MAGIC_WADDR_R5QUAD:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_QPU_MAGIC_WADDR_R5REP:
#endif
      return V3D_QPU_MAGIC_WADDR_CLASS_R5;
   case V3D_QPU_MAGIC_WADDR_NOP:
      return V3D_QPU_MAGIC_WADDR_CLASS_NOP;
   case V3D_QPU_MAGIC_WADDR_TLB:
   case V3D_QPU_MAGIC_WADDR_TLBU:
      return V3D_QPU_MAGIC_WADDR_CLASS_TLB;
   case V3D_QPU_MAGIC_WADDR_TMUD:
   case V3D_QPU_MAGIC_WADDR_TMUA:
   case V3D_QPU_MAGIC_WADDR_TMUAU:
#if V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_QPU_MAGIC_WADDR_TMUC:
   case V3D_QPU_MAGIC_WADDR_TMUS:
   case V3D_QPU_MAGIC_WADDR_TMUT:
   case V3D_QPU_MAGIC_WADDR_TMUR:
   case V3D_QPU_MAGIC_WADDR_TMUI:
   case V3D_QPU_MAGIC_WADDR_TMUB:
   case V3D_QPU_MAGIC_WADDR_TMUDREF:
   case V3D_QPU_MAGIC_WADDR_TMUOFF:
   case V3D_QPU_MAGIC_WADDR_TMUSCM:
   case V3D_QPU_MAGIC_WADDR_TMUSFETCH:
   case V3D_QPU_MAGIC_WADDR_TMUSLOD:
   case V3D_QPU_MAGIC_WADDR_TMUHS:
   case V3D_QPU_MAGIC_WADDR_TMUHSCM:
   case V3D_QPU_MAGIC_WADDR_TMUHSFETCH:
   case V3D_QPU_MAGIC_WADDR_TMUHSLOD:
#else
   case V3D_QPU_MAGIC_WADDR_TMU:
   case V3D_QPU_MAGIC_WADDR_TMUL:
#endif
      return V3D_QPU_MAGIC_WADDR_CLASS_TMU;
#if !V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_QPU_MAGIC_WADDR_VPM:
   case V3D_QPU_MAGIC_WADDR_VPMU:
      return V3D_QPU_MAGIC_WADDR_CLASS_VPM;
#endif
   case V3D_QPU_MAGIC_WADDR_SYNC:
   case V3D_QPU_MAGIC_WADDR_SYNCU:
#if V3D_VER_AT_LEAST(4,2,13,0)
   case V3D_QPU_MAGIC_WADDR_SYNCB:
#endif
      return V3D_QPU_MAGIC_WADDR_CLASS_SYNC;
   case V3D_QPU_MAGIC_WADDR_RECIP:
   case V3D_QPU_MAGIC_WADDR_RSQRT:
   case V3D_QPU_MAGIC_WADDR_EXP:
   case V3D_QPU_MAGIC_WADDR_LOG:
   case V3D_QPU_MAGIC_WADDR_SIN:
#if V3D_VER_AT_LEAST(3,3,0,0)
   case V3D_QPU_MAGIC_WADDR_RSQRT2:
#endif
      return V3D_QPU_MAGIC_WADDR_CLASS_SFU;
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_QPU_MAGIC_WADDR_UNIFA:
      return V3D_QPU_MAGIC_WADDR_CLASS_UNIF;
#endif
   default:
      unreachable();
      return V3D_QPU_MAGIC_WADDR_CLASS_INVALID;
   }
}

static uint64_t bits_at(bool* overflow, uint64_t bits, uint32_t pos, uint32_t nbits)
{
   assert(nbits + pos <= 64);
   if(bits > gfx_mask64(nbits)) *overflow = true;
   return bits << pos;
}

static uint32_t bits32_at(bool* overflow, uint32_t bits, uint32_t pos, uint32_t nbits)
{
   assert(nbits + pos <= 32);
   return (uint32_t)bits_at(overflow, bits, pos, nbits);
}

static uint32_t bits32_repl(bool* overflow, uint32_t curbits, uint32_t bits, uint32_t pos, uint32_t nbits)
{
   assert(nbits + pos <= 32);
   return (curbits & ~(gfx_mask(nbits) << pos)) | (uint32_t)bits_at(overflow, bits, pos, nbits);
}

const static v3d_qpu_sigbits_t result_write_mask =
                            V3D_QPU_SIG_LDTLB  | V3D_QPU_SIG_LDTLBU |
                            V3D_QPU_SIG_LDVARY | V3D_QPU_SIG_LDTMU
#if V3D_VER_AT_LEAST(4,1,34,0)
                          | V3D_QPU_SIG_LDUNIFRF | V3D_QPU_SIG_LDUNIFARF
#endif
                           ;

bool v3d_qpu_sig_has_result_write(v3d_qpu_sigbits_t sigbits) {
   return (sigbits & result_write_mask) != 0;
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
/* Provide default registers for those signals that can use registers but have defaults */
uint32_t v3d_qpu_sig_default_reg(v3d_qpu_sigbits_t sigbits) {
   uint32_t bit = sigbits & result_write_mask;

   switch (bit) {
      case V3D_QPU_SIG_LDVARY: return 3;
      case V3D_QPU_SIG_LDTLB:  return 3;
      case V3D_QPU_SIG_LDTLBU: return 3;
      case V3D_QPU_SIG_LDTMU:  return 4;
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_QPU_SIG_LDUNIFRF:  return 5;
      case V3D_QPU_SIG_LDUNIFARF: return 5;
#endif
      default: unreachable();  return 0;
   }
}
#endif

uint64_t v3d_qpu_instr_unused_mask(const struct v3d_qpu_instr *in)
{
   switch(in->type)
   {
      case V3D_QPU_INSTR_TYPE_ALU:
         if(in->u.alu.sig.sigbits & V3D_QPU_SIG_ROTATE)
         {
            return gfx_mask64(2) << 4;
         }
#if V3D_VER_AT_LEAST(4,0,2,0) && !V3D_VER_AT_LEAST(4,1,34,0)
         else if (v3d_qpu_sig_has_result_write(in->u.alu.sig.sigbits) && !in->u.alu.sig.sig_reg)
         {
            return gfx_mask64(6) << 46;
         }
#endif
         else
         {
            return 0;
         }

      case V3D_QPU_INSTR_TYPE_BRANCH:
         return (gfx_mask64(4) << 17) | gfx_mask64(6);

      case V3D_QPU_INSTR_TYPE_LDI:
         return gfx_mask64(1) << 53;

      case V3D_QPU_INSTR_TYPE_BKPT:
         return (gfx_mask64(19) << 36) | gfx_mask64(32);

      case V3D_QPU_INSTR_TYPE_DEBUG:
         return gfx_mask64(19) << 13;

      default: unreachable();
   }
}

uint32_t v3d_qpu_op_num_inputs(v3d_qpu_opcode_t opcode)
{
   uint32_t num_inputs = opcode >> 8;
   assert(num_inputs <= 2);
   return num_inputs;
}

bool v3d_qpu_op_is_commutative(v3d_qpu_opcode_t opcode)
{
   assert(v3d_qpu_op_num_inputs(opcode) == 2);
   switch (opcode)
   {
   case V3D_QPU_OP_FADD:
   case V3D_QPU_OP_FADDNF:
   case V3D_QPU_OP_FMIN:
   case V3D_QPU_OP_FMAX:
   case V3D_QPU_OP_ADD:
   case V3D_QPU_OP_MIN:
   case V3D_QPU_OP_MAX:
   case V3D_QPU_OP_UMIN:
   case V3D_QPU_OP_UMAX:
   case V3D_QPU_OP_VFMIN:
   case V3D_QPU_OP_AND:
   case V3D_QPU_OP_OR:
   case V3D_QPU_OP_XOR:
   case V3D_QPU_OP_VADD:
   case V3D_QPU_OP_VFMAX:
   case V3D_QPU_OP_FMUL:
   case V3D_QPU_OP_UMUL24:
   case V3D_QPU_OP_VFMUL:
   case V3D_QPU_OP_SMUL24:
   case V3D_QPU_OP_MULTOP:
      return true;
   case V3D_QPU_OP_FSUB:
   case V3D_QPU_OP_FCMP:
   case V3D_QPU_OP_VFPACK:
   case V3D_QPU_OP_SUB:
   case V3D_QPU_OP_SHL:
   case V3D_QPU_OP_SHR:
   case V3D_QPU_OP_ASR:
   case V3D_QPU_OP_ROR:
   case V3D_QPU_OP_VSUB:
   case V3D_QPU_OP_STVPMV:
   case V3D_QPU_OP_STVPMD:
   case V3D_QPU_OP_STVPMP:
   case V3D_QPU_OP_LDVPMG_IN:
   case V3D_QPU_OP_LDVPMG_OUT:
      return false;
   default:
      unreachable();
      return false;
   }
}

bool v3d_qpu_op_is_zero_when_inputs_match(v3d_qpu_opcode_t opcode)
{
   assert(v3d_qpu_op_num_inputs(opcode) == 2);
   switch (opcode)
   {
   case V3D_QPU_OP_SUB:
   case V3D_QPU_OP_VSUB:
   case V3D_QPU_OP_XOR:
      return true;
   default:
      return false;
   }
}

bool v3d_qpu_op_is_mov_when_inputs_match(v3d_qpu_opcode_t opcode)
{
   assert(v3d_qpu_op_num_inputs(opcode) == 2);
   switch (opcode)
   {
   case V3D_QPU_OP_MIN:
   case V3D_QPU_OP_MAX:
   case V3D_QPU_OP_UMIN:
   case V3D_QPU_OP_UMAX:
   case V3D_QPU_OP_AND:
   case V3D_QPU_OP_OR:
      return true;
   default:
      return false;
   }
}

static void decode_f16_unpack(v3d_qpu_unpack_t *a, v3d_qpu_unpack_t *b, uint32_t bits)
{
   *a = V3D_QPU_UNPACK_NONE;
   *b = V3D_QPU_UNPACK_NONE;
   switch (bits)
   {
   case 0:  break;
   case 1:  *a = V3D_QPU_UNPACK_32F_TO_16F_REPLICATE; break;
   case 2:  *a = V3D_QPU_UNPACK_REPLICATE_LOW_16; break;
   case 3:  *a = V3D_QPU_UNPACK_REPLICATE_HIGH_16; break;
   case 4:  *a = V3D_QPU_UNPACK_SWAP_16; break;
   case 5:  *b = V3D_QPU_UNPACK_32F_TO_16F_REPLICATE; break;
   case 6:  *b = V3D_QPU_UNPACK_REPLICATE_LOW_16; break;
   case 7:  *b = V3D_QPU_UNPACK_REPLICATE_HIGH_16; break;
   default: unreachable();
   }
}

static bool encode_f16_unpack(v3d_qpu_unpack_t a, v3d_qpu_unpack_t b, uint32_t* bits)
{
   if(a != V3D_QPU_UNPACK_NONE && b != V3D_QPU_UNPACK_NONE)
   {
      return false;
   }
   if(a == V3D_QPU_UNPACK_NONE && b == V3D_QPU_UNPACK_NONE)
   {
      *bits = 0;
      return true;
   }
   switch(a)
   {
      case V3D_QPU_UNPACK_32F_TO_16F_REPLICATE: *bits = 1; return true;
      case V3D_QPU_UNPACK_REPLICATE_LOW_16: *bits = 2; return true;
      case V3D_QPU_UNPACK_REPLICATE_HIGH_16: *bits = 3; return true;
      case V3D_QPU_UNPACK_SWAP_16: *bits = 4; return true;
      case V3D_QPU_UNPACK_NONE: break; /* fallthrough to b cases */
      default: return false;
   }
   switch(b)
   {
      case V3D_QPU_UNPACK_32F_TO_16F_REPLICATE: *bits = 5; return true;
      case V3D_QPU_UNPACK_REPLICATE_LOW_16: *bits = 6; return true;
      case V3D_QPU_UNPACK_REPLICATE_HIGH_16: *bits = 7; return true;
      default: return false;
   }
}

static bool decode_add_op(struct v3d_qpu_op *op, uint32_t op_add, uint32_t add_a,
   uint32_t add_b, uint32_t waddr_a, uint32_t magic_add)
{
   op->a.source = (v3d_qpu_in_source_t)add_a;
   op->b.source = (v3d_qpu_in_source_t)add_b;

   bool swapped = ((((op_add >> 2) & 3) * 8) + add_a) > (((op_add & 3) * 8) + add_b);
   if (op_add <= 47)
      op->opcode = swapped ? V3D_QPU_OP_FADDNF : V3D_QPU_OP_FADD;
   else if ((op_add >= 64) && (op_add <= 111))
      op->opcode = V3D_QPU_OP_FSUB;
   else if ((op_add >= 128) && (op_add <= 175))
      op->opcode = swapped ? V3D_QPU_OP_FMAX : V3D_QPU_OP_FMIN;
   else if ((op_add >= 192) && (op_add <= 239))
      op->opcode = V3D_QPU_OP_FCMP;
   else switch (op_add)
   {
   case 53: case 54: case 55:
   case 57: case 58: case 59:
   case 61: case 62: case 63:
               op->opcode = V3D_QPU_OP_VFPACK; break;
   case 56:    op->opcode = V3D_QPU_OP_ADD; break;
   case 60:    op->opcode = V3D_QPU_OP_SUB; break;
   case 120:   op->opcode = V3D_QPU_OP_MIN; break;
   case 121:   op->opcode = V3D_QPU_OP_MAX; break;
   case 122:   op->opcode = V3D_QPU_OP_UMIN; break;
   case 123:   op->opcode = V3D_QPU_OP_UMAX; break;
   case 124:   op->opcode = V3D_QPU_OP_SHL; break;
   case 125:   op->opcode = V3D_QPU_OP_SHR; break;
   case 126:   op->opcode = V3D_QPU_OP_ASR; break;
   case 127:   op->opcode = V3D_QPU_OP_ROR; break;
   case 176: case 177: case 178: case 179: case 180:
               op->opcode = V3D_QPU_OP_VFMIN; break;
   case 181:   op->opcode = V3D_QPU_OP_AND; break;
   case 182:   op->opcode = V3D_QPU_OP_OR; break;
   case 183:   op->opcode = V3D_QPU_OP_XOR; break;
   case 184:   op->opcode = V3D_QPU_OP_VADD; break;
   case 185:   op->opcode = V3D_QPU_OP_VSUB; break;
   case 186:
      switch (add_b)
      {
      case 0:  op->opcode = V3D_QPU_OP_NOT; break;
      case 1:  op->opcode = V3D_QPU_OP_NEG; break;
      case 2:  op->opcode = V3D_QPU_OP_FLAPUSH; break;
      case 3:  op->opcode = V3D_QPU_OP_FLBPUSH; break;
      case 4:  op->opcode = V3D_QPU_OP_FLPOP; break;
      case 6:  op->opcode = V3D_QPU_OP_SETMSF; break;
      case 7:  op->opcode = V3D_QPU_OP_SETREVF; break;
      default: return false;
      }
      break;
   case 187:
      switch (add_b)
      {
      case 0:
         switch (add_a)
         {
         case 0:  op->opcode = V3D_QPU_OP_NOP; break;
         case 1:  op->opcode = V3D_QPU_OP_TIDX; break;
         case 2:  op->opcode = V3D_QPU_OP_EIDX; break;
         case 3:  op->opcode = V3D_QPU_OP_LR; break;
         case 4:  op->opcode = V3D_QPU_OP_VFLA; break;
         case 5:  op->opcode = V3D_QPU_OP_VFLNA; break;
         case 6:  op->opcode = V3D_QPU_OP_VFLB; break;
         case 7:  op->opcode = V3D_QPU_OP_VFLNB; break;
         default: return false;
         }
         break;
      case 1:
         switch (add_a)
         {
         case 0: case 1: case 2: op->opcode = V3D_QPU_OP_FXCD; break;
         case 3:                 op->opcode = V3D_QPU_OP_XCD; break;
         case 4: case 5: case 6: op->opcode = V3D_QPU_OP_FYCD; break;
         case 7:                 op->opcode = V3D_QPU_OP_YCD; break;
         default:                return false;
         }
         break;
      case 2:
         switch (add_a)
         {
         case 0:  op->opcode = V3D_QPU_OP_MSF; break;
         case 1:  op->opcode = V3D_QPU_OP_REVF; break;
#if V3D_VER_AT_LEAST(4,0,2,0)
         case 2:  op->opcode = V3D_QPU_OP_IID; break;
         case 3:  op->opcode = V3D_QPU_OP_SAMPID; break;
         case 4:  op->opcode = V3D_QPU_OP_BARRIERID; break;
#endif
         case 5:  op->opcode = V3D_QPU_OP_TMUWT; break;
#if V3D_VER_AT_LEAST(4,0,2,0)
         case 6:  op->opcode = V3D_QPU_OP_VPMWT; break;
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
         case 7: op->opcode = V3D_QPU_OP_FLAFIRST; break;
#endif
         default: return false;
         }
         break;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case 3:
         op->opcode = V3D_QPU_OP_VPMSETUP;
         break;
#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
      case 4: op->opcode = V3D_QPU_OP_TMUWRCFG; break;
      case 5: op->opcode = V3D_QPU_OP_TLBWRCFG; break;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case 6: op->opcode = V3D_QPU_OP_VPMWRCFG; break;
#endif
#endif
      default:
         return false;
      }
      break;
#if V3D_VER_AT_LEAST(4,0,2,0)
   case 188:
      switch(add_b)
      {
         case 0: op->opcode = magic_add? V3D_QPU_OP_LDVPMV_OUT : V3D_QPU_OP_LDVPMV_IN; break;
         case 1: op->opcode = magic_add? V3D_QPU_OP_LDVPMD_OUT : V3D_QPU_OP_LDVPMD_IN; break;
         case 2:
            /* No in/out variants for ldvpmp as there is only one shared seg */
            if(magic_add) return false;
            op->opcode = V3D_QPU_OP_LDVPMP;
            break;
         default: return false;
      }
      break;
   case 189:
      if(magic_add)
      {
         op->opcode = V3D_QPU_OP_LDVPMG_OUT;
      }
      else
      {
         op->opcode = V3D_QPU_OP_LDVPMG_IN;
      }
      break;
#endif
   case 190: case 191:
      if(add_b >= 8) return false;
      op->opcode = V3D_QPU_OP_VFMOV + (((op_add & 1) << 3) | add_b);
      break;
   case 240: case 241: case 242: case 243: case 244:
      op->opcode = V3D_QPU_OP_VFMAX;
      break;
   case 245: case 249: case 253:
      switch (add_b)
      {
      case 0: case 1: case 2: op->opcode = V3D_QPU_OP_FROUND; break;
      case 3:                 op->opcode = V3D_QPU_OP_FTOIN; break;
      case 4: case 5: case 6: op->opcode = V3D_QPU_OP_FTRUNC; break;
      case 7:                 op->opcode = V3D_QPU_OP_FTOIZ; break;
      default:                return false;
      }
      break;
   case 246: case 250: case 254:
      switch (add_b)
      {
      case 0: case 1: case 2: op->opcode = V3D_QPU_OP_FFLOOR; break;
      case 3:                 op->opcode = V3D_QPU_OP_FTOUZ; break;
      case 4: case 5: case 6: op->opcode = V3D_QPU_OP_FCEIL; break;
      case 7:                 op->opcode = V3D_QPU_OP_FTOC; break;
      default:                return false;
      }
      break;
   case 247: case 251: case 255:
      switch (add_b)
      {
      case 0: case 1: case 2: op->opcode = V3D_QPU_OP_FDX; break;
      case 4: case 5: case 6: op->opcode = V3D_QPU_OP_FDY; break;
      default:                return false;
      }
      break;
#if V3D_VER_AT_LEAST(4,0,2,0)
   case 248:
      /* We don't allow the ma flag to be set in stvpm instructions. */
      if(magic_add) return false;

      switch(waddr_a)
      {
         case 0: op->opcode = V3D_QPU_OP_STVPMV; break;
         case 1: op->opcode = V3D_QPU_OP_STVPMD; break;
         case 2: op->opcode = V3D_QPU_OP_STVPMP; break;
         default: return false;
      }
      break;
#endif
   case 252:
      switch (add_b)
      {
      case 0: case 1: case 2: op->opcode = V3D_QPU_OP_ITOF; break;
      case 3:                 op->opcode = V3D_QPU_OP_CLZ; break;
      case 4: case 5: case 6: op->opcode = V3D_QPU_OP_UTOF; break;
      default:                return false;
      }
      break;
   default:
      return false;
   }

   op->pack = V3D_QPU_PACK_NONE;
   op->a.unpack = V3D_QPU_UNPACK_NONE;
   op->b.unpack = V3D_QPU_UNPACK_NONE;
   switch (op->opcode)
   {
   case V3D_QPU_OP_FADD:
   case V3D_QPU_OP_FADDNF:
   case V3D_QPU_OP_FSUB:
   case V3D_QPU_OP_FMIN:
   case V3D_QPU_OP_FMAX:
   case V3D_QPU_OP_FCMP:
      op->pack = (v3d_qpu_pack_t)((op_add >> 4) & 3);
      /* Fall through */
   case V3D_QPU_OP_VFPACK:
      op->a.unpack = (v3d_qpu_unpack_t)((op_add >> 2) & 3);
      op->b.unpack = (v3d_qpu_unpack_t)(op_add & 3);
      break;
   case V3D_QPU_OP_FROUND:
   case V3D_QPU_OP_FTRUNC:
   case V3D_QPU_OP_FFLOOR:
   case V3D_QPU_OP_FCEIL:
   case V3D_QPU_OP_FDX:
   case V3D_QPU_OP_FDY:
      op->a.unpack = (v3d_qpu_unpack_t)((op_add >> 2) & 3);
      /* Fall through */
   case V3D_QPU_OP_ITOF:
   case V3D_QPU_OP_UTOF:
      op->pack = (v3d_qpu_pack_t)(add_b & 3);
      break;
   case V3D_QPU_OP_FXCD:
   case V3D_QPU_OP_FYCD:
      op->pack = (v3d_qpu_pack_t)(add_a & 3);
      break;
   case V3D_QPU_OP_FTOIN:
   case V3D_QPU_OP_FTOIZ:
   case V3D_QPU_OP_FTOUZ:
   case V3D_QPU_OP_FTOC:
      op->a.unpack = (v3d_qpu_unpack_t)((op_add >> 2) & 3);
      break;
   case V3D_QPU_OP_VFMIN:
   case V3D_QPU_OP_VFMAX:
      decode_f16_unpack(&op->a.unpack, &op->b.unpack, op_add & 7);
      break;
   default:
      break;
   }

   return true;
}

static bool encode_add_op_with_pack(struct v3d_qpu_op const* op, uint32_t* op_add,
   uint32_t* add_a, uint32_t* add_b)
{
   if(!v3d_is_valid_qpu_pack(op->pack)) return false;

   bool swap_ab = false;
   /* Set the base bit patterns for the supported instructions */
   bool const agtb = ((op->a.unpack * 8) + op->a.source) > ((op->b.unpack * 8) + op->b.source);
   switch(op->opcode)
   {
      /* fadd/faddnf and fmin/fmax are distinguised based on the ordering of
       * their operands. As the operations are commutative we can swap the
       * operand ordering as required to perform the requested operation */
      case V3D_QPU_OP_FADD: *op_add = 0; swap_ab = agtb; break;
      case V3D_QPU_OP_FADDNF: *op_add = 0; swap_ab = !agtb; break;
      case V3D_QPU_OP_FMIN: *op_add = 128; swap_ab = agtb; break;
      case V3D_QPU_OP_FMAX: *op_add = 128; swap_ab = !agtb; break;

      case V3D_QPU_OP_VFPACK: *op_add = 48; break;
      case V3D_QPU_OP_FSUB: *op_add = 64; break;
      case V3D_QPU_OP_VFMIN: *op_add = 176; break;
      case V3D_QPU_OP_VFMAX: *op_add = 240; break;
      case V3D_QPU_OP_FCMP: *op_add = 192; break;

      case V3D_QPU_OP_FROUND: *op_add = 245; *add_b = 0; break;
      case V3D_QPU_OP_FTOIN: *op_add = 245; *add_b = 3; break;
      case V3D_QPU_OP_FTRUNC: *op_add = 245; *add_b = 4; break;
      case V3D_QPU_OP_FTOIZ: *op_add = 245; *add_b = 7; break;

      case V3D_QPU_OP_FFLOOR: *op_add = 246; *add_b = 0; break;
      case V3D_QPU_OP_FTOUZ: *op_add = 246; *add_b = 3; break;
      case V3D_QPU_OP_FCEIL: *op_add = 246; *add_b = 4; break;
      case V3D_QPU_OP_FTOC: *op_add = 246; *add_b = 7; break;

      case V3D_QPU_OP_FDX: *op_add = 247; *add_b = 0; break;
      case V3D_QPU_OP_FDY: *op_add = 247; *add_b = 4; break;
      case V3D_QPU_OP_ITOF: *op_add = 252; *add_b = 0; break;
      case V3D_QPU_OP_UTOF: *op_add = 252; *add_b = 4; break;

      case V3D_QPU_OP_FXCD: *op_add = 187; *add_b = 1; *add_a = 0; break;
      case V3D_QPU_OP_FYCD: *op_add = 187; *add_b = 1; *add_a = 4; break;

      default: return false;
   }

   /* set the coding bits for pack/unpack settings */
   bool ov = false;
   switch(op->opcode)
   {
      case V3D_QPU_OP_FADD:
      case V3D_QPU_OP_FADDNF:
      case V3D_QPU_OP_FSUB:
      case V3D_QPU_OP_FMIN:
      case V3D_QPU_OP_FMAX:
      case V3D_QPU_OP_FCMP:
         *op_add = bits32_repl(&ov, *op_add, op->pack, 4, 2);
         if(swap_ab)
         {
            *add_a = op->b.source;
            *add_b = op->a.source;
            *op_add = bits32_repl(&ov, *op_add, op->a.unpack, 0, 2);
            *op_add = bits32_repl(&ov, *op_add, op->b.unpack, 2, 2);
         }
         else
         {
            *op_add = bits32_repl(&ov, *op_add, op->b.unpack, 0, 2);
            *op_add = bits32_repl(&ov, *op_add, op->a.unpack, 2, 2);
         }
         return !ov;

      case V3D_QPU_OP_VFPACK:
         if(op->pack != V3D_QPU_PACK_NONE) return false;
         if(op->a.unpack == 0) return false;
         if(op->b.unpack == 0) return false;
         *op_add = bits32_repl(&ov, *op_add, op->a.unpack, 2, 2);
         *op_add = bits32_repl(&ov, *op_add, op->b.unpack, 0, 2);
         return !ov;

      case V3D_QPU_OP_FROUND:
      case V3D_QPU_OP_FTRUNC:
      case V3D_QPU_OP_FFLOOR:
      case V3D_QPU_OP_FCEIL:
      case V3D_QPU_OP_FDX:
      case V3D_QPU_OP_FDY:
         if(op->a.unpack == 0) return false;
         *op_add = bits32_repl(&ov, *op_add, op->a.unpack, 2, 2);
         *add_b = bits32_repl(&ov, *add_b, op->pack, 0, 2);
         return !ov;

      case V3D_QPU_OP_ITOF:
      case V3D_QPU_OP_UTOF:
         if(op->a.unpack != V3D_QPU_UNPACK_NONE) return false;
         *add_b = bits32_repl(&ov, *add_b, op->pack, 0, 2);
         return !ov;

      case V3D_QPU_OP_FXCD:
      case V3D_QPU_OP_FYCD:
         if(op->a.unpack != V3D_QPU_UNPACK_NONE) return false;
         *add_a = bits32_repl(&ov, *add_a, op->pack, 0, 2);
         return !ov;

      case V3D_QPU_OP_FTOIN:
      case V3D_QPU_OP_FTOIZ:
      case V3D_QPU_OP_FTOUZ:
      case V3D_QPU_OP_FTOC:
         if(op->pack != V3D_QPU_PACK_NONE) return false;
         if(op->a.unpack == 0) return false;
         *op_add = bits32_repl(&ov, *op_add, op->a.unpack, 2, 2);
         return !ov;

      case V3D_QPU_OP_VFMIN:
      case V3D_QPU_OP_VFMAX:
      {
         if(op->pack != V3D_QPU_PACK_NONE) return false;
         uint32_t encoded;
         if(!encode_f16_unpack(op->a.unpack, op->b.unpack, &encoded)
            || encoded > 4)
         {
            return false;
         }
         *op_add = bits32_repl(&ov, *op_add, encoded, 0, 3);
         return !ov;
      }

      default:
      unreachable();
   }
}

static bool encode_add_op(struct v3d_qpu_op const* op, uint32_t* op_add, uint32_t* add_a,
   uint32_t* add_b, uint32_t* waddr_a, bool* magic_a)
{
   if(encode_add_op_with_pack(op, op_add, add_a, add_b))
   {
      return true;
   }

   /* remaining instructions do not support pack/unpack */
   if(op->a.unpack != V3D_QPU_UNPACK_NONE || op->b.unpack != V3D_QPU_UNPACK_NONE
      || op->pack != V3D_QPU_PACK_NONE)
   {
      return false;
   }

   switch(op->opcode)
   {
      case V3D_QPU_OP_ADD: *op_add = 56; return true;
      case V3D_QPU_OP_SUB: *op_add = 60; return true;
      case V3D_QPU_OP_MIN: *op_add = 120; return true;
      case V3D_QPU_OP_MAX: *op_add = 121; return true;
      case V3D_QPU_OP_UMIN: *op_add = 122; return true;
      case V3D_QPU_OP_UMAX: *op_add = 123; return true;
      case V3D_QPU_OP_SHL: *op_add = 124; return true;
      case V3D_QPU_OP_SHR: *op_add = 125; return true;
      case V3D_QPU_OP_ASR: *op_add = 126; return true;
      case V3D_QPU_OP_ROR: *op_add = 127; return true;
      case V3D_QPU_OP_AND: *op_add = 181; return true;
      case V3D_QPU_OP_OR: *op_add = 182; return true;
      case V3D_QPU_OP_XOR: *op_add = 183; return true;
      case V3D_QPU_OP_VADD: *op_add = 184; return true;
      case V3D_QPU_OP_VSUB: *op_add = 185; return true;

      case V3D_QPU_OP_NOT: *op_add = 186; *add_b = 0; return true;
      case V3D_QPU_OP_NEG: *op_add = 186; *add_b = 1; return true;
      case V3D_QPU_OP_FLAPUSH: *op_add = 186; *add_b = 2; return true;
      case V3D_QPU_OP_FLBPUSH: *op_add = 186; *add_b = 3; return true;
      case V3D_QPU_OP_FLPOP: *op_add = 186; *add_b = 4; return true;
      case V3D_QPU_OP_SETMSF: *op_add = 186; *add_b = 6; return true;
      case V3D_QPU_OP_SETREVF: *op_add = 186; *add_b = 7; return true;

      case V3D_QPU_OP_NOP: *op_add = 187; *add_b = 0; *add_a = 0; return true;
      case V3D_QPU_OP_TIDX: *op_add = 187; *add_b = 0; *add_a = 1; return true;
      case V3D_QPU_OP_EIDX: *op_add = 187; *add_b = 0; *add_a = 2; return true;
      case V3D_QPU_OP_LR: *op_add = 187; *add_b = 0; *add_a = 3; return true;
      case V3D_QPU_OP_VFLA: *op_add = 187; *add_b = 0; *add_a = 4; return true;
      case V3D_QPU_OP_VFLNA: *op_add = 187; *add_b = 0; *add_a = 5; return true;
      case V3D_QPU_OP_VFLB: *op_add = 187; *add_b = 0; *add_a = 6; return true;
      case V3D_QPU_OP_VFLNB: *op_add = 187; *add_b = 0; *add_a = 7; return true;

      case V3D_QPU_OP_XCD: *op_add = 187; *add_b = 1; *add_a = 3; return true;
      case V3D_QPU_OP_YCD: *op_add = 187; *add_b = 1; *add_a = 7; return true;

      case V3D_QPU_OP_MSF: *op_add = 187; *add_b = 2; *add_a = 0; return true;
      case V3D_QPU_OP_REVF: *op_add = 187; *add_b = 2; *add_a = 1; return true;
#if V3D_VER_AT_LEAST(4,0,2,0)
      case V3D_QPU_OP_IID: *op_add = 187; *add_b = 2; *add_a = 2; return true;
      case V3D_QPU_OP_SAMPID: *op_add = 187; *add_b = 2; *add_a = 3; return true;
#endif
      case V3D_QPU_OP_TMUWT: *op_add = 187; *add_b = 2; *add_a = 5; return true;

#if V3D_VER_AT_LEAST(3,3,0,0)
      case V3D_QPU_OP_TMUWRCFG: *op_add = 187; *add_b = 4; return true;
      case V3D_QPU_OP_TLBWRCFG: *op_add = 187; *add_b = 5; return true;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case V3D_QPU_OP_VPMWRCFG: *op_add = 187; *add_b = 6; return true;
#endif
#endif

      case V3D_QPU_OP_VFMOV: *op_add = 190; *add_b = 0; return true;
      case V3D_QPU_OP_VFMOVABS: *op_add = 190; *add_b = 1; return true;
      case V3D_QPU_OP_VFMOVNEG: *op_add = 190; *add_b = 2; return true;
      case V3D_QPU_OP_VFMOVNAB: *op_add = 190; *add_b = 3; return true;
      case V3D_QPU_OP_VFABSMOV: *op_add = 190; *add_b = 4; return true;
      case V3D_QPU_OP_VFABS: *op_add = 190; *add_b = 5; return true;
      case V3D_QPU_OP_VFABSNEG: *op_add = 190; *add_b = 6; return true;
      case V3D_QPU_OP_VFABSNAB: *op_add = 190; *add_b = 7; return true;

      case V3D_QPU_OP_VFNEGMOV: *op_add = 191; *add_b = 0; return true;
      case V3D_QPU_OP_VFNEGABS: *op_add = 191; *add_b = 1; return true;
      case V3D_QPU_OP_VFNEG: *op_add = 191; *add_b = 2; return true;
      case V3D_QPU_OP_VFNEGNAB: *op_add = 191; *add_b = 3; return true;
      case V3D_QPU_OP_VFNABMOV: *op_add = 191; *add_b = 4; return true;
      case V3D_QPU_OP_VFNABABS: *op_add = 191; *add_b = 5; return true;
      case V3D_QPU_OP_VFNABNEG: *op_add = 191; *add_b = 6; return true;
      case V3D_QPU_OP_VFNABS: *op_add = 191; *add_b = 7; return true;

      case V3D_QPU_OP_CLZ: *op_add = 252; *add_b = 3; return true;

#if V3D_VER_AT_LEAST(4,0,2,0)
      /* Note: ldvpm* can only load to register file */
      case V3D_QPU_OP_LDVPMV_IN: *op_add = 188; *add_b = 0; *magic_a = false; return true;
      case V3D_QPU_OP_LDVPMD_IN: *op_add = 188; *add_b = 1; *magic_a = false; return true;
      case V3D_QPU_OP_LDVPMV_OUT: *op_add = 188; *add_b = 0; *magic_a = true; return true;
      case V3D_QPU_OP_LDVPMD_OUT: *op_add = 188; *add_b = 1; *magic_a = true; return true;
      case V3D_QPU_OP_LDVPMP: *op_add = 188; *add_b = 2; return !*magic_a;
      case V3D_QPU_OP_LDVPMG_IN: *op_add = 189; *magic_a = false; return true;
      case V3D_QPU_OP_LDVPMG_OUT: *op_add = 189; *magic_a = true; return true;
      case V3D_QPU_OP_STVPMV: *op_add = 248; *magic_a = false; *waddr_a = 0; return true;
      case V3D_QPU_OP_STVPMD: *op_add = 248; *magic_a = false; *waddr_a = 1; return true;
      case V3D_QPU_OP_STVPMP: *op_add = 248; *magic_a = false; *waddr_a = 2; return true;
      case V3D_QPU_OP_BARRIERID: *op_add = 187; *add_b = 2; *add_a = 4; return true;
      case V3D_QPU_OP_VPMWT: *op_add = 187; *add_b = 2; *add_a = 6; return true;
#else
      case V3D_QPU_OP_VPMSETUP: *op_add = 187; *add_b = 3; return true;
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
      case V3D_QPU_OP_FLAFIRST: *op_add = 187; *add_b = 2; *add_a = 7; return true;
#endif

      default: return false;
   }
}

static bool decode_mul_op(struct v3d_qpu_op *op, uint32_t op_mul, uint32_t mul_a, uint32_t mul_b)
{
   op->a.source = (v3d_qpu_in_source_t)mul_a;
   op->b.source = (v3d_qpu_in_source_t)mul_b;

   if ((op_mul >= 16) && (op_mul <= 63))
      op->opcode = V3D_QPU_OP_FMUL;
   else switch (op_mul)
   {
   case 1:  op->opcode = V3D_QPU_OP_ADD; break;
   case 2:  op->opcode = V3D_QPU_OP_SUB; break;
   case 3:  op->opcode = V3D_QPU_OP_UMUL24; break;
   case 4: case 5: case 6: case 7: case 8:
            op->opcode = V3D_QPU_OP_VFMUL; break;
   case 9:  op->opcode = V3D_QPU_OP_SMUL24; break;
   case 10: op->opcode = V3D_QPU_OP_MULTOP; break;
   case 14: op->opcode = V3D_QPU_OP_FMOV; break;
   case 15:
      switch (mul_b)
      {
      case 0: case 1: case 2: case 3:
         op->opcode = V3D_QPU_OP_FMOV;
         break;
      case 4:
         switch (mul_a)
         {
         case 0:  op->opcode = V3D_QPU_OP_NOP; break;
         default: return false;
         }
         break;
      case 7:
         op->opcode = V3D_QPU_OP_MOV;
         break;
      default:
         return false;
      }
      break;
   default:
      return false;
   }

   op->pack = V3D_QPU_PACK_NONE;
   op->a.unpack = V3D_QPU_UNPACK_NONE;
   op->b.unpack = V3D_QPU_UNPACK_NONE;
   switch (op->opcode)
   {
   case V3D_QPU_OP_FMUL:
      op->pack = (v3d_qpu_pack_t)(((op_mul >> 4) & 3) - 1);
      op->a.unpack = (v3d_qpu_unpack_t)((op_mul >> 2) & 3);
      op->b.unpack = (v3d_qpu_unpack_t)(op_mul & 3);
      break;
   case V3D_QPU_OP_FMOV:
      op->pack = (v3d_qpu_pack_t)(((op_mul & 1) * 2) + ((mul_b >> 2) & 1));
      op->a.unpack = (v3d_qpu_unpack_t)(mul_b & 3);
      break;
   case V3D_QPU_OP_VFMUL:
      decode_f16_unpack(&op->a.unpack, &op->b.unpack, (op_mul - 4) & 7);
      break;
   default:
      break;
   }

   return true;
}

static bool encode_mul_op(struct v3d_qpu_op const* op, uint32_t* op_mul,
   uint32_t* mul_a, uint32_t* mul_b)
{
   if(!v3d_is_valid_qpu_pack(op->pack)) return false;

   bool ov = false;
   switch (op->opcode)
   {
   case V3D_QPU_OP_FMUL:
      *op_mul = bits32_at(&ov, op->pack + 1, 4, 2)
         | bits32_at(&ov, op->a.unpack, 2, 2)
         | bits32_at(&ov, op->b.unpack, 0, 2);
      return !ov;

   case V3D_QPU_OP_FMOV:
      *op_mul = 14 + ((op->pack & 2)? 1 : 0);
      *mul_b = bits32_at(&ov, op->pack & 1, 2, 1)
         | bits32_at(&ov, op->a.unpack, 0, 2);
      return !ov;

   case V3D_QPU_OP_VFMUL:
      {
         if(op->pack != V3D_QPU_PACK_NONE) return false;
         uint32_t encoded;
         if(!encode_f16_unpack(op->a.unpack, op->b.unpack, &encoded)
            || encoded > 4)
         {
            return false;
         }
         *op_mul = 4 + encoded;
         return true;
      }
      break;
   default:
      break;
   }

   /* remaining instructions do not support pack/unpack */
   if(op->a.unpack != V3D_QPU_UNPACK_NONE || op->b.unpack != V3D_QPU_UNPACK_NONE
      || op->pack != V3D_QPU_PACK_NONE)
   {
      return false;
   }

   switch(op->opcode)
   {
   case V3D_QPU_OP_ADD: *op_mul = 1; return true;
   case V3D_QPU_OP_SUB: *op_mul = 2; return true;
   case V3D_QPU_OP_UMUL24: *op_mul = 3; return true;
   case V3D_QPU_OP_SMUL24: *op_mul = 9; return true;
   case V3D_QPU_OP_MULTOP: *op_mul = 10; return true;
   case V3D_QPU_OP_MOV: *op_mul = 15; *mul_b = 7; return true;
   case V3D_QPU_OP_NOP: *op_mul = 15; *mul_b = 4; *mul_a = 0; return true;

   default: return false;
   }
}

static const v3d_qpu_sigbits_t valid_sigs[] = {
#define S(SIG) (V3D_QPU_SIG_##SIG)
/*  0 */   0,
/*  1 */   S(THRSW),
/*  2 */                                             S(LDUNIF),
/*  3 */   S(THRSW) |                                S(LDUNIF),
/*  4 */                                S(LDTMU),
/*  5 */   S(THRSW) |                   S(LDTMU),
/*  6 */                                S(LDTMU) |   S(LDUNIF),
/*  7 */   S(THRSW) |                   S(LDTMU) |   S(LDUNIF),
/*  8 */                   S(LDVARY),
/*  9 */   S(THRSW) |      S(LDVARY),
/* 10 */                   S(LDVARY) |               S(LDUNIF),
/* 11 */   S(THRSW) |      S(LDVARY) |               S(LDUNIF),
#if V3D_VER_AT_LEAST(4,1,34,0)
/* 12 */                                             S(LDUNIFRF),
/* 13 */   S(THRSW) |                                S(LDUNIFRF),
#elif !V3D_VER_AT_LEAST(4,0,2,0)
/* 12 */                   S(LDVARY) |  S(LDTMU),
/* 13 */   S(THRSW) |      S(LDVARY) |  S(LDTMU),
#else
/* 12 */   ~0u,
/* 13 */   ~0u,
#endif
/* 14 */   S(SMALL_IMM) |  S(LDVARY),
/* 15 */   S(SMALL_IMM),
/* 16 */                   S(LDTLB),
/* 17 */                   S(LDTLBU),
#if V3D_VER_AT_LEAST(4,0,2,0)
/* 18 */                                             S(WRTMUC),
/* 19 */   S(THRSW) |                                S(WRTMUC),
/* 20 */                   S(LDVARY) |               S(WRTMUC),
/* 21 */   S(THRSW) |      S(LDVARY) |               S(WRTMUC),
#else
/* 18 */   ~0u,
/* 19 */   ~0u,
/* 20 */   ~0u,
/* 21 */   ~0u,
#endif
/* 22 */   S(UCB),
/* 23 */   S(ROTATE),
#if V3D_VER_AT_LEAST(4,1,34,0)
/* 24 */                                             S(LDUNIFA),
/* 25 */                                             S(LDUNIFARF),
/* 26 */   ~0u,
/* 27 */   ~0u,
/* 28 */   ~0u,
/* 29 */   ~0u,
/* 30 */   ~0u,
#elif !V3D_VER_AT_LEAST(4,0,2,0)
/* 24 */                  S(LDVPM),
/* 25 */   S(THRSW) |     S(LDVPM),
/* 26 */                  S(LDVPM) |                 S(LDUNIF),
/* 27 */   S(THRSW) |     S(LDVPM) |                 S(LDUNIF),
/* 28 */                  S(LDVPM) |    S(LDTMU),
/* 29 */   S(THRSW) |     S(LDVPM) |    S(LDTMU),
/* 30 */   S(SMALL_IMM) | S(LDVPM),
#else
/* 24 */   ~0u,
/* 25 */   ~0u,
/* 26 */   ~0u,
/* 27 */   ~0u,
/* 28 */   ~0u,
/* 29 */   ~0u,
/* 30 */   ~0u,
#endif
/* 31 */   S(SMALL_IMM) |               S(LDTMU),
#undef S
};

static v3d_qpu_sigbits_t decode_sig(uint32_t bits)
{
   assert(bits < countof(valid_sigs));
   return valid_sigs[bits];
}

bool v3d_qpu_try_encode_sigbits(v3d_qpu_sigbits_t sig, uint32_t* encoded)
{
   assert(sig != ~0u);

   for (int i=0; i<countof(valid_sigs); i++) {
      if (valid_sigs[i] == sig) {
         *encoded = i;
         return true;
      }
   }
   return false;
}

void v3d_qpu_encode_sigbits(v3d_qpu_sigbits_t sig, uint32_t* encoded)
{
   bool ok = v3d_qpu_try_encode_sigbits(sig, encoded);
   assert(ok);
}

/* These instructions are encoded like write instructions but don't actually use
 * the add write path in the QPU. The instruction encodings use the ma/waddr_a
 * fields for other things so can't just always encode a nop write. */
static bool v3d_qpu_instr_suppress_add_write(struct v3d_qpu_instr const *in)
{
   if(in->type == V3D_QPU_INSTR_TYPE_DEBUG)
   {
      return true;
   }
   else if(in->type == V3D_QPU_INSTR_TYPE_ALU
      && (in->u.alu.add.opcode == V3D_QPU_OP_STVPMV
         || in->u.alu.add.opcode == V3D_QPU_OP_STVPMD
         || in->u.alu.add.opcode == V3D_QPU_OP_STVPMP))
   {
      return true;
   }
   else
   {
      return false;
   }
}

v3d_qpu_instr_type_t v3d_qpu_instr_get_type(uint64_t bits)
{
   uint32_t i1 = (uint32_t)(bits >> 32);

   uint32_t op_mul = (i1 >> 26) & 63;
   uint32_t sig = (i1 >> 21) & 31;
   if (op_mul != 0)
      return V3D_QPU_INSTR_TYPE_ALU;
   else if ((sig & 24) == 24)
      return V3D_QPU_INSTR_TYPE_LDI;
   else if ((sig & 24) == 16)
      return V3D_QPU_INSTR_TYPE_BRANCH;
   else if ((sig & 28) == 0)
      return V3D_QPU_INSTR_TYPE_BKPT;
   else if (sig == 9)
      return V3D_QPU_INSTR_TYPE_DEBUG;
   else
      return V3D_QPU_INSTR_TYPE_INVALID;
}

void v3d_qpu_instr_unpack(struct v3d_qpu_instr* in, uint64_t bits)
{
   char err[V3D_QPU_INSTR_ERR_SIZE];
   bool ok = v3d_qpu_instr_try_unpack(in, bits, err);
   assert_msg(ok, "Instruction 0x%016" PRIx64 " invalid: %s", bits, err);
}

static bool is_write_valid(struct v3d_qpu_instr_write const* wr)
{
   return !wr->magic || v3d_is_valid_qpu_magic_waddr(wr->addr);
}

static void set_err(char err[V3D_QPU_INSTR_ERR_SIZE], const char *fmt, ...)
{
   if (!err)
      return;
   va_list args;
   va_start(args, fmt);
   size_t offset = vcos_safe_vsprintf(err, V3D_QPU_INSTR_ERR_SIZE, 0, fmt, args);
   va_end(args);
   assert(offset < V3D_QPU_INSTR_ERR_SIZE); /* Or we overflowed */
}

static bool force_no_magic(v3d_qpu_opcode_t op)
{
   switch(op)
   {
      /* These instructions use the magic flag to encode _in vs. _out and always
       * write to the regfile */
      case V3D_QPU_OP_LDVPMV_IN:
      case V3D_QPU_OP_LDVPMV_OUT:
      case V3D_QPU_OP_LDVPMD_IN:
      case V3D_QPU_OP_LDVPMD_OUT:
      case V3D_QPU_OP_LDVPMG_IN:
      case V3D_QPU_OP_LDVPMG_OUT:
         return true;

      default: return false;
   }
}

bool v3d_qpu_op_requires_read_a(v3d_qpu_opcode_t op)
{
#if V3D_VER_AT_LEAST(3,3,0,0)
   switch(op)
   {
      /* These instructions have timing restrictions that mean they may only
       * consume regfile read A */
      case V3D_QPU_OP_TLBWRCFG:
      case V3D_QPU_OP_TMUWRCFG:
#if !V3D_VER_AT_LEAST(4,0,2,0)
      case V3D_QPU_OP_VPMWRCFG:
#endif
         return true;

      default: /* Do nothing */ ;
   }
#endif
   return false;
}

bool v3d_qpu_instr_try_unpack(struct v3d_qpu_instr *in, uint64_t bits,
   char err[V3D_QPU_INSTR_ERR_SIZE])
{
   uint32_t i0 = (uint32_t)bits;
   uint32_t i1 = (uint32_t)(bits >> 32);

   in->type = v3d_qpu_instr_get_type(bits);

   switch (in->type)
   {
   case V3D_QPU_INSTR_TYPE_ALU:
   {
      in->u.alu.raddr_a = (i0 >> 6) & 0x3f;
      in->u.alu.raddr_b = i0 & 0x3f;

      uint32_t op_add = (i0 >> 24) & 255;
      uint32_t add_a = (i0 >> 12) & 7;
      uint32_t add_b = (i0 >> 15) & 7;
      if(!decode_add_op(&in->u.alu.add, op_add, add_a, add_b,
         /*waddr_a=*/(i1 & 0x3F), /*magic_add=*/(i1 >> 12) & 1))
      {
         set_err(err, "Unrecognised add op: opcode=%u, add_a=%u, add_b=%u",
            op_add, add_a, add_b);
         return false;
      }

      if (v3d_qpu_op_requires_read_a(in->u.alu.add.opcode) &&
          in->u.alu.add.a.source != V3D_QPU_IN_SOURCE_A)
      {
         set_err(err, "Bad op input source: %s requires input A", v3d_desc_qpu_opcode(in->u.alu.add.opcode));
         return false;
      }

      uint32_t op_mul = (i1 >> 26) & 63;
      uint32_t mul_a = (i0 >> 18) & 7;
      uint32_t mul_b = (i0 >> 21) & 7;
      if(!decode_mul_op(&in->u.alu.mul, op_mul, mul_a, mul_b))
      {
         set_err(err, "Unrecognised mul op: opcode=%u, mul_a=%u, mul_b=%u",
            op_mul, mul_a, mul_b);
         return false;
      }

      uint32_t sig = (i1 >> 21) & 31;
      in->u.alu.sig.sigbits = decode_sig(sig);
      if(in->u.alu.sig.sigbits == ~0u)
      {
         set_err(err, "Unrecognised signal %u", sig);
         return false;
      }
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (v3d_qpu_sig_has_result_write(in->u.alu.sig.sigbits)) {
         uint32_t cond = (i1 >> 14) & 0x7f;
         in->u.alu.sig.magic = (cond & 0x40);
         in->u.alu.sig.waddr = (cond & 0x3F);
         if (in->u.alu.sig.magic && !v3d_is_valid_qpu_magic_waddr(in->u.alu.sig.waddr))
         {
            set_err(err, "Unrecognised magic write addr %u (sig pipe)",
                         in->u.alu.sig.waddr);
            return false;
         }
      }
#elif V3D_VER_AT_LEAST(4,0,2,0)
      in->u.alu.sig.sig_reg = false;
      if(v3d_qpu_sig_has_result_write(in->u.alu.sig.sigbits)) {
         uint32_t cond = (i1 >> 14) & 0x7f;
         if (cond & 0x40) {
            in->u.alu.sig.sig_reg = true;
            in->u.alu.sig.waddr = cond & 0x3f;
         }
      }
#endif

      if((in->u.alu.sig.sigbits & V3D_QPU_SIG_ROTATE) && in->u.alu.raddr_b >= V3D_VPAR)
      {
         set_err(err, "Bad mul rotation %u", in->u.alu.raddr_b);
         return false;
      }

      if((in->u.alu.sig.sigbits & V3D_QPU_SIG_SMALL_IMM) && in->u.alu.raddr_b >= countof(v3d_qpu_small_imms))
      {
         set_err(err, "Unrecognised small immediate %u", in->u.alu.raddr_b);
         return false;
      }

      break;
   }
   case V3D_QPU_INSTR_TYPE_BRANCH:
      in->u.branch.raddr_a = (i0 >> 6) & 0x3f;

      in->u.branch.cond = (v3d_qpu_bcond_t)(i1 & 7);
      in->u.branch.msfign = (v3d_qpu_msfign_t)((i0 >> 21) & 3);

      in->u.branch.bdi = (v3d_qpu_bdest_t)((i0 >> 12) & 3);
      in->u.branch.i_addr = (i1 & 0x00fffff8) | (i0 & 0xff000000);

      in->u.branch.ub = !!(i0 & (1u << 14));
      in->u.branch.bdu = (v3d_qpu_bdest_t)((i0 >> 15) & 3);

      in->u.branch.ulr = !!((i0 >> 23) & 1);

      if (!v3d_is_valid_qpu_bcond(in->u.branch.cond))
      {
         set_err(err, "Unrecognised branch cond %u", (unsigned)in->u.branch.cond);
         return false;
      }
      if (!v3d_is_valid_qpu_msfign(in->u.branch.msfign))
      {
         set_err(err, "Unrecognised msfign value %u", (unsigned)in->u.branch.msfign);
         return false;
      }

      break;
   case V3D_QPU_INSTR_TYPE_LDI:
      in->u.ldi.mode = (v3d_qpu_ldi_mode_t)((i1 >> 22) & 3);
      in->u.ldi.imm = i0;

      if (!v3d_is_valid_qpu_ldi_mode(in->u.ldi.mode))
      {
         set_err(err, "Unrecognised LDI mode %u", (unsigned)in->u.ldi.mode);
         return false;
      }

      break;
   case V3D_QPU_INSTR_TYPE_BKPT:
      in->u.bkpt.halt_this = (i1 & 8) == 0;
      in->u.bkpt.halt_other = (i1 & 4) != 0;
      in->u.bkpt.raise_regular_int = (i1 & 2) != 0;
      in->u.bkpt.raise_debug_int = (i1 & 1) == 0;

      break;
   case V3D_QPU_INSTR_TYPE_DEBUG:
      in->u.debug.addr = i0 & 0x1fff;

      break;
   default:
      set_err(err, "Unrecognised instruction type");
      return false;
   }

   if (v3d_qpu_instr_does_write(in->type))
   {
      in->add_wr.addr = i1 & 0x3f;
      in->mul_wr.addr = (i1 >> 6) & 0x3f;
      in->add_wr.magic = !!(i1 & (1 << 12));
      in->mul_wr.magic = !!(i1 & (1 << 13));

      if((in->type == V3D_QPU_INSTR_TYPE_DEBUG) &&
         (in->add_wr.magic || in->add_wr.addr))
      {
         set_err(err, "Debug instruction encoding requires ma & waddr_a to be zero");
         return false;
      }

      if (v3d_qpu_instr_suppress_add_write(in))
      {
         in->add_wr.magic = true;
         in->add_wr.addr = V3D_QPU_MAGIC_WADDR_NOP;
      }

      if(in->type == V3D_QPU_INSTR_TYPE_ALU && force_no_magic(in->u.alu.add.opcode))
      {
         in->add_wr.magic = false;
      }

      uint32_t cond = (i1 >> 14) & 0x7f;
#if V3D_VER_AT_LEAST(4,0,2,0)
      if(in->type == V3D_QPU_INSTR_TYPE_ALU && v3d_qpu_sig_has_result_write(in->u.alu.sig.sigbits) )
      {
         cond = 0;
      }
#endif

      uint32_t ac = V3D_QPU_COND_ALWAYS;
      uint32_t mc = V3D_QPU_COND_ALWAYS;
      uint32_t af = V3D_QPU_SETF_NONE;
      uint32_t mf = V3D_QPU_SETF_NONE;
      if ((cond & 0x4c) == 0x40) {        /* 1mc00ac */
         mc = (cond >> 4) & 3;
         ac = cond & 3;
      } else if ((cond & 0x40) == 0x40) { /* 1mc:auf */
         mc = (cond >> 4) & 3;
         af = cond & 15;
      } else if ((cond & 0x70) == 0x30) { /* 011mcap or 011mc00 */
         mc = (cond >> 2) & 3;
         af = cond & 3;
      } else if ((cond & 0x70) == 0x20) { /* 010acmp or 011ac00 */
         ac = (cond >> 2) & 3;
         mf = cond & 3;
      } else if (cond == 0x10) {          /* reserved 0010000 */
         set_err(err, "Unrecognised cond field value %u", cond);
         return false;
      } else if ((cond & 0x70) == 0x10) { /* 001:muf or 00100mp */
         mf = cond & 15;
      } else {                            /* 000:auf or 00000ap or 0000000 */
         if(cond & 0x70)
         {
            set_err(err, "Unrecognised cond field value %u", cond);
            return false;
         }
         af = cond & 15;
      }
      in->add_wr.setf = (v3d_qpu_setf_t)af;
      in->mul_wr.setf = (v3d_qpu_setf_t)mf;
      in->add_wr.cond = (v3d_qpu_cond_t)ac;
      in->mul_wr.cond = (v3d_qpu_cond_t)mc;

      if (!is_write_valid(&in->add_wr))
      {
         set_err(err, "Unrecognised magic write addr %u (add pipe)",
            in->add_wr.addr);
         return false;
      }

      if (!is_write_valid(&in->mul_wr))
      {
         set_err(err, "Unrecognised magic write addr %u (mul pipe)",
            in->mul_wr.addr);
         return false;
      }
   }

   return true;
}

static bool encode_cond(struct v3d_qpu_instr const* in, uint32_t* bits)
{
   bool asf = (in->add_wr.setf != V3D_QPU_SETF_NONE);
   bool msf = (in->mul_wr.setf != V3D_QPU_SETF_NONE);
   bool ac = (in->add_wr.cond != V3D_QPU_COND_ALWAYS);
   bool mc = (in->mul_wr.cond != V3D_QPU_COND_ALWAYS);

   if(!asf && !msf && !ac && !mc) {
      *bits = 0;
      return true;
   }

   bool ov = false;
   if(msf) {
      if(in->mul_wr.setf >= V3D_QPU_SETF_ANDZ || !ac) {
         if(asf || ac || mc) return false;

         /* 001:muf or 00100mp */
         *bits = 0x10 | bits32_at(&ov, in->mul_wr.setf, 0, 4);
         return !ov;
      } else {
         if(mc || asf) return false;

         /* 010acmp */
         *bits = 0x20 | bits32_at(&ov, in->add_wr.cond, 2, 2) | bits32_at(&ov, in->mul_wr.setf, 0, 2);
         return !ov;
      }
   } else if(asf) {
      if(ac) return false;

      if(in->add_wr.setf >= V3D_QPU_SETF_ANDZ) {
         if(!mc) {
            /* 000:auf */
            *bits = bits32_at(&ov, in->add_wr.setf, 0, 4);
            return !ov;
         } else {
            /* 1mc:auf */
            *bits = 0x40 | bits32_at(&ov, in->mul_wr.cond, 4, 2) | bits32_at(&ov, in->add_wr.setf, 0, 4);
            return !ov;
         }
      } else {
         if(!mc) {
            /* 00000ap */
            *bits = bits32_at(&ov, in->add_wr.setf, 0, 2);
            return !ov;
         } else {
            /* 011mcap */
            *bits = 0x30 | bits32_at(&ov, in->mul_wr.cond, 2, 2) | bits32_at(&ov, in->add_wr.setf, 0, 2);
            return !ov;
         }
      }
   } else if(mc) {
      if(!ac) {
         /* 011mc00 */
         *bits = 0x30 | bits32_at(&ov, in->mul_wr.cond, 2, 2);
         return !ov;
      } else {
         /* 1mc00ac */
         *bits = 0x40 | bits32_at(&ov, in->mul_wr.cond, 4, 2) | bits32_at(&ov, in->add_wr.cond, 0, 2);
         return !ov;
      }
   } else {
      /* 010ac00 */
      *bits = 0x20 | bits32_at(&ov, in->add_wr.cond, 2, 2);
      return !ov;
   }

   return false;
}

bool v3d_qpu_instr_try_pack(struct v3d_qpu_instr const* in, uint64_t* bits)
{
   *bits = 0;

   if(v3d_qpu_instr_does_write(in->type))
   {
      if(!is_write_valid(&in->add_wr) || !is_write_valid(&in->mul_wr))
      {
         return false;
      }

      if (v3d_qpu_instr_suppress_add_write(in))
      {
         if(!in->add_wr.magic || in->add_wr.addr != V3D_QPU_MAGIC_WADDR_NOP) return false;
      }
   }

   bool ov = false;
   switch(in->type)
   {
   case V3D_QPU_INSTR_TYPE_ALU:
      {
         if(force_no_magic(in->u.alu.add.opcode) && in->add_wr.magic)
         {
            /* Decoder should have forced ma to false */
            return false;
         }
         if (v3d_qpu_op_requires_read_a(in->u.alu.add.opcode) && in->u.alu.add.a.source != V3D_QPU_IN_SOURCE_A)
         {
            /* Config register writes must come from the A regfile read */
            return false;
         }

         uint32_t sig;
         if(!v3d_qpu_try_encode_sigbits(in->u.alu.sig.sigbits, &sig))
         {
            return false;
         }

         uint32_t cond;
         if(!encode_cond(in, &cond))
         {
            return false;
         }

#if V3D_VER_AT_LEAST(4,0,2,0)
         if(v3d_qpu_sig_has_result_write(in->u.alu.sig.sigbits))
         {
            if (cond != 0)
            {
               return false;
            }
# if V3D_VER_AT_LEAST(4,1,34,0)
            cond = (in->u.alu.sig.magic ? 0x40 : 0) | in->u.alu.sig.waddr;
# else
            if (in->u.alu.sig.sig_reg)
               cond = 0x40 | in->u.alu.sig.waddr;
# endif
         }
#endif

         uint32_t op_mul = 0;
         uint32_t mul_a = in->u.alu.mul.a.source;
         uint32_t mul_b = in->u.alu.mul.b.source;
         if(!encode_mul_op(&in->u.alu.mul, &op_mul, &mul_a, &mul_b))
         {
            return false;
         }

         uint32_t op_add = 0;
         uint32_t add_a = in->u.alu.add.a.source;
         uint32_t add_b = in->u.alu.add.b.source;
         bool magic_a = in->add_wr.magic;
         uint32_t waddr_a = in->add_wr.addr;
         if(!encode_add_op(&in->u.alu.add, &op_add, &add_a, &add_b, &waddr_a, &magic_a))
         {
            return false;
         }

         if((in->u.alu.sig.sigbits & V3D_QPU_SIG_ROTATE) && in->u.alu.raddr_b >= V3D_VPAR)
         {
            return false;
         }
         if((in->u.alu.sig.sigbits & V3D_QPU_SIG_SMALL_IMM) && in->u.alu.raddr_b >= countof(v3d_qpu_small_imms))
         {
            /* reserved */
            return false;
         }

         *bits = *bits
            | bits_at(&ov, op_mul, 58, 6)
            | bits_at(&ov, sig, 53, 5)
            | bits_at(&ov, cond, 46, 7)
            | bits_at(&ov, in->mul_wr.magic? 1 : 0, 45, 1)
            | bits_at(&ov, magic_a? 1 : 0, 44, 1)
            | bits_at(&ov, in->mul_wr.addr, 38, 6)
            | bits_at(&ov, waddr_a, 32, 6)
            | bits_at(&ov, op_add, 24, 8)
            | bits_at(&ov, mul_b, 21, 3)
            | bits_at(&ov, mul_a, 18, 3)
            | bits_at(&ov, add_b, 15, 3)
            | bits_at(&ov, add_a, 12, 3)
            | bits_at(&ov, in->u.alu.raddr_a, 6, 6)
            | bits_at(&ov, in->u.alu.raddr_b, 0, 6);
      }
      break;

   case V3D_QPU_INSTR_TYPE_BRANCH:
      if(in->u.branch.i_addr & 7) return false;
      if(in->u.branch.cond == 1) return false; /* reserved */
      if(in->u.branch.msfign == 3) return false; /* reserved */

      *bits = *bits
         | bits_at(&ov, 0x02, 56, 8)
         | bits_at(&ov, (in->u.branch.i_addr & 0x00fffff8) >> 3, 35, 21)
         | bits_at(&ov, in->u.branch.cond, 32, 3)
         | bits_at(&ov, in->u.branch.i_addr >> 24, 24, 8)
         | bits_at(&ov, in->u.branch.ulr? 1 : 0, 23, 1)
         | bits_at(&ov, in->u.branch.msfign, 21, 2)
         | bits_at(&ov, in->u.branch.bdu, 15, 2)
         | bits_at(&ov, in->u.branch.ub? 1 : 0, 14, 1)
         | bits_at(&ov, in->u.branch.bdi, 12, 2)
         | bits_at(&ov, in->u.branch.raddr_a, 6, 6);
      break;

   case V3D_QPU_INSTR_TYPE_LDI:
      {
         uint32_t cond;
         if(!encode_cond(in, &cond)) return false;

         if(in->u.ldi.mode == 3) return false; /* reserved */

         *bits = *bits
            | bits_at(&ov, 0x03, 56, 8)
            | bits_at(&ov, in->u.ldi.mode, 54, 2)
            | bits_at(&ov, cond, 46, 7)
            | bits_at(&ov, in->mul_wr.magic? 1 : 0, 45, 1)
            | bits_at(&ov, in->add_wr.magic? 1 : 0, 44, 1)
            | bits_at(&ov, in->mul_wr.addr, 38, 6)
            | bits_at(&ov, in->add_wr.addr, 32, 6)
            | bits_at(&ov, in->u.ldi.imm, 0, 32);
      }
      break;

   case V3D_QPU_INSTR_TYPE_BKPT:
      *bits = *bits
         | (!in->u.bkpt.halt_this? 8 : 0)
         | (in->u.bkpt.halt_other? 4 : 0)
         | (in->u.bkpt.raise_regular_int? 2 : 0)
         | (!in->u.bkpt.raise_debug_int? 1 : 0);
      *bits <<= 32;
      break;

   case V3D_QPU_INSTR_TYPE_DEBUG:
      {
         uint32_t cond;
         if(!encode_cond(in, &cond)) return false;

         *bits = *bits
            | bits_at(&ov, 0x9, 53, 11)
            | bits_at(&ov, cond, 46, 7)
            | bits_at(&ov, in->mul_wr.magic? 1 : 0, 45, 1)
            | bits_at(&ov, in->mul_wr.addr, 38, 6)
            | bits_at(&ov, in->u.debug.addr, 0, 13);
      }
      break;
   default:
      return false;
      break;
   }

   return !ov;
}

v3d_qpu_res_type_t v3d_qpu_res_type_from_opcode(v3d_qpu_opcode_t opcode)
{
   switch (opcode)
   {
   case V3D_QPU_OP_TIDX:
   case V3D_QPU_OP_EIDX:
   case V3D_QPU_OP_LR:
   case V3D_QPU_OP_XCD:
   case V3D_QPU_OP_YCD:
   case V3D_QPU_OP_MSF:
   case V3D_QPU_OP_REVF:
   case V3D_QPU_OP_TMUWT:
   case V3D_QPU_OP_IID:
   case V3D_QPU_OP_NOT:
   case V3D_QPU_OP_SETMSF:
   case V3D_QPU_OP_SETREVF:
   case V3D_QPU_OP_VPMSETUP:
   case V3D_QPU_OP_FTOC:
   case V3D_QPU_OP_CLZ:
   case V3D_QPU_OP_MOV:
   case V3D_QPU_OP_TMUWRCFG:
   case V3D_QPU_OP_TLBWRCFG:
   case V3D_QPU_OP_VPMWRCFG:
   case V3D_QPU_OP_ROR:
   case V3D_QPU_OP_AND:
   case V3D_QPU_OP_OR:
   case V3D_QPU_OP_XOR:
   case V3D_QPU_OP_ADD:
   case V3D_QPU_OP_SUB:
   case V3D_QPU_OP_MIN:
   case V3D_QPU_OP_MAX:
   case V3D_QPU_OP_UMIN:
   case V3D_QPU_OP_UMAX:
   case V3D_QPU_OP_SHL:
   case V3D_QPU_OP_SHR:
   case V3D_QPU_OP_ASR:
   case V3D_QPU_OP_NEG:
   case V3D_QPU_OP_FLAPUSH:
   case V3D_QPU_OP_FLBPUSH:
   case V3D_QPU_OP_FLPOP:
   case V3D_QPU_OP_FTOIN:
   case V3D_QPU_OP_FTOIZ:
   case V3D_QPU_OP_FTOUZ:
   case V3D_QPU_OP_UMUL24:
   case V3D_QPU_OP_SMUL24:
   case V3D_QPU_OP_MULTOP:
   case V3D_QPU_OP_SAMPID:
#if V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_QPU_OP_VPMWT:
   case V3D_QPU_OP_BARRIERID:
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_QPU_OP_FLAFIRST:
#endif
      return V3D_QPU_RES_TYPE_32I;
   case V3D_QPU_OP_FADD:
   case V3D_QPU_OP_FADDNF:
   case V3D_QPU_OP_FSUB:
   case V3D_QPU_OP_FMIN:
   case V3D_QPU_OP_FMAX:
   case V3D_QPU_OP_FCMP:
   case V3D_QPU_OP_FMUL:
   case V3D_QPU_OP_FXCD:
   case V3D_QPU_OP_FYCD:
   case V3D_QPU_OP_FROUND:
   case V3D_QPU_OP_FTRUNC:
   case V3D_QPU_OP_FFLOOR:
   case V3D_QPU_OP_FCEIL:
   case V3D_QPU_OP_FDX:
   case V3D_QPU_OP_FDY:
   case V3D_QPU_OP_ITOF:
   case V3D_QPU_OP_UTOF:
   case V3D_QPU_OP_FMOV:
      return V3D_QPU_RES_TYPE_32F;
   case V3D_QPU_OP_VFLA:
   case V3D_QPU_OP_VFLNA:
   case V3D_QPU_OP_VFLB:
   case V3D_QPU_OP_VFLNB:
   case V3D_QPU_OP_VADD:
   case V3D_QPU_OP_VSUB:
      return V3D_QPU_RES_TYPE_2X16I;
   case V3D_QPU_OP_VFMOV:
   case V3D_QPU_OP_VFMOVABS:
   case V3D_QPU_OP_VFMOVNEG:
   case V3D_QPU_OP_VFMOVNAB:
   case V3D_QPU_OP_VFABSMOV:
   case V3D_QPU_OP_VFABS:
   case V3D_QPU_OP_VFABSNEG:
   case V3D_QPU_OP_VFABSNAB:
   case V3D_QPU_OP_VFNEGMOV:
   case V3D_QPU_OP_VFNEGABS:
   case V3D_QPU_OP_VFNEG:
   case V3D_QPU_OP_VFNEGNAB:
   case V3D_QPU_OP_VFNABMOV:
   case V3D_QPU_OP_VFNABABS:
   case V3D_QPU_OP_VFNABNEG:
   case V3D_QPU_OP_VFNABS:
   case V3D_QPU_OP_VFPACK:
   case V3D_QPU_OP_VFMIN:
   case V3D_QPU_OP_VFMAX:
   case V3D_QPU_OP_VFMUL:
      return V3D_QPU_RES_TYPE_2X16F;
   case V3D_QPU_OP_NOP:
   case V3D_QPU_OP_STVPMV:
   case V3D_QPU_OP_STVPMD:
   case V3D_QPU_OP_STVPMP:
      return V3D_QPU_RES_TYPE_NOP;
   case V3D_QPU_OP_LDVPMV_IN:
   case V3D_QPU_OP_LDVPMD_IN:
   case V3D_QPU_OP_LDVPMV_OUT:
   case V3D_QPU_OP_LDVPMD_OUT:
   case V3D_QPU_OP_LDVPMP:
   case V3D_QPU_OP_LDVPMG_IN:
   case V3D_QPU_OP_LDVPMG_OUT:
      return V3D_QPU_RES_TYPE_DIRECT_RF_WRITE;
   default:
      unreachable();
      return V3D_QPU_RES_TYPE_INVALID;
   }
}

void v3d_qpu_res_types_from_instr(
   v3d_qpu_res_type_t *add_type, v3d_qpu_res_type_t *mul_type,
   const struct v3d_qpu_instr *in)
{
   assert(v3d_qpu_instr_does_write(in->type));

   switch (in->type)
   {
   case V3D_QPU_INSTR_TYPE_ALU:
      *add_type = v3d_qpu_res_type_from_opcode(in->u.alu.add.opcode);
      *mul_type = v3d_qpu_res_type_from_opcode(in->u.alu.mul.opcode);
      break;
   case V3D_QPU_INSTR_TYPE_LDI:
   case V3D_QPU_INSTR_TYPE_DEBUG:
      *add_type = V3D_QPU_RES_TYPE_NOP;
      *mul_type = V3D_QPU_RES_TYPE_32I;
      break;
   default:
      unreachable();
   }
}

bool v3d_qpu_alu_uses_source(struct v3d_qpu_instr_alu const* alu, v3d_qpu_in_source_t src)
{
   uint32_t const add_arity = v3d_qpu_op_num_inputs(alu->add.opcode);
   uint32_t const mul_arity = v3d_qpu_op_num_inputs(alu->mul.opcode);
   return (add_arity > 0 && alu->add.a.source == src)
      || (add_arity > 1 && alu->add.b.source == src)
      || (mul_arity > 0 && alu->mul.a.source == src)
      || (mul_arity > 1 && alu->mul.b.source == src);
}
