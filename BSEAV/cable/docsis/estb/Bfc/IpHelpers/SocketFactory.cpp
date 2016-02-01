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
//  $Id: SocketFactory.cpp 1.14 2004/01/24 00:48:59Z dpullen Release $
//
//  Filename:       SocketFactory.cpp
//  Author:         Sean Dunlap
//  Creation Date:  June 19, 2000
//
//****************************************************************************
//  Description:
//      This class is responsible for creating the correct type of
//      Socket, based on the OS selection.
//
//
//****************************************************************************

//********************** Include Files ***************************************

// The types of Sockets that I know how to create.
#if defined(WIN32)
	#include "Win32Socket.h"
#elif defined(PSOS)       
	#include "PsosSocket.h"
#elif defined(TARGETOS_vxWorks)
	#include "VxSocket.h"
#elif defined(__linux__)       
    #include "LnxSocket.h"
#elif defined(TARGETOS_eCos)
    #include "ecosSocket.h"
#endif

// My api and definitions...
#include "typedefs.h"
#include "SocketFactory.h"
#include "BcmSocket.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// This is the method that you should call to get your socket
// instance
//
// Note that you become the owner of the object returned, and you must
// delete it when you no longer need it.
//
// Parameters:  None.
//
// Returns:
//      A pointer to your instance of the Socket class.
//
BcmSocket * BcmSocketFactory::NewSocket(int domain, int type, int protocol, int instance )
{
	CallTrace("BcmSocketFactory", "NewSocket");

    BcmSocket * pSocket = NULL;

	// Create the correct derived class.
#if   defined(WIN32)
	pSocket = new BcmWin32Socket(
									 domain,						 // address family (for example, AF_INET)
									 type,						 // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW 
									 protocol,					 // socket protocol (usually 0) 
									 instance					 // specifies the 1st or 2nd protocol stack instance
									 );
#elif defined(PSOS)

	pSocket = new BcmPsosSocket(
									domain,						 // address family (for example, AF_INET)
									type,							 // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW 
									protocol,					 // socket protocol (usually 0) 
									instance						 // specifies the 1st or 2nd protocol stack instance
									);

#elif defined(TARGETOS_vxWorks)

	pSocket = new BcmVxSocket(
								 domain,						  // address family (for example, AF_INET)
								 type,						  // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW 
								 protocol,					  // socket protocol (usually 0) 
								 instance					  // specifies the 1st or 2nd protocol stack instance
								 );
#elif defined(__linux__)       

	pSocket = new BcmLnxSocket(
								  domain,						// address family (for example, AF_INET)
								  type,							// SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW 
								  protocol,						// socket protocol (usually 0) 
								  instance						// specifies the 1st or 2nd protocol stack instance
								  );

#elif defined(TARGETOS_eCos)

	pSocket = new BcmEcosSocket( domain, type, protocol, instance );

#else

	// OS Not Supported
	gLogMessageRaw << "ERROR - BcmSocketFactory OS Not Supported!" << endl;
	return(BcmSocket *)NULL;

#endif

    // if the socket descriptor is 0 then the socket is unusable. 
    // OS resources were not available to create it.  Delete the 
    // BcmSocket object and return NULL.  
    if (pSocket->SocketDescriptor() == 0)
    {
        gLogMessageRaw << "ERROR - BcmSocketFactory::NewSocket - socket descriptor is NULL!" << endl;
        delete pSocket;
        pSocket = NULL;
    }

    return pSocket;
}


