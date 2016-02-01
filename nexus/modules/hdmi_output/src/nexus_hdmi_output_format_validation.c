/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *                      HdmiOutput: Specific interfaces for an HDMI/DVI output.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "nexus_hdmi_output_module.h"


BDBG_MODULE(nexus_hdmi_output_format_validation);

/* create the table */
typedef struct NEXUS_HdmiOutput_P_VideoSettings {
    NEXUS_ColorSpace colorSpace;
    NEXUS_HdmiColorDepth colorDepth;
} NEXUS_HdmiOutput_P_VideoSettings;

static const NEXUS_HdmiOutput_P_VideoSettings pVideoSettingsPriorityTable_privD0[] = {
    { NEXUS_ColorSpace_eYCbCr420, NEXUS_HdmiColorDepth_e8bit }
};

static const NEXUS_HdmiOutput_P_VideoSettings pVideoSettingsPriorityTable_privD1[] = {
    { NEXUS_ColorSpace_eYCbCr422, NEXUS_HdmiColorDepth_e12bit },
    { NEXUS_ColorSpace_eYCbCr420, NEXUS_HdmiColorDepth_e12bit },
    { NEXUS_ColorSpace_eYCbCr420, NEXUS_HdmiColorDepth_e10bit },
    { NEXUS_ColorSpace_eYCbCr444, NEXUS_HdmiColorDepth_e8bit },
    { NEXUS_ColorSpace_eYCbCr420, NEXUS_HdmiColorDepth_e8bit }
};

static const char * NEXUS_ColorSpace_Text[NEXUS_ColorSpace_eMax] =  {
    "Auto",
    "Rgb",
    "YCbCr422",   /* 2 */
    "YCbCr444",
    "YCbCr420",   /* 4 */
} ;


static const NEXUS_HdmiOutput_P_VideoSettings *pVideoSettingsPriorityTable_priv ;
static uint8_t numVideoSettingsPriorityTable ;

void NEXUS_HdmiOutput_RestorePrevSettings_priv(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_HdmiOutputSettings *previousSettings ;

    previousSettings = BKNI_Malloc(sizeof(NEXUS_HdmiOutputSettings)) ;
    if (previousSettings == NULL)
    {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY) ;
        return ;
    }

    BDBG_WRN(("Unable to apply requested settings; restoring previous settings")) ;
    *previousSettings = hdmiOutput->previousSettings;

    hdmiOutput->formatChangeUpdate = true;
    NEXUS_HdmiOutput_SetSettings(hdmiOutput, previousSettings);
    BKNI_Free(previousSettings) ;
}


static NEXUS_Error NEXUS_HdmiOutput_SelectEdidPreferredFormat_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred /* [out] */
)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    NEXUS_HdmiOutputEdidData edid ;
    NEXUS_HdmiOutputEdidVideoFormatSupport stPreferredVideoFormatSupport  ;
    BHDM_TxSupport platformHdmiOutputSupport ;

    BSTD_UNUSED(requested);

    /* Get the Capabilities of the attached Rx */
    errCode = NEXUS_HdmiOutput_GetEdidData(hdmiOutput, &edid);
    if (errCode) {BERR_TRACE(errCode); goto done ;}

    /* Get the Capabilities of the Tx */
    errCode = BHDM_GetTxSupportStatus(hdmiOutput->hdmHandle, &platformHdmiOutputSupport) ;
    if (errCode) {BERR_TRACE(errCode); goto done ;}


    preferred->videoFormat = edid.basicData.preferredVideoFormat ;

    /* Get the supported features of the Rx/EDID's preferred format (colorspace, 3d support, etc.) */
    errCode = NEXUS_HdmiOutput_GetVideoFormatSupport(hdmiOutput,
        preferred->videoFormat, &stPreferredVideoFormatSupport) ;
    if (errCode) {BERR_TRACE(errCode); goto done ;}


    /* use 420 colorSpace if preferred format is 4K */
    if  (((preferred->videoFormat == NEXUS_VideoFormat_e3840x2160p60hz)
    || (preferred->videoFormat == NEXUS_VideoFormat_e3840x2160p50hz))
    && (platformHdmiOutputSupport.YCbCr420))  /* platform can support 4K) */
    {
        preferred->colorSpace = NEXUS_ColorSpace_eYCbCr420 ;
    }
    else if (!stPreferredVideoFormatSupport.yCbCr444rgb444)  /* no standard HDMI 1.x 4:4:4 support */
    {
        preferred->videoFormat = NEXUS_VideoFormat_e1080p ;  /* TODO use 2nd Preferred Format */
        preferred->colorSpace = NEXUS_ColorSpace_eYCbCr444 ;
    }

    if (!edid.hdmiVsdb.yCbCr422 && !edid.hdmiVsdb.yCbCr444)
        preferred->colorSpace = NEXUS_ColorSpace_eRgb ;

    BDBG_WRN(("Switching to EDID preferred format (%d)", preferred->videoFormat)) ;

    preferred->colorDepth = NEXUS_HdmiColorDepth_e8bit ;

done:
    return errCode ;
}



static NEXUS_Error NEXUS_HdmiOutput_OverrideVideoSettings_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    bool overrideSettings = false ;

    if ((preferred->colorSpace != requested->colorSpace)
    && (!hdmiOutput->settings.overrideMatrixCoefficients))
    {
        BDBG_MSG(("Override requested color space of %d with %d",
            requested->colorSpace, preferred->colorSpace)) ;

        /* set override flag if the override value is different than current setting */
        if (preferred->colorSpace != hdmiOutput->settings.colorSpace)
            overrideSettings = true  ;
        hdmiOutput->settings.colorSpace = preferred->colorSpace ;
    }

    if (preferred->colorDepth != requested->colorDepth)
    {
        BDBG_MSG(("Override requested color depth of %d with %d",
            requested->colorDepth, preferred->colorDepth)) ;

        /* set override flag if the override value is different than current setting */
        if (preferred->colorDepth!= hdmiOutput->settings.colorDepth)
            overrideSettings = true ;
        hdmiOutput->settings.colorDepth = preferred->colorDepth ;
    }

    return errCode ;
}


static NEXUS_Error NEXUS_HdmiOutput_VideoFormatTmdsBitRate_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    const NEXUS_HdmiOutputVideoSettings *videoSettings,
    uint32_t *tmdsRate  /* [out] */
)

{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    BFMT_VideoFmt magnumFormat ;
    const BFMT_VideoInfo *pVideoFormatInfo ;

    BSTD_UNUSED(hdmiOutput) ;

    errCode = NEXUS_P_VideoFormat_ToMagnum_isrsafe(videoSettings->videoFormat, &magnumFormat) ;
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        return errCode ;
    }
    pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(magnumFormat) ;

    switch (videoSettings->colorSpace)
    {
    case NEXUS_ColorSpace_eRgb:
    case NEXUS_ColorSpace_eYCbCr444 :
        *tmdsRate = pVideoFormatInfo->ulPxlFreq * videoSettings->colorDepth / 8 ;
        break ;

    case NEXUS_ColorSpace_eYCbCr420 :
        *tmdsRate = pVideoFormatInfo->ulPxlFreq * (videoSettings->colorDepth / 8) / 2 ;
        break ;

    case NEXUS_ColorSpace_eYCbCr422 :
        *tmdsRate = pVideoFormatInfo->ulPxlFreq ;
        break ;

    default :
        BDBG_WRN(("AutoColor Space Selected; Use 422 to determine TMDS Rate")) ;
        *tmdsRate = pVideoFormatInfo->ulPxlFreq ;
        break ;
    }

    /* ulPxlFreq is stored as a multiple of BFMT_FREQ_FACTOR (100) */
    /* e.g. 74.25 x BFMT_FREQ_FACTOR = 7425 */
    *tmdsRate = *tmdsRate / BFMT_FREQ_FACTOR ;


    BDBG_MSG(("<%s> TMDS Rate of %d MHz calculated with Colorspace: %s  ColorDepth: %d  Pixel Freq: %d",
        pVideoFormatInfo->pchFormatStr,
        *tmdsRate, NEXUS_ColorSpace_Text[videoSettings->colorSpace],
        videoSettings->colorDepth,
        pVideoFormatInfo->ulPxlFreq / BFMT_FREQ_FACTOR)) ;

    return errCode ;
}


/**
Summary:
Check if the requested 4K 50/60 Hz Video Settings are supported by both the TV and the STB.
If not, reduce the settings to the best possible.
**/
static bool NEXUS_HdmiOutput_IsValid4KVideoSettings_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_VideoFormat videoFormat,
    const NEXUS_HdmiOutput_P_VideoSettings *pEntry,
    NEXUS_HdmiOutputEdidData *rXCapabilities,
    BHDM_TxSupport *tXCapabilities
)
{
    NEXUS_Error errCode ;
    NEXUS_HdmiOutputEdidVideoFormatSupport rxVideoFormatSupport ;
    BHDM_TxSupport platformHdmiOutputSupport ;
    NEXUS_HdmiOutputVideoSettings videoSettings ;
    uint32_t tmdsRate ;
    bool supported ;


    supported = false ;
    errCode = BHDM_GetTxSupportStatus(hdmiOutput->hdmHandle, &platformHdmiOutputSupport) ;
    if (errCode) { BERR_TRACE(errCode); goto done ;}

    /* For the requested format, get the features supported by the attached Rx (colorspaces, 3d support, etc.) */
    errCode = NEXUS_HdmiOutput_GetVideoFormatSupport(hdmiOutput,
        videoFormat, &rxVideoFormatSupport) ;
    if (errCode) { BERR_TRACE(errCode); goto done ;}

    /*****************/
    /*  Color Space   */
    /*****************/
    switch (pEntry->colorSpace)
    {
    case NEXUS_ColorSpace_eYCbCr422:
        supported = (tXCapabilities->YCbCr422 && rxVideoFormatSupport.yCbCr444rgb444) ;
        break ;

    case NEXUS_ColorSpace_eYCbCr444:
        supported = (tXCapabilities->YCbCr422 && rxVideoFormatSupport.yCbCr444rgb444) ;
        break ;

    case NEXUS_ColorSpace_eYCbCr420:
        supported = (tXCapabilities->YCbCr420 && rxVideoFormatSupport.yCbCr420) ;
        break ;

    default:
        /* not valid for 4K */
        supported = false ;
    }

    if (!supported)
    {
        goto done ;
    }

    /* determine the TMDS Character Rate and make sure Rx can support */
    BKNI_Memset(&videoSettings, 0, sizeof(NEXUS_HdmiOutputVideoSettings)) ;

    tmdsRate = 0 ;
    videoSettings.videoFormat = videoFormat ;
    videoSettings.colorSpace = pEntry->colorSpace ;
    videoSettings.colorDepth = pEntry->colorDepth ;
    errCode = NEXUS_HdmiOutput_VideoFormatTmdsBitRate_priv(hdmiOutput, &videoSettings, &tmdsRate) ;
    if ((errCode) || (!tmdsRate))
    {
        supported = false ;
        BERR_TRACE(errCode) ;
        goto done ;
    }

    /* confirm support for video format tmds rate */
    supported =
          (((rXCapabilities->hdmiVsdb.valid) && (tmdsRate <= rXCapabilities->hdmiVsdb.maxTmdsClockRate))
        || ((rXCapabilities->hdmiForumVsdb.valid) && (tmdsRate <= rXCapabilities->hdmiForumVsdb.maxTMDSCharacterRate))) ;

    BDBG_MSG(("Video Format TMDS Rate: %d < HDMI Rx Max TMDS Rate: %d  --  %s",
        tmdsRate, rXCapabilities->hdmiForumVsdb.maxTMDSCharacterRate,
        supported ? "SUPPORTED" : "***NOT SUPPORTED***")) ;

    if (!supported)
    {
        goto done ;
    }

    /* the HDMI Rx Supports the clock rates for the selected video format */
    /* now confirm the colordepth support is explicitly declared in the VSDB blocks */

    /*****************/
    /*  Color Depth   */
    /*****************/
    switch( pEntry->colorDepth )
    {

    /*****  8 Bit *****/
    case NEXUS_HdmiColorDepth_e8bit:
        /* All 4K 50/60 colorSpaces support 8bit */
        supported = true ;
        break ;

    /***** 10 Bit *****/
    case NEXUS_HdmiColorDepth_e10bit:
        switch (pEntry->colorSpace)
        {
        case NEXUS_ColorSpace_eYCbCr422:
            supported = rXCapabilities->hdmiVsdb.deepColor30bit ;
            break ;

        case NEXUS_ColorSpace_eYCbCr420:
            supported = rXCapabilities->hdmiForumVsdb.deepColor420_30bit ;
            break ;

        case NEXUS_ColorSpace_eYCbCr444:
            /* YCbCr 4:4:4 10 bit is not supported by HDMI 2.0 */
            supported =
                ((videoFormat != NEXUS_VideoFormat_e3840x2160p50hz)
            && (videoFormat != NEXUS_VideoFormat_e3840x2160p60hz)) ;
            if (!supported)
            {
                BDBG_WRN(("2160p50/60 YCbCr 4:4:4 10 Bit is not supported")) ;
            }
            break ;

        default:
            supported = false ;
         }
        break ;

    /***** 12 Bit *****/
    case NEXUS_HdmiColorDepth_e12bit:
        switch (pEntry->colorSpace)
        {
        case NEXUS_ColorSpace_eYCbCr422:
            supported = rXCapabilities->hdmiVsdb.deepColor36bit ;
            break ;

        case NEXUS_ColorSpace_eYCbCr420:
            supported = rXCapabilities->hdmiForumVsdb.deepColor420_36bit ;
            break ;

        case NEXUS_ColorSpace_eYCbCr444:
            /* YCbCr 4:4:4 12 bit is not supported by HDMI 2.0 */
            supported =
                ((videoFormat != NEXUS_VideoFormat_e3840x2160p50hz)
            && (videoFormat != NEXUS_VideoFormat_e3840x2160p60hz)) ;
            if (!supported)
            {
                BDBG_WRN(("2160p50/60 YCbCr 4:4:4 12 Bit is not supported")) ;
            }
            break ;

        default:
            supported = false ;
        }
        break ;

    default:
        supported = false ;
    }


done:
    return( supported );
}


NEXUS_Error NEXUS_HdmiOutput_ValidateVideoSettings4K_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred /* [out] */
)
{
    NEXUS_Error rc = NEXUS_SUCCESS ;
    NEXUS_HdmiOutputEdidVideoFormatSupport stRequestedVideoFormatSupport  ;
    NEXUS_HdmiOutputEdidData edid ;
    BHDM_TxSupport platformHdmiOutputSupport ;
    unsigned idx = 0;
    bool matchFound;
    NEXUS_HdmiOutputVideoSettings localRequested ;

    /* make/use a copy of the requested VideoSettings for validation */
    localRequested = *requested ;


    /* Get the Capabilities of the Tx */
    rc = BHDM_GetTxSupportStatus(hdmiOutput->hdmHandle, &platformHdmiOutputSupport) ;
    if (rc) {BERR_TRACE(rc); goto done ;}

    /* Get the Capabilities of the attached Rx */
    rc = NEXUS_HdmiOutput_GetEdidData(hdmiOutput, &edid);
    if (rc) {BERR_TRACE(rc); goto done ;}

    if (localRequested.videoFormat == NEXUS_VideoFormat_eUnknown)
        goto selectPreferredFormat ;

    /* Get the supported features of the requested format (colorspace, 3d support, etc.) */
    rc = NEXUS_HdmiOutput_GetVideoFormatSupport(hdmiOutput,
        localRequested.videoFormat, &stRequestedVideoFormatSupport) ;
    if (rc) {BERR_TRACE(rc); goto done ;}

    preferred->videoFormat = localRequested.videoFormat ;

    /* can the platform support 420 */
    if ((!platformHdmiOutputSupport.YCbCr420)
    && ((localRequested.videoFormat == NEXUS_VideoFormat_e3840x2160p50hz)
    || (localRequested.videoFormat == NEXUS_VideoFormat_e3840x2160p60hz)))
    {
        BDBG_WRN(("Platform does not support the 422/420 Colorspace required for 4K formats; use Rx preferred format instead")) ;
        goto selectPreferredFormat ;
    }

    /* can TV support a 4K format? */
    if ((!stRequestedVideoFormatSupport.yCbCr420)
    && (!stRequestedVideoFormatSupport.yCbCr444rgb444))
    {
        goto selectPreferredFormat ;
    }

#if BDBG_DEBUG_BUILD
    /* debug of priority table */
    BDBG_MSG((" ")) ;
    BDBG_MSG(("2160p Priority Table")) ;
    for (idx = 0; idx < numVideoSettingsPriorityTable ; idx++)
    {
        BDBG_MSG(("PriorityTable[%d] : ColorSpace: %d; ColorDepth: %d",idx,
            pVideoSettingsPriorityTable_priv[idx].colorSpace,
           pVideoSettingsPriorityTable_priv[idx].colorDepth)) ;
    }
    BDBG_MSG((" ")) ;
#endif


    matchFound = false ;

    /* when getting Auto Color Depth always get the best supported for 4K formats */
    if (localRequested.colorDepth == 0)
    {
        BDBG_MSG(("For 4K p50/60, Always use maximum color depth available")) ;
        localRequested.colorDepth = NEXUS_HdmiColorDepth_eMax ;
    }

    if (( localRequested.colorDepth > NEXUS_HdmiColorDepth_e8bit)
    &&  (edid.hdmiForumVsdb.maxTMDSCharacterRate <= BHDM_CONFIG_HDMI_1_4_MAX_RATE))
    {
        BDBG_WRN(("Attached Rx cannot support Color Depth %d; default to 8",
            requested->colorDepth)) ;
        localRequested.colorDepth = 8 ;
    }

    if (localRequested.colorSpace == NEXUS_ColorSpace_eRgb)
    {

        /* when checking our priority table and RGB is requested; search using YCbCr 444 Colorspace
            since it is the same timing as RGB 444
        */
        localRequested.colorSpace = NEXUS_ColorSpace_eYCbCr444 ;
    }

    if ((localRequested.colorSpace != NEXUS_ColorSpace_eAuto)
    && (localRequested.colorDepth != NEXUS_HdmiColorDepth_eMax))
    {
        NEXUS_HdmiOutput_P_VideoSettings videoSettings ;

        /* application specified colorSpace and colorDepth, validate requested settings */
        videoSettings.colorSpace = localRequested.colorSpace;
        videoSettings.colorDepth = localRequested.colorDepth;

        matchFound = NEXUS_HdmiOutput_IsValid4KVideoSettings_priv(hdmiOutput,
            requested->videoFormat, &videoSettings, &edid, &platformHdmiOutputSupport) ;

        if (matchFound)
        {
             preferred->colorSpace = videoSettings.colorSpace;
             preferred->colorDepth = videoSettings.colorDepth;
        }
    }
    else if ((localRequested.colorSpace != NEXUS_ColorSpace_eAuto)
          && (localRequested.colorDepth == NEXUS_HdmiColorDepth_eMax))
    {
        /* application specified colorSpace, not colorDepth, need to cycle through colorDepth */
        for (idx = 0; idx < numVideoSettingsPriorityTable ; idx++)
        {
            if (pVideoSettingsPriorityTable_priv[idx].colorSpace == localRequested.colorSpace)
            {
                matchFound = NEXUS_HdmiOutput_IsValid4KVideoSettings_priv(hdmiOutput,
                    requested->videoFormat, &pVideoSettingsPriorityTable_priv[idx],
                    &edid, &platformHdmiOutputSupport) ;

                if (matchFound)
                {
                    BDBG_MSG(("Use Video Settings Priority Table index %d", idx)) ;
                    preferred->colorSpace = pVideoSettingsPriorityTable_priv[idx].colorSpace;
                    preferred->colorDepth = pVideoSettingsPriorityTable_priv[idx].colorDepth;
                    break ;
               }
            }
        }
    }
    else if ((localRequested.colorSpace == NEXUS_ColorSpace_eAuto)
          && (localRequested.colorDepth != NEXUS_HdmiColorDepth_eMax))
    {
        /* application specified colorDepth, not colorSpace, need to cycle through colorSpace */
        for (idx = 0; idx < numVideoSettingsPriorityTable; idx++)
        {
            if (pVideoSettingsPriorityTable_priv[idx].colorDepth == localRequested.colorDepth)
            {
                matchFound = NEXUS_HdmiOutput_IsValid4KVideoSettings_priv(hdmiOutput,
                    requested->videoFormat, &pVideoSettingsPriorityTable_priv[idx],
                    &edid, &platformHdmiOutputSupport) ;

                if (matchFound)
                {
                    preferred->colorSpace = pVideoSettingsPriorityTable_priv[idx].colorSpace;
                    preferred->colorDepth = pVideoSettingsPriorityTable_priv[idx].colorDepth;
                    break ;
                }
            }
        }
    }

    if (!matchFound)
    {
        /* now find the best available color parameter */
        for (idx = 0; idx < numVideoSettingsPriorityTable; idx++)
        {
            matchFound = NEXUS_HdmiOutput_IsValid4KVideoSettings_priv(hdmiOutput,
                requested->videoFormat, &pVideoSettingsPriorityTable_priv[idx],
                &edid, &platformHdmiOutputSupport) ;

            if (matchFound)
            {
                preferred->colorSpace = pVideoSettingsPriorityTable_priv[idx].colorSpace;
                preferred->colorDepth = pVideoSettingsPriorityTable_priv[idx].colorDepth;
                BDBG_WRN(("Override with Priority Table index %d: Colorspace %d  ColorDepth: %d",
                    idx, preferred->colorSpace, preferred->colorDepth)) ;
                break ;
            }
        }
    }

    /* if requested colorspace was RGB (i.e. YCbCr was used just for timing look up */
    /* restore RGB as the preferred colorSpace  */
    if (requested->colorSpace == NEXUS_ColorSpace_eRgb)
    {
        preferred->colorSpace = NEXUS_ColorSpace_eRgb ;
    }

    BDBG_MSG(("Selected: colorSpace=%d, colorDepth=%d",
        preferred->colorSpace, preferred->colorDepth));
    goto done ;


selectPreferredFormat:
     rc = NEXUS_HdmiOutput_SelectEdidPreferredFormat_priv(hdmiOutput, requested, preferred) ;
    if (rc) {BERR_TRACE(rc); return rc ;}

done: ;
    rc = NEXUS_HdmiOutput_OverrideVideoSettings_priv(hdmiOutput, requested, preferred) ;

    return rc ;
}



/**
Summary:
Check if the requested Video Settings are supported by both the TV and the STB.
If not, reduce the settings to the best possible.
**/
NEXUS_Error NEXUS_HdmiOutput_ValidateVideoSettingsNon4K_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred /* [out] */
)
{
    NEXUS_Error rc = NEXUS_SUCCESS ;
    NEXUS_HdmiOutputEdidVideoFormatSupport stRequestedVideoFormatSupport  ;
    NEXUS_HdmiOutputEdidData edid ;
    BHDM_TxSupport platformHdmiOutputSupport ;
    bool requestedColorDepthSupported ;


    /* Get the Capabilities of the Tx */
    rc = BHDM_GetTxSupportStatus(hdmiOutput->hdmHandle, &platformHdmiOutputSupport) ;
    if (rc) {BERR_TRACE(rc); goto done ;}

    /* Get the Capabilities of the attached Rx */
    rc = NEXUS_HdmiOutput_GetEdidData(hdmiOutput, &edid);
    if (rc) {BERR_TRACE(rc); goto done ;}

    if (requested->videoFormat == NEXUS_VideoFormat_eUnknown)
        goto selectPreferredFormat ;

    /* Check the requested format settings (colorspace, etc.) */
    rc = NEXUS_HdmiOutput_GetVideoFormatSupport(hdmiOutput,
        requested->videoFormat, &stRequestedVideoFormatSupport) ;
    if (rc) {BERR_TRACE(rc); goto done ;}

    preferred->videoFormat = requested->videoFormat ;
    /************/
    /* Color Space */
    /************/
    switch(requested->colorSpace)
    {
    case  NEXUS_ColorSpace_eYCbCr420 :
        BDBG_WRN(("4:2:0 Color Space is not supported for non 4K format %d; Use 4:4:4 instead",
            requested->videoFormat)) ;
        preferred->colorSpace = NEXUS_ColorSpace_eYCbCr444 ;
        break ;

    case  NEXUS_ColorSpace_eYCbCr422 :
        if (!platformHdmiOutputSupport.YCbCr422)
        {
            BDBG_WRN(("Platform does not support the 422/420 Colorspace required for non 4K formats; Use 4:4:4 instead")) ;
            preferred->colorSpace = NEXUS_ColorSpace_eYCbCr444 ;
        }
        else
        {
            preferred->colorSpace = NEXUS_ColorSpace_eYCbCr422 ;
        }
        break ;

    case  NEXUS_ColorSpace_eRgb :
        preferred->colorSpace = NEXUS_ColorSpace_eRgb ;
        break ;

    default : /* use 444 colorspace */
        preferred->colorSpace = NEXUS_ColorSpace_eYCbCr444 ;
        break ;
    }

    /* non-4K 50/60 formats using 444 colorspace cannot exceed 8 bits on Rxs that do not support it */
    requestedColorDepthSupported = ((edid.hdmiVsdb.valid)
        && (((requested->colorDepth == 10) && (edid.hdmiVsdb.deepColor30bit))
          || ((requested->colorDepth == 12) && (edid.hdmiVsdb.deepColor36bit))
          || ((requested->colorDepth == 16) && (edid.hdmiVsdb.deepColor48bit)))) ;

    if (((preferred->colorSpace == NEXUS_ColorSpace_eRgb) || (preferred->colorSpace == NEXUS_ColorSpace_eYCbCr444))
    &&  (requested->colorDepth > 8) && (!requestedColorDepthSupported))
    {
        BDBG_WRN(("Attached Rx cannot support Color Depth %d; default to 8", requested->colorDepth)) ;
        preferred->colorDepth = 8 ;
    }
    /* current platform must support clock rates and bit depths > 8 */
    else if (platformHdmiOutputSupport.MaxTmdsRateMHz == BHDM_CONFIG_HDMI_1_4_MAX_RATE)
    {
        BDBG_MSG(("Platform Supports Clock Rates below %d only",
            BHDM_CONFIG_HDMI_1_4_MAX_RATE)) ;
        preferred->colorDepth = 8 ;
    }
    else
    {
        /***********/
        /*Color Depth */
        /***********/
        switch (requested->colorDepth)
        {
        default :
            BDBG_MSG(("Unknown ColorDepth %d ; Using Auto Color Depth",
                requested->colorDepth)) ;

            /* FALL  THROUGH */

        case NEXUS_HdmiColorDepth_eMax :
            preferred->colorDepth = 8 ;  /* initialize to 8 bit Color Depth */
            if (!stRequestedVideoFormatSupport.yCbCr444rgb444) /* supports YcbCr 422 */
            {
                /* should never get here 4:4:4: should always be supported */
                BDBG_ERR(("Requested format %d is not supported", requested->videoFormat)) ;
                goto selectPreferredFormat ;
            }

            /* ***FALL THROUGH*** */

        case NEXUS_HdmiColorDepth_e16bit:
            BDBG_MSG(("16 bit Color Depth is not supported on this platform... checking lower color depths")) ;

            /* ***FALL THROUGH*** */

        case NEXUS_HdmiColorDepth_e12bit:
            if (edid.hdmiVsdb.deepColor36bit)
            {
                preferred->colorDepth = 12 ;
                break ;
            }
            BDBG_WRN(("12 bit Color Depth is not supported by <%s> Rx... checking lower color depths",
                edid.basicData.monitorName)) ;

            /* ***FALL THROUGH*** */

        case NEXUS_HdmiColorDepth_e10bit:
            if (edid.hdmiVsdb.deepColor30bit)
            {
                preferred->colorDepth = 10 ;
                break ;
            }
            BDBG_WRN(("10 bit Color Depth is not supported by <%s>... will use standard 8 bit Color",
                edid.basicData.monitorName)) ;

            /* ***FALL THROUGH*** */

        case 0 : /* For non-4k auto colordepth(=0), use 8 bits for consistent performance in prior releases  */
        case NEXUS_HdmiColorDepth_e8bit :
            preferred->colorDepth = 8 ;
            break ;
        }
    }

    /* validate the requested colorspace if override not specified (overrideMatrixCoefficients) */
    if (!hdmiOutput->settings.overrideMatrixCoefficients)
   {
        if (!edid.hdmiVsdb.valid)  /* No VSDB... DVI support only */
        {
	        preferred->colorSpace = NEXUS_ColorSpace_eRgb;
        }
        else if ((!edid.hdmiVsdb.yCbCr422) && (!edid.hdmiVsdb.yCbCr444)
        && ((requested->colorSpace == NEXUS_ColorSpace_eYCbCr444)
        ||  (requested->colorSpace == NEXUS_ColorSpace_eYCbCr422)
        ||  (requested->colorSpace == NEXUS_ColorSpace_eAuto)))
        {
            preferred->colorSpace = NEXUS_ColorSpace_eRgb;
        }
    }

    goto done ;


selectPreferredFormat:
    rc = NEXUS_HdmiOutput_SelectEdidPreferredFormat_priv(hdmiOutput, requested, preferred) ;
    if (rc) {BERR_TRACE(rc); return rc ;}

done: ;
    rc = NEXUS_HdmiOutput_OverrideVideoSettings_priv(hdmiOutput, requested, preferred) ;

    return rc ;

}


NEXUS_Error NEXUS_HdmiOutput_ValidateVideoSettings_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred /* [out] */
)
{
    NEXUS_Error errCode ;
    uint8_t deviceAttached ;

    /* Check for device */
    errCode = BHDM_RxDeviceAttached(hdmiOutput->hdmHandle, &deviceAttached) ;
    BERR_TRACE(errCode) ;
    if (!deviceAttached)
    {
        BDBG_WRN(("HdmiOutput is not connected; Video Settings cannot be validated")) ;
        errCode = NEXUS_NOT_AVAILABLE ;
        goto done ;
    }


    BDBG_MSG(("===> Request %s Format: %d; Colorspace: (%d) %s; Colordepth: %d",
        ((requested->videoFormat == NEXUS_VideoFormat_e3840x2160p60hz)
        || (requested->videoFormat == NEXUS_VideoFormat_e3840x2160p50hz)) ? "4K" : "non-4K",
        requested->videoFormat,
        requested->colorSpace, NEXUS_ColorSpace_Text[requested->colorSpace],
        requested->colorDepth)) ;

    if ((requested->videoFormat == NEXUS_VideoFormat_e3840x2160p60hz)
    || (requested->videoFormat == NEXUS_VideoFormat_e3840x2160p50hz)
    || (requested->videoFormat == NEXUS_VideoFormat_e3840x2160p30hz)
    || (requested->videoFormat == NEXUS_VideoFormat_e3840x2160p25hz)
    || (requested->videoFormat == NEXUS_VideoFormat_e3840x2160p24hz))
    {
        /*****************/
        /* Validate 4K Format */
        /*****************/
        errCode = NEXUS_HdmiOutput_ValidateVideoSettings4K_priv(hdmiOutput, requested, preferred) ;
    }
    else
    {
        /*********************/
        /* Validate NON 4K Format */
        /*********************/
        errCode = NEXUS_HdmiOutput_ValidateVideoSettingsNon4K_priv(hdmiOutput, requested, preferred) ;
    }

    BDBG_MSG(("<=== Recommend Format: %d; Colorspace: (%d) %s; Colordepth: %d",
        preferred->videoFormat,
        preferred->colorSpace, NEXUS_ColorSpace_Text[preferred->colorSpace],
        preferred->colorDepth)) ;

done:
    return errCode ;
}


void NEXUS_HdmiOutput_SelectVideoSettingsPriorityTable_priv(void)
{
    BCHP_Info info;
    BCHP_GetInfo(g_pCoreHandles->chp, &info);
    if ((info.familyId == 0x7445) && (info.rev == 0x30))
    {
        pVideoSettingsPriorityTable_priv = &pVideoSettingsPriorityTable_privD0[0] ;
        numVideoSettingsPriorityTable = sizeof(pVideoSettingsPriorityTable_privD0) / sizeof(*pVideoSettingsPriorityTable_privD0) ;
        BDBG_MSG(("USE D0 Priority Table; Number of entries: %d", numVideoSettingsPriorityTable)) ;
    }
    else
    {
        pVideoSettingsPriorityTable_priv = &pVideoSettingsPriorityTable_privD1[0] ;
        numVideoSettingsPriorityTable = sizeof(pVideoSettingsPriorityTable_privD1) / sizeof(*pVideoSettingsPriorityTable_privD1) ;
        BDBG_MSG(("USE D1 Priority Table; Number of entries: %d", numVideoSettingsPriorityTable)) ;
    }
}
