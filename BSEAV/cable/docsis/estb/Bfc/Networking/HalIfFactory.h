//****************************************************************************
//
// Copyright (c) 2007 Broadcom Corporation
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
//  $Id: HalIfFactory.h 1.6 2007/10/17 23:55:36Z czacker-atl Release $
//
//  Filename:       HalIfFactory.h
//  Author:         David Pullen
//  Creation Date:  October 18, 2002
//
//****************************************************************************

#ifndef HalIfFactory_H
#define HalIfFactory_H

//********************** Include Files ***************************************

#include "typedefs.h"
#include "HalIf.h"
#include "Snoop.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

// This would normally be defined somewhere else, but it doesn't seem to be
// supported in all operating systems, so we define it here.
#define BCM_NUM_IP_STACKS 8

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmHalIf;
class BcmHalIfList;

//********************** Class Declaration ***********************************


/** \ingroup BfcNetworking
*
*   This is a singleton class that is used to create all of the HalIf
*   objects (and register or start associated drivers as needed).  The
*   HalIfs are enabled via nonvol, and are only created if enabled.
*
*   This class also serves as the HalIf manager, a container class that
*   stores the objects after they are created, and provides a hook to let
*   a client get a pointer to the HalIf based on the standard short name.
*/
class BcmHalIfFactory
{
public:

    /// Returns the singleton HalIf factory/manager object.
    ///
    /// PR6829 - This no longer causes drivers to be loaded as a side-effect of
    /// accessing this object the first time.  You must call Initialize() to do
    /// this.
    ///
    /// \return
    ///      A reference to the object.
    ///
    static BcmHalIfFactory *GetSingleton(void);

    /// Returns the HalIf object corresponding to the specified short name.
    /// If the HalIf with the specified name doesn't exist, then it returns
    /// NULL.
    ///
    /// \param
    ///      shortName - the short name for the interface.  There are well-known
    ///                  values for interfaces defined in HalIf.h.
    ///
    /// \return
    ///      The object with the specified short name, or NULL if not found.
    ///
    BcmHalIf *Find(uint32 shortName);

    BcmHalIf *FindByIfIndex(int ifIndex);

    /// Supports iteration over the HalIfs contained by this object.  This method
    /// returns the first HalIf in the container.
    ///
    /// \return
    ///      The first HalIf in the container, or NULL if there are none.
    ///
    BcmHalIf *First(void);

    /// Supports iteration over the HalIfs contained by this object.  This method
    /// returns the next HalIf in the container after the one specified.
    ///
    /// \param
    ///      pHalIf - the HalIf to use as the base; the next one after this one
    ///               is returned.
    ///
    /// \return
    ///      The next HalIf in the container after the one specified, or NULL if
    ///      there is no next HalIf (or if pHalIf was not found).
    ///
    BcmHalIf *Next(BcmHalIf *pHalIf);

    /// Allows an external agent to create a BcmHalIf instance and register it
    /// with me.  Normally, I create all of the interface objects in
    /// Initialize(), but there may be product- or platform-specific interfaces
    /// that are more appropriately created and configured elsewhere.
    ///
    /// \param
    ///     pHalIf - pointer to the interface to be registered.
    ///
    void Add(BcmHalIf *pHalIf);

public:

    /// This causes all of the interface drivers to be loaded, as enabled in
    /// nonvol.  Previously, this was done as a side-effect of calling
    /// GetSingleton() the first time; however, that caused problems in BFC
    /// and made it difficult to skip loading drivers, so I had to pull it out
    /// to a separate method.
    ///
    /// PR6829
    ///
    void Initialize(void);

protected:

    /// Default Constructor.  Initializes the state of the object.
    ///
    /// PR6829 - This no longer causes drivers to be loaded as a side-effect of
    /// accessing this object the first time.  You must call Initialize() to do
    /// this.
    ///
    BcmHalIfFactory(void);

public:  // The stupid GNU compiler needs this to be public, even though it
         // really should be protected/private.

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.  The HalIfs that were created in the constructor will be deleted!
    /// You must make sure they are no longer being used (deregister them from
    /// PacketSwitch objects, etc).
    ///
    ~BcmHalIfFactory();
	BcmSnoop * FindSnoopByName(const char * pSnoopName);

protected:

    /// STL container for the HalIf objects.
    BcmHalIfList *pfHalIfList;

private:

    /// Copy Constructor.  Not supported.
    BcmHalIfFactory(const BcmHalIfFactory &otherInstance);


    /// Assignment operator.  Not supported.
    BcmHalIfFactory & operator = (const BcmHalIfFactory &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif


