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
Software has been moved to the BUDP commonutils module. */

</verbatim>
***************************************************************************/

#ifndef BVBILIBSCTEPARSE_H__
#define BVBILIBSCTEPARSE_H__

#include "budp_scteparse.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Data structures */
typedef enum {
	BVBIlib_SCTEparse_Format_Unknown   = BUDP_SCTEparse_Format_Unknown,
	BVBIlib_SCTEparse_Format_SCTE20    = BUDP_SCTEparse_Format_SCTE20,
	BVBIlib_SCTEparse_Format_SCTE21CC  = BUDP_SCTEparse_Format_SCTE21CC,
	BVBIlib_SCTEparse_Format_SCTE21ACC = BUDP_SCTEparse_Format_SCTE21ACC,
	BVBIlib_SCTEparse_Format_SCTE21PAM = BUDP_SCTEparse_Format_SCTE21PAM
}
BUDP_SCTEparse_Format;

#define BVBIlib_SCTEparse_sctedata BUDP_SCTEparse_sctedata

/* Functions */
#define BVBIlib_SCTE20parse BUDP_SCTE20parse
#define BVBIlib_SCTE20parse_isr BUDP_SCTE20parse_isr
#define BVBIlib_SCTE21parse BUDP_SCTE21parse
#define BVBIlib_SCTE21parse_isr BUDP_SCTE21parse_isr

#ifdef __cplusplus
}
#endif

#endif /* BVBILIBSCTEPARSE_H__ */
