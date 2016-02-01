//****************************************************************************
//
// Copyright (c) 2004-2008 Broadcom Corporation
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
//  Filename:       root.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  December 10, 2004
//
//****************************************************************************
//  Description:  Application code entry point
//
//
//****************************************************************************

//********************** Include Files ***************************************
#if ( BCM_CONSOLE_SUPPORT )
// Target-specific initialization code for V2
#include "LinuxConsoleCommands.h"

// The command processing console thread.
#include "LnxTerminalConsoleThread.h"

#include "MainCommandTable.h"

#endif

// Application startup code.
#include "CmAppMain.h"

#if (BCM_NONVOL_SUPPORT == 1)
	#include "CmTargetMain.h"
#endif

#include "OperatingSystemFactory.h"

// Handles message logging stuff.
#include "MessageLog.h"

/*---------------------------------------------------------------------*/
/* Export local procedures without name-mangling.                      */
/*---------------------------------------------------------------------*/
extern "C"
{
	int InitializeDsgCcApplication(void);
}

#if NPTL_SUPPORT
extern "C"
{
    void SetSignalMask(void);
}
extern unsigned char gSignalMaskSet;
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

void startupThreadEntryPt(void);


//********************** Class Method Implementations ************************


/***********************************************************************/
/*   main() -                                                          */
/*                                                                     */
/*  This function executes out of the tRoot task and is called after   */
/*  the kernel is fully initializes and ready for application code     */
/*  to run.                                                            */
/*                                                                     */
/***********************************************************************/
#if (DSGCC_BUILT_AS_LIB)

	int InitializeDsgCcApplication(void)

#else

	int main(void)

#endif
{
	unsigned int numberOfIpStacks = 0;
    uint8 tempMacAddrArray[6];
	bool stopEarlyAtConsole = false;	 // PR1968

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

	gLogMessageRaw << endl << "Hello... and welcome to the wonderful world of Linux!!\n\n" << endl;

#if NPTL_SUPPORT
#if (DSGCC_BUILT_AS_LIB)
	if (!gSignalMaskSet)
	{
		gLogMessageRaw << endl << "*******************************************************************************************************************" << endl;
		gLogMessageRaw << endl << "WARNING! " << endl;
		gLogMessageRaw << endl << "Signal Mask not set correctly.  Please set the signal mask by calling SetSignalMask() in the main thread," << endl;
		gLogMessageRaw <<          "before other threads are created.  Please see Bfc/OperatingSystem/LnxInterface.c for details." << endl;
		gLogMessageRaw << endl << "*******************************************************************************************************************" << endl;
	}
#else
    SetSignalMask();
#endif
#endif

	// Initialize my OS wrapper subsystem.
	BcmOperatingSystemFactory::InitializeOperatingSystem();

#if (BCM_NONVOL_SUPPORT == 1)
    // Since we're running on the h/w target, I need to initialize the h/w.
    CmTargetLowLevelInit();

    // Also initialize the CM application, which creates the nonvol objects
    // and reads them.
    AppPreNonvolInit();
#endif
    // Now that nonvol has been read and verified, we need to start up target
    // specific stuff.
    //CmTargetPostNonvolInit();

#if ( BCM_CONSOLE_SUPPORT )
    // Create/start the console thread; this needs to happen after nonvol is
	// read and verified.  The portable app object wants it to be suspended.
	// Set the priority Low rather than Idle so that the ping thread can be
	// lower.
	BcmLnxTerminalConsoleThread *pConsoleThread;

	pConsoleThread = new BcmLnxTerminalConsoleThread(true,
	                           BcmOperatingSystem::kLowPriority);
#endif

    // Now we can start up application stuff.
	AppPostNonvolInit();

#if ( BCM_CONSOLE_SUPPORT )
	// Add OS-specific command tables and commands.
	LinuxCmdTableInit(BcmMainCommandTable::GetCommandTableSingleton(), 1000);
#endif
    // Now we kick off the application
	AppRun();

    gLogMessageRaw << endl << "root:  The application exited! \n"
	    << flush;
}


