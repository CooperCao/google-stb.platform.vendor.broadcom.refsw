/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/

#include "bstd.h"
#include "bxpt.h"
#include "bxpt_directv.h"
#include "bdbg.h"
#include "bkni.h"
#include "bxpt_priv.h"

#include "bchp_xpt_fe.h"

#if BXPT_HAS_MESG_BUFFERS
#include "bchp_xpt_msg.h"
#endif

#if BXPT_HAS_PACKETSUB
#include "bchp_xpt_psub.h"
#include "bxpt_packetsub.h"
#endif

#define MSG_BUF_CTRL1_STEPSIZE  ( 4 )
#define PID_CHANNEL_STEPSIZE    ( 4 )
#define CAP_FILT_REG_STEPSIZE   ( 4 )
#define SPID_CHNL_STEPSIZE      ( 4 )
#define DEFAULT_PEAK_RATE       ( 25000000 )

BDBG_MODULE( xpt_directv );

#if BXPT_HAS_IB_PID_PARSERS

BERR_Code BXPT_DirecTv_SetParserBandMode(
    BXPT_Handle hXpt,       /* [Input] Handle for this transport */
    unsigned int Band,          /* [Input] Which parser band */
    BXPT_ParserMode Mode    /* [Input] Which mode (packet format) is being used. */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( Band >= hXpt->MaxPidParsers )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "Band %lu is out of range!", ( unsigned long ) Band ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        /* The parser config registers are at consecutive addresses. */
        RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, Band, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );

        Reg &= ~(
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PACKET_TYPE ) |
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE ) |
            BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PKT_LENGTH )
            );

        if( Mode == BXPT_ParserMode_eMpeg )
        {
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PACKET_TYPE, 0 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE, 2 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PKT_LENGTH, 188 )
            );
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }
        else if ( Mode == BXPT_ParserMode_eDirecTv )
        {
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PACKET_TYPE, 1 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE, 3 ) |
                BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PKT_LENGTH, 130 )
            );
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }
        else
        {
            /* Unsupported parser mode. Complain. */
            BDBG_ERR(( "Unsupported parser mode %u!", (unsigned) Mode ));
            ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    return( ExitCode );
}

#endif

#if BXPT_HAS_MESG_BUFFERS

void BXPT_DirecTv_SaveMptFlag(
    BXPT_Handle hXpt,       /* [in] Handle for this transport */
    bool Enable             /* [in] Enable or disable flag saving. */
    )
{
    uint32_t Reg, RegAddr;
    unsigned Index;

    BDBG_ASSERT( hXpt );
    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_eMsg);

    for( Index = 0; Index < BXPT_NUM_MESG_BUFFERS; Index++ )
    {
        RegAddr = BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE + ( Index * MSG_BUF_CTRL1_STEPSIZE );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        Reg &= ~( BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS ) );
        Reg |= ( BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS, Enable == true ? 1 : 0 ) );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );
    }

    BXPT_P_ReleaseSubmodule(hXpt, BXPT_P_Submodule_eMsg);
}

#if (!B_REFSW_MINIMAL)
void BXPT_DirecTv_SetPesStreamIdBoundaries(
    BXPT_Handle hXpt,       /* [in] Handle for this transport */
    unsigned int UpperId,       /* [in] The upper stream id. */
    unsigned int LowerId            /* [in] The lower stream id. */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1 );

    Reg &= ~(
        BCHP_MASK( XPT_MSG_PES_CTRL1, PES_STREAM_ID_HI ) |
        BCHP_MASK( XPT_MSG_PES_CTRL1, PES_STREAM_ID_LO )
        );

    Reg |= (
        BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PES_STREAM_ID_HI, UpperId ) |
        BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PES_STREAM_ID_LO, LowerId )
        );

    BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1, Reg );
}


void BXPT_DirecTv_SetStartcodeChecking(
    BXPT_Handle hXpt,       /* [in] Handle for this transport */
    bool EnableChecking     /* [in] Enable checking, or not. */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hXpt );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1 );

    Reg &= ~(
        BCHP_MASK( XPT_MSG_PES_CTRL1, PSC_CHECK_MODE )
        );

    Reg |= (
        BCHP_FIELD_DATA( XPT_MSG_PES_CTRL1, PSC_CHECK_MODE, EnableChecking == true ? 1 : 0  )
        );

    BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_PES_CTRL1, Reg );
}
#endif /* (!B_REFSW_MINIMAL) */

BERR_Code BXPT_Mesg_StartDirecTvMessageCaptureWithOptions(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    unsigned int PidChannelNum,                 /* [in] Which PID channel. */
    unsigned int MesgBufferNum,                 /* [in] Which Message Buffer. */
    BXPT_DirecTvMessageType MessageType,        /* [in] What type of DirecTV messages. */
    const BXPT_PsiMessageSettings *Settings,    /* [in] PID, band, and filters to use. */
    const BXPT_DirecTvMessageOptions *Options   /* [in] Additional options for message capture */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Settings );
    BDBG_ASSERT( Options );

    /* Sanity check on the arguments. */
    if( PidChannelNum > hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else if( hXpt->MesgBufferIsInitialized[ MesgBufferNum ] == false )
    {
        BDBG_ERR(( "Message buffer for this channel not configured!" ));
        ExitCode = BERR_TRACE( BXPT_ERR_MESG_BUFFER_NOT_CONFIGURED );
    }
    else if ( MesgBufferNum >= BXPT_NUM_MESG_BUFFERS ) {
        /* Bad Message Buffer number. Complain. */
        BDBG_ERR(( "Message Buffer Number %lu is out of range!", ( unsigned long ) MesgBufferNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else if ( hXpt->Pid2BuffMappingOn == true )
    {
        uint32_t Reg, RegAddr;
        unsigned int GroupSel, OutputMode, GenOffset, SpecialSel, SpecialType;
        BXPT_PidChannelDestination dest;

        ExitCode = BXPT_P_GetGroupSelect( Settings->Bank, &GroupSel );
        if( ExitCode != BERR_SUCCESS )
            goto Done;

        /* Enable the filters for this PID channel. */
        BREG_Write32( hXpt->hRegister, BCHP_XPT_MSG_GEN_FILT_EN_i_ARRAY_BASE + ( MesgBufferNum * GEN_FILT_EN_STEP ), Settings->FilterEnableMask );

        /* Configure the message buffer for capturing messages. */
        switch( MessageType )
        {
            case BXPT_DirecTvMessageType_eMpt:
            GenOffset = 4;
            OutputMode = 0x0A;
            SpecialSel = 0;
            SpecialType = 0;
            break;

            case BXPT_DirecTvMessageType_eAuxOnlyPackets:
            GenOffset = 3;
            OutputMode = 0x08;
            SpecialSel = 0;
            SpecialType = 0;
            break;

            case BXPT_DirecTvMessageType_eRegular_CapFilter0:
            case BXPT_DirecTvMessageType_eRegular_CapFilter1:
            case BXPT_DirecTvMessageType_eRegular_CapFilter2:
            case BXPT_DirecTvMessageType_eRegular_CapFilter3:
            case BXPT_DirecTvMessageType_eRegular_CapFilter4:
            GenOffset = 3;
            OutputMode = 0x09;

            /* Determine which CAP filter is to used. */
            SpecialSel = MessageType - BXPT_DirecTvMessageType_eRegular_CapFilter0;
            SpecialType = 2;
            break;

            /* Regular mode is also the default case. */
            default:
            case BXPT_DirecTvMessageType_eRegular:
            GenOffset = 3;
            OutputMode = 0x09;
            SpecialSel = 0;
            SpecialType = 0;
            break;
        }

        /* Select the bank the filters are in. */
        RegAddr = BCHP_XPT_MSG_BUF_CTRL2_TABLE_i_ARRAY_BASE + ( MesgBufferNum * PID_CTRL2_TABLE_STEP );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, GEN_OFFSET ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, SKIP_BYTE2 ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, GEN_GRP_SEL ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL2_TABLE_i, FILTER_MODE )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, GEN_OFFSET, GenOffset ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SPECIAL_TYPE, SpecialType ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, SKIP_BYTE2, 1 ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, GEN_GRP_SEL, GroupSel ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL2_TABLE_i, FILTER_MODE, 0 )
        );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        RegAddr = BCHP_XPT_MSG_BUF_CTRL1_TABLE_i_ARRAY_BASE + ( MesgBufferNum * PID_CTRL1_TABLE_STEP );
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, ERR_CK_MODE ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, ERR_CK_DIS ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, SPECIAL_SEL ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, ALIGN_MODE ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS ) |
            BCHP_MASK( XPT_MSG_BUF_CTRL1_TABLE_i, DATA_OUTPUT_MODE )
        );

        /* Configure the message buffer for capturing PSI messages. */
        Reg |= (
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, SPECIAL_SEL, SpecialSel ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, ALIGN_MODE, Settings->ByteAlign == true ? 0 : 1 ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, DIRECTV_SAVE_FLAGS, Options->Flags ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, DATA_OUTPUT_MODE, OutputMode )
        );

        /* CRC checks can be disabled on a per-PID channel basis. Do this here if requested in the settings struct. */
        Reg |= (
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, ERR_CK_MODE, 0 ) |
            BCHP_FIELD_DATA( XPT_MSG_BUF_CTRL1_TABLE_i, ERR_CK_DIS, Settings->CrcDisable == true ? 1 : 0 )
        );
        BREG_Write32( hXpt->hRegister, RegAddr, Reg );

        /* Enable data from the PID channel to the mesg block. */
        dest = BXPT_PidChannelDestination_eMessageGPipe;
        BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, dest, true );
        hXpt->mesgBufferDestination[MesgBufferNum] = dest;

        if (hXpt->PidChannelTable[ PidChannelNum ].destRefCnt[dest] == 1) {
            /* Set this PID channel as allocated, in case they forced the channel assignment. */
            hXpt->PidChannelTable[ PidChannelNum ].IsAllocated = true;

            /* Enable data from the PID channel to the mesg block. */
            /* Configure the PID channel. */
            ExitCode = BXPT_ConfigurePidChannel( hXpt, PidChannelNum, Settings->Pid, Settings->Band );
            if( ExitCode != BERR_SUCCESS ) {
                BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, dest, false );
                goto Done;
            }
        }

        ConfigPid2BufferMap( hXpt, PidChannelNum, MesgBufferNum, true);

        /* Enable the PID channel. */
#if BXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL
        /* do nothing. must be explicitly enabled. */
#else
        if (hXpt->PidChannelTable[ PidChannelNum ].destRefCnt[dest] == 1)
        {
            ExitCode = BERR_TRACE( BXPT_EnablePidChannel( hXpt, PidChannelNum ) );
            if (ExitCode != BERR_SUCCESS) {
                BXPT_P_SetPidChannelDestination( hXpt, PidChannelNum, dest, false );
                goto Done;
            }
        }
#endif
    }
    else
    {
        /* Wrong function call. Complain. */
        BDBG_ERR(( "Pid2BuffMapping is OFF. You might need to call BXPT_SetPid2Buff() to turn it ON" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    Done:
    return( ExitCode );
}


BERR_Code BXPT_Mesg_StopDirecTvMessageCapture(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned int ScidChannelNum,        /* [in] Which SCID channel. */
    unsigned int MesgBufferNum     /* [in] Which Message Buffer. */
    )
{
    BDBG_ASSERT( hXpt );

    return( BERR_TRACE(BXPT_Mesg_StopPidChannelRecord( hXpt, ScidChannelNum, MesgBufferNum )) );
}


BERR_Code BXPT_DirecTv_SetCapPattern(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned AddressFilter,     /* [in] Which address filter gets the pattern. */
    uint32_t Pattern            /* [in] The pattern to load. */
    )
{
    uint32_t RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( AddressFilter > BXPT_NUM_CAP_FILTERS )
    {
        BDBG_ERR(( "AddressFilter %lu is out of range!", ( unsigned long ) AddressFilter ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
    }
    else
    {
        RegAddr = BCHP_XPT_MSG_C_MATCH0 + ( AddressFilter * CAP_FILT_REG_STEPSIZE );
        BREG_Write32( hXpt->hRegister, RegAddr, Pattern );
    }

    return( ExitCode );
}

#endif

BERR_Code BXPT_DirecTv_ConfigHdFiltering(
    BXPT_Handle hXpt,                   /* [in] Handle for this transport */
    unsigned int PidChannelNum,         /* [in] Which PID channel. */
    bool EnableFilter,                  /* [in] Which SCID channel. */
    BXPT_HdFilterMode FilterMode        /* [in] HD values to filter on. Ignored if EnableFilter == false */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Sanity check on the arguments. */
    if( PidChannelNum >= hXpt->MaxPidChannels )
    {
        /* Bad parser band number. Complain. */
        BDBG_ERR(( "PidChannelNum %lu is out of range!", ( unsigned long ) PidChannelNum ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        uint32_t Reg;
        unsigned OldHdEn = 255;
        unsigned OldFilterMode = 255;

        uint32_t RegAddr = BCHP_XPT_FE_PID_TABLE_i_ARRAY_BASE + ( PID_CHANNEL_STEPSIZE * PidChannelNum );

#ifdef ENABLE_PLAYBACK_MUX
        /*gain access to the pid table*/
        BKNI_EnterCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/

        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        OldHdEn = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER );
        OldFilterMode = BCHP_GET_FIELD_DATA( Reg, XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE );

        if( EnableFilter == true )
        {
            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER ) |
                BCHP_MASK( XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE )
                );
            Reg |= (
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER, 1 ) |
                BCHP_FIELD_DATA( XPT_FE_PID_TABLE_i, HD_FILT_EN_HD_FILTER_TYPE, FilterMode )
                );
        }
        else
        {
            Reg &= ~(
                BCHP_MASK( XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER )
                );
        }

        if( EnableFilter != OldHdEn || FilterMode != OldFilterMode )
        {
            BREG_Write32( hXpt->hRegister, RegAddr, Reg );
        }

#ifdef ENABLE_PLAYBACK_MUX
        /*leave pid table protected area*/
        BKNI_LeaveCriticalSection();
#endif /*ENABLE_PLAYBACK_MUX*/
    }

    return( ExitCode );
}


/* end of file */
