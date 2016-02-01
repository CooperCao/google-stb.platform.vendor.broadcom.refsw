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

#ifndef BVCE_PLATFORM_7445_H_
#define BVCE_PLATFORM_7445_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (BCHP_VER < BCHP_VER_B0)
/* 7445 Ax */
#error Unsupported chip
#elif (BCHP_VER < BCHP_VER_C0)
/* 7445 Bx */
#error Unsupported chip
#elif (BCHP_VER < BCHP_VER_D0)
/* 7445 Cx */
#error Unsupported chip
#else
/* 7445 D0 and greater */
#define BVCE_P_CORE_MAJOR     2
#define BVCE_P_CORE_MINOR     1
#define BVCE_P_CORE_SUBMINOR  0
#define BVCE_P_CORE_REVISION  3

#define BVCE_PLATFORM_P_NUM_ENCODE_INSTANCES 2
#define BVCE_PLATFORM_P_BOX_MODE_SUPPORT 1
#endif

#include "bvce_core.h"

#ifdef __cplusplus
}
#endif

#endif /* BVCE_PLATFORM_7445_H_ */
/* End of File */
