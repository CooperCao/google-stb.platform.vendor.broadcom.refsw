/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description: Audio Decoder Interface
 *
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bape_input.h"

BDBG_MODULE(bape_input_port);

BDBG_OBJECT_ID(BAPE_InputPort);

/* Preamble C defines */
#define  BURST_PREAM_C_ALGO_ID                              0x001f
#define  BURST_PREAM_C_ALGO_ID_AC3                               1
#define  BURST_PREAM_C_ALGO_ID_PAUSE                             3
#define  BURST_PREAM_C_ALGO_ID_MPEGL1                            4
#define  BURST_PREAM_C_ALGO_ID_MPEGL2L3                          5
#define  BURST_PREAM_C_ALGO_ID_MPEG2EXT                          6
#define  BURST_PREAM_C_ALGO_ID_AAC                               7
#define  BURST_PREAM_C_ALGO_ID_MPEG2L1                           8
#define  BURST_PREAM_C_ALGO_ID_MPEG2L2                           9
#define  BURST_PREAM_C_ALGO_ID_MPEG2L3                          10
#define  BURST_PREAM_C_ALGO_ID_DTSI                             11
#define  BURST_PREAM_C_ALGO_ID_DTSII                            12
#define  BURST_PREAM_C_ALGO_ID_DTSIII                           13
#define  BURST_PREAM_C_ALGO_ID_DTSIV                            17
#define  BURST_PREAM_C_ALGO_ID_MPEG2AAC_LSF                     19
#define  BURST_PREAM_C_ALGO_ID_MPEG4AAC                         20
#define  BURST_PREAM_C_ALGO_ID_AC3Enhanced                      21
#define  BURST_PREAM_C_ALGO_ID_MAT                              22

#define  BURST_PREAM_C_PAYLOAD_MAY_CONTAIN_ERRORS           0x0080
#define  BURST_PREAM_C_DATA_TYPE_DEPENDENT_INFO             0x1f00
#define  BURST_PREAM_C_BIT_STREAM_NUMBER                    0xD000

unsigned BAPE_InputPort_P_GetNumConsumersAttached_isrsafe(BAPE_InputPort inputPort)
{
    unsigned count = 0;
    BAPE_PathNode * consumer;
    for ( consumer = BLST_S_FIRST(&inputPort->consumerList);
        consumer != NULL;
        consumer = BLST_S_NEXT(consumer, consumerNode) )
    {
        if ( consumer )
        {
            count++;
        }
    }

    return count;
}

bool BAPE_InputPort_P_HasConsumersAttached_isrsafe(BAPE_InputPort inputPort)
{
    BAPE_PathNode * consumer;
    for ( consumer = BLST_S_FIRST(&inputPort->consumerList);
        consumer != NULL;
        consumer = BLST_S_NEXT(consumer, consumerNode) )
    {
        if ( consumer )
        {
            return true;
        }
    }

    return false;
}

bool BAPE_InputPort_P_ConsumerIsAttached(BAPE_InputPort inputPort, BAPE_PathNode * pConsumer)
{
    BAPE_PathNode * consumer;
    for ( consumer = BLST_S_FIRST(&inputPort->consumerList);
        consumer != NULL;
        consumer = BLST_S_NEXT(consumer, consumerNode) )
    {
        if ( consumer == pConsumer)
        {
            return true;
        }
    }

    return false;
}

BERR_Code BAPE_InputPort_P_AttachConsumer(
    BAPE_InputPort inputPort,
    BAPE_PathNode *pConsumer,
    BAPE_FMT_Descriptor *pInputFormat   /* [out] Current format */
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BDBG_OBJECT_ASSERT(pConsumer, BAPE_PathNode);

    if ( inputPort->fciSpGroup == NULL && BAPE_InputPort_P_HasConsumersAttached(inputPort) )
    {
        BDBG_ERR(("This chip does not support FCI splitters. Therefore we can only have one consumer for a given input port"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( BAPE_InputPort_P_ConsumerIsAttached(inputPort, pConsumer) )
    {
        return BERR_SUCCESS;
    }

    BDBG_MSG(("Attached consumer %s to input port %s", pConsumer->pName, inputPort->pName));
    BKNI_EnterCriticalSection();
    inputPort->halted = false;  /* just in case we had been halted previously. */
    BLST_S_INSERT_HEAD(&inputPort->consumerList, pConsumer, consumerNode); /* Add pConsumer to the consumer list */
    inputPort->consumerAttaching = true;
    if ( inputPort->consumerAttached_isr )
    {
        errCode = inputPort->consumerAttached_isr(inputPort, pConsumer, pInputFormat);
        if ( errCode )
        {
            BDBG_ERR(("Failed to Attach %s to %s", pConsumer->pName, inputPort->pName));
            BLST_S_REMOVE(&inputPort->consumerList, pConsumer, BAPE_PathNode, consumerNode);
            /* Will return error below after leaving critical section */
        }
    }
    else
    {
        *pInputFormat = inputPort->format;
    }
    inputPort->consumerAttaching = false;
    BKNI_LeaveCriticalSection();
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Now check to make sure that the consumer can handle the inputPort's current format. */
    if ( !BAPE_FMT_P_FormatSupported_isrsafe(&pConsumer->inputCapabilities, &inputPort->format) )
    {
        BDBG_ERR(("Node %s can not accept this InputPort's data format (%s)", pConsumer->pName, BAPE_FMT_P_GetTypeName_isrsafe(&inputPort->format)));

        errCode = BAPE_InputPort_P_DetachConsumer(inputPort, pConsumer);
        return BERR_NOT_SUPPORTED;
    }

    return BERR_SUCCESS;
}

BERR_Code BAPE_InputPort_P_DetachConsumer(
                                         BAPE_InputPort inputPort,
                                         BAPE_PathNode *pConsumer
                                         )
{
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BDBG_OBJECT_ASSERT(pConsumer, BAPE_PathNode);

    if ( !BAPE_InputPort_P_ConsumerIsAttached(inputPort, pConsumer) )
    {
        return BERR_SUCCESS;
    }
    BDBG_MSG(("Detached consumer %s from input port %s", pConsumer->pName, inputPort->pName));
    BKNI_EnterCriticalSection();
    if ( inputPort->consumerDetached_isr )
    {
        inputPort->consumerDetached_isr(inputPort, pConsumer);
    }
    BLST_S_REMOVE(&inputPort->consumerList, pConsumer, BAPE_PathNode, consumerNode);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}

void BAPE_InputPort_P_GetFciIds(
    BAPE_InputPort inputPort,
    BAPE_FciIdGroup *pFciGroup      /* [out] */
    )
{
    unsigned i;

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);

    for ( i = 0; i < BAPE_FMT_P_GetNumChannelPairs_isrsafe(&inputPort->format); i++ )
    {
        pFciGroup->ids[i] = BAPE_FCI_BASE_INPUT | inputPort->streamId[i];
    }
    for ( ; i < BAPE_ChannelPair_eMax; i++ )
    {
        pFciGroup->ids[i] = BAPE_FCI_ID_INVALID;
    }
}

void BAPE_InputPort_P_BurstPreambleToCodec_isr(uint32_t burstPreamble, BAVC_AudioCompressionStd *codec )
{
    switch (burstPreamble & BURST_PREAM_C_ALGO_ID)  /* Mask off unrelated bits. */
    {
    case BURST_PREAM_C_ALGO_ID_AC3:
        *codec = BAVC_AudioCompressionStd_eAc3;
        break;

    case BURST_PREAM_C_ALGO_ID_AAC:
    case BURST_PREAM_C_ALGO_ID_MPEG2AAC_LSF:
    case BURST_PREAM_C_ALGO_ID_MPEG4AAC:
        *codec = BAVC_AudioCompressionStd_eAac;
        break;

    case BURST_PREAM_C_ALGO_ID_MPEGL1:
    case BURST_PREAM_C_ALGO_ID_MPEGL2L3:
    case BURST_PREAM_C_ALGO_ID_MPEG2EXT:
    case BURST_PREAM_C_ALGO_ID_MPEG2L1:
    case BURST_PREAM_C_ALGO_ID_MPEG2L2:
    case BURST_PREAM_C_ALGO_ID_MPEG2L3:
        *codec = BAVC_AudioCompressionStd_eMpegL3;
        break;

    case BURST_PREAM_C_ALGO_ID_DTSI:
    case BURST_PREAM_C_ALGO_ID_DTSII:
    case BURST_PREAM_C_ALGO_ID_DTSIII:
        *codec = BAVC_AudioCompressionStd_eDts;
        break;

    case BURST_PREAM_C_ALGO_ID_DTSIV:
        *codec = BAVC_AudioCompressionStd_eDtshd;
        break;

    case BURST_PREAM_C_ALGO_ID_AC3Enhanced:
        *codec = BAVC_AudioCompressionStd_eAc3Plus;
        break;

    case BURST_PREAM_C_ALGO_ID_MAT:
        *codec = BAVC_AudioCompressionStd_eMlp;
        break;

    default:
        *codec = BAVC_AudioCompressionStd_eMax;
        break;
    } /* End Switch */
}


void BAPE_InputPort_P_GetFormat_isr(
    BAPE_InputPort inputPort,
    BAPE_FMT_Descriptor *pFormat    /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BDBG_ASSERT(NULL != pFormat);

    BKNI_ASSERT_ISR_CONTEXT();

    *pFormat = inputPort->format;
}

BERR_Code BAPE_InputPort_P_SetFormat_isr(
    BAPE_InputPort inputPort,
    const BAPE_FMT_Descriptor *pNewFormat
    )
{
    BERR_Code errCode = BERR_SUCCESS;
    BAPE_FMT_Descriptor oldFormat;

    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BDBG_ASSERT(NULL != pNewFormat);

    BKNI_ASSERT_ISR_CONTEXT();

    oldFormat = inputPort->format;
    inputPort->format = *pNewFormat;

    if ( BAPE_InputPort_P_HasConsumersAttached_isrsafe(inputPort) &&
         !inputPort->consumerAttaching )
    {
        BAPE_PathNode * pConsumer;
        for ( pConsumer = BLST_S_FIRST(&inputPort->consumerList);
            pConsumer != NULL;
            pConsumer = BLST_S_NEXT(pConsumer, consumerNode) )
        {
            if ( pConsumer->inputPortFormatChange_isr )
            {
                errCode |= pConsumer->inputPortFormatChange_isr(pConsumer, inputPort);
            }
        }
    }

    if ( errCode )
    {
        inputPort->format = oldFormat;
    }

    return(errCode); /* BERR_TRACE intentionally omitted */
}
BERR_Code BAPE_InputPort_P_Halt_isr(
    BAPE_InputPort inputPort
    )
{
    BDBG_OBJECT_ASSERT(inputPort, BAPE_InputPort);
    BKNI_ASSERT_ISR_CONTEXT();

    inputPort->halted = true;

    return BERR_SUCCESS;
}
