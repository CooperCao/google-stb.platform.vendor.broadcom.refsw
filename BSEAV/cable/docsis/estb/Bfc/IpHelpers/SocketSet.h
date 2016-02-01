//****************************************************************************
//
//  Copyright (c) 1999  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: SocketSet.h 1.7 2003/06/09 16:29:42Z koneal Release $
//
//  Filename:       SocketSet.cpp
//  Author:         Sean Dunlap
//  Creation Date:  July 11, 2000
//
//****************************************************************************
//  Description:
//      This is the class for socket sets; a socket set is a logical grouping 
//      of individual sockets into three categories, Read, Write and Exception.
//      It allows you to use select() to wait on a socket to become readable, 
//      writeable, or an exception to occur.
//
//      This OO interface alternative to creating fd_set structs based on a 
//      socket descriptors the macros FD_ZERO, FS_SET, FD_ISSET and making 
//      procedure calls to select().   Otherwise, it works the same. 
//****************************************************************************

#ifndef SOCKETSET_H
#define SOCKETSET_H

//********************** Include Files ***************************************

// For storing the sockets
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
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


