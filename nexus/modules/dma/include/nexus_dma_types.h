/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Dma
*    Specific APIs related to memory to memory DMA.
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#ifndef NEXUS_DMA_TYPES_H__
#define NEXUS_DMA_TYPES_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Handle for the Dma Interface.

Description:

See Also:
NEXUS_Dma_Open
**/
typedef struct NEXUS_Dma *NEXUS_DmaHandle;

/***************************************************************************
Summary:
Selection of DMA core type
***************************************************************************/
typedef enum NEXUS_DmaCoreType
{
    NEXUS_DmaCoreType_eM2m,
    NEXUS_DmaCoreType_eSharf,
    NEXUS_DmaCoreType_eMax
} NEXUS_DmaCoreType;

/***************************************************************************
Summary:
DMA endian mode is used to align non-32bit-aligned data

Description:
Used in NEXUS_DmaSettings
***************************************************************************/
typedef enum NEXUS_DmaEndianMode
{
    NEXUS_DmaEndianMode_eLittle,
    NEXUS_DmaEndianMode_eBig,
    NEXUS_DmaEndianMode_eMax
} NEXUS_DmaEndianMode;

/***************************************************************************
Summary:
DMA byte swap mode

Description:

See Also:
NEXUS_DmaSettings
***************************************************************************/
typedef enum NEXUS_DmaSwapMode
{
    NEXUS_DmaSwapMode_eNone,
    NEXUS_DmaSwapMode_eWord,    /* Swap 16-bit words (0x00112233 -> 0x22330011)*/
    NEXUS_DmaSwapMode_eByte,    /* Swap 8-bit bytes (0x00112233 -> 0x33221100) */
    NEXUS_DmaSwapMode_eMax
} NEXUS_DmaSwapMode;

/***************************************************************************
Summary:
DMA data format -- used when encrypting/decrypting data

Description:

See Also:
NEXUS_DmaSettings
***************************************************************************/
typedef enum NEXUS_DmaDataFormat
{
    NEXUS_DmaDataFormat_eBlock, /* scramble all data as a generic block */
    NEXUS_DmaDataFormat_eMpeg,  /* only scramble the payload of 188-byte MPEG2-TS packets. 
                                   the header is transferred unscrambled. the header content is ignored */
    NEXUS_DmaDataFormat_eDss,   /* only scramble the payload of 130-byte DSS packets.
                                   the header is transferred unscrambled. the header content is ignored */
    NEXUS_DmaDataFormat_eMax
} NEXUS_DmaDataFormat;

/***************************************************************************
Summary:
SHARF mode

Description:
This enum is only used with NEXUS_DmaCoreType_eSharf.
***************************************************************************/
typedef enum NEXUS_DmaSharfMode
{
    NEXUS_DmaSharfMode_ePassThrough,
    NEXUS_DmaSharfMode_eSha1,
    NEXUS_DmaSharfMode_eAes128CbcDecrypt,
    NEXUS_DmaSharfMode_eAes128CbcEncrypt,
    NEXUS_DmaSharfMode_eCmac,
    NEXUS_DmaSharfMode_eAes128EcbDecrypt,
    NEXUS_DmaSharfMode_eAes128EcbEncrypt,
    NEXUS_DmaSharfMode_eMax
} NEXUS_DmaSharfMode;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_DMA_TYPES_H__ */

