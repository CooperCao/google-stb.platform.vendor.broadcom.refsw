/***************************************************************************
*     Copyright (c) 2004-2011, Broadcom Corporation
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

#ifdef __cplusplus
extern "C" {
#endif

#define BRAP_DSPCHN_UNBOUNDED_START_PTS			(0)
#define BRAP_DSPCHN_UNBOUNDED_STOP_PTS				(0xFFFFFFFF)

/*	Max channel for 7.1 + concurrent stereo o/p */
#define BRAP_P_MAX_CHANNELS				( (uint32_t) 8 )

/***************************************************************************
Summary:
	This enumeration defines various decode modes.

***************************************************************************/
typedef enum BRAP_DSPCHN_DecodeMode
{
	BRAP_DSPCHN_DecodeMode_eDecode,		/* Decode mode */
	BRAP_DSPCHN_DecodeMode_ePassThru, 	/* Pass thru mode */
	BRAP_DSPCHN_DecodeMode_eSimulMode,	/* Simultaneous mode: It comprises 
										   of one decode and once pass-thru 
										   operation of the same input stream. */
   	BRAP_DSPCHN_DecodeMode_eInvalid	= 0xFF
} BRAP_DSPCHN_DecodeMode;

#ifndef BCHP_7411_VER /* For chips other than 7411 */
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
#endif

/***************************************************************************
Summary:
	This enumeration defines the supported audio input type.

***************************************************************************/
typedef enum BRAP_DSPCHN_AudioType
{
   	BRAP_DSPCHN_AudioType_eMpeg,			/* MPEG */
   	BRAP_DSPCHN_AudioType_eAac,				/* AAC */
   	BRAP_DSPCHN_AudioType_eAacSbr,			/* AAC_SBR */
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
   	BRAP_DSPCHN_AudioType_eWmaPro,			/* WMA Pro */
    BRAP_DSPCHN_AudioType_ePcmWav,   	
   	BRAP_DSPCHN_AudioType_eDra,				/* DRA */   	
#ifdef RAP_SRSTRUVOL_CERTIFICATION   	
   	BRAP_DSPCHN_AudioType_ePCM,				/* PCM Passthru */   	
#endif
   	BRAP_DSPCHN_AudioType_eMax,			/* Max value */
   	BRAP_DSPCHN_AudioType_eInvalid	= 0xFF
} BRAP_DSPCHN_AudioType;


#ifndef BCHP_7411_VER
/***************************************************************************
Summary:
	This enumeration defines the supported audio encoder types.

***************************************************************************/
typedef enum BRAP_DSPCHN_ENC_AudioType
{
	BRAP_DSPCHN_ENC_AudioType_eMpeg1Layer3 = 10,				/* MPEG1 Layer 3*/
#if ( BCHP_CHIP == 7440 )
	BRAP_DSPCHN_ENC_AudioType_eDTS	= 23	,					/* DTS */
#endif
   	BRAP_DSPCHN_ENC_AudioType_eMax,			/* Max value */
   	BRAP_DSPCHN_ENC_AudioType_eInvalid	= 0xFF
} BRAP_DSPCHN_ENC_AudioType;
#endif

/***************************************************************************
Summary:
	This enumeration defines transport formats for AAC-HE.
***************************************************************************/
typedef enum BRAP_DSPCHN_AacXptFormat
{
	BRAP_DSPCHN_AacXptFormat_eAdts = 0,
	BRAP_DSPCHN_AacXptFormat_eLoas = 1
} BRAP_DSPCHN_AacXptFormat;

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

/***************************************************************************
Summary:
	This structure contains AC3 decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3ConfigParams
{
	uint16_t			ui16DynRngScaleHi; /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
	uint16_t			ui16DynRngScaleLo; /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
	uint16_t 			ui16PcmScaleFactor; /*	PCM scale factor. Input value range = 0 to 100%
								100% corresponds to value of unity.Any value 
								above 100% is considered as unity. Each decoded
								PCM sample gets scaled by this factor */
	BRAP_DSPCHN_Ac3CompMode		eCompMode;	/*	Dynamic range compression mode */
	BRAP_DSPCHN_Ac3DualMonoMode	eDualMono;	/*	(Warning: Deprecated field. Please use eDualMonoMode
								in structure BRAP_DEC_ConfigParams.)
								Dual mono reproduction mode. Effective only
								when stream acmod = 000. In any other case
								setting of this bit has no effect. 
								This field is ignored if the structure 
								BRAP_DSPCHN_Ac3ConfigParams is field of
								struct BRAP_DEC_ConfigParams. In that case, 
								eDualMonoMode field of BRAP_DEC_ConfigParams
								gets applied. */
	bool				bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */
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
	bool				bDialNormEnable;	/*	true -Enble Dialog normalization
											false - Disable Dialog normalization*/
									
} BRAP_DSPCHN_Ac3ConfigParams;

/***************************************************************************
Summary:
	This enumeration defines downmix type for AAC decoder.
***************************************************************************/
typedef enum BRAP_DSPCHN_AAC_DownmixType
{
	BRAP_DSPCHN_AAC_DownmixType_eBRCM = 0,
	BRAP_DSPCHN_AAC_DownmixType_eARIB = 1
} BRAP_DSPCHN_AAC_DownmixType;

/***************************************************************************
Summary:
	This structure contains AAC decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AacConfigParams
{
	uint32_t ui32DrcGainControlCompress;	/* Dynamic range compression 
											cut scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint32_t ui32DrcGainControlBoost;		/* Dynamic range compression 
											boost scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint32_t ui32DrcTargetLevel;			/* Dynamic range compression 
											target level.
											Input value range = 0 to 127. Any value
											above 127 is considered as 127 */
	uint32_t	ui32Arib_matrix_mixdown_index;	/*ARIB matrix mixdown index which can 
											take integer values from 0 to 3*/ 										
                                            
	BRAP_DSPCHN_AAC_DownmixType	eDownmixType;	/*0 -> BRCM downmix
												1 -> ARIB downmix */
													
	uint32_t ui32bOutputLfeOn;			/*  Controls the LFE channel output
											1 = Output LFE channel
											0 = Do not output LFE channel */
	uint32_t ui32ExtDownmixEnable;		/* Controls the external downmix coeff usage
											1 = Enable
											0 = Disable */
	uint32_t ui32ExtDnMixCoeffs[6][6];		/*	User defined downmix (or karaoke) coefficients. 
											Valid only when ui16ExtDownmixEnable = 1 */  

	uint32_t ui32PcmBoost6dB;			/* Default = 0, 0 -> FALSE, 1 -> TRUE */												
	bool bPcmBoostMinus4p75dB;			/* Default = FALSE */											
	
} BRAP_DSPCHN_AacConfigParams;

/***************************************************************************
Summary:
	This enumeration defines downmix type for AAC SBR decoder.
***************************************************************************/
typedef enum BRAP_DSPCHN_AACSbr_DownmixType
{
	BRAP_DSPCHN_AACSbr_DownmixType_eBRCM = 0,
	BRAP_DSPCHN_AACSbr_DownmixType_eARIB = 1
} BRAP_DSPCHN_AACSbr_DownmixType;


/***************************************************************************
Summary:
	This structure contains AACSBR decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AacSbrConfigParams
{
	uint32_t ui32DrcGainControlCompress;	/* Dynamic range compression 
											cut scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint32_t ui32DrcGainControlBoost;		/* Dynamic range compression 
											boost scale factor.
											Input value range = 0% to 100%. 100%
											corresponds to value of unity. Any value
											above 100% is considered as unity */

	uint32_t ui32DrcTargetLevel;			/* Dynamic range compression 
											target level.
											Input value range = 0 to 127. Any value
											above 127 is considered as 127 */
	uint32_t	ui32Arib_matrix_mixdown_index;	/*ARIB matrix mixdown index which can 
											take integer values from 0 to 3*/ 										
                                            
	BRAP_DSPCHN_AACSbr_DownmixType	eDownmixType;	/*0 -> BRCM downmix
												1 -> ARIB downmix */
													
	uint32_t ui32bOutputLfeOn;			/*  Controls the LFE channel output
											1 = Output LFE channel
											0 = Do not output LFE channel */
	uint32_t ui32ExtDownmixEnable;		/* Controls the external downmix coeff usage
											1 = Enable
											0 = Disable */
	uint32_t ui32ExtDnMixCoeffs[6][6];		/*	User defined downmix (or karaoke) coefficients. 
											Valid only when ui16ExtDownmixEnable = 1 */ 

	uint32_t ui32PcmBoost6dB;			/* Default = 0, 0 -> FALSE, 1 -> TRUE */																								
	bool bPcmBoostMinus4p75dB;			/* Default = FALSE */											
												
} BRAP_DSPCHN_AacSbrConfigParams;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3Plus compression mode  
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3PlusCompMode
{
	BRAP_DSPCHN_Ac3PlusCompMode_eCustomA,
	BRAP_DSPCHN_Ac3PlusCompMode_eCustomD,
	BRAP_DSPCHN_Ac3PlusCompMode_eLine,
	BRAP_DSPCHN_Ac3PlusCompMode_eRf,
    BRAP_DSPCHN_AC3PlusCompMode_eOff	
} BRAP_DSPCHN_Ac3PlusCompMode;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3Plus karaoke capable mode
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect
{
	BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eNoVocal,
	BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eLtVocal,
	BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eRtVocal,
	BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eBothVocal	
} BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect; 

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	AC3Plus stereo mode. This eum is effective only when stream is
	Annex-D compliant and 
	BRAP_DEC_AudioParams.eOutputMode = BRAP_OutputMode_eStereo. 	
***************************************************************************/
typedef enum BRAP_DSPCHN_Ac3PlusStereoMode
{
	BRAP_DSPCHN_Ac3PlusStereoMode_eAuto,
	BRAP_DSPCHN_Ac3PlusStereoMode_eLtRt,
	BRAP_DSPCHN_Ac3PlusStereoMode_eLoRo
} BRAP_DSPCHN_Ac3PlusStereoMode;

/***************************************************************************
Summary:
	This enumeration defines downmix type for AC3 Plus decoder.
***************************************************************************/
typedef struct BRAP_DSPCHN_Ac3PlusConfigParams
{
	uint16_t			ui16DynRngScaleHi; /* 	Dynamic range compression cut scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
	uint16_t			ui16DynRngScaleLo; /*	Dynamic range compression boost scale factor
								Input value range = 0% to 100%. 100%
								corresponds to value of unity. Any value
								above 100% is considered as unity */
	uint16_t 			ui16PcmScaleFactor; /*	PCM scale factor. Input value range = 0 to 100%
								100% corresponds to value of unity.Any value 
								above 100% is considered as unity. Each decoded
								PCM sample gets scaled by this factor */
	BRAP_DSPCHN_Ac3PlusCompMode		eCompMode;	/*	Dynamic range compression mode */
	bool				bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */
	BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect	eKaraokeVocalSelect; /*	Karaoke capable reproduction mode */
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
	BRAP_DSPCHN_Ac3PlusStereoMode	eStereoMode; 		/*	Stereo output mode. It is effective
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
	bool				bDialNormEnable;	/*	true -Enble Dialog normalization
											false - Disable Dialog normalization*/									
} BRAP_DSPCHN_Ac3PlusConfigParams;

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
	bool			bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 

	bool			bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		i32UserDownmixCoeff[6][6];
								/* TODO: Put proper comments */
} BRAP_DSPCHN_DtsConfigParams;

#if (BRAP_DTS_SUPPORTED == 1)		
/***************************************************************************
Summary:
	This structure contains DTS decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DtsCoreConfigParams
{
    uint32_t        i32UserDRC_flag;
    uint32_t        i32DynScaleHigh;
                            /* dynamic range compression cut scale factor   */
    uint32_t        i32DynScaleLow;
                            /* dynamic range compression boost scale factor */
    uint32_t        i32NumOutputport;   
    uint32_t        i32UserAMODE;   

	bool			bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
    uint32_t i32DualMode;
    uint32_t i32Use_LFE_in_DMX; 
    uint32_t i32OutPair; 
    uint32_t i32UsePatternInDummy; 
    uint32_t i32BitPatternInDummy;
    
} BRAP_DSPCHN_DtsCoreConfigParams;
#endif

/***************************************************************************
Summary:
	This structure contains DTS-HD decoder configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DtshdConfigParams
{
	bool			bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is present */

	BRAP_DSPCHN_DtsStereoMode	eStereoMode;
								/*	Stereo output mode. It is effective
								only when stream is annex-D compliant and
								BRAP_DEC_AudioParams.eOutputMode = 
								BRAP_OutputMode_eStereo. */ 

	bool			bUseUserDownmixCoeff;
								/*	User defined downmix flag. This is set to true,
								when user wants to set the appropriate downmix
								coefficients for each channel */
	int32_t		i32UserDownmixCoeff[6][6];
								/* TODO: Put proper comments */
} BRAP_DSPCHN_DtshdConfigParams;

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
	bool				bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is ON */
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
	int32_t		i32UserDownmixCoeff[8][8];
								/* TODO: Put proper comments */
	bool				bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is ON */
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
	bool				bOutputLfeOn;	/*	LFE channel selection flag
								false = LFE channel is OFF
								true = LFE channel is ON */
} BRAP_DSPCHN_LpcmDvdConfigParams; 

/***************************************************************************
Summary:
	This enumeration selects MLP profile to decode. MLP decoder requires more
	bandwidth to decode higher profiles.
***************************************************************************/
typedef enum BRAP_DSPCHN_MlpProfileSelect
{
	BRAP_DSPCHN_MlpProfileSelect_eTwoChannel = 0,
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
	bool								bDialogueEnable; /* MLP dialogue enable */
	int32_t							i32DialogueLevel;	/* MLP diaglogue level.
														    Applicable only if
														    bDialogueEnable = true.
														    Permissible values are
														    0 to -63. */
	bool								bOutputLfeOn;	/*	LFE channel selection flag
														false = LFE channel is OFF
														true = LFE channel is ON */
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
	BRAP_DSPCHN_WmaProStereoMode_eLoRo
} BRAP_DSPCHN_WmaProStereoMode;


/***************************************************************************
Summary:
	This structure contains WMA Pro configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_WmaProConfigParams
{

	bool								bDrcFlag;	/* DRC On/Off */
	uint32_t							ui32DrcCut;		 /* dynamic range compression cut scale factor  */
	uint32_t							ui32DrcBoost;		 /* dynamic range compression boost scale factor  */
	bool								bLfeOn;		/* LFE On/Off */
	BRAP_DSPCHN_WmaProStereoMode	eStereoMode; /* Stero Mode settings */
} BRAP_DSPCHN_WmaProConfigParams;

/***************************************************************************
Summary:
	This enumeration defines configuration parameter values for
	WMA Std stereo mode. 
***************************************************************************/
typedef enum BRAP_DSPCHN_WmaStdStereoMode
{
	BRAP_DSPCHN_WmaStdStereoMode_eAuto,
	BRAP_DSPCHN_WmaStdStereoMode_eLtRt,
	BRAP_DSPCHN_WmaStdStereoMode_eLoRo
} BRAP_DSPCHN_WmaProStdMode;


/***************************************************************************
Summary:
	This structure contains WMA Std configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_WmaStdConfigParams
{
	BRAP_DSPCHN_WmaProStdMode		eStereoMode;	 
} BRAP_DSPCHN_WmaStdConfigParams;


/***************************************************************************
Summary:
	This structure contains DRA configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DraConfigParams
{
    uint32_t ui32EnableDownmix; /* Default = 1 	Range : 0,1 (0-> Downmix disabled; 1-> Downmix enabled) 
                            	This Flag is used to turn ON/OFF pcm domain downmixing. */

	uint32_t ui32nOutmode;	/* Default = 1	Range 0, 1, 2
                        	0 - Mono Downmixed output
                        	1 - LoRo downmixed output
                        	2 - LtRt downmixed output
                        	The decoded output is downmixed to a configuration based on the ui32nOutmode value,
                        	if  ui32EnableDownmix 	is set to 1 otherwise the ui32nOutmode value is ignored .*/

    uint32_t ui32nBitPCM;	/* Default = 24	Range - 24, 16
                        	This value gives the output bit width of the decoded PCM samples.*/

	uint32_t ui32nOutPair;	/*Default =0;	Range - 0, 1, 2
                        	This field decides which channel pair has to be sent on the stereo dac. The convention is as follows -
                        	0 - FrontLeft / FrontRight pair
                        	1 - RearLeft / Rear Right pair
                        	2 - Center / LFE channel pair
                        	The ui32nOutPair value is only relevant when downmixing is disabled, 
                        	i.e ui32EnableDownmix is set to 0, 	otherwise the ui32nOutPair value is ignored.*/
                        	
    uint32_t ui32nCertDummyPatternEnable;	/*Default=0; 	Range - 0,1
                                        	This define has to be made 1 for allowing a fixed dummy pattern to be o/p on dummy calls. 
                                        	This has to be set to 	1 for initial testing*/

	uint32_t ui32nDummyPattern;	/*Default - 0xabcdabcd
                                This value is relevant when ui32nCertDummyPatternEnable is set to 1. 
                                The decoder outputs this pattern on the	ring buffers in case of a dummy call */                                                                                      

} BRAP_DSPCHN_DraConfigParams;

/***************************************************************************
Summary:
	This enumeration defines Fade Configuration type values.
***************************************************************************/
typedef enum BRAP_FadeConfigType
{
    BRAP_FadeConfigType_ePrimaryChOnly = 0, 
    BRAP_FadeConfigType_eFadedChOnly,
    BRAP_FadeConfigType_ePrimaryAndFadedCh,
    BRAP_FadeConfigType_eLast,
    BRAP_FadeConfigType_eInvalid
    
} BRAP_FadeConfigType;


/***************************************************************************
Summary:
	This structure contains Fade Control configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AD_FadeCtrlConfigParams
{
    BRAP_FadeConfigType     eFadeConfig;
    uint32_t    ui32PanFadeInterfaceAddr;
} BRAP_DSPCHN_ADFadeCtrlConfigParams;



/***************************************************************************
Summary:
	This enumeration defines AD Channel values.
***************************************************************************/
typedef enum BRAP_DSPCHN_EADChannelConfig
{
    BRAP_PanConfigType_eADChannelOff = 0,
    BRAP_PanConfigType_eADChannelOn,
    BRAP_PanConfigType_eADChannelLast
    
}BRAP_DSPCHN_EADChannelConfig;


/***************************************************************************
Summary:
	This structure contains Pan Control configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_AD_PanCtrlConfigParams
{

    uint32_t    ui32UserVolumeLevel;
    BRAP_DSPCHN_EADChannelConfig    eADChannelConfig;
    uint32_t    ui32PanFadeInterfaceAddr;
    
} BRAP_DSPCHN_ADPanCtrlConfigParams; 

/***************************************************************************
Summary:
	This enumeration contains DD Bass Management (DDBM) modes.
***************************************************************************/
typedef enum BRAP_DSPCHN_DdbmMode 
{
	BRAP_DSPCHN_DdbmMode_eConfig1,
	BRAP_DSPCHN_DdbmMode_eConfig2
} BRAP_DSPCHN_DdbmMode;

/***************************************************************************
Summary:
	This structure contains  DD Bass Management (DDBM) configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DdbmConfigParams
{
	BRAP_DSPCHN_DdbmMode		eConfigMode;
} BRAP_DSPCHN_DdbmConfigParams;

/***************************************************************************
Summary:
	This structure contains  PCMWav configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_PcmWavConfigParams
{
	
	uint32_t	ui32NumOutputPorts;
	uint32_t	ui32OutMode;					 /* OutMode */
	uint32_t	ui32OutputChannelMatrix[BRAP_P_MAX_CHANNELS];

}BRAP_DSPCHN_PcmWavConfigParams;


/***************************************************************************
Summary:
	This structure contains  SRS Volume IQ configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_SRSTruVolConfigParams
{
    
    /*General Settings*/

    bool		bTrueVolume_enable;				/*Top level disable, no processing will happen at all when 0*/
												/*Deafault : 1, range [0,1]*/
												/*1 -> processing enable, 0-> total disable of the processing*/

    int32_t		i32Nchans;						/* Number of input/output channels     */
												/*default val 2*/
    int32_t		i32BlockSize;					/*Default: 256, range [256, 512, 768, 1024]*/

    bool		bMEnable;						/* default:	1;	1 -> Process, 0	->	Bypass */

	int32_t		i32InputGain;					/* Input gain */
                                                /*Expected PI Programming : Default:25 Range: 0 - 800. Step should of 25 i.e. 0,25,50 ...*/    
                                                /*SRS TVO Spec Range: 0:32, Step Size supported 0,1,2...32*/
												/*Value required by FW default: 0x00200000 rnage[0, 0x03ffffff]*/

	int32_t		i32OutputGain;					/* Output gain */
                                                /*Expected PI Programming : Default:25 Range: 0 - 100*/                                                                                                                                                                        
                                                /*SRS TVO Spec Range: 0:4*/
												/*Value required by FW  default: 0x00200000  rnage[0, 0x007fffff]*/

	int32_t		i32BypassGain;					/* Bypass gain */
												/*Expected PI Programming : Default:25 Range: 0 - 100*/                                                                                                                                                                      
												/*SRS TVO Spec Range: 0:4*/  
												/*Value required by FW  default: 0x007fffff rnage[0, 0x007fffff]*/

	int32_t		i32ReferenceLevel;				/* Reference Level */
												/*Expected PI Programming : Default:50 Range: 0 - 100*/                                                                                                                                                                        
												/*SRS TVO Spec Range: 0:1*/                    
												/*default: 0x00400000 rnage[0x00000109, 0x007fffff]*/

	int32_t		i32LimiterControl;				/* Limiter Control */
												/*default: 0x007fffff rnage[]*/

	int32_t		i32Mode;						/*default: 1 rnage[0,1] 1-> kSrsLight, 0-> kSrsNormal*/

	int32_t		i32Size;						/*default: 0 rnage[0,1,2,3,4, 5]*/
														/*	0	->	TruVol_kSrs20Hz,
															1	->	kSrs40Hz,
															2	->	kSrs110Hz,
															3	->	kSrs200Hz,
															4	->	kSrs315Hz,
															5	->	kSrs410Hz*/
	int32_t		i32MaxGain;						/* Max Gain Control */
												/*SRS TVO Spec Range: 0.25:256*/
												/*Expected PI Programming : Default: 16 Range: 0 - 256*/                                                          
												/*Value required by FW  default: 0x007fffff rnage[0x00200000, 0x007fffff]*/
	int32_t 	i32NoiseManager; 				/* default: 1; 1 -> Noise manager enable, 0 -> disable */
	int32_t 	i32NoiseManagerThreshold;
												/*Expected PI Programming : Default: 10 Range: 0 - 100*/     
												/*SRS TVO Spec Range: 0:1*/
												/* default: 0x000ccccd, range [0x0000a3d7, 0x007fffff] */
																								
} BRAP_DSPCHN_SRSTruVolConfigParams;
/***************************************************************************
Summary:
	This structure contains  Dolby Volume configuration parameters
***************************************************************************/
typedef struct BRAP_DSPCHN_DolbyVolumeConfigParams
{
    
    /*General Settings*/

    bool		bVolume_iq_enable;              /*Range of values -  0 or 1 */
    											/*0 - dolby volume disabled - acts as pass trough node*/
    											/*1 - dolby volume enabled   */ 
    											/*Default value    : 1 ( dolby volume enabled)  */

    int32_t		i32Block_size;						/* Size of processing block in samples */
    											/*Range of values - 256, 512*/
    											/*Default value    : 512*/
    int32_t		i32Nbands;							/* Number of critical bands to use      */
    											/*possible values 20 or 40, default 40*/


    int32_t		i32Nchans;							/* Number of input/output channels     */
    											/*default val 2*/

    /* System Settings*/
    int32_t		i32Input_reference_level;			/* Input reference level in dBSPL                         */
    											/*Range of values(in dB )- (0 to 100 in 8.24 format)*/
    											/*Default value    : 0x55000000 (85 dB)*/

    int32_t		i32Output_reference_level;			/* Output reference level in dBSPL                        */
    											/*Range of values(in dB) - (0 to 100 in 8.24 format)*/
    											/*Default value    : 0x55000000 (85 dB)*/

    int32_t		i32Calibration;					/* Calibration offset in dB                               */
    											/*Range of values(in dB)- (-100 to 30 in 8.24 format)*/
    											/*Default value    : 0x00000000 (0 dB)*/
    bool		bVlm_enable;						/* volume modeler on/off                                 */
    											/*possible value 0 and 1, default 1*/
    int32_t		i32Reset_now_flag;					/* User-input forced reset flag                           */
    											/*range [0,1], defaultr 0*/


    /*Volume Modeler Settings*/
    int32_t		i32Digital_volume_level;			/* Volume level gain in dB -- applied by dolby volume     */
    											/*Range of values(in dB) - (-100 to 30 in 8.24 format)*/
    											/*Default value    : 0x00000000 (0 dB)*/

    int32_t		i32Analog_volume_level;			/* Volume level gain in dB -- applied after dolby volume  */
    											/*Range of values(in dB) - (-100 to 30 in 8.24 format)*/
    											/*Default value    :  0x00000000 (0 dB)*/
    
    /*Volume Leveler Settings */
    int32_t		i32Lvl_amount;						/*Range of values - [0 to 10]*/
    											/*Default value    : 0*/

    bool		bLvl_disable;					/*Range of values -  0=FALSE , 1=TRUE*/
    											/*Default value    : 0 (FALSE - volume leveler enabled)*/

    bool		bEnable_midside_proc;			/* Enable midside processing */
    											/*range [0,1], default ->0-> disable*/
	bool		bHalfmode_flag;					/*Flag to operate Dolby Volume in half mode*/
												/*range [0,1], default ->0-> disable*/
    bool        bLimiter_enable;                /*Enable Limter*/ 
                                                /*Default - > Disabled 
                                                  When DV is enabled limiter_enable = TRUE
                                                  When DV is disabled limiter_enable = FALSE*/                                                
} BRAP_DSPCHN_DolbyVolumeConfigParams;
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

#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
/***************************************************************************
Summary:
	This structure contains PP algo settings.

***************************************************************************/
typedef struct BRAP_PP_Settings
{
#if (BRAP_AD_SUPPORTED == 1)
	BRAP_DSPCHN_ADFadeCtrlConfigParams		    sADFadeCtrlConfigParams;
	BRAP_DSPCHN_ADPanCtrlConfigParams           sADPanCtrlConfigParams;
#endif	
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
	BRAP_DSPCHN_DolbyVolumeConfigParams		    sDolbyVolumeConfigParams;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
	BRAP_DSPCHN_SRSTruVolConfigParams		    sSRSTruVolConfigParams;
#endif
} BRAP_PP_Settings;
#endif

/***************************************************************************
Summary:
	This structure contains DSPCHN channel settings.

***************************************************************************/
typedef struct BRAP_DSPCHN_Settings
{
	BRAP_DSPCHN_DualMonoMode	eDualMonoMode;
}BRAP_DSPCHN_Settings;


/***************************************************************************
Summary:
	This enumeration defines different playback rates for the DSP
***************************************************************************/
typedef enum BRAP_DSPCHN_PlayBackRate{
    BRAP_DSPCHN_PlayBackRate_e80, /* Play out the decoded data at 80% of the 
                                  original rate ie slower */
    BRAP_DSPCHN_PlayBackRate_e100, /* Play out the decoded data at 80% of the 
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
    BRAP_DSPCHN_PlayBackRate ePBRate;       /* Rate at which the decoded audio 
                                            should be played out. Implemented 
                                            only for 7401 at present */                                                    
	BRAP_DSPCHN_AacXptFormat	eAacXptFormat;	/* AAC, AACSBR transport
												   format. Only valid for
												   eType = eAac and eAacSbr */
	BRAP_DSPCHN_DvdType		eDvdType;	/* This field is used only if
												audio is DVD material. */
	int32_t		i32AVOffset;	/* Audio-Video delay path difference*/
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
                                    false = Disable PTS specific decode.*/
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
    bool    bProgramPLL;                /* TRUE : Firmware will programm PLL during sample rate change (default)
                                           FALSE :Firmware will not programm PLL during sample rate change*/											   
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
	BRAP_DSPCHN_PtsType 	ePtsType;   /* The PTS type tag */ 
} BRAP_DSPCHN_PtsInfo;


/***************************************************************************
Summary:
	This enumeration defines MPEG audio layers.

***************************************************************************/
typedef enum BRAP_DSPCHN_MpegAudioLayer
{
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer3 = 1,		/* MPEG Layer III */
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer2,    	/* MPEG Layer II */	
	BRAP_DSPCHN_MpegAudioLayer_eMpegLayer1	    	/* MPEG Layer I */
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
	BRAP_DSPCHN_MpegChannelMode_eSingleChannel		/* Single Channel */
}BRAP_DSPCHN_MpegChannelMode;

/***************************************************************************
Summary:
	This enumeration defines MPEG deemphasis mode from MPEG header
***************************************************************************/
typedef enum BRAP_DSPCHN_MpegEmphasisMode
{
	BRAP_DSPCHN_MpegEmphasisMode_None = 0,		/* No deemphasis */
	BRAP_DSPCHN_MpegEmphasisMode_Emph50_15ms,	/* Emph_50_15ms */
	BRAP_DSPCHN_MpegEmphasisMode_Reserved,		/* Reserved */
	BRAP_DSPCHN_MpegEmphasisMode_CCITT_J17		/* CCITT_J17 */
}BRAP_DSPCHN_MpegEmphasisMode;


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

#if(BCHP_CHIP == 7400)
/***************************************************************************
Summary:
	This enumeration defines AC3 ACMOD values
**************************************************************************/
typedef enum BRAP_DSPCHN_Ac3PlusAcmod
{
	BRAP_DSPCHN_Ac3PlusAcmod_eReserved,
	BRAP_DSPCHN_Ac3PlusAcmod_eC,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_I,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_I,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_I_r,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_I_r,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_Cvh,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Ts,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Ts,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Cvh,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_Lc_Rc,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Lw_Rw,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Lvh_Rvh,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Lsd_Rsd,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Lrs_Rrs,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Lc_Rc,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Lw_Rw,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Lvh_Rvh,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Lsd_Rsd,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Lrs_Rrs,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Ts_Cvh,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Cs,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Cs,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_R_l_r_Cs_Ts,	
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Cs_Cvh,
	BRAP_DSPCHN_Ac3PlusAcmod_eL_C_R_l_r_Cs_Ts
} BRAP_DSPCHN_Ac3PlusAcmod;
#else
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
#endif
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
#if(BCHP_CHIP == 7400)
	BRAP_DSPCHN_Ac3PlusAcmod	eAcmod;			/* Audio coding mode which describes 
											   the number of input channels */ 
#else
	BRAP_DSPCHN_Ac3Acmod        eAcmod;			/* Audio coding mode which describes 
											   the number of input channels */
#endif
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
	BRAP_DSPCHN_AacAcmod_eDualMono,
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
	BRAP_DSPCHN_DtsExtAudId_X96k
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
typedef struct BRAP_DSPCHN_DtshdInfo
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
} BRAP_DSPCHN_DtshdInfo;

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
	BRAP_DSPCHN_LpcmDvdChanAssign_L_C_R_Ls_Cs1_Cs2_Rs_LFE_3_4
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
	BRAP_DSPCHN_LpcmDvdChanAssign 	eChnAssignment;
								/* This field specifies the assignment.*/
	BRAP_DSPCHN_LpcmDvdBitsPerSample		eBitsPerSample;
								/* This field specifies the sampling resolution of the
								audio samples for all channels in the LPCM audio stream */
	uint32_t					ui32FrameSize;
								/*This field contains the total byte size of the current frame */
	bool						bStartFlag;	/* This field indicates the track-start or index-point */
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
	BRAP_DSPCHN_WmaStdAcmod_eTwoCh	/* Two Channels */
} BRAP_DSPCHN_WmaStdAcmod;

/***************************************************************************
Summary:
	This structure contains WMA standard  Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_WmaStdInfo
{
	uint32_t						ui32BitRate;	/* Bit Rate Value */
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
	uint32_t					ui32BitRate;	/* Bit Rate Value */
	bool						bLfeOn;			/* LFEON indicates a low-frequency 
											  	 channel effect exists when it is 
											 	  1. Otherwise, the low frequency 
											 	  channel	does not exist. */ 
	BRAP_DSPCHN_WmaProAcmod		eAcmod;			/* WMA ACMOD value */
	BRAP_DSPCHN_WmaProVersion	eVersion;		/* WMA version number */

} BRAP_DSPCHN_WmaProInfo;



/***************************************************************************
Summary:
	This enumaration defines the channel assignment for 6 and 8 ch decoder
	when presentation contains three or more encoded channels and if 
	surround ( Ls/Rs ) channels are present.

****************************************************************************/
typedef enum BRAP_DSPCHN_FbaChApplicationLsRsChRecording
{
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotIndicated = 0,		
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotDolbySurroundExEncoded,
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eDolbySurroundExEncoded,
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eReserved

}BRAP_DSPCHN_FbaChApplicationLsRsChRecording;

/***************************************************************************
Summary:
	This enumaration defines the channel assignment for 2/6/8 ch decoders
	when presentation contains exactly two channels.

****************************************************************************/
typedef enum BRAP_DSPCHN_FbaChApplication2ChRecording
{

	BRAP_DSPCHN_FbaChApplication2ChRecording_eStereo = 0,		
	BRAP_DSPCHN_FbaChApplication2ChRecording_eLtRt,
	BRAP_DSPCHN_FbaChApplication2ChRecording_eLbinRbin,
	BRAP_DSPCHN_FbaChApplication2ChRecording_eReserved
}BRAP_DSPCHN_FbaChApplication2ChRecording;

/***************************************************************************
Summary:
	This enumaration defines the multi channels types for 8ch/6ch decoders

****************************************************************************/
typedef enum BRAP_DSPCHN_FbaMultiChannelsType
{
	BRAP_DSPCHN_FbaMultiChannelsType_eStdLoudspeakerlayout= 0,		
	BRAP_DSPCHN_FbaMultiChannelsType_eNonStdLoudspeakerlayout
}BRAP_DSPCHN_FbaMultiChannelsType;

/***************************************************************************
Summary:
	This structure contain information about Format Info of FBA Stream

****************************************************************************/
typedef struct BRAP_DSPCHN_FbaFormatInfo
{
	bool bDecodeChannelAssignment8Ch[13];	/* The presence or absence of channels 
											is flagged in this bool array.	
											Some loudspeakers are added singly and 
											others as pairs. The field is 
											interpreted as follows:
											Array-Index	Exists				Number of Channels
											0	Main (Left/Right)			2
											1	Centre						1
											2	LFE							1
											3	Surrounds (Ls/Rs )			2
											4	Front Highs (Lvh/Rvh)		2
											5	Inner Fronts (Lc/Rc)		2
											6	Rears (Lrs/Rrs)				2
											7	Centre Surround (Cs)		1
											8	Top(Ts)						1
											9	Surround Directs (Lsd/Rsd)	2
											10	Front Wide (Lw/Rw)			2
											11	Centre Front High (Cvh)		1
											12	LFE2						1
											*/

	

	bool bDecodeChannelAssignment6Ch[5];	/* The presence or absence of channels is
											flagged in this bool array.	Some loudspeakers
											are added singly and others as pairs. The field is
											interpreted as follows:
											Array-Index	Exists				Number of Channels
											0	Main (Left/Right)			2
											1	Centre						1
											2	LFE							1
											3	Surrounds (Ls/Rs )			2
											4	Highs (Lvh/Rvh)				2
											*/

									
	
	BRAP_DSPCHN_FbaChApplication2ChRecording eDecodeChannelModifier8Ch;
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording eDecodeChannelModifier8ChSurround;
	BRAP_DSPCHN_FbaChApplication2ChRecording eDecodeChannelModifier6Ch;
	BRAP_DSPCHN_FbaChApplicationLsRsChRecording eDecodeChannelModifier6ChSurround;
	BRAP_DSPCHN_FbaChApplication2ChRecording eDecodeChannelModifier2Ch;

	BRAP_DSPCHN_FbaMultiChannelsType eMultiChannelType8Ch;
	BRAP_DSPCHN_FbaMultiChannelsType eMultiChannelType6Ch;
	
} BRAP_DSPCHN_FbaFormatInfo;


/***************************************************************************
Summary:
	This enumeration defines MLP quantization word length

****************************************************************************/
typedef enum BRAP_DSPCHN_MlpQuantizationWordLength 
{
	BRAP_DSPCHN_MlpQuantizationWordLength_e16Bits= 0,		
	BRAP_DSPCHN_MlpQuantizationWordLength_e20Bits,		
	BRAP_DSPCHN_MlpQuantizationWordLength_e24Bits,		
	BRAP_DSPCHN_MlpQuantizationWordLength_eNotSpecified,	
	BRAP_DSPCHN_MlpQuantizationWordLength_eReserved
}BRAP_DSPCHN_MlpQuantizationWordLength ;

/***************************************************************************
Summary:
	This structure contain information about Format Info of FBB Stream

****************************************************************************/
typedef struct BRAP_DSPCHN_FbbFormatInfo
{
	uint32_t ui32ChannelAssignment;
	uint32_t ui32MultiChannelType;
	BAVC_AudioSamplingRate eAudioSamplingFrequency1;
	BAVC_AudioSamplingRate eAudioSamplingFrequency2;
	BRAP_DSPCHN_MlpQuantizationWordLength eQuantizationWordLength1;
	BRAP_DSPCHN_MlpQuantizationWordLength eQuantizationWordLength2;
	
} BRAP_DSPCHN_FbbFormatInfo;

/***************************************************************************
Summary:
	This structure contain Channel Meaning properties of 2ch/6ch/8ch of FBA 
	stream.

****************************************************************************/

typedef struct BRAP_DSPCHN_FbaChannelMeaningProperties
{
	bool bControlEnabled;	/*If this bit is clear,decoder does not apply 
							gain adjustment (Dynamic range compression) 
							unless specifically requested to do so by the 
							user.*/

	uint32_t ui32DialogueNormalization;	/*This field indicates the average 
										dialogue level below digital 
										full scale in dB.*/

	uint32_t ui32MixLevel;	/* This field indicates the absolute acoustic
							sound	pressure level of a presentation during
							the final audio mixing session or at the 
							original recording location in dB. */

	uint32_t ui32SourceFormat;	/* This field describe hierarchical source
								information. Please	 refer to 
								MLP Technical Reference Version 1.0 ( Annex B ) 
								for more information. This field doesn't 
								apply for 2-ch decoders */
	
}BRAP_DSPCHN_FbaChannelMeaningProperties;

/***************************************************************************
Summary:
	This structure contain information about Channel Meaning of FBA Stream

****************************************************************************/
typedef struct BRAP_DSPCHN_FbaChannelMeaning
{
	bool bExtraChannelMeaningPresent;
	BRAP_DSPCHN_FbaChannelMeaningProperties s2ChProperties;
	BRAP_DSPCHN_FbaChannelMeaningProperties s6ChProperties;
	BRAP_DSPCHN_FbaChannelMeaningProperties s8ChProperties;

	uint32_t ui32StartupGain;	/* startup gain is used to initialize the 
								gain of the DRC prior to the first occurrence
								of dynamic_range_control */

	uint32_t ui32ExtraChannelMeaningLength;	/*This field indicates the length
											of extra_channel_meaning */
	
} BRAP_DSPCHN_FbaChannelMeaning;


/***************************************************************************
Summary:
	This enumeration defines Multi Channel type of FBB Stream

****************************************************************************/
typedef enum BRAP_DSPCHN_FbbMultiChannelType
{
	BRAP_DSPCHN_FbbMultiChannelType_eStandardspeakerlayout= 0,		
	BRAP_DSPCHN_FbbMultiChannelType_eNonStandardspeakerlayout,		
	BRAP_DSPCHN_FbbMultiChannelType_eNotSpeakerFeeds,		
	BRAP_DSPCHN_FbbMultiChannelType_eReserved
}BRAP_DSPCHN_FbbMultiChannelType;

/***************************************************************************
Summary:
	This enumeration defines Speaker Layouts of FBB Stream

****************************************************************************/
typedef enum BRAP_DSPCHN_FbbSpeakerLayout
{
	BRAP_DSPCHN_FbbSpeakerLayout_eNotStated= 0,		
	BRAP_DSPCHN_FbbSpeakerLayout_e2plus2plus2,		
	BRAP_DSPCHN_FbbSpeakerLayout_eReserved
}BRAP_DSPCHN_FbbSpeakerLayout;

/***************************************************************************
Summary:
	This enumeration defines Copy Protection of FBB Stream

****************************************************************************/
typedef enum BRAP_DSPCHN_FbbCopyProtection
{
	BRAP_DSPCHN_FbbCopyProtection_eUnrestricted=0,
	BRAP_DSPCHN_FbbCopyProtection_eCopyOnce,		
	BRAP_DSPCHN_FbbCopyProtection_eDoNotCopy,		
	BRAP_DSPCHN_FbbCopyProtection_eReserved 
}BRAP_DSPCHN_FbbCopyProtection;

/***************************************************************************
Summary:
	This enumeration defines Source Format of FBB Stream

****************************************************************************/
typedef enum BRAP_DSPCHN_FbbSourceFormat
{
	BRAP_DSPCHN_FbbSourceFormat_eNotStated=0,		
	BRAP_DSPCHN_FbbSourceFormat_eReserved 
}BRAP_DSPCHN_FbbSourceFormat;

/***************************************************************************
Summary:
	This structure contain information about Channel Meaning of FBB Stream

****************************************************************************/
typedef struct BRAP_DSPCHN_FbbChannelMeaning
{
	uint32_t ui32WordWidth;


	bool bChannelOccupancy[6];	/* Channel occupancy array registers whether 
								or not channels 0-5 are occupied. This 
								applies to an audio object as a whole;
								a bit within the mask will not be set to 
								zero just because a particular channel 
								is temporarily inactive. */

	BRAP_DSPCHN_FbbMultiChannelType eMultiChannelType; 	/*This field describes
															the channel purpose.*/


	BRAP_DSPCHN_FbbSpeakerLayout eSpeakerLayout;	/*speaker_layout is used to 
													describe a variety of horizontal
													and 3D speaker layouts*/


	BRAP_DSPCHN_FbbCopyProtection eCopyProtection;	
	bool bHdcdProcess;

	BRAP_DSPCHN_FbbSourceFormat eSourceFormat;	/*source_format indicates the 
													multichannel system being used*/


	uint32_t ui32SummaryInfo ;  /* summary_info is a compact encoding of the 
								common arrangements described in more detail 
								by multi_channel_type, source_format and 
								speaker_layout. Refer to section A.3.9 of 
								MLP Technical Reference Version 1.0. */

	
} BRAP_DSPCHN_FbbChannelMeaning;


/***************************************************************************
Summary:
	This enumeration defines Stream Mode of MLP.
****************************************************************************/
typedef enum BRAP_DSPCHN_MlpStreamMode
{
	BRAP_DSPCHN_MlpStreamMode_Fbb=0,		
	BRAP_DSPCHN_MlpStreamMode_Fba 
}BRAP_DSPCHN_MlpStreamMode;

/***************************************************************************
Summary:
	This structure contains MLP Stream Information.
***************************************************************************/
typedef struct BRAP_DSPCHN_MlpInfo
{
	BRAP_DSPCHN_MlpStreamMode eMlpStreamMode;
	
	bool bMajorSyncInfoConstFifoDelay;		


	bool bVariableRate;						/*true = Variable Rate, 
											  false = Fixed Rate*/

	uint32_t ui32PeakDataRate;		/*Specifies the maximum data rate of 
									the stream,taken over the whole audio 
									object in the unit of 1/16 bit per sample
									period*/


	uint32_t ui32NoOfStreams;		/*Specifies the number of Substream present*/


	uint32_t ui32SubstreamInfo;		/*Contains information on which substream(s)
									needs to be decoded based on the decoder 
									configuration.please refer to section 9.6.14 of 
									MLP Technical Reference Version 1.0*/

	
	union {
		BRAP_DSPCHN_FbaFormatInfo sFbaFormatInfo;	/*This field is populated 
													when MlpStreamMode = FBA*/

		
		BRAP_DSPCHN_FbbFormatInfo sFbbFormatInfo;	/*This field is populated 
													when MlpStreamMode = FBB*/

	} uFormatInfo;
	
	union {
		BRAP_DSPCHN_FbaChannelMeaning sFbaChannelMeaning;	/*This field is populated 
															when MlpStreamMode = FBA*/


		BRAP_DSPCHN_FbbChannelMeaning sFbbChannelMeaning;	/*This field is populated 
															when MlpStreamMode = FBB*/

	} uChannelMeaning;

} BRAP_DSPCHN_MlpInfo;



/***************************************************************************
Summary:
	This structure contains Stream Information.

***************************************************************************/
typedef struct BRAP_DSPCHN_StreamInfo
{
	BRAP_DSPCHN_AudioType    eType;   /* audio type */
	BAVC_AudioSamplingRate	eSamplingRate; 			/* Sampling freq value */

	union {
		BRAP_DSPCHN_MpegInfo  sMpegInfo;  /* MPEG stream info */
		BRAP_DSPCHN_Ac3Info  sAc3Info;    /* AC3 stream info */
		BRAP_DSPCHN_Ac3PlusInfo  sAc3LosslessInfo;    /* AC3 Lossless stream info */
		BRAP_DSPCHN_Ac3PlusInfo sAc3PlusInfo; /* AC3 PLUS stream info */
		BRAP_DSPCHN_AacInfo  sAacInfo;		/* AAC stream info */
		BRAP_DSPCHN_AacSbrInfo sAacSbrInfo;	/* AAC SBR stream info */
		BRAP_DSPCHN_DtsInfo	sDtsInfo;	/* DTS stream info */
   		BRAP_DSPCHN_DtshdInfo sDtshdInfo;	/* DTS HD stream info */
		BRAP_DSPCHN_BdLpcmInfo	sBdLpcmInfo;	/* LPCM BD stream info */
		BRAP_DSPCHN_LpcmHdDvdInfo	sLpcmHdDvdInfo;	/* LPCM HD DVD stream info */        
		BRAP_DSPCHN_LpcmDvdInfo	sLpcmDvdInfo;	/* LPCM DVD stream info */ 
		BRAP_DSPCHN_WmaStdInfo		sWmaStdInfo;	/* WMA Standard stream info */
		BRAP_DSPCHN_MlpInfo			sMlpInfo;		/* MLP stream info */
		BRAP_DSPCHN_WmaProInfo		sWmaProInfo;	/* WMA Pro stream info */		
	}uStreamInfo;

}BRAP_DSPCHN_StreamInfo;

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
	bool					bLfeOn;				/* Low frequency enhancement
    											   channel On/Off  - This value is not valid for WMA_STD and MPEG*/

    union 
    {
        BRAP_DSPCHN_MpegChannelMode  eChmod;     /* New MPEG mode for MPEG*/
#if(BCHP_CHIP == 7400)       
        BRAP_DSPCHN_Ac3PlusAcmod    eAcmod;     /* ACMOD for AC3, AC3 LOSSLESS, AC3Plus */
#else
        BRAP_DSPCHN_Ac3Acmod         eAcmod;     /* ACMOD for AC3, AC3 LOSSLESS, AC3Plus */
#endif
        BRAP_DSPCHN_AacAcmod		eAacAcmod;	/* ACMOD for AAC, AAC SBR */
        BRAP_DSPCHN_WmaStdAcmod	eWmaStdAcmod;	/* ACMOD for WMA_STD */
        BRAP_DSPCHN_WmaProAcmod	eWmaProAcmod;	/* ACMOD for WMAPRO_STD */
    }uModInfo;
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
	int32_t				i32AvOffset;		/* AV_Offset */	
	uint32_t				ui32PcrOffset;		/* PCR Offset */
	uint32_t				ui32Stc;			/* System Time Clock */
	uint32_t				ui32Pts;			/* Presentation Time Stamp */
	BRAP_DSPCHN_TsmStatus	eTsmStatus;			/* Status returned by FW TSM module */
	uint32_t				ui32UpperThreshold;	/* Upper Threshold */
	uint32_t				ui32LowerThreshold;	/* Lower Threshold */
	
	uint32_t				ui32Mbox0Val;		/* Mailbox 0 Register Value */
	uint32_t				ui32Mbox1Val;		/* Mailbox 1 Register Value */
	uint32_t				ui32Mbox2Val;		/* Mailbox 2 Register Value */
	uint32_t				ui32Mbox3Val;		/* Mailbox 3 Register Value */
	uint32_t				ui32Mbox4Val;		/* Mailbox 4 Register Value */
	uint32_t				ui32Mbox5Val;		/* Mailbox 5 Register Value */
	uint32_t				ui32Mbox6Val;		/* Mailbox 6 Register Value */
	uint32_t				ui32Mbox7Val;		/* Mailbox 7 Register Value */

	uint32_t				ui32EsrRegVal;		/* AUD_DSP_ESR_SO00_INT_STATUS Register Value */
} BRAP_DSPCHN_TsmDebugInfo;

/***************************************************************************
Summary:
	This enumeration lists various post processing algorithms supported by RAP PI.
***************************************************************************/
typedef enum BRAP_DSPCHN_PP_Algo
{
    BRAP_DSPCHN_PP_Algo_eDdbm,  /* Dolby Digital Bass Management. Supported
                                        only for 7411D0, when bMuxChannelPairsOnI2sConfig
                                        is selected in decode channel settings */
    BRAP_DSPCHN_PP_Algo_eSrc,   /* Sample Rate change. Supported only for 7401. */                                      
    BRAP_DSPCHN_PP_Algo_eAacDownmix,   /* AAC downmic. Used only for 7401 family with new mit code. */       
    BRAP_DSPCHN_PP_Algo_eDsola, /* DSOLA. Used for Fast/Slow playback in 7401 family with new mit code. */           
#if (BRAP_AD_SUPPORTED == 1)    
    BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl,    /* Fade Control. Used TODO */ 
    BRAP_DSPCHN_PP_Algo_eAD_PanCtrl,    /* Pan Control is used for controlling the horizontal position of the speacker. */ 
#endif    
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
    BRAP_DSPCHN_PP_Algo_eDolbyVolume, /* Dolby Volume post processing algorithm */
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
    BRAP_DSPCHN_PP_Algo_eSRS_TruVol, /* SRS True Volume post processing algorithm */
#endif
    BRAP_DSPCHN_PP_Algo_eMax    /* Maximum number of post processing algorithms
                                        supported in RAP PI */
} BRAP_DSPCHN_PP_Algo;

/***************************************************************************
Summary: Enable/Disable Time Stamp Management

Description:
	This function enables/disables Time Stamp Management for a given audio
	channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableTSM(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableTSM		/* [in] true = enable TSM,
							        false = disable TSM  */
	);

/***************************************************************************
Summary: ISR version of BRAP_DSPCHN_EnableTSM

Description:
	This function enables/disables Time Stamp Management for a given audio
	channel in isr context.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableTSM_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableTSM		/* [in] true = enable TSM,
							        false = disable TSM  */
	);

/***************************************************************************
Summary: Enable/Disable Adaptive System Time Management

Description:
	This function enables/disables Adaptive System Time Management(ASTM) for a given audio
	channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableASTM_isr
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableASTM(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableASTM		/* [in] true = enable ASTM,
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
BRAP_DSPCHN_EnableASTM_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableASTM		/* [in] true = enable ASTM,
							        false = disable ASTM  */
	);


/***************************************************************************
Summary: Set TSM discard threshold

Description:
	This function set discard threshold for time stamp management. If the
	PTS-STC difference for an audio frame is more than this threshold,
	then that frame is dropped.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTSMDiscardThreshold(
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32DiscardThreshold/* [in] Discard threshold value */
	);


/***************************************************************************
Summary: Set TSM upper threshold

Description:
	This function set Upper threshold for time stamp management. The TSM 
	module uses the upper threshold, lower threshold and discard threshold 
	to determine if the current frame should be decoded, waited, or 
	discarded
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM,BRAP_DSPCHN_SetTSMDiscardThreshold
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTSMUpperThreshold(
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32UpperThreshold/* [in] Upper threshold value */
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
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTsmTransitionThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmTransitionThreshold
	);

/***************************************************************************
Summary: Set TSM Smooth Tracking threshold
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTsmSTThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmSTThreshold   /* [in] The new ST threshold in ms*/
	);

/***************************************************************************
Summary: Set TSM Gross Adjustment threshold
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTsmGAThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmGAThreshold   /* [in] The new GA threshold in ms */
	);

/***************************************************************************
Summary: Set TSM sync limit threshold.

Description:
	This function sets synclimit threshold for audio master mode. The value of 
	i32TsmSLThreshold is in milliseconds. When the value is set to zero audio master mode
	is disabled.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetTsmSLThreshold (	
	BRAP_ChannelHandle	hRapCh,				/* [in] Audio channel handle */
	int32_t				i32TsmSLThreshold   /* [in] The new SL threshold */
	);

/***************************************************************************
Summary: Get current PTS

Description:
	This function returns the value of current PTS
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetCurrentPTS(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS information */ 
	);

/***************************************************************************
Summary: ISR version of Get current PTS

Description:
	This function returns the value of current PTS in ISR context
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetCurrentPTS_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS information */ 
	);

/***************************************************************************
Summary: Set PTS offset

Description:
	This function sets the value of PTS offset

Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetPTSOffset(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	int32_t				i32PtsOffset	/* [in] PTS Offset value */
	);

/***************************************************************************
Summary:
	Gets the Audio decoder delay in ms.
Description:
	This API returns the Audio delay.This delay is implemented in the FW/PI that
	includes only the silent output FMM PCM buffer play time.
Returns:
	BERR_SUCCESS 

See Also:
	BRAP_GetDefaultSettings.
***************************************************************************/
BERR_Code BRAP_GetAudioDecoderDelay(
	unsigned int *pAudioDecoderDelay	/* [out] Audio decoder delay*/
	);

/***************************************************************************
Summary: Get stream Information

Description:
	This function returns stream information of the decoded stream, if
	such information is available (i.e. DSP firmware has decoded atleast
	one audio frame and initialized stream information registers).
	Otherwise it returns error.
	The stream information is extracted from stream and then set in 
	registers AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT{0-3} by firmware.Before 
	samplerate change interrupt these information is not available in these 
	registers. So if the function is called before this interrupt, it will 
    return BRAP_ERR_STREAMINFO_NOT_AVAILABLE.

Returns:
	BERR_SUCCESS - If successful
	BRAP_ERR_STREAMINFO_NOT_AVAILABLE - If Stream Info is not available.	
**************************************************************************/

BERR_Code
BRAP_DSPCHN_GetStreamInformation(
	BRAP_ChannelHandle	hRapCh,		/* [in] Audio channel handle */
	BRAP_DSPCHN_StreamInfo *psStreamInfo	/* [out] Stream Information */
	);


/***************************************************************************
Summary:
	API returns current picture tag value.

Description:
	This API Gets the current picture tag value from firmware and returns to 
	application.
	
Returns:
	BERR_SUCCESS 

See Also:
	None
	
****************************************************************************/
BERR_Code BRAP_DSPCHN_GetPictureTag( 	
	BRAP_ChannelHandle hRapCh,         /* [in] The Raptor channel handle */
	unsigned long        *pulPictureTag  /* [out] Picture Tag value */
	);

/***************************************************************************
Summary: Set AC3 decoder configuration parameters
 
Description:
	This function sets configuration parameters of AC3 decoder
	Warning: This function's use is deprecated and will be removed in future. 
	Please use function BRAP_DEC_SetConfig for setting AC3 decoder parameters

Returns:
	BERR_SUCCESS - If successful

See Also: BRAP_DSPCHN_GetAc3ConfigurationParams
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetAc3ConfigurationParams(
	BRAP_ChannelHandle		hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_Ac3ConfigParams	*psAc3ConfigParams 	/* [in] AC3 decoder configuration parameters */
	);

/***************************************************************************
Summary: Gets AC3 decoder configuration parameters
 
Description:
	This function current settings of AC3 decoder configuration parameters
	Warning: This function's use is deprecated and will be removed in future. 
	Please use function BRAP_DEC_SetConfig for setting AC3 decoder parameters

Returns:
	BERR_SUCCESS - If successful

See Also: BRAP_DSPCHN_SetAc3ConfigurationParams
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetAc3ConfigurationParams(
	BRAP_ChannelHandle		hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_Ac3ConfigParams	*psAc3ConfigParams 	/* [out] AC3 decoder configuration parameters */
	);

/***************************************************************************
Summary: Enable/Disable TSM logging 
 
Description:
	This function enables/disables TSM logging feature in firmware. If TSM
	logging is enabled, firmware logs TSM debug data into ping-pong buffers
	and interrupts RAP PI. RAP PI interrupt handler initializes structure
	BRAP_DSPCHN_TsmLogInfo and pass it on to application callback function.
	Application function evcuates data from the log buffer.

Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableTsmLog(
	BRAP_ChannelHandle		hRapCh,					/* [in] Audio channel handle */
	bool					bEnable					/* [in] true = Enable TSM log
														false = Disable TSM log */
	);

/***************************************************************************
Summary: Get TSM related information 
Description:
	This function returns "Time Stamp Management" related information. This
	function is introduced for debug purpose.
Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetTsmDebugInfo(
	BRAP_ChannelHandle		hRapCh,					/* [in] Audio channel handle */
	BRAP_DSPCHN_TsmDebugInfo	*psTsmInfo			/* [out] TSM related info */
	);

/***************************************************************************
Summary: Get decoder lock/unlock status 
 
Description:
	This function returns decoder locked/unlocked status. When decoder
	finds frame sync pattern in the stream, this function returns decoder
	lock status otherwise it returns decoder unlock status.
	
Returns:
	BERR_SUCCESS - If successful

See Also: 
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetDecoderLockStatus(
	BRAP_ChannelHandle		hRapCh,					/* [in] Audio channel handle */
	bool					*pbLock					/* [out] true = Decoder locked 
													   false = Decoder unlocked */
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
BRAP_DSPCHN_SetStcValidFlag(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);

/***************************************************************************
Summary: Isr version of API  BRAP_DSPCHN_SetStcValidFlag

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
BRAP_DSPCHN_SetStcValidFlag_isr(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	);

/***************************************************************************
Summary: Enable/disable Dolby Digital Bass Management algorithm

Description:
	This function enables/disables Dolby Digital Bass Management post processing 
	algorithm.
	
Returns:
	BERR_SUCCESS - If successful
	
See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableDDBassManagement(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool					bEnable			/* [in] TRUE = Enable DD Bass Management 
											  FALSE = Disable DD Bass Management */
	);											  

/***************************************************************************
Summary: Enable/disable sync pattern for multi channel support on I2S

Description: This function enables/disables sync pattern for multi channel support on I2S.
	For this function to work, bMuxChannelPairsOnI2sConfig in BRAP_DEC_ChannelSettings
	should be set while opening the decode channel.

Returns:
	BERR_SUCCESS - If successful
	
See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableMultiChI2sSync(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool					bEnable			/* [in] TRUE = Enable MultiChannel I2S sync 
											  FALSE = Disable MultiChannel I2S sync  */
	);

BERR_Code
BRAP_DSPCHN_SetStartStopPts (
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	int					iStartPts,
	int					iStopPts
	);

/***************************************************************************
Summary: Get PTS to STC phase difference. 

Description:
	This function returns the PTS to STC difference programed by raptor F/W. This function
	is used for audio master mode.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetPts2StcPhase(
	BRAP_ChannelHandle	hRapCh,	/* [in] Audio channel handle */
	int32_t				*pi32Pts2StcPhase /* [Out] Difference between PTS and STC value */
	);

/***************************************************************************
Summary: Isr version of API  BRAP_DSPCHN_GetPts2StcPhase

Description:
	This function returns the PTS to STC difference programed by raptor F/W. This function
	is used for audio master mode.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetPts2StcPhase_isr(
	BRAP_ChannelHandle	hRapCh,	/* [in] Audio channel handle */
	int32_t				*pi32Pts2StcPhase /* [Out] Difference between PTS and STC value */
	);

/***************************************************************************
Summary: Enable/Disable Time Stamp Management error recovery

Description:
	This function enables/disables Time Stamp Management error recovery for a given audio
	channel.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_EnableTSM
**************************************************************************/
BERR_Code
BRAP_DSPCHN_EnableTSMErrorRecovery(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableTSMErrRecovery	/* [in] true = enable TSM Error Recovery,
							        		false = disable TSM Error Recovery  */
	);

#ifdef __cplusplus
}
#endif

#endif /* BRAP_DSPCHN_H__ */
/* End of File */

