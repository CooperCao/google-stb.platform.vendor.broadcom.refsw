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
*   Module name: SPDIFFM
*   This file lists all data structures, macros, enumerations and function 
*   prototypes for the SPDIF Formatter abstraction, which are internal ie NOT
*   exposed to the application developer. These can be used only by the 
*   Audio Manager and other FMM submodules.
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_SPDIFFM_PRIV_H_
#define _BRAP_SPDIFFM_PRIV_H_

#include "brap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BRAP_SPDIFFM_CBIT_BUFFER_SIZE 12

/* Defines for CP TOGGLE RATE */
#define BRAP_SPDIFFM_CP_TOGGLE_RATE_DISABLED 0
#define BRAP_SPDIFFM_MIN_CP_TOGGLE_RATE_HZ 4
#define BRAP_SPDIFFM_MAX_CP_TOGGLE_RATE_HZ 10




/***************************************************************************
Summary:
    This enumeration defines repeatition period of burst insertion for 
    SPDIF Formatter.

***************************************************************************/
typedef enum BRAP_SPDIFFM_P_BurstRepPeriod
{
    BRAP_SPDIFFM_P_BurstRepPeriod_eNone,    /* No burst is inserted */
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer3,    /* Repetion period = 3 
                                               Used for AC3 and DTS */
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer4,    /* Repetion period = 4 
                                               Used for AC3 Plus */                                               
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer64,   /* Repetion period = 64 
                                               Used for MPEG2, low sampling 
                                               freq. E.g. */
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer32,   /* Repetion period = 32 
                                               Used for all other MPEG1/2 
                                               formats. E.g. */
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer1024, /* Repetion period = 1024 */
    BRAP_SPDIFFM_P_BurstRepPeriod_ePer4096  /* Repetion period = 4096 
                                               Recommended for use in 
                                               null burst */
}BRAP_SPDIFFM_P_BurstRepPeriod;
 

/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Opening a SPDIF Formatter.
    Note that this includes both public and private settings.
    No settings at present

Description:


See Also:

***************************************************************************/
typedef struct BRAP_SPDIFFM_P_Settings
{
    BRAP_SPDIFFM_Settings sExtSettings;     /* Settings provided by the 
                                               application*/
}BRAP_SPDIFFM_P_Settings;



/***************************************************************************
Summary:
    Parameters to be passed by Audio Manager on Starting the SPDIF Formatter.
    Note that this includes both public and private parameters.

Description:
    bSeparateLRChanNum = 0 => Dont override the Channel Status buffer setttings.
                              Left and Right channel numbers are equal.
    bSeparateLRChanNum = 1 => Override the Channel Status buffer setttings.
                              For Left channel CBIT[20:21] is replaced by 10
                              For Right channel CBIT[20:21] is replaced by 01
                              CBIT[22:23] are unchanged.

See Also:

***************************************************************************/
typedef struct BRAP_SPDIFFM_P_Params
{
    BRAP_SPDIFFM_Params    sExtParams;   /* Parameters provided by application*/
    bool                   bCompressed;  /* TRUE: input data is compressed 
                                             FALSE: input data is not compressed*/
    bool                   bSeparateLRChanNum;   /* TRUE: CBIT[20:21] is overridden
                                                    FALSE: CBIT[20:21] is used */
	bool				bUseSpdifPackedChanStatusBits;
							/* Applies only when output port is SPDIF.
							    This field decides whether to use SPDIF channel 
							    status bits packed in two 32-bit words
							    OR seperated into individual parameters.
							    TRUE = Use field sSpdifPackedChanStatusBits
							    FALSE = Use field sSpdifChanStatusParams 
							    When TRUE, all the channel status bits can be
							    set by application. If channel is a decode channel,
							    then firmware is disabled from programming
							    any of the channel status bits. 
							    This field can be changed only when decode is
							    in "Stop" state. */

	BRAP_OP_SpdifPackedChannelStatusBits		sSpdifPackedChanStatusBits;
							/* Applies only when output port is SPDIF.
							    This field is valid only when
							    bUseSpdifPackedChanStatusBits = TRUE.
							    This field contains 39 bits of SPDIF channel
							    status packed into two 32-bit words.
							    Word 0 contains channel status bits [31:0]
							    and Word 1 contains channel status bits [39:32] */
    unsigned int            uiCpToggleRate;
                                /* Allows toggling of the copy protection bit 
                                   (bit 2, Cp) according to section A.2 of IEC 
                                   60958-3. When enabled, valid value should be
                                   between 4 to 10 (in Hz). 0 indicates it is 
                                   disabled. */
	BRAP_OP_SpdifChanStatusParams sChanStatusParams;
										 /* SPDIF channel status params used to
										    program CBIT buffer for a PCM channel*/
    BRAP_SPDIFFM_P_BurstRepPeriod eBurstRepPeriod;
                                         /* Burst repetition period used only when
                                            bCompressed is TRUE */
    BAVC_AudioSamplingRate        eSamplingRate;
                                         /* Output sampling rate */
    bool                bUseHwCBit;         /* TRUE: Use HW C-Bit programer
                                                            FALSE: Use SPDIF FM C-Bit Programer */
}BRAP_SPDIFFM_P_Params;


/***************************************************************************
Summary:
    Abstraction of a SPDIF Formatter Stream
    
Description:
    It contains all the information required to handle the SPDIF Formatter Stream
    Particularly, it stores the indexes, handles for all required chip 
    related information, parent FMM handle etc

See Also:
    
***************************************************************************/
typedef struct BRAP_SPDIFFM_P_Object
{
    BRAP_SPDIFFM_P_Settings sSettings;  /* Setting provided during _Open(). */
    BRAP_SPDIFFM_P_Params   sParams;    /* Settings provided during _Start().*/

    BCHP_Handle             hChip;      /* Handle to chip object */
    BREG_Handle             hRegister;  /* Handle to register object */
    BMEM_Handle             hHeap;      /* Handle to memory object */
    BINT_Handle             hInt;       /* Handle to interrupt object */


    BRAP_FMM_P_Handle       hFmm;           /* Parent FMM handle */
    unsigned int            uiIndex;        /* SPFIDFM index */
    unsigned int            uiStreamIndex;  /* Stream 0 or Stream 1 */
    uint32_t                ui32Offset;       /* Offset of a register for Input 1
                                               from corresponding register for
                                               Input 0 */
    unsigned int           uiOpenCnt;   /* No. of open audio channels currently
                                           routed to this SPDIFFM stream */
    unsigned int           uiStartCnt;  /* No. of active audio channels currently
                                           routed to this SPDIFFM stream */                                               
    
}BRAP_SPDIFFM_P_Object;


/***************************************************************************
Summary:
    Opens a SPDIF Formatter instance

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SPDIFFM_P_Close, BRAP_SPDIFFM_P_GetDefaultSettings.
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_Open (
    BRAP_FMM_P_Handle         hFmm,             /* [in] Parent FMM handle */
    BRAP_SPDIFFM_P_Handle *   phSpdifFm,        /* [out] SPDIFFM stream handle */ 
    unsigned int              uiStreamIndex,    /* [in] SPDIFFM Stream index */
    const BRAP_SPDIFFM_P_Settings * pSettings   /* [in] The SPDIFFM stream settings*/                                         
);

/***************************************************************************
Summary:
    Releases all the resources associated with this SPDIF Formatter and frees 
    the handles.

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SRCCH_P_Open.
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_Close ( 
    BRAP_SPDIFFM_P_Handle  hSpdifFm /* [in] SPDIF Formatter handle */
);


/***************************************************************************
Summary:
    Starts SPDIF Formatter

Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SPDIFFM_P_Stop 
    
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_Start ( 
	BRAP_ChannelHandle 		hRapCh,		  /* [in] Rap channel handle */	
    BRAP_SPDIFFM_P_Handle   hSpdifFm,     /* [in] SPDIF Formatter handle */
    const BRAP_SPDIFFM_P_Params *pParams /* [in] Pointer to start
                                                  parameters */ 
);


/***************************************************************************
Summary:
    Stops SPDIF Formatter

Description:
   
Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
    BRAP_SPDIFFM_P_Start 
    
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_Stop (
    BRAP_SPDIFFM_P_Handle  hSpdifFm /* [in] SPDIF Formatter handle */
);



/***************************************************************************
Summary:
    Returns default values for SPDIF Formatter Start time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned 

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_GetDefaultParams ( 
    BRAP_SPDIFFM_P_Params    *pDefParams   /* Pointer to memory where default
                                              parameters should be written */    
);

/***************************************************************************
Summary:
    Returns current values for SPDIF Formatter Start time parameters.

Description:
	Returns current parameter values stored in handle.

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_GetCurrentParams (
	BRAP_SPDIFFM_P_Handle  hSpdifFm, /* [in] SPDIF Formatter handle */
    BRAP_SPDIFFM_P_Params    *pCurParams   /* Pointer to memory where current 
                                              parameters should be written */    
);

/***************************************************************************
Summary:
    Returns default values for SPDIF Formatter Open time parameters.

Description:
    For parameters that the system cannot assign default values to, 
    an invalid value is returned 

Returns:
    BERR_SUCCESS on success
    Error code on failure

See Also:
**************************************************************************/
BERR_Code 
BRAP_SPDIFFM_P_GetDefaultSettings ( 
    BRAP_SPDIFFM_P_Settings *pDefSettings   /* Pointer to memory where default
                                              settings should be written */    
);

/***************************************************************************
Summary:
    Prepares and programs channel status bits. 
Description:

Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	BRAP_SPDIFFM_P_PrepareCBITData, BRAP_SPDIFFM_P_ProgramCBITData
**************************************************************************/

BERR_Code BRAP_SPDIFFM_P_ProgramChanStatusBits(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,       /* [in] SPDIF Formatter handle */
    BAVC_AudioSamplingRate  eSamplingRate   /* [in] SPDIF Output SR */
    ) ; 
/***************************************************************************
Summary:
    Prepares and programs channel status bits. 
Description:
    ISR version of BRAP_SPDIFFM_P_ProgramChanStatusBits()
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	BRAP_SPDIFFM_P_PrepareCBITData, BRAP_SPDIFFM_P_ProgramCBITData
**************************************************************************/

BERR_Code BRAP_SPDIFFM_P_ProgramChanStatusBits_isr(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,       /* [in] SPDIF Formatter handle */
    BAVC_AudioSamplingRate  eSamplingRate   /* [in] SPDIF Output SR */
    ) ; 
/***************************************************************************
Summary:
    Changes Burst repetition period for MPEG-2 as per IEC-61937-1. 
Description:
    If sampling rate of an MPEG-2 stream is < 32kHz then burst repetition 
    rate is set to PER_64 else it is set to PER_32. Applied only if the 
    data out is compressed from SPDIFFM. 
Returns:
    BERR_SUCCESS on success
See Also:

**************************************************************************/
BERR_Code BRAP_SPDIFFM_P_ChangeBurstRepPeriodForMPEG2_isr(
    BRAP_SPDIFFM_P_Handle   hSpdifFm    /* [in] SPDIF Formatter handle */
);



/***************************************************************************
Summary:
    If null/pause burst is enabled, for compressed data, this PI sets the 
    OVERWRITE_DATA flag which determines 
    whether or not the data from the mixer will be overwritten by a Burst.
    Must be called only after the output port has been configured and opened.
    
Description:

REP_PERIOD   MUTE
    0         0     pause/null/zero never inserted
    0         1     zero overwrites samples from mixer
  non-0       0     pause/null on underflow
  non-0       1     pause/null overwrites sample from mixer


Returns:
    BERR_SUCCESS on success
    Error code on failure
    
See Also:
	

**************************************************************************/
BERR_Code
BRAP_SPDIFFM_P_InsertBurstOnMute (
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    bool                    bOverwrite  /* TRUE: overwrite data from mixer
                                           FALSE: Don't overwrite data from 
                                           mixer */
) ;

BERR_Code 
BRAP_SPDIFFM_P_ProgramCBITBuffer_isr( 
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    uint32_t			*pui32CBITBuffer /* [in] SPDIF bit buffer */
);


/***************************************************************************
Summary:
    Programs dither and burst for SPDIFFM.
Description:
    This internal routine does the register programming for dither and burst
    for SPDIFFM.
Returns:
    BERR_SUCCESS on success
    Error code on failure
See Also:
	
**************************************************************************/
BERR_Code
BRAP_SPDIFFM_P_ProgramDitherAndBurst(
    BRAP_SPDIFFM_P_Handle   hSpdifFm,   /* [in] SPDIF Formatter handle */
    BRAP_SPDIFFM_P_Settings *pSpdiffmSettings,/* [in] SPDIFFM settings */
    BRAP_SPDIFFM_P_BurstRepPeriod eBurstRepPeriod 
                                              /* [in] Burst repetition period */
);


#ifdef __cplusplus
}
#endif


#endif /* _BRAP_SPDIFFM_PRIV_H_ */

/* End of File */
