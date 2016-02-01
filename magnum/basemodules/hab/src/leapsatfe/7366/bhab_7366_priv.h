/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Module Description:
*
* Revision History:
*
* $brcm_Log: $
*
*****************************************************************************/
#ifndef BHAB_7366_PRIV_H
#define BHAB_7366_PRIV_H

#include "bstd.h"
#if BHAB_CHIP==4548
#include "bi2c.h"
#include "bspi.h"
#endif
#include "bkni.h"
#include "bkni_multi.h"
#include "bhab_priv.h"
#include "bhab_7366.h"


typedef struct BHAB_7366_P_CallbackInfo
{
   BHAB_IntCallbackFunc func;
   BHAB_7366_IrqStatus  callbackParams;
} BHAB_7366_P_CallbackInfo;


typedef struct BHAB_7366_P_Handle
{
   BHAB_7366_Settings settings;
#if BHAB_CHIP==4548
   BREG_I2C_Handle  hI2cRegister;    /* handle to the master I2C device used to access registers for 4548 bondout option */
   BREG_SPI_Handle  hSpiRegister;    /* handle to the master SPI device used to access registers for 4548 bondout option */
   BKNI_EventHandle hInterruptEvent; /* interrupt event handle */
   BKNI_EventHandle hApiEvent;       /* API event handle */
#else
   BREG_Handle hReg;
#endif
   BKNI_EventHandle hInitDoneEvent;  /* AP initialization done event handle */
   BKNI_EventHandle hHabDoneEvent;   /* HAB done event handle */
   BHAB_7366_P_CallbackInfo cbSat;   /* SAT callback info */
   BHAB_7366_P_CallbackInfo cbDsq;   /* DSQ callback info */
   BHAB_7366_P_CallbackInfo cbFsk;   /* FSK callback info */
   BHAB_7366_P_CallbackInfo cbWfe;   /* WFE callback info */
   uint32_t habBaseAddr;             /* base address of the HAB */
} BHAB_7366_P_Handle;


BERR_Code BHAB_7366_P_Open(
    BHAB_Handle         *handle,      /* [out] BHAB handle */
    void                *pReg,        /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
);

BERR_Code BHAB_7366_P_Close(
    BHAB_Handle h   /* [in] BHAB handle */
);

BERR_Code BHAB_7366_P_InitAp(
    BHAB_Handle   h,       /* [in] BHAB handle */
    const uint8_t *pImage  /* [in] pointer to AP microcode image */
);

BERR_Code BHAB_7366_P_GetApStatus(
    BHAB_Handle   handle,    /* [in] HAB device handle */
    BHAB_ApStatus *pStatus   /* [out] AP status */
);

BERR_Code BHAB_7366_P_ReadRegister_isrsafe(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_7366_P_WriteRegister_isrsafe(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_7366_P_ReadMemory(
    BHAB_Handle h,  /* [in] BHAB PI Handle */
    uint32_t addr,  /* [in] starting address */
    uint8_t *buf,   /* [out] holds the data read */
    uint32_t n      /* [in] number of bytes to read */
);

BERR_Code BHAB_7366_P_WriteMemory(
    BHAB_Handle   handle, /* [in] BHAB PI Handle */
    uint32_t      addr,   /* [in] starting address in AP RAM */
    const uint8_t *buf,   /* [in] specifies the data to write */
    uint32_t      n       /* [in] number of bytes to write */
);

BERR_Code BHAB_7366_P_HandleInterrupt_isr(
    BHAB_Handle handle   /* [in] BHAB handle */
);

BERR_Code BHAB_7366_P_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] HAB device handle */
);

BERR_Code BHAB_7366_P_InstallInterruptCallback(
    BHAB_Handle          handle,    /* [in] BHAB handle */
    BHAB_DevId           eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack, /* [in] Callback function pointer */
    void                 *pParm1,   /* [in] Paramater1 for callback function*/
    int                  parm2      /* [in] Paramater2 for callback function*/
);

BERR_Code BHAB_7366_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId  eDevId   /* [in] Device ID */
);

BERR_Code BHAB_7366_P_SendHabCommand(
    BHAB_Handle h,              /* [in] BHAB PI Handle */
    uint8_t     *write_buf,     /* [in] specifies the HAB command to send */
    uint16_t    write_len,      /* [in] number of bytes in the HAB command */
    uint8_t     *read_buf,      /* [out] holds the data read from the HAB */
    uint16_t    read_len,       /* [in] number of bytes to read from the HAB */
    bool        bCheckForAck,   /* [in] true = determine if the AP has serviced the command */
    bool        bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
    uint16_t    command_len
);

BERR_Code BHAB_7366_P_GetInterruptEventHandle(
    BHAB_Handle      handle,  /* [in] BHAB handle */
    BKNI_EventHandle *hEvent  /* [out] interrupt event handle */
);

BERR_Code BHAB_7366_P_WriteBbsi(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint8_t     addr, /* [in] address */
   uint8_t     *buf, /* [in] data to write */
   uint32_t    n     /* [in] number of bytes to write */
);

BERR_Code BHAB_7366_P_ReadBbsi(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint8_t     addr, /* [in] address */
   uint8_t     *buf, /* [out] buffer that holds the data */
   uint32_t    n     /* [in] number of bytes to read */
);

BERR_Code BHAB_7366_P_ReadRbus(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint32_t    addr, /* [in] address */
   uint32_t    *buf, /* [in] data to write */
   uint32_t    n     /* [in] number of 32-bit words to read */
);

BERR_Code BHAB_7366_P_WriteRbus(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint32_t    addr, /* [in] address */
   uint32_t    *buf, /* [out] buffer that holds the data */
   uint32_t    n     /* [in] number of 32-bit words to read */
);

BERR_Code BHAB_7366_P_ResetAp(
   BHAB_Handle h  /* [in] BHAB PI Handle */
);

BERR_Code BHAB_7366_P_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
);

BERR_Code BHAB_7366_P_GetAvsData(
   BHAB_Handle handle,  /* [in] BHAB handle */
   BHAB_AvsData *pData  /* [out] returned AVS data */
);

BERR_Code BHAB_7366_P_Reset(
   BHAB_Handle handle    /* [in] BHAB handle */
);

BERR_Code BHAB_7366_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);

#if BHAB_CHIP==4548
BERR_Code BHAB_4548_P_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
);
BERR_Code BHAB_4548_P_ReadRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);

BERR_Code BHAB_4548_P_WriteRegister(
    BHAB_Handle handle,  /* [in] BHAB handle */
    uint32_t    reg,     /* [in] address of register to read */
    uint32_t    *val     /* [in] contains data that was read */
);
#endif

#endif

