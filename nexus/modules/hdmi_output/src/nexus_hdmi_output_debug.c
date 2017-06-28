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


static void NEXUS_HdmiOutput_PrintEotfSupport(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
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
        hdmiOutput = NEXUS_HdmiOutput_P_GetHandle(i) ;
        if (!hdmiOutput)
        {
            BDBG_ERR(("Inavid HdmiOutput handle")) ;
            errCode = BERR_TRACE(NEXUS_NOT_INITIALIZED) ;
            continue ;
        }

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
    NEXUS_HdmiOutputStatus hdmiOutputStatus ;
    NEXUS_HdmiOutputHdcpStatus hdmiOutputHdcpStatus ;

    BHDM_Handle hdmHandle ;
#if BHDM_HAS_HDMI_20_SUPPORT
    BHDM_SCDC_StatusControlData scdcControlData ;
#endif
    BAVC_HDMI_DRMInfoFrame dynamicRangeMetadataInfoFrame ;
    unsigned i ;


    for (i=0 ; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
    {
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

            /* Display Packet - DRM (displayed if DRM Static Metadata Block exists */
            /* DRM Packets are transmitted to HDR Capable TVs Only */

            if (!hdmiOutput->drm.hdrdb.valid)
            {
                BDBG_LOG(("   Attached Rx <%s> does not support HDR",
                    hdmiOutputStatus.monitorName)) ;
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
                if (hdmiOutput->dbv.enabled)
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
