/***************************************************************************
 *
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description:
 *   Contains tables automatically generated from VEC design files
 *
 ***************************************************************************/

/*
This file generated automatically on 2015-10-09 16:37
    by program process_all_tags

Register programming derived from
    /vobs/DVTSJ/portinginterface/vdc/7420/A0/CONSOLIDATED_VEC
*/


/* DVTSJ format tag "480i" */
/* From VF_REGISTERS/prog/VF_REGISTERS_480i.txt: */
static const uint32_t s_aulVfTable_480i[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0200 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x02b1 ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0230 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "576i" */
/* From VF_REGISTERS/prog/VF_REGISTERS_576i.txt: */
static const uint32_t s_aulVfTable_576i[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0200 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x02b1 ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0230 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "480p" */
/* From VF_REGISTERS/prog/VF_REGISTERS_480p.txt: */
static const uint32_t s_aulVfTable_480p[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x00f7 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x001e ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x0059 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x0084 )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x00eb ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x00eb )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x0003 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0017 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0003 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0017 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "576p" */
/* From VF_REGISTERS/prog/VF_REGISTERS_576p.txt: */
static const uint32_t s_aulVfTable_576p[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x00f7 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0009 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x02b1 ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0230 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x0003 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0033 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0003 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0033 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "480p54" */
/* From VF_REGISTERS/prog/VF_REGISTERS_480p54.txt: */
static const uint32_t s_aulVfTable_480p54[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0200 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x001e ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x0109 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x0109 )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x00eb ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x00eb )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0003 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0033 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "576p54" */
/* From VF_REGISTERS/prog/VF_REGISTERS_576p54.txt: */
static const uint32_t s_aulVfTable_576p54[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0200 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x026b ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0225 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "1080i" */
/* From VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
static const uint32_t s_aulVfTable_1080i[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x00f7 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x00eb ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x00eb )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "1080i_50hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080i_50hz s_aulVfTable_1080i

/* DVTSJ format tag "1250i_50hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1250i_50hz s_aulVfTable_1080i

/* DVTSJ format tag "720p" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_720p s_aulVfTable_1080i

/* DVTSJ format tag "720p_24hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_720p_24hz s_aulVfTable_1080i

/* DVTSJ format tag "720p_25hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_720p_25hz s_aulVfTable_1080i

/* DVTSJ format tag "720p_30hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_720p_30hz s_aulVfTable_1080i

/* DVTSJ format tag "720p_50hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_720p_50hz s_aulVfTable_1080i

/* DVTSJ format tag "1080p_24hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080p_24hz s_aulVfTable_1080i

/* DVTSJ format tag "1080p_25hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080p_25hz s_aulVfTable_1080i

/* DVTSJ format tag "1080p_30hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080p_30hz s_aulVfTable_1080i

/* DVTSJ format tag "1080p_50hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080p_50hz s_aulVfTable_1080i

/* DVTSJ format tag "1080p_60hz" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_720p.txt: */
#define s_aulVfTable_1080p_60hz s_aulVfTable_1080i

/* DVTSJ format tag "NTSC_ITU" */
/* From VF_REGISTERS/prog/VF_REGISTERS_NTSC_704.txt: */
static const uint32_t s_aulVfTable_NTSC_ITU[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00f0 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00f0 )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x02b1 ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0230 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "NTSC_704" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_NTSC_704.txt: */
#define s_aulVfTable_NTSC_704 s_aulVfTable_NTSC_ITU

/* DVTSJ format tag "NTSC_J" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_NTSC_704.txt: */
#define s_aulVfTable_NTSC_J s_aulVfTable_NTSC_ITU

/* DVTSJ format tag "PAL" */
/* From VF_REGISTERS/prog/VF_REGISTERS_PAL.txt: */
static const uint32_t s_aulVfTable_PAL[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x026b ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0225 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "PAL_I" */
/* Duplicated values in VF_REGISTERS/prog/VF_REGISTERS_PAL_I.txt: */
#define s_aulVfTable_PAL_I s_aulVfTable_PAL

/* DVTSJ format tag "PAL_IA" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_PAL_I.txt: */
#define s_aulVfTable_PAL_IA s_aulVfTable_PAL_I

/* DVTSJ format tag "PAL_DK" */
/* Duplicated values in VF_REGISTERS/prog/VF_REGISTERS_PAL_DK.txt: */
#define s_aulVfTable_PAL_DK s_aulVfTable_PAL

/* DVTSJ format tag "PAL_N" */
/* From VF_REGISTERS/prog/VF_REGISTERS_PAL_NC.txt: */
static const uint32_t s_aulVfTable_PAL_N[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00fb ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00fb )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x026b ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0225 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "PAL_NC" */
/* Another instance of VF_REGISTERS/prog/VF_REGISTERS_PAL_NC.txt: */
#define s_aulVfTable_PAL_NC s_aulVfTable_PAL_N

/* DVTSJ format tag "PAL_M" */
/* From VF_REGISTERS/prog/VF_REGISTERS_PAL_M.txt: */
static const uint32_t s_aulVfTable_PAL_M[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0002 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00f0 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00f0 )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x026b ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x0225 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0007 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x001c ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0046 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};

/* DVTSJ format tag "SECAM" */
/* From VF_REGISTERS/prog/VF_REGISTERS_SECAM.txt: */
static const uint32_t s_aulVfTable_SECAM[BVDC_P_VF_TABLE_SIZE+1] =
{
(
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , ADD_SYNC_TO_OFFSET       , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_POSITIVESYNC          , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C0_SYNC                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C1_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_COMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_OFFSET                , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , C2_POSITIVESYNC          , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , CLAMP_MODE               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , OFFSET                   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( FORMAT_ADDER           , SECOND_NEGATIVE_SYNC     , 0x0000 )
),
(
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_BOTTOM   , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_LINE_REMOVE_TOP      , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , BVB_SAV_REMOVE           , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C0_RAMP                  , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C1_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , C2_RAMP                  , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , NON_UPSAMPLE_TAPS_USE_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , SUM_OF_TAPS              , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE2X               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH0, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH1, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , UPSAMPLE_TAP_SYMMETRY_CH2, 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_ENABLE               , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VBI_PREFERRED            , 0x0001 ) |
  BVDC_P_VF_FIELD_DATA( MISC                   , VF_ENABLE                , 0x0001 )
),
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE0                   , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE1                   , 0x00f0 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_VALUES        , VALUE2                   , 0x00f0 )
),
(
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE0                   , 0x00f0 ) |
  BVDC_P_VF_FIELD_DATA( POS_SYNC_VALUES        , VALUE1                   , 0x00f0 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP0                     , 0x0005 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP1                     , 0x0010 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP2                     , 0x0034 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_0           , TAP3                     , 0x0044 )
),
(
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP0                     , 0x0014 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP1                     , 0x002e ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP2                     , 0x0048 ) |
  BVDC_P_VF_FIELD_DATA( SYNC_TRANS_1           , TAP3                     , 0x0064 )
),
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
(
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, SEL_D_SYNC               , 0x0000 ) |
  BVDC_P_VF_FIELD_DATA( NEG_SYNC_AMPLITUDE_EXTN, VALUE3                   , 0x0010 )
),
#endif
};
