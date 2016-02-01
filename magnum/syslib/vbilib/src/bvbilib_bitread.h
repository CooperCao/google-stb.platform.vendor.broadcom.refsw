/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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

/*= Module Overview *********************************************************
<verbatim>

Overview
Software has been moved to the BUDP commonutils module.

</verbatim>
***************************************************************************/

#ifndef BVBILIBBITREAD_H__
#define BVBILIBBITREAD_H__

#include "budp_bitread.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data structures */
#define BVBIlib_Bitread_Context BUDP_Bitread_Context

/* Functions */
#define BVBIlib_Bitread_Init_isr BUDP_Bitread_Init_isr
#define BVBIlib_Bitread_Read_isr BUDP_Bitread_Read_isr
#define BVBIlib_Bitread_Byte_isr BUDP_Bitread_Byte_isr
#define BVBIlib_Bitread_GetByteOffset_isr BUDP_Bitread_GetByteOffset_isr
#define BVBIlib_Bitread_next_start_code_isr BUDP_Bitread_next_start_code_isr

#ifdef __cplusplus
}
#endif

#endif /* BVBILIBBITREAD_H__ */
