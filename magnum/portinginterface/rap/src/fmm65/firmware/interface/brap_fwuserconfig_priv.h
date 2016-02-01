/***************************************************************************
 *
 *     Copyright (c) 2007-2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Description : 
 *	This  is required to define all the user config parameters for all the
 *	encode, decode and post processing algorithms in future.

 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * $brcm_author: $
 * $brcm_Log: $
 * 
 ***************************************************************************/
 

#ifndef BRAP_FWIF_USER_CONFIG_H
#include "brap_af_priv.h"

#define BRAP_FWIF_USER_CONFIG_H


#define  DDP_DEC_GBL_MAXPCMCHANS					6
#define  DTSHD_DEC_MAXCHANS							8

/*
	Common Enum for Acmode
*/

typedef enum BRAP_FWIF_P_AcMode
{
    BRAP_FWIF_P_AcMode_eMode10 = 1, /*C*/
    BRAP_FWIF_P_AcMode_eMode20 = 2, /*L,R*/
    BRAP_FWIF_P_AcMode_eMode30 = 3, /*L,C,R*/
    BRAP_FWIF_P_AcMode_eMode21 = 4, /*L,R,S*/
    BRAP_FWIF_P_AcMode_eMode31 = 5, /*L,C,R,S*/
    BRAP_FWIF_P_AcMode_eMode22 = 6, /*L,R,LS,RS*/
    BRAP_FWIF_P_AcMode_eMode32 = 7, /*L,C,R,LS,RS*/
    BRAP_FWIF_P_AcMode_eMode33 = 8, /*L,C,R,LS,RS,CS*/
    BRAP_FWIF_P_AcMode_eMode32_BSDigital = 9,/*L,C,R,LS,RS*/
    BRAP_FWIF_P_AcMode_eModeLtRt=0x20,/*L,R*/
    BRAP_FWIF_P_AcMode_eModePLII_Movie = 0x21,/*L,C,R,LS,RS*/
    BRAP_FWIF_P_AcMode_eModePLII_Music = 0x22,/*L,C,R,LS,RS*/
    BRAP_FWIF_P_AcMode_eModePassiveMatrix = 0x23,/*L,R*/
    BRAP_FWIF_P_AcMode_eModeCSII = 0x24,/*L,C,R,Ls,Rs,Cs*/
    BRAP_FWIF_P_AcMode_eLAST,
    BRAP_FWIF_P_AcMode_eINVALID=0x7fffffff

}BRAP_FWIF_P_AcMode;

/*
   This data structure defines AC-3/DDP decoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_Ac3UserOutput
{
	 /* This is LFE ON/OFF flag and can take  two values 0 or 1 
	  default value for this field is 0 */
	 int32_t i32OutLfe;                         
	 /* i32OutMode =7 default value;*/
	 int32_t i32OutMode;
	 /* preferred stereo mode  i32StereoMode = 0 is default value */
	 int32_t i32StereoMode;  
	 /* dual mono downmix mode i32DualMode = 0 is default value */
	 int32_t i32DualMode;
	 /* karaoke capable mode i32Kmode = 3 */
	 int32_t i32Kmode;	
	 /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
	  default value for this i32ExtDnmixEnabled field is 0 */
	 int32_t i32ExtDnmixEnabled;				
	 int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS]; 
	 /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
	  default value for this i32ExtKaraokeEnabled field is 0 */
	 int32_t i32ExtKaraokeEnabled;				
	 int32_t i32Extv1Level;						
	 int32_t i32Extv1Pan;						
	 int32_t i32Extv2Level;						
	 int32_t i32Extv2Pan;						
	 int32_t i32ExtGmLevel;					    
	 int32_t i32ExtGmPan;						
 	 /*This is channel matrix of size 8 where each index can take any values from 0 to 7
	   Depending upon  the channel routing done
	 */
	 uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
}BRAP_FWIF_P_Ac3UserOutput;

typedef struct  BRAP_FWIF_P_Ac3ConfigParams
{
	/* p_user_cfg->i32CompMode = 2;*/
	 int32_t					 i32CompMode;  
	 int32_t					 i32PcmScale;  
	/* p_user_cfg->i3DynScaleHigh = 0x7FFFFFFF;	 0x0 to  0x7FFFFFFF in Q1.31 format */
	 int32_t					 i32DynScaleHigh;    
	/* p_user_cfg->i32DynScaleLow = 0x7FFFFFFF;	 0x0 to  0x7FFFFFFF in Q1.31 format */
	 int32_t					 i32DynScaleLow;    
	/*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
	 int32_t					 i32NumOutPorts;     

	/* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
	   Default is Enable
	*/
	int32_t						 i32StreamDialNormEnable;

	/*	This value indicates how far the average dialogue level is below digital 100 percent. Valid
		values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB 
		with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization, 
		Default value is 0
	*/

	int32_t						 i32UserDialNormVal;

	 /* These are user config parameters required from user  */
	 BRAP_FWIF_P_Ac3UserOutput  sUserOutputCfg[2];
	 

}BRAP_FWIF_P_Ac3ConfigParams;



/*
   This data structure defines MPEG-1 L1/L2/L3 decoder user configuration parameters 
*/

typedef struct BRAP_FWIF_P_MpegConfigParams
{
	uint32_t	ui32OutMode;					 /* outputmode */
	uint32_t	ui32LeftChannelGain;			 /* In 1Q31 format */
	uint32_t	ui32RightChannelGain;			 /* In 1Q31 format */
	uint32_t	ui32DualMonoMode;
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
}BRAP_FWIF_P_MpegConfigParams;	


/*
   This data structure defines WMA-9 decoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_WmaConfigParams
{
	uint32_t	ui32OutputMode;

	/*These 2 variables are used to resolve problems in comparision in RTL platform	 for Certification*/
	uint32_t	decodeOnlyPatternFlag;
	uint32_t	decodePattern;

	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS]; /* Default Channel Matrix = {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF } */
}BRAP_FWIF_P_WmaConfigParams;


/*
   This data structure defines WMA-PRO decoder user configuration parameters 
*/

typedef enum BRAP_FWIF_P_eDrcSetting
{ 
	BRAP_FWIF_P_eDrcSetting_High=0,
    BRAP_FWIF_P_eDrcSetting_Med, 
    BRAP_FWIF_P_eDrcSetting_Low,
	BRAP_FWIF_P_eDrcSetting_eLAST,
    BRAP_FWIF_P_eDrcSetting_eINVALID=0x7fffffff

}BRAP_FWIF_P_eDrcSetting; 

typedef struct BRAP_FWIF_P_WmaProUserConfig
{
	uint32_t 					ui32DRCEnable;			/* Default 0 , range 0 and 1*/
	BRAP_FWIF_P_eDrcSetting		eDRCSetting;			/* Default 0 */
	int32_t						i32RmsAmplitudeRef;  
	int32_t						i32PeakAmplitudeRef;
	int32_t						i32DesiredRms;			/* desired rmsDb for normalization */
	int32_t						i32DesiredPeak;			/* desired peakDb for normalization */
	uint32_t	ui32OutMode;	/* Output channel configuration */
	uint32_t	ui32OutLfe;		/* Output LFE on/off */
	uint32_t	ui32Stereomode;	/* Stereo mode - Auto=0/ LtRt=1 /LoRo=2 */

	uint32_t					ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS]; 
								/* Default Channel Matrix = {0, 1, 2, 3, 4, 5, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF } */
}BRAP_FWIF_P_WmaProUserConfig;

typedef struct BRAP_FWIF_P_WmaProConfigParams
{
	uint32_t 						ui32NumOutports;	/* 1 -> Only Multichannel out [number of outmodes to be supported] ,  2 -> Stereo out + Multichannel */
	BRAP_FWIF_P_WmaProUserConfig	sOutputCfg[2];		/* User Config for Multichanel or Stereo Output */
	uint32_t						ui32UsageMode;		/* Decode =0, Passthru =1,SimulMode =2*/

}BRAP_FWIF_P_WmaProConfigParams;


/*
   This data structure defines AACHE multi-channel decoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_AacheUserConfig
{
	int32_t 	i32OutLfe;											/* Default = 0, Output LFE channel present */
	int32_t 	i32OutMode;											/* Default = 2, Output channel configuration */
	int32_t 	i32DualMode;										/* Default = 0, Dual mono reproduction mode */
	int32_t 	i32ExtDnmixEnabled;									/* Default = 0, Enable external downmix */
	int32_t 	i32ExtDnmixTab[6][6];								/* Default = 0, External downmix coefficient table */
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

} BRAP_FWIF_P_AacheUserConfig;

typedef struct  BRAP_FWIF_P_AacheConfigParams
{
	int32_t							i32NumOutPorts;		   			/* Default = 1, 1 -> Multichannel out, 2 -> Multichannel + Stereo out */
	int32_t							i32DownmixType;					/* Default = 0, 0 -> BRCM downmix, 1 -> ARIB downmix */
	int32_t							i32AribMatrixMixdownIndex;		/* Default = 0, ARIB matrix mixdown index which can take integer values from 0 to 3 */
	uint32_t						ui32DrcGainControlCompress;		/* Default = 0x40000000, Constant used in the DRC calculation, typically between 0 and 1, in Q2.30 format */
	uint32_t						ui32DrcGainControlBoost;		/* Default = 0x40000000, Constant used in the DRC calculation, typically between 0 and 1, in Q2.30 format */
	uint32_t			    		ui32DrcTargetLevel;				/* Default = 127, Target level desired by the host */
	BRAP_FWIF_P_AacheUserConfig		sUserOutputCfg[2];

																	/* Boost of 6dB, specific to Buffalo */
	int32_t							i32PcmBoost6dB;					/* Default = 0, 0 -> FALSE, 1 -> TRUE */

																	/* Attenuation of 4.75dB, specific to DDCO certification */
    int32_t                         i32PcmBoostMinus4p75dB;			/* Default = TRUE for LG branch and Default = FALSE for others, 0 -> FALSE, 1 -> TRUE */

	uint32_t						ui32FreqExtensionEnable;		/* Default = 0, 0->disable freq extension for high sample rate, 1->Enable Frequency extension for high sample rates */

	uint32_t						ui32SbrUserFlag;				/* default = 0, 0-> disable SBR processing, 1-> Enable SBR processing/Upsample, This flag is used currently for DVD only */ 

	uint32_t						ui32DownmixCoefScaleIndex;		/* Default = 50,  0 -> 0dB, 1 -> -0.5dB, 2 -> -1dB, ... , 23 -> -11.5dB, 24 -> -12dB; all values beyond 24 map to 50 */
																	/*				 50 -> Decoder default settings (existing normalization) */

} BRAP_FWIF_P_AacheConfigParams;



/* 
	MPEG1-Layer-2 encoder user configuration structure 
*/

typedef struct BRAP_FWIF_P_Mpeg1L2EncConfigParams
{
	uint32_t	ui32bitRateIdx;
	/* This differs from layer-3
	bitRateIdx		bit-rate
	[0]				0;
	[1]				32;
	[2]				48;
	[3]				56;
	[4]				64;
	[5]				80;
	[6]				96;
	[7]				112;
	[8]				128;
	[9]				160;
	[10]			192;
	[11]			224;
	[12]			256;
	[13]			320;
	[14]			384;
	*/

	uint32_t ui32AddCRCProtect;/* not supported */
	/*
	addCRCProtect	Implication
		0			CRC protect off
		1			CRC protect on
	*/

	uint32_t ui32Ext; /* Controls private bit setting in the header */
	/*				
	ext		Implication
	0		=> not Set 
	1		=> Set
	*/

	uint32_t ui32jStereoControl; /* Joint-Stereo Control */
	/*
	Layer-2 does not have provision for m/s stereo. The user can set either 0,1 or 3
	jStereoControl	Implication	
	0				No joint stereo (ie: no intensity or m/s stereo )
	1				Allow intensity stereo
	2				Allow m/s stereo
	3				Allow intensity and m/s stereo
	4				Reserved
	5				Reserved
	6				Reserved
	7				Reserved
	*/

	uint32_t ui32Copyright;	/* Copyright bit setting in the header */
	
	uint32_t ui32Original;	/* Original bit setting in the header */

	uint32_t ui32BitsPerSampleIdx;	/* number of bits per PCM sample */
			/* The no of bits per sample is treated as 16 + bitsPerSampleIdx */
	
	uint32_t ui32SampleRateIdx;
	/*
	sampleRateIdx	Sample-Rate
	[0]				44100;
	[1]				48000;
	[2]				32000;
	*/

	uint32_t	ui32Emphasis;	 /* not supported */
	/*
	emphasis	Implication
	0			none
	1			50/15 microseconds
	2			reserved
	3			CCITT J.17
	*/

	uint32_t ui32InputMode;
	/*
	0 stereo
	1 reserved
	2 dual channel
	3 mono
	*/

}BRAP_FWIF_P_Mpeg1L2EncConfigParams;

/*
   This data structure defines DTS-5.1 encoder user configuration parameters 
*/


typedef struct  BRAP_FWIF_P_DtsEncConfigParams
{
	uint32_t ui32Amode;				/* Audio Coding Mode 2 channel, 5 channel etc. */
	uint32_t ui32Sfreq;				/* Sampling Frequency */
	uint32_t ui32Dynf;				/* Embedded Dynamic Range Flag */
	uint32_t ui32Rate;				/* Bit Rate of the Encoder output */
	uint32_t ui32Mix;				/* Downmix to stereo enable Flag */
	uint32_t ui32Timef;				/* Embedded Time Stamp Flag */
	uint32_t ui32Auxf;				/* Auxiliary Data Flag */
	uint32_t ui32Hdcd;				/* Source in HDCD Format */
	uint32_t ui32Sumf;				/* Front Sum Difference Encode Flag */
	uint32_t ui32Sums;				/* Surround Sum Difference Encode Flag */
	uint32_t ui32Lfe;				/* Low Frequency Effect Flag */
	uint32_t ui32Dtses;				/* DTS Extended Surround Enable Format */
	uint32_t ui32Pcmr;				/* Source Input PCM Resolution */
	uint32_t ui32ExtAudio;			/* Extension Audio Present Flag */
	uint32_t ui32ExtAudioId;		/* Type of the Extension Audio like XXCH, XBR, X96 etc */
	uint32_t ui32Vernum;			/* Indicates the revision status of encoder software */
	uint32_t ui32DialNorm;			/* Dialogue Normalization */
	uint32_t ui32Joinx;				/* Joint Intensity Coding Enable Flag */
	uint32_t ui32Cpf;				/* Bit Stream CRC Enable Flag */
	uint32_t ui32Aspf;				/* Indicates the frequency with which DSYNC occurs in the stream */
	uint32_t ui32Ftype;				/* Frame type whether normal or termination frame */
	uint32_t ui32Chist;				/* Copy History.  History of the source material */
	uint32_t ui32Range;				/* Dynamic Range Coefficient */
	uint32_t ui32Addmf;				/* Auxiliary Dynamic Downmix Flag */
}BRAP_FWIF_P_DtsEncConfigParams;




/*
   This data structure defines stereo AAC-LC encoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_AaclcEncConfigParams
{
	uint32_t ui32Acmod;				/* Audio Coding Mode 2 channel, 5 channel etc.*/
	uint32_t ui32SampFreq;			/* Sampling Frequency */
	uint32_t ui32BitRate;			/* Bit Rate of the encoder */
/* This is flag which is used by AAC-LC encoder to decide whether PCM  data is coming directly or it is decimated by SBR module */
	uint32_t ui32SBR_PresentFlag;	
}BRAP_FWIF_P_AaclcEncConfigParams;



/*
   This data structure defines stereo AAC-HE encoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_AacheEncConfigParams
{
	uint32_t ui32Acmod;				/* Audio Coding Mode 2 channel, 5 channel etc.*/
	uint32_t ui32SampFreq;			/* Sampling Frequency */
	uint32_t ui32BitRate;			/* Bit Rate of the encoder */
}BRAP_FWIF_P_AacheEncConfigParams;

typedef enum BRAP_FWIF_eTsmBool
{
	BRAP_FWIF_eTsmBool_False,
	BRAP_FWIF_eTsmBool_True,
	BRAP_FWIF_eTsmBool_Last,
	BRAP_FWIF_eTsmBool_Invalid = 0x7FFFFFFF
}BRAP_FWIF_eTsmBool;


typedef struct BRAP_FWIF_P_TsmConfigParams
{
    int32_t                 i32TSMSmoothThreshold;  
    int32_t                 i32TSMSyncLimitThreshold;
    int32_t                 i32TSMGrossThreshold;
    int32_t                 i32TSMDiscardThreshold;
    int32_t                 i32TsmTransitionThreshold;     /* Transition threshold is required for the  
                                                                                DVD case not required right now*/
    uint32_t                ui32STCAddr;
    uint32_t                ui32AVOffset;           
    uint32_t                ui32PVROffset;
    uint32_t                ui32AudioOffset;
    
    /* For TSM error recovery*/
    BRAP_FWIF_eTsmBool            eEnableTSMErrorRecovery; /* Whether to go for error recovery 
                                                                                            when there are continuous TSM_FAIL */
    BRAP_FWIF_eTsmBool            eSTCValid;        /* If the STC in valid or not*/
    BRAP_FWIF_eTsmBool            ePlayBackOn;  /* If the play back in on of off */
    BRAP_FWIF_eTsmBool              eTsmEnable;   /* if the tsm is enable or not*/
    BRAP_FWIF_eTsmBool              eTsmLogEnable;  /*if the tsm log is enable or not */
    BRAP_FWIF_eTsmBool             eAstmEnable;  /* if the  Adaptive System Time Management(ASTM) enable or not */

}BRAP_FWIF_P_TsmConfigParams;	


/*
   This data structure defines Passthrough Node user configuration parameters 
*/

typedef enum BRAP_FWIF_ePassthruType
{
	BRAP_FWIF_ePassthruType_SPDIF,
	BRAP_FWIF_ePassthruType_RAW,
	BRAP_FWIF_ePassthruType_PCM,
	BRAP_FWIF_ePassthruType_LAST,
	BRAP_FWIF_ePassthruType_INVALID =0x7FFFFFFF

}BRAP_FWIF_ePassthruType;

typedef struct  BRAP_FWIF_P_PassthruConfigParams
{
	 BRAP_FWIF_ePassthruType ui32PassthruType;
}	 BRAP_FWIF_P_PassthruConfigParams;
 

/* User Configuration custom voice Post Processing tool*/
typedef struct
{
	int32_t param20;
	int32_t param21;
	int32_t param22;
	int32_t param42;
	int32_t param46[7];
	int32_t param47[7];
	int32_t param94[20];
	int32_t param109;
	int32_t param110;
	int32_t param19;
	int32_t param51[8];
	int32_t param50;
	int32_t param73;
	int32_t param74;
	int32_t param64;
	int32_t param63;
	int32_t param61;
	int32_t param62;
	int32_t param33;
	int32_t param34;
	int32_t param35;
	int32_t param36;
	int32_t param37;
	int32_t param38;
	int32_t param111;
	int32_t param67;
	int32_t param68;
	int32_t param69;
	int32_t param70;
	int32_t param18;
	int32_t param17;

}BRAP_FWIF_P_CVFreqDependentParams; 

typedef struct BRAP_FWIF_P_CustomVoiceConfigParams
{
	int32_t i32CVEnableFlag; /*1, CV3Enable;   */
	int32_t param00;		 /*1,*/
	int32_t param02;		 /*0x00000002, param02*/
	int32_t OSDVolume;		 /*0x00000001,*/
	int32_t param65;		 /*0x2037a500, param65*/
	int32_t param66;		 /*0x04FD6880, param66*/
	int32_t param71;		 /*0x04444400, param71*/
	int32_t param72;		 /*0x2aaaaa00, param72*/
	int32_t param39;		 /*0x00003000, param39*/
	int32_t param40;		 /*0x40a5c000, param40*/
	int32_t param48;		 /*0x05B7B100, param48*/
	int32_t param49;		 /*0x598C8180, param49*/
	int32_t param100;		 /*0x40000000, param100*/
	int32_t param102;		 /*0x00000014, param102*/
	int32_t param103;		 /*0x287A2680, param103*/
	int32_t param104;		 /*0x00000001, param104*/
	int32_t param105;		 /*0x51EB8005,*/
	int32_t param106;		 /*0x00000000,*/
	int32_t param107;		 /*0x00004189,*/
	int32_t param108;		 /*0x00378000,*/
	int32_t param52;		 /*0xd3000000, param52*/
	int32_t param53;		 /*0xe7000000,*/
	int32_t param54;		 /*0x40000000,*/
	int32_t param97;		 /*0x40000000, param97*/
	int32_t param59;		 /*0x00000000, param59*/
	int32_t param55;		 /*0x007FC000, param55*/
	int32_t param57;		 /*0x59f98000, param57*/
	int32_t param58;		 /*0x199b7200, param58*/
	int32_t param87;		 /*0x00000007, param87*/
	int32_t param88;		 /*0x08a3c500,*/
	int32_t param89;		 /*0x59992d00,*/
	int32_t param90;		 /*0x16666d00,*/
	int32_t param91;		 /*0x2666d300,*/
	int32_t param92;		 /*0x08a3c500, param92*/
	int32_t param93;		 /*0x16a77d00, param93*/
	int32_t param101;		 /*0x0000000d, param101*/
	int32_t param79;		 /*0x4ccccd00, param79*/
	int32_t param80;		 /*0x33333300, param80*/
	int32_t param81;		 /*0x28000000, param81*/
	int32_t param99;		 /*0x40000000, param99*/
	int32_t param98;		 /*0x00000001, param98*/
	int32_t param82;		 /*0x000000c8, param82*/
	int32_t param26;		 /*0x0146e700, param26*/
	int32_t param27;		 /*0xef02a300,*/
	int32_t param28;		 /*0x78000000,*/
	int32_t param29;		 /*0x78000000,*/
	int32_t param30;		 /* 0x16c31080,*/
	int32_t param31;		 /* 0x59f98000,*/
	int32_t param32;		 /* 0x287A2680, param32*/
	int32_t param23;		 /* 0x10000000, param23*/
	int32_t PARAM188;		 /* 0x1e000000, PARAM188*/
	int32_t PARAM189;		 /* 0xe2000000, PARAM189*/
	int32_t PARAM190;		 /* 0x4b000000, PARAM190*/
	int32_t PARAM191;		 /* 0xb5000000, PARAM191*/
	int32_t PARAM217;		 /* 0x0000001f, PARAM217*/
	int32_t PARAM218;		 /* 0x0000000b, PARAM218*/
	int32_t PARAM055;		 /* 0xc0000000, PARAM055*/
	int32_t PARAM056;		 /* 0xd9999a00, PARAM056*/
	int32_t PARAM132;		 /* 0x00000005, PARAM132*/
	int32_t PARAM133;		 /* 0x00000009, PARAM133*/
	int32_t PARAM134;		 /* 0x00000005, PARAM134*/
	int32_t PARAM135;		 /* 0x0000000a, PARAM135*/
	int32_t PARAM144;		 /* 0x00000028, PARAM144*/
	int32_t PARAM145;		 /* 0x00000028, PARAM145*/
	int32_t PARAM146;		 /* 0x0000012c, PARAM146*/
	int32_t param25; 		 /* 0x028F5C00, param25*/

	int32_t table_ram23[11];

	BRAP_FWIF_P_CVFreqDependentParams sFreqDepndentParams[3];

	int32_t param115;		/*0x00000001*/
	int32_t param116;		/*0x0000001e*/
	int32_t param117;		/*0x7fffff00*/
	int32_t table_coef33[11][2];
	int32_t table_coef34[11][3];
	int32_t table_coef35[11][2];
	int32_t table_coef36[11][3];
	int32_t table_coef37[2];
	int32_t table_coef38[3];
	int32_t table_coef39[2]; 
	int32_t table_coef40[3]; 
	int32_t DSPfxdu_C67;


}BRAP_FWIF_P_CustomVoiceConfigParams;


 /*
   This data structure defines DTS-5.1 Broadcast encoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_DtsBroadcastEncConfigParams
{
	uint32_t			ui32SpdifHeaderEnable;			/* Default value = Disable:  Enable =1 Disable =0 */
 
	/*Certification Related */
	uint32_t			ui32CertificationEnableFlag;	/*Default 0 Set to 1 while doing certification*/
	uint32_t			ui32LFEEnableFlag;				/*Used only when Certification is set to 1. Enable =1 Disable =0*/
	BRAP_FWIF_P_AcMode	eInputDataAcMode;				/*Used only when Certification is set to 1*/

}BRAP_FWIF_P_DtsBroadcastEncConfigParams;

/*
   This data structure defines AC3 encoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_Ac3EncConfigParams
{
	int32_t  i32NumChans;
	 
	/* Audio Coding mode, Default: 7(3/2)*/
	int32_t  i32AudCodMode;
	 
	/* Data Rate is dependent on Acmod, 18 for 3/2, 14 for 2/0 and 1/1 */
	int32_t  i32DataRate;
	 
	/*  1 is LFE enabeld */
	int32_t  i32LoFreqEffOn;
	 
	/* Always Set to 48 kHz */
	int32_t  i32SampRateCod;	 
	 
	/* LFE filter is not being used, always set to 0 */
	int32_t  i32LfeFiltInUse;
	/* compression characteristic 0 = off , always off*/
	int32_t  i32CompChar;
	 
	/* compression characteristic for 1+1 R chan, 0 = off, always off */
	int32_t  i32CompChar2;
	/* Always switched off in release code, can be enabled for certification testing */
	int32_t  i32SurDelayArg;
	 
	/* This is used to enable/disable SPDIF header.. Default value = Enable:  Enable =1 Disable =0 */
	BRAP_AF_P_EnableDisable eSpdifHeaderEnable;
	 
	/* This flag is set to enable when the Encoder is being used in transcode mode */
	BRAP_AF_P_EnableDisable   eTranscodeEnable;
	 
}BRAP_FWIF_P_Ac3EncConfigParams;



/*
   This data structure defines SRC user configuration parameters 
*/
typedef struct  BRAP_FWIF_P_SRCUserConfigParams
{
	uint32_t    ui32DummySrc;/*Default value is 0 
                               1-> is true and 0 is false 
							 */
}BRAP_FWIF_P_SRCUserConfigParams;


/* This data structure defines for AUDIO DESCRIPTOR */

typedef struct
{
	uint32_t   ui32FadeValue; /*Default - 0x7fffffff  (no fade) */

}BRAP_FWIF_P_sPanFadeIf; 

typedef enum BRAP_FWIF_P_FadeConfigType
{
    BRAP_FWIF_P_FadeConfigType_ePrimaryChOnly = 0, 
    BRAP_FWIF_P_FadeConfigType_eFadedChOnly,
    BRAP_FWIF_P_FadeConfigType_ePrimaryAndFadedCh,
    BRAP_FWIF_P_FadeConfigType_eLast,
	BRAP_FWIF_P_FadeConfigType_eInvalid = 0x7FFFFFFF 

} BRAP_FWIF_P_FadeConfigType;

/*
   This data structure defines Audio Descriptor Fade user configuration parameters 
*/
typedef struct  BRAP_FWIF_P_AudioDescFadeConfigParams
{
	BRAP_FWIF_P_FadeConfigType   eFadeConfig;				/* Default Value: BRAP_FWIF_P_FadeConfigType_eFadedChOnly */

    uint32_t					 ui32PanFadeInterfaceAddr;	/* This is a Dram Structure having the type 'BRAP_FWIF_P_sPanFadeIf'*/

	uint32_t					 ui32PanFadeInterfaceValidFlag;	/*Default value will be zero and set to 1 when the InterFaceAddr is filled*/

}BRAP_FWIF_P_AudioDescFadeConfigParams;

/*
   This data structure defines Audio Descriptor Fade user configuration parameters 
*/
typedef enum
{
    BRAP_FWIF_P_ADChannelConfig_eADChannelOff = 0,
    BRAP_FWIF_P_ADChannelConfig_eADChannelOn,
    BRAP_FWIF_P_ADChannelConfig_eLast,
    BRAP_FWIF_P_ADChannelConfig_eInvalid = 0x7FFFFFFF
}BRAP_FWIF_P_ADChannelConfig;


typedef struct  BRAP_FWIF_P_AudioDescPanConfigParams
{	
	/* Range 0 (mute)-0x7ffffffff(0Db) */

    int32_t    i32UserVolumeLevel;		/*Default - 0x7fffffff */

    uint32_t   ui32PanFadeInterfaceAddr;/* This is a Dram Structure having the type 'BRAP_FWIF_P_sPanFadeIf'*/	

    BRAP_FWIF_P_ADChannelConfig eADChannelConfig; /*This will give the AD channel is 
                                                    enable or disable. Default of this
                                                    is Enable*/
	 
	uint32_t ui32PanFadeInterfaceValidFlag;      /*Default value will be zero and set to 1 when the InterFaceAddr is filled*/

	uint32_t ui32AudioRampTimeInMs;					/*Default value should be 1000. Range 0 to 4Sec  */

}BRAP_FWIF_P_AudioDescPanConfigParams;


/*
   This data structure defines Frame sync configuration parameters 
*/

typedef enum BRAP_FWIF_P_CapInputPort
{
    BRAP_FWIF_P_CapInputPort_eIntCapPort0 = 0, /* Internal Capture port 0 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort1,     /* Internal Capture port 1 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort2,     /* Internal Capture port 2 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort3,     /* Internal Capture port 3 */
    BRAP_FWIF_P_CapInputPort_eExtI2s0,         /* External I2S Capture port */
    BRAP_FWIF_P_CapInputPort_eRfAudio,         /* BTSC Capture port	*/
    BRAP_FWIF_P_CapInputPort_eSpdif,           /* SPDIF Capture port	*/
    BRAP_FWIF_P_CapInputPort_eHdmi,            /* HDMI */
	BRAP_FWIF_P_CapInputPort_eAdc,
    BRAP_FWIF_P_CapInputPort_eIntCapPort4,     /* Internal Capture port 4 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort5,     /* Internal Capture port 5 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort6,     /* Internal Capture port 6 */
    BRAP_FWIF_P_CapInputPort_eIntCapPort7,     /* Internal Capture port 7 */
	BRAP_FWIF_P_CapInputPort_eDummy,		   /* This enum is used for File based input.... Mainly applicable to Certification purposes*/	
    BRAP_FWIF_P_CapInputPort_eMax,			   /* Invalid/last Entry */
	BRAP_FWIF_P_CapInputPort_eInvalid = 0x7FFFFFFF
 
} BRAP_FWIF_P_CapInputPort;


typedef enum BRAP_FWIF_P_WMAIpType
{
 BRAP_FWIF_P_WMAIpType_eASF = 0,
 BRAP_FWIF_P_WMAIpType_eTS,
 BRAP_FWIF_P_WMAIpType_eMax,
 BRAP_FWIF_P_WMAIpType_eInvalid = 0x7FFFFFFF
}BRAP_FWIF_P_WMAIpType;

typedef enum BRAP_FWIF_P_ASFPTSType
{
 BRAP_FWIF_P_ASFPTSType_eInterpolated = 0,
 BRAP_FWIF_P_ASFPTSType_eCoded,
 BRAP_FWIF_P_ASFPTSType_eMax,
 BRAP_FWIF_P_ASFPTSType_eInvalid = 0x7FFFFFFF
}BRAP_FWIF_P_ASFPTSType;
 
typedef enum BRAP_FWIF_P_AudioInputSource
{
    BRAP_FWIF_P_AudioInputSource_eExtI2s0 = 0,		/* External I2S Capture port */
    BRAP_FWIF_P_AudioInputSource_eCapPortRfAudio,   /* BTSC Capture port */
    BRAP_FWIF_P_AudioInputSource_eCapPortSpdif,     /* SPDIF Capture port      */
    BRAP_FWIF_P_AudioInputSource_eCapPortHdmi,      /* HDMI */
    BRAP_FWIF_P_AudioInputSource_eCapPortAdc,
    BRAP_FWIF_P_AudioInputSource_eRingbuffer,		/* This is for Certification needs where PCM Samples are loaded from file to Ring buffers*/
    BRAP_FWIF_P_AudioInputSource_eMax,              /* Invalid/last Entry */
    BRAP_FWIF_P_AudioInputSource_eInvalid = 0x7FFFFFFF

} BRAP_FWIF_P_AudioInputSource;

 
typedef struct BRAP_FWIF_P_FrameSyncConfigParams 
{
    BRAP_AF_P_EnableDisable         eEnablePESBasedFrameSync;    /* Default = Disabled */
    BRAP_FWIF_P_AudioInputSource    eAudioIpSourceType;           /* Capture port Type    */

    union
    {
        uint32_t ui32SamplingFrequency;					/* Will be used if IpPortType is I2S*/
        uint32_t ui32RfAudioCtrlStatusRegAddr;			/* For RfAudio i.e. BTSC */
        uint32_t ui32SpdifCtrlStatusRegAddr;			/* For SPDIF */ 
        uint32_t ui32MaiCtrlStatusRegAddr;				/* For HDMI */

    }uAudioIpSourceDetail; 

	BRAP_AF_P_EnableDisable               eEnableTargetSync;   /* Default = Enabled */

    struct
    {          
        BRAP_FWIF_P_ASFPTSType eAsfPtsType;				/* Default = 0 (Use Interpolation always). 1 = Use Coded always. */ 

        BRAP_FWIF_P_WMAIpType eWMAIpType;				/* Default = 0 (Type ASF). Set to TS only when WMATS is enabled */       

    }sAlgoSpecConfigStruct;                             /* The algo specific structures for configuration */     

} BRAP_FWIF_P_FrameSyncConfigParams ;




/*
	This data structure defines AVL configuration parameters 
*/

typedef struct BRAP_FWIF_P_AVLConfigParams
{   
   uint32_t  ui32AVLEnableFlag;		/* Default value = Enable:  Enable =1 Disable =0 */
   int32_t   i32Target;				/* -18*32768 Target level of output signal in terms of dBFs Q15   */
   int32_t   i32LowerBound;			/* -24*32768 Lower bound for primary range of volume control Q15   */
   int32_t   i32FixedBoost;			/* 6*32768   Amount of fixed boost (in dB) if level < LOWERBOUND Q15  */
   int32_t   i32RefLevel;			/* 2959245	(20*log10(32768)) Reference logarithmic gain value of 0 dBFs Q15 */
   int32_t   i32Alpha;				/* 32736	(1023/1024)		Attenuation factor for Level Maximum Q15    */
   int32_t   i32Beta;				/* 32256	(63/64)			Attenuation factor for Level Mean Q15      */
   int32_t   i32ActiveThreshold;    /* 9830		(0.3)			Threshold for detecting active portion of signal Q15   */
   int32_t   i32DTFPCNT;			/* 4		(4)				Decay Time for DTF Percent delay (in seconds) Q0    */
   int32_t   i32Alpha2;				/* 32767	(32767/32768)	Attenuation factor for Level Minimum Q15    */
   int32_t   i32NSFGR_SEC;			/* 16384	(0.5)			Fast Gain Ramp in milliseconds Q15       */
   int32_t   i32DTF;				/* 9830		(0.3)			Decay Time Fraction Q15          */

}BRAP_FWIF_P_AVLConfigParams;


/*
	This data structure defines SRS Tru Surround configuration parameters 
*/

typedef enum BRAP_FWIF_P_SpeakerSize
{
	BRAP_FWIF_P_SpeakerSize_eLFResponse40Hz  = 0,
	BRAP_FWIF_P_SpeakerSize_eLFResponse60Hz  = 1,
	BRAP_FWIF_P_SpeakerSize_eLFResponse100Hz = 2,
	BRAP_FWIF_P_SpeakerSize_eLFResponse150Hz = 3,
	BRAP_FWIF_P_SpeakerSize_eLFResponse200Hz = 4,
	BRAP_FWIF_P_SpeakerSize_eLFResponse250Hz = 5,
	BRAP_FWIF_P_SpeakerSize_eLFResponse300Hz = 6,
	BRAP_FWIF_P_SpeakerSize_eLFResponse400Hz = 7,
	BRAP_FWIF_P_SpeakerSize_eLAST,
	BRAP_FWIF_P_SpeakerSize_eINVALID = 0x7fffffff

} BRAP_FWIF_P_SpeakerSize;


/*
	This data structure defines Tru Surrnd-XT configuration parameters 
*/
 typedef struct  BRAP_FWIF_P_TruSurrndXTConfigParams
{ 
	/* TruSurround options  */
    uint32_t		ui32TSEnable;				/* Default value = Enable:  Enable =1 Disable =0 */
    uint32_t		ui32TSHeadphone;			/* Default value = Disable:  Enable =1 Disable =0 */
    int32_t			i32TruSurroundInputGain;	/* Default value 0x40000000 */
 
	/*DialogClarity options*/
    uint32_t		ui32DialogClarityEnable;	/* Default value = Enable:  Enable =1 Disable =0 */
    uint32_t		ui32DialogClarityLevel;		/* Default value 0x40000000*/
 
    /* TruBass options */
    uint32_t		ui32TBEnable;				/* Default value = Enable:  Enable =1 Disable =0 */
    int32_t			i32TBLevel;					/* Default value 0x33333333 */
    BRAP_FWIF_P_SpeakerSize eTBSpeakerSize;		/* Default value BRAP_FWIF_P_SpeakerSize_eLFResponse60Hz */    
 
    /* Certification related options */
    uint32_t		ui32CertificationEnableFlag;		
												/* Default value is 0.  Set to 1 while doing certification*/
    BRAP_FWIF_P_AcMode            eAcMode;		/* Varies depending on the input mode to be certified*/
 
}BRAP_FWIF_P_TruSurrndXTConfigParams;


/*
	This data structure defines Dolby Prologic II configuration parameters 
*/

typedef struct BRAP_FWIF_P_PL2ConfigParams
{
    uint32_t  ui32enable;		/* Default value = Enable:	 Enable =1 Disable =0 */
    uint32_t  ui32abalflg;		/* Default value = Disable:  Enable =1 Disable =0 */
    uint32_t  ui32decmode;		/* Default = 3*/                 
    uint32_t  ui32chancfg;		/* Default = 7*/                 
    uint32_t  ui32cwidthcfg;	/* Default = 3*/                 
    uint32_t  ui32dimcfg;		/* Default = 3*/                 
    uint32_t  ui32panoramaflg;	/* Default value = Disable:  Enable =1 Disable =0 */
    uint32_t  ui32sfiltflg;		/* Default value = Disable:  Enable =1 Disable =0 */
    uint32_t  ui32rspolinvflg;	/* Default value = Disable:  Enable =1 Disable =0 */
    uint32_t  ui32pcmscl;		/* Default = 100 */                 
    uint32_t  ui32debugOn;		/* Default value = Off:  On =1 Off =0 */
    uint32_t  ui32outputch;		/* Default = 0*/                 

} BRAP_FWIF_P_PL2ConfigParams;

/*
	This data structure defines DVD LPCM configuration parameters 
*/

typedef struct BRAP_FWIF_P_LpcmOutputConfig
{
	uint32_t    ui32OutMode;					/* Output channel assignment mode */
												
	uint32_t    ui32LfeOnFlag;					/* Flag for LFE 
												   Default value = Disable:  Enable =1 Disable =0 
												*/
	uint32_t    ui32DualMonoModeFlag ;			/* Mode for source selection in dual-mono LPCM-VR streams
												   Default value = Disable:  Enable =1 Disable =0 
												*/
	uint32_t    ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

}BRAP_FWIF_P_LpcmOutputConfig ;				/* Output configuration structure for an output port */
 
typedef struct BRAP_FWIF_P_LpcmUserConfig
{
	uint32_t						ui32NumOutputPorts;    /* Number of output ports */
	BRAP_FWIF_P_LpcmOutputConfig	sOutputConfig[2];      /* Array of output configuration structures, one for each output port  */
	uint32_t    ui32UseUserDownmixCoeffsFlag;	/* Flag to use user downmix coefficients.
												   Default value = Disable:  Enable =1 Disable =0 
												*/	    
	int32_t                         i32UserDownmixTables[16][8][8];  /* 16x8x8 User DownMix tables - decoder uses only 1 such 8x8 table for a given frame  */

}BRAP_FWIF_P_LpcmUserConfig;


/*
	This data structure defines dsola configuration parameters 
*/

typedef struct BRAP_FWIF_P_DsolaConfigParams
{ 
	/*The formula to calculate the Input PCM Frame Size for DSOLA for different playback rate is as below 
	
		ui32InputPcmFrameSize	= PlayBackRate * 512

		Example

		 ----------------------------------------
		 | PlayBackRate | ui32InputPcmFrameSize |
		 ----------------------------------------
		 |		1x		|		512				|
		 |		2x		|		1024			|
		 |	   .8x		|		409				|
		 |	   .9x		|		460				|
		 |	  1.2x		|		614				|
		 ----------------------------------------
	*/
     uint32_t ui32InputPcmFrameSize; /* Default value 0x266 */

}BRAP_FWIF_P_DsolaConfigParams;

/*
	This data structure defines Custom bass configuration parameters 
*/

typedef struct BRAP_FWIF_P_CustomBassUserConfig
{                                    

    uint32_t ui32enable;				/* Enable =1 Disable =0 Default=Enable */
    uint32_t ui32operation;				/* 0 (AGC and Harmonics) */
    uint32_t ui32agcType;				/* 0 (low and high) */
    uint32_t ui32harmonicsType;			/* 0 (half wave rectifier) */
    uint32_t ui32lpfFc;					/* 0xE (200 Hz) */
    uint32_t ui32hpfFc;					/* 0x12 (315 Hz) */
    uint32_t ui32agcGainMax;			/* 0x12 (6 dB) */
    uint32_t ui32agcGainMin;			/* 0xC (0 dB) */
    uint32_t ui32agcAttackTime;			/* 0x2 (5 ms)  */
    uint32_t ui32agcRelTime;			/* 0x2 (0.5 s)  */
    uint32_t ui32agcThreshold;			/* 0x16 (-22 dB) */
    uint32_t ui32agcHpfFc;				/* 0x8 (100 Hz) */
    uint32_t ui32harLevel;				/* 0 (0 dB) */
    uint32_t ui32harLpfFc;				/* 0x11 (280 Hz) */
    uint32_t ui32harHpfFc;				/* 0x8 (100 Hz) */

} BRAP_FWIF_P_CustomBassUserConfig;


/*
	This data structure defines Custom Surround configuration parameters 
*/
typedef struct BRAP_FWIF_P_CustomSurroundUserConfig
{                             
    uint32_t ui32enable;		/* Enable =1 Disable =0 Default=Enable */
    uint32_t ui32combOn;		/* FALSE;	   TURE, FALSE*/
    uint32_t ui32delay;			/* 0;          0x0 ~ 0x1F samples*/
    uint32_t ui32volume1;		/* 0(0dB);     0x0 ~ 0xC (0dB ~ 12dB), 0xF (infinite)*/
    uint32_t ui32volume2;		/* 0(0dB);     0x0 ~ 0xC (0dB ~ 12dB), 0xF (infinite)*/
    uint32_t ui32volume3;		/* 0(0dB);     0x0 ~ 0xC (0dB ~ 12dB), 0xF (infinite)*/
    uint32_t ui32volume4;		/* 0x1E(-3dB); 0x0 ~ 0x40 (12dB ~ -20dB with half dB step)*/
    uint32_t ui32volume5;		/* 0x1E(-3dB); 0x0 ~ 0x40 (12dB ~ -20dB with half dB step)*/
    uint32_t ui32lpfFc;			/* 0x74(8kHz); 0x70 ~ 0x76 (5kHz ~ 10kHz)*/
    uint32_t ui32lpfQ;			/* 0x44(1.0);  0x40 ~ 0x4F (0.33 ~ 8.20)*/
    uint32_t ui32lpfGain;		/* 0x56(-6dB); 0x40 ~ 0x4C (0dB ~ 12dB), 0x51 ~ 0x5C (-1dB ~ -12dB) */

    /* The funcVol and inputTrim are independent of SHARP Surround */
    uint32_t ui32funcVol;		/* 0x40        (0 dB)*/
    uint32_t ui32inputTrim;		/* 0x14        (-20 dB)*/

} BRAP_FWIF_P_CustomSurroundUserConfig;


/*
	This data structure defines DTS core configuration parameters 
*/
typedef struct  BRAP_FWIF_P_DtsCoreUserConfig
{
	int32_t		i32OutputMode;									/* Default: 2 */
	int32_t		i32OutLfe;										/* Default: 0 */	
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];/* Default Channel Matrix = {0, 1, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF} */

}BRAP_FWIF_P_DtsCoreUserConfig;

typedef struct  BRAP_FWIF_P_DtsCoreConfigParams
{
	int32_t							i32NumOutPorts;		/* Default: 1 */  
	BRAP_FWIF_P_DtsCoreUserConfig   sUserOutputCfg[2];	/* These are user config parameters required from user  */

}BRAP_FWIF_P_DtsCoreConfigParams; 


/*
	This data structure defines Dolby Volume configuration parameters 
*/

typedef struct BRAP_FWIF_P_DolbyVolumeUserConfig
{
	/*General Settings*/

	int32_t		i32DolbyVolumeEnable;			/*Range of values -  0 or 1 
												
												  As per the new guyidelines from Dolby, i32VolumeIqEnable =1(True) for 
												  both when DV is enabled and disabled.
													
												  The controlling factor is now i32LvlEnable 

												  Default value is 1 (True)
												*/

	int32_t		i32BlockSize;					/*Size of processing block in samples */
												/*Range of values - 256, 512 */
												/*Default value    : 512 */
	int32_t		i32nBands;						/*Number of critical bands to use */
												/*Possible values 20 or 40, default 20*/


	int32_t		i32nChans;						/*Number of input/output channels */
												/*Default val 2*/

	/* System Settings*/
	int32_t		i32InputReferenceLevel;			/*Input reference level in dBSPL */
												/*Range of values(in dB )- (0 to 100 in 8.24 format)*/
												/*Default value :0x4A000000 (74 dB)*/

	int32_t		i32OutputReferenceLevel;		/* Output reference level in dBSPL */
												/*Range of values(in dB) - (0 to 100 in 8.24 format)*/
												/*Default value    : 0x4A000000 (74 dB)*/

	int32_t		i32Calibration;					/*Calibration offset in dB */
												/*Range of values(in dB)- (-100 to 30 in 8.24 format)*/
												/*Default value    : 0x00000000 (0 dB)*/

	int32_t		i32VlmEnable;					/*Volume modeler on/off												
												  Possible values are 0 and 1,default 1 					 
												*/

	int32_t		i32ResetNowFlag;				/*User-input forced reset flag */
												/*Range [0,1], default 0*/


	/*Volume Modeler Settings*/
	int32_t		i32DigitalVolumeLevel;			/*Volume level gain in dB -- applied by dolby volume */
												/*Range of values(in dB) - (-100 to 30 in 8.24 format)*/
												/*Default value    : 0x00000000 (0 dB)*/

	int32_t		i32AnalogVolumeLevel;			/*Volume level gain in dB -- applied after dolby volume */
												/*Range of values(in dB) - (-100 to 30 in 8.24 format)*/
												/*Default value    :  0x00000000 (0 dB)*/

	/*Volume Leveler Settings */
	int32_t		i32LvlAmount;					/*Range of values - [0 to 10]*/
												/*Default value		:9 */
	
	int32_t		i32LvlEnable;					/*Range of values -  0=FALSE , 1=TRUE

												 i32LvlEnable = 1 When Dolby Volume is Enabled
												 i32LvlEnable = 0 When Dolby Volume is Disabled

												 Default value : 1 (TRUE - volume leveler enabled)
												*/
	
	int32_t		i32EnableMidsideProc;			/*Enable midside processing */
												/*Range [0,1], default ->0-> disable*/

	int32_t     i32HalfmodeFlag;				/*Flag to operate Dolby Volume in half mode*/
												/*Range [0,1], default ->0-> disable*/

	
	int32_t     i32LimiterEnable;				/*Enable Limter*/ 
												/*When DV is enabled limiter_enable = TRUE
												  When DV is disabled limiter_enable = FALSE*/


}BRAP_FWIF_P_DolbyVolumeUserConfig;




/*
	This data structure defines DTS broadcast decoder configuration parameters 
*/
typedef struct BRAP_FWIF_P_DtshdUserOutput
{
	int32_t i32UserDRCFlag; 	/*  Default =0  	Range : 0,1 (0-> DRC enabled; 1-> DRC disabled) 
									This Flag is used to turn on/off dynamic range compression 
								*/
	
	int32_t i32DynScaleHigh;
								/*  Default = 0x7fffffff
									Range 0 to 0x7fffffff (0 to 100%)
									Dynamic range compression cut scale factor. Its in Q1.31 format.*/
	
	int32_t i32DynScaleLow;

								/*  Default = 0x7fffffff
									Range 0 to 0x7fffffff (0 to 100%)
									Dynamic range compression boost scale factor. Its in Q1.31 format.
								*/

	uint32_t ui32OutMode;		/*Default =7;
								Output channel configuration */
								/* This field indicates Ouput channel Configuration
									0 = Two_mono_channels_1_1_ch1_ch2
									1 = One_centre_channel_1_0_C
									2 = Two_channels_2_0_L__R
									3 = Three_channels_3_0_L_C_R
									4 = Three_chanels_2_1_L_R_S
									5 = Four_channels_3_1_L_C_R_S
									6 = Four_channels_2_2_L_R_SL_SR
									7 = Five_channels_3_2_L_C_R_SL_SR */

	uint32_t ui32OutLfe;
								/* Default = 1 
								   Range : 0,1 (0-> LFE disabled;1-> LFE enabled)
								   Flag used to enable/disable LFE channel output */
	
	uint32_t ui32DualMode;
								/* Default =2
								   Range : 0-3 (0-> DUALLEFT_MONO; 1->DUALRIGHT_MONO;2->STEREO;3->DUAL MIX MONO)
								   Configure decoder output for dual mode */

	 uint32_t ui32StereoMode;

								/*  Default=0
									Range : 0,1 (1->Lt/Rt downmix;0->Normal output)
									Perform stereo downmix of decoder output */

	uint32_t ui32AppSampleRate;
								/* Default : 48000
								   Range : 192,000, 176400, 96000, 88200, 48000, 44100 , 320000
								   This is the sampling rate set by application layer. The decoder will limit decoding additional components in the stream (e.g, X96) to adjust the sampling rate of the decoded output, to that set by the application
								 */
	
	uint32_t ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
								/*Here BRAP_AF_P_MAX_CHANNELS = 8 -> see the detail description in bottom */

	 uint32_t ui32ExtdnmixEnabled;
								/* Default=0
									Range : 0,1 (0->Disable external downmix;1->Enable external downmix) */

	int32_t i32ExtDnmixTab[BRAP_AF_P_MAX_CHANNELS][BRAP_AF_P_MAX_CHANNELS];
								/* Here BRAP_AF_P_MAX_CHANNELS = 8*/
								/*Default all zeros; User specified downmix coefficients can be given through this matrix*/

}BRAP_FWIF_P_DtshdUserOutput;

typedef struct BRAP_FWIF_P_DtsHdConfigParams	 			
{				
	uint32_t ui32DecodeCoreOnly;
								/*Default=1; Range : 0,1 (1-> Decode only core, Disable all extension components)
								 This flag has precedence over other decode flags like  ui32DecodeDtsOnly,  ui32DecodeXLL etc) */
	uint32_t ui32DecodeDtsOnly;
								/*Default=0; Range : 0,1 (1-> decode components present in core substream only)*/
	uint32_t ui32DecodeXLL;			
								/*Default=1; Range : 0,1 (0-> does not decode XLL component)*/
	uint32_t ui32DecodeX96;
								/*Default=1; Range : 0,1 (0-> does not decode X96 component)*/			
	uint32_t ui32DecodeXCH;			
								/*Default=1; Range : 0,1 (0-> does not decode XCH component)*/
	uint32_t ui32DecodeXXCH;			
								/*Default=1; Range : 0,1 (0-> does not decode XXCH component)*/
	uint32_t ui32DecodeXBR;			
								/*Default=1; Range : 0,1 (0-> does not decode XBR component)*/
	uint32_t ui32EnableSpkrRemapping;			
								/*Default=0; Range : 0,1 (1->Enable Speaker remapping; 0->Disable speaker remapping)*/
	uint32_t ui32SpkrOut;			
								/*Default=2123*/
											/*Specifies the speaker out configuration. Instructs the decoder to perform downmixing or speaker remapping to yield the desired output speaker configuration. 
												SpkrOut - Output Speaker Configuration 
												0  - Use Encoded Native (default) 
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
												31- C L R Ls Rs LFE1 Cs	*/

	uint32_t ui32MixLFE2Primary;			
												/*Default =0; Range : 0,1 (1-> Mix LFE to primary while downmixing, when Lfe output is disabled)*/
	uint32_t ui32ChReplacementSet;
												/*Default =0*/	
												/*It can take all possible values from 0 to 7. But it is not really required in core decoder. But, it will be needed in DTS-HD decoder going forward.*/
	uint32_t i32NumOutPorts;		
												/* Default =2; Range : 1,2 (2-> enables PCM output and concurrent stereo)*/


	uint32_t  ui32EnableMetadataProcessing;		/* Default =0; Range : 0,1 (0->Disable metadata processing, 1->Enable metadata processing*/

	BRAP_FWIF_P_DtshdUserOutput sUserOutputCfg[2];					

}BRAP_FWIF_P_DtsHdConfigParams;





/*
	MPEG1-Layer-3 encoder user configuration structure
*/
typedef struct BRAP_FWIF_P_Mpeg1L3EncConfigParams
{
	uint32_t ui32BitRate;	/* Default Value = 128000; */
								/*
								Valid bit-rates
								0
								32000
								40000
								48000
								56000
								64000
								80000
								96000
								112000
								128000
								160000
								192000
								224000
								256000
								320000
								*/

	uint32_t ui32AddCRCProtect; /* Default Value = 0; */
								/*
								addCRCProtect	Implication
									0			CRC protect off
									1			CRC protect on
								*/

	/* Controls private bit setting in the header */
	uint32_t ui32PrivateBit;	/* Default Value = 0; */
								/*
								ext		Implication
								0		=> not Set
								1		=> Set
								*/

	/* Joint-Stereo Control , no joint stereo supported */
	uint32_t ui32jStereoControl; /* Default Value = 0; */
								/*
								jStereoControl	Implication
								0				No joint stereo (ie: no intensity or m/s stereo )
								1				Allow intensity stereo
								2				Allow m/s stereo
								3				Allow intensity and m/s stereo
								4				Reserved
								5				Reserved
								6				Reserved
								7				Reserved
								*/

	/* Copyright bit setting in the header */
	uint32_t ui32Copyright;		/* Default Value = 0; */
								/*
									 Copyright setting
									0			Copyright  off
									1			Copyright on
								*/

	/* Original bit setting in the header */
	uint32_t ui32Original;		/* Default Value = 0; */
								/*
									 Original bit setting
									0			Original bit setting off
									1			Original bit setting on
								*/

	/* number of bits per PCM sample */
	uint32_t ui32BitsPerSample;	/* Default Value = 16; */
								/*
								Possible values 16 and 24.
								*/

	uint32_t ui32SampleRate; /* Default Value = 48000; */
								/*
								Valid Sample-Rates
								44100;
								48000;
								32000;
								*/

	uint32_t ui32Emphasis;		/* Default Value = 0; */
								/*
								emphasis	Implication
								0			none
								1			50/15 microseconds
								2			reserved
								3			CCITT J.17
								*/

	uint32_t ui32InputMode;		/* Default Value = 0; */
								/*
								0 stereo
								1 reserved
								2 dual channel
								3 mono
								*/
	uint32_t ui32InterleavedPCM;/* Default Value = 0; */
								/*
								Input PCM Samples Interleaved or blocked
									0	De-Interleaved input. Samples seperated for available channels and MSB aligned i.e. left shifted by (32 - ui32BitsPerSample)
									1		Interleaved( Direct input from the bitstream)
								*/
}BRAP_FWIF_P_Mpeg1L3EncConfigParams;

/*
   This data structure defines SBC Encoder user configuration parameters 
*/

typedef struct BRAP_FWIF_P_SbcEncoderUserConfig
{
   	uint32_t		NumBlocks; 		/* Default = 8; Possible Values: 4, 8, 12 and 16 */
	uint32_t		NumSubbands;	/* Default = 8; Possible Values: 4 and 8 */
	uint32_t		JointStereo;	/* Default = 1; 0 -> JointStereo OFF, 1 -> JointStereo ON */
	uint32_t		BitAllocation;	/* Default = 0; 0 -> Loudness, 1 -> SNR */
	uint32_t		BitPool;		/* Default = 45; Range of Values: 2 to (16 * Number of Channels * Number of Subbands) */

} BRAP_FWIF_P_SbcEncoderUserConfig;


/*
   This data structure defines AMR Decoder user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_AmrUsrCfg
{
	uint32_t 	ui32OutMode;	/* Default = 1; Output channel configuration */
	uint32_t	ui32ScaleOp;	/* Default = 0; 0 -> FALSE, 1 -> TRUE */
	uint32_t	ui32ScaleIdx;	/* Default = 0; 0 -> +3dB, 1 -> +6dB, 2 -> +9dB, 3 -> +12dB */
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

		/*
		For Multichannel (5.1) Output Port:
					ui32OutputChannelMatrix[0] = 4;
					ui32OutputChannelMatrix[1] = 4;
					
					for(i=2; i<BRAP_AF_P_MAX_CHANNELS; i++)
							ui32OutputChannelMatrix[i] = 0xFFFFFFFF; 
						
		For Stereo Output Port:
					for(i=0; i<BRAP_AF_P_MAX_CHANNELS; i++)
							ui32OutputChannelMatrix[i] = 0xFFFFFFFF;  this value shows invalid 
		*/


} BRAP_FWIF_P_AmrUsrCfg;

typedef struct  BRAP_FWIF_P_AmrConfigParams
{
	uint32_t				ui32NumOutPorts;	/* Default = 1; 1 -> Multichannel out, 2 -> Multichannel + Stereo out */

	BRAP_FWIF_P_AmrUsrCfg	sUsrOutputCfg[2];

} BRAP_FWIF_P_AmrConfigParams;




/*
   This data structure defines DRA Decoder user configuration parameters 
*/


typedef struct BRAP_FWIF_P_DraUserOutput
{
	uint32_t ui32OutMode;	/*
							Default = 2
							Range - 1, 2, 7
							1 - Downmix to mono
							2 - Downmix to stereo
							7 - Downmix to 5.1
							The decoded output is downmixed to output configuration based on ui32OutMode value*/
	uint32_t ui32OutLfe;
							/*
							Default=1
							Range - 0, 1
							0 - LFE disabled at output
							1 - LFE enabled at output*/
	uint32_t ui32StereoMode;
							/*
							Default = 0
							Range - 0, 1
							0 - LoRo downmixed output
							1 - LtRt downmixed output
							This value is relevant only when ui32OutMode is 2.*/

	uint32_t ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
						/*This matrix decides the output order of channels

							For Multi-Channel (5.1) ports:

							ui32OutputChannelMatrix[0] = 0;
							ui32OutputChannelMatrix[1] = 1;
							ui32OutputChannelMatrix[2] = 2;
							ui32OutputChannelMatrix[3] = 3;
							ui32OutputChannelMatrix[4] = 4;
							ui32OutputChannelMatrix[5] = 5;
							ui32OutputChannelMatrix[6] = 0xffffffff; this value shows invalid 
							ui32OutputChannelMatrix[7] = 0xffffffff; this value shows invalid 

							For Stereo Output Port:

									ui32OutputChannelMatrix[0] = 0;
									ui32OutputChannelMatrix[1] = 1;
									for(i=2; i<8; i++)
										ui32OutputChannelMatrix[i] = 0xffffffff; this value shows invalid 
						*/									
}BRAP_FWIF_P_DraUserOutput;


typedef struct BRAP_FWIF_P_DraConfigParams
{
	uint32_t					ui32NumOutPorts; 	/*Default = 2; 	5.1 PCM out and concurrent Stereo */		
	BRAP_FWIF_P_DraUserOutput	sUserOutputCfg[2];	

}BRAP_FWIF_P_DraConfigParams;


/*
   This data structure defines Real Audio LBR Decoder user configuration parameters 
*/

typedef struct BRAP_FWIF_P_ralbr_UserOutput
{
	uint32_t			ui32OutMode;
					/*	Default = 2
						Value according to BCOM_eACMOD 
					*/

	uint32_t			ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
					/*	This matrix decides the output order of channels

						For Multi-Channel (5.1) ports:

						ui32OutputChannelMatrix[0] = 0;
						ui32OutputChannelMatrix[1] = 1;
						ui32OutputChannelMatrix[2] = 2;
						ui32OutputChannelMatrix[3] = 3;
						ui32OutputChannelMatrix[4] = 4;
						ui32OutputChannelMatrix[5] = 5;
						ui32OutputChannelMatrix[6] = 0xffffffff; 
						this value shows invalid 
						ui32OutputChannelMatrix[7] = 0xffffffff; this value shows invalid 

					For Stereo Output Port:

						ui32OutputChannelMatrix[0] = 0;
						ui32OutputChannelMatrix[1] = 1;

						for(i=2; i<8; i++)
							ui32OutputChannelMatrix[i] = 0xffffffff; 

						this value shows invalid 
					*/			
	
}BRAP_FWIF_P_RalbrUserOutput;

typedef struct BRAP_FWIF_P_RalbrConfigParams
{
	uint32_t			ui32NumOutPorts; 

						/*Default = 2; 	5.1 PCM out and concurrent Stereo */		

	BRAP_FWIF_P_RalbrUserOutput	sUserOutputCfg[2];	

}BRAP_FWIF_P_RalbrConfigParams;

/*
   This data structure defines Dolby Pulse Decoder user configuration parameters 
*/

typedef struct BRAP_FWIF_P_DolbyPulsePortConfig
{
	/* Default = 2, Output channel configuration. 2 -> Stereo, 7 -> 3_2 Multichannel */
	uint32_t 	ui32OutMode;		

	/* Default = 0, Output LFE channel present. 0 -> LFE Off, 1 -> LFE On. */
	uint32_t 	ui32OutLfe;			

	/* Default = 0, Dual mono reproduction mode. 0 -> Stereo, 1 -> Left Only, 2 -> Right Only. */
	uint32_t 	ui32DualMode;	
	
	/* Default = 0, Downmix mode. 0 -> Lt/Rt Downmix, 1 -> Lo/Ro Downmix, 2->ARIB Downmix.
		Dolby Pulse decoder can be configured for Lt/Rt downmixing only in the single decode usage case. If MS10 mode is enabled decoder overrides this flag and always does LoRo downmix*/
	uint32_t	ui32LoRoDownmix;

	/* Default = 0, 0 -> Line Mode, 1 -> RF Mode. 5.1 Multichannel Output can only be configured to Line mode.*/
	uint32_t	ui32RfMode;

	/* Default = 100, Dynamic range compression cut scale factor in percent, 0 to 100 */
	uint32_t	ui32DrcCut;			

	/* Default = 100, Dynamic range compression boost scale factor in percent, 0 to 100 */
	uint32_t	ui32DrcBoost;		

	/* Default = 0, 0 -> FALSE, 1 -> TRUE */
	uint32_t	ui32ApplyGain;		
	
	/* Default = 0x00B504F3, corresponding to -3dB in Q8.24 format; valid only if ui32ApplyGain = 1 */
	int32_t		i32GainFactor;		

	/*
		For Multichannel (5.1) Output Port:
			for(i=0; i<6; i++)
				ui32OutputChannelMatrix[i] = i;

		for(i=6; i<BRAP_AF_P_MAX_CHANNELS; i++)
		ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID

		For Stereo Output Port:
			for(i=0; i<2; i++)
				ui32OutputChannelMatrix[i] = i;

		for(i=2; i<BRAP_AF_P_MAX_CHANNELS; i++)
			ui32OutputChannelMatrix[i] = 0xFFFFFFFF; This represents INVALID
	*/
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

	


} BRAP_FWIF_P_DolbyPulsePortConfig;


typedef struct BRAP_FWIF_P_DolbyPulseUserConfig
{
	/* BRAP_AF_P_DolbyMsUsageMode_eMpegConformanceMode Should only be enabled for MPEG Conformance testing and in this only	one 5.1 Multichannel output port should be enabled.
	*/
	BRAP_AF_P_DolbyMsUsageMode eDolbyPulseUsageMode;	

	/* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */
	BRAP_AF_P_DolbyMsDecoderType	eDecoderType;  

	/* Default = 1, 1 -> Multichannel/Stereo, 2 -> Multichannel + Stereo */
	uint32_t	ui32NumOutPorts;	

	/* Reference Dialnorm value in steps of 0.25 dB. Range: 0-127. Default: 124 */
	uint32_t	ui32RefDialnormLevel;

	/* Default Dialnorm value in steps of 0.25 dB. Range: 0-127. Default: 108 */
	uint32_t	ui32DefDialnormLevel;

	/* Decoder User Config per output port */			
	BRAP_FWIF_P_DolbyPulsePortConfig	sOutPortCfg[2];

} BRAP_FWIF_P_DolbyPulseUserConfig;

/*
   This data structure defines Audessey Vol Decoder user configuration parameters 
*/

typedef struct BRAP_FWIF_P_AudysseyVolUserConfig
{
		
    uint32_t	 ui32NumChannels;		/*	No of channels */
    uint32_t     ui32AudysseyVolBypass;	/*	Default : 0, range [0,1]	*/
										/*	0 -> Processing enable, 1-> total disable of the processing*/

    int32_t      i32ChannelMask;		/* Specifying the channels where gain is to be applied*/
										/* Default:    0xFF	All channels */
										/* Meaning of bits

											0 -> L, 1 -> R, 2 -> C, 3 -> LFE
											4 -> Ls, 5-> Rs, 6 -> Lb, 7 -> Rb
										*/

    int32_t      i32AudysseyVolInit;       
										/* Default : 0, range [0,1]*/
										/* 0 -> Do nothing, 1-> Reset(re-initialize) ADV*/
            
    int32_t      i32HeadroomOffset;		/* In q22 format. headroom_offset in dB */
										/* Default : 0, range waiting response from Audyssey Labs*/

    int32_t      i32SwAudysseyVol;		/* ADV On/Off */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on*/

    int32_t      i32SwDynEQ;			/* Dynamic EQ On/Off/Ramp off  3 modes 0, 1 and 2 */
										/* Default : 0, range [0,1,2]*/
										/* 0 -> off, 1-> on, 2 -> Ramp off */

    int32_t      i32SwDynSurrGain;		/* Switch for surround channel */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on*/
    int32_t      i32SwHiLoCmpress;		/* Switch for having boost in calc cntr gain    */
										/* Default : 1, range [0,1]*/
										/* 0 -> off, 1-> on */

    int32_t      i32dBVolSetting;		/* In q22 format for gain function and Dynamic EQ */
										/* Default : 0, float range [0, -50] */
    int32_t      i32GCF;				/* In q22 format	Gain Compression Factor */
										/* Default : 0x733333 (1.8 in float), float range [1, 2.4] */

    int32_t      i32chCalbGain[8];		/* q22 chCAlbGain in dB in floating format. Default 0 */

    int32_t      i32chCalcLevel[8];		/*   q22 chCalcLevel in dB in floating format Default 0 */
    
} BRAP_FWIF_P_AudysseyVolUserConfig;


/*
   This data structure defines Brcm 3D surround user configuration parameters 
*/

typedef struct BRAP_FWIF_P_Brcm3DSurroundConfigParams
{
	int32_t i32BRCM3DSurroundEnableFlag;	/*Default 1 Enable =1 Disable = 0*/

	int32_t i32SoftLimiterEnableFlag;		/*Default 1 Enable =1 Disable = 0*/

}BRAP_FWIF_P_Brcm3DSurroundConfigParams;


/*
   This data structure defines Mixer user configuration parameters 
*/
typedef struct BRAP_FWIF_P_MixerConfigParams
{
      
	
	uint32_t        MixingCoeffs[BRAP_AF_P_MAX_IP_FORKS][BRAP_AF_P_MAX_CHANNELS];
                                                              /* Q5.27 foremat : This field is only for sound effect mixing.
																 Separate mixing coefficients can be provided 
																 per channel per input basis.
															  */
}BRAP_FWIF_P_MixerConfigParams;

/*
   This data structure defines MS10 DD transcode  user configuration parameters 
*/
typedef struct  BRAP_FWIF_P_DDTranscodeConfigParams
{
	/* Audio Coding mode, Possible Values 0(1+1),2(2/0),7(3/2), Default: 7(3/2)*/
	int32_t  i32AudCodMode;
 
	/*  1 is LFE enabeld: 0 is LFE disabled, Default Value: 1*/
	int32_t  i32LoFreqEffOn;
 
	/* This is used to enable/disable SPDIF header.  Enable =1 Disable =0. Default value = Enable.*/
	BRAP_AF_P_EnableDisable eSpdifHeaderEnable;
 
	/*This flag is set to enable when the Encoder is used in transcode mode Enable =1 Disable =0. Default value = Enable. */
	BRAP_AF_P_EnableDisable   eTranscodeEnable;

}BRAP_FWIF_P_DDTranscodeConfigParams;

/*
   This data structure defines Mono Downmix user configuration parameters 
*/
typedef struct BRAP_FWIF_P_MonoDownMixConfigParams
{
	int32_t i32MonoDownMixEnableFlag; /*Default=1 Enable =1 Disable =0 */

}BRAP_FWIF_P_MonoDownMixConfigParams;

/*

   This data structure defines Ms10 DDP decoder user configuration parameters 
*/
 
typedef struct  BRAP_FWIF_P_DDPMs10UserOutput
{
    /* This is LFE ON/OFF flag and can take  two values 0 or 1 
     default value for this field is 0 
	*/
    int32_t i32OutLfe;                         
    /*	enum { GBL_MODE11=0, GBL_MODE_RSVD=0, GBL_MODE10, GBL_MODE20, 
		GBL_MODE30, GBL_MODE21, GBL_MODE31, GBL_MODE22, GBL_MODE32 };
		i32OutMode =7 default value;
	*/
    int32_t i32OutMode;
    /*	preferred stereo mode
		enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
		i32StereoMode = 0 is default value ;
	*/
    int32_t i32StereoMode;  
    /* dual mono downmix mode
	   enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
       i32DualMode = 0 is default value;
	*/
    int32_t i32DualMode;
    /* karaoke capable mode 
       enum { NO_VOCALS=0, GBL_VOCAL1, GBL_VOCAL2, GBL_BOTH_VOCALS };
       i32Kmode = 3;
    */
    int32_t i32Kmode;   
    /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtDnmixEnabled field is 0
	*/
    int32_t i32ExtDnmixEnabled;            
    int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS]; 
    /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtKaraokeEnabled field is 0
    */
    int32_t i32ExtKaraokeEnabled;            
    int32_t i32Extv1Level;                  
    int32_t i32Extv1Pan;                  
    int32_t i32Extv2Level;                  
    int32_t i32Extv2Pan;                  
    int32_t i32ExtGmLevel;                   
    int32_t   i32ExtGmPan;                  
    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
	  Depending upon  the channel routing done    
	*/
    uint32_t   ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];
}BRAP_FWIF_P_DDPMs10UserOutput;
 
typedef struct  BRAP_FWIF_P_DDPMs10ConfigParams
{
   /* p_user_cfg->i32CompMode = 2;*/
    int32_t                i32CompMode;  
    int32_t                i32PcmScale;  
   /* p_user_cfg->i3DynScaleHigh = 0x7FFFFFFF;    0x0 to  0x7FFFFFFF in Q1.31 format */
    int32_t                i32DynScaleHigh;    
   /* p_user_cfg->i32DynScaleLow = 0x7FFFFFFF;    0x0 to  0x7FFFFFFF in Q1.31 format */
    int32_t                i32DynScaleLow;    
   /*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
    int32_t                i32NumOutPorts;     

	/* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
	   Default is Enable
	*/
	int32_t						 i32StreamDialNormEnable;

	/*	This value indicates how far the average dialogue level is below digital 100 percent. Valid
		values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB 
		with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization, 
		Default value is 0
	*/

	int32_t						 i32UserDialNormVal;

    /* These are user config parameters required from user  */
    BRAP_FWIF_P_DDPMs10UserOutput  sUserOutputCfg[2];

    uint32_t                   ui32SubstreamIDToDecode;/* Default 0. Independent Substream ID that needs to be decoded.. The range is 0-3 for MS10*/
 
} BRAP_FWIF_P_DDPMs10ConfigParams;


/*
   This data structure defines Audyssey user configuration parameters 
*/

typedef struct BRAP_FWIF_P_ABXConfigParams
{	
    uint32_t	ui32OperationMode;	/* (Defualt 1), 1->Run ABX Processing, 0->Bypass ABX Processing*/

	uint32_t	ui32FilterSet;		/* (Default 1), 1->is for systems that roll-off lower (42/46 inch TV), 2->is for systems that roll-off higher(22 inch TV) */

    uint32_t	ui32HarmonicGain;	/* default is 6(3 dB), rangaes from 0 to 30, every step increament is equivalent to 0.5 dB step*/

    uint32_t	ui32DryGain;		/* (default is 0) typically always 0, every step increament is equivalent to 0.5 dB step */

} BRAP_FWIF_P_ABXConfigParams;



/*
   -------------------------------------------------------------------------
   This data structure defines SRS StudioSound user configuration parameters 
   -------------------------------------------------------------------------
*/

/* 
	Common Substucture for General node configuration and top level information for all 4 nodes of SRS STUDIO
	---------------------------------------------------------------------------------------------------------
*/
/*	
	i32IsStudioSound, i32StudioSoundMode and i32mEnable have to be used by all 4 nodes
	i32mInputGain, i32mHeadroomGain will be used by First node(Node0), currently SRS TruVolume 
	i32mInputMode will be used by First node(Node0) currently SRS TruVolume and Node 1(Circle Surround Decoder/Tru-Dialog)
	i32mOutputGain and i32mBypassGain will be used by Last node(Node 3), curretly Equalizer and Hard limiter
*/
typedef struct BRAP_FWIF_P_StudioSoundTopLevelUserConfig
{
    int32_t                         i32IsStudioSound;   /*To identify whether it's part of StudioSound */
                                                        /*or stand alone configuration (for TVOL and TSHD)*/
														/*This field can change only at stop start*/
                                                        /*Default : 0, Range [0, 1], others invalid, will be treated as 0*/
                                                        /* 0 -> Stand Alone configuration */
                                                        /* 1 -> Part of StudioSound */

    int32_t                         i32StudioSoundMode; /* if i32IsStudioSound == 1, this field and  will signify the mode to run. Following is the description. */
                                                        /* if i32IsStudioSound == 0, This field and all other field in  BRAP_FWIF_P_StudioSoundTopLevelUserConfig will be ignored */

                                                        /*Default : 1, Range [1, 2], others invalid, will be treated as 1*/


/*
	In STAND ALONE MODE TVOL and TSHD : 
	-----------------------------------
		(i32IsStudioSound = 0), all other fields in  BRAP_FWIF_P_StudioSoundTopLevelUserConfig will be ignored by the FW node.
*/
/*	In STUDIO SURROUND MODE
		(i32IsStudioSound = 1), Based on mode (i32StudioSoundMode), a few of the modules will be disabled by the FW internally.
								In case of conflict, i32StudioSoundMode will have  Highest priority.
		
		Different Studio Sound Modes are given Below: 

			 i32StudioSoundMode = (1) -> TVOL(TVOL On  + HPF on )                   + CSTD(On) + TSHD(On) + EQHL(GEq-Off, HL-On)   
			 i32StudioSoundMode = (2) -> TVOL(TVOL Off, bypassgain-> 1.0 + HPF on)  + CSTD(Off, bypassgain-> 1.0)+ TSHD(Off, bypassgain-> 1.0) +EQHL(GEq-On, HL-On, PEQ on) 


 
 */
    int32_t                         i32mEnable;         /*SRS StudioSound processing enable*/
                                                        /*Range [0, 1], Default : 1*/
                                                        /*	1:Enable SRS StudioSound processing, 
															0:Bypass SRS StudioSound, 
															All modules will be disabled with bypassgain 1.0
														*/  

    int32_t                         i32mInputGain;      /*SRS StudioSound Input Gain*/ 
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31    */

    int32_t                         i32mHeadroomGain;   /*SRS StudioSound Headroom Gain*/
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/

    int32_t                         i32mInputMode;      /*Channel configuration present in input*/
                                                        /*Range: [0, 1, 2, 3], Default: 3*/
                                                        /* 0 : 1_0_0
                                                            1 : 2_0_0
                                                            2 : 3_2_1
                                                            3 : LtRt
                                                        */
                                                    
    int32_t                         i32mOutputGain;     /* SRS StudioSound Output Gain */
                                                        /* Range: Floating point: (0.0 - 1.0), Default: 1.0 */
                                                        /* Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/ 

    int32_t                         i32mBypassGain;     /*SRS StudioSound Bypass Gain*/
                                                        /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                        /*Fixed point: (0x0 - 0x7fffffff), default 0x7fffffff, QFormat - 1.31*/
 

}BRAP_FWIF_P_StudioSoundTopLevelUserConfig;


/*--------------------------*/
/*HighPass Filter UserConfig*/
/*--------------------------*/
typedef struct BRAP_FWIF_P_FilterSpecHpf
{
	uint32_t  ui32CutoffFrequency;		/*Range: 20 to 1000 hz, default: 180 Hz*/
	uint32_t  ui32Order;				/*Range: [0, 2, 4, 6], default : 4*/
	
}BRAP_FWIF_P_FilterSpecHpf;

/*Coefficients should be generated by SRS_Apollo_TruEQ_HPF_Coefficients_Generator.exe*/
typedef struct BRAP_FWIF_P_FilterCoefHpfTdf2
{
	int32_t				i32Scale;				/*specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format 
												Default - 1. i32Scale=1 implies a q-format of 2.30*/
	int32_t				i32FilterCoefficientB0;
	int32_t				i32FilterCoefficientB1;
	int32_t				i32FilterCoefficientB2;

	int32_t				i32FilterCoefficientA1;
	int32_t				i32FilterCoefficientA2;
	
}BRAP_FWIF_P_FilterCoefHpfTdf2;

typedef struct BRAP_FWIF_P_FilterCoefHpf
{
	uint32_t			ui32Order;/*HighPass Filter order. Range: [2, 4, 6] */

	BRAP_FWIF_P_FilterCoefHpfTdf2	sFilterCoefHpfTdf2[3];	/*number of active biquads filter decided by i32Order*/
	
}BRAP_FWIF_P_FilterCoefHpf;

typedef struct BRAP_FWIF_P_HighPassFilterUserConfig
{
    uint32_t     ui32mEnable;        /*Highpass filter enable, Range: [0, 1], Default - 1 */
                                                    /*0 - Disable HPF Processing, 1 - Enable HPF Processing*/

    uint32_t     ui32CoefGenMode;     /*Range - [0, 1], Default -> 0
										0 - > use the coefficients provided by user, in field sFilterCoefHpf[]
										1 - > use filter setting provided by user in BRAP_FWIF_P_FilterSpecHpf[] to generate coefficients,
													  will overwrite the coefficients in sFilterCoefHpf[][]*/
	union
	{
	BRAP_FWIF_P_FilterCoefHpf 	sFilterCoefHpf[3];		/* Array size 3, representing supported sampling rates 32, 44.1 and 48kHz, will be used if i32CoefGenMode==0*/
	BRAP_FWIF_P_FilterSpecHpf	sFilterSpecHpf;			/* Filter specification for generating filter coeffcients, will be used if i32CoefGenMode==1 */
	}uHPFSettings;
	
}BRAP_FWIF_P_HighPassFilterUserConfig;

/*Default values of HighPass filter coefficients*/

/* i32mEnable=1*/

/*Sampling frequency - 32 kHz*/
/*---------------------------*/
/*
sFilterCoefHpf[0].i32Order = 0x00000004
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fc81f80
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe06fc100
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fc81f80
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7e36e680
sFilterCoefHpf[0].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc1b4eec0

sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3df563c0
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x84153880
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3df563c0
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7be0e200
sFilterCoefHpf[0].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc40b5300
*/
/*Sampling frequency - 44.1 kHz*/
/*-----------------------------*/
/*
sFilterCoefHpf[1].i32Order = 0x00000004
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fd78db0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe050e4a0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fd78db0
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7eb71980
sFilterCoefHpf[1].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc13e3e40

sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3e826c40
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x82fb2780
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3e826c40
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7cff9680
sFilterCoefHpf[1].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc2f5e600
*/

/*Sampling frequency - 48 kHz*/
/*---------------------------*/
/*
sFilterCoefHpf[2].i32Order = 0x00000004
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32Scale = 0x00000001
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB0 = 0x0fdadc10
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB1 = 0xe04a47e0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientB2 = 0x0fdadc10
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientA1 = 0x7ed26000
sFilterCoefHpf[2].sFilterCoefHpfTdf2[0].i32FilterCoefficientA2 = 0xc1249f40

sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32Scale = 0x00000001
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB0 = 0x3ea0f4c0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB1 = 0x82be1680
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientB2 = 0x3ea0f4c0
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientA1 = 0x7d3d7780
sFilterCoefHpf[2].sFilterCoefHpfTdf2[1].i32FilterCoefficientA2 = 0xc2b9a440
*/

/*3rd biquad need not be initialized as filter order is 4*/

/*Default HighPass filter specification - 
	i32CutoffFrequency=180 Hz;
	i32Order=4*/


/*
	This data structure defines TruVolume configuration parameters.
	---------------------------------------------------------------
	
	TVOL can be a standalone Post processing module or it can be one of the nodes in SRS studio
	The selection of this mode is determined by the i32IsStudioSound field.
*/

typedef struct BRAP_FWIF_P_TruVolumeUserConfig
{
    int32_t                     i32TruVolume_enable;        /* Top level disable, no processing will happen at all when 0*/
                                                            /* Deafault : 1, range [0,1]*/
                                                            /* 1 -> processing enable, 0-> total disable of the processing*/

    int32_t                     i32nchans;                  /* Number of input/output channels     */
                                                            /* Default val 2*/

    int32_t                     i32blockSize;               /* Default: 256, range [256, 512, 768, 1024]*/      

    int32_t                     i32mEnable;                 /* Default: 1;  1 -> Process, 0 ->  Bypass */

    int32_t                     i32mInputGain;              /* Input gain */
                                                            /* Default: 0x00200000 range[0, 0x03ffffff]
															  Floating val: default 1: range [0, 32]*/

    int32_t                     i32mOutputGain;             /* Output gain */
                                                            /* Default: 0x00200000  range[0, 0x007fffff]
 Floating val: default 1: range [0, 4]*/

    int32_t                     i32mBypassGain;             /* Bypass gain */
                                                            /* default: 0x007fffff range[0, 0x007fffff]
 Floating val: default 1: range [0, 1] */

    int32_t                     i32mReferenceLevel;         /* Reference Level */
                                                            /* Default: 0x00400000 range[0x00000109, 0x007fffff]
 Floating val: default 0.5: range [3.1623 * 10^-5, 1] */

    int32_t                     i32EnableDCNotchFilter;     /* Enable DC Notch filter*/
                                                            /* Default: 0 range[0 -> disble, 1 -> enable]*/

    int32_t                     i32mMode;                   /* Default: 1 range[0,1] 1-> kSrsLight, 0-> kSrsNormal*/

    int32_t                     i32mSize;                   /* Default: 0 range[0,1,2,3,4, 5]*/
                                                            /*
 0 -> VIQ_kSrs20Hz,
 1 -> kSrs40Hz,
 2 -> kSrs110Hz,
 3 -> kSrs200Hz,
 4 -> kSrs315Hz,
 5 -> kSrs410Hz*/

    int32_t                     i32mMaxGain;                 /* Max Gain Control */
                                                             /* Default: 0x00080000 rnage[0x00002000, 0x007fffff]*/
                                                             /* Float default: 16.0 rnage[.25, 256.0], conversion formula: (f_val/256)*2^23 */


    int32_t                     i32mNoiseManager;           /* Default: 1; 1 -> Noise manager enable, 0 -> disable */
    int32_t                     i32mNoiseManagerThreshold;  /* Default: 0x000ccccd, range [0x0000a3d7, 0x007fffff] */
                                                            /* Float Default: 0.1 range[0.005, 1.0], conversion formula: f_val*2^23 */

    int32_t                     i32mCalibrate;              /* Default: 0x8000, range [0x00000148, 0x007fffff] */
                                                            /* Float Default: 1.0 range[0.01, 256.0]*/
                                
    int32_t                     i32mNormalizerEnable;       /* Default: 1;  1 -> Normalizer Enable, 0   ->  Normalizer Disable */


    BRAP_FWIF_P_StudioSoundTopLevelUserConfig   sTopLevelConfig;
    BRAP_FWIF_P_HighPassFilterUserConfig        sHighPassFilterConfig;

}BRAP_FWIF_P_TruVolumeUserConfig;


/*
	Node1 of SRS STUDIO: This is a combination of SRS-Tru-Dialog and SRS Circle Surround.
	-------------------------------------------------------------------------------------

	The user configuration of Node1 is BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams

	User CFG structure for Circle Surround Decoder/Tru-Dialog are separately defined as below 
*/

/*TruDialog user config*/
typedef struct BRAP_FWIF_P_TruDialogUserConfig
{
    int32_t         i32mEnable;                     /*Enable TruDialog, Range [0, 1], Default : 1*/
                                                    /*1:Enable TruDialog, 0:Bypass TruDialog*/  


    int32_t         i32mTruDialogProcessGain;       /*Amount of final calculated gain applied to final  output signal*/
                                                    /*Range: Floating point: (0.5 - 3.0), Default: 1.5*/
                                                    /*Fixed point: (0x04000000 - 0x18000000), default: 0x0c000000,  QFormat - 5.27*/  

    
    int32_t         i32mTruDialogInputGain;         /*TruDialog Input Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogOutputGain;        /*TruDialog Output Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogDialogClarityGain; /*Amount of Dialog Clarity enhancement applied to input signal*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mTruDialogBypassGain;        /*TruDialog Bypass Gain*/
                                                    /*Range: Floating point: (0.0 - 1.0), Default: 1.0*/    
                                                    /*Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/
}BRAP_FWIF_P_TruDialogUserConfig;

/*CSDecoder user config*/
typedef struct BRAP_FWIF_P_CSDecoderUserConfig
{
    
    int32_t         i32mEnable;                     /*Enable CS Decoding*/
                                                    /*Range [0, 1], Default : 1*/
                                                    /*1:Enable CS Decoding, 0:Bypass CS Decoding*/  

    int32_t         i32mInputGain;                  /*CS Decoder Input Gain */
                                                    /*Range: Floating point: (0.12589 - 1.0), Default: 1.0*/
                                                    /*In dB       : (-18dB - 0dB) , Default: 0 dB*/
                                                    /*Fixed point : (0x1020c49c - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t         i32mMode;                       /*CS Decoder processing mode*/
                                                    /*Range [1, 4], Default : 1, [1:Cinema 4:Music]*/
    
    int32_t         i32mOutputMode;                 /*Output mode Range: [0, 1]. Only supported mode is 0 Default : 0*/
                                                    /*0 - Stereo output mode, 1 - Multi channel output mode*/

    int32_t         i32mCSDecOutputGainLR;          /*CS Decoder output gain for LR channels*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/   
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/

    int32_t         i32mCSDecOutputGainLsRs;        /*CS Decoder output gain for LsRs channels*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/   
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/
    
    int32_t         i32mCSDecOutputGainC;           /*CS Decoder output gain for Center channel*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/   
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/

    int32_t         i32mCSDecOutputGainSub;         /*CS Decoder output gain for Subwoofer channel*/
                                                    /*Range: Floating point (0.089 to 4.0), Default: 1.0*/
                                                    /*Range: Fixed point (0x016c8b44 to 0x40000000), default: 0x10000000, QFormat: 4.28*/   
                                                    /*Range: in decibels (-21dB to 12dB), default: 0dB*/
}BRAP_FWIF_P_CSDecoderUserConfig;

/* Top level User Cfg structure for Node1 of SRS studio*/

typedef struct BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams
{
    BRAP_FWIF_P_StudioSoundTopLevelUserConfig   sTopLevelConfig;
    BRAP_FWIF_P_TruDialogUserConfig             sTruDialogConfig;
    BRAP_FWIF_P_CSDecoderUserConfig             sCSDecoderConfig;

}BRAP_FWIF_P_SRS_CSDecTruDialogConfigParams;


/*
	Node2 of SRS STUDIO: SRS TS-HD.
	-------------------------------------------------------------------------------------

	The user configuration of Node2 is BRAP_FWIF_P_TruSurrndHDConfigParams	

	SRS TS-HD can be a standalone Post processing module or it can be one of the nodes in SRS studio
	The selection of this mode is determined by the i32IsStudioSound field.
*/

typedef enum BRAP_FWIF_P_SRSTruBassProcessMode
{
    BRAP_FWIF_P_SRSTruBassProcessMode_eMono,
    BRAP_FWIF_P_SRSTruBassProcessMode_eStereo,
    BRAP_FWIF_P_SRSTruBassProcessMode_eLast,
    BRAP_FWIF_P_SRSTruBassProcessMode_eInvalid = 0x7fffffff

} BRAP_FWIF_P_SRSTruBassProcessMode;
 
typedef enum BRAP_FWIF_P_SRS3DMode
{
    BRAP_FWIF_P_SRS3DMode_eMono,
    BRAP_FWIF_P_SRS3DMode_eSingleSpeaker,
    BRAP_FWIF_P_SRS3DMode_eStereo,
    BRAP_FWIF_P_SRS3DMode_eExtreme,
    BRAP_FWIF_P_SRS3DMode_eLast,
    BRAP_FWIF_P_SRS3DMode_eInvalid = 0x7fffffff

} BRAP_FWIF_P_SRS3DMode;

typedef enum
{
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency_e80Hz = 0,
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency_e120Hz,
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency_e160Hz,
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency_e200Hz,
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency_eMax,
	BRAP_FWIF_P_SRSTSHDCrossoverFrequency_eInvalid =0x7fffffff

} BRAP_FWIF_P_SRSTSHDCrossoverFrequency;

typedef struct BRAP_FWIF_P_TruSurrndHDConfigParams
{
    int32_t     i32TruSurrndHDEnableFlag;   /*Default 1 Enable*/
    int32_t     i32HeadPhoneEnableFlag;     /*Default 0 disable*/
    int32_t     i32TruSurrndHDInputGain;    /*Range 0 to 1.0 in Q31 format 
                                            Default 0x40000000*/
    int32_t     i32TruSurrndHDOutputGain;   /*Range 0 to 1.0 in Q31 format 
                                            Default 0x7fffffff*/
    int32_t     i32TruSurrndHDByPassGain;   /*Range 0 to 1.0 in Q31 format 
                                            Default 0x7FFFFFFF*/
    BRAP_FWIF_P_SRSTSHDCrossoverFrequency eSubCrossOverFreq; /*Used only for non Apollo mode
                                            Default BRAP_FWIF_P_SRSTSHDCrossoverFrequency_e200Hz*/

    int32_t     i32TruBassFrontEnableFlag;  /*Default 1 Enable*/
    int32_t     i32TruBassFrontCtrl;        /*Range 0 to 1.0 in Q31 format
                                            Default 0x26666666*/

    BRAP_FWIF_P_SpeakerSize eTruBassFrontSpeakerSize; /*Default BRAP_FWIF_P_SpeakerSize_eLFResponse150Hz
                                            with a value of 3*/
    BRAP_FWIF_P_SRSTruBassProcessMode   eSRSTruBassProcessMode; /*Default to stereo mode*/

    int32_t     i32DefinitionFrontEnableFlag;   /*Default 1 Enable*/
    int32_t     i32DefinitionFrontCtrl;     /*Range 0 to 1.0 in Q31 format
                                            Default 0x26666666*/


    int32_t     i32DialogClarityEnableFlag; /*Default 1 Enable*/
    int32_t     i32DialogClarityCtrl;       /*Range 0 to 1.0 in Q31 format
                                            Default 0x40000000*/

    int32_t     i32SurroundLevel;           /*Range 0 to 1.0 in Q31 format
                                            Default 4CCCCCCD*/

    int32_t     i32WowHDSRS3DEnableFlag;    /*Default 1 Enable*/
    int32_t     i32SRS3DHighBitRateFlag;    /*Default 1 and sets to High Bit Rate*/
    BRAP_FWIF_P_SRS3DMode   eWowHDSRS3DMode;/*Default BRAP_FWIF_P_SRS3DMode_eStereo with value 2*/
    int32_t     i32WowHDSRS3DSpaceCtrl;     /*Range 0 to 1.0 in Q31 format
                                            Default 0x66666666*/
    int32_t     i32WowHDSRS3DCenterCtrl;    /*Range 0 to 1.0
                                            Default 0x40000000*/
    int32_t     i32WowHDFocusEnableFlag;    /*Default 1 Enable*/
    int32_t     i32WowHDFocusCtrl;          /*Range 0 to 1.0 in Q31 format
                                            Default 0x33333333*/

    int32_t     i32Mono2StereoEnableFlag;


    BRAP_FWIF_P_AcMode  eOutputAcMode;      /*Default SRSTruSurroundHDOutputMode_e2_0_0*/
    int32_t     i32OuputLFEEnableFlag;      /*Used only for TSHD4.  Not used for Studio Sound*/

    int32_t     i32CertificationEnableFlag; /*Default 0 Disable*/
    BRAP_FWIF_P_AcMode  eInputAcMode;       /*Used only during certification*/
    int32_t     i32InputLFEPresentFlag;     /*Used only during certification*/

    BRAP_FWIF_P_StudioSoundTopLevelUserConfig           sTopLevelConfig;

}BRAP_FWIF_P_TruSurrndHDConfigParams;



/*
	Node3 of SRS STUDIO: This is a combination of ParamtericEQ, Graphic equalizer and Hard Limiter.
	-----------------------------------------------------------------------------------------------

	The user configuration of Node3 is BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams

	User CFG structure for ParamtericEQ, Graphic equalizer and Hard Limiter are separately defined as below 
*/

/*------------------------*/
/*ParamtericEQ user config*/
/*------------------------*/
typedef struct  BRAP_FWIF_P_FilterSpecPeq
{
	int32_t i32BandGain;		/*Range in dB: -12 to 12 db
										Format - q24
										Range in Fixed point: 0x00404DE6 to 0x03FB2783
										Default: 0dB
										Default in fixed point: 0x01000000*/

	uint32_t ui32BandFrequency;	/*Range: 20 to 20000 Hz
								  Default: 1000 Hz*/

	int32_t i32QFactor;			/*Range in floating point: 0.25 to 16.0 
									  Format: q24
									  Range, fixed point: 0x00400000 to 0x10000000
								  Default in floating point: 2.0
								  Default in fixed point: 0x02000000

								*/
}BRAP_FWIF_P_FilterSpecPeq;

/*Coefficients should be generated by SRS_Apollo_TruEQ_HPF_Coefficients_Generator.exe*/
typedef struct
{
	int32_t				i32Order;	/*ParametricEQ filter order*/
	int32_t				i32Scale;	/*specified q-format of the coefficients. (1+i32Scale).(31-i32Scale) format 
										Default - 1. i32Scale=1 implies a q-format of 2.30*/
	int32_t				i32FilterCoefficientB0;
	int32_t				i32FilterCoefficientA1;
	int32_t				i32FilterCoefficientB1;
	int32_t				i32FilterCoefficientA2;
	int32_t				i32FilterCoefficientB2;
	
}BRAP_FWIF_P_FilterCoefPeq;


typedef struct BRAP_FWIF_P_ParametricEqUserConfig
{
    int32_t i32mEnable[2];                          /*Left/Right channel ParametricEQ Processing Enable*/ 
                                                    /*Range: [0,1], Default: i32mEnable[0] = 1, i32mEnable[1] = 1*/
                                                    /*0 - Disable PEQ processing, 1 - Enable PEQ processing*/

    int32_t i32mBandEnable[2][8];                   /*Turn ON/OFF a specific band on one or both channels.*/
                                                    /*Range: [0,1], Default: 1 for all bands*/
                                                    /*0 - Disable band, 1 - Enable band*/

    int32_t i32mInputGain;                          /*Input gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t i32mOutputGain;                         /*Output gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t i32mBypassGain;                         /*Bypass gain*/
                                                    /*Range: Floating point: [0.0 to 10.0], Default: 1.0*/
                                                    /*Fixed point   : [0x0 to 0x50000000], Default: 0x08000000, QFormat - 5.27*/

    int32_t     i32CoefGenMode;                     /*Range - [0, 1], Default -> 0
                                                      0 - > use the coefficients provided by user, in field sFilterCoefPeq[]
                                                      1 - > use filter setting provided by user in sFilterSpecPeq[] to generate coefficients,
													  will overwrite the coefficients in sFilterCoefPeq[][]*/

	union
	{
	BRAP_FWIF_P_FilterCoefPeq	sFilterCoefPeq[3][8];			/*array size 3x8 implying 3 sampling frequencies with 8 bands each, will be used if i32CoefGenMode==0*/
	BRAP_FWIF_P_FilterSpecPeq	sFilterSpecPeq[8];				/* Filter specification for generating filter coeffcients, will be used if i32CoefGenMode==1*/			
	}uPEQFilterSettings;
}BRAP_FWIF_P_ParametricEqUserConfig;

/*Default filter specification - Gain: 0dB, Frequency: 1000Hz, Q: 2 for all bands*/

/*Default ParametricEQ Filter coefficients*/

/*Sampling Rate - 32 kHz*/
/*
//band 0
sFilterCoefPeq[0][0].i32Order = 0x00000002
sFilterCoefPeq[0][0].i32Scale = 0x00000001
sFilterCoefPeq[0][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][0].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][0].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][0].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][0].i32FilterCoefficientB2 = 0x3bf14a80


//band 1
sFilterCoefPeq[0][1].i32Order = 0x00000002
sFilterCoefPeq[0][1].i32Scale = 0x00000001
sFilterCoefPeq[0][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][1].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][1].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][1].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][1].i32FilterCoefficientB2 = 0x3bf14a80


//band 2
sFilterCoefPeq[0][2].i32Order = 0x00000002
sFilterCoefPeq[0][2].i32Scale = 0x00000001
sFilterCoefPeq[0][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][2].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][2].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][2].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][2].i32FilterCoefficientB2 = 0x3bf14a80


//band 3
sFilterCoefPeq[0][3].i32Order = 0x00000002
sFilterCoefPeq[0][3].i32Scale = 0x00000001
sFilterCoefPeq[0][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][3].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][3].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][3].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][3].i32FilterCoefficientB2 = 0x3bf14a80


//band 4
sFilterCoefPeq[0][4].i32Order = 0x00000002
sFilterCoefPeq[0][4].i32Scale = 0x00000001
sFilterCoefPeq[0][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][4].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][4].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][4].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][4].i32FilterCoefficientB2 = 0x3bf14a80

//band 5
sFilterCoefPeq[0][5].i32Order = 0x00000002
sFilterCoefPeq[0][5].i32Scale = 0x00000001
sFilterCoefPeq[0][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][5].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][5].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][5].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][5].i32FilterCoefficientB2 = 0x3bf14a80

//band 6
sFilterCoefPeq[0][6].i32Order = 0x00000002
sFilterCoefPeq[0][6].i32Scale = 0x00000001
sFilterCoefPeq[0][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][6].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][6].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][6].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][6].i32FilterCoefficientB2 = 0x3bf14a80

//band 7
sFilterCoefPeq[0][7].i32Order = 0x00000002
sFilterCoefPeq[0][7].i32Scale = 0x00000001
sFilterCoefPeq[0][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[0][7].i32FilterCoefficientA1 = 0x798f9f00
sFilterCoefPeq[0][7].i32FilterCoefficientB1 = 0x86706100
sFilterCoefPeq[0][7].i32FilterCoefficientA2 = 0xc40eb580
sFilterCoefPeq[0][7].i32FilterCoefficientB2 = 0x3bf14a80
*/

/*Sampling Rate - 44.1 kHz*/
/*band 0*/
/*
sFilterCoefPeq[1][0].i32Order = 0x00000002
sFilterCoefPeq[1][0].i32Scale = 0x00000001
sFilterCoefPeq[1][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][0].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][0].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][0].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][0].i32FilterCoefficientB2 = 0x3d07cd00

//band 1
sFilterCoefPeq[1][1].i32Order = 0x00000002
sFilterCoefPeq[1][1].i32Scale = 0x00000001
sFilterCoefPeq[1][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][1].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][1].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][1].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][1].i32FilterCoefficientB2 = 0x3d07cd00

//band 2
sFilterCoefPeq[1][2].i32Order = 0x00000002
sFilterCoefPeq[1][2].i32Scale = 0x00000001
sFilterCoefPeq[1][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][2].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][2].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][2].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][2].i32FilterCoefficientB2 = 0x3d07cd00

//band 3
sFilterCoefPeq[1][3].i32Order = 0x00000002
sFilterCoefPeq[1][3].i32Scale = 0x00000001
sFilterCoefPeq[1][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][3].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][3].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][3].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][3].i32FilterCoefficientB2 = 0x3d07cd00

//band 4
sFilterCoefPeq[1][4].i32Order = 0x00000002
sFilterCoefPeq[1][4].i32Scale = 0x00000001
sFilterCoefPeq[1][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][4].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][4].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][4].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][4].i32FilterCoefficientB2 = 0x3d07cd00

//band 5
sFilterCoefPeq[1][5].i32Order = 0x00000002
sFilterCoefPeq[1][5].i32Scale = 0x00000001
sFilterCoefPeq[1][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][5].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][5].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][5].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][5].i32FilterCoefficientB2 = 0x3d07cd00

//band 6
sFilterCoefPeq[1][6].i32Order = 0x00000002
sFilterCoefPeq[1][6].i32Scale = 0x00000001
sFilterCoefPeq[1][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][6].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][6].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][6].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][6].i32FilterCoefficientB2 = 0x3d07cd00

//band 7
sFilterCoefPeq[1][7].i32Order = 0x00000002
sFilterCoefPeq[1][7].i32Scale = 0x00000001
sFilterCoefPeq[1][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[1][7].i32FilterCoefficientA1 = 0x7bc37b00
sFilterCoefPeq[1][7].i32FilterCoefficientB1 = 0x843c8500
sFilterCoefPeq[1][7].i32FilterCoefficientA2 = 0xc2f83300
sFilterCoefPeq[1][7].i32FilterCoefficientB2 = 0x3d07cd00
*/

/*Sampling Rate - 48 kHz*/
/*
//band 0
sFilterCoefPeq[2][0].i32Order = 0x00000002
sFilterCoefPeq[2][0].i32Scale = 0x00000001
sFilterCoefPeq[2][0].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][0].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][0].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][0].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][0].i32FilterCoefficientB2 = 0x3d444480

//band 1
sFilterCoefPeq[2][1].i32Order = 0x00000002
sFilterCoefPeq[2][1].i32Scale = 0x00000001
sFilterCoefPeq[2][1].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][1].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][1].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][1].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][1].i32FilterCoefficientB2 = 0x3d444480

//band 2
sFilterCoefPeq[2][2].i32Order = 0x00000002
sFilterCoefPeq[2][2].i32Scale = 0x00000001
sFilterCoefPeq[2][2].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][2].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][2].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][2].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][2].i32FilterCoefficientB2 = 0x3d444480

//band 3
sFilterCoefPeq[2][3].i32Order = 0x00000002
sFilterCoefPeq[2][3].i32Scale = 0x00000001
sFilterCoefPeq[2][3].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][3].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][3].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][3].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][3].i32FilterCoefficientB2 = 0x3d444480

//band 4
sFilterCoefPeq[2][4].i32Order = 0x00000002
sFilterCoefPeq[2][4].i32Scale = 0x00000001
sFilterCoefPeq[2][4].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][4].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][4].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][4].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][4].i32FilterCoefficientB2 = 0x3d444480

//band 5
sFilterCoefPeq[2][5].i32Order = 0x00000002
sFilterCoefPeq[2][5].i32Scale = 0x00000001
sFilterCoefPeq[2][5].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][5].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][5].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][5].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][5].i32FilterCoefficientB2 = 0x3d444480

//band 6
sFilterCoefPeq[2][6].i32Order = 0x00000002
sFilterCoefPeq[2][6].i32Scale = 0x00000001
sFilterCoefPeq[2][6].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][6].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][6].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][6].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][6].i32FilterCoefficientB2 = 0x3d444480

//band 7
sFilterCoefPeq[2][7].i32Order = 0x00000002
sFilterCoefPeq[2][7].i32Scale = 0x00000001
sFilterCoefPeq[2][7].i32FilterCoefficientB0 = 0x40000000
sFilterCoefPeq[2][7].i32FilterCoefficientA1 = 0x7c31eb80
sFilterCoefPeq[2][7].i32FilterCoefficientB1 = 0x83ce1480
sFilterCoefPeq[2][7].i32FilterCoefficientA2 = 0xc2bbbb80
sFilterCoefPeq[2][7].i32FilterCoefficientB2 = 0x3d444480
*/


/*GraphicEQ user config*/
typedef struct BRAP_FWIF_P_GraphicEqUserConfig
{
    
    int32_t i32mEnable[2];                          /*Left/Right channel GraphicEQ Processing Enable */
                                                    /*Range: [0,1], Default: i32mEnable[0] = 1, i32mEnable[1] = 1*/
                                                    /*0 - Disable GEQ processing, 1 - Enable GEQ processing*/

    int32_t i32mBandGain[2][10];                    /*Band gain for a specific band for one or both channels.*/
                                                    /*MAX_GEQ_BANDS=10*/

                                                    /*Range:*/
                                                        /*For 10 band design*/
                                                        /*Floating point: (0.251 to 4.0), Default: 1.0*/
                                                        /*In dB         : [-12dB to +12dB], Default: 0dB*/
                                                        /*Fixed point   : [0x08083127 to 0x7fffffff), Default: 0x20000000, QFormat : 3.29*/

                                                        /*For 5 band design*/
                                                        /*Floating point: (0.316 to 3.16), Default: 1.0*/
                                                        /*In dB         : [-10dB to +10dB], Default: 0dB*/
                                                        /*Fixed point   : [0x0a1cac08 to 0x651eb852), Default: 0x20000000, QFormat : 3.29*/


    int32_t i32mInputGain;                          /*Input gain applied to both channels prior to GEQ processing*/
                                                    /*Range:
                                                        For 10 band design
                                                        Floating point: (0 to 1.0), Default: 0.251
                                                        In dB         : [-ve infinity to 0dB], Default: -12dB
                                                        Fixed point   : [ 0 to 0x7fffffff), Default: 0x2020c49c
                                                        QFormat : 1.31


                                                        For 5 band design
                                                        Floating point: : (0 to 1.0), Default: 0.316
                                                        In dB         : [-ve infinity to 0dB],, Default: -10dB
                                                        Fixed point   :[ 0 to 0x7fffffff) Default: 0x2872b021, QFormat : 1.31*/


    int32_t i32mOutputGain;                         /*Output Gain*/
                                                    /*Range: 
                                                        Floating point: (0.0 - 1.0), Default: 1.0
                                                        Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t i32mBypassGain;                         /*Bypass Gain
                                                      Range: 
                                                        Floating point: (0.0 - 1.0), Default: 1.0
                                                        Fixed point: (0x0 - 0x7fffffff), default: 0x7fffffff, QFormat - 1.31*/

    int32_t  i32mFilterMode;                        /*GEQ filter configuration                                  Range: [0, 1, 2], Default: 0
                                                        0: five band config, 1: 10 band config, 2: Speaker compensation config
                                                    */

}BRAP_FWIF_P_GraphicEqUserConfig;

/*Hard limiter user config*/
typedef struct BRAP_FWIF_P_HardLimiterUserConfig
{
    int32_t             i32LimiterEnable;           /* Limiter enable separate control */
                                                    /* Default: 1;  1 -> Process, 0 ->  Disable of processing */

    int32_t             i32nchans;                  /* Number of input/output channels     */
                                                    /* Default val 2*/
    
    int32_t             i32blockSize;               /* Default: 256, range [256, 512, 768, 1024]*/      
    
    int32_t             i32InputGain;               /* Input gain */
                                                    /* Default: 0x20000000 range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 4] */
    
    int32_t             i32OutputGain;              /* Output gain */
                                                    /* Default: 0x20000000 range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 4] */
    
    int32_t             i32BypassGain;              /* Bypass gain */
                                                    /* Default: 0x7fffffff range[0, 0x7fffffff]
                                                       Floating val: default 1: range [0, 1] */
    
    int32_t             i32LimiterBoost;            /* HL gain boost */
                                                    /* Default: 0x0168f5c3 range[0x000020c5, 0x7fffffff]
                                                       Floating val: default 2.82: range [0.001, 256] */
    
    int32_t             i32HardLimit;               /* HardLimit */
                                                    /* Default: 0x7fffffff range[0x00010945, 0x7fffffff]
                                                       Floating val: default 1: range [3.16227766e-5, 1] */
    
    int32_t             i32DelayLength;             /* DelayLength */
                                                    /* Default: 22 range[6, 22]
                                                       Floating val: default 22: range [6, 22] */           
}BRAP_FWIF_P_HardLimiterUserConfig;

/*
Top Level Data structure for Node 3 of SRS Studio
*/
typedef struct BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams
{
    BRAP_FWIF_P_StudioSoundTopLevelUserConfig       sTopLevelConfig;
    BRAP_FWIF_P_ParametricEqUserConfig              sParametricEqConfig;    
    BRAP_FWIF_P_HighPassFilterUserConfig            sHighPassFilterConfig;
    BRAP_FWIF_P_GraphicEqUserConfig                 sGraphicEqConfig;
    BRAP_FWIF_P_HardLimiterUserConfig               sHardLimiterConfig; 

}BRAP_FWIF_P_SRS_EqualizerHardLimiterConfigParams;

/*
   This data structure defines DDRE user configuration parameters 
*/

typedef struct  BRAP_FWIF_P_DDReencodeUserConfig
{

	uint32_t		ui32CompMode;	/*	Compression mode setting,Possible values: 1(RF mode) and 0(Line mode).
										Default: 0(Line mode). Multichannel output can only configured for 
										Line mode in the  current reference implementation*/

	uint32_t		ui32DrcCutFac;	 /* DRC Cut Scale factor in 1.31 Format. Possble Values: 0 to 0x7FFFFFFF. 
										Default value: 0x7FFFFFFF */

	uint32_t		ui32DrcBoostFac; /* DRC Boost Scale factor in 1.31 Format. Possble Values: 0 to 0x7FFFFFFF.
										Default value: 0x7FFFFFFF */

	uint32_t 		ui32OutMode;	/*	Output channel configuration.Possible Values: 2(Stereo) or 7(3/2 Multichannel), 
										Default = 2(Stereo?). None of the other possible ACMODS are allowed. */

	uint32_t 		ui32OutLfe;		/*	Output LFE channel present .Possible Values: 0: LFE disabled, 1: LFE enabled, 
										Default = 0(Disabled). Can only enabled with 3/2 Multichannel audio*/

	uint32_t		ui32StereoMode; /*	Flag indicating the Stereo downmixed mode, is only effective if Outputmode is Stereo. 
										Possible Values: 0(LtRt), 1(LoRo) and 2(Arib). Default value: 0(LtRt) */ 

	uint32_t 		ui32DualMode;	/*	Flag indicating the dualmono mode which is to be used if the incoming stream is dual mono. 
										Possible values: 0(Stereo), 1(Left_Only) and 2(Right_Only). Default value: 0(Stereo). */


	uint32_t		ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS]; /*This is channel matrix of size 8 where each index can take any values from 0 to 7 depending upon  the channel routing done*/

} BRAP_FWIF_P_DDReencodeUserConfig;


typedef struct BRAP_FWIF_P_DDReencodeConfigParams
{
	/* Flag indicating the DDRE is in certification mode. Possible Values: 0 (disable), 1(Enable) Default value: 0 (disable) */ 

	uint32_t 		ui32DdreCertificationModeActive;
    
	/* Flag indicating that external Pcm mode is active. This flag is to be set active if input is not from the Dolby MS11 decoders(DDP and Dolby Pulse */
	uint32_t    	ui32ExternalPcmEnabled;	
	
	/* Compression profiles applicable. The Settings are only if the input is from a External PCM source.
	Possible values: 
	0 : "No Compression"
	1 : "Film Standard Compression"
	2 : "Film Light Compression"
    3 : "Music Standard Compression"
	4 : "Music Light Compression"
    5 : "Speech Compression"	
	Default Value: 1: "Film Standard Compression"*/	
	uint32_t    	ui32CompProfile;
	
	/*Index for center level in the downmixing case. Possible Values: 0-3. Default: 0. Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled*/
	uint32_t   ui32CmixLev;

	/*Index for surround level in the downmixing case. Possible Values: 0-3. Default: 0. Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled */
	uint32_t   ui32SurmixLev;

	/* Dolby Surround Encoding Mode.
		Possible Values:
		0: NOT INDICATED
		1: NOT SURROUND ENCODED
		2: SURROUND ENCODED
		Default: 0(NOT INDICATED).
		Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled 
	*/
	uint32_t   ui32DsurMod;
	
	/* Dialog level of incoming PCM in dB. Possible values: 0-31. Default: 31 Used only if External PCM Mode is enabled or PCM Soundeffect Mixing is the only context enabled */
	uint32_t	 ui32DialNorm;

	
	/* Total number of output ports enabled. If only stereo output port is set to enabled then the value should be 1, if Multichannel 5.1 PCM output is enabled the value should be 2. Only 1 output port with Multichannel 5.1 PCM output port is NOT allowed */
	uint32_t		ui32NumOutPorts;		

	/* The Settings are valid only if the Certifiction mode is active. Audio coding mode as defined for Dolby Digital. The audio coding mode associated with the output from the Dolby Pulse Decoder-Transcoder
	can be DD_ONEPLUSONE=0, DD_MONO=1, DD_STEREO=2, DD_3_0=3, DD_2_1=4, DD_3_1=5, DD_2_2=6 or DD_3_2=7.*/

	uint32_t 		ui32InputStreamAcmod;

	/* The Settings are only if the Certifiction mode is active. Flag indicating presence of the low frequency enhancement channel. Possible Values: 0 (Not Present), 1(Present) Default value: 0(Not Present)*/

	uint32_t 		ui32LfePresent;

	/* The Settings are only if the Certifiction mode is active. Flag indicating that input has been DV-258 processed  Possible Values: 0 (Disable), 1(Enable) Default value: 0(Disable) */

	uint32_t 		ui32Dv258ProcessedInput;
    
	
	/* Per Output port configuration for DDRE */
	BRAP_FWIF_P_DDReencodeUserConfig		sUserOutputCfg[2];

}BRAP_FWIF_P_DDReencodeConfigParams;



/*
   This data structure defines DV258 user configuration parameters 
*/
typedef struct BRAP_FWIF_P_DV258ConfigParams
{
	/*General Settings*/

	/*	Range of values -  0 or 1. As per the new guyidelines from Dolby, i32DolbyVolumeEnable =1(True) for both when DV is enabled and disabled. 
		The controlling factor is now i32LvlEnable. Default value is 1 (True)*/
	int32_t		i32DolbyVolumeEnable;			
	
	/*	Volume modeler on/off. Possible values are 0(off) and 1(on). 
		Default 0(off). Always set to off for DV258 
	*/
	int32_t		i32VlmMdlEnable;

	/*	Flag to operate Dolby Volume in half mode. Flag is always set to off. Range [0,1]. Default ->0 (disable)*/
	int32_t     	i32HalfmodeFlag;				

	/*	Enable midside processing. Range [0,1]. Default ->1(enable).
		Flag is always set to enable for Dv258.*/
	int32_t		i32EnableMidsideProc;			

	/*	Volume Leveler Settings */	

	/*	Range of values -  0=FALSE , 1=TRUE. 
		i32LvlEnable = 1 When Dolby Volume is Enabled. 
		i32LvlEnable = 0 When Dolby Volume is Disabled. 
		Default value : 1 (TRUE - volume leveler enabled)*/
	int32_t		i32LvlEnable;					

	/*Range of values - [0 to 10]. Default value: 9 */
	int32_t		i32LvlAmount;					


	/*	System Settings*/

	/*	Input reference level in dB. Range of values(in dB )- (0 to 130 in steps of 0.0625 dB). Possible values are 0 to 2080. Default value :1360(85 dB)*/
	int32_t		i32InputReferenceLevel;			

	/*	The gain, if any, applied to the signal prior to entering DV258 in dB.
		Range of values(in dB )- (-30 to 30 in steps of 0.0625 dB). Possible values are -480 to 480. Default value : 0 (0 dB)*/
	int32_t		i32Pregain;						

	/*	Output reference level in dB. Range of values(in dB )- (0 to 130 in steps of 0.0625 dB). 
		Possible values are 0 to 2080. Default value :1360(85 dB)*/
	int32_t		i32OutputReferenceLevel;

	/*	Calibration offset in dB. Range of values(in dB)- (-30 to 30 in steps of 0.0625 dB). Possible values are -480 to 480. Default value    : 0( 0 dB)*/
	int32_t		i32CalibrationOffset;	

	/*	Volume Modeler Settings*/
	
	/*	Volume level gain in dB -- applied after dolby volume.
		Range of values(in dB) - (-96 to 30 in steps of 0.0625 dB). Possible values are -1536 to 480.
		Default value    : 0 (0 dB)*/
	int32_t		i32AnalogVolumeLevel;			
	
	/*	Volume level gain in dB -- applied by dolby volume.
		Range of values(in dB) - (-96 to 30 in steps of 0.0625 dB). Possible values are -1536 to 480. 
		Default value    : 0 (0 dB)*/

	int32_t		i32DigitalVolumeLevel;			

	/*User-input forced reset flag. Range [0,1], default 0*/
	int32_t		i32ResetNowFlag;				

	/* Limiter Settings */

	/*	Enable Limter. When DV is enabled limiter_enable = TRUE. 
		When DV is disabled limiter_enable = FALSE*/
	int32_t     	i32LimiterEnable;				

}BRAP_FWIF_P_DV258ConfigParams;

/*

   This data structure defines Dolby MS DDP decoder user configuration parameters 
*/
 
typedef struct  BRAP_FWIF_P_DDPMultiStreamUserOutput
{
   /* p_user_cfg->i32CompMode = 2;*/
	/*Custom_0=0, Custom_1 = 1, Line = 2, RF =3 , DRC_OFF = 4 */
    int32_t                i32CompMode;  
	/* p_user_cfg->i32PcmScale = 0x7FFFFFFF.    The value is 0 to 100 in 1.31 format. Range: 0 to 0x7FFFFFFF. */
    int32_t                i32PcmScale;  
   /* p_user_cfg->i3DynScaleHigh = 0x7FFFFFFF.    The value is 0 to 100 in 1.31 format. Range: 0 to 0x7FFFFFFF. */
    int32_t                i32DynScaleHigh;    
   /* p_user_cfg->i32DynScaleLow = 0x7FFFFFFF.    The value is 0 to 100 in 1.31 format. Range: 0 to 0x7FFFFFFF. */
    int32_t                i32DynScaleLow;    
    /* This is LFE ON/OFF flag and can take  two values 0 or 1 
     default value for this field is 0 
	*/
    int32_t i32OutLfe;                         
    /*	enum { GBL_MODE11=0, GBL_MODE_RSVD=0, GBL_MODE10, GBL_MODE20, 
		GBL_MODE30, GBL_MODE21, GBL_MODE31, GBL_MODE22, GBL_MODE32 };
		i32OutMode =7 default value;
	*/
    int32_t i32OutMode;
    /*	preferred stereo mode
		enum { GBL_STEREOMODE_AUTO=0, GBL_STEREOMODE_SRND, GBL_STEREOMODE_STEREO };
		i32StereoMode = 0 is default value ;
	*/
    int32_t i32StereoMode;  
    /* dual mono downmix mode
	   enum { GBL_DUAL_STEREO=0, GBL_DUAL_LEFTMONO, GBL_DUAL_RGHTMONO, GBL_DUAL_MIXMONO };
       i32DualMode = 0 is default value;
	*/
    int32_t i32DualMode;
    /* karaoke capable mode 
       enum { NO_VOCALS=0, GBL_VOCAL1, GBL_VOCAL2, GBL_BOTH_VOCALS };
       i32Kmode = 3;
    */
    int32_t i32Kmode;   
    /* This i32ExtDnmixEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtDnmixEnabled field is 0
	*/
    int32_t i32ExtDnmixEnabled;            
    int32_t i32ExtDnmixTab[DDP_DEC_GBL_MAXPCMCHANS][DDP_DEC_GBL_MAXPCMCHANS]; 
    /* This i32ExtKaraokeEnabled flag which can take  two values 0 or 1 based on disable/enable option
       default value for this i32ExtKaraokeEnabled field is 0
    */
    int32_t i32ExtKaraokeEnabled;            
    int32_t i32Extv1Level;                  
    int32_t i32Extv1Pan;                  
    int32_t i32Extv2Level;                  
    int32_t i32Extv2Pan;                  
    int32_t i32ExtGmLevel;                   
    int32_t   i32ExtGmPan;                  
    /*This is channel matrix of size 8 where each index can take any values from 0 to 7
	  Depending upon  the channel routing done    
	*/
    uint32_t   ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

} BRAP_FWIF_P_DDPMultiStreamUserOutput;
 

typedef struct  BRAP_FWIF_P_DDPMultiStreamConfigParams
{
	/* Possible Values: 0(Single Decode), 1(MS10 Mode) and 2(MS11 Mode)
	Single Decode: No mixing, no DV258. Converter allowed
	MS10 Mode: Dual decode mode with Stereo mixing. Converter allowed on Main Decode.
	MS11 Mode: Single Decode with DV258. Dual Decode with/without DV258.
	DDRE should be added as last stahe for MS11 mode. Converter should not be enabled
	in MS11 mode. 
	*/
	BRAP_AF_P_DolbyMsUsageMode	eDolbyMsUsageMode;

    /* This enum lets the mixer know if the decoder type is primary or secondary or sound effects. */    
    BRAP_AF_P_DolbyMsDecoderType eDecoderType;
	
	/*  This value "i32NumOutPorts" can be set to 1 or 2 based on output ports */
    int32_t                i32NumOutPorts;     

    /* These are user config parameters required from user  */
    BRAP_FWIF_P_DDPMultiStreamUserOutput  sUserOutputCfg[2];

	/* This value is used to enable/disable stream dialog normalization value.  0 is for Disable and 1 is for Enable
	   Default is Enable
	 */
	 int32_t				i32StreamDialNormEnable;

	/*	This value indicates how far the average dialogue level is below digital 100 percent. Valid
		values are 1-31. The value of 0 is reserved and it means dialog normalization is turned off. The values of 1 to 31 are interpreted as -1 dB to -31dB 
		with respect to digital 100 percent. If the reserved value of 0 is received, the decoder will not do any dialog normalization, 
		Default value is 0 
	*/
	int32_t					i32UserDialNormVal;

    uint32_t                ui32SubstreamIDToDecode; /*Default value =0 ; Range 0-3 for MS10*/

} BRAP_FWIF_P_DDPMultiStreamConfigParams;

/*
	This data structure defines PCMWAV/PCM_AVI configuration parameters 
*/

typedef struct BRAP_FWIF_P_PcmWavConfigParams
{
	
	uint32_t	ui32NumOutputPorts;
	uint32_t	ui32OutMode;					 /* OutMode */
	uint32_t	ui32OutputChannelMatrix[BRAP_AF_P_MAX_CHANNELS];

	BRAP_AF_P_DolbyMsUsageMode      eDolbyMsUsageMode;

}BRAP_FWIF_P_PcmWavConfigParams;	


/*
	This data structure defines BTSC configuration parameters 
*/
typedef struct BRAP_FWIF_P_BTSCEncodeConfigParams
{
	BRAP_AF_P_EnableDisable eBTSCEnableFlag;				/*Default Enable*/
	BRAP_AF_P_EnableDisable eUseDeEmphasizedSourceSignals;			/*Default Enable*/
	BRAP_AF_P_EnableDisable eMainChannelPreEmphasisOn;			/*Default Enable*/
	BRAP_AF_P_EnableDisable eEquivalenMode75MicroSec;			/*Default Disable*/
	BRAP_AF_P_EnableDisable eClipDiffChannelData;				/*Default Enable*/
	BRAP_AF_P_EnableDisable eDiffChannelPreEmphasisOn;			/*Default Enable*/
	BRAP_AF_P_EnableDisable eUseLowerThresholdSpGain;			/*Default Enable*/

	uint32_t ui32AttenuationFactor;						/*Q-format 3.29 format: Maximum and default is : 0x20000000  */
	uint32_t ui32SRDCalibrationFactor; 					/*Q-format 4.28 format: Maximum value is 0x7fffffff corresponding to 8.0:  
										Default :12BE76C9*/


	BRAP_AF_P_EnableDisable eSumChanFreqDevCtrl;				/*Default Enable Used to Control FM Deviation of Sum Channel to 25Khz*/
	BRAP_AF_P_EnableDisable eDiffChanFreqDevCtrl;				/*Default Enable Used to Control FM Deviation of Diff Channel to 50Khz*/
	BRAP_AF_P_EnableDisable eOpFreqDevCtrl;					/*Default Enable Used to Control FM Deviation of Final Output to 50Khz*/

	BRAP_AF_P_EnableDisable eFreqGeneration;				/*Default Disable. Used to Generate Test Tones*/
	int32_t	  i32Frequency;							/*Default 0 generates frequency sweep.  Programming to a fixed value generates tone*/

    int32_t	  i32SumChannelLevelCtrl;					/*Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */
	int32_t   i32DiffChannelLevelCtrl;					/*Q-format 3.29 format: Maximum is 0x7fffffff and default is : 0x20000000  */

    BRAP_AF_P_EnableDisable eBTSCMonoModeOn; /* Default Disable */
	
	
}BRAP_FWIF_P_BTSCEncodeConfigParams;


#endif
