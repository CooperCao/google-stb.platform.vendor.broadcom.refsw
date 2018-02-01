/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bgrclib_packet.h"
#include "bstd.h"
#include "berr.h"
#include "berr_ids.h"
#include "bdbg.h"
#include "bkni.h"

#include "bm2mc_packet.h"
#include "bgrc_packet.h"
#include "bgrc_packet_write.h"

BDBG_MODULE(BGRClib);

BDBG_OBJECT_ID(GRClib);

typedef struct BGRClib_P_Data_tag
{
    BDBG_OBJECT(GRClib)
    BGRC_Handle hGRC;
    BGRC_PacketContext_Handle hContext;
    bool defaultRopPacket;
    bool defaultSourceColorkeyPacket;
    bool defaultDstColorkeyPacket;
    bool defaultBlitBlendPacket;
    bool defaultFillBlendPacket;
    bool defaultSourceFeederPacket;
    bool defaultDestinationFeederPacket;
    bool defaultColorMatrixPacket;
    bool defaultAlphaPremultiplyPacket;
    bool defaultMirrorPacket;
    bool defaultFixedScalePacket;
    struct {
        BGRC_FilterCoeffs vertCoeffs, horzCoeffs;
        struct {
            uint16_t width, height;
        } src, out;
    } filterPacket;
    BM2MC_PACKET_eFilterOrder sourceFilterOrder; /* order of scale, color key and color matrix applied to source */
}
BGRClib_P_Data;

#define DEFAULT_COLOR (0xFF000000)


/* for maximum perfornace, even in debug mode:
- do not use BDBG_ENTER/LEAVE. there are other ways to trace calls and we don't want to risk DBG overhead.
- do not BDBG_ASSERT on pointers. it's a marginal check anyway, and if it's NULL we're going to crash anyway.
*/

/*****************************************************************************/
static const BM2MC_PACKET_Blend g_aGRClib_Packet_ColorEquation_FillOp[BGRCLib_FillOp_eCount] = {
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0,BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_AlphaEquation_FillOp[BGRCLib_FillOp_eCount] = {
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_ColorEquation_BlitOp[BGRCLib_BlitColorOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_AlphaEquation_BlitOp[BGRCLib_BlitAlphaOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, BM2MC_PACKET_BlendFactor_eDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_ColorEquation_PorterDuffFillOp[BGRCLib_PorterDuffOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0,  BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_AlphaEquation_PorterDuffFillOp[BGRCLib_PorterDuffOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha,   BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseConstantAlpha, 0,  BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_ColorEquation_PorterDuffBlitOp[BGRCLib_PorterDuffOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero }
};
static const BM2MC_PACKET_Blend g_aGRClib_Packet_AlphaEquation_PorterDuffBlitOp[BGRCLib_PorterDuffOp_Count] = {
    { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, 0, BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, 0, BM2MC_PACKET_BlendFactor_eZero },
    { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eInverseDestinationAlpha, 0, BM2MC_PACKET_BlendFactor_eDestinationAlpha, BM2MC_PACKET_BlendFactor_eInverseSourceAlpha, 0, BM2MC_PACKET_BlendFactor_eZero }
};

/*****************************************************************************/
#define GRCLIB_FULL_PACKET_SIZE (2 * \
    sizeof (BM2MC_PACKET_PacketSourceFeeder) + \
    sizeof (BM2MC_PACKET_PacketDestinationFeeder) + \
    sizeof (BM2MC_PACKET_PacketOutputFeeder) + \
    sizeof (BM2MC_PACKET_PacketBlend) + \
    sizeof (BM2MC_PACKET_PacketRop) + \
    sizeof (BM2MC_PACKET_PacketSourceColorkey) * 2 + \
    sizeof (BM2MC_PACKET_PacketSourceColorkeyEnable) * 2 + \
    sizeof (BM2MC_PACKET_PacketFilter) + \
    sizeof (BM2MC_PACKET_PacketFilterEnable) + \
    sizeof (BM2MC_PACKET_PacketSourceColorMatrix) + \
    sizeof (BM2MC_PACKET_PacketSourceColorMatrixEnable) + \
    sizeof (BM2MC_PACKET_PacketSourcePalette) + \
    sizeof (BM2MC_PACKET_PacketAlphaPremultiply) + \
    sizeof (BM2MC_PACKET_PacketMirror) + \
    sizeof (BM2MC_PACKET_PacketDestripeBlit))

/*****************************************************************************/
static void BGRClib_P_GetPacketRectangle(
    const BM2MC_PACKET_Plane *pSurface,
    const BRect *pInRect,
    BM2MC_PACKET_Rectangle *pOutRect )
{
    if( pInRect && pInRect->width && pInRect->height) {
        pOutRect->x = pInRect->x;
        pOutRect->y = pInRect->y;
        pOutRect->width = pInRect->width;
        pOutRect->height = pInRect->height;
    } else {
        pOutRect->x = 0;
        pOutRect->y = 0;
        if ( pSurface ) {
            pOutRect->width = pSurface->width;
            pOutRect->height = pSurface->height;
        } else {
            pOutRect->width = 0;
            pOutRect->height = 0;
        }
    }
}

/*****************************************************************************/
static void BGRClib_P_GetStripedPlanePacketRectangle(
    const BM2MC_PACKET_StripedPlane *pSurface,
    const BRect *pInRect,
    BM2MC_PACKET_Rectangle *pOutRect )
{
    if( pInRect && pInRect->width && pInRect->height)
    {
        pOutRect->x = pInRect->x;
        pOutRect->y = pInRect->y;
        pOutRect->width = pInRect->width;
        pOutRect->height = pInRect->height;
    }
    else {
        pOutRect->x = 0;
        pOutRect->y = 0;
        if ( pSurface ) {
            pOutRect->width = pSurface->width;
            pOutRect->height = pSurface->height;
        }
        else {
            pOutRect->width = 0;
            pOutRect->height = 0;
        }
    }
}

/*****************************************************************************/
static void BGRClib_P_GetBlendEquation( BM2MC_PACKET_Blend *pEquation, const BGRClib_BlendEquation *pBlend )
{
    if( pEquation && pBlend )
    {
        pEquation->a = pBlend->a;
        pEquation->b = pBlend->b;
        pEquation->sub_cd = pBlend->subcd;
        pEquation->c = pBlend->c;
        pEquation->d = pBlend->d;
        pEquation->sub_e = pBlend->sube;
        pEquation->e = pBlend->e;
    }
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultSourceFeederPacket(grclib, pPacket) \
    ((grclib)->defaultSourceFeederPacket?(pPacket):BGRClib_P_SetSourceFeederPacket( (grclib), (pPacket), 0, DEFAULT_COLOR))

static void *BGRClib_P_SetSourceFeederPacket( BGRClib_Handle grclib,
    void *pPacket, const BM2MC_PACKET_Plane *pSrcSurface, uint32_t color )
{
    grclib->defaultSourceFeederPacket = (!pSrcSurface && color == DEFAULT_COLOR);
    BGRC_Packet_SetSourcePlanePacket( grclib->hGRC, &pPacket, pSrcSurface, 0, 0, color );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetStripedSourceFeederPacket( BGRClib_Handle grclib,
    void *pPacket, const BM2MC_PACKET_StripedPlane *pSrcPlane, uint32_t color )
{
    grclib->defaultSourceFeederPacket = false;
    BGRC_Packet_SetStripedSourcePlanePacket( grclib->hGRC, &pPacket, pSrcPlane, color);
    return pPacket;
}

#define BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket) \
    ((grclib)->defaultDestinationFeederPacket?(pPacket):BGRClib_P_SetDestinationFeederPacket( (grclib), (pPacket), NULL, DEFAULT_COLOR))

static void *BGRClib_P_SetDestinationFeederPacket( BGRClib_Handle grclib,
    void *pPacket, const BM2MC_PACKET_Plane *pSurface, uint32_t color )
{
    grclib->defaultDestinationFeederPacket = (!pSurface && color == DEFAULT_COLOR);
    BGRC_Packet_SetDestinationPlanePacket( grclib->hGRC, &pPacket, pSurface, color );
    return pPacket;
}

/* the only way to cache this without comparing everything is something like BSUR's SurfaceId to know if the surface has been recreated. */
static void *BGRClib_P_SetOutputFeederPacket( BGRClib_Handle grclib,
    void *pPacket, const BM2MC_PACKET_Plane *pSurface )
{
    BGRC_Packet_SetOutputPlanePacket( grclib->hGRC, &pPacket, pSurface );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultFillBlendPacket(grclib, pPacket) \
    ((grclib)->defaultFillBlendPacket?(pPacket):BGRClib_P_SetFillBlendPacket( (grclib), (pPacket), BGRCLib_FillOp_eCopy, BGRCLib_FillOp_eCopy, 0 ))

static void *BGRClib_P_SetFillBlendPacket( BGRClib_Handle grclib, void *pPacket,
    BGRCLib_FillOp colorOp, BGRCLib_FillOp alphaOp, uint32_t color )
{
    grclib->defaultBlitBlendPacket = false;
    grclib->defaultFillBlendPacket = (colorOp == BGRCLib_FillOp_eCopy && alphaOp == BGRCLib_FillOp_eCopy && !color);

    BGRC_Packet_SetBlendPacket( grclib->hGRC, &pPacket,
        &g_aGRClib_Packet_ColorEquation_FillOp[colorOp], &g_aGRClib_Packet_AlphaEquation_FillOp[alphaOp], color );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultBlitBlendPacket(grclib, pPacket) \
    ((grclib)->defaultBlitBlendPacket?(pPacket):BGRClib_P_SetBlitBlendPacket( (grclib), (pPacket), BGRCLib_BlitColorOp_eCopySource, BGRCLib_BlitAlphaOp_eCopySource, 0, 0, 0 ))

static void *BGRClib_P_SetBlitBlendPacket( BGRClib_Handle grclib,
    void *pPacket, BGRCLib_BlitColorOp colorOp, BGRCLib_BlitAlphaOp alphaOp,
    const BGRClib_BlendEquation *pColorBlend, const BGRClib_BlendEquation *pAlphaBlend, uint32_t color )
{
    BM2MC_PACKET_Blend colorEquation;
    BM2MC_PACKET_Blend alphaEquation;

    /* if both ops are eCopySource, the other params are ignored */
    grclib->defaultBlitBlendPacket = (colorOp == BGRCLib_BlitColorOp_eCopySource && alphaOp == BGRCLib_BlitAlphaOp_eCopySource);
    grclib->defaultFillBlendPacket = false;

    if( colorOp == BGRCLib_BlitColorOp_eUseBlendFactors )
        BGRClib_P_GetBlendEquation( &colorEquation, pColorBlend );
    if( alphaOp == BGRCLib_BlitAlphaOp_eUseBlendFactors )
        BGRClib_P_GetBlendEquation( &alphaEquation, pAlphaBlend );

    BGRC_Packet_SetBlendPacket( grclib->hGRC, &pPacket,
        (colorOp == BGRCLib_BlitColorOp_eUseBlendFactors) ? &colorEquation : &g_aGRClib_Packet_ColorEquation_BlitOp[colorOp],
        (alphaOp == BGRCLib_BlitAlphaOp_eUseBlendFactors) ? &alphaEquation : &g_aGRClib_Packet_AlphaEquation_BlitOp[alphaOp], color );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetPorterDuffFillBlendPacket( BGRClib_Handle grclib, void *pPacket,
    BGRCLib_PorterDuffOp pdOp, uint32_t color )
{
    /* no grclib defaults */
    grclib->defaultBlitBlendPacket = false;
    grclib->defaultFillBlendPacket = false;

    BGRC_Packet_SetBlendPacket( grclib->hGRC, &pPacket,
        &g_aGRClib_Packet_ColorEquation_PorterDuffFillOp[pdOp], &g_aGRClib_Packet_AlphaEquation_PorterDuffFillOp[pdOp], color );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetPorterDuffBlitBlendPacket( BGRClib_Handle grclib, void *pPacket,
    BGRCLib_PorterDuffOp pdOp, uint32_t color )
{
    /* no grclib defaults */
    grclib->defaultBlitBlendPacket = false;
    grclib->defaultFillBlendPacket = false;

    BGRC_Packet_SetBlendPacket( grclib->hGRC, &pPacket,
        &g_aGRClib_Packet_ColorEquation_PorterDuffBlitOp[pdOp], &g_aGRClib_Packet_AlphaEquation_PorterDuffBlitOp[pdOp], color );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultRopPacket(grclib, pPacket) \
    ((grclib)->defaultRopPacket?(pPacket):BGRClib_P_SetRopPacket( (grclib), (pPacket), 0xCC, NULL, 0, 0))

static void *BGRClib_P_SetRopPacket( BGRClib_Handle grclib, void *pPacket,
    uint8_t rop, uint32_t* pattern, uint32_t color0, uint32_t color1 )
{
    grclib->defaultRopPacket = (rop == 0xCC && pattern == NULL && color0==0 && color1==0);
    BGRC_Packet_SetRopPacket( grclib->hGRC, &pPacket, rop, pattern, color0, color1 );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket) \
    ((grclib)->defaultSourceColorkeyPacket?(pPacket):BGRClib_P_SetSourceColorkeyPacket( (grclib), (pPacket), false, 0, 0, 0, 0, 0))

static void *BGRClib_P_SetSourceColorkeyPacket( BGRClib_Handle grclib, void *pPacket,
    bool enable, uint32_t high, uint32_t low, uint32_t mask, uint32_t replacement, uint32_t replacement_mask )
{
    grclib->defaultSourceColorkeyPacket = (!enable && !high && !low && !mask && !replacement && !replacement_mask);
    BGRC_Packet_SetSourceColorkeyPacket( grclib->hGRC, &pPacket, enable,
        high, low, mask, replacement, replacement_mask );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket) \
    ((grclib)->defaultDstColorkeyPacket?(pPacket):BGRClib_P_SetDestinationColorkeyPacket( (grclib), (pPacket), false, 0, 0, 0, 0, 0))

static void *BGRClib_P_SetDestinationColorkeyPacket( BGRClib_Handle grclib, void *pPacket,
    bool enable, uint32_t high, uint32_t low, uint32_t mask, uint32_t replacement, uint32_t replacement_mask )
{
    grclib->defaultDstColorkeyPacket = (!enable && !high && !low && !mask && !replacement && !replacement_mask);
    BGRC_Packet_SetDestinationColorkeyPacket( grclib->hGRC, &pPacket, enable,
        high, low, mask, replacement, replacement_mask );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetDestinationControlPacket( BGRClib_Handle grclib, void *pPacket, bool zeroPad, bool chromaFilter )
{
    BGRC_Packet_SetDestinationControl( grclib->hGRC, &pPacket, zeroPad, chromaFilter );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetFilterPacket( BGRClib_Handle grclib, void *pPacket,
    BGRC_FilterCoeffs horz, BGRC_FilterCoeffs vert, BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect )
{
    /* no default value, so we cache values. the src/out rects are used to determine changes in scaling factor */
    if (grclib->filterPacket.vertCoeffs != vert ||
        grclib->filterPacket.horzCoeffs != horz ||
        grclib->filterPacket.src.width != pSrcRect->width ||
        grclib->filterPacket.src.height != pSrcRect->height ||
        grclib->filterPacket.out.width != pOutRect->width ||
        grclib->filterPacket.out.height != pOutRect->height)
    {
        BGRC_Packet_SetFilterPacket( grclib->hGRC, &pPacket, horz, vert, pSrcRect, pOutRect );

        grclib->filterPacket.vertCoeffs = vert;
        grclib->filterPacket.horzCoeffs = horz;
        grclib->filterPacket.src.width = pSrcRect->width;
        grclib->filterPacket.src.height = pSrcRect->height;
        grclib->filterPacket.out.width = pOutRect->width;
        grclib->filterPacket.out.height = pOutRect->height;
    }
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket) \
    ((grclib)->defaultColorMatrixPacket?(pPacket):BGRClib_P_SetColorMatrixPacket( (grclib), (pPacket), NULL, 0))

static void *BGRClib_P_SetColorMatrixPacket( BGRClib_Handle grclib,
    void *pPacket, const int32_t matrix[], uint32_t shift )
{
    grclib->defaultColorMatrixPacket = (!matrix && !shift);
    BGRC_Packet_SetColorMatrixPacket( grclib->hGRC, &pPacket, matrix, shift );
    return pPacket;
}

#define BM2MC_IS_PALETTE_FORMAT(FORMAT) \
    ((FORMAT)>=BM2MC_PACKET_PixelFormat_eA8_P8 && (FORMAT)<=BM2MC_PACKET_PixelFormat_eY8_P8)

/*****************************************************************************/
static void *BGRClib_P_SetSourcePalette(BGRClib_Handle grclib, void *pPacket, unsigned paletteOffset)
{
    BGRC_Packet_SetSourcePalette( grclib->hGRC, &pPacket, (uint64_t) paletteOffset );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket) \
    ((grclib)->defaultAlphaPremultiplyPacket?(pPacket):BGRClib_P_SetAlphaPremultiplyPacket( (grclib), (pPacket), false))
static void *BGRClib_P_SetAlphaPremultiplyPacket( BGRClib_Handle grclib, void *pPacket, bool enable )
{
    grclib->defaultAlphaPremultiplyPacket = (!enable);
    BGRC_Packet_SetAlphaPremultiply( grclib->hGRC, &pPacket, enable );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetSourceFilterOrderPacket( BGRClib_Handle grclib, void *pPacket, BM2MC_PACKET_eFilterOrder sourceFilterOrder )
{
    grclib->sourceFilterOrder = sourceFilterOrder;
    BGRC_Packet_SetFilterControlPacket( grclib->hGRC, &pPacket, false, false, false, sourceFilterOrder );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetSourceControlPacket( BGRClib_Handle grclib, void *pPacket, bool zeroPad, bool chromaFilter, bool linearDestripe )
{
    BGRC_Packet_SetSourceControl( grclib->hGRC, &pPacket, zeroPad, chromaFilter, linearDestripe );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetOutputControlPacket( BGRClib_Handle grclib, void *pPacket, bool dither, bool chromaFilter)
{
    BGRC_Packet_SetOutputControl( grclib->hGRC, &pPacket, dither, chromaFilter);
    return pPacket;
}

static void *BGRClib_P_SetMipmapPacket( BGRClib_Handle grclib, void *pPacket, uint32_t mipLevel)
{
    BGRC_Packet_SetMipmapControl( grclib->hGRC, &pPacket, mipLevel);
    return pPacket;
}


/*****************************************************************************/
#define BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket) \
    ((grclib)->defaultMirrorPacket?(pPacket):BGRClib_P_SetMirrorPacket( (grclib), (pPacket), false, false, false, false, false, false))
static void *BGRClib_P_SetMirrorPacket( BGRClib_Handle grclib, void *pPacket,
    uint16_t srcHorz, uint16_t srcVert, uint16_t dstHorz, uint16_t dstVert, uint16_t outHorz, uint16_t outVert )
{
    grclib->defaultMirrorPacket = (!srcHorz && !srcVert && !dstHorz && !dstVert && !outHorz && !outVert);
    BGRC_Packet_SetMirrorPacket( grclib->hGRC, &pPacket,
        0!=srcHorz, 0!=srcVert, 0!=dstHorz, 0!=dstVert, 0!=outHorz, 0!=outVert );
    return pPacket;
}

/*****************************************************************************/
#define BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket) \
    ((grclib)->defaultFixedScalePacket?(pPacket):BGRClib_P_SetFixedScalePacket( (grclib), (pPacket), NULL))
static void *BGRClib_P_SetFixedScalePacket( BGRClib_Handle grclib, void *pPacket, const BGRClib_BlitScalingControlParams *params )
{
#if 1
    /* the following usage of fixed_scale is going to be obsolete */
    grclib->defaultFixedScalePacket = (!params);
    if( params ) {
        BM2MC_PACKET_PacketFixedScale *pScale = (BM2MC_PACKET_PacketFixedScale *) pPacket;
        BM2MC_PACKET_INIT( pScale, FixedScale, false );
        pScale->shift = 20;
        pScale->hor_phase = params->iHorizontalPhase << (pScale->shift - params->ulPhaseFixedPointShift);
        pScale->ver_phase = params->iVerticalPhase << (pScale->shift - params->ulPhaseFixedPointShift);
        if (params->ulHorizontalDenominator) {
            pScale->hor_step = (params->ulHorizontalNumerator << pScale->shift) / params->ulHorizontalDenominator;
        }
        else {
            pScale->hor_step = 0;
        }
        if (params->ulVerticalDenominator) {
            pScale->ver_step = (params->ulVerticalNumerator << pScale->shift) / params->ulVerticalDenominator;
        }
        else {
            pScale->ver_step = 0;
        }
        BM2MC_PACKET_TERM( pScale );
        pPacket = pScale;
    }
    else {
        BM2MC_PACKET_PacketFixedScale *pScale = (BM2MC_PACKET_PacketFixedScale *) pPacket;
        BM2MC_PACKET_INIT( pScale, FixedScale, false );
        pScale->hor_phase = 0;
        pScale->ver_phase = 0;
        pScale->hor_step = 0;
        pScale->ver_step = 0;
        pScale->shift = 0;
        BM2MC_PACKET_TERM( pScale );
        pPacket = pScale;
    }
#else
    BSTD_UNUSED(grclib);
    BSTD_UNUSED(params);
#endif
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetFillPacket( BGRClib_Handle grclib,
    void *pPacket, BM2MC_PACKET_Rectangle *pRect )
{
    BGRC_Packet_SetBlitPacket( grclib->hGRC, &pPacket, pRect, NULL, NULL );
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetBlitPacket( BGRClib_Handle grclib, void *pPacket,
    BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pDstRect, BM2MC_PACKET_Rectangle *pOutRect,
    BGRC_FilterCoeffs horz, BGRC_FilterCoeffs vert )
{
    if( (pSrcRect->width == pOutRect->width) && (pSrcRect->height == pOutRect->height) &&
        (horz <= BGRC_FilterCoeffs_eAnisotropic) && (vert <= BGRC_FilterCoeffs_eAnisotropic) ) {
        BGRC_Packet_SetBlitPacket( grclib->hGRC, &pPacket,
            pSrcRect, (BM2MC_PACKET_Point *) pOutRect, (BM2MC_PACKET_Point *) pDstRect );
    }
    else {
        BGRC_Packet_SetScaleBlitPacket( grclib->hGRC, &pPacket,
            pSrcRect, pOutRect, (BM2MC_PACKET_Point *) pDstRect );
    }
    return pPacket;
}

/*****************************************************************************/
static void *BGRClib_P_SetDestripeBlitPacket( BGRClib_Handle grclib, void *pPacket,
    BM2MC_PACKET_Rectangle *pSrcRect, BM2MC_PACKET_Rectangle *pOutRect )
{
    BGRC_Packet_SetScaleBlitPacket( grclib->hGRC, &pPacket, pSrcRect, pOutRect, NULL /*(BM2MC_PACKET_Point *) pOutRect*/);
    return pPacket;
}

/*****************************************************************************/
static BERR_Code BGRClib_P_PacketsComplete( BGRClib_Handle grclib, void *end, void *start )
{
    BERR_Code err;
    /* coverity[overflow_assign] - coverity sees 64 bit pointer math could overflow 32 bit integer size,
    but this should never happen here */
    err = BGRC_Packet_SubmitPackets( grclib->hGRC, grclib->hContext, (uint8_t*)end - (uint8_t*)start );
    return (err == BGRC_PACKET_MSG_PACKETS_INCOMPLETE) ? BERR_SUCCESS : err;
}

/*****************************************************************************/
static void *BGRClib_P_GetPacketBuffer( BGRClib_Handle grclib, unsigned size )
{
    void *pPacket;
    size_t iAllocSize = 0;

    BERR_Code err = BGRC_Packet_GetPacketMemory( grclib->hGRC, grclib->hContext, &pPacket, &iAllocSize, size );
    if( err != BERR_SUCCESS )
        return NULL;

    return pPacket;
}

/***************************************************************************/
BERR_Code BGRClib_Open( BGRClib_Handle *pgrclib, BGRC_Handle grcHandle, BGRC_PacketContext_Handle packetContext )
{
    BGRClib_Handle grclib;
    BERR_Code err = BERR_SUCCESS;

    BDBG_ASSERT(pgrclib);
    BDBG_ASSERT(grcHandle);

    grclib = (BGRClib_Handle) BKNI_Malloc( sizeof (BGRClib_P_Data) );
    if( !grclib )
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

    BKNI_Memset( grclib, 0, sizeof(BGRClib_P_Data) );
    BDBG_OBJECT_SET(grclib, GRClib);

    grclib->hGRC = grcHandle;
    grclib->hContext = packetContext;
    *pgrclib = grclib;
    grclib->filterPacket.vertCoeffs = (BGRC_FilterCoeffs)-1;
    grclib->filterPacket.horzCoeffs = (BGRC_FilterCoeffs)-1;
    grclib->sourceFilterOrder = BM2MC_PACKET_eFilterOrder_FilterColorkeyMatrix;

    return BERR_TRACE(err);
}

/***************************************************************************/
void BGRClib_Close( BGRClib_Handle grclib )
{
    BDBG_OBJECT_ASSERT(grclib, GRClib);
    BDBG_OBJECT_DESTROY(grclib, GRClib);
    BKNI_Free( (void*)grclib );
}

/*****************************************************************************/
BERR_Code BGRClib_Memset32( BGRClib_Handle grclib, uint32_t offset, uint32_t data, uint32_t count )
{
    BERR_Code err = BERR_SUCCESS;
    BM2MC_PACKET_Rectangle outRect = { 0, 0, 0, 0 };
    uint32_t pitch = 4096;
    void *pPacket;
    void *pPacketStart;
    BM2MC_PACKET_Plane stSurface;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    outRect.width = (uint16_t) (pitch / 4);
    outRect.height = (uint16_t) (count * 4 / pitch);

    stSurface.address = offset;
    stSurface.pitch = pitch;
    stSurface.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
    stSurface.width = outRect.width;
    stSurface.height = outRect.height;

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ) {
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    pPacket = BGRClib_P_SetDefaultSourceFeederPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, &stSurface );

    pPacket = BGRClib_P_SetFillBlendPacket( grclib, pPacket, BGRCLib_FillOp_eCopy, BGRCLib_FillOp_eCopy, data );
    pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    pPacket = BGRClib_P_SetFillPacket( grclib, pPacket, &outRect );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/*****************************************************************************/
void BGRClib_GetDefaultDestripeBlitParams( BGRClib_DestripeBlitParams *params )
{
    BKNI_Memset( params, 0, sizeof(BGRClib_DestripeBlitParams) );
    /* no valid default for srcSurface, outSurface and outRect */
    params->horzFilter = BGRC_FilterCoeffs_eAnisotropic;
    params->vertFilter = BGRC_FilterCoeffs_eAnisotropic;
    params->chromaFilter = true;
}

/*****************************************************************************/
BERR_Code BGRClib_Destripe_Blit( BGRClib_Handle grclib, const BGRClib_DestripeBlitParams* params)
{
    BERR_Code err = BERR_SUCCESS;
    BM2MC_PACKET_Rectangle srcRect;
    BM2MC_PACKET_Rectangle outRect;
    void *pPacket;
    void *pPacketStart;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    BGRClib_P_GetStripedPlanePacketRectangle( params->srcSurface, params->srcRect, &srcRect );
    BGRClib_P_GetPacketRectangle( params->outSurface, params->outRect, &outRect );

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ) {
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    pPacket = BGRClib_P_SetStripedSourceFeederPacket( grclib, pPacket, params->srcSurface, DEFAULT_COLOR );
    pPacket = BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, params->outSurface );

    pPacket = BGRClib_P_SetDefaultBlitBlendPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetFilterPacket( grclib, pPacket,
        params->horzFilter, params->vertFilter, &srcRect, &outRect );

    if( params->matrixParams.conversionMatrix )
        pPacket = BGRClib_P_SetColorMatrixPacket( grclib, pPacket, params->matrixParams.conversionMatrix, params->matrixParams.matrixShift );
    else
        pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    pPacket = BGRClib_P_SetSourceControlPacket(grclib, pPacket, false, params->chromaFilter, false);
    pPacket = BGRClib_P_SetMipmapPacket(grclib, pPacket, params->miplevel);
    pPacket = BGRClib_P_SetOutputControlPacket(grclib, pPacket, false, params->chromaFilter);
    pPacket = BGRClib_P_SetDestripeBlitPacket( grclib, pPacket, &srcRect, &outRect );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/*****************************************************************************/
BERR_Code BGRClib_Blended_Fill( BGRClib_Handle grclib, const BM2MC_PACKET_Plane *pSurface, uint32_t constantColor,
    const BRect *pRect, BGRCLib_FillOp colorOp, BGRCLib_FillOp alphaOp )
{
    BERR_Code err = BERR_SUCCESS;
    BM2MC_PACKET_Rectangle outRect;
    void *pPacket;
    void *pPacketStart;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    BGRClib_P_GetPacketRectangle( pSurface, pRect, &outRect );

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ) {
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    if (colorOp == BGRCLib_FillOp_eCopy && alphaOp == BGRCLib_FillOp_eCopy) {
        pPacket = BGRClib_P_SetDefaultSourceFeederPacket(grclib, pPacket);
    }
    else {
        pPacket = BGRClib_P_SetSourceFeederPacket( grclib, pPacket, pSurface, DEFAULT_COLOR );
    }
    pPacket = BGRClib_P_SetDefaultDestinationFeederPacket( grclib, pPacket);
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, pSurface );

    pPacket = BGRClib_P_SetFillBlendPacket( grclib, pPacket, colorOp, alphaOp, constantColor );
    pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    pPacket = BGRClib_P_SetFillPacket( grclib, pPacket, &outRect );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/*****************************************************************************/
BERR_Code BGRClib_PorterDuffFill( BGRClib_Handle grclib, BGRCLib_PorterDuffOp pdOp,
    const BM2MC_PACKET_Plane *pSurface, uint32_t constantColor,
    const BRect *pRect )
{
    BERR_Code err = BERR_SUCCESS;
    BM2MC_PACKET_Rectangle outRect;
    void *pPacket;
    void *pPacketStart;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    BGRClib_P_GetPacketRectangle( pSurface, pRect, &outRect );

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ) {
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    pPacket = BGRClib_P_SetSourceFeederPacket( grclib, pPacket, pSurface, DEFAULT_COLOR );
    pPacket = BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, pSurface );

    pPacket = BGRClib_P_SetPorterDuffFillBlendPacket( grclib, pPacket, pdOp, constantColor );
    pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    pPacket = BGRClib_P_SetFillPacket( grclib, pPacket, &outRect );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/***************************************************************************/
BERR_Code BGRClib_PorterDuffBlit( BGRClib_Handle grclib, BGRCLib_PorterDuffOp pdOp,
    const BM2MC_PACKET_Plane *pSrcSurface, const BRect *pSrcRect, unsigned paletteOffset, const BM2MC_PACKET_Plane *pDstSurface, const BRect *pDstRect,
    const BM2MC_PACKET_Plane *pOutSurface, const BRect *pOutRect )
{
    BERR_Code err = BERR_SUCCESS;
    BM2MC_PACKET_Rectangle srcRect;
    BM2MC_PACKET_Rectangle dstRect;
    BM2MC_PACKET_Rectangle outRect;
    void *pPacket;
    void *pPacketStart;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    BGRClib_P_GetPacketRectangle( pSrcSurface, pSrcRect, &srcRect );
    BGRClib_P_GetPacketRectangle( pOutSurface, pDstRect, &dstRect ); /* default to output surface dimensions */
    BGRClib_P_GetPacketRectangle( pOutSurface, pOutRect, &outRect );
    if (!dstRect.width || !dstRect.height) {
        dstRect.width = outRect.width;
        dstRect.height = outRect.height;
    }

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ) {
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    pPacket = BGRClib_P_SetSourceFeederPacket( grclib, pPacket, pSrcSurface, DEFAULT_COLOR );
    if (pDstSurface) {
        pPacket = BGRClib_P_SetDestinationFeederPacket( grclib, pPacket, pDstSurface, DEFAULT_COLOR );
    }
    else {
        pPacket = BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket);
    }
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, pOutSurface );

    pPacket = BGRClib_P_SetPorterDuffBlitBlendPacket( grclib, pPacket, pdOp, 0);
    pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetSourcePalette(grclib, pPacket, paletteOffset );
    pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    pPacket = BGRClib_P_SetBlitPacket( grclib, pPacket, &srcRect, pDstSurface ? &dstRect : NULL, &outRect,
        BGRC_FilterCoeffs_ePointSample, BGRC_FilterCoeffs_ePointSample );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/*****************************************************************************/
void BGRClib_GetDefaultBlitParams( BGRClib_BlitParams *params )
{
    BKNI_Memset( params, 0, sizeof(BGRClib_BlitParams) );
    /* already zero: params->colorOp = BGRCLib_BlitColorOp_eCopySource; */
    /* already zero: params->alphaOp = BGRCLib_BlitAlphaOp_eCopySource; */
    params->horzFilter = BGRC_FilterCoeffs_eAnisotropic;
    params->vertFilter = BGRC_FilterCoeffs_eAnisotropic;
    params->chromaFilter = true;
    params->colorKeySelect = BGRC_Output_ColorKeySelection_eTakeBlend;
    params->constantColor = DEFAULT_COLOR;
}

/*****************************************************************************/
void BGRClib_GetDefaultPaletteBlitParams( BGRClib_BlitParams *params )
{
    BKNI_Memset( params, 0, sizeof(BGRClib_BlitParams) );
    /* already zero: params->horzFilter = BGRC_FilterCoeffs_ePointSample; */
    /* already zero: params->vertFilter = BGRC_FilterCoeffs_ePointSample; */
    params->colorKeySelect = BGRC_Output_ColorKeySelection_eTakeSource;
    params->constantColor = DEFAULT_COLOR;
}

/*****************************************************************************/
void BGRClib_GetDefaultColorKeyParams( BGRClib_BlitColorKeyParams *colorkeyparams )
{
    BKNI_Memset( colorkeyparams, 0, sizeof(BGRClib_BlitColorKeyParams) );

    /* TODO: these are unused. need to add packet if they become needed */
    colorkeyparams->cksOnlySrcColorKeyed = BGRC_Output_ColorKeySelection_eTakeDestination;
    colorkeyparams->cksOnlyDstColorKeyed = BGRC_Output_ColorKeySelection_eTakeSource;
    colorkeyparams->cksBothSrcDstColorKeyed = BGRC_Output_ColorKeySelection_eTakeDestination;
    colorkeyparams->cksNeitherSrcDstColorKeyed = BGRC_Output_ColorKeySelection_eTakeSource;
}

/***************************************************************************/
BERR_Code BGRClib_Blit( BGRClib_Handle grclib, const BGRClib_BlitParams *params,
    const BGRClib_BlitColorKeyParams *colorkeyParams, const BGRClib_BlitMatrixParams *matrixParams,
    const BGRClib_BlitPatternParams *patternParams, const BGRClib_BlitScalingControlParams *scalingParams )
{
    BERR_Code err = BERR_SUCCESS;

    void *pPacket;
    void *pPacketStart;

    BM2MC_PACKET_Rectangle srcRect;
    BM2MC_PACKET_Rectangle dstRect;
    BM2MC_PACKET_Rectangle outRect;

    BDBG_OBJECT_ASSERT(grclib, GRClib);

    if (params->colorOp >= BGRCLib_BlitColorOp_Count || params->alphaOp >= BGRCLib_BlitAlphaOp_Count) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (!params->outSurface) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BGRClib_P_GetPacketRectangle( params->srcSurface, params->srcRect, &srcRect );
    BGRClib_P_GetPacketRectangle( params->outSurface, params->dstRect, &dstRect ); /* default to output surface dimensions */
    BGRClib_P_GetPacketRectangle( params->outSurface, params->outRect, &outRect );
    if (!dstRect.width || !dstRect.height) {
        dstRect.width = outRect.width;
        dstRect.height = outRect.height;
    }

    pPacketStart = BGRClib_P_GetPacketBuffer( grclib, GRCLIB_FULL_PACKET_SIZE );
    if( pPacketStart == 0 ){
        return BERR_OUT_OF_DEVICE_MEMORY; /* this is normal for flow control */
    }

    pPacket = pPacketStart;
    pPacket = BGRClib_P_SetSourceFeederPacket( grclib, pPacket, params->srcSurface, params->sourceConstantColor );
    pPacket = BGRClib_P_SetSourceControlPacket(grclib, pPacket, false, params->chromaFilter, false);
    if (params->dstSurface || params->destConstantColor != DEFAULT_COLOR) {
        pPacket = BGRClib_P_SetDestinationControlPacket(grclib, pPacket, false, params->chromaFilter );
        pPacket = BGRClib_P_SetDestinationFeederPacket( grclib, pPacket, params->dstSurface, params->destConstantColor );
    }
    else {
        pPacket = BGRClib_P_SetDefaultDestinationFeederPacket(grclib, pPacket);
    }
    pPacket = BGRClib_P_SetOutputFeederPacket( grclib, pPacket, params->outSurface );
    pPacket = BGRClib_P_SetMipmapPacket(grclib, pPacket, params->miplevel);
    pPacket = BGRClib_P_SetOutputControlPacket(grclib, pPacket, false, params->chromaFilter);

    if (params->colorOp != BGRCLib_BlitColorOp_eCopySource || params->alphaOp != BGRCLib_BlitAlphaOp_eCopySource) {
        pPacket = BGRClib_P_SetBlitBlendPacket( grclib, pPacket, params->colorOp, params->alphaOp,
            &params->colorBlend, &params->alphaBlend, params->constantColor );
    }
    else {
        pPacket = BGRClib_P_SetDefaultBlitBlendPacket(grclib, pPacket);
    }

    if( patternParams ) {
        pPacket = BGRClib_P_SetRopPacket( grclib, pPacket, patternParams->ropVector,
            (uint32_t *) patternParams->pattern, patternParams->backColor, patternParams->foreColor );
    }
    else {
        pPacket = BGRClib_P_SetDefaultRopPacket(grclib, pPacket);
    }

    if( colorkeyParams ) {
        pPacket = BGRClib_P_SetSourceColorkeyPacket( grclib, pPacket, colorkeyParams->enableColorKey,
            colorkeyParams->colorKeyUpper, colorkeyParams->colorKeyLower, colorkeyParams->colorKeyMask,
            colorkeyParams->colorKeyReplace, colorkeyParams->colorKeyRplMask );
        pPacket = BGRClib_P_SetDestinationColorkeyPacket( grclib, pPacket, colorkeyParams->enableDstColorKey,
            colorkeyParams->dstColorKeyUpper, colorkeyParams->dstColorKeyLower, colorkeyParams->dstColorKeyMask,
            colorkeyParams->dstColorKeyReplace, colorkeyParams->dstColorKeyRplMask );
    }
    else {
        pPacket = BGRClib_P_SetDefaultSourceColorkeyPacket(grclib, pPacket);
        pPacket = BGRClib_P_SetDefaultDestinationColorkeyPacket(grclib, pPacket);
    }

    pPacket = BGRClib_P_SetFilterPacket( grclib, pPacket, params->horzFilter, params->vertFilter, &srcRect, &outRect );

    if( matrixParams )
        pPacket = BGRClib_P_SetColorMatrixPacket( grclib, pPacket, matrixParams->conversionMatrix, matrixParams->matrixShift );
    else
        pPacket = BGRClib_P_SetDefaultColorMatrixPacket(grclib, pPacket);

    pPacket = BGRClib_P_SetSourcePalette(grclib, pPacket, params->srcPaletteOffset );

    if (params->srcAlphaPremult) {
        pPacket = BGRClib_P_SetAlphaPremultiplyPacket( grclib, pPacket, params->srcAlphaPremult );
    }
    else {
        pPacket = BGRClib_P_SetDefaultAlphaPremultiplyPacket(grclib, pPacket);
    }

    if (params->sourceFilterOrder != grclib->sourceFilterOrder) {
        pPacket = BGRClib_P_SetSourceFilterOrderPacket(grclib, pPacket, params->sourceFilterOrder);
    }

    if (params->mirrorSrcHorizontally || params->mirrorSrcVertically ||
        params->mirrorDstHorizontally || params->mirrorDstVertically ||
        params->mirrorOutHorizontally || params->mirrorOutVertically ) {
        pPacket = BGRClib_P_SetMirrorPacket( grclib, pPacket,
            params->mirrorSrcHorizontally, params->mirrorSrcVertically,
            params->mirrorDstHorizontally, params->mirrorDstVertically,
            params->mirrorOutHorizontally, params->mirrorOutVertically );
    }
    else {
        pPacket = BGRClib_P_SetDefaultMirrorPacket(grclib, pPacket);
    }

    /* nexus will pass scalingParams == NULL if not used */
    if (scalingParams) {
        pPacket = BGRClib_P_SetFixedScalePacket( grclib, pPacket, scalingParams );
    }
    else {
        pPacket = BGRClib_P_SetDefaultFixedScalePacket(grclib, pPacket);
    }

    pPacket = BGRClib_P_SetBlitPacket( grclib, pPacket, &srcRect, params->dstSurface ? &dstRect : NULL, &outRect, params->horzFilter, params->vertFilter );

    err = BGRClib_P_PacketsComplete( grclib, pPacket, pPacketStart);

    return BERR_TRACE(err);
}

/* End of File */
