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
 * Porting interface code for the rate smoothing buffer block in the 
 * transport core. 
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BXPT_RS_BUFFER_PRIV_H__
#define BXPT_RS_BUFFER_PRIV_H__

#include "bxpt.h"

#ifdef __cplusplus
extern "C"{
#endif

/* used in bxpt.c and bxpt_rsbuf_priv.c */
#define RS_BUFFER_PTR_REG_STEPSIZE     (BCHP_XPT_RSBUFF_BASE_POINTER_IBP1 - BCHP_XPT_RSBUFF_BASE_POINTER_IBP0)

/*
** These functions are called internally from BXPT_Open() and BXPT_Close(). 
** Users should NOT uses these functions directly.
*/

BERR_Code BXPT_P_RsBuf_Init(
	BXPT_Handle hXpt, 	   	   	/* [in] Handle for this transport */
    const BXPT_BandWidthConfig *BandwidthConfig
	);

BERR_Code BXPT_P_RsBuf_Shutdown(
	BXPT_Handle hXpt 	   	   	/* [in] Handle for this transport */
	);

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_P_RsBuf_ReportOverflows( 
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* BXPT_RS_BUFFER_H__ */


