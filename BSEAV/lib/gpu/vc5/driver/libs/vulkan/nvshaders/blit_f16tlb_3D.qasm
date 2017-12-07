# Uniforms: tmu param0, tmu param1, tlb cfg
# Varyings: s,t,r
nop                                 ; wrtmuc; ldvary
nop               ; fmul r2, r3, w  ; wrtmuc; ldvary
fadd tmut,r2,r5   ; fmul r2, r3, w  ; thrsw ; ldvary
fadd tmur,r2,r5   ; fmul r2, r3, w  ; thrsw
fadd tmus,r2,r5
nop                                 ; ldtmu           # Red/Green
mov tlbu, r4                        ; thrsw
nop                                 ; ldtmu           # Blue/Alpha
mov tlb, r4
