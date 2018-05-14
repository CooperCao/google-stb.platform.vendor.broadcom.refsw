/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include "bstd.h"
#include "bkni.h"
#include "bxpt_priv.h"
#include "bxpt_playback.h"
#include "bxpt.h"
#include "bxpt_spid.h"

#include "bchp_xpt_mcpb.h"
#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"
#ifdef BXPT_USE_HOST_AGGREGATOR
#include "bchp_xpt_mcpb_host_intr_aggregator.h"
#else
#include "bchp_xpt_mcpb_cpu_intr_aggregator.h"
#endif
#include "bchp_xpt_bus_if.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#if( BDBG_DEBUG_BUILD == 1 )
    BDBG_MODULE( xpt_playback );

    /* Enabling all BDBG_FILE_MODULEs adds significant overhead. It can easily hide some race conditions. */
    BDBG_FILE_MODULE( xpt_playback_descriptors );
    BDBG_FILE_MODULE( xpt_playback_regs );
#endif

#if BXPT_HAS_MCPB_VER_3
    /* These definitions came from the SW Guide to MCPB 3.0 doc, definition of the 12 word descriptor. */
    #define DESC_CFG_WORD  3
    #define MUX_CFG_WORD   4
    #define PID_CHNL_WORD  5
    #define NEXT_PKT_PACING_TIMESTAMP_WORD 7
    #define PKT2PKT_TIMESTAMP_DELTA_WORD  8
    #define NEXT_DESC_WORD_HI 10
    #define NEXT_DESC_WORD_LO 11
#else
    /* These definitions came from the SW Guide to MCPB 2.0 doc, definition of the 12 word descriptor. */
    #define DESC_CFG_WORD  4
    #define MUX_CFG_WORD   5
    #define PID_CHNL_WORD  6
    #define NEXT_PKT_PACING_TIMESTAMP_WORD 8
    #define PKT2PKT_TIMESTAMP_DELTA_WORD  9
    #define NEXT_DESC_WORD_LO 2
#endif

#define BXPT_P_PLAYBACK_DEFAULT_USE_PCR_TIMEBASE        false
#define BXPT_P_PLAYBACK_DEFAULT_TIMESTAMP_MODE          BXPT_TimestampMode_e30_2U_Mod300
#define BXPT_PB_MAX_SYNC_LENGTH                         ( 256 )
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    #define PB_PARSER_REG_STEPSIZE                          (BCHP_XPT_MCPB_CH1_DMA_DESC_ADDR - BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR)
#else
    #define PB_PARSER_REG_STEPSIZE                          (BCHP_XPT_MCPB_CH1_DMA_DESC_CONTROL - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL)
#endif
#define GPC_REG_STEPSIZE                                (BCHP_XPT_MCPB_GPC1_CTRL - BCHP_XPT_MCPB_GPC0_CTRL)

#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
/* 4 on 7278A0, 0x400 on 7278B0 */
#define PB_PARSER_BAND_ID_CTRL_STEPSIZE (BCHP_XPT_MCPB_CH1_PARSER_BAND_ID_CTRL - BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL)
#endif

#if ((BCHP_CHIP==7445 && BCHP_VER<BCHP_VER_C0) || (BCHP_CHIP==7145 && BCHP_VER<BCHP_VER_B0))
#define BXPT_P_PLAYBACK_DISABLE_SLOT_WISE_READ 1 /* SW7445-341 */
#endif

/* See SWSTB-7713 for details. */
#ifdef BCHP_XPT_MCPB_CH0_CPU_ID_GRAB_CHANNEL
    #define MCPB_GRAB_CHNL(handle) (BXPT_Playback_P_WriteReg(handle, BCHP_XPT_MCPB_CH0_CPU_ID_GRAB_CHANNEL, 1))
    #define MCPB_RELEASE_CHNL(handle) (BXPT_Playback_P_WriteReg(handle, BCHP_XPT_MCPB_CH0_CPU_ID_CFG_DONE, 1))
#else
    #define MCPB_GRAB_CHNL(handle)
    #define MCPB_RELEASE_CHNL(handle)
#endif

#define MCPB_TOP_WRITE( handle, regname, bitfield, val ) \
    { \
        uint32_t LocalReg; \
        LocalReg = BREG_Read32( handle->hRegister, BCHP_##regname ); \
        BCHP_SET_FIELD_DATA(LocalReg, regname, bitfield, val); \
        BREG_Write32( handle->hRegister, BCHP_##regname, LocalReg ); \
    };

#define MCPB_TOP_CHNL_WRITE( handle, regname, bitfield, val ) \
        BREG_Write32( handle->hRegister, BCHP_##regname, BCHP_FIELD_DATA( regname, bitfield, val ) \
        | BCHP_FIELD_DATA( regname, MCPB_CHANNEL_NUM, handle->ChannelNo ) );

#define MCPB_CHNL_WRITE( handle, regname, bitfield, val ) \
    { \
        uint32_t LocalReg; \
        LocalReg = BXPT_Playback_P_ReadReg( handle, BCHP_##regname ); \
        LocalReg &= ~( BCHP_MASK( regname, bitfield ) ); \
        LocalReg |= ( BCHP_FIELD_DATA( regname, bitfield, val ) ); \
        BXPT_Playback_P_WriteReg( handle, BCHP_##regname, LocalReg ); \
    };

#define MCPB_CHNL_WRITE_ADDR( handle, regname, bitfield, val ) \
    { \
        uint64_t LocalReg; \
        LocalReg = BXPT_Playback_P_ReadAddr( handle, BCHP_##regname ); \
        LocalReg &= ~( BCHP_MASK( regname, bitfield ) ); \
        LocalReg |= ( BCHP_FIELD_DATA( regname, bitfield, val ) ); \
        BXPT_Playback_P_WriteAddr( handle, BCHP_##regname, LocalReg ); \
    };

#define MCPB_TOP_READ_STATUS( handle, regname, bitfield ) \
    (BCHP_GET_FIELD_DATA( BXPT_Playback_P_ReadReg( handle, BCHP_##regname ), regname, bitfield ) & (1 << hPb->ChannelNo))

#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
#define BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
#endif

static BERR_Code setChannelPacketSettings(BXPT_Playback_Handle hPb, const BXPT_Playback_ChannelPacketSettings *ChannelSettings);
static BERR_Code setChannelSettings(BXPT_Playback_Handle hPb, const BXPT_Playback_ChannelSettings *ChannelSettings);
static BERR_Code disablePacketizers(BXPT_Playback_Handle hPb);

static void BXPT_Playback_P_WriteAddr(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr,
    uint64_t RegVal
    )
{
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR + hPb->BaseAddr;
#else
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL + hPb->BaseAddr;
#endif
    BDBG_MODULE_MSG( xpt_playback_regs, ( "write reg 0x%08X: value " BDBG_UINT64_FMT "", RegAddr, BDBG_UINT64_ARG(RegVal) ));
    BREG_WriteAddr( hPb->hRegister, RegAddr, RegVal );
}

static uint64_t BXPT_Playback_P_ReadAddr(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr
    )
{
    uint64_t Reg;

#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR + hPb->BaseAddr;
#else
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL + hPb->BaseAddr;
#endif
    Reg = BREG_ReadAddr( hPb->hRegister, RegAddr );
    BDBG_MODULE_MSG( xpt_playback_regs, ( "read reg 0x%08X: value " BDBG_UINT64_FMT "", RegAddr, BDBG_UINT64_ARG(Reg) ));
    return Reg;
}

static BERR_Code SetGlobalPacingCounter(
    BXPT_Playback_Handle hPb
    )
{
    uint32_t reg;
    BERR_Code exitCode = BERR_SUCCESS;
    uint32_t regAddr = BCHP_XPT_MCPB_GPC0_CTRL + (GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index);

    BDBG_ASSERT( hPb->settings.PacingCounter->Allocated );

    if( hPb->settings.UsePcrTimeBase && hPb->settings.WhichPcrToUse >= BXPT_NUM_PCRS )
    {
        BDBG_ERR(( "Unsupported WhichPcrToUse %u", hPb->settings.WhichPcrToUse ));
        exitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    reg = BREG_Read32( hPb->hRegister, regAddr );
    reg &= ~(
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, COUNTER_MODE ) |
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, TIMEBASE_SEL ) |
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, FREE_RUN_OR_LOCKED_TIMEBASE )
    );
    reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, COUNTER_MODE, BXPT_TimestampMode_e30_2U_Binary == hPb->settings.TimestampMode ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, TIMEBASE_SEL, hPb->settings.WhichPcrToUse ) |
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, FREE_RUN_OR_LOCKED_TIMEBASE, hPb->settings.UsePcrTimeBase ? 1 : 0 )
    );
    BREG_Write32( hPb->hRegister, regAddr, reg );

    Done:
    return exitCode;
}

#if BXPT_HAS_TSMUX
BERR_Code BXPT_Playback_P_LoadPacingCounter(
    BXPT_Playback_Handle hPb,
    unsigned long pacingCount
    )
{
    uint32_t regBase;

    BERR_Code status = BERR_SUCCESS;

    if( hPb->settings.PacingCounter )
    {
        regBase = GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index;
        BREG_Write32( hPb->hRegister, BCHP_XPT_MCPB_GPC0_CURR_COUNTER_VAL + regBase, pacingCount + 0xFF );
    }
    else
    {
        BDBG_ERR(( "Invalid PacingCounter "));
        status = BERR_INVALID_PARAMETER;
    }

    return status;
}

BERR_Code BXPT_Playback_P_LoadStcMuxDelayDiff(
    BXPT_Playback_Handle hPb
    )
{
    uint32_t reg;

    BERR_Code status = BERR_SUCCESS;

    if( !hPb->settings.PacingCounter )
    {
        BDBG_ERR(( "Invalid PacingCounter "));
        status = BERR_INVALID_PARAMETER;
        goto Done;
    }

    reg = BREG_Read32( hPb->hRegister, BCHP_XPT_MCPB_GPC_COUNT_RELOAD_CTRL );
    reg &= ~(
        BCHP_MASK( XPT_MCPB_GPC_COUNT_RELOAD_CTRL, LD_STC_MUX_DELAY_DIFF ) |
        BCHP_MASK( XPT_MCPB_GPC_COUNT_RELOAD_CTRL, GPC_NUMBER )
    );
    reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_GPC_COUNT_RELOAD_CTRL, LD_STC_MUX_DELAY_DIFF, 1 ) |
        BCHP_FIELD_DATA( XPT_MCPB_GPC_COUNT_RELOAD_CTRL, GPC_NUMBER, hPb->settings.PacingCounter->Index )
    );
    BREG_Write32( hPb->hRegister, BCHP_XPT_MCPB_GPC_COUNT_RELOAD_CTRL, reg );

    Done:
    return status;
}

BERR_Code BXPT_Playback_P_SetPacingSpeed(
    BXPT_Playback_Handle hPb,
    unsigned speed
    )
{
    uint32_t regBase, reg;

    unsigned PacingSpeedBitField = 0;
    BERR_Code status = BERR_SUCCESS;

    if( hPb->settings.PacingCounter )
    {
        switch( speed )
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
            BDBG_ERR(( "Invalid pacing speed %u", speed ));
            status = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto Done;
            break;
        }

        regBase = BCHP_XPT_MCPB_GPC0_CTRL + (GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index);
        reg = BREG_Read32( hPb->hRegister, regBase );
        reg &= ~(
            BCHP_MASK( XPT_MCPB_GPC0_CTRL, PACING_SPEED )
        );
        reg |= (
            BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, PACING_SPEED, PacingSpeedBitField )
        );
        BREG_Write32( hPb->hRegister, regBase, reg );
    }
    else
    {
        BDBG_ERR(( "Invalid PacingCounter "));
        status = BERR_INVALID_PARAMETER;
    }

    Done:
    return status;
}
#endif

static BERR_Code CalcAndSetBlockout(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t BitRate            /* [in] Rate, in bits per second. */
    )
{
    uint32_t Reg, SyncLen, BoCount;

    SyncLen = BCHP_GET_FIELD_DATA( BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PKT_LEN ),
        XPT_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH );
    if(SyncLen == 0xB8)
        SyncLen = 0xBC;

    /* Do the math in units of 10kbps to avoid overflowing the 24-bit blockout bitfield in the register. */
    BoCount = (10800 * SyncLen * 8) / ( BitRate / 10000 );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL );
    Reg &= ~(
         BCHP_MASK( XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL, BO_COUNT ) |
         BCHP_MASK( XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL, BO_SPARE_BW_EN )
         );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL, BO_COUNT, BoCount )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL, Reg );

    return BERR_SUCCESS;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_Playback_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int *TotalChannels     /* [out] The number of playback channels. */
    )
{
    BDBG_ASSERT( hXpt );
    BSTD_UNUSED( hXpt );

    *TotalChannels = BXPT_NUM_PLAYBACKS;
    return BERR_SUCCESS;
}
#endif

BERR_Code BXPT_Playback_GetChannelDefaultSettings(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int ChannelNo,         /* [in] Which channel to get defaults from. */
    BXPT_Playback_ChannelSettings *ChannelSettings /* [out] The defaults */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( ChannelSettings );

    if( ChannelNo >= BXPT_NUM_PLAYBACKS )
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
        ChannelSettings->PcrBasedPacing = false;   /* Use legacy 4-byte timestamp pacing */
        ChannelSettings->PcrPacingPid = 0x1FFF;
        ChannelSettings->Use32BitTimestamps = false;

        /* Defaults, to keep existing behavior. */
        ChannelSettings->DescBasedPacing = false;
        ChannelSettings->Use8WordDesc = false;
    }

    return( ExitCode );
}

BERR_Code BXPT_Playback_OpenChannel(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_Playback_Handle *PlaybackHandle,   /* [out] Handle for opened record channel */
    unsigned int ChannelNo,                         /* [in] Which channel to open. */
    const BXPT_Playback_ChannelSettings *ChannelSettings /* [in] The defaults to use */
    )
{
    BXPT_Playback_ChannelSettings Defaults;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Playback_Handle hPb = NULL;

    BDBG_ASSERT( hXpt );

    if( !BXPT_Playback_IsAvailable( hXpt, ChannelNo ) )
    {
        BDBG_ERR(( "Channel Number %lu is out of range or already openned!", ( unsigned long ) ChannelNo ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }
    BDBG_MSG(( "BXPT_Playback_OpenChannel %u", ChannelNo ));

    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_eMcpb);

    if( !ChannelSettings )
    {
        BXPT_Playback_GetChannelDefaultSettings( hXpt, ChannelNo, &Defaults );
        ChannelSettings = &Defaults;
    }

    /* Create the playback handle. */
    hPb = &hXpt->PlaybackHandles[ ChannelNo ];

    /*
    ** Use the address of the first register in the playback block as the
    ** base address of the entire block.
    */
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    hPb->BaseAddr = BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR + ( ChannelNo * PB_PARSER_REG_STEPSIZE );
#else
    hPb->BaseAddr = BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL + ( ChannelNo * PB_PARSER_REG_STEPSIZE );
#endif
    hPb->hChip = hXpt->hChip;
    hPb->hRegister = hXpt->hRegister;
    hPb->ChannelNo = ChannelNo;
    hPb->LastDescriptor = 0;
    hPb->Running = false;
    hPb->vhXpt = hXpt;
    hPb->SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
    hPb->BitRate = 27000000;
    hPb->ForceResync = 0;

    hPb->PacingLoadMap = 0;
    hPb->PacingCount = 0;
    hPb->PacingLoadNeeded = false;

    MCPB_GRAB_CHNL(hPb);

    /* Restore all the stuff they could have changed through the API. */
#ifdef BCHP_XPT_MCPB_CH0_RUN
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_RUN, SET_CLEAR, 0 );
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_WAKE_SET, SET, 0 );
    MCPB_TOP_WRITE( hPb, XPT_MCPB_WAKE_MODE_CTRL_0_31, WAKE_MODE, 0 ); /* Set all channels to resume at end of chain. */
#else
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_RUN_SET_CLEAR, SET_CLEAR, 0 );
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_WAKE_SET, SET, 0 );
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_WAKE_MODE_SET_CLEAR, SET_CLEAR, 0 );
#endif

    /* Configure for 12-word descriptor */
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    MCPB_CHNL_WRITE_ADDR( hPb, XPT_MCPB_CH0_DMA_DESC_ADDR, LLD_TYPE, 2 );
#else
    MCPB_CHNL_WRITE_ADDR( hPb, XPT_MCPB_CH0_DMA_DESC_CONTROL, LLD_TYPE, 2 );
#endif

#ifdef BCHP_XPT_MCPB_CH0_DMA_DATA_CTRL
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CTRL, DRAM_REQ_SIZE, 256 );
#else
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CONTROL, DRAM_REQ_SIZE, 256 );
#endif
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_BBUFF_CTRL, STREAM_PROC_FEED_SIZE, 192 );

#ifdef BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL1
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL1, ALL_PASS_PID_CH_NUM, BXPT_GET_PLAYBACK_ALLPASS_CHNL( ChannelNo ) );
#endif

    /* Workaround for HW7445-756. Only needed on 7445Ax parts. HW's recommendation is
    always enable this workaroud. Impact is that the max data rate of all playback combined
    is reduced to 1.92 Gbps. */
#if (BCHP_CHIP == 7445 && BCHP_VER <= BCHP_VER_B0) || (BCHP_CHIP == 7145 && BCHP_VER == BCHP_VER_A0)
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_BBUFF_CTRL, DMA_BBUFF_SLOT_WISE_READ_EN, 1 );
#endif

    /* Now load the defaults they passed in. */
    BXPT_Playback_SetPacingErrorBound(hPb, 1024);
    ExitCode |= setChannelSettings( hPb, ChannelSettings );
    MCPB_RELEASE_CHNL(hPb);
    if(ExitCode)
        goto Error;

    hPb->Opened = true;
    hPb->Running = false;
    *PlaybackHandle = hPb;

    Done:
    return( ExitCode );

    Error:
    BXPT_P_ReleaseSubmodule(hPb->vhXpt, BXPT_P_Submodule_eMcpb);
    return ExitCode;
}

void BXPT_Playback_CloseChannel(
    BXPT_Playback_Handle hPb /* [in] Handle for the channel to close*/
    )
{
    BDBG_ASSERT(hPb);
    MCPB_GRAB_CHNL(hPb);

    if (hPb->mma.descBlock) {
        BMMA_Unlock(hPb->mma.descBlock, hPb->mma.descPtr);
        BMMA_UnlockOffset(hPb->mma.descBlock, hPb->mma.descOffset);
        hPb->mma.descBlock = NULL;
    }

    /* Stop the channel. */
#ifdef BCHP_XPT_MCPB_CH0_RUN
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_RUN, SET_CLEAR, 0 );
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_WAKE_SET, SET, 0 );
    MCPB_TOP_WRITE( hPb, XPT_MCPB_WAKE_MODE_CTRL_0_31, WAKE_MODE, 0 ); /* Set all channels to resume at end of chain. */
#else
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_RUN_SET_CLEAR, SET_CLEAR, 0 );
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_WAKE_SET, SET, 0 );
    MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_WAKE_MODE_SET_CLEAR, SET_CLEAR, 0 );
#endif

    /* Clean up all previous Packetization state, if any. */
    (void) disablePacketizers( hPb );
    MCPB_RELEASE_CHNL(hPb);
    BXPT_P_ReleaseSubmodule(hPb->vhXpt, BXPT_P_Submodule_eMcpb);

    hPb->Opened = false;
    hPb->Running = false;

    BDBG_MSG(( "BXPT_Playback_CloseChannel %u", hPb->ChannelNo ));
}

BERR_Code BXPT_Playback_SetChannelSettings(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelSettings *ChannelSettings /* [in] New settings to use */
    )
{
    BERR_Code ret;

    BDBG_ASSERT(hPb);
    MCPB_GRAB_CHNL(hPb);
    ret = setChannelSettings(hPb, ChannelSettings);
    MCPB_RELEASE_CHNL(hPb);
    return ret;
}

static BERR_Code setChannelSettings(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelSettings *ChannelSettings /* [in] New settings to use */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned atsFormat = 0;
    unsigned pacingType = 0;
    unsigned gpcSelect = 0;
    BXPT_Playback_ChannelPacketSettings PacketSettings;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ChannelSettings );

    /* unlock previous */
    if (hPb->mma.descBlock) {
        BMMA_Unlock(hPb->mma.descBlock, hPb->mma.descPtr);
        BMMA_UnlockOffset(hPb->mma.descBlock, hPb->mma.descOffset);
        hPb->mma.descBlock = NULL;
    }

    hPb->mma.descBlock = ChannelSettings->descBlock;
    if (hPb->mma.descBlock) {
        hPb->mma.descPtr = (uint8_t*)BMMA_Lock(ChannelSettings->descBlock) + ChannelSettings->descOffset;
        hPb->mma.descOffset = BMMA_LockOffset(ChannelSettings->descBlock) + ChannelSettings->descOffset;
    }

    hPb->settings = *ChannelSettings;
    PacketSettings.SyncMode = ChannelSettings->SyncMode;
    PacketSettings.PacketLength = ChannelSettings->PacketLength;
    PacketSettings.TimestampEn = ChannelSettings->TimestampEn;
    PacketSettings.DisableTimestampParityCheck = ChannelSettings->DisableTimestampParityCheck;

    /* Check: PacingOffsetAdjustDisable was removed since it was intended for debug only. */
    if( hPb->settings.PacingOffsetAdjustDisable )
    {
        BDBG_WRN(( "PacingOffsetAdjustDisable is unsupported on this chip." ));
    }

    ExitCode = setChannelPacketSettings(hPb, &PacketSettings );

#ifdef BCHP_XPT_MCPB_CH0_DMA_DATA_CTRL
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CTRL, ENDIAN_CONTROL, hPb->settings.SwapBytes == true ? 1 : 0 );
#else
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CONTROL, ENDIAN_CONTROL, hPb->settings.SwapBytes == true ? 1 : 0 );
#endif

    /* To implement RaveOutputOnly, need to walk through the PID table. Then remvoe this check. */
    if( hPb->settings.RaveOutputOnly )
        BDBG_ERR(( "RaveOutputOnly not implemented yet"));

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, PROGRAM_STREAM_MODE ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, PROGRAM_STREAM_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, PROGRAM_STREAM_MODE, hPb->settings.PsMode ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, PROGRAM_STREAM_EN, BXPT_Playback_PackHdr_Drop != hPb->settings.PackHdrConfig ? 1 : 0 )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG, Reg );

    switch( hPb->settings.TimestampMode )
    {
        case BXPT_TimestampMode_e28_4P_Mod300: atsFormat = 1; break;
        case BXPT_TimestampMode_e30_2U_Mod300: /* fall through */
        case BXPT_TimestampMode_e30_2U_Binary:
            atsFormat = hPb->settings.Use32BitTimestamps ? 0 : 2;
            break;
        default:
            BDBG_ERR(( "Unsupported TimestampMode %u", hPb->settings.TimestampMode ));
            ExitCode |= BERR_TRACE( BERR_INVALID_PARAMETER );
            goto Done;
    }

    if( hPb->settings.PcrBasedPacing )
        pacingType = 1;
    else if ( hPb->settings.DescBasedPacing )
        pacingType = 2;

    gpcSelect = hPb->settings.PacingCounter ? hPb->settings.PacingCounter->Index : 0;
    BDBG_MSG(("PB[%d] gpc = %d", hPb->ChannelNo, gpcSelect));
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_PAUSE_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, INPUT_ATS_FORMAT ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, GPC_SELECT ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_TYPE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_PAUSE_EN, hPb->settings.PacingPauseEn ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, INPUT_ATS_FORMAT, atsFormat ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, GPC_SELECT, gpcSelect ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_TYPE, pacingType )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL, Reg );

    /* If we have a global pacing counter, try to set it. */
    if( hPb->settings.PacingCounter && BERR_SUCCESS != SetGlobalPacingCounter( hPb ) )
        goto Done;

    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_TS_CONFIG, PCR_PACING_PID, hPb->settings.PcrPacingPid );

    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_AUTOSTART_ON_ERROR_EN, hPb->settings.ResetPacing ? 1 : 0 );

    Done:
    return( ExitCode );
}

BERR_Code BXPT_Playback_GetChannelSettings(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel. */
    BXPT_Playback_ChannelSettings *ChannelSettings /* [out] The current settings  */
    )
{
    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ChannelSettings );

    *ChannelSettings = hPb->settings;
    return BERR_SUCCESS;
}

static BERR_Code setChannelPacketSettings(
    BXPT_Playback_Handle hPb,                                  /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelPacketSettings *ChannelSettings /* [in] New settings to use */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned syncMode = 0, autoSync = 0;
    BDBG_ASSERT( hPb );

    if( ChannelSettings->PacketLength > BXPT_PB_MAX_SYNC_LENGTH )
    {
        BDBG_ERR(( "Packet length %d too long!", ChannelSettings->PacketLength ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH, ChannelSettings->PacketLength );
    hPb->settings.PacketLength = ChannelSettings->PacketLength;

    /* Need to recalc the blockout if the packet size has changed */
    CalcAndSetBlockout( hPb, hPb->BitRate );

    /*set the packet sync*/
    hPb->SyncMode = ChannelSettings->SyncMode;
    switch( ChannelSettings->SyncMode )
    {
        case BXPT_PB_SYNC_MPEG:       syncMode = 0; autoSync = 1; break;
        case BXPT_PB_SYNC_DSS:        syncMode = 1; autoSync = 0; break;
        case BXPT_PB_SYNC_PES:        syncMode = 2; autoSync = 0; break;
        case BXPT_PB_SYNC_BYPASS:     syncMode = 3; autoSync = 0; break;
        case BXPT_PB_SYNC_MPEG_BLIND: syncMode = 0; autoSync = 0; break;
        case BXPT_PB_SYNC_BULK:       syncMode = 6; autoSync = 0; break;
        default:
            BDBG_ERR(( "Unsupported sync mode %u", ChannelSettings->SyncMode ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto Done;
    }

    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, syncMode );
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_TS_CONFIG, MPEG_TS_AUTO_SYNC_DETECT, autoSync );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_TS_CONFIG );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_TS_CONFIG, ATS_PARITY_CHECK_DIS ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_TS_CONFIG, INPUT_HAS_ATS )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_TS_CONFIG, ATS_PARITY_CHECK_DIS, ChannelSettings->DisableTimestampParityCheck == true ? 1 : 0  ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_TS_CONFIG, INPUT_HAS_ATS, ChannelSettings->TimestampEn == true ? 1 : 0  )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_TS_CONFIG, Reg );

    Done:
    return( ExitCode );
}

#ifdef ENABLE_PLAYBACK_MUX
BERR_Code BXPT_Playback_SetChannelPacketSettings(
    BXPT_Playback_Handle hPb,                                  /* [in] Handle for the playback channel. */
    const BXPT_Playback_ChannelPacketSettings *ChannelSettings /* [in] New settings to use */
    )
{
    BERR_Code ret;

    BDBG_ASSERT(hPb);
    MCPB_GRAB_CHNL(hPb);
    ret = setChannelPacketSettings(hPb, ChannelSettings);
    MCPB_RELEASE_CHNL(hPb);
    return ret;
}
#endif

BERR_Code BXPT_Playback_GetLastCompletedDescriptorAddress(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor **LastCompletedDesc       /* [in] Address of the current descriptor. */
    )
{
    uint64_t Reg;
    BMMA_DeviceOffset DescAddr;

    void *UserSpaceDescAddr = NULL;     /* return NULL if no descriptors have completed. */
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( LastCompletedDesc );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_STATUS );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_DCPM_STATUS, DESC_ADDRESS_STATUS ) )
    {
        /* A descriptor has completed. Status is good. */
        Reg = BXPT_Playback_P_ReadAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DESC_ADDR );
        DescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_DCPM_DESC_ADDR, DESC_ADDRESS );
        DescAddr <<= 4;  /* Convert to byte-address. */
        UserSpaceDescAddr = (uint8_t*) hPb->mma.descPtr + (DescAddr - hPb->mma.descOffset); /* offset -> cached address */
    }
    else
    {
        ExitCode = BERR_NOT_AVAILABLE;
    }

    *LastCompletedDesc = ( BXPT_PvrDescriptor * ) UserSpaceDescAddr;
    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
void BXPT_Playback_GetDefaultDescSettings(
    BXPT_Handle hXpt,                                   /* [in] Handle for this transport */
    BXPT_Playback_ExtendedDescSettings *settings        /* [out] Settings. */
    )
{
    BDBG_ASSERT( hXpt );
    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( settings );

    /* Zero is a good default for all the members */
    BKNI_Memset( (void *) settings, 0, sizeof( *settings ) );
}

BERR_Code BXPT_Playback_CreateExtendedDesc(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    const BXPT_Playback_ExtendedDescSettings *settings /* [in] Settings. */
    )
{
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(settings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
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
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(Desc);
    BSTD_UNUSED(Buffer);
    BSTD_UNUSED(BufferLength);
    BSTD_UNUSED(IntEnable);
    BSTD_UNUSED(ReSync);
    BSTD_UNUSED(NextDesc);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

void BXPT_SetLastDescriptorFlag(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc     /* [in] Descriptor to initialize */
    )
{
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(Desc);
    (void)BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}
#endif

BERR_Code BXPT_Playback_AddDescriptors(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *LastDesc,   /* [in] Last descriptor in new chain */
    BXPT_PvrDescriptor *FirstDesc   /* [in] First descriptor in new chain */
    )
{
    uint64_t DescPhysAddr;
    uint32_t *DescWords;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_PvrDescriptor *LastDescriptor = hPb->LastDescriptor;

    BDBG_ASSERT(hPb);
    BDBG_ASSERT(FirstDesc);
    DescWords = (uint32_t*)FirstDesc;

    MCPB_GRAB_CHNL(hPb);

#if( BDBG_DEBUG_BUILD == 1 )

    /* Walk through the list list, dumping each descriptor's contents */
    if( true /* 3 == hPb->ChannelNo */ )    /* Use the hPb->ChannelNo check if you're only concerned about certain channels. */
    {
        BXPT_PvrDescriptor *Desc = FirstDesc;
        BXPT_PvrDescriptor *HeadOfChain = FirstDesc;
        BMMA_DeviceOffset nextDescAddr;

        do {
            uint32_t *DescContents = (uint32_t*)Desc;

            BDBG_MODULE_MSG( xpt_playback_descriptors, ( "Desc %p: %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X", (void *) Desc,
                DescContents[0], DescContents[1], DescContents[2], DescContents[3], DescContents[4], DescContents[5],
                DescContents[6], DescContents[7], DescContents[8], DescContents[9], DescContents[10], DescContents[11]
            ));

#if BXPT_HAS_MCPB_VER_3
            nextDescAddr = Desc->NextDescAddrHi;
            nextDescAddr <<= 32;
            nextDescAddr += Desc->NextDescAddr;
#else
            nextDescAddr = Desc->NextDescAddr;
#endif
            if (Desc->NextDescAddr & TRANS_DESC_LAST_DESCR_IND) { break; }

            Desc = (BXPT_PvrDescriptor*)((uint8_t*)hPb->mma.descPtr + (nextDescAddr - hPb->mma.descOffset)); /* convert NextDescAddr -> cached ptr */
            BMMA_FlushCache(hPb->mma.descBlock, Desc, sizeof(BXPT_PvrDescriptor));
        }
        while( Desc != HeadOfChain );
    }
#endif

    /* for ES playback, the FORCE_RESYNC flag must be set for all descriptors. see SW7425-1672 */
    if( hPb->ForceResync )
    {
        /* cycle through all descriptors and set force_resync flag */
        BXPT_PvrDescriptor *CurrDesc = FirstDesc;
        BXPT_PvrDescriptor *HeadDesc = FirstDesc;
        BMMA_DeviceOffset nextDescAddr;

        do {
            BMMA_FlushCache(hPb->mma.descBlock, CurrDesc, sizeof (*CurrDesc));
            DescWords = (uint32_t*)CurrDesc;

            DescWords[DESC_CFG_WORD] |= TRANS_DESC_FORCE_RESYNC_FLAG;
            if (DescWords[NEXT_DESC_WORD_LO] & TRANS_DESC_LAST_DESCR_IND) {
                #define MCPB_DW4_PUSH_PARTIAL_PKT (1 << 28)
                DescWords[DESC_CFG_WORD] |= MCPB_DW4_PUSH_PARTIAL_PKT;
                BMMA_FlushCache(hPb->mma.descBlock, CurrDesc, sizeof (*CurrDesc));
                break;
            }

            BMMA_FlushCache(hPb->mma.descBlock, CurrDesc, sizeof (*CurrDesc));
#if BXPT_HAS_MCPB_VER_3
            nextDescAddr = CurrDesc->NextDescAddrHi;
            nextDescAddr <<= 32;
            nextDescAddr += CurrDesc->NextDescAddr;
#else
            nextDescAddr = CurrDesc->NextDescAddr;
#endif
            CurrDesc = (BXPT_PvrDescriptor*)((uint8_t*)hPb->mma.descPtr + (nextDescAddr - hPb->mma.descOffset)); /* convert DescWords[2] -> cached ptr */
        }
        while (CurrDesc != HeadDesc);
    }

    DescPhysAddr = hPb->mma.descOffset + (unsigned)((uint8_t*)FirstDesc - (uint8_t*)hPb->mma.descPtr); /* convert FirstDesc -> offset */

    /* Fail if the descriptor is on a MEMC that doesn't have bandwidth for playback. */
   {
       #include "bmrc.h"
       #include "bchp_memc_clients.h"

      unsigned memcIndex;
      BCHP_MemcClient pbReadClientId;
      BCHP_MemClientConfig memCfg;

      ExitCode = BERR_SUCCESS;
      BKNI_Memset(&memCfg, 0, sizeof(BCHP_MemClientConfig));
      for (memcIndex = 0; memcIndex < BCHP_P_MEMC_COUNT; memcIndex++)
         if (BCHP_OffsetOnMemc(hPb->hChip, DescPhysAddr, memcIndex))
         {
            pbReadClientId = BMRC_P_GetClientId(BCHP_MemcClient_eXPT_RD_PB, memcIndex);
            ExitCode = BCHP_GetMemcClientConfig(hPb->hChip, memcIndex, pbReadClientId, &memCfg);
            if (ExitCode != BERR_SUCCESS || (memCfg.blockout == 0x3FFFF && !memCfg.roundRobinEn))
            {
               BDBG_ERR(("Descriptor at offset " BDBG_UINT64_FMT " is not on an MEMC that has RTS for playback.", BDBG_UINT64_ARG(DescPhysAddr)));
               ExitCode =  BERR_TRACE(BERR_NOT_SUPPORTED);
               goto Done;
            }
            break;
         }
   }

    if( LastDescriptor )
    {
        /* Channel has a linked-list loaded already. Append this descriptor to the end and use WAKE_MODE = 0 (resume from the last descriptor). */
       DescWords = (uint32_t*)LastDescriptor;
       BMMA_FlushCache(hPb->mma.descBlock, LastDescriptor, sizeof(*LastDescriptor));
#if BXPT_HAS_MCPB_VER_3
       DescWords[NEXT_DESC_WORD_HI] = DescPhysAddr >> 32;
       DescWords[NEXT_DESC_WORD_LO] = DescPhysAddr & 0xFFFFFFFF;
#else
        DescWords[NEXT_DESC_WORD_LO] = DescPhysAddr;
#endif
        BMMA_FlushCache(hPb->mma.descBlock, LastDescriptor, sizeof(*LastDescriptor));
#ifdef BCHP_XPT_MCPB_CH0_RUN
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_WAKE_SET, SET, 1 );
#else
        MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_WAKE_SET, SET, 1 );
#endif
    }
    else
    {
        /* Channel does not have a linked-list loaded. Point the FIRST_DESC_ADDR to this descriptor and set RUN */

        /*
        ** The descriptor address field in the hardware register is wants the address
        ** in 16-byte blocks. See the RDB HTML for details. So, we must shift the
        ** address 4 bits to the right before writing it to the hardware. Note that
        ** the RDB macros will shift the value 4 bits to the left, since the address
        ** bitfield starts at bit 4. Confusing, but thats what the hardware and the
        ** RDB macros require to make this work.
        */
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
       MCPB_CHNL_WRITE_ADDR(hPb, XPT_MCPB_CH0_DMA_DESC_ADDR, FIRST_DESC_ADDRESS, DescPhysAddr >> 4);
#else
       MCPB_CHNL_WRITE_ADDR(hPb, XPT_MCPB_CH0_DMA_DESC_CONTROL, FIRST_DESC_ADDRESS, DescPhysAddr >> 4);
#endif
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_STATUS, 0 );

#if BXPT_P_PLAYBACK_DISABLE_SLOT_WISE_READ
{
        /* sanity check */
        uint32_t reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_DMA_BBUFF_CTRL );
        if (BCHP_GET_FIELD_DATA(reg, XPT_MCPB_CH0_DMA_BBUFF_CTRL, STREAM_PROC_FEED_SIZE) != 0xc0) {
            BDBG_WRN(("STREAM_PROC_FEED_SIZE required to be 192"));
        }
        MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_BBUFF_CTRL, DMA_BBUFF_SLOT_WISE_READ_EN, 0);
}
#endif

        if( true == hPb->Running )
    {
#ifdef BCHP_XPT_MCPB_CH0_RUN
       MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_RUN, SET_CLEAR, 1 );
#else
            MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_RUN_SET_CLEAR, SET_CLEAR, 1 );
#endif
    }
    }

    hPb->LastDescriptor = LastDesc;
    Done:
    MCPB_RELEASE_CHNL(hPb);
    return ExitCode;
}

BERR_Code BXPT_Playback_StartChannel(
    BXPT_Playback_Handle hPb     /* [in] Handle for the playback channel */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_MSG(( "Starting playback channel %lu", ( unsigned long ) hPb->ChannelNo ));
    MCPB_GRAB_CHNL(hPb);

    if( hPb->Running == true )
    {
        BDBG_ERR(( "Playback channel %lu already running", (unsigned long) hPb->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_RUNNING );
        goto Done;
    }

    if (!hPb->mma.descBlock) {
        ExitCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        goto Done;
    }

#ifdef BCHP_PWR_RESOURCE_XPT_PLAYBACK
    BCHP_PWR_AcquireResource(hPb->hChip, BCHP_PWR_RESOURCE_XPT_PLAYBACK);
#endif

    /* Need to enable the parser band. */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ENABLE, 1 );
#ifdef BCHP_XPT_MCPB_CH0_DMA_DATA_CTRL
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CTRL, RUN_VERSION, 0 );
#else
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DATA_CONTROL, RUN_VERSION, 0 );
#endif
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DCPM_LOCAL_PACKET_COUNTER, PACKET_COUNTER, 0 );

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_STATUS, 0 );

    BXPT_Playback_P_WriteAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DESC_ADDR, 0 );
    BXPT_Playback_P_WriteAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DESC_DONE_INT_ADDR, 0 );
    BXPT_Playback_P_WriteAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_CURR_DESC_ADDR, 0 );

#ifdef BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR
    BXPT_Playback_P_WriteAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR, 0 );
#else
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR_UPPER, 0 );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR_LOWER, 0 );
#endif

    hPb->Running = true;

    Done:
    MCPB_RELEASE_CHNL(hPb);
    return( ExitCode );
}

BERR_Code BXPT_Playback_StopChannel(
    BXPT_Playback_Handle hPb     /* [in] Handle for the playback channel */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_MSG(( "Stopping playback channel %lu", ( unsigned long ) hPb->ChannelNo ));

    if( hPb->Running == false )
    {
        BDBG_ERR(( "Playback channel %lu already stopped", (unsigned long) hPb->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_STOPPED );
    }

#if BXPT_P_PLAYBACK_DISABLE_SLOT_WISE_READ
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_BBUFF_CTRL, DMA_BBUFF_SLOT_WISE_READ_EN, 1);
#endif

    /* Stop the hw */
#ifdef BCHP_XPT_MCPB_CH0_RUN
       MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_RUN, SET_CLEAR, 0 );
#else
            MCPB_TOP_CHNL_WRITE( hPb, XPT_MCPB_RUN_SET_CLEAR, SET_CLEAR, 0 );
#endif

    /* Clear the host pause. */
    BXPT_Playback_Resume(hPb);

#if 0
    /* Clear the first desc addr (for cleaner debugging) */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_DMA_DESC_CONTROL, FIRST_DESC_ADDRESS, 0 );

    /* Need to disable the parser band. */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ENABLE, 0 );
#endif

#ifdef BCHP_PWR_RESOURCE_XPT_PLAYBACK
    BCHP_PWR_ReleaseResource(hPb->hChip, BCHP_PWR_RESOURCE_XPT_PLAYBACK);
#endif

    hPb->LastDescriptor = 0;
    hPb->Running = false;
    return( ExitCode );
}

static void SetPause(
    BXPT_Playback_Handle hPb,     /* [in] Handle for the playback channel */
    bool PauseEn
    )
{
    uint32_t Reg;

#ifdef BCHP_XPT_MCPB_MICRO_PAUSE_CTRL_0_31
    Reg = BREG_Read32( hPb->hRegister, BCHP_XPT_MCPB_MICRO_PAUSE_CTRL_0_31 );
    if (PauseEn) {
       Reg |= (1 << hPb->ChannelNo);
    }
    else {
       Reg &= ~(1 << hPb->ChannelNo);
    }
    BREG_Write32( hPb->hRegister, BCHP_XPT_MCPB_MICRO_PAUSE_CTRL_0_31, Reg );
#else
    Reg = BREG_Read32( hPb->hRegister, BCHP_XPT_MCPB_MICRO_PAUSE_SET_CLEAR );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_MICRO_PAUSE_SET_CLEAR, SET_CLEAR ) |
        BCHP_MASK( XPT_MCPB_MICRO_PAUSE_SET_CLEAR, MCPB_CHANNEL_NUM )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_MICRO_PAUSE_SET_CLEAR, SET_CLEAR, PauseEn ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_MCPB_MICRO_PAUSE_SET_CLEAR, MCPB_CHANNEL_NUM, hPb->ChannelNo )
    );
    BREG_Write32( hPb->hRegister, BCHP_XPT_MCPB_MICRO_PAUSE_SET_CLEAR, Reg );
#endif
}

BERR_Code BXPT_Playback_Pause(
    BXPT_Playback_Handle hPb     /* [in] Handle for the playback channel */
    )
{
    BDBG_ASSERT( hPb );
    SetPause( hPb, true );
    return BERR_SUCCESS;
}


BERR_Code BXPT_Playback_Resume(
    BXPT_Playback_Handle hPb     /* [in] Handle for the playback channel */
    )
{
    BDBG_ASSERT( hPb );
    SetPause( hPb, false );
    return BERR_SUCCESS;
}

BERR_Code BXPT_Playback_SetBitRate(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t BitRate            /* [in] Rate, in bits per second. */
    )
{
    BERR_Code ret;

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
    MCPB_GRAB_CHNL(hPb);
    ret = CalcAndSetBlockout( hPb, BitRate );
    MCPB_RELEASE_CHNL(hPb);
    return ret;
}

BERR_Code BXPT_Playback_ConfigPacing(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PacingControl Mode                 /* [in] New mode for pacing. */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hPb );
    MCPB_GRAB_CHNL(hPb);

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_RESTART ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_AUTOSTART_ON_FORCE_RESYNC_EN )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL, Reg );

    if( Mode == BXPT_PacingControl_eStart )
    {
        Reg |= (
            BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_RESTART, 1 ) |
            BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_EN, 1 ) |

            /*
            ** Autostart will re-arm the pacing logic automatically if a
            ** force-resync condition occurrs.
            */
            BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, PACING_AUTOSTART_ON_FORCE_RESYNC_EN, 1 )
        );
    }

    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL, Reg );
    MCPB_RELEASE_CHNL(hPb);
    return BERR_SUCCESS;
}

BERR_Code BXPT_Playback_SetPacingErrorBound(
    BXPT_Playback_Handle hPb,       /* [in] Handle for the playback channel */
    unsigned long MaxTsError        /* [in] Maximum timestamp error. */
    )
{
    BDBG_ASSERT( hPb );

    if ( MaxTsError > BXPT_MAX_TS_ERROR )
    {
        BDBG_ERR(( "MaxTsError supported exceeds %u.", BXPT_MAX_TS_ERROR ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_EARLY, TS_ERROR_BOUND, MaxTsError );
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_TMEU_TS_ERR_BOUND_LATE, TS_ERROR_BOUND, MaxTsError );

    return BERR_SUCCESS;
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

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( Cfg );
    BSTD_UNUSED( Context );
    MCPB_GRAB_CHNL(hPb);

    hXpt = (BXPT_Handle) hPb->vhXpt;

    if( Enable )
    {
        if (Cfg->PacketizerMode == BXPT_Playback_PacketizerMode_Es)
        {
            hPb->ForceResync = true; /* for ES playback, the FORCE_RESYNC flag must be set for all descriptors. see SW7425-1672 */
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PES_ES_CONFIG, PROGRAM_STREAM_EN, 0 );
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, 3 );     /* Bypass mode */
        }
        else
        {
            hPb->ForceResync = false;
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, 2 );     /* PES mode */
        }

        MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH, 184 );     /* chunk size for ES, ignored in PES mode */
        MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, 1 );

        Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG );
        Reg &= ~(
            BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN ) |
            BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN )
            );

        switch(Cfg->PacketizerMode)
        {
        case BXPT_Playback_PacketizerMode_Es:
            BDBG_MSG(( "PacketizerMode_Es chnl %u", hPb->ChannelNo ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = 0xFF;
            SidFilter.FilterConfig.StreamIdRange.Lo = 0x00;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 1 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 0 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 0 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_MapAll:
            BDBG_MSG(( "PacketizerMode_Pes_MapAll chnl %u", hPb->ChannelNo ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = 0xff;
            SidFilter.FilterConfig.StreamIdRange.Lo = 0x00;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_Sid:
            BDBG_MSG(( "PacketizerMode_Pes_Sid chnl %u, PID chnl %u, StreamId 0x%02X", hPb->ChannelNo,
                Cfg->ChannelNum, Cfg->FilterConfig.StreamId ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamId;
            SidFilter.FilterConfig.StreamId = Cfg->FilterConfig.StreamId;
            BXPT_Spid_P_ConfigureChannelFilter( hXpt, Cfg->ChannelNum, SidFilter );

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidRange:
            BDBG_MSG(( "PacketizerMode_Pes_SidRange chnl %u, PID chnl %u, StreamIdHi 0x%02X, StreamIdLo 0x%02X", hPb->ChannelNo,
                Cfg->ChannelNum, Cfg->FilterConfig.StreamIdRange.Hi, Cfg->FilterConfig.StreamIdRange.Lo ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdRange;
            SidFilter.FilterConfig.StreamIdRange.Hi = Cfg->FilterConfig.StreamIdRange.Hi;
            SidFilter.FilterConfig.StreamIdRange.Lo = Cfg->FilterConfig.StreamIdRange.Lo;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidExtension:
            BDBG_MSG(( "PacketizerMode_Pes_SidExtension chnl %u, PID chnl %u, StreamId 0x%02X, Extension 0x%02X", hPb->ChannelNo,
                Cfg->ChannelNum, Cfg->FilterConfig.StreamIdAndExtension.Id, Cfg->FilterConfig.StreamIdAndExtension.Extension ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_StreamIdExtension;
            SidFilter.FilterConfig.StreamIdAndExtension.Id = Cfg->FilterConfig.StreamIdAndExtension.Id;
            SidFilter.FilterConfig.StreamIdAndExtension.Extension = Cfg->FilterConfig.StreamIdAndExtension.Extension;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 1 )
            );
            break;

        case BXPT_Playback_PacketizerMode_Pes_SidSubSid:
            BDBG_MSG(( "PacketizerMode_Pes_SidSubSid chnl %u, PID chnl %u, StreamId 0x%02X, SubStreamId 0x%02X", hPb->ChannelNo,
                Cfg->ChannelNum, Cfg->FilterConfig.StreamIdAndSubStreamId.Id, Cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId ));
            SidFilter.Mode = BXPT_Spid_eChannelFilterMode_SubStreamId;
            SidFilter.FilterConfig.StreamIdAndSubStreamId.Id = Cfg->FilterConfig.StreamIdAndSubStreamId.Id;
            SidFilter.FilterConfig.StreamIdAndSubStreamId.SubStreamId = Cfg->FilterConfig.StreamIdAndSubStreamId.SubStreamId;
            BXPT_Spid_P_ConfigureChannelFilter(hXpt,Cfg->ChannelNum,SidFilter);

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg |= (
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN, 1 ) |
                BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN, 1 )
            );
            break;
        }
        BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG, Reg );

        /* Disable CC checking, but enable CC generation in the full parser. */
        BXPT_GetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
        ccConfig.Primary_CC_CheckEnable = false;
        ccConfig.Generate_CC_Enable = Cfg->preserveCC ? false : true;
        BXPT_SetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
    }
    else
    {
        BDBG_MSG(( "BXPT_Playback_PacketizerMode DISABLED" ));
        hPb->ForceResync = false;
        SidFilter.Mode = BXPT_Spid_eChannelFilterMode_Disable;
        BXPT_Spid_P_ConfigureChannelFilter( hXpt, Cfg->ChannelNum, SidFilter);

        BXPT_GetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );
        ccConfig.Primary_CC_CheckEnable = true;
        ccConfig.Generate_CC_Enable = false;
        BXPT_SetPidChannel_CC_Config( hXpt, Cfg->ChannelNum, &ccConfig );

        /*
        ** Check the packetizer in each SPID/PID channel. If all the channels that are mapped to
        ** this playback are Invalid, disable packetizing at the PB control reg.
        */
        for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
        {
            unsigned int Band, Pid;
            bool IsPlayback;

            BXPT_GetPidChannelConfig( hXpt, Index, &Pid, &Band, &IsPlayback );

            RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * Index );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );

            /* if this pid channnel is used for spid functions or disabled */
            i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );
            if((i >= BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
                && Band == hPb->ChannelNo && IsPlayback )
                continue;
            else
                break;
        }

        if( Index == BXPT_NUM_PID_CHANNELS )
        {
            /* All contexts where invalid, so disable packetizer in the PB control reg. */
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

            Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG );
            Reg &= ~(
                BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN ) |
                BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN )
                );
            BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG, Reg );

            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, 0 );    /* MPEG TS. */
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH, 0xBC );
            MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, 0 );
        }
    }

    MCPB_RELEASE_CHNL(hPb);
    return BERR_SUCCESS;
}

BERR_Code BXPT_Playback_DisablePacketizers(
    BXPT_Playback_Handle hPb                    /* [in] Handle for the playback channel */
    )
{
    BERR_Code ret;

    BDBG_ASSERT(hPb);
    MCPB_GRAB_CHNL(hPb);
    ret = disablePacketizers(hPb);
    MCPB_RELEASE_CHNL(hPb);
    return ret;
}

static BERR_Code disablePacketizers(
    BXPT_Playback_Handle hPb                    /* [in] Handle for the playback channel */
    )
{
    uint32_t Reg, RegAddr;
    unsigned Index;
    int32_t i;
    BXPT_PidChannel_CC_Config ccConfig;

    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_Handle hXpt = (BXPT_Handle)hPb->vhXpt;

    for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
    {
        /* if the pid channel input is this parser band cleanup the SPID table */
        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( 4 * Index );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        if(!BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ))
            continue;

        /* pb parser is the input */
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) != hPb->ChannelNo )
            continue;

        RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * Index );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* if this pid channnel is used for spid functions or disabled */
        i = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, SPID_MODE );
        if(i >= BXPT_Spid_eChannelMode_Disable && i <= BXPT_Spid_eChannelMode_Remap)
        {
            continue;
        }
        else
        {
            Reg &= ~(
                BCHP_MASK( XPT_FE_SPID_TABLE_i, SPID_MODE ) |
                BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_HI ) |
                BCHP_MASK( XPT_FE_SPID_TABLE_i, STREAM_ID_RANGE_STREAM_ID_LO )
            );
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

            BXPT_GetPidChannel_CC_Config( hXpt, Index, &ccConfig );
            ccConfig.Primary_CC_CheckEnable = true;
            ccConfig.Generate_CC_Enable = false;
            BXPT_SetPidChannel_CC_Config( hXpt, Index, &ccConfig );
        }
    }

    /* Disable packetizer in the PB control reg. */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PUSI_SET_DIS, 0 );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, STREAM_ID_EXT_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SUB_STREAM_ID_EXT_EN )
        );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG, Reg );

    /* These parser settings are always loaded when BXPT_Playback_PacketizeStream() is called. */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_STREAM_TYPE, 0 );    /* MPEG TS. */
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PKT_LEN, PACKET_LENGTH, 0xBC );
    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, 0 );

    return ExitCode;
}

BERR_Code BXPT_Playback_GetParserConfig(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_ParserConfig *ParserConfig    /* [out] The current settings */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ParserConfig );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_TS_CONFIG );
    ParserConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_SP_TS_CONFIG, INPUT_TEI_IGNORE );
    ParserConfig->AcceptNulls = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL );
    ParserConfig->AllPass = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL  );
    ParserConfig->ForceRestamping = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_TMEU_TIMING_CTRL, RE_TIMESTAMP_EN );
    ParserConfig->RestampInBinary = BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_TMEU_TIMING_CTRL, OUTPUT_ATS_MODE ) == 1 ? true : false;

#if BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00_PB0_ACCEPT_ADP_00_MASK != 0x00000001 || BXPT_NUM_PLAYBACKS > 32
    #error "PI NEEDS UPDATING"
#else
    Reg = BREG_Read32( hPb->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 );
    ParserConfig->AcceptAdapt00 = (Reg >> hPb->ChannelNo) & 0x01 ? true : false;
#endif
    return BERR_SUCCESS;
}

BERR_Code BXPT_Playback_SetParserConfig(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    const BXPT_Playback_ParserConfig *ParserConfig  /* [in] The new settings */
    )
{
    uint32_t Reg;
    BXPT_Handle hXpt;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( ParserConfig );
    hXpt = (BXPT_Handle) hPb->vhXpt;
    MCPB_GRAB_CHNL(hPb);

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_TS_CONFIG );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_TS_CONFIG, INPUT_TEI_IGNORE ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_TS_CONFIG, INPUT_TEI_IGNORE, ParserConfig->ErrorInputIgnore ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_TS_CONFIG, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, ParserConfig->AcceptNulls )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_TS_CONFIG, Reg );

    MCPB_CHNL_WRITE( hPb, XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD, ParserConfig->AllPass );

    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, RE_TIMESTAMP_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_TMEU_TIMING_CTRL, OUTPUT_ATS_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, RE_TIMESTAMP_EN, ParserConfig->ForceRestamping ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_TMEU_TIMING_CTRL, OUTPUT_ATS_MODE, ParserConfig->RestampInBinary ? 1 : 0 )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_TMEU_TIMING_CTRL, Reg );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00 );
    Reg &= ~(0x01 << hPb->ChannelNo);
    if( ParserConfig->AcceptAdapt00 )
        Reg |= (0x01 << hPb->ChannelNo);
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_PBP_ACCEPT_ADAPT_00, Reg );
    MCPB_RELEASE_CHNL(hPb);
    return BERR_SUCCESS;
}

void BXPT_Playback_P_WriteReg(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr,
    uint32_t RegVal
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR + hPb->BaseAddr;
#else
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL + hPb->BaseAddr;
#endif
    BDBG_MODULE_MSG( xpt_playback_regs, ( "write reg 0x%08X: value 0x%08X", RegAddr, RegVal ));
    BREG_Write32( hPb->hRegister, RegAddr, RegVal );
}

uint32_t BXPT_Playback_P_ReadReg(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    uint32_t Pb0RegAddr
    )
{
    uint32_t Reg;

    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
#ifdef BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_ADDR + hPb->BaseAddr;
#else
    uint32_t RegAddr = Pb0RegAddr - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL + hPb->BaseAddr;
#endif
    Reg = BREG_Read32( hPb->hRegister, RegAddr );
    BDBG_MODULE_MSG( xpt_playback_regs, ( "read reg 0x%08X: value 0x%08X", RegAddr, Reg ));
    return Reg;
}

#ifdef ENABLE_PLAYBACK_MUX
void BXPT_Playback_SetDescBuf(
    BXPT_Handle hXpt,                       /* [in] Handle for this transport */
    BXPT_PvrDescriptor * const Desc,        /* [in] Descriptor to initialize */
    uint8_t *Buffer,                        /* [in] Data buffer. */
    uint32_t BufferLength                   /* [in] Size of buffer (in bytes). */
    )
{
    /* deprecated due to MMA conversion */
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(Desc);
    BSTD_UNUSED(Buffer);
    BSTD_UNUSED(BufferLength);
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}
#endif /*ENABLE_PLAYBACK_MUX*/

#include "bchp_xpt_mcpb_desc_done_intr_l2.h"
#include "bchp_xpt_mcpb_misc_ts_range_err_intr_l2.h"
#ifdef BXPT_HAS_SEPARATE_OOS_INTR
#include "bchp_xpt_mcpb_misc_oos_intr_l2.h"
#endif
#include "bchp_xpt_mcpb_misc_tei_intr_l2.h"

BINT_Id BXPT_Playback_GetIntId(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_Int PbInt
    )
{
    uint32_t RegAddr = 0;

    BDBG_ASSERT( hPb );

    switch( PbInt )
    {
        case BXPT_PbInt_eDone:
#ifdef BCHP_XPT_MCPB_DESC_DONE_INTR_L2_HOST_STATUS
        RegAddr = BCHP_XPT_MCPB_DESC_DONE_INTR_L2_HOST_STATUS;
#else
        RegAddr = BCHP_XPT_MCPB_DESC_DONE_INTR_L2_CPU_STATUS;
#endif
        break;

        case BXPT_PbInt_eTsRangeErr:
#ifdef BCHP_XPT_MCPB_MISC_TS_RANGE_ERR_INTR_L2_HOST_STATUS
        RegAddr = BCHP_XPT_MCPB_MISC_TS_RANGE_ERR_INTR_L2_HOST_STATUS;
#else
        RegAddr = BCHP_XPT_MCPB_MISC_TS_RANGE_ERR_INTR_L2_CPU_STATUS;
#endif
        break;

        case BXPT_PbInt_eOutOfSync:
#ifdef BXPT_HAS_SEPARATE_OOS_INTR
        RegAddr = BCHP_XPT_MCPB_MISC_OOS_INTR_L2_CPU_STATUS;
#else
        RegAddr = 0;
        BDBG_ERR(( "BXPT_PbInt_eOutOfSync not supported at this time." ));
#endif
        break;

        case BXPT_PbInt_eTeiError:
#ifdef BCHP_XPT_MCPB_MISC_TEI_INTR_L2_HOST_STATUS
        RegAddr = BCHP_XPT_MCPB_MISC_TEI_INTR_L2_HOST_STATUS;
#else
        RegAddr = BCHP_XPT_MCPB_MISC_TEI_INTR_L2_CPU_STATUS;
#endif
        break;

        default:
        BDBG_ERR(( "Unsupported/Invalid playback interrupt ID %u requested", (unsigned) PbInt ));
        BDBG_ASSERT(0);    /* Not much else that can be done here. */
    }

    return BCHP_INT_ID_CREATE( RegAddr, hPb->ChannelNo );
}

#if BXPT_HAS_TSMUX
void BXPT_Playback_GetMuxingInfo(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BXPT_Playback_MuxingInfo *Info
    )
{
    BDBG_ASSERT( hPb );
    BDBG_ASSERT( Info );

    if( hPb->settings.PacingCounter )
    {
        uint32_t regBase;

        regBase = GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index;
        Info->uiPacingCounter = BREG_Read32( hPb->hRegister, BCHP_XPT_MCPB_GPC0_CURR_COUNTER_VAL + regBase );
    }
    else
    {
        Info->uiPacingCounter = 0;
    }
}
#endif /* BXPT_HAS_TSMUX */

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

    return true;
}

void BXPT_Playback_P_EnableInterrupts(
        BXPT_Handle hXpt
        )
{
    /* Forward all interrupts to the host. */
#ifdef BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR,
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_STATUS_INTR_MASK |
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_PAUSE_AT_DESC_END_INTR_MASK |
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_TS_RANGE_ERR_INTR_MASK |
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_TEI_INTR_MASK |
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_FALSE_WAKE_INTR_MASK |
            BCHP_XPT_MCPB_HOST_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_DESC_DONE_INTR_MASK
    );
#else
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR,
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_CRC_COMPARE_ERROR_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_FALSE_WAKE_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_OOS_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_TS_PARITY_ERR_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_TEI_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_TS_RANGE_ERR_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_PES_NEXT_TS_RANGE_ERR_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_PAUSE_AT_DESC_READ_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_PAUSE_AT_DESC_END_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_PAUSE_AFTER_GROUP_PACKETS_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_MISC_DESC_TAGID_MISMATCH_INTR_MASK |
        BCHP_XPT_MCPB_CPU_INTR_AGGREGATOR_INTR_W0_MASK_CLEAR_MCPB_DESC_DONE_INTR_MASK
    );
#endif
}

void BXPT_Playback_P_Init(
    BXPT_Handle hXpt                        /* [in] Handle for this transport */
    )
{
    unsigned ii;
    uint32_t Reg, RegAddr;

    unsigned softInitializing = false;

    BDBG_ASSERT( hXpt );

#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
    for (ii=0; ii<BXPT_NUM_PLAYBACKS; ii++) {
        BREG_Write32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_CPU_ID_GRAB_CHANNEL + PB_PARSER_REG_STEPSIZE*ii, 1);
    }
#endif

    /* Init all MCPB registers to 0, including the RAM-based regs. */
    BREG_Write32( hXpt->hRegister, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DO_MEM_INIT,
        BCHP_FIELD_DATA( XPT_BUS_IF_SUB_MODULE_SOFT_INIT_DO_MEM_INIT, XPT_MCPB_SOFT_INIT_DO_MEM_INIT, 1 ) );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_SET,
        BCHP_FIELD_DATA( XPT_BUS_IF_SUB_MODULE_SOFT_INIT_SET, XPT_MCPB_SOFT_INIT_SET, 1 ) );
    ii = 300;
    do
    {
        softInitializing = BCHP_GET_FIELD_DATA( BREG_Read32( hXpt->hRegister, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_STATUS ),
            XPT_BUS_IF_SUB_MODULE_SOFT_INIT_STATUS, XPT_MCPB_SOFT_INIT_STATUS );
    }
    while( --ii && softInitializing );
    if( softInitializing )
    {
        BDBG_ERR(( "MCPB soft init failed" ));
        BDBG_ASSERT(0);
    }
    BREG_Write32( hXpt->hRegister, BCHP_XPT_BUS_IF_SUB_MODULE_SOFT_INIT_CLEAR,
        BCHP_FIELD_DATA( XPT_BUS_IF_SUB_MODULE_SOFT_INIT_CLEAR, XPT_MCPB_SOFT_INIT_CLEAR, 1 ) );

    /* init BO_COUNT */
    for (ii=0; ii<BXPT_NUM_PLAYBACKS; ii++) {
        Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL + PB_PARSER_REG_STEPSIZE*ii);
        BCHP_SET_FIELD_DATA(Reg, XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL, BO_COUNT, PWR_BO_COUNT);
        BREG_Write32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_TMEU_BLOCKOUT_CTRL + PB_PARSER_REG_STEPSIZE*ii, Reg);
    }

    BXPT_Playback_P_EnableInterrupts(hXpt);

    /* Set the defaults, since the register contents don't reflect them after chip reset */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
        {
            Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * ii);
            BCHP_SET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_SEL, 1);
            BCHP_SET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_BAND_ID, ii);
            BREG_Write32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * ii, Reg);
        }
#else
        RegAddr = BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL + ii * PB_PARSER_REG_STEPSIZE;
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_SEL ) |
            BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_SEL, 1 ) |
            BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID, ii )
        );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
#endif

        hXpt->BandMap.Playback[ ii ].VirtualParserIsPlayback = true;
        hXpt->BandMap.Playback[ ii ].VirtualParserBandNum = ii;

        RegAddr = BCHP_XPT_MCPB_CH0_SP_PES_ES_CONFIG + ii * PB_PARSER_REG_STEPSIZE;
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SYNC_ID_HIGH ) |
            BCHP_MASK( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SYNC_ID_LOW )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SYNC_ID_HIGH, 0xFF ) |
            BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PES_ES_CONFIG, SYNC_ID_LOW, 0xBC )
        );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    for( ii = 0; ii < BXPT_NUM_PACING_COUNTERS; ii++ )
    {
        hXpt->PacingCounters[ ii ].Allocated = false;
        hXpt->PacingCounters[ ii ].Index = ii;
    }

#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
    for (ii=0; ii<BXPT_NUM_PLAYBACKS; ii++) {
        BREG_Write32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_CPU_ID_CFG_DONE + PB_PARSER_REG_STEPSIZE*ii, 1);
    }
#endif
}

/* This API has to be ported to the 40 and 65 nm sets. */
#if (!B_REFSW_MINIMAL)
void BXPT_Playback_PvrDescriptorAdvDefault(
    BXPT_PvrDescriptor *desc
    )
{
#if 0
    BDBG_ASSERT( desc );
    BKNI_Memset((void *)desc, 0, sizeof( *desc ));
#else
    BSTD_UNUSED( desc );
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
#endif
}
#endif

BXPT_Playback_PacingCounter BXPT_Playback_AllocPacingCounter(
    BXPT_Handle hXpt           /* [in] Handle for this transport */
    )
{
    unsigned ii;

    BDBG_ASSERT( hXpt );

    for( ii = 0; ii < BXPT_NUM_PACING_COUNTERS; ii++ )
        if( !hXpt->PacingCounters[ ii ].Allocated )
        {
            uint32_t regBase;

            hXpt->PacingCounters[ ii ].Allocated = true;
            regBase = BCHP_XPT_MCPB_GPC0_CTRL + (GPC_REG_STEPSIZE * ii);
            BREG_Write32( hXpt->hRegister, regBase, 0 );
            return &( hXpt->PacingCounters[ ii ] );
        }

    BDBG_ERR(( "No playback pacing counters available." ));
    return NULL;
}

void BXPT_Playback_FreePacingCounter(
    BXPT_Playback_PacingCounter PacingCounter
    )
{
    BDBG_ASSERT( PacingCounter );
    PacingCounter->Allocated = false;
    /* ToDo: PB channels that were using it should unmap. */
}

#if BXPT_HAS_TSMUX
void BXPT_Playback_InitDescriptorFlags(
    BXPT_PvrDescriptorFlags *flags
    )
{
    BDBG_ASSERT( flags );
    BKNI_Memset((void *)flags, 0, sizeof( BXPT_PvrDescriptorFlags ));
}

void BXPT_Playback_SetDescriptorFlags(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,
    const BXPT_PvrDescriptorFlags *flags
    )
{
    uint32_t *descWords = NULL;

    BDBG_ASSERT( Desc );
    BDBG_ASSERT( flags );

    if (!hPb->mma.descBlock) {
        (void)BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    BMMA_FlushCache(hPb->mma.descBlock, Desc, sizeof(*Desc));
    descWords = (uint32_t *) Desc;

    descWords[MUX_CFG_WORD] &= ~(1 << 2);
    if( flags->muxFlags.bRandomAccessIndication )
        descWords[MUX_CFG_WORD] |= 1 << 2;

    descWords[MUX_CFG_WORD] &= ~(1 << 1);
    if( flags->muxFlags.bNextPacketPacingTimestampValid )
    {
        descWords[MUX_CFG_WORD] |= 1 << 1;
        descWords[NEXT_PKT_PACING_TIMESTAMP_WORD] = flags->muxFlags.uiNextPacketPacingTimestamp;
    }

    descWords[MUX_CFG_WORD] &= ~1;
    if( flags->muxFlags.bPacket2PacketTimestampDeltaValid )
    {
        descWords[MUX_CFG_WORD] |= 1;
        descWords[PKT2PKT_TIMESTAMP_DELTA_WORD] = flags->muxFlags.uiPacket2PacketTimestampDelta;
    }

    descWords[DESC_CFG_WORD] &= ~(1 << 28);
    if( flags->PushPartialPacket || flags->muxFlags.bPushPartialPacket )
        descWords[DESC_CFG_WORD] |= 1 << 28;

    descWords[DESC_CFG_WORD] &= ~(1 << 27);
    if( flags->muxFlags.bPushPreviousPartialPacket )
        descWords[DESC_CFG_WORD] |= 1 << 27;

    descWords[MUX_CFG_WORD] &= ~(1 << 20);
    if( flags->PauseAtDescEnd )
        descWords[MUX_CFG_WORD] |= 1 << 20;

    descWords[DESC_CFG_WORD] &= ~(1 << 29 );
    if( flags->muxFlags.bHostDataInsertion )
        descWords[DESC_CFG_WORD] |= 1 << 29;

    descWords[MUX_CFG_WORD] &= ~(1 << 19);
    if( flags->muxFlags.bPidChannelValid )
    {
        descWords[MUX_CFG_WORD] |= 1 << 19;
        descWords[PID_CHNL_WORD] = flags->muxFlags.uiPidChannelNo;
    }

    descWords[MUX_CFG_WORD] &= ~(1 << 22);
    if( flags->muxFlags.bInsertHostDataAsBtp )
    {
        descWords[MUX_CFG_WORD] |= (1 << 22);
    }

    BMMA_FlushCache(hPb->mma.descBlock, Desc, sizeof(*Desc));
    return;
}
#endif

unsigned BXPT_Playback_P_GetBandId(
    BXPT_Playback_Handle hPb
    )
{
#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
    uint32_t Reg;

    Reg = BREG_Read32(hPb->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * hPb->ChannelNo);
    return (unsigned) BCHP_GET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_BAND_ID);
#else
    /* RESUME SW7425-5193 here, then port to 65 and 40 nm code.*/
    return (unsigned) BCHP_GET_FIELD_DATA( BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL ),
        XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID );
#endif
}

unsigned int BXPT_PB_P_GetPbBandId(
    BXPT_Handle hXpt,
    unsigned int Band
    )
{
#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
    uint32_t Reg;

    Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * Band);
    return (unsigned) BCHP_GET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_BAND_ID);
#else
    uint32_t RegAddr = BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL + ( Band * PB_PARSER_REG_STEPSIZE );
    return (unsigned) BCHP_GET_FIELD_DATA( BREG_Read32( hXpt->hRegister, RegAddr ),
        XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID );
#endif
}

void BXPT_Playback_P_SetBandId(
    BXPT_Playback_Handle hPb,
    unsigned NewBandId
    )
{
    uint32_t Reg;

    BXPT_Handle hXpt = (BXPT_Handle) hPb->vhXpt;

    MCPB_GRAB_CHNL(hPb);

#ifdef BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL
    {
        Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * hPb->ChannelNo);
        BCHP_SET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_SEL, 1);
        BCHP_SET_FIELD_DATA(Reg, XPT_MCPB_CH0_PARSER_BAND_ID_CTRL, PB_PARSER_BAND_ID, NewBandId);
        BREG_Write32(hXpt->hRegister, BCHP_XPT_MCPB_CH0_PARSER_BAND_ID_CTRL + PB_PARSER_BAND_ID_CTRL_STEPSIZE * hPb->ChannelNo, Reg);
    }
#else
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_SEL ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_SEL, 1 ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PB_PARSER_BAND_ID, NewBandId )
    );
    BXPT_Playback_P_WriteReg( hPb, BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL, Reg );
#endif

    hXpt->BandMap.Playback[ hPb->ChannelNo ].VirtualParserBandNum = NewBandId;
    MCPB_RELEASE_CHNL(hPb);
}

#if !BXPT_HAS_MULTICHANNEL_PLAYBACK

/** These APIs are not supported on the current hw. The
 *  protos are here if we decide to handle the issue by
 *  returning BERR_NOT_SUPPORTED instead of failing to link.
 */
BERR_Code BXPT_Playback_GetChannelStatus(
    BXPT_Playback_Handle hPb,           /* [in] Handle for the playback channel. */
    BXPT_Playback_ChannelStatus *Status /* [out] Channel status. */
    )
{
    BSTD_UNUSED( hPb );
    BSTD_UNUSED( Status );
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_Playback_GetCurrentBufferAddress(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    uint32_t *Address                       /* [out] The address read from hardware. */
    )
{
    BSTD_UNUSED( hPb );
    BSTD_UNUSED( Address );

    /* Not used by Nexus or BPVRLib */
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_Playback_GetCurrentDescriptorAddress(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor **LastDesc       /* [in] Address of the current descriptor. */
    )
{
    BSTD_UNUSED( hPb );
    BSTD_UNUSED( LastDesc );

    /* Not used by Nexus or BPVRLib */
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_Playback_CheckHeadDescriptor(
    BXPT_Playback_Handle PlaybackHandle,    /* [in] Handle for the playback channel */
    BXPT_PvrDescriptor *Desc,       /* [in] Descriptor to check. */
    bool *InUse,                    /* [out] Is descriptor in use? */
    uint32_t *BufferSize            /* [out] Size of the buffer (in bytes). */
    )
{
    BSTD_UNUSED( PlaybackHandle );
    BSTD_UNUSED( Desc );
    BSTD_UNUSED( InUse );
    BSTD_UNUSED( BufferSize );
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
    return BERR_NOT_SUPPORTED;
}

BERR_Code BXPT_Playback_GetTimestampUserBits(
    BXPT_Playback_Handle hPb,   /* [in] Handle for the playback channel */
    unsigned int *Bits                          /* [out] The user bits read from hardware. */
    )
{
    BSTD_UNUSED( hPb );
    BSTD_UNUSED( Bits );
    BDBG_ERR(("BERR_NOT_SUPPORTED"));
    return BERR_NOT_SUPPORTED;
}

/** End of decprecated APIs. */
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

#if 0   /* SWSTB-7671: These APIs aren't used by Nexus. */
BERR_Code BXPT_Get_SkipRepeatSetting(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    BXPT_Playback_SkipRepeat *skipRepeatCfg
    )
{
    uint32_t reg;
    uint32_t regAddr;

    BERR_Code exitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( skipRepeatCfg );

    if( !hPb->settings.PacingCounter->Allocated )
    {
        BDBG_ERR(( "No pacing counter allocated for this playback" ));
        exitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    regAddr = BCHP_XPT_MCPB_GPC0_CTRL + (GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index);
    reg = BREG_Read32( hPb->hRegister, regAddr );
    skipRepeatCfg->mode = BCHP_GET_FIELD_DATA(reg, XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_MODE);
    skipRepeatCfg->enable = BCHP_GET_FIELD_DATA(reg, XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_EN);
    skipRepeatCfg->count = BCHP_GET_FIELD_DATA(reg, XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_COUNT);

    Done:
    return exitCode;
}

BERR_Code BXPT_Set_SkipRepeatSetting(
    BXPT_Playback_Handle hPb,    /* [in] Handle for the playback channel */
    const BXPT_Playback_SkipRepeat *skipRepeatCfg
    )
{
    uint32_t reg;
    uint32_t regAddr;

    BERR_Code exitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPb );
    BDBG_ASSERT( skipRepeatCfg );

    if( !hPb->settings.PacingCounter->Allocated )
    {
        BDBG_ERR(( "No pacing counter allocated for this playback" ));
        exitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    if( skipRepeatCfg->count > 0xFFFFF )
    {
        BDBG_ERR(( "Invalid skip/repeat count %u", skipRepeatCfg->count ));
        exitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    regAddr = BCHP_XPT_MCPB_GPC0_CTRL + (GPC_REG_STEPSIZE * hPb->settings.PacingCounter->Index);
    reg = BREG_Read32( hPb->hRegister, regAddr );
    reg &= ~(
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_MODE ) |
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_EN ) |
        BCHP_MASK( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_COUNT )
    );
    reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_MODE, skipRepeatCfg->mode ) |
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_EN, skipRepeatCfg->enable ) |
        BCHP_FIELD_DATA( XPT_MCPB_GPC0_CTRL, SKIP_REPEAT_COUNT, skipRepeatCfg->count )
    );
    BREG_Write32( hPb->hRegister, regAddr, reg );

    Done:
    return exitCode;
}
#endif

BERR_Code BXPT_Playback_GetLastCompletedDataAddress(
    BXPT_Playback_Handle hPb,                   /* [in] Handle for the playback channel */
    BMMA_DeviceOffset *LastCompletedDataAddress /* [out] Address of the last completed data buffer. */
    )
{
    uint64_t Reg;

    *LastCompletedDataAddress = 0;
    Reg = BXPT_Playback_P_ReadReg( hPb, BCHP_XPT_MCPB_CH0_DCPM_STATUS );
    if( BCHP_GET_FIELD_DATA( Reg, XPT_MCPB_CH0_DCPM_STATUS, DATA_ADDR_CUR_DESC_ADDR_STATUS ) )
    {
        /* A descriptor & data address update has completed. Status is good. */
#ifdef BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR
        Reg = BXPT_Playback_P_ReadAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR );
        *LastCompletedDataAddress = (BMMA_DeviceOffset)Reg;
#else
        Reg = BXPT_Playback_P_ReadAddr( hPb, BCHP_XPT_MCPB_CH0_DCPM_DATA_ADDR_LOWER );
        *LastCompletedDataAddress = (BMMA_DeviceOffset)Reg;
#endif
    }
    return BERR_SUCCESS;
}
