//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       CustomerNonvolDefaults.h
//  Author:         David Pullen
//  Creation Date:  March 25, 2002
//
//****************************************************************************
//  Description:
//      This file contains the default values that are to be used for nonvol
//      settings.  This file controls the behavior of the settings (hard-coded
//      vs use nonvol) as well as specifying the default values.
//
//      This file needs to be modified for each customer so that it reflects
//      their unique settings, and then the entire code set needs to be rebuilt
//      so that they take effect.
//
//      The setting here superseed the setting CustomerNonVolDefaults.h
//
//****************************************************************************

#ifndef CUSTOMERNONVOL_BCM97110DSG2_H
#define CUSTOMERNONVOL_BCM97110DSG2_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// +---------------------------+
// | CM DOCSIS NonVol Settings |
// +---------------------------+
// 
// The customer can taylor the product to be DOCSIS 1.0 only or DOCSIS 1.1,
// depending on how the product is being submitted for certification.
// 
// This is a bool true/false value.  If kDefaultValue_Docsis11Support is false
// (meaning the CM is to operate in pure DOCSIS 1.0 mode), then this determines
// whether or not IGMP will be supported.
// Set to true for the Ip video demo to run correctly on a Docsis 1.0 HE.
#define kDefaultValue_Docsis10IgmpSupport true


// +-----------------------+
// | HalIf NonVol Settings |
// +-----------------------+
 
// These set the interfaces that are enabled or disabled by default.  At a
// minimum, the DocsisCm, IP1 and IP2 interfaces must be enabled.  Others are
// optional.  These are all bool values.
// BCM_DSG_SUPPORT: No Ethernet interface
#define kDefaultValue_EthernetEnabled    true
#define kDefaultValue_UsbEnabled         false
#define kDefaultValue_IpStack3Enabled    true


// For the 7111 HW using a 3419 tuner, we want to specify the new method for
// getting the default DPM cal tables.  The presence of this value will cause
// DocsisCmDownstreamCalibrationNonVolSettings.cpp to include the filename
// specified instead of the old-school header file.
#define kDefaultValue_DpmDefaultsFile "DpmDefaults_3419.h"

#define kDefaultTuner_BCM3419

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************


#endif


