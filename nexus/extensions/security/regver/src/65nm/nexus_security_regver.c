/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/

#include "nexus_security_module.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"
#include "nexus_security_regver.h"

#include "bhsm.h"
#include "bhsm_verify_reg.h"
#include "bhsm_misc.h"

BDBG_MODULE(nexus_security);

void NEXUS_Security_GetDefaultRegionSettings(
    NEXUS_SecurityCodeRegionSettings *pSettings
    )
{
	if (pSettings==NULL)
		return;

    BKNI_Memset(pSettings, 0, sizeof(NEXUS_SecurityCodeRegionSettings));

}

NEXUS_Error NEXUS_Security_DefineRegion (const NEXUS_SecurityCodeRegionSettings * pRegion, unsigned int * pRegionID)
{
	BHSM_Handle hHsm;
	BHSM_VerifyRegionIO_t  verifyIO;
	BERR_Code errCode;
	unsigned int sizeRSAsign;

	NEXUS_Security_GetHsm_priv (&hHsm);
	if ( !hHsm || !pRegion || !pRegionID)
	{
		return NEXUS_NOT_SUPPORTED;
	}

	if ( pRegion->sigSize == NEXUS_SecurityRegverSignatureSize_e128Bytes )
		sizeRSAsign = 128;
	else if ( pRegion->sigSize == NEXUS_SecurityRegverSignatureSize_e256Bytes )
		sizeRSAsign = 256;
	else
	{
		return NEXUS_INVALID_PARAMETER;
	}

	BKNI_Memset(&verifyIO, 0, sizeof(BHSM_VerifyRegionIO_t));
	verifyIO.operation = BCMD_MemAuth_Operation_eEnableRegion|0x100;
	verifyIO.ucRegionNumber = pRegion->regionID;
	verifyIO.ucNumberOfBadBlocks = 0;
	verifyIO.unRegionStartAddress = pRegion->codeStartPhyAddress; /* region start_addr */;
	verifyIO.unRegionEndAddress = pRegion->codeStartPhyAddress+pRegion->codeSize-1; /*  region end_addr */;
	verifyIO.unSignatureStartAddress = pRegion->sigStartPhyAddress ; /* RSA start_addr */   ;
	verifyIO.unSignatureEndAddress = pRegion->sigStartPhyAddress+sizeRSAsign-1; /* RSA end_addr */;
	verifyIO.ucIntervalCheckBw = 0x10;
	verifyIO.unKeyId = pRegion->keyID;
	verifyIO.unCodeRule = pRegion->codeRule;

	errCode = BHSM_VerifyRegion (hHsm ,  &verifyIO);
	if ( errCode == BERR_SUCCESS )
	{
		BDBG_WRN(("\nDefine region %d verification successfully \n", pRegion->regionID));
		*pRegionID = pRegion->regionID;
		return NEXUS_SUCCESS;
	}
	else 
	{
		BDBG_WRN(("\nDefine region %d verification failed status = %x\n", pRegion->regionID, verifyIO.unStatus));
		*pRegionID = NEXUS_CRYPTO_INVALID_REGION;
		return NEXUS_NOT_SUPPORTED;
	}

}


NEXUS_Error NEXUS_Security_EnableRegion (unsigned int regionID)
{
	volatile unsigned int status;
	BHSM_Handle hHsm;

	BHSM_VerifyRegionIO_t  verifyIO;
	BERR_Code errCode;
		
	NEXUS_Security_GetHsm_priv (&hHsm);
	if ( !hHsm || regionID==NEXUS_CRYPTO_INVALID_REGION )
	{
		return NEXUS_NOT_SUPPORTED;
	}

	BKNI_Memset(&verifyIO, 0, sizeof(BHSM_VerifyRegionIO_t));

	do
	{
		verifyIO.operation = 7;
		verifyIO.ucRegionNumber = regionID;
		verifyIO.ucNumberOfBadBlocks = 0;
		verifyIO.unStatus = 0;
		errCode = BHSM_VerifyRegion (hHsm ,  &verifyIO);
		if ( errCode == BERR_SUCCESS )
		{
			BDBG_MSG(("\nEnable region %d verification successfully \n", regionID));
		}
		else 
		{
			BDBG_MSG(("\nEnable region %d verification failed status = %x\n", regionID, verifyIO.unStatus));
		}
		status =  verifyIO.unStatus;/*( unsigned int*) (BSP_REGISTER_ADDRESS_BASE|0x10307c94)*/ ;
		if ( status != 0 )
		{
			BKNI_Sleep(50);	/* delay*/
		}
	}while (status != 0);

	do
	{
		verifyIO.operation = BCMD_MemAuth_Operation_eRegionVerified;
		verifyIO.ucRegionNumber = regionID;
		BDBG_WRN(("Checking region %d\n", regionID));
		verifyIO.unStatus = 0;
		errCode = BHSM_VerifyRegion (hHsm ,  &verifyIO);
		status =  verifyIO.unStatus;/*( unsigned int*) (BSP_REGISTER_ADDRESS_BASE|0x10307c94)*/ ;
		if ( status == 0xBB) 
		{
			BDBG_ERR(("Region %d signature check failed\n", regionID));
			return NEXUS_UNKNOWN;
		}
		if ( status != 0 )
		{
			BKNI_Sleep(50);	/* delay*/
		}
	}while (status != 0);

	return NEXUS_SUCCESS;

}


NEXUS_Error NEXUS_Security_DisableRegion (unsigned int regionID)
{
	volatile unsigned int status;
	BHSM_Handle hHsm;
	BHSM_VerifyRegionIO_t  verifyIO;
	BERR_Code errCode;

	NEXUS_Security_GetHsm_priv (&hHsm);
	if ( !hHsm || regionID==NEXUS_CRYPTO_INVALID_REGION )
		return NEXUS_NOT_SUPPORTED;

	BKNI_Memset(&verifyIO, 0, sizeof(BHSM_VerifyRegionIO_t));
	verifyIO.operation = BCMD_MemAuth_Operation_eDisableRegion;
	verifyIO.ucRegionNumber = regionID;

	errCode = BHSM_VerifyRegion (hHsm ,  &verifyIO);
	if ( errCode != BERR_SUCCESS )
	{
		BDBG_MSG(("\nDisable region %d verification successfully \n", regionID));
		return NEXUS_NOT_SUPPORTED;
	}

	do
	{
		verifyIO.operation = BCMD_MemAuth_Operation_eQueryRegionInfo;
		verifyIO.ucRegionNumber = regionID;
		BDBG_MSG(("Querying region %d\n", regionID));
		verifyIO.unStatus = 0;
		errCode = BHSM_VerifyRegion (hHsm ,  &verifyIO);
		status =  verifyIO.unRegionStatus[regionID] & 1;/*( unsigned int*) (BSP_REGISTER_ADDRESS_BASE|0x10307c94)*/ ;
		if ( status != 0 )
		{
			BKNI_Sleep(50);	/* delay*/
		}
	}while (status != 0);
	
	return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_PauseRegion (unsigned int regionID)
{
	/* To be implemeted later */
	BSTD_UNUSED(regionID);
	return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Security_VerifyRegion (const NEXUS_SecurityCodeRegionSettings * pRegion, unsigned int * pRegionID)
{
	BERR_Code errCode;

	errCode=NEXUS_Security_DefineRegion(pRegion, pRegionID);
	if ( errCode != NEXUS_SUCCESS ){
		BDBG_WRN(("Define region %x failed....\n", *pRegionID));
		/* This error shall be ignored so that nexus still works on Non-OTP programmed board */
		return NEXUS_SUCCESS;
		}
	return NEXUS_Security_EnableRegion (*pRegionID);
}
