# Uniforms: tmu param 0, tmu param 1, clampVal1 (RGB), tlb cfg, clampVal2 (Alpha)
# Varyings: s
nop                                    ; wrtmuc; ldvary; thrsw
nop                  ; fmul r2, r3, w  ; wrtmuc        ; thrsw
fadd tmus,r2,r5                                          # Texture lookup + thread yield
nop                                    ; ldtmu; ldunif   # Red + clampVal1
umin tlbu, r4, r5
mov  r2, r5                            ; ldtmu; ldunif   # Green + clampVal2
umin tlb, r4, r2
nop                                    ; ldtmu           # Blue
umin tlb, r4, r2                       ; thrsw
nop                                    ; ldtmu           # Alpha
umin tlb, r4, r5
