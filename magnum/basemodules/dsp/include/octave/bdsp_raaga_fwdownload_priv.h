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
 *****************************************************************************/

#ifndef BDSP_RAAGA_FWDOWNLOAD_PRIV_H_
#define BDSP_RAAGA_FWDOWNLOAD_PRIV_H_

#include "bdsp_raaga_priv_include.h"

typedef struct BDSP_Raaga_P_CodeDownloadInfo
{
    bool  preloadImages;		/* If true, all firmware images will be loaded on startup.	Default=false. */
    BDSP_P_FwBuffer imgInfo[BDSP_IMG_ID_MAX];
    BDSP_P_LoadableImageInfo  sLoadableImageInfo;
}BDSP_Raaga_P_CodeDownloadInfo;

BERR_Code BDSP_Raaga_P_AssignAlgoSize(
    const BIMG_Interface *pImageInterface,
    void                **pImageContext,
    BDSP_P_FwBuffer      *pImgInfo
);
BERR_Code BDSP_Raaga_P_AssignAlgoSize_APITool(
	const BDSP_UsageOptions  *pUsage,
	BDSP_P_FwBuffer 	     *pImgInfo
);
BERR_Code BDSP_Raaga_P_ComputeLoadbleSection_APITool(
	const BDSP_RaagaSettings      *pSettings,
	const BDSP_UsageOptions       *pUsage,
	BDSP_Raaga_P_CodeDownloadInfo *pCodeDownloadInfo,
	unsigned *pMemReqd
);
BERR_Code BDSP_Raaga_P_RequestImg(
    const BIMG_Interface *pImageInterface,
    void **pImageContext,
    BDSP_P_FwBuffer *pImgCache,
    unsigned imageId,
    bool bDownload,
    BDSP_MMA_Memory *pMemory
);
BERR_Code BDSP_Raaga_P_ComputeResidentSection(
	BDSP_Raaga_P_CodeDownloadInfo *pCodeInfo,
	unsigned *pMemReqd
);
BERR_Code BDSP_Raaga_P_ComputeLoadbleSection(
	void     *pDevice,
	unsigned *pMemReqd
);
BERR_Code BDSP_Raaga_P_DownloadCode(
	void *pDevice
);
BERR_Code BDSP_Raaga_P_DownloadAlgorithm(
	void *pDevice,
	BDSP_Algorithm algorithm
);
BERR_Code BDSP_Raaga_P_ReleaseAlgorithm(
	void *pDevice,
	BDSP_Algorithm algorithm
);
BERR_Code BDSP_Raaga_P_GetDownloadStatus(
    void                *pDeviceHandle,
    BDSP_DownloadStatus *pStatus /* [out] */
);
BERR_Code BDSP_Raaga_P_DumpImage(
    void       *pBuffer,
    unsigned    uiBufferSize,
    void      **pvCodeStart,
    unsigned   *puiCodeSize,
    const BIMG_Interface *pImageInterface,
    void **pImageContext
);
#endif /*BDSP_RAAGA_FWDOWNLOAD_PRIV_H_*/
