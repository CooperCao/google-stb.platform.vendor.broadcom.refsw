/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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

#ifndef BHDCPLIB_KEYLOADER__
#define BHDCPLIB_KEYLOADER__

#include "bstd.h"
#include "bavc_hdmi.h"
#include "bhdcplib.h"
#include "bhdcplib_priv.h"


BERR_Code BHDCPlib_FastLoadEncryptedHdcpKeys(BHDCPlib_Handle hHDCPlib);


BERR_Code BHDCPlib_GetKeySet(BAVC_HDMI_HDCP_KSV pTxAksv, BHDCPlib_EncryptedHdcpKeyStruct * pHdcpKeys);


#endif /* BHDCPLIB_KEYLOADER_H */

