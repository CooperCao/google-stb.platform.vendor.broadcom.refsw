/***************************************************************************
*     Copyright (c) 2003-2011, Broadcom Corporation
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
*
* Module Description: 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef __BRAP_FW_ALGO_H
#define __BRAP_FW_ALGO_H

/* Enable the Capture Port cfg to FW */
#define CAP_PORT_SUPPORT  


/*	Based on the current DTS algorithm which has 5 decode stages
	Two nodes are intially reserved for Frame sync and TSM nodes */
#define BRAP_AF_P_MAX_NUM_NODES_IN_ALGO			( 5 + 2 )

/*	Max number of nodes for the current worst case scenario */
#define BRAP_AF_P_MAX_NODES					( (uint32_t) 12 ) 

/*	Max channel for 7.1 + concurrent stereo o/p */
#define BRAP_AF_P_MAX_CHANNELS				( (uint32_t) 8 )

#define BRAP_AF_P_MAX_CHANNEL_PAIR			(uint32_t)((BRAP_AF_P_MAX_CHANNELS+1)>>1)

/*Maximum frame size for an algorithm. PI uses this for allocating FMM ring buffer */
#define BRAP_AF_P_MAX_SAMPLING_RATE     ((uint32_t)48)  /* 48Khz (Max Sampling Frequency /1000) (to make in line of ms) */
#define BRAP_AF_P_MAX_BLOCKING_TIME     ((uint32_t)128) /* Blocking time * 2 (Initial zerofill size in NEWFWARCH) (in ms) */
#define BRAP_AF_P_MAX_INDEPENDENT_DELAY	((uint32_t)500) /* Max independent delay in ms */
#define	BRAP_AF_P_MAX_THRESHOLD         ((uint32_t)6144) /* Threshold based on Max 
                decoder frame size (for WMA) In Samples & DSOLA Considerations */
#define BRAP_AF_P_SAMPLE_PADDING        ((uint32_t)0) /* Padding */

#define BRAP_AF_P_NON_DELAY_RBUF_SIZE   \
    (BRAP_AF_P_MAX_BLOCKING_TIME * BRAP_AF_P_MAX_SAMPLING_RATE \
    + BRAP_AF_P_MAX_THRESHOLD + BRAP_AF_P_SAMPLE_PADDING) \
    * 4                     /* 4 to make in bytes */

#define BRAP_AF_P_DELAY_RBUF_SIZE   \
    (BRAP_AF_P_MAX_BLOCKING_TIME * BRAP_AF_P_MAX_SAMPLING_RATE \
    + BRAP_AF_P_MAX_THRESHOLD + BRAP_AF_P_SAMPLE_PADDING  \
    + BRAP_AF_P_MAX_INDEPENDENT_DELAY * BRAP_AF_P_MAX_SAMPLING_RATE) \
    * 4                     /* 4 to make in bytes */

/*	Max number of output ports. Currently we have accomadated 
	for three branches, each branch	having two o/p ports*/
#define BRAP_AF_P_MAX_BRANCHES				( (uint32_t) 3 )
#define BRAP_AF_P_MAX_OP_PORTS_PER_BRANCH	( (uint32_t) 2 )
#define BRAP_AF_P_MAX_OP_PORTS				( BRAP_AF_P_MAX_BRANCHES * \
											BRAP_AF_P_MAX_OP_PORTS_PER_BRANCH )

/*Maximum number of Adaptive blocks 
	PPM logic is using BRAP_AF_P_MAX_CHANNEL_PAIR for its operations as the CIT-PI interface is based on BRAP_AF_P_MAX_CHANNEL_PAIR. 
	And BRAP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS is used for better readability in CIT-FW interface.
	Actual HW block number is 4
*/
#define BRAP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS	BRAP_AF_P_MAX_CHANNEL_PAIR

/* Maximum number of FMM Io buffer ports in a task*/
#define  BRAP_AF_P_MAX_FMM_OP_PORTS_IN_TASK	(6)


/*	This defines the maximum number of different sources the node 
	can take input from */
#define BRAP_AF_P_MAX_IP_FORKS				3	/* We need to increase BRAP_P_MAX_FW_STG_INPUTS as well to get the change complete */

/*	This defines the maximum number of different destinations the node 
	can output */
#define BRAP_AF_P_MAX_OP_FORKS				( BRAP_AF_P_MAX_IP_FORKS * 2 )


/*	Defines for the FMM o/p buff configuration */

#define	BRAP_AF_P_MAX_NUM_DAC_PORTS			3	/* There are only 2 HIFIDACS for 7405/7325 family */
												/* There are only 3 HIFIDACS for 3548 family */
#define	BRAP_AF_P_MAX_NUM_SPDIF_PORTS		2
#define	BRAP_AF_P_MAX_NUM_PLLS				4
#define	BRAP_AF_P_MAX_NUM_I2S_PORTS			2
#define	BRAP_AF_P_MAX_NUM_CAP_PORTS			4


/**************************************************************************
		Inter Task Communication Buffer allocation
***************************************************************************/
#define	BRAP_AF_P_INTERTASK_IOBUFFER_SIZE		(uint32_t)(11200*4)
#define	BRAP_AF_P_INTERTASK_IOGENBUFFER_SIZE	(uint32_t)(4098*2)

/**************************************************************************
		Inter Task Feed back buffer path
***************************************************************************/
#define	BRAP_AF_P_INTERTASK_FEEDBACK_BUFFER_SIZE	(uint32_t)(128*4)
/***************************************************************************/

/***************************************************************************
Summary: 
	Enum data type describing MS decode mode
	
Description:
		Single Decode
		MS10/Ms11/MSSoundEffect 
See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_DolbyMsUsageMode
{ 
    BRAP_AF_P_DolbyMsUsageMode_eSingleDecodeMode=0,
    BRAP_AF_P_DolbyMsUsageMode_eMS10DecodeMode, 
    BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode,
    BRAP_AF_P_DolbyMsUsageMode_eMpegConformanceMode,
    BRAP_AF_P_DolbyMsUsageMode_eMS11SoundEffectMixing,
    BRAP_AF_P_DolbyMsUsageMode_eLAST,
    BRAP_AF_P_DolbyMsUsageMode_eINVALID=0x7fffffff

} BRAP_AF_P_DolbyMsUsageMode;

/***************************************************************************
Summary: 
	Enum data type describing MS decoder type
	
Description:
		Decoder type as Priamry, Secondary, Sound Effects
		MS10/Ms11
See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_DolbyMsDecoderType
{
 BRAP_AF_P_DolbyMsDecoderType_ePrimary = 0,
 BRAP_AF_P_DolbyMsDecoderType_eSecondary,
 
 BRAP_AF_P_DolbyMsDecoderType_eLAST,
 BRAP_AF_P_DolbyMsDecoderType_eINVALID = 0x7fffffff

}BRAP_AF_P_DolbyMsDecoderType ;

/***************************************************************************
Summary: 
	Enum data type describing PLL or NCO Select
	
Description:
	
		PLL Select
		NCO Select

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_PllNcoSelect
{
	BRAP_AF_P_NoneSelect = 0x0,	/*This is for default case */
	BRAP_AF_P_PllSelect  = 0x1,
	BRAP_AF_P_NcoSelect  = 0x2,

	BRAP_AF_P_PllNcoSelect_eMax,
	BRAP_AF_P_PllNcoSelect_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_PllNcoSelect;


/***************************************************************************
Summary: 
	Enum data type describing the types of Port Enable Fields
	
Description:
	Enum data type describing the types of Port Enable Fields

  Enable =0;
  Disable=1;

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_PortEnableDisable
{
	BRAP_AF_P_PortEnable  = 0x0,
	BRAP_AF_P_PortDisable = 0x1,
	BRAP_AF_P_PortEnableDisable_eMax,
	BRAP_AF_P_PortEnableDisable_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_PortEnableDisable;


/***************************************************************************
Summary: 
	Enum data type describing Enable and Disable
	
Description:	

  Valid  =1;
  Invalid=0;

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_EnableDisable
{
	BRAP_AF_P_eDisable = 0x0,
	BRAP_AF_P_eEnable  = 0x1,	
	BRAP_AF_P_EnableDisable_eMax,
	BRAP_AF_P_EnableDisable_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_EnableDisable;


/***************************************************************************
Summary: 
	Enum data type describing the content type of SPDIF
	
Description:

	  PCM =0;
	  Compressed=1;

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_SpdifContent
{
	BRAP_AF_P_PcmOnSpdif		= 0x0,
	BRAP_AF_P_CompressedOnSpdif = 0x1,
	BRAP_AF_P_SpdifContent_eMax,
	BRAP_AF_P_SpdifContent_eInvalid			= 0x7FFFFFFF

}BRAP_AF_P_SpdifContent;

/***************************************************************************
Summary: 
	Enum data type describing the FMM Destination's associated sinking rate
	
Description:

	 FMM Destination can be. FMM port or Capture port.
	 There are a few rates classified for FMM Destination.
	 
					1)BaseRate			
					2)StreamSamplingRate	
					3)2xBaseRate			
					4)4xBaseRate			
					5)HBRRate	

See Also:
	None.
****************************************************************************/

typedef enum BRAP_AF_P_FmmDstFsRate
{
	BRAP_AF_P_FmmDstFsRate_eBaseRate			= 0x0,
	BRAP_AF_P_FmmDstFsRate_eStreamSamplingRate	= 0x1,
	BRAP_AF_P_FmmDstFsRate_e2xBaseRate			= 0x2,
	BRAP_AF_P_FmmDstFsRate_e4xBaseRate			= 0x3,
	BRAP_AF_P_FmmDstFsRate_eHBRRate				= BRAP_AF_P_FmmDstFsRate_e4xBaseRate,	
	BRAP_AF_P_FmmDstFsRate_eInvalid,

	BRAP_AF_P_FmmDstFsRate_eMax					= 0x7FFFFFFF

}BRAP_AF_P_FmmDstFsRate;


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
	BRAP_DSPCHN_AudioType
****************************************************************************/

typedef enum BRAP_AF_P_AlgoId
{	
	/******************* Audio Algorithm Start ****************************/
	BRAP_AF_P_AlgoId_eAudioAlgoStartIdx = 0x0,			/*Audio Algorithm Start Index */
	BRAP_AF_P_AlgoId_eMpegDecode = BRAP_AF_P_AlgoId_eAudioAlgoStartIdx,

	BRAP_AF_P_AlgoId_eAc3DecodeStage1,
	BRAP_AF_P_AlgoId_eAc3DecodeStage2,

	BRAP_AF_P_AlgoId_eAacDecodeStage1,
	BRAP_AF_P_AlgoId_eAacDecodeStage2,

	BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage0,
	BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage1,
	BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage2,
	BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage3,
	BRAP_AF_P_AlgoId_eAacHeLpSbrDecodeStage4,
	
	BRAP_AF_P_AlgoId_eDdpDecodeStage1,
	BRAP_AF_P_AlgoId_eDdpDecodeStage2,

	BRAP_AF_P_AlgoId_eDdLosslessDecodeStage1,
	BRAP_AF_P_AlgoId_eDdLosslessDecodeStage2,

	BRAP_AF_P_AlgoId_eLpcmCustomDecode,
	BRAP_AF_P_AlgoId_eBdLpcmDecode,
	BRAP_AF_P_AlgoId_eDvdLpcmDecode,
	BRAP_AF_P_AlgoId_eHdDvdLpcmDecode,
	
	BRAP_AF_P_AlgoId_eMpegMcDecode,
	
	BRAP_AF_P_AlgoId_eWmaStdDecode,
	BRAP_AF_P_AlgoId_eWmaProStdDecodeStage1,
	BRAP_AF_P_AlgoId_eWmaProStdDecodeStage2,

	BRAP_AF_P_AlgoId_eMlpDecode,

	BRAP_AF_P_AlgoId_eDdp71DecodeStage1,
	BRAP_AF_P_AlgoId_eDdp71DecodeStage2,

	BRAP_AF_P_AlgoId_eDtsDecodeStage1,
	BRAP_AF_P_AlgoId_eDtsDecodeStage2,
	BRAP_AF_P_AlgoId_eDtsDecodeStage3,

	BRAP_AF_P_AlgoId_eDtsLbrDecodeStage1,
	BRAP_AF_P_AlgoId_eDtsLbrDecodeStage2,
	BRAP_AF_P_AlgoId_eDtsLbrDecodeStage3,

	BRAP_AF_P_AlgoId_eDtsHdDecodeStage0,
	BRAP_AF_P_AlgoId_eDtsHdDecodeStage1,
	BRAP_AF_P_AlgoId_eDtsHdDecodeStage2,
	BRAP_AF_P_AlgoId_eDtsHdDecodeStage3,
	BRAP_AF_P_AlgoId_eDtsHdDecodeStage4,

	BRAP_AF_P_AlgoId_ePcmWavDecode,
	BRAP_AF_P_AlgoId_eAmrDecode,
	BRAP_AF_P_AlgoId_eDraDecode,
	BRAP_AF_P_AlgoId_eRealAudioLbrDecode,
	BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage0,
	BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage1,
	BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage2,
	BRAP_AF_P_AlgoId_eDolbyPulseDecodeStage3,

	BRAP_AF_P_AlgoId_eMs10DdpDecodeStage1,
	BRAP_AF_P_AlgoId_eMs10DdpDecodeStage2,

	/*	All the Algo Ids for Decoder Frame Sync */
	BRAP_AF_P_AlgoId_eMpegFrameSync,
	BRAP_AF_P_AlgoId_eMpegMcFrameSync,
	
	BRAP_AF_P_AlgoId_eAdtsFrameSync,
	BRAP_AF_P_AlgoId_eLoasFrameSync,
	
	BRAP_AF_P_AlgoId_eWmaStdFrameSync,
	BRAP_AF_P_AlgoId_eWmaProFrameSync,
	
	BRAP_AF_P_AlgoId_eAc3FrameSync,
	BRAP_AF_P_AlgoId_eDdpFrameSync,
	BRAP_AF_P_AlgoId_eDdp71FrameSync,
	
	BRAP_AF_P_AlgoId_eDtsFrameSync,
	BRAP_AF_P_AlgoId_eDtsLbrFrameSync,
	BRAP_AF_P_AlgoId_eDtsHdFrameSync,
	BRAP_AF_P_AlgoId_eDtsHdFrameSync_1,			/*	What are continuation Frame Syncs */
	BRAP_AF_P_AlgoId_eDtsHdHdDvdFrameSync,
	
	BRAP_AF_P_AlgoId_eDdLosslessFrameSync,
	BRAP_AF_P_AlgoId_eMlpFrameSync,
	BRAP_AF_P_AlgoId_eMlpHdDvdFrameSync,
	
	BRAP_AF_P_AlgoId_ePesFrameSync,
	
	BRAP_AF_P_AlgoId_eBdLpcmFrameSync,
	BRAP_AF_P_AlgoId_eHdDvdLpcmFrameSync,
	BRAP_AF_P_AlgoId_eDvdLpcmFrameSync,
	BRAP_AF_P_AlgoId_eDvdLpcmFrameSync_1,

	BRAP_AF_P_AlgoId_ePcmWavFrameSync,
	BRAP_AF_P_AlgoId_eAmrFrameSync,
	BRAP_AF_P_AlgoId_eDraFrameSync,
	BRAP_AF_P_AlgoId_eRealAudioLbrFrameSync,
	BRAP_AF_P_AlgoId_eMs10DdpFrameSync,
	BRAP_AF_P_AlgoId_eMixerTaskFrameSync,


	/*	All the Algo Ids for TSM */
	BRAP_AF_P_AlgoId_eDecodeTsm,
	BRAP_AF_P_AlgoId_eEncodeTsm,
	BRAP_AF_P_AlgoId_ePassthroughTsm,

	/* Audio/Video/Graphic Task Ids */
	BRAP_AF_P_AlgoId_eSystemDecodeTask,
	BRAP_AF_P_AlgoId_eSystemGraphicTask,
	BRAP_AF_P_AlgoId_eSystemVideoTask,

	/* SCM Task Id */
	BRAP_AF_P_AlgoId_eSCMTask,


	/*	All the Algo Ids for the stages of encode algorithms */
	BRAP_AF_P_AlgoId_eAc3Encode,

	BRAP_AF_P_AlgoId_eMpegL2EncodeStage1,
	BRAP_AF_P_AlgoId_eMpegL2EncodeStage2,

	BRAP_AF_P_AlgoId_eMpegL3EncodeStage1,
	BRAP_AF_P_AlgoId_eMpegL3EncodeStage2,

	BRAP_AF_P_AlgoId_eAacLcEncodeStage1,
	BRAP_AF_P_AlgoId_eAacLcEncodeStage2,

	BRAP_AF_P_AlgoId_eAacHeEncodeStage1,
	BRAP_AF_P_AlgoId_eAacHeEncodeStage2,
	BRAP_AF_P_AlgoId_eAacHeEncodeStage3,

	BRAP_AF_P_AlgoId_eDtsEncode,
	BRAP_AF_P_AlgoId_eDtsBroadcastEncode,
	BRAP_AF_P_AlgoId_eSbcEncode,
	BRAP_AF_P_AlgoId_eMs10DDTranscode,

	/*	All the Algo Ids for the stages of encode Algo Frame Syncs */
	BRAP_AF_P_AlgoId_eAc3EncFrameSync,
	BRAP_AF_P_AlgoId_eMpegL3EncFrameSync,
	BRAP_AF_P_AlgoId_eMpegL2EncFrameSync,
	BRAP_AF_P_AlgoId_eAacLcEncFrameSync,
	BRAP_AF_P_AlgoId_eAacHeEncFrameSync,
	BRAP_AF_P_AlgoId_eDtsEncFrameSync,	

	/*	All the algo ids for the stages of passthrough */
	BRAP_AF_P_AlgoId_ePassThru,
	BRAP_AF_P_AlgoId_ePtAuxDataGenStg,

	/*	All the Algo Ids for the stages of Post Proc algorithms */
	BRAP_AF_P_AlgoId_eSrsTruSurroundPostProc,
	BRAP_AF_P_AlgoId_eSrcPostProc,
	BRAP_AF_P_AlgoId_eDdbmPostProc,
	BRAP_AF_P_AlgoId_eDownmixPostProc,
	BRAP_AF_P_AlgoId_eCustomSurroundPostProc,
	BRAP_AF_P_AlgoId_eCustomBassPostProc,
	BRAP_AF_P_AlgoId_eKaraokeCapablePostProc,
	BRAP_AF_P_AlgoId_eCustomVoicePostProc,
	BRAP_AF_P_AlgoId_ePeqPostProc,
	BRAP_AF_P_AlgoId_eAvlPostProc,
	BRAP_AF_P_AlgoId_ePl2PostProc,
	BRAP_AF_P_AlgoId_eXenPostProc,
	BRAP_AF_P_AlgoId_eBbePostProc,
	BRAP_AF_P_AlgoId_eDsolaPostProc,
	BRAP_AF_P_AlgoId_eDtsNeoPostProc,
	BRAP_AF_P_AlgoId_eDDConvert,
	BRAP_AF_P_AlgoId_eAudioDescriptorFadePostProc,
	BRAP_AF_P_AlgoId_eAudioDescriptorPanPostProc,
	BRAP_AF_P_AlgoId_ePCMRouterPostProc,
	BRAP_AF_P_AlgoId_eWMAPassThrough,
	BRAP_AF_P_AlgoId_eSrsTruSurroundHDPostProc,
	BRAP_AF_P_AlgoId_eSrsTruVolumePostProc,
	BRAP_AF_P_AlgoId_eDolbyVolumePostProc,
	BRAP_AF_P_AlgoId_eAudysseyVolumePostProc,
	BRAP_AF_P_AlgoId_eBrcm3DSurroundPostProc,
	BRAP_AF_P_AlgoId_eFWMixerPostProc,
	BRAP_AF_P_AlgoId_eMonoDownMixPostProc,
	BRAP_AF_P_AlgoId_eMs10DDConvert,
	BRAP_AF_P_AlgoId_eAudysseyABXPostProc,
	BRAP_AF_P_AlgoId_eSrsCircleSurrPostProc,
	BRAP_AF_P_AlgoId_eSrsEqualizerPostProc,
	BRAP_AF_P_AlgoId_eDdrePostProc,
	BRAP_AF_P_AlgoId_eDv258PostProc,
	BRAP_AF_P_AlgoId_eBtscEncPostProc,
	
	/*	All algo Ids for post proc frame sync */
	BRAP_AF_P_AlgoId_ePostProcFrameSync,	

	/*Audio Algo End Index*/
	BRAP_AF_P_AlgoId_eAudioAlgoEndIdx = BRAP_AF_P_AlgoId_ePostProcFrameSync,
	/********************* Audio Algorithm End ***************************/

	/********************* Gfx Decode Start ******************************/		
	BRAP_AF_P_AlgoId_eGfxStartIdx,
	
	BRAP_AF_P_AlgoId_eGfxDecode = BRAP_AF_P_AlgoId_eGfxStartIdx,		
	
	/* Gfx Decode End Index*/
	BRAP_AF_P_AlgoId_eGfxEndIdx = BRAP_AF_P_AlgoId_eGfxDecode,
	/********************* Gfx Decode End ********************************/

	/********************* Video Algorithm Start *************************/
		
	BRAP_AF_P_AlgoId_eVideoAlgoStartIdx,

	/* Video Codecs */		
	BRAP_AF_P_AlgoId_eRealVideo9PLF = BRAP_AF_P_AlgoId_eVideoAlgoStartIdx,		
	BRAP_AF_P_AlgoId_eRealVideo9Stage1,
	BRAP_AF_P_AlgoId_eRealVideo9Stage2,
	BRAP_AF_P_AlgoId_eRealVideo9Stage3,
	BRAP_AF_P_AlgoId_eRealVideo9PPD,
		
	/*Video Algo End Index*/
	BRAP_AF_P_AlgoId_eVideoAlgoEndIdx = BRAP_AF_P_AlgoId_eRealVideo9PPD,
	/********************* Video Algorithm End ***************************/

	/********************* SCM Start ******************************/		
	BRAP_AF_P_AlgoId_eScmStartIdx,
	
	BRAP_AF_P_AlgoId_eScm_1 = BRAP_AF_P_AlgoId_eScmStartIdx,		
	BRAP_AF_P_AlgoId_eScm_2,			
	
	BRAP_AF_P_AlgoId_eScmEndIdx = BRAP_AF_P_AlgoId_eScm_2,
	/********************* SCM End ********************************/


	BRAP_AF_P_AlgoId_eMax,
	BRAP_AF_P_AlgoId_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_AlgoId;

/***************************************************************************
Summary:
	This enumeration defines the Decode/PP/Enc audio datatype that is used 
	by BSP's conditional Execution..

***************************************************************************/
typedef enum BRAP_AF_P_DecodeEncPPAlgoType
{
   	BRAP_AF_P_DecodeAlgoType_eMpeg,				/* MPEG */   	
   	BRAP_AF_P_DecodeAlgoType_eAac,				/* AAC */
	BRAP_AF_P_DecodeAlgoType_eAacAdts = BRAP_AF_P_DecodeAlgoType_eAac, /* AAC */ 
	BRAP_AF_P_DecodeAlgoType_eAacLoas,			/* AAC */ 
   	BRAP_AF_P_DecodeAlgoType_eAacSbr,			/* AAC_SBR */
	BRAP_AF_P_DecodeAlgoType_eAacSbrAdts = BRAP_AF_P_DecodeAlgoType_eAacSbr, /* AAC_SBR */
	BRAP_AF_P_DecodeAlgoType_eAacSbrLoas,		/* AAC_SBR */ 
   	BRAP_AF_P_DecodeAlgoType_eAc3,				/* AC3 */   	
   	BRAP_AF_P_DecodeAlgoType_eAc3Plus,			/* AC3_PLUS */   	
   	BRAP_AF_P_DecodeAlgoType_eDts,				/* DTS */   	
   	BRAP_AF_P_DecodeAlgoType_eLpcmBd,			/* LPCM Blue Ray Disc */   	
   	BRAP_AF_P_DecodeAlgoType_eLpcmHdDvd,		/* LPCM HD-DVD */   	
   	BRAP_AF_P_DecodeAlgoType_eDtshd,			/* DTSHD */   	
   	BRAP_AF_P_DecodeAlgoType_eLpcmDvd,			/* LPCM DVD */   	
   	BRAP_AF_P_DecodeAlgoType_eWmaStd,			/* WMA Standard */   	
   	BRAP_AF_P_DecodeAlgoType_eAc3Lossless,		/* AC3 in LOSSLESS*/   	
   	BRAP_AF_P_DecodeAlgoType_eMlp,				/* MLP */   	
   	BRAP_AF_P_DecodeAlgoType_ePcm,				/* PCM Data */   	
   	BRAP_AF_P_DecodeAlgoType_eDtsLbr,			/* DTS-LBR */   	
   	BRAP_AF_P_DecodeAlgoType_eDdp7_1,			/* DDP 7.1 */     	
   	BRAP_AF_P_DecodeAlgoType_eMpegMc,			/* MPEG MC*/   	
   	BRAP_AF_P_DecodeAlgoType_eWmaPro,			/* WMA Pro */   	
   	BRAP_AF_P_DecodeAlgoType_eDtshdSub,			/* DTSHD SUB*/   	
   	BRAP_AF_P_DecodeAlgoType_eLpcmDvdA,			/* LPCM A DVD*/
	BRAP_AF_P_DecodeAlgoType_eDtsBroadcast,		/* DTS Broadcast*/ 
	BRAP_AF_P_DecodeAlgoType_ePcmWav,			/* PCM WAV decoder*/
	BRAP_AF_P_DecodeAlgoType_eAmr,				/* AMR decoder */
	BRAP_AF_P_DecodeAlgoType_eDra,				/* DRA Decoder */
	/* Extra Gap fillers*/
	BRAP_AF_P_DecodeAlgoType_eExtraDecodersStart,
	BRAP_AF_P_DecodeAlgoType_eExtraDecodersEnd = 45,
	/*MPEG Layers*/
	BRAP_AF_P_DecodeAlgoType_eMpegL2 = 46,
	BRAP_AF_P_DecodeAlgoType_eMpegL3 = 47,
	BRAP_AF_P_DecodeAlgoType_eEnd = BRAP_AF_P_DecodeAlgoType_eMpegL3,
	/*End of Decoders */

	/* Post processing */
	BRAP_AF_P_PostProcessingTypeStart,
	BRAP_AF_P_PostProcessingType_eDdbm = BRAP_AF_P_PostProcessingTypeStart,
	BRAP_AF_P_PostProcessingType_eDtsNeo,
	BRAP_AF_P_PostProcessingType_eAVL,
	BRAP_AF_P_PostProcessingType_eDDConvert,
	BRAP_AF_P_PostProcessingType_ePLll,
	BRAP_AF_P_PostProcessingType_eSrsXt,
	BRAP_AF_P_PostProcessingType_eXen,
	BRAP_AF_P_PostProcessingType_eBbe,
	BRAP_AF_P_PostProcessingType_eSrc,
	BRAP_AF_P_PostProcessingType_eCustomSurround,
	BRAP_AF_P_PostProcessingType_eCustomBass,
	BRAP_AF_P_PostProcessingType_eCustomVoice,
	BRAP_AF_P_PostProcessingType_ePeq,
	BRAP_AF_P_PostProcessingType_eDownmix0,
	BRAP_AF_P_PostProcessingType_eAudioDescriptorFade,
	BRAP_AF_P_PostProcessingType_eAudioDescriptorPan,
	BRAP_AF_P_PostProcessingType_ePCMRouter,
	BRAP_AF_P_PostProcessingType_eWMAPassThrough,
	BRAP_AF_P_PostProcessingType_eDsola,
	BRAP_AF_P_PostProcessingType_eSrsHd,
	BRAP_AF_P_PostProcessingType_eGenericPassThru,
	BRAP_AF_P_PostProcessingType_eSrsTruVolume,
	BRAP_AF_P_PostProcessingType_eDolbyVolume,
	/* Extra Gap fillers*/
	BRAP_AF_P_PostProcessingType_eExtraPPsStart,
	BRAP_AF_P_PostProcessingType_eExtraPPsEnd =79,
	BRAP_AF_P_PostProcessingType_eEnd = BRAP_AF_P_PostProcessingType_eExtraPPsEnd,
	/*End Post processing */

	/* Encoders processing */
	BRAP_AF_P_EncAudioTypeStart,
	BRAP_AF_P_EncAudioType_eMpeg1Layer3 = BRAP_AF_P_EncAudioTypeStart,	
	BRAP_AF_P_EncAudioType_eMpeg1Layer2,	
	BRAP_AF_P_EncAudioType_eDTS,			
	BRAP_AF_P_EncAudioType_eAacLc,			
	BRAP_AF_P_EncAudioType_eAacHe,			
	BRAP_AF_P_EncAudioType_eAc3,			
	BRAP_AF_P_EncAudioType_eDTSBroadcast,	
	BRAP_AF_P_EncAudioType_eSbc,	
	/* Extra Gap fillers*/
	BRAP_AF_P_EncAudioType_eExtraEncsStart,
	BRAP_AF_P_EncAudioType_eExtraEncsEnd =95,
	BRAP_AF_P_EncAudioType_eEnd = BRAP_AF_P_EncAudioType_eExtraEncsEnd,
   	
    BRAP_AF_P_DecodeEncPPAlgoType_eMax,			    /* Max value */    
   	BRAP_AF_P_DecodeEncPPAlgoType_eInvalid	= 0xFF
   	
} BRAP_AF_P_DecodeEncPPAlgoType;

/* Time base of the System */
typedef enum BRAP_AF_P_TimeBaseType
{
    BRAP_AF_P_TimeBaseType_e45Khz = 0,
    BRAP_AF_P_TimeBaseType_e27Mhz,
    BRAP_AF_P_TimeBaseType_eLast,
    BRAP_AF_P_TimeBaseType_eInvalid = 0x7FFFFFFF
}BRAP_AF_P_TimeBaseType;


/***************************************************************************
Summary: 
	Enum data type describing the types of Sampling Frequency in the audio
	firmware.

	Structure definition to map the input sampling frequency to output 
	sampling frequency.

See Also:
	None.
****************************************************************************/

typedef enum BRAP_AF_P_SampFreq
{
	BRAP_AF_P_SampFreq_e8Khz = 0,	
	BRAP_AF_P_SampFreq_e11_025Khz, 
	BRAP_AF_P_SampFreq_e12Khz,
	BRAP_AF_P_SampFreq_e16Khz, 
	BRAP_AF_P_SampFreq_e22_05Khz, 
	BRAP_AF_P_SampFreq_e24Khz, 
	BRAP_AF_P_SampFreq_e32Khz, 
	BRAP_AF_P_SampFreq_e44_1Khz, 
	BRAP_AF_P_SampFreq_e48Khz,
	BRAP_AF_P_SampFreq_e64Khz,
	BRAP_AF_P_SampFreq_e88_2Khz,
	BRAP_AF_P_SampFreq_e96Khz,	
	BRAP_AF_P_SampFreq_e128Khz,
	BRAP_AF_P_SampFreq_e176_4Khz,
	BRAP_AF_P_SampFreq_e192Khz,	
	BRAP_AF_P_SampFreq_eMax, 
	BRAP_AF_P_SampFreq_eInvalid = 0x7FFFFFFF 

}BRAP_AF_P_SampFreq;

typedef struct BRAP_AF_P_sOpSamplingFreq
{
	uint32_t ui32OpSamplingFrequency[BRAP_AF_P_SampFreq_eMax];

}BRAP_AF_P_sOpSamplingFreq;


/***************************************************************************
Summary: 
	Enum data type describing the types of inter-frame buffers in the audio
	firmware.
	
Description:
	This enum describes the type of the inter-frame buffer being used by the
	node. The inter-frame buffer can be present, absent or shared. when shared,
	it means that the current node of the algo shares the inter-frame buffer
	with the previous node in the same algo.

See Also:
	None.
****************************************************************************/

typedef enum BRAP_AF_P_InterFrameBuffType
{
	BRAP_AF_P_InterFrameBuffType_eAbsent = 0x0,
	BRAP_AF_P_InterFrameBuffType_ePresent,
	BRAP_AF_P_InterFrameBuffType_eShared,

	BRAP_AF_P_InterFrameBuffType_eMax,
	BRAP_AF_P_InterFrameBuffType_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_InterFrameBuffType;


/*********************************************************************
Summary:
    This enum defines the DSP mode output type

Description:
    
See Also:
**********************************************************************/
typedef enum BRAP_AF_P_DistinctOpType
{  
    BRAP_AF_P_DistinctOpType_e7_1_MixPcm,			/* Mixed 7.1 PCM */
	BRAP_AF_P_DistinctOpType_e5_1_MixPcm,			/* Mixed 5.1 PCM */
	BRAP_AF_P_DistinctOpType_eStereo_MixPcm,		/* Mixed Stereo*/

    BRAP_AF_P_DistinctOpType_e7_1_PCM,				/* 7.1 PCM */
	BRAP_AF_P_DistinctOpType_e5_1_PCM,				/* 5.1 PCM */
	BRAP_AF_P_DistinctOpType_eStereo_PCM,			/* Downmixed to stereo PCM */
    BRAP_AF_P_DistinctOpType_eCompressed,			/* Compressed Data */
    BRAP_AF_P_DistinctOpType_eAuxilaryDataOut,		/* Auxillary Data like 1) Converter input data 2) ITB generation data etc*/
	BRAP_AF_P_DistinctOpType_eGenericIsData,		/* Any kind of data on Interstage buffer*/

	BRAP_AF_P_DistinctOpType_eDolbyReEncodeAuxDataOut,	/* The is the Non DRC applied 5.1 PCM data used for DD reencode */

    BRAP_AF_P_DistinctOpType_eMax, 
	BRAP_AF_P_DistinctOpType_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_DistinctOpType;


/***************************************************************************
Summary: 
	The structure provides the outmode for various PP algos

See Also:
	None.
****************************************************************************/

typedef enum  BRAP_AF_P_OutModeType
{
    BRAP_AF_P_OutModeType_eBypass,		 /*If Output Mode is same as input Mode */
    BRAP_AF_P_OutModeType_eFixed,        /*If Output type is always fixed, irrespective of input mode */
    BRAP_AF_P_OutModeType_eConfigurable  /*If Output mode can be configured by App */

}BRAP_AF_P_OutModeType;

typedef struct BRAP_AF_P_PpOutMode
{
    BRAP_AF_P_OutModeType		eOutModeType;
    BRAP_AF_P_DistinctOpType    eOpType; /*Valid only when eOutModeType is Fixed, Otherwise it will be invalid*/
}BRAP_AF_P_PpOutMode;

/*********************************************************************
Summary:
    This enum defines the FMM buffer content:
	1) PCM content 
	2) Compressed content
Description:
	This enum is inially defined to inform FW to do Zero / Pause bursts
	during ZERO- Filling in Decode task
    
See Also:
**********************************************************************/

typedef enum BRAP_AF_P_FmmContentType
{  
    
    BRAP_AF_P_FmmContentType_ePcm,				/* PCM */
	BRAP_AF_P_FmmContentType_eCompressed,		/* Compressed Data */
	BRAP_AF_P_FmmContentType_eMax, 
	BRAP_AF_P_FmmContentType_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_FmmContentType;

/***************************************************************************
Summary: 
	Enum data type describing the types of Firmware-Status buffers in the audio
	firmware.
	
Description:
	This enum describes the type of the Firmware-Status buffer being used by the
	node. The Firmware-Status buffer can be present, absent or shared. when shared,
	it means that the current node of the algo shares the Firmware-Status buffer
	with the previous node in the same algo.

See Also:
	None.
****************************************************************************/

typedef enum
{
	BRAP_AF_P_FwStatus_eAbsent = 0x0,
	BRAP_AF_P_FwStatus_ePresent,
	BRAP_AF_P_FwStatus_eShared,

	BRAP_AF_P_FwStatus_eMax,
	BRAP_AF_P_FwStatus_eInvalid = 0x7FFFFFFF
}BRAP_AF_P_FwStatus;

/***************************************************************************
Summary: 
	Enum data types used to check the bit position status of a register
Description:

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_SetReset
{
	BRAP_AF_P_eReset	= 0x0,
	BRAP_AF_P_eSet		= 0x1,	
	BRAP_AF_P_SetReset_eMax,
	BRAP_AF_P_SetReset_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_SetReset;


/***************************************************************************
Summary: 
	Enum data types used to check a field is valid/invalid
Description:

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_ValidInvalid
{
	BRAP_AF_P_eInvalid	= 0x0,
	BRAP_AF_P_eValid	= 0x1,	
	BRAP_AF_P_ValidInvalid_eMax,
	BRAP_AF_P_ValidInvalid_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_ValidInvalid;
/***************************************************************************
Summary: 
	Enum data types used to check the Licenseing permission for a Codec
Description:
			BRAP_AF_P_eExecuteAlgo		= Licence Present
			BRAP_AF_P_eDonotExecuteAlgo = No Licence Present
			BRAP_AF_P_eExecuteOnlyAACLC = Specific for AAC codec.

See Also:
	None.
****************************************************************************/
typedef enum BRAP_AF_P_AlgoExecLicenseStatus
{
	BRAP_AF_P_eExecuteAlgo			= 0x0,
	BRAP_AF_P_eDonotExecuteAlgo		= 0x1,	
	BRAP_AF_P_eExecuteOnlyAACLC		= 0x2,	

	BRAP_AF_P_AlgoExecLicenseStatus_eMax,
	BRAP_AF_P_AlgoExecLicenseStatus_eInvalid = 0x7FFFFFFF

}BRAP_AF_P_AlgoExecLicenseStatus;

/***************************************************************************
Summary: 
	Algo info structure between the firmware and the PI.
	
Description:
	The Algo execution info structure contains information regarding the
	algorithm. It contains information on how many nodes are present for the
	algorithm and the Algo Id of each node of the algorithm.

See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sALGO_EXEC_INFO
{
	uint32_t					NumNodes;
	BRAP_AF_P_AlgoId			eAlgoIds[BRAP_AF_P_MAX_NUM_NODES_IN_ALGO];
} BRAP_AF_P_sALGO_EXEC_INFO;

/***************************************************************************
Summary: 
	Node info structure between the firmware and the PI in the audio
	firmware.
	
Description:
	Each executable to provide info to the CIT generation module on the 
	sizes of individual buffers. This is to be done through this structure.
	The max frame size and the max num channels supported will be used to
	internally partition the inter-stage I/O buffer into different channels
	
	How these two fields should be set for a post processing algorithm: The
	frame size must be set to the minimum chunk the post proc algo can
	work on. The number of channels	supported must be the max it can support.
	Ex: DDBM works on 512 samples and 7.1 channels on 7440.
	
	To save DRAM buffer, the file where this structure sits for an algorithm
	can be made chip specific. Some chips requiring to support 7.1 will need 
	additional memory

	To save some more DRAM buffer, while filling the data for post proc, we
	can ensure that the number of channels is set to a minimum (say stereo)
	this will ensure that the acmod of the decoder/encoder dictate the number
	of channel buffers required.
	The above is a drawback in a pure post process task where we may be
	working on 5.1 channels.

See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sNODE_INFO
{
	uint32_t						ui32CodeSize;
	uint32_t						ui32RomTableSize;
	uint32_t						ui32InterFrmBuffSize;
	uint32_t						ui32InterStgIoBuffSize;
	uint32_t						ui32InterStgGenericBuffSize;
	uint32_t						ui32ScratchBuffSize;
	uint32_t						ui32UserCfgBuffSize;
	uint32_t						ui32MaxSizePerChan;
	uint32_t						ui32MaxNumChansSupported;
	BRAP_AF_P_InterFrameBuffType	eInterFrameBuffType;

	BRAP_AF_P_FwStatus				eStatusBuffType;
	uint32_t						ui32StatusBuffSize;
	

} BRAP_AF_P_sNODE_INFO;


/***************************************************************************
Summary: 
	Enum to hold the type of the buffer.
	
Description:
	The buffer type indicates the type of i/o buffer being configured as
	input or output for the current node.

See Also:
	None.
****************************************************************************/

typedef enum BRAP_AF_P_BufferType
{
	BRAP_AF_P_BufferType_eDRAM		= 0,		/*	This is used in case of a DRAM ring buffer, 
												where in the read, write pointers are also in DRAM. */
	BRAP_AF_P_BufferType_eRDB,					/*	This is used in case of a ring buffer, 
												where in the read, write pointers are in RDB registers. */
	BRAP_AF_P_BufferType_eFMM,					/*	Used in case of FMM ring buffers. The address of the
												registers holding the read,write pointers etc will be 
												programmed by the host.In this case, a GISB access in done
												by the firmware to get the actual read,write etc. pointers */
	BRAP_AF_P_BufferType_eRAVE,					/*	Used in case of RAVE ring buffers. Need to differentiate
												between FMM and RAVE registers as RAVE registers use the
												wrap bit for buffer wrap around indication */
	BRAP_AF_P_BufferType_eDRAM_IS,				/*	DRAM inter stage buffer. Only the start address of the
												buffer has to be programmed by the host. */

	BRAP_AF_P_BufferType_eFMMSlave,				/* This type shows that the Buffer is FMM and its is configured 
												as Slave */		

	BRAP_AF_P_BufferType_eLAST,
	BRAP_AF_P_BufferType_eInvalid = 0x7FFFFFFF
}BRAP_AF_P_BufferType;

/***************************************************************************
Summary: 
	Structure that holds all the genereic circular buffer parameters.
	
Description:
	This structure contains the unified circular buffer structure. All the 
	circular buffer parameters like read/write base and end parameters are
	present in this structure. The circular buffer structure has been unified
	taking into account both the FMM and RAVE type of ciruclar buffer.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sDRAM_CIRCULAR_BUFFER
{	
	uint32_t	ui32BaseAddr;		/*	Circular buffer's base address */
	uint32_t	ui32EndAddr;		/*	Circular buffer's End address */
	uint32_t	ui32ReadAddr;		/*	Circular buffer's read address */
	uint32_t	ui32WriteAddr;		/*	Circular buffer's write address */
	uint32_t	ui32WrapAddr;		/*	Circular buffer's wrap address */
}BRAP_AF_P_sDRAM_CIRCULAR_BUFFER;


/***************************************************************************
Summary: 
	The I/O buffer structure used for as configuration for each node.
	
Description:
	This structure contains the I/O buffer configuration of a node. 

See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sIO_BUFFER
{
	uint32_t						ui32NumBuffers;	/*	Defines the number of 
														channels in the input 
														or output */
	BRAP_AF_P_BufferType			eBufferType;	/*	Defines the the location 
														of the input or output buffer. 
														This can take values defined 
														by eBUFFER_TYPE */
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		sCircBuffer[BRAP_AF_P_MAX_CHANNELS];	/*	All circular buffer 
																				parameters wrt to each 
																				input buffer comes */

}BRAP_AF_P_sIO_BUFFER;

/***************************************************************************
Summary: 
	The Generic buffer structure used as configuration for each node.
	
Description:
	This structure contains the configuration for the genric buffer of a node. 

See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sIO_GENERIC_BUFFER
{
	uint32_t							ui32NumBuffers;	/*	Defines the number of 
														channels in the input 
														or output */
	BRAP_AF_P_BufferType				eBufferType;	/*	Defines the the location
														of the input or output buffer.
														This can take values defined 
														by eBUFFER_TYPE */
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		sCircBuffer;	/*	All circular buffer parameters 
														wrt to each input buffer comes */

}BRAP_AF_P_sIO_GENERIC_BUFFER;


/***************************************************************************
Summary: 
	The single circ buffer structure used to provide detaios of one circular
	buffer of any buffer type
	
Description:	

See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sSINGLE_CIRC_BUFFER
{
	
	BRAP_AF_P_BufferType				eBufferType;	/*	Defines the the location
														of the input or output buffer.
														This can take values defined 
														by eBUFFER_TYPE */
	BRAP_AF_P_sDRAM_CIRCULAR_BUFFER		sCircBuffer;	/*	All circular buffer parameters 
														wrt to each input buffer comes */

}BRAP_AF_P_sSINGLE_CIRC_BUFFER;


/***************************************************************************
Summary: 
	The simple DRAM buffer structure which contains a DRAM address and Size.
	
See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sDRAM_BUFFER
{
	uint32_t					ui32DramBufferAddress;
	uint32_t					ui32BufferSizeInBytes;
}BRAP_AF_P_sDRAM_BUFFER;



/* GATE OPEN CONFIGURATION */

/***************************************************************************
Summary: 
	The gate open Configuration of a port
	
Description:

	This contains the Start write pointers of all port channels and the 
	Dram address of IO Buffer configuration


See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sFMM_GATE_OPEN_CONFIG
{
	uint32_t					ui32IndepDelay;

	uint32_t					uin32RingBufStartWrPointAddr[BRAP_AF_P_MAX_CHANNELS]; 

	uint32_t					uin32DramIoConfigAddr;  

	/*FMM buffer content : Compressed / PCM */
	BRAP_AF_P_FmmContentType	eFMMContentType;

	/*FMM buffer Sinking rate */
	BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;

} BRAP_AF_P_sFMM_GATE_OPEN_CONFIG;


/***************************************************************************
Summary: 
	The gate open Configuration of a task
	
Description:

	This contains the Start write pointers of all port channels and the 
	Dram address of IO Buffer configuration of a task.


See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG
{
	uint32_t	ui32NumPorts;

	uint32_t	ui32MaxIndepDelay ;

	BRAP_AF_P_sFMM_GATE_OPEN_CONFIG  sFmmGateOpenConfig[BRAP_AF_P_MAX_FMM_OP_PORTS_IN_TASK];

}BRAP_AF_P_TASK_sFMM_GATE_OPEN_CONFIG;


/***************************************************************************
Summary: 
	The structure that contains the PPM valid flag and the PPM cfg address.
	
See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sPPM_CFG
{
	BRAP_AF_P_EnableDisable	ePPMChannel;
	uint32_t				ui32PPMCfgAddr;	

}BRAP_AF_P_sPPM_CFG;

/***************************************************************************
Summary: 

	The structure that contains all hardware cfg RDBs used by FW .
	
See Also:
	None.
****************************************************************************/

typedef struct BRAP_AF_P_sFW_HW_CFG
{
	/*PPM Configuration */
	BRAP_AF_P_sPPM_CFG		sPpmCfg[BRAP_AF_P_MAX_ADAPTIVE_RATE_BLOCKS];

}BRAP_AF_P_sFW_HW_CFG;




/* PORT CONFIGURATION */

/***************************************************************************
Summary: 
	The structure that contains the destination configuration for a NCO
	port.
	
See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sFMM_DEST_NCO_CFG
{	
	uint32_t					ui32NcoSampleIncRegAddr;	
	uint32_t					ui32NcoPhaseIncRegAddr;	
	uint32_t					ui32NcoRateRatioRegAddr;	

}BRAP_AF_P_sFMM_DEST_NCO_CFG;

/***************************************************************************
Summary: 
	The structure that contains the destination configuration for a DAC o/p
	port.
	
See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sFMM_DEST_DAC_CFG
{
	uint32_t					ui32DacEnabled;	
	
	BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;	/* FMM Destination's associated
												   sinking rate 
												*/

	uint32_t					ui32HifidacRmSampleIncRegAddr;	
	uint32_t					ui32HifidacRmPhaseIncRegAddr;	
	uint32_t					ui32HifidacRmRateRatioRegAddr;	
	uint32_t					ui32HifidacRateManagerCfgAddr;

	

}BRAP_AF_P_sFMM_DEST_DAC_CFG;


/***************************************************************************
Summary: 
	The structure that contains the destination configuration for a SPDIF o/p
	port.
	
See Also:
	None.
****************************************************************************/

/* Host info for SPDIF parameters from Scheduler */
typedef struct BRAP_AF_P_sSPDIF_USER_CFG
{
	int32_t	i32ProfessionalModeFlag;
	int32_t	i32SoftwareCopyrightAsserted;
	int32_t	i32CategoryCode;
	int32_t	i32ClockAccuracy;
	int32_t	i32bSeparateLRChanNum;
	int32_t i32CgmsA;	
	/* Range values for this field.
		
		0   	CopyFreely  	Unlimited copies may be made of the content.
		1 		CopyNoMore	 	One generation of copies has already been made; no further copying is allowed.
		2 		CopyOnce 		One generation of copies may be made
		3	 	CopyNever 		No copies may be made of the content.

		default =0	
	*/

	int32_t i32SpdifPcmWordWidth;  /* PCM width : Possible values : 16,24 : Default value is 16 (16bit)*/

} BRAP_AF_P_sSPDIF_USER_CFG;


typedef struct
{	
	uint32_t					ui32SpdifEnabled;	
	
	BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;	

	uint32_t					ui32AudioSpdifClkMacroRegAddr;	
	
	uint32_t					ui32SpdifCbitCtrlReg;			
	
	uint32_t					ui32CbitBasePingAddr;			
													
	uint32_t					ui32CbitBasePongAddr;			
	
	uint32_t					ui32PingPongMask;		

	uint32_t					ui32AudFmmMsCtrlHwSpdifCfg;
													
	uint32_t					ui32SpdifDramConfigPtr;	

	uint32_t					ui32PcmOnSpdif; /* 0=  PCM  and 1 = Compressed*/
	
}BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG;

typedef struct
{
	uint32_t					ui32AudFmmMsEsrStatusRegAddr;	
	uint32_t					ui32AudFmmMsEsrStatusClearRegAddr;
}BRAP_AF_P_sFMM_ESR_REGISTER;


typedef struct
{
	BRAP_AF_P_sFMM_DEST_SPDIF_CLK_CBIT_CFG			sFmmDestSpdifClkCbitCfg[BRAP_AF_P_MAX_NUM_SPDIF_PORTS];
	BRAP_AF_P_sFMM_ESR_REGISTER						sFmmEsrRegister;	 

}BRAP_AF_P_sFMM_DEST_SPDIF_CFG;

/***************************************************************************
Summary: 
	The structure that contains the destination configuration for a I2S o/p
	port.
	
See Also:
	None.
****************************************************************************/
typedef struct
{
	uint32_t					ui32I2SEnabled;

	BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;	/* FMM Destination's associated
												   sinking rate 
												*/
	uint32_t					ui32AudioI2SClkMacroRegAddr;
															
}BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG;


/***************************************************************************
Summary: 
	The structure that contains the destination configuration for a Cap port
	port.
	
See Also:
	None.
****************************************************************************/
typedef struct
{
	uint32_t					ui32CapPortEnabled;

	BRAP_AF_P_FmmDstFsRate		eFmmDstFsRate;	/* FMM Destination's associated
												   sinking rate 
												*/

	uint32_t					ui32AudioCapPortClkMacroRegAddr;
															
}BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG;


/***************************************************************************
Summary: 
	The structure that contains the destination configuration for o/p PLL.
	
See Also:
	None.
****************************************************************************/
/* 

There are 2 plls in system . But can be 4... 
In a task only one PLL 
*/

typedef struct{

															
	uint32_t						ui32PllEnabled;		
	
	uint32_t						ui32PllIndex;																	

	uint32_t						ui32AudioPllMacroRegAddr;
															 

}BRAP_AF_P_sFMM_DEST_PLL_CFG;


/***************************************************************************
Summary: 
	The structure that contains the FMM destination configuration registers.

See Also:
	None.
****************************************************************************/
typedef struct
{
	/* Clock Select for The Destination Ports*/
	BRAP_AF_P_PllNcoSelect					eFmmPllNcoSelect;
	BRAP_AF_P_sFMM_DEST_NCO_CFG				sFmmDestNcoCfg;
	BRAP_AF_P_sFMM_DEST_PLL_CFG				sFmmDestPllCfg; 	

	/* Output port Cfg */
	BRAP_AF_P_sFMM_DEST_I2S_CLK_CFG			sFmmDestI2SClkCfg[BRAP_AF_P_MAX_NUM_I2S_PORTS];
	BRAP_AF_P_sFMM_DEST_DAC_CFG				sFmmDestDacCfg[BRAP_AF_P_MAX_NUM_DAC_PORTS];
	BRAP_AF_P_sFMM_DEST_SPDIF_CFG			sFmmDestSpdifCfg;
	
	/*uint32_t								ui32HbrEnable;*/
	uint32_t								ui32HwCbitsEnable;

	BRAP_AF_P_FmmDstFsRate					eHdmiFsRate; /* Sinking Rate of HDMI. 
															This information is used by Data Sync to raise Fs interrupt.
														  */

	/* Capture port Cfg */
	BRAP_AF_P_sFMM_DEST_CAP_PORT_CLK_CFG	sFmmCapPortClkCfg[BRAP_AF_P_MAX_NUM_CAP_PORTS];
	
}BRAP_AF_P_sFMM_DEST_CFG;

/***************************************************************************
Summary: 
	The structure that contains all the o/p port configurations.

Description:
	This is the address where the o/p buffer configuration for the port is
	present. 

See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sOP_PORT_CFG
{
	uint32_t					ui32OpPortCfgAddr;
	uint32_t					ui32FmmAdaptRateCntrlRegAddr;
	uint32_t					ui32IndependentDelaySign;
	uint32_t					ui32IndependentDelayValue;
}BRAP_AF_P_sOP_PORT_CFG;


/***************************************************************************
Summary: 
	The structure contains all the global configurations of a task.

Description:
	All the configuration which are common to the entire task are placed
	in the global task configuration.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_AF_P_sGLOBAL_TASK_CONFIG
{
	uint32_t					ui32NumberOfNodesInTask;
	uint32_t					ui32NumberOfZeroFillSamples;
	uint32_t					ui32StartNodeIndexOfCoreAudioAlgorithm;	/*	This node id defines 
																			the starting index of the 
																			core audio processing stages.
																			For a decoder task, which has 
																			frame sync and TSM codes in
																			separate executable, Node 0 
																			and Node 1 will be reserved
																			for frame sync and TSM. The 
																			field "StartNodeIndexOfCoreAudioAlgorithm" 
																			shall be set to 2 */



	uint32_t					ui32FmmDestCfgAddr;
																	/*	FMM destination configuration information. This structure is required
																		one per Task. How to associate an o/p port with the correct sampling 
																		frequency to be programmed in the case of SRC? */

	uint32_t                    ui32FmmGateOpenConfigAddr;

	uint32_t                    ui32FwOpSamplingFreqMapLutAddr;		/*	This is the FW Input-Output sampling frequency mapping LUT*/

	uint32_t					ui32NumOpPorts;						/*	This tells the number of output ports */

	BRAP_AF_P_sOP_PORT_CFG		sOpPortCfg[BRAP_AF_P_MAX_OP_PORTS];	/*	This will have the DRAM address of Output port's IO_buffer
																		Structure **/
	BRAP_AF_P_sDRAM_BUFFER		sDramScratchBuffer;					/*	The scratch buffer is being moved to global task config */

	uint32_t					ui32TaskFwHwCfgAddr;				/*  This address contains the structure of BRAP_AF_P_sFW_HW_CFG*/

	BRAP_AF_P_TimeBaseType		eTimeBaseType;						/*  Time base type for a task 45Khz or 27 Mhz (Direct TV) */

	/*	These fields are reserved for future usage */
	uint32_t					ui32Reserved0;
	uint32_t					ui32Reserved1;
	uint32_t					ui32Reserved2;
	uint32_t					ui32Reserved3;
}BRAP_AF_P_sGLOBAL_TASK_CONFIG;


/***************************************************************************
Summary: 
	The structure contains the configurations for an individual node.

See Also:
	None.
****************************************************************************/
typedef struct
{
	uint32_t					uiNodeId;
	BRAP_AF_P_EnableDisable		eCollectResidual;
	uint32_t					ui32AudioAlgorithmType;
	BRAP_AF_P_AlgoId			eAlgoId;
	BRAP_AF_P_sDRAM_BUFFER		sDramUserConfigBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramInterFrameBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramAlgoCodeBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramLookupTablesBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramStatusBuffer;
	
	uint32_t					ui32NumSrc;
	uint32_t					ui32NumDst;

	/*The filed that tells whether the Node input is Valid/ Invalid : Valid =1 Invalid =0
	  This field is required for Dynamic input port switching. All the input ports which
	  are interstage buffers will be set to valid
	*/

	BRAP_AF_P_ValidInvalid		eNodeIpValidFlag[BRAP_AF_P_MAX_IP_FORKS];                        

	uint32_t					ui32NodeIpBuffCfgAddr[BRAP_AF_P_MAX_IP_FORKS];
	uint32_t					ui32NodeIpGenericDataBuffCfgAddr[BRAP_AF_P_MAX_IP_FORKS];
									
	uint32_t					ui32NodeOpBuffCfgAddr[BRAP_AF_P_MAX_OP_FORKS];
	uint32_t					ui32NodeOpGenericDataBuffCfgAddr[BRAP_AF_P_MAX_OP_FORKS];

	BRAP_AF_P_DistinctOpType	eNodeOpBuffDataType[BRAP_AF_P_MAX_OP_FORKS];		
	

}BRAP_AF_P_sNODE_CONFIG;

/***************************************************************************
Summary: 
	The structure is complete task configuration structure. This contains 
	the global task configuration and an array of node configuration
	structures.

See Also:
	None.
****************************************************************************/
typedef struct
{
	BRAP_AF_P_sGLOBAL_TASK_CONFIG	sGlobalTaskConfig;
	BRAP_AF_P_sNODE_CONFIG          sNodeConfig[BRAP_AF_P_MAX_NODES];

}BRAP_AF_P_sTASK_CONFIG;

/****************************************************************************/
/****************************************************************************/
/************************* GRAPHICS TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

#define BRAP_GFX_P_MAX_NODES    (uint32_t)1
/***************************************************************************
Summary: 
	The structure contains all the global configurations of a task that comes
	as input from PI to CIT.

Description:
	All the input global configuration parameters that comes directly from PI
	to CIT are included in this structure. This structure will be passed as 
	input argument to CIT generation function for GFX.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_GFX_P_sGfxDecodeBuffCfg
{
    uint32_t		ui32Dummy; 
}BRAP_GFX_P_sGfxDecodeBuffCfg;

typedef struct BRAP_GFX_P_sGLOBAL_TASK_CONFIG
{
    uint32_t							ui32NumberOfNodesInTask;
	uint32_t							ui32StartNodeIndexOfCoreGfxAlgo;
    BRAP_AF_P_sDRAM_BUFFER				sDramScratchBuffer;
	BRAP_GFX_P_sGfxDecodeBuffCfg		sGlobalTaskConfigFromPI;
}BRAP_GFX_P_sGLOBAL_TASK_CONFIG;


typedef struct  BRAP_GFX_P_sNODE_CONFIG
{
	uint32_t					uiNodeId;
	BRAP_AF_P_AlgoId			eAlgoId;
	BRAP_AF_P_sDRAM_BUFFER		sDramUserConfigBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramInterFrameBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramAlgoCodeBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramLookupTablesBuffer;
	BRAP_AF_P_sDRAM_BUFFER		sDramStatusBuffer;

}BRAP_GFX_P_sNODE_CONFIG;


typedef struct  BRAP_GFX_P_sTASK_CONFIG
{
    BRAP_GFX_P_sGLOBAL_TASK_CONFIG  sGlobalTaskConfig;
    BRAP_AF_P_sNODE_CONFIG         sNodeConfig[BRAP_GFX_P_MAX_NODES];
}BRAP_GFX_P_sTASK_CONFIG;



/****************************************************************************/
/****************************************************************************/
/**************************  SCM TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

#define BRAP_SCM_P_MAX_NODES    (uint32_t)1

/***************************************************************************
Summary: 
	The structure contains all the global configurations of SCM task.

Description:
	All the configuration which are common to the entire task are placed
	in the global task configuration.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_SCM_P_sGLOBAL_TASK_CONFIG
{
    uint32_t							ui32NumberOfNodesInTask;

	uint32_t							ui32StartNodeIndexOfCoreScmAlgo;

    BRAP_AF_P_sDRAM_BUFFER				sDramScratchBuffer;

}BRAP_SCM_P_sGLOBAL_TASK_CONFIG;

/***************************************************************************
Summary: 
	The structure is the Top level CIT datastructure for  SCM task.

Description:
		
	 Contains 
			1) Global Configuration
			2) Node Configuration

See Also:

	None.
****************************************************************************/
typedef struct  BRAP_SCM_P_sTASK_CONFIG
{
    BRAP_SCM_P_sGLOBAL_TASK_CONFIG  sGlobalTaskConfig;
    BRAP_AF_P_sNODE_CONFIG			sNodeConfig[BRAP_SCM_P_MAX_NODES];

}BRAP_SCM_P_sTASK_CONFIG;


/****************************************************************************/
/****************************************************************************/
/************************* VIDEO TASK  **************************************/
/****************************************************************************/
/****************************************************************************/

#define BRAP_FWMAX_VIDEO_BUFF_AVAIL			(uint32_t)(16)
#define BRAP_FWMAX_MB_INFO_AVAIL			(uint32_t)(2)
#define BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL		(uint32_t)(3)


/***************************************************************************
Summary: 
	The structure contains the buffer parameters

Description:

See Also:
	None.
****************************************************************************/
typedef struct BRAP_VF_P_sFrameBuffParams
{
    BRAP_AF_P_sDRAM_BUFFER        sFrameBuffLuma;
	BRAP_AF_P_sDRAM_BUFFER        sFrameBuffChroma;

}BRAP_VF_P_sFrameBuffParams;

/***************************************************************************
Summary: 
	The structure contains all the global configurations of a task that comes
	as input from PI to CIT.

Description:
	All the input global configuration parameters that comes directly from PI
	to CIT are included in this structure. This structure will be passed as 
	input argument to CIT generation function for Video.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_VF_P_sVDecodeBuffCfg
{
    /* This will have DRAM address associated with with PDQ */
	/*    BRAP_AF_P_sDRAM_CIRCULAR_BUFFER *psPDQ; */
	uint32_t					ui32PDQCircBuffAddr;

	/* This will have DRAM address associated with with PRQ */
	/* BRAP_AF_P_sDRAM_CIRCULAR_BUFFER *psPRQ;	*/
	uint32_t					ui32PRQCircBuffAddr;
	
	/* These structure will have DRAM start addresses of the different frame buffers */
	BRAP_VF_P_sFrameBuffParams  sFrameBuffParams[BRAP_FWMAX_VIDEO_BUFF_AVAIL];

    /* These structure will have DRAM start addresses of the different Refernce buffers */
    BRAP_VF_P_sFrameBuffParams  sReferenceBuffParams[BRAP_FWMAX_VIDEO_REF_BUFF_AVAIL];

	/* These structures will give DRAM start addresses of the UPBs associated */
	/* with respective frame buffer above */
	BRAP_AF_P_sDRAM_BUFFER      sUPBs[BRAP_FWMAX_VIDEO_BUFF_AVAIL]; 

	/* DRAM Memory used by decoder internally */
	BRAP_AF_P_sDRAM_BUFFER      sDRAMMBInfoStartAdr[BRAP_FWMAX_MB_INFO_AVAIL];

	uint32_t                    ui32NumBuffAvl;

	/* Stripe width and height of the Display frame buffer allocated */
	uint32_t					ui32DispStripeWidth;
	uint32_t					ui32DispStripeHeight;

}BRAP_VF_P_sVDecodeBuffCfg;

/***************************************************************************
Summary: 
	The structure contains all the global configurations of a task.

Description:
	All the configuration which are common to the entire task are placed
	in the global task configuration.

See Also:
	None.
****************************************************************************/
typedef struct BRAP_VF_P_sGLOBAL_TASK_CONFIG
{
	uint32_t								ui32NumberOfNodesInTask;
	BRAP_VF_P_sVDecodeBuffCfg			   	sGlobalTaskConfigFromPI;		
	
}BRAP_VF_P_sGLOBAL_TASK_CONFIG;

/***************************************************************************
Summary: 
	The structure is complete task configuration structure. This contains 
	the global task configuration and an array of node configuration
	structures.

See Also:
	None.
****************************************************************************/
typedef struct
{
	BRAP_VF_P_sGLOBAL_TASK_CONFIG	sGlobalTaskConfig;
	BRAP_AF_P_sNODE_CONFIG          sNodeConfig[BRAP_AF_P_MAX_NODES];

}BRAP_VF_P_sTASK_CONFIG;

/*--------------------------------------------------------------------------*/




#endif
