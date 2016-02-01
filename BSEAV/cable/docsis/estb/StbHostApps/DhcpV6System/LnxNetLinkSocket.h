//****************************************************************************
//
// Copyright (c) 2009 Broadcom Corporation
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
//
//  Filename:       LnxNetLinkSocket.cpp
//  Author:         Weimin Feng
//  Creation Date:  August 28, 2007
//
//****************************************************************************
//  Description:
//      This class provides the NetLink interface to communicate with Linux 
//		Kernel.
//
//****************************************************************************

#ifndef LNXNETLINKSOCKET_H
#define LNXNETLINKSOCKET_H

//********************** Include Files ***************************************
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "MessageLog.h"

//********************** Global Types ****************************************
typedef struct _RT_REQ_
{
	struct nlmsghdr 	n;
	struct rtmsg 		r;
	char   			buf[1024];
} RT_REQ;

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((uint32) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))


//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

//********************** Class Declaration ***********************************


// PR2256 - the base class was changed, and many methods were moved there, since
// they are common among a number of IpHalIf classes.

class BcmLnxNetLinkSocket
{
public:

    // Initializing Constructor.  Stores the information specified.
    //
    // Parameters: N/A
    //
    // Returns:  N/A
    //
    BcmLnxNetLinkSocket();

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmLnxNetLinkSocket();

public:  // Various accessors.

    // Talk to kernel to get work done
	//
    // Parameters:
    //      p - the RT request to send
    //
    // Returns:
    //      true if okay
    //      false otherwise 
    //
	bool Talk(RT_REQ * p);

protected:

	int			fSocket;
	struct sockaddr_nl	fLocalAddr;
	struct sockaddr_nl	fPeerAddr;
	unsigned int		fSeq;
	char * pfBuf;

	// For controlling message log output.
    BcmMessageLogSettings fMessageLogSettings;

private:

   // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmLnxNetLinkSocket(const BcmLnxNetLinkSocket &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmLnxNetLinkSocket & operator = (const BcmLnxNetLinkSocket &otherInstance);

	// Dump the routing request
	//
	void BcmDumpNetLinkRequest(RT_REQ * prq);
};

#endif


