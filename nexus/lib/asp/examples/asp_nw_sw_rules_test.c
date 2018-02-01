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
BDBG_MODULE(b_asp_lib);

int get_rule(const char *pInterface, unsigned ruleIndex)
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;
    struct in_addr ipV4Addr;

    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_GRXCLSRULE;
    rule.flow_type = TCP_V4_FLOW;

    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.location = ruleIndex;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock == -1)
    {
        int myerrno = errno;
        BDBG_ERR(("%s:%u ioctl() socket() failed, errno=%d", BSTD_FUNCTION, BSTD_LINE, myerrno));
        BDBG_ASSERT(false);
        return 1;
    }
    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pInterface, IF_NAMESIZE);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0) BDBG_WRN(("%s: ETHTOOL_GRXCLSRULE ioctl failed: errno=%d", BSTD_FUNCTION, errno));

    BDBG_WRN(("%s: local ip:port=0x%x:%d remote=0x%x:%d ruleIdx=%d", BSTD_FUNCTION,
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4src),
                ntohs(rule.fs.h_u.tcp_ip4_spec.psrc),
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4dst),
                ntohs(rule.fs.h_u.tcp_ip4_spec.pdst),
                rule.fs.location
             ));

    return 0;
}

int del_rule(const char *pInterface, unsigned ruleIndex)
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;
    struct in_addr ipV4Addr;

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
        return 1;
    }
    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pInterface, IF_NAMESIZE);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0) BDBG_WRN(("%s: ETHTOOL_GRXCLSRULE ioctl failed: errno=%d", BSTD_FUNCTION, errno));

    BDBG_WRN(("%s: ruleIdx=%d deleted", BSTD_FUNCTION,
                rule.fs.location
             ));

    return 0;
}


static int add_rule(const char *pSrcIp, unsigned srcPort, const char *pDstIp, unsigned dstPort, int aspSwitchPortNumber, const char *pInterface, unsigned *pRuleIndex)
{
    int sock;
    struct ifreq ifr;
    int rc;
    struct ethtool_rxnfc rule;
    struct in_addr ipV4Addr;

    BKNI_Memset(&rule, 0, sizeof(rule));
    rule.cmd = ETHTOOL_SRXCLSRLINS;
    rule.flow_type = TCP_V4_FLOW;
    rule.fs.ring_cookie = aspSwitchPortNumber;
    rule.fs.flow_type = TCP_V4_FLOW;
    rule.fs.location = RX_CLS_LOC_ANY;

    inet_aton(pSrcIp, &ipV4Addr);
    rule.fs.h_u.tcp_ip4_spec.ip4src = ipV4Addr.s_addr;
    inet_aton(pDstIp, &ipV4Addr);
    rule.fs.h_u.tcp_ip4_spec.ip4dst = ipV4Addr.s_addr;
    rule.fs.h_u.tcp_ip4_spec.psrc = htons(srcPort);
    rule.fs.h_u.tcp_ip4_spec.pdst = htons(dstPort);
    rule.fs.h_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;

    rule.fs.m_u.tcp_ip4_spec.ip4src = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.ip4dst = 0xffffffff;
    rule.fs.m_u.tcp_ip4_spec.psrc = 0xffff;
    rule.fs.m_u.tcp_ip4_spec.pdst = 0xffff;
    rule.fs.m_u.usr_ip4_spec.ip_ver = ETH_RX_NFC_IP4;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    BDBG_ASSERT(sock);
    BKNI_Memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pInterface, IF_NAMESIZE);
    ifr.ifr_data = (void *)&rule;
    rc = ioctl(sock, SIOCETHTOOL, &ifr);
    if (rc < 0) BDBG_WRN(("%s: ETHTOOL SIOCETHTOOL ioctl failed: errno=%d", BSTD_FUNCTION, errno));
    BDBG_ASSERT(rc >= 0);
    BDBG_WRN(("%s: ipVer=%s flow_type=%u ring_cookie=%llu loc=%u local ip:port=0x%x:%d remote=0x%x:%d ruleIdx=%d", BSTD_FUNCTION,
                rule.fs.h_u.usr_ip4_spec.ip_ver == ETH_RX_NFC_IP4 ? "IPv4":"IPv6",
                rule.fs.flow_type,
                rule.fs.ring_cookie,
                rule.fs.location,
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4src),
                ntohs(rule.fs.h_u.tcp_ip4_spec.psrc),
                ntohl(rule.fs.h_u.tcp_ip4_spec.ip4dst),
                ntohs(rule.fs.h_u.tcp_ip4_spec.pdst),
                rule.fs.location
             ));

    *pRuleIndex = rule.fs.location;
    return 0;
}

int main(int argc, char *argv[])
{
    int i;
    int rc;
    unsigned ruleIndex1, ruleIndex2;
    unsigned ruleIndex3;
    char interfaceName[IFNAMSIZ];

    strncpy(interfaceName, "gphy", IFNAMSIZ);
    for (i=1; i<argc; i++)
    {
        if ( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help") )
        {
            printf("%s usage: \n", argv[0]);
            printf(" -h[--help]:     print help\n");
            printf(" -i <interfaceName>: name of interface to use for testing.\n");
            exit(0);
        }
        if ( !strcmp(argv[i], "-i") )
        {
            strncpy(interfaceName, argv[++i], IFNAMSIZ);
            printf("interfaceName=%s\n", interfaceName);
        }
    }

    /* Try to delete non-existing rules. */
    {
        rc = get_rule(interfaceName , 1);
        rc = del_rule(interfaceName , 1);
    }

    /* Add multiple rules & then delete one in the middle. */
    {
        rc = add_rule("192.168.2.130", 51234, "192.168.2.11", 5000, 7 /* aspSwitchPortNumber*/, interfaceName , &ruleIndex1);
        rc = add_rule("192.168.2.130", 51235, "192.168.2.11", 5000, 7 /* aspSwitchPortNumber*/, interfaceName , &ruleIndex2);
        rc = add_rule("192.168.2.130", 51236, "192.168.2.11", 5000, 7 /* aspSwitchPortNumber*/, interfaceName , &ruleIndex3);
        rc = del_rule(interfaceName , ruleIndex2);
        rc = get_rule(interfaceName , ruleIndex1);
        rc = get_rule(interfaceName , ruleIndex3);
        rc = del_rule(interfaceName , ruleIndex1);
        rc = del_rule(interfaceName , ruleIndex3);
    }

    /* Add multiple rules & then delete one in the middle. */
    {
        int i;
        int ruleIndex[32];
        for (i=0; i< 32; i++)
        {
            rc = add_rule("192.168.2.130", 51234+i, "192.168.2.11", 5000, 7 /* aspSwitchPortNumber*/, interfaceName , &ruleIndex[i]);
        }
        for (i=0; i< 32; i++)
        {
            rc = get_rule(interfaceName , ruleIndex[i]);
        }
        for (i=0; i< 32; i++)
        {
            rc = del_rule(interfaceName , ruleIndex[i]);
        }
    }
    return 0;
}
