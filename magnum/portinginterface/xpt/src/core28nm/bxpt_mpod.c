 /***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bxpt_priv.h"
#include "bxpt_mpod.h"

#include "bchp_xpt_mpod.h"
#include "bchp_xpt_fe.h"
#include "bchp_xpt_bus_if.h"
#include "bchp_xpt_pmu.h"

#include "bchp_xpt_mcpb_ch0.h"
#include "bchp_xpt_mcpb_ch1.h"

#define PB_PARSER_STEP      ( BCHP_XPT_MCPB_CH1_DMA_DESC_CONTROL - BCHP_XPT_MCPB_CH0_DMA_DESC_CONTROL )
#define IB_PARSER_STEP      ( BCHP_XPT_FE_MINI_PID_PARSER1_CTRL1 - BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 )

#if( BDBG_DEBUG_BUILD == 1 )
    BDBG_MODULE( xpt_mpod );
#endif

static BERR_Code SetOnlineState( BXPT_Handle hXpt, bool Online );

void BXPT_Mpod_GetDefaultConfig(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_Mpod_Config *Config
    )
{
    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Config );

    BSTD_UNUSED( hXpt );

    BKNI_Memset( Config, 0, sizeof( BXPT_Mpod_Config ));
    Config->ByteSync = 1;
    Config->ClkNrun = 0;
    Config->InvertClk = 0;
    Config->NshiftClk = 0;
    Config->OutputInvertSync = 0;
    Config->InputInvertSync = 0;

    Config->Loopback = false;

    Config->SmodeEn = false;
    Config->HostRsvd = 0;
    Config->HostRsvdEn = false;
    Config->ClkDelay = 0;
    Config->OutputInvertValid = false;
    Config->InputInvertValid = false;
    Config->InputInvertClk = false;

    Config->BandNo = 0;
    Config->PbBand = false;
    Config->BandEn = false;
    Config->TimestampInsertEn = false;

    Config->ParallelEn = false;

    /* Default for S-Card is 9 MHz. Not all chips support programmable output clocks. */
    Config->OutputClockRate = BXPT_Mpod_OutputClockRate_e108;
    Config->OutputClockDivider = BXPT_Mpod_OutputClockDivider_e12;
}

BERR_Code BXPT_Mpod_Init(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    const BXPT_Mpod_Config *Config
    )
{
    uint32_t Reg;
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Config );

    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_eMpod);

    /* MPOD Configuration Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_CFG );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_CFG, CRC_INIT_VALUE ) |
        BCHP_MASK( XPT_MPOD_CFG, SMODE_EN ) |
        BCHP_MASK( XPT_MPOD_CFG, MPOD_EXT_EN ) |
        BCHP_MASK( XPT_MPOD_CFG, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_CFG, CRC_INIT_VALUE, 0xFF ) |
        BCHP_FIELD_DATA( XPT_MPOD_CFG, SMODE_EN, Config->SmodeEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_EXT_EN, Config->Loopback == true ? 0 : 1 ) |
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_EN, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_CFG, Reg );


    /* Output Interface Formatter Control Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_OCTRL, HOST_RSVD ) |
        BCHP_MASK( XPT_MPOD_OCTRL, CLK_DELAY ) |
        BCHP_MASK( XPT_MPOD_OCTRL, HOST_RSVD_EN ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_VALID ) |
        BCHP_MASK( XPT_MPOD_OCTRL, BYTE_SYNC ) |
        BCHP_MASK( XPT_MPOD_OCTRL, CLK_NRUN ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_CLK ) |
        BCHP_MASK( XPT_MPOD_OCTRL, NSHIFT_CLK ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_SYNC ) |
        BCHP_MASK( XPT_MPOD_OCTRL, MUTE ) |
        BCHP_MASK( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, HOST_RSVD, Config->HostRsvd ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, CLK_DELAY, Config->ClkDelay ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, HOST_RSVD_EN, Config->HostRsvdEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_VALID, Config->OutputInvertValid ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, BYTE_SYNC, Config->ByteSync ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, CLK_NRUN, Config->ClkNrun ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_CLK, Config->InvertClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, NSHIFT_CLK, Config->NshiftClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_SYNC, Config->OutputInvertSync ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, MUTE, 0 ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL, Reg );

    if( Config->OutputClockRate >= BXPT_Mpod_OutputClockRate_eIb0 && BXPT_P_InputBandIsSupported( Config->OutputClockRate ) == false )
    {
        BDBG_ERR(( "Unsupported input band %d", Config->OutputClockRate - BXPT_Mpod_OutputClockRate_eIb0 ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* MPOD_PKT_DLY_CNT moved on some chips. */
#ifdef BCHP_XPT_MPOD_OCTRL2_MPOD_PKT_DLY_CNT_MASK
    {
        BXPT_Mpod_OutputClockRate OutputClockRate;
        BXPT_Mpod_OutputClockDivider OutputClockDivider;

/* For chips that support 200 Mbps Mcards, remove the restriction on the output rate and clock divider. */
#ifndef BXPT_HAS_200Mbps_CABLECARD_SUPPORT
        if( Config->SmodeEn == false )
        {
            /* The MCard spec requires 27 MHz output clock, so override the user's request if we're in MCard mode. */
            OutputClockRate = BXPT_Mpod_OutputClockRate_e108;
            OutputClockDivider = BXPT_Mpod_OutputClockDivider_e4;
        }
        else
#endif
        {
            OutputClockRate = Config->OutputClockRate;
            OutputClockDivider = Config->OutputClockDivider;
        }

        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL2 );
        Reg &= ~(
            BCHP_MASK( XPT_MPOD_OCTRL2, MPOD_CLK_SEL ) |
            BCHP_MASK( XPT_MPOD_OCTRL2, MPOD_CLK_DIV_SEL ) |
            BCHP_MASK( XPT_MPOD_OCTRL2, MPOD_PKT_DLY_CNT )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_MPOD_OCTRL2, MPOD_CLK_SEL, OutputClockRate ) |
            BCHP_FIELD_DATA( XPT_MPOD_OCTRL2, MPOD_CLK_DIV_SEL, OutputClockDivider ) |
            BCHP_FIELD_DATA( XPT_MPOD_OCTRL2, MPOD_PKT_DLY_CNT, Config->OutputPacketDelayCount )
        );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL2, Reg );
    }
#else
    /* Warning for chips that do not have programmable output packet. Just don't change the default value. */
    if( Config->OutputPacketDelayCount )
    {
        BDBG_ERR(( "OutputPacketDelayCount is not supported on this chip." ));
        return BERR_NOT_SUPPORTED;
    }
#endif

    /* Input Interface Formatter Control Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_VALID ) |
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_CLK ) |
        BCHP_MASK( XPT_MPOD_ICTRL, BAND_NO ) |
        BCHP_MASK( XPT_MPOD_ICTRL, PB_BAND ) |
        BCHP_MASK( XPT_MPOD_ICTRL, BAND_EN ) |
        BCHP_MASK( XPT_MPOD_ICTRL, TIMESTAMP_INSERT_EN ) |
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_SYNC ) |
        BCHP_MASK( XPT_MPOD_ICTRL, MUTE ) |
        BCHP_MASK( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INVERT_VALID, Config->InputInvertValid ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INVERT_CLK, Config->InputInvertClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, BAND_NO, Config->BandNo ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, PB_BAND, Config->PbBand ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, BAND_EN, Config->BandEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, TIMESTAMP_INSERT_EN, Config->TimestampInsertEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INVERT_SYNC, Config->InputInvertSync ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, MUTE, 0 ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL, Reg );

    /* All parsers *bypass* the interface */
    for( ii = 0; ii < hXpt->MaxPidParsers; ii++ )
    {
        BXPT_Mpod_RouteToMpod( hXpt, BXPT_ParserType_eIb, ii, false );
        BXPT_Mpod_AllPass( hXpt, BXPT_ParserType_eIb, ii, false );
    }
    for( ii = 0; ii < hXpt->MaxPlaybacks; ii++ )
    {
        BXPT_Mpod_RouteToMpod( hXpt, BXPT_ParserType_ePb, ii, false );
        BXPT_Mpod_AllPass( hXpt, BXPT_ParserType_ePb, ii, false );
    }

    /* Enable the interface. */
    ExitCode = SetOnlineState( hXpt, true );
    Done:
    return( ExitCode );
}

BERR_Code BXPT_Mpod_Shutdown(
    BXPT_Handle hXpt                            /* [in] Handle for this transport */
    )
{
    unsigned ii;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    /* Disable the interface. */
    SetOnlineState( hXpt, false );

    /* All parsers *bypass* the interface */
    for( ii = 0; ii < hXpt->MaxPidParsers; ii++ )
    {
        BXPT_Mpod_RouteToMpod( hXpt, BXPT_ParserType_eIb, ii, false );
        BXPT_Mpod_AllPass( hXpt, BXPT_ParserType_eIb, ii, false );
    }
    for( ii = 0; ii < hXpt->MaxPlaybacks; ii++ )
    {
        BXPT_Mpod_RouteToMpod( hXpt, BXPT_ParserType_ePb, ii, false );
        BXPT_Mpod_AllPass( hXpt, BXPT_ParserType_ePb, ii, false );
    }

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_CFG );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_CFG, Reg & 0xfffffff8);

    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL, 0);
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL, 0);

    BXPT_P_ReleaseSubmodule(hXpt, BXPT_P_Submodule_eMpod);
    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
unsigned int BXPT_Mpod_GetPodRes(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    )
{
    unsigned int Reg;

    BDBG_ASSERT( hXpt );

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_RES_FIELD );
    return BCHP_GET_FIELD_DATA( Reg, XPT_MPOD_RES_FIELD, POD_RES );
}
#endif

BERR_Code RouteMultichannelPlaybackToMpod(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    )
{
    uint32_t RegAddr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    RegAddr = BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL + ParserNum * PB_PARSER_STEP;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, MPOD_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, MPOD_EN, Enable == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    hXpt->PlaybackHandles[ ParserNum ].IsParserInAllPass = Enable;

    return ExitCode;
}

BERR_Code BXPT_Mpod_RouteToMpod(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    )
{
    uint32_t Ctrl1Addr, Ctrl2Addr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( BXPT_ParserType_ePb == ParserType )
    {
        return RouteMultichannelPlaybackToMpod( hXpt, ParserNum, Enable );
    }

    Ctrl1Addr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
    Ctrl2Addr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 );
    if( !Ctrl1Addr || !Ctrl2Addr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    Reg = BREG_Read32( hXpt->hRegister, Ctrl1Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl1Addr, Reg );

    hXpt->IsParserInAllPass[ ParserNum ] = Enable;

    Reg = BREG_Read32( hXpt->hRegister, Ctrl2Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_EN, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl2Addr, Reg );

    Done:
    return ExitCode;
}

BERR_Code RouteMultichannelPlaybackToMpodPidFiltered(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool MpodPidFilter,         /* [in] enable pid filtering prior to the MCARD */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    )
{
    uint32_t RegAddr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    RegAddr = BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL + ParserNum * PB_PARSER_STEP;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, MPOD_EN ) |
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, MPOD_EN, Enable == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_PRE_MPOD, ((false == MpodPidFilter) && Enable) ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    hXpt->PlaybackHandles[ ParserNum ].IsParserInAllPass = ((false == MpodPidFilter) && Enable);

    return ExitCode;
}

BERR_Code BXPT_Mpod_RouteToMpodPidFiltered(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool MpodPidFilter,         /* [in] enable pid filtering prior to the MCARD */
    bool ContinuityCountCheck,  /* [in] enable CC checking after the MCARD */
    bool Enable                 /* [in] Route data to the MPOD interface if true */
    )
{
    uint32_t Ctrl1Addr, Ctrl2Addr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    BSTD_UNUSED( ContinuityCountCheck );
    BDBG_ERR(( "ContinuityCountCheck is not supported. Control CC checking through the PID channel API" ));

    if( BXPT_ParserType_ePb == ParserType )
    {
        return RouteMultichannelPlaybackToMpodPidFiltered( hXpt, ParserNum, MpodPidFilter, Enable );
    }

    Ctrl1Addr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL1 );
    Ctrl2Addr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 );
    if( !Ctrl1Addr || !Ctrl2Addr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* This works since PB and FE parser control reg 2 are identical */
    Reg = BREG_Read32( hXpt->hRegister, Ctrl1Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD, ((MpodPidFilter==false) && Enable) ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl1Addr, Reg );

    hXpt->IsParserInAllPass[ ParserNum ] = ((MpodPidFilter==false) && Enable);

    Reg = BREG_Read32( hXpt->hRegister, Ctrl2Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL2, MPOD_EN, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl2Addr, Reg );

    Done:
    return ExitCode;
}

BERR_Code MultichannelPlaybackAllPassToMpod(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] All pass mode enabled if true, disabled if false */
    )
{
    uint32_t RegAddr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    RegAddr = BCHP_XPT_MCPB_CH0_SP_PARSER_CTRL + ParserNum * PB_PARSER_STEP;
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_POST_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MCPB_CH0_SP_PARSER_CTRL, PARSER_ALL_PASS_CTRL_POST_MPOD, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    return ExitCode;
}

BERR_Code BXPT_Mpod_AllPass(
    BXPT_Handle hXpt,           /* [in] Handle for this instance of transport. */
    BXPT_ParserType ParserType, /* [in] Playback or front-end parser */
    unsigned ParserNum,         /* [in] Which parser to get data from */
    bool Enable                 /* [in] All pass mode enabled if true, disabled if false */
    )
{
    uint32_t RegAddr, Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    if( BXPT_ParserType_ePb == ParserType )
    {
        return MultichannelPlaybackAllPassToMpod( hXpt, ParserNum, Enable );
    }

    RegAddr = BXPT_P_GetParserCtrlRegAddr( hXpt, ParserNum, BCHP_XPT_FE_MINI_PID_PARSER0_CTRL2 );
    if( !RegAddr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* This works since PB and FE parser control reg 2 are identical */
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_MINI_PID_PARSER0_CTRL2, PARSER_ALL_PASS_CTRL_POST_MPOD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_MINI_PID_PARSER0_CTRL2, PARSER_ALL_PASS_CTRL_POST_MPOD, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    Done:
    return ExitCode;
}

BERR_Code SetOnlineState(
    BXPT_Handle hXpt,             /* [in] Handle for this transport */
    bool Online                   /* [in] true if MPOD should be online, false otherwise */
    )
{
    int Enable;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    Enable = Online == true ? 1 : 0;

    /* MPOD Configuration Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_CFG );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_CFG, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_EN, Enable )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_CFG, Reg );

    /* MPOD output interface register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN, Enable )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL, Reg );

    /* MPOD input interface register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN, Enable )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL, Reg );

    return( ExitCode );
}


