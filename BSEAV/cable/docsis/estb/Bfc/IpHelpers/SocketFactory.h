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
//  $Id: SocketFactory.h 1.10 2004/09/09 15:54:08Z tclack Release $
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
//      This class just has a single static method; you don't need to create
//      an instance of this class.
//
//****************************************************************************

//********************** Include Files ***************************************

#ifndef SOCKET_FACTORY_H
#define SOCKET_FACTORY_H

#include "typedefs.h"
#include "BcmSocket.h"

 
#if defined(TARGETOS_vxWorks)
#include <sockLib.h>
#include <socket.h>
#include <sockLib.h>
#include <netinet/in.h>

#define ICMP   IPPROTO_ICMP
#define UDP    IPPROTO_UDP
#define TCP    IPPROTO_TCP

#endif

#if defined(__linux__) || defined( TARGETOS_eCos ) || defined( TARGETOS_Qnx6 )
#include <sys/socket.h>
#include <netinet/in.h>

#define ICMP   IPPROTO_ICMP
#define UDP    IPPROTO_UDP
#define TCP    IPPROTO_TCP

#endif

#if defined( TARGETOS_Qnx6 )
#include <arpa/inet.h>
#endif



//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmSocket;

//********************** Class Declaration ***********************************

// This is the factory method that you should call to get your socket
// instance (object)
//
// Note that you become the owner of the object returned, and you must
// delete it when you no longer need it.
//
// Parameters:  
//      instance - selects whether you wat the socket associates with the 1st or 2nd instance
//                  if the protocol stack
//      domain - usually AF_INET
//      type - datagram, stream or raw socket
//      protocol - IPPROTO_UDP, IPPROTO_TCP etc.
//
// Returns:
//      A pointer to your instance of the Socket class.
//
class BcmSocketFactory
{
public:

    static BcmSocket *NewSocket(  
                                int domain,         // address family (for example, AF_INET)
                                int type,           // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW 
                                int protocol,       // socket protocol 
                                int instance = 0    // specifies the 1st or 2nd protocol stack instance
                               );

protected:

    // I have to declare all of these as protected in order to keep this class
    // from being used inappropriately.  The compiler will generate these
    // methods for you if you don't declare them yourself!

    // Default Constructor.  Not supported.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmSocketFactory( void );

    // Destructor.  Not supported.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //

    ~BcmSocketFactory();

};

#endif

