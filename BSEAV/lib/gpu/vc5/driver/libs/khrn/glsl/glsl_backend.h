/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "glsl_backflow.h"
#include "glsl_binary_shader.h"
#include "glsl_backend_reg.h"

#define MAX_INSTRUCTIONS 20000
#define MAX_VARYINGS     128
#define MAX_STACK        4096

/* Legacy magic uniform encodings */
#define UNIF_PAIR(a,b) ((uint64_t)(a) | (uint64_t)(b) << (uint64_t)32)
#define UNIF_NONE      ((uint64_t)0)

#define VARYING_ID_HW_0 (V3D_MAX_VARYING_COMPONENTS)
#define VARYING_ID_HW_1 (V3D_MAX_VARYING_COMPONENTS + 1)

typedef enum {
   CONFLICT_NONE           = 0,
   CONFLICT_UNIF           = (1<<0),
   CONFLICT_ADDFLAG        = (1<<1),
   CONFLICT_MULFLAG        = (1<<2),
   CONFLICT_SETFLAG        = (1<<3),
   CONFLICT_TMU_W          = (1<<4),
   CONFLICT_TMU_R          = (1<<5),
   CONFLICT_TMU_C          = (1<<6),
   CONFLICT_TLB_R          = (1<<7),
   CONFLICT_TLB_W          = (1<<8),
   CONFLICT_VPM            = (1<<9),
   CONFLICT_SFU            = (1<<10),
   CONFLICT_FIRST_INSTR    = (1<<11),
   CONFLICT_THRSW_SIGNAL   = (1<<12),  // instruction signalling thrsw
   CONFLICT_THRSW_DELAY_1  = (1<<13),  // thrsw delay slot 1
   CONFLICT_THRSW_DELAY_2  = (1<<14),  // thrsw delay slot 2
   CONFLICT_POST_THRSW     = (1<<15),  // first instruction after thrsw
   CONFLICT_VARY           = (1<<16),
   CONFLICT_MSF            = (1<<17),
   CONFLICT_UNIFRF         = (1<<18),
} QBEConflict;

#define PHASE_UNVISITED 0
#define PHASE_STACKED   1
#define PHASE_COMPLETE  2

static inline unsigned get_max_regfile(unsigned threading) {
   return gfx_umin((V3D_VER_AT_LEAST(4,0,2,0) ? 128u : 64u) / threading, 64u);
}

#define SETF_NONE  0
#define SETF_PUSHZ 1
#define SETF_PUSHN 2
#define SETF_PUSHC 3
#define SETF_ANDZ  4
#define SETF_ANDNZ 5
#define SETF_NORNZ 6
#define SETF_NORZ  7
#define SETF_ANDN  8
#define SETF_ANDNN 9
#define SETF_NORNN 10
#define SETF_NORN  11
#define SETF_ANDC  12
#define SETF_ANDNC 13
#define SETF_NORNC 14
#define SETF_NORC  15
#define COND_IFA     16   /* 16 & 3 == 0 (which gives the instruction encoding for this sort of cond) */
#define COND_IFB     17
#define COND_IFNA    18
#define COND_IFNB    19
#define COND_IFFLAG  20 /* Pseudo-condition - Backend will convert into either COND_IFA or COND_IFB */
#define COND_IFNFLAG 21
static inline bool cs_is_none(uint32_t cs) { return cs==0; }
static inline bool cs_is_push(uint32_t cs) { return cs>=1 && cs<=3; }
static inline bool cs_is_updt(uint32_t cs) { return cs>=4 && cs<=15; }
static inline bool cs_is_setf(uint32_t cs) { return cs_is_push(cs) || cs_is_updt(cs); }
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
   backend_reg output;
   backend_reg input_a;
   backend_reg input_b;
   uint32_t cond_setf;
   MOV_EXCUSE mov_excuse; /* For debugging optimisations - if there's a mov here then why? */
} GLSL_OP_T;

typedef struct _INSTR_T
{
   QBEConflict       conflicts;
   v3d_qpu_sigbits_t sigbits;
   uint64_t unif;
   uint32_t varying;

   GLSL_OP_T a;
   GLSL_OP_T m;

#if V3D_VER_AT_LEAST(4,0,2,0)
   backend_reg sig_waddr;
#endif

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
   uint32_t  instruction_count;
   uint64_t *instructions;
   uint32_t  unif_count;
   uint32_t *unifs;
   uint32_t  varying_count;
   uint32_t  varyings[MAX_VARYINGS];
} GENERATED_SHADER_T;

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
                                        Backflow *branch_cond,
                                        int blackout,
                                        bool last,
                                        bool lthrsw);
