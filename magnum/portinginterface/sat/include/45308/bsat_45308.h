/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#ifndef _BSAT_45308_H_
#define _BSAT_45308_H_

#ifdef __cplusplus
extern "C" {
#endif


/* device configuration parameters */
enum {
   BSAT_45308_CONFIG_PARAM_DEBUG_LEVEL = 0,
   BSAT_45308_CONFIG_PARAM_MAX_HAB_TIME,
   BSAT_45308_CONFIG_PARAM_MAX_HAB_TIME_CMD,
   BSAT_45308_CONFIG_PARAM_OTP_DISABLE_INPUT,
   BSAT_45308_CONFIG_PARAM_OTP_DISABLE_CHAN,
   BSAT_45308_CONFIG_PARAM_OTP_DISABLE_FEATURE,
   BSAT_45308_CONFIG_PARAM_FLASH_BUF_ADDR,
   BSAT_45308_CONFIG_PARAM_OTP_DISABLE_DSEC,
   BSAT_45308_CONFIG_MAX
};


/* channel configuration parameters */
enum {
   BSAT_45308_CHAN_CONFIG_PLC_CTL = 0,
   BSAT_45308_CHAN_CONFIG_PLC_ALT_ACQ_BW,
   BSAT_45308_CHAN_CONFIG_PLC_ALT_ACQ_DAMP,
   BSAT_45308_CHAN_CONFIG_PLC_ALT_TRK_BW,
   BSAT_45308_CHAN_CONFIG_PLC_ALT_TRK_DAMP,
   BSAT_45308_CHAN_CONFIG_DFT_STATUS,
   BSAT_45308_CHAN_CONFIG_DFT_FREQ_ESTIMATE,
   BSAT_45308_CHAN_CONFIG_ACQ_TIME,
   BSAT_45308_CHAN_CONFIG_HP_CTL,
   BSAT_45308_CHAN_CONFIG_VLC_GAIN,
   BSAT_45308_CHAN_CONFIG_EQ_CTL,
   BSAT_45308_CHAN_CONFIG_ACQ_DAFE_CTL,
   BSAT_45308_CHAN_CONFIG_TRK_DAFE_CTL,
   BSAT_45308_CHAN_CONFIG_MAX
};


/* bit definitions for BSAT_45308_CONFIG_PLC_CTL */
#define BSAT_45308_CONFIG_PLC_CTL_AWGN         0x00000001
#define BSAT_45308_CONFIG_PLC_CTL_OVERRIDE_ACQ 0x00000002
#define BSAT_45308_CONFIG_PLC_CTL_OVERRIDE_TRK 0x00000004


/* bit definitions for BSAT_45308_CONFIG_DFT_FREQ_EST_STATUS */
#define BSAT_45308_CONFIG_DFT_STATUS_START     0x01000000 /* DFT freq estimate started */
#define BSAT_45308_CONFIG_DFT_STATUS_COARSE    0x02000000 /* DFT freq estimate coarse search started */
#define BSAT_45308_CONFIG_DFT_STATUS_FINE      0x04000000 /* DFT freq estimate fine search started */
#define BSAT_45308_CONFIG_DFT_STATUS_OQPSK     0x08000000 /* DFT freq estimate for OQPSK */
#define BSAT_45308_CONFIG_DFT_STATUS_PEAK_SCAN 0x10000000 /* peak scan operation */
#define BSAT_45308_CONFIG_DFT_STATUS_DONE      0x80000000 /* DFT freq estimate finished */
#define BSAT_45308_CONFIG_DFT_STATUS_STATE     0x000000FF


/* chip-specific functions */
BERR_Code BSAT_45308_GetDefaultSettings(BSAT_Settings *pDefSettings);
BERR_Code BSAT_45308_GetChannelDefaultSettings(BSAT_Handle h, uint32_t chnNo, BSAT_ChannelSettings *pChnDefSettings);

#ifdef __cplusplus
}
#endif

#endif /* _BSAT_45308_H_ */
