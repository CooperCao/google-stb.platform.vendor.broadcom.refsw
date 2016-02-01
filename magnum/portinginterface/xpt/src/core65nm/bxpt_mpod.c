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
#include "bchp_xpt_pb0.h"
#include "bchp_xpt_bus_if.h"

#if ( BCHP_CHIP == 3563 )
    /* 3563 only has 1 PB */
    #define PB_PARSER_STEP      ( 0x44 )
#else
    #include "bchp_xpt_pb1.h"
    #define PB_PARSER_STEP      ( BCHP_XPT_PB1_PARSER_CTRL2 - BCHP_XPT_PB0_PARSER_CTRL2 )
#endif

#define IB_PARSER_STEP      ( BCHP_XPT_FE_PARSER1_CTRL2 - BCHP_XPT_FE_PARSER0_CTRL2 )

BDBG_MODULE( xpt_mpod );


static BERR_Code SetOnlineState( BXPT_Handle hXpt, bool Online, bool SkipInputFormatter );
static uint32_t GetParserCtrl1Addr( BXPT_Handle hXpt, BXPT_ParserType ParserType, unsigned ParserNum );
static uint32_t GetParserCtrl2Addr( BXPT_Handle hXpt, BXPT_ParserType ParserType, unsigned ParserNum );

void BXPT_Mpod_GetDefaultConfig(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    BXPT_Mpod_Config *Config
    )
{
    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
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

#if BXPT_HAS_MPOD_SCARD_SUPPORT
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
#else
    Config->OutputDataSwap = 0;
    Config->OutputDataLsbFirst = 0;

    Config->CrcDropPacket = 1;
    Config->SyncOnPosEdge = 0;
    Config->InputDataSwap = 0;
    Config->InputDataLsbFirst = 0;
#endif

#if BXPT_HAS_MPOD_CARDBUS_SUPPORT 
    Config->CardBusModeEn = false;
#endif

#if BXPT_HAS_PARALLEL_MPOD 
    Config->ParallelEn = false;
#endif

#if BXPT_HAS_MPOD_MCLK_SELECT
    Config->Pod2ChipMClkMuxSelect = false;
#endif

    Config->SkipInputFormatter = false;

#if BXPT_HAS_PROGRAMMABLE_MPOD_CLOCK
    Config->PacketOutputClock = BXPT_Mpod_OutputClock_e108;
    Config->PacketOutputDelay = 0;
#endif
}

BERR_Code BXPT_Mpod_Init(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    const BXPT_Mpod_Config *Config
    )
{
    uint32_t Reg;
    unsigned ii;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);
    BDBG_ASSERT( Config );

    /* MPOD Configuration Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_CFG );
    Reg &= ~(
#if BXPT_HAS_PARALLEL_MPOD
        BCHP_MASK( XPT_MPOD_CFG, MPOD_PARALLEL_ENABLE ) |
#endif
        BCHP_MASK( XPT_MPOD_CFG, CRC_INIT_VALUE ) |
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_MASK( XPT_MPOD_CFG, SMODE_EN ) |
#else
        BCHP_MASK( XPT_MPOD_CFG, CRC_DROP_PACKET ) |
#endif
        BCHP_MASK( XPT_MPOD_CFG, MPOD_EXT_EN ) |
        BCHP_MASK( XPT_MPOD_CFG, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_CFG, CRC_INIT_VALUE, 0xFF ) |
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_FIELD_DATA( XPT_MPOD_CFG, SMODE_EN, Config->SmodeEn ) |
#else
        BCHP_FIELD_DATA( XPT_MPOD_CFG, CRC_DROP_PACKET, Config->CrcDropPacket ) |
#endif
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_EXT_EN, Config->Loopback == true ? 0 : 1 ) |
#if BXPT_HAS_PARALLEL_MPOD
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_PARALLEL_ENABLE, Config->ParallelEn == true ? 1 : 0 ) |
#endif
        BCHP_FIELD_DATA( XPT_MPOD_CFG, MPOD_EN, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_CFG, Reg );

    /* MPOD Pad Cable Card Control Register For 7413 */
#if (( BCHP_CHIP == 7405 ) && ( BCHP_VER >= BCHP_VER_C0 ))
if (!Config->SmodeEn)
{
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_PAD_CC_CTRL);
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MOCLK ) |
        BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MICLK ) |
        BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDO ) |
        BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDI )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MOCLK, 1 ) |
        BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MICLK, 1 ) |
        BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDO, 0 ) |
        BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDI, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_PAD_CC_CTRL, Reg );
}
#endif

    /* Output Interface Formatter Control Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL );
    Reg &= ~(
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_MASK( XPT_MPOD_OCTRL, HOST_RSVD ) |
        BCHP_MASK( XPT_MPOD_OCTRL, CLK_DELAY ) |
        BCHP_MASK( XPT_MPOD_OCTRL, HOST_RSVD_EN ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_VALID ) |
#else
        BCHP_MASK( XPT_MPOD_OCTRL, DATA_SWAP ) |
#endif
        BCHP_MASK( XPT_MPOD_OCTRL, BYTE_SYNC ) |
        BCHP_MASK( XPT_MPOD_OCTRL, CLK_NRUN ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_CLK ) |
        BCHP_MASK( XPT_MPOD_OCTRL, NSHIFT_CLK ) |
        BCHP_MASK( XPT_MPOD_OCTRL, INVERT_SYNC ) |
        BCHP_MASK( XPT_MPOD_OCTRL, MUTE ) |
        BCHP_MASK( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN )
    );
    Reg |= (
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, HOST_RSVD, Config->HostRsvd ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, CLK_DELAY, Config->ClkDelay ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, HOST_RSVD_EN, Config->HostRsvdEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_VALID, Config->OutputInvertValid ) |
#else
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, DATA_SWAP, Config->OutputDataSwap ) |
#endif
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, BYTE_SYNC, Config->ByteSync ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, CLK_NRUN, Config->ClkNrun ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_CLK, Config->InvertClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, NSHIFT_CLK, Config->NshiftClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, INVERT_SYNC, Config->OutputInvertSync ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, MUTE, 0 ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL, OUTPUT_FORMATTER_EN, 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL, Reg );

    /* Input Interface Formatter Control Register */
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL );
    Reg &= ~(
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_VALID ) |
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_CLK ) |
        BCHP_MASK( XPT_MPOD_ICTRL, BAND_NO ) |
        BCHP_MASK( XPT_MPOD_ICTRL, PB_BAND ) |
        BCHP_MASK( XPT_MPOD_ICTRL, BAND_EN ) |
        BCHP_MASK( XPT_MPOD_ICTRL, TIMESTAMP_INSERT_EN ) |
#elif ( BCHP_CHIP == 7325 ) || ( BCHP_CHIP == 7335 ) || ( BCHP_CHIP == 7336  )  || ( BCHP_CHIP == 7342 )  || ( BCHP_CHIP == 7125 ) || (BCHP_CHIP == 7340 )
    /* These chips don't have this feature */
#else
        BCHP_MASK( XPT_MPOD_ICTRL, SYNC_ON_POSEDGE ) |
        BCHP_MASK( XPT_MPOD_ICTRL, DATA_SWAP ) |
        BCHP_MASK( XPT_MPOD_ICTRL, DATA_LSB_FIRST ) |
#endif
        BCHP_MASK( XPT_MPOD_ICTRL, INVERT_SYNC ) |
        BCHP_MASK( XPT_MPOD_ICTRL, MUTE ) |
        BCHP_MASK( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN )
    );
    Reg |= (
#if BXPT_HAS_MPOD_SCARD_SUPPORT
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INVERT_VALID, Config->InputInvertValid ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INVERT_CLK, Config->InputInvertClk ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, BAND_NO, Config->BandNo ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, PB_BAND, Config->PbBand ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, BAND_EN, Config->BandEn ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, TIMESTAMP_INSERT_EN, Config->TimestampInsertEn ) |
#else
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, SYNC_ON_POSEDGE, Config->SyncOnPosEdge ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, DATA_SWAP, Config->InputDataSwap ) |
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, DATA_LSB_FIRST, Config->InputDataLsbFirst ) |
#endif
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

    /* set PAD type to CableCard */
#if BXPT_HAS_MPOD_CARDBUS_SUPPORT 
    if( Config->CardBusModeEn == true )
    {
        Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_PAD_CC_CTRL );
        Reg &= ~(
            BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MOCLK ) |
            BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MICLK ) |
            BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDO ) |
            BCHP_MASK( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDI )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MOCLK, 1 ) |
            BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MICLK, 1 ) |
            BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDO, 1 ) |
            BCHP_FIELD_DATA( XPT_MPOD_PAD_CC_CTRL, PAD_CC_CTRL_MDI, 1 )
        );
        BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_PAD_CC_CTRL, Reg );
    }
#endif
  
#if BXPT_HAS_MPOD_MCLK_SELECT
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_BUS_IF_MISC_CTRL0, POD2CHIP_MCLKI_MUX_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_BUS_IF_MISC_CTRL0, POD2CHIP_MCLKI_MUX_SEL, Config->Pod2ChipMClkMuxSelect == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0, Reg );
#endif

#if BXPT_HAS_PROGRAMMABLE_MPOD_CLOCK
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL2 );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_OCTRL2, MPOD_PKT_DLY_CNT ) |
        BCHP_MASK( XPT_MPOD_OCTRL2, MPOD_CLK_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL2, MPOD_PKT_DLY_CNT, Config->PacketOutputDelay ) |
        BCHP_FIELD_DATA( XPT_MPOD_OCTRL2, MPOD_CLK_SEL, Config->PacketOutputClock )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_OCTRL2, Reg );
#endif

    /* This member had a different name in older sw. Warn if the new one is changed. Just don't change the default value. */
    if( Config->OutputPacketDelayCount )
    {
        #if BXPT_HAS_PROGRAMMABLE_MPOD_CLOCK
            BDBG_ERR(( "Use PacketOutputClock instead of OutputPacketDelayCount on this chip" ));
        #else
            BDBG_ERR(( "OutputPacketDelayCount is not supported on this chip." ));
        #endif
        return BERR_NOT_SUPPORTED;
    }

    /* Enable the interface. */
    ExitCode = SetOnlineState( hXpt, true, Config->SkipInputFormatter );

    return( ExitCode );
}

BERR_Code BXPT_Mpod_Shutdown(
    BXPT_Handle hXpt                            /* [in] Handle for this transport */
    )
{
    unsigned ii;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    /* Disable the interface. */
    SetOnlineState( hXpt, false, false );

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

    return( ExitCode );
}

unsigned int BXPT_Mpod_GetPodRes(
    BXPT_Handle hXpt                /* [in] Handle for this transport */
    )
{
    unsigned int Reg;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_RES_FIELD );
    return BCHP_GET_FIELD_DATA( Reg, XPT_MPOD_RES_FIELD, POD_RES );
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    Ctrl1Addr = GetParserCtrl1Addr( hXpt, ParserType, ParserNum );
    Ctrl2Addr = GetParserCtrl2Addr( hXpt, ParserType, ParserNum );
    if( !Ctrl1Addr || !Ctrl2Addr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* This works since PB and FE parser control reg 2 are identical */
    Reg = BREG_Read32( hXpt->hRegister, Ctrl1Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl1Addr, Reg );

    Reg = BREG_Read32( hXpt->hRegister, Ctrl2Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL2, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL2, MPOD_EN, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl2Addr, Reg );

    Done:
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    Ctrl1Addr = GetParserCtrl1Addr( hXpt, ParserType, ParserNum );
    Ctrl2Addr = GetParserCtrl2Addr( hXpt, ParserType, ParserNum );
    if( !Ctrl1Addr || !Ctrl2Addr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* This works since PB and FE parser control reg 2 are identical */
    Reg = BREG_Read32( hXpt->hRegister, Ctrl1Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL, ((MpodPidFilter==false) && Enable) ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl1Addr, Reg );
    
    Reg = BREG_Read32( hXpt->hRegister, Ctrl2Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL2, PARSER_MPOD_CONT_COUNT_IGNORE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL2, PARSER_MPOD_CONT_COUNT_IGNORE, ((ContinuityCountCheck == false) && Enable) ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl2Addr, Reg );

    Reg = BREG_Read32( hXpt->hRegister, Ctrl2Addr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL2, MPOD_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL2, MPOD_EN, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, Ctrl2Addr, Reg );

    Done:
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

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    RegAddr = GetParserCtrl2Addr( hXpt, ParserType, ParserNum );
    if( !RegAddr )
    {
        BDBG_ERR(( "Invalid ParserNum and ParserType combination!" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    /* This works since PB and FE parser control reg 2 are identical */
    Reg = BREG_Read32( hXpt->hRegister, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_FE_PARSER0_CTRL2, PARSER_MPOD_ALL_PASS_CTRL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_FE_PARSER0_CTRL2, PARSER_MPOD_ALL_PASS_CTRL, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, RegAddr, Reg );

    Done:
    return ExitCode;
}

void BXPT_Mpod_EnableInputFormatter(
    BXPT_Handle hXpt,                           /* [in] Handle for this transport */
    bool Enable
    )
{
    uint32_t Reg;

    BDBG_OBJECT_ASSERT(hXpt, bxpt_t);

    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL );
    Reg &= ~(
        BCHP_MASK( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_MPOD_ICTRL, INPUT_FORMATTER_EN, Enable ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_MPOD_ICTRL, Reg );
}

static BERR_Code SetOnlineState(
    BXPT_Handle hXpt,             /* [in] Handle for this transport */
    bool Online,                   /* [in] true if MPOD should be online, false otherwise */
    bool SkipInputFormatter
    )
{
    int Enable;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

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
    if( SkipInputFormatter ) 
        BXPT_Mpod_EnableInputFormatter( hXpt, false );
    else
        BXPT_Mpod_EnableInputFormatter( hXpt, true );

#if (BCHP_CHIP == 7342) || (BCHP_CHIP == 35130) || (BCHP_CHIP == 7125) || ( BCHP_CHIP == 7340 ) \
|| (BCHP_CHIP ==35230) || (BCHP_CHIP == 35125) || (BCHP_CHIP == 35233)
/*RMXP_MPOD_MUX_SEL removed in these chips*/
#else
    Reg = BREG_Read32( hXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0 );
    Reg &= ~(
        BCHP_MASK( XPT_BUS_IF_MISC_CTRL0, RMXP_MPOD_MUX_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_BUS_IF_MISC_CTRL0, RMXP_MPOD_MUX_SEL, Online == true ? 1 : 0 )
    );
    BREG_Write32( hXpt->hRegister, BCHP_XPT_BUS_IF_MISC_CTRL0, Reg );
#endif
    return( ExitCode );
}

static uint32_t GetParserCtrl1Addr(
    BXPT_Handle hXpt,             /* [in] Handle for this transport */
    BXPT_ParserType ParserType,
    unsigned ParserNum
    )
{
    uint32_t Ctrl2Addr = 0;

    switch( ParserType )
    {
        case BXPT_ParserType_eIb:
        if( ParserNum < hXpt->MaxPidParsers )
			#if (BCHP_CHIP == 7342)
			Ctrl2Addr = BCHP_XPT_FE_PARSER0_CTRL1 + BXPT_P_GetParserRegOffset(ParserNum);
			#else
            Ctrl2Addr = BCHP_XPT_FE_PARSER0_CTRL1 + ( ParserNum * IB_PARSER_STEP );
			#endif
        break;

        case BXPT_ParserType_ePb:
        if( ParserNum < hXpt->MaxPlaybacks )
            Ctrl2Addr = BCHP_XPT_PB0_PARSER_CTRL1 + ( ParserNum * PB_PARSER_STEP );
        break;

        /* Return 0, tells the caller the function failed. */
        default:
        break;
    }

    return Ctrl2Addr;
}

static uint32_t GetParserCtrl2Addr(
    BXPT_Handle hXpt,             /* [in] Handle for this transport */
    BXPT_ParserType ParserType,
    unsigned ParserNum
    )
{
    uint32_t Ctrl2Addr = 0;

    switch( ParserType )
    {
        case BXPT_ParserType_eIb:
        if( ParserNum < hXpt->MaxPidParsers )
			#if (BCHP_CHIP == 7342)
			Ctrl2Addr = BCHP_XPT_FE_PARSER0_CTRL2 + BXPT_P_GetParserRegOffset(ParserNum);
			#else
            Ctrl2Addr = BCHP_XPT_FE_PARSER0_CTRL2 + ( ParserNum * IB_PARSER_STEP );
			#endif
        break;

        case BXPT_ParserType_ePb:
        if( ParserNum < hXpt->MaxPlaybacks )
            Ctrl2Addr = BCHP_XPT_PB0_PARSER_CTRL2 + ( ParserNum * PB_PARSER_STEP );
        break;

        /* Return 0, tells the caller the function failed. */
        default:
        break;
    }

    return Ctrl2Addr;
}
