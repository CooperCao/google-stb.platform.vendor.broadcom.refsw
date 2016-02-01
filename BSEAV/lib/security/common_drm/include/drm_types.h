/******************************************************************************
 *    (c)2011-2012 Broadcom Corporation
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
#ifndef DRM_TYPES_H_
#define DRM_TYPES_H_

#include "bstd.h"
#include "nexus_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRM_HASH_SIZE (20)
#define DRM_PADDED_HASH_SIZE (32)

typedef enum DrmRC{
    Drm_Success,
    Drm_Err,
    Drm_InvalidParameter,
    Drm_FileErr,
    Drm_InitErr,
    Drm_CryptoOpenErr,
    Drm_CryptoConfigErr,
    Drm_CryptoLoadSwKeyErr,
    Drm_CryptoDmaErr,
	Drm_CryptoSecureRsaErr,
    Drm_RegionHashErr,
    Drm_KeyRegionErr,
    Drm_InvalidStateErr,
    Drm_PmcCoreErr,
    Drm_UtvFormatErr,
    Drm_MemErr,
    Drm_BufTooSmall,
    Drm_BcryptErr,
    Drm_BcryptContextErr,
    Drm_BindingErr,
    Drm_NexusErr,
    Drm_NotFound,
    Drm_NotImpl,
    Drm_SraiModuleError,
    Drm_Max
}DrmRC;

typedef enum DrmCryptoOperation{
    DrmCryptoOperation_eEncrypt,
    DrmCryptoOperation_eDecrypt,
    DrmCryptoOperation_ePassThrough,
    DrmCryptoOperation_eMax
}DrmCryptoOperation;

typedef enum DrmDestinationType{
    DrmDestinationType_eInPlace,
    DrmDestinationType_eExternal,
    DrmDestinationType_eMax
}DrmDestinationType;

typedef enum DrmFileOperation{
    DrmFileOperation_eWrite,
    DrmFileOperation_eRead,
    DrmFileOperation_eMax
}DrmFileOperation;

typedef enum DrmTerminationMode{
	DrmTerminationMode_eCounter = 0,
	DrmTerminationMode_eCbc,
	DrmTerminationMode_eResidual,
	DrmTerminationMode_eMax
}DrmTerminationMode;

typedef enum DrmM2mPacketType{
	DrmM2mPacketType_eStart = 0,
	DrmM2mPacketType_eMiddle,
	DrmM2mPacketType_eEnd,
	DrmM2mPacketType_eMax
}DrmM2mPacketType;

typedef enum DrmKeyProvisioningType
{
	DrmKeyProvisioningType_eUtv = 0,
	DrmKeyProvisioningType_eBrcm = 1,
	DrmKeyProvisioningType_eMax
}DrmKeyProvisioningType;


/***************************************************************************************/
/**
Summary:
This enum defines the supported crypto alogrithms.
**/
typedef enum DRM_SecurityAlgorithm
{
    DrmSecurityAlgorithm_eDvb,    /* DVB will only be used by CA descrambler */
    DrmSecurityAlgorithm_eReserved1,
    DrmSecurityAlgorithm_eMulti2,
    DrmSecurityAlgorithm_eDes,
    DrmSecurityAlgorithm_e3DesAba,
    DrmSecurityAlgorithm_e3DesAbc,
    DrmSecurityAlgorithm_eAes,
    DrmSecurityAlgorithm_eReserved7,
    DrmSecurityAlgorithm_eAes192,
    DrmSecurityAlgorithm_eTivo,

    /* The 4 algorithms below are only supported for M2M */
    DrmSecurityAlgorithm_eC2,
    DrmSecurityAlgorithm_eCss,
    DrmSecurityAlgorithm_eM6Ke,
    DrmSecurityAlgorithm_eM6,

    /* added WMDrmPD */
    DrmSecurityAlgorithm_eRc4,
    DrmSecurityAlgorithm_eCbcMac,
    DrmSecurityAlgorithm_eWMDrmPd,

    DrmSecurityAlgorithm_eAes128G,
    DrmSecurityAlgorithm_eHdDVD,
    DrmSecurityAlgorithm_eBrDVD,

    /* Add new security algorithm definition before this line */
    DrmSecurityAlgorithm_eMax
}   DrmSecurityAlgorithm;

/**
Summary:
This enum defines the supported crypto alogrithm variants.
**/
typedef enum DrmSecurityAlgorithmVariant
{
    DrmSecurityAlgorithmVariant_eEcb = 0,
    DrmSecurityAlgorithmVariant_eXpt = 0,  /* for DrmSecurityAlgorithm_eDvb, scramble level */
    DrmSecurityAlgorithmVariant_eCbc = 1,
    DrmSecurityAlgorithmVariant_ePes = 1,  /* for DrmSecurityAlgorithm_eDvb, scramble level */
    DrmSecurityAlgorithmVariant_eCounter = 2,
    DrmSecurityAlgorithmVariant_eRCbc    = 3,

    /* Add new algorithm variant definition before this line */
    DrmSecurityAlgorithmVariant_eMax
}   DrmSecurityAlgorithmVariant;

/**
Summary:
This enum defines the supported termination modes.
For pairs of Enums, first one for ASKM; second for regular
**/
typedef enum DrmSecurityTerminationMode
{
    DrmSecurityTerminationMode_eClear                 = 0,
    DrmSecurityTerminationMode_eCbcResidual           = 1,
    DrmSecurityTerminationMode_eBlock                 = 1,
    DrmSecurityTerminationMode_eReserved2             = 2,
    DrmSecurityTerminationMode_eCipherStealing        = 2,
    DrmSecurityTerminationMode_eCtsCpcm               = 3,
    DrmSecurityTerminationMode_eCipherStealingComcast = 3,
    DrmSecurityTerminationMode_eReserved4             = 4,
    DrmSecurityTerminationMode_eReserved5             = 5,

    /* Add new termination mode definition before this line */
    DrmSecurityTerminationMode_eMax
}   DrmSecurityTerminationMode;


/**
Summary:
This enum defines crypto operations.
**/
typedef enum DrmSecurityOperation
{
    DrmSecurityOperation_eEncrypt,
    DrmSecurityOperation_eDecrypt,
    DrmSecurityOperation_ePassThrough,    /* This one is only for M2M */

    /* Add new operation definition before this line */
    DrmSecurityOperation_eMax
}  DrmSecurityOperation;

/**
Summary:
This enum defines the supported key entry types.
**/
typedef enum DrmSecurityKeyType
{
    DrmSecurityKeyType_eOdd,      /* ODD key */
    DrmSecurityKeyType_eEven,     /* EVEN key */
    DrmSecurityKeyType_eClear,    /* CLEAR key */
    DrmSecurityKeyType_eIv,       /* Initial Vector */

    /* Add new key entry type definition before this line */
    DrmSecurityKeyType_eMax
}   DrmSecurityKeyType;

typedef struct DmaBlockInfo_t
{
    uint8_t *pDstData;          /* Must be aligned */
    uint8_t *pSrcData;          /* Must be aligned, must be aligned and padded to multiple of 8 bytes if pSrcData is a key to load for external key loading. */
    uint32_t uiDataSize;        /* Size of the data buffer; padded key lenght is pSrcData is a key used for external key loading */
    bool sg_start;          /* true if start of scatter gather; false otherwise */
    bool sg_end;            /* true if end of scatter gather; false otherwise */

} DmaBlockInfo_t;

typedef struct DrmCommonInit_t
{
	NEXUS_HeapHandle heap;
}DrmCommonInit_t;

#ifdef __cplusplus
}
#endif

#endif /*DRM_TYPES_H_*/
