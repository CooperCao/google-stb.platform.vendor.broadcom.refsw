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
#ifndef _BWFE_45308_PRIV_H__
#define _BWFE_45308_PRIV_H__

#include "bwfe_45308.h"
#include "bhab.h"

#define BWFE_45308_RELEASE_VERSION 0

#define BWFE_MAX_BBSI_RETRIES 10

#define BWFE_45308_MAX_CHANNELS 2

/* default only adc0 and adc1 */
#define BWFE_45308_NUM_CHANNELS 2
#define BWFE_45308_CHANNEL_MASK 0x3

#define BWFE_45308_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }



/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST handle
Description:
   This is the chip-specific component of the BAST_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BWFE_45308_P_Handle
{
   BHAB_Handle       hHab;
   BKNI_EventHandle  hSaDoneEvent;  /* spectrum scan done event handle */
   uint16_t          numSamples;    /* number of sa samples requested */
} BWFE_45308_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST channel handle
Description:
   This is the chip-specific component of the BAST_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BWFE_45308_P_ChannelHandle
{
   BKNI_EventHandle  hWfeReady;  /* wfe ready event */
} BWFE_45308_P_ChannelHandle;


/* 45308 implementation of API functions */
BERR_Code BWFE_45308_P_Open(BWFE_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BWFE_Settings *pDefSettings);
BERR_Code BWFE_45308_P_Close(BWFE_Handle h);
BERR_Code BWFE_45308_P_GetTotalChannels(BWFE_Handle h, BWFE_ChannelInfo *pInfo);
BERR_Code BWFE_45308_P_OpenChannel(BWFE_Handle h, BWFE_ChannelHandle *pChannelHandle, uint8_t chanNum, const BWFE_ChannelSettings *pSettings);
BERR_Code BWFE_45308_P_CloseChannel(BWFE_ChannelHandle h);
BERR_Code BWFE_45308_P_Reset(BWFE_Handle h);
BERR_Code BWFE_45308_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BWFE_45308_P_EnableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_45308_P_DisableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_45308_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BWFE_45308_P_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hEvent);
BERR_Code BWFE_45308_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams);
BERR_Code BWFE_45308_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples);
BERR_Code BWFE_45308_P_IsInputEnabled(BWFE_ChannelHandle h, bool *pbEnabled);

#endif /* _BWFE_45308_PRIV_H__ */
