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
*   prototypes for the Mixer abstraction, which are exposed to the 
*   application developer.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_MIXER_H_
#define _BRAP_MIXER_H_     

#include "brap.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
Summary:
    This enumeration defines various mixer input Audio Modes

***************************************************************************/
typedef enum BRAP_MIXER_InputAudioMode
{
    
    BRAP_MIXER_Input_eStereo=0, /* Mix stereo from this source channel at mixer 
                                   input */
    BRAP_MIXER_Input_eLeftMono, /* Mix Left Mono from this source channel at
                                   mixer input */ 
    BRAP_MIXER_Input_eRightMono /* Mix Right Mono from this source channel at 
                                   mixer input */ 
}BRAP_MIXER_InputAudioMode;


/***************************************************************************
Summary:
    This structure lists input stream parameters required by the Mixer.
    It has to be passed by the application to start the Mixer.

Description:
    
See Also:

***************************************************************************/
typedef struct BRAP_MIXER_InputStreamParams
{
    
    BRAP_MIXER_InputAudioMode eMixerInputAudioMode; /* Which Audio mode 
                                                    (Stereo/LeftMono/RightMono)
                                                    of this stream to use for
                                                    Mixing.*/
    
    unsigned int        uiScaleValue;     /* Scale value at mixer input
                                             for this stream in dB.
                                             0 dB means Full value ie "No scaling" */

} BRAP_MIXER_InputStreamParams;


/***************************************************************************
Summary:
    Parameters to be passed by Application on Starting the Mixer.
    
Description:
    Contains all parameters related to the input streams. Note that the Mixer
    has BRAP_MIXER_INPUTS possible input sources and we need to store
    information for each.
***************************************************************************/
typedef struct BRAP_MIXER_Params
{

    BRAP_MIXER_InputStreamParams sInputParams; /* Mixer related parameters for 
                                                  the input stream related to 
                                                  current channel */
} BRAP_MIXER_Params;




/***************************************************************************
Summary:
    Sets the Mixer output volume for left and right channels

Description:
    This function sets the Mixer output left and right channel volume.
    Mixer output  volume control is linear in hardware, ranging from 0
    min to 8000 max volume.  This API has done a mapping
    from this linear range to 1/100 DB.  This function 
    gets values in 1/100 DB from 0 (max) to 9000 1/100 DB (min),
    and for all values above 9000 the volume will be set to 0 linear
    or 90 DB.  
    It can be used only for uncompressed data.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_GetOutputVolume()
    BRAP_OP_SetDacVolume()
    BRAP_OP_GetDacVolume()
    
**************************************************************************/
BERR_Code BRAP_MIXER_SetOutputVolume ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort eOpType,    /* [in] Output Port */
    unsigned int    uiLVolume,  /* [in] Left channel volume in 1/100 DB */
    unsigned int    uiRVolume   /* [in] Right channel volume in 1/100 DB*/
);


/***************************************************************************
Summary:
    Gets the Mixer output volume for left and right channels

Description:
    This function returns left and right volume channel settings
    for Mixer output. Values are returned in 1/100 DB.
    It can be used only for uncompressed data.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetOutputVolume()
    BRAP_OP_SetDacVolume()
    BRAP_OP_GetDacVolume()   
    
**************************************************************************/
BERR_Code BRAP_MIXER_GetOutputVolume( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,    /* [in] Output Type */
    unsigned int *  pLVolume,   /* [out] Pointer to memory where left 
                                         channel volume should be stored. 
                                         Units: 1/100 DB */
    unsigned int *  pRVolume    /* [out] Pointer to memory where right 
                                         channel volume should be stored. 
                                         Units: 1/100 DB */
);


/***************************************************************************
Summary:
    Mute/unmute Mixer Output for uncompressed data

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
    BRAP_MIXER_GetMute(), BRAP_OP_SetMute(), BRAP_OP_GetMute()
    
**************************************************************************/
BERR_Code BRAP_MIXER_SetMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool            bMute    /* [in] TRUE: Mute mixer output
                                      FALSE: UnMute mixer output */
);

/***************************************************************************
Summary:
    Gets whether output of the mixer is muted or not.

Description:
    It can be used only for uncompressed data.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetMute(), BRAP_OP_SetMute(), BRAP_OP_GetMute()
    
**************************************************************************/
BERR_Code BRAP_MIXER_GetMute ( 
    BRAP_Handle     hRap,    /* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType, /* [in] Output Type */
    bool *          pMute    /* [out] Pointer to memory where the Mute 
                                      status is to be written */            
);

/***************************************************************************
Summary:
	Controls volume at the mixer input

Description:
	This API controls the scaling coefficient (volume control at mixer input) 
	for the playback input.

    Scaling coefficient is linear in hardware, ranging from 0
    min to 8000 max volume.  This API has done a mapping
    from this linear range to 1/100 DB.  This function 
    gets values in 1/100 DB from 0 (max) to 9000 1/100 DB (min),
    and for all values above 9000 the volume will be set to 0 linear
    or 90 DB.  

    It can be used only for uncompressed data.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetOutputVolume()
****************************************************************************/
BERR_Code BRAP_MIXER_SetInputScaling ( 
	BRAP_ChannelHandle  hRapCh,			/* [in] Audio Channel Handle */
	BRAP_OutputPort     eOpType, 		/* [in] Output Type */
	unsigned int        uiScaleValue    /* [in] Scale value at mixer input in dB
										   0 dB means Full value ie "No scaling" */
	);



/***************************************************************************
Summary:
    Sets the ramping step size for scale and volume change for all output ports.

Description:
    This step size represent the volume/scale change per FS. 

    Scale/Volume Ramp step size control is linear in hardware, ranging from 0
    min to 0x8000 max volume.  This API has done a mapping from this linear 
    range to 1/100 DB.  This function gets values in 1/100 DB from 0 (max) to 
    9000 1/100 DB (min), and for all values above 9000 the step size 
    will be set to 0 linear or 90 DB.  

    Please note: Ramp Step size is common to all output ports. It cannot be 
    changed on a per-output-port basis.

    At device open, the Ramp Step is set to a default of 36 dB => transitioning 
    from full volume (ie 0x8000) to muted (ie 0x0) in 64 fs.
    If the user wants to change this default, they need to explcitly call this 
    PI after device open.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_GetRampStepSize().
    
**************************************************************************/

BERR_Code BRAP_MIXER_SetRampStepSize ( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int    uiRampStep  /* [in] Ramp Step size in 1/100 DB */
);


/***************************************************************************
Summary:
    Gets the ramping step size for scale and volume change 

Description:
    This function returns ramping step size for scale and volume change.
    Values are returned in 1/100 DB.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_MIXER_SetRampStepSize().
    
**************************************************************************/
BERR_Code BRAP_MIXER_GetRampStepSize( 
    BRAP_Handle     hRap,       /* [in] Audio Device Handle */
    unsigned int *  pRampStepSize /* [out] Pointer to memory where ramp size should be stored. 
                                         Units: 1/100 DB */
);

#ifdef __cplusplus
}
#endif

#endif /* _BRAP_MIXER_H_ */


/* End of File */

