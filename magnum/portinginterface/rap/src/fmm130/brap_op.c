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
*   Module name: OUTPUT
*   This file contains the implementation of all PIs for the Output Port
*   abstraction. 
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_output);

#define BRAP_OP_P_DEFAULT_I2S_BITS_PER_SAMPLE   24
#define BRAP_OP_P_DEFAULT_MAI_BITS_PER_SAMPLE   20
  

/* the I2S register names differ for 7411, 7401 and 7400. 
Added these macros so that we can reuse the I2S code for all 3 chips */
#define BRAP_FIELD_ENUM(x,y,z) BCHP_FIELD_ENUM(x,y,z)
#define BRAP_FIELD_DATA(x,y,z) BCHP_FIELD_DATA(x,y,z)
#define BRAP_MASK(x,y) BCHP_MASK(x,y)

#ifdef BCHP_7411_VER /* Only for the 7411 */ 
#define AUD_FMM_OP_CTRL_MCLK_CFG_I2S AUD_FMM_OP_CTRL_MCLK_CFG_I2Si
#define AUD_FMM_OP_CTRL_I2S_CFG AUD_FMM_OP_CTRL_I2S_CFG_i
#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2Si_ARRAY_BASE
#define BCHP_AUD_FMM_OP_CTRL_I2S_CFG BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE

#elif (BRAP_7401_FAMILY == 1)
#define AUD_FMM_OP_CTRL_MCLK_CFG_I2S AUD_FMM_OP_CTRL_MCLK_CFG_I2S0
#define AUD_FMM_OP_CTRL_I2S_CFG AUD_FMM_OP_CTRL_I2S_CFG_0
#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S0
#define BCHP_AUD_FMM_OP_CTRL_I2S_CFG BCHP_AUD_FMM_OP_CTRL_I2S_CFG_0

#elif (BCHP_CHIP == 7400)
#define AUD_FMM_OP_CTRL_MCLK_CFG_I2S AUD_FMM_OP_CTRL_MCLK_CFG_I2Si
#define AUD_FMM_OP_CTRL_I2S_CFG AUD_FMM_OP_CTRL_I2S_CFG_i
#define BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2Si_ARRAY_BASE
#define BCHP_AUD_FMM_OP_CTRL_I2S_CFG BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE
#endif
  
#if 0 /* TBD: in next phase */
/* The following table contains the BCHP_AUD_FMM_PLL0_USER_DIV1, 
   BCHP_AUD_FMM_PLL0_USER_DIV2 and BCHP_AUD_FMM_PLL0_DIVIDER 
   register values for the supported sampling rates 
*/
static const BRAP_OP_P_PLLSampleRate PLLSR[] =
{
    { BAVC_AudioSamplingRate_e32k,    0x000d785f, 0x000001cf, 0x00007653},
    { BAVC_AudioSamplingRate_e44_1k,  0x0002308d, 0x000001cf, 0x00004e53},
    { BAVC_AudioSamplingRate_e48k,    0x000d786f,,0x000001cf, 0x00004e53},
    { BAVC_AudioSamplingRate_e96k,    0x000e786f,0x000001cf, 0x00002653},
    { BAVC_AudioSamplingRate_e192k,   0x000e788f,0x000001cf, 0x00000e53} 
};
#endif

const BRAP_OP_P_I2sSettings defI2sSettings = 
{   
    { 
        BRAP_OP_ChannelOrder_eNormal, /* sExtParams.eChannelOrder */
        false,                        /* sExtParams.bLsbAtLRClk */
        true,                         /* sExtParams.bAlignedToLRClk */
        false,                        /* sExtParams.bLRClkPolarity */
        BRAP_OP_SClkRate_e64Fs,       /* sExtParams.eSClkRate */
        BRAP_OP_MClkRate_e256Fs,      /* sExtParams.eMClkRate */
        BRAP_OP_Pll_ePll1             /* sExtParams.ePll */
    } 
};
const BRAP_OP_P_I2sParams defI2sParams = 
{   
    BRAP_OP_P_DEFAULT_I2S_BITS_PER_SAMPLE,   /* uiBitsPerSample */
    BAVC_Timebase_e0                         /* eTimebase */
};
const BRAP_OP_P_SpdifSettings defSpdifSettings = 
{
    { 
        BRAP_OP_ChannelOrder_eNormal, /* sExtParams.eChannelOrder */
        false,                        /* sExtParams.bLimitTo16Bits */
        BRAP_OP_MClkRate_e256Fs,      /* sExtParams.eMClkRate */
        BRAP_OP_Pll_ePll1             /* sExtParams.ePll */
    }         
};
const BRAP_OP_P_SpdifParams defSpdifParams = 
{
    BAVC_Timebase_e0      /* eTimebase */
};

const BRAP_OP_SpdifChanStatusParams defChanStatusParams =
{
	/* sChanStatusParams */
	false,  /* bProfessionalMode = Consumer Mode */
	false, /* bLinear_PCM = true for linear output */
    	false,  /* bSwCopyRight = Not asserted */
    	0, /*  "0" = 2 audio channels without pre-emphasis */
    	0, /* mode 0 */
	0,      /* ui16CategoryCode = 0 */
	0, /* source number */
    	0,      /* bSeparateLRChanNum = 0 =>Left=right channel num=0000*/
   	0,       /* ui16ClockAccuracy = 0 (Level II) */
    	0, /* "0" - Maximum audio sample word length is 20 bits*/
    	0, /* "1" - 20 bits */
    	0, /* 48k -Original sampling frequency */
    	0 /* No copying is permitted */

};

const BRAP_OP_P_DacParams defDacParams = 
{
    BAVC_Timebase_e0      /* eTimebase */
};

const BRAP_OP_P_DacSettings defDacSettings = 
{
  {
    BRAP_OP_ChannelOrder_eNormal,      /* sExtParams.eChannelOrder */    
    BRAP_OP_DacMuteType_SquareWaveSame /* sExtParams.eMuteType */
  }
};

#ifndef BCHP_7411_VER
const BRAP_OP_P_MaiSettings defMaiSettings = 
{
  {
	BRAP_OutputPort_eSpdif,
	BRAP_OP_ChannelOrder_eNormal, 
	BRAP_OP_MaiAudioFormat_eSpdifLinearPcm,
	true
  }
};

const BRAP_OP_P_MaiParams defMaiParams = 
{   
    BRAP_OP_P_DEFAULT_MAI_BITS_PER_SAMPLE,   /* uiSampleWidth */
    BAVC_Timebase_e0                         /* eTimebase */
};

const BRAP_OP_P_FlexSettings defFlexSettings = 
{
  {
       BRAP_OP_SClkRate_e64Fs,       /* sExtParams.eSClkRate */
       BRAP_OP_MClkRate_e256Fs,      /* sExtParams.eMClkRate */
       BRAP_OP_Pll_ePll1             /* sExtParams.ePll */
  }
};
#endif

static BERR_Code
BRAP_OP_P_SpdifOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_SpdifSettings * pSettings /*[in] Start time parameters */
);
static BERR_Code
BRAP_OP_P_SpdifStart (
    BRAP_OP_P_Handle        hOp,             /*[in] Output port handle */
    const BRAP_OP_P_SpdifParams * pParams    /*[in] Start time parameters */
);
static BERR_Code
BRAP_OP_P_SpdifStop (
    BRAP_OP_P_Handle        hOp              /*[in] Output port handle */
);

static BERR_Code
BRAP_OP_P_I2sOpen (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_I2sSettings * pSettings  /*[in] Open time parameters */
);
static BERR_Code
BRAP_OP_P_I2sStart (
    BRAP_OP_P_Handle        hOp,             /*[in] Output port handle */
    const BRAP_OP_P_I2sParams *   pParams    /*[in] Start time parameters */
);
static BERR_Code
BRAP_OP_P_I2sStop (
    BRAP_OP_P_Handle        hOp              /*[in] Output port handle */
);

#ifndef BCHP_7411_VER
static BERR_Code
BRAP_OP_P_MaiOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_MaiSettings * pSettings /*[in] Start time parameters */
);

static BERR_Code
BRAP_OP_P_MaiStart (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_MaiParams * pParams /*[in] Start time parameter */
);

static BERR_Code
BRAP_OP_P_MaiStop (
    BRAP_OP_P_Handle        hOp    /*[in] Output port handle */
);


static BERR_Code
BRAP_OP_P_FlexOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_FlexSettings * pSettings /*[in] Open parameter */
);
#endif

BERR_Code 
BRAP_OP_P_GetDefaultSpdifChanStatusParams ( 
    BRAP_OP_SpdifChanStatusParams *  pDefParams   /*[out] Pointer to 
                           memory where default parameters should be written */
)
{
    BDBG_ENTER (BRAP_OP_P_GetDefaultSpdifChanStatusParams);
    BDBG_ASSERT (pDefParams);      

    *pDefParams = defChanStatusParams;
    
    BDBG_LEAVE (BRAP_OP_P_GetDefaultSettings);
    return BERR_SUCCESS;
}


BERR_Code 
BRAP_OP_P_GetDefaultSettings ( 
    BRAP_OutputPort  eOutput,      /*[in] Ouput Port type */
    void *       pDefSettings  /*[out] Pointer to memory where default
                                 settings should be written */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_OP_P_GetDefaultSettings);

    BDBG_ASSERT (pDefSettings);            
    
    switch (eOutput)
    {
        case BRAP_OutputPort_eSpdif:
            *((BRAP_OP_P_SpdifSettings *)pDefSettings) = defSpdifSettings;
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            *((BRAP_OP_P_I2sSettings *)pDefSettings) = defI2sSettings;
            break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
	case BRAP_OutputPort_eDac0:
	case BRAP_OutputPort_eDac1:
		*((BRAP_OP_P_DacSettings *)pDefSettings) = defDacSettings;
		break;
	case BRAP_OutputPort_eMai:
		*((BRAP_OP_P_MaiSettings *)pDefSettings) = defMaiSettings;
		break;
	case BRAP_OutputPort_eFlex:
		*((BRAP_OP_P_FlexSettings*)pDefSettings) = defFlexSettings;
		break;
#endif
        default:
            BDBG_ERR (("BRAP_OP_P_GetDefaultSettings(): Output port type not supprted %d ",
                        eOutput));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    BDBG_LEAVE (BRAP_OP_P_GetDefaultSettings);
    return ret;
}



BERR_Code 
BRAP_OP_P_GetDefaultParams ( 
    BRAP_OutputPort  eOutput,     /*[in] Ouput Port type */
    void *       pDefParams   /*[out] Pointer to memory where default
                                 settings should be written */     
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_OP_P_GetDefaultParams);

    BDBG_ASSERT (pDefParams);            
    
    switch (eOutput)
    {
        case BRAP_OutputPort_eSpdif:
            *((BRAP_OP_P_SpdifParams *)pDefParams) = defSpdifParams;
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            *((BRAP_OP_P_I2sParams *)pDefParams) = defI2sParams;
            break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
        case BRAP_OutputPort_eMai:
            *((BRAP_OP_P_MaiParams *)pDefParams) = defMaiParams;
            break;
	case BRAP_OutputPort_eDac0:
	case BRAP_OutputPort_eDac1:
		*((BRAP_OP_P_DacParams *)pDefParams) = defDacParams;
		break;
#endif
        default:
            BDBG_ERR (("BRAP_OP_P_GetDefaultParams(): Output port type not supprted %d",
                        eOutput));
            return BERR_TRACE (BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);
    }

    BDBG_LEAVE (BRAP_OP_P_GetDefaultParams);
    return ret;
}


BERR_Code BRAP_OP_P_Open (
    BRAP_FMM_P_Handle  hFmm,       /*[in] Parent FMM Handle */
    BRAP_OP_P_Handle * phOp,       /*[out] Ouput Port handle */
    BRAP_OutputPort    eOutput,    /*[in] Ouput Port type */
    const void *       pSettings   /*[in] Ouput Port settings */ 

)
{
    BRAP_OP_P_Handle hOp;
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegVal=0;
    BAVC_AudioSamplingRate eOutputSR = BAVC_AudioSamplingRate_eUnknown;
#ifndef BCHP_7411_VER
    BRAP_OP_P_MaiSettings sTemp1;
    BRAP_OP_P_SpdifSettings sSpdifpsettings;
    BRAP_OP_P_FlexSettings sFlexpsettings;
    BRAP_OP_P_Handle hSpdifOp;
    BRAP_OP_P_Handle hFlexOp;
#endif
    BDBG_ENTER (BRAP_OP_P_Open);

    /* 1. Check all input parameters to the function. Return error if
      - FMM handle is NULL
      - Given ouput type is invalid
      - Pointer to Settings structure is NULL
      - Pointer to Ouput Port handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (phOp);
    BSTD_UNUSED(sTemp1);

    eOutputSR = hFmm->hRap->sOutputSettings[eOutput].eOutputSR;
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pSettings);

        if (hFmm->hOp[eOutput] != NULL) 
        {
            /* This Outport was previoulsy opened by another audio channel.
            So this time, just increment the open count. Dont change the 
            settings etc. */
            /* OpenCount cannot exceed the no. of Mixer inputs!! */
            hOp = hFmm->hOp[eOutput];
        
            if (hOp->uiOpenCnt >= BRAP_RM_P_MAX_MIXER_INPUTS)
            {
                BDBG_ERR (("BRAP_OP_P_Open: Output port %d already has %d open"
                        "audio channels. Cannot add another!!!",
                        eOutput, hOp->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
        
            *phOp = hOp;
            hOp->uiOpenCnt ++;
#ifndef BCHP_7411_VER            
            /* if it is Mai, then the Flex/Spdif count has to be incremented also */
	     if (eOutput == BRAP_OutputPort_eMai)
	    {
	           sTemp1 = *(BRAP_OP_P_MaiSettings*)pSettings;
		    if (sTemp1.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eSpdif)
				hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uiOpenCnt ++;
		    else if (sTemp1.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eFlex)
				hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uiOpenCnt ++;			
	     }
#endif
            BDBG_MSG (("BRAP_OP_P_Open: Output port %d was already open." 
                    "New open count = %d", 
                    eOutput, hOp->uiOpenCnt));
            BDBG_LEAVE (BRAP_OP_P_Open);
            return ret;
        }
    
        /* Check if this output port type is supported */
        ret = BRAP_RM_P_IsOutputPortSupported(eOutput);
    	if( ret != BERR_SUCCESS)
    	{
    		BDBG_ERR(("BRAP_OP_P_Open: eOutputPort(%d) is not supported",
    			eOutput));
    		return BERR_TRACE(ret);
    	}        

        /* 2. Allocate memory for the Output port handle. Fill in parameters */
    
        /* Allocate handle */
        hOp = (BRAP_OP_P_Handle) BKNI_Malloc (sizeof(BRAP_OP_P_Object));
        if (hOp == NULL)
        {
            return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
        }
        
        /* Clear the handle memory */
        BKNI_Memset(hOp, 0, sizeof(BRAP_OP_P_Object));

        /* Initialise known elements in the handle */
        hOp->hChip = hFmm->hChip;
        hOp->hRegister = hFmm->hRegister;
        hOp->hHeap = hFmm->hHeap;
        hOp->hInt = hFmm->hInt;
        hOp->hFmm = hFmm;
        hOp->bMute = false;  /* Start with the output port unMuted ie active */
        hOp->eOutputPort = eOutput;
        hOp->ui32Offset = 0;
        hOp->uiOpenCnt = 1;
        hOp->uiStartCnt = 0;
	 hOp->eSamplingRate = eOutputSR;
	 hOp->bRfmodMuteState = false;
	 hOp->eRfmodMuxSelect = BRAP_OutputPort_eMax;
#ifndef BCHP_7411_VER
	 if ( eOutput == BRAP_OutputPort_eMai )
		sTemp1 = *(BRAP_OP_P_MaiSettings*)pSettings;
#endif     
    }   /* End: If not in WatchDog recovery */
    else
    {
        hOp = *phOp;
#ifndef BCHP_7411_VER        
    if (eOutput == BRAP_OutputPort_eMai)
    {
		sTemp1.sExtSettings = 
    		hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings;
    }
#endif     
    }    

    /* Set WAIT_FOR_VALIDX to KeepRequesting for all. 
       For debug purposes, clear the appropriate IGNORE_FIRST_UNDERFLOWx bit in
       AUD_FMM_OP_CTRL_STREAM_CFG in order see underflows when data has never
       been valid. */
#if 0       
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
#endif
    switch (eOutput)
    {
        case BRAP_OutputPort_eSpdif:
            ret = BRAP_OP_P_SpdifOpen (hOp, (BRAP_OP_P_SpdifSettings *) pSettings);
	     ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW0));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID0));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW0, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID0, Keep_requesting));
  	     BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32RegVal);
            break;
        case BRAP_OutputPort_eI2s0:
            ret = BRAP_OP_P_I2sOpen (hOp, (BRAP_OP_P_I2sSettings *) pSettings);
	     ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
#if ( BCHP_CHIP != 7400 )		 
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW2));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID2));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW2, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID2, Keep_requesting));
#else
/* Only for 7400 */
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW3));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID3));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW3, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID3, Keep_requesting));
#endif
	     BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32RegVal);
            break;
        case BRAP_OutputPort_eI2s1:
            hOp->ui32Offset = 4; /*TODO: verify offsets */
            ret = BRAP_OP_P_I2sOpen (hOp, (BRAP_OP_P_I2sSettings *) pSettings);
#if ( BCHP_CHIP != 7400 )		 
	     ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW3));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID3));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW3, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID3, Keep_requesting));
#else
	     ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW7));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID7));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW7, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID7, Keep_requesting));
#endif
  	     BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32RegVal);
            break;
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
        case BRAP_OutputPort_eI2s2:
            hOp->ui32Offset = 8; /*TODO: verify offsets */
            ret = BRAP_OP_P_I2sOpen (hOp, (BRAP_OP_P_I2sSettings *) pSettings);
	     ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW4));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID4));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW4, Ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID4, Keep_requesting));
  	     BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32RegVal);			
            break;
#endif	/* defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 ) */

#ifndef BCHP_7411_VER
	case BRAP_OutputPort_eDac0:
            /* TODO:Find out the DAC register offset */ 
            /* TBD later
            ret = BRAP_OP_P_DacOpen (hOp, (BRAP_OP_P_DacSettings *) pSettings);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW1));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID1));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW1, Dont_ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID1, Keep_requesting));*/
            hOp->ui32Offset = 0;
            ret = BRAP_OP_P_DacOpen (hOp, (BRAP_OP_P_DacSettings *) pSettings);
            break;

        case BRAP_OutputPort_eMai:

        /* This code is added to remove the compilation warning. PR41955 */            
        if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
        {
            sTemp1 = *(BRAP_OP_P_MaiSettings*)pSettings;
        }
        else
        {
        	sTemp1.sExtSettings = 
            hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings;
        }
            
            /* Start the MAI input ports first */
            switch (sTemp1.sExtSettings.eMaiMuxSelector )
            {
                case BRAP_OutputPort_eSpdif:
		      sSpdifpsettings.sExtSettings = 
			  	hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings;
		      hSpdifOp = hOp->hFmm->hOp[BRAP_OutputPort_eSpdif];
                    if ( hOp->hFmm->hOp[BRAP_OutputPort_eSpdif] == NULL )
                    {
      			   /* Allocate handle */
      			   hSpdifOp = (BRAP_OP_P_Handle) BKNI_Malloc (sizeof(BRAP_OP_P_Object));
      			   if (hSpdifOp == NULL)
      			   {
      			   		BKNI_Free(hOp);
      			      return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
      			   }
      
      			   /* Clear the handle memory */
      			   BKNI_Memset(hSpdifOp, 0, sizeof(BRAP_OP_P_Object));
      
      			   /* Initialise known elements in the handle */
      			   hSpdifOp->hChip = hFmm->hChip;
      			   hSpdifOp->hRegister = hFmm->hRegister;
      			   hSpdifOp->hHeap = hFmm->hHeap;
      			   hSpdifOp->hInt = hFmm->hInt;
      			   hSpdifOp->hFmm = hFmm;
      			   hSpdifOp->bMute = false;  /* Start with the output port unMuted ie active */
      			   hSpdifOp->eOutputPort = BRAP_OutputPort_eSpdif;
      			   hSpdifOp->ui32Offset = 0;
      			   hSpdifOp->uiOpenCnt = 1;
      			   hSpdifOp->uiStartCnt = 0;
                        hSpdifOp->eSamplingRate = eOutputSR;
                    }

		      ret = BRAP_OP_P_SpdifOpen ( hSpdifOp,
									&sSpdifpsettings );
			/* Store the handle to FMM handle */
		      hFmm->hOp[BRAP_OutputPort_eSpdif]=hSpdifOp;
                    break;
                    
                case BRAP_OutputPort_eFlex:
		      /* TODO: Open Flex here */
		      sFlexpsettings.sExtSettings = 
		      		hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eFlex].uOutputPortSettings.sFlexSettings;
		      hFlexOp = hOp->hFmm->hOp[BRAP_OutputPort_eFlex];
		      if ( hOp->hFmm->hOp[BRAP_OutputPort_eFlex] == NULL )
                    {
      			   /* Allocate handle */
      			   hFlexOp = (BRAP_OP_P_Handle) BKNI_Malloc (sizeof(BRAP_OP_P_Object));
      			   if (hFlexOp == NULL)
      			   {
      			   		BKNI_Free(hOp);
      			      return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
      			   }
      
      			   /* Clear the handle memory */
      			   BKNI_Memset(hFlexOp, 0, sizeof(BRAP_OP_P_Object));
      
      			   /* Initialise known elements in the handle */
      			   hFlexOp->hChip = hFmm->hChip;
      			   hFlexOp->hRegister = hFmm->hRegister;
      			   hFlexOp->hHeap = hFmm->hHeap;
      			   hFlexOp->hInt = hFmm->hInt;
      			   hFlexOp->hFmm = hFmm;
      			   hFlexOp->bMute = false;  /* Start with the output port unMuted ie active */
      			   hFlexOp->eOutputPort = BRAP_OutputPort_eFlex;
      			   hFlexOp->ui32Offset = 0;
      			   hFlexOp->uiOpenCnt = 1;
      			   hFlexOp->uiStartCnt = 0;
                        hFlexOp->eSamplingRate = eOutputSR;
                    }

		      ret = BRAP_OP_P_FlexOpen ( hFlexOp,
									&sFlexpsettings );
			/* Store the handle to FMM handle */
		      hFmm->hOp[BRAP_OutputPort_eFlex]=hFlexOp;
                    break;
                default:
                    BDBG_ERR (("BRAP_OP_P_Open(): Incorrect MAI input port type %d", 
                                sTemp1.sExtSettings.eMaiMuxSelector));
      			   		BKNI_Free(hOp);					
                    return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
            ret = BRAP_OP_P_MaiOpen (hOp, &sTemp1);
            break;
			
#endif	/*#ifndef BCHP_7411_VER */

#if ( BCHP_CHIP == 7400 )
	case BRAP_OutputPort_eDac1:
            /* TODO:Find out the DAC register offset */ 
            /* TBD later
            ret = BRAP_OP_P_DacOpen (hOp, (BRAP_OP_P_DacSettings *) pSettings);
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, IGNORE_FIRST_UNDERFLOW1));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, WAIT_FOR_VALID1));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    IGNORE_FIRST_UNDERFLOW1, Dont_ignore));
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                    WAIT_FOR_VALID1, Keep_requesting));*/
            hOp->ui32Offset = (BCHP_HIFIDAC_CTRL1_REVISION 
                                    - BCHP_HIFIDAC_CTRL0_REVISION);
            ret = BRAP_OP_P_DacOpen (hOp, (BRAP_OP_P_DacSettings *) pSettings);
            break;
#endif /* defined ( BCHP_CHIP == 7400 ) */

#if 0 /* TBD later. the followin ouput port types are not supported on current chips */            
        case BRAP_OutputPort_eFlex:
            /* TBD later
            ret = BRAP_OP_P_FlexOpen (hOp, (BRAP_OP_P_FlexSettings *) pSettings);
            break;
            */
        case BRAP_OutputPort_eRfMod:
            /* TBD later
            ret = BRAP_OP_P_RfModOpen (hOp, (BRAP_OP_P_DacSettings *) pSettings);
            break;
            */      
#endif

        default:
            BDBG_ERR(("BRAP_OP_P_Open(): Incorrect Output port type %d",
                        eOutput));
            /* Free the Handle memory*/
            BKNI_Free (hOp);   
            return BERR_TRACE (BRAP_ERR_OUPUT_PORT_NOT_SUPPORTED);
    }
	
#if 0	
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32RegVal);
#endif

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    {   /* If not in WatchDog recovery */  
        /* Store the Ouput object handle inside the FMM handle */
        hFmm->hOp[eOutput] = hOp;

        *phOp = hOp;
    } 
    
    BDBG_LEAVE (BRAP_OP_P_Open);
    return ret;

}

BERR_Code
BRAP_OP_P_Close (
    BRAP_OP_P_Handle     hOp        /*[in] Output port handle */
)                
{
    BERR_Code ret = BERR_SUCCESS;
#ifndef BCHP_7411_VER   
    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;
#endif
    BDBG_ENTER (BRAP_OP_P_Close);
    BDBG_ASSERT (hOp);
       
    if (hOp->uiOpenCnt == 0)
    { 
        /* This should never happen. If it does, it means the system
           has gone into an invalid state!!!!*/
            BDBG_ERR (("BRAP_OP_P_Close: Output port %d has NO open"
                        "audio channels!!!", hOp->eOutputPort));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    BDBG_MSG (("BRAP_OP_P_Close: Closing Output port %d ", hOp->eOutputPort));    
    hOp->uiOpenCnt --;
#ifndef BCHP_7411_VER    
    /* if it is Mai, then the Flex/Spdif count has to be decremented also */
    if (hOp->eOutputPort == BRAP_OutputPort_eMai)
    {

        eMaiMuxSelect = 
            hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
        if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
        	hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uiOpenCnt --;
        else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
        	hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uiOpenCnt --;			
    }   
#endif

    if (hOp->uiOpenCnt == 0)
    {   
        /* If this was the last open audio channel on this output port,
           free the handle etc */
#ifndef BCHP_7411_VER
	 if ( hOp->eOutputPort == BRAP_OutputPort_eMai )
	 {
	 	switch (eMaiMuxSelect)
	 	{
	 		case BRAP_OutputPort_eSpdif:
				BKNI_Free (hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]); 
				hOp->hFmm->hOp[BRAP_OutputPort_eSpdif] = NULL;
				break;
			case BRAP_OutputPort_eFlex:
				BKNI_Free (hOp->hFmm->hOp[BRAP_OutputPort_eFlex]); 
				hOp->hFmm->hOp[BRAP_OutputPort_eFlex] = NULL;
				break;
			default:
                     	BDBG_ERR (("BRAP_OP_P_Close(): Incorrect MAI input port type"));
                    		return BERR_TRACE (BERR_NOT_SUPPORTED);
	 	}
	 }
#endif    

        /* Remove referrence to this Output port from the parent FMM */ 
        hOp->hFmm->hOp[hOp->eOutputPort] = NULL;

        /* Free the Output port Handle memory*/
        BKNI_Free (hOp); 
    }             

    BDBG_LEAVE (BRAP_OP_P_Close);
    return ret;
}


BERR_Code
BRAP_OP_P_Start (
    BRAP_OP_P_Handle    hOp,        /*[in] Output port handle */
    const void *        pParams     /*[in] Output Port start time 
                                    parameters. Typecast it to proper
                                    setting structure based on the 
                                    port type */ 
)
{
    BERR_Code ret = BERR_SUCCESS;
#ifndef BCHP_7411_VER
    BRAP_OP_P_SpdifParams sSpdifParams;
    BRAP_OutputPort eMaiMuxSelect;    
    BRAP_OP_P_MaiParams * pMaiParams=NULL;
#endif	
    BDBG_ENTER (BRAP_OP_P_Start);
    BDBG_ASSERT (hOp);

    /* Make sure new port is configured */ 
    if(hOp->hFmm->hRap->bOpSettingsValid[hOp->eOutputPort] == false) 
    {
        BDBG_ERR(("BRAP_OP_P_Start: Output port %d is not configured."
            "Please configure before calling this PI.", hOp->eOutputPort));
        return BERR_TRACE(BRAP_ERR_OUTPUT_NOT_CONFIGURED);
    } 
    

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {   /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);

        if (hOp->uiStartCnt > 0)
        {
            /* This Outport was previoulsy started by another audio channel.
            So this time, just increment the start count. Dont change the 
            settings etc. */
            /* start Count cannot exceed the open count!! */
            if (hOp->uiStartCnt >= hOp->uiOpenCnt)
            {
                BDBG_ERR (("BRAP_OP_P_Start: Cannot start Output port %d. "
                        "Start Count (%d) >= Open Count (%d)",
                        hOp->eOutputPort, hOp->uiStartCnt, hOp->uiOpenCnt));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
        
            hOp->uiStartCnt ++;
#ifndef BCHP_7411_VER
            /* if it is Mai, then the Flex/Spdif count has to be incremented also */
	     if (hOp->eOutputPort == BRAP_OutputPort_eMai)
	    {
            eMaiMuxSelect = 
                hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;             
		    if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
				hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uiStartCnt ++;
		    else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
				hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uiStartCnt ++;			
	     }            
#endif
            BDBG_MSG (("BRAP_OP_P_Start: Output port %d was already started." 
                    "New start count = %d", 
                    hOp->eOutputPort, hOp->uiStartCnt));
            BDBG_LEAVE (BRAP_OP_P_Start);
            return ret;
        }
    }
    
    /* Start the o/p port operation */
    switch (hOp->eOutputPort)
    {
         case BRAP_OutputPort_eSpdif:
            ret = BRAP_OP_P_SpdifStart (hOp, (BRAP_OP_P_SpdifParams *) pParams);
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            ret = BRAP_OP_P_I2sStart (hOp, (BRAP_OP_P_I2sParams *) pParams);
            break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
        case BRAP_OutputPort_eDac0:
	 case BRAP_OutputPort_eDac1:
            ret = BRAP_OP_P_DacStart (hOp, (BRAP_OP_P_DacParams *) pParams);
	     break;
        case BRAP_OutputPort_eMai:  
            /* Start the MAI input ports first */
            eMaiMuxSelect = 
                hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
            switch (eMaiMuxSelect)
            {
                case BRAP_OutputPort_eSpdif:
    		      /*TODO: modify later if more paramters are added to sSpdifParams */					
                    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
                    {   /* If not in WatchDog recovery */              
                        pMaiParams = (BRAP_OP_P_MaiParams *) pParams;
                        sSpdifParams.eTimebase = pMaiParams->eTimebase;             
                    }                 
      
                    ret = BRAP_OP_P_SpdifStart (hOp->hFmm->hOp[BRAP_OutputPort_eSpdif], &sSpdifParams);
                    if (ret != BERR_SUCCESS)
                    {
                        BDBG_ERR(("Call to BRAP_OP_P_SpdifStart failed while trying to start Mai."));  
                        return BERR_TRACE (ret);
                    }
                    break;
                case BRAP_OutputPort_eFlex:
		      /* TODO */
                    break;
                default:
                    BDBG_ERR (("BRAP_OP_P_Start(): Incorrect MAI input port type %d", 
                                eMaiMuxSelect));
                    return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
            ret = BRAP_OP_P_MaiStart (hOp, (BRAP_OP_P_MaiParams *)pParams);
            break;                                                              

#endif
        default:
            BDBG_ERR (("BRAP_OP_P_Start(): Output port type %d not supported",
                        hOp->eOutputPort));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    if ((BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
          && (ret == BERR_SUCCESS))
    {   /* If not in WatchDog recovery */  
        hOp->uiStartCnt ++;
		
#ifndef BCHP_7411_VER
	/* if it is Mai, then the Flex/Spdif count has to be incremented also */
	if (hOp->eOutputPort == BRAP_OutputPort_eMai)
	{
		eMaiMuxSelect = 
			hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;             
		if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
			hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uiStartCnt ++;
		else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
			hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uiStartCnt ++;			
	}            
#endif

    }
    BDBG_LEAVE (BRAP_OP_P_Start);
    return ret;
}

BERR_Code
BRAP_OP_P_Stop (
    BRAP_OP_P_Handle    hOp        /*[in] Output port handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
#ifndef BCHP_7411_VER
    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;  
    BRAP_OP_P_Handle hTemp=NULL;
#endif
    
    BDBG_ENTER (BRAP_OP_P_Stop);
    BDBG_ASSERT (hOp);
   
    if (hOp->uiStartCnt == 0)
    { 
            BDBG_WRN (("BRAP_OP_P_Stop: Output port %d has NO active"
                        "audio channels. Ignoring this call to _Stop()!",
                        hOp->eOutputPort));
            BDBG_LEAVE (BRAP_OP_P_Stop);
            return ret;
    }
    hOp->uiStartCnt --;
#ifndef BCHP_7411_VER
    /* if it is Mai, then the Flex/Spdif count has to be decremented also */
    if (hOp->eOutputPort == BRAP_OutputPort_eMai)
    {

        eMaiMuxSelect = 
            hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;    
        if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
        	hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uiStartCnt --;
        else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
        	hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uiStartCnt --;			
    }       
#endif

    BDBG_MSG (("BRAP_OP_P_Stop: Output port %d new start count =%d",
                hOp->eOutputPort, hOp->uiStartCnt));
    if (hOp->uiStartCnt != 0)
    {
            BDBG_LEAVE (BRAP_OP_P_Stop);
            return ret;
    }
    
    /* Stop the o/p port operation */
    switch (hOp->eOutputPort)
    {
         case BRAP_OutputPort_eSpdif:
            ret = BRAP_OP_P_SpdifStop (hOp);
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            ret = BRAP_OP_P_I2sStop (hOp);
            break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */      
        case BRAP_OutputPort_eDac0:
	 case BRAP_OutputPort_eDac1:
            ret = BRAP_OP_P_DacStop (hOp);			
	     break;
        case BRAP_OutputPort_eMai:
            /* Start the MAI input ports first */
            switch (eMaiMuxSelect)
            {
                /* TODO: Verify */
                case BRAP_OutputPort_eSpdif:
                    hTemp = hOp->hFmm->hOp[BRAP_OutputPort_eSpdif];
                    ret = BRAP_OP_P_SpdifStop (hTemp);
                    break;
                    
                case BRAP_OutputPort_eFlex:
		      /* TODO */
                    break;
                    
                default:
                    BDBG_ERR (("BRAP_OP_P_Stop(): Incorrect MAI input port type %d", 
                                eMaiMuxSelect));
                    return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
            ret = BRAP_OP_P_MaiStop (hOp);
            break;
#endif
#if 0 /* TBD later */   
        case BRAP_OutputPort_eDac1:
            ret = BRAP_OP_P_HiFiDacStop (hOp);
            break;
        case BRAP_OutputPort_eFlex:
        {
            pTemp1 = (BRAP_OP_P_FlexParams *) (hOp->uOpParams.sFlex); 
            
            /* Start the Flex input ports first */
            switch (pTemp1->eFlexInput)
            {
                /* TODO: Check  */
                case BRAP_OutputPort_eSpdif:
                    hTemp = hOp->hFmm->hOp[BRAP_OutputPort_eSpdif];
                    ret = BRAP_OP_P_SpdifStop (hTemp);
                    break;
                    
                case BRAP_OutputPort_eDac0:
                    hTemp = hOp->hFmm->hOp[BRAP_OutputPort_eDac0];
                    ret = BRAP_OP_P_HiFiDacStop (hTemp);
                    break;
                    
                case BRAP_OutputPort_eDac1:
                    hTemp = hOp->hFmm->hOp[BRAP_OutputPort_eDac1];
                    ret = BRAP_OP_P_HiFiDacStop (hTemp);
                    break;

                default:
                    BDBG_ERR (("BRAP_OP_P_Stop(): Incorrect Flex input port type %d",
                                pTemp1->eFlexInput));
                    return BERR_TRACE (BERR_NOT_SUPPORTED);
            }
            ret = BRAP_OP_P_FlexStop (hOp);
            break;
#endif
        default:
            BDBG_ERR (("BRAP_OP_P_Stop(): Incorrect Output port type %d", hOp->eOutputPort));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }    
    BDBG_LEAVE (BRAP_OP_P_Stop);
    return ret;
}


static BERR_Code
BRAP_OP_P_SpdifOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_SpdifSettings * pSettings /*[in] Start time parameters */
)
{
    uint32_t ui32RegVal = 0;
    BDBG_ENTER (BRAP_OP_P_SpdifOpen);
    BDBG_ASSERT (hOp);
    BSTD_UNUSED(pSettings);

    /* Enable the clock while opening the output port. Never disable it */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset); 
    
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,CLOCK_ENABLE));  
    ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_SPDIF_CFG_0,  
                                    CLOCK_ENABLE, Enable));
    BRAP_Write32 (hOp->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset, 
               ui32RegVal);     
    
    BDBG_LEAVE (BRAP_OP_P_SpdifOpen);
    return BERR_SUCCESS;
}


BERR_Code BRAP_OP_P_SpdifHWConfig (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_SpdifSettings * pSettings /*[in] Parameters */
)
{
    uint32_t  ui32RegVal = 0;
    BRAP_OP_P_SpdifSettings sSettings;

    BDBG_ENTER (BRAP_OP_P_SpdifHWConfig);
    BDBG_ASSERT (hOp);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {   /* If not in WatchDog recovery */  
        BDBG_ASSERT (pSettings);
        sSettings = *pSettings;
    }
    else
    {
        sSettings = hOp->uOpSettings.sSpdif;
    }

    BDBG_MSG(("BRAP_OP_P_SpdifHWConfig(): "));
    BDBG_MSG(("sSettings.sExtSettings.eChannelOrder=%d", sSettings.sExtSettings.eChannelOrder));
    BDBG_MSG(("sSettings.sExtSettings.bLimitTo16Bits=%d", sSettings.sExtSettings.bLimitTo16Bits));
    BDBG_MSG(("sSettings.sExtSettings.eMClkRate=%d", sSettings.sExtSettings.eMClkRate));
    BDBG_MSG(("sSettings.sExtSettings.ePll=%d", sSettings.sExtSettings.ePll));

    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,LR_SELECT));    
    /* Set Channel Order */
    switch (sSettings.sExtSettings.eChannelOrder)
    {
        case BRAP_OP_ChannelOrder_eNormal:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LR_SELECT, Normal));
            break;
        case BRAP_OP_ChannelOrder_eBothLeft:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LR_SELECT, Both_Get_Left));
            break;
        case BRAP_OP_ChannelOrder_eBothRight:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LR_SELECT, Both_Get_Right));
            break;
        case BRAP_OP_ChannelOrder_eSwap:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LR_SELECT, Swap));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_SpdifHWConfig(): Incorrect Channel Order %d", 
                        sSettings.sExtSettings.eChannelOrder));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }            
       
    /* Set LIMIT_TO_16_BITS field */
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,LIMIT_TO_16_BITS));        
    if (sSettings.sExtSettings.bLimitTo16Bits == true)
    {
        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LIMIT_TO_16_BITS, Enable_limit));
    } 
    else if (sSettings.sExtSettings.bLimitTo16Bits == false)
    {
        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            LIMIT_TO_16_BITS, Disable_limit));
    } 
   
#ifdef BCHP_7411_VER /* Only for the 7411 */      
    /* Set MCLK rate */ 
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,MCLK_RATE));      
    switch (sSettings.sExtSettings.eMClkRate)
    {
        case BRAP_OP_MClkRate_e128Fs:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            MCLK_RATE, MCLK_is_128fs));
            break;     
        case BRAP_OP_MClkRate_e256Fs:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            MCLK_RATE, MCLK_is_256fs));
            break;     
        case BRAP_OP_MClkRate_e384Fs:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            MCLK_RATE, MCLK_is_384fs));
            break;     
        case BRAP_OP_MClkRate_e512Fs:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                            MCLK_RATE, MCLK_is_512fs));
            break;     
        default:
            BDBG_ERR (("BRAP_OP_P_SpdifHWConfig(): Incorrect MClk Rate %d", 
                        sSettings.sExtSettings.eMClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    } 

    /* Set Preamble Polarity to 'High' */ 
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,PREAM_POL));       
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    PREAM_POL, Exact));
#else
    /* Set Preamble Polarity to 'High' */ 
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,PREAM_POL)); 
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    PREAM_POL, High));
#endif     

    /* Set SOFT_PARITY to 'Hard' */ 
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_SPDIF_CFG_0,SOFT_PARITY)); 
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    SOFT_PARITY, Hard));

    /* Write to the register */
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset, ui32RegVal);
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {  
        /* store the settings in hOp */
        hOp->uOpSettings.sSpdif = sSettings;
    }
    return BERR_SUCCESS;
}



static BERR_Code
BRAP_OP_P_SpdifStart (
    BRAP_OP_P_Handle        hOp,             /*[in] Output port handle */
    const BRAP_OP_P_SpdifParams * pParams    /*[in] Start time parameters */
)
{

/*     Algo:
    1. Cofigure AUD_FMM_OP_CTRL_MCLK_CFG_SPDIFX
    2. Cofigure Timebase ie AUD_FMM_PLLX_CONTROL
    3. Clear the CBIT array
    4. Configure the Spdif port ie program register AUD_FMM_OP_CTRL_SPDIF_CFG_X
        x with the settings saved in hRap ie
        hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings
        by calling BRAP_OP_P_SpdifHWConfig()
    5. If the port was unmuted, enable data flow.
    6. Enable upstream request.
*/    

    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;
    BRAP_OP_P_SpdifSettings sSettings;    

    BDBG_ENTER (BRAP_OP_P_SpdifStart);
    BDBG_ASSERT (hOp);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {    
        BDBG_ASSERT (pParams);
        hOp->uOpParams.sSpdif = *pParams;
    }
  
    BDBG_MSG(("BRAP_OP_P_SpdifStart: Start time parameters are: "));

    BDBG_MSG(("hOp->uOpParams.sSpdif->eTimebase=%d",hOp->uOpParams.sSpdif.eTimebase));

    sSettings.sExtSettings = 
    hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings;


    /* Program the MClk */
    ui32RegVal = 0;
#ifdef BCHP_7411_VER /* Only for the 7411 */ 
    /* Set the PLL */
    switch (sSettings.sExtSettings.ePll)
    {
        case BRAP_OP_Pll_ePll0:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL0));
            break;    
        case BRAP_OP_Pll_ePll1:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL1));
            break;    
        case BRAP_OP_Pll_ePll2:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL2));
            break;    
        case BRAP_OP_Pll_ePll0Div2:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL0_DIV2));
            break;    
        case BRAP_OP_Pll_ePll1Div2:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL1_DIV2));
            break;    
        case BRAP_OP_Pll_ePll2Div2:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL2_DIV2));
            break;    
        default:
            BDBG_ERR (("BRAP_OP_P_SpdifStart(): Incorrect PLL chosen %d", 
                        sSettings.sExtSettings.ePll));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    } 
#else
    /* For 7401 */
    /* Set the PLL and MClock Rate */
    /* The PLL Channels are choosen in the following way.
        
       If MClock rate is 128 Fs, Channel 1 should be selected,
       If MClock rate is 256 Fs, Channel 2 should be selected,
       If MClock rate is 512 Fs, Channel 3 should be selected,
 
       Basically Channels 1,2 and 3 correspond to 128Fs, 256Fs and 512Fs respectively.
 
       (MClock 384Fs can be set by user programming only).
    */
    switch (sSettings.sExtSettings.ePll)
    {
        case BRAP_OP_Pll_ePll0:
            
        /* Set MCLK rate and PLL Channels */
        switch (sSettings.sExtSettings.eMClkRate)
        {
            case BRAP_OP_MClkRate_e128Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL0_ch1)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_128fs)));
                break;     
            case BRAP_OP_MClkRate_e256Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL0_ch2)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_256fs)));
                break;     
            case BRAP_OP_MClkRate_e512Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL0_ch3)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_512fs)));
                break;     
            default:
                BDBG_ERR (("BRAP_OP_P_SpdifStart(): Incorrect MClk Rate %d", 
                            sSettings.sExtSettings.eMClkRate));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        }     
            
        break;

        case BRAP_OP_Pll_ePll1:
        
        /* Set MCLK rate and PLL Channels */
        switch (sSettings.sExtSettings.eMClkRate)
        {
            case BRAP_OP_MClkRate_e128Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL1_ch1)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_128fs)));
                break;     
            case BRAP_OP_MClkRate_e256Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL1_ch2)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_256fs)));
                break;     
            case BRAP_OP_MClkRate_e512Fs:
                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            PLLCLKSEL, PLL1_ch3)) |
                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
                                            MCLK_RATE, MCLK_is_512fs)));
                break;     
            default:
                BDBG_ERR (("BRAP_OP_P_SpdifStart(): Incorrect MClk Rate %d", 
                            sSettings.sExtSettings.eMClkRate));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        } 

        break;
		
#if ( BCHP_CHIP == 7400 )
	case BRAP_OP_Pll_ePll2:
	        
	        /* Set MCLK rate and PLL Channels */
	        switch (sSettings.sExtSettings.eMClkRate)
	        {
	            case BRAP_OP_MClkRate_e128Fs:
	                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            PLLCLKSEL, PLL2_ch1)) |
	                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            MCLK_RATE, MCLK_is_128fs)));
	                break;     
	            case BRAP_OP_MClkRate_e256Fs:
	                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            PLLCLKSEL, PLL2_ch2)) |
	                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            MCLK_RATE, MCLK_is_256fs)));
	                break;     
	            case BRAP_OP_MClkRate_e512Fs:
	                ui32RegVal |= ((BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            PLLCLKSEL, PLL2_ch3)) |
	                               (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, 
	                                            MCLK_RATE, MCLK_is_512fs)));
	                break;     
	            default:
	                BDBG_ERR (("BRAP_OP_P_SpdifStart(): Incorrect MClk Rate %d", 
	                            sSettings.sExtSettings.eMClkRate));
	                return BERR_TRACE (BERR_NOT_SUPPORTED);
	        } 

	        break;
#endif /* defined ( BCHP_CHIP == 7400 ) */

        default:
            BDBG_ERR (("BRAP_OP_P_SpdifStart(): Incorrect PLL chosen %d", 
                        sSettings.sExtSettings.ePll));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
#endif    
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_SPDIF0, ui32RegVal);

    /* Program the Timebase */
    ret = BRAP_OP_P_ProgramTimebase (hOp, sSettings.sExtSettings.ePll,
                                     hOp->uOpParams.sSpdif.eTimebase);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_SpdifStart(): call to BRAP_OP_P_ProgramTimebase() failed"));
        return BERR_TRACE (ret);
    }

#if 0  
    /* PR16721: as requested by FW, we now clear the CBIT buffer in BRAP_OP_P_SpdifStart(). */
    for (i=BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_START; i<=BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_END; i=i+4)
    {
        BRAP_Write32 (hOp->hRegister, 
                       ( BCHP_AUD_FMM_MS_CTRL_FW_CBITSi_ARRAY_BASE + i), 0); 
    }
#endif	
    
    /* Configure the on-the-fly paramters for Spdif port */
    BRAP_OP_P_SpdifHWConfig (hOp, &sSettings);

    /* Enable the Spdif port */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK (
                AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                DATA_ENABLE));    
        ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    DATA_ENABLE, Enable));
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset, ui32RegVal);
  
    /* Enable SPDIF upstream request */
    ret = BRAP_OP_P_EnableStream (hOp, true);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_SpdifStart(): call to BRAP_OP_P_EnableStream() failed"));
        return BERR_TRACE (ret);
    }


    BDBG_LEAVE (BRAP_OP_P_SpdifStart);
    return ret;
}

    
static BERR_Code
BRAP_OP_P_SpdifStop (
    BRAP_OP_P_Handle        hOp    /*[in] Output port handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
	
#if 0	
    uint32_t  ui32RegVal = 0;
#endif

    BDBG_ENTER (BRAP_OP_P_SpdifStop);
    BDBG_ASSERT (hOp);

    BDBG_MSG(("BRAP_OP_P_SpdifStop: entering"));

#if 0
    /* Read current value of the SPDIF config register */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0);

    /* Disable DATA_ENABLE. Do not disable the Clock */
	ui32RegVal &= ~(BCHP_MASK (
                AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                DATA_ENABLE));        
    ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_SPDIF_CFG_0, 
                                    DATA_ENABLE, Disable));
    
    /* Write to the register */
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset, ui32RegVal);
   
    /* Disable SPDIF upstream request */
    ret = BRAP_OP_P_EnableStream (hOp, false);
#else
	BSTD_UNUSED(hOp);
#endif

    BDBG_LEAVE (BRAP_OP_P_SpdifStop);
    return ret;
}


static BERR_Code
BRAP_OP_P_I2sOpen (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_I2sSettings * pSettings  /*[in] Open time parameters */
)
{
    uint32_t ui32RegVal = 0;
    BDBG_ENTER (BRAP_OP_P_I2sOpen);
    BDBG_ASSERT (hOp);
    BSTD_UNUSED(pSettings);

    /* Enable the clock while opening the output port. Never disable it */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset);

	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,CLOCK_ENABLE));  
    ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG,  
                                    CLOCK_ENABLE, Enable));

    BRAP_Write32 (hOp->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset, 
               ui32RegVal);     
    

    BDBG_LEAVE (BRAP_OP_P_I2sOpen);
    return BERR_SUCCESS;
}

BERR_Code
BRAP_OP_P_I2sHWConfig (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_I2sSettings * pSettings  /*[in] Open time parameters */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;
    unsigned int uiTemp;
    BRAP_OP_P_I2sSettings sSettings;

    BDBG_ENTER (BRAP_OP_P_I2sHWConfig);
    BDBG_ASSERT (hOp);

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {   /* If not in WatchDog recovery */  
        BDBG_ASSERT (pSettings);
        sSettings = *pSettings;
    }
    else
    {
        sSettings = hOp->uOpSettings.sI2s;
    }
   
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset);
	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,LR_SELECT)); 
    /* Set Channel Order */
    switch (sSettings.sExtSettings.eChannelOrder)
    {
        case BRAP_OP_ChannelOrder_eNormal:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                            LR_SELECT, Normal));
            break;
        case BRAP_OP_ChannelOrder_eBothLeft:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                            LR_SELECT, Both_Get_Left));
            break;
        case BRAP_OP_ChannelOrder_eBothRight:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                            LR_SELECT, Both_Get_Right));
            break;
        case BRAP_OP_ChannelOrder_eSwap:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                            LR_SELECT, Swap));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_I2sHWConfig(): Incorrect Channel Order %d", 
                        sSettings.sExtSettings.eChannelOrder));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }          
    
	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,DATA_JUSTIFICATION));     
    if (sSettings.sExtSettings.bLsbAtLRClk == true)
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        DATA_JUSTIFICATION, LSB));
    }
    else 
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        DATA_JUSTIFICATION, MSB));
    }

	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,DATA_ALIGNMENT));  
    if (sSettings.sExtSettings.bAlignedToLRClk == false)
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        DATA_ALIGNMENT, Delayed));
    }
    else 
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        DATA_ALIGNMENT, Aligned));
    }
        
	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,SCLK_POLARITY));  
    ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                    SCLK_POLARITY, Falling_aligned_with_sdata));

	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,LRCK_POLARITY)); 
    if (sSettings.sExtSettings.bLRClkPolarity == true)
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        LRCK_POLARITY, High_for_left));
    }
    else 
    {
        ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_I2S_CFG, 
                                        LRCK_POLARITY, Low_for_left));
    }       
 
    /* Set SCLKS_PER_1FS_DIV32 field. This field times 32 is the number 
       of sclk cycles in one Fs period. For example, program this register
       to 2 for a 64fs bit clock. */
    switch (sSettings.sExtSettings.eSClkRate)
    {
        case BRAP_OP_SClkRate_e64Fs: 
            uiTemp = 64;
            break;
        case BRAP_OP_SClkRate_e128Fs: 
            uiTemp = 128;
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_I2sHWConfig(): Incorrect SClk %d", 
                        sSettings.sExtSettings.eSClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,SCLKS_PER_1FS_DIV32));     
    ui32RegVal |= (BRAP_FIELD_DATA(AUD_FMM_OP_CTRL_I2S_CFG, 
                                    SCLKS_PER_1FS_DIV32, 
                                    (uiTemp/32)));
#ifdef BCHP_7411_VER /* for 7411 */
    /* Set MCLK/(2*SCLK) ratio in MCLK_RATE field */
    switch (sSettings.sExtSettings.eMClkRate)
    {
        case BRAP_OP_MClkRate_e128Fs:
           uiTemp = (128 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e256Fs:
           uiTemp = (256 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e384Fs:
           uiTemp = (384 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e512Fs:
           uiTemp = (512 / (2 * uiTemp));
           break; 
        default:
            BDBG_ERR (("BRAP_OP_P_I2sHWConfig(): Incorrect MClk %d", 
                        sSettings.sExtSettings.eMClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
	ui32RegVal &= ~(BRAP_MASK(AUD_FMM_OP_CTRL_I2S_CFG,MCLK_RATE));      
    switch (uiTemp)
    {
        case 1:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_128fs_SCLK_64fs));
            break;     
        case 2:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_256fs_SCLK_64fs));
            break;     
        case 3:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_384fs_SCLK_64fs));
            break;     
        case 4:
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_512fs_SCLK_64fs));
            break;     
        default:
            BDBG_ERR (("BRAP_OP_P_I2sHWConfig(): Unsupported MClk/SClk Ratio %d", 
                        uiTemp));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    } 
#endif
    
    BRAP_Write32 (hOp->hRegister, 
               BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset, 
               ui32RegVal); 
    
    /* dont store the settings until everything's correct */
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    {  
        hOp->uOpSettings.sI2s = sSettings;
    }

    BDBG_LEAVE (BRAP_OP_P_I2sHWConfig);
    return ret; 
}

static BERR_Code
BRAP_OP_P_I2sStart (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_I2sParams *   pParams    /*[in] Start time parameters */
)
{

/*     Algo:
    1. Cofigure AUD_FMM_OP_CTRL_MCLK_CFG_I2S
    2. Cofigure Timebase ie AUD_FMM_PLLX_CONTROL 
    3. Program AUD_FMM_MISC_SEROUT_OE     
    4. Configure the i2s port ie program register AUD_FMM_OP_CTRL_SPDIF_CFG_X
        x with 
        i) on-the-fly changeable confgi saved in hRap ie
        hRap->sOutputSettings[XXX].uOutputPortSettings.sSpdifSettings
        by calling BRAP_OP_P_I2sHWConfig()
        ii) other parameters passed at start time.
    5. If the port was unmuted, enable data flow.
    6. Enable upstream request.
*/  

    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;
    BRAP_OP_P_I2sParams sParams;
    BRAP_OP_P_I2sSettings sSettings;
#ifndef BCHP_7411_VER       
    unsigned int uiTemp=0;
#endif
    BDBG_ENTER (BRAP_OP_P_I2sStart);
    BDBG_ASSERT (hOp);
    BDBG_MSG(("BRAP_OP_P_I2sStart: entering"));
    
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
    { 
        BDBG_ASSERT (pParams);
        hOp->uOpParams.sI2s = *pParams;
        sParams = *pParams;
    }
    else
    {
        sParams = hOp->uOpParams.sI2s; 
    }    

    sSettings.sExtSettings = hOp->hFmm->hRap->sOutputSettings[hOp->eOutputPort].uOutputPortSettings.sI2sSettings;


    ui32RegVal= 0; /* we can start with 0 since we're reporgramming all fields 
    of the register AUD_FMM_OP_CTRL_MCLK_CFG_I2S*/
#ifndef BCHP_7411_VER /* not for the 7411 */ 
    /* Set SCLKS_PER_1FS_DIV32 field. This field times 32 is the number 
       of sclk cycles in one Fs period. For example, program this register
       to 2 for a 64fs bit clock. */
    switch (sSettings.sExtSettings.eSClkRate)
    {
        case BRAP_OP_SClkRate_e64Fs: 
            uiTemp = 64;
            break;
        case BRAP_OP_SClkRate_e128Fs: 
            uiTemp = 128;
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect SClk %d", 
                        sSettings.sExtSettings.eSClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
    /* Set MCLK/(2*SCLK) ratio in MCLK_RATE field */
    switch (sSettings.sExtSettings.eMClkRate)
    {
        case BRAP_OP_MClkRate_e128Fs:
           uiTemp = (128 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e256Fs:
           uiTemp = (256 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e384Fs:
           uiTemp = (384 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e512Fs:
           uiTemp = (512 / (2 * uiTemp));
           break; 
        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect MClk %d", 
                        sSettings.sExtSettings.eMClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
    switch (uiTemp)
    {
        case 1:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            MCLK_RATE, MCLK_128fs_SCLK_64fs));
            break;     
        case 2:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            MCLK_RATE, MCLK_256fs_SCLK_64fs));
            break;     
        case 3:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            MCLK_RATE, MCLK_384fs_SCLK_64fs));
            break;     
        case 4:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            MCLK_RATE, MCLK_512fs_SCLK_64fs));
            break;     
        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Unsupported MClk/SClk Ratio %d", 
                        uiTemp));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    } 

  
    /* Set the PLL */
    /* The PLL Channels are choosen in the following way.
        
       If MClock rate is 128 Fs, Channel 1 should be selected,
       If MClock rate is 256 Fs, Channel 2 should be selected,
       If MClock rate is 512 Fs, Channel 3 should be selected,
 
       Basically Channels 1,2 and 3 correspond to 128Fs, 256Fs and 512Fs respectively.
 
       (MClock 384Fs can be set by user programming only).
    */
    switch (sSettings.sExtSettings.ePll)
    {
        case BRAP_OP_Pll_ePll0:
            
        /* Set MCLK rate and PLL Channels */
        switch (sSettings.sExtSettings.eMClkRate)
        {
            case BRAP_OP_MClkRate_e128Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL0_ch1));
                break;     
            case BRAP_OP_MClkRate_e256Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL0_ch2));
                break;     
            case BRAP_OP_MClkRate_e512Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL0_ch3));
                break;     
            default:
                BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect MClk Rate %d", 
                            sSettings.sExtSettings.eMClkRate));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        }     
            
        break;

        case BRAP_OP_Pll_ePll1:
        
        /* Set MCLK rate and PLL Channels */
        switch (sSettings.sExtSettings.eMClkRate)
        {
            case BRAP_OP_MClkRate_e128Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL1_ch1));
                break;     
            case BRAP_OP_MClkRate_e256Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL1_ch2));
                break;     
            case BRAP_OP_MClkRate_e512Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            PLLCLKSEL, PLL1_ch3));
                break;     
            default:
                BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect MClk Rate %d", 
                            sSettings.sExtSettings.eMClkRate));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        } 

        break;

#if ( BCHP_CHIP == 7400 )
        case BRAP_OP_Pll_ePll2:

        /* Set MCLK rate and PLL Channels */
        switch (sSettings.sExtSettings.eMClkRate)
    {
            case BRAP_OP_MClkRate_e128Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL2_ch1));
            break;
            case BRAP_OP_MClkRate_e256Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL2_ch2));
            break;
            case BRAP_OP_MClkRate_e512Fs:
                ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL2_ch3));
            break;
        default:
                BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect MClk Rate %d", 
                            sSettings.sExtSettings.eMClkRate));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }          
    
        break;
#endif

        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect PLL chosen %d", 
                        sSettings.sExtSettings.ePll));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
#else
    ui32RegVal = 0;
    /* Set the PLL */
    switch (sSettings.sExtSettings.ePll)
    {
        case BRAP_OP_Pll_ePll0:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL0));
            break;    
        case BRAP_OP_Pll_ePll1:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL1));
            break;    
        case BRAP_OP_Pll_ePll2:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            PLLCLKSEL, PLL2));
            break;    
        case BRAP_OP_Pll_ePll0Div2:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL0_DIV2));
            break;    
        case BRAP_OP_Pll_ePll1Div2:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            PLLCLKSEL, PLL1_DIV2));
            break;    
        case BRAP_OP_Pll_ePll2Div2:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S,
                                            PLLCLKSEL, PLL2_DIV2));
            break;    
        case BRAP_OP_Pll_eHifiDac0:
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            PLLCLKSEL, Hifidac0));
            break;
            ui32RegVal |= (BRAP_FIELD_ENUM(AUD_FMM_OP_CTRL_MCLK_CFG_I2S, 
                                            PLLCLKSEL, Hifidac1));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Incorrect PLL chosen %d", 
                        sSettings.sExtSettings.ePll));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
#endif
    BRAP_Write32 (hOp->hRegister, 
            BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_I2S + hOp->ui32Offset,
               ui32RegVal);     
    
    /* Program the Timebase */
    ret = BRAP_OP_P_ProgramTimebase (hOp, sSettings.sExtSettings.ePll,
                                     sParams.eTimebase);    

    /* Read-Modify-Write AUD_FMM_MISC_SEROUT_OE */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE);
    switch (hOp->eOutputPort)
    {  
        case BRAP_OutputPort_eI2s0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK0_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK0_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT0_OE));                
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK0_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK0_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT0_OE, Drive));     
            BDBG_MSG(("SEROUT_OE set for I2S0"));
            break;  
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
        case BRAP_OutputPort_eI2s1:
#if 	BCHP_7411_VER > BCHP_VER_C0
		/* For 7411, only serial out0 and serial out3 pins come out of chip. 
		 * For 7411D0 "multichannel on I2S" feature, connect I2S0 to serial 
		 * out 0 pins and I2S1 to serial out 3 pins.
		 */
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK3_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK3_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT3_OE));    		 
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK3_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK3_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT3_OE, Drive));    
            BDBG_MSG(("SEROUT_OE set for I2S1"));            
#else
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK1_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK1_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT1_OE));    
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK1_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK1_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT1_OE, Drive));    
            BDBG_MSG(("SEROUT_OE set for I2S1"));            
#endif			
            break;
        case BRAP_OutputPort_eI2s2:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK2_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK2_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT2_OE));                
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK2_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK2_OE, Drive));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT2_OE, Drive));            
            BDBG_MSG(("SEROUT_OE set for I2S2"));            
            break;   
#endif            
        default:
            BDBG_ERR (("BRAP_OP_P_I2sStart(): Incorrect Output port %d", 
                        hOp->eOutputPort));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE, ui32RegVal);

 #if 	BCHP_7411_VER > BCHP_VER_C0
     /* Read-Modify-Write AUD_FMM_MISC_SEROUT_SEL. For 7411D0 "multichannel on I2S" 
      * feature, connect I2S1 to serial out 3 pins. */
    	ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_SEL );
	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_FMM_MISC_SEROUT_SEL, SEROUT3_SEL)))
				|BCHP_FIELD_ENUM(AUD_FMM_MISC_SEROUT_SEL, SEROUT3_SEL,  I2S_Formatter_1);
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_SEL, ui32RegVal);
 #endif
    

    /* Configure the on-the-fly paramters for Spdif port */
    BRAP_OP_P_I2sHWConfig (hOp, &sSettings);

    /* Read current value of the I2S config register */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, 
            BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset);
    
	ui32RegVal &= ~(BRAP_MASK (AUD_FMM_OP_CTRL_I2S_CFG, 
                BITS_PER_SAMPLE));  

    if ((sParams.uiBitsPerSample > 0) && (sParams.uiBitsPerSample < 32))
    {
        ui32RegVal |= (BRAP_FIELD_DATA (AUD_FMM_OP_CTRL_I2S_CFG, 
                                        BITS_PER_SAMPLE, sParams.uiBitsPerSample));    
    }
    else if (sParams.uiBitsPerSample == 32)
    {
        ui32RegVal |= (BRAP_FIELD_DATA (AUD_FMM_OP_CTRL_I2S_CFG, 
                                        BITS_PER_SAMPLE, 0));    
    }
    else 
    {
        BDBG_ERR (("BRAP_OP_P_I2sStart(): Incorrect Bits per Sample %d", 
                    sParams.uiBitsPerSample));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    if (hOp->bMute == false)
    {
    	ui32RegVal &= ~(BRAP_MASK (
                    AUD_FMM_OP_CTRL_I2S_CFG, 
                    DATA_ENABLE));      
        /* Enable DATA_ENABLE only if ouput port is unMuted */
        ui32RegVal |= (BRAP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG, 
                                    DATA_ENABLE, Enable));
    }

    /* Write to the register */
    BRAP_Write32 (hOp->hRegister, 
            BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset, 
            ui32RegVal);

    /* Enable I2S upstream request */
    ret = BRAP_OP_P_EnableStream (hOp, true);

    BDBG_LEAVE (BRAP_OP_P_I2SStart);
    return ret;
}

static BERR_Code
BRAP_OP_P_I2sStop (
    BRAP_OP_P_Handle        hOp             /*[in] Output port handle */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal = 0;

    BDBG_ENTER (BRAP_OP_P_I2sStop);
    BDBG_ASSERT (hOp);
    BDBG_MSG(("BRAP_OP_P_I2sStop: entering"));
   
    /* Program the following register fields:
     (here X indicates I2S index i.e. 0,1,2)

     Program the following to enable I2S port 
     AUD_FMM_OP_CTRL_I2SX_CFG : 
                                 DATA_ENABLE = Disable 
     AUD_FMM_MISC_SEROUT_OE  : LRCKX_OE = SCLKX_OE = SDATX_OE = Drive 
    TODO: Chekc whether its Drive or Tristate??!!
    */

    /* Program AUD_FMM_OP_CTRL_I2S_CFG_i_ as following (this is to Disable I2S)
       DATA_ENABLE = Disable 
    */

    /* Read current value of the I2S config register */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, 
            BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset);


    /* Enable DATA_ENABLE */
    ui32RegVal &= ~(BRAP_MASK (AUD_FMM_OP_CTRL_I2S_CFG, DATA_ENABLE));    
    ui32RegVal |= (BRAP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG, 
                                    DATA_ENABLE, Disable));
    
    /* Write to the register */
    BRAP_Write32 (hOp->hRegister, 
            BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset, 
            ui32RegVal);

    /* Read-Modify-Write AUD_FMM_MISC_SEROUT_OE */
    ui32RegVal = BRAP_Read32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE);
    switch (hOp->eOutputPort)
    {
        case BRAP_OutputPort_eI2s0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK0_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK0_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT0_OE));               
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK0_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK0_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT0_OE, Tristate));            
            break;       
#if defined (  BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
        case BRAP_OutputPort_eI2s1:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK1_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK1_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT1_OE));               
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK1_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK1_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT1_OE, Tristate));            
            break;
        case BRAP_OutputPort_eI2s2:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, LRCK2_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SCLK2_OE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_MISC_SEROUT_OE, SDAT2_OE));               
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            LRCK2_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SCLK2_OE, Tristate));            
            ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_SEROUT_OE, 
                                            SDAT2_OE, Tristate));            
            break;   
#endif            
        default:
            BDBG_ERR (("BRAP_OP_P_I2sStop(): Incorrect Output port %d", 
                        hOp->eOutputPort));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    BRAP_Write32 (hOp->hRegister, BCHP_AUD_FMM_MISC_SEROUT_OE, ui32RegVal);
    
    /* Disable I2S upstream request */
    ret = BRAP_OP_P_EnableStream (hOp, false);

    BDBG_LEAVE (BRAP_OP_P_I2sStop);
    return ret;
}

#ifndef BCHP_7411_VER

BERR_Code BRAP_OP_P_MaiEnable ( 
		BRAP_ChannelHandle	hRapCh,	 /*[in] Rap Channel handle */
        bool                bEnable  /*[in] TRUE: Enable MAi, FALSE:Disable */ 
)
{
	uint32_t ui32RegVal=0;

	BDBG_ENTER (BRAP_OP_MaiEnable);
	BDBG_ASSERT (hRapCh);

	/* Enable the MAI  Formatter */
	ui32RegVal = BRAP_Read32 (hRapCh->hRap->hRegister,BCHP_AUD_FMM_OP_CTRL_MAI_CFG );
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,ENABLE_MAI));
	if( bEnable	== true)
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
										ENABLE_MAI, 
										Enable));
	}
	else
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
										ENABLE_MAI, 
										Disable));
	}
	BRAP_Write32 (hRapCh->hRap->hRegister,BCHP_AUD_FMM_OP_CTRL_MAI_CFG,ui32RegVal);

	BDBG_LEAVE(BRAP_OP_MaiEnable);
	return BERR_SUCCESS;
}

BERR_Code BRAP_OP_P_MaiEnableGet ( 
		BRAP_ChannelHandle	hRapCh,	       /*[in] Rap Channel handle */
        bool                *pEnableState  /*[Out] TRUE: Enable MAi, FALSE:Disable */ 
)
{
	uint32_t ui32RegVal=0;

	BDBG_ENTER(BRAP_OP_MaiEnableGet);
	BDBG_ASSERT (hRapCh);

	/* Read the MAI  Formatter */
	ui32RegVal = BRAP_Read32 (hRapCh->hRap->hRegister,BCHP_AUD_FMM_OP_CTRL_MAI_CFG );
	if(ui32RegVal & (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, ENABLE_MAI, MASK)))
	{
		*pEnableState = true;
	}
	else
	{
		*pEnableState = false;
	}

	BDBG_LEAVE(BRAP_OP_MaiEnableGet);
	return BERR_SUCCESS;
}

/* Following function Configures the MAI formatter */
BERR_Code
BRAP_OP_P_MaiHWConfig (
    BRAP_OP_P_Handle        hOp,       /*[in] Rap handle */
    const BRAP_OP_P_MaiSettings *pMaiSettings /* [in]Pointer to MAI settings */
)
{
	uint32_t  ui32RegVal = 0;

	BDBG_ENTER (BRAP_OP_P_MaiHWConfig);
	BDBG_ASSERT (hOp);

	/* Read current value of the MAI  config register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,LR_SELECT));
	
	switch ( pMaiSettings->sExtSettings.eChannelOrder )
	{
		case BRAP_OP_ChannelOrder_eBothLeft:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            LR_SELECT, 
		                            Normal));
			break;
		case BRAP_OP_ChannelOrder_eBothRight:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            LR_SELECT, 
		                            Normal));
			break;
		case BRAP_OP_ChannelOrder_eNormal:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            LR_SELECT, 
		                            Normal));
			break;
		case BRAP_OP_ChannelOrder_eSwap:				
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            LR_SELECT, 
		                            Swap));
			break;
		default:
			BDBG_ERR (("BRAP_OP_P_MaiHWConfig(): Unsupported Channel order %d", 
		         				pMaiSettings->sExtSettings.eChannelOrder));
			return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	/* Write to the MAI config register */
	BRAP_Write32 (hOp->hRegister, 
			BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset, 
			ui32RegVal);
   
	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{  
		/* store the settings in hOp */
		hOp->uOpSettings.sMai = *pMaiSettings;
	}
    
	return BERR_SUCCESS;
}

static BERR_Code
BRAP_OP_P_MaiOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_MaiSettings * pSettings /*[in] Open parameter */
)
{
	BDBG_ENTER (BRAP_OP_P_MaiOpen);
    BSTD_UNUSED (hOp);
    BSTD_UNUSED(pSettings);

      
	BDBG_LEAVE (BRAP_OP_P_MaiOpen);
    return BERR_SUCCESS;
}

static BERR_Code
BRAP_OP_P_MaiStart (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_MaiParams * pParams /*[in] Start time parameter */
)
{

/*     Algo:
    1. Program the start-time-only paramters in BCHP_AUD_FMM_OP_CTRL_MAI_CFG
       ie SPDIF_MODE, HONOR_FORMAT
    2. Program BCHP_AUD_FMM_OP_CTRL_STREAM_CFG based on eMaiMuxSelector 
    3. Program AUD_FMM_OP_CTRL_MAI_FORMAT based on eMaiAudioFormat 
    4. Program sample width in BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT 
    5. Configure the on-the-fly paramters ie program register BCHP_AUD_FMM_OP_CTRL_MAI_CFG
        x with the settings saved in hRap ie
        hRap->sOutputSettings[BRAP_OutputPort_Mai].uOutputPortSettings.sMaiSettings
        by calling BRAP_OP_P_MaiHWConfig()
        only paramter here is eChannelOrder
    6. If the port was unmuted, enable data flow.
    7. Enable upstream request.
*/
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;
	BRAP_OP_P_MaiParams sParams;
    BRAP_OP_P_MaiSettings sMaiSettings;     

	BDBG_ENTER (BRAP_OP_P_MaiStart);
	BDBG_ASSERT (hOp);

	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{ 
		BDBG_ASSERT (pParams);
		hOp->uOpParams.sMai = *pParams;
		sParams = *pParams;
	}
	else
	{
		sParams = hOp->uOpParams.sMai; 
	}

    sMaiSettings.sExtSettings= 
        hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings;    
    
	BDBG_MSG(("BRAP_OP_P_MaiStart: settings/parameters are:"));
    BDBG_MSG(("sParams->eTimebase=%d", sParams.eTimebase));
    BDBG_MSG(("sParams->uiSampleWidth=%d", sParams.uiSampleWidth));    
    BDBG_MSG(("sExtSettings.bSpdifoutput=%d", sMaiSettings.sExtSettings.bSpdifoutput));  
    BDBG_MSG(("sExtSettings.eChannelOrder=%d", sMaiSettings.sExtSettings.eChannelOrder));  
    BDBG_MSG(("sExtSettings.eMaiAudioFormat=%d", sMaiSettings.sExtSettings.eMaiAudioFormat));  
    BDBG_MSG(("sExtSettings.eMaiMuxSelector=%d", sMaiSettings.sExtSettings.eMaiMuxSelector));  
      
	/* Read current value of the MAI  config register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,SPDIF_MODE));

	if ( sMaiSettings.sExtSettings.bSpdifoutput == true )
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            SPDIF_MODE, 
		                            SPDIF_Format));
	}
	else
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            SPDIF_MODE, 
		                            PCM_Format));
	}
#if ( BCHP_CHIP == 7400 )
	/* Program the HONOR_FORMAT field according to the eMaiAudioFormat */
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,HONOR_FORMAT));

	if ( ( sMaiSettings.sExtSettings.eMaiAudioFormat == BRAP_OP_MaiAudioFormat_eSpdif6Channel ) ||
		( sMaiSettings.sExtSettings.eMaiAudioFormat == BRAP_OP_MaiAudioFormat_eSpdif6Channel ) )
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            HONOR_FORMAT, 
		                            Use_Format));
	}
	else
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
		                            HONOR_FORMAT, 
		                            Two_Channel));
	}
#endif
    
	BRAP_Write32 (hOp->hRegister, 
			BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset, 
			ui32RegVal);

	/* Read current value of the output stream config register, modify, and 
	update the register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_STREAM_CFG + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_STREAM_CFG,MAI_SEL));
	
	if ( sMaiSettings.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eSpdif )
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
								MAI_SEL, 
								STRM0));
	}
	else
	{
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
								MAI_SEL, 
								STRM1));
	}
	BRAP_Write32 (hOp->hRegister, 
				BCHP_AUD_FMM_OP_CTRL_STREAM_CFG + hOp->ui32Offset, 
				ui32RegVal);

	/* Read current value of the MAI  Formatter register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT,AUDIO_FORMAT));
	
	switch (sMaiSettings.sExtSettings.eMaiAudioFormat)
	{
		case BRAP_OP_MaiAudioFormat_eIdle:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            Idle));
			break;
#if (BRAP_7401_FAMILY == 1)
		case BRAP_OP_MaiAudioFormat_ePcmMono:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            PCM_Mono));
			break;
		case BRAP_OP_MaiAudioFormat_ePcmStereo:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            PCM_Stereo));				
			break;
#else
	/* From 7400 Onwards */
		case BRAP_OP_MaiAudioFormat_eSpdif2Channel:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            SPDIF_2_Channel));
			break;
		case BRAP_OP_MaiAudioFormat_eSpdif6Channel:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            SPDIF_6_Channel));				
			break;
		case BRAP_OP_MaiAudioFormat_eSpdif8Channel:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            SPDIF_8_Channel));				
			break;
#endif			
		case BRAP_OP_MaiAudioFormat_eSpdifCompressed:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            SPDIF_Compressed));				
			break;
		case BRAP_OP_MaiAudioFormat_eSpdifLinearPcm:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, 
		                            AUDIO_FORMAT, 
		                            SPDIF_LinearPCM));				
			break;
		default:
			BDBG_ERR (("BRAP_OP_P_MaiHWConfig(): Invalid MAI audio format %d", 
		         				sMaiSettings.sExtSettings.eMaiAudioFormat));
			return BERR_TRACE (BERR_INVALID_PARAMETER);
	}

	/* Write to the MAI Formatter register */
	BRAP_Write32 (hOp->hRegister, 
			BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT + hOp->ui32Offset, 
			ui32RegVal);

	/* Program the BIT per sample */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT + hOp->ui32Offset );
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT,SAMPLE_WIDTH));
	if ((sParams.uiSampleWidth > 0) && (sParams.uiSampleWidth < 32))
	{
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_MAI_FORMAT, 
	                            SAMPLE_WIDTH, 
	                            sParams.uiSampleWidth));
	}
	else if (sParams.uiSampleWidth == 32 )
	{
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_MAI_FORMAT, 
	                            SAMPLE_WIDTH, 
	                            0));
	}
	else
	{
		BDBG_ERR (("BRAP_OP_P_MaiStart(): Incorrect Bits per Sample %d", 
				sParams.uiSampleWidth));
		return BERR_TRACE (BERR_NOT_SUPPORTED);

	}

	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_FORMAT,SAMPLE_RATE));

        switch(hOp->eSamplingRate)
        {   /* Check enum values */ 
            case BAVC_AudioSamplingRate_e32k:
                ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, Rate_32000Hz));
                break;
            case BAVC_AudioSamplingRate_e44_1k:
                ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, Rate_44100Hz));
                break;
            case BAVC_AudioSamplingRate_e48k:
                ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, Rate_48000Hz));
                break;
            case BAVC_AudioSamplingRate_e96k:
                ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, Rate_96000Hz));
                break;
            default:
                ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_FORMAT, SAMPLE_RATE, Not_Indicated));
                break;
        }

	BRAP_Write32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_MAI_FORMAT + hOp->ui32Offset, 
					ui32RegVal);

	ret = BRAP_OP_P_MaiHWConfig ( hOp, &sMaiSettings );
	if ( ret != BERR_SUCCESS )
	{
		BDBG_ERR (("BRAP_OP_P_MaiStart(): Failure in MAI H/W Config %d"));
		return BERR_TRACE (ret);	
	}


	/* Enable the MAI  Formatter */
		ui32RegVal = BRAP_Read32 (hOp->hRegister, 
							BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset );
		ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,ENABLE_MAI));
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
							ENABLE_MAI, 
							Enable));
		BRAP_Write32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset, 
					ui32RegVal);
		
	/* Program the time base selection : If the mux selector is SPDIF it gets programmed during spdif start
	  * but for flex we do not have a start so we are programing here */
       if ( sMaiSettings.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eFlex )
        {
            BRAP_OP_P_ProgramTimebase(hOp->hFmm->hOp[BRAP_OutputPort_eFlex],
                hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uOpSettings.sFlex.sExtSettings.ePll, 
                hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uOpParams.sFlex.eTimebase);

            BDBG_MSG(("BRAP_OP_P_MaiStart: TimeBase parameters are:"));
            BDBG_MSG(("ePll=%d", hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uOpSettings.sFlex.sExtSettings.ePll));
            BDBG_MSG(("eTimeBase=%d", hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uOpParams.sFlex.eTimebase));    
        }

	/* Enable MAI upstream request */
	ret = BRAP_OP_P_EnableStream (hOp, true);

	BDBG_LEAVE (BRAP_OP_P_MaiStart);
	return ret;
}

static BERR_Code
BRAP_OP_P_MaiStop (
    BRAP_OP_P_Handle        hOp    /*[in] Output port handle */
)
{
	BERR_Code ret = BERR_SUCCESS;
#if 0
	uint32_t  ui32RegVal = 0;
#endif

	BDBG_ENTER (BRAP_OP_P_MaiStop);
	BDBG_ASSERT (hOp);

	BDBG_MSG(("BRAP_OP_P_MaiStop: entering"));

#if 0
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset );
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MAI_CFG,ENABLE_MAI));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MAI_CFG, 
						ENABLE_MAI, 
						Disable));
	BRAP_Write32 (hOp->hRegister, 
				BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset, 
				ui32RegVal);

	/*Disable the MAI upstream request */
	ret = BRAP_OP_P_EnableStream (hOp, false);
#else
	BSTD_UNUSED(hOp);
#endif

	BDBG_LEAVE (BRAP_OP_P_MaiStop);
	return ret;
}



static BERR_Code
BRAP_OP_P_FlexOpen (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_FlexSettings * pSettings /*[in] Open parameter */
)
{
	BERR_Code ret = BERR_SUCCESS;
	BRAP_OP_P_FlexSettings sSettings;
	uint32_t  ui32RegVal = 0;
	unsigned int uiTemp;

	BDBG_ENTER (BRAP_OP_P_FlexOpen);
	BDBG_ASSERT (hOp);

	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{   /* If not in WatchDog recovery */  
		BDBG_ASSERT (pSettings);
		sSettings = *pSettings;
	}
	else
	{
		sSettings = hOp->uOpSettings.sFlex;
	}

	/* Set SCLKS_PER_1FS_DIV32 field. This field times 32 is the number 
	of sclk cycles in one Fs period. For example, program this register
	to 2 for a 64fs bit clock. */
	switch (sSettings.sExtSettings.eSClkRate)
	{
		case BRAP_OP_SClkRate_e64Fs: 
			uiTemp = 64;
			break;
		case BRAP_OP_SClkRate_e128Fs: 
			uiTemp = 128;
			break;
		default:
			BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect SClk %d", 
						sSettings.sExtSettings.eSClkRate));
		return BERR_TRACE (BERR_NOT_SUPPORTED);
	}
    
	/* Set MCLK/(2*SCLK) ratio in MCLK_RATE field */
	switch (sSettings.sExtSettings.eMClkRate)
	{
		case BRAP_OP_MClkRate_e128Fs:
			uiTemp = (128 / (2 * uiTemp));
			break; 
		case BRAP_OP_MClkRate_e256Fs:
			uiTemp = (256 / (2 * uiTemp));
			break; 
		case BRAP_OP_MClkRate_e384Fs:
			uiTemp = (384 / (2 * uiTemp));
			break; 
		case BRAP_OP_MClkRate_e512Fs:
			uiTemp = (512 / (2 * uiTemp));
			break; 
		default:
			BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect MClk %d", 
						sSettings.sExtSettings.eMClkRate));
		return BERR_TRACE (BERR_NOT_SUPPORTED);
	}

	/* Read current value of the FLEX  config register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FLEX + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FLEX,MCLK_RATE));
    
	switch (uiTemp)
	{
		case 1:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
			                MCLK_RATE, MCLK_128fs_SCLK_64fs));
			break;     
		case 2:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
			                MCLK_RATE, MCLK_256fs_SCLK_64fs));
			break;     
		case 3:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
			                MCLK_RATE, MCLK_384fs_SCLK_64fs));
			break;     
		case 4:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
			                MCLK_RATE, MCLK_512fs_SCLK_64fs));
			break;     
		default:
			BDBG_ERR (("BRAP_OP_P_FlexOpen(): Unsupported MClk/SClk Ratio %d", 
						uiTemp));
		return BERR_TRACE (BERR_NOT_SUPPORTED);
	} 

	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_MCLK_CFG_FLEX,PLLCLKSEL));
  
	/* Set the PLL */
	/* The PLL Channels are choosen in the following way.

	If MClock rate is 128 Fs, Channel 1 should be selected,
	If MClock rate is 256 Fs, Channel 2 should be selected,
	If MClock rate is 512 Fs, Channel 3 should be selected,

	Basically Channels 1,2 and 3 correspond to 128Fs, 256Fs and 512Fs respectively.

	(MClock 384Fs can be set by user programming only).
	*/
	switch (sSettings.sExtSettings.ePll)
	{
		case BRAP_OP_Pll_ePll0:
			/* Set MCLK rate and PLL Channels */
			switch (sSettings.sExtSettings.eMClkRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL0_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL0_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL0_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect MClk Rate %d", 
								sSettings.sExtSettings.eMClkRate));
				return BERR_TRACE (BERR_NOT_SUPPORTED);
			}     
			break;

		case BRAP_OP_Pll_ePll1:
			/* Set MCLK rate and PLL Channels */
			switch (sSettings.sExtSettings.eMClkRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL1_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL1_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL1_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect MClk Rate %d", 
								sSettings.sExtSettings.eMClkRate));
				return BERR_TRACE (BERR_NOT_SUPPORTED);
			} 
			break;

#if ( BCHP_CHIP == 7400 )			
		case BRAP_OP_Pll_ePll2:
			/* Set MCLK rate and PLL Channels */
			switch (sSettings.sExtSettings.eMClkRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL2_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL2_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_FLEX, 
								PLLCLKSEL, PLL2_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect MClk Rate %d", 
								sSettings.sExtSettings.eMClkRate));
				return BERR_TRACE (BERR_NOT_SUPPORTED);
			} 
			break;
#endif /* defined ( BCHP_CHIP == 7400 ) */

		default:
			BDBG_ERR (("BRAP_OP_P_FlexOpen(): Incorrect PLL chosen %d", 
						sSettings.sExtSettings.ePll));
			return BERR_TRACE (BERR_NOT_SUPPORTED);
	}

	BRAP_Write32 (hOp->hRegister, 
		BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_FLEX + hOp->ui32Offset,
		ui32RegVal);

	/* Read current value of the FLEX  config register */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_FLEX_CFG1 + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG1,IGNORE_VALID));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG1,
						IGNORE_VALID, Ignore));	
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG1,IGNORE_REQ_TO_SEND));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG1,
						IGNORE_REQ_TO_SEND, Ignore));
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG1,ENABLE));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG1, 
						ENABLE, 
						Enable));	
	BRAP_Write32 (hOp->hRegister, 
		BCHP_AUD_FMM_OP_CTRL_FLEX_CFG1 + hOp->ui32Offset,
		ui32RegVal);

	/* Set the Clock to MCLK mode */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2 + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG2,CLOCK_SEL));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG2,
						CLOCK_SEL, Mclk_Mode));	
#if 0	
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG2,CLOCK_ENABLE));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG2,
						CLOCK_ENABLE, Enable));	
#endif	
	BRAP_Write32 (hOp->hRegister, 
		BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2 + hOp->ui32Offset,
		ui32RegVal);

	/* We need to program this field to 1 as per Keith's suggestion
	  * for MAI to get proper 48KHz clock when linked with Flex
	  */
	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
						BCHP_AUD_FMM_OP_CTRL_FLEX_THROTTLE + hOp->ui32Offset);
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_THROTTLE,THROTTLE));
	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_FLEX_THROTTLE,
						THROTTLE, 1));	
	BRAP_Write32 (hOp->hRegister, 
		BCHP_AUD_FMM_OP_CTRL_FLEX_THROTTLE + hOp->ui32Offset,
		ui32RegVal);

	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{  
		/* dont store the settings unless everything's correct */
		hOp->uOpSettings.sFlex = sSettings;
	}
	
	BDBG_MSG(("BRAP_OP_P_FlexOpen: Success"));
	BDBG_LEAVE (BRAP_OP_P_FlexOpen);
	return ret;
}

#if 0 /* TODO: Enable Once FLEX port is implemented properly */
static BERR_Code
BRAP_OP_P_FlexStart (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_FlexParams * pParams /*[in] Start time parameter */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;
	BRAP_OP_P_FlexParams sParams;

	BDBG_ENTER (BRAP_OP_P_FlexStart);
	BDBG_ASSERT (hOp);
	BDBG_MSG(("BRAP_OP_P_FlexStart: entering"));

	if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hOp->hFmm) == false)
	{ 
		BDBG_ASSERT (pParams);
		hOp->uOpParams.sFlex = *pParams;
		sParams = *pParams;
	}
	else
	{
		sParams = hOp->uOpParams.sFlex; 
	}

	/* TODO: Program the FLEX configuration Parameters properly */
	
	/* Enable the FLEX  Formatter */
	if (hOp->bMute == false)
	{
		ui32RegVal = BRAP_Read32 (hOp->hRegister, 
							BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2 );
		ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG2,DATA_ENABLE));
		ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG2, 
							DATA_ENABLE, 
							Enable));
		BRAP_Write32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2, 
					ui32RegVal);
	}

	/* TODO: Program the time base selection */

	/* TODO: Enable FLEX upstream request */

	BDBG_LEAVE (BRAP_OP_P_FlexStart);
	return ret;
}

static BERR_Code
BRAP_OP_P_FlexStop (
    BRAP_OP_P_Handle        hOp    /*[in] Output port handle */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;

	BDBG_ENTER (BRAP_OP_P_FlexStop);
	BDBG_ASSERT (hOp);

	BDBG_MSG(("BRAP_OP_P_FlexStop: Entering"));


	ui32RegVal = BRAP_Read32 (hOp->hRegister, 
					BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2 );
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_OP_CTRL_FLEX_CFG2,DATA_ENABLE));
	ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_FLEX_CFG2, 
						DATA_ENABLE, 
						Disable));
	
	BRAP_Write32 (hOp->hRegister, 
				BCHP_AUD_FMM_OP_CTRL_FLEX_CFG2, 
				ui32RegVal);

	/*TODO: Disable the FLEX upstream request */

	BDBG_LEAVE (BRAP_OP_P_FlexStop);
	return ret;
}
#endif
#endif

BERR_Code 
BRAP_OP_P_ProgramTimebase (
    BRAP_OP_P_Handle       hOp, 
    BRAP_OP_Pll            ePll,
    BAVC_Timebase          eTimebase
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_OP_P_ProgramTimebase);

    BKNI_EnterCriticalSection();
    ret = BRAP_OP_P_ProgramTimebase_isr(hOp, ePll, eTimebase);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE (BRAP_OP_P_ProgramTimebase);
    return ret;
}

BERR_Code 
BRAP_OP_P_ProgramTimebase_isr (
    BRAP_OP_P_Handle       hOp, 
    BRAP_OP_Pll            ePll,
    BAVC_Timebase          eTimebase
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegValue = 0;
    uint32_t  ui32Offset=0;
    
    BDBG_ENTER (BRAP_OP_P_ProgramTimebase_isr);
    BDBG_MSG (("BRAP_OP_P_ProgramTimebase_isr(): handle=0x%08x, port=%d, ePll=%d eTimebase=%d",
                hOp, hOp->eOutputPort, ePll, eTimebase));

    ui32Offset = (int)(ePll) * (BCHP_AUD_FMM_PLL1_MACRO - BCHP_AUD_FMM_PLL0_MACRO);    

    /* AUD_FMM_PLL0_MACRO, MACRO_SELECT is presently prgmed by DSP */
    
    /* Select the proper timebase in PLLX_CONTROL Register */
    ui32RegValue = BRAP_Read32_isr(hOp->hRegister, BCHP_AUD_FMM_PLL0_CONTROL + ui32Offset );
    ui32RegValue &= ~(BCHP_MASK (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT));

    /* Check: Store Timebase in OP object */
#if (BCHP_CHIP == 7400 && BCHP_VER !=BCHP_VER_A0)
    switch(eTimebase)
    {   /* Check enum values */ 
        case BAVC_Timebase_e0:
        case BAVC_Timebase_e2:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_0));
            break;
        case BAVC_Timebase_e1:
        case BAVC_Timebase_e3:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, VCXO_1));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_ProgramTimebase_isr(): Incorrect Time Base %d",
                        eTimebase));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
#else
    switch(eTimebase)
    {   /* Check enum values */ 
        case BAVC_Timebase_e0:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27a));
            break;
        case BAVC_Timebase_e1:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27b));
            break;
        case BAVC_Timebase_e2:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27c));
            break;
        case BAVC_Timebase_e3:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27d));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_ProgramTimebase_isr(): Incorrect Time Base %d",
                        eTimebase));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
#endif
    BRAP_Write32_isr(hOp->hRegister, BCHP_AUD_FMM_PLL0_CONTROL + ui32Offset , ui32RegValue);

    BDBG_LEAVE (BRAP_OP_P_ProgramTimebase_isr);

    return ret;
}


/* Following function enables/disables the upstream request from the o/p port temporarily */
BERR_Code
BRAP_OP_P_EnableStream (
    BRAP_OP_P_Handle    hOp,       /*[in] Output port handle */
    bool                bEnable    /*[in] TRUE: Enable Stream 
                                          FALSE: Disable */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_OP_P_EnableStream);

    BKNI_EnterCriticalSection();
    ret = BRAP_OP_P_EnableStream_isr(hOp, bEnable);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE (BRAP_OP_P_EnableStream);
    return ret;   
}

BERR_Code
BRAP_OP_P_EnableStream_isr (
    BRAP_OP_P_Handle    hOp,       /*[in] Output port handle */
    bool                bEnable    /*[in] TRUE: Enable Stream 
                                          FALSE: Disable */
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t ui32RegVal = 0;
    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;  

    BDBG_ENTER (BRAP_OP_P_EnableStream_isr);
    
    if (hOp->eOutputPort == BRAP_OutputPort_eMai)
    {
        eMaiMuxSelect = 
            hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }              
    
    /* Select AUD_FMM_OP_CTRL_ENABLE_SET register for enabling & 
              AUD_FMM_OP_CTRL_ENABLE_CLEAR register for disabling */
    if (bEnable == true)
    {
        switch (hOp->eOutputPort)
        {
#ifdef BCHP_7411_VER /* for the 7411 */ 
            case BRAP_OutputPort_eSpdif:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM0_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling SPDIF"));
                break;
            case BRAP_OutputPort_eI2s0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM2_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling I2S0"));
                break;
            case BRAP_OutputPort_eI2s1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM3_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling I2S1"));
                break;
            case BRAP_OutputPort_eI2s2:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM4_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling I2S2"));
                break;                
#else
            case BRAP_OutputPort_eDac0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM2_ENA, 1));
				BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling DAC"));                
                break;
            case BRAP_OutputPort_eI2s0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM3_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling I2S0"));                
                break;
            case BRAP_OutputPort_eSpdif:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM0_ENA, 1));
                BDBG_MSG(("BRAP_OP_P_EnableStream_isr(): Enabling SPDIF"));                
                break;
            case BRAP_OutputPort_eMai:
                if (eMaiMuxSelect == BRAP_OutputPort_eSpdif )
		  {
                	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM0_ENA, 1));
		  }
		  else
		  {
                	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM1_ENA, 1));
		  }
                break;
#if ( BCHP_CHIP == 7400 )
            case BRAP_OutputPort_eDac1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET,
                                                STREAM6_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM7_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s2:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                                STREAM4_ENA, 1));
                break;
#endif /* defined ( BCHP_CHIP == 7400 ) */
            case BRAP_OutputPort_eFlex:
                 /* 7401 TODO: TBD later */
#endif                
            default:
                BDBG_ERR(("BRAP_OP_P_EnableStream_isr(): Incorrect Output port type %d",
                            hOp->eOutputPort));
                return BERR_TRACE (BERR_INVALID_PARAMETER);
        }         
        BRAP_Write32_isr (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, ui32RegVal);
    }
    else
    { 
        switch (hOp->eOutputPort)
        {
#ifdef BCHP_7411_VER /* Only for the 7411 */ 
            case BRAP_OutputPort_eSpdif:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM0_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM2_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM3_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s2:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM4_ENA, 1));
                break;                
#else

            case BRAP_OutputPort_eDac0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM2_ENA, 1));
            break;
            case BRAP_OutputPort_eI2s0:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM3_ENA, 1));
                break;
            case BRAP_OutputPort_eSpdif:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM0_ENA, 1));
                break;                
            case BRAP_OutputPort_eMai:
              if (eMaiMuxSelect == BRAP_OutputPort_eSpdif )
		  {
                	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM0_ENA, 1));
		  }
		  else
		  {
                	ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM1_ENA, 1));
		  }
                break;				
#if ( BCHP_CHIP == 7400 )
            case BRAP_OutputPort_eDac1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR,
                                                STREAM6_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s1:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM7_ENA, 1));
                break;
            case BRAP_OutputPort_eI2s2:
                ui32RegVal |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
                                                STREAM4_ENA, 1));
                break;
#endif /* defined ( BCHP_CHIP == 7400 ) */
            case BRAP_OutputPort_eFlex:
                 /* 7401TODO: TBD later */
#endif                                
            default:
                BDBG_ERR(("BRAP_OP_P_EnableStream_isr(): Incorrect Output port type %d",
                            hOp->eOutputPort));
                return BERR_TRACE (BERR_INVALID_PARAMETER);
        }         
        BRAP_Write32_isr (hOp->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, ui32RegVal);
    }

    BDBG_MSG(("BRAP_OP_P_EnableStream_isr: output port %d bEnable=%d", hOp->eOutputPort, bEnable));

    BDBG_LEAVE (BRAP_OP_P_EnableStream_isr);
    return ret;   
}


BERR_Code BRAP_OP_SetMute ( 
    BRAP_Handle     hRap,    /* [in]Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in]Output Type */
    bool            bMute    /* [in]TRUE: Mute data at the output port
                                    FALSE: UnMute data at the output port */
)
{
	BERR_Code     eStatus;
	BDBG_ENTER (BRAP_OP_SetMute);

	BKNI_EnterCriticalSection();
	eStatus = BRAP_OP_P_SetMute_isr (hRap, eOpType, bMute);
	BKNI_LeaveCriticalSection();
	BDBG_LEAVE (BRAP_OP_SetMute);
	return eStatus;
}

BERR_Code BRAP_OP_GetMute ( 
    BRAP_Handle     hRap,    /* [in]Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in]Output Type */
    bool *          pMute   /* [out] Pointer to memory where the Mute 
                                status is to be written
                                TRUE: output port is Muted
                                FALSE:  output port is not Muted */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_OP_P_Handle       hOp;
    BRAP_OutputPort	eOutputPort = BRAP_OutputPort_eMax;

    BDBG_ENTER (BRAP_OP_GetMute);
#ifdef BCHP_7411_VER
    BSTD_UNUSED(eOutputPort);
#endif

#ifndef BCHP_7411_VER
    if( eOpType == BRAP_OutputPort_eRfMod )
    {
    	eOutputPort = BRAP_OP_P_GetRfmodMuxSelector (hRap);
    	if ( eOutputPort == BRAP_OutputPort_eMax )
    	{
           	BDBG_ERR (("BRAP_OP_GetMute(): Rfmod Mux selector is not yet selected "));
           	return BERR_TRACE (BERR_INVALID_PARAMETER);            
    	}
    	else
    	{
    		hOp = hRap->hFmm[0]->hOp[eOutputPort];
    		*pMute = hOp->bRfmodMuteState;
    		BDBG_MSG(("BRAP_OP_GetMute: bMute=%d",  hOp->bRfmodMuteState));
    		return ret;
    	}
    }
    else
#endif		
    {
    	hOp = hRap->hFmm[0]->hOp[eOpType]; /* TODO: dont hardcode FMM index */
    }

    if (hOp == NULL)
    {
        BDBG_ERR (("BRAP_OP_GetMute(): Output port type %d has not been opened ",
                        eOpType));
        return BERR_TRACE (BERR_INVALID_PARAMETER);            
    }

    *pMute = hOp->bMute ;
    BDBG_MSG(("BRAP_OP_GetMute: bMute=%d",  hOp->bMute));

    BDBG_LEAVE (BRAP_OP_GetMute);
    return ret;
}

BERR_Code BRAP_OP_P_SetMute_isr ( 
    BRAP_Handle     hRap,    /* [in]Audio Device Handle */
    BRAP_OutputPort eOpType, /* [in]Output Type */
    bool            bMute    /* [in]TRUE: Mute data at the output port
                                    FALSE: UnMute data at the output port */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_OP_P_Handle       hOp;
    uint32_t ui32RegVal=0;
    uint32_t ui32RegAddr=0;   

    unsigned int uiSpdiffmId=0; 
    unsigned int uiSpdiffmStreamId=0; 
    BRAP_OutputPort	eOutputPort = BRAP_OutputPort_eMax;

    BDBG_ENTER (BRAP_OP_P_SetMute_isr);

#ifndef BCHP_7411_VER    
    /* TODO: why do we need a seperate section for RFMOD. it should be part of the 
    switchcase below for all output ports */
    if( eOpType == BRAP_OutputPort_eRfMod )
    {
        eOutputPort = BRAP_OP_P_GetRfmodMuxSelector (hRap);
        if ( eOutputPort == BRAP_OutputPort_eMax )
        {
            BDBG_ERR (("BRAP_OP_P_SetMute_isr(): Rfmod Mux selector is not yet selected "));
            return BERR_TRACE (BERR_INVALID_PARAMETER);            
        }
        else
        {
            hOp = hRap->hFmm[0]->hOp[eOutputPort];
            ui32RegAddr = BCHP_HIFIDAC_CTRL0_CONFIG + hOp->ui32Offset;
            ui32RegVal = BRAP_Read32_isr(hRap->hRegister, ui32RegAddr);
            if ( bMute == false )
            {
                ui32RegVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE));
                ui32RegVal |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                    RFMOD_MUTE, Normal_operation ));    
            }
            else
            {
                ui32RegVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE));
                ui32RegVal |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_CONFIG, 
                        RFMOD_MUTE, Mute ));    
            }
            BRAP_Write32_isr (hRap->hRegister, ui32RegAddr, ui32RegVal);
            BDBG_MSG(("BRAP_OP_P_SetMute_isr: output port =%d, bmute=%d", eOpType,bMute)); 
            /* Update the rfmod mute state in handle */
            hOp->bRfmodMuteState = bMute;
            return ret;
        }
    }
    else
#endif		
    {
    	hOp = hRap->hFmm[0]->hOp[eOpType]; /* TODO: dont hardcode FMM index */
    }
	
    if (hOp == NULL)
    {
        BDBG_ERR (("BRAP_OP_P_SetMute_isr(): Output port type %d has not been opened ",
                        eOpType));
        return BERR_TRACE (BERR_INVALID_PARAMETER);            
    }

    /* Mute/unmute only if the output port is already running. If it's not 
    started yet, just store mute value, dont reprogram any registers */
    if (hOp->uiStartCnt== 0)
    {
        BDBG_MSG(("BRAP_OP_P_SetMute_isr(): output port %d hasnt been started yet."
            "Mute/unmute condition will take effect when the port is started.", hOp->eOutputPort));
        goto end_mute;
    }

    if ( eOpType == BRAP_OutputPort_eFlex )
    {
    	/* We do not enable the Flex output port internally, so we need not mute it */
        goto end_mute;
    }

    /* Find the correct register to read/write for this port */
    switch (eOpType)
    {
        case BRAP_OutputPort_eSpdif:
            ui32RegAddr = BCHP_AUD_FMM_OP_CTRL_SPDIF_CFG_0 + hOp->ui32Offset;
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            ui32RegAddr = BCHP_AUD_FMM_OP_CTRL_I2S_CFG + hOp->ui32Offset;
            break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
    case BRAP_OutputPort_eMai:
            ui32RegAddr = BCHP_AUD_FMM_OP_CTRL_MAI_CFG + hOp->ui32Offset;
            break;
    case BRAP_OutputPort_eDac0:
    case BRAP_OutputPort_eDac1:
            ui32RegAddr = BCHP_HIFIDAC_CTRL0_MUTECTRL_DACONLY + hOp->ui32Offset;     
            break;

#endif
        default:
            BDBG_ERR (("BRAP_OP_P_SetMute_isr(): Output port type %d not supported by this PI yet ",
                        eOpType));
            return BERR_TRACE (BERR_NOT_SUPPORTED);            
    }
    ui32RegVal = BRAP_Read32_isr(hRap->hRegister, ui32RegAddr);

    /* Mute/Unmute the output port by controlling the DISABLE_DATA field */
    /* Note: For output ports using the SPDIFFM, dont disable "DATA_EnABLE" 
    bit. Keep it enabled always. Instead, program the OVERWRITE_DATA bit so 
    that a null/pause burst or a dither signal (for compressed and PCM data 
    respectively) get inserted on a MUTE. This is being done for PR 26509 */
    if (bMute == false)
    {
        switch (eOpType)
        {
            case BRAP_OutputPort_eSpdif:
                BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                               BRAP_OutputPort_eSpdif, 
                                               &uiSpdiffmId, 
                                               &uiSpdiffmStreamId);     
                BRAP_SPDIFFM_P_InsertBurstOnMute (hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId], bMute);              
                break;
            case BRAP_OutputPort_eI2s0:
            case BRAP_OutputPort_eI2s1:
            case BRAP_OutputPort_eI2s2:
            	ui32RegVal &= ~(BRAP_MASK (AUD_FMM_OP_CTRL_I2S_CFG, DATA_ENABLE));      
                ui32RegVal |= (BRAP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG, 
                                DATA_ENABLE, Enable));                
                break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
    case BRAP_OutputPort_eMai:
                BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                               hOp->uOpSettings.sMai.sExtSettings.eMaiMuxSelector, 
                                               &uiSpdiffmId, 
                                               &uiSpdiffmStreamId);                
                BRAP_SPDIFFM_P_InsertBurstOnMute (hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId], bMute);                                
                break;
    case BRAP_OutputPort_eDac0:
    case BRAP_OutputPort_eDac1:
                ui32RegVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
                ui32RegVal |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                            MUTEDAC, Ramp_unmute ));    
                break;
#endif     
            default:
                BDBG_ERR (("BRAP_OP_P_SetMute_isr(): Output port type %d not supported by this PI yet ",
                        eOpType));
                return BERR_TRACE (BERR_NOT_SUPPORTED);                  
        }
    }
    else
    {
        switch (eOpType)
        {
             case BRAP_OutputPort_eSpdif:
                BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                               BRAP_OutputPort_eSpdif, 
                                               &uiSpdiffmId, 
                                               &uiSpdiffmStreamId);    
                BRAP_SPDIFFM_P_InsertBurstOnMute (hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId], bMute); 
                break;
            case BRAP_OutputPort_eI2s0:
            case BRAP_OutputPort_eI2s1:
            case BRAP_OutputPort_eI2s2:
                ui32RegVal &= ~(BRAP_MASK (AUD_FMM_OP_CTRL_I2S_CFG, DATA_ENABLE)); 
                ui32RegVal |= (BRAP_FIELD_ENUM (AUD_FMM_OP_CTRL_I2S_CFG, 
                                    DATA_ENABLE, Disable));                
                break;
#ifndef BCHP_7411_VER /* For chips other than 7411 */
            case BRAP_OutputPort_eMai:
                BRAP_RM_P_GetSpdifFmForOpPort (hRap->hRm, 
                                               hOp->uOpSettings.sMai.sExtSettings.eMaiMuxSelector, 
                                               &uiSpdiffmId, 
                                               &uiSpdiffmStreamId);       
                BRAP_SPDIFFM_P_InsertBurstOnMute (hRap->hFmm[0]->hSpdifFm[uiSpdiffmStreamId], bMute);                                
                break;
            case BRAP_OutputPort_eDac0:
            case BRAP_OutputPort_eDac1:
                ui32RegVal &= ~(BCHP_MASK(HIFIDAC_CTRL0_MUTECTRL_DACONLY,MUTEDAC));
                ui32RegVal |= (BCHP_FIELD_ENUM (HIFIDAC_CTRL0_MUTECTRL_DACONLY, 
                                                    MUTEDAC, Ramp_mute ));    
                break;
#endif    
            default:
                BDBG_ERR (("BRAP_OP_P_SetMute_isr(): Output port type not supported %d by this PI yet",
                        eOpType));
                return BERR_TRACE (BERR_NOT_SUPPORTED);                  
        }
    } 

    /* If output  ports required an SPDIFFM, dont change the DATA_ENABLE bit */
    if ((eOpType != BRAP_OutputPort_eSpdif) && (eOpType != BRAP_OutputPort_eMai))
    {
        BRAP_Write32_isr (hRap->hRegister, ui32RegAddr, ui32RegVal);
    }

end_mute:
    BDBG_MSG(("BRAP_OP_P_SetMute_isr: output port =%d, bmute=%d", hOp->eOutputPort,bMute)); 
    hOp->bMute = bMute;

    BDBG_LEAVE (BRAP_OP_P_SetMute_isr);
    return ret;
}


/* This function sets the sampling rate by just selecting MACRO_SELECT */
/* TODO: This function will cause a lock interrupt after 20 uS. 
         So please handle that interrupt */

BERR_Code 
BRAP_OP_P_ProgramPll (
    BRAP_OP_P_Handle       	hOp, 
    BRAP_OP_Pll 			ePll,
    BAVC_AudioSamplingRate 	eSamplingRate
)
{
    BERR_Code ret = BERR_SUCCESS;
    
    BDBG_ENTER (BRAP_OP_P_ProgramPll);
    BKNI_EnterCriticalSection();
    ret = BRAP_OP_P_ProgramPll_isr(hOp, ePll, eSamplingRate);
    BKNI_LeaveCriticalSection();
    
	BDBG_LEAVE (BRAP_OP_P_ProgramPll);
    return ret;
}

BERR_Code 
BRAP_OP_P_ProgramPll_isr (
    BRAP_OP_P_Handle       	hOp, 
    BRAP_OP_Pll 			ePll,
    BAVC_AudioSamplingRate 	eSamplingRate
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegValue = 0;
    uint32_t  ui32Offset=0;
    
    BDBG_ENTER (BRAP_OP_P_ProgramPll_isr);
    BDBG_MSG (("BRAP_OP_P_ProgramPll_isr(): handle=0x%08x ePll=%d eSamplingRate=%d",
                hOp, ePll, eSamplingRate));

    ui32Offset = (int)(ePll) * (BCHP_AUD_FMM_PLL1_MACRO - BCHP_AUD_FMM_PLL0_MACRO);    

    /* Program the MACRO_SELECT = The desired sampling rate */
    ui32RegValue = BRAP_Read32_isr (hOp->hRegister, 
                                BCHP_AUD_FMM_PLL0_MACRO + ui32Offset);
    ui32RegValue &= ~(BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT));

#ifdef BCHP_7411_VER
    switch(eSamplingRate)
    {   /* Check enum values */ 
        case BAVC_AudioSamplingRate_e32k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, FS_32000_Hz));
            break;
        case BAVC_AudioSamplingRate_e44_1k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, FS_44100_Hz));
            break;
        case BAVC_AudioSamplingRate_e48k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, FS_48000_Hz));
            break;
        case BAVC_AudioSamplingRate_e96k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, FS_96000_Hz));
            break;
		case BAVC_AudioSamplingRate_e192k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, FS_192000_Hz));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_ProgramPll_isr(): Incorrect Sampling Rate %d", eSamplingRate));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
#else
	switch(eSamplingRate)
    {   /* Check enum values */ 
        case BAVC_AudioSamplingRate_e32k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_32000));
            break;
        case BAVC_AudioSamplingRate_e44_1k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_44100));
            break;
        case BAVC_AudioSamplingRate_e48k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_48000));
            break;
        case BAVC_AudioSamplingRate_e96k:
            ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, Mult_of_96000));
            break;
        default:
            BDBG_ERR (("BRAP_OP_P_ProgramPll_isr(): Incorrect Sampling Rate %d", eSamplingRate));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
#endif

    BRAP_Write32_isr(hOp->hRegister, BCHP_AUD_FMM_PLL0_MACRO + ui32Offset , ui32RegValue);

	BDBG_LEAVE (BRAP_OP_P_ProgramPll);
    return ret;
}


/***************************************************************************
Summary:
	Gets the sampling rate and time base associated with the output port.
Description:
	This API is used to get the sampling rate and time base associated with 
	a particular output port.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_OP_SetSamplingRate
****************************************************************************/
BERR_Code BRAP_OP_GetSamplingRate ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
	BAVC_AudioSamplingRate  *pSamplingRate, /* [out] Output Sampling rate */
	BAVC_Timebase			*pTimebase 		/* [out] Timebase to use for the output port */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	BRAP_OP_P_Handle       	hOp = NULL;
	
	BDBG_ENTER(BRAP_OP_GetSamplingRate);	

	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRap);
	BDBG_ASSERT(pSamplingRate);
	BDBG_ASSERT(pTimebase);

	BDBG_MSG(("BRAP_OP_GetSamplingRate: hRap = %p"
		"\n\t eOutputPort = %d, pSamplingRate = %p, pTimebase = %p",
		hRap, eOutputPort, pSamplingRate, pTimebase));
	
	if(BRAP_RM_P_IsOutputPortSupported(eOutputPort) != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_OP_GetSamplingRate: eOutputPort(%d) is not supported", eOutputPort));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	/* Find the associated hOp */
       hOp = hRap->hFmm[0]->hOp[eOutputPort]; /* TODO: dont hardcode FMM index */

	if(hOp == NULL)
	{
		BDBG_ERR(("BRAP_OP_GetSamplingRate: hOp[%d] is NULL", eOutputPort));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

	*pSamplingRate = hOp->eSamplingRate;
	switch(hOp->eOutputPort)
	{
		case BRAP_OutputPort_eSpdif:
			*pTimebase = hOp->uOpParams.sSpdif.eTimebase;
			break;
				
		case BRAP_OutputPort_eDac0:
		case BRAP_OutputPort_eDac1:
			*pTimebase = hOp->uOpParams.sDac.eTimebase;
			break;
			
		case BRAP_OutputPort_eI2s0:
		case BRAP_OutputPort_eI2s1:
		case BRAP_OutputPort_eI2s2:
			*pTimebase = hOp->uOpParams.sI2s.eTimebase;
			break;

		case BRAP_OutputPort_eFlex:
			*pTimebase = hOp->uOpParams.sFlex.eTimebase;
			break;

		case BRAP_OutputPort_eMai:
    		    if (hOp->uOpSettings.sMai.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eSpdif)
    				*pTimebase = hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->uOpParams.sSpdif.eTimebase;
    		    else if (hOp->uOpSettings.sMai.sExtSettings.eMaiMuxSelector == BRAP_OutputPort_eFlex)
    				*pTimebase = hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->uOpParams.sFlex.eTimebase;	
				else
				{
					BDBG_ERR(("BRAP_OP_GetSamplingRate: Unknown Mai Mux Selector(%d)",hOp->uOpSettings.sMai.sExtSettings.eMaiMuxSelector));		
					return BERR_TRACE(BERR_INVALID_PARAMETER);
				}
            	break;			
		default:	
			BDBG_ERR(("BRAP_OP_GetSamplingRate: Unknown output port(%d)",hOp->eOutputPort));
			BDBG_ASSERT(0);
	}/* switch */	
	

	BDBG_MSG(("BRAP_OP_GetSamplingRate outparams:"
		"\n\t SamplingRate = %d, Timebase = %d", *pSamplingRate, *pTimebase));
	
	BDBG_LEAVE(BRAP_OP_GetSamplingRate);	
	return ret;
}

/***************************************************************************
Summary:
	Gets the output sampling Size.
Description:
	This API is used to get the  sampling Size  associated
	with mai for a particular rap channel.
Returns:
	BERR_SUCCESS
See Also:

****************************************************************************/
BERR_Code BRAP_OP_GetSamplingSize ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
    unsigned int			*uiSampleSize	/* [out] One Audio Frame Time */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	BRAP_OP_P_Handle       	hOp = NULL;
    BRAP_OP_P_MaiSettings sMaiSettings;     
    
	
	BDBG_ENTER(BRAP_OP_GetSamplingSize);	

	/*  Validate input parameters. */
	BDBG_ASSERT(hRap);

	BDBG_MSG(("BRAP_OP_GetSamplingSize: hRap = %p"
		"\n\t eOutputPort = %d",hRap, eOutputPort ));

    if(eOutputPort != BRAP_OutputPort_eMai)
    {
        BDBG_ERR(("OUTPUT Port must be HDMI(Mai)"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    
	/* Find the associated hOp */
    hOp = hRap->hFmm[0]->hOp[eOutputPort]; /* TODO: dont hardcode FMM index */   
	if(hOp == NULL)
	{
		BDBG_ERR(("BRAP_OP_GetSamplingSize: hOp[%d] is NULL", eOutputPort));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

    sMaiSettings.sExtSettings= 
        hOp->hFmm->hRap->sOutputSettings[eOutputPort].uOutputPortSettings.sMaiSettings;

    if(sMaiSettings.sExtSettings.eMaiAudioFormat == BRAP_OP_MaiAudioFormat_eSpdifCompressed)
    {
        *uiSampleSize = 16;
    }
    else
    {
        *uiSampleSize = 24;    
    }
    	
	BDBG_LEAVE(BRAP_OP_GetSamplingSize);	
	return ret;
}


/**************************************************************************
Summary:
	This routine gets the matching output port for the input capture type.
Description:
    This routine gets the matching output port for the capture input type.    
Returns:
    BERR_SUCCESS on success else error
    Note: The out param peOutputPort will hold invalid value if input type 
          can't be matched to an output port. Ex: External I2S.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_OP_P_GetOpPortMatchingCapInputType(
    BRAP_CapInputType eCapInputType,  /* [in] Capture input port type */                                             
    BRAP_OutputPort   *peOutputPort   /* [out] Output port */                                           
	)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_OP_P_GetOpPortMatchingCapInputType);
    BDBG_ASSERT(peOutputPort);
        
    switch(eCapInputType)
    {
        case BRAP_CapInputType_eIntI2s0: 
            *peOutputPort = BRAP_OutputPort_eI2s0; 
            break;

#if ((BCHP_CHIP != 7401) && (BCHP_CHIP != 7118) && (BCHP_CHIP != 7403))
        case BRAP_CapInputType_eIntI2s1:
            *peOutputPort = BRAP_OutputPort_eI2s1; 
            break;

        case BRAP_CapInputType_eIntI2s2:
            *peOutputPort = BRAP_OutputPort_eI2s2; 
            break;
#endif

        case BRAP_CapInputType_eIntFlex:
            *peOutputPort = BRAP_OutputPort_eFlex; 
            break;
           
        case BRAP_CapInputType_eExtI2s:            
            /* Ext I2S can't be mapped to any of the internally available 
               output ports, so return invalid output port & success */
            *peOutputPort = BRAP_OutputPort_eMax; 
            break;

        default:
            BDBG_ERR(("Unknown capture input type")); 
            BDBG_ASSERT(0);
    }

    BDBG_LEAVE(BRAP_OP_P_GetOpPortMatchingCapInputType);
    return BERR_TRACE(ret);
}

/* Returns handles of DEC/PB/CAP channels directly using this output port. */
BERR_Code BRAP_P_GetChannelsUsingOpPort(
    const BRAP_Handle       hRap, 	        /* [in] The RAP Channel handle */
    const BRAP_OutputPort   eOutputPort,    /* [in] Output port */    
    BRAP_ChannelHandle      *phChannels /* [out] Pointer to the first index 
                                            of an array of direct channel handles */
    )
{
    bool bFlag=false;
    BERR_Code ret = BERR_SUCCESS;
    unsigned int uiChnCnt = 0, i;
    BRAP_ChannelHandle hRapCh = NULL;
    
    BDBG_ENTER(BRAP_P_GetChannelsUsingOpPort);

    /* Get all the channel handles that use the this DP Id and Mixer Id */

    /* Check for decode channels */
    for(i = 0; i < BRAP_RM_P_MAX_DEC_CHANNELS; i++)
    {
        hRapCh = hRap->hRapDecCh[i];
        if(hRapCh != NULL)
        {
             bFlag = false;
             ret = BRAP_P_IsOpPortUsedbyChannel(hRapCh, eOutputPort, &bFlag, NULL /*bClone - ignore */);
             if ((ret == BERR_SUCCESS) && (true == bFlag))
             {
                /* This channel uses this output port */
                if(uiChnCnt == BRAP_RM_P_MAX_MIXER_INPUTS)                    
                {
                    BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: Found more than %d"
                              "channels using same output (%d)",
                              BRAP_RM_P_MAX_MIXER_INPUTS, eOutputPort));                        
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                phChannels[uiChnCnt] = hRapCh;
                uiChnCnt++;
                BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: Dec chan 0x%x uses output port %d", hRapCh, eOutputPort)); 
             }            
        }/* if dec channel */
    } /* for dec channel */

    /* Check for PCM playback channels */
    for(i = 0; i < BRAP_RM_P_MAX_PCM_CHANNELS; i++)
    {
        hRapCh = hRap->hRapPbCh[i];
        if(hRapCh != NULL)
        {    
             bFlag = false;    
             ret = BRAP_P_IsOpPortUsedbyChannel(hRapCh, eOutputPort, &bFlag, NULL /*bClone - ignore */);
             if ((ret == BERR_SUCCESS) && (true == bFlag))
             {
                /* This channel uses this output port */
                if(uiChnCnt == BRAP_RM_P_MAX_MIXER_INPUTS)                    
                {
                    BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: Found more than %d"
                              "channels using same output (%d)",
                              BRAP_RM_P_MAX_MIXER_INPUTS, eOutputPort));                        
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                phChannels[uiChnCnt] = hRapCh;
                uiChnCnt++;
                BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: PB chan 0x%x uses output port %d", hRapCh, eOutputPort)); 
             }  
        }/* if PCM channel */
    } /* for PCM PB channel */

    /* Check for PCM Capture channels */
    for(i = 0; i < BRAP_RM_P_MAX_CAP_CHANNELS; i++)
    {
        hRapCh = hRap->hRapCapCh[i];
        if(hRapCh != NULL)
        {
             bFlag = false;    
             ret = BRAP_P_IsOpPortUsedbyChannel(hRapCh, eOutputPort, &bFlag, NULL /*bClone - ignore */);
             if ((ret == BERR_SUCCESS) && (true == bFlag))
             {
                /* This channel uses this output port */
                if(uiChnCnt == BRAP_RM_P_MAX_MIXER_INPUTS)                    
                {
                    BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: Found more than %d"
                              "channels using same output (%d)",
                              BRAP_RM_P_MAX_MIXER_INPUTS, eOutputPort));                        
                    return BERR_TRACE(BERR_NOT_SUPPORTED);
                }
                phChannels[uiChnCnt] = hRapCh;
                uiChnCnt++;
                BDBG_MSG(("BRAP_P_GetChannelsUsingOpPort: CAP chan 0x%x uses output port %d", hRapCh, eOutputPort)); 
             }              
        }/* if CAP channel */
    } /* for PCM Cap channel */
    
    BDBG_LEAVE(BRAP_P_GetChannelsUsingOpPort);
    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Gets all the direct and indirect channel handles associated to the output
	port.
Description:
    This function returns all the channel handles (direct/indirect) associated
	to the output port. 
	    Direct channels are those which feed data directly to this output port. 
	While indirect channels are ones that feed data to some other output port 
	which gets captured by a capture channel which directly feeds to this 
	output port. OR they capture data from the concerned output port and feed 
	to some other output port.
	    So, all associated channels feed/use data to/from the output port 
	directly or indirectly.
Returns:
    BERR_SUCCESS on success, else error.
    hChannels[x] will have non-NULL ptrs for channels using mixer.
See Also:
 	None
**************************************************************************/
BERR_Code BRAP_OP_P_GetAssociatedChannels(
    const BRAP_Handle       hRap, 	        /* [in] The RAP Channel handle */
    BRAP_ChannelHandle      *phDirChannels, /* [out] Pointer to the first index 
                                                of an array of direct channel 
                                                handles */
    BRAP_ChannelHandle      *phIndirChannels,/* [out] Pointer to the first index 
                                                of an array of indirect channel 
                                                handles */
	const BRAP_OutputPort   eOutputPort,    /* [in] Output port */                                           
	bool                    bCalledFromISR  /* [in] Flag indicating if this 
	                                           routine has been called from the
	                                           sampling rate change ISR or else
	                                           where */

    )                                       
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int i;
    BRAP_ChannelHandle hCapCh = NULL;
    BRAP_OutputPort eIndirOpPort = BRAP_OutputPort_eMax;
    BRAP_OutputPort eTempOpPort = BRAP_OutputPort_eMax;
        
    BDBG_ENTER(BRAP_OP_P_GetAssociatedChannels);
    BDBG_ASSERT(phDirChannels);
    BDBG_ASSERT(phIndirChannels);
	BSTD_UNUSED(bCalledFromISR);

    /* Initialize phDirChannels and phIndirChannels to NULL */
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        phDirChannels[i] = NULL;
        phIndirChannels[i] = NULL;
    }

    /* 1. Get the associated direct channels */

#if 0   
/* Cant use GEtMixer because
1. it wont correctly handle Mai/Flex
2. It doesnt handle cloned ports. */

    /* Get the associated mixer */
    if(bCalledFromISR)
        ret = BRAP_RM_P_GetMixerForOpPort_isr(hRap->hRm, eOutputPort, &uiMixerId);
    else
        ret = BRAP_RM_P_GetMixerForOpPort(hRap->hRm, eOutputPort, &uiMixerId);
    
if (eOutputPort == BRAP_OutputPort_eMai)
{
    if(bCalledFromISR)
        ret = BRAP_RM_P_GetMixerForOpPort_isr(hRap->hRm, hRap->hFmm[0]->hOp[eOutputPort]->uOpSettings.sMai.sExtSettings.eMaiMuxSelector, &uiMixerId);
    else
        ret = BRAP_RM_P_GetMixerForOpPort(hRap->hRm, hRap->hFmm[0]->hOp[eOutputPort]->uOpSettings.sMai.sExtSettings.eMaiMuxSelector, &uiMixerId);



}
    
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get Mixer ID"));
        return BERR_TRACE (ret);
    }
	BDBG_MSG(("BRAP_OP_P_GetAssociatedChannels: opPort = %d uiMixerId = %d",eOutputPort, uiMixerId));

    /* Get channel handles using one of the 3 inputs of this mixer */
    ret = BRAP_P_GetChannelsUsingMixer(hRap, uiMixerId, phDirChannels);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get direct channel handles"));
        return BERR_TRACE (ret);
    }

#else
    /* Get channel handles using this output port. there can be upto 3 channels coz a mixer can take upto 3 inputs */
    ret = BRAP_P_GetChannelsUsingOpPort(hRap, eOutputPort, phDirChannels);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get direct channel handles"));
        return BERR_TRACE (ret);
    }
#endif
    /* 2. Get the associated indirect channels, if any*/
    /* Check if any capture channel is involved in the direct path */
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        if((phDirChannels[i] != NULL) && 
           (phDirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture))
        {
            /* Save the direct cap channel in a temp var */
            hCapCh = phDirChannels[i];
            /* There can be just one capture channel associated directly/indirectly */
            break;
        }
    }

    /* Found a direct capture channel, get the output port this capture port is capturing */
    if(hCapCh)
    {
        ret = BRAP_OP_P_GetOpPortMatchingCapInputType(hCapCh->eCapInputType, &eIndirOpPort);
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get indirect output port"));
            return BERR_TRACE (ret);
        }
    }
    else
    {
        /* Check if any capture channel is capturing this output port, thereby 
           creating an indirect path */
        for(i=0; i<BRAP_RM_P_MAX_CAP_CHANNELS; i++)
        {
            if(hRap->hRapCapCh[i] != NULL)
            {
                ret = BRAP_OP_P_GetOpPortMatchingCapInputType(hRap->hRapCapCh[i]->eCapInputType, &eTempOpPort);
                if(ret != BERR_SUCCESS)
                {
                    BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get indirect output port"));
                    return BERR_TRACE (ret);
                }
                else
                {
                    if(eOutputPort == eTempOpPort)
                    {
                        hCapCh = hRap->hRapCapCh[i];
			   /* capture only mode does not have any output port associated with it */
			   if ( hRap->hRapCapCh[i]->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR] != NULL )						
                        	eIndirOpPort = hRap->hRapCapCh[i]->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;
                    }
                }
                
                /* There can be just one capture channel associated directly/indirectly */
                break;
                
            } /* if */
        }/* for */
    }

    /* In the above steps, we might have found an associated cap channel and an indirect output port */
    if(hCapCh && eIndirOpPort != BRAP_OutputPort_eMax)
    {
        /* This capture channel will lead to the indirect channels */
        ret = BRAP_P_GetChannelsUsingOpPort(hRap, eIndirOpPort, phIndirChannels);
        if(ret != BERR_SUCCESS)
        {
            BDBG_ERR(("BRAP_OP_P_GetAssociatedChannels: Can't get indirect channel handles"));
            return BERR_TRACE (ret);
        }
    }

    /* for debug */

    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        if(phDirChannels[i] != NULL)
        {
            BDBG_MSG(("DirChan[%d] chanType = %d",i, phDirChannels[i]->eChannelType));
	    }
    }
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++)
    {
        if(phIndirChannels[i] != NULL)
        {
            BDBG_MSG(("IndirChan[%d] chanType = %d",i, phIndirChannels[i]->eChannelType));
        }
    }
    BDBG_LEAVE(BRAP_OP_P_GetAssociatedChannels);
    return ret;
}


/***************************************************************************
Summary:
	Sets the sampling rate associated with the output port.
Description:
	This API is used to set the sampling rate associated with a particular 
	output port. Note: This should not be called with an intention to change 
	output sampling rate for an output which gets data from a decode channel. 
	Decode channel decides on its own the output sampling rate.
Returns:
	BERR_SUCCESS on success, else error
See Also:
	BRAP_OP_GetOutputSamplingRate
****************************************************************************/
BERR_Code BRAP_OP_SetSamplingRate ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
	BAVC_AudioSamplingRate  eSamplingRate 	/* [in] Output Sampling rate */
	)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_OP_SetSamplingRate);	
	
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRap);

	BDBG_MSG(("\nBRAP_OP_SetSamplingRate: hRap = %p, "
		"\n\t eOutputPort = %d, eSamplingRate = %d",
		hRap, eOutputPort, eSamplingRate));

    ret = BRAP_OP_P_SetSamplingRate(hRap, 
                                    eOutputPort, 
                                    eSamplingRate, 
                                    false, /* bOverride */
                                    false /* bCalledFromISR */);

	BDBG_LEAVE(BRAP_OP_SetSamplingRate);	
	return ret;
}

/***************************************************************************
Summary:
	Private function that sets the sampling rate associated with the output 
	port.
Description:
	This private function is used to set the sampling rate associated with a 
	particular output port. 

	The associated channels that are effected due to SR change in an output 
	port can be classified/termed as: 
    	a)Direct Channels(D): Those which feed data directly to an output 
    	  port for which this routine has been called.
	    b)Indirect channels(I): are ones that feed data to some other output 
	      port which gets captured by a capture channel which directly feeds 
	      to this output port. OR they capture data from the concerned output 
	      port and feed to some other output port.
	So, all associated channels feed/use data to/from the output port directly 
	or indirectly.

	Following are the various cases (valid/invalid):
	(Note: 1. The number of same type of channels in Dir/Indir depicted below 
	          is just an example. The actual number of channels may vary. 
	       2. In the example cases below only I2S0 and I2S1 are used. They can
	          be different output ports.
	       3. Causal channel(C) is one that caused/requested a sampling rate 
	          change for the output port.
    -------------------------------------------------------------------------
	SN - Chan(D/I/C)- Output Port - SRC - Change Op SR - Change In SR -
    -------------------------------------------------------------------------
    Case 1: 3 playback channels(PB1/2/3) going out on I2S1. I2S1 captured by 
    ------  CAP and mixed with DEC and PB4 going out on I2S0.
            
    1(a) PB1(D)(C)  - I2S1        -  X  -     I2S1     -              -  
         PB2(D)     - I2S1        -  X  -              -              - 
         PB3(D)     - I2S1        -  X  -              -              -  
         PB4(I)     - I2S0        -     -              -              -  
         DEC(I)     - I2S0        -     -              -              -  
         CAP(I)     - I2S0        -  X  -              -      X       -  

         Same if causal channel is PB2 or PB3.

    1(b) PB1(I)     - I2S1        -     -              -              -              
         PB2(I)     - I2S1        -     -              -              -    
         PB3(I)     - I2S1        -     -              -              -    
         PB4(D)     - I2S0        -  X  -     I2S0     -              -    
         DEC(D)(C)  - I2S0        -     -              -              -    
         CAP(D)     - I2S0        -  X  -              -              -    

    1(c) PB1(I)     - I2S1        -     -              -              -    
         PB2(I)     - I2S1        -     -              -              -    
         PB3(I)     - I2S1        -     -              -              -    
         PB4(D)(C)  - I2S0        -     -              -              -    
         DEC(D)     - I2S0        -     -              -              -    
         CAP(D)     - I2S0        -     -              -              -    

         INVALID case as DEC is directly associated to causal chan (PB4). 
         Same if causal channel is CAP.

    Case 2: 2 playback channels(PB1/2) mixed with a DEC and going out on I2S1. 
    ------  I2S1 captured by CAP and mixed with PB3 going out on I2S0.
            
    2(a) DEC(D)(C)  - I2S1        -     -     I2S1     -              -  
         PB1(D)     - I2S1        -  X  -              -              - 
         PB2(D)     - I2S1        -  X  -              -              -  
         CAP(I)     - I2S0        -     -     I2S0     -      X       -  
         PB3(I)     - I2S0        -  X  -              -              -  

    2(b) DEC(D)     - I2S1        -     -              -              -  
         PB1(D)(C)  - I2S1        -     -              -              - 
         PB2(D)     - I2S1        -     -              -              -  
         CAP(I)     - I2S0        -     -              -              -  
         PB3(I)     - I2S0        -     -              -              -  

        INVALID case as DEC is directly associated to causal chan (PB1). 
        Same if causal channel is PB2.

    2(c) DEC(I)     - I2S1        -     -              -              -  
         PB1(I)     - I2S1        -     -              -              - 
         PB2(I)     - I2S1        -     -              -              -  
         CAP(D)     - I2S0        -     -              -              -  
         PB3(D)(C)  - I2S0        -     -              -              -  

        INVALID case as indirectly DEC channel gets associated to I2S0 through 
        CAP channel. Same if causal channel is CAP.

    Case 3: 3 playback channels(PB1/2/3) going out on I2S1. I2S1 captured 
    ------  by CAP and mixed with PB4 going out on I2S0.
            
    3(a) PB1(D)(C)  - I2S1        -  X  -     I2S1     -              -  
         PB2(D)     - I2S1        -  X  -              -              - 
         PB3(D)     - I2S1        -  X  -              -              -  
         CAP(I)     - I2S0        -  X  -              -      X       -  
         PB4(1)     - I2S0        -     -              -              -  

        Same if causal channel is PB2 or PB3.
        
    3(b) PB1(I)     - I2S1        -     -              -              -  
         PB2(I)     - I2S1        -     -              -              - 
         PB3(I)     - I2S1        -     -              -              -  
         CAP(D)     - I2S0        -  X  -     I2S0     -              -  
         PB4(D)(C)  - I2S0        -  X  -              -              -  

         Same if causal channel is CAP.
    -------------------------------------------------------------------------
	Note: bCalledFromISR is needed because atomic code is very small part of 
	      this big routine and sub-routines called from it. If we have a 
	      XYZ_isr func, and a wrapper around it XYZ with enter and leave
	      critical section, all interrupts will get masked for a considerable
	      time.!!!
Returns:
	BERR_SUCCESS on success for a valid case, else error
See Also:

****************************************************************************/
BERR_Code BRAP_OP_P_SetSamplingRate ( 
	const BRAP_Handle 		hRap, 			/* [in] The RAP Channel handle */
	const BRAP_OutputPort	eOutputPort, 	/* [in] Output port */
	BAVC_AudioSamplingRate  eSamplingRate, 	/* [in] Output Sampling rate */
	bool                    bOverride,      /* [in] Flag indicating if a dec
	                                           channel's output SR be used
	                                           to override the output SR 
	                                           passed to this func, if both
	                                           output to the same output port */
	bool                    bCalledFromISR  /* [in] Flag indicating if this 
	                                           routine has been called from the
	                                           sampling rate change ISR or else
	                                           where */
)
{
	BERR_Code ret = BERR_SUCCESS;
	BRAP_ChannelHandle hRapCh = NULL;
	BRAP_ChannelHandle hDecCh = NULL;
    BRAP_ChannelHandle hDirChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
    BRAP_ChannelHandle hIndirChannels[BRAP_RM_P_MAX_MIXER_INPUTS];
    unsigned int i, j;
    BAVC_AudioSamplingRate  eInputSR;
    bool bCapAnIndirChan = false;
    BRAP_OutputPort eIndirOpPort; 
    BRAP_OP_P_Handle       hOp;    
    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;      

	BDBG_ENTER(BRAP_OP_P_SetSamplingRate);	
	
	/* 1. Validate input parameters. */
	BDBG_ASSERT(hRap);

	BDBG_MSG(("\nBRAP_OP_P_SetSamplingRate: hRap = %p, "
		"\n\t eOutputPort = %d, eSamplingRate = %d"
		"\n\t bOverride = %d, bCalledFromISR = %d",
		hRap, eOutputPort, eSamplingRate, bOverride, bCalledFromISR));

	if(BRAP_RM_P_IsOutputPortSupported(eOutputPort) != BERR_SUCCESS)
	{
		BDBG_ERR(("BRAP_OP_P_SetSamplingRate: eOutputPort(%d) is not supported", eOutputPort));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}

    if (eOutputPort == BRAP_OutputPort_eMai)
    {
		eMaiMuxSelect = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }  


    /* Get channel handles associated directly/undirectly with this output port */
    ret = BRAP_OP_P_GetAssociatedChannels(hRap, &hDirChannels[0], &hIndirChannels[0], 
            eOutputPort, bCalledFromISR);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_SetSamplingRate: Can't get associated channel handles"));
        return BERR_TRACE (ret);
    }

    BDBG_MSG(("Check for invalid cases"));
    /* Check for invalid cases */
    if(bCalledFromISR == false && bOverride==false)
    {
        /* Invalid case: causal chan = PB, direct chan = CAP and indirect chan = DEC */
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hDirChannels[i]; i++)
        {
            if(hDirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture)
            {
                for(j=0; j<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[j]; j++)
                {
                    if(hIndirChannels[j]->eChannelType == BRAP_P_ChannelType_eDecode)
                    {
                        BDBG_ERR(("BRAP_OP_P_SetSamplingRate:"
                            "\n\tInvalid case: causal chan = PB, direct chan = CAP and indirect chan = DEC"));
                        return BERR_TRACE(BERR_NOT_SUPPORTED);
                    }/* if dec */
                }/* for j */
            }/* if cap */
        }/* for i */

        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hDirChannels[i]; i++)
        {
            if(hDirChannels[i]->eChannelType == BRAP_P_ChannelType_eDecode)
            {
                BDBG_ERR(("BRAP_OP_P_SetSamplingRate:"
                    "\n\tInvalid case: causal chan = PB, direct chan = DEC"));
                return BERR_TRACE(BERR_NOT_SUPPORTED);
            }/* if DEC */
        }/* for i */
    }/* if causal!=DEC & !bOverride*/

    /* Over-ride eOutputSR both for PB and CAP with decode channel's eOutputSR */
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hDirChannels[i]; i++)
    {
        if(hDirChannels[i]->eChannelType == BRAP_P_ChannelType_eDecode)
        {
            hDecCh = hDirChannels[i];
            if(hDecCh->sModuleHandles.hDspCh->eSamplingRate != BAVC_AudioSamplingRate_eUnknown &&
                eSamplingRate != hDecCh->sModuleHandles.hDspCh->eSamplingRate)
            {
                /* If this routine has been called internally from a PI, override user's output SR
                   with decode channel's output SR. */
                if(bOverride)
                {
                    BDBG_WRN(("BRAP_OP_P_SetSamplingRate: Overriding passed o/p SR(%d)"
                          "\n by decode channel's o/p SR(%d)", eSamplingRate, 
                           hDecCh->sModuleHandles.hDspCh->eSamplingRate));
            
                    /* Use the output SR from this decode channel */
                    eSamplingRate = hDecCh->sModuleHandles.hDspCh->eSamplingRate;
                }
                else
                {
                    /* If called externally from a user, return error */
                    BDBG_ERR(("BRAP_OP_P_SetSamplingRate: Dec channel is already using the op port(%d)",
                        eOutputPort));
                    return BERR_TRACE (BERR_NOT_SUPPORTED);
                }
            }

            /* There can be only one DEC channel going out on an output port */
            break;
        }
    }
    
    /* Change InputSR of indirect CAP channel, if any */
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[i]; i++)
    {
        if(hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture)
        {
            hRapCh = hIndirChannels[i];
            hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams.eInputSR = eSamplingRate;
        }
    }
    
    /* SRC for PB and CAP direct channels, if any */
    for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hDirChannels[i]; i++)
    {
        if(hDirChannels[i]->eChannelType != BRAP_P_ChannelType_eDecode)
        {
            hRapCh = hDirChannels[i];

            /* Modify the sampling rate available in hOp for this PB/CAP channel. Handle Mai correctly */
            hOp = hRapCh->sModuleHandles.hFmm->hOp[eOutputPort];
            hOp->eSamplingRate = eSamplingRate;
#ifndef BCHP_7411_VER            
            if (eOutputPort == BRAP_OutputPort_eMai)
	    {
		    if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
				hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->eSamplingRate = eSamplingRate;
		    else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
				hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->eSamplingRate = eSamplingRate;		
	     }   
#endif            
            
            /* Both for 7401 and 7411, we can get hSrcCh from sModuleHandles even for the cloned channels */
            /* TODO: for 7400 onwards, hSrcCh will have to be read from sClonedHandles for cloned channels */
            eInputSR = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams.eInputSR;
            
    	    /* Change eSamplingRate for this output port */	
    	    ret = BRAP_OP_P_UpdateSamplingRate(
    											hRapCh, 
    											eOutputPort, /* direct op port */
    											eSamplingRate, 
    											eInputSR,
    											bCalledFromISR);		
    		if(ret != BERR_SUCCESS)
    		{
    			BDBG_ERR(("BRAP_OP_P_SetSamplingRate: BRAP_OP_P_UpdateSamplingRate() returned error (%d)",ret));	
    		}
        }/* if !dec */
    }/* for i */

    /* Do SRC of indirect channels -
       CAP: If cause is not a DEC channel
       PB: If cause is a DEC channel and CAP is one of the indirect channels */
    if(bCalledFromISR)
    {
        for(i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[i]; i++)
        {
            if(hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture)            
                bCapAnIndirChan = true;
        }
    }
       
    for(hRapCh=NULL, i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS && hIndirChannels[i]; i++, hRapCh=NULL)
    {
        if(bCalledFromISR == false)
        {
            if(hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_eCapture)
                hRapCh = hIndirChannels[i];
        }
        else
        {
            if(bCapAnIndirChan && hIndirChannels[i]->eChannelType == BRAP_P_ChannelType_ePcmPlayback)
            {
                hRapCh = hIndirChannels[i];                
                /* Modify the sampling rate available in hOp for this PB channel. Handle Mai correctly */
                hOp = hRapCh->sModuleHandles.hFmm->hOp[eOutputPort];
                hOp->eSamplingRate = eSamplingRate;
#ifndef BCHP_7411_VER                
                if (eOutputPort == BRAP_OutputPort_eMai)
                {
                    if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
                        hOp->hFmm->hOp[BRAP_OutputPort_eSpdif]->eSamplingRate = eSamplingRate;
                    else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
                        hOp->hFmm->hOp[BRAP_OutputPort_eFlex]->eSamplingRate = eSamplingRate;		
                }   
#endif                
            
            }
        }

        if(hRapCh)
        {
            /* Both for 7401 and 7411, we can get hSrcCh from sModuleHandles even for the cloned channels */
            /* TODO: for 7400 onwards, hSrcCh will have to be read from sClonedHandles for cloned channels */
            eInputSR = hRapCh->sModuleHandles.hSrcCh[BRAP_OutputChannelPair_eLR]->sParams.eInputSR;
            eIndirOpPort = hRapCh->sModuleHandles.hOp[BRAP_OutputChannelPair_eLR]->eOutputPort;
            
    		/* Modify the sampling rate available in hOp */
    	    ret = BRAP_OP_P_UpdateSamplingRate(
    											hRapCh, 
    											eIndirOpPort, /* indirect op port */
    											eSamplingRate, 
    											eInputSR,
    											bCalledFromISR);
    		if(ret != BERR_SUCCESS)
    		{
    			BDBG_ERR(("BRAP_OP_P_SetSamplingRate: BRAP_OP_P_UpdateSamplingRate() returned error (%d)",ret));	
    		}
        }/* if hRapCh */  
    }/* for i */

	BDBG_LEAVE(BRAP_OP_P_SetSamplingRate);	
	return ret;
}


/***************************************************************************
Summary:
	Private function that updates sampling rate, program PLL and SRC.
Description:
	Private function that updates sampling rate, program PLL and SRC.
Returns:
	BERR_SUCCESS 
See Also:
	BRAP_OP_SetSamplingRate
****************************************************************************/
BERR_Code 
BRAP_OP_P_UpdateSamplingRate(
		BRAP_ChannelHandle hRapCh,
		BRAP_OutputPort eOutputPort, 
		BAVC_AudioSamplingRate eOutputSR, 
		BAVC_AudioSamplingRate eInputSR, 
		bool bCalledFromISR 
	)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_OP_Pll ePll = BRAP_OP_Pll_eMax;
    bool bValidPll = false;
    unsigned int uiSRIn = 0;
    unsigned int uiSROut = 0;
    bool bClone = false;
    bool bSimulPt = false;
    
    BRAP_OP_P_Handle hOp = NULL;
    BRAP_SPDIFFM_P_Handle hSpdifFm = NULL;
    BRAP_MIXER_P_Handle hMixer = NULL;    
    BRAP_SRCCH_P_Handle hSrcCh = NULL;

    BRAP_OutputPort eMaiMuxSelect = BRAP_OutputPort_eMax;      

    BRAP_OutputChannelPair eChanPair = BRAP_OutputChannelPair_eLR;
#ifndef BCHP_7411_VER  
    int j=0;
#endif
    bool bMute = false;
    bool bUnMute = false;

    BDBG_ENTER(BRAP_OP_P_UpdateSamplingRate);

    /* Validate SR */	
    ret = BRAP_P_ConvertSR(eInputSR, &uiSRIn);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        return ret;
    }
    ret = BRAP_P_ConvertSR(eOutputSR, &uiSROut);
    if(ret != BERR_SUCCESS)
    {
        ret = BERR_TRACE(ret);
        return ret;
    }
    BDBG_ASSERT(uiSRIn);
    BDBG_ASSERT(uiSROut);

    BDBG_MSG(("hRapCh->eChannelType=%d   eOutputPort=%d",hRapCh->eChannelType, eOutputPort));

    BDBG_ASSERT(hRapCh->sModuleHandles.hFmm);

    /* Get the channel pair in RapCh using this output port */
    ret = BRAP_P_GetChannelPairUsingOpPort_isr(hRapCh, eOutputPort, &eChanPair, 
        &bClone, &bSimulPt/* Ignored here */);
    if(ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: Can't get a channel pair using the output port"));
        return BERR_TRACE (ret);
    }
    BDBG_MSG(("eChanPair = %d, bClone=%d, bSimulPt = %d",
        eChanPair, bClone, bSimulPt));
    /* NOTE: this PI gets called only for PB and CAP channels which have 
       only one channel pair !! But we use it to find out whether this 
       port is cloned or not  */   
    if (eChanPair != BRAP_OutputChannelPair_eLR)
    {
        BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: invalid channel pair"));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }    

    if (bSimulPt == true)
    {
        BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: PCM/CAP channels do not have bSimulPt mode"));
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
#ifndef BCHP_7411_VER    
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
#endif
	BDBG_ASSERT(hOp);
	BDBG_ASSERT(hMixer);
	BDBG_ASSERT(hSrcCh);

	/* Mute the output port. For ISR case the output is muted in ISR and not here */
	if(bCalledFromISR == 0)
	{
		ret = BRAP_OP_GetMute(hRapCh->hRap, eOutputPort, &bMute);
		if (ret != BERR_SUCCESS)
			return BERR_TRACE(ret);
		if ( !bMute )
		{
			ret = BRAP_OP_SetMute(hRapCh->hRap, eOutputPort, true);
			if (ret != BERR_SUCCESS)
				return BERR_TRACE(ret);

			bUnMute = true;
		}
	}
    
	switch(hOp->eOutputPort)
	{
		case BRAP_OutputPort_eSpdif:
			ePll = 
    			hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;
            bValidPll = true;
			break;

		case BRAP_OutputPort_eI2s0:
		case BRAP_OutputPort_eI2s1:
		case BRAP_OutputPort_eI2s2:
			ePll = 
			hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eI2s2].uOutputPortSettings.sI2sSettings.ePll;
			bValidPll = true;
			break;

#ifndef BCHP_7411_VER /* For chips other than 7411 */
		case BRAP_OutputPort_eDac0:
		case BRAP_OutputPort_eDac1:
            if(bCalledFromISR)
    			ret = BRAP_OP_P_DacSetSampleRate_isr(hRapCh->hRap, eOutputPort, eOutputSR);	
            else
                ret = BRAP_OP_P_DacSetSampleRate(hRapCh->hRap, eOutputPort, eOutputSR);	
			if(ret != BERR_SUCCESS)
			{
				BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: BRAP_OP_P_DacSetSampleRate() returned error (%d)",ret));	
			}
			bValidPll = false;
			break;
			
		case BRAP_OutputPort_eFlex:
			ePll = 
    			hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eFlex].uOutputPortSettings.sFlexSettings.ePll;
			bValidPll = true;
			break;

		case BRAP_OutputPort_eMai: 
    		    if (eMaiMuxSelect == BRAP_OutputPort_eSpdif)
    				ePll = hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eSpdif].uOutputPortSettings.sSpdifSettings.ePll;
    		    else if (eMaiMuxSelect == BRAP_OutputPort_eFlex)
    				ePll = hOp->hFmm->hRap->sOutputSettings[BRAP_OutputPort_eFlex].uOutputPortSettings.sFlexSettings.ePll;	
	  			
			bValidPll = true;
			break;			
		case BRAP_OutputPort_eRfMod:
                     bValidPll = false;
                     break;
#endif /* ifndef BCHP_7411_VER */

		default:	
			BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: Unknown output port(%d)",hOp->eOutputPort));
			BDBG_ASSERT(0);
	} /* switch */

	if(bValidPll)
	{	        
		if(ret != BERR_SUCCESS)
		{
			BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: BRAP_OP_P_ProgramTimebase() returned error (%d)",ret));	
		}
		else
		{
			/* Program output sampling rate */
            if(bCalledFromISR)
            	ret = BRAP_OP_P_ProgramPll_isr(hOp, ePll, eOutputSR); 
            else
    			ret = BRAP_OP_P_ProgramPll(hOp, ePll, eOutputSR);
			if(ret != BERR_SUCCESS)
			{
				BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: ProgramPll() returned error (%d)",ret));	
			}
    	}
    }

	if((ret == BERR_SUCCESS) && (hSrcCh->eState == BRAP_SRCCH_P_State_eRunning))
	{
		/* Program SRC */
        if(bCalledFromISR)
    	    ret = BRAP_MIXER_P_SetSRC_isr(hMixer, hSrcCh, eOutputSR, eInputSR); 
        else
            ret = BRAP_MIXER_P_SetSRC(hMixer, hSrcCh, eOutputSR, eInputSR);
   		if(ret != BERR_SUCCESS)
		{
    		BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: BRAP_OP_P_UpdateSamplingRate() returned error (%d)",ret));	
	    }
        else
        {
            /* Update Channel status bits for SPDIFFM */
            if((eOutputPort== BRAP_OutputPort_eSpdif) || (eOutputPort== BRAP_OutputPort_eMai))
            {
                BDBG_ASSERT(hSpdifFm);
                if(bCalledFromISR)
                    ret = BRAP_SPDIFFM_P_ProgramChanStatusBits_isr(hSpdifFm);
                else
                    ret = BRAP_SPDIFFM_P_ProgramChanStatusBits(hSpdifFm);
           		if(ret != BERR_SUCCESS)
        		{
        			BDBG_ERR(("BRAP_OP_P_UpdateSamplingRate: ProgramChanStatusBits() returned err(%d)",ret));	
        		}
            }
        }
	}

	/* Unmute the output port muted earlier */
	if( bCalledFromISR == 0 && bUnMute == true )        
	{
		ret = BRAP_OP_SetMute(hRapCh->hRap, eOutputPort, false);
		if (ret != BERR_SUCCESS)
			return BERR_TRACE(ret);
	}
        
	BDBG_ENTER(BRAP_OP_P_UpdateSamplingRate);
	return ret;
}

#ifndef BCHP_7411_VER
/***************************************************************************
Summary:
	Configures the external Master Clocks
Description:
	This API is used to configure the external master clock for the specified pll and
	MClock rate.
Returns:
	BERR_SUCCESS 
See Also:

****************************************************************************/
BERR_Code BRAP_OP_ExtMClkConfig ( 
	const BRAP_Handle 		hRap, 				/* [in] The RAP Channel handle */
	const BRAP_OP_ExtMClk	eMClock,			/* [in] The MClock output */
	BRAP_OP_ExtMClkSettings	*pExtMClkSettings	/* [in] External Master clock settings */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegVal = 0;
	uint32_t  ui32RegAddr = 0;

	BDBG_ENTER (BRAP_OP_ExtMClkConfig);
	BDBG_ASSERT (hRap);
	BDBG_ASSERT (pExtMClkSettings);

	if ( eMClock > BRAP_OP_ExtMClk_eExternal1 )
	{
        	BDBG_ERR (("BRAP_OP_ExtMClkConfig(): Invalid ExtMClock [%d]",
                        eMClock));
		return BERR_TRACE (BERR_INVALID_PARAMETER);            
	}

	ui32RegAddr = BCHP_AUD_FMM_OP_CTRL_MCLK_CFG_EXTi_ARRAY_BASE + ( eMClock * 4 );

	/* Read current value of the register */
	ui32RegVal = BRAP_Read32 (hRap->hRegister, ui32RegAddr);
	ui32RegVal &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, PLLCLKSEL)); 

	switch ( pExtMClkSettings->ePll )
	{
		case BRAP_OP_Pll_ePll0:
			/* Set MCLK rate and PLL Channels */
			switch (pExtMClkSettings->eMClockRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL0_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL0_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL0_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_ExtMClkConfig(): Incorrect MClk Rate %d", 
								pExtMClkSettings->eMClockRate));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
			}     
			break;

		case BRAP_OP_Pll_ePll1:			
			/* Set MCLK rate and PLL Channels */
			switch (pExtMClkSettings->eMClockRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL1_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL1_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL1_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_ExtMClkConfig(): Incorrect MClk Rate %d", 
								pExtMClkSettings->eMClockRate));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
			}     
			break;

		case BRAP_OP_Pll_eHifiDac0:	
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
							PLLCLKSEL, Hifidac0));
			break;
#if ( BCHP_CHIP == 7400 )
		case BRAP_OP_Pll_ePll2:			
			/* Set MCLK rate and PLL Channels */
			switch (pExtMClkSettings->eMClockRate)
			{
				case BRAP_OP_MClkRate_e128Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL2_ch1));
					break;     
				case BRAP_OP_MClkRate_e256Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL2_ch2));
					break;     
				case BRAP_OP_MClkRate_e512Fs:
					ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
											PLLCLKSEL, PLL2_ch3));
					break;     
				default:
					BDBG_ERR (("BRAP_OP_ExtMClkConfig(): Incorrect MClk Rate %d", 
								pExtMClkSettings->eMClockRate));
					return BERR_TRACE (BERR_NOT_SUPPORTED);
			}     
			break;

		case BRAP_OP_Pll_eHifiDac1:	
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_MCLK_CFG_EXTi, 
							PLLCLKSEL, Hifidac1));
			break;
#endif /* defined ( BCHP_CHIP == 7400 ) */
		default:
        		BDBG_ERR (("BRAP_OP_ExtMClkConfig(): Invalid PLL [%d]",
                        			pExtMClkSettings->ePll));
			return BERR_TRACE (BERR_NOT_SUPPORTED); 
	}

	BRAP_Write32 (hRap->hRegister, 
			ui32RegAddr, 
			ui32RegVal);

	/* Store the settings in FMM handle for future use */
	hRap->hFmm[0]->sExtMClkSettings[eMClock].eMClockRate = pExtMClkSettings->eMClockRate;
	hRap->hFmm[0]->sExtMClkSettings[eMClock].ePll = pExtMClkSettings->ePll;

	BDBG_LEAVE (BRAP_OP_ExtMClkConfig);
	return ret;
} 
#endif

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
BERR_Code
BRAP_OP_P_AlignPorts(
        BRAP_Handle     hRap,
        bool            bSpdif,     /* [in] TRUE: Enable alignment for Spdif */
        bool            bI2s0,      /* [in] TRUE: Enable alignment for I2S0 */
        bool            bI2s1,      /* [in] TRUE: Enable alignment for I2S1 */
        bool            bI2s2       /* [in] TRUE: Enable alignment for I2S2 */    
)
{
    uint32_t  ui32StreamCfg = 0;
    uint32_t  ui32TimingInit = 0;
    uint32_t  ui32CtrlEnable = 0;    

    BDBG_ENTER(BRAP_OP_P_AlignPorts);

    /* note: This PI should be called while all the FMM modules are disabled ie before they are started.
    but after these output ports are configured.
    The sequence of register programmign should be 

    - Program AUD_FMM_OP_CTRL_ENABLE_SET : ALIGN_I2S_SAMPLES = 1 & I2S_LRCK_ENA = 1
    - Program WAIT_FOR_VALID bit = Holdoff_request, for the output ports associated.   
    - Program ALIGN_ENA bit = 1, for the output ports associated.
    - be sure to set ALIGN_FS at the same time as (or after) setting the ALIGN_ENA_I2S* bits.
    */

    /* note that AUD_FMM_OP_CTRL_TIMING_INIT &  AUD_FMM_OP_CTRL_ENABLE_SET dont require 
    read-modify-write since writing 1 enables, while writing 0 has no effect */
    ui32StreamCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);

    /* 
    AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID2 = Holdoff_request    (Stream 2 goes to I2S0)
                                WAIT_FOR_VALID3 = Holdoff_request    (Stream 3 goes to I2S1)
                                WAIT_FOR_VALID4 = Holdoff_request    (Stream 4 goes to I2S2) etc
    AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_ENA_I2S0 = 1
                                 ALIGN_ENA_I2S1 = 1
                                 ALIGN_ENA_I2S2 = 1        
    AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_FS = 1 
    AUD_FMM_OP_CTRL_ENABLE_SET : ALIGN_I2S_SAMPLES = 1
                                 I2S_LRCK_ENA = 1                
    */


    if (bSpdif == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID0));
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID0, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_SPDIF, 1));                 
    }
#if ( BCHP_CHIP != 7400 )		
    if (bI2s0 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID2));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID2, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S0, 1));             
    }
    if (bI2s1 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID3));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID3, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S1, 1));                     
    }
    if (bI2s2 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID4));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID4, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S2, 1));               
    }
#else
    if (bI2s0 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID3));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID3, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S0, 1));             
    }
    if (bI2s1 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID7));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID7, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S1, 1));                     
    }
    if (bI2s2 == true)
    {
        ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID4));    
        ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
                                WAIT_FOR_VALID4, Holdoff_request));
        ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                                ALIGN_ENA_I2S2, 1));               
    }
#endif

    if ( (bI2s0 == true) || (bI2s1 == true) || (bI2s2 == true))
    {
        ui32CtrlEnable |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                ALIGN_I2S_SAMPLES, 1));    
        ui32CtrlEnable |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_SET, 
                                I2S_LRCK_ENA, 1));    
    }
    ui32TimingInit |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_TIMING_INIT, 
                            ALIGN_FS, 1));    

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_SET, ui32CtrlEnable);     
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32StreamCfg);        
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_TIMING_INIT, ui32TimingInit);        

    BDBG_LEAVE(BRAP_OP_P_AlignPorts);
    return BERR_SUCCESS;    
}

BERR_Code
BRAP_OP_P_DisableAllignment(
        BRAP_Handle     hRap,
        bool            bSpdif,     /* [in] TRUE: Enable alignment for Spdif */
        bool            bI2s0,      /* [in] TRUE: Enable alignment for I2S0 */
        bool            bI2s1,      /* [in] TRUE: Enable alignment for I2S1 */
        bool            bI2s2       /* [in] TRUE: Enable alignment for I2S2 */    
)
{
	uint32_t  ui32CtrlEnable = 0;    
#if ( BCHP_CHIP == 7400 )			
	uint32_t  ui32StreamCfg = 0;
#endif

	BSTD_UNUSED(bSpdif);

	BDBG_ENTER(BRAP_OP_P_DisableAllignment);

	if ( (bI2s0 == true) || (bI2s1 == true) || (bI2s2 == true))
	{
		ui32CtrlEnable |= (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_ENABLE_CLEAR, 
		                        ALIGN_I2S_SAMPLES, 1));
	}

	BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_ENABLE_CLEAR, ui32CtrlEnable); 

#if ( BCHP_CHIP == 7400 )			
	ui32StreamCfg = BRAP_Read32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG);
	if (bI2s0 == true)
	{
		ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID3));    
		ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID3, Keep_requesting));
	}
	if (bI2s1 == true)
	{
		ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID7));    
		ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID7, Keep_requesting));
	}
	if (bI2s2 == true)
	{
		ui32StreamCfg &= ~(BCHP_MASK (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID4));    
		ui32StreamCfg |= (BCHP_FIELD_ENUM (AUD_FMM_OP_CTRL_STREAM_CFG, 
	                    WAIT_FOR_VALID4, Keep_requesting));
	}

	BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_OP_CTRL_STREAM_CFG, ui32StreamCfg);        
#endif

	BDBG_LEAVE(BRAP_OP_P_DisableAllignment);
	return BERR_SUCCESS;    
}

#endif

#if BCHP_7411_VER > BCHP_VER_C0
BERR_Code
BRAP_OP_P_SetI2sClk (
    BRAP_OP_P_Handle        hOp,             /*[in] Output port handle */
    BRAP_OP_SClkRate	eSClkRate,
    BRAP_OP_MClkRate      eMClkRate)          /* MClk rate */
{
	uint32_t	ui32RegVal = 0, uiTemp = 0;
	
	ui32RegVal = BRAP_Read32( hOp->hRegister, 
				BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + hOp->ui32Offset);

    switch (eSClkRate)
    {
        case BRAP_OP_SClkRate_e64Fs: 
            uiTemp = 64;
            break;
        case BRAP_OP_SClkRate_e128Fs: 
            uiTemp = 128;
            break;
        default:
            BDBG_ERR (( "BRAP_DEC_SetI2sClk(): Incorrect SClk %d", eSClkRate ));
            return BERR_TRACE ( BERR_NOT_SUPPORTED );
    }
    ui32RegVal = (ui32RegVal & (~BCHP_MASK( AUD_FMM_OP_CTRL_I2S_CFG_i, SCLKS_PER_1FS_DIV32 )))
				| (BCHP_FIELD_DATA (AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                    SCLKS_PER_1FS_DIV32, 
                                    (uiTemp/32)));
	
    /* Set MCLK/(2*SCLK) ratio in MCLK_RATE field */
    switch (eMClkRate)
    {
        case BRAP_OP_MClkRate_e128Fs:
           uiTemp = (128 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e256Fs:
           uiTemp = (256 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e384Fs:
           uiTemp = (384 / (2 * uiTemp));
           break; 
        case BRAP_OP_MClkRate_e512Fs:
           uiTemp = (512 / (2 * uiTemp));
           break; 
        default:
            BDBG_ERR (( "BRAP_DEC_SetI2sClk(): Incorrect MClk %d", 
                        eMClkRate ));
            return BERR_TRACE ( BERR_NOT_SUPPORTED );
    }

    ui32RegVal = ui32RegVal & (~BCHP_MASK( AUD_FMM_OP_CTRL_I2S_CFG_i, MCLK_RATE ));
    
    switch (uiTemp)
    {
        case 1:
            ui32RegVal |= (BCHP_FIELD_ENUM ( AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_128fs_SCLK_64fs ));
            break;     
        case 2:
            ui32RegVal |= (BCHP_FIELD_ENUM ( AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_256fs_SCLK_64fs ));
            break;     
        case 3:
            ui32RegVal |= (BCHP_FIELD_ENUM ( AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_384fs_SCLK_64fs ));
            break;     
        case 4:
            ui32RegVal |= (BCHP_FIELD_ENUM ( AUD_FMM_OP_CTRL_I2S_CFG_i, 
                                            MCLK_RATE, MCLK_512fs_SCLK_64fs ));
            break;     
        default:
            BDBG_ERR (("BRAP_OP_P_I2sOpen(): Unsupported MClk/SClk Ratio %d", 
                        uiTemp));
            return BERR_TRACE ( BERR_NOT_SUPPORTED );
    } 

	BRAP_Write32( hOp->hRegister, 
				BCHP_AUD_FMM_OP_CTRL_I2S_CFG_i_ARRAY_BASE + hOp->ui32Offset,
				ui32RegVal );

	return BERR_SUCCESS;
}

#endif /* BCHP_7411_VER > BCHP_VER_C0 */

#ifndef BCHP_7411_VER
BRAP_OutputPort
BRAP_OP_P_GetRfmodMuxSelector(
		BRAP_Handle     hRap    /* [in]Audio Device Handle */
		)
{

	BRAP_OP_P_Handle       hOp = NULL;
	BRAP_OutputPort		eOutputPort = BRAP_OutputPort_eMax;

#if ( BCHP_CHIP == 7400 )
	hOp = hRap->hFmm[0]->hOp[BRAP_OutputPort_eDac0];

	if ( (hOp != NULL)&&
	      ( hOp->eRfmodMuxSelect != BRAP_OutputPort_eMax ))
	{
		eOutputPort = hOp->eRfmodMuxSelect;
	}
	else
	{
		hOp = hRap->hFmm[0]->hOp[BRAP_OutputPort_eDac1];		
		if ( hOp != NULL )
		{
			if ( hOp->eRfmodMuxSelect != BRAP_OutputPort_eMax )
				eOutputPort = hOp->eRfmodMuxSelect;
		}
	}
#else
	BSTD_UNUSED(hRap);
	BSTD_UNUSED(hOp);
	
	/* For 7401 we have only one DAC */
	eOutputPort = BRAP_OutputPort_eDac0;
#endif

	BDBG_MSG(("BRAP_OP_P_GetRfmodMuxSelector(): Rfmod Mux Selector[%d]", eOutputPort));
	return eOutputPort;
}
#endif

#if ( BCHP_CHIP == 7400 )
BERR_Code
BRAP_OP_SetRfmodSource(
		BRAP_Handle hRap,
		BRAP_OutputPort eRfmodSource )
{
	BERR_Code     eRet = BERR_SUCCESS;
	BRAP_OP_P_Handle	hOp=NULL;
	uint32_t ui32RegVal=0;
	uint32_t ui32RegAddr=0;

	
	BDBG_ENTER (BRAP_OP_SetRfmodSource);
	BDBG_ASSERT (hRap);

	switch ( eRfmodSource )
	{
		case BRAP_OutputPort_eDac0:
		case BRAP_OutputPort_eDac1:
			break;
		default:
			BDBG_MSG(("BRAP_OP_SetRfmodSource(): Invalid source[%d]", eRfmodSource));
			return BERR_TRACE (BERR_INVALID_PARAMETER);            
	}

	hOp = hRap->hFmm[0]->hOp[eRfmodSource];
	if ( hOp == NULL )
	{
		BDBG_MSG(("BRAP_OP_SetRfmodSource(): Failed as the specified source [%d] is not opened yet", eRfmodSource));
		return BERR_TRACE (BERR_INVALID_PARAMETER);            
	}

	ui32RegAddr = BCHP_AUD_FMM_MISC_HIFIOUT_SEL;
	ui32RegVal = BRAP_Read32_isr(hRap->hRegister, ui32RegAddr);

#if ( BCHP_VER > BCHP_VER_C0)
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD0_SEL));
	switch ( eRfmodSource )
	{
		case BRAP_OutputPort_eDac0:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_HIFIOUT_SEL, 
						RFMOD0_SEL, Hifidac0 ));    
			break;
		case BRAP_OutputPort_eDac1:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_HIFIOUT_SEL, 
						RFMOD0_SEL, Hifidac1));    
			break;
		default:
			BDBG_MSG(("BRAP_OP_SetRfmodSource(): Invalid source[%d]", eRfmodSource));
			return BERR_TRACE (BERR_INVALID_PARAMETER);            
	}
#else
	ui32RegVal &= ~(BCHP_MASK(AUD_FMM_MISC_HIFIOUT_SEL, RFMOD_SEL));
	switch ( eRfmodSource )
	{
		case BRAP_OutputPort_eDac0:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_HIFIOUT_SEL, 
						RFMOD_SEL, Hifidac0 ));    
			break;
		case BRAP_OutputPort_eDac1:
			ui32RegVal |= (BCHP_FIELD_ENUM (AUD_FMM_MISC_HIFIOUT_SEL, 
						RFMOD_SEL, Hifidac1));    
			break;
		default:
			BDBG_MSG(("BRAP_OP_SetRfmodSource(): Invalid source[%d]", eRfmodSource));
			return BERR_TRACE (BERR_INVALID_PARAMETER);            
	}
#endif	

	BRAP_Write32_isr (hRap->hRegister, ui32RegAddr, ui32RegVal);

	/* Update the Rfmmod source to output handle for future use */
	hOp->eRfmodMuxSelect = eRfmodSource;
	BDBG_MSG(("BRAP_OP_SetRfmodSource(): Rmod source stored in handle[%d]", hOp->eRfmodMuxSelect));

	/* Now we have to clear the source stored in other DAC handle */
	hOp = hRap->hFmm[0]->hOp[eRfmodSource];

	if ( hOp != NULL )
	{
		hOp->eRfmodMuxSelect = BRAP_OutputPort_eMax;
	}

	BDBG_LEAVE (BRAP_OP_SetRfmodSource);
	return eRet;
}
#endif

#if (BRAP_7401_FAMILY == 1)
/***************************************************************************
Summary:
	Programs the PLL dividers.
	
Description:
	This API is used to program the PLL dividers with the correct values to get the 
	clock frequency mentioned in ulClkFreq. 
	
Returns:
	BERR_SUCCESS on success, else error
	
See Also:
****************************************************************************/
BERR_Code 
BRAP_OP_ProgramPll (
	BRAP_Handle	hRap,		/* [in]Audio Device Handle */
	BRAP_OP_Pll	ePll,		/* [in]PLL device to be programmed */
	unsigned long	ulClkFreq	/* The actual clock frequency. Currently we support only
							24000000 and 36864000 */
)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t  ui32RegValue = 0;
	uint32_t  ui32Offset=0;

	BDBG_ENTER (BRAP_OP_ProgramPll);

	if ( ePll > BRAP_OP_Pll_ePll2 )
	{
		BDBG_ERR (("BRAP_OP_ProgramPll(): Invalid PLL value %d", ePll));
        	return BERR_TRACE (BERR_INVALID_PARAMETER);            		
	}

	BDBG_MSG (("BRAP_OP_ProgramPll(): handle=0x%08x ePll=%d ulClkFreq=%d",
	        hRap, ePll, ulClkFreq));

	/* Calculate the offset */
	ui32Offset = (int)(ePll) * (BCHP_AUD_FMM_PLL1_MACRO - BCHP_AUD_FMM_PLL0_MACRO);    

	/* Program the MACRO_SELECT = User */
	ui32RegValue = BRAP_Read32_isr (hRap->hRegister, 
									BCHP_AUD_FMM_PLL0_MACRO + ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT));
	ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, User));
	BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_MACRO + ui32Offset , ui32RegValue);

	if ( ulClkFreq == 24000000 )
	{
		/*Set PLL Divider Parameters to generate 24.000 MHZ */
		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_P1_PLL1_SHIFT)|
					(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_N2_PLL1_SHIFT)|
					(0x00000010 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_N1_PLL1_SHIFT)|
					(0x0000001e << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_M1_PLL1_SHIFT); 	
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV1+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_P1_PLL2_SHIFT)|
					(0x00000003 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_N2_PLL2_SHIFT)|
					(0x00000019 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_N1_PLL2_SHIFT); 
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV2+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000005 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M2_CH2_SHIFT)|
					(0x00000009 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M1_CH2_SHIFT)|
					(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M2_CH3_SHIFT)|
					(0x00000019 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M1_CH3_SHIFT); 	
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV3+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV4_USER_VCORNG_PLL1_SHIFT)|
					(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV4_USER_VCORNG_PLL2_SHIFT ) ; 
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV4+ui32Offset, ui32RegValue);
	}
	else if ( ulClkFreq == 36864000 )
	{
		/*Set PLL Divider Parameters to generate 36.864 MHZ */	
		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_P1_PLL1_SHIFT) |
				(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_N2_PLL1_SHIFT) |
				(0x00000020 << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_N1_PLL1_SHIFT) |
				(0x0000000f << BCHP_AUD_FMM_PLL0_USER_DIV1_USER_M1_PLL1_SHIFT) ; 	
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV1+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000003 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_P1_PLL2_SHIFT) |
				(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_N2_PLL2_SHIFT) |
				(0x00000030 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_N1_PLL2_SHIFT) |
				(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_M2_CH1_SHIFT) |
				(0x00000019 << BCHP_AUD_FMM_PLL0_USER_DIV2_USER_M1_CH1_SHIFT ) ; 
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV2+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M2_CH2_SHIFT) |
				(0x00000019 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M1_CH2_SHIFT) |
				(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M2_CH3_SHIFT) |
				(0x00000019 << BCHP_AUD_FMM_PLL0_USER_DIV3_USER_M1_CH3_SHIFT); 	
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV3+ui32Offset, ui32RegValue);

		ui32RegValue = (0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV4_USER_VCORNG_PLL1_SHIFT) |
				(0x00000001 << BCHP_AUD_FMM_PLL0_USER_DIV4_USER_VCORNG_PLL2_SHIFT ) ; 
		BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_USER_DIV4+ui32Offset, ui32RegValue);
	
	}
	else
	{
		BDBG_ERR (("BRAP_OP_ProgramPll(): Invalid ulClock Frequency value %d", ulClkFreq));
        	return BERR_TRACE (BERR_INVALID_PARAMETER);            		
	}

	/* Enable PLL parameter update */
	ui32RegValue = BRAP_Read32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL+ui32Offset);
	ui32RegValue &= ~(BCHP_MASK(AUD_FMM_PLL0_CONTROL, USER_UPDATE_DIVIDERS));
	ui32RegValue |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, USER_UPDATE_DIVIDERS, Update_now));
	BRAP_Write32_isr (hRap->hRegister, BCHP_AUD_FMM_PLL0_CONTROL+ui32Offset,  ui32RegValue);

	BDBG_LEAVE (BRAP_OP_ProgramPll);
	return ret;
}
#endif

#if 0 /*TBD later*/
/* TODO: After calling the following function, remeber to enable 
         Flex port and its inputs by calling BRAP_OP_EnableOutput 
         function 
*/
BERR_Code
BRAP_OP_P_FlexStart(BRAP_OP_P_Handle    hOp,
                    BRAP_OP_FlexSettings *pSettings)
{
    BRAP_OP_P_FlexSetting flex;
    BERR_Code ret=BERR_SUCCESS;

    uint32_t ui32RegVal = 0;

    /* Flex related parameters to be programmed in AUD_FMM_OP_CTRL_STREAM_CFG  */
    bool         bFlexUseSpdif;    /* This bit tells whether the stream selected for flex has 
                                  SPDIF formatting or normal PCM formatting like I2S uses.*/
    bool         bDac0DriveReq;    /* If true, Dac 0  will control the pull rate */
    bool         bDac1DriveReq;    /* If true, Dac 1  will control the pull rate */
    uint8_t     ui8FlexSelect;    /* It will be set based uppn BRAP_Flex_Mai_Input */

    /* AUD_FMM_OP_CTRL_FLEX_CFG1 parameters to be programmed */
    bool         bSlaveStream;
    bool         bIgnoreValid;
    bool         bRequestLow;
    bool         bIgnoreReqToSend;
    bool         bLSBFirst;
    bool         bSyncLow;
    bool         bSyncFormat;
    bool         bSyncPosition;
    bool         bSynclength;
    bool         bSclkConGap;
    bool         bSclkPhase;
    uint8_t     ui8WordLength;
    
    /* AUD_FMM_OP_CTRL_FLEX_CFG2 parameters to be programmed */
    bool         bClkSelect; /* Master Clock or System Clock */
    uint8_t     ui8Clkrate;
    bool         bUseStreamByteCnt;
    uint8_t     ui8BytesPerFmmReq;
    uint8_t     ui8BitsPerSample;
    bool         bLsb;

    /* AUD_FMM_OP_CTRL_FLEX_BYTE_ORDER parameters to be programmed */
    uint32_t         ui32ByteOrder;

    /* AUD_FMM_OP_CTRL_FLEX_THROTTLE parameters to be programmed */
    uint32_t         ui32Throttle;
    

    switch(pSettings->eFlexInput)
    {
        case BRAP_Flex_Mai_Input_eSpdif:
            ui8FlexSelect=AUD_FMM_OP_CTRL_STREAM_CFG_SPDIF;
            bDac0DriveReq=1; 
            bDac1DriveReq=1; 
            bFlexUseSpdif=1;
            break;
        case BRAP_Flex_Mai_Input_eDac0:
            ui8FlexSelect=AUD_FMM_OP_CTRL_STREAM_CFG_DAC0;
            bDac0DriveReq=pSettings->bSlaveStream;
            bDac1DriveReq=1; 
            bFlexUseSpdif=1;
            break;
        case BRAP_Flex_Mai_Input_eDac1:
            ui8FlexSelect=AUD_FMM_OP_CTRL_STREAM_CFG_DAC1;
            bDac0DriveReq=1;
            bDac1DriveReq=pSettings->bSlaveStream;
            bFlexUseSpdif=1;
            break;
        default:
            ret=BERR_FAILURE;
            return ret;
            
    }
    
    /* Frame the register field values based upon the Flex type */

    bSlaveStream=pSettings->bSlaveStream;
    bIgnoreValid=1;
    bRequestLow=1;
    bIgnoreReqToSend=1;
    ui8WordLength=2;
    bLSBFirst=0;
    bSyncLow=1;
    bSyncFormat=1;
    bSyncPosition=1;
    bSynclength=1;
    bSclkConGap=1;
    bSclkPhase=0;

    bClkSelect=1;
    ui8Clkrate=2;
    bUseStreamByteCnt=pSettings->bUseStreamByteCnt;
    ui8BytesPerFmmReq=7;
    ui8BitsPerSample=24;
       bLsb=0;
    ui32ByteOrder=0x32107654;
    ui32Throttle=2200;
      
    switch(pSettings->eFlexFormat)
    {
        case BRAP_Flex_Format_eI2S:
            bSyncLow=1;
            bSyncPosition=1;
            ui32Throttle=1;
            break;
        case BRAP_Flex_Format_eI2S_INVCLK:
            bSyncLow=1;
            bSyncPosition=1;
            bSclkPhase=1;
            ui32Throttle=1;
            break;

        case BRAP_Flex_Format_eALIGNED:
            bSyncLow=0;
            bSyncPosition=0;
            break;

        case BRAP_Flex_Format_e12BIT_LSB:
            ui8WordLength=1;
            bSyncLow=0;
            bSyncPosition=0;
            ui8Clkrate=4;
            ui8BytesPerFmmReq=3;
            ui8BitsPerSample=12;
            bLsb=1;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;
                
        case BRAP_Flex_Format_e16BIT_LSB:
            bSyncLow=0;
            bSyncPosition=0;
            ui8BitsPerSample=16;
            bLsb=1;
            break;
                
        case BRAP_Flex_Format_e32BIT_LSB:
            bSyncLow=0;
            bSyncPosition=0;
            ui8BitsPerSample=0;
            bLsb=1;
            break;
                
        case  BRAP_Flex_Format_e24BIT_LSB:
            bSyncLow=0;
            bSyncPosition=0;
            ui8BitsPerSample=24;
            bLsb=1;
            break;

        case BRAP_Flex_Format_e24BIT_LSB_DLY:
            bSyncLow=0;
            bSyncPosition=1;
            ui8BitsPerSample=24;
            bLsb=1;
            break;
                
        case  BRAP_Flex_Format_eSYNC1_8BIT:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=0;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bClkSelect=0;
            ui8BytesPerFmmReq=5;
            ui32ByteOrder=0x32176500;
            ui32Throttle=1000;
            break;
                
        case  BRAP_Flex_Format_eSYNC1_16BIT:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=1;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bClkSelect=0;
            ui8BytesPerFmmReq=3;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;
            
        case BRAP_Flex_Format_eSYNC1_32BIT_LSB:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bClkSelect=0;
            ui8BitsPerSample=0;
            bLsb=1;
            ui32ByteOrder=0x32107654;
            ui32Throttle=1000;
            break;
                
        case BRAP_Flex_Format_eSYNC1_16BIT_LSBFIRST:
            bIgnoreValid=0;
            bRequestLow=0;
            bIgnoreReqToSend=0;
            ui8WordLength=1;
            bLSBFirst=1;
            bSyncLow=1;
            bSyncFormat=0;
            bSyncPosition=1;
            bSynclength=0;
            bSclkConGap=0;
            bSclkPhase=1;
            bClkSelect=0;
            ui8Clkrate=4;
            ui8BytesPerFmmReq=3;
            ui8BitsPerSample=16;
            ui32ByteOrder=0x23670000;
            ui32Throttle=10;
    
         case BRAP_Flex_Format_eSYNC1_32BIT_LSBFIRST:
             bLSBFirst=1;
            bSyncFormat=0;
            bSyncPosition=1;
            bSynclength=0;
            bSclkConGap=0;
            bClkSelect=0;
            ui8Clkrate=4;
            ui8BytesPerFmmReq=7;
            ui8BitsPerSample=0;
            ui32ByteOrder=0x01234567;
            ui32Throttle=1000;
            break;
            
        case BRAP_Flex_Format_eSYNCALL_32BIT:
            bIgnoreValid=0;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=1;
            bClkSelect=0;
            ui8Clkrate=7;    
            ui8BytesPerFmmReq=7;
            ui8BitsPerSample=0;
            bLsb=0;
            ui32ByteOrder=0x32107654;
            ui32Throttle=2000;
            break;

        case BRAP_Flex_Format_eCOMPRESSED_8BIT_GAPPED:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=0;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bSclkConGap=0;
            bClkSelect=0;    
            ui8BytesPerFmmReq=3;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;

        case BRAP_Flex_Format_eCOMPRESSED_8BIT_CONTINOUS:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=0;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bSclkConGap=1;
            bClkSelect=0;    
            ui8BytesPerFmmReq=3;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;
                
        case BRAP_Flex_Format_eCOMPRESSED_16BIT_GAPPED:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=1;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bSclkConGap=0;
            bClkSelect=0;    
            ui8BytesPerFmmReq=3;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;

        
        case BRAP_Flex_Format_eCOMPRESSED_16BIT_CONTINOUS:
            bIgnoreValid=0;
            bIgnoreReqToSend=0;
            ui8WordLength=1;
            bSyncLow=0;
            bSyncFormat=0;
            bSyncPosition=0;
            bSynclength=0;
            bSclkConGap=1;
            bClkSelect=0;    
            ui8BytesPerFmmReq=3;
            ui32ByteOrder=0x32760000;
            ui32Throttle=1000;
            break;
            
        default:
            /* TODO: Print error message and return error */
                
    }

    if(bSlaveStream == true)
        ui32RegVal |= BCHP_FIELD_ENUM (    AUD_FMM_OP_CTRL_FLEX_CFG1, 
                                        SLAVE_STREAM, Slave);
    else
        ui32RegVal |= BCHP_FIELD_ENUM (    AUD_FMM_OP_CTRL_FLEX_CFG1, 
                                        SLAVE_STREAM, Dont_Slave);
        
    if(bIgnoreValid == true)
        ui32RegVal |= BCHP_FIELD_ENUM (    AUD_FMM_OP_CTRL_FLEX_CFG1, 
                                        IGNORE_VALID, Ignore);
    else
        ui32RegVal |= BCHP_FIELD_ENUM (    AUD_FMM_OP_CTRL_FLEX_CFG1, 
                                        IGNORE_VALID, Dont_Ignore);

    /* TODO:
       Similarly program other fields of AUD_FMM_OP_CTRL_FLEX_CFG1, 
       AUD_FMM_OP_CTRL_FLEX_CFG2, AUD_FMM_OP_CTRL_FLEX_BYTE_ORDER, 
       AUD_FMM_OP_CTRL_FLEX_THROTTLE & Flex related parameters of 
       AUD_FMM_OP_CTRL_STREAM_CFG registers */

    /* TODO : Slect Flex PLL Clock by programming following register field.
        
       AUD_FMM_OP_CTRL_MCLK_CFG_FLEX :    PLLCLKSEL = pSettings->ePll; */

    /* TODO: Program the following to enable Flex 
    
    AUD_FMM_OP_CTRL_FLEX_CFG2  :    CLOCK_ENABLE = Enable
                                    DATA_ENABLE = Enable 
    AUD_FMM_OP_CTRL_FLEX_CFG1 : ENABLE = Enable
    AUD_FMM_MISC_SEROUT_OE  : LRCK3_OE = SCLK3_OE = SDAT3_OE = Drive
    
    */

    /* Store the settings */
    hOp->sFlexSettings = *pSettings;

    /* Program the Timebase */
    ret = BRAP_OP_P_ProgramTimebase (hOp, pParams->ePll, pParams->eTimebase);
    

    /* Enable Flex upstream request */
    ret = BRAP_OP_P_EnableStream(hAUD, BRAP_OutputPort_eFlex, true);

}


BERR_Code
BRAP_OP_P_FlexStop(BRAP_OP_P_Handle    hOp,        /*[in] O/P handle */
                    )
{
    /* TODO: Program the follwoing registers in order to disable Flex
    
    AUD_FMM_OP_CTRL_FLEX_CFG2  :    CLOCK_ENABLE = Disable
                                    DATA_ENABLE = Disable 
    AUD_FMM_OP_CTRL_FLEX_CFG1 : ENABLE = Disable
    AUD_FMM_MISC_SEROUT_OE  : LRCK3_OE = SCLK3_OE = SDAT3_OE = Drive
    */
    
    /* Disable Flex upstream request */
    ret = BRAP_OP_P_EnableStream(hAUD, BRAP_OutputPort_eFlex, false);
}

/* TODO: For 8-channel audio, this function has to be called after I2S alignment*/
/* This function does alignment of the Flex port with the existing I2S aligned ports */

BERR_Code
BRAP_OP_P_AlignFlex(
                BRAP_OP_P_Handle    hOp,
                bool bAlign
                )
{
    
    /* Disable the Flex outputs */
    ret = BRAP_OP_P_EnableStream(hAUD, BRAP_OutputPort_eFlex, false);

    if(bAlign == true)
    {
        /* Do Flex alignment aith I2S streams */

        /* TODO:Program the following.

        AUD_FMM_OP_CTRL_ENABLE_SET : ALIGN_FLEX_SAMPLES = 1
                                     FLEX_LRCK_ENA = 1

        switch(hAUD->sFlexCfg.eFlexInput)
        {
            case BRAP_Flex_Mai_Input_eSpdif:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID0 = Holdoff_request
                break;
            case BRAP_Flex_Mai_Input_eDac0:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID1 = Holdoff_request
                break;
            case BRAP_Flex_Mai_Input_eDac1:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID5 = Holdoff_request
                break;
        }
        
                                         
        AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_ENA_FLEX = 1

        AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_FS = 1 

        */
        
    }
    else
    {
        /* TODO:Program the following.

        AUD_FMM_OP_CTRL_ENABLE_CLEAR : ALIGN_FLEX_SAMPLES = 1
                                       FLEX_LRCK_ENA = 1

        switch(hAUD->sFlexCfg.eFlexInput)
        {
            case BRAP_Flex_Mai_Input_eSPDIF:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID0 = Keep_requesting
                break;
            case BRAP_Flex_Mai_Input_eDac0:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID1 = Keep_requesting
                break;
            case BRAP_Flex_Mai_Input_eDac1:
                AUD_FMM_OP_CTRL_STREAM_CFG: WAIT_FOR_VALID5 = Keep_requesting
                break;
        }

        AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_ENA_FLEX = 0

        AUD_FMM_OP_CTRL_TIMING_INIT: ALIGN_FS = 0 

        */
    }

    /* Enable the I2S outputs */
    ret = BRAP_OP_P_EnableStream(hAUD, eFlex, true);


}


BERR_Code
BRAP_OP_P_MaiStart(
                    BRAP_OP_P_Handle    hOp,
                    BRAP_OP_MaiSettings *pSettings
                    )
{
    /* TODO: Program following registers as given below.

       AUD_FMM_OP_CTRL_STREAM_CFG : MAI_SEL = 0, if pSettings->eMaiInput == BRAP_Flex_Mai_Input_eSPDIF
                                              1, if pSettings->eMaiInput == BRAP_Flex_Mai_Input_eDAC0

       AUD_FMM_OP_CTRL_MAI_FORMAT  : STREAM_ID = 0
                                     MAI_VERSION = 0
                                     SAMPLE_WIDTH = pSettings->eMAISampleWidth
                                     AUDIO_FORMAT = pSettings->eMAIAudioFormat
                                     SAMPLE_RATE = pSettings->eMAISamplingRate
        AUD_FMM_OP_CTRL_MAI_CFG : LR_SELECT = pSettings->eChannelOrder
        AUD_FMM_OP_CTRL_MAI_CFG : SPDIF_MODE = pSettings->bSpdifPcmInput

        Program the following to enable MAI

        AUD_FMM_OP_CTRL_MAI_CFG  :    ENABLE_MAI = Disable
    */

    /* Store the settings */
    hOp->sMaiSettings = *pSettings;

    /* Separate sampling rate is not needed for MAI, 
       as it slaves SPDIF or DAC always */

    /* Enable MAI upstream request */
    ret = BRAP_OP_P_EnableStream(hAUD, BRAP_OutputPort_eMai, true);

}


BERR_Code
BRAP_OP_P_MaiStop(BRAP_OP_P_Handle    hOp)
{
    /* TODO: Program the following to disable MAI

        AUD_FMM_OP_CTRL_MAI_CFG  :    ENABLE_MAI = Disable
    */

    /* Enable MAI upstream request */
    ret = BRAP_OP_P_EnableStream(hAUD, BRAP_OutputPort_eMai, false);

}

BERR_Code
BRAP_OP_P_ExternalMClkConfig(
                    BRAP_OP_P_Handle    hOp,
                    BRAP_OP_EXT        eExternal,
                    BRAP_OP_PLL     eMClkRate
                    )
{
    /*  TODO:
        Select AUD_FMM_OP_CTRL_MCLK_CFG_EXT0 or AUD_FMM_OP_CTRL_MCLK_CFG_EXT1 based on eExternal
    
        program PLLCLKSEL field of the selected register = eMClkRate */
}



    
/* This function sets the PLL sampling rate puting in 'User' mode */
BERR_Code 
BRAP_OP_P_ProgramSamplingRateInUserMode(BRAP_OP_P_Handle    hOp, 
                                        BRAP_OP_PLL        ePll,
                                        BAVC_AudioSamplingRate eSamplingRate)
{
    BERR_Code            ret = BERR_SUCCESS;
    BREG_Handle            hRegister = handle->hRegister;
    uint32_t            reg;
    uint32_t            offset=0;
    uint32_t            regDiv1, regDiv2, regDiv3, regControl;
    uint32_t            i;    

    BDBG_ENTER(BRAP_PLL_SetSamplingRate);
    BDBG_MSG(("BRAP_PLL_SetSamplingRate(): handle=0x%08x ePll=%d eSamplingRate=%d", handle, ePll, eSamplingRate));

    

    offset = (int)(ePll) * (BCHP_AUD_FMM_PLL1_MACRO - BCHP_AUD_FMM_PLL0_MACRO);    


    /* For seting all the frequency, 'User Mode' is used */

    regDiv1 = PLLSR[0].pll_user_div1;
    regDiv2 = PLLSR[0].pll_user_div2;
    regDiv3 = PLLSR[0].pll_divider;

    for (i=0; i < sizeof(PLLSR)/sizeof(BRAP_DEC_P_PLLSampleRate); i++)
    {
        if ( eSamplingRate == PLLSR[i].eSamplingRate )
        {
            regDiv1 = PLLSR[i].pll_user_div1;
            regDiv2 = PLLSR[i].pll_user_div2;
            regDiv3 = PLLSR[i].pll_divider;
            break;
        }
    }

    /* Program the the DIV1, DIV2 & DIVIDER register as per the values 
       corresponding to the desired sampling rate (Calculated already) */
    BRAP_Write32(hRegister, BCHP_AUD_FMM_PLL0_USER_DIV1 + offset , regDiv1);
    BRAP_Write32(hRegister, BCHP_AUD_FMM_PLL0_USER_DIV2 + offset , regDiv2);
    BRAP_Write32(hRegister, BCHP_AUD_FMM_PLL0_DIVIDER + offset , regDiv3);

    /* Program the MACRO_SELECT = User mode */
    
    reg = BRAP_Read32(hRegister, BCHP_AUD_FMM_PLL0_MACRO + offset );
    reg &= ~(BCHP_MASK(AUD_FMM_PLL0_MACRO, MACRO_SELECT));
    reg |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_MACRO, MACRO_SELECT, User));
    BRAP_Write32(hRegister, BCHP_AUD_FMM_PLL0_MACRO + offset , reg);

    regControl |= BCHP_AUD_FMM_PLL0_CONTROL_USER_UPDATE_DIVIDERS_MASK;

    /* Set OUTPUT_ENABLE, OUTPUT_OVERRIDE & UPDATE_OVERRIDE fields of 
       PLLX_CONTROL Register to 1 */
    regControl = BRAP_Read32(hRegister, BCHP_AUD_FMM_PLL0_CONTROL + offset );

    regControl &= ~((BCHP_MASK(AUD_FMM_PLL0_CONTROL, UPDATE_OVERRIDE)) |
                    (BCHP_MASK(AUD_FMM_PLL0_CONTROL, OUTPUT_OVERRIDE)) |
                    (BCHP_MASK(AUD_FMM_PLL0_CONTROL, OUTPUT_ENABLE)) );
    regControl |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, UPDATE_OVERRIDE, User_control)) |
                  (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, OUTPUT_OVERRIDE, User_control)) |
                  (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, OUTPUT_ENABLE, Enable));

    /* Select the proper timebase in PLLX_CONTROL Register */
    regControl = BRAP_Read32(hRegister, BCHP_AUD_FMM_PLL0_CONTROL + offset );
    regControl &= ~(BCHP_MASK(AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT));

    switch(hOp->eTimebase)
    {
        case BRAP_Rave_eTimebase0:
            regControl |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27a));
            break;
        case BRAP_Rave_eTimebase1:
            regControl |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27b));
            break;
        case BRAP_Rave_eTimebase2:
            regControl |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27c));
            break;
        case BRAP_Rave_eTimebase3:
            regControl |= (BCHP_FIELD_ENUM (AUD_FMM_PLL0_CONTROL, REFERENCE_SELECT, Clk27d));
            break;
        default:
            /* TODO: Print and return error */
            break;
    }

    BRAP_Write32(hRegister, BCHP_AUD_FMM_PLL0_CONTROL + offset , regControl);


    /* Store the sampling rate */
    hOp->eSamplingRate = eSamplingRate;
    
    BDBG_LEAVE(BRAP_PLL_SetSamplingRate);

    return ret;
}



/* Following function has to be called by external module for seting 
   sampling rate for a particular o/p port */
BERR_Code 
BRAP_OP_SetSamplingRate(    BRAP_ChannelHandle     hRapCh, 
                            BRAP_OutputPortChannelPairIndex eOpChPair,        /* [in] Output channel pair index */
                            BAVC_AudioSamplingRate eSamplingRate)
{
    BRAP_OP_PLL        ePll;
    BRAP_OP_P_Handle    hOp = hRapCh->hRap->hFmm[hRapCh->sModuleIds.uiFMMIndex]->
                          hOp[hRapCh->sModuleIds.eOutputPort[eOpChPair]];
    
    /* Get the PLL connected */
    switch( hOp->eOutputPort)
    {
         case BRAP_OutputPort_eSpdif:
            ePll = hOp->sSpdifSettings.ePll;
            break;
        case BRAP_OutputPort_eDac0:
            break;
        case BRAP_OutputPort_eDac1:
            break;
        case BRAP_OutputPort_eI2s0:
        case BRAP_OutputPort_eI2s1:
        case BRAP_OutputPort_eI2s2:
            ePll = hOp->sI2sSettings.ePll;
            break;
        case BRAP_OutputPort_eFlex:
            ePll = hOp->sFlexSettings.ePll;
            break;
        case BRAP_OutputPort_eMai:
            /* TODO: Return error as we can't set sampling rate for MAI */
            break;
        default:
            /* Print error message */
            break;
    }    

    /* Disable the o/p */
    ret = BRAP_OP_P_EnableStream(hOp, false);

    /* Program the sampling rate */
    if((hOp->eOutputPort == BRAP_OutputPort_eDac0) || (hOp->eOutputPort == BRAP_OutputPort_eDac1))
        /* Program HiFi DAC sampling rate */
        ret = BRAP_OP_P_ProgramDacSamplingRate(hOp, eSamplingRate);
    else
        /* Program the Timebase */
        ret = BRAP_OP_P_ProgramTimebase (hOp, pParams->ePll, pParams->eTimebase);
    
    
    /* Enable the o/p */
    ret = BRAP_OP_P_EnableStream(hOp, true);
}

BERR_Code 
BRAP_OP_GetSamplingRate(    BRAP_ChannelHandle     hRapCh, 
                            BRAP_OutputPortChannelPairIndex eOpChPair,        /* [in] Output channel pair index */
                            BAVC_AudioSamplingRate *pSamplingRate)
{
    BRAP_OP_P_Handle    hOp = hRapCh->hRap->hFmm[hRapCh->sModuleIds.uiFMMIndex]->
                          hOp[hRapCh->sModuleIds.eOutputPort[eOpChPair]];

    /* Return the stored the sampling rate */
    *pSamplingRate = hOp->eSamplingRate;
    
}

/* Audio Manager related Functionality */


#if 0
BERR_Code
BRAP_OutAlignSamples(BRAP_OP_P_Handle    hOp,
                        BRAP_Align     eOutput,
                        BRAP_OP_PLL eMClkRate,
                        bool bVal
                        )
{
    /* based upon bval and eOutput program the following registers
     AUD_FMM_OP_CTRL_ENABLE_SET : ALIGN_I2S_SAMPLES and I2S_LRCK_ENA
                OR
     AUD_FMM_OP_CTRL_ENABLE_SET : ALIGN_FLEX_SAMPLES and FLEX_LRCK_ENA
     like 
    
    if(eOutput==BRAP_Align_eI2S && bVal)
    {
         
        reg&=~(BCHIP_AUD_FMM_OP_CTRL_ENABLE_SET_ALIGN_I2S_SAMPLES_MASK
                | BCHIP_AUD_FMM_OP_CTRL_ENABLE_SET_I2S_LRCK_ENA_MASK);
    }

     Moreover if bval is zero then disable also from 
     AUD_FMM_OP_CTRL_ENABLE_CLEAR : ALIGN_I2S_SAMPLES , I2S_LRCK_ENA
    OR
    AUD_FMM_OP_CTRL_ENABLE_CLEAR : ALIGN_FLEX_SAMPLES , FLEX_LRCK_ENA

     program AUD_FMM_OP_CTRL_FLEX_CFG1 : IGNORE_VALID =1
      program AUD_FMM_OP_CTRL_FLEX_CFG1: IGNORE_REQ_TO_SEND=1
      */
}


#endif    


#if 0
BERR_Code 
BRAP_SetOutputSource(
                           BRAP_OP_P_Handle            hOp,            /* [in] an AUD handle */
                           BRAP_OutputPort                eInput,
                           BRAP_OutputPort            eOutput,    /* [in]  */ 
                           )
{
    
        /* BASED UPON eInput do the following 
         if eInput != MAI OR rFMOD THEN RETURN WITH ERROR
    
         program program AUD_FMM_OP_CTRL_STREAM_CFG : RFMOD_SEL
         program AUD_FMM_OP_CTRL_STREAM_CFG : MAI_SEL
         */
        
}

#endif
#endif /*TBD later */

/* End of File */
