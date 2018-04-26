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

#ifndef SUIF_TYPES_H__
#define SUIF_TYPES_H__

typedef enum {
    SUIF_MagicNumber_eSBL = 0x53424C21,
    SUIF_MagicNumber_eSSF = 0x53534621,
    SUIF_MagicNumber_eSDL = 0x53444C21,
    SUIF_MagicNumber_eSFC = 0x53464321,
    SUIF_MagicNumber_eTZM = 0x545A4D21,
    SUIF_MagicNumber_eTZK = 0x545A4B21,
    SUIF_MagicNumber_eTZA = 0x545A4121,
    SUIF_MagicNumber_eHVD = 0x48564421,
    SUIF_MagicNumber_eRGA = 0x52474121,
    SUIF_MagicNumber_eRVE = 0x52564521
} SUIF_MagicNumber_e;

typedef enum {
    SUIF_ImageType_eSBL = 0x01,
    SUIF_ImageType_eSSF = 0x02,
    SUIF_ImageType_eSDL = 0x03,
    SUIF_ImageType_eSFC = 0x04,
    SUIF_ImageType_eTZM = 0x05,
    SUIF_ImageType_eTZK = 0x06,
    SUIF_ImageType_eTZA = 0x07,
    SUIF_ImageType_eHVD = 0x08,
    SUIF_ImageType_eRGA = 0x09,
    SUIF_ImageType_eRVE = 0x0A
} SUIF_ImageType_e;

typedef enum {
    SUIF_Key0Select_eIndex = 0x43,
    SUIF_Key0Select_ePrime = 0x49,
    SUIF_Key0Select_eSearch = 0x90,
    SUIF_Key0Select_eIndex0 = 0xE0,
    SUIF_Key0Select_eIndex1 = 0xE1,
    SUIF_Key0Select_eIndex2 = 0xE2,
    SUIF_Key0Select_eIndex3 = 0xE3,
    SUIF_Key0Select_eIndex4 = 0xE4,
    SUIF_Key0Select_eIndex5 = 0xE5,
    SUIF_Key0Select_eIndex6 = 0xE6,
    SUIF_Key0Select_eIndex7 = 0xE7,
    SUIF_Key0Select_eIndex8 = 0xE8,
    SUIF_Key0Select_eIndex9 = 0xE9
} SUIF_Key0Select_e;

typedef enum {
    SUIF_SigningScheme_eNormal = 0x1,
    SUIF_SigningScheme_eTriple = 0x2
} SUIF_SigningScheme_e;

typedef enum {
    SUIF_MultiTierKeys_e2nd = 0x1,        /* 2nd Tier Key only */
    SUIF_MultiTierKeys_e2nd3rd = 0x2,     /* 2nd and 3rd Tier Keys */
    SUIF_MultiTierKeys_e2nd3rd4th = 0x3  /* 2nd and 3rd and 4th Tier Keys */
} SUIF_MultiTierKeys_e;

typedef enum {
    SUIF_SecureElementType_eBSP = 0x1,
    SUIF_SecureElementType_eVPU = 0x2
} SUIF_SecureElementType_e;

typedef enum {
    SUIF_SecureElementVersion_eZeus42 = 0x1,
    SUIF_SecureElementVersion_eZeus50 = 0x2,
    SUIF_SecureElementVersion_eZeus51 = 0x3
} SUIF_SecureElementVersion_e;

/******************************************************************
 * SUIF versioning
 ******************************************************************/
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
    uint8_t branch;
} SUIF_Version;


#endif /* SUIF_TYPES_H__ */
