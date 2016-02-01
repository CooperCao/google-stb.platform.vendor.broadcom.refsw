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

#include "brap.h"
#include "brap_priv.h"
#include "brap_dsphostinterface.h"
#include "brap_img.h"

BDBG_MODULE(rap_dspchn);		/* Register software module with debug interface */

/*******************************************************
 * Local defines
 *******************************************************/
#if (BRAP_7401_FAMILY == 1)
#define BRAP_AUDIO_DECODER_DELAY 128
#elif (BCHP_CHIP == 7400)
#define BRAP_AUDIO_DECODER_DELAY 144 
#elif BCHP_7411_VER
#define BRAP_AUDIO_DECODER_DELAY 64
#endif

#define BRAP_DSPCHN_P_AC3_TIMECODE_HRS_MASK	0x2E00
#define BRAP_DSPCHN_P_AC3_TIMECODE_HRS_SHIFT	9
#define BRAP_DSPCHN_P_AC3_TIMECODE_MNT_MASK	0x1F8
#define BRAP_DSPCHN_P_AC3_TIMECODE_MNT_SHIFT	3
#define BRAP_DSPCHN_P_AC3_TIMECODE_SEC_MASK	0x7
#define BRAP_DSPCHN_P_AC3_TIMECODE_SEC_SHIFT	0
#define BRAP_DSPCHN_P_ONE_FRAME_TIME		25
#define BRAP_DSPCHN_P_AC3COEFF_KARAOKE_COEFF	6
#define BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_ROWS	6
#define BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_COLUMNS	6
#define BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_ROWS	6
#define BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_COLUMNS	6
#define BRAP_DSPCHN_P_DTSCOEFF_DNMX_COEFF_ROWS	6
#define BRAP_DSPCHN_P_DTSHDCOEFF_DNMX_COEFF_ROWS	6
#define BRAP_DSPCHN_P_BDLPCMCOEFF_DNMX_COEFF_COLUMNS	8
#define BRAP_DSPCHN_P_BDLPCMCOEFF_DNMX_COEFF_ROWS	8
#define BRAP_DSPCHN_P_LPCMHDDVDCOEFF_DNMX_COEFF_COLUMNS 8 
#define BRAP_DSPCHN_P_LPCMHDDVDCOEFF_DNMX_COEFF_ROWS	  8  
#define BRAP_DSPCHN_P_LPCMDVDCOEFF_DNMX_COEFF_COLUMNS  8 
#define BRAP_DSPCHN_P_LPCMDVDCOEFF_DNMX_COEFF_ROWS	  8  
#define BRAP_DSPCHN_P_DTSCOEFF_DNMX_COEFF_COLUMNS	6
#define BRAP_DSPCHN_P_DTSHDCOEFF_DNMX_COEFF_COLUMNS	6
#define BRAP_DSPCHN_P_MPEG_LAYERS					3
#define BRAP_DSPCHN_P_BIT_RATE_INDEX_MAX			15
#define BRAP_DSPCHN_P_DTS_BIT_RATE_INDEX_MAX		32
#define BRAP_DSPCHN_P_TSM_LOG_RDB_FIELD_BUF_SIZE	3
#define BRAP_DSPCHN_P_TSM_LOG_BUF_SIZE	\
		(2048 * (1 + BRAP_DSPCHN_P_TSM_LOG_RDB_FIELD_BUF_SIZE))
#define BRAP_DSPCHN_P_FW_CMD_TIMEOUT		10
/* Temp define: Need to be in brap_dsphostinterface.h */
#define SIZE_OF_BAF_DEC_CONFIG			(540 * 2) /* Max size required for LPCM */

#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
#define SIZE_OF_BAF_PP_CONFIG			(20 * 2) /* Max size required for Post processing algo */
#endif

#ifdef SIZE_OF_BAF_SPDIF_INFO
#undef SIZE_OF_BAF_SPDIF_INFO
#endif
#define SIZE_OF_BAF_SPDIF_INFO				(26)
#define SIZE_OF_START_STOP_PTS_BUF		(20)
#define BRAP_DSPCHN_P_AC3_OUTPUT_MODE20		2

#define BRAP_DSPCHN_P_DEF_ST_THRESHOLD 2    /* 2 ms */
#define BRAP_DSPCHN_P_DEF_GA_THRESHOLD 8    /* 8 ms */
#define BRAP_DSPCHN_P_DEF_SL_THRESHOLD 0	   /* 0 ms */
#define BRAP_DSPCHN_P_DEF_MPEG_DISCARD_THRESHOLD	(0x20F58)
#define BRAP_DSPCHN_P_DEF_DSS_DISCARD_THRESHOLD		(0x20F58 * 600)
#define BRAP_DSPCHN_P_DEF_MPEG_WMA_UPPER_THRESHOLD	(45000/2)
#define BRAP_DSPCHN_P_DEF_DSS_WMA_UPPER_THRESHOLD	(27000000/2)
#define BRAP_DSP_P_PT_CXT_IN_SIMUL_MODE				2
/* TODO: BCHP_DSPCHN_P_DVD_LPVM_ALGO_ID is defined since RDB is not ready.
 * Once RDB is ready, it needs to be removed */
#define BCHP_DSPCHN_P_DVD_LPVM_ALGO_ID				(0xE)

#define BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT	1	/* Total number of downmixed ouput */
#define BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT	2	/* Total number of downmixed ouput */
#define BRAP_DSPCHN_P_WMA_SAMPLES_PER_FRAME (2048)


/* Macros to tell FW what data to write to the RBUFs */
#define BRAP_DSPCHN_P_FILL_ZERO		0xFF    /* FW should write 0's to this RBUF */


/*******************************************************
 * Local Type definitions
 *******************************************************/
 typedef struct BRAP_DSPCHN_P_StreamInfoReg
 {
 	uint32_t	ui32StreamInfoReg0;	
 	uint32_t	ui32StreamInfoReg1;	
 	uint32_t	ui32StreamInfoReg2;	
 	uint32_t	ui32StreamInfoReg3;	
 	uint32_t	ui32StreamInfoReg4;
} BRAP_DSPCHN_P_StreamInfoReg;

typedef enum BRAP_DSPCHN_P_SpeakerConfig
{
	BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft,
	BRAP_DSPCHN_P_SpeakerConfig_eFrontRight,
	BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr,
	BRAP_DSPCHN_P_SpeakerConfig_eRightSurr,
	BRAP_DSPCHN_P_SpeakerConfig_eCenter,
	BRAP_DSPCHN_P_SpeakerConfig_eLFE,
	BRAP_DSPCHN_P_SpeakerConfig_eBackLeft,
	BRAP_DSPCHN_P_SpeakerConfig_eBackRight,
	BRAP_DSPCHN_P_SpeakerConfig_eMax
}BRAP_DSPCHN_P_SpeakerConfig;


/*******************************************************
 * External References
 *******************************************************/
BERR_Code BRAP_DSP_P_CopyFWImageToMem (BRAP_DSP_Handle hDsp, uint32_t memAdr, uint32_t firmware_id);

/*******************************************************
 * Static Variables & Functions
 *******************************************************/
 static BERR_Code BRAP_DSPCHN_P_StartDspCxt(
 						BRAP_DSPCHN_Handle	pDspChn,
						const BRAP_DSPCHN_P_AudioParams *psAudioParams);
static BERR_Code BRAP_DSPCHN_P_StopDspCxt(BRAP_DSPCHN_Handle      pDspChn);
static void BRAP_DSPCHN_P_ReadStreamInfo(
					BRAP_DSPCHN_Handle hDspChn,
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					bool bIsrCxt);

static BERR_Code BRAP_DSPCHN_P_GetStreamInformation(
					BRAP_DSPCHN_Handle      hDspChn,	/* [in] DSPCHN handle */
					bool					bIsrCxt,		/* [in] Is it a ISR context */
					BRAP_DSPCHN_StreamInfo *psStreamInfo); /* [out] Stream Information */
static void BRAP_DSPCHN_P_GetMpegInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetAacInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetAacSbrInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetAc3PlusInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetAc3LosslessInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);

#ifdef RAP_PCMWAV_SUPPORT            
static void BRAP_DSPCHN_P_GetPcmWavInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
#endif

#ifdef RAP_DRA_SUPPORT
static void BRAP_DSPCHN_P_GetDraInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
#endif
#if (BCHP_7411_VER != BCHP_VER_C0)
static void BRAP_DSPCHN_P_GetWmaStdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
#endif

#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
static void BRAP_DSPCHN_P_GetWmaProInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
#endif


static void BRAP_DSPCHN_P_GetDtsInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
#ifdef BCHP_7411_VER /* Only for the 7411 */
static void BRAP_DSPCHN_P_GetDtshdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetBdLpcmInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetLpcmHdDvdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
static void BRAP_DSPCHN_P_GetLpcmDvdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);

#if BCHP_7411_VER > BCHP_VER_C0
static void BRAP_DSPCHN_P_GetMlpInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);

BERR_Code
BRAP_DSPCHN_P_SetDdbmConfigurationParams (BRAP_DSPCHN_Handle hDspCh);
#endif
#endif

static void BRAP_DSPCHN_P_GetLpcmDvdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);

 static void BRAP_DSPCHN_P_GetAc3Info(
					BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo);
 static BERR_Code BRAP_DSPCHN_P_AllocMem( BRAP_DSPCHN_Handle hDspCh);
 static void BRAP_DSPCHN_P_Free( BRAP_DSPCHN_Handle hDspCh);
static void BRAP_DSPCHN_P_AddWordToFWBuffer(
					unsigned char *bufPtr,
					uint32_t data,
					uint32_t nBytes);
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
static uint16_t BRAP_DSPCHN_P_FloatToQ115(int16_t floatVar);
#endif
static uint32_t BRAP_DSPCHN_P_FloatToQ131(int16_t floatVar);
static uint32_t BRAP_DSPCHN_P_FloatToQ230(int16_t floatVar);

void BRAP_DSPCHN_P_GetCurrentPTS(
					BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
					bool					bIsrCxt,		/* [in] Is it a ISR context */
					BRAP_DSPCHN_PtsInfo		*psPtsInfo);		/* [out] Current PTS information */ 
static BERR_Code BRAP_DSPCHN_P_DownloadFirmware(
								BRAP_DSPCHN_Handle hDspCh, 
								const BRAP_DSPCHN_P_AudioParams *psAudioParams);
static void BRAP_DSPCHN_P_SetStcValidFlag(
					BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
					bool				bIsrCxt);		/* [in] true = ISR context, false = Task context */
static BERR_Code BRAP_DSPCHN_P_DecrementCtxBufUsage(
					BRAP_DSP_Handle hDsp, 
					BRAP_DSPCHN_AudioType eAudType, 
					BRAP_DSPCHN_DecodeMode eDecMode);
static bool BRAP_DSP_P_CheckFwCtxtAvailability(
					BRAP_DSP_Handle hDsp, 
					BRAP_DSPCHN_DecodeMode eDecMode,
					BRAP_DSPCHN_AudioType eAudType,
					uint32_t *pui32FwCtxIndx );  /* [out] */

static void BRAP_DSPCHN_P_InitArrayOfConfBufPtrCopy(
	BRAP_DSPCHN_Handle		hDspCh
	);
static BERR_Code BRAP_DSPCHN_P_GetNextArrayOfConfBufPtr(
	BRAP_DSPCHN_Handle		hDspCh,
	unsigned int				*puiArrayInUse
	);
static BERR_Code BRAP_DSPCHN_P_GetNextDecConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int *puiNextConfBuf
	);
static BERR_Code BRAP_DSPCHN_P_ProgramDecConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int				uiNextConfBuf
	);

static BERR_Code BRAP_DSPCHN_P_GetNextPpConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int uiPpBranchId,
	unsigned int uiPpStage,
	unsigned int *puiNextConfBuf
	);

static BERR_Code BRAP_DSPCHN_P_ProgramPpConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	BRAP_DSPCHN_PP_Algo	ePpAlgo,
	unsigned int				uiNextConfBuf,
	bool						bUseDefault,
	unsigned int				uiPpBranchId,
	unsigned int				uiPpStgId
	);
static void BRAP_DSPCHN_P_ProgramArrayOfConfBufPtr( 
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int				uiNextArray
	);
#if (BRAP_AD_SUPPORTED == 1) 
static void BRAP_DSPCHN_P_FormFadeConfigStruct(	
            	BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	);
static void BRAP_DSPCHN_P_FormPanConfigStruct(	
            	BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	);
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
static void BRAP_DSPCHN_P_FormDolbyVolumeConfigStruct(	
            	BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	);
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
static void BRAP_DSPCHN_P_FormSRSVolumeConfigStruct(	
            	BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	);
#endif
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
static void BRAP_DSPCHN_P_FormAc3ConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *ac3ParamsPtr);
#else
static void 
BRAP_DSPCHN_P_FormDDPConfigStructForAC3 (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *ac3ParamsPtr);
#endif
static void BRAP_DSPCHN_P_FormAc3LosslessConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *ac3ParamsPtr);
static void BRAP_DSPCHN_P_FormMpegConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *mpegParamsPtr);
static void BRAP_DSPCHN_P_FormAacConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *aacParamsPtr);
static void BRAP_DSPCHN_P_FormAacSbrConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *aacSbrParamsPtr);
static void BRAP_DSPCHN_P_FormAc3PlusConfigStruct (
					BRAP_DSPCHN_Handle	hDspCh,
					unsigned char *ac3PlusParamsPtr);
static void BRAP_DSPCHN_P_FormPcmWavConfigStruct (
			    	BRAP_DSPCHN_Handle	hDspCh,
				    unsigned char *pcmWavParamsPtr);

#if (BRAP_DTS_SUPPORTED == 1)		
static void BRAP_DSPCHN_P_FormDtsCoreConfigStruct (
					BRAP_DSPCHN_Handle hDspCh,
					unsigned char *dtsParamsPtr);
#else

static void BRAP_DSPCHN_P_FormDtsConfigStruct (
					BRAP_DSPCHN_Handle hDspCh,
					unsigned char *dtsParamsPtr);
#endif

static void BRAP_DSPCHN_P_FormDtshdConfigStruct (
					BRAP_DSPCHN_Handle hDspCh,
					unsigned char *dtshdParamsPtr);
static void BRAP_DSPCHN_P_FormBdLpcmConfigStruct (
					BRAP_DSPCHN_Handle hDspCh,
					unsigned char *dtsParamsPtr);
static void BRAP_DSPCHN_P_FormLpcmHdDvdConfigStruct (
					BRAP_DSPCHN_Handle hDspCh,
					unsigned char *dtsParamsPtr);
static void BRAP_DSPCHN_P_FormLpcmDvdConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *lpcmDvdParamsPtr);
static void BRAP_DSPCHN_P_FormMlpConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *mlpParamsPtr );
static void BRAP_DSPCHN_P_FormWmaProConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *wmaProParamsPtr );
static void BRAP_DSPCHN_P_FormWmaStdConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *wmaStdParamsPtr );
static void BRAP_DSPCHN_P_FormDraConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *wmaStdParamsPtr );

static BERR_Code BRAP_DSPCHN_P_SetDecConfigurationParams(
	BRAP_DSPCHN_Handle		hDspCh);			/* [in] DSP channel handle */

unsigned int
BRAP_DSPCHN_P_SampleRateEnumToValue(BAVC_AudioSamplingRate eSampleRate);


#if (BRAP_DSP_P_7401_NEWMIT==1)
static BERR_Code BRAP_DSPCHN_P_AllocateIsIsIfBuffers( BRAP_DSPCHN_Handle hDspCh );
static void BRAP_DSPCHN_P_FreeIsIsIfBuffers( BRAP_DSPCHN_Handle hDspCh );
static void BRAP_DSPCHN_P_InitPp(BRAP_DSPCHN_Handle hDspChn);
#endif

static BERR_Code BRAP_DSPCHN_P_GetPts2StcPhase_isr(
		BRAP_ChannelHandle	hRapCh,
		int32_t				*pi32Pts2StcPhase,
		bool					bIsrCxt );


static const BRAP_DSPCHN_P_Settings sDefDspChnSettings = {
	{
		BRAP_OutputMode_eStereo,	/* Default output mode */
		BRAP_OutputMode_eStereo	/* Default output mode */
	},
    {
        false, /*bMonoToAll = false*/
        false
    },	
	{
		/* sDefDspChnSettings.sDspchnExtSettings[0] */
		{
			BRAP_DSPCHN_DualMonoMode_eStereo,	/* Dual Mono Mode */
		},
		/* sDefDspChnSettings.sDspchnExtSettings[1] */
		{
			BRAP_DSPCHN_DualMonoMode_eStereo,	/* Dual Mono Mode */
		},
	},
	{
		/* sDefDspChnSettings.sDspchnCfgParams */
		{
			/* BRAP_DSPCHN_Ac3ConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3CompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3CompMode_eLine, 			/* eCompMode */
#endif
				BRAP_DSPCHN_Ac3DualMonoMode_eStereo, 		/* eDualMono */
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3KaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3StereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */

			},
			/* BRAP_DSPCHN_AacConfigParams structure */
			{
				100,	/* ui16DrcGainControlCompress = 1 */
				100,	/* ui16DrcGainControlBoost = 1 */
				127,	/* ui16DrcTargetLevel = 0 */
				0, 		/*ARIB matrix mixdown index which can take integer values from 0 to 3*/				
				BRAP_DSPCHN_AAC_DownmixType_eBRCM,	/* DownmixType */
				0, /* ui16bOutputLfeOn = 0 */
				0, /* ui16ExtDownmixEnable = 0 */
				{{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* ui16ExtDnMixCoeffs[5] */
				0,									/* Default = 0, 0 -> FALSE, 1 -> TRUE */
				false								/* Default = FALSE */
			},
			/* BRAP_DSPCHN_AacSbrConfigParams structure */
			{
				100,	/* ui16DrcGainControlCompress = 1 */
				100,	/* ui16DrcGainControlBoost = 1 */
				127,	/* ui16DrcTargetLevel = 0 */
				0, 		/*ARIB matrix mixdown index which can take integer values from 0 to 3*/				
				BRAP_DSPCHN_AACSbr_DownmixType_eBRCM,	/* DownmixType */
				0, /* ui16bOutputLfeOn = 0 */
				0, /* ui16ExtDownmixEnable = 0 */
				{{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[4] */	
				{ 0, 0, 0, 0, 0, 0}},   			/* ui16ExtDnMixCoeffs[5] */
				0,									/* Default = 0, 0 -> FALSE, 1 -> TRUE */
				false								/* Default = FALSE */				
			},
			/* BRAP_DSPCHN_Ac3PlusConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3PlusCompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3PlusCompMode_eLine, 			/* eCompMode */
#endif
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3PlusStereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */

			},
			/* BRAP_DSPCHN_DtsConfigParams structure */
			{
				false, 								/* bOutputLfeOn */
				BRAP_DSPCHN_DtsStereoMode_eAuto, 	/* eStereoMode */
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}}   				/* i32UserDownmixCoeff[5] */
			},
#if (BRAP_DTS_SUPPORTED == 1)					
			/* BRAP_DSPCHN_DtsCoreConfigParams structure */
			{
				1, 		    						/* i32UserDRC_flag */
				0x7fffffff,                     	/* i32DynScaleHigh */
				0x7fffffff, 						/* i32DynScaleLow */
				1,  	                			/* i32NumOutputport */	
				2,  			                	/* i32UserAMODE */	
				false,  	                		/* bOutputLfeOn */	
				BRAP_DSPCHN_DtsStereoMode_eAuto,  	/* eStereoMode */
				0,
				0,
				0,
				0,
				0,
			},
#endif			
			/* BRAP_DSPCHN_DtshdConfigParams structure */
			{
				false, 								/* bOutputLfeOn */
				BRAP_DSPCHN_DtsStereoMode_eAuto, 	/* eStereoMode */
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}}   				/* i32UserDownmixCoeff[5] */
			},	
			/* BRAP_DSPCHN_BdlpcmConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
			},
			/* BRAP_DSPCHN_LpcmHdDvdConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
		
			},	
			/* BRAP_DSPCHN_LpcmDvdConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
			},	
			/* BRAP_DSPCHN_Ac3LosslessConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3PlusCompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3PlusCompMode_eLine, 			/* eCompMode */
#endif
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3PlusStereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */				

			},
			/* BRAP_DSPCHN_DdbmConfigParams structure */
			{
				BRAP_DSPCHN_DdbmMode_eConfig1,	/* eConfigMode */
			},
			/* BRAP_DSPCHN_MlpConfigParams structure */
			{
				BRAP_DSPCHN_MlpProfileSelect_eTwoChannel,		/* eProfileSelect */
				BRAP_DSPCHN_MlpDrc_eDisable,					/* eDrc */
				100,											/* ui32DrcBoost */
				100,											/* ui32DrcCut */
				false,										/* bDialogueEnable */
				-31,											/* i32DialogueLevel */
				false											/* bOutputLfeOn */
			},
			/* BRAP_DSPCHN_WmaProConfigParams structure */
			{
				false,								/* bDrcFlag 	*/
				0,									/* i32DrcBoost */
				0,									/* i32DrcCut */
				false,									/* bLfeOn */
				BRAP_DSPCHN_WmaProStereoMode_eLoRo	/* eStereo Mode */
			},
			/* BRAP_DSPCHN_WmaStdConfigParams structure */			
			{
				BRAP_DSPCHN_WmaStdStereoMode_eLoRo       /* ui32StereoMode */
			},
			/*  BRAP_DSPCHN_PcmWavConfigParams structure */
            {
                1,
                2,
                {0,1,0,0,0,0,0,0}
            },			
			/* BRAP_DSPCHN_DraConfigParams structure */			
			{
				1,          /* This Flag is used to turn ON/OFF pcm domain downmixing */
            	1,      	/* Range 0, 1, 2
                        	0 - Mono Downmixed output
                        	1 - LoRo downmixed output
                        	2 - LtRt downmixed output
                        	The decoded output is downmixed to a configuration based on the ui32nOutmode value,
                        	if  ui32EnableDownmix 	is set to 1 otherwise the ui32nOutmode value is ignored .*/
                24,       /* Default = 24	Range - 24, 16
                            This value gives the output bit width of the decoded PCM samples.*/                        	
                0,          /*Default =0;	Range - 0, 1, 2*/
                0,          /*Default=0; 	Range - 0,1
                            This define has to be made 1 for allowing a fixed dummy pattern to be o/p on dummy calls. 
                            This has to be set to 	1 for initial testing*/ 
                0xabcdabcd  /*Default - 0xabcdabcd
                            This value is relevant when ui32nCertDummyPatternEnable is set to 1. 
                            The decoder outputs this pattern on the	ring buffers in case of a dummy call */                            
            }
		}, /* sDefDspChnSettings.sDspchnCfgParams[0] */
		{ /* sDefDspChnSettings.sDspchnCfgParams[1] */
			/* BRAP_DSPCHN_Ac3ConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3CompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3CompMode_eLine, 			/* eCompMode */
#endif
				BRAP_DSPCHN_Ac3DualMonoMode_eStereo, 		/* eDualMono */
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3KaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3StereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */				

			},
			/* BRAP_DSPCHN_AacConfigParams structure */
			{
				100,	/* ui16DrcGainControlCompress = 1 */
				100,	/* ui16DrcGainControlBoost = 1 */
				127,	/* ui16DrcTargetLevel = 0 */
				0, 		/*ARIB matrix mixdown index which can take integer values from 0 to 3*/				
				BRAP_DSPCHN_AAC_DownmixType_eBRCM,	/* DownmixType */
				0, /* ui16bOutputLfeOn = 0 */
				0, /* ui16ExtDownmixEnable = 0 */
				{{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* ui16ExtDnMixCoeffs[5] */
				0,									/* Default = 0, 0 -> FALSE, 1 -> TRUE */
				false								/* Default = FALSE */				
			},
			/* BRAP_DSPCHN_AacSbrConfigParams structure */
			{
				100,	/* ui16DrcGainControlCompress = 1 */
				100,	/* ui16DrcGainControlBoost = 1 */
				127,	/* ui16DrcTargetLevel = 0 */
				0, 		/*ARIB matrix mixdown index which can take integer values from 0 to 3*/				
				BRAP_DSPCHN_AACSbr_DownmixType_eBRCM,	/* DownmixType */
				0, /* ui16bOutputLfeOn = 0 */
				0, /* ui16ExtDownmixEnable = 0 */
				{{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* ui16ExtDnMixCoeffs[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* ui16ExtDnMixCoeffs[5] */
				0,									/* Default = 0, 0 -> FALSE, 1 -> TRUE */
				false								/* Default = FALSE */				
			},
			/* BRAP_DSPCHN_Ac3PlusConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3PlusCompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3PlusCompMode_eLine, 			/* eCompMode */
#endif
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3PlusStereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */
			},
			/* BRAP_DSPCHN_DtsConfigParams structure */
			{
				false, 								/* bOutputLfeOn */
				BRAP_DSPCHN_DtsStereoMode_eAuto, 	/* eStereoMode */
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}}   				/* i32UserDownmixCoeff[5] */
			},
#if (BRAP_DTS_SUPPORTED == 1)					
			/* BRAP_DSPCHN_DtsCoreConfigParams structure */
			{
				1, 		    						/* i32UserDRC_flag */
				0x7fffffff,                     	/* i32DynScaleHigh */
				0x7fffffff, 						/* i32DynScaleLow */
				1,  	                			/* i32NumOutputport */	
				2,  			                	/* i32UserAMODE */	
				false,  	                		/* bOutputLfeOn */	
				BRAP_DSPCHN_DtsStereoMode_eAuto,  	/* eStereoMode */	
				0,
				0,
				0,
				0,
				0,
			},		
#endif
			/* BRAP_DSPCHN_DtshdConfigParams structure */
			{
				false, 								/* bOutputLfeOn */
				BRAP_DSPCHN_DtsStereoMode_eAuto, 	/* eStereoMode */
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}}   				/* i32UserDownmixCoeff[5] */
			},	
			/* BRAP_DSPCHN_BdlpcmConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
			},
			/* BRAP_DSPCHN_LpcmHdDvdConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
		
			},	
			/* BRAP_DSPCHN_LpcmDvdConfigParams structure */
			{
				false, 							/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0, 0, 0},  				/* i32UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0, 0, 0}},   				/* i32UserDownmixCoeff[5] */
				false									/* bOutputLfe */
			},	
			/* BRAP_DSPCHN_Ac3LosslessConfigParams structure */
			{
				100,	 					/* ui16DynRngScaleHi */
				100,	 					/* ui16DynRngScaleLo */
				100, 						/* ui16PcmScaleFactor */
#if (BRAP_7401_FAMILY == 1)				
				BRAP_DSPCHN_Ac3PlusCompMode_eRf, 			/* eCompMode */
#else
				BRAP_DSPCHN_Ac3PlusCompMode_eLine, 			/* eCompMode */
#endif
				false, 						/* bOutputLfeOn */
				BRAP_DSPCHN_Ac3PlusKaraokeVocalSelect_eBothVocal, 	/* eKCapableMode */
				false, 						/* bUseKaraokeLevelPanCoeff */
				{ 0, 0, 0, 0, 0, 0}, 				/* i16KaraokeLevelPanCoeff */
				BRAP_DSPCHN_Ac3PlusStereoMode_eAuto, 		/* eStereoMode */
				false, 						/* bUsrUserDownmixCoeff */
				{{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[0] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[1] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[2] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[3] */	
				{ 0, 0, 0, 0, 0, 0},  				/* i16UserDownmixCoeff[4] */	
				{ 0, 0, 0, 0, 0, 0}},  				/* i16UserDownmixCoeff[5] */
				true								/* Dialog normalization */
			},
			/* BRAP_DSPCHN_DdbmConfigParams structure */
			{
				BRAP_DSPCHN_DdbmMode_eConfig1,	/* eConfigMode */
			},
			/* BRAP_DSPCHN_MlpConfigParams structure */
			{
				BRAP_DSPCHN_MlpProfileSelect_eTwoChannel,		/* eProfileSelect */
				BRAP_DSPCHN_MlpDrc_eDisable,					/* eDrc */
				100,											/* ui32DrcBoost */
				100,											/* ui32DrcCut */
				false,										/* bDialogueEnable */
				-31,											/* i32DialogueLevel */
				false											/* bOutputLfeOn */
			},
			/* BRAP_DSPCHN_WmaProConfigParams structure */
			{
				false,								/* bDrcFlag 	*/
				0,									/* i32DrcBoost */
				0,									/* i32DrcCut */
				false	,								/* bLfeOn */
				BRAP_DSPCHN_WmaProStereoMode_eLoRo	/* eStereo Mode */				
			},
			/* BRAP_DSPCHN_WmaStdConfigParams structure */			
			{
				BRAP_DSPCHN_WmaStdStereoMode_eLoRo
			},
			/*  BRAP_DSPCHN_PcmWavConfigParams structure */
            {
                1,
                2,
                {0,1,0,0,0,0,0,0}
            },			
			/* BRAP_DSPCHN_DraConfigParams structure */			
			{
				1,          /* This Flag is used to turn ON/OFF pcm domain downmixing */
            	1,      	/* Range 0, 1, 2
                        	0 - Mono Downmixed output
                        	1 - LoRo downmixed output
                        	2 - LtRt downmixed output
                        	The decoded output is downmixed to a configuration based on the ui32nOutmode value,
                        	if  ui32EnableDownmix 	is set to 1 otherwise the ui32nOutmode value is ignored .*/
                0x24,       /* Default = 24	Range - 24, 16
                            This value gives the output bit width of the decoded PCM samples.*/                        	
                0,          /*Default =0;	Range - 0, 1, 2*/
                0,          /*Default=0; 	Range - 0,1
                            This define has to be made 1 for allowing a fixed dummy pattern to be o/p on dummy calls. 
                            This has to be set to 	1 for initial testing*/ 
                0xabcdabcd  /*Default - 0xabcdabcd
                            This value is relevant when ui32nCertDummyPatternEnable is set to 1. 
                            The decoder outputs this pattern on the	ring buffers in case of a dummy call */                            
            }
		}
	},
	true,	/* sDefDspChnSettings.bTsmEnable */
	false, /*sDefDspChnSettings.bAtsmEnable */
	false,	/* sDefDspChnSettings.bTsmLogEnable */
	0x0,		/* sDefDspChnSettings.ui32TsmDiscardThresh */
	0x0		/* sDefDspChnSettings.ui32TsmUpperThresh */
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
    ,BRAP_DSPCHN_P_DEF_ST_THRESHOLD /* sDefDspChnSettings.ui32TsmSTThresh */
    ,BRAP_DSPCHN_P_DEF_GA_THRESHOLD /* sDefDspChnSettings.ui32TsmGAThresh */  
#if (BRAP_7401_FAMILY == 1)
    ,BRAP_DSPCHN_P_DEF_SL_THRESHOLD /* sDefDspChnSettings.ui32TsmSLThresh */    
#endif    
#endif
#if BCHP_7411_VER > BCHP_VER_C0
	,
	0	/* sDefDspChnSettings.i32TsmTransitionThreshold */
#endif	
#if (BRAP_DSPCHN_P_HAS_TSM_ERROR_RECOVERY==1)
	,true 	/* sDefDspChnSettings.bEnableTSMErrRecovery */
#endif
#if (BRAP_AD_SUPPORTED == 1)||(BRAP_DOLBYVOLUME_SUPPORTED == 1) ||(BRAP_SRS_TRUVOL_SUPPORTED == 1)
    ,
    {
#if (BRAP_AD_SUPPORTED == 1)    
        {
            BRAP_FadeConfigType_eFadedChOnly,
            0
        }
        ,
        {
            0x7fffffff,
            BRAP_PanConfigType_eADChannelOn,
            0
        },
#endif		
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		/* BRAP_DSPCHN_DolbyVolumeConfigParams structure */
		{
			true,                               /*false - dolby volume disabled - acts as pass trough node
	                                            true - dolby volume enabled   */
	        512,                                /* Size of processing block in samples */
	        20,                                 /* Number of critical bands to use      */
	        2,                                  /* Number of input/output channels     */
	        85,                         /* Input reference level in dBSPL   */
	        85,                         /* Output reference level in dBSPL   */
	        0,                         /* Calibration offset in dB  */
	        true,                                  /* volume modeler on/off  */
	        0,                                  /* User-input forced reset flag */
	        0,                         /* Volume level gain in dB -- applied by dolby volume     */
	        0,                         /* Volume level gain in dB -- applied after dolby volume  */
	        0,                                  /*Volume Range of values - [0 to 10]*/
	        false,                                  /*Volume leveler  -  0=FALSE , 1=TRUE*/
	        false,                                  /* Disabled midside processing */
	        false,									/* Disabled Half mode */
	        false                                   /* Disabled bLimiter_enable*/	        	       	        	        
		},			
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		/* BRAP_DSPCHN_SRSConfigParams structure */
		{
			true,                               /*false - SRS volume disabled - acts as pass trough node
	                                            true - dolby volume enabled   */
	        2,                                /* Size of processing block in samples */
	        256,                                 /* Number of critical bands to use      */
	        1,                                  /* Number of input/output channels     */
			25,                     		    /* Input reference level in dBSPL   */
			25,                    			     /* Output reference level in dBSPL   */
			25,       			                  /* Calibration offset in dB  */
			50,                                  /* volume modeler on/off  */
	        0x007fffff,                                  /* User-input forced reset flag */
	        1,			                         /* Volume level gain in dB -- applied by dolby volume     */
	        0,			                         /* Volume level gain in dB -- applied after dolby volume  */
	        16,                                  /*Volume Range of values - [0 to 256]*/	        
            1,                                  /* default: 1; 1 -> Noise manager enable, 0 -> disable */
            10
		}			
#endif		
    }
#endif

};	

static const uint32_t 
BRAP_DSPCHN_P_MpegBitRateTable[BRAP_DSPCHN_P_MPEG_LAYERS][BRAP_DSPCHN_P_BIT_RATE_INDEX_MAX] = {
	/* Bit Rates for Layer 3 */
	{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320},
	/* Bit Rates for Layer 2 */
	{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
	/* Bit Rates for Layer 1 */
	{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448}
};

static const uint32_t 
BRAP_DSPCHN_P_DtsBitRateTable[BRAP_DSPCHN_P_DTS_BIT_RATE_INDEX_MAX] = {
	32,56, 64,96,112,128,192,224,256,320,384,448,512,576,640,768,
	960,1024,1152,1280,1344,1408,1411,1472,1536,1920,2048,3072,3840,0,0,0
};

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
	)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BRAP_DSPCHN_EnableTSM);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	rc = BRAP_DSPCHN_EnableTSM_isr(hRapCh, bEnableTSM);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_DSPCHN_EnableTSM);
	return rc;
}


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
	)
{
	uint32_t regVal = 0, regAddr;

	BDBG_ENTER(BRAP_DSPCHN_EnableTSM_isr);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	hRapCh->sModuleHandles.hDspCh->sSettings.bTsmEnable = bEnableTSM;

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset; 
		regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister, regAddr, true);
		
		if (bEnableTSM) 
			regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT))
				| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT, 0x1);
		else
			regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT))
				| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT, 0x0);

		BRAP_DSP_Write32((BARC_Handle)hRapCh->hRegister, regAddr, regVal, true);
	}

	BDBG_LEAVE(BRAP_DSPCHN_EnableTSM_isr);
	return BERR_SUCCESS;

}


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
	)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BRAP_DSPCHN_EnableASTM);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	rc = BRAP_DSPCHN_EnableASTM_isr(hRapCh, bEnableASTM);
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_DSPCHN_EnableASTM);
	return rc;
}

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
	)
{
	uint32_t regVal = 0, regAddr;

	BDBG_ENTER(BRAP_DSPCHN_EnableASTM_isr);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	hRapCh->sModuleHandles.hDspCh->sSettings.bAtsmEnable = bEnableASTM;

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset; 
		regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister, regAddr, true);
		
		if (bEnableASTM) 
			regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE))
				| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE, 0x1);
		else
			regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE))
				| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE, 0x0);

		BRAP_DSP_Write32((BARC_Handle)hRapCh->hRegister, regAddr, regVal, true);
	}

	BDBG_LEAVE(BRAP_DSPCHN_EnableASTM_isr);
	return BERR_SUCCESS;

}


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
	)
{
	uint32_t regAddr;

	BDBG_ENTER(BRAP_DSPCHN_SetTSMDiscardThreshold);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.ui32TsmDiscardThresh = i32DiscardThreshold;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_DISCARD_THRESHOLD_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		BRAP_Write32(hRapCh->hRegister, regAddr, i32DiscardThreshold);
	}

	BDBG_LEAVE(BRAP_DSPCHN_SetTSMDiscardThreshold);
	return BERR_SUCCESS;
}

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
	)
{	
	uint32_t regAddr;
	
	BDBG_ENTER(BRAP_DSPCHN_SetTSMUpperThreshold);
	
#if (!defined BCHP_7411_VER) || (BCHP_7411_VER == BCHP_VER_C0)
	BSTD_UNUSED(regAddr);
	BDBG_ERR (("This function is supported only for 7411D0, E0"));
	return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

	BDBG_ENTER(BRAP_DSPCHN_SetTSMUpperThreshold);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);
	
	if(hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eType !=BRAP_DSPCHN_AudioType_eWmaStd
		&& hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eType !=BRAP_DSPCHN_AudioType_eWmaPro)
		{
    		BDBG_ERR(("This API is supported only for audiotype BRAP_DSPCHN_AudioType_eWmaStd and BRAP_DSPCHN_AudioType_eWmaPro"));
		    return BERR_TRACE(BERR_NOT_SUPPORTED);
		}

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		BRAP_Write32(hRapCh->hRegister, regAddr, i32UpperThreshold);
	}

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.ui32TsmUpperThresh = i32UpperThreshold;
	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BRAP_DSPCHN_SetTSMUpperThreshold);

	return BERR_SUCCESS;
	
}

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
	)
{
#if BCHP_7411_VER > BCHP_VER_C0
	uint32_t	regAddr;

	BDBG_ENTER( BRAP_DSPCHN_SetTsmTransitionThreshold );
	BDBG_ASSERT( hRapCh);

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.i32TsmTransitionThreshold = i32TsmTransitionThreshold;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_TRANS_THRESHOLD_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		BRAP_Write32(hRapCh->hRegister, regAddr, i32TsmTransitionThreshold);
	}

	BDBG_LEAVE( BRAP_DSPCHN_SetTsmTransitionThreshold );
	return BERR_SUCCESS;
#else    
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(i32TsmTransitionThreshold);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif    
}



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
	int32_t				i32TsmSTThreshold   /* [in] The new ST threshold */
	)
{
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
	uint32_t regAddr;
    uint32_t ui32RegVal;

	BDBG_ENTER(BRAP_DSPCHN_SetTsmSTThreshold);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.ui32TsmSTThresh = i32TsmSTThreshold;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 + 
				hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		ui32RegVal = BRAP_Read32(hRapCh->hRegister, regAddr);

		ui32RegVal &= ~(BCHP_MASK (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_ST_THRESHOLD));    
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_ST_THRESHOLD, i32TsmSTThreshold));    

		BRAP_Write32(hRapCh->hRegister, regAddr, ui32RegVal);
	}

#else
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(i32TsmSTThreshold);
#endif    

	BDBG_LEAVE(BRAP_DSPCHN_SetTsmSTThreshold);
    return BERR_SUCCESS;
}


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
	int32_t				i32TsmGAThreshold   /* [in] The new GA threshold */
	)
{
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
	uint32_t regAddr;
    uint32_t ui32RegVal;

	BDBG_ENTER(BRAP_DSPCHN_SetTsmGAThreshold);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.ui32TsmGAThresh = i32TsmGAThreshold;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 + 
				hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		ui32RegVal = BRAP_Read32(hRapCh->hRegister, regAddr);

		ui32RegVal &= ~(BCHP_MASK (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_GA_THRESHOLD));    
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
						TSM_GA_THRESHOLD, 
						i32TsmGAThreshold));    

		BRAP_Write32(hRapCh->hRegister, regAddr, ui32RegVal);
	}
#else
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(i32TsmGAThreshold);
#endif    

	BDBG_LEAVE(BRAP_DSPCHN_SetTsmGAThreshold);
    return BERR_SUCCESS;
}

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
	)
{
#if (BRAP_7401_FAMILY == 1) && (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1) 
	uint32_t regAddr;
	uint32_t ui32RegVal;

	BDBG_ENTER(BRAP_DSPCHN_SetTsmSLThreshold);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.ui32TsmSLThresh = i32TsmSLThreshold;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 + 
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		ui32RegVal = BRAP_Read32(hRapCh->hRegister, regAddr);

		ui32RegVal &= ~(BCHP_MASK (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_SL_THRESHOLD));    
		ui32RegVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
						TSM_SL_THRESHOLD, 
						i32TsmSLThreshold));    

		BRAP_Write32(hRapCh->hRegister, regAddr, ui32RegVal);
	}
#else
	BSTD_UNUSED(hRapCh);
	BSTD_UNUSED(i32TsmSLThreshold);
#endif

	BDBG_LEAVE(BRAP_DSPCHN_SetTsmSLThreshold);
	return BERR_SUCCESS;
}

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
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS value */
	)
{
	BDBG_ENTER(BRAP_DSPCHN_GetCurrentPTS);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BRAP_DSPCHN_P_GetCurrentPTS(hRapCh, false, psPtsInfo);
	}
	else
	{
        	BDBG_ERR(("Can not ge the PTS information when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}

	BDBG_LEAVE(BRAP_DSPCHN_GetCurrentPTS);

	return BERR_SUCCESS;
}

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
	
	)
{
	BERR_Code ret = BERR_SUCCESS;
	uint32_t regVal = 0x0;

	BDBG_ENTER (BRAP_DSPCHN_GetPictureTag);

	/* Check input parameters */
	BDBG_ASSERT(hRapCh);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )	
	{
		regVal = BRAP_Read32(hRapCh->hRegister ,BCHP_AUD_DSP_CFG0_PICTURE_TAG_CXT0+
							hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);	
	}
	else
	{
        	BDBG_ERR(("Can not get current Picture Tag value when the channel is not in running state."
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}

	*pulPictureTag = regVal;

	BDBG_LEAVE(BRAP_DSPCHN_GetPictureTag);

	return ret;
}

/***************************************************************************
Summary: Get stream Information. This function should be called only 
after the channel has been started.

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
	BERR_NOT_SUPPORTED - if channel has not been started.
	BRAP_ERR_STREAMINFO_NOT_AVAILABLE - If Stream Info is not available.	
**************************************************************************/

BERR_Code
BRAP_DSPCHN_GetStreamInformation(
	BRAP_ChannelHandle	hRapCh,		/* [in] Audio channel handle */
	BRAP_DSPCHN_StreamInfo *psStreamInfo	/* [out] Stream Information */
	)
{
	BRAP_DSPCHN_Handle hDspCh;
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psStreamInfo);

    hDspCh = hRapCh->sModuleHandles.hDspCh;

    if (hRapCh->bStarted == false)
    {
        BDBG_ERR(("Can get stream info only after the channel has been started."
            "This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* If the decoder has not locked, or we havent got a valid sample rate, 
    return an error. Note that FW writes in the sample rate, along with other 
    Stream info registers in one shot and then triggers the sample rate 
    interrupt. So we should not be reading these regsiters until we've recieved 
    the first sample rate interrupt */
	if(BAVC_AudioSamplingRate_eUnknown==hDspCh->eSamplingRate)
	{   
	    /* Note: hDspCh->eSamplingRate gets set to a valid sampling rate when 
	    the interrupt occurs. Its set in the function BRAP_DSPCHN_P_GetStreamInformation_isr */
        BDBG_ERR(("Stream info not available: Sample rate interrupt has not occured "));
        return BERR_TRACE(BRAP_ERR_STREAMINFO_NOT_AVAILABLE);
	}

    /* check for decoder lock is inside BRAP_DSPCHN_P_GetStreamInformation() */
	rc = BRAP_DSPCHN_P_GetStreamInformation(hDspCh, false, psStreamInfo);

	return rc;
}

/***************************************************************************
Summary: Get stream Information in ISR context

Description:
	This function returns stream information of the decoded stream, if
	such information is available (i.e. DSP firmware has decoded atleast
	one audio frame and initialized stream information registers).
	Otherwise it returns error.
	
Returns:
	BERR_SUCCESS - If successful, error on failure.

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_GetStreamInformation_isr(
	BRAP_DSPCHN_Handle	 hDspCh,			/* [in] DSP channel handle */
	BRAP_DSPCHN_P_StreamInfo *psPrivateStreamInfo	/* [out] Stream Information */
	)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ASSERT(hDspCh);
	BDBG_ASSERT(psPrivateStreamInfo);

	rc = BRAP_DSPCHN_P_GetStreamInformation(hDspCh, true, &psPrivateStreamInfo->sStreamInfo);

	/* Initialize sample rate change information structure */
	psPrivateStreamInfo->sPrivateFsChangeInfo.sSampleRateChangeInfo.eType
    		= psPrivateStreamInfo->sStreamInfo.eType;    
    
    if (rc != BERR_SUCCESS) {
    	psPrivateStreamInfo->sPrivateFsChangeInfo.bFsChanged = false;
    	psPrivateStreamInfo->sPrivateFsChangeInfo.sSampleRateChangeInfo.eSamplingRate
    		= BAVC_AudioSamplingRate_eUnknown;
    	hDspCh->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;            
        return BERR_TRACE(rc);
    }
    
	psPrivateStreamInfo->sPrivateFsChangeInfo.bFsChanged = true;
	psPrivateStreamInfo->sPrivateFsChangeInfo.sSampleRateChangeInfo.eSamplingRate
		= psPrivateStreamInfo->sStreamInfo.eSamplingRate;

    /* Store current stream sampling rate for future sampling rate detection */
	hDspCh->eSamplingRate = psPrivateStreamInfo->sStreamInfo.eSamplingRate;

	return rc;
}

/***************************************************************************
Summary:
	Gets DSP context default settings

Description:
	The default configuration for DSP context (DSPCHN channel) are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_GetDefaultSettings( 
   BRAP_DSPCHN_P_Settings	*psDefSettings	/* [out] The DEC channel default configuration setting */
   )
{
	/* Assert on input parameters */
	BDBG_ASSERT(psDefSettings);
	*psDefSettings = sDefDspChnSettings;
	return BERR_SUCCESS;
}

/***************************************************************************
Summary:
	Gets DSP context default params

Description:
	The default parameters for DSP context (DSPCHN channel) are returned
	
Returns:
    BERR_SUCCESS                - if successful 

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_GetDefaultParams( 
   BRAP_DSPCHN_P_AudioParams	*psDefParams	/* [out] The DEC channel default parameters*/
   )
{
	BDBG_ASSERT(psDefParams);
	
	BKNI_Memset(psDefParams,0,sizeof(*psDefParams));
	psDefParams->sExtAudioParams.eOutputSampleRate = BAVC_AudioSamplingRate_eUnknown;
    psDefParams->sExtAudioParams.ePBRate = BRAP_DSPCHN_PlayBackRate_e100;
	psDefParams->sExtAudioParams.bProgramPLL=true;
	return BERR_SUCCESS;
}

/***************************************************************************
Summary: Opens a DSP context 

Description:
	This function is responsible for opening a DSP context (DSPCHN channel). 
	When a DSP context is opened, it will create a module channel handle 
	and configure the module based on the channel settings. 
	Once a channel is opened, it must be closed before it can be opened 
	again.

Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Close,
	BRAP_DSPCHN_P_GetDefaultSettings
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_Open(
	BRAP_DSPCHN_Handle		*phDspChn,		/* [out] DSP channel handle */
	BRAP_DSP_Handle			hDsp,			/* [in] DSP handle */
	uint32_t				channelIndex,	/* [in] Channel index starting from 0 to max number of channels */ 
	const BRAP_DSPCHN_P_Settings	*pDefSettings	/* [in] The DSP channel configuration setting */
	)
{
	BERR_Code rc;
	BRAP_DSPCHN_Handle	handle;

	BDBG_ENTER(BRAP_DSPCHN_P_Open);

	/* Assert the function arguments */
	BDBG_ASSERT(phDspChn);
	BDBG_ASSERT(hDsp);

	BSTD_UNUSED(channelIndex);

	if (!BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)) {
		/* Allocate DspChn handle */
		handle = (BRAP_DSPCHN_Handle)BKNI_Malloc(sizeof(BRAP_DSPCHN_P_Channel));
		if(handle == NULL)
		{
			rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
			return rc;
		}

		*phDspChn = handle;

		/* Reset the entire structure */
		BKNI_Memset( handle, 0, sizeof(BRAP_DSPCHN_P_Channel));

		if (pDefSettings==NULL)
			handle->sSettings = sDefDspChnSettings;
		else 
			handle->sSettings = *pDefSettings;
		
		handle->hDsp = hDsp;
		handle->hChip = hDsp->hChip;
		handle->hRegister = hDsp->hRegister;
		handle->hHeap = hDsp->hHeap;
		handle->hInt = hDsp->hInt;
		handle->eChState = BRAP_DSPCHN_P_ChState_eStop; 
		handle->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;

		/* Allocate device memory required by this channel */
		rc = BRAP_DSPCHN_P_AllocMem(*phDspChn);
		if (rc!=BERR_SUCCESS) 
			return BERR_TRACE(rc);

#if (BRAP_DSP_P_7401_NEWMIT==1)
		/* Initialize post process branch related state variables */
		BRAP_DSPCHN_P_InitPp(*phDspChn);
    	BRAP_DSPCHN_P_InitArrayOfConfBufPtrCopy( *phDspChn );
#endif
        
	}
	else { /* (!BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)) */
		handle = *phDspChn; 
	}

	BDBG_LEAVE(BRAP_DSPCHN_P_Open);
	return BERR_SUCCESS;
}

/***************************************************************************
Summary: Closes an DSP context

Description:
	This function is responsible for closing a DSP context and releasing
	all the resources associated with this context.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Open
**************************************************************************/
BERR_Code 
BRAP_DSPCHN_P_Close( 
	BRAP_DSPCHN_Handle hDspChn /* [in] DSP handle */
	)
{
	BRAP_DSP_Handle hDsp;
	unsigned int channelIndex;

	BDBG_ENTER(BRAP_DSPCHN_P_Close);
	
	/* Assert the function arguments */
	BDBG_ASSERT(hDspChn);

	hDsp = hDspChn->hDsp;
	channelIndex = hDspChn->channelIndex;

	/* Free device memory allocated for this DSP channel */
	BRAP_DSPCHN_P_Free(hDspChn);
	
	/* Free DSP channel handle */
	BKNI_Free(hDspChn);

	/* Clear channel handle entry from DSP handle */
	hDsp->phAudChannelHandle[channelIndex] = NULL;

	return BERR_SUCCESS;
}

/***************************************************************************
Summary: Starts a DSP context

Description:
	This function is responsible for starting a DSP context for the given
	input decode mode.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Stop
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_Start(
	BRAP_DSPCHN_Handle	pDspChn,					/* [in] DSPCHN handle */
	const BRAP_DSPCHN_P_AudioParams *psAudioParams	/* [in] Audio decode parameters */
	)
{
	BERR_Code rc = BERR_SUCCESS;
	BRAP_DSPCHN_P_AudioParams sAudioParams;
	
	BDBG_ENTER(BRAP_DSPCHN_Start);

	/* Assert on the arguments */
	BDBG_ASSERT(pDspChn);

	/* Set decoder lock status to unlocked */
	pDspChn->bDecLocked = false;	

	if (BRAP_DSP_P_GetWatchdogRecoveryFlag(pDspChn->hDsp))
		sAudioParams = pDspChn->sDspAudioParams;
	else {

		BDBG_ASSERT(psAudioParams);

		sAudioParams = *psAudioParams;
		/* Copy the params before start as it is used by
	 	* interrupt service routines
	 	*/
		pDspChn->sDspAudioParams = *psAudioParams;

	}


	rc = BRAP_DSPCHN_P_DownloadFirmware(pDspChn, &sAudioParams);
	if (rc!=BERR_SUCCESS)
		return BERR_TRACE(rc);

	rc = BRAP_DSPCHN_P_StartDspCxt(pDspChn, &sAudioParams);
	if (rc!=BERR_SUCCESS)
		return BERR_TRACE(rc);
 
	/* Set DSPCHN state variables */ 
	pDspChn->eChState = BRAP_DSPCHN_P_ChState_eStart;

#if BCHP_7411_VER > BCHP_VER_C0
	/* Reset first start-stop pts pair programmed to false */
	pDspChn->bFirstPtsPairProgrammed = false;

	/* Clear BRAP_DSPCHN_P_PtsBasedStartStop structure in DSPCHN handle */
	pDspChn->sPtsBasedStartStop.ui32NumPtsPairProgrammed = 0;
	pDspChn->sPtsBasedStartStop.ui32StartPts0 = BRAP_DSPCHN_UNBOUNDED_START_PTS;
	pDspChn->sPtsBasedStartStop.ui32StopPts0 = BRAP_DSPCHN_UNBOUNDED_STOP_PTS;
	pDspChn->sPtsBasedStartStop.ui32StartPts1 = BRAP_DSPCHN_UNBOUNDED_START_PTS;
	pDspChn->sPtsBasedStartStop.ui32StopPts1 = BRAP_DSPCHN_UNBOUNDED_STOP_PTS;
#endif

	BDBG_LEAVE(BRAP_DSPCHN_Start);
	return rc;
}

/***************************************************************************
Summary: Stops a DSP context

Description:
	This function stops a currently running context
	
Returns:
	BERR_SUCCESS - If successful

See Also:
	BRAP_DSPCHN_P_Start
**************************************************************************/
BERR_Code
BRAP_DSPCHN_P_Stop(
	BRAP_DSPCHN_Handle	pDspChn					/* [in] DSPCHN handle */
	)
{
  	BERR_Code err = BERR_SUCCESS;
#if (BRAP_DSP_P_7401_NEWMIT==1)

	unsigned int uiPpBrIndex = 0, uiPpStgIndex = 0;
	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;
#endif

    BDBG_ENTER(BRAP_DSPCHN_Stop);

    /* Assert on the arguments */
    BDBG_ASSERT(pDspChn);

	pDspChn->eChState = BRAP_DSPCHN_P_ChState_eStop;
    pDspChn->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	err = BRAP_DSPCHN_P_DecrementCtxBufUsage(pDspChn->hDsp, 
			pDspChn->sDspAudioParams.sExtAudioParams.eType,
			pDspChn->sDspAudioParams.sExtAudioParams.eDecodeMode);
	if (err!=BERR_SUCCESS)
		return BERR_TRACE(err);

	if (((pDspChn->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode) ||
		(pDspChn->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) &&
		(!BRAP_DSP_P_GetWatchdogRecoveryFlag(pDspChn->hDsp))) 

    {        
		BRAP_DSP_FreeInterframeBuf(pDspChn->hDsp,  pDspChn->uiInterframeBufIndex);


#if (BRAP_DSP_P_7401_NEWMIT==1)

		/* Free interframe buffers of post processing stages */
		for( uiPpBrIndex = 0; uiPpBrIndex < uiMaxPpBranches; uiPpBrIndex++ )
		{
			for( uiPpStgIndex = 0; uiPpStgIndex < uiMaxPpStgInBranch; uiPpStgIndex++ )
			{
				if( BRAP_DSPCHN_PP_Algo_eMax != 
					pDspChn->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex] )
				{
					BRAP_DSP_P_FreePpInterframeBuf(pDspChn->hDsp, pDspChn->uiPpInterframeBuf[uiPpBrIndex][uiPpStgIndex]);
				}
			}
		}
		BRAP_DSPCHN_P_FreeIsIsIfBuffers( pDspChn );
#endif        

		/* Free configuration buffer of decode stage */
		BRAP_DSP_P_FreeDecConfigBuf(pDspChn->hDsp, pDspChn->uiDecConfigBufIndex);

		/* Free array of configuration buffer pointers */
		BRAP_DSP_P_FreeConfigBufArray(pDspChn->hDsp, pDspChn->uiConfigBufArrayIndex);

		/* Free configuration buffer of post processing stages */
		for( uiPpBrIndex = 0; uiPpBrIndex < uiMaxPpBranches; uiPpBrIndex++ )
		{
			for( uiPpStgIndex = 0; uiPpStgIndex < uiMaxPpStgInBranch; uiPpStgIndex++ )
			{
				/* Note: We have allocated buffer for AAC post processing stage 0 even if 
				 * application has not added any PP algorithm for this stage */
				if (BRAP_DSPCHN_PP_Algo_eMax != pDspChn->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex])
				{
					BRAP_DSP_P_FreeDecConfigBuf( pDspChn->hDsp, pDspChn->uiPpConfigBufIndex[uiPpBrIndex][uiPpStgIndex]);
				}
			}
		}
	}

	/*To check for Reset of the Hardware*/
	if(pDspChn->hDsp->hRap->bResetHwDone==false)
	{
		err = BRAP_DSPCHN_P_StopDspCxt(pDspChn);
		if (err!=BERR_SUCCESS)
			return BERR_TRACE(err);
	}

	/* Set decoder lock status to unlocked */
	pDspChn->bDecLocked = false;
    
	BDBG_LEAVE(BRAP_DSPCHN_Stop);
    return err;
}

/***************************************************************************
Summary: Set AC3 decoder configuration parameters
 
Description:
	This function sets configuration parameters of AC3 decoder

Returns:
	BERR_SUCCESS - If successful

See Also: BRAP_DSPCHN_GetAc3ConfigurationParams
**************************************************************************/
BERR_Code
BRAP_DSPCHN_SetAc3ConfigurationParams(
	BRAP_ChannelHandle		hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_Ac3ConfigParams	*psAc3ConfigParams 	/* [in] AC3 decoder configuration parameters */
	)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BRAP_DSPCHN_SetAc3ConfigurationParams);

	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psAc3ConfigParams);

	hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3ConfigParam  
			= *psAc3ConfigParams;
	/* Update dual mono field in channel handle */
	switch (psAc3ConfigParams->eDualMono) {
		case BRAP_DSPCHN_Ac3DualMonoMode_eStereo:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode
				= BRAP_DSPCHN_DualMonoMode_eStereo;
			break;
		case BRAP_DSPCHN_Ac3DualMonoMode_eLtMono:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode 
				= BRAP_DSPCHN_DualMonoMode_eLeftMono;
			break;
		case BRAP_DSPCHN_Ac3DualMonoMode_eRtMono:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode
				= BRAP_DSPCHN_DualMonoMode_eRightMono;
			break;
		case BRAP_DSPCHN_Ac3DualMonoMode_eDualMono:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode 
				= BRAP_DSPCHN_DualMonoMode_eDualMonoMix;
			break;
		default:
			break;
	}
	switch(psAc3ConfigParams->eDualMono)
	{
		case 0:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eStereo;
			break;
		case 1:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eLeftMono;
			break;
		case 2:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eRightMono;
			break;
		case 3:
			hRapCh->sModuleHandles.hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode = BRAP_DSPCHN_DualMonoMode_eDualMonoMix;
			break;
		default:
			BDBG_ERR(("Invalid Dual mono mode of Ac3 %#x",psAc3ConfigParams->eDualMono));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Program parameters into decoder configuration only if following checks are satisfied.
	 * 1. Channel is in "decode start" state
	 * 2. Decode mode is either eDecode or eSimulMode
	 * 3. Audio type currently getting decoded is AC3.
	 * If any one of the above conditions not met, these parameters will be stored in handle and 
	 * programmed on next "decode start".
	 */
	if (hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart) {
		/* Parameters are programmed only if decode mode is either eDecode or eSimultaneous AND 
		 * parameter's audio type matches with audio type currently getting decoded */
		if (((hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
			||(hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode))
			&& (hRapCh->sModuleHandles.hDspCh->sDspAudioParams.sExtAudioParams.eType==BRAP_DSPCHN_AudioType_eAc3))
		{
			rc = BRAP_DSPCHN_P_SetDecConfigurationParams (hRapCh->sModuleHandles.hDspCh);
			if (rc!=BERR_SUCCESS)
				return BERR_TRACE(rc);
		}
	}
	
	BDBG_LEAVE(BRAP_DSPCHN_SetAc3ConfigurationParams);
	return rc;
}

BERR_Code
BRAP_DSPCHN_P_SetConfig (
	BRAP_DSPCHN_Handle	hDspCh,
	BRAP_DEC_DownmixPath	eDownmixPath,
	BRAP_DEC_ConfigParams	*psDecConfigParams
	)
{
	BERR_Code err = BERR_SUCCESS;
    unsigned int uiNextArray = 0, uiNextConfBuf = 0;
    
    BDBG_ENTER(BRAP_DSPCHN_P_SetConfig);    

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (psDecConfigParams);

	hDspCh->sSettings.eOutputMode[eDownmixPath] = psDecConfigParams->eOutputMode;
	hDspCh->sSettings.bMonoToAll[eDownmixPath] = psDecConfigParams->bMonoToAll;

	BDBG_MSG(("BRAP_DSPCHN_P_SetConfig: eOutputMode=%d, bMonoToAll=%d", 
		psDecConfigParams->eOutputMode, psDecConfigParams->bMonoToAll)); 
    
	hDspCh->sSettings.sDspchnExtSettings[eDownmixPath].eDualMonoMode
		= psDecConfigParams->eDualMonoMode;
	hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sDdbmConfigParams
		= psDecConfigParams->sDdbmConfigParams;

	switch (psDecConfigParams->eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
#ifdef RAP_SRSTRUVOL_CERTIFICATION          
		case BRAP_DSPCHN_AudioType_ePCM:
#endif			
		case BRAP_DSPCHN_AudioType_eWmaStd:			
			/* TODO: Store programmed MPEG parameters into DSP channel handle */
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			/* Store programmed AAC parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAacConfigParam
				= psDecConfigParams->uConfigParams.sAacConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			/* Store programmed AACSBR parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAacSbrConfigParam
				= psDecConfigParams->uConfigParams.sAacSbrConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
			/* Store programmed AC3 parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3ConfigParam  
				= psDecConfigParams->uConfigParams.sAc3ConfigParams;
			/* Update DualMono field in AC3 Config structure also */
			switch (psDecConfigParams->eDualMonoMode) {
				case BRAP_DSPCHN_DualMonoMode_eLeftMono:
					hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3ConfigParam.eDualMono
						=	BRAP_DSPCHN_Ac3DualMonoMode_eLtMono;
					break;	
				case BRAP_DSPCHN_DualMonoMode_eRightMono:
					hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3ConfigParam.eDualMono
						= BRAP_DSPCHN_Ac3DualMonoMode_eRtMono;
					break;	
				case BRAP_DSPCHN_DualMonoMode_eStereo:
					hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3ConfigParam.eDualMono
						= BRAP_DSPCHN_Ac3DualMonoMode_eStereo;
					break;	
				case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
					hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3ConfigParam.eDualMono
						= BRAP_DSPCHN_Ac3DualMonoMode_eDualMono;
					break;
				default:
					break;
			}
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			/* Store programmed AC3 LOSSLESS parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3LosslessConfigParam  
				= psDecConfigParams->uConfigParams.sAc3LosslessConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			/* Store programmed AC3Plus parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sAc3PlusConfigParam
				= psDecConfigParams->uConfigParams.sAc3PlusConfigParams;
			break;
        case BRAP_DSPCHN_AudioType_ePcmWav:
			/* Store programmed AC3Plus parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sPcmWavConfigParams
				= psDecConfigParams->uConfigParams.sPcmWavConfigParams;
            break;            
#if (BRAP_DTS_SUPPORTED == 1)		
		case BRAP_DSPCHN_AudioType_eDts:
			/* Store programmed DTS parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sDtsCoreConfigParam
				= psDecConfigParams->uConfigParams.sDtsCoreConfigParams;
			break;
#else            
		case BRAP_DSPCHN_AudioType_eDts:
			/* Store programmed DTS parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sDtsConfigParam
				= psDecConfigParams->uConfigParams.sDtsConfigParams;
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eDtshd:
			/* Store programmed DTS HD parameters into DSP channel handle */
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sDtshdConfigParam
				= psDecConfigParams->uConfigParams.sDtshdConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sBdlpcmConfigParam
				= psDecConfigParams->uConfigParams.sBdlpcmConfigParams;
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sLpcmHdDvdConfigParam
				= psDecConfigParams->uConfigParams.sLpcmHdDvdConfigParams;
			break;            
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sLpcmDvdConfigParam
				= psDecConfigParams->uConfigParams.sLpcmDvdConfigParams;
			break;            
#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		case BRAP_DSPCHN_AudioType_eInvalid:
		case BRAP_DSPCHN_AudioType_eMax:
			BDBG_MSG(("Skipping setting of decoder configuration parameters "));
			break;     
#endif            
		case BRAP_DSPCHN_AudioType_eMlp:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sMlpConfigParams
				= psDecConfigParams->uConfigParams.sMlpConfigParams;
			break;					
		case BRAP_DSPCHN_AudioType_eWmaPro:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sWmaProConfigParams
				= psDecConfigParams->uConfigParams.sWmaProConfigParams;
			break;			
		case BRAP_DSPCHN_AudioType_eDra:
			hDspCh->sSettings.sDspchnCfgParams[eDownmixPath].sDraConfigParams
				= psDecConfigParams->uConfigParams.sDraConfigParams;          
            break;
		default:
			BDBG_ERR(("Audio Type %d not supported",psDecConfigParams->eType));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* Program parameters into decoder configuration only if following checks are satisfied.
	 * 1. Channel is in "decode start" state
	 * 2. Decode mode is either eDecode or eSimulMode
	 * 3. Parameter's audio type matches with audio type currently getting decoded.
	 * If any one of the above conditions not met, these parameters will be stored in handle and 
	 * programmed on next "decode start".
	 */
	if (hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart) {
	 
        err = BRAP_DSPCHN_P_GetNextArrayOfConfBufPtr( hDspCh, &uiNextArray);
		if( err != BERR_SUCCESS )
			return BERR_TRACE( err );

		if (((hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
			||(hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode))
			&& (psDecConfigParams->eType==hDspCh->sDspAudioParams.sExtAudioParams.eType))
		{
			err = BRAP_DSPCHN_P_GetNextDecConfigBuf( hDspCh, &uiNextConfBuf );
			if( err != BERR_SUCCESS )
				return BERR_TRACE( err );

			/* Update copy of array of pointers in system memory with next dec config buf address */
			hDspCh->uiCurrentArrayOfConfBufPtrCopy[0][BAF_MAX_POSTPROCESS_STAGES] = uiNextConfBuf;
			
			err = BRAP_DSPCHN_P_ProgramDecConfigBuf( hDspCh, uiNextConfBuf );
			if( err != BERR_SUCCESS )
				return BERR_TRACE( err); 
		}
		BRAP_DSPCHN_P_ProgramArrayOfConfBufPtr( hDspCh, uiNextArray );
	}
    BDBG_LEAVE(BRAP_DSPCHN_P_SetConfig);
	return err;
}

BERR_Code
BRAP_DSPCHN_P_SetPPConfig (
	BRAP_DSPCHN_Handle	hDspCh,
	BRAP_PP_ConfigParams		*psPpConfigParsms	
	)
{
	BERR_Code err = BERR_SUCCESS;
	unsigned int uiNextArray = 0, uiNextConfBuf = 0;
	unsigned int uiPpBranch = 0, uiPpStg = 0;
	unsigned int i = 0, j = 0;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (psPpConfigParsms);


	uiPpBranch = psPpConfigParsms->uiPpBranchId;
	uiPpStg = psPpConfigParsms->uiPpStage;

	
	if ((BRAP_DSPCHN_PP_Algo_eMax == psPpConfigParsms->ePpAlgo)&&(BAF_MAX_POSTPROCESS_STAGES == psPpConfigParsms->uiPpStage))
	{
		for(i = 0; i < BAF_MAX_BRANCHES; i++)
		{
			for (j = 0; j<BAF_MAX_POSTPROCESS_STAGES;j++)
			{
				hDspCh->bUseDefaultConfig[i][j] = true;
			}
		}
	}

	if(( BRAP_DSPCHN_PP_Algo_eMax != psPpConfigParsms->ePpAlgo) &&
       ( BRAP_DSPCHN_PP_Algo_eDdbm != psPpConfigParsms->ePpAlgo) &&
       ( BRAP_DSPCHN_PP_Algo_eSrc!= psPpConfigParsms->ePpAlgo))
	{
		if( hDspCh->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg] 
			!= psPpConfigParsms->ePpAlgo )
		{
			BDBG_ERR((" Post processing algorithm mismatch "));
			BDBG_ERR((" Post processing stage %d on branch %d is set for algorithm %d",
				uiPpStg, uiPpBranch, hDspCh->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]));
			return BERR_TRACE( BERR_INVALID_PARAMETER );
		}
		hDspCh->bUseDefaultConfig[uiPpBranch][uiPpStg] = false;
	}
    
	BDBG_MSG (("uiPpStg = %d \n\n",uiPpStg));
	switch( psPpConfigParsms->ePpAlgo )
	{
#if (BRAP_AD_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
			hDspCh->uPpStgConfigParam[uiPpBranch][uiPpStg].sAdFadeConfigParams
				= psPpConfigParsms->uConfigParams.sAdFadeConfigParams;
			break;
		case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
			hDspCh->uPpStgConfigParam[uiPpBranch][uiPpStg].sAdPanConfigParams
				= psPpConfigParsms->uConfigParams.sAdPanConfigParams;
			break;
#endif	
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
			hDspCh->uPpStgConfigParam[uiPpBranch][uiPpStg].sDolbyVolumeConfigParams
				= psPpConfigParsms->uConfigParams.sDolbyVolumeConfigParams;
			break;                       
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
		case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
			hDspCh->uPpStgConfigParam[uiPpBranch][uiPpStg].sSRSTruVolConfigParams
				= psPpConfigParsms->uConfigParams.sSRSTruVolConfigParams;
			break;                       
#endif			
		case BRAP_DSPCHN_PP_Algo_eSrc:
			break;
		case BRAP_DSPCHN_PP_Algo_eMax:
			BDBG_MSG(("Skipping setting of post processing configuration parameters "));
			break;
		default:
			BDBG_ERR(("Post Processing Type %d not supported", psPpConfigParsms->ePpAlgo));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if (hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart)
	{
		err = BRAP_DSPCHN_P_GetNextArrayOfConfBufPtr( hDspCh, &uiNextArray);
		if( err != BERR_SUCCESS )
			return BERR_TRACE( err );
		
		if( BRAP_DSPCHN_PP_Algo_eMax!=psPpConfigParsms->ePpAlgo )
		{
			err = BRAP_DSPCHN_P_GetNextPpConfigBuf( hDspCh, 
				uiPpBranch,
				uiPpStg, &uiNextConfBuf);
			if( err != BERR_SUCCESS )
				return BERR_TRACE( err );

			/* Update copy of array of pointers in system memory with next pp config buf address */
			hDspCh->uiCurrentArrayOfConfBufPtrCopy[uiPpBranch][uiPpStg] = uiNextConfBuf;
			
			err = BRAP_DSPCHN_P_ProgramPpConfigBuf( hDspCh, psPpConfigParsms->ePpAlgo, uiNextConfBuf,
				hDspCh->bUseDefaultConfig[uiPpBranch][uiPpStg], uiPpBranch, uiPpStg );
			if( err != BERR_SUCCESS )
				return BERR_TRACE( err); 
		}
		BRAP_DSPCHN_P_ProgramArrayOfConfBufPtr( hDspCh, uiNextArray );
	}
	
	return err;
}

static BERR_Code
BRAP_DSPCHN_P_SetDecConfigurationParams(
	BRAP_DSPCHN_Handle		hDspCh			/* [in] DSP channel handle */
	)
{
	BERR_Code rc = BERR_SUCCESS;
	unsigned int bufPtr, bufPtr1, bufPtr2, count, memVal;
	unsigned char *decParamsArray=(unsigned char *)BKNI_Malloc(SIZE_OF_BAF_DEC_CONFIG);
	unsigned char  *decParamsPtr;
	uint32_t buf1PhysAdr, buf2PhysAdr, bufPhysAdr;
	
#ifndef BCHP_7411_VER
	BSTD_UNUSED (memVal);
#else
	BSTD_UNUSED (bufPhysAdr);
#endif


	/* Read buffer address last programmed by host */
	bufPtr1 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset);
	/* Read buffer address that is currently in use by decoder */
	bufPtr2 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER1_CXT0 + hDspCh->chCfgRegOffset);

	/* If decoder is currently not using the buffer last programmed by host
	 * then host needs to wait w/ timeout, till decoder switches to this buffer.
	 * This check is not required when decode is not started.
	 */
	if (hDspCh->eChState!=BRAP_DSPCHN_P_ChState_eStop) {
		count = 0;
		while (bufPtr1!=bufPtr2) {
			/* Wait for 5 frames time and then timeout */
			BKNI_Sleep(BRAP_DSPCHN_P_ONE_FRAME_TIME);
			bufPtr1 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset);
			bufPtr2 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER1_CXT0 + hDspCh->chCfgRegOffset);
			count++;
			if (count==5) {
				if (bufPtr1!=bufPtr2)
				{
					BKNI_Free(decParamsArray);
					return BERR_TRACE(BERR_TIMEOUT);
				}
			}
		}
	}

	/* Select next buffer to be programmed */
	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)hDspCh->sChMemInfo.uiAc3ConfBuffer1, &buf1PhysAdr);
	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)hDspCh->sChMemInfo.uiAc3ConfBuffer2, &buf2PhysAdr);
		
#ifdef BCHP_7411_VER /* Only for the 7411 */ 
	/* For 7411, pass physical address for memory access */
	if (bufPtr2==buf1PhysAdr)
		bufPtr = buf2PhysAdr;
	else if (bufPtr2==buf2PhysAdr)
		bufPtr = buf1PhysAdr;
	else
	{
		BKNI_Free(decParamsArray);
		/* If decoder buffer address doesn't match with allocated buffer addreses
		 * then device is in bad state */  
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
#else /* For chips other than 7411 */
	/* For 7401 etc pass virtual address to memory access */
	if (bufPtr2==buf1PhysAdr)
	{
		bufPtr = (unsigned int)hDspCh->sChMemInfo.uiAc3ConfBuffer2;
		bufPhysAdr = buf2PhysAdr;
	}
	else if (bufPtr2==buf2PhysAdr)
	{
		bufPtr = (unsigned int)hDspCh->sChMemInfo.uiAc3ConfBuffer1;
		bufPhysAdr = buf1PhysAdr;
	}
	else
	{
		BKNI_Free(decParamsArray);
		/* If decoder buffer address doesn't match with allocated buffer addreses
		 * then device is in bad state */  
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
	}
#endif	

	decParamsPtr = decParamsArray; 
	switch (hDspCh->sDspAudioParams.sExtAudioParams.eType) {
		case BRAP_DSPCHN_AudioType_eAc3:
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)         
			BRAP_DSPCHN_P_FormAc3ConfigStruct (hDspCh, decParamsPtr); 
			break;
#else
			BRAP_DSPCHN_P_FormDDPConfigStructForAC3(hDspCh, decParamsPtr); 
			break;
#endif
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			BRAP_DSPCHN_P_FormAc3LosslessConfigStruct (hDspCh, decParamsPtr); 
			break;
		case BRAP_DSPCHN_AudioType_eMpeg:
			BRAP_DSPCHN_P_FormMpegConfigStruct (hDspCh, decParamsPtr); 
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			BRAP_DSPCHN_P_FormAacConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			BRAP_DSPCHN_P_FormAacSbrConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			BRAP_DSPCHN_P_FormAc3PlusConfigStruct (hDspCh, decParamsPtr);
			break;
#if (BRAP_DTS_SUPPORTED == 1)		
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSPCHN_P_FormDtsCoreConfigStruct (hDspCh, decParamsPtr);
			break;
#else            
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSPCHN_P_FormDtsConfigStruct (hDspCh, decParamsPtr);
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			BRAP_DSPCHN_P_FormBdLpcmConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			BRAP_DSPCHN_P_FormLpcmHdDvdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eDtshd:
			BRAP_DSPCHN_P_FormDtshdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			BRAP_DSPCHN_P_FormLpcmDvdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eMlp:
			BRAP_DSPCHN_P_FormMlpConfigStruct(hDspCh, decParamsPtr);
			break;
#ifdef RAP_SRSTRUVOL_CERTIFICATION  
		case BRAP_DSPCHN_AudioType_ePCM:
			break;
#endif			
		case BRAP_DSPCHN_AudioType_eWmaPro:
			BRAP_DSPCHN_P_FormWmaProConfigStruct(hDspCh, decParamsPtr);
			break;	
		case BRAP_DSPCHN_AudioType_eWmaStd:
			BRAP_DSPCHN_P_FormWmaStdConfigStruct(hDspCh, decParamsPtr);
            break;
		case BRAP_DSPCHN_AudioType_ePcmWav:
			BRAP_DSPCHN_P_FormPcmWavConfigStruct(hDspCh, decParamsPtr);            
			break;			            
		case BRAP_DSPCHN_AudioType_eDra:
			BRAP_DSPCHN_P_FormDraConfigStruct(hDspCh, decParamsPtr);
			break;			                        
		default:
			BDBG_ERR(("Audio type %d not supported", hDspCh->sDspAudioParams.sExtAudioParams.eType));
			BKNI_Free(decParamsArray);
			return BERR_TRACE( BERR_INVALID_PARAMETER ); 
	}

	/* Write programmed structure into DRAM */
	decParamsPtr = decParamsArray;
	for (count = 0; count < hDspCh->hDsp->memInfo.decConfigBufSize; count+=4) {
#ifdef BCHP_7411_VER /* Only for the 7411 */
		memVal = (*(decParamsPtr+count) << 24)|(*(decParamsPtr+count+1) << 16)|(*(decParamsPtr+count+2) << 8)|(*(decParamsPtr+count+3));
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, bufPtr+count, memVal);
#else
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, bufPtr+count, *(uint32_t *)(decParamsPtr + count)); 
#endif
    }

		
	BRAP_P_DummyMemRead32((BARC_Handle)hDspCh->hRegister, bufPtr+count);

	/* Inform decoder to switch to new buffer */
#ifdef BCHP_7411_VER /* Only for the 7411 */
	BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset, bufPtr); 
#else
	BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset, bufPhysAdr); 
#endif
	BKNI_Free(decParamsArray);
	return rc;
}


static void BRAP_DSPCHN_P_InitArrayOfConfBufPtrCopy(
	BRAP_DSPCHN_Handle		hDspCh
	)
{
	unsigned int count1 = 0, count2 = 0;

	/* Update next array of pointer in dram */
	for (count1 = 0; count1 < BAF_MAX_BRANCHES; count1++)
	{
		for( count2 = 0; count2 < (BAF_MAX_POSTPROCESS_STAGES + 1 ); count2++ )
		{
			hDspCh->uiCurrentArrayOfConfBufPtrCopy[count1][count2] = 0xFFFFFFFF;
			
		}
	}
}

static BERR_Code BRAP_DSPCHN_P_GetNextArrayOfConfBufPtr(
	BRAP_DSPCHN_Handle		hDspCh,
	unsigned int				*puiArrayInUse
	)
{
	uint32_t uiPingArrayPhysAdr, uiPongArrayPhysAdr, arrayPtr1, arrayPtr2;
	unsigned int count = 0;
	BERR_Code err = BERR_SUCCESS;


	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( puiArrayInUse );
	
	/* Read array address last programmed by host */
	arrayPtr1 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset);
	/* Read array address that is currently in use by decoder */
	arrayPtr2 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER1_CXT0 + hDspCh->chCfgRegOffset);

	/* If decoder is currently not using the array last programmed by host
	 * then host needs to wait w/ timeout, till decoder switches to this array.
	 * This check is not required when decode is not started.
	 */
	if (hDspCh->eChState!=BRAP_DSPCHN_P_ChState_eStop) 
    {
		count = 0;
		if(arrayPtr1!=arrayPtr2) 
        {
			/* Wait for 1 frames time */
			BKNI_Sleep(BRAP_DSPCHN_P_ONE_FRAME_TIME);
			arrayPtr1 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset);
			arrayPtr2 = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER1_CXT0 + hDspCh->chCfgRegOffset);
		}
	}

	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)hDspCh->sChMemInfo.uiConfBufferPingArray, &uiPingArrayPhysAdr);
	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)hDspCh->sChMemInfo.uiConfBufferPongArray, &uiPongArrayPhysAdr);

	if( arrayPtr2==uiPingArrayPhysAdr )
	{
		*puiArrayInUse = hDspCh->sChMemInfo.uiConfBufferPongArray;
	}
	else if( arrayPtr2==uiPongArrayPhysAdr )
	{
		*puiArrayInUse = hDspCh->sChMemInfo.uiConfBufferPingArray;
	}
	else
	{
		return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );
	}
    
	return err;

}

static BERR_Code BRAP_DSPCHN_P_GetNextPpConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int uiPpBranchId,
	unsigned int uiPpStage,
	unsigned int *puiNextConfBuf
	)
{
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( puiNextConfBuf );


	if( hDspCh->uiCurrentArrayOfConfBufPtrCopy[uiPpBranchId][uiPpStage]
		== hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBranchId][uiPpStage] )
	{
		*puiNextConfBuf = hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBranchId][uiPpStage];
	}
	else if( hDspCh->uiCurrentArrayOfConfBufPtrCopy[uiPpBranchId][uiPpStage]
		== hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBranchId][uiPpStage] )
	{
		*puiNextConfBuf = hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBranchId][uiPpStage];
	}
	else
	{
		return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );
	}

	return BERR_SUCCESS;
	
}

static BERR_Code BRAP_DSPCHN_P_GetNextDecConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int *puiNextConfBuf
	)
{
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( puiNextConfBuf );

	if( hDspCh->uiCurrentArrayOfConfBufPtrCopy[0][BAF_MAX_POSTPROCESS_STAGES]
		== hDspCh->sChMemInfo.uiAc3ConfBuffer1 )
	{
		*puiNextConfBuf = hDspCh->sChMemInfo.uiAc3ConfBuffer2;
	}
	else if( hDspCh->uiCurrentArrayOfConfBufPtrCopy[0][BAF_MAX_POSTPROCESS_STAGES]
		== hDspCh->sChMemInfo.uiAc3ConfBuffer2 )
	{
		*puiNextConfBuf = hDspCh->sChMemInfo.uiAc3ConfBuffer1;
	}
	else
	{
		return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );
	}

	return BERR_SUCCESS;
	
}

static BERR_Code BRAP_DSPCHN_P_ProgramDecConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int				uiNextConfBuf
	)
{
	BERR_Code rc = BERR_SUCCESS;
	unsigned char *decParamsPtr;
	unsigned int count = 0;
    uint32_t uiPhysAdr;

	decParamsPtr = ( unsigned char *)BKNI_Malloc( hDspCh->hDsp->memInfo.decConfigBufSize );

	if(decParamsPtr == NULL)
	{
		rc = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
		return rc;
	}
	switch (hDspCh->sDspAudioParams.sExtAudioParams.eType)
	{
		case BRAP_DSPCHN_AudioType_eAc3:
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)         
			BRAP_DSPCHN_P_FormAc3ConfigStruct (hDspCh, decParamsPtr); 
			break;
#else
			BRAP_DSPCHN_P_FormDDPConfigStructForAC3(hDspCh, decParamsPtr); 
			break;
#endif
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			BRAP_DSPCHN_P_FormAc3LosslessConfigStruct (hDspCh, decParamsPtr); 
			break;
		case BRAP_DSPCHN_AudioType_eMpeg:
			BRAP_DSPCHN_P_FormMpegConfigStruct (hDspCh, decParamsPtr); 
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			BRAP_DSPCHN_P_FormAacConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			BRAP_DSPCHN_P_FormAacSbrConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			BRAP_DSPCHN_P_FormAc3PlusConfigStruct (hDspCh, decParamsPtr);
			break;
#if (BRAP_DTS_SUPPORTED == 1)		
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSPCHN_P_FormDtsCoreConfigStruct (hDspCh, decParamsPtr);
			break;
#else            
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSPCHN_P_FormDtsConfigStruct (hDspCh, decParamsPtr);
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			BRAP_DSPCHN_P_FormBdLpcmConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			BRAP_DSPCHN_P_FormLpcmHdDvdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eDtshd:
			BRAP_DSPCHN_P_FormDtshdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			BRAP_DSPCHN_P_FormLpcmDvdConfigStruct (hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eMlp:
			BRAP_DSPCHN_P_FormMlpConfigStruct(hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_eWmaPro:
			BRAP_DSPCHN_P_FormWmaProConfigStruct(hDspCh, decParamsPtr);
			break;	
		case BRAP_DSPCHN_AudioType_eWmaStd:
			BRAP_DSPCHN_P_FormWmaStdConfigStruct(hDspCh, decParamsPtr);
			break;
		case BRAP_DSPCHN_AudioType_ePcmWav:
			BRAP_DSPCHN_P_FormPcmWavConfigStruct(hDspCh, decParamsPtr);            
            break;    
		case BRAP_DSPCHN_AudioType_eDra:
			BRAP_DSPCHN_P_FormDraConfigStruct(hDspCh, decParamsPtr);
			break;	            
#ifdef RAP_SRSTRUVOL_CERTIFICATION  			
		case BRAP_DSPCHN_AudioType_ePCM:			
			break;
#endif			
		default:
			BDBG_ERR(("Audio type %d not supported", hDspCh->sDspAudioParams.sExtAudioParams.eType));
			BKNI_Free(decParamsPtr);
			return BERR_TRACE( BERR_INVALID_PARAMETER ); 
	}
	/* Write programmed structure into DRAM */
	for (count = 0; count < hDspCh->hDsp->memInfo.decConfigBufSize; count+=4)
	{
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, uiNextConfBuf + count, *(uint32_t *)(decParamsPtr + count)); 
	}

	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)uiNextConfBuf, &uiPhysAdr);

	BDBG_MSG(("hDspCh->hDsp->memInfo.decConfigBufSize > 0x%x",hDspCh->hDsp->memInfo.decConfigBufSize ));
	BDBG_MSG(("bufPtr > 0x%x", uiNextConfBuf));
	BDBG_MSG(("bufPhysAdr > 0x%x", uiPhysAdr));

	BKNI_Free( decParamsPtr );
	
	return rc;
	
}

static BERR_Code BRAP_DSPCHN_P_ProgramPpConfigBuf(
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	BRAP_DSPCHN_PP_Algo	ePpAlgo,
	unsigned int				uiNextConfBuf,
	bool						bUseDefault,
	unsigned int				uiPpBranchId,
	unsigned int				uiPpStgId
	)
{
	BERR_Code rc = BERR_SUCCESS;
	unsigned char *decParamsPtr;
	unsigned int count = 0;
    uint32_t uiPhysAdr;

#if (BRAP_AD_SUPPORTED == 0) || (BRAP_DOLBYVOLUME_SUPPORTED == 0) || (BRAP_SRS_TRUVOL_SUPPORTED == 0)
	BSTD_UNUSED(bUseDefault);
	BSTD_UNUSED(uiPpBranchId);
	BSTD_UNUSED(uiPpStgId);	
#endif

	
	decParamsPtr = ( unsigned char *)BKNI_Malloc( hDspCh->hDsp->memInfo.decConfigBufSize );
	if(decParamsPtr == NULL)
	{
		rc = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
		return rc;
	}
	
	switch( ePpAlgo )
	{
#if (BRAP_AD_SUPPORTED == 1)	
		case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
			BRAP_DSPCHN_P_FormFadeConfigStruct( hDspCh, decParamsPtr, bUseDefault, uiPpBranchId, uiPpStgId );
			break;
		case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
			BRAP_DSPCHN_P_FormPanConfigStruct( hDspCh, decParamsPtr, bUseDefault, uiPpBranchId, uiPpStgId  );
			break;
#endif			
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)			
		case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
			BRAP_DSPCHN_P_FormDolbyVolumeConfigStruct( hDspCh, decParamsPtr, bUseDefault, uiPpBranchId, uiPpStgId );
			break;			
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)			
		case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
			BRAP_DSPCHN_P_FormSRSVolumeConfigStruct( hDspCh, decParamsPtr, bUseDefault, uiPpBranchId, uiPpStgId );
			break;			
#endif			
		case BRAP_DSPCHN_PP_Algo_eMax:
		default:
			BDBG_ERR(( "Post processing algorithm %d not supported",ePpAlgo ));
			rc = BERR_TRACE( BERR_INVALID_PARAMETER );
			goto error;
	}

	for (count = 0; count < hDspCh->hDsp->memInfo.decConfigBufSize; count+=4)
	{
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, uiNextConfBuf + count, *(uint32_t *)(decParamsPtr + count)); 
	}

	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)uiNextConfBuf, &uiPhysAdr);

	BDBG_MSG(("hDspCh->hDsp->memInfo.decConfigBufSize > 0x%x",hDspCh->hDsp->memInfo.decConfigBufSize ));
	BDBG_MSG(("PP Config BufPtr > 0x%x", uiNextConfBuf));
	BDBG_MSG(("PP Config BufPhysAdr > 0x%x", uiPhysAdr));

error:
	BKNI_Free( decParamsPtr );

	return rc;
}

static void BRAP_DSPCHN_P_ProgramArrayOfConfBufPtr( 
	BRAP_DSPCHN_Handle		hDspCh,			/* [in] DSP channel handle */
	unsigned int				uiNextArray
	)
{
	unsigned int count1 = 0, count2 = 0;
    uint32_t bufPhysAdr;

	/* Update next array of pointer in dram */
	for (count1 = 0; count1 < BAF_MAX_BRANCHES; count1++)
	{
		for( count2 = 0; count2 < (BAF_MAX_POSTPROCESS_STAGES + 1 ); count2++ )
		{
			if( hDspCh->uiCurrentArrayOfConfBufPtrCopy[count1][count2]==0xFFFFFFFF )
			{
				bufPhysAdr = 0xFFFFFFFF;
			}
			else
			{
				BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, 
					(void *)(hDspCh->uiCurrentArrayOfConfBufPtrCopy[count1][count2]), &bufPhysAdr);
			}
			
			BRAP_MemWrite32((BARC_Handle) hDspCh->hHeap, 
				uiNextArray + 4 * (count1 * (BAF_MAX_POSTPROCESS_STAGES + 1 ) + count2), 
				bufPhysAdr); 

			BDBG_MSG(( "hDspCh->uiCurrentArrayOfConfBufPtrCopy[%d][%d] = 0x%08x, bufPhysAdr = 0x%08x",
				count1, count2, hDspCh->uiCurrentArrayOfConfBufPtrCopy[count1][count2], bufPhysAdr ));
		}
	}

	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)uiNextArray, &bufPhysAdr);

	/* Inform decoder to switch to new buffer */
	BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER0_CXT0 + hDspCh->chCfgRegOffset, bufPhysAdr); 

	BDBG_MSG(("ArrayPtr > 0x%x", uiNextArray));
	BDBG_MSG(("ArrayPhysAdr > 0x%x", bufPhysAdr));

}

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
	)
{
	BDBG_ENTER(BRAP_GetAudioDecoderDelay);
	
	/* Assert the function argument(s) */
	BDBG_ASSERT(pAudioDecoderDelay);
	
	*pAudioDecoderDelay= BRAP_AUDIO_DECODER_DELAY;
	
	BDBG_LEAVE(BRAP_GetAudioDecoderDelay);

	return BERR_SUCCESS;
}

/***************************************************************************
Summary: Gets AC3 decoder configuration parameters
 
Description:
	This function current settings of AC3 decoder configuration parameters

Returns:
	BERR_SUCCESS - If successful

See Also: BRAP_DSPCHN_SetAc3ConfigurationParams
**************************************************************************/
BERR_Code
BRAP_DSPCHN_GetAc3ConfigurationParams(
	BRAP_ChannelHandle		hRapCh,			/* [in] Audio channel handle */
	BRAP_DSPCHN_Ac3ConfigParams	*psAc3ConfigParams 	/* [out] AC3 decoder configuration parameters */
	)
{
        BERR_Code rc = BERR_SUCCESS;
	BRAP_DSPCHN_Handle hDspCh;

	BDBG_ENTER(BRAP_DSPCHN_GetAc3ConfigurationParams);

	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psAc3ConfigParams);

	/* Get DSP Channel handle */
	hDspCh = hRapCh->sModuleHandles.hDspCh;
	
	/* Get the current AC3 decoder configuration parameters from the handle */
	*psAc3ConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3ConfigParam;

	BDBG_LEAVE(BRAP_DSPCHN_GetAc3ConfigurationParams);
	return rc;
}

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
	)
{
	uint32_t regAddr;
	
	BDBG_ENTER(BRAP_DSPCHN_SetPTSOffset);

	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_AV_OFFSET_CXT0 +
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		BRAP_Write32(hRapCh->hRegister, regAddr, i32PtsOffset);
	}
	else
	{
        	BDBG_ERR(("Can not set PTS Offset when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}
	
	BDBG_LEAVE(BRAP_DSPCHN_SetPTSOffset);

	return BERR_SUCCESS;
}

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
	)
{
	uint32_t regAddr, regVal;

	BDBG_ENTER(BRAP_DSPCHN_EnableTsmLog);
	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);

#ifndef BRAP_SUPPORT_TSM_LOG
	BSTD_UNUSED(hRapCh);
	BSTD_UNUSED(regAddr);
	BSTD_UNUSED(regVal);
	BSTD_UNUSED(bEnable);
	BDBG_ERR(("To support TSM Log, enable it at compile time by defining flag BRAP_SUPPORT_TSM_LOG"));
	return BERR_TRACE(BERR_NOT_SUPPORTED);
#else

	BKNI_EnterCriticalSection();
	hRapCh->sModuleHandles.hDspCh->sSettings.bTsmLogEnable = bEnable;
	BKNI_LeaveCriticalSection();

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		regAddr = BCHP_AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0 +
					hRapCh->sModuleHandles.hDspCh->chCfgRegOffset;

		regVal = BRAP_Read32(hRapCh->hRegister, regAddr);
		/* Initialize HOST_BUF_ID and TSM_BUF_ID fields of TSM_LOG_CTRL register */
		regVal = regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, HOST_BUF_ID));
		regVal = regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, DSP_BUF_ID));
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, ENABLE)))
				| (BCHP_FIELD_DATA(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, ENABLE, bEnable));

		BRAP_Write32(hRapCh->hRegister, regAddr, regVal);
	}
	
	BDBG_LEAVE(BRAP_DSPCHN_EnableTsmLog);
	return BERR_SUCCESS;

#endif	
}


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
	)
{
	BDBG_ENTER(BRAP_DSPCHN_GetTsmDebugInfo);
	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(psTsmInfo);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
	psTsmInfo->i32AvOffset = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_AV_OFFSET_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
	psTsmInfo->ui32PcrOffset = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_PCR_OFFSET_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
	psTsmInfo->ui32Stc = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_STC_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
	psTsmInfo->ui32Pts = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_PTS_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
	psTsmInfo->eTsmStatus = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_TSM_STATUS_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
	psTsmInfo->ui32UpperThreshold = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)    
	psTsmInfo->ui32LowerThreshold = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
#else
	psTsmInfo->ui32LowerThreshold = BRAP_Read32(hRapCh->hRegister,
		BCHP_AUD_DSP_CFG0_TSM_LOWER_THRESHOLD_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset);
#endif
    	psTsmInfo->ui32Mbox0Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX0);
    	psTsmInfo->ui32Mbox1Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX1);
    	psTsmInfo->ui32Mbox2Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX2);
    	psTsmInfo->ui32Mbox3Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX3);
    	psTsmInfo->ui32Mbox4Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX4);
    	psTsmInfo->ui32Mbox5Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX5);
    	psTsmInfo->ui32Mbox6Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX6);
    	psTsmInfo->ui32Mbox7Val = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_CTRL0_MAILBOX7);

    	psTsmInfo->ui32EsrRegVal = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_ESR_SO00_INT_STATUS);
	}
	else
	{
        	BDBG_ERR(("Can not get the debug information when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}

	BDBG_LEAVE(BRAP_DSPCHN_GetTsmDebugInfo);
	return BERR_SUCCESS;
}

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
	)
{
	BDBG_ENTER(BRAP_DSPCHN_GetDecoderLockStatus);
	/* Assert on the arguments */
	BDBG_ASSERT(hRapCh);
	BDBG_ASSERT(pbLock);

	/* Since this is a single variable, we need not enter critical section */
	*pbLock = hRapCh->sModuleHandles.hDspCh->bDecLocked;

	BDBG_LEAVE(BRAP_DSPCHN_GetDecoderLockStatus);
	return BERR_SUCCESS;
}


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
	)
{
	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BRAP_DSPCHN_P_GetCurrentPTS(hRapCh, true, psPtsInfo);
	}
	else
	{
        	BDBG_ERR(("Can not get current PTS info in isr when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}
	return BERR_SUCCESS;
}


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
	)
{
	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BRAP_DSPCHN_P_SetStcValidFlag(hRapCh, false);
	}
	else
	{
        	BDBG_ERR(("Can not set STC Valid flag when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}
	return BERR_SUCCESS;
}

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
	)
{
	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BRAP_DSPCHN_P_SetStcValidFlag(hRapCh, true);

	}
	else
	{
        	BDBG_ERR(("Can not set STC Valid flag in isr when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
        	BDBG_ERR(("Rap Channel Index [%d]"
					"This channel [hRapCh=0x%8x] is currently stopped.", 
					hRapCh->uiChannelNo,
					hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);
	}
	return BERR_SUCCESS;
}

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
	)
{
	BDBG_ENTER(BRAP_DSPCHN_EnableDDBassManagement);
	
	BDBG_ASSERT(hRapCh);

	/* TODO: This implementation is temporary. Need to re-implement it using config
	 * parameter structure.
	 */
	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BRAP_Write32(hRapCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + 
				hRapCh->sModuleHandles.hDspCh->chCfgRegOffset, bEnable);
	}
	else
	{
        	BDBG_ERR(("Can not Enable/Disable DDBass management when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	
	}
	
	BDBG_LEAVE(BRAP_DSPCHN_EnableDDBassManagement);
	
	return BERR_SUCCESS;
}

BERR_Code
BRAP_DSPCHN_SetStartStopPts (
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	int					iStartPts,
	int					iStopPts
	)
{
#if BCHP_7411_VER > BCHP_VER_C0
	int dspToggleBit, HostToggleBit, count = 0;
	uint32_t ui32RegVal, memVal, statusVal = 0;
	unsigned int dramAdr;
	unsigned char ptsBuf[SIZE_OF_START_STOP_PTS_BUF], *pPtsBufPtr;
	BRAP_DSPCHN_Handle hDspCh;
	
	BDBG_ENTER( BRAP_DSPCHN_SetStartStopPts );

	BDBG_ASSERT( hRapCh );
	hDspCh = hRapCh->sModuleHandles.hDspCh;
	pPtsBufPtr = ptsBuf;
	dramAdr = hDspCh->sChMemInfo.uiStartStopPtsBuffer;

	/* Start/Stop PTS values can be programmed only after decode is started.
	 * Return error if decode is not started */
	if (false==hRapCh->bStarted) {
		BDBG_ERR(("Start/Stop PTS values can be programmed only after decode is started"));
		return BERR_TRACE(BERR_NOT_INITIALIZED);
	}

	/* Return error if DSP toggle bit and Host toggle bit are not equal */
	/* Note: ui32RegVal gets modified and programmed back in AUD_DSP_CFG0_PTS_TYPE_CXT0
	 * later in the function. Don't use it for any other purpose */
	ui32RegVal = BRAP_Read32(hRapCh->hRegister, 
			BCHP_AUD_DSP_CFG0_PTS_TYPE_CXT0 + hDspCh->chCfgRegOffset);

	dspToggleBit = BCHP_GET_FIELD_DATA(ui32RegVal, AUD_DSP_CFG0_PTS_TYPE_CXT0, DSP_PTS_COUNT);
	HostToggleBit = BCHP_GET_FIELD_DATA(ui32RegVal, AUD_DSP_CFG0_PTS_TYPE_CXT0, HOST_PTS_COUNT);

	if (dspToggleBit!=HostToggleBit)
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);

	/* Program start-stop PTS pair and number of PTS pairs programmed */
	/* TODO: If two pairs are programmed, then number of PTS pairs should
	 * get programmed to 2. Currently this case is not taken care of */
	hDspCh->sPtsBasedStartStop.ui32NumPtsPairProgrammed = 1;
	if (false==hDspCh->bFirstPtsPairProgrammed) {
		/* First PTS pair needs to be programmed in PTS 0 */
		hDspCh->bFirstPtsPairProgrammed = true;
		hDspCh->sPtsBasedStartStop.ui32StartPts0 = iStartPts;
		hDspCh->sPtsBasedStartStop.ui32StopPts0 = iStopPts;
	}
	else {
		/* Program PTS pair in PTS 1 */
		hDspCh->sPtsBasedStartStop.ui32StartPts1 = iStartPts;
		hDspCh->sPtsBasedStartStop.ui32StopPts1 = iStopPts;
	}

	BRAP_DSPCHN_P_AddWordToFWBuffer(pPtsBufPtr, hDspCh->sPtsBasedStartStop.ui32NumPtsPairProgrammed, 4); pPtsBufPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(pPtsBufPtr, hDspCh->sPtsBasedStartStop.ui32StartPts0, 4); pPtsBufPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(pPtsBufPtr, hDspCh->sPtsBasedStartStop.ui32StopPts0, 4); pPtsBufPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(pPtsBufPtr, hDspCh->sPtsBasedStartStop.ui32StartPts1, 4); pPtsBufPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(pPtsBufPtr, hDspCh->sPtsBasedStartStop.ui32StopPts1, 4); pPtsBufPtr += 4;
	
	for (count = 0; count < SIZE_OF_START_STOP_PTS_BUF; count+=4) {
		memVal = (*(ptsBuf+count+3) << 24)|(*(ptsBuf+count+2) << 16)|(*(ptsBuf+count+1) << 8)|(*(ptsBuf+count));
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, dramAdr+count, memVal );
	}

	/* Set PTS count */
	if (HostToggleBit==0)
		HostToggleBit = 1;
	else if (HostToggleBit==1)
		HostToggleBit = 0;
	else
		return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);

	ui32RegVal = (ui32RegVal & (~BCHP_MASK(AUD_DSP_CFG0_PTS_TYPE_CXT0, HOST_PTS_COUNT)))
				| BCHP_FIELD_DATA(AUD_DSP_CFG0_PTS_TYPE_CXT0, HOST_PTS_COUNT, HostToggleBit);

	BRAP_Write32( hRapCh->hRegister,
			BCHP_AUD_DSP_CFG0_PTS_TYPE_CXT0 + hDspCh->chCfgRegOffset,
			ui32RegVal );

	BDBG_ERR(("ui32RegVal = 0x%08x, HostToggleBit = %d", ui32RegVal, HostToggleBit));
		
	/* Set PTS_SET_EN in BCHP_AUD_DSP_ESR_SI00_INT_SET to inform firmware that new start-stop PTS 
	 * have been programmed */
	ui32RegVal = 0;
	ui32RegVal = BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, PTS_SET_EN, 1);

	BRAP_Write32( hRapCh->hRegister,
			BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspCh->chCfgRegOffset,
			ui32RegVal);
	
	for ( count = 0; count < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; count++)
	{
		statusVal = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hRapCh->sModuleHandles.hDspCh->chEsrSiRegOffset);
		/* Following programming needs to be redone once RDB is available */
		if (!(ui32RegVal & statusVal))
		    break;
		BKNI_Sleep(10);
	}
	if ( count >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
	{
		BDBG_ERR(("Timed Out for start stop PTS command Ack"));
		return BERR_TRACE(BERR_TIMEOUT);
	}
	   
	BDBG_MSG(("Received start stop PTS command Ack"));
	
	BDBG_LEAVE( BRAP_DSPCHN_SetStartStopPts );
	return BERR_SUCCESS;
#else
    BSTD_UNUSED(hRapCh);
    BSTD_UNUSED(iStartPts);
    BSTD_UNUSED(iStopPts);
    return BERR_NOT_SUPPORTED;
#endif
}

/* TODO: Remove following sync defines once RDB is ready */
#define BRAP_DSPCHN_P_MULTI_CH_I2S_SYNC_ENABLE_BIT	20
#define BRAP_DSPCHN_P_MULTI_CH_I2S_SYNC_DISABLE_BIT	21

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
	)
{
	uint32_t regVal = 0, statusVal = 0;
	unsigned int uiCount = 0;
	BDBG_ENTER(BRAP_DSPCHN_EnableMultiChI2sSync);
	
	BDBG_ASSERT(hRapCh);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		/* Following programming needs to be redone once RDB is available */
		if (bEnable)
			regVal = 0x1 << BRAP_DSPCHN_P_MULTI_CH_I2S_SYNC_ENABLE_BIT;
		else
			regVal = 0x1 << BRAP_DSPCHN_P_MULTI_CH_I2S_SYNC_DISABLE_BIT;

		BRAP_Write32(hRapCh->hRegister, BCHP_AUD_DSP_ESR_SI00_INT_SET + hRapCh->sModuleHandles.hDspCh->chEsrSiRegOffset, regVal);

		BDBG_MSG(("Waiting for FW ack for multichannel I2S sync..."));

		for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
		{
			statusVal = BRAP_Read32(hRapCh->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hRapCh->sModuleHandles.hDspCh->chEsrSiRegOffset);
			/* Following programming needs to be redone once RDB is available */
			if (!(regVal & statusVal))
			    break;
			BKNI_Sleep(10);
		}
		if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
		{
			BDBG_ERR(("Timed Out for multichannel I2S sync command Ack"));
			return BERR_TRACE(BERR_TIMEOUT);
		}
		   
		BDBG_MSG(("Received multichannel I2S sync ack"));
	}
	else
	{
        	BDBG_ERR(("Can not Enable multichannek I2S sync when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	

	}
	BDBG_LEAVE(BRAP_DSPCHN_EnableMultiChI2sSync);
	return BERR_SUCCESS;
}
	

void 
BRAP_DSPCHN_P_SetStcValidFlag(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool				bIsrCxt		/* [in] true = ISR context, false = Task context */
	)
{

	uint32_t regVal;

	regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,
			BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);

	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, STC_VALID))) |
			BCHP_FIELD_ENUM(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, STC_VALID, Valid);
	
	BRAP_DSP_Write32((BARC_Handle)hRapCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0
						+ hRapCh->sModuleHandles.hDspCh->chCfgRegOffset, regVal, bIsrCxt);

}

void
BRAP_DSPCHN_P_GetCurrentPTS(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool				bIsrCxt,		/* [in] true = ISR context, false = Task context */
	BRAP_DSPCHN_PtsInfo		*psPtsInfo		/* [out] Current PTS information */ 
	)
{
	uint32_t regVal;
	
	psPtsInfo->ui32RunningPts = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,  
				BCHP_AUD_DSP_CFG0_PTS_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);
	psPtsInfo->i32Pts2StcPhase = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,
				BCHP_AUD_DSP_CFG0_PTS2STC_PHASE_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);
	regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,
				BCHP_AUD_DSP_CFG0_PTS_TYPE_CXT0+ hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);	
	psPtsInfo->ePtsType = BCHP_GET_FIELD_DATA(regVal,
									AUD_DSP_CFG0_PTS_TYPE_CXT0,
									PTS_TYPE);
}

void
BRAP_DSPCHN_P_TsmLog_isr(
	BRAP_DSPCHN_Handle	hDspCh,			/* [in] DSP channel handle */
	BRAP_DSPCHN_TsmLogInfo	*psTsmLogInfo)	/* [out] Information about logged TSM data */
{
	unsigned int regVal, uiBufId;
	
	regVal = BRAP_DSP_Read32((BARC_Handle)hDspCh->hRegister,
				BCHP_AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0 + hDspCh->chCfgRegOffset,true);
	/* Read DSP BufId from TSM log control register */
	uiBufId = BCHP_GET_FIELD_DATA(regVal,
				AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0,
				DSP_BUF_ID);

	/* Program Host BufId in TSM log control register
	 * Program 
	 * HOST_BUF_ID = 1 if DSP_BUF_ID = 0
	 * HOST_BUF_ID = 0 if DSP_BUF_ID = 1
	 * i.e. HOST_BUF_ID = DSP_BUF_ID XOR 0x1 */
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, HOST_BUF_ID)))
			| BCHP_FIELD_DATA(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0,
							HOST_BUF_ID,
							uiBufId ^ 0x1);
	BRAP_DSP_Write32((BARC_Handle)hDspCh->hRegister,
		BCHP_AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0  + hDspCh->chCfgRegOffset, regVal, true);

	/* Initialize structure psTsmLogInfo and return */
	psTsmLogInfo->uiTsmLogBufAdr = hDspCh->sChMemInfo.uiTsmLogBufAdr[uiBufId]; 

	psTsmLogInfo->uiTsmLogBufSize = hDspCh->sChMemInfo.uiTsmLogBufSize[uiBufId];
}


static BERR_Code
BRAP_DSPCHN_P_StartDspCxt (
	BRAP_DSPCHN_Handle	hDspChn,
	const BRAP_DSPCHN_P_AudioParams *psAudioParams
	)
{
	BERR_Code rc = BERR_SUCCESS;
	uint32_t regVal = 0x0, opPortNum, physAddress;
	BRAP_DSPCHN_AudioParams sExtAudParams;
	unsigned int uiCount = 0;
    
	uint32_t uiPpBranch = 0, uiPpStg = 0;
	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;
#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)	
	uint32_t regVal1 = 0x0,uiCount1 = 0;	
	bool bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eMax];
#endif
    
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    uint32_t ui32RegVal1=0;
    unsigned int uiNumDelays=0;
    BRAP_DSPCHN_P_OpPortConfig * pOpPortConfig;    
#endif
#if (BRAP_7401_FAMILY == 1) && (BRAP_DSP_P_7401_NEWMIT == 0)
	bool bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eMax];
#endif
	sExtAudParams = psAudioParams->sExtAudioParams;

	regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hDspChn->chCfgRegOffset);

	/* By default TSM is enabled */
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT)))
			| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_SELECT, 
			hDspChn->sSettings.bTsmEnable);

#if (BRAP_DSPCHN_P_HAS_TSM_ERROR_RECOVERY==1)
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY)))
			| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY, 
			hDspChn->sSettings.bEnableTSMErrRecovery);
#endif

	/* Clear STC_VALID bit */
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, STC_VALID)));

#ifndef BCHP_7411_VER
	/* Set multichannel bit - bit 02 of CONTROL_REGISTER4 */	
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE)))
		| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, ASTM_ENABLE,0x0);
#else	
	/* Set multichannel bit - bit 02 of CONTROL_REGISTER4 */	
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, MULTI_CHANNEL_MODE_SELECT)))
		| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, MULTI_CHANNEL_MODE_SELECT, 	
		psAudioParams->bMultiChanOnI2S);
#endif
	
	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hDspChn->chCfgRegOffset, regVal);

	/* Initialize TSM log control register */
	regVal = 0x0;
	/* Program buffer start address field with start address of first buffer */
	BRAP_P_ConvertAddressToOffset(hDspChn->hHeap, (void *)hDspChn->sChMemInfo.uiTsmLogBufAdr[0], &physAddress);
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, BUF_START, 
									(physAddress >> 6));
	/* Initialize buffer size as per RDB requirement (and not in bytes) */
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, BUF_SIZE,
									BRAP_DSPCHN_P_TSM_LOG_RDB_FIELD_BUF_SIZE);
	/* Enable/Disabled TSm log based on the settings */
	regVal = regVal |(BCHP_FIELD_DATA(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, ENABLE, hDspChn->sSettings.bTsmLogEnable));

	/* Initialize HOST_BUF_ID and TSM_BUF_ID fields of TSM_LOG_CTRL register */
	regVal = regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, HOST_BUF_ID));
	regVal = regVal & (~BCHP_MASK(AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0, DSP_BUF_ID));

	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_LOG_CTRL_CXT0 + hDspChn->chCfgRegOffset, regVal);

#if BCHP_7411_VER > BCHP_VER_C0
	/* Program Start-Stop PTS buffer address */
	BRAP_P_ConvertAddressToOffset(hDspChn->hHeap, (void *)hDspChn->sChMemInfo.uiStartStopPtsBuffer, &physAddress);
	BRAP_Write32(hDspChn->hRegister, 
			BCHP_AUD_DSP_CFG0_CONTROL_REGISTER5_CXT0 + hDspChn->chCfgRegOffset,
			hDspChn->sChMemInfo.uiStartStopPtsBuffer);
#endif	


	/* Reset TSM related registers on channel change - this is FW requirement */
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_PTS_CXT0 + hDspChn->chCfgRegOffset , 0x0);
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_PCR_OFFSET_CXT0 + hDspChn->chCfgRegOffset , 0x0);
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 0x0);
#if (BRAP_DSPCHN_P_HAS_NEW_TSM_SCHEME==1)
    /* set the ST and GA thresholds to default values */  
    regVal = BRAP_Read32(hDspChn->hRegister, 
                            BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 
                            + hDspChn->chCfgRegOffset);

    regVal &= ~(BCHP_MASK (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_ST_THRESHOLD));    
    regVal &= ~(BCHP_MASK (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, TSM_GA_THRESHOLD));    

    /* If application has programmed a value, use it, else use defaults */
	if (hDspChn->sSettings.ui32TsmSTThresh!=0) {
        regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
                    TSM_ST_THRESHOLD, hDspChn->sSettings.ui32TsmSTThresh));    
    }
    else {
        regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
                    TSM_ST_THRESHOLD, BRAP_DSPCHN_P_DEF_ST_THRESHOLD));    
    }  
	if (hDspChn->sSettings.ui32TsmGAThresh!=0) {
        regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
                    TSM_GA_THRESHOLD, hDspChn->sSettings.ui32TsmGAThresh));    
    }
    else {
        regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0, 
                    TSM_GA_THRESHOLD, BRAP_DSPCHN_P_DEF_GA_THRESHOLD));    
    }     
	BRAP_Write32(hDspChn->hRegister, 
                 BCHP_AUD_DSP_CFG0_TSM_ST_GA_THRESHOLD_CXT0 
                 + hDspChn->chCfgRegOffset, 
                 regVal);
#else
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_LOWER_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 0x0);
#endif    
	/* If handle has zero discard threshold value, then program default value for it. If handle has non-zero 
	 * value (i.e. application has programmed it explicitly, then program it from handle */
	if (hDspChn->sSettings.ui32TsmDiscardThresh==0) {
	    	switch (sExtAudParams.eStreamType) {
	        	/* Stream types supported:  */
	        	case BAVC_StreamType_ePS:
	        	case BAVC_StreamType_ePes:
	        	case BAVC_StreamType_eTsMpeg:
	        		regVal = BRAP_DSPCHN_P_DEF_MPEG_DISCARD_THRESHOLD; 
	            	break;
	        	case BAVC_StreamType_eDssEs:
	        	case BAVC_StreamType_eDssPes:
	    			regVal = BRAP_DSPCHN_P_DEF_DSS_DISCARD_THRESHOLD; 
	            	 break;
	        	/* Not supported */
	        	case BAVC_StreamType_eEs:
	        	case BAVC_StreamType_eBes:
	        	case BAVC_StreamType_eCms:
	   			default:
						BDBG_ERR(("BRAP_DSPCHN_P_StartDecode: Stream type %d not supported", 
								  sExtAudParams.eStreamType));
			        	return BERR_TRACE(BERR_NOT_SUPPORTED);
	    	}
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_DISCARD_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 
			regVal);	
	}
	else		
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_DISCARD_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 
			hDspChn->sSettings.ui32TsmDiscardThresh);

	/* Only for WMA_STD audio type, program TSM Upper Threshold provided by user.
	 * For all other algorithms, this value is calculated and set by the firmware
	 */
	if (BRAP_DSPCHN_AudioType_eWmaStd==sExtAudParams.eType || BRAP_DSPCHN_AudioType_eWmaPro==sExtAudParams.eType) {
		/* If handle has zero upper threshold value, then program default value for it. 
		 * If handle has non-zero value (i.e. application has programmed it explicitly, 
		 * then program it from handle */
		if (0==hDspChn->sSettings.ui32TsmUpperThresh) {
		    	switch (sExtAudParams.eStreamType) {
		        	/* Stream types supported:  */
		        	case BAVC_StreamType_ePS:
		        	case BAVC_StreamType_ePes:
		        	case BAVC_StreamType_eTsMpeg:
		        		regVal = BRAP_DSPCHN_P_DEF_MPEG_WMA_UPPER_THRESHOLD; 
		            	break;
		        	case BAVC_StreamType_eDssEs:
		        	case BAVC_StreamType_eDssPes:
		    			regVal = BRAP_DSPCHN_P_DEF_DSS_WMA_UPPER_THRESHOLD; 
		            	 break;
		        	/* Not supported */
		        	case BAVC_StreamType_eEs:
		        	case BAVC_StreamType_eBes:
		        	case BAVC_StreamType_eCms:
		   			default:
							BDBG_ERR(("BRAP_DSPCHN_P_StartDecode: Stream type %d not supported", 
									  sExtAudParams.eStreamType));
				        	return BERR_TRACE(BERR_NOT_SUPPORTED);
		    	}
			BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 
				regVal);	
		}
		else		
			BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 
				hDspChn->sSettings.ui32TsmUpperThresh);
	}

#if BCHP_7411_VER > BCHP_VER_C0
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_TSM_TRANS_THRESHOLD_CXT0 + hDspChn->chCfgRegOffset , 
		hDspChn->sSettings.i32TsmTransitionThreshold);
#endif

	regVal = 0x0;
	/* Program correct timebase. Only Timebase0 and Timebase 1 are supported.
	   Return error on other values */
	if (psAudioParams->eTimebase > BAVC_Timebase_e1) {
		BDBG_ERR(("BRAP_DSPCHN_P_StartDecode: Only Timebase 0 and Timebase 1 are supported"));
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}  
	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0,
							TIMEBASE_SEL,
							psAudioParams->eTimebase);	


	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0,	
							RAVE_CTXT_SEL,
							psAudioParams->uiTransChId);


	/* Program correct STC increment rate */
#if 0    
	if (sExtAudParams.eStreamType==BAVC_StreamType_eTsMpeg)
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0, 
							STC_INCR_RATE,
							Is_45_kHz);
	else if (sExtAudParams.eStreamType==BAVC_StreamType_eDssEs ||
		sExtAudParams.eStreamType==BAVC_StreamType_eDssPes) 
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0, 
							STC_INCR_RATE,
							Is_27_MHz);
#else
    switch (sExtAudParams.eStreamType) {
        /* Stream types supported:  */
        case BAVC_StreamType_ePS:
        case BAVC_StreamType_ePes:
        case BAVC_StreamType_eTsMpeg:
        	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0, 
							STC_INCR_RATE,
							Is_45_kHz);      
            break;
        case BAVC_StreamType_eDssEs:
        case BAVC_StreamType_eDssPes:
    		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0, 
							STC_INCR_RATE,
							Is_27_MHz);            
             break;
        /* Not supported */
        case BAVC_StreamType_eEs:
        case BAVC_StreamType_eBes:
        case BAVC_StreamType_eCms:
   		default:
				BDBG_ERR(("BRAP_DSPCHN_P_StartDecode: Stream type %d not supported", 
						  sExtAudParams.eStreamType));
		        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    
#endif
	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0,
							AUD_INPUT_TYPE,
							RAVE_Module);
	/* TODO: Program AUD_MODULE_ID and AUD_INPUT_TYPE appropriately */
	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0 + hDspChn->chCfgRegOffset, regVal);

	/* Program AV_Offset */
	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_AV_OFFSET_CXT0 + hDspChn->chCfgRegOffset,
			sExtAudParams.i32AVOffset);
	
#if !(defined BCHP_7411_VER) && ( BCHP_CHIP != 7400 ) && ( BCHP_CHIP == 7401 && BCHP_VER == BCHP_VER_A0 )
	regVal = 0x0;
	if (psAudioParams->sDecOrPtParams.rBufIndex[0]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[0]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH0_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[1]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[1]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH1_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[2]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[2]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH2_EN, Enabled);
	}
	BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 = 0x%08x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 + hDspChn->chCfgRegOffset, regVal);

	regVal = 0x0;
	if (psAudioParams->sDecOrPtParams.rBufIndex[3]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[3]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH3_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[4]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[4]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH4_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[5]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[5]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH5_EN, Enabled);
	}
	BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 = 0x%08x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 + hDspChn->chCfgRegOffset, regVal);

	if (sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
		regVal = 0x0;
		if (psAudioParams->sSimulPtParams.rBufIndex[0]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH0_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[0]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH0_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[1]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH1_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[1]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH1_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[2]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH2_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[2]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH2_EN, Enabled);
		}
		BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2 = 0x%08x", regVal));
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, regVal);

		regVal = 0x0;
		if (psAudioParams->sSimulPtParams.rBufIndex[3]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH3_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[3]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH3_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[4]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH4_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[4]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH4_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[5]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH5_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[5]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH5_EN, Enabled);
		}
		BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2 = 0x%08x", regVal));
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, regVal);
	}
#else
    /* For 7401 B0, C0; 7400 etc */
	regVal = 0x0;
	if (psAudioParams->sDecOrPtParams.rBufIndex[0]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[0]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_L_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[1]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[1]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_R_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[2]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[2]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LS_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[3]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[3]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_RS_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[4]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[4]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_C_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[5]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[5]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, CH_LFE_EN, Enabled);
	}

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)   
    if (psAudioParams->sDecOrPtParams.bLargeRbuf ==true) {
        /* if the RBUFs are large for delays, inform the FW */
    	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0, LARGE_BUFFER, 1);
    }
#endif    

	BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 = 0x%08x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 + hDspChn->chCfgRegOffset, regVal);
	regVal = 0x0;
#if (( BCHP_CHIP == 7400) || (BRAP_AD_SUPPORTED == 1) ||(BRAP_DOLBYVOLUME_SUPPORTED == 1) ||(BRAP_SRS_TRUVOL_SUPPORTED == 1))
#if (BRAP_7401_FAMILY == 0)
	if (psAudioParams->sDecOrPtParams.rBufIndex[6]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH_LD_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[6]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH_LD_EN, Enabled);
	}
	if (psAudioParams->sDecOrPtParams.rBufIndex[7]!=BRAP_RM_P_INVALID_INDEX) {
		regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH_RD_BUF_ID, psAudioParams->sDecOrPtParams.rBufIndex[7]);
		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0, CH_RD_EN, Enabled);
	}
#endif	
#endif
	BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 = 0x%08x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 + hDspChn->chCfgRegOffset, regVal);

	if (sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
		regVal = 0x0;
		if (psAudioParams->sSimulPtParams.rBufIndex[0]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_L_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[0]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_L_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[1]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_R_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[1]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_R_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[2]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_LS_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[2]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_LS_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[3]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_RS_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[3]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_RS_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[4]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_C_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[4]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_C_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[5]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_LFE_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[5]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, CH_LFE_EN, Enabled);
		}
#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)        
    if (psAudioParams->sSimulPtParams.bLargeRbuf ==true) {
        /* if the RBUFs are large for delays, inform the FW */
    	regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, LARGE_BUFFER, 1);
    }
#endif

		BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 = 0x%08x", regVal));
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, regVal);

		regVal = 0x0;
#if ( BCHP_CHIP == 7400 )
		if (psAudioParams->sSimulPtParams.rBufIndex[6]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH_LD_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[6]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH_LD_EN, Enabled);
		}
		if (psAudioParams->sSimulPtParams.rBufIndex[7]!=BRAP_RM_P_INVALID_INDEX) {
			regVal = regVal | BCHP_FIELD_DATA(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH_RD_BUF_ID, psAudioParams->sSimulPtParams.rBufIndex[7]);
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, CH_RD_EN, Enabled);
		}
#endif
		BDBG_MSG(("AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 = 0x%08x", regVal));
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, regVal);
	}
#endif

	regVal = 0x0;
	for (opPortNum = 0; opPortNum < psAudioParams->sDecOrPtParams.sOpPortParams.uiNumOpPorts; opPortNum++) {
		switch (psAudioParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[opPortNum].eOutputPortType) {
			case BRAP_OutputPort_eSpdif:
                if(psAudioParams->sExtAudioParams.bProgramPLL==false)
                {
                	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, SPDIF0, Disabled);
                }
                else
                {
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, SPDIF0, Enabled);
                }				break;
			case BRAP_OutputPort_eDac0: 
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, DAC0, Enabled);
				break;
			case BRAP_OutputPort_eDac1:
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, DAC1, Enabled);
				break;
			case BRAP_OutputPort_eFlex:
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, SPDIF1, Enabled);
				break;
			default:
				break;
		}

		switch (psAudioParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[opPortNum].ePll) {
			case BRAP_OP_Pll_ePll0:
                if(psAudioParams->sExtAudioParams.bProgramPLL==false)
                {                
		    		regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL0, Disabled);
                }
                else
                {
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL0, Enabled);
                }               
				break;
			case BRAP_OP_Pll_ePll1:
                if(psAudioParams->sExtAudioParams.bProgramPLL==false)
                {                
			    	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL1, Disabled);
                }
                else
                {
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL1, Enabled);
                }  				break;
			case BRAP_OP_Pll_ePll2:
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL2, Enabled);
				break;
			default:
				break;
		}
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT0 + hDspChn->chCfgRegOffset, regVal);
	}
	if (sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
		for (opPortNum = 0; opPortNum < psAudioParams->sSimulPtParams.sOpPortParams.uiNumOpPorts; opPortNum++) {
			regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT2);
            
			switch (psAudioParams->sSimulPtParams.sOpPortParams.sOpPortConfig[opPortNum].eOutputPortType) {
				case BRAP_OutputPort_eSpdif:
				if(psAudioParams->sExtAudioParams.bProgramPLL==false)
                {                    
                	regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, SPDIF0, Disabled);
                }
                else
                {
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, SPDIF0, Enabled);
                }
					break;
				case BRAP_OutputPort_eFlex: 
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, SPDIF1, Enabled);
					break;
				case BRAP_OutputPort_eDac0: 
				case BRAP_OutputPort_eDac1:
				case BRAP_OutputPort_eMai: 
				case BRAP_OutputPort_eRfMod:
				default:
                    break;
			}
			BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT2, regVal);
            
		    regVal=BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT0 + hDspChn->chCfgRegOffset);            
			switch (psAudioParams->sSimulPtParams.sOpPortParams.sOpPortConfig[opPortNum].ePll) {
				case BRAP_OP_Pll_ePll0:
                    if(psAudioParams->sExtAudioParams.bProgramPLL==false)
                    {
                        regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, PLL0, Disabled);
                    }
                    else
                    {
						regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, PLL0, Enabled);
                    }
                    break;
				case BRAP_OP_Pll_ePll1:
                    if(psAudioParams->sExtAudioParams.bProgramPLL==false)                   
                    {
                        regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, PLL1, Disabled);
                    }
                    else
                    {
						regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, PLL1, Enabled);
                    }			
					break;
				case BRAP_OP_Pll_ePll2:
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT2, PLL2, Enabled);
					break;
				case BRAP_OP_Pll_ePll0Div2:
				case BRAP_OP_Pll_ePll1Div2:
				case BRAP_OP_Pll_ePll2Div2:
				case BRAP_OP_Pll_eHifiDac0:
				case BRAP_OP_Pll_eHifiDac1:
				default:
                    break;
			}
            BDBG_MSG(("enabling pll %d in FMM_DEST_CXT0", psAudioParams->sSimulPtParams.sOpPortParams.sOpPortConfig[opPortNum].ePll));
    		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT0 + hDspChn->chCfgRegOffset, regVal);
		}
	}

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)   
    /* If there are any RBUFs that require delay ie associated with an 
     output port that is to be delayed, pass on that info to the FW in 
     AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0
     and AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0*/
    regVal = 0;
    ui32RegVal1 = 0;
	for (opPortNum = 0; opPortNum < psAudioParams->sDecOrPtParams.sOpPortParams.uiNumOpPorts; opPortNum++) {
        
        pOpPortConfig = (BRAP_DSPCHN_P_OpPortConfig *)&(psAudioParams->sDecOrPtParams.sOpPortParams.sOpPortConfig[opPortNum]);
  
    if (pOpPortConfig->uiDlydRBufIndex[0] != BRAP_RM_P_INVALID_INDEX) { 
            
            BDBG_MSG(("Output port %d has a delay.", pOpPortConfig->eOutputPortType));    
            BDBG_MSG(("\tuiDlydRBufIndex[0]=%d", pOpPortConfig->uiDlydRBufIndex[0]));
            BDBG_MSG(("\tuiDlydRBufIndex[1]=%d", pOpPortConfig->uiDlydRBufIndex[1]));
            BDBG_MSG(("\teChanPair=%d", pOpPortConfig->eChanPair));            
            BDBG_MSG(("\tiDelay=%d", pOpPortConfig->iDelay));    
            
            switch (uiNumDelays) {
				case 0:
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,  
                                DELAYED_RBUF0, 
                                pOpPortConfig->uiDlydRBufIndex[0]);
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,
                                DELAYED_RBUF1, 
                                pOpPortConfig->uiDlydRBufIndex[1]);    
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                                DELAYED_RBUF01_CH_PAIR, 
                                pOpPortConfig->eChanPair);  
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                                DELAY_VALUE01, 
                                BRAP_P_ABSOLUTE_VALUE(pOpPortConfig->iDelay));  
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                                DELAY_SIGN01, 
                                BRAP_P_IS_NEGATIVE_VALUE(pOpPortConfig->iDelay));  
					break;  
				case 1:
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,  
                                DELAYED_RBUF2, 
                                pOpPortConfig->uiDlydRBufIndex[0]);
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,  
                                DELAYED_RBUF3, 
                                pOpPortConfig->uiDlydRBufIndex[1]);    
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,  
                                DELAYED_RBUF23_CH_PAIR, 
                                pOpPortConfig->eChanPair);  
                    /* switch over to AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0 */
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_VALUE23, 
                                BRAP_P_ABSOLUTE_VALUE(pOpPortConfig->iDelay));  
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_SIGN23, 
                                BRAP_P_IS_NEGATIVE_VALUE(pOpPortConfig->iDelay));  
					break;   
				case 2:
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAYED_RBUF4, 
                                pOpPortConfig->uiDlydRBufIndex[0]);
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAYED_RBUF5, 
                                pOpPortConfig->uiDlydRBufIndex[1]);    
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAYED_RBUF45_CH_PAIR, 
                                pOpPortConfig->eChanPair);  
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_VALUE45, 
                                BRAP_P_ABSOLUTE_VALUE(pOpPortConfig->iDelay));  
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_SIGN45, 
                                BRAP_P_IS_NEGATIVE_VALUE(pOpPortConfig->iDelay));  
					break;                     
                default:
                    BDBG_WRN(("Cant delay more than 3 ports at start time. ",
                        "Ignoring the other delayed ports here. To delay other ",
                        " ports, explictly call BRAP_SetOutputConfig() after  ",
                        "the channel has started."));
                    break;
            }
            uiNumDelays ++;
            /* TODO: handle Simul PT ports with delay. */
        }           
    }
        /*If only 1 or 2 ports have delay, make the DELAY_VALUE 0 and 
        DELAY_SIGN 1 for the unused rbufs */  
        if (uiNumDelays < BRAP_DSPCHN_P_MAX_NUM_DELAY_PORTS) {
            switch (uiNumDelays) {
				case 0:
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                                DELAY_VALUE01, 
                                0);  
        			regVal |= BCHP_FIELD_DATA (
                                AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                                DELAY_SIGN01, 
                                1); 
                    break;
                case 1:
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_VALUE23, 
                                0);  
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_SIGN23, 
                                1);  
                    break;
                case 2:
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_VALUE45, 
                                0);  
        			ui32RegVal1 |= BCHP_FIELD_DATA (
                    			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                                DELAY_SIGN45, 
                                1);  
                    break;
                default:
                    break;                    
            }
        }
        else {
            BDBG_WRN(("Cant delay more than 3 ports at start time. ",
            "Ignoring the other delayed ports here. To delay other ",
            " ports, explictly call BRAP_SetOutputConfig() after  ",
            "the channel has started."));
        }    
    
    /* We now have all the info we need to write to the delay buffers */
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0 + hDspChn->chCfgRegOffset, regVal);
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0 + hDspChn->chCfgRegOffset, ui32RegVal1);
    BDBG_MSG(("BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0 = 0x%x", regVal));
    BDBG_MSG(("BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0 = 0x%x", ui32RegVal1));

    if (sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
        /* There can be only one simulpt port. return an error if there's more 
        than one! */
        regVal = 0;
        ui32RegVal1 = 0;

        pOpPortConfig = (BRAP_DSPCHN_P_OpPortConfig *)&(psAudioParams->sSimulPtParams.sOpPortParams.sOpPortConfig[0]);


        if (pOpPortConfig->iDelay !=0) {
            
            BDBG_MSG(("SimulPt port %d has a delay.", pOpPortConfig->eOutputPortType));    
            BDBG_MSG(("\tuiDlydRBufIndex[0]=%d", pOpPortConfig->uiDlydRBufIndex[0]));
            BDBG_MSG(("\tuiDlydRBufIndex[1]=%d", pOpPortConfig->uiDlydRBufIndex[1]));
            BDBG_MSG(("\teChanPair=%d", pOpPortConfig->eChanPair));            
            BDBG_MSG(("\tiDelay=%d", pOpPortConfig->iDelay));           
            
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,  
                        DELAYED_RBUF0, 
                        pOpPortConfig->uiDlydRBufIndex[0]);
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0,
                        DELAYED_RBUF1, 
                        pOpPortConfig->uiDlydRBufIndex[1]);    
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                        DELAYED_RBUF01_CH_PAIR, 
                        pOpPortConfig->eChanPair);  
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                        DELAY_VALUE01, 
                        BRAP_P_ABSOLUTE_VALUE(pOpPortConfig->iDelay));  
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                        DELAY_SIGN01, 
                        BRAP_P_IS_NEGATIVE_VALUE(pOpPortConfig->iDelay));  

		ui32RegVal1 |= BCHP_FIELD_DATA (
        			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                    DELAY_VALUE23, 
                    0);  
		ui32RegVal1 |= BCHP_FIELD_DATA (
        			AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0, 
                    DELAY_SIGN23, 
                    1); 
        uiNumDelays ++;
        }
        else {
            /* no delay at all... so initialise even fields for 0th set as invalid*/
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                        DELAY_VALUE01, 
                        0);  
			regVal |= BCHP_FIELD_DATA (
                        AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0, 
                        DELAY_SIGN01, 
                        1);             
        }

      
        /* We now have all the info we need to write to the delay buffers for simul mode*/
    	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT2, regVal);
    	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT2, ui32RegVal1);
        BDBG_MSG(("BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT2 = 0x%x", regVal));
        BDBG_MSG(("BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT2 = 0x%x", ui32RegVal1));            
    }   
#endif
    
	if ((sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
		||(sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode))
	{

        /* At start of decode use ping array and ping config buffers for decoder as well as
         * all the enabled post processing stages */
        rc = BRAP_DSPCHN_P_ProgramDecConfigBuf( hDspChn, hDspChn->sChMemInfo.uiAc3ConfBuffer1 );
        if (rc!=BERR_SUCCESS) 
        	return BERR_TRACE(rc);

        /* Update copy of array of pointers in system memory with next dec config buf address */
        hDspChn->uiCurrentArrayOfConfBufPtrCopy[0][BAF_MAX_POSTPROCESS_STAGES] = hDspChn->sChMemInfo.uiAc3ConfBuffer1;
			
		for( uiPpBranch = 0; uiPpBranch < uiMaxPpBranches; uiPpBranch++ )
		{
			for( uiPpStg = 0; uiPpStg < uiMaxPpStgInBranch; uiPpStg++ )
			{
				/* If PP stage has a valid PP algo, then program corresponding user config
				 * buffer.
				 * For AAC, user config params for stage 0 always needs to be programmed.
				 * If application has added this stage then program user params corresponding
				 * to PP algo added by application. If application has not added any stages in this
				 * branch ( implied by hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[0]
				 * = BRAP_DSPCHN_PP_Algo_eMax ), add AAC downmix stage internally in PI
				 * and program AAC downmix user config params */
				if( (BRAP_DSPCHN_PP_Algo_eMax != hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg] )
                    && (BRAP_DSPCHN_PP_Algo_eSrc!= hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg])
                    && (BRAP_DSPCHN_PP_Algo_eDdbm!= hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg])                    
                    && (BRAP_DSPCHN_PP_Algo_eDsola!= hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg] ))
    				{
    				BDBG_MSG(("!!!!!BR > %d, ST > %d, Def > %d, Algo > %d", uiPpBranch, uiPpStg,
                        hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg],
                        hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]));
    					rc = BRAP_DSPCHN_P_ProgramPpConfigBuf( hDspChn, 
    						hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg],
    						hDspChn->sChMemInfo.uiPpPingConfBuffer[uiPpBranch][uiPpStg],
    						hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg],
    						uiPpBranch, uiPpStg );
    					if (rc!=BERR_SUCCESS) 
    						return BERR_TRACE(rc);
    					
    					/* Update copy of array of pointers in system memory with next pp config buf address */
    					hDspChn->uiCurrentArrayOfConfBufPtrCopy[uiPpBranch][uiPpStg] 
    						= hDspChn->sChMemInfo.uiPpPingConfBuffer[uiPpBranch][uiPpStg];
    				}
    			}
    		}
    		BRAP_DSPCHN_P_ProgramArrayOfConfBufPtr( hDspChn, hDspChn->sChMemInfo.uiConfBufferPingArray );
	}

	/* Initialize SPDIF channel status parameters. These are used only when one of the outputport
	   is SPDIF. In such case, audio manager should pass this field with proper initialization */
	BRAP_DSPCHN_P_InitSpdifChanStatusParams(hDspChn, psAudioParams->sSpdifChStatusParams);

#if (BRAP_7401_FAMILY == 1) && (BRAP_DSP_P_7401_NEWMIT == 0)

	rc = BRAP_P_GetSelectedPpAlgos(hDspChn->hDsp->hRap, bSupportedPpAlgo);
	if (rc!=BERR_SUCCESS)
		return BERR_TRACE(rc);

    if (!((sExtAudParams.eType==BRAP_DSPCHN_AudioType_eAacSbr)
		|| ((sExtAudParams.eType==BRAP_DSPCHN_AudioType_eAac)
			&&(sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru)))){
        if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eSrc] == true)
        {			
    		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x532);
        }
        else
        {
    		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x32);
        }
	}
	else {
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x0);
	}
#elif (BRAP_DSP_P_7401_NEWMIT == 1)
    regVal = 0;
    /* Dont program any post processing for PassThru context. PT cant do any Post processing */
    if (sExtAudParams.eDecodeMode!=BRAP_DSPCHN_DecodeMode_ePassThru)
    {
#if (BRAP_AD_SUPPORTED == 1) || (BRAP_DOLBYVOLUME_SUPPORTED == 1) || (BRAP_SRS_TRUVOL_SUPPORTED == 1)
        regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, OUTPUT_PORT_TYPE, I2S)); 
        regVal1 |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1, OUTPUT_PORT_TYPE, I2S)); 

        uiCount = 0;

	rc = BRAP_P_GetSelectedPpAlgos(hDspChn->hDsp->hRap, bSupportedPpAlgo1);

    if (bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eSrc] == true)
    {
        /* For AACHE, if PlayBackRate != 100, dont add SRC, add only DSOLA */
        if (((hDspChn->sDspAudioParams.sExtAudioParams.eType==BRAP_DSPCHN_AudioType_eAacSbr))
            && (hDspChn->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100))
        {/* Do nothing for Src*/
        }
        else
        { 
            #ifndef WMA_CERTIFICATION
            BDBG_MSG(("Adding SRC as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));

            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE0, 
                SRC));  
            regVal1 |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1, 
                POST_PROCESSING_TYPE_STAGE0, 
                SRC));
            uiCount1 ++;
            uiCount ++;                
            #endif
        }
    }
#if (BRAP_AD_SUPPORTED == 1)
    if ((bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl] == true) &&
		(hDspChn->sDspAudioParams.sExtAudioParams.ePBRate == BRAP_DSPCHN_PlayBackRate_e100))
    {

        BDBG_MSG(("Adding Fade Control as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));
        if (uiCount == 0)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE0, 
                AD_FadeCtrl)); 
            uiCount ++;  
        }
        else if (uiCount == 1)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE1, 
                AD_FadeCtrl));  
            uiCount ++;  
        }      
        else
        {
            BDBG_ERR(("7401 has only 3 stages. AD Fade Control cannot be added as stage %d", uiCount));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
    
    if ((bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eAD_PanCtrl] == true)&&
		(hDspChn->sDspAudioParams.sExtAudioParams.ePBRate == BRAP_DSPCHN_PlayBackRate_e100))

    {    
        BDBG_MSG(("Adding Pan control as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));
        if (uiCount1 == 0)
        {
            regVal1 |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1, 
                POST_PROCESSING_TYPE_STAGE0, 
                AD_PanCtrl)); 
            uiCount1 ++;  
        }
        else if (uiCount1 == 1)
        {
            regVal1 |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1, 
                POST_PROCESSING_TYPE_STAGE1, 
                AD_PanCtrl)); 
            uiCount1 ++;  
        }   
        else if (uiCount1 == 2)
        {
            regVal1 |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1, 
                POST_PROCESSING_TYPE_STAGE2, 
                AD_PanCtrl)); 
            uiCount1 ++;  
        }        
        else
        {
            BDBG_ERR(("7401 has only 3 stages. Pan control cannot be added as stage %d", uiCount));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1) 	
	/* SRS Volume post processing */
    if (bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eSRS_TruVol] == true)
    {
		
        BDBG_MSG(("Adding DolbyVolume Control as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));
        if (uiCount == 0)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE0, 
                SRS_TVOL)); 
            uiCount ++;  
        }
        else if (uiCount == 1)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE1, 
                SRS_TVOL));  
            uiCount ++;  
        }      
        else if (uiCount == 2)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE2, 
                SRS_TVOL));  
            uiCount ++;  
        }		
        else
        {
            BDBG_ERR(("7400 has only 3 stages. Dolby_Volume cannot be added as stage %d", uiCount));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1) 	
	/* Dolby Volume post processing must be Enbled for only AC3 and AC3Plus algorithm */
    if (bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eDolbyVolume] == true)
    {
		
        BDBG_MSG(("Adding DolbyVolume Control as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));
        if (uiCount == 0)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE0, 
                Dolby_Volume)); 
            uiCount ++;  
        }
        else if (uiCount == 1)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE1, 
                Dolby_Volume));  
            uiCount ++;  
        }
        else if (uiCount == 2)
        {
            regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                POST_PROCESSING_TYPE_STAGE2, 
                Dolby_Volume));  
            uiCount ++;  
        }		
        else
        {
            BDBG_ERR(("7400 has only 3 stages. Dolby_Volume cannot be added as stage %d", uiCount));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }
#endif

    if (bSupportedPpAlgo1[BRAP_DSPCHN_PP_Algo_eDsola] == true)
    {

        /* If PB rate is 0.8 or 1.2, add DSOLA. It will be stage 1, 2 or 3
        depending on whether AAC downmix / Src are also included. NOTE: DSOLA MUST be the last stage */
		BDBG_MSG(("before ePBRate=%d", hDspChn->sDspAudioParams.sExtAudioParams.ePBRate));
        if (hDspChn->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100)             
        {  

            BDBG_ERR(("Adding DSOLA as PP stage %d", uiCount));
            BDBG_ERR(("ePBRate=%d", hDspChn->sDspAudioParams.sExtAudioParams.ePBRate));
            if (uiCount == 0)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE0, 
                    DSOLA));
                uiCount ++;        
            }
            else if (uiCount == 1)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE1, 
                    DSOLA));
                uiCount ++;
            }
            else if (uiCount == 2)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE2, 
                    DSOLA));    
            }
            else 
            {
                BDBG_ERR(("7401 has only 3 stages. DSOLA cannot be added as stage %d", uiCount));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }    
    }

    BDBG_MSG(("BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 =0x%x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, regVal);


    BDBG_MSG(("BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1 =0x%x", regVal1));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT1 + hDspChn->chCfgRegOffset, regVal1);

#else
        regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, OUTPUT_PORT_TYPE, I2S)); 

        uiCount = 0;
#if (BRAP_7401_FAMILY == 1)|| (BCHP_CHIP==7400)
        /* For AACHE, if PlayBackRate != 100, dont add SRC, add only DSOLA */
        if (((hDspChn->sDspAudioParams.sExtAudioParams.eType==BRAP_DSPCHN_AudioType_eAacSbr))
            && (hDspChn->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100))
        {/* Do nothing for Src*/
        }
        else
        { 
            #ifndef WMA_CERTIFICATION
            BDBG_MSG(("Adding SRC as PP stage %d to AUD_DSP_CFG0_POST_PROCESSING", uiCount));
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE0, 
                    SRC));  
            uiCount ++;
            #endif
        }

#if ((BRAP_7401_FAMILY == 1) || (BCHP_CHIP == 7400))
        /* If PB rate is 0.8 or 1.2, add DSOLA. It will be stage 1, 2 or 3
        depending on whether AAC downmix / Src are also included. NOTE: DSOLA MUST be the last stage */
        if (hDspChn->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100)             
        {  

            BDBG_MSG(("Adding DSOLA as PP stage %d", uiCount));
            BDBG_MSG(("ePBRate=%d", hDspChn->sDspAudioParams.sExtAudioParams.ePBRate));
            if (uiCount == 0)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE0, 
                    DSOLA));
            }
            else if (uiCount == 1)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE1, 
                    DSOLA));
            }
            else if (uiCount == 2)
            {
                regVal |= (BCHP_FIELD_ENUM (AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0, 
                    POST_PROCESSING_TYPE_STAGE2, 
                    DSOLA));    
            }
            else 
            {
                BDBG_ERR(("7401 has only 3 stages. DSOLA cannot be added as stage %d", uiCount));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }  
#endif		
#endif		
    BDBG_MSG(("BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT =0x%x", regVal));
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, regVal);

#endif
    }

    
#if ((BRAP_7401_FAMILY == 1) || (BCHP_CHIP == 7400))
    regVal = 0; 
	switch (hDspChn->sDspAudioParams.sExtAudioParams.ePBRate) {
		case BRAP_DSPCHN_PlayBackRate_e80:
            regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_FSPB_SRC_CONFIG_CXT0, 
                DSOLA_FS_IN, 
                410));
            break;     
		case BRAP_DSPCHN_PlayBackRate_e100:
            regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_FSPB_SRC_CONFIG_CXT0, 
                DSOLA_FS_IN, 
                0));
			break;
		case BRAP_DSPCHN_PlayBackRate_e120:
            regVal |= (BCHP_FIELD_DATA (AUD_DSP_CFG0_FSPB_SRC_CONFIG_CXT0, 
                DSOLA_FS_IN, 
                614));
            break;
        default:
            BDBG_ERR(("%d is invalid value for Pb Rate. (valid values are 80,100,120)", hDspChn->sDspAudioParams.sExtAudioParams.ePBRate));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BRAP_Write32(hDspChn->hRegister, 
        BCHP_AUD_DSP_CFG0_FSPB_SRC_CONFIG_CXT0 + hDspChn->chCfgRegOffset, 
        regVal);
#endif	
#else
	if (psAudioParams->bMultiChanOnI2S) {
		/* Enable dolby digital bass management */
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x62);
	}
	/* TODO: temp enabling downmix, code properly through API */
	/* Don't enable downmix if algo id is AAC-HE. Don't enable downmix for AAC passthru */
    /* this has to be fixed. Downmix should be enabled 
        - only for algorithms which support it
        - only for decode mode */
	else if (!((sExtAudParams.eType==BRAP_DSPCHN_AudioType_eAacSbr)
		|| ((sExtAudParams.eType==BRAP_DSPCHN_AudioType_eAac)
			&&(sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_ePassThru)))){
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x32);
	}
	else {
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_POST_PROCESSING_BRANCH0_CXT0 + hDspChn->chCfgRegOffset, 0x0);
	}
#endif    
		
	/* Clear Heartbeat register */
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_HEARTBEAT_CXT0 + hDspChn->chCfgRegOffset, 0x0);
	
	/* For DTS-HD, MLP-HDDVD, inform firmware which framesync to be used */
	/* TODO: Program properly once RDB is ready */
	regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + hDspChn->chCfgRegOffset);
	if (sExtAudParams.eDvdType==BRAP_DSPCHN_DvdType_eHdDvd) {
		regVal |= 0x8;
	}
	else {
		regVal &= 0xFFFFFFF7;
	}
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + hDspChn->chCfgRegOffset, regVal);
	
	regVal = 0x0;	
	switch (sExtAudParams.eType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, MPEG1_STEREO);
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			if ( sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode )
			{
				if (sExtAudParams.eAacXptFormat==BRAP_DSPCHN_AacXptFormat_eAdts) 
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AAC_HE_ADTS);
				else 
					regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AAC_HE_LOAS);
			}
			else
			{
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AAC_LC);
			}
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			if (sExtAudParams.eAacXptFormat==BRAP_DSPCHN_AacXptFormat_eAdts) 
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AAC_HE_ADTS);
			else 
				regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AAC_HE_LOAS);
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AC3);
			break;
#else            
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AC3_PLUS);
			break;
#endif            
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, AC3_PLUS);
			break;
		case BRAP_DSPCHN_AudioType_eDts:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, DTS_CORE);
			break;
#ifdef RAP_DRA_SUPPORT            
		case BRAP_DSPCHN_AudioType_eDra:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, DRA);
			break;			
#endif			
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, BD_LPCM);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, HD_DVD_LPCM);
			break;            
#if (BCHP_7411_VER != BCHP_VER_C0) /* include WMA for 7411D0, 7401 and 7400 */ 
		case BRAP_DSPCHN_AudioType_eWmaStd:
			regVal |= BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, WMA);
    		break;
#endif      
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
		case BRAP_DSPCHN_AudioType_eWmaPro:
			regVal |= BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, WMAPRO);
    		break;			
#endif            
#if (BCHP_7411_VER > BCHP_VER_C0)  /* for 7411 D0 */
		case BRAP_DSPCHN_AudioType_eDtshd:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, DTS_HD_CORE);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, DD_IN_LOSSLESS);
			break;
		case BRAP_DSPCHN_AudioType_eMlp:
			regVal = regVal | BCHP_FIELD_ENUM( AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, MLP_IN_LOSSLESS );
			break;
#endif/* 7411D0 */            
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			/* TODO: Using BCHP_DSPCHN_P_DVD_LPVM_ALGO_ID  since RDB is not ready.
			 * Once RDB is ready, it needs to be removed. Then program algo id properly.  */
			regVal |= BCHP_DSPCHN_P_DVD_LPVM_ALGO_ID;
			break;
#ifdef RAP_SRSTRUVOL_CERTIFICATION            
		case BRAP_DSPCHN_AudioType_ePCM:
			regVal = regVal | BCHP_FIELD_ENUM( AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, Linear_PCM_user_model );			
			break;
#endif
#ifdef RAP_PCMWAV_SUPPORT            
		case BRAP_DSPCHN_AudioType_ePcmWav:
			regVal = regVal | BCHP_FIELD_ENUM( AUD_DSP_CFG0_ALGO_ID_CXT0, ALGO_ID, PCMWAV );			
			break;
#endif	
		default:
			BDBG_ERR(("BRAP_DSPCHN_P_Start: Unknown audio type (0x%x)", sExtAudParams.eType));
			return BERR_TRACE(BERR_INVALID_PARAMETER); 
	}

#if( BCHP_CHIP == 7400 )
        /* Setting 27th bit for firmware to indicate the HBR mode set*/
        if((sExtAudParams.eDecodeMode== BRAP_DSPCHN_DecodeMode_ePassThru)&&(sExtAudParams.eType==BRAP_DSPCHN_AudioType_eAc3Plus))
        {
           regVal = regVal | 0x08000000;       
        }
        else
        {
            regVal = regVal & 0xF7FFFFFF;
        }
#endif        

	switch (sExtAudParams.eDecodeMode) {
		case BRAP_DSPCHN_DecodeMode_eDecode:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, DECODE_MODE, Decode);
			break;
		case BRAP_DSPCHN_DecodeMode_ePassThru:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, DECODE_MODE, Pass_through);
			break;
		case BRAP_DSPCHN_DecodeMode_eSimulMode:
			regVal = regVal | BCHP_FIELD_ENUM(AUD_DSP_CFG0_ALGO_ID_CXT0, DECODE_MODE, Decode_and_pass_through);
			break;
		default:
			BDBG_ERR(("BRAP_DSPCHN_P_Start: Unknown decode mode (0x%x)", sExtAudParams.eDecodeMode));
			return BERR_TRACE(BERR_INVALID_PARAMETER); 
	}

	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0 + hDspChn->chCfgRegOffset, regVal);


#if BCHP_7411_VER > BCHP_VER_C0
	/* 7411D0 Specific: If it is multichannel on I2S mode, then program SRC */
	if (psAudioParams->bMultiChanOnI2S) {
		regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_SRC_CONFIG_CXT0  + hDspChn->chCfgRegOffset);
		
		switch (sExtAudParams.eOutputSampleRate) {
			case BAVC_AudioSamplingRate_e48k:
				regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY)))
						| BCHP_FIELD_ENUM(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY, Frequency_48_kHz);	
				break;
			case BAVC_AudioSamplingRate_e96k:
				regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY)))
						| BCHP_FIELD_ENUM(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY, Frequency_96_kHz);	
				break;
			case BAVC_AudioSamplingRate_e192k:
				regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY)))
						| BCHP_FIELD_ENUM(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY, Frequency_192_kHz);	
				break;
			default:
				/* TODO: Code properly after RDB is made available with "SRC_DISABLE" value for "OUTPUT_SAMPLING_FREQUENCY"
				 * field. 
				 */
				regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY)))
						| BCHP_FIELD_ENUM(AUD_DSP_CFG0_SRC_CONFIG_CXT0 , OUTPUT_SAMPLING_FREQUENCY, RESERVED);	
		}

		BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_CFG0_SRC_CONFIG_CXT0  + hDspChn->chCfgRegOffset, regVal);
	}
 #endif


	/* program the start-decode bit */
	regVal = 0x0; 
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, AUD_START)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, AUD_START,1);

#if (BRAP_INDEP_OP_DELAY_SUPPORTED == 1)
    /* if there is a delay on any of the ports associated with this channel, 
    program the DELAY_ENABLED bit */
    if (uiNumDelays != 0) {
    	regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, DELAY_ENABLE)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, DELAY_ENABLE,1);
    }

    if (sExtAudParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode) {
	    ui32RegVal1 = 0;         
        pOpPortConfig = (BRAP_DSPCHN_P_OpPortConfig *)&(psAudioParams->sSimulPtParams.sOpPortParams.sOpPortConfig[0]);
        if (pOpPortConfig->iDelay !=0) {
        	ui32RegVal1 = (ui32RegVal1 & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, DELAY_ENABLE)))|
    			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, DELAY_ENABLE,1);
        }  
    	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI20_INT_SET, ui32RegVal1);        
    }        
#endif        


	/* For playback transport channel, set playback enable bit: FW requires playback bit not be set before 
	 * setting start-decode bit */
	if (psAudioParams->bPlayback) {
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, PB_EN)))|
				BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, PB_EN,1);
	}

	/* If PTS specific decode mode, set PTS specific decode bit */
	if (sExtAudParams.bPtsSpecificDecode) {
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, PTS_SET_EN)))|
				BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, PTS_SET_EN,1);
	}


/*BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CTRL0_MAILBOX0, 0xA);
while(BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CTRL0_MAILBOX0) != 1);*/

	/* Change the state to start state */
	hDspChn->eChState = BRAP_DSPCHN_P_ChState_eStart;

	if(hDspChn->hDsp->hRap->bResetHwDone==false)
	{

		BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset, regVal);

		BDBG_MSG(("Waiting for FW ack for start decode..."));
	/*BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CTRL0_MAILBOX0, 0xB);
	while(BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CTRL0_MAILBOX0) != 2);*/
		for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
		{
			regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hDspChn->chEsrSiRegOffset);
			if (BCHP_GET_FIELD_DATA(regVal, AUD_DSP_ESR_SI00_INT_STATUS,AUD_START) == 0 )
			    break;
			BKNI_Sleep(10);
		}

		if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
		{
			BDBG_ERR(("Timed Out for Start Decode command Ack"));
			hDspChn->eChState = BRAP_DSPCHN_P_ChState_eStop;
			return BERR_TRACE(BERR_TIMEOUT);
		}
	}	
	   
	BDBG_MSG(("Received Start Decode ack"));
	return rc;
}

static BERR_Code
BRAP_DSPCHN_P_StopDspCxt(
        BRAP_DSPCHN_Handle      hDspChn
	)
{
	uint32_t	regVal = 0x0;
	unsigned int uiCount = 0;
    
    uint32_t regValue = 0;
    uint32_t uiRegVal1 = 0;

    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG0_CXT0 + hDspChn->chCfgRegOffset, regValue);
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_DELAYED_RBUFS_CFG1_CXT0 + hDspChn->chCfgRegOffset, uiRegVal1);
    
	/* Set Bit 12 of AUD_DSP_ESR_SI00_INT_SET to stop the DSP */
	regVal = 0x0; 
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, AUD_STOP)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, AUD_STOP,1);
	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset, regVal);
	
	BDBG_MSG(("Waiting for FW ack for stop decode..."));
	for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
	{
		regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hDspChn->chEsrSiRegOffset);
		if (BCHP_GET_FIELD_DATA(regVal, AUD_DSP_ESR_SI00_INT_STATUS, AUD_STOP) == 0 )
			break;
		BKNI_Sleep(10);
	}
	if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
	{
		BDBG_ERR(("Timed Out for Stop Decode command Ack"));
		return BERR_TRACE(BERR_TIMEOUT);
	}
	
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT0 + hDspChn->chCfgRegOffset, 0);
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_ALGO_ID_CXT0 + hDspChn->chCfgRegOffset, 0);
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT0 + hDspChn->chCfgRegOffset, 0);
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT0 + hDspChn->chCfgRegOffset, 0);
    BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0 + hDspChn->chCfgRegOffset, 0);
    
    if(hDspChn->sDspAudioParams.sExtAudioParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    {
        BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_DEST_CXT2, 0);
        BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_ALGO_ID_CXT2, 0);
        BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG0_CXT2, 0);
        BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_FMM_BUFF_CFG1_CXT2, 0);
        BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT2, 0);
    }
	
	BDBG_MSG(("Received Stop Decode ack"));
	
    return BERR_SUCCESS;
}


static BERR_Code
BRAP_DSPCHN_P_GetStreamInformation(
						BRAP_DSPCHN_Handle      hDspCh,		 /* [in] DSPCHN handle */
						bool					bIsrCxt,		/* [in] Is it a ISR context */		
						BRAP_DSPCHN_StreamInfo *psStreamInfo /* [out] Stream Information */
						)
{
	BRAP_DSPCHN_AudioType eAudioType;
	BRAP_DSPCHN_P_StreamInfoReg		sStreamInfoReg;


    /* If the decoder has not locked, or we havent got a valid sample rate, 
    return an error. Note that FW writes in the sample rate, along with other 
    Stream info registers in one shot and then triggers the sample rate 
    interrupt. So we should not be reading these regsiters until we've recieved 
    the first sample rate interrupt */
	if(false==hDspCh->bDecLocked)
	{
        BDBG_MSG(("Stream info not available: Decoder lock interrupt has not occured"));
        return BRAP_ERR_STREAMINFO_NOT_AVAILABLE;
	}
    
	BRAP_DSPCHN_P_ReadStreamInfo(hDspCh, &sStreamInfoReg, bIsrCxt);

	eAudioType = hDspCh->sDspAudioParams.sExtAudioParams.eType;

	/* Initialize stream information structure with zeros */
	BKNI_Memset(psStreamInfo, 0, sizeof(BRAP_DSPCHN_StreamInfo));

	switch (eAudioType) {
		case BRAP_DSPCHN_AudioType_eMpeg:
			BRAP_DSPCHN_P_GetMpegInfo(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eAac:
			BRAP_DSPCHN_P_GetAacInfo(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eAacSbr:
			BRAP_DSPCHN_P_GetAacSbrInfo(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eAc3:
			BRAP_DSPCHN_P_GetAc3Info(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Lossless:
			BRAP_DSPCHN_P_GetAc3LosslessInfo(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eAc3Plus:
			BRAP_DSPCHN_P_GetAc3PlusInfo(&sStreamInfoReg, psStreamInfo);
#if (BCHP_CHIP == 7400)
			if(BRAP_DSPCHN_DecodeMode_ePassThru == hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode)
			{
				if(psStreamInfo->eSamplingRate == BAVC_AudioSamplingRate_e44_1k)
				{
					psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e176_4k;
					psStreamInfo->uStreamInfo.sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e176_4k;
				}
				if(psStreamInfo->eSamplingRate == BAVC_AudioSamplingRate_e48k)
				{
					psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
					psStreamInfo->uStreamInfo.sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e192k;
				}
			}
#endif			
			break;
#if (BCHP_7411_VER != BCHP_VER_C0) /* include WMA for 7411D0, 7401 and 7400 */ 
		case BRAP_DSPCHN_AudioType_eWmaStd:
			BRAP_DSPCHN_P_GetWmaStdInfo(&sStreamInfoReg, psStreamInfo);
			break;
#endif    
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
		case BRAP_DSPCHN_AudioType_eWmaPro:
			BRAP_DSPCHN_P_GetWmaProInfo(&sStreamInfoReg, psStreamInfo);
			break;	
#endif
		case BRAP_DSPCHN_AudioType_eLpcmDvd: 
			BRAP_DSPCHN_P_GetLpcmDvdInfo(&sStreamInfoReg, psStreamInfo);
			break;
           
		case BRAP_DSPCHN_AudioType_eDts:
			BRAP_DSPCHN_P_GetDtsInfo(&sStreamInfoReg, psStreamInfo);
			break;
#ifdef BCHP_7411_VER /* Only for the 7411 */             
		case BRAP_DSPCHN_AudioType_eLpcmBd:
			BRAP_DSPCHN_P_GetBdLpcmInfo(&sStreamInfoReg, psStreamInfo);
			break;
		case BRAP_DSPCHN_AudioType_eLpcmHdDvd:
			BRAP_DSPCHN_P_GetLpcmHdDvdInfo(&sStreamInfoReg, psStreamInfo);
			break;      
		case BRAP_DSPCHN_AudioType_eDtshd:
			BRAP_DSPCHN_P_GetDtshdInfo(&sStreamInfoReg, psStreamInfo);
			break;      
		case BRAP_DSPCHN_AudioType_eLpcmDvd:
			BRAP_DSPCHN_P_GetLpcmDvdInfo(&sStreamInfoReg, psStreamInfo);
			break;
#if BCHP_7411_VER > BCHP_VER_C0            
		case BRAP_DSPCHN_AudioType_eMlp:
			BRAP_DSPCHN_P_GetMlpInfo(&sStreamInfoReg, psStreamInfo);
			break;
#endif			
#endif
#ifdef RAP_DRA_SUPPORT
		case BRAP_DSPCHN_AudioType_eDra:
			BRAP_DSPCHN_P_GetDraInfo(&sStreamInfoReg, psStreamInfo);
			break;
#endif
#ifdef RAP_SRSTRUVOL_CERTIFICATION  
		case BRAP_DSPCHN_AudioType_ePCM:
			break;
#endif
#ifdef RAP_PCMWAV_SUPPORT            
		case BRAP_DSPCHN_AudioType_ePcmWav:
			BRAP_DSPCHN_P_GetPcmWavInfo(&sStreamInfoReg, psStreamInfo);
			break;  
#endif
            
		default: 
			BDBG_ERR(("Audio type not supported"));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	return BERR_SUCCESS;
}

static void
BRAP_DSPCHN_P_ReadStreamInfo(
	BRAP_DSPCHN_Handle hDspChn,
	BRAP_DSPCHN_P_StreamInfoReg	*sStreamInfoReg, 
	bool bIsrCxt)
{
	sStreamInfoReg->ui32StreamInfoReg0 = BRAP_DSP_Read32((BARC_Handle)hDspChn->hRegister,
		BCHP_AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0 + hDspChn->chCfgRegOffset, bIsrCxt);	
	sStreamInfoReg->ui32StreamInfoReg1 = BRAP_DSP_Read32((BARC_Handle)hDspChn->hRegister,
		BCHP_AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0 + hDspChn->chCfgRegOffset, bIsrCxt);	
	sStreamInfoReg->ui32StreamInfoReg2 = BRAP_DSP_Read32((BARC_Handle)hDspChn->hRegister,
		BCHP_AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0 + hDspChn->chCfgRegOffset, bIsrCxt);	
	sStreamInfoReg->ui32StreamInfoReg3 = BRAP_DSP_Read32((BARC_Handle)hDspChn->hRegister,
		BCHP_AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0 + hDspChn->chCfgRegOffset, bIsrCxt);	
	sStreamInfoReg->ui32StreamInfoReg4 = BRAP_DSP_Read32((BARC_Handle)hDspChn->hRegister,
		BCHP_AUD_DSP_CFG0_STREAM_INFO_REGISTER4_CXT0 + hDspChn->chCfgRegOffset, bIsrCxt);	
}

static void
BRAP_DSPCHN_P_GetMpegInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_MpegInfo	sMpegInfo;
	uint32_t regVal; 

	BKNI_Memset(&sMpegInfo, 0, sizeof(sMpegInfo));

	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eMpeg;
	
	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sMpegInfo 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg0;
	
	sMpegInfo.bOriginal = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_ORIGINAL_COPY);

	sMpegInfo.bCopyRight = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_COPYRIGHT);
	
	sMpegInfo.bCRCPresent = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_CRC);

	sMpegInfo.eChmod = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_MODE);

	sMpegInfo.eLayer = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_LAYER);

	sMpegInfo.ui32BitRateIndex = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_BITRATE);

	sMpegInfo.ui32BitRate = BRAP_DSPCHN_P_MpegBitRateTable[sMpegInfo.eLayer-1][sMpegInfo.ui32BitRateIndex];

	sMpegInfo.ui32SamplingRateIndex = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_MPEG_SAMPLING_FREQ);

		switch(sMpegInfo.ui32SamplingRateIndex){
		case 0:		/* 44.1 KHz */
			sMpegInfo.eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 1:		/* 48 KHz */
			sMpegInfo.eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 2:		/* 32 KHz */
			sMpegInfo.eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		default:
			BDBG_ERR(("Error: Unknown MPEG sampling rate (0x%x)", sMpegInfo.ui32SamplingRateIndex));
            sMpegInfo.eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}
	psStreamInfo->eSamplingRate = sMpegInfo.eSamplingRate;

	sMpegInfo.eEmphasisMode = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
								MPEG_EMPHASIS);

	/* Return the stored value */
	psStreamInfo->uStreamInfo.sMpegInfo = sMpegInfo;
		
}

static void
BRAP_DSPCHN_P_GetAacInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_AacInfo	sAacInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sAacInfo, 0, sizeof(sAacInfo));
   
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eAac;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sAacInfo 
	 */

	sAacInfo.ui32BitRate = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		AAC_LC_BIT_RATE);

	sAacInfo.eProfile = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		AAC_LC_PROFILE);

	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		AAC_LC_SAMPLING_RATE);
	switch (ui32SampleRateIndex) {
		case 0:	/* 96 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e96k;
			break;
		case 1: /* 88.2 KHz */	
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e88_2k;
			break;
		case 2:	/* 64 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e64k;
			break;
		case 3:	/* 48 KHz */	
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 4:	/* 44.1 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 5:	/* 32 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 6:	/* 24 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e24k;
			break;
		case 7:	/* 22.05 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e22_05k;
			break;
		case 8:	/* 16 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e16k;
			break;
		case 9:	/*12 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e12k;
			break;
		case 10:	/* 11.025 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e11_025k;
			break;
		case 11:	/* 8 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e8k;
			break;
		default:
			BDBG_ERR(("Error: Unknown AAC sampling rate (0x%x)", ui32SampleRateIndex));
            sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
            
	}
	psStreamInfo->eSamplingRate = sAacInfo.eSamplingRate;

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0
	 * and initialize appropriate fields in structure sAacInfo 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg2;

	sAacInfo.bLfeOn =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_LFE_ON);
		
	sAacInfo.eAcmod = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_ACMOD_VALUE);

	sAacInfo.bPseudoSurroundEnable = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_PSEUDO_SURROUND_ENABLE);

	sAacInfo.uiMatrixMixdownIdx = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_MATRIX_MIXDOWN_IDX);

	sAacInfo.bMatrixMixdownPresent = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_MATRIX_MIXDOWN_PRESENT);

	sAacInfo.bDRCPresent = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
		AAC_LC_DRC_PRESENT);

	psStreamInfo->uStreamInfo.sAacInfo = sAacInfo;
}

static void
BRAP_DSPCHN_P_GetAacSbrInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_StreamInfo	sStreamInfo;
	BRAP_DSPCHN_AacSbrInfo	sAacSbrInfo;

	BKNI_Memset(&sAacSbrInfo, 0, sizeof(sAacSbrInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eAacSbr;
	
	BRAP_DSPCHN_P_GetAacInfo(psStreamInfoReg, &sStreamInfo);
	sAacSbrInfo.sAacInfo = sStreamInfo.uStreamInfo.sAacInfo;
	psStreamInfo->eSamplingRate = sStreamInfo.uStreamInfo.sAacInfo.eSamplingRate;

	psStreamInfo->uStreamInfo.sAacSbrInfo = sAacSbrInfo;
}


#if (BCHP_7411_VER != BCHP_VER_C0) /* include WMA for 7411D0, 7401 and 7400 */ 
static void BRAP_DSPCHN_P_GetWmaStdInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo)
{
	BRAP_DSPCHN_WmaStdInfo		sWmaStdInfo;
	unsigned int regVal, version;
	unsigned int uiSamplingRateIndex;

	BKNI_Memset(&sWmaStdInfo, 0x0, sizeof(BRAP_DSPCHN_WmaStdInfo));

	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eWmaStd;

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sAc3Info 
	 */
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	sWmaStdInfo.ui32BitRate = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMA_BITRATE);

	uiSamplingRateIndex = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMA_SAMPLING_FREQUENCY);

	switch (uiSamplingRateIndex) {
		case 0:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 1:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 2:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		default:
			BDBG_ERR(("Error: Unknown WMA_STD sampling rate (0x%x)",uiSamplingRateIndex));
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}

	sWmaStdInfo.eAcmod = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMA_ACMOD);

	version = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMA_VERSION_NUMBER);

	switch (version) {
		case 1:
			sWmaStdInfo.eVersion = BRAP_DSPCHN_WmaStdVersion_eVersion1;
			break;
		case 2:
			sWmaStdInfo.eVersion = BRAP_DSPCHN_WmaStdVersion_eVersion2;
			break;
		default:
			sWmaStdInfo.eVersion = BRAP_DSPCHN_WmaStdVersion_eUnknown;
			break;
	}
	
	psStreamInfo->uStreamInfo.sWmaStdInfo = sWmaStdInfo;
	
}

#endif
#if (BRAP_7401_FAMILY == 1) || ( BCHP_CHIP == 7400 )
static void BRAP_DSPCHN_P_GetWmaProInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo)
{
	BRAP_DSPCHN_WmaProInfo		sWmaProInfo;
	unsigned int regVal;
	unsigned int uiSamplingRateIndex;

	BKNI_Memset(&sWmaProInfo, 0x0, sizeof(BRAP_DSPCHN_WmaProInfo));

	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eWmaPro;

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sAc3Info 
	 */
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	sWmaProInfo.ui32BitRate = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMAPRO_BITRATE);

	sWmaProInfo.bLfeOn = BCHP_GET_FIELD_DATA(regVal,
						AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
						WMAPRO_LFEON);
			
	uiSamplingRateIndex = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMAPRO_SAMPLING_FREQUENCY);

	switch (uiSamplingRateIndex) {
		case 0:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 1:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 2:
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		default:
			BDBG_ERR(("Error: Unknown WMA_PRO sampling rate"));
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}

	sWmaProInfo.eAcmod = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMAPRO_ACMOD);

	sWmaProInfo.eVersion = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			WMAPRO_VERSION_NUMBER);

	psStreamInfo->uStreamInfo.sWmaProInfo = sWmaProInfo;

}
#endif


static void
BRAP_DSPCHN_P_GetDtsInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_DtsInfo	sDtsInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sDtsInfo, 0, sizeof(sDtsInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eDts;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_SAMPLING_FREQ);

	sDtsInfo.eAMode = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_AMODE);

	sDtsInfo.ui32FrameSize = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_FRAME_SIZE);

	sDtsInfo.ui32NoOfPcmBlocks = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_NUM_PCM_BLOCKS);

	sDtsInfo.bCrcFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_CRC_FLAG);

	switch (ui32SampleRateIndex) {
		case 2:	/* 16 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e16k;
			break;
		case 3:	/* 32 KHz */	
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 7:	/* 22.05 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e22_05k;
			break;
		case 8:	/* 44.1 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 12:	/* 24 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e24k;
			break;
		case 13:	/* 48 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
			
#if 0 /* Uncomment when this enum values are available in BAVC */
		case 1:	/* 8 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e8k;
			break;
		case 6:	/* 11.025 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e11_025k;
			break;
		case 11:	/*12 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e12k;
			break;
#endif
		default:
			BDBG_ERR(("Error: Unknown DTS sampling rate (0x%x)", ui32SampleRateIndex));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0
	 * and initialize appropriate fields in structure sDtsInfo 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg1;

	sDtsInfo.eSourcePcm = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_SOURCE_PCM);

	sDtsInfo.eCopyHistory = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_COPY_HISTORY);

	sDtsInfo.ui32VersionNumber = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_VERSION_NUMBER);

	sDtsInfo.eLfeFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_LFE_PRESENT_FLAG);

	sDtsInfo.bExtAudio = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EXT_AUDIO);

	sDtsInfo.eExtAudioId = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EXT_AUDIO_ID);

	sDtsInfo.bHdcdFormat = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_HDCD_FORMAT);

	sDtsInfo.bEmbeddedDrc = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EMB_DRC);

	sDtsInfo.bEmbeddedDownMixEn = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EMB_DWNMX_EN);

	sDtsInfo.ui32BitRateIndex = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_BIT_RATE);

	sDtsInfo.ui32BitRate = BRAP_DSPCHN_P_DtsBitRateTable[sDtsInfo.ui32BitRateIndex];
	
	psStreamInfo->uStreamInfo.sDtsInfo = sDtsInfo;
}

#ifdef BCHP_7411_VER /* Only for the 7411 */

static void
BRAP_DSPCHN_P_GetDtshdInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_DtshdInfo	sDtshdInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sDtshdInfo, 0, sizeof(sDtshdInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eDtshd;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

    /* TODO: Check if these stream info regs for DTS valid for DTS-HD */
	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_SAMPLING_FREQ);

	sDtshdInfo.eAMode = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_AMODE);

	sDtshdInfo.ui32FrameSize = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_FRAME_SIZE);

	sDtshdInfo.ui32NoOfPcmBlocks = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_NUM_PCM_BLOCKS);

	sDtshdInfo.bCrcFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		DTS_CORE_CRC_FLAG);

	switch (ui32SampleRateIndex) {
		case 2:	/* 16 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e16k;
			break;
		case 3:	/* 32 KHz */	
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 7:	/* 22.05 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e22_05k;
			break;
		case 8:	/* 44.1 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 12:	/* 24 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e24k;
			break;
		case 13:	/* 48 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
			
#if 0 /* Uncomment when this enum values are available in BAVC */
		case 1:	/* 8 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e8k;
			break;
		case 6:	/* 11.025 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e11_025k;
			break;
		case 11:	/*12 KHz */
			sAacInfo.eSamplingRate = BAVC_AudioSamplingRate_e12k;
			break;
#endif
		default:
			BDBG_ERR(("Error: Unknown DTS HD sampling rate (0x%x)", ui32SampleRateIndex));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0
	 * and initialize appropriate fields in structure sDtshdInfo 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg1;

	sDtshdInfo.eSourcePcm = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_SOURCE_PCM);

	sDtshdInfo.eCopyHistory = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_COPY_HISTORY);

	sDtshdInfo.ui32VersionNumber = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_VERSION_NUMBER);

	sDtshdInfo.eLfeFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_LFE_PRESENT_FLAG);

	sDtshdInfo.bExtAudio = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EXT_AUDIO);

	sDtshdInfo.eExtAudioId = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EXT_AUDIO_ID);

	sDtshdInfo.bHdcdFormat = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_HDCD_FORMAT);

	sDtshdInfo.bEmbeddedDrc = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EMB_DRC);

	sDtshdInfo.bEmbeddedDownMixEn = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_EMB_DWNMX_EN);

	sDtshdInfo.ui32BitRateIndex = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
		DTS_CORE_BIT_RATE);

	sDtshdInfo.ui32BitRate = BRAP_DSPCHN_P_DtsBitRateTable[sDtshdInfo.ui32BitRateIndex];
	
	psStreamInfo->uStreamInfo.sDtshdInfo = sDtshdInfo;
}


static void
BRAP_DSPCHN_P_GetBdLpcmInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_BdLpcmInfo	sBdLpcmInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sBdLpcmInfo, 0, sizeof(sBdLpcmInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eLpcmBd;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_SAMPLING_FREQ);

	sBdLpcmInfo.eChnAssignment = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_CH_ASSIGNMENT);

	sBdLpcmInfo.ui32FrameSize = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_FRAME_SIZE);

	sBdLpcmInfo.eBitsPerSample = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_BITS_PER_SAMP);

	sBdLpcmInfo.bStartFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_START_FLAG);

	switch (ui32SampleRateIndex) {
		case 1:	/* 48 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 4:	/* 96 KHz */	
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e96k;
			break;
		case 5:	/* 192 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
			break;
		default:
			BDBG_ERR(("Error: Unknown BD_LPCM sampling rate (0x%x)", ui32SampleRateIndex));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;            
	}

	psStreamInfo->uStreamInfo.sBdLpcmInfo = sBdLpcmInfo;
}



static void
BRAP_DSPCHN_P_GetLpcmHdDvdInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_LpcmHdDvdInfo	sLpcmHdDvdInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sLpcmHdDvdInfo, 0, sizeof(sLpcmHdDvdInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eLpcmHdDvd;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_SAMPLING_FREQ);

	sLpcmHdDvdInfo.eChnAssignment = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_CH_ASSIGNMENT);

	sLpcmHdDvdInfo.ui32FrameSize = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_FRAME_SIZE);

	sLpcmHdDvdInfo.eBitsPerSample = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_BITS_PER_SAMP);

	sLpcmHdDvdInfo.bStartFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_START_FLAG);

	switch (ui32SampleRateIndex) {
		case 1:	/* 48 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 4:	/* 96 KHz */	
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e96k;
			break;
		case 5:	/* 192 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
			break;
		default:
			BDBG_ERR(("Error: Unknown LpcmHdDvd sampling rate (0x%x)", ui32SampleRateIndex));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;            
	}

	psStreamInfo->uStreamInfo.sLpcmHdDvdInfo = sLpcmHdDvdInfo;
}


#if BCHP_7411_VER > BCHP_VER_C0
static void BRAP_DSPCHN_P_GetMlpInfo(
					BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
					BRAP_DSPCHN_StreamInfo *psStreamInfo)
{
BRAP_DSPCHN_MlpInfo	sMlpInfo;
	uint32_t regVal,tempRegField;
	int i;
	


	BKNI_Memset(&sMlpInfo, 0x0, sizeof(BRAP_DSPCHN_MlpInfo));
	
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eMlp;

	/*Extracting Information from register 0 */

	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	sMlpInfo.eMlpStreamMode = BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_MLP_STREAM_MODE );
	
	sMlpInfo.bMajorSyncInfoConstFifoDelay=(BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_MAJOR_SYNC_INFO )>>1 );		
	
	sMlpInfo.bVariableRate=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_VARIABLE_RATE );
	
	sMlpInfo.ui32PeakDataRate=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_PEAK_DATA_RATE );
	
	sMlpInfo.ui32NoOfStreams=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_NUMBER_OF_SUBSTREAMS );
	
	sMlpInfo.ui32SubstreamInfo=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
		MLP_IN_LOSSLESS_SUBSTREAM_INFO );

	if (sMlpInfo.eMlpStreamMode==BRAP_DSPCHN_MlpStreamMode_Fba) 	/* If FBA stream */
	{
		/*Extracting Information from register 1 */
		
		regVal = psStreamInfoReg->ui32StreamInfoReg1;
		i= BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
			MLP_FBA_Stream_AUDIO_SAMPLING_FREQUENCY );
		switch (i)
		{
			case 0:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
				break;
			case 1:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e96k;
				break;
			case 2:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
				break;
			case 8:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
				break;
			case 9:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e88_2k;
				break;
			case 10:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e176_4k;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA stream sampling rate (0x%x)", i));
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
		}

		/* Assigning values for FBA Decode Channel Assignment 8 CH */

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
			MLP_FBA_Stream_DECODER_CHANNEL_ASSIGNMENT_8CH);

		for(i=0;i<13;i++)
		{
			sMlpInfo.uFormatInfo.sFbaFormatInfo.bDecodeChannelAssignment8Ch[i]=(tempRegField >> i) & 0x1;
		}

		/* Assigning values for FBA Decode Channel Assignment 6 CH */

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
			MLP_FBA_Stream_DECODER_CHANNEL_ASSIGNMENT_6CH);
		for(i=0;i<5;i++)
		{
			sMlpInfo.uFormatInfo.sFbaFormatInfo.bDecodeChannelAssignment6Ch[i]=(tempRegField >> i) & 0x1;
		}
		
		/* Assigning values for FBA Decode Channel Modifier 8CH/6CH/2CH */

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
					MLP_FBA_Stream_DECODER_CHANNEL_MODIFIER_8CH);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotIndicated;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eStereo;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotDolbySurroundExEncoded;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLtRt;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eDolbySurroundExEncoded;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLbinRbin;
				break;
			case 3:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eReserved;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier8Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eReserved;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA stream Decode Channel Modifier 8CH (0x%x)", tempRegField));
				break;
		}

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
					MLP_FBA_Stream_DECODER_CHANNEL_MODIFIER_6CH);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotIndicated;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eStereo;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eNotDolbySurroundExEncoded;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLtRt;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eDolbySurroundExEncoded;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLbinRbin;
				break;
			case 3:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6ChSurround=BRAP_DSPCHN_FbaChApplicationLsRsChRecording_eReserved;
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier6Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eReserved;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA stream Decode Channel Modifier 6CH (0x%x)", tempRegField));
				break;
		}
		
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
					MLP_FBA_Stream_DECODER_CHANNEL_MODIFIER_2CH);
		switch (tempRegField) 
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier2Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eStereo;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier2Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLtRt;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier2Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eLbinRbin;
				break;
			case 3:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eDecodeChannelModifier2Ch=BRAP_DSPCHN_FbaChApplication2ChRecording_eReserved;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA stream Decode Channel Modifier 2CH (Presentation contain exactly 2 channels) (0x%x)", tempRegField));
				break;
		}
		
		/* Assigning values for FBA Multi Channel Types 8CH */

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBA_Stream_MULTI_CHANNEL_TYPE_8CH);
		switch (tempRegField) 
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eMultiChannelType8Ch=BRAP_DSPCHN_FbaMultiChannelsType_eStdLoudspeakerlayout;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eMultiChannelType8Ch=BRAP_DSPCHN_FbaMultiChannelsType_eNonStdLoudspeakerlayout;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA Multi Channel Type 8CH (0x%x)", tempRegField));
				break;
		}

		/* Assigning values for FBA Multi Channel Types 6CH */

		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBA_Stream_MULTI_CHANNEL_TYPE_6CH);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eMultiChannelType6Ch=BRAP_DSPCHN_FbaMultiChannelsType_eStdLoudspeakerlayout;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbaFormatInfo.eMultiChannelType6Ch=BRAP_DSPCHN_FbaMultiChannelsType_eNonStdLoudspeakerlayout;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA Multi Channel Type 6CH (0x%x)", tempRegField));
				break;
		}
			
		/*Extracting Information from register 2 */

		regVal = psStreamInfoReg->ui32StreamInfoReg2;


		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.bExtraChannelMeaningPresent=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_EXTRA_CHANNEL_MEANING_PRESENT);

		/*For 2Ch */		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s2ChProperties.bControlEnabled=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_CONTROL_ENABLED_2CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s2ChProperties.ui32DialogueNormalization=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_DIALOGUE_NORMALIZATION_2CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s2ChProperties.ui32MixLevel=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_MIX_LEVEL_2CH);

		/*For 6Ch */		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s6ChProperties.bControlEnabled=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_CONTROL_ENABLED_6CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s6ChProperties.ui32DialogueNormalization=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_DIALOGUE_NORMALIZATION_6CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s6ChProperties.ui32MixLevel=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_MIX_LEVEL_6CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s6ChProperties.ui32SourceFormat=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_SOURCE_FORMAT_6CH);

		/*For 8Ch */		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s8ChProperties.bControlEnabled=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				MLP_FBA_Stream_CONTROL_ENABLED_8CH);
		
		/*Extracting Information from register 3 */

		regVal = psStreamInfoReg->ui32StreamInfoReg3;
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s8ChProperties.ui32DialogueNormalization=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
				MLP_FBA_Stream_DIALOGUE_NORMALIZATION_8CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s8ChProperties.ui32MixLevel=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
				MLP_FBA_Stream_MIX_LEVEL_8CH);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.s8ChProperties.ui32SourceFormat=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
				MLP_FBA_Stream_SOURCE_FORMAT_8CH);
		

		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.ui32StartupGain=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
				MLP_FBA_Stream_STARTUP_GAIN);
		
		sMlpInfo.uChannelMeaning.sFbaChannelMeaning.ui32ExtraChannelMeaningLength=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
				MLP_FBA_Stream_EXTRA_CHANNEL_MEANING_LENGTH);
	}
	else /* For FBA Streams */
	{

		/*Extracting Information from register 1 */		

		regVal = psStreamInfoReg->ui32StreamInfoReg1;

		sMlpInfo.uFormatInfo.sFbbFormatInfo.ui32ChannelAssignment=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_CHANNEL_ASSIGNMENT); /*As for Details Refer to DVD Specification
													 for Read-Only Disc Part4, Annex C */

		sMlpInfo.uFormatInfo.sFbbFormatInfo.ui32MultiChannelType=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_MULTI_CHANNEL_TYPE);/*As for Details Refer to DVD Specification
													for Read-Only Disc Part4, Annex C */


		/*  Audio Sampling Frequency 2 */
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_AUDIO_SAMPLING_FREQUENCY_2);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency2=BAVC_AudioSamplingRate_e48k;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency2=BAVC_AudioSamplingRate_e96k;
				break;
			case 8:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency2=BAVC_AudioSamplingRate_e44_1k;
				break;
			case 9:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency2=BAVC_AudioSamplingRate_e88_2k;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBB stream sampling Frequency 1 (0x%x)", tempRegField));
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency2 = BAVC_AudioSamplingRate_eUnknown;

		}

		/*  Audio Sampling Frequency 1 */
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_AUDIO_SAMPLING_FREQUENCY_1);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e48k;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e96k;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e192k;
				break;
			case 8:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e44_1k;
				break;
			case 9:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e88_2k;
				break;
			case 10:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1=BAVC_AudioSamplingRate_e176_4k;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBB stream sampling Frequency 1 (0x%x)", tempRegField));
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eAudioSamplingFrequency1 = BAVC_AudioSamplingRate_eUnknown;

			}

		/* Quantization Word Length 2*/
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_QUANTIZATION_WORD_LENGTH_2);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength2=BRAP_DSPCHN_MlpQuantizationWordLength_e16Bits;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength2=BRAP_DSPCHN_MlpQuantizationWordLength_e20Bits;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength2=BRAP_DSPCHN_MlpQuantizationWordLength_e24Bits;
				break;
			case 15:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength2=BRAP_DSPCHN_MlpQuantizationWordLength_eNotSpecified;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBB Quantization word length 2 (0x%x)", tempRegField));
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength2=BRAP_DSPCHN_MlpQuantizationWordLength_eReserved;
				break;
		}
		
		/* Quantization Word Length 2*/
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				MLP_FBB_Stream_QUANTIZATION_WORD_LENGTH_1);
		switch (tempRegField)
		{
			case 0:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength1=BRAP_DSPCHN_MlpQuantizationWordLength_e16Bits;
				break;
			case 1:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength1=BRAP_DSPCHN_MlpQuantizationWordLength_e20Bits;
				break;
			case 2:
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength1=BRAP_DSPCHN_MlpQuantizationWordLength_e24Bits;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBB Quantization word length 1 (0x%x)", tempRegField));
				sMlpInfo.uFormatInfo.sFbbFormatInfo.eQuantizationWordLength1=BRAP_DSPCHN_MlpQuantizationWordLength_eReserved;
				break;
		}
		
		

		/*Extracting Information from register 2 */		


		regVal = psStreamInfoReg->ui32StreamInfoReg2;

		i= BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_SAMPLING_FFREQUENCY );
		
		switch (i)
		{
			case 9:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
				break;
			case 10:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
				break;
			case 13:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e88_2k;
				break;
			case 14:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e96k;
				break;
			case 17:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e176_4k;
				break;
			case 18:
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
				break;
			default:
				BDBG_ERR(("Error: Unknown MLP FBA stream sampling rate (0x%x)", i));
				psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
		}
		
		
		sMlpInfo.uChannelMeaning.sFbbChannelMeaning.ui32WordWidth= BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_WORD_WIDTH );

		/* Assigning values for FBB Channel Occupancy */
		tempRegField= BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_CHANNEL_OCCUPANCY );
		for(i=0;i<6;i++)
			{
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.bChannelOccupancy[i]= (tempRegField >> i) & 0x1;
			}
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_MULTI_CHANNEL_TYPE );
		switch(tempRegField){
			case 0:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eMultiChannelType=BRAP_DSPCHN_FbbMultiChannelType_eStandardspeakerlayout;
				break;
			case 1:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eMultiChannelType=BRAP_DSPCHN_FbbMultiChannelType_eNonStandardspeakerlayout;
				break;
			case 2:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eMultiChannelType=BRAP_DSPCHN_FbbMultiChannelType_eNotSpeakerFeeds;
				break;
			default:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eMultiChannelType=BRAP_DSPCHN_FbbMultiChannelType_eReserved;
				break;
			}

		/* Assigning values for FBB Speaker Layout */
		
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_SPEAKER_LAYOUT );
		switch(tempRegField){
			case 0:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eSpeakerLayout=BRAP_DSPCHN_FbbSpeakerLayout_eNotStated;
				break;
			case 1:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eSpeakerLayout=BRAP_DSPCHN_FbbSpeakerLayout_e2plus2plus2;
				break;
			default:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eSpeakerLayout=BRAP_DSPCHN_FbbSpeakerLayout_eReserved;
				break;
			}

		/* Assigning values for FBB Copy Protection */
		
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			MLP_FBB_Stream_COPY_PROTECTION );
		switch(tempRegField){
			case 0:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eCopyProtection=BRAP_DSPCHN_FbbCopyProtection_eUnrestricted;
				break;
			case 1:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eCopyProtection=BRAP_DSPCHN_FbbCopyProtection_eCopyOnce;
				break;
			case 2:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eCopyProtection=BRAP_DSPCHN_FbbCopyProtection_eDoNotCopy;
				break;
			default:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eCopyProtection=BRAP_DSPCHN_FbbCopyProtection_eReserved;
				break;
			}

/*Extracting Information from register 3 */

		regVal=psStreamInfoReg->ui32StreamInfoReg3;

		/* Assigning values for FBB Source Format */
		
		tempRegField=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
			MLP_FBB_Stream_SOURCE_FORMAT );
		switch(tempRegField){
			case 0:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eSourceFormat=BRAP_DSPCHN_FbbSourceFormat_eNotStated;
				break;
			default:
				sMlpInfo.uChannelMeaning.sFbbChannelMeaning.eSourceFormat=BRAP_DSPCHN_FbbSourceFormat_eReserved;
				break;
			}
		sMlpInfo.uChannelMeaning.sFbbChannelMeaning.bHdcdProcess=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
			MLP_FBB_Stream_HDCD_PROCESS );
		sMlpInfo.uChannelMeaning.sFbbChannelMeaning.ui32SummaryInfo=BCHP_GET_FIELD_DATA( regVal, AUD_DSP_CFG0_STREAM_INFO_REGISTER3_CXT0,
			MLP_FBB_Stream_SUMMARY_INFO );
	}
	
	psStreamInfo->uStreamInfo.sMlpInfo = sMlpInfo;

}

#endif /* BCHP_7411_VER > BCHP_VER_C0 */
#endif /* 7411 */

 #if defined ( BCHP_7411_VER ) || (BCHP_CHIP == 7403) || ( BCHP_CHIP == 7400 ) ||(BCHP_CHIP == 7118 ) || ( BCHP_CHIP == 7401 ) 
static void
BRAP_DSPCHN_P_GetLpcmDvdInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_LpcmDvdInfo	sLpcmDvdInfo;
	unsigned int regVal, ui32SampleRateIndex;

	BKNI_Memset(&sLpcmDvdInfo, 0, sizeof(sLpcmDvdInfo));
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eLpcmDvd;
	regVal = psStreamInfoReg->ui32StreamInfoReg0;

	ui32SampleRateIndex =  BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_SAMPLING_FREQ);

	sLpcmDvdInfo.eChnAssignment = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_CH_ASSIGNMENT);

	sLpcmDvdInfo.ui32FrameSize = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_FRAME_SIZE);

	sLpcmDvdInfo.eBitsPerSample = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_BITS_PER_SAMP);

	sLpcmDvdInfo.bStartFlag = BCHP_GET_FIELD_DATA(regVal,
		AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
		LPCM_START_FLAG);

	switch (ui32SampleRateIndex) {
		case 1:	/* 48 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 4:	/* 96 KHz */	
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e96k;
			break;
		case 5:	/* 192 KHz */
			psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e192k;
			break;
		default:
			BDBG_ERR(("Error: Unknown LpcmDvd sampling rate (0x%x)", ui32SampleRateIndex));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;            
	}

	psStreamInfo->uStreamInfo.sLpcmDvdInfo = sLpcmDvdInfo;
}
#endif

static void 
BRAP_DSPCHN_P_GetAc3Info(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_Ac3Info sAc3Info;
	unsigned int regVal, sTimeCodeVal;

	BKNI_Memset(&sAc3Info, 0, sizeof(sAc3Info));

	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eAc3;
	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sAc3Info 
	 */
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	sAc3Info.ui32SamplingRateIndex = BCHP_GET_FIELD_DATA(regVal, 
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
			AC3_AC3_SAMPLING_FREQUENCY);
	switch(sAc3Info.ui32SamplingRateIndex){
		case 0:		/* 48 KHz */
			sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 1:		/* 44.1 KHz */
			sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 2:		/* 32 KHz */
			sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		default:
			BDBG_ERR(("Error: Unknown AC3 sampling rate (0x%x)", sAc3Info.ui32SamplingRateIndex));
            sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}

	psStreamInfo->eSamplingRate = sAc3Info.eSamplingRate;

	sAc3Info.ui8BitStreamId = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
				AC3_AC3_BIT_STREAM_IDENTIFICATION);	

#if(BCHP_CHIP == 7400)
	sAc3Info.eBsmod = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_PLUS_BSMOD_VALUE);

	sAc3Info.eAcmod = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_PLUS_ACMOD_VALUE);
#else
	sAc3Info.eBsmod = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_BSMOD_VALUE);

	sAc3Info.eAcmod = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_ACMOD_VALUE);

#endif
	sAc3Info.eDSur = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_DSURMOD_VALUE);

	sAc3Info.bLfeOn = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_LFEON);

	sAc3Info.bCh1LCodeExist = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_LANGCODE_EXISTS);

	sAc3Info.ui8Ch1LCode = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_LANG_CODE_VALUE);
	
	sAc3Info.eRoomType = BCHP_GET_FIELD_DATA(regVal,
                                AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_ROOM_TYPE_VALUE);

	sAc3Info.eRoomType2 = BCHP_GET_FIELD_DATA(regVal,
                              AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_ROOM_TYPE2_VALUE);

	sAc3Info.bCpyrhtPresent = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0, 
				AC3_AC3_COPYRIGHTBIT);
	
	sAc3Info.bOrigBitStream = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
				AC3_AC3_ORIGINAL_BIT_STREAM); 
	
	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0
	 * and initialize appropriate fields in structure sAc3Info 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg1;

	sAc3Info.eCentreMix = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_CMIX_LEVEL);
	
	sAc3Info.eSurMix = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_SURMIX_LEVEL);

	sAc3Info.bCh2LCodeExist = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_LANGCODE2_EXISTS);

	sAc3Info.ui8Ch2LCode = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_LANG_CODE2_VALUE);

	sAc3Info.eTCode = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_TIME_CODE_EXISTS);

	sTimeCodeVal = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER1_CXT0,
				AC3_AC3_TIME_CODE_FIRST_HALF);

	sAc3Info.sFirstHalfTCode.ui32hours = 
		( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_HRS_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_HRS_SHIFT; 
	sAc3Info.sFirstHalfTCode.ui32minutes = 
		( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_MNT_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_MNT_SHIFT;
	/* Seconds field increment is 8 seconds */ 
	sAc3Info.sFirstHalfTCode.ui32seconds = 
		(( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_SEC_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_SEC_SHIFT) * 8; 

	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0
	 * and initialize appropriate fields in structure sAc3Info 
	 */
	regVal = psStreamInfoReg->ui32StreamInfoReg2;
	
	sTimeCodeVal = BCHP_GET_FIELD_DATA(regVal, 
				AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
				AC3_AC3_TIME_CODE_SECOND_HALF);

	sAc3Info.sSecondHalfTCode.ui32hours = 
		( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_HRS_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_HRS_SHIFT; 
	sAc3Info.sSecondHalfTCode.ui32minutes = 
		( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_MNT_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_MNT_SHIFT; 
	/* Seconds field increment is 8 seconds */ 
	sAc3Info.sSecondHalfTCode.ui32seconds = 
		(( sTimeCodeVal & BRAP_DSPCHN_P_AC3_TIMECODE_SEC_MASK ) >> BRAP_DSPCHN_P_AC3_TIMECODE_SEC_SHIFT) * 8; 

	sAc3Info.ui32FrameSizeCode = BCHP_GET_FIELD_DATA(regVal, 
			AUD_DSP_CFG0_STREAM_INFO_REGISTER2_CXT0,
			AC3_AC3_FRMSIZECOD);

	psStreamInfo->uStreamInfo.sAc3Info = sAc3Info;
	
}

#ifdef RAP_PCMWAV_SUPPORT            
static void 
BRAP_DSPCHN_P_GetPcmWavInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	unsigned int regVal,SampleRateIndex=0;
    
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_ePcmWav;
	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sDraInfo 
	 */
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	SampleRateIndex = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			PCMWAV_PCMWAV_SAMPLING_FREQUENCY);
    
    switch(SampleRateIndex)
    {
        case 8:		/* 48 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
            break;
        case 7:		/* 44.1 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
            break;
        case 6:		/* 32 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
            break;
		case 5:		/* 24 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e24k;
            break;
        case 4:		/* 22.05 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e22_05k;
            break;
        case 3:		/* 16 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e16k;
            break;
		case 2:		/*12 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e12k;
            break;
        case 1:		/* 11.025 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e11_025k;
            break;
        case 0:		/* 8 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e8k;
            break;
        default:
            BDBG_ERR(("Error: Unknown PCMWAV sampling rate (0x%x)", psStreamInfo->eSamplingRate));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
    }	
}
#endif

#ifdef RAP_DRA_SUPPORT
static void 
BRAP_DSPCHN_P_GetDraInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	unsigned int regVal,SampleRateIndex=0;
    
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eDra;
	/* Read stream info register AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0
	 * and initialize appropriate fields in structure sDraInfo 
	 */
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	SampleRateIndex = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			DRA_DRA_SAMPLING_FREQUENCY);
    
    switch(SampleRateIndex)
    {
        case 8:		/* 48 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e48k;
            break;
        case 7:		/* 44.1 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
            break;
        case 6:		/* 32 KHz */
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_e32k;
            break;
        default:
            BDBG_ERR(("Error: Unknown AC3 sampling rate (0x%x)", psStreamInfo->eSamplingRate));
            psStreamInfo->eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
    }	
}
#endif

static void 
BRAP_DSPCHN_P_GetAc3PlusInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_StreamInfo sStreamInfo;
	BRAP_DSPCHN_Ac3PlusInfo sAc3PlusInfo;
#if (BCHP_CHIP == 7400)    
	unsigned int regVal;
#endif
    

	BKNI_Memset(&sAc3PlusInfo, 0, sizeof(sAc3PlusInfo));

	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eAc3Plus;
	
	BRAP_DSPCHN_P_GetAc3Info(psStreamInfoReg, &sStreamInfo);
	sAc3PlusInfo.sAc3Info = sStreamInfo.uStreamInfo.sAc3Info;

#if (BCHP_CHIP == 7400)
	regVal =  psStreamInfoReg->ui32StreamInfoReg0;

	sAc3PlusInfo.sAc3Info.ui32SamplingRateIndex = BCHP_GET_FIELD_DATA(regVal,
			AUD_DSP_CFG0_STREAM_INFO_REGISTER0_CXT0,
			AC3_PLUS_SAMPLING_FREQUENCY);
	switch(sAc3PlusInfo.sAc3Info.ui32SamplingRateIndex){
		case 0:		/* 48 KHz */
			sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e48k;
			break;
		case 1:		/* 44.1 KHz */
			sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e44_1k;
			break;
		case 2:		/* 32 KHz */
			sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e32k;
			break;
		case 3:		/* 176.4 KHz */
			sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e176_4k;
			BDBG_WRN(("sample rate is 176.4KHz for DDP+"));
			break;
		case 4:		/* 192 KHz */
			sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_e192k;
			BDBG_WRN(("sample rate is 192KHz for DDP+"));
			break;
		default:
			BDBG_ERR(("Error: Unknown AC3 sampling rate (0x%x)", sAc3PlusInfo.sAc3Info.ui32SamplingRateIndex));
            sAc3PlusInfo.sAc3Info.eSamplingRate = BAVC_AudioSamplingRate_eUnknown;
	}


	psStreamInfo->uStreamInfo.sAc3PlusInfo = sAc3PlusInfo;
	psStreamInfo->eSamplingRate = sAc3PlusInfo.sAc3Info.eSamplingRate;
#else
	psStreamInfo->uStreamInfo.sAc3PlusInfo = sAc3PlusInfo;
	psStreamInfo->eSamplingRate = sStreamInfo.uStreamInfo.sAc3Info.eSamplingRate;

#endif

}

static void 
BRAP_DSPCHN_P_GetAc3LosslessInfo(
	BRAP_DSPCHN_P_StreamInfoReg	*psStreamInfoReg,
	BRAP_DSPCHN_StreamInfo *psStreamInfo
	)
{
	BRAP_DSPCHN_P_GetAc3PlusInfo(psStreamInfoReg, psStreamInfo);
	psStreamInfo->eType = BRAP_DSPCHN_AudioType_eAc3Lossless;
}

static BERR_Code BRAP_DSPCHN_P_AllocMem( BRAP_DSPCHN_Handle hDspCh)
{

#ifdef BRAP_SUPPORT_TSM_LOG
	uint32_t ui32TempAddr;
#endif
	BRAP_Handle hRap;

	hRap = hDspCh->hDsp->hRap;

	
	hDspCh->sChMemInfo.uiSpdifChStatusBuff = (uint32_t)BRAP_P_AllocAligned( hRap,
									SIZE_OF_BAF_SPDIF_INFO,
									2,
									0
#if (BRAP_SECURE_HEAP==1) 
									,false
#endif												
											
												);
	if (hDspCh->sChMemInfo.uiSpdifChStatusBuff==BRAP_P_INVALID_DRAM_ADDRESS)
		goto alloc_spdifchstatbuf_failed;

#if BCHP_7411_VER > BCHP_VER_C0
	hDspCh->sChMemInfo.uiStartStopPtsBuffer = (uint32_t)BRAP_P_AllocAligned( hRap,
									SIZE_OF_START_STOP_PTS_BUF,
									2,
									0
#if (BRAP_SECURE_HEAP==1) 
									,false
#endif												
											
												);
	if (hDspCh->sChMemInfo.uiStartStopPtsBuffer==BRAP_P_INVALID_DRAM_ADDRESS)
		goto alloc_startstopptsbuf_failed;

	BDBG_MSG(("Memory allocation for Start-Stop PTS buffer: Addr = 0x%08x, Size = 0x%08x", 
		hDspCh->sChMemInfo.uiStartStopPtsBuffer,
		SIZE_OF_START_STOP_PTS_BUF));
#endif    

#ifdef BRAP_SUPPORT_TSM_LOG
	ui32TempAddr = (uint32_t)BRAP_P_AllocAligned( hRap,
									2 * BRAP_DSPCHN_P_TSM_LOG_BUF_SIZE,
									6,
									0
#if (BRAP_SECURE_HEAP==1) 
									,false
#endif												
											
												);
	if (ui32TempAddr==BRAP_P_INVALID_DRAM_ADDRESS)
		goto alloc_tsmlogbuf_failed;

	hDspCh->sChMemInfo.uiTsmLogBufAdr[0] = ui32TempAddr;
	hDspCh->sChMemInfo.uiTsmLogBufAdr[1] = ui32TempAddr + BRAP_DSPCHN_P_TSM_LOG_BUF_SIZE;

	hDspCh->sChMemInfo.uiTsmLogBufSize[0] =
	hDspCh->sChMemInfo.uiTsmLogBufSize[1] = BRAP_DSPCHN_P_TSM_LOG_BUF_SIZE;
#endif

	return BERR_SUCCESS;
	/* Memory allocation failed. Free all the allocated memory and return error. */

#ifdef BRAP_SUPPORT_TSM_LOG
alloc_tsmlogbuf_failed:
#if BCHP_7411_VER > BCHP_VER_C0

#if (BRAP_SECURE_HEAP==1)
	BRAP_P_Free(hRap, (void *) hDspCh->sChMemInfo.uiStartStopPtsBuffer,false);
#else
	BRAP_P_Free(hRap, (void *) hDspCh->sChMemInfo.uiStartStopPtsBuffer);
#endif

#endif
#endif

#if BCHP_7411_VER > BCHP_VER_C0
alloc_startstopptsbuf_failed:
#if (BRAP_SECURE_HEAP==1)
	BRAP_P_Free(hRap, (void *) hDspCh->sChMemInfo.uiSpdifChStatusBuff,false);
#else
	BRAP_P_Free(hRap, (void *) hDspCh->sChMemInfo.uiSpdifChStatusBuff);
#endif
#endif

alloc_spdifchstatbuf_failed:
	return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);

}

 static void BRAP_DSPCHN_P_Free( BRAP_DSPCHN_Handle hDspCh)
 {
 	BDBG_ASSERT(hDspCh);

	/* Free memory allocated by BRAP_DSPCHN_P_AllocMem */ 

#ifdef BRAP_SUPPORT_TSM_LOG

#if (BRAP_SECURE_HEAP==1)
BRAP_P_Free(hDspCh->hDsp->hRap, (void *) hDspCh->sChMemInfo.uiTsmLogBufAdr[0],false );
#else
	BRAP_P_Free(hDspCh->hDsp->hRap, (void *) hDspCh->sChMemInfo.uiTsmLogBufAdr[0]);
#endif
#endif

#if (BRAP_SECURE_HEAP==1)
	BRAP_P_Free(hDspCh->hDsp->hRap, (void *) hDspCh->sChMemInfo.uiSpdifChStatusBuff,false);
#else
	BRAP_P_Free(hDspCh->hDsp->hRap, (void *) hDspCh->sChMemInfo.uiSpdifChStatusBuff);
#endif

}

static void BRAP_DSPCHN_P_AddWordToFWBuffer(
                                        unsigned char *bufPtr,
                                        uint32_t data,
                                        uint32_t nBytes)
{
        uint32_t count;

        for (count = 0; count < nBytes; count++)
                *bufPtr++ = (data >> (8 * count)) & 0xFF;
}

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
 #if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
static uint16_t BRAP_DSPCHN_P_FloatToQ115(int16_t floatVar)
{
	int32_t		temp;

	if (floatVar > 100) return (uint16_t) 0x7FFF;
	if (floatVar < -100) return (uint16_t) 0x8000;

	temp = (floatVar * 0x8000)/100;
	if (temp > (int32_t)0x7FFF)
		temp = 0x7FFF;

	return ((uint16_t)(temp & 0xFFFF));
}
#endif

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
static uint32_t BRAP_DSPCHN_P_FloatToQ131(int16_t floatVar)
{
	int32_t		temp;

	if (floatVar >= 100) return (uint32_t) 0x7FFFFFFF;
	if (floatVar <= -100) return (uint32_t) 0x80000000;

	temp = floatVar * (0x80000000/100);

	return temp; 
}

/* Function to convert input floating point value into Q1.15 format
 * Intended floating point value to be converted, should be
 * multiplied by 100 and then passed to this function
 */
static uint32_t BRAP_DSPCHN_P_FloatToQ230(int16_t floatVar)
{
	int32_t		temp;

	/*BDBG_ASSERT(floatVar > 0);*/
	/* TODO: conversion for negative values */
	if (floatVar >= 100) return (uint32_t) 0x40000000;

	temp = floatVar * (0x40000000/100);

	return temp; 
}

#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
static int32_t BRAP_DSPCHN_P_FloatToQ824(int16_t floatVar)
{
	int32_t		temp;

	if (floatVar >= 127) 
		return (int) 0x7fffffff;

	if (floatVar <= -128) 
		return (int) 0x80000000;

	temp = (int)(floatVar * 0x1000000) ;
	return temp; 
}
#endif

#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)					
static uint32_t BRAP_DSPCHN_P_FloatToFwFormat1(int16_t floatVar)
{
	int32_t		temp;

	BDBG_ASSERT(floatVar >= 0);
	/* TODO: conversion for negative values */
	if (floatVar >= 800) return (uint32_t) 0x3ffffff;
	temp = (((floatVar * 0x147AE) + (floatVar*0x3F)/800) + (unsigned int)(((floatVar*0x3F)%800 + 400)/800));
	return temp; 
}

static uint32_t BRAP_DSPCHN_P_FloatToFwFormat2(int16_t floatVar)
{
	int32_t		temp;

	BDBG_ASSERT(floatVar >= 0);
	/* TODO: conversion for negative values */
	if (floatVar >= 100) return (uint32_t) 0x7fffff;

	temp = ((floatVar * 0x7fffff)/100 + (unsigned int)((( floatVar * 0x7fffff) % 100) + 50)/100 );
	return temp; 
}

static uint32_t BRAP_DSPCHN_P_FloatToFwFormat3(int16_t floatVar)
{
	int32_t		temp;

	BDBG_ASSERT(floatVar >= 0);
	/* TODO: conversion for negative values */
	if (floatVar >= 256) return (uint32_t) 0x7fffff;

	temp = floatVar * (0x8000) ;
	return temp; 
}
static uint32_t BRAP_DSPCHN_P_FloatToFwFormat4(int16_t floatVar)
{
    int32_t  temp;

    BDBG_ASSERT(floatVar >= 0);
    /* TODO: conversion for negative values */
    if (floatVar >= 100) return (uint32_t) 0x1FFFFFC;

    temp = ((floatVar * 0x1FFFFFC)/100 + (unsigned int)((( floatVar * 0x1FFFFFC) % 100) + 50)/100 );
    return temp; 
}
#endif

void BRAP_DSPCHN_P_InitSpdifChanStatusParams (
											BRAP_DSPCHN_Handle hDspCh,
											BRAP_OP_SpdifChanStatusParams sSpdifParams)
{
	unsigned char spdifParamsArray[SIZE_OF_BAF_SPDIF_INFO], *spdifParamsPtr;
	uint32_t count, memVal, physAddress;

#ifndef BCHP_7411_VER
	BSTD_UNUSED (memVal);
#endif

	spdifParamsPtr = spdifParamsArray;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t) sSpdifParams.bProfessionalMode, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t) sSpdifParams.bLinear_PCM, 2); spdifParamsPtr+=2;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t)sSpdifParams.bSwCopyRight, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.uiPre_Emphasis, 2); spdifParamsPtr+=2;  
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.uiChannel_Status_Mode, 2); spdifParamsPtr+=2;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.ui16CategoryCode, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.uiSource_Number, 2); spdifParamsPtr+=2;        
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t)sSpdifParams.bSeparateLRChanNum, 2); spdifParamsPtr+=2;            
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.ui16ClockAccuracy, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t)sSpdifParams.bSampleWorldLength, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, (uint16_t)sSpdifParams.uiSample_Word_Length,2); spdifParamsPtr+=2;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.uiOriginal_Sampling_Frequency, 2); spdifParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(spdifParamsPtr, sSpdifParams.uiCGMS_A, 2); 

	/* Write programmed structure into DRAM */
	spdifParamsPtr = spdifParamsArray;
	for (count = 0; count < SIZE_OF_BAF_SPDIF_INFO; count+=4) {
#ifdef BCHP_7411_VER
		memVal = (*(spdifParamsPtr+count) << 24)|(*(spdifParamsPtr+count+1) << 16)|(*(spdifParamsPtr+count+2) << 8)|(*(spdifParamsPtr+count+3));
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, hDspCh->sChMemInfo.uiSpdifChStatusBuff + count, memVal);
#else
		BRAP_MemWrite32((BARC_Handle)hDspCh->hRegister, hDspCh->sChMemInfo.uiSpdifChStatusBuff + count, *(uint32_t *)(spdifParamsPtr + count)); 
#endif
	}
	BRAP_P_DummyMemRead32((BARC_Handle)hDspCh->hRegister, hDspCh->sChMemInfo.uiSpdifChStatusBuff + count);

	/* Program address of SPDIF channel status param buffer into firmware */
	BRAP_P_ConvertAddressToOffset(hDspCh->hHeap, (void *)hDspCh->sChMemInfo.uiSpdifChStatusBuff, &physAddress);
	BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_SPDIF_PARSER_PTR_CXT0 + hDspCh->chCfgRegOffset,
			physAddress);
		
	BDBG_MSG(("chan stat params written at DRAM location 0x%x (physical=0x%x)", hDspCh->sChMemInfo.uiSpdifChStatusBuff, physAddress));

    if (hDspCh->sDspAudioParams.sExtAudioParams.eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    {
	    BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_SPDIF_PARSER_PTR_CXT2,
			physAddress); /* also prgmr to Simul mode context */
    }  
    
}

/*Note: Return value and output interpretation -
	bool		uint32_t
	bCtxFree  - pui32CtxIndx
	-----     ------------------------------------
	true      - num 			=> download at index num
	false     - BAF_INVALID		=> neither free nor matching FW in contexts
	false     - num 			=> Ctx id num is already having FW
*/

static bool BRAP_DSP_P_CheckFwCtxtAvailability(
		BRAP_DSP_Handle hDsp, 
		BRAP_DSPCHN_DecodeMode eDecMode,
		BRAP_DSPCHN_AudioType eAudType,
		uint32_t *pui32CtxIndx ) /* [OUT] */
{
	bool bCtxFree = false;
	uint32_t ui32Count = 0;
#if BRAP_DSP_P_DBG_ENABLED==1
	uint32_t i;
#endif
	
	*pui32CtxIndx = BAF_INVALID;
	switch (eDecMode)
	{
		case BRAP_DSPCHN_DecodeMode_eDecode:
			for ( ui32Count = 0; ui32Count < BRAP_RM_P_MAX_DECODE_SUPPORTED; ui32Count++ )
			{
					if((hDsp->ctxtFwBufUsage[ui32Count].eAlgoType == eAudType)&&(hDsp->ctxtFwBufUsage[ui32Count].eDecodeMode == eDecMode))
					{
						*pui32CtxIndx = ui32Count;
						bCtxFree = false;
						break;
					}
			}
				

			if(ui32Count==BRAP_RM_P_MAX_DECODE_SUPPORTED)
			{
				for ( ui32Count = 0; ui32Count < BRAP_RM_P_MAX_DECODE_SUPPORTED; ui32Count++ )
				{
					if(hDsp->ctxtFwBufUsage[ui32Count].numUser == 0)
					{
						*pui32CtxIndx = ui32Count;
						bCtxFree = true;
						break;
					}
				}
			}
			
			break;

		case BRAP_DSPCHN_DecodeMode_ePassThru:
			
			for ( ui32Count = 0; ui32Count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; ui32Count++ )
			{
 				if((hDsp->ctxtFwBufUsage[ui32Count+BRAP_RM_P_MAX_DECODE_SUPPORTED].eAlgoType == eAudType)&&(hDsp->ctxtFwBufUsage[ui32Count+BRAP_RM_P_MAX_DECODE_SUPPORTED].eDecodeMode == eDecMode))
				{
					*pui32CtxIndx = ui32Count+BRAP_RM_P_MAX_DECODE_SUPPORTED;
					bCtxFree = false;
					break;
				}
			}

			if(ui32Count==BRAP_RM_P_MAX_PASSTHRU_SUPPORTED)
			{
				for ( ui32Count = 0; ui32Count < BRAP_RM_P_MAX_PASSTHRU_SUPPORTED; ui32Count++ )
				{
					if(hDsp->ctxtFwBufUsage[ui32Count+BRAP_RM_P_MAX_DECODE_SUPPORTED].numUser == 0)
					{
						*pui32CtxIndx = ui32Count+BRAP_RM_P_MAX_DECODE_SUPPORTED;
						bCtxFree = true;
						break;
					}
				}
			}
			 
			break;
		case BRAP_DSPCHN_DecodeMode_eSimulMode:
			for ( ui32Count = BRAP_RM_P_MAX_DECODE_SUPPORTED; ui32Count < BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF; ui32Count++ )
			{       
					if((hDsp->ctxtFwBufUsage[ui32Count].eAlgoType == eAudType)&&(hDsp->ctxtFwBufUsage[ui32Count].eDecodeMode == eDecMode))
					{   
						*pui32CtxIndx = ui32Count;
						bCtxFree = false;
						break;
					}
			}
				
			if(ui32Count==BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF)
			{       
				for ( ui32Count = BRAP_RM_P_MAX_DECODE_SUPPORTED; ui32Count < BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF; ui32Count++ )
				{   
					if(hDsp->ctxtFwBufUsage[ui32Count].numUser == 0)
					{   
						*pui32CtxIndx = ui32Count;
						bCtxFree = true;
						break;
					}
				}
			}
			break;
		default:
			BDBG_ASSERT(eDecMode < BRAP_DSPCHN_DecodeMode_eSimulMode);
			break;
	}

#if BRAP_DSP_P_DBG_ENABLED==1
	BDBG_MSG(("==== FW Usage Info ===="));
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF; i++)
	{
		BDBG_MSG(("Ctxt(%d): DecodeMode - %d AlgoType - %d numUser = %d",
			i,hDsp->ctxtFwBufUsage[i].eDecodeMode, hDsp->ctxtFwBufUsage[i].eAlgoType,
			hDsp->ctxtFwBufUsage[i].numUser));   
	}
#endif 

	BDBG_MSG(("==== FW Check Cntxt Avail Info ===="));
	BDBG_MSG(("bCtxFree(%d) CtxIndex(%d) : DecodeType - %d AlgoType - %d",
		bCtxFree, *pui32CtxIndx, eDecMode, eAudType));

	return bCtxFree;
}

/* 
Note: Return value and output interpretation -
	bool		uint32_t
	bCtxFree  - pui32FwCtxIndx
	-----     ------------------------------------
	true      - num 			=> download at index num
	false 	  - BAF_INVALID		=> neither free nor matching FW in contexts
	false     - num 			=> Ctx id num is already having FW
*/		
static bool BRAP_DSP_P_CheckFsCtxtAvailability(
		BRAP_DSP_Handle hDsp, 
		BRAP_DSPCHN_AudioType eAudType,
		uint32_t *pui32FsCtxIndx) /* [out] */
{
	bool bCtxFree = false;
	bool bPresent = false;
	uint32_t i;

	*pui32FsCtxIndx = BAF_INVALID;
	
#if (BCHP_CHIP == 7400)
	/* Check if already present in the buffer */
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF-1; i++)
#else
	/* Check if already present in the buffer */
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF; i++)
#endif
	{
		if(hDsp->ctxtFsBufUsage[i].eAlgoType == eAudType)
		{
			bPresent = true;
			bCtxFree = false;
			*pui32FsCtxIndx = i;				
			break;
		}
	}

	if(bPresent == false)
	{
#if (BCHP_CHIP == 7400)
    	/* Check if already present in the buffer */
    	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF-1; i++)
#else
    	/* Check if already present in the buffer */
    	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF; i++)
#endif	
		{
			if(hDsp->ctxtFsBufUsage[i].numUser == 0)
			{
				bCtxFree = true;
				*pui32FsCtxIndx = i;
				break;
			}
		}
	}
	
#if BRAP_DSP_P_DBG_ENABLED==1
	BDBG_MSG(("==== FS Usage Info ===="));
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF; i++)
	{
		BDBG_MSG(("Ctxt(%d): AlgoType - %d numUser = %d",
			i,hDsp->ctxtFsBufUsage[i].eAlgoType,hDsp->ctxtFsBufUsage[i].numUser));   
	}
#endif /* BRAP_DSP_P_DBG_ENABLED==1 */

	BDBG_MSG(("==== FS Check Avail Info ===="));
	BDBG_MSG(("bPresent(%d) fsCtxIndex(%d): AlgoType - %d",
		bPresent, *pui32FsCtxIndx, eAudType));
	
	return bCtxFree;
}

static BERR_Code
BRAP_DSPCHN_P_DownloadFirmware(
								BRAP_DSPCHN_Handle hDspCh, 
								const BRAP_DSPCHN_P_AudioParams *psAudioParams)
{
	uint32_t ui32FsCtxIndx = BAF_INVALID;
	uint32_t ui32CtxIndx = BAF_INVALID;
	uint32_t ui32TempDecCtxIndx = BAF_INVALID;
	BERR_Code err = BERR_SUCCESS;
	BRAP_DSP_Handle hDsp = hDspCh->hDsp;
	BRAP_DSP_P_ExecID sExecID;
	bool bFree = false;
	BRAP_DSPCHN_DecodeMode eDecMode = psAudioParams->sExtAudioParams.eDecodeMode;
	BRAP_DSPCHN_AudioType eAudType = psAudioParams->sExtAudioParams.eType;
	BRAP_DSPCHN_DvdType eDvdType = psAudioParams->sExtAudioParams.eDvdType;
#if (BRAP_DSP_P_7401_NEWMIT==1)
	unsigned int uiPpBrIndex = 0, uiPpStgIndex = 0;
	bool bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eMax];
#endif

	unsigned int uiMaxPpBranches = BRAP_P_MAX_BRANCH_SUPPORTED;
	unsigned int uiMaxPpStgInBranch = BRAP_P_MAX_PP_SUPPORTED;

#ifndef BCHP_7411_VER
	if(hDsp->hRap->sSettings.bFwAuthEnable==false)
	{
#endif	
		/* Get the firmware ids to be downloaded */
		BKNI_Memset(&sExecID, BAF_INVALID, sizeof(BRAP_DSP_P_ExecID));
		err = BRAP_DSP_P_GetExecIds(eDecMode, eAudType, &sExecID);
		if(err != BERR_SUCCESS)
		{
			BDBG_ERR(("Error: Aborting Firmware download - %d",err));
			return BERR_TRACE(err);
		}

		/* If AACSBR, use FS id stored in uiTransId */	
		if((eAudType == BRAP_DSPCHN_AudioType_eAacSbr &&  psAudioParams->sExtAudioParams.eAacXptFormat == 0) 
			||(eAudType == BRAP_DSPCHN_AudioType_eAac &&  psAudioParams->sExtAudioParams.eAacXptFormat == 0)
			||(eAudType==BRAP_DSPCHN_AudioType_eDtshd && eDvdType==BRAP_DSPCHN_DvdType_eHdDvd)
			||(eAudType==BRAP_DSPCHN_AudioType_eMlp && eDvdType==BRAP_DSPCHN_DvdType_eHdDvd)
		  )
		{
			BDBG_MSG(("AudioType %d: Before fs(%d) trans(%d)", eAudType, sExecID.fsID[0],sExecID.transID));
			sExecID.fsID[0] = sExecID.transID;
			BDBG_MSG(("AudioType %d: After fs(%d) trans(%d)", eAudType, sExecID.fsID[0],sExecID.transID));
		}

#if (BCHP_CHIP == 7400)
        /* For DDP passthrough we need to download different Frame sync Exec*/
        if((eAudType == BRAP_DSPCHN_AudioType_eAc3Plus)&& (eDecMode == BRAP_DSPCHN_DecodeMode_ePassThru)) 
        {

        	if(hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].eAlgoType == eAudType)
        	{
        		bFree= false;
        	}
        		else
        	{
        		bFree = true;
        	}
        	ui32FsCtxIndx = BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1;				            
        }
        else
        {        
            /* Check and download Frame Sync Exec */
            bFree = BRAP_DSP_P_CheckFsCtxtAvailability(hDsp, eAudType, &ui32FsCtxIndx);
        }
#else
        /* Check and download Frame Sync Exec */
        bFree = BRAP_DSP_P_CheckFsCtxtAvailability(hDsp, eAudType, &ui32FsCtxIndx);
#endif
        
		/* If it is a watchdog recovery, we must find FsCxtIndx for current FrameSync.
		 * Otherwise return error
		 */
		if ((bFree == true) && (BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
			return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );
		if((bFree == false) && (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))))
		{
			if(ui32FsCtxIndx == BAF_INVALID)
			{
				/* If not available in the DRAM and all ctxt buffers are in use, return error */
				BDBG_ERR(("All FS context buffers are in use - returning error"));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			else
			{
				/* Update the usage info for this firmware */
				hDsp->ctxtFsBufUsage[ui32FsCtxIndx].numUser++;
			}
		}
		else
		{
			if(ui32FsCtxIndx == BAF_INVALID)
			{
				/* If not available in the DRAM and all ctxt buffers are in use, return error */
				BDBG_ERR(("All FS context buffers are in use - returning error"));
				return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			}
			else
			{
				/* If not available in the DRAM, download the frame-sync firmware */
				BRAP_DSP_P_DownloadExecFW(hDsp, eDecMode, eAudType, BRAP_DSPCHN_PP_Algo_eMax, true, false, ui32FsCtxIndx, sExecID);
						
				/* Set the usage info for frame-sync firmware */
				if (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))) {
					hDsp->ctxtFsBufUsage[ui32FsCtxIndx].eAlgoType = eAudType;
					hDsp->ctxtFsBufUsage[ui32FsCtxIndx].numUser = 1;
				}
			}
		} /* if(bFree) */

		if ( eDecMode == BRAP_DSPCHN_DecodeMode_eSimulMode ||
			eDecMode == BRAP_DSPCHN_DecodeMode_eDecode )
		{
			/* Check and download Decode executables */
			bFree = BRAP_DSP_P_CheckFwCtxtAvailability(hDsp, BRAP_DSPCHN_DecodeMode_eDecode, eAudType, &ui32CtxIndx); 

			if ((bFree == true) && (BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
				return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );

			if((bFree == false) && (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))))
			{
				if(ui32CtxIndx == BAF_INVALID) 
				{
					/* If not available in the DRAM and all ctxt buffers are in use, return error */
					BDBG_ERR(("All FW context buffers are in use - returning error"));
					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
				}
				else
				{
					/* Dec Fw already available, update the usage info for this firmware */
					hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser++;		
				}
			}
			else
			{	
				/* Free buffer available, download the firmware */
				if(ui32CtxIndx != BAF_INVALID)
				{
					/* Download executables required for the decode mode */
					BRAP_DSP_P_DownloadExecFW(hDsp, BRAP_DSPCHN_DecodeMode_eDecode, eAudType, 
								BRAP_DSPCHN_PP_Algo_eMax, false, false, ui32CtxIndx, sExecID);

					/* Set the usage info for this firmware */
					if (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
					{
						hDsp->ctxtFwBufUsage[ui32CtxIndx].eAlgoType = eAudType;
						hDsp->ctxtFwBufUsage[ui32CtxIndx].eDecodeMode = BRAP_DSPCHN_DecodeMode_eDecode;
						hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser = 1;
					}	
				}
			}
                    if(BRAP_DSPCHN_DecodeMode_eSimulMode == eDecMode)
                    {
        			/* Check and download Decode executables */
        			bFree = BRAP_DSP_P_CheckFwCtxtAvailability(hDsp, BRAP_DSPCHN_DecodeMode_eSimulMode, eAudType, &ui32CtxIndx); 

        			if ((bFree == true) && (BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
        				return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );

        			if((bFree == false) && (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))))
        			{
        				if(ui32CtxIndx == BAF_INVALID) 
        				{
        					/* If not available in the DRAM and all ctxt buffers are in use, return error */
        					BDBG_ERR(("All FW context buffers are in use - returning error"));
        					return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        				}
        				else
        				{
        					/* Dec Fw already available, update the usage info for this firmware */
        					hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser++;		
        				}
        			}
        			else
        			{	
        				/* Free buffer available, download the firmware */
        				if(ui32CtxIndx != BAF_INVALID)
        				{
        					/* Download executables required for the decode mode */
        					BRAP_DSP_P_DownloadExecFW(hDsp, BRAP_DSPCHN_DecodeMode_ePassThru, eAudType, 
        								BRAP_DSPCHN_PP_Algo_eMax, false, false, ui32CtxIndx, sExecID);

        					/* Set the usage info for this firmware */
        					if (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
        					{
        						hDsp->ctxtFwBufUsage[ui32CtxIndx].eAlgoType = eAudType;
        						hDsp->ctxtFwBufUsage[ui32CtxIndx].eDecodeMode = BRAP_DSPCHN_DecodeMode_eSimulMode;
        						hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser = 1;
        					}	
        				}
        			}
                    }                
			/* Take a temporary copy of the decode index for futrue use */
			ui32TempDecCtxIndx = ui32CtxIndx;
		}

		if ( eDecMode == BRAP_DSPCHN_DecodeMode_ePassThru )
		{
			/* Check and download Pass through executables */
			bFree = BRAP_DSP_P_CheckFwCtxtAvailability(hDsp, BRAP_DSPCHN_DecodeMode_ePassThru, eAudType, &ui32CtxIndx); 

			if ((bFree == true) && (BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
				return BERR_TRACE( BRAP_ERR_BAD_DEVICE_STATE );

			if((bFree == false) && (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))))
			{
				if(ui32CtxIndx == BAF_INVALID) 
				{
					/* If not available in the DRAM and all ctxt buffers are in use, return error */
					BDBG_ERR(("All FW context buffers are in use - returning error"));
					return BERR_OUT_OF_DEVICE_MEMORY;
				}
				else
				{
					/* Pt Fw already available, update the usage info for this firmware */
					hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser++;		
				}		
			}
			else
			{	
				if(ui32CtxIndx != BAF_INVALID)
				{
					/* Download executables required for the pass thru mode */
					BRAP_DSP_P_DownloadExecFW(hDsp, BRAP_DSPCHN_DecodeMode_ePassThru, eAudType, 
								BRAP_DSPCHN_PP_Algo_eMax, false, false, ui32CtxIndx, sExecID);

					/* Set the usage info for this firmware */				
					if (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp)))
					{
						hDsp->ctxtFwBufUsage[ui32CtxIndx].eAlgoType = eAudType;
						hDsp->ctxtFwBufUsage[ui32CtxIndx].eDecodeMode = eDecMode;
						hDsp->ctxtFwBufUsage[ui32CtxIndx].numUser = 1;
					}	
				}
			}
		}
#ifndef BCHP_7411_VER		
	}
#endif	

	if ((psAudioParams->sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eDecode)
		||(psAudioParams->sExtAudioParams.eDecodeMode==BRAP_DSPCHN_DecodeMode_eSimulMode)) {

		/* If not watchdog recovery, allocate memory for AC3 configuration parameters ping-pong buffers */
		if (!(BRAP_DSP_P_GetWatchdogRecoveryFlag(hDsp))) {
             #ifndef WMA_CERTIFICATION
            /* add SRC  */
            uiPpStgIndex =0;
            BDBG_MSG(("Adding SRC as PP stage %d", uiPpStgIndex));            
            hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eSrc;
            uiPpStgIndex ++;
            #endif

            /* If PB rate is 0.8 or 1.2, add DSOLA. It will be stage 2 or 3
            depending on whether AAC downmix is also included */
            if (hDspCh->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100)             
            {  
                if ((hDspCh->sDspAudioParams.sExtAudioParams.eType==BRAP_DSPCHN_AudioType_eAacSbr))
                { /* For aache, replace src by dsola */  
                    uiPpStgIndex --;
                    BDBG_MSG(("Replacing DSOLA as PP stage %d for AACHE", uiPpStgIndex));            
                    hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eDsola;
                }
                else
                {  
                    BDBG_MSG(("Adding DSOLA as PP stage %d", uiPpStgIndex));            
                    hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eDsola;
                }
                uiPpStgIndex ++;
            }   
#if (BRAP_AD_SUPPORTED == 1)            
            hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eAD_PanCtrl;
            uiPpStgIndex ++;

            hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl;
            uiPpStgIndex ++;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
            hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eSRS_TruVol;
            uiPpStgIndex ++;
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
            hDspCh->sPpBranchInfo[0].sPpStgSettings.ePpAlgo[uiPpStgIndex]= BRAP_DSPCHN_PP_Algo_eDolbyVolume;
            uiPpStgIndex ++;
#endif			

            

			/* Allocate arrays of config buffer pointers */
			err = BRAP_DSP_P_AllocateConfigBufArray(hDspCh->hDsp,
					&hDspCh->uiConfigBufArrayIndex,
					&hDspCh->sChMemInfo.uiConfBufferPingArray);
			if (err!=BERR_SUCCESS)
				return BERR_TRACE(err);
			hDspCh->sChMemInfo.uiConfBufferPongArray = 
				hDspCh->sChMemInfo.uiConfBufferPingArray +
				( hDspCh->hDsp->memInfo.arrayOfConfigBufSize * BRAP_P_MAX_BRANCH_SUPPORTED );


			/* Allocate configuration buffer for decode stage */
			err = BRAP_DSP_P_AllocateDecConfigBuf(hDspCh->hDsp,
					&hDspCh->uiDecConfigBufIndex,
					&hDspCh->sChMemInfo.uiAc3ConfBuffer1);
			if (err!=BERR_SUCCESS)
				return BERR_TRACE(err);
			hDspCh->sChMemInfo.uiAc3ConfBuffer2 = 
				hDspCh->sChMemInfo.uiAc3ConfBuffer1 + hDspCh->hDsp->memInfo.decConfigBufSize;

			/* Allocate configuration buffer for post processing stage */
			for( uiPpBrIndex = 0; uiPpBrIndex < uiMaxPpBranches; uiPpBrIndex++ )
			{
				for( uiPpStgIndex = 0; uiPpStgIndex < uiMaxPpStgInBranch; uiPpStgIndex++ )
				{
					/* If PP stage has a valid PP algo, then allocate user config buffer.*/
					if(BRAP_DSPCHN_PP_Algo_eMax != hDspCh->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex])
					{
						err = BRAP_DSP_P_AllocateDecConfigBuf(hDspCh->hDsp,
								&hDspCh->uiPpConfigBufIndex[uiPpBrIndex][uiPpStgIndex],
								&hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBrIndex][uiPpStgIndex]);
						if (err!=BERR_SUCCESS)
							return BERR_TRACE(err);

						hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBrIndex][uiPpStgIndex]
							= hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBrIndex][uiPpStgIndex] +
							hDspCh->hDsp->memInfo.decConfigBufSize;
					}
					else
					{
						hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
						hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
					}
				}

				for( ; uiPpStgIndex < BAF_MAX_POSTPROCESS_STAGES; uiPpStgIndex++ )
				{
					hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
					hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
				}
			}

			for( ; uiPpBrIndex < BAF_MAX_BRANCHES; uiPpBrIndex++ )
			{
				for( uiPpStgIndex = 0; uiPpStgIndex < BAF_MAX_POSTPROCESS_STAGES; uiPpStgIndex++ )
				{
					hDspCh->sChMemInfo.uiPpPingConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
					hDspCh->sChMemInfo.uiPpPongConfBuffer[uiPpBrIndex][uiPpStgIndex] = BRAP_P_INVALID_DRAM_ADDRESS;
				}
			} 

			err = BRAP_DSP_AllocateInterframeBuf(hDspCh->hDsp,  &hDspCh->uiInterframeBufIndex);
			if (err!=BERR_SUCCESS)
				return BERR_TRACE(err);
            
        	err = BRAP_P_GetSelectedPpAlgos(hDsp->hRap, bSupportedPpAlgo);
        	if (err!=BERR_SUCCESS)
        		return BERR_TRACE(err);
	
            /* We have to check the following:
                - If PB Rate is 0.8 or 1.2, BRAP_DSPCHN_PP_Algo_eDsola MUST be enabled
                - BRAP_DSPCHN_PP_Algo_eSrc MUST always be enabled 

                Stages:
                - If DSOLA is present, it MUST be the last stage
                Note: 7401 has only 1 branch. 
            */

    		if ((hDspCh->sDspAudioParams.sExtAudioParams.ePBRate != BRAP_DSPCHN_PlayBackRate_e100) 
              && (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eDsola] == false))
		    {
                BDBG_ERR(("BRAP_DSPCHN_PP_Algo_eDsola MUST be enabled since Playback rate is %d", 
                    hDspCh->sDspAudioParams.sExtAudioParams.ePBRate));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

#ifdef RAP_SRSTRUVOL_CERTIFICATION
#else			
            #ifndef WMA_CERTIFICATION
    		if (bSupportedPpAlgo[BRAP_DSPCHN_PP_Algo_eSrc] == false)
		    {
                BDBG_ERR(("BRAP_DSPCHN_PP_Algo_eSrc MUST be enabled for 7401"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            #endif			
#endif			

 
            for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
            {
                for( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
                {
                    BDBG_MSG(("Branch %d Stage %d PP is %d",uiPpBrIndex,uiPpStgIndex,
                        hDspCh->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex]));
                }
            }

            err = BRAP_DSP_P_AllocatePpInterframeBuf(hDspCh->hDsp, &hDspCh->sPpBranchInfo[0], &hDspCh->uiPpInterframeBuf[0][0]);
            if (err!=BERR_SUCCESS)
                return BERR_TRACE(err);

            err = BRAP_DSPCHN_P_AllocateIsIsIfBuffers( hDspCh );
            if (err!=BERR_SUCCESS)
                return BERR_TRACE(err);
		}

#if (BRAP_DSP_P_7401_NEWMIT==1)
            BRAP_DSP_P_UpdateMitForIsIsIfBuffers(
                hDspCh->hDsp,
                hDspCh->channelIndex,
                hDspCh->uiDecInterstageOutputBuf,
                &( hDspCh->uiInterstageInputBuf[0][0] ),
                &( hDspCh->uiInterstageOutputBuf[0][0] ));
#endif	

		err = BRAP_DSP_P_InitInterframeBuffer(hDspCh->hDsp, hDspCh->channelIndex, hDspCh->uiInterframeBufIndex, psAudioParams->sExtAudioParams.eType);
		if (err!=BERR_SUCCESS)
			return BERR_TRACE(err);

#if (BRAP_DSP_P_7401_NEWMIT==1)
		for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
		{
			for ( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
			{
				err = BRAP_DSP_P_InitPpInterframeBuffer( hDspCh->hDsp, hDspCh->channelIndex, uiPpBrIndex, uiPpStgIndex, 
					hDspCh->uiPpInterframeBuf[uiPpBrIndex][uiPpStgIndex], hDspCh->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex] );
				if (err!=BERR_SUCCESS)
					return BERR_TRACE(err);
			}
		}
#endif	        
	}

#if BCHP_7411_VER
	/* If multi chan on I2S mode, then initialize DDBM table.
	 * TODO: Need re-architecting DDBM FW-PI interface. By definition tables are constants
	 * and need not be re-initialized on channel change. */
	if (psAudioParams->bMultiChanOnI2S) {
		err = BRAP_DSP_P_CopyFWImageToMem(hDspCh->hDsp, hDsp->memInfo.ddbmTblPtr, BRAP_IMG_DDBM_TABLE_ID);
		if (err!=BERR_SUCCESS)
			return BERR_TRACE(err);
	}
#endif	

	return BERR_SUCCESS;
}

BERR_Code
BRAP_DSPCHN_P_EnablePause(
						BRAP_DSPCHN_Handle      hDspChn,	/* [in] DSPCHN handle */
						bool					bOnOff		/* [In] TRUE = Pause audio 
																FALSE = Resume audio */
						)
{
	uint32_t regVal, uiCount;
	
	/* Assert on input parameters */
	BDBG_ASSERT(hDspChn);

	if (bOnOff) { /* Enable pause */
		regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset);
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, TRICK_PAUSE)))|
				BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, TRICK_PAUSE,1);
		BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset, regVal);

		BDBG_MSG(("Waiting for FW ack for decoder pause ..."));
		for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
    	   {
			regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hDspChn->chEsrSiRegOffset);
			if (BCHP_GET_FIELD_DATA(regVal, AUD_DSP_ESR_SI00_INT_STATUS,TRICK_PAUSE) == 0 )
		    	break;
			BKNI_Sleep(10);
		}
    	   if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
    	   {
        	    BDBG_ERR(("Timed Out for Decoder Pause command Ack"));
	   			return BERR_TRACE(BERR_TIMEOUT);
       		}
	   
		BDBG_MSG(("Received ack for Decoder Pause cmd"));
	}
	else { /* Disable pause, Resume audio */
		/* Clear STC valid bit */
		regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hDspChn->chCfgRegOffset);
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, STC_VALID)));
		BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hDspChn->chCfgRegOffset, regVal);

		regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset);
		regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, TRICK_RESUME)))|
				BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, TRICK_RESUME,1);
		BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset, regVal);

		BDBG_MSG(("Waiting for FW ack for audio resume ..."));
		for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
    	   {
			regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hDspChn->chEsrSiRegOffset);
			if (BCHP_GET_FIELD_DATA(regVal, AUD_DSP_ESR_SI00_INT_STATUS,TRICK_RESUME) == 0 )
		    	break;
			BKNI_Sleep(10);
		}
    	   if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
    	   {
        	    BDBG_ERR(("Timed Out for Audio Resume command Ack"));
	   			return BERR_TRACE(BERR_TIMEOUT);
       		}
	   
		BDBG_MSG(("Received ack for Audio Resume "));
	}
	return BERR_SUCCESS;
}


BERR_Code
BRAP_DSPCHN_P_GetOneAudioFrameTime(
						BRAP_DSPCHN_Handle      hDspChn,			/* [in] DSPCHN handle */
						unsigned int			*puiOneFrameTime	/* [out] One Audio Frame Time */
						)
{
	uint32_t regVal, stcIncrementRate;

	/* Assert on input parameters */
	BDBG_ASSERT(hDspChn);
	BDBG_ASSERT(puiOneFrameTime);

	 /* TSM upper threshold value is in 45 KHz clock for MPEG and 27 MHz clock for DSS */
	
	if (hDspChn->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssEs ||
		hDspChn->sDspAudioParams.sExtAudioParams.eStreamType==BAVC_StreamType_eDssPes) 
			stcIncrementRate = 27000000;
	else
		stcIncrementRate = 45000;

		/* One audio frame time is equivalent to TSM upper threshold programmed by FW 
		 * scheduler */

		regVal = BRAP_Read32(hDspChn->hRegister, 
				BCHP_AUD_DSP_CFG0_TSM_UPPER_THRESHOLD_CXT0 
				+ hDspChn->chCfgRegOffset);
#if BCHP_7411_VER        
		if (BRAP_DSPCHN_AudioType_eWmaStd == hDspChn->sDspAudioParams.sExtAudioParams.eType 
			||BRAP_DSPCHN_AudioType_eWmaPro == hDspChn->sDspAudioParams.sExtAudioParams.eType ) {
			/* For 7411 WMA/WMA Pro, TSM upper threshold is programmed by host and hence doesn't
			 * correspond to one audio frame time. So for 7411 WMA, FW programs one 
			 * audio frame time in register BCHP_AUD_DSP_CFG0_DECODE_OFFSET_CXT0
			 */
			regVal = BRAP_Read32(hDspChn->hRegister, 
					BCHP_AUD_DSP_CFG0_DECODE_OFFSET_CXT0 
					+ hDspChn->chCfgRegOffset);
		}
#endif        
			
		if (regVal==0) {
			BDBG_ERR(("TSM Upper Threshold is zero"));
			return BERR_TRACE(BRAP_ERR_BAD_DEVICE_STATE);
		}
		
		/* Convert TSM upper threshold value into time in millisecond. */
		*puiOneFrameTime = (regVal * 1000)/stcIncrementRate; 

	return BERR_SUCCESS;
}

BERR_Code
BRAP_DSPCHN_P_FrameAdvance(
						BRAP_DSPCHN_Handle      hDspChn,			/* [in] DSPCHN handle */
						unsigned int			uiNumFrames			/* [in] Num of frames to be 
																	   advanced */
						)
{
	uint32_t regVal, uiCount;

	/* Assert on input parameters */
	BDBG_ASSERT(hDspChn);

	/* Program num of audio frames to be advanced */
	regVal =  BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0
							+ hDspChn->chCfgRegOffset);
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, FRAME_ADVANCE))) |
			BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, FRAME_ADVANCE, uiNumFrames);
	BRAP_Write32(hDspChn->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0
							+ hDspChn->chCfgRegOffset, regVal);

	/* Give "Frame Advance" command to firmware */
	regVal = BRAP_Read32(hDspChn->hRegister, BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset);
	regVal = (regVal & (~BCHP_MASK(AUD_DSP_ESR_SI00_INT_SET, TRICK_FF)))|
			BCHP_FIELD_DATA(AUD_DSP_ESR_SI00_INT_SET, TRICK_FF,1);
	BRAP_Write32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_SET + hDspChn->chEsrSiRegOffset, regVal);

	BDBG_MSG(("Waiting for FW ack for Frame Advance ..."));
	for ( uiCount = 0; uiCount < BRAP_DSPCHN_P_FW_CMD_TIMEOUT; uiCount++)
    {
		regVal = BRAP_Read32(hDspChn->hRegister,BCHP_AUD_DSP_ESR_SI00_INT_STATUS + hDspChn->chEsrSiRegOffset);
		if (BCHP_GET_FIELD_DATA(regVal, AUD_DSP_ESR_SI00_INT_STATUS,TRICK_FF) == 0 )
	    	break;
		BKNI_Sleep(10);
	}
    if ( uiCount >= BRAP_DSPCHN_P_FW_CMD_TIMEOUT )
    {
   	    BDBG_ERR(("Timed Out for Frame Advance command Ack"));
		return BERR_TRACE(BERR_TIMEOUT);
   	}
	   
	BDBG_MSG(("Received ack for Frame Advance cmd"));

	return BERR_SUCCESS;
}

/*
	This routine is called from BRAP_DSPCHN_P_Stop() to 
	reduce the numUser and reset the algotype and decMode.
*/	
static BERR_Code BRAP_DSPCHN_P_DecrementCtxBufUsage(
	BRAP_DSP_Handle hDsp, 
	BRAP_DSPCHN_AudioType eAudType, 
	BRAP_DSPCHN_DecodeMode eDecMode)
{
	uint32_t i; 	

#if (BCHP_CHIP == 7400)
	/* Check if FS is present in DRAM & decrement numUser*/
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF-1;i++)
    {
        if((hDsp->ctxtFsBufUsage[i].eAlgoType == BRAP_DSPCHN_AudioType_eAc3Plus)&& (eDecMode ==BRAP_DSPCHN_DecodeMode_ePassThru))
        {
        	break;
        }
        else if(hDsp->ctxtFsBufUsage[i].eAlgoType == eAudType)
        {
        	if(hDsp->ctxtFsBufUsage[i].numUser != 0)
        	{
        		hDsp->ctxtFsBufUsage[i].numUser--;
        		if( hDsp->ctxtFsBufUsage[i].numUser == 0)
        		{
        			hDsp->ctxtFsBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
        		}
        		BDBG_MSG(("ChannelStop: Decreased FSCtxt(%d) numUser ->(%d)", i, hDsp->ctxtFsBufUsage[i].numUser));
        	}
        	else
        	{
        		BDBG_ERR(("ChannelStop: FS numUser is already 0", hDsp->ctxtFsBufUsage[i].numUser));
        		return BERR_TRACE(BERR_UNKNOWN);
        	}
        	break;
        }          
	}     
#else
	/* Check if FS is present in DRAM & decrement numUser*/
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FS_CTXT_IN_FS_EXE_BUF; i++)
	{
		if(hDsp->ctxtFsBufUsage[i].eAlgoType == eAudType)
		{
			if(hDsp->ctxtFsBufUsage[i].numUser != 0)
			{
				hDsp->ctxtFsBufUsage[i].numUser--;
				if( hDsp->ctxtFsBufUsage[i].numUser == 0)
				{
					hDsp->ctxtFsBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
				}
				BDBG_MSG(("ChannelStop: Decreased FSCtxt(%d) numUser ->(%d)", i, hDsp->ctxtFsBufUsage[i].numUser));
			}
			else
			{
				BDBG_ERR(("ChannelStop: FS numUser is already 0", hDsp->ctxtFsBufUsage[i].numUser));
				return BERR_TRACE(BERR_UNKNOWN);
			}
			break;
		}
	}
#endif

#if (BCHP_CHIP == 7400)
    /* For DDP passthrough Saparate frame sync is downloaded.So need to check for it and decrement the numUser*/
    if((hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].eAlgoType == BRAP_DSPCHN_AudioType_eAc3Plus)&& (eDecMode ==BRAP_DSPCHN_DecodeMode_ePassThru)) 
    {
        if(hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].numUser != 0)
        {
            hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].numUser--;
            BDBG_MSG(("ChannelStop : Decreased FSCtxt(%d) numUser ->(%d)", (BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1),
                        hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].numUser));
			if( hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].numUser == 0)
			{
				hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
			}
            
        }
        else
        {
        	BDBG_ERR(("ChannelStop: FS numUser is already 0", hDsp->ctxtFsBufUsage[BRAP_DSP_P_DDP_FS_CTXT_EXE_BUF-1].numUser));
        	return BERR_TRACE(BERR_UNKNOWN);
        }
    }
#endif    


	/* Check if FW is present in DRAM & decrement numUser*/
	for(i=0; i<BRAP_DSP_P_MAX_NUM_FW_CTXT_IN_FW_EXE_BUF; i++)
	{
        if(eDecMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
        {        
    		if(hDsp->ctxtFwBufUsage[i].eAlgoType == eAudType &&
    		   hDsp->ctxtFwBufUsage[i].eDecodeMode == BRAP_DSPCHN_DecodeMode_eDecode)
    		{
    			if(hDsp->ctxtFwBufUsage[i].numUser != 0)
    			{
    				hDsp->ctxtFwBufUsage[i].numUser--;
                    
    				BDBG_MSG(("ChannelStop: Decreased FWCtxt(%d) algoType(0x%x) decMode(0x%x) numUser ->(%d)", 
    						i, hDsp->ctxtFwBufUsage[i].eAlgoType, hDsp->ctxtFwBufUsage[i].eDecodeMode, 
    						hDsp->ctxtFwBufUsage[i].numUser));
                    
    				if( hDsp->ctxtFwBufUsage[i].numUser == 0)
    				{
    					hDsp->ctxtFwBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
    					hDsp->ctxtFwBufUsage[i].eDecodeMode = BRAP_DSPCHN_DecodeMode_eInvalid;
    				}
    			}
    			else
    			{
    				BDBG_ERR(("ChannelStop: FW numUser is already 0", hDsp->ctxtFwBufUsage[i].numUser));
    				return BERR_TRACE(BERR_UNKNOWN);
    			}
    		}
     
    		if(hDsp->ctxtFwBufUsage[i].eAlgoType == eAudType &&
    		   hDsp->ctxtFwBufUsage[i].eDecodeMode == BRAP_DSPCHN_DecodeMode_eSimulMode)
    		{
    			if(hDsp->ctxtFwBufUsage[i].numUser != 0)
    			{
    				hDsp->ctxtFwBufUsage[i].numUser--;
                    
    				BDBG_MSG(("ChannelStop: Decreased FWCtxt(%d) algoType(0x%x) decMode(0x%x) numUser ->(%d)", 
    						i, hDsp->ctxtFwBufUsage[i].eAlgoType, hDsp->ctxtFwBufUsage[i].eDecodeMode, 
    						hDsp->ctxtFwBufUsage[i].numUser));
                    
    				if( hDsp->ctxtFwBufUsage[i].numUser == 0)
    				{
    					hDsp->ctxtFwBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
    					hDsp->ctxtFwBufUsage[i].eDecodeMode = BRAP_DSPCHN_DecodeMode_eInvalid;
    				}
    			}
    			else
    			{
    				BDBG_ERR(("ChannelStop: FW numUser is already 0", hDsp->ctxtFwBufUsage[i].numUser));
    				return BERR_TRACE(BERR_UNKNOWN);
    			}
    		}
        }
        else
        {
    		if(hDsp->ctxtFwBufUsage[i].eAlgoType == eAudType &&
    		   hDsp->ctxtFwBufUsage[i].eDecodeMode == eDecMode)
    		{
    			if(hDsp->ctxtFwBufUsage[i].numUser != 0)
    			{
    				hDsp->ctxtFwBufUsage[i].numUser--;
                    
    				BDBG_MSG(("ChannelStop: Decreased FWCtxt(%d) algoType(0x%x) decMode(0x%x) numUser ->(%d)", 
    						i, hDsp->ctxtFwBufUsage[i].eAlgoType, hDsp->ctxtFwBufUsage[i].eDecodeMode, 
    						hDsp->ctxtFwBufUsage[i].numUser));
                    
    				if( hDsp->ctxtFwBufUsage[i].numUser == 0)
    				{
    					hDsp->ctxtFwBufUsage[i].eAlgoType = BRAP_DSPCHN_AudioType_eInvalid;
    					hDsp->ctxtFwBufUsage[i].eDecodeMode = BRAP_DSPCHN_DecodeMode_eInvalid;
    				}
    			}
    			else
    			{
    				BDBG_ERR(("ChannelStop: FW numUser is already 0", hDsp->ctxtFwBufUsage[i].numUser));
    				return BERR_TRACE(BERR_UNKNOWN);
    			}
    		}     
        }
        
	}
	return BERR_SUCCESS;
}

BERR_Code 
BRAP_DSPCHN_P_GetCurrentPpConfig( 
		BRAP_DSPCHN_Handle hDspCh, 
		unsigned int uiPpBranchId,
		unsigned int uiPpStageId,
		BRAP_DSPCHN_PP_Algo *ePpAlgo, 
		BRAP_DSPCHN_P_PpStgConfigParam *uPpStgConfigParam
	)
{
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( ePpAlgo );
	BDBG_ASSERT( uPpStgConfigParam );

	if( uiPpBranchId >= BAF_MAX_BRANCHES )
        {
            BDBG_ERR(("Invalid Branch Id uiPpBranchId =%d",uiPpBranchId));
		return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

    	if( uiPpStageId >= BAF_MAX_POSTPROCESS_STAGES )
        {
            BDBG_ERR(("Invalid Stage Id uiPpStageId =%d",uiPpStageId));
		return BERR_TRACE( BERR_INVALID_PARAMETER );
        }

    
	if( uiPpStageId >= BRAP_MAX_PP_PER_BRANCH_SUPPORTED )
		return BERR_TRACE( BERR_INVALID_PARAMETER );

	*ePpAlgo = hDspCh->sPpBranchInfo[uiPpBranchId].sPpStgSettings.ePpAlgo[uiPpStageId];

	if( BRAP_DSPCHN_PP_Algo_eMax!=*ePpAlgo )
	{
		if( true==hDspCh->bUseDefaultConfig[uiPpBranchId][uiPpStageId] )
		{
			switch( *ePpAlgo )
			{
#if (BRAP_AD_SUPPORTED == 1)
				case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
					uPpStgConfigParam->sAdFadeConfigParams
						= hDspCh->sSettings.sPPSettings.sADFadeCtrlConfigParams;
					break;
				case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
					uPpStgConfigParam->sAdPanConfigParams
						= hDspCh->sSettings.sPPSettings.sADPanCtrlConfigParams;
					break;
#endif					
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)					
				case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
					uPpStgConfigParam->sDolbyVolumeConfigParams
						= hDspCh->sSettings.sPPSettings.sDolbyVolumeConfigParams;
					break;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)					
				case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
					uPpStgConfigParam->sSRSTruVolConfigParams
						= hDspCh->sSettings.sPPSettings.sSRSTruVolConfigParams;
					break;
#endif					
				case BRAP_DSPCHN_PP_Algo_eSrc:
					break;
				case BRAP_DSPCHN_PP_Algo_eDdbm:
					break;
				default:
					break;
			}
		}
		else
		{
			switch( *ePpAlgo )
			{
#if (BRAP_AD_SUPPORTED == 1)
				case BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl:
					uPpStgConfigParam->sAdFadeConfigParams
						= hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStageId].sAdFadeConfigParams;
					break;
				case BRAP_DSPCHN_PP_Algo_eAD_PanCtrl:
					uPpStgConfigParam->sAdPanConfigParams
						= hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStageId].sAdPanConfigParams;
					break;
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
				case BRAP_DSPCHN_PP_Algo_eDolbyVolume:
					uPpStgConfigParam->sDolbyVolumeConfigParams
						= hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStageId].sDolbyVolumeConfigParams;
					break;           
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
				case BRAP_DSPCHN_PP_Algo_eSRS_TruVol:
					uPpStgConfigParam->sSRSTruVolConfigParams
						= hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStageId].sSRSTruVolConfigParams;
					break;           
#endif					
				case BRAP_DSPCHN_PP_Algo_eSrc:
					break;
				case BRAP_DSPCHN_PP_Algo_eDdbm:
					break;
				default:
					break;
			}
		}
	}

	return BERR_SUCCESS;
}




BERR_Code BRAP_DSPCHN_P_GetCurrentSettings (
						BRAP_DSPCHN_Handle	hDspCh,
						BRAP_DSPCHN_P_Settings	*psDspChSettings)
{
	BDBG_ASSERT(hDspCh);
	BDBG_ASSERT(psDspChSettings);

	*psDspChSettings = hDspCh->sSettings;
	return BERR_SUCCESS;
}

BERR_Code BRAP_DSPCHN_P_GetCurrentAudioParams (
						BRAP_DSPCHN_Handle	hDspCh,
						BRAP_DSPCHN_P_AudioParams	*psDspChAudioParams)
{
	BDBG_ASSERT(hDspCh);
	BDBG_ASSERT(psDspChAudioParams);

	*psDspChAudioParams = hDspCh->sDspAudioParams;
	return BERR_SUCCESS;
}

bool BRAP_DSPCHN_P_GetDecoderPauseState (
						BRAP_DSPCHN_Handle	hDspCh
						)
{
	uint32_t regVal;
	bool bDecoderPaused;
	
	BDBG_ASSERT(hDspCh);
	
	regVal = BRAP_Read32(hDspCh->hRegister,
				BCHP_AUD_DSP_CFG0_TSM_STATUS_CXT0 + hDspCh->chCfgRegOffset);

	bDecoderPaused = BCHP_GET_FIELD_DATA(regVal,
						AUD_DSP_CFG0_TSM_STATUS_CXT0,
						PAUSE_STATUS);
	return 	bDecoderPaused;				
}

static BERR_Code 
BRAP_DSPCHN_P_FormSpeakersMatrix (
				BRAP_DSPCHN_Handle	hDspCh,
				uint32_t *pSpeakers,
				BRAP_DEC_DownmixPath eDownMixPath )
{

	BRAP_OutputMode eOutputMode;
	bool bMonoToAll;
	unsigned int i=0;
	BERR_Code ret = BERR_SUCCESS;
    
	BDBG_ENTER(BRAP_DSPCHN_P_FormSpeakersMatrix);

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (pSpeakers);

	eOutputMode = hDspCh->sSettings.eOutputMode[eDownMixPath];
	bMonoToAll = hDspCh->sSettings.bMonoToAll[eDownMixPath];

	for (i=0; i<BRAP_DSPCHN_P_SpeakerConfig_eMax; i++)
	{
		pSpeakers[i] = BRAP_INVALID_VALUE;
	}

	if ( eDownMixPath == BRAP_DEC_DownmixPath_eStereoDownmix && eOutputMode != BRAP_OutputMode_e2_0 )
	{
		BDBG_ERR(("For the stereo downmix path the outputmode %d is not supported", eOutputMode));
		return BERR_TRACE(BERR_NOT_SUPPORTED);		
	}

	switch (hDspCh->sSettings.eOutputMode[eDownMixPath]) 
	{
		case BRAP_OutputMode_e1_0:
			if (bMonoToAll == true)
			{   /* Downmixed mono to both speakers */
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;
			}
			else
			{   /* Downmixed mono to only Left speaker, zero to the right speaker */
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_FILL_ZERO;
			}
			break;
		case BRAP_OutputMode_e2_0:
			if ( eDownMixPath == BRAP_DEC_DownmixPath_eMain )
			{
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			}
			else
			{
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eBackLeft]=BRAP_DSPCHN_P_SpeakerConfig_eBackLeft;
				pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eBackRight]=BRAP_DSPCHN_P_SpeakerConfig_eBackRight;          
			}
			break;
#if ( BCHP_CHIP == 7400 )			
		case BRAP_OutputMode_e3_0:
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eCenter]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLFE]=BRAP_DSPCHN_P_SpeakerConfig_eLFE;
			break;
		case BRAP_OutputMode_e2_1:
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr]=BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eRightSurr]=BRAP_DSPCHN_P_SpeakerConfig_eRightSurr;
			break;
		case BRAP_OutputMode_e3_1:
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eCenter]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLFE]=BRAP_DSPCHN_P_SpeakerConfig_eLFE;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr]=BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eRightSurr]=BRAP_DSPCHN_P_SpeakerConfig_eRightSurr;
			break;
		case BRAP_OutputMode_e2_2:			
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eRightSurr]=BRAP_DSPCHN_P_SpeakerConfig_eRightSurr;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr]=BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr;
			break;
		case BRAP_OutputMode_e3_2:			
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft]=BRAP_DSPCHN_P_SpeakerConfig_eFrontLeft;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eFrontRight]=BRAP_DSPCHN_P_SpeakerConfig_eFrontRight;          
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eRightSurr]=BRAP_DSPCHN_P_SpeakerConfig_eRightSurr;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr]=BRAP_DSPCHN_P_SpeakerConfig_eLeftSurr;
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eCenter]=BRAP_DSPCHN_P_SpeakerConfig_eCenter;			
			pSpeakers[BRAP_DSPCHN_P_SpeakerConfig_eLFE]=BRAP_DSPCHN_P_SpeakerConfig_eLFE;
			break;
#endif						
		default:
			BDBG_ERR(("Output Mode %d not supported ", eOutputMode));
			return BERR_TRACE(BERR_NOT_SUPPORTED);
	}   

    for (i=0; i<BRAP_DSPCHN_P_SpeakerConfig_eMax; i++)
    {
        BDBG_MSG(("pSpeakers[%d]=%d", i,pSpeakers[i]));
    }

    BDBG_LEAVE(BRAP_DSPCHN_P_FormSpeakersMatrix);
    return ret;
}

#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)

static void 
BRAP_DSPCHN_P_FormSRSVolumeConfigStruct (BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	)
{
	BRAP_DSPCHN_SRSTruVolConfigParams sSRSTruVolConfigParams;
	unsigned int uiInputGain = 0,uiOutputGain = 0,uiBypassGain = 0,uiMaxGain = 0,ui32ReferenceLevel =0,uiNoiseManagerThreshold =0;

	BDBG_ASSERT( avlParamsPtr );
	BDBG_ASSERT( hDspCh );


	if( bUseDefault )
	{
		sSRSTruVolConfigParams = hDspCh->sSettings.sPPSettings.sSRSTruVolConfigParams;
	}
	else
	{
		sSRSTruVolConfigParams = hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStgId].sSRSTruVolConfigParams;
	}

	uiInputGain = BRAP_DSPCHN_P_FloatToFwFormat1(sSRSTruVolConfigParams.i32InputGain);
	uiOutputGain= BRAP_DSPCHN_P_FloatToFwFormat2(sSRSTruVolConfigParams.i32OutputGain);
	uiBypassGain= BRAP_DSPCHN_P_FloatToFwFormat4(sSRSTruVolConfigParams.i32BypassGain);
	uiMaxGain= BRAP_DSPCHN_P_FloatToFwFormat3(sSRSTruVolConfigParams.i32MaxGain);        
	ui32ReferenceLevel= BRAP_DSPCHN_P_FloatToFwFormat2(sSRSTruVolConfigParams.i32ReferenceLevel);
	uiNoiseManagerThreshold= BRAP_DSPCHN_P_FloatToFwFormat2(sSRSTruVolConfigParams.i32NoiseManagerThreshold);        

	BDBG_MSG(("SRSTruVol bTrueVolume_enable = %d\n",sSRSTruVolConfigParams.bTrueVolume_enable));   
	
	/* Program SRS parameters in the buffer */
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.bTrueVolume_enable, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32Nchans, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32BlockSize, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.bMEnable, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, uiInputGain, 4 ); avlParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, uiOutputGain, 4 ); avlParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, uiBypassGain, 4 ); avlParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, ui32ReferenceLevel, 4 ); avlParamsPtr += 4;	
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32LimiterControl, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32Mode, 4); avlParamsPtr+=4;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32Size, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, uiMaxGain, 4 ); avlParamsPtr += 4;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sSRSTruVolConfigParams.i32NoiseManager, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, uiNoiseManagerThreshold, 4 ); avlParamsPtr += 4;    	        
}
#endif

#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)

static void 
BRAP_DSPCHN_P_FormDolbyVolumeConfigStruct (BRAP_DSPCHN_Handle hDspCh,
            	unsigned char *avlParamsPtr, 
            	bool	bUseDefault,
            	unsigned int uiPpBranchId,
            	unsigned int uiPpStgId
            	)
{
	BRAP_DSPCHN_DolbyVolumeConfigParams sDolbyVolumeConfigParams;
	uint32_t i32Input_reference_level = 0,i32Output_reference_level = 0;
    uint32_t i32Calibration = 0,i32Digital_volume_level = 0,i32Analog_volume_level =0;

	BDBG_ASSERT( avlParamsPtr );
	BDBG_ASSERT( hDspCh );


	if( bUseDefault )
	{
		sDolbyVolumeConfigParams = hDspCh->sSettings.sPPSettings.sDolbyVolumeConfigParams;
	}
	else
	{
        sDolbyVolumeConfigParams = hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStgId].sDolbyVolumeConfigParams;
	}

	BDBG_MSG(("DolbyVolume bVolume_iq_enable = %d\n",sDolbyVolumeConfigParams.bVolume_iq_enable));   
	
    i32Input_reference_level = BRAP_DSPCHN_P_FloatToQ824(sDolbyVolumeConfigParams.i32Input_reference_level);
    i32Output_reference_level = BRAP_DSPCHN_P_FloatToQ824(sDolbyVolumeConfigParams.i32Output_reference_level);
    i32Calibration = BRAP_DSPCHN_P_FloatToQ824(sDolbyVolumeConfigParams.i32Calibration);
    i32Digital_volume_level = BRAP_DSPCHN_P_FloatToQ824(sDolbyVolumeConfigParams.i32Digital_volume_level);
    i32Analog_volume_level = BRAP_DSPCHN_P_FloatToQ824(sDolbyVolumeConfigParams.i32Analog_volume_level);
    	
	/* Program Dolby parameters in the buffer */
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bVolume_iq_enable, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.i32Block_size, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.i32Nbands, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.i32Nchans, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, i32Input_reference_level, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, i32Output_reference_level, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, i32Calibration, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bVlm_enable, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.i32Reset_now_flag, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, i32Digital_volume_level, 4); avlParamsPtr+=4;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, i32Analog_volume_level, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.i32Lvl_amount, 4); avlParamsPtr+=4;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bLvl_disable, 4); avlParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bEnable_midside_proc, 4); avlParamsPtr+=4;    
	BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bHalfmode_flag, 4); avlParamsPtr+=4;	
    BRAP_DSPCHN_P_AddWordToFWBuffer(avlParamsPtr, sDolbyVolumeConfigParams.bLimiter_enable, 4); avlParamsPtr+=4;                      
}
#endif

#if defined ( BCHP_7411_VER ) || (BRAP_7401_FAMILY == 1)
static void 
BRAP_DSPCHN_P_FormAc3ConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *ac3ParamsPtr)
{
	unsigned int row, count;
	BRAP_DSPCHN_Ac3ConfigParams sAc3ConfigParams;
	uint32_t ui32OutputMode = 0;
	BRAP_DSPCHN_DualMonoMode	eDualMonoMode;
#if (BRAP_7401_FAMILY == 1)       
    uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];
#endif


	BDBG_ASSERT( ac3ParamsPtr );
	BDBG_ASSERT( hDspCh );

	sAc3ConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3ConfigParam;
    
	/* Map eOutputMode enum values into programming values required by AC3 decoder */
	switch (hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain]) {
		case BRAP_OutputMode_e1_0:
			ui32OutputMode = 1;
			break;
		case BRAP_OutputMode_e1_1:
			ui32OutputMode = 0;
			break;
		case BRAP_OutputMode_e2_0:
			ui32OutputMode = 2;
			break;
		case BRAP_OutputMode_e3_0:
			ui32OutputMode = 3;
			break;
		case BRAP_OutputMode_e2_1:
			ui32OutputMode = 4;
			break;
		case BRAP_OutputMode_e3_1:
			ui32OutputMode = 5;
			break;
		case BRAP_OutputMode_e2_2:
			ui32OutputMode = 6;
			break;
		case BRAP_OutputMode_e3_2:
			ui32OutputMode = 7;
			break;
		default:
			break;
	}

	/* TODO: Use common dual mono mode for programming AC3 dual mono mode
	 * parameter */
	eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode;

	/* Program AC3 parameters in the buffer */
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ115(sAc3ConfigParams.ui16DynRngScaleHi), 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ115(sAc3ConfigParams.ui16DynRngScaleLo), 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ115(sAc3ConfigParams.ui16PcmScaleFactor), 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eCompMode, 2); ac3ParamsPtr+=2;
	/* TODO: Use common dual mono mode for programming AC3 dual mono mode parameter */
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eDualMono, 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, ui32OutputMode, 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bOutputLfeOn, 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eKaraokeVocalSelect, 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bUseKaraokeLevelPanCoeff, 2); ac3ParamsPtr+=2;
	for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_KARAOKE_COEFF; count++){
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ115(sAc3ConfigParams.i16KaraokeLevelPanCoeff[count]), 2); ac3ParamsPtr+=2;
	}
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eStereoMode, 2); ac3ParamsPtr+=2;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bUseUserDownmixCoeff, 2); ac3ParamsPtr+=2;
	for (row = 0; row < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_ROWS; row++)
		for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_COLUMNS; count++){
			BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ115(sAc3ConfigParams.i16UserDownmixCoeff[row][count]), 2); ac3ParamsPtr+=2;
	}

#if (BRAP_7401_FAMILY == 1)      
    BDBG_MSG(("AC3 param: writing NumActiveSpeakers array to addr 0x%08x", ac3ParamsPtr));
    BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
    for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
    {
        BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], ac3ParamsPtr));
        BRAP_DSPCHN_P_AddWordToFWBuffer (ac3ParamsPtr, eSpeakerConfigMatrix[count], 2); ac3ParamsPtr+=2;
    }    
#endif           
}
#else
static void 
BRAP_DSPCHN_P_FormDDPConfigStructForAC3 (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *ac3ParamsPtr)
{
	unsigned int row, count;
	BRAP_DSPCHN_Ac3ConfigParams sAc3ConfigParams;
	uint32_t ui32OutputMode = 2;
	BRAP_DSPCHN_DualMonoMode	eDualMonoMode;
	unsigned int dualMono = 0;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;
       uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	BDBG_ASSERT( ac3ParamsPtr );
	BDBG_ASSERT( hDspCh );

	sAc3ConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3ConfigParam;

	/* Program AC3 Plus parameters in the buffer */
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eCompMode, 4); ac3ParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3ConfigParams.ui16PcmScaleFactor), 4); ac3ParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3ConfigParams.ui16DynRngScaleHi), 4); ac3ParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3ConfigParams.ui16DynRngScaleLo), 4); ac3ParamsPtr+=4;

	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, numDownmixPaths, 4); ac3ParamsPtr+=4;

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		sAc3ConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sAc3ConfigParam;    
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;

		/* DDP dual mono values are not in sequence with BRAP_DSPCHN_DualMonoMode enum */
		switch (eDualMonoMode) {
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				dualMono = 1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				dualMono = 2; 
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				dualMono = 0;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				dualMono = 3;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bOutputLfeOn, 4); ac3ParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, ui32OutputMode, 4); ac3ParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eStereoMode, 4); ac3ParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, dualMono, 4); ac3ParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.eKaraokeVocalSelect, 4); ac3ParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bUseUserDownmixCoeff, 4); ac3ParamsPtr+=4;
		for (row = 0; row < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3ConfigParams.i16UserDownmixCoeff[row][count]), 4); ac3ParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, sAc3ConfigParams.bUseKaraokeLevelPanCoeff, 4); ac3ParamsPtr+=4;
		for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_KARAOKE_COEFF; count++){
			BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3ConfigParams.i16KaraokeLevelPanCoeff[count]), 4); ac3ParamsPtr+=4;
		}

                BDBG_MSG(("AC3 param: writing NumActiveSpeakers array to addr 0x%08x", ac3ParamsPtr));
                BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
                for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
                {
                    BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], ac3ParamsPtr));
                    BRAP_DSPCHN_P_AddWordToFWBuffer (ac3ParamsPtr, eSpeakerConfigMatrix[count], 4); ac3ParamsPtr+=4;
                }    

	}
	sAc3ConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3ConfigParam;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3ParamsPtr,sAc3ConfigParams.bDialNormEnable, 4); ac3ParamsPtr+=4;	
}
#endif

static void
BRAP_DSPCHN_P_FormAc3LosslessConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *ac3PlusParamsPtr)
{
	unsigned int row, count;
	BRAP_DSPCHN_Ac3PlusConfigParams sAc3LosslessConfigParam;
	uint32_t ui32OutputMode = 2;
	BRAP_DSPCHN_DualMonoMode	eDualMonoMode;
	unsigned int dualMono = 0;
	int i = 0,  maxDnMixPaths = 0;

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	int numDownmixPaths = 1;
#endif

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( ac3PlusParamsPtr );

	sAc3LosslessConfigParam = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3LosslessConfigParam;

	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.eCompMode, 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3LosslessConfigParam.ui16PcmScaleFactor), 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3LosslessConfigParam.ui16DynRngScaleHi), 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3LosslessConfigParam.ui16DynRngScaleLo), 4); ac3PlusParamsPtr+=4;

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, numDownmixPaths, 4); ac3PlusParamsPtr+=4;
#else
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		sAc3LosslessConfigParam = hDspCh->sSettings.sDspchnCfgParams[i].sAc3LosslessConfigParam;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;

		/* DDP dual mono values are not in sequence with BRAP_DSPCHN_DualMonoMode enum */
		switch (eDualMonoMode) {
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				dualMono = 1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				dualMono = 2; 
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				dualMono = 0;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				dualMono = 3;
				break;
			default:
				break;
		}
		
		/* Program AC3 Plus parameters in the buffer */
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.bOutputLfeOn, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, ui32OutputMode, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.eStereoMode, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, dualMono, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.eKaraokeVocalSelect, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.bUseUserDownmixCoeff, 4); ac3PlusParamsPtr+=4;
		for (row = 0; row < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3LosslessConfigParam.i16UserDownmixCoeff[row][count]), 4); ac3PlusParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3LosslessConfigParam.bUseKaraokeLevelPanCoeff, 4); ac3PlusParamsPtr+=4;
		for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_KARAOKE_COEFF; count++){
			BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3LosslessConfigParam.i16KaraokeLevelPanCoeff[count]), 4); ac3PlusParamsPtr+=4;
		}
	}
}

static void 
BRAP_DSPCHN_P_FormMpegConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *mpegParamsPtr)
{
	BRAP_OutputMode eOutputMode = hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain];
	BRAP_DSPCHN_DualMonoMode eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[BRAP_DEC_DownmixPath_eMain].eDualMonoMode;
	unsigned int outmode;
#ifdef BCHP_7411_VER
	unsigned int swapVal;
#endif	
	unsigned int count = 0;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;

	BDBG_ASSERT (mpegParamsPtr);
	BDBG_ASSERT (hDspCh);

#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(mpegParamsPtr, numDownmixPaths, 4); mpegParamsPtr+=4;
#else
	numDownmixPaths = 1;
	maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif	

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		eOutputMode = hDspCh->sSettings.eOutputMode[i];
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;
#ifdef BCHP_7411_VER
		/* TODO (Only for 7411): Values are swapped as a workaround fix for PR 20417. Later on, this needs to be either moved to function 
		 * BRAP_DSPCHN_P_AddWordToFWBuffer or should be done at the time of actual DRAM write.
		 * Background: Firmware team wants to change DMA type from D0S0 to D2S0 across all the algorithm decoders. This will
		 * make their code endian independent. For D2S0, host needs to swap decoder configuration parameters before writing
		 * into DRAM. Currently only MPEG decoder uses D2S0, hence this workaround is here. 
		 * This applies only for 7411. For non-7411, swapping is not required.
		 * When firmware team changes DMA type for all the algorithms, move swap to appropriate low level function that is common for 
		 * all the algorithms.
		 */
		outmode = (eOutputMode >= BRAP_OutputMode_e2_0)? 0: 1;
		swapVal = ((outmode & 0xFF) << 24) | ((outmode & 0xFF00) << 8) | ((outmode & 0xFF0000) >> 8) | ((outmode & 0xFF000000) >> 24);
		BRAP_DSPCHN_P_AddWordToFWBuffer (mpegParamsPtr, swapVal, 4); mpegParamsPtr+=4;

		swapVal = ((eDualMonoMode & 0xFF) << 24) | ((eDualMonoMode & 0xFF00) << 8) | ((eDualMonoMode & 0xFF0000) >> 8) | ((eDualMonoMode & 0xFF000000) >> 24);
		BRAP_DSPCHN_P_AddWordToFWBuffer (mpegParamsPtr, swapVal, 4); mpegParamsPtr+=4;
#else
		outmode = (eOutputMode >= BRAP_OutputMode_e2_0)? 0: 1;

		BDBG_MSG(("outmode=%d, eOutputMode=%d, eDualMonoMode=%d, numDownmixPaths=%d, maxDnMixPaths=%d", 
                   outmode, eOutputMode, eDualMonoMode, numDownmixPaths, maxDnMixPaths));
                
		BRAP_DSPCHN_P_AddWordToFWBuffer (mpegParamsPtr, outmode, 4); mpegParamsPtr+=4;

		BDBG_MSG(("MPEG param: writing NumActiveSpeakers array to addr 0x%08x", mpegParamsPtr)); 
		BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],i);
		for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
		{
			BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], mpegParamsPtr)); 
			BRAP_DSPCHN_P_AddWordToFWBuffer (mpegParamsPtr, eSpeakerConfigMatrix[count], 4); mpegParamsPtr+=4;
		}
		
		BRAP_DSPCHN_P_AddWordToFWBuffer (mpegParamsPtr, eDualMonoMode, 4); mpegParamsPtr+=4;
#endif	
	}

}

static void 
BRAP_DSPCHN_P_FormAacConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *aacParamsPtr)
{
	unsigned int OutputMode = 2;
	unsigned int dualMono = 0;
	BRAP_DSPCHN_AacConfigParams sAacConfigParams;
	unsigned int drcTargetLevel;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;
	unsigned int row, count;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];
	
	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (aacParamsPtr);

#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(aacParamsPtr, numDownmixPaths, 4); aacParamsPtr+=4;	
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	sAacConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAacConfigParam;
	drcTargetLevel = (sAacConfigParams.ui32DrcTargetLevel > 127)? 127: sAacConfigParams.ui32DrcTargetLevel;
    


	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.eDownmixType, 4); aacParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.ui32Arib_matrix_mixdown_index, 4); aacParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacConfigParams.ui32DrcGainControlCompress), 4); aacParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacConfigParams.ui32DrcGainControlBoost), 4); aacParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, drcTargetLevel, 4); aacParamsPtr+=4;

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		switch (hDspCh->sSettings.eOutputMode[i])
		{
			case BRAP_OutputMode_e1_0:
				OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				OutputMode = 7;
				break;
			default:
				break;
		}

		switch (hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode)
		{
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				dualMono = 1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				dualMono = 2; 
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				dualMono = 0;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				dualMono = 3;
				break;
			default:
				break;
		}

		sAacConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sAacConfigParam;

		BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.ui32bOutputLfeOn, 4); aacParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, OutputMode, 4); aacParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, dualMono, 4); aacParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.ui32ExtDownmixEnable, 4); aacParamsPtr+=4;
		for (row = 0; row < BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_ROWS; row++)
		{
			for (count = 0; count < BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_COLUMNS; count++)
			{
				BRAP_DSPCHN_P_AddWordToFWBuffer(aacParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacConfigParams.ui32ExtDnMixCoeffs[row][count]), 4); aacParamsPtr+=4;
			}
		}

        BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
        for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
        {
            BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], aacParamsPtr));
            BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, eSpeakerConfigMatrix[count], 4); aacParamsPtr+=4;
        } 
	}
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.ui32PcmBoost6dB, 4); aacParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacParamsPtr, sAacConfigParams.bPcmBoostMinus4p75dB, 4); aacParamsPtr+=4;		
}

static void 
BRAP_DSPCHN_P_FormAacSbrConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *aacSbrParamsPtr)
{
	unsigned int OutputMode = 2;
	unsigned int dualMono = 0;
	BRAP_DSPCHN_AacSbrConfigParams sAacSbrConfigParams;
	unsigned int drcTargetLevel;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	unsigned int row, count;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (aacSbrParamsPtr);

#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)	
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(aacSbrParamsPtr, numDownmixPaths, 4); aacSbrParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	sAacSbrConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAacSbrConfigParam;
	drcTargetLevel = (sAacSbrConfigParams.ui32DrcTargetLevel > 127)? 127: sAacSbrConfigParams.ui32DrcTargetLevel;

	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.eDownmixType, 4); aacSbrParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.ui32Arib_matrix_mixdown_index, 4); aacSbrParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacSbrConfigParams.ui32DrcGainControlCompress), 4); aacSbrParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacSbrConfigParams.ui32DrcGainControlBoost), 4); aacSbrParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, drcTargetLevel, 4); aacSbrParamsPtr+=4;

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		switch (hDspCh->sSettings.eOutputMode[i])
		{
			case BRAP_OutputMode_e1_0:
				OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				OutputMode = 7;
				break;
			default:
				break;
		}

		switch (hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode)
		{
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				dualMono = 1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				dualMono = 2; 
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				dualMono = 0;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				dualMono = 3;
				break;
			default:
				break;
		}

		sAacSbrConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sAacSbrConfigParam;

		BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.ui32bOutputLfeOn, 4); aacSbrParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, OutputMode, 4); aacSbrParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, dualMono, 4); aacSbrParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.ui32ExtDownmixEnable, 4); aacSbrParamsPtr+=4;

		for (row = 0; row < BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_ROWS; row++)
		{
			for (count = 0; count < BRAP_DSPCHN_P_AACCOEFF_DNMX_COEFF_COLUMNS; count++)
			{
				BRAP_DSPCHN_P_AddWordToFWBuffer(aacSbrParamsPtr, BRAP_DSPCHN_P_FloatToQ230(sAacSbrConfigParams.ui32ExtDnMixCoeffs[row][count]), 4); aacSbrParamsPtr+=4;
			}
		}
		
        BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
        for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
        {
            BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], aacSbrParamsPtr));
            BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, eSpeakerConfigMatrix[count], 4); aacSbrParamsPtr+=4;
        } 
		
	}
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.ui32PcmBoost6dB, 4); aacSbrParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer (aacSbrParamsPtr, sAacSbrConfigParams.bPcmBoostMinus4p75dB, 4); aacSbrParamsPtr+=4;	
}

static void 
BRAP_DSPCHN_P_FormAc3PlusConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *ac3PlusParamsPtr)
{
	unsigned int row, count;
	BRAP_DSPCHN_Ac3PlusConfigParams sAc3PlusConfigParams;
	uint32_t ui32OutputMode = 2;
	BRAP_DSPCHN_DualMonoMode	eDualMonoMode;
	unsigned int dualMono = 0;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;
         uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( ac3PlusParamsPtr );

	sAc3PlusConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3PlusConfigParam;

	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.eCompMode, 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3PlusConfigParams.ui16PcmScaleFactor), 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3PlusConfigParams.ui16DynRngScaleHi), 4); ac3PlusParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3PlusConfigParams.ui16DynRngScaleLo), 4); ac3PlusParamsPtr+=4;

	/* 7411 DVD Warning: This code won't work for 7411 DVD applications. 7411 DVD requires
	 * numDownmixPaths to be programmed in decoder configuration parameters. To make
	 * this code work on 7411 DVD, enable code to program numDownmixPaths for 7411D.
	 */
#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, numDownmixPaths, 4); ac3PlusParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		sAc3PlusConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sAc3PlusConfigParam;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;

		/* DDP dual mono values are not in sequence with BRAP_DSPCHN_DualMonoMode enum */
		switch (eDualMonoMode) {
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				dualMono = 1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				dualMono = 2; 
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				dualMono = 0;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				dualMono = 3;
				break;
			default:
				break;
		}
		
		/* Program AC3 Plus parameters in the buffer */
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.bOutputLfeOn, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, ui32OutputMode, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.eStereoMode, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, dualMono, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.eKaraokeVocalSelect, 4); ac3PlusParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.bUseUserDownmixCoeff, 4); ac3PlusParamsPtr+=4;
		for (row = 0; row < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3PlusConfigParams.i16UserDownmixCoeff[row][count]), 4); ac3PlusParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, sAc3PlusConfigParams.bUseKaraokeLevelPanCoeff, 4); ac3PlusParamsPtr+=4;
		for (count = 0; count < BRAP_DSPCHN_P_AC3COEFF_KARAOKE_COEFF; count++){
			BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sAc3PlusConfigParams.i16KaraokeLevelPanCoeff[count]), 4); ac3PlusParamsPtr+=4;
		}

                BDBG_MSG(("AC3 param: writing NumActiveSpeakers array to addr 0x%08x", ac3PlusParamsPtr));
                BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
				
                for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
                {
                    BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], ac3PlusParamsPtr));
                    BRAP_DSPCHN_P_AddWordToFWBuffer (ac3PlusParamsPtr, eSpeakerConfigMatrix[count], 4); ac3PlusParamsPtr+=4;
                }    

	}
	sAc3PlusConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sAc3PlusConfigParam;
	BRAP_DSPCHN_P_AddWordToFWBuffer(ac3PlusParamsPtr,sAc3PlusConfigParams.bDialNormEnable, 4); ac3PlusParamsPtr+=4;
	
}


static void 
BRAP_DSPCHN_P_FormWmaStdConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *wmaStdParamsPtr)
{
	unsigned int count=0;
	uint32_t ui32OutputMode = 2;
	bool  bDecodeOnlyPatternFlag=false;
	uint32_t  	ui32DecodePattern=0x00;	
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	BRAP_DSPCHN_WmaStdConfigParams sWmaStdConfigParams;

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( wmaStdParamsPtr );

#ifndef WMA_CERTIFICATION			
	bDecodeOnlyPatternFlag=false;
	ui32DecodePattern=0x00;	
#else
	bDecodeOnlyPatternFlag=true;
	ui32DecodePattern=0x33333333;	
#endif	


	sWmaStdConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sWmaStdConfigParams; 

	switch (hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain])
	{
		case BRAP_OutputMode_e1_0:
			ui32OutputMode = 1;
			break;
		case BRAP_OutputMode_e1_1:
			ui32OutputMode = 0;
			break;
		case BRAP_OutputMode_e2_0:
			ui32OutputMode = 2;
			break;
		case BRAP_OutputMode_e3_0:
			ui32OutputMode = 3;
			break;
		case BRAP_OutputMode_e2_1:
			ui32OutputMode = 4;
			break;
		case BRAP_OutputMode_e3_1:
			ui32OutputMode = 5;
			break;
		case BRAP_OutputMode_e2_2:
			ui32OutputMode = 6;
			break;
		case BRAP_OutputMode_e3_2:
			ui32OutputMode = 7;
			break;
		default:
			break;
	}


	BRAP_DSPCHN_P_AddWordToFWBuffer(wmaStdParamsPtr, ui32OutputMode, 4); wmaStdParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(wmaStdParamsPtr, sWmaStdConfigParams.eStereoMode, 4); wmaStdParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(wmaStdParamsPtr, bDecodeOnlyPatternFlag, 4); wmaStdParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(wmaStdParamsPtr, ui32DecodePattern, 4); wmaStdParamsPtr+=4;

	BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
	for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
	{
		BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], wmaStdParamsPtr)); 
		BRAP_DSPCHN_P_AddWordToFWBuffer (wmaStdParamsPtr, eSpeakerConfigMatrix[count], 4); wmaStdParamsPtr+=4;
	}
}

static void 
BRAP_DSPCHN_P_FormPcmWavConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *pcmWavParamsPtr)
{
	unsigned int count=0;
	uint32_t ui32OutputMode = 2;
    uint32_t ui32NumOutputPort=0;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	BRAP_DSPCHN_PcmWavConfigParams sPcmWavConfigParams;

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( pcmWavParamsPtr );

	sPcmWavConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sPcmWavConfigParams; 
    ui32NumOutputPort = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
    
	switch (hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain])
	{
		case BRAP_OutputMode_e1_0:
			ui32OutputMode = 1;
			break;
		case BRAP_OutputMode_e1_1:
			ui32OutputMode = 0;
			break;
		case BRAP_OutputMode_e2_0:
			ui32OutputMode = 2;
			break;
		case BRAP_OutputMode_e3_0:
			ui32OutputMode = 3;
			break;
		case BRAP_OutputMode_e2_1:
			ui32OutputMode = 4;
			break;
		case BRAP_OutputMode_e3_1:
			ui32OutputMode = 5;
			break;
		case BRAP_OutputMode_e2_2:
			ui32OutputMode = 6;
			break;
		case BRAP_OutputMode_e3_2:
			ui32OutputMode = 7;
			break;
		default:
			break;
	}

	BRAP_DSPCHN_P_AddWordToFWBuffer(pcmWavParamsPtr, ui32NumOutputPort, 4); pcmWavParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(pcmWavParamsPtr, ui32OutputMode, 4); pcmWavParamsPtr+=4;

	BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
	for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
	{
		BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], pcmWavParamsPtr)); 
		BRAP_DSPCHN_P_AddWordToFWBuffer (pcmWavParamsPtr, eSpeakerConfigMatrix[count], 4); pcmWavParamsPtr+=4;
	}
}
static void 
BRAP_DSPCHN_P_FormDraConfigStruct (
				BRAP_DSPCHN_Handle	hDspCh,
				unsigned char *draParamsPtr)
{
	BRAP_DSPCHN_DraConfigParams sDraConfigParams;

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( draParamsPtr );

	sDraConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sDraConfigParams; 

	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32EnableDownmix, 4); draParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32nOutmode, 4); draParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32nBitPCM, 4); draParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32nOutPair, 4); draParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32nCertDummyPatternEnable, 4); draParamsPtr+=4;
	BRAP_DSPCHN_P_AddWordToFWBuffer(draParamsPtr, sDraConfigParams.ui32nDummyPattern, 4); draParamsPtr+=4;

}



static void 
BRAP_DSPCHN_P_FormDtshdConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *dtshdParamsPtr)
{
	uint32_t ui32OutputMode = 2;
	BRAP_DSPCHN_DualMonoMode eDualMonoMode;
	BRAP_DSPCHN_DtshdConfigParams sDtshdConfigParams;
	unsigned int row, count;
	int i = 0, numDownmixPaths =1, maxDnMixPaths = 0;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (dtshdParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(dtshdParamsPtr, numDownmixPaths, 4); dtshdParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++ )
	{
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;
		sDtshdConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sDtshdConfigParam;

		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer (dtshdParamsPtr, ui32OutputMode, 4); dtshdParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtshdParamsPtr, sDtshdConfigParams.bOutputLfeOn, 4); dtshdParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtshdParamsPtr, eDualMonoMode, 4); dtshdParamsPtr+=4;
		/* TODO: check if some re-mapping is required */
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtshdParamsPtr, sDtshdConfigParams.eStereoMode, 4); dtshdParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtshdParamsPtr, sDtshdConfigParams.bUseUserDownmixCoeff, 4); dtshdParamsPtr+=4;
		/*TODO: Check if some format conversion is reqd. for coeff */
		for (row = 0; row < BRAP_DSPCHN_P_DTSHDCOEFF_DNMX_COEFF_ROWS; row++)
	    {   
			for (count = 0; count < BRAP_DSPCHN_P_DTSHDCOEFF_DNMX_COEFF_COLUMNS; count++)
	        {
				BRAP_DSPCHN_P_AddWordToFWBuffer(dtshdParamsPtr, 
	                    BRAP_DSPCHN_P_FloatToQ131(sDtshdConfigParams.i32UserDownmixCoeff[row][count]), 
	                    4); 
	            dtshdParamsPtr+=4;
		    }
	    }
	}
}

#if (BRAP_DTS_SUPPORTED == 1)		

static void 
BRAP_DSPCHN_P_FormDtsCoreConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *dtsParamsPtr)
{

	BRAP_DSPCHN_DtsCoreConfigParams sDtsCoreConfigParams;
	int i = 0, maxDnMixPaths = 0;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (dtsParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
#else
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++ )
	{
		sDtsCoreConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sDtsCoreConfigParam;


		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32UserDRC_flag, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32DynScaleHigh, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32DynScaleLow, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32NumOutputport, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32UserAMODE, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.bOutputLfeOn, 4); dtsParamsPtr+=4;        
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.eStereoMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32DualMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32Use_LFE_in_DMX, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32OutPair, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32UsePatternInDummy, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsCoreConfigParams.i32BitPatternInDummy, 4); dtsParamsPtr+=4;
	}
}

#else

static void 
BRAP_DSPCHN_P_FormDtsConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *dtsParamsPtr)
{
	uint32_t ui32OutputMode = 2;
	BRAP_DSPCHN_DualMonoMode eDualMonoMode;
	BRAP_DSPCHN_DtsConfigParams sDtsConfigParams;
	unsigned int row, count;
	int i = 0, maxDnMixPaths = 0;

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	int numDownmixPaths = 1;
#endif

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (dtsParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, numDownmixPaths, 4); dtsParamsPtr+=4;
#else
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++ )
	{
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;
		sDtsConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sDtsConfigParam;

		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, ui32OutputMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsConfigParams.bOutputLfeOn, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, eDualMonoMode, 4); dtsParamsPtr+=4;
		/* TODO: check if some re-mapping is required */
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsConfigParams.eStereoMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sDtsConfigParams.bUseUserDownmixCoeff, 4); dtsParamsPtr+=4;
		/*TODO: Check if some format conversion is reqd. for coeff */
		for (row = 0; row < BRAP_DSPCHN_P_DTSCOEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_DTSCOEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sDtsConfigParams.i32UserDownmixCoeff[row][count]), 4); dtsParamsPtr+=4;
		}
	}
}

#endif

static void 
BRAP_DSPCHN_P_FormBdLpcmConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *dtsParamsPtr)
{
	BRAP_DSPCHN_BdlpcmConfigParams sBdLpcmConfigParams;
	uint32_t ui32OutputMode = 2;
	unsigned int row, count;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths=0;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (dtsParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, numDownmixPaths, 4); dtsParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++ )
	{
		sBdLpcmConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sBdlpcmConfigParam;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sBdLpcmConfigParams.bUseUserDownmixCoeff, 4); dtsParamsPtr+=4;
		/*TODO: Check if some format conversion is reqd. for coeff */
		for (row = 0; row < BRAP_DSPCHN_P_BDLPCMCOEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_BDLPCMCOEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sBdLpcmConfigParams.i32UserDownmixCoeff[row][count]), 4); dtsParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, ui32OutputMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sBdLpcmConfigParams.bOutputLfeOn, 4); dtsParamsPtr+=4;
	}
}


static void 
BRAP_DSPCHN_P_FormLpcmHdDvdConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *dtsParamsPtr)
{
	BRAP_DSPCHN_LpcmHdDvdConfigParams sLpcmHdDvdConfigParams;
	uint32_t ui32OutputMode = 2;
	unsigned int row, count;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths=0;

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (dtsParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 )
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, numDownmixPaths, 4); dtsParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		sLpcmHdDvdConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sLpcmHdDvdConfigParam;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sLpcmHdDvdConfigParams.bUseUserDownmixCoeff, 4); dtsParamsPtr+=4;
		/*TODO: Check if some format conversion is reqd. for coeff */
		for (row = 0; row < BRAP_DSPCHN_P_LPCMHDDVDCOEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_LPCMHDDVDCOEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(dtsParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sLpcmHdDvdConfigParams.i32UserDownmixCoeff[row][count]), 4); dtsParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, ui32OutputMode, 4); dtsParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (dtsParamsPtr, sLpcmHdDvdConfigParams.bOutputLfeOn, 4); dtsParamsPtr+=4;
	}
}

static void 
BRAP_DSPCHN_P_FormLpcmDvdConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *lpcmDvdParamsPtr)
{
	BRAP_DSPCHN_LpcmDvdConfigParams sLpcmDvdConfigParams;
	uint32_t ui32OutputMode = 2;
	uint32_t	uiDualMonoMode;
	BRAP_DSPCHN_DualMonoMode eDualMonoMode;
	unsigned int row, count;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths=0;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];	

	BDBG_ASSERT (hDspCh);
	BDBG_ASSERT (lpcmDvdParamsPtr);

#if ( BCHP_7411_VER > BCHP_VER_C0 ) || ( BCHP_CHIP == 7400 ) ||(BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
    maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(lpcmDvdParamsPtr, numDownmixPaths, 4); lpcmDvdParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		eDualMonoMode = hDspCh->sSettings.sDspchnExtSettings[i].eDualMonoMode;
		sLpcmDvdConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sLpcmDvdConfigParam;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				ui32OutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				ui32OutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				ui32OutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				ui32OutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				ui32OutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				ui32OutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				ui32OutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				ui32OutputMode = 7;
				break;
			default:
				break;
		}

		BRAP_DSPCHN_P_AddWordToFWBuffer (lpcmDvdParamsPtr, sLpcmDvdConfigParams.bUseUserDownmixCoeff, 4); lpcmDvdParamsPtr+=4;
		/*TODO: Check if some format conversion is reqd. for coeff */
		for (row = 0; row < BRAP_DSPCHN_P_LPCMDVDCOEFF_DNMX_COEFF_ROWS; row++)
			for (count = 0; count < BRAP_DSPCHN_P_LPCMDVDCOEFF_DNMX_COEFF_COLUMNS; count++){
				BRAP_DSPCHN_P_AddWordToFWBuffer(lpcmDvdParamsPtr, BRAP_DSPCHN_P_FloatToQ131(sLpcmDvdConfigParams.i32UserDownmixCoeff[row][count]), 4); lpcmDvdParamsPtr+=4;
		}
		BRAP_DSPCHN_P_AddWordToFWBuffer (lpcmDvdParamsPtr, ui32OutputMode, 4); lpcmDvdParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer (lpcmDvdParamsPtr, sLpcmDvdConfigParams.bOutputLfeOn, 4); lpcmDvdParamsPtr+=4;

		/* LPCM DVD dual mono values are not in sequence with BRAP_DSPCHN_DualMonoMode enum */
		switch(eDualMonoMode)
		{
			case BRAP_DSPCHN_DualMonoMode_eLeftMono:
				uiDualMonoMode=1;
				break;
			case BRAP_DSPCHN_DualMonoMode_eRightMono:
				uiDualMonoMode=2;
				break;
			case BRAP_DSPCHN_DualMonoMode_eStereo:
				uiDualMonoMode=3;
				break;
			case BRAP_DSPCHN_DualMonoMode_eDualMonoMix:
				uiDualMonoMode=0;
				break;
			default:
				uiDualMonoMode=3;
				break;

		}
		BRAP_DSPCHN_P_AddWordToFWBuffer (lpcmDvdParamsPtr, uiDualMonoMode, 4); lpcmDvdParamsPtr+=4;		

		BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
		for (count=0; count<=BRAP_DSPCHN_P_SpeakerConfig_eLFE; count++)
		{
			BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], lpcmDvdParamsPtr));
			BRAP_DSPCHN_P_AddWordToFWBuffer (lpcmDvdParamsPtr, eSpeakerConfigMatrix[count], 4); lpcmDvdParamsPtr+=4;
		} 
	}
}

static void
BRAP_DSPCHN_P_FormMlpConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *mlpParamsPtr )
{
	BRAP_DSPCHN_MlpConfigParams	sMlpConfigParams;
	uint32_t ui32OutputMode = 0;

	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( mlpParamsPtr );

	sMlpConfigParams = hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sMlpConfigParams;
	switch (hDspCh->sSettings.eOutputMode[BRAP_DEC_DownmixPath_eMain]) {
		case BRAP_OutputMode_e1_0:
			ui32OutputMode = 1;
			break;
		case BRAP_OutputMode_e1_1:
			ui32OutputMode = 0;
			break;
		case BRAP_OutputMode_e2_0:
			ui32OutputMode = 2;
			break;
		case BRAP_OutputMode_e3_0:
			ui32OutputMode = 3;
			break;
		case BRAP_OutputMode_e2_1:
			ui32OutputMode = 4;
			break;
		case BRAP_OutputMode_e3_1:
			ui32OutputMode = 5;
			break;
		case BRAP_OutputMode_e2_2:
			ui32OutputMode = 6;
			break;
		case BRAP_OutputMode_e3_2:
			ui32OutputMode = 7;
			break;
		default:
			break;
	}

	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.eProfileSelect, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.eDrc, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.ui32DrcBoost, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.ui32DrcCut, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.bDialogueEnable, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.i32DialogueLevel, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, ui32OutputMode, 4 ); mlpParamsPtr += 4;
	BRAP_DSPCHN_P_AddWordToFWBuffer( mlpParamsPtr, sMlpConfigParams.bOutputLfeOn, 4 ); mlpParamsPtr += 4;
}

static void
BRAP_DSPCHN_P_FormWmaProConfigStruct (
				BRAP_DSPCHN_Handle hDspCh,
				unsigned char *wmaProParamsPtr )
{
	BRAP_DSPCHN_WmaProConfigParams	sWmaProConfigParams;
	unsigned int eOutputMode = 0;
	int i = 0, numDownmixPaths = 1, maxDnMixPaths = 0;
	unsigned int count=0;
	uint32_t eSpeakerConfigMatrix[BRAP_DSPCHN_P_SpeakerConfig_eMax];

	
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( wmaProParamsPtr );

#if (BCHP_CHIP == 7400) || (BCHP_CHIP == 7401 && BCHP_VER > BCHP_VER_A0) || (BCHP_CHIP == 7118) || (BCHP_CHIP == 7403)
	numDownmixPaths = hDspCh->sDspAudioParams.sExtAudioParams.bEnableStereoDownmixPath ? 2 : 1;
	maxDnMixPaths = BRAP_DSPCHN_P_TWO_DNMIXED_OUTPUT;
	BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, numDownmixPaths, 4); wmaProParamsPtr+=4;
#else
    numDownmixPaths = 1;
    maxDnMixPaths = BRAP_DSPCHN_P_ONE_DNMIXED_OUTPUT;
#endif

	for ( i = 0; i < maxDnMixPaths; i++)
	{
		sWmaProConfigParams = hDspCh->sSettings.sDspchnCfgParams[i].sWmaProConfigParams;
		switch (hDspCh->sSettings.eOutputMode[i]) {
			case BRAP_OutputMode_e1_0:
				eOutputMode = 1;
				break;
			case BRAP_OutputMode_e1_1:
				eOutputMode = 0;
				break;
			case BRAP_OutputMode_e2_0:
				eOutputMode = 2;
				break;
			case BRAP_OutputMode_e3_0:
				eOutputMode = 3;
				break;
			case BRAP_OutputMode_e2_1:
				eOutputMode = 4;
				break;
			case BRAP_OutputMode_e3_1:
				eOutputMode = 5;
				break;
			case BRAP_OutputMode_e2_2:
				eOutputMode = 6;
				break;
			case BRAP_OutputMode_e3_2:
				eOutputMode = 7;
				break;
			default:
				break;
		}
		
		/* Program WMA-Pro parameters in the buffer */
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, sWmaProConfigParams.bDrcFlag, 4); wmaProParamsPtr+=4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, sWmaProConfigParams.ui32DrcBoost, 4 ); wmaProParamsPtr += 4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, sWmaProConfigParams.ui32DrcCut, 4 ); wmaProParamsPtr += 4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, eOutputMode, 4 ); wmaProParamsPtr += 4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, sWmaProConfigParams.bLfeOn, 4 ); wmaProParamsPtr += 4;
		BRAP_DSPCHN_P_AddWordToFWBuffer(wmaProParamsPtr, sWmaProConfigParams.eStereoMode, 4 ); wmaProParamsPtr += 4;

		BRAP_DSPCHN_P_FormSpeakersMatrix(hDspCh, &eSpeakerConfigMatrix[0],BRAP_DEC_DownmixPath_eMain);
		for (count=0; count<BRAP_DSPCHN_P_SpeakerConfig_eMax; count++)
		{
			BDBG_MSG(("eSpeakerConfigMatrix[%d]=%d, addr=0x%x", count, eSpeakerConfigMatrix[count], wmaProParamsPtr)); 
			BRAP_DSPCHN_P_AddWordToFWBuffer (wmaProParamsPtr, eSpeakerConfigMatrix[count], 4); wmaProParamsPtr+=4;
		}
		
	}

}

#if (BRAP_AD_SUPPORTED == 1)
static void
BRAP_DSPCHN_P_FormFadeConfigStruct(	
	BRAP_DSPCHN_Handle hDspCh,
	unsigned char *fadeParamsPtr,
	bool			bUseDefault,
	unsigned int	uiPpBranchId,
	unsigned int	uiPpStgId
	)
{
	BRAP_DSPCHN_ADFadeCtrlConfigParams	sFadeParams;
	
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( fadeParamsPtr );

	if( bUseDefault )
	{
		sFadeParams = hDspCh->sSettings.sPPSettings.sADFadeCtrlConfigParams;
	}
	else
	{
		sFadeParams = hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStgId].sAdFadeConfigParams;
	}

    BDBG_MSG(("sFadeParams.eFadeConfig = %d",sFadeParams.eFadeConfig));
    BDBG_MSG(("sFadeParams.ui32PanFadeInterfaceAddr = %d",sFadeParams.ui32PanFadeInterfaceAddr));
	
	BRAP_DSPCHN_P_AddWordToFWBuffer( fadeParamsPtr, sFadeParams.eFadeConfig, 4 ); fadeParamsPtr+=4; 	
	BRAP_DSPCHN_P_AddWordToFWBuffer( fadeParamsPtr, sFadeParams.ui32PanFadeInterfaceAddr, 4 ); fadeParamsPtr+=4; 
}

static void
BRAP_DSPCHN_P_FormPanConfigStruct(	
	BRAP_DSPCHN_Handle hDspCh,
	unsigned char *panParamsPtr,
	bool			bUseDefault,
	unsigned int	uiPpBranchId,
	unsigned int	uiPpStgId
	)
{
	BRAP_DSPCHN_ADPanCtrlConfigParams	sPanParams;
	
	BDBG_ASSERT( hDspCh );
	BDBG_ASSERT( panParamsPtr );


	if( bUseDefault )
	{
		sPanParams = hDspCh->sSettings.sPPSettings.sADPanCtrlConfigParams;
	}
	else
	{
		sPanParams = hDspCh->uPpStgConfigParam[uiPpBranchId][uiPpStgId].sAdPanConfigParams;
	}

    BDBG_MSG(("sPanParams.ui32UserVolumeLevel = %x",sPanParams.ui32UserVolumeLevel));
    BDBG_MSG(("sPanParams.eADChannelConfig = %d",sPanParams.eADChannelConfig));
    BDBG_MSG(("sPanParams.ui32PanFadeInterfaceAddr = %d",sPanParams.ui32PanFadeInterfaceAddr));

    
	BRAP_DSPCHN_P_AddWordToFWBuffer( panParamsPtr, sPanParams.ui32UserVolumeLevel, 4 ); panParamsPtr+=4; 	
	BRAP_DSPCHN_P_AddWordToFWBuffer( panParamsPtr, sPanParams.eADChannelConfig , 4 ); panParamsPtr+=4; 
	BRAP_DSPCHN_P_AddWordToFWBuffer( panParamsPtr, sPanParams.ui32PanFadeInterfaceAddr, 4 ); panParamsPtr+=4;
}

#endif

BERR_Code BRAP_DSPCHN_P_PrgmFmmDstCxt (
            BRAP_DSPCHN_Handle hDspChn,
            BRAP_DSPCHN_P_OpPortParams * pOpPortParams 
)
{


    unsigned int i;
    uint32_t ui32RegVal = 0, ui32RegAddr = 0;
    BERR_Code ret = BERR_SUCCESS;
    
    BDBG_ENTER(BRAP_DSPCHN_P_PrgmFmmDstCxt);
	
    /* Validate input params */
    BDBG_ASSERT(hDspChn);
    BDBG_ASSERT(pOpPortParams);
    
    if (hDspChn->sDspAudioParams.sExtAudioParams.eDecodeMode ==BRAP_DSPCHN_DecodeMode_eSimulMode) 
    {
            ui32RegAddr = BCHP_AUD_DSP_CFG0_FMM_DEST_CXT2;
    }
    else
    {
            ui32RegAddr = BCHP_AUD_DSP_CFG0_FMM_DEST_CXT0 + hDspChn->chCfgRegOffset;
    }    
    
    ui32RegVal = BRAP_Read32(hDspChn->hRegister, ui32RegAddr);
    
    for (i = 0; i < pOpPortParams->uiNumOpPorts; i++) 
    {
          switch (pOpPortParams->sOpPortConfig[i].eOutputPortType) 
          {
			case BRAP_OutputPort_eSpdif:
				ui32RegVal |= BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, SPDIF0, Enabled);
				break;
			case BRAP_OutputPort_eDac0: 
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, DAC0, Enabled);
				break;
			case BRAP_OutputPort_eDac1:
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, DAC1, Enabled);
				break;
			case BRAP_OutputPort_eFlex:
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, SPDIF1, Enabled);
				break;
			default:
				break;
	    }

	    switch (pOpPortParams->sOpPortConfig[i].ePll) 
           {
			case BRAP_OP_Pll_ePll0:
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL0, Enabled);
				break;
			case BRAP_OP_Pll_ePll1:
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL1, Enabled);
				break;
			case BRAP_OP_Pll_ePll2:
				ui32RegVal |=BCHP_FIELD_ENUM(AUD_DSP_CFG0_FMM_DEST_CXT0, PLL2, Enabled);
				break;
			default:
				break;
	    }
	    ;
	}
    
       BRAP_Write32(hDspChn->hRegister, ui32RegAddr, ui32RegVal);

       BDBG_LEAVE(BRAP_DSPCHN_P_PrgmFmmDstCxt);	
       return ret;
}

unsigned int
BRAP_DSPCHN_P_SampleRateEnumToValue(BAVC_AudioSamplingRate eSampleRate)
{
	unsigned int sampleRate;
	
	switch (eSampleRate) {
		case BAVC_AudioSamplingRate_e32k:
			sampleRate = 32000;
			break;
		case BAVC_AudioSamplingRate_e44_1k:
			sampleRate = 44100;
			break;
		case BAVC_AudioSamplingRate_e48k:
			sampleRate = 48000;
			break;
		case BAVC_AudioSamplingRate_e96k:
			sampleRate = 96000;
			break;
		case BAVC_AudioSamplingRate_e16k:
			sampleRate = 16000;
			break;
		case BAVC_AudioSamplingRate_e22_05k:
			sampleRate = 22050;
			break;
		case BAVC_AudioSamplingRate_e24k:
			sampleRate = 24000;
			break;
		case BAVC_AudioSamplingRate_e64k:
			sampleRate = 64000;
			break;
		case BAVC_AudioSamplingRate_e88_2k:
			sampleRate = 88200;
			break;
		case BAVC_AudioSamplingRate_e128k:
			sampleRate = 128000;
			break;
		case BAVC_AudioSamplingRate_e176_4k:
			sampleRate = 176400;
			break;
		case BAVC_AudioSamplingRate_e192k:
			sampleRate = 192000;
			break;
		default:
			sampleRate = 48000;
 	}

	return sampleRate;

}

#if BCHP_7411_VER > BCHP_VER_C0
BERR_Code
BRAP_DSPCHN_P_SetDdbmConfigurationParams (BRAP_DSPCHN_Handle hDspCh)
{
	uint32_t regVal = 0;

	/* Program DDBM Config Mode */
	regVal = BRAP_Read32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + hDspCh->chCfgRegOffset);

	switch (hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sDdbmConfigParams.eConfigMode) {
		case BRAP_DSPCHN_DdbmMode_eConfig1:
			regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0, AC3_DDBM_MODE)))
					| BCHP_FIELD_ENUM(AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0, AC3_DDBM_MODE,
										CONFIG1);
			break;
		case BRAP_DSPCHN_DdbmMode_eConfig2:
			regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0, AC3_DDBM_MODE)))
					| BCHP_FIELD_ENUM(AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0, AC3_DDBM_MODE,
										CONFIG2);
			break;
		default:
			BDBG_ERR(("Invalid DDBM Mode %d",  hDspCh->sSettings.sDspchnCfgParams[BRAP_DEC_DownmixPath_eMain].sDdbmConfigParams.eConfigMode));
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BRAP_Write32(hDspCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER2_CXT0 + hDspCh->chCfgRegOffset, regVal);

	return BERR_SUCCESS;
}
#endif

#if BCHP_7411_VER > BCHP_VER_C0
/* This is a temporary hack. This should be removed. */
void 
BRAP_DSPCHN_P_ClearStcValidFlag(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool				bIsrCxt		/* [in] true = ISR context, false = Task context */
	)
{
	uint32_t regVal;

  BDBG_ASSERT(NULL != hRapCh);

	regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,
			BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);

	regVal = (regVal & (~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, STC_VALID)));
	
	BRAP_DSP_Write32((BARC_Handle)hRapCh->hRegister, BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0
						+ hRapCh->sModuleHandles.hDspCh->chCfgRegOffset, regVal, bIsrCxt);
  return;
}

/***************************************************************************
Summary: BRAP_DSPCHN_ClearStcValidFlag

Description:
	This function clears the "STC Valid" flag in firmware.
	
Returns:
	BERR_SUCCESS - If successful

See Also:
**************************************************************************/
BERR_Code
BRAP_DSPCHN_ClearStcValidFlag_isr(
	BRAP_ChannelHandle	hRapCh			/* [in] Audio channel handle */
	)
{
	BDBG_ENTER(BRAP_DSPCHN_ClearStcValidFlag_isr);

	BRAP_DSPCHN_P_ClearStcValidFlag(hRapCh, true);

	BDBG_ENTER(BRAP_DSPCHN_ClearStcValidFlag_isr);
	return BERR_SUCCESS;
}
#endif 

#if (BRAP_DSP_P_7401_NEWMIT==1)
BERR_Code BRAP_DSPCHN_P_AllocateIsIsIfBuffers( BRAP_DSPCHN_Handle hDspCh )
{
	unsigned int uiPpBrIndex = 0, uiPpStgIndex = 0;
	unsigned int uiChildForkPts[BRAP_P_MAX_PP_SUPPORTED];
	unsigned int i = 0, j = 0, parentBr = 0, forkPt = 0;
	bool bStgJustAfterFork = false, bLastStg = false;
	BERR_Code err = BERR_SUCCESS;

	
	/* First, allocate interstage and interstage interface buffers for decode stage */
	BRAP_DSP_P_AllocateIsIsIfBuffers( hDspCh->hDsp,
		&(hDspCh->uiDecInterstageOutputBuf));
	
	BDBG_MSG(("uiDecInterstageOutputBuf = %d", hDspCh->uiDecInterstageOutputBuf));

	for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
	{
	
		/* Initialize elements of uiChildForkPts  to invalid values */
		for( i = 0; i < BRAP_P_MAX_PP_SUPPORTED; i++ )
		{
			uiChildForkPts[i] = BRAP_INVALID_VALUE;
		}
		
		/* Find out all the forking points on this branch */
		for( i = 0, j = 0; i < BRAP_P_MAX_BRANCH_SUPPORTED; i++ )
		{
			if( uiPpBrIndex==hDspCh->sPpBranchInfo[i].uiParentId )
			{
				uiChildForkPts[j] = hDspCh->sPpBranchInfo[i].uiForkPt;
				BDBG_MSG(("uiPpBrIndex = %d, uiChildForkPts[%d] = %d", uiPpBrIndex, j, uiChildForkPts[j]));
				j++;
			}
		}
		
		for( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
		{

			BDBG_MSG(("PpBranchInfo[%d].sPpStgSettings.ePpAlgo[%d] = %d",
				uiPpBrIndex, uiPpStgIndex,
				hDspCh->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex]));

			/* Allocate input buffers for this stage */
			if( (uiPpBrIndex==0) && (uiPpStgIndex==0) ) 
			{
				/* Decode stage */
				hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex] = hDspCh->uiDecInterstageOutputBuf;
			}
			else if( uiPpStgIndex==0 ) 
			{
				/* First stage in a branch */
				parentBr = hDspCh->sPpBranchInfo[uiPpBrIndex].uiParentId;
				forkPt = hDspCh->sPpBranchInfo[uiPpBrIndex].uiForkPt;

				hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex] = 
					hDspCh->uiInterstageInputBuf[parentBr][forkPt];
			}
			else
			{
				hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex] = 
					hDspCh->uiInterstageOutputBuf[uiPpBrIndex][uiPpStgIndex-1];
			}

			/* Check if this stage is just after a forking point on this branch */
			bStgJustAfterFork = false;
			for( i = 0; i < BRAP_P_MAX_PP_SUPPORTED; i++ )
			{
				if( uiPpStgIndex==uiChildForkPts[i] )
					bStgJustAfterFork = true;
			}

			/* Check if this is the last stage on this branch */
			bLastStg = false;

			if( BRAP_DSPCHN_PP_Algo_eMax==hDspCh->sPpBranchInfo[uiPpBrIndex].sPpStgSettings.ePpAlgo[uiPpStgIndex] )          
               bLastStg = true;

			
			/* Allocate output buffers for this stage */
			if( true==bLastStg )
			{
				/* Last stage in the branch. Don't need output buffers.  */
				hDspCh->uiInterstageOutputBuf[uiPpBrIndex][uiPpStgIndex] = BRAP_INVALID_VALUE;
			}
			else if(( true==bStgJustAfterFork ) || ( 0==uiPpStgIndex ))
			{
				/* Decode stage */
				BRAP_DSP_P_AllocateIsIsIfBuffers( hDspCh->hDsp,
					&(hDspCh->uiInterstageOutputBuf[uiPpBrIndex][uiPpStgIndex]));
			}
			else
			{
				/* Input buffer of this stage can be used as output buffer */
				hDspCh->uiInterstageOutputBuf[uiPpBrIndex][uiPpStgIndex] =
					hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex];
			}

			BDBG_MSG(("uiInterstageInputBuf[%d][%d] = %d, uiInterstageOutputBuf[%d][%d] = %d",
				uiPpBrIndex, uiPpStgIndex, hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex],
				uiPpBrIndex, uiPpStgIndex, hDspCh->uiInterstageOutputBuf[uiPpBrIndex][uiPpStgIndex]));

		}

	}

	return err;
	
}

void	BRAP_DSPCHN_P_FreeIsIsIfBuffers( BRAP_DSPCHN_Handle hDspCh )
{
	unsigned int uiPpBrIndex = 0, uiPpStgIndex = 0;

	for( uiPpBrIndex = 0; uiPpBrIndex < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBrIndex++ )
	{

		for( uiPpStgIndex = 0; uiPpStgIndex < BRAP_P_MAX_PP_SUPPORTED; uiPpStgIndex++ )
		{
			if( hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex] != BRAP_INVALID_VALUE )
				BRAP_DSP_P_FreeIsIsIfBuffers( hDspCh->hDsp,
					hDspCh->uiInterstageInputBuf[uiPpBrIndex][uiPpStgIndex] );
		}
	}
}

static void BRAP_DSPCHN_P_InitPp(BRAP_DSPCHN_Handle hDspChn)
{
	unsigned int uiPpBranch = 0, uiPpStg = 0;

	for( uiPpBranch = 0; uiPpBranch < BRAP_P_MAX_BRANCH_SUPPORTED; uiPpBranch++ )
	{
		hDspChn->sPpBranchInfo[uiPpBranch].uiParentId = BRAP_INVALID_VALUE;
		hDspChn->sPpBranchInfo[uiPpBranch].uiForkPt = BRAP_INVALID_VALUE;

        hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]= BRAP_DSPCHN_PP_Algo_eSrc;
		hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
        uiPpStg ++;
		
#if (BRAP_AD_SUPPORTED == 1)            
            hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]= BRAP_DSPCHN_PP_Algo_eAD_PanCtrl;
			hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
            uiPpStg ++;

            hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]= BRAP_DSPCHN_PP_Algo_eAD_FadeCtrl;
			hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
            uiPpStg ++;
#endif
#if (BRAP_DOLBYVOLUME_SUPPORTED == 1)
            hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]= BRAP_DSPCHN_PP_Algo_eDolbyVolume;
			hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
            uiPpStg ++;
#endif
#if (BRAP_SRS_TRUVOL_SUPPORTED == 1)
            hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg]= BRAP_DSPCHN_PP_Algo_eSRS_TruVol;
			hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
            uiPpStg ++;
#endif	
			hDspChn->sPpBranchInfo[uiPpBranch].sPpStgSettings.ePpAlgo[uiPpStg] = BRAP_DSPCHN_PP_Algo_eMax;
			hDspChn->bUseDefaultConfig[uiPpBranch][uiPpStg] = true; 
	}
}
#endif

BERR_Code
BRAP_DSPCHN_P_GetPts2StcPhase_isr(
	BRAP_ChannelHandle	hRapCh,	
	int32_t				*pi32Pts2StcPhase,
	bool					bIsrCxt
	)
{
	
	*pi32Pts2StcPhase = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister,  
				BCHP_AUD_DSP_CFG0_PTS2STC_PHASE_CXT0 + hRapCh->sModuleHandles.hDspCh->chCfgRegOffset,bIsrCxt);
	return BERR_SUCCESS;
}


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
	)
{
	BERR_Code     eStatus = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DSPCHN_GetPts2StcPhase);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		BKNI_EnterCriticalSection();
		eStatus = BRAP_DSPCHN_P_GetPts2StcPhase_isr(hRapCh, pi32Pts2StcPhase,false);
		BKNI_LeaveCriticalSection();
	}
	else
	{
        	BDBG_ERR(("Can not get PTS2STC phase when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	

	}
	
	BDBG_LEAVE (BRAP_DSPCHN_GetPts2StcPhase);

	return eStatus;
}

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
	)
{
	BERR_Code     eStatus = BERR_SUCCESS;

	BDBG_ENTER (BRAP_DSPCHN_GetPts2StcPhase_isr);

	if ( hRapCh->sModuleHandles.hDspCh->eChState == BRAP_DSPCHN_P_ChState_eStart )
	{
		eStatus = BRAP_DSPCHN_P_GetPts2StcPhase_isr(hRapCh, pi32Pts2StcPhase,true);
	}
	else
	{
        	BDBG_ERR(("Can not get PTS2STC phase in isr when the channel is not in running state"
					"This channel [hRapCh=0x%8x] is currently stopped.", hRapCh));
		return BERR_TRACE(BERR_NOT_SUPPORTED);	

	}

	BDBG_LEAVE (BRAP_DSPCHN_GetPts2StcPhase_isr);

	return eStatus;
}

BERR_Code 
BRAP_DSPCHN_P_InitializeHandle(
	BRAP_DSPCHN_Handle		hDspChn, /* [In] DSP channel handle */
	uint32_t				channelIndex	/* [in] Channel index starting from 0 to max number of channels */ 
	)
{
	BERR_Code     eStatus = BERR_SUCCESS;
	BDBG_ENTER (BRAP_DSPCHN_P_InitializeHandle);

	BDBG_ASSERT(hDspChn);

	hDspChn->channelIndex = channelIndex;
	hDspChn->chCfgRegOffset = (BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT1 - BCHP_AUD_DSP_CFG0_RAVE_TIMEBASE_SEL_CXT0)
								* channelIndex;
	hDspChn->chEsrSiRegOffset = (BCHP_AUD_DSP_ESR_SI10_INT_STATUS - BCHP_AUD_DSP_ESR_SI00_INT_STATUS)
								* channelIndex;
	
	hDspChn->hDsp->phAudChannelHandle[channelIndex] = hDspChn;

	BDBG_LEAVE (BRAP_DSPCHN_P_InitializeHandle);
	return eStatus;
}

#if (BRAP_DSPCHN_P_HAS_TSM_ERROR_RECOVERY==1)
BERR_Code
BRAP_DSPCHN_P_EnableTSMErrorRecovery_isr(
	BRAP_ChannelHandle	hRapCh,			/* [in] Audio channel handle */
	bool			bEnableTSMErrRecovery	/* [in] true = enable TSM Error Recovery,
							        		false = disable TSM Error Recovery  */
	)
{
	uint32_t regVal = 0, regAddr;

	BDBG_ENTER(BRAP_DSPCHN_P_EnableTSMErrorRecovery_isr);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

	hRapCh->sModuleHandles.hDspCh->sSettings.bEnableTSMErrRecovery =
		bEnableTSMErrRecovery;

	regAddr = BCHP_AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0 + 
				hRapCh->sModuleHandles.hDspCh->chCfgRegOffset; 
	regVal = BRAP_DSP_Read32((BARC_Handle)hRapCh->hRegister, regAddr, true);
	
	if (bEnableTSMErrRecovery) 
		regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY))
			| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY, 0x1);
	else
		regVal = (regVal & ~BCHP_MASK(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY))
			| BCHP_FIELD_DATA(AUD_DSP_CFG0_CONTROL_REGISTER4_CXT0, TSM_ERROR_RECOVERY, 0x0);

	BRAP_DSP_Write32((BARC_Handle)hRapCh->hRegister, regAddr, regVal, true);

	BDBG_LEAVE(BRAP_DSPCHN_P_EnableTSMErrorRecovery_isr);
	return BERR_SUCCESS;

}

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
	)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BRAP_DSPCHN_EnableTSMErrorRecovery);

	/* Assert the function arguments */
	BDBG_ASSERT(hRapCh);

#if (BRAP_DSPCHN_P_HAS_TSM_ERROR_RECOVERY==1)
	BKNI_EnterCriticalSection();
	rc = BRAP_DSPCHN_P_EnableTSMErrorRecovery_isr(hRapCh, bEnableTSMErrRecovery);
	BKNI_LeaveCriticalSection();
#else
	BSTD_UNUSED(bEnableTSMErrRecovery);
	BDBG_ERR(("BRAP_DSPCHN_EnableTSMErrorRecovery() is not supported for this chip"));
	return BERR_TRACE(BERR_NOT_SUPPORTED);
#endif

	BDBG_LEAVE(BRAP_DSPCHN_EnableTSMErrorRecovery);
	return rc;
}
#endif


/* End of File */
