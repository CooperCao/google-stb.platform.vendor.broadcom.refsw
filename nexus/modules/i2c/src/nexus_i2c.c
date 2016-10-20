/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* API Description:
*   API name: I2c
*    Specific APIs related to I2c Control.
*
***************************************************************************/

#include "nexus_i2c_module.h"
#include "breg_i2c.h"
#include "bi2c.h"
#include "priv/nexus_core.h"
#include "nexus_base.h"
#include "nexus_platform_features.h"
#include "breg_i2c_priv.h"  /* For BREG_I2C_Impl */
#include "priv/nexus_gpio_priv.h"

BDBG_MODULE(nexus_i2c);

BDBG_OBJECT_ID(magnum_i2c);

typedef struct magnum_i2c {
    BDBG_OBJECT(magnum_i2c)
    BLST_S_ENTRY(magnum_i2c) node; /* g_magnum_i2c */
    unsigned index;
    BI2C_ChannelHandle channel; /* Set only for local masters, NULL otherwise */
    BREG_I2C_Impl asyncReg;     /* A pointer to this will be returned when the module owner requests a handle */
    unsigned refcnt;
    bool s3standby;
    NEXUS_GpioPinData clkPin;
    NEXUS_GpioPinData dataPin;
    NEXUS_I2cSettings settings; /* settings used to open */
} magnum_i2c;

typedef struct magnum_i2c *magnum_i2c_handle;

static BLST_S_HEAD(magnum_i2c_list, magnum_i2c) g_magnum_i2c_channels = BLST_S_INITIALIZER(g_magnum_i2c_channels);

typedef struct NEXUS_I2c
{
    NEXUS_OBJECT(NEXUS_I2c);
    BLST_S_ENTRY(NEXUS_I2c) link; /* g_i2c_channels */
    unsigned index;
    BI2C_ChannelHandle channel; /* copy from g_magnum_i2c_channel[index] */
    NEXUS_ModuleHandle owner;   /* The module that created this handle.  Required for synchronization decisions. */
    BREG_I2C_Impl syncReg;      /* A pointer to this will be returned when someone other than the module owner requests a handle */
    BREG_I2C_Impl asyncReg;     /* copy from g_magnum_i2c_channel[index] */
    NEXUS_I2cSettings settings;
    bool first; /* first one to open can set settings */
} NEXUS_I2c;

static BLST_S_HEAD(nexus_i2c_list, NEXUS_I2c) g_i2c_channels;

static void NEXUS_I2c_P_FillSyncRegHandle(
    NEXUS_I2cHandle i2cHandle,
    NEXUS_ModuleHandle moduleHandle
    );

void NEXUS_I2c_GetDefaultSettings(
    NEXUS_I2cSettings *pSettings    /* [out] */
    )
{
    BI2C_ChannelSettings channelSettings;

    BDBG_ASSERT(NULL != pSettings);

    BI2C_GetChannelDefaultSettings(g_NEXUS_i2cHandle, 0, &channelSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->clockRate = (NEXUS_I2cClockRate)channelSettings.clkRate;
    pSettings->interruptMode = channelSettings.intMode;
    pSettings->timeout = channelSettings.timeoutMs;
    pSettings->burstMode = channelSettings.burstMode;
    pSettings->softI2c = channelSettings.softI2c;
    pSettings->autoI2c.enabled = channelSettings.autoI2c.enabled;
    pSettings->fourByteXferMode = channelSettings.fourByteXferMode;
    pSettings->use5v = channelSettings.inputSwitching5V;
}

static NEXUS_Error nexus_i2c_p_open_channel(NEXUS_I2cHandle i2cHandle)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BI2C_ChannelSettings channelSettings;
    BREG_I2C_Handle regHandle;
    const NEXUS_I2cSettings *pSettings = &i2cHandle->settings;
    NEXUS_I2cSettings *magnumSettings=NULL;
    magnum_i2c_handle magnumI2cHandle=NULL;
    NEXUS_GpioPinData clkPin, dataPin;

    BKNI_Memset(&clkPin, 0, sizeof(clkPin));
    BKNI_Memset(&dataPin, 0, sizeof(dataPin));

    if(pSettings->softI2c){
        if(i2cHandle->settings.clockGpio){
            NEXUS_Gpio_GetPinData_priv(i2cHandle->settings.clockGpio, &clkPin);
            NEXUS_Gpio_Close(i2cHandle->settings.clockGpio);
        }
        else{
            errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            BDBG_ERR(("Gpio handle not passed for the gpio pin to be used as I2C clock bus."));
            goto done;
        }
        if(i2cHandle->settings.dataGpio){
            NEXUS_Gpio_GetPinData_priv(i2cHandle->settings.dataGpio, &dataPin);
            NEXUS_Gpio_Close(i2cHandle->settings.dataGpio);
        }
        else{
            errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            BDBG_ERR(("Gpio handle not passed for the gpio pin to be used as I2C data bus."));
            goto done;
        }
    }

    for ( magnumI2cHandle = BLST_S_FIRST(&g_magnum_i2c_channels); NULL != magnumI2cHandle; magnumI2cHandle = BLST_S_NEXT(magnumI2cHandle, node) )
    {
        BDBG_OBJECT_ASSERT(magnumI2cHandle, magnum_i2c);
        magnumSettings = &magnumI2cHandle->settings;
        if(pSettings->softI2c){
            if((magnumSettings->softI2c) && !BKNI_Memcmp(&magnumI2cHandle->clkPin, &clkPin, sizeof(NEXUS_GpioPinData)) &&
                !BKNI_Memcmp(&magnumI2cHandle->dataPin, &dataPin, sizeof(NEXUS_GpioPinData))){
                break;
            }
        }
        else if(magnumI2cHandle->index == i2cHandle->index) {
            if(!BKNI_Memcmp(magnumSettings, pSettings, sizeof(NEXUS_I2cSettings))){
                break;
            }
            else {
                errCode = BERR_TRACE(NEXUS_INVALID_PARAMETER);
                BDBG_ERR(("Channel %d is opened atleast once before with softI2c = %d and autoI2c = %d settings.",
                            i2cHandle->index, magnumI2cHandle->settings.softI2c, magnumI2cHandle->settings.autoI2c.enabled));
                BDBG_ERR(("More instances of the same channel can only be opened the same softI2c and autoI2c settings."));
                magnumI2cHandle=NULL;
                goto done;
            }
        }
    }

    if( NULL == magnumI2cHandle) {
        magnumI2cHandle = BKNI_Malloc(sizeof(*magnumI2cHandle));
        if (!magnumI2cHandle) {
            errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto done;
        }
        BKNI_Memset(magnumI2cHandle, 0, sizeof(magnum_i2c));
        BDBG_OBJECT_SET(magnumI2cHandle, magnum_i2c);

        BLST_S_INSERT_HEAD(&g_magnum_i2c_channels, magnumI2cHandle, node);

        BDBG_CASSERT(BI2C_Clk_eClk400Khz == NEXUS_I2cClockRate_e400Khz);
        BI2C_GetBscIndexDefaultSettings(g_NEXUS_i2cHandle, i2cHandle->index, magnumI2cHandle->settings.softI2c, &channelSettings);
        channelSettings.clkRate = pSettings->clockRate;
        channelSettings.intMode = pSettings->interruptMode;
        channelSettings.timeoutMs = pSettings->timeout;
        channelSettings.burstMode = pSettings->burstMode;
        channelSettings.autoI2c.enabled = pSettings->autoI2c.enabled;
        channelSettings.softI2c = pSettings->softI2c;
        channelSettings.fourByteXferMode = pSettings->fourByteXferMode;
        channelSettings.inputSwitching5V = pSettings->use5v;
        BDBG_MSG(("Open I2C Channel %d: clk %d, int %d, timeout %d, burst %d, softI2c %d, autoI2c %d, four byte xfer mode %d",
            i2cHandle->index, channelSettings.clkRate, channelSettings.intMode, channelSettings.timeoutMs, channelSettings.burstMode, channelSettings.softI2c,
            channelSettings.autoI2c.enabled, channelSettings.fourByteXferMode));

        if(pSettings->softI2c){
            channelSettings.gpio.scl.address = clkPin.address;
            channelSettings.gpio.scl.shift = clkPin.shift;
            channelSettings.gpio.sda.address =  dataPin.address;
            channelSettings.gpio.sda.shift = dataPin.shift;
        }

        errCode = BI2C_OpenBSCChannel(g_NEXUS_i2cHandle, &magnumI2cHandle->channel, i2cHandle->index, &channelSettings);
        if ( errCode == NEXUS_NOT_AVAILABLE){
            BDBG_WRN(("Channel %d is not available on this platform.", i2cHandle->index));
            goto done;
        }
        else if ( errCode )
        {
            BDBG_ERR(("Unable to open I2C Device Channel"));
            BERR_TRACE(errCode); goto done;
        }

        errCode = BI2C_CreateI2cRegHandle(magnumI2cHandle->channel, &regHandle);
        if ( errCode )
        {
            BDBG_ERR(("Unable to create BREG I2C Handle"));
            BI2C_CloseChannel(magnumI2cHandle->channel);
            BERR_TRACE(errCode); goto done;
        }

        magnumI2cHandle->settings = *pSettings;
        magnumI2cHandle->index = i2cHandle->index;
        if(pSettings->softI2c){
            magnumI2cHandle->clkPin = clkPin;
            magnumI2cHandle->dataPin = dataPin;
        }

        /* The default handle is our async version */
        magnumI2cHandle->asyncReg = *regHandle;
        BI2C_CloseI2cRegHandle(regHandle);
    }

    magnumI2cHandle->refcnt++;

    /* copy handles */
    i2cHandle->channel = magnumI2cHandle->channel;
    i2cHandle->asyncReg = magnumI2cHandle->asyncReg;

    i2cHandle->first = (magnumI2cHandle->refcnt == 1);
done:
    if(errCode){
        if(magnumI2cHandle){
            if (magnumI2cHandle->channel) {
                BI2C_CloseChannel(magnumI2cHandle->channel);
                magnumI2cHandle->channel = NULL;
            }
            BKNI_EnterCriticalSection();
            BLST_S_REMOVE(&g_magnum_i2c_channels, magnumI2cHandle, magnum_i2c, node);
            BKNI_LeaveCriticalSection();
            BDBG_OBJECT_DESTROY(magnumI2cHandle, magnum_i2c);
            BKNI_Free(magnumI2cHandle);
            magnumI2cHandle = NULL;
        }
    }
    return errCode;
}

static void nexus_i2c_p_close_channel(NEXUS_I2cHandle i2cHandle)
{
    magnum_i2c_handle magnumI2cHandle=NULL;

    for ( magnumI2cHandle = BLST_S_FIRST(&g_magnum_i2c_channels); NULL != magnumI2cHandle; magnumI2cHandle = BLST_S_NEXT(magnumI2cHandle, node) )
    {
        BDBG_OBJECT_ASSERT(magnumI2cHandle, magnum_i2c);
        if(magnumI2cHandle->channel == i2cHandle->channel)
        {
            break;
        }
    }

    if(magnumI2cHandle){
        BDBG_ASSERT(magnumI2cHandle->refcnt);
        if (--magnumI2cHandle->refcnt == 0) {
            BI2C_CloseChannel(magnumI2cHandle->channel);
            magnumI2cHandle->channel = NULL;

            BKNI_EnterCriticalSection();
            BLST_S_REMOVE(&g_magnum_i2c_channels, magnumI2cHandle, magnum_i2c, node);
            BKNI_LeaveCriticalSection();
            BDBG_OBJECT_DESTROY(magnumI2cHandle, magnum_i2c);
            BKNI_Free(magnumI2cHandle);
            magnumI2cHandle = NULL;
        }
    }
}

NEXUS_I2cHandle NEXUS_I2c_Open(
    unsigned channelIndex,
    const NEXUS_I2cSettings *pSettings
    )
{
    NEXUS_I2cHandle i2cHandle;
    NEXUS_I2cSettings settings;
    NEXUS_Error errCode = NEXUS_SUCCESS;

    if ( NULL == pSettings )
    {
        NEXUS_I2c_GetDefaultSettings(&settings);
        pSettings = &settings;
    }

    i2cHandle = BKNI_Malloc(sizeof(*i2cHandle));
    if (!i2cHandle) {
        errCode = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_I2c, i2cHandle);
    i2cHandle->settings = *pSettings;
    if(pSettings->softI2c)
        i2cHandle->index = BI2C_SOFT_I2C_CHANNEL_NUMBER;
    else
        i2cHandle->index = channelIndex;

    errCode = nexus_i2c_p_open_channel(i2cHandle);
    if ( errCode == NEXUS_NOT_AVAILABLE){
        goto error;
    }
    else if (errCode) {errCode = BERR_TRACE(errCode); goto error;}
    BLST_S_INSERT_HEAD(&g_i2c_channels, i2cHandle, link);

    NEXUS_I2c_P_FillSyncRegHandle(i2cHandle, NEXUS_MODULE_SELF);

    NEXUS_OBJECT_REGISTER(NEXUS_I2c, i2cHandle, Create);

    return i2cHandle;

error:
    if(i2cHandle)NEXUS_I2c_Close(i2cHandle);
    return NULL;
}

static void NEXUS_I2c_P_Release( NEXUS_I2cHandle i2cHandle )
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_I2c, i2cHandle, Destroy);
}

static void NEXUS_I2c_P_Finalizer( NEXUS_I2cHandle i2cHandle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_I2c, i2cHandle);
    BDBG_MSG(("Closing I2C Channel %p, %u", (void *)i2cHandle, i2cHandle->index));
    if (i2cHandle->channel) {
        nexus_i2c_p_close_channel(i2cHandle);
        BLST_S_REMOVE(&g_i2c_channels, i2cHandle, NEXUS_I2c, link);
    }
    NEXUS_OBJECT_DESTROY(NEXUS_I2c, i2cHandle);
    BKNI_Free(i2cHandle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_I2c, NEXUS_I2c_Close);

void NEXUS_I2c_P_CloseAll(void)
{
    NEXUS_I2cHandle i2cHandle;
    while ((i2cHandle = BLST_S_FIRST(&g_i2c_channels))) {
        BDBG_MSG(("Closing I2C Channel %p", (void *)i2cHandle));
        NEXUS_I2c_Close(i2cHandle);
    }
}

NEXUS_Error NEXUS_I2c_SetSettings( NEXUS_I2cHandle i2cHandle, const NEXUS_I2cSettings *pSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);

    if (!i2cHandle->first) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    BI2C_SetClk(i2cHandle->channel, pSettings->clockRate);

    rc = BI2C_SetBurstMode(i2cHandle->channel, pSettings->burstMode);
    if (rc) return BERR_TRACE(rc);

    i2cHandle->settings = *pSettings;
    return 0;
}

void NEXUS_I2c_GetSettings( NEXUS_I2cHandle i2cHandle, NEXUS_I2cSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    *pSettings = i2cHandle->settings;
}

#define ACQUIRE_REG_HANDLE(i2cHandle) ( i2cHandle->owner == NEXUS_MODULE_SELF )?&i2cHandle->asyncReg:(NEXUS_UnlockModule(),&i2cHandle->syncReg)
#define RELEASE_REG_HANDLE(i2cHandle) do {if ( i2cHandle->owner != NEXUS_MODULE_SELF ) { NEXUS_LockModule(); }} while (0)

NEXUS_Error NEXUS_I2c_Write( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length )
{
    NEXUS_Error errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_Write(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteRead( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength )
{
    NEXUS_Error errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteRead(regHandle, chipAddr, subAddr, pWriteData, writeLength, pReadData, readLength);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteReadNoAddr( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength )
{
    NEXUS_Error errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteReadNoAddr(regHandle, chipAddr, pWriteData, writeLength, pReadData, readLength);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteNoAck( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteNoAck(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteA16( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint16_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteA16(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteA24( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteA24(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteNoAddr( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteNoAddr(regHandle, chipAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteNoAddrNoAck( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteNoAddrNoAck(regHandle, chipAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteNvram( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteNvram(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_Read( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_Read(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadNoAck( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadNoAck(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadA16( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint16_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadA16(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadA24( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadA24(regHandle, chipAddr, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadNoAddr( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadNoAddr(regHandle, chipAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadNoAddrNoAck( NEXUS_I2cHandle i2cHandle, uint16_t chipAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadNoAddrNoAck(regHandle, chipAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_ReadEDDC( NEXUS_I2cHandle i2cHandle, uint8_t chipAddr, uint8_t segment, uint8_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_ReadEDDC(regHandle, chipAddr, segment, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

NEXUS_Error NEXUS_I2c_WriteEDDC( NEXUS_I2cHandle i2cHandle, uint8_t chipAddr, uint8_t segment, uint8_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    BREG_I2C_Handle regHandle;
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    regHandle = ACQUIRE_REG_HANDLE(i2cHandle);
    errCode = BREG_I2C_WriteEDDC(regHandle, chipAddr, segment, subAddr, pData, length);
    RELEASE_REG_HANDLE(i2cHandle);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteSync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_Write(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteSwSync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_WriteSw);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteSw(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_WriteSw);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteNoAckSync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteNoAck(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteA16Sync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteA16(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteA24Sync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteA24(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteNoAddrSync( void *context, uint16_t chipAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteNoAddr(&i2cHandle->asyncReg, chipAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteNoAddrNoAckSync( void *context, uint16_t chipAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteNoAddrNoAck(&i2cHandle->asyncReg, chipAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteNvramSync( void *context, uint16_t chipAddr, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteNvram(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadSync( void *context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_Read(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadSwSync( void *context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_ReadSw);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadSw(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_ReadSw);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadNoAckSync( void *context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadNoAck(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadA16Sync( void *context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadA16(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadA24Sync( void *context, uint16_t chipAddr, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadA24(&i2cHandle->asyncReg, chipAddr, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadNoAddrSync( void *context, uint16_t chipAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadNoAddr(&i2cHandle->asyncReg, chipAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadSwNoAddrSync( void *context, uint16_t chipAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_ReadSw);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadSwNoAddr(&i2cHandle->asyncReg, chipAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_ReadSw);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadEDDCSync( void *context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_ReadEDDCSync);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadEDDC(&i2cHandle->asyncReg, chipAddr, segment, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_ReadEDDCSync);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_ReadSwEDDCSync( void *context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_ReadSwEDDCSync);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_ReadSwEDDC(&i2cHandle->asyncReg, chipAddr, segment, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_ReadSwEDDCSync);
    return errCode;
}

static BERR_Code NEXUS_I2c_P_WriteEDDCSync( void *context, uint8_t chipAddr, uint8_t segment, uint32_t subAddr, const uint8_t *pData, size_t length )
{
    BERR_Code errCode;
    NEXUS_I2cHandle i2cHandle = context;
    BDBG_ENTER(NEXUS_I2c_P_Write);
    NEXUS_Module_Lock(i2cHandle->owner);
    errCode = BREG_I2C_WriteEDDC(&i2cHandle->asyncReg, chipAddr, segment, subAddr, pData, length);
    NEXUS_Module_Unlock(i2cHandle->owner);
    BDBG_LEAVE(NEXUS_I2c_P_Write);
    return errCode;
}

/* Magnum modules call BREG_I2C, Nexus modules call NEXUS_I2C.  Both cases need to synchronize */
static void NEXUS_I2c_P_FillSyncRegHandle(
    NEXUS_I2cHandle i2cHandle,
    NEXUS_ModuleHandle moduleHandle
    )
{
    BDBG_ENTER(NEXUS_I2c_P_FillLocalRegHandle);
    i2cHandle->owner = moduleHandle;
    BKNI_Memset(&i2cHandle->syncReg, 0, sizeof(i2cHandle->syncReg));
    i2cHandle->syncReg.context = i2cHandle;
    i2cHandle->syncReg.BREG_I2C_Write_Func = NEXUS_I2c_P_WriteSync;
    i2cHandle->syncReg.BREG_I2C_WriteSw_Func = NEXUS_I2c_P_WriteSwSync;
    i2cHandle->syncReg.BREG_I2C_WriteNoAck_Func = NEXUS_I2c_P_WriteNoAckSync;
    i2cHandle->syncReg.BREG_I2C_WriteA16_Func = NEXUS_I2c_P_WriteA16Sync;
    i2cHandle->syncReg.BREG_I2C_WriteA24_Func = NEXUS_I2c_P_WriteA24Sync;
    i2cHandle->syncReg.BREG_I2C_WriteNoAddr_Func = NEXUS_I2c_P_WriteNoAddrSync;
    i2cHandle->syncReg.BREG_I2C_WriteNoAddrNoAck_Func = NEXUS_I2c_P_WriteNoAddrNoAckSync;
    i2cHandle->syncReg.BREG_I2C_WriteNvram_Func = NEXUS_I2c_P_WriteNvramSync;
    i2cHandle->syncReg.BREG_I2C_Read_Func = NEXUS_I2c_P_ReadSync;
    i2cHandle->syncReg.BREG_I2C_ReadSw_Func = NEXUS_I2c_P_ReadSwSync;
    i2cHandle->syncReg.BREG_I2C_ReadNoAck_Func = NEXUS_I2c_P_ReadNoAckSync;
    i2cHandle->syncReg.BREG_I2C_ReadA16_Func = NEXUS_I2c_P_ReadA16Sync;
    i2cHandle->syncReg.BREG_I2C_ReadA24_Func = NEXUS_I2c_P_ReadA24Sync;
    i2cHandle->syncReg.BREG_I2C_ReadNoAddr_Func = NEXUS_I2c_P_ReadNoAddrSync;
    i2cHandle->syncReg.BREG_I2C_ReadSwNoAddr_Func = NEXUS_I2c_P_ReadSwNoAddrSync;
    i2cHandle->syncReg.BREG_I2C_ReadEDDC_Func = NEXUS_I2c_P_ReadEDDCSync;
    i2cHandle->syncReg.BREG_I2C_ReadSwEDDC_Func = NEXUS_I2c_P_ReadSwEDDCSync;
    i2cHandle->syncReg.BREG_I2C_WriteEDDC_Func = NEXUS_I2c_P_WriteEDDCSync;
    BDBG_LEAVE(NEXUS_I2c_P_FillLocalRegHandle);
}

/***************************************************************************
Summary:
    This function returns a BREG_I2C_Handle from a NEXUS_I2cHandle.

Description:
    This function returns a BREG_I2C_Handle that will handle internal
    synchronization properly based upon two factors.  The first is the module
    that created the I2C Handle (not all are local I2C Masters), and the
    second is the module that intends to use it.  Supported models are as
    follows:  1) Local I2C Master, any nexus module may use it.  2) Module-
    specific I2C Master (e.g. Frontend), only that module may use it.  If
    platform requires a BREG_I2C_Handle for some reason, it may pass NULL
    for the module handle here.  That is only supported for local i2c masters.

See Also:
    NEXUS_I2c_Close(), NEXUS_I2c_GetDefaultSettings()

****************************************************************************/
BREG_I2C_Handle NEXUS_I2c_GetRegHandle(
    NEXUS_I2cHandle i2cHandle,
    NEXUS_ModuleHandle moduleHandle     /* The module requiring a BREG_I2C Handle, determines synchronization. model */
    )
{
    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);

    /* WARNING: This is a private function, but is called in nexus (mostly within frontend module) without the i2c module lock
            acquired. do not modify state or depend on non-atomic state here. In no cases is the nexus_i2c module's lock acquired
            when using the BREG_I2C_Handle returned by NEXUS_I2c_GetRegHandle */

    if ( moduleHandle == i2cHandle->owner || i2cHandle->owner == NEXUS_MODULE_SELF ){
        /* no additional lock is required, either because BI2C will lock, or because the owner is using its own i2c. */
        return &i2cHandle->asyncReg;
    }
    else
    {
        /* must lock i2cHandle->owner's mutex before making the i2c call */
        return &i2cHandle->syncReg;
    }
}

/***************************************************************************
Summary:
    This function initializes an I2C Callback structure to defaults
See Also:
    NEXUS_I2c_CreateHandle
****************************************************************************/
void NEXUS_I2c_InitCallbacks(
    NEXUS_I2cCallbacks *pCallbacks  /* [out] */
    )
{
    BKNI_Memset(pCallbacks, 0, sizeof(*pCallbacks));
}

/***************************************************************************
Summary:
    Create a NEXUS_I2cHandle for a module from a set of callbacks.

Description:
    This function allows another nexus module to create a new I2C handle
    that can be used with the standard NEXUS_I2c_Xxx routines by the application.
    This is required for several frontend devices.

See Also:
    NEXUS_I2c_DestroyHandle(), NEXUS_I2c_InitCallbacks()
****************************************************************************/
NEXUS_I2cHandle NEXUS_I2c_CreateHandle(
    NEXUS_ModuleHandle moduleHandle,            /* The module requiring a BREG_I2C Handle, determines synchronization model */
    void *pContext,                             /* This is the device context that will be passed to each callback */
    const NEXUS_I2cCallbacks *pI2cCallbacks     /* A list of callbacks supported by this device */
    )
{
    NEXUS_I2cHandle i2cHandle;

    BDBG_ASSERT(NULL != moduleHandle);
    BDBG_ASSERT(NULL != pContext);
    BDBG_ASSERT(NULL != pI2cCallbacks);

    NEXUS_LockModule();

    i2cHandle = BKNI_Malloc(sizeof(NEXUS_I2c));
    if ( NULL == i2cHandle )
    {
        BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    NEXUS_OBJECT_INIT(NEXUS_I2c, i2cHandle);

    i2cHandle->asyncReg.context = pContext;
    i2cHandle->asyncReg.BREG_I2C_Write_Func = pI2cCallbacks->write;
    i2cHandle->asyncReg.BREG_I2C_WriteSw_Func = pI2cCallbacks->writeSw;
    i2cHandle->asyncReg.BREG_I2C_WriteNoAck_Func = pI2cCallbacks->writeNoAck;
    i2cHandle->asyncReg.BREG_I2C_WriteA16_Func = pI2cCallbacks->writeA16;
    i2cHandle->asyncReg.BREG_I2C_WriteA24_Func = pI2cCallbacks->writeA24;
    i2cHandle->asyncReg.BREG_I2C_WriteNoAddr_Func = pI2cCallbacks->writeNoAddr;
    i2cHandle->asyncReg.BREG_I2C_WriteNoAddrNoAck_Func = pI2cCallbacks->writeNoAddrNoAck;
    i2cHandle->asyncReg.BREG_I2C_WriteNvram_Func = pI2cCallbacks->writeNvram;
    i2cHandle->asyncReg.BREG_I2C_Read_Func = pI2cCallbacks->read;
    i2cHandle->asyncReg.BREG_I2C_ReadSw_Func = pI2cCallbacks->readSw;
    i2cHandle->asyncReg.BREG_I2C_ReadNoAck_Func = pI2cCallbacks->readNoAck;
    i2cHandle->asyncReg.BREG_I2C_ReadA16_Func = pI2cCallbacks->readA16;
    i2cHandle->asyncReg.BREG_I2C_ReadA24_Func = pI2cCallbacks->readA24;
    i2cHandle->asyncReg.BREG_I2C_ReadNoAddr_Func = pI2cCallbacks->readNoAddr;
    i2cHandle->asyncReg.BREG_I2C_ReadSwNoAddr_Func = pI2cCallbacks->readSwNoAddr;
    i2cHandle->asyncReg.BREG_I2C_ReadEDDC_Func = pI2cCallbacks->readEDDC;
    i2cHandle->asyncReg.BREG_I2C_ReadSwEDDC_Func = pI2cCallbacks->readSwEDDC;
    i2cHandle->asyncReg.BREG_I2C_WriteEDDC_Func = pI2cCallbacks->writeEDDC;

    NEXUS_I2c_P_FillSyncRegHandle(i2cHandle, moduleHandle);

    /* Success */
    NEXUS_OBJECT_REGISTER(NEXUS_I2c, i2cHandle, Create);

done:
    NEXUS_UnlockModule();
    return i2cHandle;
}

/***************************************************************************
Summary:
    Destroy a handle created with NEXUS_I2c_CreateHandle.

See Also:
    NEXUS_I2c_CreateHandle()
****************************************************************************/
void NEXUS_I2c_DestroyHandle(
    NEXUS_I2cHandle i2cHandle
    )
{
    NEXUS_LockModule();
    NEXUS_I2c_Close(i2cHandle);
    NEXUS_UnlockModule();
}

NEXUS_Error NEXUS_I2c_SwReset( NEXUS_I2cHandle i2cHandle )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(i2cHandle, NEXUS_I2c);
    if (!i2cHandle->first) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    rc = BI2C_SwReset(i2cHandle->channel);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

NEXUS_Error NEXUS_I2cModule_Standby_priv(bool enabled, const NEXUS_StandbySettings *pSettings)
{
#if NEXUS_POWER_MANAGEMENT
    BERR_Code rc;
    magnum_i2c_handle magnumI2cHandle=NULL;
    BI2C_ChannelSettings channelSettings;

    if (enabled) {
        if (pSettings->mode==NEXUS_StandbyMode_eDeepSleep || pSettings->mode==NEXUS_StandbyMode_ePassive) { /* S3 or S2*/

            /* some I2C buses are part of the AON block and some are not. for the AON ones,
               the BI2C_Channel should not be closed/re-opened. however, nothing is currently
               using I2C during S3 standby, so we can treat them all the same for now */
            for ( magnumI2cHandle = BLST_S_FIRST(&g_magnum_i2c_channels); NULL != magnumI2cHandle; magnumI2cHandle = BLST_S_NEXT(magnumI2cHandle, node) )
            {
                BDBG_OBJECT_ASSERT(magnumI2cHandle, magnum_i2c);
                if (!magnumI2cHandle->channel) { continue; }
                BI2C_CloseChannel(magnumI2cHandle->channel);
                magnumI2cHandle->channel = NULL;
                magnumI2cHandle->s3standby = true; /* we can't rely on magnum_i2c->channel being NULL and must keep track of which channels we close, because a platform may not open all i2c channels to begin with */
            }
        }
    }
    else {
        for ( magnumI2cHandle = BLST_S_FIRST(&g_magnum_i2c_channels); NULL != magnumI2cHandle; magnumI2cHandle = BLST_S_NEXT(magnumI2cHandle, node) )
        {
            NEXUS_I2cHandle i2cHandle;
            NEXUS_I2cSettings *pSettings;

            BDBG_OBJECT_ASSERT(magnumI2cHandle, magnum_i2c);
            if (magnumI2cHandle->channel || !magnumI2cHandle->s3standby) { continue; }

            for (i2cHandle = BLST_S_FIRST(&g_i2c_channels); i2cHandle; i2cHandle = BLST_S_NEXT(i2cHandle, link)) {
                if (i2cHandle->index == magnumI2cHandle->index) {
                    break;
                }
            }
            if (i2cHandle == NULL) {
                BDBG_ERR(("Could find nexus i2c handle for index %d", magnumI2cHandle->index));
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
            pSettings = &i2cHandle->settings;

            BI2C_GetBscIndexDefaultSettings(g_NEXUS_i2cHandle, i2cHandle->index, magnumI2cHandle->settings.softI2c, &channelSettings);
            channelSettings.clkRate = pSettings->clockRate;
            channelSettings.intMode = pSettings->interruptMode;
            channelSettings.timeoutMs = pSettings->timeout;
            channelSettings.burstMode = pSettings->burstMode;
            channelSettings.autoI2c.enabled = pSettings->autoI2c.enabled;
            channelSettings.softI2c = pSettings->softI2c;
            channelSettings.fourByteXferMode = pSettings->fourByteXferMode;
            channelSettings.inputSwitching5V = pSettings->use5v;
            BDBG_MSG(("Open I2C Channel %d: clk %d, int %d, timeout %d, burst %d, softI2c %d, autoI2c %d, four byte xfer mode %d",
            i2cHandle->index, channelSettings.clkRate, channelSettings.intMode, channelSettings.timeoutMs, channelSettings.burstMode, channelSettings.softI2c,
            channelSettings.autoI2c.enabled, channelSettings.fourByteXferMode));

            /* BI2C handle can be closed/re-opened, but the BREG_I2C handle must NOT be destroyed/re-created,
               because other magnum modules (e.g. HDM) will keep using that handle */
            rc = BI2C_OpenBSCChannel(g_NEXUS_i2cHandle, &magnumI2cHandle->channel, magnumI2cHandle->index, &channelSettings);
            if (rc) { return BERR_TRACE(rc); }

            /* asyncReg.context is normally set to the BI2C_ChannelHandle via BI2C_CreateI2cRegHandle and must be updated
               syncReg.context is always set to NEXUS_magnum_i2c, so should not be touched */
            magnumI2cHandle->asyncReg.context = magnumI2cHandle->channel;
            magnumI2cHandle->settings = *pSettings;
            magnumI2cHandle->s3standby = false;

            i2cHandle->channel = magnumI2cHandle->channel;
            i2cHandle->asyncReg = magnumI2cHandle->asyncReg;

        }
    }

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(enabled);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
#endif
}
