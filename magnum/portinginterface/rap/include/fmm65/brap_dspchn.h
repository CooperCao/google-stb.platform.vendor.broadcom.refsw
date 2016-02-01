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
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

/*=************************ Module Overview ********************************
The audio DSP context (DSPCHN) is one of the object types of BRAP PI 
architecture. Each DSPCHN object is one context running in a DSP channel.
Each DSPCHN object (or DSP context) is referred to as DSPCHN channel. 
The various DSP contexts are decode, sample rate conversion (SRC), 
pass through. Out of these DSP decode context supports various post 
processing algorithms.

The audio DSPCHN channel API(BRAP_DSPCHN_P/BRAP_DSPCHN) is part of the 
BRAP PI implementation. This module manages a set of  audio DSPCHN channels.
These APIs deal with the operations carried out on DSP context 
(like "start decode" etc).

Each DSP channel is capable of running multiple DSP contexts. Prototypes 
of DSP channel APIs (BRAP_DSP_P/BRAP_DSP) are in baud_dsp.h.

DSP channel APIs (BRAP_DSP_P/BRAP_DSP) and DSP context APIs 
(BRAP_DSPCHN_P/BRAP_DSPCHN) are used by audio manager's decode channel 
APIs (BRAP_DEC).

Design
The design of this module has been broken down into the following sections:

o Part 1

  APIs that are not exposed to the application. These APIs have prefix
  BRAP_DSPCHN_P. These APIs are used by audio manager's decode channel APIs 
  only.

o Part 2

  APIs exposed to the application. These APIs have prefix BRAP_DSPCHN. They
  are implemented to expose DSP context's features to application.
  They are only for unforeseen cases and are provided on need basis.
  These APIs are used by audio manager's decode channel APIs as well as
  by application.
***************************************************************************/

#ifndef BRAP_DSPCHN_H__
#define BRAP_DSPCHN_H__

#include "brap_types.h"
#include "brap_dsp.h"
#include "bavc.h"
#include "brap_fwstatus_priv.h"

/* #include "brap_fwifdwnld.h"*/
#ifdef __cplusplus
extern "C" {
#endif

#define RAP_INVALID_INDEX	0xffff

#define BRAP_DSPCHN_UNBOUNDED_START_PTS			(0)
#define BRAP_DSPCHN_UNBOUNDED_STOP_PTS				(0xFFFFFFFF)
#define PEQ_FILTERS                             7
#define SUBSONIC_FILTERS                        2
#define PEQ1                                    0
#define PEQ2                                    1
#define PEQ3                                    2
#define PEQ4                                    3
#define PEQ5                                    4
#define PEQ6                                    5
#define PEQ7                                    6

#define BRAP_DSPCHN_PLAYBACKRATE_NORMAL (100)

#define BRAP_FW_MAX_PEQ_BANDS                   8
#define BRAP_FW_MAX_GEQ_BANDS                   10
/***************************************************************************
Summary:
	This enumeration defines the supported audio input type.

***************************************************************************/
typedef enum BRAP_DSPCHN_AudioType
{
   	BRAP_DSPCHN_AudioType_eMpeg,			/* MPEG */
   	BRAP_DSPCHN_AudioType_eAac,				/* AAC */
   	BRAP_DSPCHN_AudioType_eAacAdts = BRAP_DSPCHN_AudioType_eAac,				/* AAC */   	
  	BRAP_DSPCHN_AudioType_eAacLoas,				/* AAC */   	
   	BRAP_DSPCHN_AudioType_eAacSbrAdts,			/* AAC_SBR */
   	BRAP_DSPCHN_AudioType_eAacSbr,			/* AAC_SBR */
   	BRAP_DSPCHN_AudioType_eAacSbrLoas = BRAP_DSPCHN_AudioType_eAacSbr,			/* AAC_SBR */   	
   	BRAP_DSPCHN_AudioType_eAc3,				/* AC3 */
   	BRAP_DSPCHN_AudioType_eAc3Plus,			/* AC3_PLUS */
   	BRAP_DSPCHN_AudioType_eDts,				/* DTS */
   	BRAP_DSPCHN_AudioType_eLpcmBd,			/* LPCM Blue Ray Disc */
   	BRAP_DSPCHN_AudioType_eLpcmHdDvd,		/* LPCM HD-DVD */
   	BRAP_DSPCHN_AudioType_eDtshd,			/* DTSHD */
   	BRAP_DSPCHN_AudioType_eLpcmDvd,			/* LPCM DVD */
   	BRAP_DSPCHN_AudioType_eWmaStd,			/* WMA Standard */
   	BRAP_DSPCHN_AudioType_eAc3Lossless,		/* AC3 in LOSSLESS*/
   	BRAP_DSPCHN_AudioType_eMlp,				/* MLP */
   	BRAP_DSPCHN_AudioType_ePcm,             /* PCM Data */
   	BRAP_DSPCHN_AudioType_eDtsLbr,          /* DTS-LBR */
   	BRAP_DSPCHN_AudioType_eDdp7_1,          /* DDP 7.1 */  
   	BRAP_DSPCHN_AudioType_eMpegMc,			/* MPEG MC*/   	 	
   	BRAP_DSPCHN_AudioType_eWmaPro,			/* WMA Pro */
   	BRAP_DSPCHN_AudioType_eDtshdSub,        /* DTSHD SUB*/
   	BRAP_DSPCHN_AudioType_eLpcmDvdA,        /* LPCM A DVD*/
    BRAP_DSPCHN_AudioType_eDtsBroadcast,        /*DTS Broadcast*/   	
    BRAP_DSPCHN_AudioType_ePcmWav,        /*PCM On AVI*/   	   	
    BRAP_DSPCHN_AudioType_eAmr,    /* AMR decoder */
    BRAP_DSPCHN_AudioType_eDra,    /* DRA Decoder */     
    BRAP_DSPCHN_AudioType_eRealAudioLbr,    /* Real Audio LBR*/
    BRAP_DSPCHN_AudioType_eMax,			    /* Max value */
   	BRAP_DSPCHN_AudioType_eInvalid	= 0xFF
} BRAP_DSPCHN_AudioType;


#define BRAP_VIDEO_CODEC_START_INDEX    BRAP_DSPCHN_AudioType_eMax

typedef enum BRAP_DSPCHN_VideoType
{
    BRAP_DSPCHN_VideoType_eRealVideo9 = BRAP_VIDEO_CODEC_START_INDEX,  /* Real Video*/
    BRAP_DSPCHN_VideoType_eMax,			    /* Max value */
    BRAP_DSPCHN_VideoType_eInvalid	= 0xFF    
}BRAP_DSPCHN_VideoType;

typedef enum BRAP_DSPCHN_GfxType
{
    BRAP_DSPCHN_GfxType_eSoftGfx, /* Real Video*/
    BRAP_DSPCHN_GfxType_eMax,			    /* Max value */
    BRAP_DSPCHN_GfxType_eInvalid	= 0xFF    
}BRAP_DSPCHN_GfxType;


typedef enum BRAP_DSPCHN_ScmType
{
    BRAP_DSPCHN_ScmType_eScm_1, /* Secure Code Module 1*/
    BRAP_DSPCHN_ScmType_eScm_2, /* Secure Code Module 2*/		
    BRAP_DSPCHN_ScmType_eMax,			    /* Max value */
    BRAP_DSPCHN_ScmType_eInvalid	= 0xFF    
}BRAP_DSPCHN_ScmType;


/***************************************************************************
Summary:
	This enumeration defines various decode modes.

***************************************************************************/
typedef enum BRAP_DSPCHN_DecodeMode
{
	BRAP_DSPCHN_DecodeMode_eDecode,		/* Decode mode */
	BRAP_DSPCHN_DecodeMode_ePassThru, 	/* Pass thru mode */
	BRAP_DSPCHN_DecodeMode_eMax,
	BRAP_DSPCHN_DecodeMode_eSimulMode, 	/* Simul mode */	
   	BRAP_DSPCHN_DecodeMode_eInvalid	= 0xFF
} BRAP_DSPCHN_DecodeMode;


/***************************************************************************
Summary:
	This enumeration defines various encode modes.

***************************************************************************/
typedef enum BRAP_DSPCHN_EncodeMode
{
	BRAP_DSPCHN_EncodeMode_eRealtime,		/* Real-time encode from Line-In/Mic-In */
	BRAP_DSPCHN_EncodeMode_eNonRealtime, 	/* Non Real-time encode from CD etc. */
	BRAP_DSPCHN_EncodeMode_eSimulMode,	/* Simultaneous mode: It comprises 
										   of one real-time and one non real-time 
										   encode operation. */
   	BRAP_DSPCHN_EncodeMode_eInvalid	= 0xFF
} BRAP_DSPCHN_EncodeMode;



/*********************************************************************
Summary:
    Enums that describes whether the low delay mode is enabled/disabled

Description:
    PI conveys this information to API to indicate Lowdelay is enabled
	or not

See Also:
	
**********************************************************************/ 

typedef enum BRAP_DSPCHN_LowDelayEnableMode
{
	BRAP_DSPCHN_LowDelayMode_eDisabled,
	BRAP_DSPCHN_LowDelayMode_eEnabled	=1,
	BRAP_DSPCHN_LowDelayMode_eMax,			        /* Max value */
   	BRAP_DSPCHN_LowDelayMode_eInvalid	= 0x7FFFFFFF          

}BRAP_DSPCHN_LowDelayEnableMode;

/***************************************************************************
Summary:
	This enumeration defines various types of DVD audio material
***************************************************************************/
typedef enum BRAP_DSPCHN_DvdType
{
	BRAP_DSPCHN_DvdType_eBd = 0,
	BRAP_DSPCHN_DvdType_eHdDvd = 1
} BRAP_DSPCHN_DvdType;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3 compression mode  
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3CompMode
{
	BRAP_DSPCHN_Ac3CompMode_eCustomA,
	BRAP_DSPCHN_Ac3CompMode_eCustomD,
	BRAP_DSPCHN_Ac3CompMode_eLine,
	BRAP_DSPCHN_Ac3CompMode_eRf,
	BRAP_DSPCHN_AC3CompMode_eOff
} BRAP_DSPCHN_Ac3CompMode;


/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3 dual mono mode. This enum is effective only when acmod = 000
	in the stream.
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3DualMonoMode
{
	BRAP_DSPCHN_Ac3DualMonoMode_eStereo,
	BRAP_DSPCHN_Ac3DualMonoMode_eLtMono,
	BRAP_DSPCHN_Ac3DualMonoMode_eRtMono,
	BRAP_DSPCHN_Ac3DualMonoMode_eDualMono
}BRAP_DSPCHN_Ac3DualMonoMode; 

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3 karaoke capable mode
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3KaraokeVocalSelect
{
	BRAP_DSPCHN_Ac3KaraokeVocalSelect_eNoVocal,
	BRAP_DSPCHN_Ac3KaraokeVocalSelect_eLtVocal,
	BRAP_DSPCHN_Ac3KaraokeVocalSelect_eRtVocal,
	BRAP_DSPCHN_Ac3KaraokeVocalSelect_eBothVocal	
} BRAP_DSPCHN_Ac3KaraokeVocalSelect; 

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3 stereo mode. This eum is effective only when stream is
	Annex-D compliant and 
	BRAP_DEC_AudioParams.eOutputMode = BRAP_OutputMode_eStereo. 	
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3StereoMode
{
	BRAP_DSPCHN_Ac3StereoMode_eAuto,
	BRAP_DSPCHN_Ac3StereoMode_eLtRt,
	BRAP_DSPCHN_Ac3StereoMode_eLoRo
} BRAP_DSPCHN_Ac3StereoMode;


typedef enum BRAP_DSPCHN_MSDecodeUsageMode
{ 
    BRAP_DSPCHN_MSDecodeUsageMode_eSingleDecode=0,
    BRAP_DSPCHN_MSDecodeUsageMode_eMS10Decode, 
    BRAP_DSPCHN_MSDecodeUsageMode_eMS11Decode,
    BRAP_DSPCHN_MSDecodeUsageMode_eMax,
    BRAP_DSPCHN_MSDecodeUsageMode_eINVALID=0x7fffffff
} BRAP_DSPCHN_MSDecodeUsageMode;

/***************************************************************************
Summary: 
	This enumeration defines MS Decoder type
		Decoder type as Primary, Secondary, Sound Effects
****************************************************************************/
typedef enum BRAP_DSPCHN_DolbyMsDecoderType
{
    BRAP_DSPCHN_DolbyMsDecoderType_ePrimary = 0,
    BRAP_DSPCHN_DolbyMsDecoderType_eSecondary,
    BRAP_DSPCHN_DolbyMsDecoderType_eSoundEffects,
    BRAP_DSPCHN_DolbyMsDecoderType_eLAST,
    BRAP_DSPCHN_DolbyMsDecoderType_eINVALID = 0x7fffffff
}BRAP_DSPCHN_DolbyMsDecoderType;


/***************************************************************************
Summary:
	This structure contains AC3 decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3ConfigParams
{
    BRAP_DSPCHN_DolbyMsDecoderType eDecoderType;  /* Enum conveys whether the Decoder is 
                                                     Primary, Secondary or Sound Effects */
    
    uint16_t			ui16DynRngScaleHi; /* 	Dynamic range compression cut scale factor for Main Output 
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */

    uint16_t			ui16DynRngScaleHi_DownmixPath;  /*Applicable Only If MS11 License is available*/
                            /* 	Dynamic range compression cut scale factor for Downmixed Output 
    						Input value range = 0% to 100%. 100%
    						corresponds to value of unity. Any value
    						above 100% is considered as unity */

    						
    uint16_t			ui16DynRngScaleLo; /*	Dynamic range compression boost scale factor for Main Output 
    						Input value range = 0% to 100%. 100%
    						corresponds to value of unity. Any value
    						above 100% is considered as unity */

    uint16_t			ui16DynRngScaleLo_DownmixPath;  /*Applicable Only If MS11 License is available*/
                            /*	Dynamic range compression boost scale factor for Downmixed Output
    						Input value range = 0% to 100%. 100%
    						corresponds to value of unity. Any value
    						above 100% is considered as unity */
    						
    uint16_t 			ui16PcmScaleFactor; /*	PCM scale factor for Main Output . Input value range = 0 to 100%
    						100% corresponds to value of unity.Any value 
    						above 100% is considered as unity. Each decoded
    						PCM sample gets scaled by this factor */

    uint16_t 			ui16PcmScaleFactor_DownmixPath;  /*Applicable Only If MS11 License is available*/
                            /*	PCM scale factor for Downmixed Output. Input value range = 0 to 100% 
    						100% corresponds to value of unity.Any value 
    						above 100% is considered as unity. Each decoded
    						PCM sample gets scaled by this factor */
    						
    BRAP_DSPCHN_Ac3CompMode		eCompMode;	/*	Dynamic range compression mode for Main Output */

    BRAP_DSPCHN_Ac3CompMode		eCompMode_DownmixPath;	 /*Applicable Only If MS11 License is available*/
                                            /*	Dynamic range compression mode  for Downmixed Output*/

    bool                bStreamDialogNormEnable; /* Stream Dialog Normalization flag.  
                                 This flag is set to True if the Dialog Normalization value that comes in the stream 
                                 need to be used and to False if a user defined value need to be applied */  
                             
    uint16_t            ui16UserDialogNormVal;  /* User provided Dialog Normalization value, 
                                used only when bStreamDialNormEnable=false. 
                                The value is in dB and the range is 0 to 31. 
                                The value 0 indicates no Dialog Normalization being performed */
                            
	BRAP_DSPCHN_Ac3KaraokeVocalSelect	eKaraokeVocalSelect; /*	Karaoke capable reproduction mode */
                            
	bool				bUseKaraokeLevelPanCoeff;	/*	Karaoke user defined parameter flag
									When it is true, the user defined levels
									and panning for the vocal channels and
									melody channel is applied. */
	int16_t				i16KaraokeLevelPanCoeff[6];/*	User defined Karaoke level and panning 
									coefficients. Valid only 
									when bKaraokeParamFlag = true.
									Input value range = -100 to 100.
									Value of -100 corresponds to -1 and 100
									corresponds to +1. Each coefficient
									value should be initialized to its 
									original value multiplied by 100.
									Any value less than -100, is taken as
									-100 (-1) and any value greater than 100,
									is taken as 100 (+1).
									ui16KaraokeDramCoeff[0] = v1_lev
									ui16KaraokeDramCoeff[1] = v1_pan
									ui16KaraokeDramCoeff[2] = v2_lev
									ui16KaraokeDramCoeff[3] = v2_pan
									ui16KaraokeDramCoeff[4] = m_lev
									ui16KaraokeDramCoeff[5] = m_pan */
	BRAP_DSPCHN_Ac3StereoMode	eStereoMode; 		/*	Stereo output mode. It is effective
									only when stream is annex-D compliant and
									BRAP_DEC_AudioParams.eOutputMode = 
									BRAP_OutputMode_eStereo. */ 
	bool				bUseUserDownmixCoeff;	/*	User defined downmix flag. This is set to true,
									when user wants to set the appropriate downmix
									coefficients (or karaoke coefficients in case of
									karaoke stream) for each channel */
	int16_t				i16UserDownmixCoeff[6][6]; /*	User defined downmix (or karaoke) coefficients. 
									Valid only when bUsrUserDownmixCoeff = 1. 
									Input value range = -100 to 100.
									Value of -100 corresponds to -1 and 100
									corresponds to 1. Each coefficient
									value should be initialized to its 
									original value multiplied by 100. 
									Any value less than -100, is taken as
									-100 (-1) and any value greater than 100,
									is taken as 100 (1). Row-index in
									this two dimentional array is for output channel,
									column-index is for input channels contribution
									for that output channel.
									Coefficient ui16UserDownmixCoeff[I][J] is used 
									for mixing Ith input channel into output Jth 
									channel. The values of indices correspond to
									input/output channels as below.
									0 = Left channel
									1 = Centre channel
									2 = Right channel
									3 = Left surround channel
									4 = Right surround channel
									5 = LFE channel 
									For Karaoke stream, karaoke coefficients can be
									entered as per following mapping between karaoke
									channel assignement and normal channel assignment.
									See Annex-C of ATSC AC-3 specification.
 
									Audio	Normal Channel	Karaoke Channel
									coding	assignment	assignment
									mode
									2/0	L,R		L,R
									3/0	L,C,R		L,M,R
									2/1	L,R,S		L,R,V1
									3/1	L,C,R,S		L,M,R,V1
									2/2	L,R,Ls,Rs	L,R,V1,V2
									3/2	L,C,R,Ls,Rs	L,M,R,V1,V2 */  
        bool                          bUseDDConvert;/*Its Usage should be deprecated. Kept for compilation purpose.*/									
                                
    unsigned int 		    uiSubstreamIDToDecode;	/*Applicable Only If MS10/11 License is available*/
    
    bool                            bMsUsageMode;  /*True if MS11/MS10 usage case, False otherwise */
} BRAP_DSPCHN_Ac3ConfigParams;


/***************************************************************************
Summary:
	This enumeration defines downmix type for AAC decoder.
***************************************************************************/
typedef enum BRAP_DSPCHN_AAC_DownmixType
{
	BRAP_DSPCHN_AAC_DownmixType_eBRCM = 0,
     	BRAP_DSPCHN_AAC_DownmixType_eMatrix = 0, /*For Backward Compatibility, So that it should not break other customers build*/
	BRAP_DSPCHN_AAC_DownmixType_eArib = 1,
	BRAP_DSPCHN_AAC_DownmixType_eLtRt = 2,
        BRAP_DSPCHN_AAC_DownmixType_eLoRo = 3	
} BRAP_DSPCHN_AAC_DownmixType;

/***************************************************************************
Summary:
	This structure contains AAC decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AacConfigParams
{
	uint16_t ui16DrcGainControlCompress;	/* Dynamic range compression 
											cut scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint16_t ui16DrcGainControlBoost;		/* Dynamic range compression 
											boost scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint16_t ui16DrcTargetLevel;			/* Dynamic range compression 
											target level.
											Input value range = 0 to 127. Any value
											above 127 is considered as 127 */
	BRAP_DSPCHN_AAC_DownmixType	eDownmixType;
	bool bOutputLfeOn;					/*  Controls the LFE channel output
											1 = Output LFE channel
											0 = Do not output LFE channel */
	bool bExtDownmixEnable;		/* Controls the external downmix coeff usage
											1 = Enable
											0 = Disable */
	uint16_t ui16ExtDnMixCoeffs[6][6];		/*	User defined downmix (or karaoke) coefficients. 
											Valid only when ui16ExtDownmixEnable = 1 */  
        bool    bEnable96KhzDecode;     /*True: = Decode  96khz  content
                                                                            False: = Do not decode 96Khz*/                                                
	
} BRAP_DSPCHN_AacConfigParams;

/***************************************************************************
Summary:
	This enumeration defines downmix type for AAC SBR decoder.
***************************************************************************/
typedef enum BRAP_DSPCHN_AACSbr_DownmixType
{
	BRAP_DSPCHN_AACSbr_DownmixType_eBRCM = 0,   /* Invalid for Dolby Pulse(MS10) */
	BRAP_DSPCHN_AACSbr_DownmixType_eMatrix = 0, /* Invalid for Dolby Pulse(MS10) */
	BRAP_DSPCHN_AACSbr_DownmixType_eArib = 1,   /* Invalid for Dolby Pulse(MS10) */
	BRAP_DSPCHN_AACSbr_DownmixType_eLtRt = 2,
	BRAP_DSPCHN_AACSbr_DownmixType_eLoRo = 3
} BRAP_DSPCHN_AACSbr_DownmixType;

/***************************************************************************
Summary:
	This enumeration defines Compressed Mode for Dolby Pulse decoder.
***************************************************************************/
typedef enum BRAP_DSPCHN_DolbyPulse_CompMode
{
	BRAP_DSPCHN_DolbyPulse_CompMode_LineMode = 0,
	BRAP_DSPCHN_DolbyPulse_CompMode_RfMode = 1
} BRAP_DSPCHN_DolbyPulse_CompMode;

/***************************************************************************
Summary:
	This structure contains AACSBR decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AacSbrConfigParams
{
    BRAP_DSPCHN_DolbyMsDecoderType eDecoderType;  /* Enum conveys whether the Decoder is 
                                                     Primary, Secondary or Sound Effects */
                                                        
	uint16_t ui16DrcGainControlCompress;	/* Dynamic range compression 
											cut scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint16_t ui16DrcGainControlBoost;		/* Dynamic range compression 
											boost scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint16_t ui16DrcTargetLevel;			/* Dynamic range compression target level.
											Input value range = 0 to 127. Any value
											above 127 is considered as 127 
											For Dolby Pulse(MS10), Range is [0-124]*/
	BRAP_DSPCHN_AACSbr_DownmixType	eDownmixType;
	bool bOutputLfeOn;					    /* Invalid for Dolby Pulse(MS10).
	                                        Controls the LFE channel output
											1 = Output LFE channel
											0 = Do not output LFE channel */
	bool bExtDownmixEnable;		            /* Invalid for Dolby Pulse(MS10).
	                                        Controls the external downmix coeff usage
											1 = Enable
											0 = Disable */
	uint16_t ui16ExtDnMixCoeffs[6][6];		/* Invalid for Dolby Pulse(MS10).
	                                        User defined downmix (or karaoke) coefficients. 
											Valid only when ui16ExtDownmixEnable = 1 */             
    bool    bEnable96KhzDecode;             /* Invalid for Dolby Pulse(MS10).
                                               True: = Decode  96khz  content
                                               False: = Do not decode 96Khz */ 
    uint16_t    ui16DownmixCoefScaleIndex;  /* Invalid for Dolby Pulse(MS10).
                                                   Default = 50 Decoder default settings (existing normalization)
                                                   0 -> 0dB,
                                                   1 -> -0.5dB, 
                                                   2 -> -1dB, 
                                                   ... ,
                                                   23 -> -11.5dB,
                                                   24 -> -12dB; 
                                                   all values beyond 24 map to Default value */

                                                                                
    BRAP_DSPCHN_DolbyPulse_CompMode	    eCompMode;	/* Valid for Dolby Pulse(MS10) only */

    bool        bDolbyTranscodeEnabled; /*Deprecated : Kept just for compilation*/
                                            /* Valid for Dolby Pulse only. 
                                           True if Decoded output is being DD transcoded as well. In default set to false. */
                                                        
    bool        bApplyGain;         
                    /*  Valid for Dolby Pulse(MS10) only
                    True  - Apply the Gain Factor below 
                       False - Don't apply the Gain Factor */
    
    uint32_t    ui32GainFactor;
                    /*  Valid for Dolby Pulse(MS10) only
                    Similar to Volume Gain in 8.24 format. 
                        Examples:
                        0x00800000 : unity 
                        0x00B504F3 : -3dB */
    
    uint16_t    ui16DrcDefaultLevel;            
                    /*  Valid for Dolby Pulse(MS10) only
                    Dynamic range compression default level.Input value 
                        range = 0 to 127. */

    bool                            bMsUsageMode;  /*True if MS11/MS10 usage case, False otherwise */
    bool                            bMpegConformanceMode;  /* Set to true for MPEG conformance testing, false otherwise. 
                                                            If true, bMsUsageMode is ignored. */
} BRAP_DSPCHN_AacSbrConfigParams;


typedef struct  BRAP_DSPCHN_Ac3ConfigParams BRAP_DSPCHN_Ac3PlusConfigParams;


/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	DTS stereo mode. This eum is effective only when stream is
	Annex-D compliant and 
	BRAP_DEC_AudioParams.eOutputMode = BRAP_OutputMode_eStereo. 	
	This is also used for DTS-HD.
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsStereoMode
{
	BRAP_DSPCHN_DtsStereoMode_eAuto,
	BRAP_DSPCHN_DtsStereoMode_eLtRt,
	BRAP_DSPCHN_DtsStereoMode_eLoRo
} BRAP_DSPCHN_DtsStereoMode;

/***************************************************************************
Summary:
	This structure contains DTS decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DtsConfigParams
{
    bool                        bEnableDRC;
                                /* user DRC flag  TRUE for enable, FALSE for disable*/              

	uint16_t			        ui16DynRngScaleHi;
                                /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
								
	uint16_t			        ui16DynRngScaleLo; 
                                /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 

	bool			            bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
								
	int32_t		                i32UserDownmixCoeff[8][8];
								/* TODO: Put proper comments */
    /* Below params decide what extensions to be decoded. */
    bool                        bDecodeCoreOnly; 
                                /* This param has precedence over other. If set 
                                   to true, FW will decode only core. */
    bool                        bDecodeDtsOnly;
                                /* This param has precedence less than Core_Only and 
                                   more than others. */
    bool                        bDecodeXll;
                                /* This param has precedence less than Core_Only 
                                   & DTS_ONLY and more than others. */
    bool                        bDecodeX96;
    bool                        bDecodeXch;
    bool                        bDecodeXxch;
    bool                        bDecodeXbr;								
} BRAP_DSPCHN_DtsConfigParams;


/***************************************************************************
Summary:
	This structure contains DTS Broadcast decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DtsBroadcastConfigParams
{
    bool                        bEnableDRC;
                                /* user DRC flag  TRUE for enable, FALSE for disable*/              

	uint16_t			        ui16DynRngScaleHi;
                                /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
								
	uint16_t			        ui16DynRngScaleLo; 
                                /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 
    BAVC_AudioSamplingRate      eAppSampleRate;
                                /*This is the sampling rate set by application layer. The decoder 
                                will limit decoding additional components in the stream (e.g, X96)
                                to adjust the sampling rate of the decoded output to that set by the application. */

	bool			            bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
								
	int32_t		                i32UserDownmixCoeff[8][8];
								/* Downmix coefficients */
                                    
    /* Below params decide what extensions to be decoded. */
    bool                        bDecodeCoreOnly; 
                            	/*Default=1; Range : 0,1 (1-> Decode only core, 
                            	Disable all extension components)
	                            This flag has precedence over other decode flags like
	                            ui32DecodeDtsOnly,  ui32DecodeXLL etc) */

    bool                        bDecodeDtsOnly;
                                /* This param has precedence less than Core_Only and 
                                   more than others. 
                                   Default=0; Range : 0,1 (1-> decode components present in core substream only)*/
    bool                        bDecodeXll;
                                /* This param has precedence less than Core_Only 
                                   & DTS_ONLY and more than others. 
                                   Default=1; Range : 0,1 (0-> does not decode XLL component)*/
    bool                        bDecodeX96;
                                /*Default=1; Range : 0,1 (0-> does not decode X96 component)*/
    bool                        bDecodeXch;
                                /*Default=1; Range : 0,1 (0-> does not decode XCH component)*/
    bool                        bDecodeXxch;
                                /*Default=1; Range : 0,1 (0-> does not decode XXCH component)*/
    bool                        bDecodeXbr;			
                                /*Default=1; Range : 0,1 (0-> does not decode XBR component)*/
    bool                        bEnableSpkrRemapping;
                                    /*Enable Speaker remapping*/
    unsigned int                ui32SpkrOut;
                                    /*Specifies the speaker out configuration. Instructs the decoder
                                    to perform downmixing or speaker remapping to yield the desired 
                                    output speaker configuration. 
                                        SpkrOut  Output Speaker Configuration 
                                        0  Use Encoded Native (default) 
                                        1  - C  
                                        9  - C LFE1 
                                        2  - L R  
                                        10  - L R LFE1 
                                        262144  Lt Rt (downmix) 
                                        3  - C L R  
                                        11  - C L R LFE1 
                                        18  - L R Cs 
                                        26  - L R Cs LFE1 
                                        19  - C L R Cs 
                                        27  - C L R Cs LFE1 
                                        6  - L R Ls Rs 
                                        14  - L R Ls Rs LFE1 
                                        15  - C L R Ls Rs LFE1 
                                        8207  - C L R Ls Rs LFE1 Lhs Rhs 
                                        2123  - C L R LFE1 Lsr Rsr Lss Rss 
                                        47  - C L R Ls Rs LFE1 Lh Rh 
                                        79  - C L R Ls Rs LFE1 Lsr Rsr 
                                        159  - C L R Ls Rs LFE1 Cs Ch 
                                        287  - C L R Ls Rs LFE1 Cs Oh 
                                        1039  - C L R Ls Rs LFE1 Lw Rw 
                                        31  - C L R Ls Rs LFE1 Cs */

    bool                        bMixLFE2Primary;
                                    /*Mix LFE to primary while downmixing, when Lfe output is disabled)*/

    unsigned int                ui32ChReplacementSet;   
                                    /*It can take all possible values from 0 to 7. But it is 
                                    not really required in core decoder. But, it will be needed
                                    in DTS-HD decoder going forward.*/

	bool                        bEnableMetadataProcessing;		
                                    /* Flag that controls the Metadata processing. 
                                       This flag should be disabled for DTS DMP license and 
                                       can be enabled for other DTS profile licenses.
                                       Default = false; 
                                       false -> Disable Metadata processing, 
                                       true -> Enable Metadata processing */
} BRAP_DSPCHN_DtsBroadcastConfigParams;

/***************************************************************************
Summary:
	This structure contains DTS-HD decoder configuration parameters
***************************************************************************/
typedef struct  BRAP_DSPCHN_DtsBroadcastConfigParams BRAP_DSPCHN_DtsHdConfigParams;

#if 0
typedef struct BRAP_DSPCHN_DtsHdConfigParams
{
    bool                        bEnableDRC;
                                /* user DRC flag  TRUE for enable, FALSE for disable*/              

	uint16_t			        ui16DynRngScaleHi;
                                /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
								
	uint16_t			        ui16DynRngScaleLo; 
                                /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 
    BAVC_AudioSamplingRate    eAppSampleRate;
                                /*This is the sampling rate set by application layer. The decoder 
                                will limit decoding additional components in the stream (e.g, X96)
                                to adjust the sampling rate of the decoded output to that set by the application. */

	bool			            bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
								
	int32_t		                i32UserDownmixCoeff[8][8];
								/* Downmix coefficients */
                                    
    /* Below params decide what extensions to be decoded. */
    bool                        bDecodeCoreOnly; 
                            	/*Default=1; Range : 0,1 (1-> Decode only core, 
                            	Disable all extension components)
	                            This flag has precedence over other decode flags like
	                            ui32DecodeDtsOnly,  ui32DecodeXLL etc) */

    bool                        bDecodeDtsOnly;
                                /* This param has precedence less than Core_Only and 
                                   more than others. 
                                   Default=0; Range : 0,1 (1-> decode components present in core substream only)*/
    bool                        bDecodeXll;
                                /* This param has precedence less than Core_Only 
                                   & DTS_ONLY and more than others. 
                                   Default=1; Range : 0,1 (0-> does not decode XLL component)*/
    bool                        bDecodeX96;
                                /*Default=1; Range : 0,1 (0-> does not decode X96 component)*/
    bool                        bDecodeXch;
                                /*Default=1; Range : 0,1 (0-> does not decode XCH component)*/
    bool                        bDecodeXxch;
                                /*Default=1; Range : 0,1 (0-> does not decode XXCH component)*/
    bool                        bDecodeXbr;			
                                /*Default=1; Range : 0,1 (0-> does not decode XBR component)*/
    bool                        bEnableSpkrRemapping;
                                    /*Enable Speaker remapping*/
    unsigned int             ui32SpkrOut;
                                    /*Specifies the speaker out configuration. Instructs the decoder
                                    to perform downmixing or speaker remapping to yield the desired 
                                    output speaker configuration. 
                                        SpkrOut  Output Speaker Configuration 
                                        0  Use Encoded Native (default) 
                                        1  - C  
                                        9  - C LFE1 
                                        2  - L R  
                                        10  - L R LFE1 
                                        262144  Lt Rt (downmix) 
                                        3  - C L R  
                                        11  - C L R LFE1 
                                        18  - L R Cs 
                                        26  - L R Cs LFE1 
                                        19  - C L R Cs 
                                        27  - C L R Cs LFE1 
                                        6  - L R Ls Rs 
                                        14  - L R Ls Rs LFE1 
                                        15  - C L R Ls Rs LFE1 
                                        8207  - C L R Ls Rs LFE1 Lhs Rhs 
                                        2123  - C L R LFE1 Lsr Rsr Lss Rss 
                                        47  - C L R Ls Rs LFE1 Lh Rh 
                                        79  - C L R Ls Rs LFE1 Lsr Rsr 
                                        159  - C L R Ls Rs LFE1 Cs Ch 
                                        287  - C L R Ls Rs LFE1 Cs Oh 
                                        1039  - C L R Ls Rs LFE1 Lw Rw 
                                        31  - C L R Ls Rs LFE1 Cs */

    bool                        bMixLFE2Primary;
                                    /*Mix LFE to primary while downmixing, when Lfe output is disabled)*/

    unsigned int             ui32ChReplacementSet;   
                                    /*It can take all possible values from 0 to 7. But it is 
                                    not really required in core decoder. But, it will be needed
                                    in DTS-HD decoder going forward.*/


} BRAP_DSPCHN_DtsHdConfigParams;
#endif

/***************************************************************************
Summary:
	This structure contains BD LPCM configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_BdlpcmConfigParams
{
	bool			bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		i32UserDownmixCoeff[8][8];
								/* TODO: Put proper comments */
} BRAP_DSPCHN_BdlpcmConfigParams;

/***************************************************************************
Summary:
	This structure contains  LPCM  HD DVD configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_LpcmHdDvdConfigParams
{
	bool			bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		i32UserDownmixCoeff[32][8];
								/* TODO: Put proper comments */
} BRAP_DSPCHN_LpcmHdDvdConfigParams; 

/***************************************************************************
Summary:
	This structure contains  LPCM  DVD configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_LpcmDvdConfigParams
{
	bool			bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		i32UserDownmixCoeff[8][8];
								/* TODO: Put proper comments */
} BRAP_DSPCHN_LpcmDvdConfigParams; 

/***************************************************************************
Summary:
	This enumeration specifies the size of the speakers that the TruBass sound will be 
	played back on. TruBass adjusts its filter parameters to optimize playback for that size.
	Size should be selected based on the lowest reproducible frequency that is just above
	the specified value:
***************************************************************************/
typedef enum BRAP_DSPCHN_TbSpeakerSize
{
	BRAP_DSPCHN_TbSpeakerSize_eLfr40Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr60Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr100Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr150Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr200Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr250Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr300Hz,
	BRAP_DSPCHN_TbSpeakerSize_eLfr400Hz
} BRAP_DSPCHN_TbSpeakerSize;

/***************************************************************************
Summary:
	This enumeration specifies the mode of decode. 
***************************************************************************/
typedef enum BRAP_DSPCHN_SRS_AcMode
{
    BRAP_DSPCHN_SRS_AcMode_eMode10 = 1, /*C*/
    BRAP_DSPCHN_SRS_AcMode_eMode20 = 2, /*L,R*/
    BRAP_DSPCHN_SRS_AcMode_eMode30 = 3, /*L,C,R*/
    BRAP_DSPCHN_SRS_AcMode_eMode21 = 4, /*L,R,S*/
    BRAP_DSPCHN_SRS_AcMode_eMode31 = 5, /*L,C,R,S*/
    BRAP_DSPCHN_SRS_AcMode_eMode22 = 6, /*L,R,LS,RS*/
    BRAP_DSPCHN_SRS_AcMode_eMode32 = 7, /*L,C,R,LS,RS*/
    BRAP_DSPCHN_SRS_AcMode_eMode33 = 8, /*L,C,R,LS,RS,CS*/
    BRAP_DSPCHN_SRS_AcMode_eMode32_BSDigital = 9,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_SRS_AcMode_eModeLtRt=0x20,/*L,R*/
    BRAP_DSPCHN_SRS_AcMode_eModePLII_Movie = 0x21,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_SRS_AcMode_eModePLII_Music = 0x22,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_SRS_AcMode_eModePassiveMatrix = 0x23,/*L,R*/
    BRAP_DSPCHN_SRS_AcMode_eModeCSII = 0x24,/*L,C,R,Ls,Rs,Cs*/
    BRAP_DSPCHN_SRS_AcMode_eLAST,
    BRAP_DSPCHN_SRS_AcMode_eINVALID=0x7fffffff
}BRAP_DSPCHN_SRS_AcMode;

/***************************************************************************
Summary:
	This structure contains  TruSurroundXt configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_TruSurroundXtParams
{
	bool		bTruSurroundEnable;
							/* Turns the TruSurround portion of the TruSurroundXT 
							processing on and off.
							 TRUE = Turn ON TruSurround
							 FALSE = Turn OFF TruSurround */
	bool		bTsHeadphoneEnable;
							/* Turns the headphone processing of the TruSurround 
							process on and off.
							 TRUE = Turn ON headphone processing
							 FALSE = Turn OFF headphone processing */
	unsigned int	uiTruSurroundInputGain;	/* Pre-amplifier gain for SRS-XT. The valid range is
							0 to 200% */
	bool		    bDialogClarityEnable;
							/* Enables the Dialog Clarity portion of the TruSurroundXT 
							processing.
							TRUE = Enable Dialog clarity
							FALSE = Disable Dialog clarity */
    unsigned int    uiDialogClarityLevel;
							/* Dialog clarity. Valid only if bDialogClarityEnable = TRUE
							Sets the Dialog Clarity level.
							The valid range is 0% to 100% */                            
	bool		    bTruBassEnable;
							/* Enables the TruBass portion of the processing. TruBass has 
							no effect if TruSurround is disabled. */
	unsigned int    bTbLevel;
							/* TruBass Level.  Valid only if bTruBassEnable = TRUE.
							Specifies the amount of TruBass that is 
							mixed into the final signal. The valid range is 0% to 100% */
	BRAP_DSPCHN_TbSpeakerSize	eTbSpeakerSize;
							/* Specifies the size of the speakers that the sound will be 
							played back on. */
    bool            bCertificationEnable;
                            /* Keeps the processing in certification mode 
                               TRUE = Enables Certification mode
                               FALSE = Disables Certification mode */
    BRAP_DSPCHN_SRS_AcMode      eAcMode; 
                                /*Varies depending on the input mode to be certified*/							
} BRAP_DSPCHN_TruSurroundXtParams;


typedef struct BRAP_DSPCHN_DolbyVolumeUserConfig
{
	/*General Settings*/

	bool		bDolbyVolEna;				/*Range of values -  0 or 1 */
												/*0 - dolby volume disabled - acts as pass trough node*/
												/*1 - dolby volume enabled   */ 
												/*Default value    : 1 ( dolby volume enabled)  */

	int		iBlockSize;					/*Size of processing block in samples */
												/*Range of values - 256, 512 */
												/*Default value    : 512 */
                                                
	unsigned int 		uiBands;						/*Number of critical bands to use */
												/*Possible values 20 or 40, default 20*/

        unsigned int    uiNumInputOutputChannel;        /*Number of input/output channels */
                                                                                /*Default val 2 , Possible values 1,2*/

	/* System Settings*/
	unsigned int		uiInputReferenceLevel;			/*Input reference level in dBSPL */
                                                                                    /*RAP Range of values- (0 to 100)*/
                                                                                    /*Spec Range of values(in dB )- (0 to 100 )*/
                                                                                    /*RAP Default value : (74 dB)*/                                                
                                                                                    /*Spec Default value : (74 dB)*/                                                


	unsigned int		uiOutputReferenceLevel;		/* Output reference level in dBSPL */
                                                                                    /*RAP Range of values- (0 to 100)*/
                                                                                    /*Spec Range of values(in dB )- (0 to 100 )*/
                                                                                    /*RAP Default value : (74 dB)*/                                                
                                                                                    /*Spec Default value : (74 dB)*/  


	int		iCalibration;					/*Calibration offset in dB */
                                                                        /*RAP Range of values- (-100 to 30)*/
                                                                    /*Range of values(in dB )- (-100 to 30 )*/  
                                                                    /*RAP Default value : (0 dB)*/                                                
                                                                    /*Spec Default value : (0 dB)*/  

                                                
	bool		bVolModelerEna;					/*Volume modeler on/off */
												/*Possible value 0 and 1, default 1*/
                                                
	bool		bResetNowFlag;				/*User-input forced reset flag */
												/*Range [0,1], default 0*/


	/*Volume Modeler Settings*/
	int		iDigitalVolumeLevel;			/*Volume level gain in dB -- applied by dolby volume */
                                                                        /*RAP Range of values- (-100 to 30)*/
                                                                    /*Range of values(in dB )- (-100 to 30 )*/  
                                                                    /*RAP Default value : (0 dB)*/                                                
                                                                    /*Spec Default value : (0 dB)*/  


	int		iAnalogVolumeLevel;			/*Volume level gain in dB -- applied after dolby volume */
                                                                            /*RAP Range of values- (-100 to 30)*/
                                                                        /*Range of values(in dB )- (-100 to 30 )*/  
                                                                        /*RAP Default value : (0 dB)*/                                                
                                                                        /*Spec Default value : (0 dB)*/  


	/*Volume Leveler Settings */
	unsigned int		uiVolLvlAmount;					/*Range of values - [0 to 10]*/
												/*Default value		: 0*/
	
	bool		bVolLvlEnable;					/*Range of values -  0=FALSE , 1=TRUE*/
												/*Default value    : 1 (TRUE - volume leveler enabled)*/
	
	bool		bEnableMidsideProc;			/*Enable midside processing */
                                                                /*Range of values -  0=FALSE , 1=TRUE*/
												/*, default ->false-> disable*/

	bool     bHalfmodeEna;				/*Flag to operate Dolby Volume in half mode*/
		                                                                /*Range of values -  0=FALSE , 1=TRUE*/
												/*, default ->false-> disable*/
        bool    bLimiterEnable;                            /*Enable Limter*/ 
									/*When DV is enabled limiter_enable = TRUE
									  When DV is disabled limiter_enable = FALSE*/

}BRAP_DSPCHN_DolbyVolumeUserConfig;


/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	ProLogic-II decoder mode  
***************************************************************************/
typedef enum BRAP_DSPCHN_PLllDecMode
{
	BRAP_DSPCHN_PLllDecMode_Emulation = 0,
	BRAP_DSPCHN_PLllDecMode_VirtualCompatible,
	BRAP_DSPCHN_PLllDecMode_Music,
	BRAP_DSPCHN_PLllDecMode_MovieRStandard,
	BRAP_DSPCHN_PLllDecMode_Matrix,
	BRAP_DSPCHN_PLllDecMode_Custom = 7,
	BRAP_DSPCHN_PLllDecMode_eMax
} BRAP_DSPCHN_PLllDecMode;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	ProLogic-II center width config
***************************************************************************/
typedef enum BRAP_DSPCHN_PLllCenterWidthCfg
{
	BRAP_DSPCHN_PLllCenterWidthCfg_0_Degree,		/* Zero Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_20p8_Degree,	/* 20.8 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_28_Degree,		/* 28 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_36_Degree,		/* 36 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_54_Degree,		/* 54 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_62_Degree,		/* 62 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_69p2_Degree,	/* 69.2 Degree */
	BRAP_DSPCHN_PLllCenterWidthCfg_90_Degree		/* Phantom center */
} BRAP_DSPCHN_PLllCenterWidthCfg;

/***************************************************************************
Summary:
	This enumeration defines output channel arrangement when debug is ON
***************************************************************************/
typedef enum BRAP_DSPCHN_PLllDebugChannelCfg
{
	BRAP_DSPCHN_PLllDebugChannelCfg_eLR,
	BRAP_DSPCHN_PLllDebugChannelCfg_eLsRs,
	BRAP_DSPCHN_PLllDebugChannelCfg_eLC
} BRAP_DSPCHN_PLllDebugChannelCfg;


/***************************************************************************
Summary:
	This structure contains PL-II configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_ProLogicllConfigParams
{
    bool            bEnablePLII;    /* Flag to enable/disable PLII 
										[true]  - Enable (Default)
										[false] - Disable */

	bool			bEnableAutoBal;			/* Flag to Enable/Disable auto balance
										[true]  - Enable
										[false] - Disable */

	BRAP_DSPCHN_PLllDecMode			ePLllDecMode;	/* PL-II Decoder Mode */
	BRAP_OutputMode					eOutputMode;	/* output channel configuration */
	BRAP_DSPCHN_PLllCenterWidthCfg	eCenterWidCfg;	/* Center width control settings in degree */
	uint32_t		ui32DimSetting;			/* This has the value ranging from -3 to +3
										0 - -3 (most surround)
										1 - -2
										2 - -1
										3 -  0 (neutral)
										4 - +1
										5 - +2
										6 - +3 (most center) */
	bool			bEnablePanorama;		/* Flag to Enable/Disable Panaroma
										[true]  - Enable
										[false] - Disable */
										
	bool			bEnableSurChnShelfFilt;	/* Flag to Enable/Disable Surround channel output
										Shelf filter.
										[true]  - Enable
										[false] - Disable */

	bool			bEnableRSurPolInv;		/* Flag to Enable/Disable right surround output
										polarity inversion.
										[true]  - Enable
										[false] - Disable */
										  
	uint32_t		ui32PcmScaleFactor;		/* PCM scale factor. Input value range = 0 to 100%
										100% corresponds to value of unity.Any value 
										above 100% is considered as unity. Each decoded
										PCM sample gets scaled by this factor */
	bool			bDebugOnOff;			/* Flag to enable debug output
										0 - ON; 1-OFF */
	BRAP_DSPCHN_PLllDebugChannelCfg	eDbgChanCfg;	/* Channel config for debug out */
} BRAP_DSPCHN_ProLogicllConfigParams;

/***************************************************************************
Summary:
	This structure contains AVL configuration parameters
	WARNING - These parameters control the audio performance of the AVL algorithm.
	Except for bBypass, changing values of other parameters from default will modify 
	automatic volume level character. Modify only after careful study and analysis.
***************************************************************************/
typedef struct BRAP_DSPCHN_AVLConfigParams
{
	/* TODO: The configuration parameters for this algo is not
	very clear and descriptive at this time. Will be added in future */
	bool			bBypass;				/* Put AVL post process stage in bypass mode.
										        [ true ] - Bypass AVL
										        [ false ] - AVL Processing */
	int			    iTarget;				/* Target level of output signal in terms of dBFs */
	int	            iLowerBound;			/* Lower bound for primary range of volume control */
	unsigned int	uiFixedBoost;			/* Amount of fixed boost (in dB) if level < LOWERBOUND */
	unsigned int	uiRef;					/* Reference logarithmic gain value of 0 dBFs */
	unsigned int	uiAlpha;				/* Attenuation factor for Level Maximum */
	unsigned int	uiBeta;					/* Attenuation factor for Level Mean */
	unsigned int	uiThreshold;			/* Threshold for detecting active portion of signal . Input value should be
										        multiplied by factor of 10 */
	unsigned int	uiDtfPcnt;				/* Decay Time for DTF Percent delay (in seconds) */
	unsigned int	uiAlpha2;				/* Attenuation factor for Level Minimum */
	unsigned int	uisNsfgr;				/* Fast Gain Ramp in milliseconds. Input value should be
										        multiplied by factor of 10 */
	unsigned int	uiDtf;					/* Decay Time Fraction. Input value should be
										        multiplied by factor of 10 */
	unsigned int   i32LoudnessLevelEq;      /* Enable =1 is AVL2.0 and Disable =0 is AVL1.0*/
} BRAP_DSPCHN_AVLConfigParams;

/***************************************************************************
Summary:
	This structure contains post processing algorithm XEN configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_XenConfigParams
{
	bool		bBypassXen;					/* Bypass XEN post processing 
										     true = Bypass XEN
										     false = Don't bypass XEN */
	bool		bApply6dBHeadroom;		/* Apply -6 dB headroom 
										    true = Apply
										    false = Don't apply */
	int		iBoostCenter;				/* Reinforce gain for center sound in dbFs.
										     Valid input range is from -24dBFs to 6 dbFs.
										     If value is less than -24dbFs, then it is
										     considered as -ve Infinite which is a vaalid 
										     value. Value greater than 6dbFs is considered
										     as 6dbFs */
	int		iLevelFront;					/* Output gain ( in dbFs ) for front sound.
										     Valid input range is from -24dBFs to 6 dbFs.
										     If value is less than -24dbFs, then it is
										     considered as -ve Infinite which is a vaalid 
										     value. Value greater than 6dbFs is considered
										     as 6dbFs */
	int		iLevelRear;					/* Output gain ( in dbFs ) for rear sound.
										     Ignored for 2ch input.
										     Valid input range is from -24dBFs to 6 dbFs.
										     If value is less than -24dbFs, then it is
										     considered as -ve Infinite which is a vaalid 
										     value. Value greater than 6dbFs is considered
										     as 6dbFs */
	int		iLevelLfe;					/* Output gain ( in dbFs ) for centre sound.
										     Ignored for 2ch input.
										     Valid input range is from -24dBFs to 6 dbFs.
										     If value is less than -24dbFs, then it is
										     considered as -ve Infinite which is a vaalid 
										     value. Value greater than 6dbFs is considered
										     as 6dbFs */
	int		iLevelOutput;				/* Overall gain.
										     Valid input range is from -24dBFs to 6 dbFs.
										     If value is less than -24dbFs, then it is
										     considered as -ve Infinite which is a vaalid 
										     value. Value greater than 6dbFs is considered
										     as 6dbFs */
	unsigned int	uiSp2Sp;				/* Distance between two speakers.
										    Valid input range is from 20cm to 220cm
										    with step of 2 cm */
	unsigned int 	uiSp2Ls;					/* Distance between the center of TV speakers 
										    and listener. Valid input range is from 0 to 9.
										    Larger value indicates closer distance. */
	unsigned int	uiExpandMidHigh;		/* Level of mi-high sound range. Valid input
										     range is from 0 to 10. Larger number indicates 
										     strength level */
	unsigned int	uiExpandMidLow;		/* Level of mi-low sound range. Valid input
										     range is from 0 to 10. Larger number indicates 
										     strength level */
	unsigned int	uiLpfFc;					/* IIR filter's cut-off frequency. Valid input range
										     is from 80 Hz to 270 Hz with steps of 10 Hz */
	unsigned int	uiPfBoostGain;			/* level of bass sound range. Valid input
										     range is from 0 to 10. Larger number indicates 
										     strength level */
} BRAP_DSPCHN_XenConfigParams;

/***************************************************************************
Summary:
	This enumaration defines BBE types
***************************************************************************/
typedef enum BRAP_DSPCHN_BbeType
{
	BRAP_DSPCHN_BbeType_e1,			/* BBE I */
	BRAP_DSPCHN_BbeType_e2,			/* BBE II */
	BRAP_DSPCHN_BbeType_e3			/* BBE III */
} BRAP_DSPCHN_BbeType;

/***************************************************************************
Summary:
	This structure contains post processing algorithm BBE configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_BbeConfigParams
{
	bool		bBypassBbe;				/* Bypass BBE post processing 
										     true = Bypass BBE
										     false = Don't bypass BBE */
	BRAP_DSPCHN_BbeType eBbeType;		/* BBE Type. Currently this
										    parameter is not supported
										    in FW. FW has BBE I 
										    implementation */
	bool		bAcousticFlatSystem;		/* Acoustic Flat System 
										     true = Apply
										     false = Don't apply.
										     This parameter is not supported
										     in FW. */
	unsigned int 	uiLoContour;				/* Valid input range is 0 to 24 
										     corresponding to 0 to 12 dB with
										     step size of 0.5 dB */
	unsigned int	uiProcess;				/* Valid input range is 0 to 24 
										     corresponding to 0 to 12 dB with
										     step size of 0.5 dB */
	bool		bApplyHeadroom;			/* Apply Headroom
										     true = Apply headroom. If
										     max(lo_contour, process) > 6dB,
										     input is scaled down by -12 dB
										     internally. Otherwise scaled down
										     by -6 dB.
										     false = Don't apply headroom */
} BRAP_DSPCHN_BbeConfigParams;

/***************************************************************************
Summary:
	This structure contains post processing algorithm CUSTOM SURROUND configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_CustomSurroundConfigParams
{
	bool            bBypassCustomSurround;        /* Bypass CUSTOM SURROUND post processing 
										            true = Bypass CUSTOM SURROUND
										            false = Don't bypass CUSTOM SURROUND */
	bool            bCombOn;                      /* Switch Surround Comb */
	unsigned int 	uiDelay;                      /* delay in the unit of samples 
                                                         for cross-talk-cancelling 
                                                         Values:0x00 - 0x1F for 0 to 31 samples
                                                         Other Values are reserved */
	unsigned int	uiVolume1;                    /* L/R expand level */
	unsigned int	uiVolume2;                    /* Ls/Rs expand level */
	unsigned int	uiVolume3;                    /* Center channel gain 
                                                                   0x0 - 0xC for 0dB to 12dB
                                                                   0xF for Mute
                                                                   Other Values are reserved */
	unsigned int	uiVolume4;                    /* input gain control for 2-ch input */
	unsigned int	uiVolume5;                    /* input gain control for 5.1-ch input 
                                                                    0x0  - 12.0 dB
                                                                    0x1  - 11.5 dB
                                                                    0x2  - 11.0 dB
                                                                    0x3  - 10.5 dB
                                                                    ...
                                                                    0x18 - 0 dB
                                                                    ...
                                                                    0x3D - -18.5 dB
                                                                    0x3E - -19.0 dB
                                                                    0x3F - -19.5 dB
                                                                    0x40 - -20.0 dB
                                                                    others - reserved */
	unsigned int	uiLpfFc;                         /* cut-off frequency of Low Pass Filter (LPF)
                                                                    0x70 - 5.0 kHz
                                                                    0x71 - 5.6 kHz
                                                                    0x72 - 6.3 kHz
                                                                    0x73 - 7.0 kHz
                                                                    0x74 - 8.0 kHz
                                                                    0x75 - 9.0 kHz
                                                                    0x76 - 10.0 kHz
                                                                    others - reserved */
	unsigned int	uiLpfQ;                          /* Q value for LPF
                                                                    0x40 - 0.33
                                                                    0x41 - 0.43
                                                                    0x42 - 0.56
                                                                    0x43 - 0.75
                                                                    0x44 - 1.00
                                                                    0x45 - 1.20
                                                                    0x46 - 1.50
                                                                    0x47 - 1.80
                                                                    0x48 - 2.20
                                                                    0x49 - 2.70
                                                                    0x4A - 3.30
                                                                    0x4B - 3.90
                                                                    0x4C - 4.70
                                                                    0x4D - 5.60
                                                                    0x4E - 6.80
                                                                    0x4F - 8.20
                                                                    others - reserved */
	unsigned int	uiLpfGain;                      /* gain of LPF
                                                                    0x4C - 12 dB
                                                                    0x4B - 11 dB
                                                                    ...
                                                                    0x41 - 1 dB
                                                                    0x40 - 0 dB
                                                                    0x51 - -1 dB
                                                                    0x52 - -2 dB
                                                                    ...
                                                                    0x5B - -11 dB
                                                                    0x5C - -12 dB
                                                                    others - reserved */

    unsigned int    uiFuncVol;                      /* The LR Function Volume Gain
                                                        Same as table above for uiLpfGain */

    unsigned int    uiTrim;                         /* The Input Trim value Gain
                                                        0x00 to 0x20 corresponding to 0 to -32 */                                                                        
                                                                        
} BRAP_DSPCHN_CustomSurroundConfigParams;

/***************************************************************************
Summary:
	This enumaration defines CustomBass Operation types
***************************************************************************/
typedef enum BRAP_DSPCHN_CustomBassOperation
{
	BRAP_DSPCHN_CustomBassOperation_eAgcAndHarmonics,	    /* AGC and harmonics */
	BRAP_DSPCHN_CustomBassOperation_eAgcOnly,			    /* AGC only */
	BRAP_DSPCHN_CustomBassOperation_HarmonicsOnly           /* Harmonics only */
} BRAP_DSPCHN_CustomBassOperation;

/***************************************************************************
Summary:
	This enumaration defines CustomBass AGC types
***************************************************************************/
typedef enum BRAP_DSPCHN_CustomBassAgcType
{
	BRAP_DSPCHN_CustomBassAgcType_eUseLowAndHighForMonitor, /* Use low and high freq for monitor */
	BRAP_DSPCHN_CustomBassAgcType_eUseOnlyLowForMonitor		/* Use only low freq for monitor */
} BRAP_DSPCHN_CustomBassAgcType;

/***************************************************************************
Summary:
	This enumaration defines CustomBass Harmonics types
***************************************************************************/
typedef enum BRAP_DSPCHN_CustomBassHarmonicsType
{
	BRAP_DSPCHN_CustomBassHarmonicsType_eHalfWaveRectifier,     /* Half Wave Rectifier */
	BRAP_DSPCHN_CustomBassHarmonicsType_eHalfWaveSqRectifier,   /* Half Wave Square Rectifier */
	BRAP_DSPCHN_CustomBassHarmonicsType_eFullWaveRectifier      /* Full Wave Rectifier */
} BRAP_DSPCHN_CustomBassHarmonicsType;

/***************************************************************************
Summary:
	This structure contains post processing algorithm CUSTOM BASS configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_CustomBassConfigParams
{
    bool                            bBypassCustomBass;              /* Bypass CUSTOM BASS post processing 
        								                                true = Bypass CUSTOM BASS 
        								                                false = Don't bypass CUSTOM BASS */
    BRAP_DSPCHN_CustomBassOperation     eCustomBassOperation;       /* Operation Mode */
    BRAP_DSPCHN_CustomBassAgcType       eCustomBassAgcType;         /* Agc Type */
    BRAP_DSPCHN_CustomBassHarmonicsType eCustomBassHarmonicsType;   /* Harmonics Type */
   
    
    int                             iAgcGainMax;        /* Maximum Gain of AGC Boost
                                                            Valid range :   -12dB to +12dB 
                                                            Step Size   :   1 dB */
    int                             iAgcGainMin;        /* Minimum Gain of AGC Boost
                                                            Valid range :   -12dB to +12dB 
                                                            Step Size   :   1 dB */
    unsigned int                    uiAgcAttackTime;    /* Time of Decreasing to Minimum AGC Gain 
                                                            0x0 - 1 ms
                                                            0x1 - 2 ms
                                                            0x2 - 5 ms
                                                            0x3 - 10 ms
                                                            0x4 - 20 ms
                                                            0x5 - 50 ms
                                                            others - reserved */
    unsigned int                    uiAgcRelTime;       /* Time of Increasing to Maximum AGC Gain
                                                            0x0 - 10 ms
                                                            0x1 - 20 ms
                                                            0x2 - 50 ms
                                                            0x3 - 100 ms = 1 second
                                                            0x4 - 200 ms = 2 seconds
                                                            0x5 - 500 ms = 5 seconds
                                                            others - reserved */
    int                             iAgcThreshold;      /* Level of AGC Detection 
                                                            Valid range :   0 to -37dB 
                                                            Step Size   :   1 dB */
    unsigned int                    uiHarmonicsLevel;   /* Harmonics Gain 
                                                            Valid range :   0 to 12dB
                                                            Step Size   :   1 dB */                                                                
    unsigned int                    uiLpfFc;            /* Cut-off frequency of Low Pass Filter (LPF)
                                                            See Cut Off Frequency Table below for the values */
    unsigned int                    uiHpfFc;            /* Cut-off frequency of High Pass Filter (HPF)
                                                             See Cut Off Frequency Table below for the values */
    unsigned int                    uiAgcHpfFc;         /* Cut-off frequency of AGC HPF  
                                                             See Cut Off Frequency Table below for the values */    
    unsigned int                    uiHarmonicsLpfFc;   /* Cut-off frequency of Harmonics LPF
                                                             See Cut Off Frequency Table below for the values */    
    unsigned int                    uiHarmonicsHpfFc;   /* Cut-off frequency of Harmonics HPF
                                                             See Cut Off Frequency Table below for the values */    

                                                        /* Table for Cut Off Frequency 
                                                            0x0 - 40 Hz
                                                            0x1 - 45 Hz
                                                            0x2 - 50 Hz
                                                            0x3 - 56 Hz
                                                            0x4 - 63 Hz
                                                            0x5 - 70 Hz
                                                            0x6 - 80 Hz
                                                            0x7 - 90 Hz
                                                            0x8 - 100 Hz
                                                            0x9 - 110 Hz
                                                            0xA - 125 Hz
                                                            0xB - 140 Hz
                                                            0xC - 160 Hz
                                                            0xD - 180 Hz
                                                            0xE - 200 Hz
                                                            0xF - 220 Hz
                                                            0x10 - 250 Hz
                                                            0x11 - 280 Hz
                                                            0x12 - 315 Hz
                                                            others - reserved */                                                             
} BRAP_DSPCHN_CustomBassConfigParams;


/***************************************************************************
Summary:
	This structure contains frequency dependent parameters of Custom Voice
***************************************************************************/
typedef struct BRAP_DSPCHN_CVFreqDependentParams
{
    int32_t i32param20;
    int32_t i32param21;
    int32_t i32param22;
    int32_t i32param42;
    int32_t i32param46[7];
    int32_t i32param47[7];
    int32_t i32param94[20];
    int32_t i32param109;
    int32_t i32param110;
    int32_t i32param19;
    int32_t i32param51[8];
    int32_t i32param50;
    int32_t i32param73;
    int32_t i32param74;
    int32_t i32param64;
    int32_t i32param63;
    int32_t i32param61;
    int32_t i32param62;
	int32_t i32param33;
	int32_t i32param34;
	int32_t i32param35;
	int32_t i32param36;
	int32_t i32param37;
	int32_t i32param38;
	int32_t i32param111;
	int32_t i32param67;
	int32_t i32param68;
	int32_t i32param69;
	int32_t i32param70;
	int32_t i32param18;
	int32_t i32param17;    
} BRAP_DSPCHN_CVFreqDependentParams;

/***************************************************************************
Summary:
	This structure contains post processing algorithm CUSTOM VOICE configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_CustomVoiceConfigParams
{
    int32_t i32CVEnableFlag;
    int32_t i32param00;
    int32_t i32param02;
    int32_t i32OSDVolume;
    int32_t i32param65;
    int32_t i32param66;
    int32_t i32param71;
    int32_t i32param72;
    int32_t i32param39;
    int32_t i32param40;
    int32_t i32param48;
    int32_t i32param49;
    int32_t i32param100;
    int32_t i32param102;
    int32_t i32param103;
    int32_t i32param104;
    int32_t i32param105;
    int32_t i32param106;
    int32_t i32param107;
    int32_t i32param108;
    int32_t i32param52;
    int32_t i32param53;
    int32_t i32param54;
    int32_t i32param97;
    int32_t i32param59;
    int32_t i32param55;
    int32_t i32param57;
    int32_t i32param58;
    int32_t i32param87;
    int32_t i32param88;
    int32_t i32param89;
    int32_t i32param90;
    int32_t i32param91;
    int32_t i32param92;
    int32_t i32param93;
    int32_t i32param101;
    int32_t i32param79;
    int32_t i32param80;
    int32_t i32param81;
    int32_t i32param99;
    int32_t i32param98;
    int32_t i32param82;
	int32_t i32param26;
	int32_t i32param27;
	int32_t i32param28;
	int32_t i32param29;
	int32_t i32param30;
	int32_t i32param31;
	int32_t i32param32;
	int32_t i32param23;
	int32_t i32param188;
	int32_t i32param189;
	int32_t i32param190;
	int32_t i32param191;
	int32_t i32param217;
	int32_t i32param218;
	int32_t i32param055;
	int32_t i32param056;
	int32_t i32param132;
	int32_t i32param133;
	int32_t i32param134;
	int32_t i32param135;
	int32_t i32param144;
	int32_t i32param145;
	int32_t i32param146;
	int32_t i32param25;    
    int32_t i32table_ram23[11];
    BRAP_DSPCHN_CVFreqDependentParams sFreqDependentParams[3];
    int32_t i32param115;
    int32_t i32param116;
    int32_t i32param117;
    int32_t i32table_coef33[11][2];
    int32_t i32table_coef34[11][3];
    int32_t i32table_coef35[11][2];
    int32_t i32table_coef36[11][3];
    int32_t i32table_coef37[2];
    int32_t i32table_coef38[3];    
} BRAP_DSPCHN_CustomVoiceConfigParams;


/***************************************************************************
Summary:
	This structure contains post processing algorithm PEQ configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_PeqConfigParams
{
    bool                            bBypassPeq;         /* Bypass PEQ post processing 
        								                    true = Bypass PEQ
        								                    false = Don't bypass PEQ */

    bool                            bPeqEnable[PEQ_FILTERS];      /* Individual enable fields for 
                                                            the 7 filters of Peq */

    bool                            bSonicEnable;       /* Enable field for subsonic filter */
                                                            
    unsigned int                    uiWindowStep;       /* Window step - Time for phase in/out
                                                            for filter change 
                                                            0 - no window
                                                            1 - 170.6 ms at 48 kHz sampling rate
                                                            2 - 85.3 ms at 48 kHz sampling rate
                                                            3 - 42.6 ms at 48 kHz sampling rate
                                                            4 - 21.3 ms at 48 kHz sampling rate
                                                            5 - 10.6 ms at 48 kHz sampling rate
                                                            6 - 5.3 ms at 48 kHz sampling rate
                                                            7 - 2.6 ms at 48 kHz sampling rate
                                                            */
    unsigned int                    uiFpk[PEQ_FILTERS]; /* Peak frequency for 7 PEQ filters 
                                                            20Hz = 20
                                                            20kHz = 20000
                                                            */
    int                             iGain[PEQ_FILTERS]; /* Gain for 7 PEQ filters 
                                                            Valid values are -120 to +120 in steps of 1
                                                            referring to -12dB to +12dB in steps of .1
                                                            */    
    unsigned int                    uiPeqQ[PEQ_FILTERS];/* Peak Frequency Q Gain for the 7 filters 
                                                            Scaled to a factor of 100 so .33 should be given as 33
                                                            */

    unsigned int                    uiSubsonicFc;       /* Subsonic filter cut off frequency 
                                                            Valid Values followed by their meaning in Hz.
                                                            0x0 - 40 Hz
                                                            0x1 - 45 Hz
                                                            0x2 - 50 Hz
                                                            0x3 - 56 Hz
                                                            0x4 - 63 Hz
                                                            0x5 - 70 Hz
                                                            0x6 - 80 Hz
                                                            0x7 - 90 Hz
                                                            0x8 - 100 Hz
                                                            0x9 - 110 Hz
                                                            0xA - 125 Hz
                                                            0xB - 140 Hz
                                                            0xC - 160 Hz
                                                            0xD - 180 Hz
                                                            0xE - 200 Hz
                                                            0xF - 220 Hz
                                                            0x10 - 250 Hz
                                                            0x11 - 280 Hz
                                                            0x12 - 315 Hz
                                                            others - reserved */                                                            

    
} BRAP_DSPCHN_PeqConfigParams;

typedef struct BRAP_DSPCHN_AdPanParams
{
    unsigned int            uiVol;              /* Volume */
    bool                    bEnableAd;          /* Enable/Disable Audio Descriptor on-the-fly.
                                                   If disabled, output of descriptor channel gets
                                                   muted and fade values are reset so that
                                                   no fading happens in primary channel. 
                                                   Mixed output of  primary and descriptor
                                                   channel then contains only primary audio. */
	unsigned int            uiAudioRampTimeInMs; /* Time in which the Primary and Secondary channel audio 
	                                                should be scaled as per the fade table values.
	                                                Default value is 1000. Range is 0 to 4Sec  */                                                            
} BRAP_DSPCHN_AdPanParams;

typedef enum BRAP_DSPCHN_DtsEnc_AcMode
{
    BRAP_DSPCHN_DtsEnc_AcMode_eMode10 = 1, /*C*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode20 = 2, /*L,R*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode30 = 3, /*L,C,R*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode21 = 4, /*L,R,S*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode31 = 5, /*L,C,R,S*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode22 = 6, /*L,R,LS,RS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode32 = 7, /*L,C,R,LS,RS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode33 = 8, /*L,C,R,LS,RS,CS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eMode32_BSDigital = 9,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eModeLtRt=0x20,/*L,R*/
    BRAP_DSPCHN_DtsEnc_AcMode_eModePLII_Movie = 0x21,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eModePLII_Music = 0x22,/*L,C,R,LS,RS*/
    BRAP_DSPCHN_DtsEnc_AcMode_eModePassiveMatrix = 0x23,/*L,R*/
    BRAP_DSPCHN_DtsEnc_AcMode_eModeCSII = 0x24,/*L,C,R,Ls,Rs,Cs*/
    BRAP_DSPCHN_DtsEnc_AcMode_eLAST,
    BRAP_DSPCHN_DtsEnc_AcMode_eINVALID=0x7fffffff

}BRAP_DSPCHN_DtsEnc_AcMode;

typedef struct BRAP_DSPCHN_DTSENCParams
{
    bool    bSpdifHeaderEnable; /* Enable/disable SPDIF Header */
    bool    bCertificationApp;  /*True if Standalone DTS Encoder Certification app*/
    bool	bLfeEnable;				/*Used only when Certification is set to 1. Enable =1 Disable =0*/
    BRAP_DSPCHN_DtsEnc_AcMode	eInputDataAcMode;		/*Used only when Certification is set to 1*/    
}BRAP_DSPCHN_DTSENCParams;

/***************************************************************************
Summary:
	Audio output channel pair indexes.

Description:
	This enumeration defines different output channel pairs corresponding to 
	various speaker systems.

See Also:

**************************************************************************/
typedef enum BRAP_DSPCHN_AC3ENC_DataRate
{
    BRAP_DSPCHN_AC3ENC_DataRate_Invalid = 0,
    BRAP_DSPCHN_AC3ENC_DataRate_12Kbps  = 8,
    BRAP_DSPCHN_AC3ENC_DataRate_384Kbps = 14,
    BRAP_DSPCHN_AC3ENC_DataRate_640Kbps = 18
} BRAP_DSPCHN_AC3ENC_DataRate;

typedef enum BRAP_DSPCHN_AudCodeMode
{
    BRAP_DSPCHN_AudCodeMode_eMono = 0,
    BRAP_DSPCHN_AudCodeMode_eLR   = 1,
    BRAP_DSPCHN_AudCodeMode_eLLfe = 2,
    BRAP_DSPCHN_AudCodeMode_eLR_LsRs_Lfe = 7
} BRAP_DSPCHN_AudCodeMode;

typedef struct BRAP_DSPCHN_Ac3ENCParams
{
    bool                            bTranscodeEnable;   
                                        /* True  - All parameters below are ignored. Normal Operation
                                           False - Used for certification. 
                                                   App to program values for all params below */    

    bool                            bSpdifHeaderEnable; 
                                        /* Enable/disable SPDIF Header */        

    /* Certiciation Specific Params */
    BRAP_DSPCHN_AudCodeMode         eAudCodMode;        /* Audio Coding Mode */    
    bool                            bLfe;               /* LFE Enabled or Not */
        /* Below Params are invalid for Dolby Transcode(MS10) */
    BRAP_OutputChannel              eInputCh;           /* Channel to be tested */
    BRAP_DSPCHN_AC3ENC_DataRate     eDataRate;          /* Dependent on AcMod */
    BAVC_AudioSamplingRate          eSampleRate;        /* Only 48kHz supported */
    bool                            bLfeFilter;         /* LFE Filter */
    bool                            bCmpChar;           /* AGC for normal channel. */
    bool                            bCmpChar2;          /* AGC for dual mono channel */
    bool                            bSurrDelay;         /* For certification testing */

}BRAP_DSPCHN_Ac3ENCParams;   

typedef struct BRAP_DSPCHN_SbcEncUserParam
{
    unsigned int    uiNumBlocks;    /* Default = 8; Possible Values: 4, 8, 12 and 16 */
    unsigned int    uiNumSubBands;  /* Default = 8; Possible Values: 4 and 8 */
    bool                bJointStereoEnable;/* Default = true; false -> JointStereo OFF, true -> JointStereo ON */
    unsigned int    uiBitAllocation;/* Default = 0; 0 -> Loudness, 1 -> SNR */
    unsigned int    uiBitPool;/* Default = 45; Range of Values: 2 to (16 * Number of Channels * Number of Subbands) */

}BRAP_DSPCHN_SbcEncUserParam;

typedef struct BRAP_DSPCHN_BtscUserParams
{
	bool bBTSCEnableFlag;					/*Default Enable*/
	bool bUseDeEmphasizedSourceSignals;		/*Default Enable*/
	bool bMainChannelPreEmphasisOn;			/*Default Enable*/
	bool bEquivalenMode75MicroSec;			/*Default Disable*/
	bool bClipDiffChannelData;				/*Default Enable*/
	bool bDiffChannelPreEmphasisOn;			/*Default Enable*/
	bool bUseLowerThresholdSpGain;			/*Default Enable*/
	unsigned int            uiVolumeLevel;                /*Volume in Q9.23Format. Range [0-0x2000000], Default:0x800000 , 0 means mute*/
    uint32_t ui32SRDCalibrationFactor;      /*ui32SRDCalibrationFactor.  Range: (0 - 8000), Default: 1171 */   

	bool bSumChanFreqDevCtrl;               /*Default Enable: Used to Control FM Deviation of Sum Channel to 25Khz*/
	bool bDiffChanFreqDevCtrl;              /*Default Enable: Used to Control FM Deviation of Diff Channel to 50Khz*/
	bool bOpFreqDevCtrl;                    /*Default Enable: Used to Control FM Deviation of Final Output to 50Khz*/

	bool bFreqGeneration;                   /*Default Disable: Used to Generate Test Tones*/
	uint32_t ui32Frequency;                 /*Default 0 generates frequency sweep.  Programming to a fixed value generates tone*/
	uint32_t ui32SumChannelLevelCtrl;       /*Sum Channel Level control: Range [0-0x2000000], Default:0x800000  */
	uint32_t ui32DiffChannelLevelCtrl;      /*Diff Channel Level control: Range [0-0x2000000], Default:0x800000  */ 
    bool bBTSCMonoModeOn;                   /*Default Disable: Used when we need to output mono*/   
}BRAP_DSPCHN_BtscUserParams;


typedef struct BRAP_DSPCHN_Brcm3DSurroundParams
{

    bool    b3DSurroundEnable;              /* True  - Enable 3D Surround
                                               False - Disable 3D Surround */    

    bool    bSoftLimiterEnable;             /* Soft Limiter Enable or Not */                
                
}BRAP_DSPCHN_Brcm3DSurroundParams;


typedef struct BRAP_DSPCHN_MonoDownmixParams
{

    bool    bMonoDownMixEnableFlag;         /* True  - Mono Downmix enabled
                                               False - Mono Downmix disabled */    
               
}BRAP_DSPCHN_MonoDownmixParams;

/***************************************************************************
Summary:
	This enumeration selects MLP profile to decode. MLP decoder requires more
	bandwidth to decode higher profiles.
***************************************************************************/
typedef enum BRAP_DSPCHN_MlpProfileSelect
{
	BRAP_DSPCHN_MlpProfileSelect_eTwoChannel = 1,
	BRAP_DSPCHN_MlpProfileSelect_eSixChannel = 2,
	BRAP_DSPCHN_MlpProfileSelect_eEightChannel = 4
} BRAP_DSPCHN_MlpProfileSelect;

/***************************************************************************
Summary:
	This enumeration define the MLP Dynamic Compression Range
***************************************************************************/
typedef enum BRAP_DSPCHN_MlpDrc
{
	BRAP_DSPCHN_MlpDrc_eDisable = 0,
	BRAP_DSPCHN_MlpDrc_eFollow = 1,
	BRAP_DSPCHN_MlpDrc_eEnabled = 2
} BRAP_DSPCHN_MlpDrc;

/***************************************************************************
Summary:
	This enumeration defines the MLP Dialog Normalization Enable modes
***************************************************************************/
typedef enum BRAP_DSPCHN_MlpDnEnable
{
	BRAP_DSPCHN_MlpDnEnable_eOff = 0x0,     /* Dialog Normalization OFF */
	BRAP_DSPCHN_MlpDnEnable_eOn = 0x1,      /* Dialog Normalization ON */
	BRAP_DSPCHN_MlpDnEnable_eDefault = 0xFF /* Default: Meaning whatever is 
	                                           present in the stream */
} BRAP_DSPCHN_MlpDnEnable;

/***************************************************************************
Summary:
	This structure contains MLP configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_MlpConfigParams
{
	BRAP_DSPCHN_MlpProfileSelect		eProfileSelect;	/* MLP profile to be supported */
	BRAP_DSPCHN_MlpDrc				eDrc;			/* Dynamic range compression
													    mode */
	uint32_t							ui32DrcBoost;	/* Dynamic range compression
													    boost factor. This is taken in
													    percentage.*/
	uint32_t							ui32DrcCut;		/* Dynamic range compression
													    cut factor. This is taken in
													    percentage.*/
	BRAP_DSPCHN_MlpDnEnable				eDialogueEnable; /* MLP dialogue enable */
	int32_t							i32DialogueLevel;	/* MLP diaglogue level.
														    Applicable only if
														    bDialogueEnable = true.
														    Permissible values are
														    0 to -63. */
} BRAP_DSPCHN_MlpConfigParams;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	WMA Pro stereo mode. 
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaProStereoMode
{
	BRAP_DSPCHN_WmaProStereoMode_eAuto,
	BRAP_DSPCHN_WmaProStereoMode_eLtRt,
	BRAP_DSPCHN_WmaProStereoMode_eLoRo,
	BRAP_DSPCHN_WmaProStereoMode_eInvalid
} BRAP_DSPCHN_WmaProStereoMode;


/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	WMA std stereo mode. 
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaStdStereoMode
{
	BRAP_DSPCHN_WmaStdStereoMode_eAuto,
	BRAP_DSPCHN_WmaStdStereoMode_eLtRt,
	BRAP_DSPCHN_WmaStdStereoMode_eLoRo,
	BRAP_DSPCHN_WmaStdStereoMode_eInvalid
} BRAP_DSPCHN_WmaStdStereoMode;

/***************************************************************************
Summary:
	This enumeration defines various decode modes.

***************************************************************************/
typedef enum BRAP_DSPCHN_WmaProDecodeMode
{
	BRAP_DSPCHN_WmaProDecodeMode_eDecode,		/* Decode mode */
	BRAP_DSPCHN_WmaProDecodeMode_ePassThru, 	/* Pass thru mode */
	BRAP_DSPCHN_WmaProDecodeMode_eSimulMode, 	/* Simul mode */	
   	BRAP_DSPCHN_WmaProDecodeMode_eInvalid	= 0xFF
} BRAP_DSPCHN_WmaProDecodeMode;

typedef enum BRAP_DSPCHN_ASFPTSType
{
     BRAP_DSPCHN_ASFPTSType_eInterpolated = 0,
     BRAP_DSPCHN_ASFPTSType_eCoded,
     BRAP_DSPCHN_ASFPTSType_eMax,
     BRAP_DSPCHN_ASFPTSType_eInvalid = 0x7FFFFFFF
}BRAP_DSPCHN_ASFPTSType;


typedef enum BRAP_DSPCHN_WmaPro_DrcSetting
{ 
    BRAP_DSPCHN_WmaPro_DrcSetting_eHigh=0,
    BRAP_DSPCHN_WmaPro_DrcSetting_eMed, 
    BRAP_DSPCHN_WmaPro_DrcSetting_eLow,
    BRAP_DSPCHN_WmaPro_DrcSetting_eLAST,
    BRAP_DSPCHN_WmaPro_DrcSetting_eINVALID=0x7fffffff

}BRAP_DSPCHN_WmaPro_DrcSetting; 

/***************************************************************************
Summary:
	This structure contains WMA Pro configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_WmaProConfigParams
{

	bool								bDrcFlag;	/* DRC On/Off */
    BRAP_DSPCHN_WmaPro_DrcSetting       eDrcSettings;
    unsigned int                                        uiRmsAmplitudeRef;
    unsigned int                                        uiPeakAmplitudeRef;    
    unsigned int                                        uiDesiredRms;    
    unsigned int                                        uiDesiredPeak;        
	BRAP_DSPCHN_WmaProStereoMode	eStereoMode; /* Stero Mode settings */
       BRAP_DSPCHN_WmaProDecodeMode  eDecodeMode;
        BRAP_DSPCHN_ASFPTSType   eAsfPtsType;/* This Configuration can not be set on the fly. This should be set before starting Decode*/
                                                                      /* Default = 0 (Use Interpolation always). 1 = Use Coded always. */       
} BRAP_DSPCHN_WmaProConfigParams;

typedef enum BRAP_DSPCHN_WMAIpType
{
     BRAP_DSPCHN_WMAIpType_eASF = 0,
     BRAP_DSPCHN_WMAIpType_eTS,
     BRAP_DSPCHN_WMAIpType_eMax,
     BRAP_DSPCHN_WMAIpType_eInvalid = 0x7FFFFFFF
}BRAP_DSPCHN_WMAIpType;

/***************************************************************************
Summary:
	This structure contains WMA Pro configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_WmaStdConfigParams
{
    BRAP_DSPCHN_WMAIpType   eWmaIpType;/* This Configuration can not be set on the fly. This should be set before starting Decode*/
                                                                      /* Default = 0 (Type ASF). Set to TS only when WMATS is enabled */ 
    BRAP_DSPCHN_ASFPTSType   eAsfPtsType;/* This Configuration can not be set on the fly. This should be set before starting Decode*/
                                                                      /* Default = 0 (Use Interpolation always). 1 = Use Coded always. */       
                                                                      
} BRAP_DSPCHN_WmaStdConfigParams;


/***************************************************************************
Summary:
	This structure contains MPEG-MC decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_MpegMcConfigParams
{
	bool		bDrcFlag;	/* DRC On/Off */
	uint32_t	ui32DrcBoost;/* Dynamic range compression
							    boost factor. This is taken in
							    percentage.*/
	uint32_t	ui32DrcCut;/* Dynamic range compression
						    cut factor. This is taken in
						    percentage.*/
	bool		bUseUserDownmixCoeff;/*	User defined downmix flag. This is set to true,
									when user wants to set the appropriate downmix
									coefficients for each channel */
	int32_t		i32UserDownmixCoeff[8][8];/* User downmix coefficients */
	bool		bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is ON */
} BRAP_DSPCHN_MpegMcConfigParams;

/***************************************************************************
Summary:
	This structure contains DTS-LBR decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DtslbrConfigParams
{
#if 1 
    bool                bDecodeEmbeddedStereo; /* Enable Decoder to decode 
                                                  embedded Stereo Stream. This 
                                                  is required for DTS-LBR 
                                                  decoder certification */
#else
	bool				bDynRngScaleEnable;	/* Enable dynamic range compression
								scaling.
								false = ui16DynRngScaleHi and ui16DynRngScaleLo
								are honored in DTS decoder
								true = ui16DynRngScaleHi and ui16DynRngScaleLo
								are ignored in DTS decoder */
								
	uint16_t			ui16DynRngScaleHi; /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
								
	uint16_t			ui16DynRngScaleLo; /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
								
	bool			    bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 

	bool		    	bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		        i32UserDownmixCoeff[6][6];
								/* TODO: Put proper comments */
#endif                                
} BRAP_DSPCHN_DtslbrConfigParams;


/***************************************************************************
Summary:
	This structure contains MPEG decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_MpegConfigParams
{
	uint32_t	ui32LeftChannelGain;		/* Left Channel gain */
	uint32_t	ui32RightChannelGain;	/* Right Channel gain */
}BRAP_DSPCHN_MpegConfigParams;

typedef enum  BRAP_DSPCHN_Mp3Enc_StereoControl
{
    BRAP_DSPCHN_Mp3Enc_StereoControl_eNoJointStereo,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eIntensityStereo,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eMsStereo,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eBothIntensityAndMsStereo,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eReserved1,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eReserved2,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eReserved3,
    BRAP_DSPCHN_Mp3Enc_StereoControl_eReserved4
}BRAP_DSPCHN_Mp3Enc_StereoControl;

typedef enum    BRAP_DSPCHN_Mp3Enc_InputMode
{
BRAP_DSPCHN_Mp3Enc_InputMode_eStereo,
BRAP_DSPCHN_Mp3Enc_InputMode_eReserved,
BRAP_DSPCHN_Mp3Enc_InputMode_eDualChannel,
BRAP_DSPCHN_Mp3Enc_InputMode_eMono
}BRAP_DSPCHN_Mp3Enc_InputMode;

typedef enum    BRAP_DSPCHN_Mp3Enc_BitRate
{
BRAP_DSPCHN_Mp3Enc_BitRate_0=0,
BRAP_DSPCHN_Mp3Enc_BitRate_32=32,
BRAP_DSPCHN_Mp3Enc_BitRate_40=40,
BRAP_DSPCHN_Mp3Enc_BitRate_48=48,
BRAP_DSPCHN_Mp3Enc_BitRate_56=56,
BRAP_DSPCHN_Mp3Enc_BitRate_64=64,
BRAP_DSPCHN_Mp3Enc_BitRate_80=80,
BRAP_DSPCHN_Mp3Enc_BitRate_96=96,
BRAP_DSPCHN_Mp3Enc_BitRate_112=112,
BRAP_DSPCHN_Mp3Enc_BitRate_128=128,
BRAP_DSPCHN_Mp3Enc_BitRate_160=160,
BRAP_DSPCHN_Mp3Enc_BitRate_192=192,
BRAP_DSPCHN_Mp3Enc_BitRate_224=224,
BRAP_DSPCHN_Mp3Enc_BitRate_256=256,
BRAP_DSPCHN_Mp3Enc_BitRate_320=230
}BRAP_DSPCHN_Mp3Enc_BitRate;

/***************************************************************************
Summary:
	This enumeration defines MPEG deemphasis mode from MPEG header
***************************************************************************/
typedef enum BRAP_DSPCHN_MpegEmphasisMode
{
	BRAP_DSPCHN_MpegEmphasisMode_None = 0,		/* No deemphasis */
	BRAP_DSPCHN_MpegEmphasisMode_Emph50_15ms,	/* Emph_50_15ms */
	BRAP_DSPCHN_MpegEmphasisMode_Reserved,		/* Reserved */
	BRAP_DSPCHN_MpegEmphasisMode_CCITT_J17,		/* CCITT_J17 */
	BRAP_DSPCHN_MpegEmphasisMode_Invalid		/* Invalid */
	
}BRAP_DSPCHN_MpegEmphasisMode;

typedef struct BRAP_DSPCHN_Mp3EncUserParam
{
    BRAP_DSPCHN_Mp3Enc_BitRate    eBitRate;/* Default Value = BRAP_DSPCHN_Mp3Enc_BitRate_128*/
    bool               bCrcProtect;/*Deault Value = false*/
    bool               bPrivateBit;/*Deault Value =false*/
    BRAP_DSPCHN_Mp3Enc_StereoControl    eStereoControl;/*Deault Value =BRAP_DSPCHN_Mp3Enc_StereoControl_eNoJointStereo*/
    bool                bCopyright;/*Deault Value = false*/
    bool                bOriginalBitSetting;/*Deault Value = false*/
    unsigned int    uiBitsPerSample;/*Deault Value = 16*/
    BAVC_AudioSamplingRate          eSampleRate;/*Deault Value = BAVC_AudioSamplingRate_e48*/
    BRAP_DSPCHN_MpegEmphasisMode	eEmphasisMode;		/*Deault Value = BRAP_DSPCHN_MpegEmphasisMode_None*/
    BRAP_DSPCHN_Mp3Enc_InputMode    eInputMode;/*Deault Value =BRAP_DSPCHN_Mp3Enc_InputMode_eStereo*/
    bool                bInterleavedPcm;    /*Deault Value = false*/
}BRAP_DSPCHN_Mp3EncUserParam;


typedef enum BRAP_DSPCHN_DraStereoMode
{
    BRAP_DSPCHN_DraStereoMode_eLoRo,
    BRAP_DSPCHN_DraStereoMode_eLtRt,
    BRAP_DSPCHN_DraStereoMode_eInvalid
} BRAP_DSPCHN_DraStereoMode;

typedef struct BRAP_DSPCHN_DraConfigParam
{
    BRAP_DSPCHN_DraStereoMode   eStereoMode;
}BRAP_DSPCHN_DraConfigParam;

/***************************************************************************
Summary:
	This enumeration contains DD Bass Management (DDBM) modes.
***************************************************************************/
typedef enum BRAP_DSPCHN_Ddbm_FilterCutOff
{
    BRAP_DSPCHN_Ddbm_FilterCutOff_0,
	BRAP_DSPCHN_Ddbm_FilterCutOff_80,
	BRAP_DSPCHN_Ddbm_FilterCutOff_100,
	BRAP_DSPCHN_Ddbm_FilterCutOff_120
} BRAP_DSPCHN_Ddbm_FilterCutOff;


/***************************************************************************
Summary:
	This enumeration contains DD Bass Management (DDBM) modes.
***************************************************************************/
typedef enum BRAP_DSPCHN_DdbmMode 
{
	BRAP_DSPCHN_DdbmMode_eConfig1,
	BRAP_DSPCHN_DdbmMode_eConfig2,
	BRAP_DSPCHN_DdbmMode_eConfig2_2,
	BRAP_DSPCHN_DdbmMode_eMax
} BRAP_DSPCHN_DdbmMode;

/***************************************************************************
Summary:
	This enumeration defines dual mono mode values.
***************************************************************************/
typedef enum BRAP_DSPCHN_DualMonoMode{
	BRAP_DSPCHN_DualMonoMode_eLeftMono,
	BRAP_DSPCHN_DualMonoMode_eRightMono,
	BRAP_DSPCHN_DualMonoMode_eStereo,
	BRAP_DSPCHN_DualMonoMode_eDualMonoMix
} BRAP_DSPCHN_DualMonoMode;

/***************************************************************************
Summary:
	This enumeration defines cinema modes.
***************************************************************************/
typedef enum BRAP_DSPCHN_CinemaMode{
    BRAP_DSPCHN_CinemaMode_eCinema = 1,
	BRAP_DSPCHN_CinemaMode_eMusic
} BRAP_DSPCHN_CinemaMode;

/***************************************************************************
Summary:
	This enumeration defines fidelity sub-bands.
***************************************************************************/
typedef enum BRAP_DSPCHN_FidelitySubBands{
    BRAP_DSPCHN_FidelitySubBands_e32,
    BRAP_DSPCHN_FidelitySubBands_e64
} BRAP_DSPCHN_FidelitySubBands;


/***************************************************************************
Summary:
	This enumeration defines different playback rates for the DSP
***************************************************************************/
typedef enum BRAP_DSPCHN_PlayBackRate{
    BRAP_DSPCHN_PlayBackRate_e80, /* Play out the decoded data at 80% of the 
                                  original rate ie slower */
    BRAP_DSPCHN_PlayBackRate_e100, /* Play out the decoded data at 100% of the 
                                  original rate ie at the same rate */
    BRAP_DSPCHN_PlayBackRate_e120 /* Play out the decoded data at 120% of the 
                                  original rate ie 20% faster*/            
} BRAP_DSPCHN_PlayBackRate;

/***************************************************************************
Summary:
	This structure contains audio parameters required to decode
	input compressed audio data.

***************************************************************************/
typedef struct BRAP_DSPCHN_AudioParams
{
	BRAP_DSPCHN_DecodeMode	eDecodeMode;	/* Decode Mode */
	BRAP_DSPCHN_AudioType	eType;          /* Audio Type */
	BAVC_StreamType		eStreamType;	/* DSP needs to know the input
						   stream type for TSM purpose */
        unsigned int  uiPBRate;       /* Rate at which the decoded audio 
                                                should be played out.
                                                Range: BRAP_DSPCHN_PLAYBACKRATE_NORMAL/2 to BRAP_DSPCHN_PLAYBACKRATE_NORMAL*2. i.e.0.5x to 2x
                                                Default BRAP_DSPCHN_PLAYBACKRATE_NORMAL. Currently Granularity of this is 1*/                       

	BRAP_DSPCHN_DvdType		eDvdType;	/* This field is used only if
												audio is DVD material. */

    bool                    bDtsCdCompressed;
                                    /* This flag is used when AudioType is 
                                       DTS-HD or DTS and DecodeMode is simul. 
                                       True = DTS-CD compressed data 
                                       False = DTS-HD compressed data */
    
	bool                    bHbrCompressed;			
                                    /*False = For legacy SPDIF Compressed mode 
									  True = HBR Compressed mode
									  Note:Currently this flag is used 
									  for DTSHD/MLP/DDP 7.1 in simul mode only 
									  False = For legacy SPDIF mode where only
									  core would be sent.
									  True = The HBR mode where both the core
									  and extension had to be routed.*/

	int32_t		i32AVOffset;	/* Audio-Video delay path difference*/
    BAVC_AudioSamplingRate	eInputSampleRate; /* Sample rate for External I2S input */ 
	BAVC_AudioSamplingRate	eOutputSampleRate;	/* Output sample rate. Supported
												     only for 7411D0 in 8-ch, overclocked
												     I2S audio mode. Firmware converts
												     sample rate of the input stream to
												     the value programmed in this field.
												     Supported output sample rates are
												     48 KHz, 96 KHz and 192 KHz. Sample
												     rate programmed in this field should be
												     2x, 4x or 8x of stream sample rate for
												     upsampling case. For downsampling
												     case, it should be x/2 of stream sample
												     rate. To disable SRC, program this field
												     to BAVC_AudioSamplingRate_eUnknown */
	bool			bPtsSpecificDecode;	/* This field is valid only for playback 
									    case and ignored in live decode.
									    
									    true = Enable PTS specific decode.
									    Start and End of decode is controlled by
									    i32StartPts and i32EndPts fields. When
									    decoder meets PTS in stream whose value
									    is equal to i32StartPts, it starts rendering
									    out audio. When decoder comes across
									    PTS in stream whose value is equal to 
									    i32EndPts, it stops rendering out audio.
									    
									    false = Disable PTS specific decode.
									    */
	int32_t		i32StartPts;	/* Value of Start PTS for PTS specific decode
							    mode (bPtsSpecificDecode = true). 
							    This field is ignored if mode is not
							    PTS specific decode mode. */												   
	int32_t		i32EndPts;	/* Value of End PTS for PTS specific decode
							    mode (bPtsSpecificDecode = true). 
							    This field is ignored if mode is not
							    PTS specific decode mode. */	
	bool			bEnableStereoDownmixPath; /* Enable concurrent stereo downmix path.
											   Supported only for 7411D0 in 8-ch,
											   overclocked I2S audio mode. The stereo
											   downmix channels are sent in slot 7 and 8
											   of I2S1 */
    bool        bPPMCorrection; /* PPM Correction enable flag for enabling 2ms lipsync */
    BRAP_DSPCHN_LowDelayEnableMode	eLowDelayEnableMode;
                                        /* To enable or disable the low delay mode */
    bool        bEnableTargetSync;
                                        /*Applicable only for Certification app. In order to pass the last frame for certification, 
                                        a user config flag is required to be sent to datasync. This flag will be used to disable the
                                        target frame syncing for datasync only for certification purposes. This flag is to be set to 
                                        enable otherwise. The default value for this flag is Enable. When this flag is set to disabled, 
                                        the data sync will skip the target sync validation.  */                                        

} BRAP_DSPCHN_AudioParams;

/***************************************************************************
Summary:
	This enumeration defines the PTS types associated with PTS value
	returned by FirstPTSReady and PTSError interrupts as well as by
	BRAP_DSPCHN_GetCurrentPTS API.
***************************************************************************/
typedef enum BRAP_DSPCHN_PtsType
{
	BRAP_DSPCHN_PtsType_eCoded,         
	BRAP_DSPCHN_PtsType_eInterpolatedFromValidPTS,
	BRAP_DSPCHN_PtsType_eInterpolatedFromInvalidPTS = 3
} BRAP_DSPCHN_PtsType ;

/***************************************************************************
Summary:
***************************************************************************/
typedef struct BRAP_DSPCHN_PtsInfo
{
    uint32_t  		ui32RunningPts;   /* The PTS value of the frame going 
    					 out the next vsync 
    					 MPEG/DirecTv HD - 45KHz domain
    					 DirecTv SD- 27MHz domain */
    int32_t           i32Pts2StcPhase;    /* Difference between current PTS 
    						and the STC */
    int32_t         i32TSMUpperThreshold;      /*TSM Upper threshold*/                                       
    BRAP_DSPCHN_PtsType 	ePtsType;   /* The PTS type tag */ 
    uint32_t ui32NumContinuousFail; /* Fail count from TSM */    
} BRAP_DSPCHN_PtsInfo;

/***************************************************************************
Summary: Provides the custom voice info
***************************************************************************/
typedef struct BRAP_DSPCHN_CustomVoiceInfo
{
    int             i32PVCMonitorLevel;       /* The PVC Monitor Level */
                        
}BRAP_DSPCHN_CustomVoiceInfo;

/***************************************************************************
Summary: Provides the audyssey volume info
***************************************************************************/
typedef struct BRAP_DSPCHN_AudysseyVolumeInfo
{
    int         i32chCalcLevel[8];		/* chCalcLevel in dB in floating format */


    int		    i32Vol_Applied_dB;		/* DynamicEQ  Vol_Applied_dB in dB*/

    int         i32SwDynEQ;			    /* Dynamic EQ On/Off/Ramp off  3 modes 0, 1 and 2 */
										/* range [0,1,2]*/
										/* 0 -> off, 1-> on, 2 -> Ramp off */

    unsigned int ui32NumChannels;		/*	No of channels */
    unsigned int ui32AudysseyVolBypass;	/*	range [0,1]	*/
										/*	0 -> Processing enable, 1-> total disable of the processing*/

    int         i32ChannelMask;		    /* Specifying the channels where gain is to be applied*/
										/* Meaning of bits

											0 -> L, 1 -> R, 2 -> C, 3 -> LFE
											4 -> Ls, 5-> Rs, 6 -> Lb, 7 -> Rb
										*/

    int         i32AudysseyVolInit;     /* range [0,1]*/
										/* 0 -> Do nothing, 1-> Reset(re-initialize) ADV*/
            
    int         i32HeadroomOffset;		/* headroom_offset in dB */
										/* range -256 to 256*/

    int         i32SwAudysseyVol;		/* ADV On/Off */
										/* range [0,1]*/
										/* 0 -> off, 1-> on*/

    int         i32SwDynSurrGain;		/* Switch for surround channel */
										/* range [0,1]*/
										/* 0 -> off, 1-> on*/
    int         i32SwHiLoCmpress;		/* Switch for having boost in calc cntr gain    */
										/* range [0,1]*/
										/* 0 -> off, 1-> on */

    int         i32dBVolSetting;		/* for gain function and Dynamic EQ */
										/* float range [0, -50] */
    int         i32GCF;				    /* Gain Compression Factor */
										/* float range [1, 2.4] */

    int         i32chCalbGain[8];		/* chCAlbGain in dB in floating format. */

}BRAP_DSPCHN_AudysseyVolumeInfo;


/***************************************************************************
Summary:
	This enumeration defines MPEG audio layers.

***************************************************************************/
typedef enum BRAP_DSPCHN_MpegAudioLayer
{
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer3 = 1,		/* MPEG Layer III */
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer2,    	/* MPEG Layer II */	
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer1,	    	/* MPEG Layer I */
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayerInvalid	    	/* Invalid MPEG Layer  */	
}BRAP_DSPCHN_MpegAudioLayer;

/***************************************************************************
Summary:
	This enumeration defines MPEG input channel mode from MPEG header
***************************************************************************/
typedef enum BRAP_DSPCHN_MpegChannelMode
{
	BRAP_DSPCHN_MpegChannelMode_eStereo = 0,		/* Stereo */
	BRAP_DSPCHN_MpegChannelMode_eIntensityStereo,		/* Intensity_stereo for layer I and II */
	BRAP_DSPCHN_MpegChannelMode_eDualChannel,		/* Dual Channel */
	BRAP_DSPCHN_MpegChannelMode_eSingleChannel,		/* Single Channel */
	BRAP_DSPCHN_MpegChannelMode_eInvalid		/* Invalid*/
	
}BRAP_DSPCHN_MpegChannelMode;




/***************************************************************************
Summary:
	This structure contains MPEG Bit Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_MpegInfo
{
	bool					bOriginal;	/* true = Original, false = copy */
	bool					bCopyRight;	/* true = copyright present,
								   false = copyright absent */
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
	uint32_t				ui32SamplingRateIndex; 	/* The actual value coded in the stream */								   
	uint32_t				ui32BitRate;  	/* MPEG bit rate in kbit/s
												If bit rate is free format
												then MPEG bit rate = 0 kbit/s */
	uint32_t				ui32BitRateIndex;	/* The actual value coded in the stream */
	bool					bCRCPresent;  	/* true = present,
							           false = absent */
	BRAP_DSPCHN_MpegChannelMode  eChmod;  			/* channel mode */
	BRAP_DSPCHN_MpegAudioLayer   eLayer;  			/* mpegaudio layer */
	BRAP_DSPCHN_MpegEmphasisMode	eEmphasisMode;	/* Deemphasis mode */	
}BRAP_DSPCHN_MpegInfo;


/***************************************************************************
Summary:
	This enumeration defines Mpeg Multi Channel Surround Mode
**************************************************************************/

typedef enum BRAP_DSPCHN_MpegMcSurroundMode
{
	BRAP_DSPCHN_MpegMcSurroundMode_eNoSurround,
	BRAP_DSPCHN_MpegMcSurroundMode_eMonoSurround,
	BRAP_DSPCHN_MpegMcSurroundMode_eStereoSurround,
	BRAP_DSPCHN_MpegMcSurroundMode_eSecondStereo
}BRAP_DSPCHN_MpegMcSurroundMode;


/***************************************************************************
Summary:
	This enumeration defines Mpeg Multi Channel Dematrix Procedure
**************************************************************************/

typedef enum BRAP_DSPCHN_MpegMcDematrixProcedure
{
	BRAP_DSPCHN_MpegMcDematrixProcedure_eProcedure0,
	BRAP_DSPCHN_MpegMcDematrixProcedure_eProcedure1,
	BRAP_DSPCHN_MpegMcDematrixProcedure_eProcedure2,
	BRAP_DSPCHN_MpegMcDematrixProcedure_eProcedure3
}BRAP_DSPCHN_MpegMcDematrixProcedure;

/***************************************************************************
Summary:
	This structure contains MPEG-MC Bit Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_MpegMcInfo
{
	BRAP_DSPCHN_MpegMcSurroundMode			eSurroundMode;	/*It Indicates whether
											surround channels are contained in the multiplex */
												
	BRAP_DSPCHN_MpegMcDematrixProcedure		eDematrixProcedure; /* It Indicates which dematrix 
											procedure has to be applied in the decoder*/
					
	uint32_t				ui32NumMultiLingualChn; /*This defines no. of multilingual or 
													commentary channels in the bit stream */
	
	bool					bMcPrediction;		/*This defines whether Mc prediction is On or Off */
	
	bool					bOutputLfePresent;	/* To indicate whether a LFE channel is present */

	bool					bOriginal;	/* true = Original, false = copy */
	
	bool					bCopyRight;	/* true = copyright present,
								   false = copyright absent */
								   	
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
								   
	uint32_t				ui32SamplingRateIndex; 	/* The actual value coded in the stream */								   
	
	uint32_t				ui32BitRate;  	/* MPEG bit rate in kbit/s
												If bit rate is free format
												then MPEG bit rate = 0 kbit/s */
												
	uint32_t				ui32BitRateIndex;	/* The actual value coded in the stream */

	uint32_t				ui32CrcCount;		/* This defines CRC error counter value */
	
	bool					bCRCPresent;  	/* true = present,
							           false = absent */
									   	
	bool  					bAlgorithmId;	/* 0 = RESERVED
											1 = ISO11172_Audio	*/
												
	BRAP_DSPCHN_MpegChannelMode  eChmod;  			/* channel mode */
											
	BRAP_DSPCHN_MpegAudioLayer   eLayer;  			/* mpegaudio layer */
	
	BRAP_DSPCHN_MpegEmphasisMode	eEmphasisMode;	/* Deemphasis mode */	
	
}BRAP_DSPCHN_MpegMcInfo;




/***************************************************************************
Summary:
	This enumeration defines AC3 BSMOD values
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3Bsmod
{
	BRAP_DSPCHN_Ac3Bsmod_eMainAudCM, /* Main Audio service, complete main */
	BRAP_DSPCHN_Ac3Bsmod_eMainAudME, /* Main Audio service, music and effects */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudVI, /* Associated service, visually impaired */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudHI, /* Associated service, hearing impaired */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudD,  /* Associated service, dialogue */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudC,  /* Associated service, commentary */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudE,  /* Associated service, emergency */
	BRAP_DSPCHN_Ac3Bsmod_eAssoAudAcmodStreamId /* Associated service, stream
						 identification with ACMOD value */
} BRAP_DSPCHN_Ac3Bsmod;

/***************************************************************************
Summary:
	This enumeration defines AC3 ACMOD values
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3Acmod
{
	BRAP_DSPCHN_Ac3Acmod_eTwoMono_1_ch1_ch2,
	BRAP_DSPCHN_Ac3Acmod_eOneCentre_1_0_C,
	BRAP_DSPCHN_Ac3Acmod_eTwoCh_2_0_L_R,
	BRAP_DSPCHN_Ac3Acmod_eThreeCh_3_0_L_C_R,
	BRAP_DSPCHN_Ac3Acmod_eThreeCh_2_1_L_R_S,
	BRAP_DSPCHN_Ac3Acmod_eFourCh_3_1_L_C_R_S,
	BRAP_DSPCHN_Ac3Acmod_eFourCh_2_2_L_R_SL_SR,
	BRAP_DSPCHN_Ac3Acmod_eFiveCh_3_2_L_C_R_SL_SR
} BRAP_DSPCHN_Ac3Acmod;

/***************************************************************************
Summary:
	This enumeration defines AC3 Dolby Surround mode 
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3DSur
{
	BRAP_DSPCHN_Ac3DSur_eNotIndicated,	/* Surround mode not indicated */
	BRAP_DSPCHN_Ac3DSur_eNotEncoded,	/* Dolby surround not encoded */
	BRAP_DSPCHN_Ac3DSur_eEncoded	/* Dolby surround encoded */
} BRAP_DSPCHN_Ac3DSur;

/***************************************************************************
Summary:
	This enumeration defines AC3 type of room in which audio was mixed 
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3RTValue
{
	BRAP_DSPCHN_Ac3RTValue_eNotIndicated,	/* Room type not indicated */
	BRAP_DSPCHN_Ac3RTValue_eLargeRoom,		/* Large room type */
	BRAP_DSPCHN_Ac3RTValue_eSmallRoom		/* Small room type */
} BRAP_DSPCHN_Ac3RTValue;

/***************************************************************************
Summary:
	This enumeration defines downmix level of the centre channel 
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3CMix
{
	BRAP_DSPCHN_Ac3CMix_e3dbBelow,			/* 3 db below */
	BRAP_DSPCHN_Ac3CMix_e4nHalfdbBelow,		/* 4.5 db below */
	BRAP_DSPCHN_Ac3CMix_e6dbBelow			/* 6 db below */
} BRAP_DSPCHN_Ac3CMix;

/***************************************************************************
Summary:
	This enumeration defines downmix level of the surround channels
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3SMix
{
	BRAP_DSPCHN_Ac3SMix_e3dbBelow,			/* 3 db below */
	BRAP_DSPCHN_Ac3SMix_e6dbBelow,			/* 6 db below */
	BRAP_DSPCHN_Ac3SMix_e0dbBelow			/* 0 db below */
} BRAP_DSPCHN_Ac3SMix;

/***************************************************************************
Summary:
	This enumeration defines AC3 time code presence in stream.

***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3TCode
{
	BRAP_DSPCHN_Ac3TCode_eNotPresent,		/* Time code not present */
	BRAP_DSPCHN_Ac3TCode_eFirstHalfPresent,	/* Only first half time code present */
	BRAP_DSPCHN_Ac3TCode_eSecondHalfPresent,	/* Only second half time code present */
	BRAP_DSPCHN_Ac3TCode_eBothHalvesPresent	/* Both halves time code present */
} BRAP_DSPCHN_Ac3TCode;

/***************************************************************************
Summary:
	This structure contains AC3 time code value

***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3TCodeVal
{
	uint32_t	ui32hours;			/* Time in hours */
	uint32_t	ui32minutes;		/* Time in minutes */
	uint32_t	ui32seconds;		/* Time in seconds */
} BRAP_DSPCHN_Ac3TCodeVal;
/**************************************************************************
Summary:
Dependent Frame Channel map mode for Dolby

Lsd/Rsd: Left/Right Surround Direct
Lc/Rc: Left/Right center
L/R/Cvh: Left/Right/Center vertical height
Lw/Rw: Left/Right wide
Ts: Top Center surround
Lrs/Rrs: Left/Right rear surround

**************************************************************************/

typedef enum BRAP_DSPCHN_AC3DependentFrameChannelMapMode
{
    BRAP_DSPCHN_DependentFrameChannelMapMode_eReserved=0,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eC=1,        
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R=2,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R=3,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l=4,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l=5,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r=6,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r=7,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_Cvh=8,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Ts=9,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Ts=10,                
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Cvh=11,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_Lc_Rc=12,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Lw_Rw=13,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Lvh_Rvh=14,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Lsd_rsd=15,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Lrs_Rrs=16,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Lc_Rc=17,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Lw_Rw=18,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Lvh_Rvh=19,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Lsg_Rsd=20,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Lrs_Rrs=21,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Ts_Cvh=22,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Cs=23,            
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Cs=24,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_R_l_r_Cs_Ts=25,           
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Cs_Cvh=26,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eL_C_R_l_r_Cs_Ts=27,
    BRAP_DSPCHN_DependentFrameChannelMapMode_eMax    
}BRAP_DSPCHN_AC3DependentFrameChannelMapMode;

/***************************************************************************
Summary:
	This structure contains AC3 Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3Info
{
	BAVC_AudioSamplingRate	eSamplingRate;	/* Sampling Freq of AC3 stream */
	uint32_t					ui32SamplingRateIndex; 	/* The actual  fscod value coded in the stream */

	uint8_t					ui8BitStreamId; /* bitStreamId indicates the 
											   version of the Dolby Digital 
											   standard that the bit stream is 
											   compliant with. It is the Dolby 
											   Digital bit stream field. */

	BRAP_DSPCHN_Ac3Bsmod	eBsmod;			/* bsmod indicates the type of 
											   service the bit stream conveys 
											   as defined in the Dolby Digital 
											   specification. */

	BRAP_DSPCHN_Ac3Acmod	eAcmod;			/* Audio coding mode which describes 
											   the number of input channels */ 

	BRAP_DSPCHN_Ac3DSur	eDSur;				/* DSUR indicates whether the current 
											   Dolby Digital bit stream is coded 
											   in Dolby Surround mode only when 
											   ACMOD is in 2/0 mode. */

	bool					bLfeOn;			/* LFEON indicates a low-frequency 
											   channel effect exists when it is 
											   1. Otherwise, the low frequency 
											   channel	does not exist. */ 
	
	bool					bCh1LCodeExist;	/* Language code exist or not */

	uint8_t					ui8Ch1LCode;	/* lCode indicates the language in 
											   use by the dialogue portion of 
											   the audio. The language codes are 
											   defined in the Dolby algorithm 
											   description. The value for English 
											   is 0x09. The value for Spanish 
											   is 0x0A. */

	bool					bCh2LCodeExist;	/* Language code exist or not */

	uint8_t					ui8Ch2LCode;	/* lCode indicates the language 
											   in use by the dialogue portion 
											   of the audio. The language codes 
											   are defined in the Dolby 
											   algorithm description. The 
											   value for English is 0x09. 
											   The value for Spanish is 0x0A. */

	BRAP_DSPCHN_Ac3RTValue	eRoomType;		/* roomType indicates the type of 
											   room in which the audio was mixed. */

	BRAP_DSPCHN_Ac3RTValue	eRoomType2;		/* roomType indicates the type of 
											   room in which the audio was 
											   mixed for channel 2. */

	bool					bCpyrhtPresent; /* CYPRHT is a copyright notice. 
											   When it is 1, the audio 
											   information in the bit stream 
											   is protected by copyright. 
											   Otherwise, it is not */


	bool					bOrigBitStream; /* If ORIGBS is 1, the current bit 
											   stream is the original Dolby 
											   Digital bit stream. Otherwise, 
											   it is a copy of a bit stream. */

	BRAP_DSPCHN_Ac3CMix		eCentreMix;		/* centreMix indicates the down 
											   mix level of the center channel. */

	BRAP_DSPCHN_Ac3SMix		eSurMix;		/* surMix indicates the down mix 
											   level of the surround channels. */

	BRAP_DSPCHN_Ac3TCode	eTCode;

	BRAP_DSPCHN_Ac3TCodeVal	sFirstHalfTCode;/* time code for low resolution 
											   in 8 seconds increments up to 
											   24 hours */

	BRAP_DSPCHN_Ac3TCodeVal	sSecondHalfTCode;/* time code for low resolution 
												in 8 seconds increments up to 
												24 hours */
	uint32_t					ui32FrameSizeCode; 	/*Used in conjunction with the sample rate
												code to determine the number of 16-bit
												words before the next syncword */

        BRAP_DSPCHN_AC3DependentFrameChannelMapMode    eDependentFrameChannelMapMode;
}BRAP_DSPCHN_Ac3Info;

/***************************************************************************
Summary:
	This structure contains AC3 Plus (DDP) Stream Information. Since AC3 Plus
	is an enhanced algorithm over AC3, all AC3 stream information parameters
	apply for AC3 Plus. AC3 Plus stream information is a superstructure over 
	AC3 stream information structure.
***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3PlusInfo
{
	BRAP_DSPCHN_Ac3Info		sAc3Info;	/* Parameters that are common
										   between AC3 and AC3 plus */

} BRAP_DSPCHN_Ac3PlusInfo;

/***************************************************************************
Summary:
	This enumeration defines AAC profile
***************************************************************************/
typedef enum BRAP_DSPCHN_AacProfile
{
	BRAP_DSPCHN_AacProfile_eMainProfile,
	BRAP_DSPCHN_AacProfile_eLowComplexityProfile,
	BRAP_DSPCHN_AacProfile_eScalableSamplingRateProfile
} BRAP_DSPCHN_AacProfile;

/***************************************************************************
Summary:
	This enumeration defines ACMOD values for AAC. Audio coding mode 
	describes the number of input channels.
***************************************************************************/
typedef enum BRAP_DSPCHN_AacAcmod 
{
	BRAP_DSPCHN_AacAcmod_eTwoMono_1_ch1_ch2,
	BRAP_DSPCHN_AacAcmod_eOneCentre_1_0_C,
	BRAP_DSPCHN_AacAcmod_eTwoCh_2_0_L_R,
	BRAP_DSPCHN_AacAcmod_eThreeCh_3_0_L_C_R,
	BRAP_DSPCHN_AacAcmod_eThreeCh_2_1_L_R_S,
	BRAP_DSPCHN_AacAcmod_eFourCh_3_1_L_C_R_S,
	BRAP_DSPCHN_AacAcmod_eFourCh_2_2_L_R_SL_SR,
	BRAP_DSPCHN_AacAcmod__eFiveCh_3_2_L_C_R_SL_SR
} BRAP_DSPCHN_AacAcmod;

/***************************************************************************
Summary:
	This structure contains AAC Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_AacInfo
{
	uint32_t				ui32BitRate;		/* AAC stream bit rate */
	BRAP_DSPCHN_AacProfile	eProfile;			/* AAC profile */
	BAVC_AudioSamplingRate	eSamplingRate;		/* AAC stream sampling rate */
	uint32_t				ui32LfeChannels;	/* Number of LFE channel 
												   elements */
	uint32_t				ui32BackChannels;	/* The number of audio syntactic 
												   elements for the back channels */
	uint32_t				ui32SideChannels;	/* The number of audio syntactic 
												   elements in the side */
	uint32_t				ui32FrontChannels;	/* The number of audio syntactic 
												   elements in the front channels, 
												   front center to back center, 
												   symmetrically by left and right, 
												   or alternating by left and 
												   right in the case of single 
												   channel element */
	bool					bLfeOn;				/* Low frequency enhancement
												   channel On/Off */
	BRAP_DSPCHN_AacAcmod	eAcmod;				/* Audio coding mode which describes
												   the number of input channels */
	bool					bPseudoSurroundEnable; /* Pseudo surround decoding is
												   enabled or not */
	unsigned int			uiMatrixMixdownIdx;	/* A two bit value that indicates the 
												   coefficient to be used in 5 to 2 
												   matrix mixdown */
	bool					bMatrixMixdownPresent; /* Stereo matrix coefficient is present 
												   or not */
	bool					bDRCPresent;		/* whether DRC is present in 
												   the stream */
        unsigned int                uiNumPulseId;
                                                   /* Total number of Pulse ID found in the bitstream till the 
                                                   current frame */
} BRAP_DSPCHN_AacInfo;

/***************************************************************************
Summary:
	This structure contains AAC_SBR (AAC-HE) Stream Information. Since AAC-HE
	is an enhanced algorithm over AAC, all AAC stream information parameters
	apply for AACHE. AAC-HE stream information is a superstructure over AAC
	stream information structure.
***************************************************************************/
typedef struct BRAP_DSPCHN_AacSbrInfo
{
	BRAP_DSPCHN_AacInfo	sAacInfo;		/* Stream Information Parameters common
											between AACHE and AAC */
} BRAP_DSPCHN_AacSbrInfo;


typedef struct BRAP_DSPCHN_DolbyPulseInfo
{
	BRAP_DSPCHN_AacInfo	sAacInfo;		/* Stream Information Parameters common
											between AACHE and AAC */
} BRAP_DSPCHN_DolbyPulseInfo;

/***************************************************************************
Summary:
	This enumeration defines AMODE values for DTS. Audiomode 
	describes the number of audio channels and audio playback arrangement.
    This is also used for DTS-HD.
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsAmode 
{
	BRAP_DSPCHN_DtsAmode_eOneCh_A,
	BRAP_DSPCHN_DtsAmode_eTwoCh_A_B,
	BRAP_DSPCHN_DtsAmode_eTwoCh_L_R,
	BRAP_DSPCHN_DtsAmode_eTwoCh_LpR_LmR,
	BRAP_DSPCHN_DtsAmode_eTwoCh_LT_RT,
	BRAP_DSPCHN_DtsAmode_eThreeCh_C_L_R,
	BRAP_DSPCHN_DtsAmode_eThreeCh_L_R_S,
	BRAP_DSPCHN_DtsAmode_eFourCh_C_L_R_S,
	BRAP_DSPCHN_DtsAmode_eFourCh_L_R_SL_SR,
	BRAP_DSPCHN_DtsAmode_eFiveCh_C_L_R_SL_SR,
	BRAP_DSPCHN_DtsAmode_eSixCh_CL_CR_L_R_SL_SR,
	BRAP_DSPCHN_DtsAmode_eSixCh_C_L_R_LR_RR_OV,
	BRAP_DSPCHN_DtsAmode_eSixCh_CF_CR_LF_RF_Lr_Rr,
	BRAP_DSPCHN_DtsAmode_eSevenCh_CL_C_CR_L_R_SL_SR,
	BRAP_DSPCHN_DtsAmode_eEightCh_CL_CR_L_R_SL1_SL2_SR1_SR2,
	BRAP_DSPCHN_DtsAmode_eEightCh_CL_C_CR_L_R_SL_S_SR,	
	BRAP_DSPCHN_DtsAmode_UserDefined
} BRAP_DSPCHN_DtsAmode;

/***************************************************************************
Summary:
	This field indicates the copy history of the audio. 
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsCopyHistory 
{
	BRAP_DSPCHN_DtsCopyHistory_Copy_prohibited,
	BRAP_DSPCHN_DtsCopyHistory_First_generation,
	BRAP_DSPCHN_DtsCopyHistory_Second_generation,
	BRAP_DSPCHN_DtsCopyHistory_Original_material
} BRAP_DSPCHN_DtsCopyHistory;

/***************************************************************************
Summary:
	This field indicates the quantization resolution of source PCM samples.
	The left and right surround channels of the source material are mastered
	in DTS ES format if ES = 1.
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsSourcePcm 
{
	BRAP_DSPCHN_DtsSourcePcm_ES0_16_bits,
	BRAP_DSPCHN_DtsSourcePcm_ES1_16_bits,
	BRAP_DSPCHN_DtsSourcePcm_ES0_20_bits,
	BRAP_DSPCHN_DtsSourcePcm_ES1_20_bits,
	BRAP_DSPCHN_DtsSourcePcm_ES0_24_bits,
	BRAP_DSPCHN_DtsSourcePcm_ES1_24_bits
} BRAP_DSPCHN_DtsSourcePcm;

/***************************************************************************
Summary:
	Indicates if the LFE channel is present and the choice of the interpolation factor.
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsLfeFlag 
{
	BRAP_DSPCHN_DtsLfeFlag_Not_present,
	BRAP_DSPCHN_DtsLfeFlag_Present_128,
	BRAP_DSPCHN_DtsLfeFlag_Present_64
} BRAP_DSPCHN_DtsLfeFlag;

/***************************************************************************
Summary:
	This flag has meaning only if bExtAudio = 1 and then it indicates the type of
	data that has been placed in the extension stream(s).
***************************************************************************/
typedef enum BRAP_DSPCHN_DtsExtAudId
{
	BRAP_DSPCHN_DtsExtAudId_XCh,
	BRAP_DSPCHN_DtsExtAudId_XXCh,
	BRAP_DSPCHN_DtsExtAudId_X96k,
	BRAP_DSPCHN_DtsExtAudId_Reserved3,
	BRAP_DSPCHN_DtsExtAudId_Reserved4,
	BRAP_DSPCHN_DtsExtAudId_Reserved5,
	BRAP_DSPCHN_DtsExtAudId_Reserved6,
	BRAP_DSPCHN_DtsExtAudId_Reserved7	
} BRAP_DSPCHN_DtsExtAudId;

/***************************************************************************
Summary:
	This structure contains DTS Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_DtsInfo
{
	BRAP_DSPCHN_DtsAmode	eAMode;
								/* This field specifies the number of audio channels and
								the audio playback arrangement.*/
	BRAP_DSPCHN_DtsSourcePcm	eSourcePcm;
	BRAP_DSPCHN_DtsCopyHistory	eCopyHistory;
	BRAP_DSPCHN_DtsLfeFlag		eLfeFlag;
	BRAP_DSPCHN_DtsExtAudId		eExtAudioId;
	uint32_t					ui32VersionNumber;
								/* This field indicates the revision status of the encoder software.
								In addition it is used to indicate the presence of the dialog
								normalization parameters.
								A value of 0-6 indicates a future (compatible) revision.
								A value of 7 indicates the current version.
								A value of 8-15 indicates a future (incompatible) revision.*/
	uint32_t					ui32FrameSize;
								/*This field contains the total byte size of the current frame
								including primary and any extension audio data.
								Valid range is 95 to 16383 */
	uint32_t					ui32NoOfPcmBlocks;
								/* This field specifies the number block of 32 PCM
								samples per channel. Invalid range is 0 to 4 */
	uint32_t					ui32BitRateIndex;
								/* 0 = kbps_32	1 = kbps_56
								2 = kbps_64		3 = kbps_96
								4 = kbps_112		5 = kbps_128
								6 = kbps_192		7 = kbps_224
								8 = kbps_256		9 = kbps_320
								10 = kbps_38		11 = kbps_448
								12 = kbps_512	13 = kbps_576
								14 = kbps_640	15 = kbps_768
								16 = kbps_960	17 = kbps_1024
								18 = kbps_1152	19 = kbps_1280
								20 = kbps_1344	21 = kbps_1408
								22 = kbps_1411_2	23 = kbps_1472
								24 = kbps_1536	25 = kbps_1920
								26 = kbps_2048	27 = kbps_3072
								28 = kbps_3840	29 = Open
								30 = Variable		31 = Lossless */
	uint32_t					ui32BitRate;
								/* This Field has the actual bitrate in
								KiloBits Per Seconds. If ui32BitRateIndex
								is 29, 30 or 31 we have ui32BitRate = 0 */
	bool						bCrcFlag;	/* Whether CRC is present or not */
	bool						bExtAudio;
								/* This field indicates if extended audio coding data
								are present after the core audio data.
								0 = Not_present
								1 = Present */
	bool						bHdcdFormat;
								/*The source material is mastered in HDCD format 
								if bHdcdFormat =1, and otherwise bHdcdFormat=0 */
	bool						bEmbeddedDrc;
								/* The embedded dynamic range coefficients are included
								at the start of each subframe.
								0 = Not_present
								1 = Present */
	bool						bEmbeddedDownMixEn;
								/* The embedded downmixing coefficients are included
								at the start of each subframe.
								0 = Not_present
								1 = Present */
} BRAP_DSPCHN_DtsInfo;


/***************************************************************************
Summary:
	This structure contains DTS-HD Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_DtsHdInfo
{
    bool						bCrcFlag;	
    /* Whether CRC is present or not */
    uint32_t					ui32NoOfPcmBlocks;
    /* This field specifies the number block of 32 PCM
    samples per channel. Invalid range is 0 to 4 */    
    uint32_t					ui32FrameSize;
    /*This field contains the total byte size of the current frame
    including primary and any extension audio data.
    Valid range is 95 to 16383 */								
    BRAP_DSPCHN_DtsAmode	eAMode;
    /* This field specifies the number of audio channels and
    the audio playback arrangement.*/
    uint32_t					ui32TransRateIndex;
    /* The average data rate for the current frame of audio
    0 = kbps_32	1 = kbps_56
    2 = kbps_64		3 = kbps_96
    4 = kbps_112		5 = kbps_128
    6 = kbps_192		7 = kbps_224
    8 = kbps_256		9 = kbps_320
    10 = kbps_38		11 = kbps_448
    12 = kbps_512	13 = kbps_576
    14 = kbps_640	15 = kbps_768
    16 = kbps_960	17 = kbps_1024
    18 = kbps_1152	19 = kbps_1280
    20 = kbps_1344	21 = kbps_1408
    22 = kbps_1411_2	23 = kbps_1472
    24 = kbps_1536	25 = kbps_1920
    26 = kbps_2048	27 = kbps_3072
    28 = kbps_3840	29 = Open
    30 = Variable		31 = Lossless */	
    bool						bEmbeddedDownMixEn;
    /* The embedded downmixing coefficients are included
    at the start of each subframe.
    0 = Not_present
    1 = Present */	
    bool						bEmbeddedDrc;
    /* The embedded dynamic range coefficients are included
    at the start of each subframe.
    0 = Not_present
    1 = Present */	
    bool						bHdcdFormat;
    /*The source material is mastered in HDCD format 
    if bHdcdFormat =1, and otherwise bHdcdFormat=0 */		
    BRAP_DSPCHN_DtsExtAudId		eExtAudioId;								
    bool						bExtAudio;
    /* This field indicates if extended audio coding data
    are present after the core audio data.
    0 = Not_present
    1 = Present */	
    uint32_t					ui32VersionNumber;
    /* This field indicates the revision status of the encoder software.
    In addition it is used to indicate the presence of the dialog
    normalization parameters.
    A value of 0-6 indicates a future (compatible) revision.
    A value of 7 indicates the current version.
    A value of 8-15 indicates a future (incompatible) revision.*/		
    BRAP_DSPCHN_DtsCopyHistory	eCopyHistory;		
            /*	This field indicates the copy history of the audio.*/
    BRAP_DSPCHN_DtsSourcePcm	eSourcePcmResolution;
        /*	This field indicates the quantization resolution of source PCM samples.
        The left and right surround channels of the source material are mastered 
        in DTS ES format if ES = 1.*/


    uint32_t                                        ui32NumChannels;
    /*Total number of channels (primary + LFE) supported by the decoder depending
    on user configuration (e.g if decode_core_only then extension channels will not be counted).*/        

    BAVC_AudioSamplingRate	                eSamplingRate;
    /* Sampling freq value */    

    uint32_t                                        ui32ChannelAllocation;
    /* Output Channel configuration based on the following enum
BRAP_OutputMode_e1_0    			0 
BRAP_OutputMode_e1_1			1 
BRAP_OutputMode_e2_0			2 
BRAP_OutputMode_e3_0			3 
BRAP_OutputMode_e2_1			4 
BRAP_OutputMode_e3_1			5 
BRAP_OutputMode_e2_2			6 
BRAP_OutputMode_e3_2			7 
BRAP_OutputMode_e3_2_1			8 
BRAP_OutputMode_e3_2_2			9 
OUTMODE_NOT_SPECIFIED			10
*/
    
    bool		bLfeFlag;


    uint32_t					ui32BitRate;
    /* This Field has the actual bitrate in
    KiloBits Per Seconds.  */

    uint32_t                                        ui32ErrorStatus;
    /*The decoder sets the relevant bits in the error status based on the type of error.
Bit Field	Description
Bit 0	DTS_CORESUBSTREAM_CORE present
Bit 1	DTS_BCCORE_XXCH present
Bit 2	DTS_BCCORE_X96 present
Bit 3	DTS_BCCORE_XCH present
Bit 4	DTS_EXSUBSTREAM_CORE present
Bit 5	DTS_EXSUBSTREAM_XBR present
Bit 6	DTS_EXSUBSTREAM_XXCH present
Bit 7	DTS_EXSUBSTREAM_X96 present
Bit 8	DTS_EXSUBSTREAM_LBR present
Bit 9	DTS_EXSUBSTREAM_XLL present
Bit 28	It's a 192kHz stream but it can be played at 96kHz and not at 192kHz
Bit 29	Error set by the decoder when the stream characteristic is not supported.
This will fall into Disc-not-supported-in-this-format case. Decoder will
mute the output till some action is taken.
Bit 30	Error set by the decoder when it can run only in CoreOnly mode. The
extension available cannot be supported. In this the decoder will keep
filling output with zeroes.
Bit 31	Error set by the framesync whenever a disparity in the sample rate
programmed by BDVD and as supportable in the stream is detected.
*/

    bool                                	bDTSNeoEnable;
    /* DTS neo enabled/disabled*/

    bool                                     bEsFlag;
    /*true	->	Left and Right surround channels of source material are mastered in DTS ES format.
        false	->	Left and Right surround channels of source material not in DTS ES format.
    */
    bool                                bExtendedCodingFlag;
    /*Indicates if extension audio coding data are present after core audio data.
        false - not present
        true - present
    */
    uint32_t	 ui32ExtensionAudioDescriptorFlag;	
    /*Indicates the type of data that has been placed in the extension stream.
        0	-	 Channel Extension supporting 5.1 plus center surround channel (XCh)
        1	-	 Reserved
        2	-	 Frequency Extension (X96k)
        3	-	 Reserved
        4	-	 Reserved
        5	-	 Reserved
        6	-	Channel Extension supporting additional channels beyond 5.1 (XXCh)
        7	-	 Reserved
        */

    uint32_t	 ui32PCMFrameSize;	
    /*Total byte size of the current frame including primary audio data as well as any
    extension audio data. Valid range for FSIZE: 95 - 16383. Invalid range for FSIZE: 0 - 94.*/


} BRAP_DSPCHN_DtsHdInfo;

/***************************************************************************
Summary:
	This structure contains DTS-Broadcast Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_DtsBroadcastInfo
{
    bool						bCrcFlag;	
    /* Whether CRC is present or not */
    uint32_t					ui32NoOfPcmBlocks;
    /* This field specifies the number block of 32 PCM
    samples per channel. Invalid range is 0 to 4 */    
    uint32_t					ui32FrameSize;
    /*This field contains the total byte size of the current frame
    including primary and any extension audio data.
    Valid range is 95 to 16383 */								
    BRAP_DSPCHN_DtsAmode	eAMode;
    /* This field specifies the number of audio channels and
    the audio playback arrangement.*/
    uint32_t					ui32TransRateIndex;
    /* The average data rate for the current frame of audio
    0 = kbps_32	1 = kbps_56
    2 = kbps_64		3 = kbps_96
    4 = kbps_112		5 = kbps_128
    6 = kbps_192		7 = kbps_224
    8 = kbps_256		9 = kbps_320
    10 = kbps_38		11 = kbps_448
    12 = kbps_512	13 = kbps_576
    14 = kbps_640	15 = kbps_768
    16 = kbps_960	17 = kbps_1024
    18 = kbps_1152	19 = kbps_1280
    20 = kbps_1344	21 = kbps_1408
    22 = kbps_1411_2	23 = kbps_1472
    24 = kbps_1536	25 = kbps_1920
    26 = kbps_2048	27 = kbps_3072
    28 = kbps_3840	29 = Open
    30 = Variable		31 = Lossless */	
    bool						bEmbeddedDownMixEn;
    /* The embedded downmixing coefficients are included
    at the start of each subframe.
    0 = Not_present
    1 = Present */	
    bool						bEmbeddedDrc;
    /* The embedded dynamic range coefficients are included
    at the start of each subframe.
    0 = Not_present
    1 = Present */	
    bool						bHdcdFormat;
    /*The source material is mastered in HDCD format 
    if bHdcdFormat =1, and otherwise bHdcdFormat=0 */		
    BRAP_DSPCHN_DtsExtAudId		eExtAudioId;								
    bool						bExtAudio;
    /* This field indicates if extended audio coding data
    are present after the core audio data.
    0 = Not_present
    1 = Present */	
    uint32_t					ui32VersionNumber;
    /* This field indicates the revision status of the encoder software.
    In addition it is used to indicate the presence of the dialog
    normalization parameters.
    A value of 0-6 indicates a future (compatible) revision.
    A value of 7 indicates the current version.
    A value of 8-15 indicates a future (incompatible) revision.*/		
    BRAP_DSPCHN_DtsCopyHistory	eCopyHistory;		
            /*	This field indicates the copy history of the audio.*/
    BRAP_DSPCHN_DtsSourcePcm	eSourcePcmResolution;
        /*	This field indicates the quantization resolution of source PCM samples.
        The left and right surround channels of the source material are mastered 
        in DTS ES format if ES = 1.*/


    uint32_t                                        ui32NumChannels;
    /*Total number of channels (primary + LFE) supported by the decoder depending
    on user configuration (e.g if decode_core_only then extension channels will not be counted).*/        

    BAVC_AudioSamplingRate	                eSamplingRate;
    /* Sampling freq value */    

    uint32_t                                        ui32ChannelAllocation;
    /* Output Channel configuration based on the following enum
BRAP_OutputMode_e1_0    			0 
BRAP_OutputMode_e1_1			1 
BRAP_OutputMode_e2_0			2 
BRAP_OutputMode_e3_0			3 
BRAP_OutputMode_e2_1			4 
BRAP_OutputMode_e3_1			5 
BRAP_OutputMode_e2_2			6 
BRAP_OutputMode_e3_2			7 
BRAP_OutputMode_e3_2_1			8 
BRAP_OutputMode_e3_2_2			9 
OUTMODE_NOT_SPECIFIED			10
*/
    
    bool		bLfeFlag;


    uint32_t					ui32BitRate;
    /* This Field has the actual bitrate in
    KiloBits Per Seconds.  */

    uint32_t                                        ui32ErrorStatus;
    /*The decoder sets the relevant bits in the error status based on the type of error.
Bit Field	Description
Bit 0	DTS_CORESUBSTREAM_CORE present
Bit 1	DTS_BCCORE_XXCH present
Bit 2	DTS_BCCORE_X96 present
Bit 3	DTS_BCCORE_XCH present
Bit 4	DTS_EXSUBSTREAM_CORE present
Bit 5	DTS_EXSUBSTREAM_XBR present
Bit 6	DTS_EXSUBSTREAM_XXCH present
Bit 7	DTS_EXSUBSTREAM_X96 present
Bit 8	DTS_EXSUBSTREAM_LBR present
Bit 9	DTS_EXSUBSTREAM_XLL present
Bit 28	It's a 192kHz stream but it can be played at 96kHz and not at 192kHz
Bit 29	Error set by the decoder when the stream characteristic is not supported.
This will fall into Disc-not-supported-in-this-format case. Decoder will
mute the output till some action is taken.
Bit 30	Error set by the decoder when it can run only in CoreOnly mode. The
extension available cannot be supported. In this the decoder will keep
filling output with zeroes.
Bit 31	Error set by the framesync whenever a disparity in the sample rate
programmed by BDVD and as supportable in the stream is detected.
*/

    bool                                	bDTSNeoEnable;
    /* DTS neo enabled/disabled*/

    bool                                     bEsFlag;
    /*true	->	Left and Right surround channels of source material are mastered in DTS ES format.
        false	->	Left and Right surround channels of source material not in DTS ES format.
    */
    bool                                bExtendedCodingFlag;
    /*Indicates if extension audio coding data are present after core audio data.
        false - not present
        true - present
    */
    uint32_t	 ui32ExtensionAudioDescriptorFlag;	
    /*Indicates the type of data that has been placed in the extension stream.
        0	-	 Channel Extension supporting 5.1 plus center surround channel (XCh)
        1	-	 Reserved
        2	-	 Frequency Extension (X96k)
        3	-	 Reserved
        4	-	 Reserved
        5	-	 Reserved
        6	-	Channel Extension supporting additional channels beyond 5.1 (XXCh)
        7	-	 Reserved
        */

    uint32_t	 ui32PCMFrameSize;	
    /*Total byte size of the current frame including primary audio data as well as any
    extension audio data. Valid range for FSIZE: 95 - 16383. Invalid range for FSIZE: 0 - 94.*/


} BRAP_DSPCHN_DtsBroadcastInfo;

/***************************************************************************
Summary:
	This enumeration defines Channel Assignment values for LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_BdLpcmChanAssign 
{
	BRAP_DSPCHN_BdLpcmChanAssign_Reserved0,
	BRAP_DSPCHN_BdLpcmChanAssign_Mono,
	BRAP_DSPCHN_BdLpcmChanAssign_Reserved1,
	BRAP_DSPCHN_BdLpcmChanAssign_L_R_Stereo,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_3_0,
	BRAP_DSPCHN_BdLpcmChanAssign_L_R_S_2_1,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_S_3_1,
	BRAP_DSPCHN_BdLpcmChanAssign_L_R_Ls_Rs_2_2,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_Ls_Rs_3_2,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_Ls_Rs_LFE_3_2,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_3_4,
	BRAP_DSPCHN_BdLpcmChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4
} BRAP_DSPCHN_BdLpcmChanAssign;

/***************************************************************************
Summary:
	This enumeration defines Bits per sample for LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_BdLpcmBitsPerSample 
{
	BRAP_DSPCHN_BdLpcmBitsPerSample_Reserved,
	BRAP_DSPCHN_BdLpcmBitsPerSample_16,
	BRAP_DSPCHN_BdLpcmBitsPerSample_20,
	BRAP_DSPCHN_BdLpcmBitsPerSample_24
} BRAP_DSPCHN_BdLpcmBitsPerSample;

/***************************************************************************
Summary:
	This structure contains LPCM BD  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_BdLpcmInfo
{
	BRAP_DSPCHN_BdLpcmChanAssign 	eChnAssignment;
								/* This field specifies the assignment.*/
	BRAP_DSPCHN_BdLpcmBitsPerSample		eBitsPerSample;
								/* This field specifies the sampling resolution of the
								audio samples for all channels in the LPCM audio stream */
	uint32_t					ui32FrameSize;
								/*This field contains the total byte size of the current frame */
	bool						bStartFlag;	/* This field indicates the track-start or index-point */
} BRAP_DSPCHN_BdLpcmInfo;


/***************************************************************************
Summary:
	This enumeration defines Channel Assignment values for HD DVD LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_LpcmHdDvdChanAssign 
{
	BRAP_DSPCHN_LpcmHdDvdChanAssign_Reserved0,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_Mono,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_Reserved1,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_R_Stereo,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_3_0,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_R_S_2_1,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_S_3_1,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_R_Ls_Rs_2_2,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_Ls_Rs_3_2,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_Ls_Rs_LFE_3_2,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_3_4,
	BRAP_DSPCHN_LpcmHdDvdChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4
} BRAP_DSPCHN_LpcmHdDvdChanAssign;

/***************************************************************************
Summary:
	This enumeration defines Bits per sample for HD DVD LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_LpcmHdDvdBitsPerSample 
{
	BRAP_DSPCHN_LpcmHdDvdBitsPerSample_Reserved,
	BRAP_DSPCHN_LpcmHdDvdBitsPerSample_16,
	BRAP_DSPCHN_LpcmHdDvdBitsPerSample_20,
	BRAP_DSPCHN_LpcmHdDvdBitsPerSample_24
} BRAP_DSPCHN_LpcmHdDvdBitsPerSample;

/***************************************************************************
Summary:
	This structure contains LPCM HD DVD  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_LpcmHdDvdInfo
{
	BRAP_DSPCHN_LpcmHdDvdChanAssign 	eChnAssignment;
								/* This field specifies the assignment.*/
	BRAP_DSPCHN_LpcmHdDvdBitsPerSample		eBitsPerSample;
								/* This field specifies the sampling resolution of the
								audio samples for all channels in the LPCM audio stream */
	uint32_t					ui32FrameSize;
								/*This field contains the total byte size of the current frame */
	bool						bStartFlag;	/* This field indicates the track-start or index-point */
} BRAP_DSPCHN_LpcmHdDvdInfo;

/***************************************************************************
Summary:
	This enumeration defines Channel Assignment values for DVD LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_LpcmDvdChanAssign 
{
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved0,
	BRAP_DSPCHN_LpcmDvdChanAssign_Mono,
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved1,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_R_Stereo,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_3_0,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_R_S_2_1,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_S_3_1,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_R_Ls_Rs_2_2,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_Ls_Rs_3_2,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_Ls_Rs_LFE_3_2,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_3_4,
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4,
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved12,
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved13,
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved14,
	BRAP_DSPCHN_LpcmDvdChanAssign_Reserved15
} BRAP_DSPCHN_LpcmDvdChanAssign;

/***************************************************************************
Summary:
	This enumeration defines Bits per sample for DVD LPCM.
***************************************************************************/
typedef enum BRAP_DSPCHN_LpcmDvdBitsPerSample 
{
	BRAP_DSPCHN_LpcmDvdBitsPerSample_Reserved,
	BRAP_DSPCHN_LpcmDvdBitsPerSample_16,
	BRAP_DSPCHN_LpcmDvdBitsPerSample_20,
	BRAP_DSPCHN_LpcmDvdBitsPerSample_24
} BRAP_DSPCHN_LpcmDvdBitsPerSample;

/***************************************************************************
Summary:
	This structure contains LPCM DVD  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_LpcmDvdInfo
{
	unsigned int 					ui32FrameSize;
								/*This field contains the total byte size of the current frame */                                
	BRAP_DSPCHN_LpcmDvdChanAssign 	eChnAssignment;
								/* This field specifies the assignment.*/
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
                                
	BRAP_DSPCHN_LpcmDvdBitsPerSample		eBitsPerSample;
								/* This field specifies the sampling resolution of the
								audio samples for all channels in the LPCM audio stream */

        bool						bStartFlag;	/* This field indicates the track-start or index-point */
        bool    bOutputLfe;	/*
                                            	Output LFE Channel Configuration
                                            	1 = LFE_ON
                                            	0 = LFE_OFF
                                            	*/                                         
} BRAP_DSPCHN_LpcmDvdInfo;

/***************************************************************************
Summary:
	This structure contains WMA standard version Information.
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaStdVersion
{
	BRAP_DSPCHN_WmaStdVersion_eVersion1,
	BRAP_DSPCHN_WmaStdVersion_eVersion2,
	BRAP_DSPCHN_WmaStdVersion_eUnknown
} BRAP_DSPCHN_WmaStdVersion;

/***************************************************************************
Summary:
	This structure contains WMA standard  ACMOD Information.
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaStdAcmod
{
	BRAP_DSPCHN_WmaStdAcmod_eOneCh,	/* One Channel */
	BRAP_DSPCHN_WmaStdAcmod_eTwoCh,	/* Two Channels */
    BRAP_DSPCHN_WmaStdAcmod_NotDefined	
} BRAP_DSPCHN_WmaStdAcmod;

/***************************************************************************
Summary:
	This structure contains WMA standard  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_WmaStdInfo
{
    uint32_t						ui32BitRate;	/* Bit Rate Value */
    uint32_t                                            ui32OriginalCopy; /* Original Copy Bit*/
    uint32_t                                            ui32Copyright; /* Original Copy Bit*/       
    bool                                                  bCrcPresent;/*The field tells the presence of CRC
                                                                                	0 = CRC_Absent
                                                                                	1 = CRC_Present */
    bool						            bLfeOn;			/* LFEON indicates a low-frequency 
                                                					  	 channel effect exists when it is 
                                                					 	  1. Otherwise, the low frequency 
                                                					 	  channel	does not exist. */                                                                     	
    BRAP_DSPCHN_WmaStdStereoMode    eStereoMode;
    BRAP_DSPCHN_WmaStdAcmod	eAcmod;	/* WMA ACMOD value */
    BRAP_DSPCHN_WmaStdVersion	eVersion;	/* WMA version number */

} BRAP_DSPCHN_WmaStdInfo;

/***************************************************************************
Summary:
	This structure contains WMA Pro version Information.
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaProVersion
{
	BRAP_DSPCHN_WmaProVersion_eReserved,
	BRAP_DSPCHN_WmaProVersion_eVersion1,
	BRAP_DSPCHN_WmaProVersion_eVersion2,
	BRAP_DSPCHN_WmaProVersion_eWmaPro
} BRAP_DSPCHN_WmaProVersion;



/***************************************************************************
Summary:
	This structure contains WMA Pro  ACMOD Information.
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaProAcmod
{
	BRAP_DSPCHN_WmaProAcmod_NotDefined,
	BRAP_DSPCHN_WmaProAcmod_eOneCentre_1_0_C,
	BRAP_DSPCHN_WmaProAcmod_eTwoCh_2_0_L_R,
	BRAP_DSPCHN_WmaProAcmod_eThreeCh_3_0_L_C_R,
	BRAP_DSPCHN_WmaProAcmod_eThreeCh_2_1_L_R_S,
	BRAP_DSPCHN_WmaProAcmod_eFourCh_3_1_L_C_R_S,
	BRAP_DSPCHN_WmaProAcmod_eFourCh_2_2_L_R_SL_SR,
	BRAP_DSPCHN_WmaProAcmod_eFiveCh_3_2_L_C_R_SL_SR
} BRAP_DSPCHN_WmaProAcmod;


/***************************************************************************
Summary:
	This structure contains WMA Pro  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_WmaProInfo
{
    uint32_t                                            ui32OriginalCopy; /* Original Copy Bit*/
    uint32_t                                            ui32Copyright; /* Original Copy Bit*/       
    bool                                                  bCrcPresent;/*The field tells the presence of CRC
                                                                    	0 = CRC_Absent
                                                                    	1 = CRC_Present */
    BRAP_DSPCHN_WmaProStereoMode    eStereoMode;
    uint32_t					            ui32BitRate;	/* Bit Rate Value */
    bool						            bLfeOn;			/* LFEON indicates a low-frequency 
    										  	 channel effect exists when it is 
    										 	  1. Otherwise, the low frequency 
    										 	  channel	does not exist. */ 
    BRAP_DSPCHN_WmaProAcmod		eAcmod;			/* WMA ACMOD value */
    BRAP_DSPCHN_WmaProVersion	       eVersion;		/* WMA version number */

} BRAP_DSPCHN_WmaProInfo;

/***************************************************************************
Summary:
	This structure contains MLP Stream Information.
***************************************************************************/
typedef struct BRAP_DSPCHN_MlpInfo
{
	unsigned int tbd;
} BRAP_DSPCHN_MlpInfo;

/***************************************************************************
Summary:
	This structure contains DTS-LBR Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_DtslbrInfo
{
#if 1
    int tbd;
#else
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
	uint32_t				ui32SamplingRateIndex; 	/* The actual value coded in the stream */								   
	bool		bSdmx;		/*DTS-LBR stereo downmix enable flag
								Default value is 0*/
	bool		bLfe;			/* LFE indicates a low-frequency 
								channel effect exists when it is 
								 1. Otherwise, the low frequency 
								 channel	does not exist. */ 
	bool		bSampleSize;	/*DTS-LBR output PCM sample size
							0 = 16 bits Sample
							1 = 24 bits Sample
							Default value is 1 */
	BRAP_DSPCHN_DtslbrAcmod	eAcmod;/* This field specifies the number of audio channels and
								the audio playback arrangement.*/

							
	uint32_t					ui32BitRateIndex;
								/* 0 = kbps_32	1 = kbps_56
								2 = kbps_64		3 = kbps_96
								4 = kbps_112		5 = kbps_128
								6 = kbps_192		7 = kbps_224
								8 = kbps_256		9 = kbps_320
								10 = kbps_38		11 = kbps_448
								12 = kbps_512	13 = kbps_576
								14 = kbps_640	15 = kbps_768
								16 = kbps_960	17 = kbps_1024
								18 = kbps_1152	19 = kbps_1280
								20 = kbps_1344	21 = kbps_1408
								22 = kbps_1411_2	23 = kbps_1472
								24 = kbps_1536	25 = kbps_1920
								26 = kbps_2048	27 = kbps_3072
								28 = kbps_3840	29 = Open
								30 = Variable		31 = Lossless */
	uint32_t					ui32BitRate;
								/* This Field has the actual bitrate in
								KiloBits Per Seconds. If ui32BitRateIndex
								is 29, 30 or 31 we have ui32BitRate = 0 */
#endif								
} BRAP_DSPCHN_DtslbrInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 PCM WAV decoder which is required by PI at every frame 
	 boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_DSPCHN_PcmWavInfo
{
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */

	 uint32_t	ui32NumChannels;    /*Nuo of channels present in the stream*/

}BRAP_DSPCHN_PcmWavInfo;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 AMR decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_DSPCHN_AmrInfo
{
	uint32_t    ui32BitRate;
                        /*
                        	The bitrate of the stream.
                        	0 = 4.75kbps
                        	1 = 5.15kbps
                        	2 = 5.90kbps
                        	3 = 6.70kbps
                        	4 = 7.40kbps
                        	5 = 7.95kbps
                        	6 = 10.2kbps
                        	7 = 12.2kbps
                        	8 = Silence Frame
                        */    
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
}BRAP_DSPCHN_AmrInfo;


typedef enum    BRAP_DSPCHN_DraAcmod
{
    BRAP_DSPCHN_DraAcmod_Cf,
    BRAP_DSPCHN_DraAcmod_Lf_Rf,
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Cr,
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr,
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Cf,
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Cr_Cf,
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Ls_Rs_Cf,    
    BRAP_DSPCHN_DraAcmod_Lf_Rf_Lr_Rr_Ls_Rs_Cr_Cf
}BRAP_DSPCHN_DraAcmod;

/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 DRA decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_DSPCHN_DraInfo
{
    BAVC_AudioSamplingRate	    eSamplingRate; 			/* Sampling freq value */
    unsigned int uiFrameSize;/*This field contains the total frame size in bytes of the current frame.
                                		Max frame size is 4096 bytes*/
    unsigned int uiNumBlocks;/*No. of short window mdct blocks in the frame. Num samples = 128 * ui32NumBlocks = 1024. 
                                    	Hence ui32NumBlocks has a value of 8*/                                          
    BRAP_DSPCHN_DraAcmod    eAcmod;
    bool       bInputLfe;   /*Input LFE present, 0 - LFE Not Present, 1 - LFE Present */                                        
    BRAP_OutputMode eOutMode;
    BRAP_DSPCHN_DraStereoMode   eStereoMode;    
    bool       bOutputLfe;   /*Input LFE present, 0 - LFE Not Present, 1 - LFE Present */               
}BRAP_DSPCHN_DraInfo;


typedef enum    BRAP_DSPCHN_RealAudioLbrAcmod
{
    BRAP_DSPCHN_RealAudioLbrAcmod_Invalid =0,
    BRAP_DSPCHN_RealAudioLbrAcmod_Cf=1,
    BRAP_DSPCHN_RealAudioLbrAcmod_Lf_Rf=2
}BRAP_DSPCHN_RealAudioLbrAcmod;

/*
   This data structure defines Audessey Vol Decoder user configuration parameters 
*/

typedef struct BRAP_DSPCHN_AudysseyVolParams
{
    bool        bAudysseyVolEnable;     /*	Default : False	*/
    									/*	True -> Audyssey Volume(ADV) Enable
    									    False -> Audyssey Volume Disable */

    uint32_t    ui32ChannelMask;		/* Specifying the channels where gain is to be applied
    									   Default: 0xFF	All channels
    									   Meaning of bits
    										0 -> L, 1 -> R, 2 -> C, 3 -> LFE
    										4 -> Ls, 5-> Rs, 6 -> Lb, 7 -> Rb */

    bool        bAudysseyVolInit;       /* Default : False
    									   False -> Do nothing, 
    									   True-> Reset(re-initialize) ADV */
            
    int32_t     i32HeadroomOffset;		/* Headroom offset in dB
    									   Default : 0, Range -> -512 to 512 */

    bool        bSwAudysseyVol;		    /* Dynamic Volume Scaling On/Off
    									   Default : True.
    									   False -> off, True -> on */

    uint32_t    ui32SwDynEQ;			/* Dynamic EQ On/Off/Ramp off  3 modes 0, 1 and 2
    									   Default : 0, range [0,1,2]
    									   0 -> off, 1-> on, 2 -> Ramp off */

    bool        bSwDynSurrGain;         /* Switch for surround channel
    									   Default : True
    									   False -> off, True -> on */
    bool        bSwHiLoCmpress;         /* Switch for having boost in calc cntr gain
    									   Default : True.
    									   False -> off, True -> on */

    int32_t     i32dBVolSetting;		/* For Gain function and Dynamic EQ
    									   Default : 0. Range [0, -500] */
    int32_t     i32GCF;				    /* Gain Compression Factor 
    									   Default :180, Range [1, 240] */

    int32_t     i32chCalbGain[8];		/* chCalbGain in dB. 
                                           Default 0. Range -> -512 to 512 */

    int32_t     i32chCalcLevel[8];		/* chCalcLevel in dB. 
                                           Default 0. Range -> -512 to 512 */

} BRAP_DSPCHN_AudysseyVolParams;


/*
   This data structure defines Audyssey ABX user Decoder configuration parameters 
*/

typedef struct BRAP_DSPCHN_AudysseyABXParams
{	
    bool        bOperationMode;     /* Default : False, Range [0,1] */
                                    /*	True -> Run Audyssey ABX Enable
                                        False -> Bypass Audyssey ABX Disable */

    uint32_t    ui32FilterSet;      /* Default : 1, Range [1,2] 
                                        1->is for systems that roll-off lower (42/46 inch TV) 
                                        2->is for systems that roll-off higher(22 inch TV) */

    uint32_t    ui32HarmonicGain;   /* Default is 6(3 dB), Ranges from 0 to 30
                                        Every step increament is equivalent to 0.5 dB step*/

    uint32_t    ui32DryGain;        /* Default is 0, Ranges from 0 to 30
                                        Typically always 0, Every step increament is equivalent to 0.5 dB step */

} BRAP_DSPCHN_AudysseyABXParams;

/***************************************************************************
Summary:
	The below structure defines SRS StudioSound user configuration parameters.
	This is a common Substucture for top level information of all 4 modules 
	of SRS STUDIOSOUND post-processing

	bIsStudioSound, eStudioSoundMode and bEnable have to be used by all 4 nodes.
	ui32InputGain, ui32HeadroomGain will be used by First module, currently SRS TruVolume. 
	eInputMode will be used by module 0 and module 1, currently SRS TruVolume 
	and Circle Surround Decoder/Tru-Dialog respectively.
	ui32OutputGain and ui32BypassGain will be used by Last module(# 3), curretly Equalizer and Hard limiter.
***************************************************************************/

typedef enum BRAP_DSPCHN_StudioSoundMode
{
	BRAP_DSPCHN_StudioSoundMode_eMode1 = 1,
	BRAP_DSPCHN_StudioSoundMode_eMode2,
	BRAP_DSPCHN_StudioSoundMode_eMode3 = 2  /* Depricated */	
}BRAP_DSPCHN_StudioSoundMode;

typedef enum BRAP_DSPCHN_StudioSound_InputMode
{
    BRAP_DSPCHN_StudioSound_InputMode_eMono,
    BRAP_DSPCHN_StudioSound_InputMode_eStereo,
    BRAP_DSPCHN_StudioSound_InputMode_eMultiChannel,
    BRAP_DSPCHN_StudioSound_InputMode_eLtRt
}BRAP_DSPCHN_StudioSound_InputMode;


typedef struct BRAP_DSPCHN_TopLevelStudioSoundParams
{
    bool        bIsStudioSound;   
                        /*  To identify whether the stage is part of StudioSound
                            or a stand alone configuration (for TVOL and TSHD)
                            This field can change only at Stop/Start of Channel.
                            True  -> Part of StudioSound 
                            False -> Stand Alone configuration
                            Default : False */

    BRAP_DSPCHN_StudioSoundMode    eStudioSoundMode; 
    /*  StudioSound Mode. 
        Select the opearating mode of StudioSound.
        Default : 1. More details are below, 

		Based on mode (eStudioSoundMode), a few of the modules will be disabled by the FW internally. 
		In case of conflict, eStudioSoundMode will have Highest priority i.e., GEQ and TVOL,CSTD,TSHD are mutually exclusive. 
		Different Studio Sound Modes are given Below: 

        eStudioSoundMode = (1) -> TVOL(TVOL On  + HPF on )                   + CSTD(On) + TSHD(On) + EQHL(GEq-Off, HL-On, PEQ on)   
        eStudioSoundMode = (2) -> TVOL(TVOL Off, bypassgain-> 1.0 + HPF on)  + CSTD(Off, bypassgain-> 1.0)+ TSHD(Off, bypassgain-> 1.0) +EQHL(GEq-On, HL-On, PEQ on)         
    */
 
	bool        bEnable;			/*  Enable Circle-Surround and Tru-Dialogue processing 
									    True -> Enable processing, 
									    False -> Bypass processing 
                                        Default: True */  

    uint32_t    ui32InputGain;		/*  SRS StudioSound Input Gain 
    								    Range: (0 - 100), Default: 100 */

	uint32_t    ui32HeadroomGain;	/*  SRS StudioSound Headroom Gain
									    Range: (0 - 100), Default: 100 */

    BRAP_DSPCHN_StudioSound_InputMode    eInputMode;    /*	Channel configuration present in input
                										    Range: [0, 1, 2, 3], Default: 3
                										    0 : 1_0_0
                										    1 : 2_0_0
                										    2 : 3_2_1
                										    3 : LtRt    */

    uint32_t    ui32OutputGain;		/*  SRS StudioSound Output Gain
                                        Range: (0 - 100), Default: 100 */                                        

	uint32_t    ui32BypassGain;	    /*  SRS StudioSound Bypass Gain
       								    Range: (0 - 100), Default: 100 */
}BRAP_DSPCHN_TopLevelStudioSoundParams;

/***************************************************************************
Summary:
	This structure contains post processing algorithm 
	SRS Circle-Surround Decoder and Tru-Dialogue configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_SrsCircleSurroundParams
{
    bool        bEnable;                /*  Enable Circle Surround Decoding
    									    True -> Enable Decoding
    									    False -> Bypass Decoding
                                            Default: True */

    uint32_t    ui32InputGain;          /*  Input Gain
                                            Range: (125 - 1000), Default: 1000 */

    uint32_t    ui32Mode;               /*  Decoder processing mode
                                            Range [1,4], [1:Cinema 4:Music]. Default : 1 */
    
    uint32_t    ui32OutputMode;         /*  Output mode
                                            Range: [0, 1]. Only supported mode is 0 Default : 0
                                            0 - Stereo output mode, 
                                            1 - Multi channel output mode */

	uint32_t    ui32OutputGainLR;       /*  Output gain for LR channels
										    Range: (89 - 4000), Default: 1000 */

	uint32_t    ui32OutputGainLsRs;		/*  Output gain for LsRs channels
											Range: (89 - 4000), Default: 1000 */
	
	uint32_t    ui32OutputGainC;        /*  Output gain for Center channel
											Range: (89 - 4000), Default: 1000 */

	uint32_t    ui32OutputGainSub;      /*  Output gain for Subwoofer channel
											Range: (89 - 4000), Default: 1000 */
}BRAP_DSPCHN_SrsCircleSurroundParams;


typedef struct BRAP_DSPCHN_SrsTruDialogParams
{
    bool        bEnable;                /*  Enable TruDialog processing
    									    True -> Enable Decoding
    									    False -> Bypass Decoding
                                            Default: True */

	uint32_t    ui32ProcessGain;        /*  Final calculated gain applied to final output signal
                                            Range: (50 - 300), Default: 150 */
    
    uint32_t    ui32InputGain;          /*  Input Gain
                                            Range: (0 - 100), Default: 100 */

    uint32_t    ui32OutputGain;         /*  Output Gain
                                            Range: (0 - 100), Default: 100 */

    uint32_t    ui32DialogClarityGain;  /*  Amount of Dialog Clarity enhancement applied to input signal
                                            Range: (0 - 100), Default: 100 */

    uint32_t    ui32BypassGain;        /*   Bypass Gain
                                            Range: (0 - 100), Default: 100 */
}BRAP_DSPCHN_SrsTruDialogParams;

typedef struct BRAP_DSPCHN_SrsCsTdParams
{
    BRAP_DSPCHN_TopLevelStudioSoundParams	    sTopLevelParams;
    BRAP_DSPCHN_SrsCircleSurroundParams        	sCircleSurroundParams;
    BRAP_DSPCHN_SrsTruDialogParams          	sTruDialogParams;    
}BRAP_DSPCHN_SrsCsTdParams;

/***************************************************************************
Summary:
	This structure contains post processing algorithm 
	SRS Equalizer and Hard Limiter configuration parameters
***************************************************************************/
typedef enum BRAP_DSPCHN_CoefGenMode
{
    BRAP_DSPCHN_CoefGenMode_eUserCoef,
    BRAP_DSPCHN_CoefGenMode_eGenFromUserSetting
}BRAP_DSPCHN_CoefGenMode;

typedef enum BRAP_DSPCHN_EQFilterOrder
{
    BRAP_DSPCHN_EQFilterOrder_eOrder0 = 0,
    BRAP_DSPCHN_EQFilterOrder_eOrder2 = 2,
    BRAP_DSPCHN_EQFilterOrder_eOrder4 = 4,  
    BRAP_DSPCHN_EQFilterOrder_eOrder6 = 6
}BRAP_DSPCHN_EQFilterOrder;

/* ---------------  PEQ User Config --------------------------- */

/* Filter specifications for PEQ */
typedef struct BRAP_DSPCHN_PEQFilterSpecifications
{
	int32_t     i32BandGain;		/*  Range: (-120 to 120). Default:0 Format: q24
	                                    Range in dB: -12 to 12 db. Default: 0dB
    								    Range in Fixed point: 0x00404DE6 to 0x03FB2783 */

	uint32_t    ui32BandFrequency;	/*  Range: (20 - 20000)Hz. Default: 1000 Hz */
	int32_t     i32QFactor;			/*  Range: (25 - 1600).  Default:200 Format: q24
	                                    Range in floating point: 0.25 to 16.0
                                        Range in fixed point: 0x00400000 to 0x10000000 */
}BRAP_DSPCHN_PEQFilterSpecifications;

/* Coefficients should be generated by offline utility provided by SRS */
typedef struct BRAP_DSPCHN_PEQFilterCoef
{
	BRAP_DSPCHN_EQFilterOrder   eFilterOrder;	/* ParametricEQ filter order.
	                                               Default: 2 */
	uint32_t				    ui32Scale;	    /* Specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format 
								        	       Default: 1. i32Scale=1 implies a q-format of 2.30 */
	int32_t				        i32FilterCoefficientB0; /* Default Cefficients can be found in brap_dspchn.c */
	int32_t				        i32FilterCoefficientA1;
	int32_t				        i32FilterCoefficientB1;
	int32_t				        i32FilterCoefficientA2;
	int32_t				        i32FilterCoefficientB2;
}BRAP_DSPCHN_PEQFilterCoef;

typedef struct BRAP_DSPCHN_ParametricEqParams
{
    bool        bEnable[2];         /*  Enable Left/Right channel ParametricEQ processing
    								    True -> Enable processing
    									False -> Bypass processing
    									[0] and [1] indicate Left and Right channel respectively
                                        Default: True for both L and R channels */

    bool        bBandEnable[2][BRAP_FW_MAX_PEQ_BANDS];  
                                    /*  Turn ON/OFF a specific band of Left/Right channel
									    True -> Enable band
									    False -> Bypass band
									    [0] and [1] indicate Left and Right channel respectively
									    MAX_PEQ_BANDS=8
                                        Default: True for all bands */
                                            
    uint32_t    ui32InputGain;      /*  Input gain
                                        Range: (0 - 1000), Default: 100 */

    uint32_t    ui32OutputGain;     /*  Output gain
                                        Range: (0 - 1000), Default: 100 */

    uint32_t    ui32BypassGain;     /*  Bypass gain
                                        Range: (0 - 1000), Default: 100 */

    BRAP_DSPCHN_CoefGenMode    eCoefGenMode;    
                                    /* Coefficients generation mode
                                       Default -> 0
                                       0 -> use the coefficients provided by user, in field sPEQFilterCoef[]
                                       1 -> use filter setting provided by user in sPEQFilterSpec[] to generate coefficients */
    union
    {
    	BRAP_DSPCHN_PEQFilterCoef	        sPEQFilterCoef[3][BRAP_FW_MAX_PEQ_BANDS]; 
                                                /* The first array dimension indicates 3 sampling frequencies 32, 44.1 and 48kHz respectively
    	                                           will be used if eCoefGenMode==0 */
    	BRAP_DSPCHN_PEQFilterSpecifications	sPEQFilterSpec[BRAP_FW_MAX_PEQ_BANDS];    
                                                /* Filter specification for generating filter coeffcients, 
	                                               will be used if eCoefGenMode==1 */
    }uPEQFilterSettings;
}BRAP_DSPCHN_ParametricEqParams;
/* ---------------  End of PEQ User Config --------------------------- */

/*----------------  HighPass Filter User Config  --------------------- */

/* Filter specifications for HPF */
typedef struct BRAP_DSPCHN_HPFilterSpecifications
{
	uint32_t        ui32CutoffFrequency;		/* Range: 20 to 1000 hz, Default: 180 Hz */
	BRAP_DSPCHN_EQFilterOrder   eFilterOrder;   /* Range: [2, 4, 6], Default : 4 */
	
}BRAP_DSPCHN_HPFilterSpecifications;

/* Coefficients should be generated by offline utility provided by SRS 
   Tdf - Transfer Direct Form 2 */
typedef struct BRAP_DSPCHN_HPFilterCoefTdf2
{
	uint32_t			ui32Scale;				/* Specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format 
												   Default - 1. i32Scale=1 implies a q-format of 2.30*/
	int32_t				i32FilterCoefficientB0; /* Default Cefficients can be found in brap_dspchn.c */
	int32_t				i32FilterCoefficientB1;
	int32_t				i32FilterCoefficientB2;
	int32_t				i32FilterCoefficientA1;
	int32_t				i32FilterCoefficientA2;
}BRAP_DSPCHN_HPFilterCoefTdf2;

typedef struct BRAP_DSPCHN_HPFilterCoef
{
	BRAP_DSPCHN_EQFilterOrder       eFilterOrder;           /* HighPass Filter order. Range: [2, 4, 6] */

	BRAP_DSPCHN_HPFilterCoefTdf2    sHPFilterCoefTdf2[3];	/* Number of active biquads filter decided by eFilterOrder
                                                               N = eFilterOrder/2 */
	
}BRAP_DSPCHN_HPFilterCoef;


typedef struct BRAP_DSPCHN_HighPassFilterParams
{
    bool        bEnable;            /*  Enable High Pass Filter processing
									    True -> Enable processing
									    False -> Bypass processing
                                        Default: True */    

    BRAP_DSPCHN_CoefGenMode    eCoefGenMode;    /* Coefficients generation mode
                                                   Default -> 0
                								   0 -> use the coefficients provided by user, in field sFilterCoefHpf[]
                								   1 -> use filter setting provided by user in BRAP_DSPCHN_HPFilterSpecifications[] to generate coefficients */

    union
    {
    	BRAP_DSPCHN_HPFilterCoef 	        sHPFilterCoef[3];	    
                                            /* Array size 3, representing supported sampling rates 32, 44.1 and 48kHz, 
                                               will be used if eCoefGenMode==0*/
    	BRAP_DSPCHN_HPFilterSpecifications	sHPFilterSpec;			
                                            /* Filter specification for generating filter coeffcients, 
                                               will be used if eCoefGenMode==1 */
    }uHPFSettings;
}BRAP_DSPCHN_HighPassFilterParams;

/*----------------  End of HighPass Filter User Config  --------------------- */

/* Graphic Equalizer user config */
typedef struct BRAP_DSPCHN_GraphicEqParams
{
    
    bool        bEnable[2];             /*  Enable Left/Right channel GraphicEQ processing
        								    True -> Enable processing
        									False -> Bypass processing
        									[0] and [1] indicate Left and Right channel respectively
                                            Default: True for both L and R channels */

    uint32_t    ui32FilterMode;         /*  GEQ filter configuration									
                                            Range: [0, 1, 2], Default: 0
											0: 5 band config, 
											1: 10 band config, 
											2: Speaker compensation config */
											
    uint32_t    ui32BandGain[2][BRAP_FW_MAX_GEQ_BANDS];
                                        /*  Band gain for a specific band of Left/Right channel
        									[0] and [1] indicate Left and Right channel respectively    
                                            MAX_GEQ_BANDS=10
                                            Range:
    										For 10 band config: (251 to 4000), Default: 1000
    										For 5 band config: (316 to 3160), Default: 1000 */

    uint32_t    ui32InputGain;          /*  Input gain applied to both channels prior to GEQ processing
                                            Range:
											For 10 band config: (0 to 1000), Default: 251
											For 5 band config: (0to 1000), Default: 316 */

    uint32_t    ui32OutputGain;         /*  Output Gain
                                            Range: (0 - 100), Default: 100 */

    uint32_t    ui32BypassGain;         /*  Bypass Gain
                                            Range: (0 - 100), Default: 100 */
}BRAP_DSPCHN_GraphicEqParams;

/*Hard limiter user config*/
typedef struct BRAP_DSPCHN_HardLimiterParams
{
    bool            bEnable;           /*   Enable Hard Limiter provessing
        								    True -> Enable processing
        									False -> Bypass processing
                                            Default: True */

    uint32_t        ui32blockSize;      /*  Block Size
                                            Range: [256, 512, 768, 1024], Default: 256 */      
    
    uint32_t        ui32InputGain;      /*  Input gain
                                            Range: [0 - 400], Default: 100 */
    
    uint32_t        ui32OutputGain;     /*  Input gain
                                            Range: [0 - 400], Default: 100 */
    
    uint32_t        ui32BypassGain;     /*  Bypass gain
                                            Range: [0 - 100], Default: 100 */
    
    uint32_t        ui32LimiterBoost;   /*  HL gain boost
                                            Range: [1, 256000], Default: 2820 */
    
    uint32_t        ui32HardLimit;      /*  HardLimit 
                                            Range: [3, 100000], Default: 100000 */
    
    uint32_t        ui32DelayLength;    /*  DelayLength
                                            Range: [6, 22], Default: 22 */
}BRAP_DSPCHN_HardLimiterParams;

typedef struct BRAP_DSPCHN_SrsEqHlParams
{
    BRAP_DSPCHN_TopLevelStudioSoundParams       sTopLevelParams;
    BRAP_DSPCHN_ParametricEqParams				sPEqParams;    
	BRAP_DSPCHN_HighPassFilterParams    		sHPFParams;
    BRAP_DSPCHN_GraphicEqParams         		sGEqParams;
    BRAP_DSPCHN_HardLimiterParams       		sHardLimiterParams; 
}BRAP_DSPCHN_SrsEqHlParams;


/***************************************************************************
Summary:
	This enumeration specifies the mode of SRS
***************************************************************************/
typedef enum BRAP_DSPCHN_SrsTruVolumeMode
{
    BRAP_DSPCHN_SrsTruVolumeMode_eLight=1,
    BRAP_DSPCHN_SrsTruVolumeMode_eNormal=0,
    BRAP_DSPCHN_SrsTruVolumeMode_eHeavy=2
} BRAP_DSPCHN_SrsTruVolumeMode;

/***************************************************************************
Summary:
	This enumeration specifies the resolution of the speakers that the SRS 
	Tru Volume sound will be played back on. 
***************************************************************************/
typedef enum BRAP_DSPCHN_SrsTruVolumeSpeakerResolution
{
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_20Hz,
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_40Hz,
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_110Hz,
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_200Hz,
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_315Hz,
	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution_410Hz
} BRAP_DSPCHN_SrsTruVolumeSpeakerResolution;

/***************************************************************************
Summary:
	This structure contains  TruVolume configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_SrsTruVolumeParams
{
    BRAP_DSPCHN_TopLevelStudioSoundParams	    sTopLevelParams;
                                /* This paramter is not used.
                                To support compatibility this parameter is kept*/
    
	bool						bSrsTruVolumeEnable;
    							/*  Turns the SRS Tru Volume processing on and off.
    							    TRUE = Turn ON SRS Tru Volume
    							    FALSE = Turn OFF SRS Tru Volume */    

	unsigned int				uiBlockSize;		
                                /* Block Size: Can be 256, 512, 768, 1024 */		

	bool						bEnableNormalGain;	
                                /*  This is to turn on only bypassgain to be applied. 
                                    TRUE = Normal Processing
                                    FALSE = Only Bypassgain is applied if bSrsTruVolumeEnable is also TRUE
                                */

	unsigned int			    uiInputGain;				
                                /*  Input gain 
                                    The RAP PI range is 0 to 800       
                                    Default: 25
                                    Spec Range:0-32 , Default: 1
                                */

	unsigned int				uiOutputGain;				
                                /*  Output gain 
                                    The RAP range is 0 to 100                         
                                    RAP Default: 25                                    
                                    Spec Range:0-4 , Default: 1                                    
                                */

	unsigned int				uiBypassGain;				
                                /*  Bypass gain 
                                    The range is 0 to 100%                                
                                */

	unsigned int				uiReferenceLevel;			
                                /*  Reference Level 
                                    The range is 0 to 100%
                                */

    bool                        bEnableDCNotchFilter;
                                /* Enable DC Notch filter
                                   TRUE = Turn on DC Notch filter 
                                   FALSE = Turn off DC Notch filter
                                */

	BRAP_DSPCHN_SrsTruVolumeMode		        eMode;
                                                /*  SRS Tru Volume Mode
                                                RAP Default - BRAP_DSPCHN_SrsTruVolumeMode_eLight*/

	BRAP_DSPCHN_SrsTruVolumeSpeakerResolution	eSpeakerResolution;	
    							                /*  Specifies the resolution of the speakers 
    							                    that the sound will be played back on. */

	unsigned int				uiMaxGain;				
                                /*  Max Gain Control 
                                    The RAP range is 0 to 1024.                                                                
                                    RAP Default: 64
                                    Spec Range:0.25-256 , Default: 16                                    
                                */

	bool						bEnableNoiseManager;			
                                /* Turns on Noise Manager
                                   TRUE = Turn on Noise manager 
                                   FALSE = Turn off Noise Manager 
                                */
                                
    unsigned int				uiNoiseManagerThreshold;	
                                /* Noise Manager Threshold 
                                    The range is 0 to 200                                                                
                                */
    bool						bEnableNormalizer;			
                                /* This paramter is not used.
                                To support compatibility this parameter is kept*/
                                
    unsigned int				uiCalibrate;	
                                /* This paramter is not used.
                                To support compatibility this parameter is kept*/
                                
	BRAP_DSPCHN_HighPassFilterParams    		sHPFParams;
                                /* This paramter is not used.
                                To support compatibility this parameter is kept*/
                                
}BRAP_DSPCHN_SrsTruVolumeParams;


/***************************************************************************
Summary:
	This structure contains  TruSurroundHd configuration parameters
***************************************************************************/
typedef enum BRAP_DSPCHN_SRSTSHDCrossOverFreq
{
    BRAP_DSPCHN_SRSTSHDCrossOverFreq_e80Hz = 0,
    BRAP_DSPCHN_SRSTSHDCrossOverFreq_e120Hz,
    BRAP_DSPCHN_SRSTSHDCrossOverFreq_e160Hz,
    BRAP_DSPCHN_SRSTSHDCrossOverFreq_e200Hz,
    BRAP_DSPCHN_SRSTSHDCrossOverFreq_eMax
} BRAP_DSPCHN_SRSTSHDCrossOverFreq;

typedef enum BRAP_DSPCHN_SpeakerSize
{
	BRAP_DSPCHN_SpeakerSize_eLFResponse40Hz  = 0,
	BRAP_DSPCHN_SpeakerSize_eLFResponse60Hz  = 1,
	BRAP_DSPCHN_SpeakerSize_eLFResponse100Hz = 2,
	BRAP_DSPCHN_SpeakerSize_eLFResponse150Hz = 3,
	BRAP_DSPCHN_SpeakerSize_eLFResponse200Hz = 4,
	BRAP_DSPCHN_SpeakerSize_eLFResponse250Hz = 5,
	BRAP_DSPCHN_SpeakerSize_eLFResponse300Hz = 6,
	BRAP_DSPCHN_SpeakerSize_eLFResponse400Hz = 7,
    BRAP_DSPCHN_SpeakerSize_eMax
} BRAP_DSPCHN_SpeakerSize;

typedef enum BRAP_DSPCHN_SRSTruBassProcessMode
{
    BRAP_DSPCHN_SRSTruBassProcessMode_eMono,
    BRAP_DSPCHN_SRSTruBassProcessMode_eStereo,
    BRAP_DSPCHN_SRSTruBassProcessMode_eMax
} BRAP_DSPCHN_SRSTruBassProcessMode;
 
typedef enum BRAP_DSPCHN_SRS3DMode
{
    BRAP_DSPCHN_SRS3DMode_eMono,
    BRAP_DSPCHN_SRS3DMode_eSingleSpeaker,
    BRAP_DSPCHN_SRS3DMode_eStereo,
    BRAP_DSPCHN_SRS3DMode_eExtreme,
    BRAP_DSPCHN_SRS3DMode_eMax
} BRAP_DSPCHN_SRS3DMode;

typedef struct BRAP_DSPCHN_TruSurroundHdParams
{
    BRAP_DSPCHN_TopLevelStudioSoundParams	    sTopLevelParams;
                                /* Top Level StudioSound Parameters */
    bool            bEnable;     
							/*  Enable the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */    
    bool            bHeadPhoneEnable; 
							/*  Enable Headphone processing of the TruSurround HD.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */                            
    uint32_t        ui32InputGain; 
                            /*  TruSurround HD Input Gain
                                Range: (0 - 100) */
    uint32_t        ui32OutputGain; 
                            /*  TruSurround HD Output Gain
                                Range: (0 - 100) */
    uint32_t        ui32ByPassGain; 
                            /*  TruSurround HD Bypass Gain
                                Range: (0 - 100) */
    BRAP_DSPCHN_SRSTSHDCrossOverFreq    eSubCrossOverFreq; 
                            /*  Cross Over Frequency. Used only for multichannel mode */
    bool            bTruBassFrontEnable; 
							/*  Enable TruBass portion of the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */                                
    uint32_t        ui32TruBassFrontLevel; 
                            /*  TruBass Level
                                Range: (0 - 100) */
    BRAP_DSPCHN_SpeakerSize eTruBassFrontSpeakerSize; 
                            /*  TruBass Speaker Size*/
    BRAP_DSPCHN_SRSTruBassProcessMode eSRSTruBassProcessMode; 
                            /*  TruBass Process Mode */
    bool            bDefinitionFrontEnable;
							/*  Enable Definition portion of the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */                            
    uint32_t        ui32DefinitionFrontLevel; 
                            /*  Definition Level
                                Range: (0 - 100) */
    bool            bDialogClarityEnable;
							/*  Enable Dialog Clarity portion of the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */                            
    uint32_t        ui32DialogClarityLevel; 
                            /*  Dialog Calrity Level
                                Range: (0 - 100) */
    uint32_t        ui32SurroundLevel;  
                            /*  Surround Level
                                Range: (0 - 100) */
    bool            bWowHDSRS3DEnable;
							/*  Enable WowHD SRS 3D portion of the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */
    bool            bSRS3DHighBitRateEnable; 
							/*  Enable SRS 3D High Bit Rate.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */
    BRAP_DSPCHN_SRS3DMode eWowHDSRS3DMode; 
                            /*  SRS3D Mode */
    uint32_t        ui32WowHDSRS3DSpaceLevel;  
                            /*  WowHD SRS 3D Space Control(Level)
                                Range: (0 - 100) */
    uint32_t        ui32WowHDSRS3DCenterLevel; 
                            /*  WowHD SRS 3D Center Control(Level)
                                Range: (0 - 100) */
    bool            bWowHDFocusEnable;
							/*  Enable WowHD Focus portion of the TruSurround HD processing.
							    TRUE = Turn ON processing
							    FALSE = Turn OFF processing */                            
    uint32_t        ui32WowHDFocusLevel;   
                            /*  WowHD Focus Control(Level)
                                Range: (0 - 100) */
    bool            bMonoToStereoEnable;
							/*  Enable Mono to Stereo conversion.
							    TRUE = Turn ON conversion
							    FALSE = Turn OFF conversion */
    BRAP_DSPCHN_SRS_AcMode eOutputAcMode;  
                            /*  Output AcMode */
    bool            bOutputLFEEnable;
                            /* Output LFE control 
                               TRUE = Enable LFE content in the Output
                               FALSE = Disable LFE content in the Output */            
/*Valid for certification application*/                            
    bool                                        bCertificationApp;       /*Default: false, make it true if Certification app*/                        
    bool                                        bInputLfeEnable;        /*Default: false. TRUE if inputLfe is present.*/
    BRAP_DSPCHN_SRS_AcMode            eInputAcmod;             /*Default: BRAP_DSPCHN_SRS_AcMode_eMode20, i.e. Stereo*/                               
} BRAP_DSPCHN_TruSurroundHdParams;



typedef struct BRAP_DSPCHN_DV258UserConfig
{
    /*General Settings*/

    bool		bDolbyVolEna;				/*Range of values -  0 or 1 */
    										/*0 - dolby volume disabled - acts as pass trough node*/
    										/*1 - dolby volume enabled   */ 
    										/*Default value    : 1 ( dolby volume enabled)  */

    bool		bVolModelerEna;				/*Range of values -  0 or 1 */
    										/*0 -  volume Modeler disabled - acts as pass trough node*/
    										/*1 -  volume Modeler enabled   */ 
    										/*Default value    : 0 ( volume Modeler disabled).
    										Currently It should always be set to Off*/        
    										
    bool     bHalfmodeEna;				/*Flag to operate Dolby Volume in half mode*/
                                                                    /*Range of values -  0=FALSE , 1=TRUE*/
    										/*, default ->false-> disable*/	
                                                                    
    bool		bEnableMidsideProc;			/*Enable midside processing */
                                                            /*Range of values -  0=FALSE , 1=TRUE*/
    										/*, default ->false-> disable*/          

                                                            
    /*Volume Leveler Settings */
    unsigned int        uiVolLvlAmount;                 /*Range of values - [0 to 10]*/
                                                /*Default value     : 9*/

    bool        bVolLvlEnable;                  /*Range of values -  0=FALSE , 1=TRUE*/
                                                /*Default value    : 1 (TRUE - volume leveler enabled)*/


    /* System Settings*/
    unsigned int        uiInputReferenceLevel;          /*Input reference level */
                                                /*RAP Range of values- (0 to 2080 )*/
                                                /*RAP Default value   (1360 ), 
                                                Spec Range In Db: (0 to 130 in steps of 0.0625 dB) */                                             


    unsigned int        uiOutputReferenceLevel;     /*Output reference level  */
                                                /*RAP Range of values- (0 to 2080 )*/
                                                /*RAP Default value   (1360 ), 
                                                Spec Range In Db: (0 to 130 in steps of 0.0625 dB) */ 

    int iPreGain;     /*	The gain, if any, applied to the signal prior to entering DV258 in dB.*/
                            /*RAP Range of values- (0 to 480 )*/
                            /*RAP Default value   (0 ), 
                            Spec Range In Db: (-30 to 30 in steps of 0.0625 dB)*/                                                 

    int		iCalibrationOffset;	/*Calibration Offset  */
                                                /*RAP Range of values- (-480 to 480 )*/
                                                /*RAP Default value   (0 ), 
                                                Spec Range In Db: (-30 to 30 in steps of 0.0625 dB)*/ 



    /*Volume Modeler Settings*/
    int     iDigitalVolumeLevel;            /*Volume level gain -- applied by dolby volume */
                                                /*RAP Range of values- (-1536 to 480)*/
                                                /*RAP Default value   (0 ), 
                                                Spec Range In Db: (-96 to 30 in steps of 0.0625 dB)*/ 

    
    int     iAnalogVolumeLevel;         /*Volume level gain in dB -- applied after dolby volume */
                                                /*RAP Range of values- (-1536 to 480)*/
                                                /*RAP Default value   (0 ), 
                                                Spec Range In Db: (-96 to 30 in steps of 0.0625 dB)*/ 


    bool        bResetNowFlag;              /*User-input forced reset flag */
                                                /*Range [false,true], default false*/


    /* Limiter Settings */

    bool    bLimiterEnable;                            /*Enable Limter*/ 
                                /*When DV is enabled limiter_enable = TRUE
                                  When DV is disabled limiter_enable = FALSE*/                                                                   

}BRAP_DSPCHN_DV258UserConfig;

/***************************************************************************
Summary:
	This enumeration defines DDRE ACMOD values
**************************************************************************/
typedef enum BRAP_DSPCHN_DDRE_Acmode
{
	BRAP_DSPCHN_DDRE_Acmode_eTwoMono_1_ch1_ch2,
	BRAP_DSPCHN_DDRE_Acmode_eOneCentre_1_0_C,
	BRAP_DSPCHN_DDRE_Acmode_eTwoCh_2_0_L_R,
	BRAP_DSPCHN_DDRE_Acmode_eThreeCh_3_0_L_C_R,
	BRAP_DSPCHN_DDRE_Acmode_eThreeCh_2_1_L_R_S,
	BRAP_DSPCHN_DDRE_Acmode_eFourCh_3_1_L_C_R_S,
	BRAP_DSPCHN_DDRE_Acmode_eFourCh_2_2_L_R_SL_SR,
	BRAP_DSPCHN_DDRE_Acmode_eFiveCh_3_2_L_C_R_SL_SR,
    BRAP_DSPCHN_DDRE_Acmode_eINVALID=0x7fffffff	
} BRAP_DSPCHN_DDRE_Acmode;

/***************************************************************************
Summary:
	This enumeration defines downmix level of the centre channel 
**************************************************************************/
typedef enum BRAP_DSPCHN_DDREProfile
{
	BRAP_DSPCHN_DDREProfile_eNoCompression,
	BRAP_DSPCHN_DDREProfile_eFilmStandardCompression,
	BRAP_DSPCHN_DDREProfile_eFilmLightCompression,
	BRAP_DSPCHN_DDREProfile_eMusicStandardCompression,
	BRAP_DSPCHN_DDREProfile_eMusicLightCompression,
	BRAP_DSPCHN_DDREProfile_eSpeechCompression
} BRAP_DSPCHN_DDREProfile;

/***************************************************************************
Summary:
	This enumeration defines downmix type for DDRE.
***************************************************************************/
typedef enum BRAP_DSPCHN_DDRE_DownmixType
{
	BRAP_DSPCHN_DDRE_DownmixType_eLtRt = 0,
	BRAP_DSPCHN_DDRE_DownmixType_eLoRo = 1,
	BRAP_DSPCHN_DDRE_DownmixType_eArib = 2  	
} BRAP_DSPCHN_DDRE_DownmixType;

/***************************************************************************
Summary:
	This enumeration defines Compressed Mode for DDRE.
***************************************************************************/
typedef enum BRAP_DSPCHN_DDRE_CompMode
{
	BRAP_DSPCHN_DDRE_CompMode_eLine = 0,
	BRAP_DSPCHN_DDRE_CompMode_eRf = 1
} BRAP_DSPCHN_DDRE_CompMode;

typedef struct BRAP_DSPCHN_DDREUserConfig
{
	BRAP_OutputMode	                eOutputMode;
                                    /*	Output channel configuration. Supported Values: 
                                        2(Stereo) or 7(3/2 Multichannel). Default = 2(Stereo) */
                                
	bool				            bOutputLfeOn;	
                                    /*	LFE channel on Output. 0: Disabled, 1: Enabled, 
										Default = 0(Disabled). Can only be enabled with 3/2 Multichannel audio */

    BRAP_DSPCHN_DDRE_CompMode	    eCompMode;	/*	Compression mode setting. Default: 0(Line mode). 
                                                    Multichannel output can only be configured for 
										            Line mode in the current reference implementation */

	uint32_t		ui32DrcCutFac;	 /* DRC Cut Scale factor 
                                    	Input value range = 0% to 100%. 100%
										corresponds to value of unity. Any value
										above 100% is considered as unity */

	uint32_t		ui32DrcBoostFac; /* DRC Boost Scale factor 
                                    	Input value range = 0% to 100%. 100%
										corresponds to value of unity. Any value
										above 100% is considered as unity */

	BRAP_DSPCHN_DDRE_DownmixType	eDownmixType; /* Flag indicating the Stereo downmixed mode, is only effective if Outputmode is Stereo. 
										             Default value: 0(LtRt) */ 

	BRAP_DSPCHN_DualMonoMode        eDualMonoMode;/*Flag indicating the dualmono mode which is to be used if the incoming stream is dual mono. 
										            Possible values: 0(Stereo), 1(Left_Only) and 2(Right_Only). Default value: 0(Stereo). */

    bool        bCertificationModeEnabled;   /* True, if it is a Certification case.  If true sExternalPcmSettings must be populated. */

    bool        bExternalPcmModeEnabled; /* Flag indicating a specific DDRE operating mode.  
                                            If true, it is considered the inputs are from non-Dolby MS decoders and 
                                            sExternalPcmSettings must be populated */

    /*  Settings are valid for either of Certification Case or External PCM Mode
        (bCertificationModeEnabled == true || bExternalPcmModeEnabled == true) */     
    struct
    {
    	BRAP_DSPCHN_DDRE_Acmode eInputStreamAcmod; /* AcMode of the input. Valid only for Certification Case. */

    	bool				    bInputHasLfe;   /*	LFE channel presence in the Input. 0: Not Present, 1: Present, 
            										Default = 0(Not Present). Valid only for Certification Case. */

        bool                    bDv258ProcessedInput;   /* Flag to indicate if Input is DV258 Processed. 
                                                           0: NotDv258Processed, 1: Dv258Processed.
                                                           Default = 0(NotDv258Processed). Valid only for Certification Case. */

        BRAP_DSPCHN_DDREProfile eProfile;    /* Compression profiles applicable. The profile of the source of DDRE inputs.
                                                    However any of the profiles can be choosen for processing in DDRE.
                                            		Default Value: 1: "_eFilmStandardCompression" */

        BRAP_DSPCHN_Ac3CMix     eCentreMixLevel; /* Index for Center level in the downmixing case. 
            	                                   Possible Values: 0-3. 
            	                                   Default: 0 */

        BRAP_DSPCHN_Ac3SMix     eSurroundMixLevel;    /* Index for Surround level in the downmixing case. 
                    	                                Possible Values: 0-3. 
                    	                                Default: 0 */

        BRAP_DSPCHN_Ac3DSur     eDolbySurroundMode;  /* Dolby Surround Encoding Mode.
                                                		Default: 0(NOT INDICATED) */

        uint32_t                ui32DialogLevel;	    /* Dialog level of incoming PCM in dB. 
                    	                                   Possible values: 0-31. 
                    	                                   Default: 31 */
    }sExternalPcmSettings;
    /* End of parameters valid for either of Certification Case or External PCM Mode */    
    
}BRAP_DSPCHN_DDREUserConfig;

typedef struct BRAP_DSPCHN_SRCUserConfig
{
    bool    bDummy; /*True, If Certification app, Else False*/
}BRAP_DSPCHN_SRCUserConfig;


/*********************************************************************
Summary:

Error status as sent by the decoder
0 - Frame is sane
1 - Frame is corrupt. O/P has been zero filled
Description:

See Also:
**********************************************************************/


typedef enum    BRAP_DSPCHN_FrameErrorStatus
{
    BRAP_DSPCHN_FrameErrorStatus_Sane,
    BRAP_DSPCHN_FrameErrorStatus_Corrupt
}BRAP_DSPCHN_FrameErrorStatus;


/*********************************************************************
Summary:
     This structure defines all the stream information parameters of 
	 Real Audio LBR decoder which is required by PI at every frame boundary.

Description:

See Also:
**********************************************************************/

typedef struct BRAP_DSPCHN_RealAudioLbr
{
    BAVC_AudioSamplingRate	    eSamplingRate; 			/* Sampling freq value */
    unsigned int uiFrameSize;/*This field contains the total frame size in bytes of the current frame.
                                		Max frame size is 4096 bytes*/                                      
    BRAP_DSPCHN_RealAudioLbrAcmod    eAcmod; 
    BRAP_OutputMode eOutMode;
}BRAP_DSPCHN_RealAudioLbr;



/***************************************************************************
Summary:
	This structure contains following information.
	    cumulative count for number of Valid frames decoded
	    cumulative count for number of Dummy decodes done by decoder
	    cumulative count for number of erroneous frames detected by decoder.

***************************************************************************/
typedef struct BRAP_DSPCHN_FrameDecAndErrInfo
{
/*
	This is the cumulative count for number of Valid frames decoded
*/
	unsigned int	uiTotalFramesDecoded;
/*
	This is the cumulative count for number of Dummy decodes done by decoder
*/
	unsigned int	uiTotalFramesDummy;
/*
	This is the cumulative count for number of erroneous frames detected by decoder.
*/
	unsigned int	uiTotalFramesInError;

        /*This is the cumulative number of samples decoded by any decoder from the
        channel open time. Decoder should accumulate number of samples decoded for
        every frame. Dummy samples are not included.*/

        unsigned int    ui32NumSamplesDecoded;     


        BRAP_DSPCHN_FrameErrorStatus    eFrameErrorStatus;


}BRAP_DSPCHN_FrameDecAndErrInfo;

/***************************************************************************
Summary:
	This structure contains Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_StreamInfo
{
	BRAP_DSPCHN_AudioType       eType;          /* audio type */
	BAVC_AudioSamplingRate	    eSamplingRate; 	/* Sampling freq value */
       BRAP_DSPCHN_FrameDecAndErrInfo  sFrameDecAndErrInfo;

	union
    {
		BRAP_DSPCHN_MpegInfo        sMpegInfo;      /* MPEG stream info */
		BRAP_DSPCHN_Ac3Info         sAc3Info;       /* AC3 stream info */
		BRAP_DSPCHN_Ac3PlusInfo     sAc3LosslessInfo;/* AC3 Lossless stream info */
		BRAP_DSPCHN_Ac3PlusInfo     sAc3PlusInfo;   /* AC3 PLUS stream info */
		BRAP_DSPCHN_AacInfo         sAacInfo;		/* AAC stream info */
		BRAP_DSPCHN_AacSbrInfo      sAacSbrInfo;	/* AAC SBR stream info */
		BRAP_DSPCHN_DtsInfo	        sDtsInfo;	    /* DTS stream info */
   		BRAP_DSPCHN_DtsHdInfo       sDtsHdInfo;	    /* DTS HD stream info */
		BRAP_DSPCHN_BdLpcmInfo	    sBdLpcmInfo;	/* LPCM BD stream info */
		BRAP_DSPCHN_LpcmHdDvdInfo	sLpcmHdDvdInfo;	/* LPCM HD DVD stream info */        
		BRAP_DSPCHN_LpcmDvdInfo	    sLpcmDvdInfo;	/* LPCM DVD stream info */ 
		BRAP_DSPCHN_WmaStdInfo		sWmaStdInfo;	/* WMA Standard stream info */
		BRAP_DSPCHN_MlpInfo			sMlpInfo;		/* MLP stream info */
        BRAP_DSPCHN_DtslbrInfo		sDtslbrInfo;	/* DTS LBR stream info */
		BRAP_DSPCHN_Ac3PlusInfo     sDdp7_1Info;	/* DDp 7.1 stream info */	
    	BRAP_DSPCHN_MpegMcInfo		sMpegMcInfo;	/* DTS LBR stream info */	
        BRAP_DSPCHN_WmaProInfo      sWmaProInfo;    /* Wma Pro stream info */
        BRAP_DSPCHN_DtsBroadcastInfo    sDtsBroadcastInfo;   /* DTS Broadcast streaminfo */
        BRAP_DSPCHN_PcmWavInfo      sPcmWavInfo;/*PCM On AVI Info*/
        BRAP_DSPCHN_AmrInfo         sAmrInfo;       /*AMR stream Info*/        
        BRAP_DSPCHN_DraInfo         sDraInfo;       /*AMR stream Info*/                
        BRAP_DSPCHN_RealAudioLbr    sRealAudioLbrInfo;       /*AMR stream Info*/    
        BRAP_DSPCHN_DolbyPulseInfo  sDolbyPulseInfo;/* Dolby Pulse Info */          
	}uStreamInfo;

}BRAP_DSPCHN_StreamInfo;

typedef enum BRAP_DSPCHN_PreferredDwnMixMode
{
    BRAP_DSPCHN_PreferredDwnMixMode_eLTRT=0,
    BRAP_DSPCHN_PreferredDwnMixMode_eLORO,
    BRAP_DSPCHN_PreferredDwnMixMode_ePLII 
}BRAP_DSPCHN_PreferredDwnMixMode;

/***************************************************************************
Summary:
	This structure contains FW debug Information for stream type DDP.

***************************************************************************/

typedef struct BRAP_DSPCHN_DdpDebugInfo
{
    uint32_t    ui32CMixLev;    
    uint32_t    ui32SurMixLev;
    uint32_t    ui32DMixMod;
    uint32_t    ui32LtRtCMixLev;
    uint32_t    ui32LtRtSurMixLev;
    uint32_t    ui32LoRoCMixLev;
    uint32_t    ui32LoRoSurMixLev;
    BRAP_DSPCHN_PreferredDwnMixMode ePreferredDwnMixMode;
}BRAP_DSPCHN_DdpDebugInfo;

/***************************************************************************
Summary:
	This structure contains FW debug Information for stream type DTS.

***************************************************************************/

typedef struct BRAP_DSPCHN_DtsDebugInfo
{
    uint32_t    ui32DynDnMixCoef[8][8];
}BRAP_DSPCHN_DtsDebugInfo;

/***************************************************************************
Summary:
	This structure returns information of sample rate change in stream
Notes: 
    This structure will be passed on to application callback function
    on sample rate change interrupt
***************************************************************************/
typedef struct BRAP_DSPCHN_SampleRateChangeInfo
{
	BRAP_DSPCHN_AudioType   eType;   /* audio type */
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
} BRAP_DSPCHN_SampleRateChangeInfo;


/***************************************************************************
Summary:
	This structure returns information of sample rate change in stream
Notes: 
    This structure will be passed on to application callback function
    on sample rate change interrupt
***************************************************************************/
typedef struct BRAP_DSPCHN_DestinationSampleRateChangeInfo
{
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */
} BRAP_DSPCHN_DestinationSampleRateChangeInfo;

/***************************************************************************
Summary:
	This structure returns information of bit rate change in stream

Notes: 
    This structure will be passed on to application callback function
    on bit rate change interrupt
***************************************************************************/
typedef struct BRAP_DSPCHN_BitRateChangeInfo
{
    BRAP_DSPCHN_AudioType   eType;   /* audio type */
    uint32_t ui32BitRate;    /* New Bit Rate value*/
					     /* If eType = DTS or DTS-HD and 
					     ui32BitRateIndex = 29,30 or 31
					     ui32BitRate = 0 */
    uint32_t ui32BitRateIndex;    /* This has the Bit rate index
    							as given in the standard. This value
    							is zero for audio type AAC-HE*/
} BRAP_DSPCHN_BitRateChangeInfo;

/***************************************************************************
Summary:
	This structure returns information of mode change in stream

Notes: 
    This structure will be passed on to application callback function
    on mode change interrupt
***************************************************************************/
typedef struct BRAP_DSPCHN_ModeChangeInfo
{
    BRAP_DSPCHN_AudioType   eType;   /* audio type */
    union
    {
        BRAP_DSPCHN_MpegChannelMode eChmod;     /* New MPEG mode for MPEG*/
        BRAP_DSPCHN_Ac3Acmod        eAcmod;     /* ACMOD for AC3, AC3 LOSSLESS, AC3Plus */
        BRAP_DSPCHN_AacAcmod		eAacAcmod;	/* ACMOD for AAC, AAC SBR */
        BRAP_DSPCHN_WmaStdAcmod	    eWmaStdAcmod;	/* ACMOD for WMA_STD */
        BRAP_DSPCHN_WmaProAcmod	    eWmaProAcmod;	/* ACMOD for WMAPRO_STD */
        BRAP_DSPCHN_DtsAmode	    eDtsBroadcastAcmod;	/* ACMOD for DTS Broadcast */
        BRAP_DSPCHN_DtsAmode	    eDtsHdAcmod;	/* ACMOD for DTS Broadcast */        
        BRAP_DSPCHN_DraAcmod       eDraAcmod;   /*ACMOD for DRA*/
        
    } uModInfo;
} BRAP_DSPCHN_ModeChangeInfo;

/***************************************************************************
Summary:
	This structure returns information about logged TSM data. 

***************************************************************************/
typedef struct BRAP_DSPCHN_TsmLogInfo
{
	unsigned int	uiTsmLogBufAdr;	/* Address of TSM log buffer address */
	unsigned int	uiTsmLogBufSize;/* Size of TSM log buffer */
} BRAP_DSPCHN_TsmLogInfo;


/***************************************************************************
Summary:
	This enumeration defines decoder TSM status.
***************************************************************************/
typedef enum BRAP_DSPCHN_TsmStatus
{
	BRAP_DSPCHN_TsmStatus_eDecode = 0,	/* Decode the current frame */
	BRAP_DSPCHN_TsmStatus_eDiscard,		/* Discard the current frame */
	BRAP_DSPCHN_TsmStatus_eWait			/* Decode when PTS matures */
} BRAP_DSPCHN_TsmStatus;

/***************************************************************************
Summary:
	This structure contains TSM related information for debug purpose. 
***************************************************************************/
typedef struct BRAP_DSPCHN_TsmDebugInfo
{
    bool                                 bPlayback;
    int32_t                             i32PtsOffset;	/*Current PTS Offset value in 45Khz ticks*/
    int32_t                             i32DiscardThreshold;/*Current  Discard threshold value in ms*/
    int32_t	                             i32TsmGAThreshold  ; /*Current GA threshold in ms*/
    int32_t                             i32TsmSLThreshold ;  /*Current SL threshold  in ms*/
    bool                                 bEnableASTMFlag;    /*Current  ASTM flag*/
    int32_t	                             i32TsmTransitionThreshold;/*Current transition threshold  in ms*/
    bool                                 bStcValidFlag;/*Current  STC Valid flag*/
    bool                                 bEnableTSMFlag; /*Current  TSM flag*/   
    BRAP_DSPCHN_PtsInfo     sPtsInfo;/*PTS Info*/    
} BRAP_DSPCHN_TsmDebugInfo;

typedef struct BRAP_DestinationRbufIsrStatus
{
   BRAP_OutputChannelPair eChannelPair;
} BRAP_DestinationRbufIsrStatus;

/*********************************************************************
Summary: Enumeration to describe supported audio processing algorithms
Description:
    This enumeration describes all the supported audio processing algorithms. It
    contains pre-processing and post-processing algorithms. 
See Also:
**********************************************************************/ 
typedef enum BRAP_ProcessingType
{
    BRAP_ProcessingType_ePassthru,
    BRAP_ProcessingType_eEncodeStartIndex,    
    BRAP_ProcessingType_eEncodeMp3 = BRAP_ProcessingType_eEncodeStartIndex,
    BRAP_ProcessingType_eEncodeDts,
    BRAP_ProcessingType_eEncodeAc3,
    BRAP_ProcessingType_eEncodeSbc, /*SBC Encode*/        
    BRAP_ProcessingType_ePostprocessingStartIndex, /* Maake sure that all the Processing type which is
    												not downloaded at start time should come before this */
    BRAP_ProcessingType_eConvertDdpToAc3 = BRAP_ProcessingType_ePostprocessingStartIndex,
    BRAP_ProcessingType_eWmaProPassThru,      
    BRAP_ProcessingType_ePl2,           /* Pro-Logic II post processing */
    BRAP_ProcessingType_eSrsXt,         /* SRS-XT post processing */
    BRAP_ProcessingType_eAvl,           /* AVL post processing */
    BRAP_ProcessingType_eXen,           
    BRAP_ProcessingType_eBbe,
    BRAP_ProcessingType_eCustomSurround,/* Custom Surround */
    BRAP_ProcessingType_eCustomBass,    /* Custom Bass */
    BRAP_ProcessingType_eCustomVoice,   /* Custom Voice */ 
    BRAP_ProcessingType_eAdPan,         /* Audio descriptor panning */
    BRAP_ProcessingType_eAdFade,        /* Audio descriptor fading */    
    BRAP_ProcessingType_eSRC,           /* SRC */
    BRAP_ProcessingType_ePCMRouter,     /* PCM Router */               
    BRAP_ProcessingType_eDSOLA,         /* DSOLA */
    BRAP_ProcessingType_eSrsHd,         /* SRS-HD post processing */  
    BRAP_ProcessingType_eGenericPassthru,/*Generic Passthru processing */  
    BRAP_ProcessingType_eSrsTruVolume,  /* SRS-Tru Volume post processing */   
    BRAP_ProcessingType_eDolbyVolume, /*Dolby Volume*/
    BRAP_ProcessingType_eBrcm3DSurround,    /* Broadcom 3D surround */
    BRAP_ProcessingType_eMonoDownmix,       /* Mono Downmix post processing */
    BRAP_ProcessingType_eFwMixer,           /* Firmware Mixer post processing */
    BRAP_ProcessingType_eAudysseyVolume,    /* Audyssey Volume post processing */
    BRAP_ProcessingType_eAudysseyABX,       /* Audyssey ABX post processing */
    BRAP_ProcessingType_eSrsCsTd,           /* SRS Circle-Surround and Tru-Dialogue */
    BRAP_ProcessingType_eSrsEqHl,           /* SRS Equalizer and Hard Limiter */
    BRAP_ProcessingType_eDV258,       /*DV258*/    
    BRAP_ProcessingType_eDDRE,       /*DDRE */        
    BRAP_ProcessingType_eBtsc,       /*BTSC */            
    BRAP_ProcessingType_eNone,    
    BRAP_ProcessingType_eMax
} BRAP_ProcessingType;

/*********************************************************************
Summary: Structure to hold settings of an audio processing stage.
Description:
    This structure contains settings of an audio processing stage. All the fields in this
    structure are programmable on-the-fly unless noted otherwise in the comments.
See Also:
**********************************************************************/ 
typedef struct BRAP_ProcessingStageSettings
{
    BRAP_ProcessingType    eAudioProcessing;    /* Audio processing type. This
                                                   field is not programmable
                                                   on-the-fly. It can be changed
                                                   only when all the channels using 
                                                   this processing stage are in "stop"
                                                   condition */
    union
    {
        BRAP_DSPCHN_TruSurroundXtParams             sSrsXtParams;
        BRAP_DSPCHN_TruSurroundHdParams             sSrsHdParams;        
        BRAP_DSPCHN_SrsTruVolumeParams                    sSrsTruVolumeParams;
        BRAP_DSPCHN_ProLogicllConfigParams          sPl2Params;
        BRAP_DSPCHN_AVLConfigParams                 sAvlParams;
        BRAP_DSPCHN_XenConfigParams                 sXenParams;
        BRAP_DSPCHN_BbeConfigParams                 sBbeParams;
        BRAP_DSPCHN_CustomSurroundConfigParams      sCustomSurroundParams;
        BRAP_DSPCHN_CustomBassConfigParams          sCustomBassParams;
        BRAP_DSPCHN_CustomVoiceConfigParams         sCustomVoiceConfigParams;     
        BRAP_DSPCHN_PeqConfigParams                 sPeqConfigParams;        
        BRAP_DSPCHN_AdPanParams                     sAdPanParams; 
        BRAP_DSPCHN_DTSENCParams                    sDTSENCParams;
        BRAP_DSPCHN_Ac3ENCParams                    sAc3ENCParams;
        BRAP_DSPCHN_DolbyVolumeUserConfig       sDolbyVolConfigParams;        
        BRAP_DSPCHN_Mp3EncUserParam               sMp3EncConfigParams;        
        BRAP_DSPCHN_SbcEncUserParam               sSbcEncConfigParams;                        
        BRAP_DSPCHN_Brcm3DSurroundParams            sBrcm3DSurroundParams;
        BRAP_DSPCHN_MonoDownmixParams               sMonoDownmixParams;
        BRAP_DSPCHN_AudysseyVolParams               sAudysseyVolParams;
        BRAP_DSPCHN_AudysseyABXParams               sAudysseyABXParams;        
        BRAP_DSPCHN_SrsCsTdParams                   sSrsCsTdParams;
        BRAP_DSPCHN_SrsEqHlParams                   sSrsEqHlParams;
        BRAP_DSPCHN_DV258UserConfig             sDV258Params;
        BRAP_DSPCHN_DDREUserConfig    sDDREParams;        
        BRAP_DSPCHN_SRCUserConfig    sSRCParams;                
        BRAP_DSPCHN_BtscUserParams                  sBTSCParams;                        
    } uConfigParams;                            /* Union to describe configuration
                                                   parameters for processing types.
                                                   The fields of union are 
                                                   programmable on-the-fly */
} BRAP_ProcessingStageSettings;

/***************************************************************************
Summary: Provides the Post Processing Algorithm info
***************************************************************************/

typedef struct BRAP_DSPCHN_ProcessingInfo
{
	BRAP_ProcessingType       eProcessingType;              /* Post Processing Type */

	union 
    {
		BRAP_DSPCHN_CustomVoiceInfo     sCustomVoiceInfo;   /* Custom Voice Info */
        BRAP_DSPCHN_AudysseyVolumeInfo  sAuddseyVolumeInfo; /* ADV Info */
	}uProcessingInfo;

}BRAP_DSPCHN_ProcessingInfo;

/***************************************************************************
Summary:
	This Structue will have information of Seconday mixing meta data cofficient.
***************************************************************************/
typedef struct BRAP_DSPCHN_SecMetaDataInfo
{
    uint32_t    ui32mixmdate;
    uint32_t    ui32mixdef;
    uint32_t    ui32mixdata[17];      
    uint32_t    ui32extpgmscle;
    uint32_t    ui32extpgmscl;
    uint32_t    ui32paninfoe[2];
    uint32_t    ui32paninfo[2];
}BRAP_DSPCHN_SecMetaDataInfo;  

/********************************************************

Summary:

    Data structure containing Frame info for DTS-HD.

********************************************************/

typedef struct BRAP_DSPCHN_DtsHdFrameInfo

{
    bool     bCoreSubStreamPresent;
    bool     bBCCoreXxchPresent;
    bool     bBCCoreX96Present;
    bool     bBCCoreXChPresent;    
    bool     bExSubStreamCorePresent;
    bool     bExSubStreamXbrPresent;
    bool     bExSubStreamXxChPresent;
    bool     bExSubStreamX96Present;
    bool     bExSubStreamLbrPresent;    
    bool     bExSubStreamXllPresent;        
    bool     bErrorStreamChar;
    bool     bErrorCoreOnly;
    bool     bErrorFrameSyn;
    
}BRAP_DSPCHN_DtsHdFrameInfo;

/********************************************************

Summary:

    Data structure containing Frame info for Specified algo to be returned via 

interrupt callback registered for BRAP_Interrupt_eFrameInfo.

********************************************************/

typedef struct BRAP_DSPCHN_FrameInfo

{
    BRAP_DSPCHN_AudioType           eAlgoType;         /* Audio type for which this structure is getting initialized */
    union
    {
        BRAP_DSPCHN_DtsHdFrameInfo     sDtsHdFrameInfo;
    }uFrameInfo;

}BRAP_DSPCHN_FrameInfo;

/********************************************************
Summary: 
    Data structure containing the details of the Algorithm(Codec) being implemented.
********************************************************/
typedef struct BRAP_DSPCHN_DecoderVersionInfo
{
    uint8_t     ui8AlgoProfileId;
    uint8_t     ui8AlgoVersionId[4];
    uint8_t     ui8FWVersionId;    
}BRAP_DSPCHN_DecoderVersionInfo;

/********************************************************
Summary: 
    Data structure containing the details of the Algorithm(Codec) being implemented.
********************************************************/
typedef struct BRAP_DSPCHN_EncoderVersionInfo
{
    uint8_t     ui8AlgoProfileId;
    uint8_t     ui8AlgoVersionId[4];
    uint8_t     ui8FWVersionId;    
}BRAP_DSPCHN_EncoderVersionInfo;

/***************************************************************************
Summary: Debuug Secondary Metadata information

Description:
    API that returns the BRAP_DSPCHN_SecMetaDataInfo structure as populated
    by the Secondary decode DSP FW.  

	Note: This API is valid only for a Secondary decode channel.
	
Returns:
	BERR_SUCCESS - If successful
**************************************************************************/
BERR_Code
BRAP_GetSecMetadataInfo(
    BRAP_ChannelHandle              hRapCh,
    BRAP_DSPCHN_SecMetaDataInfo     *pSecMetadataInfo
    );

/***************************************************************************
Summary: Enable/Disable Time Stamp Management

Description:
	This function enables/disables Time Stamp Management for a given audio
	channel. 
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_EnableTSM(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			    bEnableTSM		/* [in] true = enable TSM,
							               false = disable TSM  */
	);

/***************************************************************************
Summary: ISR version of BRAP_EnableTSM

Description:
	This function enables/disables Time Stamp Management for a given audio
	channel in isr context.
	Note: This API is valid only for a decode channel.	
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_EnableTSM_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			    bEnableTSM	    /* [in] true = enable TSM,
							               false = disable TSM  */
	);

/***************************************************************************
Summary: Get stream Information

Description:
	This function returns stream information of the decoded stream, if
	such information is available (i.e. DSP firmware has decoded atleast
	one audio frame and initialized stream information registers).
	Otherwise it returns error.
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_GetStreamInformation(
	BRAP_ChannelHandle	    hRapCh,		    /* [in] Audio channel handle */
	BRAP_DSPCHN_StreamInfo  *psStreamInfo	/* [out] Stream Information */
	);

/***************************************************************************
Summary: Get post processing Information

Description:
	This function returns post processing information of the post processing algorithm, if
	such information is available (i.e. DSP firmware has decoded atleast
	one audio frame and post processing modules had got enough information).
	Otherwise it returns error.
	Note: This API is valid only if a post processing stage has been added and is functional.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_GetProcessingInformation(
	BRAP_ProcessingStageHandle hAudioProcessingStageHandle, /* [in] Processing Stage handle */
	BRAP_DSPCHN_ProcessingInfo *psProcessingInfo	        /* [out]Processing Information */
	);

/***************************************************************************
Summary: Enable/Disable TSM logging 
 
Description:
	This function enables/disables TSM logging feature in firmware. If TSM
	logging is enabled, firmware logs TSM debug data into ping-pong buffers
	and interrupts RAP PI. RAP PI interrupt handler initializes structure
	BRAP_DSPCHN_TsmLogInfo and pass it on to application callback function.
	Application function evcuates data from the log buffer.
	Note: This API is valid only for a decode channel.

Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_EnableTsmLog(
	BRAP_ChannelHandle		hRapCh,		    /* [in] Audio channel handle */
	bool					bEnable			/* [in] true = Enable TSM log
											   false = Disable TSM log */
	);

/***************************************************************************
Summary: Get TSM related information 

Description:
	This function returns "Time Stamp Management" related information. This
	function is introduced for debug purpose.
	Note: This API is valid only for a decode channel.

Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_GetTsmDebugInfo(
	BRAP_ChannelHandle		    hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_TsmDebugInfo	*psTsmInfo		/* [out] TSM related info */
	);

/***************************************************************************
Summary: Get decoder lock/unlock status 
 
Description:
	This function returns decoder locked/unlocked status. When decoder
	finds frame sync pattern in the stream, this function returns decoder
	lock status otherwise it returns decoder unlock status.
	Note: This API is valid only for a decode channel.	

Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_GetDecoderLockStatus(
	BRAP_ChannelHandle		hRapCh,			/* [in] Audio channel handle */
	bool					*pbLock			/* [out] true = Decoder locked 
											   false = Decoder unlocked */
	);

/***************************************************************************
Summary: Get current PTS

Description:
	This function returns the value of current PTS
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_GetCurrentPTS(
	BRAP_ChannelHandle	    hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS information */ 
	);

/***************************************************************************
Summary: ISR version of Get current PTS

Description:
	This function returns the value of current PTS in ISR context
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_GetCurrentPTS_isr(
	BRAP_ChannelHandle	    hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS information */ 
	);

/***************************************************************************
Summary: 
	Decoder FW state as required by the BRAP_GetFwTimingMarkerState() API.
****************************************************************************/
typedef enum BRAP_DSPCHN_FwTMState
{
	BRAP_DSPCHN_FwTMState_eClose,
	BRAP_DSPCHN_FwTMState_eIdle,
	BRAP_DSPCHN_FwTMState_eDecoding,
	BRAP_DSPCHN_FwTMState_eStopped,
	BRAP_DSPCHN_FwTMState_eMax
}BRAP_DSPCHN_FwTMState;

/***************************************************************************
Summary: 
	Data structure returned by BRAP_GetFwTimingMarkerState() API.
****************************************************************************/
typedef struct BRAP_DSPCHN_FwTimingMarker
{
	BRAP_DSPCHN_FwTMState 	eFwState;		/* Decoder state [Close | Idle | Decoding | Stopped] */
	bool 			        bPresenting; 	/* Presenting */
	bool			        bTsmError; 		/* TSMError */
    /* 	"	STC - only valid in the decoding and stopped state. App already has this so we need to pass them this. */
	uint32_t		        ui32Pts; 		/* PTS - only valid when bPresenting = TRUE */
	uint32_t				ui32Stc;
}BRAP_DSPCHN_FwTimingMarker;

/***************************************************************************
Summary: 
	Data structure returned by BRAP_GetFwTimingMarkerState() API.
****************************************************************************/
typedef struct BRAP_DSPCHN_HostTimingMarker
{
	uint32_t		        ui32Pts; 		/* PTS - only valid when bPresenting = TRUE */
}BRAP_DSPCHN_HostTimingMarker;

/***************************************************************************
Summary: 
	GetCurrenTimingMarkerState() API
****************************************************************************/
BERR_Code BRAP_GetCurrentTimingMarkerState(
	BRAP_ChannelHandle 			hRapCh,
	BRAP_DSPCHN_FwTimingMarker	*psCurrTimingMarkerState
);


/***************************************************************************
Summary:
    For UOD2, just ui32StartPTS value is honored.
**************************************************************************/
typedef struct BRAP_DSPCHN_ClipInfo
{
	uint32_t 			ui32ClipId;			/* Clip Id. Valid only for ??? */
	uint32_t 			ui32StcOffset;		/* STC Offset. Valid only for ??? */
	uint32_t 			ui32StartPTS;		/* Start PTS value - for UOD2,  */
	uint32_t 			ui32StopPTS;		/* Stop PTS value */    
}BRAP_DSPCHN_ClipInfo;

/***************************************************************************
Summary: Sets start PTS and stop PTS values

Description:
	This function sets the start PTS and stop PTS values for a decode channel.
	Note: This API is valid only for a decode channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
#if BRAP_NEW_TIMING_MARKER
BERR_Code
BRAP_SetStartStopPts (
	BRAP_ChannelHandle		hRapCh,	    	/* [in] Audio channel handle */
    BRAP_DSPCHN_ClipInfo	*pClipInfo		/* [in] Audio clip info */	
	);
#else
BERR_Code
BRAP_SetStartStopPts (
	BRAP_ChannelHandle	hRapCh,			    /* [in] Audio channel handle */
	int					iStartPts,          /* [in] Start PTS value */
	int					iStopPts            /* [in] Stop PTS value */
	);
#endif

/***************************************************************************
Summary: Set PTS offset

Description:
	This function sets the value of PTS offset. i32PtsOffset should be passed in 45Khz ticks
	i.e.  No of Milli second * 45

Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetPTSOffset(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	int32_t				i32PtsOffset	/* [in] PTS Offset value */
	);


/***************************************************************************
Summary: ISR Version of BRAP_SetPTSOffset

Description:
	This function sets the value of PTS offset. i32PtsOffset should be passed in 45Khz ticks
	i.e.  No of Milli second * 45

Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetPTSOffset_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	int32_t				i32PtsOffset	/* [in] PTS Offset value */
	);

/***************************************************************************
Summary: ISR Version of BRAP_SetPTSOffset
                Set Playback Flag

Description:
	This function sets the Playback flag, i.e. it tells if its Live Decode or Playback.
	TRUE:  Playback
	FALSE: Live Decode

Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetPlaybackFlag_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool				bPlayback	/* [in] PTS Offset value */
	);

/***************************************************************************
Summary: Set Playback Flag

Description:
	This function sets the Playback flag, i.e. it tells if its Live Decode or Playback.
	TRUE:  Playback
	FALSE: Live Decode

Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetPlaybackFlag(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool				bPlayback	/* [in] PTS Offset value */
	);
	
/***************************************************************************
Summary: Get audio decode delay

Description:
	This function gets the delay time for audio decode including post processing for current
	input type in milliseconds. This function is not supported for 7440.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_GetAudioDecodeDelay(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	unsigned int			*uiAudDecDelay	/* [out] Audio Decode time */
	);

/***************************************************************************
Summary: Set TSM transition threshold

Description:
	This function sets transition threshold for time stamp management. If the
	PTS-STC difference for an audio frame is more than this threshold,
	then audio decoder switches to non-TSM mode and also TSM error is
	raised.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_EnableTSM
**************************************************************************/
BERR_Code
BRAP_SetTsmTransitionThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmTransitionThreshold
	);

/***************************************************************************
Summary: Set "STC valid" flag in firmware 

Description:
	This function sets "STC Valid" flag in firmware. In playback operation,
	application should call this function after programming STC. In playback
	firmware starts time stamp management only after this flag is set.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetStcValidFlag(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);

/***************************************************************************
Summary: Isr version of API  BRAP_SetStcValidFlag

Description:
	This function sets "STC Valid" flag in firmware. In playback operation,
	application should call this function after programming STC. In playback
	firmware starts time stamp management only after this flag is set. This
	function should be used from isr context.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_SetStcValidFlag_isr(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);

/***************************************************************************
Summary: Invalidate  "STC " flag in firmware 

Description:
	This function Invalidate "STC " flag in firmware. 
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_InvalidateStc(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);

/***************************************************************************
Summary: Isr version of API  BRAP_InvalidateStc

Description:
	This function Invalidate "STC " flag in firmware. 
	
Returns:
	BERR_SUCCESS - If successful

See Also:

**************************************************************************/
BERR_Code
BRAP_InvalidateStc_isr(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);



/***************************************************************************
Summary: Gets the number of decoded samples

Description:
	This function gets the number of samples decoded of the input compressed stream.
	This function is supported only for MPEG, AAC and AAC-HE audio types. It is
	supported only for 3563.
	
Returns:
	BERR_SUCCESS - If successful
	BERR_NOT_SUPPORTED - If audio type is other than MPEG, AAC, AAC-HE. OR if
						    chip is 7440.

See Also:
**************************************************************************/
BERR_Code
BRAP_GetNumDecodedSamples(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	unsigned int			*uiNumSamples	/* [out] Number of samples
										     decoded */
	);										     

#if ( (BRAP_7405_FAMILY == 1) || (BRAP_3548_FAMILY == 1) )
/***************************************************************************
Summary: Set TSM discard threshold. The Parameter i32DiscardThreshold should be 
programmed in ms (milli second)

Description:
	This function set discard threshold for time stamp management. If the
	PTS-STC difference for an audio frame is more than this threshold,
	then that frame is dropped.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_EnableTSM
**************************************************************************/
BERR_Code
BRAP_SetTSMDiscardThreshold(
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32DiscardThreshold/* [in] Discard threshold value  in ms*/
	);

/***************************************************************************
Summary: Set TSM upper threshold. The Parameter i32UpperThreshold should be 
programmed in ms (milli second)

Description:
	This function set Upper threshold for time stamp management. The TSM 
	module uses the upper threshold, lower threshold and discard threshold 
	to determine if the current frame should be decoded, waited, or 
	discarded
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_EnableTSM,BRAP_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_SetTSMUpperThreshold(
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32UpperThreshold/* [in] Upper threshold value  in ms*/
	);

/***************************************************************************
Summary: Set TSM Gross Adjustment threshold. The Parameter i32TsmGAThreshold should be 
programmed in ms (milli second)
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_EnableTSM
**************************************************************************/
BERR_Code
BRAP_SetTsmGAThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmGAThreshold   /* [in] The new GA threshold  in ms */
	);
#endif


/***************************************************************************
Summary: Set TSM sync limit threshold. The Parameter i32TsmSLThreshold should be 
programmed in ms (milli second)

Description:
        This function sets synclimit threshold for audio master mode. The value of 
        i32TsmSLThreshold is in milliseconds. When the value is set to zero audio master mode
        is disabled.
        
Returns:
        BERR_SUCCESS - If successful

See Also:
        BRAP_EnableTSM
**************************************************************************/
BERR_Code
BRAP_SetTsmSLThreshold ( 
        BRAP_ChannelHandle      hRapCh,                         /* [in] Audio channel handle */
        int32_t                         i32TsmSLThreshold   /* [in] The new SL threshold  in ms*/
        );

/***************************************************************************
Summary: Enable/Disable Adaptive System Time Management

Description:
        This function enables/disables Adaptive System Time Management(ASTM) for a given audio
        channel.
        
Returns:
        BERR_SUCCESS - If successful

See Also:
        BRAP_EnableASTM_isr
**************************************************************************/
BERR_Code
BRAP_EnableASTM(
        BRAP_ChannelHandle      hRapCh,                 /* [in] Audio channel handle */
        bool                    bEnableASTM             /* [in] true = enable ASTM,
                                                                false = disable ASTM  */
        );

/***************************************************************************
Summary: ISR version of BRAP_DSPCHN_EnableASTM

Description:
        This function enables/disables Adaptive System Time Management(ASTM) for a given audio
        channel in isr context.
        
Returns:
        BERR_SUCCESS - If successful

See Also:
        BRAP_DSPCHN_EnableASTM
**************************************************************************/
BERR_Code
BRAP_EnableASTM_isr(
        BRAP_ChannelHandle      hRapCh,                 /* [in] Audio channel handle */
        bool                    bEnableASTM             /* [in] true = enable ASTM,
                                                                false = disable ASTM  */
        );


/***************************************************************************
Summary: It returns the Version of decoder .

Format of display should be
ui8AlgoProfileId.ui8AlgoVersionId[3].ui8AlgoVersionId[2].ui8AlgoVersionId[1]. \
ui8AlgoVersionId[0].ui8FWVersionId

**************************************************************************/
BERR_Code   BRAP_GetDecoderVersionInfo(
    BRAP_ChannelHandle              hRapCh,              /* [in] Raptor Channel Handle */
    BRAP_DSPCHN_DecoderVersionInfo  *psDecoderVersionInfo /* [out] Decoder Version Info*/   
    );

/***************************************************************************
Summary: It returns the Version of Encoder .

Format of display should be
ui8AlgoProfileId.ui8AlgoVersionId[3].ui8AlgoVersionId[2].ui8AlgoVersionId[1]. \
ui8AlgoVersionId[0].ui8FWVersionId

**************************************************************************/
BERR_Code   BRAP_GetEncoderVersionInfo(
    BRAP_ChannelHandle              hRapCh,              /* [in] Raptor Channel Handle */
    BRAP_DSPCHN_EncoderVersionInfo  *psEncoderVersionInfo /* [out] Encoder Version Info*/   
    );

/***************************************************************************
Summary:
	Structure for decode channel configuration parameters

Description:
	This structure contains decode configuration parameters that can be
	changed on the fly.
	
See Also:
	BRAP_SetConfig
	BRAP_GetDefaultConfig
	BRAP_GetCurrentConfig
****************************************************************************/
typedef struct BRAP_DEC_ConfigParams
{
	BRAP_OutputMode	                eOutputMode;	
                                        /* Output mode for DSP */
	bool				            bOutputLfeOn;	
                                        /*	LFE channel selection flag:
                    						false = LFE channel is OFF
                    						true = LFE channel is present 
Note: note supported for MPEG and AAC */
	BRAP_DSPCHN_DualMonoMode        eDualMonoMode;	
                                        /* Dual mono mode, applies only when
									       stream has two independent mono
									       channel */
	BRAP_DSPCHN_AudioType           eType;	
                                        /* Audio type for which this structure
									       is getting initialized */
    bool                    bMonoToAll;	 /* TRUE: Downmixed mono data is routed 
                            to all speakers in the system.
                            Output mode must be 1_0 for this to take effect.*/			   
	union 
	{
            BRAP_DSPCHN_MpegConfigParams	 sMpegConfigParams;
            BRAP_DSPCHN_Ac3ConfigParams		    sAc3ConfigParams;
            BRAP_DSPCHN_Ac3PlusConfigParams		sAc3PlusConfigParams;
            BRAP_DSPCHN_AacConfigParams		    sAacConfigParams;
            BRAP_DSPCHN_AacSbrConfigParams		sAacSbrConfigParams;
            BRAP_DSPCHN_DtsConfigParams			sDtsConfigParams;
            BRAP_DSPCHN_DtsHdConfigParams		sDtsHdConfigParams;
            BRAP_DSPCHN_BdlpcmConfigParams		sBdlpcmConfigParams;		
            BRAP_DSPCHN_LpcmHdDvdConfigParams   sLpcmHdDvdConfigParams;	        
            BRAP_DSPCHN_LpcmDvdConfigParams	    sLpcmDvdConfigParams;
            BRAP_DSPCHN_Ac3PlusConfigParams		sAc3LosslessConfigParams;
            BRAP_DSPCHN_MlpConfigParams		    sMlpConfigParams;	
            BRAP_DSPCHN_DtslbrConfigParams		sDtslbrConfigParams;
            BRAP_DSPCHN_Ac3PlusConfigParams		sDdp7_1ConfigParams;
            BRAP_DSPCHN_MpegMcConfigParams		sMpegMcConfigParams;			
            BRAP_DSPCHN_WmaStdConfigParams      sWmaStdConfigParams;        
            BRAP_DSPCHN_WmaProConfigParams      sWmaProConfigParams;
            BRAP_DSPCHN_DtsBroadcastConfigParams    sDtsBroadcastConfigParams;
            BRAP_DSPCHN_DraConfigParam                  sDraConfigParams;
	} uConfigParams;				    /* Decode configuration parameters */
} BRAP_DEC_ConfigParams;

#ifdef __cplusplus
}
#endif

#endif /* BRAP_DSPCHN_H__ */
/* End of File */

