//****************************************************************************
//
// Copyright (c) 2007-2011 Broadcom Corporation
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
//  $Id: LnxCountingSemaphore.h 1.4 2006/08/02 19:50:22Z taskiran Release $
//
//  Filename:       LnxCountingSemaphore.h
//  Author:         Mark Rush
//  Creation Date:  Oct. 19, 2001
//
//****************************************************************************
//  Description:
//      This is the Linux implementation for counting semaphores.
// The pthreads library implements POSIX 1003.1b semaphores
//
//****************************************************************************

#ifndef LNXCOUNTINGSEMAPHORE_H
#define LNXCOUNTINGSEMAPHORE_H

//********************** Include Files ***************************************

#include <sys/time.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include "CountingSemaphore.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmLnxCountingSemaphore : public BcmCountingSemaphore
{
public:

    // Initializing Constructor.  Stores the name assigned to the object, and
    // creates the counting semaphore with the specified parameters.
    //
    // Parameters:
    //      isBounded - set this to true if you want the semaphore to have
    //                  a maximum count beyond which it cannot be incremented.
    //                  Set it to false if you don't want a maximum count.
    //      maxCount - the max count to be used, if isBounded is true.  This
    //                 is ignored if isBounded is false.
    //      initialCount - the count that the semaphore should start off with.
    //      pEvent - an Event to notify when Release() is called
    //      pName - the text name that is to be given to the object, for
    //              debugging purposes.
    //
    // Returns:  N/A
    //
    BcmLnxCountingSemaphore(bool isBounded, unsigned int maxCount,
                             unsigned int initialCount,
                             BcmEvent *pEvent = NULL,
                             const char *pName = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmLnxCountingSemaphore();

    // Tries to get access to the semaphore, reducing its count by 1.  If
    // the count is 0, then the thread will block (based on the timeout
    // parameters).
    //
    // By default, if you call this with no parameters, then it will not
    // time out waiting for the semaphore.  If you call it with kTimeout and 0
    // as the timeoutMS value, then it will return immediately if the semaphore
    // is not available.
    //
    // Parameters:
    //      mode - tells whether or not you want to time out if the semaphore is
    //             not granted in the specified amount of time.
    //      timeoutMS - the amount of time to wait for the semaphore.  This is
    //                  ignored if mode is not set to kTimeout.
    //
    // Returns:
    //      true if the semaphore was successfully gotten.
    //      false if the semaphore was not available (timed out waiting for it
    //          to be released, etc.).
    //
    virtual bool Get(GetMode mode = kForever, unsigned long timeoutMS = 0);

    // Releases the semaphore, increasing its count by 1.  If the semaphore has
    // a ceiling to the count, then it will not be increased beyond it.  If the
    // count is 0 and a thread is waiting for the semaphore, then it will be
    // released to run.
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if the semaphore was released.
    //      false if there was a problem (OS-specific).
    //
    virtual bool Release(void);

    // This method lets a thread read the current count of the semaphore.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The semaphores current count.
    //
    virtual unsigned int GetCount(void) const;


private:

    bool fIsBounded;
    unsigned int fMaxCount;

	bool fSemInitialized;

	// The POSIX semaphore
	sem_t fSemaphore;
};


//********************** Inline Method Implementations ***********************

#endif


