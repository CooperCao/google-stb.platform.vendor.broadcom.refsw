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

BDBG_MODULE(bdsp_buf);

void BDSP_P_Analyse_CIT_TSMConfig_isr(
	BDSP_MMA_Memory Memory
)
{
	BDSP_AudioTaskTsmSettings *pTsmSettings;
	BDBG_ENTER(BDSP_P_Analyse_CIT_TSMConfig_isr);

	pTsmSettings = (BDSP_AudioTaskTsmSettings *)Memory.pAddr;
	BDBG_MSG(("TSM CONFIG"));
	BDBG_MSG(("-------------------- "));
	BDBG_MSG(("\t Smooth Threshold      = %d",pTsmSettings->i32TSMSmoothThreshold));
	BDBG_MSG(("\t Sync Limit Threshold  = %d",pTsmSettings->i32TSMSyncLimitThreshold));
	BDBG_MSG(("\t Gross Threshold       = %d",pTsmSettings->i32TSMGrossThreshold));
	BDBG_MSG(("\t Discard Threshold     = %d",pTsmSettings->i32TSMDiscardThreshold));
	BDBG_MSG(("\t Transaction Threshold = %d",pTsmSettings->i32TsmTransitionThreshold));
	BDBG_MSG(("\t STC ADDR              = 0x%x",pTsmSettings->ui32STCAddr));
	BDBG_MSG(("\t AV Offset             = %d",pTsmSettings->ui32AVOffset));
	BDBG_MSG(("\t SW STC Offset         = %d",pTsmSettings->ui32SwSTCOffset));
	BDBG_MSG(("\t Audio Offset          = %d",pTsmSettings->ui32AudioOffset));
	BDBG_MSG(("\t Enable Recovery       = %s",DisableEnable[pTsmSettings->eEnableTSMErrorRecovery]));
	BDBG_MSG(("\t STC Valid             = %s",TrueFalse[pTsmSettings->eSTCValid]));
	BDBG_MSG(("\t Playback On           = %s",TrueFalse[pTsmSettings->ePlayBackOn]));
	BDBG_MSG(("\t TSM Enable            = %s",DisableEnable[pTsmSettings->eTsmEnable]));
	BDBG_MSG(("\t TSM Log Enable        = %s",DisableEnable[pTsmSettings->eTsmLogEnable]));
	BDBG_MSG(("\t ASTM Enable           = %s",DisableEnable[pTsmSettings->eAstmEnable]));
	BDBG_MSG(("-------------------- "));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_TSMConfig_isr);
}

void BDSP_P_Analyse_CIT_TSMConfig(
	BDSP_MMA_Memory Memory
)
{
	BDSP_AudioTaskTsmSettings *pTsmSettings;
	BDBG_ENTER(BDSP_P_Analyse_CIT_TSMConfig);

	pTsmSettings = (BDSP_AudioTaskTsmSettings *)Memory.pAddr;
	BDBG_MSG(("TSM CONFIG"));
	BDBG_MSG(("-------------------- "));
	BDBG_MSG(("\t Smooth Threshold      = %d",pTsmSettings->i32TSMSmoothThreshold));
	BDBG_MSG(("\t Sync Limit Threshold  = %d",pTsmSettings->i32TSMSyncLimitThreshold));
	BDBG_MSG(("\t Gross Threshold       = %d",pTsmSettings->i32TSMGrossThreshold));
	BDBG_MSG(("\t Discard Threshold     = %d",pTsmSettings->i32TSMDiscardThreshold));
	BDBG_MSG(("\t Transaction Threshold = %d",pTsmSettings->i32TsmTransitionThreshold));
	BDBG_MSG(("\t STC ADDR              = 0x%x",pTsmSettings->ui32STCAddr));
	BDBG_MSG(("\t AV Offset             = %d",pTsmSettings->ui32AVOffset));
	BDBG_MSG(("\t SW STC Offset         = %d",pTsmSettings->ui32SwSTCOffset));
	BDBG_MSG(("\t Audio Offset          = %d",pTsmSettings->ui32AudioOffset));
	BDBG_MSG(("\t Enable Recovery       = %s",DisableEnable[pTsmSettings->eEnableTSMErrorRecovery]));
	BDBG_MSG(("\t STC Valid             = %s",TrueFalse[pTsmSettings->eSTCValid]));
	BDBG_MSG(("\t Playback On           = %s",TrueFalse[pTsmSettings->ePlayBackOn]));
	BDBG_MSG(("\t TSM Enable            = %s",DisableEnable[pTsmSettings->eTsmEnable]));
	BDBG_MSG(("\t TSM Log Enable        = %s",DisableEnable[pTsmSettings->eTsmLogEnable]));
	BDBG_MSG(("\t ASTM Enable           = %s",DisableEnable[pTsmSettings->eAstmEnable]));
	BDBG_MSG(("-------------------- "));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_TSMConfig);
}

void BDSP_P_Analyse_CIT_GateOpenConfig(
	BDSP_MMA_Memory Memory
)
{
	BDSP_AF_P_sTASK_GATEOPEN_CONFIG *pGateOpenConfig;
	unsigned index = 0, i =0;

	BDBG_ENTER(BDSP_P_Analyse_CIT_GateOpenConfig);
	pGateOpenConfig = (BDSP_AF_P_sTASK_GATEOPEN_CONFIG *)Memory.pAddr;

	BDBG_MSG(("GATE OPEN CONFIG"));
	BDBG_MSG(("-------------------- "));
	BDBG_MSG(("\tNumber of Ports  for Gate Open = %d", pGateOpenConfig->ui32NumPorts ));
	BDBG_MSG(("\tMax Independent Delay          = %d", pGateOpenConfig->ui32MaxIndependentDelay));
	BDBG_MSG(("\tBlocking Time                  = %d", pGateOpenConfig->ui32BlockingTime));
	for(index=0;index<pGateOpenConfig->ui32NumPorts; index++)
	{
		BDBG_MSG(("\tPORT(%d) DETAILS",index));
		BDBG_MSG(("\t Number of Buffers      = %d",pGateOpenConfig->sRingBufferInfo[index].ui32NumBuffers));
		BDBG_MSG(("\t Independent Delay      = %d",pGateOpenConfig->sRingBufferInfo[index].ui32IndependentDelay));
		BDBG_MSG(("\t Fixed Sample Rate      = %s",TrueFalse[pGateOpenConfig->sRingBufferInfo[index].bFixedSampleRate]));
		BDBG_MSG(("\t Fixed Sample Rate Val  = %d",pGateOpenConfig->sRingBufferInfo[index].ui32FixedSampleRate));
		BDBG_MSG(("\t FMM Content Type       = %s",FMMContentType[pGateOpenConfig->sRingBufferInfo[index].eFMMContentType]));
		BDBG_MSG(("\t BaseRate Multiplier    = %s",BaseRateMultiplier[pGateOpenConfig->sRingBufferInfo[index].eBaseRateMultiplier]));
		BDBG_MSG(("\t Buffer type            = %s",BufferType[pGateOpenConfig->sRingBufferInfo[index].eBufferType]));
		for(i=0;i< pGateOpenConfig->sRingBufferInfo[index].ui32NumBuffers; i++)
		{
			BDBG_MSG(("\t    Gate Open Buffer(%d): Base Addr = "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].sCircularBuffer.BaseAddr)));
			BDBG_MSG(("\t    Gate Open Buffer(%d): End  Addr = "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].sCircularBuffer.EndAddr)));
			BDBG_MSG(("\t    Gate Open Buffer(%d): Read Addr = "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].sCircularBuffer.ReadAddr)));
			BDBG_MSG(("\t    Gate Open Buffer(%d): Write Addr= "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].sCircularBuffer.WriteAddr)));
			BDBG_MSG(("\t    Gate Open Buffer(%d): Wrap Addr = "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].sCircularBuffer.WrapAddr)));
			BDBG_MSG(("\t    Gate Open Buffer(%d): StWr Addr = "BDSP_MSG_FMT,i,BDSP_MSG_ARG(pGateOpenConfig->sRingBufferInfo[index].sExtendedBuffer[i].startWriteAddr)));
		}
	}
	BDBG_MSG(("-------------------- "));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_GateOpenConfig);
}

void BDSP_P_Analyse_CIT_StcTriggerConfig(
	BDSP_MMA_Memory Memory
)
{
	BDSP_AF_P_sSTC_TRIGGER_CONFIG  *pStcTriggerConfig;

	BDBG_ENTER(BDSP_P_Analyse_CIT_StcTriggerConfig);
    pStcTriggerConfig = (BDSP_AF_P_sSTC_TRIGGER_CONFIG *)Memory.pAddr;
	BDBG_MSG(("STC TRIGGER CONFIG"));
	BDBG_MSG(("-------------------- "));
	BDBG_MSG(("\t STC TRIGGER REQUIRED  = %s",DisableEnable[pStcTriggerConfig->eStcTrigRequired]));
    BDBG_MSG(("\t STC INC ADDR - HIGH   = 0x%x",pStcTriggerConfig->ui32StcIncHiAddr));
    BDBG_MSG(("\t STC INC ADDR - LOW    = 0x%x",pStcTriggerConfig->ui32StcIncLowAddr));
    BDBG_MSG(("\t STC TRIGGER BIT       = %d",pStcTriggerConfig->ui32TriggerBit));
    BDBG_MSG(("\t STC INC TRIGGER ADDR  = 0x%x",pStcTriggerConfig->ui32stcIncTrigAddr));
	BDBG_MSG(("-------------------- "));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_StcTriggerConfig);
}

void BDSP_P_Analyse_CIT_SchedulingConfig(
	BDSP_MMA_Memory Memory
)
{
	BDSP_AF_P_sTASK_SCHEDULING_CONFIG *pSchedulingConfig;

	BDBG_ENTER(BDSP_P_Analyse_CIT_SchedulingConfig);
	pSchedulingConfig = (BDSP_AF_P_sTASK_SCHEDULING_CONFIG *)Memory.pAddr;

	BDBG_MSG(("SCHEDULING BUFFER CONFIG"));
	BDBG_MSG(("-------------------- "));
	BDBG_MSG(("\t Blocking Time           = %d",pSchedulingConfig->sSchedulingInfo.ui32BlockingTime));
	BDBG_MSG(("\t Max Independent Delay   = %d",pSchedulingConfig->sSchedulingInfo.ui32MaxIndependentDelay));
	BDBG_MSG(("\t Independent Delay       = %d",pSchedulingConfig->sSchedulingInfo.ui32IndependentDelay));
	BDBG_MSG(("\t Fixed SampleRate Enabled= %s",TrueFalse[pSchedulingConfig->sSchedulingInfo.bFixedSampleRate]));
	BDBG_MSG(("\t Fixed SampleRate        = %d",pSchedulingConfig->sSchedulingInfo.ui32FixedSampleRate));
	BDBG_MSG(("\t Buffer Type			  = %s",BufferType[pSchedulingConfig->eBufferType]));
	BDBG_MSG(("\t FMM Content Type        = %s",FMMContentType[pSchedulingConfig->sSchedulingInfo.eFMMContentType]));
	BDBG_MSG(("\t Base Rate Multipler     = %s",BaseRateMultiplier[pSchedulingConfig->sSchedulingInfo.eBaseRateMultiplier]));
	BDBG_MSG(("\t Buffer Base Addr        = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.sCircularBuffer.BaseAddr)));
	BDBG_MSG(("\t Buffer End  Addr        = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.sCircularBuffer.EndAddr)));
	BDBG_MSG(("\t Buffer Read Addr        = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.sCircularBuffer.ReadAddr)));
	BDBG_MSG(("\t Buffer Write Addr       = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.sCircularBuffer.WriteAddr)));
	BDBG_MSG(("\t Buffer Wrap Addr        = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.sCircularBuffer.WrapAddr)));
	BDBG_MSG(("\t Buffer StWr Addr        = "BDSP_MSG_FMT,BDSP_MSG_ARG(pSchedulingConfig->sExtendedBuffer.startWriteAddr)));
	BDBG_MSG(("-------------------- "));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_SchedulingConfig);
}

void BDSP_P_Analyse_CIT_PPMConfig(
	BDSP_AF_P_sHW_PPM_CONFIG    *psPPMConfig
)
{
	unsigned index = 0;
	BDBG_ENTER(BDSP_P_Analyse_CIT_PPMConfig);
	BDBG_MSG(("PPM CONFIG"));
	BDBG_MSG(("-------------------- "));
	for(index=0; index<BDSP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS; index++)
	{
		if(psPPMConfig->ePPMChannel == BDSP_AF_P_eEnable)
			BDBG_MSG(("\t PPM Config Offset = "BDSP_MSG_FMT,BDSP_MSG_ARG(psPPMConfig->PPMCfgAddr)));
		psPPMConfig++;
	}
	BDBG_MSG(("---------------------"));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_PPMConfig);
}

void BDSP_P_Analyse_CIT_DataAccess(
	BDSP_AF_P_Port_sDataAccessAttributes *pDataAccessAttributes
)
{
	BDBG_ENTER(BDSP_P_Analyse_CIT_DataAccess);
	BDBG_MSG(("--"));
	BDBG_MSG(("\t\t Data Access Type      = %s",DataAccessType[pDataAccessAttributes->eDataAccessType]));
	BDBG_MSG(("\t\t D.A Number of Channels= %d",pDataAccessAttributes->ui32numChannels));
	BDBG_MSG(("\t\t D.A bytes/sample      = %d",pDataAccessAttributes->ui32bytesPerSample));
	BDBG_MSG(("\t\t D.A Max Channel Size  = %d",pDataAccessAttributes->ui32maxChannelSize));
	BDBG_MSG(("--"));
	BDBG_LEAVE(BDSP_P_Analyse_CIT_DataAccess);
}

void BDSP_P_Analyse_CIT_BufferDetails(
	BDSP_P_FwBuffer   *pDescriptorMemory,
	dramaddr_t 		   offset,
	unsigned           index
)
{
	BDSP_AF_P_sCIRCULAR_BUFFER *pCircularBuffer;
	BDBG_ENTER(BDSP_P_Analyse_CIT_BufferDetails);

	pCircularBuffer = (BDSP_AF_P_sCIRCULAR_BUFFER *)BDSP_MMA_P_GetVirtualAddressfromOffset(
		pDescriptorMemory,
		offset);

	BDBG_MSG(("--"));
	BDBG_MSG(("\t\t IO Buffer: Descriptor[%d] Addr = %p, Offset to FW ="BDSP_MSG_FMT,index,(void *)pCircularBuffer, BDSP_MSG_ARG(offset)));
	BDBG_MSG(("\t\t IO Buffer[%d]: Base Addr = "BDSP_MSG_FMT,index,BDSP_MSG_ARG(pCircularBuffer->BaseAddr)));
	BDBG_MSG(("\t\t IO Buffer[%d]: End Addr  = "BDSP_MSG_FMT,index,BDSP_MSG_ARG(pCircularBuffer->EndAddr)));
	BDBG_MSG(("\t\t IO Buffer[%d]: Read Addr = "BDSP_MSG_FMT,index,BDSP_MSG_ARG(pCircularBuffer->ReadAddr)));
	BDBG_MSG(("\t\t IO Buffer[%d]: Write Addr= "BDSP_MSG_FMT,index,BDSP_MSG_ARG(pCircularBuffer->WriteAddr)));
	BDBG_MSG(("\t\t IO Buffer[%d]: Wrap Addr = "BDSP_MSG_FMT,index,BDSP_MSG_ARG(pCircularBuffer->WrapAddr)));
	BDBG_MSG(("--"));

	BDBG_LEAVE(BDSP_P_Analyse_CIT_BufferDetails);
}
