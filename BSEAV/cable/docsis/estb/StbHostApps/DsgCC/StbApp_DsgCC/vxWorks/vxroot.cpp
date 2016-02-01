//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id$
//
//  Filename:       vxroot.cpp
//  Author:         John McQueen
//  Creation Date:  July 25, 2000
//
//****************************************************************************
//  Description:  Application code entry point
//      
//
//****************************************************************************

//********************** Include Files ***************************************

#include "vxsys_conf.h"
#include "vxboard.h"

#include <iostream.h>
#include <shellLib.h>
#include <taskLib.h>
#include <memLib.h>
#include <usrLib.h>


// Target-specific initialization code for V2.
//#include "CmTargetMain.h"
#include "VxWorksConsoleCommands.h"

// Application startup code.

//#if (CM_HAL_INCLUDED)
#include "CmAppMain.h"
//#endif

#include "OperatingSystemFactory.h"

// The command processing console thread.
#include "vxTerminalConsoleThread.h"

// PR4517
#if (BCM_TELNET_SUPPORT)
#include "VxRemoteTerminalConsoleThread.h"
#endif

#include "MainCommandTable.h"

//#include "NonVolDevice.h"
//#include "HalIfNonVolSettings.h"


// Handles message logging stuff.
#include "MessageLog.h"

#if (CLIENT_INTERFACE_SUPPORT)
#include "SampleClient.h"
#endif

// PR4266
//#include "IpRoutingFilterSnoop.h"


/*---------------------------------------------------------------------*/
/* Export local procedures without name-mangling.                      */
/*---------------------------------------------------------------------*/
extern "C"
{
    void root(void);
 
	void InitializeIp(void);
}


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

void startupThreadEntryPt(void);

//********************** Class Method Implementations ************************


/***********************************************************************/
/*   root() -                                                          */
/*                                                                     */
/*  This function executes out of the tRoot task and is called after   */
/*  the kernel is fully initializes and ready for application code     */
/*  to run.                                                            */
/*                                                                     */
/***********************************************************************/
void root(void)
{
    int tid,rc;
    int startupThreadStackSize = 12 * 1024; // 8K is not enough

	InitializeIp();

//    #if (TORNADO_VERSION == 20)
//    
//    // PR4266 - need to register the IpInAddrErrorsCallout accessor function
//    // in the IpRoutingFilter Snoop so that packets discarded by the Snoop are
//    // counted by the stack.
//    IpInAddrErrorsCallout = BcmIpRoutingFilterSnoop::PacketsToStackDropped;
//
//    #else
//    
//    #warning IpInAddrErrorsCallout only supported for TORNADO 2.0; ipInAddrErrors will be incorrect.
//    
//    #endif

    // First, I need to set up the message logging ostream so that other objects
    // can use it.  If I don't set this up, then it's default value is NULL, and
    // all message log output will automatically be discarded.
    //
    // I need to do this as soon as possible after IO streams have been
    // initialized so that message logging output will be enabled.
    //
    // Since I want the output to go to the console, I'll set this to point to
    // cout.                      
    //
    // I could just as easily have set this up as a different ostream (going to
    // file, a trace buffer, a network socket, etc.).
    pgMessageLogStream = &cout;

    gLogMessageRaw << endl << "Hello... and welcome to the wonderful world of vxWorks!!\n\n" << endl;


    // Change tNetTask to have normal priority like the other data mover tasks
    // in the system
    tid = taskNameToId("tNetTask");
    if ( taskIdVerify(tid) == OK )
    {
        taskPrioritySet(tid,
                        BcmOperatingSystemFactory::ConvertThreadPriority(BcmOperatingSystem::kNormalPriority));
    }

    // Spawn off the Startup thread which will do our initialization and bring
    // up, let the vxWorks task tRoot exit gracefully. This will allow us to
    // use less stack space for our initialization and allow the tRoot thread's
    // stack to stay at the default value in case the OS system components
    // change in the future which need more stack space.
    //
    // Note that this task must not run higher than the watchdog priority, or
    // it can keep the watchdog from servicing the h/w timer.  The watchdog runs
    // at priority 5, so we will set this thread to priority 6.
    rc = taskSpawn("tStartup", 6, 0,
                   startupThreadStackSize,
                   (FUNCPTR)startupThreadEntryPt,
                   1,2,3,4,5,6,7,8,9,0);

    if ( rc == ERROR )
    {
        // halt everything!
        gLogMessageRaw << "Failed to spawn tStartup task.. halting system!" << endl;
        taskSuspend(0);
    }

#if (CLIENT_INTERFACE_SUPPORT)
		StartDsgTask();
#endif

		// let the tRoot task gracefully exit...
    exit(0);
}


/***********************************************************************/
/*  startupThreadEntryPt() -                                           */
/*                                                                     */
/*  This function is the entry point into the Cable Modem application. */
/*  It will be executed as the entry point of the tStartup thread.     */
/*                                                                     */
/***********************************************************************/
void startupThreadEntryPt(void)
{
    // Initialize my OS wrapper subsystem.  This needs to be done early on,
    // since the code below this may start threads and use OS wrapper resources.
    // This code doesn't use h/w resources or any drivers, so it is safe to do
    // this here.
    BcmOperatingSystemFactory::InitializeOperatingSystem();


    // Since we're running on the h/w target, I need to initialize the h/w.
    //CmTargetLowLevelInit();

    // Also initialize the CM application, which creates the nonvol objects
    // and reads them.
//    AppPreNonvolInit();

    // Now that nonvol has been read and verified, we need to start up target
    // specific stuff.
    //CmTargetPostNonvolInit();


    // Create/start the console thread; this needs to happen after nonvol is
    // read and verified.  The portable app object wants it to be suspended.
    // Set the priority to Low rather than Idle so that the ping thread can be
    // lower.
    BcmVxTerminalConsoleThread *pConsoleThread;

    // PR4517
    #if (BCM_TELNET_SUPPORT)

    // If telnet support is enabled, then create the derived class that will set
    // up the telnet server.
    pConsoleThread = new BcmVxRemoteTerminalConsoleThread(true,
                                                          BcmOperatingSystem::kLowPriority);

    #else

    pConsoleThread = new BcmVxTerminalConsoleThread(true,
                                                    BcmOperatingSystem::kLowPriority);

    #endif


    // Now we can start up application stuff.
    AppPostNonvolInit();


    // Add OS-specific command tables and commands.
    VxWorksCmdTableInit(BcmMainCommandTable::GetCommandTableSingleton(), 1000);

    // Add target-specific command tables and commands.
    //CmTargetCmdTableInit(BcmMainCommandTable::GetCommandTableSingleton(), 2000);





    //BcmHalIfNonVolSettings *pHalSettings = BcmHalIfNonVolSettings::GetSingletonInstance();

//#if (BCM_DELAY_ENDLOAD)
//
//    uint8 tempMacAddrArray[6];
//
//    for ( int j=0; j < BCM_NUM_IP_STACKS; j++ )
//    {
//        (pHalSettings->MacAddressIp(j+1)).Get(tempMacAddrArray[0], tempMacAddrArray[1],
//                                              tempMacAddrArray[2], tempMacAddrArray[3],
//                                              tempMacAddrArray[4], tempMacAddrArray[5]);
//        bcmHalMacAddrSet((char *)tempMacAddrArray,j);
//    }
//
//    loadBcmIpHalDriver();
//
//#endif



    // Now we kick off the application.
    AppRun();


    gLogMessageRaw << endl << "root:  The application exited!  Rebooting...\n" << flush;

    //BcmNonVolDevice *pNonVolDevice = BcmNonVolDevice::GetSingletonInstance();

    // Lock out access to the non-vol device before rebooting, so that we can
    // be sure the settings are not corrupted.
    //pNonVolDevice->Lock();


/*  UNFINISHED
#ifdef TELNET_SERVER

    // Force telnet users to be logged out cleanly.
    CommandHandler(NULL, kLogout, "");

#endif
*/

    // Rather than just halting operation, I'll punch the reset button...
    //BcmResetBoard();

#if 0



#if (BCM_SETTOP_SUPPORT)
 

/**********************************************************
           Settop/Cm Control Bridge Initialization 
**********************************************************/
#if (SETTOP_API_INCLUDED)          
    BcmDocsisControlBridge *pDocsisControlBridge = NULL;

    // Create the DocsisControlBridge object and establish the required links with the other objects.
    pDocsisControlBridge = new BcmDocsisControlBridge((ClientNotifier)BcmDocsisCallBackFunc); //, pCmDocsisCtlThread);    
#endif // (SETTOP_API_INCLUDED)      


    // Configure the system setting so that the Cable Modem within the Settop box does not
    // do a hard-reset but only stops and restarts the Docsis Control thread.
    pCmDocsisCtlThread->ShutdownExitsApp(false);
#endif






/**********************************************************
           Settop/Cm Control Bridge Registration 
**********************************************************/
#if (SETTOP_API_INCLUDED)


    // Register DocsisControlBridge object with DocsisCmHalIf object.
    pDocsisControlBridge->SetDocsisCmHalIf( pCmHalIf );

    // Register DocsisControlBridge object with CmDocsisCtlThread object.
    pDocsisControlBridge->SetDocsisCmCtlThread( pCmDocsisCtlThread );

    // Now establish the control bridge between the client and 
    // DocsisControlBridge object by giving the callback functions 
    // to the client.
    pDocsisControlBridge->Register( BcmDocsisCallbackRegister );

    // Who will delete this object?
    // delete pDocsisControlBridge; 

    // Register the ethernet device with the control bridge. 
    if ( pEthernetHalIf != NULL )
    {
        gLogMessageRaw << " Registering DocsisControlBridge object with the EthernetHalIf object." << endl;
        pDocsisControlBridge->SetEthernetHalIf( pEthernetHalIf );
    }

    // Register the usb device with the control bridge. 
    if (pUsbHalIf != NULL)
    {
        gLogMessageRaw << " Registering DocsisControlBridge object with the UsbHalIf object." << endl;
        pDocsisControlBridge->SetUsbHalIf( pUsbHalIf );
    } 
#endif






#if (IPSEC_SUPPORT)
    BcmCmIpsecNonVolSettings       *pipsecnonvol;        // pointer to the nonvol info

    // initialize the IPSEC module
    ipsecInit(NULL);
#ifdef BPI_SUPPORT_HARDWARE
    // init crypto hardware functions
    init_crypto_hardware();
#endif

    // Get the nonvol singleton.  
    pipsecnonvol = BcmCmIpsecNonVolSettings::GetSingletonInstance();

    // Write any data which was stored in NONVOL
    pipsecnonvol->IpsecWriteData();
#endif




#if (BCM_SETTOP_SUPPORT && SNMP_SUPPORT)


    // Since TFTP-initiated software upgrade is not used in the settop device,
    // docsBpi2CodeDownloadControl group is excluded from the SNMP view.
    // This configuration cannot be done before the SNMP agent starts.    
    // (See above pCmApplication->StartSnmp (pCmDocsisCtlThread, pForwarder)).
    unsigned long subtree_bpi2CodeDownloadControlMib[] = {1,3,6,1,2,1,10,127,6,1,4};
    AgentConfigView ("docsisNmAccessView", 
                     subtree_bpi2CodeDownloadControlMib, 
                     sizeof(subtree_bpi2CodeDownloadControlMib)/sizeof(unsigned long), 
                     VIEW_STATUS_EXCLUDE);
#endif


#if (BCM_VENDOR_SUPPORT) || (BCM_VENDOR_CMAPP_SUPPORT)

    // Give the vendor app a pointer to the main app to do with as he sees
    // fit...
    pVendorCmApplication->SetCmApplication (pCmApplication);

    // This is an entry point for a vendor application to run...
    // The Run() routine implemented by the vendor MUST return from this call 
    // in order for the CmApplication to run!
#if (CM_HAL_INCLUDED) 
    pVendorCmApplication->Run(pCmDocsisCtlThread, pForwarder);
#else
    pVendorCmApplication->Run(NULL, pForwarder);
#endif
#endif




#endif // #if 0 - DPullen, old code that is being reorganized


}






