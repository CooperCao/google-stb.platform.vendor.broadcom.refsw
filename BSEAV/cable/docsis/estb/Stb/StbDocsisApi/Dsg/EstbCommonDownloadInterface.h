//*****************************************************************************
// DSG COMMON DOWNLOAD API code
//****************************************************************************
//
// Copyright (c) 2007-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.  
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//*****************************************************************************
//    Filename:       EstbCommonDownloadInterface.h
//    Author:         Pinar Taskiran
//    Date:           12/17/2007
//
//*****************************************************************************
//    Revision History:
//                      0.1 Initial version.
//*****************************************************************************
//
//*****************************************************************************

//*****************************************************************************
#ifndef ESTB_CMN_DLOAD_H
#define ESTB_CMN_DLOAD_H

//********************** Include Files ***************************************
#if defined __cplusplus
extern "C" {
#endif

#include "bcmtypes.h"
#include "ProgramStoreDeviceDriver.h"

// Common Download Notifications
enum
{
	kMonolithicImageAuthenticationFailed		=0,
	kMonolithicImageAuthenticationSuccessful	=1,
	kMonolithicImageDownloadFailed				=2,
	kMonolithicImageDownloadSuccessful			=3,
	kEcmImageAuthenticationFailed				=4,			
	kEcmImageAuthenticationSuccessful			=5,	
	kEcmImageDownloadFailed						=6,			
	kEcmImageDownloadSuccessful					=7,
	kSnmpCvcUpdateFailed						=8,
	kConfigFileCvcFailed						=9,

	kConfigFileSwDloadTrigger					=10,
	kSnmpSwDloadTrigger							=11,
	kCvtSwDloadTrigger							=12,

	kCfgFileSwDloadTriggerSameFilename			=13,

	kEcmImageDownloadNotNeeded					=14,

	kCdlReturnCodeEnd

};

// CDL error codes for ocStbHostFirmwareDownloadFailedStatus Host MIB object.
enum
{
	kNoError  = 1,
	kCdlMfgMismatchName,
	kCdlCoSignerMismatchName,
	kCdlMfgSigningTimeLessThanCodeAccessStart,
	kCdlMfgSigningTimeGreaterThanCVCValidityEndTime,
	kCdlMfgValidityStartTimeLessThanCvcAccessStart,
	kCdlMfgSigningTimeLessThanCVCValidityStartTime,
	kCdlMfgImproperExtendedKeyUsageExtension,
	kCdlCoSignerSigningTimeLessThanCodeAccessStart,
	kCdlCoSignerSigningTimeGreaterThanCVCValidityEndTime,
	kCdlCoSignerValidityStartTimeLessThanCvcAccessStart,
	kCdlCoSignerSigningTimeLessThanCVCValidityStartTime,
	kCdlCoSignerImproperExtendedKeyUsageExtension,
	kCdlMfgCVCValidationFailure,
	kCdlMfgCVSValidationFailure,
	kCdlCoSignerCVCValidationFailure,
	kCdlCoSignerCVSValidationFailure,
	kCdlImproperConfigFileCVCFormat,
	kCdlConfigFileCVCValidationFailure,
	kCdlImproperSnmpCvcFormat,
	kCdlSnmpOrCvtCVCMfgMismatchName,
	kCdlReserved,
	kCdlSnmpCVCValidityStartTimeLessThanOrEqualCvcAccessStart,
	kCdlSnmpOrCvtCVCImproperExtendedKeyUsageExtension,
	kCdlSnmpCVCValidationFailure,

	// OC-SP-HOST2.1-CFR- Front Panel Display Requirements:
	// Table 11.4-4 - Error Codes
	// Ed26 File not found
	// Ed27 Server not available 
	kCdlErrorFileNotFound = 26,
	kCdlErrorNoResponseFromServer = 27,

	kCdlTftpHelperReadFailure = 40,
	kCdlShaDigestCodeImageFailure = 41,
	kCdlImproperCodeFileControls = 42,
	kCdlCvtCVCValidationFailure  = 43,
	kCdlClabCVCRootCACertValidationFailure = 44,
	kCdlClabCVCCACertValidationFailure = 45,
	kCdlCarouselDownloadNotSupported = 46,
	kCdlSecureDownloadNotSupported = 47,

	kCdlReturnValueEnd
};

// Common Download File Types
enum
{
	kMonolith		=1,
	kEcm			=2,
	kCvt			=3			
};

enum
{
	kAuthenticate		= 0x01,
	kUpgrade			= 0x02,

	kUseTftp			= 0x10		
};

enum
{
	kDigitalSignature = 0x01,
	kContent		  = 0x02
};

enum
{
	kManufCvc				= 0x01,
	kCosignerCvc			= 0x02,
	kCurrImageName			= 0x03,
	kSetDocsDevSwFilename	= 0x04,
	kGetCurrEcmImageVers	= 0x05,
	kGetCurrEcmImageCrc		= 0x06,
	kEstbSwDloadStatus		= 0x07
};

typedef enum
{
	kEstbSwDownloadInProgress			= 1,
	kEstbSwDownloadSuccess				= 2,
	kEstbSwDownloadFailed				= 3,
	kEstbSwDownloadUpgradeFailedCrc		= 4,
	kEstbSwDownloadUpgradeFailedLength  = 5,
	kEstbSwDownloadUpgradeFailedBoot	= 6
		
}EstbSwDownloadStatus;
				
typedef struct CommonDownloadResponseMsg
{
	uint32 returnCode;			
	uint32 returnValue;
} CommonDownloadResponseMsg;  

typedef struct CommonDownloadNotificationMsg
{
    uint32 eventCode;
    uint32 eventValue;
} CommonDownloadNotificationMsg;


typedef struct CommonDownloadExtNotificationMsg
{
    uint32 eventCode;
    uint32 eventValueLen;
	uint8* pEventValue;
} CommonDownloadExtNotificationMsg;


#if defined (COMMON_DOWNLOAD_SUPPORT)

void CommonDownloadAuthenticateOrUpgradeFile( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, 
											  uint32 ipAddrType, uint8* pTftpServerIpAddress, 
											  uint8 fileType, uint8 flags, CommonDownloadResponseMsg *responseMsg );

/*
 *	This function sends the digital signature to eCM for authentication.
 *  
 *  DOCSIS MIBs are updated per Common Download 2.0 specification. 
 *
 *	filePathOrBuffer - pointer to the buffer that holds the digital signature.				   
				   
 *	filePathOrBufferLen	- length of buffer that holds the digital signature.
					  NOTE that this function will return the length
					  of the digital signature in CommonDownloadResponseMsg. 
					  The caller of this function must to point to the start 
					  of the digital signature and send the length of the buffer 
					  that is holding the digital signature. 

 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  This function returns the result of the requested procedure and the length 
 *  of the digital signature in CommonDownloadResponseMsg buffer.
 *  
 */
void AuthenticateMonolithicImageDigitalSignature( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg  );

/*
 *	This function authenticates the signed content of a monolithic image.
 *	NOTE that the digital signature of the signed monolithic image must be 
 *	authenticated at eCM via AuthenticateMonolithicImageDigitalSignature()
 *	method prior to calling this function.
 *  
 *  DOCSIS MIBs are updated per Common Download 2.0 specification. 
 *
 *	filePathOrBuffer - path name of the unsigned image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the unsigned image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the unsigned image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void AuthenticateMonolithicImageSignedContent( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						                       uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg );

/*
 *	This function downloads an unsigned monolithic image via DOCSIS TFTP.
 *  
 *	filePathOrBuffer - path name of the unsigned image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the unsigned image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the unsigned image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void UpgradeUnsignedMonolithicImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						             uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg );
/*
 *	This function downloads a signed monolithic image via DOCSIS TFTP.
 *  
 *  DOCSIS MIBs are updated per CDL 2.0 specification.
 *
 *	filePathOrBuffer - path name of the signed image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the signed image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the signed image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void AuthenticateAndUpgradeMonolithicImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen,uint32 ipAddrType,
						                    uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg );

/*
 *	This function authenticates and validates a signed eCM image. 
 *  
 *	NOTE that this function will not update any DOCSIS MIB at eCM
 *	since doing so would overwrite the MIB values that are set
 *	in monolithic image download per CDL 2.0 specification.
 *
 *	filePathOrBuffer - path name of the signed image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the signed image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the signed image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void AuthenticateEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen,uint32 ipAddrType,
						   uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg );

/*
 *	This function upgrades eCM with an unsigned image. 
 *
 *	NOTE that this function will not update any DOCSIS MIB at eCM
 *	since doing so would overwrite the MIB values that are set
 *	in monolithic image download per CDL 2.0 specification.
 *  
 *	filePathOrBuffer - path name of the unsigned image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the unsigned image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the unsigned image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void UpgradeUnsignedEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						      uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg,
							  uint32 checkVersion);
/*
 *	This function upgrades eCM with an unsigned image.
 *  This function will check the CRC value, instead of the version number, to see if the upgrade is necessary. 
 *
 *	NOTE that this function will not update any DOCSIS MIB at eCM
 *	since doing so would overwrite the MIB values that are set
 *	in monolithic image download per CDL 2.0 specification.
 *  
 *	filePathOrBuffer - path name of the unsigned image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the unsigned image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the unsigned image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void UpgradeUnsignedEcmImageByCrc( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						      uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg,
							  uint32 checkVersion);

/*
 *	This function upgrades eCM with a signed image.
 *
 *	NOTE that this function will not update any DOCSIS MIB at eCM
 *	since doing so would overwrite the MIB values that are set
 *	in monolithic image download per CDL 2.0 specification.
 *  
 *	filePathOrBuffer - path name of the signed image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the signed image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the signed image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void AuthenticateAndUpgradeEcmImage( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						             uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg,
									 uint32 checkVersion);

/*
 *	This function upgrades eCM with a signed image.
 *  This function will check the CRC value, instead of the version number, to see if the upgrade is necessary. 
 *
 *	NOTE that this function will not update any DOCSIS MIB at eCM
 *	since doing so would overwrite the MIB values that are set
 *	in monolithic image download per CDL 2.0 specification.
 *  
 *	filePathOrBuffer - path name of the signed image.				   
 *
 *	filePathOrBufferLen	- length of the path name of the signed image.
 *
 *  tftpServerIpAddress - IP address of the TFTP server that serves the signed image.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  The return code that is returned in CommonDownloadResponseMsg indicates whether
 *  eCM received the request or not.
 *  The result of the requested procedure will be received via CommonDownloadProcessEcmNotification.
 *  
 */
void AuthenticateAndUpgradeEcmImageByCrc( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, uint32 ipAddrType,
						             uint8* pTftpServerIpAddress, CommonDownloadResponseMsg* pCommonDownloadResponseMsg,
									 uint32 checkVersion);

/*
 *	This function sends the signed CVT to eCM for authentication.
 *  
 *	filePathOrBuffer - pointer to the buffer that holds the signed CVT file.				   
 *
 *	filePathOrBufferLen	- length of buffer that holds the signed CVT file.
 *
 *	pCommonDownloadResponseMsg - pointer to the CommonDownloadResponseMsg buffer.
 *								 which is provided by the calling method. 
 *
 *  This function returns the result of the requested procedure and the length 
 *  of the digital signature in CommonDownloadResponseMsg buffer.
 *  
 */
void AuthenticateCvtFile( uint8* filePathOrBuffer, uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg );

/*
 *	This function send the CVC to eCM for authentication.
 *  
 *	cvcBuffer - pointer to the CVC buffer
 *	cvcLen	- length of the CVC buffer
 *	cvcType	- type of the CVC: 
 * 
 *            kManufCvc or kCosignerCvc
 *
 *	This function returns 
 *	true == success; CVC validated
 *	false == failed. CVC is invalid.
 *
 *  This function returns the result of the requested procedure and the length 
 *  of the CVC in CommonDownloadResponseMsg buffer.
 *  
 */
int CommonDownloadProcessCvc( uint8* cvcBuffer,  uint32 cvcLen, uint8 cvcType, CommonDownloadResponseMsg* pCommonDownloadResponseMsg);

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
void CommonDownloadProcessEcmNotification( uint8* pBuf );

void CommonDownloadSetEcmNotificationCallback( void (*callback)(CommonDownloadExtNotificationMsg)) ;

void CommonDownloadResponseMessage(uint8 *pBuf, CommonDownloadResponseMsg *pResponseMsg);

int CommonDownloadVerifyCodeImageShaDigest( uint8* codeImageShaDigest, CommonDownloadResponseMsg* pCommonDownloadResponseMsg);
int SetRunningMonolithicImageName( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg);
int SetDocsDevSwFilename( uint8* filePathOrBuffer,  uint32 filePathOrBufferLen, CommonDownloadResponseMsg* pCommonDownloadResponseMsg);

void GetLocalEcmImageVersion(const char *fileName, uint16 *pMajor, uint16 *pMinor, uint16 *pSubMinor, uint32 imageIsSigned);
void GetLocalEcmImageCrc(const char *fileName, uint32 *pCrc, uint32 imageIsSigned);
void PrintProgramStoreDriverProgramHeader( BcmProgramHeader * pProgramHeader );
int GetRunningEcmImageVersion( 	uint16 *pMajorVersion, uint16 *pMinorVersion, uint16 *pSubMinorVersion );
int GetRunningEcmImageCrc( 	uint32 *pCrc );
unsigned int CheckForEcmUpgrade(const char *fnPtr, uint8 *pUpgradeNeeded, uint32 imageIsSigned);
unsigned int CheckForEcmUpgradeByCrc(const char *fnPtr, uint8 *pUpgradeNeeded, uint32 imageIsSigned);
unsigned int BcmCdlUpdateSnmpMibs(uint32 estbDownloadStatus, CommonDownloadResponseMsg *pCommonDownloadResponseMsg );

#endif

#if defined __cplusplus
	}
#endif


#endif
