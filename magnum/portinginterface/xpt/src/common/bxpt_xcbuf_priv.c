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
#include "bxpt_priv.h"
#include "bxpt.h"
#include "bkni.h"
#include "bxpt_xcbuf_priv.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_xcbuf_priv );
#endif

#ifdef BXPT_HAS_XCBUF_HW

#include "bchp_xpt_xcbuff.h"

#define INPUT_BAND_BUF_SIZE         (200* 1024)
#ifdef BXPT_P_TSIO_USE_LARGER_BUFFERS
    #define PLAYBACK_BUF_SIZE           (58 * 4 * 256)
#else
#define PLAYBACK_BUF_SIZE           (8 * 1024)
#endif
#define BUFFER_PTR_REG_STEPSIZE     (6 * 4)
#define MAX_BITRATE                 ( 108000000 )
#define BLOCKOUT_REG_STEPSIZE       4
#define DEFAULT_PACKET_SIZE         (188)

/* Threshold for pause generation when XC Buffer for a corresponding band is almost full */
#define DEFAULT_PACKET_PAUSE_LEVEL  ( 12 )

static void SetupRsBufferRegs(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance,
    size_t Size,
    uint32_t Offset          /* [in] Size in bytes. Must be a multiple of 256. */
    )
{
    /* Change the WRITE, VALID, and READ init values per SW7445-102 */
    uint32_t InitVal = Offset ? -1 : 0xFF;

    BaseRegAddr = BaseRegAddr + WhichInstance * BUFFER_PTR_REG_STEPSIZE;

    BREG_Write32( hXpt->hRegister, BaseRegAddr, Offset );                   /* Set BASE */
    BREG_Write32( hXpt->hRegister, BaseRegAddr + 4, Offset + Size - 1 );    /* Set END */
    BREG_Write32( hXpt->hRegister, BaseRegAddr + 8, Offset + InitVal );           /* Set WRITE */
    BREG_Write32( hXpt->hRegister, BaseRegAddr + 12, Offset + InitVal );          /* Set VALID */
    BREG_Write32( hXpt->hRegister, BaseRegAddr + 16, Offset + InitVal );          /* Set READ */
    BREG_Write32( hXpt->hRegister, BaseRegAddr + 20, 0 );                   /* Set WATERMARK */
}

static int GetRsBufferIndex(unsigned BaseRegAddr, unsigned WhichInstance)
{
    unsigned start = 0, index;

    switch (BaseRegAddr) {
        case BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0: start = 0; break;
        case BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0: start = BXPT_NUM_PID_PARSERS; break;
#if BXPT_NUM_MESG_BUFFERS
        case BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0:  start = BXPT_NUM_PID_PARSERS + BXPT_NUM_PLAYBACKS; break;
        case BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0:  start = BXPT_NUM_PID_PARSERS*2 + BXPT_NUM_PLAYBACKS; break;
#endif
#if BXPT_NUM_REMULTIPLEXORS
        case BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0: start = BXPT_NUM_PID_PARSERS*2 + BXPT_NUM_PLAYBACKS*2; break;
        case BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0: start = BXPT_NUM_PID_PARSERS*3 + BXPT_NUM_PLAYBACKS*2; break;
        case BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0: start = BXPT_NUM_PID_PARSERS*4 + BXPT_NUM_PLAYBACKS*2; break;
        case BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0: start = BXPT_NUM_PID_PARSERS*4 + BXPT_NUM_PLAYBACKS*3; break;
#endif
        default: return -1; break;
    }

    index = start + WhichInstance;
    if (index >= MAX_NUM_XCBUFF) {
        return -1;
    }

    return index;
}

static BERR_Code AllocateRsBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance,
    unsigned long Size          /* [in] Size in bytes. Must be a multiple of 256. */
    )
{
    BMMA_Block_Handle block;
    uint32_t Offset;
    int index;

    /* If there is a secure heap defined, use it. */
    BMMA_Heap_Handle mmaHeap = hXpt->mmaRHeap ? hXpt->mmaRHeap : hXpt->mmaHeap;

    /* Size must be a multiple of 256. */
    Size = Size - ( Size % 256 );

    index = GetRsBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0); /* this is internal, so do a hard assert */

    block = BMMA_Alloc(mmaHeap, Size, 256, 0);
    if (!block) {
        BDBG_ERR(("XC buffer alloc failed!"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    Offset = BMMA_LockOffset(block);
    hXpt->xcbuff[index].block = block;
    hXpt->xcbuff[index].offset = Offset;

    SetupRsBufferRegs( hXpt, BaseRegAddr, WhichInstance, Size, Offset );
    return BERR_SUCCESS;
}

#ifndef BXPT_FOR_BOOTUPDATER
static BERR_Code DeleteRsBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance
    )
{
    int index;

    index = GetRsBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0);

#if BXPT_P_HAS_XCBUFF_ENABLE_WORKAROUND
{
    uint32_t Offset;
    BaseRegAddr = BaseRegAddr + WhichInstance * BUFFER_PTR_REG_STEPSIZE;
    Offset = BREG_Read32(hXpt->hRegister, BaseRegAddr);

    if (Offset==hXpt->sharedRsXcBuff.offset) {
        return BERR_SUCCESS;
    }
}
#endif

    BMMA_UnlockOffset(hXpt->xcbuff[index].block, hXpt->xcbuff[index].offset);
    BMMA_Free(hXpt->xcbuff[index].block);

    return BERR_SUCCESS;
}
#endif

static BERR_Code SetRsBufferEnable(
    BXPT_Handle hXpt,
    unsigned EnableRegAddr,
    unsigned Index,
    bool EnableIt
    )
{
    uint32_t EnReg;

    BERR_Code ExitCode = BERR_SUCCESS;

    EnReg = BREG_Read32( hXpt->hRegister, EnableRegAddr );
    if( EnableIt )
    {
        EnReg |= ( 1ul << Index );
    }
    else
    {
        EnReg &= ~( 1ul << Index );
    }
    BREG_Write32( hXpt->hRegister, EnableRegAddr, EnReg );

    return( ExitCode );
}

#ifndef BXPT_FOR_BOOTUPDATER
static bool IsRsBufferEnabled(
    BXPT_Handle hXpt,
    unsigned EnableRegAddr,
    unsigned Index
    )
{
    uint32_t EnReg = BREG_Read32( hXpt->hRegister, EnableRegAddr );
    if( EnReg & ( 1ul << Index ) )
        return true;
    return false;
}
#endif /* BXPT_FOR_BOOTUPDATER */

static unsigned long ComputeRsBlockOut(
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    char *BufferName,
    unsigned BufferIndex
    )
{
    /* Need to double the peak rate in order to support simultaneous R and G pipe traffic. */
    PeakRate *= 2;

    /* Error reporting is done in the RS buffer setup only. The RS rates really determine the
    bandwidth needs. This code just needs to make sure we don't overflow the bitfield. */
    if( PeakRate < BXPT_MIN_PARSER_RATE )
    {
#if 0
        BDBG_WRN(( "Requested rate for %s [%u] is %u bps, but supported minimum is %u bps. Minimum rate will be used.",
            BufferName, BufferIndex, PeakRate, BXPT_MIN_PARSER_RATE ));
#else
    BSTD_UNUSED( BufferName );
    BSTD_UNUSED( BufferIndex );
#endif

        PeakRate = BXPT_MIN_PARSER_RATE;
    }
    else if( PeakRate > BXPT_MAX_PARSER_RATE )
    {
#if 0
        BDBG_WRN(( "Requested rate for %s [%u] is %u bps, but supported maximum is %u bps. Maximum rate will be used.",
            BufferName, BufferIndex, PeakRate, BXPT_MAX_PARSER_RATE ));
#else
    BSTD_UNUSED( BufferName );
    BSTD_UNUSED( BufferIndex );
#endif
        PeakRate = BXPT_MAX_PARSER_RATE;
    }

    return (10800 * PacketLen * 8) / ( PeakRate / 10000 );
}

static BERR_Code SetRsBlockout(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned BufferTypeBlockoutAddr,
    unsigned WhichInstance,
    unsigned long NewB0
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

/*
NewB0 = ComputeRsBlockOut( 45000000, 192, "Buffer", WhichInstance );
BDBG_ERR(( "Overriding XC blockout to 45 Mbps: %u, 0x%04X", NewB0, NewB0 ));
*/
    RegAddr = BufferTypeBlockoutAddr + WhichInstance * BLOCKOUT_REG_STEPSIZE;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_XCBUFF_BO_RAVE_IBP0, BO_COUNT );
    Reg |= BCHP_FIELD_DATA( XPT_XCBUFF_BO_RAVE_IBP0, BO_COUNT, NewB0 );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return( ExitCode );
}

BERR_Code BXPT_P_XcBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_BandWidthConfig *BandwidthConfig
    )
{
    uint32_t Reg;
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned totalAllocated = 0;

    BDBG_ASSERT( hXpt );

    /* Set Pause to 12 packets */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_PAUSE_THRESHOLD );
    Reg &= ~( BCHP_MASK( XPT_XCBUFF_PAUSE_THRESHOLD, PACKETS ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_XCBUFF_PAUSE_THRESHOLD, PACKETS, DEFAULT_PACKET_PAUSE_LEVEL ) );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_PAUSE_THRESHOLD, Reg );

/* SW7445-2729 */
#ifdef BCHP_XPT_XCBUFF_RAVE_IBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_IBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RAVE_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_RAVE_PBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_PBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RAVE_PBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_PBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_MSG_IBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_IBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_MSG_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_MSG_PBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_PBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_MSG_PBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_PBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_RMX0_IBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_IBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RMX0_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_RMX0_PBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_PBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RMX0_PBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_PBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_RMX1_IBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_IBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RMX1_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif
#ifdef BCHP_XPT_XCBUFF_RMX1_PBP_BAND_RD_IN_PROGRESS_EN
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_PBP_BAND_RD_IN_PROGRESS_EN );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_RMX1_PBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_PBP_BAND_RD_IN_PROGRESS_EN, Reg );
#endif

#if BXPT_NUM_TSIO
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_PACKET_LENGTH );
    BCHP_SET_FIELD_DATA( Reg, XPT_XCBUFF_PACKET_LENGTH, PACKET_LENGTH, 192 );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_PACKET_LENGTH, Reg );
#endif

    /*
    ** XC buffer bitrates should be set to 2 times the expected input rate, to handle watch and
    ** record. That usage requires both the R-pipe and G-pipe to be enabled.
    */
    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_RAVE
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] && BandwidthConfig->IbParserClients[ ii ].ToRave )
        {
            BDBG_MSG(( "Alloc XC for IB parser %u to RAVE, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0, ii, INPUT_BAND_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RAVE_IBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_SIZE, "MaxInputRate", ii ) );

            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP, ii, true );
            totalAllocated += INPUT_BAND_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RAVE_IBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_RAVE
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( BandwidthConfig->MaxPlaybackRate[ ii ] && BandwidthConfig->PlaybackParserClients[ ii ].ToRave )
        {
            BDBG_MSG(( "Alloc XC for PB parser %u to RAVE, %u bps", ii, BandwidthConfig->MaxPlaybackRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0, ii, PLAYBACK_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RAVE_PBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxPlaybackRate[ ii ], DEFAULT_PACKET_SIZE, "MaxPlaybackRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP, ii, true );
            totalAllocated += PLAYBACK_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RAVE_PBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_MESG_BUFFERS
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] && BandwidthConfig->IbParserClients[ ii ].ToMsg )
        {
            BDBG_MSG(( "Alloc XC for IB parser %u to MSG, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0, ii, INPUT_BAND_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_MSG_IBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_SIZE, "MaxInputRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP, ii, true );
            totalAllocated += INPUT_BAND_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_MSG_IBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_MESG_BUFFERS
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( BandwidthConfig->MaxPlaybackRate[ ii ] && BandwidthConfig->PlaybackParserClients[ ii ].ToMsg )
        {
            BDBG_MSG(( "Alloc XC for PB parser %u to MSG, %u bps", ii, BandwidthConfig->MaxPlaybackRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0, ii, PLAYBACK_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_MSG_PBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxPlaybackRate[ ii ], DEFAULT_PACKET_SIZE, "MaxPlaybackRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP, ii, true );
            totalAllocated += PLAYBACK_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_MSG_PBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_REMUX
    /* We have at least RMX0 */
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_IBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] && BandwidthConfig->RemuxUsed[ 0 ] && BandwidthConfig->IbParserClients[ ii ].ToRmx[ 0 ] )
        {
            BDBG_MSG(( "Alloc XC for IB parser %u to RMX0, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0, ii, INPUT_BAND_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RMX0_IBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_SIZE, "MaxInputRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_IBP, ii, true );
            totalAllocated += INPUT_BAND_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RMX0_IBP0, ii, PWR_BO_COUNT);
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_IBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] && BandwidthConfig->RemuxUsed[ 1 ] && BandwidthConfig->IbParserClients[ ii ].ToRmx[ 1 ])
        {
            BDBG_MSG(( "Alloc XC for IB parser %u to RMX1, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0, ii, INPUT_BAND_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RMX1_IBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_SIZE, "MaxInputRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_IBP, ii, true );
            totalAllocated += INPUT_BAND_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RMX1_IBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, input bands"
    #endif

    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_REMUX
    /* We have at least RMX0 */
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_PBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( BandwidthConfig->MaxPlaybackRate[ ii ] && BandwidthConfig->RemuxUsed[ 0 ] && BandwidthConfig->PlaybackParserClients[ ii ].ToRmx[ 0 ] )
        {
            BDBG_MSG(( "Alloc XC for PB parser %u to RMX0, %u bps", ii, BandwidthConfig->MaxPlaybackRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0, ii, PLAYBACK_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RMX0_PBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxPlaybackRate[ ii ], DEFAULT_PACKET_SIZE, "MaxPlaybackRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_PBP, ii, true );
            totalAllocated += PLAYBACK_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RMX0_PBP0, ii, PWR_BO_COUNT);
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_PBP, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( BandwidthConfig->MaxPlaybackRate[ ii ] && BandwidthConfig->RemuxUsed[ 1 ] && BandwidthConfig->PlaybackParserClients[ ii ].ToRmx[ 1 ])
        {
            BDBG_MSG(( "Alloc XC for PB parser %u to RMX1, %u bps", ii, BandwidthConfig->MaxPlaybackRate[ ii ] ));
            AllocateRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0, ii, PLAYBACK_BUF_SIZE );
            SetRsBlockout( hXpt, BCHP_XPT_XCBUFF_BO_RMX1_PBP0, ii,
               ComputeRsBlockOut( BandwidthConfig->MaxPlaybackRate[ ii ], DEFAULT_PACKET_SIZE, "MaxPlaybackRate", ii ) );
            SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_PBP, ii, true );
            totalAllocated += PLAYBACK_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupRsBufferRegs( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset);
            SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_BO_RMX1_PBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, playback channels"
    #endif

    #endif

#if BXPT_NUM_TBG
    for (ii=0; ii<BXPT_NUM_TBG; ii++) {
        SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_TBG0_BO_RAVE, ii, PWR_BO_COUNT);
        SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_TBG0_BO_MSG, ii, PWR_BO_COUNT);
        SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_TBG0_BO_RMX0, ii, PWR_BO_COUNT);
        SetRsBlockout(hXpt, BCHP_XPT_XCBUFF_TBG0_BO_RMX1, ii, PWR_BO_COUNT);
    }
#endif

#if BXPT_P_HAS_XCBUFF_ENABLE_WORKAROUND
    /* do MSG_PBP before MSG_IBP because we don't want, i.e. MSG_IBP -> MSG_PBP -> RAVE_PBP */
    Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP);
    for (ii=0; ii<BXPT_NUM_PLAYBACKS; ii++) {
        if ((Reg >> ii) & 0x1) {
            SetRsBufferEnable(hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP, ii, true);
        }
    }

    Reg = BREG_Read32(hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP);
    for (ii=0; ii<BXPT_NUM_PID_PARSERS; ii++) {
        if ((Reg >> ii) & 0x1) {
            SetRsBufferEnable(hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP, ii, true);
            SetRsBufferEnable(hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP, ii, true);
        }
    }
#endif

    BDBG_MSG(( "XC totalAllocated: %u bytes", totalAllocated ));
    return( ExitCode );
}

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_P_XcBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_RAVE
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RAVE Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_RAVE
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RAVE_PBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RAVE Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_MESG_BUFFERS
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC Msg Input Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_MESG_BUFFERS
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_MSG_PBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC Msg Playback Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_REMUX
    /* We have at least RMX0 */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_IBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_IBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RMX0 Input Buffer %d failed", ii ));
                goto Done;
            }
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_IBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_IBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RMX1 Input Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, input bands"
    #endif

    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_REMUX
    /* We have at least RMX0 */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_PBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_PBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX0_PBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RMX0 Playback Buffer %d failed", ii ));
                goto Done;
            }
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( IsRsBufferEnabled( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_PBP, ii ) )
        {
            ExitCode |= SetRsBufferEnable( hXpt, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_PBP, ii, false );
            ExitCode |= DeleteRsBuffer( hXpt, BCHP_XPT_XCBUFF_BASE_POINTER_RMX1_PBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of XC RMX1 Playback Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, playback channels"
    #endif

    #endif

    Done:
    return( ExitCode );
}

#ifdef BXPT_IS_CORE40NM
void BXPT_XcBuf_P_EnablePlaybackPausing(
    BXPT_Handle hXpt,
    unsigned PbChannelNum,
    bool PauseEn
    )
{
    uint32_t Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_PAUSE_EN_PBP );

    if( PauseEn)
    {
        BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_PAUSE_EN_PBP, Reg | ( 1 << PbChannelNum ) );
    }
    else
    {
        BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_PAUSE_EN_PBP, Reg & ~( 1 << PbChannelNum ) );
    }
}
#endif

#endif /* BXPT_FOR_BOOTUPDATER */

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_P_XcBuf_ReportOverflows(
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    )
{
    uint32_t Overflow;
    unsigned BufferNum;

    BERR_Code Status = 0;

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_RAVE
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_IBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_OVERFLOW_STATUS_IBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_OVERFLOW_STATUS_IBP, 0 );
    Errors->overflow.XcbuffRaveIbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for IB parser %u to RAVE has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u] or enable RAVE overflow interrupts", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_RAVE
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_CTRL_BUFFER_EN_PBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_OVERFLOW_STATUS_PBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RAVE_OVERFLOW_STATUS_PBP, 0 );
    Errors->overflow.XcbuffRavePbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PLAYBACKS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for PB parser %u to RAVE has overflowed. Consider increasing BXPT_BandWidthConfig.MaxPlaybackRate[%u] or enable RAVE overflow interrupts", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_MESG_BUFFERS
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_IBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_OVERFLOW_STATUS_IBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_OVERFLOW_STATUS_IBP, 0 );
    Errors->overflow.XcbuffMsgIbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for IB parser %u to Mesg filter has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u] or enable Mesg overflow interrupts", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_MESG_BUFFERS
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_CTRL_BUFFER_EN_PBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_OVERFLOW_STATUS_PBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_MSG_OVERFLOW_STATUS_PBP, 0 );
    Errors->overflow.XcbuffMsgPbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PLAYBACKS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for PB parser %u to Mesg filter has overflowed. Consider increasing BXPT_BandWidthConfig.MaxPlaybackRate[%u] or enable Mesg overflow interrupts", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_HAS_IB_PID_PARSERS && BXPT_HAS_REMUX
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_IBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_OVERFLOW_STATUS_IBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_OVERFLOW_STATUS_IBP, 0 );
    Errors->overflow.XcbuffRmx0Ibp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for IB parser %u to Remux0 has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u]", BufferNum, BufferNum ));
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_IBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_OVERFLOW_STATUS_IBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_OVERFLOW_STATUS_IBP, 0 );
    Errors->overflow.XcbuffRmx1Ibp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for IB parser %u to Remux0 has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u]", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, input bands"
    #endif

    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS && BXPT_HAS_REMUX
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_CTRL_BUFFER_EN_PBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_OVERFLOW_STATUS_PBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX0_OVERFLOW_STATUS_PBP, 0 );
    Errors->overflow.XcbuffRmx0Pbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PLAYBACKS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for PB parser %u to Remux0 has overflowed. Consider increasing BXPT_BandWidthConfig.MaxPlaybackRate[%u]", BufferNum, BufferNum ));
        }
    }

    #if BXPT_NUM_REMULTIPLEXORS > 0
    /* We've got at least RMX1 */
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_CTRL_BUFFER_EN_PBP ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_OVERFLOW_STATUS_PBP );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_XCBUFF_RMX1_OVERFLOW_STATUS_PBP, 0 );
    Errors->overflow.XcbuffRmx1Pbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PLAYBACKS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "XC buffer for PB parser %u to Remux0 has overflowed. Consider increasing BXPT_BandWidthConfig.MaxPlaybackRate[%u]", BufferNum, BufferNum ));
        }
    }
    #endif

    #if BXPT_NUM_REMULTIPLEXORS > 2
    #error "Add support for remux2 and higher, input bands"
    #endif

    #endif

    return Status;
}
#endif

#else
/* Some chips do not have XC buffers. */

BERR_Code BXPT_P_XcBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_BandWidthConfig *BandwidthConfig
    )
{
    BSTD_UNUSED(hXpt);
    BSTD_UNUSED(BandwidthConfig);
    BDBG_MSG(( "%s: XC Buffers not present on this chip.", BSTD_FUNCTION ));
    return( BERR_SUCCESS );
}

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_P_XcBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    BSTD_UNUSED(hXpt);
    BDBG_MSG(( "%s: XC Buffers not present on this chip.", BSTD_FUNCTION ));
    return( BERR_SUCCESS );
}

#endif /* BXPT_FOR_BOOTUPDATER */

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_P_XcBuf_ReportOverflows(
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    )
{
    BERR_Code Status = 0;
    BSTD_UNUSED(hXpt);
    BDBG_MSG(( "%s: XC Buffers not present on this chip.", BSTD_FUNCTION ));
    Errors->overflow.XcbuffRaveIbp = 0;
    Errors->overflow.XcbuffRavePbp = 0;
    Errors->overflow.XcbuffMsgPbp = 0;
    Errors->overflow.XcbuffRmx0Ibp = 0;
    Errors->overflow.XcbuffRmx1Ibp = 0;
    Errors->overflow.XcbuffRmx0Pbp = 0;
    Errors->overflow.XcbuffRmx1Pbp = 0;
    return Status;
}
#endif
#endif
