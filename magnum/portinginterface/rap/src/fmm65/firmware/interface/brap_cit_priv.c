/***************************************************************************
*     Copyright (c) 2003-2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
*  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
*  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:	This is the main file for the CIT generation module.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/* HEADER FILE INCLUSION */

#ifdef TESTBENCH_BUILD
	
	/* Test bench File Inclusion */
	#include "stdio.h"
	#include "stdlib.h"
	#include "stdarg.h"
	#include "string.h"
	#include "data_types.h"
	#include "bchp_aud_dsp_cfg0.h"
	#include "bchp_aud_fmm_bf_ctrl.h"
	#include "brap_buff_defs_7043.h"
	
	#define BDBG_ERR(x) printf x; printf("\n")
	#define BDBG_MSG(x) printf x; printf("\n")

	#define	BKNI_Malloc malloc
	#define	BKNI_Free	free
	#define	BKNI_Memset memset
#else

	/* PI header File Inclusion */
	#include "brap.h"
	#include "brap_priv.h"
	#include "brap_dspchn.h"	

	BDBG_MODULE(rap_cit);

#endif

/* CIT-Gen  header File Inclusion */
#include "brap_af_priv.h"
#include "brap_fwstatus_priv.h"
#include "brap_cit_priv.h"

/*--------------------------------------------*/
/* Arrays instantiated in brap_af_priv.c file */
/*--------------------------------------------*/

extern const BRAP_AF_P_sNODE_INFO		BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMax];
extern const BRAP_AF_P_sALGO_EXEC_INFO	BRAP_sDecAlgoExecInfo[BRAP_DSPCHN_VideoType_eMax][BRAP_DSPCHN_DecodeMode_eMax];
extern const BRAP_AF_P_sALGO_EXEC_INFO	BRAP_sEncAlgoExecInfo[BRAP_CIT_P_EncAudioType_eMax][BRAP_DSPCHN_EncodeMode_eSimulMode];
extern const BRAP_AF_P_sALGO_EXEC_INFO	BRAP_sPpAlgoExecInfo[BRAP_CIT_P_ProcessingType_eMax][BRAP_DSPCHN_ProcessingMode_eMax];
extern const BRAP_AF_P_sALGO_EXEC_INFO  BRAP_sGfxDecodeExecInfo[BRAP_DSPCHN_GfxType_eMax];
extern const BRAP_AF_P_sALGO_EXEC_INFO  BRAP_sScmDecodeExecInfo[BRAP_DSPCHN_ScmType_eMax];


/*--------------------------------------------------*/
/*    Static Memory Allocations for CIT-Gen Module  */
/*--------------------------------------------------*/

#define MAX_CHAR_LENGTH		80

/* Static Memory allocation for CIT */
static BRAP_CIT_P_BranchInfo sCitStaticBranchInfo[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK];

/* Arrays for :- Display Messages */
static const char BuffTypeEnum2Char[BRAP_AF_P_BufferType_eLAST][MAX_CHAR_LENGTH] =
{
	{"DRAM Circular Buffer"},
	{"RDB"},
	{"FMM Buffer"},
	{"RAVE Buffer"},
	{"Inter-Stage Buffer"},
	{"FMM Slave buffer"}
};

static const char FwConnectivityType[BRAP_CIT_P_FwStgSrcDstType_eMax][MAX_CHAR_LENGTH] =
{
	{"Firmware Stage Connection"},		
	{"Rave Buffer"},
	{"FMM Buffer"},	
	{"RDB Buffer"},
	{"DRAM Buffer"},
	{"InterTask DRAM Buffer"}
};


static const char PortDatatType[BRAP_AF_P_DistinctOpType_eMax][MAX_CHAR_LENGTH] = 
{	
	{"Mix PCM 7.1 Channel"},
	{"Mix PCM 5.1 Channel"},
	{"Mix Stereo PCM"},	
	{"PCM 7.1 Channel"},
	{"PCM 5.1 Channel"},
	{"Stereo PCM"},	
	{"Compressed Data"},			
	{"Auxilary Data Out"},		
	{"Generic Interstage Data"},
	{"DolbyReEncodeAuxDataOut"}

};

static const char PortValidType[BRAP_AF_P_ValidInvalid_eMax][MAX_CHAR_LENGTH] = 
{	
	{"INVALID"},
	{"VALID"}
};

static const char AlgoIdEnum2Char[BRAP_AF_P_AlgoId_eMax+1][MAX_CHAR_LENGTH] =
{
	{"Mpeg Decode"},
	{"Ac3 Decode Stage1"},
	{"Ac3 Decode Stage2"},
	{"Aac Decode Stage1"},
	{"Aac Decode Stage2"},
	{"AacHe LpSbr Decode Stage0"},
	{"AacHe LpSbr Decode Stage1"},
	{"AacHe LpSbr Decode Stage2"},
	{"AacHe LpSbr Decode Stage3"},
	{"AacHe LpSbr Decode Stage4"},
	{"Ddp Decode Stage1"},
	{"Ddp Decode Stage2"},
	{"DdLossless Decode Stage1"},
	{"DdLossless Decode Stage2"},
	{"Lpcm Custom Decode"},
	{"BdLpcm Decode"},
	{"DvdLpcm Decode"},
	{"HdDvdLpcm Decode"},
	{"MpegMc Decode"},
	{"WmaStd Decode"},
	{"WmaProStd Decode Stage1"},
	{"WmaProStd Decode Stage2"},
	{"Mlp Decode"},
	{"Ddp71 Decode Stage1"},
	{"Ddp71 Decode Stage2"},
	{"Dts Decode Stage1"},
	{"Dts Decode Stage2"},
	{"Dts Decode Stage3"},
	{"DtsLbr Decode Stage1"},
	{"DtsLbr Decode Stage2"},
	{"DtsLbr Decode Stage3"},
	{"DtsHd Decode Stage1"},
	{"DtsHd Decode Stage2"},
	{"DtsHd Decode Stage3"},
	{"DtsHd Decode Stage4"},
	{"DtsHd Decode Stage5"},
	{"Pcm Wave Decode"},
	{"Amr Decode"},
	{"Dra Decode"},
	{"Real Audio LBR Decode"},
	{"Dolby Pulse Decode Stage0"},
	{"Dolby Pulse Decode Stage1"},
	{"Dolby Pulse Decode Stage2"},
	{"Dolby Pulse Decode Stage3"},
	{"Ms10 DdpDecodeFE"},
	{"Ms10 DdpDecodeBE"},
	{"Mpeg FrameSync"},
	{"MpegMc FrameSync"},
	{"Adts FrameSync"},
	{"Loas FrameSync"},
	{"WmaStd FrameSync"},
	{"WmaPro FrameSync"},
	{"Ac3 FrameSync"},
	{"Ddp FrameSync"},
	{"Ddp71 FrameSync"},
	{"Dts FrameSync"},
	{"DtsLbr FrameSync"},
	{"DtsHd FrameSync"},
	{"DtsHd FrameSync_1"},
	{"DtsHdHdDvd FrameSync"},
	{"DdLossless FrameSync"},
	{"Mlp FrameSync"},
	{"MlpHdDvd FrameSync"},
	{"Pes FrameSync"},
	{"BdLpcm FrameSync"},
	{"HdDvdLpcm FrameSync"},
	{"DvdLpcm FrameSync"},
	{"DvdLpcm FrameSync_1"},
	{"PcmWave FrameSync"},
	{"Amr FrameSync"},
	{"Dra FrameSync"},
	{"Real Audio LBR FrameSync"},
	{"Ms10 Ddp FrameSync"},
	{"MixerTask FrameSync"},
	{"Decode Tsm"},
	{"Encode Tsm"},
	{"Passthrough Tsm"},
	{"System Decode Task"},
	{"System Graphics Task"},
	{"System Video Task"},
	{"System Scm Task"},
	{"Ac3 Encode"},
	{"MpegL2 Encode Stage1"},
	{"MpegL2 Encode Stage2"},
	{"MpegL3 Encode Stage1"},
	{"MpegL3 Encode Stage2"},
	{"AacLc Encode Stage1"},
	{"AacLc Encode Stage2"},
	{"AacHe Encode Stage1"},
	{"AacHe Encode Stage2"},
	{"AacHe Encode Stage3"},
	{"Dts Encode"},
	{"Dts Broadcast Encode"},
	{"SBC Encode"},
	{"DD Transcode"},
	{"Ac3 Enc FrameSync"},
	{"MpegL3 Enc FrameSync"},
	{"MpegL2 Enc FrameSync"},
	{"AacLc Enc FrameSync"},
	{"AacHe Enc FrameSync"},
	{"Dts Enc FrameSync"},	
	{"Pass Thru"},
	{"Pass Thru Aux Data Generation Stage"},
	{"Srs TruSurround PostProc"},
	{"Src PostProc"},
	{"Ddbm PostProc"},
	{"Downmix PostProc"},
	{"Custom Surround PostProc"},
	{"Custom Bass PostProc"},
	{"Karaoke Capable PostProc"},
	{"Custom Voice PostProc"},
	{"Peq PostProc"},
	{"Avl PostProc"},
	{"Pl2 PostProc"},
	{"Xen PostProc"},
	{"Bbe PostProc"},
	{"Dsola PostProc"},
	{"DtsNeo PostProc"},
	{"DDConvert PostProc"},
	{"AudioDescriptor Fade "},
	{"AudioDescriptor Pan"},
	{"PCM Router Module"},
	{"WMA PassThrough"},
	{"SrsTru SurroundHDPostProc"},
	{"SrsTru Volume PostProc"},
	{"Dolby Volume PostProc"},
	{"Audyssey VolumePostProc"},
	{"Brcm3D SurroundPostProc"},
	{"FwMixer PostProc"},
	{"MonoDownMix PostProc"},
	{"Ms10 DDConvert"},
	{"Audyssey Abx PostProc"},
	{"SrsCircleSurrPostProc"},
	{"SrsEqualizerPostProc"},
	{"DdrePostProc"},
	{"Dv258PostProc"},
	{"BtscEncPostProc"},
	{"PostProc FrameSync"},
	{"Gfx Decode"},
	{"RealVideo9PLF"},		
	{"RealVideo9Stage1"},
	{"RealVideo9Stage2"},
	{"RealVideo9Stage3"},
	{"RealVideo9PPD"},
	{"Scm1 Processing "},	
	{"Scm2 Processing "},	

};

static const char DecodeEncPPAlgoTypeName[BRAP_AF_P_DecodeEncPPAlgoType_eMax][MAX_CHAR_LENGTH] =
{
	{"Mpeg L1"},
	{"AacLC"},
	{"AacLC"},
	{"AacSbr"},
	{"AacSbr"},
	{"Ac3"},
	{"Ac3Plus"},
	{"Dts"},
	{"LpcmBd"},
	{"LpcmHdDvd"},
	{"Dtshd"},
	{"LpcmDvd"},
	{"WmaStd"},
	{"Ac3Lossless"},
	{"Mlp"},
	{"Pcm"},
	{"DtsLbr"},
	{"Ddp7_1"},
	{"MpegMc"},
	{"WmaPro"},
	{"DtshdSub"},
	{"LpcmDvdA"},
	{"DtsBroadcast"},
	{"PcmWav"},
	{"Amr"},
	{"Dra"},
	{" "},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"MpegL2"},
	{"MpegL3"},
	{"Ddbm"},
	{"DtsNeo"},
	{"AVL"},
	{"DDConvert"},
	{"PLll"},
	{"SrsXt"},
	{"Xen"},
	{"Bbe"},
	{"Src"},
	{"CustomSurround"},
	{"CustomBass"},
	{"CustomVoice"},
	{"Peq"},
	{"Downmix0 	"},
	{"AudioDescriptorFade"},
	{"AudioDescriptorPan"},
	{"PCMRouter"},
	{"WMAPassThrough"},
	{"Dsola"},
	{"SrsHd"},
	{"GenericPassThru"},
	{"SrsTruVolume"},
	{"DolbyVolume"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"Mpeg1Layer3 Encode"},
	{"Mpeg1Layer2 Encode"},
	{"DTS Encode"},
	{"AacLc Encode"},
	{"AacHe Encode"},
	{"Ac3 Encode"},
	{"DTSBroadcast Encode"},
	{"Sbc Encode"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	{"	"},
	
};

static const char FmmDstName[2][MAX_CHAR_LENGTH] =
{
	{"Output Port"},
	{"Capture Port"},
};

static const char PortName[22][MAX_CHAR_LENGTH] =
{
	{"BRAP_OutputPort_eSpdif"},
	{"BRAP_OutputPort_eDac0"},
	{"BRAP_OutputPort_eI2s0"},
	{"BRAP_OutputPort_eI2s1"},
	{"BRAP_OutputPort_eI2s2"},
	{"BRAP_OutputPort_eDac1"},
	{"BRAP_OutputPort_eMai"},
	{"BRAP_OutputPort_eFlex"},
	{"BRAP_OutputPort_eRfMod"},
	{"BRAP_OutputPort_eI2s3"},
	{"BRAP_OutputPort_eI2s4"},
	{"BRAP_OutputPort_eSpdif1"},
	{"BRAP_OutputPort_eI2s5"},
	{"BRAP_OutputPort_eI2s6"},
	{"BRAP_OutputPort_eI2s7"},
	{"BRAP_OutputPort_eI2s8"},
	{"BRAP_OutputPort_eDac2"},
	{"BRAP_OutputPort_eMaiMulti0"},
	{"BRAP_OutputPort_eMaiMulti1"},
	{"BRAP_OutputPort_eMaiMulti2"},
	{"BRAP_OutputPort_eMaiMulti3"},	
	{"BRAP_OutputPort_eMax"}
};

static const char CapPortName[14][MAX_CHAR_LENGTH] =
{
	{"Capture Port0"},		/* Internal Capture port 0 */
    {"Capture Port1"},		/* Internal Capture port 1 */
    {"Capture Port2"},		/* Internal Capture port 2 */
    {"Capture Port3"},		/* Internal Capture port 3 */
    {"CapInputPort_eExtI2s0"},         /* External I2S Capture port */
    {"CapInputPort_eRfAudio"},         /* RF Audio Capture port */
    {"CapInputPort_eSpdif"},           /* SPDIF Capture port */
    {"CapInputPort_eHdmi"},            /* HDMI */
    {"CapInputPort_eAdc"},    
    {"Capture Port4"},     /* Internal Capture port 4 */
    {"Capture Port5"},     /* Internal Capture port 5 */
    {"Capture Port6"},     /* Internal Capture port 6 */
    {"Capture Port7"},     /* Internal Capture port 7 */
    {"CapInputPort_eMax"}  /* Invalid/last Entry */
};

static const char FrequencyRate[5][MAX_CHAR_LENGTH] =
{
	{"Base Rate"},		
    {"Stream Sampling Rate"},
    {"2x BaseRate"},	
    {"4x/HBR BaseRate"},
    {"Invalid Rate"},    
};



static const char Spdif_Content[2][MAX_CHAR_LENGTH] =
{												
	{" PCM on SPDIF "},
	{" Compressed on SPDIF "}
};	

static const char Mai_Payload[2][MAX_CHAR_LENGTH] =
{												
	{" PCM  "},
	{" Compressed "}
};	


static const char Spdif_HbrMode[2][MAX_CHAR_LENGTH] =
{												
	{" HBR Mode Off "},
	{" HBR Mode On "},
};	
											
static const char Spdif_HWCbitsMode[2][MAX_CHAR_LENGTH] =
{												
	{" FW CBIT Mode"},
	{" HW CBIT Mode "},
};
	
static const char DisableEnable[2][MAX_CHAR_LENGTH] =
{												
	{" Disabled "},
	{" Enabled "}
};

static const char PortEnableDisable[2][MAX_CHAR_LENGTH] =
{	
	{" Enabled "},
	{" Disabled "}	
};													

static const char GlobalTimeBase [2][MAX_CHAR_LENGTH] =
{												
	{" 45 Khz "},
	{" 27 Mhz "}
};		

static const char FmmDstType [3][MAX_CHAR_LENGTH] =
{												
	{"FmmDstSelect_eOpPort"},
	{"FmmDstSelect_eCapturePort"},
	{"FmmDstSelect_eMax"}
};		

/*---------------------------------------------------------------*/
/* Prototype declarations Functions inside CIT Generation Module */
/*---------------------------------------------------------------*/

static uint32_t  BRAP_CITGEN_P_DetermineAlgoExecOrder(	
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder			sAlgoExecOrder[BRAP_P_MAX_ALGOS_IN_TASK]
		);

static	void BRAP_CITGEN_P_PrepareAlgoNwStruct( 
			BRAP_CIT_P_InputInfo				*psCitIp,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo 
		);

static void BRAP_CITGEN_P_DetectSpecialTasks( 						
			BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
			BRAP_CIT_P_sSpecialTaskPresent	*psSpecialTaskPresent

		);					

static uint32_t BRAP_CITGEN_P_GetNodesInAlgo(	
			BRAP_CIT_P_sCitDataStructure		*psCitDataStructure
		);

static uint32_t BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
			BMEM_Handle							hHeap,
			BRAP_Mit							*psMit,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo 
		);

static void BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
			const BRAP_AF_P_sNODE_INFO			BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMax],
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo 
		);


static uint32_t BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo
		);

static void BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes( 
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo
		);

static uint32_t BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
			uint32_t							ui32IfBuffBaseAddr,
			uint32_t							ui32UsrCfgBuffBaseAddr,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo 
		);

static uint32_t BRAP_CITGEN_P_ComputeInterStgBuffSize(
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo 
		);

static void BRAP_CITGEN_P_ComputeScratchBuffSize(
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo 
		);

static uint32_t BRAP_CITGEN_P_AllocIoBuffCfg(
			BMEM_Handle							hHeap,
			uint32_t							ui32InterStgBuffBaseAddr,
			uint32_t							ui32IoBuffCfgStructBaseAddr,
			BRAP_CIT_P_sAlgoExecOrder			sAlgoExecOrder[],
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sOpPortInfo				*psOpPortInfo,
			BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG *psTaskFmmGateOpenConfig,
			uint32_t							ui32TaskRdbOffset,
			BRAP_CIT_P_sCitDataStructure		*psCitDataStructure,
			BRAP_AF_P_EnableDisable				eIndepDelayEnableFlag,
			BRAP_CIT_P_SameInputForFsAndDecode	eSameInputForFsAndDecode,
			BRAP_CIT_P_sSpecialTaskPresent		*psSpecialTaskPresent
		);


static void BRAP_CITGEN_P_PrepareIsIoBuffCfgStruct( 
			uint32_t							ui32InterStgBuffBaseAddr,
			BRAP_CIT_P_sTaskBuffInfo			*psTaskBuffInfo,
			BRAP_CIT_P_sBuffCfg					*psIsBuffCfg 
		);

static uint32_t BRAP_CITGEN_P_PopulatePpmCfg(
			BRAP_CIT_P_sIO_BUFF_ID				*psIoBufferId,
			BRAP_AF_P_sPPM_CFG					psPpmCfgArray[],
			uint32_t							ui32TaskRdbOffset,
			uint32_t							*pui32PPMCount
		);

static void BRAP_CITGEN_P_TransPpmBuffId2Addr(
			uint32_t							ui32BufferId,
			uint32_t							ui32TaskRdbOffset,
			uint32_t							*pui32PPMCfgAddr
		);


static void BRAP_CITGEN_P_InitializePpmCfg(
			BRAP_AF_P_sPPM_CFG					sPpmCfgArray[]
		);


static void BRAP_CITGEN_P_FwHwBufferCfg(
			BMEM_Handle							hHeap,
			BRAP_AF_P_sFW_HW_CFG				*psFwHwCfgIp,
			uint32_t							ui32TaskFwHwCfgAddr
		);


static uint32_t BRAP_CITGEN_P_GenFreeInterstageBuffer(
			BRAP_CIT_P_TaskIsBuffCfg			*psTaskIsBuffCfg,
			uint32_t							ui32NumInterStgBuffs,
			uint32_t							*pui32IsBufferCount
		);

static uint32_t BRAP_CITGEN_P_FillSamplingFrequencyMapLut(
			BMEM_Handle							hHeap,
			uint32_t							ui32FwOpSamplingFreqMapLutAddr,
			BRAP_CIT_P_sAlgoModePresent			*psAlgoModePresent
		);


static void BRAP_CITGEN_P_WriteIsIoBuffCfgStructToDram(
			BMEM_Handle							hHeap,
			uint32_t							ui32NumInterStgBuffs,
			uint32_t							ui32IoBuffCfgStructBaseAddr, 
			BRAP_CIT_P_sBuffCfg					sIsBuffCfg[] 
		);

static void BRAP_CITGEN_P_WriteToDRAM( 
			BMEM_Handle							hHeap,
			uint32_t							ui32SrcAddr,
			uint32_t							ui32DestAddr,
			uint32_t							ui32Size 
		);


static void BRAP_CITGEN_P_AllocScratchBuff(	
			uint32_t							ui32ScratchBuffBaseAddr,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo 
		);


static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoCit(	
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder			sAlgoExecOrder[],
			BRAP_AF_P_sTASK_CONFIG				*psCit 
		);


static uint32_t BRAP_CITGEN_P_UpdtNodeCfgInCit (
			BRAP_CIT_P_sNodeAttr				*psNodeAttr,
			BRAP_AF_P_sNODE_CONFIG				*psNodeCfg 
		);

static uint32_t BRAP_CITGEN_P_FillGblTaskCfgIntoCit(	
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_sOpPortInfo				*psOpPortInfo,
			uint32_t							ui32ScratchBuffBaseAddr,
			uint32_t							ui32TaskScratchMemSize,
			BRAP_AF_P_sGLOBAL_TASK_CONFIG		*psGblTaskCfg,
			uint32_t							ui32TaskPortConfigAddr,
			uint32_t							ui32TaskGateOpenConfigAddr,
			uint32_t							ui32TaskFwHwCfgAddr,
			uint32_t							ui32FwOpSamplingFreqMapLutAddr,
			uint32_t							ui32ZeroFillSamples,
			BRAP_AF_P_TimeBaseType				eTimeBaseType
		);

static uint32_t BRAP_CITGEN_P_FillOpForkCfg(
			BRAP_CIT_P_sNodeAttr				*psNodeAttr,
			BRAP_AF_P_sNODE_CONFIG				*psNodeCfg
		);

static void BRAP_CITGEN_P_FillCitOp(
			BMEM_Handle							hHeap,
			BRAP_AF_P_sDRAM_BUFFER				*psTaskStackBuff,
			BRAP_AF_P_sDRAM_BUFFER				*psTaskSPDIFPortConfigAddr,
			BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
			BRAP_CIT_P_Output					*psCitOp 
		);


static uint32_t BRAP_CITGEN_P_Max(
			uint32_t							a,
			uint32_t							b
		);

static uint32_t BRAP_CITGEN_P_TransFmmBuffId2Addr(
			BRAP_CIT_P_sIO_BUFF_ID				*psIoBuffId,
			BRAP_AF_P_sIO_BUFFER				*psIoBuffer,
			BRAP_CIT_P_sIO_BUFF_ID				*psIoGenericBuffId,
			BRAP_AF_P_sIO_GENERIC_BUFFER		*psIoGenericBuffer,
			uint32_t							ui32TaskRdbOffset,
			BRAP_CIT_P_MasterSlaveType			eMasterSlaveType
		);


static uint32_t BRAP_CITGEN_P_VerifyFmmFsRate(
			BRAP_AF_P_FmmDstFsRate				eFmmDstFsRate
		);

static uint32_t BRAP_CITGEN_P_TransRdbBuffId2Addr(
			BRAP_CIT_P_sIO_BUFF_ID				*psIoBuffId,
			BRAP_AF_P_sIO_BUFFER				*psIoBuffer,
			BRAP_CIT_P_sIO_BUFF_ID				*psIoGenericBuffId,
			BRAP_AF_P_sIO_GENERIC_BUFFER		*psIoGenericBuffer,
			uint32_t							ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_DummyIOBufferCfg(			
			BRAP_AF_P_sIO_BUFFER				*psIoBuffer,			
			BRAP_AF_P_sIO_GENERIC_BUFFER		*psIoGenericBuffer
		);


static void BRAP_CITGEN_P_ReadFromDram(
			uint32_t							ui32SrcAddr,
			uint32_t							ui32DestAddr,
			uint32_t							ui32Size
		);

static uint32_t BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo 
		);


static uint32_t BRAP_CITGEN_P_AllocFwStatusBuff(
			uint32_t								ui32FwStatusBuffBaseAddr,					
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo 
		);


static void BRAP_CITGEN_P_ComputeTaskStackBuffSize(					
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo
		);


static void BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize(
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo
		);



static uint32_t BRAP_CITGEN_P_AllocSpdifPortCfgBuff(	
			uint32_t								ui32TaskAllSPDIF_PortConfigAddr,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskSPDIFPortConfigAddr
		);


static void BRAP_CITGEN_P_PopulateGateOpenCfg(
			BRAP_CIT_P_sIO_BUFF_ID					*psIoBuffId,
			BRAP_AF_P_sIO_BUFFER					*psIoBuffer,
			BRAP_AF_P_sFMM_GATE_OPEN_CONFIG			*psGateOpenCfg,
			uint32_t								ui32IoPortCfgStructAddr,
			uint32_t								ui32IndepDelay,
			BRAP_AF_P_DistinctOpType				eDistinctOpType,
			BRAP_AF_P_FmmDstFsRate					eFmmDstFsRate
		);

static void BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize(
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo
		);

static void BRAP_CITGEN_P_ComputeHwFwCfgBufferSize(
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo
		);

static void BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize(
			BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo
		);

static void BRAP_CITGEN_P_WriteGateOpenCfgToDram(
			BMEM_Handle								hHeap,
			BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG	*psTaskFmmGateOpenConfig,
			uint32_t								ui32TaskGateOpenConfigAddr,
			uint32_t								ui32Size
		);


static uint32_t  BRAP_CITGEN_P_GetNumZeroFillSamples(
			uint32_t								*pui32ZeroFillSamples,
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo
		);

static void BRAP_CITGEN_P_PIInputPortCfgDBG(
			BRAP_CIT_P_InputInfo					*psCitIp
		);

static void BRAP_CITGEN_P_PPMChannelCfgDBG(
			BRAP_AF_P_sFW_HW_CFG					*psFwHwCfg
		);

static void BRAP_CITGEN_P_GetCitInputRdbCfg(
			BRAP_CIT_P_InputInfo					*psCitIp,
			BRAP_CIT_P_RdbConfig					*psRdbCfg
		);

static void BRAP_CITGEN_P_PrepareTaskRdbCfg(
			BRAP_CIT_P_RdbConfig					*psRdbCfg
		);


static void BRAP_CITGEN_P_InitializeCitDatastructue(											
			BRAP_CIT_P_sCitDataStructure			*psCitDataStructure
		);

static uint32_t BRAP_CITGEN_P_AllocateCitDatastructure(											
			BRAP_CIT_P_sCitDataStructure			**psCitDataStructure
		);

static void  BRAP_CITGEN_P_FreeCitDatastructure(											
			BRAP_CIT_P_sCitDataStructure			**psCitDataStructure
		);

static uint32_t BRAP_CITGEN_P_TaskOutputPortCfg(
			BMEM_Handle								hHeap,
			BREG_Handle								hRegister,
			BRAP_CIT_P_InputInfo					*psCitIp,
			uint32_t								ui32TaskPortConfigAddr,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskSPDIFPortConfigAddr,
			uint32_t								ui32TaskRdbOffset,
			BRAP_CIT_P_sAlgoModePresent				*psAlgoModePresent
		);


static void BRAP_CITGEN_P_InitializeFmmDstCfg(
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t  BRAP_CITGEN_P_FindAndSelectPllToFw(
			BRAP_CIT_P_sPortCfg						*psPortCfg,
			uint32_t								*pui32PllIndex										
		);


static uint32_t BRAP_CITGEN_P_PrepareOutputPortAssociation(
			BRAP_CIT_P_InputInfo					*psCitIp,
			uint32_t								ui32NumPlls,
			BRAP_CIT_P_sPllPortAssociation			*psPllPortAssociation,
			BRAP_AF_P_PortEnableDisable				*peDacOnI2S
		);

static uint32_t BRAP_CITGEN_P_PortConfiguration(
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg,
			BRAP_CIT_P_sPllPortAssociation			*psPllPortAssociation,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskSPDIFPortConfigAddr,
			uint32_t								ui32TaskRdbOffset,
			BRAP_CIT_P_sAlgoModePresent				*psAlgoModePresent
		);


static void BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(		
			BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sCitDataStructure			*psCitDataStructure
		);

static uint32_t BRAP_CITGEN_P_DAC_Configuration(
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg,
			BRAP_CIT_P_sPllPortAssociation			*psPllPortAssociation,
			uint32_t								ui32TaskRdbOffset
		);


static void BRAP_CITGEN_P_I2SOnMai_Spdif1NormalCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG	*psFmmDestSpdifClkCbitCfg,
			uint32_t								ui32SpdifCfgDramBufferAddress,
			uint32_t								ui32SpdifContent,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_Spdif1HwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG	*psFmmDestSpdifClkCbitCfg,
			uint32_t								ui32SpdifCfgDramBufferAddress,
			uint32_t								ui32SpdifContent,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_Spdif1FwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG	*psFmmDestSpdifClkCbitCfg,
			uint32_t								ui32SpdifCfgDramBufferAddress,
			uint32_t								ui32SpdifContent,
			uint32_t								ui32TaskRdbOffset
		);

static uint32_t BRAP_CITGEN_P_PllCfg(
			BRAP_AF_P_sFMM_DEST_PLL_CFG				*psFmmDestPllCfg,
			uint32_t								ui32PllIndex,
			uint32_t								ui32TaskRdbOffset
		);

static uint32_t BRAP_CITGEN_P_NcoCfg(
			BRAP_AF_P_sFMM_DEST_NCO_CFG				*psFmmDestNcoCfg,						
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_Spdif0FwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG	*psFmmDestSpdifClkCbitCfg,
			uint32_t								ui32SpdifCfgDramBufferAddress,
			uint32_t								ui32SpdifContent,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_Spdif0HwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG	*psFmmDestSpdifClkCbitCfg,
			uint32_t								ui32SpdifCfgDramBufferAddress,
			uint32_t								ui32SpdifContent,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_CapPortNCfg(
			BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG	*psFmmDestI2SClkCfg,
			uint32_t								ui32TaskRdbOffset,
			uint32_t								ui32CapPortIndex
		);



static uint32_t BRAP_CITGEN_P_MaiPortConfiguration(
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg,
			BRAP_CIT_P_sMaiPortInfo					*psMaiPortInfo,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskSPDIFPortConfigAddr,
			BRAP_AF_P_FmmDstFsRate					*peHdmiFsRate ,
			uint32_t								*pui32HwCbitsEnable,
			uint32_t								ui32DDP_PassThruPresent,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_I2SCfg(
			BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG			*psFmmDestI2SClkCfg,
			uint32_t								ui32TaskRdbOffset
		);

static void BRAP_CITGEN_P_I2S_MultiCfg(
			BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG			*psFmmDestI2SClkCfg,
			uint32_t								ui32TaskRdbOffset
		);


static uint32_t BRAP_CITGEN_P_CalcThreshold(			
			BRAP_CIT_P_sCitDataStructure			*psCitDataStructure,
			uint32_t								*pui32Threshold
		);


static void  BRAP_CITGEN_P_ConfigureExternalClockSource(								
			BREG_Handle								hRegister,
			uint32_t								ui32PllIndex						
		);

#if BTSC_HACK_ENABLED
static void  BRAP_CITGEN_P_OverSampleHifiDac(								
			BREG_Handle								hRegister,								
			BRAP_AF_P_EnableDisable					eEnableDisable
		);
#endif

/*- Optional Debug Function Prototype Declarations in CIT-Genn Module ---*/

#ifdef ANALYSIS_IO_GEN_ENABLE

static void BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(
			BRAP_AF_P_sIO_BUFFER					*psIoBuffStruct
		);

static void BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(
			BRAP_AF_P_sIO_GENERIC_BUFFER			*psToGenericBuffStruct
		);
#endif

/*----- SPS (Seamless Port Swithching)API Function Prototype ------------*/


static uint32_t BRAP_SPS_P_I2S0PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t BRAP_SPS_P_I2S1PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);


static uint32_t BRAP_SPS_P_Spdif0PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t BRAP_SPS_P_Spdif1PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);


static uint32_t BRAP_SPS_P_Dac0PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t BRAP_SPS_P_Dac1PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t BRAP_SPS_P_Dac2PortReconfig(
			BRAP_OutputPort							eOpPortType,
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg
		);

static uint32_t BRAP_SPS_P_MaiPortReconfig(
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg,
			BRAP_OutputPort							eMaiMuxSelector
	   );


static uint32_t BRAP_SPS_P_CapturePortNReconfig(
			BRAP_AF_P_PortEnableDisable				ePortEnableDisableFlag,
			BRAP_AF_P_sFMM_DEST_CFG					*psFmmDestCfg,
			uint32_t								ui32CapPortIndex
		);

static uint32_t BRAP_SPS_P_ReconfigureFmmPortCfg(
			BREG_Handle								hRegister,
			BRAP_SPS_InputPortCfg					*psInputPortCfg, 			
			BRAP_AF_P_sFMM_DEST_CFG					*psDummytaskFMMDestCfgAddr
		);


static uint32_t BRAP_SPS_P_CopyCitFmmDestCfgToWorkingBuff(							
			uint32_t								ui32CitTaskPortCfgAddr,
			BRAP_AF_P_sFMM_DEST_CFG					*psDummytaskFMMDestCfgAddr								
		);

static uint32_t BRAP_SPS_P_GetFmmCfgAddrAndVerifyBufferSize(
			BMEM_Handle								hHeap,
			BRAP_SPS_InputInfo						*psSpsIp,
			uint32_t								*ui32TaskId,
			BRAP_AF_P_sFMM_DEST_CFG					**psWorkingTaskFmmDestCfgBuffPtr,
			uint32_t								*pui32CitTaskFmmDestCfgAddr
		);


/*------------------------- Graphics CIT Declarations -----------------------------------*/

static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoGfxCit(	
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
			BRAP_GFX_P_sTASK_CONFIG					*psGfxCit
		);


static void BRAP_CITGEN_P_FillGfxCitOp(
			BMEM_Handle								hHeap,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskStackBuff,			
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_GfxCITOutput				     *psGfxCitOp 
		);


static uint32_t  BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit( 
			BMEM_Handle								hHeap,
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_GFX_P_sGfxDecodeBuffCfg			*psGfxDecodeBuffCfg,
			BRAP_GFX_P_sGLOBAL_TASK_CONFIG			*psGfxGlobalTaskConfig											  
		);

/*------------------------------------------------------------------------------------*/


/*------------------------- Video CIT Declarations -----------------------------------*/

static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoVideoCit(	
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
			BRAP_VF_P_sTASK_CONFIG					*psVideoCit 
		);


static void BRAP_CITGEN_P_FillVideoCitOp(
			BMEM_Handle								hHeap,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskStackBuff,			
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_VideoCITOutput				*psCitOp 
		);


static uint32_t  BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit( 
			BMEM_Handle								hHeap,
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_VF_P_sVDecodeBuffCfg				*psVDecodeBuffCfg,
			BRAP_VF_P_sGLOBAL_TASK_CONFIG			*psVideoGlobalTaskConfig											  
		);

/*-------------------------Seamless Input port Switching ------------------------------*/

void BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer(
			BRAP_SIPS_InputInfo						*psSipsIp	
		);

uint32_t BRAP_SIPS_P_SearchForFreeInputPort(
			BRAP_SIPS_InputInfo						*psSipsIp,
			uint32_t								*pui32FreeIpPortIndex
		);

uint32_t BRAP_SIPS_P_HookInputPort(
			BMEM_Handle								hHeap,
			uint32_t								ui32TaskRdbOffset,
			BRAP_SIPS_InputInfo						*psSipsIp,
			uint32_t								ui32FreeIpPortIndex
		);

uint32_t BRAP_SIPS_P_AddInputPortToTask(
			BMEM_Handle								hHeap,
			uint32_t								ui32TaskRdbOffset,
			BRAP_SIPS_InputInfo						*psSipsIp,
			BRAP_SIPS_OutputInfo					*psSipsOp	
		);

uint32_t BRAP_SIPS_P_RemoveInputPortFromTask(			
			BRAP_SIPS_InputInfo						*psSipsIp,
			BRAP_SIPS_OutputInfo					*psSipsOp	
		);

uint32_t BRAP_SIPS_P_RemoveInputPort(
			BRAP_SIPS_InputInfo						*psSipsIp,
			uint32_t								ui32InputPortIndexToRemove
		);

/*------------------------------------------------------------------------------------*/



/*--------------------------- SCM CIT Declarations -----------------------------------*/

static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoScmCit(	
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
			BRAP_SCM_P_sTASK_CONFIG					*psScmCit 
		);

static uint32_t BRAP_CITGEN_P_FillGblTaskCfgIntoScmCit(	
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,						
			uint32_t								ui32ScratchBuffBaseAddr,
			uint32_t								ui32TaskScratchMemSize,
			BRAP_SCM_P_sGLOBAL_TASK_CONFIG			*psGblTaskCfg						
		);


static void BRAP_CITGEN_P_FillScmCitOp(
			BMEM_Handle								hHeap,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskStackBuff,			
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_ScmCITOutput					*psCitOp
		);

/*------------------------------------------------------------------------------------*/




#ifdef TESTBENCH_BUILD
void BRAP_P_ConvertAddressToOffset(
			BMEM_Handle								hHeap,
			void									* a,
			uint32_t								* b
		);

void BRAP_ConvertOffsetToAddress(
			 BMEM_Handle							hHeap,
			 uint32_t								ui32DestAddr,
			 void									**b
		);
#endif


/*************************************************/
/*	 CIT Generation Module's Private defines	 */
/**************************************************/
/*
	#define BRAP_CIT_P_PRINT_STAGE_PORT_CONNECTION
	#define BRAP_CIT_P_PRINT_PPM_CFG
*/
/**************************************************/


/*---------------------------------------------------------------------
				Top level CIT Generation Function 
---------------------------------------------------------------------*/

uint32_t BRAP_P_GenCit(	BMEM_Handle					hHeap,
						BREG_Handle					hRegister,
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_CIT_P_Output			*psCitOp )
{
	uint32_t ui32Err;
	uint32_t ui32PhysAddr;
	uint32_t ui32Threshold;	
	uint32_t ui32IfBuffBaseAddr;		
	uint32_t ui32TaskFwHwCfgAddr;
	uint32_t ui32TotalTaskMemory;	
	uint32_t ui32ZeroFillSamples;	
	uint32_t ui32TotalSharedMemory;
	uint32_t ui32UsrCfgBuffBaseAddr;
	uint32_t ui32TaskPortConfigAddr;
	uint32_t ui32ScratchBuffBaseAddr;
	uint32_t ui32FwStatusBuffBaseAddr;	
	uint32_t ui32InterStgBuffBaseAddr;
	uint32_t ui32TaskGateOpenConfigAddr;
	uint32_t ui32IoBuffCfgStructBaseAddr;		
	uint32_t ui32FwOpSamplingFreqMapLutAddr;
	uint32_t ui32TaskAllSPDIF_PortConfigAddr;	
	

	BRAP_CIT_P_sTaskBuffInfo				sTaskBuffInfo;
	BRAP_AF_P_sDRAM_BUFFER					sTaskStackBuff;
									
	BRAP_CIT_P_sCitDataStructure			*psCitDataStructure =NULL;
	

	BDBG_ENTER(BRAP_P_GenCit);		

	BDBG_ASSERT(hHeap);
	BDBG_ASSERT(psCitIp);
	BDBG_ASSERT(psCitOp);

	ui32Err = BERR_SUCCESS ;
	

	/*---------------------------------------------------*/
	/*Memory Allocation for the CIT Global Datastructure */
	/*---------------------------------------------------*/

	ui32Err = BRAP_CITGEN_P_AllocateCitDatastructure(&psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{	
		return ui32Err;
	}


	/* Initializing the Contents of CitDataStructure */
	BRAP_CITGEN_P_InitializeCitDatastructue(psCitDataStructure);
			
	/*	First convert the virtual address in the CIT input structure to physical 
		addresses 
	*/
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32SharedMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32SharedMemPhysAdr = ui32PhysAddr;

	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32TaskMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32TaskMemPhysAdr = ui32PhysAddr;

	/*Geting the RDB Offsets Configuration */

	BRAP_CITGEN_P_GetCitInputRdbCfg(psCitIp,
									&psCitDataStructure->sRdbCfg
								 );
	

	/*	Prepare the node network information. This function basically copies
		all the information from CIT i/p structure to AlgoNwInfo structure 
	*/
	BRAP_CITGEN_P_PrepareAlgoNwStruct(
									psCitIp, 
									&psCitDataStructure->sAlgoNwInfo 
								 );	


	/* 
		Detects special tasks in the system. Currently only detecting the Mixer Task alone.
	*/
	BRAP_CITGEN_P_DetectSpecialTasks(
									&psCitDataStructure->sAlgoNwInfo,
									&psCitDataStructure->sSpecialTaskPresent
								 );

	/*	From the algo network, decide the execution order. The order
		of execution will be indicated by linearly traversing the 
		sAlgoExecOrder structure. This structure will contain the branchId 
		and StageId of the Algo, in the order of execution 
	*/
	ui32Err = BRAP_CITGEN_P_DetermineAlgoExecOrder(
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure->sAlgoExecOrder
								 );

	if( ui32Err != BERR_SUCCESS)
	{	
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Associate the Algo ids of nodes with the algorithm in the 
		Netwrok info structure. */

	ui32Err = BRAP_CITGEN_P_GetNodesInAlgo(psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the MIT information into Algo Network Info */
	ui32Err = BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
									hHeap,
									psCitIp->psMit,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the information in the BRAP_sNodeInfo structure into the  Algo Network */
	BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
									BRAP_sNodeInfo,
									&psCitDataStructure->sAlgoNwInfo
								);

	/* Preparing the task RDB configuration */
	BRAP_CITGEN_P_PrepareTaskRdbCfg(&psCitDataStructure->sRdbCfg);



							/*---------------------------*/
							/* TASK BUFFER CONFIGURATION */
							/*---------------------------*/

	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Function to Compute the user config and inter-frame
		buffers for the nodes in the algo netwrok
	*/
	ui32Err = BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status buffer */
	/*
		Function to Compute the Firmware status buffer
		for the nodes in the algo network
	*/
	ui32Err = BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
		
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}


	/*	This function computes the worst case size required for the I/O
		buffer configuration structures in DRAM. The worst case size is
		computed, assuming each algorithm can have different i/o buffer
		structures 
	*/
	BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								 );

	/* Adding Stack swap */
	/*	Computing the Stack swap size for the task.
		Now it is hardcoded 
	*/
	BRAP_CITGEN_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

	sTaskStackBuff.ui32BufferSizeInBytes 
						= sTaskBuffInfo.ui32TaskStackMemSize;


	/* Adding port Config and SPDIF Config */
	BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize(&sTaskBuffInfo);

	/* Adding Gate open config */
	BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize(&sTaskBuffInfo);

	/* Adding FwHw config */
	BRAP_CITGEN_P_ComputeHwFwCfgBufferSize(&sTaskBuffInfo);

	/* Adding Sampling Frequency Map LUT */
	BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize(&sTaskBuffInfo);	



	/* Buffer Allocation */
	/*-------------------*/
		
	/*	A single chunk of memeory is allocated for the user
		config and the IF buffers. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalTaskMemory =
			sTaskBuffInfo.ui32TaskUsrCfgMemSize				+		
			sTaskBuffInfo.ui32TaskInterFrmMemSize			+
			sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize	+
			sTaskBuffInfo.ui32TaskFwStatusBuffMemSize		+
			sTaskBuffInfo.ui32TaskStackMemSize				+
			sTaskBuffInfo.ui32TaskPortConfigMemSize			+
			sTaskBuffInfo.ui32TaskSPDIFConfigMemSize		+
			sTaskBuffInfo.ui32TaskGateOpenConfigMemSize		+
			sTaskBuffInfo.ui32TaskHwFwCfgMemSize			+
			sTaskBuffInfo.ui32SamplingFrequencyMapLutSize	
		;


	if(ui32TotalTaskMemory > psCitIp->sMemInfo.ui32TaskMemSize)
	{
		BDBG_ERR(("ERROR: Task Memory required is more than allocated"));	
		BDBG_ERR(("\t Task Memory required= %d  Allocated memory = %d",ui32TotalTaskMemory,psCitIp->sMemInfo.ui32TaskMemSize));	
		BDBG_ERR((""));

		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}	
	
	ui32UsrCfgBuffBaseAddr		=	psCitIp->sMemInfo.ui32TaskMemPhysAdr;

	ui32IfBuffBaseAddr			=	ui32UsrCfgBuffBaseAddr				 + 
											sTaskBuffInfo.ui32TaskUsrCfgMemSize;

	ui32IoBuffCfgStructBaseAddr =	ui32IfBuffBaseAddr					 + 
											sTaskBuffInfo.ui32TaskInterFrmMemSize;

	ui32FwStatusBuffBaseAddr	=	ui32IoBuffCfgStructBaseAddr			 +
											sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize;

	sTaskStackBuff.ui32DramBufferAddress 
								=	ui32FwStatusBuffBaseAddr			 +
											sTaskBuffInfo.ui32TaskFwStatusBuffMemSize;

	/* Adding port Config and SPDIF Config */
	ui32TaskPortConfigAddr		=	sTaskStackBuff.ui32DramBufferAddress +
											sTaskStackBuff.ui32BufferSizeInBytes;

	ui32TaskAllSPDIF_PortConfigAddr 
								=	ui32TaskPortConfigAddr				 + 
											sTaskBuffInfo.ui32TaskPortConfigMemSize;
	
	/* TaskGateOpenConfig */
	ui32TaskGateOpenConfigAddr  =   ui32TaskAllSPDIF_PortConfigAddr		 +
											sTaskBuffInfo.ui32TaskSPDIFConfigMemSize;


	ui32TaskFwHwCfgAddr			=	ui32TaskGateOpenConfigAddr		 	 +
											sTaskBuffInfo.ui32TaskGateOpenConfigMemSize;

	ui32FwOpSamplingFreqMapLutAddr
								=	ui32TaskFwHwCfgAddr					 +
											sTaskBuffInfo.ui32TaskHwFwCfgMemSize;
	


	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	Assign Inter-frame and user config addresses to nodes in the
		algos of the network
	*/
	ui32Err = BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
									ui32IfBuffBaseAddr,
									ui32UsrCfgBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status*/
	/* Assign FW status  addresses to stages in the network */
	ui32Err = BRAP_CITGEN_P_AllocFwStatusBuff(
									ui32FwStatusBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								);	

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}
	
	
	/* Adding Port Config and SPDIF Config */
	ui32Err = BRAP_CITGEN_P_AllocSpdifPortCfgBuff(
									ui32TaskAllSPDIF_PortConfigAddr,
									&psCitDataStructure->sTaskSPDIFPortConfigAddr[0]/*  Array */
								);
	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*-------------------------------------------------------------------------------*/


							/*---------------------------------*/
							/* INTERSTAGE BUFFER CONFIGURATION */
							/*---------------------------------*/


	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Compute the worst case size required for Inter-stage buffers.
		The inter-stage buffers includes the inter-stage I/O buffer 
		as well as the inter-stage generic buffer
	*/
	ui32Err = BRAP_CITGEN_P_ComputeInterStgBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Compute the Worst case scratch buffer size required by any node 
		in the configuration
	*/
	BRAP_CITGEN_P_ComputeScratchBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);




	/* Buffer Allocation */
	/*-------------------*/

	/*	A single chunk of memeory is allocated for the inter-stage and scratch
		buffer. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalSharedMemory = 
			sTaskBuffInfo.ui32TaskInterStgIoMemSize			+ 
			sTaskBuffInfo.ui32TaskInterStgGenericMemSize	+
			sTaskBuffInfo.ui32TaskScratchMemSize;

	if(ui32TotalSharedMemory >	psCitIp->sMemInfo.ui32SharedMemSize)
	{

		BDBG_ERR(("ERROR: Shared Memory required is more than allocated"));	
		BDBG_ERR(("\t Shared Memory required= %d  Allocated memory = %d",ui32TotalSharedMemory,psCitIp->sMemInfo.ui32SharedMemSize));	
		BDBG_ERR((""));
		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

	  goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	ui32InterStgBuffBaseAddr	=	psCitIp->sMemInfo.ui32SharedMemPhysAdr;

	ui32ScratchBuffBaseAddr		=	ui32InterStgBuffBaseAddr					+ 
							 		sTaskBuffInfo.ui32TaskInterStgIoMemSize		+ 
									sTaskBuffInfo.ui32TaskInterStgGenericMemSize;

	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	This is the main function that allocates I/O buffer configuration.
		This function first prepares the I/O buffer configurations structures
		for inter-stage buffers. It copies the I/O port configuration
		from the sAlgoNwInfo to DRAM and updates the address into NodeAttr structure. 

		Finally it allocates which I/O buffer configuration to use for
		inter-stage buffers based on the branching information.
	*/
	ui32Err = BRAP_CITGEN_P_AllocIoBuffCfg(	hHeap,
											ui32InterStgBuffBaseAddr,
											ui32IoBuffCfgStructBaseAddr,
											psCitDataStructure->sAlgoExecOrder,
											&sTaskBuffInfo,
											&psCitDataStructure->sAlgoNwInfo,
											&psCitDataStructure->sOpPortInfo,
											&psCitDataStructure->sTaskFmmGateOpenConfig,
											psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
											psCitDataStructure,
											psCitIp->eIndepDelayEnableFlag,
											BRAP_CIT_P_SameInputForFsAndDecode_eEnable,
											&psCitDataStructure->sSpecialTaskPresent
										  );

	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Print the Input/Output port configuration for all the stages.
		This function also prints the interstage buffer used for all output ports and
		the intermediate nodes
	*/
	BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(
									psCitDataStructure->sAlgoExecOrder,
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure
								);



	/*	Allocate scratch buffer base address in sAlgoNwInfo. This function is
		written just for completion of information in the node atribute
		structure of each node 
	*/
	BRAP_CITGEN_P_AllocScratchBuff( ui32ScratchBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								 );	
	
	
						/*--------------------------------------*/
						/*				Processing				*/
						/*--------------------------------------*/

	/* Populating the FW HW configuration registers-structures */	
	BRAP_CITGEN_P_FwHwBufferCfg(	hHeap,
	  								&psCitDataStructure->sFwHwCfg,
									ui32TaskFwHwCfgAddr
								);	
											
	/*FMM port configuration */
	ui32Err = BRAP_CITGEN_P_TaskOutputPortCfg(	hHeap,
												hRegister,
												psCitIp,
												ui32TaskPortConfigAddr,	
												&psCitDataStructure->sTaskSPDIFPortConfigAddr[0], /* Array */
												psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
												&psCitDataStructure->sAlgoModePresent
											);
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*Filling the Fw Op sampling map LUT */
	ui32Err = BRAP_CITGEN_P_FillSamplingFrequencyMapLut(
									hHeap,
									ui32FwOpSamplingFreqMapLutAddr,
									&psCitDataStructure->sAlgoModePresent
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}



	/*Adding Gate open */
	BRAP_CITGEN_P_WriteGateOpenCfgToDram( 
									hHeap,
									&psCitDataStructure->sTaskFmmGateOpenConfig,
									ui32TaskGateOpenConfigAddr,
									SIZEOF(psCitDataStructure->sTaskFmmGateOpenConfig)										
								);


	/* Finding the Zero Fill Samples  */  /*Need to check whether FW is using */
	ui32Err = BRAP_CITGEN_P_GetNumZeroFillSamples(
									&ui32ZeroFillSamples,
									&psCitDataStructure->sAlgoNwInfo
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}
	

	/* Calculate the Scheduling Threshold*/  /*Need To remove */
	ui32Err = BRAP_CITGEN_P_CalcThreshold(										
									psCitDataStructure,
									&ui32Threshold
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}


	/*	Now that we have all the components of Node config, fill in the node config into CIT  */
	ui32Err = BRAP_CITGEN_P_FillNodeCfgIntoCit(	
										&psCitDataStructure->sAlgoNwInfo,
										psCitDataStructure->sAlgoExecOrder,
										&psCitOp->sCit
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	


	/*	Fill the global task configuration into CIT */
	ui32Err = BRAP_CITGEN_P_FillGblTaskCfgIntoCit( &psCitDataStructure->sAlgoNwInfo,
										 &psCitDataStructure->sOpPortInfo,
										 ui32ScratchBuffBaseAddr,
										 sTaskBuffInfo.ui32TaskScratchMemSize,
										 &psCitOp->sCit.sGlobalTaskConfig,
										 ui32TaskPortConfigAddr,
										 ui32TaskGateOpenConfigAddr,
										 ui32TaskFwHwCfgAddr,
										 ui32FwOpSamplingFreqMapLutAddr,
										 ui32ZeroFillSamples,
										 psCitIp->eTimeBaseType
									   );


	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Populate the CIT o/p structure */
	BRAP_CITGEN_P_FillCitOp(	hHeap,
								&sTaskStackBuff,
								&psCitDataStructure->sTaskSPDIFPortConfigAddr[0],
								&psCitDataStructure->sAlgoNwInfo,
								psCitOp 
						);	

	/* Debug Messages */
	BRAP_CITGEN_P_PPMChannelCfgDBG(&psCitDataStructure->sFwHwCfg);

	/*-----------------------------------------------*/
	/* CIT Generation Module is executed successfully*/
	/*-----------------------------------------------*/	
	BDBG_MSG(("CIT Generation Module is executed successfully"));
	BDBG_MSG((""));	

	ui32Err = BERR_SUCCESS;


	/* EXIT Point */
	BRAP_CITGENMODULE_P_EXIT_POINT:
	/*--------------------------------------*/
	/* Free the Cit global memory allocated */
	/*--------------------------------------*/	
	BRAP_CITGEN_P_FreeCitDatastructure(&psCitDataStructure);

	/* Check for Error and assert */
	if(ui32Err !=BERR_SUCCESS)
	{
		BDBG_ASSERT(0);
	}

	BDBG_LEAVE(BRAP_P_GenCit);		

	return ui32Err;

}


/******************************************************************************
Summary:
		Allocating memory for CIT global Datastructure and substructures
		and initialing to ZERO

Description: 
	
		Allocates and initializes following datastructures

					1) sCitDataStructure
					2) sBranchInfo

		Any specific initialization will be done in the later part of CIT 
		generation module.

		Frees up the allocated memory in case of any error occured.		

Input:	
			psCitDataStructure		
Output:	
			psCitDataStructure		
Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_AllocateCitDatastructure(											
						BRAP_CIT_P_sCitDataStructure		**psCitDataStructure
					)
{
	
	uint32_t						ui32Error;	
	uint32_t						ui32Count;
	uint32_t						ui32ErrorBranchInfoAlloc;

#ifndef BRAP_CIT_P_NEWSTATIC_MEMORY_ALLOC
	uint32_t						ui32CountFree;	
	uint32_t						ui32MaxBranchInfoAllocated;
#endif

	BRAP_CIT_P_BranchInfo			*psBranchInfo;
	BRAP_CIT_P_sCitDataStructure	*ppsCitDataStructure;


	BDBG_ENTER(BRAP_CITGEN_P_AllocateCitDatastructure);


	ui32Error					 = BERR_SUCCESS;
	ui32ErrorBranchInfoAlloc	 = BERR_SUCCESS;

	/*Allocating the CIT global Datastructure*/
	*psCitDataStructure = (BRAP_CIT_P_sCitDataStructure*)BKNI_Malloc(SIZEOF(BRAP_CIT_P_sCitDataStructure));

	if(*psCitDataStructure == NULL)
	{
		BDBG_ERR(("ERROR: Unable to allocate memory for sCitDataStructure in function BRAP_P_GenCit "));
		return BERR_OUT_OF_SYSTEM_MEMORY;
	}
	else
	{	
		/* Intializig the memory allcated with zeroes*/
		BKNI_Memset(*psCitDataStructure ,0,sizeof( BRAP_CIT_P_sCitDataStructure ));   
	}


	/*Allocating Memory for BranchInfo*/
	ppsCitDataStructure = *psCitDataStructure;


#ifdef BRAP_CIT_P_NEWSTATIC_MEMORY_ALLOC

	for(ui32Count=0;ui32Count<BRAP_P_MAX_FW_BRANCH_PER_FW_TASK;ui32Count++)
	{
		/*Gathering the memory pool */
		psBranchInfo = 	&sCitStaticBranchInfo[ui32Count];

		/*Assigning the Memory */
		ppsCitDataStructure->sAlgoNwInfo.psBranchInfo[ui32Count] = psBranchInfo;
		
		/* Intializing the memory with zeroes*/
		BKNI_Memset(psBranchInfo,0,sizeof(BRAP_CIT_P_BranchInfo));   
	}

	ui32ErrorBranchInfoAlloc = BERR_SUCCESS;

#else

	for(ui32Count=0;ui32Count<BRAP_P_MAX_FW_BRANCH_PER_FW_TASK;ui32Count++)
	{		
		psBranchInfo = (BRAP_CIT_P_BranchInfo*)BKNI_Malloc(SIZEOF(BRAP_CIT_P_BranchInfo));	
			
		if(psBranchInfo == NULL)
		{
			ui32ErrorBranchInfoAlloc = BERR_OUT_OF_SYSTEM_MEMORY;
			break;
		}
		else
		{
			/*Allocating memory*/
			ppsCitDataStructure->sAlgoNwInfo.psBranchInfo[ui32Count] = psBranchInfo;
		
			/* Intializig the memory allcated with zeroes*/
			BKNI_Memset(psBranchInfo,0,sizeof(BRAP_CIT_P_BranchInfo));   
		}		
	}

	ui32MaxBranchInfoAllocated = ui32Count;

	/*Free up memory on error */
	if(ui32ErrorBranchInfoAlloc != BERR_SUCCESS)
	{
		
		BDBG_ERR(("ERROR: Unable to allocate memory for sBranchInfo in function BRAP_P_GenCit "));

		for(ui32CountFree=0;ui32CountFree< ui32MaxBranchInfoAllocated ;ui32CountFree++)
		{
			/*Free up sBranchInfo*/
			BKNI_Free(ppsCitDataStructure->sAlgoNwInfo.psBranchInfo[ui32CountFree]);			
		}		

		/*Free up sCitDataStructure*/ /*This memory should be freed only after freeing other memory allocations*/
		BKNI_Free(ppsCitDataStructure);
	}

#endif

	ui32Error = ui32ErrorBranchInfoAlloc;


	BDBG_LEAVE(BRAP_CITGEN_P_AllocateCitDatastructure);

	return ui32Error;
}


/******************************************************************************
Summary:
		Free memory allocated for CIT Datastructures

Description: 
	
		Free allocated of following datastructures
					1) sCitDataStructure
					2) sBranchInfo
Input:	
			psCitDataStructure		
Output:	
		
Returns:
		Error

******************************************************************************/
static void  BRAP_CITGEN_P_FreeCitDatastructure(											
						BRAP_CIT_P_sCitDataStructure		**psCitDataStructure
					)

{

#ifndef BRAP_CIT_P_NEWSTATIC_MEMORY_ALLOC
	uint32_t						ui32Count;	
	BRAP_CIT_P_BranchInfo			*psBranchInfo;
#endif

	BRAP_CIT_P_sCitDataStructure	*ppsCitDataStructure;


	
	BDBG_ENTER(BRAP_CITGEN_P_FreeCitDatastructure);


	ppsCitDataStructure = *psCitDataStructure;

#ifndef BRAP_CIT_P_NEWSTATIC_MEMORY_ALLOC	

	/*Free memory allocated for sBfranchInfo*/
	for(ui32Count=0;ui32Count< BRAP_P_MAX_FW_BRANCH_PER_FW_TASK ;ui32Count++)
	{
		/*Free up sBranchInfo*/
		psBranchInfo = ppsCitDataStructure->sAlgoNwInfo.psBranchInfo[ui32Count];
		if(psBranchInfo != NULL)
		{		
			BKNI_Free(psBranchInfo);			
		}
		else
		{
			BDBG_ERR(("ERROR: Unable to free memory for sBranchInfo in function BRAP_P_GenCit "));
		}
	}	
#endif

	/*Free memory allocated for sCitDataStructure*/
	if(ppsCitDataStructure != NULL)
	{
		BKNI_Free(ppsCitDataStructure);		
	}
	else
	{
		BDBG_ERR(("ERROR: Unable to free memory for sCitDataStructure in function BRAP_P_GenCit "));
	}

	BDBG_LEAVE(BRAP_CITGEN_P_FreeCitDatastructure);

}

/******************************************************************************
Summary:
		This function copies all the information from the CIT i/p structure to
	the AlgoNwInfo structure.

Description:
		This is needed to keep a master structure in CIT to keep all information
		of a node intact in a single place.

		The BRAP_CIT_P_AlgoNwInfo is structure which has all the details of Branches
		and stages with mode emphasis on individual nodes in stages. This includes
		all the configurations needed for every nodes to execute in DSP.

		Though this process involves redundant copy and memory requirement, 
		it makes stuffs	simple and modular in the due course of CIT module execution.
			
		People who do not know CIT generation module to its fullness keep 
		away from commenting.
Input:
		psCitIp		: CIT input datastructure 
			
Output:
		psAlgoNwInfo: Local CIT generation module structure that holds details 
		of a network.

Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_PrepareAlgoNwStruct( 
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo
					)
{
	uint32_t	ui32branch, ui32Stg;
	BRAP_CIT_P_FwBranchInfo			*psFwBranchInfo;
	BRAP_CIT_P_FwStageInfo			*psFwStageInfo;

	BRAP_CIT_P_BranchInfo	*psBranchInfo;
	BRAP_CIT_P_StageInfo		*psStageInfo;

	BDBG_ENTER(BRAP_CITGEN_P_PrepareAlgoNwStruct);

	psAlgoNwInfo->ui32NumBranches = psCitIp->ui32NumBranches;

	for( ui32branch=0; ui32branch<psAlgoNwInfo->ui32NumBranches; ui32branch++ )
	{
		psFwBranchInfo = psCitIp->pBranchInfo[ui32branch];
		psBranchInfo = psAlgoNwInfo->psBranchInfo[ui32branch];

		psBranchInfo->ui32NumStages = psFwBranchInfo->ui32NumStages;

		for( ui32Stg=0; ui32Stg<psBranchInfo->ui32NumStages; ui32Stg++ )
		{
			psFwStageInfo	= &psFwBranchInfo->sFwStgInfo[ui32Stg];
			psStageInfo		= &psBranchInfo->sStgInfo[ui32Stg];

			/*	Copy all stage info */
			psStageInfo->ui32BranchId	= psFwStageInfo->ui32BranchId;
			psStageInfo->ui32StageId	= psFwStageInfo->ui32StageId;
			psStageInfo->uAlgorithm		= psFwStageInfo->uAlgorithm;
			psStageInfo->eAudioAlgoType = psFwStageInfo->eAudioAlgoType;
			psStageInfo->uAudioMode		= psFwStageInfo->uAudioMode;

			psStageInfo->sStgConnectivity = psFwStageInfo->sStgConnectivity;
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_PrepareAlgoNwStruct);
}
/******************************************************************************
Summary:
		This function detects special tasks in the system

Description:
		
	    Function detects special tasks in the system. Based on the special 
		task configuration, setting needed to be modified from the conventional
		cases.

		Currently Mixer task is detected and considered as the special case.
Input:
		
		psAlgoNwInfo: Local CIT generation module structure that holds details 
		of a network.
		
Output:
		
		psSpecialTaskPresent:

Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_DetectSpecialTasks( 						
						BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
						BRAP_CIT_P_sSpecialTaskPresent	*psSpecialTaskPresent
					)
{	
	BDBG_ENTER(BRAP_CITGEN_P_DetectSpecialTasks);
		
	/* Detecting the MIXER TASK */
	if(psAlgoNwInfo->ui32NumBranches>0)
	{
		if(psAlgoNwInfo->psBranchInfo[0]->ui32NumStages>0)
		{
			/* If Branch 0 and Stage 0 is a Mixer Stage then the task is called as Mixer task */
			if(
				(psAlgoNwInfo->psBranchInfo[0]->sStgInfo[0].eAudioAlgoType == BRAP_CIT_P_AudioAlgoType_ePostProc) &&
				(psAlgoNwInfo->psBranchInfo[0]->sStgInfo[0].uAlgorithm.eProcAudioAlgo == BRAP_CIT_P_ProcessingType_eFWMixerPostProc)
			  )
			{
				psSpecialTaskPresent->eMixerTaskDetectionFlag = BRAP_CIT_P_MixerTaskDetection_eDetected;
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_DetectSpecialTasks);

}
/******************************************************************************
Summary:		
		From the algo network, decide the execution order of stages.
		The order of execution will be indicated by linearly traversing the 
		sAlgoExecOrder structure. This structure will contain the branchId 
		and StageId of the Algo, in the order of execution.

Description:
		
		Determining the order execution of stages should be done for the internal
		operations of CIT generation module.

		The idea is to traverse the stages in one branch till it hits a stage whose
		any of the inputs come from stages that is not evaluated.
		
		The search then goes to the next branch.
		After doing multiple parsing, all the stages will be evaluated the execution
		order will be determined.

Input:
		psAlgoNwInfo		: 
			
Output:
		sAlgoExecOrder	: Execution order

Returns:
		None

******************************************************************************/


static  uint32_t  BRAP_CITGEN_P_DetermineAlgoExecOrder(	
					BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
					BRAP_CIT_P_sAlgoExecOrder	sAlgoExecOrder[BRAP_P_MAX_ALGOS_IN_TASK]
			)
{

	uint32_t	ui32Error;
	uint32_t	ui32IpNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32AlgoNum;
	uint32_t	ui32PassNum;
	uint32_t	ui32BranchNum;		
	uint32_t	ui32SrcStageId;
	uint32_t	ui32SrcBranchId;
	uint32_t	ui32NumAlgoStgInBranch;
	
	uint32_t	bAlgoExecStatus[BRAP_P_MAX_FW_BRANCH_PER_FW_TASK][BRAP_P_MAX_FW_STG_PER_FW_BRANCH];

	BRAP_CIT_P_IP_STATUS		iIpAvaialble;

	BRAP_CIT_P_StageInfo		*psStgInfo;


	BDBG_ENTER(BRAP_CITGEN_P_DetermineAlgoExecOrder);

	ui32Error = BERR_SUCCESS;

	/*	Logic for determing execution order
		
		"If the algo has all its inputs already avialble, it is ready for execution."
		We search from the first algo in the first branch to check for the above 
		condition and continue until all the algorithms are covered. 

			The search will be done in passes. The number of passes is equal to	the
		number of mergings we intend to support within a branch. Currently we are doing
		two passes at the end of which we expect to have covered all the algos in the
		network 
	*/

	/*	First Step: Default Init. None of the algos have been executed. */
	for( ui32BranchNum=0; ui32BranchNum < psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg < ui32NumAlgoStgInBranch ; ui32AlgoStg++ )
		{
			bAlgoExecStatus[ui32BranchNum][ui32AlgoStg] = eFALSE;
		}
	}
	
	/*	This is the running index used to track the execution order */

	ui32AlgoNum = 0;	

	/*	Traverse through the CIT I/P structures to scan for algos whose inputs are already available.
		This will be done in two passes to support merging
	*/

	for( ui32PassNum=0; ui32PassNum < BRAP_CIT_P_NUM_ALGO_NW_PASSES; ui32PassNum++ )
	{
		for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
		{
			ui32NumAlgoStgInBranch = psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

			for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
			{
				psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

				if( bAlgoExecStatus[ui32BranchNum][ui32AlgoStg] == eFALSE)
				{				
					iIpAvaialble = BRAP_CIT_P_IP_STATUS_eAvaialble;

					for(ui32IpNum = 0; ui32IpNum < psStgInfo->sStgConnectivity.ui32NumSrc; ui32IpNum++ )
					{
						if( psStgInfo->sStgConnectivity.sSrcDetails[ui32IpNum].eType != 
								BRAP_CIT_P_FwStgSrcDstType_eFwStg )
						{
							iIpAvaialble &= BRAP_CIT_P_IP_STATUS_eAvaialble;
						}
						else
						{
							/*
								I/P is coming from another Fw Stage.Get the source fw stage's branch id and stage ID 
							*/
							ui32SrcBranchId = psStgInfo->sStgConnectivity.sSrcDetails[ui32IpNum].uDetails.sFwStage.ui32BranchId;
							ui32SrcStageId = psStgInfo->sStgConnectivity.sSrcDetails[ui32IpNum].uDetails.sFwStage.ui32StageId;

							/*	Check if the algo with the above branch and stage ID has already executed? */
							if( bAlgoExecStatus[ui32SrcBranchId][ui32SrcStageId] == eTRUE )
								iIpAvaialble &= BRAP_CIT_P_IP_STATUS_eAvaialble;
							else
								iIpAvaialble &= BRAP_CIT_P_IP_STATUS_eNotAvaialble;
						}
					}
					
					/*	After checking for all the inputs of the stage, if all the inputs 
						are avaiable, we can execute this algo stage
					*/
					if( iIpAvaialble == BRAP_CIT_P_IP_STATUS_eAvaialble )
					{
						sAlgoExecOrder[ui32AlgoNum].ui32BranchId = psStgInfo->ui32BranchId;
						sAlgoExecOrder[ui32AlgoNum].ui32StageId = psStgInfo->ui32StageId;

						bAlgoExecStatus[ui32BranchNum][ui32AlgoStg] = eTRUE;

						ui32AlgoNum++;
					}
					else
					{
						/*	Input is not available for an algo in the branch. So cannot
							go ahead in the branch. Need to look at the next branch.
						*/
						bAlgoExecStatus[ui32BranchNum][ui32AlgoStg] = eFALSE;
						break;
					}
				}
			} /* Algo loop */
		} /* Branch loop */
	} /* Num pass loop */


	/*	After all the passes need to check if all algos have been covered. 
		Else need to signal an error
	
		Another way is to check the uiAlgoNum with the number of algos in the
		network 
	*/
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg < ui32NumAlgoStgInBranch ; ui32AlgoStg++ )
		{
			if( bAlgoExecStatus[ui32BranchNum][ui32AlgoStg] == eFALSE )
			{
				/*	Need to flag an error and return from function */
				BDBG_ERR(("ERROR: Unable to execute all the stages in the Algorithm Network."));
				ui32Error = BERR_UNKNOWN;
				break ;
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_DetermineAlgoExecOrder);
	return ui32Error;
}



/******************************************************************************
Summary:		
		This function associates all algorithm specific node level information
		of a stage to Algo Network Info structure .

Description:
	
		This function determines the nodes of the stages based on the 
		a) AudioAlgoType, b) AudioMode and c) Audio Algorithm

		This function also eliminates the unwanted nodes(FS and TSM) in the PP stages.

		Generic PT stage:
			In case of Generic PT case, we need to pass the algorithm ID of the 
			Branch 0 and Stage 0 to the generic PT.. This is for the CDBPT node

		DTS/Ac3 Encoder detection : This is required as we need to handle some special
			case in BRAP_CITGEN_P_FillSamplingFrequencyMapLut() function

		DDP_PassThru detection:
			This is required as we need to handle some special case in FMM HBR 
				configuration		
	

Input:
		psCitDataStructure		: Global CIT structure
			
Output:
		psCitDataStructure		: Global CIT structure

  						
	Updates: 		ui32NumNodes in a stage								

					AlgoId of all nodes in the stage

					AudioAlgorithmType of the stage. needed for the CDBPT FW to workupon.

					bIpsFromFwStage	: Input nature of a stage

Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_GetNodesInAlgo(	
						BRAP_CIT_P_sCitDataStructure	*psCitDataStructure
					)

{
	uint32_t				ui32BranchNum;
	uint32_t				ui32AlgoStg;
	uint32_t				ui32Node, ui32IpNum;
	uint32_t				ui32NumAlgoStgInBranch;
	uint32_t				ui32AudioTypeForBr0Sg0;
	uint32_t				ui32GenericPassthroughStagePresent;

	BRAP_CIT_P_AlgoNwInfo	*psAlgoNwInfo;

	/*	WARNING!! This variable should be bool type. */
	int				bIpsFromFwStage;

	uint32_t				ui32AudioType;
	uint32_t				ui32AudioMode;

	BRAP_AF_P_sALGO_EXEC_INFO	sNodesInAlgo;

	BRAP_CIT_P_StageInfo		*psStgInfo;

	BDBG_ENTER(BRAP_CITGEN_P_GetNodesInAlgo);


	/*Initialization of AudioType of Br=0 and Stage =0*/
	ui32AudioTypeForBr0Sg0	= 0;

	psAlgoNwInfo = &psCitDataStructure->sAlgoNwInfo;

	/*	Need to translate the algorithm network information into
		node network information */
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch ; ui32AlgoStg++ )
		{	
			/*Generic PT present flag initialization */
			ui32GenericPassthroughStagePresent = 0;

			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			
			/*	First lets get the algorithm type running on this stage */
			switch ( psStgInfo->eAudioAlgoType )
			{

			case BRAP_CIT_P_AudioAlgoType_eDecode:

				ui32AudioType = psStgInfo->uAlgorithm.eDecAudioAlgo;
				ui32AudioMode = psStgInfo->uAudioMode.eDecAudioMode;
				sNodesInAlgo  = BRAP_sDecAlgoExecInfo[ui32AudioType][ui32AudioMode];

				/*Finding the DDP PT*/
				if((ui32AudioType == BRAP_DSPCHN_AudioType_eAc3Plus) &&
					(ui32AudioMode == BRAP_DSPCHN_DecodeMode_ePassThru)
				  )
				{					
					psCitDataStructure->sAlgoModePresent.ui32DDP_PassThruPresent 
									= BRAP_CIT_P_PRESENT;
				}				

				/*	Get the Audio type for Branch 0 and Stage 0	*/
				if((ui32BranchNum ==0) && (ui32AlgoStg ==0))
				{
					ui32AudioTypeForBr0Sg0 = ui32AudioType;
				}

#if defined RAP_MULTISTREAM_DECODER_SUPPORT
				if ((ui32AudioType == BRAP_DSPCHN_AudioType_eAacAdts) ||
					(ui32AudioType == BRAP_DSPCHN_AudioType_eAacLoas) ||
					(ui32AudioType == BRAP_DSPCHN_AudioType_eAacSbrAdts) ||
					(ui32AudioType == BRAP_DSPCHN_AudioType_eAacSbrLoas))
				{
					psCitDataStructure->sAlgoModePresent.ui32DolbyPulsePresent 
									= BRAP_CIT_P_PRESENT;
				}
#endif
				break;

			case BRAP_CIT_P_AudioAlgoType_eEncode:

				ui32AudioType = psStgInfo->uAlgorithm.eEncAudioAlgo;
				ui32AudioMode = psStgInfo->uAudioMode.eEncAudioMode;
				sNodesInAlgo  = BRAP_sEncAlgoExecInfo[ui32AudioType][ui32AudioMode];

				/* Detecting DTS encoder in the input network */
				if(ui32AudioType == BRAP_CIT_P_EncAudioType_eDTSBroadcast)
				{
					psCitDataStructure->sAlgoModePresent.ui32DTS_EncoderPresent 
									= BRAP_CIT_P_PRESENT;			
				}

				/* Detecting Ac3 encoder in the input network */
				if(ui32AudioType == BRAP_CIT_P_EncAudioType_eAc3)
				{
					psCitDataStructure->sAlgoModePresent.ui32AC3_EncoderPresent 
									= BRAP_CIT_P_PRESENT;			
				}
				
				break;

			case BRAP_CIT_P_AudioAlgoType_ePostProc:

				ui32AudioType = psStgInfo->uAlgorithm.eProcAudioAlgo;
				ui32AudioMode = psStgInfo->uAudioMode.eProcessingAudioMode;
				sNodesInAlgo  = BRAP_sPpAlgoExecInfo[ui32AudioType][ui32AudioMode];

				/*	Detect Generic Passthrough stage */
				if(BRAP_CIT_P_ProcessingType_eGenericPassThru == ui32AudioType)
				{
					ui32GenericPassthroughStagePresent = BRAP_CIT_P_PRESENT;
				}

				/*	Detect BTSC Encoder stage */
				if(BRAP_CIT_P_ProcessingType_eBtsc == ui32AudioType)
				{
					psCitDataStructure->sAlgoModePresent.ui32BTSC_EncoderPresent 
									= BRAP_CIT_P_PRESENT;
				}
			
#if defined RAP_MULTISTREAM_DECODER_SUPPORT
				if(ui32AudioType == BRAP_CIT_P_ProcessingType_eDdre)
				{
					psCitDataStructure->sAlgoModePresent.ui32DDREPresent
									= BRAP_CIT_P_PRESENT;

				}
#endif
				break;
				
			case BRAP_CIT_P_GfxAlgoType_eDecode:

				ui32AudioType = psStgInfo->uAlgorithm.eGfxAlgo;
				sNodesInAlgo  = BRAP_sGfxDecodeExecInfo[ui32AudioType];

				break;

			case BRAP_CIT_P_ScmAlgoType_eDecode:

				ui32AudioType = psStgInfo->uAlgorithm.eScmAlgo;
				sNodesInAlgo  = BRAP_sScmDecodeExecInfo[ui32AudioType];

				break;

				
			case BRAP_CIT_P_VideoAlgoType_eDecode:

				ui32AudioType = psStgInfo->uAlgorithm.eVideoAlgo;
				ui32AudioMode = psStgInfo->uAudioMode.eDecAudioMode;
				sNodesInAlgo  = BRAP_sDecAlgoExecInfo[ui32AudioType][ui32AudioMode];

				break;

			default:
				/*	Error Case. It cannot come here */				
				BDBG_ERR(("ERROR Unknown AudioAlgoType %d in Branch %d Stage %d",psStgInfo->eAudioAlgoType,ui32BranchNum,ui32AlgoStg));
				BDBG_LEAVE(BRAP_CITGEN_P_GetNodesInAlgo);
				return BERR_NOT_SUPPORTED;
				break;
			}

			/*	Remove redundant information from the node network. The following 
				principles need to be applied for removing redundant nodes
				1.	All algorithms which have their input buffer configuration
					NOT coming from a node of any other branch will have special
					nodes.
					Even in case wherein the decoder is having a pre-process stage
					the special nodes will be associated with the pre-process and
					not the decoder.
				Drawbacks:
				1. Based on the implementation of Passthrough, need to decide
					if special nodes are required or not.
				
				WARNING!!! Not sure if such a usage mode exists where one i/p is from
				RAVE/FMM and other is from Fw stage. Even in this case, special nodes 
				will be present
			*/
			bIpsFromFwStage = eFALSE;
			for (ui32IpNum = 0; ui32IpNum < psStgInfo->sStgConnectivity.ui32NumSrc; ui32IpNum ++)
			{
				if (psStgInfo->sStgConnectivity.sSrcDetails[ui32IpNum].eType == 
						BRAP_CIT_P_FwStgSrcDstType_eFwStg)
				{
					bIpsFromFwStage = eTRUE;
					break;
				}
			}

			if( bIpsFromFwStage == eTRUE )
			{
				sNodesInAlgo.NumNodes -= BRAP_CIT_P_NUM_SECIAL_NODES;

				/*	Move all the other nodes up */
				for (ui32Node=0; ui32Node<sNodesInAlgo.NumNodes; ui32Node++)
				{
					sNodesInAlgo.eAlgoIds[ui32Node] = 
						sNodesInAlgo.eAlgoIds[ui32Node + BRAP_CIT_P_NUM_SECIAL_NODES];
				}
			}

			/*	Update into Node Attributes structure of Algo Nw Info */ 
			psStgInfo->bIpsFromFwStage = bIpsFromFwStage;
			psStgInfo->ui32NumNodes = sNodesInAlgo.NumNodes;
			for ( ui32Node=0; ui32Node<sNodesInAlgo.NumNodes; ui32Node++)
			{
				psStgInfo->sNodeAttr[ui32Node].eAlgoId = sNodesInAlgo.eAlgoIds[ui32Node];

				/* Adding the Algorithm in this node */
				/*This is needed for CDBPT to workup on. */		

				if(BRAP_CIT_P_PRESENT == ui32GenericPassthroughStagePresent)
				{
					/* Audio Type Branch 0 and Stage 0 is been populated in case of Generic PT case */
					psStgInfo->sNodeAttr[ui32Node].ui32AudioAlgorithmType = ui32AudioTypeForBr0Sg0;
				}
				else
				{
					psStgInfo->sNodeAttr[ui32Node].ui32AudioAlgorithmType = ui32AudioType;
				}			
			}			
		}
	}
	BDBG_LEAVE(BRAP_CITGEN_P_GetNodesInAlgo);
	return BERR_SUCCESS;
}


/******************************************************************************
Summary:		
		This function updates all the MIT parameters of a node into the 
		CIT structure.

Description:
	
		This function extracts the following from MIT and updates them to 
		CIT local structure.
				a) LUT addr and size 
				b) Code addr and size

		This function also converts the offset addr to physical addr

Input:
		hHeap	: Heap Addr
		psMit	: MIT base addr
			
Output:
		psAlgoNwInfo: Local Node info CIT structure
						
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
						BMEM_Handle					hHeap,
						BRAP_Mit					*psMit,
						BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo
				)
{
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32Node;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;
	uint32_t	ui32PhysAddr;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_CIT_P_sNodeAttr		*psNodeAttr;

	BDBG_ENTER(BRAP_CITGEN_P_UpdtMitInfoToAlgoNw);



	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
			for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

				/*	The algo ids in MIT are linear to what
					is defined in the BRAP_AF_P_AlgoId */
				/*	Also convert the addresses into physical addressess for the DSP */
				BRAP_P_ConvertAddressToOffset(	
					hHeap, 
					(void *)(psMit->sAlgoDwnldDetail[psNodeAttr->eAlgoId].ui32AlgoCodePtr),
					&ui32PhysAddr);

				psNodeAttr->ui32CodeDramAddress = ui32PhysAddr;
				psNodeAttr->ui32CodeSize = 
					psMit->sAlgoDwnldDetail[psNodeAttr->eAlgoId].ui32AlgoCodeSize;
                
                /*Additional checkrequired before calling function BRAP_P_ConvertAddressToOffset, 
                to overcome bad addressing errrors "invalid address, cannot find heap address"*/
                if(psMit->sAlgoDwnldDetail[psNodeAttr->eAlgoId].ui32AlgoDataPtr != 0)
                {
    				BRAP_P_ConvertAddressToOffset(	
    					hHeap, 
    					(void *)(psMit->sAlgoDwnldDetail[psNodeAttr->eAlgoId].ui32AlgoDataPtr),
    					&ui32PhysAddr);
                }
				psNodeAttr->ui32RomTableDramAddress = ui32PhysAddr;
				psNodeAttr->ui32RomTableSize = 
					psMit->sAlgoDwnldDetail[psNodeAttr->eAlgoId].ui32AlgoDataSize;

			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_UpdtMitInfoToAlgoNw);
	return BERR_SUCCESS;
}


/******************************************************************************
Summary:		
		This function updates NodeInfo parmaters for brap_af_priv.c of a node 
		to the CIT structure.

Description:
	
		This function extracts the following from NodeInfo and updates them to 
		CIT local structure.
				1)InterFrameBuffType 
				2)InterFrmBuffSize
				3)InterStgGenericBuffSize
				4)InterStgIoBuffSize
				5)MaxNumChansSupported
				6)MaxSizePerChan
				7)ScratchBuffSize		
				8)UsrCfgBuffSize		
				9)FwStatusBuffSize	
				10)FwStatusBuffType	
				
		Allignment:
			FwStatusBuffSize is internally 32bit alligned.
			InterFrmBuffSize and UsrCfgBuffSize should be kept already alligned.


Input:
		BRAP_sNodeInfo	: Constant array defined in brap_af_priv.c file		
			
Output:
		psAlgoNwInfo: Local Node info CIT structure
		
Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
				const BRAP_AF_P_sNODE_INFO	BRAP_sNodeInfo[BRAP_AF_P_AlgoId_eMax],
				BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo 
			)
{

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32Node;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_CIT_P_sNodeAttr		*psNodeAttr;
	const BRAP_AF_P_sNODE_INFO	*psNodeInfo;

	BDBG_ENTER(BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw);

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg < ui32NumAlgoStgInBranch ; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
			for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				/*	The code and tables for algos are copied from MIT */
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psNodeInfo = &BRAP_sNodeInfo[psNodeAttr->eAlgoId];

				psNodeAttr->eInterFrameBuffType		= psNodeInfo->eInterFrameBuffType;				
				psNodeAttr->ui32InterFrmBuffSize	= psNodeInfo->ui32InterFrmBuffSize;

				psNodeAttr->ui32InterStgGenericBuffSize	
													= psNodeInfo->ui32InterStgGenericBuffSize;
				psNodeAttr->ui32InterStgIoBuffSize	= psNodeInfo->ui32InterStgIoBuffSize;

				psNodeAttr->ui32MaxNumChansSupported= psNodeInfo->ui32MaxNumChansSupported;
				psNodeAttr->ui32MaxSizePerChan		= psNodeInfo->ui32MaxSizePerChan;

				psNodeAttr->ui32ScratchBuffSize		= psNodeInfo->ui32ScratchBuffSize;
				psNodeAttr->ui32UsrCfgBuffSize		= psNodeInfo->ui32UserCfgBuffSize;

				/* Doing the 32bit allignment*/
				psNodeAttr->ui32FwStatusBuffSize	= (((psNodeInfo->ui32StatusBuffSize+3)>>2)<<2);
				psNodeAttr->eFwStatusBuffType		= psNodeInfo->eStatusBuffType;

				psNodeAttr->ui32BranchId			= ui32BranchNum;				
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw);
}

/******************************************************************************
Summary:		
	This function computes the sizes required for the user config and
	inter-frame buffers	required for the entire task.It returns the sizes of the
	buffers required for the entire task

Description:
		
	It is based on an assumption that interframe and the user cfg buffer
	may be required for all the Algorithm nodes. So the memory requirement is 
	calculuated by adding the buffer size mentioned in brap_af_priv.c for
	every individual nodes.

	The interframe buffer of a node can be shared with other preceeding nodes.
	in this case, such nodes should keep the interframe buffer requirement to zero.

	It is also mandated that the nodes that do not require IF buffer should
	keep the buffer size to zero.
	We are not checking the Interframe present/absent enum.


	Ideally, every node in a stage can have User Config buffer . Because of other
	FW specifics, we expect only Frame Sync and the first algorithm node can request
	user cfg buffer.

	Interframe and User-Cfg memory size specified in brap_af_priv.c should be 
	32bit alligned.	

	I cannot explain stuffs more than this..If anybody who did not understand this 
	can go to next function. 

Input:

	psAlgoNwInfo: Local Node info CIT structure
			
Output:

	psTaskBuffInfo		
		
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
					BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
					BRAP_CIT_P_sTaskBuffInfo		*psTaskBuffInfo
				)
{
	uint32_t ui32TaskUserCfgMemSize		= 0;
	uint32_t ui32TaskInterFrmMemSize	= 0;
	uint32_t ui32NodeUserCfgBufSize;	
	uint32_t ui32NodeInterFrmBufSize;
	uint32_t ui32Node;

	BRAP_AF_P_InterFrameBuffType	eInterFrameBuffType;

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_CIT_P_sNodeAttr		*psNodeAttr;
	

	BDBG_ENTER(BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes);

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

				ui32NodeUserCfgBufSize	= psNodeAttr->ui32UsrCfgBuffSize;
				ui32NodeInterFrmBufSize = psNodeAttr->ui32InterFrmBuffSize;				
				eInterFrameBuffType		= psNodeAttr->eInterFrameBuffType;

				/*	See if the stage is sharing inter-frame buffer with previous node. 
					If so, the IF size of the current node should be 0.
					If not this should be flagged as an error in the FW struct 
					generation */
				/*	WARNING!!! Do we need more error checks to ensure the sizes are 32 bit 
					aligned?*/
				if( eInterFrameBuffType == BRAP_AF_P_InterFrameBuffType_eShared &&
					ui32NodeInterFrmBufSize != 0 )
				{
					BDBG_ERR(("ERROR Interframe buffer in Branch %d Stage %d Node %d has been defined as SHARED but the size specified is NON-ZERO",ui32BranchNum,ui32AlgoStg,ui32Node));
					BDBG_LEAVE(BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes);
					return BERR_INVALID_PARAMETER ;
				}				

				/*	Inter-frame buffer sizes & User config buffer sizes MUST be 
					alligned to 32 bit. Check here */				
				if( ( ( ( ui32NodeUserCfgBufSize >> 2 ) << 2 )  != ui32NodeUserCfgBufSize ) ||
					  ( ( ui32NodeInterFrmBufSize >> 2 ) << 2 ) != ui32NodeInterFrmBufSize )
				{
					BDBG_ERR(("ERROR NodeUserCfgBufSize = %d or NodeInterFrmBufSize = %d is not uint32_t aligned in Branch %d Stage %d Node %d ",ui32NodeUserCfgBufSize,ui32NodeInterFrmBufSize,ui32BranchNum,ui32AlgoStg,ui32Node));
					BDBG_LEAVE(BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes);
					return BERR_INVALID_PARAMETER ;
				}			
				
				ui32TaskUserCfgMemSize	+= ui32NodeUserCfgBufSize;
				ui32TaskInterFrmMemSize += ui32NodeInterFrmBufSize;
			}
		}
	}

	psTaskBuffInfo->ui32TaskUsrCfgMemSize		= ui32TaskUserCfgMemSize;
	psTaskBuffInfo->ui32TaskInterFrmMemSize		= ui32TaskInterFrmMemSize;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes);

	return BERR_SUCCESS;
}


/*	This function computes the worst case size required for I/O buffer 
	configuration structures in DRAM */
static void BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes( 
				BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
				BRAP_CIT_P_sTaskBuffInfo		*psTaskBuffInfo )
{
	uint32_t	ui32NumAlgosInNw = 0;
	uint32_t	ui32BranchNum;
	uint32_t	ui32MaxNumForksInNode;

	BDBG_ENTER(BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes);

	/*	Find the number of algos within the network. */
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}

	/*This is a wrong calculation as it takes BRAP_AF_P_MAX_OP_FORKS as the max. [Arun's initial CIT code]

	Actual calculation should be (BRAP_AF_P_MAX_OP_FORKS  +  BRAP_AF_P_MAX_IP_FORKS) and
	remove Multiplication by 2 in BRAP_CIT_P_SIZEOF_IOBUFFCFG.

	Or Get the max of BRAP_AF_P_MAX_OP_FORKS  and  BRAP_AF_P_MAX_IP_FORKS and keep 2 in BRAP_CIT_P_SIZEOF_IOBUFFCFG.
	Currently I am doing this... 

	*/
	ui32MaxNumForksInNode = BRAP_CIT_P_MAX_VALUE(BRAP_AF_P_MAX_OP_FORKS,BRAP_AF_P_MAX_IP_FORKS);


	psTaskBuffInfo->ui32TaskIoBuffCfgStructMemSize = 
			ui32NumAlgosInNw * BRAP_CIT_P_SIZEOF_IOBUFFCFG * ui32MaxNumForksInNode;


	BDBG_LEAVE(BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes);
}

/******************************************************************************
Summary:		
		This function allocates the user config and inter-frame buffers
		for each node 

Description:
	
		The interframe buffer and user cfg for all the nodes are allocated.

		For user cfg, if the buffer size specified is zero then this explicitly 
		signals that the buffer is not present.

  		For Interframe, if the buffer size specified is zero then this explicitly 
		signals that the buffer is either not present or shared.

		For a node, if the IF buffer is shared, then it hsould not be the first node 
		and the buffer size should be zero.

Input:
		ui32IfBuffBaseAddr			: Base addr of Interframe Buffer
		ui32UsrCfgBuffBaseAddr		: Base addr of User Cfg Buffer
			
Output:
		psAlgoNwInfo: Local Node info CIT structure
						
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
					uint32_t					ui32IfBuffBaseAddr,
					uint32_t					ui32UsrCfgBuffBaseAddr,
					BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo
				)
{
	uint32_t	ui32Node;
	uint32_t	ui32UserCfgBuffAddr;
	uint32_t	ui32IfBuffAddr;

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;

	BRAP_AF_P_InterFrameBuffType	eInterFrameBuffType;
	BRAP_CIT_P_StageInfo			*psStgInfo;
	BRAP_CIT_P_sNodeAttr			*psNodeAttr, *psPrevNodeAttr;

	BDBG_ENTER(BRAP_CITGEN_P_AllocInterFrmUserCfgBuff);

	ui32UserCfgBuffAddr = ui32UsrCfgBuffBaseAddr;
	ui32IfBuffAddr = ui32IfBuffBaseAddr;

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

			/*	First allocate User Config buffers */
			/*-------------------------------------*/
			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

				if( psNodeAttr->ui32UsrCfgBuffSize == 0 )
				{
					psNodeAttr->ui32UsrCfgBuffDramAddress = BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					psNodeAttr->ui32UsrCfgBuffDramAddress = ui32UserCfgBuffAddr;					
					ui32UserCfgBuffAddr	+= psNodeAttr->ui32UsrCfgBuffSize;
				}
			}

			/*	Then allocate Inter-frame buffers */
			/*------------------------------------*/
			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr			= &psStgInfo->sNodeAttr[ui32Node];
				eInterFrameBuffType = psNodeAttr->eInterFrameBuffType;

				if( psNodeAttr->ui32InterFrmBuffSize == 0 )
				{
					if( eInterFrameBuffType != BRAP_AF_P_InterFrameBuffType_eShared )
					{	
						psNodeAttr->ui32InterFrmBuffDramAddress = 
											BRAP_AF_P_DRAM_ADDR_INVALID;
					}
					else if( ui32Node != 0 )
					{
						psPrevNodeAttr = &psStgInfo->sNodeAttr[ui32Node-1];
						psNodeAttr->ui32InterFrmBuffDramAddress = 
							psPrevNodeAttr->ui32InterFrmBuffDramAddress;
					}
					else
					{	/*	Error Case: First node is having shared flag set */
						BDBG_ERR(("ERROR : Interframe Allcation. First node is having shared flag set"));
						BDBG_LEAVE(BRAP_CITGEN_P_AllocInterFrmUserCfgBuff);
						return BERR_INVALID_PARAMETER;
					}
				}
				else
				{
					psNodeAttr->ui32InterFrmBuffDramAddress = ui32IfBuffBaseAddr;
					ui32IfBuffBaseAddr += psNodeAttr->ui32InterFrmBuffSize;
				}
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_AllocInterFrmUserCfgBuff);
	return BERR_SUCCESS;
}


/******************************************************************************
Summary:		
		This function computes the size of the inter-stage I/O and inter-stage
		generic buffer required by the node network. Based on the number of 
		branches in the network, this function also computes the number of 
		such buffers required by the network

Description:
	
		This function finds the maximum number of channels and and the maximum 
		memory required for per channel and Iogeneric buffer.

		We allocate a Iobuffer and IoGenbuffer based on this max number.

		The total Interstage memory required depends on the max IS buffer required
		in the present use case.

		This memory estimation is based on worst case scenario. We can optimize 
		the same if there is a memory crunch.

Input:
		psAlgoNwInfo: Local Node info CIT structure		
			
Output:
		psTaskBuffInfo:
						
Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_ComputeInterStgBuffSize(
					BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
					BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo 
				)
{
	uint32_t ui32Error;
	uint32_t ui32BranchInterStgIoMemSize		= 0;
	uint32_t ui32BranchInterStgGenericMemSize	= 0;
	uint32_t ui32MaxSizePerChannel;
	uint32_t ui32MaxNumChannelsSupported;
	uint32_t ui32NodeInterStgIoBuffSize;
	uint32_t ui32NodeInterStgGenericBuffSize;

	uint32_t ui32TaskMaxSizePerChannel			= 0;
	uint32_t ui32TaskMaxNumChannelsSupported	= 0;

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;
	uint32_t	ui32Node;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;

	BDBG_ENTER(BRAP_CITGEN_P_ComputeInterStgBuffSize);

	ui32Error = BERR_SUCCESS;

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo			= &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInAlgo	= psStgInfo->ui32NumNodes;

			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

				/*	Checking the size required for inter-stage IO buffer */
				ui32MaxSizePerChannel		= psNodeAttr->ui32MaxSizePerChan;
				ui32MaxNumChannelsSupported = psNodeAttr->ui32MaxNumChansSupported;
				ui32NodeInterStgIoBuffSize	= psNodeAttr->ui32InterStgIoBuffSize;

				/*	Check for Error in Filling the structure */
				if( ui32NodeInterStgIoBuffSize < ( ui32MaxSizePerChannel * ui32MaxNumChannelsSupported ) )
				{
					BDBG_ERR(("ERROR: Interstage Buffer requirement for Node %d of Stage %d of Branch %d is not correct",ui32Node,ui32AlgoStg,ui32BranchNum));
					BDBG_ERR(("\t The NodeInterStgIoBuffSize =  %d. The MaxSizePerChannel = %d and MaxNumChannelsSupported = %d",ui32NodeInterStgIoBuffSize,ui32MaxSizePerChannel,ui32MaxNumChannelsSupported));
					BDBG_ERR((""));

					BDBG_LEAVE(BRAP_CITGEN_P_ComputeInterStgBuffSize);
					return BERR_INVALID_PARAMETER;
				}

				ui32TaskMaxSizePerChannel		= BRAP_CITGEN_P_Max(	
																ui32MaxSizePerChannel,
																ui32TaskMaxSizePerChannel
															);

				ui32TaskMaxNumChannelsSupported = BRAP_CITGEN_P_Max(	
																ui32MaxNumChannelsSupported,
																ui32TaskMaxNumChannelsSupported
															);

				ui32BranchInterStgIoMemSize		= ui32TaskMaxSizePerChannel * ui32TaskMaxNumChannelsSupported;
				
				
				/*	Now Checking the size required for inter-stage generic buffer */
				ui32NodeInterStgGenericBuffSize = psNodeAttr->ui32InterStgGenericBuffSize;

				ui32BranchInterStgGenericMemSize
												= BRAP_CITGEN_P_Max( 
																ui32NodeInterStgGenericBuffSize,
																ui32BranchInterStgGenericMemSize 
															);
			}
		}
	}

	/*	Check for buffer size alignment */
	ui32BranchInterStgIoMemSize			= ( ( ( ui32BranchInterStgIoMemSize + 3 ) >> 2 ) << 2 );
	ui32BranchInterStgGenericMemSize	= ( ( ( ui32BranchInterStgGenericMemSize + 3 ) >> 2 ) << 2 );


	psTaskBuffInfo->ui32BranchInterStgIoMemSize			= ui32BranchInterStgIoMemSize;
	psTaskBuffInfo->ui32BranchInterStgGenericMemSize	= ui32BranchInterStgGenericMemSize;

#if OLD_BRANCH_PLUS_ONE_CONCEPT
	if( psAlgoNwInfo->ui32NumBranches != 1 )
	{
		psTaskBuffInfo->ui32NumInterStgBuffs = psAlgoNwInfo->ui32NumBranches + 1;

		psTaskBuffInfo->ui32TaskInterStgIoMemSize = 
						ui32BranchInterStgIoMemSize * psTaskBuffInfo->ui32NumInterStgBuffs;

		psTaskBuffInfo->ui32TaskInterStgGenericMemSize = 
						ui32BranchInterStgGenericMemSize * psTaskBuffInfo->ui32NumInterStgBuffs;
	}
	else
	{
		psTaskBuffInfo->ui32NumInterStgBuffs = 1;

		psTaskBuffInfo->ui32TaskInterStgIoMemSize		= ui32BranchInterStgIoMemSize;

		psTaskBuffInfo->ui32TaskInterStgGenericMemSize	= ui32BranchInterStgGenericMemSize;
	}
#else
		psTaskBuffInfo->ui32NumInterStgBuffs = psAlgoNwInfo->ui32NumBranches;

		psTaskBuffInfo->ui32TaskInterStgIoMemSize = 
						ui32BranchInterStgIoMemSize * psTaskBuffInfo->ui32NumInterStgBuffs;

		psTaskBuffInfo->ui32TaskInterStgGenericMemSize = 
						ui32BranchInterStgGenericMemSize * psTaskBuffInfo->ui32NumInterStgBuffs;
	
#endif

	psTaskBuffInfo->ui32MaxSizePerChannel		= ui32TaskMaxSizePerChannel;
	psTaskBuffInfo->ui32MaxNumChannelsSupported	= ui32TaskMaxNumChannelsSupported;
	

	/* Error Check for interstage buffers*/
	#ifdef BRAP_CIT_P_CHECK_OUTPUT_RANGE
	if(psTaskBuffInfo->ui32NumInterStgBuffs > BRAP_P_MAX_IS_BUFFER_PER_FW_TASK)
	{
		BDBG_ERR(("The number of interstage buffers [%d] required is more than allowed [%d]",psTaskBuffInfo->ui32NumInterStgBuffs,BRAP_P_MAX_IS_BUFFER_PER_FW_TASK));
		ui32Error = BRAP_CIT_P_ERROR;
	}
	#endif


	BDBG_LEAVE(BRAP_CITGEN_P_ComputeInterStgBuffSize);

	return ui32Error;
}


/******************************************************************************
Summary:		
		This function computes the size of the scratch buffer required for 
		enitre node network configuration. Since the scratch buffer will be
		resued by all the nodes within the algorithm network, the size
		of the scratch buffer is the worst case size required by any of the
		node in the given configuration

Description:	

		CIT Generation module traverse through all the nodes to find the worst 
		case buffer requirement for scratch buffer. Based on it, CIT 
		allocates memory and associate the same mnemory to all nodes.

Input:
		psAlgoNwInfo: Local Node info CIT structure		
			
Output:
		psTaskBuffInfo:
						
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_ComputeScratchBuffSize(
						BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
						BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo
					)
{
	uint32_t ui32TaskScratchMemSize = 0;
	uint32_t ui32NodeScratchBuffSize;
	uint32_t ui32Node;

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;


	BDBG_ENTER(BRAP_CITGEN_P_ComputeScratchBuffSize);

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				
				ui32NodeScratchBuffSize = psNodeAttr->ui32ScratchBuffSize;

				/*	Checking for max scratch buff size required by the configuration */
				ui32TaskScratchMemSize = BRAP_CITGEN_P_Max(	ui32NodeScratchBuffSize, 
															ui32TaskScratchMemSize
														);
			}
		}
	}

	/*	Check if the Scratch Size is 32 word aligned. If not return a 32 bit 
		aligned size */
	ui32TaskScratchMemSize = ( ( ( ui32TaskScratchMemSize + 3 ) >> 2 ) << 2 );
	
	psTaskBuffInfo->ui32TaskScratchMemSize = ui32TaskScratchMemSize;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeScratchBuffSize);

}


/******************************************************************************
Summary:

	This is the main function that allocates I/O buffer configuration.
	This function first prepares the I/O buffer configurations structures
	for inter-stage buffers. It copies the I/O port configuration
	from the sAlgoNwInfo to DRAM and updates the address into 
	NodeAttr structure. 
	Finally it allocates which I/O buffer configuration to use for
	inter-stage buffers based on the branching information.
	This also fills the o/p port configuration structure to be used
	in the global task configuration 

Description:

	1) Initialize internal data structures.
	2) Allocate inter-stage buffer
	3) Prepare handles for I/O buff Io Gen Cfg structs for inter-stage buffers.
	4) Associate interstage buffers to nodes
	5) Analyse the Input Configuration for all nodes and prepare the IO Handle
			a) Handle MS11 SIPS API special case

	6) Analyse the Ouput Configuration for all nodes and prepare the IO Handle
	7) FMM port analysis

			a) Fill the ports drive rate.
			b) Prepare gate open configurations
			c) Prepare PPM configuration
			d) Master Slave configuration

	8) Configure MAX independent delay for ports
Input:
					
Output:
		
						
Returns:
		Error

******************************************************************************/

static uint32_t BRAP_CITGEN_P_AllocIoBuffCfg(	
					BMEM_Handle								hHeap,
					uint32_t								ui32InterStgBuffBaseAddr,
					uint32_t								ui32IoBuffCfgStructBaseAddr,
					BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
					BRAP_CIT_P_sTaskBuffInfo				*psTaskBuffInfo,
					BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
					BRAP_CIT_P_sOpPortInfo					*psOpPortInfo,
					BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG	*psTaskFmmGateOpenConfig,
					uint32_t								ui32TaskRdbOffset,
					BRAP_CIT_P_sCitDataStructure			*psCitDataStructure,
					BRAP_AF_P_EnableDisable					eIndepDelayEnableFlag,
					BRAP_CIT_P_SameInputForFsAndDecode		eSameInputForFsAndDecode,
					BRAP_CIT_P_sSpecialTaskPresent			*psSpecialTaskPresent
				)
{


	uint32_t	ui32Err;
	uint32_t	ui32BranchNum,ui32Count;
	uint32_t	ui32Algo, ui32Node, ui32Src, ui32Dst, ui32DistOp, ui32OpNum,ui32FmmOpNum;
	uint32_t	ui32PPMCount;
	uint32_t	ui32NumAlgosInNw;
	uint32_t	ui32BranchId, ui32StageId;
	uint32_t	ui32SrcBranchId, ui32SrcStageId, ui32SrcOpId, ui32SrcDistictOpNum;
	uint32_t	ui32NumNodesInAlgo;
	uint32_t	ui32IsBuffId;
	uint32_t	ui32SrcCount;
	uint32_t	ui32NodesNumSrc;
	
	uint32_t	ui32IoPortCfgStructAddr;
	/*uint32_t	ui32NextAlgoBranchId; */
	uint32_t	ui32OpPortNum;
	uint32_t	ui32FwConPorts;
	uint32_t	ui32FwConBranchId;
	uint32_t	ui32InterstageBufferId;
	uint32_t	ui32FmmPortCountInDstOutput;

	BRAP_AF_P_FmmDstFsRate			eFmmDstFsRate;
	BRAP_AF_P_DistinctOpType		eDistinctOpType;
	BRAP_CIT_P_MasterSlaveType		eMasterSlaveType;	

		
	BRAP_AF_P_sIO_BUFFER			*psIoBuffTemp;
	BRAP_AF_P_sIO_GENERIC_BUFFER	*psIoGenericBuffTemp;

	BRAP_CIT_P_FwStgSrcDstDetails	*psFwStgSrcDstDetails;

	BRAP_CIT_P_StageInfo			*psStgInfo, *psSrcStgInfo;
	BRAP_CIT_P_sNodeAttr			*psNodeAttr, *psSrcNodeAttr;	

	BRAP_CIT_P_TaskIsBuffCfg		*psTaskIsBuffCfg;


	BDBG_ENTER(BRAP_CITGEN_P_AllocIoBuffCfg);
	
	ui32Err = BERR_SUCCESS;


	/* Structure pointer Initializations*/
	psIoBuffTemp		= &psCitDataStructure->sAllocIoBuffCfgStructures.sIoBuffTemp;
	psIoGenericBuffTemp = &psCitDataStructure->sAllocIoBuffCfgStructures.sIoGenericBuffTemp;

	
	/* Allocating the Task Interstage Buffer */
	psTaskIsBuffCfg = (BRAP_CIT_P_TaskIsBuffCfg*)BKNI_Malloc(SIZEOF(BRAP_CIT_P_TaskIsBuffCfg));

	if(psTaskIsBuffCfg == NULL)
	{
		BDBG_ERR(("ERROR: Unable to allocate memory for TaskIsBuffCfg in Function BRAP_CITGEN_P_AllocIoBuffCfg "));
		BDBG_LEAVE(BRAP_CITGEN_P_AllocIoBuffCfg);
		return BERR_OUT_OF_SYSTEM_MEMORY;
	}
	else
	{
		/* Intializig the memory allcated with zeroes*/
		BKNI_Memset(psTaskIsBuffCfg ,0,sizeof( BRAP_CIT_P_TaskIsBuffCfg ));   	
	}
	

	/*	First prepare I/O buff Cfg structs for inter-stage buffers */
	BRAP_CITGEN_P_PrepareIsIoBuffCfgStruct( ui32InterStgBuffBaseAddr,
											psTaskBuffInfo,
											psTaskIsBuffCfg->sIsBuffCfg 
										);

	/*	Write the above I/O buff Cfg structures to DRAM */
	BRAP_CITGEN_P_WriteIsIoBuffCfgStructToDram(	
							hHeap,
							psTaskBuffInfo->ui32NumInterStgBuffs,
							ui32IoBuffCfgStructBaseAddr,
							psTaskIsBuffCfg->sIsBuffCfg 
						);

	ui32IoPortCfgStructAddr = ui32IoBuffCfgStructBaseAddr + 
			psTaskBuffInfo->ui32NumInterStgBuffs * (	SIZEOF(BRAP_AF_P_sIO_BUFFER) +
														SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

	/*  Initializing the PPM cfg*/
	BRAP_CITGEN_P_InitializePpmCfg(psCitDataStructure->sFwHwCfg.sPpmCfg);

	/* Initialization of Inter stage  associated Flags*/
	/*------------------------------------------------*/
	for(ui32Count =0; ui32Count < BRAP_P_MAX_IS_BUFFER_PER_FW_TASK; ui32Count++)
	{
		psTaskIsBuffCfg->sIsBuffCfg[ui32Count].ui32PortAssociated	=0;
		psTaskIsBuffCfg->sIsBuffCfg[ui32Count].ui32StageAssociated	=0;

	}
	for( ui32Algo=0; ui32Algo<BRAP_P_MAX_ALGOS_IN_TASK; ui32Algo++)
	{
		for( ui32DistOp=0; ui32DistOp<BRAP_P_MAX_FW_STG_DISTINCT_OUTPUTS; ui32DistOp++ )
		{

			psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32BranchAssociated
							 = (uint32_t)-1;
			psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32InterstageBufferId
							 = (uint32_t)-1;
			psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32IsRequired
							 = (uint32_t)-1;
		}
	}


	/*	Find the number of algos within the network. */
	/*-----------------------------------------------*/
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}

	
	/*-----------------------------*/
	/*	Need to process the Stages */
	/*-----------------------------*/
	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{
		ui32BranchId = sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId = sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

		for( ui32DistOp=0; ui32DistOp<psStgInfo->sStgConnectivity.ui32NumDistinctOp; ui32DistOp++ )
		{
			ui32FwConPorts		= 0;
			ui32FwConBranchId	= 0;

			/* Analysing the output of every stage */
			for( ui32Dst = 0; ui32Dst<psStgInfo->sStgConnectivity.ui32NumDst[ui32DistOp]; ui32Dst++)
			{			
				psFwStgSrcDstDetails 
					= &psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst];				

				if(psFwStgSrcDstDetails->eType == BRAP_CIT_P_FwStgSrcDstType_eFwStg)
				{
					/* Incrementing the Fw Connectivity ports */
					ui32FwConPorts++;	

					/* Find out the Branch ID of the next Stage*/
					ui32FwConBranchId = psFwStgSrcDstDetails->uDetails.sFwStage.ui32BranchId;
				}
			}
	
			/*	If we have more than one Fw Connectivities, we need to associate an IS
				in the DistOp port and definitely there is a branch
			*/
			if(ui32FwConPorts > 1)
			{
				/* Says Is is required and branched out */
				psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32IsRequired		= 1;				
				psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32BranchAssociated = BRAP_CIT_P_ASSOCIATED;
			}

			/* If there is only one port connected, then we need to check the Branch ID of 
			   the next connected stage 
			*/
			else if(ui32FwConPorts == 1)
			{		
				/* Surely IS is required. But is it branched?*/
				psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32IsRequired = 1;

				if(ui32FwConBranchId != ui32BranchId)
				{
					/* Associate  an IS if the next stage is connected from other Branch ID*/
					/* This IS Buffer is branched out*/
					psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32BranchAssociated 
									= BRAP_CIT_P_ASSOCIATED;

				}
				else
				{
					/* This IS Buffer is not branched out... Going to stage in the same Branch*/
					psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32BranchAssociated 
									= BRAP_CIT_P_NOT_ASSOCIATED;

				}
			}
		}                                                 

		/*	Sorting the Interstage buffer allocated for different ports and allocate the IS buffer.	
			Set the Configuration for the IS buffer
		*/	
	
		for(ui32DistOp=0; ui32DistOp<psStgInfo->sStgConnectivity.ui32NumDistinctOp; ui32DistOp++)
		{
			if(psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32IsRequired == 1)
			{
				/* Get free Interstage buffer as ID*/				
				ui32Err	 =	BRAP_CITGEN_P_GenFreeInterstageBuffer(
												psTaskIsBuffCfg,
												psTaskBuffInfo->ui32NumInterStgBuffs,
												&ui32IsBuffId
											);

				if( ui32Err != BERR_SUCCESS)
				{
					BDBG_ERR(("ERROR_1: No Free interstage buffer is available. MaxNum Interstage Buffers = %d",psTaskBuffInfo->ui32NumInterStgBuffs));					
					goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
				}


				/* Mark it as associated with port for same stage */
				psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32StageAssociated =BRAP_CIT_P_ASSOCIATED;
				
				/*IS used as Port*/
				if(psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32BranchAssociated 
								== BRAP_CIT_P_ASSOCIATED
				  )
				{
					/* This Interstage buffer should not be re-used */
					psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32PortAssociated =BRAP_CIT_P_ASSOCIATED;
				}
				
				/* Here is the association happens*/
				psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32InterstageBufferId = ui32IsBuffId;				
			}
		}

		/* Clearing the ui32StageAssociated flag used for IS buffers */
		for(ui32Count=0;ui32Count<BRAP_P_MAX_IS_BUFFER_PER_FW_TASK;ui32Count++)
		{
			psTaskIsBuffCfg->sIsBuffCfg[ui32Count].ui32StageAssociated =BRAP_CIT_P_NOT_ASSOCIATED;
		}
	}



	/*----------------------------------------------------------------------------------------*/	
	/*----------------------------------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------------*/


	ui32IsBuffId	= 0;
	ui32OpPortNum	= 0;
	ui32FmmOpNum	= 0;
	ui32PPMCount	= 0;

	/* Get the next Free interstage Buffer*/	
	ui32Err	 =	BRAP_CITGEN_P_GenFreeInterstageBuffer(
												psTaskIsBuffCfg,
												psTaskBuffInfo->ui32NumInterStgBuffs,
												&ui32IsBuffId
											);

	/*	Error Check. The inter-stage buffer index cannot be greater than
		the number of inter-stage buffers itself 
	*/
	if( ui32Err != BERR_SUCCESS)
	{
		BDBG_ERR(("ERROR_2: No Free interstage buffer is available. MaxNum Interstage Buffers = %d",psTaskBuffInfo->ui32NumInterStgBuffs));		
		goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
	}



	/*	Error Check. The inter-stage buffer index cannot be greater than
		the number of inter-stage buffers itself 
	*/

	#ifdef BRAP_CIT_P_CHECK_INPUT_RANGE
	if( ui32IsBuffId >= psTaskBuffInfo->ui32NumInterStgBuffs )
	{		  
		BDBG_ERR((" ERROR:The inter-stage buffer index cannot be greater than	the number of inter-stage buffers itself"));
		ui32Err = BERR_NOT_SUPPORTED;
		goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;
	}
	#endif

	/*	Need to process the algorithms based on execution order */
	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{

		ui32BranchId	= sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId		= sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo		= &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];

		/* Holding the interstage Buffer index of the stage*/
		psStgInfo->ui32InterStageBuffIdx = ui32IsBuffId;

		
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
		for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
		{
			psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

			/*Initializing the Valid flag for all input ports as Invalid*/
			for(ui32SrcCount=0; ui32SrcCount<BRAP_AF_P_MAX_IP_FORKS;ui32SrcCount++)
			{
				psNodeAttr->eNodeIpValidFlag[ui32SrcCount]= BRAP_AF_P_eInvalid;
			}

			/*	Assign by default all i/o configurations to inter-stage 
				Later assign the first node's input from the algos input and
				the last node's output from the algos o/p
				For intermediate nodes, the number of source and destination 
				is always ONE. So initializing this as the default value
			*/
			
			/*Default: configuring a single input buffer */
			psNodeAttr->ui32NumSrc = 1;
			psNodeAttr->ui32NodeIpBuffCfgAddr[0] = 
							psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32IoBufferStructAddr;
			psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[0] = 
							psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32GenericBufferStructAddr;
			psNodeAttr->eNodeIpValidFlag[0] = 
							BRAP_AF_P_eValid;		


			/*Default: configuring a single output buffer */
			psNodeAttr->ui32NumDst = 1;			
			psNodeAttr->ui32NodeOpBuffCfgAddr[0][0] = 
							psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32IoBufferStructAddr;
			psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[0][0] = 
							psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32GenericBufferStructAddr;

			/*Default value for the intermediate nodes*/
			psNodeAttr->ui32NumDistinctOp		 = 1;
			psNodeAttr->ui32NumPortsPerDistOp[0] = 1;

			/* Default value for datatype of the output buffers of a node*/
			psNodeAttr->eNodeOpBuffDataType[0][0] = BRAP_AF_P_DistinctOpType_eGenericIsData;

			/* Default value for ConnectivityType */
			psNodeAttr->eConnectivityType[0][0]	  =   BRAP_CIT_P_FwStgSrcDstType_eFwStg;


			/*	First Node: Take i/p from Algo's i/p */
			if( ui32Node == 0  )
			{
				psNodeAttr->ui32NumSrc = psStgInfo->sStgConnectivity.ui32NumSrc;

				if(psNodeAttr->ui32NumSrc > BRAP_AF_P_MAX_IP_FORKS)
				{
					BDBG_ERR(("ERROR: The number of I/P Forks = [%d] is greater than the maximum allowed in CIT",psNodeAttr->ui32NumSrc));
					ui32Err = BERR_NOT_SUPPORTED ;
					goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;	

				}

				for( ui32Src=0; ui32Src<psStgInfo->sStgConnectivity.ui32NumSrc; ui32Src++)
				{
					if( psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].eType != BRAP_CIT_P_FwStgSrcDstType_eFwStg )
					{
						/*	Need to translate the buffers IDs into addresses for FMM and RDB 
							type of buffers */
						switch( psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].eType )
						{

						case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:
							
							/*	Translate FMM Buffer Ids to Addresses */
							BRAP_CITGEN_P_TransFmmBuffId2Addr(
								&psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
								psIoBuffTemp,
								&psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
								psIoGenericBuffTemp,
								ui32TaskRdbOffset,
								BRAP_CIT_P_MasterSlaveType_eOthers
								);

							/*	Write the configuration structure into DRAM */
							/*	First I/O buff Cfg struct */
							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) psIoBuffTemp,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );

							psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
							
							/*	Next Generic buff Cfg struct */
							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) psIoGenericBuffTemp,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

							psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);
							
							/* Making the Input port valid */
							psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eValid;		
							
							
							break;	

						case BRAP_CIT_P_FwStgSrcDstType_eRDB:

							
							/*	Translate RDB Buffer Ids to Addresses */
							BRAP_CITGEN_P_TransRdbBuffId2Addr(
								&psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
								psIoBuffTemp,
								&psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
								psIoGenericBuffTemp,
								ui32TaskRdbOffset
								);

							/*	Write the configuration structure into DRAM */
							/*	First I/O buff Cfg struct */
							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) psIoBuffTemp,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );

							psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
							
							/*	Next Generic buff Cfg struct */
							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) psIoGenericBuffTemp,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

							psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);
							

							/* Making the Input port valid */
							psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eValid;		
							
							break;

						case BRAP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf:

							/* This is the inter task DRAM buffer cfg */

							psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src]	
									= psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sInterTaskDramBuffCfg.ui32IoBuffCfgAddr;

							psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src]
									= psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sInterTaskDramBuffCfg.ui32IoGenericBuffCfgAddr;	

							/* Making the Input port valid */
							psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eValid;		

							break;

						case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
							
						/* case BRAP_CIT_P_FwStgSrcDstType_eDram:*/
						default:
							/*	Write the configuration structure into DRAM */
							/*	First I/O buff Cfg struct */

							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) &psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoBuffer.sIoBuffer,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );

							psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
							
							/*	Next Generic buff Cfg struct */
							BRAP_CITGEN_P_WriteToDRAM( 
								hHeap,
								(uint32_t) &psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffer,
								ui32IoPortCfgStructAddr,
								(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

							psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

							ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);

							/* Making the Input port valid */
							psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eValid;		

							break;
						}
					}
					else
					{
						/*	Need to take the address from the Node feeding i/p to current
							node. This will be the last node of the algorithm */						

						ui32SrcBranchId = 
							psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sFwStage.ui32BranchId;
						ui32SrcStageId = 
							psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sFwStage.ui32StageId;
						ui32SrcOpId = 
							psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sFwStage.ui32OpNum;
						ui32SrcDistictOpNum = 
							psStgInfo->sStgConnectivity.sSrcDetails[ui32Src].uDetails.sFwStage.ui32DistinctOpNum;

						psSrcStgInfo = &psAlgoNwInfo->psBranchInfo[ui32SrcBranchId]->sStgInfo[ui32SrcStageId];
						psSrcNodeAttr = &psSrcStgInfo->sNodeAttr[psSrcStgInfo->ui32NumNodes - 1];

						psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src] = 
							psSrcNodeAttr->ui32NodeOpBuffCfgAddr[ui32SrcDistictOpNum][ui32SrcOpId];

						psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src] = 
							psSrcNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32SrcDistictOpNum][ui32SrcOpId];	

						/* Making the Input port valid */
						psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eValid;		 

					}
				}
				
				/*--------------------------------------------------*/
				/* IF MIXER TASK IS PRESENT AND FOR THE MIXER STAGE */
				/*--------------------------------------------------*/

				/* Detecting the mixer task */ 

				if(BRAP_CIT_P_MixerTaskDetection_eDetected == psSpecialTaskPresent->eMixerTaskDetectionFlag)
				{	
					/* Detecting the First Branch and Stage */
					if( (0 == ui32BranchId) && (0 == ui32StageId) )
					{
						ui32NodesNumSrc = psNodeAttr->ui32NumSrc;

						if( ui32NodesNumSrc < BRAP_AF_P_MAX_IP_FORKS)
						{					
							for(; ui32Src<BRAP_AF_P_MAX_IP_FORKS; ui32Src++)
							{								
								/* Configure Dummy IoBuffer and IoGeneric Configuration */
								BRAP_CITGEN_P_DummyIOBufferCfg(
														psIoBuffTemp,
														psIoGenericBuffTemp
													);

								/* IO Buffer */
								psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

								BRAP_CITGEN_P_WriteToDRAM( 
										hHeap,
										(uint32_t) psIoBuffTemp,
										ui32IoPortCfgStructAddr,
										(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );


								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);

								/* IO Generic Buffer  */
								psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Src] = ui32IoPortCfgStructAddr;

								
								/*	Next Generic buff Cfg struct */								
								BRAP_CITGEN_P_WriteToDRAM( 
										hHeap,
										(uint32_t) psIoGenericBuffTemp,
										ui32IoPortCfgStructAddr,
										(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);

								/* Making the Input port valid */
								psNodeAttr->eNodeIpValidFlag[ui32Src] = BRAP_AF_P_eInvalid;	
								
								/* Incrementing the Num Sources */
								psNodeAttr->ui32NumSrc++;

							}
						}
					}
				}


			} 
			/*	Last Node: Take o/p from Algo's o/p */
			if( ui32Node == (ui32NumNodesInAlgo - 1)  )
			{
				psNodeAttr->ui32NumDst = 0;

				ui32OpNum=0;				


				/* Getting the number of Distinct outputs*/
				psNodeAttr->ui32NumDistinctOp = psStgInfo->sStgConnectivity.ui32NumDistinctOp;				

				for( ui32DistOp=0; ui32DistOp<psStgInfo->sStgConnectivity.ui32NumDistinctOp; ui32DistOp++ )
				{
					/*Getting the output ports in a Distinct output port */
					psNodeAttr->ui32NumPortsPerDistOp[ui32DistOp] = 
								psStgInfo->sStgConnectivity.ui32NumDst[ui32DistOp];

					/*Getting the Port datatype */
					eDistinctOpType = psStgInfo->sStgConnectivity.eDistinctOpType[ui32DistOp];

					/* Defaulting the Sink rate : For Future use only */
					eFmmDstFsRate = BRAP_AF_P_FmmDstFsRate_eBaseRate;

					/*Intialization for Master Slave Configuration */
					ui32FmmPortCountInDstOutput = 0;	/*Clearing the FMM Count in Dst output port*/
					eMasterSlaveType			= BRAP_CIT_P_MasterSlaveType_eMasterPort; /*Setting the Port mode as master */

					for( ui32Dst=0; ui32Dst<psStgInfo->sStgConnectivity.ui32NumDst[ui32DistOp]; ui32Dst++)
					{		
						
						psNodeAttr->ui32NumDst++;
						
						/* Populating the individual output port datatypes */
						psNodeAttr->eNodeOpBuffDataType[ui32DistOp][ui32Dst] = eDistinctOpType;

						/*Gettign the connectivity info */
						psNodeAttr->eConnectivityType[ui32DistOp][ui32Dst] = psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].eType;

						if( psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].eType != 
							BRAP_CIT_P_FwStgSrcDstType_eFwStg )
						{
							
							/*	Need to translate the buffers IDs into addresses for FMM and RDB type of buffers */
							switch( psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].eType )
							{
							case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:

								/* Getting the Sink rate of the FMM buffer and use it for the Gate open */
								eFmmDstFsRate = psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.eFmmDstFsRate;

								ui32Err = BRAP_CITGEN_P_VerifyFmmFsRate(eFmmDstFsRate);
					
								if(ui32Err != BERR_SUCCESS)
								{
									BDBG_ERR((" ERROR : Invalid FMM sink rate=[%x] for DistinctOp=%d",eFmmDstFsRate,ui32DistOp));
									goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
								}

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
								ui32FmmPortCountInDstOutput++;	/*Increasing the Number of FMM ports found in a Distinct output*/

								if(ui32FmmPortCountInDstOutput > 1)
								{
									eMasterSlaveType = BRAP_CIT_P_MasterSlaveType_eSlavePort;
								}

								/*	Translate FMM Buffer Ids to Addresses */
								BRAP_CITGEN_P_TransFmmBuffId2Addr(
									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
									psIoBuffTemp,
									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
									psIoGenericBuffTemp,
									ui32TaskRdbOffset,
									eMasterSlaveType
								);

							

								/* Filling the Gate Open*/ /* Independent delay and Gate open is combined */

								/*
								A distinct port can have multiple FMM outputs when the Idependent delay is enabled.
								   In this case : 1) There will be differenct delays associated with each FMM ports and 
												  2) Gate open needed for each FMM output
												  3) The FMM port content for initial zero filling / pause burst filling in FW

									Disadvantage: When the independent delay is enabled, zero filling in DSP will happen for all the slaves also.
												  Not a functional problem.
								
									The FMM Gate open and Independent delay configurations are done in 'BRAP_CITGEN_P_PopulateGateOpenCfg'
											
								*/
								
								BRAP_CITGEN_P_PopulateGateOpenCfg(
									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
									psIoBuffTemp,
									&psTaskFmmGateOpenConfig->sFmmGateOpenConfig[ui32FmmOpNum],
									ui32IoPortCfgStructAddr,
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32IndepDelay,
									eDistinctOpType,
									eFmmDstFsRate
								);
								ui32FmmOpNum++;


	
								/*	Write the configuration structure into DRAM */
								/*	First I/O buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) psIoBuffTemp,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );


								psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
								
								/*	Next Generic buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) psIoGenericBuffTemp,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

								psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);
								
								ui32OpNum++;
								
                                
        						/*PPM Configuration*/
        						ui32Err = BRAP_CITGEN_P_PopulatePpmCfg(
        									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
        									psCitDataStructure->sFwHwCfg.sPpmCfg,
        									ui32TaskRdbOffset,
											&ui32PPMCount
        								);

								if(ui32Err != BERR_SUCCESS)
								{
									BDBG_ERR((" ERROR : Exeeding the MaxLimit of Adaptive Rate control resources"));
									goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
								}
                                 
							
								/*	Fill in the o/p port cfg structure */
								/*
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32OpPortCfgAddr = 
											psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst];

								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelayValue = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32Delay;

								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelaySign = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32IndDelaySign;

								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32FmmAdaptRateCntrlRegAddr = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32AdapRateCntrlRegAddr;
							

								ui32OpPortNum++;

  								*/
								break;

							case BRAP_CIT_P_FwStgSrcDstType_eRDB:
								/*	Translate RDB Buffer Ids to Addresses */
								BRAP_CITGEN_P_TransRdbBuffId2Addr(
									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoBuffer.sIoBuffId,
									psIoBuffTemp,
									&psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
									psIoGenericBuffTemp,
									ui32TaskRdbOffset
									);

								/*	Write the configuration structure into DRAM */
								/*	First I/O buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) psIoBuffTemp,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );

								psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
								
								/*	Next Generic buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) psIoGenericBuffTemp,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

								psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);
								
								ui32OpNum++;
							
								/*	Fill in the o/p port cfg structure */
								/*
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32OpPortCfgAddr = 
											psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst];
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelayValue = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32Delay;
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelaySign = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32IndDelaySign;
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32FmmAdaptRateCntrlRegAddr = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32AdapRateCntrlRegAddr;

								ui32OpPortNum++;

  								*/
								break;

							case BRAP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf:	
								
								/* This is the inter task DRAM buffer cfg */

								psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] 
										= psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sInterTaskDramBuffCfg.ui32IoBuffCfgAddr;

								psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst]
										= psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sInterTaskDramBuffCfg.ui32IoGenericBuffCfgAddr;

								break;

							case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:

								
							/* case BRAP_CIT_P_FwStgSrcDstType_eDram:*/
							default:

								
								/*	Write the configuration structure into DRAM */
								/*	First I/O buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) &psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoBuffer.sIoBuffer,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) );

								psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_BUFFER);
								
								/*	Next Generic buff Cfg struct */
								BRAP_CITGEN_P_WriteToDRAM( 
									hHeap,
									(uint32_t) &psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffer,
									ui32IoPortCfgStructAddr,
									(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );

								psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst] = ui32IoPortCfgStructAddr;

								ui32IoPortCfgStructAddr += SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER);
								
								ui32OpNum++;

								/*	Fill in the o/p port cfg structure */
								/*
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32OpPortCfgAddr = 
											psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst];
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelayValue = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32Delay;
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32IndependentDelaySign = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32IndDelaySign;
								psOpPortInfo->sOpPortCfg[ui32OpPortNum].ui32FmmAdaptRateCntrlRegAddr = 
									psStgInfo->sStgConnectivity.sDstDetails[ui32DistOp][ui32Dst].uDetails.sIoBuf.ui32AdapRateCntrlRegAddr;
							
								ui32OpPortNum++;
								*/
								break;
							}
						}
						else
						{

							ui32InterstageBufferId = psCitDataStructure->sOutputIsCfg[ui32Algo][ui32DistOp].ui32InterstageBufferId;

							psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] =
								psTaskIsBuffCfg->sIsBuffCfg[ui32InterstageBufferId].ui32IoBufferStructAddr;

							psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst] =
								psTaskIsBuffCfg->sIsBuffCfg[ui32InterstageBufferId].ui32GenericBufferStructAddr;

							ui32OpNum++;

							/* This  is for DDP converter */
							/* Here the Idea is to configure the FW Connectivity stage op cfg same as that of the latest interstage buffer*/
							/*
							psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32Dst] =
								psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32IoBufferStructAddr;

							psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32Dst] =
								psTaskIsBuffCfg->sIsBuffCfg[ui32IsBuffId].ui32GenericBufferStructAddr;
								*/

							
						}
					}
				}
				/*	Error check: If number of o/p is greater than the maximum allowed in
					CIT, flag an Error */

				#ifdef BRAP_CIT_P_CHECK_OUTPUT_RANGE
				if( ui32OpNum > BRAP_AF_P_MAX_OP_FORKS )
				{
					BDBG_ERR(("ERROR: The number of o/p Forks =[%d] is greater than the maximum allowed in CIT",ui32OpNum));
					ui32Err = BERR_NOT_SUPPORTED ;
					goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
				}
				#endif
			}
		}	

		/*	FS and Decoder Having same input
			--------------------------------		

			For Audio task : 

				Special case: For Algorithms receiving data from an input port, special 
			nodes are present. Ex: A decoder will have Frame sync and the TSM nodes
			present. We need to configure the decoder's input buffer configuration
			to be the same as frame sync's i/p buff configuration. We short only IoBuffers.

			For Mixer Task; 

				We need to short Io Buffer and Io generic buffer for all the sources.

			For Video task : 

				There is no shoting between FS and Decoder inputs needed..

		*/
		if(BRAP_CIT_P_SameInputForFsAndDecode_eEnable == eSameInputForFsAndDecode)
		{
			/* Audio Task */		
			if( psStgInfo->bIpsFromFwStage == eFALSE )
			{

				if(BRAP_CIT_P_MixerTaskDetection_eDetected == psSpecialTaskPresent->eMixerTaskDetectionFlag)
				{	
					/* Detecting the First Branch and Stage */
					if( (0 == ui32BranchId) && (0 == ui32StageId) )
					{
						/* 
							Shorting Both IO buffers and IO generinc Buffers for all the source inputs btw FS and Decoder. 
						    Shoring the Valid/Invalid field also
						*/
						psNodeAttr = &psStgInfo->sNodeAttr[BRAP_CIT_P_NUM_SECIAL_NODES]; /*Getting the Decode Node */

						/*Shorting the Number of inputs shorted from the input */
						psNodeAttr->ui32NumSrc = psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].ui32NumSrc;

						for(ui32SrcCount =0; ui32SrcCount< psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].ui32NumSrc ; ui32SrcCount++)
						{
							/*Io Buffer and Io Generic buffer */
							psNodeAttr->ui32NodeIpBuffCfgAddr[ui32SrcCount] = 
								psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].ui32NodeIpBuffCfgAddr[ui32SrcCount];

							psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32SrcCount] = 
								psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].ui32NodeIpGenericDataBuffCfgAddr[ui32SrcCount];							

							/* Valid/Invalid Flag*/
							psNodeAttr->eNodeIpValidFlag[ui32SrcCount] = 
								psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].eNodeIpValidFlag[ui32SrcCount];
						}
					}					
				}
				else
				{
					psNodeAttr = &psStgInfo->sNodeAttr[BRAP_CIT_P_NUM_SECIAL_NODES];

					psNodeAttr->ui32NodeIpBuffCfgAddr[BRAP_CIT_P_SRC0] = 
						psStgInfo->sNodeAttr[BRAP_CIT_P_NODE0].ui32NodeIpBuffCfgAddr[BRAP_CIT_P_SRC0];
				}
			}
		}
		else
		{
			/* Video Task */
		}

	}


	/*--------------------------*/
	/* Output Range Error Check */
	/*--------------------------*/
#ifdef BRAP_CIT_P_CHECK_OUTPUT_RANGE

	if(ui32FmmOpNum > BRAP_AF_P_MAX_FMM_OP_PORTS_IN_TASK)
	{
		BDBG_ERR(("ERROR: The number of FMM Output ports = [%d] is greater than the maximum allowed in CIT",ui32FmmOpNum));
		ui32Err = BERR_NOT_SUPPORTED ;
		goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
	}

	if(ui32OpPortNum > BRAP_AF_P_MAX_OP_PORTS)
	{
		BDBG_ERR(("ERROR: The number of Output ports = [%d] is greater than the maximum allowed in CIT",ui32OpPortNum));
		ui32Err = BERR_NOT_SUPPORTED ;
		goto BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT;				
	}

#endif
	/*	Update the number of FMM o/p ports for the entire algo network and the Maxdelay */
	psTaskFmmGateOpenConfig->ui32NumPorts		= ui32FmmOpNum;

	if(BRAP_AF_P_eDisable == eIndepDelayEnableFlag)		/* Making Max Indep Dalay =0 on disable mode*/
	{
		psTaskFmmGateOpenConfig->ui32MaxIndepDelay	= 0;
	}
	else
	{
		psTaskFmmGateOpenConfig->ui32MaxIndepDelay	= BRAP_AF_P_MAX_INDEPENDENT_DELAY;
	}

	/*	Update the number of o/p ports for the entire algo network */
	psOpPortInfo->ui32NumopPorts				= ui32OpPortNum;


	/* Exit point*/
	/*--------------------------------------*/
	BRAP_CITGENMODULE_P_ALLOC_IOBUFF_EXIT:
	BKNI_Free(psTaskIsBuffCfg);
	/*--------------------------------------*/

	BDBG_LEAVE(BRAP_CITGEN_P_AllocIoBuffCfg);
	return ui32Err;
}


/******************************************************************************
Summary:
		Populating the PPM cfg addr

Description:

		This function takes PI configuration and populates PPM address for FW

Input:
		psIoBufferId: PPM PI input
		pui32PPMCount : PPM count
		
Output:
		sPpmCfgArray: PPm cfg for FW

Returns:
		Error 

******************************************************************************/
static uint32_t BRAP_CITGEN_P_PopulatePpmCfg(
				BRAP_CIT_P_sIO_BUFF_ID  *psIoBufferId,
				BRAP_AF_P_sPPM_CFG		sPpmCfgArray[],
				uint32_t				ui32TaskRdbOffset,
				uint32_t				*pui32PPMCount
			)
{

	uint32_t	ui32Count;
	uint32_t	ui32Error;
	uint32_t	ui32PPMCount;

		

	BDBG_ENTER(BRAP_CITGEN_P_PopulatePpmCfg);

	ui32Error = BERR_SUCCESS;

	ui32PPMCount = *pui32PPMCount;

	for(ui32Count =0; ui32Count<BRAP_AF_P_MAX_CHANNEL_PAIR;ui32Count++)
	{		

		if(psIoBufferId->ui32AdaptRateCtrlIds[ui32Count] != BRAP_CIT_P_PI_INVALID)
		{
			if(*pui32PPMCount >= BRAP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS)
			{

				BDBG_LEAVE(BRAP_CITGEN_P_PopulatePpmCfg);
				return BERR_LEAKED_RESOURCE;
			}			

			sPpmCfgArray[*pui32PPMCount].ePPMChannel = BRAP_AF_P_eEnable;

			BRAP_CITGEN_P_TransPpmBuffId2Addr(
								psIoBufferId->ui32AdaptRateCtrlIds[ui32Count],
								ui32TaskRdbOffset,
								&sPpmCfgArray[*pui32PPMCount].ui32PPMCfgAddr
							);		

			
			*pui32PPMCount = *pui32PPMCount + 1;
		}		
	}

	BDBG_LEAVE(BRAP_CITGEN_P_PopulatePpmCfg);

	
	return ui32Error;
}

/******************************************************************************
Summary:
		Translates the PPM ids to Addr

Description:

		Translates the PPM ids to Addr
Input:
		ui32BufferId:		PPM buffer ID information
		ui32TaskRdbOffset:	RDB Offset
		
Output:
		pui32PPMCfgAdd:		PPM RDB physical Address

Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_TransPpmBuffId2Addr(
							uint32_t ui32BufferId,
							uint32_t ui32TaskRdbOffset,
							uint32_t *pui32PPMCfgAddr
						)

{
	uint32_t	ui32Buff0AddrStart;
	uint32_t	ui32Buff2BuffOffset;
	
	BDBG_ENTER(BRAP_CITGEN_P_TransPpmBuffId2Addr);

	ui32Buff0AddrStart	= ui32TaskRdbOffset + BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0;
	ui32Buff2BuffOffset	= BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_1 - 
									BCHP_AUD_FMM_BF_CTRL_ADAPTRATE_REPEATDROP_WRCNT_0;


	*pui32PPMCfgAddr = ui32Buff0AddrStart + ui32BufferId * ui32Buff2BuffOffset;

	BDBG_LEAVE(BRAP_CITGEN_P_TransPpmBuffId2Addr);

}


/******************************************************************************
Summary:
		Initialize the PPM cfg

Description:

		Initialize the PPM cfg
Input:
		sPpmCfgArray:	PPM output
		
Output:
		sPpmCfgArray:	PPM output

Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_InitializePpmCfg(
				BRAP_AF_P_sPPM_CFG		sPpmCfgArray[]
			)
{
	uint32_t	ui32Count;

	BDBG_ENTER(BRAP_CITGEN_P_InitializePpmCfg);
	
	for(ui32Count =0; ui32Count<BRAP_AF_P_MAX_CHANNEL_PAIR;ui32Count++)
	{
		sPpmCfgArray[ui32Count].ePPMChannel		= BRAP_AF_P_eDisable;
		sPpmCfgArray[ui32Count].ui32PPMCfgAddr	= (uint32_t)NULL;
	}

	BDBG_LEAVE(BRAP_CITGEN_P_InitializePpmCfg);
}


/******************************************************************************
Summary:
		Populates the FW Hardware configuration structure

Description:
		
Input:
		psFwHwCfgIp:	Hardware configuration structure
		
Output:
		ui32TaskFwHwCfgAddr:	Dram Address

Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_FwHwBufferCfg(
					BMEM_Handle				hHeap,
					BRAP_AF_P_sFW_HW_CFG    *psFwHwCfgIp,
					uint32_t				ui32TaskFwHwCfgAddr
				)	

{
	uint32_t				ui32Error;

	BDBG_ENTER(BRAP_CITGEN_P_FwHwBufferCfg);

	ui32Error = BERR_SUCCESS;

	BRAP_CITGEN_P_WriteToDRAM(
					hHeap,
					(uint32_t)psFwHwCfgIp,
					(uint32_t)ui32TaskFwHwCfgAddr,
					(uint32_t)SIZEOF(BRAP_AF_P_sFW_HW_CFG)
			 );

	BDBG_LEAVE(BRAP_CITGEN_P_FwHwBufferCfg);
}


/******************************************************************************
Summary:
		This function prepares the I/O buff Cfg structures only for inter-stage
	buffers.

Description:

		Prepares the I/O buff Cfg and IO generic buffer structures only for
		inter-stage	buffers.

		While populating the CIT, for algos which take in configuration	from a 
		port, the configuratio is available in the CIT input structures	already.
		Ony for inter-stage buffers the configuration is not available.	This is
		being prepared here
		
Input:
		ui32InterStgBuffBaseAddr:	Inter Stage Buffer Base Address
		psTaskBuffInfo
		
Output:
		sIsBuffCfg:	

Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_PrepareIsIoBuffCfgStruct( 
				uint32_t						ui32InterStgBuffBaseAddr,
				BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo,
				BRAP_CIT_P_sBuffCfg			*psIsBuffCfg 
			)
{
	uint32_t ui32Ch;
	uint32_t ui32Buff;	
	uint32_t ui32NumIsBuffers;
	uint32_t ui32IsIoBuffBaseAddr;
	uint32_t ui32MaxSizePerChannel;
	uint32_t ui32CurrIsIoBuffBaseAddr;
	uint32_t ui32IsGenericBuffBaseAddr;	
	uint32_t ui32CurrIsGenericBuffBaseAddr;
	
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuff;
	

	BDBG_ENTER(BRAP_CITGEN_P_PrepareIsIoBuffCfgStruct);

	ui32NumIsBuffers = psTaskBuffInfo->ui32NumInterStgBuffs;

	/*	Split the inter-stage buffer chunk into two.
		Inter-stage I/O chunk and inter-stage generic buffer chunk 
	*/
	ui32IsIoBuffBaseAddr = ui32InterStgBuffBaseAddr;
	ui32IsGenericBuffBaseAddr = ui32IsIoBuffBaseAddr + 
								psTaskBuffInfo->ui32TaskInterStgIoMemSize;

	ui32MaxSizePerChannel = psTaskBuffInfo->ui32MaxSizePerChannel;

	for( ui32Buff=0; ui32Buff<ui32NumIsBuffers; ui32Buff++ )
	{
		ui32CurrIsIoBuffBaseAddr = ui32InterStgBuffBaseAddr + 
					psTaskBuffInfo->ui32BranchInterStgIoMemSize * ui32Buff;


		psIsBuffCfg->ui32PortAssociated = 0; /* No ports are associated  */

		/*	Forming the actual I/O buff struct here for Inter-stage I/O buffer 	*/
		psIsBuffCfg->sIoBuffer.ui32NumBuffers 
											= psTaskBuffInfo->ui32MaxNumChannelsSupported;

		psIsBuffCfg->sIoBuffer.eBufferType = BRAP_AF_P_BufferType_eDRAM_IS;

		for( ui32Ch=0; ui32Ch<psTaskBuffInfo->ui32MaxNumChannelsSupported; ui32Ch++ )
		{
			psCircBuff = &psIsBuffCfg->sIoBuffer.sCircBuffer[ui32Ch];
			
			psCircBuff->ui32BaseAddr = ui32CurrIsIoBuffBaseAddr + ui32Ch * ui32MaxSizePerChannel;
			psCircBuff->ui32ReadAddr = psCircBuff->ui32BaseAddr;
			psCircBuff->ui32WriteAddr = psCircBuff->ui32BaseAddr;

			psCircBuff->ui32EndAddr = psCircBuff->ui32BaseAddr + ui32MaxSizePerChannel;
			psCircBuff->ui32WrapAddr = psCircBuff->ui32EndAddr;
		}

		ui32CurrIsGenericBuffBaseAddr = ui32IsGenericBuffBaseAddr + 
					psTaskBuffInfo->ui32BranchInterStgGenericMemSize * ui32Buff;

		/*	Forming the actual I/O buff struct here for Inter-stage Generic	buffer */

		psIsBuffCfg->sGenericBuffer.ui32NumBuffers = 1;
		psIsBuffCfg->sGenericBuffer.eBufferType = BRAP_AF_P_BufferType_eDRAM_IS;
		
		psCircBuff = &psIsBuffCfg->sGenericBuffer.sCircBuffer;

		psCircBuff->ui32BaseAddr = ui32CurrIsGenericBuffBaseAddr;
		psCircBuff->ui32ReadAddr = psCircBuff->ui32BaseAddr;
		psCircBuff->ui32WriteAddr = psCircBuff->ui32BaseAddr;

		psCircBuff->ui32EndAddr = psCircBuff->ui32BaseAddr + 
						psTaskBuffInfo->ui32BranchInterStgGenericMemSize;
		psCircBuff->ui32WrapAddr = psCircBuff->ui32EndAddr;


		psIsBuffCfg++;
	}	

	BDBG_LEAVE(BRAP_CITGEN_P_PrepareIsIoBuffCfgStruct);
}

/******************************************************************************
Summary:
		This function writes the I/O buffer structures to DRAM. This function
		is called only for the Inter-stage buffers only.

Description:
		
Input:
		hHeap:	
		ui32NumInterStgBuffs
		sIsBuffCfg
		
Output:
		ui32IoBuffCfgStructBaseAddr:	

Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_WriteIsIoBuffCfgStructToDram(
							BMEM_Handle	hHeap,
							uint32_t ui32NumInterStgBuffs,
							uint32_t ui32IoBuffCfgStructBaseAddr, 
							BRAP_CIT_P_sBuffCfg sIsBuffCfg[] 
						)
{
	uint32_t ui32IsBuff;
	uint32_t ui32IoBuffCfgStructAddr;
	uint32_t ui32IoGenericBuffCfgStructAddr;
	

	BDBG_ENTER(BRAP_CITGEN_P_WriteIsIoBuffCfgStructToDram);
	
	for( ui32IsBuff=0; ui32IsBuff<ui32NumInterStgBuffs; ui32IsBuff++)
	{
		ui32IoBuffCfgStructAddr = ui32IoBuffCfgStructBaseAddr;
		ui32IoGenericBuffCfgStructAddr = ui32IoBuffCfgStructAddr + 
											SIZEOF(BRAP_AF_P_sIO_BUFFER);

		/*	Write the structures to DRAM. */
		BRAP_CITGEN_P_WriteToDRAM(	hHeap,
									(uint32_t) &sIsBuffCfg[ui32IsBuff].sIoBuffer,
									ui32IoBuffCfgStructAddr,
									SIZEOF(BRAP_AF_P_sIO_BUFFER) );

		BRAP_CITGEN_P_WriteToDRAM(	hHeap,
									(uint32_t) &sIsBuffCfg[ui32IsBuff].sGenericBuffer,
									ui32IoGenericBuffCfgStructAddr,
									SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) 
									);

		sIsBuffCfg[ui32IsBuff].ui32IoBufferStructAddr = ui32IoBuffCfgStructAddr;
		sIsBuffCfg[ui32IsBuff].ui32GenericBufferStructAddr = ui32IoGenericBuffCfgStructAddr;

		ui32IoBuffCfgStructBaseAddr += (	SIZEOF(BRAP_AF_P_sIO_BUFFER) +
										SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) );
	}

	BDBG_LEAVE(BRAP_CITGEN_P_WriteIsIoBuffCfgStructToDram);
}


/*	This function writes the buffers into DRAM. WARNING!!! This function is
	assuming that the size of the write will always be in Dwords. */
static void BRAP_CITGEN_P_WriteToDRAM( BMEM_Handle	hHeap,
								uint32_t	ui32SrcAddr,
								uint32_t	ui32DestAddr,
								uint32_t	ui32Size )
{
	uint32_t	i;
	uint32_t	*pSrc = (uint32_t *)ui32SrcAddr;
	uint32_t	*pDest;
	void		*pDestTemp;


	/* BDBG_ENTER(BRAP_CITGEN_P_WriteToDRAM); */ /* Removing to reduce stack*/

	/*	Need to convert the physical address to virtual address to access
		DRAM */
	BRAP_ConvertOffsetToAddress(	hHeap,
									ui32DestAddr,
									((void**)(&pDestTemp)));

	pDest = (uint32_t*)pDestTemp;

	for( i=0; i<(ui32Size >> 2); i++ )
	{
		*pDest++ = *pSrc++;
	}

	/* BDBG_LEAVE(BRAP_CITGEN_P_WriteToDRAM); */ /* Removing to reduce stack*/

}


/******************************************************************************
Summary:
		Allcate buffer for Scratch buffer and associate it to all nodes

Description:

  		CIT Generation module traverse through all the nodes to find the worst 
		case buffer requirement for scratch buffer. Based on it, CIT 
		allocates memory and associate the same mnemory to all nodes.
		
Input:
		ui32ScratchBuffBaseAddr:	Scratch Buffer Base Address
		
Output:
		psAlgoNwInfo: Local Node info CIT structure		

Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_AllocScratchBuff(	
					uint32_t					ui32ScratchBuffBaseAddr,
					BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo 
				)
{

	uint32_t	ui32Node;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;


	BDBG_ENTER(BRAP_CITGEN_P_AllocScratchBuff);

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psNodeAttr->ui32ScratchBuffDramAddress = ui32ScratchBuffBaseAddr;
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_AllocScratchBuff);
}


/******************************************************************************
Summary:
		Filling the node configuration of CIT

Description:

  		BRAP_AF_P_sNODE_CONFIG parameters are filled
		
Input:
		psAlgoNwInfo:
		sAlgoExecOrder:
		
Output:
		psCit : CIT output datastructure	

Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoCit(	
					BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
					BRAP_CIT_P_sAlgoExecOrder	sAlgoExecOrder[],
					BRAP_AF_P_sTASK_CONFIG		*psCit 
				)
{

	uint32_t	ui32Error;
	uint32_t	ui32BranchNum;
	uint32_t	ui32Algo, ui32Node;
	uint32_t	ui32NumAlgosInNw;
	uint32_t	ui32BranchId, ui32StageId;
	uint32_t	ui32NumNodesInAlgo;
	uint32_t	ui32NodeIndex;

	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_AF_P_sNODE_CONFIG	*psNodeCfg;


	BDBG_ENTER(BRAP_CITGEN_P_FillNodeCfgIntoCit);

	ui32Error = BERR_SUCCESS;

	/*	Find the number of algos within the network. */
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}

	/*	Update the CIT nodes based on execution order */
	psNodeCfg = psCit->sNodeConfig;
	ui32NodeIndex = 0;

	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{
		ui32BranchId = sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId = sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

		for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
		{
			psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
			psNodeCfg->uiNodeId = ui32NodeIndex;

			/*Populating the Collect Residual Flag */
			psNodeCfg->eCollectResidual = (psNodeAttr->ui32BranchId == 0) ? BRAP_AF_P_eEnable : BRAP_AF_P_eDisable;


			/*	This function updates the Node config with the parameters in 
				the	node attributes structure */
			ui32Error = BRAP_CITGEN_P_UpdtNodeCfgInCit (
												psNodeAttr,
												psNodeCfg 
											);
			if(ui32Error != BERR_SUCCESS)
			{
				return ui32Error;
			}

			psNodeCfg++;
			ui32NodeIndex++;
		}
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FillNodeCfgIntoCit);

	return ui32Error;
}



/******************************************************************************
Summary:
		Configure the node Cfg paramenters

Description:

		This function updates the node config in the CIT with the params
		in the node attributes structure

Input:
		psNodeAttr	:Node information
		
Output:
		psNodeCfg	:Node configuration

Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_UpdtNodeCfgInCit(
						BRAP_CIT_P_sNodeAttr	*psNodeAttr,
						BRAP_AF_P_sNODE_CONFIG	*psNodeCfg
					)
{

	uint32_t	ui32Error;
	uint32_t	ui32Ip;


	BDBG_ENTER(BRAP_CITGEN_P_UpdtNodeCfgInCit);

	ui32Error = BERR_SUCCESS;

	psNodeCfg->eAlgoId = psNodeAttr->eAlgoId;

	/* Audio Algorithm Type */
	psNodeCfg->ui32AudioAlgorithmType = psNodeAttr->ui32AudioAlgorithmType;

	/*	Code Buffer */
	psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress = 
									psNodeAttr->ui32CodeDramAddress;
	psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes = 
									psNodeAttr->ui32CodeSize;	

	/*	Inter-Frame buffer */
	psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress = 
									psNodeAttr->ui32InterFrmBuffDramAddress;
	psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes = 
									psNodeAttr->ui32InterFrmBuffSize;

	/*	ROM Table buffer */
	psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress = 
									psNodeAttr->ui32RomTableDramAddress;
	psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes = 
									psNodeAttr->ui32RomTableSize;

	/*	User Config buffer */
	psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress = 
									psNodeAttr->ui32UsrCfgBuffDramAddress;
	psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes = 
									psNodeAttr->ui32UsrCfgBuffSize;


	/*	Status Buffer Cfg */
	psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress=
									psNodeAttr->ui32FwStatusBuffDramAddress;

	psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes=
									psNodeAttr->ui32FwStatusBuffSize;


	/*	Input Configuration */
	for( ui32Ip=0; ui32Ip<BRAP_AF_P_MAX_IP_FORKS; ui32Ip++ )
	{

		psNodeCfg->eNodeIpValidFlag[ui32Ip] = 
					psNodeAttr->eNodeIpValidFlag[ui32Ip];

		psNodeCfg->ui32NodeIpBuffCfgAddr[ui32Ip] = 
					psNodeAttr->ui32NodeIpBuffCfgAddr[ui32Ip];

		psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32Ip] = 
					psNodeAttr->ui32NodeIpGenericDataBuffCfgAddr[ui32Ip];
	}

		
	/* Output Configuration */
	ui32Error = BRAP_CITGEN_P_FillOpForkCfg( 
								 psNodeAttr,
								 psNodeCfg 
							   );

	/*	Num Src and destination for the node */
	psNodeCfg->ui32NumSrc = psNodeAttr->ui32NumSrc;
	psNodeCfg->ui32NumDst = psNodeAttr->ui32ModifiedNumDst; /*Use the modified Dst o/ps*/


	BDBG_LEAVE(BRAP_CITGEN_P_UpdtNodeCfgInCit);

	return(ui32Error);

}

/******************************************************************************
Summary:
		Configure the node output fork configuration 

Description:

		This function creates a linear output fork array from the two Dimensional 
		DistinctOp and Output port matrix.

  Some Exceptions are made to handle the MASTER SLAVE cfg:
		We need to Merge all the FMM output ports in a distinct output for the FW.
		Here we asume that First FMM port is the master and rest are slave....
		So FW codecs/nodes need not to do any anything with SLAVE....
		FW will be configured only with the MAster buffer.

		To Implement this, I have done sorting in the following order
			1) FMM
			2) Non FMM and Non IS buffers (RDB buffers)
			3) IS buffers


Input:
		psNodeAttr	:Node information
		
Output:
		psNodeCfg	:Node configuration

Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillOpForkCfg(
					BRAP_CIT_P_sNodeAttr	*psNodeAttr,
					BRAP_AF_P_sNODE_CONFIG	*psNodeCfg
				)
{
	uint32_t ui32Error;	
	uint32_t ui32NumDst;
	uint32_t ui32DistOp;
	uint32_t ui32OpPort;
	uint32_t ui32OpForkIndex;

	BRAP_CIT_P_FwStgSrcDstType eDstType;

	BDBG_ENTER(BRAP_CITGEN_P_FillOpForkCfg);

	ui32Error = BERR_SUCCESS;

	ui32NumDst		= 0;
	ui32OpForkIndex	= 0;


	/*Process only the FMM outputs */
	/*Sorting all FMM ports together */

	for(ui32DistOp =0;ui32DistOp<psNodeAttr->ui32NumDistinctOp;ui32DistOp++)
	{
		for(ui32OpPort=0;ui32OpPort<psNodeAttr->ui32NumPortsPerDistOp[ui32DistOp];ui32OpPort++)
		{			
			eDstType = psNodeAttr->eConnectivityType[ui32DistOp][ui32OpPort];			
			
			if(BRAP_CIT_P_FwStgSrcDstType_eFMMBuf == eDstType)
			{
				psNodeCfg->ui32NodeOpBuffCfgAddr[ui32OpForkIndex]
						= psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32OpPort];

				psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32OpForkIndex] = 
						psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32OpPort];

				#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING			
					psNodeCfg->eNodeOpBuffDataType[ui32OpForkIndex] = 
							psNodeAttr->eNodeOpBuffDataType[ui32DistOp][ui32OpPort];
				#endif

				ui32NumDst++;			/*Incrementing Actual Dst node outputs */
				ui32OpForkIndex++;		/*Incrementing Fork index */				
				 /* break; */
			}
		}
	}

	/* Process only the Non-Interstage/Non-FMM outputs like RDB */
	for(ui32DistOp =0;ui32DistOp<psNodeAttr->ui32NumDistinctOp;ui32DistOp++)
	{
		for(ui32OpPort=0;ui32OpPort<psNodeAttr->ui32NumPortsPerDistOp[ui32DistOp];ui32OpPort++)
		{			
			eDstType = psNodeAttr->eConnectivityType[ui32DistOp][ui32OpPort];			
			
			if(
				((BRAP_CIT_P_FwStgSrcDstType_eFwStg		== eDstType) || 
				 (BRAP_CIT_P_FwStgSrcDstType_eFMMBuf	== eDstType) ||
				 (BRAP_CIT_P_FwStgSrcDstType_eMax		== eDstType)
				)
			  )
			{
				/* Do nothing */
			}
			else
			{				

				psNodeCfg->ui32NodeOpBuffCfgAddr[ui32OpForkIndex]
						= psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32OpPort];

				psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32OpForkIndex] = 
						psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32OpPort];

				#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING			
					psNodeCfg->eNodeOpBuffDataType[ui32OpForkIndex] = 
							psNodeAttr->eNodeOpBuffDataType[ui32DistOp][ui32OpPort];
				#endif

				ui32NumDst++;			/*Incrementing Actual Dst node outputs */
				ui32OpForkIndex++;		/*Incrementing Fork index */				
			}
		}
	}


	/*Process only the Non-Interstage outputs */
	for(ui32DistOp =0;ui32DistOp<psNodeAttr->ui32NumDistinctOp;ui32DistOp++)
	{		
		for(ui32OpPort=0;ui32OpPort<psNodeAttr->ui32NumPortsPerDistOp[ui32DistOp];ui32OpPort++)
		{
			eDstType = psNodeAttr->eConnectivityType[ui32DistOp][ui32OpPort];

			if(BRAP_CIT_P_FwStgSrcDstType_eFwStg == eDstType)
			{		
				
				psNodeCfg->ui32NodeOpBuffCfgAddr[ui32OpForkIndex]
						= psNodeAttr->ui32NodeOpBuffCfgAddr[ui32DistOp][ui32OpPort];

				psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32OpForkIndex] = 
						psNodeAttr->ui32NodeOpGenericDataBuffCfgAddr[ui32DistOp][ui32OpPort];

				#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING			
					psNodeCfg->eNodeOpBuffDataType[ui32OpForkIndex] = 
							psNodeAttr->eNodeOpBuffDataType[ui32DistOp][ui32OpPort];
				#endif

				ui32NumDst++;			/*Incrementing Actual Dst node outputs */
										/*Skipping all the IS buffers and merging to one */
				ui32OpForkIndex++;		/*Incrementing Fork index */

				break;
			}
		}
	}

	/*Updating the modified Number of Dsp Ouptuts*/
	psNodeAttr->ui32ModifiedNumDst = ui32NumDst;

	/*Range check for maximum Distinct output and output ports */

	/*WE NEED TO REDEFINE THE DEFINITION OF "BRAP_AF_P_MAX_OP_FORKS"*/
	if(ui32OpForkIndex > BRAP_AF_P_MAX_OP_FORKS)
	{				
		BDBG_ERR(("ERROR: The total number of Outputs [Distinct and Op] =[%d] exceeds the range = [%d] ",ui32OpForkIndex,BRAP_AF_P_MAX_OP_FORKS));
		BDBG_LEAVE(BRAP_CITGEN_P_FillOpForkCfg);

		ui32Error = BERR_INVALID_PARAMETER;
		return ui32Error;
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FillOpForkCfg);

	return ui32Error;
}

/******************************************************************************
Summary:
		Filling the global configuration of CIT

Description:

  		BRAP_AF_P_sGLOBAL_TASK_CONFIG parameters are filled
		
Input:
		psAlgoNwInfo:
		psOpPortInfo:
		ui32ScratchBuffBaseAddr
		ui32TaskScratchMemSize
		ui32TaskPortConfigAddr
		ui32TaskGateOpenConfigAddr
		ui32TaskFwHwCfgAddr
		ui32FwOpSamplingFreqMapLutAddr
		ui32ZeroFillSamples
		eTimeBaseType

Output:
		psGblTaskCfg : CIT global task datastructure	

Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillGblTaskCfgIntoCit(	
						BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo,
						BRAP_CIT_P_sOpPortInfo		*psOpPortInfo,
						uint32_t					ui32ScratchBuffBaseAddr,
						uint32_t					ui32TaskScratchMemSize,
						BRAP_AF_P_sGLOBAL_TASK_CONFIG *psGblTaskCfg,
						uint32_t					ui32TaskPortConfigAddr,
						uint32_t					ui32TaskGateOpenConfigAddr,
						uint32_t					ui32TaskFwHwCfgAddr,
						uint32_t					ui32FwOpSamplingFreqMapLutAddr,
						uint32_t					ui32ZeroFillSamples,
						BRAP_AF_P_TimeBaseType		eTimeBaseType
					)
{
	uint32_t	ui32Error;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInNw;
	uint32_t	ui32Port;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	
	BDBG_ENTER(BRAP_CITGEN_P_FillGblTaskCfgIntoCit);
	
	ui32NumNodesInNw = 0;
	ui32Error = BERR_SUCCESS;


	/*	Compute the number of nodes in the network */
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInNw += psStgInfo->ui32NumNodes;
		}
	}

	if(ui32NumNodesInNw>BRAP_AF_P_MAX_NODES)
	{
		BDBG_ERR(("Error : The number of nodes in the system is %d. Maximum Allowed is %d", ui32NumNodesInNw,BRAP_AF_P_MAX_NODES));

		return(BERR_NOT_SUPPORTED);
	}

	psGblTaskCfg->ui32NumberOfNodesInTask = ui32NumNodesInNw;

	/*	Fill in the o/p port configuration */
	psGblTaskCfg->ui32NumOpPorts = psOpPortInfo->ui32NumopPorts;
	for( ui32Port=0; ui32Port<psGblTaskCfg->ui32NumOpPorts; ui32Port++ )
	{
		psGblTaskCfg->sOpPortCfg[ui32Port] = psOpPortInfo->sOpPortCfg[ui32Port];
	}

	/*	Fill in the scratch buffer details */
	psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress = ui32ScratchBuffBaseAddr;
	psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes = ui32TaskScratchMemSize;

	/*	Start node index */
	psGblTaskCfg->ui32StartNodeIndexOfCoreAudioAlgorithm = BRAP_CIT_P_NUM_SECIAL_NODES;

	/*	WARNING!!! Num zero fill frames not filled. This may be required 
		for bring up */

	/*	WARNING!!! All other global task parameters are unfilled */


	/* Add port Config and SPDIF Config */
	psGblTaskCfg->ui32FmmDestCfgAddr = ui32TaskPortConfigAddr;

	/* Add port Config and SPDIF Config */
	psGblTaskCfg->ui32FmmGateOpenConfigAddr			= ui32TaskGateOpenConfigAddr;

	/* Add Fw Hw cfg address*/
	psGblTaskCfg->ui32TaskFwHwCfgAddr				= ui32TaskFwHwCfgAddr;

	/* Add FW Op Sampling Frequency Cfg*/
	psGblTaskCfg->ui32FwOpSamplingFreqMapLutAddr	= ui32FwOpSamplingFreqMapLutAddr;
	
	/* Zero Fill samples ::: Currently not used by FW */
	psGblTaskCfg->ui32NumberOfZeroFillSamples		= ui32ZeroFillSamples;

	/*Filling the time base type */
	psGblTaskCfg->eTimeBaseType						= eTimeBaseType;

	BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoCit);

	return ui32Error;
}

/******************************************************************************
Summary:
		Filling CIT Gen -PI output datastructure
Description:

  		CIT Gen - PI output data structure is filled
		All the buffer addresses should be converted to offset address
		
Input:
		psTaskStackBuff:
		psTaskSPDIFPortConfigAddr:
		psAlgoNwInfo

		
Output:
		psCitOp : CIT-Gen to PI output 	datastructure

Returns:
		Error

******************************************************************************/
static void BRAP_CITGEN_P_FillCitOp(
					BMEM_Handle						hHeap,
					BRAP_AF_P_sDRAM_BUFFER			*psTaskStackBuff,
					BRAP_AF_P_sDRAM_BUFFER			*psTaskSPDIFPortConfigAddr,
					BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
					BRAP_CIT_P_Output				*psCitOp )
{

	uint32_t	ui32Count;
	uint32_t	ui32Node;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_OpStgInfo	*psCitGenOpStgInfo;

	uint32_t	*pDest;
	void		*pDestTemp;
	
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_CIT_P_FwBufInfo		*psOpNodeInfo;

	
	BDBG_ENTER(BRAP_CITGEN_P_FillCitOp);
	/* Stack swap address */
	BRAP_ConvertOffsetToAddress(hHeap,
								psTaskStackBuff->ui32DramBufferAddress,
								(void **)&pDestTemp
								);

	pDest	= (uint32_t*)pDestTemp ;

	psCitOp->sStackSwapBuff.ui32DramBufferAddress
						=	(uint32_t)pDest;
	psCitOp->sStackSwapBuff.ui32BufferSizeInBytes
						=	psTaskStackBuff->ui32BufferSizeInBytes;


	/*  SPDIF User Cfg*/
	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_SPDIF_PORTS;ui32Count++)
	{

		BRAP_ConvertOffsetToAddress(hHeap,
									psTaskSPDIFPortConfigAddr[ui32Count].ui32DramBufferAddress,
									(void **)&pDestTemp
									);

		pDest	= (uint32_t*)pDestTemp ;

		psCitOp->sSpdifUserConfigAddr[ui32Count].ui32DramBufferAddress = 			
					(uint32_t)pDest;

		psCitOp->sSpdifUserConfigAddr[ui32Count].ui32BufferSizeInBytes = 			
					psTaskSPDIFPortConfigAddr[ui32Count].ui32BufferSizeInBytes;
	}


	psCitOp->ui32NumBranches = psAlgoNwInfo->ui32NumBranches;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
		
		psCitOp->sCitBranchInfo[ui32BranchNum].ui32NumStages =  
														ui32NumAlgoStgInBranch;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			psCitGenOpStgInfo = &psCitOp->sCitBranchInfo[ui32BranchNum].sCitStgInfo[ui32AlgoStg];

			psCitGenOpStgInfo->ui32NumNodes		= psStgInfo->ui32NumNodes;
			psCitGenOpStgInfo->uAlgorithm		= psStgInfo->uAlgorithm;
			psCitGenOpStgInfo->eAudioAlgoType	= psStgInfo->eAudioAlgoType;		
			psCitGenOpStgInfo->uAudioMode		= psStgInfo->uAudioMode;
			
			if( psStgInfo->bIpsFromFwStage == eFALSE )
			{
				psCitGenOpStgInfo->ui32StartNodeIndex	= BRAP_CIT_P_NUM_SECIAL_NODES;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_TSM_NODE_INDEX;
			}
			else
			{
				/*
				psCitGenOpStgInfo->ui32StartNodeIndex	= BRAP_CIT_P_INVALID_NODE_IDX;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
				*/

				/* This is added to get the status information from the last stage of the network*/
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}

			for ( ui32Node=0; ui32Node<psStgInfo->ui32NumNodes; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->eFwExecId = psNodeAttr->eAlgoId;

				/* Interframe address */
				if(psNodeAttr->ui32InterFrmBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32InterFrmBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				
				psOpNodeInfo->ui32InterframeBufAdr	= (uint32_t)pDest;
				psOpNodeInfo->ui32InterframeBufSize = psNodeAttr->ui32InterFrmBuffSize;
				
				/* User param configuration */
				if(psNodeAttr->ui32UsrCfgBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
				/*	Need to convert the physical address to virtual address for the CIT
					o/p structure */
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32UsrCfgBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				psOpNodeInfo->ui32UserParamBufAdr	 = (uint32_t)pDest;
				psOpNodeInfo->ui32UserParamBufSize	 = psNodeAttr->ui32UsrCfgBuffSize;

				/* Status buffer configuration */
				if(psNodeAttr->ui32FwStatusBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32FwStatusBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}
		
				psOpNodeInfo->ui32StatusBufAdr	=  (uint32_t)pDest;
				psOpNodeInfo->ui32StatusBufSize	=  psNodeAttr->ui32FwStatusBuffSize;

			}
			for(ui32Node=psStgInfo->ui32NumNodes;ui32Node<BRAP_AF_P_MAX_NUM_NODES_IN_ALGO;ui32Node++)
															
			{
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;

				psOpNodeInfo->ui32InterframeBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32InterframeBufSize	=  0;

				psOpNodeInfo->ui32UserParamBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32UserParamBufSize	=  0;

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;


			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_FillCitOp);
}




/*	Lib function to find the max */
static uint32_t BRAP_CITGEN_P_Max( uint32_t a,
							uint32_t b )
{
	if( a > b )
		return a;
	else
		return b;
}


/******************************************************************************
Summary:
		Verify the validity of eFmmDstFsRate
	
Description:
	
		This function varifies the range of eFmmDstFsRate

Input:
		eFmmDstFsRate	:  The FMM sink Rate

Output:
	     Status of Validation : BERR_SUCCESS/ FAIL

Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_VerifyFmmFsRate(
				BRAP_AF_P_FmmDstFsRate			eFmmDstFsRate
		)

{
	BDBG_ENTER(BRAP_CITGEN_P_VerifyFmmFsRate);


	if(eFmmDstFsRate < BRAP_AF_P_FmmDstFsRate_eInvalid)
	{

		BDBG_LEAVE(BRAP_CITGEN_P_VerifyFmmFsRate);
		return BERR_SUCCESS;
	}
	else
	{
		
		BDBG_LEAVE(BRAP_CITGEN_P_VerifyFmmFsRate);
		return BERR_UNKNOWN;
	}
}
/******************************************************************************
Summary:
	This function Translates the FMM buffer Id into the absolute addresses
	for the read, write wrap and end pointers 

Description:
	
	For FMM buffer, CIT generation module gets only Buffer Ids from PI,
	This function translates the FMM Ids to FMM RDB physical Address. This
	is applicable to both IOBuffers and IO Generic buffers. The Address derivation is
	based on the Base addess as BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR

	Master Slave Cfg;
			An output FMM port can be either Master/Slave. And the input FMM port
			does not has any Master/Slave configuration.

			If a port is configured as slave then the Buffer type will be configured as
			BRAP_AF_P_BufferType_eFMMSlave. otherwise (Master or others )them the
			BufferType will be copied from the input configuration.

Input:
		psIoBuffId			: IOBuffer cfg		
		psIoGenericBuffId	: IO Generic Buffer cfg				
		ui32TaskRdbOffset	:  Task RDb offset
		eMasterSlaveType	:  Master Slave Control

Output:
		psIoBuffer			: Final IOBuffer cfg
		psIoGenericBuffer	: Final IOGeneric Buffer cfg

Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_TransFmmBuffId2Addr(
			BRAP_CIT_P_sIO_BUFF_ID			*psIoBuffId,
			BRAP_AF_P_sIO_BUFFER			*psIoBuffer,
			BRAP_CIT_P_sIO_BUFF_ID			*psIoGenericBuffId,
			BRAP_AF_P_sIO_GENERIC_BUFFER	*psIoGenericBuffer,
			uint32_t						ui32TaskRdbOffset,
			BRAP_CIT_P_MasterSlaveType		eMasterSlaveType
		)
{
	uint32_t	ui32Buff;
	uint32_t	ui32BuffId;
	uint32_t	ui32Buff2BuffOffset;
	uint32_t	ui32Buff0AddrStart;
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		*psCircBuffer;


	BDBG_ENTER(BRAP_CITGEN_P_TransFmmBuffId2Addr);

	ui32Buff0AddrStart  = ui32TaskRdbOffset + BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
	ui32Buff2BuffOffset = BCHP_AUD_FMM_BF_CTRL_RINGBUF_1_RDADDR - 
								BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR;
	
	/*	First translate the I/O buffers */
	/* ui32NumBuffers population */ 
	psIoBuffer->ui32NumBuffers = psIoBuffId->ui32NumBuffers;

	/* eBufferType population */
	if(BRAP_CIT_P_MasterSlaveType_eSlavePort == eMasterSlaveType)
	{
		psIoBuffer->eBufferType	= BRAP_AF_P_BufferType_eFMMSlave;		
	}
	else
	{
		/*
			BRAP_CIT_P_MasterSlaveType_eMasterPort  ->>>>> For output FMM buffer configuration 
			BRAP_CIT_P_MasterSlaveType_eOthers		->>>>> For input FMM buffer configuration 
		*/
		psIoBuffer->eBufferType = psIoBuffId->eBufferType;
	}

	for( ui32Buff=0; ui32Buff<psIoBuffId->ui32NumBuffers; ui32Buff++)
	{
		psCircBuffer = &psIoBuffer->sCircBuffer[ui32Buff];
		ui32BuffId = psIoBuffId->ui32BufferId[ui32Buff];
		
		psCircBuffer->ui32ReadAddr	= ui32Buff0AddrStart + ui32BuffId * ui32Buff2BuffOffset;
		psCircBuffer->ui32WriteAddr = psCircBuffer->ui32ReadAddr	+ 4;
		psCircBuffer->ui32BaseAddr	= psCircBuffer->ui32ReadAddr	+ 8;
		psCircBuffer->ui32EndAddr	= psCircBuffer->ui32ReadAddr	+ 12;

		psCircBuffer->ui32WrapAddr = psCircBuffer->ui32EndAddr;
	}

	/*	Then translate the I/O generic buffer */

	/* ui32NumBuffers population */ 
	psIoGenericBuffer->ui32NumBuffers	= psIoGenericBuffId->ui32NumBuffers;

	/* Default eBufferType population */
	psIoGenericBuffer->eBufferType		= psIoGenericBuffId->eBufferType;
	
	if( psIoGenericBuffId->ui32NumBuffers != 0 )
	{
		/* eBufferType population Based on the Master Slave configuration */
		if(BRAP_CIT_P_MasterSlaveType_eSlavePort == eMasterSlaveType)
		{		
			psIoGenericBuffer->eBufferType = BRAP_AF_P_BufferType_eFMMSlave;
		}
		else
		{
			/*
				BRAP_CIT_P_MasterSlaveType_eMasterPort  ->>>>> For output FMM buffer configuration 
				BRAP_CIT_P_MasterSlaveType_eOthers		->>>>> For input FMM buffer configuration 
			*/
			psIoGenericBuffer->eBufferType = psIoGenericBuffId->eBufferType;
		}


		psCircBuffer = &psIoGenericBuffer->sCircBuffer;
		ui32BuffId = psIoGenericBuffId->ui32BufferId[0];
		
		psCircBuffer->ui32ReadAddr	= ui32Buff0AddrStart + ui32BuffId * ui32Buff2BuffOffset;
		psCircBuffer->ui32WriteAddr = psCircBuffer->ui32ReadAddr + 4;
		psCircBuffer->ui32BaseAddr	= psCircBuffer->ui32ReadAddr + 8;
		psCircBuffer->ui32EndAddr	= psCircBuffer->ui32ReadAddr + 12;

		psCircBuffer->ui32WrapAddr = psCircBuffer->ui32EndAddr;
	}


	BDBG_LEAVE(BRAP_CITGEN_P_TransFmmBuffId2Addr);

	return BERR_SUCCESS;
}




/******************************************************************************
Summary:
	This function Translates the RDB buffer Id into the absolute addresses
	for the read, write wrap and end pointers 

Description:
	
	For RDB buffer, CIT generation module gets only Buffer Ids from PI,
	This function translates the RDB Ids to  RDB physical Address. This
	is applicable to both IOBuffers and IO Generic buffers. The Address derivation is
	based on the Base addess as BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0

Input:
		psIoBuffId			: IOBuffer cfg		
		psIoGenericBuffId	: IO Generic Buffer cfg				
		ui32TaskRdbOffset	:  Task RDb offset
		eMasterSlaveType	:  Master Slave Control

Output:
		psIoBuffer			: Final IOBuffer cfg
		psIoGenericBuffer	: Final IOGeneric Buffer cfg

Returns:
		None

******************************************************************************/

/*	This function Translates the RB buffer Id into the absolute addresses
	for the read, write wrap and end pointers */
static uint32_t BRAP_CITGEN_P_TransRdbBuffId2Addr(
			BRAP_CIT_P_sIO_BUFF_ID			*psIoBuffId,
			BRAP_AF_P_sIO_BUFFER			*psIoBuffer,
			BRAP_CIT_P_sIO_BUFF_ID			*psIoGenericBuffId,
			BRAP_AF_P_sIO_GENERIC_BUFFER	*psIoGenericBuffer,
			uint32_t						ui32TaskRdbOffset
		)
{
	uint32_t	ui32Buff;
	uint32_t	ui32BuffId;
	uint32_t	ui32Buff0AddrStart;
	uint32_t	ui32Buff2BuffOffset;
	

	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		*psCircBuffer;

	BDBG_ENTER(BRAP_CITGEN_P_TransRdbBuffId2Addr);

	ui32Buff0AddrStart	= ui32TaskRdbOffset 
							+ BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0;

	ui32Buff2BuffOffset = BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_1 
							- BCHP_AUD_DSP_CFG0_BASE_ADDR_FIFO_0;
	
	/*	First translate the I/O buffers */
	psIoBuffer->eBufferType		= psIoBuffId->eBufferType;
	psIoBuffer->ui32NumBuffers	= psIoBuffId->ui32NumBuffers;
	
	for( ui32Buff=0; ui32Buff<psIoBuffId->ui32NumBuffers; ui32Buff++)
	{
		psCircBuffer	= &psIoBuffer->sCircBuffer[ui32Buff];
		ui32BuffId		= psIoBuffId->ui32BufferId[ui32Buff];
		
		psCircBuffer->ui32BaseAddr	= ui32Buff0AddrStart + ui32BuffId * ui32Buff2BuffOffset;
		psCircBuffer->ui32EndAddr	= psCircBuffer->ui32BaseAddr + 4;
		psCircBuffer->ui32WriteAddr = psCircBuffer->ui32BaseAddr + 8;
		psCircBuffer->ui32ReadAddr	= psCircBuffer->ui32BaseAddr + 12;

		psCircBuffer->ui32WrapAddr	= psCircBuffer->ui32EndAddr;
	}

	/*	Then translate the I/O generic buffer */
	psIoGenericBuffer->ui32NumBuffers	= psIoGenericBuffId->ui32NumBuffers;
	psIoGenericBuffer->eBufferType		= psIoGenericBuffId->eBufferType;

	if( psIoGenericBuffId->ui32NumBuffers != 0 )
	{
		psCircBuffer	= &psIoGenericBuffer->sCircBuffer;
		ui32BuffId		= psIoGenericBuffId->ui32BufferId[0];
		
		psCircBuffer->ui32BaseAddr	= ui32Buff0AddrStart + ui32BuffId * ui32Buff2BuffOffset;
		psCircBuffer->ui32EndAddr	= psCircBuffer->ui32BaseAddr + 4;
		psCircBuffer->ui32WriteAddr = psCircBuffer->ui32BaseAddr + 8;
		psCircBuffer->ui32ReadAddr	= psCircBuffer->ui32BaseAddr + 12;

		psCircBuffer->ui32WrapAddr	= psCircBuffer->ui32EndAddr;
	}

	BDBG_LEAVE(BRAP_CITGEN_P_TransRdbBuffId2Addr);

	return BERR_SUCCESS;
}


/******************************************************************************
Summary:
	  This fuction populates the IoBuffer and IoGeneric Buffer configuration 
	  structures with dummy values...

Description:
	
	  The dummy confugration is required for SIPS at Mixer Task input	

Input:
		
Output:
		psIoBuffer			: Final IOBuffer cfg
		psIoGenericBuffer	: Final IOGeneric Buffer cfg

Returns:
		None
******************************************************************************/
static void BRAP_CITGEN_P_DummyIOBufferCfg(			
					BRAP_AF_P_sIO_BUFFER			*psIoBuffer,			
					BRAP_AF_P_sIO_GENERIC_BUFFER	*psIoGenericBuffer	
				)
{
	
	BDBG_ENTER(BRAP_CITGEN_P_DummyIOBufferCfg);

	/* Io Buffer Configuration */
	psIoBuffer->ui32NumBuffers			= 0;
	psIoBuffer->eBufferType				= BRAP_AF_P_BufferType_eInvalid;


	/* Io Generic Buffer Configuration */
	psIoGenericBuffer->ui32NumBuffers	= 0;
	psIoGenericBuffer->eBufferType		= BRAP_AF_P_BufferType_eInvalid;

	BDBG_LEAVE(BRAP_CITGEN_P_DummyIOBufferCfg);
}
/*******************************************************************************/

/*	This function analysis the CIT. The intention of this function is to
	analyse and print out all the parameters within the CIT structure 
	Going forward it is expected to verify all the addresses and sizes also
	generated in the CIT strucuture*/
void BRAP_P_AnalyseCit(	BMEM_Handle				hHeap,
					   BRAP_AF_P_sTASK_CONFIG	*psCit
					  ) 
{
	uint32_t	ui32NumNodes;
	uint32_t	ui32Node;
	uint32_t	ui32NumSrc, ui32NumDest;

	uint32_t	*pDest;
	void		*pDestTemp;

	BRAP_AF_P_sGLOBAL_TASK_CONFIG	*psGblTaskCfg;
	BRAP_AF_P_sNODE_CONFIG			*psNodeCfg;

#ifdef ANALYZE_IO_CFG
	BRAP_AF_P_sIO_BUFFER			sIoBuffer;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
#endif

	/*	First Step: Print all the informations within the CIT structure */
	BDBG_MSG(("Global Task Configuration parameters"));
	BDBG_MSG(("===================================="));

	psGblTaskCfg = &psCit->sGlobalTaskConfig;
	ui32NumNodes = psGblTaskCfg->ui32NumberOfNodesInTask;
	

	BDBG_MSG(("Time base for the Task:%s",GlobalTimeBase[psGblTaskCfg->eTimeBaseType]));
	BDBG_MSG((""));

	BDBG_MSG(("Number of Nodes in Task: %d ",ui32NumNodes));
	BDBG_MSG(("Number of Zero Fill Frames: 0x%x ",
		psGblTaskCfg->ui32NumberOfZeroFillSamples));
	BDBG_MSG(("Start Node index of Core Algo: %d ",
		psGblTaskCfg->ui32StartNodeIndexOfCoreAudioAlgorithm));

	/*
		Removing:
	ui32NumPorts = psGblTaskCfg->ui32NumOpPorts;
	BDBG_MSG(("Number of Output Ports in Task: %d ",
		psGblTaskCfg->ui32NumOpPorts));

	for(ui32Port=0; ui32Port<ui32NumPorts; ui32Port++)
	{
		BDBG_MSG(("Output Port Configuration for Port: %d ",ui32Port));
		BDBG_MSG(("TBD "));
	}
	*/

	/*	Dram Scratch buffer Address and Size */
	BDBG_MSG((""));
	BDBG_MSG(("DRAM Scratch Buffer Address: 0x%x ",
		psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress));
	BDBG_MSG(("DRAM Scratch Buffer Size Allocated: %d Bytes ",
		psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes));
	BDBG_MSG((""));

	/* DRAM port Configuration */
	BDBG_MSG(("Port Configuration address 0x%x  ",psGblTaskCfg->ui32FmmDestCfgAddr));

	/* DRAM Gate Open Configuration */
	BDBG_MSG(("Gate Open Configuration address 0x%x  ",psGblTaskCfg->ui32FmmGateOpenConfigAddr));
	BDBG_MSG((""));
	


	BDBG_MSG(("Node Configuration parameters "));
	BDBG_MSG(("============================= "));
	for(ui32Node=0; ui32Node<ui32NumNodes; ui32Node++)
	{
		psNodeCfg = &psCit->sNodeConfig[ui32Node];
		BDBG_MSG(("Node index: %d ",psNodeCfg->uiNodeId));
		BDBG_MSG(("Algo Id: %s ",AlgoIdEnum2Char[psNodeCfg->eAlgoId]));		

		BDBG_MSG(("\neCollectResidual : %s ",DisableEnable[psNodeCfg->eCollectResidual]));
		/*BDBG_ERR(("Algo Id: %x ",psNodeCfg->eAlgoId));*/

		/*	Code Address and Size */
		BDBG_MSG(("DRAM Code Buffer Address: 0x%x ",
			psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Code Buffer Size: %d Bytes",
			psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes));

		/*	Lookup Table Address and Size */
		BDBG_MSG(("DRAM Lookup Table Buffer Address: 0x%x ",
			psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Lookup Table Buffer Size: %d Bytes",
			psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes));

		/*	Inter-frame buffer Address check */
		if( psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Not present for this Node"));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Address: 0x%x ",
				psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: %d Bytes",
				psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes));
		}

		/*	Node Status buffer Address check */
		if( psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Node Status buffer Not present for this Node"));
			BDBG_MSG(("DRAM Node Status buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Node Status buffer Address: 0x%x ",
				psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Node Status buffer Size: %d Bytes",
				psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes));
		}

		/*	User config buffer Address check */
		if( psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM User Config Buffer not present for this Node"));
			BDBG_MSG(("DRAM User Config Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM User Config Buffer Address: 0x%x ",
				psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM User Config Buffer Size: %d Bytes",
				psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes));
		}

		/*	Input buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Input Configuration Details:", ui32Node));
		BDBG_MSG(("Num Source feeding data to this node: %d", psNodeCfg->ui32NumSrc));
		for( ui32NumSrc=0; ui32NumSrc<psNodeCfg->ui32NumSrc; ui32NumSrc++)
		{
			BDBG_MSG((""));

			BDBG_MSG(("Source %d Input Buffer is [%s]",ui32NumSrc, PortValidType[psNodeCfg->eNodeIpValidFlag[ui32NumSrc]]));

			BDBG_MSG(("Source %d Input Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc]));			

#ifdef ANALYZE_IO_CFG
			/*BDBG_MSG(("Input buffer Configuration:"));*/

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if((sIoBuffer.ui32NumBuffers >0) && (BRAP_AF_P_eValid == psNodeCfg->eNodeIpValidFlag[ui32NumSrc]) )
			{
				BDBG_MSG(("Source %d Input Buffer Type: %s",
				ui32NumSrc, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			/*	Analyze Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif
			BDBG_MSG(("Source %d Input Generic Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc]));

#ifdef ANALYZE_IO_CFG
			
			/* BDBG_MSG(("Input Generic buffer Configuration:")); */

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO Generic buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));

			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Source %d Input Generic Buffer Type: %s",
				ui32NumSrc, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			 /* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

			
		}

		/*	Output buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Output Configuration Details:", ui32Node));
		BDBG_MSG(("Num Destination getting data from this node: %d", 
			psNodeCfg->ui32NumDst));
		for( ui32NumDest=0; ui32NumDest<psNodeCfg->ui32NumDst; ui32NumDest++)
		{

			/* IO BUFFER CONFIGURATION */
			/*-------------------------*/

			/*Printing Output Buffer Cfg Structure Address */
			BDBG_MSG(("Destination %d Output Buffer Cfg Structure Address: 0x%x", 
				ui32NumDest, psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest]));			
			
#ifdef ANALYZE_IO_CFG
			/* BDBG_MSG(("Output buffer Configuration:"));*/


			/* Getting contents of the Destination IO buffer */
			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;					
			
			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			
			/*	Print Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif


			/* IOGENERIC BUFFER CONFIGURATION */
			/*--------------------------------*/

			BDBG_MSG(("Destination %d Output Generic Buffer Cfg Structure Address: 0x%x",
				ui32NumDest, psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest]));


#ifdef ANALYZE_IO_CFG

			/*	Getting contents of the IO Generic buffer */

			/*Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));


			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Generic Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING						

			BDBG_MSG(("Destination %d Datatype : %s",
				ui32NumDest, PortDatatType[psNodeCfg->eNodeOpBuffDataType[ui32NumDest]]));	

			BDBG_MSG((""));
#endif
		}		
		BDBG_MSG(("============================= "));
	}
}


#ifdef ANALYZE_IO_CFG
/*	This function reads the data from the Dram Address. Currently this 
	function has been abstracted to do a data copy only for MSVC. This can be
	extended for any other platform when required */
static void BRAP_CITGEN_P_ReadFromDram(	uint32_t	ui32SrcAddr,
					uint32_t	ui32DestAddr,
					uint32_t	ui32Size )
{
	uint32_t	i;
	uint32_t	*pSrc = (uint32_t *)ui32SrcAddr;
	uint32_t	*pDest = (uint32_t *)ui32DestAddr;

	for( i=0; i<(ui32Size >> 2); i++ )
	{
		*pDest++ = *pSrc++;
	}
}


#ifdef ANALYSIS_IO_GEN_ENABLE
/*	This function Analyzes the I/O buff structure */
static void BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(BRAP_AF_P_sIO_BUFFER *psIoBuffStruct)
{
	uint32_t	ui32Buff;
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuff;

	/*	Need to put characters instead of directly printing value */
	BDBG_ERR(("Number of buffers: %d", psIoBuffStruct->ui32NumBuffers));	
	BDBG_ERR(("Buffer Type: %s", BuffTypeEnum2Char[psIoBuffStruct->eBufferType]));

	for( ui32Buff=0; ui32Buff<psIoBuffStruct->ui32NumBuffers; ui32Buff++)
	{
		psCircBuff = &psIoBuffStruct->sCircBuffer[ui32Buff];

		if( ( psIoBuffStruct->eBufferType == BRAP_AF_P_BufferType_eRDB ) ||
			( psIoBuffStruct->eBufferType == BRAP_AF_P_BufferType_eFMM ) ||
			( psIoBuffStruct->eBufferType == BRAP_AF_P_BufferType_eRAVE ) )
		{
			/*	The circular buffer will contain address of RDB registers 
				and not the actual DRAM addresses */
			BDBG_ERR(("Buffer %d Address of the Fifo Base Register in RDB: 0x%x ", 
				ui32Buff,psCircBuff->ui32BaseAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo End Register in RDB: 0x%x ", 
				ui32Buff,psCircBuff->ui32EndAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Write Pointer Register in RDB: 0x%x ", 
				ui32Buff,psCircBuff->ui32WriteAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Read Pointer Register in RDB: 0x%x ", 
				ui32Buff,psCircBuff->ui32ReadAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Wrap Pointer Register in RDB: 0x%x ", 
				ui32Buff,psCircBuff->ui32WrapAddr));
		}
		else if ( psIoBuffStruct->eBufferType == BRAP_AF_P_BufferType_eDRAM_IS )
		{
			/*	The circular buffer the actual DRAM addresses as the buffer is Inter-stage */
			BDBG_ERR(("Buffer %d Fifo DRAM Base Address: 0x%x ", 
				ui32Buff, psCircBuff->ui32BaseAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM End Address: 0x%x ", 
				ui32Buff,psCircBuff->ui32EndAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Write Pointer Address: 0x%x ", 
				ui32Buff,psCircBuff->ui32WriteAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Read Pointer Address: 0x%x ", 
				ui32Buff,psCircBuff->ui32ReadAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Wrap Pointer Address: 0x%x ", 
				ui32Buff,psCircBuff->ui32WrapAddr));
		}
		else
		{
			/*	This is a DRAM buffer */
			/*	WARNING!!! TBD */
		}
	}
}

/*	This function Analyzes the I/O Generic buffer structure */
static void BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(BRAP_AF_P_sIO_GENERIC_BUFFER *psIoGenericBuffStruct)
{
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER *psCircBuff;

	BDBG_ERR(("Number of buffers: %d", psIoGenericBuffStruct->ui32NumBuffers));	
	/*	Need to put characters instead of directly printing value */
	if( psIoGenericBuffStruct->ui32NumBuffers != 0 )
	{
		BDBG_ERR(("Buffer Type: %s", BuffTypeEnum2Char[psIoGenericBuffStruct->eBufferType]));

		psCircBuff = &psIoGenericBuffStruct->sCircBuffer;

		if( ( psIoGenericBuffStruct->eBufferType == BRAP_AF_P_BufferType_eRDB ) ||
			( psIoGenericBuffStruct->eBufferType == BRAP_AF_P_BufferType_eFMM ) ||
			( psIoGenericBuffStruct->eBufferType == BRAP_AF_P_BufferType_eRAVE ) )
		{
			/*	The circular buffer will contain address of RDB registers 
				and not the actual DRAM addresses */
			BDBG_ERR(("Buffer %d Address of the Fifo Base Register in RDB: 0x%x ", 
				0,psCircBuff->ui32BaseAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo End Register in RDB: 0x%x ", 
				0,psCircBuff->ui32EndAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Write Pointer Register in RDB: 0x%x ", 
				0,psCircBuff->ui32WriteAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Read Pointer Register in RDB: 0x%x ", 
				0,psCircBuff->ui32ReadAddr));
			BDBG_ERR(("Buffer %d Address of the Fifo Wrap Pointer Register in RDB: 0x%x ", 
				0,psCircBuff->ui32WrapAddr));
		}
		else if ( psIoGenericBuffStruct->eBufferType == BRAP_AF_P_BufferType_eDRAM_IS )
		{
			/*	The circular buffer the actual DRAM addresses as the buffer is Inter-stage */
			BDBG_ERR(("Buffer %d Fifo DRAM Base Address: 0x%x ", 
				0, psCircBuff->ui32BaseAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM End Address: 0x%x ", 
				0,psCircBuff->ui32EndAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Write Pointer Address: 0x%x ", 
				0,psCircBuff->ui32WriteAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Read Pointer Address: 0x%x ", 
				0,psCircBuff->ui32ReadAddr));
			BDBG_ERR(("Buffer %d Fifo DRAM Wrap Pointer Address: 0x%x ", 
				0,psCircBuff->ui32WrapAddr));
		}
		else
		{
			/*	This is a DRAM buffer */
			/*	WARNING!!! TBD */
		}
	}
}
#endif /* #ifdef ANALYSIS_IO_GEN_ENABLE */

#endif


/******************************************************************************
Summary:
		Compute the Buffer Size of FW-Status 

Description:

	This function computes the sizes required for the FW Status buffers	
	required for the entire task.

	It returns the sizes of the buffers required for the entire 
	task 

	Status buffer can be shared. If a node shares the status buffer from the 
	preceeding node, then the buffer size should be mentioned as zero and 
	to be configured as shared in brap_af_priv.c.

	It is also mandated that the nodes that do not require Status buffer should
	keep the buffer size to zero and keep the flag to BRAP_AF_P_FwStatus_eAbsent.
	

	Ideally, every node in a stage can have status buffer allocated. Because
	of FW specifics, we limit it to the first algorithm node in a stage and 
	all the other nodes come after it should share the same status buffer.

	I cannot explain stuffs more than this..If anybody who did not understand this 
	can go to next function. 

Input:
		psAlgoNwInfo	: Algo Node network 

Output:
		psTaskBuffInfo : Buffer information for a task 

Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
					BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
					BRAP_CIT_P_sTaskBuffInfo		*psTaskBuffInfo
				)
{

	
	uint32_t			ui32Node;
	uint32_t			ui32TaskFwStatusMemSize;
	uint32_t			ui32NodeFwStatusMemSize;
	
	uint32_t			ui32BranchNum;
	uint32_t			ui32AlgoStg;
	uint32_t			ui32NumAlgoStgInBranch;
	uint32_t			ui32NumNodesInAlgo;
	BRAP_AF_P_FwStatus	eFwStatusBuffType;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_CIT_P_sNodeAttr		*psNodeAttr;


	BDBG_ENTER(BRAP_CITGEN_P_ComputeFwStatusBuffSizes);

	ui32TaskFwStatusMemSize =0;


	for(ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;

			for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];


				eFwStatusBuffType		= psNodeAttr->eFwStatusBuffType;
				ui32NodeFwStatusMemSize = psNodeAttr->ui32FwStatusBuffSize;	

				if( eFwStatusBuffType == BRAP_AF_P_FwStatus_eShared &&
					ui32NodeFwStatusMemSize != 0 )
				{
					BDBG_ERR(("ERROR Firmware status buffer in Branch %d Stage %d Node %d has been defined as SHARED but the size specified is NON-ZERO",ui32BranchNum,ui32AlgoStg,ui32Node));
					BDBG_LEAVE(BRAP_CITGEN_P_ComputeFwStatusBuffSizes);
					return BERR_INVALID_PARAMETER ;
				}			

				ui32TaskFwStatusMemSize += ui32NodeFwStatusMemSize;
			}
		}
	}
	
	psTaskBuffInfo->ui32TaskFwStatusBuffMemSize = ui32TaskFwStatusMemSize;
	
	BDBG_LEAVE(BRAP_CITGEN_P_ComputeFwStatusBuffSizes);
	
	return BERR_SUCCESS;
}


/******************************************************************************
Summary:
		Allocate FW-Status Buffer for all nodes
Description:

	This function allocates Firmware-status buffers	for each node

  	For FwStatus Buffer, if the buffer size specified is zero then it explicitly 
	signals that the buffer is either not present or shared.

	For a node, if the FwStatus Buffer is shared, then it hsould not be the 
	first node and the buffer size should be zero.

Input:
		ui32FwStatusBuffBaseAddr : Base address for  Firmware-status buffer

Output:
		psAlgoNwInfo : Algo Node network 

Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_AllocFwStatusBuff(
					uint32_t				ui32FwStatusBuffBaseAddr,					
					BRAP_CIT_P_AlgoNwInfo	*psAlgoNwInfo 
				)
{
	uint32_t	ui32Node;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32FwStatusBuffAddr;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInAlgo;

	BRAP_AF_P_FwStatus				eFwStatusBuffType;


	BRAP_CIT_P_StageInfo			*psStgInfo;
	BRAP_CIT_P_sNodeAttr			*psNodeAttr, *psPrevNodeAttr;

	BDBG_ENTER(BRAP_CITGEN_P_AllocFwStatusBuff);


	ui32FwStatusBuffAddr = ui32FwStatusBuffBaseAddr;
	
	
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];

			ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;	

			/*	Then allocate Status buffers */
			for ( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];

				eFwStatusBuffType = psNodeAttr->eFwStatusBuffType;

				if( psNodeAttr->ui32FwStatusBuffSize == 0 )
				{
					if( eFwStatusBuffType != BRAP_AF_P_FwStatus_eShared )
					{	
						psNodeAttr->ui32FwStatusBuffDramAddress = 
											BRAP_AF_P_DRAM_ADDR_INVALID;
					}
					else if( ui32Node != 0 )
					{
						psPrevNodeAttr = &psStgInfo->sNodeAttr[ui32Node-1];

						/*Copying the Dram Status buffer address and size to all the successive nodes*/
						psNodeAttr->ui32FwStatusBuffDramAddress = 
							psPrevNodeAttr->ui32FwStatusBuffDramAddress;

						psNodeAttr->ui32FwStatusBuffSize = 
							psPrevNodeAttr->ui32FwStatusBuffSize;
					}
					else
					{
						/*	Error Case: First node is having shared flag set */
						BDBG_ERR(("ERROR : Firmware Status Buffer Allcation. First node is having shared flag set"));
						BDBG_LEAVE(BRAP_CITGEN_P_AllocFwStatusBuff);
						return BERR_INVALID_PARAMETER;						
					}
				}
				else
				{
					psNodeAttr->ui32FwStatusBuffDramAddress = ui32FwStatusBuffAddr;
					ui32FwStatusBuffAddr += psNodeAttr->ui32FwStatusBuffSize;
				}
			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_AllocFwStatusBuff);
	return BERR_SUCCESS;
}


/******************************************************************************
Summary:

	Compute the Buffer Size of a Task's stack

Description: 
	
	Every Task will be assiciated with a DRAM stack. This is to enable stack swap 
	in DSP.

	Allocated stack size per task is BRAP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES 
	bytes.

Input:
		None
Output:
		psTaskBuffInfo : Buffer information for a task 
Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_ComputeTaskStackBuffSize(					
					BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo )

{
	BDBG_ENTER(BRAP_CITGEN_P_ComputeTaskStackBuffSize);

	psTaskBuffInfo->ui32TaskStackMemSize
						= BRAP_CIT_P_TASK_SWAP_BUFFER_SIZE_INBYTES;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeTaskStackBuffSize);

}

/******************************************************************************
Summary:

	Compute the size of port configuration structure and the SPDIF 
		User Configuration buffer.

Description: 

	Compute the Buffer Size 

	  1) Port configuration
 	  2) SPDIF user Configuration  

  Port Configuration:
		The buffer is allocated = (Number of PLLs)  * (Sizeof(One PLL Cfg))

  SPDIF user Configuration
		The buffer is allocated = (Number of SPDIFs) * (Sizeof(One SPDIFs user Cfg))			

Input:
		None
Output:
		psTaskBuffInfo : Buffer information for a task 

Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize(
					BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo
				)
{

	BDBG_ENTER(BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize);

	/*Port Configuration */
	psTaskBuffInfo->ui32TaskPortConfigMemSize 
							= BRAP_CIT_P_TASK_PORT_CONFIG_MEM_SIZE;														
																	
	psTaskBuffInfo->ui32TaskSPDIFConfigMemSize 
							= BRAP_CIT_P_TASK_SPDIF_USER_CFG_MEM_SIZE;
	

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize);	
}

/******************************************************************************
Summary:

	Allocating user Configuration Buffers for SPDIF ports

Description: 

	Allocates user configuration buffer for every SPDIF ports.
	CIT generation module allocates this memory and inform the PI to fill up the
	content
  
Input:
	ui32TaskAllSPDIF_PortConfigAddr : Base address of SPDIF port configuration.

Output:
	pui32TaskSPDIFPortConfigAddr : Base address of each SPDIF port configuration.

Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_AllocSpdifPortCfgBuff(
							uint32_t				ui32TaskAllSPDIF_PortConfigAddr,							
							BRAP_AF_P_sDRAM_BUFFER	*psTaskSPDIFPortConfigAddr
						)
{
	uint32_t ui32Count;
	uint32_t ui32PortConfigAddr;


	BDBG_ENTER(BRAP_CITGEN_P_AllocSpdifPortCfgBuff);


	ui32PortConfigAddr = ui32TaskAllSPDIF_PortConfigAddr;

	for(ui32Count =0 ; ui32Count<BRAP_AF_P_MAX_NUM_SPDIF_PORTS;ui32Count++)
	{
		psTaskSPDIFPortConfigAddr[ui32Count].ui32DramBufferAddress	= ui32PortConfigAddr;

		psTaskSPDIFPortConfigAddr[ui32Count].ui32BufferSizeInBytes	
							= ( ( ( (SIZEOF(BRAP_AF_P_sSPDIF_USER_CFG)+20) +3 )>>2)<<2);

		/* Putting  extra buffer and 32 bit allignment*/
		ui32PortConfigAddr += ( ( ( (SIZEOF(BRAP_AF_P_sSPDIF_USER_CFG)+20) +3 )>>2)<<2);
	}


	BDBG_LEAVE(BRAP_CITGEN_P_AllocSpdifPortCfgBuff);

	return BERR_SUCCESS;
}


/******************************************************************************
Summary:	
		Calculate the size of Gate Open Configuration buffer

Description: 	
	 
	   This is the buffer required allocated in DRAM to facilitate gate open and
	   Independent delay
		  
Input:	None
	
Output:
		psTaskBuffInfo		
		
Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize(
						BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo
					)
{
	BDBG_ENTER(BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize);

	psTaskBuffInfo->ui32TaskGateOpenConfigMemSize = 
							BRAP_CIT_P_TASK_FMM_GATE_OPEN_CONFIG;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize);
}


/******************************************************************************
Summary:	
		Calculate the size of Firmware Hardware cfg buffer
		This is to include any FW-HW interface which cannot be coupled with 
		any other structures.... 

Description: 	

		This is the buffer required allocated in DRAM to facilitate any FW-HW
		Interface.. Currently PPM cfg registers are added here..
		Just sky opened for other future configurations.
					
Input:	None
	
Output:
		psTaskBuffInfo				
Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_ComputeHwFwCfgBufferSize(
						BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo
					)
{

	BDBG_ENTER(BRAP_CITGEN_P_ComputeHwFwCfgBufferSize);

	psTaskBuffInfo->ui32TaskHwFwCfgMemSize = 
							BRAP_CIT_P_TASK_HW_FW_CONFIG;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeHwFwCfgBufferSize);
}


/******************************************************************************
Summary:
	
	  Calculate the size of SamplingFrequency Map LUT Size

Description: 	
	
	  This is to allocated a LUT in DRAM that maps input stream sampling 
	  frequency to the desired CHIP output sampling frequency.

	  Memory allocateed is BRAP_CIT_P_TASK_FS_MAPPING_LUT_SIZE in bytes
	
Input:	None
	
Output:
		psTaskBuffInfo			
		
Returns:
		None

******************************************************************************/

static void BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize(
						BRAP_CIT_P_sTaskBuffInfo	*psTaskBuffInfo
					)
{
	BDBG_ENTER(BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize);

	psTaskBuffInfo->ui32SamplingFrequencyMapLutSize = 
							BRAP_CIT_P_TASK_FS_MAPPING_LUT_SIZE;

	BDBG_LEAVE(BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize);
}


/******************************************************************************
Summary:
	
	  Populate Gate Open Configuration

Description: 	
	
	  
	A distinct port can have multiple FMM outputs when the Independent delay is enabled.
							   In this case : 1) There will be differenct delays associated with each FMM ports and 
											  2) Gate open needed for each FMM output
											  3) The FMM port content for initial zero filling / pause burst filling in FW

								Disadvantage: When the independent delay is enabled, zero filling in DSP will happen for all the slaves also.
											  Not a functional problem.
	
	The FMM port content for initial zero filling / pause burst filling in FW

    Independent delay and Gate open is combined

Input:
	psIoBuffId				 : Buffer information
	psIoBuffer				 : 
	ui32IoPortCfgStructAddr	 : Cfg Address
	ui32IndepDelay			 : Independent delay value
	eDistinctOpType			 : Port content type :
	eFmmDstFsRate			 : FMM sink rate

Output:
	psGateOpenCfg			 : Gate Open Configuration
	
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_PopulateGateOpenCfg(
								BRAP_CIT_P_sIO_BUFF_ID			*psIoBuffId,
								BRAP_AF_P_sIO_BUFFER			*psIoBuffer,
								BRAP_AF_P_sFMM_GATE_OPEN_CONFIG	*psGateOpenCfg,
								uint32_t						ui32IoPortCfgStructAddr,
								uint32_t						ui32IndepDelay,
								BRAP_AF_P_DistinctOpType        eDistinctOpType,
								BRAP_AF_P_FmmDstFsRate			eFmmDstFsRate
							)
{
	uint32_t							ui32Buff;
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		*psCircBuffer;


	BDBG_ENTER(BRAP_CITGEN_P_PopulateGateOpenCfg);

	for( ui32Buff=0; ui32Buff<psIoBuffId->ui32NumBuffers; ui32Buff++)
	{
		psCircBuffer = &psIoBuffer->sCircBuffer[ui32Buff];

		psGateOpenCfg->uin32RingBufStartWrPointAddr[ui32Buff]
				 	 = psCircBuffer->ui32ReadAddr + 20;
	}

	psGateOpenCfg->uin32DramIoConfigAddr = ui32IoPortCfgStructAddr;

	psGateOpenCfg->ui32IndepDelay		 = ui32IndepDelay;

	
	/* FMM Content Type for Zero/Pause burst filling */
	psGateOpenCfg->eFMMContentType		 = (eDistinctOpType == BRAP_AF_P_DistinctOpType_eCompressed) ? \
													BRAP_AF_P_FmmContentType_eCompressed : BRAP_AF_P_FmmContentType_ePcm;		

	/* FMM sinking rate */
	psGateOpenCfg->eFmmDstFsRate		 = eFmmDstFsRate;

	BDBG_LEAVE(BRAP_CITGEN_P_PopulateGateOpenCfg);

}

/******************************************************************************
Summary:
	
	  Write Gate Open Configuration to DRAM

Description: 	
			
Input:
	psFmmGateOpenConfig	
	ui32TaskGateOpenConfigAddr	 : DRAM Cfg Address
	ui32Size

Output:
		None		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_WriteGateOpenCfgToDram(
			BMEM_Handle								hHeap,
			BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG	*psTaskFmmGateOpenConfig,
			uint32_t								ui32TaskGateOpenConfigAddr,
			uint32_t								ui32Size
		)

{

	BDBG_ENTER(BRAP_CITGEN_P_WriteGateOpenCfgToDram);

	BRAP_CITGEN_P_WriteToDRAM(	hHeap,
								(uint32_t)psTaskFmmGateOpenConfig,
								ui32TaskGateOpenConfigAddr,	
								ui32Size
							 );

	BDBG_LEAVE(BRAP_CITGEN_P_WriteGateOpenCfgToDram);

}

/******************************************************************************
Summary:
	
	  Finding the zero fill samples for the algorithms

Description: 	
			
Input:
		psAlgoNwInfo		

Output:
		pui32ZeroFillSamples		
Returns:
		None

******************************************************************************/

static uint32_t  BRAP_CITGEN_P_GetNumZeroFillSamples(
							uint32_t					*pui32ZeroFillSamples,
							BRAP_CIT_P_AlgoNwInfo		*psAlgoNwInfo
					 )
{

	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32AudioAlgorithm;


	BRAP_CIT_P_StageInfo		*psStgInfo;

	BDBG_ENTER(BRAP_CITGEN_P_GetNumZeroFillSamples);

	/*Intialization*/
	*pui32ZeroFillSamples = 0;
	
	/* Checking only the first branch */
	ui32BranchNum=0;
	ui32AudioAlgorithm = BRAP_CIT_P_ERROR; /*To avoid warning */

	ui32NumAlgoStgInBranch = 
		psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

	for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch ; ui32AlgoStg++ )
	{
		psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
		
		/*	First lets get the algorithm type running on this stage */
		switch ( psStgInfo->eAudioAlgoType )
		{
			
			case BRAP_CIT_P_AudioAlgoType_eDecode:
				ui32AudioAlgorithm = psStgInfo->uAlgorithm.eDecAudioAlgo;				
				break;

			case BRAP_CIT_P_AudioAlgoType_eEncode:
				ui32AudioAlgorithm = psStgInfo->uAlgorithm.eEncAudioAlgo;				
				break;

			case BRAP_CIT_P_AudioAlgoType_ePostProc:
				ui32AudioAlgorithm = psStgInfo->uAlgorithm.eProcAudioAlgo;				
				break;

			case BRAP_CIT_P_VideoAlgoType_eDecode:
				ui32AudioAlgorithm = psStgInfo->uAlgorithm.eVideoAlgo;				
				break;
			
			default:
				
				ui32AudioAlgorithm = BRAP_CIT_P_ERROR;
				/*	Error Case. It cannot come here */
				/*	WARNING!! Add some error recovery here */
				
				break;
		}

		/* Iterate till you hit the first Decode/Encode stage */
		if( (((uint32_t)psStgInfo->eAudioAlgoType) == BRAP_CIT_P_AudioAlgoType_eDecode) ||
			(((uint32_t)psStgInfo->eAudioAlgoType) == BRAP_CIT_P_AudioAlgoType_eEncode) ||
			(((uint32_t)psStgInfo->eAudioAlgoType) == BRAP_CIT_P_GfxAlgoType_eDecode)   ||
			(((uint32_t)psStgInfo->eAudioAlgoType) == BRAP_CIT_P_VideoAlgoType_eDecode)
		  )		
		{
			break;
		}
		else
		{
			/* Go to the next Node */
		}
	}

	if(ui32AudioAlgorithm == BRAP_CIT_P_ERROR)
	{
		*pui32ZeroFillSamples = 0;
		BDBG_ERR(("Unable to find the Decoder/Encoder stage in the 1st branch "));
		BDBG_LEAVE(BRAP_CITGEN_P_GetNumZeroFillSamples);
		return BERR_UNKNOWN;
	}

	switch(ui32AudioAlgorithm)
	{

		case BRAP_DSPCHN_AudioType_eMpeg :	

				*pui32ZeroFillSamples = 13824;

				break;

		case BRAP_DSPCHN_AudioType_eAc3:	

				*pui32ZeroFillSamples = 18432;

				break;

		default:
				*pui32ZeroFillSamples = 0;

				break;

	}

	BDBG_LEAVE(BRAP_CITGEN_P_GetNumZeroFillSamples);

	return BERR_SUCCESS;

}
						
						
/******************************************************************************
Summary:
	Initialize the  Cit global Datastructue

Description: 

	The CIT global data structure is already initialized with zero at the 
	time of memory allocation.

	Any specific intitialization no matter what exactly the value is done here.
	Using enums and defines adds more readability 
	
Input:	

		psCitDataStructure	
	
Output:	
		psCitDataStructure	
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_InitializeCitDatastructue(											
						BRAP_CIT_P_sCitDataStructure *psCitDataStructure
					)
{

	/* Special Algorithm Detection Flags */
	psCitDataStructure->sAlgoModePresent.ui32DDP_PassThruPresent 
										= BRAP_CIT_P_ABSENT;

	psCitDataStructure->sAlgoModePresent.ui32DTS_EncoderPresent
										= BRAP_CIT_P_ABSENT;

	psCitDataStructure->sAlgoModePresent.ui32AC3_EncoderPresent
										= BRAP_CIT_P_ABSENT;

	psCitDataStructure->sAlgoModePresent.ui32BTSC_EncoderPresent
										= BRAP_CIT_P_ABSENT;	


	psCitDataStructure->sAlgoModePresent.ui32DolbyPulsePresent
										= BRAP_CIT_P_ABSENT;

	psCitDataStructure->sAlgoModePresent.ui32DDREPresent
										= BRAP_CIT_P_ABSENT;

	/* Special Task Detection Flags */

	psCitDataStructure->sSpecialTaskPresent.eMixerTaskDetectionFlag
										= BRAP_CIT_P_MixerTaskDetection_eNotDetected;	

}
/******************************************************************************
Summary:
	Get Task RDB Configuration 

Description: 

	RDB  Configuration 
			1) Get the RDB Cfg from the CIT input structure and place in global 
			   data structure.

	This is done to decouple the CIT input structure from  direct access by
	any modules in CIT generation .

	This structure is fundamental unit in CIT generation module and needed to be
	accessed in different modules.

Input:	
	psCitIp	 : Cit Input
	
Output:

	psRdbCfg : Task RDB Cfg
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_GetCitInputRdbCfg(
							 BRAP_CIT_P_InputInfo		*psCitIp,
							 BRAP_CIT_P_RdbConfig		*psRdbCfg
						 )
{
	BDBG_ENTER(BRAP_CITGEN_P_GetCitInputRdbCfg);

	/* Get the input RDB Cfg */
	psRdbCfg->sRdbCfg = psCitIp->sRdbCfg;	

	BDBG_LEAVE(BRAP_CITGEN_P_GetCitInputRdbCfg);
}


/******************************************************************************
Summary:
	 Task RDB Configuration 

Description: 
	
	  This configures Task RDB offset derived from CIT input.
	  We use ui32TaskRdbOffset for all RDB related stuffs.
			
Input:	

	psRdbCfg			 : RDB Configuration
	
Output:

	ui32TaskRdbOffset	 : Task RDB offset
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_PrepareTaskRdbCfg(
							 BRAP_CIT_P_RdbConfig	*psRdbCfg
						 )
{
	BDBG_ENTER(BRAP_CITGEN_P_PrepareTaskRdbCfg);
	
	/* Generate the present chip RDB Cfg */
	psRdbCfg->ui32TaskRdbOffset = psRdbCfg->sRdbCfg.ui32RdbBaseOffset + 
								  psRdbCfg->sRdbCfg.ui32RdbDspOffset;


	BDBG_LEAVE(BRAP_CITGEN_P_PrepareTaskRdbCfg);

}

/******************************************************************************
Summary:
	Task output port Configuration Debug messages

Description: 
			
Input:
	psCitIp	 : Cit Input
Output:
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_PIInputPortCfgDBG(BRAP_CIT_P_InputInfo	*psCitIp)
{

	uint32_t ui32Count;	
	BRAP_CIT_P_FmmDstSelect eFmmDstSelect;


	BDBG_ENTER(BRAP_CITGEN_P_PIInputPortCfgDBG);


	BDBG_MSG((""));
	BDBG_MSG((""));
	BDBG_MSG(("============================= "));
	BDBG_MSG(("==PORT CONFIGURATION INFO === "));
	BDBG_MSG(("============================= "));


	BDBG_MSG(("The Number of Ports available %d ", psCitIp->ui32NumOutputs));
	BDBG_MSG((""));

	BDBG_MSG((""));

	/*TIGI CHANGE*/
	for(ui32Count =0 ;ui32Count<psCitIp->ui32NumOutputs;ui32Count++)
	{
		eFmmDstSelect = psCitIp->sPortCfg[ui32Count].sFmmDstDetails.eFmmDstSelect;

		if(BRAP_CIT_P_FmmDstSelect_eOpPort == eFmmDstSelect)
		{
			BDBG_MSG(("Port No. %d",ui32Count));
			BDBG_MSG(("\t Port Name: %s ",PortName[(uint32_t)(psCitIp->sPortCfg[ui32Count].sFmmDstDetails.uFmmDstPortType.eOpType)]));

			if(psCitIp->sPortCfg[ui32Count].ePllNcoSelect == BRAP_AF_P_PllSelect)
			{
				BDBG_MSG(("\t PLL Index %d ",psCitIp->sPortCfg[ui32Count].ui32PllIndex));
			}
			else if(psCitIp->sPortCfg[ui32Count].ePllNcoSelect == BRAP_AF_P_NcoSelect)
			{
				BDBG_MSG(("\t NCO associated port "));
			}
			else
			{
				BDBG_MSG(("\t PLL/NCO Association: NONE : Default PLL index assigned by PI =%d  ",psCitIp->sPortCfg[ui32Count].ui32PllIndex));
			}

			BDBG_MSG(("\t The Frequency Rate is [%s]",FrequencyRate[psCitIp->sPortCfg[ui32Count].eFmmDstFsRate]));			

			
			BDBG_MSG((""));

			if((psCitIp->sPortCfg[ui32Count].sFmmDstDetails.uFmmDstPortType.eOpType == BRAP_OutputPort_eSpdif ) ||
				(psCitIp->sPortCfg[ui32Count].sFmmDstDetails.uFmmDstPortType.eOpType == BRAP_OutputPort_eSpdif1)
				)
			{
				
				BDBG_MSG(("\t Spdif Content is [%s]. Spdif Modes : [%s] : [%s] ",Spdif_Content[(psCitIp->sPortCfg[ui32Count].uOpCfg.sSpdifCfg.bPcmOnSpdif)], Spdif_HbrMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sSpdifCfg.bHbr)],Spdif_HWCbitsMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sSpdifCfg.bHwCbits)]));				
				
			}
			if(psCitIp->sPortCfg[ui32Count].sFmmDstDetails.uFmmDstPortType.eOpType == BRAP_OutputPort_eMai)
			{	
				
				BDBG_MSG((""));
				BDBG_MSG(("\t The Mai port Mux Selector = %s ",PortName[psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.eMaiMuxSelector]));

				if(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.eMaiMuxSelector == BRAP_OutputPort_eMai)
				{		
					
					/*
					if(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bPcmOnSpdif)
					{
						BDBG_MSG((" Payload is compressed "));
					}
					else
					{
						BDBG_MSG((" Payload is PCM "));
					}		


					if(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bHbr)
					{
						BDBG_MSG((" HBR Mode is enabled "));
					}
					else
					{
						BDBG_MSG((" HBR Mode is Disabled "));
					}
					*/


					BDBG_MSG(("\t Payload is [%s]. Modes : [%s] : [%s] ",Mai_Payload[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bPcmOnSpdif)], Spdif_HbrMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bHbr)],Spdif_HWCbitsMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bHwCbits)]));
					
				}

				if(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.eMaiMuxSelector == BRAP_OutputPort_eSpdif)
				{		

					/*
					if(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bPcmOnSpdif)
					{
						BDBG_MSG((" Payload is compressed "));
					}
					else
					{
						BDBG_MSG((" Payload is PCM "));
					}
					*/

					BDBG_MSG(("\t Payload is [%s]. Modes : [%s] : [%s] ",Mai_Payload[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bPcmOnSpdif)], Spdif_HbrMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bHbr)],Spdif_HWCbitsMode[(psCitIp->sPortCfg[ui32Count].uOpCfg.sMaiCfg.sMaiContent.bHwCbits)]));
					
				}
			}
			BDBG_MSG((""));
		}
		else if(BRAP_CIT_P_FmmDstSelect_eCapturePort == eFmmDstSelect) 
		{			
			BDBG_MSG(("Port No. %d",ui32Count));		

			BDBG_MSG(("\t Port Name: %s ",CapPortName[(uint32_t)(psCitIp->sPortCfg[ui32Count].sFmmDstDetails.uFmmDstPortType.eCapPortType)]));

			BDBG_MSG(("\t The Frequency Rate is [%s]",FrequencyRate[psCitIp->sPortCfg[ui32Count].eFmmDstFsRate]));			

			if(psCitIp->sPortCfg[ui32Count].ePllNcoSelect == BRAP_AF_P_PllSelect)
			{
				BDBG_MSG(("\t PLL Index %d ",psCitIp->sPortCfg[ui32Count].ui32PllIndex));
			}
			else if(psCitIp->sPortCfg[ui32Count].ePllNcoSelect == BRAP_AF_P_NcoSelect)
			{
				BDBG_MSG(("\t NCO associated port "));
			}
			

			BDBG_MSG((""));
		}

	}	

	BDBG_MSG((""));
	BDBG_MSG((""));	
	BDBG_MSG(("============================= "));
	BDBG_MSG(("===== RDB CONFIGURATION ===== "));
	BDBG_MSG(("============================= "));	
	BDBG_MSG(("RDB Chip Offset 0x%x ",psCitIp->sRdbCfg.ui32RdbDspOffset));
	BDBG_MSG(("RDB Base Offset 0x%x ",psCitIp->sRdbCfg.ui32RdbBaseOffset));

	BDBG_MSG((""));		
	BDBG_MSG(("================================= "));
	BDBG_MSG(("Independent Delay is [%s]",DisableEnable[psCitIp->eIndepDelayEnableFlag]));
	BDBG_MSG(("================================= "));
	BDBG_MSG((""));
	BDBG_MSG((""));


	BDBG_LEAVE(BRAP_CITGEN_P_PIInputPortCfgDBG);

}


/******************************************************************************
Summary:
	PPM Configuration Debug messages

Description: 
			
Input:
	psFwHwCfg	: 
Output:
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_PPMChannelCfgDBG(BRAP_AF_P_sFW_HW_CFG *psFwHwCfg)
{

	#ifdef BRAP_CIT_P_PRINT_PPM_CFG
		uint32_t	ui32Count;

		BDBG_ENTER(BRAP_CITGEN_P_PPMChannelCfgDBG);

		BDBG_MSG((""));
		BDBG_MSG(("PPM configuration"));
		BDBG_MSG((""));
		
		for(ui32Count =0; ui32Count<BRAP_AF_P_MAX_CHANNEL_PAIR;ui32Count++)
		{
			BDBG_MSG(("PPM Adaptive Rate Block Resouece %d is %s ",ui32Count,DisableEnable[psFwHwCfg->sPpmCfg[ui32Count].ePPMChannel]));		
			BDBG_MSG(("RDB Address 0x%x ",psFwHwCfg->sPpmCfg[ui32Count].ui32PPMCfgAddr));
			BDBG_MSG((""));
		}

		BDBG_LEAVE(BRAP_CITGEN_P_PPMChannelCfgDBG);

	#else
		/* No Functionality .. To Avoid warning */
		uint32_t Temp;

		BDBG_ENTER(BRAP_CITGEN_P_PPMChannelCfgDBG);

		Temp = psFwHwCfg->sPpmCfg[0].ui32PPMCfgAddr;

		BDBG_LEAVE(BRAP_CITGEN_P_PPMChannelCfgDBG);
	#endif


}




#ifdef TESTBENCH_BUILD

/* Convert Logical to Physical Addres*/
void BRAP_P_ConvertAddressToOffset(BMEM_Handle	hHeap,
								   void * a,
								   uint32_t* b)
{
	*b = (uint32_t)a;
}

/* Convert to Physical to Logical Addres*/
void BRAP_ConvertOffsetToAddress(BMEM_Handle	hHeap,
								 uint32_t	ui32DestAddr,
								 void **b
								 )
{
	*b = (void*)ui32DestAddr;
}

#endif


/******************************************************************************
Summary:
	Task output port Configuration 

Description: 

	Port Configuration 
			1) Process the Input from PI
			2) Prepare input to port Configuration function
			3) Configure port			
			4) Programming the EXT_CLOCK
			
Input:
	hHeap
	hRegister
	psCitIp						 : Cit Input
	psTaskBuffInfo				 : Buffer information for a task 
	pui32TaskSPDIFPortConfigAddr : Base address of each SPDIF port configuration.
	ui32TaskRdbOffset			 : RDB Offset
	psAlgoModePresent			 : Algorithm Present CIT structure 

Output:
		
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_TaskOutputPortCfg(
							BMEM_Handle						hHeap,
							BREG_Handle						hRegister,
							BRAP_CIT_P_InputInfo			*psCitIp,
							uint32_t						ui32TaskPortConfigAddr,
							BRAP_AF_P_sDRAM_BUFFER			*psTaskSPDIFPortConfigAddr,
							uint32_t						ui32TaskRdbOffset,
							BRAP_CIT_P_sAlgoModePresent		*psAlgoModePresent
						)
{
	uint32_t ui32Error;
	uint32_t ui32PllCount;				
	uint32_t ui32PllIndexToFw;

	BRAP_AF_P_PortEnableDisable eDacOnI2S;

	BRAP_CIT_P_sPortCfg	*psPortCfg = NULL;
	

	BDBG_ENTER(BRAP_CITGEN_P_TaskOutputPortCfg);	

	ui32Error = BERR_SUCCESS;

	
	psPortCfg =	(BRAP_CIT_P_sPortCfg*)BKNI_Malloc(SIZEOF(BRAP_CIT_P_sPortCfg));

	if(psPortCfg == NULL)
	{
		BDBG_ERR(("Unable to allocate memory for sPortCfg in function BRAP_CITGEN_P_TaskOutputPortCfg"));
		BDBG_LEAVE(BRAP_CITGEN_P_TaskOutputPortCfg);	
		return BERR_OUT_OF_SYSTEM_MEMORY;
	}
	else
	{
		/* Intializig the memory allcated with zeroes*/
		BKNI_Memset(psPortCfg ,0,sizeof( BRAP_CIT_P_sPortCfg));   		
	}


	/* Initializing the FMM Dst port configuration */
	/*---------------------------------------------*/
	BRAP_CITGEN_P_InitializeFmmDstCfg(	&psPortCfg->sFmmDestCfg[0]
									 );

	/*Intialization of DAC on I2S port enum */
	eDacOnI2S = BRAP_AF_P_PortDisable;

	/* Perform the Port configuration */
	if(psCitIp->ui32NumOutputs != 0)
	{
		
		/*Prepare the Output port configuration association */
		ui32Error = BRAP_CITGEN_P_PrepareOutputPortAssociation(	
											psCitIp,
											BRAP_AF_P_MAX_NUM_PLLS,
											&psPortCfg->sPllPortAssociation[0],
											&eDacOnI2S
										);
		
		/* Exit from further operation */
		if(ui32Error != BERR_SUCCESS)
		{
			goto BRAP_CITGENMODULE_P_PORTCFG_EXIT;		
		}


		/* Creating the output structure for each PLL */

		for(ui32PllCount =0; ui32PllCount<BRAP_AF_P_MAX_NUM_PLLS; ui32PllCount++)
		{
			ui32Error = BRAP_CITGEN_P_PortConfiguration( 										 
											 &psPortCfg->sFmmDestCfg[ui32PllCount],
											 &psPortCfg->sPllPortAssociation[ui32PllCount],
											 psTaskSPDIFPortConfigAddr,
											 ui32TaskRdbOffset,
											 psAlgoModePresent
										);
			/* Exit from further operation */
			if(ui32Error != BERR_SUCCESS)
			{
				goto BRAP_CITGENMODULE_P_PORTCFG_EXIT;
			}

		}
	}
	else
	{
		BDBG_MSG(("WARNING : There is no output port configured: System is configured for Capture Mode/MS11 Slave Task Mode "));		
	}


	/* Writing the sFmmDestCfg to DRAM   */
	/*-----------------------------------*/

#ifdef BRAP_CIT_P_ONLY_ONE_PLL_SUPPORT

	/* Get the Pll index to be passsed to FW */
	ui32Error = BRAP_CITGEN_P_FindAndSelectPllToFw( 
										psPortCfg,
										&ui32PllIndexToFw
									  );
	

	/* Exit from further operation */
	if(ui32Error != BERR_SUCCESS)
	{
		BDBG_ERR(("ERROR : PLL CONFIGURATION IS UNSUPPORTED"));
		BDBG_ERR(("ERROR : More than One PLL CONFIGURATIONs ARE SUPPORTED FOR TASK"));
		goto BRAP_CITGENMODULE_P_PORTCFG_EXIT;		
	}

	/****************************************************************************/
	/****************************************************************************
								
								  HACK for BTSC ENCODER					

	 Hack is: When BTSC encoder is enabled we are hacking the HBR Falg to get set
			  This is to set the DAC at 4X rate
				
	 ***************************************************************************/
	/****************************************************************************/

	if(BRAP_CIT_P_PRESENT ==  psAlgoModePresent->ui32BTSC_EncoderPresent)
	{
	  #if BTSC_HACK_ENABLED
		/* Over Riding the HBR Flag with Enable */

		BDBG_MSG(("################################################################################################"));
		BDBG_MSG(("CIT HAS DETECTED BTSC ENCODER:FORCING THE FMM TO BE ON HBR MODE. ONLY DAC PORT SHOULD BE PRESENT"));
		BDBG_MSG(("################################################################################################"));

		/* Access the Selected PLL and modify the HBR flag */
		/*psPortCfg->sFmmDestCfg[ui32PllIndexToFw].ui32HbrEnable = BRAP_AF_P_PortEnable;	*/

		
		
		BRAP_CITGEN_P_OverSampleHifiDac(
										hRegister,
										BRAP_AF_P_eEnable
	  								);			
		#endif						

	}	

	/*****************************HACK ENDS HERE ********************************/


	/* Writing the sFmmDestCfg to DRAM after the address conversion */
	BRAP_CITGEN_P_WriteToDRAM(
					hHeap,
					(uint32_t)&psPortCfg->sFmmDestCfg[ui32PllIndexToFw],
					(uint32_t)ui32TaskPortConfigAddr,
					(uint32_t)SIZEOF(psPortCfg->sFmmDestCfg)
			 );
#else

	/* Writing the sFmmDestCfg to DRAM after the address conversion */

	ui32PllIndexToFw = 0; /* The default value */

	BRAP_CITGEN_P_WriteToDRAM(
					hHeap,
					(uint32_t)&psPortCfg->sFmmDestCfg[ui32PllIndexToFw],
					(uint32_t)ui32TaskPortConfigAddr,
					(uint32_t)SIZEOF(psPortCfg->sFmmDestCfg)
			 );

#endif


	/*------------------------------------------*/
	/*  EXT Clock Configuration The HARD CODE : */
	/*-+----------------------------------------*/
	if(BRAP_AF_P_PortEnable == eDacOnI2S)
	{

		BRAP_CITGEN_P_ConfigureExternalClockSource(
											hRegister,
											ui32PllIndexToFw
										);
	}

	/* Debug Modules*/	
	BRAP_CITGEN_P_PIInputPortCfgDBG(psCitIp);	

	/*--------------------------------*/
	/* Free the memory space allocated*/
	/*--------------------------------*/
	BRAP_CITGENMODULE_P_PORTCFG_EXIT:
	BKNI_Free(psPortCfg);

	BDBG_LEAVE(BRAP_CITGEN_P_TaskOutputPortCfg);	
	return(ui32Error);

}

/******************************************************************************
Summary:
	BRAP_CITGEN_P_FindAndSelectPllToFw

Description: 

	Select the PLL Configuration to be passed to FW
			1) Check there are multiple PLLs configured for a task
				Return error if there are more.

			2) Get the pll index for FW
				Looking at the last Hit

			3) We need to consider the NCO enable to find the PLL index
			
Input:
	psPortCfg:	FMM port configuration

Output:	
	pui32PllIndexToFw : Pll Index 
		
Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FindAndSelectPllToFw(
								BRAP_CIT_P_sPortCfg			*psPortCfg,
								uint32_t					*pui32PllIndexToFw
							)
{

	uint32_t	ui32Count;
	uint32_t	ui32PllCount;
	uint32_t	ui32PllIndex;
	uint32_t	ui32Error;

	ui32PllCount =	0;
	ui32PllIndex =	0;
	ui32Error	 =	BERR_SUCCESS;

	BDBG_ENTER(BRAP_CITGEN_P_FindAndSelectPllToFw);

	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_PLLS;ui32Count++)
	{
		if(  (psPortCfg->sFmmDestCfg[ui32Count].sFmmDestPllCfg.ui32PllEnabled == (uint32_t)BRAP_AF_P_PortEnable) ||
			 (psPortCfg->sFmmDestCfg[ui32Count].eFmmPllNcoSelect == BRAP_AF_P_NcoSelect) 
		  )
		{
			ui32PllCount++;
			ui32PllIndex = ui32Count;
		}	
	}

	*pui32PllIndexToFw = ui32PllIndex;
	/* Checking if there are multiple PLLs are configured for the same task*/
	if(ui32PllCount>1)
	{
		ui32Error = BERR_INVALID_PARAMETER;
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FindAndSelectPllToFw);

	return ui32Error;

}

/******************************************************************************
Summary:
	RESET Port Configuration of a task

Description: 

	RESET Port Configuration 
			1) NCO		
			2) PLL		(Only PLL0 and PLL1		Limitations are as per 7405RDB
			3) I2S		(Only I2S0 and I2S1)
			4) HiFiDAC	(Only DAC0)
			5) SPDIF	(Only SPDIF0).
			
			 SPDIF1 is used only for MAI
			
Input:

	sFmmDestCfgArray:	FMM configuration for all th PLLs

Output:	
	sFmmDestCfgArray:	FMM configuration for all th PLLs
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_InitializeFmmDstCfg(
						BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfgArray						
				)
{
	uint32_t									ui32Count;				
	uint32_t									ui32PllCount;		
	BRAP_AF_P_sFMM_DEST_CFG						*psFmmDestCfg;
	BRAP_AF_P_sFMM_DEST_NCO_CFG					*psFmmNcoCfg;
	BRAP_AF_P_sFMM_DEST_PLL_CFG					*psFmmDestPllCfg;
	BRAP_AF_P_sFMM_DEST_SPDIF_CFG				*psFmmDestSpdifCfg;
	BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG				*psFmmDestI2SClkCfg;
	BRAP_AF_P_sFMM_DEST_DAC_CFG					*psFmmDestDacCfg;
	BRAP_AF_P_sFMM_ESR_REGISTER					*psFmmEsrReg;
	BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg;
	BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG		*psCapPortClkCfg;
	

	BDBG_ENTER(BRAP_CITGEN_P_InitializeFmmDstCfg);	
	

	for(ui32PllCount =0;ui32PllCount<BRAP_AF_P_MAX_NUM_PLLS;ui32PllCount++)
	{

		psFmmDestCfg = &psFmmDestCfgArray[ui32PllCount];

		psFmmNcoCfg			= &psFmmDestCfg->sFmmDestNcoCfg;
		psFmmDestPllCfg		= &psFmmDestCfg->sFmmDestPllCfg;
		psFmmDestSpdifCfg	= &psFmmDestCfg->sFmmDestSpdifCfg;


		/*-----------------*/
		/*NCO Configuration*/
		/*-----------------*/
		psFmmDestCfg->eFmmPllNcoSelect			= BRAP_AF_P_NoneSelect;	/*Global select to None*/

		psFmmNcoCfg->ui32NcoPhaseIncRegAddr		= (uint32_t)NULL;
		psFmmNcoCfg->ui32NcoRateRatioRegAddr	= (uint32_t)NULL;
		psFmmNcoCfg->ui32NcoSampleIncRegAddr	= (uint32_t)NULL;

		/*-----------------*/
		/*PLL Configuration*/
		/*-----------------*/

		psFmmDestPllCfg->ui32PllEnabled				= (uint32_t)BRAP_AF_P_PortDisable;
		psFmmDestPllCfg->ui32PllIndex				= (uint32_t)BRAP_CIT_P_INVALID;
		psFmmDestPllCfg->ui32AudioPllMacroRegAddr	= (uint32_t)NULL;	


		/*------------------*/
		/* HBR Configuration*/
		/*------------------*/
		/*psFmmDestCfg->ui32HbrEnable = (uint32_t)BRAP_AF_P_PortDisable;*/
		psFmmDestCfg->eHdmiFsRate   = BRAP_AF_P_FmmDstFsRate_eBaseRate;

		/*---------------------*/
		/* HwCbit Configuration*/
		/*---------------------*/

		psFmmDestCfg->ui32HwCbitsEnable = (uint32_t)BRAP_AF_P_PortDisable;

		/*------------------*/
		/*I2S Configuration */
		/*------------------*/

		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_I2S_PORTS;ui32Count++)
		{
			psFmmDestI2SClkCfg = &psFmmDestCfg->sFmmDestI2SClkCfg[ui32Count];

			psFmmDestI2SClkCfg->ui32I2SEnabled				= (uint32_t)BRAP_AF_P_PortDisable;
			psFmmDestI2SClkCfg->eFmmDstFsRate				= BRAP_AF_P_FmmDstFsRate_eInvalid;

			psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr = (uint32_t)NULL;

		}


		/*-------------------*/
		/* SPDIF Configuration */
		/*-------------------*/
		psFmmEsrReg = &psFmmDestCfg->sFmmDestSpdifCfg.sFmmEsrRegister;

		psFmmEsrReg->ui32AudFmmMsEsrStatusRegAddr			= (uint32_t)NULL;
		psFmmEsrReg->ui32AudFmmMsEsrStatusClearRegAddr		= (uint32_t)NULL;


		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_SPDIF_PORTS;ui32Count++)
		{
			psFmmDestSpdifClkCbitCfg
						= &psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[ui32Count];

			
			psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled 		 = (uint32_t)BRAP_AF_P_PortDisable;	
			psFmmDestSpdifClkCbitCfg->eFmmDstFsRate			 = BRAP_AF_P_FmmDstFsRate_eInvalid;

			psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
															 = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg	 = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr	 = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr	 = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32PingPongMask		 = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr = (uint32_t)NULL;
			psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif		 = (uint32_t)NULL;	
			psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
															 = (uint32_t)NULL;	
			
		}	

		/*-------------------*/
		/* DAC Configuration */
		/*-------------------*/

		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_DAC_PORTS;ui32Count++)
		{
			psFmmDestDacCfg = &psFmmDestCfg->sFmmDestDacCfg[ui32Count];

			psFmmDestDacCfg->ui32DacEnabled					= (uint32_t)BRAP_AF_P_PortDisable;
			psFmmDestDacCfg->eFmmDstFsRate					= BRAP_AF_P_FmmDstFsRate_eInvalid;
			psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr	= (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr	= (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr	= (uint32_t)NULL;		
			psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr	= (uint32_t)NULL;
		}

		/*------------------------*/
		/* CAPTURE Configuration  */
		/*------------------------*/

		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_CAP_PORTS;ui32Count++)
		{
			psCapPortClkCfg = &psFmmDestCfg->sFmmCapPortClkCfg[ui32Count];

			psCapPortClkCfg->ui32CapPortEnabled				= (uint32_t)BRAP_AF_P_PortDisable;
			psCapPortClkCfg->eFmmDstFsRate					= BRAP_AF_P_FmmDstFsRate_eInvalid;

			psCapPortClkCfg->ui32AudioCapPortClkMacroRegAddr= (uint32_t)NULL;
		}		
	}

	BDBG_LEAVE(BRAP_CITGEN_P_InitializeFmmDstCfg);	
}

/******************************************************************************
Summary:
	Task prepare output port association

Description: 

	Port Configuration 			

		1) Initalise the sPllPortAssociation structure

		2) Sort all ports in sPllPortAssociation per PLL basis

		3) sPllPortAssociation holds only the PI-CIT port cfg only

		4) Detecting any I2S connected External Dacs

	NCO Configuration :
			In NCO configuration, we are using the PLL based structures for processsing,
			Here we use the PLL index 0 for NCO and associate all the ports associated
			with NCO to PLL index 0.
			
Input:	
	psCitIp			: CIT Input
	Nui32NumPlls	: Number of PLLs

Output:
	BRAP_CIT_P_sPllPortAssociation	: PortAssociation structure
		
Returns:
	None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_PrepareOutputPortAssociation(
				BRAP_CIT_P_InputInfo				*psCitIp,
				uint32_t							ui32NumPlls,
				BRAP_CIT_P_sPllPortAssociation		*psPllPortAssociation,
				BRAP_AF_P_PortEnableDisable			*peDacOnI2S
			)
{

	uint32_t ui32Error;
	uint32_t ui32Count;
	uint32_t ui32PllCount;
	uint32_t ui32NumPortsAvailable;
	uint32_t ui32PllIndex;

	BRAP_CapInputPort				eCapIpPort;
	BRAP_OutputPort					eOutputPort;	
	BRAP_AF_P_PllNcoSelect			ePllNcoSelect;
	BRAP_AF_P_FmmDstFsRate			eFmmDstFsRate;
	BRAP_CIT_P_FmmDstSelect			eFmmDstSelect;/* FMM port Type*/   

	BRAP_CIT_P_PortCfg		*psPortCfg;	


	BDBG_ENTER(BRAP_CITGEN_P_PrepareOutputPortAssociation);	

	/*----------------*/
	/* Initialization */
	/*----------------*/
	ui32Error = BERR_SUCCESS;


	/* Getting the number of output ports */
	/*------------------------------------*/
	ui32NumPortsAvailable	= psCitIp->ui32NumOutputs;

	if(ui32NumPortsAvailable == 0)
	{
		
		BDBG_ERR(("ERROR : There is no output port configured "));
		BDBG_LEAVE(BRAP_CITGEN_P_PrepareOutputPortAssociation);	
		return BERR_UNKNOWN;
	}

	/* Disabling all the ports */
	for(ui32PllCount =0;ui32PllCount<ui32NumPlls;ui32PllCount++)
	{	
		
		/* Initializing the NCO Cfg*/
		psPllPortAssociation[ui32PllCount].ePllNcoSelect
											=BRAP_AF_P_NoneSelect;					

		/* Initializing the PLL Cfg*/
		psPllPortAssociation[ui32PllCount].ui32PllEnable 
											= (uint32_t)BRAP_AF_P_PortDisable;

		psPllPortAssociation[ui32PllCount].ui32PllIndex 
											= (uint32_t)BRAP_CIT_P_INVALID;	

		/* Initializing the I2S port Cfg*/
		for(ui32Count =0; ui32Count < BRAP_AF_P_MAX_NUM_I2S_PORTS ; ui32Count++)
		{

			psPllPortAssociation[ui32PllCount].sI2SPortInfo[ui32Count].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortDisable;

			psPllPortAssociation[ui32PllCount].sI2SPortInfo[ui32Count].eFmmDstFsRate
											= BRAP_AF_P_FmmDstFsRate_eInvalid;
									
		}

		/* Initializing the Dac port Cfg*/
		for(ui32Count =0; ui32Count < BRAP_AF_P_MAX_NUM_DAC_PORTS ; ui32Count++)
		{

			psPllPortAssociation[ui32PllCount].sDacPortInfo[ui32Count].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortDisable;

			psPllPortAssociation[ui32PllCount].sDacPortInfo[ui32Count].eFmmDstFsRate
											= BRAP_AF_P_FmmDstFsRate_eInvalid;
			
		}

		/* Initializing the Cap port Cfg*/
		for(ui32Count =0; ui32Count < BRAP_AF_P_MAX_NUM_CAP_PORTS ; ui32Count++)
		{

			psPllPortAssociation[ui32PllCount].sCapPortInfo[ui32Count].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortDisable;

			psPllPortAssociation[ui32PllCount].sCapPortInfo[ui32Count].eFmmDstFsRate
											= BRAP_AF_P_FmmDstFsRate_eInvalid;
		}

		/* Initializing the Spdif port Cfg*/
		for(ui32Count =0; ui32Count < BRAP_AF_P_MAX_NUM_SPDIF_PORTS ; ui32Count++)
		{

			/* SPDIF Control information */
			psPllPortAssociation[ui32PllCount].sSpdifPortInfo[ui32Count].sSpdifPortControlInfo.ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortDisable;

			psPllPortAssociation[ui32PllCount].sSpdifPortInfo[ui32Count].sSpdifPortControlInfo.eFmmDstFsRate
											= BRAP_AF_P_FmmDstFsRate_eInvalid;

			
			/* SPDIF Data content information */
			psPllPortAssociation[ui32PllCount].sSpdifPortInfo[ui32Count].sSpdifInputCfg.bPcmOnSpdif	   
											= BRAP_CITGEN_P_False;

			psPllPortAssociation[ui32PllCount].sSpdifPortInfo[ui32Count].sSpdifInputCfg.bHbr
											= BRAP_CITGEN_P_False;

			psPllPortAssociation[ui32PllCount].sSpdifPortInfo[ui32Count].sSpdifInputCfg.bHwCbits
											= BRAP_CITGEN_P_False;	



		}

		
		/* MAI port Control Configuration*/
		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiPortControlInfo.ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortDisable;

		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiPortControlInfo.eFmmDstFsRate
											= BRAP_AF_P_FmmDstFsRate_eInvalid;

		/* MAI Data Configuration information */
		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiCfg.eMaiMuxSelector 
											= BRAP_OutputPort_eMax;

		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiCfg.sMaiContent.bPcmOnSpdif
											= BRAP_CITGEN_P_False;

		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiCfg.sMaiContent.bHbr
											= BRAP_CITGEN_P_False;

		psPllPortAssociation[ui32PllCount].sMaiPortInfo.sMaiCfg.sMaiContent.bHwCbits
											= BRAP_CITGEN_P_False;	


	}

	/*----------------------------------------------*/
	/* Getting the port information per PLL basis   */
	/*----------------------------------------------*/
	for(ui32Count =0; ui32Count < ui32NumPortsAvailable ; ui32Count++)
	{
		
		/* Step 1.Selecting the Port */
		/*---------------------------*/
		psPortCfg			= &psCitIp->sPortCfg[ui32Count];

		
		/* Step 2.Derive the Detination port Select and PLL/NCO select of the port*/	
		/*------------------------------------------------------------------------*/
		ePllNcoSelect		= psPortCfg->ePllNcoSelect;					/* Getting current NCO type */
		eFmmDstSelect		= psPortCfg->sFmmDstDetails.eFmmDstSelect;	/* Getting the  FMM Dst Type*/						
		
		/* Step 3.Finding the PLL index to index port cfg structures*/
		/*----------------------------------------------------------*/

		/*Enable PLL cfg if port is associated with PLL*/		
		if(BRAP_AF_P_PllSelect == ePllNcoSelect)
		{
			
			ui32PllIndex				= psPortCfg->ui32PllIndex;	/* Getting current PLL Index*/
			
			psPllPortAssociation[ui32PllIndex].ePllNcoSelect		/* Getting current NCO select*/
										=  ePllNcoSelect;

			psPllPortAssociation[ui32PllIndex].ui32PllIndex			/* Pll Select */
										= ui32PllIndex;
		
			psPllPortAssociation[ui32PllIndex].ui32PllEnable		/*Enable PLL */
										= (uint32_t)BRAP_AF_P_PortEnable;
		}
		else if(BRAP_AF_P_NcoSelect == ePllNcoSelect)
		{			
			/*
			  Selecting the PLL index as 0 for ports which are NCO coupled
			  This is to associate all the NCO ports to PLL index 0.
			 */

			ui32PllIndex =0;		
			/* ui32PllIndex					= psPortCfg->ui32NcoIndex;	Getting current NCO Index in future... */

			psPllPortAssociation[ui32PllIndex].ePllNcoSelect		/* Getting current NCO select*/
											=  ePllNcoSelect;

			/* PLL cfg already set to default as Disable */			
			psPllPortAssociation[ui32PllIndex].ui32PllEnable 
											= (uint32_t)BRAP_AF_P_PortDisable;

			psPllPortAssociation[ui32PllIndex].ui32PllIndex 
											= (uint32_t)ui32PllIndex;	
		}
		else
		{
			/*Getting the port type to check for DAC */
			eOutputPort	= psPortCfg->sFmmDstDetails.uFmmDstPortType.eOpType;

			if((BRAP_CIT_P_FmmDstSelect_eOpPort == eFmmDstSelect) && (
																		(BRAP_OutputPort_eDac0 == eOutputPort) || 
																		(BRAP_OutputPort_eDac1 == eOutputPort) || 
																		(BRAP_OutputPort_eDac2 == eOutputPort) 
																	  )
			  )
			{	
				ui32PllIndex = 0;
            
				psPllPortAssociation[ui32PllIndex].ui32PllIndex   /* Pll Select */
					= ui32PllIndex;
                
				psPllPortAssociation[ui32PllIndex].ePllNcoSelect  /* Getting None select*/
				  =  ePllNcoSelect;

				/* PLL cfg already set to default as Disable */   
				psPllPortAssociation[ui32PllIndex].ui32PllEnable 
				   = (uint32_t)BRAP_AF_P_PortDisable;
			}
			else
			{

				/*Port is neither PLL nor NCO associated*/
				ui32Error = BERR_NOT_SUPPORTED;

				BDBG_ERR(("ERROR : The input port No.[%d] is neither PLL nor NCO associated",ui32Count));

				break;
			}
		}

		/* Step 4. Deriving the Port Cfg and Fs Rate */
		/*-------------------------------------------*/

		/* a) Getting the Fs rate */
		eFmmDstFsRate = psPortCfg->eFmmDstFsRate;

		ui32Error = BRAP_CITGEN_P_VerifyFmmFsRate(eFmmDstFsRate);
		
		if(BERR_SUCCESS != ui32Error)
		{			
			BDBG_ERR(("ERROR : Wrong Fs sink rate=[%x] for the input port No.[%d]" ,eFmmDstFsRate,ui32Count));
			break;
		}
		else
		{
			/*Continue the execution*/
		}

		/* b) FMM Dst output PORT configuration */
		if(BRAP_CIT_P_FmmDstSelect_eOpPort == eFmmDstSelect)
		{	

			eOutputPort	= psPortCfg->sFmmDstDetails.uFmmDstPortType.eOpType;		/* Getting current Op Type */

			switch(eOutputPort)
			{			
				case	BRAP_OutputPort_eI2s0:
				case	BRAP_OutputPort_eMaiMulti0:


						psPllPortAssociation[ui32PllIndex].sI2SPortInfo[0].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sI2SPortInfo[0].eFmmDstFsRate
											= eFmmDstFsRate;


						break;
						
				case	BRAP_OutputPort_eI2s1:
				case	BRAP_OutputPort_eI2s2:
				case	BRAP_OutputPort_eI2s3:						
				case	BRAP_OutputPort_eI2s4:
				case	BRAP_OutputPort_eI2s5:
				case	BRAP_OutputPort_eI2s6:
				case	BRAP_OutputPort_eI2s7:
				case	BRAP_OutputPort_eI2s8:
				case	BRAP_OutputPort_eMaiMulti1:
				case	BRAP_OutputPort_eMaiMulti2:
				case	BRAP_OutputPort_eMaiMulti3:


						psPllPortAssociation[ui32PllIndex].sI2SPortInfo[1].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sI2SPortInfo[1].eFmmDstFsRate
											= eFmmDstFsRate;


						break;

				case	BRAP_OutputPort_eDac0:

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[0].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[0].eFmmDstFsRate
											= eFmmDstFsRate;

						break;

				case	BRAP_OutputPort_eDac1:

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[1].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[1].eFmmDstFsRate
											= eFmmDstFsRate;

						break;

				case	BRAP_OutputPort_eDac2:

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[2].ui32PortEnabled
											= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sDacPortInfo[2].eFmmDstFsRate
											= eFmmDstFsRate;

						break;

				case	BRAP_OutputPort_eSpdif :


						/*Control information */
						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[0].sSpdifPortControlInfo.ui32PortEnabled 
										= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[0].sSpdifPortControlInfo.eFmmDstFsRate 
										= eFmmDstFsRate;
						
						/*Data information*/
						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[0].sSpdifInputCfg.bPcmOnSpdif	   
										= (uint32_t)psPortCfg->uOpCfg.sSpdifCfg.bPcmOnSpdif;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[0].sSpdifInputCfg.bHbr
										=	psPortCfg->uOpCfg.sSpdifCfg.bHbr;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[0].sSpdifInputCfg.bHwCbits
										=	psPortCfg->uOpCfg.sSpdifCfg.bHwCbits;
						
															
						break;

				case	BRAP_OutputPort_eSpdif1:

						/*Control information */
						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[1].sSpdifPortControlInfo.ui32PortEnabled 
										= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[1].sSpdifPortControlInfo.eFmmDstFsRate 
										= eFmmDstFsRate;
						
						/*Data information*/
						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[1].sSpdifInputCfg.bPcmOnSpdif	   
										= (uint32_t)psPortCfg->uOpCfg.sSpdifCfg.bPcmOnSpdif;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[1].sSpdifInputCfg.bHbr
										=	psPortCfg->uOpCfg.sSpdifCfg.bHbr;

						psPllPortAssociation[ui32PllIndex].sSpdifPortInfo[1].sSpdifInputCfg.bHwCbits
										=	psPortCfg->uOpCfg.sSpdifCfg.bHwCbits;
						

						break;

				case	BRAP_OutputPort_eMai:
						
						/*Control information */
						psPllPortAssociation[ui32PllIndex].sMaiPortInfo.sMaiPortControlInfo.ui32PortEnabled
										= (uint32_t)BRAP_AF_P_PortEnable;

						psPllPortAssociation[ui32PllIndex].sMaiPortInfo.sMaiPortControlInfo.eFmmDstFsRate
										= eFmmDstFsRate;

				
						psPllPortAssociation[ui32PllIndex].sMaiPortInfo.sMaiCfg
										= psPortCfg->uOpCfg.sMaiCfg;


						break;
						
				case	BRAP_OutputPort_eFlex:
				case	BRAP_OutputPort_eRfMod:
				default:
						ui32Error = BERR_NOT_SUPPORTED;
						BDBG_ERR(("ERROR : The input port [%s] is not supported",PortName[(uint16_t)eOutputPort]));

						break;			
			}

			/*-------------------------*/
			/*Detecting the I2S on Dac */
			/*-------------------------*/
			switch(eOutputPort)
			{			
				case	BRAP_OutputPort_eI2s4:

						/* Enabling the DAC on I2S port for I2s4. Setting the Global Flag */

						/* HARD CODED for I2S4*/

						*peDacOnI2S	= BRAP_AF_P_PortEnable; 

						break;
				default:	

						break;
			}			
		}
		/* FMM Dst output CAP PORT configuration*/
		else if(BRAP_CIT_P_FmmDstSelect_eCapturePort == eFmmDstSelect)
		{

			eCapIpPort = psPortCfg->sFmmDstDetails.uFmmDstPortType.eCapPortType;

			switch(eCapIpPort)
			{			
				case	BRAP_CapInputPort_eIntCapPort0:

						
						psPllPortAssociation[ui32PllIndex].sCapPortInfo[0].ui32PortEnabled
										= (uint32_t)BRAP_AF_P_PortEnable;	

						psPllPortAssociation[ui32PllIndex].sCapPortInfo[0].eFmmDstFsRate
										= eFmmDstFsRate;

						break;

				case	BRAP_CapInputPort_eIntCapPort1:


						psPllPortAssociation[ui32PllIndex].sCapPortInfo[1].ui32PortEnabled
										= (uint32_t)BRAP_AF_P_PortEnable;	

						psPllPortAssociation[ui32PllIndex].sCapPortInfo[1].eFmmDstFsRate
										= eFmmDstFsRate;
						break;

				case	BRAP_CapInputPort_eIntCapPort2:

						
						psPllPortAssociation[ui32PllIndex].sCapPortInfo[2].ui32PortEnabled
										= (uint32_t)BRAP_AF_P_PortEnable;	

						psPllPortAssociation[ui32PllIndex].sCapPortInfo[2].eFmmDstFsRate
										= eFmmDstFsRate;
						break;

				case	BRAP_CapInputPort_eIntCapPort3:

						
						psPllPortAssociation[ui32PllIndex].sCapPortInfo[3].ui32PortEnabled
										= (uint32_t)BRAP_AF_P_PortEnable;	

						psPllPortAssociation[ui32PllIndex].sCapPortInfo[3].eFmmDstFsRate
										= eFmmDstFsRate;
						break;

				case	BRAP_CapInputPort_eIntCapPort4:
				case	BRAP_CapInputPort_eIntCapPort5:
				case	BRAP_CapInputPort_eIntCapPort6:
				case	BRAP_CapInputPort_eIntCapPort7:
				case	BRAP_CapInputPort_eExtI2s0:
				case	BRAP_CapInputPort_eRfAudio:  
				case	BRAP_CapInputPort_eSpdif:  
				case	BRAP_CapInputPort_eHdmi:    
				case	BRAP_CapInputPort_eAdc:   
				default:

						ui32Error = BERR_NOT_SUPPORTED;
						BDBG_ERR(("ERROR : The input port [%s] is not supported",CapPortName[(uint16_t)eCapIpPort]));
						break;
			}		
			
		}
		else
		{
			ui32Error = BERR_NOT_SUPPORTED;
			BDBG_ERR(("ERROR : FMM Dst Type is not supported"));		
		}

		if(ui32Error != BERR_SUCCESS)
		{
			/* Coming output of the For Loop */
			break;
		}
	}
	
	BDBG_LEAVE(BRAP_CITGEN_P_PrepareOutputPortAssociation);	
	return ui32Error;
}



/******************************************************************************
Summary:
	Port Configuration of a task

Description: 

	Port Configuration 
			1) PLL		(Only PLL0 and PLL1		Limitations are as per 7405RDB
			2) I2S		(Only I2S0 and I2S1)
			3) HiFiDAC	(Only DAC0)
			4) SPDIF	(Only SPDIF0)

	SPDIF1 is used only for MAI.
			
  Special case:

	SPDIF0: 
		In 3548 we can use SPDIF 0 in HBR mode for DDP PT with HWCITs always disabled.
		BCM3548 doesnot has HWCBITS so we are expected to configure FWCBITS in HBR 
		mode[4X sampling frequency]. And we should not get HwCITS flag to set.

  Common Stand :
			For A task, if the HBR/HW_FW-Cbit is selected, then it is applied for
			all SPDIF/MAI ports and PI needs to associate the same with all ports
			accordingly.....
			
Input:
	psTaskBuffInfo				 : Buffer information for a task 
	pui32TaskSPDIFPortConfigAddr : Base address of each SPDIF port configuration.
	ui32TaskRdbOffset			 : RDB Offset
	psAlgoModePresent			 : Algorithm Present CIT structure 

Output:	

  
		
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_PortConfiguration(
							BRAP_AF_P_sFMM_DEST_CFG				*psFmmDestCfg,
							BRAP_CIT_P_sPllPortAssociation		*psPllPortAssociation,
							BRAP_AF_P_sDRAM_BUFFER				*psTaskSPDIFPortConfigAddr,
							uint32_t							ui32TaskRdbOffset,
							BRAP_CIT_P_sAlgoModePresent			*psAlgoModePresent
						)
{
	uint32_t									ui32Error;	
	uint32_t									ui32Count;	
	uint32_t									ui32SpdifContent;
	uint32_t									ui32DDP_PassThruPresent;	
	uint32_t									ui32SpdifCfgDramBufferAddress;
	BRAP_AF_P_sFMM_DEST_NCO_CFG					*psFmmDestNcoCfg;
	BRAP_AF_P_sFMM_DEST_PLL_CFG					*psFmmDestPllCfg;
	BRAP_AF_P_sFMM_DEST_SPDIF_CFG				*psFmmDestSpdifCfg;
	BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG				*psFmmDestI2SClkCfg;
	BRAP_AF_P_sFMM_ESR_REGISTER					*psFmmEsrReg;
	BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg;
	BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG		*psFmmCapPortClkCfg;


	BDBG_ENTER(BRAP_CITGEN_P_PortConfiguration);

	ui32Error			= BERR_SUCCESS;
	psFmmDestPllCfg		= &psFmmDestCfg->sFmmDestPllCfg;
	psFmmDestNcoCfg		= &psFmmDestCfg->sFmmDestNcoCfg;
	psFmmDestSpdifCfg	= &psFmmDestCfg->sFmmDestSpdifCfg;
	

	/* DDP Passthrough Special case */	
	ui32DDP_PassThruPresent	= psAlgoModePresent->ui32DDP_PassThruPresent;
	
	/*Global PLL/NCO select */
	psFmmDestCfg->eFmmPllNcoSelect = psPllPortAssociation->ePllNcoSelect;

	/*------------------*/
	/*Port Configuration*/
	/*------------------*/
	
	if( (BRAP_AF_P_PllSelect  ==  psPllPortAssociation->ePllNcoSelect) ||
		(BRAP_AF_P_NcoSelect  ==  psPllPortAssociation->ePllNcoSelect)
	  )	  
	{
		/*------------------*/
		/*PLL Configuration */
		/*------------------*/

		if(BRAP_AF_P_PllSelect  ==  psPllPortAssociation->ePllNcoSelect)
		{		

			psFmmDestPllCfg->ui32PllEnabled =	psPllPortAssociation->ui32PllEnable;	
			psFmmDestPllCfg->ui32PllIndex	=	psPllPortAssociation->ui32PllIndex;	

			ui32Error = BRAP_CITGEN_P_PllCfg(psFmmDestPllCfg,
											 psFmmDestPllCfg->ui32PllIndex,
											 ui32TaskRdbOffset
										);

			if(BERR_SUCCESS != ui32Error)
			{
				goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;	
			}
		}
		/*------------------*/
		/*NCO Configuration */
		/*------------------*/
		else if(BRAP_AF_P_NcoSelect  ==  psPllPortAssociation->ePllNcoSelect)
		{
			ui32Error = BRAP_CITGEN_P_NcoCfg( psFmmDestNcoCfg,
											  ui32TaskRdbOffset
										);

			if(BERR_SUCCESS != ui32Error)
			{
				goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;	
			}

		}
		/*------------------*/
		/*I2S Configuration */
		/*------------------*/

		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_I2S_PORTS;ui32Count++)
		{
			psFmmDestI2SClkCfg = &psFmmDestCfg->sFmmDestI2SClkCfg[ui32Count];

			psFmmDestI2SClkCfg->ui32I2SEnabled = psPllPortAssociation->sI2SPortInfo[ui32Count].ui32PortEnabled;

			if(psFmmDestI2SClkCfg->ui32I2SEnabled == (uint32_t)BRAP_AF_P_PortEnable)
			{
				
				/* Getting the Clock Rate */
				psFmmDestI2SClkCfg->eFmmDstFsRate  
						= psPllPortAssociation->sI2SPortInfo[ui32Count].eFmmDstFsRate;

				/* Master Clock Configuration */
				if(ui32Count == 0)
				{	
					/* Stereo I2S */
					BRAP_CITGEN_P_I2SCfg(psFmmDestI2SClkCfg,
										 ui32TaskRdbOffset
									);						
				}
				else if(ui32Count == 1)
				{	
					/* MultiChannel I2S */					
					BRAP_CITGEN_P_I2S_MultiCfg(psFmmDestI2SClkCfg,
											   ui32TaskRdbOffset
										);					
				}
				else
				{
					/* I2S Input Valid Check */
					BDBG_ERR((" Unsupported I2S index = [%d]",ui32Count));
					psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr = (uint32_t)NULL;
					ui32Error = BERR_NOT_SUPPORTED;
					goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;
				}
			}
			else
			{
				psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr = (uint32_t)NULL;
			}
		}

		/*---------------------*/
		/* SPDIF Configuration */
		/*---------------------*/
		psFmmEsrReg = &psFmmDestCfg->sFmmDestSpdifCfg.sFmmEsrRegister;

		psFmmEsrReg->ui32AudFmmMsEsrStatusRegAddr	
							=	BCHP_AUD_FMM_MS_ESR_STATUS + ui32TaskRdbOffset;

		psFmmEsrReg->ui32AudFmmMsEsrStatusClearRegAddr
							=	BCHP_AUD_FMM_MS_ESR_STATUS_CLEAR + ui32TaskRdbOffset;


		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_SPDIF_PORTS;ui32Count++)
		{

			psFmmDestSpdifClkCbitCfg
					= &psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[ui32Count];

			psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled 
					= psPllPortAssociation->sSpdifPortInfo[ui32Count].sSpdifPortControlInfo.ui32PortEnabled;
		
			if(psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled == (uint32_t)BRAP_AF_P_PortEnable)
			{

				/* Getting the Clock Rate */
				psFmmDestSpdifClkCbitCfg->eFmmDstFsRate  
						= psPllPortAssociation->sSpdifPortInfo[ui32Count].sSpdifPortControlInfo.eFmmDstFsRate;

				if(ui32Count == 0)
				{				
					/* SPDIF Dram Cfg Address*/
					ui32SpdifCfgDramBufferAddress = 
						 (uint32_t)psTaskSPDIFPortConfigAddr[ui32Count].ui32DramBufferAddress;
					

					/* SPDIF Data type*/

					ui32SpdifContent =
						 (psPllPortAssociation->sSpdifPortInfo[ui32Count].sSpdifInputCfg.bPcmOnSpdif)
						  ? (uint32_t)BRAP_AF_P_CompressedOnSpdif : (uint32_t)BRAP_AF_P_PcmOnSpdif;



					/*FwCbit - HwCbit switch Mode*/
					if(BRAP_CITGEN_P_False == psPllPortAssociation->sSpdifPortInfo[ui32Count].sSpdifInputCfg.bHwCbits)
					{					
						/* SPDIF 0 FWCbit Configuration*/
						BRAP_CITGEN_P_Spdif0FwCbitsCfg(
											psFmmDestSpdifClkCbitCfg,
											ui32SpdifCfgDramBufferAddress,
											ui32SpdifContent,
											ui32TaskRdbOffset
										);	

						psFmmDestCfg->ui32HwCbitsEnable = BRAP_AF_P_PortDisable;
					
					}
					else 
					{
						/* SPDIF 0 HWCbit Configuration*/
						BRAP_CITGEN_P_Spdif0HwCbitsCfg(
											psFmmDestSpdifClkCbitCfg,
											ui32SpdifCfgDramBufferAddress,
											ui32SpdifContent,
											ui32TaskRdbOffset
										);	

						psFmmDestCfg->ui32HwCbitsEnable = BRAP_AF_P_PortEnable;
					}

					/*HBR Flag Configuration */
					if(BRAP_CITGEN_P_True == psPllPortAssociation->sSpdifPortInfo[ui32Count].sSpdifInputCfg.bHbr)
					{
						if(ui32DDP_PassThruPresent == BRAP_CIT_P_PRESENT)	
						{
							
							/*psFmmDestCfg->ui32HbrEnable = BRAP_AF_P_PortEnable;	 */
							psFmmDestCfg->eHdmiFsRate   = BRAP_AF_P_FmmDstFsRate_e4xBaseRate;/*Global flag */
						}
						else
						{
							BDBG_ERR(("Error: Spdif Port with index = [%d] is configured for HBR mode and DDP passthrough is absent in the network",ui32Count));

							ui32Error = BERR_NOT_SUPPORTED;
							goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;                    
								
						}
					}

																				#ifdef OLD_HWCBIT_DETERMINING_LOGIC
																									/* HwCbit Flag Configuration */
																									if(BRAP_CITGEN_P_True == psPllPortAssociation->sSpdifInputCfg[ui32Count].bHwCbits)
																									{

																										if(ui32DDP_PassThruPresent == BRAP_CIT_P_PRESENT)	
																										{
																											psFmmDestCfg->ui32HwCbitsEnable = BRAP_AF_P_PortEnable;	/*Global flag */
																										}
																										else
																										{
																											BDBG_ERR(("Error: Spdif Port with index = [%d] is configured for HwCBIT mode and DDP passthrough is absent in the network",ui32Count));

																											ui32Error = BERR_NOT_SUPPORTED;
																											goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;                    
																												
																										}
																									}
																				#endif
					
				}
				else
				{
					/* Range / valid check is required */
					BDBG_ERR(("Error: Spdif Port with index = [%d] is not supported",ui32Count));

					psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
																	 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg	 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr	 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr	 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32PingPongMask		 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif		 = (uint32_t)NULL;
					psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
																	 = (uint32_t)NULL;

					ui32Error = BERR_NOT_SUPPORTED;
					goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;                    

				}
			}
		}

		/*-------------------*/
		/* MAI Configuration */
		/*-------------------*/
		
		if(psPllPortAssociation->sMaiPortInfo.sMaiPortControlInfo.ui32PortEnabled == (uint32_t)BRAP_AF_P_PortEnable)
		{			
			ui32Error =	BRAP_CITGEN_P_MaiPortConfiguration(				
											psFmmDestCfg,						/*[This is for a PLL] */
											&psPllPortAssociation->sMaiPortInfo,/*[Mai Input Configuration]*/
											psTaskSPDIFPortConfigAddr,			/*[Spdif Dram Cfg buffer addr]*/
											&psFmmDestCfg->eHdmiFsRate,			/*[HBR enable]*/
											&psFmmDestCfg->ui32HwCbitsEnable,	/*[HCBITs enable]*/
											ui32DDP_PassThruPresent,			/*[DDP Present Flag]*/
											ui32TaskRdbOffset
										);	
			
			if(ui32Error != BERR_SUCCESS)
			{
				goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;
			}

		}

		/*-----------------------*/
		/*Cap Port Configuration */
		/*-----------------------*/

		for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_CAP_PORTS;ui32Count++)
		{			
			psFmmCapPortClkCfg = &psFmmDestCfg->sFmmCapPortClkCfg[ui32Count];


			psFmmCapPortClkCfg->ui32CapPortEnabled 
								= psPllPortAssociation->sCapPortInfo[ui32Count].ui32PortEnabled;


			if(psFmmCapPortClkCfg->ui32CapPortEnabled == (uint32_t)BRAP_AF_P_PortEnable)
			{				
				
				/*Extracting the Fs Rate */
				psFmmCapPortClkCfg->eFmmDstFsRate	   
								= psPllPortAssociation->sCapPortInfo[ui32Count].eFmmDstFsRate;


				BRAP_CITGEN_P_CapPortNCfg(	psFmmCapPortClkCfg,
											ui32TaskRdbOffset,
											ui32Count
										 );
			}
			else
			{
				/*Defaulting to NULL */
				psFmmCapPortClkCfg->ui32AudioCapPortClkMacroRegAddr
												= (uint32_t)NULL;
			}
		}
	}

	/*-------------------*/
	/* DAC Configuration */
	/*-------------------*/
	
	/* There are 3 definitions for this function.....
	   One : 
			1) 7405/7335/7325 (DAC-0)
			2) 7420 (DAC-0 DAC-1)
			3) 3548	(DAC-0 DAC-1 DAC-2)

		This is done for adding 
	*/
	ui32Error = BRAP_CITGEN_P_DAC_Configuration(psFmmDestCfg,
												psPllPortAssociation,
												ui32TaskRdbOffset
											);

	if(ui32Error != BERR_SUCCESS)
	{
		goto BRAP_CITGEN_PORT_CFG_P_EXIT_POINT;
	}





	BRAP_CITGEN_PORT_CFG_P_EXIT_POINT:

	BDBG_LEAVE(BRAP_CITGEN_P_PortConfiguration);		

	return(ui32Error);
}


/******************************************************************************
Summary:
			The I2S Stereo (I2s0) configuration
	
			BCM 7550 doesnot has I2S ports
Description:
			Configuration of Master clock frequency.	
	  
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_I2SCfg(
				BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG			*psFmmDestI2SClkCfg,
				uint32_t								ui32TaskRdbOffset
			)
{
	uint32_t ui32Error;

	#if (BCHP_CHIP == 7550)

		/* Avoiding warnings */
		ui32Error = psFmmDestI2SClkCfg->ui32I2SEnabled;
		ui32Error = ui32TaskRdbOffset;

	#else

		ui32Error = BERR_SUCCESS;

		/* Stereo I2S */
		psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr 
			= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_STEREO0 + ui32TaskRdbOffset;

	#endif
}
/******************************************************************************
Summary:
			The I2S Muti Channel (I2s Multi0) configuration

			BCM 7550 doesnot has I2S ports but clubbed with MAI multi

Description:
			Configuration of Master clock frequency.	
	  
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_I2S_MultiCfg(
				BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG			*psFmmDestI2SClkCfg,
				uint32_t								ui32TaskRdbOffset
			)
{	
	/* MultiChannel I2S */	
	#if ((BRAP_7420_FAMILY == 1) || (BCHP_CHIP == 7550))

		psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr 
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI		+ ui32TaskRdbOffset;		

	#else		

		psFmmDestI2SClkCfg->ui32AudioI2SClkMacroRegAddr 
			= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0	+ ui32TaskRdbOffset;

	#endif	
}

/******************************************************************************
Summary:
		The NCO configuration

Description:
		This function configures the NCO.
		NCo should be configured only for 7002 Family chips.
Input :
		ui32TaskRdbOffset

Output	:
		
		sFmmDestNcoCfg			
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_NcoCfg(
						BRAP_AF_P_sFMM_DEST_NCO_CFG		*psFmmDestNcoCfg,						
						uint32_t						ui32TaskRdbOffset
					)
{

	uint32_t ui32Error;

	BDBG_ENTER(BRAP_CITGEN_P_NcoCfg);	

	#if (BCHP_CHIP == 7550)

		ui32Error = BERR_SUCCESS;

		psFmmDestNcoCfg->ui32NcoPhaseIncRegAddr		= 
						BCHP_AUD_FMM_OP_SPDIF_NCO_PHASE_INC  + ui32TaskRdbOffset;

		psFmmDestNcoCfg->ui32NcoRateRatioRegAddr	= 
						BCHP_AUD_FMM_OP_SPDIF_NCO_RATE_RATIO  + ui32TaskRdbOffset;

		psFmmDestNcoCfg->ui32NcoSampleIncRegAddr	= 
						BCHP_AUD_FMM_OP_SPDIF_NCO_SAMPLE_INC  + ui32TaskRdbOffset;

	#else

		ui32Error = psFmmDestNcoCfg->ui32NcoPhaseIncRegAddr;
		ui32Error = ui32TaskRdbOffset;

		ui32Error = BERR_SUCCESS;

	#endif

	BDBG_LEAVE(BRAP_CITGEN_P_NcoCfg);	
	
	return ui32Error;

}
/******************************************************************************
Summary:
		The PLL configuration

Description:
		This function configures the PLL.
		We support only PLL0 and PLL1

Input : ui32PllIndex,
		ui32TaskRdbOffset

Output	:
		
		sFmmDestPllCfg
			
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_PllCfg(
						BRAP_AF_P_sFMM_DEST_PLL_CFG		*psFmmDestPllCfg,
						uint32_t						ui32PllIndex,
						uint32_t						ui32TaskRdbOffset
					)
{

	uint32_t ui32Error;
	
	BDBG_ENTER(BRAP_CITGEN_P_PllCfg);	

	#if(BCHP_CHIP == 7550)

		/*To Remove warnings */

		ui32Error = psFmmDestPllCfg->ui32PllEnabled;
		ui32Error = ui32PllIndex;
		ui32Error = ui32TaskRdbOffset;
		ui32Error = BERR_SUCCESS;

	#else

		ui32Error = BERR_SUCCESS;

		if(ui32PllIndex == 0)	/* PLL macro definition*/
		{
			psFmmDestPllCfg->ui32AudioPllMacroRegAddr 
						= BRAP_CIT_AUD_FMM_PLL0_MACRO + ui32TaskRdbOffset;
		}
#if (BRAP_PLL1_SUPPORTED == 1)        
		else if(ui32PllIndex == 1)
		{
			psFmmDestPllCfg->ui32AudioPllMacroRegAddr 
						= BRAP_CIT_AUD_FMM_PLL1_MACRO + ui32TaskRdbOffset;
		}
#endif        
		else
		{
			/* Need to Check the Input: Valid*/
			BDBG_ERR((" Unsupported PLL index = [%d] associated with a port",psFmmDestPllCfg->ui32PllIndex));
			psFmmDestPllCfg->ui32AudioPllMacroRegAddr = (uint32_t)NULL;	
			ui32Error = BERR_NOT_SUPPORTED;
		}
	#endif

	BDBG_LEAVE(BRAP_CITGEN_P_PllCfg);	
	
	return ui32Error;
}

/******************************************************************************
Summary:
		CapPort (0-3) configuration.
		This is a generic Capture port configuration function for all 4 Capture
		ports.
		
Description:
	

Input : 
		ui32CapPortIndex  :	Capture port index
		ui32TaskRdbOffset : RDB Offset

Output :
		psFmmDestI2SClkCfg	: Clock programing register
		Configuration of Master clock frequency.

Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_CapPortNCfg(
				BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG	*psFmmDestI2SClkCfg,
				uint32_t								ui32TaskRdbOffset,
				uint32_t								ui32CapPortIndex
			)
{
	uint32_t ui32Error;

	ui32Error = BERR_SUCCESS;


	psFmmDestI2SClkCfg->ui32AudioCapPortClkMacroRegAddr
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FSi_ARRAY_BASE + \
								ui32CapPortIndex*4 + ui32TaskRdbOffset;
}
/******************************************************************************
Summary:
			The SPDIF0 FW-Cbits configuration

Description:
			Configuration of Master clock frequency, Ping pong address and
			CBIT Registers.

			This SPDIF Configuration is for  FWCBITS..
			
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_Spdif0FwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg,
			uint32_t									ui32SpdifCfgDramBufferAddress,
			uint32_t									ui32SpdifContent,
			uint32_t									ui32TaskRdbOffset
		)
{
	BDBG_ENTER(BRAP_CITGEN_P_Spdif0FwCbitsCfg);	

	psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
			= BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0	+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr	
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0	+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg
			= BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0	+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr						
			= BCHP_AUD_FMM_MS_CTRL_FW_CBITS0		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr
			= BCHP_AUD_FMM_MS_CTRL_FW_CBITS12		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32PingPongMask						
			= BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG0_MASK;/* Problem*/


	/* Spdif dram Address is done before*/
	psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr
			= ui32SpdifCfgDramBufferAddress;
			
	/* Spdif Contents*/
	psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif
			= ui32SpdifContent;

	BDBG_LEAVE(BRAP_CITGEN_P_Spdif0FwCbitsCfg);		
}

/******************************************************************************
Summary:
			The SPDIF0 HW-Cbits configuration

Description:
			Configuration of Master clock frequency, Ping pong address and
			CBIT Registers.

			This SPDIF Configuration is for  HW-CBITS..

			We do not have HW CBITS for 3548 family chips and 
													7405/7325[A0/A1 versions]
			
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_Spdif0HwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg,
			uint32_t									ui32SpdifCfgDramBufferAddress,
			uint32_t									ui32SpdifContent,
			uint32_t									ui32TaskRdbOffset
		)
{

	/*Declared to Avoid warning*/
	uint32_t	ui32Error;
	ui32Error = BERR_SUCCESS;

	BDBG_ENTER(BRAP_CITGEN_P_Spdif0HwCbitsCfg);	

#if ((((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325)) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) ||(BRAP_3548_FAMILY == 1))
	
	/* Hardware CBITs are not defined */

	/*To avoid warning*/
	ui32Error = psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr;
	ui32Error = ui32SpdifCfgDramBufferAddress;
	ui32Error = ui32SpdifContent;
	ui32Error = ui32TaskRdbOffset;

#else


	psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
			= BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_0			+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr	
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0			+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg
			= BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_0			+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr						
			= BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr
			= BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_0		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32PingPongMask						
			= BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG0_MASK;

	/* Spdif dram Address is done before*/
	psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr
			= ui32SpdifCfgDramBufferAddress;
			
	/* Spdif Contents*/
	psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif
			= ui32SpdifContent;
#endif

	BDBG_LEAVE(BRAP_CITGEN_P_Spdif0HwCbitsCfg);		

}

/******************************************************************************
Summary:
			The SPDIF1 configuration for HBR

Description:
			Configuration of Master clock frequency, Ping pong address and
			Hardware CBIT Registers
			
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_Spdif1HwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg,
			uint32_t									ui32SpdifCfgDramBufferAddress,
			uint32_t									ui32SpdifContent,
			uint32_t									ui32TaskRdbOffset
		)
{
	/*Declared to Avoid warning*/
	uint32_t	ui32Error;
	ui32Error = BERR_SUCCESS;

	BDBG_ENTER(BRAP_CITGEN_P_Spdif1HwCbitsCfg);		

#if ((((BCHP_CHIP == 7405)||(BCHP_CHIP == 7325)) && ((BCHP_VER == BCHP_VER_A0) || (BCHP_VER == BCHP_VER_A1))) ||(BRAP_3548_FAMILY == 1))
	
	/* Hardware CBITs are not defined */

	/*To avoid warning*/
	ui32Error = psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr;
	ui32Error = ui32SpdifCfgDramBufferAddress;
	ui32Error = ui32SpdifContent;
	ui32Error = ui32TaskRdbOffset;

#else

	/* Master Clock Cfg */
	#if (BRAP_7420_FAMILY == 1)				

		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr 
				= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI		+ ui32TaskRdbOffset;			  

	#elif (BCHP_CHIP == 7550)

		/* Master Clock Cfg */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI			+ ui32TaskRdbOffset;
	
	#else

		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr 
				= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0		+ ui32TaskRdbOffset;			  

	#endif

		/*Spdif Hw Spdif Cfg */
		psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
				= BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1			+ ui32TaskRdbOffset; 

		/* CBIT cfg */
		psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg 
				= BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1			+ ui32TaskRdbOffset;

		/* Ping pong Address*/
		psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr
				= BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1		+ ui32TaskRdbOffset;
				  

		psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr
				= BRAP_CIT_AUD_FMM_MS_CTRL_HW_CHANSTAT_LO_1		+ ui32TaskRdbOffset;

		/* The ping pong mask is done before*/
		psFmmDestSpdifClkCbitCfg->ui32PingPongMask
				= BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK;
				  

		/* Spdif dram Address is done before*/
		psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr
				= ui32SpdifCfgDramBufferAddress;
				
		/* Spdif Contents*/
		psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif 
				= ui32SpdifContent;

#endif

	BDBG_LEAVE(BRAP_CITGEN_P_Spdif1HwCbitsCfg);		

}


/******************************************************************************
Summary:
			The SPDIF1 configuration for Normal operation

Description:
			Configuration of Master clock frequency, Ping pong address and
			Firmware CBIT Registers
			
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_Spdif1FwCbitsCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg,
			uint32_t									ui32SpdifCfgDramBufferAddress,
			uint32_t									ui32SpdifContent,
			uint32_t									ui32TaskRdbOffset
		)
{

	BDBG_ENTER(BRAP_CITGEN_P_Spdif1FwCbitsCfg);		

	#if ((BRAP_3548_FAMILY == 1))					
	
		/* Master Clock Cfg as I2S Multi */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr 
			= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0	+ ui32TaskRdbOffset;

	#elif (BRAP_7420_FAMILY == 1)

		/* Master Clock Cfg */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI		+ ui32TaskRdbOffset;

	#elif (BCHP_CHIP == 7550)

		/* Master Clock Cfg */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI		+ ui32TaskRdbOffset;
	
	#else

		/* Master Clock Cfg */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr
			= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_HBR0		+ ui32TaskRdbOffset;
	#endif


	/*Spdif Hw Spdif Cfg */
	psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
		= BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1	+ ui32TaskRdbOffset; 

	/* CBIT cfg */
	psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg
		= BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1	+ ui32TaskRdbOffset;

	/* Ping pong Address*/
	psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr
		= BCHP_AUD_FMM_MS_CTRL_FW_CBITS24		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr
		= BCHP_AUD_FMM_MS_CTRL_FW_CBITS36		+ ui32TaskRdbOffset;

	/* The ping pong mask is done before*/
	psFmmDestSpdifClkCbitCfg->ui32PingPongMask
		= BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK;/* Problem*/

	/* Spdif dram Address is done before*/
	psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr
		= ui32SpdifCfgDramBufferAddress;
			
	/* Spdif Contents*/
	psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif 
		= ui32SpdifContent;


	BDBG_LEAVE(BRAP_CITGEN_P_Spdif1FwCbitsCfg);
}
/******************************************************************************
Summary:
			The MAI port configuration

Description:
			Mai port configuration for all its input MuxSelector ports
			
  MAI as MAI port :
		We consider the programing for HBR mode and in Normal mode.

		In HBR mode, we expect DDP PT to be present and HWCBIT programming is required
		in SPDIF1 configuration. Here HBR enable flag is also enabled by default.

		In Normal mode, we can configure Spdif1 with FwCBits and HwCbit modes.
		The decision is made based on the PI flag 'bHwCbits' 

  MAI as bus for SPDIF port:

		Here we donot expect Spdif0 to be in HBR mode or in HWCBIT modes...
			

			
Returns:
			None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_MaiPortConfiguration(
						BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg,
						BRAP_CIT_P_sMaiPortInfo		*psMaiPortInfo,
						BRAP_AF_P_sDRAM_BUFFER		*psTaskSPDIFPortConfigAddr,
						BRAP_AF_P_FmmDstFsRate		*peHdmiFsRate,
						uint32_t					*pui32HwCbitsEnable,
						uint32_t					ui32DDP_PassThruPresent,
						uint32_t					ui32TaskRdbOffset
					)
{	
	uint32_t									ui32Error;
	uint32_t									ui32PortIndex;
	uint32_t									ui32SpdifContent;
	uint32_t									ui32SpdifCfgDramBufferAddress;
	BRAP_OutputPort								eOutputPort;	
	BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg;


	BDBG_LEAVE(BRAP_CITGEN_P_MaiPortConfiguration);	


	ui32Error		=	BERR_SUCCESS;

	eOutputPort		=	psMaiPortInfo->sMaiCfg.eMaiMuxSelector;	

	ui32PortIndex	=	0;

	switch(eOutputPort)
	{			
		/*I2S Stream*/
		case	BRAP_OutputPort_eI2s0:
		case	BRAP_OutputPort_eI2s1:
		case	BRAP_OutputPort_eI2s2:
		case	BRAP_OutputPort_eI2s3:		
		case	BRAP_OutputPort_eI2s4:		
		case	BRAP_OutputPort_eI2s5:
		case	BRAP_OutputPort_eI2s6:
		case	BRAP_OutputPort_eI2s7:
		case	BRAP_OutputPort_eI2s8:
		case    BRAP_OutputPort_eMaiMulti0:           
        case    BRAP_OutputPort_eMaiMulti1:
        case    BRAP_OutputPort_eMaiMulti2:
        case    BRAP_OutputPort_eMaiMulti3: 

				ui32PortIndex = 1;


#ifdef OLD_MAI_I2S_CODE

				/* Enabling I2S1 port*/
				psFmmDestCfg->sFmmDestI2SClkCfg[ui32PortIndex].ui32I2SEnabled		/*I2S-1 is configured for FW*/
							= (uint32_t)BRAP_AF_P_PortEnable;

				/* Configuring the I2S Multi0 */
				BRAP_CITGEN_P_I2S_MultiCfg(
								&psFmmDestCfg->sFmmDestI2SClkCfg[ui32PortIndex],
								ui32TaskRdbOffset
							);
#endif

				/*
				THIS CONFIGURATION IS NEEDED, SINCE I2S ON MAI needs CBIT cfg.

				Since CIT-FW interface doesnot has the CBIT configuration for I2S,
				in this mode, CIT tells FW as SPDIF1 as data content as PCM
				*/

				psFmmDestSpdifClkCbitCfg
							= &psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[ui32PortIndex];

				/* Enabling Spdif1 port*/
				psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled 
						= (uint32_t)BRAP_AF_P_PortEnable;	
				
				/* Fs Rate */
				psFmmDestSpdifClkCbitCfg->eFmmDstFsRate 
						= psMaiPortInfo->sMaiPortControlInfo.eFmmDstFsRate;
		
				

				/* SPDIF Data type*/
				ui32SpdifContent = (uint32_t)BRAP_AF_P_PcmOnSpdif;

				/* SPDIF Dram Cfg Address*/
				ui32SpdifCfgDramBufferAddress = 
							psTaskSPDIFPortConfigAddr[ui32PortIndex].ui32DramBufferAddress;


				
				BRAP_CITGEN_P_I2SOnMai_Spdif1NormalCfg(	
											psFmmDestSpdifClkCbitCfg,
											ui32SpdifCfgDramBufferAddress,
											ui32SpdifContent,
											ui32TaskRdbOffset
										);				
				
				break;

		case	BRAP_OutputPort_eSpdif:
			
				/* Spdif ESR Configuration is done before SPDIF port configuration*/
				
				ui32PortIndex = 0;

				psFmmDestSpdifClkCbitCfg
						= &psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[ui32PortIndex];


				/* Enabling Spdif0 port*/
				psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled 
								= (uint32_t)BRAP_AF_P_PortEnable;

				/* Fs Rate */
				psFmmDestSpdifClkCbitCfg->eFmmDstFsRate 
						= psMaiPortInfo->sMaiPortControlInfo.eFmmDstFsRate;

				/* SPDIF Data type*/
				ui32SpdifContent =							
						 (psMaiPortInfo->sMaiCfg.sMaiContent.bPcmOnSpdif)
						  ? (uint32_t)BRAP_AF_P_CompressedOnSpdif : (uint32_t)BRAP_AF_P_PcmOnSpdif;


				/* SPDIF Dram Cfg Address*/
				ui32SpdifCfgDramBufferAddress = 
							psTaskSPDIFPortConfigAddr[ui32PortIndex].ui32DramBufferAddress;


				/* Updating the Global HBR flag */	
				if(BRAP_CITGEN_P_True == psMaiPortInfo->sMaiCfg.sMaiContent.bHbr)
				{							
					/**pui32HbrEnable = BRAP_AF_P_PortEnable;	*/
					*peHdmiFsRate = BRAP_AF_P_FmmDstFsRate_e4xBaseRate;
				}	

				/*SPDIF configuration*/
				if(BRAP_CITGEN_P_False == psMaiPortInfo->sMaiCfg.sMaiContent.bHwCbits)
				{					
					/* SPDIF 0 FWCbit Configuration*/
					BRAP_CITGEN_P_Spdif0FwCbitsCfg(
										psFmmDestSpdifClkCbitCfg,
										ui32SpdifCfgDramBufferAddress,
										ui32SpdifContent,
										ui32TaskRdbOffset
									);	
					/*Global HWCBIT flag*/
					psFmmDestCfg->ui32HwCbitsEnable = BRAP_AF_P_PortDisable;
				
				}
				else 
				{
					/* SPDIF 0 HWCbit Configuration*/
					BRAP_CITGEN_P_Spdif0HwCbitsCfg(
										psFmmDestSpdifClkCbitCfg,
										ui32SpdifCfgDramBufferAddress,
										ui32SpdifContent,
										ui32TaskRdbOffset
									);	

					/*Global HWCBIT flag*/
					psFmmDestCfg->ui32HwCbitsEnable = BRAP_AF_P_PortEnable;
				}


				break;

		case	BRAP_OutputPort_eMai:

					/* Spdif ESR Configuration is done before SPDIF port configuration*/

					ui32PortIndex = 1;

					psFmmDestSpdifClkCbitCfg
							= &psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[ui32PortIndex];

					/* Enabling Spdif1 port*/
					psFmmDestSpdifClkCbitCfg->ui32SpdifEnabled 
							= (uint32_t)BRAP_AF_P_PortEnable;		
					
					/* Fs Rate */
					psFmmDestSpdifClkCbitCfg->eFmmDstFsRate 
						= psMaiPortInfo->sMaiPortControlInfo.eFmmDstFsRate;
					

					/* SPDIF Data type*/
					ui32SpdifContent
							=  (psMaiPortInfo->sMaiCfg.sMaiContent.bPcmOnSpdif)
								? (uint32_t)BRAP_AF_P_CompressedOnSpdif : (uint32_t)BRAP_AF_P_PcmOnSpdif;

					/* SPDIF Dram Cfg Address*/
					ui32SpdifCfgDramBufferAddress = 
								psTaskSPDIFPortConfigAddr[ui32PortIndex].ui32DramBufferAddress;


					/* Updating the Global HBR flag */	
					if(BRAP_CITGEN_P_True == psMaiPortInfo->sMaiCfg.sMaiContent.bHbr)
					{							

						/* Here this should be: If HBR Flag is on then do the folowing,*/
						if(ui32DDP_PassThruPresent == BRAP_CIT_P_PRESENT)
						{					
							/**pui32HbrEnable = BRAP_AF_P_PortEnable;	*/
							*peHdmiFsRate = BRAP_AF_P_FmmDstFsRate_e4xBaseRate;
						}
						else
						{
							BDBG_ERR((" Error: MAI is  Configured in HBR Mode when DDP PassThrough is absent in network "));
							ui32Error = BERR_NOT_SUPPORTED;
							goto BRAP_CITGEN_MAI_PORT_CFG_P_EXIT_POINT;
						}
					}					
					
					if(BRAP_CITGEN_P_False == psMaiPortInfo->sMaiCfg.sMaiContent.bHwCbits)
					{
						/* Spdif1 is configured for the Normal Operation with FW Cbits */
						BRAP_CITGEN_P_Spdif1FwCbitsCfg(
										psFmmDestSpdifClkCbitCfg,
										ui32SpdifCfgDramBufferAddress,
										ui32SpdifContent,
										ui32TaskRdbOffset
									);	

						*pui32HwCbitsEnable = BRAP_AF_P_PortDisable;	
					}
					else
					{						
						/* Spdif1 is configured for the Hardware CIT programming */
						BRAP_CITGEN_P_Spdif1HwCbitsCfg(
										psFmmDestSpdifClkCbitCfg,
										ui32SpdifCfgDramBufferAddress,
										ui32SpdifContent,
										ui32TaskRdbOffset
									);						

						*pui32HwCbitsEnable = BRAP_AF_P_PortEnable;	
					}
							

				break;
		default:

				ui32Error = BERR_NOT_SUPPORTED;
				BDBG_ERR(("ERROR : The input Mai stream type [%s] is not supported",PortName[(uint16_t)eOutputPort]));
				break;

	}


	BRAP_CITGEN_MAI_PORT_CFG_P_EXIT_POINT:

	BDBG_LEAVE(BRAP_CITGEN_P_MaiPortConfiguration);	

	return ui32Error;
}

/******************************************************************************
Summary:
	The DAC Configuration

Description: 	
			
		For BRAP_3548_FAMILY : DAC0 DAC1 and DAC2 available
Input:			
	psPllPortAssociation	:
	ui32TaskRdbOffset :

Output:		
	  psFmmDestCfg:
Returns:
		None

******************************************************************************/

#if(BRAP_3548_FAMILY == 1)

static uint32_t BRAP_CITGEN_P_DAC_Configuration(
							BRAP_AF_P_sFMM_DEST_CFG				*psFmmDestCfg,
							BRAP_CIT_P_sPllPortAssociation		*psPllPortAssociation,
							uint32_t							ui32TaskRdbOffset
						)
{
	uint32_t						ui32Count;
	uint32_t						ui32Error;
	BRAP_AF_P_sFMM_DEST_DAC_CFG		*psFmmDestDacCfg;

	BDBG_ENTER(BRAP_CITGEN_P_DAC_Configuration);	

	ui32Error = BERR_SUCCESS;

	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_DAC_PORTS;ui32Count++)
	{

		psFmmDestDacCfg = &psFmmDestCfg->sFmmDestDacCfg[ui32Count];

		psFmmDestDacCfg->ui32DacEnabled 
						= psPllPortAssociation->sDacPortInfo[ui32Count].ui32PortEnabled;	/* Input. */

		if(psFmmDestDacCfg->ui32DacEnabled == (uint32_t)BRAP_AF_P_PortEnable)
		{

			/* new code*/
			/* Getting the Fs rate */
			psFmmDestDacCfg->eFmmDstFsRate
						= psPllPortAssociation->sDacPortInfo[ui32Count].eFmmDstFsRate;

			if(ui32Count == 0)
			{
				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM0_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM0_RATE_RATIO + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM0_SAMPLE_INC + ui32TaskRdbOffset;	

				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL0_RATEMGRCFG + ui32TaskRdbOffset;	
			}
			else if(ui32Count == 1)
			{

				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM1_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM1_RATE_RATIO + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM1_SAMPLE_INC + ui32TaskRdbOffset;	

				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL1_RATEMGRCFG + ui32TaskRdbOffset;	

			}
			else if(ui32Count == 2)
			{
				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM2_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM2_RATE_RATIO + ui32TaskRdbOffset;
								   

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM2_SAMPLE_INC + ui32TaskRdbOffset;	

				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL2_RATEMGRCFG + ui32TaskRdbOffset;	

			}
			else
			{
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;			
				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;			

				ui32Error = BERR_NOT_SUPPORTED;

				BDBG_ERR(("ERROR : Unable to support HIFIDAC [%s]",ui32Count ));
				goto BRAP_CITGEN_DAC_CFG_P_EXIT_POINT;

			}
		}
		else
		{
			psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;
		}
	}

	BRAP_CITGEN_DAC_CFG_P_EXIT_POINT:


	BDBG_LEAVE(BRAP_CITGEN_P_DAC_Configuration);		

	return(ui32Error);

}

#elif ((BCHP_CHIP == 7420))

/******************************************************************************
Summary:
	The DAC Configuration

Description:		7420 : There are DAC-0 DAC-1 Available
	  
Returns:
		None

******************************************************************************/

static uint32_t BRAP_CITGEN_P_DAC_Configuration(
							BRAP_AF_P_sFMM_DEST_CFG				*psFmmDestCfg,
							BRAP_CIT_P_sPllPortAssociation		*psPllPortAssociation,
							uint32_t							ui32TaskRdbOffset
						)
{

	uint32_t						ui32Count;
	uint32_t						ui32Error;
	BRAP_AF_P_sFMM_DEST_DAC_CFG		*psFmmDestDacCfg;

	BDBG_ENTER(BRAP_CITGEN_P_DAC_Configuration);	

	ui32Error = BERR_SUCCESS;

	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_DAC_PORTS;ui32Count++)
	{

		psFmmDestDacCfg = &psFmmDestCfg->sFmmDestDacCfg[ui32Count];

#ifdef OLD_PORT_CODE
		psFmmDestDacCfg->ui32DacEnabled = psPllPortAssociation->ui32DacEnabled[ui32Count];	/* Input. */
#endif
		/* new code*/
		psFmmDestDacCfg->ui32DacEnabled 
						= psPllPortAssociation->sDacPortInfo[ui32Count].ui32PortEnabled;

		if(psFmmDestDacCfg->ui32DacEnabled == (uint32_t)BRAP_AF_P_PortEnable)
		{

			/* new code*/
			/* Getting the Fs rate */
			psFmmDestDacCfg->eFmmDstFsRate
						= psPllPortAssociation->sDacPortInfo[ui32Count].eFmmDstFsRate;

			if(ui32Count == 0)
			{
				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM0_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM0_RATE_RATIO + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM0_SAMPLE_INC + ui32TaskRdbOffset;	
				
				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL0_RATEMGRCFG + ui32TaskRdbOffset;
			}
			else if(ui32Count == 1)
			{
				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM1_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM1_RATE_RATIO + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM1_SAMPLE_INC + ui32TaskRdbOffset;	

				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL1_RATEMGRCFG + ui32TaskRdbOffset;

			}			
			else
			{
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;			
				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;

				ui32Error = BERR_NOT_SUPPORTED;

				BDBG_ERR(("ERROR : Unable to support HIFIDAC [%d]",ui32Count ));
				goto BRAP_CITGEN_DAC_CFG_P_EXIT_POINT;

			}
		}
		else
		{
			psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;
		}
	}

	BRAP_CITGEN_DAC_CFG_P_EXIT_POINT:


	BDBG_LEAVE(BRAP_CITGEN_P_DAC_Configuration);		

	return(ui32Error);
}


#else
/******************************************************************************
Summary:
	The DAC Configuration

Description:	7335/7405/7325
				Only DAC0 available
	  
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_DAC_Configuration(
							BRAP_AF_P_sFMM_DEST_CFG				*psFmmDestCfg,
							BRAP_CIT_P_sPllPortAssociation		*psPllPortAssociation,
							uint32_t							ui32TaskRdbOffset
						)
{
	uint32_t						ui32Count;
	uint32_t						ui32Error;
	BRAP_AF_P_sFMM_DEST_DAC_CFG		*psFmmDestDacCfg;

	BDBG_ENTER(BRAP_CITGEN_P_DAC_Configuration);	

	ui32Error = BERR_SUCCESS;

	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_DAC_PORTS;ui32Count++)
	{
		psFmmDestDacCfg = &psFmmDestCfg->sFmmDestDacCfg[ui32Count];

		psFmmDestDacCfg->ui32DacEnabled = psPllPortAssociation->sDacPortInfo[ui32Count].ui32PortEnabled;	/* Input. */

		if(psFmmDestDacCfg->ui32DacEnabled == (uint32_t)BRAP_AF_P_PortEnable)
		{
			/* new code*/
			/* Getting the Fs rate */
			psFmmDestDacCfg->eFmmDstFsRate
						= psPllPortAssociation->sDacPortInfo[ui32Count].eFmmDstFsRate;

			if(ui32Count == 0)
			{
				/* DAC Derives the Clock frequency from 27MHz STC */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  
							= BCHP_HIFIDAC_RM0_PHASE_INC  + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr
							= BCHP_HIFIDAC_RM0_RATE_RATIO + ui32TaskRdbOffset;

				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr 
							= BCHP_HIFIDAC_RM0_SAMPLE_INC + ui32TaskRdbOffset;	

				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr
							= BCHP_HIFIDAC_CTRL0_RATEMGRCFG + ui32TaskRdbOffset; 
			}
			else
			{
				/*Range / Valid Check is needed */
				psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;
				psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;

				ui32Error = BERR_NOT_SUPPORTED;

				BDBG_ERR(("ERROR : Unable to support HIFIDAC [%d]",ui32Count ));				
				goto BRAP_CITGEN_DAC_CFG_P_EXIT_POINT;
			}
		}
		else
		{
			psFmmDestDacCfg->ui32HifidacRmPhaseIncRegAddr  = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmRateRatioRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRmSampleIncRegAddr = (uint32_t)NULL;
			psFmmDestDacCfg->ui32HifidacRateManagerCfgAddr = (uint32_t)NULL;
		}
	}

	BRAP_CITGEN_DAC_CFG_P_EXIT_POINT:

	BDBG_LEAVE(BRAP_CITGEN_P_DAC_Configuration);		

	return(ui32Error);
}

#endif


/******************************************************************************
Summary:
			This function does SPDIF 1 Configuration when
			MAI is a router for I2S port.
			Here in this case we need to program CBIT.
			As we donot have interface for this mode (CBIT for I2S) in CIT-FW
			side, CIT is forced to tell FW in follwing mode 
					1) SPDIF 1
					2) CLOCK program is for I2S MULTI
					3) Data Content PCM

Description:
			Configuration of Master clock frequency, Ping pong address and
			Firmware CBIT Registers
			
Returns:
			None

******************************************************************************/
static void BRAP_CITGEN_P_I2SOnMai_Spdif1NormalCfg(
			BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG		*psFmmDestSpdifClkCbitCfg,
			uint32_t									ui32SpdifCfgDramBufferAddress,
			uint32_t									ui32SpdifContent,
			uint32_t									ui32TaskRdbOffset
		)
{

	BDBG_ENTER(BRAP_CITGEN_P_I2SOnMai_Spdif1NormalCfg);		

	#if ((BRAP_7420_FAMILY == 1) || (BCHP_CHIP == 7550))

		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr 
			= BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_MAI_MULTI + ui32TaskRdbOffset;
		
	#else
		/* Master Clock Cfg  as I2S Multi */
		psFmmDestSpdifClkCbitCfg->ui32AudioSpdifClkMacroRegAddr 
			= BRAP_CIT_AUD_FMM_OP_CTRL_MCLK_CFG_I2S_MULTI0 + ui32TaskRdbOffset;
	#endif

	/*Spdif Hw Spdif Cfg */
	psFmmDestSpdifClkCbitCfg->ui32AudFmmMsCtrlHwSpdifCfg
		= BRAP_CIT_AUD_FMM_MS_CTRL_HW_SPDIF_CFG_1	+ ui32TaskRdbOffset; 
	
	/* CBIT cfg */
	psFmmDestSpdifClkCbitCfg->ui32SpdifCbitCtrlReg
		= BCHP_AUD_FMM_MS_CTRL_FW_CBIT_CTRL_1	+ ui32TaskRdbOffset;

	/* Ping pong Address*/
	psFmmDestSpdifClkCbitCfg->ui32CbitBasePingAddr
		= BCHP_AUD_FMM_MS_CTRL_FW_CBITS24		+ ui32TaskRdbOffset;

	psFmmDestSpdifClkCbitCfg->ui32CbitBasePongAddr
		= BCHP_AUD_FMM_MS_CTRL_FW_CBITS36		+ ui32TaskRdbOffset;

	/* The ping pong mask is done before*/
	psFmmDestSpdifClkCbitCfg->ui32PingPongMask
		= BCHP_AUD_FMM_MS_ESR_STATUS_CBIT_PING_PONG1_MASK;/* Problem*/

	/* Spdif dram Address is done before*/
	psFmmDestSpdifClkCbitCfg->ui32SpdifDramConfigPtr
		= ui32SpdifCfgDramBufferAddress;
			
	/* Spdif Contents*/
	psFmmDestSpdifClkCbitCfg->ui32PcmOnSpdif 
		= ui32SpdifContent;


	BDBG_LEAVE(BRAP_CITGEN_P_I2SOnMai_Spdif1NormalCfg);


}

/******************************************************************************
Summary:
	Calculate the Scheduling the Thresholds for different

Description: 
	Scheduling the Thresholds for different
			
Input:		
	psCitDataStructure	: CIT Global Structure

Output:	
	pui32Threshold		:
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_CalcThreshold(				
				BRAP_CIT_P_sCitDataStructure *psCitDataStructure,
				uint32_t					 *pui32Threshold
			)
{

	uint32_t ui32Error;
	uint32_t ui32BranchId;
	uint32_t ui32StageId;	
	uint32_t ui32StageNumber;	

	BRAP_CIT_P_StageInfo	  *psStageInfo;
	BRAP_CIT_P_AudioAlgoType  eAudioAlgoType;
	BRAP_DSPCHN_AudioType	  eDecAudioType;
	BRAP_CIT_P_EncAudioType   eEncAudioType;

	ui32Error		 = BERR_SUCCESS;
	ui32StageNumber  = 0;

	/* Get the Branch ID and Stage Id for the first stage of execution*/
	ui32BranchId = psCitDataStructure->sAlgoExecOrder[ui32StageNumber].ui32BranchId;
	ui32StageId  = psCitDataStructure->sAlgoExecOrder[ui32StageNumber].ui32StageId;


	/* Selecting the first stage for execution */
	psStageInfo = &psCitDataStructure->sAlgoNwInfo.psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];

	/* Getting the AudioAlgo Type*/
	eAudioAlgoType = psStageInfo->eAudioAlgoType;
	

	/* Decode Type*/
	if(BRAP_CIT_P_AudioAlgoType_eDecode == eAudioAlgoType)
	{
		eDecAudioType = psStageInfo->uAlgorithm.eDecAudioAlgo;	
	

		switch(eDecAudioType)
		{
			case BRAP_DSPCHN_AudioType_eMpeg:
				*pui32Threshold = (uint32_t)2048;
				break;

			case BRAP_DSPCHN_AudioType_eWmaStd:					
			case BRAP_DSPCHN_AudioType_eWmaPro:

				*pui32Threshold = (uint32_t)3008;
				break;

			case BRAP_DSPCHN_AudioType_eAc3:
			case BRAP_DSPCHN_AudioType_eAc3Plus:			
				*pui32Threshold = (uint32_t)3008;
				break;

			default:				
				*pui32Threshold = (uint32_t)3008;
				break;
		}
	}
	/* Encode Type*/
	else if(BRAP_CIT_P_AudioAlgoType_eEncode == eAudioAlgoType)
	{
		eEncAudioType = psStageInfo->uAlgorithm.eEncAudioAlgo;

		*pui32Threshold = (uint32_t)2048;

		switch(eEncAudioType)
		{
			case BRAP_CIT_P_EncAudioType_eDTS:
				break;

			case BRAP_CIT_P_EncAudioType_eDTSBroadcast:
				break;

			case BRAP_CIT_P_EncAudioType_eAacLc:
				break;

			case BRAP_CIT_P_EncAudioType_eAacHe:
				break;

			case BRAP_CIT_P_EncAudioType_eAc3:
				break;

			case BRAP_CIT_P_EncAudioType_eMpeg1Layer2:
				break;

			case BRAP_CIT_P_EncAudioType_eMpeg1Layer3:
				break;

			default:
				break;
		}
	}
	else if((BRAP_CIT_P_GfxAlgoType_eDecode == eAudioAlgoType) || (BRAP_CIT_P_VideoAlgoType_eDecode == eAudioAlgoType))
	{
		*pui32Threshold = (uint32_t)0; /* Just for defaulting */
	}
	else
	{
		/* Error */
		/* TIGI CHANGE
		ui32Error = BERR_INVALID_PARAMETER;
		BDBG_ERR((" CIT Error : The first execution stage is neither Decoder nor Encoder"));


		*/
	}

	return (ui32Error);
}

/******************************************************************************
Summary:
	Gets Free interstage buffer available

Description: 
	Gets Free interstage buffer available.
			
Input:	
		psTaskIsBuffCfg:
		ui32NumInterStgBuffs:
Output:	
		pui32IsBufferCount

Returns:
		Error

******************************************************************************/

static uint32_t BRAP_CITGEN_P_GenFreeInterstageBuffer(
							BRAP_CIT_P_TaskIsBuffCfg	*psTaskIsBuffCfg,
							uint32_t					ui32NumInterStgBuffs,
							uint32_t					*pui32IsBufferCount
						)
{
	uint16_t ui16IsBufferCount;
	uint32_t ui32Error;

	BDBG_ENTER(BRAP_CITGEN_P_GenFreeInterstageBuffer);	

	ui32Error  = BERR_SUCCESS;
	for(ui16IsBufferCount =0; ui16IsBufferCount<ui32NumInterStgBuffs;ui16IsBufferCount++)
	{
		
		if((psTaskIsBuffCfg->sIsBuffCfg[ui16IsBufferCount].ui32PortAssociated	!= BRAP_CIT_P_ASSOCIATED) &&
			(psTaskIsBuffCfg->sIsBuffCfg[ui16IsBufferCount].ui32StageAssociated	!= BRAP_CIT_P_ASSOCIATED)
		  )
		{
			break;
		}
	}

	if(ui16IsBufferCount >= ui32NumInterStgBuffs)
	{
		ui32Error = BRAP_CIT_P_ERROR;
	}
	

	*pui32IsBufferCount =  (uint32_t)ui16IsBufferCount;

	BDBG_LEAVE(BRAP_CITGEN_P_GenFreeInterstageBuffer);

	return(ui32Error);		
}


/******************************************************************************
Summary:
	This function fills the Smaplig frequency conversion table.
	The LUT tells the output sampling frequency of the CHIP for a particular 
	input Fs.. 
			Table is divided into 
					a) QSF (8,11,12) 
					b) Half (16,22,24) 
					c) Full (32,44,48) and 
					d) High (64,88,96) 
					e) Very High (128,176,192) 

	This table is modified if the DTS and Ac3 encoder is present.

  Aim of this LUT is to tell the IDS/Framesync/system about the final sampling frequency.
  For MPEG decoders, we need to support the prime band as the final Fs. FW looks into this 
  LUT and gets the output sampling frequency LSF and QSF - > prime Frequency

  DTS Encoder exception:
		DTS encoder can do encode only on 44.1 and 48K not 32Khz. 
		So the final sampling frequency for 8, 16 and 32Khz and 64Khz should be 48Khz.

  Ac3 Encoder exception:
		Ac3 encoder can do encode only on 48K. 
		So the final sampling frequency for 8, 16,32Khz and 64Khzshould be 48Khz.

  MS11 Usage case : Output sampling frequency is always 48000
  
Description: 	
			
Input:	
	psAlgoModePresent
	 

Output:	

Returns:
		Error

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillSamplingFrequencyMapLut(
				BMEM_Handle					hHeap,
				uint32_t					ui32FwOpSamplingFreqMapLutAddr,
				BRAP_CIT_P_sAlgoModePresent	*psAlgoModePresent
			)
{

	uint32_t ui32Error;
	BRAP_AF_P_sOpSamplingFreq	sOpSamplingFrequencyMapLut =	{
		{	  /*QSF */
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

	BDBG_ENTER(BRAP_CITGEN_P_FillSamplingFrequencyMapLut);

	ui32Error  = BERR_SUCCESS;

	
	
	/*Ms11 usage case */


	if ((psAlgoModePresent->ui32DolbyPulsePresent == BRAP_CIT_P_PRESENT) ||
		(	(psAlgoModePresent->ui32DDREPresent == BRAP_CIT_P_PRESENT)))
	{
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[0] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[1] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[2] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[3] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[4] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[5] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[6] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[7] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[8] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[9] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[10] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[11] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[12] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[13] = 48000;
		sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[14] = 48000;			
	}
	else
	{
		/*Special Case for DTS encoder and Ac3 encoder*/
		if(	(BRAP_CIT_P_PRESENT == psAlgoModePresent->ui32DTS_EncoderPresent) ||
			(BRAP_CIT_P_PRESENT == psAlgoModePresent->ui32AC3_EncoderPresent)
		  )
		{
			/*Upsampling to 48K output sampling frequency */

			/*  8K-> 48K */
			/* 16K-> 48K */
			/* 32K-> 48K */
			/* 64K-> 48K */
			/* 128K->48K */

			sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[0] = 48000;
			sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[3] = 48000;
			sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[6] = 48000;
			sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[9] = 48000;
			sOpSamplingFrequencyMapLut.ui32OpSamplingFrequency[12]= 48000;
		}
	}


	BRAP_CITGEN_P_WriteToDRAM(
					hHeap,
					(uint32_t)&sOpSamplingFrequencyMapLut,
					(uint32_t)ui32FwOpSamplingFreqMapLutAddr,
					(uint32_t)(BRAP_CIT_P_TASK_FS_MAPPING_LUT_SIZE)
			 );

	BDBG_LEAVE(BRAP_CITGEN_P_FillSamplingFrequencyMapLut);

	return(ui32Error);
}

	

/******************************************************************************
Summary:
	Prints Stage Port Connectivity and InterStage buffer Allocation

Description: 
	used for Debugging purpose	
			
Input:	

Output:	
		
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(		
						BRAP_CIT_P_sAlgoExecOrder	 sAlgoExecOrder[],
						BRAP_CIT_P_AlgoNwInfo		 *psAlgoNwInfo,
						BRAP_CIT_P_sCitDataStructure *psCitDataStructure
	)
{

#ifdef BRAP_CIT_P_PRINT_STAGE_PORT_CONNECTION

	uint32_t ui32Algo;
	uint32_t ui32BranchId;
	uint32_t ui32StageId;
	uint32_t ui32NumAlgosInNw;
	uint32_t ui32BranchNum;
	uint32_t ui32NumDstIp,ui32NumDstIpCount;
	uint32_t ui32NumDstOp,ui32NumDstOpCount;
	uint32_t ui32NumOp,ui32NumOpCount;


	BRAP_CIT_P_StageInfo			*psStgInfo;
	BRAP_CIT_P_FwStgConnectivity	*psStgConnectivity;
	BRAP_CIT_P_FwStgSrcDstDetails	*psStgSrcDstDetails;
	BRAP_CIT_P_FwStgSrcDstType      eType;


	BDBG_ENTER(BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc);
	

	/*	Find the number of algos within the network. */
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->sBranchInfo[ui32BranchNum].ui32NumStages;
	}


	BDBG_ERR((""));
	BDBG_ERR((""));
	BDBG_ERR(("*************************************"));
	BDBG_ERR(("STAGE INPUT OUTPUT and INTERSTAGE CFG"));
	BDBG_ERR(("*************************************"));

	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{

		ui32BranchId = sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId = sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo			= &psAlgoNwInfo->sBranchInfo[ui32BranchId].sStgInfo[ui32StageId];
		psStgConnectivity	= &psStgInfo->sStgConnectivity;

		ui32NumDstIp		= psStgConnectivity->ui32NumSrc;
		ui32NumDstOp		= psStgConnectivity->ui32NumDistinctOp;
		

		BDBG_ERR((""));
		BDBG_ERR((""));
		BDBG_ERR(("------------------"));
		BDBG_ERR(("The Branch Id = [%d]",ui32BranchId));
		BDBG_ERR(("The Stage  Id = [%d]",ui32StageId));
		BDBG_ERR(("------------------"));
		BDBG_ERR((""));
		BDBG_ERR(("Stage Interstage Id = [%d]",psStgInfo->ui32InterStageBuffIdx));					
		BDBG_ERR(("Number of Distinct Input = [%d]",ui32NumDstIp));
		BDBG_ERR(("Number of Distinct Outputs = [%d]",ui32NumDstOp));
		
		BDBG_ERR((""));
		BDBG_ERR(("Distinct Input Port details: "));
		for(ui32NumDstIpCount = 0 ;ui32NumDstIpCount<ui32NumDstIp;ui32NumDstIpCount++)
		{
			psStgSrcDstDetails = &psStgConnectivity->sSrcDetails[ui32NumDstIpCount];
			
			BDBG_ERR(("Distinct Input Port No.%d",ui32NumDstIpCount));
			BDBG_ERR(("~~~~~~~~~~~~~~~~~~~~~~~~~"));		

			eType = psStgSrcDstDetails->eType;
			BDBG_ERR(("	Connectivity = %s",FwConnectivityType[(uint16_t)eType]));

			if(eType == BRAP_CIT_P_FwStgSrcDstType_eFwStg)
			{
				BDBG_ERR(("	Connected to Branch = %d Stage = %d",psStgSrcDstDetails->uDetails.sFwStage.ui32BranchId,psStgSrcDstDetails->uDetails.sFwStage.ui32StageId));
				BDBG_ERR(("	Distinct output = %d and Output Port = %d",psStgSrcDstDetails->uDetails.sFwStage.ui32DistinctOpNum,psStgSrcDstDetails->uDetails.sFwStage.ui32OpNum));
			}


		}

		
		BDBG_ERR((""));
		BDBG_ERR((""));
		BDBG_ERR(("Distinct output port details: "));

		for(ui32NumDstOpCount = 0 ;ui32NumDstOpCount<ui32NumDstOp;ui32NumDstOpCount++)
		{
			
			ui32NumOp = psStgConnectivity->ui32NumDst[ui32NumDstOpCount];

			BDBG_ERR(("Distinct Output Port No.%d",ui32NumDstOpCount));
			BDBG_ERR(("~~~~~~~~~~~~~~~~~~~~~~~~~~"));			
			BDBG_ERR(("Number of Outputs  [%d]",ui32NumOp));			
			BDBG_ERR((""));	
			BDBG_ERR(("Output port details: "));

			for(ui32NumOpCount=0;ui32NumOpCount<ui32NumOp;ui32NumOpCount++)
			{
				psStgSrcDstDetails =
						&psStgConnectivity->sDstDetails[ui32NumDstOpCount][ui32NumOpCount];

				BDBG_ERR(("	"));	
				BDBG_ERR(("	Output No.%d",ui32NumOpCount));

				eType = psStgSrcDstDetails->eType;
				BDBG_ERR(("	Connectivity = %s",FwConnectivityType[(uint16_t)eType]));

				if(eType == BRAP_CIT_P_FwStgSrcDstType_eFwStg)
				{
					BDBG_ERR(("	Connected to Branch = %d Stage = %d",psStgSrcDstDetails->uDetails.sFwStage.ui32BranchId,psStgSrcDstDetails->uDetails.sFwStage.ui32StageId));					
				}
			}
			BDBG_ERR((""));
			BDBG_ERR(("Distinct Output port - Interstage Association  "));
			BDBG_ERR(("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  "));
			BDBG_ERR((" Interstage Required Flag  = %d",psCitDataStructure->sOutputIsCfg[ui32Algo][ui32NumDstOpCount].ui32IsRequired));
			BDBG_ERR((" Branch is Associated Flag = %d",psCitDataStructure->sOutputIsCfg[ui32Algo][ui32NumDstOpCount].ui32BranchAssociated));
			BDBG_ERR((" Interstage BufferId       = %d",psCitDataStructure->sOutputIsCfg[ui32Algo][ui32NumDstOpCount].ui32InterstageBufferId));
			BDBG_ERR((""));
		
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc);


#else
	/* No Functionality .. To Avoid warning */
	uint32_t Temp;

	Temp	= sAlgoExecOrder[0].ui32BranchId;
	Temp	= psAlgoNwInfo->ui32NumBranches;	
	Temp	= psCitDataStructure->sOutputIsCfg[0][0].ui32IsRequired;

	BDBG_LEAVE(BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc);

#endif
}


/******************************************************************************
Summary:
	This function Calculates the Threshold And BlockTime values based on the 
	Algorithm.

	THIS FUNCTION HAS NOTHING TO DO WITH CITGEN MODULE.

	THIS FUNCTION GOT ITS PLACE IN brap_cit_priv.c file SINCE IT HAS NO PLACE
		TO SIT ANY WHERE ELSE



Description: 

		Now ui32BlockTimeInSamples -> in samples
			Later to time.....
			
				
Input:	

Output:		
	  a) Theshold
	  b) Block time in milli seconds in Q15 format
		
Returns:
		None

******************************************************************************/
void BRAP_CITGEN_P_CalcThresholdAndBlockTime(BRAP_CTB_Output *psCTB_OutputStructure)
{
		
		psCTB_OutputStructure->ui32Threshold = (BRAP_CIT_P_MINIMUM_ALGO_THRESHOLD + BRAP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);


		/* Block Time  */

		/*

			{[130MHz]/[216Mhx ZspClock]}/{48000/2048[AACHE Frame size]} == 25msec

		*/

		/*
			Block Time in Q15 == Block Time * 0x7fff = 841

			Block Time	in samples = 841 *48000 / 0x7fff 

		
		*/

		/* psCTB_OutputStructure->ui32BlockTimeInSamples = 1231;*/

		psCTB_OutputStructure->ui32BlockTime = 42;

}



void BRAP_CITGEN_P_CalcThresholdZeroFillTimeAudOffset(
					BRAP_CIT_P_LowDelayEnableMode	eLowDelayEnableMode,
					BRAP_CIT_P_InputInfo			*psCitIp,
					BRAP_CTB_Output					*psCTB_OutputStructure
				)
{


	BDBG_ENTER(BRAP_CITGEN_P_CalcThresholdZeroFillTimeAudOffset);

#ifdef ENABLE_LOW_DELAY_API

	BSTD_UNUSED(psCitIp);	
		
	psCTB_OutputStructure->ui32Threshold = (BRAP_CIT_P_MINIMUM_ALGO_THRESHOLD + BRAP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);

	if(eLowDelayEnableMode == BRAP_CIT_P_LowDelayMode_eDisabled)
	{
		/*Blocking Time*/
		psCTB_OutputStructure->ui32BlockTime = 42;


		/*AUD_OFFSET*/
		psCTB_OutputStructure->ui32AudOffset = 128;
	}
	else
	{

		/*AUD_OFFSET*/						

		/*
		Time taken for the path (based on MIPS numbers of all the executables in the data path) * 48000 / 32000 
			+ Input path delay (which is one frame for SPDIF input) + Data Sync delay + TSM time + Decode delay 
			+ Blocking Time
		*/

		/*

		Input buffer Time : (256 Samples) / (48000) = 5.33;

		FMM Delay : 1(Input) + 1(Output) = 2msec

		Data Sync delay		: 0

		PTS maturity delay	: 3 

		Decode delay : 

				Worst case MIPS = 3.9
					PCMPT	= 1.0	msec
					CV2		= 1.4 	msec
					SRS		= 0.5	msec
					Scheduler Overhead : 1msec
		
		Blocking Time =0;	

		
		Total AUD_OFFSET = 5.33 + 2 + 0 + 3 + (3.9)(48000/32000) + 0
				
		*/		

		/* psCTB_OutputStructure->ui32AudOffset = 12;  */
		
		if( NULL==psCitIp->pBranchInfo[0] )
		{
			BDBG_ERR((" ERROR LOW DELAY API : Branch0 is not configured"));
		}
		else
		{
			/*Doing Low delay cfg for Ac3 Algo */
			if( psCitIp->pBranchInfo[0]->sFwStgInfo[0].uAlgorithm.eDecAudioAlgo == BRAP_DSPCHN_AudioType_eAc3 )
			{
				psCTB_OutputStructure->ui32AudOffset = 75; 
				psCTB_OutputStructure->ui32BlockTime = 11;
			}
			/*Doing Low delay cfg for PCM pass through */
			else if ( psCitIp->pBranchInfo[0]->sFwStgInfo[0].uAlgorithm.eDecAudioAlgo == BRAP_DSPCHN_AudioType_ePcm )/* 	It is PCM */
			{
			
				/*Rounding Off */
				psCTB_OutputStructure->ui32AudOffset = 19; 


				/*Blocking Time*/
				/*psCTB_OutputStructure->ui32BlockTime = 2;*/

				/*Rounding Off*/
				psCTB_OutputStructure->ui32BlockTime = 3;
			}
			/* Default configuration */
			else
			{
				/*Blocking Time*/
				psCTB_OutputStructure->ui32BlockTime = 42;


				/*AUD_OFFSET*/
				psCTB_OutputStructure->ui32AudOffset = 128;
			}
		}
	}

#else

	
	BSTD_UNUSED(eLowDelayEnableMode);
	
	BSTD_UNUSED(psCitIp);
	
	
	psCTB_OutputStructure->ui32Threshold = (BRAP_CIT_P_MINIMUM_ALGO_THRESHOLD + BRAP_CIT_P_MAXIMUM_RESIDUAL_COLLECTION);

	/*Blocking Time*/
	psCTB_OutputStructure->ui32BlockTime = 42;

	/*AUD_OFFSET*/
	psCTB_OutputStructure->ui32AudOffset = 128;

#endif


	BDBG_LEAVE(BRAP_CITGEN_P_CalcThresholdZeroFillTimeAudOffset);
}


/******************************************************************************
Summary:
		Generate the RDB Address table in DRAM

Description: 
	
		This function generates RDB register table required for the Audio FW
		and places in DRAM memory allocated by the PI.

		PI loads the Dram-table content to DSP-SRAM memory address specified
		(ui32DspSramMemoryAddr) in device open time. 
		
		The DSP-SRAM memory address also varies across DSPs. Therefore this 
		function returns DSP-SRAM address also to PI.

		
		Operations:
				1) Get RDB Physical Address from RBD File.
				2) Generate RDB table using Base and Chip offset.
				3) Populate the DRAM table.	
				4) Return
		
Input:	
			psRdbCfg				: Chip RDB configuration
			ui32FwRdbTableDramAddr	: Dram address of RDB Table
Output:	
			ui32DspSramMemoryAddr	: DSP SRAM address on which PI should load
									  the RDB table	
Returns:
		None

******************************************************************************/
#if 0
void BRAP_CITGEN_P_GenerateFwRdbAddrTable(
							BRAP_CIT_P_RdbCfg	*psRdbCfg,
							uint32_t			ui32FwRdbTableDramAddr,
							uint32_t			ui32DspSramMemoryAddr
						)
{
	BDBG_ENTER(BRAP_CITGEN_P_GenerateFwRdbAddrTable);


	BDBG_LEAVE(BRAP_CITGEN_P_GenerateFwRdbAddrTable);
}
#endif



uint32_t BRAP_SPS_P_SeamlessPortSwitch(
							BMEM_Handle				hHeap,
							BREG_Handle				hRegister,
							BRAP_SPS_InputInfo		*psSpsIp,
							BRAP_SPS_InputPortCfg	*psInputPortCfg								
						)
{
	uint32_t	ui32Error;
	uint32_t	ui32TaskId;	
	uint32_t	ui32CitTaskFmmDestCfgAddr;
		
	BRAP_AF_P_sFMM_DEST_CFG	*psWorkingTaskFmmDestCfgBuffPtr;

	BDBG_ENTER(BRAP_SPS_P_SeamlessPortSwitch);

	BDBG_ASSERT(hHeap);
	BDBG_ASSERT(psSpsIp);
	BDBG_ASSERT(psInputPortCfg);

	/*Initialization */	
	ui32Error					= BERR_SUCCESS;	
	ui32TaskId					= 0;
	ui32CitTaskFmmDestCfgAddr	= (uint32_t)NULL;		

	/*Messages*/
	BDBG_MSG((""));
	BDBG_MSG(("=================================================== "));
	BDBG_MSG(("ENTERING SEAMLESS PORT SWITCHING API: For Task [%d] ",psSpsIp->ui32TaskId));
	BDBG_MSG(("=================================================== "));
	BDBG_MSG((""));

	/* Get and verify input buffer details*/
	ui32Error = BRAP_SPS_P_GetFmmCfgAddrAndVerifyBufferSize(	
											hHeap,
											psSpsIp,
											&ui32TaskId,
											&psWorkingTaskFmmDestCfgBuffPtr,
											&ui32CitTaskFmmDestCfgAddr
										);	
	if( ui32Error != BERR_SUCCESS)
	{
		goto BRAP_SPS_P_EXIT_POINT;
	}

	/* Copy the content of Running FMM task Cfg buffer to the  Working Buffer */
	ui32Error = BRAP_SPS_P_CopyCitFmmDestCfgToWorkingBuff(											
											ui32CitTaskFmmDestCfgAddr,								
											psWorkingTaskFmmDestCfgBuffPtr
										);

	if( ui32Error != BERR_SUCCESS)
	{
		goto BRAP_SPS_P_EXIT_POINT;
	}

	
	/* Process the input port cfg change */	
	ui32Error = BRAP_SPS_P_ReconfigureFmmPortCfg(	
											hRegister,
											psInputPortCfg,									
											psWorkingTaskFmmDestCfgBuffPtr
										);

	if( ui32Error != BERR_SUCCESS)
	{
		goto BRAP_SPS_P_EXIT_POINT;
	}

	/*SPS-API Exit point */
	BRAP_SPS_P_EXIT_POINT:

	/* Check for Error and assert */
	if(ui32Error !=BERR_SUCCESS)
	{
		BDBG_ASSERT(0);
	}


	/* Messages */
	BDBG_ERR((""));
	BDBG_ERR(("=================================================== "));
	BDBG_ERR(("LEAVING SEAMLESS PORT SWITCHING API: For Task [%d] ",psSpsIp->ui32TaskId));
	BDBG_ERR(("=================================================== "));
	
	BDBG_ERR((""));

	BDBG_LEAVE(BRAP_SPS_P_SeamlessPortSwitch);

	return ui32Error;
}

/******************************************************************************
Summary:
		This function extracts all the input information on the Buffers.

		A sanity check for buffer size is also done here		

Description: 
	
		CIT allocated FmmDestCfg Address and PI allocated dummy buffer addr is 
		extracted..

		A sanity check on PI allocated buffer and CIT allocated buffer is done.		
		
Input:	
		psSpsIp							: Input to SPS API	
			
Output:	
		pui32TaskId						: TaskID
		pui32CitTaskFmmDestCfgAddr		: CIT allocated FmmDestCfg memomy addr.									  
		psWorkingTaskFmmDestCfgBuffPtr	: PI allocated FmmDestCfg memomy addr.
Returns:
		ui32Error

******************************************************************************/
static uint32_t BRAP_SPS_P_GetFmmCfgAddrAndVerifyBufferSize(
					BMEM_Handle					hHeap,
					BRAP_SPS_InputInfo		*psSpsIp,
					uint32_t					*pui32TaskId,
					BRAP_AF_P_sFMM_DEST_CFG		**psWorkingTaskFmmDestCfgBuffPtr,
					uint32_t					*pui32CitTaskFmmDestCfgAddr					
				)
{
	uint32_t ui32Error;
	void *pTemp;


	BDBG_ENTER(BRAP_SPS_P_GetFmmCfgAddrAndVerifyBufferSize);

	/*Intialization*/
	ui32Error = BERR_SUCCESS;


	/*Checking the operating Buffer size*/
	if(psSpsIp->sOperatingTaskFmmDestCfgBuff.ui32BufferSizeInBytes < SIZEOF(BRAP_AF_P_sFMM_DEST_CFG))
	{
		
		/* Defaulting */
		*pui32TaskId					= 0;
		*psWorkingTaskFmmDestCfgBuffPtr	= NULL;
		*pui32CitTaskFmmDestCfgAddr		= (uint32_t)NULL;	
		
		ui32Error = BERR_INVALID_PARAMETER;

		BDBG_ERR(("SPS-API Error: The buffer size allocated for Operating Buffer is not correct. Allocated is [%d] and Required is [%d]",\
			psSpsIp->sOperatingTaskFmmDestCfgBuff.ui32BufferSizeInBytes,SIZEOF(BRAP_AF_P_sFMM_DEST_CFG)));		
	}
	else
	{
		*pui32TaskId  = psSpsIp->ui32TaskId;

		*pui32CitTaskFmmDestCfgAddr		
					  = psSpsIp->ui32CitTaskFmmDestCfgMemoryAddr;
		
		/*  Converting the Physical Address to Offset Address for CIT Task 
		    FMM port memory address 
		*/			
		BRAP_ConvertOffsetToAddress(
								hHeap,
								psSpsIp->ui32CitTaskFmmDestCfgMemoryAddr,
								(void**)(&pTemp)		
							);

		*pui32CitTaskFmmDestCfgAddr =(uint32_t) pTemp;
        

        /* Type casting the Working Buffer Address*/
		*psWorkingTaskFmmDestCfgBuffPtr
                      =  (BRAP_AF_P_sFMM_DEST_CFG*)psSpsIp->sOperatingTaskFmmDestCfgBuff.ui32DramBufferAddress;
	}	

	BDBG_LEAVE(BRAP_SPS_P_GetFmmCfgAddrAndVerifyBufferSize);

	return ui32Error;
}

/******************************************************************************
Summary:
		This function process the input port configuration and sets the 
		appropriate fields in port comfiguration.

		Since FW is configuring the PLL Association of a port,
		the newly added port can be added to the existing task's port cfg
		directly

Description: 
	
		I2S/Dac/MaiMulti/Spdif/Mai ports are reconfigured 
		
Input:	
		psInputPortCfg				: Input port configuration sent by PI		
		psWorkingTaskFmmDestCfgBuffPtr	: Dummy Task FMM cfg memory addr
			
Output:	
		psWorkingTaskFmmDestCfgBuffPtr	: Contents are reconfigured to 
									  this memory location
Returns:
		ui32Error

******************************************************************************/
static uint32_t BRAP_SPS_P_ReconfigureFmmPortCfg(
					BREG_Handle					hRegister,
					BRAP_SPS_InputPortCfg		*psInputPortCfg, 					
					BRAP_AF_P_sFMM_DEST_CFG		*psWorkingTaskFmmDestCfgBuffPtr
				)
{

	uint32_t	ui32Error;
	uint32_t	ui32PortCount;
	uint32_t	ui32NumOutputs;

	
	BRAP_CapInputPort			eCapIpPort;
	BRAP_OutputPort				eOpPortType;
	BRAP_OutputPort				eMaiMuxSelector;
	BRAP_AF_P_PortEnableDisable eDacOnI2S;

	BRAP_CIT_P_FmmDstSelect	eFmmDstSelect;
	BRAP_AF_P_PortEnableDisable		ePortEnableDisableFlag;
	
	BRAP_AF_P_sFMM_DEST_CFG			*psFmmDestCfg;
	
	BDBG_ENTER(BRAP_SPS_P_ReconfigureFmmPortCfg);

	
	ui32Error  = BERR_SUCCESS;

	/* Intialization for DAC on I2S*/
	eDacOnI2S = BRAP_AF_P_PortDisable;

	/* Copying the port reconfiguration memory pointer locally */
	psFmmDestCfg	= psWorkingTaskFmmDestCfgBuffPtr;
	
	/* Getting the number of Dst ports */
	ui32NumOutputs	= psInputPortCfg->ui32NumOutputs;

	/* Analying the port settings individually */
	for(ui32PortCount=0;ui32PortCount<ui32NumOutputs;ui32PortCount++)
	{
		/* a) Destination Type*/	
		eFmmDstSelect	= psInputPortCfg->sPortCfg[ui32PortCount].sFmmDstDetails.eFmmDstSelect;		
		
		/* b) FMM Dst PORT reconfiguration */
		if(BRAP_CIT_P_FmmDstSelect_eOpPort == eFmmDstSelect)
		{	
			/* Getting the Output port Type  */
			eOpPortType	= psInputPortCfg->sPortCfg[ui32PortCount].sFmmDstDetails.uFmmDstPortType.eOpType;

			/* Port Enable Disable Flag*/
			/*ePortEnableDisableFlag	= psInputPortCfg->sPortCfg[ui32PortCount].ePortEnableDisableFlag;*/

			/* Enum Translation from EnableDisable to PortEnabble and PortDisable */
			ePortEnableDisableFlag 
				= (psInputPortCfg->sPortCfg[ui32PortCount].ePortEnableDisableFlag == BRAP_AF_P_eEnable)\
								? BRAP_AF_P_PortEnable : BRAP_AF_P_PortDisable;

			switch(eOpPortType)
			{			
				/*I2S Stream*/
				case	BRAP_OutputPort_eI2s0:
				case    BRAP_OutputPort_eMaiMulti0:      
						
							BRAP_SPS_P_I2S0PortReconfig( eOpPortType,
														 ePortEnableDisableFlag,
														 psFmmDestCfg
														);
							break;

				case	BRAP_OutputPort_eI2s1:
				case	BRAP_OutputPort_eI2s2:
				case	BRAP_OutputPort_eI2s3:		
				case	BRAP_OutputPort_eI2s4:		
				case	BRAP_OutputPort_eI2s5:
				case	BRAP_OutputPort_eI2s6:
				case	BRAP_OutputPort_eI2s7:
				case	BRAP_OutputPort_eI2s8:			
				case    BRAP_OutputPort_eMaiMulti1:
				case    BRAP_OutputPort_eMaiMulti2:
				case    BRAP_OutputPort_eMaiMulti3: 

							BRAP_SPS_P_I2S1PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case    BRAP_OutputPort_eSpdif:
						
							BRAP_SPS_P_Spdif0PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case    BRAP_OutputPort_eSpdif1:

							BRAP_SPS_P_Spdif1PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case    BRAP_OutputPort_eDac0:

							BRAP_SPS_P_Dac0PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case    BRAP_OutputPort_eDac1:

							BRAP_SPS_P_Dac1PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case	BRAP_OutputPort_eDac2:

							BRAP_SPS_P_Dac2PortReconfig(	eOpPortType,
															ePortEnableDisableFlag,
															psFmmDestCfg
														);
							break;

				case	BRAP_OutputPort_eMai:

							/* Extracting the MUX Selector */
							eMaiMuxSelector = psInputPortCfg->sPortCfg[ui32PortCount].uOpCfg.sMaiCfg.eMaiMuxSelector;

							ui32Error = BRAP_SPS_P_MaiPortReconfig(	
															ePortEnableDisableFlag,
															psFmmDestCfg,
															eMaiMuxSelector
														);
							break;
				default:

					ui32Error = BERR_NOT_SUPPORTED;
					BDBG_ERR(("SPS-API ERROR : The input port [%s] is not supported.",PortName[(uint16_t)eOpPortType]));
					BDBG_ERR((""));
					BDBG_ERR(("SPI-API is QUITTING..."));
					break;
			}
			/*-------------------------*/
			/*Detecting the I2S on Dac */
			/*-------------------------*/
			switch(eOpPortType)
			{			
				case	BRAP_OutputPort_eI2s4:

						/* Enabling the DAC on I2S port for I2s4. Setting the Global Flag */

						/* HARD CODED for I2S4*/

						eDacOnI2S	= BRAP_AF_P_PortEnable; 

						break;
				default:	

						break;
			}

		}
		else if(BRAP_CIT_P_FmmDstSelect_eCapturePort == eFmmDstSelect)
		{
			/* Getting the capture port Type  */
			eCapIpPort = psInputPortCfg->sPortCfg[ui32PortCount].sFmmDstDetails.uFmmDstPortType.eCapPortType;

			/* Port Enable Disable Flag*/
			/* ePortEnableDisableFlag	= psInputPortCfg->sPortCfg[ui32PortCount].ePortEnableDisableFlag;*/

			/* Enum Translation from EnableDisable to PortEnabble and PortDisable */
			ePortEnableDisableFlag 
				= (psInputPortCfg->sPortCfg[ui32PortCount].ePortEnableDisableFlag == BRAP_AF_P_eEnable)\
								? BRAP_AF_P_PortEnable : BRAP_AF_P_PortDisable;

			switch(eCapIpPort)
			{			
				case	BRAP_CapInputPort_eIntCapPort0:		
					
						BRAP_SPS_P_CapturePortNReconfig(	ePortEnableDisableFlag,
															psFmmDestCfg,
															0
														);
						break;

				case	BRAP_CapInputPort_eIntCapPort1:

						BRAP_SPS_P_CapturePortNReconfig(	ePortEnableDisableFlag,
															psFmmDestCfg,
															1
														);													
						break;

				case	BRAP_CapInputPort_eIntCapPort2:

						BRAP_SPS_P_CapturePortNReconfig(	ePortEnableDisableFlag,
															psFmmDestCfg,
															2
														);													
						break;

				case	BRAP_CapInputPort_eIntCapPort3:

						BRAP_SPS_P_CapturePortNReconfig(	ePortEnableDisableFlag,
															psFmmDestCfg,
															3
														);											
													
						break;

				case	BRAP_CapInputPort_eIntCapPort4:
				case	BRAP_CapInputPort_eIntCapPort5:
				case	BRAP_CapInputPort_eIntCapPort6:
				case	BRAP_CapInputPort_eIntCapPort7:
				case	BRAP_CapInputPort_eExtI2s0:
				case	BRAP_CapInputPort_eRfAudio:  
				case	BRAP_CapInputPort_eSpdif:  
				case	BRAP_CapInputPort_eHdmi:    
				case	BRAP_CapInputPort_eAdc:   
				default:

						ui32Error = BERR_NOT_SUPPORTED;
						BDBG_ERR((""));
						BDBG_ERR(("SPS-API ERROR : The input port [%s] is not supported.",CapPortName[(uint16_t)eCapIpPort]));
						BDBG_ERR((""));
						BDBG_ERR(("SPS-API is QUITTING..."));
						break;
			}		
		}
		else
		{
			ui32Error = BERR_NOT_SUPPORTED;
			BDBG_ERR((""));
			BDBG_ERR(("SPS-API ERROR : The Fmm Destination type [%s] is not supported.",FmmDstType[(uint16_t)eFmmDstSelect]));
			BDBG_ERR((""));
			BDBG_ERR(("SPI-API is QUITTING..."));
		}


		/*Checking the Error and quiting the Port reconfiguration */
		if(ui32Error != BERR_SUCCESS)
		{
			break;
		}
	}


	/*------------------------------------------*/
	/*  EXT Clock Configuration The HARD CODE : */
	/*-+----------------------------------------*/
	if(BRAP_AF_P_PortEnable == eDacOnI2S)
	{			
		BRAP_CITGEN_P_ConfigureExternalClockSource(
								hRegister,
								psWorkingTaskFmmDestCfgBuffPtr->sFmmDestPllCfg.ui32PllIndex
							);
	}

	BDBG_LEAVE(BRAP_SPS_P_ReconfigureFmmPortCfg);

	return ui32Error;

}


/******************************************************************************
Summary:
		Copy the FMM port configuration of a running task content to the 
		temporary operating memory allocates by PI. 
		(Structure: BRAP_AF_P_sFMM_DEST_CFG)

Description: 
	
		The contents of FMM port configuration of the task is taken and placed 
		in to operating task memory to process.		
Input:	
		ui32CitTaskFmmDestCfgAddr		: Current FMM Task cfg memory addr		
			
Output:	
		psWorkingTaskFmmDestCfgBuffPtr	: Contents are copied to this
												memory location
Returns:
		Error

******************************************************************************/
static uint32_t BRAP_SPS_P_CopyCitFmmDestCfgToWorkingBuff(												
					uint32_t				 ui32CitTaskFmmDestCfgAddr,
					BRAP_AF_P_sFMM_DEST_CFG *psWorkingTaskFmmDestCfgBuffPtr
				)
{	

	uint32_t ui32Error;

	BRAP_AF_P_sFMM_DEST_CFG *psCitFMMDestCfgAddr;


	ui32Error = BERR_SUCCESS;

	BDBG_ENTER(BRAP_SPS_P_CopyCitFmmDestCfgToWorkingBuff);
	
	/*Pointer type Casting*/
	psCitFMMDestCfgAddr					
					= (BRAP_AF_P_sFMM_DEST_CFG *)ui32CitTaskFmmDestCfgAddr;
	
	/*Copy the contens to Dummy buffer*/
	*psWorkingTaskFmmDestCfgBuffPtr	= *psCitFMMDestCfgAddr;

	BDBG_LEAVE(BRAP_SPS_P_CopyCitFmmDestCfgToWorkingBuff);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the I2S0 Port

Description: 
	
		The port enable/disable flag of I2S0 is reconfigured based on the input
		configuration.		
Input:	
		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: I2S port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/
static uint32_t BRAP_SPS_P_I2S0PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{

	uint32_t ui32Error;
	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_I2S0PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort = psFmmDestCfg->sFmmDestI2SClkCfg[0].ui32I2SEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestI2SClkCfg[0].ui32I2SEnabled = ePortEnableDisableFlag;
	

	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));
	

	BDBG_LEAVE(BRAP_SPS_P_I2S0PortReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the I2S1 Port

Description: 
	
		The port enable/disable flag of I2S1 is reconfigured based on the input
		configuration.		
Input:	

		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: I2S port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/

static uint32_t BRAP_SPS_P_I2S1PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{	
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_I2S1PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort = psFmmDestCfg->sFmmDestI2SClkCfg[1].ui32I2SEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestI2SClkCfg[1].ui32I2SEnabled = ePortEnableDisableFlag;


	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));


	BDBG_LEAVE(BRAP_SPS_P_I2S1PortReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the Capture  Port

Description: 
	
		The port enable/disable flag of Capture port N is reconfigured based 
		on the input configuration.				
Input:	
	
		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: Cap port Enable/Disable flag
		ui32CapPortIndex		: Cap port index		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/

static uint32_t BRAP_SPS_P_CapturePortNReconfig(
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg,
							uint32_t					ui32CapPortIndex
						)
{


	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_CapturePortNReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort 
		= psFmmDestCfg->sFmmCapPortClkCfg[ui32CapPortIndex].ui32CapPortEnabled;

	psFmmDestCfg->sFmmCapPortClkCfg[ui32CapPortIndex].ui32CapPortEnabled
		= ePortEnableDisableFlag;


	BDBG_MSG(("\nCapture port %d configuration is changed from [%s] to [%s] ",\
						ui32CapPortIndex,
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));


	BDBG_LEAVE(BRAP_SPS_P_CapturePortNReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the SPDIF0 Port

Description: 
	
		The port enable/disable flag of SPDIF0 is reconfigured based on the input
		configuration.		
Input:	
		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: SPDIF0 port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/
static uint32_t BRAP_SPS_P_Spdif0PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_Spdif0PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort 
		= psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[0].ui32SpdifEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[0].ui32SpdifEnabled
										= ePortEnableDisableFlag;


	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));
	
	BDBG_LEAVE(BRAP_SPS_P_Spdif0PortReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the SPDIF1 Port

Description: 
	
		The port enable/disable flag of SPDIF1 is reconfigured based on the input
		configuration.		
Input:	
		ePortEnableDisableFlag	: SPDIF1 port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/
static uint32_t BRAP_SPS_P_Spdif1PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_Spdif1PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort 
		= psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[1].ui32SpdifEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestSpdifCfg.sFmmDestSpdifClkCbitCfg[1].ui32SpdifEnabled 

										= ePortEnableDisableFlag;

	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));

	BDBG_LEAVE(BRAP_SPS_P_Spdif1PortReconfig);

	return ui32Error;
}
	
/******************************************************************************
Summary:
		Reconfiguring the DAC0 Port

Description: 
	
		The port enable/disable flag of DAC0 is reconfigured based on the input
		configuration.
		
Input:	

		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: DAC0 port Enable/Disable flag	
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/	
static uint32_t BRAP_SPS_P_Dac0PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_Dac0PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort = psFmmDestCfg->sFmmDestDacCfg[0].ui32DacEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestDacCfg[0].ui32DacEnabled = ePortEnableDisableFlag;


	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));


	BDBG_LEAVE(BRAP_SPS_P_Dac02PortReconfig);
	
	return ui32Error;
}	

/******************************************************************************
Summary:
		Reconfiguring the DAC1 Port

Description: 
	
		The port enable/disable flag of DAC1 is reconfigured based on the input
		configuration.
		
Input:	
		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: DAC1 port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/	
static uint32_t BRAP_SPS_P_Dac1PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_Dac1PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort = psFmmDestCfg->sFmmDestDacCfg[1].ui32DacEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestDacCfg[1].ui32DacEnabled = ePortEnableDisableFlag;


	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));
	
	BDBG_LEAVE(BRAP_SPS_P_Dac1PortReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
		Reconfiguring the DAC2 Port

Description: 
	
		The port enable/disable flag of DAC2 is reconfigured based on the input
		configuration.
		
Input:	
		BRAP_OutputPort			: Port Name
		ePortEnableDisableFlag	: DAC2 port Enable/Disable flag		
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/	
static uint32_t BRAP_SPS_P_Dac2PortReconfig(
							BRAP_OutputPort				eOpPortType,
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg
						)
{
	uint32_t ui32Error;

	BRAP_AF_P_PortEnableDisable	ePresentStateOfPort;

	BDBG_ENTER(BRAP_SPS_P_Dac2PortReconfig);

	ui32Error = BERR_SUCCESS;

	/*Getting the present state of the port*/
	ePresentStateOfPort = psFmmDestCfg->sFmmDestDacCfg[2].ui32DacEnabled;

	/*Changing the port enable flag*/
	psFmmDestCfg->sFmmDestDacCfg[2].ui32DacEnabled = ePortEnableDisableFlag;


	BDBG_MSG(("\n[%s] : Port configuration is changed from [%s] to [%s] ",\
						PortName[(uint32_t)eOpPortType],
						PortEnableDisable[(uint32_t)ePresentStateOfPort],\
						PortEnableDisable[(uint32_t)ePortEnableDisableFlag]));

	BDBG_LEAVE(BRAP_SPS_P_Dac2PortReconfig);

	return ui32Error;
}	
	 
/******************************************************************************
Summary:
		Reconfiguring the MAI Port

Description: 
	
		The port enable/disable flag of MAI is reconfigured based on the input
		configuration.
		
Input:	
		ePortEnableDisableFlag	: MAI port Enable/Disable flag
		eMaiMuxSelector			: MAI Mux selector	
			
Output:	
		psFmmDestCfg			: Contents are modified

Returns:
		ui32Error

******************************************************************************/	
static uint32_t BRAP_SPS_P_MaiPortReconfig(
							BRAP_AF_P_PortEnableDisable	ePortEnableDisableFlag,
							BRAP_AF_P_sFMM_DEST_CFG		*psFmmDestCfg,
							BRAP_OutputPort				eMaiMuxSelector
						)
{
	uint32_t ui32Error;

	BDBG_ENTER(BRAP_SPS_P_MaiPortReconfig);

	ui32Error = BERR_SUCCESS;

	switch(eMaiMuxSelector)
	{			
		/*I2S Stream*/
		case	BRAP_OutputPort_eI2s0:
		case	BRAP_OutputPort_eI2s1:
		case	BRAP_OutputPort_eI2s2:
		case	BRAP_OutputPort_eI2s3:		
		case	BRAP_OutputPort_eI2s4:		
		case	BRAP_OutputPort_eI2s5:
		case	BRAP_OutputPort_eI2s6:
		case	BRAP_OutputPort_eI2s7:
		case	BRAP_OutputPort_eI2s8:
		case    BRAP_OutputPort_eMaiMulti0:           
        case    BRAP_OutputPort_eMaiMulti1:
        case    BRAP_OutputPort_eMaiMulti2:
        case    BRAP_OutputPort_eMaiMulti3: 

				BDBG_MSG(("\nMAI Port Reconfiguration"));
				BRAP_SPS_P_Spdif1PortReconfig(	eMaiMuxSelector,
												ePortEnableDisableFlag,
												psFmmDestCfg
											);		

				break;

		case	BRAP_OutputPort_eSpdif:

				BDBG_MSG(("\nMAI Port Reconfiguration"));

				BRAP_SPS_P_Spdif0PortReconfig(	eMaiMuxSelector,
												ePortEnableDisableFlag,
												psFmmDestCfg
											);				
				break;

		case	BRAP_OutputPort_eMai:

				BDBG_MSG(("\nMAI Port Reconfiguration"));

				BRAP_SPS_P_Spdif1PortReconfig(	eMaiMuxSelector,
												ePortEnableDisableFlag,
												psFmmDestCfg
											);	

				break;
		default:
				BDBG_MSG(("\nMAI Port Reconfiguration"));

				ui32Error = BERR_NOT_SUPPORTED;
				BDBG_ERR(("ERROR : The input Mai stream type [%s] is not supported",PortName[(uint16_t)eMaiMuxSelector]));				
				break;
	}


	BDBG_LEAVE(BRAP_SPS_P_MaiPortReconfig);

	return ui32Error;
}

/******************************************************************************
Summary:
	This function is used to overclock the HiFi DAC to 4X rate

Description: 
	
	This function sets the HIFIDAC_CTRL0_RATEMGRCFG to Over clock mode
			
Input:
		hRegister		:  Register handle 
		eEnableDisable  :  Enable/Disable   

Output:	
		None		
Returns:
		None

******************************************************************************/
#if BTSC_HACK_ENABLED
static void  BRAP_CITGEN_P_OverSampleHifiDac(								
								BREG_Handle				hRegister,								
								BRAP_AF_P_EnableDisable eEnableDisable
							)
{
	uint32_t	ui32RegVal;	
	uint32_t	ui3OverSampleFactor;	

	BDBG_ENTER(BRAP_CITGEN_P_ConfigureExternalClockSource);


	if(BRAP_AF_P_eEnable == eEnableDisable)
	{
		ui3OverSampleFactor = 0; /* 64Fs : 4X rate */
	}
	else
	{
		ui3OverSampleFactor = 1; /* 256Fs: 1X rate */
	}

#ifndef TESTBENCH_BUILD

	/* Reading the Register value */
	ui32RegVal = BRAP_Read32(hRegister,BCHP_HIFIDAC_CTRL0_RATEMGRCFG);

	/* Inverting the mask */
	ui32RegVal &= ~(BCHP_MASK (HIFIDAC_CTRL0_RATEMGRCFG, OVERSAMPLE));  
	
	/*Writing the oversample value*/
    ui32RegVal |= (BCHP_FIELD_DATA (HIFIDAC_CTRL0_RATEMGRCFG, 
                                OVERSAMPLE, ui3OverSampleFactor)); 

	/*Writing to the register*/
	BRAP_Write32(hRegister, 
                 BCHP_HIFIDAC_CTRL0_RATEMGRCFG, ui32RegVal
			); 

#endif

	BDBG_LEAVE(BRAP_CITGEN_P_ConfigureExternalClockSource);

}
#endif
/******************************************************************************
Summary:
	Configure the EXTERNAL CLOCK for BRCM CHIP 

Description: 
	This function configures the external clock Pin of BCM chips 
	with the correct PLL index of the task...

	We need to hardcode the External clock source index with the the PLL of the 
	task which is connected to a specific I2S port...

	THIS FUNCTION IS FULLY HARDCODED.

	Hardcoded params :
			I2S PORT :				Done in BRAP_CITGEN_P_PrepareOutputPortAssociation	
											BRAP_SPS_P_SeamlessPortSwitch	
			EXT Clock Pin Index :	Done in this function 
			
Input:
		hRegister		: Register handle 
		ui32PllIndex	: Pll index	()					

Output:	
		None
		
Returns:
		None

******************************************************************************/
static void  BRAP_CITGEN_P_ConfigureExternalClockSource(								
								BREG_Handle		hRegister,
								uint32_t		ui32PllIndex						
							)
{	

/* Dynamic port switching for 3548 */
#if (BRAP_DYNAMIC_PORT_SWITCHING == 1)

	uint32_t	ui32RegVal;	
	uint32_t	ui32PllClockSelect;
	uint32_t	ui32ExternalClockSlect;	


	BDBG_ENTER(BRAP_CITGEN_P_ConfigureExternalClockSource);


	BDBG_MSG((""));
	BDBG_MSG(("====================================== "));
	BDBG_MSG(("ENTERING TO EXTERNAL CLOCK PROGRAMMING "));
	BDBG_MSG(("====================================== "));
	BDBG_MSG((""));

	/* Default Selection */
	ui32PllClockSelect		= 0;
	ui32ExternalClockSlect	= 0;	/* HARD CODED  to Register index 0 */

	/*Selecting the Pll Clock Select */
	if(ui32PllIndex == 0)
	{
		ui32PllClockSelect =0 + 1;  /* PLL0_ch2 (same rate as of the output port)*/
	}
	else if(ui32PllIndex == 1)
	{

		#if(BRAP_3548_FAMILY == 1)

			ui32PllClockSelect =12 + 1; /* PLL1_ch2 (same rate as of the output port)*/

		#else

			ui32PllClockSelect =3  + 1; /* PLL1_ch2 (same rate as of the output port)*/

		#endif
	}
	else
	{
		BDBG_ERR(("Error : Unknown PLL index given to task" ));
	}
	
	/* Reading the Register value */
	ui32RegVal = BRAP_Read32(hRegister, 
                BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_BASE + ui32ExternalClockSlect*4);
                
	/* Inverting the mask */
	ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, PLLCLKSEL));                

	/*Writing the PLL select value*/
    ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
                                PLLCLKSEL, ui32PllClockSelect)); 
	/*Writing to the register*/
	BRAP_Write32(hRegister, 
                 BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_BASE + ui32ExternalClockSlect*4, 
                 ui32RegVal
			); 

	BDBG_LEAVE(BRAP_CITGEN_P_ConfigureExternalClockSource);

#else

	BDBG_ENTER(BRAP_CITGEN_P_ConfigureExternalClockSource);
		
	BSTD_UNUSED(hRegister);	
	BSTD_UNUSED(ui32PllIndex);	

	BDBG_LEAVE(BRAP_CITGEN_P_ConfigureExternalClockSource);

#endif

}

/******************************************************************************
Summary:
	This is an API to return the name of the Decoder/Encoder/PP type

			
Input:
		eDecodeEncPPAlgoType	: This enum BRAP_AF_P_DecodeEncPPAlgoType
								   has  unified ids for all Dec/Enc/PP

How to call the fuction 
	
	const char					*pDecPPEncName;

	Function call ---
  
	pDecPPEncName = BRAP_P_PrintDecEncPpTypeName(BRAP_AF_P_EncAudioType_eAc3);
	BDBG_ERR(("%s",pDecPPEncName));


Output:			
		
Returns:
		Return Name of Feature type

******************************************************************************/
const char*  BRAP_P_PrintDecEncPpTypeName(
					BRAP_AF_P_DecodeEncPPAlgoType  eDecodeEncPPAlgoType									 
				)
{
	return(DecodeEncPPAlgoTypeName[eDecodeEncPPAlgoType]);
}

/*---------------------------------------------------------------------
				Top level GFX CIT Generation Function 
---------------------------------------------------------------------*/
uint32_t BRAP_P_GenCitGfx(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_GFX_P_sGfxDecodeBuffCfg	*psGfxDecodeBuffCfg,
						BRAP_CIT_P_GfxCITOutput	    *psGfxCitOp
					  )
{
	uint32_t ui32Err;
	uint32_t ui32PhysAddr;
	uint32_t ui32IfBuffBaseAddr;	
	uint32_t ui32TotalTaskMemory;	
	uint32_t ui32TotalSharedMemory;
	uint32_t ui32UsrCfgBuffBaseAddr;
												
	uint32_t ui32ScratchBuffBaseAddr;
	uint32_t ui32FwStatusBuffBaseAddr;	
	uint32_t ui32InterStgBuffBaseAddr;
	uint32_t ui32IoBuffCfgStructBaseAddr;		
	

	BRAP_CIT_P_sTaskBuffInfo				sTaskBuffInfo;
	BRAP_AF_P_sDRAM_BUFFER					sTaskStackBuff;
									
	BRAP_CIT_P_sCitDataStructure			*psCitDataStructure =NULL;


	BDBG_ENTER(BRAP_P_GenCitGfx);		

	BDBG_ASSERT(hHeap);
	BDBG_ASSERT(psCitIp);
	BDBG_ASSERT(psGfxCitOp);
       BSTD_UNUSED(hRegister);
       BSTD_UNUSED(psGfxDecodeBuffCfg);       
/*	BDBG_ASSERT(psGfxDecodeBuffCfg);*/


	ui32Err = BERR_SUCCESS;

	/*---------------------------------------------------*/
	/*Memory Allocation for the CIT Global Datastructure */
	/*---------------------------------------------------*/

	ui32Err = BRAP_CITGEN_P_AllocateCitDatastructure(&psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{	
		return ui32Err;
	}


	/* Initializing the Contents of CitDataStructure */
	BRAP_CITGEN_P_InitializeCitDatastructue(psCitDataStructure);
			
	/*	First convert the virtual address in the CIT input structure to physical 
		addresses 
	*/
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32SharedMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32SharedMemPhysAdr = ui32PhysAddr;

	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32TaskMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32TaskMemPhysAdr = ui32PhysAddr;

	/*Geting the RDB Offsets Configuration */

	BRAP_CITGEN_P_GetCitInputRdbCfg(psCitIp,
									&psCitDataStructure->sRdbCfg
								 );
	

	/*	Prepare the node network information. This function basically copies
		all the information from CIT i/p structure to AlgoNwInfo structure 
	*/
	BRAP_CITGEN_P_PrepareAlgoNwStruct(
									psCitIp, 
									&psCitDataStructure->sAlgoNwInfo 
								 );	

	/* 
		Detects special tasks in the system. Currently only detecting the Mixer Task alone.
	*/
	BRAP_CITGEN_P_DetectSpecialTasks(
									&psCitDataStructure->sAlgoNwInfo,
									&psCitDataStructure->sSpecialTaskPresent
								 );


	/*	From the algo network, decide the execution order. The order
		of execution will be indicated by linearly traversing the 
		sAlgoExecOrder structure. This structure will contain the branchId 
		and StageId of the Algo, in the order of execution */
	ui32Err = BRAP_CITGEN_P_DetermineAlgoExecOrder(
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure->sAlgoExecOrder
								 );

	if( ui32Err != BERR_SUCCESS)
	{	
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Associate the Algo ids of nodes with the algorithm in the 
		Netwrok info structure. */

	ui32Err = BRAP_CITGEN_P_GetNodesInAlgo(psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the MIT information into Algo Network Info */
	ui32Err = BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
									hHeap,
									psCitIp->psMit,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the information in the BRAP_sNodeInfo structure into the  Algo Network */
	BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
									BRAP_sNodeInfo,
									&psCitDataStructure->sAlgoNwInfo
								);

	/* Preparing the task RDB configuration */
	BRAP_CITGEN_P_PrepareTaskRdbCfg(&psCitDataStructure->sRdbCfg);



							/*---------------------------*/
							/* TASK BUFFER CONFIGURATION */
							/*---------------------------*/

	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Function to Compute the user config and inter-frame
		buffers for the nodes in the algo netwrok
	*/
	ui32Err = BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status buffer */
	/*
		Function to Compute the Firmware status buffer
		for the nodes in the algo network
	*/
	ui32Err = BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
		
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}


	/*	This function computes the worst case size required for the I/O
		buffer configuration structures in DRAM. The worst case size is
		computed, assuming each algorithm can have different i/o buffer
		structures 
	*/
	BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								 );

	/* Adding Stack swap */
	/*	Computing the Stack swap size for the task.
		Now it is hardcoded 
	*/
	BRAP_CITGEN_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

	sTaskStackBuff.ui32BufferSizeInBytes 
						= sTaskBuffInfo.ui32TaskStackMemSize;


	/* Buffer Allocation */
	/*-------------------*/
		
	/*	A single chunk of memeory is allocated for the user
		config and the IF buffers. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalTaskMemory =
			sTaskBuffInfo.ui32TaskUsrCfgMemSize				+		
			sTaskBuffInfo.ui32TaskInterFrmMemSize			+
			sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize	+
			sTaskBuffInfo.ui32TaskFwStatusBuffMemSize		+
			sTaskBuffInfo.ui32TaskStackMemSize	
		;


	if(ui32TotalTaskMemory > psCitIp->sMemInfo.ui32TaskMemSize)
	{
		BDBG_ERR(("ERROR: Task Memory required is more than allocated"));	
		BDBG_ERR(("\t Task Memory required= %d  Allocated memory = %d",ui32TotalTaskMemory,psCitIp->sMemInfo.ui32TaskMemSize));	
		BDBG_ERR((""));

		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}	
	
	ui32UsrCfgBuffBaseAddr		=	psCitIp->sMemInfo.ui32TaskMemPhysAdr;

	ui32IfBuffBaseAddr			=	ui32UsrCfgBuffBaseAddr				 + 
											sTaskBuffInfo.ui32TaskUsrCfgMemSize;

	ui32IoBuffCfgStructBaseAddr =	ui32IfBuffBaseAddr					 + 
											sTaskBuffInfo.ui32TaskInterFrmMemSize;

	ui32FwStatusBuffBaseAddr	=	ui32IoBuffCfgStructBaseAddr			 +
											sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize;

	sTaskStackBuff.ui32DramBufferAddress 
								=	ui32FwStatusBuffBaseAddr			 +
											sTaskBuffInfo.ui32TaskFwStatusBuffMemSize;
	


	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	Assign Inter-frame and user config addresses to nodes in the
		algos of the network
	*/
	ui32Err = BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
									ui32IfBuffBaseAddr,
									ui32UsrCfgBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status*/
	/* Assign FW status  addresses to stages in the network */
	ui32Err = BRAP_CITGEN_P_AllocFwStatusBuff(
									ui32FwStatusBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								);	

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*-------------------------------------------------------------------------------*/


							/*---------------------------------*/
							/* INTERSTAGE BUFFER CONFIGURATION */
							/*---------------------------------*/


	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Compute the worst case size required for Inter-stage buffers.
		The inter-stage buffers includes the inter-stage I/O buffer 
		as well as the inter-stage generic buffer
	*/
	ui32Err = BRAP_CITGEN_P_ComputeInterStgBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Compute the Worst case scratch buffer size required by any node 
		in the configuration
	*/
	BRAP_CITGEN_P_ComputeScratchBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);




	/* Buffer Allocation */
	/*-------------------*/

	/*	A single chunk of memeory is allocated for the inter-stage and scratch
		buffer. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalSharedMemory = 
			sTaskBuffInfo.ui32TaskInterStgIoMemSize			+ 
			sTaskBuffInfo.ui32TaskInterStgGenericMemSize	+
			sTaskBuffInfo.ui32TaskScratchMemSize;

	if(ui32TotalSharedMemory >	psCitIp->sMemInfo.ui32SharedMemSize)
	{

		BDBG_ERR(("ERROR: Shared Memory required is more than allocated"));	
		BDBG_ERR(("\t Shared Memory required= %d  Allocated memory = %d",ui32TotalSharedMemory,psCitIp->sMemInfo.ui32SharedMemSize));	
		BDBG_ERR((""));
		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

	  goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	ui32InterStgBuffBaseAddr	=	psCitIp->sMemInfo.ui32SharedMemPhysAdr;

	ui32ScratchBuffBaseAddr		=	ui32InterStgBuffBaseAddr					+ 
							 		sTaskBuffInfo.ui32TaskInterStgIoMemSize		+ 
									sTaskBuffInfo.ui32TaskInterStgGenericMemSize;

	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	This is the main function that allocates I/O buffer configuration.
		This function first prepares the I/O buffer configurations structures
		for inter-stage buffers. It copies the I/O port configuration
		from the sAlgoNwInfo to DRAM and updates the address into 
		NodeAttr structure. 
		Finally it allocates which I/O buffer configuration to use for
		inter-stage buffers based on the branching information
	*/
	ui32Err = BRAP_CITGEN_P_AllocIoBuffCfg(	hHeap,
											ui32InterStgBuffBaseAddr,
											ui32IoBuffCfgStructBaseAddr,
											psCitDataStructure->sAlgoExecOrder,
											&sTaskBuffInfo,
											&psCitDataStructure->sAlgoNwInfo,
											&psCitDataStructure->sOpPortInfo,
											&psCitDataStructure->sTaskFmmGateOpenConfig,
											psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
											psCitDataStructure,
											psCitIp->eIndepDelayEnableFlag,
											BRAP_CIT_P_SameInputForFsAndDecode_eDisable,
											&psCitDataStructure->sSpecialTaskPresent
										  );

	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Print the Input/Output port configuration for all the stages.
		This function also prints the interstage buffer used for all output ports and
		the intermediate nodes
	*/
	BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(
									psCitDataStructure->sAlgoExecOrder,
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure
								);



	/*	Allocate scratch buffer base address in sAlgoNwInfo. This function is
		written just for completion of information in the node atribute
		structure of each node 
	*/
	BRAP_CITGEN_P_AllocScratchBuff( ui32ScratchBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								 );	
	
	
						/*--------------------------------------*/
						/*				Processing				*/
						/*--------------------------------------*/



	/*	Now that we have all the components of Node config, fill in the node config into CIT  */
	ui32Err = BRAP_CITGEN_P_FillNodeCfgIntoGfxCit(	
										&psCitDataStructure->sAlgoNwInfo,
										psCitDataStructure->sAlgoExecOrder,										
										&psGfxCitOp->sGfxCit
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}



	ui32Err = BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit( 
										hHeap,
										&psCitDataStructure->sAlgoNwInfo,
										psGfxDecodeBuffCfg,										
										&psGfxCitOp->sGfxCit.sGlobalTaskConfig
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_GFX_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Populate the CIT o/p structure */
	BRAP_CITGEN_P_FillGfxCitOp(
								hHeap,
								&sTaskStackBuff,										
								&psCitDataStructure->sAlgoNwInfo,
								psGfxCitOp 
							);



	/*-----------------------------------------------*/
	/* Gfx CIT Generation Module is executed successfully*/
	/*-----------------------------------------------*/	
	BDBG_MSG((" Gfx CIT Generation Module is executed successfully"));
	BDBG_MSG((""));	

	ui32Err = BERR_SUCCESS;


	/* EXIT Point */
	BRAP_GFX_CITGENMODULE_P_EXIT_POINT:
	/*--------------------------------------*/
	/* Free the Cit global memory allocated */
	/*--------------------------------------*/	
	BRAP_CITGEN_P_FreeCitDatastructure(&psCitDataStructure);

	/* Check for Error and assert */
	if(ui32Err !=BERR_SUCCESS)
	{
		BDBG_ASSERT(0);
	}

	BDBG_LEAVE(BRAP_P_GenCitGfx);		

	return ui32Err;	

}

/******************************************************************************
Summary:
		Fill the node cfg of Gfx CIT

Description: 

	Filling node cfg 
			
Input:
	hHeap
	psAlgoNwInfo
	sAlgoExecOrder

Output:
		psGfxCit
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoGfxCit(	
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_sAlgoExecOrder				sAlgoExecOrder[],
			BRAP_GFX_P_sTASK_CONFIG					*psGfxCit
		    )
{
	uint32_t				ui32Error;
	uint32_t				ui32BranchNum;
	uint32_t				ui32Algo, ui32Node;
	uint32_t				ui32NumAlgosInNw;
	uint32_t				ui32BranchId, ui32StageId;
	uint32_t				ui32NumNodesInAlgo;
	uint32_t				ui32NodeIndex;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_AF_P_sNODE_CONFIG	*psNodeCfg;


	BDBG_ENTER(BRAP_CITGEN_P_FillNodeCfgIntoGfxCit);

	ui32Error = BERR_SUCCESS;

	/*	Find the number of algos within the network. */
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}	

	/*	Update the CIT nodes based on execution order */
	psNodeCfg = &(psGfxCit->sNodeConfig[0]);
	ui32NodeIndex = 0;
	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{
		ui32BranchId = sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId = sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
		for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
		{
			psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
			psNodeCfg->uiNodeId = ui32NodeIndex;


			/*	This function updates the Node config with the parameters in 
				the	node attributes structure */
			ui32Error = BRAP_CITGEN_P_UpdtNodeCfgInCit (
												psNodeAttr,
												psNodeCfg 
											);
			if(ui32Error != BERR_SUCCESS)
			{
				return ui32Error;
			}

			psNodeCfg++;
			ui32NodeIndex++;
		}
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FillNodeCfgIntoGfxCit);

	return ui32Error;
}


/******************************************************************************
Summary:
	This function fills the CIT output structure

Description: 

	This function fills the CIT output structure to PI
			
Input:
	hHeap
	psTaskStackBuff
	psTaskSPDIFPortConfigAddr
	psAlgoNwInfo				 : Buffer information for a task 

Output:
		psCitOp
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_FillGfxCitOp(
			BMEM_Handle								hHeap,
			BRAP_AF_P_sDRAM_BUFFER					*psTaskStackBuff,			
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_CIT_P_GfxCITOutput				     *psGfxCitOp 
		   )
{

	uint32_t	ui32Node;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_OpStgInfo	*psCitGenOpStgInfo;

	uint32_t	*pDest;
	void		*pDestTemp;
	
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_CIT_P_FwBufInfo		*psOpNodeInfo;

	
	BDBG_ENTER(BRAP_CITGEN_P_FillGfxCitOp);
	/* Stack swap address */
	BRAP_ConvertOffsetToAddress(hHeap,
								psTaskStackBuff->ui32DramBufferAddress,
								(void **)&pDestTemp
								);

	pDest	= (uint32_t*)pDestTemp ;

	psGfxCitOp->sStackSwapBuff.ui32DramBufferAddress
						=	(uint32_t)pDest;
	psGfxCitOp->sStackSwapBuff.ui32BufferSizeInBytes
						=	psTaskStackBuff->ui32BufferSizeInBytes;



	psGfxCitOp->ui32NumBranches = psAlgoNwInfo->ui32NumBranches;
	
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
		
		psGfxCitOp->sCitBranchInfo[ui32BranchNum].ui32NumStages =  
														ui32NumAlgoStgInBranch;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			psCitGenOpStgInfo = &psGfxCitOp->sCitBranchInfo[ui32BranchNum].sCitStgInfo[ui32AlgoStg];

			psCitGenOpStgInfo->ui32NumNodes		= psStgInfo->ui32NumNodes;
			psCitGenOpStgInfo->uAlgorithm		= psStgInfo->uAlgorithm;
			psCitGenOpStgInfo->eAudioAlgoType	= psStgInfo->eAudioAlgoType;		
			psCitGenOpStgInfo->uAudioMode		= psStgInfo->uAudioMode;
			
			if( psStgInfo->bIpsFromFwStage == eFALSE )
			{
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}
			else
			{
				/*
				psCitGenOpStgInfo->ui32StartNodeIndex	= BRAP_CIT_P_INVALID_NODE_IDX;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
				*/

				/* This is added to get the status information from the last stage of the network*/
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}

			for ( ui32Node=0; ui32Node<psStgInfo->ui32NumNodes; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->eFwExecId = psNodeAttr->eAlgoId;

				/* Interframe address */
				if(psNodeAttr->ui32InterFrmBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32InterFrmBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				
				psOpNodeInfo->ui32InterframeBufAdr	= (uint32_t)pDest;
				psOpNodeInfo->ui32InterframeBufSize = psNodeAttr->ui32InterFrmBuffSize;
				
				/* User param configuration */
				if(psNodeAttr->ui32UsrCfgBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
				/*	Need to convert the physical address to virtual address for the CIT
					o/p structure */
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32UsrCfgBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				psOpNodeInfo->ui32UserParamBufAdr	 = (uint32_t)pDest;
				psOpNodeInfo->ui32UserParamBufSize	 = psNodeAttr->ui32UsrCfgBuffSize;

				/* Status buffer configuration */
				if(psNodeAttr->ui32FwStatusBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32FwStatusBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}
		
				psOpNodeInfo->ui32StatusBufAdr	=  (uint32_t)pDest;
				psOpNodeInfo->ui32StatusBufSize	=  psNodeAttr->ui32FwStatusBuffSize;

			}
			for(ui32Node=psStgInfo->ui32NumNodes;ui32Node<BRAP_AF_P_MAX_NUM_NODES_IN_ALGO;ui32Node++)
															
			{
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;

				psOpNodeInfo->ui32InterframeBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32InterframeBufSize	=  0;

				psOpNodeInfo->ui32UserParamBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32UserParamBufSize	=  0;

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;


			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_FillGfxCitOp);
}	
/******************************************************************************
Summary:
		Fill the global task configuration into Gfx CIT

Description: 

	Filling Global task cfg
			
Input:
	hHeap
	psAlgoNwInfo
	psGfxDecodeBuffCfg

Output:
		psGfxGlobalTaskConfig
Returns:
		None

******************************************************************************/
static uint32_t  BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit( 
			BMEM_Handle								hHeap,
			BRAP_CIT_P_AlgoNwInfo					*psAlgoNwInfo,
			BRAP_GFX_P_sGfxDecodeBuffCfg			*psGfxDecodeBuffCfgIp,
			BRAP_GFX_P_sGLOBAL_TASK_CONFIG			*psGfxGlobalTaskConfig												  
					)
{
	uint32_t ui32Error;	
	uint32_t ui32AlgoStg;
	uint32_t ui32BranchNum;	
	uint32_t ui32NumNodesInNw;
	uint32_t ui32NumAlgoStgInBranch;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_GFX_P_sGfxDecodeBuffCfg	*psGfxDecodeBuffCfg;
	BRAP_GFX_P_sGfxDecodeBuffCfg	*psGlobalTaskConfigFromPI;


	BDBG_ENTER(BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit);
        BSTD_UNUSED(hHeap);
        BSTD_UNUSED(psGfxDecodeBuffCfgIp);       
        


	ui32Error = BERR_SUCCESS;

	psGfxDecodeBuffCfg = (BRAP_GFX_P_sGfxDecodeBuffCfg*)BKNI_Malloc(SIZEOF(BRAP_GFX_P_sGfxDecodeBuffCfg));

	if(psGfxDecodeBuffCfg == NULL)
	{
		ui32Error = BERR_OUT_OF_SYSTEM_MEMORY;

		BDBG_ERR(("ERROR: Unable to allocate memory for sGfxDecodeBuffCfg in Function BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit "));
		BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit);

		return ui32Error;
	}
	else
	{
		/* Intializig the memory allcated with zeroes*/
		BKNI_Memset(psGfxDecodeBuffCfg ,0,sizeof( BRAP_GFX_P_sGfxDecodeBuffCfg));   	
	}	

	/*	Compute the number of nodes in the network */
	/*  ------------------------------------------ */
	ui32NumNodesInNw			= 0;
	psGlobalTaskConfigFromPI	= &psGfxGlobalTaskConfig->sGlobalTaskConfigFromPI;	
	
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInNw += psStgInfo->ui32NumNodes;
		}
	}


	psGfxGlobalTaskConfig->ui32NumberOfNodesInTask = ui32NumNodesInNw;	

#if 0
	
	/* Converting the PI offset addre to ZSP Address */
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psVDecodeBuffCfgIp->ui32PDQCircBuffAddr),
									&psGlobalTaskConfigFromPI->ui32PDQCircBuffAddr
								 );

	BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->ui32PRQCircBuffAddr),
									&psGlobalTaskConfigFromPI->ui32PRQCircBuffAddr
								  );
#endif
/*    psGlobalTaskConfigFromPI->ui32Dummy = psGfxDecodeBuffCfgIp->ui32Dummy;*/


	BKNI_Free(psGfxDecodeBuffCfg);

	BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoGfxCit);

	return ui32Error;
}


void BRAP_P_AnalyseGfxCit(
						BMEM_Handle					hHeap,
						BRAP_GFX_P_sTASK_CONFIG		*psGfxCit
					  )
{
	uint32_t	ui32NumNodes;
	uint32_t	ui32Node;
	uint32_t	ui32NumSrc, ui32NumDest;


	uint32_t	*pDest;
	void		*pDestTemp;

	BRAP_GFX_P_sGfxDecodeBuffCfg	 *psGfxDecodeBuffCfg;
    BRAP_GFX_P_sGLOBAL_TASK_CONFIG	 *psGfxGlobalTaskConfig;
	BRAP_AF_P_sNODE_CONFIG			 *psNodeCfg;

#ifdef ANALYZE_IO_CFG
	BRAP_AF_P_sIO_BUFFER			sIoBuffer;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
#endif

	/*	First Step: Print all the informations within the CIT structure */
	BDBG_MSG(("Global Task Configuration parameters"));
	BDBG_MSG(("===================================="));

	psGfxGlobalTaskConfig = &psGfxCit->sGlobalTaskConfig;
	ui32NumNodes = psGfxGlobalTaskConfig->ui32NumberOfNodesInTask;
	
	BDBG_MSG(("Number of Nodes in Task: %d ",ui32NumNodes));


	BDBG_MSG((""));
	BDBG_MSG(("Displaying the Global Task Cfg from PI"));
	BDBG_MSG(("--------------------------------------"));
	BDBG_MSG((""));

	psGfxDecodeBuffCfg = &psGfxGlobalTaskConfig->sGlobalTaskConfigFromPI;

	BDBG_MSG((" Dummy "));


	BDBG_MSG(("Node Configuration parameters "));
	BDBG_MSG(("============================= "));
	for(ui32Node=0; ui32Node<ui32NumNodes; ui32Node++)
	{
		psNodeCfg = &(psGfxCit->sNodeConfig[ui32Node]);
		BDBG_MSG(("Node index: %d ",psNodeCfg->uiNodeId));
		BDBG_MSG(("Algo Id: %s ",AlgoIdEnum2Char[psNodeCfg->eAlgoId]));		

		BDBG_MSG(("\neCollectResidual : %s ",DisableEnable[psNodeCfg->eCollectResidual]));
		/*BDBG_ERR(("Algo Id: %x ",psNodeCfg->eAlgoId));*/

		/*	Code Address and Size */
		BDBG_MSG(("DRAM Code Buffer Address: 0x%x ",
			psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Code Buffer Size: %d Bytes",
			psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes));

		/*	Lookup Table Address and Size */
		BDBG_MSG(("DRAM Lookup Table Buffer Address: 0x%x ",
			psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Lookup Table Buffer Size: %d Bytes",
			psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes));

		/*	Inter-frame buffer Address check */
		if( psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Not present for this Node"));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Address: 0x%x ",
				psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: %d Bytes",
				psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes));
		}

		/*	Node Status buffer Address check */
		if( psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Node Status buffer Not present for this Node"));
			BDBG_MSG(("DRAM Node Status buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Node Status buffer Address: 0x%x ",
				psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Node Status buffer Size: %d Bytes",
				psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes));
		}

		/*	User config buffer Address check */
		if( psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM User Config Buffer not present for this Node"));
			BDBG_MSG(("DRAM User Config Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM User Config Buffer Address: 0x%x ",
				psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM User Config Buffer Size: %d Bytes",
				psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes));
		}

		/*	Input buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Input Configuration Details:", ui32Node));
		BDBG_MSG(("Num Source feeding data to this node: %d", psNodeCfg->ui32NumSrc));
		for( ui32NumSrc=0; ui32NumSrc<psNodeCfg->ui32NumSrc; ui32NumSrc++)
		{
			BDBG_MSG(("Source %d Input Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc]));			

#ifdef ANALYZE_IO_CFG
			/*BDBG_MSG(("Input buffer Configuration:"));*/

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*	Analyze Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif
			BDBG_MSG(("Source %d Input Generic Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc]));

#ifdef ANALYZE_IO_CFG
			
			/* BDBG_MSG(("Input Generic buffer Configuration:")); */

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO Generic buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));
			
			/*	Analyze Io Genric Buff Struct */
			 /* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif
		}

		/*	Output buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Output Configuration Details:", ui32Node));
		BDBG_MSG(("Num Destination getting data from this node: %d", 
			psNodeCfg->ui32NumDst));
		for( ui32NumDest=0; ui32NumDest<psNodeCfg->ui32NumDst; ui32NumDest++)
		{

			/* IO BUFFER CONFIGURATION */
			/*-------------------------*/

			/*Printing Output Buffer Cfg Structure Address */
			BDBG_MSG(("Destination %d Output Buffer Cfg Structure Address: 0x%x", 
				ui32NumDest, psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest]));			
			
#ifdef ANALYZE_IO_CFG
			/* BDBG_MSG(("Output buffer Configuration:"));*/


			/* Getting contents of the Destination IO buffer */
			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;					
			
			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			
			/*	Print Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif


			/* IOGENERIC BUFFER CONFIGURATION */
			/*--------------------------------*/

			BDBG_MSG(("Destination %d Output Generic Buffer Cfg Structure Address: 0x%x",
				ui32NumDest, psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest]));


#ifdef ANALYZE_IO_CFG

			/*	Getting contents of the IO Generic buffer */

			/*Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));


			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Generic Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING						

			BDBG_MSG(("Destination %d Datatype : %s",
				ui32NumDest, PortDatatType[psNodeCfg->eNodeOpBuffDataType[ui32NumDest]]));	

#endif
		}		
		BDBG_MSG(("============================= "));
	}

}

/****************************************************************************/
/****************************************************************************/
/************************* VIDEO TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

/*---------------------------------------------------------------------
				Top level Video CIT Generation Function 
---------------------------------------------------------------------*/
uint32_t BRAP_P_VideoGenCit(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,
						BRAP_VF_P_sVDecodeBuffCfg	*psVDecodeBuffCfg,
						BRAP_CIT_P_VideoCITOutput	*psCitOp
					  )

{

	uint32_t ui32Err;
	uint32_t ui32PhysAddr;
	uint32_t ui32IfBuffBaseAddr;	
	uint32_t ui32TotalTaskMemory;	
	uint32_t ui32TotalSharedMemory;
	uint32_t ui32UsrCfgBuffBaseAddr;
												#ifdef VIDEO_CIT_DISABLE
													uint32_t ui32Threshold;	
													uint32_t ui32TaskFwHwCfgAddr;
													uint32_t ui32ZeroFillSamples;													
													uint32_t ui32TaskPortConfigAddr;
													uint32_t ui32TaskGateOpenConfigAddr;
													uint32_t ui32FwOpSamplingFreqMapLutAddr;
												#endif
	uint32_t ui32ScratchBuffBaseAddr;
	uint32_t ui32FwStatusBuffBaseAddr;	
	uint32_t ui32InterStgBuffBaseAddr;
	uint32_t ui32IoBuffCfgStructBaseAddr;		
	

	BRAP_CIT_P_sTaskBuffInfo				sTaskBuffInfo;
	BRAP_AF_P_sDRAM_BUFFER					sTaskStackBuff;
									
	BRAP_CIT_P_sCitDataStructure			*psCitDataStructure =NULL;


	BDBG_ENTER(BRAP_P_VideoGenCit);		

	BDBG_ASSERT(hHeap);
	BDBG_ASSERT(psCitIp);
	BDBG_ASSERT(psCitOp);
	BDBG_ASSERT(psVDecodeBuffCfg);
#ifndef VIDEO_CIT_DISABLE    
    BSTD_UNUSED(hRegister);
#endif

	ui32Err = BERR_SUCCESS;

	/*---------------------------------------------------*/
	/*Memory Allocation for the CIT Global Datastructure */
	/*---------------------------------------------------*/

	ui32Err = BRAP_CITGEN_P_AllocateCitDatastructure(&psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{	
		return ui32Err;
	}


	/* Initializing the Contents of CitDataStructure */
	BRAP_CITGEN_P_InitializeCitDatastructue(psCitDataStructure);
			
	/*	First convert the virtual address in the CIT input structure to physical 
		addresses 
	*/
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32SharedMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32SharedMemPhysAdr = ui32PhysAddr;

	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32TaskMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32TaskMemPhysAdr = ui32PhysAddr;

	/*Geting the RDB Offsets Configuration */

	BRAP_CITGEN_P_GetCitInputRdbCfg(psCitIp,
									&psCitDataStructure->sRdbCfg
								 );
	

	/*	Prepare the node network information. This function basically copies
		all the information from CIT i/p structure to AlgoNwInfo structure 
	*/
	BRAP_CITGEN_P_PrepareAlgoNwStruct(
									psCitIp, 
									&psCitDataStructure->sAlgoNwInfo 
								 );	

	/* 
		Detects special tasks in the system. Currently only detecting the Mixer Task alone.
	*/
	BRAP_CITGEN_P_DetectSpecialTasks(
									&psCitDataStructure->sAlgoNwInfo,
									&psCitDataStructure->sSpecialTaskPresent
								 );


	/*	From the algo network, decide the execution order. The order
		of execution will be indicated by linearly traversing the 
		sAlgoExecOrder structure. This structure will contain the branchId 
		and StageId of the Algo, in the order of execution */
	ui32Err = BRAP_CITGEN_P_DetermineAlgoExecOrder(
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure->sAlgoExecOrder
								 );

	if( ui32Err != BERR_SUCCESS)
	{	
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Associate the Algo ids of nodes with the algorithm in the 
		Netwrok info structure. */

	ui32Err = BRAP_CITGEN_P_GetNodesInAlgo(psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the MIT information into Algo Network Info */
	ui32Err = BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
									hHeap,
									psCitIp->psMit,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the information in the BRAP_sNodeInfo structure into the  Algo Network */
	BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
									BRAP_sNodeInfo,
									&psCitDataStructure->sAlgoNwInfo
								);

	/* Preparing the task RDB configuration */
	BRAP_CITGEN_P_PrepareTaskRdbCfg(&psCitDataStructure->sRdbCfg);



							/*---------------------------*/
							/* TASK BUFFER CONFIGURATION */
							/*---------------------------*/

	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Function to Compute the user config and inter-frame
		buffers for the nodes in the algo netwrok
	*/
	ui32Err = BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status buffer */
	/*
		Function to Compute the Firmware status buffer
		for the nodes in the algo network
	*/
	ui32Err = BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
		
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}


	/*	This function computes the worst case size required for the I/O
		buffer configuration structures in DRAM. The worst case size is
		computed, assuming each algorithm can have different i/o buffer
		structures 
	*/
	BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								 );

	/* Adding Stack swap */
	/*	Computing the Stack swap size for the task.
		Now it is hardcoded 
	*/
	BRAP_CITGEN_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

	sTaskStackBuff.ui32BufferSizeInBytes 
						= sTaskBuffInfo.ui32TaskStackMemSize;

														#ifdef VIDEO_CIT_DISABLE
																/* Adding port Config and SPDIF Config */
																BRAP_CITGEN_P_ComputeFwPortConfigSpdifBuffSize(&sTaskBuffInfo);

																/* Adding Gate open config */
																BRAP_CITGEN_P_ComputeGateOpenCfgBufferSize(&sTaskBuffInfo);

																/* Adding FwHw config */
																BRAP_CITGEN_P_ComputeHwFwCfgBufferSize(&sTaskBuffInfo);

																/* Adding Sampling Frequency Map LUT */
																BRAP_CITGEN_P_ComputeSamplingFrequencyMapLutSize(&sTaskBuffInfo);
														#endif	

	/* Buffer Allocation */
	/*-------------------*/
		
	/*	A single chunk of memeory is allocated for the user
		config and the IF buffers. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalTaskMemory =
			sTaskBuffInfo.ui32TaskUsrCfgMemSize				+		
			sTaskBuffInfo.ui32TaskInterFrmMemSize			+
			sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize	+
			sTaskBuffInfo.ui32TaskFwStatusBuffMemSize		+
			sTaskBuffInfo.ui32TaskStackMemSize	

														#ifdef VIDEO_CIT_DISABLE
																												+
																sTaskBuffInfo.ui32TaskPortConfigMemSize			+
																sTaskBuffInfo.ui32TaskSPDIFConfigMemSize		+
																sTaskBuffInfo.ui32TaskGateOpenConfigMemSize		+
																sTaskBuffInfo.ui32TaskHwFwCfgMemSize			+
																sTaskBuffInfo.ui32SamplingFrequencyMapLutSize	
														#endif
		;


	if(ui32TotalTaskMemory > psCitIp->sMemInfo.ui32TaskMemSize)
	{
		BDBG_ERR(("ERROR: Task Memory required is more than allocated"));	
		BDBG_ERR(("\t Task Memory required= %d  Allocated memory = %d",ui32TotalTaskMemory,psCitIp->sMemInfo.ui32TaskMemSize));	
		BDBG_ERR((""));

		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}	
	
	ui32UsrCfgBuffBaseAddr		=	psCitIp->sMemInfo.ui32TaskMemPhysAdr;

	ui32IfBuffBaseAddr			=	ui32UsrCfgBuffBaseAddr				 + 
											sTaskBuffInfo.ui32TaskUsrCfgMemSize;

	ui32IoBuffCfgStructBaseAddr =	ui32IfBuffBaseAddr					 + 
											sTaskBuffInfo.ui32TaskInterFrmMemSize;

	ui32FwStatusBuffBaseAddr	=	ui32IoBuffCfgStructBaseAddr			 +
											sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize;

	sTaskStackBuff.ui32DramBufferAddress 
								=	ui32FwStatusBuffBaseAddr			 +
											sTaskBuffInfo.ui32TaskFwStatusBuffMemSize;



														#ifdef VIDEO_CIT_DISABLE

																/* Adding port Config and SPDIF Config */
																ui32TaskPortConfigAddr		=	sTaskStackBuff.ui32DramBufferAddress +
																										sTaskStackBuff.ui32BufferSizeInBytes;

																ui32TaskAllSPDIF_PortConfigAddr 
																							=	ui32TaskPortConfigAddr				 + 
																										sTaskBuffInfo.ui32TaskPortConfigMemSize;
																
																/* TaskGateOpenConfig */
																ui32TaskGateOpenConfigAddr  =   ui32TaskAllSPDIF_PortConfigAddr		 +
																										sTaskBuffInfo.ui32TaskSPDIFConfigMemSize;


																ui32TaskFwHwCfgAddr			=	ui32TaskGateOpenConfigAddr		 	 +
																										sTaskBuffInfo.ui32TaskGateOpenConfigMemSize;

																ui32FwOpSamplingFreqMapLutAddr
																							=	ui32TaskFwHwCfgAddr					 +
																										sTaskBuffInfo.ui32TaskHwFwCfgMemSize;

														#endif
	


	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	Assign Inter-frame and user config addresses to nodes in the
		algos of the network
	*/
	ui32Err = BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
									ui32IfBuffBaseAddr,
									ui32UsrCfgBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status*/
	/* Assign FW status  addresses to stages in the network */
	ui32Err = BRAP_CITGEN_P_AllocFwStatusBuff(
									ui32FwStatusBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								);	

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}
	
													#ifdef VIDEO_CIT_DISABLE													
														/* Adding Port Config and SPDIF Config */
														ui32Err = BRAP_CITGEN_P_AllocSpdifPortCfgBuff(
																						ui32TaskAllSPDIF_PortConfigAddr,
																						&psCitDataStructure->sTaskSPDIFPortConfigAddr[0]/*  Array */
																					);
														
														if( ui32Err != BERR_SUCCESS)
														{
															goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
														}

													#endif

	/*-------------------------------------------------------------------------------*/


							/*---------------------------------*/
							/* INTERSTAGE BUFFER CONFIGURATION */
							/*---------------------------------*/


	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Compute the worst case size required for Inter-stage buffers.
		The inter-stage buffers includes the inter-stage I/O buffer 
		as well as the inter-stage generic buffer
	*/
	ui32Err = BRAP_CITGEN_P_ComputeInterStgBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Compute the Worst case scratch buffer size required by any node 
		in the configuration
	*/
	BRAP_CITGEN_P_ComputeScratchBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);




	/* Buffer Allocation */
	/*-------------------*/

	/*	A single chunk of memeory is allocated for the inter-stage and scratch
		buffer. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalSharedMemory = 
			sTaskBuffInfo.ui32TaskInterStgIoMemSize			+ 
			sTaskBuffInfo.ui32TaskInterStgGenericMemSize	+
			sTaskBuffInfo.ui32TaskScratchMemSize;

	if(ui32TotalSharedMemory >	psCitIp->sMemInfo.ui32SharedMemSize)
	{

		BDBG_ERR(("ERROR: Shared Memory required is more than allocated"));	
		BDBG_ERR(("\t Shared Memory required= %d  Allocated memory = %d",ui32TotalSharedMemory,psCitIp->sMemInfo.ui32SharedMemSize));	
		BDBG_ERR((""));
		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

	  goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	ui32InterStgBuffBaseAddr	=	psCitIp->sMemInfo.ui32SharedMemPhysAdr;

	ui32ScratchBuffBaseAddr		=	ui32InterStgBuffBaseAddr					+ 
							 		sTaskBuffInfo.ui32TaskInterStgIoMemSize		+ 
									sTaskBuffInfo.ui32TaskInterStgGenericMemSize;

	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	This is the main function that allocates I/O buffer configuration.
		This function first prepares the I/O buffer configurations structures
		for inter-stage buffers. It copies the I/O port configuration
		from the sAlgoNwInfo to DRAM and updates the address into 
		NodeAttr structure. 
		Finally it allocates which I/O buffer configuration to use for
		inter-stage buffers based on the branching information
	*/
	ui32Err = BRAP_CITGEN_P_AllocIoBuffCfg(	hHeap,
											ui32InterStgBuffBaseAddr,
											ui32IoBuffCfgStructBaseAddr,
											psCitDataStructure->sAlgoExecOrder,
											&sTaskBuffInfo,
											&psCitDataStructure->sAlgoNwInfo,
											&psCitDataStructure->sOpPortInfo,
											&psCitDataStructure->sTaskFmmGateOpenConfig,
											psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
											psCitDataStructure,
											psCitIp->eIndepDelayEnableFlag,
											BRAP_CIT_P_SameInputForFsAndDecode_eDisable,
											&psCitDataStructure->sSpecialTaskPresent
										  );

	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Print the Input/Output port configuration for all the stages.
		This function also prints the interstage buffer used for all output ports and
		the intermediate nodes
	*/
	BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(
									psCitDataStructure->sAlgoExecOrder,
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure
								);



	/*	Allocate scratch buffer base address in sAlgoNwInfo. This function is
		written just for completion of information in the node atribute
		structure of each node 
	*/
	BRAP_CITGEN_P_AllocScratchBuff( ui32ScratchBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								 );	
	
	
						/*--------------------------------------*/
						/*				Processing				*/
						/*--------------------------------------*/



														#ifdef VIDEO_CIT_DISABLE
		
																	/* Populating the FW HW configuration registers-structures */	
																	BRAP_CITGEN_P_FwHwBufferCfg(	hHeap,
	  																								&psCitDataStructure->sFwHwCfg,
																									ui32TaskFwHwCfgAddr
																								);	

												
																	/*FMM port configuration */
																	ui32Err = BRAP_CITGEN_P_TaskOutputPortCfg(	hHeap,
																												hRegister,
																												psCitIp,
																												ui32TaskPortConfigAddr,	
																												&psCitDataStructure->sTaskSPDIFPortConfigAddr[0], /* Array */
																												psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
																												&psCitDataStructure->sAlgoModePresent
																											);
																	if( ui32Err != BERR_SUCCESS)
																	{
																		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
																	}

											

																	/*Filling the Fw Op sampling map LUT */
																	ui32Err = BRAP_CITGEN_P_FillSamplingFrequencyMapLut(
																									hHeap,
																									ui32FwOpSamplingFreqMapLutAddr,
																									&psCitDataStructure->sAlgoModePresent
																								);

																	if( ui32Err != BERR_SUCCESS)
																	{
																		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
																	}

											

																	/*Adding Gate open */
																	BRAP_CITGEN_P_WriteGateOpenCfgToDram( 
																									hHeap,
																									&psCitDataStructure->sTaskFmmGateOpenConfig,
																									ui32TaskGateOpenConfigAddr,
																									SIZEOF(psCitDataStructure->sTaskFmmGateOpenConfig)										
																								);


											

																	/* Finding the Zero Fill Samples  */  /*Need to check whether FW is using */
																	ui32Err = BRAP_CITGEN_P_GetNumZeroFillSamples(
																									&ui32ZeroFillSamples,
																									&psCitDataStructure->sAlgoNwInfo
																								);

																	if( ui32Err != BERR_SUCCESS)
																	{
																		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
																	}
																						

																	/* Calculate the Scheduling Threshold*/  /*Need To remove */
																	ui32Err = BRAP_CITGEN_P_CalcThreshold(										
																									psCitDataStructure,
																									&ui32Threshold
																								);

																	if( ui32Err != BERR_SUCCESS)
																	{
																		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
																	}
														#endif


														#ifdef VIDEO_CIT_OLD_MODULES_DISABLE

																	/*	Now that we have all the components of Node config, fill in the node config into CIT  */
																	ui32Err = BRAP_CITGEN_P_FillNodeCfgIntoCit(	
																										&psCitDataStructure->sAlgoNwInfo,
																										psCitDataStructure->sAlgoExecOrder,
																										&psCitOp->sCit
																									);

																	if( ui32Err != BERR_SUCCESS)
																	{
																		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
																	}

																	


																	/*	Fill the global task configuration into CIT */
																	BRAP_CITGEN_P_FillGblTaskCfgIntoCit( &psCitDataStructure->sAlgoNwInfo,
																										 &psCitDataStructure->sOpPortInfo,
																										 ui32ScratchBuffBaseAddr,
																										 sTaskBuffInfo.ui32TaskScratchMemSize,
																										 &psCitOp->sCit.sGlobalTaskConfig,
																										 ui32TaskPortConfigAddr,
																										 ui32TaskGateOpenConfigAddr,
																										 ui32TaskFwHwCfgAddr,
																										 ui32FwOpSamplingFreqMapLutAddr,
																										 ui32ZeroFillSamples,
																										 psCitIp->eTimeBaseType
																									   );

																	/*	Populate the CIT o/p structure */
																	BRAP_CITGEN_P_FillCitOp(	hHeap,
																								&sTaskStackBuff,
																								&psCitDataStructure->sTaskSPDIFPortConfigAddr[0],
																								&psCitDataStructure->sAlgoNwInfo,
																								psCitOp 
																						);
														#endif


	/*	Now that we have all the components of Node config, fill in the node config into CIT  */
	ui32Err = BRAP_CITGEN_P_FillNodeCfgIntoVideoCit(	
										&psCitDataStructure->sAlgoNwInfo,
										psCitDataStructure->sAlgoExecOrder,										
										&psCitOp->sVideoCit
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}



	ui32Err = BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit( 
										hHeap,
										&psCitDataStructure->sAlgoNwInfo,
										psVDecodeBuffCfg,										
										&psCitOp->sVideoCit.sGlobalTaskConfig
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Populate the CIT o/p structure */
	BRAP_CITGEN_P_FillVideoCitOp(
									hHeap,
									&sTaskStackBuff,										
									&psCitDataStructure->sAlgoNwInfo,
									psCitOp 
								);



	/*-----------------------------------------------*/
	/* Video CIT Generation Module is executed successfully*/
	/*-----------------------------------------------*/	
	BDBG_MSG((" Video CIT Generation Module is executed successfully"));
	BDBG_MSG((""));	

	ui32Err = BERR_SUCCESS;


	/* EXIT Point */
	BRAP_VIDEO_CITGENMODULE_P_EXIT_POINT:
	/*--------------------------------------*/
	/* Free the Cit global memory allocated */
	/*--------------------------------------*/	
	BRAP_CITGEN_P_FreeCitDatastructure(&psCitDataStructure);

	/* Check for Error and assert */
	if(ui32Err !=BERR_SUCCESS)
	{
		BDBG_ASSERT(0);
	}

	BDBG_LEAVE(BRAP_P_VideoGenCit);		

	return ui32Err;	
}
/******************************************************************************
Summary:
		Fill the node cfg of Video CIT

Description: 

	Filling node cfg 
			
Input:
	hHeap
	psAlgoNwInfo
	sAlgoExecOrder

Output:
		psVideoCit
Returns:
		None

******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoVideoCit(	
						BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
						BRAP_CIT_P_sAlgoExecOrder			sAlgoExecOrder[],
						BRAP_VF_P_sTASK_CONFIG				*psVideoCit 
					)
{
	uint32_t				ui32Error;
	uint32_t				ui32BranchNum;
	uint32_t				ui32Algo, ui32Node;
	uint32_t				ui32NumAlgosInNw;
	uint32_t				ui32BranchId, ui32StageId;
	uint32_t				ui32NumNodesInAlgo;
	uint32_t				ui32NodeIndex;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_AF_P_sNODE_CONFIG	*psNodeCfg;


	BDBG_ENTER(BRAP_CITGEN_P_FillNodeCfgIntoVideoCit);

	ui32Error = BERR_SUCCESS;

	/*	Find the number of algos within the network. */
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}	

	/*	Update the CIT nodes based on execution order */
	psNodeCfg = psVideoCit->sNodeConfig;
	ui32NodeIndex = 0;
	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{
		ui32BranchId = sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId = sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
		for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
		{
			psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
			psNodeCfg->uiNodeId = ui32NodeIndex;

			/*Populating the Collect Residual Flag */
			psNodeCfg->eCollectResidual = (psNodeAttr->ui32BranchId == 0) ? BRAP_AF_P_eEnable : BRAP_AF_P_eDisable;


			/*	This function updates the Node config with the parameters in 
				the	node attributes structure */
			ui32Error = BRAP_CITGEN_P_UpdtNodeCfgInCit (
												psNodeAttr,
												psNodeCfg 
											);
			if(ui32Error != BERR_SUCCESS)
			{
				return ui32Error;
			}

			psNodeCfg++;
			ui32NodeIndex++;
		}
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FillNodeCfgIntoVideoCit);

	return ui32Error;
}


/******************************************************************************
Summary:
	This function fills the CIT output structure

Description: 

	This function fills the CIT output structure to PI
			
Input:
	hHeap
	psTaskStackBuff
	psTaskSPDIFPortConfigAddr
	psAlgoNwInfo				 : Buffer information for a task 

Output:
		psCitOp
Returns:
		None

******************************************************************************/
static void BRAP_CITGEN_P_FillVideoCitOp(
					BMEM_Handle						hHeap,
					BRAP_AF_P_sDRAM_BUFFER			*psTaskStackBuff,					
					BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
					BRAP_CIT_P_VideoCITOutput		*psCitOp 
				)
{

	uint32_t	ui32Count;
	uint32_t	ui32Node;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_OpStgInfo	*psCitGenOpStgInfo;

	uint32_t	*pDest;
	void		*pDestTemp;
	
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_CIT_P_FwBufInfo		*psOpNodeInfo;

	
	BDBG_ENTER(BRAP_CITGEN_P_FillVideoCitOp);
	/* Stack swap address */
	BRAP_ConvertOffsetToAddress(hHeap,
								psTaskStackBuff->ui32DramBufferAddress,
								(void **)&pDestTemp
								);

	pDest	= (uint32_t*)pDestTemp ;

	psCitOp->sStackSwapBuff.ui32DramBufferAddress
						=	(uint32_t)pDest;
	psCitOp->sStackSwapBuff.ui32BufferSizeInBytes
						=	psTaskStackBuff->ui32BufferSizeInBytes;


	/*  SPDIF User Cfg*/
	for(ui32Count=0;ui32Count<BRAP_AF_P_MAX_NUM_SPDIF_PORTS;ui32Count++)
	{
		psCitOp->sSpdifUserConfigAddr[ui32Count].ui32DramBufferAddress 
														= 	(uint32_t)NULL;

		psCitOp->sSpdifUserConfigAddr[ui32Count].ui32BufferSizeInBytes 
														= 0;
	}


	psCitOp->ui32NumBranches = psAlgoNwInfo->ui32NumBranches;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
		
		psCitOp->sCitBranchInfo[ui32BranchNum].ui32NumStages =  
														ui32NumAlgoStgInBranch;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			psCitGenOpStgInfo = &psCitOp->sCitBranchInfo[ui32BranchNum].sCitStgInfo[ui32AlgoStg];

			psCitGenOpStgInfo->ui32NumNodes		= psStgInfo->ui32NumNodes;
			psCitGenOpStgInfo->uAlgorithm		= psStgInfo->uAlgorithm;
			psCitGenOpStgInfo->eAudioAlgoType	= psStgInfo->eAudioAlgoType;		
			psCitGenOpStgInfo->uAudioMode		= psStgInfo->uAudioMode;
			
			if( psStgInfo->bIpsFromFwStage == eFALSE )
			{
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}
			else
			{
				/*
				psCitGenOpStgInfo->ui32StartNodeIndex	= BRAP_CIT_P_INVALID_NODE_IDX;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
				*/

				/* This is added to get the status information from the last stage of the network*/
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}

			for ( ui32Node=0; ui32Node<psStgInfo->ui32NumNodes; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->eFwExecId = psNodeAttr->eAlgoId;

				/* Interframe address */
				if(psNodeAttr->ui32InterFrmBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32InterFrmBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				
				psOpNodeInfo->ui32InterframeBufAdr	= (uint32_t)pDest;
				psOpNodeInfo->ui32InterframeBufSize = psNodeAttr->ui32InterFrmBuffSize;
				
				/* User param configuration */
				if(psNodeAttr->ui32UsrCfgBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
				/*	Need to convert the physical address to virtual address for the CIT
					o/p structure */
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32UsrCfgBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				psOpNodeInfo->ui32UserParamBufAdr	 = (uint32_t)pDest;
				psOpNodeInfo->ui32UserParamBufSize	 = psNodeAttr->ui32UsrCfgBuffSize;

				/* Status buffer configuration */
				if(psNodeAttr->ui32FwStatusBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32FwStatusBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}
		
				psOpNodeInfo->ui32StatusBufAdr	=  (uint32_t)pDest;
				psOpNodeInfo->ui32StatusBufSize	=  psNodeAttr->ui32FwStatusBuffSize;

			}
			for(ui32Node=psStgInfo->ui32NumNodes;ui32Node<BRAP_AF_P_MAX_NUM_NODES_IN_ALGO;ui32Node++)
															
			{
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;

				psOpNodeInfo->ui32InterframeBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32InterframeBufSize	=  0;

				psOpNodeInfo->ui32UserParamBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32UserParamBufSize	=  0;

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;


			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_FillVideoCitOp);
}	
/******************************************************************************
Summary:
		Fill the global task configuration into Video CIT

Description: 

	Filling Global task cfg
			
Input:
	hHeap
	psAlgoNwInfo
	psVDecodeBuffCfgIp

Output:
		psVideoGlobalTaskConfig
Returns:
		None

******************************************************************************/
static uint32_t  BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit( 
						BMEM_Handle						hHeap,
						BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
						BRAP_VF_P_sVDecodeBuffCfg		*psVDecodeBuffCfgIp,
						BRAP_VF_P_sGLOBAL_TASK_CONFIG	*psVideoGlobalTaskConfig											  
					)
{
	uint32_t ui32Error;	
	uint32_t ui32Count;
	uint32_t ui32AlgoStg;
	uint32_t ui32BranchNum;	
	uint32_t ui32NumNodesInNw;
	uint32_t ui32NumAlgoStgInBranch;

	BRAP_CIT_P_StageInfo		*psStgInfo;
	BRAP_VF_P_sVDecodeBuffCfg	*psVDecodeBuffCfg;
	BRAP_VF_P_sVDecodeBuffCfg	*psGlobalTaskConfigFromPI;


	BDBG_ENTER(BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit);


	ui32Error = BERR_SUCCESS;

	psVDecodeBuffCfg = (BRAP_VF_P_sVDecodeBuffCfg*)BKNI_Malloc(SIZEOF(BRAP_VF_P_sVDecodeBuffCfg));

	if(psVDecodeBuffCfg == NULL)
	{
		ui32Error = BERR_OUT_OF_SYSTEM_MEMORY;

		BDBG_ERR(("ERROR: Unable to allocate memory for sVDecodeBuffCfg in Function BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit "));
		BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit);

		return ui32Error;
	}
	else
	{
		/* Intializig the memory allcated with zeroes*/
		BKNI_Memset(psVDecodeBuffCfg ,0,sizeof( BRAP_VF_P_sVDecodeBuffCfg));   	
	}	

	/*	Compute the number of nodes in the network */
	/*  ------------------------------------------ */
	ui32NumNodesInNw			= 0;
	psGlobalTaskConfigFromPI	= &psVideoGlobalTaskConfig->sGlobalTaskConfigFromPI;	
	
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInNw += psStgInfo->ui32NumNodes;
		}
	}


	psVideoGlobalTaskConfig->ui32NumberOfNodesInTask = ui32NumNodesInNw;	

#if 0
	
	/* Converting the PI offset addre to ZSP Address */
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psVDecodeBuffCfgIp->ui32PDQCircBuffAddr),
									&psGlobalTaskConfigFromPI->ui32PDQCircBuffAddr
								 );

	BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->ui32PRQCircBuffAddr),
									&psGlobalTaskConfigFromPI->ui32PRQCircBuffAddr
								  );
#endif
    psGlobalTaskConfigFromPI->ui32PRQCircBuffAddr = psVDecodeBuffCfgIp->ui32PRQCircBuffAddr;
    psGlobalTaskConfigFromPI->ui32PDQCircBuffAddr = psVDecodeBuffCfgIp->ui32PDQCircBuffAddr;

    /* Frame Buffers */
	
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
	{

		/* Luma */
		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress
								  );

		psGlobalTaskConfigFromPI->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes =
						psVDecodeBuffCfgIp->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes;


		/* Chroma */

		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress
								  );

		psGlobalTaskConfigFromPI->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes =
						psVDecodeBuffCfgIp->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes;
					
	}



	
    /* Refernce Buffers */
	
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL;ui32Count++)
	{

		/* Luma */
		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress
								  );

		psGlobalTaskConfigFromPI->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes =
						psVDecodeBuffCfgIp->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes;


		/* Chroma */

		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress
								  );

		psGlobalTaskConfigFromPI->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes =
						psVDecodeBuffCfgIp->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes;
					
	}



	/* sUPBs */
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
	{

		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sUPBs[ui32Count].ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sUPBs[ui32Count].ui32DramBufferAddress
								  );

		psGlobalTaskConfigFromPI->sUPBs[ui32Count].ui32BufferSizeInBytes = 
						psVDecodeBuffCfgIp->sUPBs[ui32Count].ui32BufferSizeInBytes;

	}

	/* MB info */
	for(ui32Count =0;ui32Count<BRAP_FWMAX_MB_INFO_AVAIL;ui32Count++)
	{

		BRAP_P_ConvertAddressToOffset(
									hHeap, 
									(void *)(psVDecodeBuffCfgIp->sDRAMMBInfoStartAdr[ui32Count].ui32DramBufferAddress),
									&psGlobalTaskConfigFromPI->sDRAMMBInfoStartAdr[ui32Count].ui32DramBufferAddress
								  );
		psGlobalTaskConfigFromPI->sDRAMMBInfoStartAdr[ui32Count].ui32BufferSizeInBytes =
							psVDecodeBuffCfgIp->sDRAMMBInfoStartAdr[ui32Count].ui32BufferSizeInBytes;
	}


	/* Others */
	psGlobalTaskConfigFromPI->ui32NumBuffAvl		= psVDecodeBuffCfgIp->ui32NumBuffAvl;

	psGlobalTaskConfigFromPI->ui32DispStripeWidth	= psVDecodeBuffCfgIp->ui32DispStripeWidth;
	psGlobalTaskConfigFromPI->ui32DispStripeHeight	= psVDecodeBuffCfgIp->ui32DispStripeHeight;


	BKNI_Free(psVDecodeBuffCfg);

	BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoVideoCit);

	return ui32Error;
}

/******************************************************************************
Summary:
	This function analysis the Video CIT. The intention of this function is to
	analyse and print out all the parameters within the Video CIT structure

Description: 

	Debug prints
			
Input:
	hHeap
	psCit

Output:		
Returns:
		None

******************************************************************************/
void BRAP_P_AnalyseVideoCit(
							BMEM_Handle				hHeap,
							BRAP_VF_P_sTASK_CONFIG	*psCit
					  ) 
{
	uint32_t	ui32NumNodes;
	uint32_t	ui32Node,ui32Count;
	uint32_t	ui32NumSrc, ui32NumDest;


	uint32_t	*pDest;
	void		*pDestTemp;

	BRAP_VF_P_sVDecodeBuffCfg		*psVDecodeBuffCfg;
	BRAP_VF_P_sGLOBAL_TASK_CONFIG	*psGblTaskCfg;
	BRAP_AF_P_sNODE_CONFIG			*psNodeCfg;

#ifdef ANALYZE_IO_CFG
	BRAP_AF_P_sIO_BUFFER			sIoBuffer;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
#endif

	/*	First Step: Print all the informations within the CIT structure */
	BDBG_MSG(("Global Task Configuration parameters"));
	BDBG_MSG(("===================================="));

	psGblTaskCfg = &psCit->sGlobalTaskConfig;
	ui32NumNodes = psGblTaskCfg->ui32NumberOfNodesInTask;
	
	BDBG_MSG(("Number of Nodes in Task: %d ",ui32NumNodes));


	BDBG_MSG((""));
	BDBG_MSG(("Displaying the Global Task Cfg from PI"));
	BDBG_MSG(("--------------------------------------"));
	BDBG_MSG((""));

	psVDecodeBuffCfg = &psGblTaskCfg->sGlobalTaskConfigFromPI;

	BDBG_MSG((" PRQ Circ Buffer Addr 0x%x",psVDecodeBuffCfg->ui32PRQCircBuffAddr));
	BDBG_MSG((" PDQ Circ Buffer Addr 0x%x",psVDecodeBuffCfg->ui32PDQCircBuffAddr));	



	BDBG_MSG((""));
	BDBG_MSG((" Frame Buffer Details "));
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
	{

		BDBG_MSG(("\t  Frame Buffer [%d] Luma Buffer Addr 0x%x Luma Buffer Size  0x%x \n Chroma Buffer [%d] Addr 0x%x Chroma Buffer Size  0x%x" ,ui32Count, \
							psVDecodeBuffCfg->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress, \
							psVDecodeBuffCfg->sFrameBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes,
							ui32Count,
							psVDecodeBuffCfg->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress, \
							psVDecodeBuffCfg->sFrameBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes));
		BDBG_MSG((""));
									
	}
	
	BDBG_MSG((""));
	BDBG_MSG((" Reference Buffer Details "));
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL;ui32Count++)
	{

		BDBG_MSG(("\t  Frame Buffer [%d] Luma Buffer Addr 0x%x Luma Buffer Size  0x%x \n Chroma Buffer [%d] Addr 0x%x Chroma Buffer Size  0x%x" ,ui32Count, \
							psVDecodeBuffCfg->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32DramBufferAddress, \
							psVDecodeBuffCfg->sReferenceBuffParams[ui32Count].sFrameBuffLuma.ui32BufferSizeInBytes,
							ui32Count,
							psVDecodeBuffCfg->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32DramBufferAddress, \
							psVDecodeBuffCfg->sReferenceBuffParams[ui32Count].sFrameBuffChroma.ui32BufferSizeInBytes));
		BDBG_MSG((""));
									
	}

	BDBG_MSG((""));
	BDBG_MSG((" UPB Buffer Details "));
	for(ui32Count =0;ui32Count<BRAP_FWMAX_VIDEO_BUFF_AVAIL;ui32Count++)
	{

		BDBG_MSG(("\t UPB  [%d] Buffer Addr 0x%x Buffer Size  0x%x" ,ui32Count, \
							psVDecodeBuffCfg->sUPBs[ui32Count].ui32DramBufferAddress,\
							psVDecodeBuffCfg->sUPBs[ui32Count].ui32BufferSizeInBytes));
	}		

	/* MB info */
	BDBG_MSG((""));
	BDBG_MSG((" Dram MB Buffer Details "));

	for(ui32Count =0;ui32Count<BRAP_FWMAX_MB_INFO_AVAIL;ui32Count++)
	{
		BDBG_MSG(("\t Dram MB Buffer [%d] Buffer Addr 0x%x Buffer Size  0x%x" ,ui32Count, \
							psVDecodeBuffCfg->sDRAMMBInfoStartAdr[ui32Count].ui32DramBufferAddress,\
							psVDecodeBuffCfg->sDRAMMBInfoStartAdr[ui32Count].ui32BufferSizeInBytes));
	}	

	BDBG_MSG((""));
	BDBG_MSG((" NumBuffAvailable  = 0x%x",psVDecodeBuffCfg->ui32NumBuffAvl));	
	BDBG_MSG((" Display Stripe Width  = 0x%x",psVDecodeBuffCfg->ui32DispStripeWidth));
	BDBG_MSG((" Display Stripe height = 0x%x",psVDecodeBuffCfg->ui32DispStripeHeight));


	BDBG_MSG((""));
	BDBG_MSG((""));

	BDBG_MSG(("Node Configuration parameters "));
	BDBG_MSG(("============================= "));
	for(ui32Node=0; ui32Node<ui32NumNodes; ui32Node++)
	{
		psNodeCfg = &psCit->sNodeConfig[ui32Node];
		BDBG_MSG(("Node index: %d ",psNodeCfg->uiNodeId));
		BDBG_MSG(("Algo Id: %s ",AlgoIdEnum2Char[psNodeCfg->eAlgoId]));		

		BDBG_MSG(("\neCollectResidual : %s ",DisableEnable[psNodeCfg->eCollectResidual]));
		/*BDBG_ERR(("Algo Id: %x ",psNodeCfg->eAlgoId));*/

		/*	Code Address and Size */
		BDBG_MSG(("DRAM Code Buffer Address: 0x%x ",
			psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Code Buffer Size: %d Bytes",
			psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes));

		/*	Lookup Table Address and Size */
		BDBG_MSG(("DRAM Lookup Table Buffer Address: 0x%x ",
			psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Lookup Table Buffer Size: %d Bytes",
			psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes));

		/*	Inter-frame buffer Address check */
		if( psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Not present for this Node"));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Address: 0x%x ",
				psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: %d Bytes",
				psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes));
		}

		/*	Node Status buffer Address check */
		if( psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Node Status buffer Not present for this Node"));
			BDBG_MSG(("DRAM Node Status buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Node Status buffer Address: 0x%x ",
				psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Node Status buffer Size: %d Bytes",
				psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes));
		}

		/*	User config buffer Address check */
		if( psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM User Config Buffer not present for this Node"));
			BDBG_MSG(("DRAM User Config Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM User Config Buffer Address: 0x%x ",
				psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM User Config Buffer Size: %d Bytes",
				psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes));
		}

		/*	Input buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Input Configuration Details:", ui32Node));
		BDBG_MSG(("Num Source feeding data to this node: %d", psNodeCfg->ui32NumSrc));
		for( ui32NumSrc=0; ui32NumSrc<psNodeCfg->ui32NumSrc; ui32NumSrc++)
		{
			BDBG_MSG(("Source %d Input Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc]));			

#ifdef ANALYZE_IO_CFG
			/*BDBG_MSG(("Input buffer Configuration:"));*/

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*	Analyze Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif
			BDBG_MSG(("Source %d Input Generic Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc]));

#ifdef ANALYZE_IO_CFG
			
			/* BDBG_MSG(("Input Generic buffer Configuration:")); */

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO Generic buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));
			
			/*	Analyze Io Genric Buff Struct */
			 /* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif
		}

		/*	Output buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Output Configuration Details:", ui32Node));
		BDBG_MSG(("Num Destination getting data from this node: %d", 
			psNodeCfg->ui32NumDst));
		for( ui32NumDest=0; ui32NumDest<psNodeCfg->ui32NumDst; ui32NumDest++)
		{

			/* IO BUFFER CONFIGURATION */
			/*-------------------------*/

			/*Printing Output Buffer Cfg Structure Address */
			BDBG_MSG(("Destination %d Output Buffer Cfg Structure Address: 0x%x", 
				ui32NumDest, psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest]));			
			
#ifdef ANALYZE_IO_CFG
			/* BDBG_MSG(("Output buffer Configuration:"));*/


			/* Getting contents of the Destination IO buffer */
			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;					
			
			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			
			/*	Print Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif


			/* IOGENERIC BUFFER CONFIGURATION */
			/*--------------------------------*/

			BDBG_MSG(("Destination %d Output Generic Buffer Cfg Structure Address: 0x%x",
				ui32NumDest, psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest]));


#ifdef ANALYZE_IO_CFG

			/*	Getting contents of the IO Generic buffer */

			/*Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));


			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Generic Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING						

			BDBG_MSG(("Destination %d Datatype : %s",
				ui32NumDest, PortDatatType[psNodeCfg->eNodeOpBuffDataType[ui32NumDest]]));	

			BDBG_MSG((""));
#endif
		}		
		BDBG_MSG(("============================= "));
	}
}


/*----------------------------------------------------------------------------*/
/*		Function Prototype for Seamless Input Port Switching(SIPS) API  */
/*----------------------------------------------------------------------------*/

/******************************************************************************
Summary:
	
	  Toplevel Seamless Input Port Switching 

Description: 

	The operation of SIPS API 

		1) Get the RDB configuration 
		2) Prepare RDB offset
		3) Get the SIPS command
		4) Process SIPS command
				4.1) Add Port
				4.2) Remove Port
			
Input:

	hHeap		: Heap Memory
	psSipsIp	: Input Data Strucure for the SIPS API

Output:		

	psSipsOp	: Output Data Strucure for the SIPS API

Returns:

	Error : PI code has to check the Error after calling the SIPS API

******************************************************************************/

uint32_t BRAP_SIPS_P_SeamlessIpPortSwitchAPI(	
				BMEM_Handle				hHeap,				
				BRAP_SIPS_InputInfo		*psSipsIp,
				BRAP_SIPS_OutputInfo	*psSipsOp						
			)
{

	uint32_t						ui32Error;				/* Error Status */
	BRAP_SIPS_P_AddRemoveIpPort		eAddRemoveIpPort;		/* API Command */

	BRAP_CIT_P_RdbConfig			sRdbConfiguration;		/* RDB Configuration*/


	BDBG_ENTER(BRAP_SIPS_P_SeamlessIpPortSwitchAPI);

	/* Inititalization */
	ui32Error = BERR_SUCCESS;

	/* Get the RDB configuration */
	sRdbConfiguration.sRdbCfg = psSipsIp->sRdbCfg;

	/* Prepare the RDB offset */
	BRAP_CITGEN_P_PrepareTaskRdbCfg(&sRdbConfiguration);


	/* Get the SIPS Command  */
	eAddRemoveIpPort = psSipsIp->eAddRemoveIpPort;

	BDBG_MSG(("======================================================="));	
	BDBG_MSG(("	EXECUTING SEAMLESS INPUT PORT SWITCHING	 SIPS - API   "));	
	BDBG_MSG(("======================================================="));		
	
	/* Process the SIPS command */
	if(BRAP_SIPS_P_AddRemoveIpPort_eAddPort == eAddRemoveIpPort)
	{
		/* Adding an input port for the running TASK */

		ui32Error = BRAP_SIPS_P_AddInputPortToTask(
								hHeap,
								sRdbConfiguration.ui32TaskRdbOffset,
								psSipsIp,
								psSipsOp
							);

	}
	else if((BRAP_SIPS_P_AddRemoveIpPort_eRemovePort == eAddRemoveIpPort))
	{
		/* Removing an input port from the running TASK */
		ui32Error = BRAP_SIPS_P_RemoveInputPortFromTask(										
								psSipsIp,
								psSipsOp
							);
	}
	else
	{
		/* Update the Status of Execution */
		psSipsOp->eExecutionStatus = BRAP_SIPS_P_ExecutionStatus_eFail;

		ui32Error = BERR_INVALID_PARAMETER;

		BDBG_ERR((" SIPS API : Unknown Command [%x]",(uint32_t)eAddRemoveIpPort));
	}

	
	
	BDBG_LEAVE(BRAP_SIPS_P_SeamlessIpPortSwitchAPI);

	return ui32Error;  

}

/******************************************************************************
Summary:
	This is the top level function to remove an input port from a running task.

Description: 

	The operation of SIPS API 

		1) Copy the CIT contents of the running to Task to Working buffer
		2) Get the port index to be removed
		3) Remove Port
		4) Return Error Status
				
Input:

	psSipsIp : Input Data Strucure for the SIPS API

Output:		

	psSipsIp : Working CIT buffer gets changed
	psSipsOp : Execution status is present here

Returns:

	Error : Execution error while removing port

******************************************************************************/
uint32_t BRAP_SIPS_P_RemoveInputPortFromTask(
					BRAP_SIPS_InputInfo			*psSipsIp,
					BRAP_SIPS_OutputInfo		*psSipsOp	
				)
{
	uint32_t ui32Error;
	uint32_t ui32InputPortIndexToRemove;

	BDBG_ENTER(BRAP_SIPS_P_RemoveInputPortFromTask);


	/* Initialization */
	ui32Error = BERR_SUCCESS;

	/* Copy the Present CIT struture to the Working Buffer */
	BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer(psSipsIp);

	/* Get the input port to get removed */
	ui32InputPortIndexToRemove = psSipsIp->ui32InputPortIndexToRemove;

	/* Remove port */
	ui32Error = BRAP_SIPS_P_RemoveInputPort(
									psSipsIp,
									ui32InputPortIndexToRemove
								);


	/* Populates SIPS Output Strucuture with Execution Status*/
	if(BERR_SUCCESS != ui32Error)
	{
		psSipsOp->eExecutionStatus = BRAP_SIPS_P_ExecutionStatus_eFail;
	}
	else
	{
		BDBG_MSG(("SIPS : An input port is removed successfully"));
		psSipsOp->eExecutionStatus = BRAP_SIPS_P_ExecutionStatus_eSuccess;
	}


	BDBG_LEAVE(BRAP_SIPS_P_RemoveInputPortFromTask);

	return ui32Error;
}

/******************************************************************************
Summary:
	This function copies the CIT Data Structure of the running task to the 
	working buffer for further modification.

	The Working CIT buffer is allocated by PI.

Description: 

Input:

	psSipsIp	: Input Data Strucure for the SIPS API
				  CIT Data Structure pointer is used as input
Output:		

	psSipsIp	: Input Data Strucure for the SIPS API
				  Working CIT buffer is used as output
				  
Returns:
	None
******************************************************************************/

void BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer(
							BRAP_SIPS_InputInfo		*psSipsIp	
						)
{
	BRAP_AF_P_sTASK_CONFIG	*psRunningTaskCitBuffAddr;
	BRAP_AF_P_sTASK_CONFIG	*psWorkingTaskCitBuffAddr;


	BDBG_ENTER(BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer);

	/* Getting the Structure pointers for CIT and Working Buffer*/
	psRunningTaskCitBuffAddr 
					= (BRAP_AF_P_sTASK_CONFIG *)psSipsIp->ui32CitDataStructureDramAddr;

	psWorkingTaskCitBuffAddr 
					= (BRAP_AF_P_sTASK_CONFIG *)psSipsIp->ui32WorkingCitDataStructureDramAddr;


	/* Copy the DataStructure */	
	*psWorkingTaskCitBuffAddr =  *psRunningTaskCitBuffAddr;

	
	BDBG_LEAVE(BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer);
}


/******************************************************************************
Summary:
	This function removes an input port from the running task

Description: 

	The operation of SIPS API 

		1) Get Working Buffer pointer
		2) Get the port index to be removed
		3) Get the Node0/2 Configuration
		4) Remove Port
				
		Error :
				Detects the port to be removed is already active or not.		
Input:

	psSipsIp					: Input Data Strucure for the SIPS API
	ui32InputPortIndexToRemove  : 

Output:		

	psSipsIp : Working CIT buffer gets changed	

Returns:

	Error : Execution error while removing port

******************************************************************************/
uint32_t BRAP_SIPS_P_RemoveInputPort(
						BRAP_SIPS_InputInfo		*psSipsIp,
						uint32_t				ui32InputPortIndexToRemove
				)
				
{
	uint32_t					ui32Error;

	BRAP_AF_P_sNODE_CONFIG		*psNodeConfig;
	BRAP_AF_P_sTASK_CONFIG		*psWorkingTaskCitBuffAddr;


	BDBG_ENTER(BRAP_SIPS_P_RemoveInputPort);

	/* Initialization */
	ui32Error = BERR_SUCCESS;

	/* Getting the Working Buffer pointer */
	psWorkingTaskCitBuffAddr = (BRAP_AF_P_sTASK_CONFIG *)psSipsIp->ui32WorkingCitDataStructureDramAddr;


	/* Getting the handle for the Node0 Configuration */
	psNodeConfig			 = &psWorkingTaskCitBuffAddr->sNodeConfig[0];
	
	if(BRAP_AF_P_eInvalid == psNodeConfig->eNodeIpValidFlag[ui32InputPortIndexToRemove])
	{
		BDBG_ERR(("ERROR:SIPS API : Removing Port is unsucessfull. \
						The port Number = [%d] to be removed is not active",ui32InputPortIndexToRemove));

		return BERR_NOT_SUPPORTED;
	}
	else
	{
		/* Getting the handle for the Node0 Configuration and disabling the port*/
		psNodeConfig = &psWorkingTaskCitBuffAddr->sNodeConfig[0];

		psNodeConfig->eNodeIpValidFlag[ui32InputPortIndexToRemove] = BRAP_AF_P_eInvalid;

		
		/* Getting the handle for the Node2 Configuration and disabling the port*/
		psNodeConfig = &psWorkingTaskCitBuffAddr->sNodeConfig[BRAP_CIT_P_NUM_SECIAL_NODES];	

		psNodeConfig->eNodeIpValidFlag[ui32InputPortIndexToRemove] = BRAP_AF_P_eInvalid;
	}

	BDBG_LEAVE(BRAP_SIPS_P_RemoveInputPort);

	return ui32Error;
}

/******************************************************************************
Summary:
	This function adds an input port to the running task

Description: 

	The operation of SIPS API 

		1) Get Working Buffer pointer
		2) Get the free input port
		3) Add Port
				
		Error :
				Detects the port to be removed is already active or not.		
Input:

	hHeap				: Heap
	ui32TaskRdbOffset	: RDB Offset

	psSipsIp			: Input Data Strucure for the SIPS API
	
Output:		

	psSipsIp			: Working CIT buffer gets changed	

Returns:

	Error : Execution error while adding port

******************************************************************************/

uint32_t BRAP_SIPS_P_AddInputPortToTask(
					BMEM_Handle				hHeap,
					uint32_t				ui32TaskRdbOffset,
					BRAP_SIPS_InputInfo		*psSipsIp,
					BRAP_SIPS_OutputInfo	*psSipsOp	
				)
{

	uint32_t ui32Error;
	uint32_t ui32FreeIpPortIndex;

	BDBG_ENTER(BRAP_SIPS_P_AddInputPortToTask);

	ui32Error = BERR_SUCCESS;


	/* Copy the Present CIT struture to the Working Buffer */
	BRAP_SIPS_P_CopyRunningTaskCitToWorkingBuffer(psSipsIp);


	/* Search for any free ports to hook */
	ui32Error = BRAP_SIPS_P_SearchForFreeInputPort(
									psSipsIp,
									&ui32FreeIpPortIndex
								);

	if(BERR_SUCCESS != ui32Error)
	{		
		/* Error Status updation */
		psSipsOp->eExecutionStatus = BRAP_SIPS_P_ExecutionStatus_eFail;

		return ui32Error;
	}
	else
	{
		/*Hook the new port and set the port valid flag */
		ui32Error = BRAP_SIPS_P_HookInputPort(
								hHeap,
								ui32TaskRdbOffset,
								psSipsIp,
								ui32FreeIpPortIndex
							);
	}	

	/* Update the SIPS output data structure */
	if(BERR_SUCCESS == ui32Error)
	{
		psSipsOp->eExecutionStatus			= BRAP_SIPS_P_ExecutionStatus_eSuccess;
		psSipsOp->ui32InputPortIndexAdded	= ui32FreeIpPortIndex;

		BDBG_MSG(("SIPS-API : A new input port is added successfully"));
	}
	else
	{
		psSipsOp->eExecutionStatus			= BRAP_SIPS_P_ExecutionStatus_eFail;
		psSipsOp->ui32InputPortIndexAdded	= 0x7FFFFFFF;
		
		/* Error Status would be printed in the 'BRAP_SIPS_P_HookInputPort' */
	}


	BDBG_LEAVE(BRAP_SIPS_P_AddInputPortToTask);

	return ui32Error;

}

/******************************************************************************
Summary:
	This function searches for free input port index to hook new port

Description: 

	The operation of SIPS API 

		1) Get Working Buffer pointer
		2) Search for the free port index
		3) Check for error status
		4) Return the Free port index
				Return Port Index as 0x7FFFFFFF upon detecting error.				
Input:

	psSipsIp			: Input Data Strucure for the SIPS API	
	
Output:		

	ui32FreeIpPortIndex	: Free Input port Index	

Returns:

	Error : Execution error while searching free port

******************************************************************************/
uint32_t BRAP_SIPS_P_SearchForFreeInputPort(
							BRAP_SIPS_InputInfo		*psSipsIp,
							uint32_t				*pui32FreeIpPortIndex
						)
{
	uint32_t ui32Error;
	uint32_t ui32SrcCount;

	BRAP_SIPS_P_FreeIpPortFound eFreeInputFoundFlag;


	BRAP_AF_P_sNODE_CONFIG	*psNodeConfig;
	BRAP_AF_P_sTASK_CONFIG	*psWorkingTaskCitBuffAddr;


	BDBG_ENTER(BRAP_SIPS_P_SearchForFreeInputPort);

	ui32Error = BERR_SUCCESS;

	psWorkingTaskCitBuffAddr	= (BRAP_AF_P_sTASK_CONFIG *)psSipsIp->ui32WorkingCitDataStructureDramAddr;

	/* Getting the handle for the Node0 Configuration */
	psNodeConfig				= &psWorkingTaskCitBuffAddr->sNodeConfig[0];

	/* Initialization to invalid */
	eFreeInputFoundFlag			= BRAP_SIPS_P_FreeIpPortFound_eAbsent;
	

	for(ui32SrcCount=0;ui32SrcCount<BRAP_AF_P_MAX_IP_FORKS;ui32SrcCount++)
	{
		if(BRAP_AF_P_eInvalid == psNodeConfig->eNodeIpValidFlag[ui32SrcCount] )
		{
			eFreeInputFoundFlag = BRAP_SIPS_P_FreeIpPortFound_ePresent;
			break;
		}
	}

	/* Checking the Port detection flag */
	if(BRAP_SIPS_P_FreeIpPortFound_ePresent == eFreeInputFoundFlag)
	{
		ui32Error			  =	BERR_SUCCESS;

		*pui32FreeIpPortIndex = ui32SrcCount;/* Port index*/
	}
	else
	{
		ui32Error			  =	BERR_LEAKED_RESOURCE;

		*pui32FreeIpPortIndex = 0x7FFFFFFF; /* Invalid Case */

		BDBG_ERR(("Error: SIPS-API : Adding a port Failed. Could not find the free input available for the present task to add a new input port "));
	}

	BDBG_LEAVE(BRAP_SIPS_P_SearchForFreeInputPort);

	return ui32Error;
}

/******************************************************************************
Summary:
	This function hooks an input port to the running task

Description: 

	The operation of SIPS API 

		1) Get Working Buffer pointer
		2) Convert the Io and Io Generic Buffer address
		3) Add new port and set the port validity field
		4) Set the input port validity field for Node.2
				
				
Input:

	hHeap				: Heap
	ui32TaskRdbOffset	: RDB Offset

	psSipsIp			: Input Data Strucure for the SIPS API
	ui32FreeIpPortIndex	: Free Input port Index
	
Output:		

	psSipsIp			: Working CIT buffer gets changed	

Returns:

	Error : Execution error while adding port

******************************************************************************/
uint32_t BRAP_SIPS_P_HookInputPort(
							BMEM_Handle				hHeap,
							uint32_t				ui32TaskRdbOffset,
							BRAP_SIPS_InputInfo		*psSipsIp,
							uint32_t				ui32FreeIpPortIndex
						)
{

	uint32_t ui32Error;	

	uint32_t ui32NodeIpBuffCfgAddr;
	uint32_t ui32NodeIpGenericDataBuffCfgAddr;

	BRAP_AF_P_sIO_BUFFER			sIoBuffTemp;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffTemp;


	BRAP_AF_P_sNODE_CONFIG			*psNodeConfig;
	BRAP_AF_P_sTASK_CONFIG			*psWorkingTaskCitBuffAddr;
	BRAP_CIT_P_FwStgSrcDstDetails	*psInputSrcDetails;


	BDBG_ENTER(BRAP_SIPS_P_HookInputPort);

	ui32Error = BERR_SUCCESS;

	psWorkingTaskCitBuffAddr = (BRAP_AF_P_sTASK_CONFIG *)psSipsIp->ui32WorkingCitDataStructureDramAddr;

	/* Getting the handle for the Node0 Configuration */
	psNodeConfig			 = &psWorkingTaskCitBuffAddr->sNodeConfig[0];	

	/* Getting the IoBuffer and IoGeneric Buffer Addr */
	ui32NodeIpBuffCfgAddr    = psNodeConfig->ui32NodeIpBuffCfgAddr[ui32FreeIpPortIndex];

	ui32NodeIpGenericDataBuffCfgAddr 
							 = psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[ui32FreeIpPortIndex];

	/* Getting the New port handle */
	psInputSrcDetails = &psSipsIp->sInputSrcDetails;

	if(BRAP_CIT_P_FwStgSrcDstType_eFwStg != psInputSrcDetails->eType)
	{

		switch(psInputSrcDetails->eType)
		{
			case BRAP_CIT_P_FwStgSrcDstType_eFMMBuf:

				/* Translate FMM Buffer Ids to Addresses */
				BRAP_CITGEN_P_TransFmmBuffId2Addr(
					&psInputSrcDetails->uDetails.sIoBuf.uIoBuffer.sIoBuffId,
					&sIoBuffTemp,
					&psInputSrcDetails->uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
					&sIoGenericBuffTemp,
					ui32TaskRdbOffset,
					BRAP_CIT_P_MasterSlaveType_eOthers
				);

				/* Write the configuration structure into DRAM */
				/* First I/O buff Cfg struct */
				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t) &sIoBuffTemp,
					ui32NodeIpBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER) 
				);

				/*	Next Generic buff Cfg struct */
				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t) &sIoGenericBuffTemp,
					ui32NodeIpGenericDataBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) 
				);

				/*Making the Input port valid */
				psNodeConfig->eNodeIpValidFlag[ui32FreeIpPortIndex] = BRAP_AF_P_eValid;
				break;

			case BRAP_CIT_P_FwStgSrcDstType_eRDB:

				/*	Translate RDB Buffer Ids to Addresses */
				BRAP_CITGEN_P_TransRdbBuffId2Addr(
					&psInputSrcDetails->uDetails.sIoBuf.uIoBuffer.sIoBuffId,
					&sIoBuffTemp,
					&psInputSrcDetails->uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffId,
					&sIoGenericBuffTemp,
					ui32TaskRdbOffset
				);

				/*	Write the configuration structure into DRAM */
				/*	First I/O buff Cfg struct */
				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t) &sIoBuffTemp,
					ui32NodeIpBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER)
				);

				/*	Next Generic buff Cfg struct */
				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t) &sIoGenericBuffTemp,
					ui32NodeIpGenericDataBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER) 
				);

				/*Making the Input port valid */
				psNodeConfig->eNodeIpValidFlag[ui32FreeIpPortIndex] = BRAP_AF_P_eValid;

				break;

			case BRAP_CIT_P_FwStgSrcDstType_eInterTaskDRAMBuf:

				psNodeConfig->ui32NodeIpBuffCfgAddr[ui32FreeIpPortIndex]	
						= psInputSrcDetails->uDetails.sInterTaskDramBuffCfg.ui32IoBuffCfgAddr;

				
				psNodeConfig->ui32NodeIpGenericDataBuffCfgAddr[ui32FreeIpPortIndex]
						= psInputSrcDetails->uDetails.sInterTaskDramBuffCfg.ui32IoGenericBuffCfgAddr;	

				/* Making the Input port valid */
				psNodeConfig->eNodeIpValidFlag[ui32FreeIpPortIndex] = BRAP_AF_P_eValid;

				break;					
				
			case BRAP_CIT_P_FwStgSrcDstType_eRaveBuf:
			/* case BRAP_CIT_P_FwStgSrcDstType_eDram:*/
			default:

				/*	Write the configuration structure into DRAM */
				/*	First I/O buff Cfg struct */

				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t)&psInputSrcDetails->uDetails.sIoBuf.uIoBuffer.sIoBuffer,
					ui32NodeIpBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER)
				);								
							
				/*	Next Generic buff Cfg struct */
				BRAP_CITGEN_P_WriteToDRAM( 
					hHeap,
					(uint32_t)&psInputSrcDetails->uDetails.sIoBuf.uIoGenericBuffer.sIoGenericBuffer,
					ui32NodeIpGenericDataBuffCfgAddr,
					(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER)
				);

				/* Making the Input port valid */
				psNodeConfig->eNodeIpValidFlag[ui32FreeIpPortIndex] = BRAP_AF_P_eValid;

				break;

		}
	}
	else
	{
		ui32Error = BERR_INVALID_PARAMETER;

		BDBG_ERR(("Error : SIPS-API : Error while adding a new port. The new port added is of type %s ",FwConnectivityType[psInputSrcDetails->eType]));

		return ui32Error;
	}
 
	/* Enabling the Valid Flag for the Node 2 */
	psNodeConfig			= &psWorkingTaskCitBuffAddr->sNodeConfig[BRAP_CIT_P_NUM_SECIAL_NODES];	

	psNodeConfig->eNodeIpValidFlag[ui32FreeIpPortIndex] 
							= BRAP_AF_P_eValid;


	BDBG_LEAVE(BRAP_SIPS_P_HookInputPort);

	return ui32Error;

}







/*---------------------------------------------------------------------
				Top level SCM CIT Generation Function 
---------------------------------------------------------------------*/
uint32_t BRAP_P_GenCitScm(
						BMEM_Handle					hHeap,	
						BREG_Handle					hRegister,						
						BRAP_CIT_P_InputInfo		*psCitIp,						
						BRAP_CIT_P_ScmCITOutput	    *psScmCitOp
					  )
{
	uint32_t ui32Err;
	uint32_t ui32PhysAddr;
	uint32_t ui32IfBuffBaseAddr;	
	uint32_t ui32TotalTaskMemory;	
	uint32_t ui32TotalSharedMemory;
	uint32_t ui32UsrCfgBuffBaseAddr;
												
	uint32_t ui32ScratchBuffBaseAddr;
	uint32_t ui32FwStatusBuffBaseAddr;	
	uint32_t ui32InterStgBuffBaseAddr;
	uint32_t ui32IoBuffCfgStructBaseAddr;		
	

	BRAP_CIT_P_sTaskBuffInfo				sTaskBuffInfo;
	BRAP_AF_P_sDRAM_BUFFER					sTaskStackBuff;
									
	BRAP_CIT_P_sCitDataStructure			*psCitDataStructure =NULL;


	BDBG_ENTER(BRAP_P_GenScmCit);		

	BDBG_ASSERT(hHeap);
	BDBG_ASSERT(psCitIp);
	BDBG_ASSERT(psScmCitOp);
    BSTD_UNUSED(hRegister);


	ui32Err = BERR_SUCCESS;

	/*---------------------------------------------------*/
	/*Memory Allocation for the CIT Global Datastructure */
	/*---------------------------------------------------*/

	ui32Err = BRAP_CITGEN_P_AllocateCitDatastructure(&psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{	
		return ui32Err;
	}


	/* Initializing the Contents of CitDataStructure */
	BRAP_CITGEN_P_InitializeCitDatastructue(psCitDataStructure);
			
	/*	First convert the virtual address in the CIT input structure to physical 
		addresses 
	*/
	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32SharedMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32SharedMemPhysAdr = ui32PhysAddr;

	BRAP_P_ConvertAddressToOffset(	hHeap, 
									(void *)(psCitIp->sMemInfo.ui32TaskMemPhysAdr),
									&ui32PhysAddr
								 );

	psCitIp->sMemInfo.ui32TaskMemPhysAdr = ui32PhysAddr;

	/*Geting the RDB Offsets Configuration */

	BRAP_CITGEN_P_GetCitInputRdbCfg(psCitIp,
									&psCitDataStructure->sRdbCfg
								 );
	

	/*	Prepare the node network information. This function basically copies
		all the information from CIT i/p structure to AlgoNwInfo structure 
	*/
	BRAP_CITGEN_P_PrepareAlgoNwStruct(
									psCitIp, 
									&psCitDataStructure->sAlgoNwInfo 
								 );	


	/* 
		Detects special tasks in the system. Currently only detecting the Mixer Task alone.
	*/
	BRAP_CITGEN_P_DetectSpecialTasks(
									&psCitDataStructure->sAlgoNwInfo,
									&psCitDataStructure->sSpecialTaskPresent
								 );



	/*	From the algo network, decide the execution order. The order
		of execution will be indicated by linearly traversing the 
		sAlgoExecOrder structure. This structure will contain the branchId 
		and StageId of the Algo, in the order of execution */

	ui32Err = BRAP_CITGEN_P_DetermineAlgoExecOrder(
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure->sAlgoExecOrder
								 );

	if( ui32Err != BERR_SUCCESS)
	{	
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Associate the Algo ids of nodes with the algorithm in the 
		Netwrok info structure. */

	ui32Err = BRAP_CITGEN_P_GetNodesInAlgo(psCitDataStructure);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the MIT information into Algo Network Info */
	ui32Err = BRAP_CITGEN_P_UpdtMitInfoToAlgoNw(
									hHeap,
									psCitIp->psMit,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Update all the information in the BRAP_sNodeInfo structure into the  Algo Network */
	BRAP_CITGEN_P_UpdtNodeInfoToAlgoNw(
									BRAP_sNodeInfo,
									&psCitDataStructure->sAlgoNwInfo
								);

	/* Preparing the task RDB configuration */
	BRAP_CITGEN_P_PrepareTaskRdbCfg(&psCitDataStructure->sRdbCfg);



							/*---------------------------*/
							/* TASK BUFFER CONFIGURATION */
							/*---------------------------*/

	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Function to Compute the user config and inter-frame
		buffers for the nodes in the algo netwrok
	*/
	ui32Err = BRAP_CITGEN_P_ComputeIfUsrCfgBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status buffer */
	/*
		Function to Compute the Firmware status buffer
		for the nodes in the algo network
	*/
	ui32Err = BRAP_CITGEN_P_ComputeFwStatusBuffSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								);
		
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}


	/*	This function computes the worst case size required for the I/O
		buffer configuration structures in DRAM. The worst case size is
		computed, assuming each algorithm can have different i/o buffer
		structures 
	*/
	BRAP_CITGEN_P_ComputeIoBuffCfgStructSizes(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo 
								 );

	/* Adding Stack swap */
	/*	Computing the Stack swap size for the task.
		Now it is hardcoded 
	*/
	BRAP_CITGEN_P_ComputeTaskStackBuffSize(&sTaskBuffInfo);

	sTaskStackBuff.ui32BufferSizeInBytes 
						= sTaskBuffInfo.ui32TaskStackMemSize;


	/* Buffer Allocation */
	/*-------------------*/
		
	/*	A single chunk of memeory is allocated for the user
		config and the IF buffers. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalTaskMemory =
			sTaskBuffInfo.ui32TaskUsrCfgMemSize				+		
			sTaskBuffInfo.ui32TaskInterFrmMemSize			+
			sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize	+
			sTaskBuffInfo.ui32TaskFwStatusBuffMemSize		+
			sTaskBuffInfo.ui32TaskStackMemSize	
		;


	if(ui32TotalTaskMemory > psCitIp->sMemInfo.ui32TaskMemSize)
	{
		BDBG_ERR(("ERROR: Task Memory required is more than allocated"));	
		BDBG_ERR(("\t Task Memory required= %d  Allocated memory = %d",ui32TotalTaskMemory,psCitIp->sMemInfo.ui32TaskMemSize));	
		BDBG_ERR((""));

		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}	
	
	ui32UsrCfgBuffBaseAddr		=	psCitIp->sMemInfo.ui32TaskMemPhysAdr;

	ui32IfBuffBaseAddr			=	ui32UsrCfgBuffBaseAddr				 + 
											sTaskBuffInfo.ui32TaskUsrCfgMemSize;

	ui32IoBuffCfgStructBaseAddr =	ui32IfBuffBaseAddr					 + 
											sTaskBuffInfo.ui32TaskInterFrmMemSize;

	ui32FwStatusBuffBaseAddr	=	ui32IoBuffCfgStructBaseAddr			 +
											sTaskBuffInfo.ui32TaskIoBuffCfgStructMemSize;

	sTaskStackBuff.ui32DramBufferAddress 
								=	ui32FwStatusBuffBaseAddr			 +
											sTaskBuffInfo.ui32TaskFwStatusBuffMemSize;
	


	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	Assign Inter-frame and user config addresses to nodes in the
		algos of the network
	*/
	ui32Err = BRAP_CITGEN_P_AllocInterFrmUserCfgBuff(
									ui32IfBuffBaseAddr,
									ui32UsrCfgBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo 
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}


	/* Adding Fw Status*/
	/* Assign FW status  addresses to stages in the network */
	ui32Err = BRAP_CITGEN_P_AllocFwStatusBuff(
									ui32FwStatusBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								);	

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*-------------------------------------------------------------------------------*/


							/*---------------------------------*/
							/* INTERSTAGE BUFFER CONFIGURATION */
							/*---------------------------------*/


	/* Computation of buffer size required*/
	/*------------------------------------*/

	/*	Compute the worst case size required for Inter-stage buffers.
		The inter-stage buffers includes the inter-stage I/O buffer 
		as well as the inter-stage generic buffer
	*/
	ui32Err = BRAP_CITGEN_P_ComputeInterStgBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Compute the Worst case scratch buffer size required by any node 
		in the configuration
	*/
	BRAP_CITGEN_P_ComputeScratchBuffSize(
									&psCitDataStructure->sAlgoNwInfo,
									&sTaskBuffInfo
								);




	/* Buffer Allocation */
	/*-------------------*/

	/*	A single chunk of memeory is allocated for the inter-stage and scratch
		buffer. Check if the size allocated is sufficient
		for the current configuration before allocating the actual memory 
	*/

	ui32TotalSharedMemory = 
			sTaskBuffInfo.ui32TaskInterStgIoMemSize			+ 
			sTaskBuffInfo.ui32TaskInterStgGenericMemSize	+
			sTaskBuffInfo.ui32TaskScratchMemSize;

	if(ui32TotalSharedMemory >	psCitIp->sMemInfo.ui32SharedMemSize)
	{

		BDBG_ERR(("ERROR: Shared Memory required is more than allocated"));	
		BDBG_ERR(("\t Shared Memory required= %d  Allocated memory = %d",ui32TotalSharedMemory,psCitIp->sMemInfo.ui32SharedMemSize));	
		BDBG_ERR((""));
		ui32Err = BERR_OUT_OF_SYSTEM_MEMORY;

	  goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	ui32InterStgBuffBaseAddr	=	psCitIp->sMemInfo.ui32SharedMemPhysAdr;

	ui32ScratchBuffBaseAddr		=	ui32InterStgBuffBaseAddr					+ 
							 		sTaskBuffInfo.ui32TaskInterStgIoMemSize		+ 
									sTaskBuffInfo.ui32TaskInterStgGenericMemSize;

	/* Buffer memory configuration */
	/*-----------------------------*/

	/*	This is the main function that allocates I/O buffer configuration.
		This function first prepares the I/O buffer configurations structures
		for inter-stage buffers. It copies the I/O port configuration
		from the sAlgoNwInfo to DRAM and updates the address into 
		NodeAttr structure. 
		Finally it allocates which I/O buffer configuration to use for
		inter-stage buffers based on the branching information
	*/
	ui32Err = BRAP_CITGEN_P_AllocIoBuffCfg(	hHeap,
											ui32InterStgBuffBaseAddr,
											ui32IoBuffCfgStructBaseAddr,
											psCitDataStructure->sAlgoExecOrder,
											&sTaskBuffInfo,
											&psCitDataStructure->sAlgoNwInfo,
											&psCitDataStructure->sOpPortInfo,
											&psCitDataStructure->sTaskFmmGateOpenConfig,
											psCitDataStructure->sRdbCfg.ui32TaskRdbOffset,
											psCitDataStructure,
											psCitIp->eIndepDelayEnableFlag,
											BRAP_CIT_P_SameInputForFsAndDecode_eDisable,
											&psCitDataStructure->sSpecialTaskPresent
										  );

	
	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}

	/*	Print the Input/Output port configuration for all the stages.
		This function also prints the interstage buffer used for all output ports and
		the intermediate nodes
	*/
	BRAP_CITGEN_P_PrintStagePortCtyInterStageAlloc(
									psCitDataStructure->sAlgoExecOrder,
									&psCitDataStructure->sAlgoNwInfo,
									psCitDataStructure
								);



	/*	Allocate scratch buffer base address in sAlgoNwInfo. This function is
		written just for completion of information in the node atribute
		structure of each node 
	*/
	BRAP_CITGEN_P_AllocScratchBuff( ui32ScratchBuffBaseAddr,
									&psCitDataStructure->sAlgoNwInfo
								 );	
	
	
						/*--------------------------------------*/
						/*				Processing				*/
						/*--------------------------------------*/


	/*	Now that we have all the components of Node config, fill in the node config into CIT  */
	ui32Err = BRAP_CITGEN_P_FillNodeCfgIntoScmCit(	
										&psCitDataStructure->sAlgoNwInfo,
										psCitDataStructure->sAlgoExecOrder,										
										&psScmCitOp->sScmCit
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}


	
	ui32Err = BRAP_CITGEN_P_FillGblTaskCfgIntoScmCit( 										
										&psCitDataStructure->sAlgoNwInfo,
										ui32ScratchBuffBaseAddr,
										sTaskBuffInfo.ui32TaskScratchMemSize,									
										&psScmCitOp->sScmCit.sGlobalTaskConfig
									);

	if( ui32Err != BERR_SUCCESS)
	{
		goto BRAP_SCM_CITGENMODULE_P_EXIT_POINT;
	}
	
	/*	Populate the CIT o/p structure */
	BRAP_CITGEN_P_FillScmCitOp(
								hHeap,
								&sTaskStackBuff,										
								&psCitDataStructure->sAlgoNwInfo,
								psScmCitOp 
							);



	/*---------------------------------------------------*/
	/* SCM CIT Generation Module is executed successfully*/
	/*---------------------------------------------------*/	
	BDBG_MSG((" SCM CIT Generation Module is executed successfully"));
	BDBG_MSG((""));	

	ui32Err = BERR_SUCCESS;


	/* EXIT Point */
	BRAP_SCM_CITGENMODULE_P_EXIT_POINT:
	/*--------------------------------------*/
	/* Free the Cit global memory allocated */
	/*--------------------------------------*/	
	BRAP_CITGEN_P_FreeCitDatastructure(&psCitDataStructure);

	/* Check for Error and assert */
	if(ui32Err !=BERR_SUCCESS)
	{
		BDBG_ASSERT(0);
	}

	BDBG_LEAVE(BRAP_P_GenScmCit);		

	return ui32Err;	

}
/******************************************************************************
Summary:
	Fill the node cfg of SCM CIT

Description: 

	Filling node config information of CIT fata Structure 
			
Input:
			hHeap						: Heap
			psAlgoNwInfo				: Algorithm Network information
			sAlgoExecOrder				: Algorithm Execution 

Output:
			psScmCit					: Node Cfg of SCM CIT
Returns:
	
			ui32Error
******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillNodeCfgIntoScmCit(	
						BRAP_CIT_P_AlgoNwInfo				*psAlgoNwInfo,
						BRAP_CIT_P_sAlgoExecOrder			sAlgoExecOrder[],
						BRAP_SCM_P_sTASK_CONFIG				*psScmCit 
					)
{
	uint32_t				ui32Error;
	uint32_t				ui32BranchNum;
	uint32_t				ui32Algo, ui32Node;
	uint32_t				ui32NumAlgosInNw;
	uint32_t				ui32BranchId, ui32StageId;
	uint32_t				ui32NumNodesInAlgo;
	uint32_t				ui32NodeIndex;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_AF_P_sNODE_CONFIG	*psNodeCfg;


	BDBG_ENTER(BRAP_CITGEN_P_FillNodeCfgIntoScmCit);

	ui32Error = BERR_SUCCESS;

	/*	Find the number of algos within the network. */
	ui32NumAlgosInNw = 0;
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgosInNw += 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
	}	

	/*	Update the CIT nodes based on execution order */
	psNodeCfg		= psScmCit->sNodeConfig;
	ui32NodeIndex	= 0;

	for( ui32Algo=0; ui32Algo<ui32NumAlgosInNw; ui32Algo++)
	{
		ui32BranchId	= sAlgoExecOrder[ui32Algo].ui32BranchId;
		ui32StageId		= sAlgoExecOrder[ui32Algo].ui32StageId;

		psStgInfo		= &psAlgoNwInfo->psBranchInfo[ui32BranchId]->sStgInfo[ui32StageId];
		
		ui32NumNodesInAlgo = psStgInfo->ui32NumNodes;
		for( ui32Node=0; ui32Node<ui32NumNodesInAlgo; ui32Node++ )
		{
			psNodeAttr			= &psStgInfo->sNodeAttr[ui32Node];
			psNodeCfg->uiNodeId = ui32NodeIndex;

			/* Populating the Collect Residual Flag */
			psNodeCfg->eCollectResidual = (psNodeAttr->ui32BranchId == 0) ? BRAP_AF_P_eEnable : BRAP_AF_P_eDisable;


			/*	This function updates the Node config with the parameters in 
				the	node attributes structure 
			*/
			ui32Error = BRAP_CITGEN_P_UpdtNodeCfgInCit (
												psNodeAttr,
												psNodeCfg 
											);

			if(ui32Error != BERR_SUCCESS)
			{
				return ui32Error;
			}

			psNodeCfg++;
			ui32NodeIndex++;
		}
	}


	BDBG_LEAVE(BRAP_CITGEN_P_FillNodeCfgIntoScmCit);

	return ui32Error;
}

/******************************************************************************
Summary:

	Fill the global task configuration of SCM CIT

Description: 

	Global task configuration of SCM CIT is done here
			
Input:
		hHeap						: Heap
		psAlgoNwInfo				: Algorithm Network information
		ui32ScratchBuffBaseAddr		: Scratch Buffer Base Addre
		ui32TaskScratchMemSize		: Scratch Buffer Size
	
Output:
		sGblTaskCfg

Returns:
		ui32Error
******************************************************************************/
static uint32_t BRAP_CITGEN_P_FillGblTaskCfgIntoScmCit(	
						BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,						
						uint32_t						ui32ScratchBuffBaseAddr,
						uint32_t						ui32TaskScratchMemSize,
						BRAP_SCM_P_sGLOBAL_TASK_CONFIG	*psGblTaskCfg						
					)
{
	uint32_t	ui32Error;
	uint32_t	ui32BranchNum;
	uint32_t	ui32AlgoStg;
	uint32_t	ui32NumAlgoStgInBranch;
	uint32_t	ui32NumNodesInNw;

	BRAP_CIT_P_StageInfo	*psStgInfo;
	
	BDBG_ENTER(BRAP_CITGEN_P_FillGblTaskCfgIntoScmCit);
	
	ui32NumNodesInNw	= 0;
	ui32Error			= BERR_SUCCESS;


	/*	Compute the number of nodes in the network */
	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo = &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			ui32NumNodesInNw += psStgInfo->ui32NumNodes;
		}
	}

	if(ui32NumNodesInNw > BRAP_AF_P_MAX_NODES)
	{
		BDBG_ERR(("Error : The number of nodes in the system is %d. Maximum Allowed is %d", ui32NumNodesInNw,BRAP_AF_P_MAX_NODES));

		return(BERR_NOT_SUPPORTED);
	}

	psGblTaskCfg->ui32NumberOfNodesInTask = ui32NumNodesInNw;

	/*	Fill in the scratch buffer details */
	psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress = ui32ScratchBuffBaseAddr;
	psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes = ui32TaskScratchMemSize;

	/*	Start node index */
	psGblTaskCfg->ui32StartNodeIndexOfCoreScmAlgo = 0;	/*Start Node index in SCM is 0 */
	
	BDBG_LEAVE(BRAP_CITGEN_P_FillGblTaskCfgIntoScmCit);

	return ui32Error;
}

/******************************************************************************
Summary:

	Fill the SCM CIT output Structure to PI

Description: 

	Global task configuration of SCM CIT is done here
			
Input:
		hHeap						: Heap
		psAlgoNwInfo				: Algorithm Network information
		psTaskStackBuff				: Stack Swap Addre		
	
Output:
		psCitOp

Returns:
		ui32Error
******************************************************************************/
static void BRAP_CITGEN_P_FillScmCitOp(
					BMEM_Handle						hHeap,
					BRAP_AF_P_sDRAM_BUFFER			*psTaskStackBuff,					
					BRAP_CIT_P_AlgoNwInfo			*psAlgoNwInfo,
					BRAP_CIT_P_ScmCITOutput			*psCitOp
				)
{
	uint32_t				ui32Node;
	uint32_t				ui32BranchNum;
	uint32_t				ui32AlgoStg;
	uint32_t				ui32NumAlgoStgInBranch;
	BRAP_CIT_P_StageInfo	*psStgInfo;
	BRAP_CIT_P_OpStgInfo	*psCitGenOpStgInfo;

	uint32_t				*pDest;
	void					*pDestTemp;
	
	BRAP_CIT_P_sNodeAttr	*psNodeAttr;
	BRAP_CIT_P_FwBufInfo	*psOpNodeInfo;

	
	BDBG_ENTER(BRAP_CITGEN_P_FillScmCitOp);

	/* Stack swap address */
	BRAP_ConvertOffsetToAddress(hHeap,
								psTaskStackBuff->ui32DramBufferAddress,
								(void **)&pDestTemp
								);

	pDest	= (uint32_t*)pDestTemp ;

	psCitOp->sStackSwapBuff.ui32DramBufferAddress
						=	(uint32_t)pDest;
	psCitOp->sStackSwapBuff.ui32BufferSizeInBytes
						=	psTaskStackBuff->ui32BufferSizeInBytes;


	psCitOp->ui32NumBranches = psAlgoNwInfo->ui32NumBranches;

	for( ui32BranchNum=0; ui32BranchNum<psAlgoNwInfo->ui32NumBranches; ui32BranchNum++ )
	{
		ui32NumAlgoStgInBranch = 
			psAlgoNwInfo->psBranchInfo[ui32BranchNum]->ui32NumStages;
		
		psCitOp->sCitBranchInfo[ui32BranchNum].ui32NumStages =  
														ui32NumAlgoStgInBranch;

		for( ui32AlgoStg=0; ui32AlgoStg<ui32NumAlgoStgInBranch; ui32AlgoStg++ )
		{
			psStgInfo			= &psAlgoNwInfo->psBranchInfo[ui32BranchNum]->sStgInfo[ui32AlgoStg];
			psCitGenOpStgInfo	= &psCitOp->sCitBranchInfo[ui32BranchNum].sCitStgInfo[ui32AlgoStg];

			psCitGenOpStgInfo->ui32NumNodes		= psStgInfo->ui32NumNodes;
			psCitGenOpStgInfo->uAlgorithm		= psStgInfo->uAlgorithm;
			psCitGenOpStgInfo->eAudioAlgoType	= psStgInfo->eAudioAlgoType;		
			psCitGenOpStgInfo->uAudioMode		= psStgInfo->uAudioMode;
			
			if( psStgInfo->bIpsFromFwStage == eFALSE )
			{
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}
			else
			{
				/*
				psCitGenOpStgInfo->ui32StartNodeIndex	= BRAP_CIT_P_INVALID_NODE_IDX;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
				*/

				/* This is added to get the status information from the last stage of the network*/
				psCitGenOpStgInfo->ui32StartNodeIndex	= 0;
				psCitGenOpStgInfo->ui32TsmNodeIndex		= BRAP_CIT_P_INVALID_NODE_IDX;
			}

			for ( ui32Node=0; ui32Node<psStgInfo->ui32NumNodes; ui32Node++ )
			{
				psNodeAttr = &psStgInfo->sNodeAttr[ui32Node];
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->eFwExecId = psNodeAttr->eAlgoId;

				/* Interframe address */
				if(psNodeAttr->ui32InterFrmBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32InterFrmBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				
				psOpNodeInfo->ui32InterframeBufAdr	= (uint32_t)pDest;
				psOpNodeInfo->ui32InterframeBufSize = psNodeAttr->ui32InterFrmBuffSize;
				
				/* User param configuration */
				if(psNodeAttr->ui32UsrCfgBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
				/*	Need to convert the physical address to virtual address for the CIT
					o/p structure */
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32UsrCfgBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}

				psOpNodeInfo->ui32UserParamBufAdr	 = (uint32_t)pDest;
				psOpNodeInfo->ui32UserParamBufSize	 = psNodeAttr->ui32UsrCfgBuffSize;

				/* Status buffer configuration */
				if(psNodeAttr->ui32FwStatusBuffDramAddress == BRAP_AF_P_DRAM_ADDR_INVALID)
				{
					pDest = (uint32_t*)BRAP_AF_P_DRAM_ADDR_INVALID;
				}
				else
				{
					BRAP_ConvertOffsetToAddress(	hHeap,
													psNodeAttr->ui32FwStatusBuffDramAddress,
													(void **)&pDestTemp
												);
					pDest	= (uint32_t*)pDestTemp ;
				}
		
				psOpNodeInfo->ui32StatusBufAdr	=  (uint32_t)pDest;
				psOpNodeInfo->ui32StatusBufSize	=  psNodeAttr->ui32FwStatusBuffSize;

			}
			for(ui32Node=psStgInfo->ui32NumNodes;ui32Node<BRAP_AF_P_MAX_NUM_NODES_IN_ALGO;ui32Node++)
															
			{
				psOpNodeInfo = &psCitGenOpStgInfo->sFwOpNodeInfo[ui32Node];

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;

				psOpNodeInfo->ui32InterframeBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32InterframeBufSize	=  0;

				psOpNodeInfo->ui32UserParamBufAdr	=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32UserParamBufSize	=  0;

				psOpNodeInfo->ui32StatusBufAdr		=  BRAP_AF_P_DRAM_ADDR_INVALID;
				psOpNodeInfo->ui32StatusBufSize		=  0;


			}
		}
	}

	BDBG_LEAVE(BRAP_CITGEN_P_FillScmCitOp);
}





/*	This function analysis the CIT. The intention of this function is to
	analyse and print out all the parameters within the CIT structure 
	Going forward it is expected to verify all the addresses and sizes also
	generated in the CIT strucuture*/
void BRAP_P_AnalyseScmCit(	BMEM_Handle				hHeap,
						    BRAP_SCM_P_sTASK_CONFIG	*psCit
					  ) 
{
	uint32_t	ui32NumNodes;
	uint32_t	ui32Node;
	uint32_t	ui32NumSrc, ui32NumDest;

	uint32_t	*pDest;
	void		*pDestTemp;

	BRAP_SCM_P_sGLOBAL_TASK_CONFIG	*psGblTaskCfg;
	BRAP_AF_P_sNODE_CONFIG			*psNodeCfg;

#ifdef ANALYZE_IO_CFG
	BRAP_AF_P_sIO_BUFFER			sIoBuffer;
	BRAP_AF_P_sIO_GENERIC_BUFFER	sIoGenericBuffer;
#endif

	/*	First Step: Print all the informations within the CIT structure */
	BDBG_MSG(("===================================="));
	BDBG_MSG(("=========== SCM CIT LOG============="));

	BDBG_MSG(("Global Task Configuration parameters"));
	BDBG_MSG(("===================================="));

	psGblTaskCfg = &psCit->sGlobalTaskConfig;
	ui32NumNodes = psGblTaskCfg->ui32NumberOfNodesInTask;
	

	BDBG_MSG(("Number of Nodes in Task: %d ",ui32NumNodes));

	BDBG_MSG(("Start Node index of Core Algo: %d ",
		psGblTaskCfg->ui32StartNodeIndexOfCoreScmAlgo));

	/*
		Removing:
	ui32NumPorts = psGblTaskCfg->ui32NumOpPorts;
	BDBG_MSG(("Number of Output Ports in Task: %d ",
		psGblTaskCfg->ui32NumOpPorts));

	for(ui32Port=0; ui32Port<ui32NumPorts; ui32Port++)
	{
		BDBG_MSG(("Output Port Configuration for Port: %d ",ui32Port));
		BDBG_MSG(("TBD "));
	}
	*/

	/*	Dram Scratch buffer Address and Size */
	BDBG_MSG((""));
	BDBG_MSG(("DRAM Scratch Buffer Address: 0x%x ",
		psGblTaskCfg->sDramScratchBuffer.ui32DramBufferAddress));
	BDBG_MSG(("DRAM Scratch Buffer Size Allocated: %d Bytes ",
		psGblTaskCfg->sDramScratchBuffer.ui32BufferSizeInBytes));
	BDBG_MSG((""));

	


	BDBG_MSG(("Node Configuration parameters "));
	BDBG_MSG(("============================= "));
	for(ui32Node=0; ui32Node<ui32NumNodes; ui32Node++)
	{
		psNodeCfg = &psCit->sNodeConfig[ui32Node];
		BDBG_MSG(("Node index: %d ",psNodeCfg->uiNodeId));
		BDBG_MSG(("Algo Id: %s ",AlgoIdEnum2Char[psNodeCfg->eAlgoId]));		

		BDBG_MSG(("\neCollectResidual : %s ",DisableEnable[psNodeCfg->eCollectResidual]));
		/*BDBG_ERR(("Algo Id: %x ",psNodeCfg->eAlgoId));*/

		/*	Code Address and Size */
		BDBG_MSG(("DRAM Code Buffer Address: 0x%x ",
			psNodeCfg->sDramAlgoCodeBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Code Buffer Size: %d Bytes",
			psNodeCfg->sDramAlgoCodeBuffer.ui32BufferSizeInBytes));

		/*	Lookup Table Address and Size */
		BDBG_MSG(("DRAM Lookup Table Buffer Address: 0x%x ",
			psNodeCfg->sDramLookupTablesBuffer.ui32DramBufferAddress));
		BDBG_MSG(("DRAM Lookup Table Buffer Size: %d Bytes",
			psNodeCfg->sDramLookupTablesBuffer.ui32BufferSizeInBytes));

		/*	Inter-frame buffer Address check */
		if( psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Not present for this Node"));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Inter-Frame Buffer Address: 0x%x ",
				psNodeCfg->sDramInterFrameBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Inter-Frame Buffer Size: %d Bytes",
				psNodeCfg->sDramInterFrameBuffer.ui32BufferSizeInBytes));
		}

		/*	Node Status buffer Address check */
		if( psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM Node Status buffer Not present for this Node"));
			BDBG_MSG(("DRAM Node Status buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM Node Status buffer Address: 0x%x ",
				psNodeCfg->sDramStatusBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM Node Status buffer Size: %d Bytes",
				psNodeCfg->sDramStatusBuffer.ui32BufferSizeInBytes));
		}

		/*	User config buffer Address check */
		if( psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress == 
				BRAP_AF_P_DRAM_ADDR_INVALID )
		{
			BDBG_MSG(("DRAM User Config Buffer not present for this Node"));
			BDBG_MSG(("DRAM User Config Buffer Size: 0 Bytes"));
		}
		else
		{
			BDBG_MSG(("DRAM User Config Buffer Address: 0x%x ",
				psNodeCfg->sDramUserConfigBuffer.ui32DramBufferAddress));
			BDBG_MSG(("DRAM User Config Buffer Size: %d Bytes",
				psNodeCfg->sDramUserConfigBuffer.ui32BufferSizeInBytes));
		}

		/*	Input buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Input Configuration Details:", ui32Node));
		BDBG_MSG(("Num Source feeding data to this node: %d", psNodeCfg->ui32NumSrc));
		for( ui32NumSrc=0; ui32NumSrc<psNodeCfg->ui32NumSrc; ui32NumSrc++)
		{
			BDBG_MSG(("Source %d Input Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc]));			

#ifdef ANALYZE_IO_CFG
			/*BDBG_MSG(("Input buffer Configuration:"));*/

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Source %d Input Buffer Type: %s",
				ui32NumSrc, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			/*	Analyze Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif
			BDBG_MSG(("Source %d Input Generic Buffer Cfg Structure Address: 0x%x", 
				ui32NumSrc, psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc]));

#ifdef ANALYZE_IO_CFG
			
			/* BDBG_MSG(("Input Generic buffer Configuration:")); */

			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeIpGenericDataBuffCfgAddr[ui32NumSrc],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*	Getting contents of the IO Generic buffer */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));

			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Source %d Input Generic Buffer Type: %s",
				ui32NumSrc, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			 /* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

			BDBG_MSG((""));
		}

		/*	Output buffer configuration details */
		BDBG_MSG((""));
		BDBG_MSG(("Node %d Output Configuration Details:", ui32Node));
		BDBG_MSG(("Num Destination getting data from this node: %d", 
			psNodeCfg->ui32NumDst));
		for( ui32NumDest=0; ui32NumDest<psNodeCfg->ui32NumDst; ui32NumDest++)
		{

			/* IO BUFFER CONFIGURATION */
			/*-------------------------*/

			/*Printing Output Buffer Cfg Structure Address */
			BDBG_MSG(("Destination %d Output Buffer Cfg Structure Address: 0x%x", 
				ui32NumDest, psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest]));			
			
#ifdef ANALYZE_IO_CFG
			/* BDBG_MSG(("Output buffer Configuration:"));*/


			/* Getting contents of the Destination IO buffer */
			/* Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;					
			
			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_BUFFER));

			/*Printing Buffer Type*/
			if(sIoBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoBuffer.eBufferType]));	
			}

			
			/*	Print Io Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoBuffCfgStruct(&sIoBuffer); */			
#endif


			/* IOGENERIC BUFFER CONFIGURATION */
			/*--------------------------------*/

			BDBG_MSG(("Destination %d Output Generic Buffer Cfg Structure Address: 0x%x",
				ui32NumDest, psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest]));


#ifdef ANALYZE_IO_CFG

			/*	Getting contents of the IO Generic buffer */

			/*Getting the Virtual Address */
			BRAP_ConvertOffsetToAddress(hHeap,
										psNodeCfg->ui32NodeOpGenericDataBuffCfgAddr[ui32NumDest],
										(void **)&pDestTemp
									);
			pDest	= (uint32_t*)pDestTemp ;

			/*Read the DRAM to local structure */
			BRAP_CITGEN_P_ReadFromDram(	(uint32_t) pDest,
							(uint32_t) &sIoGenericBuffer,
							(uint32_t) SIZEOF(BRAP_AF_P_sIO_GENERIC_BUFFER));


			/*Printing Buffer Type*/
			if(sIoGenericBuffer.ui32NumBuffers >0)
			{
				BDBG_MSG(("Destination %d Output Generic Buffer Type: %s",
				ui32NumDest, BuffTypeEnum2Char[sIoGenericBuffer.eBufferType]));	
			}
			
			/*	Analyze Io Genric Buff Struct */
			/* BRAP_CITGEN_P_AnalyzeIoGenericBuffCfgStruct(&sIoGenericBuffer); */			
#endif

#ifdef BRAP_CIT_P_ENABLE_FORK_MATRIXING						

			BDBG_MSG(("Destination %d Datatype : %s",
				ui32NumDest, PortDatatType[psNodeCfg->eNodeOpBuffDataType[ui32NumDest]]));	

			BDBG_MSG((""));
#endif
		}		
		BDBG_MSG(("============================= "));
	}
}

