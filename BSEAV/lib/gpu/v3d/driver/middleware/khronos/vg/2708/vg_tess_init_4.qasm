### ===========================================================================
### Copyright (C) 2009 Broadcom Europe Limited.
### All rights reserved.
###
### Project  :  khronos
### Module   :  VG tessellation
###
### FILE DESCRIPTION
### Restores persistent VPM stuff used by fill/stroke user shaders.
###
### vg_tess_init_shader_4.c/h are generated from vg_tess_init_4.qasm by:
### qasm -mml_c:middleware/khronos/vg/2708/vg_tess_init_shader_4,VG_TESS_INIT_SHADER_4,vg_tess_init_shader,annots -tb0 vg_tess_init_4.qasm >vg_tess_init_shader_4.c
### and:
### qasm -mml_h:middleware/khronos/vg/2708/vg_tess_init_shader_4,VG_TESS_INIT_SHADER_4,vg_tess_init_shader,annots -tb0 vg_tess_init_4.qasm >vg_tess_init_shader_4.h
### So don't edit vg_tess_init_shader_4.c/h directly.
### ===========================================================================

.include "vg_util_4.qinc"

::vg_tess_init_shader

   # 2 common rows at 15 and 31
   mov  vr_setup, vdr_setup_0(2, 16, dma_h32(15, 0), 16, 64)
   mov  vr_addr, a:vg_tess_shader_if + 4

   # 12 * 8 bytes at 0, 5, 10, 16, 21, 26, 32, 37, 42, 48, 53, 58
   .rep i, 4
      mov  vr_setup, vdr_setup_0(3, 2, dma_h32(i * 16, 0), 5, 8)
      mov  vr_setup, vdr_setup_1(8)
      mov  vr_addr, a:vg_tess_shader_if + 4 + 128 + (i * 24)
   .endr

   mov  -, vr_wait

   # write -1 to indicate we're done
   mov  vw_setup, vpm_setup(1, 0, h32(1))
   mov  vpm, -1
   mov  vw_setup, vdw_setup_0(1, 1, dma_h32(1, 0))
   mov  vw_addr, a:vg_tess_shader_if
   mov  -, vw_wait

   # prod host and end
   mov  interrupt, 1
   thrend
   nop
   nop
