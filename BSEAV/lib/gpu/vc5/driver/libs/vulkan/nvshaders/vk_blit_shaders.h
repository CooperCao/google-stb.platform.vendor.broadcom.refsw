/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
struct inline_qasm { const uint64_t *code; size_t size; };
static const uint64_t blit_f16tlb_1D_instrs[] = {
0x3eb0f186bb800000ull, // [0x00000000] nop                                 ; wrtmuc; ldvary; thrsw
0x56603086bbcc0000ull, // [0x00000008] nop               ; fmul r2, r3, w  ; wrtmuc        ; thrsw
0x3c0031a10582a000ull, // [0x00000010] fadd tmus,r2,r5
0x3c913186bb800000ull, // [0x00000018] nop                                 ; ldtmu
0x3c203188b6824000ull, // [0x00000020] mov tlbu, r4                        ; thrsw
0x3c913186bb800000ull, // [0x00000028] nop                                 ; ldtmu
0x3c003187b6824000ull, // [0x00000030] mov tlb, r4
};
struct inline_qasm blit_f16tlb_1D = { blit_f16tlb_1D_instrs, countof(blit_f16tlb_1D_instrs) };
static const uint64_t blit_f16tlb_2D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                 ; wrtmuc; ldvary
0x56b0f086bbcc0000ull, // [0x00000008] nop               ; fmul r2, r3, w  ; thrsw; wrtmuc; ldvary
0x542030a205cea000ull, // [0x00000010] fadd tmut,r2,r5   ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000018] fadd tmus,r2,r5
0x3c913186bb800000ull, // [0x00000020] nop                                 ; ldtmu
0x3c203188b6824000ull, // [0x00000028] mov tlbu, r4                        ; thrsw
0x3c913186bb800000ull, // [0x00000030] nop                                 ; ldtmu
0x3c003187b6824000ull, // [0x00000038] mov tlb, r4
};
struct inline_qasm blit_f16tlb_2D = { blit_f16tlb_2D_instrs, countof(blit_f16tlb_2D_instrs) };
static const uint64_t blit_f16tlb_3D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                 ; wrtmuc; ldvary
0x5690f086bbcc0000ull, // [0x00000008] nop               ; fmul r2, r3, w  ; wrtmuc; ldvary
0x5530f0a205cea000ull, // [0x00000010] fadd tmut,r2,r5   ; fmul r2, r3, w  ; thrsw ; ldvary
0x542030a305cea000ull, // [0x00000018] fadd tmur,r2,r5   ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000020] fadd tmus,r2,r5
0x3c913186bb800000ull, // [0x00000028] nop                                 ; ldtmu
0x3c203188b6824000ull, // [0x00000030] mov tlbu, r4                        ; thrsw
0x3c913186bb800000ull, // [0x00000038] nop                                 ; ldtmu
0x3c003187b6824000ull, // [0x00000040] mov tlb, r4
};
struct inline_qasm blit_f16tlb_3D = { blit_f16tlb_3D_instrs, countof(blit_f16tlb_3D_instrs) };
static const uint64_t blit_f32tlb_1D_instrs[] = {
0x3eb0f186bb800000ull, // [0x00000000] nop                                    ; wrtmuc; ldvary; thrsw
0x56603086bbcc0000ull, // [0x00000008] nop                  ; fmul r2, r3, w  ; wrtmuc        ; thrsw
0x3c0031a10582a000ull, // [0x00000010] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000018] nop                                    ; ldtmu; ldunif
0x3c0031887a82c000ull, // [0x00000020] umin tlbu, r4, r5
0x3cd13182b682d000ull, // [0x00000028] mov  r2, r5                            ; ldtmu; ldunif
0x3c0031877a814000ull, // [0x00000030] umin tlb, r4, r2
0x3c913186bb800000ull, // [0x00000038] nop                                    ; ldtmu
0x3c2031877a814000ull, // [0x00000040] umin tlb, r4, r2                       ; thrsw
0x3c913186bb800000ull, // [0x00000048] nop                                    ; ldtmu
0x3c0031877a82c000ull, // [0x00000050] umin tlb, r4, r5
};
struct inline_qasm blit_f32tlb_1D = { blit_f32tlb_1D_instrs, countof(blit_f32tlb_1D_instrs) };
static const uint64_t blit_f32tlb_2D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                    ; wrtmuc; ldvary
0x56b0f086bbcc0000ull, // [0x00000008] nop                  ; fmul r2, r3, w  ; thrsw; wrtmuc; ldvary
0x542030a205cea000ull, // [0x00000010] fadd tmut,r2,r5      ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000018] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000020] nop                                    ; ldtmu; ldunif
0x3c0031887a82c000ull, // [0x00000028] umin tlbu, r4, r5
0x3cd13182b682d000ull, // [0x00000030] mov  r2, r5                            ; ldtmu; ldunif
0x3c0031877a814000ull, // [0x00000038] umin tlb, r4, r2
0x3c913186bb800000ull, // [0x00000040] nop                                    ; ldtmu
0x3c2031877a814000ull, // [0x00000048] umin tlb, r4, r2                       ; thrsw
0x3c913186bb800000ull, // [0x00000050] nop                                    ; ldtmu
0x3c0031877a82c000ull, // [0x00000058] umin tlb, r4, r5
};
struct inline_qasm blit_f32tlb_2D = { blit_f32tlb_2D_instrs, countof(blit_f32tlb_2D_instrs) };
static const uint64_t blit_f32tlb_3D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                 ; wrtmuc; ldvary
0x5690f086bbcc0000ull, // [0x00000008] nop               ; fmul r2, r3, w  ; wrtmuc; ldvary
0x5530f0a205cea000ull, // [0x00000010] fadd tmut,r2,r5   ; fmul r2, r3, w  ; thrsw; ldvary
0x542030a305cea000ull, // [0x00000018] fadd tmur,r2,r5   ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000020] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000028] nop                                 ; ldtmu; ldunif
0x3c0031887a82c000ull, // [0x00000030] umin tlbu, r4, r5
0x3cd13182b682d000ull, // [0x00000038] mov  r2, r5                         ; ldtmu; ldunif
0x3c0031877a814000ull, // [0x00000040] umin tlb, r4, r2
0x3c913186bb800000ull, // [0x00000048] nop                                 ; ldtmu
0x3c2031877a814000ull, // [0x00000050] umin tlb, r4, r2                    ; thrsw
0x3c913186bb800000ull, // [0x00000058] nop                                 ; ldtmu
0x3c0031877a82c000ull, // [0x00000060] umin tlb, r4, r5
};
struct inline_qasm blit_f32tlb_3D = { blit_f32tlb_3D_instrs, countof(blit_f32tlb_3D_instrs) };
static const uint64_t blit_s32tlb_1D_instrs[] = {
0x3eb0f186bb800000ull, // [0x00000000] nop                                    ; wrtmuc; ldvary; thrsw
0x56603086bbcc0000ull, // [0x00000008] nop                  ; fmul r2, r3, w  ; wrtmuc        ; thrsw
0x3c0031a10582a000ull, // [0x00000010] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000018] nop                                    ; ldtmu; ldunif
0x3cd1308178f6c000ull, // [0x00000020] min r1, r4, r5       ; mov r2, r5      ; ldtmu; ldunif
0x3c00318879829000ull, // [0x00000028] max tlbu, r1, r5
0x3c91318178814000ull, // [0x00000030] min r1, r4, r2                         ; ldtmu
0x3c00318779829000ull, // [0x00000038] max tlb, r1, r5
0x3c91318178814000ull, // [0x00000040] min r1, r4, r2                         ; ldtmu
0x3c20318779829000ull, // [0x00000048] max tlb, r1, r5                        ; thrsw
0x3c00318178814000ull, // [0x00000050] min r1, r4, r2
0x3c00318779829000ull, // [0x00000058] max tlb, r1, r5
};
struct inline_qasm blit_s32tlb_1D = { blit_s32tlb_1D_instrs, countof(blit_s32tlb_1D_instrs) };
static const uint64_t blit_s32tlb_2D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                    ; wrtmuc; ldvary
0x56b0f086bbcc0000ull, // [0x00000008] nop                  ; fmul r2, r3, w  ; thrsw; wrtmuc; ldvary
0x542030a205cea000ull, // [0x00000010] fadd tmut,r2,r5      ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000018] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000020] nop                                    ; ldtmu; ldunif
0x3cd1308178f6c000ull, // [0x00000028] min r1, r4, r5       ; mov r2, r5      ; ldtmu; ldunif
0x3c00318879829000ull, // [0x00000030] max tlbu, r1, r5
0x3c91318178814000ull, // [0x00000038] min r1, r4, r2                         ; ldtmu
0x3c00318779829000ull, // [0x00000040] max tlb, r1, r5
0x3c91318178814000ull, // [0x00000048] min r1, r4, r2                         ; ldtmu
0x3c20318779829000ull, // [0x00000050] max tlb, r1, r5                        ; thrsw
0x3c00318178814000ull, // [0x00000058] min r1, r4, r2
0x3c00318779829000ull, // [0x00000060] max tlb, r1, r5
};
struct inline_qasm blit_s32tlb_2D = { blit_s32tlb_2D_instrs, countof(blit_s32tlb_2D_instrs) };
static const uint64_t blit_s32tlb_3D_instrs[] = {
0x3e90f186bb800000ull, // [0x00000000] nop                                    ; wrtmuc; ldvary
0x5690f086bbcc0000ull, // [0x00000008] nop                  ; fmul r2, r3, w  ; wrtmuc; ldvary
0x5530f0a205cea000ull, // [0x00000010] fadd tmut,r2,r5      ; fmul r2, r3, w  ; thrsw; ldvary
0x542030a305cea000ull, // [0x00000018] fadd tmur,r2,r5      ; fmul r2, r3, w  ; thrsw
0x3c0031a10582a000ull, // [0x00000020] fadd tmus,r2,r5
0x3cd13186bb800000ull, // [0x00000028] nop                                    ; ldtmu; ldunif
0x3cd1308178f6c000ull, // [0x00000030] min r1, r4, r5       ; mov r2, r5      ; ldtmu; ldunif
0x3c00318879829000ull, // [0x00000038] max tlbu, r1, r5
0x3c91318178814000ull, // [0x00000040] min r1, r4, r2                         ; ldtmu
0x3c00318779829000ull, // [0x00000048] max tlb, r1, r5
0x3c91318178814000ull, // [0x00000050] min r1, r4, r2                         ; ldtmu
0x3c20318779829000ull, // [0x00000058] max tlb, r1, r5                        ; thrsw
0x3c00318178814000ull, // [0x00000060] min r1, r4, r2
0x3c00318779829000ull, // [0x00000068] max tlb, r1, r5
};
struct inline_qasm blit_s32tlb_3D = { blit_s32tlb_3D_instrs, countof(blit_s32tlb_3D_instrs) };
