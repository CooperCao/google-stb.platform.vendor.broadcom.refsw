/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "libs/core/v3d/v3d_ver.h"
#include "gmem.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum v3d_barrier_flags
{
   /* Default value */
   V3D_BARRIER_NO_ACCESS            = 0,

   /* V3D read-only blocks */
   V3D_BARRIER_CLE_CL_READ         = 1 << 0,
   V3D_BARRIER_CLE_SHADREC_READ    = 1 << 1,
   V3D_BARRIER_CLE_PRIMIND_READ    = 1 << 2,
   V3D_BARRIER_CLE_DRAWREC_READ    = 1 << 3,
   V3D_BARRIER_VCD_READ            = 1 << 4,
   V3D_BARRIER_QPU_INSTR_READ      = 1 << 5,
   V3D_BARRIER_QPU_UNIF_READ       = 1 << 6,
   V3D_BARRIER_TMU_CONFIG_READ     = 1 << 7,

   /* V3D write-only blocks */
   V3D_BARRIER_PTB_TF_WRITE        = 1 << 8,
   V3D_BARRIER_PTB_TILESTATE_WRITE = 1 << 10,

   /* V3D read-write blocks */
   V3D_BARRIER_PTB_PCF_READ        = 1 << 11,
   V3D_BARRIER_PTB_PCF_WRITE       = 1 << 12,
   V3D_BARRIER_TMU_DATA_READ       = 1 << 13,
   V3D_BARRIER_TMU_DATA_WRITE      = 1 << 14,
   V3D_BARRIER_TLB_IMAGE_READ      = 1 << 15,
   V3D_BARRIER_TLB_IMAGE_WRITE     = 1 << 16,
   V3D_BARRIER_TLB_OQ_READ         = 1 << 17,
   V3D_BARRIER_TLB_OQ_WRITE        = 1 << 18,
   V3D_BARRIER_TFU_READ            = 1 << 19,
   V3D_BARRIER_TFU_WRITE           = 1 << 20,

   /* Access via a non-specific unit attached to the memory */
   V3D_BARRIER_MEMORY_READ         = 1 << 24,
   V3D_BARRIER_MEMORY_WRITE        = 1 << 25,
} v3d_barrier_flags;

#define V3D_BARRIER_CORE_READ (\
   V3D_BARRIER_CLE_CL_READ\
 | V3D_BARRIER_CLE_SHADREC_READ\
 | V3D_BARRIER_CLE_PRIMIND_READ\
 | V3D_BARRIER_CLE_DRAWREC_READ\
 | V3D_BARRIER_VCD_READ\
 | V3D_BARRIER_QPU_INSTR_READ\
 | V3D_BARRIER_QPU_UNIF_READ\
 | V3D_BARRIER_TMU_CONFIG_READ\
 | V3D_BARRIER_PTB_PCF_READ\
 | V3D_BARRIER_TMU_DATA_READ\
 | V3D_BARRIER_TLB_IMAGE_READ\
 | V3D_BARRIER_TLB_OQ_READ)

#define V3D_BARRIER_CORE_WRITE (\
   V3D_BARRIER_PTB_TF_WRITE\
 | V3D_BARRIER_PTB_TILESTATE_WRITE\
 | V3D_BARRIER_PTB_PCF_WRITE\
 | V3D_BARRIER_TMU_DATA_WRITE\
 | V3D_BARRIER_TLB_IMAGE_WRITE\
 | V3D_BARRIER_TLB_OQ_WRITE)

#define V3D_BARRIER_V3D_READ (V3D_BARRIER_CORE_READ | V3D_BARRIER_TFU_READ)
#define V3D_BARRIER_V3D_WRITE (V3D_BARRIER_CORE_WRITE | V3D_BARRIER_TFU_WRITE)

#define V3D_BARRIER_CORE_RW (V3D_BARRIER_CORE_READ | V3D_BARRIER_CORE_WRITE)
#define V3D_BARRIER_V3D_RW (V3D_BARRIER_V3D_READ | V3D_BARRIER_V3D_WRITE)
#define V3D_BARRIER_TFU_RW (V3D_BARRIER_TFU_READ | V3D_BARRIER_TFU_WRITE)
#define V3D_BARRIER_MEMORY_RW (V3D_BARRIER_MEMORY_READ | V3D_BARRIER_MEMORY_WRITE)


typedef enum v3d_cache_ops
{
   V3D_CACHE_OPS_NONE = 0,
   V3D_CACHE_OPS_FORCE_32_BIT  = 1 << 31,

   V3D_CACHE_CLEAR_SIC  = 1 << 0,   // Per-slice read-only instruction cache
   V3D_CACHE_CLEAR_SUC  = 1 << 1,   // Per-slice read-only uniform cache.
   V3D_CACHE_CLEAR_L1TD = 1 << 2,   // Per-TMU read-only data cache.
   V3D_CACHE_CLEAR_L1TC = 1 << 3,   // Per-TMU read-only config cache.
   V3D_CACHE_CLEAR_VCD  = 1 << 4,   // Per-core read-only vertex cache.
#if !V3D_VER_AT_LEAST(3,3,0,0)
   V3D_CACHE_CLEAR_L2C  = 1 << 5,   // Per-core read-only L2 cache.
#endif
   V3D_CACHE_FLUSH_L2T  = 1 << 6,   // Per-core read-write L2T cache.

#if V3D_VER_AT_LEAST(3,3,0,0)
   V3D_CACHE_CLEAN_L1TD = 1 << 7,   // Per-TMU L1 write combiner (from 3.3).
#endif
   V3D_CACHE_CLEAN_L2T  = 1 << 8,   // Per-core read-write L2T cache.

#if !V3D_VER_AT_LEAST(4,0,2,0)
   V3D_CACHE_CLEAR_GCA  = 1 << 9,   // Read-only legacy L3 cache in wrapper.
#endif
   V3D_CACHE_FLUSH_L3C  = 1 << 10,   // Read-write L3 cache.
   V3D_CACHE_CLEAN_L3C  = 1 << 11,
} v3d_cache_ops;

#define V3D_CACHE_FLUSH_CORE (\
      V3D_CACHE_CLEAR_SIC\
   |  V3D_CACHE_CLEAR_SUC\
   |  V3D_CACHE_CLEAR_L1TD\
   |  V3D_CACHE_CLEAR_L1TC\
   |  V3D_CACHE_CLEAR_VCD\
   |  V3D_CACHE_IF_NOT_AT_LEAST_33(V3D_CACHE_CLEAR_L2C)\
   |  V3D_CACHE_FLUSH_L2T)

#define V3D_CACHE_CLEAN_CORE (\
      V3D_CACHE_IF_AT_LEAST_33(V3D_CACHE_CLEAN_L1TD)\
   |  V3D_CACHE_CLEAN_L2T)

#define V3D_CACHE_FLUSH_HUB (V3D_CACHE_FLUSH_L3C | V3D_CACHE_IF_NOT_NEW_WRAPPER(V3D_CACHE_CLEAR_GCA))
#define V3D_CACHE_CLEAN_HUB (V3D_CACHE_CLEAN_L3C)
#define V3D_CACHE_FLUSH_ALL (V3D_CACHE_FLUSH_CORE | V3D_CACHE_FLUSH_HUB)
#define V3D_CACHE_CLEAN_ALL (V3D_CACHE_CLEAN_CORE | V3D_CACHE_CLEAN_HUB)

//! Compute set of caches to be cleaned for barrier.
v3d_cache_ops v3d_barrier_cache_cleans(
   v3d_barrier_flags src,
   v3d_barrier_flags dst);

//! Compute set of caches to be flushed/cleared for barrier.
v3d_cache_ops v3d_barrier_cache_flushes(
   v3d_barrier_flags src,
   v3d_barrier_flags dst);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

static inline v3d_barrier_flags operator~(v3d_barrier_flags a)
{
   return (v3d_barrier_flags)(~(int)a);
}

static inline v3d_barrier_flags operator|(v3d_barrier_flags a, v3d_barrier_flags b)
{
   return (v3d_barrier_flags)((int)a | (int)b);
}

static inline v3d_barrier_flags operator|=(v3d_barrier_flags& a, v3d_barrier_flags b)
{
   a = (v3d_barrier_flags)((int)a | (int)b);
   return a;
}

static inline v3d_barrier_flags operator&(v3d_barrier_flags a, v3d_barrier_flags b)
{
   return (v3d_barrier_flags)((int)a & (int)b);
}

static inline v3d_barrier_flags operator&=(v3d_barrier_flags& a, v3d_barrier_flags b)
{
   a = (v3d_barrier_flags)((int)a & (int)b);
   return a;
}

static inline v3d_cache_ops operator~(v3d_cache_ops a)
{
   return (v3d_cache_ops)~(int)a;
}

static inline v3d_cache_ops operator|(v3d_cache_ops a, v3d_cache_ops b)
{
   return (v3d_cache_ops)((int)a | (int)b);
}

static inline v3d_cache_ops operator|=(v3d_cache_ops& a, v3d_cache_ops b)
{
   a = (v3d_cache_ops)((int)a | (int)b);
   return a;
}

static inline v3d_cache_ops operator&(v3d_cache_ops a, v3d_cache_ops b)
{
   return (v3d_cache_ops)((int)a & (int)b);
}

static inline v3d_cache_ops operator&=(v3d_cache_ops& a, v3d_cache_ops b)
{
   a = (v3d_cache_ops)((int)a & (int)b);
   return a;
}

#endif

#if V3D_VER_AT_LEAST(3,3,0,0)
#define V3D_CACHE_IF_AT_LEAST_33(x) x
#else
#define V3D_CACHE_IF_AT_LEAST_33(x) V3D_CACHE_OPS_NONE
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)
#define V3D_CACHE_IF_NOT_AT_LEAST_33(x) x
#else
#define V3D_CACHE_IF_NOT_AT_LEAST_33(x) V3D_CACHE_OPS_NONE
#endif

#if !V3D_VER_AT_LEAST(4,0,2,0)
#define V3D_CACHE_IF_NOT_NEW_WRAPPER(x) x
#else
#define V3D_CACHE_IF_NOT_NEW_WRAPPER(x) V3D_CACHE_OPS_NONE
#endif
