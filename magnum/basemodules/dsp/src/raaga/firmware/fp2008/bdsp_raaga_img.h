/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
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
 * Module Description: Host DSP Task Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BDSP_RAAGA_IMG_H_
#define BDSP_RAAGA_IMG_H_

#include "bimg.h"

#define BDSP_IMG_ID_BASE(algId)   (BDSP_SystemImgId_eMax+((algId)*3))
#define BDSP_IMG_ID_CODE(algId)   (BDSP_IMG_ID_BASE(algId)+0)
#define BDSP_IMG_ID_IFRAME(algId) (BDSP_IMG_ID_BASE(algId)+1)
#define BDSP_IMG_ID_TABLE(algId)  (BDSP_IMG_ID_BASE(algId)+2)
#define BDSP_IMG_ID_MAX           (BDSP_IMG_ID_BASE(BDSP_AF_P_AlgoId_eMax))

#define BDSP_IMG_ID_TO_ALGO(imgId) (((imgId)<BDSP_SystemImgId_eMax)?(BDSP_AF_P_AlgoId_eMax):(((imgId)-BDSP_SystemImgId_eMax)/3))

/* This chunk size will be used when the firmware binary is actually present in 
    multiple chunks. The BDSP_IMG_CHUNK_SIZE will then give the size of each 
    such chunk
*/
#define BDSP_IMG_CHUNK_SIZE	65532


extern void *BDSP_IMG_Context;
extern const BIMG_Interface BDSP_IMG_Interface;

#endif
