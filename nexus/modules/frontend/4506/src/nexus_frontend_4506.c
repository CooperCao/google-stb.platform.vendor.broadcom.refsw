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
*   API name: Frontend 4506
*    APIs to open, close, and setup initial settings for a BCM4506
*    Dual-Channel Satellite Tuner/Demodulator Device.
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include "nexus_frontend_module.h"
#include "nexus_frontend_ast.h"
#include "blst_slist.h"
#include "priv/nexus_i2c_priv.h"
#include "priv/nexus_gpio_priv.h"
#include "bast.h"
#include "bast_4506.h"
#include "bast_4506_priv.h"
#include "bast_4506_fw.h"
#include "bchp_4506.h"

BDBG_MODULE(nexus_frontend_4506);

BDBG_OBJECT_ID(NEXUS_4506Device);

typedef struct NEXUS_4506Device
{
    BDBG_OBJECT(NEXUS_4506Device)
    BLST_S_ENTRY(NEXUS_4506Device) node;
    NEXUS_4506Settings settings;
    BAST_Handle astHandle;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BAST_ChannelHandle astChannels[NEXUS_4506_MAX_FRONTEND_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_4506_MAX_FRONTEND_CHANNELS];
    uint32_t numChannels;   /* prototype to match BAST_GetTotalChannels */
    NEXUS_EventCallbackHandle ftmEventCallback;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    bool isMaster;
#endif
    NEXUS_FrontendDevice *pGenericDeviceHandle;
} NEXUS_4506Device;

static BLST_S_HEAD(devList, NEXUS_4506Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

#define MAX_4506_FRONTEND_DEVICES 8
static NEXUS_FrontendDeviceHandle g_deviceHandles[MAX_4506_FRONTEND_DEVICES] = {NULL};

static void NEXUS_Frontend_P_4506_IsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4506_GpioIsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4506_L1_isr(void *param1, int param2);
static void NEXUS_Frontend_P_4506_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_4506_DestroyDevice(void *handle);
static void NEXUS_Frontend_P_4506_IsrCallback(void *pParam);
static void NEXUS_Frontend_P_FtmEventCallback(void *context);

/***************************************************************************
Summary:
    Get the default settings for a BCM4506 frontend
See Also:
    NEXUS_Frontend_Open4506
 ***************************************************************************/
void NEXUS_Frontend_GetDefault4506Settings(
    NEXUS_4506Settings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = 0x69;
}

/***************************************************************************
Summary:
    Open a handle to a BCM4506 device.
See Also:
    NEXUS_Frontend_Close4506
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open4506(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_4506Settings *pSettings
    )
{
    NEXUS_FrontendDevice *pFrontendDevice = NULL;
    unsigned i;
    NEXUS_Error errCode;
    NEXUS_4506Device *pDevice=NULL;
    NEXUS_FrontendAstSettings astChannelSettings;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pSettings->i2cDevice);

    for ( pDevice = BLST_S_FIRST(&g_deviceList);
          NULL != pDevice;
          pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);
        if ( pSettings->i2cDevice == pDevice->settings.i2cDevice &&
             pSettings->i2cAddr == pDevice->settings.i2cAddr )
        {
            break;
        }
    }

    if ( NULL == pDevice )
    {
        BAST_Settings astSettings;
        BREG_I2C_Handle i2cHandle;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT        
        for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
        {       
            BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);
            if ((pSettings->gpioInterrupt == pDevice->settings.gpioInterrupt) && (pSettings->isrNumber == pDevice->settings.isrNumber))
            {
                pDevice->isMaster = false;
            }
        }
#endif
        pFrontendDevice = BKNI_Malloc(sizeof(*pFrontendDevice));
        if (NULL == pFrontendDevice) {BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); return NULL;}
        pFrontendDevice->familyId = 0x4506;
        {
            int ii;
            for (ii=0; ii < MAX_4506_FRONTEND_DEVICES;ii++) {
                if (g_deviceHandles[ii] == NULL)
                    g_deviceHandles[ii] = pFrontendDevice;
                break;
            }
        }

        /* Memsetting the whole structure should cover initializing the child list. */
        BKNI_Memset(pFrontendDevice, 0, sizeof(*pFrontendDevice));

        BDBG_MSG(("Opening new 4506 device"));
        pDevice = BKNI_Malloc(sizeof(NEXUS_4506Device));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_4506Device);
        pDevice->settings = *pSettings;
        pDevice->pGenericDeviceHandle = pFrontendDevice;

        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);

        BAST_4506_GetDefaultSettings(&astSettings);
        astSettings.i2c.chipAddr = pSettings->i2cAddr;
        if(pSettings->gpioInterrupt){
            astSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_4506_GpioIsrControl_isr;
            astSettings.i2c.interruptEnableFuncParam = (void*)pSettings->gpioInterrupt;
        }
        else if(pSettings->isrNumber) {
            astSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_4506_IsrControl_isr;
            astSettings.i2c.interruptEnableFuncParam = (void*)pSettings->isrNumber;
        }
        errCode = BAST_Open(&pDevice->astHandle, NULL /*chip*/, i2cHandle, NULL /*BINT*/, &astSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }

        /* Determine number of channels -- they will be opened later */
        BAST_GetTotalChannels(pDevice->astHandle, &pDevice->numChannels);
        if ( pDevice->numChannels > NEXUS_4506_MAX_FRONTEND_CHANNELS )
        {
            BDBG_WRN(("This 4506 device supports more than the expected number of channels.  Settings to %u", NEXUS_4506_MAX_FRONTEND_CHANNELS));
            pDevice->numChannels = NEXUS_4506_MAX_FRONTEND_CHANNELS;
        }

        /* Open all channels prior to InitAp */
        for ( i = 0; i < pDevice->numChannels; i++ )
        {
            BAST_ChannelSettings bastChannelSettings;
            BAST_4506_GetChannelDefaultSettings(pDevice->astHandle, i, &bastChannelSettings);
            errCode = BAST_OpenChannel(pDevice->astHandle, &pDevice->astChannels[i], i, &bastChannelSettings);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto destroy_device;
            }
        }
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        pDevice->isMaster = true;
        /* disconnect the previous for shared interrupt, last one is master */
        if(pSettings->isrNumber) {
            NEXUS_Core_DisconnectInterrupt(pSettings->isrNumber);
        }
        else if(pSettings->gpioInterrupt) {
            NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NULL, NULL, 0);
        }
#endif 
        /* Success opeining the AST Device.  Connect Interrupt */
        if(pSettings->isrNumber) {      
            errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                                 NEXUS_Frontend_P_4506_L1_isr,
                                                 (void *)pDevice,
                                                 (int)pDevice->astHandle);
            if ( errCode != BERR_SUCCESS )
            {
                errCode = BERR_TRACE(errCode);
                goto destroy_device;
            }
       }
       else if(pSettings->gpioInterrupt){
           NEXUS_Gpio_SetInterruptCallback_priv(pSettings->gpioInterrupt, NEXUS_Frontend_P_4506_L1_isr, (void *)pDevice, (int)pDevice->astHandle);
       }
       
        BDBG_WRN(("Initializing 4506 core..."));
        /* Init the acquisition processor */
        errCode = BAST_InitAp(pDevice->astHandle, bcm4506_ap_image);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }
        else
        {   
            if(pSettings->is3445ExternalLna){
                uint16_t chipId;
                uint32_t bondId;
                uint8_t chipVer, apVer, cfgVer; 
                BAST_Bcm3445Settings lnaSettings;

                errCode = BAST_GetApVersion(pDevice->astHandle, &chipId, &chipVer, &bondId, &apVer, &cfgVer);
                
                /* configure lna for In1Out1 In2Out2 */
                lnaSettings.out1 = BAST_Bcm3445OutputConfig_eIn1Vga;
                lnaSettings.out2 = BAST_Bcm3445OutputConfig_eIn2Vga;
                lnaSettings.daisy = BAST_Bcm3445OutputConfig_eIn2Vga;
                errCode = BAST_ConfigBcm3445(pDevice->astHandle, &lnaSettings);
                if (errCode != BERR_SUCCESS)
                {
                    errCode = BERR_TRACE(errCode);
                    goto destroy_device;
                }

                /* map lna outputs to tuners */
                errCode = BAST_MapBcm3445ToTuner(pDevice->astChannels[0], BAST_Mi2cChannel_e0, BAST_Bcm3445OutputChannel_eOut1);
                if (errCode != BERR_SUCCESS)
                {
                    errCode = BERR_TRACE(errCode);
                    goto destroy_device;
                }

                

                if (chipId != 0x4505)
                {
                    /* map lna outputs to second channel only for dual tuner chips */
                    errCode = BAST_MapBcm3445ToTuner(pDevice->astChannels[1], BAST_Mi2cChannel_e0, BAST_Bcm3445OutputChannel_eDaisy);
                    if (errCode != BERR_SUCCESS)
                    {
                        errCode = BERR_TRACE(errCode);
                        goto destroy_device;
                    }
                }
            }
            if (pSettings->bitWideSync) {
                uint8_t buf[2];
                errCode = BAST_ReadConfig(pDevice->astChannels[0], BCM4506_CONFIG_XPORT_CTL, buf, BCM4506_CONFIG_LEN_XPORT_CTL);
                if (errCode) {errCode = BERR_TRACE(errCode); goto destroy_device;}
                buf[0] |= (BCM4506_XPORT_CTL_SYNC1>>8)&0xFF;
                buf[1] |= BCM4506_XPORT_CTL_SYNC1&0xFF;
                errCode = BAST_WriteConfig(pDevice->astChannels[0], BCM4506_CONFIG_XPORT_CTL, buf, BCM4506_CONFIG_LEN_XPORT_CTL);
                if (errCode) {errCode = BERR_TRACE(errCode); goto destroy_device;}
            }
        }

        BAST_GetInterruptEventHandle(pDevice->astHandle, &pDevice->isrEvent);
        pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_4506_IsrCallback, pDevice);
        if ( NULL == pDevice->isrCallback )
        {
            errCode = BERR_TRACE(BERR_OS_ERROR);
            goto destroy_device;
        }

        {
            BKNI_EventHandle event;
            errCode = BAST_GetFtmEventHandle(pDevice->astHandle, &event);
            if ( errCode ) {
                errCode = BERR_TRACE(errCode);
                goto destroy_device;
            }

            pDevice->ftmEventCallback = NEXUS_RegisterEvent(event, NEXUS_Frontend_P_FtmEventCallback, pDevice);
            if ( !pDevice->ftmEventCallback ) {
                errCode = BERR_TRACE(NEXUS_UNKNOWN);
                goto destroy_device;
            }
        }

        /* Success, add to device list */
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        BKNI_EnterCriticalSection();
        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
        BKNI_LeaveCriticalSection();
#else
        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
#endif
    }
    else
    {
        BDBG_MSG(("Found existing device"));
        pFrontendDevice = pDevice->pGenericDeviceHandle;
    }

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);

    if ( pSettings->channelNumber >= pDevice->numChannels )
    {
        BDBG_ERR(("Channel %u not supported on this device", pSettings->channelNumber));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }
    if ( NULL != pDevice->handles[pSettings->channelNumber] )
    {
        BDBG_WRN(("Channel %u already open", pSettings->channelNumber));
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        return NULL;
    }

    /* Open channel */
    NEXUS_Frontend_P_Ast_GetDefaultSettings(&astChannelSettings);
    astChannelSettings.astHandle = pDevice->astHandle;
    astChannelSettings.astChannel = pDevice->astChannels[pSettings->channelNumber];
#define B_AST_CHIP 4506
    astChannelSettings.astChip = B_AST_CHIP;
    astChannelSettings.closeFunction = NEXUS_Frontend_P_4506_CloseCallback;
    astChannelSettings.pCloseParam = pDevice;
    astChannelSettings.channelIndex = pSettings->channelNumber;

    pDevice->handles[pSettings->channelNumber] = NEXUS_Frontend_P_Ast_Create(&astChannelSettings);
    if ( NULL == pDevice->handles[pSettings->channelNumber] )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_4506_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }

    pFrontendDevice->pDevice = pDevice;
    pFrontendDevice->familyId = 0x4506;
    pFrontendDevice->application = NEXUS_FrontendDeviceApplication_eSatellite;
    pFrontendDevice->close = NEXUS_Frontend_P_4506_DestroyDevice;
    pDevice->handles[pSettings->channelNumber]->pGenericDeviceHandle = pFrontendDevice;

    pFrontendDevice->getSatelliteCapabilities = NEXUS_FrontendDevice_P_Ast_GetSatelliteCapabilities;
    pFrontendDevice->nonblocking.getSatelliteCapabilities = true; /* does not require init complete to fetch number of demods */

    return pDevice->handles[pSettings->channelNumber];

destroy_device:
    NEXUS_Core_DisconnectInterrupt(pSettings->isrNumber);
    for ( i = 0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++) {
        BAST_CloseChannel(pDevice->astChannels[i]);
    }
    if (pDevice->astHandle) {
        BAST_Close(pDevice->astHandle);
    }
    {
        int ii;
        for (ii=0; ii < MAX_4506_FRONTEND_DEVICES;ii++) {
            if (g_deviceHandles[ii] == pFrontendDevice)
                g_deviceHandles[ii] = NULL;
        }
    }
    BKNI_Free(pFrontendDevice);
    BKNI_Free(pDevice);

    return NULL;
}

/***************************************************************************
Summary:
    Probe to see if a BCM4506 device exists with the specified settings
See Also:
    NEXUS_Frontend_Open4506
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe4506(
    const NEXUS_4506Settings *pSettings,
    NEXUS_4506ProbeResults *pResults        /* [out] */
    )
{
    int i;
    uint8_t sb, buf[4];
    bool found = false;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BDBG_MSG(("Probing 4506 with I2C Address 0x%02x", pSettings->i2cAddr));

    /* read TM_CHIP_ID register */
    sb = 0;
    if (NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr, BCM4506_SH_SFR_IO_MBOX_A_15_8, &sb, 1) != BERR_SUCCESS)
        return NEXUS_INVALID_PARAMETER;
    sb = 0x3C;
    if (NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr, BCM4506_SH_SFR_IO_MBOX_CMD, &sb, 1) != BERR_SUCCESS)
        return NEXUS_INVALID_PARAMETER;

    for (i = 0; i < 3 && !found; i++)
    {
        if (NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr, BCM4506_SH_SFR_IO_MBOX_STATUS, &sb, 1) != BERR_SUCCESS)
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        if ((sb & 0x80) == 0)
        {
            if ((sb & 0x40) == 0)
            {
                /* transfer completed - now get the data */
                if (NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr, BCM4506_SH_SFR_IO_MBOX_D_31_24, buf, 4) == BERR_SUCCESS)
                {
                    BDBG_MSG(("TM_CHIP_ID = 0x%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]));
                    if ((buf[2] == 0x44) && (buf[3] == 0xF9))
                    {
                        pResults->revision = 0x01;  /*TODO: hardcoded to A1 */
                        BDBG_WRN(("4506 detected with rev %02x", pResults->revision));
                        return BERR_SUCCESS;
                    }
                }
            }
            break;
        }
    }

    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static void NEXUS_Frontend_P_FtmEventCallback(void *context)
{
    unsigned i;
    NEXUS_4506Device *pDevice = context;
    /* We don't know which channel sent the message */
    for (i=0;i<NEXUS_4506_MAX_FRONTEND_CHANNELS;i++) {
        if (pDevice->handles[i]) {
            NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(pDevice->handles[i]);
            if (astDevice) {
                NEXUS_TaskCallback_Fire(astDevice->ftmCallback);
            }
        }
    }
}

static void NEXUS_Frontend_P_4506_L1_isr(void *param1, int param2)
{
    BAST_Handle astHandle = (BAST_Handle)param2;
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    NEXUS_4506Device *pDevice, *pIntDevice = (NEXUS_4506Device *)param1;
#else
    BSTD_UNUSED(param1);
#endif
    BDBG_MSG(("4506 L1 ISR"));
    BAST_HandleInterrupt_isr(astHandle);

#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
    for ( pDevice = BLST_S_FIRST(&g_deviceList); NULL != pDevice; pDevice = BLST_S_NEXT(pDevice, node) )
    {       
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);

        if ((astHandle != NULL) && 
            (pDevice->settings.isrNumber== pIntDevice->settings.isrNumber) && 
            (pDevice->settings.gpioInterrupt == pIntDevice->settings.gpioInterrupt) && 
            (pDevice->astHandle != astHandle))
        {
            BAST_HandleInterrupt_isr(pDevice->astHandle);
        }
    }
#endif  
}

static void NEXUS_Frontend_P_4506_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_4506Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);

    /* Mark handle as destroyed */
    if ( handle )
    {
        for ( i = 0; i < pDevice->numChannels; i++ )
        {
            if ( handle == pDevice->handles[i] )
            {
                pDevice->handles[i] = NULL;
                break;
            }
        }
        if ( i >= pDevice->numChannels )
        {
            BDBG_ERR(("Channel mismatch?"));
            BDBG_ASSERT(i < pDevice->numChannels);
        }
    }

    /* See if any channels are still open */
    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( pDevice->handles[i] )
        {
            return;
        }
    }

}

static void NEXUS_Frontend_P_4506_DestroyDevice(void *handle)
{
    NEXUS_4506Device *pDevice = (NEXUS_4506Device *)handle;
    unsigned i;
    NEXUS_GpioSettings gpioSettings;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    BDBG_MSG(("Destroying 4506 device %p", (void *)pDevice));

    NEXUS_UnregisterEvent(pDevice->isrCallback);

    if (pDevice->ftmEventCallback) {
        NEXUS_UnregisterEvent(pDevice->ftmEventCallback);
    }

    if(pDevice->settings.isrNumber) {
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber); 
        }
#else
        NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber); 
#endif
    }
    else if(pDevice->settings.gpioInterrupt){       
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        if(pDevice->isMaster)
        {
            NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
            NEXUS_Gpio_GetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
            gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
            gpioSettings.interrupt.callback = NULL;
            NEXUS_Gpio_SetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        }
#else
        NEXUS_Gpio_SetInterruptCallback_priv(pDevice->settings.gpioInterrupt, NULL, NULL, 0);
        NEXUS_Gpio_GetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);
        gpioSettings.interruptMode = NEXUS_GpioInterrupt_eDisabled;
        gpioSettings.interrupt.callback = NULL;
        NEXUS_Gpio_SetSettings(pDevice->settings.gpioInterrupt, &gpioSettings);

#endif      
    }
#ifdef NEXUS_SHARED_FRONTEND_INTERRUPT
        BKNI_EnterCriticalSection();
        BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_4506Device, node);
        BKNI_LeaveCriticalSection(); 
#else
        BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_4506Device, node);
#endif

    for ( i = 0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++)
        BAST_CloseChannel(pDevice->astChannels[i]);
    BAST_Close(pDevice->astHandle);
    if (pDevice->pGenericDeviceHandle) {
        for (i=0; i < MAX_4506_FRONTEND_DEVICES; i++) {
            if ( g_deviceHandles[i] == pDevice->pGenericDeviceHandle) {
                g_deviceHandles[i] = NULL;
            }
        }
        BKNI_Free(pDevice->pGenericDeviceHandle);
    }
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_4506Device);
    BKNI_Free(pDevice);
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 4506 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4506_IsrControl_isr(bool enable, void *pParam)
{
    int isrNumber = (int)pParam;

    if ( enable )
    {
        BDBG_MSG(("Enable 4506 Interrupt %u", isrNumber));
        NEXUS_Core_EnableInterrupt_isr(isrNumber);
    }
    else
    {
        BDBG_MSG(("Disable 4506 Interrupt %u", isrNumber));
        NEXUS_Core_DisableInterrupt_isr(isrNumber);
    }
}

/***************************************************************************
Summary:
    Enable/Disable gpio interrupts for a 4506 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4506_GpioIsrControl_isr(bool enable, void *pParam)
{
    NEXUS_GpioHandle gpioHandle = (NEXUS_GpioHandle)pParam;

    if(enable){ 
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, true);
    }
    else {
        NEXUS_Gpio_SetInterruptEnabled_isr(gpioHandle, false);
    }

   
}
static void NEXUS_Frontend_P_4506_IsrCallback(void *pParam)
{
    NEXUS_4506Device *pDevice = (NEXUS_4506Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4506Device);
    BAST_ProcessInterruptEvent(pDevice->astHandle);
}

NEXUS_I2cHandle NEXUS_Frontend_Get4506MasterI2c( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        return NULL;
    }
    return astDevice->deviceI2cHandle;
}

NEXUS_Error NEXUS_Frontend_Write4506HostRegister( NEXUS_FrontendHandle handle, uint8_t address, uint8_t data )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4506_WriteHostRegister(astDevice->astHandle, address, &data);
}

NEXUS_Error NEXUS_Frontend_Read4506HostRegister( NEXUS_FrontendHandle handle, uint8_t address, uint8_t *pData )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4506_ReadHostRegister(astDevice->astHandle, address, pData);
}

NEXUS_Error NEXUS_Frontend_Read4506Memory( NEXUS_FrontendHandle handle, uint16_t addr, uint8_t *buffer, uint16_t bufferSize )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4506_P_ReadMemory(astDevice->astHandle, addr, buffer, bufferSize);
}

NEXUS_Error NEXUS_Frontend_Write4506Memory( NEXUS_FrontendHandle handle, uint16_t addr, const uint8_t *buffer, uint16_t bufferSize )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    if(buffer==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return BAST_4506_P_WriteMemory(astDevice->astHandle, addr, buffer, bufferSize);
}

