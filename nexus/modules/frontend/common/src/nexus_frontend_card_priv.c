/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Frontend Card
*    APIs to detect, open, and close removable frontend cards.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_frontend_module.h"

BDBG_MODULE(nexus_frontend_card);

#if NEXUS_NUM_FRONTEND_CARD_SLOTS
typedef bool (*ProbeFunc)(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings);

#ifdef NEXUS_FRONTEND_2940
static bool Probe_2940(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_2940Settings settings;
    NEXUS_2940ProbeResults results;
    NEXUS_Frontend_GetDefault2940Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    BDBG_MSG(("Probing 2940"));
    errCode = NEXUS_Frontend_Probe2940(&settings, &results);
    if ( BERR_SUCCESS == errCode )
    {
        /* Found a 2940.  Open it. */
        BDBG_MSG(("Found 2940 -- trying to open"));
        pCard->frontends[0] = NEXUS_Frontend_Open2940(&settings);
        if ( pCard->frontends[0] )
        {
            BDBG_MSG(("Valid 2940 detected and opened"));
            pCard->numChannels = 1;
            return true;
        }
    }
    return false;
}
#endif

#ifdef NEXUS_FRONTEND_4501
static bool Probe_4501(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_4501Settings settings;
    NEXUS_4501ProbeResults results;

    NEXUS_Frontend_GetDefault4501Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.bitWideSync = pSettings->bitWideSync;
    BDBG_MSG(("Probing 4501"));
    errCode = NEXUS_Frontend_Probe4501(&settings, &results);
    if ( BERR_SUCCESS == errCode )
    {
        unsigned i;

        /* Found a 4501.  Open it. */
        for ( i = 0; i < 2 && i < pSettings->numChannels; i++ )
        {
            BDBG_MSG(("Found 4501 -- trying to open channel %d", i));
            settings.channelNumber = i;
            pCard->frontends[i] = NEXUS_Frontend_Open4501(&settings);
            if ( pCard->frontends[i] )
            {
                BDBG_MSG(("Valid 2940 detected and opened"));
                pCard->numChannels++;
            }
            else
            {
                /* May be a single-channel card */
                break;
            }
        }

        if ( pCard->frontends[0] )
        {
            uint8_t data = 0x70;
            BDBG_WRN(("Program ISL6423 voltage regulator"));
            NEXUS_I2c_WriteNoAddr(pSettings->i2cDevice, 0x08, &data, 1);
            if ( pCard->numChannels > 1 )
            {
                NEXUS_I2c_WriteNoAddr(pSettings->i2cDevice, 0x09, &data, 1);
            }
            return true;
        }
    }
    return false;
}
#endif

#if (NEXUS_FRONTEND_3128 || NEXUS_FRONTEND_3124 || NEXUS_FRONTEND_3123)
static bool Probe_3128(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_3128Settings settings;
    unsigned i;
    bool found = false;

    NEXUS_Frontend_GetDefault3128Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.gpioInterrupt = pSettings->gpioInterrupt;
    settings.i2cAddr = 0X6c;
    settings.isMtsif = true;

    /* Found a 3128.  Open it. */
    for ( i = 0; i < pSettings->numChannels; i++ )
    {
        BDBG_MSG(("Tying to open channel %d", i));
        settings.channelNumber = i;
        pCard->frontends[i] = NEXUS_Frontend_Open3128(&settings);
        if ( pCard->frontends[i] )
        {
            BDBG_MSG(("Valid 3128 down stream channel detected and opened"));
            pCard->numChannels++;
            found = true;
        }
        else
        {
            /* May be a single-channel card */
            BDBG_WRN(("Unable to open onboard 3128 downstream tuner/demodulator channel %d", i));
            break;
        }
    }
    /* Open frontend handles for out of band channels" */
    for (i = 0;  i < pSettings->numOutOfBandChannels ; i++ )
    {
        BDBG_MSG(("Tying to open channel %d", i));
        settings.channelNumber = i + pSettings->numChannels;/*NEXUS_Frontend_Open3128 treats channel numbers sequentially. */
                                                            /* First Downstream, then Out of band, then Upstream. */
        settings.type = NEXUS_3128ChannelType_eOutOfBand;
        pCard->outOfBandFrontends[i] = NEXUS_Frontend_Open3128(&settings);
        if ( pCard->outOfBandFrontends[i] )
        {
            BDBG_MSG(("Valid 3128 detected and opened"));
            pCard->numOutOfBandChannels++;
            found = true;
        }
        else
        {
            /* May be a single-channel card */
            BDBG_WRN(("Unable to open onboard 3128 out of band tuner/demodulator for channel %d", i));
            break;
        }
    }
    return found;
}
#endif

#ifdef NEXUS_FRONTEND_4506
static bool Probe_4506(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_4506Settings settings;
    NEXUS_4506ProbeResults results;

    NEXUS_Frontend_GetDefault4506Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.bitWideSync = pSettings->bitWideSync;

    for (settings.i2cAddr=0x67;settings.i2cAddr <= 0x69;settings.i2cAddr++)
    {
        errCode = NEXUS_Frontend_Probe4506(&settings, &results);
        if ( BERR_SUCCESS == errCode )
        {
            unsigned i;

            /* Found a 4506.  Open it. */
            for ( i = 0; i < 2 && i < pSettings->numChannels; i++ )
            {
                BDBG_MSG(("Found 4506 -- trying to open channel %d", i));
                settings.channelNumber = i;
                pCard->frontends[i] = NEXUS_Frontend_Open4506(&settings);
                if ( pCard->frontends[i] )
                {
                    BDBG_MSG(("Valid 4506 detected and opened"));
                    pCard->numChannels++;
                }
                else
                {
                    /* May be a single-channel card */
                    break;
                }
            }

            if ( pCard->frontends[0] )
            {
                uint8_t data = 0x70;
                BDBG_WRN(("Program ISL6423 voltage regulator"));
                NEXUS_I2c_WriteNoAddr(pSettings->i2cDevice, 0x08, &data, 1);
                if ( pCard->numChannels > 1 )
                {
                    NEXUS_I2c_WriteNoAddr(pSettings->i2cDevice, 0x09, &data, 1);
                }
                return true;
            }
        }
    }

    return false;
}
#endif

#if defined NEXUS_FRONTEND_3520 && NEXUS_FRONTEND_DTT76800
static bool Probe_93520vsbdc(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_3520Settings settings;
    NEXUS_3520ProbeResults results;
    NEXUS_Error errCode;

    NEXUS_Frontend_GetDefault3520Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.i2cAddr = 0xc;
    BDBG_MSG(("Probing 3520"));
    errCode = NEXUS_Frontend_Probe3520(&settings, &results);
    if ( BERR_SUCCESS == errCode )
    {
        /* Found a 3520.  Open it. */
        BDBG_MSG(("Found 3520 -- trying to open"));
        pCard->frontends[0] = NEXUS_Frontend_Open3520(&settings);
        if ( pCard->frontends[0] )
        {
            NEXUS_3520Devices devices;
            NEXUS_Dtt76800Settings dttSettings;

            BDBG_MSG(("Valid 3520 detected and opened...  adding tuner"));

            NEXUS_Tuner_GetDefaultDtt76800Settings(&dttSettings);
            dttSettings.i2cDevice = NEXUS_Frontend_Get3520MasterI2c(pCard->frontends[0]);
            dttSettings.i2cAddr = 0x60;
            NEXUS_Frontend_Get3520Devices(pCard->frontends[0], &devices);
            /* Open tuner */
            devices.tuner = NEXUS_Tuner_OpenDtt76800(&dttSettings);
            if ( NULL == devices.tuner )
            {
                BDBG_ERR(("Unable to open tuner for 93520vsbdc daughtercard"));
                NEXUS_Frontend_Close(pCard->frontends[0]);
                pCard->frontends[0] = NULL;
            }
            else
            {
                /* Success */
                NEXUS_Frontend_Set3520Devices(pCard->frontends[0], &devices);
                pCard->numChannels = 1;
                return true;
            }
        }
    }
    return false;
}
#endif

#ifdef NEXUS_FRONTEND_4500
static const uint8_t address_4500[] = { 
    /* Probe all valid 4500 I2C addresses */
    /* 0x10, skip onboard */0x11,0x12,0x13,0x66,0x6E,0x76,0x7E };
static bool Probe_4500(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_4500Settings settings;
    NEXUS_4500ProbeResults results;
    unsigned i;

    NEXUS_Frontend_GetDefault4500Settings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.bitWideSync = pSettings->bitWideSync;
    BDBG_MSG(("Probing 4500"));
    for (i=0;i<sizeof(address_4500)/sizeof(address_4500[0]);i++)
    {
        settings.i2cAddr = address_4500[i];
        errCode = NEXUS_Frontend_Probe4500(&settings, &results);
        if ( BERR_SUCCESS == errCode )
        {
            /* Found a 4500.  Open it. */
            BDBG_MSG(("Found 4500 -- trying to open it"));
            pCard->frontends[0] = NEXUS_Frontend_Open4500(&settings);
            if ( pCard->frontends[0] )
            {
                BDBG_MSG(("Valid 4500 detected and opened"));
                pCard->numChannels++;
            }
        }
    }
    if ( pCard->frontends[0] )
    {
        return true;
    }
    return false;
}
#endif

#ifdef NEXUS_FRONTEND_3461
static bool Probe_3461(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    NEXUS_Error errCode;
    NEXUS_FrontendDevice3461OpenSettings settings;
    NEXUS_3461ProbeResults results;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_3461Settings st3461Settings;

    NEXUS_FrontendDevice_GetDefault3461OpenSettings(&settings);
    settings.i2cDevice = pSettings->i2cDevice;
    settings.isrNumber = pSettings->isrNumber;
    settings.gpioInterrupt = pSettings->gpioInterrupt;
    settings.i2cAddr = 0X6c;

    BDBG_MSG(("Probing 3461"));
    errCode = NEXUS_Frontend_Probe3461(&settings, &results);
    if(errCode){errCode = BERR_TRACE(errCode); goto error;}

    deviceHandle = NEXUS_FrontendDevice_Open3461(0, &settings);
    if(deviceHandle == NULL){errCode = BERR_TRACE(errCode); goto error;}

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = deviceHandle;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                    /*REDUNDANT for now. */

    /* Found a 3461.  Open it. */
    BDBG_MSG(("Found 3461 -- trying to open"));
    pCard->frontends[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if(pCard->frontends[0] == NULL){errCode = BERR_TRACE(errCode); goto error;}

    BDBG_MSG(("Valid 3461 detected and opened"));
    pCard->numChannels = 1;
    
    return true;
    
error:  
    return false;
}
#endif

static ProbeFunc g_probeFuncs[] =
{
#ifdef NEXUS_FRONTEND_2940
    Probe_2940,
#endif
#ifdef NEXUS_FRONTEND_4500
    Probe_4500,
#endif
#ifdef NEXUS_FRONTEND_4501
    Probe_4501,
#endif
#ifdef NEXUS_FRONTEND_4506
    Probe_4506,
#endif
#if defined NEXUS_FRONTEND_3520 && NEXUS_FRONTEND_DTT76800
    Probe_93520vsbdc,
#endif
#if (NEXUS_FRONTEND_3128 || NEXUS_FRONTEND_3124 || NEXUS_FRONTEND_3123)
    Probe_3128,
#endif
#if defined NEXUS_FRONTEND_3461
    Probe_3461,
#endif /* NEXUS_FRONTEND_3461 */
    NULL    /* Must be last */
};

bool NEXUS_Frontend_P_ProbeCard(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    int i;
    bool found;

    BDBG_ASSERT(NULL != pCard);
    BDBG_ASSERT(NULL != pSettings);

    for ( i=0,found=false; g_probeFuncs[i] != NULL && !found; i++ )
    {
        found = g_probeFuncs[i](pCard, pSettings);
    }

    return found;
}

#else   /* No card slot support */

bool NEXUS_Frontend_P_ProbeCard(NEXUS_FrontendCard *pCard, const NEXUS_FrontendCardSettings *pSettings)
{
    BSTD_UNUSED(pCard);
    BSTD_UNUSED(pSettings);
    return false;
}

#endif
