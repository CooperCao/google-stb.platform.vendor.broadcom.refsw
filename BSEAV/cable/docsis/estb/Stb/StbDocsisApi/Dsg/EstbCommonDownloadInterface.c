/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
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
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EstbCommonDownloadInterface.h"
#include "EstbRemoteInterfaceStub.h"

#include "rProgramStoreCustomerApi.h"

//****************** Extern/Local Functions prototypes ************************

static void (* commonDownloadNotificationCallback) (CommonDownloadExtNotificationMsg) = NULL;
extern uint16 HostToNetworkUint16(uint16 value);
extern uint16 HostToNetworkUint32(uint32 value);
//********************** Local Variables *************************************

#if defined (COMMON_DOWNLOAD_SUPPORT)
/*
 * This function sends the Secure Header (digital signature)
 * to eCM for authentication.
 *
 *	filePathOrBuffer - pointer to the Secure Header buffer.

 *	filePathOrBufferLen	- length of buffer that contains the Secure Header.
					  NOTE that this function will return the length
					  of the secure header. The caller of this function
					  needs to point to the start of the Secure Header
					  and send the length of the buffer that is holding
					  the Secure Header.
 *
 *
 *
 * This function returns the length of the Secure HEader (Digital Signature).
 *
 */
void AuthenticateMonolithicImageDigitalSignature( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg  )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint32 tftpServerIpAddress = 0;

	fileType = (kMonolith << 4) | kDigitalSignature;
	flags = kAuthenticate;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, 0, (uint8*)&tftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateMonolithicImageSignedContent( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						                       uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;

	fileType = (kMonolith << 4) | kContent;
	flags = kAuthenticate | kUseTftp;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateMonolithicImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						          uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;

	fileType = (kMonolith << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate | kUseTftp;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void UpgradeUnsignedMonolithicImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						             uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;

	fileType = (kMonolith << 4) | kContent;
	flags = kUpgrade | kUseTftp;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateAndUpgradeMonolithicImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						                    uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;

	fileType = (kMonolith << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate | kUpgrade | kUseTftp;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}


void AuthenticateEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						   uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;

	fileType = (kEcm << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate | kUseTftp;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

/*
	UpgradeUnsignedEcmImage

	return value - check pCommonDownloadResponseMsg fields
 */
void UpgradeUnsignedEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
							  uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg, uint32 checkVersion )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint8 upgradeNeeded = 0;
	uint32 imageIsSigned = 0;

	fileType = (kEcm << 4) | kContent;
	flags = kUpgrade | kUseTftp;

	memset(pCommonDownloadResponseMsg, 0, sizeof(*pCommonDownloadResponseMsg));

	if( checkVersion )
	{
		if( CheckForEcmUpgrade((const char*) filePathOrBuffer, &upgradeNeeded, imageIsSigned) == 0)
		{
			/* version check failed */
			return;
		}

		if(upgradeNeeded == 0)
		{
			// return code is success? not a failure.
			pCommonDownloadResponseMsg->returnCode = 1;
			pCommonDownloadResponseMsg->returnValue = kEcmImageDownloadNotNeeded;
			return;
		}
	}

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

/*
	UpgradeUnsignedEcmImageByCrc

	return value - check pCommonDownloadResponseMsg fields
 */
void UpgradeUnsignedEcmImageByCrc( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
							  uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg, uint32 checkVersion )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint8 upgradeNeeded = 0;
	uint32 imageIsSigned = 0;

	fileType = (kEcm << 4) | kContent;
	flags = kUpgrade | kUseTftp;

	memset(pCommonDownloadResponseMsg, 0, sizeof(*pCommonDownloadResponseMsg));

	if( checkVersion )
	{
		if( CheckForEcmUpgradeByCrc((const char*) filePathOrBuffer, &upgradeNeeded, imageIsSigned) == 0)
		{
			/* CRC check failed */
			return;
		}

		if(upgradeNeeded == 0)
		{
			// return code is success? not a failure.
			pCommonDownloadResponseMsg->returnCode = 1;
			pCommonDownloadResponseMsg->returnValue = kEcmImageDownloadNotNeeded;
			return;
		}
	}

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateAndUpgradeEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
									 uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg, uint32 checkVersion )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint8 upgradeNeeded = 0;
	uint32 imageIsSigned = 1;

	fileType = (kEcm << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate | kUpgrade | kUseTftp;

	memset(pCommonDownloadResponseMsg, 0, sizeof(*pCommonDownloadResponseMsg));

	if( checkVersion )
	{
		if( CheckForEcmUpgrade((const char*) filePathOrBuffer, &upgradeNeeded, imageIsSigned) == 0)
		{
			/* version check failed */
			return;
		}

		if(upgradeNeeded == 0)
		{
			// return code is success? not a failure.
			pCommonDownloadResponseMsg->returnCode = 1;
			pCommonDownloadResponseMsg->returnValue = kEcmImageDownloadNotNeeded;
			return;
		}
	}

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateAndUpgradeEcmImageByCrc( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
									 uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg, uint32 checkVersion )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint8 upgradeNeeded = 0;
	uint32 imageIsSigned = 1;

	fileType = (kEcm << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate | kUpgrade | kUseTftp;

	memset(pCommonDownloadResponseMsg, 0, sizeof(*pCommonDownloadResponseMsg));

	if( checkVersion )
	{
		if( CheckForEcmUpgradeByCrc((const char*) filePathOrBuffer, &upgradeNeeded, imageIsSigned) == 0)
		{
			/* CRC check failed */
			return;
		}

		if(upgradeNeeded == 0)
		{
			// return code is success? not a failure.
			pCommonDownloadResponseMsg->returnCode = 1;
			pCommonDownloadResponseMsg->returnValue = kEcmImageDownloadNotNeeded;
			return;
		}
	}

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, ipAddrType, pTftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

void AuthenticateCvtFile( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg )
{
	uint8 fileType = 0;
	uint8 flags = 0;
	uint32 tftpServerIpAddress = 0;

	fileType = (kCvt << 4) | kContent | kDigitalSignature;
	flags = kAuthenticate;

	CommonDownloadAuthenticateOrUpgradeFile( filePathOrBuffer, filePathOrBufferLen, 0, (uint8*) &tftpServerIpAddress, fileType, flags, pCommonDownloadResponseMsg );
}

/*
      Diagram of the Header:

	+---------+----------------------+-------+	+---------+----------------------+-------+ +---------+----------------------+-------+
	| TransId | Command | File Type  | Flags |  |          IP address type               | |          Tftp Server IP Address        |
	+---------+----------------------+-------+	+---------+----------------------+-------+ +---------+----------------------+-------+
	 1 byte   1 byte     1 byte       1 Byte                     4 bytes                                16 bytes

	+---------------+------------------------+	+---------------+------------------------+
	|      FilePathOrBuffer Length           |  |          FilePathOrBuffer
	+---------------+------------------------+	+---------------+------------------------+
					4 Bytes								FilePathOrBuffer Length bytes



	uint8 fileType;					upper 4 bits: kCvt, kEcm, kMonolith, lower 4-bits is a bitmask: kDigitalSignature, kContent
	uint8 flags;					bitmask that indicates the request and how the file should be retrieved: kAuthenticate, kUpgrade and kUseTftp


	typedef struct CommonDownloadResponseMsg
	{
		uint32 returnCode;
		uint32 returnValue;;
	} CommonDownloadResponseMsg;

*/

/*
 * This function reports the result of a sw download or authentication.
 * CUSTOMER MUST IMPLEMENT THIS FUNCTION TO GET NOTIFIED OF THE DOWNLOAD STATUS.
 *
 *	downloadResult - result of the sw download or authentication.
 *
 *	enum
	{
		kMonolithicImageAuthenticationFailed		=0,
		kMonolithicImageAuthenticationSuccessful	=1,
		kMonolithicImageDownloadFailed				=2,
		kMonolithicImageDownloadSuccessful			=3,
		kEcmImageAuthenticationFailed				=4,
		kEcmImageAuthenticationSuccessful			=5,
		kEcmImageDownloadFailed						=6,
		kEcmImageDownloadSuccessful					=7,
		kSnmpCvcUpdateFailed						=8
	};
 */
void CommonDownloadProcessEcmNotification( uint8* pBuf )
{
	char* names[] =
	{
		"Monolithic Image Authentication Failed",
		"Monolithic Image Authentication Successful",
		"Monolithic Image Download Failed",
		"Monolithic Image Download Successful",
		"eCM Image Authentication Failed",
		"eCM Image Authentication Successful",
		"eCM Image Download Failed",
		"eCM Image Download Successful",
		"SNMP CVC Update Failed",
		"eCM Config File CVC Failure",
		"eCM Config File Sw Download Trigger",
		"eCM SNMP Sw Download Trigger",
		"eSTB CVT Sw Download Trigger",
		"eCM Config File Trigger/Same file name"
	};

	char* cdlErrorCodes[] =
	{
		"",
		"No Failure", // ??
		"CDL-ERROR-2: Improper code file controls - CVC subject organizationName for manufacturer does not match the Host device manufacturer name",
		"CDL-ERROR-3: Improper code file controls - CVC subject organizationName for code cosigning agent does not match the Host device current code cosigning agent.",
		"CDL-ERROR-4: Improper code file controls - The manufacturer's PKCS #7 signingTime value is less-than the codeAccessStart value currently held in the Host device.",
		"CDL-ERROR-5: Improper code file controls - The manufacturer's PKCS #7 signingTime is greater than the CVC validity end time.",
		"CDL-ERROR-6: Improper code file controls - The manufacturer's CVC validity start time is less-than the cvcAccessStart value currently held in the Host device.",
		"CDL-ERROR-7: Improper code file controls - The manufacturer's PKCS #7 signingTime value is less-than the CVC validity start time.",
		"CDL-ERROR-8: Improper code file controls - Missing or improper extendedKeyUsage extension in the manufacturer CVC.",
		"CDL-ERROR-9: Improper code file controls - The cosigner's PKCS #7 signingTime value is less-than the codeAccessStart value currently held in the Host device.",
		"CDL-ERROR-10: Improper code file controls - The cosigner's PKCS #7 signingTime is greater than CVC validity end time.",
		"CDL-ERROR-11: Improper code file controls - The cosigner's CVC validity start time is less-than the cvcAccessStart value currently held in the Host device.",
		"CDL-ERROR-12: Improper code file controls - The cosigner's PKCS #7 signingTime value is less-than the CVC validity start time.",
		"CDL-ERROR-13: Improper code file controls - Missing or improper extended key-usage extension in the cosigner's CVC.",
		"CDL-ERROR-14: Code file manufacturer CVC validation failure.",
		"CDL-ERROR-15: Code file manufacturer CVS validation failure.",
		"CDL-ERROR-16: Code file cosigner CVC validation failure.",
		"CDL-ERROR-17: Code file cosigner CVS validation failure.",
		"CDL-ERROR-18: Improper eCM configuration file CVC format (e.g., missing or improper key usage attribute).",
		"CDL-ERROR-19: eCM configuration file CVC validation failure.",
		"CDL-ERROR-20: Improper SNMP CVC format.",
		"CDL-ERROR-21: CVC subject organizationName for manufacturer does not match the Host devices manufacturer name.",
		"", // reserved
		"CDL-ERROR-23: The CVC validity start time is less-than or equal-to the corresponding subject's cvcAccessStart value currently held in the Host device.",
		"CDL-ERROR-24: Missing or improper key usage attribute for CVCs other than the eCM configuration file CVC.",
		"CDL-ERROR-25: SNMP CVC validation failure.",

		"ED26:File not found",
		"ED27:Server not available",

		"", "", "", "", "", "", "", "", "", "", "", "",
		"TFTP Read Failure",
		"SHA-Digest Failure",
		"Improper CodeFile Controls",
		"CVT CVC Validation Failure",
		"ClabCVCRootCACert Validation Failure",
		"ClabCVCCACert Validation Failure",
		"Carousel Download Not Supported",
		"Secure Download Not Supported"
	};

	CommonDownloadNotificationMsg notification;

	CommonDownloadExtNotificationMsg extNotification;
	CommonDownloadResponseMsg *pResponseMsg = NULL;

	pBuf++;

	// We know the data length is sizeof(CommonDownloadResponseMsg), so skip over the bytes that show the length.
	// 4 bytes are used to hold the length.
	pBuf += 4;

	// Load the data from pBuf to extNotification data fields.
	pResponseMsg = (CommonDownloadResponseMsg*) &notification;
	CommonDownloadResponseMessage(pBuf, pResponseMsg);

	switch(pResponseMsg->returnCode)
	{
		case kConfigFileSwDloadTrigger:
		case kSnmpSwDloadTrigger:
		case kCvtSwDloadTrigger:
			{
				// Load extNotification data fields.
				extNotification.eventCode = notification.eventCode;
				extNotification.eventValueLen = notification.eventValue;

				// Point to filename
				extNotification.pEventValue = pBuf + 2 * sizeof(uint32);

				// Null-terminate
				extNotification.pEventValue[extNotification.eventValueLen] = 0;

				//LogRawBuffer ("CommonDownloadProcessEcmNotification", extNotification.pEventValue, (int)extNotification.eventValueLen, 0);

				printf(" %s \n", names[extNotification.eventCode]);
				printf("CDL notification = %d  filename = %s   length =%d\n", (int)extNotification.eventCode, extNotification.pEventValue, (int)extNotification.eventValueLen);
			}
			break;

		default:
			{
				// Load extNotification data fields.
				extNotification.eventCode = notification.eventCode;
				extNotification.eventValueLen = sizeof(uint32);
				extNotification.pEventValue = (uint8*)&notification.eventValue;

				printf("CDL Notification code = %d  value = %d\n", (int)extNotification.eventCode, (int)(*extNotification.pEventValue));
				if( (extNotification.eventCode < kMonolithicImageAuthenticationFailed) ||
					(extNotification.eventCode >= kCdlReturnCodeEnd))
				{
					return;
				}
				printf(" %s ", names[extNotification.eventCode]);

				if( (*extNotification.pEventValue < kNoError) ||
					(*extNotification.pEventValue >= kCdlReturnValueEnd))
				{
					printf(" CDL return value not recognized.\n");
				}
				else
				{
					printf(" %s \n", cdlErrorCodes[*extNotification.pEventValue]);
				}
			}
			break;
	}

	if(commonDownloadNotificationCallback)
		(commonDownloadNotificationCallback)(extNotification);
}

void CommonDownloadResponseMessage(uint8 *pBuf, CommonDownloadResponseMsg *pResponseMsg)
{
	// Now read the return code and the info and return to the caller.
	pResponseMsg->returnCode  = (*pBuf++) << 24;
	pResponseMsg->returnCode |= (*pBuf++) << 16;
	pResponseMsg->returnCode |= (*pBuf++) << 8;
	pResponseMsg->returnCode |=  *pBuf++;

	pResponseMsg->returnValue  = (*pBuf++) << 24;
	pResponseMsg->returnValue |= (*pBuf++) << 16;
	pResponseMsg->returnValue |= (*pBuf++) << 8;
	pResponseMsg->returnValue |= (*pBuf);

	printf("returnCode = %08x returnValue = %08x \n", (unsigned int)pResponseMsg->returnCode, (unsigned int)pResponseMsg->returnValue);
}

void CommonDownloadSetEcmNotificationCallback( void (*callback)(CommonDownloadExtNotificationMsg))
{
	commonDownloadNotificationCallback = callback;
}

/*
 * This function sends the CVC to eCM for authentication.
 *
 *	cvcBuffer - pointer to the CVC buffer
 *	cvcLen	- length of the CVC buffer
 *	cvcType	- type of the CVC:
 *
 *            kManufCvc or kCosignerCvc
 *
 * This function returns
 * true == success; CVC validated
 * false == failed. CVC is invalid.
 *
 */
int CommonDownloadProcessCvc( uint8* cvcBuffer,  uint32 cvcLen, uint8 cvcType, CommonDownloadResponseMsg* pCommonDownloadResponseMsg)
{
	return CommonDownloadProcessEstbRequest( cvcType, cvcLen, cvcBuffer, pCommonDownloadResponseMsg);
}

/*
 * This function sends the running monolithic image name to eCM.
 *
 *	filePathOrBuffer - pointer to the image name.

 *	filePathOrBufferLen	- length of the image name
 *
 */
int SetRunningMonolithicImageName( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg)
{
	return CommonDownloadProcessEstbRequest( kCurrImageName, filePathOrBufferLen, filePathOrBuffer, pCommonDownloadResponseMsg);
}

/*
 * This function sets docsDevSwFilename.
 *
 *	filePathOrBuffer - pointer to the image name.

 *	filePathOrBufferLen	- length of the image name
 *
 */
int SetDocsDevSwFilename( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg)
{
	return CommonDownloadProcessEstbRequest( kSetDocsDevSwFilename, filePathOrBufferLen, filePathOrBuffer, pCommonDownloadResponseMsg);
}

/*
 * This function gets the version of the running eCM image.
 *

 *
 */
int GetRunningEcmImageVersion( 	uint16 *pMajorVersion, uint16 *pMinorVersion, uint16 *pSubMinorVersion )
{
	CommonDownloadResponseMsg commonDownloadResponseMsg;
	memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));

	if(CommonDownloadProcessEstbRequest( kGetCurrEcmImageVers, 0, NULL, &commonDownloadResponseMsg) )
	{
		*pMajorVersion = commonDownloadResponseMsg.returnValue >> 16;
		*pMinorVersion = (commonDownloadResponseMsg.returnValue & 0xFF) >> 8;
		*pSubMinorVersion = commonDownloadResponseMsg.returnValue & 0x7f;

		return 1;
	}

	return 0;
}

/*
 * This function gets the CRC value of the running eCM image.
 *

 *
 */
int GetRunningEcmImageCrc( uint32 *pCrc )
{
	CommonDownloadResponseMsg commonDownloadResponseMsg;
	memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));

	if(CommonDownloadProcessEstbRequest( kGetCurrEcmImageCrc, 0, NULL, &commonDownloadResponseMsg) )
	{
		*pCrc = commonDownloadResponseMsg.returnValue;

		return 1;
	}

	return 0;
}

unsigned int CheckForEcmUpgrade(const char *fnPtr, uint8 *pUpgradeNeeded, uint32 imageIsSigned)
{
	uint16 majorVersion;
	uint16 minorVersion;
	uint16 subMinorVersion;

	uint16 ecmMajorVersion;
	uint16 ecmMinorVersion;
	uint16 ecmSubMinorVersion;

		// Retrieve eCM image version from local filesystem
		GetLocalEcmImageVersion(fnPtr, &majorVersion, &minorVersion, &subMinorVersion, imageIsSigned);

		if ((majorVersion == 0) && (minorVersion == 0) && (subMinorVersion == 0))
		{
			printf("Could not determine version of image %s \n", fnPtr);
			return 0;
		}

		printf(" '%s' version = %d.%d.%d \n", fnPtr, majorVersion, minorVersion, subMinorVersion);

		// Retrieve eCM image version from eCM
		if (!GetRunningEcmImageVersion(&ecmMajorVersion, &ecmMinorVersion, &ecmSubMinorVersion))
		{
			printf("Could not determine version of eCM image.  \n");

			printf("eCM requires upgrade!\n");
			*pUpgradeNeeded = 1;
			return 1;
			//return 0;
		}

		printf("eCM version = %d.%d.%d \n", ecmMajorVersion, ecmMinorVersion, ecmSubMinorVersion);

		bool upgrade = false;
		if (majorVersion > ecmMajorVersion)
		{
			upgrade = true;
		}
		else if (majorVersion == ecmMajorVersion)
		{
			if (minorVersion > ecmMinorVersion)
			{
				upgrade = true;
			}
			else if (minorVersion == ecmMinorVersion)
			{
				if (subMinorVersion > ecmSubMinorVersion)
				{
					upgrade = true;
				}
			}
		}

		if (!upgrade)
		{
			printf("eCM does not require upgrade!\n");
			*pUpgradeNeeded = 0;
			return 1;
		}

		printf("eCM requires upgrade!\n");
		*pUpgradeNeeded = 1;
		return 1;
}

unsigned int CheckForEcmUpgradeByCrc(const char *fnPtr, uint8 *pUpgradeNeeded, uint32 imageIsSigned)
{
	uint32 imageCrc;
	uint32 ecmCrc;
	bool upgrade = false;

		// Retrieve eCM image crc from local filesystem
		GetLocalEcmImageCrc(fnPtr, &imageCrc, imageIsSigned);

		printf(" '%s' CRC = 0x%08X \n", fnPtr, (unsigned int)imageCrc);

		// Retrieve eCM image version from eCM
		if (!GetRunningEcmImageCrc(&ecmCrc))
		{
			printf("Could not determine CRC value of eCM image.  \n");
			printf("eCM requires upgrade!\n");

			*pUpgradeNeeded = 1;
			return 1;
		}

		printf("eCM CRC = 0x%08X \n", (unsigned int)ecmCrc);

		if (imageCrc != ecmCrc)
		{
			upgrade = true;
		}

		if (!upgrade)
		{
			printf("eCM does not require upgrade!\n");
			*pUpgradeNeeded = 0;
			return 1;
		}

		printf("eCM requires upgrade!\n");
		*pUpgradeNeeded = 1;
		return 1;
}

// BCDToBin(uint16 bcd)
//		Convert 16 bit binary code decival value to 16 bit binary value
//
// Parameters:
//      bcd - 16 bit binary code decimal value.
//
// Returns:
//      16 bit binary value.
//
static uint16 BCDToBin(uint16 bcd)
{
	uint16 bin = 0, digit;

	for (digit = 0; digit < 4; digit++)
	{
		bin = (bin * 10) + ((bcd & 0xf000) >> 12);
		bcd <<= 4;
	}
	return bin;
}

// GetLocalFileVersion(const char *fileName, uint16 &major, uint16 &minor, uint16 &subMinor)
//		Parse image stored in local file system and extract the version of the image.
//
// Parameters:
//		fileName - Pointer to null terminated file name string.
//      major - Major version number.
//      minor - Minor version number.
//      subMinor - Minor sub-version number.
//		imageIsSigned - Flag to indicate image has a secure header (1) or not (0)
//
// Returns:
//      None. If fails then major, minor and subMinor are returned as 0
//
void GetLocalEcmImageVersion(const char *fileName, uint16 *pMajor, uint16 *pMinor, uint16 *pSubMinor, uint32 imageIsSigned)
{
	*pMajor = 0;
	*pMinor = 0;
	*pSubMinor = 0;
	size_t bytesRead = 0;
	FILE *fh;

	// NOTE: This is very implementation dependent. This reference design assumes
	// a local file system where the signed image exists as a file called
	// signed_ecram_sto.bin.
#ifdef kDefaultValue_EstbTftpServerPath
	// if kDefaultValue_EstbTftpServerPath is defined, eSTB will find the eCM image at kDefaultValue_EstbTftpServerPath.
	char tempFilename[256];
	snprintf(tempFilename, 256, "%s/%s", kDefaultValue_EstbTftpServerPath, fileName);
	fh = fopen(tempFilename, "r");
#else
	// if kDefaultValue_EstbTftpServerPath is not defined, eSTB will find the image at current working directory.
	fh = fopen(fileName, "r");
#endif
	if (fh == NULL)
	{
		printf("Unable to open eCM image!\n" );
		return;
	}

	do { // Do once. This construct provides a way to bail simply by doing a 'break'.

		if( imageIsSigned )
		{
			// Read the TLV tag which should be 0x30 (PKCS data)
			uint8 len8;
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pas end of eCM signed image!\n" );
				break;
			}
			if (len8 != 0x30)
			{
				printf("eCM signed image is missing PKCS tag!\n" );
				break;
			}
			// Read the TLV length
			uint16 len16;
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}
			// Check for extended lenth field
			if (len8 & 0x80)
			{
				// Extended length field, compute the actual length
				len16 = 0;
				uint8 count = len8 & 0x7f;
				bool error = false;
				while (count--)
				{
					len16 <<= 8;
					bytesRead = fread(&len8, 1, sizeof(len8), fh);
					if (bytesRead < sizeof(len8))
					{
						printf("Attempt to read pass end of eCM signed image!\n" );

						error = true;
						break;
					}
					len16 |= len8;
				}
				if (error)
					break;
			}
			else
			{
				// Simple length, use it
				len16 = len8;
			}

			// Seek past the PKCS
			if (fseek(fh, len16, SEEK_CUR) != 0)
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}

			// Read the 1st byte of the download header
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}
			if (len8 != 0x1c)
			{
				printf("eCM signed image is missing download header!\n" );
				break;
			}

			// Read the length of the download header
			bytesRead = fread(&len16, 1, sizeof(len16), fh);
			if (bytesRead < sizeof(len16))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}

			// Skip past download header
			if (len16)
			{
				if (fseek(fh, len16, SEEK_CUR) != 0)
				{
					printf("Attempt to read pass end of eCM signed image!\n" );
					break;
				}
			}
		}

		// Finally! We are at the image. Read the header
		BcmProgramHeader header;
		bytesRead = fread(&header, 1, sizeof(header), fh);
		if (bytesRead < sizeof(header))
		{
			printf("Attempt to read pass end of eCM  image!\n" );
			break;
		}

		PrintProgramStoreDriverProgramHeader(&header);

		*pMajor = BCDToBin(HostToNetworkUint16(header.usMajorRevision));
		*pMinor = BCDToBin(HostToNetworkUint16(header.usMinorRevision) >> 8);
		*pSubMinor = BCDToBin(HostToNetworkUint16(header.usMinorRevision) & 0x7f);

	} while (0);

	fclose(fh);

}

// GetLocalEcmImageCrc(const char *fileName, uint32 *pCrc, uint32 imageIsSigned)
//		Parse image stored in local file system and extract the version of the image.
//
// Parameters:
//		fileName - Pointer to null terminated file name string.
//      pCrc - pointer to crc value.
//		imageIsSigned - Flag to indicate image has a secure header (1) or not (0)
//
// Returns:
//      None. If fails then major, minor and subMinor are returned as 0
//
void GetLocalEcmImageCrc(const char *fileName, uint32 *pCrc, uint32 imageIsSigned)
{
	*pCrc = 0;
	size_t bytesRead = 0;
	FILE *fh;

	// NOTE: This is very implementation dependent. This reference design assumes
	// a local file system where the signed image exists as a file called
	// signed_ecram_sto.bin.
#ifdef kDefaultValue_EstbTftpServerPath
	// if kDefaultValue_EstbTftpServerPath is defined, eSTB will find the eCM image at kDefaultValue_EstbTftpServerPath.
	char tempFilename[256];
	snprintf(tempFilename, 256, "%s/%s", kDefaultValue_EstbTftpServerPath, fileName);
	fh = fopen(tempFilename, "r");
#else
	// if kDefaultValue_EstbTftpServerPath is not defined, eSTB will find the image at current working directory.
	fh = fopen(fileName, "r");
#endif
	if (fh == NULL)
	{
		printf("Unable to open eCM image!\n" );
		return;
	}

	do { // Do once. This construct provides a way to bail simply by doing a 'break'.

		if( imageIsSigned )
		{
			// Read the TLV tag which should be 0x30 (PKCS data)
			uint8 len8;
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pas end of eCM signed image!\n" );
				break;
			}
			if (len8 != 0x30)
			{
				printf("eCM signed image is missing PKCS tag!\n" );
				break;
			}
			// Read the TLV length
			uint16 len16;
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}
			// Check for extended lenth field
			if (len8 & 0x80)
			{
				// Extended length field, compute the actual length
				len16 = 0;
				uint8 count = len8 & 0x7f;
				bool error = false;
				while (count--)
				{
					len16 <<= 8;
					bytesRead = fread(&len8, 1, sizeof(len8), fh);
					if (bytesRead < sizeof(len8))
					{
						printf("Attempt to read pass end of eCM signed image!\n" );

						error = true;
						break;
					}
					len16 |= len8;
				}
				if (error)
					break;
			}
			else
			{
				// Simple length, use it
				len16 = len8;
			}

			// Seek past the PKCS
			if (fseek(fh, len16, SEEK_CUR) != 0)
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}

			// Read the 1st byte of the download header
			bytesRead = fread(&len8, 1, sizeof(len8), fh);
			if (bytesRead < sizeof(len8))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}
			if (len8 != 0x1c)
			{
				printf("eCM signed image is missing download header!\n" );
				break;
			}

			// Read the length of the download header
			bytesRead = fread(&len16, 1, sizeof(len16), fh);
			if (bytesRead < sizeof(len16))
			{
				printf("Attempt to read pass end of eCM signed image!\n" );
				break;
			}

			// Skip past download header
			if (len16)
			{
				if (fseek(fh, len16, SEEK_CUR) != 0)
				{
					printf("Attempt to read pass end of eCM signed image!\n" );
					break;
				}
			}
		}

		// Finally! We are at the image. Read the header
		BcmProgramHeader header;
		bytesRead = fread(&header, 1, sizeof(header), fh);
		if (bytesRead < sizeof(header))
		{
			printf("Attempt to read pass end of eCM  image!\n" );
			break;
		}

		PrintProgramStoreDriverProgramHeader(&header);

		*pCrc = HostToNetworkUint32(header.ulcrc);

	} while (0);

	fclose(fh);

}

/*
 * This function updates SW download SNMP MIBs.
 *
 *	estbDownloadStatus -
 *
 */
unsigned int BcmCdlUpdateSnmpMibs(uint32 estbDownloadStatus, CommonDownloadResponseMsg *pCommonDownloadResponseMsg )
{
	// htonl
	estbDownloadStatus = htonl(estbDownloadStatus);
	return CommonDownloadProcessEstbRequest( kEstbSwDloadStatus, sizeof(uint32), (uint8*) &(estbDownloadStatus), pCommonDownloadResponseMsg);
}

/*---------------------------------------------------------------------------
    Name: ProgramStoreDriverPrintProgramHeader
 Purpose: Display the image header contents
  Inputs:
        BcmProgramHeader - pointer to a program header in memory
 Returns:
        Nothing
   Notes:
---------------------------------------------------------------------------*/
void PrintProgramStoreDriverProgramHeader( BcmProgramHeader * pProgramHeader )
{
	printf("\n************************************\n"
            "ProgramHeader\n"
            "************************************\n"
            "      Signature: %x"
            "\n      Control: %x"
            "\n    Major Rev: %x"
            "\n    Minor Rev: %x"

	        "\n  File Length: %d"
	        "\n Load Address: %08x"
	        "\n     Filename: %s"
	        "\n          HCS: %x"
	        "\n          CRC: %x\n"
	        "\n************************************\n"
							, ntohl(pProgramHeader->usSignature)
							, ntohl(pProgramHeader->usControl)
							, ntohl(pProgramHeader->usMajorRevision)
							, ntohl(pProgramHeader->usMinorRevision)
							, ntohl(pProgramHeader->ulfilelength)
							, ntohl(pProgramHeader->ulProgramLoadAddress)
							, pProgramHeader->cFilename
							, ntohl(pProgramHeader->usHcs)
							, ntohl(pProgramHeader->ulcrc));

}

#endif

