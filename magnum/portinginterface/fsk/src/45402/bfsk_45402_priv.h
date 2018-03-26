/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef _BFSK_45402_PRIV_H_
#define _BFSK_45402_PRIV_H_

#include "bfsk_45402.h"
#include "bhab.h"

#define BFSK_45402_RELEASE_VERSION 1

#define BFSK_45402_MAX_CHANNELS 1

#define BFSK_45402_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }



/***************************************************************************
Summary:
   Structure for chip-specific portion of the BFSK handle
Description:
   This is the chip-specific component of the BFSK handle.
See Also:
   None.
****************************************************************************/
typedef struct BFSK_45402_P_Handle
{
   BHAB_Handle       hHab;
} BFSK_45402_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BFSK channel handle
Description:
   This is the chip-specific component of the BFSK channel handle.
See Also:
   None.
****************************************************************************/
typedef struct BFSK_45402_P_ChannelHandle
{
   BKNI_EventHandle hRxEvent;  /* fsk rx event handle */
   BKNI_EventHandle hTxEvent;  /* fsk tx event handle */
} BFSK_45402_P_ChannelHandle;


BERR_Code BFSK_45402_P_Open(BFSK_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BFSK_Settings *pSettings);
BERR_Code BFSK_45402_P_Close(BFSK_Handle h);
BERR_Code BFSK_45402_P_GetTotalChannels(BFSK_Handle h, uint32_t *totalChannels);
BERR_Code BFSK_45402_P_OpenChannel(BFSK_Handle h, BFSK_ChannelHandle *pChannelHandle, uint32_t chan, const BFSK_ChannelSettings *pSettings);
BERR_Code BFSK_45402_P_CloseChannel(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_GetDevice(BFSK_ChannelHandle h, BFSK_Handle *pDev);
BERR_Code BFSK_45402_P_GetVersionInfo(BFSK_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BFSK_45402_P_Reset(BFSK_Handle h);
BERR_Code BFSK_45402_P_ResetChannel(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_PowerDownChannel(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_PowerUpChannel(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_IsChannelOn(BFSK_ChannelHandle h, bool *pbOn);
BERR_Code BFSK_45402_P_EnableCarrier(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_DisableCarrier(BFSK_ChannelHandle h);
BERR_Code BFSK_45402_P_Write(BFSK_ChannelHandle h, uint8_t *pBuf, uint8_t n, void *pParams);
BERR_Code BFSK_45402_P_SetRxCount(BFSK_ChannelHandle h, uint8_t n);
BERR_Code BFSK_45402_P_Read(BFSK_ChannelHandle h, uint8_t *pBuf, uint8_t nBufMax, uint8_t *n, uint8_t *nNotRead);
BERR_Code BFSK_45402_P_GetRssiLevel(BFSK_ChannelHandle h, uint32_t *pLevel);
BERR_Code BFSK_45402_P_GetRxEventHandle(BFSK_ChannelHandle h, BKNI_EventHandle *hRxEvent);
BERR_Code BFSK_45402_P_GetTxEventHandle(BFSK_ChannelHandle h, BKNI_EventHandle *hTxEvent);
BERR_Code BFSK_45402_P_SetConfig(BFSK_Handle h, uint32_t addr, uint32_t val);
BERR_Code BFSK_45402_P_GetConfig(BFSK_Handle h, uint32_t addr, uint32_t *pVal);
BERR_Code BFSK_45402_P_SetChannelConfig(BFSK_ChannelHandle h, uint32_t addr, uint32_t val);
BERR_Code BFSK_45402_P_GetChannelConfig(BFSK_ChannelHandle h, uint32_t addr, uint32_t *pVal);

#endif /* _BFSK_45402_PRIV_H_ */
