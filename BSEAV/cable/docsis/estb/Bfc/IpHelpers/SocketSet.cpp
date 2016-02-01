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
//  $Id: SocketSet.cpp 1.16 2005/11/02 18:10:51Z dpullen Release $
//
//  Filename:       SocketSet.cpp
//  Author:         Sean Dunlap
//  Creation Date:  July 11, 2000
//
//****************************************************************************
//  Description:
//      This is the class for socket sets; a socket set is a logical grouping 
//      of individual sockets into three categories, Read, Write and Exception.
//      It allows you to use Select() to wait on a socket to become readable, 
//      writeable, or an exception to occur.
//
//      This OO interface alternative to creating fd_set structs based on a 
//      socket descriptors the macros FD_ZERO, FS_SET, FD_ISSET and making 
//      procedure calls to select().   Otherwise, it works the same. 
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "SocketSet.h"
#include "OperatingSystem.h"
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <algorithm>
#else
#include <algo.h>
#endif

// Include the appropriate header file, based on OS
#if defined(WIN32)
    #include "winsock.h"
#elif defined(PSOS)       
#elif defined(TARGETOS_eCos)
    #include <sys/select.h>
#elif defined(TARGETOS_Qnx6)
    #include <sys/select.h>
#elif defined(TARGETOS_vxWorks)
    #include <selectLib.h>
    #include <sys/times.h>      /* struct timeval */
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Forward Declaration *********************************

//********************** Class Method Implementations ************************


// Default Constructor.  Stores the name assigned to the object.
//
// Parameters:
//
// Returns:  N/A
//
BcmSocketSet::BcmSocketSet(const char *pName) :
    fMessageLogSettings("BcmSocketSet")
{
    CallTrace("BcmSocketSet", "BcmSocketSet");

    // Call the helper method to create my object name.
    pfName = BcmOperatingSystem::GenerateName(pName, "EventSet");

    // Set my instance name.
    fMessageLogSettings.SetInstanceName(pfName);

    fMessageLogSettings.Register();
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmSocketSet::~BcmSocketSet()
{
    CallTrace("BcmSocketSet", "~BcmSocketSet");

    fMessageLogSettings.Deregister();

    // Clean up other members.
    delete pfName;
    pfName = NULL;
}


// Adds the given socket to the read set.  If the event is already in the set,
// then it will not be added again.
//
// Parameters:
//      pSocket - pointer to the event to add.
//
// Returns:
//      true if successful.
//      false if there was a problem (too many sockets, etc.).
//
bool BcmSocketSet::SetRead(BcmSocket *pSocket) 
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "SetRead");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "SetRead") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Make sure it isn't already in the list, if the list isn't empty.
    if (!fReadSocketList.empty())
    {
        // Find my event.
        myIterator = find(fReadSocketList.begin(), fReadSocketList.end(), pSocket);

        if (myIterator != fReadSocketList.end())
        {
            return true;
        }
    }

    // Add it to the list.
    fReadSocketList.push_back(pSocket);

    return true;
}


// Adds the given socket to the write set.  If the event is already in the set,
// then it will not be added again.
//
// Parameters:
//      pSocket - pointer to the event to add.
//
// Returns:
//      true if successful.
//      false if there was a problem (too many sockets, etc.).
//
bool BcmSocketSet::SetWrite(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "SetWrite");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "SetWrite") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Make sure it isn't already in the list, if the list isn't empty.
    if (!fWriteSocketList.empty())
    {
        // Find my event.
        myIterator = find(fWriteSocketList.begin(), fWriteSocketList.end(), pSocket);

        if (myIterator != fWriteSocketList.end())
        {
            return true;
        }
    }

    // Add it to the list.
    fWriteSocketList.push_back(pSocket);

    return true;
}

// Adds the given socket to the exception set.  If the event is already in the set,
// then it will not be added again.
//
// Parameters:
//      pSocket - pointer to the event to add.
//
// Returns:
//      true if successful.
//      false if there was a problem (too many sockets, etc.).
//
bool BcmSocketSet::SetException(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "SetException");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "SetException") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Make sure it isn't already in the list, if the list isn't empty.
    if (!fExceptionSocketList.empty())
    {
        // Find my event.
        myIterator = find(fExceptionSocketList.begin(), fExceptionSocketList.end(), pSocket);

        if (myIterator != fExceptionSocketList.end())
        {
            return true;
        }
    }

    // Add it to the list.
    fExceptionSocketList.push_back(pSocket);

    return true;
}


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
bool BcmSocketSet::ClearRead(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "ClearRead");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ClearRead") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Only do this if the list isn't empty.
    if (!fReadSocketList.empty())
    {
        // Find my event.
        myIterator = find(fReadSocketList.begin(), fReadSocketList.end(), pSocket);

        if (myIterator != fReadSocketList.end())
        {
            // Remove it from the list.
            fReadSocketList.erase(myIterator);
            
            return true;
        }
    }

    // Didn't find it!
    gWarningMsg(fMessageLogSettings, "ClearRead") << "Specified socket isn't in the set!" << endl;

    return false;
}

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
bool BcmSocketSet::ClearWrite(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "ClearWrite");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ClearWrite") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Only do this if the list isn't empty.
    if (!fWriteSocketList.empty())
    {
        // Find my event.
        myIterator = find(fWriteSocketList.begin(), fWriteSocketList.end(), pSocket);

        if (myIterator != fWriteSocketList.end())
        {
            // Remove it from the list.
            fWriteSocketList.erase(myIterator);
            
            return true;
        }
    }

    // Didn't find it!
    gWarningMsg(fMessageLogSettings, "ClearWrite") << "Specified socket isn't in the set!" << endl;

    return false;
}

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
bool BcmSocketSet::ClearException(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;

    CallTrace("BcmSocketSet", "ClearException");

    // Make sure it's a valid event.
    if (pSocket == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ClearException") << "Socket parameter is NULL!" << endl;

        return false;
    }

    // Only do this if the list isn't empty.
    if (!fExceptionSocketList.empty())
    {
        // Find my event.
        myIterator = find(fExceptionSocketList.begin(), fExceptionSocketList.end(), pSocket);

        if (myIterator != fExceptionSocketList.end())
        {
            // Remove it from the list.
            fExceptionSocketList.erase(myIterator);
            
            return true;
        }
    }

    // Didn't find it!
    gWarningMsg(fMessageLogSettings, "ClearException") << "Specified socket isn't in the set!" << endl;

    return false;
}

// Clears the read/write/exception socket sets, removing all of the sockets.
//
// Parameters:  None.
//
// Returns:
//
bool BcmSocketSet::ClearAll(void)
{
    CallTrace("BcmSocketSet", "ClearAll");

    if (fReadSocketList.empty() && fWriteSocketList.empty() && fExceptionSocketList.empty())
    {
        return false;
    }

    // Clear the list.
    fReadSocketList.clear();
    fWriteSocketList.clear();
    fExceptionSocketList.clear();

    return true;
}

// Used to determine whether a socket is in the read set or not
// This method allows the application to determine which socket
// in the set is ready for action after the return from select. 
//
// Parameters:  None.
//
// Returns:
//      true if in the read set
//      false if not in the read set
bool BcmSocketSet::IsSetRead(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;
    bool isSet = false;

    CallTrace("BcmSocketSet", "IsSetRead");

    // Find my event.
    myIterator = find(fReadSocketList.begin(), fReadSocketList.end(), pSocket);

    if (myIterator != fReadSocketList.end())
    {
        isSet = true;
    }

    return isSet;
}

// Used to determine whether a socket is in the write set or not
// This method allows the application to determine which socket
// in the set is ready for action after the return from select. 
//
// Parameters:  None.
//
// Returns:
//      true if in the read set
//      false if not in the read set
bool BcmSocketSet::IsSetWrite(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;
    bool isSet = false;

    CallTrace("BcmSocketSet", "IsSetWrite");

    // Find my event.
    myIterator = find(fWriteSocketList.begin(), fWriteSocketList.end(), pSocket);

    if (myIterator != fWriteSocketList.end())
    {
        isSet = true;
    }

    return isSet;
}

// Used to determine whether a socket is in the exception set or not
// This method allows the application to determine which socket
// in the set is ready for action after the return from select. 
//
// Parameters:  None.
//
// Returns:
//      true if in the read set
//      false if not in the read set
bool BcmSocketSet::IsSetException(BcmSocket *pSocket)
{
    list<BcmSocket *>::iterator myIterator;
    bool isSet = false;

    CallTrace("BcmSocketSet", "IsSetException");

    // Find my event.
    myIterator = find(fExceptionSocketList.begin(), fExceptionSocketList.end(), pSocket);

    if (myIterator != fExceptionSocketList.end())
    {
        isSet = true;
    }

    return isSet;
}


// DESCRIPTION
// Pends until one of the sockets added to one of the three sets becomes ready.  
// Timeout can be specified as zero in which case the call will return immediately.  
// This will allow the user to poll the status of a socket, or making a blocking call. 
// If one of the sockets in the list becomes ready, it will be remove from the 
// list, as indicated by IsAddedRead(), IsAddedWrite(), or IsAddedException().
//
// Parameters:
//      timeoutMS - amount of time to wait, or 0 to reutrn immediately
// Returns:
//      

int BcmSocketSet::Select( unsigned long timeoutMS )
{
    int rc;

    fd_set read_set;
    fd_set write_set;
    fd_set exception_set;

    CallTrace("BcmSocketSet", "Select");


    // initialize the sets
    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&exception_set);

    struct timeval *pTimeout = NULL;
    struct timeval timeout;
    
    if (timeoutMS)
    {
      pTimeout = &timeout;
      timeout.tv_sec = timeoutMS / 1000;
      timeout.tv_usec = (timeoutMS - (1000 * timeout.tv_sec)) * 1000;
    }

    list<BcmSocket *>::iterator myIterator;

    myIterator = fReadSocketList.begin();
    while(myIterator != fReadSocketList.end())
    {
        FD_SET((*myIterator)->SocketDescriptor(), &read_set);
        myIterator++;
    }
   
    myIterator = fWriteSocketList.begin();
    while(myIterator != fWriteSocketList.end())
    {
        FD_SET((*myIterator)->SocketDescriptor(), &write_set);
        myIterator++;
    }

    myIterator = fExceptionSocketList.begin();
    while(myIterator != fExceptionSocketList.end())
    {
        FD_SET((*myIterator)->SocketDescriptor(), &exception_set);
        myIterator++;
    }

    rc = select(FD_SETSIZE, &read_set, &write_set, &exception_set, pTimeout);

    // Check to see if we timed out (rc == 0); if so, clear the sets so that
    // all of the sockets will be removed.  This should be done for us by the
    // IP stack, but in eCos it appears that this doesn't happen, so we have
    // to do it ourselves.
    if (rc == 0)
    {
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&exception_set);
    }

    // now remove sockets from the list that have expired
    myIterator = fReadSocketList.begin();
    while(myIterator != fReadSocketList.end())
    {
        // for each file descriptor that was set and is now cleared, 
        // remove the socket from the list
        if(!(FD_ISSET((*myIterator)->SocketDescriptor(), &read_set)))
        {
            // erase invalidates iterator, so post-increment
            myIterator = fReadSocketList.erase(myIterator);
        }
        else
        {
            myIterator++;
        }
    }
   
    // now remove sockets from the list that have expired
    myIterator = fWriteSocketList.begin();
    while(myIterator != fWriteSocketList.end())
    {
        // for each file descriptor that was set and is now cleared, 
        // remove the socket from the list
        if(!(FD_ISSET((*myIterator)->SocketDescriptor(), &write_set)))
        {
            // erase invalidates iterator, so post-increment
            myIterator = fWriteSocketList.erase(myIterator);
        }
        else
        {
            myIterator++;
        }
    }

    // now remove sockets from the list that have expired
    myIterator = fExceptionSocketList.begin();
    while(myIterator != fExceptionSocketList.end())
    {
        // for each file descriptor that was set and is now cleared, 
        // remove the socket from the list
        if(!(FD_ISSET((*myIterator)->SocketDescriptor(), &exception_set)))
        {
            // erase invalidates iterator, so post-increment
            myIterator = fExceptionSocketList.erase(myIterator);
        }
        else
        {        
            myIterator++;
        }
    }

    return rc;
}

