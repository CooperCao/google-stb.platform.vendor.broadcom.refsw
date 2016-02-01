/******************************************************************************
 *    (c)2010-2012 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
 * USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/
#ifndef _DRM_MARLIN_H_
#define _DRM_MARLIN_H_

#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"

#include "drm_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Marlin Parameter Settings structure */
typedef struct DrmMarlinParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmMarlinParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_Marlin_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pMarlinParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Marlin_GetDefaultParamSettings(
		DrmMarlinParamSettings_t *pMarlinParamSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Marlin_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** marlinParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Marlin_SetParamSettings(
		DrmMarlinParamSettings_t marlinParamSettings);

/******************************************************************************
 FUNCTION:
  DRM_Marlin_Initialize

 DESCRIPTION:
   MUST BE CALLED PRIOR TO USING ANY OTHER API IN THE MODULE.

 PARAMETERS:
	N/A

******************************************************************************/
DrmRC DRM_Marlin_Initialize(void);


/******************************************************************************
 FUNCTION:
  DRM_Marlin_Finalize

 DESCRIPTION:
   Cleans up the Marlin module

 PARAMETERS:
 N/A

******************************************************************************/
DrmRC DRM_Marlin_Finalize(void);

/******************************************************************************
 FUNCTION:
  DRM_Marlin_SetKey

 DESCRIPTION:
   Used to set up the proper key and iv prior to decryption

 PARAMETERS:
 key_buf[in]  Buffer containing the key to load
 key_len[in]  Represents the size of the key
 iv_buf[in]  Buffer containing the IV
 iv_len[in]  Represents the length of the IV
 content_length[in]  Represents the length of the entire content buffer to decrypt
 termMode[in]  Specifies the termination type: CBC, Residual or Counter (CTR) mode

RETURNS:
	Drm_Success or other (error)

******************************************************************************/
DrmRC DRM_Marlin_SetKey(
		uint8_t *key_buf,
		uint32_t key_len,
		uint8_t *iv_buf,
		uint32_t iv_len,
		DrmTerminationMode termMode
	);


/******************************************************************************
 FUNCTION:
  DRM_Marlin_Decrypt

 DESCRIPTION:
	Decrypts the source buffer based on the key & IV set in the call to
	'DRM_Marlin_SetKey'

 src_buf[in]  Buffer containing the source data to decrypt
 src_buf_len[in]  Length of the source buffer.  Can be less than the
 	 	 	 	 'content_length' specified when the key &  IV were set
 dst_buf[in]  Buffer containing the key to load

NOTE: buffers passed to this function should be allocated with a call to
	'NEXUS_Memory_Allocate' otherwise it will fail

RETURNS:
	Drm_Success or other (error)

******************************************************************************/
DrmRC DRM_Marlin_Decrypt(
		uint8_t *src_buf,
		uint32_t src_buf_len,
		uint8_t *dst_buf,
		uint8_t *iv
	);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _DRM_MARLIN_H_ */
