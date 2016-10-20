/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_BACKEND_H
#define GLSL_BACKEND_H

#include "libs/core/v3d/v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "glsl_backflow.h"
#include "glsl_binary_shader.h"

#define MAX_INSTRUCTIONS 20000
#define MAX_UNIFORMS 10000
#define MAX_VARYINGS 128
#define MAX_STACK 1024

/* Legacy magic uniform encodings */
#define UNIF_PAIR(a,b) ((uint64_t)(a) | (uint64_t)(b) << (uint64_t)32)
#define UNIF_NONE ((uint64_t)0)

#define VARYING_ID_HW_0 (V3D_MAX_VARYING_COMPONENTS)
#define VARYING_ID_HW_1 (V3D_MAX_VARYING_COMPONENTS + 1)

#define CONFLICT_UNIF       (1<<0)
#define CONFLICT_ADDFLAG    (1<<1)
#define CONFLICT_MULFLAG    (1<<2)
#define CONFLICT_SETFLAG    (1<<3)
#define CONFLICT_TMU_W      (1<<4)
#define CONFLICT_TMU_R      (1<<5)
#define CONFLICT_TMU_C      (1<<6)
#define CONFLICT_TLB_R      (1<<7)
#define CONFLICT_TLB_W      (1<<8)
#define CONFLICT_VPM        (1<<9)
#define CONFLICT_SFU        (1<<10)
#define CONFLICT_POST_THRSW (1<<11)
#define CONFLICT_VARY       (1<<12)
#define CONFLICT_MSF        (1<<13)
/* TMU, VPM write conflicts handled by iodeps */
/* Conflicts between things that write to r3,r4,r5 are handled by reg_user and reg_available */

#define PHASE_UNVISITED 0
#define PHASE_STACKED   1
#define PHASE_COMPLETE  2

#define REG_UNDECIDED 0
#define REG_R(n) (1+n)
#define REG_RF(n) (7+(n))
#if V3D_HAS_LARGE_REGFILE
#define REGFILE_MAX 128
#else
#define REGFILE_MAX 64
#endif
#define REG_SPECIAL(r)  (REG_RF(REGFILE_MAX) + (r))
#define REG_FLAG_A  REG_SPECIAL(0)
#define REG_FLAG_B  REG_SPECIAL(1)
#define REG_R5QUAD  REG_SPECIAL(2)
#define REG_RTOP    REG_SPECIAL(3)
#define REG_MAX     REG_SPECIAL(4)
#define REG_MAGIC(n) (REG_MAX + (n))
#define IS_R(r)     ((r) >= REG_R(0)   && (r) <  REG_R(6))
#define IS_RF(r)    ((r) >= REG_RF(0)  && (r) <  REG_RF(REGFILE_MAX))
#define IS_FLAG(r)  ((r) == REG_FLAG_A || (r) == REG_FLAG_B)
#define IS_MAGIC(r) ((r) >= REG_MAX    && (r) <= REG_MAGIC(46))
#define IS_RRF(r)   (IS_R(r) || IS_RF(r))

static inline unsigned get_max_regfile(unsigned threading) {
   return gfx_umin(REGFILE_MAX / threading, 64u);
}

/* REG_MAGIC(0-5) are the accumulators */
#define REG_MAGIC_NOP     REG_MAGIC(6)
#define REG_MAGIC_TLB     REG_MAGIC(7)
#define REG_MAGIC_TLBU    REG_MAGIC(8)
#if !V3D_HAS_NEW_TMU_CFG
#define REG_MAGIC_TMU     REG_MAGIC(9)
#define REG_MAGIC_TMUL    REG_MAGIC(10)
#endif
#define REG_MAGIC_TMUD    REG_MAGIC(11)
#define REG_MAGIC_TMUA    REG_MAGIC(12)
#define REG_MAGIC_TMUAU   REG_MAGIC(13)
#if !V3D_HAS_LDVPM
#define REG_MAGIC_VPM     REG_MAGIC(14)
#define REG_MAGIC_VPMU    REG_MAGIC(15)
#endif
#define REG_MAGIC_RECIP   REG_MAGIC(19)
#define REG_MAGIC_RSQRT   REG_MAGIC(20)
#define REG_MAGIC_EXP     REG_MAGIC(21)
#define REG_MAGIC_LOG     REG_MAGIC(22)
#define REG_MAGIC_SIN     REG_MAGIC(23)
#define REG_MAGIC_RSQRT2  REG_MAGIC(24)
#if V3D_HAS_NEW_TMU_CFG
#define REG_MAGIC_TMUC    REG_MAGIC(32)
#define REG_MAGIC_TMUS    REG_MAGIC(33)
#define REG_MAGIC_TMUT    REG_MAGIC(34)
#define REG_MAGIC_TMUR    REG_MAGIC(35)
#define REG_MAGIC_TMUI    REG_MAGIC(36)
#define REG_MAGIC_TMUB    REG_MAGIC(37)
#define REG_MAGIC_TMUDREF REG_MAGIC(38)
#define REG_MAGIC_TMUOFF  REG_MAGIC(39)
#define REG_MAGIC_TMUSCM  REG_MAGIC(40)
#define REG_MAGIC_TMUSF   REG_MAGIC(41)
#define REG_MAGIC_TMUSLOD REG_MAGIC(42)
# if V3D_HAS_TMU_PRIORITY
#define REG_MAGIC_TMUHS     REG_MAGIC(43)
#define REG_MAGIC_TMUHSCM   REG_MAGIC(44)
#define REG_MAGIC_TMUHSF    REG_MAGIC(45)
#define REG_MAGIC_TMUHSLOD  REG_MAGIC(46)
# endif
#endif

#if V3D_HAS_LDVPM
#  if V3D_HAS_NEW_TMU_CFG
static const uint32_t sigbit_table[32] =
{
   0,
   SIGBIT_THRSW,
                                           SIGBIT_LDUNIF,
   SIGBIT_THRSW                           |SIGBIT_LDUNIF,
                              SIGBIT_LDTMU,
   SIGBIT_THRSW|              SIGBIT_LDTMU,
                              SIGBIT_LDTMU|SIGBIT_LDUNIF,
   SIGBIT_THRSW|              SIGBIT_LDTMU|SIGBIT_LDUNIF,
                SIGBIT_LDVARY,
   SIGBIT_THRSW|SIGBIT_LDVARY,
                SIGBIT_LDVARY|             SIGBIT_LDUNIF,
   SIGBIT_THRSW|SIGBIT_LDVARY|             SIGBIT_LDUNIF,
   ~0u,
   ~0u,
   SIGBIT_IMMED|SIGBIT_LDVARY,
   SIGBIT_IMMED,
                SIGBIT_LDTLB,
                SIGBIT_LDTLBU,
                                                         SIGBIT_WRTMUC,
   SIGBIT_THRSW|                                         SIGBIT_WRTMUC,
                SIGBIT_LDVARY|                           SIGBIT_WRTMUC,
   SIGBIT_THRSW|SIGBIT_LDVARY|                           SIGBIT_WRTMUC,
   SIGBIT_UCB,
   SIGBIT_ROTATE,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   SIGBIT_IMMED|              SIGBIT_LDTMU,
};
#  else
static const uint32_t sigbit_table[32] =
{
   0,
   SIGBIT_THRSW,
                                           SIGBIT_LDUNIF,
   SIGBIT_THRSW                           |SIGBIT_LDUNIF,
                              SIGBIT_LDTMU,
   SIGBIT_THRSW|              SIGBIT_LDTMU,
                              SIGBIT_LDTMU|SIGBIT_LDUNIF,
   SIGBIT_THRSW|              SIGBIT_LDTMU|SIGBIT_LDUNIF,
                SIGBIT_LDVARY,
   SIGBIT_THRSW|SIGBIT_LDVARY,
                SIGBIT_LDVARY|             SIGBIT_LDUNIF,
   SIGBIT_THRSW|SIGBIT_LDVARY|             SIGBIT_LDUNIF,
   ~0u,
   ~0u,
   SIGBIT_IMMED|SIGBIT_LDVARY,
   SIGBIT_IMMED,
                SIGBIT_LDTLB,
                SIGBIT_LDTLBU,
   ~0u,
   SIGBIT_THRSW,
                SIGBIT_LDVARY,
   SIGBIT_THRSW|SIGBIT_LDVARY,
   SIGBIT_UCB,
   SIGBIT_ROTATE,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   SIGBIT_IMMED|              SIGBIT_LDTMU,
};
#  endif
#else
static const uint32_t sigbit_table[32] =
{
   0,
   SIGBIT_THRSW,
                                           SIGBIT_LDUNIF,
   SIGBIT_THRSW                           |SIGBIT_LDUNIF,
                              SIGBIT_LDTMU,
   SIGBIT_THRSW|              SIGBIT_LDTMU,
                              SIGBIT_LDTMU|SIGBIT_LDUNIF,
   SIGBIT_THRSW|              SIGBIT_LDTMU|SIGBIT_LDUNIF,
                SIGBIT_LDVARY,
   SIGBIT_THRSW|SIGBIT_LDVARY,
                SIGBIT_LDVARY|             SIGBIT_LDUNIF,
   SIGBIT_THRSW|SIGBIT_LDVARY|             SIGBIT_LDUNIF,
                SIGBIT_LDVARY|SIGBIT_LDTMU,
   SIGBIT_THRSW|SIGBIT_LDVARY|SIGBIT_LDTMU,
   SIGBIT_IMMED|SIGBIT_LDVARY,
   SIGBIT_IMMED,
                SIGBIT_LDTLB,
                SIGBIT_LDTLBU,
   ~0u,
   ~0u,
   ~0u,
   ~0u,
   SIGBIT_UCB,
   SIGBIT_ROTATE,
                SIGBIT_LDVPM,
   SIGBIT_THRSW|SIGBIT_LDVPM,
                SIGBIT_LDVPM|              SIGBIT_LDUNIF,
   SIGBIT_THRSW|SIGBIT_LDVPM|              SIGBIT_LDUNIF,
                SIGBIT_LDVPM| SIGBIT_LDTMU,
   SIGBIT_THRSW|SIGBIT_LDVPM| SIGBIT_LDTMU,
   SIGBIT_IMMED|SIGBIT_LDVPM,
   SIGBIT_IMMED|              SIGBIT_LDTMU,
};
#endif

#define SETF_NONE 0
#define SETF_PUSHZ 1
#define SETF_PUSHN 2
#define SETF_PUSHC 3
#define SETF_ANDZ 4
#define SETF_ANDNZ 5
#define SETF_NORNZ 6
#define SETF_NORZ 7
#define SETF_ANDN 8
#define SETF_ANDNN 9
#define SETF_NORNN 10
#define SETF_NORN 11
#define SETF_ANDC 12
#define SETF_ANDNC 13
#define SETF_NORNC 14
#define SETF_NORC 15
#define COND_IFA 16   /* 16 & 3 == 0 (which gives the instruction encoding for this sort of cond) */
#define COND_IFB 17
#define COND_IFNA 18
#define COND_IFNB 19
#define COND_IFFLAG 20 /* Pseudo-condition - Backend will convert into either COND_IFA or COND_IFB */
#define COND_IFNFLAG 21
static inline bool cs_is_none(uint32_t cs) { return cs==0; }
static inline bool cs_is_push(uint32_t cs) { return cs>=1 && cs<=3; }
static inline bool cs_is_updt(uint32_t cs) { return cs>=4 && cs<=15; }
static inline bool cs_is_cond(uint32_t cs) { return cs>=16 && cs<=19; }
static inline bool is_cond_setf(uint32_t cs) { return cs<=19; } /* Any valid cond_setf */

typedef enum
{
   MOV_EXCUSE_NONE                  = 0,
   MOV_EXCUSE_OUTPUT                = 1,
   MOV_EXCUSE_IR                    = 2,
   MOV_EXCUSE_OUT_OF_THE_WAY        = 3,
   MOV_EXCUSE_FLAG_B_OUT_OF_THE_WAY = 4,
   MOV_EXCUSE_THRSW                 = 5,
   MOV_EXCUSE_COPY_ON_WRITE         = 6,
   MOV_EXCUSE_R5_WRITE              = 7,
   MOV_EXCUSE_FLAGIFY               = 8,
   MOV_EXCUSE_COUNT
} MOV_EXCUSE;

extern const char *mov_excuse_strings[];

typedef struct {
   bool used;
   BackflowFlavour op;
   SchedNodeUnpack unpack[2];
   uint32_t output;
   uint32_t input_a;
   uint32_t input_b;
   uint32_t cond_setf;
   MOV_EXCUSE mov_excuse; /* For debugging optimisations - if there's a mov here then why? */
} GLSL_OP_T;

typedef struct _INSTR_T
{
   uint32_t conflicts;
   uint32_t sigbits;
   uint64_t unif;
   uint32_t varying;

   GLSL_OP_T a;
   GLSL_OP_T m;

   uint32_t sig_waddr;

   struct _INSTR_T *alt_mov_i;
} INSTR_T;

typedef struct {
   /* reg_user points to the node that wrote the value to the register.     *
    * The time can be found from the node. reg_available contains the time  *
    * of the last read from the register, which is also the first time at   *
    * which a write is permitted. If reg_user!=NULL then this will probably *
    * require a mov to save the value for future users.                     */
   Backflow *user;

   uint32_t written;
   uint32_t available;
} REG_T;

typedef struct
{
   /* Both unifs and instructions are really 64 bit */
   uint32_t instruction_count;
   uint32_t instructions[2*MAX_INSTRUCTIONS];
   uint32_t unif_count;
   uint32_t unifs[2*MAX_UNIFORMS];
   uint32_t varying_count;
   uint32_t varyings[MAX_VARYINGS];
} GENERATED_SHADER_T;

typedef struct
{
   INSTR_T instructions[MAX_INSTRUCTIONS];

   REG_T reg[REG_MAX];

   uint64_t register_blackout;

   uint32_t threadability;       /* 4, 2 or 1. Restricts the regfile sizes */
   uint32_t thrsw_remaining;
   bool     lthrsw;

   ShaderFlavour shader_flavour;    /* For determining where the final thrsw goes */
   bool bin_mode;                   /* Only uesd for stats printing */
} GLSL_SCHEDULER_STATE_T;

void order_texture_lookups(SchedBlock *block);
bool get_max_threadability(struct tmu_lookup_s *tmu_lookups, int *max_threads);

extern GENERATED_SHADER_T *glsl_backend_schedule(SchedBlock *block,
                                                 RegList *presched,
                                                 RegList *outputs,
                                                 int blackout,
                                                 ShaderFlavour type,
                                                 bool bin_mode,
                                                 int threads,
                                                 bool last,
                                                 bool lthrsw,
                                                 bool sbwaited);

extern GENERATED_SHADER_T *glsl_backend(BackflowChain *iodeps,
                                        ShaderFlavour type,
                                        bool bin_mode,
                                        uint32_t thrsw_count,
                                        uint32_t threadability,
                                        BackflowPriorityQueue *sched_queue,
                                        RegList *presched,
                                        RegList *outputs,
                                        int branch_idx,
                                        int blackout,
                                        bool last,
                                        bool lthrsw);

#endif
