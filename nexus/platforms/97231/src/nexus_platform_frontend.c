 /***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2004-2016 Broadcom. All rights reserved.
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
***************************************************************************/
#include "nexus_platform_module.h"
#if NEXUS_PLATFORM == 97231
#include "priv/nexus_core.h"
#include "nexus_frontend.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"
#if NEXUS_PLATFORM_7231_3520
#include "nexus_frontend_3520.h"
#elif NEXUS_PLATFORM_7231_FBTSFF || NEXUS_PLATFORM_7231_DCSFBTSFF
#include "nexus_frontend_3128.h"
#elif NEXUS_PLATFORM_7231_CSFF
#include "nexus_frontend_31xx.h"
#else /* NEXUS_PLATFORM_7231_EUSFF && CVBS boards */
#include "nexus_frontend_3461.h"
#endif

BDBG_MODULE(nexus_platform_frontend);

NEXUS_GpioHandle tunerGpio[NEXUS_MAX_FRONTENDS];

#if NEXUS_PLATFORM_7231_3520
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_3520Settings settings3520;
    NEXUS_FrontendUserParameters userParams;

    /* Open on-board 3517A */
    NEXUS_Frontend_GetDefault3520Settings(&settings3520);
    settings3520.i2cDevice = pConfig->i2c[2];
    if (!settings3520.i2cDevice) {
        BDBG_ERR(("I2C[2]/BSC_M2 channel unavailable for 3520"));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
    }
    settings3520.i2cAddr = 0x0c;
    settings3520.isrNumber = 10;

    pConfig->frontend[0] = NEXUS_Frontend_Open3520(&settings3520);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open onboard 3520 demodulator "));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

    /* Open on-board 3517B */
    settings3520.i2cAddr = 0x0d;
    settings3520.isrNumber = 11;

    pConfig->frontend[1] = NEXUS_Frontend_Open3520(&settings3520);
    if (NULL == pConfig->frontend[1])
    {
        BDBG_ERR(("Unable to open onboard 3520 demodulator "));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED); goto done;        
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[1], &userParams);
    userParams.param1 = NEXUS_InputBand_e1;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[1], &userParams);
done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i;

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (pConfig->frontend[i]) {
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }

    return;
}

#elif NEXUS_PLATFORM_7231_EUSFF || NEXUS_PLATFORM_7231_EUSFF_V20
static NEXUS_SpiHandle g_3461spi;
NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_3461Settings st3461Settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendDeviceHandle parentDevice;
    NEXUS_FrontendDevice3461OpenSettings deviceOpenSettings;
    NEXUS_FrontendDeviceLinkSettings linkSettings;
    NEXUS_FrontendDevice3461Settings deviceSettings;

    BDBG_WRN(("Waiting for 3461 Downstream frontend(7231_EUSFF) to initialize"));

#if NEXUS_PLATFORM_97231_3461_USES_SPI
    {
        NEXUS_SpiSettings spiSettings;
        NEXUS_Spi_GetDefaultSettings(&spiSettings);
        spiSettings.clockActiveLow = true;
        g_3461spi = NEXUS_Spi_Open(0, &spiSettings);
        if (!g_3461spi) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
    }
    deviceOpenSettings.spiDevice = g_3461spi;
    deviceOpenSettings.spiAddr = 0x40;
#else
    if (!pConfig->i2c[1]) {
            BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
            return BERR_NOT_INITIALIZED;
    }
    deviceOpenSettings.i2cDevice = pConfig->i2c[1];    /* Onboard tuner/demod use BSC_M1.*/
    deviceOpenSettings.i2cAddr = 0x6c;
#endif
#if NEXUS_PLATFORM_7231_EUSFF
    deviceOpenSettings.isrNumber = 10;
#elif NEXUS_PLATFORM_7231_EUSFF_V20
    deviceOpenSettings.isrNumber = 8;
#endif
    deviceOpenSettings.loadAP = true;
    deviceOpenSettings.externalFixedGain.total = 700;       /* These are platform specific values given by the board designer. */
    deviceOpenSettings.externalFixedGain.bypassable = 1400; /* These are platform specific values given by the board designer. */
    deviceOpenSettings.crystalSettings.enableDaisyChain = true;

    parentDevice = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == parentDevice)
    {
        BDBG_ERR(("Unable to open first 3461 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }    
    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eInternalLna;
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
    deviceSettings.enableRfLoopThrough = true;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(parentDevice, &deviceSettings);

    NEXUS_Frontend_GetDefault3461Settings(&st3461Settings);
    st3461Settings.device = parentDevice;
    st3461Settings.type = NEXUS_3461ChannelType_eDvbt; /*REDUNDANT for now as there is only one instance of any demod running. */
    st3461Settings.channelNumber = 0;                    /*REDUNDANT for now. */

    pConfig->frontend[0] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open first 3461 dvbt2 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

#if NEXUS_PLATFORM_97231_3461_USES_SPI
#else
    BDBG_WRN(("Waiting for the second 3461 Downstream frontend(7231_EUSFF) to initialize"));
    st3461Settings.device = NULL;

#if NEXUS_PLATFORM_7231_EUSFF_V20
    deviceOpenSettings.isrNumber = 9;
#endif  
    deviceOpenSettings.i2cAddr = 0x6d;

    st3461Settings.device = NEXUS_FrontendDevice_Open3461(0, &deviceOpenSettings);
    if (NULL == st3461Settings.device)
    {
       BDBG_ERR(("Unable to open second 3461 tuner/demodulator device"));
       rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_FrontendDevice_GetDefault3461Settings(&deviceSettings);
    deviceSettings.rfDaisyChain = NEXUS_3461RfDaisyChain_eOff;
#if NEXUS_PLATFORM_7231_EUSFF_V20
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eInternalLna;
#else
    deviceSettings.rfInput = NEXUS_3461TunerRfInput_eExternalLna;
#endif
    deviceSettings.enableRfLoopThrough = false;
    deviceSettings.terrestrial = true;
    NEXUS_FrontendDevice_Set3461Settings(st3461Settings.device, &deviceSettings);

    st3461Settings.type = NEXUS_3461ChannelType_eInBand;
    pConfig->frontend[1] = NEXUS_Frontend_Open3461(&st3461Settings);
    if (NULL == pConfig->frontend[1])
    {
        BDBG_ERR(("Unable to open second 3461 dvbt2 tuner/demodulator channel."));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[1], &userParams);
    userParams.param1 = NEXUS_InputBand_e1;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[1], &userParams);

    NEXUS_FrontendDevice_GetDefaultLinkSettings(&linkSettings);

    rc = NEXUS_FrontendDevice_Link(parentDevice, st3461Settings.device, &linkSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
#endif
done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }
            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }
    
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

    if (g_3461spi) {
        NEXUS_Spi_Close(g_3461spi);
        g_3461spi = NULL;
    }

    return;
}

#elif NEXUS_PLATFORM_7231_CSFF

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    NEXUS_31xxProbeResults results;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_Frontend31xxSettings st31xxSettings;
    NEXUS_FrontendDevice31xxOpenSettings st31xxDeviceOpenSettings;
    NEXUS_FrontendDevice31xxSettings st31xxDeviceSettings;

    NEXUS_Frontend_GetDefault31xxSettings(&st31xxSettings);

    NEXUS_FrontendDevice_GetDefault31xxOpenSettings(&st31xxDeviceOpenSettings);
    st31xxDeviceOpenSettings.isrNumber = 9;
    st31xxDeviceOpenSettings.i2cDevice = pConfig->i2c[1];    /* Onboard tuner/demod use BSC_M1.*/    
    st31xxDeviceOpenSettings.i2cAddr = 0x66;
    st31xxDeviceOpenSettings.outOfBand.ifFrequency = 0;     
    st31xxDeviceOpenSettings.inBandOpenDrain=true;
    st31xxDeviceOpenSettings.loadAP = true;
    st31xxDeviceOpenSettings.configureWatchdog = false;

    st31xxSettings.device = NEXUS_FrontendDevice_Open31xx(0, &st31xxDeviceOpenSettings);
    if (NULL == st31xxSettings.device)
    {
        BDBG_ERR(("Unable to open onboard 31xx tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_Probe31xx(&st31xxDeviceOpenSettings, &results);
    BDBG_MSG(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.minor = 0x%x", results.chip.version.minor ));


    BDBG_WRN(("Waiting for frontend(7231_CSFF) 0 to initialize"));

    st31xxSettings.type = NEXUS_31xxChannelType_eInBand;
    st31xxSettings.channelNumber = 0;

    pConfig->frontend[0] = NEXUS_Frontend_Open31xx(&st31xxSettings);    
    if (NULL == pConfig->frontend[0])
    {
        BDBG_ERR(("Unable to open onboard 3109 tuner/demodulator 0"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[0], &userParams);
    userParams.param1 = NEXUS_InputBand_e0;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[0], &userParams);

    NEXUS_FrontendDevice_GetDefault31xxSettings(&st31xxDeviceSettings);

    st31xxDeviceSettings.enableDaisyChain = true;

    rc = NEXUS_FrontendDevice_Set31xxSettings(st31xxSettings.device, &st31xxDeviceSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}

    st31xxDeviceOpenSettings.isrNumber = 10;
    st31xxDeviceOpenSettings.i2cAddr = 0x67;

    st31xxSettings.device = NEXUS_FrontendDevice_Open31xx(0, &st31xxDeviceOpenSettings);
    if (NULL == st31xxSettings.device)
    {
        BDBG_ERR(("Unable to open onboard 31xx tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_Probe31xx(&st31xxDeviceOpenSettings, &results);
    BDBG_MSG(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.minor = 0x%x", results.chip.version.minor ));


    BDBG_WRN(("Waiting for frontend(7231_CSFF) 1 to initialize"));

    st31xxSettings.type = NEXUS_31xxChannelType_eInBand;
    st31xxSettings.channelNumber = 0;

    pConfig->frontend[1] = NEXUS_Frontend_Open31xx(&st31xxSettings);    
    if (NULL == pConfig->frontend[1])
    {
        BDBG_ERR(("Unable to open onboard 3109 tuner/demodulator 1"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_GetUserParameters(pConfig->frontend[1], &userParams);
    userParams.param1 = NEXUS_InputBand_e1;
    userParams.pParam2 = 0;
    NEXUS_Frontend_SetUserParameters(pConfig->frontend[1], &userParams);

done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
   NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;
    unsigned frontendCnt = NEXUS_MAX_FRONTENDS;

    if (NEXUS_GetEnv("disable_oob_frontend") && !BKNI_Memcmp(NEXUS_GetEnv("disable_oob_frontend"), "y", 1))
        frontendCnt = NEXUS_MAX_FRONTENDS-1;
    else
        frontendCnt = NEXUS_MAX_FRONTENDS;
    
    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<frontendCnt; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i])
        {
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            if(tempHandle != NULL){
                for( j = 0; j<i; j++){
                    if(tempHandle == deviceHandles[j])
                    handleFound = true;
                }
                if(!handleFound)
                    deviceHandles[j] = tempHandle;
            }

            NEXUS_Frontend_Close(pConfig->frontend[i]);
            pConfig->frontend[i] = NULL;
        }
    }
    
    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        if (deviceHandles[i])
        {
            NEXUS_FrontendDevice_Close(deviceHandles[i]);
            deviceHandles[i] = NULL;
        }
    }

    return;
}

#elif NEXUS_PLATFORM_7231_FBTSFF || NEXUS_PLATFORM_7231_DCSFBTSFF

NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i;
    NEXUS_Frontend3128Settings st3128Settings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_3128ProbeResults results;
    NEXUS_FrontendDevice3128OpenSettings st3128DeviceOpenSettings;


    NEXUS_Frontend_GetDefault3128Settings(&st3128Settings);

    NEXUS_FrontendDevice_GetDefault3128OpenSettings(&st3128DeviceOpenSettings);
    st3128DeviceOpenSettings.isrNumber = 9;    
    st3128DeviceOpenSettings.i2cDevice = pConfig->i2c[1];    /* Onboard tuner/demod use BSC_M1.*/    
    st3128DeviceOpenSettings.i2cAddr = 0x6c ;
    st3128DeviceOpenSettings.outOfBand.ifFrequency = 0;     
    st3128DeviceOpenSettings.inBandOpenDrain=true;
    st3128DeviceOpenSettings.loadAP = true;
    st3128DeviceOpenSettings.configureWatchdog = false;
#if (BCHP_VER >= BCHP_VER_B0)   
    st3128DeviceOpenSettings.isMtsif = true;
#else
    st3128DeviceOpenSettings.isMtsif = false;
#endif
    st3128Settings.device = NEXUS_FrontendDevice_Open3128(0, &st3128DeviceOpenSettings);
    if (NULL == st3128Settings.device)
    {
        BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator device"));
        rc = BERR_TRACE(BERR_NOT_INITIALIZED); goto done;
    }

    NEXUS_Frontend_Probe3128(&st3128DeviceOpenSettings, &results);
    BDBG_MSG(("chip.familyId = 0x%x", results.chip.familyId));
    BDBG_MSG(("chip.id = 0x%x", results.chip.id));
    BDBG_MSG(("version.major = 0x%x", results.chip.version.major ));
    BDBG_MSG(("version.minor = 0x%x", results.chip.version.minor ));

    st3128Settings.type = NEXUS_3128ChannelType_eInBand;
    /* Open downstream tuners */
    for (i=0; i < (results.chip.id & 0xF); i++)
    {
        BDBG_WRN(("Waiting for frontend(7231_FBTSFF) %d to initialize", i));
        
        st3128Settings.channelNumber = i;

        pConfig->frontend[i] = NEXUS_Frontend_Open3128(&st3128Settings);
        if (NULL == pConfig->frontend[i])
        {
            BDBG_ERR(("Unable to open onboard 3128 tuner/demodulator %d", i));
            continue;
        }

        NEXUS_Frontend_GetUserParameters(pConfig->frontend[i], &userParams);
        userParams.isMtsif = st3128DeviceOpenSettings.isMtsif;
        userParams.param1 = st3128DeviceOpenSettings.isMtsif ? st3128Settings.channelNumber : NEXUS_InputBand_e0 + i;
        userParams.pParam2 = 0;
        NEXUS_Frontend_SetUserParameters(pConfig->frontend[i], &userParams);
    }
done:
    return rc;
}

void NEXUS_Platform_UninitFrontend(void)
{
    NEXUS_PlatformConfiguration *pConfig = &g_NEXUS_platformHandles.config;
    unsigned i=0, j=0;
    NEXUS_FrontendDeviceHandle tempHandle, deviceHandles[NEXUS_MAX_FRONTENDS];
    bool handleFound = false;

    BKNI_Memset(deviceHandles, 0, sizeof(deviceHandles));

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
    {
        handleFound = false;
        if (pConfig->frontend[i]){          
            tempHandle = NEXUS_Frontend_GetDevice(pConfig->frontend[i]);
            for( j = 0; j<i; j++){
                if(tempHandle == deviceHandles[j])
                handleFound = true;
            }
            if(!handleFound)
                deviceHandles[j] = tempHandle;
             NEXUS_Frontend_Close(pConfig->frontend[i]);    
             pConfig->frontend[i] = NULL;
        }
    }

    for (i=0; i<NEXUS_MAX_FRONTENDS; i++)
        {
            if (deviceHandles[i])
            {
                NEXUS_FrontendDevice_Close(deviceHandles[i]);
                deviceHandles[i] = NULL;
            }
        }

    return;
}

#else /* SILICON VERIFICATION BOARD */

#if NEXUS_NUM_FRONTEND_CARD_SLOTS
static NEXUS_FrontendCardHandle g_frontendCards[NEXUS_NUM_FRONTEND_CARD_SLOTS];
#endif

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
    BDBG_WRN(("Waiting for frontend(7231_CBSV) to initialize"));

    if (!pConfig->i2c[1]) {
        BDBG_ERR(("Frontend cannot be initialized without first initializing I2C."));
        return BERR_NOT_INITIALIZED;
    }
    /* Probe Daughercards First */
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    /* Find first empty frontend in config */
    for (frontend=0 ; (frontend < NEXUS_MAX_FRONTENDS) && pConfig->frontend[frontend]; frontend++ );
    if ( frontend >= NEXUS_MAX_FRONTENDS ) {
        BDBG_ERR(("No front ends a available"));
        return BERR_SUCCESS;
    }

    /* Probe first slot */
    NEXUS_FrontendCard_GetDefaultSettings(&cardSettings);
    cardSettings.i2cDevice = pConfig->i2c[1];               /* First slot uses I2C 1 */
    cardSettings.isrNumber = 10;                            /* Second slot uses EXT IRQ 13 (L1 #50) */
    cardSettings.numChannels = 2;                           /* First slot has 2 channels */
    /* cardSettings does not provide fields like i2cAddr or i2cAddr.
     * Those are obtained while probing e.g. from NEXUS_Frontend_GetDefault3461Settings().
     */

    BDBG_WRN(("Probing slot 0"));
    g_frontendCards[card] = NEXUS_FrontendCard_Open(&cardSettings);
    if ( g_frontendCards[card] ) {
        BDBG_WRN(("Found tuner card in slot 0"));
        NEXUS_FrontendCard_GetNumChannels(g_frontendCards[card], &numChannels);
        for ( i=0; i < numChannels && frontend < NEXUS_MAX_FRONTENDS; frontend++, i++ ) {
            pConfig->frontend[frontend] = NEXUS_FrontendCard_GetChannel(g_frontendCards[card], i);
            NEXUS_Frontend_GetUserParameters(pConfig->frontend[frontend], &userParams);
            userParams.param1 = (i==0)?NEXUS_InputBand_e0:NEXUS_InputBand_e1;
            userParams.pParam2 = 0;
            NEXUS_Frontend_SetUserParameters(pConfig->frontend[frontend], &userParams);
        }
        card++;
    }
    if ( card >= NEXUS_NUM_FRONTEND_CARD_SLOTS || frontend >= NEXUS_MAX_FRONTENDS ) {
        return BERR_SUCCESS;
    }
    /* MF implement the other slots (2,3) */
    /* MF implement fall back to the on board FE? (3128) */
#endif /* NEXUS_NUM_FRONTEND_CARD_SLOTS */
    return BERR_SUCCESS;
}
    
void NEXUS_Platform_UninitFrontend(void)
{
#if NEXUS_NUM_FRONTEND_CARD_SLOTS
    int i;
    for ( i = NEXUS_NUM_FRONTEND_CARD_SLOTS-1; i >= 0; i-- ) {
        if ( g_frontendCards[i] ) {
            NEXUS_FrontendCard_Close(g_frontendCards[i]);
        }
    }
#endif
    return;
}

#endif

BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    *pInputBand = NEXUS_InputBand_e5;
    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
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

#elif NEXUS_PLATFORM == 97230

#include "nexus_types.h"
#include "nexus_platform.h"
#include "priv/nexus_core.h"
#include "priv/nexus_frontend_standby_priv.h"
#include "nexus_frontend.h"
#include "nexus_platform_features.h"
#include "nexus_platform_priv.h"
#include "nexus_base.h"
#include "nexus_input_band.h"
#include "bchp_gio.h"

BDBG_MODULE(nexus_platform_frontend);


NEXUS_Error NEXUS_Platform_InitFrontend(void)
{
    return 0;
}

void NEXUS_Platform_UninitFrontend(void)
{
}


BTRC_MODULE(ChnChange_TuneStreamer, ENABLE);

NEXUS_Error
NEXUS_Platform_GetStreamerInputBand(unsigned index, NEXUS_InputBand *pInputBand)
{
    BDBG_ASSERT(pInputBand);
    if (index > 0) {
        BDBG_ERR(("Only 1 streamer input available"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BTRC_TRACE(ChnChange_TuneStreamer, START);
    *pInputBand = NEXUS_InputBand_e5;
    BTRC_TRACE(ChnChange_TuneStreamer, STOP);
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
#endif


