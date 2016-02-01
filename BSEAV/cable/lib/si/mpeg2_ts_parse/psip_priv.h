/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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
#ifndef PSIP_PRIV_H__
#define PSIP_PRIV_H__

#include "ts_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSIP_PROTOCOL_VERSION_OFFSET	8
#define PSIP_TABLE_DATA_OFFSET			9

#define CHECK(COND) \
	do {if (!(COND)) BDBG_ERR(("Bad CHECK: %s at %s, %d", #COND, __FILE__, __LINE__)); } while (0)

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
