//****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
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
//
//  Filename:       EcmRemoteIfHelperThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Nov 19, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

#ifndef DSGTFTPSERVERTHREAD_H
#define DSGTFTPSERVERTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************

class BcmDsgTftpThread : public BcmThread
{
public:

	// BcmDsgTftpThread() - Object constructor
	//		Initiates thread and initializes singleton.
	//
	// Parameters:
	//      initialPriority - Initial thread priority.
	//
	// Returns:
	//      None.
	//
	BcmDsgTftpThread(BcmOperatingSystem::ThreadPriority initialPriority);

	// ~BcmDsgTftpThread() - Object constructor
	//		Signals thread and waits for termination.
	//
	// Parameters:
	//      None.
	//
	// Returns:
	//      None.
	//
    virtual ~BcmDsgTftpThread();

	static BcmDsgTftpThread *GetSingletonInstance(void);

	// Print() - Print detailed formatted object contents 
	//      to specified ostream.
	//
	// Parameters:
	//      ostream& ostrm - reference to the destination output stream.
	//
	// Returns:
	//      reference to the destination output stream.
	//
	ostream& Print( ostream& ostrm ) const;

	void ErrorMessage(char *msg);

protected:

	// Initialize() - Initialize thread
	//		This is the first method called after the thread has
	//		been spawned, and is where the thread should create all OS objects.
	//		This has to be done here, rather than in the object's constructor,
	//		because some OS objects must be created in the context of the thread
	//		that will use them.  The object's constructor is still running in the
	//		context of the thread that created this object.
	//
	// Parameters:
	//      None.
	//
	// Returns:
	//      None.
	//
	bool Initialize();

	// ThreadMain()
	//		Implements the execution thread. Retrieves the local image version and compare it
	//		to the eCM image version. Upgrade the eCM if it is running an older version.
	//
	// Parameters:
	//		None.
	//
	// Returns:
	//      None.
	//
	void ThreadMain();

private:

	//********************** Private Types ****************************************

	//********************** Private Constants ************************************

	//********************** Private Variables ************************************
	// This is the singleton instance of this class.
	static BcmDsgTftpThread *pfSingletonInstance;

};

//********************** Inline Method Implementations ***********************
inline ostream& operator<<( ostream& ostrm, const BcmDsgTftpThread& rhs )
{
    return rhs.Print( ostrm );
}

// Gets the DOCSIS Ctl Thread instance that is being used.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the DOCSIS Ctl Thread instance.
//
inline BcmDsgTftpThread *BcmDsgTftpThread::GetSingletonInstance(void)
{
	return pfSingletonInstance;
}

#endif



