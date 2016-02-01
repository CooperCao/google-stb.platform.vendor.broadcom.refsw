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
#include "bxpt_rsbuf.h"

#include "bchp_xpt_rsbuff.h"

BDBG_MODULE( xpt_rsbuf );


#define MAX_BITRATE                 ( 108000000 )

#ifdef SW7342_241_WORKAROUND
    #define DEFAULT_PEAK_RATE           30000000
#else
    #define DEFAULT_PEAK_RATE           25000000
#endif

#define DEFAULT_BUF_SIZE            ( 200 * 1024 )
#define BLOCKOUT_REG_STEPSIZE       ( BCHP_XPT_RSBUFF_BO_IBP1 - BCHP_XPT_RSBUFF_BO_IBP0 )
#define BUFFER_PTR_REG_STEPSIZE     RS_BUFFER_PTR_REG_STEPSIZE

#if BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG
    #define MAX_NUM_RSBUFF (BXPT_NUM_PID_PARSERS*2 + BXPT_NUM_PLAYBACKS*1)
    #define MAX_NUM_XCBUFF (BXPT_NUM_PID_PARSERS*4 + BXPT_NUM_PLAYBACKS*4)
#else
    #define BXPT_NUM_PID_PARSERS BXPT_P_MAX_PID_PARSERS
    #define BXPT_NUM_PLAYBACKS BXPT_P_MAX_PLAYBACKS
#endif

static BERR_Code DisableBuffer( BXPT_Handle hXpt, unsigned BandNum );
static BERR_Code EnableBuffer( BXPT_Handle hXpt, unsigned BandNum );
static BERR_Code DeleteBuffer( BXPT_Handle hXpt, unsigned BaseRegAddr, unsigned WhichInstance );
static BERR_Code AllocateBuffer( BXPT_Handle hXpt, unsigned BaseRegAddr, unsigned WhichInstance, unsigned long BufferSize );
static BERR_Code DeletePbBuffer( BXPT_Handle hXpt, unsigned BandNum );
static BERR_Code AllocatePbBuffer( BXPT_Handle hXpt, unsigned BandNum, unsigned long BufferSize );
static BERR_Code SetBandDataRate(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,               /* [in] Which input band parser to configure */
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    );
static BERR_Code SetBlockout(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
    );

BERR_Code BXPT_RsBuf_SetBandDataRate(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,               /* [in] Which input band parser to configure */
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    )
{
#ifdef SW7342_241_WORKAROUND
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( BandNum );
    BSTD_UNUSED( PeakRate );
    BSTD_UNUSED( PacketLen );
    BDBG_ERR(( "BXPT_RsBuf_SetBandDataRate() ignored" ));
    return BERR_SUCCESS;
#else
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    hXpt->RsBufBO[ BandNum ] = BXPT_P_RsBuf_ComputeBlockOut( PeakRate, PacketLen );
    return SetBandDataRate( hXpt, BandNum, PeakRate, PacketLen );
#endif
}

static int GetBufferIndex(unsigned BaseRegAddr, unsigned WhichInstance)
{
    unsigned start = 0, index;

    switch (BaseRegAddr) {
        case BCHP_XPT_RSBUFF_BASE_POINTER_IBP0: start = 0; break;
        case BCHP_XPT_RSBUFF_BASE_POINTER_PBP0: start = BXPT_NUM_PID_PARSERS; break;
#ifdef BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0
        case BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0: start = BXPT_NUM_PID_PARSERS + BXPT_NUM_PLAYBACKS; break;
#endif
        default: return -1; break;
    }

    index = start + WhichInstance;
    if (index >= MAX_NUM_RSBUFF) {
        return -1;
    }

    return index;
}

static BERR_Code SetBandDataRate(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,               /* [in] Which input band parser to configure */
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    )
{
    uint32_t NewBO;

    BERR_Code ExitCode = BERR_SUCCESS;

    NewBO = BXPT_P_RsBuf_ComputeBlockOut( PeakRate, PacketLen );
    ExitCode = SetBlockout( hXpt, BandNum, NewBO );
    return( ExitCode );
}

BERR_Code BXPT_RsBuf_SetPlaybackDataRate(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned PbNum,                 /* [in] Which playback band parser to configure */
    unsigned long PeakRate          /* [in] Max data rate (in bps) the band will handle. */
    )
{
    uint32_t Reg, RegAddr, NewBO;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    NewBO = BXPT_P_RsBuf_ComputeBlockOut( PeakRate, 188 );

    RegAddr = BCHP_XPT_RSBUFF_BO_PBP0 + ( PbNum * BLOCKOUT_REG_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_RSBUFF_BO_PBP0, BO_COUNT );
    Reg |= BCHP_FIELD_DATA( XPT_RSBUFF_BO_PBP0, BO_COUNT, NewBO );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return( ExitCode );
}


uint32_t BXPT_P_RsBuf_ComputeBlockOut(
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen             /* [in] Packet size ,130 for dss and 188 for mpeg */
    )
{
    uint32_t NewBO;

    NewBO = MAX_BITRATE / 1000000;
    PeakRate = PeakRate / 1000000;
    NewBO = ( NewBO * PacketLen * 8 ) / PeakRate;  /* default ,set for mpeg ts */
    return NewBO;
}

BERR_Code BXPT_RsBuf_SetBufferSize(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,               /* [in] Which input band parser to configure */
    unsigned long BufferSize        /* [in] Buffer size in bytes */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    if( BXPT_P_RsBuf_IsBufferEnabled( hXpt, BandNum ) )
    {
        DisableBuffer( hXpt, BandNum );
        DeleteBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, BandNum );
        ExitCode = AllocateBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, BandNum, BufferSize );
        if( !ExitCode )
            EnableBuffer( hXpt, BandNum );
    }
    else
    {
        /* BXPT_DramBufferCfg in BXPT_DefaultSettings didn't request a buffer. */
        BDBG_ERR(( "Buffer BandNum %lu not used!", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return( ExitCode );
}

BERR_Code BXPT_RsBuf_SetPlaybackBufferSize(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,               /* [in] Which input band parser to configure */
    unsigned long BufferSize        /* [in] Buffer size in bytes */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    if( BXPT_P_RsBuf_IsBufferEnabled( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET ) )
    {
        DisableBuffer( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET );
        DeletePbBuffer( hXpt, BandNum );
        AllocatePbBuffer( hXpt, BandNum, BufferSize );
        EnableBuffer( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET );
    }
    else
    {
        /* BXPT_DramBufferCfg in BXPT_DefaultSettings didn't request a buffer. */
        BDBG_ERR(( "Buffer BandNum %lu not used!", BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return( ExitCode );
}

BERR_Code BXPT_P_RsBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_DramBufferCfg *Cfg
    )
{
    unsigned BandNum;
    unsigned BufferSize;
    unsigned ii;
    uint32_t InitialBlockOut;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned long TotalRsMemory = 0;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( Cfg );

    InitialBlockOut = BXPT_P_RsBuf_ComputeBlockOut( BXPT_P_DEFAULT_PEAK_RATE, 188 );
    for( ii = 0; ii < BXPT_P_MAX_PID_PARSERS; ii++ )
        hXpt->RsBufBO[ ii ] = InitialBlockOut;

    /* Set up RS buffers for the parser bands (not the input bands.) */
    for( BandNum = 0; BandNum < hXpt->MaxPidParsers; BandNum++ )
    {
        /* PR 25771: Need a buffer of at least 256 bytes. */
        if( Cfg->IbParserRsSize[ BandNum ] )
            BufferSize = Cfg->IbParserRsSize[ BandNum ] * 1024;
        else
            BufferSize = 256;

        SetBandDataRate( hXpt, BandNum, DEFAULT_PEAK_RATE,188 );

        AllocateBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, BandNum, BufferSize );
        EnableBuffer( hXpt, BandNum );

        TotalRsMemory += BufferSize;
        BDBG_MSG(( "Enabling RS buffer for IB parser %lu", BandNum ));
    }

    /* Set up RS buffers for the playback channels. */
    for( BandNum = 0; BandNum < hXpt->MaxPlaybacks; BandNum++ )
    {
        /* PR 25771: Need a buffer of at least 256 bytes. */
        if( Cfg->PbParserRsSize[ BandNum ] )
            BufferSize = Cfg->PbParserRsSize[ BandNum ] * 1024;
        else
            BufferSize = 256;

        BXPT_RsBuf_SetPlaybackDataRate( hXpt, BandNum, DEFAULT_PEAK_RATE );

        AllocatePbBuffer( hXpt, BandNum, BufferSize );
        EnableBuffer( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET );

        TotalRsMemory += BufferSize;
        BDBG_MSG(( "Enabling RS buffer for PB parser %lu", BandNum ));
    }

    /*
    ** WORKAROUND. PR 22836: If PB0 support is enabled, PB1 support must be also. But
    ** check that PB1 support isn't already enabled..
    ** NOTE: PR 22836 is covered by the workaround for PR 25771.
    */

    BDBG_MSG(( "Total RS memory usage is %lu bytes", TotalRsMemory ));

    /* Store the initial BlockOut values we've set above */
    for( ii = 0; ii < BXPT_P_MAX_PID_PARSERS; ii++ )
        hXpt->RsBufBO[ ii ] = BXPT_P_RsBuf_GetBlockout( hXpt, ii );

    return( ExitCode );
}

BERR_Code BXPT_P_RsBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    unsigned BandNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Delete the input band buffers */
    for( BandNum = 0; BandNum < hXpt->MaxPidParsers; BandNum++ )
    {
        if( BXPT_P_RsBuf_IsBufferEnabled( hXpt, BandNum ) )
        {
            DisableBuffer( hXpt, BandNum );
            DeleteBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, BandNum );
        }
    }

    /* Delete the parser band buffers */
    for( BandNum = 0; BandNum < hXpt->MaxPlaybacks; BandNum++ )
    {
        if( BXPT_P_RsBuf_IsBufferEnabled( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET ) )
        {
            DisableBuffer( hXpt, BandNum + BXPT_P_PLAYBACK_ENABLE_BIT_OFFSET );
            DeletePbBuffer( hXpt, BandNum );
        }
    }

    return( ExitCode );
}

static BERR_Code DisableBuffer(
    BXPT_Handle hXpt,
    unsigned BandNum
    )
{
    uint32_t EnReg;

    BERR_Code ExitCode = BERR_SUCCESS;

    EnReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_BUFFER_ENABLE );
    EnReg &= ~( 1ul << BandNum );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_BUFFER_ENABLE, EnReg );

    return( ExitCode );
}

static BERR_Code EnableBuffer(
    BXPT_Handle hXpt,
    unsigned BandNum
    )
{
    uint32_t EnReg;

    BERR_Code ExitCode = BERR_SUCCESS;

    EnReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_BUFFER_ENABLE );
    EnReg |= ( 1ul << BandNum );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_BUFFER_ENABLE, EnReg );

    return( ExitCode );
}

static BERR_Code DeleteBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,
    unsigned WhichInstance
    )
{
    int index;

    index = GetBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0);

    BMMA_UnlockOffset(hXpt->rsbuff[index].block, hXpt->rsbuff[index].offset);
    BMMA_Free(hXpt->rsbuff[index].block);

    return BERR_SUCCESS;
}

static BERR_Code AllocateBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,
    unsigned WhichInstance,
    unsigned long Size
    )
{
    BMMA_Block_Handle block;
    uint32_t Offset, RegAddr;
    int index;

    /* If there is a secure heap defined, use it. */
    BMMA_Heap_Handle mmaHeap = hXpt->mmaRHeap ? hXpt->mmaRHeap : hXpt->mmaHeap;

    /* Size must be a multiple of 256. */
    Size = Size - ( Size % 256 );

    index = GetBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0); /* this is internal, so do a hard assert */

    block = BMMA_Alloc(mmaHeap, Size, 256, 0);
    if (!block) {
        BDBG_ERR(("RS buffer alloc failed!"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    Offset = BMMA_LockOffset(block);
    hXpt->rsbuff[index].block = block;
    hXpt->rsbuff[index].offset = Offset;

    RegAddr = BaseRegAddr + ( WhichInstance * BUFFER_PTR_REG_STEPSIZE );
    BREG_Write32( hXpt->hRegister, RegAddr, Offset );                       /* Set BASE */
    BREG_Write32( hXpt->hRegister, RegAddr + 4, Offset + Size - 1 );        /* Set END */
    BREG_Write32( hXpt->hRegister, RegAddr + 8, Offset - 1 );               /* Set WRITE */
    BREG_Write32( hXpt->hRegister, RegAddr + 12, Offset - 1 );              /* Set VALID */
    BREG_Write32( hXpt->hRegister, RegAddr + 16, Offset - 1 );              /* Set READ */
    BREG_Write32( hXpt->hRegister, RegAddr + 20, 0 );                       /* Set WATERMARK */

    return BERR_SUCCESS;
}

bool BXPT_P_RsBuf_IsBufferEnabled(
    BXPT_Handle hXpt,
    unsigned BandNum
    )
{
    uint32_t EnReg;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    EnReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_BUFFER_ENABLE );
    return EnReg & ( 1ul << BandNum ) ? true : false;
}

/*
** On some chips, there is a memory hole between BCHP_XPT_RSBUFF_WRITE_POINTER_PBP1 and
** BCHP_XPT_RSBUFF_VALID_POINTER_PBP1: The address delta between these two registers is
** 8 bytes, not 4. So, here we define to handle that hole. Note that the define also
** handles the possibility that the hole gets fixed.
*/

#ifdef BCHP_XPT_RSBUFF_BASE_POINTER_PBP1
    #define WRITE_VALID_REG_DELTA ( BCHP_XPT_RSBUFF_VALID_POINTER_PBP1 - BCHP_XPT_RSBUFF_WRITE_POINTER_PBP1 - 4 )
#else
    #define WRITE_VALID_REG_DELTA ( 0 )
#endif

static uint32_t GetPbBaseAddr(
    unsigned BandNum
    )
{
    uint32_t RegAddr = BCHP_XPT_RSBUFF_BASE_POINTER_PBP0 + ( BandNum * BUFFER_PTR_REG_STEPSIZE );

    /* Handle the memory hole */
    switch( BandNum )
    {
        case 0:     /* Easy, do nothing: The hole is above the BASE register */
        case 1:
        break;

        default:    /* Everybody else has an address shifted up */
        RegAddr += WRITE_VALID_REG_DELTA;
        break;
    }

    return RegAddr;
}

static BERR_Code DeletePbBuffer(
    BXPT_Handle hXpt,
    unsigned BandNum
    )
{
    int index;

    index = GetBufferIndex(BCHP_XPT_RSBUFF_BASE_POINTER_PBP0, BandNum);
    BDBG_ASSERT(index >= 0);

    BMMA_UnlockOffset(hXpt->rsbuff[index].block, hXpt->rsbuff[index].offset);
    BMMA_Free(hXpt->rsbuff[index].block);

    return BERR_SUCCESS;
}

static BERR_Code AllocatePbBuffer(
    BXPT_Handle hXpt,
    unsigned BandNum,
    unsigned long Size
    )
{
    BMMA_Block_Handle block;
    uint32_t Offset, RegAddr;
    int index;

    /* If there is a secure heap defined, use it. */
    BMMA_Heap_Handle mmaHeap = hXpt->mmaRHeap ? hXpt->mmaRHeap : hXpt->mmaHeap;

    /* Size must be a multiple of 256. */
    Size = Size - ( Size % 256 );

    index = GetBufferIndex(BCHP_XPT_RSBUFF_BASE_POINTER_PBP0, BandNum);
    BDBG_ASSERT(index >= 0); /* this is internal, so do a hard assert */

    block = BMMA_Alloc(mmaHeap, Size, 256, 0);
    if (!block) {
        BDBG_ERR(("RS buffer alloc failed!"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    Offset = BMMA_LockOffset(block);
    hXpt->rsbuff[index].block = block;
    hXpt->rsbuff[index].offset = Offset;

    RegAddr = GetPbBaseAddr( BandNum );

    /* Handle the memory hole (again) */
    switch( BandNum )
    {
        case 1:     /* Take care of the step between WRITE and VALID. */
        BREG_Write32( hXpt->hRegister, RegAddr, Offset );                       /* Set BASE */
        BREG_Write32( hXpt->hRegister, RegAddr + 4, Offset + Size - 1 );  /* Set END */
        BREG_Write32( hXpt->hRegister, RegAddr + 8, Offset - 1 );               /* Set WRITE */
        BREG_Write32( hXpt->hRegister, RegAddr + 12 + WRITE_VALID_REG_DELTA, Offset - 1 );  /* Set VALID */
        BREG_Write32( hXpt->hRegister, RegAddr + 16 + WRITE_VALID_REG_DELTA, Offset - 1 );  /* Set READ */
        BREG_Write32( hXpt->hRegister, RegAddr + 20 + WRITE_VALID_REG_DELTA, 0 );           /* Set WATERMARK */
        break;

        default:    /* All other registers are okay */
        BREG_Write32( hXpt->hRegister, RegAddr, Offset );                       /* Set BASE */
        BREG_Write32( hXpt->hRegister, RegAddr + 4, Offset + Size - 1 );  /* Set END */
        BREG_Write32( hXpt->hRegister, RegAddr + 8, Offset - 1 );               /* Set WRITE */
        BREG_Write32( hXpt->hRegister, RegAddr + 12, Offset - 1 );              /* Set VALID */
        BREG_Write32( hXpt->hRegister, RegAddr + 16, Offset - 1 );              /* Set READ */
        BREG_Write32( hXpt->hRegister, RegAddr + 20, 0 );                       /* Set WATERMARK */
        break;
    }

    return BERR_SUCCESS;
}

unsigned long BXPT_P_RsBuf_GetBlockout(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum                /* [in] Which input band parser to configure */
    )
{
    uint32_t Reg, RegAddr;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    RegAddr = BCHP_XPT_RSBUFF_BO_IBP0 + ( BandNum * BLOCKOUT_REG_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    return( BCHP_GET_FIELD_DATA( Reg, XPT_RSBUFF_BO_IBP0, BO_COUNT ) );
}

BERR_Code BXPT_P_RsBuf_SetBlockout(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
    )
{
#ifdef SW7342_241_WORKAROUND
    BSTD_UNUSED( hXpt );
    BSTD_UNUSED( BandNum );
    BSTD_UNUSED( NewBO );
    BDBG_ERR(( "BXPT_P_RsBuf_SetBlockout() ignored" ));
    return BERR_SUCCESS;
#else
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    return SetBlockout( hXpt, BandNum, NewBO );
#endif
}

static BERR_Code SetBlockout(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    RegAddr = BCHP_XPT_RSBUFF_BO_IBP0 + ( BandNum * BLOCKOUT_REG_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_RSBUFF_BO_IBP0, BO_COUNT );
    Reg |= BCHP_FIELD_DATA( XPT_RSBUFF_BO_IBP0, BO_COUNT, NewBO );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return( ExitCode );
}

BERR_Code BXPT_P_RsBuf_PlaybackSetBlockout(
    BXPT_Handle hXpt,               /* [in] Handle for this transport */
    unsigned BandNum,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned long NewBO
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    RegAddr = BCHP_XPT_RSBUFF_BO_PBP0 + ( BandNum * BLOCKOUT_REG_STEPSIZE );
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_RSBUFF_BO_PBP0, BO_COUNT );
    Reg |= BCHP_FIELD_DATA( XPT_RSBUFF_BO_PBP0, BO_COUNT, NewBO );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return( ExitCode );
}
