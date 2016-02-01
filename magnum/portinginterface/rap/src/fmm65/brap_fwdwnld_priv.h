/***************************************************************************
*     Copyright (c) 2006-2011, Broadcom Corporation
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
*	This module downloads the firmware executables into DRAM.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef AUDIO_FWIF_DWNLD_UTIL_H__
#define AUDIO_FWIF_DWNLD_UTIL_H__
#include "brap_types.h"
#include "brap_img.h"
#ifndef OFFLINE_UTIL
#include "brap_dspchn.h"
#include "brap_af_priv.h"
#include "brap_cit_priv.h"
#endif


/* #define OFFLINE_UTIL 0 */

#define BRAP_FWIF_P_MAX_TSM_FW_TYPE				3
#define BRAP_FWIF_P_MAX_FS_EXE_FOR_A_ALGO	 		3
#define BRAP_FWIF_P_MAX_DEC_CODE_EXE_IN_A_CTXT 	5 /* max number of decode algo execs */
#define BRAP_FWIF_P_MAX_DEC_TBL_EXE_IN_A_CTXT 	BRAP_FWIF_P_MAX_DEC_CODE_EXE_IN_A_CTXT /* max number of decode algo execs */
#define BRAP_FWIF_P_MAX_PT_CODE_EXE_IN_A_CTXT  	1 /* max number of pass thru algo/table execs */
#define BRAP_FWIF_P_MAX_PT_TBL_EXE_IN_A_CTXT  	BRAP_FWIF_P_MAX_PT_CODE_EXE_IN_A_CTXT /* max number of pass thru algo/table execs */
#define BRAP_FWIF_P_MAX_PROC_CODE_EXE_IN_A_CTXT 	2 /* max number of decode algo execs */
#define BRAP_FWIF_P_MAX_PROC_TBL_EXE_IN_A_CTXT 	BRAP_FWIF_P_MAX_PROC_CODE_EXE_IN_A_CTXT /* max number of decode algo execs */
#define BRAP_FWIF_P_MAX_NUM_DECODE_MODE_SUPPORTED 		(BRAP_DSPCHN_DecodeMode_eMax)




#ifdef OFFLINE_UTIL
#define BDBG_MSG(x)	printf();
#define BDBG_ERR(x)	printf(x);
#define BERR_Code	void
#endif

#ifdef OFFLINE_UTIL
#endif

typedef struct BRAP_FWDWNLD_P_Param{
#ifdef OFFLINE_UTIL
	BRAP_OFFLINEUTIL_DwnldParams *pOfflineDwnldParams;
#else
	BRAP_Handle	hRap;
#endif
}BRAP_FWDWNLD_P_Param;


/***************************************************************************
Summary:
	This enumeration defines various Executable types at the Task Node (Stages).
***************************************************************************/
typedef enum BRAP_DSP_ExecType
{
	BRAP_DSP_ExecType_eDecode,		/* Decode Exec */
	BRAP_DSP_ExecType_ePassthru, 	/* Pass thru Exec */
	BRAP_DSP_ExecType_eFrameSync,	/* FrameSync Exec */
	BRAP_DSP_ExecType_eEncode,		/* Encode Exec */
	BRAP_DSP_ExecType_eProcessingAlgo,	/* Processing Algo Exec */
	BRAP_DSP_ExecType_eTsm,			/* Tsm Exec */
#ifdef RAP_REALVIDEO_SUPPORT
    BRAP_DSP_ExecType_eVideoDecode,     /* Real Video Exec */	
#endif
#ifdef RAP_GFX_SUPPORT	
    BRAP_DSP_ExecType_eGfx,         /* Tsm Exec */	
#endif    
#ifdef RAP_SCM_SUPPORT
    BRAP_DSP_ExecType_eScm,         /* SCM Exec */	
#endif    
	BRAP_DSP_ExecType_eInvalid	= 0xFF
} BRAP_DSP_ExecType;


/******************************************************************************
Summary:
	This function downloads following at device open time.
			-- Resident Code
			-- TSM Exec
			-- Audio Processing Algo
			-- Decode algorithm Exec

Pseudo Code:			
DownloadFwExec:
{
	DownloadResidentCode();

	DownloadAllTSMExec();

	If(DownloadAudProcAtOpentime) 
	{
	 	DownloadPPAlgoAtOpentime will be decided based on some logic if
	 	it is beneficial to download the PP algos at start time to save 
	 	the memory.
	 	
		DownloadProcessingExecs();
	}
	If(OpenTimeDownload)
	{

		DownloadAllSupportedExecs();
	}
}

Input Parameter:
	void *phRap:	RAP Handle (Device Handle)
		In case offline utility,Offline function will pass the a pointer to 
		structure containing required fields to copy Fw into the file.
Return:
	BERR_Code	- If RAPTOR
	void		- If Offline Utility
*******************************************************************************/
BERR_Code BRAP_FWDWNLD_P_DownloadFwExec
								(
#ifdef OFFLINE_UTIL
	BRAP_OFFLINEUTIL_DwnldParams	*pOfflineDwnldParams
#else
	BRAP_Handle						hRap
#endif
								);

/******************************************************************************
Summary:
	This Function Copies the FW exec from Image interface to buffer in the DRAM.
*******************************************************************************/
BERR_Code BRAP_FWDWNLD_P_CopyFWImageToMem(
		const BIMG_Interface *iface,
		void *pImgContext,
		uint32_t ui32MemAddr,	/* 0 , If its a offline utility */
		BRAP_Img_Id firmware_id,
		BMEM_Handle hHeap		
		);

/******************************************************************************
Summary:
	This Function returns true, If the decode algorithm having AlgoId passed as 
	argument is supported or not.
*******************************************************************************/
bool BRAP_FWDWNLD_P_IsAudProcSupported
				(
					unsigned int AlgoId
				);
/******************************************************************************
Summary:
	This Function returns true, If the audio processing algorithm having AlgoId 
	passed as argument is supported or not.
*******************************************************************************/
bool BRAP_FWDWNLD_P_IsAudCodecSupported
				(
					unsigned int AlgoId
				);

/******************************************************************************
Summary:
	This Function returns true, If the audio processing algorithm having AlgoId 
	passed as argument is supported or not.
*******************************************************************************/
bool BRAP_FWDWNLD_P_IsVidCodecSupported(unsigned int AlgoId);

/******************************************************************************
Summary:
	This Function returns true, If the Encode algorithm having AlgoId passed as 
	argument is supported or not.
*******************************************************************************/
bool BRAP_FWDWNLD_P_IsEncodeSupported(
                                    unsigned int AlgoId
                                    );

#ifndef OFFLINE_UTIL
/*******************************************************************************
Summary:
	This function returns an structure which contains all the executable to 
	download for a particular Decode Mode and Decode algo type.
*******************************************************************************/
void BRAP_FWDWNLD_P_GetExecIds(
						BRAP_DSPCHN_DecodeMode eDecMode, 
						BRAP_DSPCHN_AudioType eAudType, 
						BRAP_AF_P_sALGO_EXEC_INFO *psExecID /* [out] */);

/*******************************************************************************
Summary:
	This function returns an structure which contains all the executable to 
	download for a particular Decode Mode and audio processing algorithm type.
*******************************************************************************/

void  BRAP_FWDWNLD_P_GetAudProcExecIds(
				BRAP_CIT_P_ProcessingType eAudProcAlgo,	/* [in]Audio processing algorithm */
				BRAP_AF_P_sALGO_EXEC_INFO *psAudProcExecID /* [out] */);

/*******************************************************************************
Summary:
	This function returns an structure which contains all the executable to 
	download for a particular Encode algorithm type.
*******************************************************************************/
void  BRAP_FWDWNLD_P_GetEncodeExecIds(
				BRAP_CIT_P_EncAudioType eEncodeAlgo,	/* [in]Audio processing algorithm */
				BRAP_AF_P_sALGO_EXEC_INFO *psEncodeExecID /* [out] */);
#endif

/*******************************************************************************
Summary:
	This function returns if the Algo table is already downloaded or not. If its
	downloaded, it sets bDownloaded=true and eCodeFirmwareId to the Algo Code 
	with which the table was associated in the MIT when it was downloaded earlier.
*******************************************************************************/
void BRAP_FWDWNLD_P_IsTableDownloaded(
					BRAP_DSP_ExecType	eExecType,	
					BRAP_AF_P_AlgoId	 eCodeExecId,
					bool		*bDownloaded,
					BRAP_AF_P_AlgoId *eExecId
					);

/*******************************************************************************
Summary:
	This returns if the audio processing needs to be downloaded at Device open 
	time or Channel Start time.
*******************************************************************************/
void BRAP_FWDWNLD_P_IfOpenTimeProcDownload(
						bool *bOpenTimeAudProcDownload
						);
#ifdef RAP_REALVIDEO_SUPPORT
BERR_Code BRAP_FWDWNLD_P_DownloadVideoDecodeExecs(BRAP_FWDWNLD_P_Param *pParam,bool bDownloadAll,BRAP_DSPCHN_VideoType    eVideoType);
void BRAP_FWDWNLD_P_GetVidExecIds(
						BRAP_DSPCHN_DecodeMode eDecMode, 
						BRAP_DSPCHN_VideoType eVidType, 
						BRAP_AF_P_sALGO_EXEC_INFO *psExecID /* [out] */);
#endif
bool    BRAP_FWDWNLD_P_IsGfxSupported(unsigned int AlgoId);
#ifdef RAP_GFX_SUPPORT
BERR_Code BRAP_FWDWNLD_P_DownloadGfxExecs(BRAP_FWDWNLD_P_Param *pParam,bool bDownloadAll,BRAP_DSPCHN_GfxType    eGfxType);

void BRAP_FWDWNLD_P_GetGfxExecIds(
						BRAP_DSPCHN_AudioType eAudType, 
						BRAP_AF_P_sALGO_EXEC_INFO *psExecID /* [out] */);
#endif
#ifdef RAP_SCM_SUPPORT
void BRAP_FWDWNLD_P_GetScmExecIds(
						BRAP_DSPCHN_AudioType eAudType, 
						BRAP_AF_P_sALGO_EXEC_INFO *psExecID /* [out] */);

BERR_Code BRAP_FWDWNLD_P_DownloadSCMExecs(BRAP_FWDWNLD_P_Param *pParam,bool bDownloadAll,BRAP_DSPCHN_ScmType    eSCMType);


void BRAP_SCM_P_SetSCMAlgorithmID(BRAP_DSPCHN_ScmType eScmAlgoType);

#endif
bool    BRAP_FWDWNLD_P_IsScmSupported(unsigned int AlgoId);


#endif
