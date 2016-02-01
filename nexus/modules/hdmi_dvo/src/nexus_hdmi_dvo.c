/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 **************************************************************************/

#include "nexus_hdmi_dvo_module.h"
#include "priv/nexus_hdmi_dvo_priv.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_core.h"
#include "bhdm.h"

BDBG_MODULE(nexus_hdmiDvo);

NEXUS_ModuleHandle g_NEXUS_hdmiDvoModule;
struct {
    NEXUS_HdmiDvoModuleSettings settings;
} g_NEXUS_hdmiDvo;


/******************
* Module functions
*******************/

void NEXUS_HdmiDvoModule_GetDefaultSettings(NEXUS_HdmiDvoModuleSettings *pSettings)
{
    BDBG_MSG((">NEXUS_HdmiDvoModule_GetDefaultSettings"));
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    BDBG_MSG(("<NEXUS_HdmiDvoModule_GetDefaultSettings"));
    return;
 }

NEXUS_ModuleHandle NEXUS_HdmiDvoModule_Init(const NEXUS_HdmiDvoModuleSettings *pSettings)
{
    BDBG_ASSERT(!g_NEXUS_hdmiDvoModule);
    BDBG_MSG((">NEXUS_HdmiDvoModule_Init"));
    g_NEXUS_hdmiDvoModule = NEXUS_Module_Create("hdmiDvo", NULL);
    if (pSettings) {
        g_NEXUS_hdmiDvo.settings = *pSettings;
    }
    else {
       NEXUS_HdmiDvoModule_GetDefaultSettings(&g_NEXUS_hdmiDvo.settings);
    }
    BDBG_MSG(("<NEXUS_HdmiDvoModule_Init"));
    return g_NEXUS_hdmiDvoModule;
}

void NEXUS_HdmiDvoModule_Uninit()
{
    BDBG_MSG((">NEXUS_HdmiDvoModule_Uninit"));
    NEXUS_Module_Destroy(g_NEXUS_hdmiDvoModule);
    g_NEXUS_hdmiDvoModule = NULL;
    BDBG_MSG(("<NEXUS_HdmiDvoModule_Uninit"));
    return;
}

/**************
* API functions
***************/

typedef struct NEXUS_HdmiDvo {
    NEXUS_OBJECT(NEXUS_HdmiDvo);
    NEXUS_HdmiDvoSettings settings;
    BHDM_Handle hdmHandle;
    bool opened;
    NEXUS_VideoOutputObject videoConnector;
    unsigned index;
}NEXUS_HdmiDvo;

/* Connect video and set format parameters */
NEXUS_Error NEXUS_HdmiDvo_SetDisplayParams_priv(
    NEXUS_HdmiDvoHandle hdmiDvo,
    BFMT_VideoFmt format,
    BAVC_MatrixCoefficients colorimetry,
    BFMT_AspectRatio aspectRatio
    )
{
    BHDM_Settings hdmSettings;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    BHDM_GetHdmiSettings(hdmiDvo->hdmHandle, &hdmSettings);
    hdmSettings.eAspectRatio = aspectRatio;
    hdmSettings.eInputVideoFmt = format;
    hdmSettings.eColorimetry = colorimetry;
    BDBG_MSG(("Setting HDMI Video Format to %d", format));
    BHDM_EnableDisplay(hdmiDvo->hdmHandle, &hdmSettings);

    return errCode;
}


void NEXUS_HdmiDvo_VideoRateChange_isr(
    NEXUS_HdmiDvoHandle hdmiDvo,
    BAVC_VdcDisplay_Info *pDisplayInfo
    )
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    BDBG_ASSERT(NULL != pDisplayInfo);

    BDBG_MSG(("HDMI Video Rate Change"));
    BHDM_AudioVideoRateChangeCB_isr(hdmiDvo->hdmHandle, BHDM_Callback_Type_eVideoChange, pDisplayInfo);
}


void NEXUS_HdmiDvo_GetColorimetry_priv(NEXUS_HdmiDvoHandle hdmiDvo,BAVC_MatrixCoefficients *pColorimetry)
{
    BDBG_MSG((">NEXUS_HdmiDvo_GetColorimetry_priv"));
    switch(hdmiDvo->settings.colorSpace){
    case NEXUS_ColorSpace_eYCbCr444:
         *pColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709;
         break;
    case NEXUS_ColorSpace_eRgb:
         *pColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB;
         break;
    default:
         *pColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB;

    }
    BDBG_MSG(("<NEXUS_HdmiDvo_GetColorimetry_priv"));
    return;
}

void NEXUS_HdmiDvo_GetDefaultSettings(NEXUS_HdmiDvoSettings *pSettings)
{
    BDBG_MSG((">NEXUS_HdmiDvo_GetDefaultSettings"));
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->outputMode = 12;
    pSettings->colorSpace = NEXUS_ColorSpace_eRgb;
    pSettings->colorDepth = NEXUS_HdmiColorDepth_e8bit;
    BDBG_MSG(("<NEXUS_HdmiDvo_GetDefaultSettings"));
    return;
}

NEXUS_HdmiDvoHandle
NEXUS_HdmiDvo_Open(unsigned index, const NEXUS_HdmiDvoSettings *pSettings)
{
    BERR_Code rc;
    NEXUS_HdmiDvo *pOutput;
    NEXUS_HdmiDvoSettings defaultSettings;
    BHDM_Settings hdmSettings;
    BDBG_MSG((">NEXUS_HdmiDvo_Open"));
    if(!pSettings) {
        NEXUS_HdmiDvo_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    if (index >= NEXUS_NUM_HDMI_DVO) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    pOutput = BKNI_Malloc(sizeof(*pOutput));
    if (!pOutput) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(pOutput, 0, sizeof(*pOutput));
    NEXUS_OBJECT_INIT(NEXUS_HdmiDvo, pOutput);

    pOutput->settings = *pSettings;
    NEXUS_VIDEO_OUTPUT_INIT(&pOutput->videoConnector, NEXUS_VideoOutputType_eHdmiDvo, pOutput);
    BHDM_GetDefaultSettings(&hdmSettings);
    hdmSettings.eOutputPort = BHDM_OutputPort_eDVO;
    switch(pSettings->outputMode)
    {
    case 12:
        hdmSettings.eOutputFormat = BHDM_OutputFormat_e12BitDVOMode;
        break;
    case 24:
        hdmSettings.eOutputFormat = BHDM_OutputFormat_e24BitDVOMode;
        break;
    default:
        break;
    }

	hdmSettings.bResumeFromS3 = false;
    rc = BHDM_Open(&pOutput->hdmHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, NULL, &hdmSettings);
    if ( rc ) {
        rc = BERR_TRACE(rc);
        goto error;
    }
    pOutput->opened = true;
    pOutput->index = index;
    BDBG_MSG(("<NEXUS_HdmiDvo_Open"));
    return pOutput;

error:
    NEXUS_HdmiDvo_Close(pOutput);
    return NULL;
}

static void NEXUS_HdmiDvo_P_Finalizer( NEXUS_HdmiDvoHandle hdmiDvo )
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    BHDM_Close(hdmiDvo->hdmHandle);
    NEXUS_OBJECT_DESTROY(NEXUS_HdmiDvo, hdmiDvo);
    BKNI_Free(hdmiDvo);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_HdmiDvo, NEXUS_HdmiDvo_Close);

NEXUS_VideoOutput NEXUS_HdmiDvo_GetConnector(NEXUS_HdmiDvoHandle hdmiDvo)
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    BDBG_MSG((">NEXUS_HdmiDvo_GetConnector"));
    return &hdmiDvo->videoConnector;
}

void NEXUS_HdmiDvo_GetSettings(NEXUS_HdmiDvoHandle hdmiDvo, NEXUS_HdmiDvoSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    BDBG_ASSERT(pSettings);
    BDBG_MSG((">NEXUS_HdmiDvo_GetSettings"));
    *pSettings = hdmiDvo->settings;
    return;
}

NEXUS_Error NEXUS_HdmiDvo_SetSettings(NEXUS_HdmiDvoHandle hdmiDvo, const NEXUS_HdmiDvoSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiDvo, hdmiDvo);
    hdmiDvo->settings = *pSettings;
    BDBG_MSG((">NEXUS_HdmiDvo_SetSettings"));
    return BERR_SUCCESS;

}

