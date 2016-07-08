/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 *****************************************************************************/
#ifndef BDSP_COMMON_CIT_PRIV_H_
#define BDSP_COMMON_CIT_PRIV_H_

#include "bdsp_common_priv_include.h"

/***************************************/
/*   CIT GENERATION CODE DEFINES       */
/***************************************/
#define BDSP_CIT_P_ENABLE_FORK_MATRIXING
#define ANALYZE_IO_CFG

#define SIZEOF(x)   ( sizeof(x) )

/* MEMORY REQUIRED FOR RAAGA SYSTEM SWAP */
#define BDSP_P_FW_SYSTEM_SWAP_MEMORY_SIZE               ((uint32_t)(12*1024))

/* STATIC MEMORY ALLOCATION FOR A TASK */

#define BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES        ((uint32_t)(2048*4)) /* Task Swap Buffer size in bytes */
#define BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE            ((uint32_t)((((SIZEOF(BDSP_AF_P_sFMM_DEST_CFG)*BDSP_AF_P_MAX_NUM_PLLS + 20)+3)>>2)<<2)) /* Task's output port configuration memory size in bytes */
#define BDSP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE         ((uint32_t)((((SIZEOF(BDSP_AF_P_sSPDIF_USER_CFG)*BDSP_AF_P_MAX_NUM_SPDIF_PORTS + 20) +3)>>2)<<2)) /* Task's SPDIF user configuration memory size in bytes for all ports*/
#define BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG            ((uint32_t)((((SIZEOF(BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG) + 20)+3)>>2)<<2)) /* FMM gate open configuration memory size in bytes*/
#define BDSP_CIT_P_TASK_HW_FW_CONFIG                    ((uint32_t)((((SIZEOF(BDSP_AF_P_sFW_HW_CFG) + 20)+3)>>2)<<2))
#define BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE             ((uint32_t)((((SIZEOF(BDSP_AF_P_sOpSamplingFreq))+3)>>2)<<2))
#define BDSP_CIT_P_TASK_STC_TRIG_CONFIG_SIZE            ((uint32_t)((((SIZEOF(BDSP_AF_P_sStcTrigConfig))+3)>>2)<<2))

#define MAX_CHAR_LENGTH     80

#define BDSP_CIT_P_PRESENT                          ((uint32_t)(1))
#define BDSP_CIT_P_ABSENT                           ((uint32_t)(0))

/*********************************************************************
Summary:
    Enumeration to describe type of source or destination for a FW stage

Description:
    This enumeration describes types of source/destiantion from/to
    which a fw stage can receive/feed data.

See Also:
**********************************************************************/

typedef enum BDSP_CIT_P_FwStgSrcDstType
{
    BDSP_CIT_P_FwStgSrcDstType_eFwStg,          /* Source or destination is another FW stage */
    BDSP_CIT_P_FwStgSrcDstType_eRaveBuf,        /* Source or destination is an IO buffer */
    BDSP_CIT_P_FwStgSrcDstType_eFMMBuf,         /* Source or destination is an IO buffer */
    BDSP_CIT_P_FwStgSrcDstType_eRDB,
    BDSP_CIT_P_FwStgSrcDstType_eDRAMBuf,
    BDSP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf,   /* The Dram Buffer Shared Across Multiple Tasks */
    BDSP_CIT_P_FwStgSrcDstType_eRDBPool,        /* Pooled buffers, indirection through a queue using RDB */
    BDSP_CIT_P_FwStgSrcDstType_eMax,
    BDSP_CIT_P_FwStgSrcDstType_eInvalid = 0x7FFFFFFF
} BDSP_CIT_P_FwStgSrcDstType;


/*********************************************************************
Summary:
    Structure for Algo present in the Node structure
    Optional fields for the CIT_Gen Structure in special case
Description:
    All the big datastructures of CIT Gen module is set here

See Also:
**********************************************************************/
typedef struct BDSP_CIT_P_sAlgoModePresent
{
    uint32_t    ui32DDP_PassThruPresent;
    uint32_t    ui32DTS_EncoderPresent;
    uint32_t    ui32AC3_EncoderPresent;
    uint32_t    ui32DolbyPulsePresent;
    uint32_t    ui32DdrePresent;

}BDSP_CIT_P_sAlgoModePresent;

void BDSP_P_PopulateStcTrigConfig(
                BDSP_AF_P_sStcTrigConfig    *psStcTrigConfig,
                BDSP_TaskStartSettings      *pStartSettings
            );

void BDSP_P_InitializeFmmDstCfg(
                BDSP_AF_P_sFMM_DEST_CFG     *psFmmDestCfgArray
            );

uint32_t BDSP_P_FillSamplingFrequencyMapLut(
                BMEM_Handle                 hHeap,
                BDSP_AF_P_DolbyMsUsageMode  eDolbyMsUsageMode,
                uint32_t                    ui32FwOpSamplingFreqMapLutAddr,
                BDSP_CIT_P_sAlgoModePresent *psAlgoModePresent
            );

extern const char BuffTypeEnum2Char[BDSP_AF_P_BufferType_eLAST][MAX_CHAR_LENGTH];
extern const char PortDatatType[BDSP_AF_P_DistinctOpType_eMax][MAX_CHAR_LENGTH];
extern const char PortValidType[BDSP_AF_P_ValidInvalid_eMax][MAX_CHAR_LENGTH];
extern const char DisableEnable[2][MAX_CHAR_LENGTH];
extern const char GlobalTimeBase [2][MAX_CHAR_LENGTH];

#endif /*BDSP_COMMON_CIT_PRIV_H_*/
