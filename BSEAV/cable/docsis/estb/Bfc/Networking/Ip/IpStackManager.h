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
//  Filename:       IpStackManager.h
//  Author:         David Pullen
//  Creation Date:  June 8, 2002
//
//****************************************************************************
//  Description:
//      This is a singleton class that is used to access one or more of the
//      IpStack HalIf objects that exist in the system.  The IpStack HalIf
//      objects register/deregister themselves at the appropriate time (after
//      driver init and driver shutdown).
//
//      Clients will call me, specifying the stack number, to retrieve the
//      pointer to the IpStack HalIf object.
//
//****************************************************************************

#ifndef IPSTACKMANAGER_H
#define IPSTACKMANAGER_H

#include "MessageLog.h"
#include "IpAddress.h"

//********************** Include Files ***************************************

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmBaseIpHalIf;
class BcmIpStackList;

//********************** Class Declaration ***********************************


class BcmIpStackManager
{
public:

    // Returns the singleton IP stack manager object.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A reference to the stack manager.
    //
    static BcmIpStackManager &GetSingleton(void);

    // Returns the IP stack corresponding to the stack number specified.  If
    // that stack hasn't been created/registered, then this method returns NULL.
    //
    // Parameters:
    //      stackNumber - the IP stack number (1..n) to retrieve.
    //
    // Returns:
    //      Pointer to the IP stack, or NULL is not found.
    //
    BcmBaseIpHalIf *Find(unsigned int stackNumber) const;

    // Selects one of the IP stack interfaces as the primary; the rest are
    // configured as non-primary.  This is used by the IP HalIf object to
    // determine whether or not certain actions are allowed.
    //
    // For example, the eCos IP stack gets all loopy if you allow more than
    // one interface to add a default route; as a result, the app needs to
    // select one of the interfaces as the primary so that only that one can
    // add routes.
    //
    // If the specified IP stack doesn't exist, then none of the stacks will
    // be selected as primary.  This effectively de-selects them all.
    //
    // Note that IP stack 1 is automatically set as the primary when the system
    // starts up; you only need to call this method to select a different stack
    // as primary.
    //
    // Parameters:
    //      stackNumber - the IP stack number (1..n) to select as primary.
    //
    // Returns:  Nothing.
    //
    void SelectPrimary(unsigned int stackNumber);


    // Accessors for primary IP address and subnet mask for the BcmBaseIpHalIf 
    // associated with the specified IP stack number.  These are "find and
    // access" helper functions.  uint32 params are returned in network order.  
    // Caller should test that the returned IP address is non-zero.
    //
    // PrimaryIpAddress() - Returns the first IP address configured for the 
    //      network interface associated with the specified IP stack number.
    // 
    const BcmIpAddress& PrimaryIpAddress( unsigned int stackNumber ) const;
    uint32 PrimaryIpAddressU32( unsigned int stackNumber ) const;
    //
    //
    // SubnetMask() - Returns the subnet mask stored when the primary
    //      IP address of the network interface was configured.  
    //
    const BcmIpAddress& SubnetMask( unsigned int stackNumber ) const;
    uint32 SubnetMaskU32( unsigned int stackNumber ) const;


    // Returns the total number of IP stacks registered.
    //
    // Parameters:
    //      N/A
    //
    // Returns:
    //      Number of IP stacks.
    //
    uint32 Size(void) const;

public:

    // Registers an IP stack instance; this will be called by BcmBaseIpHalIf
    // after driver initialization is complete.
    //
    // Parameters:
    //      pIpHalIf - pointer to the IP HalIf object to be registered.
    //      stackNumber - the IP stack number (1..n).
    //
    // Returns:  Nothing.
    //
    void Register(BcmBaseIpHalIf *pIpHalIf, unsigned int stackNumber);

    // Deregisters an IP stack instance.  This will be called by BcmBaseIpHalIf
    // when it deinitializes the driver.
    //
    // Parameters:
    //      pIpHalIf - pointer to the IP HalIf object to be deregistered.
    //
    // Returns:  Nothing.
    //
    void Deregister(BcmBaseIpHalIf *pIpHalIf);

    // Determines how much message logging I will do.
    BcmMessageLogSettings fMessageLogSettings; 

protected:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmIpStackManager(void);

public:  // The stupid GNU compile needs this to be public, even though it
         // really should be protected/private.

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmIpStackManager();

protected:

    // STL container
    BcmIpStackList *pfIpStackList;

private:

    // Copy Constructor.  Not supported.
    BcmIpStackManager(const BcmIpStackManager &otherInstance);

    // Assignment operator.  Not supported.
    BcmIpStackManager & operator = (const BcmIpStackManager &otherInstance);

};


//********************** Inline Method Implementations ***********************


#endif


