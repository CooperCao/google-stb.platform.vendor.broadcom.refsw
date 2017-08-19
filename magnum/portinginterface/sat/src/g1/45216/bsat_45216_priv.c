/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
*****************************************************************************/
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"
#include "bsat_45216_priv.h"
#include "bchp_tm.h"
#include "bchp_jtag_otp.h"


#if (BCHP_CHIP != 45216)
#error "This file is for BCM45216 firmware only"
#endif

BDBG_MODULE(bsat_45216_priv);

#define BSAT_DEBUG_POWERDOWN(x) /* x */


const uint32_t BSAT_g1_ChannelIntrID[BSAT_G1_MAX_CHANNELS][BSAT_g1_MaxIntID] =
{
   /* channel 0 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_0,
      BCHP_INT_ID_SDS_NOT_LOCK_0,
      BCHP_INT_ID_SDS_BTM_0,
      BCHP_INT_ID_SDS_BRTM_0,
      BCHP_INT_ID_SDS_GENTM1_0,
      BCHP_INT_ID_SDS_GENTM2_0,
      BCHP_INT_ID_SDS_GENTM3_0,
      BCHP_INT_ID_MI2C_0,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_0,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_0,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_0,
      BCHP_INT_ID_SDS_DFT_DONE_0,
      BCHP_INT_ID_AFEC_LOCK_0,
      BCHP_INT_ID_AFEC_NOT_LOCK_0,
      BCHP_INT_ID_AFEC_MP_LOCK_0,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_0,
      BCHP_INT_ID_TFEC_LOCK_0,
      BCHP_INT_ID_TFEC_NOT_LOCK_0,
      BCHP_INT_ID_TFEC_SYNC_0
   },
   /* channel 1 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_1,
      BCHP_INT_ID_SDS_NOT_LOCK_1,
      BCHP_INT_ID_SDS_BTM_1,
      BCHP_INT_ID_SDS_BRTM_1,
      BCHP_INT_ID_SDS_GENTM1_1,
      BCHP_INT_ID_SDS_GENTM2_1,
      BCHP_INT_ID_SDS_GENTM3_1,
      BCHP_INT_ID_MI2C_1,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_1,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_1,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_1,
      BCHP_INT_ID_SDS_DFT_DONE_1,
      BCHP_INT_ID_AFEC_LOCK_1,
      BCHP_INT_ID_AFEC_NOT_LOCK_1,
      BCHP_INT_ID_AFEC_MP_LOCK_1,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_1,
      BCHP_INT_ID_TFEC_LOCK_1,
      BCHP_INT_ID_TFEC_NOT_LOCK_1,
      BCHP_INT_ID_TFEC_SYNC_1
   },
   /* channel 2 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_2,
      BCHP_INT_ID_SDS_NOT_LOCK_2,
      BCHP_INT_ID_SDS_BTM_2,
      BCHP_INT_ID_SDS_BRTM_2,
      BCHP_INT_ID_SDS_GENTM1_2,
      BCHP_INT_ID_SDS_GENTM2_2,
      BCHP_INT_ID_SDS_GENTM3_2,
      BCHP_INT_ID_MI2C_2,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_2,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_2,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_2,
      BCHP_INT_ID_SDS_DFT_DONE_2,
      BCHP_INT_ID_AFEC_LOCK_2,
      BCHP_INT_ID_AFEC_NOT_LOCK_2,
      BCHP_INT_ID_AFEC_MP_LOCK_2,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_2,
      BCHP_INT_ID_TFEC_LOCK_2,
      BCHP_INT_ID_TFEC_NOT_LOCK_2,
      BCHP_INT_ID_TFEC_SYNC_2
   },
   /* channel 3 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_3,
      BCHP_INT_ID_SDS_NOT_LOCK_3,
      BCHP_INT_ID_SDS_BTM_3,
      BCHP_INT_ID_SDS_BRTM_3,
      BCHP_INT_ID_SDS_GENTM1_3,
      BCHP_INT_ID_SDS_GENTM2_3,
      BCHP_INT_ID_SDS_GENTM3_3,
      BCHP_INT_ID_MI2C_3,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_3,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_3,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_3,
      BCHP_INT_ID_SDS_DFT_DONE_3,
      BCHP_INT_ID_AFEC_LOCK_3,
      BCHP_INT_ID_AFEC_NOT_LOCK_3,
      BCHP_INT_ID_AFEC_MP_LOCK_3,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_3,
      BCHP_INT_ID_TFEC_LOCK_3,
      BCHP_INT_ID_TFEC_NOT_LOCK_3,
      BCHP_INT_ID_TFEC_SYNC_3
   },
   /* channel 4 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_4,
      BCHP_INT_ID_SDS_NOT_LOCK_4,
      BCHP_INT_ID_SDS_BTM_4,
      BCHP_INT_ID_SDS_BRTM_4,
      BCHP_INT_ID_SDS_GENTM1_4,
      BCHP_INT_ID_SDS_GENTM2_4,
      BCHP_INT_ID_SDS_GENTM3_4,
      BCHP_INT_ID_MI2C_4,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_4,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_4,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_4,
      BCHP_INT_ID_SDS_DFT_DONE_4,
      BCHP_INT_ID_AFEC_LOCK_4,
      BCHP_INT_ID_AFEC_NOT_LOCK_4,
      BCHP_INT_ID_AFEC_MP_LOCK_4,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_4,
      BCHP_INT_ID_TFEC_LOCK_4,
      BCHP_INT_ID_TFEC_NOT_LOCK_4,
      BCHP_INT_ID_TFEC_SYNC_4
   },
   /* channel 5 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_5,
      BCHP_INT_ID_SDS_NOT_LOCK_5,
      BCHP_INT_ID_SDS_BTM_5,
      BCHP_INT_ID_SDS_BRTM_5,
      BCHP_INT_ID_SDS_GENTM1_5,
      BCHP_INT_ID_SDS_GENTM2_5,
      BCHP_INT_ID_SDS_GENTM3_5,
      BCHP_INT_ID_MI2C_5,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_5,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_5,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_5,
      BCHP_INT_ID_SDS_DFT_DONE_5,
      BCHP_INT_ID_AFEC_LOCK_5,
      BCHP_INT_ID_AFEC_NOT_LOCK_5,
      BCHP_INT_ID_AFEC_MP_LOCK_5,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_5,
      BCHP_INT_ID_TFEC_LOCK_5,
      BCHP_INT_ID_TFEC_NOT_LOCK_5,
      BCHP_INT_ID_TFEC_SYNC_5
   },
   /* channel 6 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_6,
      BCHP_INT_ID_SDS_NOT_LOCK_6,
      BCHP_INT_ID_SDS_BTM_6,
      BCHP_INT_ID_SDS_BRTM_6,
      BCHP_INT_ID_SDS_GENTM1_6,
      BCHP_INT_ID_SDS_GENTM2_6,
      BCHP_INT_ID_SDS_GENTM3_6,
      BCHP_INT_ID_MI2C_6,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_6,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_6,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_6,
      BCHP_INT_ID_SDS_DFT_DONE_6,
      BCHP_INT_ID_AFEC_LOCK_6,
      BCHP_INT_ID_AFEC_NOT_LOCK_6,
      BCHP_INT_ID_AFEC_MP_LOCK_6,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_6,
      BCHP_INT_ID_TFEC_LOCK_6,
      BCHP_INT_ID_TFEC_NOT_LOCK_6,
      BCHP_INT_ID_TFEC_SYNC_6
   },
   /* channel 7 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_7,
      BCHP_INT_ID_SDS_NOT_LOCK_7,
      BCHP_INT_ID_SDS_BTM_7,
      BCHP_INT_ID_SDS_BRTM_7,
      BCHP_INT_ID_SDS_GENTM1_7,
      BCHP_INT_ID_SDS_GENTM2_7,
      BCHP_INT_ID_SDS_GENTM3_7,
      BCHP_INT_ID_MI2C_7,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_7,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_7,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_7,
      BCHP_INT_ID_SDS_DFT_DONE_7,
      BCHP_INT_ID_AFEC_LOCK_7,
      BCHP_INT_ID_AFEC_NOT_LOCK_7,
      BCHP_INT_ID_AFEC_MP_LOCK_7,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_7,
      BCHP_INT_ID_TFEC_LOCK_7,
      BCHP_INT_ID_TFEC_NOT_LOCK_7,
      BCHP_INT_ID_TFEC_SYNC_7
   },
   /* channel 8 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_8,
      BCHP_INT_ID_SDS_NOT_LOCK_8,
      BCHP_INT_ID_SDS_BTM_8,
      BCHP_INT_ID_SDS_BRTM_8,
      BCHP_INT_ID_SDS_GENTM1_8,
      BCHP_INT_ID_SDS_GENTM2_8,
      BCHP_INT_ID_SDS_GENTM3_8,
      BCHP_INT_ID_MI2C_8,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_8,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_8,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_8,
      BCHP_INT_ID_SDS_DFT_DONE_8,
      BCHP_INT_ID_AFEC_LOCK_8,
      BCHP_INT_ID_AFEC_NOT_LOCK_8,
      BCHP_INT_ID_AFEC_MP_LOCK_8,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_8,
      BCHP_INT_ID_TFEC_LOCK_8,
      BCHP_INT_ID_TFEC_NOT_LOCK_8,
      BCHP_INT_ID_TFEC_SYNC_8
   },
   /* channel 9 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_9,
      BCHP_INT_ID_SDS_NOT_LOCK_9,
      BCHP_INT_ID_SDS_BTM_9,
      BCHP_INT_ID_SDS_BRTM_9,
      BCHP_INT_ID_SDS_GENTM1_9,
      BCHP_INT_ID_SDS_GENTM2_9,
      BCHP_INT_ID_SDS_GENTM3_9,
      BCHP_INT_ID_MI2C_9,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_9,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_9,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_9,
      BCHP_INT_ID_SDS_DFT_DONE_9,
      BCHP_INT_ID_AFEC_LOCK_9,
      BCHP_INT_ID_AFEC_NOT_LOCK_9,
      BCHP_INT_ID_AFEC_MP_LOCK_9,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_9,
      BCHP_INT_ID_TFEC_LOCK_9,
      BCHP_INT_ID_TFEC_NOT_LOCK_9,
      BCHP_INT_ID_TFEC_SYNC_9
   },
   /* channel 10 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_10,
      BCHP_INT_ID_SDS_NOT_LOCK_10,
      BCHP_INT_ID_SDS_BTM_10,
      BCHP_INT_ID_SDS_BRTM_10,
      BCHP_INT_ID_SDS_GENTM1_10,
      BCHP_INT_ID_SDS_GENTM2_10,
      BCHP_INT_ID_SDS_GENTM3_10,
      BCHP_INT_ID_MI2C_10,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_10,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_10,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_10,
      BCHP_INT_ID_SDS_DFT_DONE_10,
      BCHP_INT_ID_AFEC_LOCK_10,
      BCHP_INT_ID_AFEC_NOT_LOCK_10,
      BCHP_INT_ID_AFEC_MP_LOCK_10,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_10,
      BCHP_INT_ID_TFEC_LOCK_10,
      BCHP_INT_ID_TFEC_NOT_LOCK_10,
      BCHP_INT_ID_TFEC_SYNC_10
   },
   /* channel 11 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_11,
      BCHP_INT_ID_SDS_NOT_LOCK_11,
      BCHP_INT_ID_SDS_BTM_11,
      BCHP_INT_ID_SDS_BRTM_11,
      BCHP_INT_ID_SDS_GENTM1_11,
      BCHP_INT_ID_SDS_GENTM2_11,
      BCHP_INT_ID_SDS_GENTM3_11,
      BCHP_INT_ID_MI2C_11,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_11,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_11,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_11,
      BCHP_INT_ID_SDS_DFT_DONE_11,
      BCHP_INT_ID_AFEC_LOCK_11,
      BCHP_INT_ID_AFEC_NOT_LOCK_11,
      BCHP_INT_ID_AFEC_MP_LOCK_11,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_11,
      BCHP_INT_ID_TFEC_LOCK_11,
      BCHP_INT_ID_TFEC_NOT_LOCK_11,
      BCHP_INT_ID_TFEC_SYNC_11
   },
   /* channel 12 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_12,
      BCHP_INT_ID_SDS_NOT_LOCK_12,
      BCHP_INT_ID_SDS_BTM_12,
      BCHP_INT_ID_SDS_BRTM_12,
      BCHP_INT_ID_SDS_GENTM1_12,
      BCHP_INT_ID_SDS_GENTM2_12,
      BCHP_INT_ID_SDS_GENTM3_12,
      BCHP_INT_ID_MI2C_12,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_12,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_12,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_12,
      BCHP_INT_ID_SDS_DFT_DONE_12,
      BCHP_INT_ID_AFEC_LOCK_12,
      BCHP_INT_ID_AFEC_NOT_LOCK_12,
      BCHP_INT_ID_AFEC_MP_LOCK_12,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_12,
      BCHP_INT_ID_TFEC_LOCK_12,
      BCHP_INT_ID_TFEC_NOT_LOCK_12,
      BCHP_INT_ID_TFEC_SYNC_12
   },
   /* channel 13 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_13,
      BCHP_INT_ID_SDS_NOT_LOCK_13,
      BCHP_INT_ID_SDS_BTM_13,
      BCHP_INT_ID_SDS_BRTM_13,
      BCHP_INT_ID_SDS_GENTM1_13,
      BCHP_INT_ID_SDS_GENTM2_13,
      BCHP_INT_ID_SDS_GENTM3_13,
      BCHP_INT_ID_MI2C_13,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_13,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_13,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_13,
      BCHP_INT_ID_SDS_DFT_DONE_13,
      BCHP_INT_ID_AFEC_LOCK_13,
      BCHP_INT_ID_AFEC_NOT_LOCK_13,
      BCHP_INT_ID_AFEC_MP_LOCK_13,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_13,
      BCHP_INT_ID_TFEC_LOCK_13,
      BCHP_INT_ID_TFEC_NOT_LOCK_13,
      BCHP_INT_ID_TFEC_SYNC_13
   },
   /* channel 14 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_14,
      BCHP_INT_ID_SDS_NOT_LOCK_14,
      BCHP_INT_ID_SDS_BTM_14,
      BCHP_INT_ID_SDS_BRTM_14,
      BCHP_INT_ID_SDS_GENTM1_14,
      BCHP_INT_ID_SDS_GENTM2_14,
      BCHP_INT_ID_SDS_GENTM3_14,
      BCHP_INT_ID_MI2C_14,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_14,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_14,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_14,
      BCHP_INT_ID_SDS_DFT_DONE_14,
      BCHP_INT_ID_AFEC_LOCK_14,
      BCHP_INT_ID_AFEC_NOT_LOCK_14,
      BCHP_INT_ID_AFEC_MP_LOCK_14,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_14,
      BCHP_INT_ID_TFEC_LOCK_14,
      BCHP_INT_ID_TFEC_NOT_LOCK_14,
      BCHP_INT_ID_TFEC_SYNC_14
   },
   /* channel 15 interrupts */
   {
      BCHP_INT_ID_SDS_LOCK_15,
      BCHP_INT_ID_SDS_NOT_LOCK_15,
      BCHP_INT_ID_SDS_BTM_15,
      BCHP_INT_ID_SDS_BRTM_15,
      BCHP_INT_ID_SDS_GENTM1_15,
      BCHP_INT_ID_SDS_GENTM2_15,
      BCHP_INT_ID_SDS_GENTM3_15,
      BCHP_INT_ID_MI2C_15,
      BCHP_INT_ID_HP_FRAME_BOUNDARY_15,
      BCHP_INT_ID_SDS_HP_STATE_MATCH_15,
      BCHP_INT_ID_SDS_HP_STATE_CHANGE_15,
      BCHP_INT_ID_SDS_DFT_DONE_15,
      BCHP_INT_ID_AFEC_LOCK_15,
      BCHP_INT_ID_AFEC_NOT_LOCK_15,
      BCHP_INT_ID_AFEC_MP_LOCK_15,
      BCHP_INT_ID_AFEC_MP_NOT_LOCK_15,
      BCHP_INT_ID_TFEC_LOCK_15,
      BCHP_INT_ID_TFEC_NOT_LOCK_15,
      BCHP_INT_ID_TFEC_SYNC_15
   }
};


/******************************************************************************
 BSAT_g1_P_GetTotalChannels()
******************************************************************************/
BERR_Code BSAT_g1_P_GetTotalChannels(BSAT_Handle h, uint32_t *totalChannels)
{
   BSAT_g1_P_Handle *hDev = (BSAT_g1_P_Handle *)(h->pImpl);
#if 0
   uint32_t product_id, chip_id, n;

   product_id = BREG_Read32(hDev->hRegister, BCHP_TM_PRODUCT_ID);
   chip_id = product_id >> 8;
   n = chip_id & 0xFF;
   if (n == 0x16)
      *totalChannels = BSAT_G1_MAX_CHANNELS;
   else if (n == 0x04)
      *totalChannels = 4;
   else if (n == 0x06)
      *totalChannels = 6;
   else /* assume 45216 if product_id=0 */
      *totalChannels = 16;
#else
   uint32_t status9, i;

   status9 = ~(BREG_Read32(hDev->hRegister, BCHP_JTAG_OTP_GENERAL_STATUS_9)) & 0x0000FFFF;
   for (i = 0; i < 16; i++)
   {
      if ((status9 & (1<<i)) == 0)
         break;
   }
   *totalChannels = i;
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ValidateAcqParams()
******************************************************************************/
BERR_Code BSAT_g1_P_ValidateAcqParams(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams)
{
   BSTD_UNUSED(h);

   if (((pParams->mode >= BSAT_Mode_eDvbs2_16apsk_2_3) && (pParams->mode <= BSAT_Mode_eDvbs2_32apsk_9_10)) || (pParams->mode == BSAT_Mode_eDvbs2_ACM))
   {
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   }
   if (BSAT_MODE_IS_DVBS2X(pParams->mode))
   {
      BDBG_ERR(("DVB-S2X not supported"));
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   }
   if ((pParams->options & BSAT_ACQ_NYQUIST_MASK) == BSAT_ACQ_NYQUIST_5)
      return (BERR_TRACE(BERR_NOT_SUPPORTED));
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetRegisterAddress_isrsafe()
******************************************************************************/
void BSAT_g1_P_GetRegisterAddress_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t *pAddr)
{
   *pAddr = reg;
   uint32_t i;

   if (h->channel > 0)
   {
      if ((reg >= BCHP_SDS_CG_0_RSTCTL) && (reg <= BCHP_SDS_GR_BRIDGE_0_SW_INIT_1))
      {
         /* SDS register access */
         *pAddr += ((uint32_t)(h->channel) * 0x100000);
      }
      else if ((reg >= BCHP_TFEC_0_TFECTL) && (reg <= BCHP_TFEC_GR_BRIDGE_0_SW_INIT_1))
      {
         /* TFEC register access */
         *pAddr += ((uint32_t)(h->channel) * 0x100000);
      }
      else if ((reg >= BCHP_STB_CHAN_CTRL_0_REVID) && (reg <= BCHP_STB_CHAN_CTRL_0_SW_SPARE1))
      {
         /* STB_CHAN_CTRL register access */
         if (h->channel >= 8)
            *pAddr += 0x100000;
      }
      else if ((reg >= BCHP_STB_CHAN_CHx_DEC_FCW) && (reg <= BCHP_STB_CHAN_CHx_SW_SPARE1))
      {
         /* STB_CHAN_CHx register access */
         if (h->channel >= 8)
         {
            *pAddr += 0x100000;
            i = h->channel - 8;
         }
         else
            i = h->channel;
         *pAddr += (i * 0x100);
      }
      else if ((reg >= BCHP_AFEC_RST) && (reg <= BCHP_AFEC_FAKEFRM_PARAM))
      {
         /* AFEC register access */
         if (h->channel & 1)
            *pAddr += 0x1000;
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x100000);
      }
      else if ((reg >= BCHP_AFEC_INTR_CPU_STATUS) && (reg <= BCHP_AFEC_INTR_PCI_MASK_CLEAR))
      {
         /* AFEC_INTR register access */
         if (h->channel & 1)
            *pAddr += 0x400;
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x100000);
      }
      else if (((reg >= BCHP_AFECNX_GLOBAL_CLK_CNTRL) && (reg <= BCHP_AFECNX_COMB_SMTH_FIFO_MAX)) ||
               ((reg >= BCHP_AFEC_GLOBAL_INTR_CPU_STATUS) && (reg <= BCHP_AFEC_GR_BRIDGE_SW_RESET_1)))
      {
         /* AFECNX_GLOBAL, AFECNX, AFEC_GLOBAL_INTR, AFEC_BR_BRIDGE register access */
         *pAddr += ((uint32_t)(h->channel >> 1) * 0x100000);
      }
   }
}


/******************************************************************************
 BSAT_g1_P_ReadRegister_isrsafe() - returns register value
******************************************************************************/
uint32_t BSAT_g1_P_ReadRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg)
{
   uint32_t addr;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   BSAT_g1_P_GetRegisterAddress_isrsafe(h, reg, &addr);
/*BDBG_ERR(("read 0x%X", addr));*/
   return BREG_Read32(pDev->hRegister, addr);
}


/******************************************************************************
 BSAT_g1_P_WriteRegister_isrsafe()
******************************************************************************/
void BSAT_g1_P_WriteRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t val)
{
   uint32_t addr, wait_time;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);

   BSAT_g1_P_GetRegisterWriteWaitTime_isrsafe(h, reg, &wait_time);
   BSAT_g1_P_GetRegisterAddress_isrsafe(h, reg, &addr);
   BREG_Write32(pDev->hRegister, addr, val);
/*BDBG_ERR(("write 0x%X=0x%X", addr, val));*/
   if (wait_time > 0)
      BKNI_Delay(wait_time);
}


/******************************************************************************
 BSAT_g1_P_InitHandleExt() - chip-specific initial settings in BSAT device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitHandleExt(BSAT_Handle h)
{
   BSAT_g1_P_Handle *hDev;

   hDev = (BSAT_g1_P_Handle *)(h->pImpl);
   hDev->xtalFreq = BSAT_G1_XTAL_FREQ;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_InitChannelHandleExt() - chip-specific initial settings in BSAT
                                    channel device handle
******************************************************************************/
BERR_Code BSAT_g1_P_InitChannelHandleExt(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   hChn->bHasAfec = true;
   hChn->bHasTfec = true;
   hChn->xportSettings.bOpllBypass = true;
   hChn->xportSettings.bSerial = false;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerUp() - power up the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerUp(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t mask, shift;

   if (h->channel >= 8)
   {
      mask = (1<<5);
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2) | mask);
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3) | mask);
   }

   shift = (h->channel < 8) ? h->channel : (h->channel - 8);
   mask = (1 << shift);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, ~mask);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_LOCAL_SW_RESET, ~mask);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_TunerPowerDown() - power down the channelizer
******************************************************************************/
BERR_Code BSAT_g1_P_TunerPowerDown(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t mask, shift, val;

   if (h->channel < 8)
      mask = (1<<4);
   else
      mask = (1<<5);
   val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   if ((mask & val) == 0)
   {
      BDBG_MSG(("BSAT_g1_P_TunerPowerDown(%d): Channelizer already powered down", h->channel));
      return BERR_SUCCESS;
   }
   val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   if ((mask & val) == 0)
   {
      BDBG_MSG(("BSAT_g1_P_TunerPowerDown(%d): Channelizer already powered down", h->channel));
      return BERR_SUCCESS;
   }

   shift = (h->channel < 8) ? h->channel : (h->channel - 8);
   mask = (1 << shift);

   BKNI_EnterCriticalSection();
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN, mask | 0x100); /* power down test channel also */
   BKNI_LeaveCriticalSection();

   if (h->channel >= 8)
   {
      val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CTRL_PWRDN);
      if ((val & 0xFF) == 0xFF)
      {
         /* all channelizers are turned off, so we can turn off the CHAN 108 MHz clock enable */
         mask = ~(1<<5);

         BKNI_EnterCriticalSection();
         val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
         val &= mask;
         BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);
         val = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
         val &= mask;
         BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);
         BKNI_LeaveCriticalSection();
      }
   }

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_IsSdsOn()
******************************************************************************/
bool BSAT_g1_P_IsSdsOn(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl2, ctrl3, ctrl5, mask;

   ctrl2 = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   ctrl3 = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   ctrl5 = BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);

   /* check SDS PLL clock enable */
   mask = 1 << h->channel;
   if ((ctrl5 & mask) == 0)
      return false;

   /* check SDS 54MHz clock enable */
   mask = 0x10000 << h->channel;
   if ((ctrl3 & mask) == 0)
      return false;

   /* check SDS 108MHz clock enable */
   if ((ctrl2 & mask) == 0)
      return false;

   return true;
}


/******************************************************************************
 BSAT_g1_P_SdsPowerUp() - power up the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerUp(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;

   BKNI_EnterCriticalSection();

   /* enable SDS PLL clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);
   val |= (1 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5, val);

   /* enable SDS 54MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   val |= (0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

   /* enable SDS 108MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   val |= (0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

   BKNI_LeaveCriticalSection();

   return BSAT_g1_P_IsSdsOn(h) ? BERR_SUCCESS : BSAT_ERR_POWERUP_FAILED;
}


/******************************************************************************
 BSAT_g1_P_SdsPowerDown() - power down the sds core
******************************************************************************/
BERR_Code BSAT_g1_P_SdsPowerDown(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;

   BKNI_EnterCriticalSection();

   /* enable SDS 108MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   val &= ~(0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

   /* enable SDS 54MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   val &= ~(0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

   /* enable SDS PLL clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);
   val &= ~(1 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5, val);

   BKNI_LeaveCriticalSection();

   return BSAT_g1_P_IsSdsOn(h) ? BSAT_ERR_POWERUP_FAILED : BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAgcStatus()
 agc[0] = {bit 0: lna_agc valid}, {bit 1: chan_agc valid}
 agc[1] = lna_agc
 agc[2] = chan_agc
 agc[3] = (not used)

******************************************************************************/
BERR_Code BSAT_g1_P_GetAgcStatus(BSAT_ChannelHandle h, BSAT_AgcStatus *pStatus)
{
   pStatus->flags = 0x02; /* indicate channelizer AGC value is valid */
   pStatus->value[1] = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_STB_CHAN_CHx_AGC_LF_INT);
   return BERR_SUCCESS;
}


#ifndef BSAT_EXCLUDE_AFEC
/******************************************************************************
 BSAT_g1_P_IsAfecOn_isrsafe() - true if afec global is on
******************************************************************************/
bool BSAT_g1_P_IsAfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl2, ctrl3, ctrl4, mask;

   ctrl2 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   ctrl3 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   ctrl4 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL4);

   /* check AFEC PLL clock enable */
   mask = 1 << (h->channel >> 1);
   if ((ctrl4 & mask) == 0)
      return false;

   /* check AFEC 54MHz clock enable */
   mask = 0x100 << (h->channel >> 1);
   if ((ctrl3 & mask) == 0)
      return false;

   /* check AFEC 108MHz clock enable */
   if ((ctrl2 & mask) == 0)
      return false;

   /* check if fec_pll_clk is gated off in AFEC */
   if ((BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL)) & BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK)
      return false;

   return true;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerUp_isr() - power up the afec core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t val;

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      BSAT_DEBUG_POWERDOWN(BDBG_MSG(("AFEC%d Power Up", h->channel)));

      /* enable AFEC PLL clock */
      val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL4);
      val |= (1 << (h->channel >> 1));
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL4, val);

      /* enable AFEC 54MHz clock */
      val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
      val |= (0x100 << (h->channel >> 1));
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

      /* enable AFEC 108MHz clock */
      val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
      val |= (0x100 << (h->channel >> 1));
      BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL, ~BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 1); /* full datapath reset */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 0);
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CONFIG, BCHP_AFECNX_0_GLOBAL_CONFIG_LDPC_ENA_MASK); /* enable LDPC decoder */

      /* per Howard, do this to reduce power supply signal drooping */
      val = h->channel >> 1;
      if (val & 1)
         val = 7;
      else
         val = 0;
      BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_AFECNX_QBUF_CTRL, 0xFFFFFF0F, val << 4);
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CONFIG, BCHP_AFECNX_0_GLOBAL_CONFIG_ENABLE_CLK_GATING_MASK);
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_AFECNX_LDPC_MEM_POWER, ~BCHP_AFECNX_0_LDPC_MEM_POWER_ENABLE_POWER_SHUT_DOWN_MASK);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_AfecPowerDown_isrsafe() - power down the afec global core
******************************************************************************/
BERR_Code BSAT_g1_P_AfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   BSAT_ChannelHandle hOtherChan = BSAT_g1_P_AfecGetOtherChannelHandle_isrsafe(h);
   BSAT_g1_P_ChannelHandle *hOtherChanImpl;
   uint32_t val;

   BSAT_DEBUG_POWERDOWN(BDBG_MSG(("AFEC%d Power Down", h->channel)));

   hChn->bAfecFlushFailed = false;

   if (hOtherChan)
   {
      hOtherChanImpl = (BSAT_g1_P_ChannelHandle *)(hOtherChan->pImpl);
      hOtherChanImpl->bAfecFlushFailed = false;
   }

   if (BSAT_g1_P_IsAfecOn_isrsafe(h) == false)
   {
      BDBG_MSG(("BSAT_g1_P_AfecPowerDown_isrsafe(%d): AFEC already powered down", h->channel));
      goto done;
   }

   /* first do a datapath reset */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CFG_RST, 0);

   /* global power down */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_AFECNX_GLOBAL_CLK_CNTRL, BCHP_AFECNX_GLOBAL_0_CLK_CNTRL_LDPC_CLK_ENABLEB_MASK);

   /* disable AFEC 54MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3);
   val &= ~(0x100 << (h->channel >> 1));
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL3, val);

   /* disable AFEC 108MHz clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2);
   val &= ~(0x100 << (h->channel >> 1));
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL2, val);

   /* disable AFEC PLL clock */
   val =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL4);
   val &= ~(1 << (h->channel >> 1));
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL4, val);

   done:
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_GetAfecClock_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetAfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
#if 1
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   uint32_t mdiv = (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4) & BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4_MDIV_CH4_MASK) >> BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_4_MDIV_CH4_SHIFT;
   uint32_t div = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TM_PLL4_SYS_PLL_DIV);
   uint32_t pdiv = (div & BCHP_TM_PLL4_SYS_PLL_DIV_PDIV_MASK) >> BCHP_TM_PLL4_SYS_PLL_DIV_PDIV_SHIFT;
   uint32_t ndiv = (div & BCHP_TM_PLL4_SYS_PLL_DIV_NDIV_INT_MASK) >> BCHP_TM_PLL4_SYS_PLL_DIV_NDIV_INT_SHIFT;

   *pFreq = (hDevImpl->xtalFreq / (pdiv * mdiv)) * ndiv;
#else
   *pFreq = 432000000;
#endif
   return BERR_SUCCESS;
}
#endif /* BSAT_EXCLUDE_AFEC */


#ifndef BSAT_EXCLUDE_TFEC
/******************************************************************************
 BSAT_g1_P_TfecPowerUp_isr() - power up the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerUp_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl5;

   ctrl5 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);
   ctrl5 |= (0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5, ctrl5);

   return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BERR_SUCCESS: BSAT_ERR_POWERDOWN_FAILED;
}


/******************************************************************************
 BSAT_g1_P_TfecPowerDown_isrsafe() - power down the tfec core
******************************************************************************/
BERR_Code BSAT_g1_P_TfecPowerDown_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl5;

   ctrl5 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);
   ctrl5 &= ~(0x10000 << h->channel);
   BREG_Write32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5, ctrl5);

   return BSAT_g1_P_IsTfecOn_isrsafe(h) ? BSAT_ERR_POWERDOWN_FAILED : BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_IsTfecOn_isrsafe()
******************************************************************************/
bool BSAT_g1_P_IsTfecOn_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_Handle *pDev = (BSAT_g1_P_Handle*)(h->pDevice->pImpl);
   uint32_t ctrl5, mask;

   ctrl5 =  BREG_Read32(pDev->hRegister, BCHP_TM_CLOCK_ENABLE_CTRL5);
   mask = 0x10000 << h->channel;
   return (ctrl5 & mask) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_GetTfecClock_isrsafe()
******************************************************************************/
BERR_Code BSAT_g1_P_GetTfecClock_isrsafe(BSAT_ChannelHandle h, uint32_t *pFreq)
{
#if 1
   BSAT_g1_P_Handle *hDevImpl = (BSAT_g1_P_Handle *)(h->pDevice->pImpl);
   uint32_t mdiv = (BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_5) & BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_5_MDIV_CH5_MASK) >> BCHP_TM_PLL4_SYS_PLL_CHANNEL_CTRL_CH_5_MDIV_CH5_SHIFT;
   uint32_t div = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_TM_PLL4_SYS_PLL_DIV);
   uint32_t pdiv = (div & BCHP_TM_PLL4_SYS_PLL_DIV_PDIV_MASK) >> BCHP_TM_PLL4_SYS_PLL_DIV_PDIV_SHIFT;
   uint32_t ndiv = (div & BCHP_TM_PLL4_SYS_PLL_DIV_NDIV_INT_MASK) >> BCHP_TM_PLL4_SYS_PLL_DIV_NDIV_INT_SHIFT;

   *pFreq = (hDevImpl->xtalFreq / (pdiv * mdiv)) * ndiv;
#else
   *pFreq = 240000000;
#endif
   return BERR_SUCCESS;
}

#endif /* BSAT_EXCLUDE_TFEC */
