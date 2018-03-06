/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 ******************************************************************************/
#include "nexus_base.h"
#include "nexus_video_types.h"
#include "nexus_hdmi_output_thunks.h"
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#include "nexus_hdmi_output_init.h"
#include "priv/nexus_hdmi_output_standby_priv.h"
#include "nexus_hdmi_output_extra.h"
#include "bhdm.h"
#include "bhdm_edid.h"
#include "nexus_platform_features.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_core_audio.h"
#include "priv/nexus_core_img_id.h"
#include "priv/nexus_core_img.h"
#include "priv/nexus_hdmi_output_priv.h"
#if NEXUS_DBV_SUPPORT
#include "nexus_hdmi_output_dbv_impl.h"
#endif

#if NEXUS_HAS_SAGE && NEXUS_HAS_HDCP_2X_SUPPORT
#include "bchp_common.h"
#ifndef BCHP_HDCP2_TX_HAE_INTR2_0_REG_START
#undef NEXUS_HAS_HDCP_2X_SUPPORT
#endif
#endif

#include "bhdm_hdcp.h"
#if NEXUS_HAS_SECURITY
#include "bhdcplib.h"
#include "bhdcplib_keyloader.h"
#endif

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
#include "bsagelib_client.h"
#include "bsagelib_management.h"
#endif

#ifndef NEXUS_HDMI_OUTPUT_MODULE_H__
#define NEXUS_HDMI_OUTPUT_MODULE_H__

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME hdmi_output
#define NEXUS_MODULE_SELF g_NEXUS_hdmiOutputModule

/* Global Types */

typedef enum NEXUS_HdmiOutputState
{
    NEXUS_HdmiOutputState_eNone,
    NEXUS_HdmiOutputState_eDisconnected,
    NEXUS_HdmiOutputState_eRxSenseCheck,

    NEXUS_HdmiOutputState_ePoweredDown,

    NEXUS_HdmiOutputState_ePoweredOn,
    NEXUS_HdmiOutputState_eMax
} NEXUS_HdmiOutputState;


typedef enum NEXUS_HdmiOutputLogicalAddrSearch
{
    NEXUS_HdmiOutputLogicalAddrSearch_eInit,
    NEXUS_HdmiOutputLogicalAddrSearch_eNext,
    NEXUS_HdmiOutputLogicalAddrSearch_eReady
} NEXUS_HdmiOutputLogicalAddrSearch;


typedef struct NEXUS_HdmiOutput
{
    NEXUS_OBJECT(NEXUS_HdmiOutput);
    bool opened;
    bool videoConnected;
    bool tmdsClockEnabled ;
    bool tmdsDataEnabled ;

#define RESOLVE_ALIAS(handle) do {(handle) = ((handle)->alias.master?(handle)->alias.master:(handle));}while(0)
#define IS_ALIAS(handle) ((handle)->alias.master != NULL)
    struct {
        NEXUS_HdmiOutputHandle master;
    } alias;

    /* isr context */
    bool forceDisconnect_isr ;
    NEXUS_HdmiOutputState lastHotplugState_isr ;

    NEXUS_HdmiOutputState lastState;
    NEXUS_HdmiOutputState lastCallbackState;

    NEXUS_HdmiOutputState rxState;
    NEXUS_HdmiOutputState lastRxState;

    NEXUS_VideoOutputObject videoConnector;
    NEXUS_AudioOutputObject audioConnector;
    BHDM_Handle hdmHandle;
    NEXUS_HdmiOutputOpenSettings openSettings;
    NEXUS_HdmiOutputSettings settings;
    NEXUS_HdmiOutputDisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings previousSettings;
    NEXUS_HdmiOutputExtraSettings extraSettings;
    NEXUS_EventCallbackHandle hotplugEventCallback;
    NEXUS_EventCallbackHandle mhlStandbyEventCallback;
    NEXUS_EventCallbackHandle rxSenseEventCallback;
    NEXUS_EventCallbackHandle scrambleEventCallback;
    NEXUS_EventCallbackHandle avRateChangeEventCallback;
    NEXUS_TimerHandle powerTimer;
    BKNI_EventHandle cecHotplugEvent;

    bool pendingDisableAuthentication_isr;
    bool forceSendRxIdList;
    bool hdcpRequiredPostFormatChange;
    bool formatChangeMute;
    bool avMuteSetting;
    bool hdcpStarted;
    bool formatChangeUpdate;
    uint8_t retryScrambleCount ;
    uint32_t pixelClkRatePreFormatChange;

    NEXUS_TimerHandle connectTimer;
    uint8_t checkRxSenseCount ;
    uint8_t lastReceiverSense ;

    bool contentChangeOnly;

    BAVC_AudioSamplingRate sampleRate;
    BAVC_AudioBits audioBits;
    BAVC_AudioFormat audioFormat;
    unsigned audioNumChannels;    /* PCM only */

    NEXUS_HdmiOutputLogicalAddrSearch searchState;
    unsigned logAddrSearchIndex;

    NEXUS_TaskCallbackHandle hdcpFailureCallback;
    NEXUS_TaskCallbackHandle hdcpStateChangedCallback;
    NEXUS_TaskCallbackHandle hdcpSuccessCallback;
    NEXUS_TaskCallbackHandle hotplugCallback;
    NEXUS_TaskCallbackHandle mhlStandbyCallback;
    NEXUS_TaskCallbackHandle rxStatusCallback;
    NEXUS_TaskCallbackHandle notifyDisplay;
    BKNI_EventHandle notifyAudioEvent;
    BKNI_EventHandle notifyHotplugEvent ;

    BHDM_HDCP_Version eHdcpVersion;
    NEXUS_HdmiOutputHdcpVersion hdcpVersionSelect;
#if NEXUS_HAS_SECURITY
    BHDCPlib_Handle hdcpHandle;
    NEXUS_EventCallbackHandle hdcpHotplugCallback;
    NEXUS_EventCallbackHandle riCallback;
    NEXUS_EventCallbackHandle pjCallback;
    NEXUS_TimerHandle hdcpTimer;
    NEXUS_TimerHandle hdcpKeepAliveTimer;
    NEXUS_HdmiOutputHdcpSettings hdcpSettings;
    NEXUS_HdmiOutputHdcpKsv *pRevokedKsvs;
    uint16_t numRevokedKsvs;
    uint16_t revokedKsvsSize;
    BHDCPlib_State hdcp1xState;
    BHDCPlib_HdcpError hdcp1xError;
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    NEXUS_HdmiInputHandle hdmiInput;
    NEXUS_EventCallbackHandle hdcp2xReAuthRequestCallback;
    NEXUS_EventCallbackHandle hdcp2xAuthenticationStatusCallback;
    struct {
        uint8_t *buffer;
        uint32_t bufferSize;
        uint32_t length;
    } hdcp2xKeys;
#endif
#endif
    NEXUS_StandbyMode standbyMode ;
    BHDM_Settings hdmSettings;
    bool edidProcDebugDisplayed ;

    bool invalidEdid ;
    bool invalidEdidReported ;
    bool edidHdmiDevice ;
    BHDM_EDID_RxVendorSpecificDB edidVendorSpecificDB ;

    NEXUS_HdmiOutputTxHardwareStatus txHwStatus ;
    NEXUS_HdmiOutputRxHardwareStatus rxHwStatus ;
    unsigned phyChangeRequestCounter ;

    /* HDCP Stats */
    struct {
        struct {
            struct {
                unsigned attemptCounter;
                unsigned passCounter;
                unsigned failCounter;
            } auth;
            unsigned bCapsReadFailureCounter;  /* i2c Read of Rx BCaps register */
            unsigned bksvReadFailureCounter;
            unsigned invalidBksvCounter;    /* unable to read Rx Bksv or read invalid/test Bksv */
        } hdcp1x;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
        struct {
            struct {
                unsigned attemptCounter;
                unsigned passCounter;
                unsigned failCounter;
            } auth;
            unsigned ReceiverIdReadError;

            unsigned validReauthReqCounter;   /* ReAuth requests when HDCP is enabled */
            unsigned invalidReauthReqCounter; /* ReAuth requests when HDCP is disabled (should not happen) */
            unsigned watchdogCounter;  /* reset of Sage */
            unsigned timeoutCounter;  /* HDCP auth step has timed out */
        } hdcp22;
#endif
    } hdcpMonitor;

    struct {
        NEXUS_HdmiOutputCrcData *queue;
        unsigned size; /* num entries, not num bytes */
        unsigned rptr, wptr;
    } crc;

    char audioConnectorName[14];   /* HDMI OUTPUT %d */

    struct
    {
        NEXUS_HdmiDynamicRangeMasteringInfoFrame inputInfoFrame;
        NEXUS_HdmiDynamicRangeMasteringInfoFrame outputInfoFrame;
        BHDM_EDID_HDRStaticDB hdrdb ; /* last one, compared to current one */
        bool connected; /* last one */
        bool printDrmInfoFrameChanges;
        NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities processingCaps;
        NEXUS_TimerHandle offTimer;
    } drm;

    NEXUS_HdmiVendorSpecificInfoFrame vsif;
    NEXUS_HdmiAviInfoFrame avif;

    uint16_t supported3DFormats[BFMT_VideoFmt_eMaxCount];
    BHDM_EDID_AudioDescriptor supportedAudioFormats[BAVC_AudioCompressionStd_eMax];

#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutputDbvData dbv;
#endif
} NEXUS_HdmiOutput;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
typedef struct NEXUS_HdmiOutputMemoryBlock {
    size_t len;
    void *buf;
} NEXUS_HdmiOutputMemoryBlock;

#define NEXUS_HDMI_OUTPUT_SAGE_INDICATION_QUEUE_SIZE 5
typedef struct NEXUS_HdmiOutputIndicationData
{
    BHDCPlib_SageIndicationData sageIndication;
    BHDCPlib_Handle hHDCPlib;
} NEXUS_HdmiOutputIndicationData;


typedef struct NEXUS_HdmiOutput_SageData
{
    BSAGElib_ClientHandle sagelibClientHandle;
    BKNI_EventHandle eventWatchdogRecv;
    NEXUS_EventCallbackHandle eventWatchdogRecvCallback;
    BKNI_EventHandle eventTATerminated;
    NEXUS_EventCallbackHandle eventTATerminatedCallback;
    BKNI_EventHandle eventResponseRecv;
    BKNI_EventHandle eventIndicationRecv;
    NEXUS_EventCallbackHandle eventIndicationRecvCallback;
    uint32_t async_id;
    NEXUS_HdmiOutputIndicationData indicationData[NEXUS_HDMI_OUTPUT_SAGE_INDICATION_QUEUE_SIZE];
    unsigned indicationReadPtr;
    unsigned indicationWritePtr;
} NEXUS_HdmiOutput_SageData;
#endif

/* Global module handle & data */
extern NEXUS_ModuleHandle g_NEXUS_hdmiOutputModule;
extern NEXUS_HdmiOutputModuleSettings g_NEXUS_hdmiOutputModuleSettings;
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
extern NEXUS_HdmiOutput_SageData g_NEXUS_hdmiOutputSageData;
extern NEXUS_HdmiOutputMemoryBlock g_hdcpTABlock;
#endif

/* Internal Private Routines */
NEXUS_HdmiOutputState NEXUS_HdmiOutput_P_GetState(NEXUS_HdmiOutputHandle output);

NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp(NEXUS_HdmiOutputHandle output);
NEXUS_Error NEXUS_HdmiOutput_P_UninitHdcp(NEXUS_HdmiOutputHandle output);
void NEXUS_HdmiOutput_P_HdcpNotifyHotplug(NEXUS_HdmiOutputHandle output);

void NEXUS_HdmiOutputModule_Print(void);

void NEXUS_HdmiOutput_P_CheckHdcpVersion(NEXUS_HdmiOutputHandle output);

void NEXUS_HdmiOutput_P_CloseHdcp(NEXUS_HdmiOutputHandle output);

const char * NEXUS_HdmiOutput_P_ColorSpace_ToText(NEXUS_ColorSpace colorSpace);

/* Proxy conversion */
#define NEXUS_P_HDMI_OUTPUT_HDCP_KSV_SIZE(num) ((num)*sizeof(NEXUS_HdmiOutputHdcpKsv))

NEXUS_Error NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(NEXUS_HdmiOutputHandle output); /* call from SetExtendedSettings */
void NEXUS_HdmiOutput_P_DrmInfoFrameConnectionChanged(NEXUS_HdmiOutputHandle output); /* call from hotplug */
NEXUS_Error NEXUS_HdmiOutput_P_SetVideoSettings(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputDisplaySettings *pstDisplaySettings
);

NEXUS_Error NEXUS_HdmiOutput_P_EnableDisplay(NEXUS_HdmiOutputHandle hdmiOutput, const BHDM_Settings * pSettings);
NEXUS_Error NEXUS_HdmiOutput_P_ApplyAviInfoFrame(NEXUS_HdmiOutputHandle handle);

#if NEXUS_DBV_SUPPORT
void NEXUS_HdmiOutput_P_DbvConnectionChanged(NEXUS_HdmiOutputHandle output);
NEXUS_Error NEXUS_HdmiOutput_P_SetDbvMode(NEXUS_HdmiOutputHandle output);
NEXUS_Error NEXUS_HdmiOutput_P_SetDolbyVisionVendorSpecificInfoFrame(NEXUS_HdmiOutputHandle handle);
void NEXUS_HdmiOutput_P_SetDolbyVisionAviInfoFrame(NEXUS_HdmiOutputHandle handle, BAVC_HDMI_AviInfoFrame * pAVIIF);
void NEXUS_HdmiOutput_P_GetDolbyVisionAviInfoFrame(NEXUS_HdmiOutputHandle handle, BAVC_HDMI_AviInfoFrame * pAVIIF);
void NEXUS_HdmiOutput_P_DbvUpdateDrmInfoFrame(NEXUS_HdmiOutputHandle output, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInfoFrame);
void NEXUS_HdmiOutput_P_DbvUpdateDisplaySettings(NEXUS_HdmiOutputHandle output, NEXUS_HdmiOutputDisplaySettings * pDisplaySettings);
#endif
#endif /* #ifndef NEXUS_HDMI_OUTPUT_MODULE_H__ */


