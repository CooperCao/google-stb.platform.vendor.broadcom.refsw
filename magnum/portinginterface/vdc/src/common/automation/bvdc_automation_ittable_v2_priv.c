/***************************************************************************
 * Copyright (C) 2019 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *   Contains tables automatically generated from VEC design files
 *
 ***************************************************************************/

/*
This file generated automatically on 2015-09-29 16:05
    by program process_all_tags

Register programming derived from
    /vobs/DVTSJ/portinginterface/vdc/7422/A0/CONSOLIDATED_VEC
*/


/* DVTSJ format tag "480i" */
/* From IT_REGISTERS/prog/IT_REGISTERS_480i.txt: */
static const uint32_t s_aulItTable_480i[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "576i" */
/* From IT_REGISTERS/prog/IT_REGISTERS_576i.txt: */
static const uint32_t s_aulItTable_576i[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "480p" */
/* From IT_REGISTERS/prog/IT_REGISTERS_480p.txt: */
static const uint32_t s_aulItTable_480p[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "576p" */
/* From IT_REGISTERS/prog/IT_REGISTERS_576p.txt: */
static const uint32_t s_aulItTable_576p[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "480p54" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_480p54.txt: */
#define s_aulItTable_480p54 s_aulItTable_576p

/* DVTSJ format tag "576p54" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_576p.txt: */
#define s_aulItTable_576p54 s_aulItTable_576p

/* DVTSJ format tag "1080i" */
/* From IT_REGISTERS/prog/IT_REGISTERS_1080i.txt: */
static const uint32_t s_aulItTable_1080i[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x0032 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0007 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "1080i_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080i_50hz.txt: */
#define s_aulItTable_1080i_50hz s_aulItTable_1080i

/* DVTSJ format tag "1250i_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1250i_50hz.txt: */
#define s_aulItTable_1250i_50hz s_aulItTable_1080i

/* DVTSJ format tag "720p" */
/* From IT_REGISTERS/prog/IT_REGISTERS_720p.txt: */
static const uint32_t s_aulItTable_720p[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x0032 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0007 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "720p_24hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_24hz.txt: */
#define s_aulItTable_720p_24hz s_aulItTable_720p

/* DVTSJ format tag "720p_25hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_25hz.txt: */
#define s_aulItTable_720p_25hz s_aulItTable_720p

/* DVTSJ format tag "720p_30hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_30hz.txt: */
#define s_aulItTable_720p_30hz s_aulItTable_720p

/* DVTSJ format tag "720p_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_720p_50hz.txt: */
#define s_aulItTable_720p_50hz s_aulItTable_720p

/* DVTSJ format tag "1080p_24hz" */
/* From IT_REGISTERS/prog/IT_REGISTERS_1080p_24hz.txt: */
static const uint32_t s_aulItTable_1080p_24hz[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x0032 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0007 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "1080p_25hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_25hz.txt: */
#define s_aulItTable_1080p_25hz s_aulItTable_1080p_24hz

/* DVTSJ format tag "1080p_30hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_30hz.txt: */
#define s_aulItTable_1080p_30hz s_aulItTable_1080p_24hz

/* DVTSJ format tag "1080p_50hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_50hz_OSCL.txt: */
#define s_aulItTable_1080p_50hz s_aulItTable_1080p_24hz

/* DVTSJ format tag "1080p_60hz" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_1080p_60hz_OSCL.txt: */
#define s_aulItTable_1080p_60hz s_aulItTable_1080p_24hz

/* DVTSJ format tag "NTSC_ITU" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_NTSC_ITU.txt: */
#define s_aulItTable_NTSC_ITU s_aulItTable_576i

/* DVTSJ format tag "NTSC_704" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_NTSC_ITU.txt: */
#define s_aulItTable_NTSC_704 s_aulItTable_NTSC_ITU

/* DVTSJ format tag "NTSC_J" */
/* From IT_REGISTERS/prog/IT_REGISTERS_NTSC_SMPTE.txt: */
static const uint32_t s_aulItTable_NTSC_J[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};

/* DVTSJ format tag "PAL" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL s_aulItTable_576i

/* DVTSJ format tag "PAL_I" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL_I s_aulItTable_PAL

/* DVTSJ format tag "PAL_IA" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL_IA s_aulItTable_PAL

/* DVTSJ format tag "PAL_DK" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL_DK s_aulItTable_PAL

/* DVTSJ format tag "PAL_N" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL_N s_aulItTable_PAL

/* DVTSJ format tag "PAL_NC" */
/* Another instance of IT_REGISTERS/prog/IT_REGISTERS_PAL.txt: */
#define s_aulItTable_PAL_NC s_aulItTable_PAL

/* DVTSJ format tag "PAL_M" */
/* Duplicated values in IT_REGISTERS/prog/IT_REGISTERS_PAL_M.txt: */
#define s_aulItTable_PAL_M s_aulItTable_576i

/* DVTSJ format tag "SECAM" */
/* From IT_REGISTERS/prog/IT_REGISTERS_SECAM.txt: */
static const uint32_t s_aulItTable_SECAM[BVDC_P_IT_TABLE_SIZE] =
{
(
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_0_START_ADDR           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_1_START_ADDR           , 0x0014 ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_2_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_0_3       , MC_3_START_ADDR           , 0x00fd )
),
(
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_4_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_5_START_ADDR           , 0x00fd ) |
  BVDC_P_IT_FIELD_DATA( ADDR_4_6       , MC_6_START_ADDR           , 0x0032 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_0                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_0_1  , REG_1                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_2                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_2_3  , REG_3                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_4                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_4_5  , REG_5                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_6                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_6_7  , REG_7                     , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_1                      , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_2                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_3                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_4                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_5                      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( EVENT_SELECTION, MC_6                      , 0x0006 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_0        , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_1        , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_2        , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_3        , 0x0004 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_AND_TERM_4        , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_3_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_4_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_D_SELECT      , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , NEGSYNC_MUX_E_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_ENABLE    , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_0          , VBI_DATA_ACTIVE_MUX_SELECT, 0x0003 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_ENABLE            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_0_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , BOTTLES_MUX_A_SELECT      , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_0    , 0x0007 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_1    , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_AND_TERM_2    , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_1_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_2_SELECT  , 0x0002 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_B_SELECT  , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_1          , COLOR_BURST_MUX_C_SELECT  , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_HSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_ENABLE          , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , EXT_VSYNC_MUX_SELECT      , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_ENABLE       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_0_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , SEC_NEG_SYNC_MUX_A_SELECT , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_0_SELECT       , 0x0007 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , U_FLIP_MUX_A_SELECT       , 0x0006 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_0_SELECT       , 0x0005 ) |
  BVDC_P_IT_FIELD_DATA( PCL_2          , V_FLIP_MUX_A_SELECT       , 0x0005 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , H_ACTIVE_MUX_SELECT       , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_ENABLE   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , LINE_COUNT_CLEAR_SELECT   , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , NEW_LINE_MUX_SELECT       , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , ODD_EVEN_MUX_SELECT       , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_ENABLE             , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VBLANK_MUX_SELECT         , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_ENABLE              , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , VSYNC_MUX_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_ENABLE           , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_3          , V_ACTIVE_MUX_SELECT       , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_0            , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_0_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_A_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSA_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_AND_TERM_2            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_2_SELECT          , 0x0003 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_4          , PSB_MUX_C_SELECT          , 0x0000 )
),
#if (BVDC_P_SUPPORT_IT_VER >= 1)
(
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID                  , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_0_SELECT     , 0x0001 ) |
  BVDC_P_IT_FIELD_DATA( PCL_5          , MV_VALID_MUX_A_SELECT     , 0x0001 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSC_MUX_B_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_0            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_AND_TERM_1            , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_0_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_1_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_A_SELECT          , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_6          , PSD_MUX_B_SELECT          , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_7          , NSD1_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_0           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_1           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_2           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_3           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_AND_TERM_4           , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_0_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_1_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_2_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_3_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_4_SELECT         , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( PCL_8          , NSD2_MUX_A_SELECT         , 0x0000 )
),
(
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_8                     , 0x0000 ) |
  BVDC_P_IT_FIELD_DATA( STACK_reg_8_9  , REG_9                     , 0x0000 )
),
#endif
};
