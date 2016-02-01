//****************************************************************************
//
//  Copyright (c) 2000, 2001  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       CmAppVersionInfo.cpp
//  Author:         Mike Sieweke
//  Creation Date:  Aug 16, 2001
//
//****************************************************************************
//  Description:
//      This file defines functions to return info about this version
//      of the code.  It is rebuilt every time to include the build date
//      and time, so it should be kept very small and quick to compile.
//
//****************************************************************************

//********************** Include Files ***************************************

#include "CompanyLogo.h"

//********************** Function definitions ********************************

// Prints the company logo and version information.
//
// Parameters:  logo - The BcmCompanyLogo object in which to place the
//                     version information..
//
// Returns:  Nothing.
//
void gGetCmAppVersionInfo(BcmCompanyLogo &logo);

// This returns a pointer to the string containing our version number, which
// is suitable for SNMP and other display.
//
// Parameters:  None.
//
// Returns:
//      The version string.
//
const char *gGetVersionString(void);

const char *gGetDateTimeString(void);

const char *gGetBuiltByString(void);

const char *gGetFeaturesString(void);

void PrintStbBanner(void);

