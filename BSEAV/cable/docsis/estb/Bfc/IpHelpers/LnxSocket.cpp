//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//  $Id$
//
//  Filename:       LnxSocket.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  November 5, 2001
//
//****************************************************************************
//  Description:
//      This is the Linux derived class of the portable Socket 'factory classes'.
//
//****************************************************************************

//********************** Include Files ***************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>

// My api and definitions...
#include "LnxSocket.h"
#include "SocketSet.h"
#include "IpAddress.h"
#include "MacAddress.h"
#include "OctetBuffer.h"
#include "OperatingSystem.h"

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
// maximum of 8 network interfaces are supported at this time.
#define BCM_MAX_NET_INTERFACES 8 
#define BCM_MIN_NETIF_NUM 1
#define BCM_MAX_NETIF_NUM BCM_MAX_NET_INTERFACES

//********************* Forward Declarations ************************************

//********************** External Functions **********************************
extern "C" unsigned long BcmGetDefaultIpGateway(unsigned int stackIndex);

//********************** Class Declaration ***********************************


BcmLnxSocket::BcmLnxSocket(
								  int domain,				  // address family (for example, AF_INET)
								  int type,					  // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
								  int protocol,			  // socket protocol (usually 0)
								  int interfaceNumber	  // the stack instance
								  ) : BcmSocket()
{
	// Override the class name given by my parent.
	fMessageLogSettings.SetModuleName("BcmLnxSocket");

	// go ahead and create the socket
	if (Create (domain, type, protocol, interfaceNumber) < 0)
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxSocket") << "Socket Create() error: " << errno << endl;
	}
}


// use this version of the constructor if you just want to wrap your
// own socket with this class.  Especially useful in the Accept method.
BcmLnxSocket::BcmLnxSocket(
								  int socketDescriptor,	  // the socket descriptor to be wrapped
								  int interfaceNumber	  // the stack instance
								  ) : BcmSocket()
{
	CallTrace("BcmLnxSocket", "BcmLnxSocket init constr");

	fSocketDescriptor = socketDescriptor;
	fInterfaceNumber = interfaceNumber;

}


// Destructor.  Cleans things up, closes the socket
//
// Parameters:  none
//
// Returns:  N/A
//

BcmLnxSocket::~BcmLnxSocket()
{
	Close();
}


// This method creates a socket for an existing object.  It is called on
// object creation, but may also be called on an existing object should the
// socket be Close()d.
//
// Parameters:
//      int domain: address family (for example, AF_INET)
//      int type: SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
//      int protocol: socket protocol
//      int instance: specifies the 1st or 2nd protocol stack instance
//
// Returns:
//      the value of fSocketDescriptor after the socket call.  Would
//      be a positive number if the call succeeds, -1 if it fails.
//
int BcmLnxSocket::Create (
								 int domain,			// address family (for example, AF_INET)
								 int type,				// SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
								 int protocol,			// socket protocol
								 int interfaceNumber	// specifies the 1st or 2nd protocol stack instance
								 )
{
	// if we have a socket already, close it.
	if (fSocketDescriptor >= 0)
		Close();

	fSocketDescriptor = socket( domain, type, protocol );
    if(fSocketDescriptor < 0)
    {
        // clear the socket descriptor so the application factory can detect the error...
        fSocketDescriptor = (unsigned int)NULL;
        gErrorMsg(fMessageLogSettings, "BcmLnxSocket") << "SOCKET_ERROR: " << errno << endl;
        return(SOCKET_ERROR);
    }

	fInterfaceNumber = interfaceNumber;

	/* bind socket to interface */
	BindSocket2Interface();

	/* ported from eCosSocket.cpp */
    {
        int optVal;

        optVal = 1;

        if (setsockopt(fSocketDescriptor, SOL_SOCKET, SO_REUSEADDR, 
                       (char *) &optVal, sizeof(optVal)) != 0)
        {
            gErrorMsg(fMessageLogSettings, "Create")
                << "Failed to clear SO_REUSEADDR for socket; " << errno << endl;
        }
    }

	return fSocketDescriptor;
}

// This method closes the socket.  Can be re-established with the Create call.
//
// Parameters:
//      none.
//
// Returns:
//      0 on success, -1 on failure.
//
int BcmLnxSocket::Close ()
{
	int Closed = 0;

	// only close the socket if it is not already closed.  Closing the NULL
	// socket will close all sockets owned by this thread, which is not what
	// the user intended.
	if (fSocketDescriptor)
	{
		Closed = close (fSocketDescriptor);
		fSocketDescriptor = 0;
	}

	return Closed;
}


// This method associates a network address (also referred to as its "name") with a
// specified socket so that other processes can connect or send to it. When a socket
// is created with socket( ), it belongs to an address family but has no assigned name.
//
// Parameters:
//      sockaddr - the name of the local socket (typically sockaddr_in struct)
//      namelen - sizeof the name field
//
// Returns:
//      OK, or ERROR if there is an invalid socket, the address is either unavailable or in use,
//      or the socket is already bound.
int BcmLnxSocket::Bind(
							 struct sockaddr * name,					  // name to be bound
							 int namelen									  // length (sizeof) of name field
							 )
{
	int rc;
	struct ifreq ifReq;

	rc = bind( fSocketDescriptor, name, namelen );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Bind") << "Socket bind() error: " << errno << endl;
	}

	if( !((fInterfaceNumber < BCM_MIN_NETIF_NUM) || (fInterfaceNumber > BCM_MAX_NETIF_NUM)) )
	{
		memset(&ifReq, 0, sizeof(ifReq));
	    if (setsockopt(fSocketDescriptor, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifReq, sizeof(ifReq)) < 0)
		{
			gErrorMsg(fMessageLogSettings, "BindSocket2Interface")
			<< "can not de-bound from eth" << fInterfaceNumber - 1 << "!" << endl;

			return -1;
		}
	}

	return rc;
}

// This method enables connections to a socket. It also specifies the maximum number of
// unaccepted connections that can be pending at one time (backlog). After enabling
// connections with listen( ), connections are actually accepted by accept( ).  A backlog of
// 5 is typical.
//
// Parameters:
//      backlog - the number of conenctions to queue
//
// Returns:
//      OK, or ERROR if the socket is invalid or unable to listen.
//
int BcmLnxSocket::Listen       (
										 int backlog							// number of connections to queue */
										 )
{
	int rc;

	rc = listen(fSocketDescriptor, backlog);

	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Listen") << "Socket listen() error: " << errno << endl;
		return rc;
	}

	return rc;
}

// If s is a socket of type SOCK_STREAM, this method establishes a virtual circuit between s and
// another socket specified by name. If s is of type SOCK_DGRAM, it permanently specifies the peer
// to which messages are sent. If s is of type SOCK_RAW, it specifies the raw socket upon which
// data is to be sent and received. The name parameter specifies the address of the other socket.
//
// Parameters:
//      name - address of the socket to connect to
//      namelen - the sizeof the name field
//
// Returns:
//      OK, or ERROR if the call fails.
int BcmLnxSocket::Connect(
								 struct sockaddr * name,				 // addr of socket to connect
								 int               namelen				 // length of name, in bytes
								 )
{
	int rc;

	rc = connect(fSocketDescriptor, name, namelen);

	if (rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Connect") << "Socket connect() error: " << errno << endl;
	}

	return rc;
}

// This method accepts a connection on a socket, and returns a new socket created for the
// connection. The socket must be bound to an address with bind( ), and enabled for connections
// by a call to listen( ). The accept( ) method dequeues the first connection and creates
// a new socket with the same properties as s. It blocks the caller until a connection is
// present, unless the socket is marked as non-blocking.
//
// The parameter addrlen should be initialized to the size of the available buffer pointed
// to by addr. Upon return, addrlen contains the size in bytes of the peer's address stored in
// addr.

// Parameters:
//      addr - address of the socket to accept a connection from
//      addrlen - the length of the addr field
//
// Returns
//      A socket descriptor, or ERROR if the call fails.
BcmSocket * BcmLnxSocket::Accept(
										  struct sockaddr * addr,			 // peer address
										  int *             addrlen		 // peer address length
										  )
{
	int socketDescriptor;

	CallTrace("BcmLnxSocket", "Accept");

	socketDescriptor = accept(fSocketDescriptor, addr, (socklen_t *) addrlen);

	if (socketDescriptor == INVALID_SOCKET)
    {
        // Don't report an error if there were no connections pending and the
        // sockt is marked as non-blocking.
        if (errno != EWOULDBLOCK)
        {
            gErrorMsg(fMessageLogSettings, "Accept") << "INVALID_SOCKET: " << errno << endl;
        }

        return NULL;
    }

	return new BcmLnxSocket( socketDescriptor, // the newly created socket from the accept
									 fInterfaceNumber	 // the stack instance
								  );
}


// This routine transmits data to a previously established connection-based (stream) socket.
//
// The maximum length of buf is subject to the limits on TCP buffer size; see the discussion
// of SO_SNDBUF in the setsockopt( ) manual entry.
//
// Parameters:
//      buf - pointer to buffer to transmit
//      bufLen - the length of that buffer
//      flags - you may OR the following values into the flags parameter with this operation:
//                      MSG_OOB (0x1) - Out-of-band data.
//                      MSG_DONTROUTE (0x4) - send without using routing tables.
//
// Returns:
//      The number of bytes actually sent, or ERROR if the call fails.
//
int BcmLnxSocket::Send(
							 char * buf,	 /* pointer to buffer to transmit */
							 int    bufLen, /* length of buffer */
							 int    flags	 /* flags to underlying protocols */
							 )
{
	int rc;
	rc = send(fSocketDescriptor, buf, bufLen, flags);

	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Send") << "Socket send() error: " << errno << endl;
	}
	return rc;
}

// This method sends a message to the datagram socket named by to. The socket s is received
// by the receiver as the sending socket.
// The maximum length of buf is subject to the limits on UDP buffer size. See the discussion
// of SO_SNDBUF in the setsockopt( ) manual entry.
//
// Parameters:
//      buf - pointer of the buffer to transmit
//      bufLen - the lenght of that buffer
//      flags - you can OR the following values into the flags parameter with this operation:
//                      MSG_OOB (0x1) - Out-of-band data.
//                      MSG_DONTROUTE (0x4) - Send without using routing tables.
//
// Returns:
//      The number of bytes sent, or ERROR if the call fails.
int BcmLnxSocket::Sendto      (
										char *            buf,			  // pointer to data buffer
										int               bufLen,		  // length of buffer
										int               flags,		  // flags to underlying protocols
										struct sockaddr * to,			  // recipient's address
										int               tolen			  // length of to sockaddr
										)
{
	int rc;

	// do not use this feature until we get the new version of netutils.  If you specify
	// the MSG_INTERFACE then the IP stack will ARP for IP addresses that are off net.

	// flag that we are specifying the interface number and then write the interface number
	//flags |= MSG_INTERFACE;
	//((sockaddr_intf *)to)->sin_ifno = fInterfaceNumber;
	rc = sendto(fSocketDescriptor, buf, bufLen, flags, to, tolen);

	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Sendto") << "Socket sendto() error: " << errno << endl;
	}
	return rc;
}

// This method receives data from a connection-based (stream) socket.
//
// The maximum length of buf is subject to the limits on TCP buffer size; see the discussion
// of SO_RCVBUF in the setsockopt( ) manual entry.
//
// Parameters:
//      You may OR the following values into the flags parameter with this operation:
//      MSG_OOB (0x1) - Out-of-band data.
//      MSG_PEEK (0x2) - Return data without removing it from socket.
//
// Returns:
//      The number of bytes received, or ERROR if the call fails.
int BcmLnxSocket::Recv        (
										char * buf,							  // buffer to write data to
										int    bufLen,						  // length of buffer
										int    flags						  // flags to underlying protocols
										)
{
	int rc;

	rc = recv( fSocketDescriptor, buf, bufLen, flags );

    if (rc < 0)
    {
        // Don't report an error if there were no connections pending and the
        // sockt is marked as non-blocking.
        if (errno != EWOULDBLOCK)
        {
            gErrorMsg(fMessageLogSettings, "Recv") << "SOCKET_ERROR: " << errno << endl;
        }
    }

	return rc;
}


// This method receives a message from a datagram socket regardless of whether it is connected.
// If from is non-zero, the address of the sender's socket is copied to it. The value-result
// parameter pFromLen should be initialized to the size of the from buffer. On return, pFromLen
// contains the actual size of the address stored in from.
//
// The maximum length of buf is subject to the limits on UDP buffer size; see the discussion of
// SO_RCVBUF in the setsockopt( ) manual entry.
//
// Parameters:
//      buf - pointer to rhe data buffer to receive into
//      bufLen - the available lenght in the buffer
//      flags - you may OR the following values into the flags parameter with this operation:
//                  MSG_OOB (0x1) - Out-of-band data.
//                  MSG_PEEK (0x2) - Return data without removing it from socket.
//
// Returns:
//      The number of number of bytes received, or ERROR if the call fails.
int BcmLnxSocket::Recvfrom    (
										char *            buf,			  // pointer to data buffer
										int               bufLen,		  // length of buffer
										int               flags,		  // flags to underlying protocols
										struct sockaddr   * from,		 // where to copy sender's addr
										int *             pFromLen		  // value/result length of from
										)
{
	int rc;

	// do not use this feature until we get the new version of netutils.  If you specify
	// the MSG_INTERFACE then the IP stack will ARP for IP addresses that are off net.

	// flag that we are specifying the interface number and then write the interface number
	//flags |= MSG_INTERFACE;
	//((sockaddr_intf *)from)->sin_ifno = fInterfaceNumber;

	rc = recvfrom(fSocketDescriptor, buf, bufLen, flags, from, (socklen_t *) pFromLen);
	if ( rc < 0 )
	{
		if (errno != EAGAIN)
			gErrorMsg(fMessageLogSettings, "Recvfrom") << "Socket recvfrom() error: " << errno << endl;
	}
	return rc;
}


// This method sets the options associated with a socket. To manipulate options at the "socket" level,
// level should be SOL_SOCKET. Any other levels should use the appropriate protocol number.
//
// Parameters:
//      level - protocol level of option
//      optname - option name
//      iotval - pointer to option value
//      optlen - length of the option value
//
//      OPTIONS FOR STREAM SOCKETS
//          SO_KEEPALIVE -- Detecting a Dead Connection
//          SO_LINGER -- Closing a Connection
//          TCP_NODELAY -- Delivering Messages Immediately
//          SO_DEBUG -- Debugging the underlying protocol
//
//      OPTIONS FOR DATAGRAM SOCKETS
//           SO_BROADCAST -- Sending to Multiple Destinations
//
//      OPTIONS FOR DATAGRAM AND RAW SOCKETS
//          IP_ADD_MEMBERSHIP -- Join a Multicast Group
//          IP_DROP_MEMBERSHIP -- Leave a Multicast Group
//          IP_MULTICAST_IF -- Select a Default Interface for Outgoing Multicasts
//          IP_MULTICAST_TTL -- Select a Default TTL
//          IP_MULTICAST_LOOP -- Enable or Disable Loopback
//
//      OPTIONS FOR BOTH STREAM AND DATAGRAM SOCKETS
//          SO_REUSEADDR -- Reusing a Socket Address
//          SO_SNDBUF -- Specifying the Size of the Send Buffer
//          SO_RCVBUF -- Specifying the Size of the Receive Buffer
//          SO_OOBINLINE -- Placing Urgent Data in the Normal Data Stream
//
// Returns:
//
//      OK, or ERROR if there is an invalid socket, an unknown option, an option length greater than MLEN,
//      insufficient mbufs, or the call is unable to set the specified option.
int BcmLnxSocket::Setsockopt   (
										 int    level,							// protocol level of option
										 int    optname,						// option name
										 char * optval,						// pointer to option value
										 int    optlen							// option length
										 )
{
	int rc;

	rc = setsockopt( fSocketDescriptor, level, optname, optval, optlen );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Setsockopt") << "Setsockopt() error: " << errno << " optname = " << optname << endl;
	}
	return rc;
}


// This method returns relevant option values associated with a socket. To manipulate
// options at the "socket" level, level should be SOL_SOCKET. Any other levels should
// use the appropriate protocol number. The parameter optlen should be initialized to
// indicate the amount of space referenced by optval. On return, the value of the option
// is copied to optval and the actual size of the option is copied to optlen.
//
// Although optval is passed as a char *, the actual variable whose address gets
// passed in should be an integer or a structure, depending on which optname is being
// passed. Refer to setsockopt( ) to determine the correct type of the actual variable
// (whose address should then be cast to a char *).
//
// Parameters:
//      level - protocol level for option
//      optname - the name of the option
//      optval - pointer to the option value
//      optlen - the length of the option
//
// Returns:
//      OK, or ERROR if there is an invalid socket, an unknown option, or the call is unable
//      to get the specified option.
//
// Example:
//      Because SO_REUSEADDR has an integer parameter, the variable to be passed to
//      getsockopt( ) should be declared as  int reuseVal; and passed in as  (char *)&reuseVal.
//
//      Otherwise the user might mistakenly declare reuseVal as a character, in which
//      case getsockopt( ) will only return the first byte of the integer representing
//      the state of this option. Then whether the return value is correct or always 0
//      depends on the endian-ness of the machine.
int BcmLnxSocket::Getsockopt   (
										 int    level,							// protocol level for options
										 int    optname,						// name of option
										 char * optval,						// where to put option
										 int *  optlen							// where to put option length
										 )
{
	int rc;

	rc = getsockopt( fSocketDescriptor, level, optname, optval, (socklen_t *) optlen );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Getsockopt") << "getsockopt() error: " << errno << endl;
	}
	return rc;
}


// This method gets the name of the peer connected to socket s. The parameter namelen should be initialized
// to indicate the amount of space referenced by name. On return, the name of the socket is copied to
// name and the actual size of the socket name is copied to namelen.
//
// Parameters:
//      sockaddr - pointer to the name of the socket that I am connected to
//      namelen - the space available for the name
//
// Returns:
//      OK, or ERROR if the socket is invalid or not connected.
int BcmLnxSocket::Getsockname  (
										 struct sockaddr * name,			// where to return name
										 int *             namelen			// space available in name, later filled in actual name size
										 )
{
	int rc;

	rc = getsockname( fSocketDescriptor, name, (socklen_t *) namelen );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Getsockname") << "getsockname() error: " << errno << endl;
	}
	return rc;
}


// This method gets the name of the peer connected to socket s. The parameter namelen should be initialized
// to indicate the amount of space referenced by name. On return, the name of the socket is copied to
// name and the actual size of the socket name is copied to namelen.
//
// Parameters:
//      sockaddr - pointer to the name of the socket that I am connected to
//      namelen - the space available for the name
//
// Returns:
//      OK, or ERROR if the socket is invalid or not connected.
int BcmLnxSocket::Getpeername  (
										 struct sockaddr * name,			// where to put name
										 int *             namelen			// space available in name, later filled in actual name size
										 )
{
	int rc;

	rc = getpeername( fSocketDescriptor, name, (socklen_t *) namelen );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Getpeername") << "Socket getpeername() error: " << errno << endl;
	}
	return rc;
}


// This method shuts down all, or part, of a connection-based socket s. If the value of how is 0,
// receives are disallowed. If how is 1, sends are disallowed. If how is 2, both sends and receives
// are disallowed.
//
// Parameters:
//
// Returns:
//      OK, or ERROR if the socket is invalid or not connected.
int BcmLnxSocket::Shutdown     (
										 int how									// 0 = receives disallowed 1 = sends disallowed 2 = sends and disallowed
										 )
{
	int rc;

	rc = shutdown( fSocketDescriptor, how );
	if ( rc < 0 )
	{
		gErrorMsg(fMessageLogSettings, "Shutdown") << "Socket shutdown() error: " << errno << endl;
	}
	return rc;
}



// This routine will set the blocking mode of the socket to non-blocking mode.
// Normally, socket operations that cannot immediately be completed cause the
// task that initiated the operation to block.  If the socket is marked as non-
// blocking, an operation that cannot complete without waiting will return
// an error.
//
// Parameters:
//      true if the socket should be non-blocking
//      false is the source should block
//
// Returns:
//      OK, or ERROR if the socket is invalid or not connected.
//
int BcmLnxSocket::SetSocketNonBlocking(bool non_blocking)
{
	int arg = 0;

	CallTrace("BcmLnxSocket", "SetSocketNonBlocking");

	// if set to non-blocking, set the argument to a 1
	if (non_blocking)
	{
		arg = 1;
	}

	return ioctl(fSocketDescriptor, (int) FIONBIO, &arg);
}


// This routine checks if there is data ready to be read
// from the socket receive buffer
//
// Parameters:
//      None.
//
// Returns:
//      true if there is data in the receive buffer ready to be read
//      false if the socket read buffer is empty
bool BcmLnxSocket::ReceiveBufferHasData(void)
{
   int bytesReady = 0;
   if(ioctl (fSocketDescriptor, FIONREAD, &bytesReady) != 0)
   {
      gErrorMsg(fMessageLogSettings, "ReceiveBufferHasData") << "ioctl failed: " << errno << endl;
      return false;
   }

   if(bytesReady)
   {
      return true;
   }
   return false;
}


// This routine will return the MAC address of the specified IP address contained in the
// sockaddr argument.  It will block the calling task while it periodically polls the
// protocol stacks ARP cache looking for the ARP reply.
//
// Parameters:
//      ipAddr - the IP address to translate
//      macAddr - where to put the MAC address that was translated
//
// Returns:
//      true if the address translation was successful
//      false is the address translation was not successful
//
int BcmLnxSocket::TranslateIPToMAC(  const BcmIpAddress  &ipAddr,
												 BcmMacAddress &macAddr
											 )
{
	BcmIpAddress myIpAddr;
	BcmMacAddress myMacAddr;

	//int rc;
	//struct ifreq interfaceRequest;

	GetMyIPAddress(myIpAddr);

	gInfoMsg(fMessageLogSettings,"TranslateIpToMac") << "My IP address is " << myIpAddr << endl;
	gInfoMsg(fMessageLogSettings,"TranslateIpToMac") << "Dest IP address is " << ipAddr << endl;


	// Do a quick check to see if this is our IP address; if so, then we already
	// know the MAC address, so just copy it and return.
	if (ipAddr == myIpAddr)
	{
		GetMyMACAddress(myMacAddr);

		// set the MAC address to my MAC address
		macAddr = myMacAddr;
		return true;
	}
	{
		int  result, numberOfAttempts;
		struct arpreq arpRequest;
		char buffer[] = "1234567890123567890";
		struct sockaddr_in destination;
		uint32 subnet_mask, ip_addr, my_ip;
		BcmIpAddress mySubnetMask;
		BcmIpAddress routerIpAddr;
		BcmIpAddress dstIpAddr;
		struct sockaddr_in *sin;
		uint32 ipaddress;

		// save the destination IP address in a temp variable.
		dstIpAddr = ipAddr;

		GetMySubnetMask(mySubnetMask);

		gInfoMsg(fMessageLogSettings,"TranslateIpToMac") << "mySubnetMask = " << mySubnetMask << endl;

		// get the uint32 representations of my IP, dest IP, and subnet mask
		mySubnetMask.Get(subnet_mask);
		myIpAddr.Get(my_ip);
		ipAddr.Get(ip_addr);

		// Check to see if the target IP address is on-net or off-net; if off
		// net, then I need to use the default router/gateway's IP addr instead.
		if ( (ip_addr & subnet_mask) != (my_ip & subnet_mask) )
		{
			// the destination is OFF net
			BcmIpAddress nullIP;
			GetRouterIpAddress(routerIpAddr);

			nullIP.Set("0.0.0.0");

			gInfoMsg(fMessageLogSettings,"TranslateIpToMac") << "routerIpAddr = " << routerIpAddr << endl;

			// check if the router is is NULL
			if (!(routerIpAddr == nullIP) )
			{
				// swap destination with my router
				dstIpAddr = routerIpAddr;
			}
			else
			{
				// we do not have a default gateway, and the destination is off net, fail
				return false;
			}
		}

		// Pick a dummy port number that nobody is likely to be using.
		memset(&destination, 0, sizeof(destination));
		destination.sin_family = AF_INET;
		destination.sin_port = 65000;

		dstIpAddr.NetworkGet(ipaddress);
		destination.sin_addr.s_addr = ipaddress;

		sin = (struct sockaddr_in *)(&arpRequest.arp_pa);

		// Make several attempts, if necessary.
		for (numberOfAttempts = 0; numberOfAttempts < 6; numberOfAttempts++)
		{
			// Initialize the arp request struct.
			memset(&arpRequest, 0, sizeof(arpRequest));

			// Set up the source IP parameters.
			sin->sin_family = AF_INET;
			sin->sin_addr.s_addr = ipaddress;

			// Set up the dest parameters.
			arpRequest.arp_ha.sa_family = AF_UNSPEC;
			sprintf(arpRequest.arp_dev, "eth%d", fInterfaceNumber - 1);

			// This reads the MAC address from the ARP table, if it is there; if
			// not, then it returns non-0 and I have to try again.  This call does
			// not cause an ARP request to be generated...it's simply a table lookup.
			result = ioctl(fSocketDescriptor, SIOCGARP, (char *) &arpRequest);

			if(result != 0)
			{
				gWarningMsg(fMessageLogSettings, "TranslateIpToMac") <<
				"did not get an arp entry! errno: "
				<< errno <<  " IP = " << dstIpAddr << " arp_dev = " << arpRequest.arp_dev << endl;
			}

			// check to see if the IP address is in the table be checking the MAC address
			// for non-zeroness
			if ( (arpRequest.arp_ha.sa_data[0] != 0) ||
				  (arpRequest.arp_ha.sa_data[1] != 0) ||
				  (arpRequest.arp_ha.sa_data[2] != 0) ||
				  (arpRequest.arp_ha.sa_data[3] != 0) ||
				  (arpRequest.arp_ha.sa_data[4] != 0) ||
				  (arpRequest.arp_ha.sa_data[5] != 0)
				)
			{
				// found it.  we are done.

				// Copy the MAC address that was found.
				macAddr.Set( arpRequest.arp_ha.sa_data[0],
								 arpRequest.arp_ha.sa_data[1],
								 arpRequest.arp_ha.sa_data[2],
								 arpRequest.arp_ha.sa_data[3],
								 arpRequest.arp_ha.sa_data[4],
								 arpRequest.arp_ha.sa_data[5]
							  );
				return true;
			}

			// Send a dummy packet to the destination.  This should force ARP to
			// occur.
			if (sendto(fSocketDescriptor, buffer, strlen(buffer) + 1,
						  0, (sockaddr *) &destination, sizeof(destination)) < 0)
			{
				// destination address was invalid due to ARP failure
				gErrorMsg(fMessageLogSettings, "TranslateIpToMac") <<
				"Could not send dummy packet to remote socket! errno: "
				<< errno <<  endl;
			}

			// Sleep, hopefully allowing the packet to be sent, issuing the
			// ARP request, and receiving the ARP response.
			sleep(1);
		}

		// if we tried ARPing 5 times, and still no success, then fail
		gErrorMsg(fMessageLogSettings,"TranslateIpToMac") <<
		"Error - could not get destination MAC address" << endl;
		return false;
	}
}


// This routine will query the protocol stack to determine the assigned IP address
//
// Parameters:
//      myIPAddr - the object to write the Ip address into
int BcmLnxSocket::GetMyIPAddress(BcmIpAddress &myIpAddr)
{

	int rc;
	struct ifreq interfaceRequest;
	struct sockaddr_in *pSocketAddress;

	memset((char *) &interfaceRequest, 0, sizeof(interfaceRequest));

	sprintf(interfaceRequest.ifr_name,"eth%d", (fInterfaceNumber - 1));

	gInfoMsg(fMessageLogSettings, "GetMyIpAddress") << "Interface: " << interfaceRequest.ifr_name << endl;

	pSocketAddress = (struct sockaddr_in *) &(interfaceRequest.ifr_addr);
	pSocketAddress->sin_family = AF_INET;

	rc = ioctl(fSocketDescriptor, SIOCGIFADDR, (char *) &interfaceRequest);

	if ( rc != 0)
	{
		gErrorMsg(fMessageLogSettings, "GetMyIPAddress")
		<< "Failed to get my IP address" << endl;
		return false;
	}

	// Your IP addr is here:
	myIpAddr.Set(pSocketAddress->sin_addr.s_addr);

	return true;
}

// This routine will query the protocol stack to determine the assigned MAC address
//
// Parameters:
//      mySubnetMask - the object to write the MAC address into
int BcmLnxSocket::GetMyMACAddress(BcmMacAddress &macAddress)
{
	int socketDescriptor;
	struct ifreq ifReq;
	int rc;
	uint8 *addr;

	// Pre-set ifReq to 0's.
	memset(&ifReq, 0, sizeof(ifReq));

	// Prime the interface for which I am making this request.
	sprintf(ifReq.ifr_name,"eth%d", (fInterfaceNumber - 1));

	// Create a socket to use for the network interface.
	socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketDescriptor >= 0)
	{
		// Get the physical address from the interface.
		rc = ioctl(fSocketDescriptor, SIOCGIFHWADDR, (char *) &ifReq);

		close(socketDescriptor);

		if (rc != 0)
		{
			gErrorMsg(fMessageLogSettings, "GetMyMACAddress")
			<< "Failed to get my MAC address" << endl;
			return false;
		}

		// Everything worked, go ahead and copy
		addr = (uint8 *)&ifReq.ifr_hwaddr.sa_data;
		macAddress.Set(addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);

	}
	else
	{
		gErrorMsg(fMessageLogSettings, "GetMyMACAddress")
		<< "Failed to create my socket!" << endl;

		return false;
	}

	return true;
}

// This routine will query the protocol stack to determine the assigned subnet mask
//
// Parameters:
//      mySubnetMask - the object to write the subnet mask into
int BcmLnxSocket::GetMySubnetMask(BcmIpAddress &mySubnetMask)
{

	int rc;
	struct ifreq interfaceRequest;
	struct sockaddr_in *pSocketAddress;

	memset((char *) &interfaceRequest, 0, sizeof(interfaceRequest));

	// set the interface number appropriately depending on how this socket was created
	sprintf(interfaceRequest.ifr_name,"eth%d", (fInterfaceNumber - 1));

	pSocketAddress = (struct sockaddr_in *) &(interfaceRequest.ifr_addr);
	pSocketAddress->sin_family = AF_INET;

	rc = ioctl(fSocketDescriptor, SIOCGIFNETMASK, (char *) &interfaceRequest);

	if (rc != 0)
	{
		gErrorMsg(fMessageLogSettings, "GetMySubnetMask")
		<< "Failed to get my subnet mask!  " << errno << endl;
		perror("GetMySubnetMask");
		return false;
	}

	// Your IP addr is here:
	mySubnetMask.Set(pSocketAddress->sin_addr.s_addr);

	return true;
}


//
// This is a somewhat indirect way of getting the systems default gateway
// by dumping and analyzing the protocol stack's routing table
//
// Parameters:
//      routerIPAddress - the object to write the router IP address into
//
int BcmLnxSocket::GetRouterIpAddress(BcmIpAddress &routerIpAddr)
{

	in_addr_t defGatewayAddr = 0;

#ifdef NOTYET
	ipRoute.DefaultGateway(defGatewayAddr);
#endif
	routerIpAddr.Set(defGatewayAddr);
#ifdef NOTDEF
	struct mib_iproutereq *routes; /* the array of routes */
	struct mib_args arg;
	int num_routes, len, i, rc;
	num_routes = 20;	  // default number of routes in array 
	BcmOctetBuffer octetBuffer;

	routes = NULL;		  // to insure it is not freed before it is allocated 
							  // loop until enough memory is allocated to hold all routes 


	// allocate memory for up to 20 entries in the route table. If does not turn out 
	// to be enough, then loop back and double the number of routes until it is
	do 
	{
		if (routes)
		{
			free(routes);		  // free memory from previous iteration 
			num_routes *= 2;	  // allocate more space for the next try 
		}
		len = sizeof(struct mib_iproutereq) * num_routes;
		// number of bytes 
		routes = (struct mib_iproutereq *)malloc(len);

		// array itself 
		arg.len = len;
		arg.buffer = (char *)routes;
		rc = ioctl(fSocketDescriptor, SIOCGIPROUTETABLE, (char *)&arg);
		if ( rc != 0)
		{
			free(routes);
			gErrorMsg(fMessageLogSettings, "GetMyRouterIp")
			<< "Failed to get my router IP  " << errno << endl;
			return false;
		}
	} while (arg.len == len); // if full there may be more routes 

	num_routes = arg.len / sizeof(struct mib_iproutereq);

	// now loop through the route table until I find an entry with a 0 destination
	// IP address.  This entry contains the default gateway's IP 
	for (i = 0; i < num_routes; i++)
	{
		// found the default gateway?
		if (routes[i].ir_idest.s_addr == 0)
		{
			routerIpAddr.Set(routes[i].ir_nexthop.s_addr);
			break;		// as soon as we find this entry, we are done
		}
	}

	// free the storage allocate for the route table
	free(routes);
#endif

#ifdef NOTDEF
	FILE *in;
	char line [256];
	struct rtentry **rthead=0;
	int rtsize=0, rtallocate=0;
	struct rtentry *rt;
	char name[16], temp[16];

	rthead = (struct rtentry **) calloc(100, sizeof(struct rtentry *));
	if (! rthead)
	{
		gErrorMsg(fMessageLogSettings, "GetMyRouterIp")
		<< "Failed to calloc for route table" << errno << endl;
		return false;
	}
	rtallocate = 100;

	/*
	 * fetch routes from the proc file-system:
	 */

	rtsize = 0;

	if (! (in = fopen ("/proc/net/route", "r")))
	{
		gErrorMsg(fMessageLogSettings, "GetMyRouterIp")
		<< "Failed to open /proc/net/route" << errno << endl;
		return false;
	}

	while (fgets (line, sizeof(line), in))
	{
		struct rtentry rtent;
		char rtent_name [32];
		int refcnt, flags, metric;
		unsigned use;

		rt = &rtent;
		memset ((char *) rt,(0), sizeof(*rt));
		rt->rt_dev = rtent_name;

		/*
		 * as with 1.99.14:
		 * Iface Dest GW Flags RefCnt Use Metric Mask MTU Win IRTT
		 * eth0 0A0A0A0A 00000000 05 0 0 0 FFFFFFFF 1500 0 0 
		 */
		if (8 != sscanf (line, "%s %x %x %x %u %d %d %x %*d %*d %*d\n",
							  rt->rt_dev,
							  &(((struct sockaddr_in *) &(rtent.rt_dst))->sin_addr.s_addr),
							  &(((struct sockaddr_in *) &(rtent.rt_gateway))->sin_addr.s_addr),
/* XXX: fix type of the args */
							  &flags, &refcnt, &use, &metric,
							  &(((struct sockaddr_in *) &(rtent.rt_genmask))->sin_addr.s_addr)))
			continue;

		strcpy (name, rt->rt_dev);
		/* linux says ``lo'', but the interface is stored as ``lo0'': */
		if (! strcmp (name, "lo"))
			strcat (name, "0");

		name[15] = '\0';

		rt->rt_flags = flags, rt->rt_refcnt = refcnt;
		rt->rt_use = use, rt->rt_metric = metric;

		Interface_Scan_Init();
		while (Interface_Scan_Next((short *)&rt->rt_unit, temp, NULL, NULL) != 0)
			if (strcmp(name, temp) == 0) break;

			/*
			 *	Allocate a block to hold it and add it to the database
			 */
		if (rtsize >= rtallocate)
		{
			rthead = (struct rtentry **) realloc((char *)rthead, 
															 2 * rtallocate * sizeof(struct rtentry *));
			memset(&rthead[rtallocate], 0, rtallocate 
					 * sizeof(struct rtentry *));
			rtallocate *= 2;
		}
		if (! rthead[rtsize])
			rthead[rtsize] = (struct rtentry *) malloc(sizeof(struct rtentry));
		/*
		 *	Add this to the database
		 */
		memcpy( (char *)rthead[rtsize],(char *)rt, sizeof(struct rtentry));
		rtsize++;
	}

	fclose (in);
#endif

	return true;
}

bool BcmLnxSocket::BindSocket2Interface(void)
{
	struct ifreq ifReq;

	if( (fInterfaceNumber < BCM_MIN_NETIF_NUM) || (fInterfaceNumber > BCM_MAX_NETIF_NUM)
	    && fInterfaceNumber != 0)
	{
		return false;
	}

	// set to 0's.
	memset(&ifReq, 0, sizeof(ifReq));

	// Prime the interface for which I am making this request.
	if (fInterfaceNumber)
		sprintf(ifReq.ifr_name,"eth%d", (fInterfaceNumber - 1));
	else
		sprintf(ifReq.ifr_name,"lo");

	if (setsockopt(fSocketDescriptor, SOL_SOCKET, SO_BINDTODEVICE, (char *)&ifReq, sizeof(ifReq)) < 0)
	{
		gErrorMsg(fMessageLogSettings, "BindSocket2Interface")
		<< "Could not set SO_BINDTODEVICE to " << ifReq.ifr_name << " fInterfaceNumber = " << fInterfaceNumber << endl;

		return false;
	}

	return true;
}

