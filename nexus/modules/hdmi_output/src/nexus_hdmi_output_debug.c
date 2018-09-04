/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "nexus_hdmi_output_module.h"
#include "priv/nexus_hdmi_output_priv.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_core_audio.h"
#include "priv/nexus_core.h"
#include "bhdm.h"
#include "bhdm_edid.h"
#include "bhdm_hdcp.h"
#include "bhdm_scdc.h"
#include "bhdm_auto_i2c.h"
#include "bhdm_monitor.h"
#include "bavc_hdmi.h"
#include "priv/nexus_hdmi_output_mhl_priv.h"


BDBG_MODULE(nexus_hdmi_output_debug);

extern NEXUS_HdmiOutput g_hdmiOutputs[NEXUS_NUM_HDMI_OUTPUTS];


void NEXUS_HdmiOutput_PrintAudioInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    BHDM_Status *hdmiStatus = NULL ;
    BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame = NULL ;
    uint8_t i ;

    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiStatus = BKNI_Malloc(sizeof(*hdmiStatus));
    if (hdmiStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    stAudioInfoFrame = BKNI_Malloc(sizeof(*stAudioInfoFrame));
    if (stAudioInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, hdmiStatus) ;

        if ((hdmiOutputStatus->connected) && (hdmiOutputStatus->rxPowered))
        {
            errCode = BHDM_GetAudioInfoFramePacket(hdmiOutput->hdmHandle , stAudioInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayAudioInfoFramePacket(&hdmiStatus->stPort, stAudioInfoFrame) ;
        }
     }
done:
    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

    if (stAudioInfoFrame)
        BKNI_Free(stAudioInfoFrame) ;

    if (hdmiStatus)
        BKNI_Free(hdmiStatus) ;

#endif
}



void NEXUS_HdmiOutput_PrintAviInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    BHDM_Status *hdmiStatus = NULL ;
    BAVC_HDMI_AviInfoFrame *stAviInfoFrame = NULL ;
    uint8_t i ;

    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiStatus = BKNI_Malloc(sizeof(*hdmiStatus)) ;
    if (hdmiStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    stAviInfoFrame = BKNI_Malloc(sizeof(*stAviInfoFrame));
    if (stAviInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, hdmiStatus) ;

        if ((hdmiOutputStatus->connected) && (hdmiOutputStatus->rxPowered))
        {
            errCode = BHDM_GetAVIInfoFramePacket(hdmiOutput->hdmHandle, stAviInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayAVIInfoFramePacket(&hdmiStatus->stPort, stAviInfoFrame);
        }
     }

done:
    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

    if (hdmiStatus)
        BKNI_Free(hdmiStatus) ;

    if (stAviInfoFrame)
        BKNI_Free(stAviInfoFrame) ;

#endif
}


void NEXUS_HdmiOutput_PrintVendorSpecificInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    BHDM_Status *hdmiStatus = NULL ;
    BAVC_HDMI_VendorSpecificInfoFrame *stVsInfoFrame = NULL ;
    uint8_t i ;

    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiStatus = BKNI_Malloc(sizeof(*hdmiStatus));
    if (hdmiStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    stVsInfoFrame= BKNI_Malloc(sizeof(*stVsInfoFrame));
    if (stVsInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, hdmiStatus) ;

        if ((hdmiOutputStatus->connected) && (hdmiOutputStatus->rxPowered))
        {
            BHDM_GetVendorSpecificInfoFrame(hdmiOutput->hdmHandle , stVsInfoFrame) ;

            BAVC_HDMI_DisplayVendorSpecificInfoFrame(&hdmiStatus->stPort, stVsInfoFrame) ;
        }
     }

done:
    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

    if (hdmiStatus)
        BKNI_Free(hdmiStatus) ;

    if (stVsInfoFrame)
        BKNI_Free(stVsInfoFrame) ;

#endif
}


void NEXUS_HdmiOutput_PrintDrmInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    BHDM_Status *hdmiStatus = NULL ;
    BHDM_EDID_HDRStaticDB *hdrdb = NULL ;
    BAVC_HDMI_DRMInfoFrame *dynamicRangeMetadataInfoFrame = NULL ;
    uint8_t i ;

    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiStatus = BKNI_Malloc(sizeof(*hdmiStatus));
    if (hdmiStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdrdb = BKNI_Malloc(sizeof(*hdrdb));
    if (hdrdb == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    dynamicRangeMetadataInfoFrame = BKNI_Malloc(sizeof(*dynamicRangeMetadataInfoFrame));
    if (dynamicRangeMetadataInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, hdmiStatus) ;

        if ((hdmiOutputStatus->connected) && (hdmiOutputStatus->rxPowered))
        {
            /* Display Packet - DRM (displayed if DRM Static Metadata Block exists */
            /* DRM Packets are transmitted to HDR Capable TVs Only */
            errCode = BHDM_EDID_GetHdrStaticMetadatadb(hdmiOutput->hdmHandle, hdrdb) ;
            /* if error retrieving HdrDB, trace error and continue on */
            if (errCode) {BERR_TRACE(errCode) ; }

            if (hdrdb->valid)
            {
                BHDM_GetDRMInfoFramePacket(hdmiOutput->hdmHandle , dynamicRangeMetadataInfoFrame) ;

                BAVC_HDMI_DisplayDRMInfoFramePacket(&hdmiStatus->stPort, dynamicRangeMetadataInfoFrame) ;
            }
        }
     }

done:
    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

    if (hdmiStatus)
        BKNI_Free(hdmiStatus) ;

    if (hdrdb)
        BKNI_Free(hdrdb) ;

    if (dynamicRangeMetadataInfoFrame)
        BKNI_Free(dynamicRangeMetadataInfoFrame) ;

#endif
}


void NEXUS_HdmiOutput_PrintAcrPacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    BHDM_Status *hdmiStatus = NULL ;
    uint8_t i ;

    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiStatus = BKNI_Malloc(sizeof(*hdmiStatus));
    if (hdmiStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, hdmiStatus) ;

        if ((hdmiOutputStatus->connected) && (hdmiOutputStatus->rxPowered))
        {
            BHDM_PACKET_ACR_DisplayConfiguration(hdmiOutput->hdmHandle, &hdmiStatus->stAcrPacketConfig) ;
        }
     }
done:
    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

    if (hdmiStatus)
        BKNI_Free(hdmiStatus) ;

#endif
}


void NEXUS_HdmiOutput_PrintRxEdid(void)
{
#if !BDBG_NO_LOG
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        if (!g_hdmiOutputs[i].opened) continue;
        BHDM_EDID_DEBUG_PrintData(g_hdmiOutputs[i].hdmHandle) ;
    }
#endif
}


static void NEXUS_HdmiOutput_PrintEotfSupport(void)
{
#if !BDBG_NO_LOG
    NEXUS_HdmiOutputHandle hdmiOutput ;

    /* Max EOTF Descrptior Size
    ** largest EOTF descriptor can be 10 bytes
    ** add 2 bytes for delimiters e.g. <eotf>
    */
    #define MAX_EOTF_DESC_SIZE 12
    static const char *g_eotfDescriptors[BAVC_HDMI_DRM_EOTF_eMax][MAX_EOTF_DESC_SIZE]
        = {{"SDR"}, {"HDR Gamma"}, {"HDR10"}, {"HLG"}} ;

    static const char pchEotfPrefix[] = "   EDID Supported Eotf: " ;
    #define MAX_EOTF_STRING_BUFFER \
        (sizeof(pchEotfPrefix) + (NEXUS_VideoEotf_eMax * MAX_EOTF_DESC_SIZE))

    char pchEotfSupport[MAX_EOTF_STRING_BUFFER];
    unsigned strOffset ;

    uint8_t i, j ;


    for (i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;
        if (!hdmiOutput->opened) continue;

        /* reset eotf string length for new string */
        strOffset = 0 ;
        strOffset += BKNI_Snprintf(pchEotfSupport+strOffset,
            sizeof (pchEotfSupport) - strOffset, pchEotfPrefix) ;

        for (j = 0; j < BAVC_HDMI_DRM_EOTF_eMax; j++)
        {
            if (hdmiOutput->drm.hdrdb.bEotfSupport[j] && (j == BAVC_HDMI_DRM_EOTF_eHDR))
                continue ;  /* leave HDR(gamma) out */

            strOffset += BKNI_Snprintf(pchEotfSupport+strOffset,
                sizeof (pchEotfSupport) - strOffset, "<%s> ", *g_eotfDescriptors[j]) ;

            /* make sure adding the next EOTF type does not exceed MAX_EOTF_STRING_BUFFER size */
            if ((strOffset + MAX_EOTF_DESC_SIZE) > (unsigned) MAX_EOTF_STRING_BUFFER)
            {
                BDBG_WRN(("   %s may not contain all supported Eotfs", pchEotfPrefix)) ;
                break ;
            }
        }
        BDBG_LOG(("%s", pchEotfSupport));

        BDBG_LOG(("   EDID Supported Luminance:")) ;
        BDBG_LOG(("      Min=%d, MinValid=%d, Avg=%d, AvgValid=%d, Max=%d, MaxValid=%d",
            hdmiOutput->drm.hdrdb.MinLuminance, hdmiOutput->drm.hdrdb.MinLuminanceValid,
            hdmiOutput->drm.hdrdb.AverageLuminance, hdmiOutput->drm.hdrdb.AverageLuminanceValid,
            hdmiOutput->drm.hdrdb.MaxLuminance, hdmiOutput->drm.hdrdb.MaxLuminanceValid)) ;
    }
#endif
}


void NEXUS_HdmiOutputModule_Print(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL ;
    NEXUS_HdmiOutputHdcpStatus *hdmiOutputHdcpStatus = NULL ;
    BHDM_MONITOR_TxHwStatusExtra *txHwStatusExtra = NULL ;

    BHDM_Handle hdmHandle ;
#if BHDM_HAS_HDMI_20_SUPPORT
    BHDM_SCDC_StatusControlData *scdcControlData = NULL ;
#endif
    BAVC_HDMI_DRMInfoFrame dynamicRangeMetadataInfoFrame ;
    unsigned i ;


    hdmiOutputStatus = BKNI_Malloc(sizeof(*hdmiOutputStatus));
    if (hdmiOutputStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    hdmiOutputHdcpStatus = BKNI_Malloc(sizeof(*hdmiOutputHdcpStatus));
    if (hdmiOutputHdcpStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

#if BHDM_HAS_HDMI_20_SUPPORT
    scdcControlData = BKNI_Malloc(sizeof(*scdcControlData));
    if (scdcControlData == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }
#endif

    txHwStatusExtra = BKNI_Malloc(sizeof(*txHwStatusExtra));
    if (txHwStatusExtra == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }



    for (i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        if (!hdmiOutput->opened) continue;
        BDBG_OBJECT_ASSERT(hdmiOutput, NEXUS_HdmiOutput);
        hdmHandle = hdmiOutput->hdmHandle ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        errCode = NEXUS_HdmiOutput_GetHdcpStatus(hdmiOutput, hdmiOutputHdcpStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        BHDM_MONITOR_GetTxHwStatusExtra(hdmiOutput->hdmHandle, txHwStatusExtra) ;

        BDBG_LOG(("HDMI %d:%s%s%s",i,
            hdmiOutput->opened ? "o" : "-",
            hdmiOutput->videoConnected ? "v" : "-",
            hdmiOutput->hdcpStarted ? "E" : "-"));

        if (hdmiOutput->videoConnected)
        {
            BDBG_LOG((" video: s:%p d:%p",hdmiOutput->videoConnector.source,hdmiOutput->videoConnector.destination));
            BDBG_LOG((" audio: t:%d o:%p md:%p p:%lu", hdmiOutput->audioConnector.objectType,(void *)hdmiOutput->audioConnector.pObjectHandle,(void *)hdmiOutput->audioConnector.pMixerData,(unsigned long)hdmiOutput->audioConnector.port));
        }

        if (hdmiOutputStatus->connected)
        {
            BDBG_LOG(("Attached device: %s", hdmiOutputStatus->monitorName)) ;

            BDBG_LOG(("  rxAttached: %c   rxPowered: %c",
                hdmiOutputStatus->connected ? 'Y' : 'N',
                hdmiOutputStatus->rxPowered ? 'Y' : 'N')) ;

            /* HARDWARE STATUS */
            BDBG_LOG(("  txPower  Clock: %c CH2: %c CH1: %c CH0: %c",
                hdmiOutput->txHwStatus.clockPower ? 'Y' : 'N',
                hdmiOutput->txHwStatus.channelPower[2] ? 'Y' : 'N',
                hdmiOutput->txHwStatus.channelPower[1] ? 'Y' : 'N',
                hdmiOutput->txHwStatus.channelPower[0] ? 'Y' : 'N')) ;

            BDBG_LOG(("  txPLL Locked: %c  txPLL Status: %#x",
                txHwStatusExtra->PllLocked ? 'Y' : 'N',
                txHwStatusExtra->PllStatus)) ;
        }
        else
        {
	        BDBG_LOG(("no device attached")) ;
        }

        BDBG_LOG(("  Total RxSense Changes:   %d",
            hdmiOutput->txHwStatus.rxSenseCounter)) ;
        BDBG_LOG(("  Total HP Changes:        %d",
            hdmiOutput->txHwStatus.hotplugCounter)) ;
        BDBG_LOG(("  Total Unstable Format Detected Count: %d",
            hdmiOutput->txHwStatus.unstableFormatDetectedCounter)) ;


        if (hdmiOutputStatus->connected)
        {
            BDBG_LOG(("HDMI Settings:")) ;
            BDBG_LOG(("  ColorSpace: %s",
                NEXUS_HdmiOutput_P_ColorSpace_ToText(hdmiOutput->displaySettings.colorSpace))) ;

            BDBG_LOG(("  ColorDepth: %d ", hdmiOutput->displaySettings.colorDepth)) ;

            BDBG_LOG(("  Matrix Coefficient ID: %d  Override: %s",
                hdmiOutput->displaySettings.eColorimetry,
                hdmiOutput->displaySettings.overrideMatrixCoefficients ? "Yes" : "No")) ;

            BDBG_LOG(("  Color Range: %s  Override: %s",
                hdmiOutput->displaySettings.colorRange ? "Limited"
                : (hdmiOutput->displaySettings.colorRange ==  NEXUS_ColorRange_eFull) ? "Full"
                : "Unknown",
                hdmiOutput->displaySettings.overrideColorRange ? "Yes" : "No")) ;

# if BHDM_HAS_HDMI_20_SUPPORT
            BHDM_SCDC_GetStatusControlData(hdmHandle, scdcControlData) ;

            if (scdcControlData->valid)
            {
                BDBG_LOG(("SCDC rxStatus:")) ;
                BDBG_LOG(("  Clock Detected: %c", scdcControlData->Clock_Detected ? 'Y' : 'N')) ;
                BDBG_LOG(("  Channel Locked? Ch0: %c  Ch1: %c  Ch2: %c  ",
                    scdcControlData->Ch0_Locked ? 'Y' : 'N',
                    scdcControlData->Ch1_Locked ? 'Y' : 'N',
                    scdcControlData->Ch2_Locked ? 'Y' : 'N')) ;

                BDBG_LOG(("  Tx Scramblng:   %s", hdmiOutput->txHwStatus.scrambling ? "Yes" : "No")) ;
                BDBG_LOG(("  Rx De-Scramblng: %s", hdmiOutput->rxHwStatus.descrambling ? "Yes" : "No")) ;
            }
            else
            {
                BDBG_LOG(("SCDC rxStatus: Unavailable")) ;
            }
#else
            BDBG_LOG(("SCDC not supported on this platform")) ;
#endif

            /* HDCP Status */
            BDBG_LOG(("HDCP Status:")) ;
            BDBG_LOG(("  Connected device: %s",
                hdmiOutputHdcpStatus->isHdcpRepeater ? "Repeater" : "Receiver")) ;
            BDBG_LOG(("  Supported Version: %s",
                hdmiOutputHdcpStatus->hdcp2_2Features ? "2.2" : "1.x")) ;

            BDBG_LOG(("  HDCP2Version i2c read failures: %d",
                txHwStatusExtra->ui2cHdcp2VersionReadFailures)) ;
            BDBG_LOG(("  HDCP2Version invalid data failures: %d",
                txHwStatusExtra->ui2cHdcp2VersionDataFailures)) ;

            if ((hdmiOutputHdcpStatus->hdcp2_2Features) && (hdmiOutputHdcpStatus->isHdcpRepeater))
            {
                BDBG_LOG(("  Downstream 1.x device(s): %s",
                hdmiOutputHdcpStatus->hdcp2_2RxInfo.hdcp1_xDeviceDownstream ? "Yes" : "No")) ;
            }

            BDBG_LOG(("  Authenticated: %s",
                hdmiOutputHdcpStatus->linkReadyForEncryption ? "Yes" : "No")) ;
            BDBG_LOG(("  Transmitting Encrypted: %s",
                hdmiOutputHdcpStatus->transmittingEncrypted ? "Yes" : "No")) ;

            BDBG_LOG(("  Current State: %d   Last Error: %d",
                hdmiOutputHdcpStatus->hdcpState, hdmiOutputHdcpStatus->hdcpError)) ;

            BDBG_LOG(("  1.x Stats")) ;
            BDBG_LOG(("    Attempts: %5d  Pass: %5d  Fail: %5d",
                hdmiOutput->hdcpMonitor.hdcp1x.auth.attemptCounter,
                hdmiOutput->hdcpMonitor.hdcp1x.auth.passCounter,
                hdmiOutput->hdcpMonitor.hdcp1x.auth.failCounter)) ;
            BDBG_LOG(("    BCaps Read Failures: %d",
                hdmiOutput->hdcpMonitor.hdcp1x.bCapsReadFailureCounter)) ;
            BDBG_LOG(("    BKSV Read Failures: %d",
                hdmiOutput->hdcpMonitor.hdcp1x.bksvReadFailureCounter)) ;
            BDBG_LOG(("    Invalid BKSV Detected: %d",
                hdmiOutput->hdcpMonitor.hdcp1x.invalidBksvCounter)) ;
 #if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
            BDBG_LOG(("  2.2 Stats")) ;
            BDBG_LOG(("    Attempts: %5d  Pass: %5d  Fail: %5d",
                hdmiOutput->hdcpMonitor.hdcp22.auth.attemptCounter,
                hdmiOutput->hdcpMonitor.hdcp22.auth.passCounter,
                hdmiOutput->hdcpMonitor.hdcp22.auth.failCounter)) ;
            BDBG_LOG(("    ReAuth Requests Valid: %d  Invalid: %d",
                hdmiOutput->hdcpMonitor.hdcp22.validReauthReqCounter,
                hdmiOutput->hdcpMonitor.hdcp22.invalidReauthReqCounter)) ;
            BDBG_LOG(("    Watchdog Counter: %d",
                hdmiOutput->hdcpMonitor.hdcp22.watchdogCounter)) ;
            BDBG_LOG(("    Timeout Counter: %d",
                hdmiOutput->hdcpMonitor.hdcp22.timeoutCounter)) ;
 #endif

            BDBG_LOG(("HDR Status:")) ;

            /* Display Packet - DRM (displayed if DRM Static Metadata Block exists */
            /* DRM Packets are transmitted to HDR Capable TVs Only */

            if (!hdmiOutput->drm.hdrdb.valid)
            {
                BDBG_LOG(("   Attached Rx <%s> does not support HDR",
                    hdmiOutputStatus->monitorName)) ;
            }
            else
            {
                static const char pchOutputPrefix[] = "   Output Dynamic Range: " ;
                #define MAX_OUTPUT_STRING (sizeof(pchOutputPrefix) + 25 )
                char pchOutputString[MAX_OUTPUT_STRING] ;
                uint8_t strOffset = 0;

                NEXUS_HdmiOutput_PrintEotfSupport() ;


                errCode = BHDM_GetDRMInfoFramePacket(hdmHandle, &dynamicRangeMetadataInfoFrame) ;
                if (errCode) {BERR_TRACE(errCode) ; goto done ;}

                strOffset += BKNI_Snprintf(pchOutputString+strOffset,
                    sizeof (pchOutputString) - strOffset, pchOutputPrefix) ;

 #if NEXUS_DBV_SUPPORT
                BDBG_LOG(("   Dolby Vision Supported: %c",
                    hdmiOutput->dbv.supported ? 'Y' : 'N')) ;

                /* Output Dolby Vision */
                if (hdmiOutput->dbv.state == NEXUS_HdmiOutputDbvState_eEnabled || hdmiOutput->dbv.state == NEXUS_HdmiOutputDbvState_eEnabling)
                {
                    strOffset += BKNI_Snprintf(pchOutputString+strOffset,
                        sizeof (pchOutputString) - strOffset, "Dolby Vision") ;
					BDBG_LOG(("%s", pchOutputString)) ;
                }
                /* Output Dynamic Range */
                else
 #else
                BDBG_LOG(("   Dolby Vision Supported: N"));
 #endif
                {
                    strOffset += BKNI_Snprintf(pchOutputString+strOffset,
                        sizeof (pchOutputString) - strOffset, "<%s>",
                        BAVC_HDMI_DRMInfoFrame_EOTFToStr(dynamicRangeMetadataInfoFrame.eEOTF)) ;

                    BDBG_LOG(("%s", pchOutputString)) ;

                    BDBG_LOG(("   Output Cll: Max=%d Average=%d",
                        dynamicRangeMetadataInfoFrame.Type1.MaxContentLightLevel,
                        dynamicRangeMetadataInfoFrame.Type1.MaxFrameAverageLightLevel)) ;

                    BDBG_LOG(("   Output Mdvc: Red=%d,%d, Green=%d,%d, Blue=%d,%d, White=%d,%d, Luminance(max)=%d, Luminance(min)=%d",
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[0].X,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[0].Y,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[1].X,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[1].Y,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[2].X,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayPrimaries[2].Y,
                        dynamicRangeMetadataInfoFrame.Type1.WhitePoint.X,
                        dynamicRangeMetadataInfoFrame.Type1.WhitePoint.Y,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayMasteringLuminance.Max,
                        dynamicRangeMetadataInfoFrame.Type1.DisplayMasteringLuminance.Min)) ;
                }
            }
        }
        else
        {
            BDBG_LOG(("HDMI Settings Unavailable: no device attached")) ;
        }
        if (!hdmiOutputStatus->txHardwareStatus.hotplugInterruptEnabled)
        {
            BDBG_ERR(("EXCESSIVE HP INTRs (%d) were detected; HP interrupt has been DISABLED",
                hdmiOutput->openSettings.hotplugChangeThreshold)) ;
        }

        BDBG_LOG((" ")) ;
    }

done:
    if (txHwStatusExtra)
        BKNI_Free(txHwStatusExtra) ;

#if BHDM_HAS_HDMI_20_SUPPORT
    if (scdcControlData)
        BKNI_Free(scdcControlData) ;
#endif

    if (hdmiOutputHdcpStatus)
        BKNI_Free(hdmiOutputHdcpStatus) ;

    if (hdmiOutputStatus)
        BKNI_Free(hdmiOutputStatus) ;

#endif
    return ;
}
