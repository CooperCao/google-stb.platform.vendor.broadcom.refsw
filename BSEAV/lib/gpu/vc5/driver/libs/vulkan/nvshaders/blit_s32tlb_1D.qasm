# Uniforms: tmu param 0, tmu param 1, clampVal1 (+ve), clampVal2 (-ve), tlb cfg
# Varyings: s
nop                                    ; wrtmuc; ldvary; thrsw
nop                  ; fmul r2, r3, w  ; wrtmuc        ; thrsw
fadd tmus,r2,r5                                          # Texture lookup + thread yield
nop                                    ; ldtmu; ldunif   # load Red + clampVal1
min r1, r4, r5       ; mov r2, r5      ; ldtmu; ldunif   # load Green + clampVal2
max tlbu, r1, r5
min r1, r4, r2                         ; ldtmu           # load Blue
max tlb, r1, r5
min r1, r4, r2                         ; ldtmu           # load Alpha
max tlb, r1, r5                        ; thrsw
min r1, r4, r2
max tlb, r1, r5
