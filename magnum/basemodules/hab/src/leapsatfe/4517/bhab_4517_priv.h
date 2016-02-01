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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BHAB_4517_PRIV_H
#define BHAB_4517_PRIV_H

#include "bstd.h"
#include "bi2c.h"
#ifdef BHAB_SUPPORT_SPI
#include "bspi.h"
#endif
#include "bkni.h"
#include "bkni_multi.h"
#include "bhab_priv.h"
#include "bhab_4517.h"


typedef struct BHAB_4517_P_CallbackInfo
{
   BHAB_IntCallbackFunc func;
   BHAB_4517_IrqStatus  callbackParams;
} BHAB_4517_P_CallbackInfo;


typedef struct BHAB_4517_P_Handle
{
    BREG_I2C_Handle  hI2cRegister;    /* handle to the master I2C device used to access registers */
#ifdef BHAB_SUPPORT_SPI
    BREG_SPI_Handle  hSpiRegister;    /* handle to the master SPI device used to access registers */  
#endif    
    BKNI_EventHandle hInterruptEvent; /* interrupt event handle */   
    BKNI_EventHandle hApiEvent;       /* API event handle */   
    BKNI_EventHandle hInitDoneEvent;  /* AP initialization done event handle */
    BKNI_EventHandle hHabDoneEvent;   /* HAB done event handle */
    BHAB_4517_P_CallbackInfo callbackInfo;
} BHAB_4517_P_Handle;


BERR_Code BHAB_4517_P_Open(
    BHAB_Handle *handle,              /* [out] BHAB handle */
    void        *pReg,                /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
);

BERR_Code BHAB_4517_P_Close(
    BHAB_Handle h   /* [in] BHAB handle */
);

BERR_Code BHAB_4517_P_InitAp(
    BHAB_Handle   h,       /* [in] BHAB handle */
    const uint8_t *pImage  /* [in] pointer to AP microcode image */
);

BERR_Code BHAB_4517_P_GetApVersion(
    BHAB_Handle    handle,     /* [in]  BHAB handle */
    uint32_t       *pFamilyId, /* [out] Chip Family id */
    uint32_t       *pChipId,   /* [out] chip id */
    uint16_t       *pChipVer,  /* [out] chip revision number */
    uint8_t        *pMajApVer,    /* [out] AP microcode major version */
    uint8_t        *pMinApVer     /* [out] AP microcode minor version */
);

BERR_Code BHAB_4517_P_GetApStatus(
    BHAB_Handle   handle,    /* [in] HAB device handle */
    BHAB_ApStatus *pStatus   /* [out] AP status */
);

BERR_Code BHAB_4517_P_ReadRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_4517_P_WriteRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_4517_P_ReadMemory(
    BHAB_Handle h,  /* [in] BHAB PI Handle */
    uint32_t addr,  /* [in] starting address */
    uint8_t *buf,   /* [out] holds the data read */
    uint32_t n      /* [in] number of bytes to read */
);

BERR_Code BHAB_4517_P_WriteMemory(
    BHAB_Handle handle, /* [in] BHAB PI Handle */ 
    uint32_t addr,      /* [in] starting address in AP RAM */
    const uint8_t *buf, /* [in] specifies the data to write */
    uint32_t n          /* [in] number of bytes to write */
);

BERR_Code BHAB_4517_P_HandleInterrupt_isr(
    BHAB_Handle handle   /* [in] BHAB handle */
);

BERR_Code BHAB_4517_P_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] HAB device handle */
);

BERR_Code BHAB_4517_P_InstallInterruptCallback(
    BHAB_Handle handle,             /* [in] BHAB handle */
    BHAB_DevId eDevId,              /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack, /* [in] Callback function pointer */
    void * pParm1,                  /* [in] Paramater1 for callback function*/
    int parm2                       /* [in] Paramater2 for callback function*/
); 

BERR_Code BHAB_4517_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
);

BERR_Code BHAB_4517_P_SendHabCommand(
    BHAB_Handle h,      /* [in] BHAB PI Handle */
    uint8_t *write_buf, /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of bytes in the HAB command */ 
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    bool bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
    uint16_t command_len
);

BERR_Code BHAB_4517_P_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
);

BERR_Code BHAB_4517_P_WriteBbsi(
   BHAB_Handle h, /* [in] BHAB PI Handle */
   uint8_t addr,  /* [in] address */
   uint8_t *buf,  /* [in] data to write */
   uint32_t n     /* [in] number of bytes to write */
);

BERR_Code BHAB_4517_P_ReadBbsi(
   BHAB_Handle h, /* [in] BHAB PI Handle */
   uint8_t addr,  /* [in] address */
   uint8_t *buf,  /* [out] buffer that holds the data */
   uint32_t n     /* [in] number of bytes to read */
);

BERR_Code BHAB_4517_P_ReadRbus(
   BHAB_Handle h, /* [in] BHAB PI Handle */
   uint32_t addr, /* [in] address */
   uint32_t *buf, /* [in] data to write */
   uint32_t n     /* [in] number of 32-bit words to read */
);

BERR_Code BHAB_4517_P_WriteRbus(
   BHAB_Handle h, /* [in] BHAB PI Handle */
   uint32_t addr, /* [in] address */
   uint32_t *buf, /* [out] buffer that holds the data */
   uint32_t n     /* [in] number of 32-bit words to read */
);

BERR_Code BHAB_4517_P_ResetAp(
   BHAB_Handle h  /* [in] BHAB PI Handle */
);

BERR_Code BHAB_4517_P_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
);

BERR_Code BHAB_4517_P_Reset(
	BHAB_Handle handle    /* [in] BHAB handle */
);

#endif
