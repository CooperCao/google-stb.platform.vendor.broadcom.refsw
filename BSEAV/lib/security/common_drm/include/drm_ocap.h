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
 * 
 *****************************************************************************/

#ifndef DRM_OCAP_H_
#define DRM_OCAP_H_

#include "drm_types.h"
#include "drm_metadata.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum drm_ocap_data_e
{
    DRM_OCAP_DATA_CA = 0,
    DRM_OCAP_DATA_ROOT,
    DRM_OCAP_DATA_CERT,
    DRM_OCAP_DATA_KEY,
    DRM_OCAP_DATA_BASE,
    DRM_OCAP_DATA_PRIME,
    DRM_OCAP_DATA_FILENAME_KEY,
    DRM_OCAP_DATA_FILENAME_CERT,
    DRM_OCAP_DATA_NONE
}drm_ocap_data_e;

/* Ocap Parameter Settings structure */
typedef struct DrmOcapParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmOcapParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_Ocap_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pOcapParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Ocap_GetDefaultParamSettings(
		DrmOcapParamSettings_t *pOcapParamSettings);


/******************************************************************************
** FUNCTION:
**  DRM_Ocap_SetParamSettings
**
** DESCRIPTION:
**   Set param settings
**
** PARAMETERS:
** ocapParamSettings - settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Ocap_SetParamSettings(
		DrmOcapParamSettings_t ocapParamSettings);

/******************************************************************************
 FUNCTION:
  DRM_Ocap_Initialize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
	N/A

******************************************************************************/
DrmRC DRM_Ocap_Initialize(char *key_file);


/******************************************************************************
 FUNCTION:
  DRM_Ocap_Finalize

 DESCRIPTION:
   Must be called only once prior to any other module API call.

 PARAMETERS:
	N/A

******************************************************************************/
void DRM_Ocap_Finalize(void);


/******************************************************************************
** FUNCTION
**   DRM_Ocap_GetDataBlock
**
** DESCRIPTION:
**   Retrieve one of six possible data blocks:
**           CA, ROOT, CERT, KEY, BASE, PRIME, Filename of KEY or CERT
**
**   The size of the 'data_block' buffer passed to this function should
**   correspond to the data being retrieved. Therefore, 2Kbytes for all data types
**   except for the filename parameters, that is:
**
**  DRM_OCAP_DATA_CA   :  OCAP_PADDED_ENC_MANUFACTURE_CA_SIZE      (2K)
**  DRM_OCAP_DATA_ROOT :  OCAP_PADDED_ENC_MANUFACTURE_ROOT_SIZE    (2K)
**  DRM_OCAP_DATA_CERT :  OCAP_PADDED_ENC_DEVICE_CERT_SIZE    (2K)
**  DRM_OCAP_DATA_KEYS :  OCAP_PADDED_ENC_DEVICE_KEY_SIZE    (2K)
**  DRM_OCAP_DATA_BASE :  OCAP_PADDED_ENC_DH_BASE    (256B)
**  DRM_OCAP_DATA_PRIME :  OCAP_PADDED_ENC_DH_PRIME    (256B)
**  DRM_OCAP_DATA_FILENAME_KEY  : 16
**  DRM_OCAP_DATA_FILENAME_CERT : 16
**
**
**
** PARAMETERS:
**   data_block - buffer to store returned data
**   data_type - DRM_OCAP_DATA_CA, DRM_OCAP_DATA_ROOT,
**               DRM_OCAP_DATA_CERT or DRM_OCAP_DATA_KEY,
**               DRM_OCAP_DATA_BASE or DRM_OCAP_DATA_PRIME,
**               DRM_OCAP_DATA_FILENAME_KEY, DRM_OCAP_DATA_FILENAME_CERT
**   *return_size - the actual size of the data block copied and returned
**                  to the caller.  Decrypted data will always be 16-byte aligned
**                  and be terminated with padded with zero (if necessary)
**
**  NOTE: The DRM_OCAP_DATA_FILENAME_KEY, DRM_OCAP_DATA_FILENAME_CERT
**          can be represented as strings although they will not be NULL
**          terminated
**
** RETURNS:
**   DRM_SUCCESS or error
**
******************************************************************************/
DrmRC DRM_Ocap_GetDataBlock(uint8_t *data_block, drm_ocap_data_e data_type, uint32_t *return_size);

#ifdef __cplusplus
}
#endif

#endif /* DRM_OCAP_H_ */
