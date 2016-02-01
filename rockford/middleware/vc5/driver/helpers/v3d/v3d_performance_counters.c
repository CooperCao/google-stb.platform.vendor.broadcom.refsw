/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "v3d_performance_counters.h"
#include "helpers/v3d/v3d_common.h"

V3D_PERF_COUNTER_T v3d_performance_counters[] = {
   {0, "FEP_Valid_Prims_no_pixels",    "FEP Valid primitives that result in no rendered pixels, for all rendered tiles"},
   {1, "FEP_Valid_primitives",         "FEP Valid primitives for all rendered tiles"},
   {2, "FEP_clipped_quads",            "FEP Early-Z / Near / Far clipped quads"},
   {3, "FEP_Valid_quads",              "FEP Valid quads"},
   {4, "TLB_Quads_fail_stencil",       "TLB Quads with no pixels passing the stencil test"},
   {5, "TLB_Quads_fail_z_stencil",     "TLB Quads with no pixels passing the Z and stencil tests"},
   {6, "TLB_Quads_pass_z_stencil",     "TLB Quads with any pixels passing the Z and stencil tests"},
   {7, "TLB_Quads_fail_coverage",      "TLB Quads with all pixels having zero coverage"},
   {8, "TLB_Quads_pass_coverage",      "TLB Quads with any pixels having non-zero coverage"},
   {9, "TLB_Quads_with_valid_pixels",  "TLB Quads with valid pixels written to colour buffer"},
   {10, "PTB_Prims_fail_viewport",     "PTB Primitives discarded by being outside the viewport"},
   {11, "PTB_Prims_need_clip",         "PTB Primitives that need clipping"},
   {12, "PTB_Prims_reversed",          "PTB Primitives that are discarded because they are reversed"},
   {13, "QPU_idle_clock_cycles",       "QPU Total idle clock cycles for all QPUs"},
   {14, "QPU_cycles_non_frag_shading", "QPU Total active clock cycles for all QPUs doing vertex/coordinate/user shading"},
   {15, "QPU_cycles_frag_shading",     "QPU Total active clock cycles for all QPUs doing fragment shading"},
   {16, "QPU_valid_instuction_cycles", "QPU Total clock cycles for all QPUs executing valid instructions"},
   {17, "QPU_cycles_stalled_TMU",      "QPU Total clock cycles for all QPUs stalled waiting for TMUs"},
   {18, "QPU_cycles_stalled_scorebrd", "QPU Total clock cycles for all QPUs stalled waiting for Scoreboard"},
   {19, "QPU_cycles_stalled_varyings", "QPU Total clock cycles for all QPUs stalled waiting for Varyings"},
   {20, "QPU_instr_cache_hits",        "QPU Total instruction cache hits for all slices"},
   {21, "QPU_instr_cache_misses",      "QPU Total instruction cache misses for all slices"},
   {22, "QPU_uniform_cache_hits",      "QPU Total uniforms cache hits for all slices"},
   {23, "QPU_uniform_cache_misses",    "QPU Total uniforms cache misses for all slices"},
   {24, "TMU_cache_accesses",          "TMU Total texture cache accesses"},
   {25, "TMU_cache_misses",            "TMU Total texture cache misses (number of fetches from memory/L2cache)"},
   {26, "VDW_cycles_stalled_VPM",      "VPM Total clock cycles VDW is stalled waiting for VPM access"},
   {27, "VCD_cycles_stalled_VPM",      "VPM Total clock cycles VCD is stalled waiting for VPM access"},
   {28, "L2C_cache_hits",              "L2C Total Level 2 cache hits"},
   {29, "L2C_cache_misses",            "L2C Total Level 2 cache misses"},
   {30, "L2T_cache_hits",              "L2T Total Level 2 cache hits"},
   {31, "L2T_cache_misses",            "L2T Total Level 2 cache misses"},
   {32, "Cycle_counter",               "Cycle counter"},
   {33, "QPU_cycles_stalled_non_frag", "QPU total stalled clock cycles for all QPUs doing vertex/coordinate/user shading"},
   {34, "QPU_cycles_stalled_frag",     "QPU total stalled clock cycles for all QPUs doing fragment shading"},
   {35, "PTB_prims_binned",            "PTB Total primitives binned"},
   {36, "AXI_writes_watch_0",          "AXI writes seen by watch 0"},
   {37, "AXI_reads_watch_0",           "AXI reads seen by watch 0"},
   {38, "AXI_write_stalls_watch_0",    "AXI write stalls seen by watch 0"},
   {39, "AXI_read_stalls_watch_0",     "AXI read stalls seen by watch 0"},
   {40, "AXI_bytes_w_watch_0",         "Total AXI bytes written seen by watch 0"},
   {41, "AXI_byes_r_watch_0",          "Total AXI bytes read seen by watch 0"},
   {42, "AXI_writes_watch_1",          "AXI writes seen by watch 1"},
   {43, "AXI_reads_watch_1",           "AXI reads seen by watch 1"},
   {44, "AXI_write_stalls_watch_1",    "AXI write stalls seen by watch 1"},
   {45, "AXI_read_stalls_watch_1",     "AXI read stalls seen by watch 1"},
   {46, "AXI_bytes_w_watch_1",         "Total AXI bytes written seen by watch 1"},
   {47, "AXI_byes_r_watch_1",          "Total AXI bytes read seen by watch 1"},
   {48, "TLB_partial_quads",           "TLB partial quads writen to the colour buffer"},
   {49, "TMU_config_accesses",         "TMU Total config accesses"},
   {50, "L2T_No_ID_Stall",             "L2T No ID Stall"},
   {51, "L2T_queue_full_stall",        "L2T Command Queue Full Stall"},
   {52, "L2T_r_w_conflict_stall",      "L2T Read/Write Conflict Stall"},
   {53, "PTB_tile_prims_total",        "PTB tiles*prims binned"},
};

// Banks of performance counters: map set of 16 to events
static unsigned int map_performance_counter_bank_to_event[][16] = { // Most deterministic ones are 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 16, 24, 35, 20, 22, 48, 49
   { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
   { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
   { 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 },
   { 48, 49, 50, 51, 52, 53, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
};

unsigned int v3d_perf_counter_for_bank(unsigned int bank, unsigned int id)
{
   assert(bank < vcos_countof(map_performance_counter_bank_to_event));
   assert(id < vcos_countof(map_performance_counter_bank_to_event[0]));
   return map_performance_counter_bank_to_event[bank][id];
}
