//****************************************************************************
//
// Copyright (c) 2008 Broadcom Corporation
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
//  Filename:       GenericDhcpV6ClientIf.h
//  Author:         Pinar Taskiran
//  Creation Date:  Feb 21, 2008
//
//****************************************************************************

#ifndef GenericDhcpV6ClientIf_H
#define GenericDhcpV6ClientIf_H

//********************** Include Files ***************************************

#include "DhcpV6ClientIf.h"

//********************** Global Types ****************************************

/// This is the prototype for callout functions that can be registered with the
/// DHCPv6 Client.  When an advertise/reply packet is received, and is acceptable.  
/// Then will call out to the registered client so it can check if the options are
/// acceptable.
///
/// This is what your function should look like:
///
/// bool DhcpV6OptionsCallout(const BcmDhcpV6OptionsList &options, unsigned int SolicitNumTries);
///
/// \param
///      dhcpPacket - the recieved DHCPv6 packet.
/// \param
///      SolicitNumTries - the number of Solicit messages this DHCPv6 Client has sent
///
/// \retval
///      true if the advertise/reply is acceptable.
/// \retval
///      false if the advertise/reply is not acceptable, and should not be selected or
///          should be NAKed.
///
typedef bool (*DhcpV6AdvertiseReplyCallout)(const BcmDhcpV6Packet &dhcpPacket, unsigned int solicitNumTries);

//********************** Global Constants ************************************
#define IPHALIF_ENABLED 1

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
#if IPHALIF_ENABLED
class BcmBaseIpHalIf;
#endif

//********************** Class Declaration ***********************************


/** \ingroup GenericDhcpV6ClientIf
*
*   This is the primary interface between a client object/thread and the
*   DHCPv6 Client Thread.  This object is associated with an IP stack, and
*   manages all of the IPv6 addresses for that stack.  In many cases, there
*   will only be a single IPv6 address for each stack.
*
*   There is a single DHCPv6 Client thread in the system, but there can be
*   multiple IP stacks, and there can even be multiple addresses for each
*   stack.
*
*   Note that this is the object that actually sends and processes packets,
*   and that services the state diagrams of each address.  The object will be
*   running in the context of the DHCPv6 client thread when this happens.
*/
class BcmGenericDhcpV6ClientIf : public BcmDhcpV6ClientIf
{
public:

    /// Initializing Constructor.  Stores the IP HalIf that I am being
    /// associated with.
    ///
    /// \note  The constructor will automatically create an Interface
    ///        Association lease object with
    ///        the client id = IP stack MAC address.  This lease will be set
    ///        so that it will automatically configure the IP stack when a
    ///        lease is acquired.  This will generally be the lease that most
    ///        clients will use when calling this object.
    ///
    /// \param
    ///      ipHalIf - the IP stack that I am being associated with.
    ///
#if IPHALIF_ENABLED
    BcmGenericDhcpV6ClientIf(BcmBaseIpHalIf &ipHalIf);
#else
	BcmGenericDhcpV6ClientIf(uint32 &dummy);
#endif
    //// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    ~BcmGenericDhcpV6ClientIf();

    /// Does formatted output of the object to the specified ostream.  It
    /// mostly just prints the client ids for the list of leases I am
    /// managing.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    ostream & Print(ostream &outputStream) const;

    /// Turns debugging on or off.  When on, excessive detail about DHCPv6 state
    /// operations will be printed to the message logger.
    ///
    /// \param
    ///      enable - true to enable debugging, false to disable.
    ///
    void Debug(bool enable);

public:

    /** \name Various accessors
    *
    * Accessors for various data members.
    */

public:

    //@{

    //@}

public:

    /** \name Methods to control the DHCPv6 Client
    *
    * UNFINISHED
    */
    //@{


    //@}

public:  // Methods called ONLY by the DHCP Client thread.  These methods should
         // not be called by any other thread or object.


private:

    /// Helper method to create, configure and send DHCPv6 packets to the
    /// network (per RFC3315).  
    ///
    
    ///
    /// \param
    ///      pPacket - the DHCPv6 packet to be sent.
    ///
    /// \retval
    ///      true if the packet was built and sent.
    /// \retval
    ///      false if we couldn't allocate a buffer or failed to send the
    ///      packet.
    ///
    virtual bool SendDhcpV6Packet(BcmDhcpV6Packet *pDhcpPacket);


private:

    /// Used for debugging and debug output control.
    BcmMessageLogSettings fMessageLogSettings;

private:

    /// Default Constructor.  Not supported.
    BcmGenericDhcpV6ClientIf(void);

    /// Copy Constructor.  Not supported.
    BcmGenericDhcpV6ClientIf(const BcmGenericDhcpV6ClientIf &otherInstance);

    /// Assignment operator.  Not supported.
    BcmGenericDhcpV6ClientIf & operator = (const BcmGenericDhcpV6ClientIf &otherInstance);

};


//********************** Inline Method Implementations ***********************


/// This allows you to do the following:
///
///    BcmGenericDhcpV6ClientIf clientIf;
///
///    cout << clientIf << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmGenericDhcpV6ClientIf &clientIf)
{
    return clientIf.Print(outputStream);
}



#endif

