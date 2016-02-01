/***************************************************************************
*     (c)2004-2014 Broadcom Corporation
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
*   API name: Frontend 73xx
*    APIs to open, close, and setup initial settings for a BCM73xx
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_ast.h"
#include "priv/nexus_i2c_priv.h"
#include "bast.h"
#if BCHP_CHIP==7346 || BCHP_CHIP==7344 || BCHP_CHIP==7358
#include "bast_g3.h"
#else
#include "bast_g2.h"
#endif

BDBG_MODULE(nexus_frontend_73xx);

BDBG_OBJECT_ID(NEXUS_73xxDevice);

typedef struct NEXUS_73xxDevice
{
    BDBG_OBJECT(NEXUS_73xxDevice)
    NEXUS_73xxFrontendSettings settings;
    BAST_Handle astHandle;
    BAST_Mi2cChannel lnaI2cChannel;
    BAST_ChannelHandle astChannels[NEXUS_P_MAX_AST_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_P_MAX_AST_CHANNELS];
    NEXUS_I2cHandle i2cHandles[NEXUS_P_MAX_AST_CHANNELS];
    NEXUS_Frontend73xxTuneSettings tuneSettings[NEXUS_P_MAX_AST_CHANNELS];
    NEXUS_73xxLnaSettings lnaSettings;
    uint32_t numChannels;   /* prototype to match BAST_GetTotalChannels */
    NEXUS_EventCallbackHandle ftmEventCallback;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
} NEXUS_73xxDevice;

static NEXUS_73xxDevice *g_p73xxDevice = NULL;

#if (BCHP_CHIP==7325)
#define NEXUS_FRONTEND_73XX_FAMILY 0x7325
#elif (BCHP_CHIP==7340)
#define NEXUS_FRONTEND_73XX_FAMILY 0x7340
#elif (BCHP_CHIP==7342)
#define NEXUS_FRONTEND_73XX_FAMILY 0x7342
#else
#define NEXUS_FRONTEND_73XX_FAMILY 0x7325
#endif

static void NEXUS_Frontend_P_73xx_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_73xx_DestroyDevice(void *handle);
static BERR_Code NEXUS_Frontend_P_73xx_I2cRead(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_73xx_I2cWrite(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_73xx_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_73xx_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
static void NEXUS_Frontend_P_FtmEventCallback(void *context);

void NEXUS_Frontend_GetDefault73xxSettings( NEXUS_73xxFrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open73xx( const NEXUS_73xxFrontendSettings *pSettings )
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    unsigned i;
    NEXUS_Error errCode;
    NEXUS_73xxDevice *pDevice=NULL;
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendAstSettings astChannelSettings;
    uint8_t sb;

    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p73xxDevice;

    if ( NULL == pDevice )
    {
        BAST_Settings astSettings;
		#if BCHP_CHIP==7344 || BCHP_CHIP==7358
    	BAST_TunerLnaSettings lnaSettings;
		#else
        BAST_Bcm3445Settings lnaSettings;
		#endif

        NEXUS_I2cCallbacks i2cCallbacks;
        BKNI_EventHandle event;

        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL;}
        pFrontendDevice->familyId = NEXUS_FRONTEND_73XX_FAMILY;

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        BDBG_MSG(("Opening new 73xx device"));
        pDevice = BKNI_Malloc(sizeof(NEXUS_73xxDevice));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_73xxDevice);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

#if BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7358
        BAST_g3_GetDefaultSettings(&astSettings);
#else
        BAST_g2_GetDefaultSettings(&astSettings);
#endif
        errCode = BAST_Open(&pDevice->astHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &astSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy;
        }

        /* Determine number of channels -- they will be opened later */
        BAST_GetTotalChannels(pDevice->astHandle, &pDevice->numChannels);

        /* Store LNA I2C channel number once num channels is known */
        pDevice->lnaI2cChannel = BAST_Mi2cChannel_e0 + pSettings->lnaI2cChannelNumber;
        if ( pSettings->lnaI2cChannelNumber > pDevice->numChannels )
        {
            BDBG_ERR(("LNA I2C Channel number invalid"));
            errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            goto destroy;
        }

        NEXUS_I2c_InitCallbacks(&i2cCallbacks);
        i2cCallbacks.read = NEXUS_Frontend_P_73xx_I2cRead;
        i2cCallbacks.write = NEXUS_Frontend_P_73xx_I2cWrite;
        i2cCallbacks.readNoAddr = NEXUS_Frontend_P_73xx_I2cReadNoAddr;
        i2cCallbacks.writeNoAddr = NEXUS_Frontend_P_73xx_I2cWriteNoAddr;

        /* Open all channels prior to InitAp */
        for ( i = 0; i < pDevice->numChannels; i++ )
        {
            BAST_ChannelSettings bastChannelSettings;
         	BAST_GetChannelDefaultSettings(pDevice->astHandle, i, &bastChannelSettings);
            errCode = BAST_OpenChannel(pDevice->astHandle, &pDevice->astChannels[i], i, &bastChannelSettings);
            if ( errCode ) {
                BDBG_ERR(("Unable to open channel %d", i));
                errCode = BERR_TRACE(errCode);
                goto destroy;
            }

            pDevice->i2cHandles[i] = NEXUS_I2c_CreateHandle(NEXUS_MODULE_SELF,
                                                            pDevice->astChannels[i],
                                                            &i2cCallbacks);
            if ( NULL == pDevice->i2cHandles[i] ) {
                errCode = BERR_TRACE(NEXUS_UNKNOWN);
                goto destroy;
            }
        }
#ifdef NEXUS_POWER_MANAGEMENT
        /* the following sequence is required to safely call InitAp from any prior state */
        BAST_PowerDownFtm(pDevice->astHandle);
        for (i=0; i<pDevice->numChannels; i++) {
            BAST_PowerDown(pDevice->astChannels[i], BAST_CORE_ALL);
        }
        BAST_PowerUpFtm(pDevice->astHandle);
        for (i=0; i<pDevice->numChannels; i++) {
            BAST_PowerUp(pDevice->astChannels[i], BAST_CORE_ALL);
        }
#endif

        BDBG_WRN(("Initializating 73xx Frontend core..."));
        /* Init the acquisition processor */
        errCode = BAST_InitAp(pDevice->astHandle, NULL);
        if ( errCode ) {
            errCode = BERR_TRACE(errCode);
            goto destroy;
        }

        errCode = BAST_GetFtmEventHandle(pDevice->astHandle, &event);
        if ( errCode ) {
            errCode = BERR_TRACE(errCode);
            goto destroy;
        }

        pDevice->ftmEventCallback = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_FtmEventCallback, pDevice);
        if ( !pDevice->ftmEventCallback ) {
            errCode = BERR_TRACE(NEXUS_UNKNOWN);
            goto destroy;
        }

#ifdef NEXUS_POWER_MANAGEMENT
        errCode = BAST_PowerDownFtm(pDevice->astHandle);
        if ( errCode ) {
            errCode = BERR_TRACE(errCode);
            goto destroy;
        }
#endif

        /* External LNA note: If you are using another LNA instead of BCM3445, you must set all of these to NEXUS_73xxLnaInput_eNone.
        This code must be bypassed. Instead, program your LNA directly in the frontend extension.
        The AST porting interface has been tightly integrated with the BCM3445 LNA. That integration is not applicable to external LNA's. */
        if ( pSettings->lnaSettings.out1 != NEXUS_73xxLnaInput_eNone ||
             pSettings->lnaSettings.out2 != NEXUS_73xxLnaInput_eNone ||
             pSettings->lnaSettings.daisy != NEXUS_73xxLnaInput_eNone )
        {
            /* this LNA configuration is board-dependent */
#if  (BCHP_CHIP==7344) || (BCHP_CHIP==7358)
		BDBG_ERR(("BAST_ConfigTunerLna()"));
#if BCHP_CHIP==7344
			lnaSettings.out0 = BAST_TunerLnaOutputConfig_eIn0;
			lnaSettings.out1 = BAST_TunerLnaOutputConfig_eOff;
			lnaSettings.daisy = BAST_TunerLnaOutputConfig_eOff;
#else
			lnaSettings.out0 = BAST_TunerLnaOutputConfig_eIn0;
			lnaSettings.out1 = BAST_TunerLnaOutputConfig_eIn1;
			lnaSettings.daisy = BAST_TunerLnaOutputConfig_eIn1;
#endif
			errCode = BAST_ConfigTunerLna(pDevice->astHandle, &lnaSettings);
			if (errCode != BERR_SUCCESS)
			{
				BDBG_ERR(("BAST_ConfigTunerLna() error"));
				goto destroy;
			}
#else
            BDBG_MSG(("initializing BCM3445..."));
            lnaSettings.mi2c = pDevice->lnaI2cChannel;
            BDBG_CASSERT((NEXUS_73xxLnaInput)BAST_Bcm3445OutputConfig_eOff == NEXUS_73xxLnaInput_eNone);
            BDBG_CASSERT((NEXUS_73xxLnaInput)BAST_Bcm3445OutputConfig_eIn2Vga == NEXUS_73xxLnaInput_eIn2Vga);
            lnaSettings.out1 = pSettings->lnaSettings.out1;
            lnaSettings.out2 = pSettings->lnaSettings.out2;
            lnaSettings.daisy = pSettings->lnaSettings.daisy;
            pDevice->lnaSettings = pSettings->lnaSettings;
            errCode = BAST_ConfigBcm3445(pDevice->astHandle, &lnaSettings);
            if ( errCode ) {
				BDBG_ERR(("BAST_ConfigBcm3445() error"));
                goto destroy;
            }
			#endif
        }
	else
	{
#if (BCHP_CHIP==7358) 	
		errCode = BAST_ReadConfig(pDevice->astChannels[0], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
		if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
		
		sb |= BAST_G3_CONFIG_TUNER_CTL_LNA_PGA_MODE;
		errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
		if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }		
#endif
	}

        /* configure fsk channels */
#if BCHP_CHIP==7344 || BCHP_CHIP==7358 /* TBD */
/* Do not modify FSK channel yet */
            sb= 0;
#elif BCHP_CHIP==7346
            sb=3;
            /* sb= BAST_FskChannelConfig_eCh1Tx_Ch1Rx; */

		    errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G3_CONFIG_FTM_CH_SELECT, &sb, BAST_G3_CONFIG_LEN_FTM_CH_SELECT);
#else
            sb = BAST_FskChannelConfig_eCh0Tx_Ch0Rx;

            errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G2_CONFIG_FTM_CH_SELECT, &sb, BAST_G2_CONFIG_LEN_FTM_CH_SELECT);
            if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }

            if (pSettings->lnbPowerUpPinSelect) {
                uint8_t buf[2];

                errCode = BAST_ReadConfig(pDevice->astChannels[0], BAST_G2_CONFIG_DISEQC_CTL, buf, BAST_G2_CONFIG_LEN_DISEQC_CTL);
                if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }

                buf[1] |= BAST_G2_DISEQC_CTL_LNBPU_TXEN;

                errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G2_CONFIG_DISEQC_CTL, buf, BAST_G2_CONFIG_LEN_DISEQC_CTL);
                if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
        }
#endif
        BDBG_WRN(("Initializating 73xx core... Done"));

        /* Success, add to device list */
        g_p73xxDevice = pDevice;
    }
    else
    {
        BDBG_MSG(("Found existing device"));
        pFrontendDevice = pDevice->pGenericDeviceHandle;
    }

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_73xxDevice);

    if ( pSettings->channelNumber >= pDevice->numChannels )
    {
        BDBG_ERR(("Channel %u not supported on this device", pSettings->channelNumber));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
    if ( NULL != pDevice->handles[pSettings->channelNumber] )
    {
        BDBG_ERR(("Channel %u already open", pSettings->channelNumber));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    BDBG_MSG(("Creating channel %u", pSettings->channelNumber));

    if ( pSettings->lnaOutput != NEXUS_73xxLnaOutput_eNone )
    {
        /* External LNA note: lnaOutput should be NEXUS_73xxLnaOutput_eNone. See first "External LNA note" above. */
        /* Map LNA output to this frontend handle */
        errCode = BAST_MapBcm3445ToTuner(pDevice->astChannels[pSettings->channelNumber],
                                         pDevice->lnaI2cChannel, pSettings->lnaOutput);
        if ( errCode != BERR_SUCCESS )
        {
            BDBG_WRN(("Unable to map LNA to frontend"));
            (void)BERR_TRACE(BERR_INVALID_PARAMETER);
            return NULL;
        }
    }

#ifdef NEXUS_POWER_MANAGEMENT
    /*  power down all channels after 3445 config and mapping */
    for (i=0; i<pDevice->numChannels; i++) {
        if (pDevice->handles[i]==NULL) {
            BDBG_MSG(("Power down [%d]=%#lx", i, pDevice->astChannels[i]));
            BAST_PowerDown(pDevice->astChannels[i], BAST_CORE_ALL);
        }
    }

    /* power up the used channel */
    BDBG_MSG(("Power up [%d]=%#lx", pSettings->channelNumber, pDevice->astChannels[pSettings->channelNumber]));
    BAST_PowerUp(pDevice->astChannels[pSettings->channelNumber], BAST_CORE_ALL);
#endif

    /* Open channel */
    NEXUS_Frontend_P_Ast_GetDefaultSettings(&astChannelSettings);
    astChannelSettings.astHandle = pDevice->astHandle;
    astChannelSettings.astChannel = pDevice->astChannels[pSettings->channelNumber];
#define B_AST_CHIP BCHP_CHIP
    astChannelSettings.astChip = B_AST_CHIP;
    astChannelSettings.closeFunction = NEXUS_Frontend_P_73xx_CloseCallback;
    astChannelSettings.pCloseParam = pDevice;
    astChannelSettings.channelIndex = pSettings->channelNumber;

#if BCHP_CHIP !=7344
    /* turn off what I don't support: last channel is qpsk only on 73xx */
    if (pSettings->channelNumber == NEXUS_P_MAX_AST_CHANNELS-1)
    {
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eQpskTurbo] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eTurboQpsk] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_e8pskTurbo] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eTurbo8psk] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eTurbo] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eQpskLdpc] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_e8pskLdpc] = false;
        astChannelSettings.capabilities.satelliteModes[NEXUS_FrontendSatelliteMode_eLdpc] = false;
        astChannelSettings.capabilities.diseqc = false;
    }
#endif
#if BCHP_CHIP == 7340
    if (pSettings->channelNumber != 0) {
        astChannelSettings.capabilities.diseqc = false;
    }
#endif
#if BCHP_CHIP == 7340
    if (pSettings->channelNumber != 0) {
        astChannelSettings.capabilities.diseqc = false;
    }
#endif
    frontend = NEXUS_Frontend_P_Ast_Create(&astChannelSettings);
    if ( !frontend )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_73xx_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }
    pDevice->handles[pSettings->channelNumber] = frontend;

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = NEXUS_FRONTEND_73XX_FAMILY;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->close = NEXUS_Frontend_P_73xx_DestroyDevice;
    frontend->pGenericDeviceHandle = pFrontendDevice;

    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Ast_GetSatelliteCapabilities;
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    return frontend;

destroy:
    NEXUS_Frontend_P_73xx_DestroyDevice(pDevice);
    return NULL;
}


static void NEXUS_Frontend_P_73xx_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_73xxDevice *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_73xxDevice);

    /* Mark handle as destroyed */
    if ( handle ) {
        for ( i = 0; i < pDevice->numChannels; i++ ) {
            if ( handle == pDevice->handles[i] ) {
                pDevice->handles[i] = NULL;
                break;
            }
        }
        BDBG_ASSERT(i < pDevice->numChannels);
    }

    /* See if any channels are still open */
    for ( i = 0; i < pDevice->numChannels; i++ ) {
        if ( pDevice->handles[i] ) {
            return;
        }
    }
}

static void NEXUS_Frontend_P_73xx_DestroyDevice(void *handle)
{
    NEXUS_73xxDevice *pDevice = (NEXUS_73xxDevice *)handle;
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_73xxDevice);

    BDBG_MSG(("Destroying 73xx device %p", pDevice));
    for ( i = 0; i < pDevice->numChannels; i++ ) {
        if (pDevice->i2cHandles[i]) {
            NEXUS_I2c_DestroyHandle(pDevice->i2cHandles[i]);
        }
        if (pDevice->astChannels[i]) {
            BAST_CloseChannel(pDevice->astChannels[i]);
        }
    }
    if (pDevice->ftmEventCallback) {
        NEXUS_UnregisterEvent(pDevice->ftmEventCallback);
    }
    if (pDevice->astHandle) {
        BAST_Close(pDevice->astHandle);
    }
    if (pDevice->pGenericDeviceHandle)
        BKNI_Free(pDevice->pGenericDeviceHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_73xxDevice);
    BKNI_Free(pDevice);
    g_p73xxDevice = NULL;
}

void NEXUS_Frontend_Get73xxLnaSettings( NEXUS_FrontendHandle handle, NEXUS_73xxLnaSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    if ( NULL == g_p73xxDevice )
    {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    else
    {
        *pSettings = g_p73xxDevice->lnaSettings;
    }
}

NEXUS_Error NEXUS_Frontend_Set73xxLnaSettings( NEXUS_FrontendHandle handle, const NEXUS_73xxLnaSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    if ( NULL == g_p73xxDevice )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        NEXUS_Error errCode;
        BAST_Bcm3445Settings lnaSettings;

        /* External LNA note: Your app should not call NEXUS_Frontend_Set73xxLnaSettings. See first "External LNA note" above.
        TODO: consider failing this call if g_p73xxDevice->settings.lnaOutput == NEXUS_73xxLnaOutput_eNone. */
        lnaSettings.mi2c = g_p73xxDevice->lnaI2cChannel;
        BDBG_CASSERT((NEXUS_73xxLnaInput)BAST_Bcm3445OutputConfig_eOff == NEXUS_73xxLnaInput_eNone);
        BDBG_CASSERT((NEXUS_73xxLnaInput)BAST_Bcm3445OutputConfig_eIn2Vga == NEXUS_73xxLnaInput_eIn2Vga);
        lnaSettings.out1 = pSettings->out1;
        lnaSettings.out2 = pSettings->out2;
        lnaSettings.daisy = pSettings->daisy;
        errCode = BAST_ConfigBcm3445(g_p73xxDevice->astHandle, &lnaSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        g_p73xxDevice->lnaSettings = *pSettings;
    }

    return NEXUS_SUCCESS;
}

NEXUS_I2cHandle NEXUS_Frontend_Get73xxMasterI2c( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        return NULL;
    }
    if (!g_p73xxDevice) {
        BDBG_ERR(("Invalid frontend handle"));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    return g_p73xxDevice->i2cHandles[astDevice->settings.channelIndex];
}

static BERR_Code NEXUS_Frontend_P_73xx_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    BAST_ChannelHandle handle = context;
    uint8_t dummy;

    BDBG_ASSERT(NULL != context);

    /* Issue a read with no preceding write */
    return BAST_ReadMi2c(handle, chipAddr<<1, &dummy, 0, pData, length);
}

static BERR_Code NEXUS_Frontend_P_73xx_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    BAST_ChannelHandle handle = context;

    BDBG_ASSERT(NULL != context);

    /* BAST does not have a const pointer for data */
    return BAST_WriteMi2c(handle, chipAddr<<1, (uint8_t *)((unsigned long)pData), length);
}

static BERR_Code NEXUS_Frontend_P_73xx_I2cRead(
    void * i2cHandle, /* I2C Handle */
    uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
    uint32_t subAddr, /* 8 bit sub address */
    uint8_t *pData, /* pointer to memory location to store read data */
    size_t length /* number of bytes to read */
    )
{
    BAST_ChannelHandle handle = i2cHandle;
    uint8_t subAddr8 = (uint8_t)subAddr;

    BDBG_ASSERT(NULL != i2cHandle);

    return BAST_ReadMi2c(handle, chipAddr<<1, &subAddr8, 1, pData, length);
}

static BERR_Code NEXUS_Frontend_P_73xx_I2cWrite(
    void * i2cHandle, /* I2C Handle */
    uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
    uint32_t subAddr, /* 8 bit sub address */
    const uint8_t *pData, /* pointer to data to write */
    size_t length /* number of bytes to write */
    )
{
    uint8_t *pBuf;
    BAST_ChannelHandle handle = i2cHandle;
    BERR_Code errCode;

    BDBG_ASSERT(NULL != i2cHandle);

    if ( length > 0 )
    {
        BDBG_ASSERT(NULL != pData);
    }

    pBuf = BKNI_Malloc(length+1);

    if ( NULL == pBuf )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    pBuf[0] = subAddr;
    if ( length > 0 )
    {
        BKNI_Memcpy(pBuf+1, pData, length);
    }

    errCode = BAST_WriteMi2c(handle, chipAddr<<1, pBuf, length+1);

    BKNI_Free(pBuf);

    return errCode;
}

NEXUS_Error NEXUS_Frontend_Get73xxLnaStatus( NEXUS_FrontendHandle handle, NEXUS_73xxLnaStatus *pStatus )
{
    NEXUS_Error errCode;
    BAST_Bcm3445Status lnaStatus;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    
    /* External LNA note: Your app should not call NEXUS_Frontend_Get73xxLnaStatus. See first "External LNA note" above.
    TODO: consider failing this call if g_p73xxDevice->settings.lnaOutput == NEXUS_73xxLnaOutput_eNone. */
    errCode = BAST_GetBcm3445Status(astDevice->astChannel, &lnaStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->lnaOutput = lnaStatus.tuner_input;
    pStatus->lnaInput = lnaStatus.out_cfg;
    pStatus->status = lnaStatus.status;
    pStatus->version = lnaStatus.version;
    pStatus->agc = lnaStatus.agc;

    return NEXUS_SUCCESS;
}

static void NEXUS_Frontend_P_FtmEventCallback(void *context)
{
    NEXUS_73xxDevice *pDevice = context;
    unsigned i;
    for (i=0;i<NEXUS_P_MAX_AST_CHANNELS;i++) {
        if (pDevice->handles[i]) {
            NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(pDevice->handles[i]);
            if (astDevice) {
                NEXUS_TaskCallback_Fire(astDevice->ftmCallback);
            }
        }
    }
}

void NEXUS_Frontend_Get73xxTuneSettings( NEXUS_FrontendHandle handle, NEXUS_Frontend73xxTuneSettings *pSettings )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
    else {
        *pSettings = g_p73xxDevice->tuneSettings[astDevice->settings.channelIndex];
    }
}

NEXUS_Error NEXUS_Frontend_Set73xxTuneSettings( NEXUS_FrontendHandle handle, const NEXUS_Frontend73xxTuneSettings *pSettings )
{
    NEXUS_Error rc;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    NEXUS_Frontend73xxTuneSettings *pCurrent;
    
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    
    pCurrent = &g_p73xxDevice->tuneSettings[astDevice->settings.channelIndex];

    if (pCurrent->disableFecReacquire != pSettings->disableFecReacquire) {
        unsigned char buf[1];
#if BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7358
        rc = BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TURBO_CTL, buf, BAST_G3_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
#if 0 /* Leave as default*/
        buf[0] &= ~(BAST_G3_TURBO_CTL_DISABLE_FEC_REACQ);
        if (pSettings->disableFecReacquire) {

            buf[0] |= BAST_G3_TURBO_CTL_DISABLE_FEC_REACQ;
        }
#endif
        rc = BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TURBO_CTL, buf, BAST_G3_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
#else
        rc = BAST_ReadConfig(astDevice->astChannel, BAST_G2_CONFIG_TURBO_CTL, buf, BAST_G2_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);

        buf[0] &= ~(BAST_G2_TURBO_CTL_DISABLE_FEC_REACQ);
        if (pSettings->disableFecReacquire) {

            buf[0] |= BAST_G2_TURBO_CTL_DISABLE_FEC_REACQ;
        }

        rc = BAST_WriteConfig(astDevice->astChannel, BAST_G2_CONFIG_TURBO_CTL, buf, BAST_G2_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
#endif
        
    }

    *pCurrent = *pSettings;
    return 0;
}
