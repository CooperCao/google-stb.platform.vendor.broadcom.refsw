/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bmem.h"
#include "bxpt_priv.h"
#include "bxpt_pcr_offset.h"
#include "bchp_xpt_pcroffset.h"
#include "bchp_xpt_fe.h"
#include "bchp.h"

#if( BDBG_DEBUG_BUILD == 1 )
BDBG_MODULE( xpt_pcr_offset_emv );
#endif

#define DEFAULT_STC_COUNT_MODE      BXPT_PcrOffset_StcCountMode_eMod300
#define DEFAULT_USE_PCR_TIMEBASE    true
#define DEFAULT_TIME_BASE           0
#define DEFAULT_OFFSET_THRESHOLD    ( 0x08 )
#define DEFAULT_MAX_PCR_ERROR       ( 0xFF )

#define PCROFF_CONTEXT_STEP             ( BCHP_XPT_PCROFFSET_CONTEXT1_PCROFFSET_CTRL - BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define STC_CONTEXT_STEP                ( BCHP_XPT_PCROFFSET_STC1_CTRL - BCHP_XPT_PCROFFSET_STC0_CTRL )
#if BXPT_HAS_STC_SNAPSHOT_XBAR
#define STC_SNAPSHOT_CONTEXT_STEP       ( BCHP_XPT_PCROFFSET_STC_SNAPSHOT1_CTRL - BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_CTRL )
#else
#define STC_SNAPSHOT_CONTEXT_STEP       STC_CONTEXT_STEP
#endif

#define PCROFF_CTRL_OFFSET              ( BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL -      BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_THRESHOLD_OFFSET         ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_MAX_ERROR_OFFSET         ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_PID_CHAN_OFFSET          ( BCHP_XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM -   BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_OFFSET            ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET -              BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_RDPTR_OFFSET      ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_RD_PTR -       BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_WRPTR_OFFSET      ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_WR_PTR -       BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_CHAN01_OFFSET     ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_PID_CH0_CH1 -  BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_CHAN23_OFFSET     ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_PID_CH2_CH3 -  BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_CHAN45_OFFSET     ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_PID_CH4_CH5 -  BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_CHAN67_OFFSET     ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_PID_CH6_CH7 -  BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_SPLICE_STATUS_OFFSET     ( BCHP_XPT_PCROFFSET_CONTEXT0_SPLICE_STATUS -       BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATE             ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATE -        BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )

#define PCROFF_OFFSET_STATUS0           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_0 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS1           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_1 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS2           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_2 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS3           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_3 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS4           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_4 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS5           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_5 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS6           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_6 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS7           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_7 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS8           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_8 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS9           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_9 -     BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS10          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_10 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS11          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_11 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS12          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_12 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS13          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_13 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS14          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_14 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )
#define PCROFF_OFFSET_STATUS15          ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_15 -    BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )

#define PCROFF_OFFSET_INVALID           ( BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_VALID -        BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL )

#define PID_CHNL_STEPSIZE       ( 4 )

static uint32_t GetStcRegAddr_isrsafe(
    uint32_t Ctx0Addr,
    unsigned WhichStc
    );

#if (!B_REFSW_MINIMAL)
static void SetSplicePidChannel(
    BXPT_PcrOffset_Handle hPcrOff,
    unsigned ChannelNum,
    unsigned Pid
    );
#endif

static uint32_t MapPcrToTimebase(
    unsigned WhichPcr
    )
{
#if BXPT_PCR_OFFSET_STC_SKIP
    if (WhichPcr < STC_FREE_RUNNING)
        return WhichPcr;
    else
        return WhichPcr + 1;
#else
    return WhichPcr;
#endif
}

static unsigned MapTimebaseToPcr(
    uint32_t Timebase
    )
{
#if BXPT_PCR_OFFSET_STC_SKIP
    if (Timebase > STC_FREE_RUNNING)
        return Timebase - 1;
    else
        return Timebase;
#else
    return Timebase;
#endif
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PcrOffset_GetTotalChannels(
    BXPT_Handle hXpt,               /* [in] The transport handle */
    unsigned int *TotalChannels     /* [out] The number of Pcr offset channels. */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( hXpt );

    /* PCR Offset channels are mapped one-to-one to the RAVE channels. */
    *TotalChannels = BXPT_NUM_PCR_OFFSET_CHANNELS;

    return( ExitCode );
}
#endif

BERR_Code BXPT_PcrOffset_GetChannelDefSettings(
    BXPT_Handle hXpt,           /* [in] The transport handle - need chip info */
    unsigned int ChannelNo,                 /* [in] Which pcr offset module */
    BXPT_PcrOffset_Defaults *Defaults   /* [out] The default settings of a pcr module */
    )
{
    BERR_Code ExitCode = BERR_SUCCESS;

    BSTD_UNUSED( hXpt );
    BDBG_ASSERT( Defaults );

    if( ChannelNo >= BXPT_NUM_PCR_OFFSET_CHANNELS )
    {
        BDBG_ERR(( "ChannelNo %u out of range!", ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    Defaults->UsePcrTimeBase = DEFAULT_USE_PCR_TIMEBASE;
    Defaults->WhichPcr = DEFAULT_TIME_BASE;
    Defaults->OffsetThreshold = DEFAULT_OFFSET_THRESHOLD;
    Defaults->MaxPcrError = DEFAULT_MAX_PCR_ERROR;

    Defaults->WhichStc = ChannelNo % BXPT_NUM_STCS;
    Defaults->ConsecutiveErrorThreshold = 2;

    Done:
    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_Open(
    BXPT_Handle hXpt,                  /* [in] The transport handle*/
    unsigned int ChannelNo,            /* [in] Which pcr offset module */
    BXPT_PcrOffset_Defaults *Defaults, /* [in] The default setting */
    BXPT_PcrOffset_Handle *hChannel    /* [out] The channel handle */
    )
{
    BXPT_PcrOffset_Handle lhPcrOff;
    uint32_t Reg, RegAddr;
    unsigned Index;

    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t TimeBase = STC_FREE_RUNNING;
    unsigned OneErrAcq = 0;
    unsigned TwoErrAcq = 0;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( Defaults );
    BDBG_ASSERT( hChannel );

    if( ChannelNo >= BXPT_NUM_PCR_OFFSET_CHANNELS )
    {
        BDBG_ERR(( "ChannelNo %u out of range!", ChannelNo ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    if( Defaults->WhichStc >= BXPT_NUM_STCS )
    {
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    /* Allocate memory for the handle. */
    lhPcrOff = BKNI_Malloc( sizeof( BXPT_P_PcrOffset_Impl ) );
    if( lhPcrOff == NULL )
    {
        BDBG_ERR(( "BKNI_Malloc() failed!" ));
        ExitCode = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
        goto Done;
    }

    BXPT_P_AcquireSubmodule(hXpt, BXPT_P_Submodule_ePcrOffset);

    lhPcrOff->BaseAddr = BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL + ( ChannelNo * PCROFF_CONTEXT_STEP );
    lhPcrOff->hChip = hXpt->hChip;
    lhPcrOff->hReg = hXpt->hRegister;
    lhPcrOff->lvXpt = ( void * ) hXpt;
    lhPcrOff->ChannelNo = ChannelNo;
    lhPcrOff->PidChannelNum = 0;
    lhPcrOff->WhichStc = Defaults->WhichStc;
    lhPcrOff->UseHostPcrs = false;

    /* Map the PCR Offset to an STC. */
    Reg = BREG_Read32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, STC_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, STC_SEL, Defaults->WhichStc )
    );
    BREG_Write32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    if( Defaults->UsePcrTimeBase )
        TimeBase = MapPcrToTimebase( Defaults->WhichPcr );
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_TIMEBASE_SEL, Defaults->WhichStc );
    Reg = BREG_Read32( lhPcrOff->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL, TimeBase )
    );
    BREG_Write32( lhPcrOff->hReg, RegAddr, Reg );
    lhPcrOff->CurrentTimeBase = TimeBase;

    Reg = BREG_Read32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD, OFFSET_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD, OFFSET_THRESHOLD, Defaults->OffsetThreshold )
    );
    BREG_Write32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_THRESHOLD_OFFSET, Reg );

    Reg = BREG_Read32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_MAX_ERROR_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR, MAX_ERROR )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR, MAX_ERROR, Defaults->MaxPcrError )
    );
    BREG_Write32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_MAX_ERROR_OFFSET, Reg );

    BXPT_PcrOffset_ClearQueue( lhPcrOff );

    Reg = BREG_Read32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_PID_CHAN_OFFSET );
    Reg &= ~( BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM ) );
    Reg |= ( BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM, 0xFE ) );
    BREG_Write32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_PID_CHAN_OFFSET, Reg );

    /* Walk through the PID Config table and clear any index that's using this Offset */
    for( Index = 0; Index < BXPT_NUM_PID_CHANNELS; Index++ )
    {
        RegAddr = BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE + Index * PID_CHNL_STEPSIZE;
        Reg = BREG_Read32( hXpt->hRegister, RegAddr );
        if( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_PID_CONFIG_TABLE_i, OFFSET_INDEX ) ==  ChannelNo )
            BREG_Write32( hXpt->hRegister, RegAddr, 0 );
    }

    Reg = BREG_Read32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, TWO_ERR_ACQ ) |
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, ONE_ERR_ACQ )
    );
    if( Defaults->ConsecutiveErrorThreshold == 1 )
    {
        OneErrAcq = 1;
    }
    else if( Defaults->ConsecutiveErrorThreshold >= 2 )
    {
        TwoErrAcq = 1;
    }
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, TWO_ERR_ACQ, TwoErrAcq ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, ONE_ERR_ACQ, OneErrAcq )
    );
    BREG_Write32( lhPcrOff->hReg, lhPcrOff->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

#if BXPT_HAS_TSMUX
    /* initialize NRT mode related STC fields to OFF */
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, lhPcrOff->WhichStc );
    Reg = BREG_Read32( lhPcrOff->hReg, RegAddr );

    #if BXPT_HAS_STC_TRIG_TYPE
        Reg &= ~(
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_TYPE ) |
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_SEL ) |
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL ) |
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN )
        );
    #else
        Reg &= ~(
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_MODE ) |
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL ) |
            BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN )
        );
    #endif

    BREG_Write32( lhPcrOff->hReg, RegAddr, Reg );
#endif

    /* Unfreeze the STC. This is the default hw state. */
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, lhPcrOff->WhichStc );
    Reg = BREG_Read32( lhPcrOff->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, FREEZE )
    );
    BREG_Write32( lhPcrOff->hReg, RegAddr, Reg );

    /* Note that no PID channels are using this Offset block */
    lhPcrOff->PidChannelNum = BXPT_NUM_PID_CHANNELS;

    hXpt->PcrOffsets[ ChannelNo ].Handle = lhPcrOff;
    hXpt->PcrOffsets[ ChannelNo ].Allocated = true;
    *hChannel = lhPcrOff;

    Done:
    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_Close(
    BXPT_PcrOffset_Handle hChannel   /* [in] The channel handle */
    )
{
    BXPT_Handle hXpt;
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );


    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, CTXT_ENABLE )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    hXpt = ( BXPT_Handle ) hChannel->lvXpt;
    hXpt->PcrOffsets[ hChannel->ChannelNo ].Allocated = false;
    BKNI_Free( hChannel );

    BXPT_P_ReleaseSubmodule(hXpt, BXPT_P_Submodule_ePcrOffset);

    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_GetSettings(
    BXPT_PcrOffset_Handle hChannel,
    BXPT_PcrOffset_Settings *Settings
    )
{
    uint32_t Reg, RegAddr;
    uint32_t Timebase;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );
    BDBG_ASSERT( Settings );

    Settings->PidChannelNum = hChannel->PidChannelNum;
    Settings->UseHostPcrs = hChannel->UseHostPcrs;

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_TIMEBASE_SEL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Timebase = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL );

    if( Timebase == STC_FREE_RUNNING )
    {
        Settings->UsePcrTimeBase = false;
    }
    else
    {
        Settings->UsePcrTimeBase = true;
        Settings->WhichPcr = MapTimebaseToPcr( Timebase );
    }

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Settings->CountMode = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE ) == 1 ?
        BXPT_PcrOffset_StcCountMode_eBinary : BXPT_PcrOffset_StcCountMode_eMod300;

#if BXPT_HAS_TSMUX
    Settings->BroadcastMode = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, BROADCAST_MODE );
#endif

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_THRESHOLD_OFFSET );
    Settings->OffsetThreshold = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD, OFFSET_THRESHOLD );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_MAX_ERROR_OFFSET );
    Settings->MaxPcrError = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR, MAX_ERROR );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Settings->StcSelect = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, STC_SEL );

    if( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, ONE_ERR_ACQ ) )
        Settings->ConsecutiveErrorThreshold = 1;
    if( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, TWO_ERR_ACQ ) )
        Settings->ConsecutiveErrorThreshold = 2;

    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_ReleasePidChannel(
    BXPT_PcrOffset_Handle hChannel,
    unsigned int PidChannel
    )
{
    BDBG_ASSERT( hChannel );

    if( PidChannel >= BXPT_NUM_PID_CHANNELS )
    {
        /* Bad PID channel number. Complain. */
        BDBG_ERR(( "PidChannel %lu is out of range!", ( unsigned long ) PidChannel ));
        return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    BXPT_P_SetPidChannelDestination( (BXPT_Handle) hChannel->lvXpt, PidChannel, 5, false );
    hChannel->PidChannelNum = BXPT_NUM_PID_CHANNELS;
    return BERR_SUCCESS;
}

BERR_Code BXPT_PcrOffset_SetSettings(
    BXPT_PcrOffset_Handle hChannel,
    const BXPT_PcrOffset_Settings *Settings
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;
    uint32_t TimeBase = STC_FREE_RUNNING;
    unsigned OneErrAcq = 0;
    unsigned TwoErrAcq = 0;
    bool routeToRave = false;

    BDBG_ASSERT( hChannel );
    BDBG_ASSERT( Settings );

    if( Settings->StcSelect >= BXPT_NUM_STCS )
    {
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }
    hChannel->WhichStc = Settings->StcSelect;

    /* Need to disable the PCR Offset block before doing anything. See the RDB */
    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, CTXT_ENABLE )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    if( Settings->UsePcrTimeBase )
        TimeBase = MapPcrToTimebase( Settings->WhichPcr );
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_TIMEBASE_SEL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL, TimeBase )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );
    hChannel->CurrentTimeBase = TimeBase;

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
#if BXPT_HAS_TSMUX
    /* if AV window pairing is enabled, must adjust av_window based on the count mode */
    if(BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN )) {
        uint32_t AvWinRegAddr, AvWinReg, AvWindow;
        uint32_t oldCountMode = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE );
        /* The units used in AV_WINDOW change with the channels' mode (either MOD300 or Binary).
        ** For MOD300, the units are 45kH ticks, whereas in binary they are 27 MHz. We want to scale this so
        ** that the AvWindow is expressed as mS of delay. */
        if(oldCountMode != Settings->CountMode) {/* update av window if count mode is changed */
            AvWinRegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_AV_WINDOW, hChannel->WhichStc );
            AvWinReg = BREG_Read32( hChannel->hReg, AvWinRegAddr );
            AvWindow = BCHP_GET_FIELD_DATA(AvWinReg, XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW ) /
                    (oldCountMode ? 27000 : 45) *
                    (Settings->CountMode? 27000 : 45);
            AvWinReg &= ~(
                BCHP_MASK( XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW )
            );
            AvWinReg |= (
                BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW, AvWindow )
            );
            BREG_Write32( hChannel->hReg, AvWinRegAddr, AvWinReg );
        }
    }
#endif
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, MODE, Settings->CountMode )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );

#if BXPT_HAS_TSMUX
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, BROADCAST_MODE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, BROADCAST_MODE, Settings->BroadcastMode )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );
#endif

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_PID_CHAN_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM )
    );

    if (hChannel->PidChannelNum != Settings->PidChannelNum) {
        routeToRave = true; /* route the PID channel to RAVE, but only when PidChannelNum changes, so that repeated calls to SetSettings is harmless */
    }
    hChannel->PidChannelNum = Settings->PidChannelNum;
    hChannel->UseHostPcrs = Settings->UseHostPcrs;
    if( Settings->UseHostPcrs == true )
    {
        Reg |= ( BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM, 0xFF ) );
        BXPT_PcrOffset_ForceInvalid( hChannel );
    }
    else
    {
        Reg |= ( BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM, Settings->PidChannelNum ) );

        /* Enable PCR Offset in the PID channel config table */
        BXPT_PcrOffset_EnableOffset( hChannel, Settings->PidChannelNum, false, true );
    }

    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_PID_CHAN_OFFSET, Reg );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_MAX_ERROR_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR, MAX_ERROR )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_MAX_ERROR, MAX_ERROR, Settings->MaxPcrError )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_MAX_ERROR_OFFSET, Reg );

    /*
    ** Need to enable data on this PID channel to go RAVE. PCR Offset ignores it otherwise. To
    ** resolve PR 28674, enable the R pipe. All data destined for the decoders will be on the R pipe.
    */
    if( (Settings->UseHostPcrs == false) && routeToRave )
    {
        BXPT_P_SetPidChannelDestination( (BXPT_Handle) hChannel->lvXpt, Settings->PidChannelNum, 5, true );
    }

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_THRESHOLD_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD, OFFSET_THRESHOLD )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_THRESHOLD, OFFSET_THRESHOLD, Settings->OffsetThreshold )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_THRESHOLD_OFFSET, Reg );

    /* Put this offset context into acquire mode */
    BXPT_PcrOffset_Acquire( hChannel );

    /* Enable the context after the config is done. */
    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, STC_SEL ) |
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, CTXT_ENABLE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, STC_SEL, Settings->StcSelect ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, CTXT_ENABLE, 1 )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, TWO_ERR_ACQ ) |
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, ONE_ERR_ACQ )
    );
    if( Settings->ConsecutiveErrorThreshold == 1 )
    {
        OneErrAcq = 1;
    }
    else if( Settings->ConsecutiveErrorThreshold >= 2 )
    {
        TwoErrAcq = 1;
    }
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, TWO_ERR_ACQ, TwoErrAcq ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, ONE_ERR_ACQ, OneErrAcq )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    Done:
    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_ChangeStcRate(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    unsigned Increment,                 /* [in] STC increment value */
    unsigned Prescale                   /* [in] STC prescale value */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    if( Increment > BXPT_MAX_STC_INCREMENT )
    {
        BDBG_ERR(( "Increment %u out of range. Max is %u", Increment, BXPT_MAX_STC_INCREMENT ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    if( Prescale > BXPT_MAX_STC_PRESCALE )
    {
        BDBG_ERR(( "Prescale %u out of range. Max is %u", Prescale, BXPT_MAX_STC_PRESCALE ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_RATE_CTRL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_RATE_CTRL, PRESCALE ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_RATE_CTRL, INCREMENT )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_RATE_CTRL, PRESCALE, Prescale ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_RATE_CTRL, INCREMENT, Increment )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );

    Done:
    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_FreezeStc(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    bool Freeze                         /* [in] STC is frozen if true, unfrozen if false */
    )
{
    uint32_t Reg, RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, FREEZE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, FREEZE, Freeze == true ? 1 : 0 )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );

    return( ExitCode );
}

uint32_t BXPT_PcrOffset_GetStc(
    BXPT_PcrOffset_Handle hChannel      /* [in] The channel handle */
    )
{
    uint32_t Stc;

    BKNI_EnterCriticalSection();
    Stc = BXPT_PcrOffset_GetStc_isr( hChannel );
    BKNI_LeaveCriticalSection();
    return Stc;
}

uint32_t BXPT_PcrOffset_GetStc_isr(
    BXPT_PcrOffset_Handle hChannel      /* [in] The channel handle */
    )
{
    uint32_t Reg, RegAddr, Stc;

    BDBG_ASSERT( hChannel );

#ifdef BCHP_XPT_PCROFFSET_STC0
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Stc = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0, COUNT );
#else
    {
        bool Mode;

        RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hChannel->WhichStc );
        Reg = BREG_Read32( hChannel->hReg, RegAddr );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE );

        if( Mode )
        {
            /* On some chips, we need to read STC0_HI to latch both _HI and _LO values into the register */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_HI, hChannel->WhichStc );
            Reg = BREG_Read32( hChannel->hReg, RegAddr );

            /* Binary */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_LO, hChannel->WhichStc );
            Reg = BREG_Read32( hChannel->hReg, RegAddr );
            Stc = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_LO, COUNT );
        }
        else
        {
            /* Mod300 */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_HI, hChannel->WhichStc );
            Reg = BREG_Read32( hChannel->hReg, RegAddr );
            Stc = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_HI, COUNT ) << 22;

            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_LO, hChannel->WhichStc );
            Reg = BREG_Read32( hChannel->hReg, RegAddr );
            Stc |= (BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_LO, COUNT ) >> 10);
        }
    }
#endif

    return Stc;
}

BERR_Code BXPT_PcrOffset_SetStc(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    uint32_t NewStc
    )
{
    BERR_Code Status;

    BKNI_EnterCriticalSection();
    Status = BXPT_PcrOffset_SetStc_isr( hChannel, NewStc );
    BKNI_LeaveCriticalSection();
    return Status;
}

BERR_Code BXPT_PcrOffset_SetStc_isr(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    uint32_t NewStc
    )
{
    uint32_t RegAddr;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

#ifdef BCHP_XPT_PCROFFSET_STC0
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0, hChannel->WhichStc );
    BREG_Write32( hChannel->hReg, RegAddr, NewStc );
#else
    {
        bool Mode;
        uint32_t Reg;

        RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hChannel->WhichStc );
        Reg = BREG_Read32( hChannel->hReg, RegAddr );
        Mode = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE );

        if( Mode )
        {
            /* Binary */
            /* This API has 32-bit STC, so should zero out hi reg for 27MHz binary count mode */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_HI, hChannel->WhichStc );
            BREG_Write32( hChannel->hReg, RegAddr, 0 );/* zero hi reg in case it's paired in NRT mode */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_LO, hChannel->WhichStc );
            BREG_Write32( hChannel->hReg, RegAddr, NewStc );
        }
        else
        {
            /* Mod300 */
            /* STC is in 45KHz */
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_HI, hChannel->WhichStc );
            BREG_Write32( hChannel->hReg, RegAddr, (NewStc >> 22) & 0x3FF );
            RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_LO, hChannel->WhichStc );
            BREG_Write32( hChannel->hReg, RegAddr, (NewStc << 10) & 0xFFFFFC00 );
        }
    }
#endif

    return( ExitCode );
}

uint32_t BXPT_PcrOffset_GetOffset(
    BXPT_PcrOffset_Handle hChannel          /* [in] The channel handle */
    )
{
    uint32_t Offset;

    BKNI_EnterCriticalSection();
    Offset = BXPT_PcrOffset_GetOffset_isr( hChannel );
    BKNI_LeaveCriticalSection();
    return Offset;
}

uint32_t BXPT_PcrOffset_GetOffset_isr(
    BXPT_PcrOffset_Handle hChannel          /* [in] The channel handle */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_OFFSET_OFFSET );
    return BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_OFFSET, PCR_OFFSET );
}

BERR_Code BXPT_PcrOffset_SetOffset(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    uint32_t UserPcrOffset                  /* [in] User-supplied PCR Offset */
    )
{
    BERR_Code Status;

    BKNI_EnterCriticalSection();
    Status = BXPT_PcrOffset_SetOffset_isr( hChannel, UserPcrOffset );
    BKNI_LeaveCriticalSection();
    return Status;
}

BERR_Code BXPT_PcrOffset_SetOffset_isr(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    uint32_t UserPcrOffset                  /* [in] User-supplied PCR Offset */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_OFFSET_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET, PCR_OFFSET )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET, PCR_OFFSET, UserPcrOffset )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_OFFSET_OFFSET, Reg );

    return( ExitCode );
}

BERR_Code BXPT_PcrOffset_FreeRun(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    bool FreeRun                        /* [in] Free run if true. */
    )
{
    uint32_t Reg, RegAddr;

    uint32_t NewTimeBase = STC_FREE_RUNNING;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    if( FreeRun == false )
        NewTimeBase = hChannel->CurrentTimeBase;

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_TIMEBASE_SEL, hChannel->WhichStc );
    Reg = BREG_Read32( hChannel->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_TIMEBASE_SEL, TIMEBASE_SEL, NewTimeBase )
    );
    BREG_Write32( hChannel->hReg, RegAddr, Reg );

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PcrOffset_PushPidChannel(
    BXPT_PcrOffset_Handle hChannel,     /* [in] The channel handle */
    unsigned int PidChannel             /* [in] Channel carrying the PID to splice. */
    )
{
    uint32_t WrPtr, Reg;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    if( BXPT_PcrOffset_GetQueueDepth( hChannel ) == MAX_PCR_SPLICING_ENTRIES )
    {
        BDBG_ERR(( "Splicing stack full!" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_WRPTR_OFFSET );
    WrPtr = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_SPLICE_WR_PTR, NEXT_PID_CH_WR_PTR );

    SetSplicePidChannel( hChannel, WrPtr, PidChannel );

    WrPtr = ( WrPtr + 1 ) % BXPT_P_SPLICING_QUEUE_SIZE;
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_WRPTR_OFFSET, WrPtr );

    /* Set the enable, just in case. Should happen automatically when RD != WR */
    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, SPLICE_ENABLE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, SPLICE_ENABLE, 1 )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    Done:
    return( ExitCode );
}
#endif

BERR_Code BXPT_PcrOffset_ClearQueue(
    BXPT_PcrOffset_Handle hChannel      /* [in] The channel handle */
    )
{
    uint32_t Reg;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, SPLICE_ENABLE )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_CTRL_OFFSET, Reg );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_RDPTR_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_SPLICE_RD_PTR, NEXT_PID_CH_RD_PTR )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_RDPTR_OFFSET, Reg );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_WRPTR_OFFSET );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_SPLICE_WR_PTR, NEXT_PID_CH_WR_PTR )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_WRPTR_OFFSET, Reg );

    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_CHAN01_OFFSET, 0 );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_CHAN23_OFFSET, 0 );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_CHAN45_OFFSET, 0 );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_CHAN67_OFFSET, 0 );

    return( ExitCode );
}

#if (!B_REFSW_MINIMAL)
unsigned BXPT_PcrOffset_GetCountdown(
    BXPT_PcrOffset_Handle hChannel      /* [in] The channel handle */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_STATUS_OFFSET );
    return BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_SPLICE_STATUS, COUNT );
}
#endif

unsigned BXPT_PcrOffset_GetQueueDepth(
    BXPT_PcrOffset_Handle hChannel      /* [in] The channel handle */
    )
{
    uint32_t RdPtr, WrPtr, Reg;

    unsigned Depth = 0;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_RDPTR_OFFSET );
    RdPtr = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_SPLICE_RD_PTR, NEXT_PID_CH_RD_PTR );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_SPLICE_WRPTR_OFFSET );
    WrPtr = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_SPLICE_WR_PTR, NEXT_PID_CH_WR_PTR );

    if( RdPtr < WrPtr )
        Depth = WrPtr - RdPtr;
    else if( WrPtr < RdPtr )
        Depth = BXPT_P_SPLICING_QUEUE_SIZE - RdPtr + WrPtr;

    return( Depth );
}

void BXPT_PcrOffset_RegenOffset_isr(
    BXPT_PcrOffset_Handle hPcrOff
    )
{
    uint32_t Addr;
    uint32_t EndRegister = PCROFF_OFFSET_STATUS7;

    BDBG_ASSERT( hPcrOff );

    /* Old Code
    Addr = hPcrOff->BaseAddr + PCROFF_OFFSET_OFFSET;
    BREG_Write32( hPcrOff->hReg, Addr, BREG_Read32( hPcrOff->hReg, Addr ) );
    */

#ifdef BCHP_XPT_PCROFFSET_CONTEXT0_OFFSET_STATUS_15
    EndRegister = PCROFF_OFFSET_STATUS15;
#endif

    for(Addr=(hPcrOff->BaseAddr + PCROFF_OFFSET_STATUS0); Addr <= (hPcrOff->BaseAddr + EndRegister); Addr+=4)
        BREG_Write32( hPcrOff->hReg, Addr, 0 );
}

BERR_Code BXPT_PcrOffset_EnableOffset(
    BXPT_PcrOffset_Handle hPcrOff,
    uint32_t PidChannelNum,
    bool FixedOffsetEn,
    bool JitterAdjustEn
    )
{
    uint32_t Reg, RegAddr;
    bool bAlreadyEnabled;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hPcrOff );

    if( PidChannelNum >= BXPT_NUM_PID_CHANNELS )
    {
        BDBG_ERR(( "PidChannelNum %u out of range!", PidChannelNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    RegAddr = BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE + PidChannelNum * PID_CHNL_STEPSIZE;
    Reg = BREG_Read32( hPcrOff->hReg, RegAddr );

    bAlreadyEnabled = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_PID_CONFIG_TABLE_i, PCROFFSET_EN ) == 1 ? true : false;
    if (bAlreadyEnabled) { ExitCode = BERR_NOT_SUPPORTED; BERR_TRACE( ExitCode); goto Done; }

    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, JITTER_DIS ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, FIXED_OFFSET_EN ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, PCROFFSET_EN ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, OFFSET_INDEX )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_PID_CONFIG_TABLE_i, JITTER_DIS, JitterAdjustEn == true ? 0 : 1 ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_PID_CONFIG_TABLE_i, FIXED_OFFSET_EN, FixedOffsetEn == true ? 1 : 0 ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_PID_CONFIG_TABLE_i, PCROFFSET_EN, 1 ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_PID_CONFIG_TABLE_i, OFFSET_INDEX, hPcrOff->ChannelNo )
    );

    BREG_Write32( hPcrOff->hReg, RegAddr, Reg );

    Done:
    return ExitCode;
}

void BXPT_PcrOffset_DisableOffset(
    BXPT_PcrOffset_Handle hPcrOff,
    uint32_t PidChannelNum
    )
{
    uint32_t Reg, RegAddr;

    BDBG_ASSERT( hPcrOff );

    if( PidChannelNum >= BXPT_NUM_PID_CHANNELS )
    {
        BDBG_ERR(( "PidChannelNum %u out of range!", PidChannelNum ));
        return;
    }

    RegAddr = BCHP_XPT_PCROFFSET_PID_CONFIG_TABLE_i_ARRAY_BASE + PidChannelNum * PID_CHNL_STEPSIZE;
    Reg = BREG_Read32( hPcrOff->hReg, RegAddr );

    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, JITTER_DIS ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, FIXED_OFFSET_EN ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, PCROFFSET_EN ) |
        BCHP_MASK( XPT_PCROFFSET_PID_CONFIG_TABLE_i, OFFSET_INDEX )
    );

    BREG_Write32( hPcrOff->hReg, RegAddr, Reg );
}

#if (!B_REFSW_MINIMAL)
void SetSplicePidChannel(
    BXPT_PcrOffset_Handle hPcrOff,
    unsigned ChannelNum,
    unsigned Pid
    )
{
    uint32_t Reg, RegAddr, Shift;

    /*
    ** Each splice register holds entries for 2 channels, so divide the
    ** channel number by 2 to figure out which reg holds the channel we're after.
    ** Then, mult by 4 since each channel is 4 bytes wide. Add in the physical
    ** address of the first register, and we're done.
    */
    RegAddr = hPcrOff->BaseAddr + 4 * ( ChannelNum / 2 );
    RegAddr += PCROFF_SPLICE_CHAN01_OFFSET;
    Reg = BREG_Read32( hPcrOff->hReg, RegAddr );

    /* Of the two channels in the reg, the lower numbered channel is in lower half of the word */
    Shift = ChannelNum % 2 ? 0 : 16;
    Reg &= ( 0xFF << Shift );
    Reg |= ( Pid << Shift );
    BREG_Write32( hPcrOff->hReg, RegAddr, Reg );
}
#endif

uint32_t GetStcRegAddr_isrsafe(
    uint32_t Ctx0Addr,
    unsigned WhichStc
    )
{
    return Ctx0Addr + STC_CONTEXT_STEP * WhichStc;
}

uint32_t GetStcSnapshotRegAddr_isrsafe(
    uint32_t Ctx0Addr,
    unsigned WhichStc
    )
{
    return Ctx0Addr + STC_SNAPSHOT_CONTEXT_STEP * WhichStc;
}

bool BXPT_PcrOffset_IsOffsetValid(
    BXPT_PcrOffset_Handle hPcrOff
    )
{
    bool Status;

    BKNI_EnterCriticalSection();
    Status = BXPT_PcrOffset_IsOffsetValid_isr( hPcrOff );
    BKNI_LeaveCriticalSection();
    return Status;
}

bool BXPT_PcrOffset_IsOffsetValid_isr(
    BXPT_PcrOffset_Handle hPcrOff
    )
{
    uint32_t Reg;
    bool Status;

    BDBG_ASSERT( hPcrOff );

    Reg = BREG_Read32( hPcrOff->hReg, hPcrOff->BaseAddr + PCROFF_OFFSET_INVALID );
    Status = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_CONTEXT0_OFFSET_VALID, PCR_OFFSET_VALID ) == 1 ? true : false;

    return( Status );
}

void  BXPT_PcrOffset_ForceInvalid( BXPT_PcrOffset_Handle hPcrOff)
{
    BKNI_EnterCriticalSection();
    BXPT_PcrOffset_ForceInvalid_isr( hPcrOff );
    BKNI_LeaveCriticalSection();
    return;
}

void  BXPT_PcrOffset_ForceInvalid_isr( BXPT_PcrOffset_Handle hPcrOff)
{
    uint32_t Reg;

    BDBG_ASSERT( hPcrOff );
    Reg = BREG_Read32( hPcrOff->hReg, hPcrOff->BaseAddr + PCROFF_OFFSET_INVALID );
    Reg &= ( BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_VALID, PCR_OFFSET_VALID, 0x0 ) );
    BREG_Write32( hPcrOff->hReg, hPcrOff->BaseAddr + PCROFF_OFFSET_INVALID, Reg );

    return;
}

#if (!B_REFSW_MINIMAL)
void BXPT_PcrOffset_CaptureStcOnce(
    BXPT_PcrOffset_Handle hPcrOff,      /* [in] Handle for the PCR Offset channel to use */
    bool Enable                         /* [in] true to capture once, false to disable a pending capture */
    )
{
    BKNI_EnterCriticalSection();
    BXPT_PcrOffset_CaptureStcOnce_isr( hPcrOff, Enable );
    BKNI_LeaveCriticalSection();
    return;
}
#endif

void BXPT_PcrOffset_CaptureStcOnce_isr(
    BXPT_PcrOffset_Handle hPcrOff,      /* [in] Handle for the PCR Offset channel to use */
    bool Enable                         /* [in] true to capture once, false to disable a pending capture */
    )
{
    uint32_t RegAddr, Reg;

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CAPTURE_PCR_ONCE, hPcrOff->WhichStc );
    Reg = BREG_Read32( hPcrOff->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CAPTURE_PCR_ONCE, ENABLE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CAPTURE_PCR_ONCE, ENABLE, Enable == true ? 1 : 0 )
    );
    BREG_Write32( hPcrOff->hReg, RegAddr, Reg );
}

#if (!B_REFSW_MINIMAL)
bool BXPT_P_PcrOffset_IsPidChannelInUse(
    BXPT_Handle hXpt,               /* [in] The transport handle */
    uint32_t PidChannelNum              /* [in] Which PID channel to disable offsets for */
    )
{
    unsigned ChannelNum;

    for( ChannelNum = 0; ChannelNum < BXPT_NUM_PCR_OFFSET_CHANNELS; ChannelNum++ )
    {
        uint32_t PidChannelReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM + ( ChannelNum * PCROFF_CONTEXT_STEP ) );
        uint32_t ControlReg = BREG_Read32( hXpt->hRegister, BCHP_XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL + ( ChannelNum * PCROFF_CONTEXT_STEP ) );

        /* The PID channel is mapped AND the offset context is enabled. */
        if( BCHP_GET_FIELD_DATA( PidChannelReg, XPT_PCROFFSET_CONTEXT0_PP_PCR_PID_CH_NUM, PCR_PID_CH_NUM ) == PidChannelNum
        && BCHP_GET_FIELD_DATA( ControlReg, XPT_PCROFFSET_CONTEXT0_PCROFFSET_CTRL, CTXT_ENABLE ) )
        {
            return true;
        }
    }

    return false;
}

BERR_Code BXPT_PcrOffset_GetIntId(
    BXPT_PcrOffset_Handle hChannel,
    BXPT_PcrOffsetIntName Name,
    BINT_Id *IntId
    )
{
    uint32_t RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS;
    uint32_t IntShift = 0;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hChannel );

    switch( hChannel->ChannelNo )
    {
        case 0:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS;
        IntShift = 0;
        break;

        case 1:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS;
        IntShift = 8;
        break;

        case 2:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS;
        IntShift = 16;
        break;

        case 3:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT0_STATUS;
        IntShift = 24;
        break;

        case 4:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS;
        IntShift = 0;
        break;

        case 5:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS;
        IntShift = 8;
        break;

        case 6:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS;
        IntShift = 16;
        break;

        case 7:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT1_STATUS;
        IntShift = 24;
        break;

        case 8:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS;
        IntShift = 0;
        break;

        case 9:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS;
        IntShift = 8;
        break;

        case 10:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS;
        IntShift = 16;
        break;

        case 11:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT2_STATUS;
        IntShift = 24;
        break;

        case 12:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS;
        IntShift = 0;
        break;

        case 13:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS;
        IntShift = 8;
        break;

        case 14:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS;
        IntShift = 16;
        break;

        case 15:
        RegAddr = BCHP_XPT_PCROFFSET_INTERRUPT3_STATUS;
        IntShift = 24;
        break;

        default:
        BDBG_ERR(( "Unsupported offset channel" ));
        ExitCode = BERR_TRACE( BXPT_ERR_NO_AVAILABLE_RESOURCES );
        goto Done;
    }

    *IntId = BCHP_INT_ID_CREATE( RegAddr, Name + IntShift );

    Done:
    return( ExitCode );
}
#endif

void BXPT_P_PcrOffset_ModuleInit(
    BXPT_Handle hXpt
    )
{
    unsigned Reg =
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST0_SEL, 0 )

#if BXPT_NUM_STCS > 1
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST1_SEL, 1 )
#endif

#if BXPT_NUM_STCS > 2
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST2_SEL, 2 )
#endif

#if BXPT_NUM_STCS > 3
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST3_SEL, 3 )
#endif

#if BXPT_NUM_STCS > 4
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST4_SEL, 4 )
#endif

#if BXPT_NUM_STCS > 5
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST5_SEL, 5 )
#endif

#if BXPT_NUM_STCS > 6
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST6_SEL, 6 )
#endif

#if BXPT_NUM_STCS > 7
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL, STC_BCAST7_SEL, 7 )
#endif
    ;

    BREG_Write32( hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL, Reg );

#if BXPT_NUM_STCS > 8
    Reg =
          BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST8_SEL, 8 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST9_SEL, 9 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST10_SEL, 10 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST11_SEL, 11 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST12_SEL, 12 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST13_SEL, 13 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST14_SEL, 14 )
        | BCHP_FIELD_DATA( XPT_PCROFFSET_STC_BROADCAST_SEL2, STC_BCAST15_SEL, 15 );

    BREG_Write32( hXpt->hRegister, BCHP_XPT_PCROFFSET_STC_BROADCAST_SEL2, Reg );
#endif

#if BXPT_NUM_STCS > 15
    #error "PCR Offset init needs updating"
#endif

}

#if BXPT_HAS_TSMUX

void BXPT_PcrOffset_GetNRTConfig(
    BXPT_PcrOffset_Handle hPcrOffset,   /* [in] The channel handle */
    BXPT_PcrOffset_NRTConfig *Config
    )
{
    uint32_t Reg;
    unsigned WindowDivisor;

    BDBG_ASSERT( hPcrOffset );
    BDBG_ASSERT( Config );

    Reg = BREG_Read32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hPcrOffset->WhichStc ) );
    Config->PairedStc = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL );
    Config->EnableAvWindowComparison = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN ) ? true : false;

    /* The units used in AV_WINDOW change with the channels' mode (either MOD300 or Binary).
    ** For MOD300, the units are 45kH ticks, whereas in binary they are 27 MHz. We want to scale this so
    ** that the AvWindow is expressed as mS of delay. */
    WindowDivisor = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE ) ? 27000 : 45;
    Config->AvWindow = BCHP_GET_FIELD_DATA(
        BREG_Read32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_AV_WINDOW, hPcrOffset->WhichStc ) ),
        XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW ) / WindowDivisor;

    Config->ExternalTriggerNum = 0xFFFF;

#if BXPT_HAS_STC_TRIG_TYPE
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, TRIG_TYPE ) )
    {
        case 0:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eTimebase;
        break;

        case 1:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eSoftIncrement;
        break;

        case 2:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eExternalTrig;
        Config->ExternalTriggerNum = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, TRIG_SEL );
        break;
    }
#else
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, TRIG_MODE ) )
    {
        case 0:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eTimebase;
        break;

        case 1:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eExternalTrig;
        Config->ExternalTriggerNum = 0;
        break;

        case 2:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eExternalTrig;
        Config->ExternalTriggerNum = 1;
        break;

        case 3:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eSoftIncrement;
        break;

        case 4:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eExternalTrig;
        Config->ExternalTriggerNum = 2;
        break;

        case 5:
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eExternalTrig;
        Config->ExternalTriggerNum = 3;
        break;

        default:
        BDBG_ERR(( "Unknown TRIG_MODE. Defaulting to timebase" ));
        Config->TriggerMode = BXPT_PcrOffset_StcTriggerMode_eTimebase;
        break;
    }
#endif

    Config->StcIncrement = (uint64_t) BCHP_GET_FIELD_DATA(
        BREG_Read32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_HI, hPcrOffset->WhichStc ) ),
        XPT_PCROFFSET_STC0_INC_HI, INCREMENT_HI ) << 32;
    Config->StcIncrement |= (uint64_t) BCHP_GET_FIELD_DATA(
        BREG_Read32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_LO, hPcrOffset->WhichStc ) ),
        XPT_PCROFFSET_STC0_INC_LO, INCREMENT_LO );
}

BERR_Code BXPT_PcrOffset_SetNRTConfig(
    BXPT_PcrOffset_Handle hPcrOffset,   /* [in] The channel handle */
    const BXPT_PcrOffset_NRTConfig *Config
    )
{
    uint32_t Reg, RegAddr;
    unsigned WindowMultiplier;

    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned TriggerMode = 0;

    BDBG_ASSERT( hPcrOffset );
    BDBG_ASSERT( Config );

#if BXPT_HAS_STC_TRIG_TYPE
    switch( Config->TriggerMode )
    {
        case BXPT_PcrOffset_StcTriggerMode_eTimebase:
        TriggerMode = 0;
        break;

        case BXPT_PcrOffset_StcTriggerMode_eSoftIncrement:
        TriggerMode = 1;
        break;

        case BXPT_PcrOffset_StcTriggerMode_eExternalTrig:
        TriggerMode = 2;
        if( BXPT_MAX_EXTERNAL_TRIGS <= Config->ExternalTriggerNum )
        {
            BDBG_ERR(( "Invalid external trigger number %u", Config->ExternalTriggerNum ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto Done;
        }
        break;

        default:
        case BXPT_PcrOffset_StcTriggerMode_eMax:
        BDBG_ERR(( "Unknown TriggerMode" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
        break;
    }

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hPcrOffset->WhichStc );
    Reg = BREG_Read32( hPcrOffset->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_TYPE ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_SEL ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, TRIG_TYPE, TriggerMode ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, TRIG_SEL, Config->ExternalTriggerNum ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL, Config->PairedStc ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN, Config->EnableAvWindowComparison ? 1 : 0 )
    );
    BREG_Write32( hPcrOffset->hReg, RegAddr, Reg );
#else
    switch( Config->TriggerMode )
    {
        case BXPT_PcrOffset_StcTriggerMode_eTimebase:
        TriggerMode = 0;
        break;

        case BXPT_PcrOffset_StcTriggerMode_eExternalTrig:
        if( BXPT_MAX_EXTERNAL_TRIGS <= Config->ExternalTriggerNum )
        {
            BDBG_ERR(( "Invalid external trigger number", Config->ExternalTriggerNum ));
            ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
            goto Done;
        }
        if( Config->ExternalTriggerNum < 2 )
            TriggerMode = Config->ExternalTriggerNum + 1;    /* Trigs 0 and 1 map to TRIG_MODE 1 and 2 */
        else
            TriggerMode = Config->ExternalTriggerNum + 2;    /* Trigs 2 and up map to TRIG_MODE 4 and up */
        break;

        case BXPT_PcrOffset_StcTriggerMode_eSoftIncrement:
        TriggerMode = 3;
        break;

        default:
        case BXPT_PcrOffset_StcTriggerMode_eMax:
        BDBG_ERR(( "Unknown TriggerMode" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
        break;
    }

    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_CTRL, hPcrOffset->WhichStc );
    Reg = BREG_Read32( hPcrOffset->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, TRIG_MODE ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL ) |
        BCHP_MASK( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, TRIG_MODE, TriggerMode ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, STC_AV_SEL, Config->PairedStc ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_CTRL, AV_WINDOW_EN, Config->EnableAvWindowComparison ? 1 : 0 )
    );
    BREG_Write32( hPcrOffset->hReg, RegAddr, Reg );
#endif

    /* The units used in AV_WINDOW change with the channels' mode (either MOD300 or Binary).
    ** For MOD300, the units are 45kH ticks, whereas in binary they are 27 MHz. We want to scale this so
    ** that the AvWindow is expressed as mS of delay. */
    WindowMultiplier = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC0_CTRL, MODE ) ? 27000 : 45;
    RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_AV_WINDOW, hPcrOffset->WhichStc );
    Reg = BREG_Read32( hPcrOffset->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_AV_WINDOW, AV_WINDOW, Config->AvWindow * WindowMultiplier )
    );
    BREG_Write32( hPcrOffset->hReg, RegAddr, Reg );

#if 0
    if( BXPT_PcrOffset_StcTriggerMode_eSoftIncrement != Config->TriggerMode )
#endif
    {
        RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_HI, hPcrOffset->WhichStc );
        Reg = BREG_Read32( hPcrOffset->hReg, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_PCROFFSET_STC0_INC_HI, INCREMENT_HI )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_INC_HI, INCREMENT_HI, (uint32_t) (Config->StcIncrement >> 32) )
        );
        BREG_Write32( hPcrOffset->hReg, RegAddr, Reg );

        RegAddr = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_LO, hPcrOffset->WhichStc );
        Reg = BREG_Read32( hPcrOffset->hReg, RegAddr );
        Reg &= ~(
            BCHP_MASK( XPT_PCROFFSET_STC0_INC_LO, INCREMENT_LO )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_PCROFFSET_STC0_INC_LO, INCREMENT_LO, (uint32_t) (Config->StcIncrement & 0xFFFFFFFF) )
        );
        BREG_Write32( hPcrOffset->hReg, RegAddr, Reg );
    }

    Done:
    return ExitCode;
}

void BXPT_PcrOffset_GetSoftIncrementRegisterOffsets(
    BXPT_PcrOffset_Handle hPcrOffset,   /* [in] The channel handle */
    BAVC_Xpt_StcSoftIncRegisters *RegMap
    )
{
    BDBG_ASSERT( hPcrOffset );
    BDBG_ASSERT( RegMap );

    RegMap->StcIncLo = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_LO, hPcrOffset->WhichStc );
    RegMap->StcIncHi = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_HI, hPcrOffset->WhichStc );
    RegMap->IncTrigger = GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_TRIG, hPcrOffset->WhichStc );
}

void BXPT_PcrOffset_TriggerStcIncrement(
    BXPT_PcrOffset_Handle hPcrOffset   /* [in] The channel handle */
    )
{
    BDBG_ASSERT( hPcrOffset );

    /* any write will do it */
    BREG_Write32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_TRIG, hPcrOffset->WhichStc ), 0x0 );
#if 0
    BREG_Write32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_TRIG, hPcrOffset->WhichStc ), 0x1 );
    BREG_Write32( hPcrOffset->hReg, GetStcRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC0_INC_TRIG, hPcrOffset->WhichStc ), 0x0 );
#endif
}

#endif

void BXPT_PcrOffset_Acquire(
    BXPT_PcrOffset_Handle hPcrOff      /* [in] Handle for the PCR Offset channel to use */
    )
{
    BKNI_EnterCriticalSection();
    BXPT_PcrOffset_Acquire_isr( hPcrOff );
    BKNI_LeaveCriticalSection();
    return;
}

void BXPT_PcrOffset_Acquire_isr(
    BXPT_PcrOffset_Handle hChannel
    )
{
    uint32_t Reg;

    BDBG_ASSERT( hChannel );

    Reg = BREG_Read32( hChannel->hReg, hChannel->BaseAddr + PCROFF_OFFSET_STATE );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_CONTEXT0_OFFSET_STATE, OFFSET_STATE )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_CONTEXT0_OFFSET_STATE, OFFSET_STATE, 1 )
    );
    BREG_Write32( hChannel->hReg, hChannel->BaseAddr + PCROFF_OFFSET_STATE, Reg );
}

#if BXPT_NUM_STC_SNAPSHOTS > 0
BXPT_PcrOffset_StcSnapshot BXPT_PcrOffset_AllocStcSnapshot(
    BXPT_Handle hXpt,           /* [in] Handle for this transport */
    unsigned WhichStc           /* [in] which STC index to snapshoot */
    )
{
    BXPT_PcrOffset_StcSnapshot Snapshot = NULL;
    uint32_t Reg, RegAddr;
    unsigned ii;

    BDBG_ASSERT( hXpt );
    BDBG_ASSERT( WhichStc < BXPT_NUM_STCS );

#if BXPT_HAS_STC_SNAPSHOT_XBAR
    for( ii = 0; ii < BXPT_NUM_STC_SNAPSHOTS; ii++ )
    {
        if( !hXpt->StcSnapshots[ ii ].Allocated )
        {
            Snapshot = &( hXpt->StcSnapshots[ ii ] );
            break;
        }
    }
#else
    if ( !hXpt->StcSnapshots[ WhichStc ].Allocated )
    {
        Snapshot = &( hXpt->StcSnapshots[ WhichStc ] );
    }
#endif

    if (Snapshot)
    {
        Snapshot->Allocated = true;
        Snapshot->WhichStc = WhichStc;

        RegAddr = GetStcSnapshotRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, Snapshot->Index );
        Reg = BREG_Read32( Snapshot->hReg, RegAddr );
        Reg &= ~(
#if BXPT_HAS_STC_SNAPSHOT_XBAR
            BCHP_MASK( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_STC_SEL ) |
#endif
            BCHP_MASK( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_MODE ) |
            BCHP_MASK( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_TRIG_SEL )
        );
        Reg |= (
#if BXPT_HAS_STC_SNAPSHOT_XBAR
            BCHP_FIELD_DATA( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_STC_SEL, WhichStc ) |
#endif
            BCHP_FIELD_DATA( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_MODE, 0 /* legacy 42-bit */ ) |
            BCHP_FIELD_DATA( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_TRIG_SEL, 0 /* external trigger 0 */ )
        );
        BREG_Write32( Snapshot->hReg, RegAddr, Reg );
    }
    else
    {
        BDBG_ERR(( "No stc snapshots available." ));
    }

    return Snapshot;
}

void BXPT_PcrOffset_FreeStcSnapshot(
    BXPT_PcrOffset_StcSnapshot StcSnapshot
    )
{
    BDBG_ASSERT( StcSnapshot );
    StcSnapshot->Allocated = false;
}

void BXPT_PcrOffset_GetStcSnapshotConfig(
    BXPT_PcrOffset_StcSnapshot Snapshot,        /* [in] Handle for the allocated snapshot */
    BXPT_PcrOffset_StcSnapshotSettings *Config   /* [out] The snapshot settings */
    )
{
    uint32_t Reg;

    BDBG_ASSERT( Snapshot );
    BDBG_ASSERT( Config );

    Reg = BREG_Read32( Snapshot->hReg, GetStcSnapshotRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, Snapshot->Index ) );
    Config->ExternalTriggerNum = BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_TRIG_SEL );
    switch( BCHP_GET_FIELD_DATA( Reg, XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_MODE ) )
    {
        case 0:
        Config->Mode = BXPT_PcrOffset_StcSnapshotMode_eLegacy;
        break;

        case 1:
        Config->Mode = BXPT_PcrOffset_StcSnapshotMode_eLsb32;
        break;

        case 2:
        Config->Mode = BXPT_PcrOffset_StcSnapshotMode_eMsb32;
        break;
    }
}

BERR_Code BXPT_PcrOffset_SetStcSnapshotConfig(
    BXPT_PcrOffset_StcSnapshot Snapshot,            /* [in] Handle for the allocated snapshot */
    const BXPT_PcrOffset_StcSnapshotSettings *Config /* [in] The snapshot settings */
    )
{
    uint32_t Reg, RegAddr;
    BERR_Code ExitCode = BERR_SUCCESS;
    unsigned Mode = 0;

    BDBG_ASSERT( Snapshot );
    BDBG_ASSERT( Config );

    if( BXPT_MAX_EXTERNAL_TRIGS <= Config->ExternalTriggerNum )
    {
        BDBG_ERR(( "Invalid external trigger number %d", Config->ExternalTriggerNum ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
    }

    switch( Config->Mode )
    {
        case BXPT_PcrOffset_StcSnapshotMode_eLegacy:
        Mode = 0;
        break;

        case BXPT_PcrOffset_StcSnapshotMode_eLsb32:
        Mode = 1;
        break;

        case BXPT_PcrOffset_StcSnapshotMode_eMsb32:
        Mode = 2;
        break;

        default:
        case BXPT_PcrOffset_StcSnapshotMode_eMax:
        BDBG_ERR(( "Unknown Mode" ));
        ExitCode = BERR_TRACE( BERR_INVALID_PARAMETER );
        goto Done;
        break;
    }

    RegAddr = GetStcSnapshotRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, Snapshot->Index );
    Reg = BREG_Read32( Snapshot->hReg, RegAddr );
    Reg &= ~(
        BCHP_MASK( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_MODE ) |
        BCHP_MASK( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_TRIG_SEL )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_MODE, Mode ) |
        BCHP_FIELD_DATA( XPT_PCROFFSET_STC_SNAPSHOT0_CTRL, SNAPSHOT_TRIG_SEL, Config->ExternalTriggerNum )
    );
    BREG_Write32( Snapshot->hReg, RegAddr, Reg );

    Done:
    return ExitCode;
}

void BXPT_PcrOffset_GetStcSnapshotRegisterOffsets(
    BXPT_PcrOffset_StcSnapshot Snapshot,   /* [in] The channel handle */
    BXPT_PcrOffset_StcSnapshotRegisters *RegMap
    )
{
    BDBG_ASSERT( Snapshot );
    BDBG_ASSERT( RegMap );

    RegMap->StcLo = GetStcSnapshotRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_LO, Snapshot->Index );
    RegMap->StcHi = GetStcSnapshotRegAddr_isrsafe( BCHP_XPT_PCROFFSET_STC_SNAPSHOT0_HI, Snapshot->Index );
}
#endif
