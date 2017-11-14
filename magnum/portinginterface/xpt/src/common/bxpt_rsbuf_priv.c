/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bint.h"
#include "bxpt_rsbuf_priv.h"
#include "bchp_xpt_rsbuff.h"

#if BXPT_HAS_MPOD_RSBUF
#ifdef BCHP_XPT_RSBUFF_BASE_POINTER_CARD_IBP0
#define BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0 BCHP_XPT_RSBUFF_BASE_POINTER_CARD_IBP0
#define BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE BCHP_XPT_RSBUFF_CARD_IBP_BUFFER_ENABLE
#define BCHP_XPT_RSBUFF_BO_MPOD_IBP0 BCHP_XPT_RSBUFF_BO_CARD_IBP0
#define BCHP_XPT_RSBUFF_MPOD_IBP_OVERFLOW_STATUS BCHP_XPT_RSBUFF_CARD_IBP_OVERFLOW_STATUS
#endif
#endif

#define INPUT_BAND_BUF_SIZE_IN_PACKETS (800)
#ifdef BXPT_P_HAS_224B_SLOT_SIZE
   #define SLOT_SIZE (224)
   #define RS_BUFFER_WRAP_THRESHOLD 4
   #define RS_OVERFLOW_THRESHOLD (3 + RS_BUFFER_WRAP_THRESHOLD)
   #define RS_CARD_BUFFER_WRAP_THRESHOLD 4
#else
   #define SLOT_SIZE (256)
   #if BXPT_NUM_TSIO
      #define RS_OVERFLOW_THRESHOLD 0x3F
      #define RS_BUFFER_WRAP_THRESHOLD 4
      #define RS_CARD_BUFFER_WRAP_THRESHOLD 1
   #else
      #define RS_OVERFLOW_THRESHOLD 3
      #define RS_BUFFER_WRAP_THRESHOLD 4
   #endif
#endif

#define INPUT_BAND_BUF_SIZE         (INPUT_BAND_BUF_SIZE_IN_PACKETS * SLOT_SIZE)

#ifdef BXPT_P_HAS_224B_SLOT_SIZE
#if INPUT_BAND_BUF_SIZE % (4 * SLOT_SIZE)
   #error "INPUT_BAND_BUF_SIZE must be a multiple of 4 * the slot size"
#endif
#endif

/* PLAYBACK_BUF_SIZE_IN_PACKETS comes from the hw architects, SLOT_SIZE from the SW_GUIDE_TO_XPTv4.4.doc */
#ifdef BXPT_P_TSIO_USE_LARGER_BUFFERS
    #define PLAYBACK_BUF_SIZE_IN_PACKETS           (58 * 25)
    #define PLAYBACK_BUF_SIZE                      (PLAYBACK_BUF_SIZE_IN_PACKETS * SLOT_SIZE)
#else
    #define PLAYBACK_BUF_SIZE_IN_PACKETS           (32)
    #define PLAYBACK_BUF_SIZE                      (PLAYBACK_BUF_SIZE_IN_PACKETS * SLOT_SIZE)
#endif

#ifdef BXPT_P_HAS_224B_SLOT_SIZE
   #define SIZE_MULTIPLE   (4 * SLOT_SIZE)
   #define BUFFER_ALIGNMENT   64
#else
   #define SIZE_MULTIPLE   SLOT_SIZE
   #define BUFFER_ALIGNMENT   256
#endif

#define MINIMUM_BUF_SIZE            SIZE_MULTIPLE
#define BUFFER_PTR_REG_STEPSIZE     RS_BUFFER_PTR_REG_STEPSIZE
#define MAX_BITRATE                 ( 108000000 )
#define BLOCKOUT_REG_STEPSIZE       (BCHP_XPT_RSBUFF_BO_IBP1 - BCHP_XPT_RSBUFF_BO_IBP0)
#define DEFAULT_PACKET_LEN          (188)

/* Threshold for pause generation when XC Buffer for a corresponding band is almost full */
#define DEFAULT_PACKET_PAUSE_LEVEL  ( 12 )

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_rsbuf_priv );
BDBG_FILE_MODULE( xpt_rsbuf_regs );
#endif

#define REG_STEP (BCHP_XPT_RSBUFF_END_POINTER_IBP0 - BCHP_XPT_RSBUFF_BASE_POINTER_IBP0)

static void SetupBufferRegs(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance,
    size_t Size,
    BMMA_DeviceOffset Offset          /* [in] Size in bytes. Must be a multiple of 256. */
    )
{
    uint64_t addrValue;

    BDBG_MSG(("%s: BaseRegAddr %u, WhichInstance %u, Size %lu, Offset " BDBG_UINT64_FMT " ",
              BSTD_FUNCTION, BaseRegAddr, WhichInstance, (unsigned long) Size, BDBG_UINT64_ARG(Offset) ));
    BaseRegAddr = BaseRegAddr + WhichInstance * BUFFER_PTR_REG_STEPSIZE;

#ifdef BCHP_XPT_RSBUFF_WRAP_POINTER_IBP0
    addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_BASE_POINTER_IBP0, BASE, Offset);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr, addrValue );                   /* Set BASE */
    BDBG_MODULE_MSG(xpt_rsbuf_regs, ("%s(%x:%u) BASE " BDBG_UINT64_FMT, BSTD_FUNCTION, BaseRegAddr, WhichInstance, BDBG_UINT64_ARG(addrValue) ));

    /* Change the WRITE, VALID, and READ init values per SW7445-102 */
    if(Offset)
        addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_WRITE_POINTER_IBP0, WRITE, Offset - 1);
    else
        addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_WRITE_POINTER_IBP0, WRITE, 0xFF);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 2 * REG_STEP, addrValue );           /* Set WRITE */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 3 * REG_STEP, addrValue );           /* Set VALID */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 5 * REG_STEP, addrValue );           /* Set READ */
    BDBG_MODULE_MSG( xpt_rsbuf_regs, ("%s(%x:%u) WRITE/VALID/READ " BDBG_UINT64_FMT, BSTD_FUNCTION, BaseRegAddr, WhichInstance, BDBG_UINT64_ARG(addrValue) ));

    addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_END_POINTER_IBP0, END, Offset + Size - 1);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 1 * REG_STEP, addrValue );              /* Set END */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 4 * REG_STEP, addrValue );           /* Set WRAP */
    BDBG_MODULE_MSG(xpt_rsbuf_regs, ("%s(%x:%u) END/WRAP " BDBG_UINT64_FMT, BSTD_FUNCTION, BaseRegAddr, WhichInstance, BDBG_UINT64_ARG(addrValue) ));

    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 6 * REG_STEP, 0 );           /* Set WATERMARK */
#else
    addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_BASE_POINTER_IBP0, BASE, Offset);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr, addrValue );                   /* Set BASE */
    addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_END_POINTER_IBP0, END, Offset + Size - 1);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 1 * REG_STEP, addrValue );              /* Set END */

    /* Change the WRITE, VALID, and READ init values per SW7445-102 */
    if(Offset)
        addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_WRITE_POINTER_IBP0, WRITE, Offset - 1);
    else
        addrValue = BCHP_FIELD_DATA(XPT_RSBUFF_WRITE_POINTER_IBP0, WRITE, 0xFF);
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 2 * REG_STEP, addrValue );           /* Set WRITE */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 3 * REG_STEP, addrValue );           /* Set VALID */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 4 * REG_STEP, addrValue );           /* Set READ */
    BREG_WriteAddr( hXpt->hRegister, BaseRegAddr + 5 * REG_STEP, 0 );           /* Set WATERMARK */
#endif
}

void BXPT_P_SetupShareRsBufferRegs(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance,
    size_t Size,
    BMMA_DeviceOffset Offset
    )
{
    SetupBufferRegs( hXpt, BaseRegAddr, WhichInstance, Size, Offset );
    return;
}

static int GetBufferIndex(unsigned BaseRegAddr, unsigned WhichInstance)
{
    unsigned start = 0, index;

    switch (BaseRegAddr) {
        case BCHP_XPT_RSBUFF_BASE_POINTER_IBP0: start = 0; break;
        case BCHP_XPT_RSBUFF_BASE_POINTER_PBP0: start = BXPT_NUM_PID_PARSERS; break;
#if BXPT_HAS_MPOD_RSBUF
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

unsigned BXPT_P_GetMpodRsBufferIndex(
    unsigned WhichInstance
    )
{
#if BXPT_HAS_MPOD_RSBUF
    return GetBufferIndex(BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0, WhichInstance);
#else
    BSTD_UNUSED(WhichInstance);
    return 0;
#endif

}

static BERR_Code AllocateBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance,
    unsigned long Size          /* [in] Size in bytes. Must be a multiple of 256. */
    )
{
    BMMA_Block_Handle block;
    BMMA_DeviceOffset Offset;
    int index;

    /* If there is a secure heap defined, use it. */
    BMMA_Heap_Handle mmaHeap = hXpt->mmaRHeap ? hXpt->mmaRHeap : hXpt->mmaHeap;

    BDBG_MSG(("%s: BaseRegAddr %u, WhichInstance %u, Size %lu, SIZE_MULTIPLE %u", BSTD_FUNCTION, BaseRegAddr, WhichInstance, Size, SIZE_MULTIPLE));

    /* Size must be a multiple of the slot size. */
    Size = Size - ( Size % SIZE_MULTIPLE );

    index = GetBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0); /* this is internal, so do a hard assert */

    block = BMMA_Alloc(mmaHeap, Size, BUFFER_ALIGNMENT, NULL);
    if (!block) {
        BDBG_ERR(("RS buffer alloc failed!"));
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    Offset = BMMA_LockOffset(block);
    hXpt->rsbuff[index].block = block;
    hXpt->rsbuff[index].offset = Offset;
    SetupBufferRegs( hXpt, BaseRegAddr, WhichInstance, Size, Offset );
    return BERR_SUCCESS;
}

#ifndef BXPT_FOR_BOOTUPDATER
static BERR_Code DeleteBuffer(
    BXPT_Handle hXpt,
    unsigned BaseRegAddr,       /* [in] Which client buffer we are dealing with */
    unsigned WhichInstance
    )
{
    int index;
    BMMA_DeviceOffset Offset;

    index = GetBufferIndex(BaseRegAddr, WhichInstance);
    BDBG_ASSERT(index >= 0);

    BaseRegAddr = BaseRegAddr + WhichInstance * BUFFER_PTR_REG_STEPSIZE;
    Offset = BREG_ReadAddr(hXpt->hRegister, BaseRegAddr);
    if (Offset==hXpt->sharedRsXcBuff.offset) {
        return BERR_SUCCESS;
    }

    BMMA_UnlockOffset(hXpt->rsbuff[index].block, hXpt->rsbuff[index].offset);
    BMMA_Free(hXpt->rsbuff[index].block);

    return BERR_SUCCESS;
}
#endif /* BXPT_FOR_BOOTUPDATER */

static BERR_Code SetBufferEnable(
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
static bool IsBufferEnabled(
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

static unsigned long ComputeBlockOut(
    unsigned long PeakRate,         /* [in] Max data rate (in bps) the band will handle. */
    unsigned PacketLen,             /* [in] Packet size ,130 for dss and 188 for mpeg */
    unsigned maxReadSize            /* Number of packets that the rbus will read at each access. */
    )
{
    if( PeakRate < BXPT_MIN_PARSER_RATE )
    {
        BDBG_WRN(( "Minimum buffer rate is %u bps. PeakRate will be clamped to this value", BXPT_MIN_PARSER_RATE ));
        PeakRate = BXPT_MIN_PARSER_RATE;
    }
    else if( PeakRate > BXPT_MAX_PARSER_RATE )
    {
        BDBG_WRN(( "Maximum buffer rate is %u bps. PeakRate will be clamped to this value", BXPT_MAX_PARSER_RATE ));
        PeakRate = BXPT_MAX_PARSER_RATE;
    }

    return ((10800 * PacketLen * 8) / ( PeakRate / 10000 )) * maxReadSize;
}

static BERR_Code SetBlockout(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned BufferTypeBlockoutAddr,
    unsigned WhichInstance,
    unsigned long NewB0
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    RegAddr = BufferTypeBlockoutAddr + WhichInstance * BLOCKOUT_REG_STEPSIZE;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~BCHP_MASK( XPT_RSBUFF_BO_IBP0, BO_COUNT );
    Reg |= BCHP_FIELD_DATA( XPT_RSBUFF_BO_IBP0, BO_COUNT, NewB0 );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return( ExitCode );
}

/*
** These functions are called internally from BXPT_Open() and BXPT_Close().
** Users should NOT uses these functions directly.
*/

BERR_Code BXPT_P_RsBuf_Init(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    const BXPT_BandWidthConfig *BandwidthConfig
    )
{
    unsigned ii;
    unsigned maxReadSize = 1;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned totalAllocated = 0;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( BandwidthConfig );

    /* SWSTB-5934. The bitfield name changed on some parts. */
#ifdef BCHP_XPT_RSBUFF_MISC_CTRL_MPOD_NO_DEST_CHK_DIS_MASK
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MISC_CTRL );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_MISC_CTRL, MPOD_NO_DEST_CHK_DIS, 1 );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_MISC_CTRL, Reg );
    }
#elif BCHP_XPT_RSBUFF_MISC_CTRL_CARD_NO_DEST_CHK_DIS_MASK
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MISC_CTRL );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_MISC_CTRL, CARD_NO_DEST_CHK_DIS, 1 );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_MISC_CTRL, Reg );
    }
#endif

#if (BCHP_CHIP == 7278 && BCHP_VER == BCHP_VER_A0)
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_SPARE_BW_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_IBP_SPARE_BW_EN, SPARE_BW_EN, 1 << 0 );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_SPARE_BW_EN, Reg );

        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_SPARE_BW_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_PBP_SPARE_BW_EN, SPARE_BW_EN, 1 << 1 );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_SPARE_BW_EN, Reg );
    }
#endif

#ifdef BCHP_XPT_RSBUFF_MISC_CTRL_MAX_READ_SIZE_DEFAULT
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MISC_CTRL );
        maxReadSize = BCHP_GET_FIELD_DATA(Reg, XPT_RSBUFF_MISC_CTRL, MAX_READ_SIZE) ? 2 : 1;
    }
#endif

#ifdef BCHP_XPT_RSBUFF_IBP_BAND_RD_IN_PROGRESS_EN
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_BAND_RD_IN_PROGRESS_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
    }
#endif
#ifdef BCHP_XPT_RSBUFF_PBP_BAND_RD_IN_PROGRESS_EN
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_BAND_RD_IN_PROGRESS_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_PBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_BAND_RD_IN_PROGRESS_EN, Reg );
    }
#endif
#ifdef BCHP_XPT_RSBUFF_MPOD_IBP_BAND_RD_IN_PROGRESS_EN
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_BAND_RD_IN_PROGRESS_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_MPOD_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
    }
#endif
#ifdef BCHP_XPT_RSBUFF_TSIO_IBP_BAND_RD_IN_PROGRESS_EN
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_TSIO_IBP_BAND_RD_IN_PROGRESS_EN );
        BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_TSIO_IBP_BAND_RD_IN_PROGRESS_EN, BAND_RD_IN_PROGRESS_EN, 0xFFFFFFFF );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_TSIO_IBP_BAND_RD_IN_PROGRESS_EN, Reg );
    }
#endif

#if BXPT_NUM_TSIO
   Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_PACKET_LENGTH );
   BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_PACKET_LENGTH, PACKET_LENGTH, 192 );
   BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_PACKET_LENGTH, Reg );
#endif

#ifdef BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD
   Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD );
   BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_OVERFLOW_THRESHOLD, OVERFLOW_THRESHOLD, RS_OVERFLOW_THRESHOLD );
   BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_OVERFLOW_THRESHOLD, Reg );
#endif

#ifdef BCHP_XPT_RSBUFF_XPT_RS_BUFFER_WRAP_THRESHOLD
   Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_XPT_RS_BUFFER_WRAP_THRESHOLD );
   BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_XPT_RS_BUFFER_WRAP_THRESHOLD, WRAP_THRESHOLD, RS_BUFFER_WRAP_THRESHOLD );
   BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_XPT_RS_BUFFER_WRAP_THRESHOLD, Reg );
#endif

#ifdef BCHP_XPT_RSBUFF_XPT_CARD_BUFFER_WRAP_THRESHOLD
   Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_XPT_CARD_BUFFER_WRAP_THRESHOLD );
   BCHP_SET_FIELD_DATA( Reg, XPT_RSBUFF_XPT_CARD_BUFFER_WRAP_THRESHOLD, WRAP_THRESHOLD, RS_CARD_BUFFER_WRAP_THRESHOLD );
   BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_XPT_CARD_BUFFER_WRAP_THRESHOLD, Reg );
#endif

    #if BXPT_HAS_IB_PID_PARSERS
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] )
        {
            BDBG_MSG(( "Alloc RS for IB parser %u, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, ii, INPUT_BAND_BUF_SIZE );
            SetBlockout( hXpt, BCHP_XPT_RSBUFF_BO_IBP0, ii,
               ComputeBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_LEN, maxReadSize ) );
            SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE, ii, true );
            totalAllocated += INPUT_BAND_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            BDBG_MSG(( "Alloc shared RS for IB parser %u, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            SetupBufferRegs( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetBlockout(hXpt, BCHP_XPT_RSBUFF_BO_IBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE, 0 );    /* Default to OFF */
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( BandwidthConfig->MaxPlaybackRate[ ii ] )
        {
            BDBG_MSG(( "Alloc RS for PB parser %u, %u bps", ii, BandwidthConfig->MaxPlaybackRate[ ii ] ));
            AllocateBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_PBP0, ii, PLAYBACK_BUF_SIZE );
            SetBlockout( hXpt, BCHP_XPT_RSBUFF_BO_PBP0, ii,
               ComputeBlockOut( 2 * BandwidthConfig->MaxPlaybackRate[ ii ], DEFAULT_PACKET_LEN, maxReadSize ) );
            SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE, ii, true );
            totalAllocated += PLAYBACK_BUF_SIZE;
        }
        else
        {
            if (!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupBufferRegs( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_PBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetBlockout(hXpt, BCHP_XPT_RSBUFF_BO_PBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

    #if BXPT_HAS_MPOD_RSBUF

    #if BXPT_NUM_TSIO
        #define INPUT_BAND_MPOD_BUF_SIZE ( 200 * 1024 )
    #else
        #define INPUT_BAND_MPOD_BUF_SIZE INPUT_BAND_BUF_SIZE
    #endif

    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE, 0 );    /* Default to OFF */

    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( BandwidthConfig->MaxInputRate[ ii ] && BandwidthConfig->IbParserClients[ii].ToMpodRs )
        {
            BDBG_MSG(( "Alloc RS for IB MPOD parser %u, %u bps", ii, BandwidthConfig->MaxInputRate[ ii ] ));
            AllocateBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0, ii, INPUT_BAND_MPOD_BUF_SIZE );
            SetBlockout( hXpt, BCHP_XPT_RSBUFF_BO_MPOD_IBP0, ii,
               ComputeBlockOut( BandwidthConfig->MaxInputRate[ ii ], DEFAULT_PACKET_LEN, maxReadSize ) );
            SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE, ii, true );
            totalAllocated += INPUT_BAND_MPOD_BUF_SIZE;
        }
        else
        {
            if(!hXpt->sharedRsXcBuff.offset)
            {
                BXPT_P_AllocSharedXcRsBuffer( hXpt );
                totalAllocated += BXPT_P_MINIMUM_BUF_SIZE;
            }
            SetupBufferRegs( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0, ii, BXPT_P_MINIMUM_BUF_SIZE, hXpt->sharedRsXcBuff.offset );
            SetBlockout(hXpt, BCHP_XPT_RSBUFF_BO_MPOD_IBP0, ii, PWR_BO_COUNT);
        }
    }
    #endif

#if BXPT_NUM_TBG
    for (ii=0; ii<BXPT_NUM_TBG; ii++) {
        uint32_t TBG_BO_STEP_SIZE = BCHP_XPT_RSBUFF_TBG1_BO - BCHP_XPT_RSBUFF_TBG0_BO;
        BREG_Write32(hXpt->hRegister, BCHP_XPT_RSBUFF_TBG0_BO + TBG_BO_STEP_SIZE*ii, PWR_BO_COUNT);
    }
#endif
    BDBG_MSG(( "RS totalAllocated: %u bytes", totalAllocated ));
    return ExitCode;
}

#ifndef BXPT_FOR_BOOTUPDATER
BERR_Code BXPT_P_RsBuf_Shutdown(
    BXPT_Handle hXpt            /* [in] Handle for this transport */
    )
{
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    #if BXPT_HAS_IB_PID_PARSERS
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsBufferEnabled( hXpt, BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE, ii ) )
        {
            ExitCode |= SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE, ii, false );
            ExitCode |= DeleteBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of RS Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_PLAYBACK_PARSERS
    for( ii = 0; ii < BXPT_NUM_PLAYBACKS; ii++ )
    {
        if( IsBufferEnabled( hXpt, BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE, ii ) )
        {
            ExitCode |= SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE, ii, false );
            ExitCode |= DeleteBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_PBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of RS Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    #if BXPT_HAS_MPOD_RSBUF
    for( ii = 0; ii < BXPT_NUM_PID_PARSERS; ii++ )
    {
        if( IsBufferEnabled( hXpt, BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE, ii ) )
        {
            ExitCode |= SetBufferEnable( hXpt, BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE, ii, false );
            ExitCode |= DeleteBuffer( hXpt, BCHP_XPT_RSBUFF_BASE_POINTER_MPOD_IBP0, ii );
            if( ExitCode != BERR_SUCCESS )
            {
                BDBG_ERR(( "Disable/Delete of RS MPOD Buffer %d failed", ii ));
                goto Done;
            }
        }
    }
    #endif

    Done:
        return ExitCode;
}
#endif /* BXPT_FOR_BOOTUPDATER */

#if BXPT_HAS_PIPELINE_ERROR_REPORTING
BERR_Code BXPT_P_RsBuf_ReportOverflows(
    BXPT_Handle hXpt,
    BXPT_PipelineErrors *Errors
    )
{
    uint32_t Overflow;
    unsigned BufferNum;

    BERR_Code Status = 0;

#if BXPT_HAS_MPOD_RSBUF
    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_BUFFER_ENABLE ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_OVERFLOW_STATUS );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_MPOD_IBP_OVERFLOW_STATUS, 0 );
    Errors->overflow.RsbuffMpodIbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "RS MPOD buffer has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u]", BufferNum ));
        }
    }
#endif

    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_BUFFER_ENABLE ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_OVERFLOW_STATUS );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_IBP_OVERFLOW_STATUS, 0 );
    Errors->overflow.RsbuffIbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PID_PARSERS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "RS buffer has overflowed. Consider increasing BXPT_BandWidthConfig.MaxInputRate[%u]", BufferNum ));
        }
    }

    Overflow = BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_BUFFER_ENABLE ) &
        BREG_Read32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_OVERFLOW_STATUS );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_RSBUFF_PBP_OVERFLOW_STATUS, 0 );
    Errors->overflow.RsbuffPbp = Overflow;
    Status |= Overflow;
    if( Overflow )
    {
        for( BufferNum = 0; BufferNum < BXPT_NUM_PLAYBACKS; BufferNum++ )
        {
            if (Overflow & 1 << BufferNum)
                BDBG_ERR(( "RS buffer has overflowed. Consider increasing BXPT_BandWidthConfig.MaxPlaybackRate[%u]", BufferNum ));
        }
    }

    return Status;
}
#endif
