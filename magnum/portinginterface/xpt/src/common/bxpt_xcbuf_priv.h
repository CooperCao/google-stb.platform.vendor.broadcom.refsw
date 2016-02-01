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
 * Porting interface code for the transport client buffer block. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXPT_XCBUFF_PRIV_H__
#define BXPT_XCBUFF_PRIV_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif


/*
** These functions are called internally from BXPT_Open() and BXPT_Close(). 
** Users should NOT uses these functions directly.
*/

BERR_Code BXPT_P_XcBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_BandWidthConfig *BandwidthConfig
    );

BERR_Code BXPT_P_XcBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    );

void BXPT_XcBuf_P_EnablePlaybackPausing( 
    BXPT_Handle hXpt, 
    unsigned PbChannelNum,
    bool PauseEn
    );

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_P_XcBuf_ReportOverflows( 
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    );
#endif

#ifdef __cplusplus
}
#endif

#endif /* BXPT_XCBUFF_PRIV_H__ */


