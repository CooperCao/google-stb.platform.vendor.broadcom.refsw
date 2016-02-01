/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
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

#include "bstd.h"
#include "bkni.h"
#include "bhdcplib_keyloader.h"

#include "bhsm.h"
#include "bhsm_keyladder.h"


#define BHDCPLib_KEY_OFFSET 0x80

BDBG_MODULE(BHDCPLIB_KEYLOADER) ;



BERR_Code BHDCPlib_FastLoadEncryptedHdcpKeys(
	BHDCPlib_Handle hHDCPlib)
{
	BERR_Code errCode = BERR_SUCCESS;
	uint8_t i;
	BHSM_EncryptedHdcpKeyStruct * EncryptedHdcpKeys ;

	BDBG_ASSERT(hHDCPlib) ;
	BDBG_MSG(("HDCP Key Loader URSR 14.4")) ;

	/* Get HDCP Encrypted Keys */
	EncryptedHdcpKeys = (BHSM_EncryptedHdcpKeyStruct *)
		&hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure ;

	for (i =0; i< BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++)
	{
		/* skip keys that are not specified to be used by the RxBksv */
		if (!(hHDCPlib->stHdcpConfiguration.RxInfo.RxBksv[i / 8] & (1 << (i % 8))))
			continue ;

		errCode = BHSM_FastLoadEncryptedHdcpKey(
			hHDCPlib->stDependencies.hHsm, i + BHDCPLib_KEY_OFFSET, &(EncryptedHdcpKeys[i]) ) ;

		if (errCode != BERR_SUCCESS)
		{
			BDBG_ERR(("BHSM_FastLoadEncryptedHdcpKey errCode: %x", errCode )) ;
			BERR_TRACE(errCode) ;
			break ;
		}
	}

	return( errCode );
}

