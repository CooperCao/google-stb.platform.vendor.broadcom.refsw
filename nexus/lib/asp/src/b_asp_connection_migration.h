/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/tcp.h>
#include <netinet/in.h>

#include "bstd.h"
#include "bkni.h"

typedef struct tcpState
{
    uint32_t            seq;
    uint32_t            ack;
    uint32_t            maxSegSize;
    uint32_t            senderTimestamp;
    struct tcp_info     tcpInfo;
} TcpState;

#define B_ASP_INTERFACE_NAME_SIZE   16
#define B_ASP_ETHER_ADDR_LEN        6
typedef struct B_AspSocketState
{
    bool                connectionLost;
    TcpState            tcpState;
    struct sockaddr_in  remoteIpAddr;
    unsigned char       remoteMacAddress[B_ASP_ETHER_ADDR_LEN];
    bool                remoteOnLocalHost;              /* true if the remote happens to be on the same node. */
    char                remoteInterfaceName[IFNAMSIZ];  /* Name of the interface corresponding to the remote IP. */
    int                 remoteInterfaceIndex;
    struct sockaddr_in  localIpAddr;
    struct sockaddr_in  aspIpAddr;
    unsigned char       localMacAddress[B_ASP_ETHER_ADDR_LEN];
    char                interfaceName[IFNAMSIZ];        /* Name of the interface behind which remote node is connected. */
    int                 interfaceIndex;
    char                masterInterfaceName[IFNAMSIZ];  /* Name of a Linux bridged device which contains multiple interfaces in it. */
                                                        /* interfaceName variable points to the real interface. */
    int                 masterInterfaceIndex;
    int                 savedErrno;
} B_AspSocketState;

/* Retrieve & return TCP State associated with the socket. Caller shouldn't use the sockFd after this call. */
int B_Asp_GetSocketStateFromLinux(
    int                 socketFd,               /* in: fd of socket to be offloaded from host to ASP. */
    B_AspSocketState    *pSocketState           /* out: associated socket state. */
    );
/* Using the caller provided socket state, create a new TCP socket and return it. Caller can then continue using this socket. */

int B_Asp_SetSocketStateToLinux(
    B_AspSocketState    *pSocketState,          /* in:  socket state to use to create the new socket. */
    int                 fdMigratedConnx,        /* in: fd associated w/ the migrated connection. */
    int                 *pOutSocketFd           /* out: fd of newly created socket. */
    );

int B_Asp_GetSocketAddrInfo(
    int                         socketFd,               /* in: fd of socket to be offloaded from host to ASP. */
    B_AspSocketState    *pSocketState           /* out: associated socket address info. */
    );

int B_Asp_GetInterfaceNameAndMacAddress(
    B_AspSocketState     *pSocketState,       /* Interface name & MAC address' is returned in the pSocketState */
    int                  aspNetFilterDrvFd
    );

int B_Asp_AddFlowClassificationRuleToNwSwitch(
    B_AspSocketState    *pSocketState,
    int                 aspSwitchPortNumber,
    int                 aspSwitchQueueNumber,
    int                 *ruleIndex              /* out */
    );

int B_Asp_DeleteFlowClassificationRuleFromNwSwitch(
    B_AspSocketState    *pSocketState,
    int                 ruleIndex
    );

int B_Asp_UpdateInterfaceName(
    B_AspSocketState    *pSocketState,
    char                *pInterfaceName
    );
