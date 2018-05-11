/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "nexus_hdmi_input_module.h"
#include "priv/nexus_hdmi_input_priv.h"
#include "priv/nexus_hdmi_input_standby_priv.h"
#include "nexus_hdmi_input_hdcp_priv.h"

#include "nexus_power_management.h"

BDBG_MODULE(nexus_hdmi_input);


static void NEXUS_HdmiInput_P_SetPower( NEXUS_HdmiInputHandle hdmiInput, bool callingHdr );
static void NEXUS_HdmiInput_P_ReleaseHotPlug(void *context) ;

/****************************************
* Module functions
***************/



void NEXUS_HdmiInput_ToggleHotPlug(NEXUS_HdmiInputHandle hdmiInput)
{
    NEXUS_HdmiInput_SetHotPlug(hdmiInput, true) ;
    if (!hdmiInput->releaseHotPlugTimer)
    {
        hdmiInput->releaseHotPlugTimer =
            NEXUS_ScheduleTimer(150, NEXUS_HdmiInput_P_ReleaseHotPlug, hdmiInput) ;
    }
}


void NEXUS_HdmiInput_GetSettings(NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiInputSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    *pSettings = hdmiInput->settings;
}

NEXUS_Error NEXUS_HdmiInput_SetSettings(NEXUS_HdmiInputHandle hdmiInput, const NEXUS_HdmiInputSettings *pSettings)
{
    BAVC_Colorspace manualColorSpace;
    BHDR_Settings hdrSettings ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    NEXUS_IsrCallback_Set(hdmiInput->avMuteCallback, &pSettings->avMuteChanged);
    NEXUS_TaskCallback_Set(hdmiInput->sourceChangedCallback, &pSettings->sourceChanged);
    NEXUS_IsrCallback_Set(hdmiInput->aviInfoFrameChanged, &pSettings->aviInfoFrameChanged);
    NEXUS_IsrCallback_Set(hdmiInput->audioInfoFrameChanged, &pSettings->audioInfoFrameChanged);
    NEXUS_IsrCallback_Set(hdmiInput->spdInfoFrameChanged, &pSettings->spdInfoFrameChanged);
    NEXUS_IsrCallback_Set(hdmiInput->vendorSpecificInfoFrameChanged, &pSettings->vendorSpecificInfoFrameChanged);
    NEXUS_IsrCallback_Set(hdmiInput->audioContentProtectionChanged, &pSettings->audioContentProtectionChanged);
    NEXUS_IsrCallback_Set(hdmiInput->hotPlugCallback, &pSettings->frontend.hotPlugCallback) ;

    manualColorSpace = NEXUS_P_ColorSpace_ToMagnum_isrsafe(pSettings->colorSpace);

    BKNI_EnterCriticalSection();
        if ( hdmiInput->settings.forcePcFormat != pSettings->forcePcFormat )
        {
            if ( hdmiInput->pPcFormatCallback_isr )
            {
                BDBG_MSG(("Force PC format option has changed.  Notifying display."));
                hdmiInput->pPcFormatCallback_isr(hdmiInput->pPcFormatCallbackParam);
            }
        }
        hdmiInput->settings = *pSettings;
        hdmiInput->manualColorSpace = manualColorSpace;

    BKNI_LeaveCriticalSection();

    BHDR_GetSettings(hdmiInput->hdr, &hdrSettings) ;

        hdrSettings.bParseAVI = pSettings->hdr.parseAviInfoframe  ;
        hdrSettings.bDisableI2cPadSclPullup = pSettings->hdr.disableI2cSclPullUp ;
        hdrSettings.bDisableI2cPadSdaPullup = pSettings->hdr.disableI2cSdaPullUp  ;
        hdrSettings.bHdmiHardwarePassthrough = pSettings->hdr.enableHdmiHardwarePassthrough ;

    BHDR_SetSettings(hdmiInput->hdr, &hdrSettings ) ;

    return NEXUS_SUCCESS;
}

NEXUS_VideoInput NEXUS_HdmiInput_GetVideoConnector(NEXUS_HdmiInputHandle hdmiInput)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    return &hdmiInput->videoInput;
}

NEXUS_AudioInputHandle NEXUS_HdmiInput_GetAudioConnector(NEXUS_HdmiInputHandle hdmiInput)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
#if NEXUS_HAS_AUDIO
    return &hdmiInput->audioInput;
#else
    return NULL;
#endif
}

/**
Apply master's stFieldData to its slave.
This function is called by both masters, slaves, and non-masters.
So we must first determine if this is a master. Only if we are, then set the slave.
**/
static void NEXUS_HdmiInput_P_SetSlaveFieldData_isr(NEXUS_HdmiInputHandle hdmiInput)
{
#if NEXUS_NUM_HDMI_INPUTS > 1
    if (hdmiInput->masterHdmiInput == NULL) {
        /* we might be a master */
        NEXUS_HdmiInputHandle slaveHdmiInput = g_NEXUS_hdmiInput.handle[1 - hdmiInput->index];
        if (slaveHdmiInput && slaveHdmiInput->masterHdmiInput == hdmiInput) {
            /* is a master, so memcpy */
            BKNI_Memcpy(&slaveHdmiInput->stFieldData, &hdmiInput->stFieldData, sizeof(BAVC_VDC_HdDvi_Picture)) ;
        }
    }
#else
    BSTD_UNUSED(hdmiInput) ;
#endif
}

void NEXUS_HdmiInput_PictureCallback_isr(NEXUS_HdmiInputHandle hdmiInput, BAVC_VDC_HdDvi_Picture **ppPicture)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    if (hdmiInput->uiFormatChangeMuteCount)
    {

        /* only report messages if AvMute is still set */
        if (hdmiInput->avMute)
        {
            BDBG_WRN(("Mute video for %d more frames after format change; AvMute: %d",
                hdmiInput->uiFormatChangeMuteCount, hdmiInput->avMute)) ;

            if (hdmiInput->uiFormatChangeMuteCount == 1)
            {
                BDBG_WRN(("Ready to un-mute video after format change")) ;
            }
        }
        hdmiInput->uiFormatChangeMuteCount-- ;
    }


    hdmiInput->stFieldData.bMute =
        hdmiInput->avMute || (hdmiInput->uiFormatChangeMuteCount);


    if (hdmiInput->bSentResetHdDviBegin)
    {
        hdmiInput->stFieldData.bMute            =  hdmiInput->uiFormatChangeMuteCount ;
        hdmiInput->stFieldData.bResetHdDviBegin = false;
        hdmiInput->bSentResetHdDviBegin         = false;
    }

    /* bResetHdDviBegin requested */
    if (hdmiInput->stFieldData.bResetHdDviBegin)
    {
        hdmiInput->bSentResetHdDviBegin = true;
    }

    NEXUS_HdmiInput_P_SetSlaveFieldData_isr(hdmiInput);

    /* Return the current state we have. */
    *ppPicture = &hdmiInput->stFieldData;
}


void NEXUS_HdmiInput_P_AvMuteNotify_isr(void *context, int param2, void *data)
{
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;
    BSTD_UNUSED(param2);

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    hdmiInput->avMute = *((bool *) data) ? true : false;
    BDBG_MSG(("hdmiInput%d AvMuteNotify_isr %d",
        hdmiInput->index, hdmiInput->avMute)) ;

    if (hdmiInput->avMute)
        hdmiInput->stFieldData.bMute = true;
    else
        hdmiInput->stFieldData.bMute = hdmiInput->uiFormatChangeMuteCount ;

    hdmiInput->stFieldData.bResetHdDviBegin = true;

    NEXUS_HdmiInput_P_SetSlaveFieldData_isr(hdmiInput);

    NEXUS_IsrCallback_Fire_isr(hdmiInput->avMuteCallback);
}

void NEXUS_HdmiInput_P_VideoFormatChange_isr(void *context, int param2, void *data)
{
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;
    BAVC_HDMI_AviInfoFrame *pAviInfoFrame = (BAVC_HDMI_AviInfoFrame*)data;
    BAVC_Colorspace colorSpace;
    BAVC_CscMode     cscMode ;

    BAVC_HDMI_AviInfoFrame_VideoFormat eAviInfoFrameFormat ;
    BFMT_AspectRatio aspectRatio;
    bool limitedRange ;

    BSTD_UNUSED(param2);
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(BAVC_Colorspace_eYCbCr444 == (BAVC_Colorspace)BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444);

    BDBG_MSG(("hdmiInput%d VideoFormatChange_isr", hdmiInput->index));

    hdmiInput->stFieldData.bHdmiMode = true ;

     /* set the default Colorspace Converter   */
    cscMode = BAVC_CscMode_e709RgbFullRange ;


    if (pAviInfoFrame->ePacketStatus == BAVC_HDMI_PacketStatus_eStopped)
    {
        /* override to DVI RGB mode */
        hdmiInput->stFieldData.bHdmiMode = false ;
        colorSpace = BAVC_Colorspace_eRGB ;

        /* assume aspect ratio 16:9 */
        aspectRatio = BFMT_AspectRatio_e16_9;

        goto NEXUS_HdmiInput_P_VideoFormatChange_isr_SETUP ;
    }


    hdmiInput->reportedColorSpace = pAviInfoFrame->ePixelEncoding;

    switch (pAviInfoFrame->ePictureAspectRatio) {
    case BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData:
        aspectRatio = BFMT_AspectRatio_eUnknown;
        break;
    case BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3:
        aspectRatio = BFMT_AspectRatio_e4_3;
        break;
    default:
    case BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9:
        aspectRatio = BFMT_AspectRatio_e16_9;
        break;
    }

    /* set the pixel repitition value */
    hdmiInput->stFieldData.ulPixelRepitionFactor = pAviInfoFrame->PixelRepeat ;

    if (!hdmiInput->settings.autoColorSpace)
    {
        colorSpace = hdmiInput->manualColorSpace; /* overwrite color space */
     goto NEXUS_HdmiInput_P_VideoFormatChange_isr_SETUP ;
    }

    /* use the reported Color Space specified in the Avi Infoframe */
    colorSpace = hdmiInput->reportedColorSpace;

    /* set the color matrix */
    if ((pAviInfoFrame->VideoIdCode == 0 )   /* Unknown, usually VESA format */
    || (pAviInfoFrame->VideoIdCode == 1 )) /* VGA */
        /* IT FORMAT */
        eAviInfoFrameFormat = BAVC_HDMI_AviInfoFrame_VideoFormat_eIT ;

    else if (((pAviInfoFrame->VideoIdCode >=  2) && (pAviInfoFrame->VideoIdCode <=  3))
    || ((pAviInfoFrame->VideoIdCode >=  6) && (pAviInfoFrame->VideoIdCode <= 15))
    || ((pAviInfoFrame->VideoIdCode >= 17) && (pAviInfoFrame->VideoIdCode <= 18))
    || ((pAviInfoFrame->VideoIdCode >= 21) && (pAviInfoFrame->VideoIdCode <= 30))
    || ((pAviInfoFrame->VideoIdCode >= 35) && (pAviInfoFrame->VideoIdCode <= 38))
    || ((pAviInfoFrame->VideoIdCode >= 42) && (pAviInfoFrame->VideoIdCode <= 45))
    || ((pAviInfoFrame->VideoIdCode >= 48) && (pAviInfoFrame->VideoIdCode <= 50)))
            /* SD FORMAT */
            eAviInfoFrameFormat = BAVC_HDMI_AviInfoFrame_VideoFormat_eSD ;

    else if (((pAviInfoFrame->VideoIdCode >=  4) && (pAviInfoFrame->VideoIdCode <=  5))
    || ((pAviInfoFrame->VideoIdCode ==  16))
    || ((pAviInfoFrame->VideoIdCode >=  19) && (pAviInfoFrame->VideoIdCode <= 20))
    || ((pAviInfoFrame->VideoIdCode >=  31) && (pAviInfoFrame->VideoIdCode <= 34))
    || ((pAviInfoFrame->VideoIdCode >=  39) && (pAviInfoFrame->VideoIdCode <= 41))
    || ((pAviInfoFrame->VideoIdCode >=  46) && (pAviInfoFrame->VideoIdCode <= 47))
    || ((pAviInfoFrame->VideoIdCode >=  63) && (pAviInfoFrame->VideoIdCode <= 64))
    || ((pAviInfoFrame->VideoIdCode >=  93) && (pAviInfoFrame->VideoIdCode <= 97)))
            /* HD FORMAT */
            eAviInfoFrameFormat = BAVC_HDMI_AviInfoFrame_VideoFormat_eHD ;

    else
    {
        /* New/Unknown Format */
        BDBG_WRN(("Unknown/Unsupported VIDEO ID Code %d; Assuming HD format",
            pAviInfoFrame->VideoIdCode)) ;
            eAviInfoFrameFormat = BAVC_HDMI_AviInfoFrame_VideoFormat_eHD ;
    }


    /* YCbCr */
    if ((pAviInfoFrame->ePixelEncoding == BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr420)
    || (pAviInfoFrame->ePixelEncoding == BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr422)
    || (pAviInfoFrame->ePixelEncoding == BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444))
    {
        /* Unspecified colorimetry, but SD Format */
        if (((pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData)
        && (eAviInfoFrameFormat == BAVC_HDMI_AviInfoFrame_VideoFormat_eSD))

        /* OR Colorimetry is specified as BT.601 */
        || (pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170)

        /* OR Colorimetry is specified as  xvYCC601
             BT2020 YCbCr = non constant luminance
             BT2020 YcCBcCRc constant luminance not supported
        */
        || ((pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
        && ((pAviInfoFrame->eExtendedColorimetry == BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601)
        || (pAviInfoFrame->eExtendedColorimetry == BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YCbCr))))
        {
            /* Set the BT.601 YCbCr flag */
         cscMode = BAVC_CscMode_e601YCbCr ;
        }
        else
        {
            /* Set the BT.709 YCbCr flag */
            cscMode = BAVC_CscMode_e709YCbCr ;
          }
    }
    /****** RGB ******/
    else if (pAviInfoFrame->ePixelEncoding == BAVC_HDMI_AviInfoFrame_Colorspace_eRGB)
    {
        /* determine whether color range limited vs full */

        /* if xv.Color (always scaled as limited range) */
        /* if ((C=3) Or (Q=1) Or  ((Q=0) And (FormatIsSD Or FormatIsHD)))        */

        if ((pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)

        /* OR limited range is expressly specified */
        ||  (pAviInfoFrame->eRGBQuantizationRange == BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eLimitedRange)

        /* OR video format is used to select */
        ||  ((pAviInfoFrame->eRGBQuantizationRange == BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault)
        &&    ((eAviInfoFrameFormat == BAVC_HDMI_AviInfoFrame_VideoFormat_eSD)
        || (eAviInfoFrameFormat == BAVC_HDMI_AviInfoFrame_VideoFormat_eHD))))
        {
            limitedRange = true ;
         }
        else
        {
            limitedRange = false ;
        }


        /* NOW determine which colorimetry */

        /* Unspecified Colorimetry, but SD Format */
        if (((pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData)
        && (eAviInfoFrameFormat == BAVC_HDMI_AviInfoFrame_VideoFormat_eSD))

        /* OR Colorimetry is specified as BT.601 */
        || (pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170)

        /* OR Colorimetry is specified as  xvYCC601 */
        || ((pAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
        && (pAviInfoFrame->eExtendedColorimetry == BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601)))
        {
            if (limitedRange)
                /****** BT.601 LIMITED ******/
                cscMode = BAVC_CscMode_e601RgbLimitedRange ;
            else

                /****** BT.601 FULL  ******/
                cscMode = BAVC_CscMode_e601RgbFullRange ;
        }
        else
        {
            if (limitedRange)
                /****** BT.709 LIMITED ******/
                cscMode = BAVC_CscMode_e709RgbLimitedRange ;
            else
                /****** BT.709 FULL  ******/
                cscMode = BAVC_CscMode_e709RgbFullRange ;
        }
    }

NEXUS_HdmiInput_P_VideoFormatChange_isr_SETUP:

   /* set the Matrix based on the CSC */

    switch(cscMode)
    {
    case BAVC_CscMode_e709RgbFullRange :
    case BAVC_CscMode_e709RgbLimitedRange :
    case BAVC_CscMode_e709YCbCr :
        hdmiInput->stFieldData.eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
        hdmiInput->stFieldData.eTransferCharacteristics = BAVC_TransferCharacteristics_eItu_R_BT_709 ;
        break ;

    case BAVC_CscMode_e601RgbFullRange :
    case BAVC_CscMode_e601RgbLimitedRange :
    case BAVC_CscMode_e601YCbCr :
        hdmiInput->stFieldData.eMatrixCoefficients = BAVC_MatrixCoefficients_eSmpte_170M ;
        hdmiInput->stFieldData.eTransferCharacteristics = BAVC_TransferCharacteristics_eSmpte_170M ;
        break ;

    /* coverity[dead_error_begin: FALSE] */
    default :
        BDBG_ERR(("Unknown Csc Mode: %d; default to 709", cscMode))  ;
        hdmiInput->stFieldData.eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
        hdmiInput->stFieldData.eTransferCharacteristics = BAVC_TransferCharacteristics_eItu_R_BT_709 ;
        break ;
    }

     /* report all changes */
    if (hdmiInput->stFieldData.bHdmiMode)
    {
         BDBG_MSG(("hdmiInput%d Receiving HDMI format %s (VIC: %d) %s; Current ColorSpace: %s",
            hdmiInput->index,
            BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr_isrsafe(pAviInfoFrame->VideoIdCode),
            pAviInfoFrame->VideoIdCode,
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(colorSpace),
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(hdmiInput->stFieldData.eColorSpace))) ;
     }
    else /* DVI MODE if (hdmiInput->vdcStatus.hdmiMode) */
    {
         BDBG_MSG(("hdmiInput%d Receiving DVI  format %s",
            hdmiInput->index,
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(colorSpace))) ;
     }

    /* Set bMute for significant changes */
    if  (colorSpace != hdmiInput->stFieldData.eColorSpace
    ||  aspectRatio != hdmiInput->stFieldData.eAspectRatio
    ||  cscMode != hdmiInput->stFieldData.eCscMode)
    {
        BDBG_MSG(("hdmiInput%d Color Space change from '%s' to '%s'",
            hdmiInput->index,
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(
            hdmiInput->stFieldData.eColorSpace),
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(colorSpace))) ;
        hdmiInput->stFieldData.eAspectRatio = aspectRatio;
        hdmiInput->stFieldData.eColorSpace = colorSpace;
        hdmiInput->stFieldData.eCscMode = cscMode ;
        hdmiInput->stFieldData.bResetHdDviBegin = true;

        hdmiInput->stFieldData.bMute = true;
        hdmiInput->uiFormatChangeMuteCount = 10 ;
    }
    else
    {
        BDBG_MSG(("No Changes detected %s",
            BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(hdmiInput->stFieldData.eColorSpace))) ;
    }
}


static void NEXUS_HdmiInput_P_SetHdmiFormat_isr(NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiVendorSpecificInfoFrame *pVSInfoFrame)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

     /* override Video Format to Normal Mode */
    if (pVSInfoFrame->packetStatus == (NEXUS_HdmiPacketStatus)BAVC_HDMI_PacketStatus_eStopped)
    {

        BDBG_WRN(("hdmiInput%d VSI packets stopped; Overriding HDMI Video Format from <%s> to <%s>",
            hdmiInput->index,
            BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(pVSInfoFrame->hdmiVideoFormat),
            BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone))) ;

        pVSInfoFrame->hdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
    }
    else
    {
        BDBG_MSG(("hdmiInput%d Receiving HDMI Video Format <%s>",
            hdmiInput->index,
            BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(pVSInfoFrame->hdmiVideoFormat))) ;
    }

    switch (pVSInfoFrame->hdmiVideoFormat)
    {
    case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
        /* app handles configuring windows for 3D mode */
        pVSInfoFrame->hdmiVideoFormat =
            NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat ;

        break ;

    case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution :
        pVSInfoFrame->hdmiVideoFormat =
            NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eExtendedResolution ;
        break ;

    default :
    case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :

        /* set to NORMAL mode */
        pVSInfoFrame->hdmiVideoFormat =
            NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone ;

        break ;
    }

    /* save current HDMI Video Format */
    Nexus_HdmiInput_P_SetHdmiVideoFormat_isr(pVSInfoFrame->hdmiVideoFormat) ;
}


static void NEXUS_HdmiInput_P_SetDrmFieldData_isr(NEXUS_HdmiInputHandle hdmiInput, const BAVC_HDMI_DRMInfoFrame * pDrmIf)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    switch (pDrmIf->ePacketStatus)
    {
    case BAVC_HDMI_PacketStatus_eUpdated:
        hdmiInput->stFieldData.eEotf = pDrmIf->eEOTF;
        hdmiInput->stFieldData.stHdrMetadata.stStatic = pDrmIf->stType1;
        break;
    case BAVC_HDMI_PacketStatus_eStopped:
    default:
        hdmiInput->stFieldData.eEotf = BAVC_HDMI_DRM_EOTF_eMax; /* indicates un-set */
        BAVC_GetDefaultStaticHdrMetadata_isrsafe(&hdmiInput->stFieldData.stHdrMetadata.stStatic);
        break;
    }
}

void NEXUS_HdmiInput_P_PacketChange_isr(void *context, int param2, void *data)
{
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)context;

    BSTD_UNUSED(data);

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_MSG(("hdmiInput%d PacketChange_isr %s (%#x)",
        hdmiInput->index, BAVC_HDMI_PacketTypeToStr_isrsafe(param2), param2)) ;

    switch ((BAVC_HDMI_PacketType)param2)
    {
    case BAVC_HDMI_PacketType_eAviInfoFrame:
        NEXUS_IsrCallback_Fire_isr(hdmiInput->aviInfoFrameChanged);
        break;
    case BAVC_HDMI_PacketType_eAudioInfoFrame:
        NEXUS_IsrCallback_Fire_isr(hdmiInput->audioInfoFrameChanged);
        break;
    case BAVC_HDMI_PacketType_eSpdInfoFrame:
        NEXUS_IsrCallback_Fire_isr(hdmiInput->spdInfoFrameChanged);
        break;
    case BAVC_HDMI_PacketType_eVendorSpecificInfoframe:
        NEXUS_HdmiInput_P_SetHdmiFormat_isr(hdmiInput, data) ;
        NEXUS_IsrCallback_Fire_isr(hdmiInput->vendorSpecificInfoFrameChanged);
        break;
    case BAVC_HDMI_PacketType_eDrmInfoFrame:
        NEXUS_HdmiInput_P_SetDrmFieldData_isr(hdmiInput, data);
        break;
    case BAVC_HDMI_PacketType_eAudioContentProtection:
        NEXUS_IsrCallback_Fire_isr(hdmiInput->audioContentProtectionChanged);
        break;
    default:
        BDBG_WRN(("HDMI Packet Type %x Change not supported", param2));
        break;
    }
}

void NEXUS_HdmiInput_GetSourceId_priv(NEXUS_HdmiInputHandle hdmiInput, BAVC_SourceId *id)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    NEXUS_ASSERT_MODULE();
    *id = BAVC_SourceId_eHdDvi0 + hdmiInput->index;
}

void NEXUS_HdmiInput_P_SetFrameRate(void *data)
{
    NEXUS_HdmiInputHandle hdmiInput = (NEXUS_HdmiInputHandle)data;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    /* I'm typecasting from Magnum to Nexus. The assert will check if this assumption fails in the future. */
    (void)NEXUS_Timebase_SetHdDviFrameRate(hdmiInput->settings.timebase, hdmiInput->frameRate);
}

void NEXUS_HdmiInput_SetFrameRate_isr(NEXUS_HdmiInputHandle hdmiInput, BAVC_FrameRateCode frameRate)
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

#if 0
    BDBG_MSG(("frameRate %d", frameRate));
#endif
    hdmiInput->stFieldData.eFrameRateCode = frameRate;
    BDBG_CASSERT((NEXUS_VideoFrameRate)BAVC_FrameRateCode_e60 == NEXUS_VideoFrameRate_e60);
    hdmiInput->frameRate = (NEXUS_VideoFrameRate)frameRate;

    NEXUS_HdmiInput_P_SetSlaveFieldData_isr(hdmiInput);

    /* There is no _isr interface for programming PCR in XPT PI, therefore we must convert to task. */
    BKNI_SetEvent(hdmiInput->frameRateEvent);
}

#define CONNECTED(hdmiInput) ((hdmiInput)->videoConnected || (hdmiInput)->audioConnected)

NEXUS_Error NEXUS_HdmiInput_GetStatus(NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiInputStatus *pStatus)
{
    NEXUS_HdmiGeneralControlPacket generalControlPacket ;
    BHDR_Status rxstatus;
    BAVC_HDMI_VideoFormat hdrFmtStatus;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    BKNI_Memset(&rxstatus, 0, sizeof(rxstatus)) ;

    /* set default for noSignal and symbolLoss */
    pStatus->symbolLoss = 1 ;
    pStatus->noSignal = 1 ;

    /*
    -- although the hdmiInput may not be selected,
    -- update its status to indicate if device is actually connected
    -- NOTE: in the case of a switch, HPD may always be connected
    --
    -- GetHdmiRxStatus will always correctly return DeviceAttached status
    -- regardless of the function return value
    */
    rc = BHDR_GetHdmiRxStatus(hdmiInput->hdr, &rxstatus);
    if (rc) {BERR_TRACE(rc);}

    pStatus->deviceAttached = rxstatus.DeviceAttached ;
    BDBG_MSG(("Source connected to hdmiInput%d: %s",
        hdmiInput->index,  pStatus->deviceAttached ? "Yes" : "No")) ;


    if (!CONNECTED(hdmiInput))
    {
        /* report status message once */
        if (hdmiInput->vdcStatus.validHdmiStatus)
        {
            BDBG_WRN(("Source connected to hdmiInput%d: %s",
                hdmiInput->index,  pStatus->deviceAttached ? "Yes" : "No")) ;
            BDBG_WRN(("Unable to get HDMI Rx status; hdmiInput%d is not selected/active",
                hdmiInput->index)) ;
            hdmiInput->vdcStatus.validHdmiStatus = false ;
        }
        goto done  ;
    }

    NEXUS_HdmiInput_GetGeneralControlPacket(hdmiInput, &generalControlPacket) ;
    pStatus->colorDepth = generalControlPacket.colorDepth ;

    rc = BHDR_GetHdmiRxDetectedVideoFormat(hdmiInput->hdr, &hdrFmtStatus);
    if (rc) {BERR_TRACE(rc); return rc;}

    pStatus->hdmiMode = rxstatus.HdmiMode;
    pStatus->deviceAttached = rxstatus.DeviceAttached;
    pStatus->pllLocked = rxstatus.PllLocked;
    pStatus->packetErrors = rxstatus.bPacketErrors;
    pStatus->avMute = rxstatus.bAvmute;
    pStatus->hdcpRiUpdating = rxstatus.bHdcpRiUpdating;
    pStatus->symbolLoss = rxstatus.bSymbolLoss;

    pStatus->colorSpace = NEXUS_P_ColorSpace_FromMagnum_isrsafe(hdmiInput->reportedColorSpace);
    pStatus->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(hdmiInput->stFieldData.eAspectRatio);
    pStatus->pixelRepeat = rxstatus.PixelRepeat;

    /* audio related information */
    pStatus->audio.packets = rxstatus.uiAudioPackets;
    pStatus->audio.validSpdifInfo = rxstatus.bValidChannelStatus ;
    pStatus->audio.streamType = rxstatus.stChannelStatus.eStreamType;
    pStatus->audio.wordLength = rxstatus.stChannelStatus.eWordLength;
    pStatus->audio.sampleFreq = rxstatus.stChannelStatus.eSamplingFrequency;


#if 0
    pStatus->audio.sampleRateHz = rxstatus.uiAudioSampleRateHz ;
#endif

    /* copy these from vdcStatus. make sure the code in NEXUS_VideoInput_P_SetHdmiInputStatus is setting these fields. */
    pStatus->interlaced = hdmiInput->vdcStatus.interlaced;
    pStatus->noSignal = hdmiInput->vdcStatus.noSignal;
    pStatus->originalFormat = hdmiInput->vdcStatus.originalFormat;
    pStatus->avHeight = hdmiInput->vdcStatus.avHeight;
    pStatus->avWidth = hdmiInput->vdcStatus.avWidth;
    pStatus->height = hdrFmtStatus.uActivelinesField1;
    pStatus->width = hdrFmtStatus.uHorizontalActivePixels;
    pStatus->vertFreq = hdmiInput->vdcStatus.vertFreq;
    pStatus->vBlank = hdmiInput->vdcStatus.vBlank;
    pStatus->hBlank = hdmiInput->vdcStatus.hBlank;
    pStatus->vPolarity = hdmiInput->vdcStatus.vPolarity;
    pStatus->hPolarity = hdmiInput->vdcStatus.hPolarity;

    rc = BHDR_GetPixelClockEstimate(hdmiInput->hdr, &pStatus->lineClock);

    pStatus->validHdmiStatus = true ;

done:
#if BDBG_DEBUG_BUILD
    /* for debugging, report changes in status */
    if ((pStatus->deviceAttached != hdmiInput->previousVdcStatus.deviceAttached)
    || (pStatus->validHdmiStatus != hdmiInput->previousVdcStatus.validHdmiStatus)
    || (pStatus->noSignal != hdmiInput->previousVdcStatus.noSignal)
    || (pStatus->pllLocked != hdmiInput->previousVdcStatus.pllLocked)
    || (pStatus->symbolLoss != hdmiInput->previousVdcStatus.symbolLoss)
    || (pStatus->hdmiMode != hdmiInput->previousVdcStatus.hdmiMode))
    {
        BDBG_MSG(("hdmiInput%d devAttached(%d) validStatus(%d) noSignal(%d) pllLock(%d) symbolLoss(%d) %s Mode",
            hdmiInput->index,
            pStatus->deviceAttached, pStatus->validHdmiStatus,
            pStatus->noSignal,  pStatus->pllLocked, pStatus->symbolLoss,
            pStatus->hdmiMode ? "HDMI" : "DVI")) ;
    }
    hdmiInput->previousVdcStatus =
#endif
    hdmiInput->vdcStatus = *pStatus ;

    return NEXUS_SUCCESS ;
}

static void NEXUS_HdmiInput_P_SetPower( NEXUS_HdmiInputHandle hdmiInput, bool callingHdr )
{
    BERR_Code rc ;
    BHDR_FE_ChannelPowerSettings powerSettings;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    BHDR_FE_GetPowerState(hdmiInput->frontend, &powerSettings);
    powerSettings.bHdmiRxPowered  =  callingHdr || CONNECTED(hdmiInput) ;

    BDBG_MSG(("Configure HDMI Rx Port%d : %s",
          hdmiInput->index, powerSettings.bHdmiRxPowered ? "ON" : "OFF")) ;

    rc = BHDR_FE_SetPowerState(hdmiInput->frontend, &powerSettings);
    if (rc) return ;
}


NEXUS_Error NEXUS_HdmiInput_SetMaster(
    NEXUS_HdmiInputHandle slaveHdmiInput, NEXUS_HdmiInputHandle masterHdmiInput)
{
    BERR_Code rc = NEXUS_SUCCESS ;

#if BHDR_CONFIG_MASTER_SLAVE_3D_SUPPORT
    BDBG_OBJECT_ASSERT(masterHdmiInput, NEXUS_HdmiInput);
    BDBG_OBJECT_ASSERT(slaveHdmiInput, NEXUS_HdmiInput);

    BDBG_WRN(("NEXUS_HdmiInput_SetMaster slave=%d(%p), master=%d(%p)",
        slaveHdmiInput->index, slaveHdmiInput,
        masterHdmiInput?masterHdmiInput->index:0, masterHdmiInput));

    if (slaveHdmiInput->masterHdmiInput != masterHdmiInput) {
        if (!masterHdmiInput && slaveHdmiInput->masterHdmiInput) {
            /* Clear Master/Slave configuration of HDR cores */
            rc = BHDR_UsageMasterSlaveClear(slaveHdmiInput->masterHdmiInput->hdr, slaveHdmiInput->hdr) ;
            if (rc) return BERR_TRACE(rc);

            /* default slaved core back to original config */
            rc = BHDR_FE_AttachHdmiRxCore(
                    slaveHdmiInput->frontend, slaveHdmiInput->hdr) ;
            if (rc) return BERR_TRACE(rc);
        }
        else if (masterHdmiInput) {
            /* must configure slave */
            rc = BHDR_UsageMasterSlaveSet(masterHdmiInput->hdr, slaveHdmiInput->hdr) ;
            if (rc) return BERR_TRACE(rc);

            rc = BHDR_FE_AttachHdmiRxCore(
                masterHdmiInput->frontend, slaveHdmiInput->hdr) ;
            if (rc) return BERR_TRACE(rc);

            BKNI_EnterCriticalSection();
            NEXUS_HdmiInput_P_SetSlaveFieldData_isr(masterHdmiInput);
            BKNI_LeaveCriticalSection();
        }
        slaveHdmiInput->masterHdmiInput = masterHdmiInput;
    }
#else
    BSTD_UNUSED(masterHdmiInput);
    BSTD_UNUSED(slaveHdmiInput);
#endif

    return rc;
}

static void NEXUS_HdmiInput_SetConnected_priv( NEXUS_HdmiInputHandle hdmiInput)
{
    BERR_Code rc ;
    if (!hdmiInput->frontend) {
        return;
    }

    if (hdmiInput->masterHdmiInput) {
        /* I am the slave. Ignore this connected call. No HDR setup required on slave path. Master will set things up. */
        BDBG_WRN(("hdmiInput%d is slaved. Ignore this connected call. No HDR setup required on slave path. Master will set things up.",
            hdmiInput->index)) ;
        return;
    }

    if (CONNECTED(hdmiInput))
    {
        BHDR_Status rxStatus ;

        /* power up the HDMI core  */
        NEXUS_HdmiInput_P_SetPower(hdmiInput, true);

        /* port is ENABLED;  enable interrupts  */
        BKNI_EnterCriticalSection() ;
        BHDR_ConfigureAfterHotPlug_isr(hdmiInput->hdr, true) ;
        BKNI_LeaveCriticalSection() ;

        rc = BHDR_GetHdmiRxStatus(hdmiInput->hdr, &rxStatus);
        if (rc) {rc = BERR_TRACE(rc); }

        if (rxStatus.DeviceAttached)
        {
            /* toggle the hot plug; so attached device can re-initialize */
            NEXUS_HdmiInput_SetHotPlug(hdmiInput, true) ;
            if (!hdmiInput->releaseHotPlugTimer)
            {
                hdmiInput->releaseHotPlugTimer =
                    NEXUS_ScheduleTimer(115, NEXUS_HdmiInput_P_ReleaseHotPlug, hdmiInput) ;
            }
        }
    }
    else
    {
        /* port is DISABLED;  disable interrupts  */
        BKNI_EnterCriticalSection() ;

            BHDR_ConfigureAfterHotPlug_isr(hdmiInput->hdr, false) ;

        BKNI_LeaveCriticalSection() ;

        /* power down the HDMI core  */
        NEXUS_HdmiInput_P_SetPower(hdmiInput, false);
    }
}

void NEXUS_HdmiInput_AudioConnected_priv( NEXUS_HdmiInputHandle hdmiInput, bool audioConnected )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    NEXUS_ASSERT_MODULE();
    hdmiInput->audioConnected = audioConnected;
    NEXUS_HdmiInput_SetConnected_priv(hdmiInput);
}

void NEXUS_HdmiInput_VideoConnected_priv( NEXUS_HdmiInputHandle hdmiInput, bool videoConnected )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    NEXUS_ASSERT_MODULE();
    hdmiInput->videoConnected = videoConnected;
    NEXUS_HdmiInput_SetConnected_priv(hdmiInput);
}

NEXUS_Error NEXUS_HdmiInput_SetHotPlug(NEXUS_HdmiInputHandle hdmiInput, bool status)
{
    BERR_Code rc = BERR_SUCCESS;
    BHDR_HotPlugSignal eHotPlugSignal ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    if (status) {
        eHotPlugSignal = BHDR_HotPlugSignal_eLow;
    }
    else {
        eHotPlugSignal = BHDR_HotPlugSignal_eHigh;
    }

    rc = BHDR_SetHotPlug(hdmiInput->hdr, eHotPlugSignal) ;

    return rc;
}

NEXUS_Error NEXUS_HdmiInput_ConfigureAfterHotPlug(NEXUS_HdmiInputHandle hdmiInput, bool status)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    BDBG_WRN(("CH%d Manual HdmiInput_ConfgureAfterHotPlug: %d, VideoConnected: %d, AudioConnected: %d",
        hdmiInput->index, status, hdmiInput->videoConnected, hdmiInput->audioConnected)) ;

#if 0
    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;
#endif

    rc = BHDR_ConfigureAfterHotPlug(hdmiInput->hdr, status) ;
    return rc;
}

NEXUS_Error NEXUS_HdmiInput_GetRawPacketData( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiInputPacketType packetType, NEXUS_HdmiPacket *pPacket )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(NEXUS_HdmiInputPacketType_eUnused == (NEXUS_HdmiInputPacketType)BAVC_HDMI_PacketType_eUnused);
    BDBG_CASSERT(sizeof(NEXUS_HdmiPacket) == sizeof(BAVC_HDMI_Packet));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    rc = BHDR_GetRawPacketData(hdmiInput->hdr, (BAVC_HDMI_PacketType)packetType, (BAVC_HDMI_Packet*)pPacket);

    return rc;
}

void NEXUS_HdmiInput_SetStatus_priv( NEXUS_HdmiInputHandle hdmiInput, const NEXUS_HdmiInputStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    NEXUS_ASSERT_MODULE();

    /* Status Update coming from HDDVI format detection block */
    /* Inform of changes from HDDVI block only  */

    if ((hdmiInput->vdcStatus.interlaced ==   pStatus->interlaced)
    && (hdmiInput->vdcStatus.originalFormat ==  pStatus->originalFormat)
    && (hdmiInput->vdcStatus.noSignal ==   pStatus->noSignal)
    && (hdmiInput->vdcStatus.avWidth ==   pStatus->avWidth)
    && (hdmiInput->vdcStatus.avHeight ==   pStatus->avHeight)
    && (hdmiInput->vdcStatus.vertFreq ==   pStatus->vertFreq)
    && (hdmiInput->vdcStatus.hPolarity ==   pStatus->hPolarity)
    && (hdmiInput->vdcStatus.vPolarity ==   pStatus->vPolarity)
    && (hdmiInput->vdcStatus.vBlank ==   pStatus->vBlank)
    && (hdmiInput->vdcStatus.hBlank ==   pStatus->hBlank)
    && (hdmiInput->vdcStatus.pixelRepeat ==   pStatus->pixelRepeat))
        return ;

    hdmiInput->vdcStatus = *pStatus;

    /* set initial HdmiStatus for reporting purposes */
    hdmiInput->vdcStatus.validHdmiStatus = true ;
    hdmiInput->vdcStatus.deviceAttached = true ;

    NEXUS_TaskCallback_Fire(hdmiInput->sourceChangedCallback);
}

void NEXUS_HdmiInput_P_HotPlug_isr(void *context, int param, void *data)
{
    NEXUS_HdmiInputHandle hdmiInput = context;
    bool deviceAttached =  * (bool *) data ;

    BSTD_UNUSED(param);
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    BHDR_ConfigureAfterHotPlug_isr(hdmiInput->hdr, deviceAttached) ;
    NEXUS_IsrCallback_Fire_isr(hdmiInput->hotPlugCallback) ;
}

NEXUS_Error NEXUS_HdmiInput_GetAviInfoFrameData( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiAviInfoFrame *pAviInfoFrame )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pAviInfoFrame) == sizeof(BAVC_HDMI_AviInfoFrame));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    return BHDR_GetAviInfoFrameData(hdmiInput->hdr, (BAVC_HDMI_AviInfoFrame*)pAviInfoFrame);
}

NEXUS_Error NEXUS_HdmiInput_GetAudioInfoFrameData( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiAudioInfoFrame *pAudioInfoFrame )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pAudioInfoFrame) == sizeof(BAVC_HDMI_AudioInfoFrame));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    return BHDR_GetAudioInfoFrameData(hdmiInput->hdr, (BAVC_HDMI_AudioInfoFrame*)pAudioInfoFrame);
}

NEXUS_Error NEXUS_HdmiInput_GetSpdInfoFrameData( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiSpdInfoFrame *pSpdInfoFrame )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pSpdInfoFrame) == sizeof(BAVC_HDMI_SPDInfoFrame));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    return BHDR_GetSPDInfoFrameData(hdmiInput->hdr, (BAVC_HDMI_SPDInfoFrame*)pSpdInfoFrame);
}

NEXUS_Error NEXUS_HdmiInput_GetVendorSpecificInfoFrameData( NEXUS_HdmiInputHandle
    hdmiInput, NEXUS_HdmiVendorSpecificInfoFrame *pVendorSpecificInfoFrame )
{
    NEXUS_Error rc ;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pVendorSpecificInfoFrame) == sizeof(BAVC_HDMI_VendorSpecificInfoFrame));

    rc = BHDR_GetVendorSpecificInfoFrameData(   hdmiInput->hdr, (BAVC_HDMI_VendorSpecificInfoFrame*)pVendorSpecificInfoFrame);
    if (rc) {rc = BERR_TRACE(rc); return rc;}

#if BDBG_DEBUG_BUILD
    BDBG_CASSERT(NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eMax == (NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat)BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eMax);
    BDBG_MSG(("hdmiInput%d HDMI Video Format: %s (%d)",
        hdmiInput->index,
        BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr_isrsafe(pVendorSpecificInfoFrame->hdmiVideoFormat),
        pVendorSpecificInfoFrame->hdmiVideoFormat)) ;

    switch (pVendorSpecificInfoFrame->hdmiVideoFormat)
    {
    case NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat :
        BDBG_MSG(("hdmiInput%d VSI HDMI 3D Structure: '%s' (%d)",
            hdmiInput->index,
            BAVC_HDMI_VsInfoFrame_3DStructureToStr(pVendorSpecificInfoFrame->hdmi3DStructure),
            pVendorSpecificInfoFrame->hdmi3DStructure)) ;
        break  ;

    case NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eExtendedResolution :
        BDBG_MSG(("hdmiInput%d VSI Extended Resolution Format : '%s' (%d)",
            hdmiInput->index,
            BAVC_HDMI_VsInfoFrame_HdmiVicToStr(pVendorSpecificInfoFrame->hdmiVIC),
            pVendorSpecificInfoFrame->hdmiVIC)) ;
        break ;

    case NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone :
        /* do nothing */
        break ;

    default :
        BDBG_MSG(("hdmiInput%d VSI HDMI Format '%d' not supported",
            hdmiInput->index, pVendorSpecificInfoFrame->hdmiVideoFormat));
    }
#endif

    return rc ;
}


NEXUS_Error NEXUS_HdmiInput_GetDrmInfoFrameData_priv(NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    BERR_Code rc = BERR_SUCCESS ;
    BAVC_HDMI_DRMInfoFrame drmInfoFrame ;
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);

    if (!CONNECTED(hdmiInput))
    {
        rc = BERR_TRACE(NEXUS_UNKNOWN) ;
        return rc ;
    }

    BKNI_Memset(pDrmInfoFrame, 0, sizeof(NEXUS_HdmiDynamicRangeMasteringInfoFrame)) ;
    rc = BHDR_GetDrmiInfoFrameData(hdmiInput->hdr, &drmInfoFrame);
    if (rc)
    {
        BDBG_WRN(("Unable to get DRM InfoFrame")) ;
        goto done ;
    }

    pDrmInfoFrame->eotf = NEXUS_P_VideoEotf_FromMagnum_isrsafe( drmInfoFrame.eEOTF ) ;
    if (drmInfoFrame.eDescriptorId == BAVC_HDMI_DRM_DescriptorId_eType1)
    {
        pDrmInfoFrame->metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1 ;
        NEXUS_P_MasteringDisplayColorVolume_FromMagnum_isrsafe(
            &pDrmInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume,
            &drmInfoFrame.stType1.stMasteringDisplayColorVolume);
        NEXUS_P_ContentLightLevel_FromMagnum_isrsafe(
            &pDrmInfoFrame->metadata.typeSettings.type1.contentLightLevel,
            &drmInfoFrame.stType1.stContentLightLevel);
    }
    else
    {
        BDBG_ERR(("Unknown Meta Data Type: %d", pDrmInfoFrame->metadata.type)) ;
        rc = BERR_TRACE(NEXUS_UNKNOWN) ;
        goto done ;
    }

done:
    return rc ;
}


NEXUS_Error NEXUS_HdmiInput_GetAudioContentProtection( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiAudioContentProtection *pAcp )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pAcp) == sizeof(BAVC_HDMI_ACP));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    BHDR_GetAudioContentProtection(hdmiInput->hdr, (BAVC_HDMI_ACP*)pAcp);
    return NEXUS_SUCCESS ;
}

NEXUS_Error NEXUS_HdmiInput_GetAudioClockRegeneration( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiAudioClockRegeneration *pAudioClockRegeneration )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pAudioClockRegeneration) == sizeof(BAVC_HDMI_AudioClockRegenerationPacket));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    return BHDR_GetAudioClockRegenerationData(hdmiInput->hdr, (BAVC_HDMI_AudioClockRegenerationPacket*)pAudioClockRegeneration);
}

NEXUS_Error NEXUS_HdmiInput_GetGeneralControlPacket( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiGeneralControlPacket *pGeneralControlPacket )
{
    NEXUS_Error errCode ;
    uint8_t temp ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pGeneralControlPacket) == sizeof(BAVC_HDMI_GcpData));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    errCode = BHDR_GetGeneralControlPacketData(hdmiInput->hdr, (BAVC_HDMI_GcpData*)pGeneralControlPacket);
    if (errCode) {
        goto done ;
    }

    /* GCP Color Depth values are different than magnum Color depth */
    switch (pGeneralControlPacket->colorDepth)
    {
        default :
        case BAVC_HDMI_GCP_ColorDepth_e24bpp : temp =  8 ; break ;
        case BAVC_HDMI_GCP_ColorDepth_e30bpp : temp = 10 ; break ;
        case BAVC_HDMI_GCP_ColorDepth_e36bpp : temp = 12 ; break ;
        case BAVC_HDMI_GCP_ColorDepth_e48bpp : temp = 16 ; break ;
    }

    pGeneralControlPacket->colorDepth = temp ;

done :
    return errCode ;
}


NEXUS_Error NEXUS_HdmiInput_GetISRCData( NEXUS_HdmiInputHandle hdmiInput, NEXUS_HdmiISRC *pISRC )
{
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BDBG_CASSERT(sizeof(*pISRC) == sizeof(BAVC_HDMI_ISRC));

    if (!CONNECTED(hdmiInput))
        return NEXUS_UNKNOWN ;

    return BHDR_GetISRCData(hdmiInput->hdr, (BAVC_HDMI_ISRC *)pISRC);
}



NEXUS_Error NEXUS_HdmiInput_DebugGetEyeDiagramData(NEXUS_HdmiInputHandle hdmiInput, uint8_t *padc_data)
{
#if BHDR_CONFIG_DEBUG_EYE_DIAGRAM

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    return BHDR_FE_DEBUG_GetEyeDiagramData(hdmiInput->hdr, padc_data);
#else
    BSTD_UNUSED(hdmiInput) ;
    BSTD_UNUSED(padc_data) ;
    return BERR_SUCCESS ;
#endif
}

void NEXUS_HdmiInput_SetFormatChangeCb_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputFormatChangeCb pFunction_isr,
    void *pFuncParam
    )
{
    NEXUS_ASSERT_MODULE();
    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    BKNI_EnterCriticalSection();
    hdmiInput->pPcFormatCallback_isr = pFunction_isr;
    hdmiInput->pPcFormatCallbackParam = pFuncParam;
    BKNI_LeaveCriticalSection();
}


static void NEXUS_HdmiInput_P_ReleaseHotPlug(void *context)
{
    NEXUS_HdmiInputHandle hdmiInput = context ;

    BDBG_OBJECT_ASSERT(hdmiInput, NEXUS_HdmiInput);
    hdmiInput->releaseHotPlugTimer = NULL;
    NEXUS_HdmiInput_SetHotPlug(hdmiInput, false) ;
}

bool NEXUS_HdmiInput_GetSecure_isrsafe(NEXUS_HdmiInputHandle hdmiInput)
{
    return hdmiInput->settings.secureVideo;
}
