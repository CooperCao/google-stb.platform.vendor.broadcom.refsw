/***************************************************************************
*     Copyright (c) 2004-2009, Broadcom Corporation
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
* Module Description: This file contains the implentation for a PCM 
*					  Capture channel.It includes implementation of public 
*					  and private PIs associated with a PCM Capture channel.
*					  Structure definitions etc are included in brap_pcm.h.
*					  This module is part of the Audio Manager. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"
#include "brap_pcm.h"


BDBG_MODULE(rap_cap); /* Register software module with debug interface */

/* Full byte water mark in percentage. Once, buffer usage reaches this value an 
   interrupt is generated */
#define BRAP_CAP_P_RBUF_DEFAULT_FULL_BYTE_MARK 	75  

extern bool bValidatePcmOpNDataModes[BRAP_PcmOutputMode_eMaxNum]	
									[BRAP_BufDataMode_eMaxNum] ;
extern BRAP_SRCCH_P_LRDataControl lrDataCtrl[];

 
static BERR_Code BRAP_CAP_P_InputConfig ( 
        BRAP_ChannelHandle  hRapCh,    /* [in] The Raptor Capture Channel handle */
        BRAP_CapInputType	eCapInputType,
		BRAP_CapInputPort	eCapInput
);

static BERR_Code BRAP_CAP_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	BRAP_RingBufInfo	*pRBufInfo
);

static BERR_Code BRAP_CAP_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	unsigned int		uiSzUpdt
);

/***************************************************************************
Summary:
	Returns the default settings of the PCM Capture Channel.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_CAP_OpenChannel.
***************************************************************************/
BERR_Code BRAP_CAP_GetChannelDefaultSettings (
        BRAP_Handle         hRap,        /* [in] The Raptor device handle */
        BRAP_CAP_ChannelSettings *pDefSettings    /* [out] Pointer to memory where the Default channel settings are returned */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ASSERT (hRap);
    BDBG_ENTER (BRAP_CAP_GetChannelDefaultSettings);

    BSTD_UNUSED(hRap);
    
    pDefSettings->eCapMode = BRAP_CaptureMode_eFullDuplex;   
    pDefSettings->eOutputPort = BRAP_OutputPort_eDac0;

	pDefSettings->eMaxBufDataMode = BRAP_BufDataMode_eStereoInterleaved;

	/* PCM stereo-interleaved needs a single buffer, so only left buffer be valid */
	pDefSettings->sLtOrSnglRbufSettings.pBufferStart = NULL; 
	pDefSettings->sLtOrSnglRbufSettings.uiSize = BRAP_RBUF_P_DEFAULT_SIZE;
	pDefSettings->sLtOrSnglRbufSettings.uiWaterMark = BRAP_CAP_P_RBUF_DEFAULT_FULL_BYTE_MARK;

	/* Right buffer not required for PCM stereo-interleaved */
	pDefSettings->sRtRbufSettings.pBufferStart = NULL; 
	pDefSettings->sRtRbufSettings.uiSize = 0;
	pDefSettings->sRtRbufSettings.uiWaterMark = 0;

    BDBG_LEAVE (BRAP_CAP_GetChannelDefaultSettings);
    return BERR_TRACE(ret);
}

/***************************************************************************
Summary:
	Returns the default audio params for a PCM Capture Channel.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_CAP_Start, BRAP_CAP_Stop
***************************************************************************/
BERR_Code BRAP_CAP_GetDefaultAudioParams(
        BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
        BRAP_CAP_AudioParams *pDefParams  /* [out] Pointer to memory where the default 
                                             channel parameters are returned */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_DSTCH_P_Params sDstChParams;    
    BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	BRAP_OP_SpdifChanStatusParams	sSpdifChanStatusParams;    

    BDBG_ASSERT (hRapCh);
    BDBG_ENTER (BRAP_CAP_GetDefaultAudioParams);

    BSTD_UNUSED(hRapCh);

    ret = BRAP_DSTCH_P_GetDefaultParams(&sDstChParams);
	if(ret != BERR_SUCCESS)
		return BERR_TRACE(ret); 
    
    pDefParams->sDstChParams = sDstChParams.sExtParams;
    pDefParams->eBufDataMode = sDstChParams.eBufDataMode; 
    pDefParams->uiDelayRbuf =  0;
    pDefParams->ui32RbufMemSize = BRAP_RBUF_P_DEFAULT_SIZE;
    pDefParams->eInputSamplingRate = BAVC_AudioSamplingRate_e32k;
    pDefParams->eInputBitsPerSample =  sDstChParams.eInputBitsPerSample; 
    pDefParams->ePcmOutputMode = BRAP_PcmOutputMode_eStereo;     
    pDefParams->eCapInputType = sDstChParams.eCapInputType;  
    
	/* Get Mixer default parameters */
	ret = BRAP_MIXER_P_GetDefaultParams(&sMixerParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sAudioOutputParams.sMixerParams = sMixerParams.sExtParams;
	else
		return BERR_TRACE(ret); 

   	pDefParams->sAudioOutputParams.uiOutputBitsPerSample = sMixerParams.uiStreamRes;

	/* Get SPDIF Formater default parameters */
	ret = BRAP_SPDIFFM_P_GetDefaultParams(&sSpdifFmParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sAudioOutputParams.sSpdifFmParams = sSpdifFmParams.sExtParams;
	else
		return BERR_TRACE(ret); 

	/* Get default SPDIF Channel Status parameters */
	ret = BRAP_OP_P_GetDefaultSpdifChanStatusParams(&sSpdifChanStatusParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sSpdifChanStatusParams = sSpdifChanStatusParams;
	else
		return BERR_TRACE(ret);

#if BRAP_UNSIGNED_PCM_SUPPORTED
    pDefParams->bIsSigned = true;
#endif

    BDBG_LEAVE (BRAP_CAP_GetDefaultAudioParams);
    return ret;
}


/***************************************************************************
Summary:
	API used to open a PCM Capture channel.
Description:
	It is used to instantiate a PCM Capture channel. It allocates channel 
	handle and resource required for the channel.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_CAP_CloseChannel, BRAP_CAP_GetChannelDefaultSettings
****************************************************************************/
BERR_Code BRAP_CAP_OpenChannel( 
    BRAP_Handle                     hRap,         /* [in] The Raptor device handle */
    BRAP_ChannelHandle              *phRapCh,     /* [out] The Raptor Capture Channel handle */
    unsigned int                    uiChannelNo,  /* [in] the desired channel ID */
    const BRAP_CAP_ChannelSettings  *pChnSettings /* [in] Pointer to Capture Channel settings*/ 
)
{
    BERR_Code ret = BERR_SUCCESS;
    
    BRAP_ChannelHandle      hRapCh;
    BRAP_RBUF_P_Settings    sRbufSettings[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
    BRAP_SRCCH_P_Settings   sSrcChSettings;
    BRAP_MIXER_P_Settings   sMixerSettings;
    BRAP_SPDIFFM_P_Settings sSpdfFmSettings;
    BRAP_OP_P_MaiSettings   sMaiSettings;
    BRAP_OP_P_SpdifSettings sSpdifSettings;
    BRAP_OP_P_I2sSettings   sI2sSettings;
    BRAP_OP_P_FlexSettings  sFlexSettings;
    BRAP_OP_P_DacSettings   sDacSettings;
    BRAP_RM_P_ResrcReq      sResrcReq;
	BRAP_DSTCH_P_Settings	sDstChSettings;
    void                    *pOpSettings = NULL;
    unsigned int            i;    
    BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;
#ifndef BCHP_7411_VER /* not for 7411 */
    unsigned int            j;
    BRAP_P_ObjectHandles        sTempHandles;    
#endif

#if BRAP_P_WATERMARK_WORKAROUND==1
	BTMR_Settings sTimerSettings;
#endif

    BDBG_ENTER(BRAP_CAP_OpenChannel);

    /* validate input params */
    BDBG_ASSERT (hRap);
    BDBG_ASSERT (phRapCh);  

    if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
    {
        BDBG_ASSERT (pChnSettings);
        BDBG_MSG(("\nBRAP_CAP_OpenChannel:" 
            "\n\tpChnSettings->eCapMode = %d"
            "\n\tpChnSettings->eMaxBufDataMode = %d"
            "\n\tpChnSettings->eOutputPort = %d"
            "\n\tpChnSettings->sLtOrSnglRbufSettings.pBufferStart = %p"
            "\n\tpChnSettings->sLtOrSnglRbufSettings.uiSize = %d"
            "\n\tpChnSettings->sLtOrSnglRbufSettings.uiWaterMark = %d"
            "\n\tpChnSettings->sRtRbufSettings.pBufferStart = %p"
            "\n\tpChnSettings->sRtRbufSettings.uiSize = %d"
            "\n\tpChnSettings->sRtRbufSettings.uiWaterMark = %d",
            pChnSettings->eCapMode,
            pChnSettings->eMaxBufDataMode,
            pChnSettings->eOutputPort,
            pChnSettings->sLtOrSnglRbufSettings.pBufferStart,
            pChnSettings->sLtOrSnglRbufSettings.uiSize,
            pChnSettings->sLtOrSnglRbufSettings.uiWaterMark,
            pChnSettings->sRtRbufSettings.pBufferStart,
            pChnSettings->sRtRbufSettings.uiSize,
            pChnSettings->sRtRbufSettings.uiWaterMark));

#ifndef BCHP_7411_VER     
        BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));
#endif 

        if (uiChannelNo >= BRAP_RM_P_MAX_CAP_CHANNELS)
        {
            /* The channel is not supported */
    	    return BERR_TRACE(BERR_NOT_SUPPORTED);
        } 

	if(pChnSettings->eCapMode == BRAP_CaptureMode_eByPass )
	{
		BDBG_ERR(("Bypass mode is only for testing purpose and it is not supported"));		
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}

        /* Capture Only mode: Ignore output port */
        if(pChnSettings->eCapMode != BRAP_CaptureMode_eCaptureOnly)
        {
            ret = BRAP_RM_P_IsOutputPortSupported(pChnSettings->eOutputPort);
        	if( ret != BERR_SUCCESS)
        	{
        		BDBG_ERR(("BRAP_CAP_OpenChannel: eOutputPort(%d) is not supported",
        			pChnSettings->eOutputPort));
        		return BERR_TRACE(ret);
        	}
        }        
        
        ret = BRAP_P_IsNewChanCompatible(hRap, BRAP_P_ChannelType_eCapture, pChnSettings->eOutputPort);
        if(ret!=BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_CAP_OpenChannel: CAP channel not compatible with existing channels"));
            return BERR_TRACE(ret);
        }

        /* Allocate a Capture Channel handle */     
        *phRapCh = hRapCh = (BRAP_ChannelHandle)BKNI_Malloc(sizeof(BRAP_P_Channel));
        if(hRapCh == NULL)
        {
            BDBG_ERR(("BRAP_CAP_OpenChannel: Memory allocation for channel handle failed"));
            return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        }

        hRap->hRapCapCh[uiChannelNo] = hRapCh;
        
        /* Memset 0 */
        BKNI_Memset(hRapCh, 0, sizeof(BRAP_P_Channel));

        /* Initialize channel handle */
        hRapCh->hRap = hRap ;
        hRapCh->hChip = hRap->hChip ;
        hRapCh->hHeap = hRap->hHeap ;
        hRapCh->hInt = hRap->hInt ;
        hRapCh->hRegister = hRap->hRegister;
        hRapCh->ui32FmmBFIntMask = 0;
        hRapCh->bStarted = false;        

    	/* PCM capture doesn't need transport channel */
#if BRAP_P_USE_BRAP_TRANS ==1
        hRapCh->hRapTransCh = NULL;
#else
        hRapCh->uiXptChannelNo = BRAP_INVALID_VALUE;
#endif

        hRapCh->eChannelType = BRAP_P_ChannelType_eCapture;
        hRapCh->eMaxBufDataMode = pChnSettings->eMaxBufDataMode;
        hRapCh->uiChannelNo = uiChannelNo;
        hRapCh->bSimulModeConfig = false;
#ifdef BCHP_7411_VER    
        hRapCh->eClone = BRAP_P_CloneState_eInvalid;
#endif	
    	hRapCh->eCapMode = pChnSettings->eCapMode;

#if BRAP_P_WATERMARK_WORKAROUND==1
    	if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    	{
		BTMR_GetDefaultTimerSettings(&sTimerSettings);
		sTimerSettings.type = BTMR_Type_eCountDown;
		sTimerSettings.cb_isr = (BTMR_CallbackFunc) BRAP_P_FmmDstRbufFullWaterMark_isr;
		sTimerSettings.pParm1 = hRapCh;
		sTimerSettings.parm2 = 0;
		sTimerSettings.exclusive = false;

		ret = BTMR_CreateTimer ( hRap->hTmr, &hRapCh->hTimer, &sTimerSettings);
		if ( ret != BERR_SUCCESS )
		{
	    		BDBG_ERR(("BRAP_CAP_OpenChannel: Create Timer Failed"));
	    		ret = BERR_TRACE(ret);
	    		goto end_open;
		}
    	}
#endif

#ifndef BCHP_7411_VER /* not for 7411 */
        /* Initialise the data structures for cloned mode */
    	for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    	{
   	        for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {             
                hRapCh->sCloneOpPathHandles[i][j].bSimul = false;
                hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
                hRapCh->sCloneOpPathResc[i][j].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiMixerId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
                hRapCh->sCloneOpPathResc[i][j].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;                 
    	    }
        }
#endif

        /* Form the resource requirement depending on the Capture Mode */
        sResrcReq.eChannelType = hRapCh->eChannelType;
        sResrcReq.eBufDataMode = pChnSettings->eMaxBufDataMode;   
        sResrcReq.bSimulModePt = false;
        sResrcReq.bAllocateDsp = false; 
    sResrcReq.bAllocateDspCtx = false; 
	
        /*  bLastChMono = TRUE for left mono and right mono or stereo-interleaved, 
                                     FALSE for true mono and stereo */    
        if(sResrcReq.eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
            sResrcReq.bLastChMono = false;
        else
            sResrcReq.bLastChMono = true;

        sResrcReq.bAllocateDstCh = true;   /* always ask for Dst Ch */

        if (pChnSettings->eCapMode == BRAP_CaptureMode_eCaptureOnly)
        {
        	sResrcReq.uiNumOpPorts = 0;

        	for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        	{
                sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
        	}
            sResrcReq.bAllocateRBuf= true;
            sResrcReq.bAllocateSrcCh= false;
        }    
        else
        {
        	sResrcReq.uiNumOpPorts = 1;
        	for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        	{
                sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
        	}

            /* Request Op Port */
            sResrcReq.sOpPortReq[eChanPair].eOpPortType = pChnSettings->eOutputPort;

        	/* If output port is MAI then initialize the MAI mux selector */			
        	if ( pChnSettings->eOutputPort == BRAP_OutputPort_eMai)
            {
        	    sResrcReq.sOpPortReq[eChanPair].eMuxSelect = 
    	    		hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
            }       
        
            /* Request SrcCh*/    
            sResrcReq.bAllocateSrcCh= true;

            /* Request RBUF if reqd */ 
            if (pChnSettings->eCapMode == BRAP_CaptureMode_eByPass)
            {
                sResrcReq.bAllocateRBuf= false;
            }
            else
            {
                sResrcReq.bAllocateRBuf= true;
            }
        }

        /* Call resource manager to allocate required resources. */
        ret = BRAP_RM_P_AllocateResources (hRap->hRm, 
    									     &sResrcReq, 
    									     &(hRapCh->sRsrcGrnt));
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_CAP_OpenChannel: Resource allocation failed for RAP CAP Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret);
            goto free_channel;
        }
        BDBG_MSG(("BRAP_CAP_OpenChannel: Resource allocation for PCM CAP succeeded"));

        /* Store DSP(NULL) and FMM handles inside the Audio Channel handle */ 
        hRapCh->sModuleHandles.hDsp = NULL; 
        hRapCh->sModuleHandles.hFmm = hRap->hFmm[hRapCh->sRsrcGrnt.uiFmmId];
        
        if (pChnSettings->eCapMode != BRAP_CaptureMode_eCaptureOnly)
        {
            /* the FMM OP path is used only in FullDuplex or Bypass mode, not in capture only mode */
            
            /* ring buffer settings: note in capture channel, the rbufs are associated with the input path, not the output path
                therefore they are not opened here */
    		BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[0]);
    		BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[1]);                  
            sRbufSettings[0].sExtSettings = pChnSettings->sLtOrSnglRbufSettings;
            sRbufSettings[0].bProgRdWrRBufAddr = true;
            sRbufSettings[1].sExtSettings = pChnSettings->sRtRbufSettings;
            sRbufSettings[1].bProgRdWrRBufAddr = true;
            
            /* source channel settings: Currently it is blank */
            
            /* Mixer settings */
            BRAP_MIXER_P_GetDefaultSettings(&sMixerSettings);            
            sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiMixerInputId;

            /* SPDIFFM settings */
            BRAP_SPDIFFM_P_GetDefaultSettings (&sSpdfFmSettings);            
            sSpdfFmSettings.sExtSettings = hRap->sOutputSettings[pChnSettings->eOutputPort].sSpdiffmSettings;

            /* Output port settings */
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
                    BDBG_ERR(("BRAP_CAP_OpenChannel: Output port type %d not supported", pChnSettings->eOutputPort));
                    ret = BERR_TRACE(BERR_NOT_SUPPORTED);
                    goto free_rs;
            }/* switch */
        }/* if (pChnSettings->eCapMode != BRAP_CaptureMode_eCaptureOnly) */
        
    	hRapCh->hInterruptCount = BRAP_P_AllocAligned( hRap,
    										28,
    										2,
    										0
#if (BRAP_SECURE_HEAP==1) 
    										,false
#endif																							
    										);
    	if ((uint32_t)hRapCh->hInterruptCount==BRAP_P_INVALID_DRAM_ADDRESS)
    	{
#if (BRAP_SECURE_HEAP==1)
    		BRAP_P_Free(hRap, (void *) hRapCh->hInterruptCount,false);
#else
    		BRAP_P_Free(hRap, (void *) hRapCh->hInterruptCount);
#endif				
    	}        
    }/* if !watchdog */
    else
    {
        /* If watchdog recovery */
    	hRapCh = *phRapCh;
        sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiMixerInputId;
    }

  /* Now open the input path modules */
    /* Open the RBUF */
    if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
    {
        sRbufSettings[0].sExtSettings = pChnSettings->sLtOrSnglRbufSettings;
        sRbufSettings[0].bProgRdWrRBufAddr = true;
        sRbufSettings[1].sExtSettings = pChnSettings->sRtRbufSettings;
        sRbufSettings[1].bProgRdWrRBufAddr = true;
    }
    
    for (i=0; i<BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
    {
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRap))
	{
		/* note: the rbufs handle attached to destination fifo for Cap channel will be in sModuleHandles.hRBuf[2]
		    and sModuleHandles.hRBuf[3] */
		BDBG_MSG(("For dst rbuf \n"
		       "uiRbufIndex=%d, " 
		       "pSettings->bProgRdWrRBufAddr=%d,"
		       "pSettings->bRbufOfClonedPort=%d,"
		       "pSettings->sExtSettings.pBufferStart=0x%x,"
		       "pSettings->sExtSettings.uiSize=%d,"
		       "pSettings->sExtSettings.uiWaterMark=%d ",
		       hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[i],
		       sRbufSettings[0].bProgRdWrRBufAddr,
		       sRbufSettings[0].bRbufOfClonedPort,
		       sRbufSettings[0].sExtSettings.pBufferStart,
		       sRbufSettings[0].sExtSettings.uiSize,
		       sRbufSettings[0].sExtSettings.uiWaterMark)); 

		sRbufSettings[0].bRbufOfClonedPort = false;
		sRbufSettings[1].bRbufOfClonedPort = false;            
	}
	else
	{
		sRbufSettings[0] = hRapCh->sModuleHandles.hRBuf[i+2]->sSettings;
		sRbufSettings[1] = hRapCh->sModuleHandles.hRBuf[i+2]->sSettings;

		/* note: the rbufs handle attached to destination fifo for Cap channel will be in sModuleHandles.hRBuf[2]
		    and sModuleHandles.hRBuf[3] */
		BDBG_MSG(("For dst rbuf \n"
		       "uiRbufIndex=%d, " 
		       "pSettings->bProgRdWrRBufAddr=%d,"
		       "pSettings->bRbufOfClonedPort=%d,"
		       "pSettings->sExtSettings.pBufferStart=0x%x,"
		       "pSettings->sExtSettings.uiSize=%d,"
		       "pSettings->sExtSettings.uiWaterMark=%d ",
		       hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[i],
		       sRbufSettings[0].bProgRdWrRBufAddr,
		       sRbufSettings[0].bRbufOfClonedPort,
		       sRbufSettings[0].sExtSettings.pBufferStart,
		       sRbufSettings[0].sExtSettings.uiSize,
		       sRbufSettings[0].sExtSettings.uiWaterMark)); 
	}
	
        if (hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[i] != (unsigned int)BRAP_RM_P_INVALID_INDEX)    
        {
		if(hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
		{		
		    ret = BRAP_RBUF_P_Open (hRapCh->sModuleHandles.hFmm, 
		                      &(hRapCh->sModuleHandles.hRBuf[i+2]),
		                      hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[i],
		                      &(sRbufSettings[i]));
		}
		else
		{
		    ret = BRAP_RBUF_P_Open (hRapCh->sModuleHandles.hFmm, 
		                      &(hRapCh->sModuleHandles.hRBuf[i]),
		                      hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[i],
		                      &(sRbufSettings[i]));
		}
        
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_CAP_OpenChannel: RBuf Open failed %d", ret));
			ret = BERR_TRACE(ret);
			goto free_rs;
		}   
        }
    }                                


    /* Open all the FMM OP path modules. This is not required for Capture Mode */
    if(hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
    {		
	 if ( hRapCh->sModuleHandles.hRBuf[2] != NULL )
	 {
	 	sRbufSettings[0] = hRapCh->sModuleHandles.hRBuf[2]->sSettings;
	       sRbufSettings[0].bRbufOfClonedPort = true;
		sRbufSettings[0].bProgRdWrRBufAddr = false;
	 }

	 if ( hRapCh->sModuleHandles.hRBuf[3] != NULL )
	 {
	 	sRbufSettings[1] = hRapCh->sModuleHandles.hRBuf[3]->sSettings;
        sRbufSettings[1].bRbufOfClonedPort = true;
		sRbufSettings[1].bProgRdWrRBufAddr = false;
 	 }

         /* This is a shared RBUF. Note that no new memory will be allocated for it. 
        We'll be reusing the memory associated with the capture RBUF and 
        also capture RBUF settings */
        BDBG_MSG(("For PB rbuf \n"
               "uiRbufIndex=%d, " 
               "pSettings->bProgRdWrRBufAddr=%d,"
               "pSettings->bRbufOfClonedPort=%d,"
               "pSettings->sExtSettings.pBufferStart=0x%x,"
               "pSettings->sExtSettings.uiSize=%d,"
               "pSettings->sExtSettings.uiWaterMark=%d ",
               hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiRbufId[0],
               sRbufSettings[0].bProgRdWrRBufAddr,
               sRbufSettings[0].bRbufOfClonedPort,
               sRbufSettings[0].sExtSettings.pBufferStart,
               sRbufSettings[0].sExtSettings.uiSize,
               sRbufSettings[0].sExtSettings.uiWaterMark)); 
     
        ret = BRAP_P_OpenOpPathFmmModules ( hRapCh,
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
            BDBG_ERR(("BRAP_CAP_OpenChannel: FMM Op Path Module Opens failed for RAP CAP Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret);
            goto free_rs;
        }   
        BDBG_MSG(("BRAP_CAP_OpenChannel: Open output main FMM modules succeeded"));

#ifndef BCHP_7411_VER
        /* If in watchdog recovery, also open cloned port resources */
        if (BRAP_P_GetWatchdogRecoveryFlag(hRap))            
        {
            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {
                if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                    && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == false))
                {                  
                    BDBG_MSG (("BRAP_CAP_OpenChannel: opening cloned port %d", j));
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
                		BDBG_ERR(("BRAP_CAP_OpenChannel: OpPathFmmModule failed for cloned port %d", j));
                        ret = BERR_TRACE(ret);
                        goto free_rs;
                	}
                 }
            }
        }
#endif
        
    }
    
  
    /* Open the DSTCH */
    ret = BRAP_DSTCH_P_Open (hRapCh->sModuleHandles.hFmm, 
                             &(hRapCh->sModuleHandles.hDstCh), 
                             hRapCh->sRsrcGrnt.sCapResrcId.uiDstChId, 
                             &sDstChSettings);
	if(ret != BERR_SUCCESS)
    {
		BDBG_ERR(("BRAP_CAP_OpenChannel: Dst channel open for RAP CAP Channel handle 0x%x", hRapCh));
        ret = BERR_TRACE(ret);
        goto free_rs;
    }   

	/* Install the interrupts */
	ret = BRAP_P_InterruptInstall(hRapCh);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("Interrupt installation failed for RAP Capture Channel handle 0x%x", hRapCh));
		goto free_rs;
	}

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
    	if(ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("Interrupt installation failed for RAP Capture Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret);
    		goto free_rs;
    	}
    }
#endif
#endif

	goto end_open;

	/* Exit doors */
free_rs:
	BDBG_MSG(("BRAP_CAP_OpenChannel: ***** Free RS called ***** "));
	/* Call Resource manager to release these resources */
	ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
	if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
		BDBG_ERR(("BRAP_CAP_OpenChannel: BRAP_RM_P_FreeResources() for main modules failed."));
    }

free_channel:
	/* Free up the channel handle */
	BKNI_Free(hRapCh);

end_open:
    BDBG_LEAVE (BRAP_CAP_OpenChannel);
    return ret;
}


/***************************************************************************
Summary:
	API used to close a PCM Capture channel.
Description:
	It closes the instance of a PCM Capture channel operation. It frees the 
	channel handle and resources associated with it
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_CAP_OpenChannel
****************************************************************************/
BERR_Code BRAP_CAP_CloseChannel ( 
        BRAP_ChannelHandle     hRapCh    /* [in] The Raptor Capture Channel handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_OutputChannelPair eChanPair = BRAP_OutputChannelPair_eLR;    
	unsigned int i;

#ifndef BCHP_7411_VER /* not for 7411 */
    unsigned int    j;
    BRAP_P_ObjectHandles   	sTempHandles;        
    BRAP_RM_P_ResrcGrant 	sResrcGrant;        
#endif    

	BDBG_ENTER(BRAP_CAP_CloseChannel);
	BDBG_ASSERT(hRapCh);

#ifndef BCHP_7411_VER /* not for 7411 */    
    /* Make sure the temp handles etc are clean */
    BKNI_Memset (&sResrcGrant, 0, sizeof(sResrcGrant));
    BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));

#if (BRAP_SECURE_HEAP==1)
	BRAP_P_Free(hRapCh->hRap, (void *) hRapCh->hInterruptCount,false);
#else
	BRAP_P_Free(hRapCh->hRap, (void *) hRapCh->hInterruptCount);
#endif

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
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
    	if(ret != BERR_SUCCESS)
    	{
            ret = BERR_TRACE(ret);
    	    BDBG_ERR(("BRAP_CAP_CloseChannel: BRAP_P_MaskInterrupt() returned err(%d)",ret));	
        }    
    }
#endif
#endif


#if (BRAP_P_WATERMARK_WORKAROUND == 1)
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
	    if (NULL != hRapCh->hTimer)
	    {
	        ret = BTMR_DestroyTimer(hRapCh->hTimer);
	        if (ret != BERR_SUCCESS)
	        {
	            BDBG_ERR (("BRAP_CAP_CloseChannel: BTMR_DestroyTimer Failed"));
	            ret = BERR_TRACE (ret);
	        }
	    }
    }
#endif

    BRAP_P_InterruptUnInstall (hRapCh);

    /* For a capture channel the rbufs associated with Destination FIFOs are
    * stored in hRapCh->sModuleHandles.hRBuf[2] and [3] while opening */
    for (i=0; i<BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
    {
       /* note: the rbuf handle for Cap channel will be in sModuleHandles.hRBuf[0]
           and sModuleHandles.hRBuf[1] */
        if (hRapCh->sModuleHandles.hRBuf[i+2] != NULL)    
        {
            BRAP_RBUF_P_Close(hRapCh->sModuleHandles.hRBuf[i+2]);

	     /* Assign the Freed Rbuff handles NULL so that close output path
	      * will not try to free this again */
	      hRapCh->sModuleHandles.hRBuf[i+2] = NULL;
        }
    }                                

    /* close the DSTCH */
    BDBG_ASSERT((hRapCh->sModuleHandles.hDstCh != NULL));
    ret = BRAP_DSTCH_P_Close (hRapCh->sModuleHandles.hDstCh);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_CAP_CloseChannel: call to BRAP_P_DSTCH_P_Close() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }
    
    /* 2. Close output path modules if reqd */  
    if (hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
    {
        /* the FMM OP path is used only in FullDuplex or Bypass mode, not in capture only mode */


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

            BDBG_MSG(("BRAP_CAP_CloseChannel: Resources for clone port %d closed.", j));
            
        }        
    }     
#endif 

        
        ret = BRAP_P_CloseOpPathFmmModules(eChanPair, &(hRapCh->sModuleHandles));
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_CAP_CloseChannel: call to BRAP_P_CloseOpPathFmmModules() failed. Ignoring error!!!!!"));
            ret = BERR_TRACE (ret);
        }
        BDBG_MSG(("BRAP_CAP_CloseChannel: Output FMM modules closed."));
    }	

    /* 3. Call Resource manager to release these resources */
    ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_CAP_CloseChannel: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }
    BDBG_MSG(("BRAP_CAP_CloseChannel: Resources Freed."));

	/* 4. Mark the place for the current channel handle to NULL inside RAP handle */
	hRapCh->hRap->hRapCapCh[hRapCh->uiChannelNo] = NULL;

	/* 5. Free the channel handle */
	BKNI_Free(hRapCh);
                     
    BDBG_LEAVE (BRAP_CAP_CloseChannel);
    return ret;
}


/***************************************************************************
Summary:
	Starts a PCM Capture channel.
Description:
	This API is required to start the PCM Capture of the selected stream, 
	as specified by input BRAP_CAP_AudioParams structure.
Returns:
	BERR_SUCCESS on success, error on failure
See Also:
	BRAP_CAP_Stop, BRAP_CAP_GetDefaultAudioParams
****************************************************************************/
BERR_Code BRAP_CAP_Start ( 
        BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
        const BRAP_CAP_AudioParams    *pAudioParams /*[in] Audio parameters required for starting this channel */
)
{
    BERR_Code ret = BERR_SUCCESS;

	BRAP_DSTCH_P_Params		sDstChParams;
	BRAP_RBUF_P_Params		sRBufParams[BRAP_RM_P_MAX_RBUFS_PER_DSTCH];
	BRAP_SRCCH_P_Params		sSrcChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	void					*pOpParams = NULL;
	unsigned int 			uiRbufSize[BRAP_RM_P_MAX_RBUFS_PER_DSTCH];
	unsigned int			k;
    BRAP_OutputChannelPair  eChanPair = BRAP_OutputChannelPair_eLR;
	BRAP_OP_I2sSettings		*pOpI2sSettings = NULL;
    BAVC_AudioSamplingRate  eInputSR = BAVC_AudioSamplingRate_eUnknown;
    BRAP_InputBitsPerSample eIpBitsPerSample = BRAP_InputBitsPerSample_e16;
	bool					bIsCapMixedWithCompressed;
    unsigned int            uiStartWRPoint = 0;

#ifndef BCHP_7411_VER  /* only for 7411 */      
    BRAP_P_ObjectHandles        sTempHandles;  
    unsigned int j;
#endif    
    
	BDBG_ENTER(BRAP_CAP_Start);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap)) 
    {
    	BDBG_ASSERT(pAudioParams);
        BDBG_MSG(("\nBRAP_CAP_Start:"
            "\n\tpAudioParams->eBufDataMode = %d"
            "\n\tpAudioParams->eCapInputType = %d"
            "\n\tpAudioParams->eInputBitsPerSample = %d"
            "\n\tpAudioParams->eInputSamplingRate = %d"
            "\n\tpAudioParams->ePcmOutputMode = %d"
            "\n\tpAudioParams->sAudioOutputParams = %p"
            "\n\tpAudioParams->sDstChParams = %p"
            "\n\tpAudioParams->sSpdifChanStatusParams = %p"
            "\n\tpAudioParams->ui32RbufMemSize = %d"
            "\n\tpAudioParams->uiDelayRbuf = %d",
            pAudioParams->eBufDataMode,pAudioParams->eCapInputType,
            pAudioParams->eInputBitsPerSample,pAudioParams->eInputSamplingRate, 
            pAudioParams->ePcmOutputMode,pAudioParams->sAudioOutputParams,
            pAudioParams->sDstChParams,pAudioParams->sSpdifChanStatusParams,
            pAudioParams->ui32RbufMemSize,pAudioParams->uiDelayRbuf));

#if BRAP_UNSIGNED_PCM_SUPPORTED
            BDBG_MSG(("\n\tpAudioParams->bIsSigned",pAudioParams->bIsSigned));
#endif

        hRapCh->hInterruptCount->uiCrcError=0;
        hRapCh->hInterruptCount->uiDecoderUnlock=0;
        hRapCh->hInterruptCount->uiFmmCRC=0;
        hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark=0;
        hRapCh->hInterruptCount->uiPtsError=0;
        hRapCh->hInterruptCount->uiFramesInError=0;
		hRapCh->hInterruptCount->uiTotalFrameDecoded=0;


    	/* If channel already started return an error */
    	if (hRapCh->bStarted) 
        {
    		BDBG_ERR(("Channel already started. Exiting BRAP_CAP_Start() without taking any action."));
    		return BERR_SUCCESS;
    	}
    	else
	    {
    		hRapCh->bStarted = true;        
	    }        

	if (hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
	{
		/*Checking if the capture channel data is being tried to mix with compressed data*/
		ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
							hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort,
							false, /*As Capture channel can have inly PCM data*/
							&bIsCapMixedWithCompressed);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_CAP_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
			return BERR_TRACE (ret);
		}
		if(bIsCapMixedWithCompressed)
		{
		    ret = BERR_NOT_SUPPORTED;
			BDBG_ERR(("BRAP_CAP_Start: Trying to mix PCM data with Compressed Data"));
			goto end_start;
		}
	}

    	/* Buffer data mode provided during channel open should be greater 
    	   than or equal to the buffer data mode passed now */
    	if(pAudioParams->eBufDataMode > hRapCh->eMaxBufDataMode)
    	{
    		BDBG_ERR(("BRAP_CAP_Start: Buffer data mode  %d provided during channel "
    				  "open should be greater than or equal to the buffer data mode "
    				  "%d passed during channel start time", 
    				  hRapCh->eMaxBufDataMode, pAudioParams->eBufDataMode));
    		return BERR_TRACE(BERR_INVALID_PARAMETER);
    	}

    	if(pAudioParams->sDstChParams.eCapPort >= BRAP_CapInputPort_eMaxInputPorts)
    	{
    		BDBG_ERR(("BRAP_CAP_OpenChannel: Input port(%d) not supported",
    				  pAudioParams->sDstChParams.eCapPort));
    		return BERR_TRACE (BERR_NOT_SUPPORTED);
    	}

    	switch(pAudioParams->eCapInputType)
    	{
    		case BRAP_CapInputType_eExtI2s:
                /* For external capture, take the input bits/sample and sampling rate from user param */
                eIpBitsPerSample = pAudioParams->eInputBitsPerSample;
                eInputSR = pAudioParams->eInputSamplingRate;
    		  break;
                
    		case BRAP_CapInputType_eIntI2s0:
		  if ( hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->uiStartCnt > 0 )
		  {
                	/* For internal capture, override user's input bits/sample and the sampling rate with 
                   	the output bits/sample and the sampling rate of the source to this capture channel */
                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputSamplingRate(%d) by"
                          "Source to capture channel's output sampling rate(%d)",
                          pAudioParams->eInputSamplingRate,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->eSamplingRate));
                	eInputSR = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->eSamplingRate;

                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputBitsPerSample(%d) by"
                          "Source to capture channel's output bits per sample(%d)",
                          pAudioParams->eInputBitsPerSample,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->uOpParams.sI2s.uiBitsPerSample));
					switch(hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->uOpParams.sI2s.uiBitsPerSample)
					{
						case 8:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e8;
							break;
						case 16:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e16;
							break;
						case 18:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e18;
							break;
						case 20:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e20;
							break;
						case 24:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e24;
							break;
						case 32:
							eIpBitsPerSample=BRAP_InputBitsPerSample_e32;
							break;							
						default:
							BDBG_ERR(("Invalid input bits per sample for I2s %#x",
								hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->uOpParams.sI2s.uiBitsPerSample));
							return BERR_TRACE(BERR_INVALID_PARAMETER);
					}
		  }
		  else
		  {
                	eIpBitsPerSample = pAudioParams->eInputBitsPerSample;
                	eInputSR = pAudioParams->eInputSamplingRate;
		  }
		  break;
                
#if (BRAP_7401_FAMILY == 0)
    		case BRAP_CapInputType_eIntI2s1:
		  if ( hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->uiStartCnt > 0 )
		  {
                	/* For internal capture, override user's input bits/sample and the sampling rate with 
                   	the output bits/sample and the sampling rate of the source to this capture channel */
                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputSamplingRate(%d) by"
                          "Source to capture channel's output sampling rate(%d)",
                          pAudioParams->eInputSamplingRate,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->eSamplingRate));
                	eInputSR = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->eSamplingRate;

                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputBitsPerSample(%d) by"
                          "Source to capture channel's output bits per sample(%d)",
                          pAudioParams->eInputBitsPerSample,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->uOpParams.sI2s.uiBitsPerSample));
                	eIpBitsPerSample = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->uOpParams.sI2s.uiBitsPerSample;   
		  }
		  else
		  {
                	eIpBitsPerSample = pAudioParams->eInputBitsPerSample;
                	eInputSR = pAudioParams->eInputSamplingRate;
		  }
    		  break;
                
    		case BRAP_CapInputType_eIntI2s2:
		  if ( hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->uiStartCnt > 0 )				
		  {
                	/* For internal capture, override user's input bits/sample and the sampling rate with 
                   	the output bits/sample and the sampling rate of the source to this capture channel */
                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputSamplingRate(%d) by"
                          "Source to capture channel's output sampling rate(%d)",
                          pAudioParams->eInputSamplingRate,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->eSamplingRate));
                	eInputSR = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->eSamplingRate;

                	BDBG_WRN(("BRAP_CAP_Start: Overriding user passed eInputBitsPerSample(%d) by"
                          "Source to capture channel's output bits per sample(%d)",
                          pAudioParams->eInputBitsPerSample,
                          hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->uOpParams.sI2s.uiBitsPerSample));
                	eIpBitsPerSample = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->uOpParams.sI2s.uiBitsPerSample;   
		  }					
		  else
		  {
                	eIpBitsPerSample = pAudioParams->eInputBitsPerSample;
                	eInputSR = pAudioParams->eInputSamplingRate;
		  }
    		  break;
#endif
    		case BRAP_CapInputType_eIntFlex:
                /* TODO: This is an intenal capture type, so take eInputSR and eIpBitsPerSecond 
                   from the source's output as above for I2S */
    		default:
    			BDBG_ERR(("BRAP_CAP_OpenChannel: Unsupported capture input type."));
    			return BERR_TRACE(BERR_NOT_SUPPORTED);
    	}    
        hRapCh->eCapInputType = pAudioParams->eCapInputType;
        
        /* Note: This point onwards eInputSamplingRate and eInputBitsPerSample available in the 
                 pAudioParams should not be used. Instead, eInputSR and eIpBitsPerSample available 
                 locally should be used */

    	/*  Check if buffer size provided during starting the channel is less than or equal 
    		   to that given during channel open.
    		   Also, check if delay-in-bytes is with in the ring buffer boundary */ 
        if (hRapCh->eCapMode != BRAP_CaptureMode_eByPass)
        {		   
            if(hRapCh->sModuleHandles.hRBuf[0] != NULL)
            {
                if(pAudioParams->ui32RbufMemSize > hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiSize)
                {
    				BDBG_ERR(("BRAP_CAP_Start: Ring buffer size(%d) should be less than the max specified at open time %d", 
                    		pAudioParams->ui32RbufMemSize,
           					hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiSize));
    		
    		        return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
            else
            {
                /* This should never happen!!!  */
                BDBG_ERR(("No RBUFs allocated!!!"));
                BDBG_ASSERT(0);
            }

            /* RBuf parameters */
            ret = BRAP_P_ConvertDelayToStartWRPoint (
    	                                pAudioParams->uiDelayRbuf,
    									pAudioParams->eBufDataMode,
    									eInputSR, 
    								    eIpBitsPerSample,
    								    &uiStartWRPoint);
            if(BERR_SUCCESS != ret)
            {
                BDBG_ERR(("BRAP_CAP_Start: BRAP_P_ConvertDelayToStartWRPoint returned %d",ret));
                return BERR_TRACE(ret);
            }
            sRBufParams[0].uiStartWRPoint = uiStartWRPoint;
            
            /* note: delay ie StartWR point should be less than rbuf size */
            if(sRBufParams[0].uiStartWRPoint >= pAudioParams->ui32RbufMemSize)
            {
                BDBG_ERR(("BRAP_CAP_Start: Ring buffer start point(%d) ie delay should be less than %d", 
           			sRBufParams[0].uiStartWRPoint,
           			pAudioParams->ui32RbufMemSize));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        
            uiRbufSize[0] = pAudioParams->ui32RbufMemSize;

            if(pAudioParams->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
            {
                /* Stereo non-interleaved requires two separate buffers with same size and delay */       
                sRBufParams[1].uiStartWRPoint = sRBufParams[0].uiStartWRPoint;
            	uiRbufSize[1] = pAudioParams->ui32RbufMemSize;
            }
            else
            {
                /* Stereo interleaved or mono requires single left buffer. So 0 size and 0 delay for other */       
            	uiRbufSize[1] = 0;
              	sRBufParams[1].uiStartWRPoint = 0;
            }
        }
        
        if (hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
        {
           	/* Validate if ePcmOutputMode and eBufDataMode compatible */
        	if(bValidatePcmOpNDataModes[pAudioParams->ePcmOutputMode][pAudioParams->eBufDataMode] == false)
        	{
        		BDBG_ERR(("BRAP_CAP_Start: ePcmOutputMode(%d) is not compatible with eBufDataMode(%d)",
        			pAudioParams->ePcmOutputMode, pAudioParams->eBufDataMode));
        		return BERR_TRACE(BERR_NOT_SUPPORTED);
        	}
        
    		/* If output path is included, check for supported output bit resolutions. */
    	    /* For I2S and Flex output, it can be between 16 bits/sample to 32 bits/sample,
    	      For other outputs, it can be between 16 bits/sample to 24 bits/sample */
    		if ((pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_I2S_MAX_BITS_PER_SAMPLE) ||
    			(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample < BRAP_P_MIN_BITS_PER_SAMPLE))
    		{
    			BDBG_ERR(("BRAP_CAP_Start: Output bit resolution %d should be between 16 to 32 bits/sample", 
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
    				BDBG_ERR(("BRAP_CAP_Start: Output bit resolution %d should be between 16 to 24 bits/sample", 
    						   pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
    				return BERR_TRACE(BERR_NOT_SUPPORTED);
    			}
    		}
    	}
    }/* if !watchdog */

	switch(hRapCh->eCapInputType)
	{
		case BRAP_CapInputType_eExtI2s:
			break;
		case BRAP_CapInputType_eIntI2s0:
			if(hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0] == NULL)
			{
				BDBG_ERR(("BRAP_CAP_Start: No RAP Channel is opened for output port "
						  "BRAP_CapInputType_eIntI2s0. Please open such a channel and "
						  "restart the Capture channel again."));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			pOpI2sSettings = 
               &(hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eI2s0].uOutputPortSettings.sI2sSettings);

			break;
            
#if (BRAP_7401_FAMILY == 0)
		case BRAP_CapInputType_eIntI2s1:
			if(hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1] == NULL)
			{
				BDBG_ERR(("BRAP_CAP_Start: No RAP Channel is opened for output port "
						  "BRAP_CapInputType_eIntI2s1. Please open such a channel and "
						  "restart the Capture channel again."));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			pOpI2sSettings = 
               &(hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eI2s1].uOutputPortSettings.sI2sSettings);
			break;
            
		case BRAP_CapInputType_eIntI2s2:
			if(hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2] == NULL)
			{
				BDBG_ERR(("BRAP_CAP_Start: No RAP Channel is opened for output port "
						  "BRAP_CapInputType_eIntI2s2. Please open such a channel and "
						  "restart the Capture channel again."));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			pOpI2sSettings = 
               &(hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eI2s2].uOutputPortSettings.sI2sSettings);
            break;
            
#endif
		case BRAP_CapInputType_eIntFlex:
		default:
			BDBG_ERR(("BRAP_CAP_Start: Unsupported capture input type."));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/* Start the internal objects */
	if (!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap)) 
	{
		/* Form internal FMM module params */
		if (hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
		{
			/* SPDIF formatter Parameters */

			/* Get the default parameters first */
			ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
			if(ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret); 
                goto end_start;
            } 
			
			/* Fill up the required parameters */
			sSpdifFmParams.sExtParams = pAudioParams->sAudioOutputParams.sSpdifFmParams;
			sSpdifFmParams.bCompressed = false; /* Always PCM data */
			sSpdifFmParams.bSeparateLRChanNum = 
						pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum;
                     sSpdifFmParams.sChanStatusParams = pAudioParams->sSpdifChanStatusParams;   

			/* Mixer parameters */
			
			/* Get the default parameters first */
			ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
			if(ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret); 
                goto end_start;
            } 
			
			/* Fill up the required parameters */
			sMixerParams.sExtParams = pAudioParams->sAudioOutputParams.sMixerParams;
			sMixerParams.bCompress = false; /* Always PCM data */

			/* Mixer output stream resolution can maximum be 24 bits/sample. 
			   If output stream resolution required is more than 24 bits/sample, 
			   set mixer output stream resolution to be 24 bits/sample and output has 
			   to pad the remaining bits. Only for I2S output more than 24 bits/sample 
				is supported. */
			if(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > 
					BRAP_P_MIXER_MAX_BITS_PER_SAMPLE)
				sMixerParams.uiStreamRes = BRAP_P_MIXER_MAX_BITS_PER_SAMPLE;
			else
				sMixerParams.uiStreamRes 
					= pAudioParams->sAudioOutputParams.uiOutputBitsPerSample;

			sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiSrcChId;

			/* Source channel parameters */
			ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
			if(ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret); 
                goto end_start;
            } 
			
			/* Fill up the required parameters */
           	sSrcChParams.eLRDataCntl = lrDataCtrl[pAudioParams->ePcmOutputMode];            
			sSrcChParams.bCompress = false; /* Always PCM data */

			/* Start playing when write pointer crosses SOURCE_RINGBUF_x_START_WRPOINT */
			sSrcChParams.bStartSelection = true; /* TODO: Check if there can be any delay from rbuf */
			sSrcChParams.bInputSrValid = false; /* Input audio data is not coming from DSP */

            /* In Bypass mode, data is written directly into SrcFIFO so set bCapDirect2SrcFIFO to true */
            if(hRapCh->eCapMode == BRAP_CaptureMode_eByPass)
                sSrcChParams.bCapDirect2SrcFIFO = true;
            else
                sSrcChParams.bCapDirect2SrcFIFO = false;
            
			switch(eIpBitsPerSample)
			{
				case BRAP_InputBitsPerSample_e16:
					sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
					break;
				case BRAP_InputBitsPerSample_e18:
					/* LS bits of the samples need to be padded with zeros to make 32 bits/sample */
				case BRAP_InputBitsPerSample_e20:
					/* LS bits of the samples need to be padded with zeros to make 32 bits/sample */
				case BRAP_InputBitsPerSample_e24:
					/* LS bits of the samples need to be padded with zeros to make 32 bits/sample */
				case BRAP_InputBitsPerSample_e32:
					sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;
					break;
				default:
					BDBG_ERR(("BRAP_CAP_Start: Input bit persample %d is not supported", 
						       eIpBitsPerSample));
					return BERR_TRACE(BERR_NOT_SUPPORTED);
			}

			sSrcChParams.bSharedRbuf = false;
			sSrcChParams.eBufDataMode = pAudioParams->eBufDataMode;

#if BRAP_UNSIGNED_PCM_SUPPORTED
            sSrcChParams.bIsSigned = pAudioParams->bIsSigned;
#endif

    		sSrcChParams.eInputSR = eInputSR;
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

			/* Ring buffers to be mapped to this source channel */
            if(pAudioParams->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
            {
                sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[0].uiRbufId[0];
    			sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[0].uiRbufId[1];
            }
            else
            {
                /* if interleaved/mono, just left id should be assigned */
                sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[0].uiRbufId[0];
    			sSrcChParams.uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
            }
            BDBG_MSG(("BRAP_CAP_Start: sSrcChParams ltRbufid, rtRbufid = (%d, %d)",
                sSrcChParams.uiLtOrSnglRBufId, sSrcChParams.uiRtRBufId));

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

			/* For debug purpose */
			BDBG_ASSERT(hRapCh->sModuleHandles.hSrcCh[eChanPair]);
			BDBG_ASSERT(hRapCh->sModuleHandles.hOp[eChanPair]);
			BDBG_ASSERT(hRapCh->sModuleHandles.hMixer[eChanPair]);
			if((hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort == BRAP_OutputPort_eSpdif) 
				&& (hRapCh->sModuleHandles.hSpdifFm[eChanPair] == NULL))
			{
				BDBG_ASSERT(0);
			}
			if((hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort == BRAP_OutputPort_eMai) && 
			   (hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector 
							== BRAP_OutputPort_eSpdif) && 
			   (hRapCh->sModuleHandles.hSpdifFm[eChanPair] == NULL))
			{
				BDBG_ASSERT(0);
			}
		}

		/* Destination channel params */
        /* Get the default parameters first */
		ret = BRAP_DSTCH_P_GetDefaultParams (&sDstChParams);  
        if(ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_CAP_Start: BRAP_DSTCH_P_GetDefaultParams() returned err(%d)",ret));
            ret = BERR_TRACE(ret);
    		goto end_start;
    	}
        
		sDstChParams.sExtParams = pAudioParams->sDstChParams;
		sDstChParams.eCapMode = hRapCh->eCapMode;
		sDstChParams.eBufDataMode = pAudioParams->eBufDataMode;
		sDstChParams.eCapInputType = hRapCh->eCapInputType;
        /* Note: If bNotPauseWhenFull is true, BF interrupts(e.g. full/free 
                 watermark) don't come. So, set it to false. */
        sDstChParams.bNotPauseWhenFull = false;
		sDstChParams.eInputBitsPerSample = eIpBitsPerSample;

        if(pAudioParams->eBufDataMode == BRAP_BufDataMode_eStereoNoninterleaved)
        {
    		sDstChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[0];
    		sDstChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[1];
        }
        else
        {
            /* If interleaved/mono, only left rbuf id should be valid */
    		sDstChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[0];
    		sDstChParams.uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
        }
		sDstChParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[eChanPair].uiSrcChId;
        BDBG_MSG(("BRAP_CAP_Start: sDstChParams ltRbufid, rtRbufid = (%d, %d)",
            sDstChParams.uiLtOrSnglRBufId, sDstChParams.uiRtRBufId));
		
		/* Override the input I2S params for loopback case, 
		   if they are different from the output one */
		if(hRapCh->eCapInputType !=  BRAP_CapInputType_eExtI2s)
		{
			if(pOpI2sSettings->bLsbAtLRClk != pAudioParams->sDstChParams.sInputI2sParams.bLsbAtLRClk)
				sDstChParams.sExtParams.sInputI2sParams.bLsbAtLRClk = pOpI2sSettings->bLsbAtLRClk;
			if(pOpI2sSettings->bAlignedToLRClk != pAudioParams->sDstChParams.sInputI2sParams.bAlignedToLRClk)
				sDstChParams.sExtParams.sInputI2sParams.bAlignedToLRClk = pOpI2sSettings->bAlignedToLRClk;
			if(pOpI2sSettings->bLRClkPolarity != pAudioParams->sDstChParams.sInputI2sParams.bLRClkPolarity)
				sDstChParams.sExtParams.sInputI2sParams.bLRClkPolarity = pOpI2sSettings->bLRClkPolarity;
		}

		/* For debug purpose only */
		BDBG_ASSERT(hRapCh->sModuleHandles.hDstCh);

	} /* !BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap) */

	/* Start the Output path FMM modules if not captureOnly mode */
    if(hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
    {
    	ret = BRAP_P_StartOpPathFmmModules(
    							hRapCh,
    							&sRBufParams[0],
    							&sSrcChParams,
    							&sMixerParams,
    							&sSpdifFmParams,
    							pOpParams,
    							(BRAP_OutputChannelPair)0,
    							&(hRapCh->sModuleHandles));
        BDBG_MSG(("BRAP_CAP_Start: Output path modules started.."));
    	if(ret != BERR_SUCCESS){ret = BERR_TRACE(ret);goto end_start;}

    	/* Set output sampling rate and program SRC */
        ret = BRAP_OP_P_SetSamplingRate ( 
                                hRapCh->hRap,
                                hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort,
                                hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate,/* output SR */ 
            	                true, /* bOverride*/
            	                false /* bCalledFromISR */);
    	if(ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_CAP_Start: BRAP_OP_P_SetSamplingRate() returned err(%d)",ret));
            ret = BERR_TRACE(ret);
    		goto end_start;
    	}
    	BDBG_MSG(("BRAP_CAP_Start: SetSamplingRate done. outputport=%d, eSamplingRate=%d!", 
             hRapCh->sModuleHandles.hOp[eChanPair]->eOutputPort, 
             hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate));
    }/* if !CaptureOnly */

#ifndef BCHP_7411_VER
    /* Start all Cloned ports associated with this channel. note: there's only 
    1 channel pair */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {    
            if ((hRapCh->sCloneOpPathHandles[eChanPair][j].hOp != NULL)
                && (hRapCh->sCloneOpPathHandles[eChanPair][j].bSimul == false))
            {      
                BDBG_MSG(("BRAP_CAP_Start: output port %d was cloned for channel pair %d", j, eChanPair));
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

                        #if (BCHP_CHIP == 7400)
                        /* We need to store the RBUF ID's, as in 7400 the Cloned 
                           channel uses separate RBUF's allocated to it.*/
                            sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[0];
                            sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[eChanPair][j].uiRbufId[1];
                        #endif

                        /* this Src Ch shares the rbufs with the Orig SrCh */
                        sSrcChParams.bSharedRbuf = true;
                        sSrcChParams.uiMasterSrcCh = 
                                        hRapCh->sModuleHandles.hSrcCh[eChanPair]->uiIndex;

                        sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[eChanPair][j].uiSrcChId ;     
                        BDBG_MSG(("BRAP_CAP_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", 
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
						
                        BDBG_MSG(("BRAP_CAP_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
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
        													j,
        													false, /*As Capture channel can have only PCM data*/
        													&bIsCapMixedWithCompressed);
        		if(ret != BERR_SUCCESS)
        		{
        			BDBG_ERR(("BRAP_CAP_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
        			return BERR_TRACE (ret);
        		}

        		if(bIsCapMixedWithCompressed)
        		{
            		ret = BERR_NOT_SUPPORTED;
        			BDBG_ERR(("BRAP_CAP_Start: Trying to mix PCM data with Compressed Data"));
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
                    BDBG_ERR(("BRAP_CAP_Start: StartOpPathFmmMod failed for clone port %d",j));
                    ret = BERR_TRACE(ret);
                    goto end_start;
                }

                BDBG_MSG(("BRAP_CAP_Start: Cloned resources started"));

                 /* Set output sampling rate and program SRC for cloned port*/
                 ret = BRAP_OP_P_SetSamplingRate ( 
                                                    hRapCh->hRap,
                                                    (BRAP_OutputPort)j,
                                                    hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate,/* output SR */ 
                                	                true, /* bOverride*/
                                  	                false /* bCalledFromISR */);
            	if(ret != BERR_SUCCESS)
            	{
            		BDBG_ERR(("BRAP_CAP_Start: BRAP_OP_P_SetSamplingRate() failed for cloned port %d", j));
                    ret = BERR_TRACE(ret);
                    goto end_start;
            	}
            	BDBG_MSG(("BRAP_CAP_Start: SetSamplingRate done. outputport=%d, eSamplingRate=%d!", 
                     j, hRapCh->sModuleHandles.hOp[eChanPair]->eSamplingRate));
            }/* if hOp != NULL */            
        }/* for j */   
#endif

	if(!BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap))
	{
		/* For capOnly mode, start rbufs associated with the capture path */
		if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
		{
			/* note: the rbuf handle for Cap channel will be in sModuleHandles.hRBuf[0]
			and sModuleHandles.hRBuf[1] */
			if(sDstChParams.uiLtOrSnglRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{       
				BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[0], &(sRBufParams[0]));
			}                       

			if(sDstChParams.uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{       
				BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[1], &(sRBufParams[1]));
			}                       
		}

		if(hRapCh->eCapMode == BRAP_CaptureMode_eFullDuplex)
		{
			/* note: the rbuf handle associated with destination fifo will be in sModuleHandles.hRBuf[2]
			and sModuleHandles.hRBuf[3] */
			if(sDstChParams.uiLtOrSnglRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{       
				BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[2], &(sRBufParams[0]));
			}                       

			if(sDstChParams.uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
			{       
				BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[3], &(sRBufParams[1]));
			}                       
		}

		/* Start the destination channels */            
		ret = BRAP_DSTCH_P_Start(hRapCh->sModuleHandles.hDstCh, &sDstChParams);
	    BDBG_MSG(("BRAP_CAP_Start: destination channels started.."));
		if(ret != BERR_SUCCESS)
		{
	        ret = BERR_TRACE(ret);
			/* Stop the FMM modules started already */
			ret |= BRAP_P_StopOpPathFmmModules((BRAP_OutputChannelPair)0, &(hRapCh->sModuleHandles));
	        ret = BERR_TRACE(ret);
	        goto end_start;
		}
	}
	else
	{
		if(hRapCh->sModuleHandles.hRBuf[0])
		{
			BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[0], NULL);
		}
		
		if(hRapCh->sModuleHandles.hRBuf[1])
		{
			BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[1], NULL);
		}
		
		if(hRapCh->sModuleHandles.hRBuf[2])
		{
			BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[2], NULL);
		}
		
		if(hRapCh->sModuleHandles.hRBuf[3])
		{
			BRAP_RBUF_P_Start(hRapCh->sModuleHandles.hRBuf[3], NULL);
		}

		if(hRapCh->sModuleHandles.hDstCh)
		{
			BRAP_DSTCH_P_Start(hRapCh->sModuleHandles.hDstCh, NULL);
		}
	}



	/* Program the Serial In Register */
	ret = BRAP_CAP_P_InputConfig ( hRapCh, hRapCh->eCapInputType , 
			hRapCh->sModuleHandles.hDstCh->sParams.sExtParams.eCapPort);
	if(ret != BERR_SUCCESS){ret = BERR_TRACE(ret);goto end_start;} 

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Unmask the Full Mark interrupt */
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        ret = BRAP_P_UnmaskInterrupt(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
    	if(ret != BERR_SUCCESS)
    	{
        	ret = BERR_TRACE(ret);
    	    BDBG_ERR(("BRAP_CAP_Start: BRAP_P_UnmaskInterrupt() returned err(%d)",ret));	
        }            
    }
#endif
#else
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
    	ret = BTMR_StartTimer(hRapCh->hTimer, 1000);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_CAP_Start: BTMR_StartTimer() returned err(%d)",ret));
		ret = BERR_TRACE(ret);
		goto end_start;
	}
	BDBG_MSG(("BRAP_CAP_Start: Start timer done!"));
    }
#endif

#ifndef BCHP_7411_VER
    if (hRapCh->eCapMode != BRAP_CaptureMode_eCaptureOnly)
    {
        BRAP_P_ProgramAdaptRateCtrl (hRapCh);
        BDBG_MSG(("Programmed adaptive rate control for the playback path of the capture channel"));
    }
#endif    
    
end_start:
/* TODO: stop resources if any were started*/
	if(ret != BERR_SUCCESS)
	{
		/* Returning on error. Restore back channel state. */
		hRapCh->bStarted = false;
    }
    
	BDBG_LEAVE (BRAP_CAP_Start);
    return ret;
}

/***************************************************************************
Summary:
	Stops a PCM Capture channel.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_CAP_Start
****************************************************************************/
BERR_Code BRAP_CAP_Stop ( 
        BRAP_ChannelHandle     hRapCh    /* [in] The Raptor Capture Channel handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
	BRAP_OutputChannelPair eChanPair = BRAP_OutputChannelPair_eLR;
#ifndef BCHP_7411_VER /* not for 7411 */   
    unsigned int j;
    BRAP_P_ObjectHandles    	sTempHandles;       
#endif

    BDBG_ASSERT (hRapCh);
    BDBG_ENTER (BRAP_CAP_Stop);

	if (hRapCh->bStarted == false) {
		BDBG_ERR(("Channel is already stopped. Exiting BRAP_PB_Stop without taking any action."));
		return BERR_SUCCESS;
	}

    hRapCh->hInterruptCount->uiCrcError=0;
    hRapCh->hInterruptCount->uiDecoderUnlock=0;
    hRapCh->hInterruptCount->uiFmmCRC=0;
    hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark=0;
    hRapCh->hInterruptCount->uiPtsError=0;


#ifndef BCHP_7411_VER
	/* TODO : Stop the cloned handles */
#endif

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
    /* Mask the Free Mark interrupt */
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        ret = BRAP_P_MaskInterrupt(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
    	if(ret != BERR_SUCCESS)
    	{
            ret = BERR_TRACE(ret);
    	    BDBG_ERR(("BRAP_CAP_Stop: BRAP_P_MaskInterrupt() returned err(%d). Ignoring err!!",ret));	
        }
    }
#endif
#endif

#if BRAP_P_WATERMARK_WORKAROUND==1
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
    	ret = BTMR_StopTimer(hRapCh->hTimer);
    	if ( ret != BERR_SUCCESS )
    	{
    		BDBG_ERR(("BRAP_CAP_Stop: BTMR_StopTimer() returned err(%d)",ret));	
    		return BERR_TRACE(ret);
    	}
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
    
	/* Stop the main FMM modules associated with the channel */
	ret = BRAP_P_StopOpPathFmmModules(eChanPair, &(hRapCh->sModuleHandles));
    if (ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);    
		BDBG_ERR (("BRAP_CAP_Stop: call to BRAP_P_StopOpPathFmmModules() failed. Ignoring error!!!!!"));
    }
	BDBG_MSG(("BRAP_CAP_Stop: Output path FMM modules stoped!"));

	if(hRapCh->eCapMode == BRAP_CaptureMode_eFullDuplex)
	{
		/* note: the rbuf handle associated with destination fifo will be in sModuleHandles.hRBuf[2]
		and sModuleHandles.hRBuf[3] */
		if(hRapCh->sModuleHandles.hRBuf[2] != NULL)
		{       
			BRAP_RBUF_P_Stop(hRapCh->sModuleHandles.hRBuf[2]);
			BDBG_MSG(("BRAP_CAP_Stop: Stopped ring buffer[%d]!", hRapCh->sModuleHandles.hRBuf[2]->uiIndex));
		}                       

		if(hRapCh->sModuleHandles.hRBuf[3] != NULL)
		{       
			BRAP_RBUF_P_Stop(hRapCh->sModuleHandles.hRBuf[3]);
			BDBG_MSG(("BRAP_CAP_Stop: Stopped ring buffer[%d]!", hRapCh->sModuleHandles.hRBuf[3]->uiIndex));
		}                       
	}
	
	/* Stop the Destination Channel */
	ret = BRAP_DSTCH_P_Stop (hRapCh->sModuleHandles.hDstCh);
	if (ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        BDBG_ERR (("BRAP_CAP_Stop: Destination channel stop failed. Ignoring error!!!!!"));
    }
	BDBG_MSG(("BRAP_CAP_Stop: Destination channel stoped!"));

	hRapCh->bStarted = false;
    
    BDBG_LEAVE (BRAP_CAP_Stop);
    return ret;
}

/***************************************************************************
Summary:
    This API changes the input sampling rate for external I2S input only. 
    For the internal loopback (when one of the I2S output is fed back to the 
    capture input), the I2S output sampling rate will override it.
Returns:
	BERR_SUCCESS 
See Also:
    BRAP_CAP_GetInputSamplingRate
****************************************************************************/
BERR_Code BRAP_CAP_SetInputSamplingRate ( 
    BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
    BAVC_AudioSamplingRate eInputSamplingRate /* [in] Sampling rate of PCM Input */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_OP_P_Handle hOp = NULL;
    BRAP_MIXER_P_Handle hMixer = NULL;
    BRAP_SRCCH_P_Handle hSrcCh = NULL;
#ifndef BCHP_7411_VER /* For chips other than 7411 */    
    unsigned int j = 0;
#endif
    BDBG_ENTER(BRAP_CAP_SetInputSamplingRate);
    BDBG_ASSERT(hRapCh);

    if(hRapCh->eChannelType != BRAP_P_ChannelType_eCapture)
    {
        BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: eChannelType(%d) is not a capture channel",
            hRapCh->eChannelType));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    
    if(hRapCh->eCapInputType != BRAP_CapInputType_eExtI2s)
    {
        BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: eCapInputType(%d) is not an external capture input",
            hRapCh->eCapInputType));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* InputSR is not required for captureOnly mode */
    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: Capture Only mode doesn't require sampling rate"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* We need to change input SR for ALL output ports/ SrcCh associated with this channel.
    ie master as well as cloned ports */

    /* 1. First handle main ports */
    /* Get reqd handles for master port */
    hOp = hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR];
    hMixer = hRapCh->sModuleHandles.hMixer[BRAP_OutputChannelPair_eLR];
    hSrcCh = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR];    
    if ((hOp == NULL) || (hMixer == NULL) || (hSrcCh == NULL))
    {
        BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: no output port associated with this channel. hOp/hSrcCh/hMixer uninitialised."));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }
   	   
    ret |= BRAP_OP_P_EnableStream(hOp, false);	/* Disable output stream */   
    ret |= BRAP_MIXER_P_EnableMixer(hMixer, false); /* Disable mixer */
    ret |= BRAP_MIXER_P_SetSRC(hMixer, hSrcCh, hOp->eSamplingRate, eInputSamplingRate); /* Program the SRC */
    ret |= BRAP_MIXER_P_EnableMixer(hMixer, true); /* Enable mixer */    
    ret |= BRAP_OP_P_EnableStream(hOp, true);	/* Enable stream */    

    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: eInputSamplingRate could not be updated for output port %d of channel 0x%h", 
            hOp->eOutputPort, hRapCh));
        return BERR_TRACE(ret);
    }
    else
    {
        hSrcCh->sParams.eInputSR = eInputSamplingRate;
    }

    /* 2. Now check if this channel pair has been cloned to any other ports and handle them */
#ifndef BCHP_7411_VER /* For chips other than 7411 */
    for(j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
    {
        hOp = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;
        if ((hOp != NULL) && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == false))
        {
            hMixer = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
            hSrcCh = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;    
            if (hSrcCh == NULL)
            {
                hSrcCh = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]; 
            }
            
            if ((hMixer == NULL) || (hSrcCh == NULL))
            {
                BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: no cloned output port associated with this channel. hSrcCh/hMixer uninitialised."));
                return BERR_TRACE(BERR_NOT_INITIALIZED);
            }
           	   
            ret |= BRAP_OP_P_EnableStream(hOp, false);	/* Disable output stream */   
            ret |= BRAP_MIXER_P_EnableMixer(hMixer, false); /* Disable mixer */
            ret |= BRAP_MIXER_P_SetSRC(hMixer, hSrcCh, hOp->eSamplingRate, eInputSamplingRate); /* Program the SRC */
            ret |= BRAP_MIXER_P_EnableMixer(hMixer, true); /* Enable mixer */    
            ret |= BRAP_OP_P_EnableStream(hOp, true);	/* Enable stream */    

            if(ret != BERR_SUCCESS)
            {
                BDBG_ERR(("BRAP_CAP_SetInputSamplingRate: eInputSamplingRate could not be updated for cloned output port %d of channel 0x%h", 
                    hOp->eOutputPort, hRapCh));
                return BERR_TRACE(ret);
            }
            else
            {
                hSrcCh->sParams.eInputSR = eInputSamplingRate;
            }                
         }
    }/* for */
#endif    
        
    BDBG_LEAVE(BRAP_CAP_SetInputSamplingRate);
    return ret;
}

BERR_Code BRAP_CAP_GetInputSamplingRate ( 
        BRAP_ChannelHandle     hRapCh,    /* [in] The Raptor Capture Channel handle */
        BAVC_AudioSamplingRate    *pInputSamplingRate /* [out] Sampling rate of PCM Input */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER (BRAP_CAP_GetInputSamplingRate);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pInputSamplingRate);

    if(hRapCh->eCapMode == BRAP_CaptureMode_eCaptureOnly)
    {
        BDBG_ERR(("BRAP_CAP_GetInputSamplingRate: Capture Only mode doesn't require sampling rate"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if(NULL == hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR])
    {
        BDBG_ERR(("BRAP_CAP_GetInputSamplingRate: hSrcCh is NULL"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }
                   
    *pInputSamplingRate = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams.eInputSR;

    BDBG_LEAVE (BRAP_CAP_GetInputSamplingRate);
    return ret;
}



static BERR_Code BRAP_CAP_P_InputConfig ( 
        BRAP_ChannelHandle  hRapCh,    /* [in] The Raptor Capture Channel handle */
        BRAP_CapInputType	eCapInputType,
		BRAP_CapInputPort	eCapInput
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal;
    BREG_Handle hRegister;

	BDBG_ASSERT (hRapCh);
    BDBG_ENTER (BRAP_CAP_P_InputConfig);

    BDBG_MSG(("BRAP_CAP_P_InputConfig: eCapInType=%d eCapInput=%d",eCapInputType, eCapInput));

	hRegister = hRapCh->hRegister;
	ui32RegVal = BRAP_Read32 (hRegister, BCHP_AUD_FMM_MISC_SERIN_SEL);
	switch(eCapInput)
	{
		case BRAP_CapInputPort_eInput0:
			switch(eCapInputType)
			{
				case BRAP_CapInputType_eExtI2s:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE0));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE0, Ext_i2s0_input));
					break;
				case BRAP_CapInputType_eIntI2s0:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE0));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE0, fmmop_serout0));
					break;
#if (BRAP_7401_FAMILY == 0)
				case BRAP_CapInputType_eIntI2s1:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE0));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE0, fmmop_serout1));
					break;
				case BRAP_CapInputType_eIntI2s2:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE0));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE0, fmmop_serout2));
					break;
#endif
				case BRAP_CapInputType_eIntFlex:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE0));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE0, fmmop_serout3));
					break;
				default:
					BDBG_ERR(("BRAP_CAP_Start: Unsupported capture port input type."));
					return BERR_TRACE(BERR_NOT_SUPPORTED);
			}
			break;
#if (BRAP_7401_FAMILY == 0)
		case BRAP_CapInputPort_eInput1:
			switch(hRapCh->eCapInputType)
			{
				case BRAP_CapInputType_eExtI2s:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE1));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE1, Ext_i2s1_input));
					break;
				case BRAP_CapInputType_eIntI2s0:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE1));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE1, fmmop_serout0));
					break;
				case BRAP_CapInputType_eIntI2s1:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE1));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE1, fmmop_serout1));
					break;
				case BRAP_CapInputType_eIntI2s2:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE1));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE1, fmmop_serout2));
					break;
				case BRAP_CapInputType_eIntFlex:
					ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SERIN_SEL, SOURCE1));
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SERIN_SEL, SOURCE1, fmmop_serout3));
					break;
				default:
					BDBG_ERR(("BRAP_CAP_Start: Unsupported capture port input type."));
					return BERR_TRACE(BERR_NOT_SUPPORTED);
			}
			break;
#endif
		default:
			BDBG_ERR(("BRAP_CAP_Start: Unsupported capture input port."));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	
	BRAP_Write32 (hRegister, BCHP_AUD_FMM_MISC_SERIN_SEL, ui32RegVal);

    BDBG_LEAVE (BRAP_CAP_P_InputConfig);
    return ret;
}

/***************************************************************************
Summary:	
	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. 
	Note: BRAP_PcmBufInfo has left and right ring buffer info structure. Each 
	      info structure has both pWritePtr and pReadPtr elements. This routine 
	      returns pWritePtr = NULL and pReadPtr = valid address (if any) as an
	      application requires only a readPtr to copy captured PCM data	from 
	      ring buffer to some local memory.
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_UpdateBufUsg
****************************************************************************/
BERR_Code BRAP_CAP_GetBufInfo(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_CAP_GetBufInfo);

    BKNI_EnterCriticalSection();
    BRAP_CAP_GetBufInfo_isr(hRapCh, psPcmBufInfo);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_CAP_GetBufInfo);
    return ret;
}


/***************************************************************************
Summary:	Gets the associated ring buffers's information.
Description:
	This API returns the associated ring buffers's information. This is an 
	ISR version of BRAP_CAP_GetBufInfo(). 
Returns:
	BERR_SUCCESS
See Also:
****************************************************************************/
BERR_Code BRAP_CAP_GetBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,		   /* [in] Audio channel handle */
	BRAP_PcmBufInfo		*psPcmBufInfo  /* [Out] Associated Pcm buffer 
													 information */		
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtRBufId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
		
	BDBG_ENTER(BRAP_CAP_GetBufInfo_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufInfo);

	BKNI_Memset(psPcmBufInfo, 0, sizeof(BRAP_PcmBufInfo));

	BDBG_MSG(("BRAP_CAP_GetBufInfo_isr: hRapCh=%d psPcmBufInfo=%d",hRapCh, psPcmBufInfo));
	
	/* Get the valid rbuf ids */
	uiLtRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[0]; 
	uiRtRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[1]; 	

	if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_CAP_P_PrepBufInfo_isr(hRapCh, 0/* left rbuf */, &(psPcmBufInfo->sLtOrSnglBufInfo));
		if(ret != BERR_SUCCESS)
		{
            BDBG_ERR(("BRAP_CAP_GetBufInfo_isr: Couldn't get left ring buffer[id - %d]info",
				uiLtRBufId));			
            return BERR_TRACE(ret);
		}
	}

	if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_CAP_P_PrepBufInfo_isr(hRapCh, 1/* right rbuf */, &(psPcmBufInfo->sRtBufInfo));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_CAP_GetBufInfo_isr: Couldn't get right ring buffer[id - %d]info",
				uiRtRBufId));			
            return BERR_TRACE(ret);
		}
	}

	BDBG_MSG(("BRAP_CAP_GetBufInfo_isr: OutParam:\n"
			"\tsLtOrSnglBufInfo(B=%p E=%p W=%p R=%p C=%d T=%d)\n"
			"\tsRtBufInfo(B=%p E=%p W=%p R=%p C=%d T=%d",
			psPcmBufInfo->sLtOrSnglBufInfo.pBasePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pEndPtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pWritePtr,
			psPcmBufInfo->sLtOrSnglBufInfo.pReadPtr,
			psPcmBufInfo->sLtOrSnglBufInfo.uiContiguousSize,
			psPcmBufInfo->sLtOrSnglBufInfo.uiTotalSize,
			psPcmBufInfo->sRtBufInfo.pBasePtr,
			psPcmBufInfo->sRtBufInfo.pEndPtr,
			psPcmBufInfo->sRtBufInfo.pWritePtr,
			psPcmBufInfo->sRtBufInfo.pReadPtr,
			psPcmBufInfo->sRtBufInfo.uiContiguousSize,
			psPcmBufInfo->sRtBufInfo.uiTotalSize));
			
	BDBG_LEAVE(BRAP_CAP_GetBufInfo_isr);
	return ret;
}

/***************************************************************************
Summary:	
	Updates the read pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	After reading PCM data from the ring buffer, an application can use this 
	routine to update the read pointer associated to that ring buffer. 
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_GetBufInfo
****************************************************************************/
BERR_Code BRAP_CAP_UpdateBufUsg( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer read 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_CAP_UpdateBufUsg);

    BKNI_EnterCriticalSection();
    ret = BRAP_CAP_UpdateBufUsg_isr(hRapCh, psPcmBufSzUpdt);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_CAP_UpdateBufUsg);
    return ret;
}

/***************************************************************************
Summary:	
	Updates the read pointers for the associated ring buffers as per the 
	ring buffer usage provided.
Description:
	After reading PCM data from the ring buffer, an application can use this 
	routine to update the read pointer associated to that ring buffer. 
	This is an ISR version of BRAP_CAP_UpdateBufUsg().
Returns:
	BERR_SUCCESS
See Also:
	BRAP_CAP_GetBufInfo
****************************************************************************/
BERR_Code BRAP_CAP_UpdateBufUsg_isr( 
	BRAP_ChannelHandle 	hRapCh,			/* [in] Audio channel handle */
    const BRAP_PcmBufSzUpdt	*psPcmBufSzUpdt	/* [in] Ring buffer usage 
    											   information */
    )
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiLtRBufId = BRAP_RM_P_INVALID_INDEX;
	unsigned int uiRtRBufId = BRAP_RM_P_INVALID_INDEX;
		
	BDBG_ENTER(BRAP_CAP_UpdateBufUsg_isr);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psPcmBufSzUpdt);

	BDBG_MSG(("BRAP_CAP_UpdateBufUsg_isr: hRapCh=%d"
			  	"\tpsPcmBufSzUpdt->uiLtOrSnglBufSzUpdt=%d"
			  	"\tpsPcmBufSzUpdt->uiRtBufSzUpdt=%d",hRapCh,
			  	psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt,
			  	psPcmBufSzUpdt->uiRtBufSzUpdt));
			  
	/* Get the valid rbuf ids */
	uiLtRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[0]; 
	uiRtRBufId = hRapCh->sRsrcGrnt.sCapResrcId.uiRbufId[1]; 	

	if(uiLtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_CAP_P_WriteBufUsg_isr(hRapCh, 0 , psPcmBufSzUpdt->uiLtOrSnglBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_CAP_UpdateBufUsg_isr: Couldn't update left ring buffer[0] usage"));			
			return BERR_TRACE(ret);	
		}
	}

	if(uiRtRBufId != (unsigned int)BRAP_RM_P_INVALID_INDEX)
	{
		ret = BRAP_CAP_P_WriteBufUsg_isr(hRapCh, 1, psPcmBufSzUpdt->uiRtBufSzUpdt);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_CAP_UpdateBufUsg_isr: Couldn't update right ring buffer[1] usage"));			
			return BERR_TRACE(ret);	
		}
	}

#if (BRAP_P_WATERMARK_WORKAROUND == 0)
#if (BRAP_P_EDGE_TRIG_INTRPT == 0)
	/* Unmask the free mark interrupt */
	ret = BRAP_P_UnmaskInterrupt_isr(hRapCh, BRAP_Interrupt_eFmmDstRbufFullMark);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_CAP_UpdateBufUsg_isr: Couldn't unmask eFmmRbufFreeByte interrupt"));			
		return BERR_TRACE(ret);	
	}
#endif
#endif
    
	BDBG_LEAVE(BRAP_CAP_UpdateBufUsg_isr);
	return ret;
}



static BERR_Code BRAP_CAP_P_PrepBufInfo_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 			uiRBufId,
	BRAP_RingBufInfo		*pRBufInfo
	)
{
#ifndef BCHP_7411_VER /* not for 7411 */
	BERR_Code err = BERR_SUCCESS;
#endif
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32RdAddr = 0;
	uint32_t ui32WrAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;
	unsigned int uiRBufSize = 0;
	unsigned int uiFrameSize = 0;
#if (BRAP_P_EDGE_TRIG_INTRPT != 0)    
    unsigned int uiGap = 256;
#endif

	BDBG_ENTER(BRAP_CAP_P_PrepBufInfo_isr);

	/* Get the ring buffer handle */
	hRBuf = hRapCh->sModuleHandles.hRBuf[uiRBufId];
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_CAP_P_PrepBufInfo_isr: hRBuf[%d] is NULL", uiRBufId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));
    	
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

	/* Prepare return buf info structure */
	uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;

	/* Each Ring buffer is logically divided into 4 frames. This is also the max 
   	   contiguous ring buffer size in bytes written with PCM data that is informed 
   	   to the user */
	BRAP_RBUF_P_GetFrameSize(hRBuf->sSettings.sExtSettings.uiSize, &uiFrameSize);
	
	pRBufInfo->pBasePtr = (hRBuf->sSettings.sExtSettings.pBufferStart);
	pRBufInfo->pEndPtr = (void *)((unsigned int)pRBufInfo->pBasePtr + uiRBufSize);

#ifndef BCHP_7411_VER /* not for 7411 */
    err = BMEM_ConvertOffsetToAddress(hRBuf->hHeap, ui32RdAddr, (void **)&(pRBufInfo->pReadPtr));
	BDBG_ASSERT(err == BERR_SUCCESS);
    
	BDBG_MSG(("Offset (%p) -> Address (%p)",ui32RdAddr,pRBufInfo->pReadPtr));
#else
    pRBufInfo->pReadPtr = (void *)ui32RdAddr;
#endif

	if(ui32WrAddr <= ui32RdAddr)
	{
		/* tot used size = rbuf size - (rdPtr - wrPtr) */ 
		pRBufInfo->uiTotalSize = uiRBufSize - (ui32RdAddr - ui32WrAddr);

		/* cont used size = endPtr - rdPtr */
		pRBufInfo->uiContiguousSize = (ui32BaseAddr + uiRBufSize) - ui32RdAddr; 	 

		/* Clip cont used size if more than tot free size */
		if(pRBufInfo->uiContiguousSize > pRBufInfo->uiTotalSize)
			pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
 	}
	else
	{
		/* cont used size = tot used size = wrPtr - rdPtr */ 
		pRBufInfo->uiTotalSize = ui32WrAddr - ui32RdAddr;
		pRBufInfo->uiContiguousSize = pRBufInfo->uiTotalSize;
	}

	BDBG_MSG(("Before FRAME logic W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));

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
    	if(pRBufInfo->uiContiguousSize > uiGap)
    	{
    		pRBufInfo->uiContiguousSize -= uiGap;
    	}
    	else
    	{
        	pRBufInfo->uiContiguousSize = 0;
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

	BDBG_MSG(("W=%p R=%p T=%p C=%p F=%p",ui32WrAddr, ui32RdAddr, 
				pRBufInfo->uiTotalSize, pRBufInfo->uiContiguousSize, uiFrameSize));
	BDBG_LEAVE(BRAP_CAP_P_PrepBufInfo_isr);
	return BERR_SUCCESS;
}

static BERR_Code BRAP_CAP_P_WriteBufUsg_isr(
	BRAP_ChannelHandle 	hRapCh,
	unsigned int 		uiRBufId,
	unsigned int		uiSzUpdt
	)
{
	BRAP_RBUF_P_Handle hRBuf;
	uint32_t ui32RdAddr = 0;
	uint32_t ui32RegVal = 0;
	uint32_t ui32BaseAddr = 0;	
	unsigned int uiRBufSize = 0;
	uint32_t ui32NewRdAddr = 0;
	uint32_t ui32WrapAroundBit = 0;
	
	BDBG_ENTER(BRAP_CAP_P_WriteBufUsg);
	BDBG_MSG(("BRAP_CAP_P_WriteBufUsg_isr: hRapCh=%d, uiRBufI=%d, uiSzUpdt=%d",
		hRapCh, uiRBufId, uiSzUpdt));

	/* Get the ring buffer handle */
	hRBuf = hRapCh->sModuleHandles.hRBuf[uiRBufId];
	if(hRBuf == NULL)
	{
		BDBG_ERR(("BRAP_CAP_P_WriteBufUsg_isr: hRBuf[%d] is NULL", uiRBufId));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Get the base address */	
	ui32BaseAddr = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRBuf->ui32Offset));

	/* Get current read address */	
	ui32RegVal = BRAP_Read32_isr(
					hRBuf->hRegister,  
                	(BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset));

#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */ 
    ui32RdAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
						SRING_RDADDR);

	ui32WrapAroundBit = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
						SOURCE_RING_RDADDR_WRAP);
#else
    ui32RdAddr = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
						RINGBUF_RDADDR);	

	ui32WrapAroundBit = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR,
						RINGBUF_RDADDR_WRAP);
#endif 

	BDBG_MSG(("uiSzUpdt(0x%x) ui32RdAddr(0x%x) ui32BaseAddr(0x%x) uiRBufSize(0x%x)",
			uiSzUpdt , ui32RdAddr, ui32BaseAddr, uiRBufSize));

	/* Check if the uiSzUpdt is within the ring buffer */
	uiRBufSize = hRBuf->sSettings.sExtSettings.uiSize;
	if(uiSzUpdt + ui32RdAddr > ui32BaseAddr + uiRBufSize)
	{
		BDBG_ERR(("BRAP_CAP_P_WriteBufUsg_isr: uiReadSize exceeds ring buffer boundary"));
		BDBG_ERR(("uiSzUpdt(0x%x) + ui32RdAddr(0x%x) > ui32BaseAddr(0x%x) + uiRBufSize(0x%x)",
			uiSzUpdt , ui32RdAddr, ui32BaseAddr, uiRBufSize));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	else if(uiSzUpdt + ui32RdAddr == ui32BaseAddr + uiRBufSize)
	{ 
		/* wrap around read ptr to base ptr */
		ui32NewRdAddr = ui32BaseAddr;
		
		BDBG_MSG(("Before WrapAroundBit = 0x%x",ui32WrapAroundBit));
		ui32WrapAroundBit ^= 1;
		BDBG_MSG(("After WrapAroundBit = 0x%x",ui32WrapAroundBit));
	}
	else
	{
		ui32NewRdAddr = ui32RdAddr + uiSzUpdt;
	}
		
	/* Write back the updated read pointer */
#if (BCHP_7411_VER == BCHP_VER_C0)  /* for 7411 C0 */   
	ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        SRING_RDADDR, 
                        ui32NewRdAddr));

	ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        SOURCE_RING_RDADDR_WRAP,
                        ui32WrapAroundBit));
#else /* for 7411 D0, 7401, 7118, 7400 etc */
	ui32RegVal = (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        RINGBUF_RDADDR, 
                        ui32NewRdAddr));

	ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR, 
                        RINGBUF_RDADDR_WRAP,
                        ui32WrapAroundBit));
#endif 

    BRAP_Write32_isr (hRBuf->hRegister,  
                  BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRBuf->ui32Offset, 
                  ui32RegVal);

	BDBG_MSG(("ui32RdAddr = %p,  uiSzUpdt = %p,  newRdAddr = %p",
		ui32RdAddr, uiSzUpdt, ui32RegVal));
	BDBG_LEAVE(BRAP_CAP_P_WriteBufUsg_isr);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:	
	Informs the PI about the completion of reading of data from ring buffers.
Description:
	This API is required to be called by the application after it has completed
	reading of PCM data from ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future.
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_CAP_BufferReadDone_isr
****************************************************************************/
BERR_Code BRAP_CAP_BufferReadDone(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER(BRAP_CAP_BufferReadDone);

#if ( BRAP_P_WATERMARK_WORKAROUND == 1 )
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);	

	BKNI_EnterCriticalSection();
	ret = BRAP_CAP_BufferReadDone_isr(hRapCh);
	BKNI_LeaveCriticalSection();
#else
    BSTD_UNUSED(hRapCh);
#endif /* BRAP_P_WATERMARK_WORKAROUND */

	BDBG_LEAVE(BRAP_CAP_BufferReadDone);
	return ret;
}

/***************************************************************************
Summary:	
	Informs the PI about the completion of reading of data from ring buffers.
Description:
	This API is required to be called by the application after it has completed
	reading of PCM data from ring buffers. This call is mandatory to ensure that
	it gets further interrupts in future. This API is used within the ISR context
Returns:
    Returns error on failure or BERR_SUCCESS on success
See Also:
	BRAP_CAP_BufferReadDone
****************************************************************************/
BERR_Code BRAP_CAP_BufferReadDone_isr(
	BRAP_ChannelHandle 	hRapCh		   /* [in] Audio channel handle */
	)
{
	BERR_Code ret = BERR_SUCCESS;
#if ( BRAP_P_WATERMARK_WORKAROUND == 1 )
	BRAP_PcmBufInfo sPcmBufInfo;

	uint32_t	ui32WaterMarkinBytes =0;
	uint32_t	ui32InitalTimerValue=0;
	uint32_t	ui32Diff=0;
	BAVC_AudioSamplingRate 	eSamplingRate=BAVC_AudioSamplingRate_eUnknown;
	uint32_t	ui32IpBitsPerSample;
    unsigned int uiSR;
	uint32_t	ui32TotalFullBytes=0;

	BDBG_ENTER(BRAP_CAP_BufferReadDone_isr);
	BDBG_MSG(("Entering BRAP_CAP_BufferWriteDone_isr"));

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRapCh);

	ret = BTMR_StopTimer_isr ( hRapCh->hTimer );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_CAP_BufferWriteDone_isr() Failed to stop timer"));
		return BERR_TRACE(ret);
	}

	ret = BRAP_CAP_GetBufInfo_isr(hRapCh, &sPcmBufInfo);
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_CAP_BufferWriteDone_isr() Failed to ger buffer info"));
		return BERR_TRACE(ret);
	}

	ui32WaterMarkinBytes = ( hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiWaterMark *
						hRapCh->sModuleHandles.hRBuf[0]->sSettings.sExtSettings.uiSize )/100;
	ui32TotalFullBytes = sPcmBufInfo.sLtOrSnglBufInfo.uiTotalSize;

	BDBG_MSG(("Before StartTimer"));
	if ( ui32TotalFullBytes >= ui32WaterMarkinBytes )
	{
		ret=BTMR_StartTimer_isr(hRapCh->hTimer, 1000);
        	if ( ret != BERR_SUCCESS )
    		{
    			BDBG_ERR(("BRAP_CAP_BufferWriteDone_isr() Failed to get buffer info"));
    			return BERR_TRACE(ret);
    		}
       	goto end_writedone;
	}
	else
	{

		ui32Diff = ( ui32WaterMarkinBytes - ui32TotalFullBytes );
		ui32IpBitsPerSample = hRapCh->sModuleHandles.hDstCh->sParams.eInputBitsPerSample;

		switch ( hRapCh->sModuleHandles.hDstCh->sParams.eCapInputType)
		{
                
    			case BRAP_CapInputType_eIntI2s0:
				eSamplingRate = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s0]->eSamplingRate;
    				break;
                
#if (BRAP_7401_FAMILY == 0)
			case BRAP_CapInputType_eIntI2s1:
				eSamplingRate = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s1]->eSamplingRate;
				break;
                
			case BRAP_CapInputType_eIntI2s2:
				eSamplingRate = hRapCh->hRap->hFmm[0]->hOp[BRAP_OutputPort_eI2s2]->eSamplingRate;
				break;
#endif
			case BRAP_CapInputType_eExtI2s:
				/* TODO */
				break;

			case BRAP_CapInputType_eIntFlex:
				/* TODO: This is an intenal capture type, so take eInputSR and eIpBitsPerSecond 
				from the source's output as above for I2S */
    			default:
    				BDBG_ERR(("BRAP_CAP_OpenChannel: Unsupported capture input type."));
    				return BERR_TRACE(BERR_NOT_SUPPORTED);

		}

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
                  "ui32WaterMarkinBytes=%d, ui32TotalFullBytes= %d",
                  ui32Diff,eSamplingRate,ui32IpBitsPerSample,ui32InitalTimerValue,
                  ui32WaterMarkinBytes,ui32TotalFullBytes));
        
	}
	BDBG_MSG(("After StartTimer"));

	if ( ui32InitalTimerValue == 0 )
	{
		ui32InitalTimerValue = 1000;
	}

	ret = BTMR_StartTimer_isr ( hRapCh->hTimer, ui32InitalTimerValue );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR(("BRAP_CAP_BufferWriteDone_isr() Failed to start timer"));
		return BERR_TRACE(ret);
	}
	BDBG_MSG(("After BTMR_StartTimer"));

end_writedone:
	BDBG_LEAVE(BRAP_CAP_BufferReadDone_isr);
#else
    BSTD_UNUSED(hRapCh);

#endif /* BRAP_P_WATERMARK_WORKAROUND */
	return ret;
}




