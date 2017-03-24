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

#include "bdsp_raaga_priv.h"
#include "bdsp_raaga_fw_cit.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"
#include "bdsp_task.h"
#include "bdsp_common_cit_priv.h"
#include "bdsp_common_priv.h"

#include "bchp_aud_fmm_bf_ctrl.h"

BDBG_MODULE(bdsp_cit_priv);

/******************************************************************************
Summary:

    Compute the Buffer Size of a Task's stack

Description:

    Every Task will be assiciated with a DRAM stack. This is to enable stack swap
    in DSP.

    Allocated stack size per task is BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES
    bytes.

Input:
        None
Output:
        psTaskBuffInfo : Buffer information for a task
Returns:
        None

******************************************************************************/
static void BDSP_Raaga_P_ComputeTaskStackBuffSize(
                    BDSP_CIT_P_sTaskBuffInfo    *psTaskBuffInfo )

{
    BDBG_ENTER(BDSP_Raaga_P_ComputeTaskStackBuffSize);

    psTaskBuffInfo->ui32TaskStackMemSize
                        = BDSP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;

    BDBG_LEAVE(BDSP_Raaga_P_ComputeTaskStackBuffSize);

}
static BERR_Code BDSP_Raaga_P_PopulateFwHwBuffer(
                                void                  *pPrimaryStageHandle,
                                BDSP_AF_P_sFW_HW_CFG  *psFwHwCfg)
{
    BERR_Code errCode = BERR_SUCCESS;
    unsigned output;

    unsigned ui32Count = 0, ui32PPMCount = 0, ui32BufferId = 0;

    BDSP_RaagaStage *pRaagaPrimaryStage = (BDSP_RaagaStage *)pPrimaryStageHandle;

    BDBG_ASSERT(NULL != pRaagaPrimaryStage);

    /*Initialization*/
    for(ui32Count =0; ui32Count<BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS;ui32Count++)
    {
        psFwHwCfg->sPpmCfg[ui32Count].ePPMChannel    = BDSP_AF_P_eDisable;
        psFwHwCfg->sPpmCfg[ui32Count].ui32PPMCfgAddr = (uint32_t)((unsigned long)NULL);
    }

    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        for(output=0;output<BDSP_AF_P_MAX_OP_FORKS;output++)
        {
            if(pRaagaConnectStage->sStageOutput[output].eConnectionType == BDSP_ConnectionType_eFmmBuffer &&
                pRaagaConnectStage->sStageOutput[output].eNodeValid==BDSP_AF_P_eValid)
            {
                for(ui32Count =0; ui32Count<BDSP_AF_P_MAX_CHANNEL_PAIR;ui32Count++)
                {
                    ui32BufferId = pRaagaConnectStage->sStageOutput[output].Metadata.rateController[ui32Count].wrcnt;
                    /*ui32BufferId would be -1 by default by init if FMM dest not added*/
                    if(ui32BufferId != BDSP_CIT_P_PI_INVALID)
                    {
                        if(ui32PPMCount >= BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS)
                        {
                            errCode = BERR_LEAKED_RESOURCE;
                            goto error;
                        }

                        psFwHwCfg->sPpmCfg[ui32PPMCount].ePPMChannel = BDSP_AF_P_eEnable;

                        psFwHwCfg->sPpmCfg[ui32PPMCount].ui32PPMCfgAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32BufferId );

                        ui32PPMCount = ui32PPMCount + 1;
                    }
                }

            }
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)
error:
    return errCode;
}

static BERR_Code BDSP_Raaga_P_PopulateAlgoMode(
                BDSP_RaagaStage *pRaagaPrimaryStage,
                BDSP_CIT_P_sAlgoModePresent *sAlgoModePresent)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_ASSERT(NULL != pRaagaPrimaryStage);

    /*Initialize first*/
    sAlgoModePresent->ui32DolbyPulsePresent   = BDSP_CIT_P_ABSENT;
    sAlgoModePresent->ui32DDP_PassThruPresent = BDSP_CIT_P_ABSENT;
    sAlgoModePresent->ui32DTS_EncoderPresent  = BDSP_CIT_P_ABSENT;
    sAlgoModePresent->ui32AC3_EncoderPresent  = BDSP_CIT_P_ABSENT;
    sAlgoModePresent->ui32DdrePresent         = BDSP_CIT_P_ABSENT;

    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        /* Handle special case algorithms */
        switch ( pRaagaConnectStage->algorithm )
        {
            case BDSP_Algorithm_eDolbyPulseAdtsDecode:
            case BDSP_Algorithm_eDolbyPulseLoasDecode:
                sAlgoModePresent->ui32DolbyPulsePresent = BDSP_CIT_P_PRESENT;
                break;
            case BDSP_Algorithm_eAc3PlusPassthrough:
            case BDSP_Algorithm_eUdcPassthrough:
                sAlgoModePresent->ui32DDP_PassThruPresent = BDSP_CIT_P_PRESENT;
                break;
            case BDSP_Algorithm_eDtsCoreEncode:
                sAlgoModePresent->ui32DTS_EncoderPresent = BDSP_CIT_P_PRESENT;
                break;
            case BDSP_Algorithm_eAc3Encode:
                sAlgoModePresent->ui32AC3_EncoderPresent = BDSP_CIT_P_PRESENT;
                break;
            case BDSP_Algorithm_eDdre:
                sAlgoModePresent->ui32DdrePresent = BDSP_CIT_P_PRESENT;
                break;
            default:
                break;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

    return errCode;
}

static BERR_Code  BDSP_Raaga_P_GetNumZeroFillSamples(
    uint32_t    *pui32ZeroFillSamples,
    BDSP_RaagaStage *pRaagaPrimaryStage
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    bool foundValidStage = false;
    BDSP_RaagaStage *pRaagaStage = NULL;

    BDBG_ENTER(BDSP_Raaga_P_GetNumZeroFillSamples);
    BDBG_ASSERT(NULL != pRaagaPrimaryStage);

    BDSP_STAGE_TRAVERSE_LOOP_V1_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage, branchId, stageId)
    BSTD_UNUSED(stageId);
    {
        pRaagaStage = pRaagaConnectStage;
        if(branchId == 0)/*check only the first branch*/
        {
            /* Iterate till you hit the first Decode/Encode stage */
            switch ( BDSP_RAAGA_P_ALGORITHM_TYPE(pRaagaConnectStage->algorithm) )
            {
                case BDSP_AlgorithmType_eAudioDecode:
                case BDSP_AlgorithmType_eAudioMixer:
                case BDSP_AlgorithmType_eAudioPassthrough:
                case BDSP_AlgorithmType_eAudioEncode:
                case BDSP_AlgorithmType_eVideoDecode:
                case BDSP_AlgorithmType_eVideoEncode:
                case BDSP_AlgorithmType_eSecurity:
                    foundValidStage = true;
                    break;
                default:
                    break;
            }
        }
        else
        {
            /*break after the end of first branch*/
            break;
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

    BDBG_ASSERT(NULL != pRaagaStage);

    if( !(foundValidStage) )
    {
        *pui32ZeroFillSamples = 0;
        BDBG_ERR(("Unable to find the Decoder/Encoder stage in the 1st branch "));
        BDBG_LEAVE(BDSP_Raaga_P_GetNumZeroFillSamples);
        return BERR_UNKNOWN;
    }

    switch(pRaagaStage->algorithm)
    {
        case BDSP_Algorithm_eMpegAudioDecode:
        case BDSP_Algorithm_eMpegAudioPassthrough:
            *pui32ZeroFillSamples = 13824;
            break;
        case BDSP_Algorithm_eAc3Decode:
        case BDSP_Algorithm_eAc3Passthrough:
        case BDSP_Algorithm_eUdcPassthrough:
            *pui32ZeroFillSamples = 18432;
            break;

        default:
            *pui32ZeroFillSamples = 0;
            break;
    }

    BDBG_LEAVE(BDSP_Raaga_P_GetNumZeroFillSamples);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_FillNodeCfg(
                    BDSP_RaagaStage *pPrimaryStageHandle,
                    uint32_t dspIndex,
                    BDSP_AF_P_sNODE_CONFIG  *psNodeCfg,
                    unsigned *ui32TotalNodes,
                    BDSP_AF_P_eSchedulingGroup eSchedulingGroup)
{

    BERR_Code   errCode = BERR_SUCCESS;
    uint32_t    ui32Node;
    uint32_t    ui32NumNodesInAlgo;
    uint32_t    ui32NodeIndex, ui32Ip, ui32Op;
    unsigned    ui32IOPhysAddr, ui32IOGenPhysAddr, i;
    bool        collectResidue;
    BDSP_AF_P_sIO_GENERIC_BUFFER *pTempIoGenBuffer_Cached = NULL;
    BDSP_AF_P_sIO_BUFFER         *pTempIoBuffer_Cached = NULL;
    uint32_t    j, k;
    uint32_t    ui32FmmPortDstCount[BDSP_AF_P_DistinctOpType_eMax] = {0};

    const BDSP_Raaga_P_AlgorithmInfo *sAlgoInfo;

    BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_RaagaContext *pRaagaContext;

    BDSP_Raaga *pRaagaDevice;

    BDSP_AF_P_ValidInvalid          eNodeValid = BDSP_AF_P_eInvalid;

    BDBG_ENTER(BDSP_Raaga_P_FillNodeCfg);

    errCode = BERR_SUCCESS;

    pRaagaPrimaryStage = (BDSP_RaagaStage *)pPrimaryStageHandle;
    BDBG_ASSERT(NULL != pRaagaPrimaryStage->pContext);
    pRaagaContext = (BDSP_RaagaContext *)pRaagaPrimaryStage->pContext;
    BDBG_ASSERT(NULL != pRaagaContext->pDevice);
    pRaagaDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    ui32NodeIndex=0;

    collectResidue = true;

    /*Now, traverse through all the stages. Hold back the stage forking for later traversing.
    The assumption is that the stage o/p connection handle will be NULL if it has no o/p interstage connection */
    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {
        sAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->algorithm);

        ui32NumNodesInAlgo = sAlgoInfo->algoExecInfo.NumNodes;

        for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
        {
            /*Intialization for Master Slave Configuration Counter */
            for(j = 0; j< BDSP_AF_P_DistinctOpType_eMax; j++)
            {
                ui32FmmPortDstCount[j] = 0;
            }

            if(sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node] != BDSP_AF_P_AlgoId_eInvalid )
            {
                psNodeCfg->uiNodeId = ui32NodeIndex;/*increment to next node*/
                /*Populating the Collect Residual Flag */
                /*Branch Id is populated during the stage traverse to Download FW execs. Use it now.*/
                psNodeCfg->eCollectResidual = (collectResidue) ? BDSP_AF_P_eEnable : BDSP_AF_P_eDisable;
                BDBG_MSG(("Collect Residue [%s] = %d", (BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->algorithm))->pName, collectResidue));

                psNodeCfg->eAlgoId = sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node];

                /* Audio Algorithm Type */
                psNodeCfg->ui32AudioAlgorithmType = sAlgoInfo->algorithm;

                /* TBD: Need to update the DDP decoder to stop using the decoderencpptype and then remove this hack */
                switch(sAlgoInfo->algorithm)
                {
                    case BDSP_Algorithm_eAc3Decode:
                    case BDSP_Algorithm_eAc3Passthrough:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eAc3;
                        break;
                    case BDSP_Algorithm_eMpegAudioDecode:
                    case BDSP_Algorithm_eMpegAudioPassthrough:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eMpeg;
                        break;
                    case BDSP_Algorithm_eAacAdtsDecode:
                    case BDSP_Algorithm_eAacAdtsPassthrough:
                    case BDSP_Algorithm_eAacLoasDecode:
                    case BDSP_Algorithm_eAacLoasPassthrough:
                    case BDSP_Algorithm_eDolbyPulseAdtsDecode:
                    case BDSP_Algorithm_eDolbyPulseLoasDecode:
                    case BDSP_Algorithm_eDolbyAacheAdtsDecode:
                    case BDSP_Algorithm_eDolbyAacheLoasDecode:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eAac;
                        break;
                    case BDSP_Algorithm_eAc3PlusDecode:
                    case BDSP_Algorithm_eAc3PlusPassthrough:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eAc3Plus;
                        break;
                    case BDSP_Algorithm_eUdcDecode:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eUdc;
                        break;
                    case BDSP_Algorithm_eUdcPassthrough:
                        /* This is specifically added if UDC Passthru is supported in Firmware and to take care of DDP or DD stream, the decision has to be made here */
                        for(k = 0; k< BDSP_AF_P_MAX_OP_FORKS; k++)
                        {
                            if(BDSP_AF_P_DistinctOpType_eCompressed4x == pRaagaConnectStage->eStageOpBuffDataType[k])
                            {
                                psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eAc3Plus;
                            }
                            else if( BDSP_AF_P_DistinctOpType_eCompressed == pRaagaConnectStage->eStageOpBuffDataType[k])
                            {
                                psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eAc3;
                            }
                            else
                            {
                                /* DO NOTHING */
                            }
                        }
                        break;
                    case BDSP_Algorithm_eWmaStdDecode:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eWmaStd;
                        break;
                    case BDSP_Algorithm_eDts14BitDecode:
                    case BDSP_Algorithm_eDts14BitPassthrough:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eDtsBroadcast;
                        break;
                    case BDSP_Algorithm_eDtsHdDecode:
                    case BDSP_Algorithm_eDtsHdPassthrough:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eDtshd;
                        break;
                    case BDSP_Algorithm_eDraDecode:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_DecodeAlgoType_eDra;
                        break;
                    default:
                        break;
                }

                BDBG_MSG(("ui32NodeIndex=%d", ui32NodeIndex));
                BDBG_MSG(("sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node]=%d, ui32Node=%d", sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node], ui32Node));

                /*  Code Buffer */
                psNodeCfg->ui32VomAlgoAddr =
                                                BDSP_sAlgoStartAddr.sVomAlgoStartAddr[psNodeCfg->eAlgoId];
                psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_CODE(psNodeCfg->eAlgoId)].offset;
                psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_CODE(psNodeCfg->eAlgoId)].size;

                if(ui32Node == 0)
                {
                    /*  Inter-Frame buffer */
                    psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress =
                                                    (dramaddr_t)pRaagaConnectStage->sDramInterFrameBuffer.Buffer.offset +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32IfOffset;
                    /*  User Config buffer*/
                    psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress =
                                                    (dramaddr_t)pRaagaConnectStage->sDramUserConfigBuffer.Buffer.offset +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32UserCfgOffset;
                    /*  Status buffer*/
                    psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress =
                                                    (dramaddr_t)pRaagaConnectStage->sDramStatusBuffer.Buffer.offset +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32StatusOffset;

                }else if(ui32Node == 1)
                {
                    /*  Inter-Frame buffer */
                    psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramInterFrameBuffer.Buffer.offset;
                    /*  User Config buffer */
                    psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramUserConfigBuffer.Buffer.offset;
                    /*  Status buffer */
                    psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramStatusBuffer.Buffer.offset;
                }
                else
                {
                    BDBG_ERR(("Number of nodes more than 2 in the branch %d and stage %d which cannot happen", pRaagaConnectStage->ui32BranchId, pRaagaConnectStage->ui32StageId));
                    BDBG_ASSERT(0);
                }
                psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32InterFrmBuffSize;

                psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32UserCfgBuffSize;

                psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32StatusBuffSize;

                /*  ROM Table buffer */
                psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_TABLE(psNodeCfg->eAlgoId)].offset;
                psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_TABLE(psNodeCfg->eAlgoId)].size;

                /*  Num Src and destination for the node */
                psNodeCfg->ui32NumSrc = pRaagaConnectStage->totalInputs;/*inputs of all type*/
                psNodeCfg->ui32NumDst = pRaagaConnectStage->totalOutputs; /*Use the modified Dst o/ps*/

                /*The logic is in filling the node configuration for a stage which has more than one node, say decoder is:
                Both use the same IO Buffer and IO Gen Buffer as input and output respectively */

                pTempIoBuffer_Cached = NULL;
                pTempIoGenBuffer_Cached = NULL;
                /*  Input Configuration */
                for( ui32Ip=0; ui32Ip<BDSP_AF_P_MAX_IP_FORKS; ui32Ip++ )
                {
                    /*BDBG_ERR(("%s:%d - psNodeCfg->eNodeIpValidFlag[ui32Ip] %d", pRaagaConnectStage->sStageInput[ui32Ip].eNodeValid));*/
                    eNodeValid = pRaagaConnectStage->sStageInput[ui32Ip].eNodeValid;
                    psNodeCfg->eNodeIpValidFlag[ui32Ip] = eNodeValid;

                    if(eNodeValid)
                    {
                        pTempIoBuffer_Cached = (BDSP_AF_P_sIO_BUFFER *)pRaagaConnectStage->sStageInput[ui32Ip].IoBuffDesc.pAddr;
                        ui32IOPhysAddr = pRaagaConnectStage->sStageInput[ui32Ip].StageIOBuffDescAddr;

                        pTempIoGenBuffer_Cached = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pRaagaConnectStage->sStageInput[ui32Ip].IoGenBuffDesc.pAddr;
                        ui32IOGenPhysAddr = pRaagaConnectStage->sStageInput[ui32Ip].StageIOGenericBuffDescAddr;

                        switch (pRaagaConnectStage->sStageInput[ui32Ip].eConnectionType)
                        {
                            case BDSP_ConnectionType_eFmmBuffer:
                            case BDSP_ConnectionType_eRaveBuffer:
                            case BDSP_ConnectionType_eRDBBuffer:
                                pTempIoBuffer_Cached->eBufferType = pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.eBufferType;
                                pTempIoBuffer_Cached->ui32NumBuffers= pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.ui32NumBuffers;

                                for(i=0;i<pTempIoBuffer_Cached->ui32NumBuffers;i++)/*audio channels*/
                                {   /*ensure that the descriptors for FMM and RAVE that are passed are physical address*/
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32BaseAddr=pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.sCircBuffer[i].ui32BaseAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32EndAddr=pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.sCircBuffer[i].ui32EndAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32ReadAddr=pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.sCircBuffer[i].ui32ReadAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32WrapAddr=pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.sCircBuffer[i].ui32WrapAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32WriteAddr=pRaagaConnectStage->sStageInput[ui32Ip].IoBuffer.sCircBuffer[i].ui32WriteAddr;
                                }
                                BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageInput[ui32Ip].IoBuffDesc, sizeof(BDSP_AF_P_sIO_BUFFER));
                                BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageInput[ui32Ip].IoGenBuffDesc, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));

                                pTempIoGenBuffer_Cached = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pRaagaConnectStage->sIdsStageOutput.IoGenBuffDesc.pAddr;
                                ui32IOGenPhysAddr = pRaagaConnectStage->sIdsStageOutput.StageIOGenericBuffDescAddr;
                                BDBG_MSG(("FMM,RAVE,RDB i/p connection,ui32Ip=%d",ui32Ip));
                                break;
                            case BDSP_ConnectionType_eStage:
                                for (i = 0; i < pTempIoBuffer_Cached->ui32NumBuffers; i++)
                                {
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32BaseAddr
                                        = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOBuff.sCircBuffer[i].ui32BaseAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32EndAddr
                                        = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOBuff.sCircBuffer[i].ui32EndAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32ReadAddr
                                        = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOBuff.sCircBuffer[i].ui32ReadAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32WriteAddr
                                        = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOBuff.sCircBuffer[i].ui32WriteAddr;
                                    pTempIoBuffer_Cached->sCircBuffer[i].ui32WrapAddr
                                        = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOBuff.sCircBuffer[i].ui32WrapAddr;
                                }
                                pTempIoGenBuffer_Cached->sCircBuffer.ui32BaseAddr
                                    = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOGenericBuff.sCircBuffer.ui32BaseAddr;
                                pTempIoGenBuffer_Cached->sCircBuffer.ui32EndAddr
                                    = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOGenericBuff.sCircBuffer.ui32EndAddr;
                                pTempIoGenBuffer_Cached->sCircBuffer.ui32ReadAddr
                                    = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOGenericBuff.sCircBuffer.ui32ReadAddr;
                                pTempIoGenBuffer_Cached->sCircBuffer.ui32WriteAddr
                                    = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOGenericBuff.sCircBuffer.ui32WriteAddr;
                                pTempIoGenBuffer_Cached->sCircBuffer.ui32WrapAddr
                                    = pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][pRaagaConnectStage->ui32BranchId].IOGenericBuff.sCircBuffer.ui32WrapAddr;
                                BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageInput[ui32Ip].IoBuffDesc, sizeof(BDSP_AF_P_sIO_BUFFER));
                                BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageInput[ui32Ip].IoGenBuffDesc, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));
                                BDBG_MSG(("Stage Ip connection and Branch id of interstage=%d", pRaagaConnectStage->ui32BranchId));
                                break;
                            case BDSP_ConnectionType_eInterTaskBuffer:
                                /* Do nothing for inter task connection as the descriptors are populated
                                at create inter task buffer and inter task buffer flush */
                                break;
                            default:

                                BDBG_ERR(("ERROR: Invalid Connection type %d in BDSP_Raaga_P_FillNodeCfg",pRaagaConnectStage->sStageInput[ui32Ip].eConnectionType));
                                break;
                        }

                        BDBG_MSG(("ui32Ip= %d of a stage,pRaagaConnectStage->algorithm=%d,ui32IOPhysAddr =%x,ui32IOGenPhysAddr=%x ",ui32Ip, pRaagaConnectStage->algorithm, ui32IOPhysAddr, ui32IOGenPhysAddr));

                        psNodeCfg->ui32NodeIpBuffCfgAddr[ui32Ip] = ui32IOPhysAddr;
                        psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32Ip] = ui32IOGenPhysAddr;
                        /*psNodeCfg->ui32NodeIpBuffCfgAddr[ui32Ip] = (unsigned) pTempIoBuffer;*//*pRaagaConnectStage->sStageInput[ui32Ip].ui32StageIOBuffCfgAddr;*/

                    }
                }

                pTempIoBuffer_Cached = NULL;
                pTempIoGenBuffer_Cached = NULL;
                /* Output Configuration */
                /*Only for a decoder, this ui32Node is valid. For other stages, lets say SRC, node 0 is invalid and wont even enter this 'if'.*/
                if ((ui32Node == 0) && (ui32NumNodesInAlgo > 1))
                {
                    /* The output buffer descriptors for IDS (node 0 of a decode / mixer stage)
                    will always be the inter-stage buffer of branch 0 */
                    BDBG_MSG(("IDS of pRaagaConnectStage->algorithm=%d",pRaagaConnectStage->algorithm));
                    pTempIoBuffer_Cached = (BDSP_AF_P_sIO_BUFFER *)pRaagaConnectStage->sIdsStageOutput.IoBuffDesc.pAddr;
                    ui32IOPhysAddr = pRaagaConnectStage->sIdsStageOutput.StageIOBuffDescAddr;

                    pTempIoGenBuffer_Cached = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pRaagaConnectStage->sIdsStageOutput.IoGenBuffDesc.pAddr;
                    ui32IOGenPhysAddr = pRaagaConnectStage->sIdsStageOutput.StageIOGenericBuffDescAddr;

                    /* Output IO buffer descriptor population */
                    pTempIoBuffer_Cached->eBufferType = BDSP_AF_P_BufferType_eDRAM_IS;
                    pTempIoBuffer_Cached->ui32NumBuffers = 0;
                    pRaagaConnectStage->sIdsStageOutput.eNodeValid = BDSP_AF_P_eValid;
                    pRaagaConnectStage->sIdsStageOutput.eConnectionType = BDSP_ConnectionType_eStage;

                    /* Output IO Generic buffer descriptor population */
                    pTempIoGenBuffer_Cached->eBufferType = BDSP_AF_P_BufferType_eDRAM_IS;
                    pTempIoGenBuffer_Cached->ui32NumBuffers = 1;
                    pTempIoGenBuffer_Cached->sCircBuffer.ui32BaseAddr =
                            pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][0].IOGenericBuff.sCircBuffer.ui32BaseAddr;
                    pTempIoGenBuffer_Cached->sCircBuffer.ui32EndAddr  =
                                        pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][0].IOGenericBuff.sCircBuffer.ui32EndAddr;
                    pTempIoGenBuffer_Cached->sCircBuffer.ui32ReadAddr =
                                        pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][0].IOGenericBuff.sCircBuffer.ui32ReadAddr;
                    pTempIoGenBuffer_Cached->sCircBuffer.ui32WrapAddr =
                                        pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][0].IOGenericBuff.sCircBuffer.ui32WrapAddr;
                    pTempIoGenBuffer_Cached->sCircBuffer.ui32WriteAddr=
                                        pRaagaDevice->memInfo.sScratchandISBuff[dspIndex].InterStageIOGenericBuff[eSchedulingGroup][0].IOGenericBuff.sCircBuffer.ui32WriteAddr;

                    BDSP_MMA_P_FlushCache(pRaagaConnectStage->sIdsStageOutput.IoBuffDesc,sizeof(BDSP_AF_P_sIO_BUFFER));
                    BDSP_MMA_P_FlushCache(pRaagaConnectStage->sIdsStageOutput.IoGenBuffDesc,sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));
                    psNodeCfg->ui32NodeOpBuffCfgAddr[0] = ui32IOPhysAddr;
                    psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[0] = ui32IOGenPhysAddr;
                    psNodeCfg->eNodeOpBuffDataType[0] = BDSP_AF_P_DistinctOpType_eGenericIsData;

                    psNodeCfg->ui32NumDst = 1; /* Number of outputs from IDS node is always one */
                }
                else
                {
                    for( ui32Op=0; ui32Op<BDSP_AF_P_MAX_OP_FORKS; ui32Op++ )
                    {
                        eNodeValid = pRaagaConnectStage->sStageOutput[ui32Op].eNodeValid;
                        if(eNodeValid)
                        {
                            pTempIoBuffer_Cached = (BDSP_AF_P_sIO_BUFFER *)pRaagaConnectStage->sStageOutput[ui32Op].IoBuffDesc.pAddr;
                            ui32IOPhysAddr = pRaagaConnectStage->sStageOutput[ui32Op].StageIOBuffDescAddr;

                            pTempIoGenBuffer_Cached = (BDSP_AF_P_sIO_GENERIC_BUFFER *)pRaagaConnectStage->sStageOutput[ui32Op].IoGenBuffDesc.pAddr;
                            ui32IOGenPhysAddr = pRaagaConnectStage->sStageOutput[ui32Op].StageIOGenericBuffDescAddr;
                            switch (pRaagaConnectStage->sStageOutput[ui32Op].eConnectionType)
                            {
                                case BDSP_ConnectionType_eFmmBuffer:
                                case BDSP_ConnectionType_eRaveBuffer:
                                case BDSP_ConnectionType_eRDBBuffer:
                                    /*no IO Generic o/p to Rave(same for MDAL also - check ?)*/

                                    BKNI_Memset(pTempIoGenBuffer_Cached, 0, sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));

                                    if( BDSP_ConnectionType_eFmmBuffer == pRaagaConnectStage->sStageOutput[ui32Op].eConnectionType)
                                    {
                                        /*Detecting the Master /Slave port */
                                        /*
                                        The Algorithm is :
                                            The PI will not provide any information telling a buffer is Master or slave..

                                                Following are the Assumptions..
                                                    All slaves of a master port comes in a Distinct output as different output ports..
                                                    The first FMM output port of a distinct port is considered as Master and all other FMM ports are
                                                    considered as slave to the Master..

                                            CIT-gen counts the FMM ports in a Distinct output. If the number of FMM ports in a distinct output is >1,
                                            then the other FMM slave ports are present.

                                            If an FMM port is identified as Slave, them the Buffer type should be ....
                                        */
                                        if(0 != ui32FmmPortDstCount[pRaagaConnectStage->eStageOpBuffDataType[ui32Op]])
                                        {
                                            pTempIoBuffer_Cached->eBufferType = BDSP_AF_P_BufferType_eFMMSlave;
                                        }
                                        else
                                        {
                                            pTempIoBuffer_Cached->eBufferType = BDSP_AF_P_BufferType_eFMM;
                                        }
                                        ui32FmmPortDstCount[pRaagaConnectStage->eStageOpBuffDataType[ui32Op]]++;
                                    }
                                    else
                                    {
                                        pTempIoBuffer_Cached->eBufferType = pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.eBufferType;
                                    }

                                    pTempIoBuffer_Cached->ui32NumBuffers= pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.ui32NumBuffers;

                                    for(i=0;i<pTempIoBuffer_Cached->ui32NumBuffers;i++)/*audio channels*/
                                    {   /*ensure that the descriptors for FMM and RAVE that are passed are physical address*/
                                        pTempIoBuffer_Cached->sCircBuffer[i].ui32BaseAddr=pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.sCircBuffer[i].ui32BaseAddr;
                                        pTempIoBuffer_Cached->sCircBuffer[i].ui32EndAddr=pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.sCircBuffer[i].ui32EndAddr;
                                        pTempIoBuffer_Cached->sCircBuffer[i].ui32ReadAddr=pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.sCircBuffer[i].ui32ReadAddr;
                                        pTempIoBuffer_Cached->sCircBuffer[i].ui32WrapAddr=pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.sCircBuffer[i].ui32WrapAddr;
                                        pTempIoBuffer_Cached->sCircBuffer[i].ui32WriteAddr=pRaagaConnectStage->sStageOutput[ui32Op].IoBuffer.sCircBuffer[i].ui32WriteAddr;
                                    }
                                    BDBG_MSG(("FMM RAVE RDB output connection"));
                                    break;

                                case BDSP_ConnectionType_eInterTaskBuffer: /* Do nothing as descriptor is populated at inter task buffer create */
                                case BDSP_ConnectionType_eStage: /* Interstage buffer descriptors are shared and gets populated in Input configuration*/
                                    break;
                                default:
                                    BDBG_ERR(("ERROR: Invalid Connection type %d in BDSP_Raaga_P_FillNodeCfg",pRaagaConnectStage->sStageOutput[ui32Op].eConnectionType));
                                    break;
                            }
                            BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageOutput[ui32Op].IoBuffDesc,sizeof(BDSP_AF_P_sIO_BUFFER));
                            BDSP_MMA_P_FlushCache(pRaagaConnectStage->sStageOutput[ui32Op].IoGenBuffDesc,sizeof(BDSP_AF_P_sIO_GENERIC_BUFFER));
                            BDBG_MSG(("ui32Op= %d of a stage,pRaagaConnectStage->algorithm=%d, ui32IOPhysAddr=%x, ui32IOGenPhysAddr=%x",ui32Op, pRaagaConnectStage->algorithm, ui32IOPhysAddr, ui32IOGenPhysAddr));

                            psNodeCfg->ui32NodeOpBuffCfgAddr[ui32Op] = ui32IOPhysAddr;
                            psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32Op] = ui32IOGenPhysAddr;
                            psNodeCfg->eNodeOpBuffDataType[ui32Op] = pRaagaConnectStage->eStageOpBuffDataType[ui32Op];
                        }
                    }
                }

                psNodeCfg++;/*next node*/
                ui32NodeIndex++;
             }/*if node of a stage is valid*/
         }/*for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )*/

        /*Node config fill for a stage ends here. Traverse to the next stage*/
        for (i = 0; i < BDSP_AF_P_MAX_OP_FORKS; i++)
        {
            if ((pRaagaConnectStage->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eFmmBuffer)
                 && (pRaagaConnectStage->sStageOutput[i].eNodeValid == BDSP_AF_P_eValid))
            {
                collectResidue = false;
            }
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

     *ui32TotalNodes = ui32NodeIndex;

    BDBG_LEAVE(BDSP_Raaga_P_FillNodeCfg);
    return errCode;
}

/*  This function fills the global task configuration */
static BERR_Code BDSP_Raaga_P_FillAudioGblTaskCfg (
                        BDSP_RaagaTask *pRaagaTask,
                        BDSP_AF_P_sTASK_CONFIG  *psCit,
                        unsigned ui32TotalNodes)
{
    BERR_Code ui32Error = BERR_SUCCESS;
    int32_t  taskindex, index, index2;
    BDSP_CIT_P_sAlgoModePresent sAlgoModePresent;
    BDSP_AF_P_DolbyMsUsageMode  eDolbyMsUsageMode;
    BDSP_AF_P_sStcTrigConfig    sStcTrigConfig;
    BDSP_AF_P_sFW_HW_CFG        sFwHwCfg;
    BDSP_AF_P_sFMM_DEST_CFG     *psFmmDestCfg;
    BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG    *psTaskFmmGateOpenConfig;
    BDSP_TaskGateOpenSettings   sDependentTaskGateOpenSettings;
    BDSP_AF_P_sFMM_GATE_OPEN_CONFIG *psFmmGateOpenConfig;
    unsigned ui32ZeroFillSamples;
    BDSP_MMA_Memory TaskPortConfigAddr, TaskGateOpenConfigAddr, TaskFwHwCfgAddr;
    BDSP_MMA_Memory FwOpSamplingFreqMapLutAddr, StcTriggerCfgAddr;

    BDSP_TaskStartSettings *pStartSettings;
    BDSP_AF_P_sIO_BUFFER   *pIOBuffer;
    BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_RaagaContext *pRaagaContext;
    BDSP_Raaga *pRaagaDevice;

    BDSP_AF_P_sGLOBAL_TASK_CONFIG *psGblTaskCfg;
    BDSP_AF_P_eSchedulingGroup eSchedulingGroup = pRaagaTask->eSchedulingGroup;

    BDBG_ENTER(BDSP_Raaga_P_FillAudioGblTaskCfg);
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
    pRaagaContext = (BDSP_RaagaContext *)pRaagaPrimaryStage->pContext;
    pRaagaDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
    psGblTaskCfg = &psCit->sGlobalTaskConfig;

    pStartSettings = &pRaagaTask->startSettings;

    /*  Fill in the scratch buffer details */
    psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress = pRaagaDevice->memInfo.sScratchandISBuff[pRaagaTask->settings.dspIndex].DspScratchMemGrant[eSchedulingGroup].Buffer.offset;
    psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes = pRaagaDevice->memInfo.sScratchandISBuff[pRaagaTask->settings.dspIndex].DspScratchMemGrant[eSchedulingGroup].ui32Size;

    /*  Start node index */
    psGblTaskCfg->ui32StartNodeIndexOfCoreAudioAlgorithm = BDSP_CIT_P_NUM_SPECIAL_NODES;

    TaskPortConfigAddr         = pRaagaTask->taskMemGrants.sTaskCfgBufInfo.Buffer;

    TaskGateOpenConfigAddr     = TaskPortConfigAddr;
    TaskGateOpenConfigAddr.pAddr  =(void *)((uint8_t *)TaskGateOpenConfigAddr.pAddr + BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE);
    TaskGateOpenConfigAddr.offset = TaskGateOpenConfigAddr.offset + BDSP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE;

    TaskFwHwCfgAddr = TaskGateOpenConfigAddr;
    TaskFwHwCfgAddr.pAddr = (void *)((uint8_t *)TaskGateOpenConfigAddr.pAddr + BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG);
    TaskFwHwCfgAddr.offset = TaskFwHwCfgAddr.offset + BDSP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG;

    FwOpSamplingFreqMapLutAddr = TaskFwHwCfgAddr;
    FwOpSamplingFreqMapLutAddr.pAddr = (void *)((uint8_t *)FwOpSamplingFreqMapLutAddr.pAddr + BDSP_CIT_P_TASK_HW_FW_CONFIG);
    FwOpSamplingFreqMapLutAddr.offset = FwOpSamplingFreqMapLutAddr.offset + BDSP_CIT_P_TASK_HW_FW_CONFIG;

    StcTriggerCfgAddr = FwOpSamplingFreqMapLutAddr;
    StcTriggerCfgAddr.pAddr = (void *)((uint8_t *)StcTriggerCfgAddr.pAddr + BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE);
    StcTriggerCfgAddr.offset= StcTriggerCfgAddr.offset+ BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE;

    /*BDSP_CIT_P_TASK_STC_TRIG_CONFIG_SIZE;*/

    psTaskFmmGateOpenConfig = (BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG *)BKNI_Malloc(SIZEOF(BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG));
    if(NULL == psTaskFmmGateOpenConfig)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Couldn't Allocate Memory for Gate Open Config"));
        return BERR_OUT_OF_DEVICE_MEMORY;
    }

    psFmmDestCfg = (BDSP_AF_P_sFMM_DEST_CFG *)BKNI_Malloc(SIZEOF(BDSP_AF_P_sFMM_DEST_CFG)*BDSP_AF_P_MAX_NUM_PLLS);
    if(NULL == psFmmDestCfg)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Couldn't Allocate Memory for FMM DEST CONFIG"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }

    BDSP_P_InitializeFmmDstCfg(psFmmDestCfg);
    ui32Error = BDSP_MMA_P_CopyDataToDram(&TaskPortConfigAddr, (void *)psFmmDestCfg, (SIZEOF(BDSP_AF_P_sFMM_DEST_CFG)*BDSP_AF_P_MAX_NUM_PLLS));
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in copying the Initialised FMM settings for FMM Port"));
        goto end;
    }
    /* Add port Config and SPDIF Config */
    psGblTaskCfg->ui32FmmDestCfgAddr = TaskPortConfigAddr.offset;

    BKNI_Memset(psTaskFmmGateOpenConfig,0,sizeof(BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG));
    if(true == pStartSettings->gateOpenReqd)
    {
        pRaagaPrimaryStage->pRaagaTask = pRaagaTask;
        ui32Error = BDSP_Raaga_P_PopulateGateOpenFMMStages(
                                        (void *)pRaagaPrimaryStage,
                                        psTaskFmmGateOpenConfig,
                                        pStartSettings->maxIndependentDelay
                                    );
        if(ui32Error != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Populating the GateOpen Settings"));
            goto end;
        }
        if(pStartSettings->DependentTaskInfo.numTasks >= BDSP_MAX_DEPENDENT_TASK)
        {
            BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Total number of Dependent task to open their respective gates is %d exceeding limit %d !!!!!!!!!!",pStartSettings->DependentTaskInfo.numTasks, BDSP_MAX_DEPENDENT_TASK));
            ui32Error = BERR_INVALID_PARAMETER;
            goto end;
        }

        for(taskindex=0; taskindex<(int32_t)pStartSettings->DependentTaskInfo.numTasks; taskindex++)
        {
            BKNI_Memset(&sDependentTaskGateOpenSettings,0,sizeof(BDSP_TaskGateOpenSettings));
            sDependentTaskGateOpenSettings.psFmmGateOpenConfig = BKNI_Malloc(BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK* sizeof(BDSP_AF_P_sFMM_GATE_OPEN_CONFIG));
            if(NULL == sDependentTaskGateOpenSettings.psFmmGateOpenConfig)
            {
                BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Couldn't allocated memory for retreiving the FMM config of dependent task"));
                ui32Error = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
                goto end;
            }

            ui32Error = BDSP_Task_RetrieveGateOpenSettings( pStartSettings->DependentTaskInfo.DependentTask[taskindex],
                                                &sDependentTaskGateOpenSettings);
            if(ui32Error != BERR_SUCCESS)
            {
                BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Retreiving the Gate Open Settings for Dependenttask %d",taskindex));
                goto end;
            }
            if(sDependentTaskGateOpenSettings.ui32MaxIndepDelay != psTaskFmmGateOpenConfig->ui32MaxIndepDelay)
            {
                BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Different Max Independent Delay provided: For Dependent task is (%d) and For Gate Open Incharge Task is (%d)",
                    sDependentTaskGateOpenSettings.ui32MaxIndepDelay,
                    psTaskFmmGateOpenConfig->ui32MaxIndepDelay));
                ui32Error = BERR_INVALID_PARAMETER;
                goto end;
            }

            /* Modify the Addresses Returned to Raaga based Address*/
            psFmmGateOpenConfig = (BDSP_AF_P_sFMM_GATE_OPEN_CONFIG *)(sDependentTaskGateOpenSettings.psFmmGateOpenConfig);
            for(index = 0; index < (int32_t)sDependentTaskGateOpenSettings.ui32NumPorts; index++)
            {
                for(index2 = 0; index2 < (int32_t)BDSP_AF_P_MAX_CHANNELS; index2++)
                {
                    psFmmGateOpenConfig->uin32RingBufStartWrPointAddr[index2]=BDSP_RAAGA_REGSET_ADDR_FOR_DSP(psFmmGateOpenConfig->uin32RingBufStartWrPointAddr[index2]);
                }

                pIOBuffer =(BDSP_AF_P_sIO_BUFFER *)((BDSP_AF_P_sIO_BUFFER *)sDependentTaskGateOpenSettings.PortAddr.pAddr + index);
                for(index2=0;index2< (int32_t)pIOBuffer->ui32NumBuffers;index2++)/*audio channels*/
                {   /*ensure that the descriptors for FMM and RAVE that are passed are physical address*/
                   pIOBuffer->sCircBuffer[index2].ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pIOBuffer->sCircBuffer[index2].ui32BaseAddr);
                   pIOBuffer->sCircBuffer[index2].ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pIOBuffer->sCircBuffer[index2].ui32EndAddr);
                   pIOBuffer->sCircBuffer[index2].ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pIOBuffer->sCircBuffer[index2].ui32ReadAddr);
                   pIOBuffer->sCircBuffer[index2].ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pIOBuffer->sCircBuffer[index2].ui32WrapAddr);
                   pIOBuffer->sCircBuffer[index2].ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(pIOBuffer->sCircBuffer[index2].ui32WriteAddr);
                }
                psFmmGateOpenConfig++;
            }
            BDSP_MMA_P_FlushCache(sDependentTaskGateOpenSettings.PortAddr,(sDependentTaskGateOpenSettings.ui32NumPorts*sizeof(BDSP_AF_P_sIO_BUFFER)));

            if((psTaskFmmGateOpenConfig->ui32NumPorts + sDependentTaskGateOpenSettings.ui32NumPorts)> BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK)
            {
                BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Total number of FMM ports (%d) in the ecosystem exceeding the limit %d !!!!!!!!!!",
                    (psTaskFmmGateOpenConfig->ui32NumPorts + sDependentTaskGateOpenSettings.ui32NumPorts),
                    BDSP_AF_P_MAX_FMM_OP_PORTS_IN_TASK));
                BKNI_Free(sDependentTaskGateOpenSettings.psFmmGateOpenConfig);
                ui32Error = BERR_INVALID_PARAMETER;
                goto end;
            }
            else
            {
                BKNI_Memcpy((void *)&(psTaskFmmGateOpenConfig->sFmmGateOpenConfig[psTaskFmmGateOpenConfig->ui32NumPorts]),
                            sDependentTaskGateOpenSettings.psFmmGateOpenConfig,
                            (sDependentTaskGateOpenSettings.ui32NumPorts * sizeof(BDSP_AF_P_sFMM_GATE_OPEN_CONFIG)));
                psTaskFmmGateOpenConfig->ui32NumPorts += sDependentTaskGateOpenSettings.ui32NumPorts;
            }
            BKNI_Free(sDependentTaskGateOpenSettings.psFmmGateOpenConfig);
        }
    }
    else
    {
        BDBG_MSG(("BDSP_Raaga_P_FillAudioGblTaskCfg: Gate Open is turned OFF for this task"));
    }

    /*Adding Gate open */
    ui32Error = BDSP_MMA_P_CopyDataToDram(&TaskGateOpenConfigAddr, (void *)psTaskFmmGateOpenConfig, SIZEOF(BDSP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG));
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Copying the Gate Open Configurations to Firmware Offset"));
        goto end;
    }
    psGblTaskCfg->ui32FmmGateOpenConfigAddr = TaskGateOpenConfigAddr.offset;

    /*Populating FwHw starts here*/
    ui32Error = BDSP_Raaga_P_PopulateFwHwBuffer((void *)pRaagaPrimaryStage, &sFwHwCfg);
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Populating the FwHwBuffer"));
        goto end;
    }
    ui32Error = BDSP_MMA_P_CopyDataToDram(&TaskFwHwCfgAddr, (void *)&sFwHwCfg, SIZEOF(BDSP_AF_P_sFW_HW_CFG));
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in copying the FwHwBuffer to Firmware Offset"));
        goto end;
    }
    /* Add Fw Hw cfg address*/
    psGblTaskCfg->ui32TaskFwHwCfgAddr = TaskFwHwCfgAddr.offset;

    /*Populating LUTTable starts here*/
    /*First populate sAlgoModePresent, along with that get the number of nodes in Network also*/
    ui32Error = BDSP_Raaga_P_PopulateAlgoMode(pRaagaPrimaryStage, &sAlgoModePresent);
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Populating the Algorithm Mode"));
        goto end;
    }
    if (pStartSettings->pSampleRateMap)
    {
        /*Filling the Fw Op sampling map LUT */
        ui32Error = BDSP_MMA_P_CopyDataToDram(&FwOpSamplingFreqMapLutAddr,(void *)pStartSettings->pSampleRateMap, BDSP_CIT_P_TASK_FS_MAPPING_LUT_SIZE);
        if(ui32Error != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in Copying the SampleRate LUT provided by APE for Firmware"));
            goto end;
        }
    }
    else
    {
        /*No idea at all where this eDolbyMsUsageMode has to be initialized*/
        eDolbyMsUsageMode = BDSP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
        ui32Error = BDSP_P_FillSamplingFrequencyMapLut(eDolbyMsUsageMode, &sAlgoModePresent, &FwOpSamplingFreqMapLutAddr);
        if(ui32Error != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in filling the Samplerate LUT when APE doesnot provide the same"));
            goto end;
        }
    }

    /* Add FW Op Sampling Frequency Cfg*/
    psGblTaskCfg->ui32FwOpSamplingFreqMapLutAddr = FwOpSamplingFreqMapLutAddr.offset;

    /*First populate sStcTrigConfig*/
    BDSP_P_PopulateStcTrigConfig(&sStcTrigConfig, pStartSettings);

    ui32Error = BDSP_MMA_P_CopyDataToDram(&StcTriggerCfgAddr, (void *)&sStcTrigConfig, SIZEOF(BDSP_AF_P_sStcTrigConfig));
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in copying the STCTrigger configuration to firmware offset"));
        goto end;
    }

    /* STC trigger config address */
    psGblTaskCfg->ui32StcTrigConfigAddr = StcTriggerCfgAddr.offset;

    /* Finding the Zero Fill Samples  */  /*Need to check whether FW is using */
    ui32Error = BDSP_Raaga_P_GetNumZeroFillSamples(&ui32ZeroFillSamples, pRaagaPrimaryStage);
    if(ui32Error != BERR_SUCCESS)
    {
        BDBG_ERR(("BDSP_Raaga_P_FillAudioGblTaskCfg: Error in filling the Number of Zerofill samples"));
        goto end;
    }

    /* Zero Fill samples ::: Currently not used by FW */
    psGblTaskCfg->ui32NumberOfZeroFillSamples = ui32ZeroFillSamples;

    psGblTaskCfg->ui32NumberOfNodesInTask = ui32TotalNodes;
    psGblTaskCfg->eTimeBaseType = pRaagaTask->startSettings.timeBaseType;

end:
    BKNI_Free(psTaskFmmGateOpenConfig);
    BKNI_Free(psFmmDestCfg);

    BDBG_LEAVE(BDSP_Raaga_P_FillAudioGblTaskCfg);
    return ui32Error;;
}

static BERR_Code BDSP_Raaga_P_FillVideoDecodeGblTaskCfg (
                        BDSP_RaagaTask               *pRaagaTask,
                        BDSP_VF_P_sDEC_TASK_CONFIG   *psVideoDecodeCit,
                        BDSP_sVDecoderIPBuffCfg      *psVDecodeBuffCfgIp,
                        unsigned                      ui32TotalNodes)
{
    BERR_Code   ui32Error = BERR_SUCCESS;
    uint32_t    ui32Count, ui32RegOffset;

    BDSP_VF_P_sGLOBAL_TASK_CONFIG *psGblTaskCfg;
    BDSP_VF_P_sVDecodeBuffCfg     *psGlobalTaskConfigFromPI;

    BDBG_ENTER(BDSP_Raaga_P_FillVideoDecodeGblTaskCfg);

    ui32Error = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    psGblTaskCfg             = &psVideoDecodeCit->sGlobalTaskConfig;
    psGlobalTaskConfigFromPI = &psGblTaskCfg->sGlobalTaskConfigFromPI;

    /*Update the number if Nodes in the Task*/
    psGblTaskCfg->ui32NumberOfNodesInTask = ui32TotalNodes;

    /* Calculate the PDQ and PRQ Structure Addresses and
            Update it into The Global Config parameters*/
    /* PDQ - FIFO 15  values passed */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
    (ui32RegOffset * pRaagaTask->hPDQueue->i32FifoId);


    psGlobalTaskConfigFromPI->sPDQ.ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_BASE_OFFSET );
    psGlobalTaskConfigFromPI->sPDQ.ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );
    psGlobalTaskConfigFromPI->sPDQ.ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_WRITE_OFFSET );
    psGlobalTaskConfigFromPI->sPDQ.ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_READ_OFFSET );
    psGlobalTaskConfigFromPI->sPDQ.ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );

    /* PRQ - FIFO 16 values passed */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
    (ui32RegOffset * pRaagaTask->hPRQueue->i32FifoId);

    psGlobalTaskConfigFromPI->sPRQ.ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_BASE_OFFSET );
    psGlobalTaskConfigFromPI->sPRQ.ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );
    psGlobalTaskConfigFromPI->sPRQ.ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_WRITE_OFFSET );
    psGlobalTaskConfigFromPI->sPRQ.ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_READ_OFFSET );
    psGlobalTaskConfigFromPI->sPRQ.ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );


    /* Set the Reference Values to 720 */
    /* Currently hard coded but remove if PI provides proper configuration */
#if 0
    psGlobalTaskConfigFromPI->ui32MaxFrameWidth  = psVDecodeBuffCfgIp->MaxFrameWidth;
    psGlobalTaskConfigFromPI->ui32MaxFrameHeight = psVDecodeBuffCfgIp->MaxFrameHeight;
#else
    psGlobalTaskConfigFromPI->ui32MaxFrameWidth  = 720;
    psGlobalTaskConfigFromPI->ui32MaxFrameHeight = 576;
#endif  /* 0 */
    psGlobalTaskConfigFromPI->ui32StripeWidth    = pRaagaTask->pContext->pDevice->settings.memc[0].stripeWidth;

    /* DISPLAY FRAME BUFFER CONFIGURATION */
    psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.ui32NumBuffAvl =
                                            psVDecodeBuffCfgIp->sDisplayFrameBuffParams.ui32NumBuffAvl;

    /* Currently hard coded but remove if PI provides proper configuration */
#if 0
    psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.ui32ChromaStripeHeight
                            = psVDecodeBuffCfgIp->sDisplayFrameBuffParams.ui32ChromaStripeHeight;
    psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.ui32LumaStripeHeight
                            = psVDecodeBuffCfgIp->sDisplayFrameBuffParams.ui32LumaStripeHeight;
#else

    /*(CEILING((MaxVertSize/2)/16) x 16)*/
    /*(CEILING((576/2)/16) x 16) */
    psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.ui32ChromaStripeHeight= 288;

    /*(CEILING(MaxVertSize/16) x 16) */
    /*(CEILING(576/16) x 16) */
    psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.ui32LumaStripeHeight = 576;
#endif  /* 0 */

    for(ui32Count =0;ui32Count<BDSP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
    {
        /* Luma */
        psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress =
                        psVDecodeBuffCfgIp->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress;

        psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes =
                        psVDecodeBuffCfgIp->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes;

        /* Chroma */
        psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress =
                psVDecodeBuffCfgIp->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress;

        psGlobalTaskConfigFromPI->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes =
                        psVDecodeBuffCfgIp->sDisplayFrameBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes;

    }

    /* REFERENCE FRAME BUFFER CONFIGURATION */
        /* Currently hard coded but remove if PI provides proper configuration */
#if 0
    psGlobalTaskConfigFromPI->sReferenceBuffParams.ui32ChromaStripeHeight
                            = psVDecodeBuffCfgIp->sReferenceBuffParams.ui32ChromaStripeHeight;
    psGlobalTaskConfigFromPI->sReferenceBuffParams.ui32LumaStripeHeight
                            = psVDecodeBuffCfgIp->sReferenceBuffParams.ui32LumaStripeHeight;
#else
    /*TotalHorzPadd = 96, TotalVertPadd_Luma = 96, and TotalVertPadd_Chroma = 48*/
    /*(CEILING(((MaxVertSize/2) + TotalVertPadd_Chroma)/16) x 16)*/
    psGlobalTaskConfigFromPI->sReferenceBuffParams.ui32ChromaStripeHeight = 336;

    /*(CEILING((MaxVertSize + TotalVertPadd_Luma)/16) x 16)*/
    psGlobalTaskConfigFromPI->sReferenceBuffParams.ui32LumaStripeHeight = 672;
#endif /*0 */

    psGlobalTaskConfigFromPI->sReferenceBuffParams.ui32NumBuffAvl =
                                            psVDecodeBuffCfgIp->sReferenceBuffParams.ui32NumBuffAvl;

    for(ui32Count =0;ui32Count<BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;ui32Count++)
    {
        /* Luma */
        psGlobalTaskConfigFromPI->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress =
                        psVDecodeBuffCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress;

        psGlobalTaskConfigFromPI->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes =
                        psVDecodeBuffCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes;


        /* Chroma */
        psGlobalTaskConfigFromPI->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress =
                        psVDecodeBuffCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress;

        psGlobalTaskConfigFromPI->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes =
                        psVDecodeBuffCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes;
    }

    /* UPB BUFFER CONFIGURATION  */
    for(ui32Count =0;ui32Count<BDSP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
    {
        psGlobalTaskConfigFromPI->sUPBs[ui32Count].ui32DramBufferAddress =
                        psVDecodeBuffCfgIp->sUPBs[ui32Count].ui32DramBufferAddress;

        psGlobalTaskConfigFromPI->sUPBs[ui32Count].ui32BufferSizeInBytes =
                        psVDecodeBuffCfgIp->sUPBs[ui32Count].ui32BufferSizeInBytes;
    }

    BDBG_LEAVE(BDSP_Raaga_P_FillVideoDecodeGblTaskCfg);
    return ui32Error;
}

static BERR_Code BDSP_Raaga_P_FillInputforVideoEncode (BDSP_RaagaTask *pRaagaTask)
{
    BERR_Code  ui32Error = BERR_SUCCESS;
    uint32_t   ui32RegOffset;
    BDSP_RaagaStage *pRaagaPrimaryStage;

    BDBG_ENTER(BDSP_Raaga_P_FillInputforVideoEncode);

    ui32Error = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);
    pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;

    pRaagaPrimaryStage->sStageInput[0].eNodeValid              = BDSP_AF_P_eValid;
    pRaagaPrimaryStage->sStageInput[0].eConnectionType         = BDSP_ConnectionType_eRDBBuffer;
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.eBufferType    = BDSP_AF_P_BufferType_eRDB;
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.ui32NumBuffers = 3;

    /* PDQ - FIFO 15  values passed */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
    (ui32RegOffset * pRaagaTask->hRDQueue->i32FifoId);

    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[0].ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32RegOffset + BDSP_RAAGA_P_FIFO_BASE_OFFSET ) ;
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[0].ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[0].ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32RegOffset + BDSP_RAAGA_P_FIFO_WRITE_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[0].ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32RegOffset + BDSP_RAAGA_P_FIFO_READ_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[0].ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP( ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );

    /* PRQ - FIFO 16 values passed */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
    BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
    (ui32RegOffset * pRaagaTask->hRRQueue->i32FifoId);

    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[1].ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_BASE_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[1].ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[1].ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_WRITE_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[1].ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_READ_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[1].ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );


    /* CCDQ - FIFO 14  values passed */
    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_1_BASE_ADDR - \
        BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR;

    ui32RegOffset = BCHP_RAAGA_DSP_FW_CFG_FIFO_0_BASE_ADDR + \
    (ui32RegOffset * pRaagaTask->hCCDQueue->i32FifoId);

    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[2].ui32BaseAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_BASE_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[2].ui32EndAddr  = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[2].ui32WriteAddr= BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_WRITE_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[2].ui32ReadAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_READ_OFFSET );
    pRaagaPrimaryStage->sStageInput[0].IoBuffer.sCircBuffer[2].ui32WrapAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP(  ui32RegOffset + BDSP_RAAGA_P_FIFO_END_OFFSET );

    pRaagaPrimaryStage->totalInputs++;

    pRaagaPrimaryStage->sStageInput[0].StageIOBuffDescAddr = pRaagaPrimaryStage->sStageInput[0].IoBuffDesc.offset;
    pRaagaPrimaryStage->sStageInput[0].StageIOGenericBuffDescAddr = pRaagaPrimaryStage->sStageInput[0].IoGenBuffDesc.offset;

    BDBG_LEAVE(BDSP_Raaga_P_FillInputforVideoEncode);
    return ui32Error;
}

static BERR_Code BDSP_Raaga_P_FillVideoEncodeGblTaskCfg(
                        BDSP_RaagaTask              *pRaagaTask,
                        BDSP_VF_P_sENC_TASK_CONFIG  *psVideoEncodeCit,
                        BDSP_sVEncoderIPConfig      *psVEncoderCfgIp,
                        unsigned                     ui32TotalNodes)
{
    BERR_Code   ui32Error = BERR_SUCCESS;
    uint32_t    ui32Count;

    BDSP_VF_P_sENC_GLOBAL_TASK_CONFIG *psVideoEncodeGlobalTaskConfig;
    BDSP_VF_P_sVEncodeConfig          *psGlobalEncodeTaskConfig;

    BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_RaagaContext *pRaagaContext;
    BDSP_Raaga *pRaagaDevice;
    BDSP_MMA_Memory  vRrqAddr;
    unsigned int                    i=0;

#if (defined BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG)
    unsigned int                    uiDspIndex;
    unsigned int                    uiOffset;
    uint32_t                        ui32DramMap5AddrCfg;
#endif

    BDBG_ENTER(BDSP_Raaga_P_FillVideoEncodeGblTaskCfg);
    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    ui32Error = BERR_SUCCESS;
#if (defined BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG)
    uiDspIndex = 0;
    uiOffset = 0;
    ui32DramMap5AddrCfg = 0;
#endif
    pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
    pRaagaContext      = (BDSP_RaagaContext *)pRaagaPrimaryStage->pContext;
    pRaagaDevice       = (BDSP_Raaga *)pRaagaContext->pDevice;

    psVideoEncodeGlobalTaskConfig  = &psVideoEncodeCit->sEncGlobalTaskConfig;
    psGlobalEncodeTaskConfig       = &psVideoEncodeGlobalTaskConfig->sGlobalVideoEncoderConfig;

    psVideoEncodeGlobalTaskConfig->ui32NumberOfNodesInTask = ui32TotalNodes;

    /* Hook RDQ/RRQ to CIT's branch input */

    /* From base Address of pRaagaTask->hRRQueue fill all the buffers. Fill RRQ in the beginning */
    /* This conversion is done to pass a virtual address as the second argument
        of BDSP_Raaga_P_MemWrite32 */
    vRrqAddr = pRaagaTask->hRRQueue->Memory;
    for(i = 0; i < BDSP_FWMAX_VIDEO_BUFF_AVAIL; i++)
    {
        ui32Error = BDSP_MMA_P_MemWrite32(&vRrqAddr, psVEncoderCfgIp->sPPBs[i].ui32DramBufferAddress);
        if(ui32Error != BERR_SUCCESS)
        {
            BDBG_ERR(("BDSP_Raaga_P_FillVideoEncodeGblTaskCfg: Error in Updating the PPB Dram Buffer Address"));
            goto end;
        }
        vRrqAddr.pAddr = (void *)((uint8_t *)vRrqAddr.pAddr + 4);
    }
    /* Common Parameters */
    psGlobalEncodeTaskConfig->ui32MaxFrameHeight = psVEncoderCfgIp->MaxFrameHeight;
    psGlobalEncodeTaskConfig->ui32MaxFrameWidth  = psVEncoderCfgIp->MaxFrameWidth;
    psGlobalEncodeTaskConfig->ui32StripeWidth    = pRaagaDevice->settings.memc[0].stripeWidth;
#if (defined BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG)
    if ( pRaagaDevice->settings.memc[0].stripeWidth == 128 )
    {
        ui32DramMap5AddrCfg =0x00080005;
    }
    else if ( pRaagaDevice->settings.memc[0].stripeWidth == 256 )
    {
        ui32DramMap5AddrCfg =0x00090005;
    }

    for (uiDspIndex =0 ; uiDspIndex < pRaagaDevice->numDsp; uiDspIndex++)
    {
            uiOffset = pRaagaDevice->dspOffset[uiDspIndex];
#ifdef BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG
            BDSP_Write32(pRaagaDevice->regHandle, BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG + uiOffset,
                            ui32DramMap5AddrCfg);
#endif
#ifdef BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG
            BDSP_Write32(pRaagaDevice->regHandle, BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG + uiOffset,
                                ui32DramMap5AddrCfg);
#endif
#ifdef BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG
            BDSP_Write32(pRaagaDevice->regHandle, BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG + uiOffset,
                                ui32DramMap5AddrCfg);
#endif

#if 0
#ifdef BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_MASK
            ui32RegVal = BDSP_Read32(pRaagaDevice->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0 + uiOffset);\
            ui32RegVal = ui32RegVal & ( ~(BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_MASK) );
            ui32RegVal |= (5 << BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0_MDIV_CH0_SHIFT);
            BDSP_Write32(pRaagaDevice->regHandle, BCHP_CLKGEN_PLL_RAAGA_PLL_CHANNEL_CTRL_CH_0 + uiOffset, ui32RegVal);


#endif
#ifdef BCHP_MEMC_ARB_0_CLIENT_INFO_55_RR_EN_MASK
            ui32RegVal = BDSP_Read32(pRaagaDevice->regHandle, BCHP_MEMC_ARB_0_CLIENT_INFO_55 + uiOffset);
            ui32RegVal |= (BCHP_MEMC_ARB_0_CLIENT_INFO_55_RR_EN_ENABLED << BCHP_MEMC_ARB_0_CLIENT_INFO_55_RR_EN_SHIFT);
            BDSP_Write32(pRaagaDevice->regHandle, BCHP_MEMC_ARB_0_CLIENT_INFO_55 + uiOffset, ui32RegVal);


#endif
#ifdef BCHP_MEMC_ARB_0_CLIENT_INFO_127_RR_EN_MASK
                ui32RegVal = BDSP_Read32(pRaagaDevice->regHandle, BCHP_MEMC_ARB_0_CLIENT_INFO_127 + uiOffset);
                ui32RegVal |= (BCHP_MEMC_ARB_0_CLIENT_INFO_127_RR_EN_ENABLED << BCHP_MEMC_ARB_0_CLIENT_INFO_127_RR_EN_SHIFT);
                BDSP_Write32(pRaagaDevice->regHandle, BCHP_MEMC_ARB_0_CLIENT_INFO_127 + uiOffset, ui32RegVal);
#endif
#endif
    }
#endif /*(defined BCHP_RAAGA_DSP_DMA_SCB0_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB1_DRAM_MAP5_ADDR_CFG || defined BCHP_RAAGA_DSP_DMA_SCB2_DRAM_MAP5_ADDR_CFG)*/

    /* REFERENCE FRAME BUFFER SETTINGS */
    psGlobalEncodeTaskConfig->sReferenceBuffParams.ui32NumBuffAvl = psVEncoderCfgIp->sReferenceBuffParams.ui32NumBuffAvl;

    /*TotalHorzPadd = 96, TotalVertPadd_Luma = 96, and TotalVertPadd_Chroma = 48*/
    /*(CEILING(((MaxVertSize/2) + TotalVertPadd_Chroma)/16) x 16)*/
    psGlobalEncodeTaskConfig->sReferenceBuffParams.ui32LumaStripeHeight = psVEncoderCfgIp->sReferenceBuffParams.ui32LumaStripeHeight;

    /*(CEILING((MaxVertSize + TotalVertPadd_Luma)/16) x 16)*/
    psGlobalEncodeTaskConfig->sReferenceBuffParams.ui32ChromaStripeHeight = psVEncoderCfgIp->sReferenceBuffParams.ui32ChromaStripeHeight;

    /* Reference Frame Buffers */
    for(ui32Count =0;ui32Count<BDSP_FWMAX_VIDEO_REF_BUFF_AVAIL;ui32Count++)
    {
        /* Luma */
        psGlobalEncodeTaskConfig->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress =
            psVEncoderCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress;

        psGlobalEncodeTaskConfig->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes =
                        psVEncoderCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes;

        /* Chroma */
                psGlobalEncodeTaskConfig->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress =
                    psVEncoderCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress;

        psGlobalEncodeTaskConfig->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes =
                        psVEncoderCfgIp->sReferenceBuffParams.sBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes;
    }

    /* sPPBs */
    for(ui32Count =0;ui32Count<BDSP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
    {
        psGlobalEncodeTaskConfig->sPPBs[ui32Count].ui32DramBufferAddress =
                        psVEncoderCfgIp->sPPBs[ui32Count].ui32DramBufferAddress;

        psGlobalEncodeTaskConfig->sPPBs[ui32Count].ui32BufferSizeInBytes =
                        psVEncoderCfgIp->sPPBs[ui32Count].ui32BufferSizeInBytes;
    }

    psGlobalEncodeTaskConfig->sEncoderParams.ui32Frames2Accum = psVEncoderCfgIp->sEncoderParams.ui32Frames2Accum;
    psGlobalEncodeTaskConfig->sEncoderParams.IsGoBitInterruptEnabled =
                                                psVEncoderCfgIp->sEncoderParams.IsGoBitInterruptEnabled;
    psGlobalEncodeTaskConfig->sEncoderParams.eEncodeFrameRate = psVEncoderCfgIp->sEncoderParams.eEncodeFrameRate;

    psGlobalEncodeTaskConfig->sEncoderParams.ui32InterruptBit[0] = psVEncoderCfgIp->sEncoderParams.ui32InterruptBit[0];
    psGlobalEncodeTaskConfig->sEncoderParams.ui32InterruptBit[1] = psVEncoderCfgIp->sEncoderParams.ui32InterruptBit[1];

    psGlobalEncodeTaskConfig->sEncoderParams.ui32StcAddr = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((psVEncoderCfgIp->sEncoderParams.ui32StcAddr));
    psGlobalEncodeTaskConfig->sEncoderParams.ui32StcAddr_hi = BDSP_RAAGA_REGSET_ADDR_FOR_DSP((psVEncoderCfgIp->sEncoderParams.ui32StcAddr_hi));

    /* We need to send the RDQ and RRQ's DRAM address too in global task configuration */
    psGlobalEncodeTaskConfig->sRawDataQueues.ui32DramBufferAddress
                = psVideoEncodeCit->sNodeConfig[0].ui32NodeIpBuffCfgAddr[0];
    psGlobalEncodeTaskConfig->sRawDataQueues.ui32BufferSizeInBytes
                = SIZEOF(BDSP_AF_P_sIO_BUFFER);

end:
    BDBG_LEAVE(BDSP_Raaga_P_FillVideoEncodeGblTaskCfg);
    return ui32Error;
}

static BERR_Code BDSP_Raaga_P_FillNodeCfgIntoNewScmCit(
                    BDSP_RaagaStage *pPrimaryStageHandle,
                    BDSP_SCM_P_sTASK_CONFIG         *psCit,
                    unsigned *ui32TotalNodes)
{
    BERR_Code   errCode = BERR_SUCCESS;
    uint32_t    ui32Node;
    uint32_t    ui32NumNodesInAlgo;
    uint32_t    ui32NodeIndex;
    unsigned    i;
    BDSP_AF_P_sNODE_CONFIG  *psNodeCfg;
    bool collectResidue;

    const BDSP_Raaga_P_AlgorithmInfo *sAlgoInfo;

    BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_RaagaContext *pRaagaContext;

    BDSP_Raaga *pRaagaDevice;

    BDBG_ENTER(BDSP_Raaga_P_FillNodeCfgIntoNewScmCit);

    errCode = BERR_SUCCESS;

    pRaagaPrimaryStage = (BDSP_RaagaStage *)pPrimaryStageHandle;
    BDBG_ASSERT(NULL != pRaagaPrimaryStage->pContext);
    pRaagaContext = (BDSP_RaagaContext *)pRaagaPrimaryStage->pContext;
    BDBG_ASSERT(NULL != pRaagaContext->pDevice);
    pRaagaDevice = (BDSP_Raaga *)pRaagaContext->pDevice;

    ui32NodeIndex=0;

    /*  Update the CIT nodes based on execution order */
    psNodeCfg = psCit->sNodeConfig;

    collectResidue = true;

    /*Now, traverse through all the stages. Hold back the stage forking for later traversing.
    The assumption is that the stage o/p connection handle will be NULL if it has no o/p interstage connection */
    BDSP_STAGE_TRAVERSE_LOOP_BEGIN(pRaagaPrimaryStage, pRaagaConnectStage)
    BSTD_UNUSED(macroBrId);
    BSTD_UNUSED(macroStId);
    {

        sAlgoInfo = BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->algorithm);

        ui32NumNodesInAlgo = sAlgoInfo->algoExecInfo.NumNodes;
            BDBG_MSG((" algo id = %d should be - %d numnodes - %d",pRaagaConnectStage->algorithm, BDSP_Algorithm_eSecurityC ,ui32NumNodesInAlgo ));

        for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
        {
            if(sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node] != BDSP_AF_P_AlgoId_eInvalid )
            {
                psNodeCfg->uiNodeId = ui32NodeIndex;/*increment to next node*/
                /*Populating the Collect Residual Flag */
                /*Branch Id is populated during the stage traverse to Download FW execs. Use it now.*/
                psNodeCfg->eCollectResidual = (collectResidue) ? BDSP_AF_P_eEnable : BDSP_AF_P_eDisable;
                BDBG_MSG(("Collect Residue [%s] = %d", (BDSP_Raaga_P_LookupAlgorithmInfo(pRaagaConnectStage->algorithm))->pName, collectResidue));

                psNodeCfg->eAlgoId = sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node];

                /* Audio Algorithm Type */
                psNodeCfg->ui32AudioAlgorithmType = sAlgoInfo->algorithm;

                switch(sAlgoInfo->algorithm)
                {
                case BDSP_Algorithm_eSecurityA:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_ScmAlgoType_eScm1;
                        break;
                case BDSP_Algorithm_eSecurityB:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_ScmAlgoType_eScm2;
                        break;
                case BDSP_Algorithm_eSecurityC:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_ScmAlgoType_eScm3;
                        break;
                    default:
                        psNodeCfg->ui32AudioAlgorithmType = BDSP_AF_P_ScmAlgoType_eScm3;
                        break;
                }

                BDBG_MSG(("ui32NodeIndex=%d", ui32NodeIndex));
                BDBG_MSG(("sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node]=%d, ui32Node=%d", sAlgoInfo->algoExecInfo.eAlgoIds[ui32Node], ui32Node));


                /*  Code Buffer */
                psNodeCfg->ui32VomAlgoAddr =
                                                BDSP_sAlgoStartAddr.sVomAlgoStartAddr[psNodeCfg->eAlgoId];
                psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_CODE(psNodeCfg->eAlgoId)].offset;
                psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_CODE(psNodeCfg->eAlgoId)].size;

                if(ui32Node==0)
                {
                    /*  Inter-Frame buffer */
                    psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramInterFrameBuffer.Buffer.offset +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32IfOffset;
                    /*  User Config buffer*/
                    psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramUserConfigBuffer.Buffer.offset  +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32UserCfgOffset;
                    /*  Statusbuffer*/
                    psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramStatusBuffer.Buffer.offset  +
                                                    pRaagaConnectStage->sFrameSyncOffset.ui32StatusOffset;
                }else if(ui32Node==1)
                {
                    /*  Inter-Frame buffer */
                    psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramInterFrameBuffer.Buffer.offset;
                    /*  User Config buffer */
                    psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramUserConfigBuffer.Buffer.offset;
                    /*  Status buffer */
                    psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress =
                                                    pRaagaConnectStage->sDramStatusBuffer.Buffer.offset;
                }
                else
                {
                    BDBG_ERR(("Number of nodes more than 2 in the branch %d and stage %d which cannot happen", pRaagaConnectStage->ui32BranchId, pRaagaConnectStage->ui32StageId));
                    BDBG_ASSERT(0);
                }

                psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32InterFrmBuffSize;

                psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32UserCfgBuffSize;

                psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes =
                                                BDSP_sNodeInfo[psNodeCfg->eAlgoId].ui32StatusBuffSize;

                /*  ROM Table buffer */
                psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_TABLE(psNodeCfg->eAlgoId)].offset;
                psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes =
                                                pRaagaDevice->imgCache[BDSP_IMG_ID_TABLE(psNodeCfg->eAlgoId)].size;

                /*  Num Src and destination for the node */
                psNodeCfg->ui32NumSrc = pRaagaConnectStage->totalInputs;/*inputs of all type*/
                psNodeCfg->ui32NumDst = pRaagaConnectStage->totalOutputs; /*Use the modified Dst o/ps*/

                psNodeCfg++;/*next node*/
                ui32NodeIndex++;
             }/*if node of a stage is valid*/
        }/*for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )*/

        /*Node config fill for a stage ends here. Traverse to the next stage*/
        for (i = 0; i < BDSP_AF_P_MAX_OP_FORKS; i++)
        {
            if ((pRaagaConnectStage->sStageOutput[i].eConnectionType == BDSP_ConnectionType_eFmmBuffer)
                 && (pRaagaConnectStage->sStageOutput[i].eNodeValid == BDSP_AF_P_eValid))
            {
                collectResidue = false;
            }
        }
    }
    BDSP_STAGE_TRAVERSE_LOOP_END(pRaagaConnectStage)

     *ui32TotalNodes = ui32NodeIndex;

    BDBG_LEAVE(BDSP_Raaga_P_FillNodeCfgIntoNewScmCit);
    return errCode;
}

static BERR_Code BDSP_Raaga_P_FillScmGblTaskCfg(
                        BDSP_RaagaTask *pRaagaTask,
                        BDSP_SCM_P_sTASK_CONFIG *psCit,
                        unsigned ui32TotalNodes)

{
    BERR_Code    ui32Error = BERR_SUCCESS;
    BDSP_CIT_P_sAlgoModePresent sAlgoModePresent;

    BDSP_RaagaStage *pRaagaPrimaryStage;
    BDSP_RaagaContext *pRaagaContext;

    BDSP_Raaga *pRaagaDevice;

    BDSP_SCM_P_sGLOBAL_TASK_CONFIG *psGblTaskCfg;
    BDSP_AF_P_eSchedulingGroup eSchedulingGroup = pRaagaTask->eSchedulingGroup;

    BDBG_ENTER(BDSP_Raaga_P_FillScmGblTaskCfg);

    ui32Error = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(pRaagaTask, BDSP_RaagaTask);

    pRaagaPrimaryStage = (BDSP_RaagaStage *)pRaagaTask->startSettings.primaryStage->pStageHandle;
    pRaagaContext = (BDSP_RaagaContext *)pRaagaPrimaryStage->pContext;
    pRaagaDevice = (BDSP_Raaga *)pRaagaContext->pDevice;
    psGblTaskCfg = &psCit->sGlobalTaskConfig;

    /*  Fill in the scratch buffer details */
    psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress = pRaagaDevice->memInfo.sScratchandISBuff[pRaagaTask->settings.dspIndex].DspScratchMemGrant[eSchedulingGroup].Buffer.offset;
    psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes = pRaagaDevice->memInfo.sScratchandISBuff[pRaagaTask->settings.dspIndex].DspScratchMemGrant[eSchedulingGroup].ui32Size;
    /*  Start node index */
    psGblTaskCfg->ui32StartNodeIndexOfCoreScmAlgo = BDSP_CIT_P_NODE0;

    /*First populate sAlgoModePresent, along with that get the number of nodes in Network also*/
    BDSP_Raaga_P_PopulateAlgoMode(pRaagaPrimaryStage, &sAlgoModePresent);

    psGblTaskCfg->ui32NumberOfNodesInTask = ui32TotalNodes;

    BDBG_LEAVE(BDSP_Raaga_P_FillScmGblTaskCfg);
    return ui32Error;
}

BERR_Code BDSP_Raaga_P_GenCit(void* pTaskHandle)
{
    BERR_Code ui32Err = BERR_SUCCESS ;

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_CIT_P_Output   *psCitOp = &(pRaagaTask->citOutput);

    BDSP_CIT_P_sTaskBuffInfo sTaskBuffInfo;
    unsigned ui32TotalNodes = 0;

    BDBG_ENTER(BDSP_Raaga_P_GenCit);

    BDSP_Raaga_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

    BDBG_ASSERT(NULL != pRaagaTask);
    BDBG_ASSERT(NULL != pRaagaTask->startSettings.primaryStage);

    ui32Err = BDSP_Raaga_P_FillNodeCfg(
                    (void *) pRaagaTask->startSettings.primaryStage->pStageHandle,
                    pRaagaTask->settings.dspIndex,
                    &psCitOp->sCit.sNodeConfig[0]/*BDSP_AF_P_sTASK_CONFIG*/ ,
                    &ui32TotalNodes,
                    pRaagaTask->eSchedulingGroup);

    if( ui32Err != BERR_SUCCESS || ui32TotalNodes == 0)
    {
        goto BDSP_CITGENMODULE_P_EXIT_POINT;
    }

    if(ui32TotalNodes > BDSP_AF_P_MAX_NODES)
    {
        BDBG_ERR(("Error : The number of nodes in the system is %d. Maximum Allowed is %d", ui32TotalNodes,BDSP_AF_P_MAX_NODES));
        return(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("ui32TotalNodes in Network = %d", ui32TotalNodes));
    /*  Fill the global task configuration into CIT */
    ui32Err = BDSP_Raaga_P_FillAudioGblTaskCfg(
                            (void *) pRaagaTask,
                            &psCitOp->sCit,
                            ui32TotalNodes);
BDSP_CITGENMODULE_P_EXIT_POINT:
    if(ui32Err !=BERR_SUCCESS)
    {
    BDBG_ERR(("Error in Filling the CIT for AUDIO"));
        BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BDSP_Raaga_P_GenCit);

    return ui32Err;
}

BERR_Code BDSP_Raaga_P_GenVideoCit( void            *pTaskHandle,
                                            BDSP_AlgorithmType   eAlgorithm)
{
    BERR_Code ui32Err = BERR_SUCCESS ;

    BDSP_RaagaTask            *pRaagaTask      = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_CIT_P_VideoCITOutput *psVideoCitOp    = &(pRaagaTask->videoCitOutput);
    BDSP_RaagaStage *pPrimaryStageHandle;

    BDSP_CIT_P_sTaskBuffInfo sTaskBuffInfo;
    unsigned ui32TotalNodes = 0;

    BDBG_ENTER(BDSP_Raaga_P_GenVideoCit);

    BDSP_Raaga_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

    BDBG_ASSERT(NULL != pRaagaTask);
    BDBG_ASSERT(NULL != pRaagaTask->startSettings.primaryStage);

    pPrimaryStageHandle = (BDSP_RaagaStage *) pRaagaTask->startSettings.primaryStage->pStageHandle;

    if(BDSP_AlgorithmType_eVideoDecode == eAlgorithm)
    {
        ui32Err = BDSP_Raaga_P_FillNodeCfg(
                        pPrimaryStageHandle,
                        pRaagaTask->settings.dspIndex,
                        &psVideoCitOp->uVideoCit.sVideoDecTaskConfig.sNodeConfig[0],
                        &ui32TotalNodes,
                        pRaagaTask->eSchedulingGroup);
    }
    else
    {
        /* Fill IO Buffer Explicitly for Video Encode case with the HRD , HRR and CCD Queue */
        ui32Err |= BDSP_Raaga_P_FillInputforVideoEncode(pRaagaTask);

        ui32Err |= BDSP_Raaga_P_FillNodeCfg(
                        pPrimaryStageHandle,
                        pRaagaTask->settings.dspIndex,
                        &psVideoCitOp->uVideoCit.sVideoEncTaskConfig.sNodeConfig[0],
                        &ui32TotalNodes,
                        pRaagaTask->eSchedulingGroup);
    }
    if( ui32Err != BERR_SUCCESS)
    {
        goto BDSP_VIDEOCITGENMODULE_P_EXIT_POINT;
    }

    if(ui32TotalNodes > BDSP_AF_P_MAX_NODES)
    {
        BDBG_ERR(("Error : The number of nodes in the system is %d. Maximum Allowed is %d", ui32TotalNodes,BDSP_AF_P_MAX_NODES));
        return(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("ui32TotalNodes in Network = %d", ui32TotalNodes));

    /*  Fill the global task configuration into CIT */
    if(BDSP_AlgorithmType_eVideoDecode == eAlgorithm)
    {
        ui32Err = BDSP_Raaga_P_FillVideoDecodeGblTaskCfg(
                                (void *)pRaagaTask,
                                &psVideoCitOp->uVideoCit.sVideoDecTaskConfig,/*BDSP_VF_P_sDEC_TASK_CONFIG*/
                                (BDSP_sVDecoderIPBuffCfg *)pRaagaTask->startSettings.psVDecoderIPBuffCfg,
                                ui32TotalNodes);
    }
    else
    {
        ui32Err = BDSP_Raaga_P_FillVideoEncodeGblTaskCfg(
                                (void *)pRaagaTask,
                                &psVideoCitOp->uVideoCit.sVideoEncTaskConfig,/*BDSP_VF_P_sENC_TASK_CONFIGs*/
                                (BDSP_sVEncoderIPConfig *)pRaagaTask->startSettings.psVEncoderIPConfig,
                                ui32TotalNodes);
    }
BDSP_VIDEOCITGENMODULE_P_EXIT_POINT:
    if(ui32Err != BERR_SUCCESS)
    {
        BDBG_ERR(("Error in Filling the CIT for VIDEO"));
        BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BDSP_Raaga_P_GenVideoCit);

    return ui32Err;
}

BERR_Code BDSP_Raaga_P_GenScmCit( void* pTaskHandle )
{
    BERR_Code ui32Err = BERR_SUCCESS ;

    BDSP_RaagaTask *pRaagaTask = (BDSP_RaagaTask *)pTaskHandle;
    BDSP_CIT_P_ScmCITOutput     *psScmCitOp = &(pRaagaTask->scmCitOutput);

    BDSP_CIT_P_sTaskBuffInfo sTaskBuffInfo;
    unsigned ui32TotalNodes = 0;

    BDBG_ENTER(BDSP_Raaga_P_GenScmCit);

    BDSP_Raaga_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

    BDBG_ASSERT(NULL != pRaagaTask);
    BDBG_ASSERT(NULL != pRaagaTask->startSettings.primaryStage);

    ui32Err = BDSP_Raaga_P_FillNodeCfgIntoNewScmCit(
                    (void *) pRaagaTask->startSettings.primaryStage->pStageHandle,
                    &psScmCitOp->sScmCit,
                    &ui32TotalNodes);

    if( ui32Err != BERR_SUCCESS || ui32TotalNodes == 0)
    {
        goto BDSP_CITGENMODULE_P_EXIT_POINT;
    }
    if(ui32TotalNodes > BDSP_AF_P_MAX_NODES)
    {
        BDBG_ERR(("Error : The number of nodes in the system is %d. Maximum Allowed is %d", ui32TotalNodes,BDSP_AF_P_MAX_NODES));
        return(BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("ui32TotalNodes in Network = %d", ui32TotalNodes));

    /*  Fill the global task configuration into CIT */
    ui32Err = BDSP_Raaga_P_FillScmGblTaskCfg(
                    (void *) pRaagaTask,
                    &psScmCitOp->sScmCit,
                    ui32TotalNodes);

BDSP_CITGENMODULE_P_EXIT_POINT:

    /* Check for Error and assert */
    if(ui32Err !=BERR_SUCCESS)
    {
    BDBG_ERR(("Error in Filling the CIT for SCM"));
        BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BDSP_Raaga_P_GenScmCit);

    return ui32Err;
}
