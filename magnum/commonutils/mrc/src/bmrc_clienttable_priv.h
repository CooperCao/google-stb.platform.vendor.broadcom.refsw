/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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

#ifndef BMRC_CLIENTTABLE_PRIV_H
#define BMRC_CLIENTTABLE_PRIV_H

#include "bchp.h"                /* Chip information */

#ifdef __cplusplus
extern "C" {
#endif

/* client table function prototypes */
int BMRC_P_GET_CLIENT_ID(uint16_t usMemcId, BMRC_Client   eClient);
BMRC_Client BMRC_P_GET_CLIENT_ENUM_isrsafe(uint16_t usMemcId, uint16_t usClientId);
const char *BMRC_P_GET_CLIENT_NAME(BMRC_Client   eClient);
BERR_Code BMRC_Checker_P_GetClientInfo_isrsafe(unsigned memcId, BMRC_Client eClient, BMRC_ClientInfo *pClientInfo);

#ifdef __cplusplus
}
#endif

#endif
/* End of File */
