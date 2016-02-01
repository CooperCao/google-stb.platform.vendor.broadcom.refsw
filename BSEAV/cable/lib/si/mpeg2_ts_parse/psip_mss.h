/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
#ifndef PSIP_MSS_H__
#define PSIP_MSS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "psip_common.h"

/* TODO: add support for Unicode beyond ASCII */

uint8_t PSIP_MSS_getNumStrings( PSIP_MSS_string mss );
BERR_Code PSIP_MSS_getString( PSIP_MSS_string mss, int stringNum, int *p_stringSize, char *p_string );
BERR_Code PSIP_MSS_getCode( PSIP_MSS_string mss, int stringNum, char **ppLanguageCode );

#ifdef __cplusplus
}
#endif
#endif
/* End of File */
