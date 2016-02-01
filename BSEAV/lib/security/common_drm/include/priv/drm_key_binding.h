/***************************************************************************
 *     Copyright (c) 2012, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * Module Description: This file binds the DRM binary data to the chip
 *
 * Revision History:
 *
 *
 ***************************************************************************/

#ifndef DRM_KEY_BINDING_H_
#define DRM_KEY_BINDING_H_

#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "drm_types.h"

typedef struct drm_key_binding_t
{
	uint8_t proc_in1[16];
	uint8_t proc_in2[16];
	uint8_t devIdA[8];
	uint8_t devIdB[8];
}drm_key_binding_t;

void DRM_KeyBinding_Init(void);
void DRM_KeyBinding_UnInit(void);

DrmRC DRM_KeyBinding_FetchDeviceIds(
				drm_key_binding_t *pStruct);

DrmRC DRM_KeyBinding_GetProcsFromOtp(
				drm_key_binding_t *pStruct);

DrmRC DRM_KeyBinding_GenerateProcsFromOtp(
				drm_key_binding_t *pStruct,
				uint8_t *pProcIn1FromBinHeader,
				uint8_t *pProcIn2FromBinHeader);

#endif /*DRM_KEY_BINDING_H_*/
