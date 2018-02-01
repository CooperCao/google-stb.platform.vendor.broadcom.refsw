/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef DRM_HDCP_TX_H_
#define DRM_HDCP_TX_H_

#include "drm_common_priv.h"
#include "drm_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif

/* HDCP-TX parameter settings structure */
typedef struct DrmHdcpTxSettings_t
{
    char * drm_bin_file_path;
    DrmCommonInit_t drmCommonInit;
}DrmHdcpTxSettings_t;

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpTx_GetDefaultParamSettings
 **
 ** DESCRIPTION:
 **   Retrieve the default settings
 **
 ** PARAMETERS:
 **   pHdcpTxParamSettings - pointer to settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_HdcpTx_GetDefaultParamSettings(
    DrmHdcpTxSettings_t *pHdcpTxSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpTx_Initialize
 **
 ** DESCRIPTION:
 **   Reads the bin file specified and pre-loads the confidential info.
 **   Must be called only once prior to any other module API call.
 **
 ** PARAMETERS:
 **   pHdcpTxSettings
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_HdcpTx_Initialize(
    DrmHdcpTxSettings_t *pHdcpTxSettings);

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpTx_Finalize
 **
 ** DESCRIPTION:
 **   Close the HdcpTx module
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_HdcpTx_Finalize(void);

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpTx_GetData
 **
 ** DESCRIPTION:
 **   Extract data from HdcpTx credential
 **
 ** PARAMETERS:
 **   pHdcpTxData[out] structure containing HdcpTx data from DRM key region
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_HdcpTx_GetData(drm_hdcp_tx_data_t *pHdcpTxData);

#ifdef __cplusplus
}
#endif

#endif /* DRM_HDCP_TX_H_ */
