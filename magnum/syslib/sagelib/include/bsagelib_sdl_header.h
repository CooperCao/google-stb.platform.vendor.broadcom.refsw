/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/


#ifndef BSAGELIB_SDL_HEADER_H__
#define BSAGELIB_SDL_HEADER_H__


/***********************************************************
 * SAGE Dynamic Loader header
 ***********************************************************/


typedef struct
{
    uint8_t ucSigningRights;
    uint8_t ucSdlID;
    uint8_t ucPubExp;
    uint8_t ucRsvd0;

    uint8_t ucMarketID[4];
    uint8_t ucMarketIDMask[4];

    uint8_t ucMktIdSelect;
    uint8_t ucEpochSelect;
    uint8_t ucEpochMask;
    uint8_t ucEpoch;

    uint8_t ucSignatureVersion;
    uint8_t ucSignatureType;
    uint8_t ucRsvd2;
    uint8_t ucRsvd1;
} BSAGElib_SDLControllingParameters;


typedef struct
{
    uint8_t ucHeaderIndex[4];                 /* [0-1] is used for header type (SAGE Bootloader or SAGE SW) */
    uint8_t ucSecurityType;                   /* security type: signature, encryption or both */
    uint8_t ucImageType;                      /* data type: SAGE bootloader or SAGE SW or SDL */
    uint8_t ucHeaderVersion[2];               /* version of the structure used for the header */
    uint8_t ucSdlVersion[4];                  /* SDL version */
    uint8_t ucSageSecureBootToolVersion[4];   /* Version of the secure boot tool used to signed the binary */
    uint8_t ucSdlAntiRollbackVersion[4];      /* SDL (antirollback) epoch */
    uint8_t ucSsfVersion[4];                  /* SAGE Software Framework Version link against */
    uint8_t ucSsfThlShortSig[4];              /* 32 firsts bits of the SAGE Software Framework Thin-Layer signature*/
    uint8_t ucGlobalKeyOwnerIdSelect;         /* Can be 0 (MSP0), 1 (MSP1) or 3 (Use1) */
    uint8_t ucKeyZeroSelect;                  /* Select Key0 (value=0x43) or Key0Prime (value=0x49) */
    uint8_t ucReserved3[2];
    /* 32 bytes above */

    uint8_t ucProcIn1[16];                    /* Proc In 1 */
    uint8_t ucProcIn2[16];                    /* Proc In 2 */
    uint8_t ucProcIn3[16];                    /* Proc In 3 */
    /* 80 bytes above */
    uint8_t ucSdlImageSize[4];                /* Full size of the binary including header, payload etc */
    uint8_t ucSdlInstPlusRODataSize[4];       /* Size of the text section (instructions+RO.data) */
    uint8_t ucSdlModifiableDataSectionSize[4];/* Size of the data section (RW.data) */
    uint8_t ucSdlBssSectionSize[4];           /* Size of the BSS section */
    uint8_t ucSdlEntryPointVaddr[4];          /* Virtual address of entry point */
    uint8_t ucSdlId[2];                       /* SDL-ID */
    uint8_t ucObjectType;
    uint8_t ucSlotType;                       /* 0x00 for SDL TA-Extension in SRAM, 0x10,0x11 for SDL TA-extension in DRAM, 0x20 for SDL TA */
    /* 104 bytes above */
    uint8_t ucSignature2Short[128];            /* first 128 bytes of the Signature 2 (text-section) */
    uint8_t ucSignature3Short[128];            /* first 128 bytes of the Signature 3 (data-section) */
    uint8_t ucMetadata0Size[4];                /* Size of the first metadata section embedded in the SDL */
    uint8_t ucMetadata1Size[4];                /* Size of the second metadata section embedded in the SDL */
    uint8_t ucPlatformId[4];                   /* platform ID of the TA or to which TA the extension belong */
    /* 104 + 268 = 372 bytes above */

    uint8_t ucEpochRegionId;                   /* Epoch region ID */
    uint8_t ucReserved4[3];                    /* place holder for Epoch auto update mechanism */
    /* 372 + 4 = 376 */

    unsigned char ucPadding[628];
    /*  628 + 376 = 1004 bytes above */

    BSAGElib_SDLControllingParameters controlData;      /* 20 bytes */

    /* end of 1024 byte boundary */

    uint8_t ucHeaderSignature[256];            /* signature of all preceding fields in the header */

} BSAGElib_SDLHeader;


#endif /* BSAGELIB_SDL_HEADER_H__ */
