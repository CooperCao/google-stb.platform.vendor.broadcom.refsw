/***************************************************************************
 *     Copyright (c) 2004-2006, Broadcom Corporation
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
 
#ifndef AUDIO_DSP_HOST_ENCODER_INTERFACE_H__
#define AUDIO_DSP_HOST_ENCODER_INTERFACE_H__

#include "brap_dsphostcommon.h"

#define BAF_HOST_MAX_PRE_PROCESS_DL_MODULE              5       /* Max number of downloadable pre-processing algorithms  */
#define BAF_HOST_MAX_ENCODED_DL_MODULE                       25      /* Max number of downloadable encoding algorithms  */
#define BAF_HOST_MAX_POST_PROCESS_DL_MODULE              5      /* Max number of downloadable post-processing algorithms  */
#define BAF_ENCODED_NUM_CHANNELS 	2  					/* Maximum number of encoded channels			*/
#define BAF_HOST_MAX_ENCODE_BUF_COUNT		BAF_HOST_MAX_ENCODED_DL_MODULE + BAF_HOST_MAX_PRE_PROCESS_DL_MODULE + BAF_HOST_MAX_POST_PROCESS_DL_MODULE	/* Maximum number of iframe/istage bufs */


/* All the various AlgoID's to be used for audio encoders */
#define BAF_ALGORITHM_MPEG1_L1_ENCODER			1
#define BAF_ALGORITHM_MPEG1_L2_ENCODER			5
#define BAF_ALGORITHM_MPEG1_L3_ENCODER			10
#define BAF_ALGORITHM_DDCE_ENCODER				15
#define BAF_ALGORITHM_AAC_HE_ENCODER			20
#define BAF_ALGORITHM_DTS_ENCODER                   		23


typedef struct BAF_HostEncodeBufInfo
{
    uint32_t                 id;     /* One of AUDIO_DSP_ENCODER_xxx*/
    uint32_t                 size;   /* size */    
    BAF_HostDramAddress      data;   /* data pointer to DRAM address */
}BAF_HostEncodeBufInfo;

#define SIZE_OF_BAF_HOST_ENCODE_BUF_INFO (4 + 4 + 4)

typedef struct BAF_EncodeDownload
{
    BAF_HostEncodeBufInfo          sFirmware;	/* Firmware */
    BAF_HostEncodeBufInfo          sTables;  /* Data tables for given encode algorithm only one DRAM pointer to all the lookup tables */    
/* These two can be part of main MIT if need arises */
    BAF_HostEncodeBufInfo  InterFrameBuf[BAF_ENCODED_NUM_CHANNELS];  /* Inter frame buffers ( This buffer is context dependant) */
    BAF_HostEncodeBufInfo  InterStageBuf[BAF_ENCODED_NUM_CHANNELS];  /* Inter stage buffers also to be used as an intermediate */                   
                  /* results storing buffer ( This buffer is also context dependant )*/
}BAF_EncodeDownload;

#define SIZE_OF_BAF_ENCODE_DOWNLOAD (SIZE_OF_BAF_HOST_ENCODE_BUF_INFO*2 + (SIZE_OF_BAF_HOST_ENCODE_BUF_INFO*BAF_ENCODED_NUM_CHANNELS*2))

typedef struct BAF_HostEncodeInfo
{
    uint32_t         	 			id;						/* BAF_HOST_AL_TABLE_ID */
    uint32_t          				version;					/* BAF_HOST_AL_VERSION */
    BAF_HostEncodeBufInfo   		EncodeSchedulerBuf;		/* Encoder scheduler data buffer for storing the state variables */
    BAF_HostEncodeBufInfo		EncodeScratchBuf; 		/* Encoder scratch buffers */
    BAF_EncodeDownload  		*pPreProcess[BAF_HOST_MAX_PRE_PROCESS_DL_MODULE]; /* Array of pre_process algorithms pointers */
    BAF_EncodeDownload  		*pEncode[ BAF_HOST_MAX_ENCODED_DL_MODULE ];   /* Array of Encode algorithms pointers */
    BAF_EncodeDownload  		*pPostProcess[BAF_HOST_MAX_POST_PROCESS_DL_MODULE]; /* Array of pre_process algorithms pointers */
}BAF_HostEncodeInfo;

#define SIZE_OF_BAF_HOST_ENCODE_INFO ( (4 + 4) + (SIZE_OF_BAF_HOST_ENCODE_BUF_INFO * 2) + (4 * BAF_HOST_MAX_PRE_PROCESS_DL_MODULE) + (4 * BAF_HOST_MAX_ENCODED_DL_MODULE) + (4 * BAF_HOST_MAX_POST_PROCESS_DL_MODULE))


#endif









