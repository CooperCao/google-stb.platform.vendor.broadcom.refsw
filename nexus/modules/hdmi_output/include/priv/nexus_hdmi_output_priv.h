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
#ifndef NEXUS_HDMI_OUTPUT_PRIV_H__
#define NEXUS_HDMI_OUTPUT_PRIV_H__

#include "nexus_hdmi_output.h"
#include "bavc.h"
#include "bfmt.h"

#if NEXUS_HAS_SECURITY
#include "bhdcplib.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef struct NEXUS_HdmiOutputVideoSettings
{
    NEXUS_VideoFormat videoFormat;
    NEXUS_ColorSpace colorSpace;
    unsigned colorDepth;
} NEXUS_HdmiOutputVideoSettings;


NEXUS_Error NEXUS_HdmiOutput_ValidateVideoSettings_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputVideoSettings *requested,
    NEXUS_HdmiOutputVideoSettings *preferred /* [out] */
) ;

void NEXUS_HdmiOutput_SelectVideoSettingsPriorityTable_priv(
	void
)  ;

/* VDC -> HDM callback info for rate changes */
void NEXUS_HdmiOutput_VideoRateChange_isr(
    NEXUS_HdmiOutputHandle handle,
    BAVC_VdcDisplay_Info *pDisplayInfo
    );

/* RAP -> HDM callback for sample rate changes */
void NEXUS_HdmiOutput_AudioSampleRateChange_isr(
    NEXUS_HdmiOutputHandle handle,
    BAVC_AudioSamplingRate sampleRate
    );

typedef struct NEXUS_HdmiOutputDisplaySettings
{
    bool overrideMatrixCoefficients ;
    NEXUS_MatrixCoefficients eColorimetry ;
    NEXUS_ColorSpace colorSpace ;
    bool overrideColorRange ;
    NEXUS_ColorRange colorRange ;
    unsigned colorDepth ;
} NEXUS_HdmiOutputDisplaySettings ;


typedef struct NEXUS_HdmiOutputColorimetryParameters
{
    NEXUS_VideoFormat format ;
    bool xvYccEnabled ;
} NEXUS_HdmiOutputColorimetryParameters ;


/* Returns false if the format is not supported, true if it is */
bool NEXUS_HdmiOutput_GetColorimetry_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    const NEXUS_HdmiOutputColorimetryParameters *parameters,
    BAVC_MatrixCoefficients *pColorimetry
    );

bool NEXUS_HdmiOutput_GetEotf_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_VideoEotf *pEotf
	);

NEXUS_Error NEXUS_HdmiOutput_GetDisplaySettings_priv(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputDisplaySettings *stDisplaySettings
    );

NEXUS_Error NEXUS_HdmiOutput_SetDisplaySettings_priv(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputDisplaySettings *stDisplaySettings
    );

/* Connect video and set format parameters */
NEXUS_Error NEXUS_HdmiOutput_SetDisplayParams_priv(
    NEXUS_HdmiOutputHandle handle,
    BFMT_VideoFmt format,
    BAVC_MatrixCoefficients colorimetry,
    BFMT_AspectRatio aspectRatio,
    bool masterMode,
    const NEXUS_CallbackDesc *notifyDisplay
    );

/* Disconnect video */
NEXUS_Error NEXUS_HdmiOutput_Disconnect_priv(
    NEXUS_HdmiOutputHandle handle
    );

/* Audio can only start if video is connected */
bool NEXUS_HdmiOutput_VideoIsConnected_priv(
    NEXUS_HdmiOutputHandle handle
    );

/* Sanity checks for the audio module */
bool NEXUS_HdmiOutput_AudioFormatSupported_priv(
    NEXUS_HdmiOutputHandle handle,
    BAVC_AudioFormat format,
    unsigned numChannels    /* PCM only */
    );

/* Set audio format information */
NEXUS_Error NEXUS_HdmiOutput_SetAudioParams_priv(
    NEXUS_HdmiOutputHandle handle,
    BAVC_AudioBits bitsPerSample,
    BAVC_AudioSamplingRate sampleRate,
    BAVC_AudioFormat format,
    unsigned numChannels    /* PCM only */
    );

/**
Summary:
Set the notifyAudio event
**/
void NEXUS_HdmiOutput_SetNotifyAudioEvent_priv(
    NEXUS_HdmiOutputHandle handle,
    BKNI_EventHandle notifyAudioEvent
    );

    /* called before Display NEXUS_VideoFormat change */
NEXUS_Error NEXUS_HdmiOutput_P_PreFormatChange_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    bool aspectRatioChangeOnly
    );

/* called after Display NEXUS_VideoFormat change */
NEXUS_Error NEXUS_HdmiOutput_P_PostFormatChange_priv(
    NEXUS_HdmiOutputHandle hdmiOutput
    );


#if BAVC_HDMI_1_3_SUPPORT

#define NEXUS_HAS_HDMI_1_3 1

#endif

void NEXUS_HdmiOutput_P_Vsync_isr(
    void *handle /* NEXUS_HdmiOutputHandle */
    );


void NEXUS_HdmiOutput_ReadFormatChangeStatus_priv(
    NEXUS_HdmiOutputHandle handle,
    bool *pFormatChangeUpdate
    );

void NEXUS_HdmiOutput_RestorePrevSettings_priv(
	NEXUS_HdmiOutputHandle hdmiOutput
	) ;


NEXUS_Error NEXUS_HdmiOutput_P_SetTmdsSignalData(
    NEXUS_HdmiOutputHandle handle, bool tmdsDataEnable);

NEXUS_Error NEXUS_HdmiOutput_P_SetTmdsSignalClock(
    NEXUS_HdmiOutputHandle handle, bool tmdsClockEnable);




#if NEXUS_HAS_SECURITY
BHDCPlib_State NEXUS_HdmiOutput_P_GetCurrentHdcplibState(
	NEXUS_HdmiOutputHandle hdmiOutput);
#endif

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_HdmiOutput);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HDMI_OUTPUT_PRIV_H__ */

