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
*	This file contains the implementations of the APIs for a decode channel, 
*   which are exposed to the upper layer by the Raptor Audio PI. This file 
*   is part of Audio Manager Module. Audio Manager module is the top level 
*   module of the RAP PI, which interfaces with the caller and manages the 
*   internal channel objects to get RAP PI produce the desired result.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"		/* API function and data structures declarations */

/* Internal module includes */
#include "brap_priv.h"

#ifdef RAP_SRSTRUVOL_CERTIFICATION
#include "bchp_xpt_rave.h"
#endif

#if BRAP_P_USE_BRAP_TRANS ==1
#include "brap_transport_priv.h"	
#endif 

BDBG_MODULE(rap_dec);		/* Register software module with debug interface */

/*{{{ Local Defines */

#define BRAP_DEC_P_MAX_OUTPUT_MODES			0x9 /* Derived from BRAP_OutputMode */
#define BRAP_DEC_P_MAX_CHANNEL_PAIRS			0x3 /* Derived from BRAP_OutputMode */
#define BRAP_P_MUXED_I2S_CHANNEL_PAIRS		0x2 /* Mux 2 channel pairs on I2S ports */

/*}}}*/

/*{{{ Local Typedefs */

/*}}}*/


/*{{{ Static Variables & Function prototypes */
static BERR_Code BRAP_DEC_P_GetCurrentAudioParams (BRAP_ChannelHandle	hRapCh,
	BRAP_DEC_AudioParams	*psAudioParams);

/* This array contains the audio output channel informations 
   for each of the audio output mode. This array should be 
   indexed based on the enum BRAP_OutputMode */
#ifdef BCHP_7411_VER
const BRAP_P_OpAudModProp opAudModeProp[BRAP_DEC_P_MAX_OUTPUT_MODES] = 
{
    {1, {true, false, false, false, false, false}},    /* Left Mono */
    {1, {false, true, false, false, false, false}},    /* Right Mono */
    {2, {true, true, false, false, false, false}},    /* True Mono */
    {2, {true, true, false, false, false, false}},    /* Stereo */
    {3, {true, true, false, false, false, true}},        /* 2.1 */
    {4, {true, true, true, false, false, true}},        /* 3.1 */
    {5, {true, true, true, true, false, true}},        /* 4.1 */
    {6, {true, true, true, true, true, true}}         /* 5.1 */
};
#endif

#ifndef BCHP_7411_VER
const bool bChannelPairExistance[BRAP_DEC_P_MAX_OUTPUT_MODES][BRAP_DEC_P_MAX_CHANNEL_PAIRS] =
{
	{false,false,true},
	{false,true,true},
	{true,false,false},
	{true,false,false},
	{true,false,true},
	{true,true,false},
	{true,true,true},
	{true,true,false},
	{true,true,true}
};
#endif

#if BRAP_P_USE_BRAP_TRANS ==1
/* Mapping between 7411 input port and Timebase */
static const BAVC_Timebase inPortTimebaseMapping[] = 
{
	BAVC_Timebase_e0,	/* For BARC_InputPort_ePortA */
	BAVC_Timebase_e1,	/* For BARC_InputPort_ePortB */
	BAVC_Timebase_e0,	/* For BARC_InputPort_eHostPortA */
	BAVC_Timebase_e1	/* For BARC_InputPort_eHostPortB */
};
#endif

/*}}}*/

/***************************************************************************
Summary:
	API to retrieve maximum number of decode channels supported. 

Description:
	This API used to retrieve the maximum number of decode channels (channels 
	where either decode, or pass-through or simultaneous mode or SRC can be 
	done) supported by the Raptor Audio PI.
Returns:
	BERR_SUCCESS 
	
See Also:
	BRAP_Open
	
****************************************************************************/
BERR_Code BRAP_DEC_GetTotalChannels( 
	BRAP_Handle 	hRap,			/* [in] The Raptor Audio device handle*/
	unsigned int	*pTotalChannels	/* [out] Total decode Audio channels 
											supported */
	)
{
	BDBG_ENTER(BRAP_DEC_GetTotalChannels);
	
	/* Assert the function argument(s) */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pTotalChannels);
	BSTD_UNUSED (hRap);
	
	*pTotalChannels = BRAP_RM_P_MAX_DEC_CHANNELS;
	
	BDBG_LEAVE(BRAP_DEC_GetTotalChannels);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Gets the default channel settings of a decode Audio Channel.
Description:
	This API returns the default channel settings of a decode Audio channel.
	"Stereo decode on I2S0 output port without Simul Mode and Cloning" 
	configuration is supported as default setting.

	Default values of BRAP_DEC_ChannelSettings are:
        eOutputMode = BRAP_OutputMode_e2_0
        eOutputLR = BRAP_OutputPort_eI2s0
        eOutputLRSurround = BRAP_OutputPort_eI2s1
        eOutputCntrLF = BRAP_OutputPort_eI2s2
        uiWaterMark = BRAP_RBUF_P_DEFAULT_FREE_BYTE_MARK
        bSimulModeConfig = false
        bCloneConfig = false
        bMuxChannelPairsOnI2sConfig = false
        eSimulModePtOutput = BRAP_OutputPort_eSpdif

        The DSP/Algorithm settings are not listed here since they are too numerous.

        Note that this PI returns default values for all fields in the 
        BRAP_DEC_ChannelSettings structure, but depending on the usage, 
        not all of them may be used/required.
	
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_OpenChannel, BRAP_DEC_CloseChannel
***************************************************************************/
BERR_Code BRAP_DEC_GetChannelDefaultSettings(
	BRAP_Handle 			 hRap,			/* [in] The RAP device handle */
	BRAP_DEC_ChannelSettings *pDefSettings	/* [out] Default channel settings */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	BRAP_DSPCHN_P_Settings	*psDefDspChSettings;
	int i = 0;

	BDBG_ENTER(BRAP_DEC_GetChannelDefaultSettings);

	/* Validate input parameters. */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pDefSettings);
	BSTD_UNUSED (hRap);

	psDefDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
	if (NULL==psDefDspChSettings )
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

       /* NOTE: if default value of any of these settings is changed, please update 
       it in the description as well */
       
	pDefSettings->uiWaterMark = BRAP_RBUF_P_DEFAULT_FREE_BYTE_MARK;	/* Default free 
															   byte mark */
#ifdef BCHP_7411_VER  /* only for 7411 */                                                                  
	pDefSettings->eOutputMode = BRAP_OutputMode_e2_0;	/* Default output is 
															   stereo */
	pDefSettings->eOutputLR = BRAP_OutputPort_eI2s0;		/* Default stero output port 
															   is I2S0 */
	pDefSettings->eOutputLRSurround = BRAP_OutputPort_eI2s1;/* Default LR Surround output 
															   port is I2S1 */
	pDefSettings->eOutputCntrLF = BRAP_OutputPort_eI2s2;	/* Default Centre and LF output 
															   port is I2S2 */

	pDefSettings->bCloneConfig = false;					    /* No clone by default */
    pDefSettings->bMuxChannelPairsOnI2sConfig = false;
	pDefSettings->bSimulModeConfig = false;					/* No simultaneous mode 
															   by default */
	pDefSettings->eSimulModePtOutput = BRAP_OutputPort_eSpdif;/* Default Output for 
															   Simultaneous mode's 
															   Pass-thru and Clone output
                                                               is SPDIF */
#endif    

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
	pDefSettings->bLargeRbuf = false; /* By default, dont allocate large RBUFs */
#endif                                                           

	/* Fill up the default DSP Channel Settings */
	ret = BRAP_DSPCHN_P_GetDefaultSettings(psDefDspChSettings);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("Unable to get DSP Channel default settings"));
		BKNI_Free(psDefDspChSettings);
		return BERR_TRACE(ret);
	}

	for ( i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
	{
		pDefSettings->sDspChSettings[i] = psDefDspChSettings->sDspchnExtSettings[i];
	}

	BKNI_Free(psDefDspChSettings);
	BDBG_LEAVE(BRAP_DEC_GetChannelDefaultSettings);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	API used to open a decode channel.

Description:
	It is used to instantiate a decode channel. It allocates channel handle 
	and resource required for the channel if any.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_CloseChannel
****************************************************************************/
#ifndef BCHP_7411_VER
BERR_Code BRAP_DEC_OpenChannel( 
    BRAP_Handle             hRap,           /* [in] The Raptor Audio device handle*/
    BRAP_ChannelHandle      *phRapCh,       /* [out] The RAP Channel handle */
    unsigned int            uiChannelNo,    /* [in] the desired channel ID */                   
    const BRAP_DEC_ChannelSettings  *pChnSettings /*[in] Channel settings*/ 
    )
{
    BERR_Code               ret = BERR_SUCCESS;
    BRAP_ChannelHandle      hRapCh;
    BRAP_DSPCHN_P_Settings  *psDspChSettings = NULL;
    BRAP_MIXER_P_Settings   sMixerSettings;
    BRAP_RM_P_ResrcReq      sResrcReq;
    unsigned int            i, j;
    BRAP_P_ObjectHandles    sTempHandles; 
    bool                    bWatchdogRecoveryOn = false;
    
    BDBG_ENTER(BRAP_DEC_OpenChannel);
    
    /* Assert the function argument(s) */
    BDBG_ASSERT(hRap);
    BDBG_ASSERT(phRapCh);
    
    /* Check if this is watchdog recovery */
    bWatchdogRecoveryOn = BRAP_P_GetWatchdogRecoveryFlag(hRap);

    if (false==bWatchdogRecoveryOn) 
    {

        BDBG_ASSERT(pChnSettings);     
        BDBG_MSG (( "BRAP_DEC_OpenChannel():"
                    "hRap=0x%x, \n\tuiChannelNo=%d," 
                    "\n\tpChnSettings->uiWaterMark=%d,",
                    hRap, uiChannelNo, pChnSettings->uiWaterMark));

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
		BDBG_MSG(("\n\tpChnSettings->bLargeRbuf=%d", pChnSettings->bLargeRbuf));
#endif
        BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));  /* make sure this struct is clean */

        BKNI_Memset (&sResrcReq, BRAP_INVALID_VALUE, sizeof(sResrcReq));  /* make sure this struct is clean */

        if(uiChannelNo > BRAP_RM_P_MAX_DEC_CHANNELS)
        {
            /* The channel is not supported */
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

	/* Allocating memory for BRAP_DSPCHN_P_Settings after input parameter checking to 
	 * avoid freeing it on bad input parameter */
	psDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
	if (NULL==psDspChSettings)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

        /* Allocate an Audio Decode Channel handle */
        *phRapCh = hRapCh = (BRAP_ChannelHandle)BKNI_Malloc (sizeof(BRAP_P_Channel));
        if(hRapCh == NULL)
        {
            BDBG_ERR(("Memory allocation for channel handle failed"));
            ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto end_open;
        }

        /* Initialize the Channel Handle */
        BKNI_Memset (hRapCh, 0, sizeof(BRAP_P_Channel));

        /* Reset channel state variables explicitly */
        hRapCh->bStarted = false;
        hRapCh->bInternalCallFromRap = false;
        
        hRapCh->hRap = hRap ;
        hRapCh->hChip = hRap->hChip ;
        hRapCh->hHeap = hRap->hHeap ;
        hRapCh->hInt = hRap->hInt ;
        hRapCh->hRegister = hRap->hRegister;
        hRapCh->ui32FmmBFIntMask = 0;
        hRapCh->bSimulModeConfig = false;
        hRapCh->eChannelType = BRAP_P_ChannelType_eDecode; /* It is a decode channel */
        hRapCh->uiChannelNo = uiChannelNo; /*store the channel id */
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
        hRapCh->bLargeRbuf = pChnSettings->bLargeRbuf;
#endif


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

#ifdef RAP_SRSTRUVOL_CERTIFICATION  
    hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.LeftDramBufferReadAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.LeftDramBufferWriteAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.LeftDramBufferEndAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.RightDramBufferBaseAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.RightDramBufferReadAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.RightDramBufferWriteAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.RightDramBufferEndAddr = BRAP_P_INVALID_DRAM_ADDRESS;
    hRapCh->sPcmBufferInfo.uiSize = 0;
    hRapCh->sPcmBufferInfo.bLeftWrWrapAround= 0;    
    hRapCh->sPcmBufferInfo.bLeftRdWrapAround= 0;        
    hRapCh->sPcmBufferInfo.bRightWrWrapAround= 0;    
    hRapCh->sPcmBufferInfo.bRightRdWrapAround= 0;       
#endif
		
        /* Form the resource requirement */
        sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
        sResrcReq.uiNumOpPorts = 0;
        sResrcReq.bLastChMono = false;
        sResrcReq.bSimulModePt = false; 
        sResrcReq.bAllocateDsp = true; 
        sResrcReq.bAllocateDspCtx  = false; 
        sResrcReq.bAllocateRBuf= true;
        sResrcReq.bAllocateSrcCh= true;
        sResrcReq.bAllocatePpm= false;
        sResrcReq.bAllocateDstCh= false;                                            

        for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
        }   

        /* Call resource manager to allocate required resources. */
        ret = BRAP_RM_P_AllocateResources (hRap->hRm, 
                                           &sResrcReq, 
                                           &(hRapCh->sRsrcGrnt));
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("Resource allocation failed for RAP DEC Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret); 
            goto error;
        }

        /* Store DSP and FMM handles inside the Audio Channel handle */
        hRapCh->sModuleHandles.hDsp = hRap->hDsp[hRapCh->sRsrcGrnt.uiDspId];
        hRapCh->sModuleHandles.hFmm = hRap->hFmm[hRapCh->sRsrcGrnt.uiFmmId];

        /* Form the DSP Channel Settings */
        ret = BRAP_DSPCHN_P_GetDefaultSettings(psDspChSettings);
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("Error getting DSPCHN default settings for RAP DEC Channel handle 0x%x", hRapCh));
            ret = BERR_TRACE(ret); goto error;
        }
        
	for ( i = 0; i < BRAP_DEC_DownmixPath_eMax; i++ )
	{
		psDspChSettings->sDspchnExtSettings[i] = pChnSettings->sDspChSettings[i];
	}

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

    } /* (!BRAP_GetWatchdogRecoveryFlag(hRap)) */
    else 
    { /* If watchdog recovery */
        hRapCh = *phRapCh;
    }
    
    /* Instantiate the DSP Channel corresponding to the DSP 
       context resource manager has allocated */
    ret = BRAP_DSPCHN_P_Open(
                &(hRapCh->sModuleHandles.hDspCh),
                hRapCh->sModuleHandles.hDsp,
                hRapCh->sRsrcGrnt.uiDspContextId,       
                psDspChSettings);

    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        BDBG_ERR(("DSP Channel Open failed for RAP DEC Channel handle 0x%x", hRapCh));
        if (true==bWatchdogRecoveryOn)
        {
        	return (ret);
        }
        else
        {
            goto error;
        }
    }

    if (true==bWatchdogRecoveryOn)            
    {
        /* If in watchdog recovery, Open all FMM Modules */
        for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {

            if(hRapCh->sModuleHandles.hOp[i] == NULL)
            {
                /* No output port is connected for this channel pair*/
            
                continue;
            }
            BDBG_MSG (("BRAP_DEC_Open: WAtchdog recovery: opening channel pair %d, port %d", 
                i, hRapCh->sModuleHandles.hOp[i]->eOutputPort));
            
            sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[i].uiMixerInputId;
            /* Open all internal modules */
            ret = BRAP_P_OpenOpPathFmmModules (hRapCh,
                                                NULL,
                                                NULL,
                                                &sMixerSettings,
                                                NULL,
                                                NULL,
                                                &(hRapCh->sRsrcGrnt.sOpResrcId[i]),
                                                (BRAP_OutputChannelPair)i,
                                                &(hRapCh->sModuleHandles));
            if(ret != BERR_SUCCESS)
            {
                BDBG_ERR(("FMM Module Opens failed for RAP DEC Channel handle 0x%x, channel pair %d", 
                   hRapCh, i));
                return BERR_TRACE(ret);
            }

            BDBG_MSG(("\nhRapCh->sModuleHandles.hOp[i]: %x"
              "\nhRapCh->sModuleHandles.hMixer[i]: %x"
              "\nhRapCh->sModuleHandles.hSrcCh[i]: %x"
              "\nhRapCh->sModuleHandles.hSpdifFm[i]: %x"
              "\nhRapCh->sModuleHandles.hRbuf[2*i]: %x"
              "\nhRapCh->sModuleHandles.hRbuf[2*i+1]: %x", 
              hRapCh->sModuleHandles.hOp[i],
              hRapCh->sModuleHandles.hMixer[i],
              hRapCh->sModuleHandles.hSrcCh[i],
              hRapCh->sModuleHandles.hSpdifFm[i],
              hRapCh->sModuleHandles.hRBuf[2*i],
              hRapCh->sModuleHandles.hRBuf[2*i+1]));

            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {
                if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
                {                  
                    /* note: this handles all cloned ports (ie includes clones 
                    of normal ports as well as clones of simul pt ports */
                    BDBG_MSG (("BRAP_DEC_Open: WAtchdog recovery: opening cloned port %d", j));
                    sMixerSettings.uiMixerInput = hRapCh->sCloneOpPathResc[i][j].uiMixerInputId;

                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                    sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                    sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                    sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;      
                    sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
                    
                    ret = BRAP_P_OpenOpPathFmmModules(
                            hRapCh,
                            NULL,
                            NULL,
                            &sMixerSettings,
                            NULL,
                            NULL,
                            &(hRapCh->sCloneOpPathResc[i][j]),
                            (BRAP_OutputChannelPair)i,
                            &(sTempHandles));   
                    if(ret != BERR_SUCCESS)
                    {
                        BDBG_ERR(("FMM Module Opens failed for RAP DEC Channel handle 0x%x,"
                           "channel pair %d, cloned port %d", 
                           hRapCh, i, j));
                        return BERR_TRACE(ret);
                    }                    
                 } /* if hOp!= NULL */
            }/* for loop:  BRAP_RM_P_MAX_OUTPUTS*/
        }/* for loop: BRAP_RM_P_MAX_OP_CHANNEL_PAIRS*/

        if (hRapCh->bSimulModeConfig == true)
        {
            sMixerSettings.uiMixerInput 
                = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiMixerInputId;
            ret = BRAP_P_OpenOpPathFmmModules(
                                    hRapCh,
                                    NULL,
                                    NULL,
                                    &sMixerSettings,
                                    NULL,
                                    NULL,
                                    &(hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR]),
                                    BRAP_OutputChannelPair_eLR,
                                    &(hRapCh->sSimulPtModuleHandles));
            if(ret != BERR_SUCCESS)
            {
                ret = BERR_TRACE(ret); 
                BDBG_ERR(("FMM Module Opens for 2nd context of Simul Mode"
                      "failed for RAP DEC Channel handle 0x%x", hRapCh));
                return (ret);
            }

            BDBG_MSG(("\nhRapCh->sSimulPtModuleHandles.hOp[0]: %x"
              "\nhRapCh->sSimulPtModuleHandles.hMixer[0]: %x"
              "\nhRapCh->sSimulPtModuleHandles.hSrcCh[0]: %x"
              "\nhRapCh->sSimulPtModuleHandles.hSpdifFm[0]: %x"
              "\nhRapCh->sSimulPtModuleHandles.hRbuf[2*0]: %x"
              "\nhRapCh->sSimulPtModuleHandles.hRbuf[2*0+1]: %x", 
              hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR],
              hRapCh->sSimulPtModuleHandles.hMixer[BRAP_OutputChannelPair_eLR],
              hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR],
              hRapCh->sSimulPtModuleHandles.hSpdifFm[BRAP_OutputChannelPair_eLR],
              hRapCh->sSimulPtModuleHandles.hRBuf[2*BRAP_OutputChannelPair_eLR],
              hRapCh->sSimulPtModuleHandles.hRBuf[2*BRAP_OutputChannelPair_eLR+1]));   
        } /* if Simul */
    }/* if watchdog recovery */

    goto end_open;

error:
    /* Free up the channel handle */
    BKNI_Free(hRapCh);
    
end_open:
    if (psDspChSettings)    
        BKNI_Free(psDspChSettings);
   /* only if channel has been successfully opened, save the handle */
    if ((ret == BERR_SUCCESS) && (false==bWatchdogRecoveryOn))
    {
       hRap->hRapDecCh[uiChannelNo] = hRapCh ;    
    }
    BDBG_LEAVE(BRAP_DEC_OpenChannel);

    return BERR_TRACE(ret);
}

#else

BERR_Code BRAP_DEC_OpenChannel( 
	BRAP_Handle 			hRap,			/* [in] The Raptor Audio device handle*/
	BRAP_TRANS_ChannelHandle	hRapTransCh,/* [in] The Raptor Audio transport 
													channel handle.
													This is specific to 7411 only */
	BRAP_ChannelHandle 		*phRapCh,		/* [out] The RAP Channel handle */
	unsigned int			uiChannelNo,	/* [in] the desired channel ID */					
	const BRAP_DEC_ChannelSettings	*pChnSettings /*[in] Channel settings*/ 
	)
{
	BERR_Code				ret = BERR_SUCCESS;
	BERR_Code				rc = BERR_SUCCESS;
	BRAP_ChannelHandle		hRapCh = NULL;
	BRAP_DSPCHN_P_Settings	*psDspChSettings = NULL;
	BRAP_RBUF_P_Settings	sRbufSettings[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
	BRAP_SRCCH_P_Settings	sSrcChSettings;
	BRAP_MIXER_P_Settings	sMixerSettings;
	BRAP_SPDIFFM_P_Settings	sSpdifFmSettings;
	BRAP_OP_P_MaiSettings	sMaiSettings;
	BRAP_OP_P_SpdifSettings	sSpdifSettings;
	BRAP_OP_P_I2sSettings	sI2sSettings;
	BRAP_OP_P_FlexSettings	sFlexSettings;
	BRAP_OP_P_DacSettings	sDacSettings;
	BRAP_RM_P_ResrcReq		sResrcReq;
	BRAP_OutputPort			eOutput[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS];
	void					*pOpSettings = NULL;
	unsigned int			i, uiChnPair, j, uiMaxChPairs=0;
	bool						bWatchdogRecoveryOn = false;
#if BCHP_7411_VER > BCHP_VER_C0	
	unsigned int			ui2ndRbufIdLR = BRAP_RM_P_INVALID_INDEX, ui2ndRbufIdLsRs = BRAP_RM_P_INVALID_INDEX;
	BRAP_RBUF_P_Settings		sTempRbufSettings;
#endif	
    
	BDBG_ENTER(BRAP_DEC_OpenChannel);


    for(i=0; i<BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        eOutput[i] = (unsigned int)BRAP_RM_P_INVALID_INDEX;
    
	/* Assert the function argument(s) */
	BDBG_ASSERT(hRap);
	
	/* Check if this is watchdog recovery */
	bWatchdogRecoveryOn = BRAP_P_GetWatchdogRecoveryFlag(hRap);
	
	BDBG_ASSERT(phRapCh);
	BDBG_ASSERT(hRapTransCh);
	if (false==bWatchdogRecoveryOn) {
		BDBG_ASSERT(pChnSettings);
    	BDBG_MSG (( "BRAP_DEC_OpenChannel():"
        	        "hRap=0x%x, \n\tuiChannelNo=%d," 
            	    "\n\tpChnSettings->eOutputMode=%d,"
                	"\n\tpChnSettings->eOutputLR=%d,"
					"\n\tpChnSettings->eOutputLRSurround=%d,"
					"\n\tpChnSettings->eOutputCntrLF=%d,"
					"\n\tpChnSettings->uiWaterMark=%d,"
                	"\n\tpChnSettings->bSimulModeConfig=%d,"
					"\n\tpChnSettings->eSimulModePtOutput=%d",
                	hRap, uiChannelNo, pChnSettings->eOutputMode, 
					pChnSettings->eOutputLR, pChnSettings->eOutputLRSurround, 
					pChnSettings->eOutputCntrLF, pChnSettings->uiWaterMark,
        	        pChnSettings->bSimulModeConfig, 
					pChnSettings->eSimulModePtOutput));

    	BDBG_MSG (( "\n\tpChnSettings->bCloneConfig=%d,"
    				"\n\tpChnSettings->bMuxChannelPairsOnI2sConfig=%d",
				pChnSettings->bCloneConfig,
				pChnSettings->bMuxChannelPairsOnI2sConfig));

       BKNI_Memset (&sResrcReq, BRAP_INVALID_VALUE, sizeof(sResrcReq));  /* make sure this struct is clean */
	
	if(uiChannelNo > BRAP_RM_P_MAX_DEC_CHANNELS)
	{
		/* The channel is not supported */
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if (pChnSettings->bMuxChannelPairsOnI2sConfig==true) 
    {
#if BCHP_7411_VER==BCHP_VER_C0
		BDBG_ERR(("Channel pair muxing on I2S is supported only on 7411D0"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
#elif BCHP_7411_VER > BCHP_VER_C0
		if (hRap->sSettings.bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eDdbm]==false) 
        {
			BDBG_ERR(("For channel pairs multiplexing on I2S, Dolby Digital Post Processing algorithm needs"
				"to be selected in BRAP Settings"));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
		}
#endif	
	}

	    /* For 7411, the only permissible external output ports are I2S0 and SPDIF. internally I2s1, I2s2 and flex are also present*/
    	/* NOTE: these checks should always be updated to reflect latest supported
       	options on each platform */
    	/* 4. Check requested o/p port with the currently supported output ports:*/

        ret = BRAP_RM_P_IsOutputPortSupported(pChnSettings->eOutputLR);
    	if( ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_DEC_OpenChannel: eOutputPort(%d) is not supported",
    			pChnSettings->eOutputLR));
    		return BERR_TRACE(ret);
    	}

        ret = BRAP_P_IsNewChanCompatible(hRap, BRAP_P_ChannelType_eDecode, pChnSettings->eOutputLR);
        if(ret!=BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_DEC_OpenChannel: DEC channel not compatible with existing channels"));
    		return BERR_TRACE(ret);
        }

#if BCHP_7411_VER > BCHP_VER_C0
    	if (pChnSettings->bMuxChannelPairsOnI2sConfig) 
        {
            ret = BRAP_RM_P_IsOutputPortSupported(pChnSettings->eOutputLRSurround);
        	if( ret != BERR_SUCCESS)
        	{
        		BDBG_ERR(("BRAP_DEC_OpenChannel: eOutputPort(%d) is not supported",
        			pChnSettings->eOutputLRSurround));
        		return BERR_TRACE(ret);
        	}

            ret = BRAP_P_IsNewChanCompatible(hRap, BRAP_P_ChannelType_eDecode, pChnSettings->eOutputLRSurround);
            if(ret!=BERR_SUCCESS)
            {
                BDBG_ERR(("BRAP_DEC_OpenChannel: DEC channel not compatible with existing channels"));
        		return BERR_TRACE(ret);
            }

            /* Check if M Clock is twice of S Clock for both the output ports */
            /* For eOutputLR */
            sI2sSettings.sExtSettings = hRap->sOutputSettings[pChnSettings->eOutputLR].
                uOutputPortSettings.sI2sSettings;
            if(sI2sSettings.sExtSettings.eSClkRate == BRAP_OP_SClkRate_e64Fs)
            {
                if(sI2sSettings.sExtSettings.eMClkRate != BRAP_OP_MClkRate_e128Fs)
                {
    	            BDBG_ERR(("BRAP_DEC_OpenChannel: eMClkRate(%d) should be twice of eSClkRate(%d)",
                        sI2sSettings.sExtSettings.eMClkRate, sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
                }
            }
            else if(sI2sSettings.sExtSettings.eSClkRate == BRAP_OP_SClkRate_e128Fs)
            {
                if(sI2sSettings.sExtSettings.eMClkRate != BRAP_OP_MClkRate_e256Fs)
                {
                    BDBG_ERR(("BRAP_DEC_OpenChannel: eMClkRate(%d) should be twice of eSClkRate(%d)",
                        sI2sSettings.sExtSettings.eMClkRate, sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
                }
            }
            else
            {
                BDBG_ERR(("BRAP_DEC_OpenChannel: Unknown eSClkRate(%d)",sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
            }

            /* For eOutputLRSurround */
            sI2sSettings.sExtSettings = hRap->sOutputSettings[pChnSettings->eOutputLRSurround].
                uOutputPortSettings.sI2sSettings;
            if(sI2sSettings.sExtSettings.eSClkRate == BRAP_OP_SClkRate_e64Fs)
            {
                if(sI2sSettings.sExtSettings.eMClkRate != BRAP_OP_MClkRate_e128Fs)
                {
    	            BDBG_ERR(("BRAP_DEC_OpenChannel: eMClkRate(%d) should be twice of eSClkRate(%d)",
                        sI2sSettings.sExtSettings.eMClkRate, sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
                }
            }
            else if(sI2sSettings.sExtSettings.eSClkRate == BRAP_OP_SClkRate_e128Fs)
            {
                if(sI2sSettings.sExtSettings.eMClkRate != BRAP_OP_MClkRate_e256Fs)
                {
                    BDBG_ERR(("BRAP_DEC_OpenChannel: eMClkRate(%d) should be twice of eSClkRate(%d)",
                        sI2sSettings.sExtSettings.eMClkRate, sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
                }
            }
            else
            {
                BDBG_ERR(("BRAP_DEC_OpenChannel: Unknown eSClkRate(%d)",sI2sSettings.sExtSettings.eSClkRate));
    	    		return BERR_TRACE(ret);
            }       
    	}
#endif
    	/* 7411 has only setero output. Therefore eOutputLRSurround and 
     	 eOutputCntrLF are not used. For other platforms, these should also 
     	 be checked */
#if BCHP_7411_VER > BCHP_VER_C0
    	if ((pChnSettings->eOutputMode != BRAP_OutputMode_e2_0) &&
    		(pChnSettings->bMuxChannelPairsOnI2sConfig==false))
        	{
           		BDBG_ERR(("BRAP_DEC_OpenChannel: If I2S channel pairing is not enabled, 7411D0 supports only stereo"));
    	    	return BERR_TRACE(BERR_NOT_SUPPORTED); 
        	}
#else
    	if (pChnSettings->eOutputMode != BRAP_OutputMode_e2_0) 
        	{
           		BDBG_ERR(("BRAP_DEC_OpenChannel: 7411 supports only stereo"));
    	    	return BERR_TRACE(BERR_NOT_SUPPORTED); 
        	}
#endif

		/* Allocating memory for BRAP_DSPCHN_P_Settings after input parameter checking to 
		 * avoid freeing it on bad input parameter */
		psDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
		if (NULL==psDspChSettings)
		{
			return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		}
    
		/* Check if for all required outputs are configured before or not */

		eOutput[BRAP_OutputChannelPair_eLR] = pChnSettings->eOutputLR;
		eOutput[BRAP_OutputChannelPair_eLRSurround] = pChnSettings->eOutputLRSurround;
		eOutput[BRAP_OutputChannelPair_eCentreLF] = pChnSettings->eOutputCntrLF;

#if BCHP_7411_VER > BCHP_VER_C0
		if (pChnSettings->bMuxChannelPairsOnI2sConfig==false) 
		{
#endif			
			for(uiChnPair = 0; uiChnPair < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; uiChnPair++)
			{
				if((opAudModeProp[pChnSettings->eOutputMode].bChnExists[uiChnPair * 2] == true) || 
			   		(opAudModeProp[pChnSettings->eOutputMode].bChnExists[uiChnPair * 2 + 1] == true))
				{
					/* This output channel exists for the current output mode, 
				   		so do the checking */
					if(hRap->bOpSettingsValid[eOutput[uiChnPair]] == false)
					{
						BDBG_ERR(("Output port %d not configured", eOutput[uiChnPair]));
						ret = BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
						goto end_open;
					}
				}
			}

#if BCHP_7411_VER > BCHP_VER_C0
		}
		else {
			for (uiChnPair = 0; uiChnPair < BRAP_P_MUXED_I2S_CHANNEL_PAIRS; uiChnPair++)
			{
				if(hRap->bOpSettingsValid[eOutput[uiChnPair]] == false)
				{
                                BDBG_ERR(("Output port %d not configured", eOutput[uiChnPair]));
                                ret = BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
                                goto end_open;
				}
			}
		}
#endif		

		/* Allocate an Audio Decode Channel handle */
		*phRapCh = hRapCh = (BRAP_ChannelHandle)BKNI_Malloc( sizeof(BRAP_P_Channel));
		if(hRapCh == NULL)
		{
			BDBG_ERR(("Memory allocation for channel handle failed"));
			ret = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                     goto end_open;
		}

		/* Initialize the Channel Handle */
		BKNI_Memset( hRapCh, 0, sizeof(BRAP_P_Channel) );

		/* Reset channel state variables explicitly */
		hRapCh->bStarted = false;
		hRapCh->bInternalCallFromRap = false;
		
		hRapCh->hRap = hRap ;
		hRapCh->hChip = hRap->hChip ;
		hRapCh->hHeap = hRap->hHeap ;
		hRapCh->hInt = hRap->hInt ;
		hRapCh->hRegister = hRap->hRegister;
        hRapCh->ui32FmmBFIntMask = 0;
		hRapCh->hRapTransCh = hRapTransCh;	

		hRapCh->eCurOpMode = pChnSettings->eOutputMode;
		hRapCh->bSimulModeConfig = pChnSettings->bSimulModeConfig;
#if BCHP_7411_VER > BCHP_VER_C0
		hRapCh->bMuxChPairOnI2s = pChnSettings->bMuxChannelPairsOnI2sConfig;
#endif        

          
		if (pChnSettings->bCloneConfig == true)
    	{
       		hRapCh->eClone = BRAP_P_CloneState_eConfigured;
    	}
    	else
    	{
        	hRapCh->eClone = BRAP_P_CloneState_eInvalid;
    	}
       
		hRapCh->eChannelType = BRAP_P_ChannelType_eDecode; /* It is a decode channel */
		hRapCh->uiChannelNo = uiChannelNo; /*store the channel id */


		/* Form the resource requirement */
		sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
#if BCHP_7411_VER > BCHP_VER_C0
		if (pChnSettings->bMuxChannelPairsOnI2sConfig==false)
		{
#endif
			sResrcReq.uiNumOpPorts = (opAudModeProp[pChnSettings->eOutputMode].uiNoChannels + 1) / 2;
			sResrcReq.bLastChMono = (opAudModeProp[pChnSettings->eOutputMode].uiNoChannels) % 2;
#if BCHP_7411_VER > BCHP_VER_C0
		}
		else
		{
			sResrcReq.uiNumOpPorts = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
			sResrcReq.bLastChMono = false;
		}
#endif
		
		sResrcReq.bSimulModePt = false; /* Resource allocation is not for the pass-thru 
									 		context of the simultaneous mode */
        sResrcReq.bAllocateDsp = true;                                                 

        for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
        }
        
#if BCHP_7411_VER > BCHP_VER_C0
		if (pChnSettings->bMuxChannelPairsOnI2sConfig==false)
		{
#endif
    		/* We are here concerned about the output ports to be allocated. 
    	   		We will assign the output ports and the associated resources 
    	   		to proper output channels later. */
    		for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    		{
    			if((opAudModeProp[pChnSettings->eOutputMode].bChnExists[i * 2] == false) && 
    		   		(opAudModeProp[pChnSettings->eOutputMode].bChnExists[i * 2 + 1] == false))
    			{
    			
    				sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
    			
    			}
    			else
    			{
    				/* This output channel exists for the current output mode, 
    			  	 include the corresponding output port */
    				sResrcReq.sOpPortReq[i].eOpPortType = eOutput[i];
    				/* If output port is MAI then initialize the MAI mux selector */			
    				if ( eOutput[i] == BRAP_OutputPort_eMai )
    				{
    					sResrcReq.sOpPortReq[i].eMuxSelect = 
    						hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    				}
    			}
    		}
#if BCHP_7411_VER > BCHP_VER_C0
		}
		else
		{
			for(i = 0; i < BRAP_P_MUXED_I2S_CHANNEL_PAIRS; i++)
			{
				sResrcReq.sOpPortReq[i].eOpPortType = eOutput[i];
			}
		}	
#endif			

		sResrcReq.bAllocateRBuf= true;
		sResrcReq.bAllocateSrcCh= true;
		sResrcReq.bAllocateDstCh= false;

		/* Call resource manager to allocate required resources. */
		ret = BRAP_RM_P_AllocateResources(	hRap->hRm, 
											&sResrcReq, 
											&(hRapCh->sRsrcGrnt));
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("Resource allocation failed for RAP DEC Channel handle 0x%x", hRapCh));
			ret = BERR_TRACE(ret); goto error;
		}

#if BCHP_7411_VER > BCHP_VER_C0
		if (pChnSettings->bMuxChannelPairsOnI2sConfig==true)
		{
			/* When decode is started in overclocked (I2S) multichannel mode, we  
			 * use single stereo interleaved ring buffer for each of the output ports (I2S0, I2S1).
			 * When decode is started in at-clock (I2S) stereo mode, we use two
			 * mono non-interleaved ring buffers only for output port I2S0. Therefor we really
			 * don't require second ring buffer for I2S1. Since in both modes (overclocked 
			 * and at-clocked) we use only two ring buffers, we don't want to allocate memory
			 * for more than two ring buffers. This is essential since ring buffer size is huge
			 * for SRC operation or multichannel decode. This we achieve as below.
			 * 1. Open only first ring buffer for each of the output ports. This will allocate
			 *     memory for first ring buffers.
			 * 2. Only for I2S0, open second ring buffer but pass ring buffer start address
			 *     and size of first ring buffer of I2S1. This will prevent allocating memory
			 *     for second ring buffer of I2S0 and make it share memory with first ring
			 *     buffer of I2S1. As explained above, at any given point of time only one
			 *     of these ring buffers would be active.
			 */

			/* Make ring buffer Ids of second ring buffers invalid. But preserve their Ids to restore
			 * back after allocation of memory.*/
			ui2ndRbufIdLR = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1];
			hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1] = 
				(unsigned int)BRAP_RM_P_INVALID_INDEX;
			ui2ndRbufIdLsRs = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLRSurround].uiRbufId[1];
			hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLRSurround].uiRbufId[1] = 
				(unsigned int)BRAP_RM_P_INVALID_INDEX;
		}
#endif

		/* Store DSP and FMM handles inside the Audio Channel handle */
		hRapCh->sModuleHandles.hDsp = hRap->hDsp[hRapCh->sRsrcGrnt.uiDspId];
		hRapCh->sModuleHandles.hFmm = hRap->hFmm[hRapCh->sRsrcGrnt.uiFmmId];

		/* Store the output audio mode */
		hRapCh->eMaxOpMode = pChnSettings->eOutputMode;

		/* Form the DSP Channel Settings */
		ret = BRAP_DSPCHN_P_GetDefaultSettings(psDspChSettings);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("Error getting DSPCHN default settings for RAP DEC Channel handle 0x%x", hRapCh));
			ret = BERR_TRACE(ret); goto error;
		}
#ifdef BCHP_7411_VER  /* only for 7411 */ 
		for ( i = 0; i < BRAP_DEC_DownmixPath_eMax; i++ )
		{
			psDspChSettings->sDspchnExtSettings[i] = pChnSettings->sDspChSettings[i];
		}
#else
	    psDspChSettings->sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain] 
	        = pChnSettings->sDspChSettings;
#endif        
	} /* (!BRAP_GetWatchdogRecoveryFlag(hRap)) */
	else { /* If watchdog recovery */
		hRapCh = *phRapCh;
	}
	
	/* Instantiate the DSP Channel corresponding to the DSP 
	   context resource manager has allocated */
	ret = BRAP_DSPCHN_P_Open(
				&(hRapCh->sModuleHandles.hDspCh),
				hRapCh->sModuleHandles.hDsp,
				hRapCh->sRsrcGrnt.uiDspContextId,		
				psDspChSettings);

	if(ret != BERR_SUCCESS)
	{
	       ret = BERR_TRACE(ret);
		BDBG_ERR(("DSP Channel Open failed for RAP DEC Channel handle 0x%x", hRapCh));
		if (true==bWatchdogRecoveryOn)
		{
			BKNI_Free(psDspChSettings);
			return (ret);
		}
		else
			goto free_rs;
	}

#if BCHP_7411_VER > BCHP_VER_C0
	if (true==hRapCh->bMuxChPairOnI2s)
		uiMaxChPairs = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
	else
		uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#else
    uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#endif		

	/* Open all FMM Modules, as per the resources allocated */
	/* Open all FMM Modules, as per the resources allocated */
	for(i=0; i < uiMaxChPairs; i++)
	{

#if BCHP_7411_VER > BCHP_VER_C0
			if (hRapCh->bMuxChPairOnI2s==false)
			{
#endif		
				if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
				   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
				{
					/* For the current output mode, this output channel pair is not 
					   used, so skip the opening of the modules for this channel */
				
					continue;
				}
#if BCHP_7411_VER > BCHP_VER_C0
		    }
#endif		
		if(false==bWatchdogRecoveryOn) {
		
			/* Form the module settings. */


			/* Ring buffer settings */
			for(j=0; j < BRAP_RM_P_MAX_RBUFS_PER_SRCCH; j++)
			{
        			BRAP_RBUF_P_GetDefaultSettings (&sRbufSettings[j]);
				sRbufSettings[j].sExtSettings.uiWaterMark = pChnSettings->uiWaterMark;
				/* This is a decode channel, so ring buffer allocation will be internal */
				sRbufSettings[j].sExtSettings.pBufferStart = NULL;
				sRbufSettings[j].sExtSettings.uiSize = 0;
                sRbufSettings[j].bProgRdWrRBufAddr = false;
#if BCHP_7411_VER > BCHP_VER_C0
			    if (pChnSettings->bMuxChannelPairsOnI2sConfig)
                {         
                    /* Four times the normal size is required for muxing channels on I2s */
    				sRbufSettings[j].sExtSettings.uiSize = (BRAP_RBUF_P_DEFAULT_SIZE*4);
                }            
#endif		
			}

			/* Source channel settings: Currently it is blank.  */

			/* Mixer settings */
                     BRAP_MIXER_P_GetDefaultSettings(&sMixerSettings);
			sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[i].uiMixerInputId;
#if BCHP_7411_VER > BCHP_VER_C0
			if (hRapCh->bMuxChPairOnI2s==true)
			{
				sMixerSettings.bDisableRampUp = true;
			}
#endif		

			/* Spdif formater settings */
                    BRAP_SPDIFFM_P_GetDefaultSettings (&sSpdifFmSettings);
                    sSpdifFmSettings.sExtSettings = hRap->sOutputSettings[eOutput[i]].sSpdiffmSettings;
            
			/* Output settings */
			switch(eOutput[i])
			{
				case BRAP_OutputPort_eSpdif:
					sSpdifSettings.sExtSettings = 
						hRap->sOutputSettings[eOutput[i]].uOutputPortSettings.sSpdifSettings;
					pOpSettings = &sSpdifSettings;
					break;
				case BRAP_OutputPort_eI2s0:
				case BRAP_OutputPort_eI2s1:
				case BRAP_OutputPort_eI2s2:
					sI2sSettings.sExtSettings = 
						hRap->sOutputSettings[eOutput[i]].uOutputPortSettings.sI2sSettings;
					pOpSettings = &sI2sSettings;
					break;
				case BRAP_OutputPort_eMai:
					sMaiSettings.sExtSettings = 
						hRap->sOutputSettings[eOutput[i]].uOutputPortSettings.sMaiSettings;
					pOpSettings = &sMaiSettings;
					break;
				case BRAP_OutputPort_eFlex:
					sFlexSettings.sExtSettings = 
						hRap->sOutputSettings[eOutput[i]].uOutputPortSettings.sFlexSettings;
					pOpSettings = &sFlexSettings;
					break;
				case BRAP_OutputPort_eDac0:
				case BRAP_OutputPort_eDac1:
					sDacSettings.sExtSettings = 
						hRap->sOutputSettings[eOutput[i]].uOutputPortSettings.sDacSettings;
					pOpSettings = &sDacSettings;
					break;
            	case BRAP_OutputPort_eRfMod:
            	default:
                	BDBG_ERR(("BRAP_DEC_OpenChannel: Output port type %d not supported", 
                    	       eOutput[i] ));
                	ret = BERR_TRACE(BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);
                	goto close_dspchn;
			}
		} /* !BRAP_GetWatchdogRecoveryFlag(hRap) */
		else { /* Watchdog case */
			sMixerSettings.uiMixerInput = hRapCh->sRsrcGrnt.sOpResrcId[i].uiMixerInputId;
		}

		/* Open all internal modules */
		ret = BRAP_P_OpenOpPathFmmModules(
									hRapCh,
									&sRbufSettings[0],
									&sSrcChSettings,
									&sMixerSettings,
									&sSpdifFmSettings,
									pOpSettings,
									&(hRapCh->sRsrcGrnt.sOpResrcId[i]),
									i,
									&(hRapCh->sModuleHandles));
		if(ret != BERR_SUCCESS)
		{
        		ret = BERR_TRACE(ret); 
			BDBG_ERR(("FMM Module Opens failed for RAP DEC Channel handle 0x%x", hRapCh));
			if (true==bWatchdogRecoveryOn)
			{
				BKNI_Free(psDspChSettings);
				return (ret);
			}
			else
				goto close_dspchn;
		}

		BDBG_MSG(("\nhRapCh->sModuleHandles.hOp[i]: %x"
		  "\nhRapCh->sModuleHandles.hMixer[i]: %x"
		  "\nhRapCh->sModuleHandles.hSrcCh[i]: %x"
		  "\nhRapCh->sModuleHandles.hSpdifFm[i]: %x"
		  "\nhRapCh->sModuleHandles.hRbuf[2*i]: %x"
		  "\nhRapCh->sModuleHandles.hRbuf[2*i+1]: %x", 
		  hRapCh->sModuleHandles.hOp[i],
		  hRapCh->sModuleHandles.hMixer[i],
		  hRapCh->sModuleHandles.hSrcCh[i],
		  hRapCh->sModuleHandles.hSpdifFm[i],
		  hRapCh->sModuleHandles.hRBuf[2*i],
		  hRapCh->sModuleHandles.hRBuf[2*i+1]));
        
	}

#if BCHP_7411_VER > BCHP_VER_C0
	if (true==hRapCh->bMuxChPairOnI2s)
	{      
		/* Restore back ring buffer ids of second ring buffers for each of the output ports.
		 * This is required to free the ring buffer resources when channel is closed. */
		hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1] = ui2ndRbufIdLR;
		hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLRSurround].uiRbufId[1] = ui2ndRbufIdLsRs;

            	/* Open second ring buffer of I2S0 by passing start address and size of first
            	 * ring buffer of I2S1. This will make these two ring buffer to use same
            	 * device memory.
            	 */
		hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1] = ui2ndRbufIdLR;

		BRAP_RBUF_P_GetDefaultSettings (&sTempRbufSettings);
		sTempRbufSettings.sExtSettings.uiWaterMark = pChnSettings->uiWaterMark;
		/* Reuse the memory */
		sTempRbufSettings.sExtSettings.pBufferStart = 
			hRapCh->sModuleHandles.hRBuf[BRAP_OutputChannelPair_eLRSurround* BRAP_RM_P_MAX_RBUFS_PER_PORT + 0]->sSettings.sExtSettings.pBufferStart;
		sTempRbufSettings.sExtSettings.uiSize = 
			hRapCh->sModuleHandles.hRBuf[BRAP_OutputChannelPair_eLRSurround* BRAP_RM_P_MAX_RBUFS_PER_PORT + 0]->sSettings.sExtSettings.uiSize;
            	sTempRbufSettings.bProgRdWrRBufAddr = false;

		ret = BRAP_RBUF_P_Open (
			hRapCh->sModuleHandles.hFmm,
			&(hRapCh->sModuleHandles.hRBuf[BRAP_OutputChannelPair_eLR * BRAP_RM_P_MAX_RBUFS_PER_PORT + 1]),
			ui2ndRbufIdLR,
			&sTempRbufSettings);

		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("Could not open ring buffer %d for RAP DEC Channel handle 0x%x", ui2ndRbufIdLR, hRapCh));
			ret = BERR_TRACE(ret); goto close_master;
		}

	}
#endif					
	
	/* Allocate resources for related to the additional port of simultaneous mode or 
	   cloned mode and open them, if configured by the user */
	if ( (hRapCh->bSimulModeConfig == true)
    	|| (hRapCh->eClone != BRAP_P_CloneState_eInvalid)) 
	{
		if (false==bWatchdogRecoveryOn)    
		{	
			/* Allocate the resources for the Simul Mode PT context or for the cloned output */
		
			/* Form the resource requirement */

			/* Initialize the output ports to invald first */
			for(uiChnPair = 0; uiChnPair < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; uiChnPair++)
			{
				sResrcReq.sOpPortReq[uiChnPair].eOpPortType = BRAP_RM_P_INVALID_INDEX;
			}
			sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
			sResrcReq.uiNumOpPorts = 1;	/* Only one port is required for pass-thru */
			sResrcReq.bLastChMono = false;
			sResrcReq.sOpPortReq[BRAP_OutputChannelPair_eLR].eOpPortType = pChnSettings->eSimulModePtOutput;

			/* If output port is MAI then initialize the MAI mux selector */			
			if ( pChnSettings->eSimulModePtOutput == BRAP_OutputPort_eMai )
			{
				sResrcReq.sOpPortReq[BRAP_OutputChannelPair_eLR].eMuxSelect = 
					hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
			}

        	sResrcReq.bSimulModePt = pChnSettings->bSimulModeConfig; 
      sResrcReq.bAllocateDsp = false;  
      sResrcReq.bAllocateRBuf = true;
      sResrcReq.bAllocateSrcCh= true;
      sResrcReq.bAllocateDstCh= false;
     

			/* Call resource manager to allocate required resources. */
			ret = BRAP_RM_P_AllocateResources(	hRap->hRm, 
												&sResrcReq, 
												&(hRapCh->sSimulPtRsrcGrnt));
			if(ret != BERR_SUCCESS)
			{
				BDBG_ERR(("Resource allocation for 2nd context of Simul Mode"
					  	"failed for RAP DEC Channel handle 0x%x", hRapCh));
				ret = BERR_TRACE(ret); goto close_master;
			}

			/* Open all FMM Modules, as per the resources allocated */
		
			/* form the module settings. */

			/* Ring buffer settings */
			for(j=0; j < BRAP_RM_P_MAX_RBUFS_PER_SRCCH; j++)
			{
				sRbufSettings[j].sExtSettings.uiWaterMark = pChnSettings->uiWaterMark;
				/* This is a decode channel, so ring buffer allocation will be internal */
				sRbufSettings[j].sExtSettings.pBufferStart = NULL;
                sRbufSettings[j].sExtSettings.uiSize = 0;
                sRbufSettings[j].bProgRdWrRBufAddr = false;
			}

			/* No need to fill up other parameters of sRbufSettings, 
		   	as memory allocation is internal */ 

			/* Source channel settings: Currently it is blank.  */

			/* Mixer settings */
			sMixerSettings.uiMixerInput = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiMixerInputId;

			/* Spdif formater settings */
                     sSpdifFmSettings.sExtSettings = hRap->sOutputSettings[pChnSettings->eSimulModePtOutput].sSpdiffmSettings;

			/* Output settings */
			switch(pChnSettings->eSimulModePtOutput)
			{
				case BRAP_OutputPort_eSpdif:
					sSpdifSettings.sExtSettings = 
						hRap->sOutputSettings[pChnSettings->eSimulModePtOutput].uOutputPortSettings.sSpdifSettings;
						pOpSettings = &sSpdifSettings;
				break;
				case BRAP_OutputPort_eMai:
					sMaiSettings.sExtSettings = 
						hRap->sOutputSettings[pChnSettings->eSimulModePtOutput].uOutputPortSettings.sMaiSettings;
					pOpSettings = &sMaiSettings;
					break;
            	             default:
					BDBG_ERR(("BRAP_DEC_OpenChannel: Output port type %d "
						 	 "cannot carry compressed data", 
                          	pChnSettings->eSimulModePtOutput ));
                	ret = BERR_TRACE(BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);
                	goto free_slave_rs;
			}
			/* Open all internal modules */
		
			hRapCh->sSimulPtModuleHandles.hDsp = hRapCh->sModuleHandles.hDsp;
			hRapCh->sSimulPtModuleHandles.hFmm = hRapCh->sModuleHandles.hFmm;
			hRapCh->sSimulPtModuleHandles.hDspCh = NULL;
		} /* (!BRAP_WatchdogRecoveryGetFlag(hRap)) */
		else { /* Watchdog case */
			sMixerSettings.uiMixerInput = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiMixerInputId;
		}

		ret = BRAP_P_OpenOpPathFmmModules(
									hRapCh,
									&sRbufSettings[0],
									&sSrcChSettings,
									&sMixerSettings,
									&sSpdifFmSettings,
									pOpSettings,
									&(hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR]),
									0,
									&(hRapCh->sSimulPtModuleHandles));
		if(ret != BERR_SUCCESS)
		{
        		ret = BERR_TRACE(ret); 
			BDBG_ERR(("FMM Module Opens for 2nd context of Simul Mode"
					  "failed for RAP DEC Channel handle 0x%x", hRapCh));
			if (true==bWatchdogRecoveryOn)
			{
				BKNI_Free(psDspChSettings);
				return (ret);
			}
			else
				goto free_slave_rs;
		}

		BDBG_MSG(("\nhRapCh->sSimulPtModuleHandles.hOp[0]: %x"
		  "\nhRapCh->sSimulPtModuleHandles.hMixer[0]: %x"
		  "\nhRapCh->sSimulPtModuleHandles.hSrcCh[0]: %x"
		  "\nhRapCh->sSimulPtModuleHandles.hSpdifFm[0]: %x"
		  "\nhRapCh->sSimulPtModuleHandles.hRbuf[2*0]: %x"
		  "\nhRapCh->sSimulPtModuleHandles.hRbuf[2*0+1]: %x", 
		  hRapCh->sSimulPtModuleHandles.hOp[0],
		  hRapCh->sSimulPtModuleHandles.hMixer[0],
		  hRapCh->sSimulPtModuleHandles.hSrcCh[0],
		  hRapCh->sSimulPtModuleHandles.hSpdifFm[0],
		  hRapCh->sSimulPtModuleHandles.hRBuf[2*0],
		  hRapCh->sSimulPtModuleHandles.hRBuf[2*0+1]));        


	}
	/*else*/

        /* Intialise Raptor interrupt handling */
        ret = BRAP_P_InterruptInstall (hRapCh);
		if(ret != BERR_SUCCESS)
		{
		       ret = BERR_TRACE(ret);
			BDBG_ERR(("Interrupt installation failed for RAP DEC Channel handle 0x%x", hRapCh));
			if (true==bWatchdogRecoveryOn)
			{
				BKNI_Free(psDspChSettings);
				return (ret);
			}
			else
				goto close_slave;
		}
		goto end_open;

close_slave:
	if (   (hRapCh->bSimulModeConfig == true)
        || (hRapCh->eClone == BRAP_P_CloneState_eConfigured))   
	{
    	rc = BRAP_P_CloseOpPathFmmModules(0, &(hRapCh->sSimulPtModuleHandles));
	    BDBG_ASSERT(rc == BERR_SUCCESS);
    }
free_slave_rs:
	if (   (hRapCh->bSimulModeConfig == true)
     || (hRapCh->eClone == BRAP_P_CloneState_eConfigured))   
	{
	    rc = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sSimulPtRsrcGrnt));
           BDBG_ASSERT(rc == BERR_SUCCESS);
    }

    
close_master:
	for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
	{
		rc = BRAP_P_CloseOpPathFmmModules(i, &(hRapCh->sModuleHandles));
		BDBG_ASSERT(rc == BERR_SUCCESS);
	}
close_dspchn:
	/* Close the DSP Channel */
	rc = BRAP_DSPCHN_P_Close(hRapCh->sModuleHandles.hDspCh);
	BDBG_ASSERT(rc == BERR_SUCCESS);
free_rs:
	/* Call Resource manager to release these resources */
	rc = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
        BDBG_ASSERT(rc == BERR_SUCCESS);


error:
	/* Free up the channel handle */
	BKNI_Free(hRapCh);
	
end_open:
       /* only if channel has been successfully opened, save the handle */
       if (ret == BERR_SUCCESS)
           hRap->hRapDecCh[uiChannelNo] = hRapCh ;   
	BKNI_Free(psDspChSettings);
	BDBG_LEAVE(BRAP_DEC_OpenChannel);

	return BERR_TRACE(ret);
}
#endif

/***************************************************************************
Summary:
	API used to close a decode channel.

Description:
	It closes the instance of a decode channel operation. It frees the 
	channel handle and resources associated with it if any.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_OpenChannel
****************************************************************************/
BERR_Code BRAP_DEC_CloseChannel( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	)
{
	BERR_Code		ret = BERR_SUCCESS;
	unsigned int	i;
    unsigned int    uiMaxChPairs;
#ifndef BCHP_7411_VER /* not for 7411 */
       unsigned int j;
       unsigned int uiCount;
    	BRAP_P_ObjectHandles    	sTempHandles;        
    	BRAP_RM_P_ResrcGrant 	sResrcGrant;        
#endif
	BDBG_ENTER(BRAP_DEC_CloseChannel);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

#ifndef BCHP_7411_VER /* not for 7411 */    
       /* Make sure the temp handles etc are clean */
       BKNI_Memset (&sResrcGrant, 0, sizeof(sResrcGrant));
       BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));
#endif

#if 0
    /* Mask interrupts and uninstall callbacks */
    BRAP_P_InterruptUnInstall (hRapCh);
#endif

#if (BRAP_SECURE_HEAP==1)
	BRAP_P_Free(hRapCh->hRap, (void *) hRapCh->hInterruptCount,false);
#else
	BRAP_P_Free(hRapCh->hRap, (void *) hRapCh->hInterruptCount);
#endif

	/* Close the DSP Channel */
	ret = BRAP_DSPCHN_P_Close(hRapCh->sModuleHandles.hDspCh);
	BDBG_ASSERT(ret == BERR_SUCCESS);
	hRapCh->sModuleHandles.hDspCh = NULL;	

#if BCHP_7411_VER > BCHP_VER_C0
	if (true == hRapCh->bMuxChPairOnI2s)
		uiMaxChPairs = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
	else
		uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#else
    uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#endif		

	/* Close internal FMM modules */
	for(i=0; i < uiMaxChPairs; i++)
	{
#if BCHP_7411_VER > BCHP_VER_C0
    	if (false == hRapCh->bMuxChPairOnI2s)
        {   
#endif
		if ( hRapCh->sModuleHandles.hOp[i] == NULL )
		{
			/* There is not valid output port for this channel pair 
			 * So we need not close this channel pair */
			continue;
		}
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if bMuxChPairOnI2s == false */
#endif

#ifndef BCHP_7411_VER /* not for 7411 */
                /* For cloned ports */
	        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
                {

                    if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
                    {

                        /* Form BRAP_P_ObjectHandles structure */
                        sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                        sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                        sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                        sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                        sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                        sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                        sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;                  

                        /* Stop cloned modules associated with this output port */
                        ret = BRAP_P_CloseOpPathFmmModules ((BRAP_OutputChannelPair)i, &sTempHandles);
                        if (ret != BERR_SUCCESS)
                        {
                            BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() for cloned ports failed. Ignoring error!!!!!"));
                            ret = BERR_TRACE (ret);
                        }

                        /* Remove these modules' handle frmo the channel handle */
                        hRapCh->sCloneOpPathHandles[i][j].hSrcCh = NULL;
                        hRapCh->sCloneOpPathHandles[i][j].hMixer = NULL;  
                        hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex = BRAP_RM_P_INVALID_INDEX;  
                        hRapCh->sCloneOpPathHandles[i][j].hSpdifFm = NULL;  
                        hRapCh->sCloneOpPathHandles[i][j].hOp = NULL;          
                        hRapCh->sCloneOpPathHandles[i][j].bSimul= false;

                        /* Initialize the Complete Grant structure	*/			
   			   sResrcGrant.uiNumOpPorts = 1;
   			   for (uiCount = 0; uiCount < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; uiCount++)
   			   {    
   				sResrcGrant.sOpResrcId[uiCount].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
   				sResrcGrant.sOpResrcId[uiCount].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiMixerId = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
   				sResrcGrant.sOpResrcId[uiCount].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
   			   }

			   sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
   			   sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
   			   sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
   			   sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
   			   sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
   			   for(uiCount = 0; uiCount < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; uiCount++)
   				sResrcGrant.sCapResrcId.uiRbufId[uiCount] = BRAP_RM_P_INVALID_INDEX; 	   

                        /* Free the resources in the RM */
                        sResrcGrant.sOpResrcId[i] = hRapCh->sCloneOpPathResc[i][j];    
                        ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
                        if (ret != BERR_SUCCESS)
                        {
                            BDBG_ERR (("BRAP_RemoveOutputPort: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
                            ret = BERR_TRACE (ret);
                        }    

                        /* Clear the Resc Grant struct for this output port!!! */
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

		ret = BRAP_P_CloseOpPathFmmModules((BRAP_OutputChannelPair)i, &(hRapCh->sModuleHandles));
              if (ret != BERR_SUCCESS)
            {
                    BDBG_ERR (("BRAP_DEC_CloseChannel: call to BRAP_P_CloseOpPathFmmModules() failed. Ignoring error!!!!!"));
                    ret = BERR_TRACE (ret);
            }

	}
	
	/* Call Resource manager to release these resources */
	ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sRsrcGrnt));
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_DEC_CloseChannel: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }

	BDBG_MSG(("Resources Freed."));


    

	/* Close the resources allocated for 2nd context of simultaneous mode, 
	   if configured */
	if (   (hRapCh->bSimulModeConfig == true)
#ifdef BCHP_7411_VER  /* only for 7411 */           
        || (hRapCh->eClone == BRAP_P_CloneState_eConfigured)
#endif        
        )     
	{
		ret = BRAP_P_CloseOpPathFmmModules((BRAP_OutputChannelPair)0, &(hRapCh->sSimulPtModuleHandles));
		BDBG_ASSERT(ret == BERR_SUCCESS);

		/* Call Resource manager to release these resources */
		ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(hRapCh->sSimulPtRsrcGrnt));
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_DEC_CloseChannel: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
            ret = BERR_TRACE (ret);
        }
		BDBG_MSG(("SimulPt resources Freed."));
	}

	/* Mark the place for the current channel handle 
	   to 'invalid' inside RAP handle */
	hRapCh->hRap->hRapDecCh[hRapCh->uiChannelNo] = NULL;
	
	/* Free the channel handle */
	BKNI_Free(hRapCh);
	
	BDBG_LEAVE(BRAP_DEC_CloseChannel);

	return (BERR_SUCCESS);
}

/***************************************************************************
Summary:
	Gets the default params for a Decode Channel to start a channel
Description:
	This API returns the default parameters for a Decode Audio channel. 
	
	"AC3 stereo decode on I2S0 output port without Simul Mode and Cloning, 
	 with 24 bits/sample output and Timebase 0" configuration is supported 
	 as default parameters.
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_Start, BRAP_DEC_Stop
***************************************************************************/
BERR_Code BRAP_DEC_GetDefaultAudioParams(
	BRAP_ChannelHandle 	hRapCh,			/* [in] The RAP channel handle */
	BRAP_DEC_AudioParams *pDefParams	/* [out] Default channel parameters */
	)
{
	BERR_Code				ret = BERR_SUCCESS;
	BRAP_DSPCHN_P_AudioParams	sDspChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	BRAP_OP_SpdifChanStatusParams	sSpdifChanStatusParams;

	BDBG_ENTER(BRAP_DEC_GetDefaultAudioParams);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pDefParams);
	BSTD_UNUSED (hRapCh);
	
	/* Get DSP Channel default parameters */
	ret = BRAP_DSPCHN_P_GetDefaultParams(&sDspChParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sDspChParams = sDspChParams.sExtAudioParams;
	else
		return BERR_TRACE(ret); 

	/* Get Mixer default parameters */
	ret = BRAP_MIXER_P_GetDefaultParams(&sMixerParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sAudioOutputParams.sMixerParams = sMixerParams.sExtParams;
	else
		return BERR_TRACE(ret); 

	/* Get SPDIF Formater default parameters */
	ret = BRAP_SPDIFFM_P_GetDefaultParams(&sSpdifFmParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sAudioOutputParams.sSpdifFmParams 
						= sSpdifFmParams.sExtParams;
	else
		return BERR_TRACE(ret); 

	/* Get default SPDIF Channel Status parameters */
	ret = BRAP_OP_P_GetDefaultSpdifChanStatusParams(&sSpdifChanStatusParams);
	if(ret == BERR_SUCCESS)
		pDefParams->sSpdifChanStatusParams 
						= sSpdifChanStatusParams;
	else
		return BERR_TRACE(ret); 


	/* Fillup other default parameters */
	pDefParams->eTimebase = BAVC_Timebase_e0;
	pDefParams->bPlayback = false;
#ifdef BCHP_7411_VER  /* only for 7411 */    
	pDefParams->eOutputMode = BRAP_OutputMode_e2_0;
	pDefParams->bCloneEnable = false;
	pDefParams->bMuxChannelPairsOnI2sEnable = false;
#endif
	pDefParams->sAudioOutputParams.uiOutputBitsPerSample 
		= sMixerParams.uiStreamRes;

#ifdef RAP_SRSTRUVOL_CERTIFICATION
    pDefParams->eInputSamplingRate = BAVC_AudioSamplingRate_e48k;
    pDefParams->uiNumChannels = 2;
#endif

	/* Fill up sSimulPtAudioOutputParams with invalid values, 
	   since Simultaneous Mode is not part of default params */
	BKNI_Memset(&(pDefParams->sSimulPtAudioOutputParams), 
				BRAP_INVALID_VALUE, 
				sizeof(BRAP_AudioOutputParams));

#if BRAP_P_USE_BRAP_TRANS ==0
    /* Fill up sXptContextMap with invalid values, 
	   since PI can't provide these values */
	BKNI_Memset(&(pDefParams->sXptContextMap), 
				BRAP_INVALID_VALUE, 
				sizeof(BAVC_XptContextMap));
#endif

	BDBG_LEAVE(BRAP_DEC_GetDefaultAudioParams);

	return BERR_SUCCESS;
}

#ifdef BCHP_7411_VER
/***************************************************************************
Summary:
	Starts a decode channel.

Description:
	This API is used to start the decode/pass-through/simul /SRC of the selected 
	channel (stream), as specified by input BRAP_DEC_AudioParams structure.
    
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_Stop

****************************************************************************/

BERR_Code BRAP_DEC_Start ( 
	BRAP_ChannelHandle 			hRapCh,			/* [in] The RAP Channel handle */
	const BRAP_DEC_AudioParams	*pAudioParams	/* [in] Audio parameters required 
														for starting this channel */
	)
{

/* Algorithm followed for starting various modules is:

1. Mute associate channel in DP (was Muted in BRAP_DEC_Stop()) 

2. Start the associated Output operation

3. Start the associated Microsequencer operation 

4. Start the associated Mixer operation

5. Unmute the channel (taken care of in BRAP_MIXER_P_Start() which starts 
    with SCALE_MUTE_ENA & VOLUME_MUTE_ENA both unMuted)

6. Start the associated Ring buffers' operation

7. Start the associated Source FIFOs' operation

8. Start the associated DSP Context operation (Program audio PLL, open the gate)
*/

	BERR_Code				ret = BERR_SUCCESS;
	BERR_Code				rc = BERR_SUCCESS;
	BRAP_DSPCHN_P_AudioParams	sDspChParams;
	BRAP_RBUF_P_Params		sRBufParams[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
	BRAP_SRCCH_P_Params		sSrcChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	BRAP_DSPCHN_DecodeMode 	eDecodeMode;
#ifndef BCHP_7411_VER  /* only for 7411 */      
    BRAP_P_ObjectHandles        sTempHandles;  
#endif
	bool					bCompress = false;
	bool					bOutputAlignment = false;	/* If outputs to be aligned */
	void					*pOpParams = NULL;
	unsigned int			i, j, k;
    bool                    bValid = false;
    unsigned int            uiMaxChPairs;
	bool					bIsPCMMixedWithCompressed;
#if BCHP_7411_VER > BCHP_VER_C0
    bool                    bI2s0 = false;
    bool                    bI2s1 = false;
    bool                    bI2s2 = false;    
    BRAP_OP_P_Handle        hI2sOp[BRAP_P_MUXED_I2S_CHANNEL_PAIRS] = {NULL, NULL};
#endif
	bool			bWatchdogRecoveryOn = false;
    BRAP_OutputPort eMaiMuxSelect=BRAP_OutputPort_eMax;  
        
	BDBG_ENTER(BRAP_DEC_Start);

#if BCHP_7411_VER > BCHP_VER_C0
	BSTD_UNUSED(bI2s0);
	BSTD_UNUSED(bI2s1);
	BSTD_UNUSED(bI2s2);
	BSTD_UNUSED(hI2sOp);
#endif	

	/* Check if this is a watchdog recovery. */
	bWatchdogRecoveryOn = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);

    BDBG_MSG(("BRAP_DEC_Start: hRapCh=0x%x, hRapCh->uiChannelNo=%d", 
        hRapCh,hRapCh->uiChannelNo));

	if (false==bWatchdogRecoveryOn) {

	    BDBG_MSG(("BRAP_DEC_Start: Paramters:"));
	    BDBG_MSG(("\t eOutputMode=0x%x", pAudioParams->eOutputMode));
	    BDBG_MSG(("\t eTimebase=0x%x", pAudioParams->eTimebase));

	    BDBG_MSG(("\t sDspChParams.eDecodeMode=0x%x", pAudioParams->sDspChParams.eDecodeMode));
	    BDBG_MSG(("\t sDspChParams.eType=0x%x", pAudioParams->sDspChParams.eType));
	    BDBG_MSG(("\t sDspChParams.eStreamType=0x%x", pAudioParams->sDspChParams.eStreamType));
	    BDBG_MSG(("\t sDspChParams.eAacXptFormat=0x%x", pAudioParams->sDspChParams.eAacXptFormat));
	    BDBG_MSG(("\t sDspChParams.i32AVOffset=0x%x", pAudioParams->sDspChParams.i32AVOffset));

	    BDBG_MSG(("\t sAudioOutputParams.uiOutputBitsPerSample=0x%x", pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
	    BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", pAudioParams->sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
	    BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", pAudioParams->sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
	    BDBG_MSG(("\t sAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", pAudioParams->sAudioOutputParams.sSpdifFmParams.bSpdifFormat));

	    BDBG_MSG(("\t sSimulPtAudioOutputParams.uiOutputBitsPerSample=0x%x", pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", pAudioParams->sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", pAudioParams->sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", pAudioParams->sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat));

	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bProfessionalMode=0x%x", pAudioParams->sSpdifChanStatusParams.bProfessionalMode));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum=0x%x", pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSwCopyRight=0x%x", pAudioParams->sSpdifChanStatusParams.bSwCopyRight));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16CategoryCode=0x%x", pAudioParams->sSpdifChanStatusParams.ui16CategoryCode));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16ClockAccuracy=0x%x", pAudioParams->sSpdifChanStatusParams.ui16ClockAccuracy));

	    BDBG_MSG(("\t bPlayback=0x%x", pAudioParams->bPlayback));
	}


#if BCHP_7411_VER > BCHP_VER_C0
	if (false==bWatchdogRecoveryOn)
	{
		if (true == pAudioParams->bMuxChannelPairsOnI2sEnable)
    			uiMaxChPairs = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
    		else
    			uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
	}
	else
	{
		/* If it is watchdog recovery, then pAudioParams is NULL.
		 * Get required information from channel handle. */
		if (true == hRapCh->bMuxChPairOnI2sStarted)
    			uiMaxChPairs = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
    		else
    			uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
	}
#else
   	uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#endif	

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);
	if (false==bWatchdogRecoveryOn) 
     {
		BDBG_ASSERT(pAudioParams);

	BDBG_MSG (( "BRAP_DEC_Start():"
                "hRapCh=0x%x",
                hRapCh));

	/* If channel already started return an error */
	if (hRapCh->bStarted) {
		BDBG_ERR(("Channel already started"));
		return BERR_SUCCESS;
	}

    eMaiMuxSelect = 
        hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;

#if 0
	else
		hRapCh->bStarted = true;
#endif

#ifdef BCHP_7411_VER
#if BCHP_7411_VER==BCHP_VER_C0
	if (pAudioParams->bMuxChannelPairsOnI2sEnable==true)
	{
		BDBG_ERR((" Channel pair muxing on I2S can be enabled only for 7411 D0."));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
#endif	
	if ((pAudioParams->bMuxChannelPairsOnI2sEnable==true) && (hRapCh->bMuxChPairOnI2s==false))
	{
		BDBG_ERR((" To enable channel pair muxing on I2S, set bMuxChannelPairsOnI2sConfig"
			" in BRAP_DEC_ChannelSettings while opening a decode channel"));
		return BERR_TRACE( BERR_INVALID_PARAMETER );
	}
	hRapCh->bMuxChPairOnI2sStarted = pAudioParams->bMuxChannelPairsOnI2sEnable;
#endif

	
    /* Make sure atleast one output port is attached to this channel */
    for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
    	if(hRapCh->sModuleHandles.hOp[i] != NULL) 
    	{
            bValid = true;
            break;
    	}
    }    
    if (bValid == false)
    {
        BDBG_ERR(("No output ports are connected to this channel!!! First add an output port then start!!"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
	goto return_error;
    }

    /* Check DecodeMode */
    switch (pAudioParams->sDspChParams.eDecodeMode)
    {
        /* Modes supported:  */
        case BRAP_DSPCHN_DecodeMode_eDecode:
        case BRAP_DSPCHN_DecodeMode_ePassThru:
        case BRAP_DSPCHN_DecodeMode_eSimulMode:
             break;
        /* Not supported */
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Decode mode %d not supported", 
						  pAudioParams->sDspChParams.eDecodeMode));
		        	ret = BERR_TRACE(BERR_INVALID_PARAMETER);
				goto return_error;
    }

    	
	/* For debug purpose */
	BDBG_ASSERT(hRapCh->sModuleHandles.hDspCh);

		/* For debug purpose */
        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
		BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]);
    		BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hRBuf[0] && hRapCh->sSimulPtModuleHandles.hRBuf[1]);
        
        BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]);
	   	BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hMixer[BRAP_OutputChannelPair_eLR]);
	   	BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hSpdifFm[BRAP_OutputChannelPair_eLR]);    
        }
        
	if ( (hRapCh->hRap->uiDecodeCount + hRapCh->hRap->uiPassThruCount) >=
		(BRAP_RM_P_MAX_DSPS * BRAP_RM_P_MAX_CXT_PER_DSP)) {
		BDBG_ERR(("No more bandwidth to perform decode mode requested. Please see"
			"comments in brap.h for various combinations of decode modes supported in"
			"decode channels."));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;
	}	
		
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) ||
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		if (hRapCh->hRap->uiDecodeCount >= BRAP_RM_P_MAX_DECODE_SUPPORTED) {
			BDBG_ERR(("Number of decode operations exceeding maximum number of decode"
				"operations supported %d. Please see comments in brap.h module overview for various combinations"
				"of decode modes supported in decode channels.", BRAP_RM_P_MAX_DECODE_SUPPORTED));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;
		}
	}
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) || 
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		if (hRapCh->hRap->uiPassThruCount >= BRAP_RM_P_MAX_PASSTHRU_SUPPORTED) {
			BDBG_ERR(("Number of passthru operations exceeding maximum number of passthru"
				"operations supported %d. Please see comments in brap.h module overview for various combinations"
				"of decode modes supported in decode channels.", BRAP_RM_P_MAX_DECODE_SUPPORTED));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;
		}
	}
			
    /* Check algo type: 
       NOTE: This  list should be constantly updated to refelct current status for 
       each platform */
    switch (pAudioParams->sDspChParams.eType)
    {
        /* Algo types supported:  */
        case BRAP_DSPCHN_AudioType_eAc3:
        case BRAP_DSPCHN_AudioType_eMpeg:  			
        case BRAP_DSPCHN_AudioType_eAac:
	 case BRAP_DSPCHN_AudioType_eAacSbr:
	 case BRAP_DSPCHN_AudioType_eAc3Plus:
#if (BCHP_7411_VER != BCHP_VER_C0)      /* include WMA for 7411D0, 7401 and 7400 */      
        case BRAP_DSPCHN_AudioType_eWmaStd:         
#endif     
#if (BRAP_7401_FAMILY == 1)
        case BRAP_DSPCHN_AudioType_eWmaPro:         						
#endif     

#ifdef BCHP_7411_VER /* For the 7411 */ 
		case BRAP_DSPCHN_AudioType_eDts:
#if	BCHP_7411_VER > BCHP_VER_C0		
		case BRAP_DSPCHN_AudioType_eLpcmBd:
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:    
		case BRAP_DSPCHN_AudioType_eLpcmDvd:    
		case BRAP_DSPCHN_AudioType_eDtshd:
        	case BRAP_DSPCHN_AudioType_eAc3Lossless:
		case BRAP_DSPCHN_AudioType_eMlp:
#endif			
#endif
             break;
        /* Not supported */
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Algo type %d not supported", 
						  pAudioParams->sDspChParams.eType));
		         	ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;	
    }
	/* Check if algo type was selected at BRAP_Open time. Otherwise we may
	 * not have sufficient memory for this algo */
	 if (hRapCh->hRap->sSettings.bSupportAlgos[pAudioParams->sDspChParams.eType]==false) {
	 	BDBG_ERR(("Audio type %d was not selected at BRAP_Open. This audio type can't be run.",
			pAudioParams->sDspChParams.eType));
		ret = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto return_error;	
	 }

	/* Check Transport Stream type: 
       NOTE: This  list should be constantly updated to refelct current status for 
       each platform */
    switch (pAudioParams->sDspChParams.eStreamType)
    {
        /* Stream types supported:  */
        case BAVC_StreamType_eTsMpeg:
        case BAVC_StreamType_eDssEs:
        case BAVC_StreamType_eDssPes:
        case BAVC_StreamType_ePS:
        case BAVC_StreamType_ePes:
             break;
        /* Not supported */
        case BAVC_StreamType_eEs:
        case BAVC_StreamType_eBes:
        case BAVC_StreamType_eCms:
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Stream type %d not supported", 
						  pAudioParams->sDspChParams.eStreamType));
		        	ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;		
    }

#ifdef BCHP_7411_VER  /* only for 7411 */        
 	if ( (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
         && (pAudioParams->bCloneEnable == true))          
    {
		BDBG_ERR(("Cloning is not supported in SimulMode. ",
		          "Either Disable Cloning or start the channel in Decode Mode.\n"));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;	
    }
#endif
    
	if((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode) 
        && (hRapCh->bSimulModeConfig == false))
	{
		BDBG_ERR(("No Output port has been reserved for the compressed data yet. So cant start in Simul Mode."));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;	
	}

	/* Simulmode and PassThru are not supported for following algorithms.
	 * AAC-SBR
	 * LPCM-BD, LPCM-DVD
	 * Return error if decode mode is simulmode or passthru for these algorithms.
	 */
    if ((pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAacSbr) 
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmBd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmHdDvd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmDvd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eWmaStd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eWmaPro)
#ifdef RAP_SRSTRUVOL_CERTIFICATION            	
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_ePCM)	
#endif	
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eMlp))
    { 
        if ((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
            || (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
        {
        
                BDBG_ERR(("BRAP_DEC_Start: Compressed data cant be sent on the SPDIF output port for algorithm %d",
					pAudioParams->sDspChParams.eType));
              ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
		goto return_error;			
        }        
    }

#if (BCHP_CHIP == 7400)
	if((pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus)
     &&(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
    {
        BDBG_WRN(("BRAP_DEC_Start: NOTE: For DDP, in Simul Mode, we transcode"
        " and send out compressed AC3 data on the SPDIF"));            
    }
#else
    /* For DDP ie AC3plus, if it is pure passthru - disallow. 
    in simul mode, the FW will automatically transcode to AC3 and send out compressed AC3. */
	if (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus)
    { 
        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)          
        {
        
            BDBG_ERR(("BRAP_DEC_Start: Compressed data cant be sent on the SPDIF output port for algorithm %d",
                pAudioParams->sDspChParams.eType));
            ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
            goto return_error;          
        }        
        else if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
           BDBG_WRN(("BRAP_DEC_Start: NOTE: For DDP, in Simul Mode, we transcode"
            " and send out compressed AC3 data on the SPDIF/HDMI"));        
        }              
    }
#endif


	/* Check for supported bit resolutions.
	   For I2S and Flex output, it can be between 16 bits/sample to 32 bits/sample,
	   For other outputs, it can be between 16 bits/sample to 24 bits/sample */
	if((pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_I2S_MAX_BITS_PER_SAMPLE) ||
	   (pAudioParams->sAudioOutputParams.uiOutputBitsPerSample < BRAP_P_MIN_BITS_PER_SAMPLE))
	{
		BDBG_ERR(("Output bit resolution %d should be between 16 to 32 bits/sample", 
                   pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;	
	}
	else if(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_MIXER_MAX_BITS_PER_SAMPLE)
	{
		for(i=0; i < uiMaxChPairs; i++)
		{
#if BCHP_7411_VER > BCHP_VER_C0
        	if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
    		{
#endif    		
    		    if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
    			   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
    			{
    				/* For the current output mode, this output channel pair is not 
    				   used, so skip the opening of the modules for this channel */
    				continue;
    			}
#if BCHP_7411_VER > BCHP_VER_C0
            }/* if pAudioParams->bMuxChannelPairsOnI2sEnable == false */
#endif        
			if((hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s0) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s1) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s2) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eFlex))
			{
				/* Only I2S and Flex can have more than 24 bits/sample output.
				   So for other outputs return error */
				BDBG_ERR(("Output bit resolution %d should be between 16 to 24 bits/sample", 
                           pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
				ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;	
			}
		}
	}

	/* If configured, check for supported bit resolutions for the pass thru context of  
	   simultaneous mode / clone port. It should be between 16 bits/sample to 24 bits/sample, 
	   as the output can be SPDIF or MAI. */

	
#ifdef BCHP_7411_VER  /* only for 7411 */         
         if  (pAudioParams->bCloneEnable == true)
	{
        	if ( (pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample > BRAP_P_MIXER_MAX_BITS_PER_SAMPLE) 
    	         || (pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample < BRAP_P_MIN_BITS_PER_SAMPLE))
            {
                BDBG_ERR(("Output bit resolution of cloned port"
    			        " %d should be between 16 to 24 bits/sample", 
                        pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample));
    		     ret = BERR_TRACE(BERR_NOT_SUPPORTED);
    			goto return_error;	 
            }
	}    
#endif

    if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    {
        if (pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample 
                != BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA) 
        {
            BDBG_WRN(("Compressed data can have only 16 bits pers sample. Overruling user supplied bits per sample (%d).", 
                    pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample));
            /*pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample 
                    = BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA;            */

        }
    }

#if BRAP_P_USE_BRAP_TRANS ==1
	/* Check if the timebase corresponds to the 7411 Input port. 
	   This check is for 7411 only */
	if(inPortTimebaseMapping[hRapCh->hRapTransCh->sSettings.eInPort] 
		!= pAudioParams->eTimebase)
	{
		BDBG_ERR(("Timebase %d don't match with the timebase %d "
				  "corresponding to 7411 input port %d", 
                  pAudioParams->eTimebase, 
				  inPortTimebaseMapping[hRapCh->hRapTransCh->sSettings.eInPort], 
				  hRapCh->hRapTransCh->sSettings.eInPort));
		ret = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto return_error;	
	}
#endif


/* Make sure that the application doesnt try to send compressed data on a port that doesnt support it */
		if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru ) 
		{
		    if ((hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eSpdif) 
#ifndef BCHP_7411_VER			
                        && (hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eMai) 
#endif              
                     )
		    {
			BDBG_ERR(("PassThru mode %d "
					  "does not allow to use the output port %d since it cannot carry compressed data", 
	              pAudioParams->sDspChParams.eDecodeMode,
	              hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort));
			ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
			goto return_error;	
                }
		}
		if ( pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode )
		{
			if ( (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eSpdif)
#ifndef BCHP_7411_VER			
                        && (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eMai) 
#endif              
                     )               
			{
				BDBG_ERR(("Simul %d "
						  "does not allow to use the output port %d for compressed data", 
		              pAudioParams->sDspChParams.eDecodeMode,
		              hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort));
				ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
				goto return_error;	
			}
		}
	

#if BCHP_7411_VER > BCHP_VER_C0
		if ((pAudioParams->sDspChParams.bPtsSpecificDecode==true) &&
			(pAudioParams->bPlayback==false))
		{
			BDBG_ERR(("PTS specific decode is supported only in playback case, not in live decode"));
			ret = BERR_TRACE(BERR_INVALID_PARAMETER);
			goto return_error;	
		}
#else
		if (pAudioParams->sDspChParams.bPtsSpecificDecode==true) 
		{
			BDBG_ERR(("PTS specific decode feature is supported only for 7411 D0"));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;	
		}
#endif

	} /* false==bWatchdogRecoveryOn */
	
	/* Check if alignment between outputs is required */
	if(hRapCh->eMaxOpMode > BRAP_OutputMode_e2_0)
	{
		/* Multiple output ports are needed. So alignment required. */
		BDBG_MSG(("Output alignment required"));
		bOutputAlignment = true;
	}

#if BCHP_7411_VER > BCHP_VER_C0
    if (true ==hRapCh->bMuxChPairOnI2sStarted)
	{
		/* I2S0 and I2S1 output ports are needed. So alignment required. */
		BDBG_MSG(("I2S0 and I2S1 Output alignment required"));
		bOutputAlignment = true;
    }
#endif    	

	if (false==bWatchdogRecoveryOn) {
		/* If BRAP_DEC_Start is getting called from within PI (internal call), don't reset
		 * trick mode states. Reset these states for external call.
		 */
		if (!BRAP_P_GetInternalCallFlag(hRapCh)) {
			/* Clear some of the channel states */
			hRapCh->sTrickModeState.bAudioPaused = false;
			hRapCh->sTrickModeState.uiFrameAdvResidualTime = 0;
		}		

#if BRAP_P_USE_BRAP_TRANS ==0
		/* XPT Channel number for the current channel. This is used by the 
		   DSP Firmware to determine the CDB and ITB used for the current 
		   DSP Context  */
		hRapCh->uiXptChannelNo = pAudioParams->sXptContextMap.ContextIdx;
#endif

		eDecodeMode = pAudioParams->sDspChParams.eDecodeMode;

		/* For Passthru mode, the first context ie context 0 has to carry compressed data.
		 For Decode and Simul mode, context 0 carries PCM data*/
		if(eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
			bCompress = true;
		else
			bCompress = false;

		/* Form internal FMM module params */
		
		/* SPDIF formatter Parameters */

		/* Get the default parameters first */
		ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
		if(ret != BERR_SUCCESS){goto end_start;} 
		
		/* Fill up the required parameters */
		sSpdifFmParams.sExtParams = pAudioParams->sAudioOutputParams.sSpdifFmParams;
		sSpdifFmParams.bCompressed = bCompress;
	    	sSpdifFmParams.bSeparateLRChanNum = 
	                pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum;

	    	ret = BRAP_P_GetBurstRepetitionPeriodForAlgo(
	                        pAudioParams->sDspChParams.eStreamType,
	                        pAudioParams->sDspChParams.eType,
	                        &sSpdifFmParams.eBurstRepPeriod);
	    	if(ret != BERR_SUCCESS){goto end_start;} 
	    
		/* Mixer parameters */
		
		/* Get the default parameters first */
		ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
		if(ret != BERR_SUCCESS){goto end_start;} 
		
		/* Fill up the required parameters */
		sMixerParams.sExtParams = pAudioParams->sAudioOutputParams.sMixerParams;
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
			sMixerParams.uiStreamRes 
				= pAudioParams->sAudioOutputParams.uiOutputBitsPerSample;


		/* RBuf parameters, use same parameters for all associated ring buffers */
		    for(k=0; k< BRAP_RM_P_MAX_RBUFS_PER_SRCCH; k++)
		    {
			    ret = BRAP_RBUF_P_GetDefaultParams (&sRBufParams[k]);
			    if(ret != BERR_SUCCESS){goto end_start;}    
		    }

		/* Source channel parameters */

		/* Get the default parameters first */
		ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
		if(ret != BERR_SUCCESS){goto end_start;} 

	    /* Normal playback path: data is not written to srcFIFO directly */
	    sSrcChParams.bCapDirect2SrcFIFO = false;
		
		/* Fill up the required parameters */
		sSrcChParams.bCompress = bCompress;

		/* Start playing when write pointer crosses SOURCE_RINGBUF_x_START_WRPOINT */
		sSrcChParams.bStartSelection = true; 

		if(bCompress)
		{
			/* Set bits per sample = 16, as DSP generates 16 bits per sample for compressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
		}
		else
		{
			/* Set bits per sample = 32, as DSP generates 32 bits per sample for PCM data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;
		}
			
		sSrcChParams.bInputSrValid = true;  /* Input audio data coming from DSP */

	    	sSrcChParams.bSharedRbuf = false;    

		/* For a decode channel, no need to program eInputSR & eOutputSR, 
		   as this is going to be programmed by DSP */

	} /* false==bWatchdogRecoveryOn */

#if BCHP_7411_VER > BCHP_VER_C0
  	if (true == hRapCh->bMuxChPairOnI2sStarted && true == bOutputAlignment)
    {
        /* Allign I2s output ports */        
        ret = BRAP_OP_P_AlignPorts( hRapCh->hRap, 
                                    false, /* bSpdif */ 
                                    true, /* bI2s0 */   
                                    true, /* bI2s1 */   
                                    true  /* bI2s2 */);
        if(ret != BERR_SUCCESS) {goto end_start;}
    }/* if pAudioParams->bMuxChannelPairsOnI2sEnable */
#endif

	/* Start Fmm modules */
	
	for(i=0; i < uiMaxChPairs; i++)
	{

	if (false==bWatchdogRecoveryOn) {
#if BCHP_7411_VER > BCHP_VER_C0
        if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
		{
#endif    		
    		if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
    		   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
    		{
    			/* For the current output mode, this output channel pair is not 
    			   used, so skip the opening of the modules for this channel */
    			continue;
    		}
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if pAudioParams->bMuxChannelPairsOnI2sEnable == false */   
#endif    		
	
	/*Checking if the PCM data is being tried to mix with compressed data*/
	ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
												hRapCh->sModuleHandles.hOp[i]->eOutputPort,
												bCompress,
												&bIsPCMMixedWithCompressed);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
		goto return_error;
	}

	if(bIsPCMMixedWithCompressed)
	{
	    ret = BERR_NOT_SUPPORTED;
		BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data"));
		goto return_error;
	}
        sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiSrcChId;

        ret = BRAP_P_FormOpParams (hRapCh, 
                hRapCh->sModuleHandles.hOp[i]->eOutputPort, 
                pAudioParams->eTimebase, 
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
		

#if BCHP_7411_VER > BCHP_VER_C0
    	if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
        {
#endif        
    		/* Source channel params: Set bMono to TRUE only if only one channel is used */
    		if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == true) && 
    		   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == true))
    		{
    			sSrcChParams.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;
    		}
    		else
    		{
    			sSrcChParams.eBufDataMode = BRAP_BufDataMode_eMono;
    		}
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if pAudioParams->bMuxChannelPairsOnI2sEnable == false */
        else
        {
            /* bMuxChannelPairsOnI2sEnable is true: Special case where single ring buffer will be
               used per output port in stereo interleaved mode */
  			sSrcChParams.eBufDataMode = BRAP_BufDataMode_eStereoInterleaved;
        }
#endif        

		/* Ring buffers to be mapped to this source channel */
		sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiRbufId[0];
		sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiRbufId[1];

		/* For debug purpose */
		BDBG_ASSERT(hRapCh->sModuleHandles.hSrcCh[i]);
        BDBG_ASSERT(hRapCh->sModuleHandles.hOp[i]);
		BDBG_ASSERT(hRapCh->sModuleHandles.hMixer[i]);

#if BCHP_7411_VER > BCHP_VER_C0
    	if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
        {
#endif        
        	BDBG_ASSERT(hRapCh->sModuleHandles.hRBuf[i*2] && hRapCh->sModuleHandles.hRBuf[(2*i)+1]);
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if bMuxChPairsOnI2s == false */
#endif    

		if(((hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eSpdif) 
                    || (hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eMai))
			&& (hRapCh->sModuleHandles.hSpdifFm[i] == NULL))
		{
                    ret = BERR_TRACE (BRAP_ERR_BAD_DEVICE_STATE);
                    goto end_start;
		}

		} /* false==bWatchdogRecoveryOn */
		/* Start the internal FMM modules */
		ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							i,
							&(hRapCh->sModuleHandles));
		if(ret != BERR_SUCCESS){goto end_start;}
	}

#if BCHP_7411_VER > BCHP_VER_C0
    	if (true == hRapCh->bMuxChPairOnI2sStarted)
    	{
		ret = BRAP_P_I2sAlignmentWorkaround(hRapCh);
		if(ret != BERR_SUCCESS) 
			goto end_start;
	}	
#endif	
	
#ifndef BCHP_7411_VER /* not for 7411 */
       /* Start all Cloned ports (for main context) associated with this channel */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {    
                if ((hRapCh->sCloneOpPathHandles[i][j].hOp != NULL) 
                    && (hRapCh->sCloneOpPathHandles[i][j].bSimul == false))
                {      
                    BDBG_MSG(("BRAP_DEC_Start: output port %d was cloned for channel pair %d", j, i));
                    if (false==bWatchdogRecoveryOn) 
                    {                      
                        ret = BRAP_P_FormOpParams (hRapCh, j, pAudioParams->eTimebase, 
                                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                                &pOpParams);

                        if (ret!=BERR_SUCCESS)
                        {
                            ret = BERR_TRACE(ret);
                            goto stop_master;
                        }
                
                        /* ReUse the same SpdifFmParams as filled in for the main port above */
                        /* ReUse the same MixerParams as filled in for the main port above, except SrcChId */
                        if (hRapCh->sCloneOpPathHandles[i][j].hSrcCh != NULL)
                        {
                            /* cloned port uses another SrcCh */
                            /* take the same Params as the orig src ch */
                            sSrcChParams = hRapCh->sModuleHandles.hSrcCh[i]->sParams;

                            /* We need to store the RBUF ID's, as in 7400 the Cloned 
                               channel uses separate RBUF's allocated to it.*/
                                sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[i][j].uiRbufId[0];
                                sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[i][j].uiRbufId[1];
                                BDBG_MSG(("sSrcChParams.uiLtOrSnglRBufId=%d,sSrcChParams.uiRtRBufId=%d,output=%d",
                                        sSrcChParams.uiLtOrSnglRBufId,sSrcChParams.uiRtRBufId,j));

                           /* this Src Ch shares the rbufs with the Orig SrCh */
                           sSrcChParams.bSharedRbuf = true;
                           sSrcChParams.uiMasterSrcCh = 
                                            hRapCh->sModuleHandles.hSrcCh[i]->uiIndex;
               
                           sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[i][j].uiSrcChId ;     
                           BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", j, sMixerParams.uiSrcChId, sSrcChParams.uiMasterSrcCh));                       
                        }
                        else
                        {
                                sMixerParams.uiSrcChId = hRapCh->sModuleHandles.hSrcCh[i]->uiIndex; 

			            for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
			            {    
			                if (hRapCh->sCloneOpPathHandles[i][k].hSrcCh != NULL)
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
			                if (hRapCh->sCloneOpPathResc[i][j].uiDataPathId
			                        == hRapCh->sCloneOpPathResc[i][k].uiDataPathId)

			                {
			                    sMixerParams.uiSrcChId = hRapCh->sCloneOpPathHandles[i][k].hSrcCh->uiIndex;
			                }
			            }
							
                                BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
                        }
                    }
                    else
                    {
                        sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
                    }
                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                    sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                    sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                    sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;   
                       

        	        /*Checking if the PCM data is being tried to mix with compressed data*/
        	        ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
        			    									j,
        					    							sTempHandles.hMixer[i]->bCompress,
        				    								&bIsPCMMixedWithCompressed);
        	
        	        if(ret != BERR_SUCCESS)
        	        {
        		        BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error for Clonned output"));
        		        goto stop_master;
        	        }

        	        if(bIsPCMMixedWithCompressed)
        	        {
        	            ret = BERR_NOT_SUPPORTED;
        		        BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for Clonned output"));
        		        goto stop_master;
        	        }
        	
                    BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
                    /* Start the internal FMM modules */
                    ret = BRAP_P_StartOpPathFmmModules(
                                    hRapCh,
                                    &sRBufParams[0],
                                    &sSrcChParams,
                                    &sMixerParams,
                                    &sSpdifFmParams,
                                    pOpParams,
                                    i,
                                    &(sTempHandles));
                    if(ret != BERR_SUCCESS){goto stop_master;}

                }            
            }
        }
#endif /* not for 7411 */

	if (false==bWatchdogRecoveryOn) { /* TODO: Handle the watchdog case */
	/* Start the modules of 2nd context of simultaneous mode or the clone if configured */
	if ( (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
#ifdef BCHP_7411_VER  /* only for 7411 */           
         || (pAudioParams->bCloneEnable == true)
#endif         
         )    
	{
		/* Should Mute Mixer ouput before starting Output block, MS and Mixer.
		But since Mixer hasnt been started yet, cant do so here. Instead, we
		make sure that when Mixer is started, it is Muted by default */
		
		/* Start the associated output block */

        ret = BRAP_P_FormOpParams (hRapCh, 
                hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort, 
                pAudioParams->eTimebase, 
                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                &pOpParams);

        if (ret!=BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto stop_clone;
        }

		/* SPDIF Formater parameters */
		
		/* Get the default parameters first */
		ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
		if(ret != BERR_SUCCESS){goto stop_clone;}
        
		/* Fill up the required parameters */
		sSpdifFmParams.sExtParams 
			= pAudioParams->sSimulPtAudioOutputParams.sSpdifFmParams;
        ret = BRAP_P_GetBurstRepetitionPeriodForAlgo(
                        pAudioParams->sDspChParams.eStreamType,
                        pAudioParams->sDspChParams.eType,
                        &sSpdifFmParams.eBurstRepPeriod);
        if(ret != BERR_SUCCESS){goto stop_clone;} 
		
		/* Get the default mixer parameters*/
		ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
		if(ret != BERR_SUCCESS){goto stop_clone;} 
		
		/* Get the default source channel parameters */
		ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
		if(ret != BERR_SUCCESS){goto stop_clone;} 

        /* Normal playback path: data is not written to srcFIFO directly */
        sSrcChParams.bCapDirect2SrcFIFO = false;

        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
            /* It is pass-thru context of simul mode, so data is compressed */
    		sSpdifFmParams.bCompressed = true;
            sMixerParams.bCompress = true;	
		    sSrcChParams.bCompress = true;		
			sSrcChParams.bStartSelection = true;		
            sMixerParams.uiSrcChId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiSrcChId;
                    sMixerParams.uiStreamRes  = BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA;

			/* Set bits per sample = 16, as DSP generates 16 bits per sample for compressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;

			/* Ring buffers to be mapped to this source channel */
			sSrcChParams.uiLtOrSnglRBufId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0];
			sSrcChParams.uiRtRBufId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1];
        }
        else
        {
            sSpdifFmParams.bCompressed = false;
            sMixerParams.bCompress = false;	
	     sSrcChParams.bCompress = false;		
            sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiSrcChId;        

			/* Set bits per sample = 32, as DSP generates 32 bits per sample for uncompressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;

			/* Ring buffers to be mapped to this source channel */
			sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0];
			sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1];
        sMixerParams.uiStreamRes 
			= pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample;
            
		}
		
        /* whether or not to override CBIT[20:21] for left/right channel num */
        sSpdifFmParams.bSeparateLRChanNum = 
                pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum;
        
		/* Mixer parameters */
		sMixerParams.sExtParams 
			= pAudioParams->sSimulPtAudioOutputParams.sMixerParams;
	
		/* RBuf parameters, use same parameters for all associated ring buffers */
        for(k=0; k< BRAP_RM_P_MAX_RBUFS_PER_SRCCH; k++)
        {
	        ret = BRAP_RBUF_P_GetDefaultParams (&sRBufParams[k]);
	        if(ret != BERR_SUCCESS){goto stop_clone;}    
        }

		/* Source channel parameters */
		sSrcChParams.bInputSrValid = true;  /* Input audio data coming from DSP */
		sSrcChParams.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;	/* Stereo output by default */

		/* For a decode channel, no need to program eInputSR & eOutputSR, 
		   as this is going to be programmed by DSP */


	/*Checking if the PCM data is being tried to mix with compressed data*/
	ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
												hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort,
												sMixerParams.bCompress,
												&bIsPCMMixedWithCompressed);
	
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error for simultaneous mode "));
		goto stop_clone;
	}

	if(bIsPCMMixedWithCompressed)
	{
	    ret = BERR_NOT_SUPPORTED;
		BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for simultaneous mode "));
		goto stop_clone;
	}

		/* Start the internal FMM modules */
		ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							BRAP_OutputChannelPair_eLR,
							&(hRapCh->sSimulPtModuleHandles));
		if(ret != BERR_SUCCESS){
			if (true==bWatchdogRecoveryOn)
				return BERR_TRACE(ret);
			else
				goto stop_clone;
		}
        

#ifndef BCHP_7411_VER /* not for 7411 */
       /* Start all Cloned ports (for simulPt context) associated with this channel */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {    
            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp != NULL) 
                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == true))
            {      
                BDBG_MSG(("BRAP_DEC_Start: output port %d was cloned for SimultPT %d", j));
                
                ret = BRAP_P_FormOpParams (hRapCh, 
                        hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp->eOutputPort, 
                        pAudioParams->eTimebase, 
                        pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                        &pOpParams);

                /* ReUse the same SpdifFmParams as filled in for parent SimulPt port above */
                /* ReUse the same MixerParams as filled in for the parent SimulPt port above, except SrcChId */
                if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh != NULL)
                {
                    /* cloned port uses another SrcCh */
                    /* take the same Params as the orig src ch */
                    sSrcChParams = hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams;

                    sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiRbufId[0];
                    sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiRbufId[1];
                    BDBG_MSG(("sSrcChParams.uiLtOrSnglRBufId=%d,sSrcChParams.uiRtRBufId=%d,output=%d",
                            sSrcChParams.uiLtOrSnglRBufId,sSrcChParams.uiRtRBufId,j));

                   /* this Src Ch shares the rbufs with the Orig SrCh */
                   sSrcChParams.bSharedRbuf = true;
                   sSrcChParams.uiMasterSrcCh = 
                                    hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->uiIndex;
       
                   sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiSrcChId ;     
                   BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", j, sMixerParams.uiSrcChId, sSrcChParams.uiMasterSrcCh));                       
                }
                else
                {
                    sMixerParams.uiSrcChId = hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->uiIndex; 

                    for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
                    {    
                        if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hSrcCh != NULL)
                        { 
                            break;
                        }
                    }
                    if (k!=BRAP_RM_P_MAX_OUTPUTS)
                    {
                        /* One of the cloned ports has a specially allocated hSrcCh
                        => one port was cloned in another DP, say DPx */
                        /* if eOutputPort belongs to DPx, it should use 
                        hSrcCh from sCloneOpPathHandles, else from sSimulPtModuleHandles */
                        if (hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiDataPathId
                            == hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][k].uiDataPathId)

                        {
                            sMixerParams.uiSrcChId = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hSrcCh->uiIndex;
                        }
                    }							
                    BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
                }            

                /* Form BRAP_P_ObjectHandles structure */
                sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[0];
                sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR + 1] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[1]; 
                sTempHandles.hSrcCh[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;
                sTempHandles.hMixer[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
                sTempHandles.uiMixerInputIndex[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].uiMixerInputIndex;  
                sTempHandles.hSpdifFm[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSpdifFm;  
                sTempHandles.hOp[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;   
                   

    	        /*Checking if the PCM data is being tried to mix with compressed data*/
    	        ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
    			    									j,
    					    							sTempHandles.hMixer[BRAP_OutputChannelPair_eLR]->bCompress,
    				    								&bIsPCMMixedWithCompressed);
    	
    	        if(ret != BERR_SUCCESS)
    	        {
    		        BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM failed for Cloned Simul PT output"));
    		        goto stop_simul;
    	        }

    	        if(bIsPCMMixedWithCompressed)
    	        {
    	            ret = BERR_NOT_SUPPORTED;
    		        BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for Cloned output"));
    		        goto stop_simul;
    	        }
    	
                BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
                /* Start the internal FMM modules */
                ret = BRAP_P_StartOpPathFmmModules(
                                hRapCh,
                                &sRBufParams[0],
                                &sSrcChParams,
                                &sMixerParams,
                                &sSpdifFmParams,
                                pOpParams,
                                BRAP_OutputChannelPair_eLR,
                                &(sTempHandles));
                if(ret != BERR_SUCCESS){goto stop_simul;}
            }
        }            
      
#else     
		if (pAudioParams->bCloneEnable == true)
		{
            /* Update the clone's status */
            hRapCh->eClone = BRAP_P_CloneState_eStarted;

		}
#endif	
		}
	} /* false==bWatchdogRecoveryOn */
	else { /* If watchdog recovery */
		ret = BRAP_DSPCHN_P_GetCurrentAudioParams(hRapCh->sModuleHandles.hDspCh, &sDspChParams);
		if(ret != BERR_SUCCESS)
			return BERR_TRACE(ret);
		if ( (sDspChParams.sExtAudioParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
#ifdef BCHP_7411_VER  /* only for 7411 */                 
           || (hRapCh->eClone==BRAP_P_CloneState_eStarted)
#endif           
		   ) {
          
			/* Start the internal FMM modules */
			ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							BRAP_OutputChannelPair_eLR,
							&(hRapCh->sSimulPtModuleHandles));
			if(ret != BERR_SUCCESS)
				return BERR_TRACE(ret);


#ifndef BCHP_7411_VER
            /* If the SimulPt port has been cloned, start the clones also */
            sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
            /* Form BRAP_P_ObjectHandles structure */
            sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[0];
            sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR + 1] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[1]; 
            sTempHandles.hSrcCh[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;
            sTempHandles.hMixer[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
            sTempHandles.uiMixerInputIndex[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].uiMixerInputIndex;  
            sTempHandles.hSpdifFm[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSpdifFm;  
            sTempHandles.hOp[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;   
    	
            BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
            /* Start the internal FMM modules */
            ret = BRAP_P_StartOpPathFmmModules(
                            hRapCh,
                            &sRBufParams[0],
                            &sSrcChParams,
                            &sMixerParams,
                            &sSpdifFmParams,
                            pOpParams,
                            BRAP_OutputChannelPair_eLR,
                            &(sTempHandles));
            if(ret != BERR_SUCCESS){goto stop_simul;}
#endif
            
		}
	}


	/* Start the DSP Channel */

    
	if (false==bWatchdogRecoveryOn) {
	/* Populate sDspChnParams structure */
	sDspChParams.sExtAudioParams = pAudioParams->sDspChParams;

	/* Initialize the ring buffer index array in sDspChnParams */
	for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)
	{
		sDspChParams.sDecOrPtParams.rBufIndex[i] = BRAP_RM_P_INVALID_INDEX;
        sDspChParams.sSimulPtParams.rBufIndex[i] = BRAP_RM_P_INVALID_INDEX;
	}

    /* Fill up the ring buffer index array in sDspChnParams */
	j = 0;
    for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++) 
	{
		if(hRapCh->sModuleHandles.hRBuf[i] == NULL)
		{
			/* For the current output mode, this rbuf is not 
			   used, so skip the opening of the modules for this channel */
			continue;
		}

		sDspChParams.sDecOrPtParams.rBufIndex[j] = hRapCh->sModuleHandles.hRBuf[i]->uiIndex;
		j++;
	}
    

	if(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    {
        /* Pass the RBUFS to be used for passthru context of simul mode.
           No extra RBUFS are needed for the clone output */
	    j = 0;
        for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++) 
    	{
	    	if(hRapCh->sSimulPtModuleHandles.hRBuf[i] == NULL)
		    {
			    /* For the current output mode, this output channel is not 
	    		   used, so skip the opening of the modules for this channel */
    			continue;
		    }

            sDspChParams.sSimulPtParams.rBufIndex[j] = hRapCh->sSimulPtModuleHandles.hRBuf[i]->uiIndex;
	    	j++;
        }
    }
    
	sDspChParams.eTimebase = pAudioParams->eTimebase;
      /* Initialize the Transport Channel ID.
	  This is used to program the RAVE_CTX_SEL field */


#if BRAP_P_USE_BRAP_TRANS ==1
	sDspChParams.uiTransChId = hRapCh->hRapTransCh->uiChannelId;
#else
	sDspChParams.uiTransChId = hRapCh->uiXptChannelNo;
#endif

	sDspChParams.bPlayback =  pAudioParams->bPlayback;

        /* FIXME: usage of uiNumOpPorts and j here is redundant. only one shud be used. 
        should count sOpPortParams.uiNumOpPorts instead of taking from this array */
	/* Number of output port required: Each output port can carry 2 audio channels */
#if BCHP_7411_VER > BCHP_VER_C0
        if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
		{
#endif             
    	    sDspChParams.sDecOrPtParams.sOpPortParams.uiNumOpPorts = 
    	        (opAudModeProp[hRapCh->eMaxOpMode].uiNoChannels + 1) / 2;
            sDspChParams.bMultiChanOnI2S = false;
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if pAudioParams->bMuxChannelPairsOnI2sEnable == false */
        else
        {
            /* Special case: MuxChannelPairsOnI2sEnable==true: 2 I2s ports associated with a 
               single decode channel */
            sDspChParams.sDecOrPtParams.sOpPortParams.uiNumOpPorts = 
                BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
            sDspChParams.bMultiChanOnI2S = true;
        }
#endif /* BCHP_7411_VER > BCHP_VER_C0 */ 

#ifdef BCHP_7411_VER  /* only for 7411 */  
	/* For decode mode, if we have cloned output, pass additional output ports also in 
	   sDspChParams.sDecOrPtParams.sOpPortParams structure */
	if (pAudioParams->bCloneEnable == true)
		sDspChParams.sDecOrPtParams.sOpPortParams.uiNumOpPorts++;
#endif

	/* Pre-initialize output ports and PLL fields to BRAP_RM_P_INVALID_INDEX */
	for (i = 0; i < sDspChParams.sDecOrPtParams.sOpPortParams.uiNumOpPorts; i++) {
		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[i].eOutputPortType 
			 = BRAP_OutputPort_eMax;
		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[i].ePll 
			 = BRAP_OP_Pll_eMax;
	}

	/* Fill up the output ports required for the current Output Audio Mode */
	j = 0;
	for (i = 0; i < uiMaxChPairs; i++)
	{
#if BCHP_7411_VER > BCHP_VER_C0
        if (false == pAudioParams->bMuxChannelPairsOnI2sEnable)
		{
#endif	
    		if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
    		   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
    		{
    			/* For the current output mode, this output channel pair is not 
    			   used, so skip the opening of the modules for this channel */
    			continue;
    		}
#if BCHP_7411_VER > BCHP_VER_C0
        }/* if bMuxChannelPairsOnI2sEnable == false */   
#endif	

        /* For the main output port associated withthis channel pair */
		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
			= hRapCh->sModuleHandles.hOp[i]->eOutputPort;
               BRAP_P_GetPllForOp (hRapCh->hRap, 
                            hRapCh->sModuleHandles.hOp[i]->eOutputPort, 
                            &(sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].ePll));        

              /* If port is Mai, pass the MuxSelect option to DSP instead of Mai */
		if (hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eMai)
		{
				/* Populate the SPDIF settings properly if MAI input is SPDIF */
				if ( eMaiMuxSelect == BRAP_OutputPort_eSpdif)
				{
					sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
						= BRAP_OutputPort_eSpdif;
				}
				if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
				{
					sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
						= BRAP_OutputPort_eFlex;
				}
		
		}
		j++;    /* increment count */

#ifndef BCHP_7411_VER
            
            /* If there are any cloned ports, also add them */
            for (k=0; k< BRAP_RM_P_MAX_OUTPUTS; k++)
            {
                if (hRapCh->sCloneOpPathHandles[i][k].hOp != NULL)
                {
        		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
	        		= hRapCh->sCloneOpPathHandles[i][k].hOp->eOutputPort;
                
                     BRAP_P_GetPllForOp (hRapCh->hRap, 
                                hRapCh->sCloneOpPathHandles[i][k].hOp->eOutputPort, 
                                &(sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].ePll)); 

                      /* If port is Mai, pass the MuxSelect option to DSP instead of Mai */
        		if (hRapCh->sCloneOpPathHandles[i][k].hOp->eOutputPort == BRAP_OutputPort_eMai)
        		{
    				/* Populate the SPDIF settings properly if MAI input is SPDIF */
    				if (eMaiMuxSelect == BRAP_OutputPort_eSpdif )
    				{
    					sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
    						= BRAP_OutputPort_eSpdif;
    				}
    				if (eMaiMuxSelect == BRAP_OutputPort_eFlex )
    				{
    					sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
    						= BRAP_OutputPort_eFlex;
    				}
        		}                     
                  
                     j ++;
                         
                }
            }
	}
       sDspChParams.sDecOrPtParams.sOpPortParams.uiNumOpPorts = j;    

#endif

        

#ifdef BCHP_7411_VER  /* only for 7411 */  
	}
	/* If output is cloned, initialize output ports and PLLs for
	   cloned output also. */
	if (pAudioParams->bCloneEnable == true) {
		/* TODO: Currently cloning to SPDIF is supportd. For cloning to other
		   output ports, following initialization should be done properly */
		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType
			= hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;
        /* clone in 7411 can only be spdif. so take spdif's pll */        
		sDspChParams.sDecOrPtParams.sOpPortParams.sOpPortConfig[j].ePll
			= hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;
	}
#endif
	/* For Simultaneous mode, add the output port for the Pass-thru context
       and for cloned output.  */
	if ( (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
    {
        sDspChParams.sSimulPtParams.sOpPortParams.uiNumOpPorts = 1;

	 if ( hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort == BRAP_OutputPort_eMai )
	 {
		sDspChParams.sSimulPtParams.sOpPortParams.sOpPortConfig[BRAP_OutputChannelPair_eLR].eOutputPortType
			= eMaiMuxSelect;
        sDspChParams.sSimulPtParams.sOpPortParams.sOpPortConfig[BRAP_OutputChannelPair_eLR].ePll
            = hRapCh->hRap->sOutputSettings[eMaiMuxSelect].uOutputPortSettings.sSpdifSettings.ePll;
	 }
	 else
	 {
 		sDspChParams.sSimulPtParams.sOpPortParams.sOpPortConfig[BRAP_OutputChannelPair_eLR].eOutputPortType
				= hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;

        /* at present, mai and spdif are the only 2 options for the SimulPt 
        port. Mai is dealt with in the if case above. so here, the port can 
        only be spdif */
        if 
        (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort 
            != BRAP_OutputPort_eSpdif)
        {
            BDBG_ERR(("Invalid simul pt port"));
	 }
        sDspChParams.sSimulPtParams.sOpPortParams.sOpPortConfig[BRAP_OutputChannelPair_eLR].ePll 
			= hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;       

	 }       
    }
    /* Add the SPDIF channel status buffer params to the sDspChParams */
    sDspChParams.sSpdifChStatusParams=pAudioParams->sSpdifChanStatusParams;

  	} /* false==bWatchdogRecoveryOn */
	/* Start the DSP Channel */
	ret = BRAP_DSPCHN_P_Start(hRapCh->sModuleHandles.hDspCh, &sDspChParams);
	if(ret != BERR_SUCCESS){
		if (true==bWatchdogRecoveryOn)
			return BERR_TRACE(ret);
		else
			goto stop_simul;
	}

	if (false==bWatchdogRecoveryOn) {
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) ||
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
			hRapCh->hRap->uiDecodeCount++;
	}

	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) || 
	(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		hRapCh->hRap->uiPassThruCount++;
	}
	}

    /*Since everything went fine, Set channel status to started */
    hRapCh->bStarted = true;
	goto end_start;

stop_simul:
		rc = BRAP_P_StopOpPathFmmModules(BRAP_OutputChannelPair_eLR, &(hRapCh->sSimulPtModuleHandles));
		BDBG_ASSERT(rc == BERR_SUCCESS);    


stop_clone:
#ifndef BCHP_7411_VER /* not for 7411 */
    sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;

   /* stop all Cloned ports associated with this channel */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {  
                if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
                {              
                     /* Form BRAP_P_ObjectHandles structure */
                     sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                     sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                     sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                     sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                     sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                     sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                     sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;   

        		rc = BRAP_P_StopOpPathFmmModules(i, &(sTempHandles));
        		BDBG_ASSERT(rc == BERR_SUCCESS);    
                  }
            }
     }
#endif

#ifndef BCHP_7411_VER  /* not  for 7411 */      
stop_master:
	for(i=0; i < uiMaxChPairs; i++)
	{
    		if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
    		   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
    		{
    			/* For the current output mode, this output channel pair is not 
    			   used, so skip the stopping of the modules for this channel */
    			continue;
    		}

		rc = BRAP_P_StopOpPathFmmModules(i, &(hRapCh->sModuleHandles));
		BDBG_ASSERT(rc == BERR_SUCCESS);
	}   
#endif
    
return_error:
		/* Returning on error. Restore back channel state variables. */
		hRapCh->bStarted = false;
	
end_start:

	
	BDBG_LEAVE(BRAP_DEC_Start);

	return (ret);
}
#else /* for non-7411 platforms */
/***************************************************************************
Summary:
	Starts a decode channel.

Description:
	This API is used to start the decode/pass-through/simul /SRC of the selected 
	channel (stream), as specified by input BRAP_DEC_AudioParams structure.
    
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_Stop

****************************************************************************/

BERR_Code BRAP_DEC_Start ( 
	BRAP_ChannelHandle 			hRapCh,			/* [in] The RAP Channel handle */
	const BRAP_DEC_AudioParams	*pAudioParams	/* [in] Audio parameters required 
														for starting this channel */
	)
{

/* Algorithm followed for starting various modules is:

1. Mute associate channel in DP (was Muted in BRAP_DEC_Stop()) 

2. Start the associated Output operation

3. Start the associated Microsequencer operation 

4. Start the associated Mixer operation

5. Unmute the channel (taken care of in BRAP_MIXER_P_Start() which starts 
    with SCALE_MUTE_ENA & VOLUME_MUTE_ENA both unMuted)

6. Start the associated Ring buffers' operation

7. Start the associated Source FIFOs' operation

8. Start the associated DSP Context operation (Program audio PLL, open the gate)
*/

	BERR_Code				ret = BERR_SUCCESS;
	BERR_Code				rc = BERR_SUCCESS;
	BRAP_DSPCHN_P_AudioParams	*psDspChParams=NULL;
	BRAP_RBUF_P_Params		sRBufParams[BRAP_RM_P_MAX_RBUFS_PER_SRCCH];
	BRAP_SRCCH_P_Params		sSrcChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	BRAP_DSPCHN_DecodeMode 	eDecodeMode;
	BRAP_P_ObjectHandles        sTempHandles;  
	bool					bCompress = false;
	bool					bOutputAlignment = false;	/* If outputs to be aligned */
	void					*pOpParams = NULL;

	unsigned int			i, j, k;
   	bool                    bValid = false;
    	unsigned int            uiMaxChPairs;
	bool					bIsPCMMixedWithCompressed;
	bool			bWatchdogRecoveryOn = false;
	BRAP_OutputPort eMaiMuxSelect=BRAP_OutputPort_eMax;  
	BRAP_OutputPort eOutputPort=BRAP_OutputPort_eMax;
	uint32_t physAddress=0x0;
	

	BRAP_RM_P_ResrcReq sResrcReq;
	BRAP_RM_P_ResrcGrant sResrcGrant;
    	unsigned int            uiIntCnt; 
#if (BCHP_CHIP == 7400)        
    unsigned int uiCount_Stored=0;
    bool bIndpDly=false;
#endif        
        
	/* Allocate memory for DSP Channel params structure */
	psDspChParams = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_AudioParams)); 

	if (NULL==psDspChParams)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	/* Initialize the Channel Handle */
	BKNI_Memset( psDspChParams, 0, sizeof(BRAP_DSPCHN_P_AudioParams));


	BDBG_ENTER(BRAP_DEC_Start);

   	uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;

	/* Check if this is a watchdog recovery. */
	bWatchdogRecoveryOn = BRAP_P_GetWatchdogRecoveryFlag(hRapCh->hRap);

    	BDBG_MSG(("BRAP_DEC_Start: hRapCh=0x%x, hRapCh->uiChannelNo=%d", 
        	hRapCh,hRapCh->uiChannelNo));

	if (false==bWatchdogRecoveryOn) {

	    BDBG_MSG(("BRAP_DEC_Start: Paramters:"));
	    BDBG_MSG(("\t Decoder Timebase=0x%x", pAudioParams->eTimebase));

	    BDBG_MSG(("\t sDspChParams.eDecodeMode=0x%x", pAudioParams->sDspChParams.eDecodeMode));
	    BDBG_MSG(("\t sDspChParams.eType=0x%x", pAudioParams->sDspChParams.eType));
	    BDBG_MSG(("\t sDspChParams.eStreamType=0x%x", pAudioParams->sDspChParams.eStreamType));
	    BDBG_MSG(("\t sDspChParams.eAacXptFormat=0x%x", pAudioParams->sDspChParams.eAacXptFormat));
	    BDBG_MSG(("\t sDspChParams.i32AVOffset=0x%x", pAudioParams->sDspChParams.i32AVOffset));
	    BDBG_MSG(("\t ePBrate=0x%x", pAudioParams->sDspChParams.ePBRate));                

	    BDBG_MSG(("\t sAudioOutputParams.uiOutputBitsPerSample=0x%x", pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
	    BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", pAudioParams->sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
	    BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", pAudioParams->sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
	    BDBG_MSG(("\t sAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", pAudioParams->sAudioOutputParams.sSpdifFmParams.bSpdifFormat));

	    BDBG_MSG(("\t sSimulPtAudioOutputParams.uiOutputBitsPerSample=0x%x", pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", pAudioParams->sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", pAudioParams->sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
	    BDBG_MSG(("\t sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", pAudioParams->sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat));

	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bProfessionalMode=0x%x", pAudioParams->sSpdifChanStatusParams.bProfessionalMode));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum=0x%x", pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSwCopyRight=0x%x", pAudioParams->sSpdifChanStatusParams.bSwCopyRight));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16CategoryCode=0x%x", pAudioParams->sSpdifChanStatusParams.ui16CategoryCode));
	    BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16ClockAccuracy=0x%x", pAudioParams->sSpdifChanStatusParams.ui16ClockAccuracy));

	    BDBG_MSG(("\t bPlayback=0x%x", pAudioParams->bPlayback));
	}

        hRapCh->hInterruptCount->uiCrcError=0;
        hRapCh->hInterruptCount->uiDecoderUnlock=0;
        hRapCh->hInterruptCount->uiFmmCRC=0;
        hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark=0;
        hRapCh->hInterruptCount->uiPtsError=0;
        hRapCh->hInterruptCount->uiFramesInError=0;
		hRapCh->hInterruptCount->uiTotalFrameDecoded=0;

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);
	if (false==bWatchdogRecoveryOn) 
     {
		BDBG_ASSERT(pAudioParams);

	BDBG_MSG (( "BRAP_DEC_Start():"
                "hRapCh=0x%x",
                hRapCh));

	/* If channel already started return an error */
	if (hRapCh->bStarted) {
		BDBG_ERR(("Channel already started"));
		BKNI_Free(psDspChParams);		
		return BERR_SUCCESS;
	}

    eMaiMuxSelect = 
        hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;

    /* Make sure atleast one output port is attached to this channel */
    for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
    	if(hRapCh->sModuleHandles.hOp[i] != NULL) 
    	{
            bValid = true;
            break;
    	}
    }    
    if (bValid == false)
    {
        BDBG_ERR(("No output ports are connected to this channel!!! First add an output port then start!!"));
        ret = BERR_TRACE(BERR_NOT_SUPPORTED);
	goto return_error;
    }

    /* Check DecodeMode */
    switch (pAudioParams->sDspChParams.eDecodeMode)
    {
        /* Modes supported:  */
        case BRAP_DSPCHN_DecodeMode_eDecode:
        case BRAP_DSPCHN_DecodeMode_ePassThru:
        case BRAP_DSPCHN_DecodeMode_eSimulMode:
             break;
        /* Not supported */
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Decode mode %d not supported", 
						  pAudioParams->sDspChParams.eDecodeMode));
		        	ret = BERR_TRACE(BERR_INVALID_PARAMETER);
				goto return_error;
    }

    	
	/* For debug purpose */
	BDBG_ASSERT(hRapCh->sModuleHandles.hDspCh);

		/* For debug purpose */
        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
		BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]);
    		BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hRBuf[0] && hRapCh->sSimulPtModuleHandles.hRBuf[1]);
        
        BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]);
	   	BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hMixer[BRAP_OutputChannelPair_eLR]);
	   	BDBG_ASSERT(hRapCh->sSimulPtModuleHandles.hSpdifFm[BRAP_OutputChannelPair_eLR]);    
        }
        
	if ( (hRapCh->hRap->uiDecodeCount + hRapCh->hRap->uiPassThruCount) >=
		(BRAP_RM_P_MAX_DSPS * BRAP_RM_P_MAX_CXT_PER_DSP)) {
		BDBG_ERR(("No more bandwidth to perform decode mode requested. Please see"
			"comments in brap.h for various combinations of decode modes supported in"
			"decode channels."));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;
	}	
		
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) ||
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		if (hRapCh->hRap->uiDecodeCount >= BRAP_RM_P_MAX_DECODE_SUPPORTED) {
			BDBG_ERR(("Number of decode operations exceeding maximum number of decode"
				"operations supported %d. Please see comments in brap.h module overview for various combinations"
				"of decode modes supported in decode channels.", BRAP_RM_P_MAX_DECODE_SUPPORTED));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;
		}
	}
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) || 
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		if (hRapCh->hRap->uiPassThruCount >= BRAP_RM_P_MAX_PASSTHRU_SUPPORTED) {
			BDBG_ERR(("Number of passthru operations exceeding maximum number of passthru"
				"operations supported %d. Please see comments in brap.h module overview for various combinations"
				"of decode modes supported in decode channels.", BRAP_RM_P_MAX_DECODE_SUPPORTED));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;
		}
	}

#ifndef RAP_AC3_SUPPORT
        if((pAudioParams->sDspChParams.eType==BRAP_DSPCHN_AudioType_eAc3) && 
            (pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)&&
            (hRapCh->hRap->sSettings.bSupportAlgos[pAudioParams->sDspChParams.eType]==true))
        {
            BDBG_ERR(("Cann't decode Algo type %d. Please define RAP_AC3_SUPPORT",
                    pAudioParams->sDspChParams.eType));
            ret = BERR_SUCCESS;
            goto return_error;	
        }
#endif

#ifndef RAP_DRA_SUPPORT
        if(pAudioParams->sDspChParams.eType==BRAP_DSPCHN_AudioType_eDra)
        {
            BDBG_ERR(("Cann't decode Algo type %d. Please define RAP_DRA_SUPPORT",
                    pAudioParams->sDspChParams.eType));
            ret = BERR_SUCCESS;
            goto return_error;	
        }
#endif


#ifndef RAP_PCMWAV_SUPPORT
        if(pAudioParams->sDspChParams.eType==BRAP_DSPCHN_AudioType_ePcmWav)
        {
            BDBG_ERR(("Cann't decode Algo type %d. Please define RAP_PCMWAV_SUPPORT",
                    pAudioParams->sDspChParams.eType));
            ret = BERR_SUCCESS;
            goto return_error;	
        }
#else
        if((pAudioParams->sDspChParams.eType==BRAP_DSPCHN_AudioType_ePcmWav) &&
           (pAudioParams->sDspChParams.eDecodeMode!=BRAP_DSPCHN_DecodeMode_eDecode) &&
           (hRapCh->hRap->sSettings.bSupportAlgos[pAudioParams->sDspChParams.eType]==true))
        {
            BDBG_ERR(("PCMWAV algorithm must be started in decode mode only"));
            ret = BERR_SUCCESS;
            goto return_error;	
        }
#endif
    /* Check algo type: 
       NOTE: This  list should be constantly updated to refelct current status for 
       each platform */
    switch (pAudioParams->sDspChParams.eType)
    {
        /* Algo types supported:  */
        case BRAP_DSPCHN_AudioType_eAc3:
#ifdef RAP_SRSTRUVOL_CERTIFICATION            
        case BRAP_DSPCHN_AudioType_ePCM:
#endif 			
        case BRAP_DSPCHN_AudioType_eMpeg:
        case BRAP_DSPCHN_AudioType_eAac:
#if ((BRAP_DTS_SUPPORTED == 1) || (BRAP_DTS_PASSTHRU_SUPPORTED==1))
        case BRAP_DSPCHN_AudioType_eDts:       
#endif            
	 case BRAP_DSPCHN_AudioType_eAacSbr:
	 case BRAP_DSPCHN_AudioType_eAc3Plus:   
        case BRAP_DSPCHN_AudioType_eWmaStd:          	 	
        case BRAP_DSPCHN_AudioType_eWmaPro:
        case BRAP_DSPCHN_AudioType_ePcmWav:             
	 case BRAP_DSPCHN_AudioType_eLpcmDvd:
	 case BRAP_DSPCHN_AudioType_eDra:
             break;
        /* Not supported */
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Algo type %d not supported", 
						  pAudioParams->sDspChParams.eType));
		         	ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;	
    }
	/* Check if algo type was selected at BRAP_Open time. Otherwise we may
	 * not have sufficient memory for this algo */
	 if (hRapCh->hRap->sSettings.bSupportAlgos[pAudioParams->sDspChParams.eType]==false) {
	 	BDBG_ERR(("Audio type %d was not selected at BRAP_Open. This audio type can't be run.",
			pAudioParams->sDspChParams.eType));
		ret = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto return_error;	
	 }

	/* Check Transport Stream type: 
       NOTE: This  list should be constantly updated to refelct current status for 
       each platform */
    switch (pAudioParams->sDspChParams.eStreamType)
    {
        /* Stream types supported:  */
        case BAVC_StreamType_eTsMpeg:
        case BAVC_StreamType_eDssEs:
        case BAVC_StreamType_eDssPes:
        case BAVC_StreamType_ePS:
        case BAVC_StreamType_ePes:
             break;
        /* Not supported */
        case BAVC_StreamType_eEs:
        case BAVC_StreamType_eBes:
        case BAVC_StreamType_eCms:
   		default:
				BDBG_ERR(("BRAP_DEC_Start: Stream type %d not supported", 
						  pAudioParams->sDspChParams.eStreamType));
		        	ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;		
    }

 
	if((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode) 
        && (hRapCh->bSimulModeConfig == false))
	{
		BDBG_ERR(("No Output port has been reserved for the compressed data yet. So cant start in Simul Mode."));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;	
	}

	/* Simulmode and PassThru are not supported for following algorithms.
	 * AAC-SBR
	 * LPCM-BD, LPCM-DVD
	 * Return error if decode mode is simulmode or passthru for these algorithms.
	 */
    if ((pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmBd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmHdDvd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eLpcmDvd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eWmaStd)
	|| (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eMlp))
    { 
        if ((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
            || (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
        {
        
                BDBG_ERR(("BRAP_DEC_Start: Compressed data cant be sent on the SPDIF output port for algorithm %d",
					pAudioParams->sDspChParams.eType));
              ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
		goto return_error;			
        }        
    }

#if (BCHP_CHIP == 7400)
	if((pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus)
     &&(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
    {
        BDBG_WRN(("BRAP_DEC_Start: NOTE: For DDP, in Simul Mode, we transcode"
        " and send out compressed AC3 data on the SPDIF"));            
    }
#else
    /* For DDP(ie AC3plus)/ WMA PRO, if it is pure passthru - disallow. 
    in simul mode, the FW will automatically transcode to AC3 and send out compressed AC3. */
	if ((pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus)
		|| (pAudioParams->sDspChParams.eType ==BRAP_DSPCHN_AudioType_eWmaPro))
    { 
        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)          
        {
        
            BDBG_ERR(("BRAP_DEC_Start: Compressed data cant be sent on the SPDIF output port for algorithm %d",
                pAudioParams->sDspChParams.eType));
            ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
            goto return_error;          
        }        
        else if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
           BDBG_WRN(("BRAP_DEC_Start: NOTE: For algorithm %d, in Simul Mode, we transcode"
            " and send out compressed AC3 data on the SPDIF/HDMI",pAudioParams->sDspChParams.eType));        
        }              
    }
#endif


#if (BRAP_DTS_PASSTHRU_SUPPORTED == 1)		

	if( (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eDts)
		&&(pAudioParams->sDspChParams.eDecodeMode != BRAP_DSPCHN_DecodeMode_ePassThru))
    	{         
	            BDBG_ERR(("BRAP_DEC_Start: Only Compressed mode supported for algorithm %d",
	                pAudioParams->sDspChParams.eType));
	            ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
	            goto return_error;          
	}
#endif





	/* Check for supported bit resolutions.
	   For I2S and Flex output, it can be between 16 bits/sample to 32 bits/sample,
	   For other outputs, it can be between 16 bits/sample to 24 bits/sample */
	if((pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_I2S_MAX_BITS_PER_SAMPLE) ||
	   (pAudioParams->sAudioOutputParams.uiOutputBitsPerSample < BRAP_P_MIN_BITS_PER_SAMPLE))
	{
		BDBG_ERR(("Output bit resolution %d should be between 16 to 32 bits/sample", 
                   pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
		ret = BERR_TRACE(BERR_NOT_SUPPORTED);
		goto return_error;	
	}
	else if(pAudioParams->sAudioOutputParams.uiOutputBitsPerSample > BRAP_P_MIXER_MAX_BITS_PER_SAMPLE)
	{
		for(i=0; i < uiMaxChPairs; i++)
		{
			if ( hRapCh->sModuleHandles.hOp[i] == NULL )
    			{
    				/* This output channel pair is not used, so skip the
    				opening of the modules for this channel */
    				continue;
    			}

			if((hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s0) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s1) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eI2s2) &&
			   (hRapCh->sModuleHandles.hOp[i]->eOutputPort != BRAP_OutputPort_eFlex))
			{
				/* Only I2S and Flex can have more than 24 bits/sample output.
				   So for other outputs return error */
				BDBG_ERR(("Output bit resolution %d should be between 16 to 24 bits/sample", 
                           pAudioParams->sAudioOutputParams.uiOutputBitsPerSample));
				ret = BERR_TRACE(BERR_NOT_SUPPORTED);
				goto return_error;	
			}
		}
	}

	/* If configured, check for supported bit resolutions for the pass thru context of  
	   simultaneous mode / clone port. It should be between 16 bits/sample to 24 bits/sample, 
	   as the output can be SPDIF or MAI. */
    if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    {
        if (pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample 
                != BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA) 
        {
            BDBG_WRN(("Compressed data can have only 16 bits pers sample. Overruling user supplied bits per sample (%d).", 
                    pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample));
            /*pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample 
                    = BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA;            */

        }
    }

#ifndef RAP_I2S_COMPRESS_SUPPORT
/* Make sure that the application doesnt try to send compressed data on a port that doesnt support it */
		if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru ) 
		{
		    if ((hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eSpdif) 
                        && (hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eMai))
		    {
			BDBG_ERR(("PassThru mode %d "
					  "does not allow to use the output port %d since it cannot carry compressed data", 
	              pAudioParams->sDspChParams.eDecodeMode,
	              hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort));
			ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
			goto return_error;	
                }
		}
		if ( pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode )
		{
			if ( (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eSpdif)
                       && (hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort != BRAP_OutputPort_eMai))
			{
				BDBG_ERR(("Simul %d "
						  "does not allow to use the output port %d for compressed data", 
		              pAudioParams->sDspChParams.eDecodeMode,
		              hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort));
				ret = BERR_TRACE(BRAP_ERR_COMP_SPDIF_NOT_ALLOWED);
				goto return_error;	
			}
		}
#endif
		if (pAudioParams->sDspChParams.bPtsSpecificDecode==true) 
		{
			BDBG_ERR(("PTS specific decode feature is supported only for 7411 D0"));
			ret = BERR_TRACE(BERR_NOT_SUPPORTED);
			goto return_error;	
		}
	} /* false==bWatchdogRecoveryOn */
	
	/* Check if alignment between outputs is required */
	if(hRapCh->sModuleHandles.hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain] > BRAP_OutputMode_e2_0)
	{
		/* Multiple output ports are needed. So alignment required. */
		BDBG_MSG(("Output alignment required"));
		bOutputAlignment = true;
	}

	if (false==bWatchdogRecoveryOn) 
	{
		/* If BRAP_DEC_Start is getting called from within PI (internal call), don't reset
		 * trick mode states. Reset these states for external call.
		 */
		if (!BRAP_P_GetInternalCallFlag(hRapCh)) 
		{
			/* Clear some of the channel states */
			hRapCh->sTrickModeState.bAudioPaused = false;
			hRapCh->sTrickModeState.uiFrameAdvResidualTime = 0;
		}		

		/* XPT Channel number for the current channel. This is used by the 
		   DSP Firmware to determine the CDB and ITB used for the current 
		   DSP Context  */
		hRapCh->uiXptChannelNo = pAudioParams->sXptContextMap.ContextIdx;

		eDecodeMode = pAudioParams->sDspChParams.eDecodeMode;

		/* For Passthru mode, the first context ie context 0 has to carry compressed data.
		 For Decode and Simul mode, context 0 carries PCM data*/
		if(eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru)
			bCompress = true;
		else
			bCompress = false;

		/* Allocate the DSP context for the channel */

		if( pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru )
		{
			sResrcReq.bPassthrough=true;
		}
		else
		{
			sResrcReq.bPassthrough=false;
		}
		
		/* Form the resource requirement */
		sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
		sResrcReq.uiNumOpPorts = 0;
		sResrcReq.bLastChMono = false;
		sResrcReq.bSimulModePt = false; 
		sResrcReq.bAllocateDsp = true; 
		sResrcReq.bAllocateDspCtx  = true; 
		sResrcReq.bAllocateRBuf= false;
		sResrcReq.bAllocateSrcCh= false;
		sResrcReq.bAllocateDstCh= false;                                            

		for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
		{
			sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
		}
		
		ret = BRAP_RM_P_AllocateResources (hRapCh->hRap->hRm, 
									    &sResrcReq,
									    &sResrcGrant);
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("Resource allocation failed"));
			goto end_start;
		} 
		
		/* Initialize the resouce allocated to channel handle */
		hRapCh->sRsrcGrnt.uiDspId = sResrcGrant.uiDspId;
		hRapCh->sRsrcGrnt.uiDspContextId = sResrcGrant.uiDspContextId;

		/* Need to initialize the DSP channel handle properly */
		ret = BRAP_DSPCHN_P_InitializeHandle ( hRapCh->sModuleHandles.hDspCh, hRapCh->sRsrcGrnt.uiDspContextId );
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_DSPCHN_P_InitializeHandle failed"));
			goto end_start;
		} 

		if ( pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode )
		{
			/* Form the resource requirement */
			sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
			sResrcReq.uiNumOpPorts = 0;
			sResrcReq.bLastChMono = false;
			sResrcReq.bSimulModePt = true; 
			sResrcReq.bAllocateDsp = true; 
			sResrcReq.bAllocateDspCtx  = true; 
			sResrcReq.bAllocateRBuf= false;
			sResrcReq.bAllocateSrcCh= false;
			sResrcReq.bAllocateDstCh= false;                                            

			for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
			{
				sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
			}
		
			ret = BRAP_RM_P_AllocateResources (hRapCh->hRap->hRm, 
									    &sResrcReq,
									    &sResrcGrant);
			if(ret != BERR_SUCCESS)
			{
				BDBG_ERR(("Resource allocation failed"));
				goto end_start;
			}

			/* Store the allocated context to the channel handle */
			hRapCh->sSimulPtRsrcGrnt.uiDspId = sResrcGrant.uiDspId;
			hRapCh->sSimulPtRsrcGrnt.uiDspContextId = sResrcGrant.uiDspContextId;
		}

		/* Intialise Raptor interrupt handling */
		ret = BRAP_P_InterruptInstall (hRapCh);
		if(ret != BERR_SUCCESS)
		{
			ret = BERR_TRACE(ret);
			BDBG_ERR(("Interrupt installation failed for RAP DEC Channel handle 0x%x", hRapCh));
		}

		for ( uiIntCnt=0; uiIntCnt < BRAP_Interrupt_eMaxInterrupts; uiIntCnt++ )
		{
			if ( hRapCh->sAppIntCbInfo[uiIntCnt].pfAppCb != NULL )
			{
				ret= BRAP_P_UnmaskInterrupt (hRapCh, uiIntCnt);
				if(ret != BERR_SUCCESS)
				{
					BDBG_ERR(("BRAP_DEC_Start: Error in unmasking interuupt Id [%d]",uiIntCnt));
					goto end_start;
				}
			}
		}

		/* Form internal FMM module params */
		
		/* SPDIF formatter Parameters */

		/* Get the default parameters first */
		ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
		if(ret != BERR_SUCCESS){goto end_start;} 
		
		/* Fill up the required parameters */
		sSpdifFmParams.sExtParams = pAudioParams->sAudioOutputParams.sSpdifFmParams;
		sSpdifFmParams.bCompressed = bCompress;

#if (BCHP_CHIP == 7400)
        if((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_ePassThru) &&
            (pAudioParams->sDspChParams.eType == BRAP_DSPCHN_AudioType_eAc3Plus))
        {
            BDBG_MSG(("HBR mode Enabled"));
            sSpdifFmParams.bHbrEnable = true;
        }
#endif
        
	    	sSpdifFmParams.bSeparateLRChanNum = 
	                pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum;

	    	ret = BRAP_P_GetBurstRepetitionPeriodForAlgo(
	                        pAudioParams->sDspChParams.eStreamType,
	                        pAudioParams->sDspChParams.eType,
	                        &sSpdifFmParams.eBurstRepPeriod);
	    	if(ret != BERR_SUCCESS){goto end_start;} 
	    
		/* Mixer parameters */
		
		/* Get the default parameters first */
		ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
		if(ret != BERR_SUCCESS){goto end_start;} 
		
		/* Fill up the required parameters */
		sMixerParams.sExtParams = pAudioParams->sAudioOutputParams.sMixerParams;
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
			sMixerParams.uiStreamRes 
				= pAudioParams->sAudioOutputParams.uiOutputBitsPerSample;


		/* RBuf parameters, use same parameters for all associated ring buffers */
		    for(k=0; k< BRAP_RM_P_MAX_RBUFS_PER_SRCCH; k++)
		    {
			    ret = BRAP_RBUF_P_GetDefaultParams (&sRBufParams[k]);
			    if(ret != BERR_SUCCESS){goto end_start;}    
		    }

		/* Source channel parameters */

		/* Get the default parameters first */
		ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
		if(ret != BERR_SUCCESS){goto end_start;} 

	    /* Normal playback path: data is not written to srcFIFO directly */
	    sSrcChParams.bCapDirect2SrcFIFO = false;
		
		/* Fill up the required parameters */
		sSrcChParams.bCompress = bCompress;

		/* Start playing when write pointer crosses SOURCE_RINGBUF_x_START_WRPOINT */
		sSrcChParams.bStartSelection = true; 

		if(bCompress)
		{
			/* Set bits per sample = 16, as DSP generates 16 bits per sample for compressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;
		}
		else
		{
			/* Set bits per sample = 32, as DSP generates 32 bits per sample for PCM data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;
		}
			
		sSrcChParams.bInputSrValid = true;  /* Input audio data coming from DSP */

	    	sSrcChParams.bSharedRbuf = false;    

		/* For a decode channel, no need to program eInputSR & eOutputSR, 
		   as this is going to be programmed by DSP */

	} /* false==bWatchdogRecoveryOn */
	else
	{
		/* Intialise Raptor interrupt handling */
		ret = BRAP_P_InterruptInstall (hRapCh);
		if(ret != BERR_SUCCESS)
		{
			ret = BERR_TRACE(ret);
			BDBG_ERR(("Interrupt installation failed for RAP DEC Channel handle 0x%x", hRapCh));
		}		

		/* Unmask all the interrupts for which the callbacks are installed by the application */
		for ( uiIntCnt=0; uiIntCnt < BRAP_Interrupt_eMaxInterrupts; uiIntCnt++ )
		{
			if ( hRapCh->sAppIntCbInfo[uiIntCnt].pfAppCb != NULL )
			{
				ret= BRAP_P_UnmaskInterrupt (hRapCh, uiIntCnt);
				if(ret != BERR_SUCCESS)
				{
					BDBG_ERR(("BRAP_DEC_Start: Error in unmasking interuupt Id [%d]",uiIntCnt));
					goto end_start;
				}
			}
		}
	}

	BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)hRapCh->hInterruptCount, &physAddress);
	BRAP_Write32(hRapCh->hRegister,BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,
			physAddress);

#if (BCHP_CHIP == 7400)

	if ((false==bWatchdogRecoveryOn)&& ((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
     ||(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)))
	{
        /* Check for all DownMixed LR */
        if(hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eDownMixedLR]!=NULL)
        {
        	/* Form the resource requirement */
        	sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
        	sResrcReq.uiNumOpPorts = 0;
        	sResrcReq.bLastChMono = false;
        	sResrcReq.bSimulModePt = false; 
        	sResrcReq.bAllocateDsp = false; 
        	sResrcReq.bAllocateDspCtx  = false; 
        	sResrcReq.bAllocateRBuf= false;
        	sResrcReq.bAllocateSrcCh= false;
        	sResrcReq.bAllocatePpm= true;        
        	sResrcReq.bAllocateDstCh= false;    

        	for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        	{
        		sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
        	}
       
        sResrcReq.sOpPortReq[BRAP_OutputChannelPair_eLR].eOpPortType
            = hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;
        
        	ret = BRAP_RM_P_AllocateResources (hRapCh->hRap->hRm, 
        							    &sResrcReq,
        							    &sResrcGrant);
        	if(ret != BERR_SUCCESS)
        	{
        		BDBG_ERR(("Resource allocation failed"));
        		goto end_start;
        	}  
            for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
            {
                if(sResrcGrant.sOpResrcId[i].uiPpmId != BRAP_RM_P_INVALID_INDEX)
                {
                	/* Initialize the resouce allocated to channel handle */
            		hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId = sResrcGrant.sOpResrcId[i].uiPpmId;
                    uiCount_Stored++;
                }            

            }			
        }
        /* check if LsRs,CLFE has any o/p added if yes then don't allocate ppm ID*/
        if((hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLRSurround]!=NULL)||
            (hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eCentreLF]!=NULL))
        {   
            BDBG_MSG(("Adaptive Rate Control is not Supported for PCM 5.1"));
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)			
			hRapCh->hRap->sSettings.bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eDolbyVolume]=false;			
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
			hRapCh->hRap->sSettings.bSupportPpAlgos[BRAP_DSPCHN_PP_Algo_eSRS_TruVol]=false;
#endif
        }
        else
        {
            /* For all LR allocate PPm */
            if(hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]!=NULL)
            {      
            	/* Form the resource requirement */
            	sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
            	sResrcReq.uiNumOpPorts = 0;
            	sResrcReq.bLastChMono = false;
            	sResrcReq.bSimulModePt = false; 
            	sResrcReq.bAllocateDsp = false; 
            	sResrcReq.bAllocateDspCtx  = false; 
            	sResrcReq.bAllocateRBuf= false;
            	sResrcReq.bAllocateSrcCh= false;
            	sResrcReq.bAllocatePpm= true;        
            	sResrcReq.bAllocateDstCh= false;    

            	for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
            	{
            		sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
            	}
                
              sResrcReq.sOpPortReq[BRAP_OutputChannelPair_eLR].eOpPortType = 
                hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;
         
            	ret = BRAP_RM_P_AllocateResources (hRapCh->hRap->hRm, 
            							    &sResrcReq,
            							    &sResrcGrant);
            	if(ret != BERR_SUCCESS)
            	{
            		BDBG_ERR(("Resource allocation failed"));
            		goto end_start;
            	}                                         
            }

            for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
            {
                if(sResrcGrant.sOpResrcId[i].uiPpmId != BRAP_RM_P_INVALID_INDEX)
                {
                	/* Initialize the resouce allocated to channel handle */
            		hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId = sResrcGrant.sOpResrcId[i].uiPpmId;
                    uiCount_Stored++;
                }            

            }

            
            /*for all cloned*/
      
            for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
            {
                for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
                {
                if (hRapCh->hRap->sOutputSettings[j].bIndpDlyRqd == true)
                {                  
                    if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL) 
                        {
                            /* Allocate PPm Id*/
                            /* Form the resource requirement */
                            sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
                            sResrcReq.uiNumOpPorts = 0;
                            sResrcReq.bLastChMono = false;
                            sResrcReq.bSimulModePt = false; 
                            sResrcReq.bAllocateDsp = false; 
                            sResrcReq.bAllocateDspCtx  = false; 
                            sResrcReq.bAllocateRBuf= false;
                            sResrcReq.bAllocateSrcCh= false;
                            sResrcReq.bAllocatePpm= true;        
                            sResrcReq.bAllocateDstCh= false;
                            bIndpDly = true;

                            for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
                            {
                            	sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
                            }
                            
                            sResrcReq.sOpPortReq[BRAP_OutputChannelPair_eLR].eOpPortType 
                                = hRapCh->sCloneOpPathResc[BRAP_RM_P_MAX_OP_CHANNEL_PAIRS][j].eOutputPortType;
                             /*   hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;*/
                                    
                            
                            ret = BRAP_RM_P_AllocateResources (hRapCh->hRap->hRm, 
                            						    &sResrcReq,
                            						    &sResrcGrant);
                            if(ret != BERR_SUCCESS)
                            {
                            	BDBG_ERR(("Resource allocation failed"));
                            	goto end_start;
                            } 
                            
                            if(bIndpDly == true)
                            {        
                                for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
                                {
                                    if(sResrcGrant.sOpResrcId[i].uiPpmId != BRAP_RM_P_INVALID_INDEX)
                                    {
                                        /* Initialize the resouce allocated to channel handle */
                                        hRapCh->sRsrcGrnt.sOpResrcId[uiCount_Stored].uiPpmId = sResrcGrant.sOpResrcId[i].uiPpmId;
                                    }            

                                }   
                            }   
                            uiCount_Stored++;
                        }
                    }
                 }                    
            } 
        }        
    }

#endif

                

#if (BCHP_CHIP == 7400)                
	if (true == bOutputAlignment)
	{
		/* Allign I2s output ports */        
		ret = BRAP_OP_P_AlignPorts( hRapCh->hRap, 
	                    false, /* bSpdif */ 
	                    true, /* bI2s0 */   
	                    true, /* bI2s1 */   
	                    true  /* bI2s2 */);
		if(ret != BERR_SUCCESS) {goto end_start;}
	}/* if bOutputAlignment */
#endif

	/* Start Fmm modules */
	for(i=0; i < uiMaxChPairs; i++)
	{
	if (false==bWatchdogRecoveryOn) {
		if ( hRapCh->sModuleHandles.hOp[i] == NULL )
		{
    			/* This output channel pair is not used, so skip
    			the opening of the modules for this channel */
			continue;
		}
	/*Checking if the PCM data is being tried to mix with compressed data*/
	ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
												hRapCh->sModuleHandles.hOp[i]->eOutputPort,
												bCompress,
												&bIsPCMMixedWithCompressed);
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error"));
		goto return_error;
	}

	if(bIsPCMMixedWithCompressed)
	{
	    ret = BERR_NOT_SUPPORTED;	    
		BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data"));
		goto return_error;
	}
        sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiSrcChId;

        ret = BRAP_P_FormOpParams (hRapCh, 
                hRapCh->sModuleHandles.hOp[i]->eOutputPort, 
                hRapCh->hRap->sOutputSettings[hRapCh->sModuleHandles.hOp[i]->eOutputPort].eOutputTimebase,
                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                &pOpParams);

        if (ret!=BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto end_start;
        }

	/* We allways allocate two ringbuffers irrespective of the output mode */
	sSrcChParams.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;

	/* Ring buffers to be mapped to this source channel */
	sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiRbufId[0];
	sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiRbufId[1];

	sSrcChParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_LR_2_LR;
        
	if(!bCompress)
	{        

        
		switch (hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode)
		{
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				sSrcChParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_L_2_LR;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				sSrcChParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_R_2_LR;
				break;
			default:
				break;
		}
	}
        

		/* For debug purpose */
		BDBG_ASSERT(hRapCh->sModuleHandles.hSrcCh[i]);
        	BDBG_ASSERT(hRapCh->sModuleHandles.hOp[i]);
		BDBG_ASSERT(hRapCh->sModuleHandles.hMixer[i]);
        	BDBG_ASSERT(hRapCh->sModuleHandles.hRBuf[i*2] && hRapCh->sModuleHandles.hRBuf[(2*i)+1]);

		if(((hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eSpdif) 
                    || (hRapCh->sModuleHandles.hOp[i]->eOutputPort == BRAP_OutputPort_eMai))
			&& (hRapCh->sModuleHandles.hSpdifFm[i] == NULL))
		{
                     ret = BERR_TRACE (BRAP_ERR_BAD_DEVICE_STATE);
                    goto end_start;
		}

		} /* false==bWatchdogRecoveryOn */
		/* Start the internal FMM modules */
		ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							(BRAP_OutputChannelPair)i,
							&(hRapCh->sModuleHandles));
		if(ret != BERR_SUCCESS){goto end_start;}
	}

       /* Start all Cloned ports (for main context) associated with this channel */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {    
                if ((hRapCh->sCloneOpPathHandles[i][j].hOp != NULL) 
                    && (hRapCh->sCloneOpPathHandles[i][j].bSimul == false))
                {      
                    BDBG_MSG(("BRAP_DEC_Start: output port %d was cloned for channel pair %d", j, i));
                    if (false==bWatchdogRecoveryOn) 
                    {                      
                        ret = BRAP_P_FormOpParams (hRapCh, (BRAP_OutputPort)j, 
                                hRapCh->hRap->sOutputSettings[j].eOutputTimebase,
                                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                                &pOpParams);

                        if (ret!=BERR_SUCCESS)
                        {
                            ret = BERR_TRACE(ret);
                            goto stop_master;
                        }
                
                        /* ReUse the same SpdifFmParams as filled in for the main port above */
                        /* ReUse the same MixerParams as filled in for the main port above, except SrcChId */
                        if (hRapCh->sCloneOpPathHandles[i][j].hSrcCh != NULL)
                        {
                            /* cloned port uses another SrcCh */
                            /* take the same Params as the orig src ch */
                            sSrcChParams = hRapCh->sModuleHandles.hSrcCh[i]->sParams;

                            /* We need to store the RBUF ID's, as in 7400 the Cloned 
                               channel uses separate RBUF's allocated to it.*/
                                sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[i][j].uiRbufId[0];
                                sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[i][j].uiRbufId[1];
                                BDBG_MSG(("sSrcChParams.uiLtOrSnglRBufId=%d,sSrcChParams.uiRtRBufId=%d,output=%d",
                                        sSrcChParams.uiLtOrSnglRBufId,sSrcChParams.uiRtRBufId,j));

                           /* this Src Ch shares the rbufs with the Orig SrCh */
                           sSrcChParams.bSharedRbuf = true;
                           sSrcChParams.uiMasterSrcCh = hRapCh->sModuleHandles.hSrcCh[i]->uiIndex;
               
                           sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[i][j].uiSrcChId ;     
                           BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", j, sMixerParams.uiSrcChId, sSrcChParams.uiMasterSrcCh));                       
                        }
                        else
                        {
                            sMixerParams.uiSrcChId = hRapCh->sModuleHandles.hSrcCh[i]->uiIndex; 

                            /* normal clones will have extra SrcCh only if they're in diff DP. 
                            With indep delay, clones in same DP will have new SrcCh */
                            
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)    
                            /* if the master port has an indep delay, look for the clone 
                            port without indep delay which is acting as master and use its 
                            srcch */
                            if (hRapCh->hRap->sOutputSettings[hRapCh->sModuleHandles.hOp[i]->eOutputPort].bIndpDlyRqd == true)
                            {
                                for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
                                {    
                                    if ((hRapCh->sCloneOpPathHandles[i][k].hSrcCh != NULL) 
                                        && (hRapCh->sCloneOpPathHandles[i][k].bSimul == false)
                                        && (hRapCh->hRap->sOutputSettings[k].bIndpDlyRqd == false))     
                                    {
                                        sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[i][k].uiSrcChId ;
                                    }
                                }
                            }
#else   
                            for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
                            {    
                                if (hRapCh->sCloneOpPathHandles[i][k].hSrcCh != NULL)
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
                                if (hRapCh->sCloneOpPathResc[i][j].uiDataPathId
                                        == hRapCh->sCloneOpPathResc[i][k].uiDataPathId)

                                {
                                    sMixerParams.uiSrcChId = hRapCh->sCloneOpPathHandles[i][k].hSrcCh->uiIndex;
                                }
                            }  
#endif                             
                            BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
                        }
                    } /* !watchdgo */
                    else
                    {
                        sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
                    }
                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                    sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                    sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                    sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;   
                       

        	        /*Checking if the PCM data is being tried to mix with compressed data*/
        	        ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
        			    									j,
        					    							sTempHandles.hMixer[i]->bCompress,
        				    								&bIsPCMMixedWithCompressed);
        	
        	        if(ret != BERR_SUCCESS)
        	        {
        		        BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error for Clonned output"));
        		        goto stop_master;
        	        }

        	        if(bIsPCMMixedWithCompressed)
        	        {
        	            ret = BERR_NOT_SUPPORTED;
        		        BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for Clonned output"));
        		        goto stop_master;
        	        }
        	
                    BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
                    /* Start the internal FMM modules */
                    ret = BRAP_P_StartOpPathFmmModules(
                                    hRapCh,
                                    &sRBufParams[0],
                                    &sSrcChParams,
                                    &sMixerParams,
                                    &sSpdifFmParams,
                                    pOpParams,
                                    (BRAP_OutputChannelPair)i,
                                    &(sTempHandles));
                    if(ret != BERR_SUCCESS){goto stop_master;}

                }            
            }
        }

	if (false==bWatchdogRecoveryOn) { /* TODO: Handle the watchdog case */
	/* Start the modules of 2nd context of simultaneous mode or the clone if configured */
	if ( (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode) )    
	{
		/* Should Mute Mixer ouput before starting Output block, MS and Mixer.
		But since Mixer hasnt been started yet, cant do so here. Instead, we
		make sure that when Mixer is started, it is Muted by default */
		
		/* Start the associated output block */

        ret = BRAP_P_FormOpParams (hRapCh, 
                hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort, 
                hRapCh->hRap->sOutputSettings[hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort].eOutputTimebase,
                pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                &pOpParams);

        if (ret!=BERR_SUCCESS)
        {
            ret = BERR_TRACE(ret);
            goto stop_clone;
        }

		/* SPDIF Formater parameters */
		
		/* Get the default parameters first */
		ret = BRAP_SPDIFFM_P_GetDefaultParams (&sSpdifFmParams);
		if(ret != BERR_SUCCESS){goto stop_clone;}
        
		/* Fill up the required parameters */
		sSpdifFmParams.sExtParams 
			= pAudioParams->sSimulPtAudioOutputParams.sSpdifFmParams;
        ret = BRAP_P_GetBurstRepetitionPeriodForAlgo(
                        pAudioParams->sDspChParams.eStreamType,
                        pAudioParams->sDspChParams.eType,
                        &sSpdifFmParams.eBurstRepPeriod);
        if(ret != BERR_SUCCESS){goto stop_clone;} 
		
		/* Get the default mixer parameters*/
		ret = BRAP_MIXER_P_GetDefaultParams (&sMixerParams);
		if(ret != BERR_SUCCESS){goto stop_clone;} 
		
		/* Get the default source channel parameters */
		ret = BRAP_SRCCH_P_GetDefaultParams (&sSrcChParams);
		if(ret != BERR_SUCCESS){goto stop_clone;} 

        /* Normal playback path: data is not written to srcFIFO directly */
        sSrcChParams.bCapDirect2SrcFIFO = false;

        if (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
            /* It is pass-thru context of simul mode, so data is compressed */
    		sSpdifFmParams.bCompressed = true;
            sMixerParams.bCompress = true;	
		    sSrcChParams.bCompress = true;		
			sSrcChParams.bStartSelection = true;		
            sMixerParams.uiSrcChId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiSrcChId;
                    sMixerParams.uiStreamRes  = BRAP_P_BITS_PER_SAMPLE_FOR_COMPRESSED_DATA;

			/* Set bits per sample = 16, as DSP generates 16 bits per sample for compressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e16;

			/* Ring buffers to be mapped to this source channel */
			sSrcChParams.uiLtOrSnglRBufId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0];
			sSrcChParams.uiRtRBufId = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1];
        }
        else
        {
            sSpdifFmParams.bCompressed = false;
            sMixerParams.bCompress = false;	
	     sSrcChParams.bCompress = false;		
            sMixerParams.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiSrcChId;        

			/* Set bits per sample = 32, as DSP generates 32 bits per sample for uncompressed data */
			sSrcChParams.eInputBitsPerSample = BRAP_InputBitsPerSample_e32;

			/* Ring buffers to be mapped to this source channel */
			sSrcChParams.uiLtOrSnglRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[0];
			sSrcChParams.uiRtRBufId = hRapCh->sRsrcGrnt.sOpResrcId[BRAP_OutputChannelPair_eLR].uiRbufId[1];
        sMixerParams.uiStreamRes 
			= pAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample;
            
		}
		
        /* whether or not to override CBIT[20:21] for left/right channel num */
        sSpdifFmParams.bSeparateLRChanNum = 
                pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum;
        
		/* Mixer parameters */
		sMixerParams.sExtParams 
			= pAudioParams->sSimulPtAudioOutputParams.sMixerParams;
	
		/* RBuf parameters, use same parameters for all associated ring buffers */
        for(k=0; k< BRAP_RM_P_MAX_RBUFS_PER_SRCCH; k++)
        {
	        ret = BRAP_RBUF_P_GetDefaultParams (&sRBufParams[k]);
	        if(ret != BERR_SUCCESS){goto stop_clone;}    
        }

		/* Source channel parameters */
		sSrcChParams.bInputSrValid = true;  /* Input audio data coming from DSP */
		sSrcChParams.eBufDataMode = BRAP_BufDataMode_eStereoNoninterleaved;	/* Stereo output by default */

		/* For a decode channel, no need to program eInputSR & eOutputSR, 
		   as this is going to be programmed by DSP */


	/*Checking if the PCM data is being tried to mix with compressed data*/
	ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
												hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort,
												sMixerParams.bCompress,
												&bIsPCMMixedWithCompressed);
	
	if(ret != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM returned Error for simultaneous mode "));
		goto stop_clone;
	}

	if(bIsPCMMixedWithCompressed)
	{
	    ret = BERR_NOT_SUPPORTED;
		BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for simultaneous mode "));
		goto stop_clone;
	}

		/* Start the internal FMM modules */
		ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							BRAP_OutputChannelPair_eLR,
							&(hRapCh->sSimulPtModuleHandles));
		if(ret != BERR_SUCCESS){
			if (true==bWatchdogRecoveryOn)
				return BERR_TRACE(ret);
			else
				goto stop_clone;
		}

       /* Start all Cloned ports (for simulPt context) associated with this channel */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
        {    
            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp != NULL) 
                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == true))
            {      
                BDBG_MSG(("BRAP_DEC_Start: output port %d was cloned for SimultPT", j));
                
                ret = BRAP_P_FormOpParams (hRapCh, 
                        (BRAP_OutputPort)j, 
                        hRapCh->hRap->sOutputSettings[j].eOutputTimebase, 
                        pAudioParams->sAudioOutputParams.uiOutputBitsPerSample, 
                        &pOpParams);

                /* ReUse the same SpdifFmParams as filled in for parent SimulPt port above */
                /* ReUse the same MixerParams as filled in for the parent SimulPt port above, except SrcChId */
                if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh != NULL)
                {
                    /* cloned port uses another SrcCh */
                    /* take the same Params as the orig src ch */
                    sSrcChParams = hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams;

                    /* We need to store the RBUF ID's, as in 7400 the Cloned 
                       channel may use separate RBUF's allocated to it.*/
                    sSrcChParams.uiLtOrSnglRBufId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiRbufId[0];
                    sSrcChParams.uiRtRBufId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiRbufId[1];
                    BDBG_MSG(("sSrcChParams.uiLtOrSnglRBufId=%d,sSrcChParams.uiRtRBufId=%d,output=%d",
                            sSrcChParams.uiLtOrSnglRBufId,sSrcChParams.uiRtRBufId,j));

                   /* this Src Ch shares the rbufs with the Orig SrCh */
                   sSrcChParams.bSharedRbuf = true;
                   sSrcChParams.uiMasterSrcCh = 
                                    hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->uiIndex;
       
                   sMixerParams.uiSrcChId = hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiSrcChId ;     
                   BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the new SrcCh %d (master =srcch %d)", j, sMixerParams.uiSrcChId, sSrcChParams.uiMasterSrcCh));                       
                }
                else
                {
                    sMixerParams.uiSrcChId = hRapCh->sSimulPtModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->uiIndex; 

                    for (k = 0; k < BRAP_RM_P_MAX_OUTPUTS; k++)
                    {    
                        if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hSrcCh != NULL)
                        { 
                            break;
                        }
                    }
                    if (k!=BRAP_RM_P_MAX_OUTPUTS)
                    {
                        /* One of the cloned ports has a specially allocated hSrcCh
                        => one port was cloned in another DP, say DPx */
                        /* if eOutputPort belongs to DPx, it should use 
                        hSrcCh from sCloneOpPathHandles, else from sSimulPtModuleHandles */
                        if (hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][j].uiDataPathId
                            == hRapCh->sCloneOpPathResc[BRAP_OutputChannelPair_eLR][k].uiDataPathId)

                        {
                            sMixerParams.uiSrcChId = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hSrcCh->uiIndex;
                        }
                    }							
                    BDBG_MSG(("BRAP_DEC_Start: cloned output port %d uses the SrcCh %d ", j, sMixerParams.uiSrcChId));                                               
                }            

                /* Form BRAP_P_ObjectHandles structure */
                sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[0];
                sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR + 1] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[1]; 
                sTempHandles.hSrcCh[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;
                sTempHandles.hMixer[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
                sTempHandles.uiMixerInputIndex[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].uiMixerInputIndex;  
                sTempHandles.hSpdifFm[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSpdifFm;  
                sTempHandles.hOp[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;   
                   

    	        /*Checking if the PCM data is being tried to mix with compressed data*/
    	        ret=BRAP_P_CheckIfCompressedMixedWithPCM(hRapCh->hRap,
    			    									j,
    					    							sTempHandles.hMixer[BRAP_OutputChannelPair_eLR]->bCompress,
    				    								&bIsPCMMixedWithCompressed);
    	
    	        if(ret != BERR_SUCCESS)
    	        {
    		        BDBG_ERR(("BRAP_DEC_Start: BRAP_P_CheckIfCompressedMixedWithPCM failed for Cloned Simul PT output"));
    		        goto stop_simul;
    	        }

    	        if(bIsPCMMixedWithCompressed)
    	        {
    	            ret = BERR_NOT_SUPPORTED;
    		        BDBG_ERR(("BRAP_DEC_Start: Trying to mix PCM data with Compressed Data for Cloned output"));
    		        goto stop_simul;
    	        }
    	
                BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
                /* Start the internal FMM modules */
                ret = BRAP_P_StartOpPathFmmModules(
                                hRapCh,
                                &sRBufParams[0],
                                &sSrcChParams,
                                &sMixerParams,
                                &sSpdifFmParams,
                                pOpParams,
                                BRAP_OutputChannelPair_eLR,
                                &(sTempHandles));
                if(ret != BERR_SUCCESS){goto stop_simul;}
            }
        	}            
	}
	} /* false==bWatchdogRecoveryOn */
	else { /* If watchdog recovery */
		ret = BRAP_DSPCHN_P_GetCurrentAudioParams(hRapCh->sModuleHandles.hDspCh, psDspChParams);
		if(ret != BERR_SUCCESS)
		{
			ret = BERR_TRACE(ret);				
			goto end_start;			
		}
		
		if ( (psDspChParams->sExtAudioParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode) ) {
          
			/* Start the internal FMM modules */
			ret = BRAP_P_StartOpPathFmmModules(
							hRapCh,
							&sRBufParams[0],
							&sSrcChParams,
							&sMixerParams,
							&sSpdifFmParams,
							pOpParams,
							BRAP_OutputChannelPair_eLR,
							&(hRapCh->sSimulPtModuleHandles));
			if(ret != BERR_SUCCESS)
				return BERR_TRACE(ret);

            /* If the SimulPt port has been cloned, start the clones also */
            sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;
			
	        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
	        {    
	            if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp != NULL) 
	                && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == true))
	            {      
				
		            /* Form BRAP_P_ObjectHandles structure */
		            sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[0];
		            sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR + 1] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[1]; 
		            sTempHandles.hSrcCh[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;
		            sTempHandles.hMixer[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
		            sTempHandles.uiMixerInputIndex[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].uiMixerInputIndex;  
		            sTempHandles.hSpdifFm[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSpdifFm;  
		            sTempHandles.hOp[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;   
						
		            BDBG_MSG (("BRAP_DEC_Start: temp handle formed.")); 
		            /* Start the internal FMM modules */
		            ret = BRAP_P_StartOpPathFmmModules(
		                            hRapCh,
		                            &sRBufParams[0],
		                            &sSrcChParams,
		                            &sMixerParams,
		                            &sSpdifFmParams,
		                            pOpParams,
		                            BRAP_OutputChannelPair_eLR,
		                            &(sTempHandles));
		            if(ret != BERR_SUCCESS){goto stop_simul;}
	            }
	        }
		}
	}


	/* Start the DSP Channel */
	if (false==bWatchdogRecoveryOn) 
    {

    	/* Populate sDspChnParams structure */
    	psDspChParams->sExtAudioParams = pAudioParams->sDspChParams;
    	psDspChParams->eTimebase = pAudioParams->eTimebase;

        /* Initialize the Transport Channel ID.
        This is used to program the RAVE_CTX_SEL field */
    	psDspChParams->uiTransChId = hRapCh->uiXptChannelNo;

        psDspChParams->bPlayback =  pAudioParams->bPlayback;

        /* FIXME: usage of uiNumOpPorts and j here is redundant. only one shud be used. 
        should count sOpPortParams.uiNumOpPorts instead of taking from this array */
    	/* Number of output port required: Each output port can carry 2 audio channels */

        psDspChParams->bMultiChanOnI2S = false;

    	/* Initialize the ring buffer index array in sDspChnParams */
    	for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++)
    	{
    		psDspChParams->sDecOrPtParams.rBufIndex[i] = BRAP_RM_P_INVALID_INDEX;
            psDspChParams->sSimulPtParams.rBufIndex[i] = BRAP_RM_P_INVALID_INDEX;
    	}

        /* Fill up the ring buffer index array in sDspChnParams */
        for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++) 
    	{
    		if(hRapCh->sModuleHandles.hRBuf[i] == NULL)
    		{
    			/* For the current output mode, this rbuf is not 
    			   used, so skip the opening of the modules for this channel */
    			continue;
    		}
    		psDspChParams->sDecOrPtParams.rBufIndex[i] = hRapCh->sModuleHandles.hRBuf[i]->uiIndex;
    	}

    	if(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {
            /* Pass the RBUFS to be used for passthru context of simul mode.
               No extra RBUFS are needed for the clone output */
            for(i=0; i < BRAP_RM_P_MAX_OP_CHANNELS; i++) 
        	{
    	    	if(hRapCh->sSimulPtModuleHandles.hRBuf[i] == NULL)
    		    {
    			    /* For the current output mode, this output channel is not 
    	    		   used, so skip the opening of the modules for this channel */
        			continue;
    		    }
                psDspChParams->sSimulPtParams.rBufIndex[i] = hRapCh->sSimulPtModuleHandles.hRBuf[i]->uiIndex;
            }
        }

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    psDspChParams->sDecOrPtParams.bLargeRbuf = hRapCh->bLargeRbuf;
    psDspChParams->sSimulPtParams.bLargeRbuf = hRapCh->bLargeRbuf;     
#endif        
        
    	/* Pre-initialize output ports and PLL fields to BRAP_RM_P_INVALID_INDEX */
    	for (i = 0; i <  BRAP_RM_P_MAX_OUTPUTS; i++) 
        {
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].eOutputPortType 
    			 = BRAP_OutputPort_eMax;
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].ePll 
    			 = BRAP_OP_Pll_eMax;

    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].eOutputPortType 
    			 = BRAP_OutputPort_eMax;
    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].ePll 
    			 = BRAP_OP_Pll_eMax;
            
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)            
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].iDelay = 0; 
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].uiDlydRBufIndex[0]
    			 = BRAP_RM_P_INVALID_INDEX;
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].uiDlydRBufIndex[1]
    			 = BRAP_RM_P_INVALID_INDEX;      
    		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].uiSrcChIndex
    			 = BRAP_RM_P_INVALID_INDEX;        
            psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[i].eChanPair
    			 = BRAP_RM_P_INVALID_INDEX;        

    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].iDelay = 0; 
    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].uiDlydRBufIndex[0]
    			 = BRAP_RM_P_INVALID_INDEX;
    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].uiDlydRBufIndex[1]
    			 = BRAP_RM_P_INVALID_INDEX;      
    		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].uiSrcChIndex
    			 = BRAP_RM_P_INVALID_INDEX;        
            psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[i].eChanPair
    			 = BRAP_RM_P_INVALID_INDEX;              
#endif            
    	}

    	/* Fill up the output ports required for the current Output Audio Mode */
    	j = 0;
    	for (i = 0; i < uiMaxChPairs; i++)
    	{
            if ( hRapCh->sModuleHandles.hOp[i] == NULL )
            {
                /* This output channel pair is not used,
                so skip the opening of the modules for this channel */
                continue;
            }

            eOutputPort = hRapCh->sModuleHandles.hOp[i]->eOutputPort;
            /* For the main output port associated with this channel pair */
            psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                    = eOutputPort;
            BRAP_P_GetPllForOp (hRapCh->hRap, 
                    eOutputPort, 
                    &(psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].ePll));        

            /* If port is Mai, pass the MuxSelect option to DSP instead of Mai */
            if (eOutputPort == BRAP_OutputPort_eMai)
            {
                /* Populate the SPDIF settings properly if MAI input is SPDIF */
                if ( eMaiMuxSelect == BRAP_OutputPort_eSpdif)
                {
                    psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                        = BRAP_OutputPort_eSpdif;
                }
                if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
                {
                    psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                        = BRAP_OutputPort_eFlex;
                }        
            }

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
            /* If this channel has large rbufs and if indep delay is required for 
            this port, configure parameters required by FW.
            Note: this is a master port which has a delay */            
            if ((hRapCh->bLargeRbuf == true) 
                && (hRapCh->hRap->sOutputSettings[eOutputPort].bIndpDlyRqd == true))
            {
                BDBG_MSG(("Note: this master port %d has a %d ms delay requested.", eOutputPort, 
                    hRapCh->hRap->sOutputSettings[eOutputPort].iDelay));     

            
        		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].iDelay 
                    = hRapCh->hRap->sOutputSettings[eOutputPort].iDelay;
        		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[0]
        			 = hRapCh->sModuleHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_PORT*i]->uiIndex;
        		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[1]
        			 = hRapCh->sModuleHandles.hRBuf[BRAP_RM_P_MAX_RBUFS_PER_PORT*i+1]->uiIndex;
        		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiSrcChIndex
        			 = hRapCh->sModuleHandles.hSrcCh[i]->uiIndex;  
                psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eChanPair = (BRAP_OutputChannelPair)i;              
            }   
#endif            
            j++;    /* increment count */
          
            /* If there are any cloned ports, also add them */
            for (k=0; k< BRAP_RM_P_MAX_OUTPUTS; k++)
            {
                if (hRapCh->sCloneOpPathHandles[i][k].hOp != NULL)
                {
                    if (hRapCh->sCloneOpPathHandles[i][k].bSimul == true)
                        continue;
                    psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                        = hRapCh->sCloneOpPathHandles[i][k].hOp->eOutputPort;
                
                    BRAP_P_GetPllForOp (hRapCh->hRap, 
                        hRapCh->sCloneOpPathHandles[i][k].hOp->eOutputPort, 
                        &(psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].ePll)); 

                    /* If port is Mai, pass the MuxSelect option to DSP instead of Mai */
                    if (k == BRAP_OutputPort_eMai)
                    {
                        /* Populate the SPDIF settings properly if MAI input is SPDIF */
                        if (eMaiMuxSelect == BRAP_OutputPort_eSpdif )
                        {
                            psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                                = BRAP_OutputPort_eSpdif;
                        }
                        if (eMaiMuxSelect == BRAP_OutputPort_eFlex )
                        {
                            psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                                = BRAP_OutputPort_eFlex;
                        }
                    }   
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
                    /* If this channel has large rbufs and if indep delay is required for 
                    this port, configure parameters required by FW.
                    Note: this is a cloned port which has a delay */
                    if ((hRapCh->bLargeRbuf == true) 
                        && (hRapCh->hRap->sOutputSettings[k].bIndpDlyRqd == true))
                    {

                        BDBG_MSG(("Note: this cloned port %d has a %d ms delay requested.", k, 
                            hRapCh->hRap->sOutputSettings[k].iDelay));     
                
                		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].iDelay 
                            = hRapCh->hRap->sOutputSettings[k].iDelay;
                		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[0]
                            = hRapCh->sCloneOpPathHandles[i][k].hRbuf[0]->uiIndex;
                		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[1]
                            = hRapCh->sCloneOpPathHandles[i][k].hRbuf[1]->uiIndex;
                		psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].uiSrcChIndex
                            = hRapCh->sCloneOpPathHandles[i][k].hSrcCh->uiIndex; 
                        psDspChParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[j].eChanPair = (BRAP_OutputChannelPair)i;                      
                    }                   
#endif                    
                    j ++;                     
                }
            }
        }
        psDspChParams->sDecOrPtParams.sOpPortParams.uiNumOpPorts = j;    
        
        /* For Simultaneous mode, add the output port for the Pass-thru context
           and for cloned output.  */
        if ( (pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
        {
            psDspChParams->sSimulPtParams.sOpPortParams.uiNumOpPorts = 1;
            j=0;
            eOutputPort = hRapCh->sSimulPtModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;

            if (eOutputPort == BRAP_OutputPort_eMai)
            {
                psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType
                    = eMaiMuxSelect;
                psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].ePll
                    = hRapCh->hRap->sOutputSettings[eMaiMuxSelect].uOutputPortSettings.sSpdifSettings.ePll;
            }
            else
            {
                psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType
                        = eOutputPort;

                /* at present, mai and spdif are the only 2 options for the SimulPt 
                port. Mai is dealt with in the if case above. so here, the port can 
                only be spdif */
                if (eOutputPort != BRAP_OutputPort_eSpdif)
                {
                    BDBG_ERR(("Invalid simul pt port"));
                }
                psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].ePll 
                    = hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;
            }       
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
            /* If this channel has large rbufs and if indep delay is required for 
            this port, configure parameters required by FW.
            Note: this is a master SimulPt port which has a delay */
            if ((hRapCh->bLargeRbuf == true) 
                && (hRapCh->hRap->sOutputSettings[eOutputPort].bIndpDlyRqd == true))
            {
                BDBG_MSG(("Note: this SimulPt port %d has a %d ms delay requested.", eOutputPort, 
                    hRapCh->hRap->sOutputSettings[eOutputPort].iDelay));
                
        		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].iDelay 
                    = hRapCh->hRap->sOutputSettings[eOutputPort].iDelay;
        		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[0]
        			 = hRapCh->sSimulPtModuleHandles.hRBuf[j]->uiIndex;
        		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].uiDlydRBufIndex[1]
        			 = hRapCh->sSimulPtModuleHandles.hRBuf[j+1]->uiIndex;
        		psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].uiSrcChIndex
        			 = hRapCh->sSimulPtModuleHandles.hSrcCh[j]->uiIndex;  
                psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eChanPair = BRAP_OutputChannelPair_eLR;
            }  
#endif            
            j++;
            
            /* If there are any cloned ports, also add them */
            for (k=0; k< BRAP_RM_P_MAX_OUTPUTS; k++)
            {
                if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hOp != NULL)
                {
                    if (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].bSimul == false)
                        continue;
                    psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                        = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][k].hOp->eOutputPort;
                

                    /* If port is Mai, pass the MuxSelect option to DSP instead of Mai */
                    if (k == BRAP_OutputPort_eMai)
                    {
                        /* Populate the SPDIF settings properly if MAI input is SPDIF */
                        if (eMaiMuxSelect == BRAP_OutputPort_eSpdif )
                        {
                            psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                                = BRAP_OutputPort_eSpdif;
                        }
                        if (eMaiMuxSelect == BRAP_OutputPort_eFlex )
                        {
                            psDspChParams->sSimulPtParams.sOpPortParams.sOpPortConfig[j].eOutputPortType 
                                = BRAP_OutputPort_eFlex;
                        }
                    }                       
                    j ++;                     
                }
            } 
            psDspChParams->sSimulPtParams.sOpPortParams.uiNumOpPorts = j;           
        }
        /* Add the SPDIF channel status buffer params to the sDspChParams */
        psDspChParams->sSpdifChStatusParams=pAudioParams->sSpdifChanStatusParams;

#ifdef RAP_SRSTRUVOL_CERTIFICATION
        BRAP_Write32(hRapCh->hRegister,BCHP_AUD_DSP_CFG0_SW_UNDEFINED_SPAREi_ARRAY_BASE+40,pAudioParams->uiNumChannels); /* Number of channels */
        BRAP_Write32(hRapCh->hRegister,BCHP_AUD_DSP_CFG0_SW_UNDEFINED_SPAREi_ARRAY_BASE+56,pAudioParams->eInputSamplingRate);	
#endif

#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)

#if (BCHP_CHIP == 7400)
        for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            if(hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId != BRAP_RM_P_INVALID_INDEX)
            {
                if((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
                     ||(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode))
                {
                    /* For the new TSM scheme, program adaptive rate control */
                    BRAP_P_ProgramAdaptRateCtrl (hRapCh);
                }             
            }            

        } 
#else
    /* For the new TSM scheme, program adaptive rate control */
    BRAP_P_ProgramAdaptRateCtrl (hRapCh);
#endif

#endif
  	} /* false==bWatchdogRecoveryOn */



	/* Start the DSP Channel */
	ret = BRAP_DSPCHN_P_Start(hRapCh->sModuleHandles.hDspCh, psDspChParams);
	if(ret != BERR_SUCCESS){
		if (true==bWatchdogRecoveryOn)
			return BERR_TRACE(ret);
		else
			goto stop_simul;
	}

	if (false==bWatchdogRecoveryOn) {
	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) ||
		(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
			hRapCh->hRap->uiDecodeCount++;
	}

	if ((pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru) || 
	(pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {
		hRapCh->hRap->uiPassThruCount++;
	}
	}

    /*Since everything went fine, Set channel status to started */
    hRapCh->bStarted = true;
	goto end_start;

stop_simul:
		rc = BRAP_P_StopOpPathFmmModules(BRAP_OutputChannelPair_eLR, &(hRapCh->sSimulPtModuleHandles));
		BDBG_ASSERT(rc == BERR_SUCCESS);    


stop_clone:
    sTempHandles.hFmm = hRapCh->sModuleHandles.hFmm;

   /* stop all Cloned ports associated with this channel */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {  
                if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL)
                {              
                     /* Form BRAP_P_ObjectHandles structure */
                     sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                     sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                     sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                     sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                     sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                     sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                     sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;   

        		rc = BRAP_P_StopOpPathFmmModules((BRAP_OutputChannelPair)i, &(sTempHandles));
        		BDBG_ASSERT(rc == BERR_SUCCESS);    
                  }
            }
     }

stop_master:
	for(i=0; i < uiMaxChPairs; i++)
	{
		if ( hRapCh->sModuleHandles.hOp[i] == NULL )
		{
    			/* This output channel pair is not used,
    			so skip the opening of the modules for this channel */
			continue;
		}
		rc = BRAP_P_StopOpPathFmmModules((BRAP_OutputChannelPair)i, &(hRapCh->sModuleHandles));
		BDBG_ASSERT(rc == BERR_SUCCESS);
	}   
    
return_error:
		/* Returning on error. Restore back channel state variables. */
		hRapCh->bStarted = false;

	/* Free the DSP Contexts allocated during the start */
	sResrcGrant.uiNumOpPorts = 1;
	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
	{    
		sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
		sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;
        sResrcGrant.sOpResrcId[i].uiPpmId = BRAP_RM_P_INVALID_INDEX;
	}
	sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
	sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
	sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
	sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
	sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
	for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
		sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 

	
        sResrcGrant.uiDspId = hRapCh->sRsrcGrnt.uiDspId;
        sResrcGrant.uiDspContextId = hRapCh->sRsrcGrnt.uiDspContextId;

	/* Free the resources in the RM */
	rc = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
	if (rc != BERR_SUCCESS)
	{
		BDBG_ERR (("BRAP_DEC_Start: call to BRAP_RM_P_FreeResources() failed.!!!!!"));
		rc = BERR_TRACE (ret);
	}    
	BDBG_MSG(("Resources freed"));

	if ( pAudioParams->sDspChParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode )
	{
		sResrcGrant.uiNumOpPorts = 1;
		for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
		{    
			sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
			sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;
            sResrcGrant.sOpResrcId[i].uiPpmId = BRAP_RM_P_INVALID_INDEX;
		}
		sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
		sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
		sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
		for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
			sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 

		
	        sResrcGrant.uiDspId = hRapCh->sSimulPtRsrcGrnt.uiDspId;
	        sResrcGrant.uiDspContextId = hRapCh->sSimulPtRsrcGrnt.uiDspContextId;

		/* Free the resources in the RM */
		rc = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR (("BRAP_DEC_Start: call to simul BRAP_RM_P_FreeResources() failed.!!!!!"));
			rc = BERR_TRACE (ret);
		}    
		BDBG_MSG(("Simulpt resources freed"));
	}

#if (BCHP_CHIP == 7400)

	if ((false==bWatchdogRecoveryOn)&& ((pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
     ||(pAudioParams->sDspChParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)))
	{

        for(k = BRAP_OutputChannelPair_eLR; k <= BRAP_OutputChannelPair_eDownMixedLR; k++)
        {
            /* Check for all DownMixed LR */
            if(hRapCh->sModuleHandles.hOp[k]!=NULL)
            {
        		sResrcGrant.uiNumOpPorts = 1;
        		for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        		{    
        			sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
        			sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
        			sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;
                    sResrcGrant.sOpResrcId[i].uiPpmId = BRAP_RM_P_INVALID_INDEX;
        		}
        		sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
        		sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
        		sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
        		sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
        		sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
        		for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
        			sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 

                for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
                {
                    if(hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId != BRAP_RM_P_INVALID_INDEX)
                    {
                		sResrcGrant.sOpResrcId[i].uiPpmId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId;
                    }            

                }

        		/* Free the resources in the RM */
        		rc= BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
        		if (rc != BERR_SUCCESS)
        		{
        			BDBG_ERR (("BRAP_DEC_Start: call to BRAP_RM_P_FreeResources() failed.!!!!!"));
        			rc = BERR_TRACE (ret);
        		}    
        		BDBG_MSG(("PPM resources freed"));
        	}
        }

        for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
        {
            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {
                if (hRapCh->hRap->sOutputSettings[j].bIndpDlyRqd == true)
                {                  
                    if (hRapCh->sCloneOpPathHandles[i][j].hOp != NULL) 
                        {
                    		sResrcGrant.uiNumOpPorts = 1;
                    		for (k = 0; k < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; k++)
                    		{    
                    			sResrcGrant.sOpResrcId[k].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
                    			sResrcGrant.sOpResrcId[k].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiMixerId = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
                    			sResrcGrant.sOpResrcId[k].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;
                                sResrcGrant.sOpResrcId[k].uiPpmId = BRAP_RM_P_INVALID_INDEX;
                    		}
                    		sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
                    		sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
                    		sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
                    		sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
                    		sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
                    		for(k = 0; k < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; k++)
                    			sResrcGrant.sCapResrcId.uiRbufId[k] = BRAP_RM_P_INVALID_INDEX; 

                            for (k = 0; k < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; k++)
                            {
                                if(hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId != BRAP_RM_P_INVALID_INDEX)
                                {
                            		sResrcGrant.sOpResrcId[k].uiPpmId = hRapCh->sRsrcGrnt.sOpResrcId[k].uiPpmId;
                                }            

                            }

                    		/* Free the resources in the RM */
                    		rc= BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
                    		if (rc != BERR_SUCCESS)
                    		{
                    			BDBG_ERR (("BRAP_DEC_Start: call to BRAP_RM_P_FreeResources() failed.!!!!!"));
                    			rc = BERR_TRACE (ret);
                    		}    
                    		BDBG_MSG(("PPM resources freed"));
                        }
                }
            }
        } 
     }
#endif

end_start:

	BKNI_Free(psDspChParams);
	
	BDBG_LEAVE(BRAP_DEC_Start);

	return (ret);
}
#endif

/***************************************************************************
Summary:
	Stops a decode channel.

Description:
	This API is required to stop the decode/pass-through/SRC of the selected 
	channel (stream).	

1a. (PCM mode)Stop the associated DSP Context operation(stop decoding but ring buffer still playing)

1b. (Compress SPDIF mode)Stop the associated DSP Context operation, insert one pause/null burst in ring buffer

2a. (PCM mode)Mute associated PCM output stream in DP and wait for DP volume ramp down done interrupt  

2b. (Compressed SPDIF mode)Play until ring buffer empty 

3. *Stop the associated Output operation.

4. *Stop the associated Microsequencer operation

5. Stop the associated Mixer operation

5. Stop the associated Source FIFOs' operation(close gate)

6. Stop the associated Ring buffers' operation

At step 2 ring buffer should have more than 1 frame of data left.

*Never shut down SPDIF output.

 
    
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_StartDecode
	
****************************************************************************/
BERR_Code BRAP_DEC_Stop ( 
	BRAP_ChannelHandle 	hRapCh		/* [in] The RAP Decode Channel handle */
	)
{
	BERR_Code		ret = BERR_SUCCESS;
	unsigned int	i;
    unsigned int    uiMaxChPairs;
	BRAP_DEC_AudioParams	sAudioParams;
#ifndef BCHP_7411_VER /* not for 7411 */   
       unsigned int j;
    	BRAP_P_ObjectHandles    	sTempHandles;       
#endif
	BRAP_RM_P_ResrcGrant sResrcGrant;

	BDBG_ENTER(BRAP_DEC_Stop);


	if (hRapCh->bStarted == false) {
		BDBG_ERR(("Channel is already stopped"));
		return BERR_SUCCESS;
	}

#if BCHP_7411_VER > BCHP_VER_C0
    if (true == hRapCh->bMuxChPairOnI2sStarted)
    	uiMaxChPairs = BRAP_P_MUXED_I2S_CHANNEL_PAIRS;
    else
    	uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#else
  	uiMaxChPairs = BRAP_RM_P_MAX_OP_CHANNEL_PAIRS;
#endif		
    
	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

	ret = BRAP_DEC_P_GetCurrentAudioParams(hRapCh, &sAudioParams);
	if (ret!=BERR_SUCCESS)
		return BERR_TRACE(ret);
	
    	switch (sAudioParams.sDspChParams.eDecodeMode)
    	{
        	/* Modes supported:  */
        	case BRAP_DSPCHN_DecodeMode_eDecode:
			hRapCh->hRap->uiDecodeCount--;
			break;
       	case BRAP_DSPCHN_DecodeMode_ePassThru:
			hRapCh->hRap->uiPassThruCount--;
			break;
	       case BRAP_DSPCHN_DecodeMode_eSimulMode:
			hRapCh->hRap->uiDecodeCount--;
			hRapCh->hRap->uiPassThruCount--;
 	            	break;
	        /* Not supported */
  	 	default:
  	 		/* Decode mode field in RAP decode channel handle corrupted */
		        return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
  	  }


        hRapCh->hInterruptCount->uiCrcError=0;
        hRapCh->hInterruptCount->uiDecoderUnlock=0;
        hRapCh->hInterruptCount->uiFmmCRC=0;
        hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark=0;
        hRapCh->hInterruptCount->uiPtsError=0;

#ifndef BCHP_7411_VER /* not for 7411 */   
       BKNI_Memset (&sTempHandles, 0, sizeof(sTempHandles));
#endif
	/* Mute this channel with a ramp down. */
    /* Start ramp down for all mixers/srcch associated with this audio channel*/


	/* Note: For pass-thru context of simultaneous mode, data is compressed therefore cant mute it.
       For cloned output ports: Muting the srcCh automatically mutes both mixers connected to it.*/
	for(i=0; i < uiMaxChPairs; i++)
	{
#if BCHP_7411_VER > BCHP_VER_C0
        if (false == hRapCh->bMuxChPairOnI2sStarted)
		{
#endif		
		if ( hRapCh->sModuleHandles.hOp[i] == NULL )
		{
    			/* This output channel pair is not used,
    			so skip the opening of the modules for this channel */
			continue;
		}

#if BCHP_7411_VER > BCHP_VER_C0
        }/* if hRapCh->bMuxChPairOnI2sStarted == false */
#endif		

	    /* TODO: Dont call BRAP_MIXER_SetMute since that will mute the output
           port while other Audio Channels are using it */

        /* We can mute/unmute only for uncompressed data */
        if (hRapCh->sModuleHandles.hSrcCh[i]->sParams.bCompress == false)
        {
            ret = BRAP_SRCCH_P_SetMute (hRapCh->sModuleHandles.hSrcCh[i], 
                                    hRapCh->sModuleHandles.hMixer[i],
                                    true);
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_SRCCH_P_SetMute() failed. Ignoring error!!!!!"));
                ret = BERR_TRACE (ret);
            }
        }
	}
#if 0
/* The ramp down interrupt does not happen currently. So this PI just times out.
Commenting it out since it only adds extra delay */
    /* Make sure ramp down is completed for all mixers/srcch associated with 
       this audio channel*/
	for(i=0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
	{
		if((opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2] == false) && 
		   (opAudModeProp[hRapCh->eMaxOpMode].bChnExists[i * 2 + 1] == false))
		{
			/* For the current output mode, this output channel pair is not 
			   used, so skip the stopping of the modules for this channel */
			continue;
		}

		/* We can mute/unmute only for uncompressed data */
		if (hRapCh->sModuleHandles.hSrcCh[i]->sParams.bCompress == false)
        {
			ret = BRAP_SRCCH_P_WaitForRampDown (hRapCh->sModuleHandles.hSrcCh[i], 
										hRapCh->sModuleHandles.hMixer[i]);
            if (ret != BERR_SUCCESS)
            {
                BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_SRCCH_P_WaitForRampDown() failed. Ignoring error!!!!!"));
                BERR_TRACE (ret);
            }
		}
	}
#endif

    /* Program the Ramp Amount */
    for(j = 0; j < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; j++)
    {
        if(NULL == hRapCh->sModuleHandles.hSpdifFm[j])
            continue;
        BRAP_Write32 (hRapCh->sModuleHandles.hSpdifFm[j]->hRegister,
                      BCHP_AUD_FMM_MS_CTRL_FW_RAMP_AMOUNT_0 + hRapCh->sModuleHandles.hSpdifFm[j]->ui32Offset, 
                      0x800); 
    }

	/* Stop the DSP Channel */
	ret = BRAP_DSPCHN_P_Stop(hRapCh->sModuleHandles.hDspCh);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_DSPCHN_P_Stop() failed. Ignoring error!!!!!"));
        ret = BERR_TRACE (ret);
    }

	/* Stop the modules of passThru context of simultaneous mode or clone output, if configured.*/
   
	if ((hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode 
	   == BRAP_DSPCHN_DecodeMode_eSimulMode)
#ifdef BCHP_7411_VER  /* only for 7411 */  	   
         || (hRapCh->eClone == BRAP_P_CloneState_eStarted)
#endif         
         )           

	{

#ifndef BCHP_7411_VER /* not for 7411 */    
        /* Stop all cloned ports associated with SimulPt ports */
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {
                if ((hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp != NULL) 
                    && (hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].bSimul == true))
                {

                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[0];
                    sTempHandles.hRBuf[2*BRAP_OutputChannelPair_eLR + 1] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSrcCh;
                    sTempHandles.hMixer[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hSpdifFm;  
                    sTempHandles.hOp[BRAP_OutputChannelPair_eLR] = hRapCh->sCloneOpPathHandles[BRAP_OutputChannelPair_eLR][j].hOp;                  

                    /* Stop cloned modules associated with this output port */
                    ret = BRAP_P_StopOpPathFmmModules (BRAP_OutputChannelPair_eLR, &sTempHandles);
                    if (ret != BERR_SUCCESS)
                    {
                        BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() for simulpt cloned ports failed. Ignoring error!!!!!"));
                        ret = BERR_TRACE (ret);
                    }
               }
         }
#endif

		/* Stop the internal FMM modules associated with the channel */
		ret = BRAP_P_StopOpPathFmmModules(BRAP_OutputChannelPair_eLR, &(hRapCh->sSimulPtModuleHandles));
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() failed. Ignoring error!!!!!"));
            ret = BERR_TRACE (ret);
        }
#ifdef BCHP_7411_VER  /* only for 7411 */          
        /* Update Cloning state */
        hRapCh->eClone = BRAP_P_CloneState_eConfigured;
#endif
	}

	
#ifndef BCHP_7411_VER /* not for 7411 */    
        /* Stop all cloned ports associated with this channel */
    for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    {
        for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
           {
                if ((hRapCh->sCloneOpPathHandles[i][j].hOp != NULL) 
                    && (hRapCh->sCloneOpPathHandles[i][j].bSimul == false))
                {

                    /* Form BRAP_P_ObjectHandles structure */
                    sTempHandles.hRBuf[2*i] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[0];
                    sTempHandles.hRBuf[2*i + 1] = hRapCh->sCloneOpPathHandles[i][j].hRbuf[1]; 
                    sTempHandles.hSrcCh[i] = hRapCh->sCloneOpPathHandles[i][j].hSrcCh;
                    sTempHandles.hMixer[i] = hRapCh->sCloneOpPathHandles[i][j].hMixer;  
                    sTempHandles.uiMixerInputIndex[i] = hRapCh->sCloneOpPathHandles[i][j].uiMixerInputIndex;  
                    sTempHandles.hSpdifFm[i] = hRapCh->sCloneOpPathHandles[i][j].hSpdifFm;  
                    sTempHandles.hOp[i] = hRapCh->sCloneOpPathHandles[i][j].hOp;                  

                    /* Stop cloned modules associated with this output port */
                    ret = BRAP_P_StopOpPathFmmModules ((BRAP_OutputChannelPair)i, &sTempHandles);
                    if (ret != BERR_SUCCESS)
                    {
                        BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() for cloned ports failed. Ignoring error!!!!!"));
                        ret = BERR_TRACE (ret);
                    }
               }
         }
    }
#endif

	/* Stop the internal FMM modules associated with the channel */
	for(i=0; i < uiMaxChPairs; i++)
	{
#if BCHP_7411_VER > BCHP_VER_C0
        if (false == hRapCh->bMuxChPairOnI2sStarted)
		{
#endif		
		if ( hRapCh->sModuleHandles.hOp[i] == NULL )
		{
    			/* This output channel pair is not used,
    			so skip the opening of the modules for this channel */
			continue;
		}
#if BCHP_7411_VER > BCHP_VER_C0
        }
#endif			
		ret = BRAP_P_StopOpPathFmmModules((BRAP_OutputChannelPair)i, &(hRapCh->sModuleHandles));
        if (ret != BERR_SUCCESS)
        {
            BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_P_StopOpPathFmmModules() failed. Ignoring error!!!!!"));
            ret = BERR_TRACE (ret);
        }
	}

#if BCHP_7411_VER > BCHP_VER_C0
    	if (true == hRapCh->bMuxChPairOnI2sStarted)
    	{
		ret = BRAP_P_I2sAlignmentWorkaroundStop(hRapCh);
	}	
#endif	

#if (BCHP_CHIP == 7400)                
	if(hRapCh->sModuleHandles.hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain] > BRAP_OutputMode_e2_0)
	{
		BDBG_MSG(("Disabling output allignment"));
		ret = BRAP_OP_P_DisableAllignment( hRapCh->hRap, 
	                    false, /* bSpdif */ 
	                    true, /* bI2s0 */   
	                    true, /* bI2s1 */   
	                    true  /* bI2s2 */);
	}
#endif	

	/* Free the DSP Contexts allocated during the start */
	sResrcGrant.uiNumOpPorts = 1;
	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
	{    
		sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
		sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
        sResrcGrant.sOpResrcId[i].uiPpmId = hRapCh->sRsrcGrnt.sOpResrcId[i].uiPpmId;
	}
	sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
	sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
	sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
	sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
	sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
	for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
		sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 

	
        sResrcGrant.uiDspId = hRapCh->sRsrcGrnt.uiDspId;
        sResrcGrant.uiDspContextId = hRapCh->sRsrcGrnt.uiDspContextId;

	/* Mask interrupts and uninstall callbacks */
	BRAP_P_InterruptUnInstall (hRapCh);

    
	/* Free the resources in the RM */
	ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
	if (ret != BERR_SUCCESS)
	{
		BDBG_ERR (("BRAP_DEC_Stop: call to BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
		ret = BERR_TRACE (ret);
	}    
	BDBG_MSG(("freed dsp context %d ", sResrcGrant.uiDspContextId));

	if ( hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode 
	   	== BRAP_DSPCHN_DecodeMode_eSimulMode)
	{
		sResrcGrant.uiNumOpPorts = 1;
		for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
		{    
			sResrcGrant.sOpResrcId[i].eOutputPortType = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiRbufId[0] = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiRbufId[1] = BRAP_RM_P_INVALID_INDEX;    
			sResrcGrant.sOpResrcId[i].uiSrcChId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiDataPathId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiMixerId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiMixerInputId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiSpdiffmId = BRAP_RM_P_INVALID_INDEX;
			sResrcGrant.sOpResrcId[i].uiSpdiffmStreamId = BRAP_RM_P_INVALID_INDEX;  
		}
		sResrcGrant.uiDspId= BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.uiDspContextId = BRAP_RM_P_INVALID_INDEX;
		sResrcGrant.uiFmmId = BRAP_RM_P_INVALID_INDEX;   
		sResrcGrant.sCapResrcId.eInputPortType = BRAP_RM_P_INVALID_INDEX;   
		sResrcGrant.sCapResrcId.uiDstChId = BRAP_RM_P_INVALID_INDEX;   
		for(i = 0; i < BRAP_RM_P_MAX_RBUFS_PER_DSTCH; i++)
			sResrcGrant.sCapResrcId.uiRbufId[i] = BRAP_RM_P_INVALID_INDEX; 

		
	        sResrcGrant.uiDspId = hRapCh->sSimulPtRsrcGrnt.uiDspId;
	        sResrcGrant.uiDspContextId = hRapCh->sSimulPtRsrcGrnt.uiDspContextId;

		/* Free the resources in the RM */
		ret = BRAP_RM_P_FreeResources(hRapCh->hRap->hRm, &(sResrcGrant));
		if (ret != BERR_SUCCESS)
		{
			BDBG_ERR (("BRAP_DEC_Stop: call to simul BRAP_RM_P_FreeResources() failed. Ignoring error!!!!!"));
			ret = BERR_TRACE (ret);
		}    
		BDBG_MSG(("freed Simul dsp context %d ", sResrcGrant.uiDspContextId));
	}

	hRapCh->bStarted = false;
	hRapCh->bMuxChPairOnI2sStarted = false;

	BDBG_LEAVE(BRAP_DEC_Stop);

	return BERR_SUCCESS;
}


/***************************************************************************
Summary:
	This API flushes CDB/ITB buffers associated with a decode channel for 7411 only.
	And for other chips this fucntion clears the itnernal buffers owned by PI and FW.

Description:
	Along with CDB/ITB buffers(7411), various buffers present in the decode pipeline also get flushed.
	BRAP_DEC_DisableForFlush() API should be called before calling BRAP_DEC_Flush() API.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
****************************************************************************/
BERR_Code	BRAP_DEC_Flush (
	BRAP_ChannelHandle	hRapCh		/* [in] The RAP Decode Channel handle */
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_DEC_AudioParams	sAudioParams;

	BDBG_ENTER(BRAP_DEC_Flush);
	/* Assert on invalid input parameters */
	BDBG_ASSERT(hRapCh);

	BKNI_Memset (&sAudioParams, 0, sizeof(sAudioParams));

	/* Set "Internal Call" flag in the channel handle. Some of the APIs
	 * behave differently for internal Vs external calls. For example,
	 * in BRAP_DEC_Start, trick mode status is reset only on external
	 * calls. 
	 */
	BKNI_EnterCriticalSection();
	hRapCh->bInternalCallFromRap = true;
	BKNI_LeaveCriticalSection();

	/* Get current audio parameters for this channel */
	err = BRAP_DEC_P_GetCurrentAudioParams(hRapCh, &sAudioParams);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);

        BDBG_MSG(("BRAP_DEC_Flush: got the following parameters"));

#ifdef BCHP_7411_VER /* For 7411 Only */
        BDBG_MSG(("\t eOutputMode=0x%x", sAudioParams.eOutputMode));
#endif

        BDBG_MSG(("\t Decoder eTimebase=0x%x", sAudioParams.eTimebase));

        BDBG_MSG(("\t sDspChParams.eDecodeMode=0x%x", sAudioParams.sDspChParams.eDecodeMode));
        BDBG_MSG(("\t sDspChParams.eType=0x%x", sAudioParams.sDspChParams.eType));
        BDBG_MSG(("\t sDspChParams.eStreamType=0x%x", sAudioParams.sDspChParams.eStreamType));
        BDBG_MSG(("\t sDspChParams.eAacXptFormat=0x%x", sAudioParams.sDspChParams.eAacXptFormat));
        BDBG_MSG(("\t sDspChParams.i32AVOffset=0x%x", sAudioParams.sDspChParams.i32AVOffset));

        BDBG_MSG(("\t sAudioOutputParams.uiOutputBitsPerSample=0x%x", sAudioParams.sAudioOutputParams.uiOutputBitsPerSample));
        BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", sAudioParams.sAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
        BDBG_MSG(("\t sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", sAudioParams.sAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
        BDBG_MSG(("\t sAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", sAudioParams.sAudioOutputParams.sSpdifFmParams.bSpdifFormat));

        BDBG_MSG(("\t sSimulPtAudioOutputParams.uiOutputBitsPerSample=0x%x", sAudioParams.sSimulPtAudioOutputParams.uiOutputBitsPerSample));
        BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode=0x%x", sAudioParams.sSimulPtAudioOutputParams.sMixerParams.sInputParams.eMixerInputAudioMode));
        BDBG_MSG(("\t sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue=0x%x", sAudioParams.sSimulPtAudioOutputParams.sMixerParams.sInputParams.uiScaleValue));
        BDBG_MSG(("\t sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat=0x%x", sAudioParams.sSimulPtAudioOutputParams.sSpdifFmParams.bSpdifFormat));

        BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bProfessionalMode=0x%x", sAudioParams.sSpdifChanStatusParams.bProfessionalMode));
        BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSeparateLRChanNum=0x%x", sAudioParams.sSpdifChanStatusParams.bSeparateLRChanNum));
        BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.bSwCopyRight=0x%x", sAudioParams.sSpdifChanStatusParams.bSwCopyRight));
        BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16CategoryCode=0x%x", sAudioParams.sSpdifChanStatusParams.ui16CategoryCode));
        BDBG_MSG(("\t pAudioParams->sSpdifChanStatusParams.ui16ClockAccuracy=0x%x", sAudioParams.sSpdifChanStatusParams.ui16ClockAccuracy));

        BDBG_MSG(("\t bPlayback=0x%x", sAudioParams.bPlayback));
   
	
#ifdef BCHP_7411_VER /* For 7411 Only */
	/* Stop DEC channel */
	err = BRAP_DEC_Stop(hRapCh);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);
#endif	

#if BRAP_P_USE_BRAP_TRANS == 1
	/* Stop transport channel */
	err = BRAP_TRANS_StopChannel(hRapCh->hRapTransCh);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);

	/* Start transport channel */
	err = BRAP_TRANS_P_StartChannel(hRapCh->hRapTransCh);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);
#endif

	/* Start DEC channel. Since this is an internal call, all the modules
	 * need to use audioparams stored in their respective handles 
	 */
	 if(hRapCh->bStarted==false)
	 {
		err = BRAP_DEC_Start(hRapCh, &sAudioParams);
		if (err != BERR_SUCCESS)
			return BERR_TRACE(err);
	 }
	 else
	 {
	 	BDBG_ERR(("Please call BRAP_DEC_DisableForFlush function before calling BRAP_DEC_Flush function"));
	 }
	/* Restore the trick modes */
	if (hRapCh->sTrickModeState.bAudioPaused==true) {
		err = BRAP_PVR_EnablePause(hRapCh, true);
		if (err != BERR_SUCCESS)
			return BERR_TRACE(err);
		/* Reset the frame advance residual time */
		hRapCh->sTrickModeState.uiFrameAdvResidualTime = 0;
	}


	/* Reset "Internal Call" flag in the channel handle */
	BKNI_EnterCriticalSection();
	hRapCh->bInternalCallFromRap = false;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_DEC_Flush);
	return err;
}

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/***************************************************************************
Summary:
	This API is used to inform the decoder to stop reading data from RAVE in
	preparation for flush. This API should be called before calling BRAP_DEC_Flush() PI
	
Description:
	This API just stops the decoder channel from reading data from CDB/ITB.
	This API should be called before calling BRAP_DEC_Flush() PI.

Returns:
	BERR_SUCCESS on success
	Error value on failure

See Also:
****************************************************************************/
BERR_Code  BRAP_DEC_DisableForFlush ( 
	BRAP_ChannelHandle	hRapCh		/* [in] The RAP Decode Channel handle */
	)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER(BRAP_DEC_DisableForFlush);
	/* Assert on invalid input parameters */
	BDBG_ASSERT(hRapCh);

#ifdef BCHP_7411_VER /* For 7411 this is not supported */
	return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

	/* Set "Internal Call" flag in the channel handle. Some of the APIs
	 * behave differently for internal Vs external calls. For example,
	 * in BRAP_DEC_Start, trick mode status is reset only on external
	 * calls. 
	 */
	BKNI_EnterCriticalSection();
	hRapCh->bInternalCallFromRap = true;
	BKNI_LeaveCriticalSection();

	/* Stop DEC channel */
	err = BRAP_DEC_Stop(hRapCh);
	if (err != BERR_SUCCESS)
		return BERR_TRACE(err);

	/* Reset "Internal Call" flag in the channel handle */
	BKNI_EnterCriticalSection();
	hRapCh->bInternalCallFromRap = false;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_DEC_DisableForFlush);
	return err;

}
#endif /* For chips other than 7411 */

/***************************************************************************
Summary:
	Change decoder configuration parameters on the fly

Description:
	This function sets decoder configuration parameters that can be set
	on the fly for audio type (eType) mentioned in the configuration
	parameter structure (psDecConfigParams). Decoder configuration parameteters
	contain audio-type specific parameters (elements in union) and common
	parameters for all the audio types (elements outside union).
	Change in configuration parameters depend on the timing of calling
	this API. There are following three cases.
	1. If currently decode is not happening on this channel then all configuration
	parameters are stored in PI. When next decode starts, common parameters are applied
	irrespective of audio type of next decode, but audio-type specific parameters
	are applied only on next decode with same audio type.
	2. If currently channel is decoding other audio type,then only common parameters 
	are applied immediately and audio-type specific parameters are stored in PI. 
	Audio-type specific parameters are applied on next decode start with same audio 
	type.
	3. If decode is running with same audio type, all the configuration parameters are 
	applied immediately.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_GetDefaultConfig
	BRAP_DEC_GetCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_SetConfig (
	BRAP_ChannelHandle		hRapCh,		/* [in] RAP channel handle */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_DEC_DownmixPath eDownmixPath;
#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
	BRAP_PP_ConfigParams sPpConfigParams;
#endif

#ifndef  BCHP_7411_VER	
	bool	bLfeOn = 0;
#endif

	BDBG_ENTER (BRAP_DEC_SetConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psDecConfigParams);

#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
   	sPpConfigParams = psDecConfigParams->sPpConfigParams;
#endif


#ifdef  BCHP_7411_VER
	if(psDecConfigParams->eOutputMode > hRapCh->eMaxOpMode)
	{
		BDBG_ERR(("Current output mode %d should be less than or equal "
				  "to the output mode %d configured at the channel open time", 
                   psDecConfigParams->eOutputMode, hRapCh->eMaxOpMode));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
#endif

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	eDownmixPath = psDecConfigParams->eDownmixPath;
#else
	eDownmixPath = BRAP_DEC_DownmixPath_eMain;
#endif	

#ifndef  BCHP_7411_VER
#if ( BCHP_CHIP == 7400 )
	if ( eDownmixPath == BRAP_DEC_DownmixPath_eStereoDownmix )
	{
		if ( hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eDownMixedLR] == NULL || 
		      psDecConfigParams->eOutputMode != BRAP_OutputMode_eStereo )
		{
			BDBG_ERR(("Either the output port is not added for Downmixed LR port OR"
				  		"the output mode is not stereo"));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
		}
	}
	else
	{
#endif

	/* Current FW will handle case "mono to ALL" only if the output mode is 1_0.
	For other output modes, we should return error if bMonoToAll is set as TRUE.*/
        if ((psDecConfigParams->bMonoToAll == true)&&(psDecConfigParams->eOutputMode!=BRAP_OutputMode_e1_0))
        {
            BDBG_ERR (("BRAP_DEC_SetConfig: eOutputMode=%d. bMonoToALL==TRUE is "
                "supported only for Output MOde 1_0. Please change output mode to 1_0.", 
                psDecConfigParams->eOutputMode)); 
            return BERR_TRACE(BERR_NOT_SUPPORTED);	
        }

		switch ( psDecConfigParams->eType )
		{
			case BRAP_DSPCHN_AudioType_eAc3:
				bLfeOn = psDecConfigParams->uConfigParams.sAc3ConfigParams.bOutputLfeOn;
				break;
			case BRAP_DSPCHN_AudioType_eAc3Lossless:
				bLfeOn = psDecConfigParams->uConfigParams.sAc3LosslessConfigParams.bOutputLfeOn;
				break;
			case BRAP_DSPCHN_AudioType_eAc3Plus:
				bLfeOn = psDecConfigParams->uConfigParams.sAc3PlusConfigParams.bOutputLfeOn;
				break;
#if (BRAP_DTS_SUPPORTED == 1)		
			case BRAP_DSPCHN_AudioType_eDts:
				bLfeOn = psDecConfigParams->uConfigParams.sDtsCoreConfigParams.bOutputLfeOn;
				break;                
#else                
			case BRAP_DSPCHN_AudioType_eDts:
				bLfeOn = psDecConfigParams->uConfigParams.sDtsConfigParams.bOutputLfeOn;
				break;
#endif                
			case BRAP_DSPCHN_AudioType_eLpcmBd:
				bLfeOn = psDecConfigParams->uConfigParams.sBdlpcmConfigParams.bOutputLfeOn;
				break;
			case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
				bLfeOn = psDecConfigParams->uConfigParams.sLpcmHdDvdConfigParams.bOutputLfeOn;
				break; 
			case BRAP_DSPCHN_AudioType_eDtshd:
				bLfeOn = psDecConfigParams->uConfigParams.sDtshdConfigParams.bOutputLfeOn;
				break;           
			case BRAP_DSPCHN_AudioType_eLpcmDvd:
				bLfeOn = psDecConfigParams->uConfigParams.sLpcmDvdConfigParams.bOutputLfeOn;
				break;
			case BRAP_DSPCHN_AudioType_eMlp:
				bLfeOn = psDecConfigParams->uConfigParams.sMlpConfigParams.bOutputLfeOn;
				break;
			case BRAP_DSPCHN_AudioType_eMpeg:
#ifdef RAP_SRSTRUVOL_CERTIFICATION  
			case BRAP_DSPCHN_AudioType_ePCM:
#endif				
			case BRAP_DSPCHN_AudioType_eAac:
			case BRAP_DSPCHN_AudioType_eAacSbr:
			case BRAP_DSPCHN_AudioType_eWmaStd:				
			case BRAP_DSPCHN_AudioType_eWmaPro:		
			case BRAP_DSPCHN_AudioType_ePcmWav:		                
			case BRAP_DSPCHN_AudioType_eDra:			
				break;
			default:
				BDBG_ERR(("Audio Type %d not supported",psDecConfigParams->eType ));
				return BERR_TRACE(BERR_INVALID_PARAMETER);
		}

		if ( (psDecConfigParams->eType != BRAP_DSPCHN_AudioType_eAc3) &&
			(psDecConfigParams->eType != BRAP_DSPCHN_AudioType_eAc3Plus) )
		{
			if ( bLfeOn == true &&
				hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eCentreLF] == NULL )
			{
				BDBG_ERR(("There is no output port added for the channel pair CentreLFE"
							"But LFE output is getting enabled"));
				return BERR_TRACE(BERR_NOT_SUPPORTED);				
			}
		}
#if ( BCHP_CHIP == 7400 )		
	}
#endif
#endif
	
	err = BRAP_DSPCHN_P_SetConfig (
			hRapCh->sModuleHandles.hDspCh, eDownmixPath, psDecConfigParams);
	if (err != BERR_SUCCESS)
		return BERR_TRACE (err);

#ifdef BCHP_7411_VER  /* only for 7411 */     			
	/* Store output mode in RAP channel handle */
	if ( eDownmixPath==BRAP_DEC_DownmixPath_eMain )
	{
		hRapCh->eCurOpMode = psDecConfigParams->eOutputMode;
	}

#endif

	if ( hRapCh->bStarted == true )
	{
		if ( hRapCh->sModuleHandles.hSrcCh[0] != NULL )
		{
			switch ( psDecConfigParams->eDualMonoMode )
			{
				case BRAP_DSPCHN_DualMonoMode_eLeftMono:
					hRapCh->sModuleHandles.hSrcCh[0]->sParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_L_2_LR;
					break;
				case BRAP_DSPCHN_DualMonoMode_eRightMono:
					hRapCh->sModuleHandles.hSrcCh[0]->sParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_R_2_LR;
					break;
				case BRAP_DSPCHN_DualMonoMode_eStereo:
					hRapCh->sModuleHandles.hSrcCh[0]->sParams.eLRDataCntl = BRAP_SRCCH_P_LRDataControl_LR_2_LR;
					break;
				default:
					break;
			}

			err = BRAP_SRCCH_P_ProgramLRControl(hRapCh->sModuleHandles.hSrcCh[0]);
			if (err != BERR_SUCCESS)
			{
				BDBG_ERR(("Error in progarming the LR control"));
				return BERR_TRACE (err);
			}
		}
	}

	BDBG_LEAVE (BRAP_DEC_SetConfig);
	return err;
}

/***************************************************************************
Summary:
	Change Post Processing configuration parameters on the fly

Description:
	This function sets post processing configuration parameters that can be set
	on the fly for PP algo type  mentioned in the configuration
	parameter structure (psPpConfigParsms). Pp configuration parameteters
	contain pp-type specific parameters (elements in union) and common
	parameters for all the pp types (elements outside union).
	Change in configuration parameters depend on the timing of calling
	this API. There are following three cases.
	1. If currently decode is not happening on this channel then all configuration
	parameters are stored in PI. When next decode starts, common parameters are applied
	irrespective of audio type of next decode, but pp-type specific parameters
	are applied only on next decode with same audio type.
	2. If currently channel is decoding other audio type,then only common parameters 
	are applied immediately and audio-type specific parameters are stored in PI. 
	Audio-type specific parameters are applied on next decode start with same audio 
	type.
	3. If decode is running with same audio type, all the configuration parameters are 
	applied immediately.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_P_GetDefaultPPConfig
	BRAP_DEC_GetPpCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_SetPPConfig (
	BRAP_ChannelHandle		hRapCh,		        /* [in] RAP channel handle */
	BRAP_PP_ConfigParams		*psPpConfigParsms	/* [in] Post processing configuration
												    parameters */
	)
{
	BERR_Code               err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_SetPPConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psPpConfigParsms);
	
	err = BRAP_DSPCHN_P_SetPPConfig(hRapCh->sModuleHandles.hDspCh, 
                        		  psPpConfigParsms);
	if (err != BERR_SUCCESS)
		return BERR_TRACE (err);

	BDBG_LEAVE (BRAP_DEC_SetPPConfig);
	return err;
}





BERR_Code BRAP_DEC_P_GetCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   Current configuration parameters */
	BRAP_DEC_DownmixPath	eDownmixPath,	/* [in] Downmix path of interest.
											     Default values of output mode 
											     related parameters belongs to
											     this downmix path */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_DSPCHN_P_Settings	*psDefDspChSettings;

	BDBG_ENTER (BRAP_DEC_P_GetCurrentConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psDecConfigParams);
    if(eDownmixPath >= BRAP_DEC_DownmixPath_eMax)
    	return BERR_TRACE(BERR_INVALID_PARAMETER);

	psDefDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
	if (NULL==psDefDspChSettings)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}

	psDecConfigParams->eType = eType;
#ifdef BCHP_7411_VER  /* only for 7411 */      		
	psDecConfigParams->eOutputMode = hRapCh->eCurOpMode;	
#else
	psDecConfigParams->eOutputMode = hRapCh->sModuleHandles.hDspCh->sSettings.eOutputMode[eDownmixPath];	
#endif

#if ( defined BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
	psDecConfigParams->eDownmixPath = eDownmixPath;
#endif
	err = BRAP_DSPCHN_P_GetCurrentSettings(
			hRapCh->sModuleHandles.hDspCh, psDefDspChSettings); 
	if (err != BERR_SUCCESS)
	{
		BKNI_Free(psDefDspChSettings);
		return BERR_TRACE(err);
	}

	psDecConfigParams->eOutputMode = psDefDspChSettings->eOutputMode[eDownmixPath];	
	psDecConfigParams->bMonoToAll = psDefDspChSettings->bMonoToAll[eDownmixPath];
	BDBG_MSG(("BRAP_DEC_P_GetCurrentConfig: bMonoToAll=%d, eDownmixPath=%d", 
		psDecConfigParams->bMonoToAll, eDownmixPath));        
    
	psDecConfigParams->eDualMonoMode = psDefDspChSettings->sDspchnExtSettings[eDownmixPath].eDualMonoMode;
	/* Get DDBM parameters only from Main */
	psDecConfigParams->sDdbmConfigParams = psDefDspChSettings->sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sDdbmConfigParams;

	
	switch (eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
#ifdef RAP_SRSTRUVOL_CERTIFICATION          
		case BRAP_DSPCHN_AudioType_ePCM:
#endif			
		case BRAP_DSPCHN_AudioType_eWmaStd:			
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			psDecConfigParams->uConfigParams.sAacConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAacConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			psDecConfigParams->uConfigParams.sAacSbrConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAacSbrConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
			psDecConfigParams->uConfigParams.sAc3ConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3ConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			psDecConfigParams->uConfigParams.sAc3LosslessConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3LosslessConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			psDecConfigParams->uConfigParams.sAc3PlusConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3PlusConfigParam;
			break;
#if (BRAP_DTS_SUPPORTED == 1)		
		case BRAP_DSPCHN_AudioType_eDts:
			psDecConfigParams->uConfigParams.sDtsCoreConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtsCoreConfigParam;
			break;
#else            
		case BRAP_DSPCHN_AudioType_eDts:
			psDecConfigParams->uConfigParams.sDtsConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtsConfigParam;
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			psDecConfigParams->uConfigParams.sBdlpcmConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sBdlpcmConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			psDecConfigParams->uConfigParams.sLpcmHdDvdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sLpcmHdDvdConfigParam;
			break; 
		case BRAP_DSPCHN_AudioType_eDtshd:
			psDecConfigParams->uConfigParams.sDtshdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtshdConfigParam;
			break;           
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			psDecConfigParams->uConfigParams.sLpcmDvdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sLpcmDvdConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eMlp:
			psDecConfigParams->uConfigParams.sMlpConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sMlpConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eWmaPro:
			psDecConfigParams->uConfigParams.sWmaProConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sWmaProConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_ePcmWav:
			psDecConfigParams->uConfigParams.sPcmWavConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sPcmWavConfigParams;
			break;            
		case BRAP_DSPCHN_AudioType_eDra:
			psDecConfigParams->uConfigParams.sDraConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDraConfigParams;          
			break;
		default:
			BDBG_ERR(("Audio Type %d not supported",eType));
			err = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BKNI_Free(psDefDspChSettings);
	BDBG_LEAVE (BRAP_DEC_P_GetCurrentConfig);
	return err;
}


BERR_Code BRAP_DEC_P_GetDefaultConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_DEC_DownmixPath	eDownmixPath,	/* [in] Downmix path of interest.
											     Default values of output mode 
											     related parameters belongs to
											     this downmix path */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
    BERR_Code err = BERR_SUCCESS;
	BRAP_DSPCHN_P_Settings	*psDefDspChSettings;

	BDBG_ENTER (BRAP_DEC_P_GetDefaultConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psDecConfigParams);
	BSTD_UNUSED  (hRapCh);
    if(eDownmixPath >= BRAP_DEC_DownmixPath_eMax)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	psDefDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
	if (NULL==psDefDspChSettings)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
    
	psDecConfigParams->eType = eType;
	psDecConfigParams->eOutputMode = BRAP_OutputMode_e2_0;	/* Default output is 
														   stereo */
	psDecConfigParams->bMonoToAll = false;

#if ( defined BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
	psDecConfigParams->eDownmixPath = eDownmixPath;
#endif
	err = BRAP_DSPCHN_P_GetDefaultSettings(psDefDspChSettings);
	if (err != BERR_SUCCESS)
	{
		BKNI_Free(psDefDspChSettings);
		return BERR_TRACE(err);
	}

	psDecConfigParams->eDualMonoMode = psDefDspChSettings->sDspchnExtSettings[eDownmixPath].eDualMonoMode;

	/* Get post processing parameters only from Main */
	psDecConfigParams->sDdbmConfigParams = psDefDspChSettings->sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sDdbmConfigParams;

	switch (eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
#ifdef RAP_SRSTRUVOL_CERTIFICATION
		case BRAP_DSPCHN_AudioType_ePCM:
#endif 			
		case BRAP_DSPCHN_AudioType_eWmaStd:						
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			psDecConfigParams->uConfigParams.sAacConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAacConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			psDecConfigParams->uConfigParams.sAacSbrConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAacSbrConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
			psDecConfigParams->uConfigParams.sAc3ConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3ConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			psDecConfigParams->uConfigParams.sAc3LosslessConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3LosslessConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			psDecConfigParams->uConfigParams.sAc3PlusConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sAc3PlusConfigParam;
			break;
#if (BRAP_DTS_SUPPORTED == 1)		
		case BRAP_DSPCHN_AudioType_eDts:
			psDecConfigParams->uConfigParams.sDtsCoreConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtsCoreConfigParam;
			break;
#else 
		case BRAP_DSPCHN_AudioType_eDts:
			psDecConfigParams->uConfigParams.sDtsConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtsConfigParam;
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			psDecConfigParams->uConfigParams.sBdlpcmConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sBdlpcmConfigParam;
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			psDecConfigParams->uConfigParams.sLpcmHdDvdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sLpcmHdDvdConfigParam;
			break; 
		case BRAP_DSPCHN_AudioType_eDtshd:
			psDecConfigParams->uConfigParams.sDtshdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sDtshdConfigParam;
			break;           
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			psDecConfigParams->uConfigParams.sLpcmDvdConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sLpcmDvdConfigParam;
			break; 
		case BRAP_DSPCHN_AudioType_eMlp:
			psDecConfigParams->uConfigParams.sMlpConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sMlpConfigParams;
			break; 
		case BRAP_DSPCHN_AudioType_eWmaPro:
			psDecConfigParams->uConfigParams.sWmaProConfigParams
				= psDefDspChSettings->sDspchnCfgParams[eDownmixPath].sWmaProConfigParams;
			break; 
		default:
			BDBG_ERR(("Audio Type %d not supported",eType));
			err = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BKNI_Free(psDefDspChSettings);
	BDBG_LEAVE (BRAP_DEC_P_GetDefaultConfig);
	return err;
}


BERR_Code BRAP_DEC_P_GetInterruptCount (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_InterruptCount	*pInterruptCount	/* [out] Decoder configuration
												   parameters */
	)
{
    BERR_Code err = BERR_SUCCESS;
	BDBG_ENTER (BRAP_DEC_P_GetInterruptCount);

	pInterruptCount->uiCrcError = hRapCh->hInterruptCount->uiCrcError;
    pInterruptCount->uiDecoderUnlock = hRapCh->hInterruptCount->uiDecoderUnlock;
    pInterruptCount->uiFmmCRC = hRapCh->hInterruptCount->uiFmmCRC;
    pInterruptCount->uiFmmDstRbufFullWaterMark = hRapCh->hInterruptCount->uiFmmDstRbufFullWaterMark;
    pInterruptCount->uiPtsError = hRapCh->hInterruptCount->uiPtsError;

	pInterruptCount->uiTotalFrameDecoded = hRapCh->hInterruptCount->uiTotalFrameDecoded;
	pInterruptCount->uiFramesInError = hRapCh->hInterruptCount->uiFramesInError;
	BDBG_LEAVE (BRAP_DEC_P_GetInterruptCount);
	return err;
}

BERR_Code BRAP_DEC_P_GetDefaultPPConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_PP_Algo	ePpType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_PP_ConfigParams	*psPpConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
    BERR_Code err = BERR_SUCCESS;
	BRAP_DSPCHN_P_Settings	*psDefDspChSettings = NULL;
	
	BDBG_ENTER (BRAP_DEC_P_GetDefaultPPConfig);
	
	BDBG_ASSERT (hRapCh);
	BSTD_UNUSED  (hRapCh);

	psDefDspChSettings = BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Settings));
	if (NULL==psDefDspChSettings)
	{
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
	}
   
	err = BRAP_DSPCHN_P_GetDefaultSettings(psDefDspChSettings);
	if (err != BERR_SUCCESS)
	{
		BKNI_Free(psDefDspChSettings);
		return BERR_TRACE(err);
	}

	psPpConfigParams->ePpAlgo  = ePpType;
	psPpConfigParams->uiPpBranchId = 0;
	psPpConfigParams->uiPpStage = 0;
	
	switch( psPpConfigParams->ePpAlgo )
	{
#if (BRAP_AD_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
			psPpConfigParams->uConfigParams.sAdFadeConfigParams
				= psDefDspChSettings->sPPSettings.sADFadeCtrlConfigParams;
			break;
		case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
			psPpConfigParams->uConfigParams.sAdPanConfigParams
				= psDefDspChSettings->sPPSettings.sADPanCtrlConfigParams;
			break;
#endif			
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
			psPpConfigParams->uConfigParams.sDolbyVolumeConfigParams
				= psDefDspChSettings->sPPSettings.sDolbyVolumeConfigParams;
			break;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
			psPpConfigParams->uConfigParams.sSRSTruVolConfigParams
				= psDefDspChSettings->sPPSettings.sSRSTruVolConfigParams;
			break;
#endif			
		default:
			break;
	}

	BKNI_Free(psDefDspChSettings);
	BDBG_LEAVE (BRAP_DEC_P_GetDefaultPPConfig);
	return err;
}





/***************************************************************************
Summary:
	Get default decoder configuration parameters

Description:
	This function gets default configuration parameters for given audio 
	type (eType) parameter. 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetConfig
	BRAP_DEC_GetCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetDefaultConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_GetDefaultConfig);
    err = BRAP_DEC_P_GetDefaultConfig(hRapCh, eType, 
            BRAP_DEC_DownmixPath_eMain, psDecConfigParams); 
	BDBG_LEAVE (BRAP_DEC_GetDefaultConfig);
	return err;
}


/***************************************************************************
Summary:
	API used to get the output port configuration.

Description:
	This API is used to get the output configuration of an output port. 
	If the caller has not configured the output port before, this API 
	will return error. 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_GetInterruptCount
****************************************************************************/
BERR_Code BRAP_DEC_GetInterruptCount( 
	BRAP_ChannelHandle 		hRapCh,			/* [in] The Raptor Channel handle*/
	BRAP_InterruptCount	*pInterruptCount/* [out] Interrupt Count */
	)
{
    BERR_Code ret = BERR_SUCCESS;
	
           
	BDBG_ENTER(BRAP_GetInterruptCount);
    
	/* Assert the function argument(s) */
	BDBG_ASSERT(hRapCh);
	
	ret = BRAP_DEC_P_GetInterruptCount(hRapCh,pInterruptCount);
	
	BDBG_LEAVE(BRAP_GetInterruptCount);

	return ret;
}

									   
/***************************************************************************
Summary:
	Get default post processing configuration parameters

Description:
	This function gets default configuration parameters for given post 
	processing type (ePpType) parameter. 

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetPPConfig
	BRAP_DEC_GetPpCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetDefaultPPconfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_PP_Algo	ePpType,	/* [in] PP algo type for which to get
									   default configuration parameters */
	BRAP_PP_ConfigParams		*psPpConfigParsms	/* [Out] Post processing configuration
												    parameters */							   
	)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_GetDefaultPPconfig);
    err = BRAP_DEC_P_GetDefaultPPConfig(hRapCh, ePpType,psPpConfigParsms); 
	BDBG_LEAVE (BRAP_DEC_GetDefaultPPconfig);
	return err;
}


/***************************************************************************
Summary:
	Get current decoder configuration parameters

Description:
	This function gets current configuration parameters for given audio 
	type (eType) parameter.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetConfig
	BRAP_DEC_GetDefaultConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   Current configuration parameters */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_GetCurrentConfig);
    err = BRAP_DEC_P_GetCurrentConfig (hRapCh, eType, 
        BRAP_DEC_DownmixPath_eMain, psDecConfigParams);
	BDBG_LEAVE (BRAP_DEC_GetCurrentConfig);
	return err;
}



/***************************************************************************
Summary:
	Get current Post Processing configuration parameters

Description:
	This function gets current configuration parameters for given Pp 
	type (uiPpAlgo) parameter.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetPPConfig
	BRAP_DEC_GetDefaultPPConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetPpCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_PP_Algo	uiPpAlgo,	/* [in] Audio type for which to get
									   Current configuration parameters */
	unsigned int				uiPpBranchId,	/* [in] Branch Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_BRANCH_SUPPORTED */
	unsigned int				uiPpStageId,		/* [in] Stage Id of the post
											processing stage for which to get
											current counfiguration parameters.
											If current configuration is not
											required for any post processing
											stage, this field should be set to
											BRAP_MAX_PP_PER_BRANCH_SUPPORTED */																		   
	BRAP_PP_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;
	BRAP_DSPCHN_PP_Algo ePpAlgo;
	BRAP_DSPCHN_P_PpStgConfigParam sPpStgConfigParam;

	BDBG_ENTER (BRAP_DEC_GetPpCurrentConfig);

	err = BRAP_DSPCHN_P_GetCurrentPpConfig( hRapCh->sModuleHandles.hDspCh, uiPpBranchId, uiPpStageId, &ePpAlgo, &sPpStgConfigParam );
	if (err != BERR_SUCCESS)
	{
		return BERR_TRACE(err);
	}

	if(ePpAlgo !=uiPpAlgo)
	{
		BDBG_ERR(("Requested BranchId = %d, StageID = %d and PPAlgo Id = %d are not matching",
			uiPpBranchId,uiPpStageId,uiPpAlgo));
		err=BERR_INVALID_PARAMETER;
		return BERR_TRACE(err);
	}

	psDecConfigParams->ePpAlgo = ePpAlgo;
	psDecConfigParams->uiPpBranchId = uiPpBranchId;
	psDecConfigParams->uiPpStage = uiPpStageId;

	switch( ePpAlgo )
	{
#if (BRAP_AD_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
			psDecConfigParams->uConfigParams.sAdFadeConfigParams
				= sPpStgConfigParam.sAdFadeConfigParams;
			break;
		case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
			psDecConfigParams->uConfigParams.sAdPanConfigParams
				= sPpStgConfigParam.sAdPanConfigParams;
			break;
#endif			
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)					
		case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
			psDecConfigParams->uConfigParams.sDolbyVolumeConfigParams
				= sPpStgConfigParam.sDolbyVolumeConfigParams;
			break;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)					
		case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
			psDecConfigParams->uConfigParams.sSRSTruVolConfigParams
				= sPpStgConfigParam.sSRSTruVolConfigParams;
			break;
#endif			
		default:
			break;
	}

	BDBG_LEAVE (BRAP_DEC_GetPpCurrentConfig);
	return err;
}


#if ( defined BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
/***************************************************************************
Summary:
	Gets default decoder configuration parameters for a given downmix path

Description:
	This function gets default configuration parameters for given audio type (eType) 
	parameter and a given downmix path. Essentially this API is similar to 
	BRAP_DEC_GetDefaultConfig with the difference being BRAP_DEC_GetDefaultConfig
	gets default configuration parameters of only main downmix path.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetConfig
	BRAP_DEC_GetDownmixPathCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetDownmixPathDefaultConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   default configuration parameters */
	BRAP_DEC_DownmixPath	eDownmixPath,	/* [in] Downmix path of interest.
											     Default values of output mode 
											     related parameters belongs to
											     this downmix path */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
    BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_GetDownmixPathDefaultConfig);
    err = BRAP_DEC_P_GetDefaultConfig(hRapCh, eType, eDownmixPath, psDecConfigParams);
	BDBG_LEAVE (BRAP_DEC_GetDownmixPathDefaultConfig);
	return err;
}
	
/***************************************************************************
Summary:
	Gets current decoder configuration parameters for a given downmix path

Description:
	This function gets current configuration parameters for given audio type (eType) 
	parameter and a given downmix path. Essentially this API is similar to 
	BRAP_DEC_GetCurrentConfig with the difference being BRAP_DEC_GetCurrentConfig
	gets current configuration parameters of only main downmix path.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_DEC_SetConfig
	BRAP_DEC_GetDefaultPathCurrentConfig
****************************************************************************/
BERR_Code BRAP_DEC_GetDownmixPathCurrentConfig (
	BRAP_ChannelHandle	hRapCh,		/* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	/* [in] Audio type for which to get
									   Current configuration parameters */
	BRAP_DEC_DownmixPath	eDownmixPath,	/* [in] Downmix path of interest.
											     Default values of output mode 
											     related parameters belongs to
											     this downmix path */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DEC_GetDownmixPathCurrentConfig);
    err = BRAP_DEC_P_GetCurrentConfig (hRapCh, eType, 
        eDownmixPath, psDecConfigParams);
	BDBG_LEAVE (BRAP_DEC_GetDownmixPathCurrentConfig);
	return err;
}
#endif

#ifdef  BCHP_7411_VER 
/***************************************************************************
Summary:
	Reconfigures the I2S SClock and MClock

Description:
	This API is used to re-configure MClock and SClock of I2S ports for switching 
	between overclocked multichannel mode ( channel pair on I2S enabled mode ) to 
	atclocked stereo mode. 	This API assumes that LR channel pair is sent on I2S0 
	and LsRs is sent on I2S1. The SClock and MClock parameters are applied to 
	both the I2S ports.
    
Returns:
	BERR_SUCCESS 

See Also:
****************************************************************************/

BERR_Code
BRAP_DEC_SetI2sClk (
	BRAP_ChannelHandle	hRapCh,			/* [in] RAP channel handle */
	BRAP_OP_SClkRate	eSClkRate,		/* [in] SClk rate */
    	BRAP_OP_MClkRate      eMClkRate )		/* [in] MClk rate */
{
#if BCHP_7411_VER > BCHP_VER_C0
	BERR_Code err = BERR_SUCCESS;
#endif
	
	BDBG_ENTER( BRAP_DEC_SetI2sClk );


#if BCHP_7411_VER==BCHP_VER_C0
	BSTD_UNUSED(hRapCh);
	BSTD_UNUSED(eSClkRate);
	BSTD_UNUSED(eMClkRate);
	BDBG_ERR(("This API is supported only for 7411D0."));
	return BERR_TRACE(BERR_NOT_SUPPORTED);
#else /* 7411 D0 */

	BDBG_ASSERT( hRapCh  );

	/* This function is supported only for switching between multichannel mode
	 * to stereo mode. Return error if this channel is not configured for 
	 * multichannel mode on I2S.
	 */

	if (hRapCh->bMuxChPairOnI2s==false)
	{
		BDBG_ERR(("This function is supported only if channel pair multiplexing"
			"is enabled on I2S ports"));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/* Set MClk and SClk parameters for I2S0, that is used for output LR channel
	 * pair */

	err = BRAP_OP_P_SetI2sClk(
			hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR],
			eSClkRate,
			eMClkRate );
	if (err!=BERR_SUCCESS)
		return BERR_TRACE( err );

	/* Set MClk and SClk parameters for I2S1, that is used for output LsRs channel
	 * pair */

	err = BRAP_OP_P_SetI2sClk(
			hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLRSurround],
			eSClkRate,
			eMClkRate );
	if (err!=BERR_SUCCESS)
		return BERR_TRACE( err );
	
	BDBG_LEAVE( BRAP_DEC_SetI2sClk );
	return err;
#endif	
}
	
#endif

static BERR_Code
BRAP_DEC_P_GetCurrentAudioParams (
	BRAP_ChannelHandle	hRapCh,
	BRAP_DEC_AudioParams	*psAudioParams)
{
	BERR_Code err;
	BRAP_DSPCHN_P_AudioParams	sDspChParams;
	BRAP_MIXER_P_Params			sMixerParams;
	BRAP_SPDIFFM_P_Params		sSpdifFmParams;
	int	i;
#ifndef  BCHP_7411_VER  /* for all chips other than 7411 */     
       int j=0;
#endif

	BDBG_ENTER( BRAP_DEC_P_GetCurrentAudioParams );
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psAudioParams);

	/* Get parameters that are available in channel handle */
#ifdef BCHP_7411_VER  /* only for 7411 */      
	psAudioParams->eOutputMode = hRapCh->eCurOpMode;
	psAudioParams->bCloneEnable = (hRapCh->eClone==BRAP_P_CloneState_eStarted)? true: false;
#endif
	/* Get DSP channel parameters */
	err = BRAP_DSPCHN_P_GetCurrentAudioParams(hRapCh->sModuleHandles.hDspCh, &sDspChParams);
	if (err!=BERR_SUCCESS)
		return err;
	psAudioParams->sDspChParams = sDspChParams.sExtAudioParams;
	psAudioParams->eTimebase = sDspChParams.eTimebase;
	psAudioParams->sSpdifChanStatusParams = sDspChParams.sSpdifChStatusParams;
	psAudioParams->bPlayback = sDspChParams.bPlayback;
#ifdef BCHP_7411_VER  /* only for 7411 */      
	psAudioParams->bMuxChannelPairsOnI2sEnable = sDspChParams.bMultiChanOnI2S;
#endif

	/* Get output blocks parameters for decode context */
	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++) {
		if (hRapCh->sModuleHandles.hMixer[i]!=NULL) {
			err = BRAP_MIXER_P_GetCurrentParams( hRapCh->sModuleHandles.hMixer[i],
									hRapCh->sModuleHandles.uiMixerInputIndex[i],
									&sMixerParams );
			if (err!=BERR_SUCCESS)
				return err;
			psAudioParams->sAudioOutputParams.sMixerParams = sMixerParams.sExtParams;
			psAudioParams->sAudioOutputParams.uiOutputBitsPerSample = sMixerParams.uiStreamRes;
			break;
		}
	}

	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++) {
		if (hRapCh->sModuleHandles.hSpdifFm[i]!=NULL) {
			err = BRAP_SPDIFFM_P_GetCurrentParams( hRapCh->sModuleHandles.hSpdifFm[i],
									&sSpdifFmParams);
			if (err!=BERR_SUCCESS)
				return err;
			psAudioParams->sAudioOutputParams.sSpdifFmParams = sSpdifFmParams.sExtParams;
			break;
		}
#ifndef  BCHP_7411_VER  /* for all chips other than 7411 */             
              /* If SPDIFFM is used by both the main port, and the cloned port, it will have same settings for both.
               But if SPDIFFM is used in a cloned port but not in main port, then it will be found in  instead of sModuleHandles. */
            for (j = 0; j < BRAP_RM_P_MAX_OUTPUTS; j++)
            {    
                if (hRapCh->sCloneOpPathHandles[i][j].hSpdifFm != NULL)
                {                 
                    err = BRAP_SPDIFFM_P_GetCurrentParams(hRapCh->sCloneOpPathHandles[i][j].hSpdifFm,
                    				&sSpdifFmParams);
                    if (err!=BERR_SUCCESS) return err;
                    psAudioParams->sAudioOutputParams.sSpdifFmParams = sSpdifFmParams.sExtParams;
                    break;
                }
            }
#endif            
              
	}

	/* Get output blocks parameters for simul context */
	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++) {
		if (hRapCh->sSimulPtModuleHandles.hMixer[i]!=NULL) {
			err = BRAP_MIXER_P_GetCurrentParams( hRapCh->sSimulPtModuleHandles.hMixer[i],
									hRapCh->sSimulPtModuleHandles.uiMixerInputIndex[i],
									&sMixerParams );
			if (err!=BERR_SUCCESS)
				return err;
			psAudioParams->sSimulPtAudioOutputParams.sMixerParams = sMixerParams.sExtParams;
			psAudioParams->sSimulPtAudioOutputParams.uiOutputBitsPerSample = sMixerParams.uiStreamRes;
			break;
		}
	}

	for (i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++) {
		if (hRapCh->sSimulPtModuleHandles.hSpdifFm[i]!=NULL) {
			err = BRAP_SPDIFFM_P_GetCurrentParams( hRapCh->sSimulPtModuleHandles.hSpdifFm[i],
									&sSpdifFmParams);
			if (err!=BERR_SUCCESS)
				return err;
			psAudioParams->sSimulPtAudioOutputParams.sSpdifFmParams = sSpdifFmParams.sExtParams;
			break;
		}
	}
    
#if BRAP_P_USE_BRAP_TRANS ==0	
    /* XPT Channel number for the current channel. This is used by the 	   
       DSP Firmware to determine the CDB and ITB used for the current 	   
       DSP Context  */	
    psAudioParams->sXptContextMap.ContextIdx = hRapCh->uiXptChannelNo;
#endif	

	BDBG_LEAVE( BRAP_DEC_P_GetCurrentAudioParams );
	return err;
}

#ifdef RAP_SRSTRUVOL_CERTIFICATION  

/**********************************************************************
Summary: Allocates 2 PCM Buffer of size uiSize. These Buffers are used for SRS
Certification.RD/WR/Base/End values are programmed in psBufInfo.
***********************************************************************/
BERR_Code BRAP_AllocatePCMBuffers(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   uiSize,
    unsigned int   ui32Context,
    BRAP_BufInfo    *psBufInfo)
{
	uint32_t physAddress;
	uint32_t ui32Offset;
      BDBG_ASSERT(hRapCh);
      BDBG_ASSERT(psBufInfo);

    BDBG_ENTER(BRAP_AllocatePCMBuffers);

	ui32Offset = ui32Context * (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR);

      BKNI_Memset((void *)psBufInfo,0,sizeof(BRAP_BufInfo));

      psBufInfo->LeftDramBufferBaseAddr = (unsigned int)BRAP_P_AllocAligned( hRapCh->hRap,
																			uiSize*2,
																			8,
																			0
#if (BRAP_SECURE_HEAP==1) 
																			,false
#endif																							
																			);
		if (psBufInfo->LeftDramBufferBaseAddr == BRAP_P_INVALID_DRAM_ADDRESS )
    	{
       		return BERR_TRACE (BERR_OUT_OF_DEVICE_MEMORY);
    	}


        BRAP_Write32(hRapCh->hRegister,BCHP_AUD_DSP_CFG0_SW_UNDEFINED_SPAREi_ARRAY_BASE+44,ui32Context);	
		
        psBufInfo->LeftDramBufferReadAddr = psBufInfo->LeftDramBufferBaseAddr;
        psBufInfo->LeftDramBufferWriteAddr = psBufInfo->LeftDramBufferBaseAddr;
        psBufInfo->LeftDramBufferEndAddr = psBufInfo->LeftDramBufferBaseAddr + uiSize -1;        

        /*Program above Four address to Register*/

		/*Left Base Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->LeftDramBufferBaseAddr, &physAddress);    
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_CDB_BASE_PTR +ui32Offset), physAddress);		
		
        /*Left Read Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->LeftDramBufferReadAddr, &physAddress);  
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR +ui32Offset), physAddress);
		
        /*Left Write Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->LeftDramBufferWriteAddr, &physAddress);        
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR +ui32Offset), physAddress);		
		
        /*Left End Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->LeftDramBufferEndAddr, &physAddress);        
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_CDB_END_PTR +ui32Offset), physAddress);
		



    
        psBufInfo->RightDramBufferBaseAddr = (unsigned int)psBufInfo->LeftDramBufferBaseAddr + uiSize;
        psBufInfo->RightDramBufferReadAddr = psBufInfo->RightDramBufferBaseAddr;
        psBufInfo->RightDramBufferWriteAddr = psBufInfo->RightDramBufferBaseAddr;
        psBufInfo->RightDramBufferEndAddr = psBufInfo->RightDramBufferBaseAddr + uiSize -1;        

		/*Right Base Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->RightDramBufferBaseAddr, &physAddress);  
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_ITB_BASE_PTR +ui32Offset), physAddress);

		/*Right Read Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->RightDramBufferReadAddr, &physAddress);        
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_ITB_READ_PTR +ui32Offset), physAddress);

		/*Right Write Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->RightDramBufferWriteAddr, &physAddress);        
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_ITB_WRITE_PTR +ui32Offset), physAddress);

		/*Right End Addr*/
        BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->RightDramBufferEndAddr, &physAddress);        
		BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_ITB_END_PTR +ui32Offset), physAddress);

        psBufInfo->uiSize = uiSize;

        psBufInfo->bLeftRdWrapAround =false;
        psBufInfo->bLeftWrWrapAround =false;
        psBufInfo->bRightRdWrapAround =false;
        psBufInfo->bRightWrWrapAround =false;        
        /*Program above Four address to Register*/        

    hRapCh->sPcmBufferInfo = *psBufInfo;

    BDBG_LEAVE(BRAP_AllocatePCMBuffers);      
      return BERR_SUCCESS;
}

/**********************************************************************
Summary: Free  PCM Buffer allocated  for SRS
Certification.
***********************************************************************/

BERR_Code BRAP_FreePCMBuffers(
    BRAP_ChannelHandle      hRapCh)
{
    BERR_Code   err=BERR_SUCCESS;
    BDBG_ENTER(BRAP_FreePCMBuffers);    
    BDBG_ASSERT(hRapCh);

    if((hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr !=BRAP_P_INVALID_DRAM_ADDRESS)
        ||(hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr !=0))
    {
#if (BRAP_SECURE_HEAP == 1 )
		err = BRAP_P_Free(hRapCh->hRap,(void *)hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr,false);
#else
		err = BRAP_P_Free(hRapCh->hRap, (void *)hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr);
#endif
	
        if(err == BERR_SUCCESS)
        {
            hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.LeftDramBufferReadAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.LeftDramBufferWriteAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.LeftDramBufferEndAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.RightDramBufferBaseAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.RightDramBufferReadAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.RightDramBufferWriteAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.RightDramBufferEndAddr = BRAP_P_INVALID_DRAM_ADDRESS;
            hRapCh->sPcmBufferInfo.uiSize = 0;  			
        }
    }
    BDBG_LEAVE(BRAP_FreePCMBuffers);    
    return err;
}

/**********************************************************************
Summary: Returns current PCM Buffer Info.
***********************************************************************/

BERR_Code BRAP_GetPCMBufferInfo(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   ui32Context,    
    BRAP_BufInfo    *psBufInfo)
{
    unsigned int ui32Regval;
    void *virtualAddress;
	uint32_t ui32Offset;	

    BDBG_ENTER(BRAP_GetPCMBufferInfo);
        
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psBufInfo);

	ui32Offset = ui32Context * (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR);

    /* Update Read pointers in the buffer info on reading from registers*/
	
	/*Left Read Addr*/
	ui32Regval = BRAP_Read32(hRapCh->hRegister, ui32Offset + BCHP_XPT_RAVE_CX0_AV_CDB_READ_PTR);
	hRapCh->sPcmBufferInfo.bLeftRdWrapAround = (((unsigned int)ui32Regval)>>31);	
	ui32Regval	= ((unsigned int)ui32Regval& 0x7FFFFFFF);
	BRAP_ConvertOffsetToAddress(hRapCh->hHeap, ui32Regval, (void **)&(virtualAddress));
	hRapCh->sPcmBufferInfo.LeftDramBufferReadAddr = ((unsigned int)virtualAddress & 0x7FFFFFFF);

	BDBG_MSG(("GETBufINfo: LeftDramBufferBaseAddr =%x, LeftDramBufferReadAddr =%x , bLeftRdWrapAround =%d",
	hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr,hRapCh->sPcmBufferInfo.LeftDramBufferReadAddr,hRapCh->sPcmBufferInfo.bLeftRdWrapAround));

	/*Right Read Addr*/
	ui32Regval = BRAP_Read32(hRapCh->hRegister, ui32Offset + BCHP_XPT_RAVE_CX0_AV_ITB_READ_PTR);
	hRapCh->sPcmBufferInfo.bRightRdWrapAround  = (((unsigned int)ui32Regval)>>31);	
	ui32Regval	= ((unsigned int)ui32Regval& 0x7FFFFFFF);
	BRAP_ConvertOffsetToAddress(hRapCh->hHeap, ui32Regval, (void **)&(virtualAddress));
	hRapCh->sPcmBufferInfo.RightDramBufferReadAddr = ((unsigned int)virtualAddress & 0x7FFFFFFF);
	
	BDBG_MSG(("GETBufINfo: RightDramBufferBaseAddr =%x RightDramBufferReadAddr =%x , bRightRdWrapAround =%d",
	hRapCh->sPcmBufferInfo.RightDramBufferBaseAddr,hRapCh->sPcmBufferInfo.RightDramBufferReadAddr,hRapCh->sPcmBufferInfo.bRightRdWrapAround));
	
    *psBufInfo = hRapCh->sPcmBufferInfo;


    BDBG_LEAVE(BRAP_GetPCMBufferInfo);
    return BERR_SUCCESS;

}

/**********************************************************************
Summary: Returns current PCM Buffer Info. It returns updated
 Read pointer as used by FW at that instant.
***********************************************************************/

BERR_Code BRAP_UpdatePCMBufferInfo(
    BRAP_ChannelHandle      hRapCh,
    unsigned int   ui32Context,    
    BRAP_BufInfo    *psBufInfo)
{
    uint32_t physAddress,ui32Regval=0;
	uint32_t ui32Offset;

    BDBG_ENTER(BRAP_UpdatePCMBufferInfo);
    
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psBufInfo);

	ui32Offset = ui32Context * (BCHP_XPT_RAVE_CX1_AV_CDB_WRITE_PTR - BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR);

    if((psBufInfo->LeftDramBufferWriteAddr <  hRapCh->sPcmBufferInfo.LeftDramBufferBaseAddr)
        ||(psBufInfo->LeftDramBufferWriteAddr >  hRapCh->sPcmBufferInfo.LeftDramBufferEndAddr)
        ||(psBufInfo->RightDramBufferWriteAddr <  hRapCh->sPcmBufferInfo.RightDramBufferBaseAddr)
        ||(psBufInfo->RightDramBufferWriteAddr >  hRapCh->sPcmBufferInfo.RightDramBufferEndAddr))
    {
        BDBG_ERR(("Write Pointer not within the Range"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Program These 2 Write pointers  to the register*/
	/*Left Write Addr*/
    /*BDBG_MSG(("psBufInfo->LeftDramBufferWriteAddr = %x",psBufInfo->LeftDramBufferWriteAddr));*/
    BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->LeftDramBufferWriteAddr, &physAddress);     
	physAddress = (psBufInfo->bLeftWrWrapAround << 31) | (0x7FFFFFFF & physAddress);
	BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR +ui32Offset), physAddress);
	ui32Regval = BRAP_Read32(hRapCh->hRegister, (ui32Offset + BCHP_XPT_RAVE_CX0_AV_CDB_WRITE_PTR));
    BDBG_MSG(("psBufInfo->LeftDramBufferWriteAddr = %x",ui32Regval));
    
    
	/* BDBG_MSG(("psBufInfo->RightDramBufferWriteAddr = %x",psBufInfo->RightDramBufferWriteAddr));*/
	/*Right Write Addr*/
    BRAP_P_ConvertAddressToOffset(hRapCh->hHeap, (void *)psBufInfo->RightDramBufferWriteAddr, &physAddress);        
	physAddress = (psBufInfo->bRightWrWrapAround << 31) | (0x7FFFFFFF & physAddress);
	BRAP_Write32 (hRapCh->hRegister,(BCHP_XPT_RAVE_CX0_AV_ITB_WRITE_PTR +ui32Offset), physAddress);
	ui32Regval = BRAP_Read32(hRapCh->hRegister, (ui32Offset + BCHP_XPT_RAVE_CX0_AV_ITB_WRITE_PTR));
    BDBG_MSG(("psBufInfo->LeftDramBufferWriteAddr = %x",ui32Regval));    
    
    hRapCh->sPcmBufferInfo.LeftDramBufferWriteAddr = psBufInfo->LeftDramBufferWriteAddr;
    hRapCh->sPcmBufferInfo.RightDramBufferWriteAddr = psBufInfo->RightDramBufferWriteAddr;  
    hRapCh->sPcmBufferInfo.bLeftWrWrapAround = psBufInfo->bLeftWrWrapAround;
    hRapCh->sPcmBufferInfo.bRightWrWrapAround = psBufInfo->bRightWrWrapAround;    
   
    BDBG_LEAVE(BRAP_UpdatePCMBufferInfo);
    return BERR_SUCCESS;

}


BERR_Code BRAP_GetRbufPtr(BRAP_ChannelHandle      hRapCh,
                                                      BRAP_RingBufInfo  *psLeftRbuf,
                                                      BRAP_RingBufInfo  *psRightRbuf)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_GetRbufPtr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psLeftRbuf);
    BDBG_ASSERT(psRightRbuf);    

    /* Pre-fill with invalid values */
    psLeftRbuf->pBasePtr = NULL;
    psLeftRbuf->pReadPtr = NULL;
    psLeftRbuf->pWritePtr = NULL;
    psLeftRbuf->pEndPtr = NULL;    

    psRightRbuf->pBasePtr = NULL;
    psRightRbuf->pReadPtr = NULL;
    psRightRbuf->pWritePtr = NULL;
    psRightRbuf->pEndPtr = NULL;        

    BKNI_EnterCriticalSection();
    ret = BRAP_GetRbufPtr_isr(hRapCh,psLeftRbuf,psRightRbuf);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_GetRbufPtr);
    return ret;
}


BERR_Code BRAP_GetRbufPtr_isr(BRAP_ChannelHandle      hRapCh,
                                                      BRAP_RingBufInfo  *psLeftRbuf,
                                                      BRAP_RingBufInfo  *psRightRbuf)
{
        unsigned int i=0;

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psLeftRbuf);
    BDBG_ASSERT(psRightRbuf);            

        for(i=0 ;i <BRAP_RM_P_MAX_OP_CHANNELS;i++)
        {
            if(hRapCh->sModuleHandles.hRBuf[i] != NULL)
            {
                BDBG_MSG(("hRapCh->sModuleHandles.hRBuf[%d].uiIndex =%d , oofset =%d",i,hRapCh->sModuleHandles.hRBuf[i]->uiIndex,hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psLeftRbuf->pBasePtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psLeftRbuf->pReadPtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psLeftRbuf->pWritePtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psLeftRbuf->pEndPtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));

                BDBG_MSG(("psLeftRbuf->pBasePtr =%#x",psLeftRbuf->pBasePtr));
                BDBG_MSG(("psLeftRbuf->pReadPtr =%#x",psLeftRbuf->pReadPtr));
                BDBG_MSG(("psLeftRbuf->pWritePtr =%#x",psLeftRbuf->pWritePtr));
                BDBG_MSG(("psLeftRbuf->pEndPtr =%#x",psLeftRbuf->pEndPtr));                
                
                i++;

            if(hRapCh->sModuleHandles.hRBuf[i] != NULL)
            {
                BDBG_MSG(("hRapCh->sModuleHandles.hRBuf[%d].uiIndex =%d , offset =%d",i,hRapCh->sModuleHandles.hRBuf[i]->uiIndex,hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psRightRbuf->pBasePtr =(void *) BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_BASEADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psRightRbuf->pReadPtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psRightRbuf->pWritePtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_WRADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                psRightRbuf->pEndPtr = (void *)BRAP_Read32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_ENDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                BDBG_MSG(("psRightRbuf->pBasePtr =%#x",psRightRbuf->pBasePtr));
                BDBG_MSG(("psRightRbuf->pReadPtr =%#x",psRightRbuf->pReadPtr));
                BDBG_MSG(("psRightRbuf->pWritePtr =%#x",psRightRbuf->pWritePtr));
                BDBG_MSG(("psRightRbuf->pEndPtr =%#x",psRightRbuf->pEndPtr));                   
            }                
                
            }
        }
        return BERR_SUCCESS;
}
BERR_Code BRAP_UpdateRbufReadPtr(BRAP_ChannelHandle      hRapCh,
                                                      BRAP_RingBufInfo  *psLeftRbuf,
                                                      BRAP_RingBufInfo  *psRightRbuf)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_UpdateRbufReadPtr);

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psLeftRbuf);
    BDBG_ASSERT(psRightRbuf);           

    BKNI_EnterCriticalSection();
    ret = BRAP_UpdateRbufReadPtr_isr(hRapCh,psLeftRbuf,psRightRbuf);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_UpdateRbufReadPtr);
    return ret;
}
BERR_Code BRAP_UpdateRbufReadPtr_isr(BRAP_ChannelHandle      hRapCh,
                                                      BRAP_RingBufInfo  *psLeftRbuf,
                                                      BRAP_RingBufInfo  *psRightRbuf)
{
        unsigned int i=0;

    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(psLeftRbuf);
    BDBG_ASSERT(psRightRbuf);            

        for(i=0 ;i <BRAP_RM_P_MAX_OP_CHANNELS;i++)
        {
            if(hRapCh->sModuleHandles.hRBuf[i] != NULL)
            {
                BDBG_MSG(("hRapCh->sModuleHandles.hRBuf[%d].uiIndex =%d , oofset =%d",i,hRapCh->sModuleHandles.hRBuf[i]->uiIndex,hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));
                BRAP_Write32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset),(uint32_t)psLeftRbuf->pReadPtr);
                
                BDBG_MSG(("psLeftRbuf->pBasePtr =%#x",psLeftRbuf->pBasePtr));
                BDBG_MSG(("psLeftRbuf->pReadPtr =%#x",psLeftRbuf->pReadPtr));
                BDBG_MSG(("psLeftRbuf->pWritePtr =%#x",psLeftRbuf->pWritePtr));
                BDBG_MSG(("psLeftRbuf->pEndPtr =%#x",psLeftRbuf->pEndPtr));                
                
                i++;

            if(hRapCh->sModuleHandles.hRBuf[i] != NULL)
            {
                BDBG_MSG(("hRapCh->sModuleHandles.hRBuf[%d].uiIndex =%d , offset =%d",i,hRapCh->sModuleHandles.hRBuf[i]->uiIndex,hRapCh->sModuleHandles.hRBuf[i]->ui32Offset));

                BRAP_Write32_isr(hRapCh->hRegister, (BCHP_AUD_FMM_BF_CTRL_RINGBUF_0_RDADDR + hRapCh->sModuleHandles.hRBuf[i]->ui32Offset),(uint32_t)psRightRbuf->pReadPtr);

                BDBG_MSG(("psRightRbuf->pBasePtr =%#x",psRightRbuf->pBasePtr));
                BDBG_MSG(("psRightRbuf->pReadPtr =%#x",psRightRbuf->pReadPtr));
                BDBG_MSG(("psRightRbuf->pWritePtr =%#x",psRightRbuf->pWritePtr));
                BDBG_MSG(("psRightRbuf->pEndPtr =%#x",psRightRbuf->pEndPtr));                   
            }                                
        }
    }
    return BERR_SUCCESS;
}


#endif



#if 0
BERR_Code BRAP_DEC_EnableClone ( 
    BRAP_ChannelHandle   hRapCh      /*[in]The RAP Decode Channel handle */

)
{
    BDBG_ENTER(BRAP_DEC_EnableClone);
    /* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    if (hRapCh->eClone != BRAP_P_CloneState_eInvalid)
    {
        hRapCh->eClone = BRAP_P_CloneState_eEnabled;
    }
    else 
    {
        BDBG_ERR (("BRAP_DEC_EnableClone: Cloning was not configured during ChannelOpen."));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_LEAVE(BRAP_DEC_EnableClone);
	return BERR_SUCCESS;


#if 0    
    BRAP_RM_P_ResrcReq		sResrcReq;
	BRAP_RBUF_P_Settings	sRbufSettings;
	BRAP_SRCCH_P_Settings	sSrcChSettings;
	BRAP_MIXER_P_Settings	sMixerSettings;
	BRAP_SPDIFFM_P_Settings	sSpdifFmSettings;
    BRAP_OP_P_SpdifSettings	sSpdifSettings;    
	
    BRAP_RBUF_P_Params		sRBufParams;
	BRAP_SRCCH_P_Params		sSrcChParams;
	BRAP_MIXER_P_Params		sMixerParams;
	BRAP_SPDIFFM_P_Params	sSpdifFmParams;
	BRAP_OP_P_SpdifParams	sSpdifParams;

/* TODO: Make sure the FS and timebase for both Output ports match, else return error*/	

/*Ask resource manager to allocate resources related to the new output port . While requesting resources, set bCloneOutput so that RM knows that it doesnt have to allocate DSP and has to use the above mentioned algo for the other resources. */

    if (hRapCh->bSimulModeConfig != true)
    {
        /* Resources for the spdif port have not been allocated. Allocate and open them */

    	/* Form the resource requirement */
	    for(i = 0; i < BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
    	{
	    	if((opAudModeProp[pChnSettings->eOutputMode].bChnExists[i * 2] == false) && 
		       (opAudModeProp[pChnSettings->eOutputMode].bChnExists[i * 2 + 1] == false))
		    {
			
			    sResrcReq.sOpPortReq[i].eOpPortType = BRAP_RM_P_INVALID_INDEX;
			
    		}
	    }

		sResrcReq.eChannelType = BRAP_P_ChannelType_eDecode;
		sResrcReq.uiNumOpPorts = 1;	/* Only one port required */
		sResrcReq.bSimulModePt = false; 
		sResrcReq.sOpPortReq[0].eOpPortType = eOutput;
		sResrcReq.bLastChMono = false;
		sResrcReq.bCloneOutput = true; 

    	/* Call resource manager to allocate required resources. */
	    ret = BRAP_RM_P_AllocateResources (	hRapCh->hRap->hRm, 
										&sResrcReq, 
										&(hRapCh->sSimulPtRsrcGrnt));
    	if(ret != BERR_SUCCESS){goto error;}
		
        /* Call BRAP_P_OpenOpPathFmmModules() to open all newly allocated resources i.e. cloned and save their handles in hRapCh->sModuleHandles  . If duplication is at RBUF->Srch level, will need to introduce a special flag for the SrcCh to set the SHARED_ fields in the RDB registers correctly. No such flag is reqd for duplication at SrcCh->Mixer level*/
 
    	/* Mixer settings */
	    sMixerSettings.uiSrcChId = hRapCh->sRsrcGrnt.sOpResrcId[0].uiSrcChId;
	    sMixerSettings.uiMixerInput = hRapCh->sSimulPtRsrcGrnt.sOpResrcId[0].uiMixerInputId;

        /* Spdif formater settings: Currently it is blank. */
	
        /* Output settings */
    	sSpdifSettings.sExtSettings = 
					hRap->sOutputSettings[eOutput].uOutputPortSettings.sSpdifSettings;

		/* Open all newly allocated internal modules */
		
        hRapCh->sSimulPtModuleHandles.hDsp = hRapCh->sModuleHandles.hDsp; /* for completeness */
    	hRapCh->sSimulPtModuleHandles.hFmm = hRapCh->sModuleHandles.hFmm; /* for completeness */
		
	    ret = BRAP_P_OpenOpPathFmmModules(
									hRapCh,
									&sRbufSettings,
									&sSrcChSettings,
									&sMixerSettings,
									&sSpdifFmSettings,
									&sSpdifSettings,
									&(hRapCh->sSimulPtRsrcGrnt.sOpResrcId[0]),
									0,
									&(hRapCh->sSimulPtModuleHandles));
    	if(ret != BERR_SUCCESS){goto free_slave_rs;}

	    hRapCh->sSimulPtModuleHandles.hDspCh = NULL;   
    } 

#endif   

}

BERR_Code BRAP_DEC_DisableClone ( 
    BRAP_ChannelHandle   hRapCh      /*[in]The RAP Decode Channel handle */

)
{
    BDBG_ENTER(BRAP_DEC_DisableClone);
    /* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    if (hRapCh->eClone != BRAP_P_CloneState_eInvalid)
    {
        hRapCh->eClone = BRAP_P_CloneState_eDisable;
    }
    else 
    {
        BDBG_ERR (("BRAP_DEC_EnableClone: Cloning was not configured during ChannelOpen."));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BDBG_LEAVE(BRAP_DEC_DisableClone);
	return BERR_SUCCESS;
}


#endif
/* End of File */

