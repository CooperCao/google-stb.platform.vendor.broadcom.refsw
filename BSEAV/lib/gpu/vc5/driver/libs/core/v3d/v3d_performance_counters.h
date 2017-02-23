/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  V3D helpers

FILE DESCRIPTION
Performance counters tables
=============================================================================*/

#ifndef V3D_PERF_COUNTERS_H
#define V3D_PERF_COUNTERS_H

#include "v3d_common.h"

typedef enum {
   /* these should be identical to the hardware performance counters, except where noted */

   /* PFE: relevant stuff in simpenrose.c */
   PCTR_FEP_VALID_PRIMS_NO_PIXELS, /* number of primitives that get to the rasteriser but produce no pixels */
   PCTR_FEP_VALID_PRIMITIVES, /* number of primitives that get to the rasteriser */
   PCTR_FEP_CLIPPED_QUADS, /* number of quads produced by the rasteriser that are entirely infront of the near plane or behind the far plane */
   PCTR_FEP_VALID_QUADS, /* number of quads that are produced by the rasteriser that aren't entirely infront of the near plane or behind the far plane */

   /* TLB: relevant stuff in qpu.c */
   PCTR_TLB_QUADS_FAIL_STENCIL, /* number of quads that entirely fail the stencil test */
   PCTR_TLB_QUADS_FAIL_Z_STENCIL, /* number of quads that entirely fail the stencil and z tests */
   PCTR_TLB_QUADS_PASS_Z_STENCIL, /* number of quads that don't entirely fail the stencil and z tests */
   PCTR_TLB_QUADS_FAIL_COVERAGE, /* number of quads that have zero coverage when read by the QPUs */
   PCTR_TLB_QUADS_PASS_COVERAGE, /* number of quads that don't have zero coverage when read by the QPUSs */
   PCTR_TLB_QUADS_WITH_VALID_PIXELS, /* number of colour quads written by the QPUs */

   /* PTB/PSE: relevant stuff in ptb.c/setup_cm.c */
   PCTR_PTB_PRIMS_FAIL_VIEWPORT, /* number of primitives culled in the PTB because they were entirely outside one of the clip planes */
   PCTR_PTB_PRIMS_NEED_CLIP, /* number of primitives clipped in the PTB because they crossed a guard band (but weren't entirely outside any of the clip planes) */
   PCTR_PTB_PRIMS_REVERSED, /* number of primitives culled in the PTB because they were reverse-facing, had zero area, or had "zero" width/height */

   /* QPU: relevant stuff in qpu.c */
   PCTR_QPU_IDLE_CLOCK_CYCLES, /* HW ONLY. Number of QPU idle cycles. this counts up n per cycle when n QPUs are idle */
   PCTR_QPU_CYCLES_NON_FRAG_SHADING, /* number of cycles (4 per instruction) spent in vertex/coord/user shaders. In the HW, this includes stalls */
   PCTR_QPU_CYCLES_FRAG_SHADING, /* number of cycles (4 per instruction) spent in fragment shaders. In the HW, this includes stalls */
   PCTR_QPU_VALID_INSTUCTION_CYCLES, /* number of valid instructions * 4 */
   PCTR_QPU_CYCLES_STALLED_TMU, /* HW ONLY. Number of cycles spent stalled waiting only on the TMU */
   PCTR_QPU_CYCLES_STALLED_SCOREBRD, /* HW ONLY. Number of cycles spent stalled waiting only on the scoreboard */
   PCTR_QPU_CYCLES_STALLED_VARYINGS, /* HW ONLY. Number of cycles spent stalled waiting only on the varyings fifo */
   /* the way the instruction and uniform caches work is that a cache request is
    * either a hit and data is returned or a miss and no data is returned. a hit
    * is counted for every hit. a miss is counted for the first in a series of
    * misses (ie only one miss is counted even if we have to request 4 times
    * before the data arrives and we get a hit). so the number of instruction
    * cache hits will correspond to the number of instructions executed (which
    * means PCTR_QPUVALINSTR is actually redundant -- woops) and the number of
    * uniform cache hits will correspond to the number of uniforms read
    * (actually this isn't entirely true as the qpus will always try to
    * read 2 uniforms past the end of the stream) */
   PCTR_QPU_INSTR_CACHE_HITS,
   PCTR_QPU_INSTR_CACHE_MISSES, /* HW ONLY */
   PCTR_QPU_UNIFORM_CACHE_HITS, /* might not match HW exactly as threads may end before the HW has time to read 2 uniforms past the end */
   PCTR_QPU_UNIFORM_CACHE_MISSES, /* HW ONLY */

   /* TMU: relevant stuff in texture.c */
   PCTR_TMU_CACHE_ACCESSES, /* number of texture lookups */
   PCTR_TMU_CACHE_MISSES, /* HW ONLY (we could probably get a figure from the sw sim, but it wouldn't match) */

   PCTR_VDW_CYCLES_STALLED_VPM, /* HW ONLY. Number of cycles VDW spent stalled waiting on the VPM */
   PCTR_VCD_CYCLES_STALLED_VPM, /* HW ONLY. Number of cycles VDR spent stalled waiting on the VPM */
   /* the l2 cache is transparent to the clients -- each request always returns
    * data, but is either a hit (data was in the cache) or a miss (data had to
    * be fetched) */
#if V3D_VER_AT_LEAST(3,3,0,0)
   PCTR_BIN_ACTIVE, /* HW ONLY */
   PCTR_RDR_ACTIVE, /* HW ONLY */
#else
   PCTR_L2C_CACHE_HITS, /* HW ONLY */
   PCTR_L2C_CACHE_MISSES, /* HW ONLY */
#endif
   PCTR_L2T_CACHE_HITS, /* HW ONLY */
   PCTR_L2T_CACHE_MISSES, /* HW ONLY */

   PCTR_CYCLE_COUNTER, /* HW ONLY */

   PCTR_QPU_CYCLES_STALLED_NON_FRAG, /* HW ONLY */
   PCTR_QPU_CYCLES_STALLED_FRAG, /* HW ONLY */

   PCTR_PTB_PRIMS_BINNED, /* PTB total primitives binned */

   PCTR_AXI_WRITES_WATCH_0, /* HW ONLY */
   PCTR_AXI_READS_WATCH_0, /* HW ONLY */
   PCTR_AXI_WRITE_STALLS_WATCH_0, /* HW ONLY */
   PCTR_AXI_READ_STALLS_WATCH_0, /* HW ONLY */
   PCTR_AXI_BYTES_W_WATCH_0, /* HW ONLY */
   PCTR_AXI_BYTES_R_WATCH_0, /* HW ONLY */
   PCTR_AXI_WRITES_WATCH_1, /* HW ONLY */
   PCTR_AXI_READS_WATCH_1, /* HW ONLY */
   PCTR_AXI_WRITE_STALLS_WATCH_1, /* HW ONLY */
   PCTR_AXI_READ_STALLS_WATCH_1, /* HW ONLY */
   PCTR_AXI_BYTES_W_WATCH_1, /* HW ONLY */
   PCTR_AXI_BYES_R_WATCH_1, /* HW ONLY */

   PCTR_TLB_PARTIAL_QUADS, /* TLB partial quads written to the colour buffer */

   PCTR_TMU_CONFIG_ACCESSES, /* TMU total config accesses */

   PCTR_L2T_NO_ID_STALL, /* HW ONLY */
   PCTR_L2T_QUEUE_FULL_STALL, /* HW ONLY */
   PCTR_L2T_R_W_CONFLICT_STALL, /* HW ONLY */
   PCTR_TMU_ACTIVE, /* HW ONLY */
   PCTR_TMU_STALL, /* HW ONLY */

#if V3D_VER_AT_LEAST(3,2,1,0)
   PCTR_CLE_ACTIVE, /* HW ONLY */
#if !V3D_VER_AT_LEAST(3,3,0,0)
   PCTR_BIN_ACTIVE, /* HW ONLY */
   PCTR_RDR_ACTIVE, /* HW ONLY */
#endif
#endif

   /*** Simpenrose only. ***/
   PCTR_PTB_TILE_PRIMS_TOTAL, /* total PRIMS * TILES binned */

   PCTR_N
} PCTR_EVENT_T;

typedef struct {
   unsigned int id;
   const char name[32];
   const char desc[100];
} V3D_PERF_COUNTER_T;

VCOS_EXTERN_C_BEGIN

extern V3D_PERF_COUNTER_T v3d_performance_counters[];

extern unsigned int v3d_perf_counter_for_bank(unsigned int bank, unsigned int id);

VCOS_EXTERN_C_END

#endif
