/***************************************************************************
 *     Copyright (c) 2006-2009, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: Audio Decoder Interface
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bape.h"
#include "bape_priv.h"
#include "bchp_hifidac_ctrl0.h"

BDBG_OBJECT_ID(BAPE_RfMod);
BDBG_MODULE(bape_rfmod);

typedef struct BAPE_RfMod
{
    BDBG_OBJECT(BAPE_RfMod)
    unsigned index;
    BAPE_Handle deviceHandle;
    BAPE_RfModSettings settings;
} BAPE_RfMod;

void BAPE_RfMod_GetDefaultSettings(
    BAPE_RfModSettings *pSettings
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BAPE_RfMod_Open(
    BAPE_Handle deviceHandle,
    unsigned index,
    const BAPE_RfModSettings *pSettings,
    BAPE_RfModHandle *pHandle             /* [out] */
    )
{
    BAPE_RfModSettings defaultSettings;
    BAPE_RfModHandle handle;
    BERR_Code errCode;

    BDBG_OBJECT_ASSERT(deviceHandle, BAPE_Device);
    BDBG_ASSERT(NULL != pHandle);

    if ( index >= BAPE_CHIP_MAX_RFMODS )
    {
        BDBG_ERR(("Request to open RFMOD %d but chip only has %u RFMODs", index, BAPE_CHIP_MAX_RFMODS));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if ( deviceHandle->rfmods[index] )
    {
        BDBG_ERR(("RFMOD %u already open", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    handle = BKNI_Malloc(sizeof(BAPE_RfMod));
    if ( NULL == handle )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(handle, 0, sizeof(BAPE_RfMod));
    BDBG_OBJECT_SET(handle, BAPE_RfMod);
    handle->index = index;
    handle->deviceHandle = deviceHandle;

    if ( NULL == pSettings )
    {
        BAPE_RfMod_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    errCode = BAPE_RfMod_SetSettings(handle, pSettings);
    if ( errCode )
    {
        BDBG_OBJECT_DESTROY(handle, BAPE_RfMod);
        BKNI_Free(handle);
        return BERR_TRACE(errCode);
    }

    deviceHandle->rfmods[index] = handle;
    *pHandle = handle;
    return BERR_SUCCESS;
}

void BAPE_RfMod_Close(
    BAPE_RfModHandle handle
    )
{
    BAPE_RfModSettings defaults;

    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);

    handle->deviceHandle->rfmods[handle->index] = NULL;
    BAPE_RfMod_GetDefaultSettings(&defaults);
    BAPE_RfMod_SetSettings(handle, &defaults);
    BDBG_OBJECT_DESTROY(handle, BAPE_RfMod);
    BKNI_Free(handle);
}

void BAPE_RfMod_GetSettings(
    BAPE_RfModHandle handle,
    BAPE_RfModSettings *pSettings     /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);
    BDBG_ASSERT(NULL != pSettings);
    *pSettings = handle->settings;
}

BERR_Code BAPE_RfMod_SetSettings(
    BAPE_RfModHandle handle,
    const BAPE_RfModSettings *pSettings
    )
{
    uint32_t regVal, regAddr;

    BDBG_OBJECT_ASSERT(handle, BAPE_RfMod);
    BDBG_ASSERT(NULL != pSettings);
    
    #if BAPE_CHIP_MAX_RFMODS != 1 || BAPE_CHIP_MAX_DACS != 1
    #error Need to handle cases of multiple RFMs or multiple DACs
    #else
    regAddr = BCHP_HIFIDAC_CTRL0_CONFIG;
    regVal = BREG_Read32(handle->deviceHandle->regHandle, regAddr);
    regVal &= ~BCHP_MASK(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE);
    if ( pSettings->master && !pSettings->muted )
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE, Normal_operation);
    }
    else
    {
        regVal |= BCHP_FIELD_ENUM(HIFIDAC_CTRL0_CONFIG, RFMOD_MUTE, Mute);
    }
    BREG_Write32(handle->deviceHandle->regHandle, regAddr, regVal);
    #endif

    handle->settings = *pSettings;
    return BERR_SUCCESS;
}

