//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//  $Id$
//
//  Filename:       ocStbHostTrap.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  14 JUN 2010
//		
//**************************************************************************
//
//    Description:
//      Traps associated with the ocStbHost MIB
//
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#include "ocStbHostTrap.h"
#include "ocStbHostMib.h"

/*
ocStbPanicDumpTrap NOTIFICATION-TYPEOBJECTS {
ocStbHostDumpFilePath

}
STATUS current 
DESCRIPTION 


"Notifies host that a panic dump condition has occurred.
The headend then may issue a TFTP Get to retrieve thedump or ignore the trap and allow the hostocStbHostDumpEventTimeout timer to expire. This trap issent only when the ocStbHostDumpEventCount object has anon-zero value." 

::= { ocStbHostNotifications 1 } 
*/
ocStbPanicDumpTrap::ocStbPanicDumpTrap (BcmSnmpAgent *pAgent)
  : BcmTrap (BcmObjectId ("1.3.6.1.4.1.4491.2.3.1.0.1"), BcmString ("ocStbPanicDumpTrap"), pAgent, TRAP_ID_ENTERPRISE)
{
  // for testing BcmObjectId *tempOID = new BcmObjectId("1.3.6.1.2.1.1.1.0");
  BcmObjectId *tempOID = new BcmObjectId(kOID_ocStbHostDumpFilePath);
  AddObject (*tempOID);
}

ocStbPanicDumpTrap::~ocStbPanicDumpTrap ()
{
}

