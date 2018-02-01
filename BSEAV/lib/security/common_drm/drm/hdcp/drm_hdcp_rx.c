/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "drm_hdcp_rx.h"

#include "drm_key_region.h"
#include "drm_common.h"
#include "drm_common_priv.h"

#include "drm_common_swcrypto.h"
#include "drm_data.h"

BDBG_MODULE(drm_hdcp_rx);

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpRx_GetDefaultParamSettings
 **
 ** DESCRIPTION:
 **   Retrieve the default settings
 **
 ** PARAMETERS:
 **   pHdcpRxSettings - pointer to settings structure
 **
 ** RETURNS:
 **   void
 **
 ******************************************************************************/
void DRM_HdcpRx_GetDefaultParamSettings(
    DrmHdcpRxSettings_t *pHdcpRxSettings)
{
    BDBG_MSG(("%s - Entered function",BSTD_FUNCTION));

    BKNI_Memset((uint8_t*)pHdcpRxSettings, 0x00, sizeof(DrmHdcpRxSettings_t));

    pHdcpRxSettings->drm_bin_file_path = bdrm_get_drm_bin_file_path();

    BDBG_MSG(("%s - Exiting function",BSTD_FUNCTION));
    return;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpRx_Initialize
 **
 ** DESCRIPTION:
 **   Reads the bin file specified and pre-loads the confidential info.
 **   Must be called only once prior to any other module API call.
 **
 ** PARAMETERS:
 **   pHdcpRxSettings
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_HdcpRx_Initialize(DrmHdcpRxSettings_t *pHdcpRxSettings)
{
    DrmRC rc = Drm_Success;

    BDBG_MSG(("%s - Entering...", BSTD_FUNCTION));

    if(pHdcpRxSettings == NULL)
    {
        BDBG_ERR(("%s - Settings structure is NULL", BSTD_FUNCTION));
        goto ErrorExit;
    }

    rc = DRM_Common_Initialize(&pHdcpRxSettings->drmCommonInit, pHdcpRxSettings->drm_bin_file_path);
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error calling 'DRM_Common_Initialize'", BSTD_FUNCTION));
        goto ErrorExit;
    }

 ErrorExit:
    BDBG_MSG(("%s - ... Exiting", BSTD_FUNCTION));

    return rc;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpRx_Finalize
 **
 ** DESCRIPTION:
 **   Close the HdcpRx module
 **
 ** PARAMETERS:
 **   N/A
 **
 ** RETURNS:
 **   N/A
 **
 ******************************************************************************/
void DRM_HdcpRx_Finalize(void)
{
    BDBG_MSG(("%s - Entering...", BSTD_FUNCTION));

    DRM_Common_Finalize();

    BDBG_MSG(("%s - ...Exiting", BSTD_FUNCTION));
    return;
}

/******************************************************************************
 ** FUNCTION:
 **   DRM_HdcpRx_GetData
 **
 ** DESCRIPTION:
 **   Extract data from HdcpRx credential
 **
 ** PARAMETERS:
 **   pHdcpRxData[out] structure containing HdcpRx data from DRM key region
 **
 ** RETURNS:
 **   Drm_Success when the operation is successful
 **
 ******************************************************************************/
DrmRC DRM_HdcpRx_GetData(drm_hdcp_rx_data_t *pHdcpRxData)
{
    DrmRC rc = Drm_Success;
    uint8_t sha1_digest[20] = {0x00};
    drm_hdcp_rx_data_t hdcpRxData;
    uint32_t keyset_size = 0;

    BDBG_MSG(("%s - Entering function", BSTD_FUNCTION));

    rc = DRM_KeyRegion_GetKeyData(DRM_HDCP_RX, (uint8_t*)&hdcpRxData);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - ", BSTD_FUNCTION));
        goto ErrorExit;
    }

    rc = DRM_Common_SwSha1(hdcpRxData.licensed_constant, sha1_digest, sizeof(hdcpRxData.licensed_constant));
    if (rc != Drm_Success)
    {
        BKNI_Memset((uint8_t*)&hdcpRxData, 0x00, sizeof(drm_hdcp_rx_data_t));
        BDBG_ERR(("%s - Hash calculation of licensed constant failed", BSTD_FUNCTION));
        goto ErrorExit;
    }

    DRM_MSG_PRINT_BUF("sha1_digest", sha1_digest, sizeof(sha1_digest));
    DRM_MSG_PRINT_BUF("hdcpRxData.sha1_of_lc", hdcpRxData.sha1_of_lc, sizeof(hdcpRxData.sha1_of_lc));
    DRM_MSG_PRINT_BUF("hdcpRxData.licensed_constant", hdcpRxData.licensed_constant, sizeof(hdcpRxData.licensed_constant));

    if(BKNI_Memcmp(sha1_digest, hdcpRxData.sha1_of_lc, sizeof(hdcpRxData.sha1_of_lc)) != 0)
    {
        BKNI_Memset((uint8_t*)&hdcpRxData, 0x00, sizeof(drm_hdcp_rx_data_t));
        BDBG_ERR(("%s - Licensed constant verification failed", BSTD_FUNCTION));
        goto ErrorExit;
    }

    keyset_size = GET_UINT32_FROM_BUF(hdcpRxData.size_of_keyset);
    BDBG_MSG(("%s - size of keyset '%u'", BSTD_FUNCTION, keyset_size));

    rc = DRM_Common_SwSha1(hdcpRxData.rx_id, sha1_digest, keyset_size - 20);
    if (rc != Drm_Success)
    {
        BKNI_Memset((uint8_t*)&hdcpRxData, 0x00, sizeof(drm_hdcp_rx_data_t));
        BDBG_ERR(("%s - Keyset validation failed", BSTD_FUNCTION));
        goto ErrorExit;
    }

    if(BKNI_Memcmp(sha1_digest, hdcpRxData.keyset_sha1, sizeof(hdcpRxData.keyset_sha1)) != 0)
    {
        BKNI_Memset((uint8_t*)&hdcpRxData, 0x00, sizeof(drm_hdcp_rx_data_t));
        BDBG_ERR(("%s - Keyset hash verification failed", BSTD_FUNCTION));
        goto ErrorExit;
    }

    BKNI_Memcpy((uint8_t *)pHdcpRxData, (uint8_t*)&hdcpRxData, sizeof(drm_hdcp_rx_data_t));

 ErrorExit:
    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}
