//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
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
//  Filename:       CmApplication.h
//  Author:         David Pullen
//  Creation Date:  Feb 21, 2000
//
//****************************************************************************
//  Description:
//      This is the portable Cable Modem application main.  This class is
//      created by the target-specific main, configured with various platform-
//      specific devices and objects, and is then called to create and start
//      the portable, application-specific objects.
//
//****************************************************************************

#ifndef CMAPPLICATION_H
#define CMAPPLICATION_H

//********************** Include Files ***************************************

#include "BfcApplication.h"

#include "MessageLog.h"
//#include "DsgClientCtlThread.h"
//#include "DsgDownloadCtlThread.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmCountingSemaphore;
class BcmPacketSwitch;

#if (BCM_PING_SUPPORT)
class BcmPingThread;
#endif

class BcmDsgClientCtlThread;

#if (BCM_DUAL_DOWNLOAD_SUPPORT)
class BcmDsgDownloadCtlThread;
#endif

//********************** Class Declaration ***********************************


class BcmCmApplication : public BcmBfcApplication
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmCmApplication(void);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmCmApplication();

    // Sets the DSG-CC Ctl Thread instance that is to be used.
    //
    // Parameters:
    //      pDsgClientCtlThread - pointer to the DSG-CC Protocol Control object.
    //
    // Returns:  Nothing.
    //
    inline void SetDsgClientCtlThread(BcmDsgClientCtlThread *pDsgClientCtlThread);

    // Gets the DSG-CC Ctl Thread instance that is being used.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A pointer to the DSG-CC Ctl Thread instance.
    //
    inline BcmDsgClientCtlThread *GetDsgClientCtlThread(void);

    // Sets the Packet Switch instance that is to be used.
    //
    // Parameters:
    //      pPacketSwitch - pointer to the Packet Switch object.
    //
    // Returns:  Nothing.
    //
    inline void SetPacketSwitch(BcmPacketSwitch *pPacketSwitch);

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


    // Starts up DSG operation.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void StartDsgCC(void);

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

    BcmDsgClientCtlThread *pfDsgClientCtlThread;
    BcmPacketSwitch *pfPacketSwitch;
    BcmCountingSemaphore *pfAppSemaphore;

    #if (BCM_PING_SUPPORT)

    BcmPingThread *pfPingHelper;

    #endif

    bool fStartConsole;

private:

    // Copy Constructor.  Not supported.
    //
    BcmCmApplication(const BcmCmApplication &otherInstance);

    // Assignment operator.  Not supported.
    BcmCmApplication & operator = (const BcmCmApplication &otherInstance);

};


//********************** Inline Method Implementations ***********************


// Sets the DSG-CC Ctl Thread instance that is to be used.
//
// Parameters:
//      pDsgClientCtlThread - pointer to the DSG-CC Protocol Control object.
//
// Returns:  Nothing.
//
inline void BcmCmApplication::SetDsgClientCtlThread(BcmDsgClientCtlThread *pDsgClientCtlThread)
{
    pfDsgClientCtlThread = pDsgClientCtlThread;
}


// Gets the DOCSIS Ctl Thread instance that is being used.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the DOCSIS Ctl Thread instance.
//
inline BcmDsgClientCtlThread *BcmCmApplication::GetDsgClientCtlThread(void)
{
    return pfDsgClientCtlThread;
}

// Sets the Packet Switch instance that is to be used.
//
// Parameters:
//      pPacketSwitch - pointer to the Packet Switch object.
//
// Returns:  Nothing.
//
inline void BcmCmApplication::SetPacketSwitch(BcmPacketSwitch *pPacketSwitch)
{
    pfPacketSwitch = pPacketSwitch;
}


/*
#if (BCM_PING_SUPPORT)

// Accessors for the ping thread (primarily for use by the console).
//
// Parameters:
//      pPingHelper - pointer to the ping thread.
//
// Returns:
//      A pointer to the ping thread.
//
inline void BcmCmApplication::PingHelper(BcmPingThread *pPingHelper)
{
    pfPingHelper = pPingHelper;
}


inline BcmPingThread *BcmCmApplication::PingHelper(void)
{
    return pfPingHelper;
}

#endif
*/
// Tells the CM App whether to run the console or to leave it suspended.
//
// Parameters:
//      runConsole - true to run the console; false to leave it suspended.
//
// Returns:  Nothing.
//
inline void BcmCmApplication::RunConsole(bool runConsole)
{
    fStartConsole = runConsole;
}

inline bool BcmCmApplication::RunConsole(void) const
{
    return fStartConsole;
}


#endif


