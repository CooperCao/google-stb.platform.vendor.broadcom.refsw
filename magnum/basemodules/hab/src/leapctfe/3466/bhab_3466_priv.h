/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ******************************************************************************/
#ifndef BHAB_LEAP_PRIV_H
#define BHAB_LEAP_PRIV_H

#include "bstd.h"
#include "bi2c.h"
#include "bspi.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bhab_priv.h"
#include "bchp_3466_leap_ctrl.h"
#include "bchp_3466_leap_ctrl_misc.h"
#include "bchp_3466_leap_hab_mem.h"
#include "bchp_3466_leap_l1.h"
#include "bchp_3466_leap_l2_0.h"
#include "bchp_3466_leap_l2_1.h"
#include "bchp_3466_leap_prog0_mem.h"
#include "bhab_3466.h"

/* HAB mesg header */
#define HAB_MSG_HDR(OPCODE,N,CORE_TYPE,CORE_ID) \
    { ((uint8_t)(((uint16_t)(OPCODE)) >> 2)), \
    (((uint8_t)(0x03 & (OPCODE)) << 6) | ((uint8_t)((N)>>4))), \
    ((((uint8_t)(((N)& 0x0F) << 4))) | ((uint8_t)(0x0F & (CORE_TYPE)))), \
    ((uint8_t)(CORE_ID)) }

#define BHAB_COMMAND_BUF_SIZE   80

typedef enum BHAB_LockStatus
{
    BBHAB_LockStatus_eLocked=1,
    BBHAB_LockStatus_eUnlocked=2,
    BBHAB_LockStatus_eNoSignal=3,
    BBHAB_LockStatus_eLast
} BHAB_LockStatus;

typedef enum BHAB_EventId
{
    BHAB_EventId_eHabDone = 3,
    BHAB_EventId_eHabReady = 4,
    BHAB_EventId_eLockChange = 64,
    BHAB_EventId_eStatusReady = 65,
    BHAB_EventId_eSpectrumAnalyzerDataReady = 66,
    BHAB_EventId_eEmergencyWarningSystem = 69,
    BHAB_EventId_eIfDacAcquireComplete = 70,
    BHAB_EventId_eIfDacStatusReady = 71,
    BHAB_EventId_eCppm = 72,
    BHAB_EventId_eLast
} BHAB_EventId;

typedef enum BHAB_CoreType
{
    BHAB_CoreType_eGlobal,
    BHAB_CoreType_eADS,
    BHAB_CoreType_eOOB,
    BHAB_CoreType_eReserved,
    BHAB_CoreType_eDvbt,
    BHAB_CoreType_eIsdbt,
    BHAB_CoreType_eDvbt2,
    BHAB_CoreType_eDvbc2,
    BHAB_CoreType_eVsb,
    BHAB_CoreType_eWFE = 14,
    BHAB_CoreType_eTNR = 15,
    BHAB_CoreType_eLast
} BHAB_CoreType;
#define BHAB_HAB_DONE               0x03000040
#define BHAB_HAB_RESET              0x04000000

#define BHAB_AP_INIT_DONE           0x80000000
#define BHAB_AP_ERROR               0x40000000
#define BHAB_AP_EVENT               0x20000000
#define BHAB_CORE_TYPE_MASK         0x00F80000
#define BHAB_CORE_ID_MASK           0x0007F800
#define BHAB_EVENT_DATA_MASK        0x000007C0
#define BHAB_CPPM_PWR_LVL_CHANGE    1
#define BHAB_CALIBRATION_COMPLETE   2
#define BLNA_CORE_ID                0x0
#define BLNA_WRITE_REGISTER         0x2A
#define BLNA_READ_REGISTER          0xAA
#define CORE_TYPE_GLOBAL            0x0
#define BHAB_TNR_SPECTRUM_DATA_RDY  0x80000000
#define BHAB_WINDOW_IRAM            0x00000000
#define BHAB_WINDOW_IROM            0x00008000
#define BHAB_WINDOW_HAB             0x00010000
#define BHAB_WINDOW_IDATA           0x00010400
#define BHAB_IRAM_SIZE              0xffff
#define BHAB_IROM_SIZE              0x8000
#define BHAB_MEM_SIZE               1024
#define BHAB_HAB_SIZE               1024
#define BCHP_LEAP_HAB_MEM_WORD_TWO  BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + 4
#define BHAB_CPU_RUNNIG             0x40
#define BHAB_INIT_RETRIES           0x14
#define BHAB_CSR_STATUS_ERROR_MASK  0xF
#define BHAB_CORE_TYPE              0xF
#define BHAB_GLOBAL_CORE_TYPE       0x0
#define BHAB_CORE_ID                0x0
#define BHAB_GETVERSION             0xB9
#define BHAB_GET_SYS_VERSION        0xBB
#define BHAB_READ_DAISY             0xC0
#define BHAB_WRITE_DAISY            0x20
#define BHAB_LOOP_THROUGH_WRITE     0x21
#define BHAB_LOOP_THROUGH_READ      0xC1
#define BHAB_TNR_APPLICATION_WRITE  0x11
#define BHAB_TNR_APPLICATION_READ   0x91
#define BHAB_RF_INPUT_MODE_WRITE    0x23
#define BHAB_RF_INPUT_MODE_READ     0xC3
#define BHAB_GET_CAPABILITIES       0X9B
#define BHAB_INTERNAL_GAIN_READ     0xC6
#define BHAB_EXTERNAL_GAIN_READ     0xC5
#define BHAB_EXTERNAL_GAIN_WRITE    0x25
#define BTNR_CORE_ID                0x0
#define BHAB_VSB_STATUS_RDY         0x1
#define BHAB_GET_TUNER_STATUS       0x96
#define BHAB_REQUEST_TUNER_STATUS   0x16
#define BHAB_SET_CPPM_SETTINGS      0x27
#define BHAB_AVS_DATA               0x273
#define BHAB_SET_STANDYBY_MODE      0x38
#define BTNR_CORE_TYPE              0xF
#define BTNR_CORE_ID                0x0

#define BHAB_P_ACQUIRE_MUTEX(handle) BKNI_AcquireMutex(((BHAB_3466_P_Handle *)(handle->pImpl))->hMutex)
#define BHAB_P_RELEASE_MUTEX(handle) BKNI_ReleaseMutex(((BHAB_3466_P_Handle *)(handle->pImpl))->hMutex)
#define BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle) BKNI_AcquireMutex(((BHAB_3466_P_Handle *)(handle->pImpl))->hRegAccessMutex)
#define BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle) BKNI_ReleaseMutex(((BHAB_3466_P_Handle *)(handle->pImpl))->hRegAccessMutex)

#define BCHP_CSR_CONFIG_READ_RBUS_READ              0x01
#define BCHP_CSR_CONFIG_SPECULATIVE_READ_EN_BITS    1
#define BCHP_CSR_STATUS_ERROR_BITS                  1
#define BCHP_CSR_CONFIG_READ_RBUS_WRITE             0x00
#define  DEMOD_XPT_WAKEUP_STATUS                    0x4401004
#define  DEMOD_XPT_WAKEUP_INTR_STATUS_REG           0x4401008

#define BHAB_ERROR_RECOVERY_TIMEOUT 1500
#define BHAB_INITAP_TIMEOUT         150
#define BHAB_HAB_TIMEOUT            10100
#define BHAB_HAB_UART_ENABLE        0x80000001
#define FW_DATA_OFFSET              312
#define FW_SIZE_OFFSET              72
#define FW_INST_ADDR_OFFSET         64

typedef struct BHAB_P_CallbackInfo
{
    BHAB_IntCallbackFunc func;
    void * pParm1;
    int parm2 ;
    int callbackType ;
} BHAB_P_CallbackInfo;


typedef struct BHAB_3466_P_Handle
{
#if BHAB_CHIP==7364
    BHAB_7364_Settings settings;
#elif BHAB_CHIP==75525
    BHAB_75525_Settings settings;
#elif BHAB_CHIP==3466
    BHAB_3466_Settings settings;
    BHAB_3466_StandbySettings standbySettings;
#endif
    BREG_I2C_Handle  hI2cRegister;  /* handle to the master I2C device used to access registers */
    BREG_SPI_Handle  hSpiRegister;  /* handle to the master SPI device used to access registers */
    BREG_Handle      hRegHandle;    /* register handle */
    BKNI_EventHandle hInterruptEvent;   /* interrupt event handle */
    BKNI_EventHandle hApiEvent;         /* API event handle */
    BKNI_EventHandle hInitDoneEvent;    /* AP initialization done event handle */
    BKNI_EventHandle hHabDoneEvent;     /* HAB done event handle */
    BKNI_EventHandle hHabReady;     /* HAB Ready event handle */
    BKNI_MutexHandle hMutex;            /* Mutex handle for serialization */
    BKNI_MutexHandle hRegAccessMutex;            /* Mutex handle for serialization */
    BHAB_P_CallbackInfo InterruptCallbackInfo[BHAB_DevId_eMax];
    BHAB_IntCallbackFunc powerLevelChangeCallback;
    BHAB_IntCallbackFunc calibrationCompleteCallback;
    BHAB_WatchDogTimerSettings wdtSettings;
    BHAB_NmiSettings nmiSettings;
    bool loadAP;                          /* Load(ed) Acquisition Processor microcode when starting. */
    bool isSpi;
    BHAB_TunerApplication tnrApplication;
    BHAB_RfInputMode rfInputMode;
    BHAB_RfDaisyChain daisyChain;   /* daisy chain output*/
    BHAB_RecalibrateSettings recalibrateSettings;
    uint8_t inBuf[BHAB_COMMAND_BUF_SIZE];
} BHAB_3466_P_Handle;



/******************************************************************************
 Summary:
   I2c Read for 3466.

Description:
   I2c Read for 3466.

Returns:
   BERR_Code
******************************************************************************/
BERR_Code BREG_I2C_P_Read3466(
    BREG_I2C_Handle i2cHandle,    /* [in] BREG_I2C Handle */
    uint16_t chipAddr,
    uint8_t subAddr,
    uint8_t *pData,
    size_t length
);


/******************************************************************************
Summary:
   Enables/Disables the L1 host interrupt.
Description:
   Enables/Disables the L1 host interrupt by calling the application-supplied
   callback routine the the BHAB settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_EnableHostInterrupt(
    BHAB_Handle handle, /* [in] BHAB PI handle */
    bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
);


/******************************************************************************
Summary:
   Disables all host interrupts.
Description:
   This function clears the host interrupt enable bits on the BCM3466.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_DisableInterrupts(
   BHAB_Handle handle   /* [in] BHAB Handle */
);


/******************************************************************************
Summary:
   Enables the AP initialization done interrupt.
Description:
   This function is called by the BHAB_InitAp().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_EnableInitDoneInterrupt(
   BHAB_Handle handle  /* [in] BHAB Handle */
);


/******************************************************************************
Summary:
   Waits for a BHAB event.
Description:
   This function waits for an interrupt within the given timeout period.  Any
   interrupts are then decoded via I2C.  If the given event has been signaled,
   then this function returns BERR_SUCCESS.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_WaitForEvent(
    BHAB_Handle handle,             /* [in] BHAB Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    int timeoutMsec            /* [in] timeout in milliseconds */
);


/******************************************************************************
Summary:
   Runs the AP.
Description:
   This function takes the AP to out of reset/idle state and starts running.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_RunAp(
    BHAB_Handle handle   /* [in] BHAB PI Handle */
);


/******************************************************************************
Summary:
   Resets the AP.
Description:
   This function puts the AP in reset state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_ResetAp(
    BHAB_Handle handle   /* [in] BHAB PI Handle */
);


/******************************************************************************
Summary:
   Reads from the host access buffer.
Description:
   This function positions the 128 byte window in the AP address space to the
   host access buffer area.  (n) bytes are then read into (buf) starting from
   the given HAB (offset) address.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_ReadHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t offset,  /* [in] starting offset within the HAB to read */
    uint8_t *buf,    /* [out] holds the data read */
    uint16_t n       /* [in] number of bytes to read */
);


/******************************************************************************
Summary:
   Writes to the host access buffer.
Description:
   This function positions the 128 byte window in the AP address space to the
   host access buffer area.  (n) bytes in (buf) are then written starting from
   the given HAB (offset) address.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_WriteHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t offset,  /* [in] starting offset in the HAB to write */
    uint8_t *buf,    /* [in] specifies the data to write */
    uint8_t n        /* [in] number of bytes to write */
);


/******************************************************************************
Summary:
   This function sends the command already written in the HAB.
Description:
   This function asserts HABR, waits for the AP to release the HAB, then
   optionally checks for a service acknowlegement from the AP.  If bCheckForAck
   is true, then read_len must be greater than 0.  This function is called by
   BHAB_P_SendHabCommand().
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_ServiceHab(
    BHAB_Handle h,   /* [in] BHAB Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
);


/******************************************************************************
Summary:
   Sets certain BKNI_Events based on the source of the BCM3466 host interrupt.
Description:
   This function reads and clears the BCM3466 interrupt status registers,
   clears the enable mask of the interrupts, and sets BKNI_Events based on the
   interrupt sources.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_DecodeInterrupt(
    BHAB_Handle handle /* [in] BHAB Handle */
);



/******************************************************************************
Summary:
   Returns the BERR_Code corresponding to the given BHAB_ApStatus.
Description:
   This function returns ths BERR_Code of any errors in the given BHAB_ApStatus.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_3466_P_DecodeError(
   BHAB_Handle h,           /* [in] BHAB PI Handle */
   BHAB_ApStatus *pApStatus /* [in] AP status returned by BHAB_GetApStatus */
);


/******************************************************************************
Summary:
   This function checks if the host is currently permitted to send an HAB
   command.
Description:
   The HAB is available to the host if all 3 conditions currently apply:
   1) The AP is running.
   2) The AP has initialized.
   3) The AP is not currently servicing the HAB (i.e. HABR bit is 0)
Returns:
   BERR_Code - BERR_SUCCESS if HAB is available, otherwise BERR_Code is reason
   why HAB is not available
******************************************************************************/
BERR_Code BHAB_3466_P_CheckHab(
   BHAB_Handle h  /* [in] BHAB PI Handle */
);


/****************** BCM3466 implementation of API functions *****************/


BERR_Code BHAB_3466_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
);

BERR_Code BHAB_3466_Close(
    BHAB_Handle h   /* [in] BHAB handle */
);

BERR_Code BHAB_3466_InitAp(
    BHAB_Handle   h,       /* [in] BHAB handle */
    const uint8_t *pImage  /* [in] pointer to AP microcode image */
);

BERR_Code BHAB_3466_GetApStatus(
    BHAB_Handle   handle,    /* [in] HAB device handle */
    BHAB_ApStatus *pStatus   /* [out] AP status */
);

BERR_Code BHAB_3466_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    *pFamilyId, /* [out] Chip Family id */
    uint32_t    *pChipId,   /* [out] BHAB chip ID */
    uint16_t    *pChipVer,  /* [out] chip revision number */
    uint8_t     *pMajApVer, /* [out] AP microcode major version */
    uint8_t     *pMinApVer  /* [out] AP microcode minor version */
);

BERR_Code BHAB_3466_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
);

BERR_Code BHAB_3466_ReadRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_3466_WriteRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_3466_ReadMemory(
    BHAB_Handle h,  /* [in] BHAB PI Handle */
    uint32_t addr,  /* [in] starting address */
    uint8_t *buf,   /* [out] holds the data read */
    uint32_t n      /* [in] number of bytes to read */
);

BERR_Code BHAB_3466_WriteMemory(
    BHAB_Handle handle, /* [in] BHAB PI Handle */
    uint32_t addr, /* [in] starting address in AP RAM */
    const uint8_t *buf,  /* [in] specifies the data to write */
    uint32_t n     /* [in] number of bytes to write */
);

BERR_Code BHAB_3466_HandleInterrupt_isr(
    BHAB_Handle handle   /* [in] BHAB handle */
);

BERR_Code BHAB_3466_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] HAB device handle */
);

BERR_Code BHAB_3466_EnableLockInterrupt(
    BHAB_Handle handle, /* [in] BHAB Handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
);

BERR_Code BHAB_3466_InstallInterruptCallback(
    BHAB_Handle handle,                /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,    /* [in] Callback function pointer */
    void * pParm1,                     /* [in] Paramater1 for callback function*/
    int parm2                          /* [in] Paramater2 for callback function*/
);

BERR_Code BHAB_3466_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
);

BERR_Code BHAB_3466_SendHabCommand(
    BHAB_Handle h,      /* [in] BHAB PI Handle */
    uint8_t *write_buf, /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of bytes in the HAB command */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    bool bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
    uint16_t command_len
);

BERR_Code BHAB_3466_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
);

BERR_Code BHAB_3466_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings     /* [out] HAB config settings. */
);

BERR_Code BHAB_3466_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings     /* [in] HAB config settings. */
);

BERR_Code BHAB_3466_GetLnaStatus(
    BHAB_Handle handle,                     /* [in] Device handle */
    BHAB_LnaStatus *pStatus
);

BERR_Code BHAB_3466_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings     /* [out] internal gain settings. */
);

BERR_Code BHAB_3466_GetExternalGain(
    BHAB_Handle handle,                               /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings /* [in] external gain settings. */
);

BERR_Code BHAB_3466_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings   /* [in] external gain settings. */
);

BERR_Code BHAB_3466_Reset(
    BHAB_Handle handle    /* [in] BHAB handle */
);

BERR_Code BHAB_3466_GetRecalibrateSettings(
    BHAB_Handle handle,
    BHAB_RecalibrateSettings *pSettings
);

BERR_Code BHAB_3466_SetRecalibrateSettings (
    BHAB_Handle handle,
    const BHAB_RecalibrateSettings *pSettings
);

BERR_Code BHAB_3466_GetAvsData(
    BHAB_Handle handle,         /* [in] Device handle */
    BHAB_AvsData *pAvsData      /* [out] pointer to AVS Data */
);

BERR_Code BHAB_3466_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels   /* [out] Returns total tuner channels */
);

BERR_Code BHAB_3466_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities    /* [out] Returns chip capabilities */
);

#endif
