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
 ******************************************************************************/

#include "bdsp_raaga_priv.h"
#include "bdsp_raaga_fw_cit.h"
#include "bdsp_raaga_fw.h"
#include "bdsp_raaga_fw_status.h"
#include "bdsp_task.h"
#include "bdsp_raaga_cit_priv.h"
#include "bdsp_raaga_cit_log.h"
#include "bdsp_common_cit_priv.h"
#include "bdsp_common_priv.h"

BDBG_MODULE(bdsp_cit);

static const char AlgoIdEnum2Char[BDSP_AF_P_AlgoId_eMax+1][MAX_CHAR_LENGTH] =
{
    {"MpegDecode"},
    {"Ac3Decode"},
    {"AacDecode"},
    {"AacHeLpSbrDecode"},
    {"DdpDecode"},
    {"DdLosslessDecode"},
    {"LpcmCustomDecode"},
    {"BdLpcmDecode"},
    {"DvdLpcmDecode"},
    {"HdDvdLpcmDecode"},
    {"MpegMcDecode"},
    {"WmaStdDecode"},
    {"WmaProStdDecode"},
    {"MlpDecode"},
    {"Ddp71Decode"},
    {"DtsDecode"},
    {"DtsLbrDecode"},
    {"DtsHdDecode"},
    {"PcmWavDecode"},
    {"Amr Decode"},
    {"Dra Decode"},
    {"Real Audio LBR Decode"},
    {"Dolby Pulse Decode"},
    {"Ms10 DdpDecode"},
    {"Adpcm Decode"},
    {"G.711/G.726 Decode"},
    {"G.729 Decode"},
    {"VORBIS Decode"},
    {"G.723.1 Decode"},
    {"FLAC Decode"},
    {"MAC Decode"},
    {"Amrwb Decode"},
    {"iLBC Decode"},
    {"ISAC Decode"},
    {"UDC Decode"},
    {"Dolby AACHE Decode"},
    {"Opus Decode"},
    {"ALS Decode"},
    {"AC4 Decode"},
    {"EndOfAudioDecodeAlgos"},
    {"Real Video Decode"},
    {"VP6 Video Decode"},
    {"EndOfDecodeAlgos"},
    {"MpegFrameSync"},
    {"MpegMcFrameSync"},
    {"AdtsFrameSync"},
    {"LoasFrameSync"},
    {"WmaStdFrameSync"},
    {"WmaProFrameSync"},
    {"Ac3FrameSync"},
    {"DdpFrameSync"},
    {"Ddp71FrameSync"},
    {"DtsFrameSync"},
    {"DtsLbrFrameSync"},
    {"DtsHdFrameSync"},
    {"DtsHdFrameSync_1"},
    {"DtsHdHdDvdFrameSync"},
    {"DdLosslessFrameSync"},
    {"MlpFrameSync"},
    {"MlpHdDvdFrameSync"},
    {"PesFrameSync"},
    {"BdLpcmFrameSync"},
    {"HdDvdLpcmFrameSync"},
    {"DvdLpcmFrameSync"},
    {"DvdLpcmFrameSync_1"},
    {"PcmWavFrameSync"},
    {"Dra FrameSync"},
    {"Real Audio LBR FrameSync"},
    {"Ms10 Ddp FrameSync"},
    {"VORBIS FrameSync"},
    {"FLAC FrameSync"},
    {"MAC FrameSync"},
    {"UDC FrameSync"},
    {"AC4 FrameSync"},
    {"ALS FrameSync"},
    {"EndOfAudioDecFsAlgos"},
    {"Real Video FrameSync"},
    {"VP6 Video FrameSync"},
    {"EndOfDecFsAlgos"},
    {"Ac3Encode"},
    {"MpegL2Encode"},
    {"MpegL3Encode"},
    {"AacLcEncode"},
    {"AacHeEncode"},
    {"DtsEncode"},
    {"DtsBroadcastEncode"},
    {"SBC Encode"},
    {"DD Transcode"},
    {"G.711/G.726 Encode"},
    {"G.729 Encode"},
    {"G.723.1 Encode"},
    {"G.722 Encode"},
    {"Amr Encode"},
    {"Amrwb Encode"},
    {"ILBC Encode"},
    {"ISAC Encode"},
    {"Lpcm Encode"},
    {"Opus Encode"},
    {"DDP Encode"},
    {"EndOfAudioEncodeAlgos"},
    {"H.264 Video Encoder"},
    {"X.264 Video Encoder"},
    {"X.VP8 Video Encoder"},
    {"EndOfVideoEncodeAlgos"},
    {"Ac3EncFrameSync"},
    {"MpegL3EncFrameSync"},
    {"MpegL2EncFrameSync"},
    {"AacLcEncFrameSync"},
    {"AacHeEncFrameSync"},
    {"DtsEncFrameSync"},
    {"EndOfEncFsAlgos"},
    {"PassThru"},
    {"MlpPassThru"},
    {"EndOfAuxAlgos"},
    {"SrsTruSurroundPostProc"},
    {"SrcPostProc"},
    {"DdbmPostProc"},
    {"DownmixPostProc"},
    {"CustomSurroundPostProc"},
    {"CustomBassPostProc"},
    {"KaraokeCapablePostProc"},
    {"CustomVoicePostProc"},
    {"PeqPostProc"},
    {"AvlPostProc"},
    {"Pl2PostProc"},
    {"XenPostProc"},
    {"BbePostProc"},
    {"DsolaPostProc"},
    {"DtsNeoPostProc"},
    {"DDConvert"},
    {"AudioDescriptorFadePostProc"},
    {"AudioDescriptorPanPostProc"},
    {"PCMRouterPostProc"},
    {"WMAPassThrough"},
    {"SrsTruSurroundHDPostProc"},
    {"SrsTruVolumePostProc"},
    {"DolbyVolumePostProc"},
    {"Brcm3D SurroundPostProc"},
    {"FwMixer PostProc"},
    {"MonoDownMix PostProc"},
    {"Ms10 DDConvert"},
    {"DdrePostProc"},
    {"Dv258PostProc"},
    {"DpcmrPostProc"},
    {"CdbItbGenPostProc"},
    {"Btsc Encoder"},
    {"Speex Acoustic echo canceller"},
    {"Karaoke"},
    {"MixerDapv2 PostProc"},
    {"Output Formatter"},
    {"Vocal PostProc"},
    {"Fade Control"},
    {"EndOfPpAlgos"},
    {"MixerFrameSync"},
    {"MixerDapv2FrameSync"},
    {"EndOfPpFsAlgos"},
    {"SysLib"},
    {"AlgoLib"},
    {"IDSCommonLib"},
    {"VideoIDSCommonLib"},
    {"EndOfLibAlgos"},
    {"Scm1 Processing"},
    {"Scm2 Processing"},
    {"Scm3Processing"},
    {"EndOfScmAlgos"},
    {"SCMTask"},
    {"EndOfTasks"},
    {"EndOfAlgos"}
};

void BDSP_Raaga_P_Analyse_CIT(BDSP_RaagaTask  *pRaagaTask,
							              bool             CitReconfig)
{
    uint32_t    ui32NumNodes=0;
    uint32_t    ui32Node;
    BDSP_AF_P_sGLOBAL_TASK_CONFIG   *psAudioGblTaskCfg;
	BDSP_SCM_P_sGLOBAL_TASK_CONFIG  *psScmGblTaskCfg;
	BDSP_VF_P_sGLOBAL_TASK_CONFIG   *psVideoDecGblTaskCfg;
	BDSP_VF_P_sENC_GLOBAL_TASK_CONFIG  *psVideoEncGblTaskCfg;
    BDSP_AF_P_sNODE_CONFIG          *psNodeCfg;
	BDSP_AF_P_sTASK_CONFIG          *psAudioTaskCfg;

    BDBG_ENTER(BDSP_Raaga_P_Analyse_CIT);

	if(CitReconfig){
	    BDBG_MSG(("============================================ "));
		BDBG_MSG(("\t RE-CONFIG CIT FOR TASK %d ",pRaagaTask->taskId));
	    BDBG_MSG(("============================================ "));
	}
	else{
	    BDBG_MSG(("============================================ "));
		BDBG_MSG(("\t\t   CIT FOR TASK %d ",pRaagaTask->taskId));
	    BDBG_MSG(("============================================ "));
	}
	BDBG_MSG(("Running on DSP: %d ",pRaagaTask->settings.dspIndex));
	BDBG_MSG(("Context: %s ",ContextType[pRaagaTask->pContext->settings.contextType]));
	BDBG_MSG(("Scheduling Mode: %s ",SchedulingMode[pRaagaTask->schedulingMode]));
	if(BDSP_TaskSchedulingMode_eSlave == pRaagaTask->schedulingMode)
	{
		BDBG_MSG(("Master Task ID: %d ",pRaagaTask->masterTaskId));
	}
	BDBG_MSG(("Scheduling Type: %s ",SchedulingType[pRaagaTask->startSettings.realtimeMode]));
	BDBG_MSG(("PPM Correction: %s ",DisableEnable[pRaagaTask->startSettings.ppmCorrection]));
	BDBG_MSG(("Delay Mode: %s ",DelayMode[pRaagaTask->startSettings.audioTaskDelayMode]));
	BDBG_MSG(("Zero Phase Correction: %s ",DisableEnable[pRaagaTask->startSettings.eZeroPhaseCorrEnable]));
	BDBG_MSG(("Gate Open Required: %s",DisableEnable[pRaagaTask->startSettings.gateOpenReqd]));
	BDBG_MSG(("Open Gate At Start: %s",DisableEnable[pRaagaTask->startSettings.openGateAtStart]));
	BDBG_MSG(("No of Dependent tasks for which Gate Open is required: %d",pRaagaTask->startSettings.DependentTaskInfo.numTasks));
	BDBG_MSG(("-------------------------------------------- "));

	switch (pRaagaTask->pContext->settings.contextType){
		case BDSP_ContextType_eAudio:
			psAudioTaskCfg = &(pRaagaTask->citOutput.sCit);
			psAudioGblTaskCfg = &(psAudioTaskCfg->sGlobalTaskConfig);
			ui32NumNodes = psAudioGblTaskCfg->ui32NumberOfNodesInTask;
			psNodeCfg = &(psAudioTaskCfg->sNodeConfig[0]);
			BDSP_Raaga_P_Analyse_CIT_Audio_GlobalTaskConfig(pRaagaTask);
			break;
		case BDSP_ContextType_eScm:
			psScmGblTaskCfg = &(pRaagaTask->scmCitOutput.sScmCit.sGlobalTaskConfig);
			ui32NumNodes = psScmGblTaskCfg->ui32NumberOfNodesInTask;
			psNodeCfg = &(pRaagaTask->scmCitOutput.sScmCit.sNodeConfig[0]);
			BDSP_Raaga_P_Analyse_CIT_Scm_GlobalTaskConfig(psScmGblTaskCfg);
			break;
		case BDSP_ContextType_eVideo:
			psVideoDecGblTaskCfg = &(pRaagaTask->videoCitOutput.uVideoCit.sVideoDecTaskConfig.sGlobalTaskConfig);
			ui32NumNodes = psVideoDecGblTaskCfg->ui32NumberOfNodesInTask;
			psNodeCfg = &(pRaagaTask->videoCitOutput.uVideoCit.sVideoDecTaskConfig.sNodeConfig[0]);
			BDSP_Raaga_P_Analyse_CIT_VideoDecode_GlobalTaskConfig(psVideoDecGblTaskCfg);
			break;
		case BDSP_ContextType_eVideoEncode:
			psVideoEncGblTaskCfg = &(pRaagaTask->videoCitOutput.uVideoCit.sVideoEncTaskConfig.sEncGlobalTaskConfig);
			ui32NumNodes = psVideoEncGblTaskCfg->ui32NumberOfNodesInTask;
			psNodeCfg = &(pRaagaTask->videoCitOutput.uVideoCit.sVideoEncTaskConfig.sNodeConfig[0]);
			BDSP_Raaga_P_Analyse_CIT_VideoEncode_GlobalTaskConfig(psVideoEncGblTaskCfg);
			break;
		case BDSP_ContextType_eGraphics:
		case BDSP_ContextType_eMax:
		default:
			BDBG_MSG(("*****Trying to print CIT for a task of Invalid Context*******"));
			return;
	}

    BDBG_MSG(("Node Configuration "));
	BDBG_MSG(("Num of Nodes: %d ",ui32NumNodes));
	BDBG_MSG(("-------------------------------------------- "));
    for(ui32Node=0; ui32Node<ui32NumNodes; ui32Node++)
    {
        BDBG_MSG(("Node index: %d ",psNodeCfg->uiNodeId));
        BDBG_MSG(("Algo Id: %s ",AlgoIdEnum2Char[psNodeCfg->eAlgoId]));
		BDSP_Raaga_P_Analyse_CIT_NodeConfig(psNodeCfg);
		BDBG_MSG(("-------------------------------------------- "));
		psNodeCfg++;
    }
	BDBG_MSG(("============================================ "));
    BDBG_LEAVE(BDSP_Raaga_P_Analyse_CIT);
}
