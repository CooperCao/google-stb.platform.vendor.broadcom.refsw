//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//  Filename:       EstbAppVersionInfo.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This file includes functions to return info about this version
//      of the code.  It is rebuilt every time to include the build date
//      and time, so it should be kept very small and quick to compile.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "EstbAppVersionInfo.h"
#include "kversion.h"
#include <string.h>

// Define the username of the person building.
#include "username.h"

#include "MergedNonVolDefaults.h"
// #include "BootloaderStoreDeviceDriver.h"

//********************** Local Constants *************************************


#define RELEASE_STATE "$State: Release $"

#if (SNMPv3)
#define SNMP_STRING "SNMPv3 "
#elif (SNMPv2)
#define SNMP_STRING "SNMPv2 "
#elif (SNMPv1)
#define SNMP_STRING "SNMPv1 "
#else
#define SNMP_STRING "(no SNMP) "
#endif

#if (SNMP_SUPPORT)
#include "SnmpApi.h"
#endif

#if (BCM_VENDOR_SUPPORT)
// DPullen - include the vendor name string along with the vendor build.
#define VENDOR_STRING "Vendor:" kDefaultValue_VendorName " "
#else
#define VENDOR_STRING ""
#endif

// This is the feature set string that will be displayed.

#define FEATURE_STRING "Broadcom eSTB SNMP AGENT V2 Reference Design, "
       
//********************** Function definitions ********************************
#if !defined(CORE_ESTBSNMP_BUILT_AS_LIB)
// Stores the version information.
//
// Parameters:  logo - The BcmCompanyLogo object in which to place the
//                     version information.
//
// Returns:  Nothing.
//
void gGetEstbAppVersionInfo(BcmCompanyLogo &logo)
{
    // Set the build date/time to the date and time that this module is
    // built.  I have contrived the build tools to rebuild this module every
    // time.
    logo.SetBuildDateTime(__DATE__, __TIME__);

    // Set the username of the person building the app.
    logo.SetUsername( USERNAME );

    // Set the revision and feature code strings.
    logo.SetRevisionString( gGetVersionString(), gGetFeaturesString() );
}
#endif

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

    strcpy(versionString, kVersion);
    if (strstr(RELEASE_STATE, "Rel") != NULL)
    {
        strcat(versionString, " RELEASE");
    }
    else
    {
        strcat(versionString, " PRE-RELEASE");
    }
    strcat( versionString, " " );

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

  strcpy( featureString, FEATURE_STRING );

  // add SNMP version and agent.
  // strcat (featureString, SNMP_STRING);

  strcat (featureString, "(");
  strcat (featureString, AgentGetName());
  strcat (featureString, ") ");

  // add vendor, if any.
  strcat (featureString, VENDOR_STRING);

  return featureString;
}
