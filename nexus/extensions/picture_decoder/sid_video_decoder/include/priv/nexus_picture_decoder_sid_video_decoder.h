/***************************************************************************
 *     (c)2007-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  .  Except as set forth in an Authorized License, Broadcom grants
 *  no license , right to use, or waiver of any kind with respect to the
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
 *  LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR  ANY AMOUNT IN EXCESS OF THE AMOUNT
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
#ifndef NEXUS_PICTURE_DECODER_SID_VIDEO_DECODER_H__
#define NEXUS_PICTURE_DECODER_SID_VIDEO_DECODER_H__

#include "priv/nexus_rave_priv.h"
#include "bxdm_decoder.h"

typedef struct NEXUS_SidVideoDecoder *NEXUS_SidVideoDecoderHandle;

typedef struct NEXUS_SidVideoDecoderOpenSettings
{
    NEXUS_HeapHandle pictureHeap;
} NEXUS_SidVideoDecoderOpenSettings;

typedef struct NEXUS_SidVideoDecoderStartSettings
{
    NEXUS_RaveHandle raveContext;
    NEXUS_VideoCodec codec;             /* The type of video being decoded. */
} NEXUS_SidVideoDecoderStartSettings;

void NEXUS_SidVideoDecoder_GetDefaultStartSettings_priv( NEXUS_SidVideoDecoderStartSettings *pSettings );
void NEXUS_SidVideoDecoder_GetDefaultOpenSettings_priv( NEXUS_SidVideoDecoderOpenSettings *pSettings );
NEXUS_SidVideoDecoderHandle NEXUS_SidVideoDecoder_Open_priv(unsigned index, const NEXUS_SidVideoDecoderOpenSettings *openSettings);
void NEXUS_SidVideoDecoder_Close_priv(NEXUS_SidVideoDecoderHandle decoder);
NEXUS_Error NEXUS_SidVideoDecoder_Start_priv(NEXUS_SidVideoDecoderHandle decoder, const NEXUS_SidVideoDecoderStartSettings *startSettings);
void NEXUS_SidVideoDecoder_Stop_priv(NEXUS_SidVideoDecoderHandle decoder);
NEXUS_Error NEXUS_SidVideoDecoder_Flush_priv(NEXUS_SidVideoDecoderHandle decoder);
void NEXUS_SidVideoDecoder_GetDecoderInterface_priv(NEXUS_SidVideoDecoderHandle decoder, const BXDM_Decoder_Interface **decoderInterface, void **decoderContext);
void NEXUS_SidVideoDecoder_GetRaveSettings_priv(NEXUS_RaveOpenSettings *raveSettings);

#endif /* NEXUS_PICTURE_DECODER_SID_VIDEO_DECODER_H__ */

