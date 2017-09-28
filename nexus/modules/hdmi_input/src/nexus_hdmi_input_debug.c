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

#include "nexus_hdmi_input_module.h"
#include "priv/nexus_hdmi_input_priv.h"


BDBG_MODULE(nexus_hdmi_input_debug);

extern NEXUS_gHdmiInput g_NEXUS_hdmiInput;


void NEXUS_HdmiInput_PrintAudioInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    BHDR_Status *pstHdmiRxStatus = NULL ;
    BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame = NULL ;
    uint8_t i ;

    pstHdmiRxStatus = BKNI_Malloc(sizeof(*pstHdmiRxStatus));
    if (pstHdmiRxStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    pstAudioInfoFrame = BKNI_Malloc(sizeof(*pstAudioInfoFrame));
    if (pstAudioInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }


    for (i = 0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        hdmiInput = g_NEXUS_hdmiInput.handle[i] ;
        if (!hdmiInput) continue;

        errCode = BHDR_GetHdmiRxStatus(hdmiInput->hdr, pstHdmiRxStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if (pstHdmiRxStatus->DeviceAttached && pstHdmiRxStatus->bValidStatus)
        {
            errCode = BHDR_GetAudioInfoFrameData(hdmiInput->hdr, pstAudioInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayAudioInfoFramePacket(&pstHdmiRxStatus->stPort, pstAudioInfoFrame) ;
        }
     }
done:
    if (pstHdmiRxStatus)
        BKNI_Free(pstHdmiRxStatus) ;

    if (pstAudioInfoFrame)
        BKNI_Free(pstAudioInfoFrame) ;

#endif
}


void NEXUS_HdmiInput_PrintAviInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    BHDR_Status *pstHdmiRxStatus = NULL ;
    BAVC_HDMI_AviInfoFrame *pstAviInfoFrame = NULL ;
    uint8_t i ;

    pstHdmiRxStatus = BKNI_Malloc(sizeof(*pstHdmiRxStatus));
    if (pstHdmiRxStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    pstAviInfoFrame = BKNI_Malloc(sizeof(*pstAviInfoFrame));
    if (pstAviInfoFrame == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }


    for (i = 0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        hdmiInput = g_NEXUS_hdmiInput.handle[i] ;
        if (!hdmiInput) continue;

        errCode = BHDR_GetHdmiRxStatus(hdmiInput->hdr, pstHdmiRxStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if (pstHdmiRxStatus->DeviceAttached && pstHdmiRxStatus->bValidStatus)
        {
            errCode = BHDR_GetAviInfoFrameData(hdmiInput->hdr, pstAviInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayAVIInfoFramePacket(&pstHdmiRxStatus->stPort, pstAviInfoFrame) ;
        }
     }

done:
    if (pstHdmiRxStatus)
        BKNI_Free(pstHdmiRxStatus) ;

    if (pstAviInfoFrame)
        BKNI_Free(pstAviInfoFrame) ;

#endif
}


void NEXUS_HdmiInput_PrintVendorSpecificInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    BHDR_Status *pstHdmiRxStatus = NULL ;
    BAVC_HDMI_VendorSpecificInfoFrame *pstVsInfoFrame = NULL ;
    uint8_t i ;

    pstHdmiRxStatus = BKNI_Malloc(sizeof(*pstHdmiRxStatus));
    if (pstHdmiRxStatus == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
        goto done ;
    }


    pstVsInfoFrame= BKNI_Malloc(sizeof(*pstVsInfoFrame));
    if (pstVsInfoFrame == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        hdmiInput = g_NEXUS_hdmiInput.handle[i] ;
        if (!hdmiInput) continue;

        errCode = BHDR_GetHdmiRxStatus(hdmiInput->hdr, pstHdmiRxStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if (pstHdmiRxStatus->DeviceAttached && pstHdmiRxStatus->bValidStatus)
        {
            errCode = BHDR_GetVendorSpecificInfoFrameData(hdmiInput->hdr, pstVsInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayVendorSpecificInfoFrame(&pstHdmiRxStatus->stPort, pstVsInfoFrame) ;
        }
     }

done:
    if (pstHdmiRxStatus)
        BKNI_Free(pstHdmiRxStatus) ;

    if (pstVsInfoFrame)
        BKNI_Free(pstVsInfoFrame) ;

#endif
}


void NEXUS_HdmiInput_PrintDrmInfoFramePacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    BHDR_Status *pstHdmiRxStatus = NULL ;
    BAVC_HDMI_DRMInfoFrame *dynamicRangeMetadataInfoFrame = NULL ;
    uint8_t i ;

    pstHdmiRxStatus = BKNI_Malloc(sizeof(*pstHdmiRxStatus));
    if (pstHdmiRxStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    dynamicRangeMetadataInfoFrame = BKNI_Malloc(sizeof(*dynamicRangeMetadataInfoFrame));
    if (dynamicRangeMetadataInfoFrame == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
        goto done ;
    }


    for (i = 0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        hdmiInput = g_NEXUS_hdmiInput.handle[i] ;
        if (!hdmiInput) continue;

        errCode = BHDR_GetHdmiRxStatus(hdmiInput->hdr, pstHdmiRxStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if (pstHdmiRxStatus->DeviceAttached && pstHdmiRxStatus->bValidStatus)
        {
            errCode = BHDR_GetDrmiInfoFrameData(hdmiInput->hdr, dynamicRangeMetadataInfoFrame) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ; }

            BAVC_HDMI_DisplayDRMInfoFramePacket(&pstHdmiRxStatus->stPort, dynamicRangeMetadataInfoFrame) ;
        }
     }
done:
    if (pstHdmiRxStatus)
        BKNI_Free(pstHdmiRxStatus) ;

    if (dynamicRangeMetadataInfoFrame)
        BKNI_Free(dynamicRangeMetadataInfoFrame) ;
#endif
}


void NEXUS_HdmiInput_PrintACRPacket(void)
{
#if !BDBG_NO_LOG
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;
    BHDR_Status *pstHdmiRxStatus = NULL ;
    BAVC_HDMI_AudioClockRegenerationPacket *pstACRData = NULL ;
    uint8_t i ;

    pstHdmiRxStatus = BKNI_Malloc(sizeof(*pstHdmiRxStatus));
    if (pstHdmiRxStatus == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
        goto done ;
    }

    pstACRData = BKNI_Malloc(sizeof(*pstACRData));
    if (pstACRData == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
        goto done ;
    }

    for (i = 0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        hdmiInput = g_NEXUS_hdmiInput.handle[i] ;
        if (!hdmiInput) continue;

        errCode = BHDR_GetHdmiRxStatus(hdmiInput->hdr, pstHdmiRxStatus) ;
        if (errCode) {BERR_TRACE(errCode) ; goto done ; }

        if (pstHdmiRxStatus->DeviceAttached && pstHdmiRxStatus->bValidStatus)
        {
            errCode = BHDR_GetAudioClockRegenerationData(hdmiInput->hdr, pstACRData) ;
            if (errCode) {BERR_TRACE(errCode) ; goto done ;}

            BAVC_HDMI_DisplayACRData(&pstHdmiRxStatus->stPort, pstACRData) ;
        }
     }
done:
    if (pstHdmiRxStatus)
        BKNI_Free(pstHdmiRxStatus) ;

    if (pstACRData)
        BKNI_Free(pstACRData) ;


#endif
}

void NEXUS_HdmiInputModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_HdmiInputStatus *hdmiInputStatus;

    BDBG_LOG(("HdmiInputModule:"));

    hdmiInputStatus = BKNI_Malloc(sizeof(*hdmiInputStatus));
    if (hdmiInputStatus == NULL)
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
        goto done ;
    }


    for (i=0 ; i < NEXUS_NUM_HDMI_INPUTS; i++)
    {
        NEXUS_HdmiInputHandle hdmiInput = g_NEXUS_hdmiInput.handle[i];
        if (!hdmiInput)
            continue;

#if NEXUS_HAS_AUDIO
        BDBG_LOG(("   hdmi_input %d: handle=%p, videoInput=%p, audioInput=%p",
            i, (void *)hdmiInput, (void *)&hdmiInput->videoInput, (void *)&hdmiInput->audioInput));
#endif

        BDBG_LOG(("   videoConnected=%d audioConnected=%d",
            hdmiInput->videoConnected, hdmiInput->audioConnected));

        errCode = NEXUS_HdmiInput_GetStatus(hdmiInput, hdmiInputStatus) ;
        BSTD_UNUSED(errCode);

        BDBG_LOG(("   Rx Phy Status:")) ;
        if (!hdmiInputStatus->deviceAttached)
        {
            BDBG_LOG(("      No Device connected to HDMI Input")) ;
            goto done ;
        }

        if (!hdmiInputStatus->validHdmiStatus)
        {
            BDBG_LOG(("      Invalid or no HDMI Input Status")) ;
            goto done ;
        }

        BDBG_LOG(("      PLL Lock: %s; Packet Errors %s; Symbol Loss: %s",
            hdmiInputStatus->pllLocked ? "Yes" : "No",
            hdmiInputStatus->packetErrors ? "Yes" : "No",
            hdmiInputStatus->symbolLoss ? "Yes" : "No")) ;
        BDBG_LOG(("      Rx Clock: %d", hdmiInputStatus->lineClock)) ;

        BDBG_LOG(("   Video Format:")) ;
        BDBG_LOG(("      %d x %d%c (%d bpp) ColorSpace: %d ; Aspect Ratio: %d",
            hdmiInputStatus->width, hdmiInputStatus->height,
            hdmiInput->vdcStatus.interlaced ? 'i' : 'p',
            hdmiInputStatus->colorDepth, hdmiInputStatus->colorSpace,
            hdmiInputStatus->aspectRatio)) ;

        BDBG_LOG(("      Mode: %s", hdmiInputStatus->hdmiMode ? "HDMI" : "DVI")) ;
        BDBG_LOG(("      AvMute (HDMI only): %s", hdmiInputStatus->avMute ? "Yes" : "No")) ;


        BDBG_LOG(("   HDCP Encryption: %s",
            hdmiInputStatus->hdcpRiUpdating ? "Yes" : "No")) ;

    }

done:
    if (hdmiInputStatus)
        BKNI_Free(hdmiInputStatus) ;
#endif
}
