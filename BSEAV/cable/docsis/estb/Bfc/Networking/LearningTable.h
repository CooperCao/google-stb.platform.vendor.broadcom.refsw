//****************************************************************************
//
// Copyright (c) 2000-2010 Broadcom Corporation
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
//  $Id: LearningTable.h 1.7 2007/11/07 21:34:04Z koneal Release $
//
//  Filename:       LearningTable.h
//  Author:         David Pullen
//  Creation Date:  March 21, 2000
//
//****************************************************************************
//  Description:
//      This is a simple wrapper class for the map that is used as the learning
//      table.  It's only purpose is to avoid including map.h in the header
//      file for BcmPacketSwitch, which causes lots of warnings to be displayed
//      in the pSOS compiler.
//
//      Previously, I had declared the map directly in PacketSwitch.h, but this
//      was problematic.
//
//      This class doesn't provide any helper functions.  It's simply a
//      workaround for some compiler wackiness.
//
//****************************************************************************

#ifndef LEARNINGTABLE_H
#define LEARNINGTABLE_H

//********************** Include Files ***************************************

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <map>
#include <list>
#else
#include <map.h>
#include <list.h>
#endif

#include "MacAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declaration *********************************

class BcmHalIf;

//********************** Class Declaration ***********************************


class BcmLearningTable
{
public:

    // Default Constructor.  Does nothing.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmLearningTable(void) {}

    // Destructor.  Does nothing.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmLearningTable() {}
    
    // Map iterator typedef just to save some typing...
    typedef map <BcmMacAddress, BcmHalIf *, less<BcmMacAddress> >::iterator Iterator;
    
    // This is the map that forms the basis of the learning table.
    map<BcmMacAddress, BcmHalIf *, less<BcmMacAddress> > fLearningTable;

    // This is a list of MAC addresses whose associations are static,
    // meaning they won't be re-associated if found on another interface.
    list<BcmMacAddress> fStaticAssociations;
    
protected:

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmLearningTable(const BcmLearningTable &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmLearningTable & operator = (const BcmLearningTable &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


