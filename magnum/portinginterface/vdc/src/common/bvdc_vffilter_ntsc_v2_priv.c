/***************************************************************************
 *
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   Contains tables for Display settings.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#if BVDC_P_ORTHOGONAL_VEC
#define register_NTSC_704_VF_BLOCK_CH0_TAP1_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP1               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP1               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP1               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP1               , B_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP2_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , A_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , A_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP2               , C_PHASE1                 , 0x0001 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP3_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , A_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , C_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , C_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP3               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP4_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , A_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , C_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP4               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP5_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP5               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP6_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , C_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , D_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP6               , D_PHASE1                 , 0x0001 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP7_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , C_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , D_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , E_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP7               , E_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP8_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , A_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , A_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , C_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , D_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , D_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , E_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP8               , E_PHASE1                 , 0x0001 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP9_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , C_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , D_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , D_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , E_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP9               , E_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH0_TAP10_value \
( \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , C_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , D_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , E_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , E_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , F_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH0_TAP10              , F_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP1_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP1               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP1               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP1               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP1               , B_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP2_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP2               , C_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP3_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP3               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP4_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , A_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP4               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP5_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , C_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP5               , D_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP6_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , C_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , D_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP6               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP7_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , A_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , D_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , E_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP7               , E_PHASE1                 , 0x0001 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP8_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , D_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , D_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , E_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP8               , E_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP9_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , C_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , D_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , E_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP9               , E_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH1_TAP10_value \
( \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , D_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , E_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , E_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , F_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH1_TAP10              , F_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP1_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP1               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP1               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP1               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP1               , B_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP2_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP2               , C_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP3_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP3               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP4_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , A_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP4               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP5_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , B_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , C_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , C_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP5               , D_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP6_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , A_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , B_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , C_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , D_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP6               , D_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP7_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , A_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , A_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , C_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , D_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , E_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP7               , E_PHASE1                 , 0x0001 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP8_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , B_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , B_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , C_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , D_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , D_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , E_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP8               , E_PHASE1                 , 0x0000 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP9_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , A_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , A_PHASE1                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , B_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , B_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , C_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , D_PHASE0                 , 0x0001 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , D_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , E_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP9               , E_PHASE1                 , 0x0002 ) \
)
#define register_NTSC_704_VF_BLOCK_CH2_TAP10_value \
( \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , A_PHASE0                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , A_PHASE1                 , 0x0002 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , B_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , B_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , C_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , C_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , D_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , D_PHASE1                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , E_PHASE0                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , E_PHASE1                 , 0x0003 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , F_PHASE0                 , 0x0000 ) | \
  BVDC_P_VF_FIELD_DATA( CH2_TAP10              , F_PHASE1                 , 0x0000 ) \
)
#endif
