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
#include "bkni.h"
#include "bxpt_priv.h"
#include "bxpt_playback.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if BXPT_HAS_XCBUF
#include "bxpt_xcbuf.h"
#endif
#include "bxpt_spid.h"

#if BXPT_HAS_RSBUF
#include "bxpt_rsbuf.h"
#endif

#include "bchp_xpt_pb0.h"
#include "bchp_xpt_fe.h"

#include "bxpt.h"
#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#include "bxpt_packetsub.h"
#endif

#if ( BCHP_CHIP == 3563 ) || (BCHP_CHIP == 7002)
    /* 3563 has only 1 playback. */
    #define PB_PARSER_REG_STEPSIZE  ( 0x44 )
#else
    /* Everybody else has at least 2 */
    #include "bchp_xpt_pb1.h"
    #define PB_PARSER_REG_STEPSIZE  ( BCHP_XPT_PB1_CTRL1 - BCHP_XPT_PB0_CTRL1 )
#endif

#ifndef BXPT_P_MAX_PACKETIZERS_PER_PB
#define BXPT_P_MAX_PACKETIZERS_PER_PB 1
#endif

BDBG_MODULE( xpt_playback );


#define BXPT_P_PLAYBACK_DEFAULT_USE_PCR_TIMEBASE        false
#define BXPT_P_PLAYBACK_DEFAULT_TIMESTAMP_MODE          BXPT_TimestampMode_e28_4P_Mod300
#define BXPT_PB_MAX_SYNC_LENGTH                         ( 256 )
#define FLUSH_COUNTDOWN                                 ( 10 )


static BERR_Code BXPT_Playback_P_CreateDesc(
    BXPT_Handle hXpt, BXPT_PvrDescriptor * const Desc, uint8_t *Buffer,
    uint32_t BufferLength, bool IntEnable, BXPT_PvrDescriptor * const NextDesc );

static void TsRangeErrorIsr ( void *hPb , int Param2);

BERR_Code BXPT_Playback_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int *TotalChannels     /* [out] The number of playback channels. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
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
        ChannelSettings->SyncMode = BXPT_PB_SYNC_MPEG;

#if BCHP_CHIP == 7630 && BCHP_VER == BCHP_VER_B2
        ChannelSettings->RaveOutputOnly = true; /* 7630B2/7631B0 does not have XC buffers. */
#else
        ChannelSettings->RaveOutputOnly = false;
#endif

        ChannelSettings->DisableTimestampParityCheck = false;

#if BCHP_CHIP == 7400 && BCHP_VER == BCHP_VER_A0
    /* 7400A0 is the oddball. It does NOT follow the chip-level endianess signal. */
    #if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
        ChannelSettings->SwapBytes = true;      /* The input file is big endian. Swap */
    #else
        ChannelSettings->SwapBytes = false;     /* Input file is also big-endian. Don't swap */
    #endif
#else
    /* Everyone else follows the chip endianess. */
    ChannelSettings->SwapBytes = false;     /* Input file is also big-endian. Don't swap */
#endif

#if !((BCHP_CHIP == 7400) && ( BCHP_VER == BCHP_VER_A0 ) )
    ChannelSettings->PsMode = BXPT_Playback_PS_Mode_MPEG2;
    ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Drop;
#endif
    }

#if BXPT_HAS_PCR_PACING
    ChannelSettings->PcrBasedPacing = false;   /* Use legacy 4-byte timestamp pacing */
    ChannelSettings->PcrPacingPid = 0x1FFF;
#endif

    ChannelSettings->AlwaysResumeFromLastDescriptor = false;

#if BXPT_HAS_32BIT_PB_TIMESTAMPS
    ChannelSettings->Use32BitTimestamps = false;
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
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
    hPb->LastDescriptor = 0;
    hPb->Running = false;
    hPb->vhXpt = hXpt;
    hPb->SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
    hPb->ForceResync = 0;

    BDBG_OBJECT_SET(hPb, bxpt_t);

    /* If they've got a separate heap for playback, use it */
    if( hXpt->hPbHeap )
        hPb->hMemory = hXpt->hPbHeap;
    else
        hPb->hMemory = hXpt->hMemory;

    TsRangeErrorIntId = BXPT_Playback_GetIntId( hPb, BXPT_PbInt_eTsRangeErr );

    BINT_CreateCallback(
        &hPb->hPbIntCallback,
        hXpt->hInt,
        TsRangeErrorIntId,
        TsRangeErrorIsr,
        ( void * ) hPb,
        0 );

    if ( ChannelSettings->ResetPacing)
    {
        hPb->ResetPacingOnTsRangeError = true;
        BINT_EnableCallback( hPb->hPbIntCallback );
    }
    else
    {
        hPb->ResetPacingOnTsRangeError = false;
        BINT_DisableCallback( hPb->hPbIntCallback );
    }

    /* Restore all the stuff they could have changed through the API. */
    Reg = (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 0 )
    );

/* Anand's playback hw fix didn't catch one corner case. Turn on the chicken bit. */
#if (BCHP_CHIP == 7550) || (BCHP_CHIP == 7408) || (BCHP_CHIP == 7468)
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL1, AGRESSIVE_DESC_READ, 1 ) );
#endif

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );

    /* Now load the defaults they passed in. */
    BXPT_Playback_SetChannelSettings( hPb, ChannelSettings );

    /* All data destined for the RAVE will be mapped by PID channel */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE, 1 )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2, Reg );

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
    hPb->DummyBuffer[ 0 ] = hPb->DummyBuffer[ 1 ] = 0x55;
    BXPT_Playback_CreateDesc( hXpt, hPb->DummyDescriptor, hPb->DummyBuffer, 2, false, false, NULL );

    hPb->Opened = true;
    *PlaybackHandle = hPb;

    return( ExitCode );

    ErrorCleanUp:
    BINT_DisableCallback( hPb->hPbIntCallback );
    BINT_DestroyCallback( hPb->hPbIntCallback );
    return( ExitCode );
}

void BXPT_Playback_CloseChannel(
    BXPT_Playback_Handle PlaybackHandle /* [in] Handle for the channel to close*/
    )
{
    uint32_t Reg;

    /* Stop the channel. */
    if (PlaybackHandle->Running) {
        BDBG_WRN(("Stopping playback channel %d before closing", PlaybackHandle->ChannelNo));
        BXPT_Playback_StopChannel(PlaybackHandle);
    }
    Reg = (BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 0 )
    );

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

    BINT_DestroyCallback( PlaybackHandle->hPbIntCallback );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );

    /* Clean up all previous Packetization state, if any. */
    BXPT_Playback_DisablePacketizers( PlaybackHandle );

    BMEM_Free( PlaybackHandle->hMemory, PlaybackHandle->DummyDescriptor );
    BMEM_Free( PlaybackHandle->hMemory, PlaybackHandle->DummyBuffer );

    PlaybackHandle->Opened = false;

    BDBG_OBJECT_UNSET(PlaybackHandle, bxpt_t);
}

static void TsRangeErrorIsr( void *hPb, int Param2 )
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
    uint32_t Reg, TimeBaseSel, DummyVar;

    unsigned int PacketLength = 188;    /* Use MPEG length as the default. */
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( ChannelSettings );

    hPb->SyncMode = ChannelSettings->SyncMode;
    hPb->AlwaysResumeFromLastDescriptor = ChannelSettings->AlwaysResumeFromLastDescriptor;

    if( !ChannelSettings->AlwaysResumeFromLastDescriptor )
    {
        BDBG_ERR(( "WARNING: Playback descriptor code needs updating!!!" ));
        BSTD_UNUSED( DummyVar );

        /*
        ** This is a workaround for PR 51821. When using this workaround, applications must either use bpvrlib_feed.c
        ** version 23 (or later), or change their descriptor management code to match. It is NOT backward compatible.
        **
        ** This workaround will default to disabled for now. At some point in the future, it will be enabled by
        ** default. The bug uncovered by PR 51821 is bad enough, and hard enough to track down, that adoption of
        ** this workaround is considered mandatory.
        **
        ** This message is printed when the app elects not to enable the workaround. It is done to warn of
        ** the bug and prompt the developers to change their descriptor handling to conform to bpvrlib_feed.c
        ** version 23 (or later).
        */
    }

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

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE ) |
        BCHP_MASK( XPT_PB0_CTRL2, ENDIAN_CTRL ) |
        BCHP_MASK( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS ) |
        BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_EN )
#if (BCHP_CHIP == 7440) || ( BCHP_CHIP == 7403 ) || ( BCHP_CHIP == 7405 )|| ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) \
        || (( BCHP_CHIP == 7401 ) && ( BCHP_VER >= BCHP_VER_C0 )) || ( BCHP_CHIP == 7118 ) || (BCHP_CHIP == 7002) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
        |
        BCHP_MASK( XPT_PB0_CTRL2, PROGRAM_STREAM_MODE ) |
        BCHP_MASK( XPT_PB0_CTRL2, PROGRAM_STREAM_EN )
#endif
        |
        BCHP_MASK( XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS )
        );

    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, ChannelSettings->SyncMode ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS, ChannelSettings->DisableTimestampParityCheck == true ? 1 : 0 ) |
#if !((BCHP_CHIP==7401 && BCHP_VER >= BCHP_VER_B0)||(BCHP_CHIP==7440)||( BCHP_CHIP == 7405 )||( BCHP_CHIP == 7325 )||( BCHP_CHIP == 7335 ) \
     || ( BCHP_CHIP == 7336  )) || (BCHP_CHIP == 7002) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
        /* no need to change this bit for 7401B0 plus chips */
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, ENDIAN_CTRL, ChannelSettings->SwapBytes == true ? 1 : 0 ) |
#endif
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_EN, ChannelSettings->TimestampEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS, ChannelSettings->PacingOffsetAdjustDisable == true ? 1 : 0 )
        );
#if !((BCHP_CHIP == 7400) && ( BCHP_VER == BCHP_VER_A0 ) )
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, PROGRAM_STREAM_MODE, ChannelSettings->PsMode );
        if( ChannelSettings->PackHdrConfig != BXPT_Playback_PackHdr_Drop)
        {
            Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, PROGRAM_STREAM_EN, 1 );
        }
#endif
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL4, OUTPUT_PIPE_SEL ) |
        BCHP_MASK( XPT_PB0_CTRL4, TIMEBASE_SEL ) |
        BCHP_MASK( XPT_PB0_CTRL4, TIMESTAMP_MODE )
#if (BCHP_CHIP == 7440) || ( BCHP_CHIP == 7403 ) || ( BCHP_CHIP == 7405 )|| ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) \
     || ( BCHP_CHIP == 7336  ) || (BCHP_CHIP == 7002) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
        | BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN )
#endif
        );

#if BXPT_HAS_DPCRS
    if( ChannelSettings->UsePcrTimeBase == false )
    {
        /* Use the free running 27 MHz clock. */
        TimeBaseSel = 0;
    }
    else
    {
        if( ChannelSettings->WhichPcrToUse >= BXPT_P_MAX_PCRS )
        {
            /* Bad PCR module number. Complain, and default to free running 27 MHz clock. */
            BDBG_ERR(( "WhichPcrToUse %lu is out of range!", ( unsigned long ) ChannelSettings->WhichPcrToUse ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            TimeBaseSel =  0;
        }

        TimeBaseSel = ChannelSettings->WhichPcrToUse + 1;
    }
#else
        TimeBaseSel =  0; /* This part only supports free-running clock */
#endif

    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, OUTPUT_PIPE_SEL, ChannelSettings->RaveOutputOnly == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, TIMEBASE_SEL, TimeBaseSel ) |
        BCHP_FIELD_DATA( XPT_PB0_CTRL4, TIMESTAMP_MODE, ChannelSettings->TimestampMode )
        );
#if (BCHP_CHIP == 7440) || ( BCHP_CHIP == 7403 ) || ( BCHP_CHIP == 7400 && BCHP_VER > BCHP_VER_A0 ) || ( BCHP_CHIP == 7405 ) \
    || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336 ) || (BCHP_CHIP == 7002) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
    if (ChannelSettings->PackHdrConfig == BXPT_Playback_PackHdr_Adaptation_Insert)
    {
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN, 1 );
    }
#endif

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

    /* PCR-based pacing */
#if BXPT_HAS_PCR_PACING
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
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PACING_PCR_PID, PACING_PCR_PID )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PACING_PCR_PID, PACING_PCR_PID, ChannelSettings->PcrPacingPid )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID, Reg );
#endif

#if BXPT_HAS_32BIT_PB_TIMESTAMPS
    /* Don't overwrite TIMESTAMP_MODE_32BIT if PCR pacing is in use */
    #if BXPT_HAS_PCR_PACING
    if( ChannelSettings->PcrBasedPacing == false )
    #endif
    {
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
        Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT, ChannelSettings->Use32BitTimestamps == true ? 1 : 0 ) );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );
    }
#endif

    if ( ChannelSettings->ResetPacing)
    {
        BERR_Code rc;

        hPb->ResetPacingOnTsRangeError = true;
        rc = BINT_EnableCallback( hPb->hPbIntCallback );
        if (rc)
            ExitCode = BERR_TRACE(rc);
    }
    else
    {
        hPb->ResetPacingOnTsRangeError = false;
        BINT_DisableCallback( hPb->hPbIntCallback );
    }

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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( ChannelSettings );

    ChannelSettings->AlwaysResumeFromLastDescriptor = hPb->AlwaysResumeFromLastDescriptor;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    ChannelSettings->PacketLength = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, SYNC_LENGTH );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    ChannelSettings->SyncMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, SYNC_EXT_MODE );
    ChannelSettings->SwapBytes = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, ENDIAN_CTRL ) ? true : false;
    ChannelSettings->TimestampEn = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TIMESTAMP_EN ) ? true : false;
    ChannelSettings->DisableTimestampParityCheck = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TS_PARITY_CHECK_DIS ) ? true : false;
    ChannelSettings->ResetPacing = hPb->ResetPacingOnTsRangeError;
    ChannelSettings->PacingOffsetAdjustDisable = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_OFFSET_ADJ_DIS ) ? true : false;
#if BXPT_HAS_32BIT_PB_TIMESTAMPS
    ChannelSettings->Use32BitTimestamps = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, TIMESTAMP_MODE_32BIT ) ? true : false;
#endif

#if !((BCHP_CHIP == 7400) && ( BCHP_VER == BCHP_VER_A0 ) )
    ChannelSettings->PsMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PROGRAM_STREAM_MODE );
    if( !BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PROGRAM_STREAM_EN ) )
        ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Drop;
    else
        ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Payload_Insert;
#endif

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    ChannelSettings->TimestampMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMESTAMP_MODE );
    ChannelSettings->RaveOutputOnly = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, OUTPUT_PIPE_SEL ) ? true : false;
    Timebase = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMEBASE_SEL );

#if BXPT_HAS_DPCRS
    /*
    ** In the Timebase check below, note that the hardware maps timebases with DPCR0 given a value of 1,
    ** DPCR1 is 2, etc. Thus, the check below is > as opposed to >=
    */
    if( Timebase == 0 )
        ChannelSettings->UsePcrTimeBase = false;
    else if( Timebase > BXPT_P_MAX_PCRS )
    {
        BDBG_ERR(( "Invalid timebase %u configured in hardware.", ( unsigned long ) Timebase ));
        ChannelSettings->UsePcrTimeBase = false;
    }
    else
    {
        ChannelSettings->UsePcrTimeBase = true;
        ChannelSettings->WhichPcrToUse = Timebase - 1;
    }
#else
        ChannelSettings->UsePcrTimeBase = false;
        ChannelSettings->WhichPcrToUse = 0;
#endif

#if (BCHP_CHIP == 7440) || ( BCHP_CHIP == 7403 )|| ( BCHP_CHIP == 7400 && BCHP_VER > BCHP_VER_A0 ) || ( BCHP_CHIP == 7405 ) \
    || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || (BCHP_CHIP == 7002) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || (BCHP_CHIP == 7340 )
    if( ChannelSettings->PackHdrConfig != BXPT_Playback_PackHdr_Drop )
    {
        if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, PKTZ_PACK_HDR_INSERT_EN ) )
            ChannelSettings->PackHdrConfig = BXPT_Playback_PackHdr_Adaptation_Insert;
    }
#endif

    /* PCR-based pacing */
#if BXPT_HAS_PCR_PACING
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );
    ChannelSettings->PcrBasedPacing = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL2, PACING_TYPE ) ? true : false;
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PACING_PCR_PID );
    ChannelSettings->PcrPacingPid = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PACING_PCR_PID, PACING_PCR_PID );
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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CURR_DESC_ADDR );
    CurrentDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CURR_DESC_ADDR, CURR_DESC_ADDR );
    CurrentDescAddr <<= 4;  /* Convert to byte-address. */
    BMEM_ConvertOffsetToAddress( hPb->hMemory, CurrentDescAddr, ( void ** ) &UserDescAddr );
    BMEM_ConvertAddressToCached(hPb->hMemory, UserDescAddr, &UserDescAddr);
    *LastDesc = ( BXPT_PvrDescriptor * ) UserDescAddr;

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
    BERR_Code RetCode;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( Desc );

#ifdef BXPT_2_BYTE_MINIMUM_PLAYBACK_BUFFER
    if( BufferLength == 1 || !BufferLength )
    {
        BDBG_ERR(( "Buffer lengths of %lu are not supported.", (unsigned long) BufferLength ));
        return BERR_NOT_SUPPORTED;
    }
#endif

    RetCode = BXPT_Playback_P_CreateDesc( hXpt, Desc, Buffer, BufferLength, IntEnable, NextDesc );

    if( RetCode == BERR_SUCCESS && ReSync == true )
        Desc->Flags |= TRANS_DESC_FORCE_RESYNC_FLAG;

    return RetCode;
}

void BXPT_SetLastDescriptorFlag(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc         /* [in] Descriptor to initialize */
    )
{
    BDBG_ASSERT( Desc );
    BSTD_UNUSED( hXpt );

    /*  Set the Last Descriptor bit. */
    Desc->NextDescAddr = TRANS_DESC_LAST_DESCR_IND;
}

static BERR_Code AddDescriptorByAppending(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *LastDesc,   /* [in] Last descriptor in new chain */
    BXPT_PvrDescriptor *FirstDesc   /* [in] First descriptor in new chain */
    )
{
    uint32_t Reg;
    uint32_t DescPhysAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, RUN ) )
    {
        /* Channel is already running. Append this descriptor to the end and use WAKE_MODE = 0 (resume from the last descriptor). */
        BXPT_PvrDescriptor *LastDescriptor = ( BXPT_PvrDescriptor * ) hPb->LastDescriptor;

        /* Set the last descriptor in the chain to point to the descriptor we're adding. */
        BMEM_ConvertAddressToOffset( hPb->hMemory, ( void * ) FirstDesc, &DescPhysAddr );
        LastDescriptor->NextDescAddr = ( uint32_t ) DescPhysAddr;
        BMEM_FlushCache(hPb->hMemory, LastDescriptor, sizeof (BXPT_PvrDescriptor) );

        /* SWDTV-8330: read register after uncache memory write operation to maintain consistency */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
        Reg &= ~ ( BCHP_MASK( XPT_PB0_CTRL1, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 1 );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
    }
    else
    {
        /* Channel isn't running. Point the FIRST_DESC_ADDR to this descriptor and set RUN */
        BMEM_ConvertAddressToOffset( hPb->hMemory, ( void * ) FirstDesc, &DescPhysAddr );

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
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 1 );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
    }

    if ( LastDesc )
    {
        hPb->LastDescriptor = ( uint32_t ) LastDesc;
    }
    else
    {
        hPb->LastDescriptor = 0;
    }
    return ExitCode;
}

BERR_Code BXPT_Playback_AddDescriptors(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *LastDesc,   /* [in] Last descriptor in new chain */
    BXPT_PvrDescriptor *FirstDesc   /* [in] First descriptor in new chain */
    )
{
    uint32_t ChanFinished, Reg;
    uint32_t RunBit;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    /* BDBG_ASSERT( LastDesc ); */
    BDBG_ASSERT( FirstDesc );

    BDBG_MSG(("Adding Desc Addr 0x%08lX to Playback Channel %d", ( unsigned long ) FirstDesc, hPb->ChannelNo ));

    BMEM_FlushCache(hPb->hMemory, FirstDesc, sizeof (BXPT_PvrDescriptor) );
    if (hPb->ForceResync) {
        /* cycle through all descriptors and set force_resync flag */
        BXPT_PvrDescriptor *Desc = FirstDesc;
        BXPT_PvrDescriptor *HeadOfChain = FirstDesc;
        void *ptr;

        do {
            Desc->Flags |= TRANS_DESC_FORCE_RESYNC_FLAG;
            BMEM_FlushCache(hPb->hMemory, Desc, sizeof (BXPT_PvrDescriptor) );
            if (Desc->NextDescAddr & TRANS_DESC_LAST_DESCR_IND) { break; }
            BMEM_ConvertOffsetToAddress(hPb->hMemory, Desc->NextDescAddr, &ptr);
            Desc = ptr;
            BMEM_FlushCache(hPb->hMemory, Desc, sizeof (BXPT_PvrDescriptor) );
        } while (Desc != HeadOfChain);
    }

    if( hPb->AlwaysResumeFromLastDescriptor )
        return AddDescriptorByAppending( hPb, LastDesc, FirstDesc );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
    ChanFinished = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, FINISHED );

    if( ChanFinished )
    {
        /* Channel has finished, so start over at the beginning of the chain. */
        Reg &= ~ ( BCHP_MASK( XPT_PB0_CTRL1, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 1 );
        hPb->LastDescriptor = 0;
    }
    else
    {
        /* Channel has finished, so start over at the beginning of the chain. */
        Reg &= ~ ( BCHP_MASK( XPT_PB0_CTRL1, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE_MODE, 0 );
    }
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );

    /* Do we already have a list going? */
    if( hPb->LastDescriptor )
    {
        uint32_t DescPhysAddr;

        /*
        ** Yes, there is list already. Append this descriptor to the last descriptor,
        ** then set the wake bit.
        */
        BXPT_PvrDescriptor *LastDescriptor = ( BXPT_PvrDescriptor * ) hPb->LastDescriptor;

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
        RunBit = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, RUN );

        /* Set the last descriptor in the chain to point to the descriptor we're adding. */
        BMEM_ConvertAddressToOffset( hPb->hMemory, ( void * ) FirstDesc, &DescPhysAddr );
        LastDescriptor->NextDescAddr = ( uint32_t ) DescPhysAddr;

        /* If the channel is running, we need to set the wake bit to let the hardware know we added a new buffer */
        if( RunBit )
        {
            Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 1 );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
        }
    }
    else
    {
        /*
        ** If this is the first descriptor (the channel has not been started)
        ** then load the address into the first descriptor register
        */
        uint32_t DescPhysAddr;

        /* This is our first descriptor, so we must load the first descriptor register */
        BMEM_ConvertAddressToOffset( hPb->hMemory, ( void * ) FirstDesc, &DescPhysAddr );

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

        /*
        ** If this channel has been started, we need to kick off the hardware
        ** by setting the RUN bit.
        */
        if( hPb->Running == true )
        {
            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL1 );
            RunBit = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, RUN );

            if( RunBit )
            {
                /*
                ** Since the channel was already running in hardware, this means that we
                ** are reloading the first descriptor address due to the channel
                ** finishing before a new descriptor was added.  Therefore
                ** we use the wake bit (as we previously set the WAKE_MODE above.
                */
                Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, WAKE ) );
                Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, WAKE, 1 );
            }
            else
            {
                Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, RUN ) );
                Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 1 );
            }

            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL1, Reg );
        }
    }

    /* This descriptor is always the new last descriptor */
    hPb->LastDescriptor = ( uint32_t ) LastDesc;

    return( ExitCode );
}

BERR_Code BXPT_Playback_StartChannel(
    BXPT_Playback_Handle PlaybackHandle     /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

    BDBG_MSG(( "Starting playback channel %d", ( unsigned long ) PlaybackHandle->ChannelNo ));

    if( PlaybackHandle->Running == true )
    {
        BDBG_ERR(( "Playback channel %d cannot be started because it's already running!",
            ( unsigned long ) PlaybackHandle->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_RUNNING );
    }

#ifdef BCHP_PWR_RESOURCE_XPT_PLAYBACK
    if( PlaybackHandle->Running == false )
    {
        BCHP_PWR_AcquireResource(PlaybackHandle->hChip, BCHP_PWR_RESOURCE_XPT_PLAYBACK);
    }
#endif

#if 0
    /* Wait for the BUSY bit to clear. */
    WaitCount = 100;          /* 100 mS max wait before we declare failure */
    do
    {
        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
        Busy = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY ) ? true : false;

        if( Busy )
        {
            WaitCount--;
            if( !WaitCount )
            {
                BDBG_ERR(( "Playback channel %d timed-out waiting for BUSY bit to clear",
                    ( unsigned long ) PlaybackHandle->ChannelNo ));
                ExitCode = BERR_TRACE( BERR_TIMEOUT );
                goto Done;
            }

            BKNI_Sleep( 1 );
        }
    }
    while( Busy );
#endif

    /* Check if we have buffers already loaded for this channel */
    if( PlaybackHandle->LastDescriptor )
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

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

    BDBG_MSG(( "Stopping playback channel %d", ( unsigned long ) PlaybackHandle->ChannelNo ));

    if( PlaybackHandle->Running == false )
    {
        BDBG_ERR(( "Playback channel %d cannot be stopped because it's not running!",
            ( unsigned long ) PlaybackHandle->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_STOPPED );
    }

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    /* Set the host pause.  */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, MICRO_PAUSE, 1 );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

    /* Wait for the data to drain. */
    BKNI_Sleep( 40 );

    /* Disable the parser band. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ENABLE ) );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

    /* Clear the host pause. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL2, MICRO_PAUSE ) );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );
#endif

#if BXPT_HAS_XCBUF
    /* Stop the channel hardware */

    /* Some parts don't have this fallback */
#ifdef BCHP_XPT_PB0_CTRL1_DISCARD_DMA_DATA_SHIFT
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_CTRL1, DISCARD_DMA_DATA, 1 )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );
#endif

    {
        int Wait;

        /* Disable pausing from the XC buffer. Allows data to drain */
        BXPT_P_XcBuf_Pause( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo + BXPT_XcBuf_Id_RAVE_PBP0, false );
        for( Wait = 0; Wait < 100; Wait++ )
        {
            BKNI_Delay( 1 );  /* Busy wait for 1 uS */
        }

        /* Re-enable pausing from the XC buffer. Prevents overflows in RAVE. */
        BXPT_P_XcBuf_Pause( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo + BXPT_XcBuf_Id_RAVE_PBP0, true );
    }
#endif

    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL1, RUN ) );
    Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL1, RUN, 0 );
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

    PlaybackHandle->LastDescriptor = 0;

    {
        uint32_t Busy;
        int WaitCount;
        uint32_t FirstDescAddr, SyncExtMode;
        uint32_t DescPhysAddr;
        uint32_t DescNotDone;
        uint32_t BandHoldStatus = 0;

        /* Store the BAND_HOLD_EN status for all RAVE contexts so that they can be restored back to original values.
         SW7335-664 */
        BXPT_Rave_P_GetBandHold( (BXPT_Handle) PlaybackHandle->vhXpt, &BandHoldStatus );

        /* Disable the band-hold at the RAVE end of the pipeline. This will let the PB channel finish */
        BXPT_Rave_P_SetBandHold( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo, BandHoldStatus, false );

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
                    BDBG_ERR(( "Playback channel %d timed-out waiting for BUSY bit to clear",
                        ( unsigned long ) PlaybackHandle->ChannelNo ));
                    ExitCode = BERR_TRACE( BERR_TIMEOUT );
                    goto Done;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }
        }
        while( Busy );

        Done:
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
                    BDBG_ERR(( "Playback channel %d timed-out waiting for CURR_DESC_NOT_DONE bit to clear",
                        ( unsigned long ) PlaybackHandle->ChannelNo ));
                    ExitCode = BERR_TRACE( BERR_TIMEOUT );
                    goto Done2;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }

        }
        while( DescNotDone );

        Done2:
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
                    BDBG_ERR(( "Playback channel %d timed-out waiting for BUSY bit to clear, 2nd loop",
                        ( unsigned long ) PlaybackHandle->ChannelNo ));
                    ExitCode = BERR_TRACE( BERR_TIMEOUT );
                    goto Done3;
                }

                BKNI_Delay( 1 );  /* Busy wait for 1 uS */
            }
        }
        while( Busy );

        Done3:
        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR );
        Reg &= ~( BCHP_MASK( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR ) );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_FIRST_DESC_ADDR, FIRST_DESC_ADDR, FirstDescAddr );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_FIRST_DESC_ADDR, Reg );

        Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2 );
        Reg &= ~BCHP_MASK( XPT_PB0_CTRL2, SYNC_EXT_MODE );
        Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL2, SYNC_EXT_MODE, SyncExtMode );
        BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL2, Reg );

        /* Put band hold back. Pass BandHoldStatus saved earlier to restore the BAND_HOLD_EN values
           SW7335-664*/
        BXPT_Rave_P_SetBandHold( (BXPT_Handle) PlaybackHandle->vhXpt, PlaybackHandle->ChannelNo, BandHoldStatus, true );
    }

    /* Need to disable the parser band. */
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ENABLE )
    );
    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_PARSER_CTRL1, Reg );


#if BXPT_HAS_XCBUF
    Reg = BXPT_Playback_P_ReadReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1 );

    /* Some parts don't have this fallback */
    #ifdef BCHP_XPT_PB0_CTRL1_DISCARD_DMA_DATA_SHIFT
        Reg &= ~(
            BCHP_MASK( XPT_PB0_CTRL1, DISCARD_DMA_DATA )
        );
    #endif

    BXPT_Playback_P_WriteReg( PlaybackHandle, BCHP_XPT_PB0_CTRL1, Reg );
#endif

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

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

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

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

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
    uint32_t Reg, SyncLen, BoCount;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

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

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL3 );
    SyncLen = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL3, SYNC_LENGTH );

    if(SyncLen == 0xB8)
        SyncLen = 0xBC;

    {
        unsigned SpareBwEn = 0;

#if BXPT_HAS_XCBUF
        /* Control rate with the XC buffer blockouts. The local playback blockout is set to the max */
        BXPT_XcBuf_SetBandDataRate((BXPT_Handle)hPb->vhXpt, BXPT_XcBuf_Id_RAVE_PBP0+hPb->ChannelNo, BitRate,SyncLen);
#endif

#if BXPT_HAS_RSBUF
        /* MPOD/MCARD usages require data to be sent through the RS buffers. So, update their blockouts as well. */
        BXPT_RsBuf_SetPlaybackDataRate( (BXPT_Handle)hPb->vhXpt, hPb->ChannelNo, BitRate );
#endif

#if ( BCHP_CHIP == 7440 )
        BoCount = 108000000 / BitRate;
        BoCount *= ( 8 * SyncLen );
#else
        BoCount = (10800 * SyncLen * 8) / ( BitRate / 10000 );
#endif

        /* PR 29510 workaround. Do not enable spare bandwidth usage if the packetizer is used */
        /* SW7425-153: Using the spare bandwidth is causing overflows. Also forumula is wrong. */
        SpareBwEn = 0;

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_BLOCKOUT );
        Reg &= ~(
                 BCHP_MASK( XPT_PB0_BLOCKOUT, BO_COUNT ) |
                 BCHP_MASK( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN )
                 );
        Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_BLOCKOUT, BO_COUNT, BoCount ) |
                BCHP_FIELD_DATA( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN, SpareBwEn )
                );

        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_BLOCKOUT, Reg );
    }
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

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

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

#if BXPT_HAS_DESCRIPTOR_WORKAROUND
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
        *BufferSize = Desc->BufferLength;
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

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    *Bits = ( int ) ( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, TIMESTAMP_USER_BITS ) & 0xFF );

    return( ExitCode );
}

BERR_Code BXPT_Playback_ConfigPacing(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PacingControl Mode                 /* [in] New mode for pacing. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL2 );

    /*
    ** Init the Pacing bitfields to 0. The PB_PACING_START bit must transistion from 0 to 1
    ** later on in order to arm the pacing logic.
    */
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL2, PACING_START ) |
        BCHP_MASK( XPT_PB0_CTRL2, PACING_EN ) |
        BCHP_MASK( XPT_PB0_CTRL2, PACING_AUTOSTART_EN )
        );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    if( Mode == BXPT_PacingControl_eStart )
    {
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_START, 1 ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_EN, 1 ) |

            /*
            ** Autostart will re-arm the pacing logic automatically if a
            ** force-resync condition occurrs.
            */
            BCHP_FIELD_DATA( XPT_PB0_CTRL2, PACING_AUTOSTART_EN, 1 )
        );
    }

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL2, Reg );

    return( ExitCode );
}

BERR_Code BXPT_Playback_SetPacingErrorBound(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel */
    unsigned long MaxTsError        /* [in] Maximum timestamp error. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

#if BXPT_PLAYBACK_TS_ERROR_BOUNDS
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
#else
    /* Legacy chips */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_TS_ERR_BOUND, TS_ERR_BOUND )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_TS_ERR_BOUND, TS_ERR_BOUND, MaxTsError )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_TS_ERR_BOUND, Reg );
#endif

    return( ExitCode );
}

void BXPT_Playback_GetPacketizerDefaults(
    const BXPT_Playback_PacketizeConfig *Cfg   /* [in] Config to initialize */
    )
{
    BDBG_ASSERT( Cfg );
    BKNI_Memset( (void *) Cfg, 0, sizeof( BXPT_Playback_PacketizeConfig ) );
}

#if BXPT_HAS_PID_CHANNEL_PES_FILTERING

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_P_PacketSubCfg PsubCfg;
#endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( Cfg );

#if 0
    /* This check isn't necessary, since we're not using the packetizer contexts in playback. */
    if( Context >= BXPT_P_MAX_PACKETIZERS_PER_PB )
    {
        BDBG_ERR(( "Context %lu is out of range!", ( unsigned long ) Context ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }
#endif

    hXpt = (BXPT_Handle)hPb->vhXpt;
    hPb->ForceResync = (Enable==true) && (Cfg->PacketizerMode == BXPT_Playback_PacetizerMode_Es); /* true only if doing ES packetization */

    if( Enable == true )
    {
        /* PR 29510 workaround */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_BLOCKOUT );
        Reg &= ~( BCHP_MASK( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN ) );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_BLOCKOUT, Reg );

        RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Context );

        /* support for context will be dropped in next chip revisions
           keeping the code just in case of ES mode. These settings are
           redundant if context enable bit is not turned on*/
        if (Cfg->PacketizerMode == BXPT_Playback_PacetizerMode_Es)
        {
            Reg = BXPT_Playback_P_ReadReg( hPb, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PID ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, CH_NUM ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, PID, Cfg->Pid ) |
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, CH_NUM, Cfg->ChannelNum ) |
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID, 1 )
            );
            BXPT_Playback_P_WriteReg( hPb, RegAddr, Reg );
        }

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 0x02 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT, 1 )
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
        case BXPT_Playback_PacetizerMode_Es:
            Reg &= ~( BCHP_MASK( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN )|
                      BCHP_MASK( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN )
                      );
            Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS, 1 );
            Reg |= BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN, 1 );
            break;

        case BXPT_Playback_PacetizerMode_Pes_MapAll:
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
            break;

        case BXPT_Playback_PacetizerMode_Pes_Sid:
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamId;
            SidFilter.FilterConfig.StreamId = Cfg->FilterConfig.StreamId;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacetizerMode_Pes_SidRange:
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

        case BXPT_Playback_PacetizerMode_Pes_SidExtension:
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

        case BXPT_Playback_PacetizerMode_Pes_SidSubSid:
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

    }
    else
    {
        RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Context );
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PID ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, CH_NUM ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0, Reg );

        SidFilter.Mode = BXPT_Spid_eChannelFilterMode_Disable;
        BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

        /*
        ** SCheck the packetizer in each SPID/PID channel. If all the channels that are mapped to
        ** this playback are Invalid, disable packetizing at the PB control reg.
        */
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, 0, &PsubCfg );
#endif

       for( Index = 0; Index < BXPT_P_MAX_PID_CHANNELS; Index++ )
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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, 0, &PsubCfg );
#endif

        if( Index == BXPT_P_MAX_PID_CHANNELS )
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
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
                );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );
        }
    }

    return( ExitCode );
}

#else /* ( BCHP_CHIP == 7440 ) || ( BCHP_CHIP == 7403 ) */

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )

static BERR_Code SetDvdMode(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    unsigned Context,                           /* [in] Which context to map the packets to. */
    const BXPT_Playback_PacketizeConfig *Cfg,   /* [in] Configuration for this context */
    bool Enable                                 /* [in] Start or stop packetization. */
    );

static BERR_Code SetDvdMode(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    unsigned Context,                           /* [in] Which context to map the packets to. */
    const BXPT_Playback_PacketizeConfig *Cfg,   /* [in] Configuration for this context */
    bool Enable                                 /* [in] Start or stop packetization. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t Reg;

    BSTD_UNUSED( Context );
    BSTD_UNUSED( Cfg );

    if( Enable == true )
    {
        /* PR 29510 workaround */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_BLOCKOUT );
        Reg &= ~( BCHP_MASK( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN ) );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_BLOCKOUT, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS ) |
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
            BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS, 0 ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN, 0 ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PACKETIZE_EN, 0 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 0x04 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0x80 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE, 1 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL, 1 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT, 1 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE, 0 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2, Reg );
    }
    else
    {
        /*
        ** Some of the LPCM-specific config above are setting bits to 0, which is their default.
        */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE, 1 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL2, Reg );
    }

    return( ExitCode );
}
#endif

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

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( Cfg );

    if( Context >= BXPT_P_MAX_PACKETIZERS_PER_PB )
    {
        BDBG_ERR(( "Context %lu is out of range!", ( unsigned long ) Context ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    if( Cfg->IsDvd == true )
        return SetDvdMode( hPb, Context, Cfg, Enable );
#endif

    if( Enable == true )
    {
        unsigned StreamId = Cfg->MapAll == true ? 1 : Cfg->PesStreamId;
        unsigned DisablePusi = Cfg->IsEs == true ? 1 : 0;

        /* PR 29510 workaround */
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_BLOCKOUT );
        Reg &= ~( BCHP_MASK( XPT_PB0_BLOCKOUT, BO_SPARE_BW_EN ) );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_BLOCKOUT, Reg );

        RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Context );

        Reg = BXPT_Playback_P_ReadReg( hPb, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PID ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, CH_NUM ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, PID, Cfg->Pid ) |
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, CH_NUM, Cfg->ChannelNum ) |
            BCHP_FIELD_DATA( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID, StreamId )
            );
        BXPT_Playback_P_WriteReg( hPb, RegAddr, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
            BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE, 0x02 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xB8 ) |
            BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT, 1 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS ) |
            BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
            BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_PUSI_SET_DIS, DisablePusi ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN, 1 ) |
            BCHP_FIELD_DATA( XPT_PB0_CTRL4, PACKETIZE_EN, 1 )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );
    }
    else
    {
        RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Context );
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PID ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, CH_NUM ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0, Reg );

        /* Check each packetizer context. If all are Invalid, disable packetizing at the PB control reg. */
        for( Index = 0; Index < BXPT_P_MAX_PACKETIZERS_PER_PB; Index++ )
        {
            RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Index );
            Reg = BXPT_Playback_P_ReadReg( hPb, RegAddr );
            if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID ) != 0 )
                break;
        }
        if( Index == BXPT_P_MAX_PACKETIZERS_PER_PB )
        {
            /* All contexts where invalid, so disable packetizer in the PB control reg. */
            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
                BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
                BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
                );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH, 0xBC )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );
        }
    }

    Done:
    return( ExitCode );
}


#endif /* ( BCHP_CHIP == 7440 ) */

BERR_Code BXPT_Playback_DisablePacketizers(
    BXPT_Playback_Handle hPb                    /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg, RegAddr;
    unsigned Index;

    BERR_Code ExitCode = BERR_SUCCESS;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_P_PacketSubCfg PsubCfg;
#endif

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7118 )
    /* If the 7401 is in the legacy DVD mode, disable is done differently */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) == 0x80 )
        SetDvdMode( hPb, 0, NULL, false );
#endif

#if BXPT_HAS_PID_CHANNEL_PES_FILTERING
    {
        int32_t i;
        BXPT_Handle hXpt = (BXPT_Handle)hPb->vhXpt;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, 0, &PsubCfg );
#endif

        for( Index = 0; Index < BXPT_P_MAX_PID_CHANNELS; Index++ )
        {
            /* if the pid channel input is this parser band cleanup the SPID table */
            RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( 4 * Index );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );

            if(!BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i,PLAYBACK_FE_SEL))
                continue;
            /* pb parser is the input */
#if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2)
            if(BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i,PID_CHANNEL_INPUT_SELECT)!= hPb->ChannelNo)
                continue;
#else
            if(BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i,PLAYBACK_BAND_PARSER_PID_CHANNEL_INPUT_SELECT)!= hPb->ChannelNo)
                continue;
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_PacketSub_P_RestoreCfg( hXpt, 0, &PsubCfg );
#endif

    }

#endif

    for( Index = 0; Index < BXPT_P_MAX_PACKETIZERS_PER_PB; Index++ )
    {
        RegAddr = BCHP_XPT_PB0_PKTZ_CONTEXT0 + ( 4 * Index );
        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0 );
        Reg &= ~(
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PID ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, CH_NUM ) |
            BCHP_MASK( XPT_PB0_PKTZ_CONTEXT0, PES_STREAM_ID )
            );
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PKTZ_CONTEXT0, Reg );
    }

    /* Disable packetizer in the PB control reg. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_CONTEXT_EN ) |
        BCHP_MASK( XPT_PB0_CTRL4, PACKETIZE_EN )
#if BXPT_HAS_PID_CHANNEL_PES_FILTERING
        |
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_PT_EN )|
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_SUB_ID_EN )|
        BCHP_MASK( XPT_PB0_CTRL4, PKTZ_STREAM_ID_EXT_EN )
#endif
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

    /* These parser settings are always loaded when BXPT_Playback_PacketizeStream() is called. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PACKET_TYPE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_PKT_LENGTH ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT )
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

    uint32_t Timebase = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( ParserConfig );

    hXpt = ( BXPT_Handle ) hPb->vhXpt;

    /* The parser config registers are at consecutive addresses. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );

    ParserConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE );
    ParserConfig->ContCountIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE );
    ParserConfig->TsMode = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE );
    ParserConfig->AllPass = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL );
    ParserConfig->AcceptNulls = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT );
    ParserConfig->AcceptAdapt00 = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_ADAPT_00 );

#if (BCHP_CHIP != 7002)
    /* The location of these bits moved from the 7038. So we store flags in the handle. */
    ParserConfig->SaveShortPsiMsg = hXpt->PbParserTable[ hPb->ChannelNo ].SaveShortPsiMsg;
    ParserConfig->SaveLongPsiMsg = hXpt->PbParserTable[ hPb->ChannelNo ].SaveLongPsiMsg;
    ParserConfig->PsfCrcDis = hXpt->PbParserTable[ hPb->ChannelNo ].PsfCrcDis;
    ParserConfig->PsiMod = hXpt->PbParserTable[ hPb->ChannelNo ].PsiMsgMod;
#endif
    Timebase = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_PARSER_CTRL1, PARSER_TIMEBASE_SEL );

#if BXPT_HAS_DPCRS
    /*
    ** In the Timebase check below, note that the hardware maps timebases with DPCR0 given a value of 1,
    ** DPCR1 is 2, etc. Thus, the check below is > as opposed to >=
    */
    if( Timebase == 0 )
        ParserConfig->UsePcrTimeBase = false;
    else if( Timebase > BXPT_P_MAX_PCRS )
    {
        BDBG_ERR(( "Invalid timebase %u configured in hardware.", ( unsigned long ) Timebase ));
        ParserConfig->UsePcrTimeBase = false;
    }
    else
    {
        ParserConfig->UsePcrTimeBase = true;
        ParserConfig->WhichPcrToUse = Timebase - 1;
    }
#else
        ParserConfig->UsePcrTimeBase = false;
        ParserConfig->WhichPcrToUse = 0;
#endif

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    ParserConfig->ForceRestamping = BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP );

    return( ExitCode );
}

BERR_Code BXPT_Playback_SetParserConfig(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    const BXPT_Playback_ParserConfig *ParserConfig  /* [in] The new settings */
    )
{
    uint32_t Reg, TimeBaseSel;
    BXPT_Handle hXpt;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);
    BDBG_ASSERT( ParserConfig );

    hXpt = ( BXPT_Handle ) hPb->vhXpt;

    /* The parser config registers are at consecutive addresses. */
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1 );

    /* Clear all the bits we are about to change. */
    Reg &= ~(
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_TIMEBASE_SEL ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT ) |
        BCHP_MASK( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_ADAPT_00 )
    );

    /* Now set the new values. */
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE, ParserConfig->ErrorInputIgnore ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_CONT_COUNT_IGNORE, ParserConfig->ContCountIgnore ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ALL_PASS_CTRL, ParserConfig->AllPass == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_TIMESTAMP_MODE, ParserConfig->TsMode ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_NULL_PKT, ParserConfig->AcceptNulls ) |
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_ACCEPT_ADAPT_00, ParserConfig->AcceptAdapt00 )
    );

    if( ParserConfig->UsePcrTimeBase == false )
    {
        /* Use the free running 27 MHz clock. */
        TimeBaseSel = 0;
    }
    else
    {
        TimeBaseSel = ParserConfig->WhichPcrToUse + 1;
    }
    Reg |= (
        BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL1, PARSER_TIMEBASE_SEL, TimeBaseSel )
    );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_PARSER_CTRL1, Reg );

#if BXPT_HAS_MESG_BUFFERS
    hXpt->PbParserTable[ hPb->ChannelNo ].SaveShortPsiMsg = ParserConfig->SaveShortPsiMsg;
    hXpt->PbParserTable[ hPb->ChannelNo ].SaveLongPsiMsg = ParserConfig->SaveLongPsiMsg;
    hXpt->PbParserTable[ hPb->ChannelNo ].PsfCrcDis = ParserConfig->PsfCrcDis;
    hXpt->PbParserTable[ hPb->ChannelNo ].PsiMsgMod = ParserConfig->PsiMod;

    /*
    ** The PSI settings above are now done in the message filter block. Make sure all the
    ** filters get the new settings.
    */
    BXPT_P_ApplyParserPsiSettings( hXpt, hPb->ChannelNo, true );
#endif

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_PB0_CTRL4 );
    Reg &= ~( BCHP_MASK( XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_PB0_CTRL4, PARSER_FORCE_RESTAMP, ParserConfig->ForceRestamping ) );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_PB0_CTRL4, Reg );

    return( ExitCode );
}

void BXPT_Playback_P_WriteReg(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr,
    uint32_t RegVal
    )
{
    uint32_t RegAddr;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    RegAddr = Pb0RegAddr - BCHP_XPT_PB0_CTRL1 + PlaybackHandle->BaseAddr;
    BREG_Write32( PlaybackHandle->hRegister, RegAddr, RegVal );
}


uint32_t BXPT_Playback_P_ReadReg(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr
    )
{
    uint32_t RegAddr;

    BDBG_OBJECT_ASSERT(PlaybackHandle, bxpt_t);

    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    RegAddr = Pb0RegAddr - BCHP_XPT_PB0_CTRL1 + PlaybackHandle->BaseAddr;
    return( BREG_Read32( PlaybackHandle->hRegister, RegAddr ));
}


static BERR_Code BXPT_Playback_P_CreateDesc(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc,        /* [in] Descriptor to initialize */
    uint8_t *Buffer,                        /* [in] Data buffer. */
    uint32_t BufferLength,                  /* [in] Size of buffer (in bytes). */
    bool IntEnable,                         /* [in] Interrupt when done? */
    BXPT_PvrDescriptor * const NextDesc     /* [in] Next descriptor, or NULL */
    )
{
    uint32_t BufferPhysicalAddr;
    uint32_t ThisDescPhysicalAddr;
    BMEM_Handle hHeap;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( Desc );
    BDBG_ASSERT( Buffer );

    if( hXpt->hPbHeap )
        hHeap = hXpt->hPbHeap;
    else
        hHeap = hXpt->hMemory;

    BMEM_ConvertAddressToOffset( hHeap, ( void * ) Buffer, &BufferPhysicalAddr );

    /* Verify that the descriptor we're creating sits on a 16-byte boundary. */
    BMEM_ConvertAddressToOffset( hHeap, ( void * ) Desc, &ThisDescPhysicalAddr );
    if( ThisDescPhysicalAddr % 16 )
    {
        BDBG_ERR(( "Desc is not 16-byte aligned!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    /* Load the descriptor's buffer address, length, and flags. */
    Desc->BufferStartAddr = BufferPhysicalAddr;
    Desc->BufferLength = BufferLength;

    /* Clear everything, then set the ones we want below. */
    Desc->Flags = 0;

    if( IntEnable )
        Desc->Flags |= TRANS_DESC_INT_FLAG;

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
        Desc->NextDescAddr = NextDescPhysAddr;
    }
    else
    {
        /* There is NOT another descriptor. Set the Last Descriptor bit. */
        BXPT_SetLastDescriptorFlag( hXpt, Desc );
    }

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
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BMEM_ConvertAddressToOffset(hXpt->hMemory, ( void * ) Buffer, &(Desc->BufferStartAddr));
    Desc->BufferLength = BufferLength;
}
#endif /*ENABLE_PLAYBACK_MUX*/

BINT_Id BXPT_Playback_GetIntId(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_Int PbInt
    )
{
    uint32_t RegAddr;

    BDBG_OBJECT_ASSERT(hPb, bxpt_t);

    RegAddr = BCHP_XPT_PB0_INTR - BCHP_XPT_PB0_CTRL1 + hPb->BaseAddr;

    /* SW7405-3308: TEI and Parser Length error bits are swapped. */
    switch( PbInt )
    {
        case BXPT_PbInt_eDone:
        PbInt = BCHP_XPT_PB0_INTR_DONE_INT_SHIFT;   /* Stop the compiler's complaints */
        break;

        case BXPT_PbInt_eTsRangeErr:
        PbInt = BCHP_XPT_PB0_INTR_TS_RANGE_ERROR_SHIFT;   /* Stop the compiler's complaints */
        break;

        case BXPT_PbInt_eOutOfSync:
        PbInt = BCHP_XPT_PB0_INTR_SE_OUT_OF_SYNC_INT_SHIFT;   /* Stop the compiler's complaints */
        break;

        case BXPT_PbInt_eTeiError:
        PbInt = BCHP_XPT_PB0_INTR_PARSER_LENGTH_ERROR_SHIFT;
        break;

        case BXPT_PbInt_eLengthError:
        PbInt = BCHP_XPT_PB0_INTR_PARSER_TRANSPORT_ERROR_SHIFT;
        break;

        default:
        /* Pass through */
        break;
    }

    return BCHP_INT_ID_CREATE( RegAddr, PbInt );
}

bool BXPT_Playback_IsAvailable(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    unsigned int ChannelNo                      /* [in] Which channel to check. */
    )
{
    if( ChannelNo >= hXpt->MaxPlaybacks )
    {
        /* Bad playback channel number. Complain. */
        return false;
    }
    else if( hXpt->PlaybackHandles[ ChannelNo ].Opened )
    {
        return false;
    }

    return true;
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
        *ltsid = map.VirtualParserIsPlayback ? map.VirtualParserBandNum + 0x10 : map.VirtualParserBandNum;
    }
#else
    *ltsid = hPb->ChannelNo + 0x10;
#endif
}
