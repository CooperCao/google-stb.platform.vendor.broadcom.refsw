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

#ifndef BVCE_PLATFORM_7435_H_
#define BVCE_PLATFORM_7435_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (BCHP_VER < BCHP_VER_B0)
/* 7435 Ax */
#error Unsupported chip
#else
/* 7435 B0 or greater */
#define BVCE_P_CORE_MAJOR     1
#define BVCE_P_CORE_MINOR     1
#define BVCE_P_CORE_SUBMINOR  1
#define BVCE_P_CORE_REVISION  1

#define BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES 2
#define BVCE_PLATFORM_P_BOX_MODE_SUPPORT 1
#endif

#include "bvce_core.h"

#ifdef __cplusplus
}
#endif

#endif /* BVCE_PLATFORM_7435_H_ */
/* End of File */
