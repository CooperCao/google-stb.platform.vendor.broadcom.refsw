//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//  Filename:       DhcpV6System.h
//  Author:         Pinar Taskiran Cyr
//  Creation Date:  May 23, 2007
//
//****************************************************************************
//  $Id$
//
//  Filename:       DhcpV6System.h
//  Author:         Pinar Taskiran
//  Creation Date:  March 10, 2008
//
//****************************************************************************

#ifndef DhcpV6System_H
#define DhcpV6System_H

//********************** Include Files ***************************************

// My base class
#include "BfcSystem.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup DhcpV6System
*
*   This is the derived class for the DhcpV6 Regression system.  It
*   kicks off regression tests for the new BFC DHCPv6 component.
*/
class BcmDhcpV6System : public BcmBfcSystem
{
public:

    /// Default Constructor.  Initializes the state of the object...
    ///
    BcmDhcpV6System(void);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    virtual ~BcmDhcpV6System();

protected:  // These methods can be implemented/overridden by derived classes.

    /// Gives the derived class a chance to add it's own version information to
    /// the version banner object.  The base class provides a dummy
    /// implementation that does nothing in case the derived class doesn't have
    /// anything to add.
    ///
    /// \param
    ///      pVersionBanner - pointer to the version banner object to which the
    ///                       version info should be added.
    ///
    virtual void AddVersionInfo(BcmVersionBanner *pVersionBanner);

    /// Called by Run() after basic initialization has completed.  This lets the
    /// derived class kick off its application components.
    ///
    /// The derived class MUST return from this method in order for the base
    /// class to continue with other processing.
    ///
    /// Note that this method can be called multiple times!  Run() is a public
    /// method of this class, and it can be called by several sources, including
    /// the console after the system has already started up and is running.
    ///
    /// \retval
    ///      true if the application components were started successfully.
    /// \retval
    ///      false if there was a problem.
    ///
    virtual bool RunImpl(void);

    /// This method is called by Initialize() (near the end of the method), and
    /// it gives the derived class a final chance to do its own app-specific
    /// initialization.
    ///
    /// The base class provides a dummy implementation that just returns true.
    ///
    /// \retval
    ///      true if initialization succeeded.
    /// \retval
    ///      false if initialization failed for some reason.
    ///
    virtual bool InitializeImpl(void);

private:

    /// Copy Constructor.  Not supported.
    BcmDhcpV6System(const BcmDhcpV6System &otherInstance);

    /// Assignment operator.  Not supported.
    BcmDhcpV6System & operator = (const BcmDhcpV6System &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


