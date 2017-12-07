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

#ifndef SOCKETSET_H
#define SOCKETSET_H

//********************** Include Files ***************************************

// For storing the sockets
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <list>
#else
#include <list.h>
#endif
#include "BcmSocket.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmSocketSet
{
public:

    // Default Constructor.  Stores the name assigned to the object.
    //
    // Parameters:
    //
    // Returns:  N/A
    //
    BcmSocketSet(const char *pName);


    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    ~BcmSocketSet();

    // Adds the given socket to the read set.  If the socket is already in the set,
    // then it will not be added again.
    //
    // Parameters:
    //      pSocket - pointer to the socket to add.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (too many sockets, etc.).
    bool SetRead(BcmSocket *pSocket);

    // Adds the given socket to the write set.  If the socket is already in the set,
    // then it will not be added again.
    //
    // Parameters:
    //      pSocket - pointer to the socket to add.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (too many sockets, etc.).
    bool SetWrite(BcmSocket *pSocket);

    // Adds the given socket to the exception set.  If the socket is already in the set,
    // then it will not be added again.
    //
    // Parameters:
    //      pSocket - pointer to the socket to add.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (too many sockets, etc.).
    bool SetException(BcmSocket *pSocket);

    // Removes the given socket from the read set.  If the event wasn't a member of
    // the set, then false is returned.
    //
    // Parameters:
    //      pSocket - pointer to the socket to remove
    //
    // Returns:
    //      true if successful.
    //      false if the event isn't a member of the set, other problem.
    //
    bool ClearRead(BcmSocket *pSocket);

    // Removes the given socket from the write set.  If the event wasn't a member of
    // the set, then false is returned.
    //
    // Parameters:
    //      pSocket - pointer to the socket to remove
    //
    // Returns:
    //      true if successful.
    //      false if the event isn't a member of the set, other problem.
    //
    bool ClearWrite(BcmSocket *pSocket);

    // Removes the given socket from the exception set.  If the event wasn't a member of
    // the set, then false is returned.
    //
    // Parameters:
    //      pSocket - pointer to the socket to remove 
    //
    // Returns:
    //      true if successful.
    //      false if the event isn't a member of the set, other problem.
    //
    bool ClearException(BcmSocket *pSocket);

    // Clears the socket set, removing all of the sockets
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if successful.
    //      false if there were no sockets to remove
    bool ClearAll(void);

    // Used to determine whether a socket is in the read set or not
    // This method allows the application to determine which socket
    // in the set is ready for action after the return from select. 
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if in the read set
    //      false if not in the read set
    bool IsSetRead(BcmSocket *pSocket);

    // Used to determine whether a socket is in the write set or not
    // This method allows the application to determine which socket
    // in the set is ready for action after the return from select. 
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if in the read set
    //      false if not in the read set
    bool IsSetWrite(BcmSocket *pSocket);

    // Used to determine whether a socket is in the exception set or not
    // This method allows the application to determine which socket
    // in the set is ready for action after the return from select. 
    //
    // Parameters:  None.
    //
    // Returns:
    //      true if in the read set
    //      false if not in the read set
    bool IsSetException(BcmSocket *pSocket);

    // This routine permits a task to pend until one of a set sockets becomes ready. 
    //
    // Parameters:  
    //      timeoutMs - number of ms to wait or NULL if infinite. 
    int Select( unsigned long timeoutMS = 0 );

    // Simple accessor for the name of this object.
    //
    // Parameters:  None.
    //
    // Returns:
    //      The pointer to the name string.
    //
    inline const char *Name(void);

protected:    
    // The set of sockets that has been added to this class.for each of three types
    list<BcmSocket *> fReadSocketList;
    list<BcmSocket *> fWriteSocketList;
    list<BcmSocket *> fExceptionSocketList;

    BcmMessageLogSettings fMessageLogSettings;

private:

    // My assigned name.
    char *pfName;

};

//********************** Inline Method Implementations ***********************

// Simple accessor for the name of this object.
//
// Parameters:  None.
//
// Returns:
//      The pointer to the name string.
//
inline const char *BcmSocketSet::Name(void)
{
    return pfName;
}
#endif


