/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_hdmi_input_hdcp_priv.h"

#if !NEXUS_NUM_HDMI_INPUTS
#error Platform must define NEXUS_NUM_HDMI_INPUTS
#endif

NEXUS_ModuleHandle g_NEXUS_hdmiInputModule;
NEXUS_HdmiInputModuleSettings g_NEXUS_hdmiInputModuleSettings;
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
NEXUS_HdmiInput_SageData g_NEXUS_hdmiInputSageData;
NEXUS_HdmiInputMemoryBlock g_hdmiInputTABlock;
#endif

/* HdmiInput global state */
static struct {
    NEXUS_HdmiInputModuleSettings settings;
    BHDR_FE_Handle fe;
    bool initInProgress;
    NEXUS_HdmiInputHandle handle[NEXUS_NUM_HDMI_INPUTS] ;
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat hdmiVideoFormat ;
} g_NEXUS_hdmiInput;

BDBG_MODULE(nexus_hdmi_input_module);

static void NEXUS_HdmiInputModule_Print(void)
{
#if BDBG_DEBUG_BUILD
    unsigned i;
    BDBG_LOG(("HdmiInputModule:"));
    for (i=0;i<NEXUS_NUM_HDMI_INPUTS;i++) {
        NEXUS_HdmiInputHandle hdmiInput = g_NEXUS_hdmiInput.handle[i];
        if (!hdmiInput) continue;
#if NEXUS_HAS_AUDIO
        BDBG_LOG(("hdmi_input %d: handle=%p, videoInput=%p, audioInput=%p",
            i, (void *)hdmiInput, (void *)&hdmiInput->videoInput, (void *)&hdmiInput->audioInput));
#endif
        BDBG_LOG(("    videoConnected=%d, %dx%d%c, noSignal=%d, originalFormat=%d, master=%p",
            hdmiInput->videoConnected, hdmiInput->vdcStatus.avWidth, hdmiInput->vdcStatus.avHeight, hdmiInput->vdcStatus.interlaced?'i':'p',
            hdmiInput->vdcStatus.noSignal, hdmiInput->vdcStatus.originalFormat, (void *)hdmiInput->masterHdmiInput));
    }
#endif
}

void NEXUS_HdmiInputModule_GetDefaultSettings(NEXUS_HdmiInputModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_ModuleHandle NEXUS_HdmiInputModule_Init(const NEXUS_HdmiInputModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    NEXUS_Error errCode ;

    BERR_Code rc;
    BHDR_FE_Settings feSettings;

    uint8_t index ;
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_HdmiInputSettings hdmiInputSettings ;

    if (g_NEXUS_hdmiInputModule) {
        BDBG_ERR(("Module already initialized"));
        return NULL;
    }

    if (!pSettings) {
        NEXUS_HdmiInputModule_GetDefaultSettings(&g_NEXUS_hdmiInputModuleSettings);
        pSettings = &g_NEXUS_hdmiInputModuleSettings;
    }
    else {
        g_NEXUS_hdmiInputModuleSettings = *pSettings;
    }
#if NEXUS_HAS_SECURITY
    if (!pSettings->modules.security)
    {
        BDBG_WRN(("security module handle required"));
        return NULL;
    }
#endif

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow;
    moduleSettings.dbgPrint = NEXUS_HdmiInputModule_Print;
    moduleSettings.dbgModules = "nexus_hdmi_input_module";
    g_NEXUS_hdmiInputModule = NEXUS_Module_Create("hdmi_input", &moduleSettings);
    if ( NULL == g_NEXUS_hdmiInputModule )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        return NULL;
    }
    NEXUS_LockModule();

    rc = BHDR_FE_GetDefaultSettings(g_pCoreHandles->chp, &feSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BHDR_FE_Open(&g_NEXUS_hdmiInput.fe, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &feSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    g_NEXUS_hdmiInput.initInProgress = true;
    for (index = 0 ; index < NEXUS_NUM_HDMI_INPUTS; index++)
    {
        NEXUS_HdmiInput_GetDefaultSettings(&hdmiInputSettings) ;
        hdmiInput = NEXUS_HdmiInput_Open(index, &hdmiInputSettings) ;

        if (hdmiInput) {
            NEXUS_HdmiInput_Close(hdmiInput) ;
        }
    }

    g_NEXUS_hdmiInput.initInProgress = false;

    g_NEXUS_hdmiInput.settings = *pSettings;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    /* Delay the process of populating until Sage is up */
    BKNI_Memset(&g_NEXUS_hdmiInputSageData, 0, sizeof(g_NEXUS_hdmiInputSageData));
    g_hdmiInputTABlock.buf = NULL;
    g_hdmiInputTABlock.len = 0;
#endif

    BDBG_MSG(("hdmiInput Module Init Complete...")) ;

    NEXUS_UnlockModule();
    return g_NEXUS_hdmiInputModule;

error:
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_hdmiInputModule);
    g_NEXUS_hdmiInputModule = NULL;
    return NULL;
}

void NEXUS_HdmiInputModule_Uninit()
{
    NEXUS_LockModule();
    BHDR_FE_Close(g_NEXUS_hdmiInput.fe);
    NEXUS_UnlockModule();

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    if (g_hdmiInputTABlock.buf != NULL)
    {
        NEXUS_Memory_Free(g_hdmiInputTABlock.buf);
        g_hdmiInputTABlock.buf = NULL;
        g_hdmiInputTABlock.len = 0;
    }
#endif

    NEXUS_Module_Destroy(g_NEXUS_hdmiInputModule);
    g_NEXUS_hdmiInputModule = NULL;
    BKNI_Memset(&g_NEXUS_hdmiInput, 0, sizeof(g_NEXUS_hdmiInput));
}


NEXUS_Error NEXUS_HdmiInputModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BHDR_StandbySettings standbySettings;
    BERR_Code  rc;

    BSTD_UNUSED(pSettings);

    if(!g_NEXUS_hdmiInput.handle[0]) {return NEXUS_SUCCESS;}

    if(enabled) {
	BHDR_GetDefaultStandbySettings(&standbySettings);
	rc = BHDR_Standby(g_NEXUS_hdmiInput.fe, &standbySettings);
    } else {
	rc = BHDR_Resume(g_NEXUS_hdmiInput.fe);
    }
    if (rc) return BERR_TRACE(rc);

#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
#endif

    return NEXUS_SUCCESS ;
}


/****************************************
* API functions
***************/

void NEXUS_HdmiInput_GetDefaultSettings(NEXUS_HdmiInputSettings *pHdmiInputDefaultSettings)
{
    BHDR_FE_ChannelSettings defaultHdmiFeChannelSettings ;
    BHDR_Settings defaultHdmiRxSettings ;

    BKNI_Memset(pHdmiInputDefaultSettings, 0, sizeof(*pHdmiInputDefaultSettings));

    pHdmiInputDefaultSettings->timebase = NEXUS_Timebase_e0;
    pHdmiInputDefaultSettings->autoColorSpace = true;
    pHdmiInputDefaultSettings->colorSpace = NEXUS_ColorSpace_eRgb;

    /* get default Front End Channel Settings */
    BHDR_FE_GetDefaultChannelSettings(g_NEXUS_hdmiInput.fe, &defaultHdmiFeChannelSettings) ;
    pHdmiInputDefaultSettings->frontend.equalizationEnabled = defaultHdmiFeChannelSettings.bEnableEqualizer ;
    pHdmiInputDefaultSettings->frontend.externalSwitch = defaultHdmiFeChannelSettings.bExternalSwitch ;
    pHdmiInputDefaultSettings->frontend.hpdDisconnected = defaultHdmiFeChannelSettings.bHpdDisconnected ;

    /* get default HDMI Rx Phy Settings */
    BHDR_GetDefaultSettings(&defaultHdmiRxSettings) ;
    pHdmiInputDefaultSettings->hdr.parseAviInfoframe =
	defaultHdmiRxSettings.bParseAVI ;

    pHdmiInputDefaultSettings->hdr.disableI2cSclPullUp =
	defaultHdmiRxSettings.bDisableI2cPadSclPullup ;

    pHdmiInputDefaultSettings->hdr.disableI2cSdaPullUp =
	defaultHdmiRxSettings.bDisableI2cPadSdaPullup ;

    pHdmiInputDefaultSettings->hdr.enableHdmiHardwarePassthrough =
	defaultHdmiRxSettings.bHdmiHardwarePassthrough ;

    /* default to use internally declared EDID */
    pHdmiInputDefaultSettings->useInternalEdid = true ;
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->avMuteChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->sourceChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->aviInfoFrameChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->audioInfoFrameChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->spdInfoFrameChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->vendorSpecificInfoFrameChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->audioContentProtectionChanged);
    NEXUS_CallbackDesc_Init(&pHdmiInputDefaultSettings->frontend.hotPlugCallback);

    return;
}


NEXUS_Error NEXUS_HdmiInput_P_OpenHdmiFe(NEXUS_HdmiInputHandle hdmiInput, const NEXUS_HdmiInputSettings *pHdmiInputSettings)
{
    BERR_Code rc;
    BHDR_FE_ChannelSettings frontendSettings;

    rc = BHDR_FE_GetDefaultChannelSettings(g_NEXUS_hdmiInput.fe, &frontendSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    /* override frontend default settings w/ user specified settings */
    if (pHdmiInputSettings)
    {
        frontendSettings.bEnableEqualizer = pHdmiInputSettings->frontend.equalizationEnabled;
        frontendSettings.bExternalSwitch  = pHdmiInputSettings->frontend.externalSwitch;
        frontendSettings.bHpdDisconnected = pHdmiInputSettings->frontend.hpdDisconnected ;
    }
    frontendSettings.uiChannel = hdmiInput->index;

    rc = BHDR_FE_OpenChannel(g_NEXUS_hdmiInput .fe, &hdmiInput->frontend, &frontendSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}


    hdmiInput->hotPlugCallback = NEXUS_IsrCallback_Create(hdmiInput->frontend, NULL);


    rc = BHDR_FE_InstallHotPlugCallback(hdmiInput->frontend,
        NEXUS_HdmiInput_P_HotPlug_isr, hdmiInput, 0);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return NEXUS_SUCCESS ;


error:
    BDBG_ERR(("Error Opening HDMI_RX Frontend")) ;
    return rc ;

}


NEXUS_Error  NEXUS_HdmiInput_P_OpenHdmiRx(NEXUS_HdmiInputHandle  hdmiInput,
	const NEXUS_HdmiInputSettings *pHdmiInputSettings)
{
    NEXUS_HdmiInputSettings defaultHdmiInputSettings;
    BHDR_Settings hdrSettings;
    BERR_Code rc;

    hdmiInput->avMuteCallback        = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->aviInfoFrameChanged   = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->audioInfoFrameChanged = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->spdInfoFrameChanged   = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->vendorSpecificInfoFrameChanged = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->audioContentProtectionChanged  = NEXUS_IsrCallback_Create(hdmiInput, NULL);
    hdmiInput->hdcpRxChanged = NEXUS_IsrCallback_Create(hdmiInput, NULL);

    hdmiInput->sourceChangedCallback = NEXUS_TaskCallback_Create(hdmiInput, NULL);

    hdmiInput->bSentResetHdDviBegin = false;
    hdmiInput->stFieldData.eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709;
    hdmiInput->stFieldData.eTransferCharacteristics = BAVC_TransferCharacteristics_eItu_R_BT_709 ;
    hdmiInput->stFieldData.eColorSpace = BAVC_Colorspace_eRGB;
    hdmiInput->stFieldData.eAspectRatio = BFMT_AspectRatio_eUnknown;
    hdmiInput->manualColorSpace = BAVC_Colorspace_eRGB;
    hdmiInput->reportedColorSpace = BAVC_Colorspace_eRGB;

    BHDR_GetDefaultSettings(&hdrSettings);

    if (!pHdmiInputSettings)
    {
        NEXUS_HdmiInput_GetDefaultSettings(&defaultHdmiInputSettings);
        pHdmiInputSettings = &defaultHdmiInputSettings;
    }
    else
    {
        /* overwrite magnum defaults with selected settings from app */
        hdrSettings.bDisableI2cPadSclPullup = pHdmiInputSettings->hdr.disableI2cSclPullUp ;
        hdrSettings.bDisableI2cPadSdaPullup = pHdmiInputSettings->hdr.disableI2cSdaPullUp ;
        hdrSettings.bParseAVI = pHdmiInputSettings->hdr.parseAviInfoframe ;
        hdrSettings.bHdmiHardwarePassthrough = pHdmiInputSettings->hdr.enableHdmiHardwarePassthrough ;
    }
    hdrSettings.eCoreId = hdmiInput->index;
    hdrSettings.hTmr = g_pCoreHandles->tmr;


    rc = BHDR_Open(&hdmiInput->hdr, g_pCoreHandles->chp,
        g_pCoreHandles->reg, g_pCoreHandles->bint, &hdrSettings) ;
    if (rc) {BERR_TRACE(rc); goto error;}


    rc = BHDR_InstallVideoFormatChangeCallback(hdmiInput->hdr,
        NEXUS_HdmiInput_P_VideoFormatChange_isr, hdmiInput, 0);
    if (rc) {BERR_TRACE(rc); goto error;}

    rc = BHDR_InstallAvMuteNotifyCallback(hdmiInput->hdr,
        NEXUS_HdmiInput_P_AvMuteNotify_isr, hdmiInput, 0);
    if (rc) {BERR_TRACE(rc); goto error;}

    rc = BHDR_InstallPacketChangeCallback(hdmiInput->hdr,
        NEXUS_HdmiInput_P_PacketChange_isr, hdmiInput, 0);
    if (rc) {BERR_TRACE(rc); goto error;}

    BKNI_CreateEvent(&hdmiInput->frameRateEvent);
    hdmiInput->frameRateHandler = NEXUS_RegisterEvent(hdmiInput->frameRateEvent,
        NEXUS_HdmiInput_P_SetFrameRate, hdmiInput);

    NEXUS_VIDEO_INPUT_INIT(&hdmiInput->videoInput, NEXUS_VideoInputType_eHdmi, hdmiInput);
#if NEXUS_HAS_AUDIO
    BKNI_Snprintf(hdmiInput->audioConnectorName, sizeof(hdmiInput->audioConnectorName), "HDMI INPUT");
    NEXUS_AUDIO_INPUT_INIT(&hdmiInput->audioInput, NEXUS_AudioInputType_eHdmi, hdmiInput);
    hdmiInput->audioInput.pName = hdmiInput->audioConnectorName;
#endif

    /* do not attach the core (i.e. enable interrupts) at module init ; wait until core is opened for use   */
    if (!g_NEXUS_hdmiInput.initInProgress)
    {
        rc = BHDR_FE_AttachHdmiRxCore(hdmiInput->frontend, hdmiInput->hdr) ;
        if (rc) {rc = BERR_TRACE(rc); goto error;}
    }


    NEXUS_HdmiInput_SetSettings(hdmiInput, pHdmiInputSettings);

     return NEXUS_SUCCESS ;

error:
    BDBG_WRN(("Error Opening HdmiRx Phy")) ;
    return rc ;
}


NEXUS_HdmiInputHandle NEXUS_HdmiInput_Open(unsigned index, const NEXUS_HdmiInputSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_HdmiInputHandle hdmiInput;
    BERR_Code rc;
    NEXUS_HdmiInputSettings defaultSettings;


    if (index >= NEXUS_NUM_HDMI_INPUTS) {
        BDBG_ERR(("invalid HdmiInput[%d]", index));
        return NULL;
    }
    if (g_NEXUS_hdmiInput.handle[index]) {
        BDBG_ERR(("HdmiInput[%d] already open", index));
        return NULL;
    }

    if (!pSettings) {
        NEXUS_HdmiInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    hdmiInput = BKNI_Malloc(sizeof(*hdmiInput));
    if (!hdmiInput) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_HdmiInput, hdmiInput);

    hdmiInput->index = index ;

    /* Open HDMI Rx Frontend first */
    rc = NEXUS_HdmiInput_P_OpenHdmiFe(hdmiInput, pSettings) ;
    if (rc) {rc = BERR_TRACE(rc); goto error;}


    /* Open HDMI Rx core second */
    rc = NEXUS_HdmiInput_P_OpenHdmiRx(hdmiInput, pSettings) ;

    if (pSettings->useInternalEdid)
    {
        errCode  = NEXUS_HdmiInput_LoadEdidData(hdmiInput, NULL, 0) ;
        if (errCode) {errCode = BERR_TRACE(errCode); goto error;}

        /* always toggle the HPD line when an EDID is loaded */
        NEXUS_HdmiInput_ToggleHotPlug(hdmiInput) ;
    }
    else /* toggle HP now if using separate PROM - DTV case */
    {
        if (!g_NEXUS_hdmiInput.initInProgress) /* only toggle hot plug if module has been initialized and device is connected */
        {
            BHDR_Status rxStatus ;

            rc = BHDR_GetHdmiRxStatus(hdmiInput->hdr, &rxStatus);
            if (rc) {rc = BERR_TRACE(rc); }

            if (rxStatus.DeviceAttached)
                NEXUS_HdmiInput_ToggleHotPlug(hdmiInput) ;
            else
            {
                BKNI_EnterCriticalSection() ;
                BHDR_ConfigureAfterHotPlug_isr(hdmiInput->hdr, false) ;
                BKNI_LeaveCriticalSection() ;
            }
        }
    }


    g_NEXUS_hdmiInput.handle[index] = hdmiInput ;
    NEXUS_OBJECT_REGISTER(NEXUS_VideoInput, &hdmiInput->videoInput, Open);
#if NEXUS_HAS_AUDIO
    NEXUS_OBJECT_REGISTER(NEXUS_AudioInput, &hdmiInput->audioInput, Open);
#endif

	/* Initialize HDCP2.2 sage module */
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
	/* Only Initialize HDCP2.2 once the module has been initialized */
	if (!g_NEXUS_hdmiInput.initInProgress)
	{
		errCode = NEXUS_HdmiInput_P_InitHdcp2x(hdmiInput);
		if (errCode != NEXUS_SUCCESS)
		{
			BDBG_ERR(("Error initialize Hdcp2.2 module"));
		}
	}
#endif

    return hdmiInput;
error:


    NEXUS_HdmiInput_Close(hdmiInput);
    return NULL;
}


NEXUS_HdmiInputHandle NEXUS_HdmiInput_OpenWithEdid(
    unsigned index,
    const NEXUS_HdmiInputSettings *pHdmiInputSettings,
    const uint8_t *edidData,
    uint16_t edidDataSize )
{
    NEXUS_Error errCode ;
    NEXUS_HdmiInputHandle hdmiInput ;

    hdmiInput = NEXUS_HdmiInput_Open(index, pHdmiInputSettings) ;
    if (!hdmiInput)
        return NULL ;


    BDBG_MSG(("Using BCM%d on-chip EDID RAM...", BCHP_CHIP)) ;

    if (edidData && (edidDataSize % 128))
    {
        BDBG_ERR(("EDID Block(s) size '%d' is not a multiple of 128", edidDataSize));
        BERR_TRACE(BERR_INVALID_PARAMETER);
        goto error ;
    }

    errCode  = NEXUS_HdmiInput_LoadEdidData(hdmiInput, edidData, edidDataSize) ;
    if (errCode) {errCode = BERR_TRACE(errCode); goto error;}

    /* always toggle the HPD line when an EDID is loaded */
    NEXUS_HdmiInput_ToggleHotPlug(hdmiInput) ;

    return hdmiInput ;

error:
    NEXUS_HdmiInput_Close(hdmiInput);
    return NULL;
}

static void NEXUS_HdmiInput_P_Finalizer(NEXUS_HdmiInputHandle hdmiInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiInput, hdmiInput);
    if (hdmiInput->releaseHotPlugTimer) {
        NEXUS_CancelTimer(hdmiInput->releaseHotPlugTimer);
        hdmiInput->releaseHotPlugTimer = NULL;
    }

#if NEXUS_HAS_AUDIO
    if ( NULL != hdmiInput->audioInput.pMixerData )
    {
        NEXUS_Error errCode;
        BDBG_ERR(("Audio connector is still active.  Please call NEXUS_AudioInput_Shutdown()."));
        errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
#endif

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    /* Do not uninit Hdcp22 during the hdmiInput module initialization process */
    if (!g_NEXUS_hdmiInput.initInProgress) {
        NEXUS_HdmiInput_P_UninitHdcp2x(hdmiInput);
    }
#endif


    /* now uninstall the HDMI Rx Frontend call backs */
    if (hdmiInput->frontend) {
        BHDR_FE_UnInstallHotPlugCallback(hdmiInput->frontend, NEXUS_HdmiInput_P_HotPlug_isr);
    }


    /* now, uninstall the HDMI Rx Core callbacks ; close the core */
    if (hdmiInput->hdr)
    {
        BHDR_UnInstallVideoFormatChangeCallback(
            hdmiInput->hdr, NEXUS_HdmiInput_P_VideoFormatChange_isr) ;

        BHDR_UnInstallAvMuteNotifyCallback(hdmiInput->hdr,
            NEXUS_HdmiInput_P_AvMuteNotify_isr);

        BHDR_UnInstallPacketChangeCallback(hdmiInput->hdr,
            NEXUS_HdmiInput_P_PacketChange_isr) ;

#if BHDR_CONFIG_HDCP_KEY_OTP_ROM
        BHDR_HDCP_UnInstallHdcpStatusChangeCallback(
            hdmiInput->hdr, NEXUS_HdmiInput_P_HdcpStateChange_isr) ;
#endif
        BHDR_Close(hdmiInput->hdr);
    }

    /* close the frontend channel */
    if (hdmiInput->frontend)
    {
        BHDR_FE_CloseChannel(hdmiInput->frontend);
        hdmiInput->frontend = NULL;
    }

    /* now clean up the nexus callbacks */
    if (hdmiInput->frameRateHandler) {
        NEXUS_UnregisterEvent(hdmiInput->frameRateHandler);
        hdmiInput->frameRateHandler = NULL;
    }

    if (hdmiInput->frameRateEvent) {
        BKNI_DestroyEvent(hdmiInput->frameRateEvent);
        hdmiInput->frameRateEvent = NULL;
    }

    if (hdmiInput->aviInfoFrameChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->aviInfoFrameChanged);
        hdmiInput->aviInfoFrameChanged = NULL;
    }

    if (hdmiInput->audioInfoFrameChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->audioInfoFrameChanged);
        hdmiInput->audioInfoFrameChanged = NULL;
    }

    if (hdmiInput->spdInfoFrameChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->spdInfoFrameChanged);
        hdmiInput->spdInfoFrameChanged = NULL;
    }

    if (hdmiInput->vendorSpecificInfoFrameChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->vendorSpecificInfoFrameChanged);
        hdmiInput->vendorSpecificInfoFrameChanged = NULL;
    }

    if (hdmiInput->audioContentProtectionChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->audioContentProtectionChanged);
        hdmiInput->audioContentProtectionChanged = NULL;
    }

    if (hdmiInput->avMuteCallback) {
        NEXUS_IsrCallback_Destroy(hdmiInput->avMuteCallback);
        hdmiInput->avMuteCallback = NULL;
    }

    if (hdmiInput->hdcpRxChanged) {
        NEXUS_IsrCallback_Destroy(hdmiInput->hdcpRxChanged);
        hdmiInput->hdcpRxChanged = NULL;
    }

    if (hdmiInput->sourceChangedCallback) {
        NEXUS_TaskCallback_Destroy(hdmiInput->sourceChangedCallback);
        hdmiInput->sourceChangedCallback = NULL;
    }

    if (hdmiInput->hotPlugCallback) {
        NEXUS_IsrCallback_Destroy(hdmiInput->hotPlugCallback);
        hdmiInput->hotPlugCallback = NULL;
    }

    NEXUS_HdmiInput_UnloadEdidData(hdmiInput) ;

    g_NEXUS_hdmiInput.handle[hdmiInput->index] = NULL;

    BDBG_OBJECT_DESTROY(hdmiInput, NEXUS_HdmiInput);
    BKNI_Free(hdmiInput);
}

static void NEXUS_HdmiInput_P_Release(NEXUS_HdmiInputHandle hdmiInput)
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdmiInput, hdmiInput);
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoInput, &hdmiInput->videoInput, Close);
#if NEXUS_HAS_AUDIO
    NEXUS_OBJECT_UNREGISTER(NEXUS_AudioInput, &hdmiInput->audioInput, Close);
#endif
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_HdmiInput, NEXUS_HdmiInput_Close);


void  Nexus_HdmiInput_P_SetHdmiVideoFormat_isr(
	    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat hdmiVideoFormat)
{
    g_NEXUS_hdmiInput.hdmiVideoFormat = hdmiVideoFormat;
}
