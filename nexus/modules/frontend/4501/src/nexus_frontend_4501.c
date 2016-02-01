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
*   API name: Frontend 4501
*    APIs to open, close, and setup initial settings for a BCM4501
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
#include "bast.h"
#include "bast_4501.h"
#include "bast_4501_priv.h"
#include "bast_4501_fw.h"
#include "bchp_4501.h"

BDBG_MODULE(nexus_frontend_4501);

BDBG_OBJECT_ID(NEXUS_4501Device);

#define NEXUS_MAX_NUM_4501_CHANNELS 2
typedef struct NEXUS_4501Device
{
    BDBG_OBJECT(NEXUS_4501Device)
    BLST_S_ENTRY(NEXUS_4501Device) node;
    NEXUS_4501Settings settings;
    BAST_Handle astHandle;
    BKNI_EventHandle isrEvent;
    NEXUS_EventCallbackHandle isrCallback;
    BAST_ChannelHandle astChannels[NEXUS_MAX_NUM_4501_CHANNELS];
    NEXUS_FrontendHandle handles[NEXUS_MAX_NUM_4501_CHANNELS];
    uint32_t numChannels;   /* prototype to match BAST_GetTotalChannels */
} NEXUS_4501Device;

static BLST_S_HEAD(devList, NEXUS_4501Device) g_deviceList = BLST_S_INITIALIZER(g_deviceList);

static void NEXUS_Frontend_P_4501_IsrControl_isr(bool enable, void *pParam);
static void NEXUS_Frontend_P_4501_L1_isr(void *param1, int param2);
static void NEXUS_Frontend_P_4501_CloseCallback(NEXUS_FrontendHandle handle, void *pParam);
static void NEXUS_Frontend_P_4501_DestroyDevice(NEXUS_4501Device *pDevice);
static void NEXUS_Frontend_P_4501_IsrCallback(void *pParam);

/***************************************************************************
Summary:
    Get the default settings for a BCM4501 frontend
See Also:
    NEXUS_Frontend_Open4501
 ***************************************************************************/
void NEXUS_Frontend_GetDefault4501Settings(
    NEXUS_4501Settings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->i2cAddr = 0x69;
}

/***************************************************************************
Summary:
    Open a handle to a BCM4501 device.
See Also:
    NEXUS_Frontend_Close4501
 ***************************************************************************/
NEXUS_FrontendHandle NEXUS_Frontend_Open4501(  /* attr{destructor=NEXUS_Frontend_Close} */
    const NEXUS_4501Settings *pSettings
    )
{
    unsigned i;
    NEXUS_Error errCode;
    NEXUS_4501Device *pDevice=NULL;
    NEXUS_FrontendAstSettings astChannelSettings;

    if (!pSettings || !pSettings->i2cDevice) {
        errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return NULL;
    }

    for ( pDevice = BLST_S_FIRST(&g_deviceList);
          NULL != pDevice;
          pDevice = BLST_S_NEXT(pDevice, node) )
    {
        BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);
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

        BDBG_MSG(("Opening new 4501 device"));
        pDevice = BKNI_Malloc(sizeof(NEXUS_4501Device));
        if ( NULL == pDevice )
        {
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            return NULL;
        }
        BKNI_Memset(pDevice, 0, sizeof(*pDevice));
        BDBG_OBJECT_SET(pDevice, NEXUS_4501Device);
        pDevice->settings = *pSettings;

        i2cHandle = NEXUS_I2c_GetRegHandle(pSettings->i2cDevice, NEXUS_MODULE_SELF);
        BDBG_ASSERT(NULL != i2cHandle);

        BAST_4501_GetDefaultSettings(&astSettings);
        astSettings.i2c.chipAddr = pSettings->i2cAddr;
        astSettings.i2c.interruptEnableFunc = NEXUS_Frontend_P_4501_IsrControl_isr;
        astSettings.i2c.interruptEnableFuncParam = (void *)pDevice;
        errCode = BAST_Open(&pDevice->astHandle, NULL /*chip*/, i2cHandle, NULL /*BINT*/, &astSettings);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }

        /* Determine number of channels -- they will be opened later */
        BAST_GetTotalChannels(pDevice->astHandle, &pDevice->numChannels);
        if ( pDevice->numChannels > NEXUS_MAX_NUM_4501_CHANNELS )
        {
            BDBG_WRN(("This 4501 device supports more than the expected number of channels.  Settings to %u", NEXUS_MAX_NUM_4501_CHANNELS));
            pDevice->numChannels = NEXUS_MAX_NUM_4501_CHANNELS;
        }

        /* Open all channels prior to InitAp */
        for ( i = 0; i < pDevice->numChannels; i++ )
        {
            BAST_ChannelSettings bastChannelSettings;
            BAST_4501_GetChannelDefaultSettings(pDevice->astHandle, i, &bastChannelSettings);
            errCode = BAST_OpenChannel(pDevice->astHandle, &pDevice->astChannels[i], i, &bastChannelSettings);
            if ( errCode )
            {
                errCode = BERR_TRACE(errCode);
                goto destroy_device;
            }
        }

        /* Success opeining the AST Device.  Connect Interrupt */
        errCode = NEXUS_Core_ConnectInterrupt(pSettings->isrNumber,
                                              NEXUS_Frontend_P_4501_L1_isr,
                                              (void *)pDevice,
                                              (int)pDevice->astHandle);
        if ( errCode != BERR_SUCCESS )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }

        BDBG_WRN(("Initializating 4501 core..."));
        /* Init the acquisition processor */
        errCode = BAST_InitAp(pDevice->astHandle, bcm4501_ap_image);
        if ( errCode )
        {
            errCode = BERR_TRACE(errCode);
            goto destroy_device;
        }
        BDBG_WRN(("Initializating 4501 core... Done"));

        if (pSettings->bitWideSync) {
            uint8_t buf[2];
            errCode = BAST_ReadConfig(pDevice->astChannels[0], BCM4501_CONFIG_XPORT_CTL, buf, BCM4501_CONFIG_LEN_XPORT_CTL);
            if (errCode) {errCode = BERR_TRACE(errCode); goto destroy_device;}
            buf[0] |= (BCM4501_XPORT_CTL_SYNC1>>8)&0xFF;
            buf[1] |= BCM4501_XPORT_CTL_SYNC1&0xFF;
            errCode = BAST_WriteConfig(pDevice->astChannels[0], BCM4501_CONFIG_XPORT_CTL, buf, BCM4501_CONFIG_LEN_XPORT_CTL);
            if (errCode) {errCode = BERR_TRACE(errCode); goto destroy_device;}
        }

        BAST_GetInterruptEventHandle(pDevice->astHandle, &pDevice->isrEvent);
        pDevice->isrCallback = NEXUS_RegisterEvent(pDevice->isrEvent, NEXUS_Frontend_P_4501_IsrCallback, pDevice);
        if ( NULL == pDevice->isrCallback )
        {
            errCode = BERR_TRACE(NEXUS_UNKNOWN);
            goto destroy_device;
        }

        /* Success, add to device list */
        BLST_S_INSERT_HEAD(&g_deviceList, pDevice, node);
    }
    else
    {
        BDBG_MSG(("Found existing device"));
    }

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);

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

    /* Open channel */
    NEXUS_Frontend_P_Ast_GetDefaultSettings(&astChannelSettings);
    astChannelSettings.astHandle = pDevice->astHandle;
    astChannelSettings.astChannel = pDevice->astChannels[pSettings->channelNumber];
#define B_AST_CHIP 4501
    astChannelSettings.astChip = B_AST_CHIP;
    astChannelSettings.closeFunction = NEXUS_Frontend_P_4501_CloseCallback;
    astChannelSettings.pCloseParam = pDevice;
    astChannelSettings.channelIndex = pSettings->channelNumber;

    pDevice->handles[pSettings->channelNumber] = NEXUS_Frontend_P_Ast_Create(&astChannelSettings);
    if ( NULL == pDevice->handles[pSettings->channelNumber] )
    {
        errCode = BERR_TRACE(BERR_NOT_SUPPORTED);
        NEXUS_Frontend_P_4501_CloseCallback(NULL, pDevice); /* Check if channel needs to be closed */
        return NULL;
    }

    return pDevice->handles[pSettings->channelNumber];

destroy_device:
    NEXUS_Core_DisconnectInterrupt(pSettings->isrNumber);
    for ( i = 0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++) {
        BAST_CloseChannel(pDevice->astChannels[i]);
    }
    if (pDevice->astHandle) {
        BAST_Close(pDevice->astHandle);
    }
	BKNI_Free(pDevice);

    return NULL;
}

/***************************************************************************
Summary:
    Probe to see if a BCM4501 device exists with the specified settings
See Also:
    NEXUS_Frontend_Open4501
 ***************************************************************************/
NEXUS_Error NEXUS_Frontend_Probe4501(
    const NEXUS_4501Settings *pSettings,
    NEXUS_4501ProbeResults *pResults        /* [out] */
    )
{
    int i;
    uint8_t sb, buf[4];
    bool found = false;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pResults);

    BDBG_MSG(("Probing 4501 with I2C Address 0x%02x", pSettings->i2cAddr));

    /* read TM_CHIP_ID register */
    sb = 0;
    if (NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr, BCM4501_SH_SFR_IO_MBOX_A_15_8, &sb, 1) != BERR_SUCCESS)
        return NEXUS_INVALID_PARAMETER;
    sb = 0x3C;
    if (NEXUS_I2c_Write(pSettings->i2cDevice, pSettings->i2cAddr, BCM4501_SH_SFR_IO_MBOX_CMD, &sb, 1) != BERR_SUCCESS)
        return NEXUS_INVALID_PARAMETER;

    for (i = 0; i < 3 && !found; i++)
    {
        if (NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr, BCM4501_SH_SFR_IO_MBOX_STATUS, &sb, 1) != BERR_SUCCESS)
            return BERR_TRACE(BERR_INVALID_PARAMETER);

        if ((sb & 0x80) == 0)
        {
            if ((sb & 0x40) == 0)
            {
                /* transfer completed - now get the data */
                if (NEXUS_I2c_Read(pSettings->i2cDevice, pSettings->i2cAddr, BCM4501_SH_SFR_IO_MBOX_D_31_24, buf, 4) == BERR_SUCCESS)
                {
                    BDBG_MSG(("TM_CHIP_ID = 0x%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3]));
                    if ((buf[0] == 0x45) && (buf[1] == 0x01))
                    {
                        pResults->revision = 0x10; /* TODO: hardcoded to B0 */
                        BDBG_WRN(("4501 detected with rev %02x", pResults->revision));
                        return BERR_SUCCESS;
                    }
                }
            }
            break;
        }
    }

    return BERR_TRACE(BERR_INVALID_PARAMETER);
}

static void NEXUS_Frontend_P_4501_L1_isr(void *param1, int param2)
{
    BAST_Handle astHandle = (BAST_Handle)param2;
    BSTD_UNUSED(param1);
    BDBG_MSG(("4501 L1 ISR"));
    BAST_HandleInterrupt_isr(astHandle);
}

static void NEXUS_Frontend_P_4501_CloseCallback(NEXUS_FrontendHandle handle, void *pParam)
{
    unsigned i;
    NEXUS_4501Device *pDevice = pParam;

    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);

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

    /* If we got here, the device is ready to be destroyed */
    NEXUS_Frontend_P_4501_DestroyDevice(pDevice);
}

static void NEXUS_Frontend_P_4501_DestroyDevice(NEXUS_4501Device *pDevice)
{
    unsigned i;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);

    for ( i = 0; i < pDevice->numChannels; i++ )
    {
        if ( NULL != pDevice->handles[i] )
        {
            BDBG_ERR(("All channels must be closed before destroying device"));
            BDBG_ASSERT(NULL == pDevice->handles[i]);
        }
    }

    BDBG_MSG(("Destroying 4501 device %p", (void *)pDevice));

    NEXUS_UnregisterEvent(pDevice->isrCallback);
    BLST_S_REMOVE(&g_deviceList, pDevice, NEXUS_4501Device, node);
    NEXUS_Core_DisconnectInterrupt(pDevice->settings.isrNumber);
    for ( i = 0; i < pDevice->numChannels && NULL != pDevice->astChannels[i]; i++)
        BAST_CloseChannel(pDevice->astChannels[i]);
    BAST_Close(pDevice->astHandle);
    BDBG_OBJECT_DESTROY(pDevice, NEXUS_4501Device);
    BKNI_Free(pDevice);
}

/***************************************************************************
Summary:
    Enable/Disable interrupts for a 4501 device
 ***************************************************************************/
static void NEXUS_Frontend_P_4501_IsrControl_isr(bool enable, void *pParam)
{
    NEXUS_4501Device *pDevice = (NEXUS_4501Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);
    if ( enable )
    {
        BDBG_MSG(("Enable 4501 Interrupt %u", pDevice->settings.isrNumber));
        NEXUS_Core_EnableInterrupt_isr(pDevice->settings.isrNumber);
    }
    else
    {
        BDBG_MSG(("Disable 4501 Interrupt %u", pDevice->settings.isrNumber));
        NEXUS_Core_DisableInterrupt_isr(pDevice->settings.isrNumber);
    }
}

static void NEXUS_Frontend_P_4501_IsrCallback(void *pParam)
{
    NEXUS_4501Device *pDevice = (NEXUS_4501Device *)pParam;
    BDBG_OBJECT_ASSERT(pDevice, NEXUS_4501Device);
    BAST_ProcessInterruptEvent(pDevice->astHandle);
}

NEXUS_I2cHandle NEXUS_Frontend_Get4501MasterI2c( NEXUS_FrontendHandle handle )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
        return NULL;
    }
    return astDevice->deviceI2cHandle;
}

NEXUS_Error NEXUS_Frontend_Write4501HostRegister( NEXUS_FrontendHandle handle, uint8_t address, uint8_t data )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4501_WriteHostRegister(astDevice->astHandle, address, &data);
}

NEXUS_Error NEXUS_Frontend_Read4501HostRegister( NEXUS_FrontendHandle handle, uint8_t address, uint8_t *pData )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4501_ReadHostRegister(astDevice->astHandle, address, pData);
}

NEXUS_Error NEXUS_Frontend_Read4501Memory( NEXUS_FrontendHandle handle, uint16_t addr, uint8_t *buffer, uint16_t bufferSize )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    return BAST_4501_P_ReadMemory(astDevice->astHandle, addr, buffer, bufferSize);
}

NEXUS_Error NEXUS_Frontend_Write4501Memory( NEXUS_FrontendHandle handle, uint16_t addr, const uint8_t *buffer, uint16_t bufferSize )
{
    NEXUS_AstDevice *astDevice = NEXUS_Frontend_P_GetAstDevice(handle);
    if (!astDevice) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* wrong frontend */
    }
    if(buffer==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return BAST_4501_P_WriteMemory(astDevice->astHandle, addr, buffer, bufferSize);
}

