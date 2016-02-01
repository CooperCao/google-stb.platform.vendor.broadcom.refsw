/***************************************************************************
*     Copyright (c) 2004-2007, Broadcom Corporation
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
*   Module name: MIXER
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the Mixer abstraction, which are internal ie NOT
*   exposed to the application developer. These can be used only by the 
*   Audio Manager and other FMM submodules. 
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/


#ifndef _BRAP_MIXER_PRIV_H_
#define _BRAP_MIXER_PRIV_H_

#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
    This enumeration defines Mixer state.

***************************************************************************/
typedef enum BRAP_MIXER_P_State
{
    BRAP_MIXER_P_State_eStopped = 0,      /* Stopped */
    BRAP_MIXER_P_State_eRunning,          /* Running */
    BRAP_MIXER_P_State_eInvalid           /* Invalid/Unusable */
}BRAP_MIXER_P_State;


/***************************************************************************
Summary:
    This enumeration lists various permissible Stream Resolutions at the Mixer.

***************************************************************************/
typedef enum BRAP_MIXER_P_StreamRes
{
	BRAP_MIXER_P_StreamRes_e16=0,   /* 16 bit resolution */
	BRAP_MIXER_P_StreamRes_e17,     /* 17 bit resolution */
	BRAP_MIXER_P_StreamRes_e18,     /* 18 bit resolution */
	BRAP_MIXER_P_StreamRes_e19,     /* 19 bit resolution */
	BRAP_MIXER_P_StreamRes_e20,     /* 20 bit resolution */
	BRAP_MIXER_P_StreamRes_e21,     /* 21 bit resolution */
	BRAP_MIXER_P_StreamRes_e22,     /* 22 bit resolution */
	BRAP_MIXER_P_StreamRes_e23,     /* 23 bit resolution */
	BRAP_MIXER_P_StreamRes_e24      /* 24 bit resolution */
}BRAP_MIXER_P_StreamRes;


/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Opening the Mixer.

Description:
    This lists all the parameters required to open the Mixer, 
    including both internal and external parameters.
    Note that the internal parameters are not exposed to the application.
    Currently no extra external settings are required.

See Also:

***************************************************************************/
typedef struct BRAP_MIXER_P_Settings
{
    unsigned int  uiMixerInput; /* Id of the mixer input to which this source 
                                   channel is connected */    
	bool			bDisableRampUp;	/* DVD kind of application requires
									    disabling volume ramp-up as well
									    as audio ramp-up at start.
									    TRUE = Disable volume ramp-up
									    and audio ramp-up at start
									    FALSE = Enable volume ramp-up
									    and audio ramp-up at start */
} BRAP_MIXER_P_Settings;

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Starting the Mixer.
    
Description:
    This lists all the parameters required to start the Mixer, 
    including both internal and external parameters.
    Note that the internal parameters are not exposed to the application.

See Also:

***************************************************************************/
typedef struct BRAP_MIXER_P_Params
{
    BRAP_MIXER_Params    sExtParams;   /* Parameters provided by the application */
    unsigned int         uiSrcChId;    /* Id of the source channel to be connected */
    bool                 bCompress;    /* TRUE: The input is compressed data
                                          FALSE: The input is uncompressed */
     
    unsigned int         uiStreamRes;  /* Stream Resolution (bits/sample) 
                                          at the mixer o/p */
   
}BRAP_MIXER_P_Params; 

/***************************************************************************
Summary:
    Abstraction of a Mixer 
    
Description:
    It contains all the information required to handle the Mixer.
    Particularly, it stores the Mixer index, handles for all required chip 
    related information, parent FMM handle, offset required to access 
    different Mixers etc

    Mute can be done at 2 places:
    i) At source channel o/p: This will mute the source channel o/p itself, 
       so if this source channel goes to multiple mixer inputs all of them 
       will be muted. Handled in the Source Channel submodule.
   ii) At mixer output: This will mute the mixer ouput itself.
       Handled in the Mixer submodule.
 

See Also:
***************************************************************************/
typedef struct BRAP_MIXER_P_Object
{
    BCHP_Handle           hChip;       /* Handle to chip object */
    BREG_Handle           hRegister;   /* Handle to register object */
    BMEM_Handle           hHeap;       /* Handle to memory object */
    BINT_Handle           hInt;        /* Handle to interrupt object */

    BRAP_FMM_P_Handle     hFmm;        /* Parent FMM handle */
    
    unsigned int          uiDpIndex;      /* Parent DP Index */
    unsigned int          uiMixerIndex;   /* Mixer Index */
    
    uint32_t              ui32DpOffset;     /* DP Register Offset from start of 
                                             AUD_FMM_DP_CTRL0_STRM_ENA */
    uint32_t              ui32MixerOffset;  /* Offset of register of current mixer 
                                             from the corresponding register of 
                                             first mixer */
    
    BRAP_MIXER_P_State    eState[BRAP_RM_P_MAX_MIXER_INPUTS]; /* State of the 
                                             Various Mixer Input streams */
    
    bool                  bOutputMute;    /* Flag to indicate if this mixer 
                                             output has been muted. This bit 
                                             is valid only for uncompressed 
                                             data */
    bool                  bCompress;      /* TRUE: The input is compressed data
                                             FALSE: The input is uncompressed */ 
    unsigned int          uiLVolume;      /* Left channel volume level in 
                                             1/100dB */    
    unsigned int          uiRVolume;      /* Right channel volume level in 
                                             1/100 dB */     
    unsigned int          uiStreamRes;    /* Stream Resolution (bits/sample) 
                                             at the mixer o/p */   
    
    unsigned int          uiMixerInput[BRAP_RM_P_MAX_MIXER_INPUTS]; /* Ids of   
                                             the source channels connected at 
                                             the various mixer inputs */    
    
    BRAP_MIXER_InputStreamParams sInputParams[BRAP_RM_P_MAX_MIXER_INPUTS];
                                          /* Mixer related parameters for each 
                                             input stream. These parameters are 
                                             obtained from _Start(). Save here 
                                             for all input streams.*/
	bool			bDisableRampUp;	/* DVD kind of application requires
									    disabling volume ramp-up as well
									    as audio ramp-up at start.
									    TRUE = Disable volume ramp-up
									    and audio ramp-up at start
									    FALSE = Enable volume ramp-up
									    and audio ramp-up at start */

}BRAP_MIXER_P_Object;

#if 0 /* TBD later if reqd */

typedef struct BRAP_MIXER_P_SR2DecHZ
{
    BAVC_AudioSamplingRate   eSR; 
    unsigned int             uiHz;
}BRAP_MIXER_P_SR2DecHZ;
#endif



/***************************************************************************
Summary:
    Opens a Mixer.

Description:
    Initializes the Mixer and returns a Mixer handle.    
    The handle can then be used for all other Mixer function calls.
    Each call to this function configures only one Mixer input.
    It should be called multiple times in order to configure
    different mixer inputs.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_Close, BRAP_MIXER_P_GetDefaultSettings.
**************************************************************************/
BERR_Code BRAP_MIXER_P_Open (
    BRAP_FMM_P_Handle     hFmm,       /* [in] Parent FMM handle */
    BRAP_MIXER_P_Handle * phMixer,    /* [out] Pointer to Mixer handle.
                                         If this Mixer has been opened before, 
                                         this will return the previously created
                                         handle, else, it will return the newly 
                                         created handle */
    unsigned int          uiIndex,    /* [in] Mixer index */
    const BRAP_MIXER_P_Settings * pSettings  /* [in] Mixer settings */
);


/***************************************************************************
Summary:
    Releases all the resources associated with this Mixer and frees 
    the handles.

Description:
    Each call to this function 'closes' only one Mixer input.
    It should be called multiple times in order to close
    different mixer inputs. When all Mixer inputs have been closed, 
    this function frees the Mixer handle.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_Open
**************************************************************************/
BERR_Code BRAP_MIXER_P_Close ( 
    BRAP_MIXER_P_Handle hMixer,       /* [in] Mixer handle */
    unsigned int        uiMixerInput  /* [in] Id of the mixer input 
                                          associated with this audio stream*/
);


/***************************************************************************
Summary:
    Starts ie "Enables" a particular Mixer input.

Description:
    Has to be called mutliple times for various mixer inputs.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_Stop
    
**************************************************************************/
BERR_Code BRAP_MIXER_P_Start ( 
    BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput,  /* [in] Id of the mixer input 
                                            associated with this audio stream*/
    const BRAP_MIXER_P_Params * pParams  /* [in] Pointer to start
                                            parameters for this stream */ 
);


/***************************************************************************
Summary:
    Stops ie "Disables" a particular Mixer input.

Description:
    Has to be called mutliple times for various mixer inputs.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_Start
    
**************************************************************************/
BERR_Code BRAP_MIXER_P_Stop ( 
    BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput   /* [in] Id of the mixer input 
                                            associated with this audio stream*/
);

/***************************************************************************
Summary:
    Returns default values for Source Channel Open time settings.

Description:
    For settings that the system cannot assign default values to, 
    an invalid value is returned. Note that the default settings are common
    for all Mixers.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_GetDefaultParams.
**************************************************************************/
BERR_Code 
BRAP_MIXER_P_GetDefaultSettings ( 
    BRAP_MIXER_P_Settings  * pDefSettings   /*[out] Pointer to memory where default
                                              settings should be written */    
);

/***************************************************************************
Summary:
    Returns default values for Source Channel Start time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all Mixers.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_P_GetDefaultSettings.
**************************************************************************/
BERR_Code 
BRAP_MIXER_P_GetDefaultParams ( 
    BRAP_MIXER_P_Params  * pDefParams   /*[out] Pointer to memory where default
                                        settings should be written */    
);

/***************************************************************************
Summary:
    Returns current values for Mixer Start time parameters.

Description:
	Returns current Mixer parameters stored in mixer handle.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
**************************************************************************/
BERR_Code 
BRAP_MIXER_P_GetCurrentParams ( 
	BRAP_MIXER_P_Handle   hMixer,        /* [in] Mixer handle */
    unsigned int          uiMixerInput,  /* [in] Id of the mixer input 
                                            associated with this audio stream*/
    BRAP_MIXER_P_Params  * pCurParams   /*[out] Pointer to memory where current 
                                        params should be written */    
);

/***************************************************************************
Summary:
    Mute/unmute Mixer Output for uncompressed data for ISR context

Description:
    This function mutes or unmutes output of Mixer.
    It can be used only for uncompressed data.
    These mutes/umutes are ramp up/down mutes by hardware, so
    there is no need to bring the volume down when using these
    mute controls.  

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetMute, BRAP_MIXER_GetMute, BRAP_MIXER_P_GetMute_isr.
    
**************************************************************************/
BERR_Code BRAP_MIXER_P_SetMute_isr ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool            bMute    /* [in] TRUE: Mute mixer output
                                     FALSE: UnMute mixer output */
);

/***************************************************************************
Summary:
    Gets whether output of the mixer is muted or not for ISR context.

Description:
    It can be used only for uncompressed data.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetMute, BRAP_MIXER_GetMute, BRAP_MIXER_P_SetMute_isr.
    
**************************************************************************/
BERR_Code BRAP_MIXER_P_GetMute_isr ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool *          pMute    /* [out] Pointer to memory where the Mute 
                                      status is to be written */            
);

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
);

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
);

/***************************************************************************
Summary:
	Sets the SRC for the mixer

Description:
	This routine sets the scaling rate conversion (SRC) ratio for the mixer.

Returns:
	BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_SetSRC ( 
	BRAP_MIXER_P_Handle hMixer, /*[in] Mixer handle */
	BRAP_SRCCH_P_Handle hSrcCh, /*[in] SrcCh handle */
	BAVC_AudioSamplingRate eOutputSR, 	/* [in] output sampling rate */
	BAVC_AudioSamplingRate eInputSR 	/* [in] input sampling rate */
);


/***************************************************************************
Summary:
	Sets the SRC for the mixer

Description:
    ISR version of BRAP_MIXER_P_SetSRC().
    
Returns:
	BERR_SUCCESS

See Also:

****************************************************************************/
BERR_Code BRAP_MIXER_P_SetSRC_isr ( 
	BRAP_MIXER_P_Handle hMixer, /*[in] Mixer handle */
	BRAP_SRCCH_P_Handle hSrcCh, /*[in] SrcCh handle */
	BAVC_AudioSamplingRate eOutputSR, 	/* [in] output sampling rate */
	BAVC_AudioSamplingRate eInputSR 	/* [in] input sampling rate */
);
#ifdef __cplusplus
}
#endif


#endif /* _BRAP_MIXER_PRIV_H_ */
/* End of File */
