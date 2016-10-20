/***************************************************************************
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
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_HDMI_INPUT_PRIV_H__
#define NEXUS_HDMI_INPUT_PRIV_H__


#include "nexus_hdmi_input.h"
#include "nexus_hdmi_input_init.h"
#include "bavc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Called by Display
**/
void NEXUS_HdmiInput_PictureCallback_isr(
    NEXUS_HdmiInputHandle hdmiInput,
    BAVC_VDC_HdDvi_Picture **ppPicture /* in/out param */
    );

void NEXUS_HdmiInput_GetSourceId_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    BAVC_SourceId *id /* [out] */
    );

void NEXUS_HdmiInput_SetFrameRate_isr(
    NEXUS_HdmiInputHandle hdmiInput,
    BAVC_FrameRateCode frameRate
    );

/* Display module informs HdmiInput that its connection to a VideoWindow has changed. */
void NEXUS_HdmiInput_VideoConnected_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    bool connected
    );

/**
Called by Audio
**/
void NEXUS_HdmiInput_GetIndex_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    unsigned *pIndex /* [out] */
    );

/**
Pass HdmiInput status which can only be obtained by the Display module
**/
void NEXUS_HdmiInput_SetStatus_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    const NEXUS_HdmiInputStatus *pStatus
    );

/**
Pass HdmiInput status which can only be obtained by the Display module
**/
typedef void (*NEXUS_HdmiInputFormatChangeCb)(void *pParam);
void NEXUS_HdmiInput_SetFormatChangeCb_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiInputFormatChangeCb pFunction_isr,
    void *pFuncParam
    );


void NEXUS_HdmiInput_SetHdrEvent_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    BKNI_EventHandle notifyHdrPacketEvent
) ;

/**
Summary:
**/
NEXUS_Error NEXUS_HdmiInput_GetDrmInfoFrameData_priv(
    NEXUS_HdmiInputHandle hdmiInput,
    NEXUS_HdmiDynamicRangeMasteringInfoFrame *pDrmInfoFrame /* [out] */
    );


/**
Called by Hdmi_Output
**/
NEXUS_Error NEXUS_HdmiInput_LoadHdcp2xReceiverIdList_priv(
	NEXUS_HdmiInputHandle hdmiInput,
	NEXUS_Hdcp2xReceiverIdListData *pData,
	bool downstreamIsRepeater
	);

NEXUS_Error NEXUS_HdmiInput_UpdateHdcp2xRxCaps_priv(
	NEXUS_HdmiInputHandle hdmiInput,
	bool downstreamDeviceAttached
	);

NEXUS_Error NEXUS_HdmiInput_LoadHdcpTA_priv(
    void *buf, size_t length
	);


NEXUS_OBJECT_CLASS_DECLARE(NEXUS_HdmiInput);

#ifdef __cplusplus
}
#endif

#endif
