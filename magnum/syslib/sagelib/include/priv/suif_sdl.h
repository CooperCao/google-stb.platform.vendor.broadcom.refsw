/***************************************************************************
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
 **************************************************************************/


/*
 * Definitions for the "Secure Unified Image Format" (SUIF)
 * SAGE Dynamically Loaded (Aplication and extension) (SDL) image.
 */


#ifndef SUIF_SDL_H__
#define SUIF_SDL_H__

#include "suif.h"
#include "suif_kl.h"


/* To process a SUIF SDL, we can cast the dummy header into SDL header:
     SUIF_PackageHeader::image to SUIF_SDLImageHeader
   example:
     uint8_t *SUIF_binary_buffer = <...>;
     SUIF_PackageHeader *pHeader = (SUIF_PackageHeader *)SUIF_binary_buffer;
     SUIF_PackageHeader *pSdlHeader = (SUIF_SDLImageHeader *)&pHeader->image;
     // now can access to SDLSpecificHeader fields through pSdlHeader->sdl
*/


typedef enum {
    SUIF_SecureLogType_eDisabled = 0x0,
    SUIF_SecureLogType_ePrivate = 0x1,
    SUIF_SecureLogType_eCommon = 0x2
} SUIF_SecureLogType_e;

typedef enum {
    SUIF_SdlSlotType_eLocalSRAM_TAExt1           = 0x00,
    SUIF_SdlSlotType_eSlotType_eDRAM_TAExt1      = 0x10,
    SUIF_SdlSlotType_eSlotType_eDRAM_TAExt2      = 0x11,
    SUIF_SdlSlotType_eSlotType_eDRAM_TA          = 0x20
} SUIF_SdlSlotType_e;

#define SUIF_TA_NAME_SIZE_BYTES (16) /* size of the TA name in bytes */

typedef struct {
/*   0 [offset] */
    SUIF_KeyLadder keyladder;
/*  80 [offset] */
    uint8_t taName[SUIF_TA_NAME_SIZE_BYTES];
/*  96 [offset] */
    SUIF_Version ssfVersion; /* SSF version (4 bytes: [major][minor][revision][branch]) */
    uint8_t secureLogType;       /* See SUIF_SecureLogType_e */
    uint8_t slotType;            /*  */
    uint8_t demandPaging;        /* 0x0 Disabled ; 0x1 Enabled */
    uint8_t ssfBounded;          /* 0x0 Not bounded to ssfVersion ; 0x1 bounded to ssfVersion */
    uint8_t epochRegionId;
    uint8_t reserved0[3];        /* 3 reserved bytes */
    uint8_t platformId_BE[4];
/* 112 [offset] */
    uint8_t bssSize_BE[4];
    uint8_t startingAddress_BE[4];
    uint8_t dynamicOffset_BE[4];
/* 104 [offset] */
} SUIF_SDLSpecificHeader;

#define SUIF_SDL_IMG_PAD_SIZE_BYTES (SUIF_HEADER_SIZE_BYTES - sizeof(SUIF_CommonHeader) - sizeof(SUIF_SDLSpecificHeader))
typedef struct {
    SUIF_CommonHeader common;
    SUIF_SDLSpecificHeader sdl;
    uint8_t pad[SUIF_SDL_IMG_PAD_SIZE_BYTES];
} SUIF_SDLImageHeader;

#define SUIF_GetSdlImageHeaderFromPackageHeader(PKG) ((const SUIF_SDLImageHeader *)&(PKG)->imageHeader)
#define SUIF_GetSdlHeaderFromPackageHeader(PKG) &SUIF_GetSdlImageHeaderFromPackageHeader(PKG)->sdl


#endif /* SUIF_SDL_H__ */
