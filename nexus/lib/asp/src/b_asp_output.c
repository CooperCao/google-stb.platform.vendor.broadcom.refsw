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
#include "b_asp_output.h"
#include "b_asp_priv.h"
#include "b_asp_connection_migration.h"
#include "asp_netfilter_drv.h"
#include "nexus_asp_output.h"

BDBG_MODULE(b_asp_output);
B_ASP_CLASS_DECLARE(B_Asp_Output);

typedef struct B_AspOutput
{
    B_ASP_CLASS_DEFINE_INSTANCE_VARS(B_Asp_Output);                     /* Per-instance data used by B_ASP_Class. */

    NEXUS_AspOutputHandle                   hNexusAspOutput;
    B_AspOutputState                        state;
    B_AspSocketState                        socketState;
    int                                     fdMigratedConnx;            /* sockFd of migrated socket. */
    int                                     aspNetFilterDrvFd;

    B_AspOutputCreateSettings               createSettings;
    B_AspOutputSettings                     settings;
    B_AspOutputDtcpIpSettings               dtcpIpSettings;
    int                                     socketFdToOffload;
    B_AspOutputConnectHttpSettings          connectHttpSettings;
    B_AspOutputStartSettings                startSettings;

    B_AspOutputDrmType                      drmType;

    int                                     switchFlowLabel;
    int                                     switchPortNumberForAsp;     /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchQueueNumberForAsp;    /* TODO: once we know the final API to get it from the Linux, we should move it to the global structure above. */
    int                                     switchPortNumberForRemoteNode;

    B_AspOutputStatus                       status;
    NEXUS_AspOutputHttpStatus               nexusHttpStatus;            /* Saved previous Nexus ASP status. */
    bool                                    nexusHttpStatusIsValid;     /* true => nexusStatus has been populated. */
    B_Time                                  nexusHttpStatusTime;        /* Time that nexusStatus was acquired. */

    uint32_t                                txBitRate;                  /* last calculated ASP channel bit-rate. */
    uint32_t                                rxBitRate;                  /* last calculated ASP channel bit-rate. */
} B_AspOutput;


static void endOfStreamingCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error             nrc;
    B_AspOutputHandle       hAspOutput;
    NEXUS_AspOutputStatus   nexusStatus;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspOutput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Get the latest status from Nexus. */
    {
        NEXUS_AspOutput_GetStatus(hAspOutput->hNexusAspOutput, &nexusStatus);
    }

    /* The endOfStreaming callback must have been invoked because of one of these events! */
    BDBG_ASSERT(nexusStatus.connectionReset || nexusStatus.finishCompleted || nexusStatus.networkTimeout);
    {
        /* Check if previously started Finish sequence completed. */
        if (hAspOutput->state == B_AspOutputState_eStreaming &&
            nexusStatus.finishRequested && nexusStatus.finishCompleted)
        {
            /* Since ASP is done synchronizing, we can now remove the switch entry to redirect */
            /* any lingering packets back to the host. ASP NetFilter will capture them. */
            if (hAspOutput->switchFlowLabel)
            {
                int retCode;

                retCode = B_Asp_DeleteFlowClassificationRuleFromNwSwitch(&hAspOutput->socketState, hAspOutput->switchFlowLabel);
                if (retCode)
                {
                    BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p: B_Asp_DeleteFlowClassificationRuleFromNwSwitch() Failed!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
                }
                else
                {
                    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Removed CFP Entry from Switch." B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
                    hAspOutput->switchFlowLabel = 0;
                }
            }

            /* Finish operation has now completed, so transition from _eStreaming back to _eConnected state. */
            hAspOutput->state = B_AspOutputState_eConnected;
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Finished synchronizing previously started Finish sequence, transitioned to _eConnected State!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        }

        /* For connectionReset & networkTimeout type events, nothing specific needs to be done here. */
        /* We can just rely on Nexus's status & reuse that for our status purpose too. */
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

    /* Notify caller. */
    if (hAspOutput->settings.endOfStreaming.callback)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p: invoking endOfStreaming callback!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        hAspOutput->settings.endOfStreaming.callback(hAspOutput->settings.endOfStreaming.context, hAspOutput->settings.endOfStreaming.param);
    }

error:
    return;
} /* endOfStreamingCallback */


static void remoteDoneSendingCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error             nrc;
    B_AspOutputHandle       hAspOutput;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspOutput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Object is valid, but nothing much to do locally in this callback. */
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

    /* Notify caller. */
    if (hAspOutput->settings.remoteDoneSending.callback)
    {
        BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: invoking remoteDoneSending callback!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        hAspOutput->settings.remoteDoneSending.callback(hAspOutput->settings.remoteDoneSending.context, hAspOutput->settings.remoteDoneSending.param);
    }

error:
    return;
} /* remoteDoneSendingCallback */


static void bufferReadyCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error             nrc;
    B_AspOutputHandle       hAspOutput;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspOutput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Object is valid, but nothing much to do locally in this callback. */
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

    /* Notify caller. */
    if (hAspOutput->settings.bufferReady.callback)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p: invoking bufferReady callback!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        hAspOutput->settings.bufferReady.callback(hAspOutput->settings.bufferReady.context, hAspOutput->settings.bufferReady.param);
    }

error:
    return;
} /* bufferReadyCallback */


static void httpRequestDataReadyCallback(
    void                    *pCtx,
    int                     param
    )
{
    NEXUS_Error         nrc;
    B_AspOutputHandle   hAspOutput;

    BSTD_UNUSED(param);
    BDBG_ASSERT(pCtx);

    hAspOutput = pCtx;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Object is valid, but nothing much to do locally in this callback. */
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

    /* Notify caller. */
    if (hAspOutput->settings.httpRequestDataReady.callback)
    {
        BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: invoking httpRequestDataReady callback!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        hAspOutput->settings.httpRequestDataReady.callback(hAspOutput->settings.httpRequestDataReady.context, hAspOutput->settings.httpRequestDataReady.param);
    }

error:
    return;
} /* httpRequestDataReadyCallback */


/**
Summary:
Get Default create settings.
**/
void B_AspOutput_GetDefaultCreateSettings(
    B_AspOutputCreateSettings               *pSettings    /* [out] */
    )
{
    NEXUS_AspOutputCreateSettings nexusCreateSettings;

    if (!pSettings) return;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* Our create settings are mostly 1:1 with Nexus. So copy them over. */
    {
        NEXUS_AspOutput_GetDefaultCreateSettings(&nexusCreateSettings);

        pSettings->writeFifo.size       = nexusCreateSettings.writeFifo.size;
        pSettings->reTransmitFifo.size  = nexusCreateSettings.reTransmitFifo.size;
        pSettings->receiveFifo.size     = nexusCreateSettings.receiveFifo.size;
        pSettings->miscBuffer.size      = nexusCreateSettings.miscBuffer.size;
    }
} /* B_AspOutput_GetDefaultCreateSettings */


static void copyCreateSettings(
    NEXUS_AspOutputCreateBufferSettings   *pDst,
    B_AspOutputCreateBufferSettings       *pSrc
    )
{
    pDst->size          = pSrc->size;
    pDst->heap          = pSrc->heap;
    pDst->memory        = pSrc->memory;
    pDst->memoryOffset  = pSrc->memoryOffset;
} /* copyCreateSettings */


/**
Summary:
Create an AspOutput.
**/
B_AspOutputHandle B_AspOutput_Create(
    const B_AspOutputCreateSettings         *pSettings   /* [in] */
    )
{
    NEXUS_Error         nrc;
    B_AspOutputHandle   hAspOutput;

    BDBG_MSG((B_ASP_MSG_PRE_FMT "pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)pSettings));

    hAspOutput = BKNI_Malloc(sizeof(B_AspOutput));
    BDBG_ASSERT(hAspOutput);
    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "BKNI_Malloc() failed to allocate %zu bytes!" B_ASP_MSG_PRE_ARG, sizeof(B_AspOutput)));
        return NULL;
    }
    BKNI_Memset(hAspOutput, 0, sizeof(B_AspOutput));

    /* Add this object instance to the B_Asp_Output class. */
    {
        nrc = B_ASP_CLASS_ADD_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_ADD_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy the create settings. */
    {
        if (!pSettings)
        {
            B_AspOutput_GetDefaultCreateSettings(&hAspOutput->createSettings);
        }
        else
        {
            hAspOutput->createSettings = *pSettings;
        }
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: sizes: writeFifo=%u reTransmitFifo=%u receiveFifo=%u miscBuffer=%u" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput,
                    hAspOutput->createSettings.writeFifo.size,
                    hAspOutput->createSettings.reTransmitFifo.size,
                    hAspOutput->createSettings.receiveFifo.size,
                    hAspOutput->createSettings.miscBuffer.size));
    }

    /* Create Nexus ASP object. */
    {
        NEXUS_AspOutputCreateSettings createSettings;

        NEXUS_AspOutput_GetDefaultCreateSettings(&createSettings);

        copyCreateSettings(&createSettings.writeFifo, &hAspOutput->createSettings.writeFifo);
        copyCreateSettings(&createSettings.reTransmitFifo, &hAspOutput->createSettings.reTransmitFifo);
        copyCreateSettings(&createSettings.receiveFifo, &hAspOutput->createSettings.receiveFifo);
        copyCreateSettings(&createSettings.miscBuffer, &hAspOutput->createSettings.miscBuffer);

        hAspOutput->hNexusAspOutput = NEXUS_AspOutput_Create(&createSettings);
        B_ASP_CHECK_GOTO(hAspOutput->hNexusAspOutput, ("NEXUS_AspOutput_Create() Failed"), error, nrc, NEXUS_NOT_AVAILABLE);
    }

    /* Success, so object now moves to idle state and is ready to get connected. */
    {
        hAspOutput->state = B_AspOutputState_eIdle;

        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: hNexusAspOutput=%p state=%d" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, (void*)hAspOutput->hNexusAspOutput, hAspOutput->state ));
    }

    return (hAspOutput);

error:
    B_AspOutput_Destroy(hAspOutput);
    return (NULL);
} /* B_AspOutput_Create */


/**
Summary:
API to Get Current Settings.
**/
void B_AspOutput_GetSettings(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputSettings                     *pSettings   /* [out] */
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);
    if (!hAspOutput) return;
    if (!pSettings) return;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy object's current settings. */
    {
        *pSettings = hAspOutput->settings;
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return;
} /* B_AspOutput_GetSettings */


/**
Summary:
API to Update Current Settings.
**/
NEXUS_Error B_AspOutput_SetSettings(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputSettings               *pSettings   /* [in] */
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);
    if (!hAspOutput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, (void *)pSettings,
                    hAspOutput==NULL?"hAspOutput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: can't call B_AspOutput_SetSettings() in B_AspOutputState_eStreaming" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        nrc = NEXUS_NOT_SUPPORTED;
    }
    else
    {
        hAspOutput->settings = *pSettings;
        nrc = NEXUS_SUCCESS;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Settings updated, current state=%u" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return (nrc);
} /* B_AspOutput_SetSettings */


/**
Summary:
API to Get Current DTCP-IP Settings
**/
void B_AspOutput_GetDtcpIpSettings(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputDtcpIpSettings               *pSettings /* [out] */
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);
    if (!hAspOutput) return;
    if (!pSettings) return;

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Copy object's current settings. */
    {
        *pSettings = hAspOutput->dtcpIpSettings;
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return;
} /* B_AspOutput_GetDtcpIpSettings */


/**
Summary:
API to Set Current DTCP-IP Settings.  This must be called prior to B_AspOutput_Start().
**/
NEXUS_Error B_AspOutput_SetDtcpIpSettings(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputDtcpIpSettings         *pSettings  /* [in] */
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);
    if (!hAspOutput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, (void *)pSettings,
                    hAspOutput==NULL?"hAspOutput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p: can't call B_AspOutput_SetDtcpIpSettings() in B_AspOutputState_eStreaming!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        nrc = NEXUS_NOT_SUPPORTED;
    }
    else
    {
        hAspOutput->dtcpIpSettings = *pSettings;
        hAspOutput->drmType = B_AspOutputDrmType_eDtcpIp;
        nrc = NEXUS_SUCCESS;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Settings updated, current state=%u" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return (nrc);
} /* B_AspOutput_SetDtcpIpSettings */


/**
Summary:
API to get default connect HTTP settings

**/
void B_AspOutput_GetDefaultConnectHttpSettings(
    B_AspOutputConnectHttpSettings          *pSettings /* [out] */
    )
{
    NEXUS_AspOutputConnectHttpSettings  nexusSettings;

    BDBG_ASSERT(pSettings);
    if (!pSettings) return;

    NEXUS_AspOutput_GetDefaultConnectHttpSettings(&nexusSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    {
        pSettings->transportType               = nexusSettings.transportType;
        pSettings->maxBitRate                  = nexusSettings.maxBitRate;

        pSettings->version.major               = nexusSettings.version.major;
        pSettings->version.minor               = nexusSettings.version.minor;
        pSettings->enableChunkTransferEncoding = nexusSettings.enableChunkTransferEncoding;
        pSettings->chunkSize                   = nexusSettings.chunkSize;
    }

} /* B_AspOutput_GetDefaultConnectHttpSettings */


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
API to Connect to AspOutput.

This API migrates an existing network socket state from Linux to the ASP.
After this, ASP becomes responsible for all network activity for that connection.

Note: After the successfull return from this API, App must not use the socketFd
until it calls B_AspOutput_Disconnect().

Note2:
This API should be called when Caller is ready to Offload Network Connection from Host to ASP.

The connection must be offloaded BEFORE ANY AV DATA TRANSFER BEGINS on this connection.

App can call this API either immediately after TCP connection is accepted,
or after it has also received a valid HTTP Request from a Client.

**/
static NEXUS_Error processConnectHttpApi_locked(
    B_AspOutputHandle   hAspOutput
    )
{
    int             retCode = 0;
    NEXUS_Error     nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspOutput);

    /* Get the local & remote IP Address & Port# info associated with this socket. */
    {
        retCode = B_Asp_GetSocketAddrInfo(hAspOutput->socketFdToOffload, &hAspOutput->socketState);
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: B_Asp_GetSocketAddrInfo() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);
    }

    /* Open ASP NetFilter Driver context & add entry identifying this connection flow. */
    /* This enables ASP NetFilter driver to capture any stale TCP packets or re-assembled UDP packets. */

    /* If the socket is being closed, skip additional configuration. */
    if (!hAspOutput->socketState.connectionLost)
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo;

        hAspOutput->aspNetFilterDrvFd = open("/dev/brcm_asp", O_RDWR);
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(hAspOutput->aspNetFilterDrvFd>=0,
                ("hAspOutput=%p: Failed to open ASP NetFilter Driver /dev/brcm_asp, check if asp_netfilter_drv.ko is built & insmoded!", (void *)hAspOutput),
                error, nrc, NEXUS_OS_ERROR);

        socket5TupleInfo.srcIpAddr[0] = hAspOutput->socketState.remoteIpAddr.sin_addr.s_addr;
        socket5TupleInfo.srcPort = hAspOutput->socketState.remoteIpAddr.sin_port;
        socket5TupleInfo.dstIpAddr[0] = hAspOutput->socketState.localIpAddr.sin_addr.s_addr;
        socket5TupleInfo.dstPort = hAspOutput->socketState.localIpAddr.sin_port;
        socket5TupleInfo.l4Protocol = ASP_ChLayer4Protocol_eTCP;
        socket5TupleInfo.ipVersion = ASP_ChLayer3IpVersion_eIpv4;

        if (hAspOutput->socketState.localIpAddr.sin_addr.s_addr == hAspOutput->socketState.remoteIpAddr.sin_addr.s_addr)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: We don't yet support streaming to the remote on same node!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
            nrc = NEXUS_NOT_SUPPORTED;
            goto error;
        }
        retCode = ioctl(hAspOutput->aspNetFilterDrvFd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO ioctl() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);

        retCode = ioctl(hAspOutput->aspNetFilterDrvFd, ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO ioctl() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);
        /* TODO: add rule to catch ICMP Path MTU packets for TCP. */
    }

    /* Freeze the socket & obtain TCP/UDP state associated with it. */
    if (!hAspOutput->socketState.connectionLost)
    {
        retCode = B_Asp_GetSocketStateFromLinux(hAspOutput->socketFdToOffload, &hAspOutput->socketState);
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: B_Asp_GetSocketStateFromLinux() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);

        /*
         * Note: we dont close the socket after offloading it from Linux.
         * This makes Linux to reserve the local & remote port combination and
         * thus not allocate it to another connection between the same peers.
         * We will close this migrated socket right before offloading connx back
         * (in the B_AspOutput_Stop().
         */
        hAspOutput->fdMigratedConnx = hAspOutput->socketFdToOffload;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Obtained socketState from Linux for socketFd=%d" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->socketFdToOffload));
    }

    /* If the socket is being closed, skip additional configuration. */
    if (!hAspOutput->socketState.connectionLost)
    {
        /* Now find the network interface name & MAC address (local & remote). */
        retCode = B_Asp_GetInterfaceNameAndMacAddress(&hAspOutput->socketState, hAspOutput->aspNetFilterDrvFd);
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: B_Asp_GetInterfaceNameAndMacAddress() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);

        /* Tell switch to redirect all incoming Ethernet frames (non-fragmented one) of this flow to the ASP Port. */
        hAspOutput->switchPortNumberForAsp = getSwitchPortNumberForAsp();
        hAspOutput->switchQueueNumberForAsp = getSwitchQueueNumberForAsp();
        retCode = getSwitchPortNumberForRemoteNode(&hAspOutput->socketState, &hAspOutput->switchPortNumberForRemoteNode) ;

        retCode = B_Asp_AddFlowClassificationRuleToNwSwitch(&hAspOutput->socketState, hAspOutput->switchPortNumberForAsp, hAspOutput->switchQueueNumberForAsp, &hAspOutput->switchFlowLabel);
        hAspOutput->socketState.savedErrno = errno;
        B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: B_AspOutput_AddFlowClassificationRuleToNwSwitch() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);
    }

    /* Connect with Nexus AspOutput & pass the socket state info. */
    {
        NEXUS_AspTcpSettings                  tcpSettings;
        NEXUS_AspOutputConnectHttpSettings    connectHttpSettings;

        if (!hAspOutput->socketState.connectionLost)
        {
            /* Fill-in optional ConnectHttp related settings. */
            {
                connectHttpSettings.maxBitRate = hAspOutput->connectHttpSettings.maxBitRate;
                connectHttpSettings.transportType = hAspOutput->connectHttpSettings.transportType;
            }

            /* Fill-in HTTP related settings. */
            {
                connectHttpSettings.version.major = hAspOutput->connectHttpSettings.version.major;
                connectHttpSettings.version.minor = hAspOutput->connectHttpSettings.version.minor;
                connectHttpSettings.enableChunkTransferEncoding = hAspOutput->connectHttpSettings.enableChunkTransferEncoding;
                connectHttpSettings.chunkSize = hAspOutput->connectHttpSettings.chunkSize;
            }

            /* Fill-in TCP related settings. */
            {
                tcpSettings.localPort = ntohs(hAspOutput->socketState.localIpAddr.sin_port);
                tcpSettings.remotePort = ntohs(hAspOutput->socketState.remoteIpAddr.sin_port);
                tcpSettings.initialSendSequenceNumber = hAspOutput->socketState.tcpState.seq; /* Linux returns it in the LE format. */
                tcpSettings.initialRecvSequenceNumber = hAspOutput->socketState.tcpState.ack;
                tcpSettings.currentAckedNumber = hAspOutput->socketState.tcpState.ack;
                tcpSettings.maxSegmentSize = hAspOutput->socketState.tcpState.maxSegSize;
                /* Note: can't yet find a way to obtain the remote peer's current advertized receive window, so hardcoding it to 10 segments worth. */
                /* FW will get the correct receiver window once it gets 1st ACK from the remote. */
                tcpSettings.remoteWindowSize = hAspOutput->socketState.tcpState.maxSegSize * 10;
                if (hAspOutput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_WSCALE)
                {
                    tcpSettings.remoteWindowScaleValue = hAspOutput->socketState.tcpState.tcpInfo.tcpi_snd_wscale;
                    tcpSettings.localWindowScaleValue = hAspOutput->socketState.tcpState.tcpInfo.tcpi_rcv_wscale;
                    tcpSettings.remoteWindowSize = (tcpSettings.remoteWindowSize >> tcpSettings.remoteWindowScaleValue);
                }

                tcpSettings.enableSack = hAspOutput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_SACK;
                tcpSettings.enableTimeStamps = hAspOutput->socketState.tcpState.tcpInfo.tcpi_options & TCPI_OPT_TIMESTAMPS;
                tcpSettings.timestampEchoValue = 0; /* Not available from Linux, setting to 0 is OK. FW will determine the correct value using the timestamp value in the next incoming ACK from peer. */
                tcpSettings.senderTimestamp = hAspOutput->socketState.tcpState.senderTimestamp;

                /* IP level settings. */
                tcpSettings.ip.ipVersion = NEXUS_AspIpVersion_e4; /* TODO IPv6 */
                tcpSettings.ip.ver.v4.remoteIpAddr = ntohl(hAspOutput->socketState.remoteIpAddr.sin_addr.s_addr); /* TODO IPv6 */

                if (hAspOutput->socketState.localIpAddr.sin_addr.s_addr == hAspOutput->socketState.remoteIpAddr.sin_addr.s_addr)
                {
                    tcpSettings.ip.ver.v4.localIpAddr = ntohl(hAspOutput->socketState.aspIpAddr.sin_addr.s_addr);
                }
                else
                {
                    tcpSettings.ip.ver.v4.localIpAddr = ntohl(hAspOutput->socketState.localIpAddr.sin_addr.s_addr); /* TODO IPv6 */
                }
                tcpSettings.ip.ver.v4.timeToLive = 64;   /* TODO: get this from socketState. */
                tcpSettings.ip.ver.v4.dscp = 56;  /* TODO: verify this from the DSCP values. */
                tcpSettings.ip.ver.v4.initialIdentification = 0; /* TODO: get this from socketState. */
                tcpSettings.ip.ver.v4.ecn = 0; /* TODO: get this from socketState. */

                /* Ethernet level settings. */
                tcpSettings.ip.eth.etherType = 0x0800;   /* TODO: IPv6, it is 0x86DD */
                BKNI_Memcpy(tcpSettings.ip.eth.localMacAddress, hAspOutput->socketState.localMacAddress, NEXUS_ETHER_ADDR_LEN);
                BKNI_Memcpy(tcpSettings.ip.eth.remoteMacAddress, hAspOutput->socketState.remoteMacAddress, NEXUS_ETHER_ADDR_LEN);

                /* Switch level settings. */
                tcpSettings.ip.eth.networkSwitch.queueNumber = hAspOutput->switchQueueNumberForAsp;
                tcpSettings.ip.eth.networkSwitch.ingressBrcmTag = buildIngressBrcmTag(hAspOutput->switchQueueNumberForAsp, hAspOutput->switchPortNumberForRemoteNode);
                tcpSettings.ip.eth.networkSwitch.egressClassId = hAspOutput->switchFlowLabel;
            }
        }
        else
        {
            tcpSettings.connectionLost = true;
        }

        connectHttpSettings.hRecpump = hAspOutput->connectHttpSettings.hRecpump;
        nrc = NEXUS_AspOutput_ConnectHttp( hAspOutput->hNexusAspOutput, &tcpSettings, &connectHttpSettings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("NEXUS_AspOutput_ConnectHttp() Failed"), error, nrc, nrc);
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: NEXUS_AspOutput_ConnectHttp() is successful!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
    }

    nrc = NEXUS_SUCCESS;
    return (nrc);

error:
    if (nrc == NEXUS_SUCCESS) nrc = NEXUS_NOT_SUPPORTED; /* nrc was not overwritten by a Nexus API. */
    if (hAspOutput->aspNetFilterDrvFd >= 0) close(hAspOutput->aspNetFilterDrvFd);
    errno = hAspOutput->socketState.savedErrno;
    return (nrc);
} /* processConnectHttpApi_locked */


NEXUS_Error B_AspOutput_ConnectHttp(
    B_AspOutputHandle                       hAspOutput,
    int                                     socketFdToOffload,  /* [in] */
    const B_AspOutputConnectHttpSettings    *pSettings          /* [in] */
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);

    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspOutput can't be NULL" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: socketFdToOffload=%d pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, socketFdToOffload, (void *)pSettings ));
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
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Validate state. */
    B_ASP_CHECK_GOTO( hAspOutput->state == B_AspOutputState_eIdle,
                ("hAspOutput=%p: can't call B_AspOutput_ConnectHttp() in any state other than B_AspOutputState_eIdle, current state=%d", (void *)hAspOutput, hAspOutput->state),
                error_class_unlock, nrc, NEXUS_NOT_SUPPORTED);

    /* Save the connect settings. */
    {
        if (!pSettings)
        {
            B_AspOutput_GetDefaultConnectHttpSettings(&hAspOutput->connectHttpSettings);
        }
        else
        {
            hAspOutput->connectHttpSettings = *pSettings;
        }
        hAspOutput->socketFdToOffload = socketFdToOffload;
    }

    /* Now process the API. */
    {
        nrc = processConnectHttpApi_locked(hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: processConnectHttpApi_locked() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);

        /* This object is now connected. Caller should now call _Start() to start streaming. */
        hAspOutput->state = B_AspOutputState_eConnected;
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p is connected, socketFdToOffload=%d connectionLost=%u" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, socketFdToOffload, hAspOutput->socketState.connectionLost ));
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return (nrc);
} /* B_AspOutput_ConnectHttp */


#if 0
/**
Summary:
API to receive an HTTP request from  the network.

This is an optional API that is NOT needed if App chooses to directly receive the HTTP Request
using Linux socket APIs & before calling B_AspOutput_Connect().

This API is intended to be called after the application receives a B_AspOutputSettings.httpRequestDataReady callback.

If this API indicates that it has more available data (via pMoreAvailable), the caller should call this API again to receive additional data.

In the case that a partial HTTP request is returned, the caller should wait for the next httpRequestDataReady callback,
then call this API again to receive additional data.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.

**/
NEXUS_Error B_AspOutput_RecvHttpRequestData(
    B_AspOutputHandle                       hAspOutput,
    void                                    *pBuffer,       /* [out] attr{memory=cached} pointer to buffer to be filled with the HTTP request. */
    unsigned                                bufferSize,     /* [in]  size of the buffer (in bytes). */
    unsigned                                *pByteCount,    /* [out] number of bytes that has been written to the buffer. */
    bool                                    *pMoreAvailable /* [out] if true, it indicates that more data is available. */
    );

#endif

/**
Summary:
API to provide an HTTP response to be sent out on the network.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.

**/
NEXUS_Error B_AspOutput_SendHttpResponse(
    B_AspOutputHandle                       hAspOutput,
    void                                    *pBuffer,       /* [in] attr{memory=cached} pointer to HTTP response to be sent to network. */
    unsigned                                byteCount       /* [in] number of bytes in HTTP Response buffer. */
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pBuffer);
    BDBG_ASSERT(byteCount);

    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspOutput can't be NULL" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: pBuffer=%p byteCount=%u" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, (void *)pBuffer, byteCount ));

    nrc = NEXUS_AspOutput_SendHttpResponse(hAspOutput->hNexusAspOutput, pBuffer, byteCount);

    return (nrc);
}

/**
Summary:
Get Default StartSettings.
**/
void B_AspOutput_GetDefaultStartSettings(
    B_AspOutputStartSettings                *pSettings      /* [out] */
    )
{
    BDBG_ASSERT(pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->feedMode = B_AspOutputFeedMode_eAuto;
} /* B_AspOutput_GetDefaultStartSettings */


static void mapFeedModeToNexus(
    NEXUS_AspOutputFeedMode *pDst,
    B_AspOutputFeedMode *pSrc
    )
{
    switch(*pSrc)
    {
        case B_AspOutputFeedMode_eAuto:
            *pDst = NEXUS_AspOutputFeedMode_eAuto;
            break;
        case B_AspOutputFeedMode_eHost:
            *pDst = NEXUS_AspOutputFeedMode_eHost;
            break;
        case B_AspOutputFeedMode_eAutoWithHostEncryption:
            *pDst = NEXUS_AspOutputFeedMode_eAutoWithHostEncryption;
            break;
        default:
            *pDst = NEXUS_AspOutputFeedMode_eAuto;
            break;
    }
} /* mapFeedModeToNexus */

/**
Summary:
Start an AspOutput.

AspOutput will start streaming out after this API returns.

Note: This API can only be called when B_AspOutputStatus.state is _eConnected.
AspOutput changes its state to _eStreaming when it returns from this API. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

**/
NEXUS_Error B_AspOutput_Start(
    B_AspOutputHandle                       hAspOutput,
    const B_AspOutputStartSettings          *pSettings
    )
{
    NEXUS_Error nrc;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pSettings);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: pSettings=%p" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, (void *)pSettings));
    if (!hAspOutput || !pSettings)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p pSettings=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, (void *)pSettings,
                    hAspOutput==NULL?"hAspOutput can't be NULL ":"",
                    pSettings==NULL?"pSettings can't be NULL ":""));
        return (NEXUS_NOT_SUPPORTED);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    B_ASP_CHECK_GOTO( hAspOutput->state == B_AspOutputState_eConnected,
                ("hAspOutput=%p: can't call B_AspOutput_Start() in any state other than B_AspOutputState_eConnected, current state=%d", (void *)hAspOutput, hAspOutput->state),
                error_class_unlock, nrc, NEXUS_NOT_SUPPORTED);

    /* Validate settings. */
    {
        if (pSettings->feedMode != B_AspOutputFeedMode_eHost && !hAspOutput->connectHttpSettings.hRecpump)
        {
            BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: connectHttpSettings->hRecpump can't be NULL for this pSettings->feedMode=%u" B_ASP_MSG_PRE_ARG,
                        (void *)hAspOutput, pSettings->feedMode ));
            nrc = NEXUS_NOT_SUPPORTED;
            goto error_class_unlock;
        }
        hAspOutput->startSettings = *pSettings;
    }

    /* Setup the callbacks with Nexus. */
    {
        NEXUS_AspOutputSettings settings;

        NEXUS_AspOutput_GetSettings(hAspOutput->hNexusAspOutput, &settings);

        settings.endOfStreaming.callback = endOfStreamingCallback;
        settings.endOfStreaming.context = hAspOutput;

        settings.remoteDoneSending.callback = remoteDoneSendingCallback;
        settings.remoteDoneSending.context = hAspOutput;

        settings.bufferReady.callback = bufferReadyCallback;
        settings.bufferReady.context = hAspOutput;

        settings.httpRequestDataReady.callback = httpRequestDataReadyCallback;
        settings.httpRequestDataReady.context = hAspOutput;

        nrc = NEXUS_AspOutput_SetSettings(hAspOutput->hNexusAspOutput, &settings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_SetSettings() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);
    }

    /* Update DRM Settings w/ Nexus AspOutput. */
    if (hAspOutput->drmType == B_AspOutputDrmType_eDtcpIp)
    {
        NEXUS_AspOutputDtcpIpSettings settings;

        NEXUS_AspOutput_GetDtcpIpSettings(hAspOutput->hNexusAspOutput, &settings);

        settings.exchKeyLabel = hAspOutput->dtcpIpSettings.exchKeyLabel;
        settings.emiModes = hAspOutput->dtcpIpSettings.emiModes;
        settings.pcpPayloadSize = hAspOutput->dtcpIpSettings.pcpPayloadSize;
        BKNI_Memcpy(settings.initialNonce, hAspOutput->dtcpIpSettings.initialNonce, sizeof(settings.initialNonce));
        BKNI_Memcpy(settings.exchKey, hAspOutput->dtcpIpSettings.exchKey, sizeof(settings.exchKey));

        nrc = NEXUS_AspOutput_SetDtcpIpSettings(hAspOutput->hNexusAspOutput, &settings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_SetDtcpIpSettings() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);
    }

    /* Start AspOutput. */
    {
        NEXUS_AspOutputStartSettings startSettings;

        mapFeedModeToNexus(&startSettings.feedMode, &hAspOutput->startSettings.feedMode);

        nrc = NEXUS_AspOutput_Start( hAspOutput->hNexusAspOutput, &startSettings);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_Start() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);
    }

    /* All is well, so transition to _eStreaming state. */
    {
        hAspOutput->state = B_AspOutputState_eStreaming;
        BDBG_WRN((B_ASP_MSG_PRE_FMT "hAspOutput=%p streaming stated!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return (nrc);
} /* B_AspOutput_Start */


/**
Summary:
API to Get Current Status.
**/
NEXUS_Error B_AspOutput_GetStatus(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputStatus                       *pStatus     /* [out] */
    )
{
    NEXUS_Error nrc;
    NEXUS_AspOutputHttpStatus   nexusHttpStatus;
    NEXUS_AspOutputStatus       nexusStatus;

    BDBG_ASSERT(hAspOutput);
    BDBG_ASSERT(pStatus);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: pStatus=%p!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, (void *)pStatus));
    if (!hAspOutput || !pStatus)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "hAspOutput=%p pStatus=%p %s%s" B_ASP_MSG_PRE_ARG,
                    (void *)hAspOutput, (void *)pStatus,
                    hAspOutput==NULL?"hAspOutput can't be NULL ":"",
                    pStatus==NULL?"pStatus can't be NULL ":""));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Get current status from Nexus & fill-in the caller's structure. */
    {
        nrc = NEXUS_AspOutput_GetStatus(hAspOutput->hNexusAspOutput, &nexusStatus);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_GetStatus() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);

        nrc = NEXUS_AspOutput_GetHttpStatus(hAspOutput->hNexusAspOutput, &nexusHttpStatus);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_GetHttpStatus() Failed", (void *)hAspOutput), error_class_unlock, nrc, nrc);

        pStatus->state              = hAspOutput->state;
        pStatus->aspChannelIndex    = nexusStatus.aspChannelIndex;
        pStatus->bytesStreamed      = nexusHttpStatus.tcp.mcpbConsumedInBytes;
        pStatus->finishRequested    = nexusStatus.finishRequested;
        pStatus->finishCompleted    = nexusStatus.finishCompleted;
        pStatus->connectionReset    = nexusStatus.connectionReset;
        pStatus->networkTimeout     = nexusStatus.networkTimeout;
        pStatus->remoteDoneSending  = nexusStatus.remoteDoneSending;
    }

error_class_unlock:
    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }
error:

    return (NEXUS_SUCCESS);
} /* B_AspOutput_GetStatus */


#if 0
/**
Summary:
API to provide AV buffer to the caller before ASP sends it out.

This is an optional API that is NOT needed in the default feed mode:
    B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAuto.

When B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eHost

    This API returns a buffer available to be filled with data to be output.
    After the caller fills the buffer, it must call B_AspOutput_BufferSubmit()
    to submit the buffer to ASP which will then send it out to the network.

When B_AspOutputStartSettings.feedMode == B_AspOutputFeedMode_eAutoWithHostEncrytion

    This API returns a buffer that contains current data from Recpump before it has
    been sent to the network.  It allows the application to encrypt the data (in place).
    After the caller encrypts the buffer, it must call B_AspOutput_BufferSubmit()
    to return the buffer back to ASP which will then send it out to the network.

Note: This API can only be called when B_AspOutputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspOutput_GetBufferWithWrap(
    B_AspOutputHandle                       hAspOutput,
    void                                    **pBuffer,   /* [out] attr{memory=cached} pointer to data buffer which can be written to network. */
    unsigned                                *pByteCount, /* [out] size of the available space in the pBuffer before the wrap. */
    void                                    **pBuffer2,  /* [out] attr{null_allowed=y;memory=cached} optional pointer to data after wrap. */
    unsigned                                *pByteCount2 /* [out] size of the available space in the pBuffer2 that can be written to network. */
    );

/**
Summary:
API to submit a buffer back to AspOuput.

This allows app to notify AspOutput that app is done with the all or some of the buffer
(indicated via the byteCount parameter).
This buffer was previously obtained using the B_AspOutput_GetBufferWithWrap() API.

Note: This API can only be called when B_AspOutputStatus.state is _eStreaming or _eStreamingDone.

**/
NEXUS_Error B_AspOutput_BufferSubmit(
    B_AspOutputHandle                       hAspOutput,
    unsigned                                byteCount   /* number of bytes that AspOutput can send to the network. */
    );

#endif

/**
Summary:
Stop an AspOutput.

Description:
This API stops the data transmission by the AspOutput.
It does NOT synchronize the protocol state with the network peer.

Note: AspOutput changes its state to _Connected when it returns from this API. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For normal stopping, caller should use B_AspOutput_Finish().

**/
static void processStopApi_locked(
    B_AspOutputHandle                       hAspOutput
    )
{
    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        /* Note: Since App is doing the Stop (which is like aborting the connection), */
        /* we remove the flow lookup entry from switch. This way any incoming packets (such as ACKs) */
        /* are not sent to ASP & thus dont steal its cycles. ASP NetFilter driver will contine to drop */
        /* these packets. */
        if (hAspOutput->switchFlowLabel)
        {
            int retCode;

            retCode = B_Asp_DeleteFlowClassificationRuleFromNwSwitch( &hAspOutput->socketState, hAspOutput->switchFlowLabel);
            if (retCode)
            {
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p B_Asp_DeleteFlowClassificationRuleFromNwSwitch() Failed" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
            }
            else
            {
                BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Removed CFP Entry(=%u) from Switch." B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->switchFlowLabel));
                hAspOutput->switchFlowLabel = 0;
            }
        }

        /* Stop Nexus Output. */
        {
            NEXUS_AspOutput_Stop(hAspOutput->hNexusAspOutput);
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Stopped Nexus AspOutput!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
        }

        /* Stop work is complete, so transition state back to _eConnected. */
        hAspOutput->state = B_AspOutputState_eConnected;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Stopped complete & transitioned to _eConnected state!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: No work to do for B_AspOutput_Stop() in state other than B_AspOutputState_eStreaming, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));
    }

    return;
} /* processStopApi_locked */

void B_AspOutput_Stop(
    B_AspOutputHandle                       hAspOutput
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspOutput);

    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspOutput can't be NULL" B_ASP_MSG_PRE_ARG));
        return;
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: state=%d" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Carry out the stop related work.*/
    {
        processStopApi_locked(hAspOutput);
    }

    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Done!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
    return;
}


/**
Summary:
API to cleanly stop an AspOutput.

Description:
This API initiates the finishing of data transmission by the AspOutput & returns immediately
to the caller.

It will synchronize the protocol state with the network peer (if required for a protocol).
E.g. This may involve waiting for TCP ACKs for any pending data and thus may take time.
AspOutput will invoke the statusChanged callback when finished.

Note: AspOutput changes its state to _Connected when output is cleaned stopped. This can be
checked via the B_AspOutputStatus.state variable obtained from _AspOutput_GetStatus().

For immediate (un-clean) stopping, app should use B_AspOutput_Stop().
**/
void B_AspOutput_Finish(
    B_AspOutputHandle                       hAspOutput
    )
{
    NEXUS_Error     nrc;

    BDBG_ASSERT(hAspOutput);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p:" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));
    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspOutput can't be NULL" B_ASP_MSG_PRE_ARG));
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        /* Request Finish() on Nexus AspOutput. */
        NEXUS_AspOutput_Finish(hAspOutput->hNexusAspOutput);
        hAspOutput->status.finishRequested = true;
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: NEXUS_AspOutput_Finish() work started!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput));

        /* Note: Nexus will issue the endOfStreaming callback when Finishing sequence completes. */
        /* App can then call _GetStatus() to check the completion status. */
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: No work to do for B_AspOutput_Finish() in state other than B_AspOutputState_eStreaming, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));
    }

    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return;
}

/**
Summary:
API to Get Current Status.
**/
void B_AspOutput_PrintStatus(
    B_AspOutputHandle                       hAspOutput
    )
{
    B_Time                  nexusHttpStatusTime;
    uint32_t                txBitRate = 0;
    uint32_t                rxBitRate = 0;
    uint64_t                diffTimeInUs;
    NEXUS_RecpumpStatus     recpumpStatus = {0};
    NEXUS_RecpumpHandle     hRecpump = NULL;
    NEXUS_PlaypumpStatus    playpumpStatus = {0};
    NEXUS_PlaypumpHandle    hPlaypump = NULL;
    NEXUS_Error             nrc;
    NEXUS_AspOutputHttpStatus nexusHttpStatus;
    NEXUS_AspOutputStatus   nexusStatus;
    NEXUS_AspOutputHttpStatus   *pStatus;

    BDBG_ASSERT(hAspOutput);


    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    nrc = NEXUS_AspOutput_GetStatus(hAspOutput->hNexusAspOutput, &nexusStatus);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    nrc = NEXUS_AspOutput_GetHttpStatus(hAspOutput->hNexusAspOutput, &nexusHttpStatus);
    BDBG_ASSERT(nrc == NEXUS_SUCCESS);

    B_Time_Get(&nexusHttpStatusTime);

    pStatus = &nexusHttpStatus;

    /* In order to compute the bit rate, we need to have a prior saved status struct and its associated time.
     * If we haven't saved a status struct for this channel, we'll just say the txBitRate is zero for now.  */
    if ( ! hAspOutput->nexusHttpStatusIsValid)
    {
        txBitRate = 0;        /* No prior sample, assume bit rate is zero. */
        rxBitRate = 0;        /* No prior sample, assume bit rate is zero. */
    }
    else
    {
        /* We have a prior sample, determine it's age. */

        diffTimeInUs = B_Time_DiffMicroseconds(&nexusHttpStatusTime, &hAspOutput->nexusHttpStatusTime);

        /* We only want to use the previous status struct for computing the bit rate
         * if it was from more than a tenth of a second or so... otherwise it can give misleading
         * results (because the byte count seems to increment by 1316 bytes at a time).
         * So anyway, if the time since the last status struct is too small, continue to report
         * the last calculated bit rate (or zero).  */
        if (diffTimeInUs < 100000)
        {
            txBitRate = hAspOutput->txBitRate;      /* use last reported bit rate */
            rxBitRate = hAspOutput->rxBitRate;      /* use last reported bit rate */

#if 1
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld savedBitRate=%u diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        nexusStatus.aspChannelIndex,
                        nexusHttpStatusTime.tv_sec, nexusHttpStatusTime.tv_usec,
                        hAspOutput->nexusHttpStatusTime.tv_sec, hAspOutput->nexusHttpStatusTime.tv_usec,
                        txBitRate,
                        diffTimeInUs ));
#endif
        }
        else
        {
            /* We have a saved status struct that's old enough, use the elapsed time and
             * byte counts to calculate the  bit rate.  */

            txBitRate = (((nexusHttpStatus.tcp.mcpbConsumedInBytes - hAspOutput->nexusHttpStatus.tcp.mcpbConsumedInBytes)*8*1000000)/diffTimeInUs);
            rxBitRate = (((uint64_t)(nexusHttpStatus.tcp.fwStats.rcvdSequenceNumber - hAspOutput->nexusHttpStatus.tcp.fwStats.rcvdSequenceNumber))*8*1000000)/diffTimeInUs;

#if 1
            BDBG_LOG(("%s : %d : Ch=%d nowTimeInMs=%ld %ld prevTimeInMs=%ld %ld now bytes=%lu prev bytes=%lu diffBytes=%lu diffTime=%lu\n",
                        BSTD_FUNCTION, __LINE__,
                        nexusStatus.aspChannelIndex,
                        nexusHttpStatusTime.tv_sec, nexusHttpStatusTime.tv_usec,
                        hAspOutput->nexusHttpStatusTime.tv_sec, hAspOutput->nexusHttpStatusTime.tv_usec,
                        nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        hAspOutput->nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        nexusHttpStatus.tcp.mcpbConsumedInBytes - hAspOutput->nexusHttpStatus.tcp.mcpbConsumedInBytes,
                        diffTimeInUs ));
#endif
        }
    }

    {
        if (hAspOutput->connectHttpSettings.hRecpump)
        {
            hRecpump = hAspOutput->connectHttpSettings.hRecpump;
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
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: NEXUS_Recpump_GetStatus() failed, status=%u" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, nrc));
                goto error_class_unlock;
            }
        }
    }

    {
        if (hAspOutput->connectHttpSettings.hPlaypump)
        {
            hPlaypump = hAspOutput->connectHttpSettings.hPlaypump;
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
                BDBG_WRN(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: NEXUS_Playpump_GetStatus() failed, status=%u" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, nrc));
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
    BDBG_LOG(("RxStats[ch=%d]: p0=%u p7=%u umac=%u edpkt=%u edpktPending=%u p8=%u Discards: p0=%u",
                nexusStatus.aspChannelIndex,
                pStatus->tcp.nwSwRxP0InUnicastIpPkts,
                pStatus->tcp.nwSwTxToAspInUnicastIpPkts,
                pStatus->tcp.unimacRxUnicastIpPkts,
                pStatus->tcp.eDpktRxIpPkts,
                pStatus->tcp.eDpktPendingPkts,
                pStatus->tcp.nwSwRxP8InUnicastIpPkts,
                pStatus->tcp.nwSwRxP0InDiscards
             ));
    BDBG_LOG(("FwStats[ch=%d]: window: congestion=%u rcv=%u send=%u, pkts: sent=%"PRId64 " rcvd=%"PRId64 " dropped=%u dataDropped=%u retx=%d, seq#: send=%x ack=%x rcvd=%x retx=%x",
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
                nexusHttpStatus.tcp.fwStats.retxSequenceNumber
             ));

    hAspOutput->nexusHttpStatus = nexusHttpStatus;
    hAspOutput->nexusHttpStatusIsValid = true;
    hAspOutput->nexusHttpStatusTime = nexusHttpStatusTime;
    hAspOutput->txBitRate = txBitRate;
    hAspOutput->rxBitRate = rxBitRate;

error_class_unlock:
    /* Unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return;
}

/**
Summary:
API to disconnect from AspOutput.

This call transfers the ownership of connection from ASP to Host.
It recreates the connection state back in the Linux socket that was
passed in the B_AspOutput_Connect*().

After this API returns, caller must close this socket using "close()".
TODO: clarify if this socket can be used for further sending or receiving.

**/
static NEXUS_Error processDisconnectApi_locked(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputDisconnectStatus             *pStatus
    )
{
    NEXUS_Error     nrc = NEXUS_SUCCESS;

    if (hAspOutput->state == B_AspOutputState_eConnected)
    {
        /* Disconnect from Nexus AspOutput. */
        {
            NEXUS_AspOutputDisconnectStatus     disconnectStatus;

            nrc = NEXUS_AspOutput_Disconnect(hAspOutput->hNexusAspOutput, &disconnectStatus);
            B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("hAspOutput=%p: NEXUS_AspOutput_Disconnect() Failed", (void *)hAspOutput), error, nrc, nrc);
        }

        /* Offload connection back to Linux if needed. */
        if (pStatus && !hAspOutput->socketState.connectionLost)
        {
            int retCode;

            retCode = B_Asp_SetSocketStateToLinux(&hAspOutput->socketState, hAspOutput->fdMigratedConnx, pStatus->pSocketFd);
            B_ASP_CHECK_GOTO(retCode == 0, ("hAspOutput=%p: B_Asp_SetSocketStateToLinux() Failed", (void *)hAspOutput), error, nrc, NEXUS_OS_ERROR);
        }

        /* Close ASP NetFilter Driver. */
        {
            BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: Closing ASP NetFilterDriver socketFd=%d!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->aspNetFilterDrvFd));
            if (hAspOutput->aspNetFilterDrvFd) close(hAspOutput->aspNetFilterDrvFd);
        }

        /* Reset states back to Idle. */
        {
            hAspOutput->state = B_AspOutputState_eIdle;
            hAspOutput->drmType = B_AspOutputDrmType_eNone;
            hAspOutput->nexusHttpStatusIsValid = false;
        }
    }
    else
    {
        BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: No work to do for B_AspOutput_Disconnect() in state other than B_AspOutputState_eConnected, current state=%d"
                    B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));
    }
error:
    return (nrc);
} /* processDisconnectApi_locked */


NEXUS_Error B_AspOutput_Disconnect(
    B_AspOutputHandle                       hAspOutput,
    B_AspOutputDisconnectStatus             *pStatus
    )
{
    NEXUS_Error nrc = NEXUS_SUCCESS;

    BDBG_ASSERT(hAspOutput);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: pStatus=%p!" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, (void *)pStatus));
    if (!hAspOutput)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT " hAspOutput can't be NULL!" B_ASP_MSG_PRE_ARG));
        return (NEXUS_INVALID_PARAMETER);
    }

    /* Lock the class. */
    {
        nrc = B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_LOCK_AND_CHECK_INSTANCE() Failed"), error, nrc, nrc);
    }

    /* Stop the object if app didn't explicitly call _Stop(). */
    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        processStopApi_locked(hAspOutput);
    }

    /* Carry out the disconnect related work. */
    {
        processDisconnectApi_locked(hAspOutput, pStatus);
    }

    /* And then unlock the class. */
    {
        B_ASP_CLASS_UNLOCK(B_Asp_Output, hAspOutput);
    }

error:
    return (nrc);
} /* B_AspOutput_Disconnect */

/**
Summary:
Destroy AspOutput object.

Description:
Stops and Destroys an AspOutput. The handle can no longer be used.

**/
void B_AspOutput_Destroy(
    B_AspOutputHandle                       hAspOutput
    )
{
    NEXUS_Error     nrc;

    if (!hAspOutput) return;

    /* Object is being destroyed, so remove it from the class list. */
    /* This was any further callbacks from Nexus will be ignored. */
    {
        nrc = B_ASP_CLASS_REMOVE_INSTANCE(B_Asp_Output, hAspOutput);
        B_ASP_CHECK_GOTO(nrc == NEXUS_SUCCESS, ("B_ASP_CLASS_REMOVE_INSTANCE() Failed"), error, nrc, nrc);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "hAspOutput=%p: state=%d" B_ASP_MSG_PRE_ARG, (void *)hAspOutput, hAspOutput->state ));

    if (hAspOutput->state == B_AspOutputState_eStreaming)
    {
        processStopApi_locked(hAspOutput);
    }

    /* Carry out the disconnect related work. */
    if (hAspOutput->state == B_AspOutputState_eConnected)
    {
        processDisconnectApi_locked(hAspOutput, NULL);
    }

    if (hAspOutput->hNexusAspOutput) NEXUS_AspOutput_Destroy(hAspOutput->hNexusAspOutput);

error:
    BKNI_Free(hAspOutput);

    return;
} /* B_AspOutput_Destroy */
