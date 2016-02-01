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
//****************************************************************************

#ifndef CUSTOMERNONVOL_BCM97110DSG_H
#define CUSTOMERNONVOL_BCM97110DSG_H

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

// Unsigned integer value from 2-4.  Other values are reserved and must not
// be used.  Typically, you will want to set this to 2.  This controls the
// stack affinity in cases where multiple stacks have the same subnet, and
// tells the CPE Access agent which stack to initialize with the IP address
// and subnet specified above.  For the EMTA/Packet Cable build, this needs to
// be 3, since the EMTA uses stack 2. For settop, it is also 3 to allow stack2 
// to be the internal CPE device
#define kDefaultValue_RemoteAccessIpStackNumber 3

// +-----------------------+
// | HalIf NonVol Settings |
// +-----------------------+
 
// These set the interfaces that are enabled or disabled by default.  At a
// minimum, the DocsisCm, IP1 and IP2 interfaces must be enabled.  Others are
// optional.  These are all bool values.
// BCM_DSG_SUPPORT: No Ethernet interface
#define kDefaultValue_EthernetEnabled    false
#define kDefaultValue_UsbEnabled         false
#define kDefaultValue_IpStack3Enabled    true

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

//********************** Inline Method Implementations ***********************


#endif


