/***************************************************************************
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
 **************************************************************************/
#ifndef NEXUS_HDMI_INPUT_MODULE_H__
#define NEXUS_HDMI_INPUT_MODULE_H__

#include "nexus_hdmi_input_thunks.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_init.h"
#include "nexus_hdmi_input_ext.h"
#include "nexus_hdmi_input_hdcp.h"
#include "bhdr.h"
#include "bhdr_fe.h"
#include "bhdr_hdcp.h"
#if NEXUS_POWER_MANAGEMENT
#include "bhdr_pwr.h"
#endif

#include "priv/nexus_core.h"
#include "priv/nexus_core_video.h"
#include "priv/nexus_hdmi_input_priv.h"
#if NEXUS_HAS_AUDIO
#include "priv/nexus_core_audio.h"
#endif

/* HDCP 2.x defaults on where available. 3 things determine it:
1) HW capability - checked via bchp_common.h RDB test
2) SAGE support - can be turned off with SAGE_SUPPORT=n. tested with #if NEXUS_HAS_SAGE.
3) user flag - can be turned off with NEXUS_HDCP_SUPPORT=n. tested with #if NEXUS_HAS_HDCP_2X_SUPPORT.
*/
#if NEXUS_HAS_SAGE && NEXUS_HAS_HDCP_2X_SUPPORT
#include "bchp_common.h"
#ifdef BCHP_HDCP2_RX_HAE_INTR2_0_REG_START
#define NEXUS_HAS_HDCP_2X_RX_SUPPORT 1
#endif
#endif


#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
#include "bhdcplib.h"
#include "bhdcplib_hdcp2x.h"
#include "bsagelib_client.h"
#include "bsagelib_management.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME hdmiInput
#define NEXUS_MODULE_SELF g_NEXUS_hdmiInputModule

/* global handle. there is no global data. */
extern NEXUS_ModuleHandle g_NEXUS_hdmiInputModule;


BDBG_OBJECT_ID_DECLARE(NEXUS_HdmiInput);

struct NEXUS_HdmiInput {
    NEXUS_OBJECT(NEXUS_HdmiInput);
    BHDR_FE_ChannelHandle frontend; /* current connection */

    NEXUS_IsrCallbackHandle hotPlugCallback;
    NEXUS_HdmiInputHandle masterHdmiInput;

    bool videoConnected, audioConnected;
    NEXUS_HdmiInputSettings settings;
    NEXUS_HdmiInputHdcpSettings hdcpSettings ;

    BAVC_Colorspace manualColorSpace; /* cached mapping of color space (to avoid isr-time conversion) */
    BAVC_Colorspace reportedColorSpace; /* color space from the receiver */
    NEXUS_VideoInputObject videoInput;
#if NEXUS_HAS_AUDIO
    NEXUS_AudioInputObject audioInput;
#endif
    uint8_t index;
    BHDR_Handle hdr;

    NEXUS_HdmiInputHdcpStatus stHdcpStatus ;
    NEXUS_HdmiInputHdcpKeyset	hdcpKeyset ;

#if BHDR_CONFIG_EDID_RAM
    BHDR_EDID_Info stEdidInfo ;
#endif

    BKNI_EventHandle frameRateEvent ;
    NEXUS_EventCallbackHandle frameRateHandler ;

    BKNI_EventHandle hdrPacketEvent ;

    NEXUS_IsrCallbackHandle avMuteCallback;
    NEXUS_IsrCallbackHandle aviInfoFrameChanged;
    NEXUS_IsrCallbackHandle audioInfoFrameChanged;
    NEXUS_IsrCallbackHandle spdInfoFrameChanged;
    NEXUS_IsrCallbackHandle vendorSpecificInfoFrameChanged;
    NEXUS_IsrCallbackHandle drmInfoFrameChanged;
    NEXUS_IsrCallbackHandle audioContentProtectionChanged;
    NEXUS_IsrCallbackHandle gamutMetadataPacketChanged;
    NEXUS_IsrCallbackHandle hdcpKeysetStatusUpdate ;

    NEXUS_IsrCallbackHandle hdcpRxChanged ;
    NEXUS_TaskCallbackHandle sourceChangedCallback;

    NEXUS_TimerHandle releaseHotPlugTimer;

    /* status */
    bool bInitialized;
    bool bSentResetHdDviBegin;
    bool avMute;
    uint8_t uiFormatChangeMuteCount ;
    BAVC_VDC_HdDvi_Picture stFieldData;
    NEXUS_VideoFrameRate frameRate;
    NEXUS_HdmiInputStatus vdcStatus;
#if BDBG_DEBUG_BUILD
    NEXUS_HdmiInputStatus previousVdcStatus;
#endif
    NEXUS_HdmiInputFormatChangeCb pPcFormatCallback_isr;
    void *pPcFormatCallbackParam;
    char audioConnectorName[11];   /* HDMI INPUT */

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
    NEXUS_EventCallbackHandle hdcp2xAuthenticationStatusCallback;

    BHDCPlib_Handle hdcpHandle;
    BHDCPlib_State hdcpState;
    BHDCPlib_HdcpError hdcpError;
#endif

} ;

/* HdmiInput global state */
typedef struct NEXUS_gHdmiInput {
    NEXUS_HdmiInputModuleSettings settings;
    BHDR_FE_Handle fe;
    bool initInProgress;
    NEXUS_HdmiInputHandle handle[NEXUS_NUM_HDMI_INPUTS] ;
    NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat hdmiVideoFormat ;
} NEXUS_gHdmiInput ;


BDBG_OBJECT_ID_DECLARE(NEXUS_HdmiInputFrontend);

struct NEXUS_HdmiInputFrontend
{
    BDBG_OBJECT(NEXUS_HdmiInputFrontend)
    unsigned index;
    BHDR_FE_ChannelHandle channel;
    NEXUS_IsrCallbackHandle hotPlugCallback;
    NEXUS_HdmiInputHandle attachment;
};

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_RX_SUPPORT)
typedef struct NEXUS_HdmiInputMemoryBlock {
    size_t len;
    void *buf;
} NEXUS_HdmiInputMemoryBlock;

#define NEXUS_HDMI_INPUT_SAGE_INDICATION_QUEUE_SIZE 5
typedef struct NEXUS_HdmiInputIndicationData
{
    BHDCPlib_SageIndicationData sageIndication;
    BHDCPlib_Handle hHDCPlib;
} NEXUS_HdmiInputIndicationData;

typedef struct NEXUS_HdmiInput_SageData
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
	NEXUS_HdmiInputIndicationData indicationData[NEXUS_HDMI_INPUT_SAGE_INDICATION_QUEUE_SIZE];
	unsigned indicationReadPtr;
	unsigned indicationWritePtr;
} NEXUS_HdmiInput_SageData;

extern NEXUS_HdmiInput_SageData g_NEXUS_hdmiInputSageData;
extern NEXUS_HdmiInputMemoryBlock g_hdmiInputTABlock;
extern NEXUS_HdmiInputMemoryBlock g_hdmiInputFCBlock;

#endif


void NEXUS_HdmiInput_P_VideoFormatChange_isr(void *context, int param2, void *data);
void NEXUS_HdmiInput_P_PacketChange_isr(void *context, int param2, void *data);
void NEXUS_HdmiInput_P_AvMuteNotify_isr(void *context, int param2, void *data);

void NEXUS_HdmiInput_P_HotPlug_isr(void *context, int param, void *data);

void NEXUS_HdmiInput_P_SetFrameRate(void *data);

void Nexus_HdmiInput_P_SetHdmiVideoFormat_isr(NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat hdmiVideoFormat);

void NEXUS_HdmiInput_PrintAviInfoFramePacket(void);
void NEXUS_HdmiInput_PrintAudioInfoFramePacket(void);
void NEXUS_HdmiInput_PrintVendorSpecificInfoFramePacket(void);
void NEXUS_HdmiInput_PrintDrmInfoFramePacket(void);
void NEXUS_HdmiInput_PrintACRPacket(void);

void NEXUS_HdmiInputModule_Print(void);


/* Proxy conversion */
#define NEXUS_P_HDMI_INPUT_HDCP_KSV_SIZE(num) ((num)*sizeof(NEXUS_HdmiOutputHdcpKsv))

#ifdef __cplusplus
}
#endif

#endif
