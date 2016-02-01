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
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#ifndef BRAP_DSP_MIT_H__
#define BRAP_DSP_MIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "brap_img.h"

#define NOT_ALLOTTED_BUFPTR				0xFFFFFFFF
#define NOT_ALLOTTED_BUFSIZE			0x0

#define BRAP_DSP_P_AAC_TABLE_DELAY_SIZE		(512 * 4)	
#define BRAP_DSP_P_AC3_TABLE_DELAY_SIZE		3072
#define BRAP_DSP_P_MPEG_TABLE_DELAY_SIZE	10880

#ifndef BCHP_7411_VER /* For chips other than 7411 */
/* Adding MP3 & DTS Encoder */
/*#define BAF_ALGORITHM_MPEG1_L3_ENCODER_STAGES 5*/
/*#define BAF_ALGORITHM_MPEG1_L3_ENCODER_STAGES 3*/
#define BAF_ALGORITHM_MPEG1_L3_ENCODER_STAGES 2
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
#define BAF_ALGORITHM_DTS_ENCODER_STAGES	1
#endif

const uint32_t BRAP_DSP_P_supportedEncode[] = {
	BAF_ALGORITHM_MPEG1_L3_ENCODER
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BAF_ALGORITHM_DTS_ENCODER
#endif
};

const uint32_t BRAP_DSP_P_numEnc = sizeof(BRAP_DSP_P_supportedEncode)/sizeof(uint32_t);

const uint32_t BRAP_DSP_P_EncodeStages[] = {
	BAF_ALGORITHM_MPEG1_L3_ENCODER_STAGES
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BAF_ALGORITHM_DTS_ENCODER_STAGES
#endif
};

const uint32_t BRAP_DSP_P_EncodeAlgoIDs[] = {
	BRAP_IMG_MP3_ENCODE_STG0_ID
	,BRAP_IMG_MP3_ENCODE_STG1_ID
/*	,BRAP_IMG_MP3_ENCODE_STG2_ID*/
/*	,BRAP_IMG_MP3_ENCODE_STG3_ID
	,BRAP_IMG_MP3_ENCODE_STG4_ID*/
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BRAP_IMG_DTS_ENCODE_CODE_ID
#endif
};

const uint32_t BRAP_DSP_P_EncodeTblIDs[] = {
	BRAP_IMG_MP3_ENCODE_TABLE_ID
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BRAP_IMG_DTS_ENCODE_TABLE_ID
#endif
};

const uint32_t BRAP_DSP_P_EncodeScratchIDs[] = {
	BRAP_IMG_MP3_ENCODE_SCRATCH_ID
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BRAP_IMG_DTS_ENCODE_SCRATCH_ID
#endif
};

const uint32_t BRAP_DSP_P_EncodeIFrameIDs[] = {
	BRAP_IMG_MP3_ENCODE_INTER_FRAME_ID
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BRAP_IMG_DTS_ENCODE_INTER_FRAME_ID
#endif
};

const uint32_t BRAP_DSP_P_EncodeIStageIDs[] = {
	BRAP_IMG_MP3_ENCODE_INTER_STAGE_ID
#if (( BRAP_DVD_FAMILY == 1)||(BRAP_7405_FAMILY == 1))
	,BRAP_IMG_DTS_ENCODE_INTER_STAGE_ID
#endif
};

/* Re-enable these once needed
const uint32_t BRAP_DSP_P_supportedEncodePreP[] = {

};

const uint32_t BRAP_DSP_P_numEncPreP = sizeof(BRAP_DSP_P_supportedEncodePreP)/sizeof(uint32_t);

const uint32_t BRAP_DSP_P_supportedEncodePostP[] = {

};

const uint32_t BRAP_DSP_P_numEncPostP = sizeof(BRAP_DSP_P_supportedEncodePostP)/sizeof(uint32_t);
*/
#endif

const uint32_t BRAP_DSP_P_supportedFS[] = {
	BAF_FRAME_SYNC_MPEG,
	BAF_FRAME_SYNC_AC3
};

const uint32_t BRAP_DSP_P_numFS = sizeof(BRAP_DSP_P_supportedFS)/sizeof(uint32_t);

const uint32_t BRAP_DSP_P_supportedDecode[] = {
	BAF_ALGORITHM_MPEG_L1,
	BAF_ALGORITHM_MPEG_L2,
	BAF_ALGORITHM_MP3,
	BAF_ALGORITHM_AC3,
};

const uint32_t BRAP_DSP_P_numDec = sizeof(BRAP_DSP_P_supportedDecode)/sizeof(uint32_t);

const uint32_t BRAP_DSP_P_supportedPP[] = {
	BAF_PP_DOWNMIX_AC3,
};

const uint32_t BRAP_DSP_P_numPP = sizeof(BRAP_DSP_P_supportedPP)/sizeof(uint32_t);

const uint32_t BRAP_DSP_P_supportedPT[] = {
	BAF_PASS_THRU_AC3,
	BAF_PASS_THRU_MPEG
};

const uint32_t BRAP_DSP_P_numPT = sizeof(BRAP_DSP_P_supportedPT)/sizeof(uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* BRAP_DSP_MIT_H__ */

/* End of File */
