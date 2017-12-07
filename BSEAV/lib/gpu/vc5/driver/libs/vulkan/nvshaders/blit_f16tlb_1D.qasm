# Uniforms: tmu param0, tmu param1, tlb cfg
# Varyings: s
nop                                 ; wrtmuc; ldvary; thrsw
nop               ; fmul r2, r3, w  ; wrtmuc        ; thrsw
fadd tmus,r2,r5                                       # texture lookup + thread yield
nop                                 ; ldtmu           # Red/Green
mov tlbu, r4                        ; thrsw
nop                                 ; ldtmu           # Blue/Alpha
mov tlb, r4
