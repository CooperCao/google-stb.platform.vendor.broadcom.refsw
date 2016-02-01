/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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

#ifndef BRAP_DSP_PRIV_H__
#define BRAP_DSP_PRIV_H__

#include "brap_types.h"
#include "brap_dsp.h"
#include "brap_dspchn.h"
#include "brap_fwdwnld_priv.h"
#include "brap_cit_priv.h"
#include "brap_fwif_priv.h"
#include "brap_fwuserconfig_priv.h"
#ifdef __cplusplus
extern "C" {
#endif

#define	BRAP_DSP_P_MAX_AUDIO_CHANNELS					3

/* Inter-channel delay user param buffer size */
#ifdef BRAP_P_FW_DBG_ENABLE
#define BRAP_DSP_P_DBG_BUF_SIZE     2*1024*1024
#else
#define BRAP_DSP_P_DBG_BUF_SIZE     4
#endif

#define BRAP_DSP_P_NUM_FIFOS            18
#define BRAP_DSP_P_NUM_PTR_PER_FIFO     5

/* Hardcode cmd queue as FIFO 0 */
#define BRAP_DSP_P_FIFO_CMD     0
/* Hardcode response queue which doesn't have any task id associated */
#define BRAP_DSP_P_FIFO_GENERIC_RSP     1
#define BRAP_DSP_P_FIFO_DEBUG           17
#define BRAP_REALVIDEO_DSP_P_FIFO_PDQ   15
#define BRAP_REALVIDEO_DSP_P_FIFO_PRQ   16
#define BRAP_REALVIDEO_DSP_P_FIFO_DSQ   14
#define BRAP_DSP_P_FIFO_INVALID         ((unsigned int)(-1))

#define BRAP_GET_TASK_INDEX(uiTaskId) (uiTaskId -BRAP_FWIF_P_TASK_ID_START_OFFSET)

#define BRAP_DSPCHN_P_EVENT_TIMEOUT_IN_MS   400
#define BRAP_DSPCHN_P_START_STOP_EVENT_TIMEOUT_IN_MS   400


/***************************************************************************
Summary:
	This structure contains DSP settings.

***************************************************************************/
typedef struct BRAP_DSP_P_Settings
{
	BRAP_DSP_Settings sDspExtSettings;
} BRAP_DSP_P_Settings;

typedef struct BRAP_FWIF_P_UserConfigStruct
{
    BRAP_FWIF_P_MpegConfigParams	    sMpegConfigParam;	
    BRAP_FWIF_P_Ac3ConfigParams	        sAc3ConfigParam;
    BRAP_FWIF_P_Ac3ConfigParams	        sAc3PlusConfigParam;
    BRAP_FWIF_P_AacheConfigParams       sAacheConfigParam;
    BRAP_FWIF_P_WmaConfigParams         sWmaStdConfigParam;    
    BRAP_FWIF_P_WmaProConfigParams      sWmaProConfigParam;    
    BRAP_FWIF_P_LpcmUserConfig      	    sLpcmDvdConfigParam;       
    BRAP_FWIF_P_DtsHdConfigParams       sDtsBroadcastConfigParam;
    BRAP_FWIF_P_DtsHdConfigParams       sDtsHdConfigParam;   
    BRAP_FWIF_P_PcmWavConfigParams       sPcmWavConfigParam;       
    BRAP_FWIF_P_AmrConfigParams         sAmrConfigParam;    
    BRAP_FWIF_P_DraConfigParams         sDraConfigParam;    
    BRAP_FWIF_P_RalbrConfigParams       sRealAudioLbrConfigParam;
    BRAP_FWIF_P_DolbyPulseUserConfig    sDolbyPulseConfigParam;
    BRAP_FWIF_P_DDPMs10ConfigParams sMs10DDPConfigParam;    
    BRAP_FWIF_P_DDPMultiStreamConfigParams sMs11DDPConfigParam;        
    BRAP_FWIF_P_DDPMultiStreamConfigParams sMs11AC3ConfigParam;            
    
/*	BRAP_FWIF_P_AacConfigParams         sAacConfigParam;
	BRAP_FWIF_P_DtsConfigParams	        sDtsConfigParam;
  	BRAP_FWIF_P_DtshdConfigParams       sDtshdConfigParam;
	BRAP_FWIF_P_BdlpcmConfigParams	    sBdlpcmConfigParam;
	BRAP_FWIF_P_LpcmHdDvdConfigParams	sLpcmHdDvdConfigParam;    
	BRAP_FWIF_P_LpcmDvdConfigParams	    sLpcmDvdConfigParam;    
	BRAP_FWIF_P_Ac3PlusConfigParams		sAc3LosslessConfigParam;
	BRAP_FWIF_P_MlpConfigParams		    sMlpConfigParams;	
	BRAP_FWIF_P_DtslbrConfigParams      sDtslbrConfigParam;
	BRAP_FWIF_P_Ac3PlusConfigParams     sDdp7_1ConfigParam;	
	BRAP_FWIF_P_MpegMcConfigParams      sMpegMcConfigParam;*/
	BRAP_FWIF_P_TsmConfigParams         sDecodeTsmConfigParam;	
	BRAP_FWIF_P_TsmConfigParams         sEncodeTsmConfigParam;	
	BRAP_FWIF_P_TsmConfigParams         sPassthruTsmConfigParam;	
	BRAP_FWIF_P_PassthruConfigParams    sPassthruConfigParam;	  
    BRAP_FWIF_P_FrameSyncConfigParams   sFrameSyncConfigParams;
}BRAP_FWIF_P_UserConfigStruct;


/***************************************************************************
Summary:
	This structure stores user config parameter in App format who are converted to Q131 format
	while programming to FW.

***************************************************************************/
typedef struct BRAP_DEC_P_UserConfigStruct
{
/*MPEG*/
    int32_t i32MpegLeftChannelGain;
    int32_t i32MpegRightChannelGain;
/*Ac3*/    
    int32_t i32Ac3PcmScale;
    int32_t i32Ac3DynScaleHigh;
    int32_t i32Ac3DynScaleLow;

/*MS11 AC3*/    
    int32_t i32Ms11Ac3PcmScale_DownMixPath;
    int32_t i32Ms11Ac3DynScaleHigh_DownMixPath;
    int32_t i32Ms11Ac3DynScaleLow_DownMixPath;

/*MS11 DDP*/    
    int32_t i32Ms11DdpPcmScale_DownMixPath;
    int32_t i32Ms11DdpDynScaleHigh_DownMixPath;
    int32_t i32Ms11DdpDynScaleLow_DownMixPath;
    
/*Ac3Plus*/    
    int32_t i32Ac3PlusPcmScale;
    int32_t i32Ac3PlusDynScaleHigh;
    int32_t i32Ac3PlusDynScaleLow;
/* AAC/AACHE */    
    int32_t i32AacDrcGainControlCompress;
    int32_t i32AacDrcGainControlBoost;

/*DTS Broadcast*/
    int32_t i32DtsBroadcastDynScaleHigh;
    int32_t i32DtsBroadcastDynScaleLow;
   
    bool    bMsUsageMode;  
    bool    bMpegConformanceMode;
}BRAP_DEC_P_UserConfigStruct;

/***************************************************************************
Summary:
	This structure contains decoder related settings.

***************************************************************************/
typedef struct BRAP_P_DecoderSettings
{
	BRAP_FWIF_P_UserConfigStruct sUserConfigStruct; 
       BRAP_DEC_P_UserConfigStruct  sUserConfigAppFormat;
} BRAP_P_DecoderSettings;

/***************************************************************************
Summary:
	This structure contains DSPCHN channel settings.

***************************************************************************/
typedef struct BRAP_DSPCHN_P_Settings
{
    unsigned int    tbd;
}BRAP_DSPCHN_P_Settings;


/***************************************************************************
Summary:
	Handle structure for DSP device.
***************************************************************************/
typedef struct BRAP_DSP_P_Device
{
    BCHP_Handle hChip;  /* handle to chip interface*/
    BREG_Handle hRegister;  /* handle to register interface */
    BMEM_Handle hHeap;  /* handle to memory interface */
    BINT_Handle hInt;   /* handle to interrupt interface*/
    BRAP_Handle hRap;   /* handle to parent audio device*/
    unsigned int    uiDspIndex; /* DSP object index */
    uint32_t    ui32Offset; /* Offset of a register of current 
                                          DSP from the corresponding register of 
                                          the first DSP */
    uint32_t                 ui32DbgBufAddr; /* Address of 2MB Debug buffer for FW */



    /* Following field is used in interrupt routine. Need cleanup!!! */                                                                
    BRAP_DSPCHN_Handle  phAudChannelHandle[BRAP_DSP_P_MAX_AUDIO_CHANNELS];
                                                                            /* handles to DSP contexts for
                                                                                this DSP channel */
    BKNI_EventHandle hEvent[BRAP_RM_P_MAX_FW_TASK_PER_DSP]; /* Event to be used during slow handshake with 
                                                  firmware */
    BINT_CallbackHandle     hDspAckCallback;    /* This will install the Callback for
                                                  Ping the DSp*/
                                                    
    BRAP_FWIF_MsgQueueHandle      hCmdQueue;      /* Cmd queue */
    BRAP_FWIF_MsgQueueHandle      hGenRspQueue;      /* Generic Response queue */
#ifdef RAP_VIDEOONDSP_SUPPORT    
    BRAP_FWIF_MsgQueueHandle      hPDQueue;      /* Picture Delivery queue(PDQ) */
    BRAP_FWIF_MsgQueueHandle      hPRQueue;      /* Picture Release queue(PRQ) */
    BRAP_FWIF_MsgQueueHandle      hDSQueue;      /* Display queue(DSQ) */    
#endif    
    
    uint32_t        ui32DSPFifoAddrStruct; /* contains fifo address structure */
    bool            bFifoUsed[BRAP_DSP_P_NUM_FIFOS]; /* Whether this Fifo is used by any task */
    uint32_t        ui32EventIdMatrix[BRAP_RM_P_MAX_FW_TASK_PER_DSP];  /* Contains information abt. event ids already enabled */
} BRAP_DSP_P_Device;

/***************************************************************************
Summary:
***************************************************************************/
typedef struct BRAP_DSPCHN_OpPortConfig
{
	BRAP_OutputPort			eOutputPortType;
	BRAP_OP_Pll			ePll;
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    /* The following information is required for the FW to implement indep
    delay on an output port */
    int             iDelay; /* The independent delay to be applied to this 
                            output port in units of milliseconds. 
                            Note: This delay value is used only if the 
                            channel is opened with bSelectDelay as TRUE  */
    unsigned int    uiDlydRBufIndex[BRAP_RM_P_MAX_RBUFS_PER_SRCCH]; 
                            /* If this port has to be delayed at start time, 
                            list the associated RBUFs here. Else leave this
                            field as BRAP_RM_P_INVALID_INDEX */
    unsigned int    uiSrcChIndex; /* If this port has to be delayed at start 
                            time, list the associated SrcCh here. Else leave 
                            this field as BRAP_RM_P_INVALID_INDEX */    
    BRAP_OutputChannelPair eChanPair; /* This output port can be a master 
                            or cloned port. FW needs to know which channel
                            pair data it is carrying */
#endif                            
} BRAP_DSPCHN_OpPortConfig; 


/***************************************************************************
Summary:
***************************************************************************/
typedef struct BRAP_DSPCHN_OpPortParams
{
	unsigned int			uiNumOpPorts;
	BRAP_DSPCHN_OpPortConfig	sOpPortConfig[BRAP_RM_P_MAX_OUTPUTS];
} BRAP_DSPCHN_OpPortParams;

/***************************************************************************
Summary:
***************************************************************************/
typedef enum BRAP_DSPCHN_P_SourceType
{
	BRAP_DSPCHN_P_SourceType_eFMMBuf,
	BRAP_DSPCHN_P_SourceType_eDRAMBuf,     
	BRAP_DSPCHN_P_SourceType_eMax	
} BRAP_DSPCHN_P_SourceType;
/***************************************************************************
Summary: Mixer input params provided by the AM. This will be used by the DSP
    to prepare the Metadata.
    The uiScale and uiPan values can be set to the user passed / default value
    during DSPCHN start.
    TODO: Once RM is designed, this data structure can be a part of MIXER module
***************************************************************************/
typedef struct BRAP_DSPCHN_P_MixerInputParams
{
    unsigned int                uiMixerInputId; /* Mixer input index */
    BRAP_OutputChannelPair      eChanPair;      /* Channel pair fed to this 
                                                   mixer input */
    unsigned int                uiScale;        /* Host scale value for this 
                                                   channel pair */
    unsigned int                uiPan;          /* Host pan value for this 
                                                   channel pair */
}BRAP_DSPCHN_P_MixerInputParams;

/***************************************************************************
Summary: Mixer params provided by the AM. This will be used by the DSP to 
    prepare the Metadata.
    TODO: Once RM is designed, this data structure can be a part of MIXER module
***************************************************************************/
typedef struct BRAP_DSPCHN_P_MixerParams
{
    unsigned int    uiDPId;                     /* DP Id of the mixer */           
    unsigned int    uiMixerId;                  /* Mixer Id */
    BRAP_DSPCHN_P_MixerInputParams sMixerInputParams[BRAP_RM_P_MAX_MIXER_INPUTS];
                                                /* Mixer input params */
}BRAP_DSPCHN_P_MixerParams;


typedef struct BRAP_DSPCHN_P_FMMParams
{
	int8_t				        rBufIndex[BRAP_RM_P_MAX_OP_CHANNELS];
	int8_t				        inputRBufIndex[BRAP_RM_P_MAX_OP_CHANNELS]; /* Used when 
	                                            the input audio source is ring buffer*/  
    BRAP_DSPCHN_P_MixerParams   sMixerParams[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];

    int8_t                      i8HbrRbufId;
    bool                        bIsSingleI2SHbrMode;
} BRAP_DSPCHN_P_FMMParams;

typedef struct BRAP_DSPCHN_P_DRAMBufDetails
{                                                   
    BRAP_AF_P_sIO_BUFFER            sDecIoBuffer;                                                
	BRAP_AF_P_sIO_GENERIC_BUFFER	sDecIoGenericBuffer;
}BRAP_DSPCHN_P_DRAMBufDetails;

/***************************************************************************
Summary:
    Some AM API will take scale/pan coeffs from the user/host. That API will
    internally program uiScale and uiPan available in sMixerInParams in 
    sDecPathFmmParams and sEncPathFmmParams. These values will be re-informed to 
    the DSP.   
***************************************************************************/
typedef struct BRAP_DSPCHN_DecModeParams
{
    BRAP_DSPCHN_P_SourceType    eDecSourceType;     /* Source type of DSP Channel */
    union
    {
        BRAP_DSPCHN_P_FMMParams sDecPathFmmParams; /* Decode Path FMM rsrcs required 
                                                       for decode channel */ 
        BRAP_DSPCHN_P_DRAMBufDetails  sDecSourceBufDetails;                                                            
    }uDecBufFmmParams;

}BRAP_DSPCHN_DecModeParams;

typedef struct BRAP_DSPCHN_P_VideoParams
{
    BRAP_DSPCHN_VideoType	eType;          /* Video Type */
    unsigned int    ui32DispStripeWidth;                    /*What will be Default Values?? */                    
    unsigned int    ui32DispStripeHeight;                   /*What will be Default Values?? */ 
    unsigned int    ui32NumBuffAvl;                     /*What will be Default Values?? */ 
}BRAP_DSPCHN_P_VideoParams;

/***************************************************************************
Summary:
	This structure contains audio parameters required to decode
	input compressed audio data.

***************************************************************************/
typedef struct BRAP_DSPCHN_P_AudioParams
{
	BRAP_DSPCHN_AudioParams		sExtAudioParams;
	BRAP_DSPCHN_P_VideoParams	sExtVideoParams;    
	BAVC_Timebase				eTimebase;
	unsigned int				uiTransChId;	
	BAVC_XptContextMap	        sXptContextMap;	 
                                        /* This structure holds the CDB and 
										   ITB register addresses for the 
										   corresponding Rave Context 
										   allocated by the Xpt PI.*/	
	bool						bPlayback;
	bool                       	bMultiChanOnI2S;
	BRAP_DSPCHN_DecModeParams	sDecOrPtParams;
	BRAP_DSPCHN_DecModeParams	sSimulPtParams;
	bool						bUpdateChStatusParams;
	BRAP_OP_SpdifChanStatusParams sSpdifChStatusParams[BRAP_AF_P_MAX_NUM_SPDIF_PORTS]; /* This field is valid only
										when bUpdateChStatusParams = TRUE */
    bool                        bFwToUpdateRdPtr;
                                        /* Flag to indicate if FW needs to 
                                           update ring buffer Read Pointers */
} BRAP_DSPCHN_P_AudioParams;


/***************************************************************************
Summary:
	This structure returns information of sample rate change in stream
***************************************************************************/
typedef struct BRAP_DSPCHN_P_SampleRateChangeInfo
{
	bool 					bFsChanged; /* true = Change in sample rate,
											false = Sample rate not changed*/
	BRAP_DSPCHN_SampleRateChangeInfo	sSampleRateChangeInfo;
										/* Sample rate change information */
} BRAP_DSPCHN_P_SampleRateChangeInfo;

/***************************************************************************
Summary:
	This structure contains Stream Information private to RAP PI.
***************************************************************************/
typedef struct BRAP_DSPCHN_P_StreamInfo
{
	BRAP_DSPCHN_P_SampleRateChangeInfo sPrivateFsChangeInfo; /* RAP private
											sample rate change structure */
	BRAP_DSPCHN_StreamInfo	sStreamInfo; /* Stream information */
} BRAP_DSPCHN_P_StreamInfo;

/***************************************************************************
Summary:
	This enumeration defines current state of the DSP channel 
***************************************************************************/
typedef enum BRAP_DSPCHN_P_ChState
{
	BRAP_DSPCHN_P_ChState_eStart,
	BRAP_DSPCHN_P_ChState_eStop
}BRAP_DSPCHN_P_ChState;

typedef struct BRAP_DSPCHN_P_PtsBasedStartStop
{
#if (BRAP_NEW_TIMING_MARKER==0)
	uint32_t	ui32NumPtsPairProgrammed;
	uint32_t	ui32StartPts0;
	uint32_t 	ui32StopPts0;
	uint32_t	ui32StartPts1;
	uint32_t 	ui32StopPts1;
#else
    BRAP_DSPCHN_FwTimingMarker      sTimingMarker;
    BRAP_DSPCHN_HostTimingMarker    sHostTimingmarker;
#endif
} BRAP_DSPCHN_P_PtsBasedStartStop;

#if ((BRAP_7405_FAMILY == 1) || (BRAP_3548_FAMILY == 1) )
/***************************************************************************
Summary:
	Structure for Setting the post process stages in a post processing branch
    Note: This structure is exposed through RAP PI for 3563.	
****************************************************************************/

typedef struct BRAP_PpStageSettings
{
	BRAP_ProcessingType	ePpAlgo[BRAP_MAX_PP_PER_BRANCH_SUPPORTED];
					/* Stage wise post processing 
					   algorithms. If no post
					   processing required for a
					   stage, then corresponding
 					   entry should be set to
				 	   BRAP_ProcessingType_eMax */
} BRAP_PpStageSettings;

typedef struct BRAP_DSPCHN_DdbmConfigParams
{
    BRAP_OutputMode        			eAcmod;
	BAVC_AudioSamplingRate			eInputSamplingRate;
    BRAP_OutputMode        			eOpMode;
    bool                    		bInLfeOn;
    bool                    		bOutLfeOn;
	BRAP_DSPCHN_Ddbm_FilterCutOff 	eFilterCutOff;	
	BRAP_DSPCHN_AudioType 			eAlgoType; /*if user doesn't know primary
										algo id, pass BRAP_DSPCHN_Algo_eMax.
										Else pass a valid value. */
	bool							bInterleaved;
	bool							bBmEnable;										
    BRAP_DSPCHN_DdbmMode    		eDdbmMode;

    bool                            bEnableUserGainSettings;
                                            /* if True User gain Setting is applied
                                               if False, user gains not applied.*/
    unsigned int                    uiConfig1Gain[BRAP_RM_P_MAX_OP_CHANNELS];
                                            /* 
                                                ui32Config1Gain[0] = front left
                                                ui32Config1Gain[1] = center
                                                ui32Config1Gain[2] = front right
                                                ui32Config1Gain[3] = surround left
                                                ui32Config1Gain[4] = surround right
                                                ui32Config1Gain[5] = back left
                                                ui32Config1Gain[6] = back right
                                                ui32Config1Gain[7] = LFE
                                            */
    unsigned int                    uiConfig2aGain[6];
                                            /*
                                                ui32Config2aGain[0] = center
                                                ui32Config2aGain[1] = surround left
                                                ui32Config2aGain[2] = surround right
                                                ui32Config2aGain[3] = back left
                                                ui32Config2aGain[4] = back right
                                                ui32Config2aGain[5] = LFE
                                            */
    unsigned int                    uiConfig2bGain[6];
                                            /*
                                                ui32Config2bGain[0] = center
                                                ui32Config2bGain[1] = back left to front
                                                ui32Config2bGain[2] = back right to front
                                                ui32Config2bGain[3] = back left to surround
                                                ui32Config2bGain[4] = back right to surround
                                                ui32Config2bGain[5] = LFE 
                                            */
}BRAP_DSPCHN_DdbmConfigParams;

typedef struct BRAP_DSPCHN_DtsNeoConfigParams
{
    BRAP_OutputMode                 eOpMode;
    bool                            bLfeOn;
    BRAP_DSPCHN_FidelitySubBands    eFidelitySubBands;
    BRAP_DSPCHN_CinemaMode          eCinemaMode;
    BAVC_AudioSamplingRate          eSamplingRate;
	unsigned int	                uiCenterGain;
                                    /* Center gain. Input value range = 0% to 100%. 
                                       100% corresponds to value of unity. Any 
                                       value above 100% is considered as unity */

}BRAP_DSPCHN_DtsNeoConfigParams;

typedef struct BRAP_PP_ConfigParams
{	
	unsigned int				uiPpBranchId;	
	unsigned int				uiPpStage;	
	BRAP_ProcessingType	ePpAlgo;	
	union	
	{		
		BRAP_DSPCHN_TruSurroundXtParams		    sTruSurroundXtParams;	    /* TruSurroundXT config parameters */		
		BRAP_DSPCHN_AVLConfigParams		        sAvlConfigParams;		    /* Auto Volume Level Config params */		
		BRAP_DSPCHN_ProLogicllConfigParams	    sProLogicllConfigParams;    /* ProLogic-II config params */	
		BRAP_DSPCHN_DdbmConfigParams            sDdbmConfigParams;          /* DDBM config params */
		BRAP_DSPCHN_DtsNeoConfigParams          sDtsNeoConfigParams;        /* DDBM config params */
		BRAP_DSPCHN_XenConfigParams		        sXenConfigParams;           /* XEN config params */
		BRAP_DSPCHN_BbeConfigParams		        sBbeConfigParams;	        /* BBE config params */
        BRAP_DSPCHN_CustomSurroundConfigParams  sCustomSurroundConfigParams;/* CUSTOM SURROUND config params */
        BRAP_DSPCHN_CustomBassConfigParams      sCustomBassConfigParams;    /* CUSTOM BASS config params */        
        BRAP_DSPCHN_CustomVoiceConfigParams     sCustomVoiceConfigParams;   /* CUSTOM VOICE config params */                
        BRAP_DSPCHN_PeqConfigParams             sPeqConfigParams;           /* PEQ config params */                        
	} uConfigParams;
} BRAP_PP_ConfigParams;

#endif 

typedef struct BRAP_DSPCHN_P_PpBranchInfo
{
	unsigned int			uiParentId;	
	unsigned int			uiForkPt;	
    BRAP_OutputPort         eOutputPort;
	BRAP_PpStageSettings	sPpStgSettings;
} BRAP_DSPCHN_P_PpBranchInfo;



/*************************************************************************
Summary:
        Structure for describing task information.
Description:
        This structure describes the firmware task information.
***************************************************************************/
typedef struct BRAP_DSPCHN_P_FirmwareTaskInfo
{
	BRAP_FWIF_P_FwTaskHandle		hFwTask;
	BRAP_P_AudioProcNetwork	sProcessingNw;
	BRAP_FWIF_P_TaskInterface	sInputInterface;
	BRAP_FWIF_P_TaskInterface	sOutputInterface;
} BRAP_DSPCHN_P_FirmwareTaskInfo;

/***************************************************************************
Summary:
	Handle structure for DSP context.
***************************************************************************/
typedef struct BRAP_DSPCHN_P_Channel
{
	uint32_t				channelIndex;			/* DSP context number */
	BRAP_DSPCHN_P_Settings	sSettings;				/* setting that is public */

	BCHP_Handle				hChip;					/* handle to chip object */
	BREG_Handle				hRegister;				/* handle to register object */
	BMEM_Handle				hHeap;					/* handle to memory object */
	BINT_Handle				hInt;					/* handle to interrupt object */

    bool bChSpecificDspCh;   /* TRUE = DSPCHN is channel specific,
                                FALSE = DSPCHN belongs to association */
    union
    {
        BRAP_ChannelHandle hRapCh; /* Valid if DSPCHN is channel specific, bChSpecificDspCh = true */
        BRAP_AssociatedChannelHandle hAssociation; /* Valid if DSPCHN is shared between channels, 
                                                              bChSpecificDspCh = false */
    } uHandle;
	uint32_t				chCfgRegOffset;			/* channel configuration reg offset */
	uint32_t				chEsrSiRegOffset;	/* ESR SI register offset for the channel */
	uint32_t				chEsrSoRegOffset;	/* ESR SO register offset for the channel */
	BRAP_DSPCHN_P_AudioParams		sDspAudioParams;	/* Audio parameters for the channel */
	BRAP_DSPCHN_P_ChState			eChState;		/* Channel state */
	bool					bFirstPtsPairProgrammed;
	BRAP_DSPCHN_P_PtsBasedStartStop	sPtsBasedStartStop;	
#if ( (BRAP_3548_FAMILY == 1) )
    unsigned int            uiPPBranchId; 
	BRAP_DSPCHN_P_AudioParams		sDspAudioParamsforPPonSPDIFInput;	/* Audio parameters for the channel */
#endif
	BRAP_DSPCHN_P_FirmwareTaskInfo	sFwTaskInfo[BRAP_FWIF_P_MAX_FW_TASK_PER_DSPCHN];
    BRAP_P_NetworkInfo      sNetworkInfo;
}BRAP_DSPCHN_P_Channel;

typedef struct BRAP_DSPCHN_P_InterChanDelay
{
    int iChDelay[BRAP_RM_P_MAX_OP_CHANNELS];
    unsigned int    uiChannelIdWithMaxDelay; /* 0 for L, 1 for R, etc.. */
}BRAP_DSPCHN_P_InterChanDelay;

/***************************************************************************
Summary:
	This enumeration defines various firmwares type that needs to be 
	downloaded.
***************************************************************************/
typedef enum BRAP_DSP_P_FwType
{
	BRAP_DSP_P_FwType_eFs,
	BRAP_DSP_P_FwType_eDec,
	BRAP_DSP_P_FwType_ePp,
	BRAP_DSP_P_FwType_ePt
}BRAP_DSP_P_FwType;
#define BRAP_DSP_P_MAX_FWTYPE 	(BRAP_DSP_P_FwType_ePt+1)
	
/***************************************************************************
Summary:
	Gets DSP default channel setting

Description:
	The default DSP channel configuration settings are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSP_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSP_P_GetDefaultSettings( 
   BRAP_DSP_P_Settings	*psDefSettings	/* [out] The DEC channel default configuration setting */
   );

/***************************************************************************
Summary: Opens an BRAP_DSP channel 

Description:
	This function is responsible for opening a DSP channel. 
	When a BRAP_DSP channel is opened, it will create a module channel handle 
	and configure the module based on the channel settings. 
	Once a channel is opened, it must be closed before it can be opened 
	again.

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSP_P_Close,
	BRAP_DSP_P_GetDefaultSettings
**************************************************************************/
BERR_Code 
BRAP_DSP_P_Open(
	BRAP_DSP_Handle			*phDsp,			/* [out] DSP handle */
	BRAP_Handle				hRap,			/* [in] AUD handle */
	uint32_t				dspIndex,		/* [in] Channel index starting from 0 to max number of channels */ 
	const BRAP_DSP_P_Settings	*pDefSettings	/* [in] The DSP channel configuration setting */
	);

/***************************************************************************
Summary: Closes an BRAP_DSP channel

Description:
	This function is responsible for closing a DSP channel and releasing
	all the resources associated with this channel.
	

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSP_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSP_P_Close( 
	BRAP_DSP_Handle handle /* [in] DSP handle */
	);

/***************************************************************************
Summary: Gets the watchdog recovery flag 

Description:
	This function returns current status of watchdog recovery flag. This
	is required to know whether a function is getting called in watchdog
	context or from application.

Returns:
	Watchdog flag

See Also:
**************************************************************************/
bool
BRAP_DSP_P_GetWatchdogRecoveryFlag(BRAP_DSP_Handle hDsp);

/***************************************************************************
Summary:
	Gets DSP context default settings

Description:
	The default configuration for DSP context (DSPCHN channel) are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_GetDefaultSettings( 
   BRAP_DSPCHN_P_Settings	*psDefSettings	/* [out] The DEC channel default configuration setting */
   );

/***************************************************************************
Summary:
	Gets DSP context current settings

Description:
	The current configuration for DSP context (DSPCHN channel) are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_GetCurrentSettings(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
   BRAP_DSPCHN_P_Settings	*psDefSettings	/* [out] The DEC channel default configuration setting */
   );

/***************************************************************************
Summary:
	Gets DSP context default params 

Description:
	The default parameters for DSP context (DSPCHN channel) are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_GetDefaultParams( 
   BRAP_DSPCHN_P_AudioParams	*psDefParams	/* [out] The DEC channel default parameters */
   );

/***************************************************************************
Summary: Opens a DSP context 

Description:
	This function is responsible for opening a DSP context (DSPCHN channel). 
	When a DSP context is opened, it will create a module channel handle 
	and configure the module based on the channel settings. 
	Once a channel is opened, it must be closed before it can be opened 
	again.

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Close,
	BRAP_DSPCHN_P_GetDefaultSettings
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_Open(
	BRAP_DSPCHN_Handle		*phDspChn,		/* [out] DSP channel handle */
	void	                *pHandle,           /* [in] Raptor Channel or Association handle */	
    bool                    bChSpecificDspCh,   /* [in] DSP channel handle is Raptor Channel 
                                                        or Association specific */	
	const BRAP_DSPCHN_P_Settings	*pDefSettings	/* [in] The DSP channel configuration setting */
	);

/***************************************************************************
Summary: Closes an DSP context

Description:
	This function is responsible for closing a DSP context and releasing
	all the resources associated with this context.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_Close( 
	BRAP_DSPCHN_Handle hDspChn /* [in] DSP handle */
	);

/***************************************************************************
Summary: This is a dummy start for a DSP Context

Description:
	This function is responsible for copying the DSP Channel parameters from
	Audio params. It does not do the actual DSP channel start.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Start
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_DummyStart(
	BRAP_DSPCHN_Handle	pDspChn,					/* [in] DSPCHN handle */
	const BRAP_DSPCHN_P_AudioParams *psAudioParams	/* [in] Audio decode parameters */
	);
	
/***************************************************************************
Summary: Starts a DSP context

Description:
	This function is responsible for starting a DSP context for the given
	input decode mode.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Stop
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_Start(
	BRAP_DSPCHN_Handle	pDspChn,					/* [in] DSPCHN handle */
	const BRAP_DSPCHN_P_AudioParams *psAudioParams	/* [in] Audio decode parameters */
	);
	
/***************************************************************************
Summary: Creates DSP Out Config in NetWorkInfo

Description:
	This function is responsible for creating the whole DSP Output Config.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Start
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_CreateDstConfig(
	BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
	const BRAP_DSPCHN_P_AudioParams *psAudioParams	/* [in] Audio decode parameters */
	);
	

/***************************************************************************
Summary: Stops a DSP context

Description:
	This function stops a currently running context
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Start
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_Stop(
	BRAP_DSPCHN_Handle	pDspChn					/* [in] DSPCHN handle */
	);

/***************************************************************************
Summary:
    Private Structre BRAP_P_IOBufferDetails holds Input/Output Buffer details 
    i.e., FMM Ringbuffer Ids or DRAM Buffer pointers. The buffers of interest 
    here carry the outputs of Decode/PB/PCM capture channel, fed to the the 
    Association network(currently FW Mixer task).
**************************************************************************/
typedef enum BRAP_P_IOBufferType
{
    BRAP_P_IOBufferType_eFMM,
    BRAP_P_IOBufferType_eDRAM,
    BRAP_P_IOBufferType_eMax
}BRAP_P_IOBufferType;

typedef struct BRAP_P_CircularBuffer
{
	uint32_t	ui32BaseAddr;		/*	Circular buffer's base address */
	uint32_t	ui32EndAddr;		/*	Circular buffer's End address */
	uint32_t	ui32ReadAddr;		/*	Circular buffer's read address */
	uint32_t	ui32WriteAddr;		/*	Circular buffer's write address */
	uint32_t	ui32WrapAddr;		/*	Circular buffer's wrap address */
}BRAP_P_CircularBuffer;

typedef struct BRAP_P_IODRAMBuffer
{
    unsigned int            uiNumBuffers;
    BRAP_P_CircularBuffer   sIOCircularBuf[BRAP_AF_P_MAX_CHANNELS];
    BRAP_P_CircularBuffer   sIOGenericCircularBuf;    
}BRAP_P_IODRAMBuffer;

typedef struct BRAP_P_IOBufferID
{
    unsigned int            uiNumBuffers;
    uint32_t                ui32RbufId[BRAP_RM_P_MAX_OP_CHANNELS];
}BRAP_P_IOBufferID;

typedef struct BRAP_P_IOBufferDetails
{
    BRAP_P_IOBufferType     eBufferType;
    union
    {
        BRAP_P_IODRAMBuffer     sIOBuffer;
        BRAP_P_IOBufferID       sIOBufId;        
    }uBufferDetails;
}BRAP_P_IOBufferDetails;
/**************************************************************************/

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT

/***************************************************************************
Summary: Dynamically add an input to a task

Description:
	This function enables successive inputs of a running task. 

	In MS11 case FW Mixer being a task takes multiple inputs and any input 
	addition need to be seamless. So the DSP Channel(inc CIT and Task) need not 
	be restarted, instead the below API is called which takse the address of the 
	CIT of FW Mixer and the details of the input to be enabled, and also the 
	address of the DRAM where the updated CIT is to be stored. This updated CIT 
	address along with the current FW mixer CIT address need to be passed to FW 
	through command interface for FW to switch to the new CIT seamlessly.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Start
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_AddRemoveInputToTask(
	BRAP_ChannelHandle 	    hRapCh,		        /* [in] RAP Channel handle */
	BRAP_DSPCHN_Handle	    hDspCh,             /* [in] DSPCHN handle */
    bool                    bAddInput ,
    BRAP_P_IOBufferDetails  *psIOBuffer
	);

#endif

BERR_Code BRAP_DSPCHN_P_StopVideo(
	BRAP_DSPCHN_Handle	pDspChn					/* [in] DSPCHN handle */
	);


void
BRAP_DSPCHN_P_TsmLog_isr(
	BRAP_DSPCHN_Handle	hDspCh,			/* [in] DSP channel handle */
	BRAP_DSPCHN_TsmLogInfo	*psTsmLogInfo);	/* [out] Information about logged TSM data */

BERR_Code
BRAP_DSPCHN_P_EnablePause(
						BRAP_DSPCHN_Handle      hDspCh,	/* [in] DSPCHN handle */
						bool					bOnOff		/* [In] TRUE = Pause video
																FALSE = Resume video */
						);
BERR_Code
BRAP_DSPCHN_P_GetOneAudioFrameTime(
						BRAP_DSPCHN_Handle      hDspChn,			/* [in] DSPCHN handle */
						unsigned int			*puiOneFrameTime	/* [out] One Audio Frame Time */
						);

BERR_Code
BRAP_DSPCHN_P_FrameAdvance(
						BRAP_DSPCHN_Handle      hDspCh,			/* [in] DSPCHN handle */
						unsigned int			uiFrameAdvTime				/* [In] Frame advance time in msec */
						);

void BRAP_DSP_P_GetFwVersionInfo(
		BRAP_DSP_Handle hDsp,
		BRAP_DSPVersionInfo	*psDSPVersion
		);

/* BRAP_DSP_P_InitInterframeBuffer: This function initializes interframe buffer as per the 
 * input audio type and programs corresponding entry in MIT. */
BERR_Code BRAP_DSP_P_InitInterframeBuffer(
				BRAP_FWIF_P_FwTaskHandle	hTask);

/* BRAP_DSP_P_InitInterframeBuffer: This function initializes status buffer  */
BERR_Code BRAP_DSP_P_InitStatusBuffer(
				BRAP_FWIF_P_FwTaskHandle	hTask);


BERR_Code BRAP_DSPCHN_P_GetCurrentAudioParams (
						BRAP_DSPCHN_Handle	hDspCh,
						BRAP_DSPCHN_P_AudioParams	*psDspChAudioParams
						);

bool BRAP_DSPCHN_P_GetDecoderPauseState (
						BRAP_DSPCHN_Handle	hDspCh
						);

BERR_Code
BRAP_DSPCHN_P_SetConfig (
	BRAP_DSPCHN_Handle	hDspCh,
	BRAP_DEC_DownmixPath	eDownmixPath,
	BRAP_DSPCHN_AudioType eType
	);

BERR_Code BRAP_DSP_P_EnableDspWatchdogTimer (
						BRAP_DSP_Handle		hDsp,
						bool				bEnable
						);

void BRAP_DSP_P_WatchDogResetHardware(BRAP_DSP_Handle hDsp);



/***************************************************************************
Summary:
    This private routine returns the DSPCH handle associated with the
    decode channel handle.
**************************************************************************/
BRAP_DSPCHN_Handle 
BRAP_DSPCHN_P_GetDspChHandle(
	void	                *pHandle,       /* [in] Raptor Channel or Association handle */
	bool                    bChSpecificDspCh	
	);

/***************************************************************************
Summary:
    Isr version of BRAP_DSPCHN_P_GetDspChHandle
**************************************************************************/
#define BRAP_DSPCHN_P_GetDspChHandle_isr(pHandle, bChSpecific) \
     BRAP_DSPCHN_P_GetDspChHandle(pHandle, bChSpecific)

    
void BRAP_DSPCHN_P_AddWordToFWBuffer(
					unsigned char *bufPtr,
					uint32_t data,
					uint32_t nBytes);

/***************************************************************************
Summary:
    This Privte function calls a CIT function that updates a DRAM memory provided
    by PI with the updated FMM Port Config and this address needs to be given to
    FW through FMM Port Config command to reconfigure the output/capture ports accordingly
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_FMMPortReconfig (
    BRAP_Handle             hRap,
	BRAP_ChannelHandle      hRapCh,
    BRAP_DstDetails         *psExtDstDetails,
	bool                    bPortEnable
	);

/***************************************************************************
Summary:
    Destroys audio processing stage.

Description:
    This function free up the processing Handle memory.This function will not free's the memory 
    which is allocated in BRAP_CreateProcessingStage.To free the memory which is allocated in 
    create stage,we must call the BRAP_DestroyProcessingStage.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CreateProcessingStage,BRAP_DestroyProcessingStage
    
**************************************************************************/
BERR_Code BRAP_P_DestroyProcessingStage(
    BRAP_ProcessingStageHandle   hAudioProcessingStageHandle  
                                                                        /* [in] Handle for the audio processing
                                                                            stage */
);

/********* New Changes***************/

#define BRAP_DSP_P_MAX_AUD_PROCESSING_CTX 6
#define BRAP_DSP_P_MAX_FS_CTXT  2
#if (BRAP_7405_FAMILY ==1)   
#define BRAP_DSP_P_MAX_DECODE_CTXT 2
#else
#define BRAP_DSP_P_MAX_DECODE_CTXT 4
#endif
#define BRAP_DSP_P_MAX_PASSTHRU_CTXT 1

#define BRAP_DSP_P_MAX_ENCODE_CTXT 2

#ifdef RAP_REALVIDEO_SUPPORT
#define BRAP_DSP_P_MAX_VIDEODECODE_CTXT 1
#endif

#ifdef RAP_GFX_SUPPORT
#define BRAP_DSP_P_MAX_GFX_CTXT 1
#endif
#ifdef RAP_SCM_SUPPORT
#define BRAP_DSP_P_MAX_SCM_CTXT 1
#endif

#define	BRAP_DSP_P_MAX_TSM_FW_TYPE BRAP_FWIF_P_MAX_TSM_FW_TYPE



/***************************************************************************
Summary:
	This structure contains the Buffer Properties.
***************************************************************************/
typedef struct BRAP_DSP_P_CtxtFwBufInfo
{
	uint32_t				ui32BaseAddr;
	uint32_t				ui32Size;
	union
	{
		BRAP_DSPCHN_AudioType				eAlgoType;
		BRAP_ProcessingType 			ePAlgoType;
#ifdef RAP_REALVIDEO_SUPPORT
		BRAP_DSPCHN_VideoType				eVideoAlgoType;
#endif
#ifdef RAP_GFX_SUPPORT        
		BRAP_DSPCHN_GfxType				eGfxAlgoType;        
#endif
#ifdef RAP_SCM_SUPPORT
		BRAP_DSPCHN_ScmType				eScmAlgoType;
#endif
	}uExecAlgoType;
	
	int32_t				numUser;
}BRAP_DSP_P_CtxtFwBufInfo;


/*******************************************************************************

Note: In case of OpenTimeDownload, The memory allocation for following should 
be consecutive and in the same order as it is mentioned in this datstructure.

*******************************************************************************/
typedef struct BRAP_DSP_DwnldMemInfo
{
	uint32_t					ui32SystemRdbVariablesPtr; /*Ptr to buffer  for System RDB Variables*/
	uint32_t					ui32SystemRdbVariablesSize; /*Size of buffer  for System RDB Variables*/    
	uint32_t					ui32SystemIbootCodePtr; /*Ptr to buffer  for System Iboot Code*/
	uint32_t					ui32SystemIbootCodeSize; /*size of buffer  for System Iboot Code*/

	uint32_t					ui32SystemCodePtr;/*Ptr to buffer for system
													resident code*/
	uint32_t					ui32SystemCodeSize;/*size of buffer for system
													resident code*/
	uint32_t					ui32SystemDataPtr;/*Ptr to buffer for system
													resident code*/
	uint32_t					ui32SystemDataSize;/*size of buffer for system
													resident code*/
#ifdef RAP_SCM_SUPPORT														
	uint32_t					ui32SystemTableOfHashesPtr;/*Ptr to buffer for system
													table of hashes*/
	uint32_t					ui32SystemTableOfHashesSize;/*size of buffer for system
													table of hashes*/
#endif														
														

	uint32_t					ui32TsmCodePtr;
	uint32_t					ui32TsmCodeSize;													
						/*This will have the information regarding the ptr and 
						size of buffer allocated to download all the Tsm Exec.*/			

	uint32_t					ui32SystemTaskCodePtr;
	uint32_t					ui32SystemTaskCodeSize;													
						/*This will have the information regarding the ptr and 
						size of buffer allocated to download all the System Task code.*/		                        

	BRAP_DSP_P_CtxtFwBufInfo	sProcessingAlgoBufInfo[BRAP_DSP_P_MAX_AUD_PROCESSING_CTX];
						/*This will have the information regarding the Buffer 
						for Processing Algorithm.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						BRAP_FWIF_P_MAX_PROCESSING_CTX = Max no. of Processing 
						algorithm which can simultaneously execute at a time in 
						the DSP*/

	BRAP_DSP_P_CtxtFwBufInfo	sFsBufInfo[BRAP_DSP_P_MAX_FS_CTXT]; 
						/*This will have the information regarding the all the 
						buffers for Frame Sync.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the Buf for downloading 
						framesync of all the supported algorithm
						BRAP_FWIF_P_MAX_FS_CTXT = Max no. of Framesync Executable
						whose algorithm can simultaneously execute at a time in 
						the DSP*/

	BRAP_DSP_P_CtxtFwBufInfo	sDecCodecBufInfo[BRAP_DSP_P_MAX_DECODE_CTXT];
						/*This will have the information regarding the all the 
						buffers for Decode Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Decode Exec of all the supported algorithm 
						BRAP_FWIF_P_MAX_CODEC_CTXT = Max no. of Decode
						executable which can simultaneously execute at a time in 
						the DSP*/

	BRAP_DSP_P_CtxtFwBufInfo	sPtCodecBufInfo[BRAP_DSP_P_MAX_PASSTHRU_CTXT];
						/*This will have the information regarding the all the 
						buffers for Passthru Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Passthru Exec of all the supported algorithm 
						BRAP_FWIF_P_MAX_CODEC_CTXT = Max no. of Decode
						executable which can simultaneously execute at a time in 
						the DSP*/

	BRAP_DSP_P_CtxtFwBufInfo	sEncodeCodecBufInfo[BRAP_DSP_P_MAX_ENCODE_CTXT];
						/*This will have the information regarding the all the 
						buffers for Passthru Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Passthru Exec of all the supported algorithm 
						BRAP_FWIF_P_MAX_CODEC_CTXT = Max no. of Decode
						executable which can simultaneously execute at a time in 
						the DSP*/
#ifdef RAP_REALVIDEO_SUPPORT
	BRAP_DSP_P_CtxtFwBufInfo	sVideoDecCodecBufInfo[BRAP_DSP_P_MAX_VIDEODECODE_CTXT];
						/*This will have the information regarding the all the 
						buffers for Decode Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Decode Exec of all the supported algorithm 
						BRAP_FWIF_P_MAX_CODEC_CTXT = Max no. of Decode
						executable which can simultaneously execute at a time in 
						the DSP*/						

#endif
#ifdef RAP_GFX_SUPPORT
BRAP_DSP_P_CtxtFwBufInfo	sGfxBufInfo[BRAP_DSP_P_MAX_GFX_CTXT];
						/*This will have the information regarding the all the 
						buffers for Passthru Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Passthru Exec of all the supported algorithm 
						BRAP_FWIF_P_MAX_CODEC_CTXT = Max no. of Decode
						executable which can simultaneously execute at a time in 
						the DSP*/                        
#endif						
#ifdef RAP_SCM_SUPPORT
BRAP_DSP_P_CtxtFwBufInfo	sScmBufInfo[BRAP_DSP_P_MAX_SCM_CTXT];
						/*This will have the information regarding the all the 
						buffers for SCM Exec.
						If the OpenTimeDownload is True, The the 0th Index will
						contain the information of the buffer for downloading 
						Passthru Exec of all the supported algorithm 
						BRAP_DSP_P_MAX_SCM_CTXT = Max no. of SCM
						executable which can simultaneously execute at a time in 
						the DSP*/                        
#endif						

}BRAP_DSP_DwnldMemInfo;


/***************************************************************************
Summary:
	This structure is one to one mapping between Exec_Image_Id and the Exec_Id 
	(BAF Ids) common between PI and FW. This will also contain the sizes of the 
	Exec_image. 
		Since for the Exec_Image_Id of Decode table, there is no Exec_Id, so it will store 
	the if its already downloaded.

	Also ui32InterframeImgId stores the Image id of interframe Fw image for each ExecId.
***************************************************************************/
typedef struct BRAP_ImgIdMappingArrays
{
	uint32_t ui32CodeImgId[BRAP_AF_P_AlgoId_eMax];
	uint32_t ui32CodeSize[BRAP_AF_P_AlgoId_eMax];
	uint32_t ui32TableImgId[BRAP_AF_P_AlgoId_eMax];	
	uint32_t ui32TableSize[BRAP_AF_P_AlgoId_eMax];	
	uint32_t ui32InterframeImgId[BRAP_AF_P_AlgoId_eMax];
	uint32_t	ui32InterframeSize[BRAP_AF_P_AlgoId_eMax];
} BRAP_ImgIdMappingArrays;

void BRAP_MapImgIdWithExecId(BRAP_Handle hRap,BRAP_ImgIdMappingArrays *psImgIdMappingArrays);
void BRAP_UpdateMitEntry(
		BRAP_Handle		hRap,
		BRAP_AF_P_AlgoId	eExecId,
		uint32_t		ui32CodePtr,
		uint32_t 		ui32CodeSize,
		uint32_t		ui32TblPtr,		
		uint32_t		ui32TblSize				
		);

void BRAP_InitializeMit(BRAP_Handle	hRap);
void BRAP_PrintMit(BRAP_Handle	hRap);

BERR_Code BRAP_DSP_GetDownloadMemRequirement(
		BRAP_Handle	 hRap,
		BRAP_DSP_DwnldMemInfo *pDwnldMemInfo,
		uint32_t	*pui32Size);

BERR_Code BRAP_DSP_P_InitHardware(BRAP_DSP_Handle hDsp, bool bFwAuthEnabled);

/***************************************************************************
Summary: Enable/Disable Time Stamp Management

Description:
	This function enables/disables Time Stamp Management for a given audio
	channel. 
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code 
BRAP_GetTsmNodeInfo(
    BRAP_ChannelHandle	            hRapCh,             /*[In] Rap channel handle */
	BRAP_DSPCHN_Handle              hDspCh,             /*[In] Dsp CHannel handle */
    unsigned int                    *pConfigBufAddr,    /*[Out] Config buffer address */
    BRAP_FWIF_P_FwTaskHandle        *hFwTask,       /*{out] task handle */    
    BRAP_FWIF_P_TsmConfigParams     **psTsmConfigParam /*[out] Tsm config param*/   
);

#define BRAP_GetTsmNodeInfo_isr(hRapCh,hDspCh,pConfigBufAddr,hFwTask,psTsmConfigParam) \
    BRAP_GetTsmNodeInfo(hRapCh,hDspCh,pConfigBufAddr,hFwTask,psTsmConfigParam)


/**************************************/
BERR_Code BRAP_DSP_AllocMem(BRAP_Handle hRap);



BERR_Code BRAP_DSP_FreeMem(BRAP_Handle hRap);


 BERR_Code
BRAP_DSPCHN_P_CreateFwTask(
        BRAP_DSPCHN_Handle hDspCh,
        BRAP_P_AudioProcNetwork     *psAudProcNetwork
        );

 BERR_Code BRAP_DSPCHN_P_AllocateFwCtxtBuf(
	BRAP_Handle hRap,
	uint32_t eAudType,	/* For PostProcessing it wiil be BRAP_ProcessingType. For Decode it will be audio type*/
	uint32_t *pui32FwCtxIndx,
	bool *bDownload,
	BRAP_DSP_ExecType eExecType,
	bool bWatchDogRecovery);

BERR_Code BRAP_DSPCHN_P_FreeFwCtxtBuf(
	BRAP_Handle hRap,
	uint32_t eAudType,	/* For PostProcessing it wiil be BRAP_ProcessingType. For Decode it will be audtio type*/
	BRAP_DSP_ExecType eExecType);

/***************************************************************************
Summary: Starts a DSP context

Description:
	This function is responsible for starting a DSP context for the given
	input decode mode.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Stop
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_StartVideo(
	BRAP_DSPCHN_Handle	hDspCh,					/* [in] DSPCHN handle */
	const BRAP_DSPCHN_P_AudioParams *psAudioParams	/* [in] Audio decode parameters */
	);


#ifdef __cplusplus
}
#endif

#endif /* BRAP_DSP_PRIV_H__ */

/* End of File */
