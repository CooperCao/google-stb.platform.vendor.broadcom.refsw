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
#ifndef _BDSQ_45308_PRIV_H_
#define _BDSQ_45308_PRIV_H_

#include "bdsq_45308.h"
#include "bhab.h"

#define BDSQ_45308_RELEASE_VERSION 0

#define BDSQ_45308_MAX_CHANNELS 2


#define BDSQ_45308_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }



/***************************************************************************
Summary:
   Structure for chip-specific portion of the BDSQ handle
Description:
   This is the chip-specific component of the BDSQ handle.
See Also:
   None.
****************************************************************************/
typedef struct BDSQ_45308_P_Handle
{
   BHAB_Handle       hHab;
} BDSQ_45308_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BDSQ channel handle
Description:
   This is the chip-specific component of the BDSQ channel handle.
See Also:
   None.
****************************************************************************/
typedef struct BDSQ_45308_P_ChannelHandle
{
   BKNI_EventHandle hTxEvent;       /* diseqc tx event handle */
   BKNI_EventHandle hRxEvent;       /* diseqc rx event handle */
   BKNI_EventHandle hVsenseEvent;   /* vsense event handle */
} BDSQ_45308_P_ChannelHandle;


BERR_Code BDSQ_45308_P_Open(BDSQ_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BDSQ_Settings *pSettings);
BERR_Code BDSQ_45308_P_Close(BDSQ_Handle h);
BERR_Code BDSQ_45308_P_GetTotalChannels(BDSQ_Handle h, uint32_t *totalChannels);
BERR_Code BDSQ_45308_P_OpenChannel(BDSQ_Handle h, BDSQ_ChannelHandle *pChannelHandle, uint32_t chan, const BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_45308_P_CloseChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_GetDevice(BDSQ_ChannelHandle h, BDSQ_Handle *pDev);
BERR_Code BDSQ_45308_P_GetVersionInfo(BDSQ_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BDSQ_45308_P_Reset(BDSQ_Handle h);
BERR_Code BDSQ_45308_P_ResetChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_PowerDownChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_PowerUpChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_IsChannelOn(BDSQ_ChannelHandle h, bool *pbOn);
BERR_Code BDSQ_45308_P_PowerDownVsense(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_PowerUpVsense(BDSQ_ChannelHandle h);
BERR_Code BDSQ_45308_P_IsVsenseOn(BDSQ_ChannelHandle h, bool *pbOn);
BERR_Code BDSQ_45308_P_SetTone(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_45308_P_GetTone(BDSQ_ChannelHandle h, bool *pbTone);
BERR_Code BDSQ_45308_P_SetVoltage(BDSQ_ChannelHandle h, bool bVtop);
BERR_Code BDSQ_45308_P_GetVoltage(BDSQ_ChannelHandle h, uint16_t *pVoltage);
BERR_Code BDSQ_45308_P_EnableVsenseInterrupt(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_45308_P_Write(BDSQ_ChannelHandle h, const uint8_t *pBuf, uint8_t n);
BERR_Code BDSQ_45308_P_EnableRx(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_45308_P_Read(BDSQ_ChannelHandle h, uint8_t *pBuf, uint8_t nBufMax, uint8_t *n, uint8_t *nNotRead);
BERR_Code BDSQ_45308_P_SendAcw(BDSQ_ChannelHandle h, uint8_t acw);
BERR_Code BDSQ_45308_P_GetStatus(BDSQ_ChannelHandle h, BDSQ_Status *pStatus);
BERR_Code BDSQ_45308_P_SetChannelSettings(BDSQ_ChannelHandle h, BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_45308_P_GetChannelSettings(BDSQ_ChannelHandle h, BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_45308_P_SetConfig(BDSQ_Handle h, uint32_t addr, uint32_t val);
BERR_Code BDSQ_45308_P_GetConfig(BDSQ_Handle h, uint32_t addr, uint32_t *pVal);
BERR_Code BDSQ_45308_P_SetChannelConfig(BDSQ_ChannelHandle h, uint32_t addr, uint32_t val);
BERR_Code BDSQ_45308_P_GetChannelConfig(BDSQ_ChannelHandle h, uint32_t addr, uint32_t *pVal);
BERR_Code BDSQ_45308_P_GetTxEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BDSQ_45308_P_GetRxEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BDSQ_45308_P_GetVsenseEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hVsenseEvent);

#endif /* _BDSQ_45308_PRIV_H_ */
