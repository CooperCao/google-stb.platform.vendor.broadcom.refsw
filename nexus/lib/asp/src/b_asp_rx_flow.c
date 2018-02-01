/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <linux/sockios.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <unistd.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/ethtool.h>

#if 0
ETHTOOL_SRXCLSRLINS
Receive (RX) Classification (CLS) Rule (RL) Inserting (INS)
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bstd.h"
#include "bkni.h"
#include "b_asp_lib_impl.h"
BDBG_MODULE(b_asp_lib);

B_Error B_AspChannel_DeleteFlowClassificationRuleFromNwSwitch(
    B_AspChannelSocketState     *pSocketState,
    int                          ruleIndex
    )
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;

    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_SRXCLSRLDEL;
    rule.flow_type = TCP_V4_FLOW;

    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.location = ruleIndex;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        int myerrno = errno;
        BDBG_ERR(("%s:%u ioctl() socket() failed, errno=%d", BSTD_FUNCTION, BSTD_LINE, myerrno));
        BDBG_ASSERT(false);
        return B_ERROR_UNKNOWN;
    }

    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pSocketState->interfaceName, IF_NAMESIZE);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0) BDBG_WRN(("%s: ETHTOOL_GRXCLSRULE ioctl failed: errno=%d", BSTD_FUNCTION, errno));

    BDBG_WRN(("%s: ruleIdx=%d deleted", BSTD_FUNCTION, rule.fs.location));

    close(sock);
    return B_ERROR_SUCCESS;
}

int B_AspChannel_AddFlowClassificationRuleToNwSwitch(
    B_AspChannelSocketState     *pSocketState,
    int                         aspSwitchPortNumber,
    int                         aspSwitchQueueNumber,
    int                         *ruleIndex              /* out */
    )
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;

    /*
     * Example ethtool string (converted to ioctl below):
     * ethtool --config-nfc gphy flow-type udp4 src-ip 192.168.1.1 dst-ip 192.168.1.201 src-port 41262 dst-port 5001 action 7
     * Note: gphy is the interface/device for which this rule applies! Kernel will map this name to the switch port # on which to set this CFP rule.
     * action: implies the port number to which the CFP classified packets are forwarded to (it is 7 in this example above).
     *
     * Also, note that rule is setup with view of incoming packet in mind. The rule instructs the switch to redirect those  packets to ASP.
     * Such packets will have local IP & Port as the destination & remote IP & Port as the source.
     */
    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_SRXCLSRLINS;
    rule.flow_type = TCP_V4_FLOW;

#define PORT_NUMBER_MULTIPLER   8   /* Since we only have one field (ring_cookie) to indicate both Port & Q #s to Linux, we use this multiplier for identifying the port number. */
    rule.fs.ring_cookie = aspSwitchPortNumber * PORT_NUMBER_MULTIPLER + aspSwitchQueueNumber;
    rule.fs.location = RX_CLS_LOC_ANY;
    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.h_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;
    rule.fs.h_u.tcp_ip4_spec.ip4src = pSocketState->remoteIpAddr.sin_addr.s_addr;

    if (pSocketState->remoteIpAddr.sin_addr.s_addr == pSocketState->localIpAddr.sin_addr.s_addr)
    {
        rule.fs.h_u.tcp_ip4_spec.ip4dst = pSocketState->aspIpAddr.sin_addr.s_addr;
    }
    else
    {
        rule.fs.h_u.tcp_ip4_spec.ip4dst = pSocketState->localIpAddr.sin_addr.s_addr;
    }
    rule.fs.h_u.tcp_ip4_spec.psrc = pSocketState->remoteIpAddr.sin_port;
    rule.fs.h_u.tcp_ip4_spec.pdst = pSocketState->localIpAddr.sin_port;

    /* Set the masks. */
    rule.fs.m_u.tcp_ip4_spec.ip4src = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.ip4dst = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.psrc = 0xffff;
    rule.fs.m_u.tcp_ip4_spec.pdst = 0xffff;
    rule.fs.m_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        int myerrno = errno;
        BDBG_ERR(("%s:%u ioctl() socket() failed, errno=%d", BSTD_FUNCTION, BSTD_LINE, myerrno));
        BDBG_ASSERT(sock != -1);
        return 1;
    }

    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pSocketState->interfaceName, IF_NAMESIZE);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0)
    {
        BDBG_ERR(("%s: ioctl() SIOCETHTOOL Failed to add the Classification rule to network switch, errno=%d", BSTD_FUNCTION, errno));
        perror("ioctl: ");
    }
    BDBG_ASSERT(rc >= 0);
    BDBG_WRN(("%s: IP:Port local=0x%x:%d, remote=0x%x:%d, switchFwdPort#=%d, switchFwdPortQ#=%d ring_cookie=%d ruleIdx=%d", BSTD_FUNCTION,
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4src),
                ntohs(rule.fs.h_u.tcp_ip4_spec.psrc),
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4dst),
                ntohs(rule.fs.h_u.tcp_ip4_spec.pdst),
                aspSwitchPortNumber,
                aspSwitchQueueNumber,
                (int)rule.fs.ring_cookie,
                rule.fs.location
             ));

    *ruleIndex = rule.fs.location;
    close(sock);
    return 0;
}
