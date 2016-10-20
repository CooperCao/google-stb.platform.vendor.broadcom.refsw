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
#include "bxpt_xcbuf_priv.h"
#include "bxpt_spid.h"

#include "bchp_xpt_pb0.h"
#include "bchp_xpt_fe.h"

#include "bxpt.h"
#include "bchp_xpt_pb1.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BXPT_HAS_TSMUX
#include "bchp_xpt_pb_top.h"
#endif

#define PB_PARSER_REG_STEPSIZE  ( BCHP_XPT_PB1_CTRL1 - BCHP_XPT_PB0_CTRL1 )

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_playback );
#endif

#define BXPT_P_PLAYBACK_DEFAULT_USE_PCR_TIMEBASE        false
#define BXPT_P_PLAYBACK_DEFAULT_TIMESTAMP_MODE          BXPT_TimestampMode_e30_2U_Mod300
#define BXPT_PB_MAX_SYNC_LENGTH                         ( 256 )
#define FLUSH_COUNTDOWN                                 ( 10 )


static BERR_Code BXPT_Playback_P_CreateDesc( BXPT_Handle hXpt, BXPT_PvrDescriptor * const Desc, uint8_t *Buffer,
    uint32_t BufferLength, bool IntEnable, bool ReSync, BXPT_PvrDescriptor * const NextDesc );

static void TsRangeErrorIsr ( void *hPb , int Param2);

static BERR_Code CalcAndSetBlockout(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t BitRate            /* [in] Rate, in bits per second. */
    );

BERR_Code BXPT_Playback_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int *TotalChannels     /* [out] The number of playback channels. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    *TotalChannels = hXpt->MaxPlaybacks;

    return( ExitCode );
}


BERR_Code BXPT_Playback_GetChannelDefaultSettings(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int ChannelNo,         /* [in] Which channel to get defaults from. */
    BXPT_Playback_ChannelSettings *ChannelSettings /* [out] The defaults */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ChannelSettings );

    if( ChannelNo >= hXpt->MaxPlaybacks )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BKNI_Memset( (void *) ChannelSettings, 0, sizeof(*ChannelSettings) );
        ChannelSettings->UsePcrTimeBase = BXPT_P_PLAYBACK_DEFAULT_USE_PCR_TIMEBASE;
        ChannelSettings->WhichPcrToUse = 0;
        ChannelSettings->TimestampEn = false;
        ChannelSettings->TimestampMode = BXPT_P_PLAYBACK_DEFAULT_TIMESTAMP_MODE;
        ChannelSettings->PacketLength = 188;
        ChannelSettings->SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
        ChannelSettings->RaveOutputOnly = false;
        ChannelSettings->DisableTimestampParityCheck = false;
        ChannelSettings->SwapBytes = false;     /* Input file is also big-endian. Don't swap */
    }

    ChannelSettings->PcrBasedPacing = false;   /* Use legacy 4-byte timestamp pacing */
    ChannelSettings->PcrPacingPid = 0x1FFF;
    ChannelSettings->Use32BitTimestamps = false;

#if BXPT_HAS_TSMUX
    /* Defaults, to keep existing behavior. */
    ChannelSettings->PesBasedPacing = false;
    ChannelSettings->Use8WordDesc = false;
#endif

    return( ExitCode );
}

BERR_Code BXPT_Playback_OpenChannel(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_Playback_Handle *PlaybackHandle,   /* [out] Handle for opened record channel */
    unsigned int ChannelNo,                         /* [in] Which channel to open. */
    const BXPT_Playback_ChannelSettings *ChannelSettings /* [in] The defaults to use */
    )
{
    uint32_t Reg;

    uint32_t BaseAddr = 0;
    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Playback_Handle hPb = NULL;
    BINT_Id TsRangeErrorIntId;
    BXPT_PidChannel_CC_Config DefaultCcConfig = { false, false, false, 0 };
    uint8_t *CachedDummyBuffer;
    void *DummyDescCached;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ChannelSettings );

    if( !BXPT_Playback_IsAvailable( hXpt, ChannelNo ) )
    {
        BDBG_ERR(( "Channel Number %lu is out of range or already openned!", ( unsigned long ) ChannelNo ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        return ExitCode;
    }

    /*
    ** Use the address of the first register in the playback block as the
    ** base address of the entire block.
    */
    BaseAddr = BCHP_XPT_PB0_CTRL1 + ( ChannelNo * PB_PARSER_REG_STEPSIZE );

    /* Create the playback handle. */
    hPb = &hXpt->PlaybackHandles[ ChannelNo ];
    hPb->hChip = hXpt->hChip;
    hPb->hRegister = hXpt->hRegister;
    hPb->BaseAddr = BaseAddr;
    hPb->ChannelNo = ChannelNo;
    hPb->LastDescriptor_Cached = 0;
    hPb->Running = false;
    hPb->vhXpt = hXpt;
    hPb->SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
    hPb->BitRate = 27000000;
    hPb->IsParserInAllPass = false;
    hPb->CcConfigBeforeAllPass = DefaultCcConfig;
    hPb->ForceResync = 0;
    hPb->maxTsError = 1024;

    /* If they've got a separate heap for playback, use it */
    if( hXpt->hPbHeap )
        hPb->hMemory = hXpt->hPbHeap;
    else
        hPb->hMemory = hXpt->hMemory;

    TsRangeErrorIntId = BXPT_Playback_GetIntId( hPb, BXPT_PbInt_eTsRangeErr );

    ExitCode = BINT_CreateCallback(
        &hPb->hPbIntCallback,
        hXpt->hInt,
        TsRangeErrorIntId,
        TsRangeErrorIsr,
        ( void * ) hPb,
        0 );
    if( BERR_SUCCESS != ExitCode )
    {
        ExitCode = BERR_TRACE( ExitCode );
        goto Done;
    }

    if ( ChannelSettings->ResetPacing)
    {
        hPb->ResetPacingOnTsRangeError = true;
        ExitCode = BINT_EnableCallback( hPb->hPbIntCallback );
        if( BERR_SUCCESS != ExitCode )
        {
            ExitCode = BERR_TRACE( ExitCode );
            goto ErrorCleanUp;
        }
    }
    else
    {
        hPb->ResetPacingOnTsRangeError = false;
        ExitCode = BINT_DisableCallback( hPb->hPbIntCallback );
        if( BERR_SUCCESS != ExitCode )
        {
            ExitCode = BERR_TRACE( ExitCode );
            goto ErrorCleanUp;
        }
    }

    /* Restore all the stuff they could have changed through the API. */
    Reg = (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 0 )
    );

    /* Anand's playback hw fix didn't catch one corner case. Turn on the chicken bit. */
    #if 0
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL1, AGRESSIVE_DESC_READ, 1 ) );
    #endif

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_AUTOSTART_ON_TS_ERROR, 1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, PACING_EN ) );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    /* Now load the defaults they passed in. */
    BXPT_Playback_SetChannelSettings( hPb, ChannelSettings );

    /* Create the dummy descriptor and buffer */
    hPb->DummyDescriptor = BMEM_AllocAligned( hPb->hMemory, sizeof( BXPT_PvrDescriptor ), 4, 0 );
    if( !hPb->DummyDescriptor )
    {
        BDBG_ERR(( "DummyDescriptor alloc failed!" ));
        ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
        goto ErrorCleanUp;
    }
    hPb->DummyBuffer = BMEM_AllocAligned( hPb->hMemory, 2, 0, 0 );
    if( !hPb->DummyBuffer )
    {
        BDBG_ERR(( "DummyBuffer alloc failed!" ));
        BMEM_Free( hPb->hMemory, hPb->DummyDescriptor );
        ExitCode = BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
        goto ErrorCleanUp;
    }

    /* Flushing the cache is done in BXPT_Playback_CreateDesc() */
    BMEM_ConvertAddressToCached(hPb->hMemory, (void *) hPb->DummyBuffer, (void **) &CachedDummyBuffer);
    CachedDummyBuffer[ 0 ] = CachedDummyBuffer[ 1 ] = 0x55;
    BMEM_ConvertAddressToCached(hPb->hMemory, hPb->DummyDescriptor, &DummyDescCached);

    BXPT_Playback_CreateDesc( hXpt, DummyDescCached, hPb->DummyBuffer, 2, false, false, NULL );
    BMEM_FlushCache(hPb->hMemory, CachedDummyBuffer, 2 );
    hPb->Opened = true;
    *PlaybackHandle = hPb;

    Done:
    return( ExitCode );

    ErrorCleanUp:
    (void) BINT_DisableCallback( hPb->hPbIntCallback );
    (void) BINT_DestroyCallback( hPb->hPbIntCallback );
    return( ExitCode );
}

void BXPT_Playback_CloseChannel(
    BXPT_Playback_Handle PlaybackHandle /* [in] Handle for the channel to close*/
    )
{
    /* Stop the channel. */
    uint32_t Reg = (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 0 )
    );

    BINT_DestroyCallback( PlaybackHandle->hPbIntCallback );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

    /* Clean up all previous Packetization state, if any. */
    BXPT_Playback_DisablePacketizers( PlaybackHandle );

    BMEM_Free( PlaybackHandle->hMemory, PlaybackHandle->DummyDescriptor );
    BMEM_Free( PlaybackHandle->hMemory, PlaybackHandle->DummyBuffer );

    PlaybackHandle->Opened = false;
}

void TsRangeErrorIsr( void *hPb, int Param2 )
{
    uint32_t Reg;

    BDBG_ASSERT( hPb );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );

    /*
    ** Init the Pacing bitfields to 0. The PB_PACING_START bit must transistion from 0 to 1
    ** in order to arm the pacing logic.
    */
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, PACING_START ) );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_START, 1 ) );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );
    BSTD_UNUSED( Param2 );
    return;
}

BERR_Code BXPT_Playback_SetChannelSettings(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelSettings *ChannelSettings /* [in] New settings to use */
    )
{
    uint32_t Reg, TimeBaseSel;

    unsigned int PacketLength = 188;    /* Use MPEG length as the default. */
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ChannelSettings );

    hPb->SyncMode = ChannelSettings->SyncMode;

    if( ChannelSettings->PacketLength > BXPT_PB_MAX_SYNC_LENGTH )
    {
        BDBG_ERR(( "Packet length %d too long! Clamped to %d", ChannelSettings->PacketLength, BXPT_PB_MAX_SYNC_LENGTH ));
        PacketLength = BXPT_PB_MAX_SYNC_LENGTH;
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        PacketLength = ChannelSettings->PacketLength;
    }

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL3, SYNC_LENGTH )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL3, SYNC_LENGTH, PacketLength )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL3, Reg );

    /* Need to recalculate the blockout value if the sync length has changed. */
    CalcAndSetBlockout( hPb, hPb->BitRate );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE ) |
        BCHP_MASK( XPT_PB0_CTRL2, ENDIAN_CTRL ) |
        BCHP_MASK( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS ) |
        BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_EN ) |
        BCHP_MASK( XPT_PB0_CTRL2, PROGRAM_STREAM_MODE ) |
        BCHP_MASK( XPT_PB0_CTRL2, PROGRAM_STREAM_EN )
        |
        BCHP_MASK( XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS )
        );

    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, ChannelSettings->SyncMode ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS, ChannelSettings->DisableTimestampParityCheck == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, ENDIAN_CTRL, ChannelSettings->SwapBytes == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_EN, ChannelSettings->TimestampEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS, ChannelSettings->PacingOffsetAdjustDisable == true ? 1 : 0 )
        );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, PROGRAM_STREAM_MODE, ChannelSettings->PsMode );
        if( ChannelSettings->PackHdrConfig != BXPT_Playback_PackHdr_Drop)
        {
            Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, PROGRAM_STREAM_EN, 1 );
        }
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL4, OUTPUT_PIPE_SEL ) |
        BCHP_MASK( XPT_PB0_CTRL4, TIMEBASE_SEL ) |
        BCHP_MASK( XPT_PB0_CTRL4, TIMESTAMP_MODE )|
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN )
        );

    if( ChannelSettings->UsePcrTimeBase == false )
    {
        /* Use the free running 27 MHz clock. */
        TimeBaseSel = 0;
    }
    else
    {
        if( ChannelSettings->WhichPcrToUse > BXPT_NUM_PCRS )
        {
            /* Bad PCR module number. Complain, and default to free running 27 MHz clock. */
            BDBG_ERR(( "WhichPcrToUse %lu is out of range!", ( unsigned long ) ChannelSettings->WhichPcrToUse ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            TimeBaseSel = 0;
        }

        TimeBaseSel = ChannelSettings->WhichPcrToUse + 1;
    }

    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, OUTPUT_PIPE_SEL, ChannelSettings->RaveOutputOnly == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, TIMEBASE_SEL, TimeBaseSel ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, TIMESTAMP_MODE, ChannelSettings->TimestampMode )
        );
    if (ChannelSettings->PackHdrConfig == BXPT_Playback_PackHdr_Adaptation_Insert)
    {
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN, 1 );
    }

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

    /* PCR-based pacing */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, PACING_TYPE ) |
        BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT )
        );
    if( ChannelSettings->PcrBasedPacing == true )
    {
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_TYPE, 1 ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT, 1 )
            );
    }

#if BXPT_HAS_TSMUX
    else if( ChannelSettings->PesBasedPacing == true )
    {
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_TYPE, 2 )
            );
    }
#endif

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PACING_PCR_PID, PACING_PCR_PID )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PACING_PCR_PID, PACING_PCR_PID, ChannelSettings->PcrPacingPid )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID, Reg );

    /* Don't overwrite TIMESTAMP_MODE_32BIT if PCR pacing is in use */
    if( ChannelSettings->PcrBasedPacing == false )
    {
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
        Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT, ChannelSettings->Use32BitTimestamps == true ? 1 : 0 ) );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );
    }

    if ( ChannelSettings->ResetPacing)
    {
        hPb->ResetPacingOnTsRangeError = true;
        ExitCode = BINT_EnableCallback( hPb->hPbIntCallback );
        if (ExitCode)
            return BERR_TRACE(ExitCode);
    }
    else
    {
        hPb->ResetPacingOnTsRangeError = false;
        BINT_DisableCallback( hPb->hPbIntCallback );
    }

#if BXPT_HAS_TSMUX
    /* Set LLD Type */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, LLD_TYPE ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL1, LLD_TYPE, ChannelSettings->Use8WordDesc == true ? 1 : 0 ) );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
#endif

    return( ExitCode );
}

BERR_Code BXPT_Playback_GetChannelSettings(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel. */
    BXPT_Playback_ChannelSettings *ChannelSettings /* [out] The current settings  */
    )
{
    uint32_t Reg;

    uint32_t Timebase = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ChannelSettings );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    ChannelSettings->PacketLength = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, SYNC_LENGTH );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    ChannelSettings->SyncMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, SYNC_EXT_MODE );
    ChannelSettings->SwapBytes = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, ENDIAN_CTRL ) ? true : false;
    ChannelSettings->TimestampEn = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TIMESTAMP_EN ) ? true : false;
    ChannelSettings->DisableTimestampParityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS ) ? true : false;
    ChannelSettings->ResetPacing = hPb->ResetPacingOnTsRangeError;
    ChannelSettings->PacingOffsetAdjustDisable = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS ) ? true : false;
    ChannelSettings->Use32BitTimestamps = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT ) ? true : false;

    ChannelSettings->PsMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PROGRAM_STREAM_MODE );
    if( !BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PROGRAM_STREAM_EN ) )
        ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Drop;
    else
        ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Payload_Insert;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    ChannelSettings->TimestampMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMESTAMP_MODE );
    ChannelSettings->RaveOutputOnly = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, OUTPUT_PIPE_SEL ) ? true : false;
    Timebase = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMEBASE_SEL );

    /*
    ** In the Timebase check below, note that the hardware maps timebases with DPCR0 given a value of 1,
    ** DPCR1 is 2, etc. Thus, the check below is > as opposed to >=
    */
    if( Timebase == 0 )
        ChannelSettings->UsePcrTimeBase = false;
    else if( Timebase > BXPT_NUM_PCRS )
    {
        BDBG_ERR(( "Invalid timebase %u configured in hardware.", (unsigned) Timebase ));
        ChannelSettings->UsePcrTimeBase = false;
    }
    else
    {
        ChannelSettings->UsePcrTimeBase = true;
        ChannelSettings->WhichPcrToUse = Timebase - 1;
    }

    if( ChannelSettings->PackHdrConfig != BXPT_Playback_PackHdr_Drop )
    {
        if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN ) )
            ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Adaptation_Insert;
    }

    /* PCR-based pacing */
    ChannelSettings->PcrBasedPacing = false;
#if BXPT_HAS_TSMUX
    ChannelSettings->PesBasedPacing = false;
#endif
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_TYPE ) )
    {
        case 1:
        ChannelSettings->PcrBasedPacing = true;
        break;

#if BXPT_HAS_TSMUX
        case 2:
        ChannelSettings->PesBasedPacing = true;
        break;
#endif

        case 0:
        default:
        /* Do nothing. Timestamp mode is the hw default. */
        break;
    }

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID );
    ChannelSettings->PcrPacingPid = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PACING_PCR_PID, PACING_PCR_PID );

#if BXPT_HAS_TSMUX
    /* Get the LLD Type */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    ChannelSettings->Use8WordDesc = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, LLD_TYPE ) ? true : false;
#endif

    return( ExitCode );
}

#ifdef ENABLE_PLAYBACK_MUX
BERR_Code BXPT_Playback_SetChannelPacketSettings(
    BXPT_Playback_Handle hPb,                                  /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelPacketSettings *ChannelSettings /* [in] New settings to use */
    )
{
    uint32_t Reg;

    unsigned int PacketLength = 188;    /* Use MPEG length as the default. */
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    hPb->SyncMode = ChannelSettings->SyncMode;

    if( ChannelSettings->PacketLength > BXPT_PB_MAX_SYNC_LENGTH )
    {
        BDBG_ERR(( "Packet length %d too long! Clamped to %d", ChannelSettings->PacketLength, BXPT_PB_MAX_SYNC_LENGTH ));
        PacketLength = BXPT_PB_MAX_SYNC_LENGTH;
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        PacketLength = ChannelSettings->PacketLength;
    }

    /*set the packet length*/
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    Reg &= ~(BCHP_MASK( XPT_PB0_CTRL3, SYNC_LENGTH ));
    Reg |= (BCHP_FIELD_DATA( XPT_PB0_CTRL3, SYNC_LENGTH, PacketLength ));
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL3, Reg );

    /* Need to recalc the blockout if the packet size has changed */
    CalcAndSetBlockout( hPb, hPb->BitRate );

    /*set the packet sync*/
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE ) |
        BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_EN ) |
        BCHP_MASK( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS )
    );

    Reg |= (BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, ChannelSettings->SyncMode ));
    if (ChannelSettings->TimestampEn == true) {
        Reg |= (BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_EN, 1 ));
    }
    if (ChannelSettings->DisableTimestampParityCheck == true) {
        Reg |= (BCHP_FIELD_DATA( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS, 1 ));
    }

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );


    return( ExitCode );
}
#endif /*ENABLE_PLAYBACK_MUX*/

BERR_Code BXPT_Playback_GetChannelStatus(
    BXPT_Playback_Handle hPb,           /* [in] Handle for the playback channel. */
    BXPT_Playback_ChannelStatus *Status /* [out] Channel status. */
    )
{
    uint32_t Reg, CurrDescReg, CurrDescNotDone, IntEnReg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( Status );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    Status->Busy = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY ) ? true : false;
    Status->Run = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, RUN ) ? true : false;

    CurrDescReg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CURR_DESC_ADDR );
    CurrDescNotDone = BCHP_GET_FIELD_DATA( CurrDescReg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_NOT_DONE );
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    Status->Finished = ~CurrDescNotDone && BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, FINISHED ) && Status->Run;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_INTR );
    Status->OutOfSync = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_INTR, SE_OUT_OF_SYNC_INT ) ? true : false;

    /* We can clear the status bit here if the interrupt isn't enabled */
    IntEnReg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_INTR_EN );
    if( !BCHP_GET_FIELD_DATA( IntEnReg, XPT_PB0_INTR, SE_OUT_OF_SYNC_INT ))
    {
        Reg = ~(
            BCHP_MASK( XPT_PB0_INTR, SE_OUT_OF_SYNC_INT )
        );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_INTR, Reg );
    }

    return( ExitCode );
}

BERR_Code BXPT_Playback_GetCurrentBufferAddress(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t *Address                       /* [out] The address read from hardware. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CURR_BUFF_ADDR );
    *Address = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_BUFF_ADDR, CURR_BUFF_ADDR );

    return( ExitCode );
}

BERR_Code BXPT_Playback_GetCurrentDescriptorAddress(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor **LastDesc       /* [in] Address of the current descriptor. */
    )
{
    uint32_t Reg, CurrentDescAddr;
    void *UserDescAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    *LastDesc = NULL;
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CURR_DESC_ADDR );
    CurrentDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_ADDR );
    if( CurrentDescAddr )
    {
        CurrentDescAddr <<= 4;  /* Convert to byte-address. */
        BMEM_ConvertOffsetToAddress( hPb->hMemory, CurrentDescAddr, ( void ** ) &UserDescAddr );
        BMEM_ConvertAddressToCached(hPb->hMemory, UserDescAddr, &UserDescAddr);
        *LastDesc = ( BXPT_PvrDescriptor * ) UserDescAddr;
    }

    return( ExitCode );
}

BERR_Code BXPT_Playback_CreateDesc(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc,        /* [in] Descriptor to initialize */
    uint8_t *Buffer,                        /* [in] Data buffer. */
    uint32_t BufferLength,                  /* [in] Size of buffer (in bytes). */
    bool IntEnable,                         /* [in] Interrupt when done? */
    bool ReSync,                            /* [in] Re-sync extractor engine? */
    BXPT_PvrDescriptor * const NextDesc     /* [in] Next descriptor, or NULL */
    )
{
    BDBG_ASSERT( Desc );
    return BXPT_Playback_P_CreateDesc( hXpt, Desc, Buffer, BufferLength, IntEnable, ReSync, NextDesc );
}

void BXPT_SetLastDescriptorFlag(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc     /* [in] Descriptor to initialize */
    )
{
    BMEM_Handle hHeap;
    BXPT_PvrDescriptor *CachedDescPtr;

    BDBG_ASSERT( Desc );

    if( hXpt->hPbHeap )
        hHeap = hXpt->hPbHeap;
    else
        hHeap = hXpt->hMemory;

    /*  Set the Last Descriptor bit. */
    CachedDescPtr = Desc;
    CachedDescPtr->NextDescAddr = TRANS_DESC_LAST_DESCR_IND;
    BMEM_FlushCache(hHeap, CachedDescPtr, sizeof (BXPT_PvrDescriptor) );
}

BERR_Code BXPT_Playback_AddDescriptors(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *LastDesc,   /* [in] Last descriptor in new chain */
    BXPT_PvrDescriptor *FirstDesc   /* [in] First descriptor in new chain */
    )
{
    uint32_t Reg;
    uint32_t DescPhysAddr;
    BXPT_PvrDescriptor *CachedFirstDesc, *CachedDescPtr;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_PvrDescriptor *LastDescriptor_Cached = ( BXPT_PvrDescriptor * ) hPb->LastDescriptor_Cached;

    BDBG_ASSERT( FirstDesc );
    CachedFirstDesc = FirstDesc;

    /* When the sync extractor is in bypass mode, the first descriptor must have it's FORCE_RESYNC flag set. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    if( !LastDescriptor_Cached && BXPT_PB_SYNC_BYPASS == BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, SYNC_EXT_MODE ) )
    {
        CachedFirstDesc->Flags |= TRANS_DESC_FORCE_RESYNC_FLAG;
    }

    BMEM_FlushCache(hPb->hMemory, CachedFirstDesc, sizeof (BXPT_PvrDescriptor) );

    if( hPb->ForceResync )
    {
        /* cycle through all descriptors and set force_resync flag */
        BXPT_PvrDescriptor *TempDesc;
        BXPT_PvrDescriptor *Desc = CachedFirstDesc;
        BXPT_PvrDescriptor *HeadOfChain = CachedFirstDesc;

        do
        {
            Desc->Flags |= TRANS_DESC_FORCE_RESYNC_FLAG;
            BMEM_FlushCache(hPb->hMemory, Desc, sizeof (BXPT_PvrDescriptor) );

            if( Desc->NextDescAddr == TRANS_DESC_LAST_DESCR_IND )
                break;

            BMEM_ConvertOffsetToAddress( hPb->hMemory, Desc->NextDescAddr, ( void ** ) &TempDesc );
            Desc = TempDesc;
            BMEM_FlushCache(hPb->hMemory, Desc, sizeof (BXPT_PvrDescriptor) );
        }
        while( Desc != HeadOfChain );
    }

#if 0
    /* Walk through the list list, dumping each descriptor's contents */
    if( 3 == hPb->ChannelNo ) /* Dump only data for channel 3 */
    {
        #define FLAG_STR_LEN 40
        char FlagsStr[ FLAG_STR_LEN ];

        BXPT_PvrDescriptor *Desc = CachedFirstDesc;
        BXPT_PvrDescriptor *HeadOfChain = CachedFirstDesc;

        do
        {
            #include  <stdio.h>

            FILE *Fp;
            void *Buffer, *CachedBuffer;
            BXPT_PvrDescriptor *TempDesc;

            Fp = fopen( "/data/videos/pb_dump.mpg", "ab" );

            BMEM_ConvertOffsetToAddress( hPb->hMemory, Desc->BufferStartAddr, ( void ** ) &Buffer );
            BMEM_ConvertAddressToCached(hPb->hMemory, (void *) Buffer, (void **) &CachedBuffer);
            BMEM_FlushCache(hPb->hMemory, CachedBuffer, Desc->BufferLength );
            fwrite( CachedBuffer, 1, Desc->BufferLength, Fp );
            fclose( Fp );

            BKNI_Snprintf( FlagsStr, FLAG_STR_LEN, "%s%s",
                     Desc->Flags & TRANS_DESC_FORCE_RESYNC_FLAG ? "Force Resync,": "",
                     Desc->Flags & TRANS_DESC_INT_FLAG ? "Interrupt": ""
                     );
            BDBG_MSG(( "Desc 0x%08lX: BufferStartAddr 0x%08lX, BufferLength %u, Flags 0x%08lX (%s), NextDescAddr 0x%08lX",
                       Desc, Desc->BufferStartAddr, Desc->BufferLength, Desc->Flags, FlagsStr, Desc->NextDescAddr ));

            if( Desc->NextDescAddr == TRANS_DESC_LAST_DESCR_IND )
                break;
            BMEM_ConvertOffsetToAddress( hPb->hMemory, Desc->NextDescAddr, ( void ** ) &TempDesc );
            BMEM_ConvertAddressToCached( hPb->hMemory, (void *) TempDesc, (void **) &Desc);
            BMEM_FlushCache(hPb->hMemory, Desc, sizeof (BXPT_PvrDescriptor) );
        }
        while( Desc != HeadOfChain );
    }
#endif

    BMEM_ConvertAddressToOffset( hPb->hMemory, ( void * ) FirstDesc, &DescPhysAddr );
    /* fail if descriptor not on MEMC0 */
    if (!BCHP_OffsetOnMemc(hPb->hChip, DescPhysAddr, 0)) {
        BDBG_ERR(("Descriptor at offset 0x%08x is not on MEMC0 and inaccessible by default RTS", DescPhysAddr));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if( LastDescriptor_Cached )
    {
        /* Channel has a linked-list loaded already. Append this descriptor to the end and use WAKE_MODE = 0 (resume from the last descriptor). */

        /* Set the last descriptor in the chain to point to the descriptor we're adding. */
        LastDescriptor_Cached->NextDescAddr = ( uint32_t ) DescPhysAddr;
        BMEM_FlushCache(hPb->hMemory, LastDescriptor_Cached, sizeof (BXPT_PvrDescriptor) );

        /* SWDTV-8330: read register after uncache memory write operation to maintain consistency */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
        Reg &= ~ ( BCHP_MASK( XPT_PB0_CTRL1, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 1 );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
    }
    else
    {
        /* Channel does not have a linked-list loaded. Point the FIRST_DESC_ADDR to this descriptor and set RUN */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_FIRST_DESC_ADDR );
        Reg &= ~( BCHP_MASK( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR ) );

        /*
        ** The descriptor address field in the hardware register is wants the address
        ** in 16-byte blocks. See the RDB HTML for details. So, we must shift the
        ** address 4 bits to the right before writing it to the hardware. Note that
        ** the RDB macros will shift the value 4 bits to the left, since the address
        ** bitfield starts at bit 4. Confusing, but thats what the hardware and the
        ** RDB macros require to make this work.
        */
        DescPhysAddr >>= 4;
        Reg |= BCHP_FIELD_DATA( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR, DescPhysAddr );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_FIRST_DESC_ADDR, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
        Reg &= ~ (
                  BCHP_MASK( XPT_PB0_CTRL1, WAKE_MODE ) |
                  BCHP_MASK( XPT_PB0_CTRL1, WAKE ) |
                  BCHP_MASK( XPT_PB0_CTRL1, RUN )
        );
        Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 1 )
        );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
    }

    if ( LastDesc )
    {
        CachedDescPtr = LastDesc;
        hPb->LastDescriptor_Cached = ( uint32_t ) CachedDescPtr;
    }
    else
    {
        hPb->LastDescriptor_Cached = 0;
    }
    return ExitCode;
}

BERR_Code BXPT_Playback_StartChannel(
    BXPT_Playback_Handle PlaybackHandle     /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

    BDBG_MSG(( "Starting playback channel %u", PlaybackHandle->ChannelNo ));

    if( PlaybackHandle->Running == true )
    {
        BDBG_ERR(( "Playback channel %u cannot be started because it's already running!",
            PlaybackHandle->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_RUNNING );
    }

#ifdef BCHP_PWR_RESOURCE_XPT_PLAYBACK
    if( PlaybackHandle->Running == false )
    {
        BCHP_PWR_AcquireResource(PlaybackHandle->hChip, BCHP_PWR_RESOURCE_XPT_PLAYBACK);
    }
#endif

    /* Check if we have buffers already loaded for this channel */
    if( PlaybackHandle->LastDescriptor_Cached )
    {
        /* Since we already have some buffers loaded, we can start the pvr channel */
        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 1 );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );
    }

    /* Need to enable the parser band. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ENABLE, 1 )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

    PlaybackHandle->Running = true;

    return( ExitCode );
}


BERR_Code BXPT_Playback_StopChannel(
    BXPT_Playback_Handle PlaybackHandle     /* [in] Handle for the playback channel */
    )
{
    volatile uint32_t Reg;
    uint32_t PacingEn;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

    BDBG_MSG(( "Stopping playback channel %u", PlaybackHandle->ChannelNo ));

    if( PlaybackHandle->Running == false )
    {
        BDBG_ERR(( "Playback channel %u cannot be stopped because it's not running!",
            PlaybackHandle->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_STOPPED );
    }

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    PacingEn = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_EN );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, PACING_EN )
        );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

    /* Stop the channel hardware */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, DISCARD_DMA_DATA, 1 )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );
    {
        int Wait;

/* HW team confirmed we need at least 8 * 188 * 32 / 25 Mbps = 1.925 ms
 * drain delay for playback stop to drain the stuff between playback DMA and
 * Rave (basically RS, XC, and a few packet buffers) */
#define DRAIN_DELAY 2000

        /* Disable pausing from the XC buffer. Allows data to drain */
        BXPT_XcBuf_P_EnablePlaybackPausing( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo, false );
        for( Wait = 0; Wait < DRAIN_DELAY; Wait++ )
        {
            BKNI_Delay( 1 );  /* Busy wait for 1 uS */
        }

        /* Re-enable pausing from the XC buffer. Prevents overflows in RAVE. */
        BXPT_XcBuf_P_EnablePlaybackPausing( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo, true );
    }

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, RUN ) );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

    /* Clear the first desc addr (for cleaner debugging) */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR );
    Reg &= ~( BCHP_MASK( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR ) );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR, Reg );

    /* Clear the host pause. This shouldn't be restored, though. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, MICRO_PAUSE )
        );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

    PlaybackHandle->LastDescriptor_Cached = 0;

#if 1
    /* Keep this around, just in case. */
    {
        uint32_t Busy;
        int WaitCount;
        uint32_t FirstDescAddr, SyncExtMode;
        uint32_t DescPhysAddr;
        uint32_t DescNotDone;

        /* Wait for the BUSY bit to clear. */
        WaitCount = 100;          /* 100 uS max wait before we declare failure */
        do
        {
            Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
            Busy = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY ) ? true : false;

            if( Busy )
            {
                WaitCount--;
                if( !WaitCount )
                {
                    BDBG_WRN(( "Playback channel %u timed-out waiting for BUSY bit to clear", PlaybackHandle->ChannelNo ));
                    break;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }
        }
        while( Busy );

        BKNI_Delay( 10 );   /* Wait 10 uS; */
        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR );
        FirstDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR );
        Reg &= ~( BCHP_MASK( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR ) );
        BMEM_ConvertAddressToOffset( PlaybackHandle->hMemory, ( void * ) PlaybackHandle->DummyDescriptor, &DescPhysAddr );
        DescPhysAddr >>= 4;
        Reg |= BCHP_FIELD_DATA( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR, DescPhysAddr );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR, Reg );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
        SyncExtMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, SYNC_EXT_MODE );
        Reg &= ~BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, 3 );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 1 );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

        WaitCount = 100;          /* 100 uS max wait before we declare failure */
        do
        {
            Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CURR_DESC_ADDR );
            DescNotDone = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_NOT_DONE );

            if( DescNotDone )
            {
                WaitCount--;
                if( !WaitCount )
                {
                    BDBG_WRN(( "Playback channel %u timed-out waiting for CURR_DESC_NOT_DONE bit to clear", PlaybackHandle->ChannelNo ));
                    break;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }

        }
        while( DescNotDone );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
        Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, RUN ) );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

        WaitCount = 100;          /* 100 uS max wait before we declare failure */
        do
        {
            Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
            Busy = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY ) ? true : false;

            if( Busy )
            {
                WaitCount--;
                if( !WaitCount )
                {
                    BDBG_WRN(( "Playback channel %u timed-out waiting for BUSY bit to clear, 2nd loop", PlaybackHandle->ChannelNo ));
                    break;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }
        }
        while( Busy );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR );
        Reg &= ~( BCHP_MASK( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR, FirstDescAddr );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR, Reg );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
        Reg &= ~BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, SyncExtMode );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );
    }
#endif

    /* Need to disable the parser band. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ENABLE )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

    /* Stop dropping data in the DMA buffers. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, DISCARD_DMA_DATA ) );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

    /* Restore the pacing enable. */
    if( PacingEn )
    {
        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_EN, 1 )
            );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );
    }

#ifdef BCHP_PWR_RESOURCE_XPT_PLAYBACK
    if (PlaybackHandle->Running) {
        BCHP_PWR_ReleaseResource(PlaybackHandle->hChip, BCHP_PWR_RESOURCE_XPT_PLAYBACK);
    }
#endif

    PlaybackHandle->Running = false;

    return( ExitCode );
}


BERR_Code BXPT_Playback_Pause(
    BXPT_Playback_Handle PlaybackHandle     /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, MICRO_PAUSE ) );
    Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, MICRO_PAUSE, 1 );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

    return( ExitCode );
}


BERR_Code BXPT_Playback_Resume(
    BXPT_Playback_Handle PlaybackHandle     /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_EN ) )
    {
        TsRangeErrorIsr( PlaybackHandle, 0 );
    }

    /* Reread the register, since TsRangeErrorIsr() modifies it. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, MICRO_PAUSE ) );
    Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, MICRO_PAUSE, 0 );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

    return( ExitCode );
}

BERR_Code BXPT_Playback_SetBitRate(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t BitRate            /* [in] Rate, in bits per second. */
    )
{
    BDBG_ASSERT( hPb );

    if( BitRate < BXPT_MIN_PLAYBACK_RATE )
    {
        BDBG_ERR(( "Minimum playback rate is %u bps. Rate will be clamped to that value", BXPT_MIN_PLAYBACK_RATE ));
        BitRate = BXPT_MIN_PLAYBACK_RATE;
    }
    else if ( BitRate > BXPT_MAX_PLAYBACK_RATE )
    {
        BDBG_ERR(( "Maximum playback rate is %u bps. Rate will be clamped to that value", BXPT_MAX_PLAYBACK_RATE ));
        BitRate = BXPT_MAX_PLAYBACK_RATE;
    }

    hPb->BitRate = BitRate;
    return CalcAndSetBlockout( hPb, BitRate );
}

BERR_Code CalcAndSetBlockout(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t BitRate            /* [in] Rate, in bits per second. */
    )
{
    uint32_t Reg, SyncLen, BoCount;

    BERR_Code ExitCode = BERR_SUCCESS;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    SyncLen = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, SYNC_LENGTH );

#if BXPT_SW7425_1323_WORKAROUND
    {
        BXPT_Handle lhXpt = (BXPT_Handle) hPb->vhXpt;

        if( lhXpt->DoWorkaround )
            BitRate = BitRate <= 30000000 ? BitRate : 30000000;
    }
#endif

    if(SyncLen == 0xB8)
        SyncLen = 0xBC;

    /* Do the math in units of 10kbps to avoid overflowing the 24-bit blockout bitfield in the register. */
    BoCount = (10800 * SyncLen * 8) / ( BitRate / 10000 );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_BLOCKOUT );
    Reg &= ~(
         BCHP_MASK( XPT_PB0_BLOCKOUT, BO_COUNT ) |
         BCHP_MASK( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN )
         );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_BLOCKOUT, BO_COUNT, BoCount )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_BLOCKOUT, Reg );

    return( ExitCode );
}


BERR_Code BXPT_Playback_CheckHeadDescriptor(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,       /* [in] Descriptor to check. */
    bool *InUse,                    /* [out] Is descriptor in use? */
    uint32_t *BufferSize            /* [out] Size of the buffer (in bytes). */
    )
{
    uint32_t Reg, ChanBusy, CurrentDescNotDone, CurrentDescAddr, CandidateDescPhysAddr;
    uint32_t DescFinish;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( PlaybackHandle );

    /*
    ** Check if the current descriptor being processed by the
    ** playback hardware is the first on our hardware list
    ** (which means this descriptor is still being used)
    */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CURR_DESC_ADDR );
    CurrentDescNotDone = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_NOT_DONE );
    DescFinish = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, FINISHED_SHADOW ) ;
    DescFinish = (~CurrentDescNotDone && DescFinish) ;
    CurrentDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_ADDR );
    CurrentDescAddr <<= 4;  /* Convert to byte-addressing */

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    ChanBusy = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY );

    BMEM_ConvertAddressToOffset( PlaybackHandle->hMemory, ( void * ) Desc, &CandidateDescPhysAddr );

    if( CurrentDescAddr == CandidateDescPhysAddr )
    {
/*        if( ChanBusy && CurrentDescNotDone ) */
    if( !DescFinish )
        {
            /* The candidate descriptor is being used by hardware. */
            *InUse = true;
        }
        else
        {
            *InUse = false;
        }
    }
    else
    {
        /*
        ** The candidate descriptor isn't being processed. If this is the head descriptor
        ** we can conclude that the hardware is finished with the descriptor.
        */
        *InUse = false;

#if 0
/* Keep this around, just in case */
        if (Desc->NextDescAddr == 1)
        {
           /* If there's no next descriptor and the current descriptor isn't the head descriptor,
            * the head descriptor hasn't been read by the HW yet.  It's still InUse.
            */
           *InUse = true;
        }
        else
        {
            /* Check if our head descriptor is after the current one */
            BXPT_PvrDescriptor *  CurrentHwDesc;
            BMEM_ConvertOffsetToAddress( PlaybackHandle->hMemory, CurrentDescAddr, ( void * ) &CurrentHwDesc );

            /* If the head descriptor is the next descriptor, its still InUse */
            if (CurrentHwDesc->NextDescAddr == CandidateDescPhysAddr){
                *InUse = true;
            }
        }
#endif
    }

    if( *InUse == false )
    {
        BXPT_PvrDescriptor *CachedDescPtr;
        CachedDescPtr = Desc;
        BMEM_FlushCache(PlaybackHandle->hMemory, CachedDescPtr, sizeof (BXPT_PvrDescriptor) );
        *BufferSize = CachedDescPtr->BufferLength;
    }
    else
    {
        *BufferSize = 0;
    }

    return( ExitCode );
}

BERR_Code BXPT_Playback_GetTimestampUserBits(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    unsigned int *Bits                          /* [out] The user bits read from hardware. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    *Bits = ( int ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMESTAMP_USER_BITS ) & 0xFF );

    return( ExitCode );
}


BERR_Code BXPT_Playback_SetPacingErrorBound_impl(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel */
    unsigned long MaxTsError        /* [in] Maximum timestamp error. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    /* Newer chips */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND_EARLY );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_TS_ERR_BOUND_EARLY, TS_ERR_BOUND )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_TS_ERR_BOUND_EARLY, TS_ERR_BOUND, MaxTsError )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND_EARLY, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND_LATE );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_TS_ERR_BOUND_LATE, TS_ERR_BOUND )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_TS_ERR_BOUND_LATE, TS_ERR_BOUND, MaxTsError )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND_LATE, Reg );

    return( ExitCode );
}

BERR_Code BXPT_Playback_ConfigPacing(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PacingControl Mode                 /* [in] New mode for pacing. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    if( Mode == BXPT_PacingControl_eStart )
    {
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_EN, 1 )
        );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );
        BXPT_Playback_SetPacingErrorBound_impl(hPb, hPb->maxTsError);
    }
    else
    {
        BXPT_Playback_SetPacingErrorBound_impl(hPb, 0);
    }

    return( ExitCode );
}

BERR_Code BXPT_Playback_SetPacingErrorBound(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel */
    unsigned long MaxTsError        /* [in] Maximum timestamp error. */
    )
{
    hPb->maxTsError = MaxTsError;
    return BXPT_Playback_SetPacingErrorBound_impl(hPb, MaxTsError);
}

void BXPT_Playback_GetPacketizerDefaults(
    const BXPT_Playback_PacketizeConfig *Cfg   /* [in] Config to initialize */
    )
{
    BDBG_ASSERT( Cfg );
    BKNI_Memset( (void *) Cfg, 0, sizeof( BXPT_Playback_PacketizeConfig ) );
}

BERR_Code BXPT_Playback_PacketizeStream(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    unsigned Context,                           /* [in] Which context to map the packets to. */
    const BXPT_Playback_PacketizeConfig *Cfg,   /* [in] Configuration for this context */
    bool Enable                                 /* [in] Start or stop packetization. */
    )
{
    /* In _CloseChannel(), disable all the packetizers */
    unsigned Index;
    uint32_t Reg, RegAddr;
    int32_t i;
    BXPT_Spid_eChannelFilter SidFilter;
    BXPT_Handle hXpt;
    BXPT_PidChannel_CC_Config ccConfig;

    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( Context );
    BDBG_ASSERT( hPb );
    BDBG_ASSERT( Cfg );

    hXpt = (BXPT_Handle)hPb->vhXpt;

    if( Enable == true )
    {
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
        Reg &= ~(
                 BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE )
        );
        if (Cfg->PacketizerMode == BXPT_Playback_PacketizerMode_Es)
        {
            hPb->ForceResync = true; /* for ES playback, the FORCE_RESYNC flag must be set for all descriptors. see SW7425-1672 */
            Reg &= ~(
                 BCHP_MASK( XPT_PB0_CTRL2, PROGRAM_STREAM_EN )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, BXPT_PB_SYNC_BYPASS )
            );
        }
        else
        {
            hPb->ForceResync = false;
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, BXPT_PB_SYNC_PES )
            );
        }
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 0x00 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, 1 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS ) |
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
            BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN )|
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PT_EN )
            );

        switch(Cfg->PacketizerMode)
        {
        case BXPT_Playback_PacketizerMode_Es:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = 0xFF;
            SidFilter.FilterConfig.StreamIdRange.Lo = 0x00;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 0 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 0 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_MapAll:
            /* Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN, 1 );*/
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = 0xff;
            SidFilter.FilterConfig.StreamIdRange.Lo = 0x00;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_Sid:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamId;
            SidFilter.FilterConfig.StreamId = Cfg->FilterConfig.StreamId;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidRange:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = Cfg->FilterConfig.StreamIdRange.Hi;
            SidFilter.FilterConfig.StreamIdRange.Lo = Cfg->FilterConfig.StreamIdRange.Lo;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidExtension:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdExtension;
            SidFilter.FilterConfig.StreamIdAndExtension.Id = Cfg->FilterConfig.StreamIdAndExtension.Id;
            SidFilter.FilterConfig.StreamIdAndExtension.Extension = Cfg->FilterConfig.StreamIdAndExtension.Extension;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidSubSid:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_SubStreamId;
            SidFilter.FilterConfig.StreamIdAndSubStreamId.Id = Cfg->FilterConfig.StreamIdAndSubStreamId.Id;
            SidFilter.FilterConfig.StreamIdAndSubStreamId.SubStreamId = Cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;
       }
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PACKETIZE_EN, 1 );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

        /* Disable CC checking, but enable CC generation in the full parser. */
        BXPT_GetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
        ccConfig.Primary_CC_CheckEnable = false;
        ccConfig.Generate_CC_Enable = true;
        BXPT_SetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
    }
    else
    {
        hPb->ForceResync = false;
        SidFilter.Mode = BXPT_Spid_eChannelFilterMode_Disable;
        BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

        BXPT_GetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
        ccConfig.Primary_CC_CheckEnable = true;
        ccConfig.Generate_CC_Enable = false;
        BXPT_SetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );

        /*
        ** SCheck the packetizer in each SPID/PID channel. If all the channels that are mapped to
        ** this playback are Invalid, disable packetizing at the PB control reg.
        */
       for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
        {
            unsigned int Band, Pid;
            bool IsPlayback;

            BXPT_GetPidChannelConfig( hXpt, Index, &Pid, &Band, &IsPlayback );

            RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * Index );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );

            i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );

            /* if this pid channnel is used for spid functions or disabled */
            if((i >= BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
            && Band == hPb->ChannelNo && IsPlayback )
                continue;
            else
                break;
        }

        if( Index == BXPT_NUM_PID_CHANNELS )
        {
            /* All contexts where invalid, so disable packetizer in the PB control reg. */
            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
                BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN ) |
                BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PT_EN )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
                );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, BXPT_PB_SYNC_MPEG_BLIND )
            );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );
        }
    }

    return( ExitCode );
}


BERR_Code BXPT_Playback_DisablePacketizers(
    BXPT_Playback_Handle hPb                    /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg, RegAddr;
    unsigned Index;

    BERR_Code ExitCode = BERR_SUCCESS;

    {
        int32_t i;
        BXPT_Handle hXpt = (BXPT_Handle)hPb->vhXpt;

        for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
        {
            /* if the pid channel input is this parser band cleanup the SPID table */
            RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( 4 * Index );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );

            if(!BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i,PLAYBACK_FE_SEL))
                continue;
            /* pb parser is the input */
            if(BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i,PLAYBACK_BAND_PARSER_PID_CHANNEL_INPUT_SELECT)!= hPb->ChannelNo)
                continue;

            RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * Index );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );

            i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );

            /* if this pid channnel is used for spid functions or disabled */
            if(i >= BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
                    continue;
            else
            {
                Reg &= ~(
                BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
                BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI ) |
                BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO )
                );

                BREG_Write32( hXpt->hRegister, RegAddr, Reg );
            }
        }
    }

    /* Disable packetizer in the PB control reg. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
        BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN ) |
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PT_EN )|
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN )|
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

    /* These parser settings are always loaded when BXPT_Playback_PacketizeStream() is called. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

    return ExitCode;
}

BERR_Code BXPT_Playback_GetParserConfig(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_ParserConfig *ParserConfig    /* [out] The current settings */
    )
{
    uint32_t Reg;
    BXPT_Handle hXpt;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ParserConfig );

    hXpt = ( BXPT_Handle ) hPb->vhXpt;

    /* The parser config registers are at consecutive addresses. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );

    ParserConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE );
    ParserConfig->AllPass = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD );
    ParserConfig->AcceptNulls = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD );
    ParserConfig->RestampInBinary = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE ) == 3 ? true : false;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    ParserConfig->ForceRestamping = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP );

#if BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00_PB0_ACCEPT_ADP_00_MASK != 0x00000001 || BXPT_NUM_PLAYBACKS > 32
    #error "PI NEEDS UPDATING"
#else
    Reg = BREG_Read32( hPb->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 );
    ParserConfig->AcceptAdapt00 = (Reg >> hPb->ChannelNo) & 0x01 ? true : false;
#endif

    return( ExitCode );
}

BERR_Code BXPT_Playback_SetParserConfig(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    const BXPT_Playback_ParserConfig *ParserConfig  /* [in] The new settings */
    )
{
    uint32_t Reg;
    BXPT_Handle hXpt;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ParserConfig );

    hXpt = ( BXPT_Handle ) hPb->vhXpt;

    /* The parser config registers are at consecutive addresses. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );

    /* Clear all the bits we are about to change. */
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
    );

    /* Now set the new values. */
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE, ParserConfig->RestampInBinary ? 3 : 2 ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE, ParserConfig->ErrorInputIgnore ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD, ParserConfig->AllPass == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, ParserConfig->AcceptNulls )
    );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

    if( ParserConfig->AllPass && false == hPb->IsParserInAllPass )
    {
        BXPT_PidChannel_CC_Config AllCcDisabled = { false, false, false, 0 };

        /* Remember the config before entering all-pass. */
        BXPT_GetPidChannel_CC_Config( hXpt, BXPT_GET_PLAYBACK_ALLPASS_CHNL(hPb->ChannelNo) , &hPb->CcConfigBeforeAllPass );

        /* Now disable the CC checks */
        BXPT_SetPidChannel_CC_Config( hXpt, BXPT_GET_PLAYBACK_ALLPASS_CHNL(hPb->ChannelNo), &AllCcDisabled );
        hPb->IsParserInAllPass = true;
    }
    else if( false == ParserConfig->AllPass )
    {
        /* Restore the config we had before entering all-pass. */
        BXPT_SetPidChannel_CC_Config( hXpt, BXPT_GET_PLAYBACK_ALLPASS_CHNL(hPb->ChannelNo), &hPb->CcConfigBeforeAllPass );
        hPb->IsParserInAllPass = false;
    }

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP, ParserConfig->ForceRestamping ) );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

#if BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00_PB0_ACCEPT_ADP_00_MASK != 0x00000001 || BXPT_NUM_PLAYBACKS > 32
    #error "PI NEEDS UPDATING"
#else
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 );
        Reg &= ~(0x01 << hPb->ChannelNo);
        if( ParserConfig->AcceptAdapt00 )
            Reg |= (0x01 << hPb->ChannelNo);
        BREG_Write32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00, Reg );
#endif

    return( ExitCode );
}

void BXPT_Playback_P_WriteReg(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr,
    uint32_t RegVal
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_PB0_CTRL1 + PlaybackHandle->BaseAddr;

    BREG_Write32( PlaybackHandle->hRegister, RegAddr, RegVal );
}


uint32_t BXPT_Playback_P_ReadReg(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_PB0_CTRL1 + PlaybackHandle->BaseAddr;

    return( BREG_Read32( PlaybackHandle->hRegister, RegAddr ));
}


BERR_Code BXPT_Playback_P_CreateDesc(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc,        /* [in] Descriptor to initialize */
    uint8_t *Buffer,                        /* [in] Data buffer. */
    uint32_t BufferLength,                  /* [in] Size of buffer (in bytes). */
    bool IntEnable,                         /* [in] Interrupt when done? */
    bool ReSync,                            /* [in] Re-sync extractor engine? */
    BXPT_PvrDescriptor * const NextDesc     /* [in] Next descriptor, or NULL */
    )
{
    uint32_t BufferPhysicalAddr;
    uint32_t ThisDescPhysicalAddr;
    BXPT_PvrDescriptor *CachedDescPtr;

    BERR_Code ExitCode = BERR_SUCCESS;
    BMEM_Handle hHeap = hXpt->hPbHeap ? hXpt->hPbHeap : hXpt->hMemory;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Desc );
    BDBG_ASSERT( Buffer );

    BMEM_ConvertAddressToOffset( hHeap, ( void * ) Buffer, &BufferPhysicalAddr );

    /* Verify that the descriptor we're creating sits on a 16-byte boundary. */
    BMEM_ConvertAddressToOffset( hHeap, ( void * ) Desc, &ThisDescPhysicalAddr );
    if( ThisDescPhysicalAddr % 16 )
    {
        BDBG_ERR(( "Desc is not 16-byte aligned!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    CachedDescPtr = Desc;

    /* Load the descriptor's buffer address, length, and flags. */
    CachedDescPtr->BufferStartAddr = BufferPhysicalAddr;
    CachedDescPtr->BufferLength = BufferLength;

    /* Clear everything, then set the ones we want below. */
    CachedDescPtr->Flags = 0;

    if( IntEnable )
        CachedDescPtr->Flags |= TRANS_DESC_INT_FLAG;

    if( ReSync )
        CachedDescPtr->Flags |= TRANS_DESC_FORCE_RESYNC_FLAG;

    /* Load the pointer to the next descriptor in the chain, if there is one. */
    if( NextDesc != 0 )
    {
        /* There is a another descriptor in the chain after this one. */
        uint32_t NextDescPhysAddr;

        BMEM_ConvertAddressToOffset( hHeap, ( void * ) NextDesc, &NextDescPhysAddr );
        if( NextDescPhysAddr % 16 )
        {
            BDBG_ERR(( "NextDescDesc is not 32-bit aligned!" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        }

        /* Next descriptor address must be 16-byte aligned. */
        NextDescPhysAddr &= ~( 0xF );
        CachedDescPtr->NextDescAddr = NextDescPhysAddr;
    }
    else
    {
        /* There is NOT another descriptor. Set the Last Descriptor bit. */
        CachedDescPtr->NextDescAddr = TRANS_DESC_LAST_DESCR_IND;
    }

    BMEM_FlushCache(hHeap, CachedDescPtr, sizeof (BXPT_PvrDescriptor) );
    return( ExitCode );
}

#ifdef ENABLE_PLAYBACK_MUX
void BXPT_Playback_SetDescBuf(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc,        /* [in] Descriptor to initialize */
    uint8_t *Buffer,                        /* [in] Data buffer. */
    uint32_t BufferLength                   /* [in] Size of buffer (in bytes). */
    )
{
    BXPT_PvrDescriptor *CachedDescPtr;
    BMEM_Handle hHeap = hXpt->hPbHeap ? hXpt->hPbHeap : hXpt->hMemory;

    CachedDescPtr = Desc;
    BMEM_ConvertAddressToOffset(hHeap, ( void * ) Buffer, &(CachedDescPtr->BufferStartAddr));
    CachedDescPtr->BufferLength = BufferLength;
    BMEM_FlushCache(hHeap, CachedDescPtr, sizeof (BXPT_PvrDescriptor) );
}
#endif /*ENABLE_PLAYBACK_MUX*/

BINT_Id BXPT_Playback_GetIntId(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_Int PbInt
    )
{
    uint32_t RegAddr;

    BDBG_ASSERT( hPb );

    RegAddr = BCHP_XPT_PB0_INTR - BCHP_XPT_PB0_CTRL1 + hPb->BaseAddr;
    return BCHP_INT_ID_CREATE( RegAddr, PbInt );
}

#if BXPT_HAS_TSMUX

void BXPT_Playback_GetMuxingInfo(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_MuxingInfo *Info
    )
{
    BDBG_ASSERT( hPb );
    BDBG_ASSERT( Info );

    Info->uiPacingCounter = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PACING_COUNTER );
}

BERR_Code BXPT_Playback_P_SetPacingSpeed(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    unsigned Speed
    )
{
    uint32_t Reg;

    unsigned PacingSpeedBitField = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );

    switch( Speed )
    {
        case 1: PacingSpeedBitField = 0; break;
        case 2: PacingSpeedBitField = 1; break;
        case 4: PacingSpeedBitField = 2; break;
        case 8: PacingSpeedBitField = 3; break;
        case 16: PacingSpeedBitField = 4; break;
        case 32: PacingSpeedBitField = 5; break;
        case 64: PacingSpeedBitField = 6; break;
        case 128: PacingSpeedBitField = 7; break;

        default:
        BDBG_ERR(( "Invalid pacing speed %u", Speed ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
        break;
    }

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, PACING_SPEED ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_SPEED, PacingSpeedBitField ) );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Done:
    return ExitCode;
}

void BXPT_Playback_P_SetPacingCount(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    unsigned PacingLoadMap,
    unsigned PacingCount
    )
{
    uint32_t Reg = PacingLoadMap;

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_NEXT_PACKET_PACING_TIMESTAMP, PacingCount + 0xff );

    /* Done to intentionally clear LD_STC_MUX_DELAY_DIFF */
    Reg &= ~( BCHP_MASK( XPT_PB_TOP_PACING_COUNT_RELOAD_CTRL, LD_STC_MUX_DELAY_DIFF ) );

    BREG_Write32( hPb->hRegister, BCHP_XPT_PB_TOP_PACING_COUNT_WR_VALUE, PacingCount );
    BREG_Write32( hPb->hRegister, BCHP_XPT_PB_TOP_PACING_COUNT_RELOAD_CTRL, Reg );

    hPb->PacingLoadNeeded = false;
}

#endif

bool BXPT_Playback_IsAvailable(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    unsigned int ChannelNo                      /* [in] Which channel to check. */
    )
{
    if( ChannelNo >= BXPT_NUM_PLAYBACKS )
    {
        return false;
    }
    else if( hXpt->PlaybackHandles[ ChannelNo ].Opened )
    {
        return false;
    }
#if (BCHP_CHIP == 7425) || (BCHP_CHIP == 7346) || (BCHP_CHIP == 73465)
    else if( 13 == ChannelNo || 14 == ChannelNo )
    {
        /* SW7425-3701. */
        return false;
    }
#endif

    return true;
}


void BXPT_Playback_InitDescriptorFlags(
    BXPT_PvrDescriptorFlags *flags
    )
{
    BDBG_ASSERT( flags );
    BKNI_Memset((void *)flags, 0, sizeof( BXPT_PvrDescriptorFlags ));
}

#if BXPT_HAS_TSMUX
void BXPT_Playback_SetDescriptorFlags(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,
    const BXPT_PvrDescriptorFlags *flags
    )
{
    BXPT_PvrDescriptor *CachedDesc = NULL;

    BDBG_ASSERT( Desc );
    BDBG_ASSERT( flags );

    CachedDesc = Desc;
    BMEM_FlushCache( hPb->hMemory, CachedDesc, sizeof (*CachedDesc) );

    CachedDesc->Reserved0 = 0;
    CachedDesc->MuxingFlags = flags->muxFlags.bRandomAccessIndication ? 1 << 2 : 0;
    if( flags->muxFlags.bNextPacketPacingTimestampValid )
    {
        CachedDesc->MuxingFlags |= 1 << 1;
        CachedDesc->NextPacketPacingTimestamp = flags->muxFlags.uiNextPacketPacingTimestamp;
    }
    if( flags->muxFlags.bPacket2PacketTimestampDeltaValid )
    {
        CachedDesc->MuxingFlags |= 1;
        CachedDesc->Pkt2PktPacingTimestampDelta = flags->muxFlags.uiPacket2PacketTimestampDelta;
    }
    BMEM_FlushCache( hPb->hMemory, CachedDesc, sizeof (*CachedDesc) );

    return;
}
#endif

unsigned BXPT_Playback_P_GetBandId(
    BXPT_Playback_Handle hPb
    )
{
    return (unsigned) BCHP_GET_FIELD_DATA( BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID ),
        XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID );
}

unsigned int BXPT_PB_P_GetPbBandId(
    BXPT_Handle hXpt,
    unsigned int Band
    )
{
    uint32_t RegAddr = BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID + Band * (BCHP_XPT_PB1_CTRL1 - BCHP_XPT_PB0_CTRL1);
    return (unsigned) BCHP_GET_FIELD_DATA( BREG_Read32( hXpt->hRegister, RegAddr ),
        XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID );
}

void BXPT_Playback_P_SetBandId(
    BXPT_Playback_Handle hPb,
    unsigned NewBandId
    )
{
    uint32_t Reg;
    BXPT_BandMap map;
    BXPT_Handle hXpt = (BXPT_Handle) hPb->vhXpt;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_SEL ) |
        BCHP_MASK( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_SEL, 1 ) |
        BCHP_FIELD_DATA( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID, NewBandId )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID, Reg );

    map = hXpt->BandMap.Playback[ hPb->ChannelNo ];
    map.VirtualParserBandNum = NewBandId;
}

#if 0
void BXPT_Playback_PvrDescriptorAdvDefault(
    BXPT_PvrDescriptorAdv *desc
    )
{
    BDBG_ASSERT( desc );
    BKNI_Memset((void *)desc, 0, sizeof( BXPT_PvrDescriptorAdv ));
}
#endif

void BXPT_Playback_GetLTSID(
    BXPT_Playback_Handle hPb,
    unsigned *ltsid
    )
{
    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ltsid );

    /* See SW7425-5193 for info about mapping parser number to LTSID */
#if BXPT_HAS_PARSER_REMAPPING
    {
        BXPT_BandMap map;
        BXPT_Handle hXpt = (BXPT_Handle) hPb->vhXpt;

        /* See SW7425-5193 for the band-to-LTSID mapping. */
        map = hXpt->BandMap.Playback[ hPb->ChannelNo ];
        *ltsid = map.VirtualParserIsPlayback ? map.VirtualParserBandNum + 0x40 : map.VirtualParserBandNum;
    }
#else
    *ltsid = hPb->ChannelNo + 0x40;
#endif
}
