/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include "nexus_transport_module.h"
#include "nexus_mpod.h"

BDBG_MODULE(nexus_mpod);

#if BXPT_HAS_MPOD_SCARD_SUPPORT

#include "bxpt_mpod.h"
#include "nexus_playpump_impl.h"

/**
TODO: suggestions on rework:
- require a NEXUS_MpodInput_Open, even more spod. this would allow us to remove the parserBand/playpump from OpenSettings.
**/

struct NEXUS_Mpod
{
    NEXUS_OBJECT(NEXUS_Mpod);
    NEXUS_MpodOpenSettings openSettings;
};

struct NEXUS_MpodInput
{
    NEXUS_OBJECT(NEXUS_MpodInput);
    BXPT_ParserType type;
    unsigned number;
    NEXUS_ParserBandHandle parserBand;
    NEXUS_MpodInputSettings inputSettings;
};

NEXUS_MpodHandle NEXUS_Mpod_Open( unsigned int index, const NEXUS_MpodOpenSettings *pOpenSettings )
{
    BERR_Code rc;
    NEXUS_MpodOpenSettings openSettings;
    NEXUS_MpodHandle mpod;
    BXPT_Mpod_Config config;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_ParserBandHandle parserBand;

    BSTD_UNUSED(index);
    if (!pOpenSettings) {
        NEXUS_Mpod_GetDefaultOpenSettings(&openSettings);
        pOpenSettings = &openSettings;
    }

    mpod = (NEXUS_MpodHandle)BKNI_Malloc(sizeof(*mpod));
    if (!mpod) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Mpod, mpod);
    mpod->openSettings = *pOpenSettings;
    BXPT_Mpod_GetDefaultConfig( pTransport->xpt, &config );
    if (pOpenSettings->bandType == NEXUS_MpodBandType_ePlaypump)
    {
        playpump = pOpenSettings->band.playpump;
        parserBand = NULL;
    }
    else
    {
        playpump = NULL;
        /* returns NULL on platforms with no parser bands */
        parserBand = NEXUS_ParserBand_Resolve_priv(pOpenSettings->band.parserBand);
        if (!parserBand)
        {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error;
        }
    }

    config.SmodeEn = (pOpenSettings->mpodMode==NEXUS_MpodMode_eSpod) ;
    config.BandEn = pOpenSettings->enableBandAssociation;
    config.PbBand = playpump != NULL;
    config.BandNo = playpump ? playpump->index : (parserBand ? parserBand->hwIndex : (unsigned char)-1);
    config.ByteSync = pOpenSettings->byteSync;
    if (pOpenSettings->clockDelay) {
        config.NshiftClk = 1;
        config.ClkDelay = pOpenSettings->clockDelay;
    }
    else {
        config.NshiftClk = 0;
        config.ClkDelay = 0;
    }
    config.InvertClk = pOpenSettings->invertClock;
    config.ClkNrun = pOpenSettings->clockNotRunning;
    config.OutputInvertSync = pOpenSettings->outputInvertSync;
    config.OutputPacketDelayCount = pOpenSettings->outputPacketDelay;

    /* if MPOD mode, set 54MHz clockrate */
#if BXPT_HAS_MPOD_OUTPUT_CLOCK_RATE
    if (pOpenSettings->mpodMode==NEXUS_MpodMode_eMpod) {
        config.OutputClockRate = BXPT_Mpod_OutputClockRate_e54;
        config.OutputClockDivider = BXPT_Mpod_OutputClockDivider_eNone;
    }
#endif
    if ( NEXUS_GetEnv("mpod_loopback") )
    {
        /* Use for debugging loopback */
        config.Loopback = true;
    }

    rc = BXPT_Mpod_Init( pTransport->xpt, &config );
    if(rc != BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto error;
    }

    if (pOpenSettings->enableBandAssociation && playpump==NULL && parserBand) {
        parserBand->mpodBand = true;
    }

    /* Successful return */
    return mpod;

error:
    NEXUS_Mpod_Close(mpod);
    return NULL;
}

static void NEXUS_Mpod_P_Finalizer(NEXUS_MpodHandle mpod)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Mpod, mpod);
    BXPT_Mpod_Shutdown(pTransport->xpt);
    if (mpod->openSettings.band.parserBand) {
        NEXUS_ParserBandHandle handle;
        handle = NEXUS_ParserBand_Resolve_priv(mpod->openSettings.band.parserBand);
        if (handle) {
            handle->mpodBand = false;
            pTransport->overflow.rsbuff.mpodIbp[handle->hwIndex] = 0;
        }
    }
    NEXUS_OBJECT_DESTROY(NEXUS_Mpod, mpod);
    BKNI_Free(mpod);
}

/**
Summary:
Opens an input channel to Mpod
**/
NEXUS_MpodInputHandle NEXUS_MpodInput_Open(
    NEXUS_MpodHandle mpod,
    const NEXUS_MpodInputSettings *pInputSettings
    )
{
    BERR_Code brc ;
    NEXUS_MpodInputSettings inputSettings;
    NEXUS_MpodInputHandle mpodInput ;
    NEXUS_PlaypumpHandle playpump;
    
    BDBG_OBJECT_ASSERT(mpod, NEXUS_Mpod);

    if (!pInputSettings) 
    {
        NEXUS_MpodInput_GetDefaultSettings(&inputSettings);
        pInputSettings = &inputSettings;
    }

    playpump = pInputSettings->bandType == NEXUS_MpodBandType_ePlaypump ? 
        pInputSettings->band.playpump : NULL;

#if !NEXUS_NUM_PARSER_BANDS
    if (!playpump) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
#endif

    mpodInput = (NEXUS_MpodInputHandle)BKNI_Malloc(sizeof(*mpodInput));
    if (NULL == mpodInput)
    {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_MpodInput, mpodInput);

    mpodInput->inputSettings = *pInputSettings ;

    if (playpump)
    {
        mpodInput->type = BXPT_ParserType_ePb;
        mpodInput->number = playpump->index;
    }
    else
    {
        mpodInput->parserBand = NEXUS_ParserBand_Resolve_priv(pInputSettings->band.parserBand);
        if (!mpodInput->parserBand)
        {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            goto error_openinput;
        }
        
        mpodInput->type = BXPT_ParserType_eIb;
        mpodInput->number = mpodInput->parserBand->hwIndex;
        mpodInput->parserBand->refcnt++;
        NEXUS_ParserBand_P_SetEnable(mpodInput->parserBand);
    }

    if (pInputSettings->mpodPreFilter)
    {
        brc = BXPT_Mpod_RouteToMpodPidFiltered(
            pTransport->xpt,
            mpodInput->type,
            mpodInput->number,
            true, /* bool MpodPidFilter */
            false, /* bool ContinuityCountCheck */
            true /* bool Enable */
            );
    }
    else
    {
        brc = BXPT_Mpod_RouteToMpod(
                pTransport->xpt,
                mpodInput->type,
                mpodInput->number,
                true);
    }
    
    if (brc != BERR_SUCCESS)
    {
        BERR_TRACE(brc);
        goto error_openinput;
    }

    /* TODO: is this here rather than grouped above because we want to ensure routing 
    above succeeds before changing these internal xpt settings? */
    if (playpump)
    {
        playpump->settings.allPass = pInputSettings->mpodPreFilter ? false : true;
    }
    else
    {
#if NEXUS_NUM_PARSER_BANDS
        mpodInput->parserBand->settings.allPass = pInputSettings->mpodPreFilter ? false : true;
        mpodInput->parserBand->settings.cableCard = NEXUS_CableCardType_eMCard;
#endif
    }
    
    return mpodInput ;

error_openinput :
    NEXUS_MpodInput_Close(mpodInput);
    return NULL ;
}

static void NEXUS_MpodInput_P_Finalizer( NEXUS_MpodInputHandle mpodInput )
{
    BERR_Code brc;

    BDBG_OBJECT_ASSERT(mpodInput, NEXUS_MpodInput);

    if (mpodInput->inputSettings.mpodPreFilter)
    {
        brc = BXPT_Mpod_RouteToMpodPidFiltered(
            pTransport->xpt,
            mpodInput->type,
            mpodInput->number,
            false, /* bool MpodPidFilter */
            true, /* bool ContinuityCountCheck */
            false /* bool Enable */
            );
    }
    else
    {
        brc = BXPT_Mpod_RouteToMpod(
                pTransport->xpt,
                mpodInput->type,
                mpodInput->number,
                false);
    }
    
    if (brc != BERR_SUCCESS)
    {
        BERR_TRACE(brc);
    }

    if (mpodInput->type == BXPT_ParserType_ePb)
    {
        mpodInput->inputSettings.band.playpump->settings.allPass = false;
    }
#if NEXUS_NUM_PARSER_BANDS
    else if (mpodInput->parserBand) {
        mpodInput->parserBand->settings.allPass = false;
        mpodInput->parserBand->settings.cableCard = NEXUS_CableCardType_eNone;
        mpodInput->parserBand->refcnt--;
        NEXUS_ParserBand_P_SetEnable(mpodInput->parserBand);
    }
#endif

    NEXUS_OBJECT_DESTROY(NEXUS_MpodInput, mpodInput);
    BKNI_Free(mpodInput);
}

void NEXUS_MpodInput_GetSettings(
    NEXUS_MpodInputHandle mpodInput,
    NEXUS_MpodInputSettings *pMpodInputSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(mpodInput, NEXUS_MpodInput);
    *pMpodInputSettings = mpodInput->inputSettings ;
}

NEXUS_Error NEXUS_MpodInput_SetSettings(
    NEXUS_MpodInputHandle mpodInput,
    const NEXUS_MpodInputSettings *pMpodInputSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS ;
    BERR_Code brc ;

    BDBG_OBJECT_ASSERT(mpodInput, NEXUS_MpodInput);
    BDBG_ASSERT(pMpodInputSettings);

    /* All Pass */
    if(mpodInput->inputSettings.allPass != pMpodInputSettings->allPass)
    {
        if (pMpodInputSettings->allPass)
        {
            /* TODO: this looks like it assumes parser band input.
            Shouldn't it check if playpump or not? */
            rc = BXPT_ParserAllPassMode(
                    pTransport->xpt,
                    mpodInput->number,
                    pMpodInputSettings->allPass);
            if (rc != BERR_SUCCESS)
                    return BERR_TRACE(rc);
        }
        brc = BXPT_Mpod_AllPass(
                pTransport->xpt,
                mpodInput->type,
                mpodInput->number,
                pMpodInputSettings->allPass);
        if (brc != BERR_SUCCESS)
            rc = BERR_TRACE(brc);
    }

    mpodInput->inputSettings = *pMpodInputSettings ;

    return rc ;
}

#else

struct NEXUS_Mpod
{
    NEXUS_OBJECT(NEXUS_Mpod);
};

struct NEXUS_MpodInput
{
    NEXUS_OBJECT(NEXUS_MpodInput);
};

NEXUS_MpodHandle NEXUS_Mpod_Open(unsigned int index, const NEXUS_MpodOpenSettings *pOpenSettings)
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pOpenSettings);
    BDBG_WRN(("Mpod not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_Mpod_P_Finalizer(NEXUS_MpodHandle mpod)
{
    BSTD_UNUSED(mpod);
}

NEXUS_MpodInputHandle NEXUS_MpodInput_Open(NEXUS_MpodHandle mpod, const NEXUS_MpodInputSettings *pInputSettings)
{
    BSTD_UNUSED(mpod);
    BSTD_UNUSED(pInputSettings);
    BDBG_WRN(("Mpod not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return NULL;
}

static void NEXUS_MpodInput_P_Finalizer( NEXUS_MpodInputHandle mpodInput )
{
    BSTD_UNUSED(mpodInput);
}

void NEXUS_MpodInput_GetSettings(NEXUS_MpodInputHandle mpodInput, NEXUS_MpodInputSettings *pMpodInputSettings)
{
    BSTD_UNUSED(mpodInput);
    BSTD_UNUSED(pMpodInputSettings);
    BDBG_WRN(("Mpod not enabled on this chipset"));
    BERR_TRACE(BERR_NOT_SUPPORTED);
    return;
}

NEXUS_Error NEXUS_MpodInput_SetSettings(NEXUS_MpodInputHandle mpodInput, const NEXUS_MpodInputSettings *pMpodInputSettings)
{
    BSTD_UNUSED(mpodInput);
    BSTD_UNUSED(pMpodInputSettings);
    BDBG_WRN(("Mpod not enabled on this chipset"));
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif /* BXPT_HAS_MPOD_SCARD_SUPPORT */

void NEXUS_Mpod_GetDefaultOpenSettings(NEXUS_MpodOpenSettings* pOpenSettings)
{
    BKNI_Memset(pOpenSettings, 0, sizeof(NEXUS_MpodOpenSettings));
    pOpenSettings->byteSync = true;
}

void NEXUS_MpodInput_GetDefaultSettings( NEXUS_MpodInputSettings *pMpodDefInputSettings )
{
    BKNI_Memset(pMpodDefInputSettings, 0 , sizeof(*pMpodDefInputSettings));
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Mpod, NEXUS_Mpod_Close);
NEXUS_OBJECT_CLASS_MAKE(NEXUS_MpodInput, NEXUS_MpodInput_Close);
