/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef __ASP_DRIVER_H__
#define __ASP_DRIVER_H__

#include <asm/types.h>
#include <linux/types.h>

#define ASP_DEVICE_NAME                 "brcm_asp"  /* Device name that shows up in /dev, /proc/devices, sysfs, etc. */
#define ASP_CLASS_NAME                  "brcm_asp"
#define ASP_MINOR_DEVICES_COUNT         1
#define ASP_DEVICE_MAGIC_TYPE           'Z'

/* Command to ADD SOCKET_5TUPLE_INFO */
typedef enum
{
    ASP_ChLayer3IpVersion_eIpv4,
    ASP_ChLayer3IpVersion_eIpv6,
    ASP_ChLayer3IpVersion_eMax
} ASP_ChLayer3IpVersion;
typedef enum
{
    ASP_ChLayer4Protocol_eTCP,
    ASP_ChLayer4Protocol_eUDP,
    ASP_ChLayer4Protocol_eMax
} ASP_ChLayer4Protocol;
typedef enum
{
    ASP_RxFrameHdrOffset_eL2,          /* L2: MAC header onwards. */
    ASP_RxFrameHdrOffset_eL3,          /* L3: IP header onwards. */
    ASP_RxFrameHdrOffset_eL4,          /* L4: TCP/UDP header onwards. */
    ASP_RxFrameHdrOffset_eL4Payload,   /* L4Payload: payload of TCP or UDP. */
    ASP_RxFrameHdrOffset_eMax
} ASP_RxFrameHdrOffset;
typedef struct ASP_Socket5TupleInfo
{
    ASP_ChLayer3IpVersion       ipVersion;
    ASP_ChLayer4Protocol        l4Protocol;
    uint32_t                    dstPort;
    uint32_t                    srcPort;
    uint32_t                    dstIpAddr[4];
    uint32_t                    srcIpAddr[4];
    uint32_t                    aspIpAddr[4];
    ASP_RxFrameHdrOffset        rxFrameHdrOffset;
} ASP_Socket5TupleInfo;

#define ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO      _IOW(ASP_DEVICE_MAGIC_TYPE, 1, ASP_Socket5TupleInfo )

/* Command to DELETE the current SOCKET_5TUPLE_INFO added to this ASP Channel. */
#define ASP_CHANNEL_IOC_DEL_SOCKET_5TUPLE_INFO      _IO(ASP_DEVICE_MAGIC_TYPE,  2)

/* Command to Get the current SOCKET_5TUPLE_INFO added to this ASP Channel. */
#define ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO      _IOR(ASP_DEVICE_MAGIC_TYPE, 3, ASP_Socket5TupleInfo)

/* Command to Get the ASP Channel Status. */
typedef struct ASP_ChannelStatus
{
    uint64_t                    totalIpPktsFiltered;
    uint32_t                    totalChannelsOpened;
} ASP_ChannelStatus;
#define ASP_CHANNEL_IOC_GET_STATUS                  _IOR(ASP_DEVICE_MAGIC_TYPE, 4, ASP_ChannelStatus)

/* Command to Get the ASP Device Status. */
typedef struct ASP_DeviceStatus
{
    uint64_t                    totalIpPktsFiltered;
    uint32_t                    totalSocket5TupleAdded;
} ASP_DeviceStatus;
#define ASP_DEVICE_IOC_GET_STATUS                   _IOR(ASP_DEVICE_MAGIC_TYPE, 5, ASP_DeviceStatus)

/* Command to control pkt accept or drop behavior. */
typedef enum
{
    ASP_PktMatchAction_eDropAllPkts,    /* Default: Drop all matching packets for this flow. */
    ASP_PktMatchAction_eDropOnePkt,     /* Drop just 1 matching packet for this flow & let the rest of the packets thru. */
    ASP_PktMatchAction_eDropNPkts,      /* Drop next N matching packets & then let the rest of the packets thru. */
    ASP_PktMatchAction_eDropNthPkts,    /* Drop pkt# N & then let the rest of the packets thru. */
    ASP_PktMatchAction_eDropPktRate,    /* Randomly drop packets at a certain rate: drop x% of incoming packes. */
    ASP_PktMatchAction_eRandomDrops,    /* Randomly drop packets here & n there: TODO: needs more definition. */
    ASP_PktMatchAction_eNoDrops,        /* Accept all incoming packets. */
    ASP_PktMatchAction_eMax
} ASP_PktMatchAction;
typedef struct ASP_ChannelPktControl
{
    ASP_PktMatchAction          action;
    uint32_t                    numPktsToDrop;      /* Number of packets to drop. */
    uint32_t                    pktPositionToDrop;  /* Drop packet from this packet# */
} ASP_ChannelPktControl;
#define ASP_CHANNEL_IOC_PKT_CONTROL                 _IOW(ASP_DEVICE_MAGIC_TYPE, 6, ASP_ChannelPktControl)

typedef struct ASP_DeviceGetGateway
{
    uint32_t                    remoteIpAddr[4];
    uint32_t                    routeUsesGateway;
    uint32_t                    gatewayIpAddr[4];
} ASP_DeviceGetGateway;
#define ASP_DEVICE_IOC_GET_GATEWAY                  _IOWR(ASP_DEVICE_MAGIC_TYPE, 7, ASP_DeviceGetGateway)


#define ASP_DEVICE_IOC_MAXNR                        7

#define IPv4(addr) \
    ((unsigned char *)&(addr))[0], \
    ((unsigned char *)&(addr))[1], \
    ((unsigned char *)&(addr))[2], \
    ((unsigned char *)&(addr))[3]

#define MacAddrPrintFmt \
    "macAddr: %2x:%2x:%2x:%2x:%2x:%2x"

#define MacAddrPrintArgs(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3], \
    ((unsigned char *)&addr)[4], \
    ((unsigned char *)&addr)[5]

#endif /* __ASP_DRIVER_H__ */
