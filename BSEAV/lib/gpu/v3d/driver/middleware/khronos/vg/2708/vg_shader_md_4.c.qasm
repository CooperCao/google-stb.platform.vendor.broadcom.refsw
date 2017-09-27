/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  vg_shader_md_4.c is generated from vg_shader_md_4.c.qasm by:
 *  qasm -minline_c -tb0 vg_shader_md_4.c.qasm >vg_shader_md_4.c
 *  So don't edit vg_shader_md_4.c directly.
 ******************************************************************************/

%[ .eval pragma_allow_xor_0(True) %]

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/vg/2708/vg_shader_4.h"

%[
# todo: get rid of this stuff...
.macro extra_thrsws, n
   .rep i, n
      thrsw
      nop
      nop
   .endr
.endm
%]

const uint32_t VG_SHADER_MD_CLEAR_EXTRA_THRSWS = 5;

const uint32_t VG_SHADER_MD_CLEAR[] = { %[
   extra_thrsws  5
   nop              ; nop          # sbwait can't be first instr
   mov  tlbam, unif ; nop ; sbdone # todo: not 2nd either? (see hw-2796)
   nop              ; nop ; thrend
   nop              ; nop
   nop              ; nop          %] };

const uint32_t VG_SHADER_MD_CLEAR_SIZE = sizeof(VG_SHADER_MD_CLEAR);

const uint32_t VG_SHADER_MD_IMAGE_EXTRA_THRSWS = 4;

%[
.macro image_fetch
   itof  r0, y_coord  ; nop
   fadd  r0, r0, unif ; nop
   itof  r1, x_coord  ; fmul  r0, r0, unif
   fadd  r1, r1, unif ; nop
   nop                ; fmul  r1, r1, unif ; thrsw
   mov  t0t, r0       ; nop
   mov  t0s, r1       ; nop
.endm
%]

const uint32_t VG_SHADER_MD_IMAGE_SET[] = { %[
   extra_thrsws  4
   image_fetch
   nop                ; nop ; sbwait
   nop                ; nop ; ldtmu0
   mov  tlbam, r4.8dr ; nop ; sbdone
   nop                ; nop ; thrend
   nop                ; nop
   nop                ; nop          %] };

const uint32_t VG_SHADER_MD_IMAGE_SET_SIZE = sizeof(VG_SHADER_MD_IMAGE_SET);

const uint32_t VG_SHADER_MD_IMAGE_UNION[] = { %[
   extra_thrsws  4
   image_fetch
   nop                   ; nop                    ; loadam
   not  r0, r4           ; mov  r1, r4            ; ldtmu0
   nop                   ; v8muld  r0, r0, r4.8dr
   v8adds  tlbam, r0, r1 ; nop                    ; sbdone
   nop                   ; nop                    ; thrend
   nop                   ; nop
   nop                   ; nop                             %] };

const uint32_t VG_SHADER_MD_IMAGE_UNION_SIZE = sizeof(VG_SHADER_MD_IMAGE_UNION);

const uint32_t VG_SHADER_MD_IMAGE_INTERSECT[] = { %[
   extra_thrsws  4
   image_fetch
   nop ; nop                       ; loadam
   nop ; mov  r0, r4               ; ldtmu0
   nop ; v8muld  tlbam, r0, r4.8dr ; sbdone
   nop ; nop                       ; thrend
   nop ; nop
   nop ; nop                                %] };

const uint32_t VG_SHADER_MD_IMAGE_INTERSECT_SIZE = sizeof(VG_SHADER_MD_IMAGE_INTERSECT);

const uint32_t VG_SHADER_MD_IMAGE_SUBTRACT[] = { %[
   extra_thrsws  4
   image_fetch
   nop             ; nop                   ; ldtmu0
   not  r0, r4.8dr ; nop                   ; loadam
   nop             ; v8muld  tlbam, r0, r4 ; sbdone
   nop             ; nop                   ; thrend
   nop             ; nop
   nop             ; nop                            %] };

const uint32_t VG_SHADER_MD_IMAGE_SUBTRACT_SIZE = sizeof(VG_SHADER_MD_IMAGE_SUBTRACT);

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SET[] = { %[
   extra_thrsws  4
   image_fetch
   nop            ; nop ; sbwait
   nop            ; nop ; ldtmu0
   mov  tlbam, r4 ; nop ; sbdone
   nop            ; nop ; thrend
   nop            ; nop
   nop            ; nop          %] };

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SET_SIZE = sizeof(VG_SHADER_MD_IMAGE_NO_ALPHA_SET);

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_UNION[] = { %[
   extra_thrsws  4
   image_fetch
   nop                   ; nop                ; loadam
   not  r0, r4           ; mov  r1, r4        ; ldtmu0
   nop                   ; v8muld  r0, r0, r4
   v8adds  tlbam, r0, r1 ; nop                ; sbdone
   nop                   ; nop                ; thrend
   nop                   ; nop
   nop                   ; nop                         %] };

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_UNION_SIZE = sizeof(VG_SHADER_MD_IMAGE_NO_ALPHA_UNION);

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_INTERSECT[] = { %[
   extra_thrsws  4
   image_fetch
   nop ; nop                   ; loadam
   nop ; mov  r0, r4           ; ldtmu0
   nop ; v8muld  tlbam, r0, r4 ; sbdone
   nop ; nop                   ; thrend
   nop ; nop
   nop ; nop                            %] };

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_INTERSECT_SIZE = sizeof(VG_SHADER_MD_IMAGE_NO_ALPHA_INTERSECT);

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SUBTRACT[] = { %[
   extra_thrsws  4
   image_fetch
   nop         ; nop                   ; ldtmu0
   not  r0, r4 ; nop                   ; loadam
   nop         ; v8muld  tlbam, r0, r4 ; sbdone
   nop         ; nop                   ; thrend
   nop         ; nop
   nop         ; nop                            %] };

const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SUBTRACT_SIZE = sizeof(VG_SHADER_MD_IMAGE_NO_ALPHA_SUBTRACT);

const uint32_t VG_SHADER_MD_CVG_EXTRA_THRSWS = 5;

const uint32_t VG_SHADER_MD_CVG_SET[] = { %[
   extra_thrsws  5
   nop            ; nop          # sbwait can't be first instr
   nop            ; nop ; loadcv
   mov  tlbam, r4 ; nop ; sbdone
   nop            ; nop ; thrend
   nop            ; nop
   nop            ; nop          %] };

const uint32_t VG_SHADER_MD_CVG_SET_SIZE = sizeof(VG_SHADER_MD_CVG_SET);

const uint32_t VG_SHADER_MD_CVG_UNION[] = { %[
   extra_thrsws  5
   nop                   ; nop                         # sbwait can't be first instr
   nop                   ; nop                ; loadcv
   not  r0, r4           ; mov  r1, r4        ; loadam
   nop                   ; v8muld  r0, r0, r4
   v8adds  tlbam, r0, r1 ; nop                ; sbdone
   nop                   ; nop                ; thrend
   nop                   ; nop
   nop                   ; nop                         %] };

const uint32_t VG_SHADER_MD_CVG_UNION_SIZE = sizeof(VG_SHADER_MD_CVG_UNION);

const uint32_t VG_SHADER_MD_CVG_INTERSECT[] = { %[
   extra_thrsws  5
   nop ; nop                            # sbwait can't be first instr
   nop ; nop                   ; loadcv
   nop ; mov  r0, r4           ; loadam
   nop ; v8muld  tlbam, r0, r4 ; sbdone
   nop ; nop                   ; thrend
   nop ; nop
   nop ; nop                            %] };

const uint32_t VG_SHADER_MD_CVG_INTERSECT_SIZE = sizeof(VG_SHADER_MD_CVG_INTERSECT);

const uint32_t VG_SHADER_MD_CVG_SUBTRACT[] = { %[
   extra_thrsws  5
   nop         ; nop                            # sbwait can't be first instr
   nop         ; nop                   ; loadcv
   not  r0, r4 ; nop                   ; loadam
   nop         ; v8muld  tlbam, r0, r4 ; sbdone
   nop         ; nop                   ; thrend
   nop         ; nop
   nop         ; nop                            %] };

const uint32_t VG_SHADER_MD_CVG_SUBTRACT_SIZE = sizeof(VG_SHADER_MD_CVG_SUBTRACT);

const uint32_t VG_SHADER_MD_CVG_SCISSOR_EXTRA_THRSWS = 4;

%[
.macro scissor_fetch
   itof  r0, x_coord ; mov  ra0, f(0.5)
   itof  r1, y_coord ; mov  r2, unif
   fadd  r1, r1, ra0 ; mov  r3, unif     ; thrsw
   fadd  r0, r0, ra0 ; fmul  t0t, r1, r3
   nop               ; fmul  t0s, r0, r2
.endm
%]

const uint32_t VG_SHADER_MD_CVG_SCISSOR_SET[] = { %[
   extra_thrsws  4
   scissor_fetch
   nop                ; nop ; loadcv
   mov  r0, r4        ; nop ; ldtmu0
   and  tlbam, r0, r4 ; nop ; sbdone
   nop                ; nop ; thrend
   nop                ; nop
   nop                ; nop          %] };

const uint32_t VG_SHADER_MD_CVG_SCISSOR_SET_SIZE = sizeof(VG_SHADER_MD_CVG_SCISSOR_SET);

const uint32_t VG_SHADER_MD_CVG_SCISSOR_UNION[] = { %[
   extra_thrsws  4
   scissor_fetch
   nop                   ; nop                ; loadcv
   mov  r0, r4           ; nop                ; ldtmu0
   and  r0, r0, r4       ; nop                ; loadam
   not  r1, r0           ; nop
   nop                   ; v8muld  r1, r1, r4
   v8adds  tlbam, r0, r1 ; nop                ; sbdone
   nop                   ; nop                ; thrend
   nop                   ; nop
   nop                   ; nop                         %] };

const uint32_t VG_SHADER_MD_CVG_SCISSOR_UNION_SIZE = sizeof(VG_SHADER_MD_CVG_SCISSOR_UNION);

const uint32_t VG_SHADER_MD_CVG_SCISSOR_INTERSECT[] = { %[
   extra_thrsws  4
   scissor_fetch
   nop             ; nop                   ; loadcv
   mov  r0, r4     ; nop                   ; ldtmu0
   and  r0, r0, r4 ; nop                   ; loadam
   nop             ; v8muld  tlbam, r0, r4 ; sbdone
   nop             ; nop                   ; thrend
   nop             ; nop
   nop             ; nop                            %] };

const uint32_t VG_SHADER_MD_CVG_SCISSOR_INTERSECT_SIZE = sizeof(VG_SHADER_MD_CVG_SCISSOR_INTERSECT);

const uint32_t VG_SHADER_MD_CVG_SCISSOR_SUBTRACT[] = { %[
   extra_thrsws  4
   scissor_fetch
   nop             ; nop                   ; loadcv
   mov  r0, r4     ; nop                   ; ldtmu0
   and  r0, r0, r4 ; nop
   not  r0, r0     ; nop                   ; loadam
   nop             ; v8muld  tlbam, r0, r4 ; sbdone
   nop             ; nop                   ; thrend
   nop             ; nop
   nop             ; nop                            %] };

const uint32_t VG_SHADER_MD_CVG_SCISSOR_SUBTRACT_SIZE = sizeof(VG_SHADER_MD_CVG_SCISSOR_SUBTRACT);
