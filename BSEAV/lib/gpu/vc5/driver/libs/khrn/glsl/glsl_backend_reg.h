/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include <stdbool.h>

typedef enum backend_reg
{
   REG_UNDECIDED,

   REG_R0_, REG_R1_, REG_R2_, REG_R3_, REG_R4_, REG_R5_,

   REG_RF0,  REG_RF1,  REG_RF2,  REG_RF3,  REG_RF4,  REG_RF5,  REG_RF6,  REG_RF7,  REG_RF8,  REG_RF9,
   REG_RF10, REG_RF11, REG_RF12, REG_RF13, REG_RF14, REG_RF15, REG_RF16, REG_RF17, REG_RF18, REG_RF19,
   REG_RF20, REG_RF21, REG_RF22, REG_RF23, REG_RF24, REG_RF25, REG_RF26, REG_RF27, REG_RF28, REG_RF29,
   REG_RF30, REG_RF31, REG_RF32, REG_RF33, REG_RF34, REG_RF35, REG_RF36, REG_RF37, REG_RF38, REG_RF39,
   REG_RF40, REG_RF41, REG_RF42, REG_RF43, REG_RF44, REG_RF45, REG_RF46, REG_RF47, REG_RF48, REG_RF49,
   REG_RF50, REG_RF51, REG_RF52, REG_RF53, REG_RF54, REG_RF55, REG_RF56, REG_RF57, REG_RF58, REG_RF59,
   REG_RF60, REG_RF61, REG_RF62, REG_RF63,

   REG_FLAG_A,
   REG_FLAG_B,
   REG_RTOP,

#define REG_MAX (REG_RTOP + 1)

   // Don't enumerate REG_MAGIC_R0 to R5, just write to R0 to R5 (that includes R5QUAD).
   REG_MAGIC_BASE       = 0x100,
   REG_MAGIC_NOP        = REG_MAGIC_BASE + 6,
   REG_MAGIC_TLB        = REG_MAGIC_BASE + 7,
   REG_MAGIC_TLBU       = REG_MAGIC_BASE + 8,
#if V3D_VER_AT_LEAST(4,1,34,0)
   REG_MAGIC_UNIFA      = REG_MAGIC_BASE + 9,
#else
   REG_MAGIC_TMU        = REG_MAGIC_BASE + 9,
   REG_MAGIC_TMUL       = REG_MAGIC_BASE + 10,
#endif
   REG_MAGIC_TMUD       = REG_MAGIC_BASE + 11,
   REG_MAGIC_TMUA       = REG_MAGIC_BASE + 12,
   REG_MAGIC_TMUAU      = REG_MAGIC_BASE + 13,
#if !V3D_VER_AT_LEAST(4,1,34,0)
   REG_MAGIC_VPM        = REG_MAGIC_BASE + 14,
   REG_MAGIC_VPMU       = REG_MAGIC_BASE + 15,
#endif
   REG_MAGIC_SYNC       = REG_MAGIC_BASE + 16,
   REG_MAGIC_SYNCU      = REG_MAGIC_BASE + 17,
#if V3D_VER_AT_LEAST(4,2,13,0)
   REG_MAGIC_SYNCB      = REG_MAGIC_BASE + 18,
#endif
#if !V3D_HAS_NO_SFU_MAGIC
   REG_MAGIC_RECIP      = REG_MAGIC_BASE + 19,
   REG_MAGIC_RSQRT      = REG_MAGIC_BASE + 20,
   REG_MAGIC_EXP        = REG_MAGIC_BASE + 21,
   REG_MAGIC_LOG        = REG_MAGIC_BASE + 22,
   REG_MAGIC_SIN        = REG_MAGIC_BASE + 23,
   REG_MAGIC_RSQRT2     = REG_MAGIC_BASE + 24,
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
   REG_MAGIC_TMUC       = REG_MAGIC_BASE + 32,
   REG_MAGIC_TMUS       = REG_MAGIC_BASE + 33,
   REG_MAGIC_TMUT       = REG_MAGIC_BASE + 34,
   REG_MAGIC_TMUR       = REG_MAGIC_BASE + 35,
   REG_MAGIC_TMUI       = REG_MAGIC_BASE + 36,
   REG_MAGIC_TMUB       = REG_MAGIC_BASE + 37,
   REG_MAGIC_TMUDREF    = REG_MAGIC_BASE + 38,
   REG_MAGIC_TMUOFF     = REG_MAGIC_BASE + 39,
   REG_MAGIC_TMUSCM     = REG_MAGIC_BASE + 40,
   REG_MAGIC_TMUSF      = REG_MAGIC_BASE + 41,
   REG_MAGIC_TMUSLOD    = REG_MAGIC_BASE + 42,
   REG_MAGIC_TMUHS      = REG_MAGIC_BASE + 43,
   REG_MAGIC_TMUHSCM    = REG_MAGIC_BASE + 44,
   REG_MAGIC_TMUHSF     = REG_MAGIC_BASE + 45,
   REG_MAGIC_TMUHSLOD   = REG_MAGIC_BASE + 46,

   REG_MAGIC_R5REP      = REG_MAGIC_BASE + 55,
#endif
} backend_reg;

static inline backend_reg REG_R(unsigned n)  { assert(n <= 5);  return (backend_reg)(REG_R0_ + n); }
static inline backend_reg REG_RF(unsigned n) { assert(n <= 63); return (backend_reg)(REG_RF0 + n); }

static inline bool IS_R(backend_reg r)       { return r >= REG_R0_ && r <= REG_R5_; }
static inline bool IS_RF(backend_reg  r)     { return r >= REG_RF0 && r <= REG_RF63; }
static inline bool IS_RRF(backend_reg r)     { return IS_R(r) || IS_RF(r); }
static inline bool IS_FLAG(backend_reg r)    { return r == REG_FLAG_A || r == REG_FLAG_B; }
static inline bool IS_MAGIC(backend_reg r)   { return r >= REG_MAGIC_NOP; }
