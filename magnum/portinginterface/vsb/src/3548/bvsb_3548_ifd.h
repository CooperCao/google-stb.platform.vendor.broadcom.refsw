/*************************************************************************** *     
 * Copyright (c) 2005-2010, Broadcom Corporation *     
 * All Rights Reserved
 *   Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 * 
 *
 ***************************************************************************/
#ifndef _BVSB_3548_IFD_H_
#define _BVSB_3548_IFD_H_

typedef BVSB_NtscStatus BVSB_IfdStatus;
/***************************************************************************
Summary:
	Enumeration for analog input

Description:
	This enum specifies analog audio input options for the IFD decoder. This
	only applies to IFD core in cases where this PI controls both ofdm and
	analog video receiver. 

See Also:
	None.

****************************************************************************/
typedef enum BVSB_AnalogInput {
	BVSB_AnalogInput_eIf = 0,		/* Audio input with IF signal */
	BVSB_AnalogInput_eSif_1 = 0x80	/* Audio input from SIF Channel 1 */
} BVSB_AnalogInput;


/***************************************************************************
Summary:
	Enumeration for analog video/audio modes

Description:
	This enum specifies analog video mode options for the IFD decoder. 

See Also:
	None.

****************************************************************************/
typedef enum BVSB_IfdMode
{
	BVSB_IfdMode_eM_BTSC = 0,
	BVSB_IfdMode_eM_A2 =1,
	BVSB_IfdMode_eN = 3,
	BVSB_IfdMode_eBGH_A2 = 5,
	BVSB_IfdMode_eBGH_NICAM = 6,
	BVSB_IfdMode_eI = 8,
	BVSB_IfdMode_eDK_NICAM = 9,
	BVSB_IfdMode_eDK1_A2 = 10,
	BVSB_IfdMode_eDK2_A2 = 11,
	BVSB_IfdMode_eDK3_A2 = 12,
	BVSB_IfdMode_eL = 14
} BVSB_IfdMode;

/***************************************************************************
Summary:
	Structure containg input parameters to IFD acquire command

Description:
	This structure contains the setting for IFD acquire command

See Also:
	None.

****************************************************************************/
typedef struct BVSB_IfdParams
{
   BVSB_AnalogVideoMode	eVideoMode;		/* Selects analog video standards */
   BVSB_AnalogAudioMode    eAudioMode;     /* Selects analog audio standards */
   BVSB_AnalogInput        eAnalogInput;    /* Select analog input options */
   uint32_t                ifFreqOffset;    /* IF Frequency offset */
   BVSB_IfdAcquisitionMode eAcquisitionMode; /* Acquistion Mode */
   bool                      ifdAgcControl; /* Gives control of AGC pins to IFD */
} BVSB_IfdParams;


/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_FE(
        BVSB_Handle h,
        BVSB_IfdMode videoMode
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_Carrier_Limits(
        BVSB_Handle h
        /*BVSB_Ifd_PullInRange eRange */
        );


/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Ifd_load_coef(
        BVSB_Handle h,
        uint16_t *pCoef,
        uint16_t nCoefLen,
        uint16_t nCoefStart
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_NICAM(
        BVSB_Handle h                      /* [in] BVSB handle */
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_AP1AP2_and_VarDet(
        BVSB_Handle h,                      /* [in] BVSB handle */
        BVSB_IfdMode eVideoMode,
        BVSB_AnalogAudioDeviation eAnalogAudioDeviation      
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_SIF_Source(
        BVSB_Handle h,                      /* [in] BVSB handle */
        BVSB_IfdMode eVideoMode,
        BVSB_AnalogInput eAnalogInput
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Init_Ifd_VP(
        BVSB_Handle h,                      /* [in] BVSB handle */
        BVSB_IfdMode videoMode
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_Ifd_set_if_freq(
        BVSB_Handle h                      /* [in] BVSB handle */
        /* const BVSB_IfdParams* pParam */
        );

/***************************************************************************
Summary:
	Internal IFD acquisition function. Should not be called from the outside

Description:

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_IfdTop(
      BVSB_Handle h,                      /* [in] BVSB handle */
      const BVSB_IfdParams* pParam,
      BVSB_IfdMode videoMode,
      BVSB_AnalogAudioDeviation eAnalogAudioDeviation      
      );

/***************************************************************************
Summary:
	Internal video mode decode function

Description:
    Top-level software will pass in video and audio mode separately. This 
    function decodes to the value that IFD expects

See Also:
	None.
****************************************************************************/
BERR_Code BVSB_3548_P_GetIfdMode(
    BVSB_AnalogVideoMode    eVideoMode,     /* [in] Video mode */
    BVSB_AnalogAudioMode    eAudioMode,     /* [in] Audio mode */
    BVSB_IfdMode*           eIfdMode        /* [out] Returns desired IFD mode */
    );


/* API Functions */
BERR_Code BVSB_3548_P_SetIfdAudioSettings(BVSB_Handle, const BVSB_IfdAudioSettings*);
BERR_Code BVSB_3548_P_GetIfdAudioSettings(BVSB_Handle, BVSB_IfdAudioSettings*);
BERR_Code BVSB_3548_P_ResetIfdStatus(BVSB_Handle);

#endif


