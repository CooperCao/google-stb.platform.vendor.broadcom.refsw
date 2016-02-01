/***************************************************************************
 *     (c)2012-2015 Broadcom Corporation
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
 **************************************************************************/
#include "nexus_audio_module.h"
#include "priv/nexus_pid_channel_priv.h"

BDBG_MODULE(nexus_audio_decoder_primer);

struct NEXUS_AudioDecoderPrimer
{
    NEXUS_OBJECT(NEXUS_AudioDecoderPrimer);
};

NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Open( NEXUS_AudioDecoderHandle audioDecoder )
{
    BSTD_UNUSED(audioDecoder);
    return NULL;
}
NEXUS_AudioDecoderPrimerHandle NEXUS_AudioDecoderPrimer_Create( const NEXUS_AudioDecoderOpenSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
    return NULL;
}
static void NEXUS_AudioDecoderPrimer_P_Finalizer( NEXUS_AudioDecoderPrimerHandle primer )
{
    BSTD_UNUSED(primer);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_AudioDecoderPrimer, NEXUS_AudioDecoderPrimer_Close);

void NEXUS_AudioDecoderPrimer_Flush( NEXUS_AudioDecoderPrimerHandle primer )
{
    BSTD_UNUSED(primer);
}

NEXUS_Error NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer( NEXUS_AudioDecoderPrimerHandle primer, NEXUS_AudioDecoderHandle audioDecoder )
{
    BSTD_UNUSED(primer);
    BSTD_UNUSED(audioDecoder);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_AudioDecoderPrimer_Start( NEXUS_AudioDecoderPrimerHandle primer, const NEXUS_AudioDecoderStartSettings *pStartSettings )
{
    BSTD_UNUSED(primer);
    BSTD_UNUSED(pStartSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_AudioDecoderPrimer_Stop( NEXUS_AudioDecoderPrimerHandle primer )
{
    BSTD_UNUSED(primer);
}

NEXUS_Error NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode( NEXUS_AudioDecoderPrimerHandle primer , NEXUS_AudioDecoderHandle audioDecoder )
{
    BSTD_UNUSED(primer);
    BSTD_UNUSED(audioDecoder);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_AudioDecoder_GetPrimerSettings( NEXUS_AudioDecoderPrimerHandle primer, NEXUS_AudioDecoderPrimerSettings *pSettings )
{
    BSTD_UNUSED(primer);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_AudioDecoder_SetPrimerSettings( NEXUS_AudioDecoderPrimerHandle primer, const NEXUS_AudioDecoderPrimerSettings *pSettings )
{
    BSTD_UNUSED(primer);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
void NEXUS_AudioDecoderPrimer_P_DecodeStopped(NEXUS_AudioDecoderPrimerHandle primer)
{
    BSTD_UNUSED(primer);
}
