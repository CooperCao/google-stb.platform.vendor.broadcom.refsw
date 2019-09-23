/******************************************************************************
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
 *****************************************************************************/
#ifndef NEXUS_HDMI_OUTPUT_DYNRNG_IMPL_H
#define NEXUS_HDMI_OUTPUT_DYNRNG_IMPL_H

#include "nexus_hdmi_types.h"
#include "bavc_hdmi.h"
#include "nexus_hdmi_output_drmif_impl.h"
#if NEXUS_DBV_SUPPORT
#include "nexus_hdmi_output_dbv_impl.h"
#endif
#if NEXUS_HDR10PLUS_SUPPORT
#include "nexus_hdmi_output_hdr10plus_impl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_HdmiOutputDynrngData
{
    bool connected; /* last one */
    bool printDynrngChanges;
    NEXUS_VideoDynamicRangeMode inputMode;
    NEXUS_VideoDynamicRangeMode outputMode;
    NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities processingCaps;
    NEXUS_HdmiOutputDrmifData drmif;
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutputDbvData dbv;
#endif
#if NEXUS_HDR10PLUS_SUPPORT
    NEXUS_HdmiOutputHdr10PlusData hdr10Plus;
#endif
} NEXUS_HdmiOutputDynrngData;

void NEXUS_HdmiOutput_Dynrng_P_Init(NEXUS_HdmiOutputHandle hdmiOutput);
void NEXUS_HdmiOutput_Dynrng_P_InitStatus(NEXUS_HdmiOutputHandle output); /* called from InitExtraStatus */
void NEXUS_HdmiOutput_Dynrng_P_ConnectionChanged(NEXUS_HdmiOutputHandle hdmiOutput);
NEXUS_VideoEotf NEXUS_HdmiOutput_Dynrng_P_GetOutputEotf(NEXUS_HdmiOutputHandle hdmiOutput);
void NEXUS_HdmiOutput_Dynrng_P_UpdateVendorSpecificInfoFrame(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_VendorSpecificInfoFrame * pAvcInfoFrame);
void NEXUS_HdmiOutput_Dynrng_P_UpdateAviInfoFrameSettings(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_AviInfoFrame * pAVIIF);
void NEXUS_HdmiOutput_Dynrng_P_UpdateAviInfoFrameStatus(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_AviInfoFrame * pAVIIF);
NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_ReapplyVsif(NEXUS_HdmiOutputHandle hdmiOutput);
void NEXUS_HdmiOutput_Dynrng_P_ResolveMode(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiOutputDisplaySettings * pDisplaySettings);
NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_SetMode(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_VideoDynamicRangeMode dynamicRangeMode);
void NEXUS_HdmiOutput_Dynrng_P_NotifyDisplay(NEXUS_HdmiOutputHandle hdmiOutput);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_HDMI_OUTPUT_DYNRNG_IMPL_H */
