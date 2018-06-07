/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#ifndef NEXUS_HDMI_OUTPUT_DBV_H__
#define NEXUS_HDMI_OUTPUT_DBV_H__

#include "nexus_hdmi_output.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
The specification that the colorimetry of the Dolby Vision HDMI sink is closest to
****************************************************************************/
typedef enum NEXUS_DolbyVisionColorimetry
{
    NEXUS_DolbyVisionColorimetry_eUnknown, /* not specified in the VSVDB */
    NEXUS_DolbyVisionColorimetry_eRec709, /* same meaning as NEXUS_MatrixCoefficients_eItu_R_BT_709*/
    NEXUS_DolbyVisionColorimetry_eDciP3, /* defined by film industry; between 709 and 2020 */
    NEXUS_DolbyVisionColorimetry_eMax
} NEXUS_DolbyVisionColorimetry;

/***************************************************************************
Summary:
Dolby Vision EDID Data
Description:
Contains some of the data present in the Dolby Vision proprietary VSVDB.
Access to this information requires a Dolby Vision license from Dolby.
A value of Auto/NotSet for any TristateEnable field here means that
the VSVDB did not contain this information and the client should look
into the normal EDID data blocks to find out what is supported by the
connected receiver.
****************************************************************************/
typedef struct NEXUS_HdmiOutputDbvEdidData
{
    bool valid;
    NEXUS_TristateEnable supports2160p60; /* also applies to 2160p50 */
    struct
    {
        NEXUS_TristateEnable supports10Bit;
        NEXUS_TristateEnable supports12Bit;
    } colorDepthSupport[NEXUS_ColorSpace_eMax];
    NEXUS_MasteringDisplayColorVolume masteringDisplayColorVolume; /* uses 1 nits convention for max luminance */
    NEXUS_DolbyVisionColorimetry colorimetry;
} NEXUS_HdmiOutputDbvEdidData;

/***************************************************************************
Summary:
Returns information from the Dolby Vision VSVDB that is useful to the application
****************************************************************************/
void NEXUS_HdmiOutput_GetDbvEdidData(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_HdmiOutputDbvEdidData * pData /* [out] */
);

#ifdef __cplusplus
}
#endif

#endif
