//****************************************************************************
//
// Copyright (c) 2004-2010 Broadcom Corporation
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
//  Filename:       CmAppVersionInfo.cpp
//  Author:         Mike Sieweke
//  Creation Date:  Aug 16, 2001
//
//****************************************************************************
//  Description:
//      This file includes functions to return info about this version
//      of the code.  It is rebuilt every time to include the build date
//      and time, so it should be kept very small and quick to compile.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "CmAppVersionInfo.h"
#include "kversion.h"
#include <string.h>

// Define the username of the person building.
#include "username.h"

#if !defined(WIN32)
#include "MergedNonVolDefaults.h"
#endif

//********************** Local Constants *************************************

#define kDsgCCBanner \
"+----------------------------------------------------------------------------+\n"\
"        _/_/_/     _/_/_/   _/_/            _/_/       _/_/ \n" \
"       _/    _/  _/       _/    _/        _/    _/   _/    _/\n"\
"      _/     _/ _/       _/        ____  _/         _/\n"\
"     _/     _/   _/_/   _/  _/_/  /___/ _/         _/\n"\
"    _/     _/       _/ _/    _/        _/         _/\n"\
"   _/    _/        _/ _/    _/        _/    _/   _/     _/\n"\
"  _/_/__/    _/_/_/    _/_/            _/_/       _/_/\n"\
"+----------------------------------------------------------------------------+\n"\
"\n"

#define kRpsBanner \
"+----------------------------------------------------------------------------+\n"\
" REMOTE PROGRAM STORE HOST APPLICATION										   \n"\
"+----------------------------------------------------------------------------+\n"\

#define kCdlBanner \
"+----------------------------------------------------------------------------+\n"\
" COMMON DOWNLOAD API - OpenCable Common Download 2.0 compatible			   \n"\
"+----------------------------------------------------------------------------+\n"\

#define VERSION_STRING "Broadcom DSG-CC Reference Design"

#define RELEASE_STATE "$State: Release $"

#if defined(TARGETOS_vxWorks)
#define OS_STRING "vxWorks"
#elif defined(PSOS)
#define OS_STRING "pSOS"
#elif defined(WIN32)
#define OS_STRING "Win32"
#elif defined(__linux__)
#define OS_STRING "Linux"
#else
#define OS_STRING "Undefined OS"
#endif

#if (BCM_REDUCED_IMAGE_SIZE)
#define IMAGE_SIZE_STRING " slim"
#else
#define IMAGE_SIZE_STRING ""
#endif

#if (BCM_VENDOR_SUPPORT)
// DPullen - include the vendor name string along with the vendor build.
#define VENDOR_STRING "Vendor:" kDefaultValue_VendorName " "
#else
#define VENDOR_STRING ""
#endif

#if (BCM_DSG_SUPPORT)
#define DSG_STRING "DSG "
#else
#define DSG_STRING ""
#endif

// This is the feature set string that will be displayed.
#define FEATURE_STRING " "  VENDOR_STRING DSG_STRING

//********************** Function definitions ********************************

// Stores the version information.
//
// Parameters:  logo - The BcmCompanyLogo object in which to place the
//                     version information.
//
// Returns:  Nothing.
//
void gGetCmAppVersionInfo(BcmCompanyLogo &logo)
{
/*
    static char versionString[64];
    static char featureString[256];
    char        signatureString[6];
*/
    // Set the build date/time to the date and time that this module is
    // built.  I have contrived the build tools to rebuild this module every
    // time.
    logo.SetBuildDateTime(__DATE__, __TIME__);

    // Set the username of the person building the app.
    logo.SetUsername( USERNAME );
/*
    strcpy(versionString, VERSION_STRING);
    if (strstr(RELEASE_STATE, "Rel") != NULL)
    {
        strcat(versionString, " RELEASE");
    }
    else
    {
        strcat(versionString, " PRE-RELEASE");
    }
    strcat( versionString, " " );
    strcat( versionString, OS_STRING);

    strcat( versionString, IMAGE_SIZE_STRING);

    sprintf( signatureString, "%04X", BootloaderStoreDriverSignature());
    strcat( versionString, " PID=0x");
    strcat( versionString, signatureString);

    // PR2919 Docsis 1.1 support is now a compiled-in option, so let the banner
    // reflect that.  I'm not happy with this bit of code, but it can't be done
    // as a #if because the preprocessor doesn't know the difference between
    // "true" and "false".
    strcpy(featureString, "DOCSIS 1.0");
    if (kDefaultValue_Docsis11Support == true)
    {
        strcat(featureString, "/1.1");

        if (kDefaultValue_AdvancedPhySupport == true)
        {
            strcat(featureString, "/2.0");
        }
    }

    if (kDefaultValue_Docsis1xCmtsHackForDocsis20 == true)
    {
        strcat(featureString, "/2.0hack");
    }

    strcat( featureString, FEATURE_STRING );
*/
    // Set the revision and feature code strings.
    logo.SetRevisionString( gGetVersionString(), gGetFeaturesString() );
}

// This returns a pointer to the string containing our version number, which
// is suitable for SNMP and other display.
//
// Parameters:  None.
//
// Returns:
//      The version string.
//
const char *gGetVersionString(void)
{
    static char versionString[64];
//    char        signatureString[6];

    strcpy(versionString, VERSION_STRING);
    strcat( versionString, " " );
    strcat(versionString, kVersion);
    strcat( versionString, " for " );
    strcat( versionString, OS_STRING);

    strcat( versionString, IMAGE_SIZE_STRING);

//    sprintf( signatureString, "%04X", BootloaderStoreDriverSignature());
//    strcat( versionString, " PID=0x");
//    strcat( versionString, signatureString);

    return versionString;
}


const char *gGetDateTimeString(void)
{
  static char DateTime[32];
  sprintf (DateTime, "%s %s", __DATE__, __TIME__);
  return DateTime;
}

const char *gGetBuiltByString(void)
{
  return USERNAME;
}

const char *gGetFeaturesString(void)
{
  static char featureString[256];

  strcpy(featureString, "DSG-I10");
/*
  if (kDefaultValue_Docsis11Support == true)
  {
      strcat(featureString, "/1.1");

      if (kDefaultValue_AdvancedPhySupport == true)
      {
          strcat(featureString, "/2.0");
      }
  }

  if (kDefaultValue_Docsis1xCmtsHackForDocsis20 == true)
  {
      strcat(featureString, "/2.0hack");
  }
  */
  strcat( featureString, FEATURE_STRING );

  // put SNMP version and agent at the end of the list.
//  strcat (featureString, SNMP_STRING);

/*#if (SNMP_SUPPORT)
  strcat (featureString, "(");
  strcat (featureString, AgentGetName());
  strcat (featureString, ") ");
#endif  */
  return featureString;
}

void PrintStbBanner(void)
{
	#if defined (ESTB_RSWDLOAD_SUPPORT)
	cout << kRpsBanner << endl;
	#endif

	#if defined (COMMON_DOWNLOAD_SUPPORT)
	cout << kCdlBanner << endl;
	#endif

	cout << kDsgCCBanner << endl;
}
