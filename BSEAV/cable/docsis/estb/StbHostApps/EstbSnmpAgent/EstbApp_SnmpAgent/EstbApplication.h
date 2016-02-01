//****************************************************************************
//
//  Copyright (c) 2006  Broadcom Corporation
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
//  Filename:       EstbApplication.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This is the portable eSTB SNMP Agent application main.  This class is
//      created by the target-specific main, configured with various platform-
//      specific devices and objects, and is then called to create and start
//      the portable, application-specific objects.
//
//****************************************************************************

#ifndef ESTBAPPLICATION_H
#define ESTBAPPLICATION_H

//********************** Include Files ***************************************

#include "BfcApplication.h"

#include "MessageLog.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmCountingSemaphore;
class BcmPacketSwitch;

#if (BCM_PING_SUPPORT)
class BcmPingThread;
#endif

//********************** Class Declaration ***********************************


class BcmEstbApplication : public BcmBfcApplication
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmEstbApplication(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmEstbApplication();

    // Sets the Packet Switch instance that is to be used.
    //
    // Parameters:
    //      pPacketSwitch - pointer to the Packet Switch object.
    //
    // Returns:  Nothing.
    //
    inline void SetPacketSwitch(BcmPacketSwitch *pPacketSwitch);

#if (BCM_SETTOP_SUPPORT)
    // Sets the eSTB Packet Switch instance that is to be used.
    //
    // Parameters:
    //      pStbPacketSwitch - pointer to the Packet Switch object.
    //
    // Returns:  Nothing.
    //
    inline void SetStbPacketSwitch(BcmPacketSwitch *pStbPacketSwitch);
    // Gets the STB Packet Switch instance that is used.
    //
    // Parameters:
    //      VOID
    //
    // Returns:  
    //      pPacketSwitch - pointer to the Packet Switch object.
    //
    inline BcmPacketSwitch *GetStbPacketSwitch(void);
#endif

    // The target main will call this to start the portable application running.
    //
    // Note that in a typical embedded system, this will generally never return,
    // since the application is supposed to run forever.  In host simulators,
    // this will exit typically when the user types 'quit'.
    //
    // The target main can choose to handle returns from this method however it
    // wants (it can exit the app, call Run again, tear objects down and restart,
    // reset the hardware, etc.).
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the application ran and exited successfully.
    //      false if the application exited abnormally.
    //
    virtual bool Run(void);


#if (SNMP_SUPPORT)
    // Instantiate, register, populate and start the SNMP agent(s).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartSnmp(void);
#endif

    // Prints the company logo and version information.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void PrintVersion(void);

    #if (BCM_PING_SUPPORT)

    // Accessors for the ping thread (primarily for use by the console).
    //
    // Parameters:
    //      pPingHelper - pointer to the ping thread.
    //
    // Returns:
    //      A pointer to the ping thread.
    //
    inline void PingHelper(BcmPingThread *pPingHelper);
    inline BcmPingThread *PingHelper(void);

    #endif

    // Tells the CM App whether to run the console or to leave it suspended.
    //
    // Parameters:
    //      runConsole - true to run the console; false to leave it suspended.
    //
    // Returns:  Nothing.
    //
    inline void RunConsole(bool runConsole);
    inline bool RunConsole (void) const; // "Get" method for above

private:

    BcmPacketSwitch *pfPacketSwitch;
    BcmCountingSemaphore *pfAppSemaphore;

#if (BCM_SETTOP_SUPPORT)
    BcmPacketSwitch *pfStbPacketSwitch;
#endif

    #if (BCM_PING_SUPPORT)

    BcmPingThread *pfPingHelper;

    #endif

    bool fStartConsole;

private:

    // Copy Constructor.  Not supported.
    //
    BcmEstbApplication(const BcmEstbApplication &otherInstance);

    // Assignment operator.  Not supported.
    BcmEstbApplication & operator = (const BcmEstbApplication &otherInstance);

};


//********************** Inline Method Implementations ***********************


// Sets the Packet Switch instance that is to be used.
//
// Parameters:
//      pPacketSwitch - pointer to the Packet Switch object.
//
// Returns:  Nothing.
//
inline void BcmEstbApplication::SetPacketSwitch(BcmPacketSwitch *pPacketSwitch)
{
    pfPacketSwitch = pPacketSwitch;
}

#if (BCM_SETTOP_SUPPORT)

// Sets the Packet Switch instance that is to be used.
//
// Parameters:
//      pPacketSwitch - pointer to the Packet Switch object.
//
// Returns:  Nothing.
//
inline void BcmEstbApplication::SetStbPacketSwitch(BcmPacketSwitch *pStbPacketSwitch)
{
    pfStbPacketSwitch = pStbPacketSwitch;
}

// Gets the STB Packet Switch instance that is used.
//
// Parameters:
//      VOID
//
// Returns:  
//      pPacketSwitch - pointer to the Packet Switch object.
//
inline BcmPacketSwitch *BcmEstbApplication::GetStbPacketSwitch(void)
{
    return pfStbPacketSwitch;
}

#endif


#if (BCM_PING_SUPPORT)

// Accessors for the ping thread (primarily for use by the console).
//
// Parameters:
//      pPingHelper - pointer to the ping thread.
//
// Returns:
//      A pointer to the ping thread.
//
inline void BcmEstbApplication::PingHelper(BcmPingThread *pPingHelper)
{
    pfPingHelper = pPingHelper;
}


inline BcmPingThread *BcmEstbApplication::PingHelper(void)
{
    return pfPingHelper;
}

#endif

// Tells the eSTB SNMP Agent App whether to run the console or to leave it suspended.
//
// Parameters:
//      runConsole - true to run the console; false to leave it suspended.
//
// Returns:  Nothing.
//
inline void BcmEstbApplication::RunConsole(bool runConsole)
{
    fStartConsole = runConsole;
}

inline bool BcmEstbApplication::RunConsole(void) const
{
    return fStartConsole;
}


#endif


