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

//********************** Include Files ***************************************

// My api and definitions...
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <net/route.h>
#include <resolv.h>
#include "LnxNetLinkSocket.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************
#define RT_RESP_BUF_LEN 16384
//********************** Local Variables *************************************

//********************** External Functions **********************************

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


// Initializing Constructor.  Stores the information specified, and registers
// itself with the IP HAL.
//
// Parameters:
//      macAddress - the MAC address that is to be used for this IP stack.
//      pDescription - text string describing the HalIf, for debugging.
//
// Returns:  N/A
//
BcmLnxNetLinkSocket::BcmLnxNetLinkSocket()
{
	socklen_t alen;
	int buf_size = 32768;

    // Override the module name set up by my base class.
    fMessageLogSettings.SetModuleName("BcmLnxNetLinkSocket");

	// Set up the socket.
  	fSocket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  	//fSocket = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	if(fSocket < 0)
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
		<< "Can not get netlink socket!" << endl;
		return;
	}

	// Set TX buf size
	if(setsockopt(fSocket,SOL_SOCKET,SO_SNDBUF,&buf_size,sizeof(buf_size)) < 0)
	{
		close(fSocket);
		fSocket = -1;
	}

	// Set RX buf size
	if(setsockopt(fSocket,SOL_SOCKET,SO_RCVBUF,&buf_size,sizeof(buf_size)) < 0)
	{
		close(fSocket);
		fSocket = -1;
	}

	// Set local addr
	memset(&fLocalAddr, 0, sizeof(fLocalAddr));
	fLocalAddr.nl_family = AF_NETLINK;

	alen = sizeof(fLocalAddr);
	if (getsockname(fSocket, (struct sockaddr*)&fLocalAddr, &alen) < 0) 
	{
		close(fSocket);
		fSocket = -1;
	}
	if (alen != sizeof(fLocalAddr)) 
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
			<< "Address length mismatch " << alen << " expecting " << sizeof(fLocalAddr) <<endl;
		close(fSocket);
		fSocket = -1;
	}

	if (fLocalAddr.nl_family != AF_NETLINK) 
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
			<< "Address family mismatch " << fLocalAddr.nl_family << " expecting " << AF_NETLINK <<endl;
		close(fSocket);
		fSocket = -1;
	}

	// Allocate RX memory
	if((pfBuf = new char[RT_RESP_BUF_LEN]) == NULL)
	{
		close(fSocket);
		fSocket = -1;
	}

	fSeq = time(NULL);
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmLnxNetLinkSocket::~BcmLnxNetLinkSocket()
{
	if(fSocket != -1)
	{
		close(fSocket);		
		if(pfBuf)
		{
			delete pfBuf;
		}
	}
}


// Talk to kernel to get work done
//
// Parameters:
//      p - the RT request to send
//
// Returns:
//      true if okay
//      false otherwise 
//
bool BcmLnxNetLinkSocket::Talk(RT_REQ *prq)
{
	int rc;
	struct sockaddr_nl naddr;
	struct iovec iov;
	struct msghdr msg;

	// dump the request
	//BcmDumpNetLinkRequest(prq);
	//cout << "fSocket = " << fSocket << endl;
	if(fSocket == -1)
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
		<< "invalid socket handle" << endl;
		return false;
	}

	// send the request
	memset(&naddr, 0, sizeof(naddr));
	naddr.nl_family = AF_NETLINK;
	naddr.nl_pid = 0;
	naddr.nl_groups = 0;

	prq->n.nlmsg_seq = ++fSeq;
	prq->n.nlmsg_flags |= NLM_F_ACK;

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void*) &prq->n;
	iov.iov_len = prq->n.nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &naddr;
	msg.msg_namelen = sizeof(naddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if (sendmsg(fSocket, &msg, 0) < 0) 
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
		<< "Can not talk to rt!" << endl;
		perror("sendmsg failed:");
		return false;
	}

	// Re-init the msg & iov
	memset(pfBuf,0,RT_RESP_BUF_LEN);
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &naddr;
	msg.msg_namelen = sizeof(naddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	iov.iov_base = pfBuf;

	// Wait for reply
	while (1) 
	{
		iov.iov_len = RT_RESP_BUF_LEN;
		rc = recvmsg(fSocket, &msg, 0);

//		gAlwaysMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
//			<< "rc = " << rc << endl;

		if(rc == 0)
		{
			gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
				<< "EOF" <<endl;
			return false;
		}

		if(rc < 0) 
		{
			continue;
		}

		// Got a reply
		break;
	}

	if (msg.msg_namelen != sizeof(struct sockaddr_nl)) 
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
			<< "incorrect address length!" <<endl;
		return false;
	}

	if(rc < (int)sizeof(struct nlmsghdr))
	{
		gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
			<< "message too short!" <<endl;
		return false;
	}

	{
		struct nlmsghdr* ptr = (struct nlmsghdr*)pfBuf;
		if(ptr->nlmsg_seq != fSeq)
		{
			gErrorMsg(fMessageLogSettings, "BcmLnxNetLinkSocket")
				<< "sequence number mismatch!" <<endl;
			return false;
		}

		if (ptr->nlmsg_type == NLMSG_ERROR) 
		{
			struct nlmsgerr *err = (struct nlmsgerr*)NLMSG_DATA(ptr);
			errno = -err->error;
			if (errno == 0) 
			{
				return true;
			}
			perror("RTNETLINK answers");
		}
	}

	return false;
}

// Dump the routing request
//
void BcmLnxNetLinkSocket::BcmDumpNetLinkRequest(RT_REQ * prq)
{
	printf("nlmsg_len = %d\n", prq->n.nlmsg_len);
	printf("nlmsg_type = %04x\n", prq->n.nlmsg_type);
	printf("nlmsg_flags = %04x\n", prq->n.nlmsg_flags);
	printf("nlmsg_seq = %08x\n", prq->n.nlmsg_seq);
	printf("nlmsg_pid = %08x\n", prq->n.nlmsg_pid);
	printf("\n");
	printf("rtm_family : %02x\n", prq->r.rtm_family);
	printf("rtm_dst_len : %02x\n", prq->r.rtm_dst_len);
	printf("rtm_src_len : %02x\n", prq->r.rtm_src_len);
	printf("rtm_tos : %02x\n", prq->r.rtm_tos);
	printf("rtm_table : %02x\n", prq->r.rtm_table);
	printf("rtm_protocol : %02x\n", prq->r.rtm_protocol);
	printf("rtm_scope : %02x\n", prq->r.rtm_scope);
	printf("rtm_type : %02x\n", prq->r.rtm_type);
	printf("rtm_flags : %08x\n", prq->r.rtm_flags);
	{
		unsigned int i, len;
		unsigned char *ptr = (unsigned char *)prq->buf;
		len = prq->n.nlmsg_len - sizeof(struct nlmsghdr) - sizeof(struct rtmsg);
		for(i = 0; i < len; i++)
		{
			printf("%02x", *ptr++);
			if(i % 4 == 3)
				printf(" ");
			if(i % 16 == 15)
				printf("\n");
		}
		printf("\n");
	}
}
