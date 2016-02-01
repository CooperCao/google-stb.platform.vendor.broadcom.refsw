/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
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
*   API name: Platform Frontend
*    Platform Frontend Setup
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"

BDBG_MODULE(nexus_platform_frontend);

#if NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND
#include "priv/nexus_core.h"
#include "nexus_i2c.h"
#include "bfpga.h"
#include "bfpga_name.h"
#include "nexus_frontend.h"
#include "nexus_platform_features.h"
#include "nexus_base.h"
#if NEXUS_FRONTEND_3510
#include "nexus_tuner_3420.h"
#include "nexus_frontend_3510.h"
#elif NEXUS_FRONTEND_3520
#include "nexus_tuner_dtt76800.h"
#include "nexus_frontend_3520.h"
#endif
#if NEXUS_BOARD_7405_IFE
#include "nexus_gpio.h"
#include "nexus_frontend_31xx.h"
#endif

#if NEXUS_NUM_FRONTEND_CARD_SLOTS
static NEXUS_FrontendCardHandle g_frontendCards[NEXUS_NUM_FRONTEND_CARD_SLOTS];
#endif

#if NEXUS_BOARD_7405_MSG /* front end for 7405 MSG board */

NEXUS_Error NEXUS_Platform_InitFrontend()
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    int frontend=0;
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    int card=0;
    unsigned numChannels, i;
    NEXUS_FrontendCardSettings cardSettings;
    NEXUS_FrontendUserParameters userParams;
#endif

    if (!pConfig->i2c[0]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    /* Probe Daughercards First */
    #if NEXUS_NUM_FRONTEND_CARD_SLOTS
    /* Find first empty frontend in config */
    for ( ; pConfig->frontend[frontend] && frontend < NEXUS_MAX_FRONTENDS; frontend++ );
    if ( frontend >= NEXUS_MAX_FRONTENDS )
    {
        return BERR_SUCCESS;
    }

    /* Probe first slot */
    NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
    cardSettings.i2cDevice = pConfig->i2c[0];               /* First slot uses I2C 0 */
    cardSettings.isrNumber = 50;                            /* Second slot uses EXT IRQ 13 (L1 #50) */
    cardSettings.numChannels = 2;                           /* First slot has 2 channels */
    BDBG_MSG(("Probing slot 0"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] )
    {
        BDBG_WRN(("Found tuner card in slot 0"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ )
        {

            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e0:NEXUS_InputBand_e1;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
        card++;
    }
    if ( card >= NEXUS_NUM_FRONTEND_CARD_SLOTS || frontend >= NEXUS_MAX_FRONTENDS )
    {
        return BERR_SUCCESS;
    }

    NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
    cardSettings.i2cDevice = pConfig->i2c[2];               /* Second slot uses I2C 2 */
    cardSettings.isrNumber = 48;                            /* First slot uses EXT IRQ 11 (L1 #48) */
    cardSettings.numChannels = 2;                           /* Second slot has 2 channels */
    BDBG_MSG(("Probing slot 1"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] )
    {
        BDBG_WRN(("Found tuner card in slot 1"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ )
        {
            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e2:NEXUS_InputBand_e3;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
        card++;
    }
    if ( card >= NEXUS_NUM_FRONTEND_CARD_SLOTS || frontend >= NEXUS_MAX_FRONTENDS )
    {
        return BERR_SUCCESS;
    }
    NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
    cardSettings.i2cDevice = pConfig->i2c[3];               /* Second slot uses I2C 3 */
    cardSettings.isrNumber = 49;                            /* Second slot uses EXT IRQ 12 (L1 #49) */
    cardSettings.numChannels = 2;                           /* Second slot has 2 channels */
    BDBG_MSG(("Probing slot 2"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] )
    {
        BDBG_WRN(("Found tuner card in slot 2"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ )
        {
            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e4:NEXUS_InputBand_e5;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
        card++;
    }
    #endif

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    #if NEXUS_NUM_FRONTEND_CARD_SLOTS
    int i;
    for ( i = NEXUS_NUM_FRONTEND_CARD_SLOTS-1; i >= 0; i-- )
    {
        if ( g_frontendCards[i] )
        {
            NEXUS_FrontendCard_Close(g_frontendCards[i]);
        }
    }
    #endif
}

#elif NEXUS_BOARD_7405_IFE /* front end for 7405 IFE board */

NEXUS_Error NEXUS_Platform_InitFrontend()
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned frontend=0;
    NEXUS_31xxSettings st31xxSettings;
    NEXUS_GpioHandle pin = NULL;

    NEXUS_Frontend_GetDefault31xxSettings(&st31xxSettings);
    
#if NEXUS_FRONTEND_GPIO_INTERRUPT
    NEXUS_GpioSettings gpioSettings;
    NEXUS_Gpio_GetDefaultSettings(NEXUS_GpioType_eStandard, &gpioSettings);
    pin = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 82, &gpioSettings);
    if ( pin == NULL ) {return BERR_TRACE(NEXUS_NOT_INITIALIZED);}  
#else
    st31xxSettings.isrNumber = 37;                  /* EXT_IRQ_1 for 31xx. */
#endif
    st31xxSettings.gpioInterrupt = pin;
    st31xxSettings.i2cDevice = pConfig->i2c[0];     /* Onboard tuner/demod use BSC 0.*/
    st31xxSettings.i2cAddr = 0x66;
    st31xxSettings.channelNumber = frontend;
    st31xxSettings.type = NEXUS_31xxChannelType_eInBand;
    st31xxSettings.ifFrequency = 44000000;          /* Only applicable for 3117 and 3114 OOB module. */
    st31xxSettings.configureWatchdog = true;
    st31xxSettings.i2cSlaveAddr = 0x60;             /* 31xx's slave LNA device i2c address. */

    pConfig->frontend[frontend] = NEXUS_Frontend_Open31xx(&st31xxSettings);

    if(pConfig->frontend[frontend]) {
        NEXUS_FrontendUserParameters userParams;
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
        userParams.param1 = NEXUS_InputBand_e0;
        userParams.pParam2 = NULL;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        frontend++;
        st31xxSettings.type = NEXUS_31xxChannelType_eOutOfBand;
        pConfig->frontend[frontend] = NEXUS_Frontend_Open31xx(&st31xxSettings);

        if(pConfig->frontend[frontend]) {
            frontend++;
            st31xxSettings.type = NEXUS_31xxChannelType_eUpstream;
            pConfig->frontend[frontend] = NEXUS_Frontend_Open31xx(&st31xxSettings);

            if(pConfig->frontend[frontend]) {
                frontend++;
                BDBG_WRN(("Slot 0 has 3117 frontend."));
            }
            else {
                BDBG_WRN(("Slot 0 has 3114 frontend."));
            }
        }
        else {
            BDBG_WRN(("Slot 0 has only one downstream."));
        }

    }
    else
    {
        BDBG_WRN(("No frontend found in slot 0"));
    }

#if NEXUS_FRONTEND_GPIO_INTERRUPT
    pin = NEXUS_Gpio_Open(NEXUS_GpioType_eStandard, 83, &gpioSettings);
    if ( pin == NULL ) {return BERR_TRACE(NEXUS_NOT_INITIALIZED);}  
#else
    st31xxSettings.isrNumber = 38;                  /* EXT_IRQ_2 for 31xx. */
#endif
    st31xxSettings.gpioInterrupt = pin;
    st31xxSettings.configureWatchdog = false;
    st31xxSettings.i2cAddr = 0x67;
    st31xxSettings.type = NEXUS_31xxChannelType_eInBand;

    pConfig->frontend[frontend] = NEXUS_Frontend_Open31xx(&st31xxSettings);

    if(pConfig->frontend[frontend]) {
        NEXUS_FrontendUserParameters userParams;
        NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
        userParams.param1 = NEXUS_InputBand_e1;
        userParams.pParam2 = NULL;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        BDBG_WRN(("Slot 1 has one downstream."));
    }
    else {
        BDBG_WRN(("No frontend found in slot 1."));
    }

    return 0;
}
void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned frontend=0;

    for(frontend=0; frontend < NEXUS_MAX_FRONTENDS; frontend++) {
        if(pConfig->frontend[frontend])
             NEXUS_Frontend_Close(pConfig->frontend[frontend]);
    }
    return;
}

#else /* front end for 97405 board*/

static NEXUS_FrontendHandle g_onboard3510;
static NEXUS_TunerHandle g_onboard3420;

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    int frontend=0;
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    int card=0;
    unsigned numChannels, i;
    NEXUS_FrontendCardSettings cardSettings;
    NEXUS_FrontendUserParameters userParams;
#endif

    if (!pConfig->i2c[2]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }

    /* Probe Daughercards First */
    #if NEXUS_NUM_FRONTEND_CARD_SLOTS
    /* Find first empty frontend in config */
    for ( ; pConfig->frontend[frontend] && frontend < NEXUS_MAX_FRONTENDS; frontend++ );
    if ( frontend >= NEXUS_MAX_FRONTENDS )
    {
        return BERR_SUCCESS;
    }

    /* Probe first slot */
    NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
    cardSettings.i2cDevice = pConfig->i2c[2];               /* First slot uses I2C 2 */
    cardSettings.isrNumber = 48;                            /* First slot uses EXT IRQ 11 (L1 #48) */
    cardSettings.numChannels = 2;                           /* First slot has 2 channels */
    BDBG_MSG(("Probing slot 0"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] )
    {
        BDBG_WRN(("Found tuner card in slot 0"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ )
        {

            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e1:NEXUS_InputBand_e2;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
        card++;
    }
    if ( card >= NEXUS_NUM_FRONTEND_CARD_SLOTS || frontend >= NEXUS_MAX_FRONTENDS )
    {
        return BERR_SUCCESS;
    }
    cardSettings.i2cDevice = pConfig->i2c[3];               /* Second slot uses I2C 3 */
    cardSettings.isrNumber = 50;                            /* Second slot uses EXT IRQ 13 (L1 #50) */
    cardSettings.numChannels = 2;                           /* Second slot has 2 channels */
    BDBG_MSG(("Probing slot 1"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] )
    {
        BDBG_WRN(("Found tuner card in slot 1"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ )
        {
            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e3:NEXUS_InputBand_e4;
            userParams.pParam2 = NULL;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
    }
    #endif

    #if NEXUS_FRONTEND_3510
    if ( frontend < NEXUS_MAX_FRONTENDS )
    {
        NEXUS_3510Settings st3510Settings;
        NEXUS_3420Settings st3420Settings;

        NEXUS_Tuner_GetDefault3420Settings(&st3420Settings);
        NEXUS_Frontend_GetDefault3510Settings(&st3510Settings);

        st3420Settings.i2cDevice = pConfig->i2c[0];         /* Onboard tuner/demod use BSC 0 */
        st3510Settings.i2cDevice = pConfig->i2c[0];
        #if BCHP_VER >= BCHP_VER_B0
        st3510Settings.isrNumber = 47;                      /* EXT_IRQ_10 (P9 rework or later) */
        #else
        st3510Settings.isrNumber = 42;                      /* EXT_IRQ_5 */
        #endif

        BDBG_MSG(("Opening onboard 3420 tuner"));
        g_onboard3420 = NEXUS_Tuner_Open3420(&st3420Settings);
        if ( NULL == g_onboard3420 )
        {
            BDBG_ERR(("Unable to open onboard 3420 tuner"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }

        st3510Settings.devices.tuner = g_onboard3420;
        st3510Settings.i2cAddr = 0xf;
        BDBG_MSG(("Opening onboard 3510"));
        g_onboard3510 = NEXUS_Frontend_Open3510(&st3510Settings);
        if ( NULL == g_onboard3510 )
        {
            BDBG_ERR(("Unable to open onboard 3510 demodulator"));
            return BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        /* Store in config structure */
        pConfig->frontend[frontend] = g_onboard3510;
    }
    else
    {
        BDBG_WRN(("Frontend Limit Reached.  Onboard 3510/3420 will not be initialized."));
    }
    #endif

    return BERR_SUCCESS;
}

void NEXUS_Platform_UninitFrontend(void)
{
    #if NEXUS_NUM_FRONTEND_CARD_SLOTS
    int i;
    for ( i = NEXUS_NUM_FRONTEND_CARD_SLOTS-1; i >= 0; i-- )
    {
        if ( g_frontendCards[i] )
        {
            NEXUS_FrontendCard_Close(g_frontendCards[i]);
            g_frontendCards[i] = NULL;
        }
    }
    if ( g_onboard3510 )
    {
        NEXUS_Frontend_Close(g_onboard3510);
        g_onboard3510 = NULL;
    }
    if ( g_onboard3420 )
    {
        NEXUS_Tuner_Close(g_onboard3420);
        g_onboard3420 = NULL;
    }
    #endif

}
#endif /* front end for 97405 end */

#else
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return 0;
}
void NEXUS_Platform_UninitFrontend(void)
{
}

#endif /* NEXUS_HAS_I2C && NEXUS_HAS_FRONTEND */

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *pInputBand = NEXUS_InputBand_e5;
    return NEXUS_SUCCESS;
}

NEXUS_FrontendHandle NEXUS_Platform_OpenFrontend(
    unsigned id /* platform assigned ID for this frontend. See NEXUS_FrontendUserParameters.id.
                   See nexus_platform_frontend.c for ID assignment and/or see 
                   nexus_platform_features.h for possible platform-specific macros.
                */
    )
{
    NEXUS_Error errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
    BSTD_UNUSED(errCode);
    BSTD_UNUSED(id);
    return NULL;
}


