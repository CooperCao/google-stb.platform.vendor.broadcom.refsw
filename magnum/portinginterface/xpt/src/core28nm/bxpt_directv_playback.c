/******************************************************************************
 * (c) 2003-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/


#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_playback.h"
#include "bxpt_directv_playback.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_directv_playback );
#endif

BERR_Code BXPT_DirecTvPlayback_SetParserBandMode(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel */
    BXPT_ParserMode Mode                    /* [in] Which mode (packet format) is being used. */
    )
{

    /* no-op for 28nm chips. for DSS playback, call BXPT_Playback_SetChannelSettings with PacketLength = 130 and SyncMode = BXPT_PB_SYNC_DSS */
    BSTD_UNUSED(hPb);
    BSTD_UNUSED(Mode);
    return 0;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_DirecTvPlayback_GetSyncThresholds(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    unsigned int *SyncInCount,          /* [out] In-sync threshold. */
    unsigned int *SyncOutCount          /* [out] Out-of-sync threshold. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

#if 0
    /* Get the sync in/out counts. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3 );
    *SyncInCount = ( uint8_t ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT ) & 0xFF );
    *SyncOutCount = ( uint8_t ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT ) & 0xFF );
#else
    BSTD_UNUSED( Reg );
    *SyncInCount = 0;
    *SyncOutCount = 0;
#endif
    return( ExitCode );
}


BERR_Code BXPT_DirecTvPlayback_SetSyncThresholds(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    unsigned int SyncInCount,           /* [in] In-sync threshold. */
    unsigned int SyncOutCount           /* [in] Out-of-sync threshold. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );
#if 0
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3 );

    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT ) |
        BCHP_MASK( XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL3, DIRECTV_SYNC_IN_CNT, SyncInCount ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL3, DIRECTV_SYNC_OUT_CNT, SyncOutCount )
        );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL3, Reg );
#else
    BSTD_UNUSED( Reg );
    BSTD_UNUSED( SyncInCount );
    BSTD_UNUSED( SyncOutCount );
#endif

    return( ExitCode );
}
#endif
