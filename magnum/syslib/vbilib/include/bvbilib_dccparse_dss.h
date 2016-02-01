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
This software has been moved to the BUDP commonutils module.

</verbatim>
***************************************************************************/

#ifndef BVBILIBDCCPARSEDSS_H__
#define BVBILIBDCCPARSEDSS_H__

#include "budp_dccparse_dss.h"

typedef enum {
	BVBIlib_DCCparse_CC_Dss_Type_Undefined =
		BUDP_DCCparse_CC_Dss_Type_Undefined,
	BVBIlib_DCCparse_CC_Dss_Type_ClosedCaption =
		BUDP_DCCparse_CC_Dss_Type_ClosedCaption,
	BVBIlib_DCCparse_CC_Dss_Type_Subtitle =
		BUDP_DCCparse_CC_Dss_Type_Subtitle
} BVBIlib_DCCparse_Dss_CC_Type;

#define BVBIlib_DCCparse_dss_cc_subtitle BUDP_DCCparse_dss_cc_subtitle

#define BVBIlib_DCCparse_DSS BUDP_DCCparse_DSS
#define BVBIlib_DCCparse_DSS_isr BUDP_DCCparse_DSS_isr

#endif /* BVBILIBDCCPARSEDSS_H__ */

