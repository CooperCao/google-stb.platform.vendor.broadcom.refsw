/******************************************************************************
 * (c) 2006-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#ifndef BDSP_RAAGA_FW_ALGO_H_
#define BDSP_RAAGA_FW_ALGO_H_

#include "bdsp_types.h"
#include "bdsp_raaga_fw_settings.h"
#include "bdsp_raaga_fw.h"
#include "bdsp.h"
#include "bimg.h"

typedef struct BDSP_AF_P_sALGO_EXEC_INFO
{
    uint32_t                    NumNodes;
    BDSP_AF_P_AlgoId            eAlgoIds[BDSP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BDSP_AF_P_sALGO_EXEC_INFO;

typedef struct BDSP_Raaga_P_AlgorithmSupportInfo
{
    BDSP_Algorithm              algorithm;
    BDSP_AlgorithmType          type;
    const char                  *pName;
    bool                        DolbyLicensePresent;
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion;
    BDSP_AF_P_sALGO_EXEC_INFO   algoExecInfo;
}BDSP_Raaga_P_AlgorithmSupportInfo;

typedef struct BDSP_Raaga_P_AlgorithmInfo
{
    BDSP_Algorithm            algorithm;
    BDSP_AlgorithmType        type;
    const char               *pName;
    bool                      supported;
    const void               *pDefaultUserConfig;
    size_t                    userConfigSize;
    size_t                    statusBufferSize;
    unsigned                  statusValidOffset;
    const void               *pDefaultIdsConfig;
    size_t                    idsConfigSize;
    BDSP_AF_P_sALGO_EXEC_INFO algoExecInfo;
} BDSP_Raaga_P_AlgorithmInfo;

const BDSP_Raaga_P_AlgorithmSupportInfo *BDSP_Raaga_P_LookupAlgorithmSupported(
    BDSP_Algorithm              algorithm,
    BDSP_AudioDolbyCodecVersion DolbyCodecVersion
    );

const BDSP_Raaga_P_AlgorithmInfo *BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe(
    BDSP_Algorithm algorithm
    );

BERR_Code BDSP_Raaga_P_GetFWSize (
    const BIMG_Interface *iface,
    void *context,
    unsigned firmware_id,
    uint32_t *size
    );

bool BDSP_Raaga_P_AlgoHasTables(
    BDSP_AF_P_AlgoId algorithm);

#define BDSP_Raaga_P_LookupAlgorithmInfo BDSP_Raaga_P_LookupAlgorithmInfo_isrsafe

extern  const BDSP_Raaga_Audio_FrameSyncTsmConfigParams     BDSP_sDefaultFrameSyncTsmSettings;
extern  const BDSP_VideoEncodeTaskDatasyncSettings          BDSP_sDefaultVideoEncodeFrameSyncSettings;
extern  const BDSP_AF_P_sNODE_INFO                          BDSP_sNodeInfo [BDSP_AF_P_AlgoId_eMax];
extern  const uint32_t                                      BDSP_SystemID_MemoryReqd[BDSP_SystemImgId_eMax];

#endif
