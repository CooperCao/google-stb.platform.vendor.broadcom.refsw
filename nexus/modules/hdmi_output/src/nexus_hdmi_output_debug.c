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
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    BAVC_HDMI_AudioInfoFrame stAudioInfoFrame ;
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if ((hdmiOutputStatus.connected) && (hdmiOutputStatus.rxPowered))
        {
            errCode = BHDM_GetAudioInfoFramePacket(hdmiOutput->hdmHandle , &stAudioInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }
            BHDM_DisplayAudioInfoFramePacket(	hdmiOutput->hdmHandle, &stAudioInfoFrame) ;
        }
     }
done: ;
#endif
}



void NEXUS_HdmiOutput_PrintAviInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    BAVC_HDMI_AviInfoFrame stAviInfoFrame ;
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if ((hdmiOutputStatus.connected) && (hdmiOutputStatus.rxPowered))
        {
            errCode = BHDM_GetAVIInfoFramePacket(hdmiOutput->hdmHandle , &stAviInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }
            BHDM_DisplayAVIInfoFramePacket(hdmiOutput->hdmHandle, &stAviInfoFrame) ;
        }
     }
done: ;
#endif
}


void NEXUS_HdmiOutput_PrintVendorSpecificInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    BAVC_HDMI_VendorSpecificInfoFrame stVsInfoFrame ;
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if ((hdmiOutputStatus.connected) && (hdmiOutputStatus.rxPowered))
        {
            BHDM_GetVendorSpecificInfoFrame(hdmiOutput->hdmHandle , &stVsInfoFrame) ;
            BHDM_DisplayVendorSpecificInfoFrame(hdmiOutput->hdmHandle, &stVsInfoFrame) ;
        }
     }
done: ;
#endif
}


void NEXUS_HdmiOutput_PrintDrmInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    BHDM_EDID_HDRStaticDB hdrdb ;
    BAVC_HDMI_DRMInfoFrame dynamicRangeMetadataInfoFrame ;
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if ((hdmiOutputStatus.connected) && (hdmiOutputStatus.rxPowered))
        {
            /* Display Packet - DRM (displayed if DRM Static Metadata Block exists */
            /* DRM Packets are transmitted to HDR Capable TVs Only */
            errCode = BHDM_EDID_GetHdrStaticMetadatadb(hdmiOutput->hdmHandle, &hdrdb) ;
            /* if error retrieving HdrDB, trace error and continue on */
            if (errCode) {BERR_TRACE(errCode) ; }

            if (hdrdb.valid)
            {
                BHDM_GetDRMInfoFramePacket(hdmiOutput->hdmHandle , &dynamicRangeMetadataInfoFrame) ;
                BHDM_DisplayDRMInfoFramePacket(hdmiOutput->hdmHandle, &dynamicRangeMetadataInfoFrame) ;
            }
        }
     }
done: ;
#endif
}


void NEXUS_HdmiOutput_PrintAcrPacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    BHDM_Status hdmiStatus ;
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        hdmiOutput = &g_hdmiOutputs[i] ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if ((hdmiOutputStatus.connected) && (hdmiOutputStatus.rxPowered))
        {
            errCode = BHDM_GetHdmiStatus(hdmiOutput->hdmHandle, &hdmiStatus) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ;}
            BHDM_PACKET_ACR_DisplayConfiguration(hdmiOutput->hdmHandle, &hdmiStatus.stAcrPacketConfig) ;

        }
     }
done: ;
#endif
}


void NEXUS_HdmiOutput_PrintRxEdid(void)
{
#if !BDBG_NO_LOG
    uint8_t i ;

    for (i = 0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
		BHDM_EDID_DEBUG_PrintData(g_hdmiOutputs[i].hdmHandle) ;
    }
#endif
}


void NEXUS_HdmiOutputModule_Print(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputHandle hdmiOutput ;
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus ;

    BHDM_Handle hdmHandle ;
    BHDM_EDID_HDRStaticDB hdrdb ;
#if BHDM_HAS_HDMI_20_SUPPORT
    BHDM_SCDC_StatusControlData scdcControlData ;
#endif
    BAVC_HDMI_DRMInfoFrame dynamicRangeMetadataInfoFrame ;
    unsigned i,j;


    for (i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
        #if !BDBG_NO_LOG
        static const char *g_eotfStr[NEXUS_VideoEotf_eMax] = {"SDR", "HLG", "HDR10", "Invalid"};
        #endif

        hdmiOutput = NEXUS_HdmiOutput_P_GetHandle(i) ;
        hdmHandle = hdmiOutput->hdmHandle ;

        errCode = NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        errCode = NEXUS_HdmiOutput_GetHdcpStatus(hdmiOutput, &hdmiOutputHdcpStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

#if BHDM_HAS_HDMI_20_SUPPORT
        BHDM_SCDC_GetStatusControlData(hdmHandle, &scdcControlData) ;
#endif

        BDBG_LOG(("HDMI %d:%s%s%s",i,
            hdmiOutput->opened ? "o" : "-",
            hdmiOutput->videoConnected ? "v" : "-",
            hdmiOutput->hdcpStarted ? "E" : "-"));

        if (hdmiOutput->videoConnected)
        {
            BDBG_LOG((" video: s:%p d:%p",hdmiOutput->videoConnector.source,hdmiOutput->videoConnector.destination));
            BDBG_LOG((" audio: t:%d o:%p md:%p p:%lu", hdmiOutput->audioConnector.objectType,(void *)hdmiOutput->audioConnector.pObjectHandle,(void *)hdmiOutput->audioConnector.pMixerData,(unsigned long)hdmiOutput->audioConnector.port));
        }

        if (hdmiOutputStatus.connected)
        {
            BDBG_LOG(("Attached device: %s", hdmiOutputStatus.monitorName)) ;



	        /* HARDWARE STATUS */
	        BDBG_LOG(("  rxAttached: %c   rxPowered: %c",
	            hdmiOutputStatus.connected ? 'Y' : 'N',
	            hdmiOutputStatus.rxPowered ? 'Y' : 'N')) ;

	        BDBG_LOG(("  txPower  Clock: %c CH2: %c CH1: %c CH0: %c",
	            hdmiOutput->txHwStatus.clockPower ? 'Y' : 'N',
	            hdmiOutput->txHwStatus.channelPower[2] ? 'Y' : 'N',
	            hdmiOutput->txHwStatus.channelPower[1] ? 'Y' : 'N',
	            hdmiOutput->txHwStatus.channelPower[0] ? 'Y' : 'N')) ;
        }
        else
        {
	        BDBG_LOG(("no device attached")) ;
        }

        BDBG_LOG(("  Total RxSense Changes:   %d",
            hdmiOutput->txHwStatus.rxSenseCounter)) ;
        /* total HP Change count is incorrect, disable */
#if 0
        BDBG_LOG(("  Total HP Changes:        %d",
            hdmiOutput->txHwStatus.hotplugCounter)) ;
#endif
        BDBG_LOG(("  Total Unstable Format Detected Count: %d",
            hdmiOutput->txHwStatus.unstableFormatDetectedCounter)) ;


        if (hdmiOutputStatus.connected)
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
            if (scdcControlData.valid)
            {
                BDBG_LOG(("SCDC rxStatus:")) ;
                BDBG_LOG(("  Clock Detected: %c", scdcControlData.Clock_Detected ? 'Y' : 'N')) ;
                BDBG_LOG(("  Channel Locked? Ch0: %c  Ch1: %c  Ch2: %c  ",
                    scdcControlData.Ch0_Locked ? 'Y' : 'N',
                    scdcControlData.Ch1_Locked ? 'Y' : 'N',
                    scdcControlData.Ch2_Locked ? 'Y' : 'N')) ;

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
                hdmiOutputHdcpStatus.isHdcpRepeater ? "Repeater" : "Receiver")) ;
            BDBG_LOG(("  Supported Version: %s",
                hdmiOutputHdcpStatus.hdcp2_2Features ? "2.2" : "1.x")) ;

            if ((hdmiOutputHdcpStatus.hdcp2_2Features) && (hdmiOutputHdcpStatus.isHdcpRepeater))
            {
                BDBG_LOG(("  Downstream 1.x device(s): %s",
                hdmiOutputHdcpStatus.hdcp2_2RxInfo.hdcp1_xDeviceDownstream ? "Yes" : "No")) ;
            }

            BDBG_LOG(("  Authenticated: %s",
                hdmiOutputHdcpStatus.linkReadyForEncryption ? "Yes" : "No")) ;
            BDBG_LOG(("  Transmitting Encrypted: %s",
                hdmiOutputHdcpStatus.transmittingEncrypted ? "Yes" : "No")) ;

            BDBG_LOG(("  Current State: %d",  hdmiOutputHdcpStatus.hdcpState)) ;
            BDBG_LOG(("  Last Error: %d",  hdmiOutputHdcpStatus.hdcpError)) ;

            BDBG_LOG(("HDR Status:")) ;

            if (!hdrdb.valid)
            {
                BDBG_LOG(("   Attached Rx <%s> does not support HDR",
                    hdmiOutputStatus.monitorName)) ;
            }
            else
            {
#define MAX_EOTF_STRING (NEXUS_VideoEotf_eMax * 10)

                char pchEotfSupport[MAX_EOTF_STRING];
                uint8_t strOffset = 0;

                /* Display Packet - DRM (displayed if DRM Static Metadata Block exists */
                /* DRM Packets are transmitted to HDR Capable TVs Only */
                errCode = BHDM_EDID_GetHdrStaticMetadatadb(hdmHandle, &hdrdb) ;
                /* if error retrieving HdrDB, trace error and continue on */
                if (errCode) {BERR_TRACE(errCode) ; }

                if (hdrdb.valid)
                {
                    errCode = BHDM_GetDRMInfoFramePacket(hdmHandle, &dynamicRangeMetadataInfoFrame) ;
                    if (errCode) {BERR_TRACE(errCode) ; goto done ;}
                }

                /* get information from previous call to BHDM_GetDrmInfoFrame */
                BDBG_LOG(("   Tx Mode: <%s>",
                    BAVC_HDMI_DRMInfoFrame_EOTFToStr(dynamicRangeMetadataInfoFrame.eEOTF))) ;

                strOffset += BKNI_Snprintf(pchEotfSupport+strOffset,
                    sizeof (pchEotfSupport) - strOffset, "   Rx Support: ") ;

                for (j = 0; j < NEXUS_VideoEotf_eMax; j++)
                {
                    if (hdmiOutput->drm.hdrdb.eotfSupported[j])
                    {
                        strOffset += BKNI_Snprintf(pchEotfSupport+strOffset,
                            sizeof (pchEotfSupport) - strOffset, "%s ", g_eotfStr[j]) ;
                        if (strOffset + 10 > MAX_EOTF_STRING)
                        {
                            BDBG_WRN(("   Rx Support may not contain all supported EOFTs")) ;
                            break ;
                        }
                    }
                }
                BDBG_LOG(("%s", pchEotfSupport));
            }

        }
        else
        {
            BDBG_LOG(("HDMI Settings Unavailable: no device attached")) ;
        }
        if (!hdmiOutputStatus.txHardwareStatus.hotplugInterruptEnabled)
        {
            BDBG_ERR(("EXCESSIVE HP INTRs (%d) were detected; HP interrupt has been DISABLED",
                hdmiOutput->openSettings.hotplugChangeThreshold)) ;
        }

        BDBG_LOG((" ")) ;
    }

done:
#endif
	return ;
}
