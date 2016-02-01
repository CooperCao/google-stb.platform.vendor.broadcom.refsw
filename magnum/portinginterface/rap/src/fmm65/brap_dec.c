/***************************************************************************
*     Copyright (c) 2006-2012, Broadcom Corporation
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
**************************************************************************/
#include "brap.h"
#include "brap_priv.h"	

BDBG_MODULE(rap_dec);		/* Register software module with debug interface */

extern const BRAP_P_DecoderSettings  BRAP_sDefaultDecSettings;

static uint32_t ui32OutputChannelMatrix[BRAP_OutputMode_eLast][BRAP_AF_P_MAX_CHANNELS] = 
    {/* [TODO] Change the outputchannelmatrix as per FW requirement, Currentlt e1_0,e1_1,e2_0,e3_2 is done*/
        {4,4,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e1_0*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e1_1*/
        {4,4,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_eTrueMono*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},           /*BRAP_OutputMode_e2_0*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e3_0*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e2_1*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e3_1*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e2_2*/
        {0,1,2,3,4,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e3_2*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e3_3*/
        {0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF},  /*BRAP_OutputMode_e3_4*/        
};

/**************************************************************************
Summary:
    Set the Dolby MS System Usage Mode
**************************************************************************/
static void BRAP_P_SetDolbyMsUsageMode(
    BRAP_ChannelHandle          hRapCh,     /* [in] Channel Handle */
    BRAP_AF_P_DolbyMsUsageMode  *peDolbyMsUsageMode
);

/**************************************************************************
Summary:
    Set the Dolby MS Decoder Type
**************************************************************************/
static void BRAP_P_SetDolbyMsDecoderType(
    BRAP_ChannelHandle              hRapCh,     /* [in] Channel Handle */
    BRAP_AF_P_DolbyMsUsageMode	    eDolbyMsUsageMode, /* [in] Dolby MS Usage Mode */    
    BRAP_AF_P_DolbyMsDecoderType    *peDolbyMsDecoderType
);

/**************************************************************************
Summary:
    Set the Dolby MS Decoder Output Mode and LFE
**************************************************************************/
static void BRAP_P_SetDolbyMsDecoderOutputCfg(
    BRAP_ChannelHandle          hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType       eDecodeAlgo, /* [in] Decode Audio Algorithm Type */    
    BRAP_AF_P_DolbyMsUsageMode  eDolbyMsUsageMode /* [in] Dolby MS Usage Mode */
);

BERR_Code BRAP_P_MapAc3UserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapMs11DdpUserConfigApptoFw(
                    BRAP_ChannelHandle  hRapCh,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapMs11DdpUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapMs11Ac3UserConfigApptoFw(
                    BRAP_ChannelHandle  hRapCh,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapMs11Ac3UserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapLpcmDvdUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapLpcmDvdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);
BERR_Code BRAP_P_MapMpegUserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);
BERR_Code BRAP_P_MapAc3UserConfigFwtoApp(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);
BERR_Code BRAP_P_MapMpegUserConfigFwtoApp(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapAc3PlusUserConfigFwtoApp(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapAc3PlusUserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);
BERR_Code BRAP_P_MapAacheUserConfigFwtoApp(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapAacheUserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);


BERR_Code BRAP_P_MapDolbyPulseUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);


BERR_Code BRAP_P_MapDolbyPulseUserConfigApptoFw(
                    BRAP_ChannelHandle  hRapCh,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);


BERR_Code BRAP_P_MapWmaStdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);
BERR_Code BRAP_P_MapWmaStdUserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);
BERR_Code BRAP_P_MapWmaProUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);
BERR_Code BRAP_P_MapWmaProUserConfigApptoFw(
    BRAP_P_DecoderSettings          *psDecSettings,
    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapDtsBroadcastUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapDtsBroadcastUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapDtsHdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);


BERR_Code BRAP_P_MapDtsHdUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);


BERR_Code BRAP_P_MapPcmWavUserConfigApptoFw(
                    BRAP_ChannelHandle          hRapCh,       
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapPcmWavUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapAmrUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapDraUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapDraUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);

BERR_Code BRAP_P_MapRealAudioLbrUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

BERR_Code BRAP_P_MapRealAudioLbrUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
);



BERR_Code BRAP_P_MapAmrUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
);

static uint32_t BRAP_P_FloatToQ131(int32_t floatVal, unsigned int uiRange);
static uint32_t BRAP_P_FloatToQ230(int16_t floatVar);
static int32_t BRAP_P_FloatToQ923(uint32_t floatVar, unsigned int uiRange);
static int32_t BRAP_P_FloatToQ824(int32_t floatVar, unsigned int uiRange);
static uint32_t BRAP_P_ConvertPIToFwFormat(uint32_t PI_Value, unsigned int uiScaleFactor, 
                                  unsigned int uiMaxPIValue, uint32_t ui32FWFormat);
static int32_t BRAP_P_FloatToQ1022(int32_t floatVar, unsigned int uiRange);
#if 0/*Enable when used*/
static uint32_t BRAP_P_FloatToQ518(uint32_t floatVar, unsigned int uiRange);
static uint32_t BRAP_P_FloatToQ815(uint32_t floatVar, unsigned int uiRange);
#endif
static uint32_t BRAP_P_FloatToQ527(uint32_t floatVar, unsigned int uiRange);
static uint32_t BRAP_P_FloatToQ329(uint32_t floatVal, unsigned int uiRange);
static uint32_t BRAP_P_FloatToQ428(uint32_t floatVal, unsigned int uiRange);


static void BRAP_DSPCHN_P_GetOutputModeApptoFw(BRAP_OutputMode eOutputMode,unsigned int *uiOutputMode);
static void BRAP_DSPCHN_P_GetOutputModeFwtoApp(unsigned int uiOutputMode , BRAP_OutputMode *eOutputMode);
static void BRAP_DSPCHN_P_GetDualMonoModeApptoFw(BRAP_DSPCHN_DualMonoMode eDualMonoMode,unsigned int *uiDualMonoMode);
static void BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(unsigned int uiDualMonoMode,BRAP_DSPCHN_DualMonoMode *eDualMonoMode);
static void BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode eOutputMode,uint32_t *pui32outputChannel,bool bLfe);

/***************************************************************************
Summary:
	Private API used to open a decode channel.
Description:
	It is used to instantiate a decode channel. It allocates channel handle 
	and resource required for the channel if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelClose
****************************************************************************/
BERR_Code BRAP_DEC_P_ChannelOpen( 
    BRAP_Handle                     hRap,		    /* [in] Raptor Audio Device 
                                                       handle*/
    BRAP_ChannelHandle              hRapCh,		    /* [in] Raptor Decode 
                                                       Channel handle */
    const BRAP_ChannelSettings      *pChnSettings   /* [in] Channel settings*/ 
	)
{
    BERR_Code               err = BERR_SUCCESS;
    unsigned int            uiChannelNo = 0;
    bool                    bWdgRecovery = BRAP_P_GetWatchdogRecoveryFlag(hRap);
    
	/* Validate input params */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(hRapCh);
    BDBG_ENTER(BRAP_DEC_P_ChannelOpen);
    
	if(false == bWdgRecovery)
	{
        /* pChnSettings is valid only when not in watchdog recovery */
        BDBG_ASSERT(pChnSettings);
        BDBG_MSG(("BRAP_DEC_P_ChannelOpen():"
                  "hRap=0x%x," 
                  "\n\t pChnSettings->eChType = %d" 
                  "\n\t pChnSettings->eChSubType = %d",
                  hRap, pChnSettings->eChType, pChnSettings->eChSubType));

        /* Validate if this channel can be added to hRap */
        for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_DEC_CHANNELS; uiChannelNo++)
        {
        	if(!(BRAP_P_IsPointerValid((void *)hRap->hRapDecCh[uiChannelNo])))
        	{
        		break;
        	}
        }

        if(BRAP_RM_P_MAX_DEC_CHANNELS == uiChannelNo)
        {
        	BDBG_ERR(("Max number of DEC channels(%d) already exist", uiChannelNo));
        	err = BERR_TRACE(BERR_NOT_SUPPORTED);
        	goto end_open;
        }
        hRapCh->uiChannelNo = uiChannelNo;

    }    
    BSTD_UNUSED(pChnSettings);

	/* TODO: For the simulMode, just get the DSP context(2) for the same DSP 
	   using the RM. */

	/* Intialise Raptor interrupt handling */
	err = BRAP_P_InterruptInstall (hRapCh);
	if(err != BERR_SUCCESS)
	{
		err = BERR_TRACE(err);
		BDBG_ERR(("InstallInterrupt()failed for RAP DEC Channel handle 0x%x", 
                hRapCh));
		if(true == bWdgRecovery)
		{
    		goto error;
    	}
		else
		{
    		goto free_rs;
    	}
	}

    if(false == bWdgRecovery)
    {
    /* Initialize the default values for user configuration parameters */
    hRapCh->sDecSettings = BRAP_sDefaultDecSettings;
    }

   	goto end_open;

free_rs:
	/* TODO: check all exit conditions and resource free/close */
error:
    /* TODO: */
end_open:

	
   /* only if channel has been successfully opened, save the handle */
	if((BERR_SUCCESS == err) && (false == bWdgRecovery))
	{
    	hRapCh->eState = BRAP_P_State_eOpened; /* Opened successfully */
    	hRap->hRapDecCh[uiChannelNo] = hRapCh ;    
	}

	BDBG_LEAVE(BRAP_DEC_P_ChannelOpen);
	return err;
}


/***************************************************************************
Summary:
	API used to close a decode channel.
Description:
	It closes the instance of a decode channel operation. It frees the 
	channel handle and resources associated with it if any.
Returns:
	BERR_SUCCESS on success else error
See Also:
	BRAP_DEC_P_ChannelOpen
****************************************************************************/
BERR_Code BRAP_DEC_P_ChannelClose( 
	BRAP_ChannelHandle 	hRapCh	/* [in] The RAP Channel handle */
	)
{
    BERR_Code	    ret = BERR_SUCCESS;
    unsigned int    uiGrpId = 0;
    int             i = 0;
    unsigned int    uiChannelNo = 0;
    BRAP_Handle 	hRap = NULL;

	BDBG_ENTER(BRAP_DEC_P_ChannelClose);

	/* Validate input parameters. */
	BDBG_ASSERT(hRapCh);

    hRap = hRapCh->hRap;

	/* Mask interrupts and uninstall callbacks */
	BRAP_P_InterruptUnInstall(hRapCh);


	/* We have associtated channel pairs and we need to check each of them to 
	   see if the channel exits there. If so we need to remove the channel from 
	   there */

	/* Find the group */
	for(uiGrpId=0; uiGrpId < BRAP_MAX_ASSOCIATED_GROUPS; uiGrpId++)
	{
        if((BRAP_ChannelSubType_ePrimary == hRapCh->eChannelSubType)||
		(BRAP_ChannelSubType_eNone == hRapCh->eChannelSubType))
        {
        	for(i=0; i<BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP; i++)
    		{   
    			if(hRap->sAssociatedCh[uiGrpId].hPriDecCh[i] == hRapCh)
    			{
    				hRap->sAssociatedCh[uiGrpId].hPriDecCh[i] = NULL;
    				break;
    			}
    		}
            if(i != BRAP_MAX_PRI_DEC_CHAN_IN_ASSOCIATED_GRP)
    		break;
        }
        else if(BRAP_ChannelSubType_eSecondary == hRapCh->eChannelSubType)
        {
        	for(i=0; i<BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP; i++)
    		{   
    			if(hRap->sAssociatedCh[uiGrpId].hSecDecCh[i] == hRapCh)
    			{
    				hRap->sAssociatedCh[uiGrpId].hSecDecCh[i] = NULL;
    				break;
    			}
    		}
            if(i != BRAP_MAX_SEC_DEC_CHAN_IN_ASSOCIATED_GRP)
    		break;
        }
        else
            BDBG_ASSERT(0); /* TODO */
	}


    /* Mark the place for the current channel handle to 'invalid' inside RAP 
    handle */
    for(uiChannelNo=0; uiChannelNo<BRAP_RM_P_MAX_DEC_CHANNELS; uiChannelNo++)
    {
        if(hRapCh == hRap->hRapDecCh[uiChannelNo])
        {
            break;
        }
    }	
    hRapCh->hRap->hRapDecCh[hRapCh->uiChannelNo] = NULL;

	/* Free the channel handle */
	/* BKNI_Free(hRapCh);*/

	BDBG_LEAVE(BRAP_DEC_P_ChannelClose);
	return(ret);
}

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_SetConfig (
	BRAP_ChannelHandle		hRapCh,		        /* [in] RAP channel handle */
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */
	)
{
    BERR_Code               err = BERR_SUCCESS;
    BRAP_DEC_DownmixPath	eDownmixPath = BRAP_DEC_DownmixPath_eMain;
    BRAP_DSPCHN_Handle      hDspCh = NULL;
    unsigned int            uiPth = 0;

    BDBG_ENTER (BRAP_DEC_P_SetConfig);
	
    BDBG_ASSERT (hRapCh);
    BDBG_ASSERT (psDecConfigParams);
    BSTD_UNUSED(eDownmixPath);

    if((BRAP_P_State_eStarted == hRapCh->eState)
        &&(hRapCh->eChannelOutputMode != psDecConfigParams->eOutputMode))
    {
        if(!((hRapCh->eChannelOutputMode <= BRAP_OutputMode_e2_0)
            &&(psDecConfigParams->eOutputMode <= BRAP_OutputMode_e2_0)))
        {
            BDBG_ERR(("If Required for a OutMode doesn't match with existing Buffer allocated, Output Mode can't be changed on the Fly."));
            return  BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    hRapCh->eChannelOutputMode = psDecConfigParams->eOutputMode;
    switch(psDecConfigParams->eType)
    {
        case BRAP_DSPCHN_AudioType_eMpeg:
		BRAP_P_MapMpegUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
            break;
        case BRAP_DSPCHN_AudioType_eAac:
        case BRAP_DSPCHN_AudioType_eAacLoas:                        
        case BRAP_DSPCHN_AudioType_eAacSbr:
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT                       
            BRAP_P_MapDolbyPulseUserConfigApptoFw(hRapCh,psDecConfigParams);
#else            
		    BRAP_P_MapAacheUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
#endif
            break;
        case BRAP_DSPCHN_AudioType_eAc3:
            BRAP_P_MapMs11Ac3UserConfigApptoFw(hRapCh,psDecConfigParams);
            break;
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            BRAP_P_MapMs11DdpUserConfigApptoFw(hRapCh,psDecConfigParams);
            break;
        case BRAP_DSPCHN_AudioType_eDts:
            break;
        case BRAP_DSPCHN_AudioType_eLpcmBd:
            break;
        case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
            break;
        case BRAP_DSPCHN_AudioType_eDtshd:
            BRAP_P_MapDtsHdUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);            
            break;
        case BRAP_DSPCHN_AudioType_eLpcmDvd:
            BRAP_P_MapLpcmDvdUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);            
            break;
        case BRAP_DSPCHN_AudioType_eWmaStd:
	    BRAP_P_MapWmaStdUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
           if(hRapCh->eState == BRAP_P_State_eStarted)
            {
                if(hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eWMAIpType
                    != (BRAP_FWIF_P_WMAIpType)psDecConfigParams->uConfigParams.sWmaStdConfigParams.eWmaIpType)
                {
                    BDBG_ERR(("WmaIpType Can not be changed on the fly at run time, Please Stop the channel then change it"));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }

                if(hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eAsfPtsType
                    != (BRAP_FWIF_P_ASFPTSType)psDecConfigParams->uConfigParams.sWmaStdConfigParams.eAsfPtsType)
                {
                    BDBG_ERR(("eAsfPtsType Can not be changed on the fly at run time, Please Stop the channel then change it"));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }                
            }
            else
            {
                hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eWMAIpType = psDecConfigParams->uConfigParams.sWmaStdConfigParams.eWmaIpType;
                hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eAsfPtsType = psDecConfigParams->uConfigParams.sWmaStdConfigParams.eAsfPtsType;                
            }
            break;
        case BRAP_DSPCHN_AudioType_eAc3Lossless:
            break;
        case BRAP_DSPCHN_AudioType_eMlp:
            break;
        case BRAP_DSPCHN_AudioType_ePcm:
            break;
        case BRAP_DSPCHN_AudioType_eDtsLbr:
            break;
        case BRAP_DSPCHN_AudioType_eDdp7_1:
            break;
        case BRAP_DSPCHN_AudioType_eMpegMc:
            break;
        case BRAP_DSPCHN_AudioType_eWmaPro:
	    BRAP_P_MapWmaProUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);  
           if(hRapCh->eState == BRAP_P_State_eStarted)
            {
                if(hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eAsfPtsType
                    != (BRAP_FWIF_P_ASFPTSType)psDecConfigParams->uConfigParams.sWmaProConfigParams.eAsfPtsType)
                {
                    BDBG_ERR(("eAsfPtsType Can not be changed on the fly at run time, Please Stop the channel then change it"));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }

            }
            else
            {
                hRapCh->sDecSettings.sUserConfigStruct.sFrameSyncConfigParams.sAlgoSpecConfigStruct.eAsfPtsType = psDecConfigParams->uConfigParams.sWmaProConfigParams.eAsfPtsType;
            }        
            break;    
        case BRAP_DSPCHN_AudioType_eDtsBroadcast:
            BRAP_P_MapDtsBroadcastUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);            
            break;    
        case BRAP_DSPCHN_AudioType_ePcmWav:
            BRAP_P_MapPcmWavUserConfigApptoFw(hRapCh,psDecConfigParams);            
            break;               
        case BRAP_DSPCHN_AudioType_eAmr:
            BRAP_P_MapAmrUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
            break;
        case BRAP_DSPCHN_AudioType_eDra:
            BRAP_P_MapDraUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
            break;            
        case BRAP_DSPCHN_AudioType_eRealAudioLbr:
            BRAP_P_MapRealAudioLbrUserConfigApptoFw(&(hRapCh->sDecSettings),psDecConfigParams);
            break;             
        default:
            break;
    }

    if(BRAP_P_State_eStarted != hRapCh->eState)
    {
        /* Store output mode in RAP channel handle */
        hRapCh->eInputAudMode = psDecConfigParams->eOutputMode;
        hRapCh->bInputLfeOn = psDecConfigParams->bOutputLfeOn;
    }
            
    if(hRapCh->eState == BRAP_P_State_eStarted)
    {
        /* Find a decPath index */
        for(uiPth = 0; uiPth < BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
        {
            if ((BRAP_P_IsPointerValid((void *)hRapCh->pPath[uiPth])) &&
                    ((BRAP_P_UsgPath_eDecodePcm==hRapCh->pPath[uiPth]->eUsgPath) ||
                    (BRAP_P_UsgPath_eDecodeCompress==hRapCh->pPath[uiPth]->eUsgPath)))
            {
                hDspCh = hRapCh->pPath[uiPth]->hDspCh;
                break;
            }
        }
        if(!(BRAP_P_IsPointerValid((void *)hDspCh)))
        {
            BDBG_ERR(("hDspCh not found"));
    		return BERR_TRACE(BERR_NOT_INITIALIZED);
        }

        if(psDecConfigParams->eType == hDspCh->sDspAudioParams.sExtAudioParams.eType)
        {
        	err = BRAP_DSPCHN_P_SetConfig(hDspCh, 
                                		  BRAP_DEC_DownmixPath_eMain,
                                		  psDecConfigParams->eType);
            
        	if (err != BERR_SUCCESS)
        		return BERR_TRACE (err);
        }
    }

    BDBG_LEAVE (BRAP_DEC_P_SetConfig);
    return err;
}

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_GetDefaultConfig (
	BRAP_ChannelHandle	    hRapCh,		        /* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	            /* [in] Audio type for which to 
	                                               get default configuration 
	                                               parameters */
	BRAP_ProcessingType	ePpAlgo,	/* [in] Post processing type for
										which to get default configuration
										parameter. */
	BRAP_DEC_ConfigParams	*psDecConfigParams  /* [out] Decoder configuration
												   parameters */
	)
{
    BERR_Code err = BERR_SUCCESS;
    /* Adding code to bypass compiler warnings */
    BRAP_P_DecoderSettings *pSettings;
   
	BDBG_ENTER (BRAP_DEC_P_GetDefaultConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psDecConfigParams);

    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(ePpAlgo);
      	psDecConfigParams->eType = eType;
	psDecConfigParams->bMonoToAll = false;        
    /* Adding code to bypass compiler warnings */
    pSettings = (BRAP_P_DecoderSettings *)BKNI_Malloc(sizeof(BRAP_P_DecoderSettings));
    if ( NULL==pSettings)
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }   
    *pSettings = BRAP_sDefaultDecSettings;
    switch (eType) {
    		case BRAP_DSPCHN_AudioType_eMpeg:
/*                 BRAP_P_MapMpegUserConfigFwtoApp((BRAP_P_DecoderSettings *)&(sDefaultDecSettings),psDecConfigParams);              */
                 BRAP_P_MapMpegUserConfigFwtoApp(pSettings,psDecConfigParams);              
    			break;
        case BRAP_DSPCHN_AudioType_eAac:
        case BRAP_DSPCHN_AudioType_eAacLoas:                        
        case BRAP_DSPCHN_AudioType_eAacSbr:
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT                
                BRAP_P_MapDolbyPulseUserConfigFwtoApp(pSettings,psDecConfigParams);
#else
/*                BRAP_P_MapAacheUserConfigFwtoApp((BRAP_P_DecoderSettings *)&(sDefaultDecSettings),psDecConfigParams);*/
                BRAP_P_MapAacheUserConfigFwtoApp(pSettings,psDecConfigParams);
#endif
    			break;
    		case BRAP_DSPCHN_AudioType_eAc3:
            BRAP_P_MapMs11Ac3UserConfigFwtoApp(pSettings,psDecConfigParams);
            break;
        case BRAP_DSPCHN_AudioType_eAc3Lossless:
            break;
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            BRAP_P_MapMs11DdpUserConfigFwtoApp(pSettings,psDecConfigParams);
    			break;
    		case BRAP_DSPCHN_AudioType_eWmaStd:
/*                BRAP_P_MapWmaStdUserConfigFwtoApp((BRAP_P_DecoderSettings *)&(sDefaultDecSettings),psDecConfigParams);*/
                BRAP_P_MapWmaStdUserConfigFwtoApp(pSettings,psDecConfigParams);
    			break;
    		case BRAP_DSPCHN_AudioType_eDtsBroadcast:
                BRAP_P_MapDtsBroadcastUserConfigFwtoApp(pSettings,psDecConfigParams);
    			break;                
    		case BRAP_DSPCHN_AudioType_eDts:
    			break;
    		case BRAP_DSPCHN_AudioType_eLpcmBd:
    			break;
    		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
    			break; 
    		case BRAP_DSPCHN_AudioType_eDtshd:
                BRAP_P_MapDtsHdUserConfigFwtoApp(pSettings,psDecConfigParams);
    			break;                
              
              case BRAP_DSPCHN_AudioType_eDtshdSub:
    			break;           
    		case BRAP_DSPCHN_AudioType_eLpcmDvd:
                    BRAP_P_MapLpcmDvdUserConfigFwtoApp(pSettings,psDecConfigParams);              
    			break; 
    		case BRAP_DSPCHN_AudioType_eMlp:
    			break;
    		case BRAP_DSPCHN_AudioType_eDtsLbr:
    			break;            
    		case BRAP_DSPCHN_AudioType_eDdp7_1:
    			break;	
    		case BRAP_DSPCHN_AudioType_eMpegMc:
    			break;						
    		case BRAP_DSPCHN_AudioType_eWmaPro:
/*                BRAP_P_MapWmaProUserConfigFwtoApp((BRAP_P_DecoderSettings *)&(sDefaultDecSettings),psDecConfigParams);              */
                BRAP_P_MapWmaProUserConfigFwtoApp(pSettings,psDecConfigParams);              
                break;
    		case BRAP_DSPCHN_AudioType_ePcm:
    			break;
    		case BRAP_DSPCHN_AudioType_ePcmWav:
                BRAP_P_MapPcmWavUserConfigFwtoApp(pSettings,psDecConfigParams);                    
                break;                
    		case BRAP_DSPCHN_AudioType_eAmr:
                BRAP_P_MapAmrUserConfigFwtoApp(pSettings,psDecConfigParams);                    
                break;
    		case BRAP_DSPCHN_AudioType_eDra:
                BRAP_P_MapDraUserConfigFwtoApp(pSettings,psDecConfigParams);                    
                break;                                                    
    		case BRAP_DSPCHN_AudioType_eRealAudioLbr:
                BRAP_P_MapRealAudioLbrUserConfigFwtoApp(pSettings,psDecConfigParams);                    
                break;            
                    
   		default:
    			BDBG_ERR(("Audio Type %d not supported",eType));
                     BKNI_Free(pSettings);
    			return BERR_TRACE(BERR_INVALID_PARAMETER);
    	}
    
       BKNI_Free(pSettings);
	BDBG_LEAVE (BRAP_DEC_P_GetDefaultConfig);
	return err;
}

/* 
TODO: Write summary
*/
BERR_Code BRAP_DEC_P_GetCurrentConfig (
	BRAP_ChannelHandle	    hRapCh,		        /* [in] RAP channel handle */
	BRAP_DSPCHN_AudioType	eType,	            /* [in] Audio type for which to 
	                                               get current configuration 
	                                               parameters */
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
	BRAP_DEC_ConfigParams	*psDecConfigParams	/* [out] Decoder configuration
												   parameters */
	)
{
	BERR_Code               err = BERR_SUCCESS;
     
	BDBG_ENTER (BRAP_DEC_P_GetCurrentConfig);
	
	BDBG_ASSERT (hRapCh);
	BDBG_ASSERT (psDecConfigParams);
    BSTD_UNUSED(uiPpBranchId);
    BSTD_UNUSED(uiPpStageId);
    
	psDecConfigParams->eType = eType;
	psDecConfigParams->bMonoToAll = false;        
    switch (eType) {
    		case BRAP_DSPCHN_AudioType_eMpeg:
                     BRAP_P_MapMpegUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;
            case BRAP_DSPCHN_AudioType_eAac:
            case BRAP_DSPCHN_AudioType_eAacLoas:                        
            case BRAP_DSPCHN_AudioType_eAacSbr:
            case BRAP_DSPCHN_AudioType_eAacSbrAdts:
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT                
                BRAP_P_MapDolbyPulseUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
#else                
                BRAP_P_MapAacheUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
#endif
    			break;
    		case BRAP_DSPCHN_AudioType_eAc3:
                    BRAP_P_MapMs11Ac3UserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;
    		case BRAP_DSPCHN_AudioType_eAc3Lossless:
    			break;
    		case BRAP_DSPCHN_AudioType_eAc3Plus:
             BRAP_P_MapMs11DdpUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;
    		case BRAP_DSPCHN_AudioType_eWmaStd:
                BRAP_P_MapWmaStdUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;                
    		case BRAP_DSPCHN_AudioType_eDtsBroadcast:
                BRAP_P_MapDtsBroadcastUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;                                
    		case BRAP_DSPCHN_AudioType_eDts:
    			break;
    		case BRAP_DSPCHN_AudioType_eLpcmBd:
    			break;
    		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
    			break; 
    		case BRAP_DSPCHN_AudioType_eDtshd:
                BRAP_P_MapDtsHdUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break;              
              case BRAP_DSPCHN_AudioType_eDtshdSub:
    			break;           
    		case BRAP_DSPCHN_AudioType_eLpcmDvd:
                    BRAP_P_MapLpcmDvdUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
    			break; 
    		case BRAP_DSPCHN_AudioType_eMlp:
    			break;
    		case BRAP_DSPCHN_AudioType_eDtsLbr:
    			break;            
    		case BRAP_DSPCHN_AudioType_eDdp7_1:
    			break;	
    		case BRAP_DSPCHN_AudioType_eMpegMc:
    			break;						
    		case BRAP_DSPCHN_AudioType_eWmaPro:
                BRAP_P_MapWmaProUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);              
                break;
    		case BRAP_DSPCHN_AudioType_ePcm:
    			break;
    		case BRAP_DSPCHN_AudioType_ePcmWav:
                BRAP_P_MapPcmWavUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);              
                break;                
    		case BRAP_DSPCHN_AudioType_eAmr:
                BRAP_P_MapAmrUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
                break;                
    		case BRAP_DSPCHN_AudioType_eDra:
                BRAP_P_MapDraUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);
                break;                        
    		case BRAP_DSPCHN_AudioType_eRealAudioLbr:
                BRAP_P_MapRealAudioLbrUserConfigFwtoApp(&(hRapCh->sDecSettings),psDecConfigParams);                    
                break;    
    		default:
    			BDBG_ERR(("Audio Type %d not supported",eType));
    			return BERR_TRACE(BERR_INVALID_PARAMETER);
    	}
    	psDecConfigParams->eOutputMode = hRapCh->eChannelOutputMode;	
	BDBG_LEAVE (BRAP_DEC_P_GetCurrentConfig);
	return err;
}


/**************************************************************************
Summary:
    Private function that gets the current audio params for a decode channel.
**************************************************************************/
BERR_Code
BRAP_DEC_P_GetCurrentAudioParams (
	BRAP_ChannelHandle	    hRapCh,         /* [in] Decode channel handle */
	BRAP_ChannelParams	    *pAudioParams  /* [out] Current channel params */
	)
{
    BERR_Code                   err = BERR_SUCCESS;
    BRAP_DSPCHN_P_AudioParams	*pDspChParams = NULL;
    BRAP_OutputChannelPair      eChP = BRAP_OutputChannelPair_eMax;
    BRAP_SRCCH_P_Params         sSrcChParams;
    unsigned int                uiPth = 0;
    BRAP_DSPCHN_Handle          hDspCh = NULL;

    BDBG_ENTER(BRAP_DEC_P_GetCurrentAudioParams);
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(pAudioParams);
    if(hRapCh->eChannelType != BRAP_ChannelType_eDecode)
    {
        BDBG_ERR(("BRAP_DEC_P_GetCurrentAudioParams: ChType(%d) is not decode"
            " ChType(%d)", hRapCh->eChannelType, BRAP_ChannelType_eDecode));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    BKNI_Memset(pAudioParams, 0, sizeof(BRAP_ChannelParams));
    
	/* Get parameters that are available in channel handle */
    pAudioParams->eAudioSource      = hRapCh->eAudioSource;
    BRAP_P_ConvertSrToEnum(hRapCh->uiInputSamplingRate,&(pAudioParams->eInputSR));        
    pAudioParams->bInputLfePresent  = hRapCh->bInputLfeOn;
    pAudioParams->eInputAudMode     = hRapCh->eInputAudMode;
    pAudioParams->sMixingCoeff      = hRapCh->sMixingCoeff;


    /* XPT Channel number for the current channel. This is used by the DSP 
       Firmware to determine the CDB and ITB used for the current DSP Context */	
    pAudioParams->sXptContextMap.ContextIdx = hRapCh->uiXptChannelNo;
    pAudioParams->sXptContextMap = hRapCh->sXptContextMap;

	/* Get DSP channel parameters */
    for(uiPth = 0; uiPth < BRAP_P_MAX_PATHS_IN_A_CHAN; uiPth++)
    {
    	if(!(BRAP_P_IsPointerValid((void *)hRapCh->pPath[uiPth])))
    	{
		continue;
    	}
        if(((hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eDecodePcm)||
           (hRapCh->pPath[uiPth]->eUsgPath == BRAP_P_UsgPath_eDecodeCompress)) &&
           (BRAP_P_IsPointerValid((void *)hRapCh->pPath[uiPth]->hDspCh )))
        {
            hDspCh = hRapCh->pPath[uiPth]->hDspCh;
            break;
        }
    }
    if(!(BRAP_P_IsPointerValid((void *)hDspCh)))
    {
        BDBG_ERR(("BRAP_DEC_P_GetCurrentAudioParams: hDspCh not found"));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

	/* Malloc large local structures */
	pDspChParams = hRapCh->hRap->sOpenTimeMallocs.pDspChParams;
	if( NULL==pDspChParams)
	{
		err =  BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
              goto error;
	}
        BKNI_Memset(pDspChParams ,0,sizeof( BRAP_DSPCHN_P_AudioParams ));   
	err = BRAP_DSPCHN_P_GetCurrentAudioParams(hDspCh, pDspChParams);
	if(err != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_DEC_P_GetCurrentAudioParams: "
            " BRAP_DSPCHN_P_GetCurrentAudioParams returned err(%d)", err));
        err =  BERR_TRACE(err);
        goto error;        
    }
    
	pAudioParams->sDspChParams  = pDspChParams->sExtAudioParams;
	pAudioParams->eTimebase     = pDspChParams->eTimebase;
    pAudioParams->bPlayback     = pDspChParams->bPlayback;

    if(!(BRAP_P_IsFwMixingPostLoopbackEnabled(hRapCh)))
    {
    /* Get SrcCh Params */
    for(eChP=0; eChP < BRAP_OutputChannelPair_eMax; eChP++)
    {
            if(!(BRAP_P_IsPointerValid((void *)hRapCh->pPath[uiPth]->sSrcCh[eChP].hSrcCh )))
        {
            continue;
        }

        err = BRAP_SRCCH_P_GetCurrentParams(
                                hRapCh->pPath[uiPth]->sSrcCh[eChP].hSrcCh,
                                &sSrcChParams);
        if(err != BERR_SUCCESS)
        {
		err =  BERR_TRACE( err );
              goto error;
        }
        /* We got one SrcCh, break from the loop */
        break;
    }/* for eChP */
    if(BRAP_OutputChannelPair_eMax == eChP)
    {
        BDBG_ERR(("BRAP_DEC_P_GetCurrentAudioParams: hSrcCh not found"));
        err =  BERR_TRACE( BERR_NOT_INITIALIZED );
        goto error;        
    }
    pAudioParams->eInputBitsPerSample = sSrcChParams.eInputBitsPerSample;

#if (BRAP_UNSIGNED_PCM_SUPPORTED ==1 )
    pAudioParams->bIsPcmSigned = sSrcChParams.bIsSigned;
#endif
    }
    else
    {
        pAudioParams->eInputBitsPerSample = BRAP_InputBitsPerSample_e32;
#if (BRAP_UNSIGNED_PCM_SUPPORTED ==1 )
        pAudioParams->bIsPcmSigned = true;
#endif
    }
    
    /* Invalid entries for params not required for a decode channel */
    pAudioParams->eCapMode      = BRAP_CaptureMode_eMaxCaptureMode;
    pAudioParams->eCapInputPort = hRapCh->eCapInputPort;
    pAudioParams->eInputChPair  = BRAP_OutputChannelPair_eLR;
    pAudioParams->eBufDataMode  = BRAP_BufDataMode_eMaxNum;

   error:
	BDBG_LEAVE( BRAP_DEC_P_GetCurrentAudioParams );
	return err;
}

/**************************************************************************
Details: Refer to Prototype
**************************************************************************/
static void BRAP_P_SetDolbyMsUsageMode(
    BRAP_ChannelHandle          hRapCh,     /* [in] Channel Handle */
    BRAP_AF_P_DolbyMsUsageMode  *peDolbyMsUsageMode
    )
{
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(peDolbyMsUsageMode);

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
    if(true == hRapCh->sDecSettings.sUserConfigAppFormat.bMpegConformanceMode)
    {
        *peDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eMpegConformanceMode;
    }
    else if(true == hRapCh->sDecSettings.sUserConfigAppFormat.bMsUsageMode)
    {
        *peDolbyMsUsageMode = (BRAP_P_IsMs11UsageMode(hRapCh))? \
            BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode:BRAP_AF_P_DolbyMsUsageMode_eMS10DecodeMode;  
    }
    else if((BRAP_P_IsMs11UsageMode(hRapCh)) || (BRAP_P_IsMs10UsageMode(hRapCh)))
    {
        *peDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eMS11SoundEffectMixing;
    }
    else
#endif        
    {        
        *peDolbyMsUsageMode = BRAP_AF_P_DolbyMsUsageMode_eSingleDecodeMode;
    }
}

/**************************************************************************
Details: Refer to Prototype
**************************************************************************/
static void BRAP_P_SetDolbyMsDecoderType(
    BRAP_ChannelHandle              hRapCh,     /* [in] Channel Handle */
    BRAP_AF_P_DolbyMsUsageMode	    eDolbyMsUsageMode, /* [in] Dolby MS Usage Mode */
    BRAP_AF_P_DolbyMsDecoderType    *peDolbyMsDecoderType
    )
{
    BDBG_ASSERT(hRapCh);
    BDBG_ASSERT(peDolbyMsDecoderType);
    
    if((BRAP_AF_P_DolbyMsUsageMode_eMS10DecodeMode == eDolbyMsUsageMode) ||
       (BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode == eDolbyMsUsageMode) ||
       (BRAP_AF_P_DolbyMsUsageMode_eMpegConformanceMode == eDolbyMsUsageMode))
    {
#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
    if(hRapCh->hMultiStreamDecoder != NULL)
    {
            if(hRapCh == hRapCh->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.hPrimaryChannel)
        {
                *peDolbyMsDecoderType = BRAP_AF_P_DolbyMsDecoderType_ePrimary;
        }
            else /* Support for Sound Effects mixing through a Decoder does not exist as yet */
            {
                unsigned int uiChIndex = 0;
                for(uiChIndex = 0; uiChIndex < BRAP_MAX_SEC_CHANNEL_FOR_MS_DECODER; uiChIndex++)
                {
                    if(hRapCh == hRapCh->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.hSecondaryChannel[uiChIndex])
                    {
                        *peDolbyMsDecoderType = BRAP_AF_P_DolbyMsDecoderType_eSecondary;
                    }
                }
            }
        }
        else
        {
            *peDolbyMsDecoderType = BRAP_AF_P_DolbyMsDecoderType_ePrimary;
        }
#endif
    }
    else
    {
        *peDolbyMsDecoderType = BRAP_AF_P_DolbyMsDecoderType_eINVALID;
    }
}

/**************************************************************************
Details: Refer to Prototype
**************************************************************************/
static void BRAP_P_SetDolbyMsDecoderOutputCfg(
    BRAP_ChannelHandle          hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType       eDecodeAlgo, /* [in] Decode Audio Algorithm Type */    
    BRAP_AF_P_DolbyMsUsageMode  eDolbyMsUsageMode /* [in] Dolby MS Usage Mode */
    )
{
    bool            bOutputLfeOn = false;
    uint32_t        ui32OutputMode = 2;
    BRAP_OutputMode	eOutputMode = BRAP_OutputMode_e2_0;
    uint32_t        *pui32OutMode = 0, *pui32OutLfe = 0;
    uint32_t        *pui32OutputChannelMatrix, *pi32NumOutPorts=0;
    
    BDBG_ASSERT(hRapCh);

    if(BRAP_AF_P_DolbyMsUsageMode_eMS11DecodeMode == eDolbyMsUsageMode)
    {
        eOutputMode = BRAP_OutputMode_e3_2;
        bOutputLfeOn = true;
    }
    else if(BRAP_AF_P_DolbyMsUsageMode_eMS10DecodeMode == eDolbyMsUsageMode)
    {
        eOutputMode = BRAP_OutputMode_e2_0;
        bOutputLfeOn = false;
    }
    else
    {
        BDBG_MSG(("Dolby MS Decoder Output configuration need not be updated for MS Usage Mode %d",eDolbyMsUsageMode));
        return;
    }
    
    switch(eDecodeAlgo)
    {
        case BRAP_DSPCHN_AudioType_eAc3:
            pi32NumOutPorts = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.i32NumOutPorts;
            pui32OutputChannelMatrix = hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.sUserOutputCfg[0].ui32OutputChannelMatrix;
            pui32OutMode = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.sUserOutputCfg[0].i32OutMode;
            pui32OutLfe = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam.sUserOutputCfg[0].i32OutLfe;
            break;
            
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            pi32NumOutPorts = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.i32NumOutPorts;
            pui32OutputChannelMatrix = hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.sUserOutputCfg[0].ui32OutputChannelMatrix;
            pui32OutMode = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.sUserOutputCfg[0].i32OutMode;
            pui32OutLfe = (uint32_t *)&hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam.sUserOutputCfg[0].i32OutLfe;
            break;  
            
        case BRAP_DSPCHN_AudioType_eAac:
        case BRAP_DSPCHN_AudioType_eAacLoas:
        case BRAP_DSPCHN_AudioType_eAacSbr:
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:            
            pi32NumOutPorts = &hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.ui32NumOutPorts;
            pui32OutputChannelMatrix = hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.sOutPortCfg[0].ui32OutputChannelMatrix;
            pui32OutMode = &hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.sOutPortCfg[0].ui32OutMode;
            pui32OutLfe = &hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam.sOutPortCfg[0].ui32OutLfe;
            break;    

        default:
            BDBG_MSG(("Dolby MS Decoder Output configuration need not be updated for Algo %d",eDecodeAlgo));
            return;
    }

    /* MS10 and MS11 modes support only one port from decoder, Stereo and Multichannel respectively. */
    *pi32NumOutPorts = 1;
    
    BRAP_DSPCHN_P_GetOutputModeApptoFw(eOutputMode, &ui32OutputMode);
        
    BRAP_P_GetOutputChannelmatrix(eOutputMode,pui32OutputChannelMatrix,bOutputLfeOn);
    *pui32OutMode = ui32OutputMode ;
    *pui32OutLfe = bOutputLfeOn;
}

/**************************************************************************
Details: Refer to Prototype
**************************************************************************/
BERR_Code BRAP_P_UpdateStartTimeDeducibleFWUserConfig(
    BRAP_ChannelHandle      hRapCh,     /* [in] Channel Handle */
    BRAP_DSPCHN_AudioType   eDecodeAlgo /* [in] Decode Audio Algorithm Type */    
)
{
    BERR_Code   ret = BERR_SUCCESS;

    BDBG_ASSERT(hRapCh);
    BDBG_ENTER(BRAP_P_UpdateStartTimeDeducibleFWUserConfig);

    switch(eDecodeAlgo)
    {
        case BRAP_DSPCHN_AudioType_eAc3:
        case BRAP_DSPCHN_AudioType_eAc3Plus:
            {
                BRAP_FWIF_P_DDPMultiStreamConfigParams *psMs11DdpFwFormatConfigParam = NULL;

                if(BRAP_DSPCHN_AudioType_eAc3 == eDecodeAlgo)
                    psMs11DdpFwFormatConfigParam = &(hRapCh->sDecSettings.sUserConfigStruct.sMs11AC3ConfigParam);
                else if(BRAP_DSPCHN_AudioType_eAc3Plus == eDecodeAlgo)
                    psMs11DdpFwFormatConfigParam = &(hRapCh->sDecSettings.sUserConfigStruct.sMs11DDPConfigParam);
                 
                BRAP_P_SetDolbyMsUsageMode(hRapCh, &psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode);

                BRAP_P_SetDolbyMsDecoderType(hRapCh, psMs11DdpFwFormatConfigParam->\
                        eDolbyMsUsageMode, &psMs11DdpFwFormatConfigParam->eDecoderType);

                BRAP_P_SetDolbyMsDecoderOutputCfg(hRapCh, eDecodeAlgo,
                    psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode);
                
                BDBG_MSG(("%s: AC3/DDP eDolbyMsUsageMode = %d eDecoderType = %d",
                    __FUNCTION__,psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode,
                    psMs11DdpFwFormatConfigParam->eDecoderType));
            }
            break;
            
        case BRAP_DSPCHN_AudioType_eAac:
        case BRAP_DSPCHN_AudioType_eAacLoas:
        case BRAP_DSPCHN_AudioType_eAacSbr:
        case BRAP_DSPCHN_AudioType_eAacSbrAdts:
            {
                BRAP_FWIF_P_DolbyPulseUserConfig    *psDolbyPulseFwFormatConfigParam = NULL;
                psDolbyPulseFwFormatConfigParam = &(hRapCh->sDecSettings.sUserConfigStruct.sDolbyPulseConfigParam);
                
                BRAP_P_SetDolbyMsUsageMode(hRapCh, &psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode);
                
                BRAP_P_SetDolbyMsDecoderType(hRapCh, psDolbyPulseFwFormatConfigParam->\
                        eDolbyPulseUsageMode, &psDolbyPulseFwFormatConfigParam->eDecoderType);

                BRAP_P_SetDolbyMsDecoderOutputCfg(hRapCh, eDecodeAlgo,
                        psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode);                

                BDBG_MSG(("%s: Dolby Pulse eDolbyPulseUsageMode = %d eDecoderType = %d",
                    __FUNCTION__,psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode,
                        psDolbyPulseFwFormatConfigParam->eDecoderType));
            }
            break;

        case BRAP_DSPCHN_AudioType_ePcmWav:
            {
                BRAP_FWIF_P_PcmWavConfigParams	        *psPcmWavFwFormatConfigParam = NULL;
                psPcmWavFwFormatConfigParam = &(hRapCh->sDecSettings.sUserConfigStruct.sPcmWavConfigParam);
                
                BRAP_P_SetDolbyMsUsageMode(hRapCh, &psPcmWavFwFormatConfigParam->eDolbyMsUsageMode);

                BDBG_MSG(("%s: Dolby Pulse eDolbyMsUsageMode = %d",
                    __FUNCTION__,psPcmWavFwFormatConfigParam->eDolbyMsUsageMode));
            }            
            break;
            
        default:
            break;
    }

    BDBG_LEAVE(BRAP_P_UpdateStartTimeDeducibleFWUserConfig);
    return ret;
}

BERR_Code BRAP_P_MapMs11DdpUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
	BRAP_DSPCHN_Ac3ConfigParams *psMs11DdpAppFormatConfigParams;
       BRAP_FWIF_P_DDPMultiStreamConfigParams	        *psMs11DdpFwFormatConfigParam;   
	unsigned int row = 0, count = 0, i = 0;
    	BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
       BDBG_ENTER(BRAP_P_MapMs11DdpUserConfigFwtoApp);

        psMs11DdpAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAc3PlusConfigParams);
        psMs11DdpFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMs11DDPConfigParam);

        psDecConfigParams->bOutputLfeOn = psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32OutLfe;	        
        BRAP_DSPCHN_P_GetOutputModeFwtoApp(psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32OutMode,&eOutputMode);
        psDecConfigParams->eOutputMode	 =  eOutputMode ;

        BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32DualMode,
            &(psDecConfigParams->eDualMonoMode));

/* Returning these values which already stored in App format */
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi = psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleHigh;
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo = psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleLow;
        psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor = psDecSettings->sUserConfigAppFormat.i32Ac3PcmScale;        
        psMs11DdpAppFormatConfigParams->eCompMode = psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32CompMode;   

        psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath= psDecSettings->sUserConfigAppFormat.i32Ms11DdpDynScaleHigh_DownMixPath;
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath= psDecSettings->sUserConfigAppFormat.i32Ms11DdpDynScaleLow_DownMixPath;
        psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath=psDecSettings->sUserConfigAppFormat.i32Ms11DdpPcmScale_DownMixPath;        
        psMs11DdpAppFormatConfigParams->eCompMode_DownmixPath= psMs11DdpFwFormatConfigParam->sUserOutputCfg[1].i32CompMode;                

        psMs11DdpAppFormatConfigParams->bMsUsageMode = psDecSettings->sUserConfigAppFormat.bMsUsageMode;
        
        psMs11DdpAppFormatConfigParams->bStreamDialogNormEnable = psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable;
        psMs11DdpAppFormatConfigParams->ui16UserDialogNormVal = psMs11DdpFwFormatConfigParam->i32UserDialNormVal;
        psMs11DdpAppFormatConfigParams->eKaraokeVocalSelect = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode;   
        psMs11DdpAppFormatConfigParams->bUseKaraokeLevelPanCoeff = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[0] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[1] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[2] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[3] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[4] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[5] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan ;

        psMs11DdpAppFormatConfigParams->eStereoMode = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode;

        psMs11DdpAppFormatConfigParams->bUseUserDownmixCoeff = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled;

	for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
	{
		for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
		{	
			psMs11DdpAppFormatConfigParams->i16UserDownmixCoeff[row][count] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count]  ;
		}
    	}
          psMs11DdpAppFormatConfigParams->uiSubstreamIDToDecode= psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode;  
       BDBG_LEAVE(BRAP_P_MapMs11DdpUserConfigFwtoApp);
    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapMs11DdpUserConfigApptoFw(
                    BRAP_ChannelHandle  hRapCh,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_Ac3ConfigParams *psMs11DdpAppFormatConfigParams;
    BRAP_FWIF_P_DDPMultiStreamConfigParams *psMs11DdpFwFormatConfigParam;
    unsigned int row = 0, count = 0, i = 0,ui32OutputMode = 2;
    BRAP_P_DecoderSettings          *psDecSettings = &(hRapCh->sDecSettings);    

    BDBG_ENTER(BRAP_P_MapMs11DdpUserConfigApptoFw);

    psMs11DdpAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAc3PlusConfigParams);
    psMs11DdpFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMs11DDPConfigParam);

    if((BRAP_P_State_eStarted == hRapCh->eState) &&
       (psDecSettings->sUserConfigAppFormat.bMsUsageMode != psMs11DdpAppFormatConfigParams->bMsUsageMode))
    {
        BDBG_ERR(("Error!!! DDP User Config parameter 'bMsUsageMode' cannot be changed on the fly"));
        return BERR_NOT_SUPPORTED;
    }

    psDecSettings->sUserConfigAppFormat.bMsUsageMode = psMs11DdpAppFormatConfigParams->bMsUsageMode;
    
    BRAP_P_SetDolbyMsUsageMode(hRapCh, &psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode);

    BRAP_P_SetDolbyMsDecoderType(hRapCh, psMs11DdpFwFormatConfigParam->\
            eDolbyMsUsageMode, &psMs11DdpFwFormatConfigParam->eDecoderType);                
    
    BDBG_MSG(("FUNCTION= %s eDolbyMsUsageMode = %d",__FUNCTION__,
                psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode));    

    psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable = psMs11DdpAppFormatConfigParams->bStreamDialogNormEnable;
    psMs11DdpFwFormatConfigParam->i32UserDialNormVal = psMs11DdpAppFormatConfigParams->ui16UserDialogNormVal;
    
    /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32Ac3PcmScale = psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor;
    psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleHigh = psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi;
    psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleLow = psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo;       
    
    psDecSettings->sUserConfigAppFormat.i32Ms11DdpPcmScale_DownMixPath = psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath;
    psDecSettings->sUserConfigAppFormat.i32Ms11DdpDynScaleHigh_DownMixPath = psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath;
    psDecSettings->sUserConfigAppFormat.i32Ms11DdpDynScaleLow_DownMixPath = psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath;        

    /* Defaulted to 2, updated accordingly for MS10/MS11 in BRAP_P_SetDolbyMsDecoderOutputCfg() */
    psMs11DdpFwFormatConfigParam->i32NumOutPorts = 2;

    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {	
        BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode = psMs11DdpAppFormatConfigParams->eStereoMode;

        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(psDecConfigParams->eDualMonoMode,
        (unsigned int *)&(psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DualMode));

        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode = psMs11DdpAppFormatConfigParams->eKaraokeVocalSelect;
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled = psMs11DdpAppFormatConfigParams->bUseUserDownmixCoeff;

        for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count] = psMs11DdpAppFormatConfigParams->i16UserDownmixCoeff[row][count];
            }
        }
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled = psMs11DdpAppFormatConfigParams->bUseKaraokeLevelPanCoeff;
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level =  psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[0];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[1];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[2];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[3];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[4];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[5];
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = ui32OutputMode ;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = psDecConfigParams->bOutputLfeOn;  
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode = psMs11DdpAppFormatConfigParams->eCompMode;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo,100);            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);       
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = 2 ;          
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = false;    
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode = psMs11DdpAppFormatConfigParams->eCompMode_DownmixPath;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath,100);            
        }
    }
    psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode = psMs11DdpAppFormatConfigParams->uiSubstreamIDToDecode;      


    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Ms11Ddp : APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
	
	BDBG_MSG((" eDolbyMsUsageMode = %d",psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode));
	BDBG_MSG((" eDecoderType = %d",psMs11DdpFwFormatConfigParam->eDecoderType));
	BDBG_MSG((" i32NumOutPorts = %d",psMs11DdpFwFormatConfigParam->i32NumOutPorts));
	BDBG_MSG(("------------------"));
    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {
		BDBG_MSG((" i32StereoMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode));
        BDBG_MSG((" i32DualMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DualMode));
		BDBG_MSG((" i32Kmode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode));
		BDBG_MSG((" i32ExtDnmixEnabled = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled));
		for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                BDBG_MSG((" i32ExtDnmixTab[%d][%d] = %d",row,count,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count] ));
            }
        }
		BDBG_MSG((" i32ExtKaraokeEnabled = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled ));
		BDBG_MSG((" i32Extv1Leve = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level));
		BDBG_MSG((" i32Extv1Pan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan));
		BDBG_MSG((" i32Extv2Level = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level));
		BDBG_MSG((" i32Extv2Pan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan));
		BDBG_MSG((" i32ExtGmLevel = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel));
		BDBG_MSG((" i32ExtGmPan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan));
		BDBG_MSG((" i32OutMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode));
		BDBG_MSG((" i32OutLfe = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe));
		BDBG_MSG((" i32CompMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode));
        BDBG_MSG(("------------------"));
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BDBG_MSG((" ui16PcmScaleFactor = 0x%x -> ui16PcmScaleFactori32PcmScale = 0x%x",psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale));
    		BDBG_MSG((" ui16DynRngScaleHi = 0x%x -> i32DynScaleHigh = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh));
    		BDBG_MSG((" ui16DynRngScaleLo = 0x%x -> i32DynScaleLo = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow ));
        }
        else
        {
            BDBG_MSG((" ui16PcmScaleFactor_DownmixPath = 0x%x -> i32PcmScale = 0x%x",psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale));
    		BDBG_MSG((" ui16DynRngScaleHi_DownmixPath = 0x%x -> i32DynScaleHigh = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh));
    		BDBG_MSG((" ui16DynRngScaleLo_DownmixPath = 0x%x -> i32DynScaleLo = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow ));
        }
        BDBG_MSG(("------------------"));    
		for (row = 0; row < BRAP_AF_P_MAX_CHANNELS; row++)
		{
			BDBG_MSG((" ui32OutputChannelMatrix[%d] = %d",row,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix[row]));
		}
		
	}
	BDBG_MSG(("------------------"));
	BDBG_MSG((" i32StreamDialNormEnable = %d",psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable));
	BDBG_MSG((" i32UserDialNormVal = %d",psMs11DdpFwFormatConfigParam->i32UserDialNormVal));
	BDBG_MSG((" ui32SubstreamIDToDecode = %d",psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode));

	BDBG_MSG(("-------------------------------------------")); 
    
    
    BDBG_LEAVE(BRAP_P_MapMs11DdpUserConfigApptoFw);
    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapMs11Ac3UserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
	BRAP_DSPCHN_Ac3ConfigParams *psMs11DdpAppFormatConfigParams;
       BRAP_FWIF_P_DDPMultiStreamConfigParams	        *psMs11DdpFwFormatConfigParam;   
	unsigned int row = 0, count = 0, i = 0;
    	BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
       BDBG_ENTER(BRAP_P_MapMs11Ac3UserConfigFwtoApp);

        psMs11DdpAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAc3ConfigParams);
        psMs11DdpFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMs11AC3ConfigParam);

        psDecConfigParams->bOutputLfeOn = psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32OutLfe;	        
        BRAP_DSPCHN_P_GetOutputModeFwtoApp(psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32OutMode,&eOutputMode);
        psDecConfigParams->eOutputMode	 =  eOutputMode ;

        BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32DualMode,
            &(psDecConfigParams->eDualMonoMode));

/* Returning these values which already stored in App format */
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi = psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleHigh;
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo = psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleLow;
        psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor = psDecSettings->sUserConfigAppFormat.i32Ac3PcmScale;        
        psMs11DdpAppFormatConfigParams->eCompMode = psMs11DdpFwFormatConfigParam->sUserOutputCfg[0].i32CompMode;   

        psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath= psDecSettings->sUserConfigAppFormat.i32Ms11Ac3DynScaleHigh_DownMixPath;
        psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath= psDecSettings->sUserConfigAppFormat.i32Ms11Ac3DynScaleLow_DownMixPath ;
        psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath= psDecSettings->sUserConfigAppFormat.i32Ms11Ac3PcmScale_DownMixPath;        
        psMs11DdpAppFormatConfigParams->eCompMode_DownmixPath= psMs11DdpFwFormatConfigParam->sUserOutputCfg[1].i32CompMode;                

        psMs11DdpAppFormatConfigParams->bMsUsageMode = psDecSettings->sUserConfigAppFormat.bMsUsageMode;
        
        psMs11DdpAppFormatConfigParams->bStreamDialogNormEnable = psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable;
        psMs11DdpAppFormatConfigParams->ui16UserDialogNormVal = psMs11DdpFwFormatConfigParam->i32UserDialNormVal;
        psMs11DdpAppFormatConfigParams->eKaraokeVocalSelect = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode;   
        psMs11DdpAppFormatConfigParams->bUseKaraokeLevelPanCoeff = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[0] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[1] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[2] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[3] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[4] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel ;
        psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[5] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan ;

        psMs11DdpAppFormatConfigParams->eStereoMode = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode;

        psMs11DdpAppFormatConfigParams->bUseUserDownmixCoeff = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled;

	for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
	{
		for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
		{	
			psMs11DdpAppFormatConfigParams->i16UserDownmixCoeff[row][count] = psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count]  ;
		}
    	}
          psMs11DdpAppFormatConfigParams->uiSubstreamIDToDecode= psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode;  
       BDBG_LEAVE(BRAP_P_MapMs11Ac3UserConfigFwtoApp);
    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapMs11Ac3UserConfigApptoFw(
                    BRAP_ChannelHandle  hRapCh,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_Ac3ConfigParams *psMs11DdpAppFormatConfigParams;
    BRAP_FWIF_P_DDPMultiStreamConfigParams *psMs11DdpFwFormatConfigParam;
    unsigned int row = 0, count = 0, i = 0,ui32OutputMode = 2;
    BRAP_P_DecoderSettings          *psDecSettings = &(hRapCh->sDecSettings);    

    BDBG_ENTER(BRAP_P_MapMs11Ac3UserConfigApptoFw);

    psMs11DdpAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAc3ConfigParams);
    psMs11DdpFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMs11AC3ConfigParam);

    if((BRAP_P_State_eStarted == hRapCh->eState) &&
       (psDecSettings->sUserConfigAppFormat.bMsUsageMode != psMs11DdpAppFormatConfigParams->bMsUsageMode))
        {
        BDBG_ERR(("Error!!! DDP User Config parameter 'bMsUsageMode' cannot be changed on the fly"));
        return BERR_NOT_SUPPORTED;
    }

    psDecSettings->sUserConfigAppFormat.bMsUsageMode = psMs11DdpAppFormatConfigParams->bMsUsageMode;

    BRAP_P_SetDolbyMsUsageMode(hRapCh, &psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode);

    BRAP_P_SetDolbyMsDecoderType(hRapCh, psMs11DdpFwFormatConfigParam->\
            eDolbyMsUsageMode, &psMs11DdpFwFormatConfigParam->eDecoderType);                
    
    BDBG_MSG(("psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode = %d",psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode));    

    psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable = psMs11DdpAppFormatConfigParams->bStreamDialogNormEnable;
    psMs11DdpFwFormatConfigParam->i32UserDialNormVal = psMs11DdpAppFormatConfigParams->ui16UserDialogNormVal;

    
    /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32Ac3PcmScale = psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor;
    psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleHigh = psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi;
    psDecSettings->sUserConfigAppFormat.i32Ac3DynScaleLow = psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo;       
    
    psDecSettings->sUserConfigAppFormat.i32Ms11Ac3PcmScale_DownMixPath = psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath;
    psDecSettings->sUserConfigAppFormat.i32Ms11Ac3DynScaleHigh_DownMixPath = psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath;
    psDecSettings->sUserConfigAppFormat.i32Ms11Ac3DynScaleLow_DownMixPath = psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath;        

    /* Defaulted to 2, updated accordingly for MS10/MS11 in BRAP_P_SetDolbyMsDecoderOutputCfg() */
    psMs11DdpFwFormatConfigParam->i32NumOutPorts = 2;

    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {
	
        BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode = psMs11DdpAppFormatConfigParams->eStereoMode;

        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(psDecConfigParams->eDualMonoMode,
        (unsigned int *)&(psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DualMode));

        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode = psMs11DdpAppFormatConfigParams->eKaraokeVocalSelect;
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled = psMs11DdpAppFormatConfigParams->bUseUserDownmixCoeff;

        for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count] = psMs11DdpAppFormatConfigParams->i16UserDownmixCoeff[row][count];
            }
        }
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled = psMs11DdpAppFormatConfigParams->bUseKaraokeLevelPanCoeff;
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level =  psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[0];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[1];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[2];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[3];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[4];
        psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan = psMs11DdpAppFormatConfigParams->i16KaraokeLevelPanCoeff[5];
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = ui32OutputMode ;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = psDecConfigParams->bOutputLfeOn;  
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode = psMs11DdpAppFormatConfigParams->eCompMode;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo,100);            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);       
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = 2 ;          
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = false;    
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode = psMs11DdpAppFormatConfigParams->eCompMode_DownmixPath;
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath,100);
            psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath,100);            
        }
    }
    psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode = psMs11DdpAppFormatConfigParams->uiSubstreamIDToDecode;      

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Ms11Ac3 : APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
	
	BDBG_MSG((" eDolbyMsUsageMode = %d",psMs11DdpFwFormatConfigParam->eDolbyMsUsageMode));
	BDBG_MSG((" eDecoderType = %d",psMs11DdpFwFormatConfigParam->eDecoderType));
	BDBG_MSG((" i32NumOutPorts = %d",psMs11DdpFwFormatConfigParam->i32NumOutPorts));
	BDBG_MSG(("------------------"));
    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {
		BDBG_MSG((" i32StereoMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32StereoMode));
		BDBG_MSG((" i32DualMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DualMode));
		BDBG_MSG((" i32Kmode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Kmode));
		BDBG_MSG((" i32ExtDnmixEnabled = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled));
		for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                BDBG_MSG((" i32ExtDnmixTab[%d][%d] ",row,count,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count] ));
            }
        }
		BDBG_MSG((" i32ExtKaraokeEnabled = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtKaraokeEnabled ));
		BDBG_MSG((" i32Extv1Leve = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Level));
		BDBG_MSG((" i32Extv1Pan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv1Pan));
		BDBG_MSG((" i32Extv2Level = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Level));
		BDBG_MSG((" i32Extv2Pan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32Extv2Pan));
		BDBG_MSG((" i32ExtGmLevel = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmLevel));
		BDBG_MSG((" i32ExtGmPan = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32ExtGmPan));
		BDBG_MSG((" i32OutMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutMode));
		BDBG_MSG((" i32OutLfe = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe));
		BDBG_MSG((" i32CompMode = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32CompMode));

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BDBG_MSG((" ui16PcmScaleFactor = 0x%x -> ui16PcmScaleFactori32PcmScale = 0x%x",psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale));
    		BDBG_MSG((" ui16DynRngScaleHi = 0x%x -> i32DynScaleHigh = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh));
    		BDBG_MSG((" ui16DynRngScaleLo = 0x%x -> i32DynScaleLo = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow ));
        }
        else
        {
            BDBG_MSG((" ui16PcmScaleFactor_DownmixPath = 0x%x -> i32PcmScale = 0x%x",psMs11DdpAppFormatConfigParams->ui16PcmScaleFactor_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale));
    		BDBG_MSG((" ui16DynRngScaleHi_DownmixPath = 0x%x -> i32DynScaleHigh = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleHi_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh));
    		BDBG_MSG((" ui16DynRngScaleLo_DownmixPath = 0x%x -> i32DynScaleLo = 0x%x",psMs11DdpAppFormatConfigParams->ui16DynRngScaleLo_DownmixPath,psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow ));
        }
        BDBG_MSG((" i32PcmScale = 0x%x",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32PcmScale));
		BDBG_MSG((" i32DynScaleHigh = 0x%x",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh));
		BDBG_MSG((" i32DynScaleLo = 0x%x",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow ));
		
		for (row = 0; row < BRAP_AF_P_MAX_CHANNELS; row++)
		{
			BDBG_MSG((" ui32OutputChannelMatrix = %d",psMs11DdpFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix));
		}
		
	}
	BDBG_MSG(("------------------"));
	BDBG_MSG((" i32StreamDialNormEnable = %d",psMs11DdpFwFormatConfigParam->i32StreamDialNormEnable));
	BDBG_MSG((" i32UserDialNormVal = %d",psMs11DdpFwFormatConfigParam->i32UserDialNormVal));
	BDBG_MSG((" ui32SubstreamIDToDecode = %d",psMs11DdpFwFormatConfigParam->ui32SubstreamIDToDecode));

	BDBG_MSG(("-------------------------------------------")); 
    BDBG_LEAVE(BRAP_P_MapMs11Ac3UserConfigApptoFw);
    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapLpcmDvdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_DSPCHN_LpcmDvdConfigParams *psLpcmDvdAppFormatConfigParams;
    BRAP_FWIF_P_LpcmUserConfig*  psLpcmDvdFwFormatConfigParam;   
    unsigned int row = 0, count = 0, i = 0;
    BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
    BDBG_ENTER(BRAP_P_MapLpcmDvdUserConfigFwtoApp);

    psLpcmDvdAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sLpcmDvdConfigParams);
    psLpcmDvdFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sLpcmDvdConfigParam);

    psDecConfigParams->bOutputLfeOn = psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32LfeOnFlag;	
    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;

    BRAP_DSPCHN_P_GetDualMonoModeFwtoApp((unsigned int)psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32DualMonoModeFlag,
    &(psDecConfigParams->eDualMonoMode));

/* Returning these values which already stored in App format */
        
    psLpcmDvdAppFormatConfigParams->bUseUserDownmixCoeff = psLpcmDvdFwFormatConfigParam->ui32UseUserDownmixCoeffsFlag;

    for (row = 0; row < 8; row++)
    {
        for (count = 0; count < 8; count++)
        {	
            psLpcmDvdAppFormatConfigParams->i32UserDownmixCoeff[row][count] = psLpcmDvdFwFormatConfigParam->i32UserDownmixTables[0][row][count];
        }
    }
    BDBG_LEAVE(BRAP_P_MapLpcmDvdUserConfigFwtoApp);
    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapLpcmDvdUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_LpcmDvdConfigParams *psLpcmDvdAppFormatConfigParams;
    BRAP_FWIF_P_LpcmUserConfig	        *psLpcmDvdFwFormatConfigParam;
    unsigned int row = 0, count = 0, i = 0 ,ui32OutputMode = 2;

    BDBG_ENTER(BRAP_P_MapLpcmDvdUserConfigApptoFw);

    psLpcmDvdAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sLpcmDvdConfigParams);
    psLpcmDvdFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sLpcmDvdConfigParam);

    /* TODO: Program i32NumOutPorts properly when support is added for concurrent stereo downmix */
    psLpcmDvdFwFormatConfigParam->ui32NumOutputPorts = 2;

    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {	
        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(psDecConfigParams->eDualMonoMode,
        (unsigned int *)&(psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32DualMonoModeFlag));

        BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);              
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutMode = ui32OutputMode ;
            psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32LfeOnFlag = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutputChannelMatrix,false);       
            psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutMode = 2 ;          
            psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32LfeOnFlag = false;            
        }
    }
    psLpcmDvdFwFormatConfigParam->ui32UseUserDownmixCoeffsFlag = psLpcmDvdAppFormatConfigParams->bUseUserDownmixCoeff;    
    for(i =0 ; i < 16 ; i++)
    {
        for (row = 0; row < 8; row++)
        {
            for (count = 0; count < 8; count++)
            {	
                psLpcmDvdFwFormatConfigParam->i32UserDownmixTables[i][row][count] = psLpcmDvdAppFormatConfigParams->i32UserDownmixCoeff[row][count];
            }
        }
    }


    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("LpcmDvd : APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
	

	BDBG_MSG((" i32NumOutPorts = %d",psLpcmDvdFwFormatConfigParam->ui32NumOutputPorts));
    BDBG_MSG(("------------------"));
	for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {	
        
        
        BDBG_MSG((" ui32OutMode = %d",psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutMode));
		BDBG_MSG((" ui32LfeOnFlag = %d",psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32LfeOnFlag));
		BDBG_MSG((" ui32DualMonoModeFlag = %d",psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32DualMonoModeFlag));

		for (row = 0; row < BRAP_AF_P_MAX_CHANNELS; row++)
        {
			BDBG_MSG((" ui32OutputChannelMatrix[%d] = %d",row,psLpcmDvdFwFormatConfigParam->sOutputConfig[i].ui32OutputChannelMatrix[row]));
		}
	    
    }
	BDBG_MSG(("------------------"));
	BDBG_MSG((" ui32UseUserDownmixCoeffsFlag = %d ",psLpcmDvdFwFormatConfigParam->ui32UseUserDownmixCoeffsFlag));
	
	for(i =0 ; i < 16 ; i++)
    {
        for (row = 0; row < 8; row++)
        {
            for (count = 0; count < 8; count++)
            {	
                BDBG_MSG((" i32UserDownmixTables[%d][%d][%d]  = %d ",i,row,count,psLpcmDvdFwFormatConfigParam->i32UserDownmixTables[i][row][count] ));
            }
        }
    }



	BDBG_MSG(("-------------------------------------------")); 
    BDBG_LEAVE(BRAP_P_MapLpcmDvdUserConfigApptoFw);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapMpegUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_MpegConfigParams *psMpegAppFormatConfigParams;
    BRAP_FWIF_P_MpegConfigParams	        *psMpegFwFormatConfigParam;
    unsigned int ui32OutputMode = 2,i=0;

    psMpegAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sMpegConfigParams);
    psMpegFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMpegConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);

    psMpegFwFormatConfigParam->ui32OutMode = ui32OutputMode;

    psMpegFwFormatConfigParam->ui32LeftChannelGain= BRAP_P_FloatToQ131(psMpegAppFormatConfigParams->ui32LeftChannelGain, 100);
    psMpegFwFormatConfigParam->ui32RightChannelGain= BRAP_P_FloatToQ131(psMpegAppFormatConfigParams->ui32RightChannelGain, 100);	

    /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32MpegLeftChannelGain = psMpegAppFormatConfigParams->ui32LeftChannelGain;
    psDecSettings->sUserConfigAppFormat.i32MpegRightChannelGain = psMpegAppFormatConfigParams->ui32RightChannelGain;        


    psMpegFwFormatConfigParam->ui32DualMonoMode= psDecConfigParams->eDualMonoMode;

    BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psMpegFwFormatConfigParam->ui32OutputChannelMatrix,false);


    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Mpeg :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" ui32OutMode = %d",psMpegFwFormatConfigParam->ui32OutMode));
    BDBG_MSG((" ui32LeftChannelGain = %d -> ui32LeftChannelGain = 0x%x",psMpegAppFormatConfigParams->ui32LeftChannelGain,psMpegFwFormatConfigParam->ui32LeftChannelGain));
    BDBG_MSG((" ui32RightChannelGain = %d -> ui32RightChannelGain = 0x%x",psMpegAppFormatConfigParams->ui32RightChannelGain,psMpegFwFormatConfigParam->ui32RightChannelGain));    
    BDBG_MSG((" ui32DualMonoMode = %d",psMpegFwFormatConfigParam->ui32DualMonoMode));

    for(i=0; i<BRAP_AF_P_MAX_CHANNELS; i++)
    {
        BDBG_MSG(("ui32OutputChannelMatrix[%d] = 0x%x",i,psMpegFwFormatConfigParam->ui32OutputChannelMatrix[i]));        
    }
    BDBG_MSG(("-------------------------------------------")); 

    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapMpegUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_DSPCHN_MpegConfigParams *psMpegAppFormatConfigParams;
    BRAP_FWIF_P_MpegConfigParams	        *psMpegFwFormatConfigParam;
    /*    	BRAP_OutputMode	                eOutputMode;	*/
    BDBG_ENTER(BRAP_P_MapMpegUserConfigFwtoApp);

    psMpegAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sMpegConfigParams);
    psMpegFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sMpegConfigParam);

    psDecConfigParams->eDualMonoMode = psMpegFwFormatConfigParam->ui32DualMonoMode;
    /* Hardcoding as of now till FW adds these two fields */
    psDecConfigParams->bOutputLfeOn = false;	

    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psMpegFwFormatConfigParam->ui32OutMode,&(psDecConfigParams->eOutputMode));

    /* Returning these values which already stored in App format */        
    psMpegAppFormatConfigParams->ui32LeftChannelGain = psDecSettings->sUserConfigAppFormat.i32MpegLeftChannelGain;
    psMpegAppFormatConfigParams->ui32RightChannelGain =psDecSettings->sUserConfigAppFormat.i32MpegRightChannelGain;        

    BDBG_LEAVE(BRAP_P_MapMpegUserConfigFwtoApp);

    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapAacheUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
	BRAP_DSPCHN_AacSbrConfigParams *psAacheAppFormatConfigParams;
    BRAP_FWIF_P_AacheConfigParams  *psAacheFwFormatConfigParam;   
	unsigned int row = 0, count = 0, i = 0;
	BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
    BDBG_ENTER(BRAP_P_MapAacheUserConfigFwtoApp);

    psAacheAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAacSbrConfigParams);
    psAacheFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sAacheConfigParam);

    psDecConfigParams->bOutputLfeOn = psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe;	
    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;

    BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(psAacheFwFormatConfigParam->sUserOutputCfg[i].i32DualMode,
        &(psDecConfigParams->eDualMonoMode));

/* Returning these values which already stored in App format */
    psAacheAppFormatConfigParams->ui16DrcGainControlCompress = psDecSettings->sUserConfigAppFormat.i32AacDrcGainControlCompress;
    psAacheAppFormatConfigParams->ui16DrcGainControlBoost = psDecSettings->sUserConfigAppFormat.i32AacDrcGainControlBoost;
    psAacheAppFormatConfigParams->ui16DrcTargetLevel = psAacheFwFormatConfigParam->ui32DrcTargetLevel;
    
    psAacheAppFormatConfigParams->eDownmixType = psAacheFwFormatConfigParam->i32DownmixType;        
    psAacheAppFormatConfigParams->bOutputLfeOn = psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe;   
    psAacheAppFormatConfigParams->bExtDownmixEnable = psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled;
    psAacheAppFormatConfigParams->bEnable96KhzDecode = psAacheFwFormatConfigParam->ui32FreqExtensionEnable;        

    for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
    {
    	for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
    	{	
    		psAacheAppFormatConfigParams->ui16ExtDnMixCoeffs[row][count] = psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count]  ;
    	}
    }
    
    psAacheAppFormatConfigParams->ui16DownmixCoefScaleIndex =  psAacheFwFormatConfigParam->ui32DownmixCoefScaleIndex;    
    BDBG_LEAVE(BRAP_P_MapAacheUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapAacheUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_AacSbrConfigParams *psAacheAppFormatConfigParams;
    BRAP_FWIF_P_AacheConfigParams  *psAacheFwFormatConfigParam;
    unsigned int row = 0, count = 0, i = 0,ui32OutputMode = 2;

    BDBG_ENTER(BRAP_P_MapAacheUserConfigApptoFw);

    psAacheAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAacSbrConfigParams);
    psAacheFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sAacheConfigParam);

    psAacheFwFormatConfigParam->ui32DrcGainControlCompress = BRAP_P_FloatToQ230(psAacheAppFormatConfigParams->ui16DrcGainControlCompress);
    psAacheFwFormatConfigParam->ui32DrcGainControlBoost = BRAP_P_FloatToQ230(psAacheAppFormatConfigParams->ui16DrcGainControlBoost);
    psAacheFwFormatConfigParam->ui32DrcTargetLevel = psAacheAppFormatConfigParams->ui16DrcTargetLevel;
    psAacheFwFormatConfigParam->ui32FreqExtensionEnable = psAacheAppFormatConfigParams->bEnable96KhzDecode;
    

    /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32AacDrcGainControlCompress = psAacheAppFormatConfigParams->ui16DrcGainControlCompress;
    psDecSettings->sUserConfigAppFormat.i32AacDrcGainControlBoost = psAacheAppFormatConfigParams->ui16DrcGainControlBoost;


    /* TODO: Program i32NumOutPorts properly when support is added for concurrent stereo downmix */
    psAacheFwFormatConfigParam->i32NumOutPorts = 2;

    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {
        psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = psDecConfigParams->bOutputLfeOn;	
        BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
        psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = ui32OutputMode ;

        /* For Concurrent Downmix */
        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(psDecConfigParams->eDualMonoMode,
        (unsigned int *)&(psAacheFwFormatConfigParam->sUserOutputCfg[i].i32DualMode));

        psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled = psAacheAppFormatConfigParams->bExtDownmixEnable;

        for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count] = psAacheAppFormatConfigParams->ui16ExtDnMixCoeffs[row][count];
            }
        }
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psAacheFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);  
            psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = ui32OutputMode ;
            psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psAacheFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);  
            psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutMode = 2 ;          
            psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe = false;    
        } 
    }

    psAacheFwFormatConfigParam->i32DownmixType = psAacheAppFormatConfigParams->eDownmixType;
    psAacheFwFormatConfigParam->ui32SbrUserFlag= false;    
    psAacheFwFormatConfigParam->ui32DownmixCoefScaleIndex = psAacheAppFormatConfigParams->ui16DownmixCoefScaleIndex;
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Aache :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" i32NumOutPorts = %d",psAacheFwFormatConfigParam->i32NumOutPorts));
    BDBG_MSG((" i32DownmixType = %d",psAacheFwFormatConfigParam->i32DownmixType));
    BDBG_MSG((" i32AribMatrixMixdownIndex = %d",psAacheFwFormatConfigParam->i32AribMatrixMixdownIndex));
    BDBG_MSG((" ui32DrcGainControlCompress = %d -> ui32DrcGainControlCompress = 0x%x",psAacheAppFormatConfigParams->ui16DrcGainControlCompress,psAacheFwFormatConfigParam->ui32DrcGainControlCompress));
    BDBG_MSG((" ui32DrcGainControlBoost = %d -> ui32DrcGainControlBoost = 0x%x",psAacheAppFormatConfigParams->ui16DrcGainControlBoost,psAacheFwFormatConfigParam->ui32DrcGainControlBoost));
    BDBG_MSG((" ui32DrcTargetLevel = %d",psAacheFwFormatConfigParam->ui32DrcTargetLevel));
    BDBG_MSG((" ui32FreqExtensionEnable = %d",psAacheFwFormatConfigParam->ui32FreqExtensionEnable));

    BDBG_MSG(("------------------"));
    for(i = 0; i < BRAP_DEC_DownmixPath_eMax; i++)
    {
        BDBG_MSG((" i32OutLfe = %d",psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutLfe));
        BDBG_MSG((" i32OutMode = %d",psAacheFwFormatConfigParam->sUserOutputCfg[i].i32OutMode));
        BDBG_MSG((" i32ExtDnmixEnabled = %d",psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixEnabled));
		BDBG_MSG(("------------------"));
        for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
        {
            for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
            {	
                
				BDBG_MSG((" i32ExtDnmixTab[%d][%d]=%d",row,count,psAacheFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[row][count]));
            }
        }
		BDBG_MSG(("------------------"));
		for (row=0; row < BRAP_AF_P_MAX_CHANNELS; row++)
		{
			BDBG_MSG((" ui32OutputChannelMatrix[%d]=0x%x",row,psAacheFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix[row]));
		}
		BDBG_MSG(("------------------"));
	}
    BDBG_MSG(("------------------"));
    BDBG_MSG((" i32PcmBoost6dB = %d",psAacheFwFormatConfigParam->i32PcmBoost6dB));
    BDBG_MSG((" i32PcmBoostMinus4p75dB = %d",psAacheFwFormatConfigParam->i32PcmBoostMinus4p75dB));
    BDBG_MSG((" ui32SbrUserFlag = %d",psAacheFwFormatConfigParam->ui32SbrUserFlag));
    BDBG_MSG((" ui32DownmixCoefScaleIndex  = %d",psAacheFwFormatConfigParam->ui32DownmixCoefScaleIndex ));
    BDBG_MSG(("-------------------------------------------"));     

    BDBG_LEAVE(BRAP_P_MapAacheUserConfigApptoFw);
    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapDolbyPulseUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
	BRAP_DSPCHN_AacSbrConfigParams    *psDolbyPulseAppFormatConfigParams;
    BRAP_FWIF_P_DolbyPulseUserConfig  *psDolbyPulseFwFormatConfigParam;   
	unsigned int row = 0, count = 0;

    BDBG_ENTER(BRAP_P_MapDolbyPulseUserConfigFwtoApp);

    psDolbyPulseAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAacSbrConfigParams);
    psDolbyPulseFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDolbyPulseConfigParam);

    psDolbyPulseAppFormatConfigParams->bMsUsageMode = psDecSettings->sUserConfigAppFormat.bMsUsageMode;
    psDolbyPulseAppFormatConfigParams->bMpegConformanceMode = psDecSettings->sUserConfigAppFormat.bMpegConformanceMode;    

    psDecConfigParams->bOutputLfeOn = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32OutLfe;	
    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32OutMode,&psDecConfigParams->eOutputMode);
    BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32DualMode,
        &(psDecConfigParams->eDualMonoMode));

    /* Returning these values which already stored in App format */
    psDolbyPulseAppFormatConfigParams->ui16DrcGainControlCompress = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32DrcCut;
    psDolbyPulseAppFormatConfigParams->ui16DrcGainControlBoost = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32DrcBoost;
    
    switch(psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32LoRoDownmix)
    {
        case 0:
            psDolbyPulseAppFormatConfigParams->eDownmixType = BRAP_DSPCHN_AACSbr_DownmixType_eLtRt;                 
            break;
        case 1:
            psDolbyPulseAppFormatConfigParams->eDownmixType = BRAP_DSPCHN_AACSbr_DownmixType_eLoRo;                 
            break;
        case 2:
            psDolbyPulseAppFormatConfigParams->eDownmixType = BRAP_DSPCHN_AACSbr_DownmixType_eArib;                                
            break;
        default:
            BDBG_MSG(("Downmix type %d not supported under MS10/MS11 license. Forcing it to LtRt Downmix",
            psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32LoRoDownmix));                            
            psDolbyPulseAppFormatConfigParams->eDownmixType = BRAP_DSPCHN_AACSbr_DownmixType_eLtRt;                               
            break;                                               
    }
    
    psDolbyPulseAppFormatConfigParams->eCompMode = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32RfMode;

    /* Return Invalid value in the params that are not applicable for Dolby Pulse */
    psDolbyPulseAppFormatConfigParams->ui16DrcTargetLevel = 0;
    psDolbyPulseAppFormatConfigParams->bOutputLfeOn = false;
    psDolbyPulseAppFormatConfigParams->bExtDownmixEnable = false;
    psDolbyPulseAppFormatConfigParams->bEnable96KhzDecode = false;
    for (row = 0; row < DDP_DEC_GBL_MAXPCMCHANS; row++)
    {
    	for (count = 0; count < DDP_DEC_GBL_MAXPCMCHANS; count++)
    	{	
            psDolbyPulseAppFormatConfigParams->ui16ExtDnMixCoeffs[row][count] = 0;
	    }
    }

    psDolbyPulseAppFormatConfigParams->ui16DrcTargetLevel = psDolbyPulseFwFormatConfigParam->ui32RefDialnormLevel;
    psDolbyPulseAppFormatConfigParams->ui16DrcDefaultLevel = psDolbyPulseFwFormatConfigParam->ui32DefDialnormLevel;
    psDolbyPulseAppFormatConfigParams->bApplyGain = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].ui32ApplyGain;
    psDolbyPulseAppFormatConfigParams->ui32GainFactor = psDolbyPulseFwFormatConfigParam->sOutPortCfg[0].i32GainFactor;    

    BDBG_LEAVE(BRAP_P_MapDolbyPulseUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapDolbyPulseUserConfigApptoFw(
                    BRAP_ChannelHandle      hRapCh,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_DSPCHN_AacSbrConfigParams      *psDolbyPulseAppFormatConfigParams;
    BRAP_FWIF_P_DolbyPulseUserConfig    *psDolbyPulseFwFormatConfigParam;
    unsigned int i=0,j=0,ui32OutputMode=0;
    BRAP_P_DecoderSettings              *psDecSettings = &(hRapCh->sDecSettings);    

    BDBG_ENTER(BRAP_P_MapDolbyPulseUserConfigApptoFw);

    psDolbyPulseAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sAacSbrConfigParams);
    psDolbyPulseFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDolbyPulseConfigParam);

    if((BRAP_P_State_eStarted == hRapCh->eState) &&
       ((psDecSettings->sUserConfigAppFormat.bMsUsageMode != 
                psDolbyPulseAppFormatConfigParams->bMsUsageMode) ||
        (psDecSettings->sUserConfigAppFormat.bMpegConformanceMode != 
                psDolbyPulseAppFormatConfigParams->bMpegConformanceMode))
      )
    {
        BDBG_ERR(("Error!!! DDP User Config parameters 'bMsUsageMode' and "
                "'bMpegConformanceMode' cannot be changed on the fly"));
        return BERR_NOT_SUPPORTED;
        }
    
        psDecSettings->sUserConfigAppFormat.bMsUsageMode =     psDolbyPulseAppFormatConfigParams->bMsUsageMode ;
    psDecSettings->sUserConfigAppFormat.bMpegConformanceMode = psDolbyPulseAppFormatConfigParams->bMpegConformanceMode;        
        
    BRAP_P_SetDolbyMsUsageMode(hRapCh, &psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode);

    BRAP_P_SetDolbyMsDecoderType(hRapCh, psDolbyPulseFwFormatConfigParam->\
            eDolbyPulseUsageMode, &psDolbyPulseFwFormatConfigParam->eDecoderType);                
        
    BDBG_MSG(("psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode = %d",
                psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode));    

    /* Defaulted to 2, updated accordingly for MS10/MS11 in BRAP_P_SetDolbyMsDecoderOutputCfg() */
    psDolbyPulseFwFormatConfigParam->ui32NumOutPorts = 2;

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode, &ui32OutputMode);
    psDolbyPulseFwFormatConfigParam->ui32RefDialnormLevel = psDolbyPulseAppFormatConfigParams->ui16DrcTargetLevel;
    psDolbyPulseFwFormatConfigParam->ui32DefDialnormLevel = psDolbyPulseAppFormatConfigParams->ui16DrcDefaultLevel;        

    for(i = 0; i < psDolbyPulseFwFormatConfigParam->ui32NumOutPorts; i++)
    {    
        psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DrcBoost = psDolbyPulseAppFormatConfigParams->ui16DrcGainControlBoost;

        psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DrcCut =psDolbyPulseAppFormatConfigParams->ui16DrcGainControlCompress;

        psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32ApplyGain = \
            psDolbyPulseAppFormatConfigParams->bApplyGain;

        psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].i32GainFactor = \
            psDolbyPulseAppFormatConfigParams->ui32GainFactor;        

        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(psDecConfigParams->eDualMonoMode,
                (unsigned int *)&(psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DualMode));

        switch(psDolbyPulseAppFormatConfigParams->eDownmixType)
        {
            case BRAP_DSPCHN_AACSbr_DownmixType_eLtRt:
                psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32LoRoDownmix = 0;                 
                break;
            case BRAP_DSPCHN_AACSbr_DownmixType_eLoRo:
                psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32LoRoDownmix = 1;                 
                break;
            case BRAP_DSPCHN_AACSbr_DownmixType_eArib:
                psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32LoRoDownmix = 2;                                
                break;
            default:
                BDBG_MSG(("Downmix type %d not supported under MS10/MS11 license. Forcing it to LtRt Downmix",
                psDolbyPulseAppFormatConfigParams->eDownmixType));                    
                psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32LoRoDownmix = 0;                               
                break;                                               
        }

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutMode = ui32OutputMode ;
            psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutLfe = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutputChannelMatrix,false);       
            psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutMode = 2 ;          
            psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutLfe = false;            
        }           

        psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32RfMode = \
        psDolbyPulseAppFormatConfigParams->eCompMode;
    }

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DOLBYPLUSE :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("eDolbyPulseUsageMode = %d",psDolbyPulseFwFormatConfigParam->eDolbyPulseUsageMode));
    BDBG_MSG(("eDecoderType = %x",psDolbyPulseFwFormatConfigParam->eDecoderType));
    BDBG_MSG(("ui32NumOutPorts = %d",psDolbyPulseFwFormatConfigParam->ui32NumOutPorts));
    BDBG_MSG(("ui32RefDialnormLevel = %x",psDolbyPulseFwFormatConfigParam->ui32RefDialnormLevel));
    BDBG_MSG(("ui32DefDialnormLevel = %x",psDolbyPulseFwFormatConfigParam->ui32DefDialnormLevel));

    for(i = 0; i < psDolbyPulseFwFormatConfigParam->ui32NumOutPorts; i++)
    {    
        BDBG_MSG(("------------------"));
        BDBG_MSG(("ui32OutMode = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutMode));
        BDBG_MSG(("ui32OutLfe = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutLfe));
        BDBG_MSG(("ui32DualMode = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DualMode));
        BDBG_MSG(("ui32LoRoDownmix = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32LoRoDownmix));
        BDBG_MSG(("ui32RfMode = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32RfMode));
        BDBG_MSG(("ui32DrcCut = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DrcCut));
        BDBG_MSG(("ui32DrcBoost = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32DrcBoost));
        BDBG_MSG(("ui32ApplyGain = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32ApplyGain));
        BDBG_MSG(("i32GainFactor = %x",psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].i32GainFactor));
        for(j=0; j < BRAP_AF_P_MAX_CHANNELS; j++)
        {
            BDBG_MSG(("ui32OutputChannelMatrix[%d] = 0x%x",j,psDolbyPulseFwFormatConfigParam->sOutPortCfg[i].ui32OutputChannelMatrix[j]));		
        }
        BDBG_MSG(("------------------"));		
    }
    BDBG_MSG(("--------------------------------------------"));	
    
    BDBG_LEAVE(BRAP_P_MapDolbyPulseUserConfigApptoFw);
    return BERR_SUCCESS;
}




BERR_Code BRAP_P_MapWmaStdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_FrameSyncConfigParams  *psFrameSyncConfigParam;   
    BRAP_FWIF_P_WmaConfigParams  *psWmaFwFormatConfigParam;   
    BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
     BRAP_DSPCHN_WmaStdConfigParams *psWmaStdAppFormatConfigParams;    
    BDBG_ENTER(BRAP_P_MapWmaStdUserConfigFwtoApp);

    psWmaFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sWmaStdConfigParam);
    psWmaStdAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sWmaStdConfigParams);    
    psFrameSyncConfigParam = &(psDecSettings->sUserConfigStruct.sFrameSyncConfigParams);

    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psWmaFwFormatConfigParam->ui32OutputMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;
    psWmaStdAppFormatConfigParams->eWmaIpType = psFrameSyncConfigParam->sAlgoSpecConfigStruct.eWMAIpType;      
    psWmaStdAppFormatConfigParams->eAsfPtsType= psFrameSyncConfigParam->sAlgoSpecConfigStruct.eAsfPtsType;        

    BDBG_LEAVE(BRAP_P_MapWmaStdUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapWmaStdUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_WmaConfigParams  *psWmaFwFormatConfigParam;
    
    unsigned int ui32OutputMode = 2,i=0;

    BDBG_ENTER(BRAP_P_MapWmaStdUserConfigApptoFw);

    psWmaFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sWmaStdConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
    psWmaFwFormatConfigParam->ui32OutputMode= ui32OutputMode ;

    BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psWmaFwFormatConfigParam->ui32OutputChannelMatrix,false);
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("WmaStd :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("ui32OutputMode = %d",psWmaFwFormatConfigParam->ui32OutputMode));
    BDBG_MSG(("decodeOnlyPatternFlag = %d",psWmaFwFormatConfigParam->decodeOnlyPatternFlag));
    BDBG_MSG(("decodePattern = %d",psWmaFwFormatConfigParam->decodePattern));
    BDBG_MSG(("--------------"));
    for(i=0;i<BRAP_AF_P_MAX_CHANNELS;i++)
    {
        BDBG_MSG(("ui32OutputChannelMatrix[%d] = 0x%x",i,psWmaFwFormatConfigParam->ui32OutputChannelMatrix[i]));
    }
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_LEAVE(BRAP_P_MapWmaStdUserConfigApptoFw);
    return BERR_SUCCESS;
}




BERR_Code BRAP_P_MapWmaProUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_WmaProConfigParams  *psWmaProFwFormatConfigParam;   
    BRAP_DSPCHN_WmaProConfigParams *psWmaProAppFormatConfigParams;    
    BRAP_FWIF_P_FrameSyncConfigParams  *psFrameSyncConfigParam;          
    BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;	
    BDBG_ENTER(BRAP_P_MapWmaProUserConfigFwtoApp);

    psWmaProAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sWmaProConfigParams);    
    psWmaProFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sWmaProConfigParam);
    psFrameSyncConfigParam = &(psDecSettings->sUserConfigStruct.sFrameSyncConfigParams);
        

    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psWmaProFwFormatConfigParam->sOutputCfg[0].ui32OutMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;

    psWmaProAppFormatConfigParams->bDrcFlag = psWmaProFwFormatConfigParam->sOutputCfg[0].ui32DRCEnable;
    psWmaProAppFormatConfigParams->eDrcSettings = psWmaProFwFormatConfigParam->sOutputCfg[0].eDRCSetting;
    psWmaProAppFormatConfigParams->uiRmsAmplitudeRef = psWmaProFwFormatConfigParam->sOutputCfg[0].i32RmsAmplitudeRef;
    psWmaProAppFormatConfigParams->uiPeakAmplitudeRef = psWmaProFwFormatConfigParam->sOutputCfg[0].i32PeakAmplitudeRef;
    psWmaProAppFormatConfigParams->uiDesiredRms = psWmaProFwFormatConfigParam->sOutputCfg[0].i32DesiredRms;
    psWmaProAppFormatConfigParams->uiDesiredPeak = psWmaProFwFormatConfigParam->sOutputCfg[0].i32DesiredPeak;
    
    psDecConfigParams->bOutputLfeOn = psWmaProFwFormatConfigParam->sOutputCfg[0].ui32OutLfe;
    psWmaProAppFormatConfigParams->eStereoMode = psWmaProFwFormatConfigParam->sOutputCfg[0].ui32Stereomode;       
    psWmaProAppFormatConfigParams->eDecodeMode = psWmaProFwFormatConfigParam->ui32UsageMode;    
    psWmaProAppFormatConfigParams->eAsfPtsType= psFrameSyncConfigParam->sAlgoSpecConfigStruct.eAsfPtsType;        
    
    
    BDBG_LEAVE(BRAP_P_MapWmaProUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapWmaProUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_WmaProConfigParams  *psWmaProFwFormatConfigParam;
    BRAP_DSPCHN_WmaProConfigParams *psWmaProAppFormatConfigParams;
    unsigned int i = 0, j = 0, ui32OutputMode = 2;

    BDBG_ENTER(BRAP_P_MapWmaProUserConfigApptoFw);

    psWmaProAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sWmaProConfigParams);    
    psWmaProFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sWmaProConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);

    psWmaProFwFormatConfigParam->ui32NumOutports = BRAP_DEC_DownmixPath_eMax;

    for(i = 0; i < psWmaProFwFormatConfigParam->ui32NumOutports ; i++)
    {
        psWmaProFwFormatConfigParam->sOutputCfg[i].ui32DRCEnable= false;
        psWmaProFwFormatConfigParam->sOutputCfg[i].eDRCSetting = psWmaProAppFormatConfigParams->eDrcSettings;
        psWmaProFwFormatConfigParam->sOutputCfg[i].i32RmsAmplitudeRef = psWmaProAppFormatConfigParams->uiRmsAmplitudeRef;
        psWmaProFwFormatConfigParam->sOutputCfg[i].i32PeakAmplitudeRef = psWmaProAppFormatConfigParams->uiPeakAmplitudeRef;
        psWmaProFwFormatConfigParam->sOutputCfg[i].i32DesiredRms = psWmaProAppFormatConfigParams->uiDesiredRms;
        psWmaProFwFormatConfigParam->sOutputCfg[i].i32DesiredPeak = psWmaProAppFormatConfigParams->uiDesiredPeak;        
        psWmaProFwFormatConfigParam->sOutputCfg[i].ui32Stereomode=(uint32_t)psWmaProAppFormatConfigParams->eStereoMode;

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutMode = ui32OutputMode ;
            psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutLfe = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutputChannelMatrix,false);       
            psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutMode = 2 ;          
            psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutLfe = false;            
        }
    }

    psWmaProFwFormatConfigParam->ui32UsageMode = (uint32_t)psWmaProAppFormatConfigParams->eDecodeMode;
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("WmaPro :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("ui32NumOutports = %d",psWmaProFwFormatConfigParam->ui32NumOutports));
    BDBG_MSG(("-------------------"));
    for(i = 0; i < psWmaProFwFormatConfigParam->ui32NumOutports ; i++)
    {
        BDBG_MSG((" ui32DRCEnable = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].ui32DRCEnable));
        BDBG_MSG((" eDRCSetting = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].eDRCSetting)); 
        BDBG_MSG((" i32RmsAmplitudeRef = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].i32RmsAmplitudeRef));
        BDBG_MSG((" i32PeakAmplitudeRef = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].i32PeakAmplitudeRef));
        BDBG_MSG((" i32DesiredRms = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].i32DesiredRms));
        BDBG_MSG((" i32DesiredPeak = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].i32DesiredPeak));
        BDBG_MSG((" ui32OutMode = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutMode));
		BDBG_MSG((" ui32OutLfe = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutLfe));
		BDBG_MSG((" ui32Stereomode = %d",psWmaProFwFormatConfigParam->sOutputCfg[i].ui32Stereomode));        
        for(j=0; j < BRAP_AF_P_MAX_CHANNELS; j++)
        {
            BDBG_MSG((" ui32OutputChannelMatrix[%d] = %d",j,psWmaProFwFormatConfigParam->sOutputCfg[i].ui32OutputChannelMatrix[j]));
        }
    }
    BDBG_MSG(("-------------------"));
    BDBG_MSG(("ui32UsageMode = %d",psWmaProFwFormatConfigParam->ui32UsageMode));
    BDBG_MSG(("--------------------------------------------"));	

    BDBG_LEAVE(BRAP_P_MapWmaProUserConfigApptoFw);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapDtsBroadcastUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_DtsHdConfigParams  *psDtsBroadcastFwFormatConfigParam;   
    BRAP_DSPCHN_DtsBroadcastConfigParams *psDtsBroadcastAppFormatConfigParams;         
    BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;
    unsigned int i=0,j=0;
    BDBG_ENTER(BRAP_P_MapDtsBroadcastUserConfigFwtoApp);

    psDtsBroadcastAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sDtsBroadcastConfigParams);    
    psDtsBroadcastFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDtsBroadcastConfigParam);
        

    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32OutMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;
    psDecConfigParams->eDualMonoMode = psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32DualMode;
    psDecConfigParams->bOutputLfeOn= psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32OutLfe;    

    
    psDtsBroadcastAppFormatConfigParams->bEnableDRC = psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].i32UserDRCFlag;
    psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleHi = psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleHigh;
    psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleLo =psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleLow;
    psDtsBroadcastAppFormatConfigParams->eStereoMode = psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32StereoMode;

    BRAP_P_ConvertSrToEnum(psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32AppSampleRate,&(psDtsBroadcastAppFormatConfigParams->eAppSampleRate));
        
    psDtsBroadcastAppFormatConfigParams->bDecodeCoreOnly = psDtsBroadcastFwFormatConfigParam->ui32DecodeCoreOnly;
    psDtsBroadcastAppFormatConfigParams->bDecodeDtsOnly = psDtsBroadcastFwFormatConfigParam->ui32DecodeDtsOnly;
    psDtsBroadcastAppFormatConfigParams->bDecodeXll = psDtsBroadcastFwFormatConfigParam->ui32DecodeXLL;
    psDtsBroadcastAppFormatConfigParams->bDecodeX96 = psDtsBroadcastFwFormatConfigParam->ui32DecodeX96;
    psDtsBroadcastAppFormatConfigParams->bDecodeXch = psDtsBroadcastFwFormatConfigParam->ui32DecodeXCH;
    psDtsBroadcastAppFormatConfigParams->bDecodeXxch = psDtsBroadcastFwFormatConfigParam->ui32DecodeXXCH;
    psDtsBroadcastAppFormatConfigParams->bDecodeXbr = psDtsBroadcastFwFormatConfigParam->ui32DecodeXBR;
    psDtsBroadcastAppFormatConfigParams->bUseUserDownmixCoeff = psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].ui32ExtdnmixEnabled;
    psDtsBroadcastAppFormatConfigParams->bEnableSpkrRemapping = psDtsBroadcastFwFormatConfigParam->ui32EnableSpkrRemapping;
    psDtsBroadcastAppFormatConfigParams->ui32SpkrOut = psDtsBroadcastFwFormatConfigParam->ui32SpkrOut;
    psDtsBroadcastAppFormatConfigParams->bMixLFE2Primary = psDtsBroadcastFwFormatConfigParam->ui32MixLFE2Primary;
    psDtsBroadcastAppFormatConfigParams->ui32ChReplacementSet = psDtsBroadcastFwFormatConfigParam->ui32ChReplacementSet;    
    psDtsBroadcastAppFormatConfigParams->bEnableMetadataProcessing = psDtsBroadcastFwFormatConfigParam->ui32EnableMetadataProcessing;

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {	
            psDtsBroadcastAppFormatConfigParams->i32UserDownmixCoeff[i][j] = psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[0].i32ExtDnmixTab[i][j]  ;
        }
    }

    BDBG_LEAVE(BRAP_P_MapDtsBroadcastUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapDtsHdUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_DtsHdConfigParams  *psDtsHdFwFormatConfigParam;   
    BRAP_DSPCHN_DtsHdConfigParams  *psDtsHdAppFormatConfigParams;         
    BRAP_OutputMode	                eOutputMode = BRAP_OutputMode_e2_0;
    unsigned int i=0,j=0;
    BDBG_ENTER(BRAP_P_MapDtsHdUserConfigFwtoApp);

    psDtsHdAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sDtsHdConfigParams);    
    psDtsHdFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDtsHdConfigParam);
        

    BRAP_DSPCHN_P_GetOutputModeFwtoApp(psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32OutMode,&eOutputMode);
    psDecConfigParams->eOutputMode	 =  eOutputMode ;
    psDecConfigParams->eDualMonoMode = psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32DualMode;
    psDecConfigParams->bOutputLfeOn= psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32OutLfe;    

    
    psDtsHdAppFormatConfigParams->bEnableDRC = psDtsHdFwFormatConfigParam->sUserOutputCfg[0].i32UserDRCFlag;
    psDtsHdAppFormatConfigParams->ui16DynRngScaleHi = psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleHigh;
    psDtsHdAppFormatConfigParams->ui16DynRngScaleLo =psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleLow;
    psDtsHdAppFormatConfigParams->eStereoMode = psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32StereoMode;

    BRAP_P_ConvertSrToEnum(psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32AppSampleRate,&(psDtsHdAppFormatConfigParams->eAppSampleRate));
        
    psDtsHdAppFormatConfigParams->bDecodeCoreOnly = psDtsHdFwFormatConfigParam->ui32DecodeCoreOnly;
    psDtsHdAppFormatConfigParams->bDecodeDtsOnly = psDtsHdFwFormatConfigParam->ui32DecodeDtsOnly;
    psDtsHdAppFormatConfigParams->bDecodeXll = psDtsHdFwFormatConfigParam->ui32DecodeXLL;
    psDtsHdAppFormatConfigParams->bDecodeX96 = psDtsHdFwFormatConfigParam->ui32DecodeX96;
    psDtsHdAppFormatConfigParams->bDecodeXch = psDtsHdFwFormatConfigParam->ui32DecodeXCH;
    psDtsHdAppFormatConfigParams->bDecodeXxch = psDtsHdFwFormatConfigParam->ui32DecodeXXCH;
    psDtsHdAppFormatConfigParams->bDecodeXbr = psDtsHdFwFormatConfigParam->ui32DecodeXBR;
    psDtsHdAppFormatConfigParams->bUseUserDownmixCoeff = psDtsHdFwFormatConfigParam->sUserOutputCfg[0].ui32ExtdnmixEnabled;
    psDtsHdAppFormatConfigParams->bEnableSpkrRemapping = psDtsHdFwFormatConfigParam->ui32EnableSpkrRemapping;
    psDtsHdAppFormatConfigParams->ui32SpkrOut = psDtsHdFwFormatConfigParam->ui32SpkrOut;
    psDtsHdAppFormatConfigParams->bMixLFE2Primary = psDtsHdFwFormatConfigParam->ui32MixLFE2Primary;
    psDtsHdAppFormatConfigParams->ui32ChReplacementSet = psDtsHdFwFormatConfigParam->ui32ChReplacementSet;    
    psDtsHdAppFormatConfigParams->bEnableMetadataProcessing = psDtsHdFwFormatConfigParam->ui32EnableMetadataProcessing;

    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {	
            psDtsHdAppFormatConfigParams->i32UserDownmixCoeff[i][j] = psDtsHdFwFormatConfigParam->sUserOutputCfg[0].i32ExtDnmixTab[i][j]  ;
        }
    }

    BDBG_LEAVE(BRAP_P_MapDtsHdUserConfigFwtoApp);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapDtsBroadcastUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_DtsHdConfigParams  *psDtsBroadcastFwFormatConfigParam;
    BRAP_DSPCHN_DtsBroadcastConfigParams *psDtsBroadcastAppFormatConfigParams;
    unsigned int i = 0, ui32OutputMode = 2,j=0,k=0,l=0;

    BDBG_ENTER(BRAP_P_MapDtsBroadcastUserConfigApptoFw);

    psDtsBroadcastAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sDtsBroadcastConfigParams);    
    psDtsBroadcastFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDtsBroadcastConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);

    psDtsBroadcastFwFormatConfigParam->i32NumOutPorts = BRAP_DEC_DownmixPath_eMax;
    psDtsBroadcastFwFormatConfigParam->ui32DecodeCoreOnly = psDtsBroadcastAppFormatConfigParams->bDecodeCoreOnly;    
    psDtsBroadcastFwFormatConfigParam->ui32DecodeDtsOnly = psDtsBroadcastAppFormatConfigParams->bDecodeDtsOnly;        
    psDtsBroadcastFwFormatConfigParam->ui32DecodeXLL = psDtsBroadcastAppFormatConfigParams->bDecodeXll;
    psDtsBroadcastFwFormatConfigParam->ui32DecodeX96 = psDtsBroadcastAppFormatConfigParams->bDecodeX96;
    psDtsBroadcastFwFormatConfigParam->ui32DecodeXCH = psDtsBroadcastAppFormatConfigParams->bDecodeXch;
    psDtsBroadcastFwFormatConfigParam->ui32DecodeXXCH = psDtsBroadcastAppFormatConfigParams->bDecodeXxch;
    psDtsBroadcastFwFormatConfigParam->ui32DecodeXBR = psDtsBroadcastAppFormatConfigParams->bDecodeXbr;
    psDtsBroadcastFwFormatConfigParam->ui32EnableSpkrRemapping = psDtsBroadcastAppFormatConfigParams->bEnableSpkrRemapping;
    psDtsBroadcastFwFormatConfigParam->ui32SpkrOut = psDtsBroadcastAppFormatConfigParams->ui32SpkrOut;
    psDtsBroadcastFwFormatConfigParam->ui32MixLFE2Primary = psDtsBroadcastAppFormatConfigParams->bMixLFE2Primary;
    psDtsBroadcastFwFormatConfigParam->ui32ChReplacementSet = psDtsBroadcastAppFormatConfigParams->ui32ChReplacementSet;
#ifdef RAP_DTSMETADATA_SUPPORT    
    psDtsBroadcastFwFormatConfigParam->ui32EnableMetadataProcessing = psDtsBroadcastAppFormatConfigParams->bEnableMetadataProcessing;
#else
    /* Metadata support not present for DTS DMP license */
    psDtsBroadcastFwFormatConfigParam->ui32EnableMetadataProcessing = false;
#endif
    
    for(i = 0; i < psDtsBroadcastFwFormatConfigParam->i32NumOutPorts ; i++)
    {
        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].i32UserDRCFlag = psDtsBroadcastAppFormatConfigParams->bEnableDRC;
        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleHi, 100);
        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleLo, 100);
        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32DualMode = psDecConfigParams->eDualMonoMode;
        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32StereoMode = psDtsBroadcastAppFormatConfigParams->eStereoMode;

        BRAP_P_ConvertSR(psDtsBroadcastAppFormatConfigParams->eAppSampleRate,&(psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32AppSampleRate));

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode = ui32OutputMode ;
            psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);       
            psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode = 2 ;          
            psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = false;            
        }

        psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].ui32ExtdnmixEnabled = psDtsBroadcastAppFormatConfigParams->bUseUserDownmixCoeff;

        for (j = 0; j < 8; j++)
        {
            for (k = 0; k < 8; k++)
            {	
                psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[j][k] = psDtsBroadcastAppFormatConfigParams->i32UserDownmixCoeff[j][k];
            }
        }        
    }
            /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleHigh= psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleHi;
    psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleLow= psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleLo;

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DtsBroadcast :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32DecodeCoreOnly=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeCoreOnly));
    BDBG_MSG(("ui32DecodeDtsOnly=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeDtsOnly));
    BDBG_MSG(("ui32DecodeXLL=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeXLL));
    BDBG_MSG(("ui32DecodeX96=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeX96));
    BDBG_MSG(("ui32DecodeXCH=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeXCH));
    BDBG_MSG(("ui32DecodeXXCH=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeXXCH));
    BDBG_MSG(("ui32DecodeXBR=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeXBR));
    BDBG_MSG(("ui32DecodeXCH=%d",psDtsBroadcastFwFormatConfigParam->ui32DecodeXCH));
    BDBG_MSG(("ui32EnableSpkrRemapping=%d",psDtsBroadcastFwFormatConfigParam->ui32EnableSpkrRemapping));

    BDBG_MSG(("ui32SpkrOut=%d",psDtsBroadcastFwFormatConfigParam->ui32SpkrOut));
    BDBG_MSG(("ui32MixLFE2Primary=%d",psDtsBroadcastFwFormatConfigParam->ui32MixLFE2Primary));
    BDBG_MSG(("ui32ChReplacementSet=%d",psDtsBroadcastFwFormatConfigParam->ui32ChReplacementSet));
    BDBG_MSG(("i32NumOutPorts=%d",psDtsBroadcastFwFormatConfigParam->i32NumOutPorts));
    BDBG_MSG(("ui32EnableMetadataProcessing=%d",psDtsBroadcastFwFormatConfigParam->ui32EnableMetadataProcessing));
    BDBG_MSG(("------------------"));
    for(j=0;j<2;j++)
    {
        BDBG_MSG(("i32UserDRCFlag=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32UserDRCFlag));
        BDBG_MSG(("ui16DynRngScaleHi=0x%x -> i32DynScaleHigh= 0x%x",psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleHi,psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("ui16DynRngScaleLo=0x%x -> i32DynScaleLow = 0x%x",psDtsBroadcastAppFormatConfigParams->ui16DynRngScaleLo,psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleLow));

        BDBG_MSG(("ui32OutMode=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32OutMode));
        BDBG_MSG(("ui32OutLfe=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32OutLfe));
        BDBG_MSG(("ui32DualMode=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32DualMode));
        BDBG_MSG(("ui32StereoMode=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32StereoMode));
        BDBG_MSG(("ui32AppSampleRate=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32AppSampleRate));
        BDBG_MSG(("------------------"));

        for(k=0;k<BRAP_AF_P_MAX_CHANNELS;k++)
        {
            BDBG_MSG(("i32DynScaleHigh=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32OutputChannelMatrix[k]));
        }
        BDBG_MSG(("------------------"));
        
        BDBG_MSG(("ui32ExtdnmixEnabled=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].ui32ExtdnmixEnabled));
        BDBG_MSG(("i32DynScaleHigh=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("i32DynScaleHigh=%d",psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("------------------"));
        for(k=0;k<BRAP_AF_P_MAX_CHANNELS;k++)
        {   
            for(l=0;l<BRAP_AF_P_MAX_CHANNELS;l++)
            {
                BDBG_MSG(("i32ExtDnmixTab[%d][%d]=%d",k,l,psDtsBroadcastFwFormatConfigParam->sUserOutputCfg[j].i32ExtDnmixTab[k][l]));        
            }
        }
        BDBG_MSG(("------------------"));
    }
    BDBG_MSG(("-------------------------------------------"));
    BDBG_LEAVE(BRAP_P_MapWmaProUserConfigApptoFw);
    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapDtsHdUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_DtsHdConfigParams  *psDtsHdFwFormatConfigParam;
    BRAP_DSPCHN_DtsHdConfigParams  *psDtsHdAppFormatConfigParams;
    unsigned int i = 0, ui32OutputMode = 2,j=0,k=0,l=0;

    BDBG_ENTER(BRAP_P_MapDtsHdUserConfigApptoFw);

    psDtsHdAppFormatConfigParams = &(psDecConfigParams->uConfigParams.sDtsHdConfigParams);    
    psDtsHdFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDtsHdConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);

    psDtsHdFwFormatConfigParam->i32NumOutPorts = BRAP_DEC_DownmixPath_eMax;
    psDtsHdFwFormatConfigParam->ui32DecodeCoreOnly = psDtsHdAppFormatConfigParams->bDecodeCoreOnly;    
    psDtsHdFwFormatConfigParam->ui32DecodeDtsOnly = psDtsHdAppFormatConfigParams->bDecodeDtsOnly;        
    psDtsHdFwFormatConfigParam->ui32DecodeXLL = psDtsHdAppFormatConfigParams->bDecodeXll;
    psDtsHdFwFormatConfigParam->ui32DecodeX96 = psDtsHdAppFormatConfigParams->bDecodeX96;
    psDtsHdFwFormatConfigParam->ui32DecodeXCH = psDtsHdAppFormatConfigParams->bDecodeXch;
    psDtsHdFwFormatConfigParam->ui32DecodeXXCH = psDtsHdAppFormatConfigParams->bDecodeXxch;
    psDtsHdFwFormatConfigParam->ui32DecodeXBR = psDtsHdAppFormatConfigParams->bDecodeXbr;
    psDtsHdFwFormatConfigParam->ui32EnableSpkrRemapping = psDtsHdAppFormatConfigParams->bEnableSpkrRemapping;
    psDtsHdFwFormatConfigParam->ui32SpkrOut = psDtsHdAppFormatConfigParams->ui32SpkrOut;
    psDtsHdFwFormatConfigParam->ui32MixLFE2Primary = psDtsHdAppFormatConfigParams->bMixLFE2Primary;
    psDtsHdFwFormatConfigParam->ui32ChReplacementSet = psDtsHdAppFormatConfigParams->ui32ChReplacementSet;
#ifdef RAP_DTSMETADATA_SUPPORT        
    psDtsHdFwFormatConfigParam->ui32EnableMetadataProcessing = psDtsHdAppFormatConfigParams->bEnableMetadataProcessing;
#else
    /* Metadata support not present for DTS DMP license */
    psDtsHdFwFormatConfigParam->ui32EnableMetadataProcessing = false;
#endif
    
    for(i = 0; i < psDtsHdFwFormatConfigParam->i32NumOutPorts ; i++)
    {
        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].i32UserDRCFlag = psDtsHdAppFormatConfigParams->bEnableDRC;
        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleHigh = BRAP_P_FloatToQ131(psDtsHdAppFormatConfigParams->ui16DynRngScaleHi, 100);
        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].i32DynScaleLow = BRAP_P_FloatToQ131(psDtsHdAppFormatConfigParams->ui16DynRngScaleLo, 100);
        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32DualMode = psDecConfigParams->eDualMonoMode;
        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32StereoMode = psDtsHdAppFormatConfigParams->eStereoMode;

        BRAP_P_ConvertSR(psDtsHdAppFormatConfigParams->eAppSampleRate,&(psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32AppSampleRate));

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode = ui32OutputMode ;
            psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = psDecConfigParams->bOutputLfeOn;            
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);       
            psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode = 2 ;          
            psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = false;            
        }

        psDtsHdFwFormatConfigParam->sUserOutputCfg[i].ui32ExtdnmixEnabled = psDtsHdAppFormatConfigParams->bUseUserDownmixCoeff;

        for (j = 0; j < 8; j++)
        {
            for (k = 0; k < 8; k++)
            {	
                psDtsHdFwFormatConfigParam->sUserOutputCfg[i].i32ExtDnmixTab[j][k] = psDtsHdAppFormatConfigParams->i32UserDownmixCoeff[j][k];
            }
        }        
    }
            /* Storing these values in App format as well */
    psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleHigh= psDtsHdAppFormatConfigParams->ui16DynRngScaleHi;
    psDecSettings->sUserConfigAppFormat.i32DtsBroadcastDynScaleLow= psDtsHdAppFormatConfigParams->ui16DynRngScaleLo;

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DtsHd :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32DecodeCoreOnly=%d",psDtsHdFwFormatConfigParam->ui32DecodeCoreOnly));
    BDBG_MSG(("ui32DecodeDtsOnly=%d",psDtsHdFwFormatConfigParam->ui32DecodeDtsOnly));
    BDBG_MSG(("ui32DecodeXLL=%d",psDtsHdFwFormatConfigParam->ui32DecodeXLL));
    BDBG_MSG(("ui32DecodeX96=%d",psDtsHdFwFormatConfigParam->ui32DecodeX96));
    BDBG_MSG(("ui32DecodeXCH=%d",psDtsHdFwFormatConfigParam->ui32DecodeXCH));
    BDBG_MSG(("ui32DecodeXXCH=%d",psDtsHdFwFormatConfigParam->ui32DecodeXXCH));
    BDBG_MSG(("ui32DecodeXBR=%d",psDtsHdFwFormatConfigParam->ui32DecodeXBR));
    BDBG_MSG(("ui32DecodeXCH=%d",psDtsHdFwFormatConfigParam->ui32DecodeXCH));
    BDBG_MSG(("ui32EnableSpkrRemapping=%d",psDtsHdFwFormatConfigParam->ui32EnableSpkrRemapping));

    BDBG_MSG(("ui32SpkrOut=%d",psDtsHdFwFormatConfigParam->ui32SpkrOut));
    BDBG_MSG(("ui32MixLFE2Primary=%d",psDtsHdFwFormatConfigParam->ui32MixLFE2Primary));
    BDBG_MSG(("ui32ChReplacementSet=%d",psDtsHdFwFormatConfigParam->ui32ChReplacementSet));
    BDBG_MSG(("i32NumOutPorts=%d",psDtsHdFwFormatConfigParam->i32NumOutPorts));
    BDBG_MSG(("ui32EnableMetadataProcessing=%d",psDtsHdFwFormatConfigParam->ui32EnableMetadataProcessing));
    BDBG_MSG(("------------------"));
    for(j=0;j<2;j++)
    {
        BDBG_MSG(("i32UserDRCFlag=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32UserDRCFlag));
        BDBG_MSG(("ui16DynRngScaleHi = 0x%x -> i32DynScaleHigh=0x%x",psDtsHdAppFormatConfigParams->ui16DynRngScaleHi,psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("ui16DynRngScaleLo = 0x%x -> i32DynScaleLow=0x%x",psDtsHdAppFormatConfigParams->ui16DynRngScaleLo,psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleLow));
        BDBG_MSG(("ui32OutMode=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32OutMode));
        BDBG_MSG(("ui32OutLfe=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32OutLfe));
        BDBG_MSG(("ui32DualMode=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32DualMode));
        BDBG_MSG(("ui32StereoMode=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32StereoMode));
        BDBG_MSG(("ui32AppSampleRate=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32AppSampleRate));
        BDBG_MSG(("------------------"));

        for(k=0;k<BRAP_AF_P_MAX_CHANNELS;k++)
        {
            BDBG_MSG(("i32DynScaleHigh=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32OutputChannelMatrix[k]));
        }
        BDBG_MSG(("------------------"));
        
        BDBG_MSG(("ui32ExtdnmixEnabled=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].ui32ExtdnmixEnabled));
        BDBG_MSG(("i32DynScaleHigh=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("i32DynScaleHigh=%d",psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32DynScaleHigh));
        BDBG_MSG(("------------------"));
        for(k=0;k<BRAP_AF_P_MAX_CHANNELS;k++)
        {   
            for(l=0;l<BRAP_AF_P_MAX_CHANNELS;l++)
            {
                BDBG_MSG(("i32ExtDnmixTab[%d][%d]=%d",k,l,psDtsHdFwFormatConfigParam->sUserOutputCfg[j].i32ExtDnmixTab[k][l]));        
            }
        }
        BDBG_MSG(("------------------"));
    }
    BDBG_MSG(("-------------------------------------------"));

    BDBG_LEAVE(BRAP_P_MapDtsHdUserConfigApptoFw);
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapPcmWavUserConfigApptoFw(
                    BRAP_ChannelHandle          hRapCh,       
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_PcmWavConfigParams	        *psPcmWavFwFormatConfigParam;
    unsigned int i=0,ui32OutputMode = 2;
     BRAP_P_DecoderSettings          *psDecSettings = &(hRapCh->sDecSettings);    
 
    psPcmWavFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sPcmWavConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
    
    psPcmWavFwFormatConfigParam->ui32OutMode = ui32OutputMode;
    psPcmWavFwFormatConfigParam->ui32NumOutputPorts =1;
    BRAP_P_SetDolbyMsUsageMode(hRapCh, &psPcmWavFwFormatConfigParam->eDolbyMsUsageMode);
    
    /* LFE hardcoded to true(required for certification) since FW does not intend to support configurability for PCM Wav */
    BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psPcmWavFwFormatConfigParam->ui32OutputChannelMatrix,true);

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("PcmWav :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32NumOutputPorts=%d",psPcmWavFwFormatConfigParam->ui32NumOutputPorts));
    BDBG_MSG(("ui32OutMode=%d",psPcmWavFwFormatConfigParam->ui32OutMode));
    BDBG_MSG(("------------------"));
    for(i=0;i<BRAP_AF_P_MAX_CHANNELS;i++)
    {
        BDBG_MSG(("ui32OutputChannelMatrix[%d]=%d",i,psPcmWavFwFormatConfigParam->ui32OutputChannelMatrix[i]));
    }
    BDBG_MSG(("------------------"));

    BDBG_MSG(("eDolbyMsUsageMode=%d",psPcmWavFwFormatConfigParam->eDolbyMsUsageMode));
    BDBG_MSG(("-------------------------------------------"));
    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapPcmWavUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_PcmWavConfigParams  *psPcmWavFwFormatConfigParam;

    BDBG_ENTER(BRAP_P_MapPcmWavUserConfigFwtoApp);

    psPcmWavFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sPcmWavConfigParam);

    /* Hardcoding as of now till FW adds these two fields */
    psDecConfigParams->bOutputLfeOn = false;	
    psDecConfigParams->eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;

    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psPcmWavFwFormatConfigParam->ui32OutMode,&(psDecConfigParams->eOutputMode));       

    BDBG_LEAVE(BRAP_P_MapPcmWavUserConfigFwtoApp);

    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapDraUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_DraConfigParams *psDraFwFormatConfigParam;
    BRAP_DSPCHN_DraConfigParam  *psDraAppFormatConfigParams;
    unsigned int ui32OutputMode = 2,i=0,j=0;
 
    psDraFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDraConfigParam);
    psDraAppFormatConfigParams =&(psDecConfigParams->uConfigParams.sDraConfigParams);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
    
    psDraFwFormatConfigParam->ui32NumOutPorts=2;
    for(i = 0; i < psDraFwFormatConfigParam->ui32NumOutPorts; i++)
    {    
        psDraFwFormatConfigParam->sUserOutputCfg[i].ui32StereoMode= psDraAppFormatConfigParams->eStereoMode;
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = psDecConfigParams->bOutputLfeOn;
            psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode= ui32OutputMode;
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);
            psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe = false;
            psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode= 2;
        }        
    }
    
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Dra :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32NumOutputPorts=%d",psDraFwFormatConfigParam->ui32NumOutPorts));
    BDBG_MSG(("------------------"));

    for(i=0;i<2;i++)
    {
        BDBG_MSG(("ui32OutMode=%d",psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode));
        BDBG_MSG(("ui32OutLfe=%d",psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutLfe));
        BDBG_MSG(("ui32StereoMode=%d",psDraFwFormatConfigParam->sUserOutputCfg[i].ui32StereoMode));
        for(j=0;j<BRAP_AF_P_MAX_CHANNELS;j++)
        {
            BDBG_MSG(("ui32OutputChannelMatrix[%d]= 0x%x",j,psDraFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix[j]));
        }    
    }  
    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapDraUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_DraConfigParams *psDraFwFormatConfigParam;
    BRAP_DSPCHN_DraConfigParam  *psDraAppFormatConfigParams;

    BDBG_ENTER(BRAP_P_MapDraUserConfigFwtoApp);

    psDraFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sDraConfigParam);
    psDraAppFormatConfigParams =&(psDecConfigParams->uConfigParams.sDraConfigParams);    

    psDecConfigParams->bOutputLfeOn = psDraFwFormatConfigParam->sUserOutputCfg[0].ui32OutLfe;	
    psDecConfigParams->eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psDraFwFormatConfigParam->sUserOutputCfg[0].ui32OutMode,&(psDecConfigParams->eOutputMode));       

    psDraAppFormatConfigParams->eStereoMode = psDraFwFormatConfigParam->sUserOutputCfg[0].ui32StereoMode;

    BDBG_LEAVE(BRAP_P_MapDraUserConfigFwtoApp);

    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapRealAudioLbrUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_RalbrConfigParams   *psRealAudioLbrFwFormatConfigParam;
    unsigned int ui32OutputMode = 2,i=0,j=0;

    BDBG_ENTER(BRAP_P_MapRealAudioLbrUserConfigApptoFw);    
 
    psRealAudioLbrFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sRealAudioLbrConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);
    
    psRealAudioLbrFwFormatConfigParam->ui32NumOutPorts=2;
    for(i = 0; i < psRealAudioLbrFwFormatConfigParam->ui32NumOutPorts; i++)
    {    
        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,psDecConfigParams->bOutputLfeOn);
            psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode= ui32OutputMode;
        }
        else
        {
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix,false);
            psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode= 2;
        }        
    }    

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("RealAudioLbr :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32NumOutputPorts=%d",psRealAudioLbrFwFormatConfigParam->ui32NumOutPorts));
    BDBG_MSG(("------------------"));
    for(i=0;i<2;i++)
    {
        BDBG_MSG(("sUserOutputCfg[%d].ui32OutMode=%d",psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutMode));  
        for(j=0;j<BRAP_AF_P_MAX_CHANNELS;j++)
        {
            BDBG_MSG(("ui32OutputChannelMatrix[%d] = 0x%x",j,psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[i].ui32OutputChannelMatrix[j]));    
        }    
    }        
    BDBG_MSG(("-------------------------------------------"));
    BDBG_LEAVE(BRAP_P_MapRealAudioLbrUserConfigApptoFw);    
    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapRealAudioLbrUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_RalbrConfigParams   *psRealAudioLbrFwFormatConfigParam;

    BDBG_ENTER(BRAP_P_MapRealAudioLbrUserConfigFwtoApp);

    psRealAudioLbrFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sRealAudioLbrConfigParam);

    psDecConfigParams->eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
    
    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psRealAudioLbrFwFormatConfigParam->sUserOutputCfg[0].ui32OutMode,&(psDecConfigParams->eOutputMode));       

    BDBG_LEAVE(BRAP_P_MapRealAudioLbrUserConfigFwtoApp);

    return BERR_SUCCESS;
}



BERR_Code BRAP_P_MapAmrUserConfigApptoFw(
                    BRAP_P_DecoderSettings          *psDecSettings,        
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */    
)
{
    BRAP_FWIF_P_AmrConfigParams	        *psAmrFwFormatConfigParam;
    unsigned int i =0,j=0, ui32OutputMode = 2;

    BDBG_ENTER(BRAP_P_MapAmrUserConfigApptoFw);

    psAmrFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sAmrConfigParam);

    BRAP_DSPCHN_P_GetOutputModeApptoFw(psDecConfigParams->eOutputMode,&ui32OutputMode);

    psAmrFwFormatConfigParam->ui32NumOutPorts =1;

    for(i = 0; i < psAmrFwFormatConfigParam->ui32NumOutPorts; i++)
    {
        psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32OutMode = ui32OutputMode;
        psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32ScaleOp = 0;
        psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32ScaleIdx = 0;    
        BRAP_P_GetOutputChannelmatrix(psDecConfigParams->eOutputMode,psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32OutputChannelMatrix,false);
    }
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Amr :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("ui32NumOutputPorts=%d",psAmrFwFormatConfigParam->ui32NumOutPorts));
    for(i=0;i<2;i++)
    {
        BDBG_MSG(("ui32OutMode=%d",psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32OutMode));
        BDBG_MSG(("ui32ScaleOp=%d",psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32ScaleOp));
        BDBG_MSG(("ui32ScaleIdx=%d",psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32ScaleIdx));
        for(j=0;j<BRAP_AF_P_MAX_CHANNELS;j++)
        {
            BDBG_MSG(("ui32OutputChannelMatrix[j] = 0x%x",j,psAmrFwFormatConfigParam->sUsrOutputCfg[i].ui32OutputChannelMatrix[j]));
        }               
        
    }
    BDBG_LEAVE(BRAP_P_MapAmrUserConfigApptoFw);
    
    return BERR_SUCCESS;
}

BERR_Code BRAP_P_MapAmrUserConfigFwtoApp(
                    BRAP_P_DecoderSettings          *psDecSettings,            
                    BRAP_DEC_ConfigParams	*psDecConfigParams	/* [in] Decoder configuration
												   parameters */ 
)
{
    BRAP_FWIF_P_AmrConfigParams  *psAmrFwFormatConfigParam;

    BDBG_ENTER(BRAP_P_MapAmrUserConfigFwtoApp);

    psAmrFwFormatConfigParam = &(psDecSettings->sUserConfigStruct.sAmrConfigParam);

    /* Hardcoding as of now till FW adds these two fields */
    psDecConfigParams->bOutputLfeOn = false;	
    psDecConfigParams->eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono;

    BRAP_DSPCHN_P_GetOutputModeFwtoApp((unsigned int)psAmrFwFormatConfigParam->sUsrOutputCfg[0].ui32OutMode,&(psDecConfigParams->eOutputMode));

    BDBG_LEAVE(BRAP_P_MapAmrUserConfigFwtoApp);

    return BERR_SUCCESS;
}


BERR_Code BRAP_P_MapSrcUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize    
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_SRCUserConfigParams sSrcUserConfig;
    BDBG_ASSERT(hStage);
    BSTD_UNUSED(hStage);
    sSrcUserConfig.ui32DummySrc = hStage->sProcessingStageSettings.uConfigParams.sSRCParams.bDummy;
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sSrcUserConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Src :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("ui32DummySrc=%d",sSrcUserConfig.ui32DummySrc));
    BDBG_MSG(("-------------------------------------------"));

    return err;
}

BERR_Code BRAP_P_MapDsolaUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize    
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_DsolaConfigParams sDsolaUserConfig;
    BDBG_ASSERT(hStage);
    BSTD_UNUSED(hStage);
    sDsolaUserConfig.ui32InputPcmFrameSize = (512 * hStage->uHandle.hRapCh->uiPBRate)/BRAP_DSPCHN_PLAYBACKRATE_NORMAL;

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Dsola :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("ui32InputPcmFrameSize = %d",sDsolaUserConfig.ui32InputPcmFrameSize));
    BDBG_MSG(("-------------------------------------------"));

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sDsolaUserConfig),uiActualConfigSize);             
    return err;
}

BERR_Code BRAP_P_MapAdPanUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_AudioDescPanConfigParams sPanFwConfig;
    BRAP_DSPCHN_AdPanParams sPanUserConfig;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("Audio Descriptor (Pan) should  be part of channel specfic network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(hStage->sProcessingStageSettings.uConfigParams.sAdPanParams.uiAudioRampTimeInMs > 4000)
    {
        BDBG_WRN(("Ramp Time value %d is not supported, clipping it to 4000ms", 
            hStage->sProcessingStageSettings.uConfigParams.sAdPanParams.uiAudioRampTimeInMs));
        hStage->sProcessingStageSettings.uConfigParams.sAdPanParams.uiAudioRampTimeInMs = 4000;
    }    
    sPanUserConfig = hStage->sProcessingStageSettings.uConfigParams.sAdPanParams;

    /* Copy user Config params to Firmware params */
    sPanFwConfig.i32UserVolumeLevel = sPanUserConfig.uiVol;
    sPanFwConfig.eADChannelConfig = sPanUserConfig.bEnableAd;
    sPanFwConfig.ui32AudioRampTimeInMs = sPanUserConfig.uiAudioRampTimeInMs;        

    if((hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr != BRAP_RM_P_INVALID_INDEX)
        &&(BRAP_P_IsPointerValid((void *)hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr)) )
    {
        sPanFwConfig.ui32PanFadeInterfaceValidFlag =true;
    }
    else
    {
        sPanFwConfig.ui32PanFadeInterfaceValidFlag =false;
    }
    
    BRAP_P_ConvertAddressToOffset(hStage->hRap->hHeap, 
                (void *)hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr, 
                (void *)&(sPanFwConfig.ui32PanFadeInterfaceAddr));
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sPanFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("AdPan :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));
    BDBG_MSG(("i32UserVolumeLevel = %d",sPanFwConfig.i32UserVolumeLevel));
    BDBG_MSG(("ui32PanFadeInterfaceAddr = %d",sPanFwConfig.ui32PanFadeInterfaceAddr));
    BDBG_MSG(("eADChannelConfig = %d",sPanFwConfig.eADChannelConfig));
    BDBG_MSG(("ui32PanFadeInterfaceValidFlag = %d",sPanFwConfig.ui32PanFadeInterfaceValidFlag));
    BDBG_MSG(("ui32AudioRampTimeInMs = %d",sPanFwConfig.ui32AudioRampTimeInMs));
    BDBG_MSG(("-------------------------------------------"));

    return err;
}

BERR_Code BRAP_P_MapAdFadeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_AudioDescFadeConfigParams sFadeFwConfig;
    BDBG_ASSERT(hStage);
    BSTD_UNUSED(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("Audio Descriptor (Fade) should  be part of channel specfic network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    
    /* Copy user Config params to Firmware params */
    sFadeFwConfig.eFadeConfig = BRAP_FWIF_P_FadeConfigType_eFadedChOnly;
    sFadeFwConfig.ui32PanFadeInterfaceAddr = hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr;
    if((hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr != BRAP_RM_P_INVALID_INDEX)
        &&(BRAP_P_IsPointerValid((void *)hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr)))
    {
        sFadeFwConfig.ui32PanFadeInterfaceValidFlag =true;
        BRAP_P_DRAMWRITE(hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr, 0x7FFFFFFF);
    }
    else
    {
        sFadeFwConfig.ui32PanFadeInterfaceValidFlag =false;
    }
    BRAP_P_ConvertAddressToOffset(hStage->hRap->hHeap, 
                (void *)hStage->uHandle.hRapCh->hAdFade->ui32PanFadeInterfaceAddr, 
                (void *)&sFadeFwConfig.ui32PanFadeInterfaceAddr);
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sFadeFwConfig),uiActualConfigSize);             


    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("AdFade :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------"));

    BDBG_MSG(("eFadeConfig = %d",sFadeFwConfig.eFadeConfig));
    BDBG_MSG(("ui32PanFadeInterfaceAddr = %d",sFadeFwConfig.ui32PanFadeInterfaceAddr));
    BDBG_MSG(("ui32PanFadeInterfaceValidFlag = %d",sFadeFwConfig.ui32PanFadeInterfaceValidFlag));
    BDBG_MSG(("-------------------------------------------"));

    return err;
}    

BERR_Code BRAP_P_MapCustomVoiceUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;
    BDBG_ASSERT(hStage);

    /* Copy the whole user Config params structure to Firmware params structure */
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&hStage->sProcessingStageSettings.uConfigParams.sCustomVoiceConfigParams),uiActualConfigSize);             
    return err;
}

BERR_Code BRAP_P_MapCustomSurroundUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_CustomSurroundUserConfig sCustomSurroundFwConfig;
    BDBG_ASSERT(hStage);

    /* Copy user Config params to Firmware params */
    sCustomSurroundFwConfig.ui32enable = !(hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.bBypassCustomSurround);
    sCustomSurroundFwConfig.ui32combOn = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.bCombOn;
    sCustomSurroundFwConfig.ui32delay = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiDelay;
    sCustomSurroundFwConfig.ui32volume1 = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiVolume1;
    sCustomSurroundFwConfig.ui32volume2 = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiVolume2;
    sCustomSurroundFwConfig.ui32volume3 = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiVolume3;
    sCustomSurroundFwConfig.ui32volume4 = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiVolume4;
    sCustomSurroundFwConfig.ui32volume5 = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiVolume5;
    sCustomSurroundFwConfig.ui32lpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiLpfFc;
    sCustomSurroundFwConfig.ui32lpfQ = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiLpfQ;
    sCustomSurroundFwConfig.ui32lpfGain = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiLpfGain;
    sCustomSurroundFwConfig.ui32funcVol = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiFuncVol;
    sCustomSurroundFwConfig.ui32inputTrim = hStage->sProcessingStageSettings.uConfigParams.sCustomSurroundParams.uiTrim;
    
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sCustomSurroundFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("CustomSurround :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" ui32enable = %d",sCustomSurroundFwConfig.ui32enable));
    BDBG_MSG((" ui32combOn = %d",sCustomSurroundFwConfig.ui32combOn));
    BDBG_MSG((" ui32delay = %d",sCustomSurroundFwConfig.ui32delay));
    BDBG_MSG((" ui32volume1 = %d",sCustomSurroundFwConfig.ui32volume1));
    BDBG_MSG((" ui32volume2 = %d",sCustomSurroundFwConfig.ui32volume2));
    BDBG_MSG((" ui32volume3 = %d",sCustomSurroundFwConfig.ui32volume3));
    BDBG_MSG((" ui32volume4 = %d",sCustomSurroundFwConfig.ui32volume4));
    BDBG_MSG((" ui32volume5 = %d",sCustomSurroundFwConfig.ui32volume5));
    BDBG_MSG((" ui32lpfFc = %d",sCustomSurroundFwConfig.ui32lpfFc));
    BDBG_MSG((" ui32lpfQ = %d",sCustomSurroundFwConfig.ui32lpfQ));
    BDBG_MSG((" ui32lpfGain = %d",sCustomSurroundFwConfig.ui32lpfGain));
    BDBG_MSG((" ui32funcVol = %d",sCustomSurroundFwConfig.ui32funcVol));
    BDBG_MSG((" ui32inputTrim = %d",sCustomSurroundFwConfig.ui32inputTrim));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapCustomBassUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_CustomBassUserConfig sCustomBassFwConfig;
    BDBG_ASSERT(hStage);

    /* Copy user Config params to Firmware params */
    sCustomBassFwConfig.ui32enable = !(hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.bBypassCustomBass); 
    sCustomBassFwConfig.ui32operation = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.eCustomBassOperation; 
    sCustomBassFwConfig.ui32agcType = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.eCustomBassAgcType; 
    sCustomBassFwConfig.ui32harmonicsType = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.eCustomBassHarmonicsType; 
    sCustomBassFwConfig.ui32lpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiLpfFc; 
    sCustomBassFwConfig.ui32hpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiHpfFc; 
    sCustomBassFwConfig.ui32agcGainMax = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.iAgcGainMax; 
    sCustomBassFwConfig.ui32agcGainMin = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.iAgcGainMin; 
    sCustomBassFwConfig.ui32agcAttackTime = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiAgcAttackTime; 
    sCustomBassFwConfig.ui32agcRelTime = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiAgcRelTime; 
    sCustomBassFwConfig.ui32agcThreshold = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.iAgcThreshold; 
    sCustomBassFwConfig.ui32agcHpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiAgcHpfFc; 
    sCustomBassFwConfig.ui32harLevel = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiHarmonicsLevel; 
    sCustomBassFwConfig.ui32harLpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiHarmonicsLpfFc; 
    sCustomBassFwConfig.ui32harHpfFc = hStage->sProcessingStageSettings.uConfigParams.sCustomBassParams.uiHarmonicsHpfFc; 
    

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sCustomBassFwConfig),uiActualConfigSize);             
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("CustomBass :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" ui32enable  = %d",sCustomBassFwConfig.ui32enable ));
    BDBG_MSG((" ui32operation  = %d",sCustomBassFwConfig.ui32operation ));
    BDBG_MSG((" ui32agcType = %d",sCustomBassFwConfig.ui32agcType));
    BDBG_MSG((" ui32harmonicsType = %d",sCustomBassFwConfig.ui32harmonicsType));
    BDBG_MSG((" ui32lpfFc = %d",sCustomBassFwConfig.ui32lpfFc));
    BDBG_MSG((" ui32hpfFc = %d",sCustomBassFwConfig.ui32hpfFc));
    BDBG_MSG((" ui32agcGainMax = %d",sCustomBassFwConfig.ui32agcGainMax));
    BDBG_MSG((" ui32agcGainMin = %d",sCustomBassFwConfig.ui32agcGainMin));
    BDBG_MSG((" ui32agcAttackTime = %d",sCustomBassFwConfig.ui32agcAttackTime));
    BDBG_MSG((" ui32agcRelTime = %d",sCustomBassFwConfig.ui32agcRelTime));
    BDBG_MSG((" ui32agcThreshold  = %d",sCustomBassFwConfig.ui32agcThreshold ));
    BDBG_MSG((" ui32agcHpfFc = %d",sCustomBassFwConfig.ui32agcHpfFc));
    BDBG_MSG((" ui32harLevel = %d",sCustomBassFwConfig.ui32harLevel));
    BDBG_MSG((" ui32harLpfFc = %d",sCustomBassFwConfig.ui32harLpfFc));
    BDBG_MSG((" ui32harHpfFc = %d",sCustomBassFwConfig.ui32harHpfFc));
    BDBG_MSG(("-------------------------------------------")); 


    return err;
}

BERR_Code BRAP_P_MapPl2UserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_PL2ConfigParams sPl2FwConfig;
    BDBG_ASSERT(hStage);

    /* Copy user Config params to Firmware params */
    sPl2FwConfig.ui32enable = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bEnablePLII;
    sPl2FwConfig.ui32abalflg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bEnableAutoBal;
    sPl2FwConfig.ui32decmode = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.ePLllDecMode;

    sPl2FwConfig.ui32chancfg = 7;
    switch (hStage->sProcessingStageSettings.uConfigParams.sPl2Params.eOutputMode)
	{
		case BRAP_OutputMode_e3_0:
			sPl2FwConfig.ui32chancfg = 3;
			break;
		case BRAP_OutputMode_e2_2:
			sPl2FwConfig.ui32chancfg = 6;
			break;
		case BRAP_OutputMode_e3_2:
			sPl2FwConfig.ui32chancfg = 7;
			break;
		case BRAP_OutputMode_e1_0:
		case BRAP_OutputMode_e1_1:
		case BRAP_OutputMode_e2_0:
		case BRAP_OutputMode_e2_1:
		case BRAP_OutputMode_e3_1:
		default:
			break;
	}
    
    sPl2FwConfig.ui32cwidthcfg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.eCenterWidCfg;
    sPl2FwConfig.ui32dimcfg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.ui32DimSetting;
    sPl2FwConfig.ui32panoramaflg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bEnablePanorama;
    sPl2FwConfig.ui32sfiltflg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bEnableSurChnShelfFilt;
    sPl2FwConfig.ui32rspolinvflg = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bEnableRSurPolInv;
    sPl2FwConfig.ui32pcmscl = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sPl2Params.ui32PcmScaleFactor, 100);
    sPl2FwConfig.ui32debugOn = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.bDebugOnOff;
    sPl2FwConfig.ui32outputch = hStage->sProcessingStageSettings.uConfigParams.sPl2Params.eDbgChanCfg;
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sPl2FwConfig),uiActualConfigSize);             
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Pl2 :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" ui32enable  = %d",sPl2FwConfig.ui32enable ));
    BDBG_MSG((" ui32abalflg = %d",sPl2FwConfig.ui32abalflg));
    BDBG_MSG((" ui32decmode = %d",sPl2FwConfig.ui32decmode));
    BDBG_MSG((" ui32chancfg = %d",sPl2FwConfig.ui32chancfg));
    BDBG_MSG((" ui32cwidthcfg = %d",sPl2FwConfig.ui32cwidthcfg));
    BDBG_MSG((" ui32dimcfg = %d",sPl2FwConfig.ui32dimcfg));
    BDBG_MSG((" ui32panoramaflg = %d",sPl2FwConfig.ui32panoramaflg));
    BDBG_MSG((" ui32sfiltflg = %d",sPl2FwConfig.ui32sfiltflg));
    BDBG_MSG((" ui32rspolinvflg = %d",sPl2FwConfig.ui32rspolinvflg));
    BDBG_MSG((" ui32PcmScaleFactor = %d -> ui32pcmscl = %d",hStage->sProcessingStageSettings.uConfigParams.sPl2Params.ui32PcmScaleFactor,sPl2FwConfig.ui32pcmscl));
    BDBG_MSG((" ui32debugOn = %d",sPl2FwConfig.ui32debugOn));
    BDBG_MSG((" ui32outputch = %d",sPl2FwConfig.ui32outputch));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapSrsxtUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_TruSurrndXTConfigParams sSrsxtFwConfig;
    BDBG_ASSERT(hStage);

    /* Copy user Config params to Firmware params */
    sSrsxtFwConfig.ui32TSEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bTruSurroundEnable;
    sSrsxtFwConfig.ui32TSHeadphone = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bTsHeadphoneEnable;
    sSrsxtFwConfig.i32TruSurroundInputGain = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.uiTruSurroundInputGain, 200);
    sSrsxtFwConfig.ui32DialogClarityEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bDialogClarityEnable;    
    sSrsxtFwConfig.ui32DialogClarityLevel = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.uiDialogClarityLevel, 100);
    sSrsxtFwConfig.ui32TBEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bTruBassEnable;
    sSrsxtFwConfig.i32TBLevel = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bTbLevel, 100);
    sSrsxtFwConfig.eTBSpeakerSize = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.eTbSpeakerSize;
    sSrsxtFwConfig.ui32CertificationEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bCertificationEnable;
    sSrsxtFwConfig.eAcMode = hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.eAcMode;
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sSrsxtFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Srsxt :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("  ui32TSEnable = %d", sSrsxtFwConfig.ui32TSEnable));
    BDBG_MSG(("  ui32TSHeadphone = %d", sSrsxtFwConfig.ui32TSHeadphone));
    BDBG_MSG(("  uiTruSurroundInputGain = 0x%x -> i32TruSurroundInputGain = 0x%x", hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.uiTruSurroundInputGain,sSrsxtFwConfig.i32TruSurroundInputGain));
    BDBG_MSG(("  ui32DialogClarityEnable = %d", sSrsxtFwConfig.ui32DialogClarityEnable));
    BDBG_MSG(("  ui32DialogClarityLevel = 0x%x-> ui32DialogClarityLevel = 0x%x",hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.uiDialogClarityLevel, sSrsxtFwConfig.ui32DialogClarityLevel));
    BDBG_MSG(("  ui32TBEnable = %d", sSrsxtFwConfig.ui32TBEnable));
    BDBG_MSG(("  bTbLevel = 0x%x -> i32TBLevel = 0x%x", hStage->sProcessingStageSettings.uConfigParams.sSrsXtParams.bTbLevel,sSrsxtFwConfig.i32TBLevel));
    BDBG_MSG(("  eTBSpeakerSize = %d", sSrsxtFwConfig.eTBSpeakerSize));
    BDBG_MSG(("  ui32CertificationEnableFlag = %d", sSrsxtFwConfig.ui32CertificationEnableFlag));
    BDBG_MSG(("  eAcMode = %d", sSrsxtFwConfig.eAcMode));
    BDBG_MSG(("-------------------------------------------")); 

    return err;
}

BERR_Code BRAP_P_MapSrshdUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_TruSurrndHDConfigParams sSrsHdFwConfig;
    BRAP_DSPCHN_TruSurroundHdParams     sSrsHdFwConfigAppFormat;
    BDBG_ASSERT(hStage);

    sSrsHdFwConfigAppFormat = hStage->sProcessingStageSettings.uConfigParams.sSrsHdParams;
        
    /* Copy user Config params to Firmware params */
    sSrsHdFwConfig.sTopLevelConfig.i32IsStudioSound = sSrsHdFwConfigAppFormat.sTopLevelParams.bIsStudioSound;
    sSrsHdFwConfig.sTopLevelConfig.i32StudioSoundMode = sSrsHdFwConfigAppFormat.sTopLevelParams.eStudioSoundMode;
    sSrsHdFwConfig.sTopLevelConfig.i32mEnable = sSrsHdFwConfigAppFormat.sTopLevelParams.bEnable;
    sSrsHdFwConfig.sTopLevelConfig.i32mInputGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.sTopLevelParams.ui32InputGain,100);
    sSrsHdFwConfig.sTopLevelConfig.i32mHeadroomGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.sTopLevelParams.ui32HeadroomGain,100);
    sSrsHdFwConfig.sTopLevelConfig.i32mInputMode = sSrsHdFwConfigAppFormat.sTopLevelParams.eInputMode;
    sSrsHdFwConfig.sTopLevelConfig.i32mOutputGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.sTopLevelParams.ui32OutputGain,100);    
    sSrsHdFwConfig.sTopLevelConfig.i32mBypassGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.sTopLevelParams.ui32BypassGain,100);    
    sSrsHdFwConfig.i32TruSurrndHDEnableFlag = sSrsHdFwConfigAppFormat.bEnable;
    sSrsHdFwConfig.i32HeadPhoneEnableFlag = sSrsHdFwConfigAppFormat.bHeadPhoneEnable;
    sSrsHdFwConfig.i32TruSurrndHDInputGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32InputGain, 100);
    sSrsHdFwConfig.i32TruSurrndHDOutputGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32OutputGain, 100);    
    sSrsHdFwConfig.i32TruSurrndHDByPassGain = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32ByPassGain, 100);
    sSrsHdFwConfig.eSubCrossOverFreq = sSrsHdFwConfigAppFormat.eSubCrossOverFreq;
    sSrsHdFwConfig.i32TruBassFrontEnableFlag = sSrsHdFwConfigAppFormat.bTruBassFrontEnable;
    sSrsHdFwConfig.i32TruBassFrontCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32TruBassFrontLevel, 100);
    sSrsHdFwConfig.eTruBassFrontSpeakerSize = sSrsHdFwConfigAppFormat.eTruBassFrontSpeakerSize;    
    sSrsHdFwConfig.eSRSTruBassProcessMode = sSrsHdFwConfigAppFormat.eSRSTruBassProcessMode;
    sSrsHdFwConfig.i32DefinitionFrontEnableFlag = sSrsHdFwConfigAppFormat.bDefinitionFrontEnable;
    sSrsHdFwConfig.i32DefinitionFrontCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32DefinitionFrontLevel, 100);
    sSrsHdFwConfig.i32DialogClarityEnableFlag = sSrsHdFwConfigAppFormat.bDialogClarityEnable;
    sSrsHdFwConfig.i32DialogClarityCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32DialogClarityLevel, 100);
    sSrsHdFwConfig.i32SurroundLevel = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32SurroundLevel, 100);
    sSrsHdFwConfig.i32WowHDSRS3DEnableFlag = sSrsHdFwConfigAppFormat.bWowHDSRS3DEnable;
    sSrsHdFwConfig.i32SRS3DHighBitRateFlag = sSrsHdFwConfigAppFormat.bSRS3DHighBitRateEnable;
    sSrsHdFwConfig.eWowHDSRS3DMode = sSrsHdFwConfigAppFormat.eWowHDSRS3DMode;
    sSrsHdFwConfig.i32WowHDSRS3DSpaceCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32WowHDSRS3DSpaceLevel, 100);
    sSrsHdFwConfig.i32WowHDSRS3DCenterCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32WowHDSRS3DCenterLevel, 100);
    sSrsHdFwConfig.i32WowHDFocusEnableFlag = sSrsHdFwConfigAppFormat.bWowHDFocusEnable;
    sSrsHdFwConfig.i32WowHDFocusCtrl = BRAP_P_FloatToQ131(sSrsHdFwConfigAppFormat.ui32WowHDFocusLevel, 100);
    sSrsHdFwConfig.i32Mono2StereoEnableFlag = sSrsHdFwConfigAppFormat.bMonoToStereoEnable;
    sSrsHdFwConfig.eOutputAcMode = sSrsHdFwConfigAppFormat.eOutputAcMode;
    sSrsHdFwConfig.i32OuputLFEEnableFlag = sSrsHdFwConfigAppFormat.bOutputLFEEnable;
    sSrsHdFwConfig.i32CertificationEnableFlag = sSrsHdFwConfigAppFormat.bCertificationApp;
    sSrsHdFwConfig.i32InputLFEPresentFlag = sSrsHdFwConfigAppFormat.bInputLfeEnable;
    sSrsHdFwConfig.eInputAcMode = sSrsHdFwConfigAppFormat.eInputAcmod;
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sSrsHdFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Srsxt :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 

    BDBG_MSG((" i32IsStudioSound  = %d",sSrsHdFwConfig.sTopLevelConfig.i32IsStudioSound ));
    BDBG_MSG((" i32StudioSoundMode = %d",sSrsHdFwConfig.sTopLevelConfig.i32StudioSoundMode));
    BDBG_MSG((" i32mEnable = %d",sSrsHdFwConfig.sTopLevelConfig.i32mEnable));
    BDBG_MSG((" ui32InputGain = 0x%x -> i32mInputGain = 0x%x",sSrsHdFwConfigAppFormat.sTopLevelParams.ui32InputGain,sSrsHdFwConfig.sTopLevelConfig.i32mInputGain));
	BDBG_MSG((" ui32HeadroomGain = 0x%x -> i32mHeadroomGain = 0x%x", sSrsHdFwConfigAppFormat.sTopLevelParams.ui32HeadroomGain,sSrsHdFwConfig.sTopLevelConfig.i32mHeadroomGain));
    BDBG_MSG((" i32mInputMode = %d",sSrsHdFwConfig.sTopLevelConfig.i32mInputMode));
	BDBG_MSG((" ui32OutputGain = 0x%x -> i32mOutputGain = 0x%x",sSrsHdFwConfigAppFormat.sTopLevelParams.ui32OutputGain,sSrsHdFwConfig.sTopLevelConfig.i32mOutputGain));
	BDBG_MSG((" ui32BypassGain = 0x%x -> i32mBypassGain = 0x%x",sSrsHdFwConfigAppFormat.sTopLevelParams.ui32BypassGain,sSrsHdFwConfig.sTopLevelConfig.i32mBypassGain));
    BDBG_MSG((" i32TruSurrndHDEnableFlag = %d",sSrsHdFwConfig.i32TruSurrndHDEnableFlag));
    BDBG_MSG((" i32HeadPhoneEnableFlag = %d",sSrsHdFwConfig.i32HeadPhoneEnableFlag));
    BDBG_MSG((" ui32InputGain = 0x%x -> i32TruSurrndHDInputGain = 0x%x",sSrsHdFwConfigAppFormat.ui32InputGain,sSrsHdFwConfig.i32TruSurrndHDInputGain));
    BDBG_MSG((" ui32OutputGain = 0x%x -> i32TruSurrndHDOutputGain = 0x%x",sSrsHdFwConfigAppFormat.ui32OutputGain, sSrsHdFwConfig.i32TruSurrndHDOutputGain));
    BDBG_MSG((" ui32ByPassGain = 0x%x -> i32TruSurrndHDByPassGain = 0x%x",sSrsHdFwConfigAppFormat.ui32ByPassGain,sSrsHdFwConfig.i32TruSurrndHDByPassGain));
    BDBG_MSG((" eSubCrossOverFreq = %d",sSrsHdFwConfig.eSubCrossOverFreq));
    BDBG_MSG((" i32TruBassFrontEnableFlag = %d",sSrsHdFwConfig.i32TruBassFrontEnableFlag));
    BDBG_MSG((" ui32TruBassFrontLevel = 0x%x -> i32TruBassFrontCtrl = %d",sSrsHdFwConfigAppFormat.ui32TruBassFrontLevel, sSrsHdFwConfig.i32TruBassFrontCtrl));
    BDBG_MSG((" eTruBassFrontSpeakerSize = %d",sSrsHdFwConfig.eTruBassFrontSpeakerSize));
    BDBG_MSG((" eSRSTruBassProcessMode = %d",sSrsHdFwConfig.eSRSTruBassProcessMode));
    BDBG_MSG((" i32DefinitionFrontEnableFlag = %d",sSrsHdFwConfig.i32DefinitionFrontEnableFlag));
	BDBG_MSG((" ui32DefinitionFrontLevel = 0x%x -> i32DefinitionFrontCtrl = %d",sSrsHdFwConfigAppFormat.ui32DefinitionFrontLevel, sSrsHdFwConfig.i32DefinitionFrontCtrl));	
    BDBG_MSG((" i32DialogClarityEnableFlag = %d",sSrsHdFwConfig.i32DialogClarityEnableFlag));
    BDBG_MSG((" ui32DialogClarityLevel = 0x%x -> i32DialogClarityCtrl = 0x%x",sSrsHdFwConfigAppFormat.ui32DialogClarityLevel, sSrsHdFwConfig.i32DialogClarityCtrl));
    BDBG_MSG((" ui32SurroundLevel = 0x%x -> i32SurroundLevel = 0x%x",sSrsHdFwConfigAppFormat.ui32SurroundLevel,sSrsHdFwConfig.i32SurroundLevel));
    BDBG_MSG((" i32WowHDSRS3DEnableFlag = %d",sSrsHdFwConfig.i32WowHDSRS3DEnableFlag));
    BDBG_MSG((" i32SRS3DHighBitRateFlag = %d",sSrsHdFwConfig.i32SRS3DHighBitRateFlag));
    BDBG_MSG((" eWowHDSRS3DMode = %d",sSrsHdFwConfig.eWowHDSRS3DMode));
    BDBG_MSG((" ui32WowHDSRS3DSpaceLevel = 0x%x -> i32WowHDSRS3DSpaceCtrl = %d",sSrsHdFwConfigAppFormat.ui32WowHDSRS3DSpaceLevel,sSrsHdFwConfig.i32WowHDSRS3DSpaceCtrl));
	BDBG_MSG((" ui32WowHDSRS3DCenterLevel = 0x%x -> i32WowHDSRS3DCenterCtrl = %d",sSrsHdFwConfigAppFormat.ui32WowHDSRS3DCenterLevel,sSrsHdFwConfig.i32WowHDSRS3DCenterCtrl));
    BDBG_MSG((" i32WowHDFocusEnableFlag = %d",sSrsHdFwConfig.i32WowHDFocusEnableFlag));
    BDBG_MSG((" ui32WowHDFocusLevel = 0x%x -> i32WowHDFocusCtrl = %d",sSrsHdFwConfigAppFormat.ui32WowHDFocusLevel,sSrsHdFwConfig.i32WowHDFocusCtrl));
    BDBG_MSG((" i32Mono2StereoEnableFlag = %d",sSrsHdFwConfig.i32Mono2StereoEnableFlag));
    BDBG_MSG((" eOutputAcMode = %d",sSrsHdFwConfig.eOutputAcMode));
    BDBG_MSG((" i32OuputLFEEnableFlag = %d",sSrsHdFwConfig.i32OuputLFEEnableFlag));
    BDBG_MSG((" i32CertificationEnableFlag = %d",sSrsHdFwConfig.i32CertificationEnableFlag));
    BDBG_MSG((" i32InputLFEPresentFlag = %d",sSrsHdFwConfig.i32InputLFEPresentFlag));
    BDBG_MSG((" eInputAcMode = %d",sSrsHdFwConfig.eInputAcMode));
    BDBG_MSG(("-------------------------------------------"));

    
    return err;
}

BERR_Code BRAP_P_MapSrsTruVolumeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_TruVolumeUserConfig sSrsTruVolumeFwConfig;
    BDBG_ASSERT(hStage);

    /* Copy user Config params to Firmware params */
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32IsStudioSound = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.bIsStudioSound;
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32StudioSoundMode = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.eStudioSoundMode;
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.bEnable;
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mInputGain = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32InputGain,100);
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mHeadroomGain = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32HeadroomGain,100);
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mInputMode = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.eInputMode;
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mOutputGain = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32OutputGain,100);    
    sSrsTruVolumeFwConfig.sTopLevelConfig.i32mBypassGain = BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sTopLevelParams.ui32BypassGain,100);
    sSrsTruVolumeFwConfig.i32TruVolume_enable = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bSrsTruVolumeEnable;
    sSrsTruVolumeFwConfig.i32nchans = 2;
    sSrsTruVolumeFwConfig.i32blockSize = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize;
    sSrsTruVolumeFwConfig.i32mEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalGain;
    sSrsTruVolumeFwConfig.i32mInputGain = BRAP_P_ConvertPIToFwFormat(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiInputGain, 25, 800, 0x00200000);
    sSrsTruVolumeFwConfig.i32mOutputGain = BRAP_P_ConvertPIToFwFormat(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiOutputGain, 25, 100, 0x00200000);
    sSrsTruVolumeFwConfig.i32mBypassGain = BRAP_P_FloatToQ923(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiBypassGain, 100);
    sSrsTruVolumeFwConfig.i32mReferenceLevel = BRAP_P_FloatToQ923(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiReferenceLevel, 100);
    sSrsTruVolumeFwConfig.i32EnableDCNotchFilter = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableDCNotchFilter;
    sSrsTruVolumeFwConfig.i32mMode = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.eMode;
    sSrsTruVolumeFwConfig.i32mSize = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution;
    sSrsTruVolumeFwConfig.i32mMaxGain = BRAP_P_ConvertPIToFwFormat(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain, 4, 1024, 0x00008000);
    sSrsTruVolumeFwConfig.i32mNoiseManager = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNoiseManager;   
    sSrsTruVolumeFwConfig.i32mNoiseManagerThreshold = BRAP_P_FloatToQ923(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiNoiseManagerThreshold, 200);
    sSrsTruVolumeFwConfig.i32mCalibrate = BRAP_P_ConvertPIToFwFormat(hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiCalibrate, 100, 25600, 0x00008000);
    sSrsTruVolumeFwConfig.i32mNormalizerEnable= hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalizer;       
    sSrsTruVolumeFwConfig.sHighPassFilterConfig.ui32mEnable = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.bEnable;
    sSrsTruVolumeFwConfig.sHighPassFilterConfig.ui32CoefGenMode = hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode;
    
    if(0 == hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode)
    {
        if(sizeof(BRAP_DSPCHN_HPFilterCoef) != sizeof(BRAP_FWIF_P_FilterCoefHpf))
        {
            BDBG_ERR(("SRS_TVOL: High Pass Filter Coefficients' structure sizes don't match between PI and FW"));
            return BERR_INVALID_PARAMETER;
        }
        
        BKNI_Memcpy((void *)(volatile void *)(sSrsTruVolumeFwConfig.sHighPassFilterConfig.uHPFSettings.sFilterCoefHpf),
                            (void *)(&hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterCoef),
                            sizeof(BRAP_FWIF_P_FilterCoefHpf)*3);
    }
    else
    {
        sSrsTruVolumeFwConfig.sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32CutoffFrequency = 
            hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency;
        sSrsTruVolumeFwConfig.sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32Order = 
            hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder;
    }

    BDBG_MSG (("SRS-Tru Volume :: FW CONFIG <-> USER CONFIG MAPPING")); 
    BDBG_MSG (("--------------------------------------------")); 
    BDBG_MSG (("i32TruVolume_enable = %u <-> bSrsTruVolumeEnable = %d",sSrsTruVolumeFwConfig.i32TruVolume_enable,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bSrsTruVolumeEnable));
    BDBG_MSG (("i32nchans = %u <-> uiNumChannels = NONE",sSrsTruVolumeFwConfig.i32nchans));
    BDBG_MSG (("i32blockSize = %u <-> uiBlockSize = %u",sSrsTruVolumeFwConfig.i32blockSize,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiBlockSize));
    BDBG_MSG (("i32mEnable = %u <-> bEnableNormalGain = %d",sSrsTruVolumeFwConfig.i32mEnable,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalGain));
    BDBG_MSG (("i32mInputGain = %x <-> uiInputGain = %u",sSrsTruVolumeFwConfig.i32mInputGain,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiInputGain));
    BDBG_MSG (("i32mOutputGain = %x <-> uiOutputGain = %u",sSrsTruVolumeFwConfig.i32mOutputGain,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiOutputGain));
    BDBG_MSG (("i32mBypassGain = %x <-> uiBypassGain = %u",sSrsTruVolumeFwConfig.i32mBypassGain,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiBypassGain));            
    BDBG_MSG (("i32mReferenceLevel = %x <-> uiReferenceLevel = %d",sSrsTruVolumeFwConfig.i32mReferenceLevel,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiReferenceLevel));
    BDBG_MSG (("i32EnableDCNotchFilter = %u <-> bEnableDCNotchFilter = %d",sSrsTruVolumeFwConfig.i32EnableDCNotchFilter,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableDCNotchFilter));
    BDBG_MSG (("i32mMode = %u <-> eMode = %d",sSrsTruVolumeFwConfig.i32mMode,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.eMode));                        
    BDBG_MSG (("i32mSize = %u <-> eSpeakerResolution = %d",sSrsTruVolumeFwConfig.i32mSize,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.eSpeakerResolution));                            
    BDBG_MSG (("i32mMaxGain = %x <-> uiMaxGain = %u",sSrsTruVolumeFwConfig.i32mMaxGain,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiMaxGain));                                
    BDBG_MSG (("i32mNoiseManager = %u <-> bEnableNoiseManager = %d",sSrsTruVolumeFwConfig.i32mNoiseManager,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNoiseManager));                                
    BDBG_MSG (("i32mNoiseManagerThreshold = %x <-> uiNoiseManagerThreshold = %u",sSrsTruVolumeFwConfig.i32mNoiseManagerThreshold,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiNoiseManagerThreshold));
    BDBG_MSG (("i32mCalibrate = %x <-> uiCalibrate = %u",sSrsTruVolumeFwConfig.i32mCalibrate,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.uiCalibrate));
    BDBG_MSG (("i32mNormalizerEnable = %d <-> bEnableNormalizer = %d",sSrsTruVolumeFwConfig.i32mNoiseManagerThreshold,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.bEnableNormalizer));    
    BDBG_MSG (("sHighPassFilterConfig.ui32mEnable = %x <-> sHPFParams.bEnable = %d",sSrsTruVolumeFwConfig.sHighPassFilterConfig.ui32mEnable,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.bEnable));
    BDBG_MSG (("sHighPassFilterConfig.ui32CoefGenMode = %x <-> sHPFParams.eCoefGenMode = %d",
                sSrsTruVolumeFwConfig.sHighPassFilterConfig.ui32CoefGenMode,hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode));
    if(0 == hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.eCoefGenMode)
    {
        BDBG_MSG(("Filter coefficients are provided by the user. Refer to corresponding DRAM for the verification"));    
    }
    else
    {
        BDBG_MSG (("sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32CutoffFrequency = %x <-> sHPFParams.eCoefGenMode = %d",
                    sSrsTruVolumeFwConfig.sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32CutoffFrequency,
                    hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency));
        BDBG_MSG (("sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32Order = %x <-> sHPFParams.eFilterOrder = %d",
                    sSrsTruVolumeFwConfig.sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32Order,
                    hStage->sProcessingStageSettings.uConfigParams.sSrsTruVolumeParams.sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder));
    }
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sSrsTruVolumeFwConfig),uiActualConfigSize);             
    return err;
}


BERR_Code BRAP_P_MapDolbyVolUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_DolbyVolumeUserConfig   sDolbyVolumeFwConfig;
    BDBG_ASSERT(hStage);

    sDolbyVolumeFwConfig.i32DolbyVolumeEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bDolbyVolEna;
    sDolbyVolumeFwConfig.i32BlockSize = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iBlockSize;
    sDolbyVolumeFwConfig.i32nBands = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiBands;
    sDolbyVolumeFwConfig.i32nChans = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiNumInputOutputChannel;
    sDolbyVolumeFwConfig.i32InputReferenceLevel = (int32_t)BRAP_P_FloatToQ824((int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiInputReferenceLevel,127);
    sDolbyVolumeFwConfig.i32OutputReferenceLevel = (int32_t)BRAP_P_FloatToQ824((int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiOutputReferenceLevel,127);
    sDolbyVolumeFwConfig.i32Calibration = (int32_t)BRAP_P_FloatToQ824((int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iCalibration,127);
    sDolbyVolumeFwConfig.i32VlmEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bVolModelerEna;
    sDolbyVolumeFwConfig.i32ResetNowFlag = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bResetNowFlag;
    sDolbyVolumeFwConfig.i32DigitalVolumeLevel = (int32_t)BRAP_P_FloatToQ824((int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iDigitalVolumeLevel,127);
    sDolbyVolumeFwConfig.i32AnalogVolumeLevel = (int32_t)BRAP_P_FloatToQ824((int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iAnalogVolumeLevel,127);
    sDolbyVolumeFwConfig.i32LvlAmount = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiVolLvlAmount;
    sDolbyVolumeFwConfig.i32LvlEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bVolLvlEnable;
    sDolbyVolumeFwConfig.i32EnableMidsideProc = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bEnableMidsideProc;
    sDolbyVolumeFwConfig.i32HalfmodeFlag = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bHalfmodeEna;    
    sDolbyVolumeFwConfig.i32LimiterEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.bLimiterEnable;    

 
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sDolbyVolumeFwConfig),uiActualConfigSize);             
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DolbyVolume :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 

    BDBG_MSG((" i32DolbyVolumeEnable = %d",sDolbyVolumeFwConfig.i32DolbyVolumeEnable));
    BDBG_MSG((" i32BlockSize = %d",sDolbyVolumeFwConfig.i32BlockSize));
    BDBG_MSG((" i32nBands = %d",sDolbyVolumeFwConfig.i32nBands));
    BDBG_MSG((" i32nChans = %d",sDolbyVolumeFwConfig.i32nChans));
    BDBG_MSG((" uiInputReferenceLevel = 0x%x -> i32InputReferenceLevel = 0x%x",hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiInputReferenceLevel,sDolbyVolumeFwConfig.i32InputReferenceLevel));
    BDBG_MSG((" uiOutputReferenceLevel = 0x%x -> i32OutputReferenceLevel = 0x%x",hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.uiOutputReferenceLevel,sDolbyVolumeFwConfig.i32OutputReferenceLevel));
    BDBG_MSG((" i32Calibration = 0x%x -> i32Calibration = 0x%x",hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iCalibration,sDolbyVolumeFwConfig.i32Calibration));
    BDBG_MSG((" i32VlmEnable = %d",sDolbyVolumeFwConfig.i32VlmEnable));
    BDBG_MSG((" i32ResetNowFlag = %d",sDolbyVolumeFwConfig.i32ResetNowFlag));
    BDBG_MSG((" iDigitalVolumeLevel = %d -> i32DigitalVolumeLevel = %d",hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iDigitalVolumeLevel,sDolbyVolumeFwConfig.i32DigitalVolumeLevel));
    BDBG_MSG((" iAnalogVolumeLevel = %d -> i32AnalogVolumeLevel = %d",hStage->sProcessingStageSettings.uConfigParams.sDolbyVolConfigParams.iAnalogVolumeLevel,sDolbyVolumeFwConfig.i32AnalogVolumeLevel));
    BDBG_MSG((" i32LvlAmount = %d",sDolbyVolumeFwConfig.i32LvlAmount));
    BDBG_MSG((" i32LvlEnable = %d",sDolbyVolumeFwConfig.i32LvlEnable));
    BDBG_MSG((" i32EnableMidsideProc = %d",sDolbyVolumeFwConfig.i32EnableMidsideProc));
    BDBG_MSG((" i32HalfmodeFlag = %d",sDolbyVolumeFwConfig.i32HalfmodeFlag));
    BDBG_MSG((" i32LimiterEnable  = %d",sDolbyVolumeFwConfig.i32LimiterEnable ));
    BDBG_MSG(("-------------------------------------------")); 


    return err;
}

BERR_Code BRAP_P_MapDV258UserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_DV258ConfigParams   sDV258FwConfig;
    BDBG_ASSERT(hStage);

    sDV258FwConfig.i32DolbyVolumeEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bDolbyVolEna;
    sDV258FwConfig.i32VlmMdlEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bVolModelerEna;    
    sDV258FwConfig.i32HalfmodeFlag = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bHalfmodeEna;    
    sDV258FwConfig.i32EnableMidsideProc = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bEnableMidsideProc;
    sDV258FwConfig.i32LvlEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bVolLvlEnable;
    sDV258FwConfig.i32LvlAmount = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.uiVolLvlAmount;

    sDV258FwConfig.i32InputReferenceLevel = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.uiInputReferenceLevel;
    sDV258FwConfig.i32OutputReferenceLevel = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.uiOutputReferenceLevel;
    sDV258FwConfig.i32Pregain   = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.iPreGain;
    sDV258FwConfig.i32CalibrationOffset= (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.iCalibrationOffset;

    sDV258FwConfig.i32DigitalVolumeLevel = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.iDigitalVolumeLevel;
    sDV258FwConfig.i32AnalogVolumeLevel = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.iAnalogVolumeLevel;
    sDV258FwConfig.i32ResetNowFlag = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bResetNowFlag;
    sDV258FwConfig.i32LimiterEnable = (int32_t)hStage->sProcessingStageSettings.uConfigParams.sDV258Params.bLimiterEnable;    

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sDV258FwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DolbyVolume :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" sDV258FwConfig.i32DolbyVolumeEnable = %d",sDV258FwConfig.i32DolbyVolumeEnable));
    BDBG_MSG((" sDV258FwConfig.i32VlmMdlEnable = %d",sDV258FwConfig.i32VlmMdlEnable));
    BDBG_MSG((" sDV258FwConfig.i32HalfmodeFlag = %d",sDV258FwConfig.i32HalfmodeFlag));
    BDBG_MSG((" sDV258FwConfig.i32EnableMidsideProc = %d",sDV258FwConfig.i32EnableMidsideProc));
    BDBG_MSG((" sDV258FwConfig.i32LvlEnable = %d",sDV258FwConfig.i32LvlEnable));
    BDBG_MSG((" sDV258FwConfig.i32LvlAmount = %d",sDV258FwConfig.i32LvlAmount));
    BDBG_MSG((" sDV258FwConfig.i32InputReferenceLevel = %d",sDV258FwConfig.i32InputReferenceLevel));
    BDBG_MSG((" sDV258FwConfig.i32Pregain = %d",sDV258FwConfig.i32Pregain));
    BDBG_MSG((" sDV258FwConfig.i32OutputReferenceLevel  = %d",sDV258FwConfig.i32OutputReferenceLevel ));
    BDBG_MSG((" sDV258FwConfig.i32CalibrationOffset = %d",sDV258FwConfig.i32CalibrationOffset));
    BDBG_MSG((" sDV258FwConfig.i32DigitalVolumeLevel = %d",sDV258FwConfig.i32DigitalVolumeLevel));
    BDBG_MSG((" sDV258FwConfig.i32AnalogVolumeLevel = %d",sDV258FwConfig.i32AnalogVolumeLevel));
    BDBG_MSG((" sDV258FwConfig.i32ResetNowFlag = %d",sDV258FwConfig.i32ResetNowFlag));
    BDBG_MSG((" sDV258FwConfig.i32LimiterEnable = %d",sDV258FwConfig.i32LimiterEnable));
    BDBG_MSG(("-------------------------------------------")); 
    
    return err;
}



BERR_Code BRAP_P_MapGenericPassthruUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_PassthruConfigParams sPassthruConfig;
    BDBG_ASSERT(hStage);
    BSTD_UNUSED(hStage);
    /* Copy user Config params to Firmware params */
    sPassthruConfig.ui32PassthruType = BRAP_FWIF_ePassthruType_SPDIF;
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sPassthruConfig),uiActualConfigSize);
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("GenericPassthru :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" sPassthruConfig.ui32PassthruType = %d",sPassthruConfig.ui32PassthruType));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapAvlUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_AVLConfigParams sAvlFwConfig;
    BDBG_ASSERT(hStage);
    BSTD_UNUSED(hStage);

    /* Copy user Config params to Firmware params */
    sAvlFwConfig.ui32AVLEnableFlag = !(hStage->sProcessingStageSettings.uConfigParams.sAvlParams.bBypass);
    sAvlFwConfig.i32Target = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.iTarget*32768;
    sAvlFwConfig.i32LowerBound = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.iLowerBound*32768;    
    sAvlFwConfig.i32FixedBoost = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiFixedBoost*32768;
    sAvlFwConfig.i32RefLevel = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiRef;
    sAvlFwConfig.i32Alpha = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiAlpha;
    sAvlFwConfig.i32Beta = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiBeta;
    sAvlFwConfig.i32ActiveThreshold = (hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiThreshold * 32768)/10;
    sAvlFwConfig.i32DTFPCNT = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiDtfPcnt;
    sAvlFwConfig.i32Alpha2 = hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiAlpha2;
    sAvlFwConfig.i32NSFGR_SEC = (hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uisNsfgr*32768)/10;
    sAvlFwConfig.i32DTF = (hStage->sProcessingStageSettings.uConfigParams.sAvlParams.uiDtf)*32768/10;
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sAvlFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Avl :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" sAvlFwConfig.ui32AVLEnableFlag = %d",sAvlFwConfig.ui32AVLEnableFlag));
    BDBG_MSG((" sAvlFwConfig.i32Target = %d",sAvlFwConfig.i32Target));
    BDBG_MSG((" sAvlFwConfig.i32LowerBound = %d",sAvlFwConfig.i32LowerBound));
    BDBG_MSG((" sAvlFwConfig.i32FixedBoost = %d",sAvlFwConfig.i32FixedBoost));
    BDBG_MSG((" sAvlFwConfig.i32RefLevel = %d",sAvlFwConfig.i32RefLevel));
    BDBG_MSG((" sAvlFwConfig.i32Alpha = %d",sAvlFwConfig.i32Alpha));
    BDBG_MSG((" sAvlFwConfig.i32Beta = %d",sAvlFwConfig.i32Beta));
    BDBG_MSG((" sAvlFwConfig.i32ActiveThreshold  = %d",sAvlFwConfig.i32ActiveThreshold ));
    BDBG_MSG((" sAvlFwConfig.i32DTFPCNT = %d",sAvlFwConfig.i32DTFPCNT));
    BDBG_MSG((" sAvlFwConfig.i32Alpha2 = %d",sAvlFwConfig.i32Alpha2));
    BDBG_MSG((" sAvlFwConfig.i32NSFGR_SEC = %d",sAvlFwConfig.i32NSFGR_SEC));
    BDBG_MSG((" sAvlFwConfig.i32DTF = %d",sAvlFwConfig.i32DTF));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapDTSENCUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_DtsBroadcastEncConfigParams sDTSENCFwConfig;
    BDBG_ASSERT(hStage);

    /* Map Firmware to User params */
    sDTSENCFwConfig.ui32SpdifHeaderEnable = hStage->sProcessingStageSettings.uConfigParams.sDTSENCParams.bSpdifHeaderEnable; 
 
    sDTSENCFwConfig.ui32CertificationEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sDTSENCParams.bCertificationApp;
    sDTSENCFwConfig.ui32LFEEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sDTSENCParams.bLfeEnable;
    sDTSENCFwConfig.eInputDataAcMode = hStage->sProcessingStageSettings.uConfigParams.sDTSENCParams.eInputDataAcMode;         
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sDTSENCFwConfig),uiActualConfigSize);             
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DTSENC :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" sDTSENCFwConfig.ui32SpdifHeaderEnable = %d",sDTSENCFwConfig.ui32SpdifHeaderEnable));
    BDBG_MSG((" sDTSENCFwConfig.ui32CertificationEnableFlag = %d",sDTSENCFwConfig.ui32CertificationEnableFlag));
    BDBG_MSG((" sDTSENCFwConfig.ui32LFEEnableFlag = %d",sDTSENCFwConfig.ui32LFEEnableFlag));
    BDBG_MSG((" sDTSENCFwConfig.eInputDataAcMode = %d",sDTSENCFwConfig.eInputDataAcMode));
    BDBG_MSG(("-------------------------------------------"));  
    return err;
}

BERR_Code BRAP_P_MapAc3ENCUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_Ac3EncConfigParams sAc3ENCFwConfig;
    BDBG_ASSERT(hStage);

    /* Map Firmware to User params */
    sAc3ENCFwConfig.eTranscodeEnable = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bTranscodeEnable;    
    sAc3ENCFwConfig.eSpdifHeaderEnable = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bSpdifHeaderEnable; 

    sAc3ENCFwConfig.i32NumChans = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.eInputCh; 
    sAc3ENCFwConfig.i32AudCodMode = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.eAudCodMode; 
    sAc3ENCFwConfig.i32DataRate = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.eDataRate; 
    sAc3ENCFwConfig.i32LoFreqEffOn = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bLfe;     

    switch (hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.eSampleRate)
    {
        case BAVC_AudioSamplingRate_e48k:
        default:            
            /* Only 48kHz is supported */
            sAc3ENCFwConfig.i32SampRateCod = 0;
            break;
    }
    sAc3ENCFwConfig.i32LfeFiltInUse = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bLfeFilter; 
    sAc3ENCFwConfig.i32CompChar = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bCmpChar; 
    sAc3ENCFwConfig.i32CompChar2 = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bCmpChar2; 
    sAc3ENCFwConfig.i32SurDelayArg = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bSurrDelay; 

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sAc3ENCFwConfig),uiActualConfigSize);             
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Ac3ENC :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" eTranscodeEnable = %d",sAc3ENCFwConfig.eTranscodeEnable));
    BDBG_MSG((" eSpdifHeaderEnable = %d",sAc3ENCFwConfig.eSpdifHeaderEnable));
    BDBG_MSG((" i32NumChans = %d",sAc3ENCFwConfig.i32NumChans));
    BDBG_MSG((" i32AudCodMode = %d",sAc3ENCFwConfig.i32AudCodMode));
    BDBG_MSG((" i32DataRate = %d",sAc3ENCFwConfig.i32DataRate));
    BDBG_MSG((" i32LoFreqEffOn = %d",sAc3ENCFwConfig.i32LoFreqEffOn));
    BDBG_MSG((" i32SampRateCod = %d",sAc3ENCFwConfig.i32SampRateCod));
    BDBG_MSG((" i32LfeFiltInUse = %d",sAc3ENCFwConfig.i32LfeFiltInUse));
    BDBG_MSG((" i32CompChar = %d",sAc3ENCFwConfig.i32CompChar));
    BDBG_MSG((" i32CompChar2 = %d",sAc3ENCFwConfig.i32CompChar2));
    BDBG_MSG((" i32SurDelayArg = %d",sAc3ENCFwConfig.i32SurDelayArg));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapMp3EncUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_Mpeg1L3EncConfigParams  sMp3EncFwConfig;
    BDBG_ASSERT(hStage);

    sMp3EncFwConfig.ui32BitRate = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.eBitRate;    
    sMp3EncFwConfig.ui32AddCRCProtect = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.bCrcProtect;    
    sMp3EncFwConfig.ui32PrivateBit = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.bPrivateBit;    
    sMp3EncFwConfig.ui32jStereoControl = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.eStereoControl;    
    sMp3EncFwConfig.ui32Copyright = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.bCopyright;    
    sMp3EncFwConfig.ui32Original = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.bOriginalBitSetting;        
    sMp3EncFwConfig.ui32BitsPerSample = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.uiBitsPerSample;  

    BRAP_P_ConvertSR(hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.eBitRate,&(sMp3EncFwConfig.ui32SampleRate));
    
    sMp3EncFwConfig.ui32Emphasis = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.eEmphasisMode;    
    sMp3EncFwConfig.ui32InputMode = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.eInputMode;    
    sMp3EncFwConfig.ui32InterleavedPCM = hStage->sProcessingStageSettings.uConfigParams.sMp3EncConfigParams.bInterleavedPcm;    

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Ac3ENC :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" ui32BitRate = %d",sMp3EncFwConfig.ui32BitRate));
    BDBG_MSG((" ui32AddCRCProtect  = %d",sMp3EncFwConfig.ui32AddCRCProtect ));
    BDBG_MSG((" ui32PrivateBit = %d",sMp3EncFwConfig.ui32PrivateBit));
    BDBG_MSG((" ui32jStereoControl = %d",sMp3EncFwConfig.ui32jStereoControl));
    BDBG_MSG((" ui32Copyright = %d",sMp3EncFwConfig.ui32Copyright));
    BDBG_MSG((" ui32Original = %d",sMp3EncFwConfig.ui32Original));
    BDBG_MSG((" ui32BitsPerSample  = %d",sMp3EncFwConfig.ui32BitsPerSample ));
    BDBG_MSG((" ui32Emphasis = %d",sMp3EncFwConfig.ui32Emphasis));
    BDBG_MSG((" ui32InputMode = %d",sMp3EncFwConfig.ui32InputMode));
    BDBG_MSG((" ui32InterleavedPCM = %d",sMp3EncFwConfig.ui32InterleavedPCM));
    BDBG_MSG(("-------------------------------------------")); 
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sMp3EncFwConfig),uiActualConfigSize);             
    return err;
}


BERR_Code BRAP_P_MapSbcEncUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_SbcEncoderUserConfig    sSbcEncFwConfig;
    BDBG_ASSERT(hStage);

    sSbcEncFwConfig.NumBlocks = hStage->sProcessingStageSettings.uConfigParams.sSbcEncConfigParams.uiNumBlocks;
    sSbcEncFwConfig.NumSubbands = hStage->sProcessingStageSettings.uConfigParams.sSbcEncConfigParams.uiNumSubBands;
    sSbcEncFwConfig.JointStereo = hStage->sProcessingStageSettings.uConfigParams.sSbcEncConfigParams.bJointStereoEnable;
    sSbcEncFwConfig.BitAllocation = hStage->sProcessingStageSettings.uConfigParams.sSbcEncConfigParams.uiBitAllocation;
    sSbcEncFwConfig.BitPool = hStage->sProcessingStageSettings.uConfigParams.sSbcEncConfigParams.uiBitPool;    

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sSbcEncFwConfig),uiActualConfigSize);
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("SbcEnc :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" NumBlocks = %d",sSbcEncFwConfig.NumBlocks));
    BDBG_MSG((" NumSubbands = %d",sSbcEncFwConfig.NumSubbands));
    BDBG_MSG((" JointStereo = %d",sSbcEncFwConfig.JointStereo));
    BDBG_MSG((" BitAllocation = %d",sSbcEncFwConfig.BitAllocation));
    BDBG_MSG((" BitPool = %d",sSbcEncFwConfig.BitPool));
    BDBG_MSG(("-------------------------------------------"));    
    return err;
}

BERR_Code BRAP_P_MapDolbyTranscodeUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_DDTranscodeConfigParams sDolbyTranscodeFwConfig;
    BDBG_ASSERT(hStage);


    sDolbyTranscodeFwConfig.eTranscodeEnable = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bTranscodeEnable;
    sDolbyTranscodeFwConfig.eSpdifHeaderEnable = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bSpdifHeaderEnable;    
    sDolbyTranscodeFwConfig.i32LoFreqEffOn = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.bLfe;
    sDolbyTranscodeFwConfig.i32AudCodMode = hStage->sProcessingStageSettings.uConfigParams.sAc3ENCParams.eAudCodMode;

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sDolbyTranscodeFwConfig),uiActualConfigSize);

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("SbcEnc :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" eTranscodeEnable = %d",sDolbyTranscodeFwConfig.eTranscodeEnable));
    BDBG_MSG((" eSpdifHeaderEnable = %d",sDolbyTranscodeFwConfig.eSpdifHeaderEnable));
    BDBG_MSG((" i32LoFreqEffOn = %d",sDolbyTranscodeFwConfig.i32LoFreqEffOn));
    BDBG_MSG((" i32AudCodMode = %d",sDolbyTranscodeFwConfig.i32AudCodMode));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapBrcm3DSurroundUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_Brcm3DSurroundConfigParams sBrcm3DSurroundFwConfig;
    BDBG_ASSERT(hStage);

    sBrcm3DSurroundFwConfig.i32BRCM3DSurroundEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sBrcm3DSurroundParams.b3DSurroundEnable;
    sBrcm3DSurroundFwConfig.i32SoftLimiterEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sBrcm3DSurroundParams.bSoftLimiterEnable;

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sBrcm3DSurroundFwConfig),uiActualConfigSize);
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Brcm3DSurround :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" i32BRCM3DSurroundEnableFlag = %d",sBrcm3DSurroundFwConfig.i32BRCM3DSurroundEnableFlag));
    BDBG_MSG((" i32SoftLimiterEnableFlag = %d",sBrcm3DSurroundFwConfig.i32SoftLimiterEnableFlag));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

BERR_Code BRAP_P_MapMonoDownmixUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_MonoDownMixConfigParams sMonoDownmixFwConfig;
    BDBG_ASSERT(hStage);

    sMonoDownmixFwConfig.i32MonoDownMixEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sMonoDownmixParams.bMonoDownMixEnableFlag;

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sMonoDownmixFwConfig),uiActualConfigSize);
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("MonoDownmixFwConfig :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG((" i32MonoDownMixEnableFlag = %d",sMonoDownmixFwConfig.i32MonoDownMixEnableFlag));
    BDBG_MSG(("-------------------------------------------")); 
    return err;
}

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BERR_Code BRAP_P_MapFwMixerUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_MixerConfigParams   *psFwMixerConfig;
    unsigned int i=0, j=BRAP_INVALID_VALUE, uiOpChnl=0;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == true)
    {
        BDBG_ERR(("Fw Mixer PP should  be part of Association network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    psFwMixerConfig = (BRAP_FWIF_P_MixerConfigParams *)BKNI_Malloc(sizeof(BRAP_FWIF_P_MixerConfigParams));
    if(psFwMixerConfig == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        return err;
    }
    BKNI_Memset(psFwMixerConfig, 0, sizeof(BRAP_FWIF_P_MixerConfigParams));
    
    if((BRAP_P_IsPointerValid((void *)hStage->uHandle.hAssociation))
        &&(BRAP_P_IsPointerValid((void *)hStage->uHandle.hAssociation->hMultiStreamDecoder)))
    {          
        if((BRAP_P_IsPointerValid((void *)hStage->uHandle.hAssociation->hMultiStreamDecoder->
                        sExtMultiStreamDecoderDetails.hPrimaryChannel)) &&
           (hStage->uHandle.hAssociation->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.
                        hPrimaryChannel->eState == BRAP_P_State_eStarted))
        {
			j = hStage->uHandle.hAssociation->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.hPrimaryChannel->uiFWMixerIpIndex;
            if((j != BRAP_INVALID_VALUE) && (j < BRAP_AF_P_MAX_IP_FORKS))
            {
                for(uiOpChnl = 0; uiOpChnl  < BRAP_OutputChannel_eMax ; uiOpChnl++)
                {
                    /*Fw requires in 5.27Format. App gives in 5.23 format*/
                    psFwMixerConfig->MixingCoeffs[j][uiOpChnl]  = hStage->uHandle.hAssociation->hMultiStreamDecoder->
                            sExtMultiStreamDecoderDetails.hPrimaryChannel->sMixingCoeff.ui32Coef[uiOpChnl][uiOpChnl] << 4;
                    
                }
            }
        }
        for(i=0; i < BRAP_MAX_SEC_CHANNEL_FOR_MS_DECODER; i++)
        {
            if((BRAP_P_IsPointerValid((void *)hStage->uHandle.hAssociation->hMultiStreamDecoder->
                            sExtMultiStreamDecoderDetails.hSecondaryChannel[i])) &&
               (hStage->uHandle.hAssociation->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.
                            hSecondaryChannel[i]->eState == BRAP_P_State_eStarted))            
            {        
    			j = hStage->uHandle.hAssociation->hMultiStreamDecoder->sExtMultiStreamDecoderDetails.hSecondaryChannel[i]->uiFWMixerIpIndex;
                if((j != BRAP_INVALID_VALUE) && (j < BRAP_AF_P_MAX_IP_FORKS))
                {    
                    for(uiOpChnl = 0; uiOpChnl  < BRAP_OutputChannel_eMax ; uiOpChnl++)
                    {
                        /*Fw requires in 5.27Format. App gives in 5.23 format*/
                        psFwMixerConfig->MixingCoeffs[j][uiOpChnl]  = hStage->uHandle.hAssociation->hMultiStreamDecoder->
                                sExtMultiStreamDecoderDetails.hSecondaryChannel[i]->sMixingCoeff.ui32Coef[uiOpChnl][uiOpChnl] << 4;            
                    }
                }
            }
        }
    }

    BDBG_MSG(("---------------------------------------")); 
    BDBG_MSG(("Firmware Mixer :: FW CONFIG VALUES")); 
    BDBG_MSG(("---------------------------------------")); 
    for(j = 0; j  < BRAP_AF_P_MAX_IP_FORKS ; j++)
    {
        for(uiOpChnl = 0; uiOpChnl  < BRAP_OutputChannel_eMax ; uiOpChnl++)
        {
            BDBG_MSG(("MixingCoeffs[I/P=%d][OpChnl=%d]=0x%x",j,uiOpChnl,psFwMixerConfig->MixingCoeffs[j][uiOpChnl]));
        }
    }
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)psFwMixerConfig,uiActualConfigSize);  

    if(psFwMixerConfig)
        BKNI_Free(psFwMixerConfig);
    
    return err;
}
#endif

BERR_Code BRAP_P_MapAudysseyVolUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_AudysseyVolUserConfig *pAudysseyVolConfig = NULL;
    BRAP_DSPCHN_AudysseyVolParams   *pAudysseyVolConfigAppFormat = NULL;
    int32_t i =0;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("Audyssey Vol PP should  be part of channel specfic network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pAudysseyVolConfig = (BRAP_FWIF_P_AudysseyVolUserConfig *) BKNI_Malloc(sizeof(BRAP_FWIF_P_AudysseyVolUserConfig));
    if(pAudysseyVolConfig == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }

    pAudysseyVolConfigAppFormat = (BRAP_DSPCHN_AudysseyVolParams *) BKNI_Malloc(sizeof(BRAP_DSPCHN_AudysseyVolParams));
    if(pAudysseyVolConfigAppFormat == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }
    
    *pAudysseyVolConfigAppFormat = hStage->sProcessingStageSettings.uConfigParams.sAudysseyVolParams;

    pAudysseyVolConfig->ui32NumChannels = 2;
    pAudysseyVolConfig->ui32AudysseyVolBypass = !(pAudysseyVolConfigAppFormat->bAudysseyVolEnable);
    pAudysseyVolConfig->i32ChannelMask = pAudysseyVolConfigAppFormat->ui32ChannelMask;
    pAudysseyVolConfig->i32AudysseyVolInit = pAudysseyVolConfigAppFormat->bAudysseyVolInit;
    pAudysseyVolConfig->i32HeadroomOffset = BRAP_P_FloatToQ1022(pAudysseyVolConfigAppFormat->i32HeadroomOffset,100);
    pAudysseyVolConfig->i32SwAudysseyVol = pAudysseyVolConfigAppFormat->bSwAudysseyVol;
    pAudysseyVolConfig->i32SwDynEQ = pAudysseyVolConfigAppFormat->ui32SwDynEQ;
    pAudysseyVolConfig->i32SwDynSurrGain = pAudysseyVolConfigAppFormat->bSwDynSurrGain;
    pAudysseyVolConfig->i32SwHiLoCmpress = pAudysseyVolConfigAppFormat->bSwHiLoCmpress;
    pAudysseyVolConfig->i32dBVolSetting = BRAP_P_FloatToQ1022(pAudysseyVolConfigAppFormat->i32dBVolSetting,10);
    pAudysseyVolConfig->i32GCF = BRAP_P_FloatToQ1022(pAudysseyVolConfigAppFormat->i32GCF,100);

    for(i = 0 ;i < 8; i++ )
    {
        pAudysseyVolConfig->i32chCalbGain[i] = BRAP_P_FloatToQ1022(pAudysseyVolConfigAppFormat->i32chCalbGain[i],100);
        pAudysseyVolConfig->i32chCalcLevel[i] = BRAP_P_FloatToQ1022(pAudysseyVolConfigAppFormat->i32chCalcLevel[i],100);
    }

    BDBG_MSG(("----------------------------------------------------")); 
    BDBG_MSG(("Audyssey Volume :: FW CONFIG <-> USER CONFIG MAPPING")); 
    BDBG_MSG(("----------------------------------------------------")); 
    BDBG_MSG(("ui32AudysseyVolBypass = %u ,<-> bAudysseyVolEnable = %u",pAudysseyVolConfig->ui32AudysseyVolBypass,pAudysseyVolConfigAppFormat->bAudysseyVolEnable));
    BDBG_MSG(("i32ChannelMask = %u ,<-> i32ChannelMask = %u",pAudysseyVolConfig->i32ChannelMask,pAudysseyVolConfigAppFormat->ui32ChannelMask));
    BDBG_MSG(("i32AudysseyVolInit = %u ,<-> bAudysseyVolInit = %d",pAudysseyVolConfig->i32AudysseyVolInit,pAudysseyVolConfigAppFormat->bAudysseyVolInit));
    BDBG_MSG(("i32HeadroomOffset = %u ,<-> i32HeadroomOffset = %u",pAudysseyVolConfig->i32HeadroomOffset,pAudysseyVolConfigAppFormat->i32HeadroomOffset));
    BDBG_MSG(("i32SwAudysseyVol = %u ,<-> bSwAudysseyVol = %d",pAudysseyVolConfig->i32SwAudysseyVol,pAudysseyVolConfigAppFormat->bSwAudysseyVol));
    BDBG_MSG(("i32SwDynEQ = %u ,<-> ui32SwDynEQ = %u",pAudysseyVolConfig->i32SwDynEQ,pAudysseyVolConfigAppFormat->ui32SwDynEQ));
    BDBG_MSG(("i32SwDynSurrGain = %u ,<-> bSwDynSurrGain = %d",pAudysseyVolConfig->i32SwDynSurrGain,pAudysseyVolConfigAppFormat->bSwDynSurrGain));
    BDBG_MSG(("i32SwHiLoCmpress = %u ,<-> bSwHiLoCmpress = %d",pAudysseyVolConfig->i32SwHiLoCmpress,pAudysseyVolConfigAppFormat->bSwHiLoCmpress));
    BDBG_MSG(("i32dBVolSetting = %u ,<-> i32dBVolSetting = %u",pAudysseyVolConfig->i32dBVolSetting,pAudysseyVolConfigAppFormat->i32dBVolSetting));
    BDBG_MSG(("i32GCF = %u ,<-> i32GCF = %u",pAudysseyVolConfig->i32GCF,pAudysseyVolConfigAppFormat->i32GCF));
            
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)pAudysseyVolConfig,uiActualConfigSize);  

   error:
        if(pAudysseyVolConfig)
            BKNI_Free(pAudysseyVolConfig);

        if(pAudysseyVolConfigAppFormat)
            BKNI_Free(pAudysseyVolConfigAppFormat);
    
    return err;
}

BERR_Code BRAP_P_MapAudysseyABXUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_ABXConfigParams sAudysseyABXConfig;
    BRAP_DSPCHN_AudysseyABXParams   sAudysseyABXConfigAppFormat;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("Audyssey ABX PP should  be part of channel specfic network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    sAudysseyABXConfigAppFormat = hStage->sProcessingStageSettings.uConfigParams.sAudysseyABXParams;

    sAudysseyABXConfig.ui32OperationMode = sAudysseyABXConfigAppFormat.bOperationMode;
    sAudysseyABXConfig.ui32FilterSet = sAudysseyABXConfigAppFormat.ui32FilterSet;
    sAudysseyABXConfig.ui32HarmonicGain = sAudysseyABXConfigAppFormat.ui32HarmonicGain;
    sAudysseyABXConfig.ui32DryGain = sAudysseyABXConfigAppFormat.ui32DryGain;

    BDBG_MSG(("----------------------------------------------------")); 
    BDBG_MSG(("Audyssey ABX :: FW CONFIG <-> USER CONFIG MAPPING")); 
    BDBG_MSG(("----------------------------------------------------")); 
    BDBG_MSG(("ui32OperationMode = %u <-> bOperationMode = %d",sAudysseyABXConfig.ui32OperationMode,sAudysseyABXConfigAppFormat.bOperationMode));
    BDBG_MSG(("ui32FilterSet = %u ,<-> ui32FilterSet = %u",sAudysseyABXConfig.ui32FilterSet,sAudysseyABXConfigAppFormat.ui32FilterSet));
    BDBG_MSG(("ui32HarmonicGain = %u ,<-> ui32HarmonicGain = %u",sAudysseyABXConfig.ui32HarmonicGain,sAudysseyABXConfigAppFormat.ui32HarmonicGain));
    BDBG_MSG(("ui32DryGain = %u ,<-> ui32DryGain = %u",sAudysseyABXConfig.ui32DryGain,sAudysseyABXConfigAppFormat.ui32DryGain));

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sAudysseyABXConfig),uiActualConfigSize);  
    
    return err;
}

BERR_Code BRAP_P_MapSrsCsTdUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams  *pSrsCsTdConfig = NULL;
    BRAP_DSPCHN_SrsCsTdParams                   *pSrsCsTdConfigAppFormat = NULL;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("SrsCsTd PP should  be part of channel specfic network"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    pSrsCsTdConfig = (BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams *) 
                        BKNI_Malloc(sizeof(BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams));
    if(pSrsCsTdConfig == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }
    
    pSrsCsTdConfigAppFormat = (BRAP_DSPCHN_SrsCsTdParams *) BKNI_Malloc(sizeof(BRAP_DSPCHN_SrsCsTdParams));
    if(pSrsCsTdConfigAppFormat == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }
    
    *pSrsCsTdConfigAppFormat = hStage->sProcessingStageSettings.uConfigParams.sSrsCsTdParams;

    /* BRAP_DSPCHN_TopLevelStudioSoundParams */
    pSrsCsTdConfig->sTopLevelConfig.i32IsStudioSound = pSrsCsTdConfigAppFormat->sTopLevelParams.bIsStudioSound;
    pSrsCsTdConfig->sTopLevelConfig.i32StudioSoundMode = pSrsCsTdConfigAppFormat->sTopLevelParams.eStudioSoundMode;
    pSrsCsTdConfig->sTopLevelConfig.i32mEnable = pSrsCsTdConfigAppFormat->sTopLevelParams.bEnable;
    pSrsCsTdConfig->sTopLevelConfig.i32mInputGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTopLevelParams.ui32InputGain,100);
    pSrsCsTdConfig->sTopLevelConfig.i32mHeadroomGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTopLevelParams.ui32HeadroomGain,100);
    pSrsCsTdConfig->sTopLevelConfig.i32mInputMode = pSrsCsTdConfigAppFormat->sTopLevelParams.eInputMode;
    pSrsCsTdConfig->sTopLevelConfig.i32mOutputGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTopLevelParams.ui32OutputGain,100);    
    pSrsCsTdConfig->sTopLevelConfig.i32mBypassGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTopLevelParams.ui32BypassGain,100);
    /* BRAP_DSPCHN_SrsCircleSurroundParams */    
    pSrsCsTdConfig->sCSDecoderConfig.i32mEnable = pSrsCsTdConfigAppFormat->sCircleSurroundParams.bEnable;
    pSrsCsTdConfig->sCSDecoderConfig.i32mInputGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32InputGain,1000);
    pSrsCsTdConfig->sCSDecoderConfig.i32mMode = pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32Mode;
    pSrsCsTdConfig->sCSDecoderConfig.i32mOutputMode = pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputMode;
    pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainLR = BRAP_P_FloatToQ428(pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainLR,1000);
    pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainLsRs = BRAP_P_FloatToQ428(pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainLsRs,1000);
    pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainC = BRAP_P_FloatToQ428(pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainC,1000);
    pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainSub = BRAP_P_FloatToQ428(pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainSub,1000);
    /* BRAP_DSPCHN_SrsTruDialogParams */
    pSrsCsTdConfig->sTruDialogConfig.i32mEnable = pSrsCsTdConfigAppFormat->sTruDialogParams.bEnable;
    pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogProcessGain = BRAP_P_FloatToQ527(pSrsCsTdConfigAppFormat->sTruDialogParams.ui32ProcessGain,100);
    pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogInputGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTruDialogParams.ui32InputGain,100);
    pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogOutputGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTruDialogParams.ui32OutputGain,100);
    pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogDialogClarityGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTruDialogParams.ui32DialogClarityGain,100);
    pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogBypassGain = BRAP_P_FloatToQ131(pSrsCsTdConfigAppFormat->sTruDialogParams.ui32BypassGain,100);

    BDBG_MSG(("-------------------------------------------------")); 
    BDBG_MSG(("Srs CsTd :: FW CONFIG <-> USER CONFIG MAPPING")); 
    BDBG_MSG(("-------------------------------------------------")); 
    BDBG_MSG((" TopLevelSrsCsTdParams:"));
    BDBG_MSG(("i32mEnable = %x <-> bEnable = %d",
        pSrsCsTdConfig->sTopLevelConfig.i32mEnable,pSrsCsTdConfigAppFormat->sTopLevelParams.bEnable));
    BDBG_MSG(("i32mBypassGain = %x ,<-> ui32BypassGain = %u",
        pSrsCsTdConfig->sTopLevelConfig.i32mBypassGain,pSrsCsTdConfigAppFormat->sTopLevelParams.ui32BypassGain));
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsCsTdConfig->sTopLevelConfig.i32mInputGain,pSrsCsTdConfigAppFormat->sTopLevelParams.ui32InputGain));
    BDBG_MSG(("i32mHeadroomGain = %x ,<-> ui32HeadroomGain = %u",
        pSrsCsTdConfig->sTopLevelConfig.i32mHeadroomGain,pSrsCsTdConfigAppFormat->sTopLevelParams.ui32HeadroomGain));
    BDBG_MSG(("i32mInputMode = %x ,<-> eInputMode = %u",
        pSrsCsTdConfig->sTopLevelConfig.i32mInputMode,pSrsCsTdConfigAppFormat->sTopLevelParams.eInputMode));
    
    BDBG_MSG((" SrsCircleSurroundParams:"));
    BDBG_MSG(("i32mEnable = %x <-> bEnable = %d",
        pSrsCsTdConfig->sCSDecoderConfig.i32mEnable,pSrsCsTdConfigAppFormat->sCircleSurroundParams.bEnable));
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mInputGain,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32InputGain));
    BDBG_MSG(("i32mMode = %x ,<-> ui32Mode = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mMode,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32Mode));
    BDBG_MSG(("i32mOutputMode = %x ,<-> ui32OutputMode = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mOutputMode,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputMode));
    BDBG_MSG(("i32mCSDecOutputGainLR = %x ,<-> ui32OutputGainLR = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainLR,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainLR));    
    BDBG_MSG(("i32mCSDecOutputGainLsRs = %x ,<-> ui32OutputGainLsRs = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainLsRs,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainLsRs));
    BDBG_MSG(("i32mCSDecOutputGainC = %x ,<-> ui32OutputGainC = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainC,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainC));
    BDBG_MSG(("i32mCSDecOutputGainSub = %x ,<-> ui32OutputGainSub = %u",
        pSrsCsTdConfig->sCSDecoderConfig.i32mCSDecOutputGainSub,pSrsCsTdConfigAppFormat->sCircleSurroundParams.ui32OutputGainSub));
    
    BDBG_MSG((" SrsTruDialogParams:"));
    BDBG_MSG(("i32mEnable = %x <-> bEnable = %d",
        pSrsCsTdConfig->sTruDialogConfig.i32mEnable,pSrsCsTdConfigAppFormat->sTruDialogParams.bEnable));
    BDBG_MSG(("i32mTruDialogProcessGain = %x ,<-> ui32ProcessGain = %u",
        pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogProcessGain,pSrsCsTdConfigAppFormat->sTruDialogParams.ui32ProcessGain));
    BDBG_MSG(("i32mTruDialogInputGain = %x ,<-> ui32InputGain = %u",
        pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogInputGain,pSrsCsTdConfigAppFormat->sTruDialogParams.ui32InputGain));
    BDBG_MSG(("i32mTruDialogOutputGain = %x ,<-> ui32OutputGain = %u",
        pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogOutputGain,pSrsCsTdConfigAppFormat->sTruDialogParams.ui32OutputGain));
    BDBG_MSG(("i32mTruDialogDialogClarityGain = %x ,<-> ui32DialogClarityGain = %u",
        pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogDialogClarityGain,pSrsCsTdConfigAppFormat->sTruDialogParams.ui32DialogClarityGain));    
    BDBG_MSG(("i32mTruDialogBypassGain = %x ,<-> ui32BypassGain = %u",
        pSrsCsTdConfig->sTruDialogConfig.i32mTruDialogBypassGain,pSrsCsTdConfigAppFormat->sTruDialogParams.ui32BypassGain));            
    BDBG_MSG(("----------------------------------------------------"));     

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)pSrsCsTdConfig,uiActualConfigSize);  

   error:
        if(pSrsCsTdConfig)
            BKNI_Free(pSrsCsTdConfig);

        if(pSrsCsTdConfigAppFormat)
            BKNI_Free(pSrsCsTdConfigAppFormat);
    
    return err;
}

BERR_Code BRAP_P_MapSrsEqHlUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,        /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;
    unsigned int i=0, j=0;
    BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams    *pSrsEqHlConfig = NULL;
    BRAP_DSPCHN_SrsEqHlParams                           *pSrsEqHlConfigAppFormat = NULL;
    BDBG_ASSERT(hStage);

    if(hStage->bChSpecificStage == false)
    {
        BDBG_ERR(("SrsEqHl PP should  be part of channel specfic network"));
		err = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto error;        
    }

    pSrsEqHlConfig = (BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams *) 
                        BKNI_Malloc(sizeof(BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams));
    if(pSrsEqHlConfig == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }

    pSrsEqHlConfigAppFormat = (BRAP_DSPCHN_SrsEqHlParams *) BKNI_Malloc(sizeof(BRAP_DSPCHN_SrsEqHlParams));
    if(pSrsEqHlConfigAppFormat == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto error;
    }
    
    *pSrsEqHlConfigAppFormat = hStage->sProcessingStageSettings.uConfigParams.sSrsEqHlParams;

    /* BRAP_DSPCHN_TopLevelStudioSoundParams */
    pSrsEqHlConfig->sTopLevelConfig.i32IsStudioSound = pSrsEqHlConfigAppFormat->sTopLevelParams.bIsStudioSound;
    pSrsEqHlConfig->sTopLevelConfig.i32StudioSoundMode = pSrsEqHlConfigAppFormat->sTopLevelParams.eStudioSoundMode;
    pSrsEqHlConfig->sTopLevelConfig.i32mEnable = pSrsEqHlConfigAppFormat->sTopLevelParams.bEnable;    
    pSrsEqHlConfig->sTopLevelConfig.i32mInputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sTopLevelParams.ui32InputGain,100);
    pSrsEqHlConfig->sTopLevelConfig.i32mHeadroomGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sTopLevelParams.ui32HeadroomGain,100);
    pSrsEqHlConfig->sTopLevelConfig.i32mInputMode = pSrsEqHlConfigAppFormat->sTopLevelParams.eInputMode;
    pSrsEqHlConfig->sTopLevelConfig.i32mOutputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sTopLevelParams.ui32OutputGain,100);
    pSrsEqHlConfig->sTopLevelConfig.i32mBypassGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sTopLevelParams.ui32BypassGain,100);    

    /* BRAP_DSPCHN_ParametricEqParams */
    for(i = 0; i < 2; i++)
    {
        pSrsEqHlConfig->sParametricEqConfig.i32mEnable[i] = pSrsEqHlConfigAppFormat->sPEqParams.bEnable[i];
        for(j = 0; j < BRAP_FW_MAX_PEQ_BANDS; j++)
        {
            pSrsEqHlConfig->sParametricEqConfig.i32mBandEnable[i][j] = pSrsEqHlConfigAppFormat->sPEqParams.bBandEnable[i][j];
        }
    }
    pSrsEqHlConfig->sParametricEqConfig.i32mInputGain = BRAP_P_FloatToQ527(pSrsEqHlConfigAppFormat->sPEqParams.ui32InputGain,100);
    pSrsEqHlConfig->sParametricEqConfig.i32mOutputGain = BRAP_P_FloatToQ527(pSrsEqHlConfigAppFormat->sPEqParams.ui32OutputGain,100);
    pSrsEqHlConfig->sParametricEqConfig.i32mBypassGain = BRAP_P_FloatToQ527(pSrsEqHlConfigAppFormat->sPEqParams.ui32BypassGain,100);
    pSrsEqHlConfig->sParametricEqConfig.i32CoefGenMode = pSrsEqHlConfigAppFormat->sPEqParams.eCoefGenMode;
    if(pSrsEqHlConfigAppFormat->sPEqParams.eCoefGenMode == 0)
    {
        if(sizeof(BRAP_DSPCHN_PEQFilterCoef) != sizeof(BRAP_FWIF_P_FilterCoefPeq))
        {
            BDBG_ERR(("SRS_EqHl: PEQ Filter Coefficients' structure sizes don't match between PI and FW"));
            return BERR_INVALID_PARAMETER;
        }    
        BKNI_Memcpy((void *)(volatile void *)(pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterCoefPeq),
                            (void *)(&pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterCoef),
                            sizeof(BRAP_FWIF_P_FilterCoefPeq)*3*BRAP_FW_MAX_PEQ_BANDS);
    }
    else
    {
        for(i = 0; i < BRAP_FW_MAX_PEQ_BANDS; i++)
        {    
            pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].i32BandGain = 
                BRAP_P_FloatToQ824(pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].i32BandGain,120);
            pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].ui32BandFrequency = 
                pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].ui32BandFrequency;
            pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].i32QFactor = 
                BRAP_P_FloatToQ824(pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].i32QFactor, 1600);
        }
    }

    /* BRAP_DSPCHN_HighPassFilterParams */    
    pSrsEqHlConfig->sHighPassFilterConfig.ui32mEnable = pSrsEqHlConfigAppFormat->sHPFParams.bEnable;
    pSrsEqHlConfig->sHighPassFilterConfig.ui32CoefGenMode = pSrsEqHlConfigAppFormat->sHPFParams.eCoefGenMode;

    if(pSrsEqHlConfigAppFormat->sHPFParams.eCoefGenMode == 0)
    {
        if(sizeof(BRAP_DSPCHN_HPFilterCoef) != sizeof(BRAP_FWIF_P_FilterCoefHpf))
        {
            BDBG_ERR(("SRS_EqHl: High Pass Filter Coefficients' structure sizes don't match between PI and FW"));
            return BERR_INVALID_PARAMETER;
        }    
        BKNI_Memcpy((void *)(volatile void *)(pSrsEqHlConfig->sHighPassFilterConfig.uHPFSettings.sFilterCoefHpf),
                            (void *)(&pSrsEqHlConfigAppFormat->sHPFParams.uHPFSettings.sHPFilterCoef),
                            sizeof(BRAP_FWIF_P_FilterCoefHpf)*3);
    }
    else
    {
        pSrsEqHlConfig->sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32CutoffFrequency = 
            pSrsEqHlConfigAppFormat->sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency;
        pSrsEqHlConfig->sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32Order = 
            pSrsEqHlConfigAppFormat->sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder;
    }

    /* BRAP_DSPCHN_GraphicEqParams */
    for(i = 0; i < 2; i++)
    {
        pSrsEqHlConfig->sGraphicEqConfig.i32mEnable[i] = pSrsEqHlConfigAppFormat->sGEqParams.bEnable[i];
        for(j = 0; j < BRAP_FW_MAX_GEQ_BANDS; j++)
        {
            pSrsEqHlConfig->sGraphicEqConfig.i32mBandGain[i][j] = BRAP_P_FloatToQ329(pSrsEqHlConfigAppFormat->sGEqParams.ui32BandGain[i][j],1000);
        }
    }
    pSrsEqHlConfig->sGraphicEqConfig.i32mFilterMode = pSrsEqHlConfigAppFormat->sGEqParams.ui32FilterMode;    
    if(pSrsEqHlConfigAppFormat->sGEqParams.ui32FilterMode == 0) /* 5 Band Config */
    {
        pSrsEqHlConfig->sGraphicEqConfig.i32mInputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sGEqParams.ui32InputGain,4000);
    }
    else if(pSrsEqHlConfigAppFormat->sGEqParams.ui32FilterMode == 1) /* 10 Band Config */
    {
    pSrsEqHlConfig->sGraphicEqConfig.i32mInputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sGEqParams.ui32InputGain,1000);
    }
    pSrsEqHlConfig->sGraphicEqConfig.i32mOutputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sGEqParams.ui32OutputGain,100);
    pSrsEqHlConfig->sGraphicEqConfig.i32mBypassGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sGEqParams.ui32BypassGain,100);

    /* BRAP_DSPCHN_HardLimiterParams */
    pSrsEqHlConfig->sHardLimiterConfig.i32LimiterEnable = pSrsEqHlConfigAppFormat->sHardLimiterParams.bEnable;
    pSrsEqHlConfig->sHardLimiterConfig.i32nchans = 2;
    pSrsEqHlConfig->sHardLimiterConfig.i32blockSize = pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32blockSize;
    pSrsEqHlConfig->sHardLimiterConfig.i32InputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32InputGain,400);
    pSrsEqHlConfig->sHardLimiterConfig.i32OutputGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32OutputGain,400);
    pSrsEqHlConfig->sHardLimiterConfig.i32BypassGain = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32BypassGain,100);
    pSrsEqHlConfig->sHardLimiterConfig.i32LimiterBoost = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32LimiterBoost,256000);
    pSrsEqHlConfig->sHardLimiterConfig.i32HardLimit = BRAP_P_FloatToQ131(pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32HardLimit, 100000);
    pSrsEqHlConfig->sHardLimiterConfig.i32DelayLength = pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32DelayLength;

    BDBG_MSG(("-------------------------------------------------")); 
    BDBG_MSG(("Srs EqHl :: FW CONFIG <-> USER CONFIG MAPPING")); 
    BDBG_MSG(("-------------------------------------------------")); 
    BDBG_MSG((" TopLevelSrsEqHlParams:"));
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsEqHlConfig->sTopLevelConfig.i32mInputGain,pSrsEqHlConfigAppFormat->sTopLevelParams.ui32InputGain));
    BDBG_MSG(("i32mHeadroomGain = %x ,<-> ui32HeadroomGain = %u",
        pSrsEqHlConfig->sTopLevelConfig.i32mHeadroomGain,pSrsEqHlConfigAppFormat->sTopLevelParams.ui32HeadroomGain));
    BDBG_MSG(("i32mInputMode = %x ,<-> eInputMode = %u",
        pSrsEqHlConfig->sTopLevelConfig.i32mInputMode,pSrsEqHlConfigAppFormat->sTopLevelParams.eInputMode));
    BDBG_MSG(("i32mOutputGain = %x ,<-> ui32OutputGain = %u",
        pSrsEqHlConfig->sTopLevelConfig.i32mOutputGain,pSrsEqHlConfigAppFormat->sTopLevelParams.ui32OutputGain));    
    
    BDBG_MSG((" ParametricEqParams:"));
    for(i = 0; i < 2; i++)
    {    
        BDBG_MSG(("i32mEnable[%d] = %d <-> bEnable[%d] = %d",
            i,pSrsEqHlConfig->sParametricEqConfig.i32mEnable[i],i,pSrsEqHlConfigAppFormat->sPEqParams.bEnable[i]));
        for(j = 0; j < BRAP_FW_MAX_PEQ_BANDS; j++)
        {
            BDBG_MSG(("i32mBandEnable[%d][%d] = %d ,<-> bBandEnable[%d][%d] = %d",
                i,j,pSrsEqHlConfig->sParametricEqConfig.i32mBandEnable[i][j],
                i,j,pSrsEqHlConfigAppFormat->sPEqParams.bBandEnable[i][j]));
        }
    }
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsEqHlConfig->sParametricEqConfig.i32mInputGain,pSrsEqHlConfigAppFormat->sPEqParams.ui32InputGain));
    BDBG_MSG(("i32mOutputGain = %x ,<-> ui32OutputGain = %u",
        pSrsEqHlConfig->sParametricEqConfig.i32mOutputGain,pSrsEqHlConfigAppFormat->sPEqParams.ui32OutputGain));
    BDBG_MSG(("i32mBypassGain = %x ,<-> ui32BypassGain = %u",
        pSrsEqHlConfig->sParametricEqConfig.i32mBypassGain,pSrsEqHlConfigAppFormat->sPEqParams.ui32BypassGain));
    BDBG_MSG(("i32CoefGenMode = %x ,<-> eCoefGenMode = %u",
        pSrsEqHlConfig->sParametricEqConfig.i32CoefGenMode,pSrsEqHlConfigAppFormat->sPEqParams.eCoefGenMode));
    if(pSrsEqHlConfigAppFormat->sPEqParams.eCoefGenMode == 0)
    {
        BDBG_MSG(("Filter coefficients are provided by the user. Refer to corresponding DRAM for the verification"));
    }
    else
    {
        for(i = 0; i < BRAP_FW_MAX_PEQ_BANDS; i++)
        {        
            BDBG_MSG (("sFilterSpecPeq[%d].i32BandGain = %x <-> sPEQFilterSpec[%d].i32BandGain = %d",
                        i,pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].i32BandGain,
                        i,pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].i32BandGain));
            BDBG_MSG (("sFilterSpecPeq[%d].ui32BandFrequency = %x <-> sPEQFilterSpec[%d].ui32BandFrequency = %d",
                        i,pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].ui32BandFrequency,
                        i,pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].ui32BandFrequency));
            BDBG_MSG (("sFilterSpecPeq[%d].i32QFactor = %x <-> sPEQFilterSpec[%d].i32QFactor = %d",
                        i,pSrsEqHlConfig->sParametricEqConfig.uPEQFilterSettings.sFilterSpecPeq[i].i32QFactor,
                        i,pSrsEqHlConfigAppFormat->sPEqParams.uPEQFilterSettings.sPEQFilterSpec[i].i32QFactor));
        }
    }
    
    BDBG_MSG((" HighPassFilterParams:"));
    BDBG_MSG(("ui32mEnable = %x <-> bEnable = %d",
        pSrsEqHlConfig->sHighPassFilterConfig.ui32mEnable,pSrsEqHlConfigAppFormat->sHPFParams.bEnable));
    BDBG_MSG(("ui32CoefGenMode = %x ,<-> eCoefGenMode = %u",
        pSrsEqHlConfig->sHighPassFilterConfig.ui32CoefGenMode,pSrsEqHlConfigAppFormat->sHPFParams.eCoefGenMode));
    if(pSrsEqHlConfigAppFormat->sHPFParams.eCoefGenMode == 0)
    {
        BDBG_MSG(("Filter coefficients are provided by the user. Refer to corresponding DRAM for the verification"));
    }
    else
    {
        BDBG_MSG (("sFilterSpecHpf.ui32CutoffFrequency = %x <-> sHPFilterSpec.ui32CutoffFrequency = %d",
                    pSrsEqHlConfig->sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32CutoffFrequency,
                    pSrsEqHlConfigAppFormat->sHPFParams.uHPFSettings.sHPFilterSpec.ui32CutoffFrequency));
        BDBG_MSG (("sFilterSpecHpf.ui32Order = %x <-> sHPFilterSpec.eFilterOrder = %d",
                    pSrsEqHlConfig->sHighPassFilterConfig.uHPFSettings.sFilterSpecHpf.ui32Order,
                    pSrsEqHlConfigAppFormat->sHPFParams.uHPFSettings.sHPFilterSpec.eFilterOrder));
    }

    BDBG_MSG((" GraphicEqParams:"));
    for(i = 0; i < 2; i++)
    {
        BDBG_MSG(("i32mEnable[%d] = %u <-> bEnable[%d] = %d",
            i,pSrsEqHlConfig->sGraphicEqConfig.i32mEnable[i],i,pSrsEqHlConfigAppFormat->sGEqParams.bEnable[i]));
        for(j = 0; j < BRAP_FW_MAX_GEQ_BANDS; j++)
        {
            BDBG_MSG(("i32mBandGain[%d][%d] = %x ,<-> ui32BandGain[%d][%d] = %u",
                i,j,pSrsEqHlConfig->sGraphicEqConfig.i32mBandGain[i][j],
                i,j,pSrsEqHlConfigAppFormat->sGEqParams.ui32BandGain[i][j]));
        }
    }
    BDBG_MSG(("i32mFilterMode = %x ,<-> ui32FilterMode = %u",
        pSrsEqHlConfig->sGraphicEqConfig.i32mFilterMode,pSrsEqHlConfigAppFormat->sGEqParams.ui32FilterMode));
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsEqHlConfig->sGraphicEqConfig.i32mInputGain,pSrsEqHlConfigAppFormat->sGEqParams.ui32InputGain));
    BDBG_MSG(("i32mOutputGain = %x ,<-> ui32OutputGain = %u",
        pSrsEqHlConfig->sGraphicEqConfig.i32mOutputGain,pSrsEqHlConfigAppFormat->sGEqParams.ui32OutputGain));
    BDBG_MSG(("i32mBypassGain = %x ,<-> ui32BypassGain = %u",
        pSrsEqHlConfig->sGraphicEqConfig.i32mBypassGain,pSrsEqHlConfigAppFormat->sGEqParams.ui32BypassGain));

    BDBG_MSG((" HardLimiterParams:"));
    BDBG_MSG(("i32LimiterEnable = %x <-> bEnable = %d",
        pSrsEqHlConfig->sHardLimiterConfig.i32LimiterEnable,pSrsEqHlConfigAppFormat->sHardLimiterParams.bEnable));
    BDBG_MSG(("i32blockSize = %x ,<-> ui32blockSize = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32blockSize,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32blockSize));
    BDBG_MSG(("i32mInputGain = %x ,<-> ui32InputGain = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32InputGain,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32InputGain));
    BDBG_MSG(("i32mOutputGain = %x ,<-> ui32OutputGain = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32OutputGain,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32OutputGain));
    BDBG_MSG(("i32mBypassGain = %x ,<-> ui32BypassGain = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32BypassGain,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32BypassGain));    
    BDBG_MSG(("i32LimiterBoost = %x ,<-> ui32LimiterBoost = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32LimiterBoost,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32LimiterBoost));    
    BDBG_MSG(("i32HardLimit = %x ,<-> ui32HardLimit = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32HardLimit,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32HardLimit));    
    BDBG_MSG(("i32DelayLength = %x ,<-> ui32DelayLength = %u",
        pSrsEqHlConfig->sHardLimiterConfig.i32DelayLength,pSrsEqHlConfigAppFormat->sHardLimiterParams.ui32DelayLength));        
    BDBG_MSG(("----------------------------------------------------"));     

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)pSrsEqHlConfig,uiActualConfigSize);  

   error:
        if(pSrsEqHlConfig)
            BKNI_Free(pSrsEqHlConfig);

        if(pSrsEqHlConfigAppFormat)
            BKNI_Free(pSrsEqHlConfigAppFormat);

    return err;
}

#ifdef RAP_MULTISTREAM_DECODER_SUPPORT
BERR_Code BRAP_P_MapDDREUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    unsigned int i=0, ui32OutputMode = 2;
    BRAP_FWIF_P_DDReencodeConfigParams  *psDDREFwConfig;
    BDBG_ASSERT(hStage);

    psDDREFwConfig = (BRAP_FWIF_P_DDReencodeConfigParams *)BKNI_Malloc(sizeof(BRAP_FWIF_P_DDReencodeConfigParams));
    if(psDDREFwConfig == NULL)
    {
		err = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        return err;
    }
    BKNI_Memset(psDDREFwConfig, 0, sizeof(BRAP_FWIF_P_DDReencodeConfigParams));          

	/* Total number of output ports enabled. 
	If only stereo output port is set to enabled then the value should be 1,
	if Multichannel 5.1 PCM output is enabled, the value should be 2.
	Only 1 output port with Multichannel 5.1 PCM output port is NOT allowed */
	
    if(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eOutputMode == BRAP_OutputMode_e3_2)
        psDDREFwConfig->ui32NumOutPorts = 2;
    else 
        psDDREFwConfig->ui32NumOutPorts = 1;

    BRAP_DSPCHN_P_GetOutputModeApptoFw(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eOutputMode,
    &ui32OutputMode);

    for(i = 0; i < psDDREFwConfig->ui32NumOutPorts; i++)
    {
        psDDREFwConfig->sUserOutputCfg[i].ui32CompMode = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eCompMode;
        psDDREFwConfig->sUserOutputCfg[i].ui32DrcCutFac = 
            BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.ui32DrcCutFac,100);
        psDDREFwConfig->sUserOutputCfg[i].ui32DrcBoostFac = 
            BRAP_P_FloatToQ131(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.ui32DrcBoostFac,100);

        psDDREFwConfig->sUserOutputCfg[i].ui32StereoMode = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eDownmixType;
        BRAP_DSPCHN_P_GetDualMonoModeApptoFw(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eDualMonoMode,
                (unsigned int *)&(psDDREFwConfig->sUserOutputCfg[i].ui32DualMode));

        if(i == BRAP_DEC_DownmixPath_eMain)
        {
            psDDREFwConfig->sUserOutputCfg[i].ui32OutMode = ui32OutputMode;
            psDDREFwConfig->sUserOutputCfg[i].ui32OutLfe = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.bOutputLfeOn;
            BRAP_P_GetOutputChannelmatrix(hStage->sProcessingStageSettings.uConfigParams.sDDREParams.eOutputMode,
                psDDREFwConfig->sUserOutputCfg[i].ui32OutputChannelMatrix,
                hStage->sProcessingStageSettings.uConfigParams.sDDREParams.bOutputLfeOn);
        }
        else
        {
            psDDREFwConfig->sUserOutputCfg[i].ui32OutMode = 2;
            psDDREFwConfig->sUserOutputCfg[i].ui32OutLfe = 0;
            BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode_e2_0,psDDREFwConfig->sUserOutputCfg[i].ui32OutputChannelMatrix,false);               
        }                
    }
                        
    psDDREFwConfig->ui32ExternalPcmEnabled = 
        (hStage->sProcessingStageSettings.uConfigParams.sDDREParams.bExternalPcmModeEnabled)?1:0;
    psDDREFwConfig->ui32DdreCertificationModeActive = 
        (hStage->sProcessingStageSettings.uConfigParams.sDDREParams.bCertificationModeEnabled)?1:0;    

    if((psDDREFwConfig->ui32ExternalPcmEnabled == 1) || (psDDREFwConfig->ui32DdreCertificationModeActive == 1))
    {
        psDDREFwConfig->ui32InputStreamAcmod = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eInputStreamAcmod;
        psDDREFwConfig->ui32LfePresent = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bInputHasLfe;
        psDDREFwConfig->ui32Dv258ProcessedInput = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.bDv258ProcessedInput;
        psDDREFwConfig->ui32CompProfile = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eProfile;
        psDDREFwConfig->ui32CmixLev = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eCentreMixLevel;
        psDDREFwConfig->ui32SurmixLev = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eSurroundMixLevel;
        psDDREFwConfig->ui32DsurMod = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.eDolbySurroundMode;    
        psDDREFwConfig->ui32DialNorm = hStage->sProcessingStageSettings.uConfigParams.sDDREParams.sExternalPcmSettings.ui32DialogLevel;
    }

    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(psDDREFwConfig),uiActualConfigSize);             

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("DDRE : APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 

    for(i = 0; i < psDDREFwConfig->ui32NumOutPorts; i++)
    {    
        BDBG_MSG(("------------------"));
        BDBG_MSG(("sUserOutputCfg[%d]:",i));
        BDBG_MSG(("ui32CompMode = %x",psDDREFwConfig->sUserOutputCfg[i].ui32CompMode));
        BDBG_MSG(("App ui32DrcCutFac  = %x -> FW ui32DrcCutFac = %x",hStage->sProcessingStageSettings.uConfigParams.\
                        sDDREParams.ui32DrcCutFac,psDDREFwConfig->sUserOutputCfg[i].ui32DrcCutFac));
        BDBG_MSG(("App ui32DrcBoostFac = %x -> FW ui32DrcBoostFac = %x",hStage->sProcessingStageSettings.uConfigParams.\
                        sDDREParams.ui32DrcBoostFac,psDDREFwConfig->sUserOutputCfg[i].ui32DrcBoostFac));
        BDBG_MSG(("ui32OutMode = %x",psDDREFwConfig->sUserOutputCfg[i].ui32OutMode));
        BDBG_MSG(("ui32OutLfe = %x",psDDREFwConfig->sUserOutputCfg[i].ui32OutLfe));
        BDBG_MSG(("ui32StereoMode = %x",psDDREFwConfig->sUserOutputCfg[i].ui32StereoMode));
        BDBG_MSG(("ui32DualMode = %x",psDDREFwConfig->sUserOutputCfg[i].ui32DualMode));
    }
    if((psDDREFwConfig->ui32ExternalPcmEnabled == 1) || (psDDREFwConfig->ui32DdreCertificationModeActive == 1))
    {
        BDBG_MSG(("ui32ExternalPcmEnabled = %d",psDDREFwConfig->ui32ExternalPcmEnabled));
        BDBG_MSG(("ui32CompProfile = %x",psDDREFwConfig->ui32CompProfile));
        BDBG_MSG(("ui32CmixLev = %x",psDDREFwConfig->ui32CmixLev));
        BDBG_MSG(("ui32SurmixLev = %x",psDDREFwConfig->ui32SurmixLev));
        BDBG_MSG(("ui32DsurMod = %x",psDDREFwConfig->ui32DsurMod));
        BDBG_MSG(("ui32DialNorm = %x",psDDREFwConfig->ui32DialNorm));
        BDBG_MSG(("ui32NumOutPorts = %x",psDDREFwConfig->ui32NumOutPorts));
    }
    else
    {
        BDBG_MSG(("Both External PCM and Certificaion modes are disabled, External PCM settings are not valid"));
    }
    
    if(psDDREFwConfig)
        BKNI_Free(psDDREFwConfig);
    
    return err;
}
#endif

BERR_Code BRAP_P_MapBtscUserConfigApptoFw(
    BRAP_ProcessingStageHandle  hStage,     /* [in] Stage handle */
    unsigned int    uiConfigBufAddr,    /* [in] Config Buf Address */
    unsigned int   uiActualConfigSize 
    )
{
    BERR_Code err = BERR_SUCCESS;    
    BRAP_FWIF_P_BTSCEncodeConfigParams    sBtscFwConfig;
    BDBG_ASSERT(hStage);

    sBtscFwConfig.eBTSCEnableFlag = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bBTSCEnableFlag;
    sBtscFwConfig.eUseDeEmphasizedSourceSignals = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bUseDeEmphasizedSourceSignals;
    sBtscFwConfig.eMainChannelPreEmphasisOn = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bMainChannelPreEmphasisOn;
    sBtscFwConfig.eEquivalenMode75MicroSec = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bEquivalenMode75MicroSec;
    sBtscFwConfig.eClipDiffChannelData = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bClipDiffChannelData;    
    sBtscFwConfig.eDiffChannelPreEmphasisOn = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bDiffChannelPreEmphasisOn;
    sBtscFwConfig.eUseLowerThresholdSpGain = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bUseLowerThresholdSpGain;


    if(hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.uiVolumeLevel >= 0x2000000)
    {
        sBtscFwConfig.ui32AttenuationFactor = 0x7fffffff;
    }
    else
    {
        sBtscFwConfig.ui32AttenuationFactor = (hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.uiVolumeLevel << 6);/*Fw requires it in Q3.29*/
    }
       
    sBtscFwConfig.ui32SRDCalibrationFactor = BRAP_P_FloatToQ428(hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32SRDCalibrationFactor,1000);    

    sBtscFwConfig.eSumChanFreqDevCtrl = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bSumChanFreqDevCtrl;
    sBtscFwConfig.eDiffChanFreqDevCtrl = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bDiffChanFreqDevCtrl;
    sBtscFwConfig.eOpFreqDevCtrl = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bOpFreqDevCtrl;
    sBtscFwConfig.eFreqGeneration = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bFreqGeneration;
    sBtscFwConfig.i32Frequency = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32Frequency;        
               
    if(hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32SumChannelLevelCtrl >= 0x2000000)
    {
        sBtscFwConfig.i32SumChannelLevelCtrl = 0x7fffffff;
    }
    else
    {
        sBtscFwConfig.i32SumChannelLevelCtrl = (hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32SumChannelLevelCtrl << 6);/*Fw requires it in Q3.29*/
    }

    if(hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32DiffChannelLevelCtrl >= 0x2000000)
    {
        sBtscFwConfig.i32DiffChannelLevelCtrl = 0x7fffffff;
    }
    else
    {
        sBtscFwConfig.i32DiffChannelLevelCtrl = (hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32DiffChannelLevelCtrl << 6);/*Fw requires it in Q3.29*/
    }    	

    sBtscFwConfig.eBTSCMonoModeOn = hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.bBTSCMonoModeOn;	
    
    BKNI_Memcpy((void *)(volatile void *)uiConfigBufAddr,(void *)(&sBtscFwConfig),uiActualConfigSize);

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("Btsc :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 

    BDBG_MSG((" eBTSCEnableFlag = %d",sBtscFwConfig.eBTSCEnableFlag));
    BDBG_MSG((" eUseDeEmphasizedSourceSignals  = %d",sBtscFwConfig.eUseDeEmphasizedSourceSignals ));
    BDBG_MSG((" eMainChannelPreEmphasisOn = %d",sBtscFwConfig.eMainChannelPreEmphasisOn));
    BDBG_MSG((" eEquivalenMode75MicroSec = %d",sBtscFwConfig.eEquivalenMode75MicroSec));
    BDBG_MSG((" eClipDiffChannelData = %d",sBtscFwConfig.eClipDiffChannelData));
    BDBG_MSG((" eDiffChannelPreEmphasisOn = %d",sBtscFwConfig.eDiffChannelPreEmphasisOn));
    BDBG_MSG((" eUseLowerThresholdSpGain = %d",sBtscFwConfig.eUseLowerThresholdSpGain));
    BDBG_MSG((" uiVolumeLevel = %d ->ui32AttenuationFactor = %d",hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.uiVolumeLevel,sBtscFwConfig.ui32AttenuationFactor));
    BDBG_MSG((" ui32SRDCalibrationFactor = %d -> ui32SRDCalibrationFactor  = %d",hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32SRDCalibrationFactor,sBtscFwConfig.ui32SRDCalibrationFactor ));
    BDBG_MSG((" eSumChanFreqDevCtrl = %d",sBtscFwConfig.eSumChanFreqDevCtrl));
    BDBG_MSG((" eDiffChanFreqDevCtrl = %d",sBtscFwConfig.eDiffChanFreqDevCtrl));
    BDBG_MSG((" eOpFreqDevCtrl = %d",sBtscFwConfig.eOpFreqDevCtrl));
    BDBG_MSG((" eFreqGeneration = %d",sBtscFwConfig.eFreqGeneration));
    BDBG_MSG((" i32Frequency = %d",sBtscFwConfig.i32Frequency));
    BDBG_MSG((" ui32SumChannelLevelCtrl = %d -> i32SumChannelLevelCtrl = %d",hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32SumChannelLevelCtrl,sBtscFwConfig.i32SumChannelLevelCtrl));
    BDBG_MSG((" ui32DiffChannelLevelCtrl = %d -> i32DiffChannelLevelCtrl = %d",hStage->sProcessingStageSettings.uConfigParams.sBTSCParams.ui32DiffChannelLevelCtrl,sBtscFwConfig.i32DiffChannelLevelCtrl));
    BDBG_MSG((" eBTSCMonoModeOn = %d",sBtscFwConfig.eBTSCMonoModeOn));

    BDBG_MSG(("-------------------------------------------")); 
    return err;
}



/* Function to convert input floating point value into Q1.31 format
 * Intended floating point value to be converted, should be
 * multiplied by uiRange value and then passed to this function
 */
static uint32_t BRAP_P_FloatToQ131(int32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	if (floatVar >= (int32_t)uiRange) return (uint32_t) 0x7FFFFFFF;
	if (floatVar <= (int32_t)-uiRange) return (uint32_t) 0x80000000;

	/* Conversion formula for float to Q1.31 is
	 * q = float * 2^31,
	 * Since we take float values scaled by uiRange from application, we need to scale it down
	 * by uiRange. Hence above formula would become
	 * q = float * ( 2^31/uiRange )
	 * However this won't be a precise computation, as reminder of (2^31/uiRange) gets dropped
	 * in this calculation. To compesate for this reminder formula needs to be modified as below
	 * q = float * ( 2^31/uiRange ) + (float * (2^31 %uiRange))/uiRange
	 */

    temp = floatVar * (0x80000000/uiRange) + ( floatVar * (0x80000000 % uiRange))/uiRange;

	return temp; 
}

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
static uint32_t BRAP_P_FloatToQ230(int16_t floatVar)
{
	int32_t		temp;

	BDBG_ASSERT(floatVar >= 0);
	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q2.30 is
	 * q = float * 2^30,
	 * Since we take float values scaled by 100 from application, we need to scale it down
	 * by 100. Hence above formula would become
	 * q = float * ( 2^30/100 )
	 * However this won't be a precise computation, as reminder of (2^30/100) gets dropped
	 * in this calculation. To compesate for this reminder formula needs to be modified as below
	 * q = float * ( 2^30/100 ) + ( float * ( 2^30 %100))/100
	 */
	if (floatVar >= 100) return (uint32_t) 0x40000000;

	temp = floatVar * (0x40000000/100) + ( floatVar * (0x40000000 % 100))/100;

	return temp; 
}

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
static int32_t BRAP_P_FloatToQ1022(int32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q10.22 is
	 * q = float * 2^22,
	 * Since the entire range of values in PI is mapped to the range of 0 to 2^22 in FW, 
	 * a given value in PI gets converted to corresponding Q10.22 value as below,
	 * q = float * ( 2^22/uiRange )
	 * However this won't be a precise computation, as remainder of (2^22/uiRange) gets dropped
	 * in this calculation. To compensate for this remainder formula needs to be modified as below
	 * q = float * ( 2^22/uiRange ) + ( float * ( 2^22 % uiRange))/uiRange
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = float * ( 2^22/uiRange ) + (unsigned int)((float * ( 2^22 % uiRange) + (uiRange/2))/uiRange)
	 */
	 

        if(floatVar >= 0)
        {
        temp = floatVar * (0x003FFFFF/uiRange) + (unsigned int)(( floatVar * (0x003FFFFF % uiRange) + (uiRange/2))/uiRange);
        }
        else
        {
            floatVar = (-1)*floatVar;
        temp = floatVar * (0x003FFFFF/uiRange) + (unsigned int)(( floatVar * (0x003FFFFF % uiRange) + (uiRange/2))/uiRange);
        temp = (-1)*temp;
        }
	return temp;
}

/*
    This function converts the PI value to Firmware fixed format.
    If it is Q21, lower 21 bits are used for Decimal value representation and
    upper 11 bits for Integer. Q21 represents x.21 i.e., 5.21, 7.21, 11.21 etc.,
    depending on the range of the integer part supported
*/
static uint32_t BRAP_P_ConvertPIToFwFormat(uint32_t PI_Value, unsigned int uiScaleFactor, unsigned int uiMaxPIValue, uint32_t ui32FWFormat)
{
	int32_t		temp;
    uint32_t    ui32ScaledValue;

	/* TODO: conversion for negative values */
	/* Consider FW format is 2^21. Conversion formula for PI value to Q21 is
	 * q = Spec_value * 2^21, (Spec_value -> value as per the algorithm specification range)
	 * Since floating point values are not allowed, a scaling(multiply) factor 
	 * is employed to support minimum precision required , thus PI_Value=Spec_value*uiScaleFactor
	 * So a given value in PI gets converted to corresponding Q21 value as below,
	 * q = (PI_Value/uiScaleFactor) * 2^21
	 * However this won't be a precise computation, as remainder of (PI_Value/uiScaleFactor) gets dropped
	 * in this calculation. To compensate for this remainder formula needs to be modified as below
	 * q = (PI_Value/uiScaleFactor) * 0x001FFFFF + (unsigned int) (((PI_Value % uiScaleFactor) * 0x001FFFFF) /uiScaleFactor)
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiScaleFactor/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = (PI_Value/uiScaleFactor) * 0x001FFFFF + (unsigned int) ( ((PI_Value % uiScaleFactor) * 0x001FFFFF  + (uiScaleFactor/2) ) /uiScaleFactor)

	 * However FW treats the upper most bit(what ever be the bitwidth) as Sign indicator. 
	 * So for the Max value return Max -1 i.e., 0x8000 -1 = 0x7FFF.
	 */

	if (PI_Value >= uiMaxPIValue) /*return (uint32_t)((ui32FWFormat * (uiMaxPIValue/uiScaleFactor))-1);*/
        ui32ScaledValue = uiMaxPIValue;
    else
        ui32ScaledValue = PI_Value;

    temp = (ui32ScaledValue/uiScaleFactor) * ui32FWFormat + (unsigned int) ( ((ui32ScaledValue % uiScaleFactor) * ui32FWFormat  + (uiScaleFactor/2) ) /uiScaleFactor);

    /* Upper most bit is a Sign indicator */
    if (PI_Value >= uiMaxPIValue)
        temp-=1;
    
	return temp; 
}

#if 0/*Enable when used*/
/*
    This function converts the floating point value to 5.18 fixed format
*/
static uint32_t BRAP_P_FloatToQ518(uint32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q5.18 is
	 * q = float * 2^23,
	 * Since the entire range of values in PI is mapped to the range of 0 to 2^23 in FW, 
	 * a given value in PI gets converted to corresponding Q5.18 value as below,
	 * q = float * ( 2^23/uiRange )
	 * However this won't be a precise computation, as remainder of (2^23/uiRange) gets dropped
	 * in this calculation. To compensate for this remainder formula needs to be modified as below
	 * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange) + (uiRange/2))/uiRange)
	 */

	if (floatVar >= uiRange) return (uint32_t)0x007FFFFF;

    temp = floatVar * (0x007FFFFF/uiRange) + (unsigned int)(( floatVar * (0x007FFFFF % uiRange) + (uiRange/2))/uiRange);

	return temp; 
}


/*
    This function converts the floating point value to 8.15 fixed format
*/
static uint32_t BRAP_P_FloatToQ815(uint32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q8.15 is
	 * q = float * 2^23,
	 * Since the entire range of values in PI is mapped to the range of 0 to 2^23 in FW, 
	 * a given value in PI gets converted to corresponding Q8.15 value as below,
	 * q = float * ( 2^23/uiRange )
	 * However this won't be a precise computation, as remainder of (2^23/uiRange) gets dropped
	 * in this calculation. To compensate for this remainder formula needs to be modified as below
	 * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange) + (uiRange/2))/uiRange)
	 */

	if (floatVar >= uiRange) return (uint32_t)0x007FFFFF;

    temp = floatVar * (0x007FFFFF/uiRange) + (unsigned int)(( floatVar * (0x007FFFFF % uiRange) + (uiRange/2))/uiRange);

	return temp; 
}
#endif

/*
    This function converts the floating point value to 8.24 fixed format
*/
static int32_t BRAP_P_FloatToQ824(int32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

           /*Range should be multiple of 127 i.e. 127,254,381,508.....*/
        
        if (floatVar >= (int32_t)uiRange) return (uint32_t) 0x7F000000;
        if (floatVar <= -(int32_t)(uiRange + (uiRange/127))) return (uint32_t)0x80000000;
    
        temp = floatVar * (0x7F000000/uiRange) + (unsigned int)(( floatVar * (0x7F000000 % uiRange) + (uiRange/2))/uiRange);
    
        return temp; 
    }


/*
    This function converts the floating point value to 9.23 fixed format
*/
static int32_t BRAP_P_FloatToQ923(uint32_t floatVar, unsigned int uiRange)
{
     int32_t  temp;

	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q9.23 is
	 * q = float * 2^23,
	 * Since the entire range of values in PI is mapped to the range of 0 to 2^23 in FW, 
	 * a given value in PI gets converted to corresponding Q9.23 value as below,
	 * q = float * ( 2^23/uiRange )
	 * However this won't be a precise computation, as remainder of (2^23/uiRange) gets dropped
	 * in this calculation. To compensate for this remainder, formula needs to be modified as below
	 * q = float * ( 2^23/uiRange ) + ( float * ( 2^23 % uiRange))/uiRange
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = float * ( 2^23/uiRange ) + (unsigned int)((float * ( 2^23 % uiRange) + (uiRange/2))/uiRange)
	 */

     if (floatVar >= uiRange) return (uint32_t)0x007fffff;
     
     temp = floatVar *(0x007fffff/uiRange) + (unsigned int)(((floatVar * (0x7fffff % uiRange))+(uiRange/2))/uiRange);
     
     return temp; 
}

/*
    This function converts the floating point value to 5.27 fixed format
*/
static uint32_t BRAP_P_FloatToQ527(uint32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* TODO: conversion for negative values */
	/* Conversion formula for float to Q5.27 is
	 * q = float * 2^27,
	 * Since the entire range of values in PI is mapped to the range of 0 to 2^27 in FW, 
	 * a given value in PI gets converted to corresponding Q5.27 value as below,
	 * q = float * ( 2^27/uiRange )
	 * However this won't be a precise computation, as remainder of (2^27/uiRange) gets dropped
	 * in this calculation. To compensate for this remainder formula needs to be modified as below
	 * q = float * ( 2^27/uiRange ) + ( float * ( 2^27 % uiRange))/uiRange
	 * But if the value corresponding to the multiplication of reminder turns out to be 
	 * a fractional value then the value gets rounded off to zero but if the value is >= uiRange/2
	 * it should be rounded off to 1(as per SRS). Hence forth the formula becomes,
	 * q = float * ( 2^27/uiRange ) + (unsigned int)((float * ( 2^27 % uiRange) + (uiRange/2))/uiRange)
	 * 0x08000000 = 2^27 + 1, the value 2^27 is being rounded off as that will be the precise value in
	 * decoder implementation. Either ways of using 2^27 or 0x08000000 doesn't result in much difference.
	 */

    temp = floatVar * (0x08000000/uiRange) + (unsigned int)(( floatVar * (0x08000000 % uiRange) + (uiRange/2))/uiRange);

	return temp; 
}


/* Function to convert input floating point value into Q3.29 format
 * Intended floating point value to be converted, should be
 * multiplied by uiRange value and then passed to this function
 */
static uint32_t BRAP_P_FloatToQ329(uint32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* Conversion formula for float to Q3.29 is
	 * q = float * 2^29,
	 * Since we take float values scaled by uiRange from application, we need to scale it down
	 * by uiRange. Hence above formula would become
	 * q = float * ( 2^29/uiRange )
	 * However this won't be a precise computation, as reminder of (2^29/uiRange) gets dropped
	 * in this calculation. To compesate for this reminder formula needs to be modified as below
	 * q = float * ( 2^29/uiRange ) + (float * (2^29 %uiRange))/uiRange
	 */

    temp = floatVar * (0x1FFFFFFF/uiRange) + ( floatVar * (0x1FFFFFFF % uiRange))/uiRange;

	return temp; 
}

/* Function to convert input floating point value into Q4.28 format
 * Intended floating point value to be converted, should be
 * multiplied by uiRange value and then passed to this function
 */
static uint32_t BRAP_P_FloatToQ428(uint32_t floatVar, unsigned int uiRange)
{
	int32_t		temp;

	/* Conversion formula for float to Q4.28 is
	 * q = float * 2^28,
	 * Since we take float values scaled by uiRange from application, we need to scale it down
	 * by uiRange. Hence above formula would become
	 * q = float * ( 2^28/uiRange )
	 * However this won't be a precise computation, as reminder of (2^28/uiRange) gets dropped
	 * in this calculation. To compesate for this reminder formula needs to be modified as below
	 * q = float * ( 2^28/uiRange ) + (float * (2^28 %uiRange))/uiRange
	 */

    temp = floatVar * (0x10000000/uiRange) + ( floatVar * (0x10000000 % uiRange))/uiRange;
     
     return temp; 
}

static void BRAP_DSPCHN_P_GetDualMonoModeApptoFw(BRAP_DSPCHN_DualMonoMode eDualMonoMode,unsigned int *uiDualMonoMode)
{
	switch (eDualMonoMode) {
                case BRAP_DSPCHN_DualMonoMode_eLeftMono: *uiDualMonoMode = 1; break;
                case BRAP_DSPCHN_DualMonoMode_eRightMono: *uiDualMonoMode = 2; break;
                case BRAP_DSPCHN_DualMonoMode_eStereo: *uiDualMonoMode = 0; break;
                case BRAP_DSPCHN_DualMonoMode_eDualMonoMix: *uiDualMonoMode = 3; break;                
	    }

    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("eDualMonoMode :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
	BDBG_MSG((" eDualMonoMode = %d",eDualMonoMode));
    BDBG_MSG(("-------------------------------------------")); 
}
static void BRAP_DSPCHN_P_GetDualMonoModeFwtoApp(unsigned int uiDualMonoMode,BRAP_DSPCHN_DualMonoMode *eDualMonoMode)
{
    switch(uiDualMonoMode){
        case 1: *eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono; break;
        case 2: *eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eRightMono; break;
        case 0: *eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo; break;
        case 3: *eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eDualMonoMix; break;        
        }
}
static void BRAP_DSPCHN_P_GetOutputModeApptoFw(BRAP_OutputMode eOutputMode,unsigned int *uiOutputMode)
{
	switch (eOutputMode) {
		case BRAP_OutputMode_e1_0: *uiOutputMode = 1; break;
		case BRAP_OutputMode_e1_1: *uiOutputMode = 0; break;
		case BRAP_OutputMode_e2_0: *uiOutputMode = 2; break;
		case BRAP_OutputMode_e3_0: *uiOutputMode = 3; break;
		case BRAP_OutputMode_e2_1: *uiOutputMode = 4; break;
		case BRAP_OutputMode_e3_1: *uiOutputMode = 5; break;
		case BRAP_OutputMode_e2_2: *uiOutputMode = 6; break;
		case BRAP_OutputMode_e3_2: *uiOutputMode = 7; break;
		case BRAP_OutputMode_e3_3: *uiOutputMode = 8; break;
		case BRAP_OutputMode_e3_4: *uiOutputMode = 9; break;				
		default: BDBG_ASSERT(0); break;
	}
    BDBG_MSG(("-------------------------------------------")); 
    BDBG_MSG(("OutputMode :: APP->FW CONFIG VALUES")); 
    BDBG_MSG(("-------------------------------------------")); 
	BDBG_MSG((" eOutputMode = %d , *uiOutputMode = %d",eOutputMode,*uiOutputMode));
    BDBG_MSG(("-------------------------------------------")); 

    return;
}
static void BRAP_DSPCHN_P_GetOutputModeFwtoApp(unsigned int uiOutputMode , BRAP_OutputMode *eOutputMode)
{
	switch (uiOutputMode) {
		case 1: *eOutputMode = BRAP_OutputMode_e1_0; break;
		case 0: *eOutputMode = BRAP_OutputMode_e1_1; break;
		case 2: *eOutputMode = BRAP_OutputMode_e2_0; break;
		case 3: *eOutputMode = BRAP_OutputMode_e3_0; break;
		case 4: *eOutputMode = BRAP_OutputMode_e2_1; break;
		case 5: *eOutputMode = BRAP_OutputMode_e3_1; break;
		case 6: *eOutputMode = BRAP_OutputMode_e2_2; break;
		case 7: *eOutputMode = BRAP_OutputMode_e3_2; break;
		case 8: *eOutputMode = BRAP_OutputMode_e3_3; break;
		case 9: *eOutputMode = BRAP_OutputMode_e3_4; break;				
		default: BDBG_ASSERT(0); break;
	}
    return;
}
static void BRAP_P_GetOutputChannelmatrix(BRAP_OutputMode eOutputMode,uint32_t *pui32outputChannel,bool bLfe)
{
    uint32_t i=0;
    for(i = 0; i<BRAP_AF_P_MAX_CHANNELS; i++)
    {
        pui32outputChannel[i] = ui32OutputChannelMatrix[eOutputMode][i];
    }
    pui32outputChannel[BRAP_P_LFE_CHAN_INDEX] = 0xFFFFFFFF;    
    if(eOutputMode == BRAP_OutputMode_e3_2)
    {
    if(true ==bLfe)
    {
        pui32outputChannel[BRAP_P_LFE_CHAN_INDEX] = BRAP_P_LFE_CHAN_INDEX;
    }
    }
    return;
}

