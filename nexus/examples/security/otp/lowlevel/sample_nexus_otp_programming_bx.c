/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "nexus_otpmsp.h"

NEXUS_Error EnableSecureBoot(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eSecureBootEnable;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if(readMspIO.mspDataBuf[3]==0x0) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eSecureBootEnable;
		progMspIO.dataBitLen     = 1;
		progMspIO.dataBitMask[0] = 0x0;
		progMspIO.dataBitMask[1] = 0x0;
		progMspIO.dataBitMask[2] = 0x0;
		progMspIO.dataBitMask[3] = 0x01;
		progMspIO.mspData[0]     = 0x0;
		progMspIO.mspData[1]     = 0x0;
		progMspIO.mspData[2]     = 0x0;
		progMspIO.mspData[3]     = 0x1;
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eSecureBootEnable;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if(readMspIO.mspDataBuf[3] != 0x1) 
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}

NEXUS_Error CrLockEnable(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eCrLockEnable;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if(readMspIO.mspDataBuf[3]==0x0) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eCrLockEnable;
		progMspIO.dataBitLen     = 1;
		progMspIO.dataBitMask[0] = 0x0;
		progMspIO.dataBitMask[1] = 0x0;
		progMspIO.dataBitMask[2] = 0x0;
		progMspIO.dataBitMask[3] = 0x01;
		progMspIO.mspData[0]     = 0x0;
		progMspIO.mspData[1]     = 0x0;
		progMspIO.mspData[2]     = 0x0;
		progMspIO.mspData[3]     = 0x1;
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eCrLockEnable;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if(readMspIO.mspDataBuf[3] != 0x1) 
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}

NEXUS_Error ForceDramScrambler(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eForceDramScrambler;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if(readMspIO.mspDataBuf[3]==0x0) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eForceDramScrambler;
		progMspIO.dataBitLen     = 1;
		progMspIO.dataBitMask[0] = 0x0;
		progMspIO.dataBitMask[1] = 0x0;
		progMspIO.dataBitMask[2] = 0x0;
		progMspIO.dataBitMask[3] = 0x01;
		progMspIO.mspData[0]     = 0x0;
		progMspIO.mspData[1]     = 0x0;
		progMspIO.mspData[2]     = 0x0;
		progMspIO.mspData[3]     = 0x1;
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eForceDramScrambler;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if(readMspIO.mspDataBuf[3] != 0x1) 
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}

NEXUS_Error PublicKey0Index(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_ePublicKey0Index;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if(readMspIO.mspDataBuf[3]==0x0) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_ePublicKey0Index;
		progMspIO.dataBitLen     = 4;
		progMspIO.dataBitMask[0] = 0x0;
		progMspIO.dataBitMask[1] = 0x0;
		progMspIO.dataBitMask[2] = 0x0;
		progMspIO.dataBitMask[3] = 0x07;
		progMspIO.mspData[0]     = 0x0;
		progMspIO.mspData[1]     = 0x0;
		progMspIO.mspData[2]     = 0x0;
		progMspIO.mspData[3]     = 0x0;	/*replace 0x0 with the value to be programmed for PublicKey0Index*/
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_ePublicKey0Index;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if(readMspIO.mspDataBuf[3] != 0x0) /*replace 0x0 with the value to be programmed for PublicKey0Index*/
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}

NEXUS_Error MarketId(unsigned long market_id)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eMarketId;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if( (readMspIO.mspDataBuf[0]==0x0) &&
		(readMspIO.mspDataBuf[1]==0x0) &&
		(readMspIO.mspDataBuf[2]==0x0) &&
		(readMspIO.mspDataBuf[3]==0x0) ) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eMarketId;
		progMspIO.dataBitLen     = 32;
		progMspIO.dataBitMask[0] = 0xff;
		progMspIO.dataBitMask[1] = 0xff;
		progMspIO.dataBitMask[2] = 0xff;
		progMspIO.dataBitMask[3] = 0xff;
		progMspIO.mspData[0]     = (market_id >> 24) & 0xFF;
		progMspIO.mspData[1]     = (market_id >> 16) & 0xFF;
		progMspIO.mspData[2]     = (market_id >> 8) & 0xFF;
		progMspIO.mspData[3]     = (market_id) & 0xFF;		/*replace 0xFF with the value to be programmed for MarketID*/
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eMarketId;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if( (readMspIO.mspDataBuf[0] != ((market_id >> 24) & 0xFF)) ||
			(readMspIO.mspDataBuf[1] != ((market_id >> 16) & 0xFF)) ||
			(readMspIO.mspDataBuf[2] != ((market_id >> 8) & 0xFF)) ||
			(readMspIO.mspDataBuf[3] != ((market_id) & 0xFF)) )	/*replace 0xFF with the value to be programmed for MarketID*/
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}




NEXUS_Error ASKMStbOwnerID(void StbOwnerID)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eReserved43;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if( (readMspIO.mspDataBuf[0]==0x0) &&
		(readMspIO.mspDataBuf[1]==0x0) &&
		(readMspIO.mspDataBuf[2]==0x0) &&
		(readMspIO.mspDataBuf[3]==0x0) ) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eReserved43;
		progMspIO.dataBitLen     = 16;
		progMspIO.dataBitMask[0] = 0x00;
		progMspIO.dataBitMask[1] = 0x00;
		progMspIO.dataBitMask[2] = 0xff;
		progMspIO.dataBitMask[3] = 0xff;
		progMspIO.mspData[0]     = 0x00;
		progMspIO.mspData[1]     = 0x00;
		progMspIO.mspData[2]     = (StbOwnerID>> 8) & 0xFF;
		progMspIO.mspData[3]     = (StbOwnerID) & 0xFF;	/*replace 0xFF with the value to be programmed for StbOwnerID*/
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eReserved43;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if( (readMspIO.mspDataBuf[0] != (0x00)) ||
			(readMspIO.mspDataBuf[1] != (0x00)) ||
			(readMspIO.mspDataBuf[2] != ((StbOwnerID>> 8) & 0xFF)) ||
			(readMspIO.mspDataBuf[3] != ((StbOwnerID) & 0xFF)) )	/*replace 0xFF with the value to be programmed for StbOwnerID*/
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}


NEXUS_Error HostBootCodeDecryptionSelect(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = NEXUS_OtpCmdMsp_eReserved80;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if( (readMspIO.mspDataBuf[0]==0x0) &&
		(readMspIO.mspDataBuf[1]==0x0) &&
		(readMspIO.mspDataBuf[2]==0x0) &&
		(readMspIO.mspDataBuf[3]==0x0) ) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = NEXUS_OtpCmdMsp_eReserved80;
		progMspIO.dataBitLen     = 16;
		progMspIO.dataBitMask[0] = 0x00;
		progMspIO.dataBitMask[1] = 0x00;
		progMspIO.dataBitMask[2] = 0x00;
		progMspIO.dataBitMask[3] = 0x0f;
		progMspIO.mspData[0]     = 0x00;
		progMspIO.mspData[1]     = 0x00;
		progMspIO.mspData[2]     = 0x00;
		progMspIO.mspData[3]     = 0x0F;	/*replace 0x0F with the value to be programmed for HostBootCodeDecryptionSelect*/
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  NEXUS_OtpCmdMsp_eReserved80;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if( (readMspIO.mspDataBuf[0] != (0x00)) ||
			(readMspIO.mspDataBuf[1] != (0x00)) ||
			(readMspIO.mspDataBuf[2] != (0x00)) ||
			(readMspIO.mspDataBuf[3] != (0x0F))
		  )	/*replace 0x0F with the value to be programmed for HostBootCodeDecryptionSelect*/
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}


NEXUS_Error DramScramblerKeyReuseDisable(void)
{
	NEXUS_ReadMspParms readMspParams;
	NEXUS_ProgramMspIO progMspIO;
	NEXUS_ReadMspIO readMspIO;
	NEXUS_Error rc;

	readMspParams.readMspEnum    = BCMD_Otp_CmdMsp_eReserved152;
	if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
		return rc;

	if(readMspIO.mspDataBuf[3]==0x0) 
	{
		progMspIO.progMode       = 0x10112;
		progMspIO.progMspEnum    = BCMD_Otp_CmdMsp_eReserved152;
		progMspIO.dataBitLen     = 1;
		progMspIO.dataBitMask[0] = 0x0;
		progMspIO.dataBitMask[1] = 0x0;
		progMspIO.dataBitMask[2] = 0x0;
		progMspIO.dataBitMask[3] = 0x01;
		progMspIO.mspData[0]     = 0x0;
		progMspIO.mspData[1]     = 0x0;
		progMspIO.mspData[2]     = 0x0;
		progMspIO.mspData[3]     = 0x1;
		
		/* Issue command to program the OTP. */
		if(rc = NEXUS_Security_ProgramMSP (&progMspIO))
			return rc;
		
		readMspParams.readMspEnum =  BCMD_Otp_CmdMsp_eReserved152;

		if(rc = NEXUS_Security_ReadMSP (&readMspParams, &readMspIO))
			return rc;

		if(readMspIO.mspDataBuf[3] != 0x1) 
			return NEXUS_INVALID_PARAMETER;
	}
	
	return NEXUS_SUCCESS;

}
