/***************************************************************************
 *     (c)2007-2008 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/
#ifndef NEXUS_SPDIF_TYPES_H__
#define NEXUS_SPDIF_TYPES_H__

/*=***********************************
SPDIF types
*************************************/

#include "nexus_types.h"
#include "nexus_audio_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
An enum of the channel status clock accuracies.
**************************************************************************/
#define NEXUS_SPDIF_CHAN_STAT3_FS_44100 0x0
#define NEXUS_SPDIF_CHAN_STAT3_FS_Unspecified 0x1
#define NEXUS_SPDIF_CHAN_STAT3_FS_48000 0x2
#define NEXUS_SPDIF_CHAN_STAT3_FS_32000 0x3
#define NEXUS_SPDIF_CHAN_STAT3_FS_22050 0x4
#define NEXUS_SPDIF_CHAN_STAT3_FS_24000 0x6
#define NEXUS_SPDIF_CHAN_STAT3_FS_88200 0x8
#define NEXUS_SPDIF_CHAN_STAT3_FS_96000 0xa
#define NEXUS_SPDIF_CHAN_STAT3_FS_176400 0xc
#define NEXUS_SPDIF_CHAN_STAT3_FS_192000 0xe

typedef enum NEXUS_SpdifSamplingFrequency
{
    NEXUS_SpdifSamplingFrequency_e44100 = NEXUS_SPDIF_CHAN_STAT3_FS_44100,
    NEXUS_SpdifSamplingFrequency_eUnspecified = NEXUS_SPDIF_CHAN_STAT3_FS_Unspecified,
    NEXUS_SpdifSamplingFrequency_e48000 = NEXUS_SPDIF_CHAN_STAT3_FS_48000,
    NEXUS_SpdifSamplingFrequency_e32000 = NEXUS_SPDIF_CHAN_STAT3_FS_32000,
    NEXUS_SpdifSamplingFrequency_e22050 = NEXUS_SPDIF_CHAN_STAT3_FS_22050,
    NEXUS_SpdifSamplingFrequency_e24000 = NEXUS_SPDIF_CHAN_STAT3_FS_24000,
    NEXUS_SpdifSamplingFrequency_e88200 = NEXUS_SPDIF_CHAN_STAT3_FS_88200,
    NEXUS_SpdifSamplingFrequency_e96000 = NEXUS_SPDIF_CHAN_STAT3_FS_96000,
    NEXUS_SpdifSamplingFrequency_e176400 = NEXUS_SPDIF_CHAN_STAT3_FS_176400,
    NEXUS_SpdifSamplingFrequency_e192000 = NEXUS_SPDIF_CHAN_STAT3_FS_192000
} NEXUS_SpdifSamplingFrequency;

/***************************************************************************
Summary:
This enumeration defines the stream type of incoming digaud data
***************************************************************************/
#define NEXUS_SPDIF_CHAN_STAT0_STREAM_TYPE_Linear 0
#define NEXUS_SPDIF_CHAN_STAT0_STREAM_TYPE_Nonlinear 1

typedef enum NEXUS_SpdifStreamType
{
    NEXUS_SpdifStreamType_eLinear = NEXUS_SPDIF_CHAN_STAT0_STREAM_TYPE_Linear,
    NEXUS_SpdifStreamType_eNonlinear = NEXUS_SPDIF_CHAN_STAT0_STREAM_TYPE_Nonlinear
} NEXUS_SpdifStreamType;

/***************************************************************************
Summary:
An enum of the channel status clock accuracies.
**************************************************************************/
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_20_BITS 0x0
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_16_OF_20_BITS 0x2
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_17_OF_20_BITS 0xc
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_18_OF_20_BITS 0x4
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_19_OF_20_BITS 0x8
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_20_BITS 0xa
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_24_BITS 0x1
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_24_BITS 0x3
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_21_OF_24_BITS 0xd
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_22_OF_24_BITS 0x5
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_23_OF_24_BITS 0x9
#define NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_24_OF_24_BITS 0xb

typedef enum NEXUS_SpdifWordLength
{
    NEXUS_SpdifWordLength_eUnspecifiedOf20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_20_BITS,
    NEXUS_SpdifWordLength_e16Of20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_16_OF_20_BITS,
    NEXUS_SpdifWordLength_e17Of20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_17_OF_20_BITS,
    NEXUS_SpdifWordLength_e18Of20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_18_OF_20_BITS,
    NEXUS_SpdifWordLength_e19Of20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_19_OF_20_BITS,
    NEXUS_SpdifWordLength_e20Of20 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_20_BITS,
    NEXUS_SpdifWordLength_eUnspecifiedOf24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_UNSPECIFIED_OF_24_BITS,
    NEXUS_SpdifWordLength_e20Of24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_20_OF_24_BITS,
    NEXUS_SpdifWordLength_e21Of24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_21_OF_24_BITS,
    NEXUS_SpdifWordLength_e22Of24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_22_OF_24_BITS,
    NEXUS_SpdifWordLength_e23Of24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_23_OF_24_BITS,
    NEXUS_SpdifWordLength_e24Of24 = NEXUS_SPDIF_CHAN_STAT4_WORD_LEN_24_OF_24_BITS
} NEXUS_SpdifWordLength;

#ifdef __cplusplus
}
#endif

#endif
