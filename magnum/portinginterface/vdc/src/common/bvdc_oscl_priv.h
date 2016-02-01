/***************************************************************************
 *     Copyright (c) 2009, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BVDC_OSCL_PRIV_H__
#define BVDC_OSCL_PRIV_H__

#include "bstd.h"
#include "bavc.h"
#include "bvdc.h"
#include "bdbg.h"
#include "bvdc_compositor_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

void BVDC_P_OSCL_BuildRul_isr
	( BVDC_Compositor_Handle           hCompositor,
	  BVDC_P_ListInfo                 *pList,
	  BAVC_Polarity                    eFieldId );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_OSCL_PRIV_H__ */
/* End of File */

