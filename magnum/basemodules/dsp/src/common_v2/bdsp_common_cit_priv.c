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

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_common_cit_priv);

void BDSP_P_FillSamplingFrequencyLut(
	BDSP_MMA_Memory *pLUTMemory
)
{
    BDSP_AF_P_sOpSamplingFreq sOpSamplingFrequencyMapLut =  {
																{ /*QSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*HSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*FSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*HSF */
                                                                  32000,
                                                                  44100,
                                                                  48000,
                                                                  /*VHSF */
                                                                  32000,
                                                                  44100,
                                                                  48000
                                                                }
                                                            };

    BDBG_ENTER(BDSP_P_FillSamplingFrequencyLut);

	BDSP_MMA_P_CopyDataToDram(pLUTMemory,
		(void *)&sOpSamplingFrequencyMapLut,
		sizeof(BDSP_AF_P_sOpSamplingFreq));

	BDBG_LEAVE(BDSP_P_FillSamplingFrequencyLut);
	return;
}

void BDSP_P_GetFMMDetails(
	BDSP_AF_P_DistinctOpType  eOutputType,
	BDSP_Algorithm			  eAlgorithm,
    BDSP_AF_P_FmmDstFsRate   *pBaseRateMultiplier,
    BDSP_AF_P_FmmContentType *pFMMContentType
)
{
	switch (eOutputType)
	{
		case BDSP_AF_P_DistinctOpType_e7_1_PCM:
		case BDSP_AF_P_DistinctOpType_e5_1_PCM:
		case BDSP_AF_P_DistinctOpType_eStereo_PCM:
		case BDSP_AF_P_DistinctOpType_eMono_PCM:
			*pFMMContentType 	 = BDSP_AF_P_FmmContentType_ePcm;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressed:
		case BDSP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eCompressed;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressed4x:
		/*SW7425-6056: It is based on ui32FMMContentType that the FW decides on a type of zero fill.
		Without this check, Spdif preambles were filled in during a zero fill for BTSC
		SWRAA-162: New FMM Dest type added to address the same*/
			if (eAlgorithm == BDSP_Algorithm_eBtscEncoder)
			{
				*pFMMContentType = BDSP_AF_P_FmmContentType_eAnalogCompressed;
			}else
			{
				*pFMMContentType = BDSP_AF_P_FmmContentType_eCompressed;
			}
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_e4xBaseRate;
			break;
		case BDSP_AF_P_DistinctOpType_eCompressedHBR:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eCompressed;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_e16xBaseRate;
			break;
		default:
			*pFMMContentType     = BDSP_AF_P_FmmContentType_eInvalid;
			*pBaseRateMultiplier = BDSP_AF_P_FmmDstFsRate_eInvalid;
			break;
	}
}

BERR_Code BDSP_P_PopulateStcTriggerConfig(
    BDSP_TaskStartSettings  *pStartSettings,
    BDSP_MMA_Memory          Memory
)
{
    BERR_Code errCode = BERR_SUCCESS;
    BDSP_AF_P_sSTC_TRIGGER_CONFIG *pStcTriggerConfig;

    BDBG_ENTER(BDSP_P_PopulateStcTriggerConfig);
    pStcTriggerConfig = (BDSP_AF_P_sSTC_TRIGGER_CONFIG *)Memory.pAddr;

    pStcTriggerConfig->eStcTrigRequired   = pStartSettings->stcIncrementConfig.enableStcTrigger;
    pStcTriggerConfig->ui32StcIncHiAddr   = pStartSettings->stcIncrementConfig.stcIncHiAddr;
    pStcTriggerConfig->ui32StcIncLowAddr  = pStartSettings->stcIncrementConfig.stcIncLowAddr;
    pStcTriggerConfig->ui32stcIncTrigAddr = pStartSettings->stcIncrementConfig.stcIncTrigAddr;
    pStcTriggerConfig->ui32TriggerBit     = pStartSettings->stcIncrementConfig.triggerBit;

	BDSP_MMA_P_FlushCache(Memory,sizeof(BDSP_AF_P_sSTC_TRIGGER_CONFIG));

    BDBG_LEAVE(BDSP_P_PopulateStcTriggerConfig);
    return errCode;
}

void BDSP_P_UpdateResidueCollection(
    BDSP_P_ConnectionDetails *pOutputDetails,
    BDSP_AF_P_EnableDisable  *pCollectResidual
)
{
    unsigned i = 0;
    BDBG_ENTER(BDSP_P_UpdateResidualCollection);

    for(i=0; i<BDSP_AF_P_MAX_OP_FORKS; i++)
    {
        if((pOutputDetails->eValid == BDSP_AF_P_eValid)&&
            (pOutputDetails->eConnectionType == BDSP_ConnectionType_eFmmBuffer))
        {
            *pCollectResidual = BDSP_AF_P_eDisable;
            break;
        }
        pOutputDetails++;
    }
    BDBG_LEAVE(BDSP_P_UpdateResidualCollection);
}
