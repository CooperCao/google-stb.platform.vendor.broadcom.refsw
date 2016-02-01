//****************************************************************************
//
//  Copyright (c) 2008  Broadcom Corporation
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
//  $Id$
//
//  Filename:       DhcpV6SocketIf.cpp
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  April 29, 2008
//
//****************************************************************************
// 
//
//****************************************************************************

#include "DhcpV6SocketIf.h"
#include "EthernetPacket.h"

// Initializing Constructor.  Stores the information specified.
//
// Parameters:
//      pWrappedHal - if the design is using a real physical medium
//                    (ie Ethernet, USB, etc) to transmit the data, a 
//                    pointer to that HAL.  If it's a pure software
//                    design, then this will be NULL.
//
// Returns:  N/A
//
BcmDhcpV6SocketIf::BcmDhcpV6SocketIf (BcmHalIf *pWrappedHal)
  : BcmStubHalIf (118, // interface number - arbitrary.  Use 118 since we have to be ifIndex 18 (why not).
                  BcmHalIf::kIanaTypeOpenCableDsgClientInterface, // ifType, internal value, really return '0' per eDOCSIS section 5.2.3.1, table 5-2
                  "Set-Top Box Embedded DSG Interface",// ifDescr, per eDOCSIS section 5.2.3.1, table 5-2
                  0, // ifSpeed, per eDOCSIS section 5.2.3.1, table 5-2
                  0, // ifMtu, per eDOCSIS section 5.2.3.1, table 5-2
                  pWrappedHal, // Wrapped HAL, if any
                  BcmMacAddress(0,0,0,0,0,0))// ifPhysAddress, per eDOCSIS section 5.2.3.1, table 5-2)
{
}

// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDhcpV6SocketIf::~BcmDhcpV6SocketIf()
{
}

// This method is called by the base class after it has done initial
// processing of the packet that is being released.  If the base class
// calls this, then the packet really needs to be released.  This must be
// provided by the derived classes.
//
// Parameters:
//      pPacket - the packet to be released.
//
// Returns:
//      true if the packet was released.
//      false if the packet was not released.
//
bool BcmDhcpV6SocketIf::ReleasePacketImpl(BcmEthernetPacket *pPacket)
{
  return false;
}

