/***************************************************************************
 *	   (c)2007-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").	Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.	   This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *	2.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.	   TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
#include "bhsm_keyladder_enc.h"

BDBG_MODULE(BHDCPLIB_KEYLOADER) ;


BERR_Code BHDCPlib_FastLoadEncryptedHdcpKeys(
	BHDCPlib_Handle hHDCPlib)
{
	BERR_Code errCode = BERR_SUCCESS;
	BHSM_GenerateKey2IO_t		generateKey2IO;
	BHSM_GenerateKeyXIO_t		generateKeyXIO;
	BHSM_GenerateHDMIKeyIO_t	 generateHDMIKeyIO;
	
	unsigned int i;
	uint32_t KeyParameter ;
	BHDCPlib_EncryptedHdcpKeyStruct * EncryptedHdcpKeys;
	
	BDBG_ASSERT(hHDCPlib) ;
	
	BDBG_ERR(("Version 20090403 - HDCPlib keyloader for 97550/97408")) ; 
	
	EncryptedHdcpKeys = (BHDCPlib_EncryptedHdcpKeyStruct *) &hHDCPlib->stHdcpConfiguration.TxKeySet.TxKeyStructure;
	
	BKNI_Memset( &generateKey2IO, 0, sizeof(BHSM_GenerateKey2IO_t))  ;	
	BKNI_Memset( &generateKeyXIO, 0, sizeof(BHSM_GenerateKeyXIO_t));
	BKNI_Memset( &generateHDMIKeyIO, 0, sizeof(BHSM_GenerateHDMIKeyIO_t));
	
	for (i =0; i< BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++)
	{
		if (!(hHDCPlib->stHdcpConfiguration.RxInfo.RxBksv[i / 8] & (1 << (i % 8))))
			continue ;

		generateKey2IO.eKey2Source = BHSM_Key2Source_eCustKey;
		generateKey2IO.eOperation = BHSM_Operation_eCP ; 
		generateKey2IO.unCustKey = EncryptedHdcpKeys[i].cusKeySel ;
		generateKey2IO.unKeyVar = EncryptedHdcpKeys[i].cusKeyVarL ;
		BKNI_Memset(generateKey2IO.procIn, 0, 16);
		BDBG_MSG(("generateKey2IO.unCustKey = %x generateKey2IO.unKeyVar = %x\n",generateKey2IO.unCustKey, generateKey2IO.unKeyVar));
		errCode=  BHSM_GenerateKey2 (hHDCPlib->stDependencies.hHsm, &generateKey2IO);

		if (errCode != 0) 
		{
			BDBG_ERR(("BHSM_GenerateKey2 key2 errCode: %x\n", errCode ));
			goto done;
		}

		/* generate Key3 from key 2*/
		generateKeyXIO.eOperation = BHSM_Operation_eCP;
		generateKeyXIO.eKeyDest = BHSM_KeyBuffer_eKey3 ;	
		BKNI_Memset(generateKeyXIO.procIn, 0, 16);
		KeyParameter = EncryptedHdcpKeys[i].TCaDataHi;
		BDBG_MSG(("EncryptedHdcpKeys[i].TCaDataHi = %x\n",KeyParameter));
		generateKeyXIO.procIn[0]  =  (KeyParameter >> 24) & 0xff ;
		generateKeyXIO.procIn[1]  =  (KeyParameter >> 16) & 0xff ;
		generateKeyXIO.procIn[2]  =  (KeyParameter >> 8) & 0xff ;
		generateKeyXIO.procIn[3]  =  (KeyParameter) & 0xff ;

		KeyParameter = EncryptedHdcpKeys[i].TCaDataLo;
		BDBG_MSG(("EncryptedHdcpKeys[i].TCaDataLo = %x\n",KeyParameter));
		generateKeyXIO.procIn[4]  =  (KeyParameter >> 24) & 0xff ;
		generateKeyXIO.procIn[5]  =  (KeyParameter >> 16) & 0xff ;
		generateKeyXIO.procIn[6]  =  (KeyParameter >> 8) & 0xff ;
		generateKeyXIO.procIn[7]  =  (KeyParameter) & 0xff ;					

		KeyParameter = EncryptedHdcpKeys[i].CaDataHi;
		BDBG_MSG(("EncryptedHdcpKeys[i].CaDataHi = %x\n",KeyParameter));
		generateKeyXIO.procIn[8]  =  (KeyParameter >> 24) & 0xff ;
		generateKeyXIO.procIn[9]  =  (KeyParameter >> 16) & 0xff ;
		generateKeyXIO.procIn[10]  =  (KeyParameter >> 8) & 0xff ;
		generateKeyXIO.procIn[11]  =  (KeyParameter) & 0xff ;

		KeyParameter = EncryptedHdcpKeys[i].CaDataLo;
		BDBG_MSG(("EncryptedHdcpKeys[i].CaDataLo = %x\n",KeyParameter));
		generateKeyXIO.procIn[12]  =  (KeyParameter >> 24) & 0xff ;
		generateKeyXIO.procIn[13]  =  (KeyParameter >> 16) & 0xff ;
		generateKeyXIO.procIn[14]  =  (KeyParameter >> 8) & 0xff ;
		generateKeyXIO.procIn[15]  =  (KeyParameter) & 0xff ;					

		errCode=  BHSM_GenerateKeyX (hHDCPlib->stDependencies.hHsm, &generateKeyXIO);
		if (errCode != 0) 
		{
			BDBG_ERR(("BHSM_GenerateKeyX key3 errCode: %x\n", errCode ));
			goto done;
		}	

		/* generate Key4(HDCP key) from key 3*/
		BKNI_Memset(&generateHDMIKeyIO.procIn, 0, sizeof(generateHDMIKeyIO.procIn));				
		KeyParameter = EncryptedHdcpKeys[i].HdcpKeyHi;
		BDBG_MSG(("EncryptedHdcpKeys[i].HdcpKeyHi = %x\n",KeyParameter));
		generateHDMIKeyIO.procIn[0]  =	(KeyParameter >> 24) & 0xff ;
		generateHDMIKeyIO.procIn[1]  =	(KeyParameter >> 16) & 0xff ;
		generateHDMIKeyIO.procIn[2]  =	(KeyParameter >> 8) & 0xff ;
		generateHDMIKeyIO.procIn[3]  =	(KeyParameter) & 0xff ;

		KeyParameter = EncryptedHdcpKeys[i].HdcpKeyLo;
		BDBG_MSG(("EncryptedHdcpKeys[i].HdcpKeyLo = %x\n",KeyParameter));
		generateHDMIKeyIO.procIn[4]  =	(KeyParameter >> 24) & 0xff ;
		generateHDMIKeyIO.procIn[5]  =	(KeyParameter >> 16) & 0xff ;
		generateHDMIKeyIO.procIn[6]  =	(KeyParameter >> 8) & 0xff ;
		generateHDMIKeyIO.procIn[7]  =	(KeyParameter) & 0xff ; 				

		generateHDMIKeyIO.keySlot = i ;

		errCode= BHSM_GenerateHDMIKey (hHDCPlib->stDependencies.hHsm, &generateHDMIKeyIO) ; 
		if (errCode != 0) 
		{
			BDBG_ERR(("BHSM_GenerateHDMIKey key4 errCode: %x\n", errCode ));
			goto done;
		}	
	}
	
done:
	return( errCode );
}

