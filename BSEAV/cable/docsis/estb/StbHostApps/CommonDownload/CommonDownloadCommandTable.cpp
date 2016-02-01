//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//****************************************************************************
//
//  Filename:       CommonDownloadCommandTable.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  July 20, 2007
//
//****************************************************************************
//  Description:
//      This derived class handles controlling the DSG-CC objects.
//
//****************************************************************************
     
//********************** Include Files ***************************************

// My api and definitions...
#include "CommonDownloadCommandTable.h"
#include "EstbCommonDownloadInterface.h"

#include "DsgClientCtlThread.h"
#include "DsgClientRegistrar.h"
#include "DsgClientId.h"
#include "EthernetPacket.h"

#include "BitmaskCommandParameter.h"
#include "BoolCommandParameter.h"
#include "FlagCommandParameter.h"
#include "IpAddressCommandParameter.h"
#include "StringCommandParameter.h"
#include "StringSetCommandParameter.h"
#include "UnsignedIntCommandParameter.h"
#include "MacAddressCommandParameter.h"

#include "OperatingSystemFactory.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
static const char *pgTableDescr = "Test commands for Open Cable Common Download 2.0 eCM-eSTB API. ";

enum
{
	kAuthenticateOnly,
	kSecureUpgrade,
	kUnsecureUpgrade,
	kProcessCvc,
	kVerifyCodeImageShaDigest
};

enum
{
	kSecureHeader,
	kSignedContent,
	kCodeImage,
	kSecureCvt,
	kManuf,
	kCosigner
};

enum
{
	kSignedImage,
	kUnsignedImage
};

enum
{
	kCheckByVersion,
	kCheckByCrc
};

enum
{
	kMonolithicImg,
	kEcmImg,
	kProcess,
	kSimCarouselDload,
	kSimCvtTftpDload,
	kSetImageName,
	kSetDocsDevSwFileName,
	kLocalEcmImgVersion,
	kLocalEcmImgCrc,
	kEcmImgUpgrade,
	kTestEstbSwDloadStatus
};

// Constants for the command parameters.
enum
{
    kPrimaryParameter = 1,
    kSecondParameter,
    kThirdParameter,
    kFourthParameter,
    kImageFlag,
    kSecureFlag,
    kLargeImageFlag,
    kFilenameParameter,
    kForceImageFlag,        // PR1744
};

//********************** Local Variables *************************************
//********************** Local Functions *************************************
//********************** Class Method Implementations ************************


// Default Constructor.  Creates the command table, adds all of the
// commands to it and sets the command handler function.
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmCommonDownloadCommandTable::BcmCommonDownloadCommandTable(void) :
    BcmCommandTable("Common Download eCM-eSTB API Test Commands", "common_download", "common_download", pgTableDescr)
{
 
    BcmCommand command;

    // Set up fields that don't change.
    command.fIsEnabled = true;
    command.fIsHidden = false;
    command.fCommandFunctionWithParms = CommandHandler;

	command.fCommandId = kMonolithicImg;
	command.pfCommandName = "monolithic_img";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("auth|s_upgrade|upgrade", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("sechdr|content|image", kSecondParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, true, "filename", 255, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kFourthParameter, true));
	command.pfCommandHelp = "monolithic_img <auth|s_upgrade|upgrade> <sechdr|content|image> <path to filename> <tftp server IP address>";
	command.pfExamples = "monolithic_img auth sechdr sec_hdr_ecram_sto.bin.7455 10.24.64.80\n"
						 "Authenticate secure header of code image\tAPI tested: void AuthenticateMonolithicImageDigitalSignature(..)\n\n"						  
						 "monolithic_img auth content ecram_sto.bin.7455 10.24.64.80\n"
						 "Authenticate signed content of code image.\tAPI tested: void AuthenticateMonolithicImageSignedContent(..)\n\n"												 
						 "monolithic_img s_upgrade image signed_ecram_sto.bin.7455 10.24.64.80\n"
						 "Upgrade signed image.\tAPI tested: void AuthenticateAndUpgradeMonolithicImage(..)\n\n"						 
						 "monolithic_img upgrade image ecram_sto.bin.7455 10.24.64.80\n"
						 "Upgrade unsigned image.\tAPI tested: void UpgradeUnsignedMonolithicImage(..)\n\n";
	AddCommand(command);

	command.fCommandId = kEcmImg;
	command.pfCommandName = "ecm_img";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("auth|s_upgrade|upgrade", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("sechdr|content|image|", kSecondParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, true, "filename", 255, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kFourthParameter, true));
	command.pfCommandHelp = "ecm_img <auth|s_upgrade|upgrade> image <path to filename> <tftp server IP address>";
	command.pfExamples = "ecm_img auth sechdr sec_hdr_ecram_sto.bin.3255 10.24.64.80\n"
						 "-- NOT SUPPORTED Authenticate secure header of eCM code image.\n\n" 
						 "ecm_img auth content ecram_sto.bin.3255 10.24.64.80\n"
						 "-- NOT SUPPORTED Authenticate signed content of eCM code image.\n\n"
						 "ecm_img auth image signed_ecram_sto.bin.3255 192.168.17.10\n"
						 "-- Authenticate signed image.\tAPI tested: void AuthenticateEcmImage(..)\n\n" 
						 "ecm_img s_upgrade image signed_ecram_sto.bin.3255 192.168.17.10\n"
						 "-- Upgrade signed image.\tAPI tested: void AuthenticateAndUpgradeEcmImage(..)\n\n"
						 "ecm_img upgrade image	ecram_sto.bin.3255 192.168.17.10\n"
						 "-- Upgrade unsigned image.\tAPI tested: void UpgradeEcmImage(..)\n";
	AddCommand(command);

	command.fCommandId = kProcess;
	command.pfCommandName = "process";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("auth|s_upgrade|upgrade|cvc|sha_digest", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("sechdr|content|image|cvt|manuf|cosign", kSecondParameter, true));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, true, "filename", 255, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kFourthParameter, true));
	command.pfCommandHelp = "process <auth|cvc|sha_digest> <cvt|manuf|cosign> <path to filename>";
	command.pfExamples = "process auth cvt <path to file>\n"
						 "Process signed CVT.\tAPI tested: void AuthenticateCvtFile(..)\n\n" 	
						 "process cvc manuf <path to file>\n"
						 "Process manufacturer CVC.\tAPI tested: void CommonDownloadProcessCvc(..)\n\n"
					     "process cvc cosign <path to file>\n"
						 "Process co-signer CVC.\tAPI tested: void CommonDownloadProcessCvc(..)\n\n"
					     "process sha_digest\n"
						 "Verify code image SHA-digest.\tAPI tested: int CommonDownloadVerifyCodeImageShaDigest(..)\n\n";
	AddCommand(command);

	command.fCommandId = kSetImageName;
	command.pfCommandName = "set_img_name";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "monolithic image name", 255, false));
	command.pfCommandHelp = "set_img_name <imagename>";
	command.pfExamples = "set_img_name monolithic_image.bin \n"
						 "Send current running image name to eCM\tAPI tested: int SetRunningMonolithicImageName(..)\n\n";
	AddCommand(command);

	command.fCommandId = kSetDocsDevSwFileName;
	command.pfCommandName = "set_docsDevSwFilename";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "monolithic image name", 255, false));
	command.pfCommandHelp = "set_docsDevSwFilename <imagename>";
	command.pfExamples = "set_docsDevSwFilename 7425/monolithic_image.bin \n"
						 "Set eCM's docsDevSwFilename MIB\tAPI tested: int SetDocsDevSwFilename(..)\n\n";;
	AddCommand(command);

	command.fCommandId = kLocalEcmImgVersion;
	command.pfCommandName = "get_local_ecm_img_vers";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "eCM image name", 255, false));
	command.pfCommandHelp = "Reads the program header of the local file ";
	command.pfExamples = "get_local_ecm_img_vers ecram_sto.bin";
	AddCommand(command);

	command.fCommandId = kLocalEcmImgCrc;
	command.pfCommandName = "get_local_ecm_img_crc";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kPrimaryParameter, true, "eCM image name", 255, false));
	command.pfCommandHelp = "Reads the program header of the local file ";
	command.pfExamples = "get_local_ecm_img_crc ecram_sto.bin";
	AddCommand(command);

	command.fCommandId = kEcmImgUpgrade;
	command.pfCommandName = "ecm_img_upgrade";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("signed|unsigned", kPrimaryParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringSetCommandParameter("ver|crc", kSecondParameter, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmStringCommandParameter(kThirdParameter, true, "filename", 255, false));
	command.pfCommandParameterList->AddOrderDependent(new BcmIpAddressCommandParameter(kFourthParameter, true));
	command.pfCommandHelp = "ecm_img_upgrade <signed|unsigned> <ver|crc> <path to filename> <tftp server IP address>";
	command.pfExamples = "ecm_img_upgrade signed ver signed_ecram_sto.bin 192.168.17.10\n"
						 "-- Upgrade signed image.\tAPI tested: void AuthenticateAndUpgradeEcmImage(..)\n\n"
						 "ecm_img_upgrade signed crc signed_ecram_sto.bin 192.168.17.10\n"
						 "-- Upgrade signed image.\tAPI tested: void AuthenticateAndUpgradeEcmImageByCrc(..)\n\n"
						 "ecm_img_upgrade unsigned ver ecram_sto.bin 192.168.17.10\n"
						 "-- Upgrade unsigned image.\tAPI tested: void UpgradeEcmImage(..)\n"
						 "ecm_img_upgrade unsigned crc ecram_sto.bin 192.168.17.10\n"
						 "-- Upgrade unsigned image.\tAPI tested: void UpgradeEcmImageByCrc(..)\n";
	AddCommand(command);

	// kTestEstbSwDloadStatus
    command.fCommandId = kTestEstbSwDloadStatus;
	command.fIsHidden = false;
    command.pfCommandName = "estb_dload_status";
	command.pfCommandParameterList = new BcmCommandParameterList;
	command.pfCommandParameterList->AddOrderDependent(new BcmUnsignedIntCommandParameter(kPrimaryParameter, true, "download_status"));
    command.pfCommandHelp = "estb_dload_status 1:inprogress 2: success 3: failed\n"
							"";
    command.pfExamples = "estb_dload_status	1";
    AddCommand(command);

	command.fCommandId = kSimCarouselDload;
	command.pfCommandName = "sim_carousel_dload";
	command.fIsHidden = true;
	command.pfCommandHelp = "";
	command.pfExamples = "";
	AddCommand(command);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmCommonDownloadCommandTable::~BcmCommonDownloadCommandTable()
{
    // Nothing to do...
}


// This is the entrypoint that is called whenever one of my commands is
// entered.
//
// Parameters:
//      pInstanceValue - the instance value to which the command should be
//                       applied.  Instances must be registered with the
//                       command table in order for them to receive the
//                       command.
//      command - the command (with parameters) that was entered by the user.
//
// Returns:  Nothing.
//
void BcmCommonDownloadCommandTable::CommandHandler(void *pInstanceValue, 
                                             const BcmCommand &command)
{
    const BcmCommandParameter *pParameter = NULL;
    YesNo answer = kNo;

    // By convention, the settings object registers its instance so that the
    // user value is the 'this' pointer.  This cast should be safe.
    BcmDsgClientCtlThread *pDsgClientCtlThread = (BcmDsgClientCtlThread *) pInstanceValue;

    // Validate this just for sanity.
    if (pDsgClientCtlThread == NULL)
    {
        gLogMessageRaw 
            << "BcmCommonDownloadCommandTable::CommandHandler:  ERROR - "
            << "User value (pointer to CM DSG Ctl Thread instance) is NULL!  Cant process the command!\n";

        return;
    }

    // Many commands below have a single primary parameter.  Retrieve it for
    // them.
    if (command.pfCommandParameterList != NULL)
    {
        pParameter = command.pfCommandParameterList->Find(kPrimaryParameter);
    }

	BcmIpAddress tftpServerIpAddress;
    switch (command.fCommandId)
    {
        case kMonolithicImg:
            {
              const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
              const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
			  const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);
			  const char* fileName = NULL;
                                        
             /* AUTHENTICATE */
              if (pParameter->AsUnsignedInt() == kAuthenticateOnly)
              {
                // Type: 
                if (pParameter2->WasParsed())
                {                
					if (pParameter2->AsUnsignedInt() == kSecureHeader)
					{
						gLogMessageRaw << " kCodeImageDigitalSignature"  << endl;
						const char* defaultFileName = "sec_hdr_ecram_sto.bin.3255";
						fileName = defaultFileName;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						// Open the file and read into the buffer.						
						FILE* pFile = fopen(fileName, "rb");
						if( pFile )
						{
							// Move the file pointer to the end of the file
							fseek(pFile, 0, SEEK_END);

							// Now get the size of the file.
							int size = ftell (pFile);
							if(size)
							{			
								bool result = false;
								BcmOctetBuffer *testBuffer;
								testBuffer = new BcmOctetBuffer(size);

								gLogMessageRaw << "Size = " << size << endl;
								result = ReadFile(*testBuffer, fileName);
								if(result)
								{
									uint16 headerSize ;

									CommonDownloadResponseMsg commonDownloadResponseMsg;
									memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
									AuthenticateMonolithicImageDigitalSignature( testBuffer->AccessBuffer(),  testBuffer->BytesUsed(), &commonDownloadResponseMsg);
									headerSize = commonDownloadResponseMsg.returnValue;

									if( headerSize  == 0 )
									{
										gLogMessageRaw
											<< "Failed to authenticate the digital signature." << endl;
									}
									else
									{
										gLogMessageRaw
												<< "Digital Signature is authenticated. Secure header size is: " << headerSize <<" bytes.\n"
												<< "eCM is now expecting to receive the content of the code image. (eSTB MUST call AuthenticateMonolithicImageSignedContent(..)) \n"
												<< "OR the SHA-digest of the code image (eSTB MUST call CommonDownloadVerifyCodeImageShaDigest(..))\n"
												<< "Use command: monolithic_img auth content <path to the monolith with the digital signature stripped> <tftp server IP> \n" 
												<< "OR: process sha_digest cvt " << endl;
									}
								}
								else
								{
									gLogMessageRaw
										<< "ReadFile failed!" << endl;
								}
							}
							else
							{
								gLogMessageRaw  << fileName << " size is zero." << endl;
							}
						}
						else
						{
							gLogMessageRaw << "Failed to open " << fileName << endl;
						}
					}
					else if(pParameter2->AsUnsignedInt() == kSignedContent)
					{					
						const char* defaultFileName = "ecram_sto.bin.3255";
						fileName = NULL;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						if (pParameter4->WasParsed())
						{
							tftpServerIpAddress = pParameter4->AsIpAddress();
							gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
						}
						
						uint8 tftpIpAddress[16];												
						tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
						uint32 ipAddrType = 0;
						if(tftpServerIpAddress.IsIpV6Address())
							ipAddrType = 6;
						else
							ipAddrType = 4;
						
						CommonDownloadResponseMsg commonDownloadResponseMsg;
						memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
						AuthenticateMonolithicImageSignedContent((uint8*)fileName, strlen(fileName), ipAddrType, tftpIpAddress, &commonDownloadResponseMsg);
						if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Content Authentication received by eCM.\n" 
																					   << "eCM will report the result to eSTB via a notification."<< endl;

					}
					else if(pParameter2->AsUnsignedInt() == kCodeImage)
					{					
						gLogMessageRaw << "Not supported. " << endl;
					/*	char* defaultFileName = "signed_ecram_sto.bin.3255";
						fileName = NULL;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						if (pParameter4->WasParsed())
						{
							tftpServerIpAddress = pParameter4->AsIpAddress();
							gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
						}
						
						uint32 tftpIpAddress = 0;
						tftpServerIpAddress.Get(tftpIpAddress);
						
						CommonDownloadResponseMsg commonDownloadResponseMsg;
						memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
						AuthenticateMonolithicImage((uint8*)fileName, strlen(fileName), tftpIpAddress, &commonDownloadResponseMsg);
						if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Image Authentication received by eCM.\n" 
																					   << "eCM will report the result to eSTB via a notification."<< endl;
																					   */

					}
					else if(pParameter2->AsUnsignedInt() == kSecureCvt)
					{
						const char* defaultFileName = "secure_cvt";

						gLogMessageRaw << "SecureCvt API not implemented." << endl;

					}
					else
					{
						// not recognized.
					}
				}
				else
				{
					// Not recognized.
				}
              }

			  /* UPGRADE SIGNED MONOLITHIC IMAGE */
			  else if(pParameter->AsUnsignedInt() == kSecureUpgrade)              
			  {
					const char* defaultFileName = "signed_ecram_sto.bin.3255";
					fileName = NULL;

					 // File path
					if (pParameter3->WasParsed())
					{                 
						fileName = pParameter3->AsString();												
					}
					else
					{							
						fileName = defaultFileName;
						gLogMessageRaw << "Using default file path " << defaultFileName << endl;
					}

					if (pParameter4->WasParsed())
					{
						tftpServerIpAddress = pParameter4->AsIpAddress();
						gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
					}

					uint8 tftpIpAddress[16];												
					tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
					uint32 ipAddrType = 0;
					if(tftpServerIpAddress.IsIpV6Address())
						ipAddrType = 6;
					else
						ipAddrType = 4;
					
					cout << "tftpServerIpAddress = " << tftpServerIpAddress << " tftpIpAddress = " << hex << tftpIpAddress[0] << endl;

					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));


					AuthenticateAndUpgradeMonolithicImage((uint8*)fileName, strlen(fileName), ipAddrType, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg);
					
					if(commonDownloadResponseMsg.returnCode == 0 ) gLogMessageRaw << "Request rejected by eCM.\n" << endl;
					else if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Image Upgrade received by eCM.\n" 
																					<< "eCM will report the result to eSTB via a notification."<< endl;
			  }

			  /* UPGRADE UNSIGNED MONOLITHIC IMAGE */
			  else if(pParameter->AsUnsignedInt() == kUnsecureUpgrade)
			  {
				  if(pParameter2->AsUnsignedInt() != kCodeImage)
				  {
					gLogMessageRaw << "Incorrect usage! Second parameter must be -image-."  << endl;
					return;
				  }
					const char* defaultFileName = "ecram_sto.bin.3255";
					fileName = NULL;

					 // File path
					if (pParameter3->WasParsed())
					{                 
						fileName = pParameter3->AsString();												
					}
					else
					{							
						fileName = defaultFileName;
						gLogMessageRaw << "Using default file path " << defaultFileName << endl;
					}

					if (pParameter4->WasParsed())
					{
						tftpServerIpAddress = pParameter4->AsIpAddress();
						gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
					}
					
					uint8 tftpIpAddress[16];												
					tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
					uint32 ipAddrType = 0;
					if(tftpServerIpAddress.IsIpV6Address())
						ipAddrType = 6;
					else
						ipAddrType = 4;
					
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
					UpgradeUnsignedMonolithicImage((uint8*)fileName, strlen(fileName), ipAddrType, tftpIpAddress, &commonDownloadResponseMsg);
					if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Unsigned Image Upgrade received by eCM.\n" 
																					<< "eCM will report the result to eSTB via a notification."<< endl;
					else
					{
						gLogMessageRaw << "Request rejected by eCM."<< endl;
					}
			  }		
              else 
              {
				gLogMessageRaw << "Command not recognized!"  << endl;
              }
            }
            break;

		////////////////////////////////////////////// ECM /////////////////////////////////////
       case kEcmImg:
            {
              const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
              const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
			  const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);
			  const char* fileName = NULL;
                                        
             /* AUTHENTICATE */
              if (pParameter->AsUnsignedInt() == kAuthenticateOnly)
              {
                // Type: 
                if (pParameter2->WasParsed())
                {                					
					if(pParameter2->AsUnsignedInt() == kCodeImage)
					{					
						const char* defaultFileName = "signed_ecram_sto.bin.3255";
						fileName = NULL;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						if (pParameter4->WasParsed())
						{
							tftpServerIpAddress = pParameter4->AsIpAddress();
							gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
						}
						
						uint8 tftpIpAddress[16];												
						tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
						uint32 ipAddrType = 0;
						if(tftpServerIpAddress.IsIpV6Address())
							ipAddrType = 6;
						else
							ipAddrType = 4;
						
						CommonDownloadResponseMsg commonDownloadResponseMsg;
						memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
						AuthenticateEcmImage((uint8*)fileName, strlen(fileName), ipAddrType, tftpIpAddress, &commonDownloadResponseMsg);
						if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Image Authentication received by eCM.\n" 
																					   << "eCM will report the result to eSTB via a notification."<< endl;
					}
				}
              }

			  /* UPGRADE SIGNED  IMAGE */
			  else if(pParameter->AsUnsignedInt() == kSecureUpgrade)              
			  {
					const char* defaultFileName = "signed_ecram_sto.bin.3255";
					fileName = NULL;

					 // File path
					if (pParameter3->WasParsed())
					{                 
						fileName = pParameter3->AsString();												
					}
					else
					{							
						fileName = defaultFileName;
						gLogMessageRaw << "Using default file path " << defaultFileName << endl;
					}

					if (pParameter4->WasParsed())
					{
						tftpServerIpAddress = pParameter4->AsIpAddress();
						gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
					}

					uint8 tftpIpAddress[16];												
					tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
					uint32 ipAddrType = 0;
					if(tftpServerIpAddress.IsIpV6Address())
						ipAddrType = 6;
					else
						ipAddrType = 4;
					
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));

					uint32 checkVersion = 0;
					AuthenticateAndUpgradeEcmImage((uint8*)fileName, strlen(fileName), ipAddrType, tftpIpAddress, &commonDownloadResponseMsg, checkVersion );
					if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Image Upgrade received by eCM.\n" 
																					<< "eCM will report the result to eSTB via a notification."<< endl;
			  }

			  /* UPGRADE UNSIGNED  IMAGE */
			  else if(pParameter->AsUnsignedInt() == kUnsecureUpgrade)
			  {
				  if(pParameter2->AsUnsignedInt() != kCodeImage)
				  {
					gLogMessageRaw << "Incorrect usage! Second parameter must be -image-."  << endl;
					return;
				  }
					const char* defaultFileName = "ecram_sto.bin.3255";
					fileName = NULL;

					 // File path
					if (pParameter3->WasParsed())
					{                 
						fileName = pParameter3->AsString();												
					}
					else
					{							
						fileName = defaultFileName;
						gLogMessageRaw << "Using default file path " << defaultFileName << endl;
					}

					if (pParameter4->WasParsed())
					{
						tftpServerIpAddress = pParameter4->AsIpAddress();
						gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
					}
					
					uint8 tftpIpAddress[16];												
					tftpServerIpAddress.Get(tftpIpAddress, sizeof(tftpIpAddress));
					uint32 ipAddrType = 0;
					if(tftpServerIpAddress.IsIpV6Address())
						ipAddrType = 6;
					else
						ipAddrType = 4;
					
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));

					uint32 checkVersion = 1;
					UpgradeUnsignedEcmImageByCrc((uint8*)fileName, strlen(fileName), ipAddrType, tftpIpAddress, &commonDownloadResponseMsg, checkVersion);
					if( commonDownloadResponseMsg.returnCode == 1 ) 
					{
						if(commonDownloadResponseMsg.returnValue == kEcmImageDownloadNotNeeded )
						{
							gLogMessageRaw << "eCM image version is the same. Upgrade not needed.\n"  << endl;

						}
						else
						{
							gLogMessageRaw << "Request for Unsigned Image Upgrade received by eCM.\n"
										   << "eCM will report the result to eSTB via a notification."<< endl;
						}
					}
					else
					{
						gLogMessageRaw << "Either the request rejected by eCM or failed to read the file version."<< endl;
					}
			  }
              else 
              {
				gLogMessageRaw << "Command not recognized!"  << endl;
              }
            }
            break;

		/////////////////////////////// CVT/ CVC PROCESSING ///////////////////////////////////////////

        case kProcess:
            {
              const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
              const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
			  const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);
			  const char* fileName = NULL;
                                        
             /* AUTHENTICATE */
              if (pParameter->AsUnsignedInt() == kAuthenticateOnly)
              {
                // Type: 
                if (pParameter2->WasParsed())
                {                
					if(pParameter2->AsUnsignedInt() == kSecureCvt)
					{
						gLogMessageRaw << " kAuthenticateCvt"  << endl;
						const char* defaultFileName = "sec_cvt.bin";
						fileName = defaultFileName;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						// Open the file and read into the buffer.						
						FILE* pFile = fopen(fileName, "rb");
						if( pFile )
						{
							// Move the file pointer to the end of the file
							fseek(pFile, 0, SEEK_END);

							// Now get the size of the file.
							int size = ftell (pFile);
							if(size)
							{			
								bool result = false;
								BcmOctetBuffer *testBuffer;
								testBuffer = new BcmOctetBuffer(size);

								gLogMessageRaw << "Size = " << size << endl;
								result = ReadFile(*testBuffer, fileName);
								if(result)
								{
									uint16 headerSize ;

									CommonDownloadResponseMsg commonDownloadResponseMsg;
									memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
									AuthenticateCvtFile( testBuffer->AccessBuffer(),  testBuffer->BytesUsed(), &commonDownloadResponseMsg);
									headerSize = commonDownloadResponseMsg.returnValue;

									if( headerSize  == 0 )
									{
										gLogMessageRaw
											<< "Failed to authenticate the CVT file." << endl;
									}
									else
									{
										gLogMessageRaw
												<< "CVT is authenticated.  Digital Signature length is: " << headerSize <<" bytes."<< endl;
									}
								}
								else
								{
									gLogMessageRaw
										<< "ReadFile failed!" << endl;
								}
							}
							else
							{
								gLogMessageRaw  << fileName << " size is zero." << endl;
							}
						}
						else
						{
							gLogMessageRaw << "Failed to open " << fileName << endl;
						}
					

					}
					else
					{
						// not recognized.
					}
				}
				else
				{
					gLogMessageRaw << "ERROR - required parameter ( sechdr|content|image|cvt|manuf|cosign) was not specified!"  << endl;
					return;				
				}
              }

			  /* PROCESS CVC */
			  else if(pParameter->AsUnsignedInt() == kProcessCvc)
			  {                         
                 // Type: Manuf or Co-signer ?
                if (pParameter2->WasParsed())
                {                 
					uint8 cvcType = pParameter2->AsUnsignedInt();
					if( (cvcType == kManuf) || (cvcType == kCosigner) )					
					{
						const char* defaultFileName = "BRCM_DOCSIS_cvc.brcm";
						fileName = defaultFileName;

						 // File path
						if (pParameter3->WasParsed())
						{                 
							fileName = pParameter3->AsString();												
						}
						else
						{							
							fileName = defaultFileName;
							gLogMessageRaw << "Using default file path " << defaultFileName << endl;
						}

						// Open the file and read into the buffer.
						
						FILE* pFile = fopen(fileName, "rb");
						if( pFile )
						{
							// Move the file pointer to the end of the file
							fseek(pFile, 0, SEEK_END);

							// Now get the size of the file.
							int size = ftell (pFile);
							if(size)
							{			
								bool result = false;
								BcmOctetBuffer *testBuffer;
								testBuffer = new BcmOctetBuffer(size);

								gLogMessageRaw << "Size = " << size << endl;
								result = ReadFile(*testBuffer, fileName);
								if(result)
								{
									if(cvcType == kManuf) 
										cvcType = kManufCvc;
									else if (cvcType == kCosigner) 
										cvcType = kCosignerCvc;

									CommonDownloadResponseMsg commonDownloadResponseMsg;
									memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
									result = CommonDownloadProcessCvc(testBuffer->AccessBuffer(), testBuffer->BytesUsed(), cvcType, &commonDownloadResponseMsg); 
									if( result )
									{
										gLogMessageRaw  << fileName << " is validated. CVC length is " << commonDownloadResponseMsg.returnValue << " bytes." << endl;
									}
									else
									{
										gLogMessageRaw  <<" Failed to validate " << fileName << endl;
									}
								}
								else
								{
									gLogMessageRaw
										<< "ReadFile failed!" << endl;
								}
							}
							else
							{
								gLogMessageRaw  << fileName << " size is zero." << endl;
							}
						}
						else
						{
							gLogMessageRaw << "Failed to open " << fileName << endl;
						}
					}
					else
					{
						gLogMessageRaw << "Cvc type not recognized. Please type manuf or cosigner!"  << endl;
					}
				}
				else
				{
					gLogMessageRaw << "ERROR - required parameter ( sechdr|content|image|cvt|manuf|cosign) was not specified!"  << endl;
					return;					
				}
			  }            
			  /* VERIFY CODE IMAGE SHA-digest */
			  else if(pParameter->AsUnsignedInt() == kVerifyCodeImageShaDigest)
			  {                         
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
					uint8 shaDigestBuffer[20] = { 0xed, 0x51, 0x7a, 0xeb, 0x27, 0x60, 0xc6, 0x1d, 0xc7, 0x35, 0x9a, 0x62, 0xb9, 0x02, 0xcf, 0x34, 0x60, 0xe6, 0x73, 0xbf};
					//{ 0x8b, 0x4d, 0x43, 0x93, 0x6e, 0x75, 0x04, 0x5c, 0x8e, 0x18, 0x62, 0x63, 0x25, 0xdd, 0x16, 0x75, 0xb1, 0x02, 0xe1, 0x90};								
					
					gLogMessageRaw << "ATTENTION - Using a canned SHA-digest for testing. \n"
						<<"shaDigestBuffer[20] = { 0xb1, 0x09, 0x73, 0xf5, 0x6a, 0x45, 0xd7, 0xf5, 0x05, 0x3e, 0x87, 0xe2, 0x35, 0x53, 0x22, 0x73, 0x99, 0x5d, 0x05, 0x70}\n"
						<<"Please note that you must replace this SHA-digest and rebuild to test.\n"
						<<"This command is added for testing purposes only."  << endl;

					bool result = CommonDownloadVerifyCodeImageShaDigest(shaDigestBuffer, &commonDownloadResponseMsg); 
					if( result )
					{
						gLogMessageRaw  << " SHA-digest is verified." << endl;
					}
					else
					{
						gLogMessageRaw  <<" Failed to validate SHA-digest" << endl;
					}				
			  }
              else 
              {
				gLogMessageRaw << "Command not recognized!"  << endl;
              }
            }
            break;

		case kSimCarouselDload:
			{
				// First digital signature
				{
					const char* fileName = "sec_hdr_ecram_sto.bin.3255";

					// Open the file and read into the buffer.						
					FILE* pFile = fopen(fileName, "rb");
					if( pFile )
					{
						// Move the file pointer to the end of the file
						fseek(pFile, 0, SEEK_END);

						// Now get the size of the file.
						int size = ftell (pFile);
						if(size)
						{			
							bool result = false;
							BcmOctetBuffer *testBuffer;
							testBuffer = new BcmOctetBuffer(size);

							gLogMessageRaw << "Reading the digital signature file " << fileName <<" into memory size = " << size << endl;
							result = ReadFile(*testBuffer, fileName);
							if(result)
							{
								uint16 headerSize ;
								CommonDownloadResponseMsg commonDownloadResponseMsg;
								memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
								AuthenticateMonolithicImageDigitalSignature( testBuffer->AccessBuffer(),  testBuffer->BytesUsed(), &commonDownloadResponseMsg);
								headerSize = commonDownloadResponseMsg.returnValue;

								if( headerSize  == 0 )
								{
									gLogMessageRaw
										<< "Failed to authenticate the digital signature." << endl;
								}
								else
								{
									gLogMessageRaw
											<< "Digital Signature is authenticated. Secure header size is: " << headerSize <<" bytes."<< endl;

									// Tftp the content part of the monolith. Normally at this point digital signature would be stripped of 
									// the first block of the monolith and monolith would be downloaded from the carousel and stored to the temporary storage.
									system("rm monolith.temp");
									system("ln -s mtd_block_2 monolith.temp");

									gLogMessageRaw<< "Start TFTP of the monolith content (simulating the download from the carousel)..."<< endl;
									system("tftp -g -r ecram_sto.bin.3255 -l monolith.temp 10.24.64.80");
									
									//sleep(5);
									gLogMessageRaw<< "Monolith is downloaded."<< endl;

													
										//char* defaultFileName = "ecram_sto.bin.3255";
										fileName = "monolith.temp"; //NULL;

										 // File path
										/*if (pParameter3->WasParsed())
										{                 
											fileName = pParameter3->AsString();												
										}
										else
										{							
											fileName = defaultFileName;
											gLogMessageRaw << "Using default file path " << defaultFileName << endl;
										}

										if (pParameter4->WasParsed())
										{
											tftpServerIpAddress = pParameter4->AsIpAddress();
											gLogMessageRaw << "pParameter4->WasParsed " << tftpServerIpAddress << endl;
										}*/
										tftpServerIpAddress.Set(10,24,6,227);
										uint32 tftpIpAddress = 0;
										tftpServerIpAddress.Get(tftpIpAddress);
										
										system("tftpdemon 69 &");
										//CommonDownloadResponseMsg commonDownloadResponseMsg;
										memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
										AuthenticateMonolithicImageSignedContent((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg);
										if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Content Authentication received by eCM.\n" 
																									   << "eCM will report the result to eSTB via a notification."<< endl;

									

									sleep(10);

									// Assume notification is received.
									system("rm ecm.temp");
									system("ln -s mtd_block_4 ecm.temp");

									gLogMessageRaw<< "Start TFTP of the ecm image (simulating extraction of ecm image from monolith)..."<< endl;
									system("tftp -g -r signed_ecram_sto.bin.3349 -l ecm.temp 10.24.64.80");

									gLogMessageRaw<< "eCM image extracted. Ask eCM to authenticate its image."<< endl;
									fileName = "ecm.temp"; 
									memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
									AuthenticateEcmImage((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg);
									if( commonDownloadResponseMsg.returnCode == 1 ) gLogMessageRaw << "Request for Signed Image Authentication received by eCM.\n" 
																								   << "eCM will report the result to eSTB via a notification."<< endl;

								}
							}
							else
							{
								gLogMessageRaw
									<< "ReadFile failed!" << endl;
							}
						}
						else
						{
							gLogMessageRaw  << fileName << " size is zero." << endl;
						}
					}
					else
					{
						gLogMessageRaw << "Failed to open " << fileName << endl;
					}
				}
			}
			break;

		case kSetImageName:
			{
				const char* fileName = NULL;								

				 // File path
				if (pParameter->WasParsed())
				{        
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
					
					fileName = pParameter->AsString();	
					
					SetRunningMonolithicImageName((uint8*)fileName, strlen(fileName), &commonDownloadResponseMsg);
					gLogMessageRaw << "commonDownloadResponseMsg.returnValue is " << commonDownloadResponseMsg.returnValue << endl;
				}
			}
			break;

		case kSetDocsDevSwFileName:
			{
				const char* fileName = NULL;								

				 // File path
				if (pParameter->WasParsed())
				{        
					CommonDownloadResponseMsg commonDownloadResponseMsg;
					memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
					
					fileName = pParameter->AsString();	
					
					if( SetDocsDevSwFilename((uint8*)fileName, strlen(fileName), &commonDownloadResponseMsg) == 1)
						gLogMessageRaw << "DocsDevSwFilename is set to " << fileName <<" returnValue is " << commonDownloadResponseMsg.returnValue << endl;
				}
			}
			break;

		case kLocalEcmImgVersion:
			{
				uint16 majorVersion;
				uint16 minorVersion;
				uint16 subMinorVersion;
				uint8 upgradeNeeded = 0;
				uint32 imageIsSigned = 0;

				const char* fileName = NULL;							

				// Retrieve eCM image version from local filesystem
				const char *fnPtr = "ecram_sto.bin";

				 // File path
				if (pParameter->WasParsed())
				{        									
					fileName = pParameter->AsString();	

					if(fileName)
						fnPtr = fileName;
				}

				gLogMessageRaw <<" Signed image version check not supported yet!" << endl;
				//GetLocalEcmImageVersion(fnPtr, &majorVersion, &minorVersion, &subMinorVersion, 0);
				CheckForEcmUpgrade(fnPtr, &upgradeNeeded, imageIsSigned);
								
				gLogMessageRaw <<" upgradeNeeded = " << (upgradeNeeded?"YES":"NO") << endl;
			}
			break;

		case kLocalEcmImgCrc:
			{
				uint8 upgradeNeeded = 0;
				uint32 imageIsSigned = 0;

				const char* fileName = NULL;							

				// Retrieve eCM image version from local filesystem
				const char *fnPtr = "ecram_sto.bin";

				 // File path
				if (pParameter->WasParsed())
				{        									
					fileName = pParameter->AsString();	

					if(fileName)
						fnPtr = fileName;
				}

				gLogMessageRaw <<" Signed image version check not supported yet!" << endl;
				CheckForEcmUpgradeByCrc(fnPtr, &upgradeNeeded, imageIsSigned);
								
				gLogMessageRaw <<" upgradeNeeded = " << (upgradeNeeded?"YES":"NO") << endl;
			}
			break;

		case kEcmImgUpgrade:
			{
				const BcmCommandParameter *pParameter2 = command.pfCommandParameterList->Find(kSecondParameter);
				const BcmCommandParameter *pParameter3 = command.pfCommandParameterList->Find(kThirdParameter);
				const BcmCommandParameter *pParameter4 = command.pfCommandParameterList->Find(kFourthParameter);
				CommonDownloadResponseMsg commonDownloadResponseMsg;
				unsigned int imageType;
				unsigned int checkBy;
				const char* fileName = NULL;
				uint32 tftpIpAddress = 0;

				/* check first parameter: image type */
				if (pParameter->WasParsed())
				{
					imageType = pParameter->AsUnsignedInt();
				} 
				else
				{
					gLogMessageRaw << "Specify image type: signed|unsigned" << endl;
					return;
				}

				/* check second parameter: image checking method */
				if (pParameter2->WasParsed())
				{
					checkBy = pParameter2->AsUnsignedInt();
				}
				else
				{
					gLogMessageRaw << "Specify image checking method: ver|crc" << endl;
					return;
				}
					
				/* check third parameter: file name */
				if (pParameter3->WasParsed())
				{				   
					fileName = pParameter3->AsString();											 
				}
				else
				{							 
					gLogMessageRaw << "Specify image file name" << endl;
					return;
				}

				/* check fourth parameter: server IP address */
				if (pParameter4->WasParsed())
				{
					tftpServerIpAddress = pParameter4->AsIpAddress();
					tftpServerIpAddress.Get(tftpIpAddress);
					tftpIpAddress = htonl(tftpIpAddress);
				}
				else
				{
					gLogMessageRaw << "Specify Server IP address" << endl;
					return;
				}

				/* upgrade image */
				memset(&commonDownloadResponseMsg, 0, sizeof(CommonDownloadResponseMsg));
				if(imageType == kSignedImage) /* signed image */		   
				{
					if (checkBy == kCheckByVersion)
					{
						AuthenticateAndUpgradeEcmImage((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg, true);
					}
					else if (checkBy == kCheckByCrc)  /* kCheckByCrc */
					{
						AuthenticateAndUpgradeEcmImageByCrc((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg, true);
					}
					else
					{
						gLogMessageRaw << "Unknown image checking method" << endl;
						return;
					}

					if (commonDownloadResponseMsg.returnCode == 1) 
					{
						gLogMessageRaw << "Request for Signed Image Upgrade received by eCM.\n";
					}
				}
				else if (imageType == kUnsignedImage)
				{
					if (checkBy == kCheckByVersion)
					{
						UpgradeUnsignedEcmImage((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg, true);
					}
					else if (checkBy == kCheckByCrc)  /* kCheckByCrc */
					{
						UpgradeUnsignedEcmImageByCrc((uint8*)fileName, strlen(fileName), 4, (uint8*) &tftpIpAddress, &commonDownloadResponseMsg, true);
					}
					else
					{
						gLogMessageRaw << "Unknown image checking method" << endl;
						return;
					}

					if (commonDownloadResponseMsg.returnCode == 1) 
					{
						gLogMessageRaw << "Request for Unsigned Image Upgrade received by eCM.\n";
					}
				}
				else
				{
					gLogMessageRaw << "Unknown image type" << endl;
					return;
				}
			}
			break;

        case kTestEstbSwDloadStatus:
            {
				uint32 dloadStatus = 0;
				
				if (pParameter->WasParsed())
				{
					dloadStatus = pParameter->AsUnsignedInt();
				}

				CommonDownloadResponseMsg commonDownloadResponseMsg;
				BcmCdlUpdateSnmpMibs(dloadStatus, &commonDownloadResponseMsg);

				if (commonDownloadResponseMsg.returnCode == 1) 
				{
					gLogMessageRaw << "Request for Unsigned Image Upgrade received by eCM. commonDownloadResponseMsg.returnValue = " << commonDownloadResponseMsg.returnValue << endl;
				}
				
			}
			break;

        default:
            gLogMessageRaw << "WARNING - unknown command id (" << command.fCommandId << ")!  Ignoring...\n";
            break;
    }

}


// This is where the derived class reads data from the device into the
// buffer.
//
// Parameters:
//      inputBuffer - the buffer into which the data should be read.
//      filename    - the filename that should be read.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually hardware failure).
//
bool BcmCommonDownloadCommandTable::ReadFile(BcmOctetBuffer &inputBuffer,
                                       const char *filename)
{
    FILE *pFile;
    bool result = true;
    size_t bytesRead;

    // Quick sanity check.
    if (filename == NULL)
    {
        //gErrorMsg(fMessageLogSettings, "ReadFile") 
		gLogMessageRaw
            << "Filename string is NULL!" << endl;

        return false;
    }

    // Open the file for reading in binary mode.  If it doesn't exist, then
    // this should fail.
    pFile = fopen(filename, "rb");

    if (pFile == NULL)
    {
        //gErrorMsg(fMessageLogSettings, "ReadFile") 
		gLogMessageRaw
            << "Failed to open file '" << filename << "' for reading!" << endl;

        return false;
    }

	//go to the begining of the file
	fseek(pFile, 0, SEEK_SET);

    // Read the buffer from the file.
    bytesRead = fread(inputBuffer.AccessBuffer(), 1, inputBuffer.BufferSize(), pFile);
	gLogMessageRaw
		<< "ReadFile - Read " << bytesRead << " bytes from " << filename << endl;
#if 0
    if (!feof(pFile))
    {
        gErrorMsg(fMessageLogSettings, "ReadFile") 
            << "Only read " << bytesRead << " bytes. Buffer [ " << inputBuffer.BufferSize() <<" ] too small to hold the entire file!" << endl;

        result = false;
    }
#endif
    // Finally, make the buffer think that the correct number of bytes have been
    // consumed.
    inputBuffer.SetBytesUsed(bytesRead);

    // Close the file. 
    fclose(pFile);

    return result;
}
