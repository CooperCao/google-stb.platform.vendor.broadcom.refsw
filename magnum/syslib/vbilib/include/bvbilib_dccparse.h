/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
This software has been moved to the BUDP commonutils module.

</verbatim>
***************************************************************************/

#ifndef BVBILIBDCCPARSE_H__
#define BVBILIBDCCPARSE_H__

#include "budp_dccparse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	BVBIlib_DCCparse_Format_Unknown = BUDP_DCCparse_Format_Unknown,
	BVBIlib_DCCparse_Format_DVS157  = BUDP_DCCparse_Format_DVS157,
	BVBIlib_DCCparse_Format_ATSC53  = BUDP_DCCparse_Format_ATSC53,
	BVBIlib_DCCparse_Format_DVS053  = BUDP_DCCparse_Format_DVS053,
	BVBIlib_DCCparse_Format_SEI     = BUDP_DCCparse_Format_SEI,
	BVBIlib_DCCparse_Format_SEI2    = BUDP_DCCparse_Format_SEI2,
	BVBIlib_DCCparse_Format_AFD53   = BUDP_DCCparse_Format_AFD53,
	BVBIlib_DCCparse_Format_LAST    = BUDP_DCCparse_Format_LAST
}
BVBIlib_DCCparse_Format;

/* Data structure */
#define BVBIlib_DCCparse_ccdata BUDP_DCCparse_ccdata

/* Functions */
#define BVBIlib_DCCparse BUDP_DCCparse
#define BVBIlib_DCCparse_isr BUDP_DCCparse_isr
#define BVBIlib_DCCparse_SEI BUDP_DCCparse_SEI
#define BVBIlib_DCCparse_SEI_isr BUDP_DCCparse_SEI_isr

#ifdef __cplusplus
}
#endif

#endif /* BVBILIBDCCPARSE_H__ */
