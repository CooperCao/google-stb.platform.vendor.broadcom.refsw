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
#include "bkni.h"
#include "bxpt_pcr.h"

#if BXPT_NUM_PCRS > 1
#include "bchp_xpt_dpcr1.h"
#endif
#include "bchp_xpt_dpcr_pp.h"
#include "bxpt_priv.h"

BDBG_MODULE( xpt_pcr );

#define XPR_PCR_DEF_MAX_PCR_ERROR  100

/* Support for automatically scaling the number of DPCR blocks. */
#ifdef BCHP_XPT_DPCR1_PID_CH
    #define BXPT_PCR_P_REGOFFSET  (BCHP_XPT_DPCR1_PID_CH - BCHP_XPT_DPCR0_PID_CH)
#else
    #define BXPT_PCR_P_REGOFFSET  (BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP - BCHP_XPT_DPCR0_PID_CH)
#endif

#define RESET_FILTA   7
#define RESET_FILTB   8
#define RESET_FILTC   3
#define RESET_TRACK_RANGE   3

static void SetPcrIncAndPrescale_isr( BXPT_PCR_Handle hPcr, const BXPT_PCR_TimebaseFreqRefConfig *Timebasefreq );
static void GetPcrIncAndPrescale( BXPT_PCR_Handle hPcr, BXPT_PCR_TimebaseFreqRefConfig  *TimebaseFreqConfig );

static BERR_Code BXPT_PCR_P_Lock2Source_isr(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimeRef eTimeRef,
    BXPT_PCR_TimebaseFreqRefConfig *Timebasefreq
    )
{
    uint32_t      Reg;
    uint8_t     FiltA, FiltB, FiltC;
    BERR_Code     ret = BERR_SUCCESS;
    BREG_Handle   hRegister = hPcr->hRegister;

    /* Always allow the PRESCALE and PCR_INC to be adjusted */
    if (Timebasefreq && eTimeRef != BXPT_PCR_TimeRef_eXpt)
    {
        SetPcrIncAndPrescale_isr( hPcr, Timebasefreq );
    }

    /*
    ** Don't write to the loop filter or polarity bitfields if the values aren't being
    ** changed. It can cause a glitch that interferes with the HDCP authentication.
    */
    if( Timebasefreq )
    {
        FiltA = Timebasefreq->FiltA;
        FiltB = Timebasefreq->FiltB;
        FiltC = Timebasefreq->FiltC;
    }
    else
    {
        FiltA = RESET_FILTA;
        FiltB = RESET_FILTB;
        FiltC = RESET_FILTC;
    }

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    hPcr->FiltB = FiltB;
    hPcr->FiltC = FiltC;
#endif

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_LOOP_CTRL+ hPcr->RegOffset);

    if( BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, TIME_REF ) != eTimeRef
    || BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_C ) != FiltC
    || BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_B ) != FiltB
    || BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_A ) != FiltA
    )
    {
        /* First Set Ref_Polarity to 0-None */
        Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_LOOP_CTRL+ hPcr->RegOffset);
        Reg &= ~(BCHP_MASK( XPT_DPCR0_LOOP_CTRL, REF_POLARITY ));
        BREG_Write32( hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg);

        /* do the follow 2 steps if time reference is non-xpt*/
        /* 1) change frequency time reference in XPT_DPCR0_REF_PCR*/
        /* 2) Set the packetmode to 1 for non_xpt reference*/
            /*
            Does it mean that we are in direct TV mode?
            Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_CTRL + hPcr->RegOffset);
            Reg &= ~(BCHP_MASK( XPT_DPCR0_CTRL, PCR_PACKET_MODE ));
            Reg |= BCHP_FIELD_DATA(XPT_DPCR0_CTRL, PCR_PACKET_MODE,1);
            BREG_Write32( hRegister, BCHP_XPT_DPCR0_CTRL + hPcr->RegOffset, Reg);
            */

        Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_CTRL + hPcr->RegOffset);
        Reg &= ~(BCHP_MASK( XPT_DPCR0_CTRL, PCR_PACKET_MODE ));
        Reg |= BCHP_FIELD_DATA( XPT_DPCR0_CTRL, PCR_PACKET_MODE, hPcr->DMode == true ? 1 : 0 );
        BREG_Write32( hRegister, BCHP_XPT_DPCR0_CTRL + hPcr->RegOffset, Reg);

        /* Set the Time_Ref */
        /* Change the loop constants, if they where given. Otherwise, reload the reset defaults. */
        Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_LOOP_CTRL+ hPcr->RegOffset);
        Reg &= ~(
            BCHP_MASK( XPT_DPCR0_LOOP_CTRL, TIME_REF ) |
            BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_C ) |
            BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_B ) |
            BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_A )
        );
        Reg |= (
            BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, TIME_REF, eTimeRef ) |
            BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_C, FiltC ) |
            BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_B, FiltB ) |
            BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_A, FiltA )
        );
        BREG_Write32( hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg);
        BDBG_MSG(( "DPCR%u TIME_REF 0x%X, FILT_A 0x%X, FILT_B 0x%X, FILT_C 0x%X",
            hPcr->ChannelNo, eTimeRef, FiltA, FiltB, FiltC ));

        /* Set Ref_Polarity to 1*/
        Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_LOOP_CTRL+ hPcr->RegOffset);
        Reg &= ~(BCHP_MASK( XPT_DPCR0_LOOP_CTRL, REF_POLARITY ));
        Reg |= BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, REF_POLARITY, 1);
        BREG_Write32( hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg);

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
        if(hPcr->eTimeRef != eTimeRef)
        {
            hPcr->ChangingToNonPcrSource = BXPT_PCR_TimeRef_eXpt != eTimeRef ? true : false;
            hPcr->eTimeRef = eTimeRef;
        }
        else
        {
            hPcr->ChangingToNonPcrSource = false;
        }
#endif
    }

    return ret;
}


#if (!B_REFSW_MINIMAL)
BERR_Code BXPT_PCR_GetTotalChannels(
    BXPT_Handle hXpt,
    unsigned int *TotalChannels
    )
{
    BERR_Code  ret = BERR_SUCCESS;
    *TotalChannels = hXpt->MaxPcrs;
    if(!(hXpt->MaxPcrs))
    {
        BDBG_ERR(("No PCR Channels Available!"));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return ret;
}
#endif

BERR_Code BXPT_PCR_GetChannelDefSettings(
    BXPT_Handle hXpt,                       /* [in] The transport handle - need chip info */
    unsigned int WhichPcr,                      /* [In] Which pcr module */
    BXPT_PCR_DefSettings *pcrSettings       /* [out] The default settings of a pcr module */
    )
{
    BERR_Code  ret = BERR_SUCCESS;

    BKNI_Memset( (void *) pcrSettings, 0, sizeof(*pcrSettings) );
    if ( WhichPcr >= hXpt->MaxPcrs )
    {
        ret = BERR_TRACE( BERR_INVALID_PARAMETER );
        BDBG_ERR(("Pcr number %d exceeds max PCR channels!",WhichPcr));
        return ret;
    }
    /* the current default setting for all the PCR blocks are the same*/
    pcrSettings -> PcrTwoErrReaquireEn = 1;
    pcrSettings -> MaxPcrError = XPR_PCR_DEF_MAX_PCR_ERROR;
    return ret;
}


BERR_Code BXPT_PCR_Open(
    BXPT_Handle hXpt,
    unsigned int WhichPcr,
    BXPT_PCR_DefSettings *pcrSettings,
    BXPT_PCR_Handle *hPcr
    )
{
    BXPT_PCR_Handle     handle;
    BREG_Handle         hRegister = hXpt->hRegister;
    uint32_t            Reg;
    BERR_Code           ret = BERR_SUCCESS;

    BDBG_ASSERT( pcrSettings );
    if ( WhichPcr >= BXPT_NUM_PCRS )
    {
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        BDBG_ERR(("Pcr number %d exceeds max PCR channels!",WhichPcr));
        return ret;
    }
    /* allocate pcr channel handle */
    handle = (BXPT_P_PcrHandle_Impl * )BKNI_Malloc( sizeof(BXPT_P_PcrHandle_Impl));
    if ( handle == NULL )
    {
        ret = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        return ret;
    }

    handle->hRegister = hXpt->hRegister;
    handle->ChannelNo = WhichPcr;
    handle->RegOffset = WhichPcr * BXPT_PCR_P_REGOFFSET;
    handle->DMode = false;
    handle->vhXpt = ( void * ) hXpt;
    handle->pidChnlConfigured = false;

    hXpt->PcrHandles[WhichPcr] = handle;
    *hPcr = handle;

    /* set the default settings */
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_MAX_PCR_ERROR + handle->RegOffset, pcrSettings->MaxPcrError);

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_CTRL + handle->RegOffset);
    Reg &= ~(BCHP_MASK( XPT_DPCR0_CTRL, PCR_TWO_ERR_REACQUIRE_EN ));
    Reg |=  BCHP_FIELD_DATA( XPT_DPCR0_CTRL, PCR_TWO_ERR_REACQUIRE_EN, pcrSettings->PcrTwoErrReaquireEn);
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_CTRL + handle->RegOffset, Reg);

    /* This reg is shared by all DPCR instances, so init only on the first open. */
#ifdef BXPT_P_JITTER_CORRECTION_FIX
    if( !hXpt->DpcrRefCount++ )
    {
        Reg = 0;
        Reg |=  BCHP_FIELD_DATA( XPT_DPCR_PP_PP_CTRL, PP_PLAYBACK_PCR_JITTER_DIS, 1 );
        BREG_Write32( hRegister, BCHP_XPT_DPCR_PP_PP_CTRL, Reg );
    }
#endif

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    handle->FiltB = RESET_FILTB;
    handle->FiltC = RESET_FILTC;
    handle->TrackRange = RESET_TRACK_RANGE;
    handle->Accum = 0;
    handle->InErrState = false;
    handle->PcrCount = 0;
    handle->PcrThreshold = 0;
    handle->ChangingToNonPcrSource = false;
    handle->PhaseErrorDelay = 0;
    handle->PcrInc = 249856;
    handle->eTimeRef = BXPT_PCR_TimeRef_eInternal;

    Reg = BREG_Read32( handle->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + handle->RegOffset );
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FREEZE_INTEGRATOR, 1 );
    BREG_Write32( handle->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + handle->RegOffset, Reg );
#else
    Reg = BREG_Read32( handle->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + handle->RegOffset );
    BCHP_SET_FIELD_DATA(Reg, XPT_DPCR0_LOOP_CTRL, FREEZE_INTEGRATOR, 0);
    BREG_Write32( handle->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + handle->RegOffset, Reg );
#endif

    BREG_Write32( handle->hRegister, BCHP_XPT_DPCR0_INTR_STATUS_REG, 0 );

    return ret;
}


BERR_Code BXPT_PCR_Close(
    BXPT_PCR_Handle hPcr
    )
{
    uint32_t      Reg;
    BERR_Code     ret = BERR_SUCCESS;
    BREG_Handle   hRegister = hPcr->hRegister;
    BXPT_Handle   lhXpt = ( BXPT_Handle ) hPcr->vhXpt;

    lhXpt->PcrHandles[ hPcr->ChannelNo ] = ( void * ) NULL;

#ifdef BXPT_P_JITTER_CORRECTION_FIX
    if( ! --lhXpt->DpcrRefCount )
    {
        BREG_Write32( hRegister, BCHP_XPT_DPCR_PP_PP_CTRL, 0 );
    }
#endif

    /* set pid_valid_bit to 0 in case it is still active*/
    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    Reg &= ~(BCHP_MASK( XPT_DPCR0_PID_CH , PCR_PID_CH_VALID));
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_PID_CH + hPcr->RegOffset, Reg);

    BKNI_Free(hPcr);
    return ret;
}


BERR_Code   BXPT_PCR_GetStreamPcrConfig(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_XptStreamPcrCfg *PcrCfg
    )
{
    BREG_Handle         hRegister = hPcr->hRegister;
    BXPT_Handle hXpt = (BXPT_Handle) hPcr->vhXpt;
    uint32_t            Reg;
    uint32_t StreamSelect;
    BERR_Code           ret = BERR_SUCCESS;

    BDBG_ENTER(BXPT_PCR_GetStreamPcrConfig);

    BSTD_UNUSED( StreamSelect );

    PcrCfg->JitterTimestamp = hXpt->JitterTimestamp[ hPcr->ChannelNo ];
    PcrCfg->PbJitterDisable = hXpt->PbJitterDisable[ hPcr->ChannelNo ];
    PcrCfg->LiveJitterDisable = hXpt->LiveJitterDisable[ hPcr->ChannelNo ];

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    PcrCfg->PidChannel = BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_PID_CH , PCR_PID_CH);
    PcrCfg->MaxPcrError = BREG_Read32( hRegister, BCHP_XPT_DPCR0_MAX_PCR_ERROR + hPcr->RegOffset );
    ret = BXPT_PCR_GetfreqRefConfig( hPcr, &PcrCfg->TimebaseCfg );

    BDBG_LEAVE(BXPT_PCR_GetStreamPcrConfig);

    return ret;
}

BERR_Code   BXPT_PCR_SetStreamPcrConfig(
    BXPT_PCR_Handle hPcr,
    const BXPT_PCR_XptStreamPcrCfg *PcrCfg
    )
{
    BERR_Code ret = BERR_SUCCESS;

    BKNI_EnterCriticalSection();
    ret = BXPT_PCR_SetStreamPcrConfig_isr( hPcr, PcrCfg );
    BKNI_LeaveCriticalSection();
    return ret;
}

BERR_Code   BXPT_PCR_SetStreamPcrConfig_isr(
    BXPT_PCR_Handle hPcr,
    const BXPT_PCR_XptStreamPcrCfg *PcrCfg
    )
{
    BREG_Handle         hRegister = hPcr->hRegister;
    BXPT_Handle hXpt = (BXPT_Handle) hPcr->vhXpt;
    uint32_t            Reg;
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            tempStreamSelect = 0;
    unsigned JitterTimestamp, PbJitterDisable, LiveJitterDisable;

    BDBG_ENTER(BXPT_PCR_SetStreamPcrConfig_isr);

    BSTD_UNUSED( tempStreamSelect );

    /* SWSTB-1525: Bug in the jitter correction hw. Recommended solution is to force a fixed offset of 0
     * This overrided the jitter disable bits
     * */
#ifndef BXPT_P_JITTER_CORRECTION_FIX
    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR_PP_PP_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_DPCR_PP_PP_CTRL, PP_FIXED_OFFSET_EN )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_DPCR_PP_PP_CTRL, PP_FIXED_OFFSET_EN, 1 )
        );
    BREG_Write32( hRegister, BCHP_XPT_DPCR_PP_PP_CTRL, Reg );

    BREG_Write32( hRegister, BCHP_XPT_DPCR_PP_PP_FIXED_OFFSET, 0 );
#endif

    if( PcrCfg->JitterTimestamp >= BXPT_PCR_JitterTimestampMode_eMax )
    {
        BDBG_ERR(( "Invalid jitter timestamp mode %u", PcrCfg->JitterTimestamp ));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto Done;
    }

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR_PP_PP_CTRL );
    JitterTimestamp = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR_PP_PP_CTRL , PP_JITTER_TIMESTAMP_MODE );
    PbJitterDisable = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR_PP_PP_CTRL , PP_PLAYBACK_PCR_JITTER_DIS );
    LiveJitterDisable = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR_PP_PP_CTRL , PP_LIVE_PCR_JITTER_DIS );

    if( PcrCfg->JitterTimestamp != BXPT_PCR_JitterTimestampMode_eAuto )
    {
        JitterTimestamp = PcrCfg->JitterTimestamp;
    }
    hXpt->JitterTimestamp[ hPcr->ChannelNo ] = PcrCfg->JitterTimestamp;

    if( PcrCfg->PbJitterDisable != BXPT_PCR_JitterCorrection_eAuto )
    {
        PbJitterDisable = PcrCfg->PbJitterDisable;
    }
    hXpt->PbJitterDisable[ hPcr->ChannelNo ] = PcrCfg->PbJitterDisable;

    if( PcrCfg->LiveJitterDisable != BXPT_PCR_JitterCorrection_eAuto )
    {
        LiveJitterDisable = PcrCfg->LiveJitterDisable;
    }
    hXpt->LiveJitterDisable[ hPcr->ChannelNo ] = PcrCfg->LiveJitterDisable;

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR_PP_PP_CTRL );
    Reg &= ~(
        BCHP_MASK( XPT_DPCR_PP_PP_CTRL , PP_JITTER_TIMESTAMP_MODE ) |
        BCHP_MASK( XPT_DPCR_PP_PP_CTRL , PP_PLAYBACK_PCR_JITTER_DIS ) |
        BCHP_MASK( XPT_DPCR_PP_PP_CTRL , PP_LIVE_PCR_JITTER_DIS )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_DPCR_PP_PP_CTRL, PP_JITTER_TIMESTAMP_MODE, JitterTimestamp ) |
        BCHP_FIELD_DATA( XPT_DPCR_PP_PP_CTRL, PP_PLAYBACK_PCR_JITTER_DIS, PbJitterDisable ) |
        BCHP_FIELD_DATA( XPT_DPCR_PP_PP_CTRL, PP_LIVE_PCR_JITTER_DIS, LiveJitterDisable )
        );
    BREG_Write32( hRegister, BCHP_XPT_DPCR_PP_PP_CTRL, Reg );

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    Reg &= ~(
        BCHP_MASK( XPT_DPCR0_PID_CH , PCR_PID_CH_VALID ) |
        BCHP_MASK( XPT_DPCR0_PID_CH , PCR_PID_CH )
        );
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_PID_CH, PCR_PID_CH, PcrCfg->PidChannel );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_PID_CH + hPcr->RegOffset, Reg);
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_MAX_PCR_ERROR + hPcr->RegOffset, PcrCfg->MaxPcrError );

    /* Set the timebase select to xpt source */
    ret = BERR_TRACE( BXPT_PCR_P_Lock2Source_isr( hPcr, BXPT_PCR_TimeRef_eXpt, NULL ));

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    hPcr->FiltB = PcrCfg->TimebaseCfg.FiltB;
    hPcr->FiltC = PcrCfg->TimebaseCfg.FiltC;
#endif

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_LOOP_CTRL+ hPcr->RegOffset);
    Reg &= ~(
        BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_C ) |
        BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_B ) |
        BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FILT_A )
    );
    Reg |= (
        BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_C, PcrCfg->TimebaseCfg.FiltC ) |
        BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_B, PcrCfg->TimebaseCfg.FiltB ) |
        BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FILT_A, PcrCfg->TimebaseCfg.FiltA )
    );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg);

    SetPcrIncAndPrescale_isr( hPcr, &PcrCfg->TimebaseCfg );

    /* Set PCR Valid bit to 1 */
    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_PID_CH, PCR_PID_CH_VALID, 1 );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_PID_CH + hPcr->RegOffset, Reg);

    Done:
    hPcr->pidChnlConfigured = true;
    BDBG_LEAVE(BXPT_PCR_SetStreamPcrConfig_isr);

    return ret;
}

#if (!B_REFSW_MINIMAL)
void BXPT_PCR_RefreshPcrPid(
    BXPT_PCR_Handle hPcr
    )
{
    BKNI_EnterCriticalSection();
    BXPT_PCR_RefreshPcrPid_isr(hPcr);
    BKNI_LeaveCriticalSection();
    return ;
}
#endif

void BXPT_PCR_RefreshPcrPid_isr(
    BXPT_PCR_Handle hPcr               /*[in] The pcr handle  */
    )
{
    uint32_t Reg;

    if(!hPcr->pidChnlConfigured)
    {
        return; /* Ignore calls if they haven't pidChnlConfigured the hardware. */
    }

    Reg = BREG_Read32(hPcr->hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    Reg &= ~(BCHP_MASK( XPT_DPCR0_PID_CH , PCR_PID_CH_VALID ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_PID_CH, PCR_PID_CH_VALID, 1 );
    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_PID_CH + hPcr->RegOffset, Reg);
    return;
}

#if (!B_REFSW_MINIMAL)
BERR_Code   BXPT_PCR_GetLastPcr(
    BXPT_PCR_Handle hPcr,
    uint32_t *p_pcrHi,
    uint32_t *p_pcrLo
    )
{
    BERR_Code ret = BERR_SUCCESS;

    BKNI_EnterCriticalSection();
    ret = BXPT_PCR_GetLastPcr_isr(hPcr, p_pcrHi, p_pcrLo);
    BKNI_LeaveCriticalSection();
    return ret;
}
#endif

BERR_Code   BXPT_PCR_GetLastPcr_isr(
    BXPT_PCR_Handle hPcr,
    uint32_t *p_pcrHi,
    uint32_t *p_pcrLo
    )
{
    BREG_Handle         hRegister = hPcr->hRegister;
    uint32_t            Reg;
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            Offset = hPcr->RegOffset;

    BDBG_ENTER(BXPT_PCR_GetLastPcr);

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_CTRL + Offset);
    if ( BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_CTRL, PCR_PACKET_MODE) )
    {
        BDBG_ERR(("Unsupported PI for this mode"));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        return ret;
    }
    *p_pcrHi = BREG_Read32(hRegister,BCHP_XPT_DPCR0_LAST_PCR_HI + Offset);
    *p_pcrLo = BREG_Read32(hRegister,BCHP_XPT_DPCR0_LAST_PCR_LO + Offset);

    BDBG_LEAVE(BXPT_PCR_GetLastPcr);

    return ret;

}

BERR_Code   BXPT_PCR_GetPhaseError_isr(
    BXPT_PCR_Handle hPcr,
    int32_t *p_error
    )
{
    BREG_Handle         hRegister = hPcr->hRegister;
    uint32_t            Reg;
    bool                sign;
    uint32_t            magnitude;
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            Offset = hPcr->RegOffset;

    BDBG_ENTER(BXPT_PCR_GetPhaseError_isr);

    Reg = BREG_Read32(hRegister,BCHP_XPT_DPCR0_PHASE_ERROR + Offset);
    Reg = BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_PHASE_ERROR, PHASE_ERROR);

    /* stored as 2's complement */
    sign = ((Reg & 0x100000) >> 20) ? true : false; /* sign bit is bit 20 */
        magnitude = Reg;
    if (sign) magnitude = ~Reg + 1; /* 2's complement */
    magnitude &= 0x0fffff; /* magnitude is 19 to 0 */

    /*  bits 10:1 of pcr base - stc base are stored in 19:10 here.
    bit 9 has bit 0 of base diff. bits 8:0 have extension diff. */
    *p_error = (sign ? -1 : 1) * (magnitude >> 9);
    BDBG_LEAVE(BXPT_PCR_GetPhaseError_isr);

    return ret;

}


#if (!B_REFSW_MINIMAL)
BERR_Code   BXPT_PCR_GetStc(
    BXPT_PCR_Handle hPcr,
    uint32_t *p_stcHi,
    uint32_t *p_stcLo
    )
{
    BREG_Handle         hRegister = hPcr->hRegister;
    uint32_t            Reg;
    BERR_Code           ret = BERR_SUCCESS;
    uint32_t            Offset = hPcr->RegOffset;

    BDBG_ENTER(BXPT_PCR_GetStc);
    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_CTRL + Offset);
    if ( BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_CTRL, PCR_PACKET_MODE) )
    {
        BDBG_ERR(("Unsupported PI for this mode"));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        return ret;
    }
    *p_stcHi = BREG_Read32(hRegister,BCHP_XPT_DPCR0_STC_HI+Offset);
    *p_stcLo = BREG_Read32(hRegister,BCHP_XPT_DPCR0_STC_LO+Offset);
    BDBG_LEAVE(BXPT_PCR_GetStc);

    return ret;
}
#endif

#if (!B_REFSW_MINIMAL)
BERR_Code   BXPT_PCR_SetStcExtRateControl(
    BXPT_PCR_Handle hPcr,
    const BXPT_PCR_STCExtRateConfig  *StcExtRateCfg
    )
{

    uint32_t            Reg;
    uint32_t            Offset = hPcr->RegOffset;
    BERR_Code           ret = BERR_SUCCESS;
    BREG_Handle         hRegister = hPcr->hRegister;

    BDBG_ENTER(BXPT_PCR_SetStcExtRateControl);

    BDBG_MSG(( "StcExtRateControl: Prescale %lu, Inc %lu", StcExtRateCfg->Prescale, StcExtRateCfg->Inc ));

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_STC_EXT_CTRL+Offset);

    Reg &= ~(
        BCHP_MASK( XPT_DPCR0_STC_EXT_CTRL, PRESCALE ) |
        BCHP_MASK( XPT_DPCR0_STC_EXT_CTRL, INC_VAL )
        );

    Reg |= (
        BCHP_FIELD_DATA(XPT_DPCR0_STC_EXT_CTRL,PRESCALE,StcExtRateCfg->Prescale)|
        BCHP_FIELD_DATA(XPT_DPCR0_STC_EXT_CTRL,INC_VAL,StcExtRateCfg->Inc)
        );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_STC_EXT_CTRL+Offset, Reg);

    BDBG_LEAVE(BXPT_PCR_SetStcExtRateControl);

    return ret;
}
#endif

BERR_Code   BXPT_PCR_ConfigNonStreamTimeBase(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimeRef  eNonStreamTimeBaseCfg,
    BXPT_PCR_TimebaseFreqRefConfig *Timebasefreq
    )
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER(BXPT_PCR_ConfigNonStreamTimeBase);
    BDBG_ASSERT( Timebasefreq );
    if (eNonStreamTimeBaseCfg == BXPT_PCR_TimeRef_eXpt)
    {
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto Done;
    }

    if ( eNonStreamTimeBaseCfg >= BXPT_PCR_TimeRef_eMax )
    {
        BDBG_ERR(( "Unsupported BXPT_PCR_TimeRef" ));
        ret = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto Done;
    }

    BKNI_EnterCriticalSection();
    BXPT_PCR_P_Lock2Source_isr( hPcr,eNonStreamTimeBaseCfg, Timebasefreq);
    BKNI_LeaveCriticalSection();

    Done:
    BDBG_LEAVE(BXPT_PCR_ConfigNonStreamTimeBase);
    return ret;

}


BERR_Code   BXPT_PCR_GetTimeBaseFreqRefDefaults(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimeRef TimeBase,
    BXPT_PCR_TimebaseFreqRefConfig *Def
    )
{
    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER(BXPT_PCR_GetTimeBaseFreqRefDefaults);
    BDBG_ASSERT( Def );
    BSTD_UNUSED( hPcr );

    Def->Prescale = 0;
    Def->Inc = 1;
    Def->FiltA = RESET_FILTA;
    Def->FiltB = RESET_FILTB;
    Def->FiltC = RESET_FILTC;

    switch( TimeBase )
    {
        case BXPT_PCR_TimeRef_eI656_Vl:         /* Lock to ITU 656 VSync */
        Def->Prescale = 0;
        Def->Inc = 450450;
        Def->FiltA = 7;
        Def->FiltB = 4;
        Def->FiltC = 1;
        break;

        case BXPT_PCR_TimeRef_eI656_Fl:         /* Lock to ITU 656 Frame Sync */
        Def->Prescale = 0;
        Def->Inc = 900900;
        Def->FiltA = 7;
        Def->FiltB = 4;
        Def->FiltC = 1;
        break;

        case BXPT_PCR_TimeRef_eInternal:
        Def->Prescale = 243;
        Def->Inc = 249856;
        Def->FiltA = 7;
        Def->FiltB = 4;
        Def->FiltC = 1;
        break;

        default:    /* Use defaults set before the switch() */
        break;
    }

    BDBG_LEAVE(BXPT_PCR_GetTimeBaseFreqRefDefaults);

    return ret;
}

BERR_Code   BXPT_PCR_GetfreqRefConfig(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimebaseFreqRefConfig  *TimebaseFreqConfig
    )
{
    uint32_t            Reg;
    BREG_Handle         hRegister = hPcr->hRegister;

    BERR_Code  ret = BERR_SUCCESS;

    BDBG_ENTER(BXPT_PCR_GetfreqRefConfig);

    GetPcrIncAndPrescale( hPcr, TimebaseFreqConfig );

    Reg = BREG_Read32( hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset );
    TimebaseFreqConfig->FiltA = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_A );
    TimebaseFreqConfig->FiltB = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_B );
    TimebaseFreqConfig->FiltC = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_LOOP_CTRL, FILT_C );

    BDBG_LEAVE(BXPT_PCR_GetfreqRefConfig);

    return ret;

}


void BXPT_PCR_FreezeIntegrator(
    BXPT_PCR_Handle hPcr,    /* [in] The pcr handle  */
    bool Freeze              /* [in] Freeze integrator if true, run if false. */
    )
{
#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    BDBG_MSG(( "BXPT_PCR_FreezeIntegrator() calls are ignored." ));
    BSTD_UNUSED(( hPcr ));
    BSTD_UNUSED(( Freeze ));
#else
    uint32_t Reg;

    if(!hPcr->pidChnlConfigured)
    {
        return; /* Ignore calls if they haven't pidChnlConfigured the hardware. */
    }

    Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset );
    Reg &= ~( BCHP_MASK( XPT_DPCR0_LOOP_CTRL, FREEZE_INTEGRATOR ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, FREEZE_INTEGRATOR, Freeze == true ? 1 : 0 );
    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg );

    /* Freezing the integrator requires stopping PCR processing, so mark the PCR PID as invalid */
    Reg = BREG_Read32(hPcr->hRegister, BCHP_XPT_DPCR0_PID_CH  + hPcr->RegOffset);
    Reg &= ~(BCHP_MASK( XPT_DPCR0_PID_CH , PCR_PID_CH_VALID ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_PID_CH, PCR_PID_CH_VALID, Freeze == true ? 0 : 1 );
    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_PID_CH + hPcr->RegOffset, Reg);
#endif
}

void BXPT_PCR_SetCenterFrequency(
    BXPT_PCR_Handle hPcr,       /* [in] The pcr handle  */
    uint32_t CenterFreq         /* [in] Center frequency */
    )
{
    uint32_t Reg;

    Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_CENTER + hPcr->RegOffset );

    Reg &= ~( BCHP_MASK( XPT_DPCR0_CENTER, CENTER ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_CENTER, CENTER, CenterFreq );

    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_CENTER + hPcr->RegOffset, Reg );
}

void BXPT_PCR_SetTimeRefTrackRange(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_RefTrackRange TrackRange
    )
{
    uint32_t Reg;

    Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset );

    Reg &= ~( BCHP_MASK( XPT_DPCR0_LOOP_CTRL, TRACK_RANGE ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_LOOP_CTRL, TRACK_RANGE, TrackRange );

    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_LOOP_CTRL + hPcr->RegOffset, Reg );

}

#if (!B_REFSW_MINIMAL)
void BXPT_PCR_SetPhaseErrorClampRange(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_PhaseErrorClampRange ClampRange
    )
{
    uint32_t Reg;

    Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP + hPcr->RegOffset );
    Reg &= ~( BCHP_MASK( XPT_DPCR0_PHASE_ERROR_CLAMP, PHASE_ERROR_CLAMP_RANGE ));
    Reg |= BCHP_FIELD_DATA( XPT_DPCR0_PHASE_ERROR_CLAMP, PHASE_ERROR_CLAMP_RANGE, ClampRange );
    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_PHASE_ERROR_CLAMP + hPcr->RegOffset, Reg );
}
#endif

static void SetPcrIncAndPrescale_isr(
    BXPT_PCR_Handle hPcr,
    const BXPT_PCR_TimebaseFreqRefConfig *Timebasefreq
    )
{
    uint32_t Reg;
    BREG_Handle         hRegister = hPcr->hRegister;

    BDBG_MSG(( "Prescale %u, Inc %u", (unsigned) Timebasefreq->Prescale, (unsigned) Timebasefreq->Inc ));

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_REF_PCR_PRESCALE + hPcr->RegOffset);
    Reg &= ~(
        BCHP_MASK( XPT_DPCR0_REF_PCR_PRESCALE, PRESCALE )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_DPCR0_REF_PCR_PRESCALE, PRESCALE, Timebasefreq->Prescale )
        );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_REF_PCR_PRESCALE + hPcr->RegOffset, Reg);

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_REF_PCR_INC + hPcr->RegOffset);
    Reg &= ~(
        BCHP_MASK( XPT_DPCR0_REF_PCR_INC, PCR_INC )
        );
    Reg |= (
        BCHP_FIELD_DATA( XPT_DPCR0_REF_PCR_INC, PCR_INC, Timebasefreq->Inc )
        );
    BREG_Write32( hRegister, BCHP_XPT_DPCR0_REF_PCR_INC + hPcr->RegOffset, Reg);

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND
    hPcr->PcrInc = Timebasefreq->Inc;
#endif
}

static void GetPcrIncAndPrescale(
    BXPT_PCR_Handle hPcr,
    BXPT_PCR_TimebaseFreqRefConfig  *TimebaseFreqConfig
    )
{
    uint32_t            Reg;
    BREG_Handle         hRegister = hPcr->hRegister;

    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_REF_PCR_INC + hPcr->RegOffset);
    TimebaseFreqConfig->Inc =  BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_REF_PCR_INC,PCR_INC );
    Reg = BREG_Read32(hRegister, BCHP_XPT_DPCR0_REF_PCR_PRESCALE + hPcr->RegOffset);
    TimebaseFreqConfig->Prescale = BCHP_GET_FIELD_DATA( Reg, XPT_DPCR0_REF_PCR_PRESCALE, PRESCALE );
}


BERR_Code BXPT_PCR_GetIntId(
    unsigned WhichPcr,
    BXPT_PCR_IntName Name,
    BINT_Id *IntId
    )
{
    uint32_t RegAddr;
    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( IntId );

    if(WhichPcr >= BXPT_NUM_PCRS)
    {
        BDBG_ERR(("Pcr number %u exceeds max PCR channels (%u)!",WhichPcr, BXPT_NUM_PCRS));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto Done;
    }

    switch( Name )
    {
        case BXPT_PCR_IntName_ePhaseCompare:
        case BXPT_PCR_IntName_eTwoPcrErrors:
        RegAddr = BCHP_XPT_DPCR0_INTR_STATUS_REG + ( WhichPcr * BXPT_PCR_P_REGOFFSET );
        *IntId = BCHP_INT_ID_CREATE( RegAddr, Name );
        break;

        default:
        BDBG_ERR(( "Unsupported interrupt enum %u", (unsigned) Name ));
        ExitCode = BERR_TRACE(BERR_INVALID_PARAMETER);
        break;
    }

    Done:
    return( ExitCode );
}

#if BXPT_HAS_DPCR_INTEGRATOR_WORKAROUND

static int64_t Saturate38(
    int64_t Input,
    unsigned OutputSizeSelect
    )
{
    int64_t Output = 0;
    int64_t Input37 = (Input & ((int64_t) 1 << 37)) >> 37;
    int64_t OnesCheck = 0;
    int64_t OnesSaturation = 0;
    int64_t ZerosSaturation = 0;
    int64_t MsbMask = 0;
    int64_t LsbMask = 0;

    switch( OutputSizeSelect )
    {
        case 0:     /* saturate the 38-bit input to a 31-bit two's complement number */
        OnesCheck =       (int64_t)(0x1FC000) << 16;       /* Bits 36:30 are 1 */
        OnesSaturation =  (int64_t)(0xFFFFFFFF) << 32 | 0xC0000000;    /* Was (0x0FC000) << 16 */
        ZerosSaturation = (int64_t)(0x003FFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFFF) << 32;
        LsbMask =         (int64_t)(0x003FFF) << 16 | 0xFFFF;
        break;

        case 1:     /* saturate the 38-bit input to a 32-bit two's complement number */
        OnesCheck =       (int64_t)(0x1F8000) << 16;       /* Bits 36:31 are 1 */
        OnesSaturation =  (int64_t)(0xFFFFFFFF) << 32 | 0x80000000;   /* Was (0x0F8000) << 16 */
        ZerosSaturation = (int64_t)(0x007FFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFFE) << 32;
        LsbMask =         (int64_t)(0x007FFF) << 16 | 0xFFFF;
        break;

        case 2:     /* saturate the 38-bit input to a 33-bit two's complement number */
        OnesCheck =       (int64_t)(0x1F0000) << 16;       /* Bits 36:32 are 1 */
        OnesSaturation =  (int64_t)(0xFFFFFFFF) << 32;        /* Was (0x0F0000) << 16 */
        ZerosSaturation = (int64_t)(0x00FFFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFFC) << 32;
        LsbMask =         (int64_t)(0x00FFFF) << 16 | 0xFFFF;
        break;

        case 3:     /* saturate the 38-bit input to a 34-bit two's complement number */
        OnesCheck =       (int64_t)(0x1E0000) << 16;       /* Bits 36:33 are 1 */
        OnesSaturation =  (int64_t)(0xFFFFFFFE) << 32;        /* Was (0x0E0000) << 16. Need to extend sign to 64 bits */
        ZerosSaturation = (int64_t)(0x01FFFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFF8) << 32;
        LsbMask =         (int64_t)(0x01FFFF) << 16 | 0xFFFF;
        break;

        case 4:     /* saturate the 38-bit input to a 35-bit two's complement number */
        OnesCheck =       (int64_t)(0x1C0000) << 16;
        OnesSaturation =  (int64_t)(0xFFFFFFFC) << 32;      /* Was (0x0C0000) << 16. */
        ZerosSaturation = (int64_t)(0x03FFFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFF0) << 32;
        LsbMask =         (int64_t)(0x03FFFF) << 16 | 0xFFFF;
        break;

        case 5:     /* Use as a hook for 38-bit to 36-bit. */
        default:    /* All other values, saturate the 38-bit input to a 36-bit two's complement number */
        OnesCheck =       (int64_t)(0x180000) << 16;       /* Bits 36:35 are 1 */
        OnesSaturation =  (int64_t)(0xFFFFFFF8) << 32;       /* Was (0x080000) << 16 */
        ZerosSaturation = (int64_t)(0x07FFFF) << 16 | 0xFFFF;
        MsbMask =         (int64_t)(0xFFFFFFE0) << 32;
        LsbMask =         (int64_t)(0x07FFFF) << 16 | 0xFFFF;
        break;
    }

    if( (!Input37 && (Input & OnesCheck)) || (Input37 && ( (Input & OnesCheck) != OnesCheck)) )
    {
        /* Do saturation */
        if( Input37 )
        {
            Output = OnesSaturation;
        }
        else
        {
            Output = ZerosSaturation;
        }
    }
    else
    {
        /* No saturation */
        Output = (Input & MsbMask) >> 2;
        Output |= Input & LsbMask;
    }

    return Output;
}

static int64_t CorrectLeakyIntegrator(
    BXPT_PCR_Handle hPcr,
    int64_t PcrThreshold,
    int64_t Accum,
    uint8_t TrackRange,
    uint8_t FiltB,
    uint8_t FiltC
    )
{
    int64_t NewAccum, PcrCTemp, PcrB, PcrB_PreSat, PcrB2, PcrCNew, PcrC2, PcrC3, PcrC4;

    if( PcrThreshold & (1 << 20) )
    {
        PcrThreshold |= (int64_t)(0xFFFFFFFF) << 32;
        PcrThreshold |= (int64_t) 0xFFF00000;
    }
    BDBG_MSG(( "DPCR%u: Accum " BDBG_UINT64_FMT ", PHASE_ERROR " BDBG_UINT64_FMT "", hPcr->ChannelNo, BDBG_UINT64_ARG(hPcr->Accum), BDBG_UINT64_ARG(PcrThreshold) ));

    PcrB = PcrThreshold << (15 - FiltB);   /* hPcr->FiltB is gainB */
    PcrB_PreSat =  PcrB << 2;
    PcrB2 = Saturate38(PcrB_PreSat, 5); /* TrackRange == 5 forces 38-bit to 36-bit */
    BDBG_MSG(( "DPCR%u: PcrB " BDBG_UINT64_FMT ", PcrB_PreSat " BDBG_UINT64_FMT ", PcrB2 " BDBG_UINT64_FMT "",
            hPcr->ChannelNo, BDBG_UINT64_ARG(PcrB), BDBG_UINT64_ARG(PcrB_PreSat), BDBG_UINT64_ARG(PcrB2) ));

    PcrCTemp = Accum << 5;
    PcrC2 = PcrCTemp >> (8 + FiltC);
    if( 7 == FiltC )
    {
        PcrC3 = PcrCTemp;
    }
    else
    {
        int64_t PcrC2_28_0 = PcrC2; /*  & 0x1FFFFFFF;*/
        PcrC3 = PcrCTemp - PcrC2_28_0;
    }
    BDBG_MSG(( "DPCR%u: PcrC2 " BDBG_UINT64_FMT ", PcrC3 " BDBG_UINT64_FMT "", hPcr->ChannelNo, BDBG_UINT64_ARG(PcrC2), BDBG_UINT64_ARG(PcrC3) ));

    PcrC4 = (int64_t) Saturate38( PcrC3, (uint64_t) TrackRange );
    PcrCNew = PcrB2 + PcrC4;
    NewAccum = PcrCNew;
    BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_ACCUM_VALUE + hPcr->RegOffset, NewAccum >> 5);

    BDBG_MSG(( "DPCR%u: PcrC4 " BDBG_UINT64_FMT ", Accum(new) " BDBG_UINT64_FMT ", Accum(new) >> 5 " BDBG_UINT64_FMT "",
        hPcr->ChannelNo,
        BDBG_UINT64_ARG(PcrC4), BDBG_UINT64_ARG(Accum), BDBG_UINT64_ARG(NewAccum >> 5) ));
    return NewAccum >> 5;
}

BERR_Code BXPT_PCR_P_Integrator(
    BXPT_Handle hXpt
    )
{
    int PcrNum;

    BERR_Code ExitCode = BERR_SUCCESS;

    BDBG_ASSERT( hXpt );

    for( PcrNum = 0; PcrNum < BXPT_NUM_PCRS && hXpt->PcrHandles[ PcrNum ]; PcrNum++ )
    {
        int64_t PcrThreshold;
        uint32_t Reg;
        unsigned PcrCount;

        bool PcrErrSeen = false;
        BXPT_PCR_Handle hPcr = hXpt->PcrHandles[ PcrNum ];

#if DEBUG_DPCR_INTEGRATOR_WORKAROUND
        /* Don't run the workaround. Instead, just print these values: ACCUM_VALUE, PHASE_ERROR, PCR_COUNT, INTR_STATUS_REG */
        BDBG_MSG(( "DPCR%u ACCUM: 0x%08lX, PHASE_ERROR: 0x%08lX, PCR_COUNT 0x%08lX, INTR_STATUS_REG 0x%08lX", PcrNum,
            BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_ACCUM_VALUE + hPcr->RegOffset),
            BCHP_GET_FIELD_DATA( BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_PHASE_ERROR + hPcr->RegOffset), XPT_DPCR0_PHASE_ERROR, PHASE_ERROR),
            BCHP_GET_FIELD_DATA( BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_PCR_COUNT + hPcr->RegOffset), XPT_DPCR0_PCR_COUNT, PCR_COUNT),
            BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_INTR_STATUS_REG + hPcr->RegOffset)
        ));
#else
        if( hPcr->ChangingToNonPcrSource )
        {
            BDBG_MSG(( "Changing to non-PCR source" ));
            hPcr->ChangingToNonPcrSource = false;
            hPcr->PhaseErrorDelay = (long) ((2 * hPcr->PcrInc * 38) / 1000); /* PhaseErrorDelay should be in mS */
        }
        if( hPcr->PhaseErrorDelay )
        {
            /* This is based on the Nexus thread calling once every 2 mS */
            hPcr->PhaseErrorDelay -= 2;
            if( hPcr->PhaseErrorDelay <= 0 )
            {
                BDBG_MSG(( "Finished change to non-PCR source" ));
                hPcr->PhaseErrorDelay = 0;
            }
            return ExitCode;
        }

        /* Note: this number is stored in hw as 2's complement format. See BXPT_PCR_GetPhaseError_isr() */
        Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_PHASE_ERROR + hPcr->RegOffset );
        PcrThreshold = BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_PHASE_ERROR, PHASE_ERROR);
        BDBG_MSG(( "DPCR%u latest PHASE_ERROR " BDBG_UINT64_FMT ", Saved PHASE_ERROR " BDBG_UINT64_FMT "",
            PcrNum, BDBG_UINT64_ARG(PcrThreshold), BDBG_UINT64_ARG(hPcr->PcrThreshold) ));

        Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_PCR_COUNT + hPcr->RegOffset );
        BDBG_MSG(( "DPCR%u latest PCR_COUNT: 0x%08lX", PcrNum, (unsigned long) Reg ));
        PcrCount = BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_PCR_COUNT, PCR_COUNT);

        /* Don't use the current PHASE_ERROR if a PCR ERROR occurred. */
        Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_INTR_STATUS_REG + hPcr->RegOffset );
        BDBG_MSG(( "DPCR%u INTR_STATUS_REG: 0x%08lX", PcrNum, (unsigned long) Reg ));
        PcrErrSeen = BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_INTR_STATUS_REG, ONE_PCR_ERROR )
            || BCHP_GET_FIELD_DATA(Reg, XPT_DPCR0_INTR_STATUS_REG, TWO_PCR_ERROR );

        if( hPcr->InErrState )
        {
            /* Leave the error state if we've seen at least 2 PCRs since we entered */
            if( PcrCount >= (hPcr->PcrCount + 2) % 256)
            {
                BDBG_MSG(( "DPCR%u leaving PCR ERROR state", PcrNum ));
                Reg = BREG_Read32( hPcr->hRegister, BCHP_XPT_DPCR0_INTR_STATUS_REG + hPcr->RegOffset );
                Reg &= ~(
                    BCHP_MASK( XPT_DPCR0_INTR_STATUS_REG, ONE_PCR_ERROR ) |
                    BCHP_MASK( XPT_DPCR0_INTR_STATUS_REG, TWO_PCR_ERROR )
                );
                BREG_Write32( hPcr->hRegister, BCHP_XPT_DPCR0_INTR_STATUS_REG + hPcr->RegOffset, Reg );
                hPcr->InErrState = false;
            }
            else
            {
                PcrThreshold = hPcr->PcrThreshold;  /* Use the last valid PHASE_ERROR */
            }
        }
        else if( PcrErrSeen && !hPcr->InErrState )
        {
            BDBG_MSG(( "DPCR%u entering PCR ERROR state", PcrNum ));
            hPcr->InErrState = true;
            hPcr->PcrCount = PcrCount;
            PcrThreshold = hPcr->PcrThreshold;  /* Use the last valid PHASE_ERROR */
        }

        if( !hPcr->InErrState )
        {
            hPcr->PcrThreshold = PcrThreshold;  /* Save this PHASE_ERROR in case we get a PCR_ERROR before the next execution. */
        }

        if( !hPcr->Accum && !PcrThreshold )
        {
            BDBG_MSG(( "No integrator correction done for DPCR%u", PcrNum ));
            continue; /* Don't bother when both the last Accum and the current Phase Error are 0 */
        }

        hPcr->Accum = CorrectLeakyIntegrator( hPcr, PcrThreshold, hPcr->Accum, hPcr->TrackRange,
            hPcr->FiltB, hPcr->FiltC );
#endif /* DEBUG_DPCR_INTEGRATOR_WORKAROUND */
    }

    return ExitCode;
}

#endif
