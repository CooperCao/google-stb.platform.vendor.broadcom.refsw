/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_clear_layers.h"
#include "glxx_hw.h"
#if V3D_VER_AT_LEAST(4,1,34,0)

/* .vs
 * row 0 contains instance_id, and rows 1, 2, 3 vec3 pos (x, y, z);
 * use combined vpm segment and leave the data as is for gs
 */
static uint64_t vs_shader[] =
{
0x3c203186bb800000ull, // [0x00000000] nop                  ; nop              ; thrsw
0x3c003186bb800000ull, // [0x00000008] nop                  ; nop
0x3c003186bb800000ull, // [0x00000010] nop                  ; nop
};

/*.gs
 * layout(triangles) in;
 * layout(max_vertices = 16, triangle_strip) out;
 * in vec3 inp[];
 * flat in int instance[];
 *  void main()
 *  {
 *     for (int i = 0; i < 3; i++)
 *     {
 *        gl_Layer = instance[i];
 *        gl_Position = vec4(inp[i], 1);
 *        EmitVertex();
 *     }
 *
 *     EndPrimitive();
 *
 *  }
 */
//ldvpmg_in Data, Row, Col
//stvmpv Row, Data
static uint64_t gs_bin_shader[] =
{
/*
                                                                #col 0 (load instance, xs, ys)
   ldvpmg_in  rf0,  0, 0    ; mov r0, 0                         #row=0 col=0 (instance[0] used as layer)
   ldvpmg_in  rf2,  1, r0   ; add r2, 1, 1                      #row=1 col=0, r2 = 2
   ldvpmg_in  rf3, r2, r0   ; nop               ; ldunifrf.rf4  #row=2 col=0, unif = 0x00030004 {3 vertices << 16 | offset to vertex 0 (4)}

                                                                #store gs header
   stvpmv     r0,  rf4      ; nop               ; ldunifrf.rf5  #row=0, unif = 16
   shl        rf6, rf0, rf5 ; nop               ; ldunifrf.rf7  #layer << 16 (= layer_shl16), unif = 0x00000600 (length vertex data for PTB << 8)
   or         rf8, rf6, rf7 ; nop                               #layer_shl16 | length_shl8
   stvpmv     1, rf8        ; mov r1, 1                         #row=1
   stvpmv     2, rf8        ; nop                               #row=2
   stvpmv     3, rf8        ; nop                               #row=3

                                                                #store vertex 0 data
                                                                #(we do not store values for xc, yc, zc, wc --> skip to row 14, store xs, ys)
   stvpmv     8, rf2       ; nop                                #row=8
   stvpmv     9, rf3       ; nop                                #row=9

                                                                #col1 (load xs, ys)
   ldvpmg_in  rf4, r1, r1  ; nop                                #row=1 col=1
   ldvpmg_in  rf5, r2, r1  ; nop                                #row=2 col=1

                                                                #col2 (load xs, ys)
   ldvpmg_in  rf6, r1, r2   ; nop               ; ldunifrf.r3   #row=1 col=2 , unif = 20
   ldvpmg_in  rf7, r2, r2   ; mov r4, 15                        #row=2 col=2

                                                                #store vertex 1 data
                                                                #(we do not store values for xc, yc, zc, wc --> skip to row 14, store xs, ys)
   stvpmv     14, rf4       ; nop                               #row=14
   stvpmv     r4, rf5       ; nop               ; thrsw         #row=15

                                                                #store vertex 2 data
                                                                #(we do not store values for xc, yc, zc, wc --> skip to row 20, store xs, ys)
   stvpmv     r3, rf6       ; add r3, r3, r1                    #row=20      , r3 = 20 + 1
   stvpmv     r3, rf7       ; nop                               #row=21
*/
0x3de02000bdfff000ull, // [0x00000000] ldvpmg_in  rf0,  0, 0    ; mov r0, 0
0x05e02082bdfc7001ull, // [0x00000008] ldvpmg_in  rf2,  1, r0   ; add r2, 1, 1
0x3d812183bd802000ull, // [0x00000010] ldvpmg_in  rf3, r2, r0   ; nop               ; ldunifrf.rf4
0x3d816180f8830100ull, // [0x00000018] stvpmv     r0,  rf4      ; nop               ; ldunifrf.rf5
0x3d81e1867c83e005ull, // [0x00000020] shl        rf6, rf0, rf5 ; nop               ; ldunifrf.rf7
0x3c002188b683e187ull, // [0x00000028] or         rf8, rf6, rf7 ; nop
0x3de02040f8ff7201ull, // [0x00000030] stvpmv     1, rf8        ; mov r1, 1
0x3de02180f8837202ull, // [0x00000038] stvpmv     2, rf8        ; nop
0x3de02180f8837203ull, // [0x00000040] stvpmv     3, rf8        ; nop
0x3de02180f8837088ull, // [0x00000048] stvpmv     8, rf2       ; nop
0x3de02180f88370c9ull, // [0x00000050] stvpmv     9, rf3       ; nop
0x3c002184bd809000ull, // [0x00000058] ldvpmg_in  rf4, r1, r1  ; nop
0x3c002185bd80a000ull, // [0x00000060] ldvpmg_in  rf5, r2, r1  ; nop
0x3d90e186bd811000ull, // [0x00000068] ldvpmg_in  rf6, r1, r2   ; nop               ; ldunifrf.r3
0x3de02107bdfd200full, // [0x00000070] ldvpmg_in  rf7, r2, r2   ; mov r4, 15
0x3de02180f883710eull, // [0x00000078] stvpmv     14, rf4       ; nop
#if V3D_HAS_GFXH1684_FIX
0x3c202180f8834140ull, // [0x00000080] stvpmv     r4, rf5       ; nop               ; thrsw
0x040020c0f82f3180ull, // [0x00000088] stvpmv     r3, rf6       ; add r3, r3, r1
0x3c002180f88331c0ull, // [0x00000090] stvpmv     r3, rf7       ; nop
#else
// GFXH-1684 workaround
0x3c002180f8834140ull, // [0x00000080] stvpmv     r4, rf5       ; nop
0x040020c0f82f3180ull, // [0x00000088] stvpmv     r3, rf6       ; add r3, r3, r1
0x3c202180f88331c0ull, // [0x00000090] stvpmv     r3, rf7       ; nop               ; thrsw
0x3c003186bb816000ull, // [0x00000098] vpmwt      -
0x3c003186bb800000ull, // [0x000000a0] nop
#endif
};

static uint32_t gs_bin_unif[] =
{
   0x00030004, // vertices | offset to vertex 0 ( 4 rows)
   0x00000010,
   0x00000600, // length vertex data for PTB << 8 ( 6 << 8)
   0x00000014,
};

//ldvpmg_in Data, Row, Col
//stvmpv Row, Data
static uint64_t gs_render_shader[] =
{
/*
                                                                #col 0 (load instance, xs, ys, zs)
   ldvpmg_in  rf0,  0, 0    ; mov r0, 0                         #row=0 col=0 (instance[0] used as layer)
   ldvpmg_in  rf2,  1, r0   ; add r2, 1, 1                      #row=1 col=0, r2 = 2
   ldvpmg_in  rf3, r2, r0   ; add r3, r2, 1                     #row=2 col=0
   ldvpmg_in  rf9, r3, r0   ; nop               ; ldunifrf.rf4  #row=3 col=0, unif = 0x00030004 {3 vertices << 16 | offset to vertex 0 (4)}

                                                                #store gs header
   stvpmv     r0,  rf4      ; nop               ; ldunifrf.rf5  #row=0, unif = 16
   shl        rf6, rf0, rf5 ; nop               ; ldunifrf.rf7  #layer << 16 (= layer_shl16), unif = 0x00000400 (length vertex data for PSE << 8)
   or         rf8, rf6, rf7 ; nop               ;ldunifrf.rf10  #layer_shl16 | length_shl8, unif = 1.0f (0x3f800000)
   stvpmv     1, rf8        ; mov r1, 1                         #row=1
   stvpmv     2, rf8        ; nop                               #row=2
   stvpmv     3, rf8        ; nop                               #row=3

                                                                #store vertex 0 data
                                                                #store xs, ys, zs and 1.0f for 1/wc
   stvpmv     4, rf2       ; nop                                #row=4
   stvpmv     5, rf3       ; nop                                #row=5
   stvpmv     6, rf9       ; nop                                #row=6
   stvpmv     7, rf10      ; nop                                #row=7

                                                                #col1 (load xs, ys)
   ldvpmg_in  rf4, r1, r1  ; nop                                #row=1 col=1
   ldvpmg_in  rf5, r2, r1  ; nop                                #row=2 col=1

                                                                #col2 (load xs, ys)
   ldvpmg_in  rf6, r1, r2   ; nop                               #row=1 col=2
   ldvpmg_in  rf7, r2, r2   ; mov r3, 13                        #row=2 col=2

                                                                #store vertex 1 data
                                                                #store xs, ys, zs (from vertex 0) and 1.0f for 1/wc
   stvpmv     8, rf4       ; nop                                #row=8
   stvpmv     9, rf5       ; nop                                #row=9
   stvpmv    10, rf9       ; nop                                #row=10
   stvpmv    11, rf10      ; nop                                #row=11

                                                                #store vertex 2 data
                                                                #store xs, ys, zs (from vertex 0) and 1.0f for 1/wc
   stvpmv    12, rf6       ; nop                                #row=12
   stvpmv    r3, rf7       ; nop                 ; thrsw        #row=13
   stvpmv    14, rf9       ; nop                                #row=14
   stvpmv    15, rf10      ; nop                                #row=15
*/
0x3de02000bdfff000ull, // [0x00000000] ldvpmg_in  rf0,  0, 0    ; mov r0, 0
0x05e02082bdfc7001ull, // [0x00000008] ldvpmg_in  rf2,  1, r0   ; add r2, 1, 1
0x05e020c3bde82001ull, // [0x00000010] ldvpmg_in  rf3, r2, r0   ; add r3, r2, 1
0x3d812189bd803000ull, // [0x00000018] ldvpmg_in  rf9, r3, r0   ; nop               ; ldunifrf.rf4
0x3d816180f8830100ull, // [0x00000020] stvpmv     r0,  rf4      ; nop               ; ldunifrf.rf5
0x3d81e1867c83e005ull, // [0x00000028] shl        rf6, rf0, rf5 ; nop               ; ldunifrf.rf7
0x3d82a188b683e187ull, // [0x00000030] or         rf8, rf6, rf7 ; nop               ;ldunifrf.rf10
0x3de02040f8ff7201ull, // [0x00000038] stvpmv     1, rf8        ; mov r1, 1
0x3de02180f8837202ull, // [0x00000040] stvpmv     2, rf8        ; nop
0x3de02180f8837203ull, // [0x00000048] stvpmv     3, rf8        ; nop
0x3de02180f8837084ull, // [0x00000050] stvpmv     4, rf2       ; nop
0x3de02180f88370c5ull, // [0x00000058] stvpmv     5, rf3       ; nop
0x3de02180f8837246ull, // [0x00000060] stvpmv     6, rf9       ; nop
0x3de02180f8837287ull, // [0x00000068] stvpmv     7, rf10      ; nop
0x3c002184bd809000ull, // [0x00000070] ldvpmg_in  rf4, r1, r1  ; nop
0x3c002185bd80a000ull, // [0x00000078] ldvpmg_in  rf5, r2, r1  ; nop
0x3c002186bd811000ull, // [0x00000080] ldvpmg_in  rf6, r1, r2   ; nop
0x3de020c7bdfd200dull, // [0x00000088] ldvpmg_in  rf7, r2, r2   ; mov r3, 13
0x3de02180f8837108ull, // [0x00000090] stvpmv     8, rf4       ; nop
0x3de02180f8837149ull, // [0x00000098] stvpmv     9, rf5       ; nop
0x3de02180f883724aull, // [0x000000a0] stvpmv    10, rf9       ; nop
0x3de02180f883728bull, // [0x000000a8] stvpmv    11, rf10      ; nop
0x3de02180f883718cull, // [0x000000b0] stvpmv    12, rf6       ; nop
#if V3D_HAS_GFXH1684_FIX
0x3c202180f88331c0ull, // [0x000000b8] stvpmv    r3, rf7       ; nop                 ; thrsw
0x3de02180f883724eull, // [0x000000c0] stvpmv    14, rf9       ; nop
0x3de02180f883728full, // [0x000000c8] stvpmv    15, rf10      ; nop
#else
// GFXH-1684 workaround
0x3de02180f883724eull, // [0x000000b8] stvpmv    14, rf9       ; nop
0x3de02180f883728full, // [0x000000c0] stvpmv    15, rf10      ; nop
0x3c202180f88331c0ull, // [0x000000c8] stvpmv    r3, rf7       ; nop                 ; thrsw
0x3c003186bb816000ull, // [0x000000d0] vpmwt     -
0x3c003186bb800000ull, // [0x000000d8] nop
#endif
};

static uint32_t gs_render_unif[] =
{
   0x00030004, // vertices | offset to vertex 0 (4 rows )
   0x00000010,
   0x00000400, // length vertex data for PSE << 8 ( 4 << 8)
   0x3f800000, //for 1/wc
};

v3d_addr_t glxx_create_clear_gl_g_shader_record(khrn_fmem *fmem,
      v3d_addr_t fshader_addr, v3d_addr_t funif_addr,
      const gfx_rect *rect, float clear_depth_val)
{
   uint32_t *unif[2]; // 0 = gs_bin, 1 = gs_render
   v3d_addr_t unif_addr[2];
   uint32_t *shader[2];
   v3d_addr_t shader_addr[2];

   unif[0] = khrn_fmem_data(&unif_addr[0], fmem, sizeof(gs_bin_unif), V3D_QPU_UNIFS_ALIGN);
   unif[1] = khrn_fmem_data(&unif_addr[1], fmem, sizeof(gs_render_unif), V3D_QPU_UNIFS_ALIGN);

   shader[0] = khrn_fmem_data(&shader_addr[0], fmem, sizeof(gs_bin_shader), V3D_QPU_INSTR_ALIGN);
   shader[1] = khrn_fmem_data(&shader_addr[1], fmem, sizeof(gs_render_shader), V3D_QPU_INSTR_ALIGN);

   v3d_addr_t vshader_addr;
   uint32_t *vshader = khrn_fmem_data(&vshader_addr, fmem, sizeof(vs_shader), V3D_QPU_INSTR_ALIGN);

   for (unsigned i = 0; i < 2; i++)
   {
      if (unif[i] == NULL || shader[i] == NULL)
         return 0;
   }
   if (vshader == NULL)
      return 0;

   memcpy(unif[0], gs_bin_unif, sizeof(gs_bin_unif));
   memcpy(unif[1], gs_render_unif, sizeof(gs_render_unif));
   memcpy(shader[0], gs_bin_shader, sizeof(gs_bin_shader));
   memcpy(shader[1], gs_render_shader, sizeof(gs_render_shader));

   memcpy(vshader, vs_shader, sizeof(vs_shader));

   V3D_SHADREC_GL_GEOM_T g_rec;
   g_rec.gs_bin = (V3D_SHADER_ARGS_T){.threading = V3D_THREADING_4,
                        .single_seg = true, .propagate_nans = true,
                        .addr = shader_addr[0],
                        .unifs_addr = unif_addr[0] };
   g_rec.gs_render = (V3D_SHADER_ARGS_T){.threading = V3D_THREADING_4,
                        .single_seg = true, .propagate_nans = true,
                        .addr = shader_addr[1],
                        .unifs_addr = unif_addr[1] };

   V3D_SHADREC_GL_TESS_OR_GEOM_T tg_rec = {0, };
   tg_rec.num_tcs_invocations = 1;
   tg_rec.geom_output = V3D_CL_GEOM_PRIM_TYPE_TRIANGLE_STRIP;
   tg_rec.geom_num_instances = 1;
   tg_rec.bin.geom_output = (V3D_GEOM_SEG_ARGS_T){.size_sectors = 3, .pack = V3D_CL_GEOM_OUTPUT_PACK_X16};
   tg_rec.bin.min_gs_segs= 1;
   tg_rec.bin.per_patch_depth=1;
   tg_rec.bin.max_patches_per_tcs_batch=1;
   tg_rec.bin.min_tcs_segs=1;
   tg_rec.bin.min_per_patch_segs=1;
   tg_rec.bin.max_patches_per_tes_batch=1;
   tg_rec.bin.max_tcs_segs_per_tes_batch=1;
   tg_rec.bin.min_tes_segs=1;

   tg_rec.render.geom_output =(V3D_GEOM_SEG_ARGS_T) {.size_sectors = 2, .pack = V3D_CL_GEOM_OUTPUT_PACK_X16};
   tg_rec.render.min_gs_segs = 1;
   tg_rec.render.per_patch_depth=1;
   tg_rec.render.max_patches_per_tcs_batch=1;
   tg_rec.render.min_tcs_segs=1;
   tg_rec.render.min_per_patch_segs=1;
   tg_rec.render.max_patches_per_tes_batch=1;
   tg_rec.render.max_tcs_segs_per_tes_batch=1;
   tg_rec.render.min_tes_segs=1;

   V3D_SHADREC_GL_MAIN_T main_rec = {0, };
   main_rec.cs_instance_id = true;
   main_rec.vs_instance_id = true;
   main_rec.cs_separate_blocks = false;
   main_rec.vs_separate_blocks = false;
   main_rec.cs_output_size = (V3D_OUT_SEG_ARGS_T){.sectors = 1};
   main_rec.vs_output_size = (V3D_OUT_SEG_ARGS_T){.sectors = 1};
   // add the following 2 just to stop v3d_pack_shadrec_gl_main asserting (though they should be ignored )
   main_rec.cs_input_size = (V3D_IN_SEG_ARGS_T){.min_req = 1};
   main_rec.vs_input_size = (V3D_IN_SEG_ARGS_T){.min_req = 1};
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   main_rec.defaults = 0;
#endif
   main_rec.fs = (V3D_SHADER_ARGS_T){.threading = V3D_THREADING_4,
                   .single_seg = false, .propagate_nans = true,
                   .addr = fshader_addr,
                   .unifs_addr = funif_addr};
   main_rec.vs = (V3D_SHADER_ARGS_T){.threading = V3D_THREADING_4,
                   .single_seg = true, .propagate_nans = true,
                   .addr = vshader_addr,
                   .unifs_addr = 0 };
   main_rec.cs = (V3D_SHADER_ARGS_T){.threading = V3D_THREADING_4,
                   .single_seg = true, .propagate_nans = true,
                   .addr = vshader_addr,
                   .unifs_addr = 0};


   V3D_SHADREC_GL_ATTR_T attr = {0, };
   unsigned vdata_max_index;

   v3d_addr_t vdata_addr = glxx_draw_rect_vertex_data(
         &vdata_max_index, fmem, rect, gfx_float_to_bits(clear_depth_val));
   if (!vdata_addr)
      return 0;
   attr.addr = vdata_addr;
   assert(vdata_max_index == 3);
   attr.size = 3;
   attr.type = V3D_ATTR_TYPE_FLOAT;
   attr.cs_num_reads = 3;
   attr.vs_num_reads = 3;
   attr.stride = 12;
   attr.max_index = vdata_max_index;

   unsigned sh_rec_size = V3D_SHADREC_GL_GEOM_PACKED_SIZE +
      V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE +
      V3D_SHADREC_GL_MAIN_PACKED_SIZE +
      V3D_SHADREC_GL_ATTR_PACKED_SIZE;

   v3d_addr_t sh_rec_addr;
   uint32_t *sh_rec = khrn_fmem_data(&sh_rec_addr, fmem, sh_rec_size, V3D_SHADREC_ALIGN);
   uint32_t *curr = sh_rec;
   if (!sh_rec)
      return 0;
   v3d_pack_shadrec_gl_geom(curr, &g_rec);
   curr += V3D_SHADREC_GL_GEOM_PACKED_SIZE / sizeof(*sh_rec);
   v3d_pack_shadrec_gl_tess_or_geom(curr, &tg_rec);
   curr += V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE / sizeof(*sh_rec);
   v3d_pack_shadrec_gl_main(curr, &main_rec);
   curr += V3D_SHADREC_GL_MAIN_PACKED_SIZE / sizeof(*sh_rec);
   v3d_pack_shadrec_gl_attr(curr, &attr);

   return sh_rec_addr;
}

#endif
