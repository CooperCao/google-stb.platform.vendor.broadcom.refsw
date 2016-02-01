/*********************************************************************************
*     Copyright (c) 2013, Broadcom Corporation
*     All Rights Reserved
*     Confidential Property of Broadcom Corporation
*
*   This program is the proprietary software of Broadcom Corporation and/or its licensors, 
*   and may only be used, duplicated, modified or distributed pursuant to the terms and 
*   conditions of a separate, written license agreement executed between you and Broadcom 
*   (an "Authorized License").  Except as set forth in an Authorized License, 
*   Broadcom grants no license (express or implied), right to use, or waiver of any kind 
*   with respect to the Software, and Broadcom expressly reserves all rights in and to the 
*   Software and all intellectual property rights therein.  
* 
*   IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, 
*   AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*   Except as expressly set forth in the Authorized License,
*   1.     This program, including its structure, sequence and organization, constitutes the 
*       valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect 
*       the confidentiality thereof, and to use this information only in connection with your use 
*       of Broadcom integrated circuit products.
*   2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL 
*       FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, 
*       STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
*       ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*       LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO 
*       DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*   3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE 
*       FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT 
*       OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN 
*       ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID 
*       FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
*       ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* PIFF Encoder Types 
* 
* Revision History:
*
* $brcm_Log: $
* 
*********************************************************************************/
#ifndef BPIFF_ENCODER_TYPES_H
#define BPIFF_ENCODER_TYPES_H

#include "drm_prdy_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 * CENC types
 *****************************************************************/    
#define BMP4_CENC_SAMPLEENCRYPTION  BMP4_TYPE('s','e','n','c')
#define BMP4_CENC_AVCC              BMP4_TYPE('a','v','c','C')
#define BMP4_CENC_ESDS              BMP4_TYPE('e','s','d','s')

/****************************************************************
 * BPIFF_Error is the common return type for PIFF Encoder APIs .
 * Any non-zero BPIFF_Error value is a failure. Zero is success.
 *****************************************************************/    
typedef unsigned BPIFF_Error;

#define PIFF_ENCODE_SUCCESS                  0
#define PIFF_ENCODE_INVALID_HANDLE           1
#define PIFF_ENCODE_INVALID_AUDIO_HANDLE     2
#define PIFF_ENCODE_INVALID_VIDEO_HANDLE     3
#define PIFF_ENCODE_INVALID_LOCK             4
#define PIFF_ENCODE_INVALID_ARG              5
#define PIFF_ENCODE_KEY_ID_NOT_AVAILABLE     6
#define PIFF_ENCODE_UNKNOWN_ERROR            7

#define PIFF_GUID_LENGTH                     16  /* Lenght of DRM GUID in bytes */
#define PIFF_KEY_ID_LENGTH                   16  /* Key identifier length in bytes */

typedef struct bpiff_encoder_context *PIFF_encoder_handle;

/**********************************************************
 * Generic call back definition to signal the completion
 * of PIFF encoding.
 **********************************************************/
typedef void (*PIFF_Completion_CB)(void *context);

typedef struct PIFF_Encoder_Callback {
    PIFF_Completion_CB callback;
    void *             context;  /* optional user defined callback context */

} PIFF_Encoder_Callback;

typedef struct PIFF_Pssh_System_ID {
    uint8_t   data[PIFF_GUID_LENGTH];

} PIFF_Pssh_System_ID;

typedef enum PIFF_Encryption_Type {
    PIFF_Encryption_Type_eNone,        /* no encryption */             
    PIFF_Encryption_Type_eWmdrm,       /* wmdrm encrypted asf */       
    PIFF_Encryption_Type_eAesCtr,      /* AES CTR encrypted stream */  
    PIFF_Encryption_Type_eAesCbc,      /* AES CBC encrypted stream */  
    PIFF_Encryption_Type_eMax

} PIFF_Encryption_Type;

typedef struct PIFF_Encoder_Settings {
    PIFF_Pssh_System_ID    systemId;
    PIFF_Encryption_Type   algorithm; 
    unsigned               ivSize;
    char                  *destPiffFileName;  /* destination file string */
    PIFF_Encoder_Callback  completionCallBack;  

    /* =========================================================== */
    /* Any of the following license acquisition data is optional.  */  
    /* NOTE: when PR 2.5 ND is fully supported, they will not be   */
    /* used.                                                       */
    /* =========================================================== */
    char *                licAcqLAURL; 
    uint32_t              licAcqLAURLLen;

    char *                licAcqLUIURL; 
    uint32_t              licAcqLUIURLLen;

    /* if Key ID is set, it will override the one being generated */ 
    uint8_t *             licAcqKeyId; 
    uint32_t              licAcqKeyLen; /* must be 16 */

    uint8_t *             licAcqDSId; 
    uint32_t              licAcqDSIdLen;

    char *                licCustomAttr; 
    uint32_t              licCustomAttrLen;

    /* ===================================
     * PR 2.5 ND License creation setting
     * =================================== */
    DRM_Prdy_local_license_policy_descriptor_t licPolicyDescriptor; 
    DRM_Prdy_local_license_type_e              licType;        
    Drm_Prdy_KID_t                             keyId;  

} PIFF_Encoder_Settings;


#ifdef __cplusplus
}
#endif


#endif /*BPIFF_ENCODER_TYPES_H*/
