/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
Responsible for fitting nodes into the ADD and MUL units and allocating registers
for their destinations.
=============================================================================*/

#include <stdlib.h>
#include <setjmp.h>

#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_binary_shader.h"

#include "helpers/gfx/gfx_util.h"

#ifdef KHRN_SHADER_STATS
#include <stdio.h>
#endif

#include <limits.h>

#define SCHEDULER_LOOKBACK 10

const char *mov_excuse_strings[] = {
   "",
   "   # unfolded output",
   "   # mov present in IR",
   "   # move out of the way",
   "   # move flag B out of the way for a push",
   "   # move acc due to thrsw",
   "   # copy on write",
   "   # can't write to r5",
   "   # flagify",
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

/* Look up the code for this combination of signals, or return ~0u if the combination is invalid */
static uint32_t translate_sigbits(uint32_t sigbits)
{
   for (uint32_t i = 0; i < 32; i++)
   {
      if (sigbit_table[i] == sigbits)
         return i;
   }
   return ~0u;
}

static void translate_write(uint32_t *waddr_out, bool *is_magic, uint32_t reg)
{
   assert(reg != REG_UNDECIDED);

   if (IS_RF(reg))
   {
      *waddr_out = reg - REG_RF(0);
      *is_magic = false;
   }
   else
   {
      *is_magic = true;

      if (reg == REG_R5QUAD) *waddr_out = 5;
      else if (IS_R(reg))
      {
         assert(reg != REG_R(5)); /* r5 writes should go via REG_R5QUAD */
         *waddr_out = reg - REG_R(0);
      }
      else  /* REG_MAGIC_* */
      {
         assert(IS_MAGIC(reg));
         assert(reg >= REG_MAGIC(6));
         *waddr_out = reg - REG_MAGIC(0);
      }
   }
}

static uint32_t translate_wac(bool magic_a, bool magic_b)
{
   uint32_t wac = 0;
   if (magic_b) wac |= 0x2;
   if (magic_a) wac |= 0x1;
   return wac;
}

static uint32_t translate_mux(uint32_t reg, const uint32_t *reads)
{
   assert(IS_RRF(reg));

   if (IS_R(reg))
      return reg - REG_R(0);
   else {
      if (reg == reads[0]) return 6;
      else {
         assert(reg == reads[1]);
         return 7;
      }
   }
}

static uint32_t translate_cond_setf(uint32_t a_cond_setf, uint32_t m_cond_setf)
{
   uint32_t cond;
   /* Internally we represent PUSH and UPDT with the correct numbers. COND needs to be anded with 3 though. */
   if (     cs_is_none(a_cond_setf) && cs_is_none(m_cond_setf))
      cond = 0;
   else if (cs_is_push(a_cond_setf) && cs_is_none(m_cond_setf))
      cond = a_cond_setf;
   else if (cs_is_updt(a_cond_setf) && cs_is_none(m_cond_setf))
      cond = a_cond_setf;
   else if (cs_is_none(a_cond_setf) && cs_is_push(m_cond_setf))
      cond = 16 | m_cond_setf;
   else if (cs_is_none(a_cond_setf) && cs_is_updt(m_cond_setf))
      cond = 16 | m_cond_setf;
   else if (cs_is_cond(a_cond_setf) && cs_is_none(m_cond_setf))
      cond = 32 | (a_cond_setf&3) << 2;
   else if (cs_is_cond(a_cond_setf) && cs_is_push(m_cond_setf))
      cond = 32 | (a_cond_setf&3) << 2 | m_cond_setf;
   else if (cs_is_none(a_cond_setf) && cs_is_cond(m_cond_setf))
      cond = 48 | (m_cond_setf&3) << 2;
   else if (cs_is_push(a_cond_setf) && cs_is_cond(m_cond_setf))
      cond = 48 | (m_cond_setf&3) << 2 | a_cond_setf;
   else if (cs_is_cond(a_cond_setf) && cs_is_cond(m_cond_setf))
      cond = 64 | (m_cond_setf&3) << 4 | (a_cond_setf&3);
   else if (cs_is_updt(a_cond_setf) && cs_is_cond(m_cond_setf))
      cond = 64 | (m_cond_setf&3) << 4 | a_cond_setf;
   else
      UNREACHABLE();

   return cond;
}

static void gen_nop(uint32_t *op, uint32_t *mux_a, uint32_t *mux_b, bool add)
{
   /* Fill in with a nop */
   if (add) *op = 187;
   else     *op = 15;

   *mux_a = 0;
   *mux_b = add ? 0 : 4;
}

static void gen_from_op(const GLSL_OP_T *op_struct, uint32_t *out_op,
                        uint32_t *out_mux_a, uint32_t *out_mux_b,
                        const uint32_t *instr_reads)
{
   uint32_t op = op_struct->op;
   uint32_t mux_a, mux_b;

   assert (op_struct->used);

   /* Sort out input muxes (for 0 and 1 input instructions these also hold part of the opcode) */
   if (op_struct->input_a != REG_UNDECIDED) {
      assert(op_struct->op2 == ~0u);
      mux_a = translate_mux(op_struct->input_a, instr_reads);
   } else {
      assert(op_struct->op2 < 8);
      mux_a = op_struct->op2;
   }
   if (op_struct->input_b != REG_UNDECIDED) {
      assert(op_struct->op1 == ~0u);
      mux_b = translate_mux(op_struct->input_b, instr_reads);
   } else {
      assert(op_struct->op1 < 8);
      mux_b = op_struct->op1;
   }

   if (op_struct->swap_mode != 0)
   {
      /* Swap arguments and unpack modes for add/addnf and fmin/fmax */
      uint32_t temp;
      uint32_t arga = (op_struct->op & 12) * 2 + mux_a;
      uint32_t argb = (op_struct->op & 3) * 8 + mux_b;
      if (arga == argb)
      {
         /* Special case: fmax can be used with equal arguments, as it's equivalent to fmin */
         assert(op_struct->swap_mode == 1 || (op >= 128 && op <= 175));
      }
      else if ( (op_struct->swap_mode == 1 && arga > argb) || (op_struct->swap_mode == 2 && arga < argb) )
      {
         op = (op & ~15) | (op & 12) >> 2 | (op & 3) << 2;
         temp = mux_a;
         mux_a = mux_b;
         mux_b = temp;
      }
   }

   *out_op = op;
   *out_mux_a = mux_a;
   *out_mux_b = mux_b;
}

static inline void add_read(uint32_t *read, uint32_t reg) {
   assert(IS_RF(reg));
   /* No need to record this, it's already here */
   if (read[0] == reg || read[1] == reg) return;

   if (read[0] == REG_UNDECIDED) read[0] = reg;
   else {
      assert(read[1] == REG_UNDECIDED);
      read[1] = reg;
   }
}

static void instr_get_reads(const INSTR_T *ci, uint32_t *read) {
   read[0] = read[1] = REG_UNDECIDED;
   if (ci->a.used && IS_RF(ci->a.input_a)) add_read(read, ci->a.input_a);
   if (ci->a.used && IS_RF(ci->a.input_b)) add_read(read, ci->a.input_b);
   if (ci->m.used && IS_RF(ci->m.input_a)) add_read(read, ci->m.input_a);
   if (ci->m.used && IS_RF(ci->m.input_b)) add_read(read, ci->m.input_b);
}

static uint64_t generate(const INSTR_T *ci)
{
   uint32_t a_output, m_output;
   uint32_t sig, cond, waddr_b, waddr_a;
   uint32_t i0, i1;
   bool magic_a, magic_b;

   /* Replace uninitialised half instructions with nops */
   if (ci->a.used) a_output = ci->a.output;
   else            a_output = REG_MAGIC_NOP;
   if (ci->m.used) m_output = ci->m.output;
   else            m_output = REG_MAGIC_NOP;

   /* Sort out writes first - these apply to both ALU and immediate instructions */
   translate_write(&waddr_a, &magic_a, a_output);
   translate_write(&waddr_b, &magic_b, m_output);
   cond = translate_cond_setf(ci->a.cond_setf, ci->m.cond_setf);

   uint32_t op_add, op_mul;
   uint32_t read[2];
   uint32_t raddr[2] = {0, 0};
   uint32_t add_a, add_b;
   uint32_t mul_a, mul_b;

   instr_get_reads(ci, read);

   /* ALU instruction */
   sig = translate_sigbits(ci->sigbits);

   if (ci->a.used) gen_from_op(&ci->a, &op_add, &add_a, &add_b, read);
   else gen_nop(&op_add, &add_a, &add_b, true);
   if (ci->m.used) gen_from_op(&ci->m, &op_mul, &mul_a, &mul_b, read);
   else gen_nop(&op_mul, &mul_a, &mul_b, false);

   for (int i=0; i<2; i++) {
      if (read[i] != REG_UNDECIDED) {
         assert(IS_RF(read[i]));
         raddr[i] = read[i] - REG_RF(0);
      }
   }

   if (ci->a.used && op_add == 248) {
      /* stvpm -- override waddr_a to get the correct type */
      assert(waddr_a == 6 && magic_a);
      waddr_a = 0;
      magic_a = false;
   }

   uint32_t wac = translate_wac(magic_a, magic_b);

   i1 = op_mul << 26 | sig << 21 | cond << 14 | wac << 12 | waddr_b << 6 | waddr_a;
   i0 = op_add << 24 | mul_b << 21 | mul_a << 18 | add_b << 15 | add_a << 12 | raddr[0] << 6 | raddr[1];

   return (uint64_t)i1 << 32 | (uint64_t)i0;
}

void generate_all_instructions(const INSTR_T *instructions,
                               uint32_t instr_count,
                               GENERATED_SHADER_T *gen)
{
   uint32_t unif_count = 0;
   uint32_t varying_count = 0;

   for (unsigned i = 0; i < instr_count; i++)
   {
      uint64_t instr;
      const INSTR_T *ci = &instructions[i];

      /* instruction */
      instr = generate(ci);
      gen->instructions[2*i] = (uint32_t)instr;
      gen->instructions[2*i+1] = (uint32_t)(instr >> 32);

      /* uniform */
      if (ci->unif != UNIF_NONE)
      {
         if (unif_count >= MAX_UNIFORMS) backend_error("Too many uniforms");

         assert((ci->conflicts & CONFLICT_UNIF) != 0);
         gen->unifs[2*unif_count] = (uint32_t)ci->unif;
         gen->unifs[2*unif_count+1] = (uint32_t)(ci->unif >> 32);
         unif_count++;
      }
      else
      {
         assert((ci->conflicts & CONFLICT_UNIF) == 0);
      }

      /* varying */
      if (ci->sigbits & SIGBIT_LDVARY && ci->varying < VARYING_ID_HW_0)
      {
         assert(varying_count < MAX_VARYINGS);
         gen->varyings[varying_count] = ci->varying;
         varying_count++;
      }
   }
   gen->instruction_count = instr_count;
   gen->unif_count = unif_count;
   gen->varying_count = varying_count;
}

static bool can_add_sigbits(uint32_t a, uint32_t b) {
   return (a & b) == 0 && translate_sigbits(a | b) != ~0u;
}

static uint32_t sig_conflicts(uint32_t sigbits) {
   switch (sigbits)
   {
   case SIGBIT_WRTMUC:
   case SIGBIT_LDUNIF:
      return CONFLICT_UNIF;
   case SIGBIT_LDTLBU:
      return (CONFLICT_TLB | CONFLICT_PERIPH | CONFLICT_UNIF);
   case SIGBIT_LDTLB:
      return (CONFLICT_TLB | CONFLICT_PERIPH);
   case SIGBIT_LDVPM:
   case SIGBIT_LDTMU:
      return CONFLICT_PERIPH;
   default:
      return 0;
   }
}

static uint32_t fit_sig(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t sigbits, uint64_t unif)
{
   uint32_t conflicts = sig_conflicts(sigbits);

   /* Advance current instruction until we find one which doesn't conflict */
   INSTR_T *ci;
   while (true) {
      ci = &sched->instructions[time];
      if (can_add_sigbits(ci->sigbits, sigbits) && (ci->conflicts & conflicts) == 0)
         break;

      time++;

      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");
   }

   /* Fill in instruction */
   assert( (conflicts & CONFLICT_UNIF) || unif == UNIF_NONE);

   ci->sigbits |= sigbits;
   ci->conflicts |= conflicts;
   if (unif != UNIF_NONE) ci->unif = unif;

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
         return (CONFLICT_TLB | CONFLICT_PERIPH);
      case REG_MAGIC_VPM:
      case REG_MAGIC_VPMU:
      case REG_MAGIC_TMU:
      case REG_MAGIC_TMUL:
      case REG_MAGIC_TMUA:
      case REG_MAGIC_TMUAU:
      case REG_MAGIC_TMUD:
#if V3D_HAS_NEW_TMU_CFG
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
#endif
      case REG_MAGIC_RECIP:
      case REG_MAGIC_RSQRT:
      case REG_MAGIC_EXP:
      case REG_MAGIC_LOG:
      case REG_MAGIC_SIN:
         return CONFLICT_PERIPH;
      }
   }

   return 0;
}

static inline bool IS_SFU(uint32_t output) {
   return output == REG_MAGIC_RECIP ||
          output == REG_MAGIC_RSQRT ||
          output == REG_MAGIC_EXP   ||
          output == REG_MAGIC_LOG   ||
          output == REG_MAGIC_SIN;
}

static uint32_t get_output_delay(uint32_t output) {
   if (IS_SFU(output)) return 2;
   else return 1;
}

static uint32_t get_cond_setf_conflicts(uint32_t cond_setf, ALU_TYPE_T alu_type)
{
   uint32_t conflict = 0;
   assert(is_cond_setf(cond_setf));
   if (cs_is_none(cond_setf)) return 0;

   switch (alu_type)
   {
   case ALU_A:
   case ALU_A_SWAP0:
   case ALU_A_SWAP1:
      conflict |= CONFLICT_ADDFLAG; break;
   case ALU_M:
      conflict |= CONFLICT_MULFLAG; break;
   default: UNREACHABLE();
   }

   if (cs_is_push(cond_setf) || cs_is_updt(cond_setf)) conflict |= CONFLICT_SETFLAG;

   /* Special Case: there isn't encoding space for muf + an add thing */
   if (cs_is_updt(cond_setf) && alu_type == ALU_M) conflict |= CONFLICT_ADDFLAG;

   return conflict;
}

static uint32_t get_special_instruction_conflicts(uint32_t op, uint32_t op1, uint32_t op2) {
   /* TODO: Replace these with constants. */
   if (op == 187) {  /* 187, 3 == VPM_SETUP */
      if (op1 == 3) return CONFLICT_PERIPH;
   }
   if (op == 188 || op == 189) return CONFLICT_PERIPH;      /* ldvpm* */
   if (op == 248)              return CONFLICT_PERIPH;      /* stvpm */
   return 0;
}

static void read_register(GLSL_SCHEDULER_STATE_T *sched, uint32_t reg, uint32_t time) {
   sched->reg[reg].available = MAX(sched->reg[reg].available, time);
}

/* This functions like 'read_register' but cleans the register if needed  */
/* NB. Although we clean the register here, we may have been inserted before
 * another read, so the final read time of the register may be in the future. */
static void clean_register(GLSL_SCHEDULER_STATE_T *sched, uint32_t reg, uint32_t time) {
   assert(IS_RRF(reg) || IS_FLAG(reg));

   read_register(sched, reg, time);

   /* Since we didn't do this in dereference() we need to do it now. */
   if (sched->reg[reg].user != NULL && sched->reg[reg].user->remaining_dependents == 0)
      sched->reg[reg].user = NULL;
}

static void invalidate_register(GLSL_SCHEDULER_STATE_T *sched, uint32_t reg, uint32_t time)
{
   assert(sched->reg[reg].user == NULL);
   assert(sched->reg[reg].available <= time);
   sched->reg[reg].available = time;
}

static void write_register(GLSL_SCHEDULER_STATE_T *sched, Backflow *node, uint32_t reg, uint32_t time)
{
   /* node may be NULL, in which case we are writing throw-away data */
   if (node != NULL) node->reg = reg;
   sched->reg[reg].user = node;
   sched->reg[reg].written = time;

   assert(sched->reg[reg].available <= time);
   sched->reg[reg].available = time;
}

static inline bool no_read_clashes(const uint32_t *old_read, const uint32_t *new_read)
{
   int n_old_reads = (old_read[0] != REG_UNDECIDED) + (old_read[1] != REG_UNDECIDED);
   int n_new_reads = (new_read[0] != REG_UNDECIDED) + (new_read[1] != REG_UNDECIDED);
   if (new_read[0] == old_read[0] || new_read[0] == old_read[1]) n_new_reads--;
   if (new_read[1] == old_read[0] || new_read[1] == old_read[1]) n_new_reads--;

   return (n_old_reads + n_new_reads) < 3;
}

static inline bool can_place_alu(const INSTR_T *ci, ALU_TYPE_T alu_type, uint32_t conflicts, const uint32_t *read)
{
   bool can_place = true;
   uint32_t instr_read[2];
   instr_get_reads(ci, instr_read);

   can_place &= ((alu_type == ALU_M) ? (!ci->m.used) : (!ci->a.used));
   can_place &= ((ci->conflicts & conflicts) == 0);
   can_place &= no_read_clashes(instr_read, read);

   return can_place;
}

static inline void set_alu(GLSL_OP_T *alu, uint32_t op, uint32_t op1, uint32_t op2, uint32_t output, uint32_t input_a, uint32_t input_b, uint32_t cond_setf, uint32_t swap_mode, MOV_EXCUSE mov_excuse)
{
   assert(!alu->used);

   alu->used = true;
   alu->op = op;
   alu->op1 = op1;
   alu->op2 = op2;
   alu->output = output;
   alu->input_a = input_a;
   alu->input_b = input_b;
   alu->cond_setf = cond_setf;
   alu->swap_mode = swap_mode;
   alu->mov_excuse = mov_excuse;
}

uint32_t get_conflicts(uint32_t output, uint32_t cond_setf, ALU_TYPE_T alu_type,
                       uint32_t op, uint32_t op1, uint32_t op2, uint64_t unif)
{
   uint32_t conflicts[4];
   conflicts[0] = get_output_conflicts(output);
   conflicts[1] = get_cond_setf_conflicts(cond_setf, alu_type);
   conflicts[2] = get_special_instruction_conflicts(op, op1, op2);
   conflicts[3] = (unif != UNIF_NONE ? CONFLICT_UNIF : 0);
   for (int i=1; i<4; i++) {
      /* If conflicts[] contain the same bits then it conflicts with itself */
      assert((conflicts[0] & conflicts[i]) == 0);
      conflicts[0] |= conflicts[i];
   }
   return conflicts[0];
}

static uint32_t fit_alu(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, ALU_TYPE_T alu_type,
                        uint32_t op, uint32_t op1, uint32_t op2, uint32_t output,
                        uint32_t input_a, uint32_t input_b, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   uint32_t read[2] = { REG_UNDECIDED, REG_UNDECIDED };
   bool mul = (alu_type == ALU_M);
   bool add = !mul;
   int swap_mode = 0;
   INSTR_T *ci;
   int n_reads = 0;

   /* Determine conflicts */
   uint32_t conflicts = get_conflicts(output, cond_setf, alu_type, op, op1, op2, unif);

   /* See which regfiles we're reading from */
   if (IS_RF(input_a)) read[n_reads++] = input_a;
   if (IS_RF(input_b)) read[n_reads++] = input_b;

   /* Advance current instruction until we find one which doesn't conflict */
   while (true) {
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
            set_alu(&ci->m, ci->alt_mov_i->m.op, ci->alt_mov_i->m.op1, ci->alt_mov_i->m.op2,
                            ci->alt_mov_i->m.output, ci->alt_mov_i->m.input_a, ci->alt_mov_i->m.input_b,
                            ci->alt_mov_i->m.cond_setf, ci->alt_mov_i->m.swap_mode, ci->alt_mov_i->m.mov_excuse);
            ci->a.used = false;
            ci->alt_mov_i = NULL;
            break;
         } else if(suitable && mul && ci->m.used && !ci->a.used && ci->alt_mov_i && ci->alt_mov_i->a.used) {
            set_alu(&ci->a, ci->alt_mov_i->a.op, ci->alt_mov_i->a.op1, ci->alt_mov_i->a.op2,
                            ci->alt_mov_i->a.output, ci->alt_mov_i->a.input_a, ci->alt_mov_i->a.input_b,
                            ci->alt_mov_i->a.cond_setf, ci->alt_mov_i->a.swap_mode, ci->alt_mov_i->a.mov_excuse);
            ci->m.used = false;
            ci->alt_mov_i = NULL;
            break;
         }
      }

      time++;

      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");
   }

   /* Set swap mode so that we know what to do when generating. */
   /* Different modes to determine which argument to put first. */
   if (alu_type == ALU_A_SWAP0) swap_mode = 1;
   if (alu_type == ALU_A_SWAP1) swap_mode = 2;

   assert(output != REG_R(5));

   /* Fill in instruction */
   assert( add || mul );
   if (add) set_alu(&ci->a, op, op1, op2, output, input_a, input_b, cond_setf, swap_mode, mov_excuse);
   else set_alu(&ci->m, op, op1, op2, output, input_a, input_b, cond_setf, swap_mode, mov_excuse);

   if (unif != UNIF_NONE) {
      assert(ci->unif == UNIF_NONE);
      ci->unif = unif;
   }
   assert(!(ci->conflicts & conflicts));
   ci->conflicts |= conflicts;

   return time;
}

static uint32_t fit_multi(GLSL_SCHEDULER_STATE_T *sched, uint32_t time,
                          uint32_t op_a, uint32_t op1_a, uint32_t op2_a,
                          uint32_t op_m, uint32_t op1_m, uint32_t op2_m,
                          uint32_t output, uint32_t input, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   uint32_t conflicts_a, conflicts_m;
   uint32_t read[2] = { REG_UNDECIDED, REG_UNDECIDED };
   bool add = false, mul = false;

   /* Determine conflicts for each lane */
   conflicts_a = get_conflicts(output, cond_setf, ALU_A, op_a, op1_a, op2_a, unif);
   conflicts_m = get_conflicts(output, cond_setf, ALU_M, op_m, op1_m, op2_m, unif);

   /* See which regfiles we're reading from */
   assert(input!=REG_UNDECIDED);
   if (IS_RF(input)) read[0] = input;

   /* Advance current instruction until we find one which doesn't conflict */
   INSTR_T *ci;
   while (true) {
      ci = &sched->instructions[time];

      if(can_place_alu(ci, ALU_M, conflicts_m, read)) mul = true;
      if(can_place_alu(ci, ALU_A, conflicts_a, read)) add = true;

      if( add || mul) break;

      time++;

      if (time >= MAX_INSTRUCTIONS) backend_error("Too many instructions");
   }

   assert(output != REG_R(5));

   if (unif != UNIF_NONE) {
      assert(ci->unif == UNIF_NONE);
      ci->unif = unif;
   }

   /* Fill in instruction */
   assert(add || mul);
   if (mul) {
      set_alu(&ci->m, op_m, op1_m, op2_m, output, input, REG_UNDECIDED, cond_setf, 0, mov_excuse);

      assert(!(ci->conflicts & conflicts_m));
      ci->conflicts |= conflicts_m;
      ci->alt_mov_i = NULL;

      if(add) {
         INSTR_T *alti = (INSTR_T*)malloc_fast(sizeof(INSTR_T));
         memset(alti, 0, sizeof(INSTR_T));

         ci->alt_mov_i = alti;
         set_alu(&alti->a, op_a, op1_a, op2_a, output, input, input, cond_setf, 0, mov_excuse);

         assert(alti->conflicts == 0);
         alti->conflicts = conflicts_a;
      }
   } else {
      set_alu(&ci->a, op_a, op1_a, op2_a, output, input, input, cond_setf, 0, mov_excuse);

      assert(!(ci->conflicts & conflicts_a));
      ci->conflicts |= conflicts_a;
      ci->alt_mov_i = NULL;
   }

   return time;
}

static uint32_t fit_mov(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t output, uint32_t input, uint64_t unif, uint32_t cond_setf, MOV_EXCUSE mov_excuse)
{
   if (input == REG_FLAG_A)            /* flna instruction */
      return fit_alu(sched, time, ALU_A, 187, 0, 5, output, REG_UNDECIDED, REG_UNDECIDED, unif, cond_setf, mov_excuse);
   else if (input == REG_FLAG_B)       /* flnb instruction */
      return fit_alu(sched, time, ALU_A, 187, 0, 7, output, REG_UNDECIDED, REG_UNDECIDED, unif, cond_setf, mov_excuse);
   else {
      assert(IS_RRF(input));
      return fit_multi(sched, time, 182, ~0u, ~0u, 15, 7, ~0u, output, input, unif, cond_setf, mov_excuse);
   }
}

/* No op may write rf* or read rf[0,2] on or after thrend */
static bool op_valid_for_thrend(const GLSL_OP_T *op) {
   if (!op->used) return true;

   if (IS_RF(op->output)) return false;
   if (op->input_a == REG_RF(0) || op->input_a == REG_RF(1) || op->input_a == REG_RF(2)) return false;
   if (op->input_b == REG_RF(0) || op->input_b == REG_RF(1) || op->input_b == REG_RF(2)) return false;
   return true;
}

/* Instructions must have valid ops and not load uniforms on or after thrend */
static bool instr_valid_for_thrend(const INSTR_T *i) {
   if (!op_valid_for_thrend(&i->a)) return false;
   if (!op_valid_for_thrend(&i->m)) return false;

   if (i->conflicts & CONFLICT_UNIF) return false;
   return true;
}

static bool can_add_thrsw(const GLSL_SCHEDULER_STATE_T *sched, uint32_t time, bool lthrsw, bool thrend)
{
   if (time < 3) return false;

   /* Check that this is not the delay slot of another thrsw */
   for (unsigned i=5; i>3; i--) {
      int instr = time > i ? time - i : 0;
      if (sched->instructions[instr].sigbits & SIGBIT_THRSW) return false;
   }

   const INSTR_T *i0 = &sched->instructions[time-3];
   const INSTR_T *i1 = &sched->instructions[time-2];
   const INSTR_T *i2 = &sched->instructions[time-1];

   if (!can_add_sigbits(i0->sigbits, SIGBIT_THRSW)) return false;

   if (lthrsw && !can_add_sigbits(i1->sigbits, SIGBIT_THRSW)) return false;

   if (thrend) {
      if (!instr_valid_for_thrend(i0)) return false;
      if (!instr_valid_for_thrend(i1)) return false;
      if (!instr_valid_for_thrend(i2)) return false;
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

   if (thrend && (sched->shader_flavour == BINARY_SHADER_VERTEX ||
                  sched->shader_flavour == BINARY_SHADER_COORDINATE) )
   {
      /* nothing is allowed to happen after thrend in vertex shaders */
      /* We already waited for root->io_timestamp */
      time += 3;
      if (time >= MAX_INSTRUCTIONS)
         backend_error("Too many instructions");
   }

   while (true) {
      if (can_add_thrsw(sched, time, lthrsw, thrend)) break;

      time++;
      if (time >= MAX_INSTRUCTIONS)
         backend_error("Too many instructions");
   }

   sched->instructions[time-3].sigbits |= SIGBIT_THRSW;
   if (lthrsw)
      sched->instructions[time-2].sigbits |= SIGBIT_THRSW;

   uint32_t new_thread_start = time--;

   /* Make sure writes to accumulators don't get scheduled *before* the thrsw */
   for (int i = 0; i < 6; i++)
      invalidate_register(sched, REG_R(i), new_thread_start);

   return time;
}

/* Insert code to calculate the varying value.
 * The dependencies determine where the "ldvary" may be placed. The return value
 * is the first instruction that can read from the fadd result.  */
static uint32_t fit_varying(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t output,
                            uint32_t temp_reg, uint32_t w_reg, uint32_t varying_id, uint32_t varying_type)
{
   assert((varying_type == VARYING_FLAT                                ) ||
          (varying_type == VARYING_LINE_COORD && w_reg == REG_UNDECIDED) ||
          (varying_type == VARYING_DEFAULT    && w_reg != REG_UNDECIDED)    );

   GLSL_OP_T v_ops[3][2] = {
      { { true, 16|1<<2|1, ~0u, ~0u, temp_reg, 0, REG_R(3),    w_reg, 0, 0},   /* Default: fmul temp_reg, r3, w_reg  */
        { true,  0|1<<2|1, ~0u, ~0u, output,   1, temp_reg, REG_R(5), 0, 0} }, /*          fadd output, temp_reg, r5 */
      { { true,        15,   7, ~0u, temp_reg, 0, REG_R(3),        0, 0, 0},   /* Lines:   mov temp_reg, r3          */
        { true,  0|1<<2|1, ~0u, ~0u, output,   1, temp_reg, REG_R(5), 0, 0} }, /*          fadd output, temp_reg, r5 */
      { { false, 0,                                                       },   /* Flat:    nop                       */
        { true,       182, ~0u, ~0u, output,   0, REG_R(5), REG_R(5), 0, 0} } }; /*        mov output, r5            */
   GLSL_OP_T *v = v_ops[varying_type-1];

   uint32_t conflicts[2] = { 0, 0 };
   uint32_t read[2][2] = { { REG_UNDECIDED, REG_UNDECIDED }, { REG_UNDECIDED, REG_UNDECIDED} };
   int      n_reads[2] = { 0, 0 };
   for (int i=0; i<2; i++) {
      if (v[i].used) {
         conflicts[i] = get_output_conflicts(v[i].output);
         if (IS_RF(v[i].input_a)) read[i][n_reads[i]++] = v[i].input_a;
         if (IS_RF(v[i].input_b)) read[i][n_reads[i]++] = v[i].input_b;
      }
   }

   INSTR_T *ci;
   while (true) {
      if (time + 2 >= MAX_INSTRUCTIONS)
         backend_error("Too many instructions");

      ci = &sched->instructions[time];

      bool suitable = can_add_sigbits(ci[0].sigbits, SIGBIT_LDVARY);
      if (v[0].used) suitable &= can_place_alu(&ci[1], ALU_M, conflicts[0], read[0]);
      if (v[1].used) suitable &= can_place_alu(&ci[2], ALU_A, conflicts[1], read[1]);
      if (suitable) break;

      time++;
   }

   ci[0].sigbits |= SIGBIT_LDVARY;
   ci[0].varying = varying_id;

   for (int i=0; i<2; i++) {
      if (v[i].used) {
         GLSL_OP_T *dest = (i == 0) ? &ci[i+1].m : &ci[i+1].a;
         set_alu(dest, v[i].op, v[i].op1, v[i].op2, v[i].output, v[i].input_a, v[i].input_b,
                       v[i].cond_setf, v[i].swap_mode, v[i].mov_excuse);

         ci[i+1].conflicts |= conflicts[i];
      }
   }

   /* Update our temporary registers. Main code updates inputs/outputs */
   assert(sched->reg[REG_R(3)].user == NULL);
   assert(sched->reg[REG_R(5)].user == NULL);
   assert(sched->reg[temp_reg].user == NULL);
   write_register(sched, NULL, REG_R(3), time);
   write_register(sched, NULL, REG_R(5), time + 1);
   write_register(sched, NULL, temp_reg, time + 1);
   read_register(sched, REG_R(3), time + 1);
   read_register(sched, REG_R(5), time + 2);
   read_register(sched, temp_reg, time + 2);

   return time + 1;
}

static uint32_t fit_imul32(GLSL_SCHEDULER_STATE_T *sched, uint32_t time, uint32_t output, uint32_t left, uint32_t right)
{

   GLSL_OP_T v[2] = {
        { true, 10, ~0u, ~0u, REG_MAGIC_NOP, 0, left, right, 0, 0},   /* Default: multop -,   left, right */
        { true,  3, ~0u, ~0u, output,        0, left, right, 0, 0} }; /*          umul24 out, left, right */

   //conflicts = get_conflicts(output, cond_setf, alu_type, op, op1, op2, unif);
   uint32_t conflicts[2] = { 0, 0 };
   uint32_t read[2][2] = { { REG_UNDECIDED, REG_UNDECIDED }, { REG_UNDECIDED, REG_UNDECIDED} };
   int n_reads[2] = { 0, 0 };
   for (int i=0; i<2; i++) {
      if (v[i].used) {
         conflicts[i] = get_output_conflicts(v[i].output);
         if (IS_RF(v[i].input_a)) read[i][n_reads[i]++] = v[i].input_a;
         if (IS_RF(v[i].input_b)) read[i][n_reads[i]++] = v[i].input_b;
      }
   }

   INSTR_T *ci;
   while (true) {
      if (time + 1 >= MAX_INSTRUCTIONS)
         backend_error("Too many instructions");

      ci = &sched->instructions[time];
      bool suitable = true;
      for (int i=0; i<2; i++)
         suitable &= can_place_alu(&ci[i], ALU_M, conflicts[i], read[i]);

      if (suitable) break;

      time++;
   }

   for (int i=0; i<2; i++) {
      if (v[i].used) {
         set_alu(&ci[i].m, v[i].op, v[i].op1, v[i].op2, v[i].output, v[i].input_a, v[i].input_b,
                           v[i].cond_setf, v[i].swap_mode, v[i].mov_excuse);

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
static uint32_t choose_acc(const GLSL_SCHEDULER_STATE_T *sched, bool insist)
{
   uint32_t oldest = ~0u, oldest_r = REG_UNDECIDED;

   /* Choose a free register, failing that one that wasn't used recently */
   for (int i = 0; i < 5; i++) {
      if ( sched->reg[REG_R(i)].user == NULL ) return REG_R(i);

      if (sched->reg[REG_R(i)].written < oldest) {
         oldest = sched->reg[REG_R(i)].written;
         oldest_r = REG_R(i);
      }
   }
   assert(oldest_r != REG_UNDECIDED);
   if (oldest + 4 < sched->current_instruction) return oldest_r;

   /* If we've insisted, then return the least recently used */
   if (insist) return oldest_r;
   else return REG_UNDECIDED;
}

static uint32_t choose_regfile(const GLSL_SCHEDULER_STATE_T *sched)
{
   uint32_t regfile_max = REGFILE_MAX / sched->threadability;
   uint32_t best_time = ~0u;
   uint32_t best = REG_UNDECIDED;
   for (unsigned i = 0; i < regfile_max; i++)
   {
      if (sched->register_blackout & (1ull << i)) continue;

      if (sched->reg[REG_RF(i)].user == NULL) {
         if (sched->reg[REG_RF(i)].available < best_time) {
            best_time = sched->reg[REG_RF(i)].available;
            best = REG_RF(i);
         }
      }
      if (best_time <= sched->current_instruction) break;
   }

   if (best == REG_UNDECIDED) backend_error("No regfile space");

   return best;
}

/*
   Move a value in a register into a different register. We update the register
   tracking structures so that everything is consistent in the new state
*/
static void move_to_register(GLSL_SCHEDULER_STATE_T *sched, uint32_t reg,
                             uint32_t output, uint32_t mov_excuse)
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
   mov_timestamp = MAX(mov_timestamp, sched->reg[output].available);
   mov_timestamp = fit_mov(sched, mov_timestamp, output, reg, UNIF_NONE, SETF_NONE, mov_excuse);
   uint32_t delay = get_output_delay(output);

   write_register(sched, node, output, mov_timestamp + delay);

   sched->reg[reg].user = NULL;
   read_register(sched, reg, mov_timestamp);
}

static void move_result(GLSL_SCHEDULER_STATE_T *sched, uint32_t reg, uint32_t mov_excuse)
{
   if (sched->reg[reg].user == NULL) return;    /* Nothing to move */
   move_to_register(sched, reg, choose_regfile(sched), mov_excuse);
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
      switch (node->sigbits) {
      case SIGBIT_LDUNIF: return REG_R(5);
      case SIGBIT_LDTMU:  return REG_R(4);
      case SIGBIT_LDTLB:  return REG_R(3);
      case SIGBIT_LDTLBU: return REG_R(3);
      case SIGBIT_LDVPM:  return REG_R(3);
      default:            return REG_MAGIC_NOP; /* Other SIGS have no output */
      }
   }

   switch (node->magic_write) {
      /* Node genuinely has no output */
      case REG_MAGIC_NOP:  return REG_MAGIC_NOP;
      /* Output to the SFU comes back in r4 */
      case REG_MAGIC_RECIP:
      case REG_MAGIC_RSQRT:
      case REG_MAGIC_LOG:
      case REG_MAGIC_EXP:
      case REG_MAGIC_SIN:  return REG_R(4);
      /* These are shader output so the results aren't returned */
      case REG_MAGIC_TMU:
      case REG_MAGIC_TMUL:
      case REG_MAGIC_TMUA:
      case REG_MAGIC_TMUAU:
      case REG_MAGIC_TMUD:
#if V3D_HAS_NEW_TMU_CFG
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
#endif
      case REG_MAGIC_TLBU:
      case REG_MAGIC_TLB:
      case REG_MAGIC_VPM:
      case REG_MAGIC_VPMU: return REG_MAGIC_NOP;
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
      timestamp = MAX(timestamp, sched->reg[REG_FLAG_B].available);
      timestamp = MAX(timestamp, sched->reg[node->reg].written);
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

static void schedule_node(GLSL_SCHEDULER_STATE_T *sched, Backflow *node)
{
   uint32_t temp_reg = REG_UNDECIDED;
   uint32_t input_a  = REG_UNDECIDED;
   uint32_t input_b  = REG_UNDECIDED;
   uint32_t flag_reg = REG_UNDECIDED;
   BackflowChainNode *chain_node;

   /* Allow to bubble up a bit */
   if (sched->current_instruction > SCHEDULER_LOOKBACK)
      sched->current_instruction -= SCHEDULER_LOOKBACK;
   else
      sched->current_instruction = 0;

   /* Store copies of things. Some so we can modify, others just for kicks */
   uint32_t waddr = node->magic_write;
   uint32_t cond_setf = node->cond_setf;

   uint64_t unif = node_get_unif(node);
   /* If the result comes to a fixed register (sig, sfu, etc) fill in which */
   node->reg = find_output_reg(node);

   Backflow *flag_dep     = node->dependencies[0];  /* cond_setf determines usage */
   Backflow *output_dep   = node->dependencies[3];  /* NULL if not overwriting another node */
   Backflow *input_dep[2];
   input_dep[0] = node->dependencies[1];
   input_dep[1] = node->dependencies[2];

#ifndef NDEBUG
   for (int i=0; i<BACKFLOW_DEP_COUNT; i++) validate_reg(sched, node->dependencies[i]);
#endif

   /* If we need flags as inputs(/outputs) sort them out first */
   if (flag_dep != NULL) {
      bool writes_flag_a = cs_is_updt(cond_setf);
      flagify(sched, flag_dep, writes_flag_a);
      flag_reg = dereference(sched, flag_dep);

      /* TODO: Work out whether we need this */
      if (flag_dep->remaining_dependents != 0 && writes_flag_a) {
         move_result(sched, flag_reg, MOV_EXCUSE_COPY_ON_WRITE);
      }

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
      if (cur_reg == REG_R(5)) move_result(sched, cur_reg, MOV_EXCUSE_R5_WRITE);
      if (IS_RF(cur_reg) && ((1ull << (cur_reg - REG_RF(0))) & sched->register_blackout) != 0)
         move_result(sched, cur_reg, MOV_EXCUSE_COPY_ON_WRITE);

      node->reg = dereference(sched, output_dep);

      /* TODO: This isn't strictly required because it has the same effect as
       * forgetting about writing and just moving the value out of the way. */
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

   /*
   Special register allocation for VARYING.
   The boilerplate code requires a temporary register (r0-r4) and also makes use of r3 and r5.

   Note that it *is* safe for the temporary and output registers to be the same. It's also
   safe to use r3 as the temporary register (though it makes stacking less efficient).
   So we're sloppy about making sure the same thing doesn't get allocated for two different
   purposes, but it won't actually break anything.
   */
   if (node->type == SPECIAL_VARYING) {
      move_result(sched, REG_R(5), MOV_EXCUSE_OUT_OF_THE_WAY);
      move_result(sched, REG_R(3), MOV_EXCUSE_OUT_OF_THE_WAY);
      temp_reg = choose_acc(sched, true);   /* Insist so that we can stack */
      move_result(sched, temp_reg, MOV_EXCUSE_OUT_OF_THE_WAY);
   }

   /* Accumulators aren't preserved over a thrsw */
   if (node->type == SPECIAL_THRSW)
      for (int i = 0; i < 6; i++) move_result(sched, REG_R(i), MOV_EXCUSE_THRSW);

   /* Register allocation. If not needed then REG_MAGIC_NOP should already be set */
   if (node->reg == REG_UNDECIDED) {
      /* Try an accumulator first then fallback to regfile */
      if ( !(node->type == ALU_A && node->op == 188) ) {
         if (node->remaining_dependents == 0) node->reg = REG_MAGIC_NOP;
         else node->reg = choose_acc(sched, false);
      }
      if (node->reg == REG_UNDECIDED) node->reg = choose_regfile(sched);
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

   uint32_t timestamp, delay;
   timestamp = sched->current_instruction;

   if (node->type == SPECIAL_VARYING) {
      timestamp = MAX(timestamp, sched->reg[REG_R(3)].available);
      timestamp = MAX(timestamp, sched->reg[temp_reg].available-1);
      timestamp = MAX(timestamp, sched->reg[REG_R(5)].available-1);
   }
   if (node->type == SPECIAL_THRSW) {
      /* offset==1 because timestamp wants to be *after* the mov, not on the same line. */
      for (int i=0; i<6; i++) timestamp = MAX(timestamp, sched->reg[REG_R(i)].available+1);
      timestamp = MAX(timestamp, sched->reg[REG_RTOP].available + 1);
   }

   /* Wait until the inputs we need to read have been written */
   if (flag_reg  != REG_UNDECIDED) timestamp = MAX(timestamp, sched->reg[flag_reg ].written);
   if (input_a   != REG_UNDECIDED) timestamp = MAX(timestamp, sched->reg[input_a  ].written);
   if (input_b   != REG_UNDECIDED) timestamp = MAX(timestamp, sched->reg[input_b  ].written);
   /* Wait until any register we'll write to has been read for the last time */
   if (node->reg != REG_MAGIC_NOP) timestamp = MAX(timestamp, sched->reg[node->reg].available);
   if (cs_is_push(cond_setf)) timestamp = MAX(timestamp, sched->reg[REG_FLAG_B].available);
   /* Wait for our iodependencies */
   LIST_FOR_EACH(chain_node, &node->io_dependencies, l)
      timestamp = MAX(timestamp, chain_node->ptr->io_timestamp);

   switch (node->type)
   {
   case SIG:
      timestamp = fit_sig(sched, timestamp, node->sigbits, unif);
      delay = 1;
      break;
   case SPECIAL_THRSW:
      timestamp = fit_thrsw(sched, timestamp, false);
      delay = 0;
      break;
   case SPECIAL_VARYING:
      /* For a varying input_a contains our w value */
      /* TODO: This puts the iotimestamp 1 instruction too late lest input_a should be marked free too early */
      /* TODO: There's no reason we can't load a unif here, except that we don't track conflicts for it */
      assert(node->unif == UNIF_NONE);
      timestamp = fit_varying(sched, timestamp, waddr, temp_reg, input_a, node->varying, node->op);
      delay = 1 + get_output_delay(waddr);
      break;
   case SPECIAL_IMUL32:
      timestamp = fit_imul32(sched, timestamp, waddr, input_a, input_b);
      delay = get_output_delay(waddr);
      break;
   case ALU_A:
   case ALU_A_SWAP0:
   case ALU_A_SWAP1:
   case ALU_M:
      timestamp = fit_alu(sched, timestamp, node->type,
                          node->op, node->op1, node->op2, waddr,
                          input_a, input_b, unif, cond_setf, MOV_EXCUSE_NONE);
      delay = get_output_delay(waddr);
      break;
   case ALU_MOV:
      timestamp = fit_mov(sched, timestamp, waddr, input_a, unif, cond_setf, MOV_EXCUSE_IR);
      delay = get_output_delay(waddr);
      break;
   case ALU_FMOV:
      timestamp = fit_multi(sched, timestamp,
                            128|node->op1<<4|node->op2<<2|node->op2, ~0u, ~0u,   /* fmin */
                            14, node->op2, ~0u,                                  /* fmov */
                            waddr, input_a, unif, cond_setf, MOV_EXCUSE_IR);
      delay = get_output_delay(waddr);
      break;
   case SPECIAL_VOID:
      /* timestamp = timestamp; */
      delay = 0;
      break;
   default:
      UNREACHABLE();
   }

   sched->current_instruction = timestamp;
   node->io_timestamp = timestamp + 1;  /* We must go *after* the actual timestamp */

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
static void print_shader_stats(const GLSL_SCHEDULER_STATE_T *sched, int n_instrs) {
   int i;
   int a_valid = 0, a_nop = 0, a_mov = 0, m_valid = 0, m_nop = 0, m_mov = 0;
   int unifs = 0;
   bool tmu = false;
   int reg_used = REGFILE_MAX;
   int n_mov_excuses[2][MOV_EXCUSE_COUNT] = { { 0, }, {0, } };

   for (i=0; i<n_instrs; i++) {
      if (sched->instructions[i].a.used) a_valid++;
      else a_nop++;
      if (sched->instructions[i].m.used) m_valid++;
      else m_nop++;

      if (sched->instructions[i].a.mov_excuse != MOV_EXCUSE_NONE) {
         n_mov_excuses[0][sched->instructions[i].a.mov_excuse] ++;
         a_mov++;
      }
      if (sched->instructions[i].m.mov_excuse != MOV_EXCUSE_NONE) {
         n_mov_excuses[1][sched->instructions[i].m.mov_excuse] ++;
         m_mov++;
      }

      if (sched->instructions[i].unif != UNIF_NONE) unifs++;
      if (sched->instructions[i].sigbits & SIGBIT_LDTMU) tmu = true;
   }
   a_valid -= a_mov;
   m_valid -= m_mov;
   for (i=REGFILE_MAX-1; i>=0; i--) {
      if (sched->reg[REG_RF(i)].available != 0) break;
      reg_used = i;
   }

   printf("Shader stats:\n");
   printf("   Shader Type: %s, N threads: %d/%d\n",
          glsl_binary_shader_flavour_name(sched->shader_flavour),
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
   for (i=1; i<MOV_EXCUSE_COUNT; i++) {
      if (n_mov_excuses[0][i] > 0 || n_mov_excuses[1][i] > 0) {
         printf("      %s\t   %3d %3d\n", mov_excuse_strings[i], n_mov_excuses[0][i], n_mov_excuses[1][i]);
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
   /* TODO: Could do better here */
   if (node->io_dependencies.count != 0) return false;
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
      int p = 0;
      for (int i=0; i<REGFILE_MAX; i++) {
         if (sched->reg[REG_RF(i)].user != NULL) p++;
      }
      /* If this exceeds a threshold (arbitrarily set at 50) then search for
       * nodes that free up locations and schedule them next.                */
      if (p > 50) {
         for (int i=0; i<REGFILE_MAX; i++) {
            if (sched->reg[REG_RF(i)].user) {
               Backflow *node = sched->reg[REG_RF(i)].user;
               for (BackflowChainNode *n = node->data_dependents.head; n; n=n->l.next) {
                  /* n->ptr may not be needed for the shader, so check if anything will use it */
                  if (n->ptr->phase == PHASE_UNVISITED && n->ptr->remaining_dependents > 0) {
                     /* If we will free the location used by 'node' and don't need to
                      * calculate and other dependencies then scheduling this node will
                      * not consume any more register file space. If it depends on another
                      * stored value then we'll gain than location back                    */
                     if(node->remaining_dependents == 1 && all_deps_ready(n->ptr))
                     {
                        stack_push(stack, n->ptr);
                        break;
                     }
                  }
               }
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

#if !V3D_HAS_LDVPM
static void break_alias(GLSL_SCHEDULER_STATE_T *sched) {
   if (sched->reg[REG_RF(0)].user != NULL && sched->reg[REG_RF(1)].user != NULL)
   {
      uint32_t time = fit_mov(sched, 0, REG_RF(0), REG_RF(0), UNIF_NONE,
                              SETF_NONE, MOV_EXCUSE_OUT_OF_THE_WAY);
      vcos_unused_in_release(time);
      assert(time == 0);
   }
}
#endif

GENERATED_SHADER_T *glsl_backend(BackflowChain               *iodeps,
                                 glsl_binary_shader_flavour_t flavour,
                                 uint32_t                     thrsw_count,
                                 uint32_t                     threadability,
                                 BackflowPriorityQueue       *sched_queue,
                                 RegList                     *presched,
                                 RegList                     *outputs,
                                 int                          branch_idx,
                                 int                          blackout,
                                 bool                         last,
                                 bool                         lthrsw)
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
   sched->register_blackout = gfx_mask64(blackout);
   /* If data starts in the regfile record that now */
   for ( ; presched; presched = presched->next)
      prescheduled(sched, presched->node, presched->reg);

   /* Remove any register that we output to from the blackout */
   for (RegList *l = outputs; l!=NULL; l=l->next) {
      uint64_t reg_bit = (1ull << (l->reg - REG_RF(0)));
      sched->register_blackout &= ~reg_bit;
   }

#if !V3D_HAS_LDVPM
   /* Break any potential aliasing between w and w_c immediately */
   break_alias(sched);
#endif

   sched->lthrsw          = lthrsw;
   sched->thrsw_remaining = thrsw_count;
   sched->threadability   = threadability;

   /* TLB access is not allowed in the first instruction */
   sched->instructions[0].conflicts = CONFLICT_TLB;

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

   sched->current_instruction = 0;
   uint32_t last_instr_idx = 0;
   if (!last) {
      /* Restore the complete blackout here. A single output value that is being
       * placed in multiple registers goes wrong otherwise. It will be moved out
       * of its first register to the second, and the first could be reused. */
      for (RegList *r = outputs; r; r=r->next) {
         uint64_t reg_bit = (1ull << (r->reg - REG_RF(0)));
         sched->register_blackout |= reg_bit;
      }

      /* Place the outputs in their assigned registers */
      for (RegList *r = outputs; r; r=r->next) {
         if (r->node->reg == r->reg) continue;

         if (sched->reg[r->reg].user != NULL) {
            move_result(sched, r->reg, MOV_EXCUSE_OUT_OF_THE_WAY);
         }
         move_to_register(sched, r->node->reg, r->reg, MOV_EXCUSE_OUTPUT);
         /* The write is done one instruction before the data will be available */
         last_instr_idx = MAX(last_instr_idx, sched->reg[r->reg].available-1);
      }

      if (branch_idx != -1) {
         RegList *r = outputs;
         for (int i=0; i<branch_idx; i++) r = r->next;
         flagify(sched, r->node, true);      /* Need this in flag a */
         last_instr_idx = MAX(last_instr_idx, sched->reg[REG_FLAG_A].available-1);
      }
   }

   for (BackflowChainNode *n=iodeps->head; n; n=n->l.next)
      last_instr_idx = MAX(last_instr_idx, n->ptr->io_timestamp);

   if (last) {
      assert(outputs == NULL);
      last_instr_idx = fit_thrsw(sched, last_instr_idx, true);
   }

   uint32_t instr_count = last_instr_idx + 1;
   assert(instr_count <= MAX_INSTRUCTIONS);

#ifndef NDEBUG
   /* Ensure that all registers are free at the end, otherwise we've leaked */
   /* First clean up the outputs */
   for (RegList *r = outputs; r; r=r->next) {
      dereference(sched, r->node);
      clean_register(sched, r->node->reg, instr_count-1);
   }
   for (int i=0; i<REG_MAX; i++) assert(sched->reg[i].user == NULL);
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
