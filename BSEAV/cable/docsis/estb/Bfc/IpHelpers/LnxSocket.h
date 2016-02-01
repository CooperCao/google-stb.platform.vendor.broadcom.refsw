//****************************************************************************
//
//  Copyright (c) 2001  Broadcom Corporation
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
//  Filename:       LnxSocket.h
//  Author:         Maurice Turcotte
//  Creation Date:  November 5, 2001
//
//****************************************************************************
//  Description:
//      This is the implementation of the sockets wrapper class for PSOS
//
//****************************************************************************

//********************** Include Files ***************************************

#ifndef BCM_PSOS_SOCKET_H
#define BCM_PSOS_SOCKET_H

// My api and definitions...
#include "typedefs.h"
#include "BcmSocket.h"
#include "SocketSet.h"
#include "IpAddress.h"
#include "MacAddress.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


class BcmLnxSocket : public BcmSocket
{

public:

    // Constructor.
    //
    // Parameters:
    //      domain - AF_INET
    //      type - SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
    //      protocol - socket protocol, IPPROTO_TCP, IPPROTO_UDP etc.
    //
    // Returns:  N/A
    //
    BcmLnxSocket(
                        int domain,     // address family (for example, AF_INET)
                        int type,       // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
                        int protocol,   // socket protocol
                        int instance    // the stack instance
                 );

    // Constructor
    //
    // Parameters:
    //      socketDescriptor - socket descriptor that you would like this class to wrap.
    //      interfaceNumber  - interface number that this socket applies to.
    //
    // Returns:  N/A
    //
    BcmLnxSocket(
                                    int socketDescriptor,   // the socket to be wrapped
                                    int interfaceNumber     // the stack instance
                                );


    // Destructor.  Cleans things up
    //
    // Parameters:  none
    //
    // Returns:  N/A
    //
    virtual ~BcmLnxSocket();

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
    virtual int Create (
                                int domain,         // address family (for example, AF_INET)
                                int type,           // SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW
                                int protocol,       // socket protocol
                                int instance = 0    // specifies the 1st or 2nd protocol stack instance
                        );

    // This method closes the socket.  Can be re-established with the Create call.
    //
    // Parameters:
    //      none.
    //
    // Returns:
    //      0 on success, -1 on failure.
    //
    virtual int Close ();

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
    virtual int Bind(
                        struct sockaddr * name,                 // name to be bound
                        int namelen                             // length (sizeof) of name field
                    );


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
    virtual int Listen         (
                                int backlog                     // number of connections to queue */
                               );



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
    virtual int Connect(
                            struct sockaddr * name,             // addr of socket to connect
                            int               namelen           // length of name, in bytes
                       );



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
    virtual BcmSocket * Accept(
                                struct sockaddr * addr,         // peer address
                                int *             addrlen       // peer address length
                      );

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
    virtual int Send(
                        char * buf,    /* pointer to buffer to transmit */
                        int    bufLen, /* length of buffer */
                        int    flags   /* flags to underlying protocols */
                    );

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

    virtual int Sendto         (
                                char *            buf,          // pointer to data buffer
                                int               bufLen,       // length of buffer
                                int               flags,        // flags to underlying protocols
                                struct sockaddr * to,           // recipient's address
                                int               tolen         // length of to sockaddr
                               );

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

    virtual int Recv           (
                                char * buf,                     // buffer to write data to
                                int    bufLen,                  // length of buffer
                                int    flags                    // flags to underlying protocols
                               );


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
    virtual int Recvfrom       (
                                    char *            buf,          // pointer to data buffer
                                    int               bufLen,       // length of buffer
                                    int               flags,        // flags to underlying protocols
                                    struct sockaddr * from,         // where to copy sender's addr
                                    int *             pFromLen      // value/result length of from
                               );


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
    virtual int Setsockopt     (
                                int    level,                   // protocol level of option
                                int    optname,                 // option name
                                char * optval,                  // pointer to option value
                                int    optlen                   // option length
                               );

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
    virtual int Getsockopt     (
                                int    level,                   // protocol level for options
                                int    optname,                 // name of option
                                char * optval,                  // where to put option
                                int *  optlen                   // where to put option length
                               );

    // This method gets the current name for the specified socket s. The parameter namelen
    // should be initialized to indicate the amount of space referenced by name. On return,
    // the name of the socket is copied to name and the actual size of the socket name is
    // copied to namelen.
    //
    // Parameters:
    //      name - pointer to the name
    //      namelen - space available for the name field
    //
    // Returns:
    //      OK, or ERROR if the socket is invalid or not connected.
    virtual int Getsockname    (
                                struct sockaddr * name,         // where to return name
                                int *             namelen       // space available in name, later filled in actual name size
                               );

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
     virtual int Getpeername   (
                                struct sockaddr * name,         // where to put name
                                int *             namelen       // space available in name, later filled in actual name size
                               );


    // This method shuts down all, or part, of a connection-based socket s. If the value of how is 0,
    // receives are disallowed. If how is 1, sends are disallowed. If how is 2, both sends and receives
    // are disallowed.
    //
    // Parameters:
    //
    // Returns:
    //      OK, or ERROR if the socket is invalid or not connected.
    virtual int Shutdown       (
                                int how                         // 0 = receives disallowed 1 = sends disallowed 2 = sends and disallowed
                               );


    // This routine will return the MAC address of the specified IP address contained in the
    // sockaddr argument.  It will block the calling task while it periodically polls the
    // protocol stacks ARP cache looking for the ARP reply.
    //
    // Parameters:
    //      ipAddr - the IP address to translate
    //      macAddr - where to put the MAC address that was translated
    //
    // Returns:
    //      kTrue if the address translation was successful
    //      kFalse is the address translation was not successful
    //

    virtual int TranslateIPToMAC(   const BcmIpAddress  &ipAddr,
                                    BcmMacAddress &MacAddr
                                );

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
    virtual int SetSocketNonBlocking(bool non_blocking);

    // This routine checks if there is data ready to be read
    // from the socket receive buffer
    //
    // Parameters:
    //      None.
    //
    // Returns:
    //      true if there is data in the receive buffer ready to be read
    //      false if the socket read buffer is empty
    virtual bool ReceiveBufferHasData(void);

protected:

    // This routine will query the protocol stack to determine the assigned IP address
    //
    // Parameters:
    //      myIPAddr - the object to write the Ip address into
    int GetMyIPAddress(BcmIpAddress &myIpAddr);

    // This routine will query the protocol stack to determine the assigned subnet mask
    //
    // Parameters:
    //      mySubnetMask - the object to write the subnet mask into
    int GetMySubnetMask(BcmIpAddress &mySubnetMask);

    // This routine will query the protocol stack to determine the assigned router IP address
    //
    // Parameters:
    //      routerIPAddress - the object to write the router IP address into
    int GetRouterIpAddress(BcmIpAddress &routerIpAddr);

    // This routine will query the protocol stack to determine the assigned MAC address
    //
    // Parameters:
    //      mySubnetMask - the object to write the MAC address into
    int GetMyMACAddress(BcmMacAddress &myMacAddr);

    // This routine binds the my socket descriptor to my network interface
    //
    // Parmameters: None
    //		
    bool BindSocket2Interface(void);

    // This routine binds the my socket descriptor to my network VRF
    //
    // Parmameters: None
    //		
    bool BindSocket2VRF(void);
};
#endif
