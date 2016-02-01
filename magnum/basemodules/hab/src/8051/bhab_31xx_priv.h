/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $ 
 * 
 ***************************************************************************/
#ifndef BHAB_31XX_PRIV_H
#define BHAB_31XX_PRIV_H

#include "bstd.h"
#include "bi2c.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bhab_priv.h"

#include "bchp_31xx.h"

#define BHAB_WINDOW_IRAM      0x00000000
#define BHAB_WINDOW_IROM      0x00008000
#define BHAB_WINDOW_HAB       0x00010000
#define BHAB_WINDOW_IDATA     0x00010400

#define BHAB_IRAM_SIZE        0xffff
#define BHAB_IROM_SIZE        0x8000
#define BHAB_MEM_SIZE         0x00010000
#define BHAB_HAB_SIZE         0x80
#define BHAB_INIT_RETRIES     0x14

#define BHAB_P_ACQUIRE_MUTEX(handle) BKNI_AcquireMutex(((BHAB_31xx_P_Handle *)(handle->pImpl))->hMutex)
#define BHAB_P_RELEASE_MUTEX(handle) BKNI_ReleaseMutex(((BHAB_31xx_P_Handle *)(handle->pImpl))->hMutex)


typedef struct BHAB_P_CallbackInfo
{
	BHAB_IntCallbackFunc func;
	void * pParm1;
	int parm2 ;
} BHAB_P_CallbackInfo;


typedef struct BHAB_31xx_P_Handle
{
	BREG_I2C_Handle  hRegister;         /* handle to the master I2C device used to access registers */
	BKNI_EventHandle hInterruptEvent;   /* interrupt event handle */
	BKNI_EventHandle hApiEvent;         /* API event handle */
	BKNI_EventHandle hInitDoneEvent;    /* AP initialization done event handle */
	BKNI_EventHandle hHabDoneEvent;     /* HAB done event handle */
	BKNI_MutexHandle hMutex;            /* Mutex handle for serialization */
	uint8_t          last_page_16_15;   /* most recent position of the 128 byte window into the AP address space */
	uint8_t          last_page_14_7;    /* most recent position of the 128 byte window into the AP address space */
	BHAB_P_CallbackInfo InterruptCallbackInfo[BHAB_DevId_eMax];
	BHAB_WatchDogTimerSettings wdtSettings;
	BHAB_NmiSettings nmiSettings;
	bool loadAP;						  /* Load(ed) Acquisition Processor microcode when starting. */
	uint8_t slaveChipAddr;  /* i2c chip address for the 31xx's i2c  slave device */
	bool isDaisyChain;
} BHAB_31xx_P_Handle;


/******************************************************************************
Summary:
   Enables/Disables the L1 host interrupt.
Description:
   Enables/Disables the L1 host interrupt by calling the application-supplied 
   callback routine the the BHAB settings.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_31xx_P_EnableHostInterrupt(
    BHAB_Handle handle, /* [in] BHAB PI handle */
    bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
);


/******************************************************************************
Summary:
   Disables all host interrupts.
Description:
   This function clears the host interrupt enable bits on the BCM31xx.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_31xx_P_DisableInterrupts(
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
BERR_Code BHAB_31xx_P_EnableInitDoneInterrupt(
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
BERR_Code BHAB_31xx_P_WaitForEvent(
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
BERR_Code BHAB_31xx_P_RunAp(
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
BERR_Code BHAB_31xx_P_ResetAp(
    BHAB_Handle handle   /* [in] BHAB PI Handle */
);


/******************************************************************************
Summary:
   Positions the 128 byte window in the AP address space.
Description:
   The last 128-byte window position is stored in the BHAB_Handle struct,
   so I2C accesses will occur only if the window has moved.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_31xx_P_SetApWindow(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t window   /* [in] base address of the 128-byte window */
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
BERR_Code BHAB_31xx_P_ReadHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t offset,  /* [in] starting offset within the HAB to read */
    uint8_t *buf,    /* [out] holds the data read */
    uint8_t n        /* [in] number of bytes to read */ 
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
BERR_Code BHAB_31xx_P_WriteHab(
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
BERR_Code BHAB_31xx_P_ServiceHab(
    BHAB_Handle h,   /* [in] BHAB Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
    uint8_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
);


/******************************************************************************
Summary:
   Sets certain BKNI_Events based on the source of the BCM31xx host interrupt.
Description:
   This function reads and clears the BCM31xx interrupt status registers, 
   clears the enable mask of the interrupts, and sets BKNI_Events based on the 
   interrupt sources.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BHAB_31xx_P_DecodeInterrupt(
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
BERR_Code BHAB_31xx_P_DecodeError(
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
BERR_Code BHAB_31xx_P_CheckHab(
   BHAB_Handle h  /* [in] BHAB PI Handle */
);



/****************** BCM31xx implementation of API functions *****************/



BERR_Code BHAB_31xx_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
);

BERR_Code BHAB_31xx_Close(
    BHAB_Handle h   /* [in] BHAB handle */
);

BERR_Code BHAB_31xx_InitAp(
    BHAB_Handle   h,       /* [in] BHAB handle */
    const uint8_t *pImage  /* [in] pointer to AP microcode image */
);

BERR_Code BHAB_31xx_GetApStatus(
    BHAB_Handle   handle,    /* [in] HAB device handle */
    BHAB_ApStatus *pStatus   /* [out] AP status */
);

BERR_Code BHAB_31xx_GetApVersion(
    BHAB_Handle    handle,     /* [in]  BHAB handle */
    uint32_t       *pFamilyId, /* [out] Chip Family id */
    uint32_t       *pChipId,   /* [out] chip id */
    uint16_t       *pChipVer,  /* [out] chip revision number */
    uint8_t        *pMajApVer,    /* [out] AP microcode major version */
    uint8_t        *pMinApVer     /* [out] AP microcode minor version */
);

BERR_Code BHAB_31xx_ReadRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_31xx_WriteRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_31xx_ReadMemory(
    BHAB_Handle h,  /* [in] BHAB PI Handle */
    uint32_t addr,  /* [in] starting address */
    uint8_t *buf,   /* [out] holds the data read */
    uint32_t n      /* [in] number of bytes to read */
);

BERR_Code BHAB_31xx_WriteMemory(
    BHAB_Handle handle, /* [in] BHAB PI Handle */ 
    uint32_t addr, /* [in] starting address in AP RAM */
    const uint8_t *buf,  /* [in] specifies the data to write */
    uint32_t n     /* [in] number of bytes to write */
);

BERR_Code BHAB_31xx_ReadMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
);

BERR_Code BHAB_31xx_WriteMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
);

BERR_Code BHAB_31xx_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings     /* [out] HAB config settings. */
);

BERR_Code BHAB_31xx_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings     /* [in] HAB config settings. */
);

BERR_Code BHAB_31xx_HandleInterrupt_isr(
    BHAB_Handle handle   /* [in] BHAB handle */
);

BERR_Code BHAB_31xx_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] HAB device handle */
);

BERR_Code BHAB_31xx_EnableLockInterrupt(
    BHAB_Handle handle, /* [in] BHAB Handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    bool bEnable   /* [in] true = enable lock interrupts, false = disables lock interrupts */
);

BERR_Code BHAB_31xx_InstallInterruptCallback(
    BHAB_Handle handle,                /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,    /* [in] Callback function pointer */
    void * pParm1,                     /* [in] Paramater1 for callback function*/
    int parm2                          /* [in] Paramater2 for callback function*/
); 

BERR_Code BHAB_31xx_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
);

BERR_Code BHAB_31xx_SendHabCommand(
    BHAB_Handle h,      /* [in] BHAB PI Handle */
    uint8_t *write_buf, /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of bytes in the HAB command */ 
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    bool bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
    uint16_t command_len
);

BERR_Code BHAB_31xx_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
);

BERR_Code BHAB_31xx_GetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_WatchDogTimerSettings *wdtSettings
);

BERR_Code BHAB_31xx_SetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_WatchDogTimerSettings *wdtSettings
);

BERR_Code BHAB_31xx_GetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_NmiSettings *nmiSettings
);

BERR_Code BHAB_31xx_SetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_NmiSettings *nmiSettings
);

BERR_Code BHAB_31xx_Reset(
	BHAB_Handle handle    /* [in] BHAB handle */
);

#endif
