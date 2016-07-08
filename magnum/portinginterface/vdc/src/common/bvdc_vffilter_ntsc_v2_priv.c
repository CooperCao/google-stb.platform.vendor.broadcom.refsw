/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *   Contains tables for Display settings.
 *
 ***************************************************************************/

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
