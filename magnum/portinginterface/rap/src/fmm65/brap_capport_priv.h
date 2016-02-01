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
* Module Description:
*   Module name: CAPPORT
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the Capture Port abstraction, which are internal ie NOT 
*   exposed to the application developer. These can be used only by the Audio
*   Manager and other FMM submodules. It covers the functionality of the 
*   Capture Hardware Block . This object is only used by a PCM Capture Channel
*   
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_CAPPORT_PRIV_H_
#define _BRAP_CAPPORT_PRIV_H_

#include "brap.h"
#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Opening a Capture Port.
    Note that this includes both public and private settings.

Description:
    No settings at present.
See Also:
    BRAP_CAPPORT_P_Open
***************************************************************************/
typedef struct BRAP_CAPPORT_P_Settings
{
    int tbd;
}BRAP_CAPPORT_P_Settings;

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Starting a Capture Port.
    Note that this includes both public and private parameters.

See Also:
    BRAP_CAPPORT_P_Start
***************************************************************************/

typedef struct BRAP_CAPPORT_P_Params
{
    unsigned int            uiInputFciId;
    BRAP_InputBitsPerSample	eInputBitsPerSample;
    unsigned int            uiGrpId;
    unsigned int            uiFsTmgSrc;     /* FS Timing Source */
    BRAP_OP_Pll 			ePll;           /* PLL to be associated */
    BAVC_AudioSamplingRate  eSamplingRate;  /* Sampling rate of the data flowing 
                                               through internal CapPort */
}BRAP_CAPPORT_P_Params;

/***************************************************************************
Summary:
    Abstraction of a Capture Port 
    
Description:
    It contains all the information required to handle the Capture Port
    Particularly, it stores the type, handles for all required chip 
    related information, parent FMM handle, state etc

See Also:
    
***************************************************************************/
typedef struct BRAP_CAPPORT_P_Object
{
    BRAP_CAPPORT_P_Settings sSettings;  /* Capture Port settings provided 
                                           during Open() */
    BRAP_CAPPORT_P_Params   sParams;    /* Capture Port settings  
                                           provided during Start() */
    BCHP_Handle             hChip;      /* Handle to chip object */
    BREG_Handle             hRegister;  /* Handle to register object */
    BMEM_Handle             hHeap;      /* Handle to memory object */
    BINT_Handle             hInt;       /* Handle to interrupt object */
    BRAP_FMM_P_Handle       hFmm;       /* Parent FMM handle */
    BRAP_CapInputPort       eCapPort;   /* Capture Port Type */
    unsigned int            uiOpenCnt;  /* No. of open audio channels currently
                                           routed to this capture port */
    unsigned int            uiStartCnt; /* No. of active audio channels currently
                                           routed to this capture port */
}BRAP_CAPPORT_P_Object;

/***************************************************************************
Summary:
    Opens a Capture port
    
Description:
    Initializes the Capture port and returns a Capture Port handle.The handle 
    can then be used for all other function calls.

Returns:
    BERR_SUCCESS else error

See Also:
    BRAP_CAPPORT_P_Close
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Open (
    BRAP_FMM_P_Handle           hFmm,           /* [in] Parent FMM handle */
    BRAP_CAPPORT_P_Handle       *phCapPort,     /* [out] Pointer to Capture Port handle */
    BRAP_CapInputPort           eCapPort,       /* [in] Capture Port index */
    const BRAP_CAPPORT_P_Settings *pSettings      /* [in] Capture Port settings */
);

/***************************************************************************
Summary:
    Releases all the resources associated with this Capture port and frees 
    the handles.
    
Description:

Returns:
    BERR_SUCCESS else error

See Also:
BRAP_CAPPORT_P_Open
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Close ( 
    BRAP_CAPPORT_P_Handle   hCapPort      /* [in] Capture Port Handle */
);

/***************************************************************************
Summary:
    Enables a Capture Port.

Description:
    This function should be called for enabling the capture port to get the 
    date from its input.
    
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_CAPPORT_P_Stop, BRAP_CAPPORT_P_HWConfig
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Start ( 
	BRAP_CAPPORT_P_Handle       hCapPort,   /* [in] Capture Port Handle */
    const BRAP_CAPPORT_P_Params *pParams    /* [in] Pointer to start parameters 
                                               for this Capture Port */ 
);

/***************************************************************************
Summary:
    Disables the Capture port.

Description:
    This function disables the capture port.
Returns:
    BERR_Success else Error
See Also:
    BRAP_CAPPORT_P_Start
**************************************************************************/
BERR_Code BRAP_CAPPORT_P_Stop ( 
    BRAP_CAPPORT_P_Handle   hCapPort    /* [in] Capture Port Handle */
);

/***************************************************************************
Summary:
    Returns the Default Params for Capture Port.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned. Note that the default parameters are common
    for all Capture Ports.
    
Returns:
    BERR_SUCCESS else error
    
See Also:

**************************************************************************/
BERR_Code 
BRAP_CAPPORT_P_GetDefaultParams ( 
    BRAP_CAPPORT_P_Params    *pDefParams    /* Pointer to memory where default
                                               settings should be written */    
);

#if ((BRAP_3548_FAMILY == 1) )

typedef struct BRAP_P_RfAudioRateMangerSR
{
	BAVC_AudioSamplingRate   eSamplingRate;
	uint32_t			ui32Numerator;
	uint32_t			ui32Denominator;
	uint32_t			ui32SampleInc;
	uint32_t			ui32PhaseInc;
}BRAP_P_RfAudioRateMangerSR;

void BRAP_RFAUDIO_P_SapXMute_isr (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);

BERR_Code BRAP_RFAUDIO_P_SetOutputSampleRate ( 
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);

BERR_Code BRAP_RFAUDIO_P_InitDecoder (
	BRAP_Handle	hRap		/* [in] RAP Handle */
);

BERR_Code BRAP_RFAUDIO_P_SetSettings (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [in] Pointer to Capture Port handle */
);

BERR_Code BRAP_RFAUDIO_P_Start (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);

BERR_Code BRAP_RFAUDIO_P_Stop (
    BRAP_CAPPORT_P_Handle       phCapPort     /* [out] Pointer to Capture Port handle */
);   

BERR_Code BRAP_RFAUDIO_P_GetStatus (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioStatus  *sStatus			   /* [out] Status of the channel */
);

BERR_Code BRAP_RFAUDIO_P_GetEnvelopeFreqAmpl (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t  *ui32EnvelopeFreqAmpl			/* [out] Status of the channel */
);

BERR_Code BRAP_RFAUDIO_P_SetMute (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    bool						bMute		/* [out ] Mute Enable
    											true = mute
    											false = un-mute */
);   

BERR_Code BRAP_RFAUDIO_P_GetMute (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    bool *bMute							/* [out] Mute status */
);

BERR_Code BRAP_RFAUDIO_P_SetBtscOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioBtscOutputMode		eOutputMode	 /* [in] BTSC Output Mode */
);

BERR_Code BRAP_RFAUDIO_P_GetBtscOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioBtscOutputMode		*eOutputMode	 /* [out] BTSC Output Mode */
);

BERR_Code BRAP_RFAUDIO_P_SetEiajOutputMode (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioEiajOutputMode		eOutputMode	 /* [in] Eiaj Output Mode */
);

BERR_Code BRAP_RFAUDIO_P_GetEiajOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioEiajOutputMode		*eOutputMode	 /* [out] Eiaj Output Mode */
);

BERR_Code BRAP_RFAUDIO_P_SetKoreaA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioKoreaA2OutputMode  eOutputMode	  /* [in] Korea A2 Output mode */
);

BERR_Code BRAP_RFAUDIO_P_GetKoreaA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioKoreaA2OutputMode  *eOutputMode	/* [out] Korea A2 Output mode */
);

BERR_Code BRAP_RFAUDIO_P_SetNicamOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioNicamOutputMode  eOutputMode	  /* [in] Nicam Output mode */
);

BERR_Code BRAP_RFAUDIO_P_GetNicamOutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioNicamOutputMode  *eOutputMode	/* [out] Nicam Output mode */
);

BERR_Code BRAP_RFAUDIO_P_SetPalA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioPalA2OutputMode  eOutputMode	  /* [in] PAL-A2 Output mode */
);

BERR_Code BRAP_RFAUDIO_P_GetPalA2OutputMode (
	BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
	BRAP_RfAudioPalA2OutputMode  *eOutputMode	/* [out] PAL-A2 Output mode */
);

BERR_Code BRAP_RFAUDIO_P_SetStereoConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    const BRAP_RfAudioStereoConfig	*psStereoCfg	/* [in]  Stereo Config */
);

BERR_Code BRAP_RFAUDIO_P_GetStereoConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioStereoConfig		*psStereoCfg	/* [out] Stereo Config */
);

BERR_Code BRAP_RFAUDIO_P_SetSapConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    const BRAP_RfAudioSapConfig	*psSapConfig		/* [in] SAP config */
);

BERR_Code BRAP_RFAUDIO_P_GetSapConfig (
    BRAP_CAPPORT_P_Handle       phCapPort,     /* [in] Pointer to Capture Port handle */
    BRAP_RfAudioSapConfig			*psSapConfig		/* [out] SAP config */
);

BERR_Code BRAP_RFAUDIO_P_SetFirmwareReg ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t					uRegNum,	   /* [in] Reg number 1, 2, 3, 4 */
	bool						bValue		   /* true or false */
);

BERR_Code BRAP_RFAUDIO_P_ScaleOutput ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t ui32Reg
);

BERR_Code BRAP_RFAUDIO_P_ScaleInput ( 
	BRAP_CAPPORT_P_Handle		phCapPort,     /* [in] Pointer to Capture Port handle */
	uint32_t ui32InputScaleValue				/* [in] Input scale value */
);
#endif /* ( BCHP_CHIP==3548 ) */

#ifdef __cplusplus
}
#endif


#endif /* _BRAP_CAPPORT_PRIV_H_ */

/* End of File */

