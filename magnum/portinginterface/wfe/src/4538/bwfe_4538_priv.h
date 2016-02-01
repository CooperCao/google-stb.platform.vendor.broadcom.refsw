/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _BWFE_4538_PRIV_H__
#define _BWFE_4538_PRIV_H__

#include "bwfe_4538.h"
#include "bhab.h"

#define BWFE_4538_RELEASE_VERSION 1

#define BWFE_MAX_BBSI_RETRIES 10

#define BWFE_4538_MAX_CHANNELS 4

#define BWFE_4538_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }



/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST handle
Description:
   This is the chip-specific component of the BAST_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BWFE_4538_P_Handle
{
   BHAB_Handle       hHab;
   BKNI_EventHandle  hSaDoneEvent;  /* spectrum scan done event handle */
   uint16_t          numSamples;    /* number of sa samples requested */
} BWFE_4538_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST channel handle
Description:
   This is the chip-specific component of the BAST_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BWFE_4538_P_ChannelHandle
{
   BKNI_EventHandle  hWfeReady;  /* wfe ready event */
} BWFE_4538_P_ChannelHandle;


/* 4538 implementation of API functions */
BERR_Code BWFE_4538_P_Open(BWFE_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BWFE_Settings *pDefSettings);
BERR_Code BWFE_4538_P_Close(BWFE_Handle h);
BERR_Code BWFE_4538_P_GetTotalChannels(BWFE_Handle h, BWFE_ChannelInfo *pInfo);
BERR_Code BWFE_4538_P_OpenChannel(BWFE_Handle h, BWFE_ChannelHandle *pChannelHandle, uint8_t chanNum, const BWFE_ChannelSettings *pSettings);
BERR_Code BWFE_4538_P_CloseChannel(BWFE_ChannelHandle h);
BERR_Code BWFE_4538_P_Reset(BWFE_Handle h);
BERR_Code BWFE_4538_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BWFE_4538_P_EnableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_4538_P_DisableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_4538_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BWFE_4538_P_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hEvent);
BERR_Code BWFE_4538_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams);
BERR_Code BWFE_4538_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples);
BERR_Code BWFE_4538_P_IsInputEnabled(BWFE_ChannelHandle h, bool *pbEnabled);

#endif /* _BWFE_4538_PRIV_H__ */

