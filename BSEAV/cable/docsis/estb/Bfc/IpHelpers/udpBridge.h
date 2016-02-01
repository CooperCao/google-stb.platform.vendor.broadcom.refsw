//**************************************************************************
//                                                                          
//    Copyright 2007 Broadcom Corporation                                   
//    All Rights Reserved                                                   
//    No portions of this material may be reproduced in any form without the
//    written permission of:                                                
//             Broadcom Corporation                                         
//             16251 Laguna Canyon Road                                     
//             Irvine, California  92618                                    
//    All information contained in this document is Broadcom Corporation    
//    company private, proprietary, and trade secret.                       
//                                                                          
//                                                                          
//                                                                          
//**************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: udpBridge.h
//    Creation Date: Monday, April 16, 2007 at 15:13:40
//    Created by Broadcom BFC Mib Compiler BCMIBC.EXE version 3.6.0b
//    from input file udp-mib_rfc4113.mib
//
//**************************************************************************

#ifndef UDPBRIDGE_H
#define UDPBRIDGE_H


#include "CoreObjs.h"
#include "MibObjs.h"
#include "MibBridge.h"

#include "udpMib.h"


/*\
 *  udpGroup 
\*/


class udpGroupBridge : public ScalarMibBridge
{
  friend class udpGroup;

  public:
    udpGroupBridge (BcmSnmpAgent *pAgent);
   ~udpGroupBridge ();

    /* GET methods */

    virtual unsigned int Get_udpInDatagrams ();
    virtual unsigned int Get_udpNoPorts ();
    virtual unsigned int Get_udpInErrors ();
    virtual unsigned int Get_udpOutDatagrams ();
    virtual unsigned int Get_udpHCInDatagrams (BcmCounter64 &udpHCInDatagrams);
    virtual unsigned int Get_udpHCOutDatagrams (BcmCounter64 &udpHCOutDatagrams);

    /* TEST methods - not required */


    /* SET methods */
    
    // Helper methods - base class does nothing, dervied class must override
    virtual void UpdateUdpTable ();
    virtual void UpdateUdpEndpointTable ();
};



/*\
 *  udpEndpointEntry 
\*/

#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_UNKNOWN  0
#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV4  1
#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV6  2
#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV4Z  3
#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_IPV6Z  4
#define CONSTVAL_UDPENDPOINTLOCALADDRESSTYPE_DNS  16
#define CONSTLEN_UDPENDPOINTLOCALADDRESS_UNKNOWN  0
#define CONSTLEN_UDPENDPOINTLOCALADDRESS_IPV4  4
#define CONSTLEN_UDPENDPOINTLOCALADDRESS_IPV6  16
#define MINVAL_UDPENDPOINTLOCALPORT  0
#define MAXVAL_UDPENDPOINTLOCALPORT  65535
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_UNKNOWN  0
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV4  1
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV6  2
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV4Z  3
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_IPV6Z  4
#define CONSTVAL_UDPENDPOINTREMOTEADDRESSTYPE_DNS  16
#define CONSTLEN_UDPENDPOINTREMOTEADDRESS_UNKNOWN  0
#define CONSTLEN_UDPENDPOINTREMOTEADDRESS_IPV4  4
#define CONSTLEN_UDPENDPOINTREMOTEADDRESS_IPV6  16
#define MINVAL_UDPENDPOINTREMOTEPORT  0
#define MAXVAL_UDPENDPOINTREMOTEPORT  65535
#define MINVAL_UDPENDPOINTINSTANCE  1
#define MAXVAL_UDPENDPOINTINSTANCE  'ffffffff'h

class udpEndpointEntryBridge : public TabularMibBridge
{
  friend class udpEndpointEntry;

  public:
    udpEndpointEntryBridge (BcmSnmpAgent *pAgent, const BcmIpAddress &udpEndpointLocalAddress, unsigned long udpEndpointLocalPort, const BcmIpAddress &udpEndpointRemoteAddress = kIPv6UnspecifiedAddress, unsigned long udpEndpointRemotePort = 0, unsigned long udpEndpointInstance = 1);
   ~udpEndpointEntryBridge ();

    /* GET methods */

    int Get_udpEndpointLocalAddressType ();
    int Get_udpEndpointLocalAddress (BcmString &udpEndpointLocalAddress);
    unsigned int Get_udpEndpointLocalPort ();
    int Get_udpEndpointRemoteAddressType ();
    int Get_udpEndpointRemoteAddress (BcmString &udpEndpointRemoteAddress);
    unsigned int Get_udpEndpointRemotePort ();
    unsigned int Get_udpEndpointInstance ();
    unsigned int Get_udpEndpointProcess ();

    /* TEST methods - not required */


    /* SET methods */

  protected:
    BcmIpAddress fLocalAddr;
    unsigned long fLocalPort;
    BcmIpAddress fRemoteAddr;
    unsigned long fRemotePort;
    int fInstance;
};



/*\
 *  udpEntry 
\*/

#define MINVAL_UDPLOCALPORT  0
#define MAXVAL_UDPLOCALPORT  65535

class udpEntryBridge : public TabularMibBridge
{
  friend class udpEntry;

  public:
    udpEntryBridge (const BcmIpAddress &udpLocalAddress, int udpLocalPort, BcmSnmpAgent *pAgent);
   ~udpEntryBridge ();

    /* GET methods */

    void Get_udpLocalAddress (BcmIpAddress &udpLocalAddress);
    int Get_udpLocalPort ();

    /* TEST methods - not required */


    /* SET methods */

    
  protected:
    BcmIpAddress fAddr;
    int fPort;
};



#endif
