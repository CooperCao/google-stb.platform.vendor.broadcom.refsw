/******************************************************************************
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
 *****************************************************************************/


#include "bstd.h"
#include "bxpt_priv.h"
#include "bxpt_packetsub.h"
#include "bkni.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#include "bchp_xpt_fe.h"
#include "bchp_int_id_xpt_bus_if.h"
#include "bchp_xpt_psub.h"

/* Size of an individual packetsub module register space, in bytes */
#ifdef BCHP_XPT_PSUB_PSUB1_CTRL0
#define PACKET_SUB_REGISTER_STEP    ( BCHP_XPT_PSUB_PSUB1_CTRL0 - BCHP_XPT_PSUB_PSUB0_CTRL0 )
#else
#define PACKET_SUB_REGISTER_STEP    ( BCHP_XPT_PSUB_PSUB0_STAT2 - BCHP_XPT_PSUB_PSUB0_CTRL0 )
#endif

#define BXPT_P_PSUB_DEFAULT_PACKET_LEN      ( 188 )
#define BXPT_P_PSUB_DEFAULT_BAND_NUM        ( 0 )
#define BXPT_P_PSUB_DEFAULT_DMA_PRIORITY    BXPT_PacketSubDmaPriority_eLow
#define BXPT_P_MAX_PSUB_OUTPUT_RATE         ( 1000000 )
#define BXPT_NUM_OUTPUT_RATE_REGISTER_VALUE ( 65535 )

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_packetsub );
#endif

static void BXPT_PacketSub_P_WriteReg(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr,
    uint32_t RegVal
    );

static uint32_t BXPT_PacketSub_P_ReadReg_isrsafe(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr
    );

void BXPT_PacketSub_P_WriteAddr(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr,
    BMMA_DeviceOffset RegVal
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Reg0Addr - BCHP_XPT_PSUB_PSUB0_CTRL0 + hPSub->BaseAddr;

    BREG_Write32( hPSub->hRegister, RegAddr, RegVal );
}

BMMA_DeviceOffset BXPT_PacketSub_P_ReadAddr_isrsafe(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Reg0Addr - BCHP_XPT_PSUB_PSUB0_CTRL0 + hPSub->BaseAddr;

    return BREG_ReadAddr( hPSub->hRegister, RegAddr );
}

#define BXPT_PacketSub_P_ReadReg BXPT_PacketSub_P_ReadReg_isrsafe
#define BXPT_PacketSub_P_ReadAddr BXPT_PacketSub_P_ReadAddr_isrsafe

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PacketSub_GetTotalChannels(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int *TotalChannels     /* [out] The number of PacketSub channels. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    *TotalChannels = hXpt->MaxPacketSubs;

    return( ExitCode );
}
#endif

BERR_Code BXPT_PacketSub_GetChannelDefaultSettings(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int ChannelNo,         /* [in] Which channel to get defaults from. */
    BXPT_PacketSub_ChannelSettings *ChannelSettings /* [out] The defaults */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ChannelSettings );

    if( ChannelNo >= hXpt->MaxPacketSubs )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        ChannelSettings->PacketLen = BXPT_P_PSUB_DEFAULT_PACKET_LEN;
        ChannelSettings->ForcedInsertionEn = false;
    }

    ChannelSettings->OutputRate = 0xBC * 1649; /* this will produce a register value of 0xBC the hardware default */

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_OpenChannel(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_PacketSub_Handle *hPSub,               /* [out] Handle for opened packet sub channel */
    unsigned int ChannelNo,                         /* [in] Which channel to open. */
    BXPT_PacketSub_ChannelSettings *ChannelSettings /* [in] The defaults to use */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;
    BXPT_PacketSub_Handle hLocal = ( BXPT_PacketSub_Handle ) NULL;
    uint32_t Reg;
    /*
    ** Use the address of the first register in the packet sub block as the
    ** base address of the entire block.
    */
    uint32_t BaseAddr = BCHP_XPT_PSUB_PSUB0_CTRL0 + ( ChannelNo * PACKET_SUB_REGISTER_STEP );

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( ChannelSettings );

    if( ChannelNo >= BXPT_NUM_PACKETSUBS )
    {
        /* Bad playback channel number. Complain. */
        BDBG_ERR(( "ChannelNo %lu is out of range!", ( unsigned long ) ChannelNo ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    if( hXpt->PacketSubHandles[ ChannelNo ].Opened )
    {
        BDBG_ERR(( "PacketSub channel %u already opened.", ChannelNo ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    if (!ChannelSettings->descBlock) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    /* Create the packet sub channel handle. */
    hLocal = &hXpt->PacketSubHandles[ ChannelNo ];
    hLocal->hChip = hXpt->hChip;
    hLocal->hRegister = hXpt->hRegister;
    hLocal->BaseAddr = BaseAddr;
    hLocal->ChannelNo = ChannelNo;
    hLocal->LastDescriptor_Cached = NULL;
    hLocal->Running = false;
    hLocal->vhXpt = (void *) hXpt;

    /* Do a sanity check on the defaults they passed in, then load them. */
    if( ChannelSettings->PacketLen > 255 )
    {
        BDBG_ERR(( "PacketLen %lu is out of range!. Clamped to 255.", ( unsigned long ) ChannelSettings->PacketLen ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        ChannelSettings->PacketLen = 255;
    }

    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_ePsub);

    /* Use their value for a default output rate.  Don't fail if this value is out of range */
    BXPT_PacketSub_SetOutputRate( hLocal, ChannelSettings->OutputRate );

    Reg = BXPT_PacketSub_P_ReadReg( hLocal, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, PACKET_LENGTH ) |
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, PACKET_LENGTH, ChannelSettings->PacketLen ) |
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN, ChannelSettings->ForcedInsertionEn == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hLocal, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    Reg = BXPT_PacketSub_P_ReadReg( hLocal, BCHP_XPT_PSUB_PSUB0_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL1, DATA_ENDIAN_CTRL )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL1, DATA_ENDIAN_CTRL, hXpt->IsLittleEndian == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hLocal, BCHP_XPT_PSUB_PSUB0_CTRL1, Reg );
    hLocal->Opened = true;

    hLocal->mma.block = ChannelSettings->descBlock;
    hLocal->mma.ptr = (uint8_t*)BMMA_Lock(ChannelSettings->descBlock) + ChannelSettings->descOffset;
    hLocal->mma.offset = BMMA_LockOffset(ChannelSettings->descBlock) + ChannelSettings->descOffset;

    *hPSub = hLocal;
    return( ExitCode );
}

void BXPT_PacketSub_CloseChannel(
    BXPT_PacketSub_Handle hPSub /* [in] Handle for the channel to close*/
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_STAT0, WAKE_MODE ) |
        BCHP_MASK( XPT_PSUB_PSUB0_STAT0, RUN ) |
        BCHP_MASK( XPT_PSUB_PSUB0_STAT0, WAKE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE_MODE, 0 ) |
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, RUN, 0 ) |
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE, 0 )
    );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );
    BXPT_P_ReleaseSubmodule(hPSub->vhXpt, BXPT_P_Submodule_ePsub);

    hPSub->Opened = false;
}


#if (!B_REFSW_MINIMAL)
unsigned int BXPT_PacketSub_GetPidChanNum(
    BXPT_PacketSub_Handle hPSub     /* [in] Handle for the channel. */
    )
{
    unsigned int PidChannelNum;
    BXPT_Handle hXpt;                           /* [in] Handle for this transport */

    BDBG_ASSERT( hPSub );

    hXpt = (BXPT_Handle) hPSub->vhXpt;
    BXPT_AllocPidChannel( hXpt, false, &PidChannelNum );

    return PidChannelNum;
}
#endif

BERR_Code BXPT_PacketSub_SetPidChanNum(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    unsigned int PidChanNum,        /* [in] Which PID channel to assign the output to. */
    unsigned int BandNum            /* [in] Which band number to assign the output to */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;
    bool IsPbBand = BXPT_P_IS_PB( BandNum );

    BDBG_ASSERT( hPSub );

    BXPT_P_CLEAR_PB_FLAG( BandNum );

    if( PidChanNum >= BXPT_NUM_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChanNum %lu is out of range!", ( unsigned long ) PidChanNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( !IsPbBand && BandNum > BXPT_NUM_PID_PARSERS )
    {
        BDBG_ERR(( "Input band BandNum %lu is out of range!", ( unsigned long ) BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if( IsPbBand && BandNum > BXPT_NUM_PLAYBACKS )
    {
        BDBG_ERR(( "Playback band BandNum %lu is out of range!", ( unsigned long ) BandNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        /* For playbacks, band numbers are mapped beginning at an architecture-specific offset. */
        BandNum = IsPbBand ? (BandNum + BXPT_PB_PARSER_BAND_BASE) : BandNum;

#ifdef BCHP_XPT_PSUB_PSUB0_CTRL1_OUTPUT_CH_NUM_MASK
        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
        Reg &= ~(
            BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, BAND_SEL )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, BAND_SEL, BandNum )
            );
        BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL1 );
        Reg &= ~(
            BCHP_MASK( XPT_PSUB_PSUB0_CTRL1, OUTPUT_CH_NUM )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL1, OUTPUT_CH_NUM, PidChanNum )
            );
        BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL1, Reg );
#else
        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
        Reg &= ~(
            BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, BAND_SEL ) |
            BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, OUTPUT_CH_NUM )
            );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, BAND_SEL, BandNum ) |
            BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, OUTPUT_CH_NUM, PidChanNum )
            );
        BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );
#endif
    }

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PacketSub_SetForcedOutput(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    bool Enable         /* [in] Force output immediately if TRUE */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, FORCED_OUTPUT_ENABLE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FORCED_OUTPUT_ENABLE, Enable == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    return( ExitCode );
}
#endif

BERR_Code BXPT_PacketSub_SetForcedInsertion(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    bool Enable         /* [in] Force output immediately if TRUE */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN, Enable == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_SetFullRateOutput(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    bool Enable         /* [in] Use full rate if TRUE */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, FULL_RATE_OUTPUT_ENABLE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FULL_RATE_OUTPUT_ENABLE, Enable == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_SetOutputRate(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t OutputRate /* [in] The output rate, in bits/second */
    )
{
    uint32_t Reg;
    uint32_t NewRate;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    /*
        See SW7420-879 for details. The time between each packet substitution (sec) = 0.912 / OUTPUT_RATE.
        We are given the insertion rate in (the OutputRate argument) bits/sec, and we need to solve for
        the OUTPUT_RATE register setting.
            packets/sec = OUTPUT_RATE/0.912
            bits/sec = (188*8) bits/packet * OUTPUT_RATE/0.912
            OUTPUT_RATE = bits/sec * 0.912/1504
            OUTPUT_RATE = OutputRate / 1649
    */

    if( OutputRate > BXPT_P_MAX_PSUB_OUTPUT_RATE )
    {
        BDBG_ERR(( "OutputRate %lu is out of range! Clamped to %lu",
            ( unsigned long ) OutputRate, ( unsigned long ) BXPT_P_MAX_PSUB_OUTPUT_RATE ));
        NewRate = 65535;    /* Max value this bitfield can hold */
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        NewRate = (uint32_t) OutputRate / 1649;
    }
    if( NewRate == 0 )
        NewRate = 1;    /* Handle round-down condition */

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL1 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL1, OUTPUT_RATE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL1, OUTPUT_RATE, NewRate )
        );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL1, Reg );

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_PauseChannel(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    bool Pause          /* [in] Pause channel if TRUE, continue if FALSE */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, PAUSE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, PAUSE, Pause == true ? 1 : 0 )
        );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_CreateDesc(
    BXPT_PacketSub_Handle hPSub,                /* [in] packetsub handle */
    BXPT_PacketSub_Descriptor * const Desc,     /* [in] Descriptor to initialize */
    BMMA_DeviceOffset BufferOffset,             /* [in] physical address of buffer */
    uint32_t BufferLength,                      /* [in] Size of buffer (in bytes). */
    bool IntEnable,                             /* [in] Interrupt when done? */
    BXPT_PacketSub_Descriptor * const NextDesc  /* [in] Next descriptor, or NULL */
    )
{
    BMMA_DeviceOffset ThisDescPhysicalAddr;
    BXPT_PacketSub_Descriptor *CachedDescPtr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT(hPSub);
    BDBG_ASSERT( Desc );
    BDBG_ASSERT(BufferOffset);

    /* Get the physical address for this buffer. Verify its on a 4-byte boundary*/
    if (BufferOffset % 4)
    {
        BDBG_ERR(( "Buffer is not 32-bit aligned!" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Verify that the buffer length is multiple of 4 bytes (i.e. a word). */
    if( BufferLength % 4 )
    {
        BDBG_ERR(( "BufferLength is not 32-bit aligned!" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Verify that the descriptor we're creating sits on a 16-byte boundary. */
    ThisDescPhysicalAddr = hPSub->mma.offset + (unsigned)((uint8_t*)Desc - (uint8_t*)hPSub->mma.ptr); /* convert Desc -> offset */
    BDBG_MSG(( "%s: Desc 0x%08lX -> Offset 0x%08lX", __FUNCTION__, (unsigned long) Desc, (unsigned long) ThisDescPhysicalAddr ));
    if( ThisDescPhysicalAddr % 16 )
    {
        BDBG_ERR(( "Desc is not 32-bit aligned!" ));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    CachedDescPtr = Desc;

    /* Load the descriptor's buffer address, length, and flags. */
    CachedDescPtr->BufferStartAddr = BufferOffset & 0xFFFFFFFF;
    CachedDescPtr->BufferLength = BufferLength;
#ifdef BXPT_PSUB_40BIT_SUPPORT
    CachedDescPtr->BufferStartAddrHi = BufferOffset >> 32;
#endif

    /* Clear everything, then set the ones we want below. */
    CachedDescPtr->Flags = 0;

    if( IntEnable == true )
        CachedDescPtr->Flags |= TRANS_DESC_INT_FLAG;

    /* Load the pointer to the next descriptor in the chain, if there is one. */
    if( NextDesc != 0 )
    {
        /* There is a another descriptor in the chain after this one. */
        BMMA_DeviceOffset NextDescPhysAddr;

        NextDescPhysAddr = hPSub->mma.offset + (unsigned)((uint8_t*)NextDesc - (uint8_t*)hPSub->mma.ptr); /* NextDesc -> offset */
        if( NextDescPhysAddr % 16 )
        {
            BDBG_ERR(( "NextDescDesc is not 32-bit aligned!" ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        }

        /* Next descriptor address must be 16-byte aligned. */
        NextDescPhysAddr &= ~( 0xF );
        CachedDescPtr->NextDescAddr = NextDescPhysAddr & 0xFFFFFFFF;
#ifdef BXPT_PSUB_40BIT_SUPPORT
        CachedDescPtr->NextDescAddrHi = NextDescPhysAddr >> 32;
#endif
    }
    else
    {
        /* There is NOT another descriptor. Set the Last Descriptor bit. */
        CachedDescPtr->NextDescAddr = TRANS_DESC_LAST_DESCR_IND;
    }

    BMMA_FlushCache(hPSub->mma.block, CachedDescPtr, sizeof(BXPT_PacketSub_Descriptor));
    return( ExitCode );
}

BERR_Code BXPT_PacketSub_AddDescriptors(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    BXPT_PacketSub_Descriptor *LastDesc,    /* [in] Last descriptor in new chain */
    BXPT_PacketSub_Descriptor *FirstDesc    /* [in] First descriptor in new chain */
    )
{
    uint32_t Reg;
    uint32_t RunBit;
    uint64_t DescPhysAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );
    BDBG_ASSERT( LastDesc );
    BDBG_ASSERT( FirstDesc );

    BDBG_MSG(("Adding Desc Addr 0x%08lX to Packet Sub Channel %d", ( unsigned long ) FirstDesc,
        hPSub->ChannelNo ));

    /* Do we already have a list going? */
    if( hPSub->LastDescriptor_Cached )
    {
        /*
        ** Yes, there is list already. Append this descriptor to the last descriptor,
        ** then set the wake bit.
        */
        BXPT_PacketSub_Descriptor *LastDescriptor_Cached = ( BXPT_PacketSub_Descriptor * ) hPSub->LastDescriptor_Cached;

        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
        RunBit = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, RUN );

        /* Set the last descriptor in the chain to point to the descriptor we're adding. */
        DescPhysAddr = hPSub->mma.offset + (unsigned)((uint8_t*)FirstDesc - (uint8_t*)hPSub->mma.ptr); /* convert FirstDesc -> offset */
        LastDescriptor_Cached->NextDescAddr = DescPhysAddr & 0xFFFFFFFF;
#ifdef BXPT_PSUB_40BIT_SUPPORT
        LastDescriptor_Cached->NextDescAddrHi = DescPhysAddr >> 32;
#endif
        BMMA_FlushCache(hPSub->mma.block, LastDescriptor_Cached, sizeof(BXPT_PacketSub_Descriptor));
        BDBG_MSG(( "%s (LastDescriptor_Cached): Desc %p -> Offset " BDBG_UINT64_FMT "", __FUNCTION__, (void *) FirstDesc, BDBG_UINT64_ARG(DescPhysAddr) ));

        /* Wake mode should resume from last descriptor on the list. */
        Reg &= ~ ( BCHP_MASK( XPT_PSUB_PSUB0_STAT0, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE_MODE, 0 );

        /* If the channel is running, we need to set the wake bit to let the hardware know we added a new buffer */
        if( RunBit )
        {
            /* PR 16985: Need to write 0 after writing a 1 */
            Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE, 1 );
            BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );

            Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE, 0 );
            BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );
        }
    }
    else
    {
        BMMA_DeviceOffset Addr;

        /*
        ** If this is the first descriptor (the channel has not been started)
        ** then load the address into the first descriptor register
        */

        /* This is our first descriptor, so we must load the first descriptor register */
        DescPhysAddr = hPSub->mma.offset + (unsigned)((uint8_t*)FirstDesc - (uint8_t*)hPSub->mma.ptr); /* convert FirstDesc -> offset */
        BDBG_MSG(( "%s (New chain): Desc %p -> Offset " BDBG_UINT64_FMT "", __FUNCTION__, (void *) FirstDesc, BDBG_UINT64_ARG(DescPhysAddr) ));

        /*
        ** The descriptor address field in the hardware register is wants the address
        ** in 16-byte blocks. See the RDB HTML for details. So, we must shift the
        ** address 4 bits to the right before writing it to the hardware. Note that
        ** the RDB macros will shift the value 4 bits to the left, since the address
        ** bitfield starts at bit 4. Confusing, but thats what the hardware and the
        ** RDB macros require to make this work.
        */
        Addr = BXPT_PacketSub_P_ReadAddr( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL2 );
        Addr &= ~( BCHP_MASK( XPT_PSUB_PSUB0_CTRL2, FIRST_DESC_ADDR ) );
        DescPhysAddr >>= 4;
        Addr |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL2, FIRST_DESC_ADDR, DescPhysAddr );
        BXPT_PacketSub_P_WriteAddr( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL2, Addr );

        /*
        ** If this channel has been started, we need to kick off the hardware
        ** by setting the RUN bit.
        */

        /* Wake up from the first descriptor on the list, ie re-read the FIRST_DESC_ADDR register. */
        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
        Reg &= ~ ( BCHP_MASK( XPT_PSUB_PSUB0_STAT0, WAKE_MODE ) );
        Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE_MODE, 1 );
        BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );

        if( hPSub->Running == true )
        {
            Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
            RunBit = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, RUN );

            if( RunBit )
            {
                /*
                ** Since the channel was already running in hardware, this means that we
                ** are reloading the first descriptor address due to the channel
                ** finishing before a new descriptor was added.  Therefore
                ** we use the wake bit (as we previously set the WAKE_MODE above.
                */
                Reg &= ~( BCHP_MASK( XPT_PSUB_PSUB0_STAT0, WAKE ) );
                Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, WAKE, 1 );
            }
            else
            {
                Reg &= ~( BCHP_MASK( XPT_PSUB_PSUB0_STAT0, RUN ) );
                Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, RUN, 1 );
            }

            BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );
        }
    }

    /* This descriptor is always the new last descriptor */
    hPSub->LastDescriptor_Cached = LastDesc;

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_GetCurrentDescriptorAddress_isrsafe(
    BXPT_PacketSub_Handle hPSub,            /* [in] Handle for the channel. */
    BXPT_PacketSub_Descriptor **LastDesc        /* [in] Address of the current descriptor. */
    )
{
    BMMA_DeviceOffset Reg, CurrentDescAddr;
    void *UserDescAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    Reg = BXPT_PacketSub_P_ReadAddr( hPSub, BCHP_XPT_PSUB_PSUB0_STAT1 );
    CurrentDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT1, CURR_DESC_ADDR );
    CurrentDescAddr <<= 4;  /* Convert to byte-address. */
    UserDescAddr = (uint8_t*)hPSub->mma.ptr + (CurrentDescAddr - hPSub->mma.offset); /* convert CurrentDescAddr -> cached ptr */
    *LastDesc = ( BXPT_PacketSub_Descriptor * ) UserDescAddr;

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PacketSub_CheckHeadDescriptor(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    BXPT_PacketSub_Descriptor *Desc,    /* [in] Descriptor to check. */
    bool *InUse,                        /* [out] Is descriptor in use? */
    uint32_t *BufferSize                /* [out] Size of the buffer (in bytes). */
    )
{
    BMMA_DeviceOffset Reg, CurrentDescAddr, CandidateDescPhysAddr;
    uint32_t ChanBusy;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    /*
    ** Check if the current descriptor being processed by the
    ** playback hardware is the first on our hardware list
    ** (which means this descriptor is still being used)
    */
    Reg = BXPT_PacketSub_P_ReadAddr( hPSub, BCHP_XPT_PSUB_PSUB0_STAT1 );

    CurrentDescAddr = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT1, CURR_DESC_ADDR );
    CurrentDescAddr <<= 4;  /* Convert to byte-address. */

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
    ChanBusy = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, BUSY );

    CandidateDescPhysAddr = hPSub->mma.offset + (unsigned)((uint8_t*)Desc - (uint8_t*)hPSub->mma.ptr); /* convert Desc -> offset */

    if( CurrentDescAddr == CandidateDescPhysAddr )
    {
        if( ChanBusy )
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
    }

    if( *InUse == false )
    {
        if( ChanBusy )
        {
            BXPT_PacketSub_Descriptor *CachedDescPtr = Desc;
            *BufferSize = CachedDescPtr->BufferLength;
        }
        else
        {
            /*
            ** Since there is valid data in the record channel even after it is stopped,
            ** we are unable to detect if we are done or not with a specific descriptor
            ** after the record channel has been halted.
            ** This check needs to be performed at a higher level
            */
            *BufferSize = 0;
            *InUse = true;
        }
    }
    else
    {
        *BufferSize = 0;
    }

    return( ExitCode );
}
#endif

BERR_Code BXPT_PacketSub_StartChannel(
    BXPT_PacketSub_Handle hPSub /* [in] Handle for the channel. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    BDBG_MSG(( "Starting Packet Sub channel %lu", ( unsigned long ) hPSub->ChannelNo ));

    if( hPSub->Running == true )
    {
        BDBG_ERR(( "Packet Sub channel %lu cannot be started because it's already running!",
            ( unsigned long ) hPSub->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_RUNNING );
    }

#ifdef BCHP_PWR_RESOURCE_XPT_PACKETSUB
    if( hPSub->Running == false )
    {
        BCHP_PWR_AcquireResource(hPSub->hChip, BCHP_PWR_RESOURCE_XPT_PACKETSUB);
    }
#endif

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, PSUB_ENABLE, 1 );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    /* Check if we have buffers already loaded for this channel */
    if( hPSub->LastDescriptor_Cached )
    {
        /* Since we already have some buffers loaded, we can start the pvr channel */
        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
        Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_STAT0, RUN, 1 );
        BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );
    }

    hPSub->Running = true;

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_StopChannel(
    BXPT_PacketSub_Handle hPSub /* [in] Handle for the channel. */
    )
{
    uint32_t Reg, ChanBusy, WaitCount, OldForcedInsertionState;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );

    BDBG_MSG(( "Stopping Packet Sub channel %lu", ( unsigned long ) hPSub->ChannelNo ));

    if( hPSub->Running == false )
    {
        BDBG_ERR(( "Packet Sub channel %lu cannot be stopped because it's not running!",
            ( unsigned long ) hPSub->ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_CHANNEL_ALREADY_STOPPED );
    }

    /* Forcing the issue should clear out the BUSY bit. */
    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    OldForcedInsertionState = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN );
    Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN, 1 );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    /* Stop the channel hardware */
    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
    Reg &= ~( BCHP_MASK( XPT_PSUB_PSUB0_STAT0, RUN ) );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0, Reg );

    WaitCount = 100;
    do
    {
        Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
        ChanBusy = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, BUSY );
        if( ChanBusy )
        {
            WaitCount--;
            if( !WaitCount )
            {
                BDBG_ERR(("Busy is still set when Packet Sub chan %lu has been stopped!",
                    ( unsigned long ) hPSub->ChannelNo ));
                return BERR_TRACE( BERR_TIMEOUT );
            }

            BKNI_Sleep( 1 );
        }
    }
    while( ChanBusy );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, PSUB_ENABLE ) |
        BCHP_MASK( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN )
    );
    Reg |= BCHP_FIELD_DATA( XPT_PSUB_PSUB0_CTRL0, FORCED_INSERTION_EN, OldForcedInsertionState );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL0, Reg );

    /* Clear the first desc addr (for cleaner debugging) */
    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL2 );
    Reg &= ~( BCHP_MASK( XPT_PSUB_PSUB0_CTRL2, FIRST_DESC_ADDR ) );
    BXPT_PacketSub_P_WriteReg( hPSub, BCHP_XPT_PSUB_PSUB0_CTRL2, Reg );

    hPSub->LastDescriptor_Cached = NULL;

#ifdef BCHP_PWR_RESOURCE_XPT_PACKETSUB
    if (hPSub->Running==true)
    {
        BCHP_PWR_ReleaseResource(hPSub->hChip, BCHP_PWR_RESOURCE_XPT_PACKETSUB);
    }
#endif

    hPSub->Running = false;

    return( ExitCode );
}

BERR_Code BXPT_PacketSub_GetChannelStatus_isrsafe(
    BXPT_PacketSub_Handle hPSub,            /* [in] Handle for the channel. */
    BXPT_PacketSub_ChannelStatus *Status    /* [out] Channel status. */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPSub );
    BDBG_ASSERT( Status );

    Reg = BXPT_PacketSub_P_ReadReg( hPSub, BCHP_XPT_PSUB_PSUB0_STAT0 );
    Status->Finished = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, FINISHED ) ? true : false;
    Status->Busy = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, BUSY ) ? true : false;
    Status->Run = BCHP_GET_FIELD_DATA( Reg, XPT_PSUB_PSUB0_STAT0, RUN ) ? true : false;

    return( ExitCode );
}

void BXPT_PacketSub_P_WriteReg(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr,
    uint32_t RegVal
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Reg0Addr - BCHP_XPT_PSUB_PSUB0_CTRL0 + hPSub->BaseAddr;

    BREG_Write32( hPSub->hRegister, RegAddr, RegVal );
}


uint32_t BXPT_PacketSub_P_ReadReg_isrsafe(
    BXPT_PacketSub_Handle hPSub,    /* [in] Handle for the channel. */
    uint32_t Reg0Addr
    )
{
    /*
    ** The address is the offset of the register from the beginning of the
    ** block, plus the base address of the block ( which changes from
    ** channel to channel ).
    */
    uint32_t RegAddr = Reg0Addr - BCHP_XPT_PSUB_PSUB0_CTRL0 + hPSub->BaseAddr;

    return( BREG_Read32( hPSub->hRegister, RegAddr ));
}

BERR_Code BXPT_PacketSub_GetEobIntId(
    BXPT_PacketSub_Handle hPSub,            /* [in] Handle for the channel. */
    BINT_Id *IntId
    )
{
    BDBG_ASSERT( hPSub );
    BDBG_ASSERT( IntId );

    switch( hPSub->ChannelNo )
    {
        case 0: *IntId = BCHP_INT_ID_PSUB0_EOB_INT; break;
        case 1: *IntId = BCHP_INT_ID_PSUB1_EOB_INT; break;
        case 2: *IntId = BCHP_INT_ID_PSUB2_EOB_INT; break;

#ifdef BCHP_INT_ID_PSUB3_EOB_INT
        case 3: *IntId = BCHP_INT_ID_PSUB3_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB4_EOB_INT
        case 4: *IntId = BCHP_INT_ID_PSUB4_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB5_EOB_INT
        case 5: *IntId = BCHP_INT_ID_PSUB5_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB6_EOB_INT
        case 6: *IntId = BCHP_INT_ID_PSUB6_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB7_EOB_INT
        case 7: *IntId = BCHP_INT_ID_PSUB7_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB8_EOB_INT
        case 8: *IntId = BCHP_INT_ID_PSUB8_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB9_EOB_INT
        case 9: *IntId = BCHP_INT_ID_PSUB9_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB10_EOB_INT
        case 10: *IntId = BCHP_INT_ID_PSUB10_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB11_EOB_INT
        case 11: *IntId = BCHP_INT_ID_PSUB11_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB12_EOB_INT
        case 12: *IntId = BCHP_INT_ID_PSUB12_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB13_EOB_INT
        case 13: *IntId = BCHP_INT_ID_PSUB13_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB14_EOB_INT
        case 14: *IntId = BCHP_INT_ID_PSUB14_EOB_INT; break;
#endif

#ifdef BCHP_INT_ID_PSUB15_EOB_INT
        case 15: *IntId = BCHP_INT_ID_PSUB15_EOB_INT; break;
#endif

        default:
            return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    return BERR_SUCCESS;
}

/* end of file */
