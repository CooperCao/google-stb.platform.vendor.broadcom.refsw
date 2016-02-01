//****************************************************************************
//
//  Copyright (c) 2000-2007  Broadcom Corporation
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
//  Filename:       Socket.cpp
//  Author:         Sean Dunlap
//  Creation Date:  June 19, 2000
//
//****************************************************************************
//  Description:
//      This is the abstract base class of the portable Socket 'factory classes'.  
//      There will be protocol stack (OS) specific derived classes that do the 
//      actual work.  Most stacks will be pretty close to BSD release 4.4 compliant.
//
//      I stuck to the naming conventions of BSD 4.4 Sockets, but because this is an 
//      OO interface, it is not necessary to specify the socket descriptor as the first
//      argument in each method.  The object itself keeps track of this.  
//
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "typedefs.h"
#include "BcmSocket.h"

#if defined(TARGETOS_vxWorks)
    #include "netinet/in.h"  // included to find definition of struct sockaddr_in
#elif defined(TARGETOS_Linux)
    #include <arpa/inet.h>
#elif defined(TARGETOS_eCos)
    #include <sys/socket.h> 
    #include <netinet/in.h>
#elif defined(TARGETOS_Qnx6)
    #include <sys/socket.h> 
    #include <arpa/inet.h>
#endif


//********************** Class Declaration ***********************************


BcmSocket::BcmSocket():
    fMessageLogSettings("BcmSocket")
{
    CallTrace("BcmSocket", "BcmSocket");

    fSocketDescriptor = 0;
}
    

BcmSocket::~BcmSocket(void)
{
    CallTrace("BcmSocket", "~BcmSocket");
}


int BcmSocket::SocketDescriptor(void) const
{ 
    CallTrace("BcmSocket", "SocketDescriptor");

    return fSocketDescriptor;
}


int BcmSocket::Bind( const BcmIpAddress &addr, uint16 port )
{
    CallTrace("BcmSocket", "Bind");

    if (addr.IsIpV6Address())
    {
        sockaddr_in6 sockaddr_param;

        loadSockAddrIn6(sockaddr_param, addr, port);

        return Bind( (sockaddr *)&sockaddr_param, sizeof( sockaddr_param ) );
    }
    else
    {
        // prepare params for virtual Bind() call.
        sockaddr_in sockaddr_param;  
        loadSockAddrIn( sockaddr_param, addr, port );

        // make the actual call
        return Bind( (sockaddr *)&sockaddr_param, sizeof( sockaddr_param ) );
    }
}


int BcmSocket::Connect( const BcmIpAddress &addr, uint16 port )
{
    CallTrace("BcmSocket", "Connect");

    if (addr.IsIpV6Address())
    {
        sockaddr_in6 sockaddr_param;

        loadSockAddrIn6(sockaddr_param, addr, port);

        return Connect( (sockaddr *)&sockaddr_param, sizeof( sockaddr_param ) );
    }
    else
    {
        // prepare params for virtual Connect() call.
        sockaddr_in sockaddr_param;  
        loadSockAddrIn( sockaddr_param, addr, port );

        return Connect( (sockaddr *)&sockaddr_param, sizeof( sockaddr_param ) );
    }
}


BcmSocket * BcmSocket::Accept(
                               BcmIpAddress   &addr,            // IP address of connecting device
                               uint16         &port             // port number of connecting device
                             )
{
    CallTrace("BcmSocket", "Accept");

    sockaddr_in name;
    int addrlen = sizeof(name);     // initialized to size of the name field 
    
    BcmSocket* result = Accept( (sockaddr *)&name, &addrlen );

    // the address that connected to us in now in the name field

    // extract the addressing information into the sockaddr struct
    addr.NetworkSet(name.sin_addr.s_addr);
    port = name.sin_port;
    name.sin_family = AF_INET;

    return result;
}


int BcmSocket::Sendto         (
                                char *          buf,
                                int             bufLen,
                                int             flags,
                                const BcmIpAddress   &addr,
                                uint16          port
                              )
{
    CallTraceDatapath("BcmSocket", "Sendto");

    if (addr.IsIpV6Address())
    {
        sockaddr_in6 sockaddr_param;

        loadSockAddrIn6(sockaddr_param, addr, port);

        return Sendto( buf, bufLen, flags, (sockaddr *)&sockaddr_param, 
            sizeof( sockaddr_param ) );
    }
    else
    {
        // prepare params for virtual Sendto() call.
        sockaddr_in sockaddr_param;  
        loadSockAddrIn( sockaddr_param, addr, port );

        return Sendto( buf, bufLen, flags, (sockaddr *)&sockaddr_param, 
            sizeof( sockaddr_param ) );
    }
}


int BcmSocket::Recvfrom       (
                                char *                  buf,          // pointer to data buffer 
                                int                     bufLen,       // length of buffer
                                int                     flags,        // flags to underlying protocols 
                                BcmIpAddress           &addr,         // senders IP address
                                uint16                 &port          // senders port number
                              )
{
    sockaddr_in6 from;
    int fromlen = sizeof(from);        // throw away
    int result;

    CallTraceDatapath("BcmSocket", "Recvfrom");

    // extract the addressing information into the sockaddr struct
    result = Recvfrom(buf, bufLen, flags, (struct sockaddr *) &from, &fromlen);

    if (from.sin6_family == AF_INET6)
    {
        addr.Set((uint8 *) &(from.sin6_addr), true);
        port = NetworkToHost(from.sin6_port);
    }
    else
    {
        sockaddr_in *pFrom = (sockaddr_in *) &from;

        addr.NetworkSet(pFrom->sin_addr.s_addr);
        port = NetworkToHost(pFrom->sin_port);
    }

    return result;
}


int BcmSocket::Getsockname    (
                                 BcmIpAddress     &addr,         // where to return name 
                                 uint16           &port          // where to write the port
                              )
{
    int result;
    sockaddr_in name;
    int namelen = sizeof(sockaddr_in); // throw away

    CallTrace("BcmSocket", "Getsockname");

    result = Getsockname((sockaddr *)&name, &namelen );

    addr.NetworkSet(name.sin_addr.s_addr);
    port = name.sin_port;

    return result;
}


void BcmSocket::loadSockAddrIn( sockaddr_in& sockaddr_in_, const BcmIpAddress &ip_addr_obj,
    uint16 port )
{
    // zero contents of target.
    memset( &sockaddr_in_, 0, sizeof(sockaddr_in_) );

    // load the uint32 IP address representation in sockaddr_in_.sin_addr.s_addr
    // with the contents of BcmIpAddress, in network order.
    ip_addr_obj.NetworkGet( (uint32 &)(sockaddr_in_.sin_addr.s_addr) );
    
    // load the uint16 port representation in sockaddr_in_.sin_port
    // in network order.
    sockaddr_in_.sin_port = htons(port);

    sockaddr_in_.sin_family = AF_INET;
#if !defined(TARGETOS_Linux) && !defined(TARGETOS_Win32)
    sockaddr_in_.sin_len = sizeof( struct sockaddr_in );
#endif
}


void BcmSocket::loadSockAddrIn6(sockaddr_in6 &sockaddr_in6_,
                                const BcmIpAddress &ip_addr_obj, uint16 port)
{
    memset(&sockaddr_in6_, 0, sizeof(sockaddr_in6_));

// TODO:  PORT TO LINUX WHEN REQUIRED
#if !defined(TARGETOS_Linux) && !defined(TARGETOS_Win32)
    sockaddr_in6_.sin6_len = sizeof(sockaddr_in6_);

    // specify interface
    sockaddr_in6_.sin6_scope_id = fInterfaceNumber;

#endif

    sockaddr_in6_.sin6_family = AF_INET6;
    ip_addr_obj.GetIpV6Address().Get((uint8 *) &(sockaddr_in6_.sin6_addr));
    sockaddr_in6_.sin6_port = htons(port);
}


