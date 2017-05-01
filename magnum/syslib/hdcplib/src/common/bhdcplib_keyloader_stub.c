/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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

		hHDCPlib->stDependencies.lockHsm();
		errCode= BHSM_LoadRouteUserKey (hHDCPlib->stDependencies.hHsm, &loadRouteUserKeyIO); 
		hHDCPlib->stDependencies.unlockHsm();
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


