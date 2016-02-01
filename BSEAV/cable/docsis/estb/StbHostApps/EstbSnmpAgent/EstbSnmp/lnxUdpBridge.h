//**************************************************************************
//                                                                          
//    Copyright 2003 Broadcom Corporation                                   
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
//    Creation Date: 11/14/03
//    Created by Broadcom V2/BFC/BOSS Mib Compiler BCMIBC.EXE version 1.4.0c
//    from input file rfc2013.mib
//
//**************************************************************************

#ifndef LNXUDPBRIDGE_H
#define LNXUDPBRIDGE_H

#ifdef BCM_STANDALONE_SNMP_AGENT

#include "estbUdpBridge.h"

#else


#include <sys/socket.h>
#include <netinet/in.h>

#include "udpBridge.h"

struct inpcb {
        struct  inpcb *inp_next;        /* pointers to other pcb's */
        struct  in_addr inp_faddr;      /* foreign host table entry */
        u_short inp_fport;              /* foreign port */
        struct  in_addr inp_laddr;      /* local host table entry */
        u_short inp_lport;              /* local port */
        int     inp_state;
        int     uid;                    /* owner of the connection */
};




/*\
 *  udpGroup 
\*/

// Forward reference:
class lnxUdpPort0Snoop;

class lnxUdpGroupBridge : public udpGroupBridge
{
  friend class udpGroup;

  public:
    lnxUdpGroupBridge (BcmSnmpAgent *pAgent);
   ~lnxUdpGroupBridge ();

    /* GET methods */

    virtual unsigned int Get_udpInDatagrams ();
    virtual unsigned int Get_udpNoPorts ();
    virtual unsigned int Get_udpInErrors ();
    virtual unsigned int Get_udpOutDatagrams ();

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */
    
    static void UpdateUdpTable (BcmSnmpAgent *pAgent);
    
  protected:
    static bool UdpEntryExists (const BcmIpAddress &udpLocalAddress, int udpLocalPort);
    static void linux_read_udp_stat();
    static struct inpcb*UDP_Scan_Init(void);
    static struct inpcb*UDP_Scan_Next(struct inpcb *TargetInPcb);
    
  protected:
    static lnxUdpPort0Snoop *fpPort0Snoop;
    static unsigned long   UdpInDatagrams;
    static unsigned long   UdpNoPorts;
    static unsigned long   UdpInErrors;
    static unsigned long   UdpOutDatagrams;
    static struct inpcb *udp_inpcb_list;
};

#endif

#endif

