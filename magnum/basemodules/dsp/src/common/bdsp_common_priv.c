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
 ******************************************************************************/

#include "bdsp_common_priv.h"

BDBG_MODULE(bdsp_common_priv);

void BDSP_P_GetDistinctOpTypeAndNumChans(
    BDSP_DataType dataType, /* [in] */
    unsigned *numChans, /* [out] */
    BDSP_AF_P_DistinctOpType *distinctOp /* [out] */
    )
{
    switch(dataType)
    {
        case BDSP_DataType_ePcmMono:
            *distinctOp = BDSP_AF_P_DistinctOpType_eMono_PCM;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937x16:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressedHBR;
            *numChans = 1;
            break;
        case BDSP_DataType_eIec61937x4:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed4x;
            *numChans = 1;
            break;
        case BDSP_DataType_eCompressedRaw:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcmRf:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCompressed4x;
            *numChans = 1;
            break;
        case BDSP_DataType_eRave:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcm5_1:
            *distinctOp = BDSP_AF_P_DistinctOpType_e5_1_PCM;
            *numChans = 6;
            break;
        case BDSP_DataType_eDolbyTranscodeData:
            *distinctOp = BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut;
#if (BDSP_MS12_SUPPORT == 65)
            *numChans = 8;
#else
			*numChans = 6;
#endif
            break;
        case BDSP_DataType_ePcm7_1:
            *distinctOp = BDSP_AF_P_DistinctOpType_e7_1_PCM;
            *numChans = 8;
            break;
        case BDSP_DataType_eRdbCdb:
            *distinctOp = BDSP_AF_P_DistinctOpType_eCdb;
            *numChans = 1;
            break;
        case BDSP_DataType_eRdbItb:
            *distinctOp = BDSP_AF_P_DistinctOpType_eItb;
            *numChans = 1;
            break;
        case BDSP_DataType_eRdbAnc:
            *distinctOp = BDSP_AF_P_DistinctOpType_eAncillaryData;
            *numChans = 1;
            break;
        case BDSP_DataType_eRDBPool:
            *distinctOp = BDSP_AF_P_DistinctOpType_eDescriptorQueue;
            *numChans = 1;
            break;
        case BDSP_DataType_ePcmStereo:
        default:
            *distinctOp = BDSP_AF_P_DistinctOpType_eStereo_PCM;
            *numChans = 2;
            break;
    }

    return;
}

/***********************************************************************
Name        :   BDSP_Raaga_P_GetFreeInputPortIndex

Type        :   BDSP Internal

Input       :   psStageInput      -   Pointer of the StageInput details for which we need to return the Input port index
                index              -    Index returned back.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the first free input index for the stage and return the same to the caller.
***********************************************************************/

void BDSP_P_GetFreeInputPortIndex(BDSP_StageSrcDstDetails *psStageInput, unsigned *index)
{
    unsigned i;
    BDSP_StageSrcDstDetails *psStageSrcDstDetails = psStageInput;

    for (i = 0; i < BDSP_AF_P_MAX_IP_FORKS; i++)
    {
        if (psStageSrcDstDetails->eNodeValid != BDSP_AF_P_eValid)
        {
            break;
        }
        psStageSrcDstDetails++;
    }

    *index = i;
}


/***********************************************************************
Name        :   BDSP_P_GetFreeOutputPortIndex

Type        :   BDSP Internal

Input       :   psStageOutput      -   Pointer of the StageOutput details for which we need to return the Output port index
                index              -    Index returned back.

Return      :   Error Code to return SUCCESS or FAILURE

Functionality   :   Following are the operations performed.
        1)  Return the first free output index for the stage and return the same to the caller.
***********************************************************************/

void BDSP_P_GetFreeOutputPortIndex(BDSP_StageSrcDstDetails *psStageOutput, unsigned *index)
{
    unsigned i;
    BDSP_StageSrcDstDetails *psStageSrcDstDetails = psStageOutput;

    for (i = 0; i < BDSP_AF_P_MAX_OP_FORKS; i++)
    {
        if (psStageSrcDstDetails->eNodeValid != BDSP_AF_P_eValid)
        {
            break;
        }
        psStageSrcDstDetails++;
    }

    *index = i;
}



/***********************************************************************
Name        :   BDSP_DSP_P_InterframeRunLengthDecode

Type        :   PI Interface

Input       :   pSrc                    -   Pointer to Source Array which is encoded using Run Length Algorithm
                pDst                    -   Pointer to Memory Location in DRAM where the uncompressed array needs to be copied.
                ui32IfBuffEncodedSize   -   Size of encoded Inter-Frame array
                ui32AllocatedBufferSize -   Size of Memory Allocated in Dram.

Return      :   BERR_Code

Functionality   :   Following are the operations performed.
        1)  Uncompress the Inter-Frame Array using the Run Length Decode Algorithm
        2)  Confirm that enough size has been allocated in DRAM by BDSP.
        3)  Copy the original uncompressed opcodes into DRAM
***********************************************************************/

BERR_Code BDSP_DSP_P_InterframeRunLengthDecode(void *pSrc, void *pDst, uint32_t ui32IfBuffEncodedSize, uint32_t ui32AllocatedBufferSize)
{
    BERR_Code   rc = BERR_SUCCESS;
    uint32_t ui32IfBuffActualSize = 0;
    uint32_t i =0, j =0;
    uint32_t * pTempSrc = (uint32_t *) pSrc;
    uint32_t * pTempDest = (uint32_t *) pDst;

    BDBG_ENTER(BDSP_DSP_P_InterframeRunLengthDecode);
    BDBG_ASSERT(( ui32IfBuffEncodedSize % 8 == 0)); /* Any array that has been encoded using the run length encode has to be in multiples of 8. Otherwise something has gone wrong */

    for (i = 0; i < (ui32IfBuffEncodedSize >> 3); i++)
    {
        ui32IfBuffActualSize += pTempSrc[2*i + 1];
    }

    /* Size has already been allocated in DRAM. Confirm whether it is big enough. */
    if (ui32AllocatedBufferSize < ui32IfBuffActualSize)
    {
        BDBG_ERR(("Allocated memory (%d) for interframe buffer is less than required (%d)",
                    ui32AllocatedBufferSize,
                    ui32IfBuffActualSize
                 ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Then start copying in chunks element by element */
    for (i = 0; i < (ui32IfBuffEncodedSize >> 3); i++)
    {
        for (j = 0; j < pTempSrc[2*i + 1]; j++)
        {
            *pTempDest++ = pTempSrc[2*i];
        }
    }
    BDBG_LEAVE(BDSP_DSP_P_InterframeRunLengthDecode);
    return rc;
}
