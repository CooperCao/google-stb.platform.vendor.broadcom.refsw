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

#ifndef BDSP_COMMON_FWDOWNLOAD_PRIV_H_
#define BDSP_COMMON_FWDOWNLOAD_PRIV_H_

#include "bdsp_common_priv_include.h"

#define BDSP_MAX_DOWNLOAD_BUFFERS      4
/* This chunk size will be used when the firmware binary is actually present in
    multiple chunks. The BDSP_IMG_CHUNK_SIZE will then give the size of each
    such chunk
*/
#define BDSP_IMG_CHUNK_SIZE 65532

typedef struct BDSP_P_ImageBlockInfo
{
    BDSP_MMA_Memory Memory;
    BDSP_Algorithm algorithm;
    int32_t numUser;
    bool bDownloadValid;
}BDSP_P_ImageBlockInfo;

typedef struct BDSP_P_AlgoTypeSplitInfo
{
    unsigned maxImageSize;
    unsigned numImageBlock;
    BDSP_P_ImageBlockInfo sImageBlockInfo[BDSP_MAX_DOWNLOAD_BUFFERS];
}BDSP_P_AlgoTypeSplitInfo;

typedef struct BDSP_P_LoadableImageInfo
{
    unsigned allocatedSize;
    unsigned supportedSize;
    BDSP_P_AlgoTypeSplitInfo sAlgoTypeSplitInfo[BDSP_AlgorithmType_eMax];
}BDSP_P_LoadableImageInfo;

BERR_Code BDSP_P_GetFWSize (
    const BIMG_Interface *iface,
    void *context,
    uint32_t firmware_id,
    uint32_t *size
);
BERR_Code BDSP_P_CopyFWImageToMem(
    const BIMG_Interface *iface,
    void                 *pImgContext,
    BDSP_MMA_Memory      *pMemory,
    unsigned              firmware_id
);
#endif /* BDSP_COMMON_FWDOWNLOAD_PRIV_H_ */
