//****************************************************************************
//
// Copyright (c) 2007-2009 Broadcom Corporation
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
//**************************************************************************
//    Description:                                                          
//                                                                          
//**************************************************************************
//    Revision History:                                                     
//                                                                          
//**************************************************************************
//    Filename: estbUdpBridge.h
//    Creation Date: 11/14/03
//    Created by Broadcom V2/BFC/BOSS Mib Compiler BCMIBC.EXE version 1.4.0c
//    from input file rfc2013.mib
//
//**************************************************************************

#ifndef ESTBUDPBRIDGE_H
#define ESTBUDPBRIDGE_H


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
// class lnxUdpPort0Snoop;

class lnxUdpGroupBridge : public udpGroupBridge
{
  friend class udpGroup;

  public:
    lnxUdpGroupBridge (BcmSnmpAgent *pAgent);
   ~lnxUdpGroupBridge ();

    /* GET methods */

    virtual unsigned int Get_udpHCInDatagrams (BcmCounter64 &udpHCInDatagrams);
    virtual unsigned int Get_udpNoPorts ();
    virtual unsigned int Get_udpInErrors ();
    virtual unsigned int Get_udpHCOutDatagrams (BcmCounter64 &udpHCInDatagrams);

    /* TEST methods - not required because this is not a READ-CREATE table */


    /* SET methods */
    
    static void UpdateUdpTable (BcmSnmpAgent *pAgent);
    
  protected:
    static bool UdpEntryExists (const BcmIpAddress &udpLocalAddress, int udpLocalPort);
    static void linux_read_udp_stat();
    static struct inpcb*UDP_Scan_Init(void);
    static struct inpcb*UDP_Scan_Next(struct inpcb *TargetInPcb);
    
  protected:
    // static lnxUdpPort0Snoop *fpPort0Snoop;
    static unsigned long   UdpInDatagrams;
    static unsigned long   UdpNoPorts;
    static unsigned long   UdpInErrors;
    static unsigned long   UdpOutDatagrams;
    static struct inpcb *udp_inpcb_list;
};

#endif
