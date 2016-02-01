/***************************************************************************
*     Copyright (c) 2004-2008, Broadcom Corporation
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
*		This file contains all private macros, enums, structures and 
*		functions privately used inside the hifidac module of the 
*		Raptor PI.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef _BRAP_HIFIDAC_PRIV_H__ /*{{{*/
#define _BRAP_HIFIDAC_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	BAVC_AudioSamplingRate   eSamplingRate;
	uint32_t			uiNumerator;
	uint32_t			uiDenominator;
	uint32_t			uiSampleInc;
	uint32_t			uiPhaseInc;
}BRAP_OP_P_DacRateMangerSR;

#if(BCHP_CHIP != 7601)

#if (BCHP_CHIP == 7440)
BERR_Code
BRAP_OP_P_DacOpenUnMute (
    BRAP_OP_P_Handle      hOp               /*[in] Output port handle */
);
#endif

BERR_Code
BRAP_OP_P_DacOpen (
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
    const BRAP_OP_P_DacSettings * pSettings  /*[in] Open time parameters */
);

/***************************************************************************
Summary:
    Programs the on-the-fly changeable parameters to the HW registers 
    for the Dac ie BCHP_HIFIDAC_CTRL0_CONFIG

Description:
    This PI takes a BRAP_OP_P_DacSettings structure.
    Based on this it programs the register BCHP_HIFIDAC_CTRL0_CONFIG
    It then saves the BRAP_OP_P_DacSettings structure in hOp.
    At present, the  on-the-fly changeable parameters for Dac are 
    eChannelOrder and eMuteType
    
Returns:
    BERR_SUCCESS on success
    
See Also:
    BRAP_OP_P_DacStart(), BRAP_OP_P_DacOpen(), BRAP_SetOutputConfig()
**************************************************************************/
BERR_Code BRAP_OP_P_DacHWConfig (
    BRAP_OP_P_Handle        hOp,              /*[in] Output port handle */
    const BRAP_OP_P_DacSettings * pSettings /*[in] Parameters */
);

BERR_Code
BRAP_OP_P_DacStart (
    BRAP_OP_P_Handle        hOp,             /*[in] Output port handle */
    const BRAP_OP_P_DacParams * pParams    /*[in] Start time parameters */
);

BERR_Code
BRAP_OP_P_DacStop (
    BRAP_OP_P_Handle        hOp              /*[in] Output port handle */
);


/***************************************************************************
Summary:
	Mutes/Unmutes the DAC output

Description:
	This API is required to mute/unmute the output of the specified DAC.
	PI maintains this state upon channel change.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_P_DacGetMute
	
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetMute (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool bMute						/* [in] True = Mute; false = unmute*/
);

BERR_Code
BRAP_OP_P_DacSetMute_isr (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool bMute						/* [in] True = Mute; false = unmute*/
);
#endif
/***************************************************************************
Summary:
	Retrieves the mute status

Description:
	This API is required to get mute/unmute status of the
	output of the specified DAC.

Returns:
	BERR_SUCCESS 

See Also:
	BRAP_OP_P_DacSetMute
	
****************************************************************************/
BERR_Code
BRAP_OP_P_DacGetMute (
    BRAP_Handle     hRap,			/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,	/* [in] Output Type */
    bool *pbMute					/* [out]True = Mute; false = unmute*/
);
#if(BCHP_CHIP != 7601)
/***************************************************************************
Summary:
	Sets the sample rate for the given DAC output

Description:
	This API is required to set the sampling rate of DAC.
	This is mainly used for playback of PCM buffers from
	memory. In decode mode the F/W sets it according to
	the frequency.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetSampleRate(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BAVC_AudioSamplingRate eSamplingRate	/* [in]The sampling rate to be programmed */
);

/***************************************************************************
Summary:
	Sets the sample rate for the given DAC output

Description:
    ISR version of BRAP_OP_P_DacSetSampleRate
    
Returns:
	BERR_SUCCESS 

See Also:
		
****************************************************************************/
BERR_Code
BRAP_OP_P_DacSetSampleRate_isr(
    BRAP_Handle     hRap,					/* [in] Audio Device Handle */
    BRAP_OutputPort     eOpType,			/* [in] Output Type */
    BAVC_AudioSamplingRate eSamplingRate	/* [in]The sampling rate to be programmed */
);

/***************************************************************************
Summary:
	Sets the timebase for the given DAC output

Description:
	This API is required to set the timebase of DAC.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_OP_P_DacSetTimebase(
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
	BAVC_Timebase		  eTimebase			 /*[in] time base to program */
);

/***************************************************************************
Summary:
	Sets the timebase for the given DAC output

Description:
    ISR version of BRAP_OP_P_DacSetTimebase.

Returns:
	BERR_SUCCESS 

See Also:
	
****************************************************************************/
BERR_Code BRAP_OP_P_DacSetTimebase_isr(
    BRAP_OP_P_Handle      hOp,               /*[in] Output port handle */
	BAVC_Timebase		  eTimebase			 /*[in] time base to program */
);
#endif
#if ( BCHP_CHIP == 3563 )
/***************************************************************************
Summary:
	Sets the 5 band coeff coeffs for the given DAC output
****************************************************************************/
void BRAP_P_PogramDacEqualizerCoeff (
	BRAP_Handle     				hRap,			/* [in] Audio Device Handle */
	BRAP_OutputPort				eOutputPort,		/* [in] Output port to be set, currently being DAC or I2S */
	BRAP_P_Equalizer_GainCoeff	*pEqualizerCoeff	/* [in] Equalizer coeff */
);

/***************************************************************************
Summary:
	Sets the Tone control coeffs for the given DAC output

****************************************************************************/
BERR_Code BRAP_OP_P_DacSetToneCtrlCoeff(
	BRAP_Handle     				hRap,			/* [in] Audio Device Handle */
	BRAP_OutputPort				eOutputPort,		/* [in] Output port to be set, currently being DAC or I2S */
	BRAP_Equalizer_ToneControl		*psToneControl	/* [in] structure with parameters for Tone Control */
);
#endif

#ifdef __cplusplus
}
#endif

#endif /*}}} #ifndef _BRAP_HIFIDAC_PRIV_H__ */

