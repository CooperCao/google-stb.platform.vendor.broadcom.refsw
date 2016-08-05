/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"

#include "bxpt_priv.h"
#include "bxpt_playback.h"
#include "bxpt_tsmux.h"

#include "bchp_xpt_pb0.h"
#include "bchp_xpt_pb1.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_pb_top.h"
#include "bchp_xpt_pcroffset.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_tsmux );
#endif

typedef struct BXPT_P_TsMuxHandle
{
    BXPT_Handle hXpt;
    BXPT_Playback_Handle Playbacks[ BXPT_NUM_PLAYBACKS ];
    bool PbChanInMux[ BXPT_NUM_PLAYBACKS ];
    BXPT_TsMux_Settings Settings;
    bool MasterPbDefined;
    unsigned MasterPbBandNum;
    unsigned OriginalPbBandNum[ BXPT_NUM_PLAYBACKS ];
}
BXPT_P_TsMuxHandle;

void BXPT_TsMux_P_ResetBandPauseMap(
    BXPT_Handle hXpt
    )
{
    unsigned Index;

    uint32_t RegAddr = BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0;

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        BREG_Write32( hXpt->hRegister, RegAddr, 1 << Index );
        RegAddr += 4;
    }
}

typedef enum MapAction
{
    MapAction_Add,
    MapAction_Remove,
    MapAction_Refresh
}
MapAction;

static void UpdatePlaybackMap(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle hPb,        /* [in] Handle for allocated playback channel */
    uint32_t MapAddr,               /* [in] Base address of map to update */
    MapAction Action
    )
{
    unsigned Index;

    unsigned MapBits = 0;
    bool EnableSelf = (MapAddr == BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0); /* reset BAND_PAUSE_MAP to itself. reset PACING_PAUSE_MAP to 0 */

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->PbChanInMux[ Index ] )
            MapBits |= 1 << Index;
    }

    switch (Action)
    {
        case MapAction_Add:
        MapBits |= 1 << hPb->ChannelNo;
        break;

        case MapAction_Remove:
        MapBits &= ~( 1 << hPb->ChannelNo );
        break;

        case MapAction_Refresh:
        default:
        break;
    }

    /* Walk through the list of playbacks in this mux. Each one must have it's map set. */
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
        {
            BXPT_Playback_Handle hPbx = hTsMux->Playbacks[ Index ];
            uint32_t val = BREG_Read32( hPbx->hRegister, MapAddr + (4 * hPbx->ChannelNo) );

            switch (Action)
            {
                case MapAction_Add:
                val |= MapBits;
                break;

                case MapAction_Remove:
                if( EnableSelf && (hPbx == hPb) )
                {
                    val = 1 << hPb->ChannelNo;
                }
                else
                {
                    val &= MapBits;
                }
                break;

                case MapAction_Refresh:
                val = MapBits;
                break;
            }

            BREG_Write32( hPbx->hRegister, MapAddr + (4 * hPbx->ChannelNo), val );
        }
    }
}

static void UpdatePlaybackConfig(
    BXPT_TsMux_Handle hTsMux
    )
{
    unsigned Index;
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
        {
            BXPT_Playback_ChannelSettings ChannelSettings;
            BXPT_Playback_Handle hPb = hTsMux->Playbacks[ Index ] ;

            BXPT_Playback_GetChannelSettings( hPb, &ChannelSettings );
            if (hTsMux->Settings.bAFAPMode)
            {
                ChannelSettings.UsePcrTimeBase = false;
            }
            BXPT_Playback_SetChannelSettings( hPb, &ChannelSettings );
        }
    }
}

void BXPT_TsMux_P_ResetPacingPauseMap(
    BXPT_Handle hXpt
    )
{
    unsigned Index;

    uint32_t RegAddr = BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0;

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        BREG_Write32( hXpt->hRegister, RegAddr, 0 );
        RegAddr += 4;
    }
}

static void LoadStcMuxDelayDiff(
    BXPT_TsMux_Handle hTsMux               /* [in] Handle for the TsMux */
    )
{
    unsigned Index;
    uint32_t Reg;

    Reg = BCHP_FIELD_DATA( XPT_PB_TOP_PACING_COUNT_RELOAD_CTRL, LD_STC_MUX_DELAY_DIFF, 1 );
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
            Reg |= 1 << hTsMux->Playbacks[ Index ]->ChannelNo;
    }

    BREG_Write32( hTsMux->hXpt->hRegister, BCHP_XPT_PB_TOP_PACING_COUNT_RELOAD_CTRL, Reg );
}

static void LoadPacingCount(
    BXPT_TsMux_Handle hTsMux               /* [in] Handle for the TsMux */
    )
{
    unsigned Index;

    unsigned LoadMap = 0;

    /* Build up the bitmap for loading the count.*/
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
            LoadMap |= 1 << hTsMux->Playbacks[ Index ]->ChannelNo;
    }

    /* Pass down the count and bitmap to each of the playback instances, since
    ** we don't know which one will get encoder data first.
    */
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
            BXPT_Playback_P_SetPacingCount(
                hTsMux->Playbacks[ Index ],
                LoadMap,
                hTsMux->Settings.AFAPSettings.uiPacingCounter
            );
    }
}

static void LoadPacingSpeed(
    BXPT_TsMux_Handle hTsMux               /* [in] Handle for the TsMux */
    )
{
    unsigned Index;

    unsigned Speed = hTsMux->Settings.AFAPSettings.uiPacingSpeed;

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
            BXPT_Playback_P_SetPacingSpeed( hTsMux->Playbacks[ Index ], Speed );
    }
}

void BXPT_TsMux_GetDefaultSettings(
    BXPT_TsMux_Settings *Defaults   /* [out] The defaults */
    )
{
    BDBG_ASSERT( Defaults );
    BKNI_Memset( (void *) Defaults, 0, sizeof( *Defaults ) );
    Defaults->uiMuxDelay = 40;
    Defaults->AFAPSettings.uiPacingSpeed = 4;   /* Was 8. See SW7425-5730. */
}

BERR_Code BXPT_TsMux_Create(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_TsMux_Handle *hTsMux              /* [out] Handle for opened record channel */
    )
{
    BXPT_P_TsMuxHandle *lhMux;

    BERR_Code Ret = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( hTsMux );

    lhMux = BKNI_Malloc( sizeof( *lhMux ) );
    if( !lhMux )
    {
        BDBG_ERR(( "lhMux malloc failed" ));
        Ret = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto Error;
    }

    BKNI_Memset( (void *) lhMux, 0, sizeof( *lhMux ) );
    lhMux->hXpt = hXpt;
    BXPT_TsMux_GetDefaultSettings( &lhMux->Settings );

    *hTsMux = lhMux;
    BDBG_MSG(( "%s hTsMux %p", __FUNCTION__, (void *) lhMux ));
    return Ret;

    Error:
    *hTsMux = NULL;
    return Ret;
}

void BXPT_TsMux_Destroy(
    BXPT_TsMux_Handle hTsMux     /* [in] Handle for the TsMux to destroy */
    )
{
    unsigned Index;

    BDBG_ASSERT( hTsMux );

    BDBG_MSG(( "%s hTsMux %p", __FUNCTION__, (void *) hTsMux ));
    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
        {
            UpdatePlaybackMap( hTsMux, hTsMux->Playbacks[ Index ], BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0, MapAction_Remove );
            UpdatePlaybackMap( hTsMux, hTsMux->Playbacks[ Index ], BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0, MapAction_Remove );
            hTsMux->Playbacks[ Index ] = NULL;
        }
    }

    BKNI_Free( (void *) hTsMux );
}

static void UpdatePidTableBandMapping(
    BXPT_TsMux_Handle hTsMux,
    unsigned OldBandNum,
    unsigned NewBandNum
    )
{
    #define PID_CHNL_STEPSIZE       ( 4 )

    unsigned Index;

    BDBG_MSG(( "Updating PID table for OldBand %u to NewBand %u", OldBandNum, NewBandNum ));
    for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
    {
        uint32_t Reg, RegAddr;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( Index * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hTsMux->hXpt->hRegister, RegAddr );

        if( !BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) )
            continue;  /* Skip live input parsers. We only want the playbacks */

        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) == OldBandNum )
        {
            BDBG_MSG(( "Remapping PID channel %u, PB %u to PB %u", Index, OldBandNum, NewBandNum ));
            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, NewBandNum )
            );
            BREG_Write32( hTsMux->hXpt->hRegister, RegAddr, Reg );
        }
    }
}

BERR_Code BXPT_TsMux_AddPlayback(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle hPb        /* [in] Handle for allocated playback channel */
    )
{
    unsigned Index;

    BERR_Code Ret = BERR_SUCCESS;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( hPb );

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] == hPb )
            goto Done;  /* They already added it. */

        if( !hTsMux->Playbacks[ Index ] )
            break;      /* Found an empty slot */
    }

    if( Index == BXPT_NUM_PLAYBACKS )
    {
        /*
        ** This probably should be an internal error. We took care of duplicated
        ** additions above, so there should not be more playbacks than BXPT_NUM_PLAYBACKS .
        */
        BDBG_ERR(( "All playback slots are used" ));
        Ret = BERR_INVALID_PARAMETER;
        goto Done;
    }

    BDBG_MSG(( "%s hTsMux %p, hPb (channel) {bandId} %p (%u) {%u}", __FUNCTION__, (void *) hTsMux, (void *) hPb, hPb->ChannelNo, BXPT_Playback_P_GetBandId( hPb ) ));
    hTsMux->OriginalPbBandNum[ Index ] = BXPT_Playback_P_GetBandId( hPb );
    if( !hTsMux->MasterPbDefined )
    {
        hTsMux->MasterPbBandNum = hTsMux->OriginalPbBandNum[ Index ]; /* First playback will be the band-hold master. */
        hTsMux->MasterPbDefined = true;
        BDBG_MSG(( "PB%u is the mux master", hTsMux->MasterPbBandNum ));
    }
    else
    {
        BDBG_MSG(( "PB%u remapped to PB%u", BXPT_Playback_P_GetBandId( hPb ), hTsMux->MasterPbBandNum ));
        UpdatePidTableBandMapping( hTsMux, hTsMux->OriginalPbBandNum[ Index ], hTsMux->MasterPbBandNum );
        BXPT_Playback_P_SetBandId( hPb, hTsMux->MasterPbBandNum );  /* All others have their bandIds remapped. */
    }

    hTsMux->Playbacks[ Index ] = hPb;
    hTsMux->PbChanInMux[ hPb->ChannelNo ] = true;
    UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0, MapAction_Add );
    if( hTsMux->Settings.bAFAPMode )
    {
        UpdatePlaybackMap( hTsMux, hTsMux->Playbacks[ Index ], BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0, MapAction_Add );
        LoadPacingSpeed( hTsMux );
    }

    UpdatePlaybackConfig( hTsMux );
    /*LoadStcMuxDelayDiff( hTsMux ); */

    /*
    ** Packets carrying the NEXT_PACKET_TIMESTAMP values will be ignored by the threshold comparison logic
    ** For Mux usage, the value of this threshold is calculated based on the worst case audio bit rate (4kbps)supported
    ** The value of 300 msec should be programmed in this register
    ** For Binary format = 0x3DAD (dec 15789) Mod-300 = 0x6978 (dec 27002)
    */
    BXPT_Playback_SetPacingErrorBound( hPb, 0x3DAD );   /* We're using the binary format */

    Done:
    return Ret;
}

void BXPT_TsMux_RemovePlayback(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle hPb    /* [in] Handle for allocated playback channel */
    )
{
    unsigned Index;
    unsigned OldPbBandNum;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( hPb );

    BDBG_MSG(( "%s hTsMux %p, hPb (channel) %p (%u)", __FUNCTION__, (void *) hTsMux, (void *) hPb, hPb->ChannelNo ));

    /* UpdatePlaybackMap() needs to be called before removing the hPb handles from hTsMux. */
    if( hTsMux->Settings.bAFAPMode )
    {
        UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0, MapAction_Remove );
        UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0, MapAction_Remove );
    }

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] == hPb )
        {
            BXPT_Playback_P_SetBandId( hPb, hTsMux->OriginalPbBandNum[ Index ] );
            OldPbBandNum = hTsMux->OriginalPbBandNum[ Index ];
            hTsMux->Playbacks[ Index ] = NULL;
            hTsMux->PbChanInMux[ hPb->ChannelNo ] = false;
            break;
        }
    }

    /* Check for the possiblity that this PB was already removed. */
    if( BXPT_NUM_PLAYBACKS == Index )
        return;

    /* Is the master playback being removed? */
    if( OldPbBandNum == hTsMux->MasterPbBandNum )
    {
        /* Yes, need to find a new master band Id. */
        hTsMux->MasterPbDefined = false;
        for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
        {
            BXPT_Playback_Handle thisPb = hTsMux->Playbacks[ Index ];

            if( thisPb )
            {
                if( !hTsMux->MasterPbDefined )
                {
                    BXPT_Playback_P_SetBandId( thisPb, hTsMux->OriginalPbBandNum[ Index ] );
                    hTsMux->MasterPbBandNum = hTsMux->OriginalPbBandNum[ Index ];
                    hTsMux->MasterPbDefined = true;
                    UpdatePidTableBandMapping( hTsMux, OldPbBandNum, hTsMux->MasterPbBandNum );
                }
                else
                {
                    BXPT_Playback_P_SetBandId( thisPb, hTsMux->MasterPbBandNum );
                }
            }
        }
    }
}

void BXPT_TsMux_GetSettings(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_TsMux_Settings *Settings   /* [out] The current settings  */
    )
{
    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( Settings );

    *Settings = hTsMux->Settings;
}

BERR_Code BXPT_TsMux_SetSettings(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    const BXPT_TsMux_Settings *pSettings /* [in] New settings to use */
    )
{
    uint32_t Reg;

    BERR_Code Ret = BERR_SUCCESS;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( pSettings );
    BDBG_ASSERT( pSettings->hPcrOffset );

    /* Largest delay (in mS) that hw supports: MUX_DELAY is 28 bits wide, and runs at 27 MHz*/
    #define MAX_MUX_DELAY   (9942)
    if( pSettings->uiMuxDelay > MAX_MUX_DELAY )
    {
        BDBG_ERR(( "uiMuxDelay out of range (0 to %u)", MAX_MUX_DELAY ));
        Ret = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    hTsMux->Settings = *pSettings;

    /* SW7425-5730: Limit the pacing speed to 4x, for the 40nm chips only. */
    if( hTsMux->Settings.bAFAPMode &&
        (4 < hTsMux->Settings.AFAPSettings.uiPacingSpeed || !hTsMux->Settings.AFAPSettings.uiPacingSpeed) )
    {
        BDBG_MSG(( "uiPacingSpeed out of range (1 to %u)", 4 ));
        hTsMux->Settings.AFAPSettings.uiPacingSpeed = 4;
    }

    /* STC_MUX_DELAY is in the PCR Offset block */
    Reg = BREG_Read32( hTsMux->hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_MUX_DELAY );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC_MUX_DELAY, MUX_DELAY ) |
        BCHP_MASK( XPT_PCROFFSET_STC_MUX_DELAY, STC_SEL )
    );

    UpdatePlaybackMap( hTsMux, NULL, BCHP_XPT_PB_TOP_BAND_PAUSE_MAP_VECT_PB0, MapAction_Refresh );
    if (pSettings->bAFAPMode)
    {
        UpdatePlaybackMap( hTsMux, NULL, BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0, MapAction_Refresh );
        LoadPacingCount( hTsMux );
        LoadPacingSpeed( hTsMux );
    }
    else
    {
        unsigned Index;

        for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
        {
            BXPT_Playback_Handle hPb = hTsMux->Playbacks[ Index ];

            if( hPb )
            {
                UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_PB_TOP_PACING_PAUSE_MAP_VECT_PB0, MapAction_Remove );
                BXPT_Playback_P_SetPacingSpeed( hPb, 1 );
            }
        }
    }
    UpdatePlaybackConfig( hTsMux );

    /* The MUX_DELAY bitfield is a 27 MHz domain, straight binary number. */
    Reg |= BCHP_FIELD_DATA( XPT_PCROFFSET_STC_MUX_DELAY, MUX_DELAY, pSettings->uiMuxDelay * 27000 );
    Reg |= BCHP_FIELD_DATA( XPT_PCROFFSET_STC_MUX_DELAY, STC_SEL, pSettings->hPcrOffset->WhichStc );
    BREG_Write32( hTsMux->hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_MUX_DELAY, Reg );

    if( !pSettings->bAFAPMode )
    {
        LoadStcMuxDelayDiff( hTsMux );
    }

    Done:
    return BERR_SUCCESS;
}

void BXPT_TsMux_GetStatus(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_TsMux_Status *Status       /* [out] Channel status. */
    )
{
    #define STC_REG_STEP ( BCHP_XPT_PCROFFSET_STC1_HI - BCHP_XPT_PCROFFSET_STC0_HI )

    uint32_t Reg, RegAddr;
    uint64_t Temp;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( Status );

    RegAddr = BCHP_XPT_PCROFFSET_STC0_HI + hTsMux->Settings.hPcrOffset->WhichStc * STC_REG_STEP;
    Reg = BREG_Read32( hTsMux->hXpt->hRegister, RegAddr );
    Temp = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_HI, COUNT );
    Status->uiSTC = Temp << 32;

    RegAddr = BCHP_XPT_PCROFFSET_STC0_LO + hTsMux->Settings.hPcrOffset->WhichStc * STC_REG_STEP;
    Reg = BREG_Read32( hTsMux->hXpt->hRegister, RegAddr );
    Status->uiSTC |= BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_LO, COUNT );

    /* Return the PACING_COUNTER value of the first PB channel that is registered
     * with TsMux.  All PB channels should have the same value, so we just use the
     * first one.
     */
    {
       unsigned Index;
       BXPT_Playback_MuxingInfo Info;

       for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
       {
           if( hTsMux->Playbacks[ Index ] )
           {
               BXPT_Playback_GetMuxingInfo( hTsMux->Playbacks[ Index ], &Info );
               Status->uiESCR = Info.uiPacingCounter;
               break;
           }
       }
       if ( BXPT_NUM_PLAYBACKS == Index )
       {
          BDBG_ERR(("No PB Channels associated with BXPT_TsMux.  ESCR Valid is not valid."));
          Status->uiESCR = 0;
       }
    }
}

void BXPT_Playback_ConfigTsMuxDesc(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,              /* [in] Descriptor to be configured */
    const BAVC_TsMux_DescConfig *Config     /* [in] Data to be loaded */
    )
{
    BXPT_PvrDescriptorFlags flags;
    BDBG_ASSERT( Desc );
    BDBG_ASSERT( Config );

    BXPT_Playback_InitDescriptorFlags( &flags );
    flags.muxFlags = *Config;
    BXPT_Playback_SetDescriptorFlags( hPb, Desc, &flags );
}

void BXPT_Tsmux_GetDescConfig(
    const BXPT_PvrDescriptor *Desc,
    BAVC_TsMux_DescConfig *Config     /* [out] muxing flags unpacked from the descriptor */
    )
{
    BDBG_ASSERT( Desc );
    BDBG_ASSERT( Config );

    /* These definitions came from the SW Guide to MCPB 2.0 doc, definition of the 12 word descriptor. */
    Config->bRandomAccessIndication = Desc->MuxingFlags & 1 << 2 ? true : false;
    Config->bNextPacketPacingTimestampValid = Desc->MuxingFlags & 1 << 1 ? true : false;
    Config->uiNextPacketPacingTimestamp = Desc->NextPacketPacingTimestamp;
    Config->bPacket2PacketTimestampDeltaValid = Desc->MuxingFlags & 1 ? true : false;
    Config->uiPacket2PacketTimestampDelta = Desc->Pkt2PktPacingTimestampDelta;
}
