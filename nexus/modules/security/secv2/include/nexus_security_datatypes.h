/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
#ifndef NEXUS_SECURITY_DATATYPES_H___
#define NEXUS_SECURITY_DATATYPES_H___

#include "nexus_types.h"
#include "nexus_base_types.h"
#include "nexus_keyslot.h"


#ifdef __cplusplus
extern "C" {
#endif


/* defines to interprete Zeus version. */
#define NEXUS_ZEUS_VERSION_CALC(major,minor)            ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8)                      )
#define NEXUS_ZEUS_VERSION_CALC_3(major,minor,subMinor) ((((major) & 0xFF)<<16) | (((minor) & 0xFF)<<8) | ((subMinor) & 0xFF))

#define NEXUS_ZEUS_VERSION  NEXUS_ZEUS_VERSION_CALC_3( NEXUS_SECURITY_ZEUS_VERSION_MAJOR,   \
                                                       NEXUS_SECURITY_ZEUS_VERSION_MINOR,   \
                                                       NEXUS_SECURITY_ZEUS_VERSION_SUBMINOR )

/* defines to interprete the BFW firmware version */
#define NEXUS_BFW_VERSION_CALC(major,minor,subMinor) ( (((major) & 0xFF)<<16) |(((minor) & 0xFF)<<8) | ((subMinor) & 0xFF) )



/**
Summary:
Deprecated: Use unsigned integer instead.
                   This enum did define the supported Module IDs used for platforms with ASKM support.
**/
#define NEXUS_SECURITY_MAX_HOST_FIRMWARE_KEYLADDER_MODULE_ID           127
#define NEXUS_SECURITY_MAX_HOST_HARDWARE_KEYLADDER_MODULE_ID           224
#define NEXUS_SECURITY_MAX_SAGE_KEYLADDER_MODULE_ID                    255

#define NEXUS_SECURITY_IP_LICENCE_SIZE     (64)

typedef enum NEXUS_SecurityFirmwareType{
    NEXUS_SecurityFirmwareType_eTransport,      /* RAVE */
    NEXUS_SecurityFirmwareType_eVideoDecoder,   /* HVD */
    NEXUS_SecurityFirmwareType_eAudioDecoder,   /* RAAGA */
    NEXUS_SecurityFirmwareType_ePictureDecoder, /* SID */
    NEXUS_SecurityFirmwareType_eVideoEncoder,   /* VCE */
    NEXUS_SecurityFirmwareType_eMax
}NEXUS_SecurityFirmwareType;

/**
    Security Module setting that are *not* accessible from application space.
**/
typedef struct NEXUS_SecurityModuleInternalSettings
{
    NEXUS_ModuleHandle transport;
    bool callTransportPostInit;
} NEXUS_SecurityModuleInternalSettings;


/**
    The security module configuration structure. This structure is accessible to the integration engineer via NEXUS_Platform_Init.
**/
typedef struct NEXUS_SecurityModuleSettings
{
    NEXUS_CommonModuleSettings common;

    unsigned numKeySlotsForType[NEXUS_KeySlotType_eMax];   /* Number of each type of keyslot. */

    bool enforceAuthentication[NEXUS_SecurityFirmwareType_eMax];

} NEXUS_SecurityModuleSettings;



#ifdef __cplusplus
}
#endif

#endif
