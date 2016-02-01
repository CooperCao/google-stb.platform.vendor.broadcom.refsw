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
*      Module name: MIXER
*      This file contains the implementation of all PIs for the Mixer 
*      abstraction. 
*      
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "brap.h"
#include "brap_priv.h"

BDBG_MODULE (rap_mixer);

#define BRAP_MIXER_P_INVALID_SRCCH 0xf
#define BRAP_MIXER_P_INVALID_MIXER_INPUT 0xffffffff

#define   BRAP_MIXER_P_MAX_8000_DB_VOLUME   89
#define   BRAP_MIXER_P_FRACTION_IN_DB       100
#define   BRAP_MIXER_P_DEFAULT_VOLUME       0 /* in dB. 0 dB => 0x8000 => full volume */
#define   BRAP_MIXER_P_DEFAULT_SCALE_VALUE  0  /* 0 dB => No scaling */        
#define   BRAP_MIXER_P_DEFAULT_BITS_PER_SAMPLE      24
    

static BERR_Code BRAP_MIXER_P_HWConfig ( 
    BRAP_MIXER_P_Handle   hMixer,       /* [in] Mixer handle */
    unsigned int          uiMixerInput  /* [in] Id of the mixer input 
                                            associated with this audio stream*/    
);


static const unsigned int  BRAP_Vol8000_DB[] = 
{
    0x8000, 0x7214, 0x65ac, 0x5a9d, 0x50c3, 0x47fa, 0x4026, 0x392c,
    0x32f5, 0x2d6a, 0x287a, 0x2413, 0x2026, 0x1ca7, 0x198a, 0x16c3,
    0x1449, 0x1214, 0x101d, 0xe5c,  0xccc,  0xb68,  0xa2a,  0x90f,
    0x813,  0x732,  0x66a,  0x5b7,  0x518,  0x48a,  0x40c,  0x39b,
    0x337,  0x2dd,  0x28d,  0x246,  0x207,  0x1ce,  0x19c,  0x16f,
    0x147,  0x124,  0x104,  0xe7,   0xce,   0xb8,   0xa4,   0x92,
    0x82,   0x74,   0x67,   0x5c,   0x52,   0x49,   0x41,   0x3a,
    0x33,   0x2e,   0x29,   0x24,   0x20,   0x1d,   0x1a,   0x17,
    0x14,   0x12,   0x10,   0xe,    0xd,    0xb,    0xa,    0x9,
    0x8,    0x7,    0x6,    0x5,    0x5,    0x4,    0x4,    0x3,
    0x3,    0x2,    0x2,    0x2,    0x2,    0x1,    0x1,    0x1,
    0x1,    0x0
};


static const BRAP_MIXER_P_Settings  defMixerSettings=
{
    BRAP_MIXER_P_INVALID_MIXER_INPUT,   /* uiMixerInput */
	false								/* bDisableRampUp */
};


static const BRAP_MIXER_P_Params  defMixerParams =
{

    {
        { 
            BRAP_MIXER_Input_eStereo,  /* sExtParams.sInputParams.eMixerInputAudioMode */
            BRAP_MIXER_P_DEFAULT_SCALE_VALUE  /* sExtParams.sInputParams.uiScaleValue */
        }
    },
    BRAP_MIXER_P_INVALID_SRCCH,           /* uiSrcChId */
    false,                                /* bCompress */
    BRAP_MIXER_P_DEFAULT_BITS_PER_SAMPLE  /* uiStreamRes */        
};


BERR_Code 
BRAP_MIXER_P_GetDefaultSettings ( 
    BRAP_MIXER_P_Settings   * pDefSettings  /* Pointer to memory where default
                                               settings should be written */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_MIXER_P_GetDefaultSettings);

    BDBG_ASSERT (pDefSettings);            
    
    *pDefSettings = defMixerSettings;
    BDBG_LEAVE (BRAP_MIXER_P_GetDefaultSettings);
    return ret;
}



BERR_Code 
BRAP_MIXER_P_GetDefaultParams ( 
    BRAP_MIXER_P_Params  * pDefParams   /* Pointer to memory where default
                                           settings should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER (BRAP_MIXER_P_GetDefaultParams);

    BDBG_ASSERT (pDefParams);            
    
    *pDefParams = defMixerParams;
    BDBG_LEAVE (BRAP_MIXER_P_GetDefaultParams);
    return ret;
}

BERR_Code 
BRAP_MIXER_P_GetCurrentParams ( 
	BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput,  /* [in] Id of the mixer input 
                                            associated with this audio stream*/
    BRAP_MIXER_P_Params  * pCurParams   /*[out] Pointer to memory where current 
                                        params should be written */    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BDBG_ENTER ( BRAP_MIXER_P_GetCurrentParams );
	BDBG_ASSERT( hMixer );
	BDBG_ASSERT( pCurParams );

	pCurParams->sExtParams.sInputParams = hMixer->sInputParams[uiMixerInput];
	pCurParams->uiSrcChId = hMixer->uiMixerInput[uiMixerInput];
	pCurParams->bCompress = hMixer->bCompress;
	pCurParams->uiStreamRes = hMixer->uiStreamRes;
	
    BDBG_LEAVE ( BRAP_MIXER_P_GetCurrentParams );
    return ret;
}


BERR_Code BRAP_MIXER_P_Open (
    BRAP_FMM_P_Handle     hFmm,       /* [in] Parent FMM handle */
    BRAP_MIXER_P_Handle * phMixer,    /* [out] Pointer to Mixer handle.
                                         If this Mixer has been opened before, 
                                         this will return the previously created
                                         handle, else, it will return the newly 
                                         created handle */
    unsigned int          uiIndex,    /* [in] Mixer index */
    const BRAP_MIXER_P_Settings * pSettings  /* [in] Mixer settings */
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_MIXER_P_Handle hMixer;
    unsigned int i, uiVolume;
    uint32_t ui32RegVal = 0;
    uint32_t ui32Temp=0, ui32AttenDb=0, ui32Delta=0;

    BDBG_ENTER (BRAP_MIXER_P_Open);

    /* 1. Check all input parameters to the function. Return error if
      - FMM handle is NULL
      - Given index exceeds maximum no. of Mixers
      - Pointer to Settings structure is NULL
      - Pointer to Mixer handle is NULL     */
    BDBG_ASSERT (hFmm);
    BDBG_ASSERT (pSettings);
    BDBG_ASSERT (phMixer);
    BDBG_MSG (("BRAP_MIXER_P_Open:"
               " hFmm=0x%x, uiMixerIndex=%d," 
               " pSettings->uiMixerInput=%d", 
                hFmm, uiIndex, pSettings->uiMixerInput));
    
    if (uiIndex > (BRAP_RM_P_MAX_MIXERS -1))
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }     

    /* Check the Settings structure */
    if (pSettings->uiMixerInput >= BRAP_RM_P_MAX_MIXER_INPUTS) 
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    { /* If not in WatchDog recovery */  

        if (hFmm->hMixer[uiIndex] != NULL) 
        {        
            /* This Mixer has already been Opened. We are only adding a new Input 
               to the Mixer. Dont allocate new memory or set common settings again. */
            hMixer = hFmm->hMixer[uiIndex];
        }
        else 
        {
            /* 2. Allocate memory for the Mixer handle. Fill in parameters in the Mixer
            handle. */
    
            /* Allocate Mixer handle */
            hMixer = (BRAP_MIXER_P_Handle) BKNI_Malloc (sizeof(BRAP_MIXER_P_Object));
            if (hMixer == NULL)
            {
                return BERR_TRACE (BERR_OUT_OF_SYSTEM_MEMORY);
            }
        
            /* Clear the handle memory */
            BKNI_Memset(hMixer, 0, sizeof(BRAP_MIXER_P_Object));

            /* Initialise known elements in Source Channel handle */
            hMixer->hChip = hFmm->hChip;
            hMixer->hRegister = hFmm->hRegister;
            hMixer->hHeap = hFmm->hHeap;
            hMixer->hInt = hFmm->hInt;
            hMixer->hFmm = hFmm;
            hMixer->uiMixerIndex = uiIndex;
            hMixer->bOutputMute = false; /* UnMute Mixer outputs on open time */
		hMixer->bDisableRampUp = pSettings->bDisableRampUp;
    
            /*Check: Call Resource manager to get DP id*/
            BRAP_RM_P_GetDpId(uiIndex, &i);
            hMixer->uiDpIndex = i;
            BDBG_MSG (("BRAP_MIXER_P_Open: Mixer Id=%d, DP Id=%d", 
               hMixer->uiMixerIndex, hMixer->uiDpIndex));

            /* Mixer Register offset */
            hMixer->ui32MixerOffset = (BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT1_CONFIG - 
                                  BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG)
                                  * (uiIndex - (hMixer->uiDpIndex*(BRAP_RM_P_MAX_MIXER_PER_DP-1)));
    
            /* DP Register offset for the current mixer */
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
            hMixer->ui32DpOffset = (BCHP_AUD_FMM_DP_CTRL1_STRM_ENA - 
                              BCHP_AUD_FMM_DP_CTRL0_STRM_ENA)
                              * hMixer->uiDpIndex;
#else
            hMixer->ui32DpOffset = 0;
#endif
       
            for (i=0; i < BRAP_RM_P_MAX_MIXER_INPUTS; i++) 
            {
                hMixer->eState[i] = BRAP_MIXER_P_State_eInvalid;
                hMixer->uiMixerInput[i] = BRAP_MIXER_P_INVALID_MIXER_INPUT;
            }

            /* Set default volume */
            hMixer->uiLVolume = BRAP_MIXER_P_DEFAULT_VOLUME;
            hMixer->uiRVolume = BRAP_MIXER_P_DEFAULT_VOLUME;

            ui32AttenDb = BRAP_MIXER_P_DEFAULT_VOLUME
                        / BRAP_MIXER_P_FRACTION_IN_DB;

        
            ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
            ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
            ui32Delta = (ui32Delta * 
                     (BRAP_MIXER_P_DEFAULT_VOLUME%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
            ui32Temp -= ui32Delta;        
    
            BDBG_MSG (("BRAP_MIXER_P_Open:" 
                       " BRAP_MIXER_P_DEFAULT_VOLUME=%d dB, converted reg val =0x%x",
                        BRAP_MIXER_P_DEFAULT_VOLUME, ui32Temp));     
    
            ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        RIGHT_VOLUME_LEVEL, 
                        ui32Temp));  
            ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        LEFT_VOLUME_LEVEL, 
                        ui32Temp));  
            BRAP_Write32 (hMixer->hRegister, 
                   (hMixer->ui32DpOffset 
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME
                   + hMixer->ui32MixerOffset), 
                  ui32RegVal);  
        } /* End: If not first Open */
    }   /* End: If not in WatchDog recovery */
    else
    {	
		hMixer = *phMixer;
		/* TODO: this should be called only the firs ttime MIXER_P_Open
           is called during watchdog recovery, not each time. */
        /* Set volume */
        
        
        /* Calculate register value for Left Channel volume */ 
        uiVolume = hMixer->uiLVolume;
        ui32AttenDb = uiVolume/BRAP_MIXER_P_FRACTION_IN_DB;
    
        if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
        {
            uiVolume = BRAP_MIXER_P_MAX_8000_DB_VOLUME 
                    * BRAP_MIXER_P_FRACTION_IN_DB;
            ui32Temp = 0;
        }
        else
        {
            ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
            ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
            ui32Delta = (ui32Delta * (uiVolume%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
            ui32Temp -= ui32Delta;
        }
        ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        LEFT_VOLUME_LEVEL, 
                        ui32Temp));      
        BDBG_MSG (("BRAP_MIXER_P_Open(): Left Volume Reg value =0x%x",
                ui32Temp));

        /* Calculate register value for Right Channel volume */ 
        uiVolume = hMixer->uiRVolume;
        ui32AttenDb = uiVolume/BRAP_MIXER_P_FRACTION_IN_DB;
    
        if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
        {
            uiVolume = BRAP_MIXER_P_MAX_8000_DB_VOLUME 
                    * BRAP_MIXER_P_FRACTION_IN_DB;
            ui32Temp = 0;
        }
        else
        {
            ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
            ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
            ui32Delta = (ui32Delta * (uiVolume%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
            ui32Temp -= ui32Delta;
        }
        ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        RIGHT_VOLUME_LEVEL, 
                        ui32Temp));      
        BDBG_MSG (("BRAP_MIXER_P_Open(): Right Volume Reg value =0x%x",
                ui32Temp));        
        
        BRAP_Write32 (hMixer->hRegister, 
                   (hMixer->ui32DpOffset 
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME
                   + hMixer->ui32MixerOffset), 
                  ui32RegVal);          
    }

    hMixer->eState[pSettings->uiMixerInput] = BRAP_MIXER_P_State_eStopped;

    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hFmm) == false)
    { /* If not in WatchDog recovery */  
    
        if (hFmm->hMixer[uiIndex] == NULL) {
            /* Store the Mixer handle inside the FMM handle */
            hFmm->hMixer[uiIndex] = hMixer;
        }   
        /* Return the filled handle */
        *phMixer = hMixer;  
    }

    BDBG_LEAVE (BRAP_MIXER_P_Open);
    return ret;

}



BERR_Code BRAP_MIXER_P_Close ( 
    BRAP_MIXER_P_Handle hMixer,       /* [in] Mixer handle */
    unsigned int        uiMixerInput  /* [in] Id of the mixer input 
                                          associated with this audio stream*/
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int count=0, i=0;

    BDBG_ENTER (BRAP_MIXER_P_Close);
    BDBG_ASSERT (hMixer);
    BDBG_MSG (("BRAP_MIXER_P_Close:"
               " hMixer->uiMixerIndex=%d, uiMixerInput=%d", 
                hMixer->uiMixerIndex, uiMixerInput));

    if (uiMixerInput > (BRAP_RM_P_MAX_MIXER_INPUTS -1))
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }    
    if (hMixer->eState[uiMixerInput] == BRAP_MIXER_P_State_eRunning)
    {
        BDBG_ERR(("BRAP_MIXER_P_Close: Input stream %d to Mixer %d is still active."
                    "Cant close it",  
                    uiMixerInput, hMixer->uiMixerIndex));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else if (hMixer->eState[uiMixerInput] == BRAP_MIXER_P_State_eInvalid)
    {
        BDBG_WRN(("BRAP_MIXER_P_Close: Input stream %d to Mixer %d was never opened."
                    "Ignoring the close command.",  
                    uiMixerInput, hMixer->uiMixerIndex));
        BDBG_LEAVE (BRAP_MIXER_P_Close);
        return BERR_SUCCESS;        
    }    
    hMixer->eState[uiMixerInput] = BRAP_MIXER_P_State_eInvalid;
            

    /* If there are no valid inputs left to this Mixer, remove all reference
       to it and free the handle. */
       
    for (i=0; i<BRAP_RM_P_MAX_MIXER_INPUTS; i++) 
    {
        if (hMixer->eState[i] != BRAP_MIXER_P_State_eInvalid)
        { 
            count ++;
        }
    }
    if (count == 0) 
    {
        /* Remove referrence to this Mixer from the parent FMM */ 
        hMixer->hFmm->hMixer[hMixer->uiMixerIndex] = NULL;

        /* Free the Mixer Handle memory*/
        BKNI_Free (hMixer);     
    }
                 
    BDBG_LEAVE (BRAP_MIXER_P_Close);
    return ret;
}



BERR_Code BRAP_MIXER_P_Start ( 
    BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput,  /* [in] Id of the mixer input 
                                            associated with this audio stream*/
    const BRAP_MIXER_P_Params * pParams  /* [in] Pointer to start
                                            parameters for this stream */ 
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    unsigned int temp;
	unsigned int uiScaleValue=0;
	unsigned int ui32AttenDb;
	uint32_t ui32Temp;
	uint32_t ui32Delta;           

    BDBG_ENTER (BRAP_MIXER_P_Start);
    BDBG_ASSERT (hMixer);

    if (uiMixerInput > (BRAP_RM_P_MAX_MIXER_INPUTS -1))
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }   

    if (hMixer->eState[uiMixerInput] == BRAP_MIXER_P_State_eRunning)
    {
        BDBG_WRN(("BRAP_MIXER_P_Start: Input stream %d to Mixer %d is already active."
                    "Ignoring the start command.",  
                    uiMixerInput, hMixer->uiMixerIndex));
        BDBG_LEAVE (BRAP_MIXER_P_Start);
        return BERR_SUCCESS;        
    }
    else if (hMixer->eState[uiMixerInput] == BRAP_MIXER_P_State_eInvalid)
    {
        BDBG_ERR(("BRAP_MIXER_P_Start: Input stream %d to Mixer %d has not been opened."
                    "Cant start it!!",  
                    uiMixerInput, hMixer->uiMixerIndex));
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }
   
    if (BRAP_FMM_P_GetWatchdogRecoveryFlag (hMixer->hFmm) == false)
    { /* If not in WatchDog recovery */  
    
        BDBG_ASSERT (pParams);

        BDBG_MSG (("BRAP_MIXER_P_Start:"
                "hMixer->uiMixerIndex=%d, uiMixerInput=%d," 
                "pParams->sExtParams.sInputParams.eMixerInputAudioMode =%d,"
                "pParams->sExtParams.sInputParams.uiScaleValue=%d,"
                "pParams->uiSrcChId=%d, pParams->bCompress=%d, pParams->uiStreamRes=%d", 
                hMixer->uiMixerIndex, uiMixerInput,
                pParams->sExtParams.sInputParams.eMixerInputAudioMode, 
                pParams->sExtParams.sInputParams.uiScaleValue,
                pParams->uiSrcChId, pParams->bCompress, pParams->uiStreamRes));
  
        if  (pParams->uiSrcChId > BRAP_RM_P_MAX_SRC_CHANNELS) 
        {
            return BERR_TRACE (BERR_INVALID_PARAMETER);
        }   

        /* make sure this mixer input is not already connected to some 
        Source Channel */
        if (hMixer->uiMixerInput[uiMixerInput] != BRAP_MIXER_P_INVALID_MIXER_INPUT)
        {
            BDBG_ERR(("BRAP_MIXER_P_Start(): SrcCh %d is already connected to Mixer input %d."
                    "Cannot connect SrcCh %d",
                    hMixer->uiMixerInput[uiMixerInput],
                    uiMixerInput, pParams->uiSrcChId));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
        }      
        /* Save info about which source channel is connected to which input of the Mixer */
        hMixer->uiMixerInput[uiMixerInput] = pParams->uiSrcChId;

        /* Save required parameters in the Mixer Object */
        hMixer->sInputParams[uiMixerInput] = pParams->sExtParams.sInputParams; 
        hMixer->bCompress = pParams->bCompress;
        hMixer->uiStreamRes = pParams->uiStreamRes;        

    }
    else          
    {
        /* Read the Config register for this mixer input */
        ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                            ( hMixer->ui32DpOffset
                            + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                            + hMixer->ui32MixerOffset 
                            + (uiMixerInput * 4)));    

        /* Enable/Disable Scaling and set scale coefficient */
        ui32RegVal &= ~(BCHP_MASK (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALE_ENB));        
        ui32RegVal &= ~(BCHP_MASK (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALING_COEF));        

#ifndef EMULATION
        /* Enable scaling and set scaling coefficient */
        ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALE_ENB, 
                        Enable));       
#else
        /* Disable scaling and set scaling coefficient */
        ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALE_ENB, 
                        Disable));       
#endif                        

        uiScaleValue = hMixer->sInputParams[uiMixerInput].uiScaleValue;

        /* Calculate value */ 
        ui32AttenDb = uiScaleValue/BRAP_MIXER_P_FRACTION_IN_DB;

        if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
        {
            uiScaleValue = BRAP_MIXER_P_MAX_8000_DB_VOLUME
                                   * BRAP_MIXER_P_FRACTION_IN_DB;
            ui32Temp=0;
        }
        else
        {
            ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
            ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
            ui32Delta = (ui32Delta 
        				* (uiScaleValue%BRAP_MIXER_P_FRACTION_IN_DB))
                        /(BRAP_MIXER_P_FRACTION_IN_DB);
            ui32Temp -= ui32Delta;
        }

        ui32RegVal |= (BCHP_FIELD_DATA (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALING_COEF, 
                        ui32Temp));       
        BDBG_MSG (("BRAP_MIXER_SetInputScaling: Scaling value =%d Reg value =0x%x",
                uiScaleValue, ui32Temp));
        	     
        /* Write to the Config register */
        BRAP_Write32 (hMixer->hRegister, 
                      (hMixer->ui32DpOffset
                       + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                       + hMixer->ui32MixerOffset 
                       + (uiMixerInput * 4)), 
                      ui32RegVal);    
    }
     
    /* MIX_PB_ID is the Stream's index in this DP. range=0-3*/
    ret = BRAP_RM_P_GetDpStreamId (hMixer->uiMixerInput[uiMixerInput], 
            hMixer->uiDpIndex, &temp); 
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_P_Start(): Cant get stream index in this DP"));
        return BERR_TRACE (ret);
    } 
            
    /* Program the Source Channel id in 
       AUD_FMM_DP_CTRL0_MS_CLIENT[mixerId]_MIX[mixerInputId]_CONFIG */
       
    /* Read from the appropriate register */
    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                  (  hMixer->ui32DpOffset
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                   + hMixer->ui32MixerOffset 
                   + (uiMixerInput * 4))); 
           
    ui32RegVal &= ~(BCHP_MASK (
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MIX_PB_ID)); 
   
    ui32RegVal |= (BCHP_FIELD_DATA (    
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MIX_PB_ID, 
                    temp));

    /* Enable/Disable Scaling and set scale coefficient */
    ui32RegVal &= ~(BCHP_MASK (
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MIX_SCALE_ENB)); 
    if((hMixer->bCompress==false) &&(hMixer->sInputParams[uiMixerInput].uiScaleValue != 0))
    {
        /* Enable scaling and set scaling coefficient */
        ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALE_ENB, 
                        Enable));       
    }
    else
    {
        /* Disable scaling and set scaling coefficient */
        ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_SCALE_ENB, 
                        Disable));       
    }
    

    /* Write to the appropriate register */
    BRAP_Write32 (hMixer->hRegister, 
                  (  hMixer->ui32DpOffset
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                   + hMixer->ui32MixerOffset 
                   + (uiMixerInput * 4)), 
                  ui32RegVal);        
    
    /* Configure Mixer Hardware as per the channel parameters */
    BRAP_MIXER_P_HWConfig (hMixer, uiMixerInput);

    /* Enable the Mixer output */
    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                        (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_ENA));
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, reserved_for_eco0));

    /* Select correct register field based on Mixer Index */
    temp = hMixer->uiMixerIndex - (hMixer->uiDpIndex*(BRAP_RM_P_MAX_MIXER_PER_DP-1));
    switch (temp)
    {
        case 0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM0_ENA));
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                           STREAM0_ENA, Enable);
            break;
        case 1:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA));
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                           STREAM1_ENA, Enable);
            break;
        case 2:
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM2_ENA));
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                           STREAM2_ENA, Enable);
            break;
        case 3:
            ui32RegVal &= ~(BCHP_MASK(AUD_FMM_DP_CTRL0_STRM_ENA, STREAM3_ENA));
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                           STREAM3_ENA, Enable);
            break;
        default:
            BDBG_ERR(("BRAP_MIXER_P_Start(): Incorrect Mixer index ", hMixer->uiMixerIndex));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("BRAP_MIXER_P_Start(): Stream %d enabled", temp));

    BRAP_Write32 (hMixer->hRegister, 
                 (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_ENA), 
                 ui32RegVal);

    hMixer->eState[uiMixerInput] = BRAP_MIXER_P_State_eRunning;

    BDBG_LEAVE (BRAP_MIXER_P_Start);
    return ret;
}



BERR_Code BRAP_MIXER_P_Stop ( 
    BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput   /* [in] Id of the mixer input 
                                            associated with this audio stream*/
)
{
    BERR_Code ret = BERR_SUCCESS;
    uint32_t  ui32RegVal;
    unsigned int temp;
    bool bDisableMixer = true;

    BDBG_ENTER (BRAP_MIXER_P_Stop);
    BDBG_ASSERT (hMixer);
    BDBG_MSG (("BRAP_MIXER_P_Stop:"
               "hMixer->uiMixerIndex=%d, uiMixerInput=%d", 
                hMixer->uiMixerIndex, uiMixerInput));
  
    if (uiMixerInput > (BRAP_RM_P_MAX_MIXER_INPUTS -1))
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }       

    if (hMixer->eState[uiMixerInput] != BRAP_MIXER_P_State_eRunning)
    {
        BDBG_WRN(("BRAP_MIXER_P_Stop: Input stream %d to Mixer %d is not active."
                    "Ignoring the stop command.",  
                    uiMixerInput, hMixer->uiMixerIndex));
        BDBG_LEAVE (BRAP_MIXER_P_Stop);
        return BERR_SUCCESS;
    }

    for (temp=0; temp<BRAP_RM_P_MAX_MIXER_INPUTS; temp++) 
    {
        if ((BRAP_MIXER_P_State_eRunning == hMixer->eState[temp]) && 
            (temp != uiMixerInput))
        {
            /* Some other input of the mixer is in the running state, so
               do not disable the mixer output */
            bDisableMixer = false;
        }
    }

    if (true == bDisableMixer)
    {
	    /* Disable the Mixer output */
        ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                        (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_ENA));
    
        /* Select correct register field based on Mixer Index */
        temp = hMixer->uiMixerIndex - (hMixer->uiDpIndex*(BRAP_RM_P_MAX_MIXER_PER_DP-1));    
        switch (temp)
        {
            case 0:
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM0_ENA));
                ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                               STREAM0_ENA, Disable);
                break;
            case 1:
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM1_ENA));
                ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                               STREAM1_ENA, Disable);
                break;
            case 2:
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM2_ENA));
                ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                               STREAM2_ENA, Disable);
                break;
            case 3:
                ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_ENA, STREAM3_ENA));
                ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_STRM_ENA, 
                                               STREAM3_ENA, Disable);
                break;
            default:
                BDBG_ERR(("BRAP_MIXER_P_Stop(): Incorrect Mixer index (%d)", hMixer->uiMixerIndex));
                return BERR_TRACE (BERR_NOT_SUPPORTED);
        }
    
        BRAP_Write32 (hMixer->hRegister, 
                      (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_ENA), 
                      ui32RegVal);
    }

    /* Remove info about the source channel connection to this input of the
       Mixer */
    hMixer->uiMixerInput[uiMixerInput] = BRAP_MIXER_P_INVALID_MIXER_INPUT;

    /* Disable mixing for this mixer input */
    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                        ( hMixer->ui32DpOffset
                        + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                        + hMixer->ui32MixerOffset 
                        + (uiMixerInput * 4)));

    ui32RegVal &= ~(BCHP_MASK (
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MIX_ENABLE));  
    
    ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_ENABLE, 
                        Disable));
            
    /* Remove the Source Channel id from 
       AUD_FMM_DP_CTRL0_MS_CLIENT[mixerId]_MIX[mixerInputId_CONFIG */
    ui32RegVal &= ~(BCHP_MASK (
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MIX_PB_ID));        
    
    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_PB_ID, 
                        BRAP_MIXER_P_INVALID_SRCCH));
            
    /* Write to the appropriate register */
    BRAP_Write32 (hMixer->hRegister, 
                  (  hMixer->ui32DpOffset
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                   + hMixer->ui32MixerOffset 
                   + (uiMixerInput * 4)), 
                  ui32RegVal);    

#if (BCHP_CHIP == 7400)   
    ui32RegVal = BRAP_Read32 (hMixer->hRegister,BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP);

    ui32RegVal &= ~(BCHP_MASK (    
                    AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                    SCALE_RAMP_STEP_SIZE));
    
    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                        SCALE_RAMP_STEP_SIZE, 
                        0x00));

    BRAP_Write32 (hMixer->hRegister, BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, ui32RegVal);
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hMixer->hRegister, BCHP_AUD_FMM_DP_CTRL1_FMM_SCALE_VOL_STEP, ui32RegVal);
#endif
#endif   
    hMixer->eState[uiMixerInput] = BRAP_MIXER_P_State_eStopped;
    BDBG_LEAVE (BRAP_MIXER_P_Stop);
    
    return ret;
}


/***************************************************************************
Summary:
    Configures the HW registers for the Mixer
Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    None.
**************************************************************************/
static BERR_Code BRAP_MIXER_P_HWConfig ( 
    BRAP_MIXER_P_Handle   hMixer,       /* [in] Mixer handle */
    unsigned int          uiMixerInput  /* [in] Id of the mixer input 
                                            associated with this audio stream*/    
)
{
    BERR_Code ret = BERR_SUCCESS;
    BRAP_SRCCH_P_Handle hSrcCh;
    BRAP_MIXER_InputStreamParams sParams;
    uint32_t ui32RegVal = 0;
    uint32_t ui32Temp=0;
    unsigned int uiStreamId=0;
    BRAP_MIXER_P_StreamRes eStreamRes;
    bool bUnderflowPause=false;
    
    BDBG_ENTER (BRAP_MIXER_P_HWConfig);
    BDBG_ASSERT (hMixer);
    BDBG_MSG (("BRAP_MIXER_P_HWConfig:"
               "hMixer->uiMixerIndex=%d, uiMixerInput=%d, DP Id =%d", 
                hMixer->uiMixerIndex, uiMixerInput, hMixer->uiDpIndex));
  
    if (uiMixerInput > (BRAP_RM_P_MAX_MIXER_INPUTS -1))
    {
        return BERR_TRACE (BERR_INVALID_PARAMETER);
    }       
    
    sParams = hMixer->sInputParams[uiMixerInput]; 
    hSrcCh = hMixer->hFmm->hSrcCh[hMixer->uiMixerInput[uiMixerInput]];
        
    switch (hMixer->uiStreamRes)
    {
        case 16:
           eStreamRes = BRAP_MIXER_P_StreamRes_e16;
           break;
        case 17:
           eStreamRes = BRAP_MIXER_P_StreamRes_e17;
           break;
        case 18:
           eStreamRes = BRAP_MIXER_P_StreamRes_e18;
           break;
        case 19:
           eStreamRes = BRAP_MIXER_P_StreamRes_e19;
           break;
        case 20:
           eStreamRes = BRAP_MIXER_P_StreamRes_e20;
           break;
        case 21:
           eStreamRes = BRAP_MIXER_P_StreamRes_e21;
           break;
        case 22:
           eStreamRes = BRAP_MIXER_P_StreamRes_e22;
           break;
        case 23:
           eStreamRes = BRAP_MIXER_P_StreamRes_e23;
           break;
        case 24:
           eStreamRes = BRAP_MIXER_P_StreamRes_e24;
           break;
        default: 
            BDBG_ERR(("BRAP_MIXER_P_HWConfig(): Incorrect Stream resolution %d", hMixer->uiStreamRes));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* If this is uncompressed data, enable Voume control for this Mixer,
       else disable */
    
    ui32RegVal = 0;
    if (hMixer->bCompress == false)
    {

#ifndef WMA_CERTIFICATION 
#ifndef EMULATION
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_ENA, 
	                                   Enable);
		if (hMixer->bDisableRampUp==false)
		{
		        /* Enable ramping during volume change */
		        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
		                                   VOLUME_RAMP_ENA, 
		                                   Enable);
		        /* Enable volume ramp during startup. */
		        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
		                                   VOLUME_STARTUP_RAMP_DIS, 
		                                   Enable);
		}
		else
		{
		        /* Disable ramping during volume change */
		        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
		                                   VOLUME_RAMP_ENA, 
		                                   Disable);
		        /* Disable volume ramp during startup. */
		        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
		                                   VOLUME_STARTUP_RAMP_DIS, 
		                                   Disable);
		}
#else
	        /* Disable volume control for emulation */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_ENA, 
	                                   Disable); 
	        /* Disable ramping during volume change */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_RAMP_ENA, 
	                                   Disable);                                    
	        /* Disable volume ramp during startup. */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_STARTUP_RAMP_DIS, 
	                                   Disable);
#endif   
#else
	        /* Disable volume control for emulation */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_ENA, 
	                                   Disable); 
	        /* Disable ramping during volume change */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_RAMP_ENA, 
	                                   Disable);                                    
	        /* Disable volume ramp during startup. */
	        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
	                                   VOLUME_STARTUP_RAMP_DIS, 
	                                   Disable);
#endif    


        if (hMixer->bOutputMute == false)
        {
            /* if it was unmuted earlier, UnMute at start time */
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
                                   VOLUME_MUTE_ENA, 
                                   Disable);  
        }
        else
        {
            /* if it was Muted earlier, Mute at start time */
            ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
                                   VOLUME_MUTE_ENA, 
                                   Enable); 
        }
    }

    BRAP_Write32 (hMixer->hRegister, 
                   (hMixer->ui32DpOffset 
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG
                   + hMixer->ui32MixerOffset), 
                  ui32RegVal);

    /* Read the Config register for this mixer input */
    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                        ( hMixer->ui32DpOffset
                        + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                        + hMixer->ui32MixerOffset 
                        + (uiMixerInput * 4)));    

    /* Select stereo, left or right mono for this mix input.*/
    ui32RegVal &= ~(BCHP_MASK (
                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                    MONO_SEL));        
    switch (sParams.eMixerInputAudioMode)
    {
        case BRAP_MIXER_Input_eStereo:
            ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MONO_SEL, STEREO));        
            break;
        case BRAP_MIXER_Input_eLeftMono:
            ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MONO_SEL, LEFT_MONO));        
            break;
        case BRAP_MIXER_Input_eRightMono:
            ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MONO_SEL, RIGHT_MONO));  
            break;
        default:
            BDBG_ERR (("BRAP_MIXER_P_HWConfig(): Invalud Mixer Input Audio Mode" 
                       "%d", sParams.eMixerInputAudioMode));      
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* Enable mixing */
    ui32RegVal |= (BCHP_FIELD_ENUM (
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
                        MIX_ENABLE, 
                        Enable));

     
    /* Write to the Config register */
    BRAP_Write32 (hMixer->hRegister, 
                  (  hMixer->ui32DpOffset
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
                   + hMixer->ui32MixerOffset 
                   + (uiMixerInput * 4)), 
                  ui32RegVal);   


    /* Configure blocks connected to source channel o/p like SRC, Mute 
       and Scaler, corresponding to the current mixer input */

    ui32RegVal = 0;
    
    /* Disable SRC at the init time, it can be enabled by a separate API */
    /* Relevant fields are already 0. Dont do anything here. */       

    /* Enable scale ramp and scale ramp at start up for uncompressed 
       stream and disable them for compressed stream */
    if(hMixer->bCompress != true)
    {

#ifndef EMULATION
        /* Uncompressed stream, so scale ramp */
	if (hMixer->bDisableRampUp==false)
	{
	        ui32RegVal |= (BCHP_FIELD_ENUM 
	                                (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
	                                 SCALE_RAMP_ENA, 
	                                 Enable)); 
	        ui32RegVal |= (BCHP_FIELD_ENUM 
	                                (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
	                                 SCALE_STARTUP_RAMP_ENB, 
	                                Enable));   
	}
	else
	{
#else
	        /* Disable scale ramp for emulation */
	        ui32RegVal |= (BCHP_FIELD_ENUM 
	                                (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
	                                 SCALE_RAMP_ENA, 
	                                 Disable));    
	        ui32RegVal |= (BCHP_FIELD_ENUM 
	                                (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
	                                 SCALE_STARTUP_RAMP_ENB, 
	                                Disable));   
#endif 
#ifndef EMULATION
	}
#endif		
        if (hSrcCh->bMute == false) 
        {    /* UnMute only if the channel was already un-Muted*/    
        ui32RegVal |= (BCHP_FIELD_ENUM 
                                (AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG, 
                                 SCALE_MUTE_ENA, 
                                 Disable));
        }
    }
           
    /* Write to the SRC Config register */
    ret = BRAP_RM_P_GetDpStreamId (hMixer->uiMixerInput[uiMixerInput], 
                                   hMixer->uiDpIndex, &uiStreamId); 
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_P_HWConfig(): Cant get stream index in this DP"));
        return BERR_TRACE (ret);
    }
    
    ui32Temp = BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK1_SCALE_SRC_CONFIG
                - BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG;
    BRAP_Write32 (hMixer->hRegister, 
                    (   hMixer->ui32DpOffset 
                      + BCHP_AUD_FMM_DP_CTRL0_BB_PLAYBACK0_SCALE_SRC_CONFIG + 
                      + (uiStreamId * ui32Temp)), 
                     ui32RegVal);

         
    /* Program the mixer output */

    /* Program stream resolution. It is better to control the stream 
       resolution at this place, because doing it here will cause rounding 
       off whereas if we do it at the o/p block, the lower order bits will 
       be just discarded.
       
       If the desired o/p bit resolution is 32 bits, set this bit resolution 
       to 24 bits and set the bit resolution at the o/p block to 32 bits */
    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
                    (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_FORMAT));
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, reserved0));
    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, reserved1));


    /* As a software workaround for Pr 14690 make 
       STREAMX_UNDERFLOW_PAUSE = 1 dor compressed data */
    if(hMixer->bCompress == true)
    {
        bUnderflowPause = true;
    }
    
    /* Select correct register field based on Mixer Index */
    ui32Temp = hMixer->uiMixerIndex - (hMixer->uiDpIndex*(BRAP_RM_P_MAX_MIXER_PER_DP-1));    
    switch (ui32Temp)
    {
        case 0:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM0_UNDERFLOW_PAUSE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM0_BIT_RESOLUTION));
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM0_UNDERFLOW_PAUSE, bUnderflowPause);
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM0_BIT_RESOLUTION, eStreamRes);           
            BDBG_MSG(("BRAP_MIXER_P_HWConfig(): Stream 0 configured "));
            break;
        case 1:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM1_UNDERFLOW_PAUSE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM1_BIT_RESOLUTION));
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM1_UNDERFLOW_PAUSE, bUnderflowPause);
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM1_BIT_RESOLUTION, eStreamRes);           
            BDBG_MSG(("BRAP_MIXER_P_HWConfig(): Stream 1 configured "));
            break;
        case 2:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM2_UNDERFLOW_PAUSE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM2_BIT_RESOLUTION));
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM2_UNDERFLOW_PAUSE, bUnderflowPause);
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM2_BIT_RESOLUTION, eStreamRes);           
            BDBG_MSG(("BRAP_MIXER_P_HWConfig(): Stream 2 configured "));
            break;
        case 3:
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM3_UNDERFLOW_PAUSE));
            ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_STRM_FORMAT, STREAM3_BIT_RESOLUTION));
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM3_UNDERFLOW_PAUSE, bUnderflowPause);
            ui32RegVal |= BCHP_FIELD_DATA (AUD_FMM_DP_CTRL0_STRM_FORMAT, 
                                           STREAM3_BIT_RESOLUTION, eStreamRes);           
            BDBG_MSG(("BRAP_MIXER_P_HWConfig(): Stream 3 configured "));
            break;
        default:
            BDBG_ERR(("BRAP_MIXER_P_HWConfig(): Incorrect Mixer index %d", hMixer->uiMixerIndex));
            return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    BRAP_Write32 (hMixer->hRegister, 
                  (hMixer->ui32DpOffset + BCHP_AUD_FMM_DP_CTRL0_STRM_FORMAT), 
                  ui32RegVal);    
    
    BDBG_LEAVE (BRAP_MIXER_P_HWConfig);

    return ret;

}

BERR_Code BRAP_MIXER_SetMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool            bMute    /* [in] TRUE: Mute mixer output
                                     FALSE: UnMute mixer output */
)
{
    BERR_Code     eStatus;
    BDBG_ENTER (BRAP_MIXER_SetMute);

    BKNI_EnterCriticalSection();
    eStatus = BRAP_MIXER_P_SetMute_isr (hRap, eOpType, bMute);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE (BRAP_MIXER_SetMute);
    return eStatus;
}

BERR_Code BRAP_MIXER_P_SetMute_isr ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool            bMute    /* [in] TRUE: Mute mixer output
                                     FALSE: UnMute mixer output */
)
{
    BRAP_MIXER_P_Handle  hMixer;
    BERR_Code  ret = BERR_SUCCESS;
    uint32_t ui32RegVal;
    unsigned int mixerId;
    BRAP_OutputPort eTempOpType;

    BDBG_ASSERT (hRap);
    BDBG_ENTER (BRAP_MIXER_P_SetMute_isr);


    /* check if this output port has been opened */
    if (hRap->hFmm[0]->hOp[eOpType] == NULL)
    {
            BDBG_ERR(("This output port hasnt been opened. Cant mute."));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* If Mai port get the Input to Mai */
    if ( eOpType == BRAP_OutputPort_eMai )
    {
    	eTempOpType = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }
    else
    {
    	eTempOpType = eOpType;
    }

    /* Get the mixer handle */
    ret = BRAP_RM_P_GetMixerForOpPort_isr (hRap->hRm, eTempOpType, &mixerId);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_P_SetMute_isr(): Cant get Mixer ID"));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    hMixer = hRap->hFmm[0]->hMixer[mixerId]; /* TODO: remove hardcoded FMM index */
    if(hMixer == NULL)
    {
        BDBG_ERR(("BRAP_MIXER_P_SetMute_isr: Got a NULL mixer handle"));
        return BERR_TRACE (BERR_NOT_INITIALIZED);
    }

    BDBG_MSG (("BRAP_MIXER_P_SetMute_isr():" 
               "Mixer Index = %d, eOpType=%d, bMute=%d", 
                hMixer->uiMixerIndex, eTempOpType, bMute));

    if (hMixer->bCompress == true )
    {
        BDBG_ERR(("BRAP_MIXER_P_SetMute_isr(): valid only for uncompressed data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
    ui32RegVal = BRAP_Read32_isr (hMixer->hRegister,
                               (hMixer->ui32DpOffset 
                                + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG
                                + hMixer->ui32MixerOffset));

    ui32RegVal &= ~(BCHP_MASK (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, VOLUME_MUTE_ENA));

    if (bMute == true )
    {
        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
                                   VOLUME_MUTE_ENA, 
                                   Enable);
    }
    else
    {
        ui32RegVal |= BCHP_FIELD_ENUM (AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG, 
                                   VOLUME_MUTE_ENA, 
                                   Disable);
    }
    BRAP_Write32_isr (hMixer->hRegister, 
                   (hMixer->ui32DpOffset 
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_CONFIG
                   + hMixer->ui32MixerOffset), 
                  ui32RegVal);

    hMixer->bOutputMute = bMute;

    BDBG_LEAVE (BRAP_MIXER_P_SetMute_isr);
    return ret;
}

BERR_Code BRAP_MIXER_GetMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool *          pMute    /* [out] Pointer to memory where the Mute 
                                      status is to be written */            )
{
    BERR_Code     eStatus;
    BDBG_ENTER (BRAP_MIXER_GetMute);

    BKNI_EnterCriticalSection();
    eStatus = BRAP_MIXER_P_GetMute_isr (hRap, eOpType, pMute);
    BKNI_LeaveCriticalSection();
    BDBG_LEAVE (BRAP_MIXER_GetMute);
    return eStatus;
}

BERR_Code BRAP_MIXER_P_GetMute_isr ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool *          pMute    /* [out] Pointer to memory where the Mute 
                                      status is to be written */            
)
{
    BRAP_MIXER_P_Handle  hMixer;
    BERR_Code  ret = BERR_SUCCESS;
    unsigned int mixerId;
    BRAP_OutputPort eTempOpType;
    
    BDBG_ASSERT (hRap);
    BDBG_ASSERT (pMute);
    
    BDBG_ENTER (BRAP_MIXER_P_GetMute_isr);

    /* check if this output port has been opened */
    if (hRap->hFmm[0]->hOp[eOpType] == NULL)
    {
            BDBG_ERR(("This output port hasnt been opened. Cant mute."));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* If Mai port get the Input to Mai */
    if ( eOpType == BRAP_OutputPort_eMai )
    {
    	eTempOpType = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }
    else
    {
    	eTempOpType = eOpType;
    }

    /* Get the mixer handle */
    ret = BRAP_RM_P_GetMixerForOpPort_isr (hRap->hRm, eTempOpType, &mixerId);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_P_GetMute_isr(): Cant get Mixer ID"));
        return BERR_TRACE (ret);
    }
    hMixer = hRap->hFmm[0]->hMixer[mixerId]; /* TODO: remove hardcoded FMm index */
    if(hMixer == NULL)
    {
        BDBG_ERR(("BRAP_MIXER_P_GetMute_isr: Got a NULL mixer handle"));
        return BERR_TRACE (BERR_NOT_INITIALIZED);
    }

    if (hMixer->bCompress == true )
    {
        BDBG_ERR (("BRAP_MIXER_P_GetMute_isr(): valid only for uncompressed"
                   "data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    
    *pMute = hMixer->bOutputMute;

    BDBG_MSG (("BRAP_MIXER_P_GetMute_isr():" 
               "Mixer Index = %d, eOpType=%d, *pMute=%d", 
                hMixer->uiMixerIndex, eTempOpType, *pMute));

    BDBG_LEAVE (BRAP_MIXER_P_GetMute_isr);
    return ret;
}

BERR_Code BRAP_MIXER_SetOutputVolume ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType,    /* [in] Output Port */
    unsigned int    uiLVolume,  /* [in] Left channel volume in 1/100 DB */
    unsigned int    uiRVolume   /* [in] Right channel volume in 1/100 DB*/
)
{
    BERR_Code ret = BERR_SUCCESS;    
    BRAP_MIXER_P_Handle hMixer;
    BREG_Handle hRegister;    
    uint32_t ui32AttenDb = 0, ui32Delta=0, ui32Temp=0;
    uint32_t ui32RegVal = 0;
    unsigned int uiMixerId;
    BRAP_OutputPort eTempOpType;

    BDBG_ASSERT (hRap);
    BDBG_ENTER (BRAP_MIXER_SetOutputVolume);

    /* check if this output port has been opened */
    if (hRap->hFmm[0]->hOp[eOpType] == NULL)
    {
            BDBG_ERR(("This output port hasnt been opened. Cant set volume."));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* If Mai port get the Input to Mai */
    if ( eOpType == BRAP_OutputPort_eMai )
    {
    	eTempOpType = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }
    else
    {
    	eTempOpType = eOpType;
    }

    /* Get the mixer handle */
    ret = BRAP_RM_P_GetMixerForOpPort (hRap->hRm, eTempOpType, &uiMixerId);

    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_SetOutputVolume(): Cant get Mixer ID"));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    hMixer = hRap->hFmm[0]->hMixer[uiMixerId]; /* TODO: remove hardcoded FMm index */
    if(hMixer == NULL)
    {
        BDBG_ERR(("BRAP_MIXER_SetOutputVolume: Got a NULL mixer handle"));
        return BERR_TRACE (BERR_NOT_INITIALIZED);
    }
    
    if (hMixer->bCompress == true )
    {
        BDBG_ERR(("BRAP_MIXER_SetOutputVolume(): valid only for uncompressed data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }    
    
    BDBG_MSG (("BRAP_MIXER_SetOutputVolume():" 
               "Mixer Index = %d, eOpType=%d, uiLVolume=%d, uiRVolume=%d", 
                hMixer->uiMixerIndex, eTempOpType, uiLVolume, uiRVolume));

    hRegister = hMixer->hRegister;;
   
    /* Calculate register value for Left Channel volume */ 
    ui32AttenDb = uiLVolume/BRAP_MIXER_P_FRACTION_IN_DB;
    
    if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
    {
         uiLVolume = BRAP_MIXER_P_MAX_8000_DB_VOLUME 
                    * BRAP_MIXER_P_FRACTION_IN_DB;
        ui32Temp = 0;
    }
    else
    {
          ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
           ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
           ui32Delta = (ui32Delta * (uiLVolume%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
        ui32Temp -= ui32Delta;
    }
    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        LEFT_VOLUME_LEVEL, 
                        ui32Temp));      
    BDBG_MSG (("BRAP_MIXER_SetOutputVolume(): Left Volume Reg value =0x%x",
                ui32Temp));

    /* Calculate register value for Right Channel volume */ 
    ui32AttenDb = uiRVolume/BRAP_MIXER_P_FRACTION_IN_DB;
    
    if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
    {
         uiRVolume = BRAP_MIXER_P_MAX_8000_DB_VOLUME 
                    * BRAP_MIXER_P_FRACTION_IN_DB;
        ui32Temp=0;
    }
    else
    {
          ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
           ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
           ui32Delta = (ui32Delta * (uiRVolume%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
        ui32Temp -= ui32Delta;
    }
    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME, 
                        RIGHT_VOLUME_LEVEL, 
                        ui32Temp));  
    BDBG_MSG (("BRAP_MIXER_SetOutputVolume(): Left Volume Reg value =0x%x",
                ui32Temp));

    BRAP_Write32 (hRegister, 
                   (hMixer->ui32DpOffset 
                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_VOLUME
                   + hMixer->ui32MixerOffset), 
                  ui32RegVal); 

    hMixer->uiRVolume = uiRVolume;    
    hMixer->uiLVolume = uiLVolume;
    
    BDBG_LEAVE (BRAP_MIXER_SetOutputVolume);
    return ret;
}


BERR_Code BRAP_MIXER_GetOutputVolume ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType,    /* [in] Output Type */
    unsigned int *  pLVolume,   /* [out] Pointer to memory where left 
                                         channel volume should be stored. 
                                         Units: 1/100 DB */
    unsigned int *  pRVolume    /* [out] Pointer to memory where right 
                                         channel volume should be stored. 
                                         Units: 1/100 DB */
)
{
    BERR_Code ret = BERR_SUCCESS;    
    BRAP_MIXER_P_Handle hMixer;
    unsigned int uiMixerId;
    BRAP_OutputPort eTempOpType;

    BDBG_ASSERT (hRap);
    BDBG_ASSERT (pLVolume);
    BDBG_ASSERT (pRVolume);
    BDBG_ENTER (BRAP_MIXER_GetOutputVolume);

    /* check if this output port has been opened */
    if (hRap->hFmm[0]->hOp[eOpType] == NULL)
    {
            BDBG_ERR(("This output port hasnt been opened. Cant get volume."));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }    

    /* If Mai port get the Input to Mai */
    if ( eOpType == BRAP_OutputPort_eMai )
    {
    	eTempOpType = hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }
    else
    {
    	eTempOpType = eOpType;
    }
   
    /* Get the mixer handle */
    ret = BRAP_RM_P_GetMixerForOpPort (hRap->hRm, eTempOpType, &uiMixerId);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_GetOutputVolume(): Cant get Mixer ID"));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    hMixer = hRap->hFmm[0]->hMixer[uiMixerId]; /* TODO: remove hardcoded FMm index */
    if(hMixer == NULL)
    {
        BDBG_ERR(("BRAP_MIXER_GetOutputVolume: Got a NULL mixer handle"));
        return BERR_TRACE (BERR_NOT_INITIALIZED);
    }

    if (hMixer->bCompress == true )
    {
        BDBG_ERR (("BRAP_MIXER_GetOutputVolume(): valid only for uncompressed"
                   "data."));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }    

    *pLVolume = hMixer->uiLVolume;
    *pRVolume = hMixer->uiRVolume;
    
    BDBG_MSG (("BRAP_MIXER_GetOutputVolume():" 
               "Mixer Index = %d, eOpType=%d, *pLVolume=%d DB, *pRVolume=%d DB", 
                hMixer->uiMixerIndex, eTempOpType, *pLVolume, *pRVolume));
    
    
    BDBG_LEAVE (BRAP_MIXER_GetOutputVolume);
    return ret;
}




BERR_Code BRAP_MIXER_SetRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size in 1/100 DB */
)
{
    uint32_t ui32AttenDb = 0, ui32Delta=0, ui32Temp=0;
    uint32_t ui32RegVal = 0;


    BDBG_ASSERT (hRap);
    BDBG_ENTER (BRAP_MIXER_SetRampStepSize);

    /* Calculate register value for Left Channel volume */ 
    ui32AttenDb = uiRampStep/BRAP_MIXER_P_FRACTION_IN_DB;
    
    if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
    {
         uiRampStep = BRAP_MIXER_P_MAX_8000_DB_VOLUME 
                    * BRAP_MIXER_P_FRACTION_IN_DB;
        ui32Temp = 0;
    }
    else
    {
          ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
           ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
           ui32Delta = (ui32Delta * (uiRampStep%BRAP_MIXER_P_FRACTION_IN_DB))
                    /(BRAP_MIXER_P_FRACTION_IN_DB);
        ui32Temp -= ui32Delta;
    }
    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                        SCALE_RAMP_STEP_SIZE, 
                        ui32Temp)); 

    ui32RegVal |= (BCHP_FIELD_DATA (    
                        AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, 
                        VOL_RAMP_STEP_SIZE, 
                        ui32Temp));     

    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP, ui32RegVal);
#if defined ( BCHP_7411_VER ) || ( BCHP_CHIP == 7400 )
    BRAP_Write32 (hRap->hRegister, BCHP_AUD_FMM_DP_CTRL1_FMM_SCALE_VOL_STEP, ui32RegVal);
#endif
    
    BDBG_MSG (("Ramp step size in 1/100 db=%d, written to Reg =0x%x",
                uiRampStep, ui32Temp));           

    hRap->uiRampStepSize = uiRampStep;
    
    BDBG_LEAVE (BRAP_MIXER_SetRampStepSize);
    return BERR_SUCCESS;
}


BERR_Code BRAP_MIXER_GetRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp size should be stored. 
                                         Units: 1/100 DB */
)
{  
    uint32_t ui32RegVal = 0;
    BDBG_ASSERT (hRap);
    BDBG_ENTER (BRAP_MIXER_GetRampStepSize);
         
    *pRampStepSize = hRap->uiRampStepSize;

    ui32RegVal = BRAP_Read32 (hRap->hRegister, 
        BCHP_AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP);  
    
    ui32RegVal = BCHP_GET_FIELD_DATA(ui32RegVal,
						AUD_FMM_DP_CTRL0_FMM_SCALE_VOL_STEP,
						VOL_RAMP_STEP_SIZE);
    
    BDBG_MSG (("Ramp step size in 1/100 db=%d, written to Reg =0x%x",
                 *pRampStepSize, ui32RegVal)); 
    
    BDBG_LEAVE (BRAP_MIXER_GetRampStepSize);
    return BERR_SUCCESS;
}

BERR_Code BRAP_MIXER_SetInputScaling ( 
	BRAP_ChannelHandle  hRapCh,			/* [in] Audio Channel Handle */
	BRAP_OutputPort     eOpType, 		/* [in] Output Type */
	unsigned int        uiScaleValue    /* [in] Scale value at mixer input in dB
										   0 dB means Full value ie "No scaling" */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int i;
	unsigned int uiMixerInput;
	unsigned int uiMixerId;
	unsigned int ui32AttenDb;
	uint32_t ui32Temp;
	uint32_t ui32Delta;
	uint32_t ui32RegVal = 0;
	BRAP_MIXER_P_Handle hMixer = NULL;
#if 0
	BRAP_MIXER_InputStreamParams sParams;
#endif
    BRAP_OutputPort eTempOpType;    
	
	BDBG_ENTER(BRAP_MIXER_SetInputScaling);

	/* Validate input params */
	BDBG_ASSERT(hRapCh);

    BDBG_MSG(("BRAP_MIXER_SetInputScaling:hRapCh=0x%x, hRapCh->uiChannelNo=%d, eOpType=%d, uiScaleValue=%d", 
        hRapCh, hRapCh->uiChannelNo, eOpType, uiScaleValue));

    /* check if this output port has been opened */
    if (hRapCh->hRap->hFmm[0]->hOp[eOpType] == NULL)
    {
            BDBG_ERR(("This output port hasnt been opened. Cant set volume."));
            return BERR_TRACE (BERR_INVALID_PARAMETER);
    }

    /* If Mai port get the Input to Mai */
    if ( eOpType == BRAP_OutputPort_eMai )
    {
    	eTempOpType = hRapCh->hRap->sOutputSettings[BRAP_OutputPort_eMai].uOutputPortSettings.sMaiSettings.eMaiMuxSelector;
    }
    else
    {
    	eTempOpType = eOpType;
    }    

    /* Get the mixer handle */
    ret = BRAP_RM_P_GetMixerForOpPort (hRapCh->hRap->hRm, eTempOpType, &uiMixerId);
    if (ret != BERR_SUCCESS)
    {
        BDBG_ERR(("BRAP_MIXER_SetInputScaling: Cant get Mixer ID"));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }
    BDBG_MSG(("BRAP_MIXER_SetInputScaling: uiMixerId=%d", uiMixerId));  
	/* Get the associated mixer */
    hMixer = hRapCh->hRap->hFmm[0]->hMixer[uiMixerId]; /* TODO: remove hardcoded FMm index */
	if(hMixer == NULL)
    {
        BDBG_ERR(("BRAP_MIXER_SetInputScaling: Got a NULL mixer handle for mixer %d", uiMixerId));
        return BERR_TRACE (BERR_NOT_SUPPORTED);
    }

    BDBG_MSG(("BRAP_MIXER_SetInputScaling: hMixer=0x%x",hMixer));    

	for(i=0; i<BRAP_RM_P_MAX_OP_CHANNEL_PAIRS; i++)
	{
		/* Locate the mixer handle in the sModuleHandles and get associated mixer input */
		if(hMixer == hRapCh->sModuleHandles.hMixer[i]
#ifndef BCHP_7411_VER          
          || hMixer == hRapCh->sCloneOpPathHandles[i][eOpType].hMixer
#ifdef MOTO_PLATFORM
          || hMixer == hRapCh->sSimulPtModuleHandles.hMixer[i]
#endif
#endif          
          )
		{
            if(hMixer == hRapCh->sModuleHandles.hMixer[i])
                uiMixerInput = hRapCh->sModuleHandles.uiMixerInputIndex[i];
#ifndef BCHP_7411_VER             
#ifdef MOTO_PLATFORM
			else if(hMixer == hRapCh->sSimulPtModuleHandles.hMixer[i])
				uiMixerInput = hRapCh->sSimulPtModuleHandles.uiMixerInputIndex[i];
#endif
            else
                uiMixerInput = hRapCh->sCloneOpPathHandles[i][eOpType].uiMixerInputIndex;
#endif

			if (uiMixerInput > (BRAP_RM_P_MAX_MIXER_INPUTS -1))
    		{
    			BDBG_ERR(("BRAP_MIXER_SetInputScaling: Invalid mixer input (%d)",uiMixerInput));
		        return BERR_TRACE (BERR_INVALID_PARAMETER);
    		}
#if 0
		 	sParams = hMixer->sInputParams[uiMixerInput]; 
#endif

		    /* Read the Config register for this mixer input */
		    ui32RegVal = BRAP_Read32 (hMixer->hRegister, 
		                        ( hMixer->ui32DpOffset
		                        + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
		                        + hMixer->ui32MixerOffset 
		                        + (uiMixerInput * 4)));    

		    /* Enable/Disable Scaling and set scale coefficient */
		    ui32RegVal &= ~(BCHP_MASK (
		                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
		                    MIX_SCALE_ENB));        
		    ui32RegVal &= ~(BCHP_MASK (
		                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
		                    MIX_SCALING_COEF));        

#ifndef EMULATION
	        /* Enable scaling and set scaling coefficient */
	        ui32RegVal |= (BCHP_FIELD_ENUM (
	                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
	                        MIX_SCALE_ENB, 
	                        Enable));       
#else
	        /* Disable scaling and set scaling coefficient */
	        ui32RegVal |= (BCHP_FIELD_ENUM (
	                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
	                        MIX_SCALE_ENB, 
	                        Disable));       
#endif                        
		        
	        /* Calculate value */ 
	        ui32AttenDb = uiScaleValue/BRAP_MIXER_P_FRACTION_IN_DB;
	    
	        if (ui32AttenDb >= BRAP_MIXER_P_MAX_8000_DB_VOLUME)
	        {
	            uiScaleValue = BRAP_MIXER_P_MAX_8000_DB_VOLUME
	                                   * BRAP_MIXER_P_FRACTION_IN_DB;
	            ui32Temp=0;
	        }
	        else
	        {
	            ui32Temp = BRAP_Vol8000_DB[ui32AttenDb];   
	            ui32Delta = ui32Temp - BRAP_Vol8000_DB[ui32AttenDb+1];
	            ui32Delta = (ui32Delta 
							* (uiScaleValue%BRAP_MIXER_P_FRACTION_IN_DB))
	                        /(BRAP_MIXER_P_FRACTION_IN_DB);
	            ui32Temp -= ui32Delta;
	        }
         
	        ui32RegVal |= (BCHP_FIELD_DATA (
	                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
	                        MIX_SCALING_COEF, 
	                        ui32Temp));       
	        BDBG_MSG (("BRAP_MIXER_SetInputScaling: Scaling value =%d Reg value =0x%x",
	                uiScaleValue, ui32Temp));
	    		     
		    /* Write to the Config register */
		    BRAP_Write32 (hMixer->hRegister, 
	    	              (hMixer->ui32DpOffset
		                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
		                   + hMixer->ui32MixerOffset 
		                   + (uiMixerInput * 4)), 
		                  ui32RegVal);   

			/* Update the new scaling value */
			hMixer->sInputParams[uiMixerInput].uiScaleValue = uiScaleValue;
			
			/* We are done, so break from the for loop now */
			break;
			
		} /* If mixer handle valid */
	}/* for */

	/* Check if for loop ended without finding a valid mixer handle */
	if (i == BRAP_RM_P_MAX_OP_CHANNEL_PAIRS)
	{
		ret = BERR_NOT_INITIALIZED;
		BDBG_ERR(("BRAP_MIXER_SetInputScaling: Mixer %d does not belong to this channel (hRapCh=0x%x, uiChannelNo=%d)", 
          uiMixerId, hRapCh, hRapCh->uiChannelNo));
	}
	
	BDBG_LEAVE (BRAP_MIXER_SetInputScaling);
	return ret;
}

/***************************************************************************
Summary:
	Sets the SRC for the mixer

Description:
	This routine sets the sampling rate conversion (SRC) ratio for the mixer.

Returns:
	BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_SetSRC ( 
	BRAP_MIXER_P_Handle hMixer, /*[in] Mixer handle */
	BRAP_SRCCH_P_Handle hSrcCh, /*[in] SrcCh handle */
	BAVC_AudioSamplingRate eOutputSR, 	/* [in] output sampling rate */
	BAVC_AudioSamplingRate eInputSR 	/* [in] input sampling rate */
)
{
    BERR_Code ret = BERR_SUCCESS;

    BDBG_ENTER(BRAP_MIXER_P_SetSRC);

    BKNI_EnterCriticalSection();
    ret = BRAP_MIXER_P_SetSRC_isr(hMixer, hSrcCh, eOutputSR, eInputSR);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_MIXER_P_SetSRC);
    return ret;
}


/***************************************************************************
Summary:
	Sets the SRC for the mixer

Description:
    ISR version of BRAP_MIXER_P_SetSRC().
    
Returns:
	BERR_SUCCESS on success else error

See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_SetSRC_isr ( 
	BRAP_MIXER_P_Handle hMixer, /*[in] Mixer handle */
	BRAP_SRCCH_P_Handle hSrcCh, /*[in] SrcCh handle */
	BAVC_AudioSamplingRate eOutputSR, 	/* [in] output sampling rate */
	BAVC_AudioSamplingRate eInputSR 	/* [in] input sampling rate */
)
{
    BERR_Code ret = BERR_SUCCESS;
    unsigned int uiSRIn = 0;
    unsigned int uiSROut = 0;
    unsigned int uiStreamId = 0;
    uint32_t ui32SrcIORatio = 0;
    uint32_t ui32RegVal = 0;
    uint32_t ui32Temp = 0;    


    BDBG_ENTER(BRAP_MIXER_P_SetSRC_isr);
    BDBG_ASSERT(hMixer);
    BDBG_ASSERT(hSrcCh);
    
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

    BDBG_MSG(("\nBRAP_MIXER_P_SetSRC_isr:" "\n\t hMixer = 0x%x"
    		"\n\t hSrcCh = 0x%x" "\n\t eOutputSR = %d"
    		"\n\t eInputSR = %d",hMixer, hSrcCh,eOutputSR,eInputSR));

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
		
	BDBG_LEAVE(BRAP_MIXER_P_SetSRC_isr);
	return ret;
}


/***************************************************************************
Summary:
	Enables/Disables a mixer
Description:
	This routine enables/disalbes the mixer associated to the output port.
	This routine will have an effect only if BRAP_MIXER_P_Start() has already
	been called. In other words, hMixer->eState[uiMixerInput] should be in 
	the running state.
Returns:
	BERR_SUCCESS
See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_EnableMixer ( 
	BRAP_MIXER_P_Handle hMixer,         /* [in] Mixer handle */
	bool				bEnable			/* [in] TRUE: Enable Stream 
                                          		FALSE: Disable */
	)
{
	BERR_Code ret = BERR_SUCCESS;

	BDBG_ENTER(BRAP_MIXER_P_EnableMixer);

    BKNI_EnterCriticalSection();
    ret = BRAP_MIXER_P_EnableMixer_isr (hMixer, bEnable);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BRAP_MIXER_P_EnableMixer);
	return ret;
}

/***************************************************************************
Summary:
	Enables/Disables a mixer

Description:
	This routine enables/disalbes the mixer associated to the output port.
	ISR version of BRAP_MIXER_P_EnableMixer.

Returns:
	BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_EnableMixer_isr ( 
	BRAP_MIXER_P_Handle hMixer,         /* [in] Mixer handle */
	bool				bEnable			/* [in] TRUE: Enable Stream 
                                          		FALSE: Disable */
	)
{
	BERR_Code ret = BERR_SUCCESS;
	unsigned int uiMixerInput;
	uint32_t ui32RegVal = 0;

	BDBG_ENTER(BRAP_MIXER_P_EnableMixer_isr);

	/* Validate input params */
	BDBG_ASSERT(hMixer);
	BDBG_MSG(("BRAP_MIXER_P_EnableMixer_isr: hMixer = %p",hMixer));

	for(uiMixerInput = 0; uiMixerInput < BRAP_RM_P_MAX_MIXER_INPUTS; uiMixerInput++)
	{
		if (hMixer->eState[uiMixerInput] == BRAP_MIXER_P_State_eRunning)
		{
			ui32RegVal = BRAP_Read32_isr(hMixer->hRegister, 
			                     	( hMixer->ui32DpOffset
			                        + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
			                        + hMixer->ui32MixerOffset
			                        + (uiMixerInput * 4))); 

			ui32RegVal &= ~(BCHP_MASK (
	                    AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
	                    MIX_ENABLE));        

			if(bEnable)
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (
			                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
			                        MIX_ENABLE, Enable));   
			}
			else
			{
				ui32RegVal |= (BCHP_FIELD_ENUM (
			                        AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG, 
			                        MIX_ENABLE, Disable)); 
			}
			 
			BRAP_Write32_isr(hMixer->hRegister, 
		    	              (hMixer->ui32DpOffset
			                   + BCHP_AUD_FMM_DP_CTRL0_MS_CLIENT0_MIX1_CONFIG 
			                   + hMixer->ui32MixerOffset
			                   + (uiMixerInput * 4)),
			                   ui32RegVal); 
		} /* if */
	}/* for */

	BDBG_LEAVE(BRAP_MIXER_P_EnableMixer_isr);
	return ret;
}


/* End of File */
