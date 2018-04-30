/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 **************************************************************************/

/* Now give them real values */
   BV3D_P_SetCounterNames(d, 0, "fep_valid_prims_no_pixels", "prims");
   BV3D_P_SetCounterNames(d, 1, "fep_valid_prims", "prims");
   BV3D_P_SetCounterNames(d, 2, "fep_ez_nfclip_quads", "quads");
   BV3D_P_SetCounterNames(d, 3, "fep_valid_quads", "quads");
   BV3D_P_SetCounterNames(d, 4, "tlb_quads_stencil_fail", "quads");
   BV3D_P_SetCounterNames(d, 5, "tlb_quads_stencilz_fail", "quads");
   BV3D_P_SetCounterNames(d, 6, "tlb_quads_stencilz_pass", "quads");
   BV3D_P_SetCounterNames(d, 7, "tlb_quads_zero_cov", "quads");
   BV3D_P_SetCounterNames(d, 8, "tlb_quads_nonzero_cov", "quads");
   BV3D_P_SetCounterNames(d, 9, "tlb_quads_written", "quads");
   BV3D_P_SetCounterNames(d, 10, "ptb_prim_viewpoint_discard", "prims");
   BV3D_P_SetCounterNames(d, 11, "ptb_prim_clip", "prims");
   BV3D_P_SetCounterNames(d, 12, "ptb_prim_rev", "prims");
   BV3D_P_SetCounterNames(d, 13, "qpu_idle_cycles", "cycles");
   BV3D_P_SetCounterNames(d, 14, "qpu_active_cycles_vertex_coord_user", "cycles");
   BV3D_P_SetCounterNames(d, 15, "qpu_active_cycles_frag", "cycles");
   BV3D_P_SetCounterNames(d, 16, "qpu_cycles_valid_instr", "cycles");
   BV3D_P_SetCounterNames(d, 17, "qpu_cycles_tmu_stall", "cycles");
   BV3D_P_SetCounterNames(d, 18, "qpu_cycles_scoreboard_stall", "cycles");
   BV3D_P_SetCounterNames(d, 19, "qpu_cycles_varyings_stall", "cycles");
   BV3D_P_SetCounterNames(d, 20, "qpu_ic_hit", "hits");
   BV3D_P_SetCounterNames(d, 21, "qpu_ic_miss", "misses");
   BV3D_P_SetCounterNames(d, 22, "qpu_uc_hit", "hits");
   BV3D_P_SetCounterNames(d, 23, "qpu_uc_miss", "misses");
   BV3D_P_SetCounterNames(d, 24, "tmu_tcache_access", "accesses");
   BV3D_P_SetCounterNames(d, 25, "tmu_tcache_miss", "misses");
   BV3D_P_SetCounterNames(d, 26, "vpm_vdw_stall", "cycles");
   BV3D_P_SetCounterNames(d, 27, "vpm_vcd_stall", "cycles");
   BV3D_P_SetCounterNames(d, 28, "l2c_hits", "hits");
   BV3D_P_SetCounterNames(d, 29, "l2c_misses", "misses");
