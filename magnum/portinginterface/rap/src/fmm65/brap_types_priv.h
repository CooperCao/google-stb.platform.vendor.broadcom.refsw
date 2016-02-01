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
* Module Description: Private data structures common to all the modules of
*					  Raptor PI.
* Revision History:
* $brcm_Log: $
* 
***************************************************************************/
#ifndef _BRAP_TYPES_PRIV_H__ /*{{{*/
#define _BRAP_TYPES_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BRAP_UNSIGNED_PCM_SUPPORTED 1

#define HIFIDAC_VERSION 65

#if ((BCHP_CHIP == 7340)||(BCHP_CHIP == 7342)||(BCHP_CHIP == 7125)||(BRAP_7550_FAMILY == 1))
#define BRAP_PLL1_SUPPORTED 0
#else
#define BRAP_PLL1_SUPPORTED 1
#endif



#if (BCHP_CHIP == 7420)
#define BRAP_MAX_RFMOD_OUT  2
#else
#define BRAP_MAX_RFMOD_OUT  0
#endif


/***************************************************************************
Summary:
    This enumeration lists the 'states' - started and stopped.
    Following are the various operations and corresponding states:

    Operation   -   State
    ---------       -----
    Open        -   Stop
    Start       -   Start
    Stop        -   Stop
    Close       -   The channel/resource handle itself doesn't exist. So, 
                    state doesn't matter. 
***************************************************************************/     
typedef enum BRAP_P_State
{
    BRAP_P_State_eInvalid,
    BRAP_P_State_eOpened,
    BRAP_P_State_eStarted
}BRAP_P_State;

/***************************************************************************
Summary:
    This enumeration lists the various Downmix paths. 
    Note: In 7440, this is present only for the code reusability's sake. It 
    is not exposed to the user.
***************************************************************************/ 
typedef enum BRAP_DEC_DownmixPath
{
	BRAP_DEC_DownmixPath_eMain,			/* Main downmix path. This path 
											    supports multiple channels */
	BRAP_DEC_DownmixPath_eStereoDownmix,  /* Stereo downmix path */
	BRAP_DEC_DownmixPath_eMax
}BRAP_DEC_DownmixPath;


typedef struct BRAP_DSP_P_Device			*BRAP_DSP_Handle;

typedef struct BRAP_DSPCHN_P_Channel			*BRAP_DSPCHN_Handle;	/* Opaque DSP context handle */

typedef uint32_t TIME_45KHZ_TICKS;

#ifdef RAP_REALVIDEO_SUPPORT
#define RAP_VIDEOONDSP_SUPPORT 1
#endif


/***************************************************************************
Summary:
    Multi Stream Decoder handle
Description:
    This is an opaque handle for an Multi Stream Decoder object 
See Also:
***************************************************************************/ 
typedef struct BRAP_P_MultiStreamDecoderDetail *BRAP_MultiStreamDecoderHandle;


#ifdef __cplusplus
}
#endif

#endif /*}}} #ifndef _BRAP_TYPES_PRIV_H__ */

/* End of File */
