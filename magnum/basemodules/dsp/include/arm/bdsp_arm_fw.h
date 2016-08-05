/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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


#ifndef BDSP_ARM_FW_H_
#define BDSP_ARM_FW_H_

#include "bdsp_common_fw.h"

#define BDSP_ARM_INVALID_DRAM_ADDRESS 0xFFFFFFFF

#define BDSP_ARM_MAX_ALLOC_DEVICE               8 /* Cmd Q + Response Q + Scratch + InterStage IO + InterStage IOGen + Map Table + RDB incase Raaga is used + Interface Q handle Dram Mem*/
#define BDSP_ARM_MAX_ALLOC_TASK                 9 /* Task Cfg + Sync Q + Async Q + Stack Swap + CIT + Spare CIT + Start Task + Feedback */
#define BDSP_ARM_MAX_ALLOC_STAGE                (6+BDSP_AF_P_MAX_IP_FORKS+BDSP_AF_P_MAX_OP_FORKS+6) /* Interframe + User config + Table+ Spare user config + Status + IO/IOGEN Descriptor  */

#define BDSP_ARM_MAX_STAGES_PER_TASK            2
#define BDSP_ARM_MAX_FW_TASK_PER_DSP            (int32_t)4 /* Right now we forsee only max of 4 tasks running at ARM */

#define BDSP_ARM_MAX_MAP_TABLE_ENTRY    (BDSP_ARM_MAX_ALLOC_DEVICE+(BDSP_ARM_MAX_ALLOC_TASK*BDSP_ARM_MAX_FW_TASK_PER_DSP)+(BDSP_ARM_MAX_ALLOC_STAGE*BDSP_ARM_MAX_STAGES_PER_TASK))

/**************************************************************************
        Inter Task Feed back buffer path
***************************************************************************/
#define BDSP_ARM_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE    (uint32_t)(128*4)

/***************************************************************************
Summary:
    Enum data type having all algorithm ids of all the stages of algorithms
    supported in Audio Firmware.

Description:
    This is the enumerated data type used between Audio firmware and the PI
    to indicate the algorithm id of the code to be executed for a node. This
    enum is comprahensive and contains the stages of encode, decode & Post
    process algorithms. The frame sync and TSM executables of all the
    algorithms are also present in this enum.

See Also:
    BDSP_DSPCHN_AudioType
****************************************************************************/


typedef enum BDSP_Arm_SystemImgId
{
    BDSP_ARM_SystemImgId_eSystemCode,
	BDSP_ARM_SystemImgId_eHbcMonitorCode,
#if 0
    /*BDSP_ARM_SystemImgId_eSystemRdbvars,        */
    BDSP_ARM_SystemImgId_eSyslibCode,
    BDSP_ARM_SystemImgId_eAlgolibCode,
    BDSP_ARM_SystemImgId_eCommonIdsCode,

    BDSP_ARM_SystemImgId_eCommonVideoEncodeIdsCode,
    BDSP_ARM_SystemImgId_eCommonVideoEncodeIdsInterframe,
    BDSP_ARM_SystemImgId_eScm_Task_Code,
    BDSP_ARM_SystemImgId_eVideo_Decode_Task_Code,
    BDSP_ARM_SystemImgId_eVideo_Encode_Task_Code,
    BDSP_ARM_SystemImgId_eScm1_Digest,
    BDSP_ARM_SystemImgId_eScm2_Digest,
#endif
    BDSP_ARM_SystemImgId_eMax
} BDSP_Arm_SystemImgId;

typedef enum BDSP_ARM_AF_P_AlgoId
{
    /******************* Audio Algorithm Start ****************************/
    BDSP_ARM_AF_P_AlgoId_eAudioAlgoStartIdx = 0x0,          /*Audio Algorithm Start Index */
    BDSP_ARM_AF_P_AlgoId_eDDPEncode = BDSP_ARM_AF_P_AlgoId_eAudioAlgoStartIdx,
    BDSP_ARM_AF_P_AlgoId_eAudioAlgoEndIdx = BDSP_ARM_AF_P_AlgoId_eDDPEncode,
    BDSP_ARM_AF_P_AlgoId_eDDPEncFrameSync,
    BDSP_ARM_AF_P_AlgoId_eEndOfEncFsAlgos = BDSP_ARM_AF_P_AlgoId_eDDPEncFrameSync,
    BDSP_ARM_AF_P_AlgoId_eMax,
    BDSP_ARM_AF_P_AlgoId_eInvalid = 0x7FFFFFFF
}BDSP_ARM_AF_P_AlgoId;


/***************************************************************************
Summary:
    Enum data type describing the Memory mapping to be used in Astra

Description:

Memory can be mapped as following
     DRAM
     DEVICE
     RD_ONLY
     NO_EXEC

See Also:
    None.
****************************************************************************/
typedef enum BDSP_ARM_AF_P_MemoryMapType
{
    BDSP_ARM_AF_P_Map_eDram            = 0x0,
    BDSP_ARM_AF_P_Map_eDevice,
    BDSP_ARM_AF_P_Map_eRdOnly,
    BDSP_ARM_AF_P_Map_eNoExec,
    BDSP_ARM_AF_P_Map_eInvalid,
    BDSP_ARM_AF_P_Map_eMax             = 0x7FFFFFFF
}BDSP_ARM_AF_P_MemoryMapType;

/*********************************************************************
Summary:
    Structure to describe one MAP table entry

Description:
    This structure describes one entry of the MAP Table.

See Also:
**********************************************************************/
typedef struct BDSP_MAP_Table_Entry
{
    uint32_t            ui32PageStart;
    uint32_t            ui32Size;
    BDSP_ARM_AF_P_MemoryMapType eMemMapType;
}BDSP_MAP_Table_Entry;

/*********************************************************************
Summary:
    Structure to describe MAP Table
Description:
    This structure describes MAP Table.
See Also:
**********************************************************************/
typedef struct BDSP_MAP_Table
{
    BDSP_MAP_Table_Entry sMapTableDetail[BDSP_ARM_MAX_MAP_TABLE_ENTRY];
}BDSP_MAP_Table;

/***************************************************************************
Summary:
    The structure contains the configurations for an individual node.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_ARM_AF_P_sNODE_CONFIG
{
    uint32_t                        uiNodeId;
    BDSP_AF_P_EnableDisable         eCollectResidual;
    uint32_t                        ui32AudioAlgorithmType;
    BDSP_ARM_AF_P_AlgoId            eAlgoId;
    BDSP_AF_P_sDRAM_BUFFER          sDramUserConfigBuffer;
    BDSP_AF_P_sDRAM_BUFFER          sDramInterFrameBuffer;
    BDSP_AF_P_sDRAM_BUFFER          sDramAlgoCodeBuffer;
    BDSP_AF_P_sDRAM_BUFFER          sDramLookupTablesBuffer;
    BDSP_AF_P_sDRAM_BUFFER          sDramStatusBuffer;
    uint32_t                        ui32VomAlgoAddr;

    uint32_t                        ui32NumSrc;
    uint32_t                        ui32NumDst;

    /*The filed that tells whether the Node input is Valid/ Invalid : Valid =1 Invalid =0
      This field is required for Dynamic input port switching. All the input ports which
      are interstage buffers will be set to valid
    */
    BDSP_AF_P_ValidInvalid      eNodeIpValidFlag[BDSP_AF_P_MAX_IP_FORKS];

    uint32_t                    ui32NodeIpBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];
    uint32_t                    ui32NodeIpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_IP_FORKS];

    uint32_t                    ui32NodeOpBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];
    uint32_t                    ui32NodeOpGenericDataBuffCfgAddr[BDSP_AF_P_MAX_OP_FORKS];

    BDSP_AF_P_DistinctOpType    eNodeOpBuffDataType[BDSP_AF_P_MAX_OP_FORKS];


}BDSP_ARM_AF_P_sNODE_CONFIG;

/***************************************************************************
Summary:
    The structure is complete task configuration structure. This contains
    the global task configuration and an array of node configuration
    structures.

See Also:
    None.
****************************************************************************/
typedef struct BDSP_ARM_AF_P_sTASK_CONFIG
{
    BDSP_AF_P_sGLOBAL_TASK_CONFIG   sGlobalTaskConfig;
    BDSP_ARM_AF_P_sNODE_CONFIG          sNodeConfig[BDSP_AF_P_MAX_NODES];
}BDSP_ARM_AF_P_sTASK_CONFIG;

#endif /* BDSP_ARM_FW_H_ */
