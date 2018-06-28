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

#include "b_os_lib.h"
#include "b_asp_input.h"
#include "b_asp_priv.h"
#include "b_asp_connection_migration.h"
#include "asp_netfilter_drv.h"
#include "nexus_asp_input.h"

BDBG_MODULE(b_asp_input);
B_ASP_CLASS_DECLARE(B_Asp_Input);

typedef struct B_AspInput
{
    B_ASP_CLASS_DEFINE_INSTANCE_VARS(B_Asp_Input);                     /* Per-instance data used by B_ASP_Class. */

    NEXUS_AspInputHandle                    hNexusAspInput;
    B_AspInputState                         state;
    B_AspSocketState                        socketState;
    int                                     fdMigratedConnx;            /* sockFd of migrated socket. */
    int                                     aspNetFilterDrvFd;

    B_AspInputCreateSettings                createSettings;
    B_AspInputSettings                      settings;
    B_AspInputDtcpIpSettings                dtcpIpSettings;
    int                                     socketFdToOffload;
    B_AspInputConnectHttpSettings           connectHttpSettings;
    B_AspInputStartSettings                 startSettings;

    B_AspInputDrmType                       drmType;

    int                                     switchFlowLabel;
    int                                     switchPortNumberForAsp;     /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchQueueNumberForAsp;    /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchPortNumberForRemoteNode;

    B_AspInputStatus                        status;
    NEXUS_AspInputHttpStatus                nexusHttpStatus;            /* Saved previous Nexus ASP status. */
    bool                                    nexusHttpStatusIsValid;     /* true => nexusStatus has been populated. */
    B_Time                                  nexusHttpStatusTime;        /* Time that nexusStatus was acquired. */

    uint32_t                                txBitRate;                  /* last calculated ASP channel bit-rate. */
    uint32_t                                rxBitRate;                  /* last calculated ASP channel bit-rate. */
} B_AspInput;


static void endOfStreamingCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error             nrc;
    B_AspInputHandle       hAspInput;
    NEXUS_AspInputStatus   nexusStatus;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspInput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Get the latest status from Nexus. */
    {
        NEXUS_AspInput_GetStatus(hAspInput->hNexusAspInput, &nexusStatus);
    }

    /* The endOfStreaming callback must have been invoked because of one of these events! */
    BDBG_ASSERT(nexusStatus.connectionReset || nexusStatus.remoteDoneSending || nexusStatus.networkTimeout);

    {
#if 0 /* ******************** Temporary by Gary **********************/
        /* Check with Sanjeev... do we need to do something like this for streamin? */

        /* Check if previously started Finish sequence completed. */
        if (hAspInput->state == B_AspInputState_eStreaming &&
            nexusStatus.finishRequested && nexusStatus.finishCompleted)
        {
            /* Since ASP is done synchronizing, we can now remove the switch entry to redirect */
            /* any lingering packets back to the host. ASP NetFilter will capture them. */
            if (hAspInput->switchFlowLabel)
            {
                int retCode;

                retCode = B_Asp_DeleteFlowClassificationRuleFromNwSwitch(&hAspInput->socketState, hAspInput->switchFlowLabel);
                if (retCode)
                {
                    BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p: B_Asp_DeleteFlowClassificationRuleFromNwSwitch() Failed!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
                }
                else
                {
                    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Removed CFP Entry from Switch." B_ASP_MSG_PRE_ARG, (void *)hAspInput));
                    hAspInput->switchFlowLabel = 0;
                }
            }

            /* Finish operation has now completed, so transition from _eStreaming back to _eConnected state. */
            hAspInput->state = B_AspInputState_eConnected;
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Finished synchronizing previously started Finish sequence, transitioned to _eConnected State!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        }
#endif /* ******************** Temporary by Gary **********************/

        /* For connectionReset & networkTimeout type events, nothing specific needs to be done here. */
        /* We can just rely on Nexus's status & reuse that for our status purpose too. */
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

    /* Notify caller. */
    if (hAspInput->settings.endOfStreaming.callback)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p: invoking endOfStreaming callback!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        hAspInput->settings.endOfStreaming.callback(hAspInput->settings.endOfStreaming.context, hAspInput->settings.endOfStreaming.param);
    }

error:
    return;
} /* endOfStreamingCallback */


static void bufferReadyCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error             nrc;
    B_AspInputHandle       hAspInput;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspInput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Object is valid, but nothing much to do locally in this callback. */
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

    /* Notify caller. */
    if (hAspInput->settings.bufferReady.callback)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p: invoking bufferReady callback!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        hAspInput->settings.bufferReady.callback(hAspInput->settings.bufferReady.context, hAspInput->settings.bufferReady.param);
    }

error:
    return;
} /* bufferReadyCallback */


static void httpResponseDataReadyCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error         nrc;
    B_AspInputHandle   hAspInput;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspInput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Object is valid, but nothing much to do locally in this callback. */
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

    /* Notify caller. */
    if (hAspInput->settings.httpResponseDataReady.callback)
    {
        BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspInput=%p: invoking httpResponseDataReady callback!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        hAspInput->settings.httpResponseDataReady.callback(hAspInput->settings.httpResponseDataReady.context, hAspInput->settings.httpResponseDataReady.param);
    }

error:
    return;
} /* httpResponseDataReadyCallback */


/**
Summary:
Get Default create settings.
**/
void B_AspInput_GetDefaultCreateSettings(
    B_AspInputCreateSettings               *pSettings    /* [out] */
    )
{
    NEXUS_AspInputCreateSettings nexusCreateSettings;

    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* Our create settings are mostly 1:1 with Nexus. So copy them over. */
    {
        NEXUS_AspInput_GetDefaultCreateSettings(&nexusCreateSettings);

        pSettings->writeFifo.size        = nexusCreateSettings.writeFifo.size;
        pSettings->reassembledFifo.size  = nexusCreateSettings.reassembledFifo.size;
        pSettings->reTransmitFifo.size   = nexusCreateSettings.reTransmitFifo.size;
        pSettings->receiveFifo.size      = nexusCreateSettings.receiveFifo.size;
        pSettings->miscBuffer.size       = nexusCreateSettings.miscBuffer.size;
        pSettings->m2mDmaDescBuffer.size = nexusCreateSettings.m2mDmaDescBuffer.size;

    }
} /* B_AspInput_GetDefaultCreateSettings */


static void copyCreateSettings(
    NEXUS_AspInputCreateBufferSettings   *pDst,
    B_AspInputCreateBufferSettings       *pSrc
    )
{
    pDst->size          = pSrc->size;
    pDst->heap          = pSrc->heap;
    pDst->memory        = pSrc->memory;
    pDst->memoryOffset  = pSrc->memoryOffset;
} /* copyCreateSettings */


/**
Summary:
Create an AspInput.
**/
B_AspInputHandle B_AspInput_Create(
    const B_AspInputCreateSettings         *pSettings   /* [in] */
    )
{
    NEXUS_Error         nrc;
    B_AspInputHandle    hAspInput;

    BDBG_MSG((B_ASP_MSG_PRE_FMT "pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)pSettings));

    hAspInput = BKNI_Malloc(sizeof(B_AspInput));
    BDBG_ASSERT(hAspInput);
    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "BKNI_Malloc() failed to allocate %zu bytes!" B_ASP_MSG_PRE_ARG, sizeof(B_AspInput)));
        return NULL;
    }
    BKNI_Memset(hAspInput, 0, sizeof(B_AspInput));

    /* Add this object instance to the B_Asp_Input class. */
    {
        nrc = B_ASP_CLASS_ADD_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_ADD_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy the create settings. */
    {
        if (!pSettings)
        {
            B_AspInput_GetDefaultCreateSettings(&hAspInput->createSettings);
        }
        else
        {
            hAspInput->createSettings = *pSettings;
        }
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: sizes: writeFifo=%u reassembledFifo=%u reTransmitFifo=%u receiveFifo=%u miscBuffer=%u m2mDmaDescBuffer=%u" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput,
                    hAspInput->createSettings.writeFifo.size,
                    hAspInput->createSettings.reassembledFifo.size,
                    hAspInput->createSettings.reTransmitFifo.size,
                    hAspInput->createSettings.receiveFifo.size,
                    hAspInput->createSettings.miscBuffer.size,
                    hAspInput->createSettings.m2mDmaDescBuffer.size));
    }

    /* Create Nexus ASP object. */
    {
        NEXUS_AspInputCreateSettings createSettings;

        NEXUS_AspInput_GetDefaultCreateSettings(&createSettings);

        copyCreateSettings(&createSettings.writeFifo, &hAspInput->createSettings.writeFifo);
        copyCreateSettings(&createSettings.reassembledFifo, &hAspInput->createSettings.reassembledFifo);
        copyCreateSettings(&createSettings.reTransmitFifo, &hAspInput->createSettings.reTransmitFifo);
        copyCreateSettings(&createSettings.receiveFifo, &hAspInput->createSettings.receiveFifo);
        copyCreateSettings(&createSettings.miscBuffer, &hAspInput->createSettings.miscBuffer);
        copyCreateSettings(&createSettings.m2mDmaDescBuffer, &hAspInput->createSettings.m2mDmaDescBuffer);

        hAspInput->hNexusAspInput = NEXUS_AspInput_Create(&createSettings);
        B_ASP_CHECK_GOTO(hAspInput->hNexusAspInput, ("NEXUS_AspInput_Create() Failed"), error, nrc, NEXUS_NOT_AVAILABLE);
    }

    /* Success, so object now moves to idle state and is ready to get connected. */
    {
        hAspInput->state = B_AspInputState_eIdle;

        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: hNexusAspInput=%p state=%d" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, (void*)hAspInput->hNexusAspInput, hAspInput->state ));
    }

    return (hAspInput);

error:
    B_AspInput_Destroy(hAspInput);
    return (NULL);
} /* B_AspInput_Create */


/**
Summary:
API to Get Current Settings.
**/
void B_AspInput_GetSettings(
    B_AspInputHandle                       hAspInput,
    B_AspInputSettings                     *pSettings   /* [out] */
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);
    if (!hAspInput) return;
    if (!pSettings) return;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy object's current settings. */
    {
        *pSettings = hAspInput->settings;
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return;
} /* B_AspInput_GetSettings */


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error B_AspInput_SetSettings(
    B_AspInputHandle                       hAspInput,
    const B_AspInputSettings               *pSettings   /* [in] */
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);
    if (!hAspInput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, (void *)pSettings,
                    hAspInput==NULL?"hAspInput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspInput=%p: can't call B_AspInput_SetSettings() in B_AspInputState_eStreaming" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        nrc = NEXUS_NOT_SUPPORTED;
    }
    else
    {
        hAspInput->settings = *pSettings;
        nrc = NEXUS_SUCCESS;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Settings updated, current state=%u" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return (nrc);
} /* B_AspInput_SetSettings */


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void B_AspInput_GetDtcpIpSettings(
    B_AspInputHandle                       hAspInput,
    B_AspInputDtcpIpSettings               *pSettings /* [out] */
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);
    if (!hAspInput) return;
    if (!pSettings) return;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy object's current settings. */
    {
        *pSettings = hAspInput->dtcpIpSettings;
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return;
} /* B_AspInput_GetDtcpIpSettings */


/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to B_AspInput_Start().
**/
NEXUS_Error B_AspInput_SetDtcpIpSettings(
    B_AspInputHandle                       hAspInput,
    const B_AspInputDtcpIpSettings         *pSettings  /* [in] */
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);
    if (!hAspInput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, (void *)pSettings,
                    hAspInput==NULL?"hAspInput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p: can't call B_AspInput_SetDtcpIpSettings() in B_AspInputState_eStreaming!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        nrc = NEXUS_NOT_SUPPORTED;
    }
    else
    {
        hAspInput->dtcpIpSettings = *pSettings;
        hAspInput->drmType = B_AspInputDrmType_eDtcpIp;
        nrc = NEXUS_SUCCESS;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Settings updated, current state=%u" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return (nrc);
} /* B_AspInput_SetDtcpIpSettings */


/**
Summary:
API to get default connect HTTP settings

**/
void B_AspInput_GetDefaultConnectHttpSettings(
    B_AspInputConnectHttpSettings          *pSettings /* [out] */
    )
{
    BDBG_ASSERT(pSettings);
    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    {
        NEXUS_AspInputConnectHttpSettings       settings;

        NEXUS_AspInput_GetDefaultConnectHttpSettings(&settings);
        pSettings->transportType = settings.transportType;
        pSettings->hPlaypump     = settings.hPlaypump;
        pSettings->hDma          = settings.hDma;
    }

} /* B_AspInput_GetDefaultConnectHttpSettings */


static int getSwitchPortNumberForAsp(
    void
    )
{
    int switchPortNumberForAsp;
    char physPortNameProcEntryValue[16];
    FILE *fp;
    ssize_t bytesRead;

    /* Read ASP's port# from phys_port_name entry of sysfs. */
    {
        fp = fopen("/sys/class/net/asp/phys_port_name", "r");
        BDBG_ASSERT(fp);
        bytesRead = fread(physPortNameProcEntryValue, 1, sizeof(physPortNameProcEntryValue)-1, fp);
        BDBG_ASSERT(bytesRead >= 2);
        fclose(fp);
        switchPortNumberForAsp = (int)strtol(physPortNameProcEntryValue+1, NULL, 10);
    }

    BDBG_MSG((B_ASP_MSG_PRE_FMT "switchPortNumberForAsp=%d" B_ASP_MSG_PRE_ARG, switchPortNumberForAsp));
    return switchPortNumberForAsp;
} /* getSwitchPortNumberForAsp */


static int getSwitchQueueNumberForAsp(
    void
    )
{
    int switchQueueNumberForAsp;

    /* Each Switch Port has 8 Queues pertaining to Quality of Service. */
    /* All frames streamed out from ASP will be assigned to the highest priority queue (which is #7). */
    switchQueueNumberForAsp = 7;
    return switchQueueNumberForAsp;
} /* getSwitchQueueNumberForAsp */


static int getSwitchPortNumberForRemoteNode(
    B_AspSocketState   *pSocketState,
    int                *pPortNumber
    )
{
    int switchPortNumberForRemoteNode = 0;
    char physPortNameProcEntryName[128];
    char physPortNameProcEntryValue[128];
    char bridgeEntryName[128];
    FILE *fp;
    ssize_t bytesRead;
    int retCode;

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
        retCode = B_Asp_UpdateInterfaceName(pSocketState, &pSocketState->interfaceName[0]);
        BDBG_ASSERT(retCode == 0);
        B_ASP_CHECK_GOTO(retCode == 0, ("pSocketState=%p: B_Asp_UpdateInterfaceName() Failed", (void *)pSocketState), error, retCode, retCode);
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
        retCode = fclose(fp);
        BDBG_ASSERT(retCode==0);
        switchPortNumberForRemoteNode = (int)strtol(physPortNameProcEntryValue+1, NULL, 10);
    }

    BDBG_MSG((B_ASP_MSG_PRE_FMT "pInterfaceName=%s masterInterfaceName=%s procEntryName=%s switchPortNumberForRemoteNode=%d" B_ASP_MSG_PRE_ARG,
                pSocketState->interfaceName, pSocketState->masterInterfaceName, physPortNameProcEntryName, switchPortNumberForRemoteNode));

    *pPortNumber = switchPortNumberForRemoteNode;
    return 0;
error:
    return -1;
} /* getSwitchPortNumberForRemoteNode */


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

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "switchQueueNumberForAsp=%d switchPortNumberForRemoteNode=%d ingressBrcmTag=0x%x" B_ASP_MSG_PRE_ARG,
                switchQueueNumberForAsp, switchPortNumberForRemoteNode, ingressBrcmTag ));
    return ingressBrcmTag;
} /* buildIngressBrcmTag */


/**
Summary:
API to Connect to AspInput.

This API migrates an existing network socket state from Linux to the ASP.
After this, ASP becomes responsible for all network activity for that connection.

Note: After the successfull return from this API, App must not use the socketFd
until it calls B_AspInput_Disconnect().

Note2:
This API should be called when Caller is ready to Offload Network Connection from Host to ASP.

The connection must be offloaded BEFORE ANY AV DATA TRANSFER BEGINS on this connection.

App can call this API either immediately after TCP connection is accepted,
or after it has also received a valid HTTP Request from a Client.

**/
static NEXUS_Error processConnectHttpApi_locked(
    B_AspInputHandle   hAspInput
    )
{
    int             retCode = 0;
    NEXUS_Error     nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspInput);

    /* Enable the HttpResponseDataReady callback from Nexus. */
    {
        NEXUS_AspInputSettings settings;

        NEXUS_AspInput_GetSettings(hAspInput->hNexusAspInput, &settings);

        settings.endOfStreaming.callback = endOfStreamingCallback;
        settings.endOfStreaming.context = hAspInput;

        settings.bufferReady.callback = bufferReadyCallback;
        settings.bufferReady.context = hAspInput;

        settings.httpResponseDataReady.callback = httpResponseDataReadyCallback;
        settings.httpResponseDataReady.context = hAspInput;
        nrc = NEXUS_AspInput_SetSettings(hAspInput->hNexusAspInput, &settings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: NEXUS_AspInput_SetSettings() Failed", (void *)hAspInput), error, nrc, nrc);
    }

    /* Get the local & remote IP Address & Port# info associated with this socket. */
    {
        retCode = B_Asp_GetSocketAddrInfo(hAspInput->socketFdToOffload, &hAspInput->socketState);
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: B_Asp_GetSocketAddrInfo() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);
    }

    /* Open ASP NetFilter Driver context & add entry identifying this connection flow. */
    /* This enables ASP NetFilter driver to capture any stale TCP packets or re-assembled UDP packets. */

    /* If the socket is being closed, skip additional configuration. */
    if (!hAspInput->socketState.connectionLost)
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo;

        hAspInput->aspNetFilterDrvFd = open("/dev/brcm_asp", O_RDWR);
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(hAspInput->aspNetFilterDrvFd>=0,
                ("hAspInput=%p: Failed to open ASP NetFilter Driver /dev/brcm_asp, check if asp_netfilter_drv.ko is built & insmoded!", (void *)hAspInput),
                error, nrc, NEXUS_OS_ERROR);

        socket5TupleInfo.srcIpAddr[0] = hAspInput->socketState.remoteIpAddr.sin_addr.s_addr;
        socket5TupleInfo.srcPort = hAspInput->socketState.remoteIpAddr.sin_port;
        socket5TupleInfo.dstIpAddr[0] = hAspInput->socketState.localIpAddr.sin_addr.s_addr;
        socket5TupleInfo.dstPort = hAspInput->socketState.localIpAddr.sin_port;
        socket5TupleInfo.l4Protocol = ASP_ChLayer4Protocol_eTCP;
        socket5TupleInfo.ipVersion = ASP_ChLayer3IpVersion_eIpv4;

        if (hAspInput->socketState.localIpAddr.sin_addr.s_addr == hAspInput->socketState.remoteIpAddr.sin_addr.s_addr)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p: We don't yet support streaming to the remote on same node!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
            nrc = NEXUS_NOT_SUPPORTED;
            goto error;
        }
        retCode = ioctl(hAspInput->aspNetFilterDrvFd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO ioctl() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);

        retCode = ioctl(hAspInput->aspNetFilterDrvFd, ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO ioctl() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);
        /* TODO: add rule to catch ICMP Path MTU packets for TCP. */
    }

    /* Freeze the socket & obtain TCP/UDP state associated with it. */
    if (!hAspInput->socketState.connectionLost)
    {
        retCode = B_Asp_GetSocketStateFromLinux(hAspInput->socketFdToOffload, &hAspInput->socketState);
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: B_Asp_GetSocketStateFromLinux() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);

        /*
         * Note: we dont close the socket after offloading it from Linux.
         * This makes Linux to reserve the local & remote port combination and
         * thus not allocate it to another connection between the same peers.
         * We will close this migrated socket right before offloading connx back
         * (in the B_AspInput_Stop().
         */
        hAspInput->fdMigratedConnx = hAspInput->socketFdToOffload;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Obtained socketState from Linux for socketFd=%d" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->socketFdToOffload));
    }

    /* If the socket is being closed, skip additional configuration. */
    if (!hAspInput->socketState.connectionLost)
    {
        /* Now find the network interface name & MAC address (local & remote). */
        retCode = B_Asp_GetInterfaceNameAndMacAddress(&hAspInput->socketState, hAspInput->aspNetFilterDrvFd);
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: B_Asp_GetInterfaceNameAndMacAddress() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);

        /* Tell switch to redirect all incoming Ethernet frames (non-fragmented one) of this flow to the ASP Port. */
        hAspInput->switchPortNumberForAsp = getSwitchPortNumberForAsp();
        hAspInput->switchQueueNumberForAsp = getSwitchQueueNumberForAsp();
        retCode = getSwitchPortNumberForRemoteNode(&hAspInput->socketState, &hAspInput->switchPortNumberForRemoteNode) ;

        retCode = B_Asp_AddFlowClassificationRuleToNwSwitch(&hAspInput->socketState, hAspInput->switchPortNumberForAsp, hAspInput->switchQueueNumberForAsp, &hAspInput->switchFlowLabel);
        hAspInput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: B_AspInput_AddFlowClassificationRuleToNwSwitch() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);
    }

    /* Connect with Nexus AspInput & pass the socket state info. */
    {
        NEXUS_AspTcpSettings                  tcpSettings;
        NEXUS_AspInputConnectHttpSettings     connectHttpSettings;

        NEXUS_AspInput_GetDefaultTcpSettings(&tcpSettings);
        NEXUS_AspInput_GetDefaultConnectHttpSettings(&connectHttpSettings);

        if (!hAspInput->socketState.connectionLost)
        {
            /* Fill-in optional ConnectHttp related settings. */
            {
                connectHttpSettings.transportType = hAspInput->connectHttpSettings.transportType;
            }

            /* Fill-in TCP related settings. */
            {
                tcpSettings.localPort = ntohs(hAspInput->socketState.localIpAddr.sin_port);
                tcpSettings.remotePort = ntohs(hAspInput->socketState.remoteIpAddr.sin_port);
                tcpSettings.initialSendSequenceNumber = hAspInput->socketState.tcpState.seq; /* Linux returns it in the LE format. */
                tcpSettings.initialRecvSequenceNumber = hAspInput->socketState.tcpState.ack;
                tcpSettings.currentAckedNumber = hAspInput->socketState.tcpState.ack;
                tcpSettings.maxSegmentSize = hAspInput->socketState.tcpState.maxSegSize;
                /* Note: can't yet find a way to obtain the remote peer's current advertized receive window, so hardcoding it to 10 segments worth. */
                /* FW will get the correct receiver window once it gets 1st ACK from the remote. */
                tcpSettings.remoteWindowSize = hAspInput->socketState.tcpState.maxSegSize * 10;
                if (hAspInput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_WSCALE)
                {
                    tcpSettings.remoteWindowScaleValue = hAspInput->socketState.tcpState.tcpInfo.tcpi_snd_wscale;
                    tcpSettings.localWindowScaleValue = hAspInput->socketState.tcpState.tcpInfo.tcpi_rcv_wscale;
                    tcpSettings.remoteWindowSize = (tcpSettings.remoteWindowSize >> tcpSettings.remoteWindowScaleValue);
                }

                tcpSettings.enableSack = hAspInput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_SACK;
                tcpSettings.enableTimeStamps = hAspInput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_TIMESTAMPS;
                tcpSettings.timestampEchoValue = 0; /* Not available from Linux, setting to 0 is OK. FW will determine the correct value using the timestamp value in the next incoming ACK from peer. */
                tcpSettings.senderTimestamp = hAspInput->socketState.tcpState.senderTimestamp;

                /* IP level settings. */
                tcpSettings.ip.ipVersion = NEXUS_AspIpVersion_e4; /* TODO IPv6 */
                tcpSettings.ip.ver.v4.remoteIpAddr = ntohl(hAspInput->socketState.remoteIpAddr.sin_addr.s_addr); /* TODO IPv6 */

                if (hAspInput->socketState.localIpAddr.sin_addr.s_addr == hAspInput->socketState.remoteIpAddr.sin_addr.s_addr)
                {
                    tcpSettings.ip.ver.v4.localIpAddr = ntohl(hAspInput->socketState.aspIpAddr.sin_addr.s_addr);
                }
                else
                {
                    tcpSettings.ip.ver.v4.localIpAddr = ntohl(hAspInput->socketState.localIpAddr.sin_addr.s_addr); /* TODO IPv6 */
                }
                tcpSettings.ip.ver.v4.timeToLive = 64;   /* TODO: get this from socketState. */
                tcpSettings.ip.ver.v4.dscp = 56;  /* TODO: verify this from the DSCP values. */
                tcpSettings.ip.ver.v4.initialIdentification = 0; /* TODO: get this from socketState. */
                tcpSettings.ip.ver.v4.ecn = 0; /* TODO: get this from socketState. */

                /* Ethernet level settings. */
                tcpSettings.ip.eth.etherType = 0x0800;   /* TODO: IPv6, it is 0x86DD */
                BKNI_Memcpy(tcpSettings.ip.eth.localMacAddress, hAspInput->socketState.localMacAddress, NEXUS_ETHER_ADDR_LEN);
                BKNI_Memcpy(tcpSettings.ip.eth.remoteMacAddress, hAspInput->socketState.remoteMacAddress, NEXUS_ETHER_ADDR_LEN);

                /* Switch level settings. */
                tcpSettings.ip.eth.networkSwitch.queueNumber = hAspInput->switchQueueNumberForAsp;
                tcpSettings.ip.eth.networkSwitch.ingressBrcmTag = buildIngressBrcmTag(hAspInput->switchQueueNumberForAsp, hAspInput->switchPortNumberForRemoteNode);
                tcpSettings.ip.eth.networkSwitch.egressClassId = hAspInput->switchFlowLabel;
            }
        }
        else
        {
            tcpSettings.connectionLost = true;
        }

        connectHttpSettings.hPlaypump = hAspInput->connectHttpSettings.hPlaypump;

        nrc = NEXUS_AspInput_ConnectHttp( hAspInput->hNexusAspInput, &tcpSettings, &connectHttpSettings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("NEXUS_AspInput_ConnectHttp() Failed"), error, nrc, nrc);
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: NEXUS_AspInput_ConnectHttp() is successful!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
    }

    nrc = NEXUS_SUCCESS;
    return (nrc);

error:
    /* Disable the HttpResponseDataReady callback from Nexus. */
    {
        NEXUS_AspInputSettings settings;

        NEXUS_AspInput_GetSettings(hAspInput->hNexusAspInput, &settings);

        settings.endOfStreaming.callback = NULL;
        settings.endOfStreaming.context  = NULL;

        settings.bufferReady.callback = NULL;
        settings.bufferReady.context  = NULL;

        settings.httpResponseDataReady.callback = NULL;
        settings.httpResponseDataReady.context  = NULL;
        NEXUS_AspInput_SetSettings(hAspInput->hNexusAspInput, &settings);
    }

    if (nrc == NEXUS_SUCCESS) nrc = NEXUS_NOT_SUPPORTED; /* nrc was not overwritten by a Nexus API. */
    if (hAspInput->aspNetFilterDrvFd >= 0) close(hAspInput->aspNetFilterDrvFd);
    errno = hAspInput->socketState.savedErrno;
    return (nrc);
} /* processConnectHttpApi_locked */


NEXUS_Error B_AspInput_ConnectHttp(
    B_AspInputHandle                       hAspInput,
    int                                     socketFdToOffload,  /* [in] */
    const B_AspInputConnectHttpSettings    *pSettings          /* [in] */
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);

    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspInput can't be NULL" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: socketFdToOffload=%d pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)hAspInput, socketFdToOffload, (void *)pSettings ));
    BDBG_MSG(( B_ASP_MSG_PRE_FMT "transportType=%u, hPlaypump=%p, hDma=%p" B_ASP_MSG_PRE_ARG, pSettings->transportType, (void*)pSettings->hPlaypump, (void*)pSettings->hDma));
    /*
     * Typical API top level code will look like:
     *  -get class lock,
     *  -validate parameters & save them,
     *  -call function to process the API,
     *  -unlock the class, and
     *  -return.
     */

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Validate state. */
    B_ASP_CHECK_GOTO( hAspInput->state == B_AspInputState_eIdle,
                ("hAspInput=%p: can't call B_AspInput_ConnectHttp() in any state other than B_AspInputState_eIdle, current state=%d", (void *)hAspInput, hAspInput->state),
                error_class_unlock, nrc, NEXUS_NOT_SUPPORTED);

    /* Save the connect settings. */
    {
        if (!pSettings)
        {
            B_AspInput_GetDefaultConnectHttpSettings(&hAspInput->connectHttpSettings);
        }
        else
        {
            hAspInput->connectHttpSettings = *pSettings;
        }
        hAspInput->socketFdToOffload = socketFdToOffload;
    }

    /* Now process the API. */
    {
        nrc = processConnectHttpApi_locked(hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: processConnectHttpApi_locked() Failed", (void *)hAspInput), error_class_unlock, nrc, nrc);

        /* This object is now connected. Caller should now call _Start() to start streaming. */
        hAspInput->state = B_AspInputState_eConnected;
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p is connected, socketFdToOffload=%d connectionLost=%u" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, socketFdToOffload, hAspInput->socketState.connectionLost ));
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return (nrc);
} /* B_AspInput_ConnectHttp */

/**
Summary:
API to provide an HTTP response to be sent out on the network.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.

**/
NEXUS_Error B_AspInput_SendHttpRequest(
    B_AspInputHandle                        hAspInput,
    void                                    *pBuffer,       /* [in] attr{memory=cached} pointer to HTTP response to be sent to network. */
    unsigned                                byteCount       /* [in] number of bytes in HTTP Response buffer. */
    )
{
    NEXUS_Error     nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pBuffer);

    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspInput can't be NULL" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    if (!pBuffer)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " pBuffer can't be NULL" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: pBuffer=%p byteCount=%u" B_ASP_MSG_PRE_ARG, (void *)hAspInput, (void*)pBuffer, byteCount ));

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Validate state. */
    B_ASP_CHECK_GOTO( hAspInput->state == B_AspInputState_eConnected,
                ("hAspInput=%p: can't call B_AspInput_SendHttpRequest() in any state other than B_AspInputState_eConnected, current state=%d", (void *)hAspInput, hAspInput->state),
                error_class_unlock, nrc, NEXUS_NOT_SUPPORTED);

    nrc = NEXUS_AspInput_SendHttpRequest(hAspInput->hNexusAspInput, pBuffer, byteCount);

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
        return (nrc);
}


/**
Summary:
API to receive an HTTP request from  the network.

This is an optional API that is NOT needed if App chooses to directly receive the HTTP Request
using Linux socket APIs & before calling B_AspInput_Connect().

This API is intended to be called after the application receives a B_AspInputSettings.httpResponseDataReady callback.

If this API indicates that it has more available data (via pMoreAvailable), the caller should call this API again to receive additional data.

In the case that a partial HTTP request is returned, the caller should wait for the next httpResponseDataReady callback,
then call this API again to receive additional data.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.

**/

NEXUS_Error B_AspInput_GetHttpResponseData(
    B_AspInputHandle                    hAspInput,
    const void                          **pBuffer,      /* [out] pointer to buffer containing data read from network. */
    unsigned                            *pByteCount     /* [out] number of bytes available in the data buffer pBuffer. */
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(pByteCount);

    nrc = NEXUS_AspInput_GetHttpResponseData(hAspInput->hNexusAspInput, pBuffer, pByteCount);

    return (nrc);
}

/**
Summary:
API to indicate the number of consumed bytes in the HTTP Response data.

Caller must call B_AspInput_HttpResponseDataConsumed() to indicate the bytes that have been consumed while processing the
HTTP Response data bytes read via the B_AspInput_RecvHttpResponseData().

**/
NEXUS_Error B_AspInput_HttpResponseDataConsumed(
    B_AspInputHandle                    hAspInput,
    bool                                responseCompleted,    /* [in] false => Entire buffer is consumed. End of HttpResponse not found, more data is required. */
                                                              /*      true  => End of the HttpResponse has been found and consumed. No more data is required. */

    unsigned                            bytesConsumed         /* [in] If responseComplete is true this is the number of bytes consumed from the current buffer.*/
    )                                                        /*       Else bytesConsumed must be equal to byte count returned by NEXUS_AspInput_GetHttpResponseData. */
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);

    nrc = NEXUS_AspInput_HttpResponseDataConsumed(hAspInput->hNexusAspInput, responseCompleted, bytesConsumed);

    return (nrc);
}


/**
Summary:
Get Default StartSettings.
**/
void B_AspInput_GetDefaultStartSettings(
    B_AspInputStartSettings                *pSettings      /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->feedMode = B_AspInputFeedMode_eAuto;
} /* B_AspInput_GetDefaultStartSettings */


static void mapFeedModeToNexus(
    NEXUS_AspInputFeedMode *pDst,
    B_AspInputFeedMode *pSrc
    )
{
    switch(*pSrc)
    {
        case B_AspInputFeedMode_eAuto:
            *pDst = NEXUS_AspInputFeedMode_eAuto;
            break;
        case B_AspInputFeedMode_eHost:
            *pDst = NEXUS_AspInputFeedMode_eHost;
            break;
        case B_AspInputFeedMode_eAutoWithHostDecryption:
            *pDst = NEXUS_AspInputFeedMode_eAutoWithHostDecryption;
            break;
        default:
            *pDst = NEXUS_AspInputFeedMode_eAuto;
            break;
    }
} /* mapFeedModeToNexus */

/**
Summary:
Start an AspInput.

AspInput will start streaming out after this API returns.

Note: This API can only be called when B_AspInputStatus.state is _eConnected.
AspInput changes its state to _eStreaming when it returns from this API. This can be
checked via the B_AspInputStatus.state variable obtained from _AspInput_GetStatus().

**/
NEXUS_Error B_AspInput_Start(
    B_AspInputHandle                       hAspInput,
    const B_AspInputStartSettings          *pSettings
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)hAspInput, (void *)pSettings));
    if (!hAspInput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, (void *)pSettings,
                    hAspInput==NULL?"hAspInput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_NOT_SUPPORTED);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    B_ASP_CHECK_GOTO( hAspInput->state == B_AspInputState_eConnected,
                ("hAspInput=%p: can't call B_AspInput_Start() in any state other than B_AspInputState_eConnected, current state=%d", (void *)hAspInput, hAspInput->state),
                error_class_unlock, nrc, NEXUS_NOT_SUPPORTED);

    /* Validate settings. */
    {
#if 0 /* ******************** Temporary by Gary **********************/
        /* Ask Sanjeev if hPlaypump is required for feedMode==eHost */

        if (pSettings->feedMode != B_AspInputFeedMode_eHost && !pSettings->hRecpump)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p: pSettings->hRecpump can't be NULL for this pSettings->feedMode=%u" B_ASP_MSG_PRE_ARG,
                        (void *)hAspInput, pSettings->feedMode ));
            nrc = NEXUS_NOT_SUPPORTED;
            goto error_class_unlock;
        }
#endif /* ******************** Temporary by Gary **********************/
        hAspInput->startSettings = *pSettings;
    }

    /* Update DRM Settings w/ Nexus AspInput. */
    if (hAspInput->drmType == B_AspInputDrmType_eDtcpIp)
    {
        NEXUS_AspInputDtcpIpSettings settings;

        NEXUS_AspInput_GetDtcpIpSettings(hAspInput->hNexusAspInput, &settings);

        settings.exchKeyLabel = hAspInput->dtcpIpSettings.exchKeyLabel;
        settings.emiModes = hAspInput->dtcpIpSettings.emiModes;
        settings.pcpPayloadSize = hAspInput->dtcpIpSettings.pcpPayloadSize;
        BKNI_Memcpy(settings.initialNonce, hAspInput->dtcpIpSettings.initialNonce, sizeof(settings.initialNonce));
        BKNI_Memcpy(settings.exchKey, hAspInput->dtcpIpSettings.exchKey, sizeof(settings.exchKey));

        nrc = NEXUS_AspInput_SetDtcpIpSettings(hAspInput->hNexusAspInput, &settings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: NEXUS_AspInput_SetDtcpIpSettings() Failed", (void *)hAspInput), error_class_unlock, nrc, nrc);
    }

    /* Start AspInput. */
    {
        NEXUS_AspInputStartSettings startSettings;

        mapFeedModeToNexus(&startSettings.feedMode, &hAspInput->startSettings.feedMode);

        nrc = NEXUS_AspInput_Start( hAspInput->hNexusAspInput, &startSettings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: NEXUS_AspInput_Start() Failed", (void *)hAspInput), error_class_unlock, nrc, nrc);
    }

    /* All is well, so transition to _eStreaming state. */
    {
        hAspInput->state = B_AspInputState_eStreaming;
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspInput=%p streaming stated!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return (nrc);
} /* B_AspInput_Start */


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error B_AspInput_GetStatus(
    B_AspInputHandle                       hAspInput,
    B_AspInputStatus                       *pStatus     /* [out] */
    )
{
    NEXUS_Error nrc;
    NEXUS_AspInputStatus   nexusStatus;

    BDBG_ASSERT(hAspInput);
    BDBG_ASSERT(pStatus);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: pStatus=%p!" B_ASP_MSG_PRE_ARG, (void *)hAspInput, (void *)pStatus));
    if (!hAspInput || !pStatus)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspInput=%p pStatus=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspInput, (void *)pStatus,
                    hAspInput==NULL?"hAspInput can't be NULL ":"",
                    pStatus==NULL?"pStatus can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Get current status from Nexus & fill-in the caller's structure. */
    {
        nrc = NEXUS_AspInput_GetStatus(hAspInput->hNexusAspInput, &nexusStatus);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: NEXUS_AspInput_GetStatus() Failed", (void *)hAspInput), error_class_unlock, nrc, nrc);

        pStatus->aspChannelIndex    = nexusStatus.aspChannelIndex;
        pStatus->state              = hAspInput->state;
        pStatus->remoteDoneSending  = nexusStatus.remoteDoneSending;
        pStatus->connectionReset    = nexusStatus.connectionReset;
        pStatus->networkTimeout     = nexusStatus.networkTimeout;
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }
error:

    return (NEXUS_SUCCESS);
} /* B_AspInput_GetStatus */


#if 0
/**
Summary:
API to provide AV buffer to the caller before ASP sends it out.

This is an optional API that is NOT needed in the default feed mode:
    B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eAuto.

When B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eHost

    This API returns a buffer available to be filled with data to be output.
    After the caller fills the buffer, it must call B_AspInput_BufferSubmit()
    to submit the buffer to ASP which will then send it out to the network.

When B_AspInputStartSettings.feedMode == B_AspInputFeedMode_eAutoWithHostEncrytion

    This API returns a buffer that contains current data from Recpump before it has
    been sent to the network.  It allows the application to encrypt the data (in place).
    After the caller encrypts the buffer, it must call B_AspInput_BufferSubmit()
    to return the buffer back to ASP which will then send it out to the network.

Note: This API can only be called when B_AspInputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspInput_GetBufferWithWrap(
    B_AspInputHandle                       hAspInput,
    void                                    **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                                *pByteCount, /* [out] size of the available space in the pBuffer before the wrap. */
    void                                    **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                                *pByteCount2 /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

/**
Summary:
API to submit a buffer back to AspOuput.

This allows app to notify AspInput that app is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the B_AspInput_GetBufferWithWrap() API.

Note: This API can only be called when B_AspInputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspInput_BufferSubmit(
    B_AspInputHandle                       hAspInput,
    unsigned                                byteCount   /* number of bytes that AspInput can send to the network. */
    );

#endif

/**
Summary:
Stop an AspInput.

Description:
This API stops the data transmission by the AspInput.
It does NOT synchronize the protocol state with the network peer.

Note: AspInput changes its state to _Connected when it returns from this API. This can be
checked via the B_AspInputStatus.state variable obtained from _AspInput_GetStatus().

For normal stopping, caller should use B_AspInput_Finish().

**/
static void processStopApi_locked(
    B_AspInputHandle                       hAspInput
    )
{
    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        /* Note: Since App is doing the Stop (which is like aborting the connection), */
        /* we remove the flow lookup entry from switch. This way any incoming packets (such as ACKs) */
        /* are not sent to ASP & thus dont steal its cycles. ASP NetFilter driver will contine to drop */
        /* these packets. */
        if (hAspInput->switchFlowLabel)
        {
            int retCode;

            retCode = B_Asp_DeleteFlowClassificationRuleFromNwSwitch( &hAspInput->socketState, hAspInput->switchFlowLabel);
            if (retCode)
            {
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspInput=%p B_Asp_DeleteFlowClassificationRuleFromNwSwitch() Failed" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
            }
            else
            {
                BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Removed CFP Entry(=%u) from Switch." B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->switchFlowLabel));
                hAspInput->switchFlowLabel = 0;
            }
        }

        /* Stop Nexus Input. */
        {
            NEXUS_AspInput_Stop(hAspInput->hNexusAspInput);
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Stopped Nexus AspInput!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
        }

        /* Stop work is complete, so transition state back to _eConnected. */
        hAspInput->state = B_AspInputState_eConnected;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Stopped complete & transitioned to _eConnected state!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: No work to do for B_AspInput_Stop() in state other than B_AspInputState_eStreaming, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));
    }

    return;
} /* processStopApi_locked */

void B_AspInput_Stop(
    B_AspInputHandle                       hAspInput
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);

    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspInput can't be NULL" B_ASP_MSG_PRE_ARG));
        return;
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: state=%d" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Carry out the stop related work.*/
    {
        processStopApi_locked(hAspInput);
    }

    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Done!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
    return;
}


#if 0 /* ******************** Temporary by Gary **********************/
/**
Summary:
API to cleanly stop an AspInput.

Description:
This API initiates the finishing of data transmission by the AspInput & returns immediately
to the caller.

It will synchronize the protocol state with the network peer (if required for a protocol).
E.g. This may involve waiting for TCP ACKs for any pending data and thus may take time.
AspInput will invoke the statusChanged callback when finished.

Note: AspInput changes its state to _Connected when output is cleaned stopped. This can be
checked via the B_AspInputStatus.state variable obtained from _AspInput_GetStatus().

For immediate (un-clean) stopping, app should use B_AspInput_Stop().
**/
void B_AspInput_Finish(
    B_AspInputHandle                       hAspInput
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspInput);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p:" B_ASP_MSG_PRE_ARG, (void *)hAspInput));
    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspInput can't be NULL" B_ASP_MSG_PRE_ARG));
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        /* Request Finish() on Nexus AspInput. */
        NEXUS_AspInput_Finish(hAspInput->hNexusAspInput);
        hAspInput->status.finishRequested = true;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: NEXUS_AspInput_Finish() work started!" B_ASP_MSG_PRE_ARG, (void *)hAspInput));

        /* Note: Nexus will issue the endOfStreaming callback when Finishing sequence completes. */
        /* App can then call _GetStatus() to check the completion status. */
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: No work to do for B_AspInput_Finish() in state other than B_AspInputState_eStreaming, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return;
}
#endif /* ******************** Temporary by Gary **********************/

/**
Summary:
API to Get Current Status.
**/
void B_AspInput_PrintStatus(
    B_AspInputHandle                       hAspInput
    )
{
    B_Time                  nexusHttpStatusTime;
    uint32_t                txBitRate = 0;
    uint32_t                rxBitRate = 0;
    uint64_t                diffTimeInUs;
    NEXUS_RecpumpStatus     recpumpStatus = {0};
#if 0 /* ******************** Temporary by Gary **********************/
    NEXUS_RecpumpHandle     hRecpump = NULL;
#endif /* ******************** Temporary by Gary **********************/
    NEXUS_PlaypumpStatus    playpumpStatus = {0};
    NEXUS_PlaypumpHandle    hPlaypump = NULL;
    NEXUS_Error             nrc;
    NEXUS_AspInputHttpStatus nexusHttpStatus;
    NEXUS_AspInputStatus   nexusStatus;
    NEXUS_AspInputHttpStatus   *pStatus;

    BDBG_ASSERT(hAspInput);


    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    nrc = NEXUS_AspInput_GetStatus(hAspInput->hNexusAspInput, &nexusStatus);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    nrc = NEXUS_AspInput_GetHttpStatus(hAspInput->hNexusAspInput, &nexusHttpStatus);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    B_Time_Get(&nexusHttpStatusTime);

    pStatus = &nexusHttpStatus;

    /* In order to compute the bit rate, we need to have a prior saved status struct and its associated time.
     * If we haven't saved a status struct for this channel, we'll just say the txBitRate is zero for now.  */
    if ( ! hAspInput->nexusHttpStatusIsValid)
    {
        txBitRate = 0;        /* No prior sample, assume bit rate is zero. */
        rxBitRate = 0;        /* No prior sample, assume bit rate is zero. */
    }
    else
    {
        /* We have a prior sample, determine it's age. */

        diffTimeInUs = B_Time_DiffMicroseconds(&nexusHttpStatusTime, &hAspInput->nexusHttpStatusTime);

        /* We only want to use the previous status struct for computing the bit rate
         * if it was from more than a tenth of a second or so... otherwise it can give misleading
         * results (because the byte count seems to increment by 1316 bytes at a time).
         * So anyway, if the time since the last status struct is too small, continue to report
         * the last calculated bit rate (or zero).  */
        if (diffTimeInUs < 100000)
        {
            txBitRate = hAspInput->txBitRate;      /* use last reported bit rate */
            rxBitRate = hAspInput->rxBitRate;      /* use last reported bit rate */

#if 1
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld savedBitRate=%u diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        nexusStatus.aspChannelIndex,
                        nexusHttpStatusTime.tv_sec, nexusHttpStatusTime.tv_usec,
                        hAspInput->nexusHttpStatusTime.tv_sec, hAspInput->nexusHttpStatusTime.tv_usec,
                        txBitRate,
                        diffTimeInUs ));
#endif
        }
        else
        {
            /* We have a saved status struct that's old enough, use the elapsed time and
             * byte counts to calculate the  bit rate.  */

            txBitRate = (((nexusHttpStatus.tcp.mcpbConsumedInBytes - hAspInput->nexusHttpStatus.tcp.mcpbConsumedInBytes)*8*1000000)/diffTimeInUs);
            rxBitRate = (((uint64_t)(nexusHttpStatus.tcp.fwStats.rcvdSequenceNumber - hAspInput->nexusHttpStatus.tcp.fwStats.rcvdSequenceNumber))*8*1000000)/diffTimeInUs;

#if 1
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld now bytes=%lu prev bytes=%lu diffBytes=%lu diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        nexusStatus.aspChannelIndex,
                        nexusHttpStatusTime.tv_sec, nexusHttpStatusTime.tv_usec,
                        hAspInput->nexusHttpStatusTime.tv_sec, hAspInput->nexusHttpStatusTime.tv_usec,
                        nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        hAspInput->nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        nexusHttpStatus.tcp.mcpbConsumedInBytes - hAspInput->nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        diffTimeInUs ));
#endif
        }
    }

#if 0 /* ******************** Temporary by Gary **********************/
    {
        if (hAspInput->startSettings.hRecpump)
        {
            hRecpump = hAspInput->startSettings.hRecpump;
        }
        else
        {
            hRecpump = NULL;
        }
        if (hRecpump)
        {
            nrc = NEXUS_Recpump_GetStatus(hRecpump, &recpumpStatus);
            if (nrc != NEXUS_SUCCESS)
            {
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspInput=%p: NEXUS_Recpump_GetStatus() failed, status=%u" B_ASP_MSG_PRE_ARG, (void *)hAspInput, nrc));
                goto error_class_unlock;
            }
        }
    }
#endif /* ******************** Temporary by Gary **********************/

    {
        if (hAspInput->connectHttpSettings.hPlaypump)
        {
            hPlaypump = hAspInput->connectHttpSettings.hPlaypump;
        }
        else
        {
            hPlaypump = NULL;
        }
        if (hPlaypump)
        {
            nrc = NEXUS_Playpump_GetStatus(hPlaypump, &playpumpStatus);
            if (nrc != NEXUS_SUCCESS)
            {
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspInput=%p: NEXUS_Playpump_GetStatus() failed, status=%u" B_ASP_MSG_PRE_ARG, (void *)hAspInput, nrc));
                goto error_class_unlock;
            }
        }
    }

    BDBG_LOG(("TxStats[ch=%d en=%s desc=%s TxRate=%uMbps RxRate=%uMpbs avPaused=%s mcpbStalled=%s]: Playpump=%u/%u RAVE=%u/%u mcpbPendng=%u bytes, SndWnd=%u TsPkts=%"PRId64 " IpPkts: mcpb=%"PRId64 " umac=%u p7=%u p8=%u p0=%u",
                nexusStatus.aspChannelIndex, pStatus->tcp.mcpbChEnabled? "Y":"N", pStatus->tcp.mcpbDescFifoEmpty? "N":"Y", txBitRate/1000000, rxBitRate/1000000,
                nexusHttpStatus.tcp.mcpbAvPaused ? "Y":"N",
                nexusHttpStatus.tcp.mcpbStalled ? "Y":"N",
                (unsigned)playpumpStatus.fifoDepth, (unsigned)playpumpStatus.fifoSize,
                (unsigned)recpumpStatus.data.fifoDepth, (unsigned)recpumpStatus.data.fifoSize, pStatus->tcp.mcpbPendingBufferDepth,
                pStatus->tcp.mcpbSendWindow,
                pStatus->tcp.mcpbConsumedInTsPkts, pStatus->tcp.mcpbConsumedInIpPkts,
                pStatus->tcp.unimacTxUnicastIpPkts,
                pStatus->tcp.nwSwRxFmAspInUnicastIpPkts,
                pStatus->tcp.nwSwTxToHostInUnicastIpPkts,
                pStatus->tcp.nwSwTxToP0InUnicastIpPkts
             ));
    BDBG_LOG(("RxStats[ch=%d]: p0=%u p7=%u umac=%u edpkt=%u edpktPending=%u p8=%u Discards: p0=%u xptTsPkts=%"PRIu64 " xptBytes=%"PRIu64,
                nexusStatus.aspChannelIndex,
                pStatus->tcp.nwSwRxP0InUnicastIpPkts,
                pStatus->tcp.nwSwTxToAspInUnicastIpPkts,
                pStatus->tcp.unimacRxUnicastIpPkts,
                pStatus->tcp.eDpktRxIpPkts,
                pStatus->tcp.eDpktPendingPkts,
                pStatus->tcp.nwSwRxP8InUnicastIpPkts,
                pStatus->tcp.nwSwRxP0InDiscards,
                pStatus->tcp.xptMcpbConsumedInTsPkts, pStatus->tcp.xptMcpbConsumedInBytes
             ));
    BDBG_LOG(("FwStats[ch=%d]: window: congestion=%u rcv=%u send=%u, pkts: sent=%"PRId64 " rcvd=%"PRId64 " dropped=%u dataDropped=%u retx=%d, seq#: send=%x ack=%x rcvd=%x retx=%x rx: xptDesc=%u bytes=%u OutOfOrder pkts=%u events=%u curEvents=%u",
                nexusStatus.aspChannelIndex,
                nexusHttpStatus.tcp.fwStats.congestionWindow,
                nexusHttpStatus.tcp.fwStats.receiveWindow,
                nexusHttpStatus.tcp.fwStats.sendWindow,
                nexusHttpStatus.tcp.fwStats.pktsSent,
                nexusHttpStatus.tcp.fwStats.pktsRcvd,
                nexusHttpStatus.tcp.fwStats.pktsDropped,
                nexusHttpStatus.tcp.fwStats.dataPktsDropped,
                nexusHttpStatus.tcp.fwStats.pktsRetx,
                nexusHttpStatus.tcp.fwStats.sendSequenceNumber,
                nexusHttpStatus.tcp.fwStats.rcvdAckNumber,
                nexusHttpStatus.tcp.fwStats.rcvdSequenceNumber,
                nexusHttpStatus.tcp.fwStats.retxSequenceNumber,
                nexusHttpStatus.tcp.fwStats.descriptorsFedToXpt,
                nexusHttpStatus.tcp.fwStats.bytesFedToXpt,
                nexusHttpStatus.tcp.fwStats.outOfOrderPacketsRcvd,
                nexusHttpStatus.tcp.fwStats.outOfOrderEventsRcvd,
                nexusHttpStatus.tcp.fwStats.curOutOfOrderEventsRcvd
             ));

    hAspInput->nexusHttpStatus = nexusHttpStatus;
    hAspInput->nexusHttpStatusIsValid = true;
    hAspInput->nexusHttpStatusTime = nexusHttpStatusTime;
    hAspInput->txBitRate = txBitRate;
    hAspInput->rxBitRate = rxBitRate;

error_class_unlock:
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return;
}

/**
Summary:
API to disconnect from AspInput.

This call transfers the ownership of connection from ASP to Host.
It recreates the connection state back in the Linux socket that was
passed in the B_AspInput_Connect*().

After this API returns, caller must close this socket using "close()".
TODO: clarify if this socket can be used for further sending or receiving.

**/
static NEXUS_Error processDisconnectApi_locked(
    B_AspInputHandle                       hAspInput,
    B_AspInputDisconnectStatus             *pStatus
    )
{
    NEXUS_Error     nrc = NEXUS_SUCCESS;

#if 1 /* ******************** Temporary by Gary **********************/
    BSTD_UNUSED(pStatus);
#endif /* ******************** Temporary by Gary **********************/

    if (hAspInput->state == B_AspInputState_eConnected)
    {
        /* Disconnect from Nexus AspInput. */
        {
            NEXUS_AspInputDisconnectStatus     disconnectStatus;

            nrc = NEXUS_AspInput_Disconnect(hAspInput->hNexusAspInput, &disconnectStatus);
            B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspInput=%p: NEXUS_AspInput_Disconnect() Failed", (void *)hAspInput), error, nrc, nrc);
        }

#if 0 /* ******************** Temporary by Gary **********************/
        /* Offload connection back to Linux if needed. */
        if (pStatus && !hAspInput->socketState.connectionLost)
        {
            int retCode;

            retCode = B_Asp_SetSocketStateToLinux(&hAspInput->socketState, hAspInput->fdMigratedConnx, pStatus->pSocketFd);
            B_ASP_CHECK_GOTO(retCode == 0, ("hAspInput=%p: B_Asp_SetSocketStateToLinux() Failed", (void *)hAspInput), error, nrc, NEXUS_OS_ERROR);
        }
#endif /* ******************** Temporary by Gary **********************/

        /* Close ASP NetFilter Driver. */
        {
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: Closing ASP NetFilterDriver socketFd=%d!" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->aspNetFilterDrvFd));
            if (hAspInput->aspNetFilterDrvFd) close(hAspInput->aspNetFilterDrvFd);
        }

        /* Reset states back to Idle. */
        {
            hAspInput->state = B_AspInputState_eIdle;
            hAspInput->drmType = B_AspInputDrmType_eNone;
            hAspInput->nexusHttpStatusIsValid = false;
        }
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: No work to do for B_AspInput_Disconnect() in state other than B_AspInputState_eConnected, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));
    }
error:
    return (nrc);
} /* processDisconnectApi_locked */


NEXUS_Error B_AspInput_Disconnect(
    B_AspInputHandle                       hAspInput,
    B_AspInputDisconnectStatus             *pStatus
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspInput);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: pStatus=%p!" B_ASP_MSG_PRE_ARG, (void *)hAspInput, (void *)pStatus));
    if (!hAspInput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspInput can't be NULL!" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Stop the object if app didn't explicitly call _Stop(). */
    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        processStopApi_locked(hAspInput);
    }

    /* Carry out the disconnect related work. */
    {
        processDisconnectApi_locked(hAspInput, pStatus);
    }

    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Input, hAspInput);
    }

error:
    return (nrc);
} /* B_AspInput_Disconnect */

/**
Summary:
Destroy AspInput object.

Description:
Stops and Destroys an AspInput. The handle can no longer be used.

**/
void B_AspInput_Destroy(
    B_AspInputHandle                       hAspInput
    )
{
    NEXUS_Error     nrc;

    if (!hAspInput) return;

    /* Object is being destroyed, so remove it from the class list. */
    /* This way any further callbacks from Nexus will be ignored. */
    {
        nrc = B_ASP_CLASS_REMOVE_INSTANCE(B_Asp_Input, hAspInput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_REMOVE_INSTANCE() Failed"), error, nrc, nrc);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspInput=%p: state=%d" B_ASP_MSG_PRE_ARG, (void *)hAspInput, hAspInput->state ));

    if (hAspInput->state == B_AspInputState_eStreaming)
    {
        processStopApi_locked(hAspInput);
    }

    /* Carry out the disconnect related work. */
    if (hAspInput->state == B_AspInputState_eConnected)
    {
        processDisconnectApi_locked(hAspInput, NULL);
    }

    if (hAspInput->hNexusAspInput) NEXUS_AspInput_Destroy(hAspInput->hNexusAspInput);

error:
    BKNI_Free(hAspInput);

    return;
} /* B_AspInput_Destroy */
