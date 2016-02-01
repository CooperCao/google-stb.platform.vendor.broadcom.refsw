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
*   API name: Frontend 7346
*    APIs to open, close, and setup initial settings for a BCM7346
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
#include "bast_g3.h"

BDBG_MODULE(nexus_frontend_7346);

BDBG_OBJECT_ID(NEXUS_7346Device);

typedef struct NEXUS_7346Device
{
    BDBG_OBJECT(NEXUS_7346Device)
    NEXUS_7346FrontendSettings settings;
    BAST_Handle astHandle;
    NEXUS_FrontendDevice *pGenericDeviceHandle;
    BAST_Mi2cChannel lnaI2cChannel;
    BAST_ChannelHandle astChannels[NEXUS_7346_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_7346_MAX_FRONTEND_CHANNELS];
    NEXUS_I2cHandle i2cHandles[NEXUS_7346_MAX_FRONTEND_CHANNELS];
    NEXUS_Frontend7346TuneSettings tuneSettings[NEXUS_7346_MAX_FRONTEND_CHANNELS];
    NEXUS_3445LnaSettings external3445LnaSettings;
    NEXUS_7346LnaSettings internalLnaSettings;
    uint32_t numChannels;   /* prototype to match BAST_GetTotalChannels */
    NEXUS_EventCallbackHandle ftmEventCallback;
} NEXUS_7346Device;

static NEXUS_7346Device *g_p7346Device = NULL;

static void NEXUS_Frontend_P_7346_RestoreLnaCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_7346_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_7346_DestroyDevice(void *handle);
static BERR_Code NEXUS_Frontend_P_7346_I2cRead(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7346_I2cWrite(void * i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7346_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length);
static BERR_Code NEXUS_Frontend_P_7346_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length);
static void NEXUS_Frontend_P_FtmEventCallback(void *context);
static NEXUS_Error NEXUS_Frontend_P_7346_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus);

void NEXUS_Frontend_GetDefault7346Settings( NEXUS_7346FrontendSettings *pSettings )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

NEXUS_FrontendHandle NEXUS_Frontend_Open7346( const NEXUS_7346FrontendSettings *pSettings )
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    unsigned i;
    NEXUS_Error errCode;
    NEXUS_7346Device *pDevice=NULL;
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendAstSettings astChannelSettings;
  

    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p7346Device;

    if ( NULL == pDevice )
    {
        BAST_Settings astSettings;
        BAST_TunerLnaSettings internalLnaSettings;
        BAST_Bcm3445Settings external3445LnaSettings;

        NEXUS_I2cCallbacks i2cCallbacks;
        BKNI_EventHandle event;

        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL;}
        pFrontendDevice->familyId = 0x7346;

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        BDBG_MSG(("Opening new 7346 device"));
        pDevice = BKNI_Malloc(sizeof(NEXUS_7346Device));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_7346Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

        BAST_g3_GetDefaultSettings(&astSettings);

        errCode = BAST_Open(&pDevice->astHandle, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &astSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy;
        }

        /* Determine number of channels -- they will be opened later */
        BAST_GetTotalChannels(pDevice->astHandle, &pDevice->numChannels);

        NEXUS_I2c_InitCallbacks(&i2cCallbacks);
        i2cCallbacks.read = NEXUS_Frontend_P_7346_I2cRead;
        i2cCallbacks.write = NEXUS_Frontend_P_7346_I2cWrite;
        i2cCallbacks.readNoAddr = NEXUS_Frontend_P_7346_I2cReadNoAddr;
        i2cCallbacks.writeNoAddr = NEXUS_Frontend_P_7346_I2cWriteNoAddr;

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

        BDBG_WRN(("Initializing 7346 Frontend core..."));
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

     /* External LNA note: If you are using another LNA instead of BCM3445, you must set all of these to NEXUS_7346LnaInput_eNone.
             This code must be bypassed. Instead, program your LNA directly in the frontend extension.
             The AST porting interface has been tightly integrated with the BCM3445 LNA. That integration is not applicable to external LNA's. */

        if (pSettings->usePga
#if (BCHP_CHIP==7358) || (BCHP_CHIP==7360) || (BCHP_CHIP==7362) || (BCHP_CHIP==7228) || (BCHP_CHIP==73625)
                || pSettings->external3445Lna.enabled
#endif
                ) {
            uint8_t sb;
            BDBG_MSG(("NEXUS_Frontend_Open7346: PGA"));
            errCode = BAST_ReadConfig(pDevice->astChannels[0], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
            if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }

            sb |= BAST_G3_CONFIG_TUNER_CTL_LNA_PGA_MODE;
            errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G3_CONFIG_TUNER_CTL, &sb, BAST_G3_CONFIG_LEN_TUNER_CTL);
            if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
        } else {
            if(pSettings->external3445Lna.enabled)
            {
                BDBG_MSG(("NEXUS_Frontend_Open7346: external LNA"));
                /* Store LNA I2C channel number once num channels is known */
                pDevice->lnaI2cChannel = BAST_Mi2cChannel_e0 + pSettings->external3445Lna.i2cChannelNumber;
                if ( pSettings->external3445Lna.i2cChannelNumber > pDevice->numChannels )
                {
                    BDBG_ERR(("LNA I2C Channel number invalid"));
                    errCode = BERR_TRACE(BERR_INVALID_PARAMETER);
                    goto destroy;
                }

                external3445LnaSettings.mi2c = pDevice->lnaI2cChannel;
                external3445LnaSettings.out1 = pSettings->external3445Lna.settings.out1;
                external3445LnaSettings.out2 = pSettings->external3445Lna.settings.out2;
                external3445LnaSettings.daisy = pSettings->external3445Lna.settings.daisy;
                errCode = BAST_ConfigBcm3445(pDevice->astHandle, &external3445LnaSettings);
                if ( errCode ) {
                    BDBG_ERR(("BAST_ConfigBcm3445() error"));
                    goto destroy;
                }

                for (i=0; i<pDevice->numChannels; i++) {
                    errCode = BAST_MapBcm3445ToTuner(pDevice->astChannels[i],
                            pDevice->lnaI2cChannel, pSettings->external3445Lna.lnaOutput);
                    if ( errCode != BERR_SUCCESS )
                    {
                        BDBG_WRN(("Unable to map LNA to frontend"));
                        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
                        return NULL;
                    }
                }
                pDevice->external3445LnaSettings = pSettings->external3445Lna.settings;

            }
        }

        if(pSettings->isInternalLna){
                            
            internalLnaSettings.out0 = pSettings->lnaSettings.out0;
            internalLnaSettings.out1 = pSettings->lnaSettings.out1;
            internalLnaSettings.daisy = pSettings->lnaSettings.daisy;
            
            BDBG_MSG(("NEXUS_Frontend_Open7346: internal LNA"));

            errCode = BAST_ConfigTunerLna(pDevice->astHandle, &internalLnaSettings);
            if (errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BAST_ConfigTunerLna() error"));
                goto destroy;
            }
            pDevice->internalLnaSettings = pSettings->lnaSettings;
        }

        /* configure fsk channels */
#if BCHP_CHIP==7344 || BCHP_CHIP==7358 || BCHP_CHIP==7360 || BCHP_CHIP==7362 || BCHP_CHIP==7228  || BCHP_CHIP==73625 /* TBD */
        /* Do not modify FSK channel yet */
        errCode = BAST_SetFskChannel(pDevice->astHandle, BAST_FskChannel_e0, BAST_FskChannel_e0);
        if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
        /* Do not modify FSK channel yet */
        errCode = BAST_SetFskChannel(pDevice->astHandle, BAST_FskChannel_e0, BAST_FskChannel_e0);
        if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
#elif BCHP_CHIP==7346 || BCHP_CHIP==73465
        errCode = BAST_SetFskChannel(pDevice->astHandle, BAST_FskChannel_e1, BAST_FskChannel_e1);
        if ( errCode ) { errCode = BERR_TRACE(errCode); goto destroy; }
#endif

#ifdef NEXUS_POWER_MANAGEMENT
    /*  power down all channels after 3445 config and mapping */
    for (i=0; i<pDevice->numChannels; i++) {
        if (pDevice->handles[i]==NULL) {
            BDBG_MSG(("Power down [%d]=%#lx", i, pDevice->astChannels[i]));
            BAST_PowerDown(pDevice->astChannels[i], BAST_CORE_ALL);
        }
    }
#endif

        BDBG_WRN(("Initializing 7346 core... Done"));

        /* Success, add to device list */
        g_p7346Device = pDevice;
    }
    else
    {
        BDBG_WRN(("Found existing device"));
        pFrontendDevice = pDevice->pGenericDeviceHandle;
    }

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7346Device);

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

#ifdef NEXUS_POWER_MANAGEMENT
    /* power up this channel propr to creating the matching AstDevice */
    BDBG_MSG(("Power up [%d]=%#lx", pSettings->channelNumber, pDevice->astChannels[pSettings->channelNumber]));
    BAST_PowerUp(pDevice->astChannels[pSettings->channelNumber], BAST_CORE_ALL);
#endif

    /* Open channel */
    NEXUS_Frontend_P_Ast_GetDefaultSettings(&astChannelSettings);
    astChannelSettings.astHandle = pDevice->astHandle;
    astChannelSettings.astChannel = pDevice->astChannels[pSettings->channelNumber];
#if (BCHP_CHIP > 9999)
#define B_AST_CHIP (BCHP_CHIP/10)
#else
#define B_AST_CHIP BCHP_CHIP
#endif
    astChannelSettings.astChip = B_AST_CHIP;
    astChannelSettings.restoreInternalLnaFunction = NEXUS_Frontend_P_7346_RestoreLnaCallback;
    astChannelSettings.pRestoreParam = pDevice;
    astChannelSettings.closeFunction = NEXUS_Frontend_P_7346_CloseCallback;
    astChannelSettings.pCloseParam = pDevice;
    astChannelSettings.channelIndex = pSettings->channelNumber;
    frontend = NEXUS_Frontend_P_Ast_Create(&astChannelSettings);
    if ( !frontend )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_7346_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }
    pDevice->handles[pSettings->channelNumber] = frontend;

#ifdef NEXUS_POWER_MANAGEMENT
    /* ...and leave this channel powered down */
    BDBG_MSG(("Power down [%d]=%#lx", pSettings->channelNumber, pDevice->astChannels[pSettings->channelNumber]));
    BAST_PowerDown(pDevice->astChannels[pSettings->channelNumber], BAST_CORE_ALL);
#endif

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x7346;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->close = NEXUS_Frontend_P_7346_DestroyDevice;
    frontend->pGenericDeviceHandle = pFrontendDevice;
    frontend->getSatelliteAgcStatus = NEXUS_Frontend_P_7346_GetSatelliteAgcStatus;

    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Ast_GetSatelliteCapabilities;
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    return frontend;

destroy:
    NEXUS_Frontend_P_7346_DestroyDevice(pDevice);
    return NULL;
}


static void NEXUS_Frontend_P_7346_RestoreLnaCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    NEXUS_7346Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7346Device);
    BSTD_UNUSED(handle);
    if(pDevice->settings.isInternalLna){
        BERR_Code errCode;
        BAST_TunerLnaSettings internalLnaSettings;

        internalLnaSettings.out0 = pDevice->internalLnaSettings.out0;
        internalLnaSettings.out1 = pDevice->internalLnaSettings.out1;
        internalLnaSettings.daisy = pDevice->internalLnaSettings.daisy;

        errCode = BAST_ConfigTunerLna(pDevice->astHandle, &internalLnaSettings);
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BAST_ConfigTunerLna() error"));
        }
    }
}

static void NEXUS_Frontend_P_7346_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_7346Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7346Device);

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

static void NEXUS_Frontend_P_7346_DestroyDevice(void *handle)
{
    NEXUS_7346Device *pDevice = (NEXUS_7346Device *)handle;
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_7346Device);

    BDBG_MSG(("Destroying 7346 device %p", pDevice));
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
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_7346Device);
    BKNI_Free(pDevice);
    g_p7346Device = NULL;
}

void NEXUS_Frontend_Get7346LnaSettings( NEXUS_FrontendHandle handle, NEXUS_7346LnaSettings *pSettings )
{
    
    NEXUS_7346Device *pDevice=NULL;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p7346Device;

    if ( NULL == pDevice )
    {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    else
    {
        *pSettings = pDevice->internalLnaSettings;        
    }
}

NEXUS_Error NEXUS_Frontend_Set7346LnaSettings( NEXUS_FrontendHandle handle, const NEXUS_7346LnaSettings *pSettings )
{
    NEXUS_7346Device *pDevice=NULL;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p7346Device;

    if ( NULL == pDevice )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        BAST_TunerLnaSettings internalLnaSettings;
        
                            
        internalLnaSettings.out0 = pSettings->out0;
        internalLnaSettings.out1 = pSettings->out1;
        internalLnaSettings.daisy = pSettings->daisy;
        
        errCode = BAST_ConfigTunerLna(pDevice->astHandle, &internalLnaSettings);
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("BAST_ConfigTunerLna() error"));
            return errCode;
        }
        
        pDevice->internalLnaSettings = *pSettings;    
    }

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_Frontend_Get7346LnaStatus( NEXUS_FrontendHandle handle, NEXUS_7346LnaStatus *pStatus )
{
    NEXUS_Error errCode;
    BAST_TunerLnaStatus lnaStatus;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    
    errCode = BAST_GetTunerLnaStatus(astDevice->astChannel, &lnaStatus);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    pStatus->lnaInput = lnaStatus.outCfg;
    pStatus->status = lnaStatus.status;
    pStatus->agc = lnaStatus.lnaAgc;
    pStatus->baseBandAgc = lnaStatus.bbAgc;

    return NEXUS_SUCCESS;
}

void NEXUS_Frontend_Get3445LnaSettings( NEXUS_FrontendHandle handle, NEXUS_3445LnaSettings *pSettings )
{
    
    NEXUS_7346Device *pDevice=NULL;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p7346Device;

    if ( NULL == pDevice )
    {
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    else
    {
        *pSettings = pDevice->external3445LnaSettings;
    }
}

NEXUS_Error NEXUS_Frontend_Set3445LnaSettings( NEXUS_FrontendHandle handle, const NEXUS_3445LnaSettings *pSettings )
{
    NEXUS_7346Device *pDevice=NULL;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_Frontend);
    BDBG_ASSERT(NULL != pSettings);

    pDevice = g_p7346Device;

    if ( NULL == pDevice )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    else
    {
        
        BAST_Bcm3445Settings external3445lnaSettings;

        external3445lnaSettings.mi2c = pDevice->lnaI2cChannel;
        external3445lnaSettings.out1 = pSettings->out1;
        external3445lnaSettings.out2 = pSettings->out2;
        external3445lnaSettings.daisy = pSettings->daisy;
        {
            uint8_t val;
            errCode = BAST_ReadConfig(pDevice->astChannels[0], BAST_G3_CONFIG_BCM3445_CTL, &val, BAST_G3_CONFIG_LEN_BCM3445_CTL);
            if ( errCode ) { return BERR_TRACE(errCode); }
            if (pSettings->lowPowerMode) {
                val |= BAST_G3_BCM3445_CTL_LOW_POWER_MODE;
            } else {
                val &= ~BAST_G3_BCM3445_CTL_LOW_POWER_MODE;
            }
            errCode = BAST_WriteConfig(pDevice->astChannels[0], BAST_G3_CONFIG_BCM3445_CTL, &val, BAST_G3_CONFIG_LEN_BCM3445_CTL);
            if ( errCode ) { return BERR_TRACE(errCode); }
        }
        errCode = BAST_ConfigBcm3445(pDevice->astHandle, &external3445lnaSettings);
        if ( errCode )
        {
            return BERR_TRACE(errCode);
        }

        pDevice->external3445LnaSettings = *pSettings;    
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Frontend_Get3445LnaStatus( NEXUS_FrontendHandle handle, NEXUS_3445LnaStatus *pStatus )
{
    NEXUS_Error errCode;
    BAST_Bcm3445Status lnaStatus;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    
    /* External LNA note: Your app should not call NEXUS_Frontend_Get7346LnaStatus. See first "External LNA note" above.
    TODO: consider failing this call if g_p7346Device->settings.lnaOutput == NEXUS_7346LnaOutput_eNone. */
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

NEXUS_I2cHandle NEXUS_Frontend_Get7346MasterI2c( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        return NULL;
    }
    
    if (!g_p7346Device) {
        BDBG_ERR(("Invalid frontend handle"));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    return g_p7346Device->i2cHandles[astDevice->settings.channelIndex];
}

static BERR_Code NEXUS_Frontend_P_7346_I2cReadNoAddr(void * context, uint16_t chipAddr, uint8_t *pData, size_t length)
{
    BAST_ChannelHandle handle = context;
    uint8_t dummy;

    BDBG_ASSERT(NULL != context);

    /* Issue a read with no preceding write */
    return BAST_ReadMi2c(handle, chipAddr<<1, &dummy, 0, pData, length);
}

static BERR_Code NEXUS_Frontend_P_7346_I2cWriteNoAddr(void * context, uint16_t chipAddr, const uint8_t *pData, size_t length)
{
    BAST_ChannelHandle handle = context;

    BDBG_ASSERT(NULL != context);

    /* BAST does not have a const pointer for data */
    return BAST_WriteMi2c(handle, chipAddr<<1, (uint8_t *)((unsigned long)pData), length);
}

static BERR_Code NEXUS_Frontend_P_7346_I2cRead(
    void * i2cHandle, /* I2C Handle */
    uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
    uint32_t subAddr, /* 32 bit sub address */
    uint8_t *pData, /* pointer to memory location to store read data */
    size_t length /* number of bytes to read */
    )
{
    uint8_t subAddr8;
    BAST_ChannelHandle handle = i2cHandle;

    BDBG_ASSERT(NULL != i2cHandle);

    if (subAddr & 0xFFFFFF00) {
        BDBG_WRN(("subAddr: 0x%08x is too large, expected uint8_t",subAddr));
    }
    subAddr8 = (uint8_t)subAddr;

    return BAST_ReadMi2c(handle, chipAddr<<1, &subAddr8, 1, pData, length);
}

static BERR_Code NEXUS_Frontend_P_7346_I2cWrite(
    void * i2cHandle, /* I2C Handle */
    uint16_t chipAddr, /* 7 or 10 bit chip address (_unshifted_) */
    uint32_t subAddr, /* 32 bit sub address */
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

static void NEXUS_Frontend_P_FtmEventCallback(void *context)
{
    NEXUS_7346Device *pDevice = context;
    unsigned i;
    for (i=0;i<NEXUS_7346_MAX_FRONTEND_CHANNELS;i++) {
        if (pDevice->handles[i]) {
            NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(pDevice->handles[i]);
            if (astDevice) {
                NEXUS_TaskCallback_Fire(astDevice->ftmCallback);
            }
        }
    }
}

void NEXUS_Frontend_Get7346TuneSettings( NEXUS_FrontendHandle handle, NEXUS_Frontend7346TuneSettings *pSettings )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
    else {
        *pSettings = g_p7346Device->tuneSettings[astDevice->settings.channelIndex];
    }
}

NEXUS_Error NEXUS_Frontend_Set7346TuneSettings( NEXUS_FrontendHandle handle, const NEXUS_Frontend7346TuneSettings *pSettings )
{
    NEXUS_Error rc;
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    NEXUS_Frontend7346TuneSettings *pCurrent;
    
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    
    pCurrent = &g_p7346Device->tuneSettings[astDevice->settings.channelIndex];

    if (pCurrent->disableFecReacquire != pSettings->disableFecReacquire) {
        unsigned char buf[1];
#if BCHP_CHIP==7344 || BCHP_CHIP==7346 || BCHP_CHIP==7358 || BCHP_CHIP==7360 || BCHP_CHIP==7362 || BCHP_CHIP==7228 || BCHP_CHIP==73625 || BCHP_CHIP==73465
        rc = BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TURBO_CTL, buf, BAST_G3_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
        rc = BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TURBO_CTL, buf, BAST_G3_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
#else
        rc = BAST_ReadConfig(astDevice->astChannel, BCM7346_CONFIG_TURBO_CTL, buf, BCM7346_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);

        buf[0] &= ~(BAST_7346_TURBO_CTL_DISABLE_FEC_REACQ);
        if (pSettings->disableFecReacquire) {

            buf[0] |= BAST_7346_TURBO_CTL_DISABLE_FEC_REACQ;
        }

        rc = BAST_WriteConfig(astDevice->astChannel, BCM7346_CONFIG_TURBO_CTL, buf, BCM7346_CONFIG_LEN_TURBO_CTL);
        if (rc) return BERR_TRACE(rc);
#endif
    }

    if (pCurrent->bypassLnaGain != pSettings->bypassLnaGain) {
        uint8_t tuner_ctl;
        BAST_ReadConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
        if (pSettings->bypassLnaGain) {
            tuner_ctl |= BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS;
        } else {
            tuner_ctl &= ~BAST_G3_CONFIG_TUNER_CTL_LNA_BYPASS;
        }
        BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_CTL, &tuner_ctl, BAST_G3_CONFIG_LEN_TUNER_CTL);
    }

    if (pSettings->daisyGain.override) {
        if (pCurrent->daisyGain.value != pSettings->daisyGain.value) {
            uint8_t gain = pSettings->daisyGain.value;
            BAST_WriteConfig(astDevice->astChannel, BAST_G3_CONFIG_TUNER_DAISY_GAIN, &gain, BAST_G3_CONFIG_LEN_TUNER_DAISY_GAIN);
        }
    }

    *pCurrent = *pSettings;
    return 0;
}

static NEXUS_Error NEXUS_Frontend_P_7346_GetSatelliteAgcStatus(void *handle, NEXUS_FrontendSatelliteAgcStatus *pStatus)
{
    NEXUS_AstDevice *pAstDevice = (NEXUS_AstDevice *)handle;
    NEXUS_7346Device *pDevice=NULL;
    BERR_Code errCode;

    BAST_ChannelStatus astStatus;
    BAST_TunerLnaStatus lnaStatus;

    BDBG_ASSERT(NULL != pStatus);

    if (pAstDevice->astChip != B_AST_CHIP) {
        return NEXUS_INVALID_PARAMETER;
    }

    pDevice = g_p7346Device;

    BKNI_Memset(pStatus,0,sizeof(*pStatus));

    errCode = BAST_GetChannelStatus(pAstDevice->astChannel, &astStatus);
    if ( !errCode ) {
        pStatus->agc[0].valid = true;
        pStatus->agc[0].value = astStatus.IFagc;
        pStatus->agc[1].valid = true;
        pStatus->agc[1].value = astStatus.agf;
        pStatus->agc[2].valid = true;
        pStatus->agc[2].value = astStatus.tunerFreq;
    }

    errCode = BAST_GetTunerLnaStatus(pAstDevice->astChannel, &lnaStatus);
    if ( !errCode )
    {
        pStatus->agc[3].valid = true;
        pStatus->agc[3].value = lnaStatus.lnaAgc;
        pStatus->agc[4].valid = true;
        pStatus->agc[4].value = lnaStatus.bbAgc;
    }

    return NEXUS_SUCCESS;
}
