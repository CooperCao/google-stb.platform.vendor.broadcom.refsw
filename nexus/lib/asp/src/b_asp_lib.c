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

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <pthread.h>

#include "b_asp_lib.h"
#include "b_asp_lib_impl.h"
#include "asp_netfilter_drv.h"

BDBG_MODULE(b_asp_lib);

#define ASP_IP_ADDR_FOR_LOCAL_STREAMING "192.168.1.250"

/* Global ASP Mgr Context. */
typedef struct B_Asp
{
    int initialized;        /* 0 => not initialized */
    B_AspInitSettings settings;
    char *pAspProxyServerIp;
} B_Asp;
static B_Asp g_aspMgr;      /* Initialized to zero at compile time. */

static pthread_mutex_t g_initMutex = PTHREAD_MUTEX_INITIALIZER;



typedef struct B_AspChannel
{
    B_AspChannelCreateSettings              createSettings;
    B_AspChannelSettings                    settings;
    B_AspChannelState                       state;
    B_AspChannelStatus                      status;
    int                                     fdMigratedConnx;            /* sockFd of migrated socket. */
    NEXUS_AspChannelHandle                  hNexusAspChannel;
    int                                     aspNetFilterDrvFd;
    B_AspChannelSocketState                 socketState;
    int                                     switchFlowLabel;
    int                                     switchPortNumberForAsp;     /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchQueueNumberForAsp;    /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchPortNumberForRemoteNode;

    NEXUS_AspChannelStatus                  nexusStatus;            /* Saved previous Nexus ASP status. */
    int                                     nexusStatusIsValid;     /* true => nexusStatus has been populated. */
    B_Time                                  nexusStatusTime;        /* Time that nexusStatus was acquired. */
    uint32_t                                nexusStatusBitRate;     /* last calculated ASP channel bit-rate. */
} B_AspChannel;

void B_Asp_Uninit(void)
{
    pthread_mutex_lock(&g_initMutex);

    if (g_aspMgr.initialized)
    {
        g_aspMgr.initialized = false;

        if (g_aspMgr.pAspProxyServerIp)
        {
            free(g_aspMgr.pAspProxyServerIp);
            g_aspMgr.pAspProxyServerIp = NULL;
        }

        BKNI_Uninit();
    }

    pthread_mutex_unlock(&g_initMutex);
    return;
}

void B_Asp_GetDefaultInitSettings(
    B_AspInitSettings                       *pSettings
    )
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

B_Error B_Asp_Init(
    const B_AspInitSettings                 *pSettings
    )
{

    B_Error errCode = B_ERROR_SUCCESS;
    int rc;

    rc = pthread_mutex_lock(&g_initMutex);
    if ( rc )
    {
        return B_ERROR_OS_ERROR;
    }

    /* Check for first open */
    if ( g_aspMgr.initialized )
    {
        BDBG_ERR(("Error, B_Asp_Init() has already been called."));
        errCode = B_ERROR_NOT_INITIALIZED;
    }

    if (errCode == B_ERROR_SUCCESS)
    {
        errCode = BKNI_Init();
    }

    if ( errCode == B_ERROR_SUCCESS)
    {
        if (!pSettings)
        {
            B_Asp_GetDefaultInitSettings(&g_aspMgr.settings);
        }
        else
        {
            g_aspMgr.settings = *pSettings;
        }

        /* TODO: Create the Daemon. */
    }

    if ( errCode == B_ERROR_SUCCESS)
    {
        g_aspMgr.initialized = true;
    }

    pthread_mutex_unlock(&g_initMutex);
    return errCode;
}

void B_AspChannel_Destroy(
    B_AspChannelHandle                      hAspChannel,
    int                                     *pSocketFd    /* out: fd of socket offloaded back from ASP to host. */
    )
{
    B_Error rc;

    BDBG_ASSERT(hAspChannel);

    BDBG_WRN(("%s: hAspChannel=%p pSocketFd=%p", BSTD_FUNCTION, (void *)hAspChannel, (void *)pSocketFd));

    /* Call stopStreaming if it hasn't been called. */
    if (hAspChannel->state == B_AspChannelState_eStartedStreaming)
    {
        B_AspChannel_StopStreaming(hAspChannel);
    }

    /* Now get the last Protocol state from Nexus & update final protocol state. */
    if (pSocketFd)
    {
        NEXUS_AspChannelStatus  nexusStatus;

        NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &nexusStatus);
        hAspChannel->socketState.tcpState.seq = nexusStatus.tcpState.finalSendSequenceNumber;
        hAspChannel->socketState.tcpState.ack = nexusStatus.tcpState.finalRecvSequenceNumber;
        BDBG_MSG(("%s: hAspChannel=%p Updating TCP State: seq=%u ack=%u", BSTD_FUNCTION, (void *)hAspChannel, hAspChannel->socketState.tcpState.seq, hAspChannel->socketState.tcpState.ack));
    }

    /* Offload connection back to Linux */
    if ( ! hAspChannel->socketState.connectionLost){
        rc = B_AspChannel_SetSocketStateToLinux(&hAspChannel->socketState, hAspChannel->fdMigratedConnx, pSocketFd);
        BDBG_ASSERT(rc == B_ERROR_SUCCESS);
    }

    /* Close ASP NetFilter Driver. */
    {
        BDBG_MSG(("%s: hAspChannel=%p Closing ASP NetFilterDriver socketFd=%d!", BSTD_FUNCTION, (void *)hAspChannel, hAspChannel->aspNetFilterDrvFd));
        if (hAspChannel->aspNetFilterDrvFd) close(hAspChannel->aspNetFilterDrvFd);
    }

    /* Free up Nexus resources. */
    {
        if (hAspChannel->hNexusAspChannel) NEXUS_AspChannel_Destroy(hAspChannel->hNexusAspChannel);
    }
    BDBG_WRN(("%s:%p Done!", BSTD_FUNCTION, (void *)hAspChannel));
    BKNI_Free(hAspChannel);
}

static int getSwitchPortNumberForAsp(
    void
    )
{
    int switchPortNumberForAsp;

    /* On 7278 A0, ASP device is connected to the Port#7 of the internal StarFighter (SF2) Switch. */
    /* TODO: we should obtain this info from Kernel's sysfs attribute /sys/class/net/<interfaceName>/phys_port_name>. */
    switchPortNumberForAsp = 7;

    return switchPortNumberForAsp;
} /* getSwitchPortNumberForAsp */

static int getSwitchQueueNumberForAsp(
    void
    )
{
    int switchQueueNumberForAsp;

    /* Each Switch Port has 8 Queues pertaining to Quality of Service. */
    /* All frames streamed out from ASP will be assigned to the highest priority queue (which is #7). */
    /* TODO: we should obtain this info from Kernel's sysfs attribute /sys/class/net/<interfaceName>/phys_port_name>. */
    switchQueueNumberForAsp = 7;

    return switchQueueNumberForAsp;
} /* getSwitchQueueNumberForAsp */

static int getSwitchPortNumberForRemoteNode(
    B_AspChannelSocketState   *pSocketState
    )
{
    int switchPortNumberForRemoteNode = 0;
    char physPortNameProcEntryName[128];
    char physPortNameProcEntryValue[128];
    char bridgeEntryName[128];
    FILE *fp;
    ssize_t bytesRead;
    int rc;

    BSTD_UNUSED(pSocketState);

    /* On 7278, remote nodes can be behind one of the externally exposed switch ports such as Port#0 (Gphy), Port#1 (RGMII_1), Port#2 (RGMII_2). */
    /* Or the remote node can also be on the same host (which requires special routing rules to direct the packets to go via the IMP/wifi ports of switch). */
    /* In addition, a remote can also be indirectly connected via the USB Ethernet or PCIe (WIFI). Since these interfaces are not directly connected to the switch, */
    /* ASP interacts to them via the 2nd System Port (called Wifi System Port on 7278). The 2nd system port is designed for this purpose so that */
    /* all ASP <--> WIFI related traffic can be isolated & possibly handled by separate logic in kernel/user space. In such cases, a network bridge is created between the */
    /* USB/PCIe interfaces & 2nd System Port. */
    /* This topology complicates the logic to determine the port # associated w/ the remote. */
    /* The logic uses the interface associated w/ the local IP address (for this TCP connection) and determines if it is part of a Linux bridge or not. */
    /* If it is not (meaining that the remote is behind the externally exposed switch ports,  then logic obtains the switch port# associated w/ this interface (using /sys FS entry). */
    /* If it is behind the bridge, then we get the dump of the neighbor table from the kernel and search for an entry matching w/ the remote's MAC address. */
    /* This entry then contains the switch's interface that can be used. */

    /* Linux provides the interface name to port# mapping in /sys/class/net/<interfaceName>/phys_port_name. */
    /* E.g. cat of /sys/class/net/gphy/phys_port_name shows p0 */
    BKNI_Memset(physPortNameProcEntryName, 0, sizeof(physPortNameProcEntryName));
    BKNI_Memset(physPortNameProcEntryValue, 0, sizeof(physPortNameProcEntryValue));
    BKNI_Memset(bridgeEntryName, 0, sizeof(bridgeEntryName));
    snprintf(bridgeEntryName, sizeof(bridgeEntryName)-1, "%s%s%s", "/sys/class/net/", pSocketState->interfaceName, "/bridge");
    if (access(bridgeEntryName, R_OK) == 0)
    {
        /* Local interface is bridged, so we will read the Linux's neighbor table and look for entry */
        /* matching the remote's MAC address. That will give us the interface name that the remote is behind. */
        /* We can then find the port# from the sys FS entry of that port. */
        BKNI_Memcpy(pSocketState->masterInterfaceName, pSocketState->interfaceName, sizeof(pSocketState->interfaceName));
        pSocketState->masterInterfaceIndex = pSocketState->interfaceIndex;
        rc = B_AspChannel_UpdateInterfaceName(pSocketState, &pSocketState->interfaceName[0]);
        BDBG_ASSERT(rc == 0);
    }
    else if (pSocketState->remoteOnLocalHost)
    {
        /* Remote node happens to be on the same host, so we will instead use the interface corresponding to the remote IP. */
        /* This is needed because the both the BRCM tag (needed for ASP to send frames to the remote via the switch) & */
        /* CFP rule (needed for remote's frames to be forwarded to ASP port) need the interface associated with the remote. */
        /* In all non-local cases, the interface associated w/ either bridge (as found above) or to the local interface is the one */
        /* used to send/receive frames to the remote. */
        BKNI_Memcpy(pSocketState->masterInterfaceName, pSocketState->interfaceName, sizeof(pSocketState->interfaceName));
        pSocketState->masterInterfaceIndex = pSocketState->interfaceIndex;
        BKNI_Memcpy(pSocketState->interfaceName, pSocketState->remoteInterfaceName, sizeof(pSocketState->interfaceName));
        pSocketState->interfaceIndex = pSocketState->remoteInterfaceIndex;
    }
    /* We have name of the real interface. Read its port# from phys_port_name entry of sysfs. */
    {
        BDBG_MSG(("Looking up switch port name for pInterfaceName=%s masterInterfaceName=%s",
                    pSocketState->interfaceName, pSocketState->masterInterfaceName));
        snprintf(physPortNameProcEntryName, sizeof(physPortNameProcEntryName)-1, "%s%s%s", "/sys/class/net/", pSocketState->interfaceName, "/phys_port_name");
        fp = fopen(physPortNameProcEntryName, "r");
        BDBG_ASSERT(fp);
        bytesRead = fread(physPortNameProcEntryValue, 1, sizeof(physPortNameProcEntryValue)-1, fp);
        BDBG_ASSERT(bytesRead >= 2);
        rc = fclose(fp);
        BDBG_ASSERT(rc==0);
        switchPortNumberForRemoteNode = (int)strtol(physPortNameProcEntryValue+1, NULL, 10);
    }

    BDBG_MSG(("pInterfaceName=%s masterInterfaceName=%s procEntryName=%s switchPortNumberForRemoteNode=%d",
                pSocketState->interfaceName, pSocketState->masterInterfaceName, physPortNameProcEntryName, switchPortNumberForRemoteNode));

    return switchPortNumberForRemoteNode;
} /* getSwitchPortNumberForRemoteNode */

void B_AspChannel_GetDefaultCreateSettings(
    B_AspStreamingProtocol                  streamingProtocol,
    B_AspChannelCreateSettings              *pSettings
    )
{
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(streamingProtocol < B_AspStreamingProtocol_eMax);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->protocol = streamingProtocol;
    if (streamingProtocol == B_AspStreamingProtocol_eHttp)
    {
        pSettings->protocolSettings.http.version.minor = 1;
        pSettings->protocolSettings.http.version.major = 1;
    }
    pSettings->mediaInfoSettings.maxBitRate = 40*1024*1024;
}

B_AspChannelHandle B_AspChannel_Create(
    int                                 socketFdToOffload,      /* in: fd of socket to be offloaded from host to ASP. */
    const B_AspChannelCreateSettings    *pSettings              /* in: create settings. */
    )
{
    int                                 rc;
    B_AspChannelHandle                  hAspChannel;
#if 0
    NEXUS_Error                         nrc;
#endif

    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(pSettings->mode < B_AspStreamingMode_eMax);
    BDBG_ASSERT(pSettings->protocol < B_AspStreamingProtocol_eMax);

    BDBG_MSG(("%s: socketFd=%d", BSTD_FUNCTION, socketFdToOffload));

    if ( !g_aspMgr.initialized )
    {
        BDBG_ERR(("Error, B_Asp_Init() has not been called"));
        BDBG_ASSERT(g_aspMgr.initialized);
        return NULL;
    }

    if (pSettings->mode == B_AspStreamingMode_eOut && !pSettings->modeSettings.streamOut.hRecpump)
    {
        BDBG_ERR(("%s: Missing parameter: StreamingOut mode requires pSettings->modeSettings.streamOut.hRecpump!", BSTD_FUNCTION));
        return NULL;
    }

    if (pSettings->mode == B_AspStreamingMode_eIn && !pSettings->modeSettings.streamIn.hPlaypump)
    {
        BDBG_ERR(("%s: Missing parameter: StreamingIn mode requires pSettings->modeSettings.streamIn.hPlaypump!", BSTD_FUNCTION));
        return NULL;
    }

    if (pSettings->mediaInfoSettings.transportType != NEXUS_TransportType_eTs)
    {
        BDBG_ERR(("%s: Incorrect parameter: NEXUS TransportType=%d is NOT YET supported by ASP!", BSTD_FUNCTION, pSettings->mediaInfoSettings.transportType));
        return NULL;
    }

    hAspChannel = BKNI_Malloc(sizeof(B_AspChannel));
    BDBG_ASSERT(hAspChannel);
    BKNI_Memset(hAspChannel, 0, sizeof(B_AspChannel));
    hAspChannel->aspNetFilterDrvFd = -1;    /* Indicate that fd is not open. */

    hAspChannel->createSettings = *pSettings;

    /* Create Nexus ASP object. */
    {
        NEXUS_AspChannelCreateSettings createSettings;

        NEXUS_AspChannel_GetDefaultCreateSettings(&createSettings);
        hAspChannel->hNexusAspChannel = NEXUS_AspChannel_Create(&createSettings);
        CHECK_PTR_GOTO("ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO ioctl() Failed", hAspChannel->hNexusAspChannel, error);
    }

    BDBG_WRN(("%s: hAspChannel=%p socketFd=%d hNexusAspChannel=%p", BSTD_FUNCTION, (void *)hAspChannel, socketFdToOffload, (void*)hAspChannel->hNexusAspChannel ));

    /* Get the local & remote IP Address: Port info associated with this socket. */
    {
        rc = B_AspChannel_GetSocketAddrInfo(socketFdToOffload, &hAspChannel->socketState);
        CHECK_ERR_NZ_GOTO("B_AspChannel_GetSocketAddrInfo() Failed!", rc, error);
    }

    /* Open ASP NetFilter Driver context & add entry identifying this connection flow. */
    /* This enables ASP NetFilter driver to capture any stale TCP packets or re-assembled UDP packets. */
    if ( ! hAspChannel->socketState.connectionLost)
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo;

        hAspChannel->aspNetFilterDrvFd = open("/dev/brcm_asp", O_RDWR);
        CHECK_ERR_LZ_GOTO("Failed to open ASP NetFilter Driver /dev/brcm_asp, check if asp_netfilter_drv.ko is built & insmoded!", strerror(errno), hAspChannel->aspNetFilterDrvFd, error);

        socket5TupleInfo.srcIpAddr[0] = hAspChannel->socketState.remoteIpAddr.sin_addr.s_addr;
        socket5TupleInfo.srcPort = hAspChannel->socketState.remoteIpAddr.sin_port;
        socket5TupleInfo.dstIpAddr[0] = hAspChannel->socketState.localIpAddr.sin_addr.s_addr;
        socket5TupleInfo.dstPort = hAspChannel->socketState.localIpAddr.sin_port;
        socket5TupleInfo.l4Protocol = ASP_ChLayer4Protocol_eTCP; /* TODO: need to find a way to determine this from given socket. */
        socket5TupleInfo.ipVersion = ASP_ChLayer3IpVersion_eIpv4;

        /* Fill-in ASP Ip address incase we need it when client happens to be on the same local host as ASP. */
        {
            /* When remote client is on the local host itself, we use a different IP address for packets originating from ASP. */
            int err;
            err = inet_aton(ASP_IP_ADDR_FOR_LOCAL_STREAMING, &hAspChannel->socketState.aspIpAddr.sin_addr);
            BDBG_ASSERT(err > 0);
        }

        if (hAspChannel->socketState.localIpAddr.sin_addr.s_addr == hAspChannel->socketState.remoteIpAddr.sin_addr.s_addr)
        {
            socket5TupleInfo.aspIpAddr[0] = hAspChannel->socketState.aspIpAddr.sin_addr.s_addr;
#if 1
            system("arp -s 192.168.1.250 00:10:18:00:33:01");
#else
            /* Client STB on P0. */
            system("arp -s 192.168.1.250 00:10:18:A2:23:7B");
#endif
        }
        rc = ioctl(hAspChannel->aspNetFilterDrvFd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        CHECK_ERR_LZ_GOTO("ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO ioctl() Failed", strerror(errno), rc, error);

        rc = ioctl(hAspChannel->aspNetFilterDrvFd, ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        CHECK_ERR_LZ_GOTO("ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO ioctl() Failed", strerror(errno), rc, error);

        /* TODO: add rule to catch ICMP Path MTU packets for TCP. */
    }

    /* Freeze the socket & obtain TCP/UDP state associated with it. */
    if ( ! hAspChannel->socketState.connectionLost)
    {
        rc = B_AspChannel_GetSocketStateFromLinux(socketFdToOffload, &hAspChannel->socketState);
        CHECK_ERR_NZ_GOTO("B_AspChannel_GetSocketStateFromLinux() Failed ...", rc, error);

        /*
         * Note: we dont close the socket after offloading it from Linux.
         * This makes Linux to reserve the local & remote port combination and
         * thus not allocate it to another connection between the same peers.
         * We will close this migrated socket right before offloading connx back
         * (in the B_AspChannel_Stop().
         */
        hAspChannel->fdMigratedConnx = socketFdToOffload;
        BDBG_MSG(("%s: Obtained socketState from Linux for socketFd=%d", BSTD_FUNCTION, socketFdToOffload));
    }

    /* If the socket is being closed, skip additional configuration. */
    if ( ! hAspChannel->socketState.connectionLost)
    {
        /* Now find the network interface name & MAC address (local & remote). */
        rc = B_AspChannel_GetInterfaceNameAndMacAddress(&hAspChannel->socketState, hAspChannel->aspNetFilterDrvFd);
        CHECK_ERR_NZ_GOTO("B_AspChannel_GetInterfaceNameAndMacAddress() Failed ...", rc, error);

        /* Tell switch to redirect all incoming Ethernet frames (non-fragmented one) of this flow to the ASP Port. */
        hAspChannel->switchPortNumberForAsp = getSwitchPortNumberForAsp();
        hAspChannel->switchQueueNumberForAsp = getSwitchQueueNumberForAsp();
        hAspChannel->switchPortNumberForRemoteNode = getSwitchPortNumberForRemoteNode(&hAspChannel->socketState);

        rc = B_AspChannel_AddFlowClassificationRuleToNwSwitch(&hAspChannel->socketState, hAspChannel->switchPortNumberForAsp, hAspChannel->switchQueueNumberForAsp, &hAspChannel->switchFlowLabel);
        CHECK_ERR_NZ_GOTO("B_AspChannel_AddFlowClassificationRuleToNwSwitch() Failed ...", rc, error);
    }

    /* This channel is ready for offload. Caller will either call _StartStreaming or _WriteComplete() to start the channel. */

    hAspChannel->state = B_AspChannelState_eCreated;
    BDBG_WRN(("%s: hAspChannel=%p is ready for Offloading to ASP!", BSTD_FUNCTION, (void *)hAspChannel));
    return (hAspChannel);

error:
    if (hAspChannel->aspNetFilterDrvFd >= 0) close(hAspChannel->aspNetFilterDrvFd);
    BDBG_ASSERT(hAspChannel);   /* Coverity says that hAspChannel will always be non-NULL here. */
    B_AspChannel_Destroy(hAspChannel, NULL);
    return (NULL);
} /* B_AspChannel_Create */

static int buildIngressBrcmTag(
    int switchQueueNumberForAsp,
    int switchPortNumberForRemoteNode
    )
{
    uint32_t    ingressBrcmTag;
    /*
     * Ingress BrcmTag is in the *ingress* frames to a switch port,
     * meaning inserted by special device nodes such as System Port (host) or ASP)
     * when they send a frame to the switch.
     *
     * This tag instructs the switch to NOT use its Address Resolution Logic (ARL) &
     * instead use the Port & Queue # specified in this tag.
     *
     * Tag is a 32bit number with following syntax:
     *  OpCode[31-29]
     *  Traffic Class [28:26]
     *  N/A [25-9]
     *  Destination Map [8-0]
     *
     * To override default ARP, following values will be used:
     *  Opcode = 3b'001
     *  Traffic Class = 3b'111
     *  Destination Map = bit map containing 1 bit per port where this frame should be forwarded to.
     *      In this case, it will be the one corresponding to the switchPortNumberForRemoteNode.
     *
     */

    /* Set the OpCode[31-29] */
#define B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_ARL_VALUE 1
#define B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_ARL_BIT_SHIFT 29
    ingressBrcmTag = B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_ARL_VALUE << B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_ARL_BIT_SHIFT;

    /* Set the Traffic Class[28-26] */
#define B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_TRAFFIC_CLASS_VALUE 7     /* Queue #7 */
#define B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_TRAFFIC_CLASS_BIT_SHIFT 26
    ingressBrcmTag |= switchQueueNumberForAsp << B_SWITCH_BRCM_TAG_OPCODE_OVERRIDE_TRAFFIC_CLASS_BIT_SHIFT;

    /* Set the Destination Map[8-0]: 1 bit per port. Port 0 is bit 1 */
    ingressBrcmTag |= 1 << switchPortNumberForRemoteNode;

    BDBG_MSG(("%s: switchQueueNumberForAsp=%d switchPortNumberForRemoteNode=%d ingressBrcmTag=0x%x", BSTD_FUNCTION,
                switchQueueNumberForAsp, switchPortNumberForRemoteNode, ingressBrcmTag ));
    return ingressBrcmTag;
} /* buildIngressBrcmTag */

static B_Error startNexusAsp(
    B_AspChannelHandle      hAspChannel,
    bool                    autoStartStreaming
    )
{
    NEXUS_Error                         nrc;
    NEXUS_AspChannelStartSettings       startSettings;
    NEXUS_AspStreamingProtocol          streamingProtocol = NEXUS_AspStreamingProtocol_eTcp;

    /* Map B_Asp Settings to NEXUS_Asp Channel settings. */
    if (hAspChannel->createSettings.protocol == B_AspStreamingProtocol_eHttp)
    {
        streamingProtocol = NEXUS_AspStreamingProtocol_eHttp;
    }
    else
    {
        /* TODO: add logic for UDP/RTP Protocols later. */
        BDBG_ASSERT(hAspChannel->createSettings.protocol);
    }
    NEXUS_AspChannel_GetDefaultStartSettings(streamingProtocol, &startSettings);

    /* Settings common across streaming mode & protocol. */
    startSettings.protocol = streamingProtocol;
    startSettings.connectionLost =  hAspChannel->socketState.connectionLost;
    startSettings.autoStartStreaming = autoStartStreaming;
    startSettings.mediaInfoSettings.transportType = hAspChannel->createSettings.mediaInfoSettings.transportType;
    startSettings.mediaInfoSettings.maxBitRate = hAspChannel->createSettings.mediaInfoSettings.maxBitRate;
    startSettings.drmType = hAspChannel->createSettings.drmType;

    /* Fill-in mode specific settings. */
    if (hAspChannel->createSettings.mode == B_AspStreamingMode_eOut)
    {
        startSettings.mode = NEXUS_AspStreamingMode_eOut;
        startSettings.modeSettings.streamOut.recpump = hAspChannel->createSettings.modeSettings.streamOut.hRecpump;
    }
    else if (hAspChannel->createSettings.mode == B_AspStreamingMode_eIn)
    {
        startSettings.mode = NEXUS_AspStreamingMode_eIn;
        startSettings.modeSettings.streamIn.playpump = hAspChannel->createSettings.modeSettings.streamIn.hPlaypump;
        startSettings.modeSettings.streamIn.dma = hAspChannel->createSettings.modeSettings.streamIn.hDma;
    }

    /* Now fill-in the protocol specific settings. */
    if (hAspChannel->createSettings.protocol == B_AspStreamingProtocol_eHttp)
    {
        /* HTTP level settings. */
        startSettings.protocolSettings.http.enableChunkTransferEncoding = hAspChannel->createSettings.protocolSettings.http.enableChunkTransferEncoding;
        startSettings.protocolSettings.http.chunkSize = hAspChannel->createSettings.protocolSettings.http.chunkSize;
        startSettings.protocolSettings.http.version.major  = hAspChannel->createSettings.protocolSettings.http.version.major;
        startSettings.protocolSettings.http.version.minor  = hAspChannel->createSettings.protocolSettings.http.version.minor;

        /* TCP level settings. */
        startSettings.protocolSettings.http.tcp.localPort = ntohs(hAspChannel->socketState.localIpAddr.sin_port);
        startSettings.protocolSettings.http.tcp.remotePort = ntohs(hAspChannel->socketState.remoteIpAddr.sin_port);
        startSettings.protocolSettings.http.tcp.initialSendSequenceNumber = hAspChannel->socketState.tcpState.seq; /* Linux returns it in the LE format. */
        startSettings.protocolSettings.http.tcp.initialRecvSequenceNumber = hAspChannel->socketState.tcpState.ack;
        startSettings.protocolSettings.http.tcp.currentAckedNumber = hAspChannel->socketState.tcpState.ack;
        startSettings.protocolSettings.http.tcp.maxSegmentSize = hAspChannel->socketState.tcpState.maxSegSize;
        /* TODO: can't yet find a way to obtain the peer's current advertized receive window, so hardcoding it to 10 segments worth. */
        /* FW will get the correct receiver window once it gets 1st ACK from the remote. */
        startSettings.protocolSettings.http.tcp.remoteWindowSize = hAspChannel->socketState.tcpState.maxSegSize * 10;
        if (hAspChannel->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_WSCALE)
        {
            startSettings.protocolSettings.http.tcp.remoteWindowScaleValue = hAspChannel->socketState.tcpState.tcpInfo.tcpi_snd_wscale;
            startSettings.protocolSettings.http.tcp.localWindowScaleValue = hAspChannel->socketState.tcpState.tcpInfo.tcpi_rcv_wscale;
            startSettings.protocolSettings.http.tcp.remoteWindowSize =
                (startSettings.protocolSettings.http.tcp.remoteWindowSize >>
                startSettings.protocolSettings.http.tcp.remoteWindowScaleValue);
        }

        startSettings.protocolSettings.http.tcp.enableSack = hAspChannel->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_SACK;
        startSettings.protocolSettings.http.tcp.enableTimeStamps = hAspChannel->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_TIMESTAMPS;
        startSettings.protocolSettings.http.tcp.timestampEchoValue = 0; /* Not available from Linux, setting to 0 is OK. FW will determine the correct value using the timestamp value in the next incoming ACK from peer. */
        startSettings.protocolSettings.http.tcp.senderTimestamp = hAspChannel->socketState.tcpState.senderTimestamp;

        /* IP level settings. */
        startSettings.protocolSettings.http.tcp.ip.ipVersion = NEXUS_AspIpVersion_e4; /* TODO IPv6 */
        startSettings.protocolSettings.http.tcp.ip.ver.v4.remoteIpAddr = ntohl(hAspChannel->socketState.remoteIpAddr.sin_addr.s_addr); /* TODO IPv6 */

        if (hAspChannel->socketState.localIpAddr.sin_addr.s_addr == hAspChannel->socketState.remoteIpAddr.sin_addr.s_addr)
        {
            startSettings.protocolSettings.http.tcp.ip.ver.v4.localIpAddr = ntohl(hAspChannel->socketState.aspIpAddr.sin_addr.s_addr);
        }
        else
        {
            startSettings.protocolSettings.http.tcp.ip.ver.v4.localIpAddr = ntohl(hAspChannel->socketState.localIpAddr.sin_addr.s_addr); /* TODO IPv6 */
        }
        startSettings.protocolSettings.http.tcp.ip.ver.v4.timeToLive = 64;   /* TODO: get this from socketState. */
        startSettings.protocolSettings.http.tcp.ip.ver.v4.dscp = 56;  /* TODO: verify this from the DSCP values. */
        startSettings.protocolSettings.http.tcp.ip.ver.v4.initialIdentification = 0; /* TODO: get this from socketState. */
        startSettings.protocolSettings.http.tcp.ip.ver.v4.ecn = 0; /* TODO: get this from socketState. */

        /* Ethernet level settings. */
        startSettings.protocolSettings.http.tcp.ip.eth.etherType = 0x0800;   /* TODO: IPv6, it is 0x86DD */
        BKNI_Memcpy(startSettings.protocolSettings.http.tcp.ip.eth.localMacAddress, hAspChannel->socketState.localMacAddress, NEXUS_ETHER_ADDR_LEN);
        BKNI_Memcpy(startSettings.protocolSettings.http.tcp.ip.eth.remoteMacAddress, hAspChannel->socketState.remoteMacAddress, NEXUS_ETHER_ADDR_LEN);

        /* Switch level settings. */
        startSettings.protocolSettings.http.tcp.ip.eth.networkSwitch.queueNumber = hAspChannel->switchQueueNumberForAsp;
        startSettings.protocolSettings.http.tcp.ip.eth.networkSwitch.ingressBrcmTag = buildIngressBrcmTag(hAspChannel->switchQueueNumberForAsp, hAspChannel->switchPortNumberForRemoteNode);
        startSettings.protocolSettings.http.tcp.ip.eth.networkSwitch.egressClassId = hAspChannel->switchFlowLabel;
    }

    nrc = NEXUS_AspChannel_Start(hAspChannel->hNexusAspChannel, &startSettings);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    return B_ERROR_SUCCESS;
} /* startNexusAsp */

static void stateChangedCallback(
    void                    *pCtx,
    int                     param
    )
{
    B_AspChannelHandle      hAspChannel;
    NEXUS_AspChannelStatus  nexusStatus;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspChannel = pCtx;

    NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &nexusStatus);
    if (nexusStatus.state == NEXUS_AspChannelState_eNetworkError)
    {
        hAspChannel->state = B_AspChannelState_eNetworkError;
    }
    else if (nexusStatus.state == NEXUS_AspChannelState_eRcvdEndOfStream)
    {
        hAspChannel->state = B_AspChannelState_eRcvdEndOfStream;
    }
    else
    {
        BDBG_WRN(("%s: TODO: Add handling for remaining state change callbacks!.", BSTD_FUNCTION));
    }

    /* Notify caller. */
    if (hAspChannel->settings.stateChanged.callback)
    {
        BDBG_WRN(("%s:%p invoking stateChanged callback!", BSTD_FUNCTION, (void *)hAspChannel));
        hAspChannel->settings.stateChanged.callback(hAspChannel->settings.stateChanged.context, hAspChannel->settings.stateChanged.param);
    }
}

static void dataReadyCallback(
    void                    *pCtx,
    int                     param
    )
{
    B_AspChannelHandle      hAspChannel;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspChannel = pCtx;
    /* Notify caller. */
    if (hAspChannel->settings.dataReady.callback)
    {
        BDBG_WRN(("%s:%p invoking dataReady callback!", BSTD_FUNCTION, (void *)hAspChannel));
        hAspChannel->settings.dataReady.callback(hAspChannel->settings.dataReady.context, hAspChannel->settings.dataReady.param);
    }
}

B_Error B_AspChannel_StartStreaming(
    B_AspChannelHandle      hAspChannel
    )
{
    B_Error                 rc;
    NEXUS_Error             nrc;
    NEXUS_AspChannelStatus  nexusAspChannelStatus;

    BDBG_ASSERT(hAspChannel);
    BDBG_WRN(("%s: hAspChannel=%p:", BSTD_FUNCTION, (void *)hAspChannel));
    if (hAspChannel->state == B_AspChannelState_eStartedStreaming)
    {
        BDBG_ERR(("%s: hAspChannel=%p: API Sequence Error: B_AspChannel_StartStreaming() is already called!", BSTD_FUNCTION, (void *)hAspChannel));
        return (B_ERROR_INVALID_PARAMETER);
    }

    /* Determiner if Nexus ASP Channel needs to be started or is already started by a previous API such (as B_AspChannel_WriteComplete). */
    NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &nexusAspChannelStatus);
    if (nexusAspChannelStatus.state == NEXUS_AspChannelState_eIdle)
    {
        /* Setup the callbacks. */
        {
            NEXUS_AspChannelSettings settings;

            NEXUS_AspChannel_GetSettings(hAspChannel->hNexusAspChannel, &settings);
            settings.stateChanged.callback = stateChangedCallback;
            settings.stateChanged.context = hAspChannel;
            nrc = NEXUS_AspChannel_SetSettings(hAspChannel->hNexusAspChannel, &settings);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }
        /* Provide all this info to the NEXUS ASP via the NEXUS_AspChannelStartSettings. */
        rc = startNexusAsp(hAspChannel, true /* autoStartStreaming */);
        BDBG_ASSERT(rc == B_ERROR_SUCCESS);
    }
    hAspChannel->state = B_AspChannelState_eStartedStreaming;
    BDBG_WRN(("%s: hAspChannel=%p: Done!", BSTD_FUNCTION, (void *)hAspChannel));
    return (B_ERROR_SUCCESS);
}

/*
* B_AspChannel_StopStreaming()
*/
void B_AspChannel_StopStreaming(
    B_AspChannelHandle    hAspChannel
    )
{
    int rc;
    BDBG_ASSERT(hAspChannel);

    if (hAspChannel->switchFlowLabel)
    {
        rc = B_AspChannel_DeleteFlowClassificationRuleFromNwSwitch( &hAspChannel->socketState, hAspChannel->switchFlowLabel);
        BDBG_ASSERT(rc == B_ERROR_SUCCESS);
        BDBG_MSG(("%s:%p Removed CFP Entry!", BSTD_FUNCTION, (void *)hAspChannel));
    }

    /* Stop the channel w/ Nexus. */
    {
        NEXUS_AspChannel_Stop(hAspChannel->hNexusAspChannel);
        BDBG_MSG(("%s:%p Stopped Nexus ASP Channel!", BSTD_FUNCTION, (void *)hAspChannel));
    }

    hAspChannel->state = B_AspChannelState_eIdle;

}

void B_AspChannel_GetSettings(
    B_AspChannelHandle                      hAspChannel,
    B_AspChannelSettings                    *pSettings
)
{
    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pSettings);
    *pSettings = hAspChannel->settings;
}

B_Error B_AspChannel_SetSettings(
    B_AspChannelHandle                      hAspChannel,
    const B_AspChannelSettings              *pSettings
)
{
    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pSettings);
    hAspChannel->settings = *pSettings;

    return (B_ERROR_SUCCESS);
}


B_Error B_AspChannel_GetStatus(
    B_AspChannelHandle                      hAspChannel,
    B_AspChannelStatus                      *pStatus
)
{
    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pStatus);
    *pStatus = hAspChannel->status;
    NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &pStatus->nexusStatus);

    return (B_ERROR_SUCCESS);
}

B_Error B_AspChannel_SetDtcpIpSettings(
    B_AspChannelHandle                      hAspChannel,
    const B_AspChannelDtcpIpSettings        *pSettings
    )
{
    NEXUS_Error             nrc;

    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pSettings);

    nrc = NEXUS_AspChannel_SetDtcpIpSettings(hAspChannel->hNexusAspChannel, &pSettings->settings);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    return (B_ERROR_SUCCESS);
}


B_Error B_AspChannel_PrintStatus(
    B_AspChannelHandle                      hAspChannel
)
{
    B_AspChannelStatus      status;
    B_Time                  nexusStatusTime;
    NEXUS_AspChannelStatus   *pStatus;
    uint32_t                bitRate = 0;
    uint64_t                diffTimeInUs;
    NEXUS_RecpumpStatus     recpumpStatus = {0};
    NEXUS_RecpumpHandle     hRecpump = NULL;
    NEXUS_PlaypumpStatus    playpumpStatus = {0};
    NEXUS_PlaypumpHandle    hPlaypump = NULL;
    NEXUS_Error             nrc;

    BDBG_ASSERT(hAspChannel);
    NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &status.nexusStatus);
    B_Time_Get(&nexusStatusTime);

    pStatus = &status.nexusStatus;

    /* In order to compute the bit rate, we need to have a prior saved status struct and its associated time.
     * If we haven't saved a status struct for this channel, we'll just say the bitRate is zero for now.  */
    if ( ! hAspChannel->nexusStatusIsValid)
    {
        bitRate = 0;        /* No prior sample, assume bit rate is zero. */

#if 0
        BDBG_LOG(("%s : %d : Ch=%d nexusStatusIsValid is false. bitRate=%u",
                    BSTD_FUNCTION, __LINE__,
                    status.nexusStatus.aspChannelIndex,
                    bitRate));
#endif
    }
    else
    {
        /* We have a prior sample, determine it's age. */

        diffTimeInUs = B_Time_DiffMicroseconds(&nexusStatusTime, &hAspChannel->nexusStatusTime);

        /* We only want to use the previous status struct for computing the bit rate
         * if it was from more than a tenth of a second or so... otherwise it can give misleading
         * results (because the byte count seems to increment by 1316 bytes at a time).
         * So anyway, if the time since the last status struct is too small, continue to report
         * the last calculated bit rate (or zero).  */
        if (diffTimeInUs < 100000)
        {
            bitRate = hAspChannel->nexusStatusBitRate;      /* use last reported bit rate */

#if 0
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld savedBitRate=%u diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        status.nexusStatus.aspChannelIndex,
                        nexusStatusTime.tv_sec, nexusStatusTime.tv_usec,
                        hAspChannel->nexusStatusTime.tv_sec, hAspChannel->nexusStatusTime.tv_usec,
                        bitRate,
                        diffTimeInUs ));
#endif
        }
        else
        {
            /* We have a saved status struct that's old enough, use the elapsed time and
             * byte counts to calculate the  bit rate.  */

            bitRate = (((status.nexusStatus.stats.mcpbConsumedInBytes - hAspChannel->nexusStatus.stats.mcpbConsumedInBytes)*8*1000000)/diffTimeInUs);

#if 0
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld byte count=%lu diffBytes=%lu diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        status.nexusStatus.aspChannelIndex,
                        nexusStatusTime.tv_sec, nexusStatusTime.tv_usec,
                        hAspChannel->nexusStatusTime.tv_sec, hAspChannel->nexusStatusTime.tv_usec,
                        status.nexusStatus.stats.mcpbConsumedInBytes,
                        status.nexusStatus.stats.mcpbConsumedInBytes - hAspChannel->nexusStatus.stats.mcpbConsumedInBytes,
                        diffTimeInUs ));
#endif
        }
    }

    {
        if (hAspChannel->createSettings.modeSettings.streamOut.hRecpump)
        {
            hRecpump = hAspChannel->createSettings.modeSettings.streamOut.hRecpump;
        }
        else if (hAspChannel->createSettings.modeSettings.streamIn.hRecpump)
        {
            hRecpump = hAspChannel->createSettings.modeSettings.streamIn.hRecpump;
        }
        if (hRecpump)
        {
            nrc = NEXUS_Recpump_GetStatus(hRecpump, &recpumpStatus);
            if (nrc != NEXUS_SUCCESS)
            {
                BDBG_WRN(("%s:%u: NEXUS_Recpump_GetStatus() failed, status=%u", BSTD_FUNCTION, BSTD_LINE, nrc));
                return(nrc);
            }
        }
    }

    {
        if (hAspChannel->createSettings.modeSettings.streamOut.hPlaypump)
        {
            hPlaypump = hAspChannel->createSettings.modeSettings.streamOut.hPlaypump;
        }
        else if (hAspChannel->createSettings.modeSettings.streamIn.hPlaypump)
        {
            hPlaypump = hAspChannel->createSettings.modeSettings.streamIn.hPlaypump;
        }
        if (hPlaypump)
        {
            nrc = NEXUS_Playpump_GetStatus(hPlaypump, &playpumpStatus);
            if (nrc != NEXUS_SUCCESS)
            {
                BDBG_WRN(("%s:%u: NEXUS_Playpump_GetStatus() failed, status=%u", BSTD_FUNCTION, BSTD_LINE, nrc));
                return(nrc);
            }
        }
    }

    BDBG_LOG(("TxStats[ch=%d en=%s desc=%s rate=%uMbps avPaused=%s mcpbStalled=%s]: Playpump=%u/%u RAVE=%u/%u mcpbPendng=%u bytes, SndWnd=%u TsPkts=%"PRId64 " IpPkts: mcpb=%"PRId64 " umac=%u p7=%u p8=%u p0=%u",
                status.nexusStatus.aspChannelIndex, pStatus->stats.mcpbChEnabled? "Y":"N", pStatus->stats.mcpbDescFifoEmpty? "N":"Y", bitRate/1000000,
                status.nexusStatus.stats.mcpbAvPaused ? "Y":"N",
                status.nexusStatus.stats.mcpbStalled ? "Y":"N",
                (unsigned)playpumpStatus.fifoDepth, (unsigned)playpumpStatus.fifoSize,
                (unsigned)recpumpStatus.data.fifoDepth, (unsigned)recpumpStatus.data.fifoSize, pStatus->stats.mcpbPendingBufferDepth,
                pStatus->stats.mcpbSendWindow,
                pStatus->stats.mcpbConsumedInTsPkts, pStatus->stats.mcpbConsumedInIpPkts,
                pStatus->stats.unimacTxUnicastIpPkts,
                pStatus->stats.nwSwRxFmAspInUnicastIpPkts,
                pStatus->stats.nwSwTxToHostInUnicastIpPkts,
                pStatus->stats.nwSwTxToP0InUnicastIpPkts
             ));
    BDBG_LOG(("RxStats[ch=%d]: p0=%u p7=%u umac=%u edpkt=%u edpktPending=%u p8=%u Discards: p0=%u",
                status.nexusStatus.aspChannelIndex,
                pStatus->stats.nwSwRxP0InUnicastIpPkts,
                pStatus->stats.nwSwTxToAspInUnicastIpPkts,
                pStatus->stats.unimacRxUnicastIpPkts,
                pStatus->stats.eDpktRxIpPkts,
                pStatus->stats.eDpktPendingPkts,
                pStatus->stats.nwSwRxP8InUnicastIpPkts,
                pStatus->stats.nwSwRxP0InDiscards
             ));
    BDBG_LOG(("FwStats[ch=%d]: window: congestion=%u rcv=%u send=%u, pkts: sent=%"PRId64 " rcvd=%"PRId64 " dropped=%u dataDropped=%u retx=%d, seq#: send=%x ack=%x retx=%x",
                status.nexusStatus.aspChannelIndex,
                status.nexusStatus.stats.fwStats.congestionWindow,
                status.nexusStatus.stats.fwStats.receiveWindow,
                status.nexusStatus.stats.fwStats.sendWindow,
                status.nexusStatus.stats.fwStats.pktsSent,
                status.nexusStatus.stats.fwStats.pktsRcvd,
                status.nexusStatus.stats.fwStats.pktsDropped,
                status.nexusStatus.stats.fwStats.dataPktsDropped,
                status.nexusStatus.stats.fwStats.pktsRetx,
                status.nexusStatus.stats.fwStats.sendSequenceNumber,
                status.nexusStatus.stats.fwStats.rcvdAckNumber,
                status.nexusStatus.stats.fwStats.retxSequenceNumber
             ));

    hAspChannel->nexusStatus = status.nexusStatus;
    hAspChannel->nexusStatusIsValid = true;
    hAspChannel->nexusStatusTime = nexusStatusTime;
    hAspChannel->nexusStatusBitRate = bitRate;

    return (B_ERROR_SUCCESS);
}

/**
Summary:
Send Data via AspChannel to the network.

Description:

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eIn mode, it must call this API
to send the HTTP Request out.

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingMode_eOut mode, it must call this API
to send the HTTP Response to the network client.

This API will prepare the NEXUS_AspChannelStartSettings using Network settings from previous steps &
then call NEXUS_AspChannel_Start().

Returns:
== 0: for success & pBytesSent contains the number of bytes given to the ASP Channel for sending on to network.
<  0 for Error cases.

Note: This is a non-blocking API. If there is no space to buffer up the data, it will return an EAGAIN type
error code. Caller can send more data when it gets the spaceAvailableCallback.
**/
B_Error B_AspChannel_GetWriteBufferWithWrap(
    B_AspChannelHandle                  hAspChannel,
    void                                **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                            *pAmount,    /* [out] size of the available space in the pBuffer before the wrap. */
    void                                **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                            *pAmount2    /* [out] size of the available space in the pBuffer2 that can be written to network. */
    )
{
    NEXUS_Error             nrc;

    BDBG_ASSERT(hAspChannel);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pAmount);

    nrc = NEXUS_AspChannel_GetWriteBufferWithWrap(hAspChannel->hNexusAspChannel, pBuffer, pAmount, pBuffer2, pAmount2);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    return B_ERROR_SUCCESS;
}


B_Error B_AspChannel_WriteComplete(
    B_AspChannelHandle                  hAspChannel,
    unsigned                            amount       /* number of bytes written to the buffer. */
)
{
    B_Error                 rc;
    NEXUS_Error             nrc;
    NEXUS_AspChannelStatus  nexusAspChannelStatus;

    /* TODO: this needs more checks here. */

    BDBG_ASSERT(hAspChannel);
    if (hAspChannel->state == B_AspChannelState_eStartedStreaming)
    {
        BDBG_ERR(("%s: API Sequence Error: B_AspChannel_StartStreaming() is already called!", BSTD_FUNCTION));
        return (B_ERROR_INVALID_PARAMETER);
    }

    nrc = NEXUS_AspChannel_WriteComplete(hAspChannel->hNexusAspChannel, amount);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    /* Determiner if Nexus ASP Channel needs to be started or is already started by a previous API such (as B_AspChannel_WriteComplete). */
    NEXUS_AspChannel_GetStatus(hAspChannel->hNexusAspChannel, &nexusAspChannelStatus);
    BDBG_ASSERT(nexusAspChannelStatus.state == NEXUS_AspChannelState_eIdle);
    {
        /* Setup the callbacks. */
        {
            NEXUS_AspChannelSettings settings;

            NEXUS_AspChannel_GetSettings(hAspChannel->hNexusAspChannel, &settings);
            settings.stateChanged.callback = stateChangedCallback;
            settings.stateChanged.context = hAspChannel;
            settings.dataReady.callback = dataReadyCallback;
            settings.dataReady.context = hAspChannel;
            nrc = NEXUS_AspChannel_SetSettings(hAspChannel->hNexusAspChannel, &settings);
            BDBG_ASSERT(nrc == NEXUS_SUCCESS);
        }
        /* Provide all this info to the NEXUS ASP via the NEXUS_AspChannelStartSettings. */
        rc = startNexusAsp(hAspChannel, true /* autoStartStreaming */);
        BDBG_ASSERT(rc == B_ERROR_SUCCESS);
    }
    hAspChannel->state = B_AspChannelState_eStartedStreaming;
    return (B_ERROR_SUCCESS);
}


/**
Summary:
Recv Data via AspChannel from the network.

Description:

For caller using AspChannel with B_AspHttpStreaming Protocol & B_AspStreamingIn mode, it must call this API
after calling B_Asp_SendPayload() to receive the HTTP response back from server.

Returns:
== 0: for success & pBytesReceived contains the number of bytes received using ASP Channel from the network.
<  0 for Error cases.

Note: This is a non-blocking API. If there is no data available, it will return an EAGAIN type
error code. Caller can send more data when it gets the dataReadyCallback.
**/
B_Error B_AspChannel_GetReadBufferWithWrap(
    B_AspChannelHandle                  hAspChannel,
    const void                          **pBuffer,   /* [out] attr{memory=cached} pointer to buffer containing data read from network. */
    unsigned                            *pAmount,    /* [out] number of bytes available in the data buffer pBuffer. */
    const void                          **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} pointer to buffer after wrap containing data read from network. */
    unsigned                            *pAmount2    /* [out] number of bytes available in the data buffer pBuffer2. */
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspChannel);

    nrc = NEXUS_AspChannel_GetReadBufferWithWrap(hAspChannel->hNexusAspChannel, pBuffer, pAmount, pBuffer2, pAmount2);
    if (nrc != NEXUS_SUCCESS)
    {
        *pAmount = 0;
        BDBG_MSG(("%s: hAspChannel=%p nrc=%d: no data available at this time!", BSTD_FUNCTION, (void *)hAspChannel, nrc));
    }
    else
    {
        BDBG_MSG(("%s: hAspChannel=%p: buffer mmount=%u available!", BSTD_FUNCTION, (void *)hAspChannel, *pAmount));
    }
    return (B_ERROR_SUCCESS);
}


B_Error B_AspChannel_ReadComplete(
    B_AspChannelHandle                  hAspChannel,
    unsigned                            bytesRead    /* number of bytes read/consumed by the caller. */
    )
{
    NEXUS_Error nrc;
    BDBG_ASSERT(hAspChannel);

    nrc = NEXUS_AspChannel_ReadComplete(hAspChannel->hNexusAspChannel, bytesRead);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    return (B_ERROR_SUCCESS);
}
