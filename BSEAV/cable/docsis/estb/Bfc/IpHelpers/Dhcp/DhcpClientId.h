/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//  $Id$
//
//  Filename:       DhcpClientId.h
//  Author:         David Pullen
//  Creation Date:  June 10, 2002
//
//****************************************************************************

#ifndef DHCPCLIENTID_H
#define DHCPCLIENTID_H

//********************** Include Files ***************************************

#include "OctetBuffer.h"
#include "MacAddress.h"

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


/** \ingroup DhcpClientServer
*
*   This is a common helper class for the DHCP Client and Server; the client id
*   is a required parameter for many different objects, and there are two
*   approaches to representing it.  Most often, it is a MAC address, but it can
*   be any arbitrary array of octets.  There are other approaches that can be
*   added as needed.
*
*   Rather than duplicating the APIs to handle both MAC address and octet buffer
*   in the other DHCP objects, this class does this once, and then it is used
*   in all of the other APIs.
*/
class BcmDhcpClientId
{
public:

    /// Default Constructor.  Sets the htype to 0, and the value to 0.  Not
    /// very usable, but useful as a placeholder until this object can be
    /// copied or otherwise configured to some other value.
    ///
    BcmDhcpClientId(void);

    /// Initializing Constructor.  Sets the client id to the MAC address
    /// specified, with the htype value set to 1.
    ///
    /// \param
    ///      macAddress - the MAC address to use as the client id.
    ///
    BcmDhcpClientId(const BcmMacAddress &macAddress);

    /// Initializing Constructor.  Sets the client id to the values specified
    /// in the octet buffer, with the htype value specified.  Usually, the
    /// htype will be set to 0 in this case, so this is the default.
    ///
    /// \param
    ///      buffer - buffer of data to use at the client id.  Must be at least
    ///               one byte, and must be less than 254 bytes long.
    /// \param
    ///      htype - the htype value to be specified for the client id.
    ///
    BcmDhcpClientId(const BcmOctetBuffer &buffer, uint8 htype = 0);

    /// Copy Constructor.  Initializes the state of this object to match that
    /// of the instance passed in.
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    ///
    BcmDhcpClientId(const BcmDhcpClientId &otherInstance);

    /// Destructor.  Frees up any memory/objects allocated, cleans up internal
    /// state.
    ///
    ~BcmDhcpClientId();

    /// Assignment operator.  Copies the state of the instance passed in so that
    /// this object is "identical".
    ///
    /// \param
    ///      otherInstance - reference to the object to copy.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmDhcpClientId & operator = (const BcmDhcpClientId &otherInstance);

    /// Alternate assignment operator, using an octet buffer as the data.  The
    /// octet buffer must contain the htype as well as the value.
    ///
    /// \param
    ///      buffer - the buffer to be used for the assignment.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmDhcpClientId & operator = (const BcmOctetBuffer &buffer);

    /// Another alternate assignment operator, using a mac address as the data.
    /// The htype will be set to 1.
    ///
    /// \param
    ///      macAddress - the mac address to store.
    ///
    /// \return
    ///      A reference to "this" so that operator = can be chained.
    ///
    BcmDhcpClientId & operator = (const BcmMacAddress &macAddress);

    /// Equality operator.  Checks to see if this object is equivalent to the
    /// one specified.
    ///
    /// \param
    ///      otherInstance - reference to the object to compare.
    ///
    /// \retval
    ///      true if this object is equivalent to the one specified.
    /// \retval
    ///      false if this object is not equivalent to the one specified.
    ///
    inline bool operator == (const BcmDhcpClientId &otherInstance) const;

    /// Inequality operator.  This is the opposite of operator ==.
    ///
    /// \param
    ///      otherInstance - reference to the object to compare.
    ///
    /// \retval
    ///      true if this object is not equivalent to the one specified.
    /// \retval
    ///      false if this object is equivalent to the one specified.
    ///
    inline bool operator != (const BcmDhcpClientId &otherInstance) const;

    /// Less-than operator; primarily for use in STL containers.
    ///
    /// \param
    ///      otherInstance - reference to the object to compare.
    ///
    /// \retval
    ///      true if this object is less than the one specified.
    /// \retval
    ///      false if this object is not less than the one specified.
    ///
    inline bool operator < (const BcmDhcpClientId &otherInstance) const;

    /// Resets the object to an empty state (same as the default constructor).
    ///
    void Reset(void);

    /// Sets the value as a MAC address.
    ///
    /// \param
    ///      value - the value to use.
    ///
    void Set(const BcmMacAddress &value);

    /// Sets the value as an octet buffer, with the htype specified.
    ///
    /// \param
    ///      value - the value to use.
    /// \param
    ///      htype - the htype to use.
    ///
    void Set(const BcmOctetBuffer &value, uint8 htype = 0);

    /// The RFC4361 updates RFC2132 regarding the construction of client identifiers for 
    /// DHCPv4.  This helper method constructs this new-sytle client Id.
    /// 
    /// \param
    ///      value - The MAC address for the interface associated with this client Identifier.
    ///
    /// \retval  
    ///      true if successful.
    /// \retval
    ///      false if fail.
    ///
    bool Set4361Type(const BcmMacAddress &value);

    /// Does formatted output of the object to the specified ostream.
    ///
    /// \param
    ///      outputStream - the ostream to be used.
    /// \param
    ///      shortForm - tells whether or not to use the short form of output.
    ///
    /// \return
    ///      The reference to the ostream that was passed in.
    ///
    ostream & Print(ostream &outputStream, bool shortForm = false) const;

public:  // Various accessors.

    /// Checks to see if the format is a MAC address.
    ///
    /// \retval
    ///      true if the client id is a MAC address.
    /// \retval
    ///      false if the cliend id is not a MAC address.
    ///
    bool IsMacAddress(void) const;

    /// If the client id is a MAC address, then this returns it.
    ///
    /// \param
    ///      macAddress - storage for the MAC address to be returned.
    ///
    /// \retval
    ///      true if the MAC address returned is valid.
    /// \retval
    ///      false if the client id is not stored as a MAC address.
    ///
    bool AsMacAddress(BcmMacAddress &macAddress) const;

    /// Returns the client id data, excluding the htype value.
    ///
    /// \param
    ///      buffer - storage for the client id data to be returned.
    ///
    void AsOctetBuffer(BcmOctetBuffer &buffer) const;

    /// Returns the htype value for the client id.
    ///
    /// \return
    ///      The htype value.
    ///
    uint8 Htype(void) const;

    /// Returns the octet buffer used as storage for the client id data.  This
    /// includes the htype value.
    ///
    /// \return
    ///      A reference to the client id data.
    ///
    inline const BcmOctetBuffer &Value(void) const;

protected:

private:

    /// The client id, after formatting, including the htype value.
    BcmOctetBuffer fClientId;

};


//********************** Global instances ************************************

/// This is a global instance of a DHCP Client Id that is akin to the
/// kAllZerosMac object for BcmMacAddress objects; it can be used for comparison
/// purposes, and to assign to another object to reset its state.
extern const BcmDhcpClientId kDefaultDhcpClientId;

//********************** Inline Method Implementations ***********************


/// This allows you to do the following:
///
///    BcmDhcpClientId clientId;
///
///    cout << clientId << endl;
///
inline ostream & operator << (ostream &outputStream, const BcmDhcpClientId &clientId)
{
    // Print using the short form.
    return clientId.Print(outputStream, true);
}


inline bool BcmDhcpClientId::operator == (const BcmDhcpClientId &otherInstance) const
{
    // Just call the equality operator for octet buffers.
    return (fClientId == otherInstance.Value());
}


inline bool BcmDhcpClientId::operator != (const BcmDhcpClientId &otherInstance) const
{
    // Just call the inequality operator for octet buffers.
    return (fClientId != otherInstance.Value());
}


inline bool BcmDhcpClientId::operator < (const BcmDhcpClientId &otherInstance) const
{
    // Just call the less than operator for octet buffers.
    return (fClientId < otherInstance.Value());
}


inline const BcmOctetBuffer &BcmDhcpClientId::Value(void) const
{
    return fClientId;
}


#endif


