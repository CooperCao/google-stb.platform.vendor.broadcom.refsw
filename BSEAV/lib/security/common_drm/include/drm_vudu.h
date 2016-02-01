/******************************************************************************
 *    (c)2012 Broadcom Corporation
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
 *****************************************************************************/
#ifndef DRM_VUDU_H_
#define DRM_VUDU_H_

#include "drm_types.h"
#include "nexus_security.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct DRM_VuduContentKeyParams_t
{
	uint8_t procInForKey3[32];
	uint8_t procInForKey4[32];
	uint32_t procInLength;
	uint8_t CustKeySelect;
	uint8_t KeyVarHigh;
	uint8_t KeyVarLow;
	uint8_t Iv[32];
	uint32_t IvLength;
}DRM_VuduContentKeyParams_t;

/* Vudu Parameter Settings structure */
typedef struct DrmVuduParamSettings_t
{
	DrmCommonInit_t drmCommonInit;
}DrmVuduParamSettings_t;

/******************************************************************************
** FUNCTION:
**  DRM_Vudu_GetDefaultParamSettings
**
** DESCRIPTION:
**   Retrieve the default settings
**
** PARAMETERS:
** pVuduParamSettings - pointer to settings structure
**
** RETURNS:
**   void.
**
******************************************************************************/
void DRM_Vudu_GetDefaultParamSettings(
		DrmVuduParamSettings_t *pVuduParamSettings);

/**********************************************************
 * FUNCTION:
 *  DRM_Vudu_Initialize
 *
 * DESCRIPTION:
 *   Initialize the Vudu DRM module
 *
 * RETURN:
 * 	Drm_Success or other
***********************************************************/
DrmRC DRM_Vudu_Initialize(void);

/*
// Frees the key slots for vudu decryption
*/
void DRM_Vudu_Finalize(void);



/**********************************************************
 * FUNCTION:
 *  DRM_Vudu_SetConfigAlgParams
 *
 * DESCRIPTION:
 *   Prepare the vudu module with the proper configuration
 *   settings.
 *
 * RETURN:
 * 	Drm_Success or other
***********************************************************/
DrmRC DRM_Vudu_SetConfigAlgParams(
					DrmSecurityAlgorithm alg,
					DrmSecurityAlgorithmVariant algVar,
					DrmSecurityOperation operation);



/*//////////////////////////////////////////////////////////////////////////////
/// Loads the CW using dynamic keys located in the DRM binary file.
/// @param[in]      pCommonKey  Pointer to the buffer of encrypted Common Key.
/// @param[in]      pEcm        Pointer to the buffer of encrypted ECM.
/// @param[in/out]  pId         Pointer to the buffer where the 4-byte ID of the CW is copied into.
/// @param[in]      keyslot     Keyslot Type(odd or even key slot)
/// @return drm_wv_ok if the operation is successful or an error.
//////////////////////////////////////////////////////////////////////////////*/
DrmRC
DRM_Vudu_LoadClearKey(
                    const uint32_t keyLen,
                    const uint8_t* keyData,
                    const uint32_t ivLen,
                    const uint8_t* ivData
);


DrmRC
DRM_Vudu_LoadContentKey(
		DRM_VuduContentKeyParams_t *pContentKeyParams
);

DrmRC
DRM_Vudu_PrepContentKey(
		const uint8_t *clearIV
);

DrmRC
DRM_Vudu_UpdateContentKey(
        const uint8_t *procInForKey3,
        const uint8_t *procInForKey4,
        DrmSecurityKeyType keyType);

DrmRC DRM_Vudu_Decrypt(uint8_t* pSrc,
                    uint32_t length,
                    uint8_t* pDst);

/**********************************************************
** FUNCTION:
**  DRM_Vudu_AttachPidChannel
**
** DESCRIPTION:
**   Attach a pid channel to the content key
**
** RETURN:
** 	Drm_Success or other
***********************************************************/
DrmRC DRM_Vudu_AttachPidChannel(unsigned int pidChannel);


/******************************************************************************
** FUNCTION:
**  DRM_Vudu_DetachPidChannel
**
** DESCRIPTION:
**  Detach the pid channel from the content key keyslot
******************************************************************************/
DrmRC DRM_Vudu_DetachPidChannel(unsigned int pidChannel);

/******************************************************************************
** FUNCTION:
** DRM_Vudu_GetKeySlot
**
** DESCRIPTION:
**  get keyslot handle
******************************************************************************/
DrmRC DRM_Vudu_GetKeySlot(NEXUS_SecurityKeyType keyslot_type, NEXUS_KeySlotHandle*pKeySlot);

#ifdef __cplusplus
}
#endif

#endif /*DRM_VUDU_H_*/
