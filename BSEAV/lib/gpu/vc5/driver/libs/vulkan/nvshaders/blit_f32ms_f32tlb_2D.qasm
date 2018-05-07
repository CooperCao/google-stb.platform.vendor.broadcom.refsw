# Special case for sfloat32 image multisample resolve, as the TMU doesn't
# support linear filtering of 32bit components, using texture gather
# for each colour component and computing the average manually.
#
# Uniforms: tmu param 0 [c0], tmu param 1 [c0], tmu param 2 [c0],
#           tmu param 0 [c1], tmu param 1 [c1], tmu param 2 [c1],
#           tmu param 0 [c2], tmu param 1 [c2], tmu param 2 [c2],
#           tmu param 0 [c3], tmu param 1 [c3], tmu param 2 [c3],
#           0.25f, tlb cfg
# Varyings: t,s
#
# Note: as this is run as an NV shader with the pre-computed screenspace
#       vertices having no perspective, 'w' is a constant 1.0f for all
#       pixels. Not multiplying the varying by 'w' allows us to save one
#       instruction on the first TMU load which would overwise be required
#       in order to save the multiplied result for the following lookups.
nop                                         ; wrtmuc; ldvary.rf3
nop                                         ; wrtmuc; ldvary.rf4; thrsw
fadd tmut,rf3,r5     ; mov rf0, r5          ; wrtmuc
fadd tmus,rf4,r5     ; mov rf1, r5
nop                                         ; ldtmu.rf5    # Component 0
nop                                         ; ldtmu.r4
fadd rf5, rf5, r4                           ; ldtmu.r4
fadd rf5, rf5, r4                           ; ldtmu.r4
fadd rf5, rf5, r4                           ; wrtmuc; thrsw
fadd tmut, rf3, rf0                         ; wrtmuc
fadd tmus, rf4, rf1                         ; wrtmuc
nop                                         ; ldtmu.rf6    # Component 1
nop                                         ; ldtmu.r4
fadd rf6, rf6, r4                           ; ldtmu.r4
fadd rf6, rf6, r4                           ; ldtmu.r4
fadd rf6, rf6, r4                           ; wrtmuc; thrsw
fadd tmut, rf3, rf0                         ; wrtmuc
fadd tmus, rf4, rf1                         ; wrtmuc
nop                                         ; ldtmu.rf7    # Component 2
nop                                         ; ldtmu.r4
fadd rf7, rf7, r4                           ; ldtmu.r4
fadd rf7, rf7, r4                           ; ldtmu.r4
fadd rf7, rf7, r4                           ; wrtmuc; thrsw
fadd tmut, rf3, rf0                         ; wrtmuc; thrsw
fadd tmus, rf4, rf1                         ; wrtmuc
nop                                         ; ldtmu.rf8    # Component 3
nop                                         ; ldtmu.r4
fadd rf8, rf8, r4                           ; ldtmu.r4
fadd rf8, rf8, r4                           ; ldtmu.r4; ldunif   # load 0.25f
fadd rf8, rf8, r4    ; fmul tlbu, rf5, r5
nop                  ; fmul tlb,  rf6, r5   ; thrsw
nop                  ; fmul tlb,  rf7, r5
nop                  ; fmul tlb,  rf8, r5
