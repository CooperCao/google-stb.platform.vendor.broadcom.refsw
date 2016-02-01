/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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

/* #define READ_FROM_BINARY_FILE_TEST */
/* BINARY_FILE_TEST  CAN NOT be done in kernel mode! */

#ifdef READ_FROM_BINARY_FILE_TEST
#include <stdio.h>
#endif
 
#include "bhdcplib_keyloader.h"

BDBG_MODULE(BHDCPLIB_HDCPKEYS) ;


static const uint8_t HdcpTxAksv[BAVC_HDMI_HDCP_KSV_LENGTH] =
	{0x14, 0xF7, 0x61, 0x03, 0xB7} ;

static const BHDCPlib_EncryptedHdcpKeyStruct EncryptedHdcpTxKeys[BAVC_HDMI_HDCP_N_PRIVATE_KEYS]= 
{   
	/* LSB.. MSB */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},  
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400}, 
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
} ;


BERR_Code BHDCPlib_GetKeySet(BAVC_HDMI_HDCP_KSV pTxAksv, BHDCPlib_EncryptedHdcpKeyStruct * pHdcpKeys) 
{
	
#ifdef READ_FROM_BINARY_FILE_TEST
	FILE *fp ;	
	uint8_t pad[3] ;
	
	fp = fopen("keys.bin", "rb");
	if (!fp) 
	{
		BDBG_MSG(("Error opening binary key file")) ;
		return BERR_INVALID_PARAMETER ;
	}
	
	BDBG_WRN(("Test Reading from Binary File")) ;
	fread(pTxAksv, sizeof(uint8_t), BAVC_HDMI_HDCP_KSV_LENGTH, fp) ;
	
	/* remember Bksv is padded with three 0x00 to round out to 8 bytes */
	/* skip over these bytes */
	fread(pad, sizeof(uint8_t), 3, fp) ;
	
	fread(pHdcpKeys, sizeof(BHDCPlib_EncryptedHdcpKeyStruct), BAVC_HDMI_HDCP_N_PRIVATE_KEYS, fp) ;
	
	/* Verify bytes read into TxAksv and pHdcpKeys using SHA-1 algorithm */
	
#else

	BKNI_Memcpy(pTxAksv, HdcpTxAksv, BAVC_HDMI_HDCP_KSV_LENGTH) ; 

	BKNI_Memcpy(pHdcpKeys, EncryptedHdcpTxKeys,
		sizeof(BHDCPlib_EncryptedHdcpKeyStruct) * BAVC_HDMI_HDCP_N_PRIVATE_KEYS) ;

#endif		

	return BERR_SUCCESS ;
}
