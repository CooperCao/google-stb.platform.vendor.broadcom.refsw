/***************************************************************************
 *     Copyright (c) 2002-2013, Broadcom Corporation
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

#include "bhsm.h"
#include "bhsm_keyladder.h"
#include "bhdcplib_keyloader.h"
#include "breg_endian.h"

BDBG_MODULE(BHDCPLIB_KEYLOADER) ;


BERR_Code BHDCPlib_FastLoadEncryptedHdcpKeys(
	BHDCPlib_Handle hHDCPlib)
{
#if !BHDM_CONFIG_65NM_SUPPORT || (BCHP_CHIP == 7550) || (BCHP_CHIP == 7408) 

	BDBG_WRN(("*********************************************************")) ;
	BDBG_WRN(("HDCP feature is disabled. HDCP authentication will")) ;
	BDBG_WRN(("not succeed. Please rebuild the application with ")) ; 
	BDBG_WRN(("NEXUS_HDCP_SUPPORT=y to enable HDCP")) ;
	BDBG_WRN(("*********************************************************")) ;

	BSTD_UNUSED(hHDCPlib);

	/* HDCP will not authenticate; force the app to exiit */
	BDBG_ASSERT(false) ;
	return BERR_NOT_SUPPORTED;
#else

	uint8_t  i ;
	uint32_t HdcpKey[4] ;
		
	BERR_Code rc = BERR_SUCCESS ;
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_LoadRouteUserKeyIO_t loadRouteUserKeyIO;	
	BHDCPlib_EncryptedHdcpKeyStruct * EncryptedHdcpKeys;
	
	BDBG_ASSERT(hHDCPlib) ;


	/* Get HDCP Encrypted Keys */
	EncryptedHdcpKeys = (BHDCPlib_EncryptedHdcpKeyStruct *) &hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure;

	BDBG_WRN(("*********************************************************")) ;
	BDBG_WRN(("Loading HDCP Spec 1.1 A1 Transmitter Test Keys...")) ;
	BDBG_WRN(("HDCP Test and Production Keys are not compatible")) ; 
	BDBG_WRN(("Test Keys WILL NOT AUTHENTICATE with production TVs, RXs")) ;
	BDBG_WRN(("Separate Agreement needed for use of Encrypted Key Loader")) ;
	BDBG_WRN(("*********************************************************")) ;

	
	HdcpKey[1] = 0;
	HdcpKey[0] = 0;
	
	BDBG_MSG(("Start HDCP Key Loading")) ;
	for(i= 0; i < BHDM_HDCP_NUM_KEYS ; i++)
	{
		HdcpKey[3] = EncryptedHdcpKeys[i].HdcpKeyLo ;
		HdcpKey[2] = EncryptedHdcpKeys[i].HdcpKeyHi ;
		
		BREG_LE32(HdcpKey[3]) ;
		BREG_LE32(HdcpKey[2]) ;
		
#if 0
		BDBG_MSG(("%08x%08x %08X %08X %08X %08X %d", 
			EncryptedHdcpKeys[i].HdcpKeyHi, EncryptedHdcpKeys[i].HdcpKeyLo,
			HdcpKey[3], HdcpKey[2], HdcpKey[1], HdcpKey[0], i)) ;
#endif			

		loadRouteUserKeyIO.keySource =	BCMD_KeyRamBuf_eSecondRam;	/* BCMD_KeyRamBuf_eFirstRam;*/
		loadRouteUserKeyIO.keySize.eKeySize =  BCMD_KeySize_e64;
		BKNI_Memset(loadRouteUserKeyIO.aucKeyData, 0, sizeof(loadRouteUserKeyIO.aucKeyData));

		loadRouteUserKeyIO.aucKeyData[3] =	(HdcpKey[2] >> 24) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[2] =	(HdcpKey[2] >> 16) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[1] =	(HdcpKey[2] >>	8) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[0] =	(HdcpKey[2]) & 0xff ;
		
		loadRouteUserKeyIO.aucKeyData[7] =	(HdcpKey[3] >> 24) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[6] =	(HdcpKey[3] >> 16) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[5] =	(HdcpKey[3] >>	8) & 0xff ;
		loadRouteUserKeyIO.aucKeyData[4] =	(HdcpKey[3]) & 0xff ;	

		
		loadRouteUserKeyIO.bIsRouteKeyRequired = true ;
		loadRouteUserKeyIO.keyDestBlckType	 =	BCMD_KeyDestBlockType_eHdmi ;
		loadRouteUserKeyIO.keyDestEntryType = BCMD_KeyDestEntryType_eOddKey; /* Aegis does not care */ 
		loadRouteUserKeyIO.caKeySlotType	  = BCMD_XptSecKeySlot_eType0;	   /* Aegis does not care */ 
		loadRouteUserKeyIO.unKeySlotNum = i ;					

		errCode= BHSM_LoadRouteUserKey (hHDCPlib->stDependencies.hHsm, &loadRouteUserKeyIO); 
		if (errCode != 0) 
		{
			BDBG_MSG(("loadRouteUserKeyIO unStatus = 0x%08X", loadRouteUserKeyIO.unStatus)) ;
			BDBG_ERR(("BHSM_LoadRouteUserKey errCode: %x", errCode )) ; 
			return errCode;
		}	
	}

	BDBG_MSG(("END HDCP Key Loading")) ;

	return rc ;
#endif
}


