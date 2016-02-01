/***************************************************************************
*     Copyright (c) 2006-2010, Broadcom Corporation
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
*	This file contains structures, enums, macros and function prototypes, 
*	which are exposed to the upper layer for Grahics on ZSP by Rap PI. In other 
*	words this file defines the interfaces between the upper layer and the 
*	Raptor Gfx PI layer.
*
* Revision History:
*
* $brcm_Log: $
* 
*****************************************************************************/
#ifndef _BRAP_GFX_PRIV_H_
#define _BRAP_GFX_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#include "brap_priv.h"
#include "brap_gfx.h"
#include "brap.h"
#endif
#include "brap_types.h"
#include "brap_types_priv.h"
#include "brap_gfx.h"
#include "brap_cmdresp_priv.h"


BERR_Code
BRAP_GFX_P_GetCurrentChannelSettings (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Decode channel handle */
	BRAP_GFX_OpenSettings    *pChanSettings  /* [out] Current channel settings */
	);

BERR_Code BRAP_GFX_P_StartChannel ( 
    BRAP_GFX_Handle          hGfxCh,         /* [in] Gfx Channel handle */
    const BRAP_GFX_OpenSettings *pGfxParams   /* [in] Gfx parameters for 
                                                   starting this channel */
    );

BERR_Code BRAP_GFX_P_StopChannel ( 
    BRAP_GFX_Handle          hGfxCh          /* [in] Gfx channel handle */
    );

BERR_Code
BRAP_GFX_P_MapFillSettings (
    BRAP_Handle    hRap,
    BRAP_ChannelHandle  hRapCh,
    const BRAP_GFX_FillSettings                   *psFillSetting, /*[in] Fill Settings*/
        BRAP_GFX_P_Fill    *psFillSettingsInFwFormat  /* [out] Current channel settings */
    );

BERR_Code
BRAP_GFX_P_MapBlitSettings (
    BRAP_Handle    hRap,
    BRAP_ChannelHandle  hRapCh,
    const BRAP_GFX_BlitSettings                   *psBlitSetting, /*[in] Fill Settings*/
        BRAP_GFX_P_Blit    *psBlitSettingsInFwFormat  /* [out] Current channel settings */
    );

BERR_Code   BRAP_GFX_P_GetAndFreeBuffer(
    BRAP_ChannelHandle  hRapCh,
    unsigned int    uiLastCompletedOperationId,/*[In]*/
    unsigned int    *uiBufferId /*Out*/);

BERR_Code   BRAP_P_GetGfxStatusInfo(
unsigned int	        uiStatusBufAddr,    
BRAP_FWIF_P_GfxStageStatus  *psGfxStatusBuf);



#ifdef __cplusplus
    }
#endif
    
    
#endif /* !_BRAP_GFX_PRIV_H_ */

