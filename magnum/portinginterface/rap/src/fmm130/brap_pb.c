/***************************************************************************
*     Copyright (c) 2004-2010, Broadcom Corporation
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
/*{{{ Includes */
#include "brap_priv.h"
#include "brap_pcm.h"
/*}}}*/

BDBG_MODULE(rap_pb); /* Register software module with debug interface */

/*{{{ Defines */

/* Free byte water mark in percentage. Once, buffer usage increases this value an 
   interrupt is generated */
#define BRAP_PB_P_RBUF_DEFAULT_FREE_BYTE_MARK 	75
#define BRAP_PB_P_MIXER_DEFAULT_SCALE_VALUE     0 /* 0 dB => No scaling */ 
#define BRAP_PB_P_MAX_OUTPUT_MODE 				0x08/* Derived from BRAP_OutputMode */
#define BRAP_PB_P_DEBUG_PCM 					0	/* Debug flag */
/*}}}*/

/*{{{ Typedefs */

/*}}}*/

/*{{{ Static Variables & Function Prototypes */

/* This array gives the corresponding LRDataControl */
BRAP_SRCCH_P_LRDataControl lrDataCtrl[] = 
{
	BRAP_SRCCH_P_LRDataControl_L_2_LR, /* BRAP_PcmOutputMode_eLeftMono */
    BRAP_SRCCH_P_LRDataControl_R_2_LR, /* BRAP_PcmOutputMode_eRightMono */
    BRAP_SRCCH_P_LRDataControl_LR_2_LR,/* BRAP_PcmOutputMode_eStereo */
    BRAP_SRCCH_P_LRDataControl_LR_2_LR /* BRAP_PcmOutputMode_eMax */
};

/*
	Supported configurations with various BRAP_PcmDataMode:
	------------- --------------------------------------------------	
	|			 |		  	*BRAP_PcmDataMode*					   | 
	|O/p Mode 	 | eMono - StereoInterleaved - StereoNonInterleaved|		
	-------------|------   -----------------   --------------------|
	|Left Mono	 | NO	 | YES				 | YES				   |
	|Right Mono	 | NO 	 | YES				 | YES	 			   |
	|Stereo		 | YES	 | YES				 | YES				   |	
	----------------------------------------------------------------
*/
bool bValidatePcmOpNDataModes[BRAP_PcmOutputMode_eMaxNum]	
									[BRAP_BufDataMode_eMaxNum] = 
{  	/* 	{eMono, stereoInterleaved, stereoNonInterleaved}*/  /* ePcmOutputMode */  
		{false, true, 				true}, 					/* eLeftMono */
		{false, true, 				true}, 					/* eRightMono*/	
		{true, 	true, 				true} 					/* eStereo   */   
};

static BERR_Code BRAP_PB_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	BRAP_RingBufInfo	*pRBufInfo
	);

static BERR_Code BRAP_PB_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	unsigned int		uiSzUpdt
	);
/*}}}*/

/***************************************************************************
Summary:
	Gets the default settings of the PCM Playback Channel.
Description:
	This API returns the default settings of the PCM Playback Channel.
	The default setting is -
		1. output port - I2s0(7411) or Dac0(others)
		2. Output mode - stereo
		3. PCM data mode - stereo interleaved (single ring buffer)
		4. water mark - 50% 
		5. with internal ring buffersReturns:
	BERR_SUCCESS 
See Also:
	BRAP_PB_Open.
***************************************************************************/
BERR_Code BRAP_PB_GetChannelDefaultSettings(
	BRAP_Handle 			hRap,		 	/* [in] The RAP device handle */
	BRAP_PB_ChannelSettings *pDefSettings	/* [out] Default channel settings */
	)
{
	BERR_Code err = BERR_SUCCESS;
			
	BDBG_ENTER(BRAP_PB_GetChannelDefaultSettings);

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pDefSettings);

    BSTD_UNUSED(hRap);

#ifdef BCHP_7411_VER
	pDefSettings->eOutputPort 	= BRAP_OutputPort_eI2s0;   
#else
	pDefSettings->eOutputPort 	= BRAP_OutputPort_eDac0; 
#endif /* BCHP_7411_VER */

	pDefSettings->eBufDataMode 	= BRAP_BufDataMode_eStereoInterleaved;

	/* PCM stereo-interleaved needs a single buffer, so only left buffer be valid */
	pDefSettings->sLtOrSnglRbufSettings.pBufferStart = NULL; 
	pDefSettings->sLtOrSnglRbufSettings.uiSize = BRAP_RBUF_P_DEFAULT_SIZE;
	pDefSettings->sLtOrSnglRbufSettings.uiWaterMark = BRAP_PB_P_RBUF_DEFAULT_FREE_BYTE_MARK;

	/* Right buffer not required for PCM stereo-interleaved */
	pDefSettings->sRtRbufSettings.pBufferStart = NULL; 
	pDefSettings->sRtRbufSettings.uiSize = 0;
	pDefSettings->sRtRbufSettings.uiWaterMark = 0;
	
	BDBG_LEAVE(BRAP_PB_GetChannelDefaultSettings);
	return err;
}


/***************************************************************************
Summary:
	Gets the default audio params for a PCM Playback Channel.
Description:
	This API returns the default parameters for a PCM Playback channel. 
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_Start, BRAP_PB_Stop
***************************************************************************/
BERR_Code BRAP_PB_GetDefaultAudioParams(
	BRAP_ChannelHandle  	hRapCh,		/* [in] The RAP channel handle */
	BRAP_PB_AudioParams 	*pDefParams /* [out] Default channel parameters */
	)
{
	BERR_Code 		ret = BERR_SUCCESS;
	BRAP_OP_SpdifChanStatusParams sSpdifChanStatusParams;
	BRAP_MIXER_P_Params   sMixerParams;
    BRAP_SPDIFFM_P_Params sSpdifFmParams;
	
	BDBG_ENTER(BRAP_PB_GetDefaultAudioParams);
	
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pDefParams);

    BSTD_UNUSED(hRapCh);

	/* 2. Fill up the default PCM playback audio params */
	/* 2.1 Audio output params: sMixerParams, uiOutputBitsPerSample, sSpdifFmParams */
    
    ret = BRAP_MIXER_P_GetDefaultParams(&sMixerParams);
	if(ret != BERR_SUCCESS)
		return BERR_TRACE(ret); 

	/* Note: Default PCM mode is PCM stereo interleaved so mixer input should be 
	         BRAP_MIXER_Input_eStereo */
	sMixerParams.sExtParams.sInputParams.eMixerInputAudioMode = BRAP_MIXER_Input_eStereo; 
	sMixerParams.sExtParams.sInputParams.uiScaleValue = BRAP_PB_P_MIXER_DEFAULT_SCALE_VALUE; 
	pDefParams->sAudioOutputParams.sMixerParams = sMixerParams.sExtParams;

	pDefParams->sAudioOutputParams.uiOutputBitsPerSample = 16;

    ret = BRAP_SPDIFFM_P_GetDefaultParams(&sSpdifFmParams);
	if(ret != BERR_SUCCESS)
		return BERR_TRACE(ret); 
    
    sSpdifFmParams.sExtParams.bSpdifFormat = true; 
  	pDefParams->sAudioOutputParams.sSpdifFmParams = sSpdifFmParams.sExtParams;

	/* 2.2 Delay params */
	pDefParams->uiDelayRbuf = BRAP_RBUF_P_NO_DELAY;
				
	/* 2.3 Max rbuf size */
	pDefParams->ui32RbufMemSize = BRAP_RBUF_P_DEFAULT_SIZE;

	/* 2.4 Get default SPDIF Channel Status parameters */
	ret = BRAP_OP_P_GetDefaultSpdifChanStatusParams(&sSpdifChanStatusParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sSpdifChanStatusParams = sSpdifChanStatusParams;
	else
		return BERR_TRACE(ret); 
	
	/* Note: Input Sampling Freq = any, 
			 Output Sampling freq = 192kHz, 96kHz, 48kHz, 44.1kHz & 32kHz */
	/* 2.5 eInputSamplingRate = BAVC_AudioSamplingRate_e32k */
	pDefParams->eInputSamplingRate = BAVC_AudioSamplingRate_e32k;

	/* 2.6 eInputBitsPerSample = 16 bits/sample */
	pDefParams->eInputBitsPerSample = BRAP_InputBitsPerSample_e16;

	/* 2.7 bLoopArroundEn = true; */
	pDefParams->bLoopAroundEn = true;

    /* 2.9 eBufDataMode */
    pDefParams->eBufDataMode = BRAP_BufDataMode_eStereoInterleaved;

    /* 2.10 ePcmOutputMode */
    pDefParams->ePcmOutputMode = BRAP_PcmOutputMode_eStereo; 

#if BRAP_UNSIGNED_PCM_SUPPORTED
    /* 2.11 bIsSigned */
    pDefParams->bIsSigned = true;
#endif    

	BDBG_LEAVE(BRAP_PB_GetDefaultAudioParams);
	return ret;
}


/***************************************************************************
Summary:
	API used to open a PCM playback channel.
Description:
	It is used to instantiate a PCM playback channel. It allocates channel 
	handle and resource required for the channel, if any.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_CloseChannel
****************************************************************************/
BERR_Code BRAP_PB_OpenChannel( 
	const BRAP_Handle 				hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_ChannelHandle 				*phRapCh,		/* [out] The RAP Channel handle */
	const unsigned int				uiChannelNo,	/* [in] the desired channel ID */	
	const BRAP_PB_ChannelSettings 	*pChnSettings 	/* [in] Channel settings*/ 
	)
{
	BERR_Code 				ret = BERR_SUCCESS;
	BRAP_ChannelHandle		hRapCh = NULL;
	BRAP_RM_P_ResrcReq		sResrcReq;
	BRAP_RBUF_P_Settings	sRbufSettings[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
	BRAP_SRCCH_P_Settings	sSrcChSettings;
	BRAP_MIXER_P_Settings	sMixerSettings;
	BRAP_SPDIFFM_P_Settings	sSpdfFmSettings;
	BRAP_OP_P_MaiSettings	sMaiSettings;
	BRAP_OP_P_SpdifSettings	sSpdifSettings;
	BRAP_OP_P_I2sSettings	sI2sSettings;
	BRAP_OP_P_FlexSettings	sFlexSettings;
	BRAP_OP_P_DacSettings	sDacSettings;
	BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;
	void					*pOpSettings = NULL;
	BRAP_OutputMode 		eOutputMode;
    unsigned int			j;
#ifndef BCHP_7411_VER
    BRAP_P_ObjectHandles        sTempHandles;    
#endif

#if BRAP_P_WATERMARK_WORKAROUND==1
	BTMR_Settings sTimerSettings;
#endif
	
	BDBG_ENTER(BRAP_PB_OpenChannel);

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(phRapCh);
    if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
    {
        BDBG_ASSERT(pChnSettings);

    	BDBG_MSG(("BRAP_PB_OpenChannel: hRap=%d uiChannelNo=%d\n"
    				"\tpChnSettings->eOutputPort=%d\n"
    				"\tpChnSettings->eBufDataMode=%d\n"
    				"\tpChnSettings->sLtOrSnglRbufSettings.pBufferStart=%d\n"
    				"\tpChnSettings->sLtOrSnglRbufSettings.uiSize=%d\n"
    				"\tpChnSettings->sLtOrSnglRbufSettings.uiWaterMark=%d\n"
    				"\tpChnSettings->sRtRbufSettings.pBufferStart=%d\n"
    				"\tpChnSettings->sRtRbufSettings.uiSize=%d\n"
    				"\tpChnSettings->sRtRbufSettings.uiWaterMark=%d\n",
    				hRap, uiChannelNo, 
    				pChnSettings->eOutputPort, pChnSettings->eBufDataMode,
    				pChnSettings->sLtOrSnglRbufSettings.pBufferStart,
    				pChnSettings->sLtOrSnglRbufSettings.uiSize,
    				pChnSettings->sLtOrSnglRbufSettings.uiWaterMark,
    				pChnSettings->sRtRbufSettings.pBufferStart,
    				pChnSettings->sRtRbufSettings.uiSize,
    				pChnSettings->sRtRbufSettings.uiWaterMark));

#ifndef BCHP_7411_VER  /* only for 7411 */    
        BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));
#endif

    	/* 2. Validate if this channel number is less than the max supported */
    	if(uiChannelNo > BRAP_RM_P_MAX_PCM_CHANNELS)
    	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: uiChannelNo(%d) is more than max supported(%d)",
    			uiChannelNo, BRAP_RM_P_MAX_PCM_CHANNELS));
    		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
    		goto end_open;
    	}

    	/* Validate if buffer size is a multiple of 256 bytes */
    	if(	((pChnSettings->sLtOrSnglRbufSettings.uiSize % BRAP_RBUF_P_ALIGNMENT) != 0) ||
    	   	((pChnSettings->sRtRbufSettings.uiSize % BRAP_RBUF_P_ALIGNMENT) != 0))
      	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: Buffer size not a multiple of 256 bytes - Left(%p, Right(%p)",
    			pChnSettings->sLtOrSnglRbufSettings.uiSize, pChnSettings->sRtRbufSettings.uiSize));
    		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
    		goto end_open;
    	}

    	eOutputMode = BRAP_OutputMode_e2_0;

    	/* 4. Check requested o/p port with the currently supported output ports:
    			7411: I2S0 and SPDIF 
    			7401: DAC0, I2S0 and SPDIF
    		  Else, return error - BERR_NOT_SUPPORTED. */
        ret = BRAP_RM_P_IsOutputPortSupported(pChnSettings->eOutputPort) ;
    	if( ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: eOutputPort(%d) is not supported",
    			pChnSettings->eOutputPort));
    		ret = BERR_TRACE(ret);
    		goto end_open;
    	}

        ret = BRAP_P_IsNewChanCompatible(hRap, BRAP_P_ChannelType_ePcmPlayback, pChnSettings->eOutputPort);
        if(ret!=BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_PB_OpenChannel: PB channel not compatible with existing channels"));
            ret = BERR_TRACE(ret);
    		goto end_open;
        }
    		  
    	/* 5. Allocate a PCM Playback Channel handle */	 
    	*phRapCh = hRapCh = (BRAP_ChannelHandle)BKNI_Malloc(sizeof(BRAP_P_Channel));
    	if(hRapCh == NULL)
    	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: Memory allocation for channel handle failed"));
    		ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto end_open;
    	}

    	hRap->hRapPbCh[uiChannelNo] = hRapCh;
    	
    	/* 6. Memset 0 */
    	BKNI_Memset(hRapCh, 0, sizeof(BRAP_P_Channel));

    	/* 7. Initialize channel handle */
    		/*	hRap, hChip, hRegister, hHeap, hInt, eCurOpMode, eMaxOpMode
    			hRapTransCh(!7401) = NULL, 
    			uiXptChannelNo(7401) = INVALID(-1) 
    			bSimulModeConfig = FALSE;
    			eClone = BRAP_P_CloneState_eInvalid;
    			eChannelType = BRAP_P_ChannelType_ePcmPlayback
    			uiChannelNo = uiChannelNo
    		*/
    	hRapCh->hChip 		= hRap->hChip;
    	hRapCh->hRegister 	= hRap->hRegister;
    	hRapCh->hHeap 		= hRap->hHeap;
    	hRapCh->hInt 		= hRap->hInt;
    	hRapCh->hRap 		= hRap;
        hRapCh->ui32FmmBFIntMask = 0;
        hRapCh->bStarted = false;
        
    	/* PCM Playback doesn't need transport channel */
#if BRAP_P_USE_BRAP_TRANS ==1
    	hRapCh->hRapTransCh = NULL;
#else
    	hRapCh->uiXptChannelNo = BRAP_INVALID_VALUE;
#endif

    	hRapCh->eChannelType 		= BRAP_P_ChannelType_ePcmPlayback;
#ifdef BCHP_7411_VER /* 7411 */
    	hRapCh->eMaxOpMode 		= eOutputMode;
    	hRapCh->eCurOpMode 			= eOutputMode;
#endif		
    	hRapCh->eBufDataMode		= pChnSettings->eBufDataMode;
        hRapCh->uiChannelNo 		= uiChannelNo;
    	hRapCh->bSimulModeConfig 	= false;
#ifdef BCHP_7411_VER    
	hRapCh->eClone 				= BRAP_P_CloneState_eInvalid;
#endif

#if BRAP_P_WATERMARK_WORKAROUND==1
	BTMR_GetDefaultTimerSettings(&sTimerSettings);
	sTimerSettings.type = BTMR_Type_eCountDown;
	sTimerSettings.cb_isr = (BTMR_CallbackFunc) BRAP_P_FmmRbufFreeWaterMark_isr;
	sTimerSettings.pParm1 = hRapCh;
	sTimerSettings.parm2 = 0;
	sTimerSettings.exclusive = false;

	ret = BTMR_CreateTimer ( hRap->hTmr, &hRapCh->hTimer, &sTimerSettings);
	if ( ret != BERR_SUCCESS )
	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: Create Timer Failed"));
    		ret = BERR_TRACE(ret);
    		goto end_open;
	}
#endif

#ifndef BCHP_7411_VER /* not for 7411 */
        /* Initialise the data structures for cloned mode */
    	for(eChanPair = BRAP_OutputChannelPair_eLR; eChanPair < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; eChanPair++)
    	{
    	    for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {             
                hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul = false;             
                hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
                hRapCh->sCloneOpPathResc[eChanPair][j].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiMixerId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[eChanPair][j].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;                 
            }
        }
#endif

    	/* 8. Form the resource requirement */
    	/* 		eChannelType = BRAP_P_ChannelType_ePcmPlayback
    	   		uiNumOpPorts = 1 //Max one for stereo 
    	   		bLastChMono = TRUE for left mono and right mono or stereo-interleaved, 
    						  FALSE for true mono and stereo
    			bSimulModePt = false;
    			for(i 0->2) depending upon o/p mode
    				sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX or pChnSettings->eOutputPort
    			bCloneOutput = FALSE;
    	*/	
    	sResrcReq.eChannelType 	= hRapCh->eChannelType;
    	sResrcReq.uiNumOpPorts 	= 1;

    	/* Set bLastChMono to true if single ring buffer is required */
    	if(hRapCh->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
    		sResrcReq.bLastChMono = false;
    	else
    		sResrcReq.bLastChMono = true;

    	sResrcReq.bSimulModePt 	= false; /* Not a simul mode */
        sResrcReq.bAllocateDsp = false;   
        sResrcReq.bAllocateDspCtx = false;   
        sResrcReq.bAllocateRBuf = true;
        sResrcReq.bAllocateSrcCh = true;
        sResrcReq.bAllocateDstCh=false;
        sResrcReq.bAllocatePpm = false;
        sResrcReq.bPassthrough = false;

    	sResrcReq.eBufDataMode = hRapCh->eBufDataMode;

    	for(j = 0; j < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; j++)
        {
            /* Only the 0th index is used for PCM playback and mixing */
            if(j == 0) 
                sResrcReq.sOpPortReq[j].eOpPortType = pChnSettings->eOutputPort;
            else
                sResrcReq.sOpPortReq[j].eOpPortType = (unsigned int)BRAP_RM_P_INVALID_INDEX;

            /* If output port is MAI then initialize the MAI mux selector */			
			if (pChnSettings->eOutputPort == BRAP_OutputPort_eMai )
			{
				sResrcReq.sOpPortReq[j].eMuxSelect = 
					hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
			}
        }

    	/* 9. Call resource manager to give ids of required resources */		
    	/* NOTE: Modification required in BRAP_RM_P_AllocateResources() to return ID of a single 
    			 ring buffer for PCM mono/stereo interleaved type */
    	ret = BRAP_RM_P_AllocateResources( hRap->hRm, &sResrcReq, &(hRapCh->sRsrcGrnt));
    	if(ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_PB_OpenChannel: Resource allocation failed for RAP PB Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret);
            goto free_hChn;
    	}
    	BDBG_MSG(("BRAP_PB_OpenChannel: Resource allocation for PCM PB succeeded"));

    	/* 10. Store DSP(NULL) and FMM handles inside the Audio Channel handle */ 
    	hRapCh->sModuleHandles.hDsp = NULL; /* alternate: hRap->hDsp[hRapCh->sRsrcGrnt.uiDspId] */;
    	hRapCh->sModuleHandles.hFmm = hRap->hFmm[hRapCh->sRsrcGrnt.uiFmmId];
    } /* if !watchdog */
    else
    {
        /* If watchdog recovery */
		hRapCh = *phRapCh;
    }
    
	/* 11. Open all FMM Modules, as per the resources allocated */
    eChanPair = BRAP_OutputChannelPair_eLR;
    if(!BRAP_P_GetWatchdogRecoveryFlag(hRap)) 
    {
		/* 11.1 Do ring buffer settings */
		BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[0]);
		BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[1]);        
		sRbufSettings[0].sExtSettings = pChnSettings->sLtOrSnglRbufSettings;
        sRbufSettings[0].bProgRdWrRBufAddr = true;
        sRbufSettings[1].sExtSettings = pChnSettings->sRtRbufSettings;
        sRbufSettings[1].bProgRdWrRBufAddr = true;
        
		/* 11.2 Do source channel settings: Currently it is blank */
		
		/* 11.3 Do Mixer settings */
        BRAP_MIXER_P_GetDefaultSettings(&sMixerSettings);
		sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiMixerInputId;

        /* Spdif formater settings */
        BRAP_SPDIFFM_P_GetDefaultSettings (&sSpdfFmSettings);
        sSpdfFmSettings.sExtSettings = hRap->sOutputSettings[pChnSettings->eOutputPort].sSpdiffmSettings;

		/* 11.4 Do output port settings */
		switch(pChnSettings->eOutputPort)
		{
			case BRAP_OutputPort_eSpdif:
				sSpdifSettings.sExtSettings = 
					hRap->sOutputSettings[pChnSettings->eOutputPort].uOutputPortSettings.sSpdifSettings;
				pOpSettings = &sSpdifSettings;
				break;
			case BRAP_OutputPort_eI2s0:
			case BRAP_OutputPort_eI2s1:
			case BRAP_OutputPort_eI2s2:
				sI2sSettings.sExtSettings = 
					hRap->sOutputSettings[pChnSettings->eOutputPort].uOutputPortSettings.sI2sSettings;
				pOpSettings = &sI2sSettings;
				break;
			case BRAP_OutputPort_eMai:
				sMaiSettings.sExtSettings = 
					hRap->sOutputSettings[pChnSettings->eOutputPort].uOutputPortSettings.sMaiSettings;
				pOpSettings = &sMaiSettings;
				break;
			case BRAP_OutputPort_eFlex:
				sFlexSettings.sExtSettings = 
					hRap->sOutputSettings[pChnSettings->eOutputPort].uOutputPortSettings.sFlexSettings;
				pOpSettings = &sFlexSettings;
				break;
			case BRAP_OutputPort_eDac0:
			case BRAP_OutputPort_eDac1:
				sDacSettings.sExtSettings = 
					hRap->sOutputSettings[pChnSettings->eOutputPort].uOutputPortSettings.sDacSettings;
				pOpSettings = &sDacSettings;
				break;
           	case BRAP_OutputPort_eRfMod:
           	default:
               	BDBG_ERR(("BRAP_PB_OpenChannel: Output port type %d not supported",pChnSettings->eOutputPort));
               	ret = BERR_TRACE(BERR_NOT_SUPPORTED);
                goto free_rs;
   		}/* switch */
    }/* if !watchdog */
    else
    {
        /* if watchdog recovery */
        sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiMixerInputId;
    }
    
	/* 11.5 Open all the internal modules */
	ret = BRAP_P_OpenOpPathFmmModules( 	hRapCh,
									    &sRbufSettings[0],
										&sSrcChSettings,
										&sMixerSettings,
										&sSpdfFmSettings,
										pOpSettings,
										&(hRapCh->sRsrcGrnt.sOpResrcId[eChanPair]),
										eChanPair,
										&(hRapCh->sModuleHandles));
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("FMM Module Opens failed for RAP PB Channel handle 0x%x", hRapCh));
        ret = BERR_TRACE(ret);
        goto free_rs;
	}

	BDBG_MSG(("BRAP_PB_OpenChannel: Open output FMM modules succeeded"));
	BDBG_MSG(("hRapCh->sModuleHandles.hOp[%d]: %x\n"
		  "\thRapCh->sModuleHandles.hMixer[%d]: %x\n"
		  "\thRapCh->sModuleHandles.hSrcCh[%d]: %x\n"
		  "\thRapCh->sModuleHandles.hSpdifFm[%d]: %x\n"
		  "\thRapCh->sModuleHandles.hRbuf[%d]: %x\n"
		  "\thRapCh->sModuleHandles.hRbuf[%d]: %x\n", 
		  eChanPair, hRapCh->sModuleHandles.hOp[eChanPair],
		  eChanPair, hRapCh->sModuleHandles.hMixer[eChanPair],
		  eChanPair, hRapCh->sModuleHandles.hSrcCh[eChanPair],
		  eChanPair, hRapCh->sModuleHandles.hSpdifFm[eChanPair],
		  2*eChanPair, hRapCh->sModuleHandles.hRBuf[2*eChanPair],
		  2*eChanPair+1, hRapCh->sModuleHandles.hRBuf[2*eChanPair+1]));

#ifndef BCHP_7411_VER
    if (BRAP_P_GetWatchdogRecoveryFlag(hRap))            
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {
            if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == false))
            {                  
                BDBG_MSG (("BRAP_PB_Open: opening cloned port %d", j));
                /* If this is watchdog recovery, re-open the cloned outputs also */
                sMixerSettings.uiMixerInput = hRapCh->sCloneOpPathResc[eChanPair][j].uiMixerInputId;
                sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;

                /* Form BRAP_P_ObjectHandles structure */
                sTempHandles.hRBuf[2*eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[0];
                sTempHandles.hRBuf[2*eChanPair + 1] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[1]; 
                sTempHandles.hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh;
                sTempHandles.hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hMixer;  
                sTempHandles.uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex;  
                sTempHandles.hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSpdifFm;  
                sTempHandles.hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hOp;      

                sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
                ret = BRAP_P_OpenOpPathFmmModules(
                        hRapCh,
                        &sRbufSettings[0],
                        &sSrcChSettings,
                        &sMixerSettings,
                        &sSpdfFmSettings,
                        pOpSettings,
                        &(hRapCh->sCloneOpPathResc[eChanPair][j]),
                        eChanPair,
                        &(sTempHandles)); 
                if(ret != BERR_SUCCESS)
            	{
            		BDBG_ERR(("OpPathFmmModule failed for clone part"));
                    ret = BERR_TRACE(ret);
                    goto free_rs;
            	}
             }
        }
    }
#endif

    ret = BRAP_P_InterruptInstall(hRapCh);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("Interrupt installation failed for RAP Playback Channel handle 0x%x", hRapCh));
        ret = BERR_TRACE(ret);
        goto free_rs;
	}
#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("Interrupt installation failed for RAP Playback Channel handle 0x%x", hRapCh));
        ret = BERR_TRACE(ret);
		goto free_rs;
	}
#endif
#endif
    /* Just for debugging */
    BDBG_ASSERT(ret == BERR_SUCCESS);
    
	goto end_open;

	/* 12. Exit doors */
free_rs:
	BDBG_MSG(("BRAP_PB_OpenChannel: ***** Free RS called ***** "));
	/* Call Resource manager to release these resources */
	ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
	if(ret != BERR_SUCCESS)
    {
		BDBG_ERR(("BRAP_PB_OpenChannel: BRAP_RM_P_FreeResources() for main modules failed."));
    }

free_hChn:
	/* Free up the channel handle */
	BKNI_Free(hRapCh);

end_open:
	BDBG_LEAVE(BRAP_PB_OpenChannel);
	return ret;
}


/***************************************************************************
Summary:
	API used to close a PCM playback channel.
Description:
	It closes the instance of a PCM playback channel operation. It frees the 
	channel handle and resources associated with it, if any.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_OpenChannel
****************************************************************************/
BERR_Code BRAP_PB_CloseChannel( 
	BRAP_ChannelHandle 	hRapCh		/* [in] The RAP Channel handle */
	)
{
	BERR_Code 		ret = BERR_SUCCESS;
#ifndef BCHP_7411_VER /* not for 7411 */
    BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;
    unsigned int    j;
    BRAP_P_ObjectHandles   	sTempHandles;        
    BRAP_RM_P_ResrcGrant 	sResrcGrant;        
#endif    
	
	BDBG_ENTER(BRAP_PB_CloseChannel);

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);

#ifndef BCHP_7411_VER /* not for 7411 */    
    /* Make sure the temp handles etc are clean */
    BKNI_Memset (&sResrcGrant, 0, sizeof(sResrcGrant));
    BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));

    sResrcGrant.uiNumOpPorts = 1;
    for (j = 0; j < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; j++) {    
        sResrcGrant.sOpResrcId[j].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
        sResrcGrant.sOpResrcId[j].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiMixerId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[j].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
    }
    sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
    sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
    sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
    for(j = 0; j < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; j++)
		sResrcGrant.sCapResrcId.uiRbufId[j] = BRAP_RM_P_INVALID_INDEX; 
	
#endif

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
	if(ret != BERR_SUCCESS)
	{
	    BDBG_ERR(("BRAP_PB_CloseChannel: BRAP_P_MaskInterrupt() returned err(%d)",ret));	
    }
#endif    
#else
    if (NULL != hRapCh->hTimer)
    {
        ret = BTMR_DestroyTimer(hRapCh->hTimer);
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_PB_CloseChannel: BTMR_DestroyTimer Failed"));
            ret = BERR_TRACE(ret);
        }
    }

#endif
    BRAP_P_InterruptUnInstall (hRapCh);

	/* 2. Close internal FMM modules */
    
#ifndef BCHP_7411_VER /* not for 7411 */
    /* For cloned ports */
    for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
    {
        if (hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
        {
            /* Form BRAP_P_ObjectHandles structure */
            sTempHandles.hRBuf[2*eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[0];
            sTempHandles.hRBuf[2*eChanPair + 1] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[1]; 
            sTempHandles.hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh;
            sTempHandles.hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hMixer;  
            sTempHandles.uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex;  
            sTempHandles.hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSpdifFm;  
            sTempHandles.hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hOp;                  

            /* Stop cloned modules associated with this output port */
            ret = BRAP_P_CloseOpPathFmmModules (eChanPair, &sTempHandles);
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() for cloned ports failed. Ignoring error!!!!!"));
                ret = BERR_TRACE(ret);
            }

            /* Remove these modules' handle frmo the channel handle */
            hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh = NULL;
            hRapCh->sCloneOpPathHandles[eChanPair][j].hMixer = NULL;  
            hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;  
            hRapCh->sCloneOpPathHandles[eChanPair][j].hSpdifFm = NULL;  
            hRapCh->sCloneOpPathHandles[eChanPair][j].hOp = NULL;          

            /* Free the resources in the RM */
            sResrcGrant.sOpResrcId[eChanPair] = hRapCh->sCloneOpPathResc[eChanPair][j];    
            ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_RemoveOutputPort: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
                ret = BERR_TRACE(ret);
            }    

            /* Clear the Resc Grant struct for this output port!!! */
            hRapCh->sCloneOpPathResc[eChanPair][j].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
            hRapCh->sCloneOpPathResc[eChanPair][j].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiMixerId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
            hRapCh->sCloneOpPathResc[eChanPair][j].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;    
        }        
    }     
#endif  

	ret = BRAP_P_CloseOpPathFmmModules(BRAP_OutputChannelPair_eLR, &(hRapCh->sModuleHandles));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_PB_CloseChannel: call to BRAP_P_CloseOpPathFmmModules() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE(ret);
    }
	BDBG_MSG(("BRAP_PB_CloseChannel: Output FMM modules closed."));
	
	/* 3. Call Resource manager to release these resources */
	ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_PB_CloseChannel: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE(ret);
    }
	BDBG_MSG(("BRAP_PB_CloseChannel: Resources Freed."));

	/* 4. Mark the place for the current channel handle to NULL inside RAP handle */
	hRapCh->hRap->hRapPbCh[hRapCh->uiChannelNo] = NULL;

	/* 5. Free the channel handle */
	BKNI_Free(hRapCh);

	BDBG_LEAVE(BRAP_PB_CloseChannel);
	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	Starts a PCM playback channel.
Description:
	This API is required to start the PCM playback of the selected stream, 
	as specified by input BRAP_PB_AudioParams structure.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_Stop
****************************************************************************/
BERR_Code BRAP_PB_Start ( 
	BRAP_ChannelHandle 		  hRapCh,		/* [in] The RAP Channel handle */
	const BRAP_PB_AudioParams *pAudioParams /* [in] Audio parameters required 
												for starting this channel */
	)
{
	BERR_Code				ret = BERR_SUCCESS;
    BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;
	bool 					bCompress;
	BRAP_RBUF_P_Params		sRBufParams[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
	BRAP_SRCCH_P_Params		sSrcChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	void					*pOpParams = NULL;
	unsigned int 			uiRbufSize[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
    unsigned int            num = 0;
	bool						bIsPbMixedWithCompressed;
    unsigned int            uiStartWRPoint;

#ifndef BCHP_7411_VER  /* only for 7411 */      
    BRAP_P_ObjectHandles        sTempHandles;  
    unsigned int j, k;
#endif
	
	BDBG_ENTER(BRAP_PB_Start);

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);

	if(hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR] == NULL)
	{
		BDBG_ERR(("BRAP_PB_Start: No output port is associated with this playback channel. First add an outpu port"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
    
    if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap)) 
    {
        BDBG_ASSERT(pAudioParams);

    	BDBG_MSG(("BRAP_PB_Start: hRapCh=%d\n"
    				"\tpAudioParams->bLoopAroundEn=%d\n"
    				"\tpAudioParams->uiDelayRbuf=%d\n"
    				"\tpAudioParams->ui32RbufMemSize=%d\n"
    				"\tpAudioParams->eInputBitsPerSample=%d\n"
    				"\tpAudioParams->eInputSamplingRate=%d\n",
    				hRapCh,	pAudioParams->bLoopAroundEn,
    				pAudioParams->uiDelayRbuf,
    				pAudioParams->ui32RbufMemSize,
    				pAudioParams->eInputBitsPerSample,
    				pAudioParams->eInputSamplingRate));

#if BRAP_UNSIGNED_PCM_SUPPORTED
            BDBG_MSG(("\n\tpAudioParams->bIsSigned",pAudioParams->bIsSigned));
#endif

    	/* If channel already started return an error */
    	if (hRapCh->bStarted) 
        {
    		BDBG_ERR(("Channel already started. Exiting BRAP_PB_Start."));
    		return BERR_SUCCESS;
    	}
    	else
	    {
    		hRapCh->bStarted = true;        
	    }
        
        /* Validate compatibility of eBufDataMode passed during channel open and start */
        if((hRapCh->eBufDataMode == BRAP_BufDataMode_eMono ||
            hRapCh->eBufDataMode == BRAP_BufDataMode_eStereoInterleaved) &&
            (pAudioParams->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)) 
        {
    		BDBG_ERR(("BRAP_PB_Start: eBufDataMode(%d) passed requires 2 rbuf which "
                      "is incompatible with eBufDataMode(%d) passed during channel open",
                       pAudioParams->eBufDataMode, hRapCh->eBufDataMode));
    		return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

       	/* Validate if ePcmOutputMode and eBufDataMode compatible */
    	if(bValidatePcmOpNDataModes[pAudioParams->ePcmOutputMode][pAudioParams->eBufDataMode] == false)
    	{
    		BDBG_ERR(("BRAP_PB_Start: ePcmOutputMode(%d) is not compatible with eBufDataMode(%d)",
    			pAudioParams->ePcmOutputMode, pAudioParams->eBufDataMode));
    		return BERR_TRACE(BERR_NOT_SUPPORTED);
    	}

	/*Checking if the Play Back Channel data is being tried to mix with compressed data*/
	ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
												hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort,
												false, /*As Capture channel can have only PCM data*/
												&bIsPbMixedWithCompressed);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
		return BERR_TRACE (ret);
	}

	if(bIsPbMixedWithCompressed)
	{
		ret = BERR_NOT_SUPPORTED;
		BDBG_ERR(("BRAP_PB_Start: Trying to mix PCM data with Compressed Data"));
		goto end_start;
	}
        
    	/* RBuf parameters */
        uiRbufSize[0] = pAudioParams->ui32RbufMemSize;
      	
        ret = BRAP_P_ConvertDelayToStartWRPoint(pAudioParams->uiDelayRbuf,
        								pAudioParams->eBufDataMode,
        								pAudioParams->eInputSamplingRate,
                                        pAudioParams->eInputBitsPerSample,
                                        &uiStartWRPoint);

        if(BERR_SUCCESS != ret)
        {
            BDBG_ERR(("BRAP_PB_Start: BRAP_P_ConvertDelayToStartWRPoint returned %d",ret));
            return BERR_TRACE(ret);
        }
        sRBufParams[0].uiStartWRPoint = uiStartWRPoint;
	sRBufParams[0].uiStartSize = pAudioParams->ui32RbufMemSize;
        
        if(pAudioParams->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
        {
            /* Stereo non-interleaved requires two separate buffers with same size and delay */       
            uiRbufSize[1] = pAudioParams->ui32RbufMemSize;

            /* Both the StartWRPoints should be same */
           	sRBufParams[1].uiStartWRPoint = sRBufParams[0].uiStartWRPoint;
		sRBufParams[1].uiStartSize = sRBufParams[0].uiStartSize;
        }
        else
        {
            /* Stereo interleaved or mono requires single left buffer. So 0 size and 0 delay for other */       
        	uiRbufSize[1] = 0;
           	sRBufParams[1].uiStartWRPoint = 0;
		sRBufParams[1].uiStartSize = 0;
        }

    	/* 1.1 Check for supported output bit resolutions. */
    	/* For I2S and Flex output, it can be between 16 bits/sample to 32 bits/sample,
    	   For other outputs, it can be between 16 bits/sample to 24 bits/sample */
     	if((pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_I2S_MAX_BITS_PER_SAMPLE) ||
    	   (pAudioParams->sAudioOutputParams.uiOutputBitsPerSample < BRAP_P_MIN_BITS_PER_SAMPLE))
    	{
    		BDBG_ERR(("BRAP_PB_Start: Output bit resolution %d should be between 16 to 32 bits/sample", 
                       pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
    		return BERR_TRACE(BERR_NOT_SUPPORTED);
    	}
    	else if(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_MIXER_MAX_BITS_PER_SAMPLE)
    	{
    		if((hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort != BRAP_OutputPort_eI2s0) &&
    		   (hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort != BRAP_OutputPort_eI2s1) &&
    		   (hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort != BRAP_OutputPort_eI2s2) &&
    		   (hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort != BRAP_OutputPort_eFlex))
    		{
    			/* Only I2S and Flex can have more than 24 bits/sample output.
    			   So for other outputs return error */
    			BDBG_ERR(("BRAP_PB_Start: Output bit resolution %d should be between 16 to 24 bits/sample", 
                              pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
    			return BERR_TRACE(BERR_NOT_SUPPORTED);
    		}
       	}

    	/* 1.2 Check if buffer size provided during starting the channel is less than or equal 
    		   to that given during channel open.
    		   Also, check if delay-in-bytes is with in the ring buffer boundary */ 
    	for(num=0; num < BRAP_RM_P_MAX_RBUFS_PER_SRCCH; num++)
    	{
    	    /* since this is a PB channel and can carry only one channel pair,
    	    only hRbuf[0] and hRbuf[1] can be populated.*/    	    
    		if(hRapCh->sModuleHandles.hRBuf[num] != NULL)
    		{
    			if(uiRbufSize[num] > hRapCh->sModuleHandles.hRBuf[num]->sSettings.sExtSettings.uiSize)
    			{
    				BDBG_ERR(("BRAP_PB_Start: Ring buffer[%d] size(%d) should be less than %d", 
           				eChanPair, uiRbufSize[num],
           				hRapCh->sModuleHandles.hRBuf[num]->sSettings.sExtSettings.uiSize));
    		
    				return BERR_TRACE(BERR_INVALID_PARAMETER);
    			}

    			if(sRBufParams[num].uiStartWRPoint >= uiRbufSize[num])
    			{
    				BDBG_ERR(("BRAP_PB_Start: Ring buffer[%d] start point(%d) should be less than %d", 
           				eChanPair, sRBufParams[num].uiStartWRPoint,
           				hRapCh->sModuleHandles.hRBuf[num]->sSettings.sExtSettings.uiSize));
    				return BERR_TRACE(BERR_NOT_SUPPORTED);
    			}
    		}
    	}/* for */
        
    	/* 2. Prepare parameters required by BRAP_P_StartOpPathFmmModules() */
    	/* 2.1 SPDIF formatter Parameters */
    	/* Get the default parameters first */
    	ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
    	if(ret != BERR_SUCCESS) 
    	{
    		BDBG_ERR(("BRAP_PB_Start: BRAP_SPDIFFM_P_GetDefaultParams returned error(%d)",ret));
            ret = BERR_TRACE(ret);
            goto end_start;
    	} 

    	/* PCM data is always un-compressed */
    	bCompress = false; 
    	
    	/* Fill up the required parameters */
    	sSpdifFmParams.sExtParams = pAudioParams->sAudioOutputParams.sSpdifFmParams;
    	sSpdifFmParams.bCompressed = bCompress; 
       sSpdifFmParams.sChanStatusParams = pAudioParams->sSpdifChanStatusParams;      


    	/* 2.2 Mixer parameters */
    	/* Get the default parameters first */
    	ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
    	if(ret != BERR_SUCCESS) 
    	{
    		BDBG_ERR(("BRAP_PB_Start: BRAP_MIXER_P_GetDefaultParams returned error(%d)",ret));
            ret = BERR_TRACE(ret);
            goto end_start;
    	} 
    	
    	/* Fill up the required parameters */
    	sMixerParams.sExtParams = pAudioParams->sAudioOutputParams.sMixerParams;
    	sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId;
    	sMixerParams.bCompress = bCompress;
    	/* Mixer output stream resolution can maximum be 24 bits/sample. 
    	   If output stream resolution required is more than 24 bits/sample, 
    	   set mixer output stream resolution to be 24 bits/sample and output has 
    	   to pad the remaining bits. Only for I2S output more than 24 bits/sample 
    		is supported. */
        if(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > 
    			BRAP_P_MIXER_MAX_BITS_PER_SAMPLE)
    		sMixerParams.uiStreamRes = BRAP_P_MIXER_MAX_BITS_PER_SAMPLE;
    	else
    		sMixerParams.uiStreamRes = pAudioParams->sAudioOutputParams.uiOutputBitsPerSample;
    		
    	/* 2.4 Source channel parameters */
    	/* Get the default parameters first */
    	ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
    	if(ret != BERR_SUCCESS) 
    	{
    		BDBG_ERR(("BRAP_PB_Start: BRAP_SRCCH_P_GetDefaultParams returned error(%d)",ret));
            ret = BERR_TRACE(ret);
            goto end_start;
    	} 
    	
    	/* Fill up the required parameters: 
    		bInputSrValid, eBufDataMode, bCompress, eInputSR, eOutputSR */
    	sSrcChParams.bCompress = bCompress;
    	sSrcChParams.bInputSrValid = false;  /* Input audio data is not coming from DSP */
    	sSrcChParams.eInputBitsPerSample = pAudioParams->eInputBitsPerSample;
    	sSrcChParams.eBufDataMode = pAudioParams->eBufDataMode;
    	sSrcChParams.eLRDataCntl = lrDataCtrl[pAudioParams->ePcmOutputMode];
    	sSrcChParams.bNotPauseWhenEmpty = pAudioParams->bLoopAroundEn;

        /* Normal playback path: data is not written to srcFIFO directly */
        sSrcChParams.bCapDirect2SrcFIFO = false;

    	/* Set the start selection bit, if user has requested for a delay */
    	if(pAudioParams->uiDelayRbuf)
    		sSrcChParams.bStartSelection = true;
    	else
    		sSrcChParams.bStartSelection = false;

#if BRAP_UNSIGNED_PCM_SUPPORTED
            sSrcChParams.bIsSigned = pAudioParams->bIsSigned ;
#endif

		/* Input sampling rate is provided by the user */
		sSrcChParams.eInputSR = pAudioParams->eInputSamplingRate;
		if(hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate != 
					BAVC_AudioSamplingRate_eUnknown)
		{
			/* OutputSR: If already set (may be by DSP), use it. */
			sSrcChParams.eOutputSR = hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate;
		}
		else
		{
			/* OutputSR: Else, use the inputSR as outputSR */
			sSrcChParams.eOutputSR = sSrcChParams.eInputSR;
			hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate = sSrcChParams.eOutputSR; 
		}

		sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiRbufId[0];
		sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiRbufId[1];
		
		/* 2.5 Output port parameters */
        sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiSrcChId;

		/* Output block params */
        ret = BRAP_P_FormOpParams (hRapCh, 
                hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort, 
                hRapCh->hRap->sOutputSettings[hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort].eOutputTimebase,
                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                &pOpParams);

        if (ret!=BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto end_start;
        }

		/* Should Mute Mixer ouput before starting Output block, MS and Mixer.
		But since Mixer hasnt been started yet, cant do so here. Instead, we
		make sure that when Mixer is started, it is Muted by default */

		/* For debug purpose */
		BDBG_ASSERT(hRapCh->sModuleHandles.hSrcCh[eChanPair]);
        BDBG_ASSERT(hRapCh->sModuleHandles.hOp[eChanPair]);
		BDBG_ASSERT(hRapCh->sModuleHandles.hMixer[eChanPair]);
		BDBG_ASSERT(hRapCh->sModuleHandles.hRBuf[2*eChanPair] || hRapCh->sModuleHandles.hRBuf[2*eChanPair+1]);
		if((hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort == BRAP_OutputPort_eSpdif) 
			&& (hRapCh->sModuleHandles.hSpdifFm[eChanPair] == NULL))
		{
    		BDBG_ERR(("BRAP_PB_Start: hSpdifFm is NULL"));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end_start;
		}
        if((hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort == BRAP_OutputPort_eMai) && 
		   (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector 
						== BRAP_OutputPort_eSpdif) && 
		   (hRapCh->sModuleHandles.hSpdifFm[eChanPair] == NULL))
		{
    		BDBG_ERR(("BRAP_PB_Start: For MAI, hSpdifFm is NULL"));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto end_start;
		}
    }/* if !watchdog */
    else
    {
#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 1)
        /* Unmask the Free Mark interrupt */
        ret = BRAP_P_UnmaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_PB_Start: BRAP_P_UnmaskInterrupt() returned err(%d)",ret));	
        }
#if ( BCHP_CHIP == 7400 )		
	BRAP_P_ReArmSrcChInterrupt_isr(hRapCh);
#endif
#endif
#endif
    }
    
	/* 2.6 Start the internal FMM modules -> BRAP_P_StartOpPathFmmModules() */
	ret = BRAP_P_StartOpPathFmmModules(
						hRapCh,
						&sRBufParams[0],
						&sSrcChParams,
						&sMixerParams,
						&sSpdifFmParams,
						pOpParams,
						eChanPair,
						&(hRapCh->sModuleHandles));
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_Start: BRAP_P_StartOpPathFmmModules() returned err(%d)",ret));
        ret = BERR_TRACE(ret);
        goto end_start;
	}
	BDBG_MSG(("BRAP_PB_Start: Output path FMM modules started!"));

    /* Set output sampling rate and program SRC */
    ret = BRAP_OP_P_SetSamplingRate ( 
                        hRapCh->hRap,
                        hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort,
                        hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate,/* output SR */ 
    	                true, /* bOverride*/
      	                false /* bCalledFromISR */);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_Start: BRAP_OP_P_SetSamplingRate() returned err(%d)",ret));
        ret = BERR_TRACE(ret);
        goto end_start;
	}
	BDBG_MSG(("BRAP_PB_Start: SetSamplingRate done!"));

#ifndef BCHP_7411_VER /* not for 7411 */
    /* Start all Cloned ports associated with this channel. note: there's only 
    1 channel pair */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {    
            if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == false))
            {      
                BDBG_MSG(("BRAP_PB_Start: output port %d was cloned for channel pair %d", j, eChanPair));
                if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap)) 
                {                      
                    /* Output block params */

                    ret = BRAP_P_FormOpParams (hRapCh, 
                            (BRAP_OutputPort)j, 
                            hRapCh->hRap->sOutputSettings[j].eOutputTimebase,
                            pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                            &pOpParams);

                    if (ret!=BERR_SUCCESS)
                    {
                        ret = BERR_TRACE(ret);
                        goto end_start;
                    }
                    sSpdifFmParams.sChanStatusParams = pAudioParams->sSpdifChanStatusParams;                            

                    /* TODO: Handle SRC programming for 7400 where o/p port from one DP is 
                             cloned to o/p port of another DP */
                    
                    /* ReUse the same SpdifFmParams as filled in for the main port above */
                    /* ReUse the same MixerParams as filled in for the main port above, except SrcChId */
                    if (hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh != NULL)
                    {
                        /* cloned port uses another SrcCh */
                        /* take the same Params as the orig src ch */
                        sSrcChParams = hRapCh->sModuleHandles.hSrcCh[eChanPair]->sParams;

                        sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[0];
                        sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[1];

                        /* this Src Ch shares the rbufs with the Orig SrCh */
                        sSrcChParams.bSharedRbuf = true;
                        sSrcChParams.uiMasterSrcCh = 
                                        hRapCh->sModuleHandles.hSrcCh[eChanPair]->uiIndex;

                        sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[eChanPair][j].uiSrcChId ;     
                        BDBG_MSG(("BRAP_PB_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", 
                            j, sMixerParams.uiSrcChId, sSrcChParams.uiMasterSrcCh));                       
                    }
                    else
                    {
                        sMixerParams.uiSrcChId = hRapCh->sModuleHandles.hSrcCh[eChanPair]->uiIndex; 

		            for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
		            {    
		                if (hRapCh->sCloneOpPathHandles[eChanPair][k].hSrcCh != NULL)
		                { 
		                    break;
		                }
		            }
		            if (k!=BRAP_RM_P_MAX_OUTPUTS)
		            {
		                /* One of the cloned ports has a specially allocated hSrcCh
		                => one port was cloned in another DP, say DPx */
		                /* if eOutputPort belongs to DPx, it should use 
		                hSrcCh from sCloneOpPathHandles, else from sModuleHandles */
		                if (hRapCh->sCloneOpPathResc[eChanPair][j].uiDataPathId
		                        == hRapCh->sCloneOpPathResc[eChanPair][k].uiDataPathId)

		                {
		                    sMixerParams.uiSrcChId = hRapCh->sCloneOpPathHandles[eChanPair][k].hSrcCh->uiIndex;
		                }
		            }
						
                        BDBG_MSG(("BRAP_PB_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
                    }
                }

                /* Form BRAP_P_ObjectHandles structure */
                sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
                sTempHandles.hRBuf[2*eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[0];
                sTempHandles.hRBuf[2*eChanPair + 1] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[1];
                sTempHandles.hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh;
                sTempHandles.hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hMixer;  
                sTempHandles.uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex;  
                sTempHandles.hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSpdifFm;  
                sTempHandles.hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hOp;                 

        		/*Checking if the Play Back Channel data is being tried to mix with compressed data*/
        		ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
        													(BRAP_OutputPort)j,
        													false, /*As Capture channel can have only PCM data*/
        													&bIsPbMixedWithCompressed);
        		if(ret != BERR_SUCCESS)
        		{
        			BDBG_ERR(("BRAP_PB_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
        			return BERR_TRACE (ret);
        		}

        		if(bIsPbMixedWithCompressed)
        		{
        			ret = BERR_NOT_SUPPORTED;
        			BDBG_ERR(("BRAP_PB_Start: Trying to mix PCM data with Compressed Data"));
        			goto end_start;
        		}

                BDBG_MSG (("BRAP_PB_Start: temp handle formed.")); 
                /* Start the internal FMM modules */
                ret = BRAP_P_StartOpPathFmmModules(
                                hRapCh,
                                &sRBufParams[0],
                                &sSrcChParams,
                                &sMixerParams,
                                &sSpdifFmParams,
                                pOpParams,
                                eChanPair,
                                &(sTempHandles));
                if(ret != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_PB_Start: StartOpPathFmmMod returned err = %d",ret));
                    ret = BERR_TRACE(ret);
                    goto end_start;
                }

                BDBG_MSG(("BRAP_PB_Start: Cloned resources started"));

                 /* Set output sampling rate and program SRC for cloned port*/
                 ret = BRAP_OP_P_SetSamplingRate ( 
                                                    hRapCh->hRap,
                                                    (BRAP_OutputPort)j,
                                                    hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate,/* output SR */ 
                                	                true, /* bOverride*/
                                  	                false /* bCalledFromISR */);
            	if(ret != BERR_SUCCESS)
            	{
            		BDBG_ERR(("BRAP_PB_Start: BRAP_OP_P_SetSamplingRate() returned err(%d)",ret));
                    ret = BERR_TRACE(ret);
                    goto end_start;
            	}
            	BDBG_MSG(("BRAP_PB_Start: SetSamplingRate done!"));
            }/* if hOp != NULL */            
        }/* for j */   
#endif /* not for 7411 */

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Unmask the Free Mark interrupt */
    ret = BRAP_P_UnmaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
	if(ret != BERR_SUCCESS)
	{
	    BDBG_ERR(("BRAP_PB_Start: BRAP_P_UnmaskInterrupt() returned err(%d)",ret));	
    }
#else
#if ( BCHP_CHIP == 7400 )
	BRAP_P_ReArmSrcChInterrupt_isr(hRapCh);
#endif
#endif
#else
    ret = BTMR_StartTimer(hRapCh->hTimer, 1000);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_Start: BTMR_StartTimer() returned err(%d)",ret));
		ret = BERR_TRACE(ret);
		goto end_start;
	}
	BDBG_MSG(("BRAP_PB_Start: Start timer done!"));
#endif

end_start:
/* TODO: stop resources if any were started*/
	if(ret != BERR_SUCCESS)
	{
		/* Returning on error. Restore back channel state. */
		hRapCh->bStarted = false;
    }
    
	BDBG_LEAVE(BRAP_PB_Start);
	return ret;
}


/***************************************************************************
Summary:
	Stops a PCM playback channel.
Description:
	This API is required to stop the PCM playback channel.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_Start
****************************************************************************/
BERR_Code BRAP_PB_Stop ( 
	BRAP_ChannelHandle 	hRapCh /* [in] The RAP Decode Channel handle */
	)
{
	BERR_Code		ret = BERR_SUCCESS;
    BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;    
#ifndef BCHP_7411_VER /* not for 7411 */   
    unsigned int j;
    BRAP_P_ObjectHandles    	sTempHandles;       
#endif

	BDBG_ENTER(BRAP_PB_Stop);

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);
    
	if (hRapCh->bStarted == false) {
		BDBG_ERR(("Channel is already stopped. Exiting BRAP_PB_Stop without taking any action."));
		return BERR_SUCCESS;
	}

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_Stop: BRAP_P_MaskInterrupt() returned err(%d)",ret));	
	}
#endif
#endif

#if BRAP_P_WATERMARK_WORKAROUND==1
	ret = BTMR_StopTimer(hRapCh->hTimer);
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_Stop: BTMR_StopTimer() returned err(%d)",ret));	
		ret = BERR_TRACE(ret);
		return ( ret );
	}
#endif

#ifndef BCHP_7411_VER /* not for 7411 */    
    BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));
	/* 3. Stop the cloned FMM modules associated with the channel. note: 
	there's only 1 channel pair for PB channels */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {
            if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == false))

                {

                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[0];
                    sTempHandles.hRBuf[2*eChanPair + 1] = hRapCh->sCloneOpPathHandles[eChanPair][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh;
                    sTempHandles.hMixer[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hSpdifFm;  
                    sTempHandles.hOp[eChanPair] = hRapCh->sCloneOpPathHandles[eChanPair][j].hOp;                  

                    /* Stop cloned modules associated with this output port */
                    ret = BRAP_P_StopOpPathFmmModules (eChanPair, &sTempHandles);
                    if (ret != BERR_SUCCESS)
                    {
                        BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() for cloned ports failed. Ignoring error!!!!!"));
                        ret = BERR_TRACE(ret);
                    }
               }
         }

#endif    
	
	/* 3. Stop the main FMM modules associated with the channel */
	ret = BRAP_P_StopOpPathFmmModules(eChanPair, &(hRapCh->sModuleHandles));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_PB_Stop: call to BRAP_P_StopOpPathFmmModules() failed. Ignoring error!!!!!"));
    }
	BDBG_MSG(("BRAP_PB_Stop: Output path FMM modules stoped!"));

	hRapCh->bStarted = false;
    
	BDBG_LEAVE(BRAP_PB_Stop);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:	
	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_UpdateBufUsg
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_PB_GetBufInfo);

    /* Pre-fill with invalid values */
    psPcmBufInfo->sRtBufInfo.pBasePtr = NULL;
    psPcmBufInfo->sRtBufInfo.pEndPtr = NULL;
    psPcmBufInfo->sRtBufInfo.pReadPtr = NULL;
    psPcmBufInfo->sRtBufInfo.pWritePtr = NULL;
    psPcmBufInfo->sRtBufInfo.uiContiguousSize = 0;
    psPcmBufInfo->sRtBufInfo.uiTotalSize = 0;

    psPcmBufInfo->sLtOrSnglBufInfo.pBasePtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pEndPtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pReadPtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.pWritePtr = NULL;
    psPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize = 0;
    psPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize = 0;

    BKNI_EnterCriticalSection();
    ret = BRAP_PB_GetBufInfo_isr(hRapCh, psPcmBufInfo);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PB_GetBufInfo);
    return ret;
}


/***************************************************************************
Summary:	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. This is an 
	ISR version of BRAP_PB_GetBufInfo(). If ring buffer is written with PCM
    data from an ISR context, this API should be used to get the rbuf info.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
****************************************************************************/
BERR_Code BRAP_PB_GetBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo 			*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtRBufId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
		
	BDBG_ENTER(BRAP_PB_GetBufInfo_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufInfo);

	BKNI_Memset(psPcmBufInfo, 0, sizeof(BRAP_PcmBufInfo));

	BDBG_MSG(("BRAP_PB_GetBufInfo_isr: hRapCh=%d psPcmBufInfo=%d",hRapCh, psPcmBufInfo));
	
	/* Get the valid rbuf ids */
	uiLtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0]; 
	uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1]; 	

	if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_PrepBufInfo_isr(hRapCh, 0/* uiLtRBufId*/, &(psPcmBufInfo->sLtOrSnglBufInfo));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Couldn't get left ring buffer[id - %d]info",
				uiLtRBufId));
            ret = BERR_TRACE(ret);
            return ret;
		}
	}
    else
    {
		BDBG_ERR(("BRAP_PB_GetBufInfo_isr: left ring buffer id(%d) is INVALID", uiLtRBufId));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

	if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_PrepBufInfo_isr(hRapCh, 1/* uiRtRBufId*/, &(psPcmBufInfo->sRtBufInfo));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_GetBufInfo_isr: Couldn't get right ring buffer[id - %d]info",
				uiRtRBufId));			
            ret = BERR_TRACE(ret);
            return ret;
		}
	}

	BDBG_MSG(("BRAP_PB_GetBufInfo_isr: OutParam:\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pBasePtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pEndPtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.pWritePtr=%p\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize=%d\n"
			"\tpsPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize=%d\n"
			"\tpsPcmBufInfo->sRtBufInfo.pBasePtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.pEndPtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.pWritePtr=%p\n"
			"\tpsPcmBufInfo->sRtBufInfo.uiContiguousSize=%d\n"
			"\tpsPcmBufInfo->sRtBufInfo.uiTotalSize=%d\n",
			psPcmBufInfo->sLtOrSnglBufInfo.pBasePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pEndPtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pWritePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize,
			psPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize,
			psPcmBufInfo->sRtBufInfo.pBasePtr,
			psPcmBufInfo->sRtBufInfo.pEndPtr,
			psPcmBufInfo->sRtBufInfo.pWritePtr,
			psPcmBufInfo->sRtBufInfo.uiContiguousSize,
			psPcmBufInfo->sRtBufInfo.uiTotalSize));
			
	BDBG_LEAVE(BRAP_PB_GetBufInfo_isr);
	return ret;
}

/***************************************************************************
Summary:	
	Updates the write pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data written in the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_GetBufInfo
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_PB_UpdateBufUsg);

    BKNI_EnterCriticalSection();
    ret = BRAP_PB_UpdateBufUsg_isr(hRapCh, psPcmBufSzUpdt);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_PB_UpdateBufUsg);
    return ret;
}

/***************************************************************************
Summary:	Updates the write pointers for the associated ring buffers as 
    per the ring buffer usage provided.
Description:
	This API updates the write pointer associated to the ring-buffer as per 
	the information related to the data rwritten to the ring buffer, which 
	is provided by the caller through the structure BRAP_PcmBufSzUpdt.
	This is an ISR version of BRAP_PB_UpdateBufUsg(). If ring buffer is 
	written with PCM data from an ISR context, this API should be used to 
	update the ring buffer usage info.
Returns:
	Returns error on failure or BERR_SUCCESS on success
See Also:
	
****************************************************************************/
BERR_Code BRAP_PB_UpdateBufUsg_isr( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtRBufId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
		
	BDBG_ENTER(BRAP_PB_UpdateBufUsg_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufSzUpdt);

#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("BRAP_PB_UpdateBufUsg_isr: hRapCh=%d"
			  	"\tpsPcmBufSzUpdt->uiLtOrSnglBufSzUpdt=%d"
			  	"\tpsPcmBufSzUpdt->uiRtBufSzUpdt=%d",hRapCh,
			  	psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt,
			  	psPcmBufSzUpdt->uiRtBufSzUpdt));
#endif

	/* Get the valid rbuf ids */
	uiLtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0]; 
	uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1]; 	

	if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_WriteBufUsg_isr(hRapCh, 0 , psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't update left ring buffer[0] usage"));			
			return BERR_TRACE(ret);	
		}
	}
    else
    {
		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: left ring buffer id(%d) is INVALID", uiLtRBufId));			
		return BERR_TRACE(ret);	
    }        
        
	if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_PB_P_WriteBufUsg_isr(hRapCh, 1, psPcmBufSzUpdt->uiRtBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't update right ring buffer[1] usage"));			
			return BERR_TRACE(ret);	
		}
	}

    /* Unmask the free mark interrupt */
#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    ret = BRAP_P_UnmaskInterrupt_isr(hRapCh, BRAP_Interrupt_eFmmRbufFreeByte);
    if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_PB_UpdateBufUsg_isr: Couldn't unmask eFmmRbufFreeByte interrupt"));			
		return BERR_TRACE(ret);	
	}
#else
#if ( BCHP_CHIP == 7400 )
	BRAP_P_ReArmSrcChInterrupt_isr(hRapCh);
#endif
#endif    
#endif

	BDBG_LEAVE(BRAP_PB_UpdateBufUsg_isr);
	return ret;
}


static BERR_Code BRAP_PB_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 			uiRBufId,
	BRAP_RingBufInfo		*pRBufInfo
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32RdAddr = 0;
	uint32_t ui32WrAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;
    uint32_t ui32EndAddrOffset = 0;
	unsigned int uiRBufSize = 0;
	unsigned int uiFrameSize = 0,i=0;
    bool    bLoopAroundEn = false;
#if (BRAP_P_EDGE_TRIG_INTRPT != 0)    
    unsigned int uiGap = 256;
#endif
	BDBG_ENTER(BRAP_PB_P_PrepBufInfo_isr);

	/* Get the ring buffer handle */
	hRBuf = hRapCh->sModuleHandles.hRBuf[uiRBufId];
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: hRBuf[%d] is NULL", uiRBufId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));

    /* Get the end address */
    ui32EndAddrOffset = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRBuf->ui32Offset));
    	
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */
	/* Get current write address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset));

    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
				SOURCE_RING_WRADDR);	

	/* Get current read address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset));

    ui32RdAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
				SRING_RDADDR);	

#else
	/* Get current write address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset));

    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
				RINGBUF_WRADDR);	

	/* Get current read address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset));

    ui32RdAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
				AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
				RINGBUF_RDADDR);	

#endif /* 7411 */


	if ( hRapCh->bStarted == true )
	{
		
		uiRBufSize = hRBuf->sParams.uiStartSize;
	}
	else
	{
		/* If it is not started use the open time size */
		uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;
	}

	/* Each Ring buffer is logically divided into 4 frames. This is also the max free
   	   contiguous ring buffer size in bytes that is informed to the user */
    BRAP_RBUF_P_GetFrameSize(uiRBufSize, &uiFrameSize);
		
	pRBufInfo->pBasePtr = (hRBuf->sSettings.sExtSettings.pBufferStart);
    err = BRAP_ConvertOffsetToAddress(hRBuf->hHeap, ui32EndAddrOffset, (void **)&(pRBufInfo->pEndPtr));
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: BRAP_ConvertOffsetToAddress returned err(%d)", err));
        return BERR_TRACE(err);
    }

#ifndef BCHP_7411_VER /* not for 7411 */
    err = BRAP_ConvertOffsetToAddress(hRBuf->hHeap, ui32WrAddr, (void **)&(pRBufInfo->pWritePtr));
    if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_P_PrepBufInfo_isr: BMEM_ConvertOffsetToAddress returned err(%d)", err));
        return BERR_TRACE(err);
    }
#else
    pRBufInfo->pWritePtr = (void *)ui32WrAddr;
#endif

	if(ui32WrAddr >= ui32RdAddr)
	{
		/* tot free size = rbuf size - (wrPtr - rdPtr) */ 
		pRBufInfo->uiTotalSize = uiRBufSize - (ui32WrAddr - ui32RdAddr);

		/* cont free size = endPtr - wrPtr */
		pRBufInfo->uiContiguousSize = (ui32BaseAddr + uiRBufSize) - ui32WrAddr; 	 

		/* Clip cont free size if more than tot free size */
		if(pRBufInfo->uiContiguousSize > pRBufInfo->uiTotalSize)
			pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
 	}
	else
	{
		/* cont free size = tot free size = rdPtr - wrPtr */ 
		pRBufInfo->uiTotalSize = ui32RdAddr - ui32WrAddr;
		pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
	}

#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("Before FRAME logic W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));
#endif

    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS ; i ++)
    {
        if (hRapCh->sModuleHandles.hSrcCh[i] !=NULL)
        {
            bLoopAroundEn = hRapCh->sModuleHandles.hSrcCh[i]->sParams.bNotPauseWhenEmpty;
        }
    }

#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
	/* Return contiguous size limited to a single frame */
	if(pRBufInfo->uiContiguousSize > uiFrameSize)
	{
		pRBufInfo->uiContiguousSize = uiFrameSize;
	}
	else
	{
    	if(pRBufInfo->uiTotalSize <= uiFrameSize)
		{
			pRBufInfo->uiContiguousSize = 0;
		}
	}
#else
	/* Return contiguous size keeping some gap between R-W ptrs */
   	if(pRBufInfo->uiTotalSize == pRBufInfo->uiContiguousSize)
    {
        if(hRapCh->bStarted == true)
        {
            if(bLoopAroundEn == false)
            {    
            	if(pRBufInfo->uiContiguousSize > uiGap)
            	{
                    pRBufInfo->uiContiguousSize -= uiGap;
            	}
            	else
            	{
                	pRBufInfo->uiContiguousSize = 0;
                }    
            }
        }

    }
    else if(pRBufInfo->uiTotalSize > pRBufInfo->uiContiguousSize)
    {
        /* The complete contiguous size can be returned to the app */
    }        
    else
    {
        /* This case should never occur */
        BDBG_ERR(("T(0x%x) < C(0x%x)",pRBufInfo->uiTotalSize,pRBufInfo->uiContiguousSize));
        BDBG_ASSERT(0);
    }

#endif

#if BRAP_PB_P_DEBUG_PCM
	BDBG_ERR(("W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));
#endif

	BDBG_LEAVE(BRAP_PB_P_PrepBufInfo_isr);
	return BERR_SUCCESS;
}

static BERR_Code BRAP_PB_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	unsigned int		uiSzUpdt
	)
{
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32WrAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;	
	unsigned int uiRBufSize = 0;
	uint32_t ui32NewWrAddr = 0;
	uint32_t ui32WrapAroundBit = 0;
	
	BDBG_ENTER(BRAP_PB_P_WriteBufUsg);
#if BRAP_PB_P_DEBUG_PCM    
	BDBG_MSG(("BRAP_PB_P_WriteBufUsg_isr: hRapCh=%d, uiRBufI=%d, uiSzUpdt=%d",
		hRapCh, uiRBufId, uiSzUpdt));
#endif

	/* Get the ring buffer handle */
	hRBuf = hRapCh->sModuleHandles.hRBuf[uiRBufId];
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_PB_P_WriteBufUsg_isr: hRBuf[%d] is NULL", uiRBufId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));

	/* Get current write address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset));

#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */ 
    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						SOURCE_RING_WRADDR);

	ui32WrapAroundBit = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						SOURCE_RING_WRADDR_WRAP);
#else
    ui32WrAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						RINGBUF_WRADDR);	

	ui32WrapAroundBit = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR,
						RINGBUF_WRADDR_WRAP);
#endif /* 7411 */

#if BRAP_PB_P_DEBUG_PCM
	BDBG_MSG(("uiSzUpdt(0x%x) ui32WrAddr(0x%x) ui32BaseAddr(0x%x) uiRBufSize(0x%x)",
			uiSzUpdt , ui32WrAddr, ui32BaseAddr, uiRBufSize));
#endif

	/* Check if the uiSzUpdt is within the ring buffer */
       if ( hRapCh->bStarted == true )
       {
		uiRBufSize = hRBuf->sParams.uiStartSize;       
       }
	else
	{
		uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;
	}
	
	if(uiSzUpdt + ui32WrAddr > ui32BaseAddr + uiRBufSize)
	{
		BDBG_ERR(("BRAP_PB_P_WriteBufUsg_isr: uiUsedSize exceeds ring buffer boundary"));
		BDBG_ERR(("uiSzUpdt(0x%x) + ui32WrAddr(0x%x) > ui32BaseAddr(0x%x) + uiRBufSize(0x%x)",
			uiSzUpdt , ui32WrAddr, ui32BaseAddr, uiRBufSize));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else if(uiSzUpdt + ui32WrAddr == ui32BaseAddr + uiRBufSize)
	{ 
		/* wrap around write ptr to base ptr */
		ui32NewWrAddr = ui32BaseAddr;
		ui32WrapAroundBit ^= 1;
	}
	else
	{
		ui32NewWrAddr = ui32WrAddr + uiSzUpdt;
	}
		
	/* Write back the updated write pointer */
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
	ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        SOURCE_RING_WRADDR, 
                        ui32NewWrAddr));

	ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        SOURCE_RING_WRADDR_WRAP,
                        ui32WrapAroundBit));
#else /* for 7411 D0, 7401 etc*/
	ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        RINGBUF_WRADDR, 
                        ui32NewWrAddr));

	ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR, 
                        RINGBUF_WRADDR_WRAP,
                        ui32WrapAroundBit));
#endif /* 7411 */

    BRAP_Write32_isr (hRBuf->hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

	BDBG_LEAVE(BRAP_PB_P_WriteBufUsg_isr);
	return BERR_SUCCESS;
}
/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
Description:
	This API is required to be called by the application after it has completed
	writing of PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_BufferWriteDone_isr
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_PB_BufferWriteDone);

#if BRAP_P_WATERMARK_WORKAROUND
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);	

	BKNI_EnterCriticalSection();
	ret = BRAP_PB_BufferWriteDone_isr(hRapCh);
	BKNI_LeaveCriticalSection();
#else
    BSTD_UNUSED(hRapCh);
#endif /* BRAP_P_WATERMARK_WORKAROUND */

	BDBG_LEAVE(BRAP_PB_BufferWriteDone);
	return ret;
}

/***************************************************************************
Summary:	
	Informs the PI about the completion of pumping of data to ring buffers.
Description:
	This API is required to be called by the application after it has completed
	writing of PCM data to ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future. This API is used within the ISR context
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_PB_BufferWriteDone
****************************************************************************/
BERR_Code BRAP_PB_BufferWriteDone_isr(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
#if BRAP_P_WATERMARK_WORKAROUND
    BRAP_PcmBufInfo sPcmBufInfo;

	uint32_t	ui32WaterMarkinBytes =0;
	uint32_t	ui32TotalFreeBytes=0;
	uint32_t	ui32InitalTimerValue=0;
	uint32_t	ui32Diff=0;
	BAVC_AudioSamplingRate 	eSamplingRate;
	uint32_t	ui32IpBitsPerSample;
    unsigned int uiSR;

	BDBG_ENTER(BRAP_PB_BufferWriteDone_isr);
    BDBG_MSG(("Entering BRAP_PB_BufferWriteDone_isr"));

    /* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    ret = BTMR_StopTimer_isr ( hRapCh->hTimer );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to stop timer"));
		return BERR_TRACE(ret);
	}

	ret = BRAP_PB_GetBufInfo_isr(hRapCh, &sPcmBufInfo);
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to ger buffer info"));
		return BERR_TRACE(ret);
	}

	if ( hRapCh->bStarted == true )
	{
		/* If it is started use the start time size */
		ui32WaterMarkinBytes = ( hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiWaterMark *
						hRapCh->sModuleHandles.hRBuf[0]->sParams.uiStartSize )/100;

	}
	else
	{
		/* If it is not started use the open time size */
		ui32WaterMarkinBytes = ( hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiWaterMark *
						hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiSize )/100;
	}

	ui32TotalFreeBytes = sPcmBufInfo.sLtOrSnglBufInfo.uiTotalSize;
	BDBG_MSG(("Before StartTimer"));
	if ( ui32TotalFreeBytes >= ui32WaterMarkinBytes )
	{
		ret=BTMR_StartTimer_isr(hRapCh->hTimer, 1000);
        if ( ret != BERR_SUCCESS )
    	{
    		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to ger buffer info"));
    		return BERR_TRACE(ret);
    	}
        goto end_writedone;
	}
	else
	{
	    
		ui32Diff = ( ui32WaterMarkinBytes - ui32TotalFreeBytes );
		ui32IpBitsPerSample = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams.eInputBitsPerSample;
		eSamplingRate = hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eSamplingRate;

        ret = BRAP_P_ConvertSR(eSamplingRate, &uiSR);
        if(ret != BERR_SUCCESS)
        {
            return BERR_TRACE(ret);
        }
		if ( hRapCh->eMaxBufDataMode == BRAP_BufDataMode_eStereoInterleaved )
		{
			ui32InitalTimerValue = ( ui32Diff * 4 * 1000 *1000) / ( uiSR * ui32IpBitsPerSample);
		}
		else
		{
			ui32InitalTimerValue = ( ui32Diff * 8 * 1000 *1000) / ( uiSR * ui32IpBitsPerSample);
		}
		BDBG_MSG(("ui32Diff= %d, eSamplingRate= %d,ui32IpBitsPerSample = %d, ui32InitalTimerValue=%d"
                  "ui32WaterMarkinBytes=%d, ui32TotalFreeBytes= %d",
                  ui32Diff,eSamplingRate,ui32IpBitsPerSample,ui32InitalTimerValue,
                  ui32WaterMarkinBytes,ui32TotalFreeBytes));
        
	}
    BDBG_MSG(("After StartTimer"));

	if ( ui32InitalTimerValue == 0 )
	{
		ui32InitalTimerValue = 1;
	}

	ret = BTMR_StartTimer_isr ( hRapCh->hTimer, ui32InitalTimerValue );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_PB_BufferWriteDone_isr() Failed to start timer"));
		return BERR_TRACE(ret);
	}
    BDBG_MSG(("After BTMR_StartTimer"));

end_writedone:
	BDBG_LEAVE(BRAP_PB_BufferWriteDone_isr);
#else
    BSTD_UNUSED(hRapCh);

#endif /* BRAP_P_WATERMARK_WORKAROUND */
	return ret;
}

/***************************************************************************
Summary:
	Function that updates sampling rate of SRC.
Description:
	Function that updates sampling rate of SRC.This function must be called
	for PCM playback channels only.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_PB_SetSRCInputFrequency
****************************************************************************/
BERR_Code 
BRAP_PB_SetSRCFrequency(
		BRAP_ChannelHandle hRapCh,
		BRAP_OutputPort eOutputPort, 
		unsigned int uiSROut, 
		unsigned int uiSRIn)
{
    BERR_Code ret = BERR_SUCCESS;
    bool bClone = false;
    bool bSimulPt = false;
    
    BRAP_OP_P_Handle hOp = NULL;
    BRAP_SPDIFFM_P_Handle hSpdifFm = NULL;
    BRAP_MIXER_P_Handle hMixer = NULL;    
    BRAP_SRCCH_P_Handle hSrcCh = NULL;
    unsigned int uiStreamId = 0;
    uint32_t ui32SrcIORatio = 0;
    uint32_t ui32RegVal = 0;
    uint32_t ui32Temp = 0;
    int j=0;

    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;      

    BRAP_OutputChannelPair eChanPair = BRAP_OutputChannelPair_eLR;

    BDBG_ENTER(BRAP_PB_SetSRCFrequency);

    BDBG_ASSERT(uiSRIn);
    BDBG_ASSERT(uiSROut);

    BDBG_MSG(("hRapCh->eChannelType=%d   eOutputPort=%d",hRapCh->eChannelType, eOutputPort));

    BDBG_ASSERT(hRapCh->sModuleHandles.hFmm);

    /* Get the channel pair in RapCh using this output port */
    ret = BRAP_P_GetChannelPairUsingOpPort_isr(hRapCh, eOutputPort, &eChanPair, 
        &bClone, &bSimulPt/* Ignored here */);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_PB_SetSRCFrequency: Can't get a channel pair using the output port"));
        return BERR_TRACE (ret);
    }
    
    BDBG_MSG(("eChanPair = %d, bClone=%d, bSimulPt = %d",
        eChanPair, bClone, bSimulPt));
    /* NOTE: this PI gets called only for PB and CAP channels which have 
       only one channel pair !! But we use it to find out whether this 
       port is cloned or not  */   
    if (eChanPair != BRAP_OutputChannelPair_eLR)
    {
        BDBG_ERR(("BRAP_PB_SetSRCFrequency: invalid channel pair"));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }    

    if (bSimulPt == true)
    {
        BDBG_ERR(("BRAP_PB_SetSRCFrequency: PCM/CAP channels do not have bSimulPt mode"));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }    


    if (eOutputPort == BRAP_OutputPort_eMai)
    {
		eMaiMuxSelect = hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }  

    /* Get the associated module handles */
    if (bClone == false)
    {
        hOp = hRapCh->sModuleHandles.hOp[eChanPair];  
        hSpdifFm = hRapCh->sModuleHandles.hSpdifFm[eChanPair];  
        hMixer = hRapCh->sModuleHandles.hMixer[eChanPair];  
        hSrcCh = hRapCh->sModuleHandles.hSrcCh[eChanPair];          
    }
    else
    {
        hOp = hRapCh->sCloneOpPathHandles[eChanPair][eOutputPort].hOp;  
        hSpdifFm = hRapCh->sCloneOpPathHandles[eChanPair][eOutputPort].hSpdifFm;  
        hMixer = hRapCh->sCloneOpPathHandles[eChanPair][eOutputPort].hMixer;  
        hSrcCh = hRapCh->sCloneOpPathHandles[eChanPair][eOutputPort].hSrcCh;  

        if (hSrcCh == NULL)
        {
            /* This means that no new SrcCh was allocated for this Cloned port.
               we need to find out the correct SrcCh to associate with it */

            hSrcCh = hRapCh->sModuleHandles.hSrcCh[eChanPair]; 

            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {    
                if (hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh != NULL)
                { 
                    break;
                }
            }
            if (j!=BRAP_RM_P_MAX_OUTPUTS)
            {
                /* One of the cloned ports has a specially allocated hSrcCh
                => one port was cloned in another DP, say DPx */
                /* if eOutputPort belongs to DPx, it should use 
                hSrcCh from sCloneOpPathHandles, else from sModuleHandles */
                if (hRapCh->sCloneOpPathResc[eChanPair][eOutputPort].uiDataPathId
                        == hRapCh->sCloneOpPathResc[eChanPair][j].uiDataPathId)

                {
                    hSrcCh = hRapCh->sCloneOpPathHandles[eChanPair][j].hSrcCh;
                }
            }
        }
    }

	BDBG_ASSERT(hOp);
	BDBG_ASSERT(hMixer);
	BDBG_ASSERT(hSrcCh);


	if((ret == BERR_SUCCESS) && (hSrcCh->eState == BRAP_SRCCH_P_State_eRunning))
	{
		/* Program SRC */
         BDBG_MSG(("\nBRAP_MIXER_P_SetSRC_isr:" "\n\t hMixer = 0x%x"
    		"\n\t hSrcCh = 0x%x" "\n\t eOutputSR = %d"
    		"\n\t eInputSR = %d",hMixer, hSrcCh,uiSROut,uiSRIn));

        ret = BRAP_RM_P_GetDpStreamId (hSrcCh->uiIndex,hMixer->uiDpIndex, &uiStreamId); 
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_MIXER_P_SetSRC_isr: Cant get stream index in this DP"));
            return BERR_TRACE (ret);
        }

    	BDBG_MSG(("BRAP_MIXER_P_SetSRC_isr: uiStreamId = %d",uiStreamId));
        
    	/* Program Sampling Rate Conversion Ratio */
    	ui32SrcIORatio = (((uiSRIn<<14)/uiSROut) <<1);
    	ui32Temp = BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK1_SCALE_SRC_CONFIG
                    - BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG;

    	ui32RegVal = BRAP_Read32_isr (hMixer->hRegister, 
                        		(hMixer->ui32DpOffset + 
                        		BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG + 
                        		(uiStreamId * ui32Temp)));

       	ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, SRC_IOR));
        
       	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, SRC_IOR, 
    									ui32SrcIORatio));
    	
    	/* Enable SRC */
    	ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, SRC_ENB));
       	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, SRC_ENB, 1));

       	BRAP_Write32_isr (hMixer->hRegister, 
    		    	  (hMixer->ui32DpOffset + 
                   	   BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG + 
                   	   (uiStreamId * ui32Temp)), 
    				  ui32RegVal);
    	
    	BDBG_MSG(("BRAP_MIXER_P_SetSRC_isr: uiSRIn = %d, uiSROut = %d, ui32SrcIORatio = 0x%x",
                uiSRIn, uiSROut,ui32SrcIORatio));
        

	}
    else
    {
        BDBG_ERR(("PCM playback channel is stopped"));
    }

	BDBG_LEAVE(BRAP_PB_SetSRCFrequency);
	return ret;
}


