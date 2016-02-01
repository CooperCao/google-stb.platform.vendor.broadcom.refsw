//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  Filename:       EstbAppVersionInfo.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
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
void gGetEstbAppVersionInfo(BcmCompanyLogo &logo);

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


