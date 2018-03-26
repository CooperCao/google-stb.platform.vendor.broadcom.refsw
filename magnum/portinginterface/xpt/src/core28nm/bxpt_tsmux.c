/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include "bstd.h"
#include "bkni.h"

#include "bxpt_priv.h"
#include "bxpt_playback.h"
#include "bxpt_tsmux.h"

#include "bchp_xpt_mcpb.h"
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"

#include "bchp_xpt_fe.h"
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

    BXPT_Playback_PacingCounter PacingCounter;  /* Default value is NULL (no counter allocated). */

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

    uint32_t RegAddr = BCHP_XPT_MCPB_BAND_PAUSE_MAPPING_VECTOR_0;

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
                if( hPbx->ChannelNo == hPb->ChannelNo )
                    val = 1 << hPb->ChannelNo;
                else
                    val &= MapBits;
                break;

                case MapAction_Refresh:
                val = MapBits;
                break;

                default:
                BDBG_ERR(( "%s: internal error, unsupported map action", BSTD_FUNCTION ));
                BDBG_ASSERT( 0 );
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
            ChannelSettings.UsePcrTimeBase = hTsMux->Settings.bAFAPMode ? false : true;
            ChannelSettings.PacingPauseEn = hTsMux->Settings.bAFAPMode ? true : false;
            BXPT_Playback_SetChannelSettings( hPb, &ChannelSettings );
        }
    }
}

void BXPT_TsMux_GetDefaultSettings(
    BXPT_TsMux_Settings *Defaults   /* [out] The defaults */
    )
{
    BDBG_ASSERT( Defaults );
    BKNI_Memset( (void *) Defaults, 0, sizeof( *Defaults ) );
    Defaults->uiMuxDelay = 40;
    Defaults->AFAPSettings.uiPacingSpeed = 8;
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

    lhMux->PacingCounter = BXPT_Playback_AllocPacingCounter( hXpt );
    if( !lhMux->PacingCounter )
    {
        BDBG_ERR(( "PacingCounter alloc failed" ));
        Ret = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        BKNI_Free( lhMux );
        goto Error;
    }

    *hTsMux = lhMux;
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

    for( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ )
    {
        if( hTsMux->Playbacks[ Index ] )
        {
            BXPT_Playback_ChannelSettings pbSettings;

            UpdatePlaybackMap( hTsMux, hTsMux->Playbacks[ Index ], BCHP_XPT_MCPB_BAND_PAUSE_MAPPING_VECTOR_0, MapAction_Remove );

            BXPT_Playback_GetChannelSettings( hTsMux->Playbacks[ Index ], &pbSettings );
            pbSettings.PacingCounter = NULL;
            pbSettings.PacingPauseEn = false;
            BXPT_Playback_SetChannelSettings( hTsMux->Playbacks[ Index ], &pbSettings );

            hTsMux->Playbacks[ Index ] = NULL;
        }
    }

    BXPT_Playback_FreePacingCounter( hTsMux->PacingCounter );
    BKNI_Free( (void *) hTsMux );
}

BERR_Code BXPT_TsMux_AddPlayback(
    BXPT_TsMux_Handle hTsMux,               /* [in] Handle for the TsMux */
    BXPT_Playback_Handle hPb        /* [in] Handle for allocated playback channel */
    )
{
    unsigned Index;
    BXPT_Playback_ChannelSettings pbSettings;

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

    hTsMux->OriginalPbBandNum[ Index ] = BXPT_Playback_P_GetBandId( hPb );

    BDBG_MSG(( "Adding PB%u to Mux", hPb->ChannelNo ));
    hTsMux->Playbacks[ Index ] = hPb;
    hTsMux->PbChanInMux[ hPb->ChannelNo ] = true;
    UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_MCPB_BAND_PAUSE_MAPPING_VECTOR_0, MapAction_Add );

    BXPT_Playback_GetChannelSettings( hTsMux->Playbacks[ Index ], &pbSettings );
    pbSettings.PacingCounter = hTsMux->PacingCounter;

    pbSettings.PacingPauseEn = hTsMux->Settings.bAFAPMode ? true : false;
    BXPT_Playback_SetChannelSettings( hTsMux->Playbacks[ Index ], &pbSettings );
    BDBG_MSG(( "PB handle 0x%08lX: PacingPauseEn %s", (unsigned long) hTsMux->Playbacks[ Index ], pbSettings.PacingPauseEn ? "true" : "false" ));

    UpdatePlaybackConfig( hTsMux );
    /* Ret = BXPT_Playback_P_LoadStcMuxDelayDiff( hPb ); */

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
    BXPT_Playback_ChannelSettings pbSettings;
    unsigned OldPbBandNum = 0xFFFF;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( hPb );

    BDBG_MSG(( "Removing PB%u from Mux", hPb->ChannelNo ));

    /* UpdatePlaybackMap() needs to be called before removing the hPb handles from hTsMux. */
    if( hTsMux->Settings.bAFAPMode )
    {
        UpdatePlaybackMap( hTsMux, hPb, BCHP_XPT_MCPB_BAND_PAUSE_MAPPING_VECTOR_0, MapAction_Remove );
    }

    BXPT_Playback_GetChannelSettings( hPb, &pbSettings );
    pbSettings.PacingCounter = NULL;
    pbSettings.PacingPauseEn = false;
    BXPT_Playback_SetChannelSettings( hPb, &pbSettings );

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

    /* Check that it might already been removed. */
    if( BXPT_NUM_PLAYBACKS == Index || OldPbBandNum == 0xFFFF )
        return;
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

    BERR_Code status = BERR_SUCCESS;
    BXPT_Playback_Handle hPb = NULL;

    BDBG_ASSERT( hTsMux );
    BDBG_ASSERT( pSettings );
    BDBG_ASSERT( pSettings->hPcrOffset );

    /* Largest delay (in mS) that hw supports: MUX_DELAY is 28 bits wide, and runs at 27 MHz*/
    #define MAX_MUX_DELAY   (9942)
    if( pSettings->uiMuxDelay > MAX_MUX_DELAY )
    {
        BDBG_ERR(( "uiMuxDelay out of range (0 to %u)", MAX_MUX_DELAY ));
        status = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    BDBG_MSG(( "AFAPMode %s", pSettings->bAFAPMode ? "TRUE" : "FALSE" ));
    hTsMux->Settings = *pSettings;
    hPb = hTsMux->Playbacks[ 0 ];

    if( hTsMux->Settings.bAFAPMode &&
        (BXPT_TSMUX_MAX_PACING_SPEED < hTsMux->Settings.AFAPSettings.uiPacingSpeed || !hTsMux->Settings.AFAPSettings.uiPacingSpeed) )
    {
        BDBG_MSG(( "uiPacingSpeed out of range (1 to %u)", BXPT_TSMUX_MAX_PACING_SPEED ));
        hTsMux->Settings.AFAPSettings.uiPacingSpeed = BXPT_TSMUX_MAX_PACING_SPEED;
    }

    /* STC_MUX_DELAY is in the PCR Offset block */
    Reg = BREG_Read32( hTsMux->hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_MUX_DELAY );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC_MUX_DELAY, MUX_DELAY ) |
        BCHP_MASK( XPT_PCROFFSET_STC_MUX_DELAY, STC_SEL )
    );

    UpdatePlaybackMap( hTsMux, NULL, BCHP_XPT_MCPB_BAND_PAUSE_MAPPING_VECTOR_0, MapAction_Refresh );
    if( hPb )
    {
        status |= BXPT_Playback_P_SetPacingSpeed( hPb, pSettings->bAFAPMode ? hTsMux->Settings.AFAPSettings.uiPacingSpeed : 1 );
        if( BERR_SUCCESS != status )
            goto Done;
    }

    if( hTsMux->Settings.bAFAPMode )
    {
        status |= BXPT_Playback_P_LoadPacingCounter( hPb, pSettings->AFAPSettings.uiPacingCounter );
        if( BERR_SUCCESS != status )
            goto Done;
    }

    UpdatePlaybackConfig( hTsMux );

    /* The MUX_DELAY bitfield is a 27 MHz domain, straight binary number. */
    Reg |= BCHP_FIELD_DATA( XPT_PCROFFSET_STC_MUX_DELAY, MUX_DELAY, pSettings->uiMuxDelay * 27000 );
    Reg |= BCHP_FIELD_DATA( XPT_PCROFFSET_STC_MUX_DELAY, STC_SEL, pSettings->hPcrOffset->WhichStc );
    BREG_Write32( hTsMux->hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_MUX_DELAY, Reg );

    if( hTsMux->Playbacks[ 0 ] && !pSettings->bAFAPMode )
         status = BXPT_Playback_P_LoadStcMuxDelayDiff( hPb );

    Done:
    return status;
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

#if (!B_REFSW_MINIMAL)
void BXPT_Tsmux_GetDescConfig(
    const BXPT_PvrDescriptor *Desc,
    BAVC_TsMux_DescConfig *Config     /* [out] muxing flags unpacked from the descriptor */
    )
{
    uint32_t *descWords = (uint32_t *) Desc;

    BDBG_ASSERT( Desc );
    BDBG_ASSERT( Config );

    /* These definitions came from the SW Guide to MCPB 2.0 doc, definition of the 12 word descriptor. */
    Config->bRandomAccessIndication = descWords[5] & 1 << 2 ? true : false;
    Config->bNextPacketPacingTimestampValid = descWords[5] & 1 << 1 ? true : false;
    Config->uiNextPacketPacingTimestamp = descWords[8];
    Config->bPacket2PacketTimestampDeltaValid = descWords[5] & 1 ? true : false;
    Config->uiPacket2PacketTimestampDelta = descWords[9];
    Config->bPushPartialPacket = descWords[4] & (1 << 28) ? true : false;
    Config->bPushPreviousPartialPacket = descWords[4] & (1 << 27) ? true : false;
    Config->bHostDataInsertion = descWords[4] & (1 << 29) ? true : false;
    Config->bPidChannelValid = descWords[5] & (1 << 19) ? true : false;
    Config->uiPidChannelNo = descWords[6] & 0x3FF;
    Config->bInsertHostDataAsBtp = descWords[5] & (1 << 22) ? true : false;
}
#endif

void BXPT_TsMux_P_ResetPacingPauseMap(
    BXPT_Handle hXpt
    )
{
    BSTD_UNUSED(hXpt);
}
