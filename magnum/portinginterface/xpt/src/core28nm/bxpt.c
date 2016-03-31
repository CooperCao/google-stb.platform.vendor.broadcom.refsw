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
#include "bxpt.h"
#include "bkni.h"
#include "bint.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#include "bxpt_pwr_mgmt_priv.h"
#endif
#include "bchp_xpt_pmu.h"

#if BXPT_HAS_FIXED_RSBUF_CONFIG
    #include "bxpt_rsbuf_priv.h"
    #include "bchp_xpt_rsbuff.h"
#endif

#if BXPT_HAS_FIXED_XCBUF_CONFIG
    #include "bxpt_xcbuf_priv.h"
#endif

#include "bxpt_rave.h"
#include "bxpt_pcr_offset.h"

#include "bchp_sun_top_ctrl.h"
#include "bchp_xpt_fe.h"

#if BXPT_HAS_MESG_BUFFERS
    #include "bchp_xpt_msg.h"
#endif

#if BXPT_HAS_MEMDMA
    #include "bxpt_dma.h"
#endif

#if BXPT_HAS_PLAYBACK_PARSERS
    #if BXPT_HAS_MULTICHANNEL_PLAYBACK
        #include "bxpt_playback.h"
    #else
        #include "bchp_xpt_pb0.h"
        #include "bchp_xpt_pb1.h"
        #define PB_PARSER_REG_STEPSIZE  ( BCHP_XPT_PB1_CTRL1 - BCHP_XPT_PB0_CTRL1 )
    #endif
#endif

#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_xmemif.h"
#include "bchp_xpt_rave.h"

#if BXPT_HAS_PACKETSUB
    #include "bchp_xpt_psub.h"
    #include "bxpt_packetsub.h"
    #define PACKET_SUB_REGISTER_STEP    ( BCHP_XPT_PSUB_PSUB1_CTRL0 - BCHP_XPT_PSUB_PSUB0_CTRL0 )
#endif

#if BXPT_HAS_TSMUX
    #include "bxpt_tsmux.h"
#endif

#if BXPT_HAS_REMUX
    #include "bxpt_remux.h"
#endif

/* Distance between Item X regs and Item X+1 */
#define PID_CHNL_STEPSIZE       ( 4 )
#define SPID_CHNL_STEPSIZE      ( 4 )

/* Locally defined interrupt IDs. The RDB structure makes it impossible to generate these automatically at the moment. */
#define BCHP_INT_ID_XPT_MSG_INTR_FLAG         BCHP_INT_ID_CREATE( BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31, BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31_INTR_FLAG_SHIFT )
#define BCHP_INT_ID_XPT_MSG_OVFL_INTR_FLAG    BCHP_INT_ID_CREATE( BCHP_XPT_MSG_BUF_OVFL_INTR_00_31, BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_INTR_FLAG_SHIFT )
#define MESG_FILTER_ARRAY_ELEMENTS  ( 512 )

#if BXPT_HAS_MTSIF
    #define MTSIF_STEPSIZE ( BCHP_XPT_FE_MTSIF_RX1_CTRL1 - BCHP_XPT_FE_MTSIF_RX0_CTRL1 )
#endif

#if( BDBG_DEBUG_BUILD == 1 )
    BDBG_MODULE( xpt );
#endif

void BXPT_Interrupt_P_Init(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    );
void BXPT_Interrupt_P_Shutdown(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    );

static void SetChannelEnable( BXPT_Handle hXpt, unsigned int PidChannelNum, bool EnableIt );
static bool PidChannelHasDestination( BXPT_Handle hXpt, unsigned int PidChannelNum );
static bool IsPidDuplicated( BXPT_Handle hXpt, unsigned int PidChannelNum );
static void BXPT_P_ConfigArbiter(BREG_Handle hReg);
void BXPT_Mesg_SetPid2Buff( BXPT_Handle hXpt, bool SetPid2Buff );
static unsigned int GetParserIndex( BXPT_Handle hXpt, unsigned ParserNum );

#ifndef BXPT_FOR_BOOTUPDATER
static void BXPT_P_FreeSharedXcRsBuffer(
    BXPT_Handle hXpt
    )
{
    if (hXpt->sharedRsXcBuff.block) {
        BMMA_UnlockOffset(hXpt->sharedRsXcBuff.block, hXpt->sharedRsXcBuff.offset);
        BMMA_Free(hXpt->sharedRsXcBuff.block);
        hXpt->sharedRsXcBuff.block = NULL;
        hXpt->sharedRsXcBuff.offset = 0;
    }
}
#endif /* BXPT_FOR_BOOTUPDATER */

 BERR_Code BXPT_P_AllocSharedXcRsBuffer(
    BXPT_Handle hXpt
    )
{
    BMMA_Block_Handle block;
    uint32_t Offset = 0;

    block = BMMA_Alloc(hXpt->mmaHeap, BXPT_P_MINIMUM_BUF_SIZE, 1 << 8, 0);
    if (!block) {
        BDBG_ERR(("Shared XC/RS buffer alloc failed!"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    Offset = BMMA_LockOffset(block);
    hXpt->sharedRsXcBuff.block = block;
    hXpt->sharedRsXcBuff.offset = Offset;

    return BERR_SUCCESS;
}

BERR_Code BXPT_GetDefaultSettings(
    BXPT_DefaultSettings *Defaults, /* [out] Defaults to use during init.*/
    BCHP_Handle hChip               /* [in] Handle to used chip. */
    )
{
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Defaults );
    BDBG_ASSERT( hChip );

    BSTD_UNUSED( hChip );
    BSTD_UNUSED( ii );      /* Might not be used if no XC and RS buffers are instantiated. */

    BKNI_Memset( Defaults, 0, sizeof( *Defaults) );

    #if BXPT_HAS_FIXED_XCBUF_CONFIG || BXPT_HAS_FIXED_RSBUF_CONFIG
        #if BXPT_HAS_IB_PID_PARSERS
        for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
        {
            Defaults->BandwidthConfig.MaxInputRate[ ii ] = BXPT_P_INITIAL_BUF_BITRATE;
            Defaults->BandwidthConfig.IbParserClients[ ii ].ToRave = true;
            Defaults->BandwidthConfig.IbParserClients[ ii ].ToMsg = true;
            #if BXPT_HAS_REMUX
            {
                unsigned remuxNum;

                for( remuxNum = 0; remuxNum < BXPT_NUM_REMULTIPLEXORS; remuxNum++ )
                {
                    Defaults->BandwidthConfig.IbParserClients[ ii ].ToRmx[ remuxNum ] = true;
                }
            }
            #endif
            Defaults->BandwidthConfig.IbParserClients[ ii ].ToMpodRs = false;
        }
        #endif

        #if BXPT_HAS_PLAYBACK_PARSERS
        for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
        {
            Defaults->BandwidthConfig.MaxPlaybackRate[ ii ] = BXPT_P_INITIAL_BUF_BITRATE;
            Defaults->BandwidthConfig.PlaybackParserClients[ ii ].ToRave = true;
            Defaults->BandwidthConfig.PlaybackParserClients[ ii ].ToMsg = true;
            #if BXPT_HAS_REMUX
            {
                unsigned remuxNum;

                for( remuxNum = 0; remuxNum < BXPT_NUM_REMULTIPLEXORS; remuxNum++ )
                {
                    Defaults->BandwidthConfig.PlaybackParserClients[ ii ].ToRmx[ remuxNum ] = true;
                }
            }
            #endif
        }
        #endif
    #endif

    #if BXPT_HAS_REMUX
    for( ii = 0; ii < BXPT_NUM_REMULTIPLEXORS; ii++ )
    {
        Defaults->BandwidthConfig.RemuxUsed[ ii ] = true;
    }
    #endif

    Defaults->MesgDataOnRPipe = false;

    #if BXPT_HAS_MTSIF
    for( ii = 0; ii < BXPT_NUM_MTSIF; ii++ )
    {
        Defaults->MtsifConfig[ ii ].RxInterfaceWidth = 8; /* 8 bits wide */
        Defaults->MtsifConfig[ ii ].RxClockPolarity = 0;    /* Neg edge */
        Defaults->MtsifConfig[ ii ].Enable = false;
    }
    #endif

    return( ExitCode );
}

static void BXPT_P_PMUMemPwr_Control(BREG_Handle hReg, bool powerOn, const BXPT_StandbySettings *pStandbySettings)
{
#ifdef BCHP_XPT_PMU_FE_SP_PD_MEM_PWR_DN_CTRL /* use single ifdef for all registers */
    uint32_t val = powerOn ? 0 : 0xffffffff;
#if (!BXPT_POWER_MANAGEMENT)
    val = 0;
#endif
    BREG_Write32(hReg, BCHP_XPT_PMU_FE_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_MCPB_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_MEMDMA_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_MSG_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_RAVE_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_TSIO_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_PCROFFSET_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_PSUB_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_RSBUFF_SP_PD_MEM_PWR_DN_CTRL, val);
    BREG_Write32(hReg, BCHP_XPT_PMU_XCBUFF_SP_PD_MEM_PWR_DN_CTRL, val);
    if (pStandbySettings && pStandbySettings->UseWakeupPacket) {
        BREG_Write32(hReg, BCHP_XPT_PMU_WAKEUP_SP_PD_MEM_PWR_DN_CTRL, 0);
    }
    else {
        BREG_Write32(hReg, BCHP_XPT_PMU_WAKEUP_SP_PD_MEM_PWR_DN_CTRL, val);
    }
#else
    BSTD_UNUSED(hReg);
    BSTD_UNUSED(powerOn);
    BSTD_UNUSED(pStandbySettings);
#endif
}

BERR_Code BXPT_Open(
    BXPT_Handle *hXpt,                      /* [out] Transport handle. */
    BCHP_Handle hChip,                      /* [in] Handle to used chip. */
    BREG_Handle hRegister,                  /* [in] Handle to access regiters. */
    BMMA_Heap_Handle hMmaHeap,              /* [in] Handle to memory heap to use. */
    BINT_Handle hInt,                       /* [in] Handle to interrupt interface to use. */
    const BXPT_DefaultSettings *Defaults    /* [in] Defaults to use during init.*/
    )
{
    BXPT_Handle lhXpt;
    unsigned i;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ENTER( BXPT_Open );

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hInt );
    BDBG_ASSERT( Defaults );

    /* Alloc memory from the system heap. */
    lhXpt = BKNI_Malloc( sizeof( BXPT_P_TransportData ) );
    if( lhXpt == NULL )
    {
        BDBG_ERR(( "BKNI_Malloc() failed!" ));
        ExitCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto done;
    }

    BKNI_Memset( (void *) lhXpt, 0, sizeof( BXPT_P_TransportData ));
    lhXpt->hChip = hChip;
    lhXpt->hRegister = hRegister;
    lhXpt->hInt = hInt;
    lhXpt->mmaHeap = hMmaHeap;
    lhXpt->mmaRHeap = Defaults->mmaRHeap;
    lhXpt->MesgDataOnRPipe = Defaults->MesgDataOnRPipe;

#ifdef BCHP_PWR_RESOURCE_XPT
    /* turn on the 108M and 216M for BXPT_Open */
#ifdef BCHP_PWR_RESOURCE_XPT_108M
    BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
    BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_SRAM
    BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_XPT_SRAM);
#endif
#endif

    /* Reset the hardware. Make sure the SCB client mapping is correct. */
    BXPT_P_ResetTransport( hRegister );
    BXPT_P_ConfigArbiter( hRegister );

    BXPT_P_PMUMemPwr_Control(hRegister, true, NULL); /* required for register access */

    BXPT_Dma_P_Init(lhXpt);

    /* Set the number of resources this transport has. */
    lhXpt->MaxPlaybacks = BXPT_NUM_PLAYBACKS;
    lhXpt->MaxPidChannels = BXPT_NUM_PID_CHANNELS;

#if BXPT_HAS_IB_PID_PARSERS
    lhXpt->MaxPidParsers = BXPT_NUM_PID_PARSERS;
    lhXpt->MaxInputBands = BXPT_NUM_INPUT_BANDS;
#endif

    lhXpt->MaxTpitPids = BXPT_NUM_TPIT_PIDS;

#if BXPT_HAS_MESG_BUFFERS
    lhXpt->MaxFilterBanks = BXPT_NUM_FILTER_BANKS;
    lhXpt->MaxFiltersPerBank = BXPT_NUM_FILTERS_PER_BANK;
#endif

#if BXPT_HAS_PACKETSUB
    lhXpt->MaxPacketSubs = BXPT_NUM_PACKETSUBS;
#endif

#if BXPT_HAS_DPCRS
    lhXpt->MaxPcrs = BXPT_NUM_PCRS;
#endif

    lhXpt->MaxRaveContexts = BXPT_NUM_RAVE_CONTEXTS;

#ifdef ENABLE_PLAYBACK_MUX
    /* By default, use one playback block for muxing. */
    lhXpt->NumPlaybackMuxes = 1;
#endif

    Reg = BREG_Read32( hRegister, BCHP_XPT_FE_IB_SYNC_DETECT_CTRL );
    BCHP_SET_FIELD_DATA( Reg, XPT_FE_IB_SYNC_DETECT_CTRL, IB_SYNC_IN_CNT, Defaults->syncInCount );
    BCHP_SET_FIELD_DATA( Reg, XPT_FE_IB_SYNC_DETECT_CTRL, IB_SYNC_OUT_CNT, Defaults->syncOutCount );
    BREG_Write32( hRegister, BCHP_XPT_FE_IB_SYNC_DETECT_CTRL, Reg );

#if BXPT_HAS_MTSIF
    for( i = 0; i < BXPT_NUM_MTSIF; i++ )
    {
        uint32_t RegAddr;
        unsigned ClockPol;

        unsigned IfWidth = 3;   /* 8 bit wide */

        if( Defaults->MtsifConfig[ i ].RxInterfaceWidth != 8 )
        {
            BDBG_ERR(( "Support for other than 8-bit wide MTSIF needs to be added" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            BKNI_Free( lhXpt );
            lhXpt = NULL;
            goto done;
        }

        if( Defaults->MtsifConfig[ i ].RxClockPolarity > 1 )
        {
            BDBG_ERR(( "Invalid MTSIF RX clock edge. Valid values are 0 and 1" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            BKNI_Free( lhXpt );
            lhXpt = NULL;
            goto done;
        }

    /* SW7425-1487: Invert the polarity on some parts. */
    #if (BCHP_CHIP == 7425 && BCHP_VER < BCHP_VER_B0) \
    || (BCHP_CHIP == 7346 && BCHP_VER < BCHP_VER_B0) \
    || (BCHP_CHIP == 7344 && BCHP_VER < BCHP_VER_B0) \
    || (BCHP_CHIP == 7422) \
    || (BCHP_CHIP == 7358 && BCHP_VER <= BCHP_VER_A1) \
    || (BCHP_CHIP == 7231 && BCHP_VER < BCHP_VER_B0)
        ClockPol = Defaults->MtsifConfig[ i ].RxClockPolarity;  /* Some parts do *NOT* need the flop. */
    #else
        ClockPol = Defaults->MtsifConfig[ i ].RxClockPolarity ? 0 : 1;  /* Some parts do. */
    #endif

        RegAddr = BCHP_XPT_FE_MTSIF_RX0_CTRL1 + i * MTSIF_STEPSIZE;
        Reg = BREG_Read32( hRegister, RegAddr );
        Reg &= ~(
             BCHP_MASK( XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_IF_WIDTH ) |
             BCHP_MASK( XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_CLOCK_POL_SEL ) |
             BCHP_MASK( XPT_FE_MTSIF_RX0_CTRL1, PARSER_ENABLE )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_IF_WIDTH, IfWidth ) |
            BCHP_FIELD_DATA( XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_CLOCK_POL_SEL, ClockPol ) |
            BCHP_FIELD_DATA( XPT_FE_MTSIF_RX0_CTRL1, PARSER_ENABLE, Defaults->MtsifConfig[ i ].Enable == true ? 1 : 0 )
        );
        BREG_Write32( hRegister, RegAddr, Reg );

        #ifdef BCHP_XPT_FE_MTSIF_RX0_SECRET_WORD
        BREG_Write32( hRegister, BCHP_XPT_FE_MTSIF_RX0_SECRET_WORD + (i * MTSIF_STEPSIZE), 0x829eecde );
        #endif
    }
#endif

    /* Create and init the PID channel table. */
    for( i = 0; i < lhXpt->MaxPidChannels; i++ )
    {
        /*
        ** On some devices, not all PID channels have a message buffer. HasMessageBuffer
        ** will be updated below, when we init the message buffer table
        */
#ifdef ENABLE_PLAYBACK_MUX
    #if BXPT_SW7425_1323_WORKAROUND
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, false, false, 0, false, false };
    #else
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, false, false, 0 };
    #endif
#else /*ENABLE_PLAYBACK_MUX*/
    #if BXPT_SW7425_1323_WORKAROUND
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, 0, false, false  };
    #else
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, 0 };
    #endif
#endif /*ENABLE_PLAYBACK_MUX*/

        BXPT_PidChannel_CC_Config DefaultCcConfig = { true, true, false, 0 };

        lhXpt->PidChannelTable[ i ] = InitEntry;

        BREG_Write32( hRegister, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( 4 * i ),
            1 << BCHP_XPT_FE_PID_TABLE_i_IGNORE_PID_VERSION_SHIFT
        );
        BREG_Write32( hRegister, BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );

        #if BXPT_P_HAS_SPID_EXTENSION_TABLE
        BREG_Write32( hRegister, BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        #endif

        lhXpt->CcConfigBeforeAllPass[ i ] = DefaultCcConfig;
        BXPT_SetPidChannel_CC_Config( lhXpt, i, &DefaultCcConfig );
    }

#if BXPT_HAS_MESG_BUFFERS
    /* Create and init the message buffer table. */
    for( i = 0; i < BXPT_NUM_MESG_BUFFERS; i++ )
    {
        MessageBufferEntry InitEntry = { false, 0, 0 };
        lhXpt->MessageBufferTable[ i ] = InitEntry;

        lhXpt->MesgBufferIsInitialized[ i ] = false;

        lhXpt->PidChannelTable[ i ].HasMessageBuffer = true;
        lhXpt->PidChannelTable[ i ].MessageBuffercount = 0;

#if BXPT_HAS_MESG_L2
#else
        lhXpt->MesgIntrCallbacks[ i ].Callback = ( BINT_CallbackFunc ) NULL;
        lhXpt->OverflowIntrCallbacks[ i ].Callback = ( BINT_CallbackFunc ) NULL;
#endif

        BREG_Write32( hRegister, BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_DMA_BP_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( 4 * i ), 0 );

        /* for normal legacy mode set false, set true to override settings */
        lhXpt->PidChannelParserConfigOverride[i] = false;
    }

    /* Mark all the PSI filters as not allocated ( or unused ). */
    for( i = 0; i < lhXpt->MaxFilterBanks; i++ )
    {
        unsigned j;

        for( j = 0; j < lhXpt->MaxFiltersPerBank; j++ )
        {
            lhXpt->FilterTable[ i ][ j ].IsAllocated = false;
        }
    }

    /* Clear the mesg filter banks. */
    for( i = 0; i < MESG_FILTER_ARRAY_ELEMENTS; i++ )
    {
        BREG_Write32( hRegister, BCHP_XPT_MSG_GEN_FILT_COEF_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_GEN_FILT_MASK_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_GEN_FILT_EXCL_i_ARRAY_BASE + ( 4 * i ), 0 );
    }

    #if BXPT_HAS_MESG_L2
    BXPT_Interrupt_P_Init( lhXpt );
    #endif

#endif

#if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_MESG_BUFFERS
    /*
    ** All parser bands initially do NOT modify PSI messages in the DMA buffers.
    ** This is the hardware default.
    */
    for( i = 0; i < BXPT_NUM_PID_PARSERS; i++ )
    {
        ParserConfig ParserInit = { false, false, false, BXPT_PsiMesgModModes_eNoMod };

        lhXpt->IbParserTable[ i ] = ParserInit;
        lhXpt->IsParserInAllPass[ i ] = false;
    }
#endif

    /* Each playback has a hard-wired PID parser. So init the table for those too. */
    for( i = 0; i < BXPT_NUM_PLAYBACKS; i++ )
    {
        ParserConfig ParserInit = { false, false, false, BXPT_PsiMesgModModes_eNoMod };

        lhXpt->PbParserTable[ i ] = ParserInit;
    }

    /* Init the RAVE structure in the xpt handle. */
    for( i = 0; i < BXPT_NUM_RAVE_CHANNELS; i++ )
    {
        lhXpt->RaveChannels[ i ].Allocated = false;
    }

    /* Init the RAVE structure in the xpt handle. */
    for( i = 0; i < BXPT_NUM_PCR_OFFSET_CHANNELS; i++ )
    {
        lhXpt->PcrOffsets[ i ].Handle = NULL;
        lhXpt->PcrOffsets[ i ].Allocated = false;
    }

#if BXPT_NUM_STC_SNAPSHOTS > 0
    for( i = 0; i < BXPT_NUM_STC_SNAPSHOTS; i++ )
    {
        lhXpt->StcSnapshots[ i ].Allocated = false;
        lhXpt->StcSnapshots[ i ].Index = i;
        lhXpt->StcSnapshots[ i ].WhichStc = i;
        lhXpt->StcSnapshots[ i ].hReg = hRegister;
    }
#endif

#if BXPT_HAS_MESG_BUFFERS && !BXPT_HAS_MESG_L2
    /* Register our message and overflow interrupts */
    ExitCode = BERR_TRACE( BINT_CreateCallback( &lhXpt->hMsgCb, lhXpt->hInt, BCHP_INT_ID_XPT_MSG_INTR_FLAG, (BINT_CallbackFunc) BXPT_P_Interrupt_MsgVector_isr, lhXpt, 0 ));
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("Unabled to create XPT Message Interrupt Callback!"));
        goto done;
    }

    ExitCode = BERR_TRACE( BINT_CreateCallback( &lhXpt->hMsgOverflowCb, lhXpt->hInt, BCHP_INT_ID_XPT_MSG_OVFL_INTR_FLAG, (BINT_CallbackFunc) BXPT_P_Interrupt_MsgOverflowVector_isr, lhXpt, 0 ));
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("Unabled to create XPT Message Overflow Callback!"));
        goto done;
    }
#endif

#if BXPT_HAS_FIXED_RSBUF_CONFIG
    ExitCode = BERR_TRACE( BXPT_P_RsBuf_Init( lhXpt, &Defaults->BandwidthConfig ) );
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("Rate smoothing buffer init FAILED!"));
        goto done;
    }
#endif

#if BXPT_HAS_FIXED_XCBUF_CONFIG
    ExitCode = BERR_TRACE( BXPT_P_XcBuf_Init( lhXpt, &Defaults->BandwidthConfig ) );
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("XPT client buffer init FAILED!"));
        goto done;
    }
#endif

#ifndef BXPT_FOR_BOOTUPDATER
    BXPT_P_PcrOffset_ModuleInit( lhXpt );
#endif

#if BXPT_HAS_MESG_BUFFERS
    BXPT_Mesg_SetPid2Buff( lhXpt, true );
#endif

#if BXPT_HAS_TSMUX
    BXPT_TsMux_P_ResetBandPauseMap( lhXpt );
#endif

#if BXPT_HAS_PARSER_REMAPPING
    {
        BXPT_ParserBandMapping ParserMap;
        unsigned ii;
        uint32_t Reg, RegAddr;

        /* Default virtual parser to equal the underlying physical parser. */
        for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
        {
            ParserMap.FrontEnd[ ii ].VirtualParserBandNum = ii;
            ParserMap.FrontEnd[ ii ].VirtualParserIsPlayback = false;
        }

        for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
        {
            ParserMap.Playback[ ii ].VirtualParserBandNum = ii;
            ParserMap.Playback[ ii ].VirtualParserIsPlayback = true;
        }

        BXPT_SetParserMapping( lhXpt, &ParserMap );

        #if BXPT_HAS_MULTICHANNEL_PLAYBACK
        BSTD_UNUSED( Reg );
        BSTD_UNUSED( RegAddr );
        #else
        /* Set the defaults, since the register contents don't reflect them after chip reset */
        for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
        {
            RegAddr = BCHP_XPT_PB0_PLAYBACK_PARSER_BAND_ID + ii * PB_PARSER_REG_STEPSIZE;

            Reg = BREG_Read32( hRegister, RegAddr );
            Reg &= ~(
                BCHP_MASK( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_SEL ) |
                BCHP_MASK( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID )
                );
            Reg |= (
                BCHP_FIELD_DATA( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_SEL, 1 ) |
                BCHP_FIELD_DATA( XPT_PB0_PLAYBACK_PARSER_BAND_ID, PB_PARSER_BAND_ID, ii )
            );
            BREG_Write32( hRegister, RegAddr, Reg );
        }
        #endif
    }
#endif

#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    BXPT_Playback_P_Init( lhXpt );
#endif

#if BXPT_HAS_DPCRS
    {
        unsigned Index;

        for( Index = 0; Index < BXPT_NUM_PCRS; Index++ )
        {
            lhXpt->JitterTimestamp[ Index ] = BXPT_PCR_JitterTimestampMode_eAuto;
            lhXpt->PbJitterDisable[ Index ] = BXPT_PCR_JitterCorrection_eAuto;
            lhXpt->LiveJitterDisable[ Index ] = BXPT_PCR_JitterCorrection_eAuto;
        }
    }
#endif

#if BXPT_HAS_MPOD_RSBUF
    for( i = 0; i < lhXpt->MaxPidParsers; i++ )
    {
        /* must be called after BXPT_SetParserMapping has been called above */
        uint32_t Addr = BXPT_P_GetParserCtrlRegAddr( lhXpt, i, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 );
        uint32_t Reg = BREG_Read32( hRegister, Addr );
        Reg &= ~(
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_MODE_SEL )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_MODE_SEL,
                Defaults->BandwidthConfig.IbParserClients[ i ].ToMpodRs ? 1 : 0 )
        );
        BREG_Write32( hRegister, Addr, Reg );
    }
#endif

#if BXPT_P_ENABLE_SUBMODULE_CLOCKGATING
    /* clock-gate submodules. these are ungated individually later, if used */
    Reg = BREG_Read32(hRegister, BCHP_XPT_PMU_CLK_CTRL);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, MSG_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, RAVE_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, PB_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, PSUB_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, PCROFFSET_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, XPT_IO_DISABLE, 1); /* RMX-related */
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, RMX0_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, RMX1_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, MPOD_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, XMEMIF_108_DISABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, XMEMIF_216_DISABLE, 1);
#ifdef BCHP_XPT_PMU_CLK_CTRL_TSIO_DISABLE_MASK
    BCHP_SET_FIELD_DATA(Reg, XPT_PMU_CLK_CTRL, TSIO_DISABLE, 1);
#endif
    BREG_Write32(hRegister, BCHP_XPT_PMU_CLK_CTRL, Reg);
#endif

    done:
    *hXpt = lhXpt;
    BDBG_LEAVE( BXPT_Open );

#ifdef BCHP_PWR_RESOURCE_XPT
    /* a failed BXPT_Open powers down the 216 and 108M */
    if (ExitCode!=BERR_SUCCESS) {
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
        BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_108M
        BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_SRAM
        BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_XPT_SRAM);
#endif
    }
#endif

    return( ExitCode );
}

#ifndef BXPT_FOR_BOOTUPDATER
void BXPT_Close(
    BXPT_Handle hXpt        /* [in] Handle for the Transport to be closed. */
    )
{
    unsigned int Index, i;

    BERR_Code Res;

    BDBG_ASSERT( hXpt );

    /* Shutdown PID parsers and playbacks before XC and RS buffers. */
#if BXPT_HAS_IB_PID_PARSERS
    for( Index = 0; Index < hXpt->MaxPidParsers; Index++ )
        BXPT_SetParserEnable( hXpt, Index, false );
#endif

#if BXPT_HAS_FIXED_RSBUF_CONFIG
    Res = BXPT_P_RsBuf_Shutdown( hXpt );
    BDBG_ASSERT( Res == BERR_SUCCESS );
#endif

#if BXPT_HAS_FIXED_XCBUF_CONFIG
    Res = BXPT_P_XcBuf_Shutdown( hXpt );
    BDBG_ASSERT( Res == BERR_SUCCESS );
#endif

    if (hXpt->sharedRsXcBuff.offset)
    {
        BXPT_P_FreeSharedXcRsBuffer( hXpt );
    }

    for( Index = 0; Index < BXPT_NUM_RAVE_CHANNELS; Index++ )
        if( hXpt->RaveChannels[ Index ].Allocated == true )
        {
            Res = BXPT_Rave_CloseChannel( ( BXPT_Rave_Handle ) hXpt->RaveChannels[ Index ].Handle );
            BDBG_ASSERT( Res == BERR_SUCCESS );
        }

    for( Index = 0; Index < BXPT_NUM_PCR_OFFSET_CHANNELS; Index++ )
        if( hXpt->PcrOffsets[ Index ].Allocated == true )
        {
            Res = BXPT_PcrOffset_Close( ( BXPT_PcrOffset_Handle ) hXpt->PcrOffsets[ Index ].Handle );
            BDBG_ASSERT( Res == BERR_SUCCESS );
        }

    for ( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ ) {
        if ( hXpt->PlaybackHandles[Index].Opened == true ) {
            BXPT_Playback_CloseChannel( (BXPT_Playback_Handle) &hXpt->PlaybackHandles[Index] );
        }
    }

#if BXPT_HAS_PACKETSUB
    for ( Index = 0; Index < BXPT_NUM_PACKETSUBS; Index++ ) {
        if ( hXpt->PacketSubHandles[Index].Opened == true ) {
            BXPT_PacketSub_CloseChannel( (BXPT_PacketSub_Handle) &hXpt->PacketSubHandles[Index]);
        }
    }
#endif

#if BXPT_HAS_REMUX
    for ( Index = 0; Index < BXPT_NUM_REMULTIPLEXORS; Index++ ) {
        if ( hXpt->RemuxHandles[Index].Opened == true ) {
            BXPT_Remux_CloseChannel( (BXPT_Remux_Handle) &hXpt->RemuxHandles[Index]);
        }
    }
#endif

#if BXPT_HAS_MESG_BUFFERS
    #if BXPT_HAS_MESG_L2
        BXPT_Interrupt_P_Shutdown( hXpt );
    #else
    BINT_DestroyCallback(hXpt->hMsgCb);
    BINT_DestroyCallback(hXpt->hMsgOverflowCb);
    #endif
#endif

    /* Reset the core, thus stopping any unwanted interrupts. */
    BXPT_P_ResetTransport( hXpt->hRegister );

    for (i=0; i<BXPT_P_Submodule_eMax; i++) {
        if (hXpt->power.refcnt[i]>0) {
            BDBG_WRN(("Submodule %s: left powered on (refcnt %u)", BXPT_P_SUBMODULE_STRING[i], hXpt->power.refcnt[i]));
        }
    }
    BXPT_P_PMUMemPwr_Control(hXpt->hRegister, false, NULL);

#ifdef BCHP_PWR_RESOURCE_XPT
#ifdef BCHP_PWR_RESOURCE_XPT_SRAM
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_SRAM);
#endif
    /* just in case we're still in standby */
    BXPT_P_FreeBackup(&hXpt->regBackup);
    BXPT_P_FreeBackup(&hXpt->sramBackup);

    /* release the 261 and 108M after BXPT_P_ResetTransport */
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_108M
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif
#endif

    /* Dont need the transport handle any more. */
    BKNI_Free( hXpt );
}


void BXPT_GetDefaultStandbySettings(
    BXPT_StandbySettings *pSettings
    )
{
#if BXPT_HAS_WAKEUP_PKT_SUPPORT
    pSettings->UseWakeupPacket = false;
#endif
    pSettings->S3Standby = false;
}

BERR_Code BXPT_Standby(
    BXPT_Handle hXpt,
    BXPT_StandbySettings *pSettings
    )
{
    BERR_Code rc;
#ifdef BCHP_PWR_RESOURCE_XPT
    unsigned int Index;
#endif
    BSTD_UNUSED(rc);
    BDBG_ASSERT(pSettings);

#ifdef BCHP_PWR_RESOURCE_XPT
    if (hXpt->bStandby) {
        BDBG_WRN(("Already in standby"));
        return BERR_SUCCESS;
    }

    /* check if XPT is still in use. if so, we cannot enter standby */

#if BXPT_HAS_IB_PID_PARSERS
    for( Index = 0; Index < hXpt->MaxPidParsers; Index++ ) {
            uint32_t Reg, RegAddr;
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, Index, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if (BCHP_GET_FIELD_DATA(Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE)) {
            BDBG_ERR(("One or more parsers still enabled. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

    for ( Index = 0; Index < BXPT_NUM_PLAYBACKS; Index++ ) {
        if ( hXpt->PlaybackHandles[Index].Running == true ) {
            BDBG_ERR(("One or more playback channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }

#if BXPT_HAS_PACKETSUB
    for ( Index = 0; Index < BXPT_NUM_PACKETSUBS; Index++ ) {
        if ( hXpt->PacketSubHandles[Index].Running == true ) {
            BDBG_ERR(("One or more packetsub channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

#if BXPT_HAS_REMUX
    for ( Index = 0; Index < BXPT_NUM_REMULTIPLEXORS; Index++ ) {
        if ( hXpt->RemuxHandles[Index].Running == true ) {
            BDBG_ERR(("One or more remux channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

    /* XPT_PMU_CLK_CTRL needs special handling, as it gates all reg R/W */
    hXpt->pmuClockCtrl = BREG_Read32(hXpt->hRegister, BCHP_XPT_PMU_CLK_CTRL);
    BREG_Write32(hXpt->hRegister, BCHP_XPT_PMU_CLK_CTRL, 0);

    /* if we get to this point, then XPT is not in use */
    if( pSettings->S3Standby )
    {
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
        BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif
        rc = BXPT_P_RegisterToMemory(hXpt->hRegister, &hXpt->regBackup, XPT_REG_SAVE_LIST);
        if (rc) return BERR_TRACE(rc);
        rc = BXPT_P_Dma_Standby(hXpt);
        if (rc) return BERR_TRACE(rc);
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
        BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif
    }
    hXpt->bS3Standby = pSettings->S3Standby;

    rc = BXPT_P_RegisterToMemory(hXpt->hRegister, &hXpt->sramBackup, XPT_SRAM_LIST);
    if (rc) return BERR_TRACE(rc);

    rc = BXPT_P_Mcpb_Standby(hXpt);
    if (rc) return BERR_TRACE(rc);

    BXPT_P_PMUMemPwr_Control(hXpt->hRegister, false, pSettings);

#ifdef BCHP_PWR_RESOURCE_XPT_SRAM
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_SRAM);
#endif

    /* XPT can now release the 216 and 108M */
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_108M
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif

#ifdef BCHP_PWR_RESOURCE_XPT_WAKEUP
    /* Keep the 216 clock running for the XPT_WAKEUP block */
    if( true == pSettings->UseWakeupPacket && !hXpt->WakeupEnabled )
    {
        BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_WAKEUP);
        hXpt->WakeupEnabled = true;
    }
#endif /* BCHP_PWR_RESOURCE_XPT_WAKEUP */

    hXpt->bStandby = true;

#else
    BSTD_UNUSED( hXpt );
#endif /* #ifdef BCHP_PWR_RESOURCE_XPT */

    return BERR_SUCCESS;
}

BERR_Code BXPT_Resume(
    BXPT_Handle hXpt
    )
{
#ifdef BCHP_PWR_RESOURCE_XPT
    if (!hXpt->bStandby) {
        BDBG_WRN(("Not in standby"));
        return BERR_SUCCESS;
    }
#ifdef BCHP_PWR_RESOURCE_XPT_XMEMIF
    BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif
#ifdef BCHP_PWR_RESOURCE_XPT_108M
    BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif

    /* required before any XPT register access */
    BREG_Write32(hXpt->hRegister, BCHP_XPT_PMU_CLK_CTRL, 0);
    BXPT_P_PMUMemPwr_Control(hXpt->hRegister, true, NULL);

#ifdef BCHP_PWR_RESOURCE_XPT_SRAM
    BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_SRAM);
#endif
    BXPT_P_MemoryToRegister(hXpt->hRegister, &hXpt->sramBackup, XPT_SRAM_LIST);
    BXPT_P_Mcpb_Resume(hXpt);

    if( hXpt->bS3Standby )
    {
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
        BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif
        BXPT_P_MemoryToRegister(hXpt->hRegister, &hXpt->regBackup, XPT_REG_SAVE_LIST);
        BXPT_P_Dma_Resume(hXpt);
#ifdef BCHP_PWR_RESOURCE_XPT_REMUX
        BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_REMUX);
#endif
    }

    if (hXpt->bS3Standby) {
        BXPT_P_RaveRamInit( (BXPT_Rave_Handle) hXpt->vhRave );
    }

#ifdef BCHP_PWR_RESOURCE_XPT_WAKEUP
    if( hXpt->WakeupEnabled )
    {
        BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_WAKEUP);
        hXpt->WakeupEnabled = false;
    }
#endif /* BCHP_PWR_RESOURCE_XPT_WAKEUP */

    BXPT_Playback_P_EnableInterrupts(hXpt);
#if BXPT_HAS_MESG_L2
    BXPT_Interrupt_P_Init( hXpt );
#endif
    BXPT_Rave_P_EnableInterrupts( hXpt );
#if BXPT_HAS_MEMDMA
    BXPT_Dma_P_EnableInterrupts( hXpt );
#endif

    BREG_Write32(hXpt->hRegister, BCHP_XPT_PMU_CLK_CTRL, hXpt->pmuClockCtrl);
    hXpt->bStandby = false;
#else
    BSTD_UNUSED( hXpt );
#endif /* #ifdef BCHP_PWR_RESOURCE_XPT */

    return BERR_SUCCESS;
}

#if (!B_REFSW_MINIMAL)
void BXPT_GetCapability(
    BXPT_Handle hXpt,           /* [in] Which transport to get data. */
    BXPT_Capability *Capability /* [out] Where to put the capability data. */
    )
{
    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( Capability );

    /*
    ** Set the values in the caller's structure.
    */
#if BXPT_HAS_PLAYBACK_PARSERS
    Capability->MaxPlaybacks = BXPT_NUM_PLAYBACKS;
    Capability->MaxPidChannels = BXPT_NUM_PID_CHANNELS;
#endif

#if BXPT_HAS_IB_PID_PARSERS
    Capability->MaxPidParsers = BXPT_NUM_PID_PARSERS;
    Capability->MaxInputBands = BXPT_NUM_INPUT_BANDS;
#endif

    Capability->MaxTpitPids = BXPT_NUM_TPIT_PIDS;

#if BXPT_HAS_MESG_BUFFERS
    Capability->MaxFilterBanks = BXPT_NUM_FILTER_BANKS;
    Capability->MaxFiltersPerBank = BXPT_NUM_FILTERS_PER_BANK;
#endif

#if BXPT_HAS_PACKETSUB
    Capability->MaxPacketSubs = BXPT_NUM_PACKETSUBS;
#endif

#if BXPT_HAS_DPCRS
    Capability->MaxPcrs = BXPT_NUM_PCRS;
#endif

    Capability->MaxRaveContexts = BXPT_NUM_RAVE_CONTEXTS;
}
#endif
#endif /* BXPT_FOR_BOOTUPDATER */

#if BXPT_HAS_IB_PID_PARSERS

BERR_Code BXPT_GetParserConfig(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    BXPT_ParserConfig *ParserConfig /* [out] The current settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %u is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        ParserConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE );
        ParserConfig->TsMode = BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE );
        ParserConfig->AcceptNulls = BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD );

#if BCHP_XPT_FULL_PID_PARSER_IBP_ACCEPT_ADAPT_00_PARSER0_ACCEPT_ADP_00_MASK != 0x00000001 || BXPT_NUM_PID_PARSERS > 32
    #error "PI NEEDS UPDATING"
#else
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_IBP_ACCEPT_ADAPT_00 );
        ParserConfig->AcceptAdapt00 = (Reg >> GetParserIndex (hXpt, ParserNum)) & 0x01 ? true : false;
#endif
    }

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_GetDefaultParserConfig(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    BXPT_ParserConfig *ParserConfig /* [out] The current settings */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        ParserConfig->ErrorInputIgnore = false;
        ParserConfig->TsMode = BXPT_ParserTimestampMode_eAutoSelect;
        ParserConfig->AcceptNulls = false;
        ParserConfig->AcceptAdapt00 = false;
    }

    return( ExitCode );
}
#endif

BERR_Code BXPT_SetParserConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    const BXPT_ParserConfig *ParserConfig   /* [in] The new settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %u is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~(
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE ) |
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE ) |
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD )
        );

        /* Now set the new values. */
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE, ParserConfig->ErrorInputIgnore ) |
            BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE, ParserConfig->TsMode ) |
            BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, ParserConfig->AcceptNulls )
        );

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );


#if BCHP_XPT_FULL_PID_PARSER_IBP_ACCEPT_ADAPT_00_PARSER0_ACCEPT_ADP_00_MASK != 0x00000001 || BXPT_NUM_PID_PARSERS > 32
    #error "PI NEEDS UPDATING"
#else
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_IBP_ACCEPT_ADAPT_00 );
        Reg &= ~(0x01 << ParserNum);
        if( ParserConfig->AcceptAdapt00 )
            Reg |= (0x01 << GetParserIndex (hXpt, ParserNum));
        BREG_Write32( hXpt->hRegister, BCHP_XPT_FULL_PID_PARSER_IBP_ACCEPT_ADAPT_00, Reg );
#endif
    }

    return( ExitCode );
}

bool BXPT_P_InputBandIsSupported(
    unsigned ib
    )
{
    bool isSupported = false;

    switch( ib )
    {
        #ifdef BCHP_XPT_FE_IB0_SYNC_COUNT
        case 0:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB1_SYNC_COUNT
        case 1:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB2_SYNC_COUNT
        case 2:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB3_SYNC_COUNT
        case 3:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB4_SYNC_COUNT
        case 4:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB5_SYNC_COUNT
        case 5:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB6_SYNC_COUNT
        case 6:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB7_SYNC_COUNT
        case 7:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB8_SYNC_COUNT
        case 8:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB9_SYNC_COUNT
        case 9:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB10_SYNC_COUNT
        case 10:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB11_SYNC_COUNT
        case 11:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB12_SYNC_COUNT
        case 12:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB13_SYNC_COUNT
        case 13:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB14_SYNC_COUNT
        case 14:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB15_SYNC_COUNT
        case 15:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB16_SYNC_COUNT
        case 16:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB17_SYNC_COUNT
        case 17:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB18_SYNC_COUNT
        case 18:
        isSupported = true;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB19_SYNC_COUNT
        case 19:
        isSupported = true;
        break;
        #endif

        default:
        break;
    }

    return isSupported;
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_GetDefaultInputBandConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    BXPT_InputBandConfig *InputBandConfig   /* [in] The current settings */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if ( !BXPT_P_InputBandIsSupported( BandNum ) )
    {
        BDBG_ERR(( "InputBand %u is not supported on this chip.", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        InputBandConfig->ClockPolSel = BXPT_Polarity_eActiveHigh;
        InputBandConfig->SyncPolSel = BXPT_Polarity_eActiveHigh;
        InputBandConfig->DataPolSel = BXPT_Polarity_eActiveHigh;
        InputBandConfig->ValidPolSel = BXPT_Polarity_eActiveHigh;
        InputBandConfig->ForceValid = false;
        InputBandConfig->SyncDetectEn = false;
        InputBandConfig->LsbFirst = false;
        InputBandConfig->UseSyncAsValid = false;
        InputBandConfig->ErrorPolSel = BXPT_Polarity_eActiveHigh;
        InputBandConfig->EnableErrorInput = false;
        InputBandConfig->IbPktLength = 188;     /* Default to MPEG */

        if ( BandNum == 4 )
            InputBandConfig->ParallelInputSel = true;
        else
            InputBandConfig->ParallelInputSel = false;
    }

    return( ExitCode );
}
#endif

uint32_t BXPT_P_GetInputBandRegAddr(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int BandNum                      /* [in] Which input band to access. */
    )
{
    uint32_t addr = 0;

    BSTD_UNUSED( hXpt );
    switch( BandNum )
    {
        #ifdef BCHP_XPT_FE_IB0_CTRL
        case 0:
        addr = BCHP_XPT_FE_IB0_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB1_CTRL
        case 1:
        addr = BCHP_XPT_FE_IB1_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB2_CTRL
        case 2:
        addr = BCHP_XPT_FE_IB2_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB3_CTRL
        case 3:
        addr = BCHP_XPT_FE_IB3_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB4_CTRL
        case 4:
        addr = BCHP_XPT_FE_IB4_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB5_CTRL
        case 5:
        addr = BCHP_XPT_FE_IB5_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB6_CTRL
        case 6:
        addr = BCHP_XPT_FE_IB6_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB7_CTRL
        case 7:
        addr = BCHP_XPT_FE_IB7_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB8_CTRL
        case 8:
        addr = BCHP_XPT_FE_IB8_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB9_CTRL
        case 9:
        addr = BCHP_XPT_FE_IB9_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB10_CTRL
        case 10:
        addr = BCHP_XPT_FE_IB10_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB11_CTRL
        case 11:
        addr = BCHP_XPT_FE_IB11_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB12_CTRL
        case 12:
        addr = BCHP_XPT_FE_IB12_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB13_CTRL
        case 13:
        addr = BCHP_XPT_FE_IB13_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB14_CTRL
        case 14:
        addr = BCHP_XPT_FE_IB14_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB15_CTRL
        case 15:
        addr = BCHP_XPT_FE_IB15_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB16_CTRL
        case 16:
        addr = BCHP_XPT_FE_IB16_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB17_CTRL
        case 17:
        addr = BCHP_XPT_FE_IB17_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB18_CTRL
        case 18:
        addr = BCHP_XPT_FE_IB18_CTRL;
        break;
        #endif

        #ifdef BCHP_XPT_FE_IB19_CTRL
        case 19:
        addr = BCHP_XPT_FE_IB19_CTRL;
        break;
        #endif

        default:
        BDBG_ERR(( "%s: unsupported input band %u", __FUNCTION__, BandNum ));
        break;
    }

    return addr;
}

BERR_Code BXPT_GetInputBandConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    BXPT_InputBandConfig *InputBandConfig   /* [in] The current settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if ( !BXPT_P_InputBandIsSupported( BandNum ) )
    {
        BDBG_ERR(( "InputBand %u is not supported on this chip.", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr =  BXPT_P_GetInputBandRegAddr(hXpt, BandNum);
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        InputBandConfig->ClockPolSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_CLOCK_POL_SEL ) ?
            BXPT_Polarity_eActiveLow : BXPT_Polarity_eActiveHigh;

        InputBandConfig->SyncPolSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_SYNC_POL_SEL ) ?
            BXPT_Polarity_eActiveLow : BXPT_Polarity_eActiveHigh;

        InputBandConfig->DataPolSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_DATA_POL_SEL ) ?
            BXPT_Polarity_eActiveLow : BXPT_Polarity_eActiveHigh;

        InputBandConfig->ValidPolSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_VALID_POL_SEL ) ?
            BXPT_Polarity_eActiveLow : BXPT_Polarity_eActiveHigh;

        InputBandConfig->ForceValid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_FORCE_VALID ) ?
            true : false;

        InputBandConfig->SyncDetectEn = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_SYNC_DETECT_EN ) ?
            true : false;

        InputBandConfig->LsbFirst = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_LSB_FIRST ) ?
            true : false;

        InputBandConfig->UseSyncAsValid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_USE_SYNC_AS_VALID ) ?
            true : false;

        InputBandConfig->ErrorPolSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_ERROR_POL_SEL ) ?
            BXPT_Polarity_eActiveLow : BXPT_Polarity_eActiveHigh;

        InputBandConfig->EnableErrorInput = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_ERROR_INPUT_EN ) ?
            true : false;

        InputBandConfig->IbPktLength = ( int ) BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PKT_LENGTH );

        InputBandConfig->ParallelInputSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) ? true : false;
    }

    return( ExitCode );
}

BERR_Code BXPT_SetInputBandConfig(
    BXPT_Handle hXpt,                           /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    const BXPT_InputBandConfig *InputBandConfig /* [in] The new settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned short ParallelInputEn = 0;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if ( !BXPT_P_InputBandIsSupported( BandNum ) )
    {
        BDBG_ERR(( "InputBand %u is not supported on this chip.", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetInputBandRegAddr( hXpt, BandNum );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~(
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_CLOCK_POL_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_SYNC_POL_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_DATA_POL_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_VALID_POL_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_FORCE_VALID ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_SYNC_DETECT_EN ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_LSB_FIRST ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_USE_SYNC_AS_VALID ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_ERROR_POL_SEL ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_ERROR_INPUT_EN ) |
            BCHP_MASK( XPT_FE_IB0_CTRL, IB_PKT_LENGTH )
        );

        if (BandNum==8 || BandNum==10) {
            ParallelInputEn = InputBandConfig->ParallelInputSel == true ? 1 : 0;
        }
        else
        {
            if (InputBandConfig->ParallelInputSel) {
                BDBG_ERR(( "Parallel input is not supported on IB %u", BandNum));
                goto Done;
            }
        }

        /* Now set the new values. */
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, ParallelInputEn ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_CLOCK_POL_SEL,
                InputBandConfig->ClockPolSel == BXPT_Polarity_eActiveLow ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_SYNC_POL_SEL,
                InputBandConfig->SyncPolSel == BXPT_Polarity_eActiveLow ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_DATA_POL_SEL,
                InputBandConfig->DataPolSel == BXPT_Polarity_eActiveLow ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_VALID_POL_SEL,
                InputBandConfig->ValidPolSel == BXPT_Polarity_eActiveLow ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_FORCE_VALID,
                InputBandConfig->ForceValid == true ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_SYNC_DETECT_EN,
                InputBandConfig->SyncDetectEn == true ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_LSB_FIRST,
                InputBandConfig->LsbFirst == true ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_USE_SYNC_AS_VALID,
                InputBandConfig->UseSyncAsValid == true ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_ERROR_POL_SEL,
                InputBandConfig->ErrorPolSel == BXPT_Polarity_eActiveLow ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_ERROR_INPUT_EN,
                InputBandConfig->EnableErrorInput == true ? 1 : 0 ) |

            BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PKT_LENGTH, InputBandConfig->IbPktLength )
        );

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

Done:
    return( ExitCode );
}

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_ParserAllPassMode(
    BXPT_Handle hXpt,   /* [in] Handle for the transport to access. */
    unsigned int ParserNum,                     /* [in] Which input band to access. */
    bool AllPass        /* [in] All-pass enabled if true, or not if false. */
    )
{
    uint32_t ParserReg, ParserRegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %u is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        ParserRegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        ParserReg = BREG_Read32( hXpt->hRegister, ParserRegAddr );
        ParserReg &= ~(
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD )
        );

        /* Also need to update the Primary and Secondary CC checking: in all-pass mode, the will see the multitude of
        ** different IDs as so many CC errors, and drop 'duplicated' PIDs. In all-pass, the PIDs for accepted by parser
        ** X are all mapped to PID channel X.
        */

        /* Now set the new values. */
        if( AllPass && false == hXpt->IsParserInAllPass[ ParserNum ] )
        {
            BXPT_PidChannel_CC_Config AllCcDisabled = { false, false, false, 0 };

            ParserReg |= BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD, 1 );
            /* RS and XC blockouts are set at BXPT_Open() */

            /* Remember the config before entering all-pass. */
            BXPT_GetPidChannel_CC_Config( hXpt, ParserNum, &hXpt->CcConfigBeforeAllPass[ ParserNum ] );

            /* Now disable the CC checks */
            BXPT_SetPidChannel_CC_Config( hXpt, ParserNum, &AllCcDisabled );
            hXpt->IsParserInAllPass[ ParserNum ] = true;
            BREG_Write32( hXpt->hRegister, ParserRegAddr, ParserReg );
        }
        else if( false == AllPass && true == hXpt->IsParserInAllPass[ ParserNum ] )
        {
            /* Restore the config we had before entering all-pass. */
            BXPT_SetPidChannel_CC_Config( hXpt, ParserNum, &hXpt->CcConfigBeforeAllPass[ ParserNum ] );
            hXpt->IsParserInAllPass[ ParserNum ] = false;
            BREG_Write32( hXpt->hRegister, ParserRegAddr, ParserReg );
        }
    }

    return( ExitCode );
}
#endif /* BXPT_FOR_BOOTUPDATER */

BERR_Code BXPT_SetParserDataSource(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ParserNum,             /* [in] Which parser to configure */
    BXPT_DataSource DataSource,     /* [in] The data source. */
    unsigned int WhichSource            /* [in] Which instance of the source */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %u is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    /* Is the requested DataSource valid? */
    else if( DataSource == BXPT_DataSource_eInputBand && !BXPT_P_InputBandIsSupported( WhichSource ) )
    {
        /* Requested an input band we dont support. Complain. */
        BDBG_ERR(( "WhichSource %u is out of range!", WhichSource ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
#ifdef BXPT_NUM_LEGACY_PID_PARSERS
    else if( DataSource == BXPT_DataSource_eInputBand && ParserNum >= BXPT_NUM_LEGACY_PID_PARSERS )
    {
        BDBG_ERR(( "ParserNum %u does not support legacy input!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
#endif
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~(
             BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB ) |
             BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL )
        );

        /* Now set the new values. */
        switch( DataSource )
        {
            /* Values for input band selection start at 0 and are sequential. */
            case BXPT_DataSource_eInputBand:
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB, WhichSource > 15 ? 1 : 0 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL, WhichSource & 0xFF )
            );
            break;

            /* Values for remux feedback selection start at 0x0E and are sequential. */
            case BXPT_DataSource_eRemuxFeedback:
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB, 1 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL, WhichSource )
            );
            break;

            default:
            BDBG_ERR(( "Unsupported DataSource %lu!", ( unsigned long ) DataSource ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            break;
        }

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_GetParserDataSource(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ParserNum,             /* [in] Which parser to configure */
    BXPT_DataSource *DataSource,    /* [out] The data source. */
    unsigned int *WhichSource           /* [out] Which instance of the source */
    )
{
    uint32_t Reg, RegAddr, InputBits;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        InputBits = BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL );

        if( InputBits < 0x10)
        {
            *DataSource = BXPT_DataSource_eInputBand;
            *WhichSource = InputBits;
        }
        else if( InputBits >= 0x10 )
        {
            #if BXPT_HAS_REMUX
                *DataSource = BXPT_DataSource_eRemuxFeedback;
                *WhichSource = InputBits - 0x10;
            #else
                BDBG_ERR(( "DataSource %lu is out of range!", InputBits ));
                ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            #endif
        }
    }

    return( ExitCode );
}
#endif

BERR_Code BXPT_SetParserEnable(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ParserNum,             /* [in] Which parser to configure */
    bool Enable                 /* [in] Parser enabled if true, disabled if false. */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

#ifdef BCHP_PWR_RESOURCE_XPT_PARSER
    unsigned wasEnabled;
#endif

    BDBG_ASSERT( hXpt );

    /* Is the parser number within range? */
    if( ParserNum > hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %u is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#ifdef BCHP_PWR_RESOURCE_XPT_PARSER
        wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE);
        /* only change refcnt if changing state */
        if (!wasEnabled && Enable) {
            BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_PARSER);
        }
#endif

        /* Clear all the bits we are about to change. */
        Reg &= ~( BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE, Enable == true ? 1 : 0 ) );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#ifdef BCHP_PWR_RESOURCE_XPT_PARSER
        if (wasEnabled && !Enable) {
            BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_PARSER);
        }
#endif
    }

    return( ExitCode );
}

#endif

BERR_Code BXPT_EnablePidChannel(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( IsPidDuplicated( hXpt, PidChannelNum ) )
    {
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        BXPT_P_EnablePidChannel( hXpt, PidChannelNum );
    }

    return( ExitCode );
}

BERR_Code BXPT_P_EnablePidChannel(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

#ifdef ENABLE_PLAYBACK_MUX
    /*gain access to the pid table*/
    BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    hXpt->PidChannelTable[ PidChannelNum ].EnableRequested = true;

    if( PidChannelHasDestination( hXpt, PidChannelNum ) )
        SetChannelEnable( hXpt, PidChannelNum, true );

#ifdef ENABLE_PLAYBACK_MUX
    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    return( ExitCode );
}

BERR_Code BXPT_DisablePidChannel(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum          /* [in] Which PID channel. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
#ifdef ENABLE_PLAYBACK_MUX
        /*gain access to the pid table*/
        BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

        hXpt->PidChannelTable[ PidChannelNum ].EnableRequested = false;
/* PR 55511: Need to allow nexus to shut off the PID channel even when there are still consumers attached.
So, don't do the destination check before calling SetChannelEnable().
        if( PidChannelHasDestination( hXpt, PidChannelNum ) == false )     */
            SetChannelEnable( hXpt, PidChannelNum, false );

#ifdef ENABLE_PLAYBACK_MUX
        /*leave pid table protected area*/
        BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
    }

    return( ExitCode );
}

BERR_Code BXPT_AllocPidChannel(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    bool NeedMessageBuffer,     /* [in] Is a message buffer required? */
    unsigned int *PidChannelNum     /* [out] The allocated channel number. */
    )
{
    unsigned int i = 0;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Search for channels with buffers, if thats what they asked for. */
    if( NeedMessageBuffer == true )
    {
        for( i = 0; i < hXpt->MaxPidChannels; i++ )
        {
            if( hXpt->PidChannelTable[ i ].IsAllocated == false
            && hXpt->PidChannelTable[ i ].HasMessageBuffer ==  true )
            {
                hXpt->PidChannelTable[ i ].IsAllocated = true;
                break;
            }
        }
    }
    else
    {
        /* Otherwise, grab the first free channel we find. */
        for( i= 0; i < hXpt->MaxPidChannels; i++ )
        {
            if( hXpt->PidChannelTable[ i ].IsAllocated == false )
            {
                hXpt->PidChannelTable[ i ].IsAllocated = true;
                break;
            }
        }
    }

    *PidChannelNum = i;

    return( ExitCode );
}

BERR_Code BXPT_ConfigurePidChannel(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to configure. */
    unsigned int Pid,               /* [in] PID to use. */
    unsigned int Band               /* [in] The parser band to use. */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

#if BXPT_HAS_TSMUX
    unsigned int BXPT_PB_P_GetPbBandId( BXPT_Handle hXpt, unsigned int Band );
#endif

    BDBG_ASSERT( hXpt );
    BDBG_MSG(( "BXPT_ConfigurePidChannel: PidChannel %u, PID 0x%04X, %s parser %u",
               PidChannelNum, Pid, BXPT_P_IS_PB( Band ) ? "Playback" : "Live", Band & 0x7FFF));

    if( Pid >= 0x2000 )
    {
        /* Bad PID. Complain. */
        BDBG_ERR(( "Pid %lu is out of range!", ( unsigned long ) Pid ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
#if BXPT_HAS_IB_PID_PARSERS
        unsigned FeSel = 0;
        unsigned OldFeSel = 255;
        unsigned OldPid = 0x2000;
        unsigned OldBand = 0x2000;     /* Never have this many parser bands. */

        PidChannelTableEntry *Entry = &hXpt->PidChannelTable[ PidChannelNum ];

#ifdef ENABLE_PLAYBACK_MUX
        /*gain access to the pid table*/
        BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

        Entry->Pid = Pid;
        Entry->Band = Band;
        Entry->IsAllocated = true;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        if( BXPT_P_IS_PB( Band ) )
        {
            BXPT_P_CLEAR_PB_FLAG( Band );   /* Remove the PB parser flag. */
            FeSel = 1;
        }

#if BXPT_HAS_TSMUX
    /* Band ID in playback may have been remapped. */
    if( FeSel )
    {
        unsigned OldBand = Band;

        BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_eMcpb);
        Band = BXPT_PB_P_GetPbBandId( hXpt, Band );
        BXPT_P_ReleaseSubmodule(hXpt, BXPT_P_Submodule_eMcpb);
        BDBG_MSG(( "Remapping PID channel %u PB %u to PB %u", (unsigned) PidChannelNum, OldBand, Band ));
    }
#endif

#if BXPT_SW7425_1323_WORKAROUND
        Entry->IsPb = FeSel;
#endif

        OldFeSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL );
        OldBand = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );

#if BXPT_HAS_DIRECTV_SUPPORT
        /* Need to preserve the HD filter bits, if enabled */
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) )
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID, Pid )
            );
        }
        else
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
            );
        }
#else
        OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_PID );

        Reg &= ~(
            BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID )
        );

        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, FeSel ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID, Pid )
        );
#endif

        /* Write to the PID table ONLY if the configuration has changed. */
        if( OldFeSel != FeSel || OldBand != Band || OldPid != Pid )
        {
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }

        Entry->IsPidChannelConfigured = true;
#ifdef ENABLE_PLAYBACK_MUX
        /*leave pid table protected area*/
        BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

#else /* BXPT_HAS_IB_PID_PARSERS */
        PidChannelTableEntry *Entry = &hXpt->PidChannelTable[ PidChannelNum ];
        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        Reg &= ~(
            BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1 ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, 1 ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
        );

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_SW7425_1323_WORKAROUND
        Entry->IsPb = true;
#endif
        Entry->IsPidChannelConfigured = true;
#endif /* BXPT_HAS_IB_PID_PARSERS */
    }

    return( ExitCode );
}

#ifdef ENABLE_PLAYBACK_MUX

BERR_Code BXPT_SetPidChannelSwitched(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool IsSwitched
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    /*gain access to the pid table*/
    BKNI_EnterCriticalSection();

    ExitCode = BXPT_SetPidChannelSwitched_isr( hXpt, PidChannelNum, IsSwitched );

    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();

    return ExitCode;
}

BERR_Code BXPT_SetPidChannelSwitched_isr(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool IsSwitched
    )
{
    uint32_t Reg, RegAddr;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* Set the PID channels enable bit. */
        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        if(IsSwitched)
        {
            /*ignore pid version so our transport buffers aren't flushed*/
            Reg |= BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, IGNORE_PID_VERSION, 1 );
        }
        else
        {
            /*look at pid versions to behave like normal*/
            Reg &= ~BCHP_MASK( XPT_FE_PID_TABLE_i, IGNORE_PID_VERSION );
        }

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return(ExitCode);
}

bool BXPT_SwitchPidChannel(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr;

    bool result = true;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        return(false);
    }
    else if(!hXpt->PidChannelTable[PidChannelNum].IsAllocated)
    {
        return(false);
    }

    /*gain access to the pid table*/
    BKNI_EnterCriticalSection();

    /*set whether the muxed pid is enabled or not*/
    hXpt->PidChannelTable[PidChannelNum].MuxEnable = EnableIt;

#if BXPT_SW7425_1323_WORKAROUND
    hXpt->PidChannelTable[PidChannelNum].IsEnabled = EnableIt;
#endif

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE );

    /*check if we're supposed to enable the channel*/
    if(EnableIt)
    {
        /*need to enable the channel*/
        if(!hXpt->PidChannelTable[PidChannelNum].EnableRequested)
        {
            result = false;
        }
        else if(!PidChannelHasDestination( hXpt, PidChannelNum ))
        {
            result = false;
        }
        else
        {
            Reg |= BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1 );
        }
    }

    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();

    return(result);
}

bool BXPT_SwitchPidChannelISR(
    BXPT_Handle hXpt,                   /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr;
    bool result = true;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        return(false);
    }
    else if(!hXpt->PidChannelTable[PidChannelNum].IsAllocated)
    {
        return(false);
    }

    /*set whether the muxed pid is enabled or not*/
    hXpt->PidChannelTable[PidChannelNum].MuxEnable = EnableIt;

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE );

#if BXPT_SW7425_1323_WORKAROUND
    hXpt->PidChannelTable[PidChannelNum].IsEnabled = EnableIt;
#endif

    /*check if we're supposed to enable the channel*/
    if(EnableIt)
    {
        /*need to enable the channel*/
        if(!hXpt->PidChannelTable[PidChannelNum].EnableRequested)
        {
            result = false;
        }
        else if(!PidChannelHasDestination( hXpt, PidChannelNum ))
        {
            result = false;
        }
        else
        {
            Reg |= BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1 );
        }
    }

    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return(result);
}

BERR_Code BXPT_SetNumPlaybackMux(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int Num                /* [in] Number of playback mux blocks*/
    )
{
    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. The 'Num' argument is 1-based, not 0-based like everything else. */
    if( Num > BXPT_NUM_PLAYBACKS )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "Num %u is out of range!", Num ));
        return(BERR_INVALID_PARAMETER);
    }

    hXpt->NumPlaybackMuxes = Num;

    return(BERR_SUCCESS);
}

#endif /*ENABLE_PLAYBACK_MUX*/

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_GetPidChannelConfig(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get config for. */
    unsigned int *Pid,              /* [out] The PID its using. */
    unsigned int *Band,             /* [out] The parser band the channel is mapped to. */
    bool *IsPlayback                /* [out] true if band is a playback parser, false if input */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Pid );
    BDBG_ASSERT( Band );

    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Reg, RegAddr;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        *IsPlayback = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) ? true : false;
        *Band = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );

        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) )
        {
            *Pid = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID );
        }
        else
        {
            *Pid = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID );
        }
    }

    return ExitCode;
}
#endif /* BXPT_FOR_BOOTUPDATER */

BERR_Code BXPT_FreePidChannel(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int PidChannelNum      /* [in] PID channel to free up. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* Must be done BEFORE disabling the PID channel. */
        BXPT_P_ClearAllPidChannelDestinations( hXpt, PidChannelNum );

        BXPT_DisablePidChannel( hXpt, PidChannelNum );
        hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = false;
        hXpt->PidChannelTable[ PidChannelNum ].Pid = 0x2000;
        hXpt->PidChannelTable[ PidChannelNum ].IsPidChannelConfigured = false;
#ifdef ENABLE_PLAYBACK_MUX
        hXpt->PidChannelTable[ PidChannelNum ].MuxEnable = false;
        hXpt->PidChannelTable[ PidChannelNum ].HasDestination = false;
#endif /*ENABLE_PLAYBACK_MUX*/
    }

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
void BXPT_FreeAllPidChannels(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    unsigned int i;

    BDBG_ASSERT( hXpt );

    for( i= 0; i < hXpt->MaxPidChannels; i++ )
    {
        /* Must be done BEFORE disabling the PID channel. */
        BXPT_P_ClearAllPidChannelDestinations( hXpt, i );

        BXPT_DisablePidChannel( hXpt, i );
        hXpt->PidChannelTable[ i ].IsAllocated = false;
        hXpt->PidChannelTable[ i ].Pid = 0x2000;
        hXpt->PidChannelTable[ i ].IsPidChannelConfigured = false;
#ifdef ENABLE_PLAYBACK_MUX
        hXpt->PidChannelTable[ i ].MuxEnable = false;
        hXpt->PidChannelTable[ i ].HasDestination = false;
#endif /*ENABLE_PLAYBACK_MUX*/
    }
}

BERR_Code BXPT_RequestPidChannel(
    BXPT_Handle hXpt,       /* [Input] Handle for this transport */
    unsigned int PidChannelNum  /* [Output] The channel number the user wants. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        if( hXpt->PidChannelTable[ PidChannelNum ].IsAllocated == false )
            hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = true;
        else
            ExitCode = BXPT_ERR_PID_ALREADY_ALLOCATED;
    }

    return( ExitCode );
}
#endif

void BXPT_P_ResetTransport(
    BREG_Handle hReg
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hReg );

    BKNI_EnterCriticalSection();

    /* Assert the transport core reset */
    BREG_Write32( hReg, BCHP_SUN_TOP_CTRL_SW_INIT_0_SET,
                             BCHP_SUN_TOP_CTRL_SW_INIT_0_SET_xpt_sw_init_MASK );

    /* Now clear the reset. */
    BREG_Write32( hReg, BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR,
                             BCHP_SUN_TOP_CTRL_SW_INIT_0_CLEAR_xpt_sw_init_MASK );

    Reg = BREG_Read32( hReg, BCHP_SUN_TOP_CTRL_FINAL_SW_INIT_0_MONITOR );
    if( BCHP_GET_FIELD_DATA( Reg, SUN_TOP_CTRL_FINAL_SW_INIT_0_MONITOR, xpt_sw_init ) )
    {
        BDBG_ERR(( "XPT did not leave init state!" ));
    }

    BKNI_LeaveCriticalSection();
}

void SetChannelEnable(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr;

#ifdef ENABLE_PLAYBACK_MUX
    if(EnableIt && hXpt->PidChannelTable[PidChannelNum].Band >
        BXPT_PB_PARSER((BXPT_NUM_PLAYBACKS - hXpt->NumPlaybackMuxes)))
    {
        return;
    }

    /* If the channel is enabled for playback muxing, don't disable it here. BXPT_SwitchPidChannel() will do it instead. */
    if( hXpt->PidChannelTable[PidChannelNum].MuxEnable )
    {
        return;
    }
#endif /*ENABLE_PLAYBACK_MUX*/

#if BXPT_SW7425_1323_WORKAROUND
    hXpt->PidChannelTable[PidChannelNum].IsEnabled = EnableIt;
#endif

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

    /* if the pid channel is already enabled, and request is to enable it again
       then make sure that we do not write the PID table register, this will prevent
       PID version from changing to a new value hence the buffers will not be flushed
       by the XPT HW.This step is imortant to get a glitch free av when record.
     */
    if((Reg & BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE )) && EnableIt)
        return;

    Reg &= ~BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE );
    if( EnableIt )
    {
        Reg |= BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1 );
    }

    BREG_Write32( hXpt->hRegister, RegAddr, Reg );
}

bool PidChannelHasDestination(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum
    )
{
    uint32_t Reg, RegAddr;
    bool HasDestination = false;

#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    RegAddr = BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

    if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT ) )
        HasDestination = true;
#endif

    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

    if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, PID_DESTINATION ) )
        HasDestination = true;

    return HasDestination;
}

BERR_Code BXPT_P_ClearAllPidChannelDestinations(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

#ifdef ENABLE_PLAYBACK_MUX
    BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    /* Disable PID channel */
    SetChannelEnable( hXpt, PidChannelNum, false );

    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_DESTINATION );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_MULTICHANNEL_PLAYBACK
    RegAddr = BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );
#endif

#ifdef ENABLE_PLAYBACK_MUX
    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
    return ExitCode;
}

static BERR_Code BXPT_P_SetPidChannelDestinationExtension(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum,
    unsigned SelectedDestination,
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr, CurrentDestinations;

    BERR_Code ExitCode = BERR_SUCCESS;

    SelectedDestination -= 8;
    RegAddr = BCHP_XPT_FE_SPID_EXT_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    CurrentDestinations = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT );

    if( EnableIt )
    {
        CurrentDestinations |= ( 1ul << SelectedDestination );  /* Enable the pipe */
        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT, CurrentDestinations );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        /* TODO: Enable PID channel if it was requested */
        if( hXpt->PidChannelTable[ PidChannelNum ].EnableRequested == true )
            SetChannelEnable( hXpt, PidChannelNum, true );
    }
    else
    {
        CurrentDestinations &= ~( 1ul << SelectedDestination ); /* Clear pipe enables */

        /* Disable PID channel if there are no other destinations selected */
        if( !CurrentDestinations )
            SetChannelEnable( hXpt, PidChannelNum, false );

        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_EXT_TABLE_i, PID_DESTINATION_EXT, CurrentDestinations );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return ExitCode;
}

BERR_Code BXPT_P_SetPidChannelDestination(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum,
    unsigned SelectedDestination,
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr, CurrentDestinations;

    BERR_Code ExitCode = BERR_SUCCESS;

    if( SelectedDestination > 7 )
        return BXPT_P_SetPidChannelDestinationExtension( hXpt, PidChannelNum, SelectedDestination, EnableIt );

    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    CurrentDestinations = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, PID_DESTINATION );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_DESTINATION );

    if( EnableIt )
    {
        CurrentDestinations |= ( 1ul << SelectedDestination );  /* Enable the pipe */
        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, PID_DESTINATION, CurrentDestinations );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        /* TODO: Enable PID channel if it was requested */
        if( hXpt->PidChannelTable[ PidChannelNum ].EnableRequested == true )
            SetChannelEnable( hXpt, PidChannelNum, true );
    }
    else
    {
        CurrentDestinations &= ~( 1ul << SelectedDestination ); /* Clear pipe enables */

        /* Disable PID channel if there are no other destinations selected */
        if( !CurrentDestinations )
            SetChannelEnable( hXpt, PidChannelNum, false );

        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, PID_DESTINATION, CurrentDestinations );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    return ExitCode;
}

bool IsPidDuplicated(
    BXPT_Handle hXpt,           /* [Input] Handle for this transport */
    unsigned int PidChannelNum
    )
{
    unsigned int i, TargetChannelCfg, ThisChannelCfg, ThisChannelEnable;
    uint32_t RegAddr, Reg;

    bool IsDuplicated = false;

#ifdef ENABLE_PLAYBACK_MUX
    if(hXpt->PidChannelTable[PidChannelNum].Band >
        BXPT_PB_PARSER((BXPT_NUM_PLAYBACKS - hXpt->NumPlaybackMuxes)))
    {
        /*there are allowed to be duplicated pids on the muxed band*/
        return(false);
    }

    /*gain access to the pid table*/
    BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
    TargetChannelCfg = BREG_Read32( hXpt->hRegister, RegAddr );

    /* This mask covers all the bitfields we need to compare. */
    TargetChannelCfg &= (
        BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
    );

    for( i = 0; i < hXpt->MaxPidChannels; i++ )
    {
        /* Skip the legitimate channel assignment. */
        if( i == PidChannelNum )
            continue;

        if( hXpt->PidChannelTable[ i ].IsPidChannelConfigured == true )
        {
            RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( i * PID_CHNL_STEPSIZE );
            ThisChannelCfg = Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            ThisChannelCfg &= (
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
            );

            /* We need to know if the channel has been enabled or has just been *requested* to be enabled */
            ThisChannelEnable = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE ) || hXpt->PidChannelTable[ i ].EnableRequested == true;

            if( TargetChannelCfg == ThisChannelCfg && ThisChannelEnable )
            {
                BDBG_ERR(( "Pid channels %u and %u are duplicates!", i, PidChannelNum ));
                IsDuplicated = true;
                break;
            }
        }
    }

#ifdef ENABLE_PLAYBACK_MUX
    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    return( IsDuplicated );
}

static void BXPT_P_ConfigArbiter(BREG_Handle hReg)
{
    /* Nothing specific needed yet. */
    BSTD_UNUSED( hReg );
}

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_GetPidChannel_CC_Config(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get config for. */
    BXPT_PidChannel_CC_Config *Cfg
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Cfg );

    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* All-pass mode overrides the user settings, so return the last ones they gave before all-pass was enabled */
        unsigned Band = hXpt->PidChannelTable[ PidChannelNum ].Band;
        bool IsAllPass = false;
        struct BXPT_P_PbHandle *hPb = NULL;

        if( BXPT_P_IS_PB(Band)==false ) {
            IsAllPass = hXpt->IsParserInAllPass[ Band ];
        }
        else {
            hPb = &hXpt->PlaybackHandles[ BXPT_P_GET_PB_BAND_NUM(Band) ];
            IsAllPass = hPb->IsParserInAllPass;
        }

        if( IsAllPass ) {
            if( BXPT_P_IS_PB(Band)==false ) {
                *Cfg = hXpt->CcConfigBeforeAllPass[ PidChannelNum ];
            }
            else {
                *Cfg = hPb->CcConfigBeforeAllPass;
            }
        }
        else
        {
            uint32_t Reg, RegAddr;

            RegAddr = BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            Cfg->Primary_CC_CheckEnable = BCHP_GET_FIELD_DATA( Reg, XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PCC_ERROR_EN ) ? true : false;
            Cfg->Secondary_CC_CheckEnable = BCHP_GET_FIELD_DATA( Reg, XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, SCC_ERROR_EN ) ? true : false;
            Cfg->Generate_CC_Enable = BCHP_GET_FIELD_DATA( Reg, XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PROC_CC ) ? true : false;

            RegAddr = BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            Cfg->Initial_CC_Value = BCHP_GET_FIELD_DATA( Reg, XPT_FULL_PID_PARSER_STATE_CONFIG_3_i, PKT_INSERTED );
        }
    }

    return ExitCode;
}
#endif /* BXPT_FOR_BOOTUPDATER */

BERR_Code BXPT_SetPidChannel_CC_Config(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get config for. */
    const BXPT_PidChannel_CC_Config *Cfg
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Cfg );

    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* All-pass must override the new settings, so store them until all-pass is disabled */
        unsigned Band = hXpt->PidChannelTable[ PidChannelNum ].Band;
        bool IsAllPass = false;
        struct BXPT_P_PbHandle *hPb = NULL;

        if( BXPT_P_IS_PB(Band)==false ) {
            IsAllPass = hXpt->IsParserInAllPass[ Band ];
        }
        else {
            hPb = &hXpt->PlaybackHandles[ BXPT_P_GET_PB_BAND_NUM(Band) ];
            IsAllPass = hPb->IsParserInAllPass;
        }

        if( IsAllPass ) {
            if( BXPT_P_IS_PB(Band)==false ) {
                hXpt->CcConfigBeforeAllPass[ PidChannelNum ] = *Cfg;
            }
            else {
                hPb->CcConfigBeforeAllPass = *Cfg;
            }
        }
        else
        {
            uint32_t Reg, RegAddr;

            RegAddr = BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            Reg &= ~(
                BCHP_MASK( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PCC_ERROR_EN ) |
                BCHP_MASK( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, SCC_ERROR_EN ) |
                BCHP_MASK( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PROC_CC )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PCC_ERROR_EN, Cfg->Primary_CC_CheckEnable ? 1 : 0 ) |
                BCHP_FIELD_DATA( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, SCC_ERROR_EN, Cfg->Secondary_CC_CheckEnable ? 1 : 0 ) |
                BCHP_FIELD_DATA( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PROC_CC, Cfg->Generate_CC_Enable ? 1 : 0 )
            );
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

            RegAddr = BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_3_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            Reg &= ~(
                BCHP_MASK( XPT_FULL_PID_PARSER_STATE_CONFIG_3_i, PKT_INSERTED )
            );
            Reg |= (
                BCHP_FIELD_DATA( XPT_FULL_PID_PARSER_STATE_CONFIG_3_i, PKT_INSERTED, Cfg->Initial_CC_Value )
            );
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }
    }

    return ExitCode;
}

#if BXPT_HAS_PARSER_REMAPPING
#if (!B_REFSW_MINIMAL)
void BXPT_GetParserMapping(
    BXPT_Handle hXpt,           /* [in] Handle for the Transport. */
    BXPT_ParserBandMapping *ParserMap
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ParserMap );

    *ParserMap = hXpt->BandMap;

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID );
    ParserMap->FrontEnd[ 0 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_BAND_ID );
    ParserMap->FrontEnd[ 0 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 1 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_BAND_ID );
    ParserMap->FrontEnd[ 1 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 2 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_BAND_ID );
    ParserMap->FrontEnd[ 2 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 3 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_BAND_ID );
    ParserMap->FrontEnd[ 3 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_PARSER_SEL ) ? true : false;

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID );
    ParserMap->FrontEnd[ 4 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_BAND_ID );
    ParserMap->FrontEnd[ 4 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 5 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_BAND_ID );
    ParserMap->FrontEnd[ 5 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 6 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_BAND_ID );
    ParserMap->FrontEnd[ 6 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 7 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_BAND_ID );
    ParserMap->FrontEnd[ 7 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_PARSER_SEL ) ? true : false;

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID );
    ParserMap->FrontEnd[ 4 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_BAND_ID );
    ParserMap->FrontEnd[ 4 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 5 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_BAND_ID );
    ParserMap->FrontEnd[ 5 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 6 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_BAND_ID );
    ParserMap->FrontEnd[ 6 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 7 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_BAND_ID );
    ParserMap->FrontEnd[ 7 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_PARSER_SEL ) ? true : false;

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID );
    ParserMap->FrontEnd[ 8 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_BAND_ID );
    ParserMap->FrontEnd[ 8 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 9 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_BAND_ID );
    ParserMap->FrontEnd[ 9 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_PARSER_SEL ) ? true : false;

#if BXPT_NUM_REMAPPABLE_FE_PARSERS > 10
    ParserMap->FrontEnd[ 10 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_BAND_ID );
    ParserMap->FrontEnd[ 10 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 11 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_BAND_ID );
    ParserMap->FrontEnd[ 11 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_PARSER_SEL ) ? true : false;
#endif

#if BXPT_NUM_REMAPPABLE_FE_PARSERS > 12
    ParserMap->FrontEnd[ 12 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER12_BAND_ID );
    ParserMap->FrontEnd[ 13 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER13_PARSER_SEL ) ? true : false;
    ParserMap->FrontEnd[ 14 ].VirtualParserBandNum =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER14_BAND_ID );
    ParserMap->FrontEnd[ 15 ].VirtualParserIsPlayback =
        BCHP_GET_FIELD_DATA( Reg, XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER15_PARSER_SEL ) ? true : false;
#endif
    /* ToDo: Add playback support. */
}
#endif

BERR_Code BXPT_SetParserMapping(
    BXPT_Handle hXpt,           /* [in] Handle for the Transport. */
    const BXPT_ParserBandMapping *ParserMap
    )
{
    uint32_t Reg;
    unsigned ii;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ParserMap );

    /* Only certain parsers can be remapped. Check for attempted remapping of parsers that don't support it. */
#if BXPT_NUM_REMAPPABLE_FE_PARSERS < BXPT_NUM_PID_PARSERS
    for( ii = BXPT_NUM_REMAPPABLE_FE_PARSERS; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        const BXPT_BandMap *Mapping;

        Mapping = &ParserMap->FrontEnd[ ii ];
        if( Mapping->VirtualParserBandNum != ii || Mapping->VirtualParserIsPlayback )
        {
            BDBG_ERR(( "ParserNum %u cannot be remapped", ii ));
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
    }
#else
    BSTD_UNUSED( ii );
#endif

    hXpt->BandMap = *ParserMap;

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_PARSER_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_BAND_ID, ParserMap->FrontEnd[ 0 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_PARSER_SEL, ParserMap->FrontEnd[ 0 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_BAND_ID, ParserMap->FrontEnd[ 1 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_PARSER_SEL, ParserMap->FrontEnd[ 1 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_BAND_ID, ParserMap->FrontEnd[ 2 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_PARSER_SEL, ParserMap->FrontEnd[ 2 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_BAND_ID, ParserMap->FrontEnd[ 3 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_PARSER_SEL, ParserMap->FrontEnd[ 3 ].VirtualParserIsPlayback ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, Reg );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_PARSER_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_BAND_ID, ParserMap->FrontEnd[ 4 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER4_PARSER_SEL, ParserMap->FrontEnd[ 4 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_BAND_ID, ParserMap->FrontEnd[ 5 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER5_PARSER_SEL, ParserMap->FrontEnd[ 5 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_BAND_ID, ParserMap->FrontEnd[ 6 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER6_PARSER_SEL, ParserMap->FrontEnd[ 6 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_BAND_ID, ParserMap->FrontEnd[ 7 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, PARSER7_PARSER_SEL, ParserMap->FrontEnd[ 7 ].VirtualParserIsPlayback ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER4_TO_PARSER7_BAND_ID, Reg );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_PARSER_SEL )

#if BXPT_NUM_REMAPPABLE_FE_PARSERS > 10
        |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_PARSER_SEL )
#endif
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_BAND_ID, ParserMap->FrontEnd[ 8 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER8_PARSER_SEL, ParserMap->FrontEnd[ 8 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_BAND_ID, ParserMap->FrontEnd[ 9 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER9_PARSER_SEL, ParserMap->FrontEnd[ 9 ].VirtualParserIsPlayback ? 1 : 0 )
#if BXPT_NUM_REMAPPABLE_FE_PARSERS > 10
        |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_BAND_ID, ParserMap->FrontEnd[ 10 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER10_PARSER_SEL, ParserMap->FrontEnd[ 10 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_BAND_ID, ParserMap->FrontEnd[ 11 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, PARSER11_PARSER_SEL, ParserMap->FrontEnd[ 11 ].VirtualParserIsPlayback ? 1 : 0 )
#endif
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER8_TO_PARSER11_BAND_ID, Reg );

#if BXPT_NUM_REMAPPABLE_FE_PARSERS > 12
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER12_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER12_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER13_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER13_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER14_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER14_PARSER_SEL ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER15_BAND_ID ) |
        BCHP_MASK( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER15_PARSER_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER12_BAND_ID, ParserMap->FrontEnd[ 12 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER12_PARSER_SEL, ParserMap->FrontEnd[ 12 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER13_BAND_ID, ParserMap->FrontEnd[ 13 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER13_PARSER_SEL, ParserMap->FrontEnd[ 13 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER14_BAND_ID, ParserMap->FrontEnd[ 14 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER14_PARSER_SEL, ParserMap->FrontEnd[ 14 ].VirtualParserIsPlayback ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER15_BAND_ID, ParserMap->FrontEnd[ 15 ].VirtualParserBandNum ) |
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, PARSER15_PARSER_SEL, ParserMap->FrontEnd[ 15 ].VirtualParserIsPlayback ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_MINI_PID_PARSER12_TO_PARSER15_BAND_ID, Reg );
#endif

    return BERR_SUCCESS;
}

#endif


uint32_t BXPT_P_GetParserCtrlRegAddr(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned ParserNum,
    unsigned Reg0
    )
{
#if BXPT_HAS_PARSER_REMAPPING
    {
        if( BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 == Reg0 || BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 == Reg0 )
        {
            unsigned ii;

            for (ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++)
            {
                BXPT_BandMap *pMap = &hXpt->BandMap.FrontEnd[ ii ];

                if( pMap->VirtualParserBandNum == ParserNum && !pMap->VirtualParserIsPlayback )
                {
                    return Reg0 + ( ii * PARSER_REG_STEPSIZE );
                }
            }

            /* Had to be an invalid band number to get this far. */
            BDBG_ERR(( "Invalid virtual parser band number" ));
            return 0;
        }
        else
        {
            /* Read or write of addr 0 will probably cause a segfault, but we were passed an unsupported Reg0 anyway. */
            BDBG_ERR(( "Parser reg %u is unsupported", Reg0 ));
            return 0;
        }
    }
#else
    BSTD_UNUSED( hXpt );
    return Reg0 + ( ParserNum * PARSER_REG_STEPSIZE );
#endif
}

static unsigned int GetParserIndex(
    BXPT_Handle hXpt,                               /* [in] Handle for the transport to access. */
    unsigned ParserNum )
{
    unsigned ii;

#if BXPT_HAS_PARSER_REMAPPING
    for (ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++)
    {
        BXPT_BandMap *pMap = &hXpt->BandMap.FrontEnd[ ii ];

        if( pMap->VirtualParserBandNum == ParserNum && !pMap->VirtualParserIsPlayback )
        {
            break;
        }
    }

    if (ii == BXPT_NUM_PID_PARSERS)
            BDBG_ERR(( "Invalid virtual parser band number" ));
#else
    BSTD_UNUSED( hXpt );
    ii = ParserNum;
#endif
    return ii;
}

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_CheckPipelineErrors(
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    )
{
    BERR_Code status = BERR_SUCCESS;

#if BXPT_HAS_FIXED_RSBUF_CONFIG
    status |= BXPT_P_RsBuf_ReportOverflows( hXpt, Errors );
#endif
#if BXPT_HAS_FIXED_XCBUF_CONFIG
    status |= BXPT_P_XcBuf_ReportOverflows( hXpt, Errors );
#endif

    return status ? BXPT_ERR_DATA_PIPELINE : status;
}
#endif

#if BXPT_HAS_FIXED_RSBUF_CONFIG
BERR_Code BXPT_IsDataPresent(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get status for. */
    bool *IsDataPresent
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( IsDataPresent );

    if( PidChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        *IsDataPresent = false;
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Reg, RegAddr, WaterMark;
        unsigned IsPlayback, ParserBand;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        IsPlayback = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL );
        ParserBand = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );

        if (IsPlayback)
        {
            WaterMark = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_WATERMARK_PBP0 + ParserBand * RS_BUFFER_PTR_REG_STEPSIZE );
            BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_WATERMARK_PBP0 + ParserBand * RS_BUFFER_PTR_REG_STEPSIZE, 0x0 );
        }
        else
        {
            WaterMark = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_WATERMARK_IBP0 + ParserBand * RS_BUFFER_PTR_REG_STEPSIZE );
            BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_WATERMARK_IBP0 + ParserBand * RS_BUFFER_PTR_REG_STEPSIZE, 0x0 );
        }
        *IsDataPresent = WaterMark > 0;
    }

    return ExitCode;
}
#endif

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_GetLiveLTSID(
    BXPT_Handle hXpt,
    unsigned parserNum,
    unsigned *ltsid
    )
{
    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ltsid );

    if( parserNum >= BXPT_NUM_PID_PARSERS )
    {
        return BERR_INVALID_PARAMETER;
    }

#if BXPT_HAS_PARSER_REMAPPING
    {
        BXPT_BandMap map;

        /* See SW7425-5193 for the band-to-LTSID mapping. */
        map = hXpt->BandMap.FrontEnd[ parserNum ];
        *ltsid = map.VirtualParserIsPlayback ? map.VirtualParserBandNum + 0x40 : map.VirtualParserBandNum;
    }
#else
    *ltsid = parserNum;
#endif

    return BERR_SUCCESS;
}
#endif

#if BXPT_HAS_MTSIF
bool BXPT_IsMtsifDecryptionEnabled(
    BXPT_Handle hXpt,
    unsigned channelNo
    )
{
#ifdef BCHP_XPT_FE_MTSIF_RX0_CTRL1_MTSIF_RX_DECRYPTION_ENABLE_STATUS_MASK
    uint32_t RegAddr, Reg;

    RegAddr = BCHP_XPT_FE_MTSIF_RX0_CTRL1 + channelNo * MTSIF_STEPSIZE;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    return BCHP_GET_FIELD_DATA( Reg, XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_DECRYPTION_ENABLE_STATUS ) ? true : false;
#else
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( channelNo );
    return false;
#endif
}
#endif

#if (!B_REFSW_MINIMAL)
void BXPT_ResetAtsCounter(
    BXPT_Handle hXpt
    )
{
    uint32_t atsCtrlReg = 0;

    BDBG_ASSERT( hXpt );

    atsCtrlReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, COUNT_RESET, 1 );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL, atsCtrlReg );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, COUNT_RESET, 0 );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL, atsCtrlReg );
}

void BXPT_SetAtsInternal(
    BXPT_Handle hXpt
    )
{
    uint32_t atsCtrlReg = 0;

    BDBG_ASSERT( hXpt );

    atsCtrlReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, EXT_RESET_ENABLE, 0 );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, INC_MUX_SEL, 0 );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL, atsCtrlReg );
}

void BXPT_SetAtsExternal(
    BXPT_Handle hXpt
    )
{
    uint32_t atsCtrlReg = 0;

    BDBG_ASSERT( hXpt );

    atsCtrlReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, EXT_RESET_ENABLE, 1 );
    BCHP_SET_FIELD_DATA( atsCtrlReg, XPT_FE_ATS_COUNTER_CTRL, INC_MUX_SEL, 1 );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_ATS_COUNTER_CTRL, atsCtrlReg );
}
#endif

#if (!B_REFSW_MINIMAL)
uint32_t BXPT_GetBinaryAts(
    BXPT_Handle hXpt
    )
{
    BDBG_ASSERT( hXpt );

    return BREG_Read32( hXpt->hRegister, BCHP_XPT_FE_ATS_TS_BINARY );
}

void BXPT_SetBinaryAts(
    BXPT_Handle hXpt,
    uint32_t newAts
    )
{
    BDBG_ASSERT( hXpt );

    BREG_Write32( hXpt->hRegister, BCHP_XPT_FE_ATS_TS_BINARY, newAts );
}
#endif

/* end of file */
