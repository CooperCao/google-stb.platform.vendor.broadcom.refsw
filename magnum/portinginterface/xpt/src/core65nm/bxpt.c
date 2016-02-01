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
#endif

#if BXPT_HAS_RSBUF
#include "bchp_xpt_rsbuff.h"
#include "bxpt_rsbuf.h"
#endif

#if BXPT_HAS_XCBUF
#include "bchp_xpt_xcbuff.h"
#include "bxpt_xcbuf.h"
#endif

#include "bmem.h"
#include "bxpt_rave.h"
#include "bxpt_pcr_offset.h"
#include "bxpt_pcr.h"

#include "bchp_sun_top_ctrl.h"
#include "bchp_xpt_fe.h"

#if BXPT_HAS_MESG_BUFFERS
    #include "bchp_xpt_msg.h"
#endif

#include "bchp_xpt_pb0.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_rave.h"

#if BXPT_HAS_PACKETSUB
    #include "bchp_xpt_psub.h"
    #include "bxpt_packetsub.h"
#endif

#if BXPT_HAS_REMUX
#include "bxpt_remux.h"
#endif


#if (BCHP_CHIP == 7408)
    /* Workaround for HW7408-91. */
    #include "bchp_xpt_pcroffset.h"
#endif

#if( BCHP_CHIP == 7405 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336 ) || \
    ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7550 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || \
    ( BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 35230 ) || ( BCHP_CHIP == 7420 ) || ( BCHP_CHIP == 35125 )
#include "bchp_xpt_xmemif.h"
#endif

#if ( BCHP_CHIP == 3563 )
    /* 3563 has only 1 playback. */
    #define PB_PARSER_REG_STEPSIZE  ( 0x44 )
#else
    /* Everybody else has at least 2 */
    #include "bchp_xpt_pb1.h"
    #define PB_PARSER_REG_STEPSIZE  ( BCHP_XPT_PB1_CTRL1 - BCHP_XPT_PB0_CTRL1 )
#endif

#ifdef BCHP_XPT_PSUB_PSUB1_CTRL0
#define PACKET_SUB_REGISTER_STEP    ( BCHP_XPT_PSUB_PSUB1_CTRL0 - BCHP_XPT_PSUB_PSUB0_CTRL0 )
#else
#define PACKET_SUB_REGISTER_STEP    ( BCHP_XPT_PSUB_PSUB0_STAT2 - BCHP_XPT_PSUB_PSUB0_CTRL0 )
#endif

/* Distance between Item X regs and Item X+1 */
#define PARSER_REG_STEPSIZE     ( BCHP_XPT_FE_PARSER1_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1 )
#define IB_REG_STEPSIZE         ( 8 )
#define PID_CHNL_STEPSIZE       ( 4 )
#define SPID_CHNL_STEPSIZE      ( 4 )

/* Locally defined interrupt IDs. The RDB structure makes it impossible to generate these automatically at the moment. */
#define BCHP_INT_ID_XPT_MSG_INTR_FLAG         BCHP_INT_ID_CREATE( BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31, BCHP_XPT_MSG_BUF_DAT_RDY_INTR_00_31_INTR_FLAG_SHIFT )
#define BCHP_INT_ID_XPT_MSG_OVFL_INTR_FLAG    BCHP_INT_ID_CREATE( BCHP_XPT_MSG_BUF_OVFL_INTR_00_31, BCHP_XPT_MSG_BUF_OVFL_INTR_00_31_INTR_FLAG_SHIFT )
#define MESG_FILTER_ARRAY_ELEMENTS  ( 512 )

#include "bdbg.h"
BDBG_MODULE( xpt );
BDBG_OBJECT_ID(bxpt_t);

static void SetChannelEnable( BXPT_Handle hXpt, unsigned int PidChannelNum, bool EnableIt );
static bool PidChannelHasDestination( BXPT_Handle hXpt, unsigned int PidChannelNum );
static bool IsPidDuplicated( BXPT_Handle hXpt, unsigned int PidChannelNum );
static void BXPT_P_ConfigArbiter(BREG_Handle hReg);

BERR_Code BXPT_GetDefaultSettings(
    BXPT_DefaultSettings *Defaults, /* [out] Defaults to use during init.*/
    BCHP_Handle hChip               /* [in] Handle to used chip. */
    )
{
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( Defaults );
    BSTD_UNUSED( hChip );

    BKNI_Memset( Defaults, 0, sizeof( *Defaults) );

#if BXPT_HAS_IB_PID_PARSERS
    for( ii = 0; ii < BXPT_P_MAX_PID_PARSERS; ii++ )
        Defaults->DramBuffers.IbParserRsSize[ ii ] = 200;
#endif

    for( ii = 0; ii < BXPT_P_MAX_PLAYBACKS; ii++ )
        Defaults->DramBuffers.PbParserRsSize[ ii ] = 8;

#if BXPT_HAS_XCBUF
    for( ii = 0; ii < BXPT_P_MAX_PID_PARSERS; ii++ )
        Defaults->DramBuffers.RaveXcCfg.IbParserXcSize[ ii ] = 200;

    for( ii = 0; ii < BXPT_P_MAX_PLAYBACKS; ii++ )
        Defaults->DramBuffers.RaveXcCfg.PbParserXcSize[ ii ] = 8;

    for( ii = 0; ii < BXPT_P_MAX_PID_PARSERS; ii++ )
        Defaults->DramBuffers.MesgBufXcCfg.IbParserXcSize[ ii ] = 8;

    for( ii = 0; ii < BXPT_P_MAX_PLAYBACKS; ii++ )
        Defaults->DramBuffers.MesgBufXcCfg.PbParserXcSize[ ii ] = 8;

    #if !( BCHP_CHIP == 7440 )
        for( ii = 0; ii < BXPT_P_MAX_REMULTIPLEXORS; ii++ )
        {
            Defaults->DramBuffers.RemuxXcCfg[ ii ].BandAXcSize = 200;
            Defaults->DramBuffers.RemuxXcCfg[ ii ].BandBXcSize = 200;
        }
    #endif
#endif

#if ( BCHP_CHIP == 3563 )
    Defaults->EnableMpodOut = false;
#endif

#if 0   /* Deprecated by conversion to BMMA */
    Defaults->hRHeap = NULL;
    Defaults->hPbHeap = NULL;
#endif
    Defaults->MesgDataOnRPipe = false;

    return( ExitCode );
}

BERR_Code BXPT_Open(
    BXPT_Handle *hXpt,                      /* [out] Transport handle. */
    BCHP_Handle hChip,                      /* [in] Handle to used chip. */
    BREG_Handle hRegister,                  /* [in] Handle to access regiters. */
    BMMA_Heap_Handle hMemory,               /* [in] Handle to memory heap to use. */
    BINT_Handle hInt,                       /* [in] Handle to interrupt interface to use. */
    const BXPT_DefaultSettings *Defaults    /* [in] Defaults to use during init.*/
    )
{
    BXPT_Handle lhXpt;
    unsigned i;

    uint32_t Reg = 0;
    uint32_t RegAddr = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ENTER( BXPT_Open );

    /* Sanity check on the handles we've been given. */
    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( hChip );
    BDBG_ASSERT( hRegister );
    BDBG_ASSERT( hMemory );
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
    BDBG_OBJECT_SET(lhXpt, bxpt_t);

    lhXpt->hChip = hChip;
    lhXpt->hRegister = hRegister;
    lhXpt->hMemory = (BMEM_Heap_Handle) hMemory;
    lhXpt->hInt = hInt;

    lhXpt->mmaHeap = hMemory;
    lhXpt->mmaRHeap = Defaults->mmaRHeap;
    lhXpt->hMemory = Defaults->memHeap;
    lhXpt->hRHeap = Defaults->memRHeap;
#if 0 /* deprecated as part of MMA conversion */
    lhXpt->hPbHeap = Defaults->hPbHeap;
#endif
    lhXpt->MesgDataOnRPipe = Defaults->MesgDataOnRPipe;

#ifdef BCHP_PWR_RESOURCE_XPT
    /* turn on the 108M and 216M for BXPT_Open */
    BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_XPT_108M);
    BCHP_PWR_AcquireResource(hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);
#endif

    /* Reset the hardware. */
    BXPT_P_ResetTransport( hRegister );

    /* set arbiter settings for 7401, remove it when CFE sets these values for 7401 */
#if ( BCHP_CHIP == 7401 ) || ( BCHP_CHIP == 7440 ) || ( BCHP_CHIP == 7400 ) || ( BCHP_CHIP == 3563 ) \
    || ( BCHP_CHIP == 35230 ) || ( BCHP_CHIP == 7420 ) || (BCHP_CHIP == 7340) || (  BCHP_CHIP == 7342 ) || ( BCHP_CHIP == 35125 )
    BXPT_P_ConfigArbiter( hRegister );
#else
    BSTD_UNUSED( BXPT_P_ConfigArbiter );
#endif

    /* Set the number of resources this transport has. */
    lhXpt->MaxPlaybacks = BXPT_P_MAX_PLAYBACKS;
    lhXpt->MaxPidChannels = BXPT_P_MAX_PID_CHANNELS;
#if BXPT_HAS_IB_PID_PARSERS
    lhXpt->MaxPidParsers = BXPT_P_MAX_PID_PARSERS;
    lhXpt->MaxInputBands = BXPT_P_MAX_INPUT_BANDS;
#endif
    lhXpt->MaxTpitPids = BXPT_P_MAX_TPIT_PIDS;
#if BXPT_HAS_MESG_BUFFERS
    lhXpt->MaxFilterBanks = BXPT_P_MAX_FILTER_BANKS;
    lhXpt->MaxFiltersPerBank = BXPT_P_MAX_FILTERS_PER_BANK;
#endif
#if BXPT_HAS_PACKETSUB
    lhXpt->MaxPacketSubs = BXPT_P_MAX_PACKETSUBS;
#endif
#if BXPT_HAS_DPCRS
    lhXpt->MaxPcrs = BXPT_P_MAX_PCRS;
#endif
    lhXpt->MaxRaveContexts = BXPT_P_MAX_RAVE_CONTEXTS;

#ifdef ENABLE_PLAYBACK_MUX
    /* By default, use one playback block for muxing. */
    lhXpt->NumPlaybackMuxes = 1;
#endif

    Reg = BREG_Read32( hRegister, BCHP_XPT_FE_IB_SYNC_DETECT_CTRL );
    BCHP_SET_FIELD_DATA( Reg, XPT_FE_IB_SYNC_DETECT_CTRL, IB_SYNC_IN_CNT, Defaults->syncInCount );
    BCHP_SET_FIELD_DATA( Reg, XPT_FE_IB_SYNC_DETECT_CTRL, IB_SYNC_OUT_CNT, Defaults->syncOutCount );
    BREG_Write32( hRegister, BCHP_XPT_FE_IB_SYNC_DETECT_CTRL, Reg );

    /* Create and init the PID channel table. */
    for( i = 0; i < lhXpt->MaxPidChannels; i++ )
    {
        /*
        ** On some devices, not all PID channels have a message buffer. HasMessageBuffer
        ** will be updated below, when we init the message buffer table
        */
#ifdef ENABLE_PLAYBACK_MUX
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, false, false, 0 };
#else /*ENABLE_PLAYBACK_MUX*/
        PidChannelTableEntry InitEntry = { false, false, false, 0, 0, false, 0 };
#endif /*ENABLE_PLAYBACK_MUX*/

        lhXpt->PidChannelTable[ i ] = InitEntry;

        BREG_Write32( hRegister, BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( 4 * i ), 1 << 28 );
        BREG_Write32( hRegister, BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
    }

#if BXPT_HAS_IB_PID_PARSERS
    /* Init the PID parsers. */
    for( i = 0; i < lhXpt->MaxPidParsers; i++ )
    {
        #if (BCHP_CHIP == 7342)
        BREG_Write32( hRegister, BCHP_XPT_FE_PARSER0_CTRL2 + BXPT_P_GetParserRegOffset(i), 0 );
        #else
        BREG_Write32( hRegister, BCHP_XPT_FE_PARSER0_CTRL2 + ( PARSER_REG_STEPSIZE * i ), 0 );
        #endif
    }
#endif

#if BXPT_HAS_MESG_BUFFERS
    /* Create and init the message buffer table. */
    for( i = 0; i < BXPT_P_MAX_MESG_BUFFERS; i++ )
    {
        MessageBufferEntry InitEntry = { false, 0, 0 };
        lhXpt->MessageBufferTable[ i ] = InitEntry;

        lhXpt->MesgBufferIsInitialized[ i ] = false;

        lhXpt->PidChannelTable[ i ].HasMessageBuffer = true;
        lhXpt->PidChannelTable[ i ].MessageBuffercount = 0;

        lhXpt->MesgIntrCallbacks[ i ].Callback = ( BINT_CallbackFunc ) NULL;
        lhXpt->OverflowIntrCallbacks[ i ].Callback = ( BINT_CallbackFunc ) NULL;

#if BXPT_HAS_PID2BUF_MAPPING
        BREG_Write32( hRegister, BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
#else
        BREG_Write32( hRegister, BCHP_XPT_MSG_PID_CTRL1_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_PID_CTRL2_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
#endif
        BREG_Write32( hRegister, BCHP_XPT_MSG_DMA_BP_TABLE_i_ARRAY_BASE + ( 4 * i ), 0 );
        BREG_Write32( hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( 4 * i ), 0 );

        /* for normal legacy mode set false, set true to override settings */
        lhXpt->PidChannelParserConfigOverride[i] = false;
    }

    lhXpt->Pid2BuffMappingOn = false;

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
#endif

#if BXPT_HAS_IB_PID_PARSERS
    /*
    ** All parser bands initially do NOT modify PSI messages in the DMA buffers.
    ** This is the hardware default.
    */
    for( i = 0; i < BXPT_P_MAX_PID_PARSERS; i++ )
    {
#if BXPT_HAS_MESG_BUFFERS
        ParserConfig ParserInit = { false, false, false, BXPT_PsiMesgModModes_eNoMod };

        lhXpt->IbParserTable[ i ] = ParserInit;
#endif

        /* Map all PIDs to the RAVE based on PID channel number. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL2 + BXPT_P_GetParserRegOffset(i);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL2 + ( i * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( lhXpt->hRegister, RegAddr );
        Reg &= ~( BCHP_MASK( XPT_FE_PARSER0_CTRL2, RAVE_CX_MODE ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL2, RAVE_CX_MODE, 1 ));
        BREG_Write32( lhXpt->hRegister, RegAddr, Reg );
    }
#endif

    /* Each playback has a hard-wired PID parser. So init the table for those too. */
    for( i = 0; i < BXPT_P_MAX_PLAYBACKS; i++ )
    {
        ParserConfig ParserInit = { false, false, false, BXPT_PsiMesgModModes_eNoMod };

        lhXpt->PbParserTable[ i ] = ParserInit;

        /* Map all PIDs to the RAVE based on PID channel number. */
        RegAddr = BCHP_XPT_PB0_PARSER_CTRL2 + ( i * PB_PARSER_REG_STEPSIZE );
        Reg = BREG_Read32( lhXpt->hRegister, RegAddr );
        Reg &= ~( BCHP_MASK( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_PB0_PARSER_CTRL2, RAVE_CX_MODE, 1 ));
        BREG_Write32( lhXpt->hRegister, RegAddr, Reg );
    }

    /* Init the RAVE structure in the xpt handle. */
    for( i = 0; i < BXPT_P_MAX_RAVE_CHANNELS; i++ )
    {
        lhXpt->RaveChannels[ i ].Allocated = false;
    }

    /* Init the RAVE structure in the xpt handle. */
    for( i = 0; i < BXPT_P_MAX_PCR_OFFSET_CHANNELS; i++ )
    {
        lhXpt->PcrOffsets[ i ].Handle = NULL;
        lhXpt->PcrOffsets[ i ].Allocated = false;
    }

#if BCHP_CHIP == 7400 && BCHP_VER == BCHP_VER_A0 && BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
    /* 7400A0 is the oddball. It does NOT follow the chip-level endianess signal. Everyone else does. */
    Reg = BREG_Read32( hRegister, BCHP_XPT_MSG_MSG_CTRL1 );
    Reg |= BCHP_FIELD_DATA( XPT_MSG_MSG_CTRL1, ENDIAN_CTRL, 1 );
    BREG_Write32( hRegister, BCHP_XPT_MSG_MSG_CTRL1, Reg );
#endif

#if BXPT_HAS_MESG_BUFFERS
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

#if BXPT_HAS_RSBUF
    ExitCode = BERR_TRACE( BXPT_P_RsBuf_Init( lhXpt, &Defaults->DramBuffers ) );
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("Rate smoothing buffer init FAILED!"));
        goto done;
    }
#endif

#if BXPT_HAS_XCBUF
    ExitCode = BERR_TRACE( BXPT_P_XcBuf_Init( lhXpt, &Defaults->DramBuffers ) );
    if( ExitCode != BERR_SUCCESS )
    {
        BDBG_ERR(("XPT client buffer init FAILED!"));
        goto done;
    }
#endif

#if ( BCHP_CHIP == 3563 )
    Reg = BREG_Read32( lhXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0 );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_MISC_CTRL0, RMXP_MPOD_MUX_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_MISC_CTRL0, RMXP_MPOD_MUX_SEL, Defaults->EnableMpodOut == true ? 1 : 0 ));
    BREG_Write32( lhXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0, Reg );
#endif

#if ( BCHP_CHIP == 35230 ) || ( BCHP_CHIP == 35125 )
/* This bit isn't in the RDB */
{
    uint32_t reg = BREG_Read32( lhXpt->hRegister, 0x00770000 + 0x00000048 );
    BREG_Write32( lhXpt->hRegister, 0x00770000 + 0x00000048, reg & 0xFFFDFFFF );
}
#endif

#if BXPT_DPCR_GLOBAL_PACKET_PROC_CTRL
    {
        unsigned Index;

        for( Index = 0; Index < BXPT_P_MAX_PCRS; Index++ )
        {
            lhXpt->JitterTimestamp[ Index ] = BXPT_PCR_JitterTimestampMode_eAuto;
            lhXpt->PbJitterDisable[ Index ] = BXPT_PCR_JitterCorrection_eAuto;
            lhXpt->LiveJitterDisable[ Index ] = BXPT_PCR_JitterCorrection_eAuto;
        }
    }
#endif

#ifndef BXPT_FOR_BOOTUPDATER
    BXPT_P_PcrOffset_ModuleInit( lhXpt );
#endif

    done:
    *hXpt = lhXpt;
    BDBG_LEAVE( BXPT_Open );

#ifdef BCHP_PWR_RESOURCE_XPT
    /* turn off only the 216M */
    BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_XPT_XMEMIF);

    /* a failed BXPT_Open powers down the 108M as well */
    if (ExitCode!=BERR_SUCCESS) {
        BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_XPT_108M);
    }
#endif

    return( ExitCode );
}

void BXPT_Close(
    BXPT_Handle hXpt        /* [in] Handle for the Transport to be closed. */
    )
{
    unsigned int Index;

    BERR_Code Res;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

#if BXPT_HAS_IB_PID_PARSERS
    for( Index = 0; Index < hXpt->MaxPidParsers; Index++ )
        BXPT_SetParserEnable( hXpt, Index, false );
#endif

#if BXPT_HAS_RSBUF
    Res = BXPT_P_RsBuf_Shutdown( hXpt );
    BDBG_ASSERT( Res == BERR_SUCCESS );
#endif

#if BXPT_HAS_XCBUF
    Res = BXPT_P_XcBuf_Shutdown( hXpt );
    BDBG_ASSERT( Res == BERR_SUCCESS );
#endif

    for( Index = 0; Index < BXPT_P_MAX_RAVE_CHANNELS; Index++ )
        if( hXpt->RaveChannels[ Index ].Allocated == true )
        {
            Res = BXPT_Rave_CloseChannel( ( BXPT_Rave_Handle ) hXpt->RaveChannels[ Index ].Handle );
            BDBG_ASSERT( Res == BERR_SUCCESS );
        }

    for( Index = 0; Index < BXPT_P_MAX_PCR_OFFSET_CHANNELS; Index++ )
        if( hXpt->PcrOffsets[ Index ].Allocated == true )
        {
            Res = BXPT_PcrOffset_Close( ( BXPT_PcrOffset_Handle ) hXpt->PcrOffsets[ Index ].Handle );
            BDBG_ASSERT( Res == BERR_SUCCESS );
        }

    for ( Index = 0; Index < BXPT_P_MAX_PLAYBACKS; Index++ ) {
        if ( hXpt->PlaybackHandles[Index].Opened == true ) {
            BXPT_Playback_CloseChannel( (BXPT_Playback_Handle) &hXpt->PlaybackHandles[Index] );
        }
    }

#if BXPT_HAS_PACKETSUB
    for ( Index = 0; Index < BXPT_P_MAX_PACKETSUBS; Index++ ) {
        if ( hXpt->PacketSubHandles[Index].Opened == true ) {
            BXPT_PacketSub_CloseChannel( (BXPT_PacketSub_Handle) &hXpt->PacketSubHandles[Index]);
        }
    }
#endif

#if BXPT_HAS_REMUX
    for ( Index = 0; Index < BXPT_P_MAX_REMULTIPLEXORS; Index++ ) {
        if ( hXpt->RemuxHandles[Index].Opened == true ) {
            BXPT_Remux_CloseChannel( (BXPT_Remux_Handle) &hXpt->RemuxHandles[Index]);
        }
    }
#endif

#if BXPT_HAS_MESG_BUFFERS
    /*
    ** Free any message buffers we've allocated. If the user has allocated any, its
    ** the their responsibility to free them.
    */
    for( Index = 0; Index < BXPT_P_MAX_MESG_BUFFERS; Index++ )
    {
        MessageBufferEntry *Buffer = &hXpt->MessageBufferTable[ Index ];

        /* Free the old buffer, if there is one. */
        if( Buffer->IsAllocated == true )
        {
            if( Buffer->Address )
                BMEM_Free( hXpt->hMemory, ( void * ) Buffer->Address );
            Buffer->IsAllocated = false;
        }
    }

    BINT_DestroyCallback(hXpt->hMsgCb);
    BINT_DestroyCallback(hXpt->hMsgOverflowCb);
#endif

    /* Reset the core, thus stopping any unwanted interrupts. */
    BXPT_P_ResetTransport( hXpt->hRegister );
    /* set arbiter settings for 7401, remove it when CFE sets these values for 7401 */
#if ( BCHP_CHIP == 7401 )
    BXPT_P_ConfigArbiter( hXpt->hRegister );
#endif

#ifdef BCHP_PWR_RESOURCE_XPT
    /* release the 108M after BXPT_P_ResetTransport */
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
#endif

    /* Dont need the transport handle any more. */
    BDBG_OBJECT_DESTROY(hXpt, bxpt_t);
    BKNI_Free( hXpt );
}

void BXPT_GetDefaultStandbySettings(
    BXPT_StandbySettings *pSettings
    )
{
    BSTD_UNUSED(pSettings);
}

BERR_Code BXPT_Standby(
    BXPT_Handle hXpt,
    BXPT_StandbySettings *pSettings
    )
{
#ifdef BCHP_PWR_RESOURCE_XPT
    unsigned int Index;
#endif
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BSTD_UNUSED(pSettings);

#ifdef BCHP_PWR_RESOURCE_XPT
    if (hXpt->bStandby) {
        BDBG_WRN(("Already in standby"));
        return BERR_SUCCESS;
    }

    /* check if XPT is still in use. if so, we cannot enter standby */

#if BXPT_HAS_IB_PID_PARSERS
    for( Index = 0; Index < hXpt->MaxPidParsers; Index++ ) {
            uint32_t Reg, RegAddr;
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(Index);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( Index * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if (BCHP_GET_FIELD_DATA(Reg, XPT_FE_PARSER0_CTRL1, PARSER_ENABLE)) {
            BDBG_ERR(("One or more parsers still enabled. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

    for ( Index = 0; Index < BXPT_P_MAX_PLAYBACKS; Index++ ) {
        if ( hXpt->PlaybackHandles[Index].Running == true ) {
            BDBG_ERR(("One or more playback channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }

#if BXPT_HAS_PACKETSUB
    for ( Index = 0; Index < BXPT_P_MAX_PACKETSUBS; Index++ ) {
        if ( hXpt->PacketSubHandles[Index].Running == true ) {
            BDBG_ERR(("One or more packetsub channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

#if BXPT_HAS_REMUX
    for ( Index = 0; Index < BXPT_P_MAX_REMULTIPLEXORS; Index++ ) {
        if ( hXpt->RemuxHandles[Index].Running == true ) {
            BDBG_ERR(("One or more remux channels still running. Cannot enter standby"));
            return BERR_UNKNOWN;
        }
    }
#endif

    /* if we get to this point, then XPT is not in use, and all clocks except the 108M
       were dynamically turned off earlier. XPT can now release the 108M */
    BCHP_PWR_ReleaseResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
    hXpt->bStandby = true;
#endif /* #ifdef BCHP_PWR_RESOURCE_XPT */

    return BERR_SUCCESS;
}

BERR_Code BXPT_Resume(
    BXPT_Handle hXpt
    )
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

#ifdef BCHP_PWR_RESOURCE_XPT
    if (!hXpt->bStandby) {
        BDBG_WRN(("Not in standby"));
        return BERR_SUCCESS;
    }
    BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_108M);
    hXpt->bStandby = false;
#endif /* #ifdef BCHP_PWR_RESOURCE_XPT */

    return BERR_SUCCESS;
}

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
    Capability->MaxPlaybacks = BXPT_P_MAX_PLAYBACKS;
    Capability->MaxPidChannels = BXPT_P_MAX_PID_CHANNELS;
#if BXPT_HAS_IB_PID_PARSERS
    Capability->MaxPidParsers = BXPT_P_MAX_PID_PARSERS;
    Capability->MaxInputBands = BXPT_P_MAX_INPUT_BANDS;
#endif
    Capability->MaxTpitPids = BXPT_P_MAX_TPIT_PIDS;
#if BXPT_HAS_MESG_BUFFERS
    Capability->MaxFilterBanks = BXPT_P_MAX_FILTER_BANKS;
    Capability->MaxFiltersPerBank = BXPT_P_MAX_FILTERS_PER_BANK;
#endif
#if BXPT_HAS_PACKETSUB
    Capability->MaxPacketSubs = BXPT_P_MAX_PACKETSUBS;
#endif
#if BXPT_HAS_DPCRS
    Capability->MaxPcrs = BXPT_P_MAX_PCRS;
#endif
    Capability->MaxRaveContexts = BXPT_P_MAX_RAVE_CONTEXTS;
}


#if BXPT_HAS_IB_PID_PARSERS

#if (BCHP_CHIP == 7342)
int BXPT_P_GetParserRegOffset(int parserIndex)
{
    switch (parserIndex) {
    case 0:
        return(BCHP_XPT_FE_PARSER0_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    case 1:
        return(BCHP_XPT_FE_PARSER1_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    case 2:
        return(BCHP_XPT_FE_PARSER2_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    case 3:
        return(BCHP_XPT_FE_PARSER3_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    case 4:
        return(BCHP_XPT_FE_PARSER4_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    case 5:
        return(BCHP_XPT_FE_PARSER5_CTRL1 - BCHP_XPT_FE_PARSER0_CTRL1);
    default:
        BDBG_ERR(("Parser# %d does not exist in RDB", parserIndex));
        return 0;
    }
}
#endif

BERR_Code BXPT_GetParserConfig(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    BXPT_ParserConfig *ParserConfig /* [out] The current settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Timebase = 0;

        /* The parser config registers are at consecutive addresses. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 +  BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        ParserConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE );
        ParserConfig->ContCountIgnore = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE );
        ParserConfig->AcceptNulls = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT );
        ParserConfig->AcceptAdapt00 = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_ADAPT_00 );

        /* The location of these bits moved from the 7038. So we store flags in the handle. */
        ParserConfig->SaveShortPsiMsg = hXpt->IbParserTable[ ParserNum ].SaveShortPsiMsg;
        ParserConfig->SaveLongPsiMsg = hXpt->IbParserTable[ ParserNum ].SaveLongPsiMsg;
        ParserConfig->PsfCrcDis = hXpt->IbParserTable[ ParserNum ].PsfCrcDis;
        ParserConfig->PsiMod = hXpt->IbParserTable[ ParserNum ].PsiMsgMod;

        ParserConfig->TsMode = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE );

        Timebase = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_TIMEBASE_SEL );
        if( Timebase == 0 )
            ParserConfig->UsePcrTimeBase    = false;
        else if( Timebase > BXPT_P_MAX_PCRS )
        {
            BDBG_ERR(( "Invalid timebase %u configured in hardware.", ( unsigned long ) Timebase ));
            ParserConfig->UsePcrTimeBase    = false;
        }
        else
        {
            ParserConfig->UsePcrTimeBase = true;
            ParserConfig->WhichPcrToUse = Timebase - 1;
        }
    }

    return( ExitCode );
}

BERR_Code BXPT_GetDefaultParserConfig(
    BXPT_Handle hXpt,               /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    BXPT_ParserConfig *ParserConfig /* [out] The current settings */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        ParserConfig->ErrorInputIgnore = false;
        ParserConfig->ContCountIgnore = false;
        ParserConfig->AcceptNulls = false;
        ParserConfig->AcceptAdapt00 = false;

        ParserConfig->UsePcrTimeBase = false;
        ParserConfig->WhichPcrToUse = 0;

        /* The location of these bits moved from the 7038. So we store flags in the handle. */
        ParserConfig->SaveShortPsiMsg = false;
        ParserConfig->SaveLongPsiMsg = false;
        ParserConfig->PsfCrcDis = false;
        ParserConfig->PsiMod = BXPT_PsiMesgModModes_eNoMod;
        ParserConfig->TsMode = BXPT_ParserTimestampMode_eAutoSelect;
    }

    return( ExitCode );
}

BERR_Code BXPT_SetParserConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int ParserNum,             /* [in] Which parser band to access. */
    const BXPT_ParserConfig *ParserConfig   /* [in] The new settings */
    )
{
    uint32_t Reg, RegAddr, TimeBaseSel;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( ParserConfig );

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~(
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE ) |
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE ) |
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE ) |
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_TIMEBASE_SEL ) |
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT ) |
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_ADAPT_00 )
        );

        /* Now set the new values. */
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE, ParserConfig->ErrorInputIgnore ) |
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_CONT_COUNT_IGNORE, ParserConfig->ContCountIgnore ) |
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE, ParserConfig->TsMode ) |
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT, ParserConfig->AcceptNulls ) |
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ACCEPT_ADAPT_00, ParserConfig->AcceptAdapt00 )
        );

        /* Store the ContCountIgnore setting only if we're in MPEG mode. */
        if( !BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE ) )
        {
            hXpt->InputParserContCountIgnore[ ParserNum ] = ParserConfig->ContCountIgnore;
        }

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
            BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_TIMEBASE_SEL, TimeBaseSel )
        );

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_MESG_BUFFERS
        hXpt->IbParserTable[ ParserNum ].SaveShortPsiMsg = ParserConfig->SaveShortPsiMsg;
        hXpt->IbParserTable[ ParserNum ].SaveLongPsiMsg = ParserConfig->SaveLongPsiMsg;
        hXpt->IbParserTable[ ParserNum ].PsfCrcDis = ParserConfig->PsfCrcDis;
        hXpt->IbParserTable[ ParserNum ].PsiMsgMod = ParserConfig->PsiMod;

        /*
        ** The PSI settings above are now done in the message filter block. Make sure all the
        ** filters get the new settings.
        */
        BXPT_P_ApplyParserPsiSettings( hXpt, ParserNum, false );
#endif
    }

    return( ExitCode );
}

BERR_Code BXPT_GetDefaultInputBandConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    BXPT_InputBandConfig *InputBandConfig   /* [in] The current settings */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if( BandNum >= hXpt->MaxInputBands )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "BandNum %lu is out of range!", BandNum ));
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

        InputBandConfig->ParallelInputSel = false;
#if ( BCHP_CHIP == 7325 )
    if ( BandNum == 2 )
        InputBandConfig->ParallelInputSel = true;
#elif ( BCHP_CHIP == 7340 )
    if (( BandNum == 2 ) || ( BandNum == 3 ))
        InputBandConfig->ParallelInputSel = true;
#elif ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )
    if (( BandNum == 2 ) || ( BandNum == 3 ) || ( BandNum == 5 ))
        InputBandConfig->ParallelInputSel = true;
#elif ( BCHP_CHIP == 7342 )
    if ( BandNum == 4 )
        InputBandConfig->ParallelInputSel = true;
#endif
    }

    return( ExitCode );
}

static bool GetParallelSetting(
    unsigned int BandNum,
    uint32_t Reg
    )
{
#if ( BCHP_CHIP == 7325 )
        if ( BandNum == 2 || BandNum == 4 )
            return BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) ? true : false;
        else
            return false;
#elif ( BCHP_CHIP == 7340 )
        if (( BandNum == 2 ) || ( BandNum == 3 ))
            return BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) ? true : false;
        else
            return false;
#elif ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )
        if (( BandNum >= 2 ) || ( BandNum <= 5 ))
            return BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) ? true : false;
        else
            return false;
#elif ( BCHP_CHIP == 7342 )
        BSTD_UNUSED( Reg );
        if( BandNum == 4 )  /* Only band 4 has parallel input support. */
            return true;    /* Parallel is forced on by hardware */
        else
            return false;
#else
        if( BandNum == 4 )  /* Only band 4 has parallel input support. */
            return BCHP_GET_FIELD_DATA( Reg, XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) ? true : false;
        else
            return false;
#endif
}

bool BXPT_P_InputBandIsSupported(
    unsigned ib
    )
{
    if( ib < BXPT_P_MAX_INPUT_BANDS )
        return true;
    else
        return false;
}

BERR_Code BXPT_GetInputBandConfig(
    BXPT_Handle hXpt,                       /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    BXPT_InputBandConfig *InputBandConfig   /* [in] The current settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if( BandNum >= hXpt->MaxInputBands )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "BandNum %lu is out of range!", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr =  BCHP_XPT_FE_IB0_CTRL + ( BandNum * IB_REG_STEPSIZE );
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

        InputBandConfig->ParallelInputSel = GetParallelSetting( BandNum, Reg );
    }

    return( ExitCode );
}

static void SetParallelMode(
    unsigned int BandNum,
    uint32_t *Reg,
    const BXPT_InputBandConfig *IbCfg
    )
{
#if ( BCHP_CHIP != 7342 )
    *Reg &= ~( BCHP_MASK( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL ) );
#endif

#if ( BCHP_CHIP == 7342 )
    if ( 4 == BandNum )
    {
        BDBG_ASSERT(IbCfg->ParallelInputSel); /* BandNum does not support serial mode */
    }
    else
    {
        BDBG_ASSERT(!IbCfg->ParallelInputSel); /* BandNum does not support parallel mode */
    }
#elif ( BCHP_CHIP == 7325 )
    if ( 2 == BandNum || 4 == BandNum )
    {
        *Reg |= BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, IbCfg->ParallelInputSel == true ? 1 : 0 );
    }
    else
    {
        BDBG_ASSERT(!IbCfg->ParallelInputSel); /* BandNum does not support parallel mode */
    }
#elif ( BCHP_CHIP == 7340 )
    if ( BandNum >= 2 || BandNum <= 4 )
    {
        *Reg |= BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, IbCfg->ParallelInputSel == true ? 1 : 0 );
    }
    else
    {
        BDBG_ASSERT(!IbCfg->ParallelInputSel); /* BandNum does not support parallel mode */
    }
#elif ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )
    if (( 2 == BandNum ) || ( 3 == BandNum ) || ( 5 == BandNum ))
    {
        *Reg |= BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, IbCfg->ParallelInputSel == true ? 1 : 0 );
    }
    else
    {
        BDBG_ASSERT(!IbCfg->ParallelInputSel); /* BandNum does not support parallel mode */
    }
#else
    if( BandNum == 4 )  /* Only band 4 has parallel input support. */
    {
        *Reg |= BCHP_FIELD_DATA( XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, IbCfg->ParallelInputSel == true ? 1 : 0 );
    }
    else
    {
        BDBG_ASSERT(!IbCfg->ParallelInputSel); /* BandNum does not support parallel mode */
    }
#endif
}

BERR_Code BXPT_SetInputBandConfig(
    BXPT_Handle hXpt,                           /* [in] Handle for the transport to access. */
    unsigned int BandNum,                       /* [in] Which input band to access. */
    const BXPT_InputBandConfig *InputBandConfig /* [in] The new settings */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( InputBandConfig );

    /* Is the parser number within range? */
    if( BandNum >= hXpt->MaxInputBands )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "BandNum %lu is out of range!", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr =  BCHP_XPT_FE_IB0_CTRL + ( BandNum * IB_REG_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~(
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

        SetParallelMode( BandNum, &Reg, InputBandConfig );

        /* Now set the new values. */
        Reg |= (
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

    return( ExitCode );
}

BERR_Code BXPT_ParserAllPassMode(
    BXPT_Handle hXpt,   /* [in] Handle for the transport to access. */
    unsigned int ParserNum,                     /* [in] Which input band to access. */
    bool AllPass        /* [in] All-pass enabled if true, or not if false. */
    )
{
    uint32_t Reg, RegAddr,PacketLen;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE)== 1)
            PacketLen = 130;
        else
            PacketLen = 188;

        /* Clear all the bits we are about to change. */
        Reg &= ~(
            BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL )
        );

        /* Now set the new values. */
        if(AllPass)
        {
            Reg |= BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL, 1 );

#if BXPT_HAS_RSBUF && BXPT_HAS_XCBUF
            /* Save the old blockout values */
            hXpt->RsBufBO[ ParserNum ] = BXPT_P_RsBuf_GetBlockout( hXpt, ParserNum );
            hXpt->XcBufBO[ BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum ] = BXPT_P_XcBuf_GetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum );

            BXPT_P_RsBuf_SetBlockout( hXpt, ParserNum, BXPT_P_RsBuf_ComputeBlockOut( 81000000, PacketLen ) );
            BXPT_P_XcBuf_SetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum, BXPT_P_RsBuf_ComputeBlockOut( 81000000, PacketLen ) );

#if ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) ||  ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7342 )\
    || ( BCHP_CHIP == 7340 )
            BXPT_P_XcBuf_SetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum, BXPT_P_RsBuf_ComputeBlockOut( 81000000, PacketLen ) );
#endif

            BDBG_WRN(("***************************************************************************************"));
            BDBG_WRN(("* WARNING: The max bitrate for the all-pass parser band has been increased to 81 Mbps. "));
            BDBG_WRN(("* Check your usage mode.Software will decrease this when all pass mode is disabled. "));
            BDBG_WRN(("***************************************************************************************"));
        }
        else
        {
            /* Adjust Rave data read BW */
            BXPT_P_RsBuf_SetBlockout( hXpt, ParserNum, hXpt->RsBufBO[ ParserNum ] );
            BXPT_P_XcBuf_SetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum, hXpt->XcBufBO[ BXPT_XcBuf_Id_RAVE_IBP0 + ParserNum ] );
#endif
        }

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    }

    return( ExitCode );
}

BERR_Code BXPT_SetParserDataSource(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ParserNum,             /* [in] Which parser to configure */
    BXPT_DataSource DataSource,     /* [in] The data source. */
    unsigned int WhichSource            /* [in] Which instance of the source */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    /* Is the requested DataSource valid? */
    else if( DataSource == BXPT_DataSource_eInputBand && WhichSource >= hXpt->MaxInputBands )
    {
        /* Requested an input band we dont support. Complain. */
        BDBG_ERR(( "WhichSource %lu is out of range!. Not enough input bands.", WhichSource ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        /* Clear all the bits we are about to change. */
        Reg &= ~( BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_INPUT_SEL ) );

        /* Now set the new values. */
        switch( DataSource )
        {
            /* Values for input band selection start at 0 and are sequential. */
            case BXPT_DataSource_eInputBand:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_INPUT_SEL, WhichSource ) );
            break;

            /* Values for remux feedback selection start at 0x0E and are sequential. */
            case BXPT_DataSource_eRemuxFeedback:
            Reg |= ( BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_INPUT_SEL, 0x0E + WhichSource ) );
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

BERR_Code BXPT_GetParserDataSource(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int ParserNum,             /* [in] Which parser to configure */
    BXPT_DataSource *DataSource,    /* [out] The data source. */
    unsigned int *WhichSource           /* [out] Which instance of the source */
    )
{
    uint32_t Reg, RegAddr, InputBits;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        InputBits = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_INPUT_SEL );

        switch( InputBits )
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            *DataSource = BXPT_DataSource_eInputBand;
            *WhichSource = InputBits;
            break;

            /* Remux feedback paths start at 0x0E in the RDB */
            case 0x0E:
            case 0x0F:
            *DataSource = BXPT_DataSource_eRemuxFeedback;
            *WhichSource = InputBits - 0x0E;
            break;

            default:
            BDBG_ERR(( "DataSource %lu is out of range!", InputBits ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            break;
        }
    }

    return( ExitCode );
}

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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Is the parser number within range? */
    if( ParserNum >= hXpt->MaxPidParsers )
    {
        /* Bad parser number. Complain. */
        BDBG_ERR(( "ParserNum %lu is out of range!", ParserNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#ifdef BCHP_PWR_RESOURCE_XPT_PARSER
        wasEnabled = BCHP_GET_FIELD_DATA(Reg, XPT_FE_PARSER0_CTRL1, PARSER_ENABLE);
        /* only change refcnt if changing state */
        if (!wasEnabled && Enable) {
            BCHP_PWR_AcquireResource(hXpt->hChip, BCHP_PWR_RESOURCE_XPT_PARSER);
        }
#endif

        /* Clear all the bits we are about to change. */
        Reg &= ~( BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ENABLE ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ENABLE, Enable == true ? 1 : 0 ) );
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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

    unsigned FeSel = 0;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    BDBG_MSG(( "Configuring PID chnl %u for PID 0x%04lX on %s parser %u",
               PidChannelNum, Pid, BXPT_P_IS_PB( Band ) ? "PB" : "IB", Band & 0xFF ));

    if( Pid >= 0x2000 )
    {
        /* Bad PID. Complain. */
        BDBG_ERR(( "Pid %lu is out of range!", ( unsigned long ) Pid ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( PidChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
#if BXPT_HAS_IB_PID_PARSERS
        unsigned OldFeSel = 255;
        unsigned OldPid = 0x2000;
        unsigned OldBand = 0x2000;     /* Never have this many parser bands. */

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif
        PidChannelTableEntry *Entry = &hXpt->PidChannelTable[ PidChannelNum ];

#ifdef ENABLE_PLAYBACK_MUX
        /*gain access to the pid table*/
        BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
        Entry->Pid = Pid;
        Entry->Band = Band;
        Entry->IsAllocated = true;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        if( BXPT_P_IS_PB( Band ) )
        {
            BXPT_P_CLEAR_PB_FLAG( Band );   /* Remove the PB parser flag. */
            FeSel = 1;
        }

        OldFeSel = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL );
        OldBand = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );

#if BXPT_HAS_DIRECTV_SUPPORT
        /* Need to preserve the HD filter bits, if enabled */
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) )
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID, Pid )
            );
        }
        else
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
            );
        }
#else
        OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_PID );

        Reg &= ~(
            BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID )
        );

        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, FeSel ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID, Pid )
        );
#endif

        /* Write to the PID table ONLY if the configuration has changed. */
        if( OldFeSel != FeSel || OldBand != Band || OldPid != Pid )
        {
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
        }

        Entry->IsPidChannelConfigured = true;
#ifdef ENABLE_PLAYBACK_MUX
        /*leave pid table protected area*/
        BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

#else /* BXPT_HAS_IB_PID_PARSERS */
        /* unsigned OldFeSel = 255; */
        unsigned OldPid = 0x2000;
        unsigned OldBand = 0x2000;     /* Never have this many parser bands. */

        PidChannelTableEntry *Entry = &hXpt->PidChannelTable[ PidChannelNum ];

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        BSTD_UNUSED( FeSel );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        OldBand = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

#if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 )
        Reg &= ~(
            BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1 ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT, Band ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
        );
#else   /*(BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 ) */

#if BXPT_HAS_DIRECTV_SUPPORT
        /* Need to preserve the HD filter bits, if enabled */
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) )
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1 ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID, Pid )
            );
        }
        else
        {
            OldPid = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID );

            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
            );

            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1 ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
            );
        }
#else /*  BXPT_HAS_DIRECTV_SUPPORT */
        Reg &= ~(
            BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
            BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 1 ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, Band ) |
            BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, Pid )
        );
#endif /* BXPT_HAS_DIRECTV_SUPPORT */

#endif  /* (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 ) */

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        if ((OldPid != Pid) || (OldBand != Band)) {
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Entry->IsPidChannelConfigured = true;
#endif /* BXPT_HAS_IB_PID_PARSERS */

#ifdef ENABLE_PLAYBACK_MUX
    #ifdef SW7335_895_WORKAROUND
        /* For this workaround, enable PID Version Checking ONLY for channels not use by the playback mux. */
        if (Band < BXPT_P_MAX_PLAYBACKS - hXpt->NumPlaybackMuxes && FeSel )
        {
        #if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_P_PacketSubCfg TempPsubCfg;

                BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &TempPsubCfg );
        #endif

            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            Reg &= ~(1 << 28);
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        #if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
                    BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &TempPsubCfg );
        #endif
        }
    #endif
#endif
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* Set the PID channels enable bit. */
        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

    bool result = true;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

            BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
            BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_P_PacketSubCfg PsubCfg;
#endif

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    return(result);
}

BERR_Code BXPT_SetNumPlaybackMux(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int Num                /* [in] Number of playback mux blocks*/
    )
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. The 'Num' argument is 1-based, not 0-based like everything else. */
    if( Num > BXPT_P_MAX_PLAYBACKS )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "Num %u is out of range!", Num ));
        return(BERR_INVALID_PARAMETER);
    }

    hXpt->NumPlaybackMuxes = Num;

    return(BERR_SUCCESS);
}

#endif /*ENABLE_PLAYBACK_MUX*/

BERR_Code BXPT_GetPidChannelConfig(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get config for. */
    unsigned int *Pid,              /* [out] The PID its using. */
    unsigned int *Band,             /* [out] The parser band the channel is mapped to. */
    bool *IsPlayback                /* [out] true if band is a playback parser, false if input */
    )
{
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_P_PacketSubCfg PsubCfg;
#endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( Pid );
    BDBG_ASSERT( Band );

    if( PidChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        uint32_t Reg, RegAddr;

        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        *IsPlayback = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) ? true : false;

#if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 )
        *Band = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT );
#else
        *Band = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT );
#endif

#ifdef BCHP_XPT_FE_PID_TABLE_i_HD_FILT_DIS_PID_CHANNEL_PID_SHIFT
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) )
        {
            *Pid = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_PID_CHANNEL_SCID );
        }
        else
        {
            *Pid = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID );
        }
#else
        *Pid = (unsigned int) BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_PID );
#endif
    }

    return ExitCode;
}

BERR_Code BXPT_FreePidChannel(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int PidChannelNum      /* [in] PID channel to free up. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

void BXPT_FreeAllPidChannels(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    unsigned int i;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
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

void BXPT_P_ResetTransport(
    BREG_Handle hReg
    )
{
    BDBG_ASSERT( hReg );
    BKNI_EnterCriticalSection();

    /* Assert the transport core reset */
    BREG_AtomicUpdate32_isr(hReg, BCHP_SUN_TOP_CTRL_SW_RESET,
        BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset),
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 1 ));

    /* Now clear the reset. */
    BREG_AtomicUpdate32_isr(hReg, BCHP_SUN_TOP_CTRL_SW_RESET,
        BCHP_MASK( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset),
        BCHP_FIELD_DATA( SUN_TOP_CTRL_SW_RESET, xpt_sw_reset, 0 ));

    BKNI_LeaveCriticalSection();
}

static void SetChannelEnable(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool EnableIt
    )
{
    uint32_t Reg, RegAddr;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

#ifdef ENABLE_PLAYBACK_MUX
    if(EnableIt && hXpt->PidChannelTable[PidChannelNum].Band >=
        BXPT_PB_PARSER((BXPT_P_MAX_PLAYBACKS - hXpt->NumPlaybackMuxes)))
    {
        return;
    }

    /* If the channel is enabled for playback muxing, don't disable it here. BXPT_SwitchPidChannel() will do it instead. */
    if( hXpt->PidChannelTable[PidChannelNum].MuxEnable )
    {
        return;
    }
#endif /*ENABLE_PLAYBACK_MUX*/

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    /* if the pid channel is already enabled, and request is to enable it again
       then make sure that we do not write the PID table register, this will prevent
       PID version from changing to a new value hence the buffers will not be flushed
       by the XPT HW.This step is imortant to get a glitch free av when record.
     */
    if((Reg & BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE ))&&EnableIt)
        return;

    Reg &= ~BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE );
    if( EnableIt )
    {
        Reg |= BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1 );
    }

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
}

static bool PidChannelHasDestination(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum
    )
{
    uint32_t Reg, RegAddr;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

    /* Set the PID channels enable bit. */
    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, PID_DESTINATION ) )
        return true;
    else
        return false;
}

BERR_Code BXPT_P_ClearAllPidChannelDestinations(
    BXPT_Handle hXpt,
    unsigned int PidChannelNum
    )
{
    uint32_t Reg, RegAddr;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

#ifdef ENABLE_PLAYBACK_MUX
    BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
    /* Disable PID channel */
    SetChannelEnable( hXpt, PidChannelNum, false );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_DESTINATION );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

#ifdef ENABLE_PLAYBACK_MUX
    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
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

#if BXPT_HAS_XCBUF
    uint32_t ParserBand, PacketSize;
#endif

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
    BXPT_P_PacketSubCfg PsubCfg;
#endif

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    RegAddr = BCHP_XPT_FE_SPID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    CurrentDestinations = BCHP_GET_FIELD_DATA( Reg, XPT_FE_SPID_TABLE_i, PID_DESTINATION );
    Reg &= ~BCHP_MASK( XPT_FE_SPID_TABLE_i, PID_DESTINATION );

    if( EnableIt )
    {
        CurrentDestinations |= ( 1ul << SelectedDestination );  /* Enable the pipe */
        Reg |= BCHP_FIELD_DATA( XPT_FE_SPID_TABLE_i, PID_DESTINATION, CurrentDestinations );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        /* ToDo: Enable PID channel if it was requested */
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

    /* determine the parser band */
    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * SPID_CHNL_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

#ifndef SW7342_241_WORKAROUND
    #if BXPT_HAS_XCBUF
        if (! BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ))
        {
            ParserBand = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) ;
            RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserBand * PARSER_REG_STEPSIZE );
            Reg = BREG_Read32( hXpt->hRegister, RegAddr );
            if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_PACKET_TYPE)== 1)
                PacketSize = 130;
            else
                PacketSize = 188;

            /* Increase the rate only when both G and R pipe are enabled. */
            if (( CurrentDestinations & 0x30 ) == 0x30 )
            {
                #define MAX_QAM_RATE    104000000

                unsigned long BlockOut = BXPT_P_XcBuf_GetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserBand );

                if( BlockOut != MAX_QAM_RATE )
                    hXpt->XcBufBO[ BXPT_XcBuf_Id_RAVE_IBP0 + ParserBand ] = BlockOut;

                /* Adjust Rave data read BW */
                BXPT_XcBuf_SetBandDataRate( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserBand, MAX_QAM_RATE, PacketSize );
            }
            else
            {
                BXPT_P_XcBuf_SetBlockout( hXpt, BXPT_XcBuf_Id_RAVE_IBP0 + ParserBand, hXpt->XcBufBO[ BXPT_XcBuf_Id_RAVE_IBP0 + ParserBand ] );
            }
        }
    #endif
#endif

    return ExitCode;
}

static bool IsPidDuplicated(
    BXPT_Handle hXpt,           /* [Input] Handle for this transport */
    unsigned int PidChannelNum
    )
{
    unsigned int i, TargetChannelCfg, ThisChannelCfg, ThisChannelEnable;
    uint32_t RegAddr, Reg;

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

    bool IsDuplicated = false;

#ifdef ENABLE_PLAYBACK_MUX
    if(hXpt->PidChannelTable[PidChannelNum].Band >=
        BXPT_PB_PARSER((BXPT_P_MAX_PLAYBACKS - hXpt->NumPlaybackMuxes)))
    {
        /*there are allowed to be duplicated pids on the muxed band*/
        return(false);
    }

    /*gain access to the pid table*/
    BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

    RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    TargetChannelCfg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    /* This mask covers all the bitfields we need to compare. */
#ifdef BCHP_XPT_FE_PID_TABLE_i_HD_FILT_DIS_PID_CHANNEL_PID_SHIFT

#if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 )
    TargetChannelCfg &= (
        BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
    );
#else
    TargetChannelCfg &= (
        BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
    );
#endif

#else
    TargetChannelCfg &= (
        BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
        BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID )
    );
#endif

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

    for( i = 0; i < hXpt->MaxPidChannels; i++ )
    {
        /* Skip the legitimate channel assignment. */
        if( i == PidChannelNum )
            continue;

        if( hXpt->PidChannelTable[ i ].IsPidChannelConfigured == true )
        {
            RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( i * PID_CHNL_STEPSIZE );
            ThisChannelCfg = Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#ifdef BCHP_XPT_FE_PID_TABLE_i_HD_FILT_DIS_PID_CHANNEL_PID_SHIFT
    #if (BCHP_CHIP == 7630 && BCHP_VER >= BCHP_VER_B2 )
                ThisChannelCfg &= (
                    BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_INPUT_SELECT ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
                );
    #else
                ThisChannelCfg &= (
                    BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
                    BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID )
                );
    #endif
#else
            ThisChannelCfg &= (
                BCHP_MASK( XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, PID_CHANNEL_PID )
            );
#endif

            /* We need to know if the channel has been enabled or has just been *requested* to be enabled */
            ThisChannelEnable = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE ) || hXpt->PidChannelTable[ i ].EnableRequested == true;

            if( TargetChannelCfg == ThisChannelCfg && ThisChannelEnable )
            {
                BDBG_ERR(( "Pid channels %lu and %lu are duplicates!", i, PidChannelNum ));
                IsDuplicated = true;
                break;
            }
        }
    }

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

#ifdef ENABLE_PLAYBACK_MUX
    /*leave pid table protected area*/
    BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
    return( IsDuplicated );
}

#ifdef SW7342_241_WORKAROUND

static void BXPT_P_ConfigArbiter(BREG_Handle hReg)
{
    uint32_t Reg = 0;

#if ( BCHP_CHIP == 7340 ) || ( BCHP_CHIP == 7342 )

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, Reg );

#else
    BSTD_UNUSED( Reg );
    #error "SW7342-241 Port needed"
#endif
}

#else

static void BXPT_P_ConfigArbiter(BREG_Handle hReg)
{
    uint32_t Reg = 0;

#if( BCHP_CHIP == 7405 ) || ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  ) || \
    ( BCHP_CHIP == 3548 ) || ( BCHP_CHIP == 3556 ) || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125) || ( BCHP_CHIP == 7340 )

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );

#elif ( BCHP_CHIP == 7440 )

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB0 );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_PB0, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_PB0, ARB_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB1 );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_PB1, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_PB1, ARB_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB2 );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_PB2, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_PB2, ARB_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB2, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, Reg );

#elif ( BCHP_CHIP == 3563 )

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RSBUFF_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RSBUFF_WR, ARB_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RSBUFF_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RSBUFF_RD, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_WR, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RMX0_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_RMX0_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_RMX0_RD, ARB_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RMX0_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_MSG_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_MSG_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_MSG_RD, ARB_SEL, 3 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_MSG_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, ARB_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL, 3 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB0 );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_PB0, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_PB0, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PB0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PSUB );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_PSUB, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_PSUB, ARB_SEL, 3 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_PSUB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_MSG_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_MSG_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_MSG_WR, ARB_SEL, 3 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_MSG_WR, Reg );

#elif (BCHP_CHIP == 7118) || (BCHP_CHIP == 7400 ) || (BCHP_CHIP == 7401 ) || (BCHP_CHIP == 7403 )

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RSBUFF_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RSBUFF_RD, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RSBUFF_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_WR, ARB_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, ARB_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_XCBUFF_RAVE_RD, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, ARB_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_CDB_WR, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR );
    Reg &= ~( BCHP_MASK( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, ARB_SEL, 3 ));
    BREG_Write32( hReg, BCHP_XPT_BUS_IF_ARB_SEL_RAVE_ITB_WR, Reg );

#elif ( BCHP_CHIP == 35230 ) || ( BCHP_CHIP == 35125 )

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, Reg );

#elif ( BCHP_CHIP == 7420 )
    uint32_t ulChipIdReg=0;
    ulChipIdReg = BREG_Read32(hReg, BCHP_SUN_TOP_CTRL_PROD_REVISION);
    ulChipIdReg &= 0xffff0000;
    ulChipIdReg >>= 16;
    BDBG_MSG(("Chip ID %x",ulChipIdReg));
    switch(ulChipIdReg)
    {
    case 0x7420:
         {
          BDBG_MSG((" SCB client mapping for 7420 transport h/w "));
    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL, 1 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7 );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, Reg );

    Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB );
    Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL, 2 ));
    BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, Reg );
       }
        break;
    case 0x7410:
        {
          BDBG_MSG((" SCB client mapping for 7410 transport h/w "));
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL, 1));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, Reg );
        }
        break;
    case 0x7409:
        {
          BDBG_MSG(("SCB client mapping for 7409 transport h/w "));
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, SCB_WR_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RSBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, SCB_WR_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_XCBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, SCB_WR_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_CDB, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, SCB_WR_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_RAVE_ITB, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, SCB_WR_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_WR_ARB_SEL_MSG, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, SCB_RD_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_RSBUFF, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, SCB_RD_CLIENT_SEL, 0 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RAVE, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, SCB_RD_CLIENT_SEL, 1));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_MSG, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, SCB_RD_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX0, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, SCB_RD_CLIENT_SEL, 1 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_XCBUFF_RMX1, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB0, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB1, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB2, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB3, Reg );
          Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4 );
          Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL ) );
          Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, SCB_RD_CLIENT_SEL, 2 ));
          BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB4, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB5, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB6, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7 );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PB7, Reg );
         Reg = BREG_Read32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB );
         Reg &= ~( BCHP_MASK( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL ) );
         Reg |= ( BCHP_FIELD_DATA( XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, SCB_RD_CLIENT_SEL, 2 ));
         BREG_Write32( hReg, BCHP_XPT_XMEMIF_SCB_RD_ARB_SEL_PSUB, Reg );
        }
        break;
    default:
        BDBG_ERR(("SCB client mapping for transport h/w module not defined"));
        BDBG_ERR(("Using the chip reset values"));
        break;
    }
#else
    BSTD_UNUSED( Reg );
    BSTD_UNUSED( hReg );
    /* Nothing specific needed yet. */
#endif
}

#endif

#if BXPT_HAS_IB_PID_PARSERS

static bool AllInputParsersAreOff(
    BXPT_Handle hXpt
    )
{
    uint32_t RegAddr, Reg;
    unsigned Index;

    for( Index = 0; Index < BXPT_P_MAX_PID_PARSERS; Index++ )
    {
        #if (BCHP_CHIP == 7342)
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(Index);
        #else
        RegAddr = BCHP_XPT_FE_PARSER0_CTRL1 + ( Index * PARSER_REG_STEPSIZE );
        #endif
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if( BCHP_GET_FIELD_DATA( Reg, XPT_FE_PARSER0_CTRL1, PARSER_ENABLE ) )
            return false;
    }

    return true;
}

#endif

static bool AllPlaybackChannelsAreOff(
    BXPT_Handle hXpt
    )
{
    uint32_t RegAddr, Reg;
    unsigned Index;

    for( Index = 0; Index < BXPT_P_MAX_PLAYBACKS; Index++ )
    {
        RegAddr = BCHP_XPT_PB0_CTRL1 + ( Index * PB_PARSER_REG_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if( BCHP_GET_FIELD_DATA( Reg, XPT_PB0_CTRL1, BUSY ) )
            return false;
    }

    return true;
}

#if BXPT_HAS_PACKETSUB

static bool AllPacketSubChannelsAreOff(
    BXPT_Handle hXpt
    )
{
    uint32_t RegAddr, Reg;
    unsigned Index;

    for( Index = 0; Index < BXPT_P_MAX_PACKETSUBS; Index++ )
    {
        RegAddr = BCHP_XPT_PSUB_PSUB0_STAT0 + ( Index * PACKET_SUB_REGISTER_STEP );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if( BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, BUSY ) )
            return false;
    }

    return true;
}

#endif

bool BXPT_P_CanPowerDown(
    BXPT_Handle hXpt
    )
{
    uint32_t RaveStatus;
    unsigned ItbIdle, CdbIdle, DmaBusy, AvMuxBufferDepth;
    unsigned WaitCount;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

#if BXPT_HAS_IB_PID_PARSERS
    /* All parser bands must be off */
    if( !AllInputParsersAreOff( hXpt ) )
    {
        BDBG_MSG(( "At least 1 parser band is enabled. XPT can't power down" ));
        return false;
    }
#endif

    /* All playback channels must be off */
    if( !AllPlaybackChannelsAreOff( hXpt ) )
    {
        BDBG_MSG(( "At least 1 playback channel is enabled. XPT can't power down" ));
        return false;
    }

#if BXPT_HAS_PACKETSUB
    /* All packet substitution channels must be off */
    if( !AllPacketSubChannelsAreOff( hXpt ) )
    {
        BDBG_MSG(( "At least 1 packet substitution channel is enabled. XPT can't power down" ));
        return false;
    }
#endif

#if BXPT_HAS_RSBUF && BXPT_HAS_XCBUF
    {
        unsigned RsBufDataRdy, XcBufDataRdy;

        /*
        ** If we got this far, we can wait for the RS and XC buffers to clear in a
        ** reasonable amount of time. What's reasonable? Start with 500uS.
        */
        WaitCount = 500;
        do
        {
            RsBufDataRdy = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_DATA_RDY );
            XcBufDataRdy = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_DATA_RDY );

            if( RsBufDataRdy || XcBufDataRdy )
            {
                WaitCount--;
                if( !WaitCount )
                {
                    BDBG_MSG(("RS/XC buffers haven't finished memory transactions. XPT can't power down" )) ;
                    return false;
                }

                BKNI_Delay( 1 );    /* Busy wait for 1 uS */
            }
        }
        while( RsBufDataRdy || XcBufDataRdy );
    }
#endif

    /* Make sure that RAVE has finished all memory transactions */
    WaitCount = 500;
    do
    {
        RaveStatus = BREG_Read32( hXpt->hRegister, BCHP_XPT_RAVE_AV_STATUS );
        ItbIdle = BCHP_GET_FIELD_DATA( RaveStatus, XPT_RAVE_AV_STATUS, ITB_MEM_STATE_IDLE );
        CdbIdle = BCHP_GET_FIELD_DATA( RaveStatus, XPT_RAVE_AV_STATUS, CDB_MEM_STATE_IDLE );
        DmaBusy = BCHP_GET_FIELD_DATA( RaveStatus, XPT_RAVE_AV_STATUS, DMA_BUSY );
        AvMuxBufferDepth = BCHP_GET_FIELD_DATA( RaveStatus, XPT_RAVE_AV_STATUS, AV_MUX_BUFFER_DEPTH );

        if( !ItbIdle || !CdbIdle || DmaBusy || AvMuxBufferDepth )
        {

            WaitCount--;
            if( !WaitCount )
            {
                BDBG_MSG(("RAVE hasn't finished memory transactions. Can't power down" )) ;
                return false;
            }

            BKNI_Sleep( 1 );
        }
    }
    while( !ItbIdle || !CdbIdle || DmaBusy );

    /* All the above tests had to pass to get this far. */
    BKNI_Delay( 500 );      /* Wait for XMEMIF buffers to clear. No way to tell when that happens, so wait */
    return true;
}

#if BXPT_HAS_RSBUF
BERR_Code BXPT_IsDataPresent(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned int PidChannelNum,     /* [in] Which channel to get status for. */
    bool *IsDataPresent
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
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

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_P_PacketSubCfg PsubCfg;
#endif

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_SaveCfg( hXpt, PidChannelNum, &PsubCfg );
#endif
        RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PidChannelNum * PID_CHNL_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

#if BXPT_HAS_PACKETSUB && BXPT_PSUB_PID_TABLE_WORKAROUND
        BXPT_PacketSub_P_RestoreCfg( hXpt, PidChannelNum, &PsubCfg );
#endif

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

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_CheckPipelineErrors(
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    )
{
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( Errors );
    BDBG_WRN(( "Data pipeline error checking is not supported" ));
    return BERR_NOT_SUPPORTED;
}
#endif

BERR_Code BXPT_GetLiveLTSID(
    BXPT_Handle hXpt,
    unsigned parserNum,
    unsigned *ltsid
    )
{
    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ltsid );

    if( parserNum >= BXPT_P_MAX_PLAYBACKS )
    {
        return BERR_INVALID_PARAMETER;
    }

#if BXPT_HAS_PARSER_REMAPPING
    {
        BXPT_BandMap map;

        /* See SW7425-5193 for the band-to-LTSID mapping. */
        map = hXpt->BandMap.FrontEnd[ parserNum ];
        *ltsid = map.VirtualParserIsPlayback ? map.VirtualParserBandNum + 0x10 : map.VirtualParserBandNum;
    }
#else
    BSTD_UNUSED( hXpt );
    *ltsid = parserNum;
#endif

    return BERR_SUCCESS;
}

/* end of file */
