/***************************************************************************
 * Copyright (C) 2005-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/

#include "bstd.h"
#include "bi2c.h"
#include "bhab.h"
#include "bhab_priv.h"

BDBG_MODULE(BHAB);

/******************************************************************************
 BHAB_Open()
******************************************************************************/
BERR_Code BHAB_Open(
   BHAB_Handle *handle,     /* [out] BHAB handle */
   void        *pReg,       /* [in] pointer ot i2c or spi handle */
   const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{

   BDBG_ASSERT(handle);
   BDBG_ASSERT(pReg);
   BDBG_ASSERT(pDefSettings);

   return (pDefSettings->api.Open(handle, pReg, pDefSettings));
}


/******************************************************************************
 BHAB_Close()
******************************************************************************/
BERR_Code BHAB_Close(BHAB_Handle handle)
{

   BDBG_ASSERT(handle);
   return (handle->settings.api.Close(handle));
}


/******************************************************************************
 BHAB_3520_P_InitAp()
******************************************************************************/
BERR_Code BHAB_InitAp(
   BHAB_Handle handle,       /* [in] BHAB handle */
   const uint8_t *pHexImage  /* [in] pointer to BCM3520 microcode image */
)
{
   BDBG_ASSERT(handle);
   return (handle->settings.api.InitAp(handle, pHexImage));
}


/******************************************************************************
 BHAB_GetApStatus()
******************************************************************************/
BERR_Code BHAB_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetApStatus)
      return (handle->settings.api.GetApStatus(handle, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_GetApVersion()
******************************************************************************/
BERR_Code BHAB_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    *pFamilyId, /* [out] Chip Family id */
    uint32_t    *pChipId,   /* [out] BHAB chip ID */
    uint16_t    *pChipVer,  /* [out] chip revision number */
    uint8_t     *pMajApVer, /* [out] AP microcode major version */
    uint8_t     *pMinApVer  /* [out] AP microcode minor version */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetApVersion)
      return (handle->settings.api.GetApVersion(handle, pFamilyId, pChipId, pChipVer, pMajApVer, pMinApVer));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
 BHAB_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetVersionInfo)
      return (handle->settings.api.GetVersionInfo(handle, pVersionInfo));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
 BHAB_ReadRegister()
******************************************************************************/
BERR_Code BHAB_ReadRegister(
   BHAB_Handle handle,     /* [in] BHAB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.ReadRegister)
      return (handle->settings.api.ReadRegister(handle, reg, val));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_WriteRegister()
******************************************************************************/
BERR_Code BHAB_WriteRegister(
   BHAB_Handle handle,     /* [in] BHAB handle */
   uint32_t    reg,   /* [in] address of register to read */
   uint32_t    *val   /* [in] contains data that was read */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.WriteRegister)
      return (handle->settings.api.WriteRegister(handle, reg, val));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_ReadMemory()
******************************************************************************/
BERR_Code BHAB_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.ReadMemory)
      return (handle->settings.api.ReadMemory(handle, addr, buf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_WriteMemory()
******************************************************************************/
BERR_Code BHAB_WriteMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.WriteMemory)
      return (handle->settings.api.WriteMemory(handle, addr, buf, n));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_ReadMbox()
******************************************************************************/
BERR_Code BHAB_ReadMbox(
   BHAB_Handle handle,    /* [in] BHAB PI Handle */
   uint32_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [out] value read from register */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.ReadMbox)
      return (handle->settings.api.ReadMbox(handle, reg, val));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_WriteMbox()
******************************************************************************/
BERR_Code BHAB_WriteMbox(
   BHAB_Handle handle,    /* [in] BHAB PI Handle */
   uint32_t    reg,  /* [in] RBUS register address */
   uint32_t    *val  /* [in] value to write */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.WriteMbox)
      return (handle->settings.api.WriteMbox(handle, reg, val));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_HandleInterrupt_isr(
   BHAB_Handle handle /* [in] BHAB handle */
)
{
   BDBG_ASSERT(handle);
   BKNI_ASSERT_ISR_CONTEXT();
   return (handle->settings.api.HandleInterrupt_isr(handle));
}


/******************************************************************************
 BHAB_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_ProcessInterruptEvent(
   BHAB_Handle handle  /* [in] BHAB handle */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.ProcessInterruptEvent)
      return (handle->settings.api.ProcessInterruptEvent(handle));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_EnableLockInterrupt()
******************************************************************************/
BERR_Code BHAB_EnableLockInterrupt(
   BHAB_Handle handle,  /* [in] BHAB handle */
   BHAB_DevId eDevId,    /* [in] Device ID */
   bool bEnable   /* [in] true = enable Lock  interrupt */
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.EnableLockInterrupt)
      return (handle->settings.api.EnableLockInterrupt(handle, eDevId, bEnable));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BHAB_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_InstallInterruptCallback(
   BHAB_Handle handle,  /* [in] BHAB handle */
   BHAB_DevId eDevId,    /* [in] Device ID */
   BHAB_IntCallbackFunc fCallBack_isr,
   void * pParm1,
   int parm2
)
{
   BDBG_ASSERT(handle);
   return (handle->settings.api.InstallInterruptCallback(handle, eDevId, fCallBack_isr, pParm1, parm2));
}

/******************************************************************************
 BHAB_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_UnInstallInterruptCallback(
   BHAB_Handle handle,  /* [in] BHAB handle */
   BHAB_DevId eDevId    /* [in] Device ID */
)
{
   BDBG_ASSERT(handle);
   return (handle->settings.api.UnInstallInterruptCallback(handle, eDevId));
}


/******************************************************************************
 BHAB_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_SendHabCommand(
   BHAB_Handle handle, /* [in] BHAB PI Handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send */
   uint16_t write_len,  /* [in] number of bytes of the HAB command to be written using either I2c or SPI. */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */
   uint16_t read_len,   /* [in] number of bytes to read from the HAB */
   bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
   bool bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
   uint16_t command_len /* [in] Total number of bytes in the HAB command so that the termination byte is inserted at the right HAB location. */
)
{
   BDBG_ASSERT(handle);
   return (handle->settings.api.SendHabCommand(handle, write_buf, write_len, read_buf, read_len, bCheckForAck, bInsertTermination, command_len));
}
/******************************************************************************
 BHAB_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_GetInterruptEventHandle(
   BHAB_Handle handle,     /* [out] BHAB handle */
   BKNI_EventHandle *event
)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetInterruptEventHandle)
      return (handle->settings.api.GetInterruptEventHandle(handle, event));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   Gets the watch dog timer configuration.

Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetWatchDogTimer(BHAB_Handle handle,      /* [in] BHAB PI Handle */
   BHAB_WatchDogTimerSettings *wdtSettings)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetWatchDogTimer)
      return (handle->settings.api.GetWatchDogTimer(handle, wdtSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   Sets the watch dog timer configuration.

Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_SetWatchDogTimer(BHAB_Handle handle,      /* [in] BHAB PI Handle */
   const BHAB_WatchDogTimerSettings *wdtSettings)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.SetWatchDogTimer)
      return (handle->settings.api.SetWatchDogTimer(handle, wdtSettings));
   else
      return BERR_NOT_SUPPORTED;
}


/******************************************************************************
Summary:
   Gets the 31xx non maskable interrupt configuration.

Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetNmiConfig(BHAB_Handle handle,      /* [in] BHAB PI Handle */
   BHAB_NmiSettings *nmiSettings)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetNmiConfig)
      return (handle->settings.api.GetNmiConfig(handle, nmiSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   Sets the 31xx non maskable interrupt configuration.

Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_SetNmiConfig(BHAB_Handle handle,      /* [in] BHAB PI Handle */
   const BHAB_NmiSettings *nmiSettings)
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.SetNmiConfig)
      return (handle->settings.api.SetNmiConfig(handle, nmiSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    This function gets HAB settings.

Description:
    This function returns the current HAB Settings.

Returns:
    BERR_Code

****************************************************************************/
BERR_Code BHAB_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetConfigSettings)
      return (handle->settings.api.GetConfigSettings(handle, settings));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    This function sets HAB settings.

Description:
    This function sets the current HAB Settings.

Returns:
    BERR_Code

****************************************************************************/
BERR_Code BHAB_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.SetConfigSettings)
      return (handle->settings.api.SetConfigSettings(handle, settings));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function allows the reading from a slave device attached to the demod chip.
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_ReadSlave(
    BHAB_Handle handle,     /* [in] Device channel handle */
   uint8_t chipAddr,       /* [in] chip addr of the i2c slave device */
   uint32_t subAddr,       /* [in] sub addr of the register to read from the slave device */
   uint32_t *data,         /* [out] ptr to the data that we will read from the slave device */
   uint8_t dataLen)        /* [in] how many bytes are we going to read? one to four, For 3412 LNA, this should be in multiples of 4-bytes*/
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.ReadSlave)
      return (handle->settings.api.ReadSlave(handle, chipAddr, subAddr, data, dataLen));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function allows the writing to a slave device attached to the demod chip.
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_WriteSlave(
    BHAB_Handle handle,     /* [in] Device channel handle */
   uint8_t chipAddr,       /* [in] chip addr of the i2c slave device */
   uint32_t subAddr,       /* [in] sub addr of the register to read from the slave device */
   uint32_t *data,         /* [out] ptr to the data that we will read from the slave device */
   uint8_t dataLen)        /* [in] how many bytes are we going to read? one to four, For 3412 LNA, this should be in multiples of 4-bytes*/
{
   BDBG_ASSERT(handle);

   if (handle->settings.api.WriteSlave)
      return (handle->settings.api.WriteSlave(handle, chipAddr, subAddr, data, dataLen));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function retrieves the internal gain settings of a demod chip
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings)    /* [out] internal gain settings. */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetInternalGain)
      return (handle->settings.api.GetInternalGain(handle, inputParams, internalGainSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function sets the external gain settings to a demod chip
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetExternalGain(
    BHAB_Handle handle,                                   /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings)    /* [in] external gain settings. */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetExternalGain)
      return (handle->settings.api.GetExternalGain(handle, externalGainSettings));
   else
      return BERR_NOT_SUPPORTED;
}
/******************************************************************************
Summary:
   This function retrieves the external gain settings of a demod chip
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings)  /* [in] external gain settings. */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.SetExternalGain)
      return (handle->settings.api.SetExternalGain(handle, externalGainSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function retrieves the total tuner channels present in the frontend device.

Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels    /* [out] Returns total tuner channels */
    )
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetTunerChannels)
      return (handle->settings.api.GetTunerChannels(handle, numChannels));
   else
      return BERR_NOT_SUPPORTED;
}

/******************************************************************************
Summary:
   This function retrieves the capabilities of the frontend chip
Description:

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities)   /* [out] Returns chip capabilities */
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetCapabilities)
      return (handle->settings.api.GetCapabilities(handle, pCapabilities));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    Return status data for AVS.

Description:
    This can be used to get the current voltage and temperature of the frontend
    chip. This API is only valid for stand-alone frontend chips.

Returns:
   BERR_Code
**************************************************************************/
BERR_Code BHAB_GetAvsData(
    BHAB_Handle handle,         /* [in] Device handle */
    BHAB_AvsData *pAvsData)     /* [out] pointer to AVS Data */
{
   BDBG_ASSERT(handle);

   if (handle->settings.api.GetAvsData)
      return (handle->settings.api.GetAvsData(handle, pAvsData));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    Resets Ap.

Description:
    This can be used to get put the AP/Leap in reset state.

Returns:
   BERR_Code
**************************************************************************/
BERR_Code BHAB_Reset(
    BHAB_Handle handle         /* [in] Device handle */
    )
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.Reset)
      return (handle->settings.api.Reset(handle));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    Get Recalibrate Settings.

Description:
    This can be used to get Recalibrate Settings.

Returns:
   BERR_Code
**************************************************************************/
BERR_Code BHAB_GetRecalibrateSettings(
    BHAB_Handle handle,
    BHAB_RecalibrateSettings *pSettings
    )
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.GetRecalibrateSettings)
      return (handle->settings.api.GetRecalibrateSettings(handle, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    Get Recalibrate Settings.

Description:
    This can be used to set Recalibrate Settings.If performance degrades,
    the frontend device may need to be recalibrated for optimal performance.
    Internally, this may include triggering the CPPM(composite plant power
    measurement).

Returns:
   BERR_Code
**************************************************************************/
BERR_Code BHAB_SetRecalibrateSettings (
    BHAB_Handle handle,
    const BHAB_RecalibrateSettings *pSettings
    )
{
   BDBG_ASSERT(handle);
   if (handle->settings.api.SetRecalibrateSettings)
      return (handle->settings.api.SetRecalibrateSettings(handle, pSettings));
   else
      return BERR_NOT_SUPPORTED;
}

/***************************************************************************
Summary:
    Get LNA Status

Description:
    This function returns the status of the LNA controlled by the Fronend
    chip

Returns:
   BERR_Code
**************************************************************************/
BERR_Code BHAB_GetLnaStatus(
    BHAB_Handle handle,                     /* [in] Device handle */
    BHAB_LnaStatus *pStatus
    )
{
   BDBG_ASSERT(handle);

   if (handle->settings.api.GetLnaStatus)
      return (handle->settings.api.GetLnaStatus(handle, pStatus));
   else
      return BERR_NOT_SUPPORTED;
}
