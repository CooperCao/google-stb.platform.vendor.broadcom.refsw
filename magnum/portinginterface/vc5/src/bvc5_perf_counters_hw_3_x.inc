/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

/* Now give them real values */
   BVC5_P_SetCounterNames(d, 0, "fep_valid_prims_no_pixels", "prims");
   BVC5_P_SetCounterNames(d, 1, "fep_valid_prims", "prims");
   BVC5_P_SetCounterNames(d, 2, "fep_ez_nfclip_quads", "quads");
   BVC5_P_SetCounterNames(d, 3, "fep_valid_quads", "quads");
   BVC5_P_SetCounterNames(d, 4, "tlb_quads_stencil_fail", "quads");
   BVC5_P_SetCounterNames(d, 5, "tlb_quads_stencilz_fail", "quads");
   BVC5_P_SetCounterNames(d, 6, "tlb_quads_stencilz_pass", "quads");
   BVC5_P_SetCounterNames(d, 7, "tlb_quads_zero_cov", "quads");
   BVC5_P_SetCounterNames(d, 8, "tlb_quads_nonzero_cov", "quads");
   BVC5_P_SetCounterNames(d, 9, "tlb_quads_written", "quads");
   BVC5_P_SetCounterNames(d, 10, "ptb_prim_viewpoint_discard", "prims");
   BVC5_P_SetCounterNames(d, 11, "ptb_prim_clip", "prims");
   BVC5_P_SetCounterNames(d, 12, "ptb_prim_rev", "prims");
   BVC5_P_SetCounterNames(d, 13, "qpu_idle_cycles", "cycles");
   BVC5_P_SetCounterNames(d, 14, "qpu_active_cycles_vertex_coord_user", "cycles");
   BVC5_P_SetCounterNames(d, 15, "qpu_active_cycles_frag", "cycles");
   BVC5_P_SetCounterNames(d, 16, "qpu_cycles_valid_instr", "cycles");
   BVC5_P_SetCounterNames(d, 17, "qpu_cycles_tmu_stall", "cycles");
   BVC5_P_SetCounterNames(d, 18, "qpu_cycles_scoreboard_stall", "cycles");
   BVC5_P_SetCounterNames(d, 19, "qpu_cycles_varyings_stall", "cycles");
   BVC5_P_SetCounterNames(d, 20, "qpu_ic_hit", "hits");
   BVC5_P_SetCounterNames(d, 21, "qpu_ic_miss", "misses");
   BVC5_P_SetCounterNames(d, 22, "qpu_uc_hit", "hits");
   BVC5_P_SetCounterNames(d, 23, "qpu_uc_miss", "misses");
   BVC5_P_SetCounterNames(d, 24, "tmu_tcache_access", "accesses");
   BVC5_P_SetCounterNames(d, 25, "tmu_tcache_miss", "misses");
   BVC5_P_SetCounterNames(d, 26, "vpm_vdw_stall", "cycles");
   BVC5_P_SetCounterNames(d, 27, "vpm_vcd_stall", "cycles");
   BVC5_P_SetCounterNames(d, 28, "l2c_hits", "hits");
   BVC5_P_SetCounterNames(d, 29, "l2c_misses", "misses");
   BVC5_P_SetCounterNames(d, 30, "l2t_hits", "hits");
   BVC5_P_SetCounterNames(d, 31, "l2t_misses", "misses");
   BVC5_P_SetCounterNames(d, 32, "cycle_count", "cycles");
   BVC5_P_SetCounterNames(d, 33, "qpu_cycles_stalled_vertex_coord_user", "cycles");
   BVC5_P_SetCounterNames(d, 34, "qpu_cycles_stalled_fragment", "cycles");
   BVC5_P_SetCounterNames(d, 35, "ptb_prims_binned", "prims");
   BVC5_P_SetCounterNames(d, 36, "axi_writes_watch_0", "writes");
   BVC5_P_SetCounterNames(d, 37, "axi_reads_watch_0", "reads");
   BVC5_P_SetCounterNames(d, 38, "axi_write_stalls_watch_0", "stalls");
   BVC5_P_SetCounterNames(d, 39, "axi_read_stalls_watch_0", "stalls");
   BVC5_P_SetCounterNames(d, 40, "axi_write_bytes_watch_0", "bytes");
   BVC5_P_SetCounterNames(d, 41, "axi_read_bytes_watch_0", "bytes");
   BVC5_P_SetCounterNames(d, 42, "axi_writes_watch_1", "writes");
   BVC5_P_SetCounterNames(d, 43, "axi_reads_watch_1", "reads");
   BVC5_P_SetCounterNames(d, 44, "axi_write_stalls_watch_1", "stalls");
   BVC5_P_SetCounterNames(d, 45, "axi_read_stalls_watch_1", "stalls");
   BVC5_P_SetCounterNames(d, 46, "axi_write_bytes_watch_1", "bytes");
   BVC5_P_SetCounterNames(d, 47, "axi_read_bytes_watch_1", "bytes");
   BVC5_P_SetCounterNames(d, 48, "tlb_partial_quads", "quads");
   BVC5_P_SetCounterNames(d, 49, "tmu_config_accesses", "accesses");
   BVC5_P_SetCounterNames(d, 50, "l2t_no_id_stall", "stalls");
   BVC5_P_SetCounterNames(d, 51, "l2t_com_que_stall", "stalls");
   BVC5_P_SetCounterNames(d, 52, "l2t_rw_conflict_stall", "stalls");
   BVC5_P_SetCounterNames(d, 53, "tmu_active_cycles", "cycles");
   BVC5_P_SetCounterNames(d, 54, "tmu_stalled_cycles", "cycles");
