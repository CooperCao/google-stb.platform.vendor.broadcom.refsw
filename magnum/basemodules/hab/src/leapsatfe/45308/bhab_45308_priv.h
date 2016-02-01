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
#ifndef _BHAB_45308_PRIV_H_
#define _BHAB_45308_PRIV_H_

#include "bstd.h"
#include "bi2c.h"
#include "bspi.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bhab_priv.h"
#include "bhab_45308.h"


typedef struct BHAB_45308_P_CallbackInfo
{
   BHAB_IntCallbackFunc func;
   BHAB_45308_IrqStatus  callbackParams;
} BHAB_45308_P_CallbackInfo;


typedef struct BHAB_45308_P_Handle
{
   uint32_t         habBaseAddr;     /* address of HAB */
   BREG_I2C_Handle  hI2cRegister;    /* handle to the master I2C device used to access registers for 4548 bondout option */
   BREG_SPI_Handle  hSpiRegister;    /* handle to the master SPI device used to access registers for 4548 bondout option */
   BKNI_EventHandle hInterruptEvent; /* interrupt event handle */
   BKNI_EventHandle hApiEvent;       /* API event handle */
   BKNI_EventHandle hInitDoneEvent;  /* AP initialization done event handle */
   BKNI_EventHandle hHabDoneEvent;   /* HAB done event handle */
   BKNI_EventHandle hFlashDoneEvent; /* Flash operation done event handle */
   BHAB_45308_P_CallbackInfo cbSat;  /* SAT callback info */
   BHAB_45308_P_CallbackInfo cbDsq;  /* DSQ callback info */
   BHAB_45308_P_CallbackInfo cbFsk;  /* FSK callback info */
   BHAB_45308_P_CallbackInfo cbWfe;  /* WFE callback info */
} BHAB_45308_P_Handle;


BERR_Code BHAB_45308_P_Open(BHAB_Handle *handle, void *pReg, const BHAB_Settings *pDefSettings);
BERR_Code BHAB_45308_P_Close(BHAB_Handle h);
BERR_Code BHAB_45308_P_InitAp(BHAB_Handle h, const uint8_t *pImage);
BERR_Code BHAB_45308_P_GetApStatus(BHAB_Handle handle, BHAB_ApStatus *pStatus);
BERR_Code BHAB_45308_P_ReadRegister(BHAB_Handle handle, uint32_t reg, uint32_t *val);
BERR_Code BHAB_45308_P_WriteRegister(BHAB_Handle handle, uint32_t reg, uint32_t *val);
BERR_Code BHAB_45308_P_ReadMemory(BHAB_Handle h, uint32_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_HandleInterrupt_isr(BHAB_Handle handle);
BERR_Code BHAB_45308_P_ProcessInterruptEvent(BHAB_Handle handle);
BERR_Code BHAB_45308_P_InstallInterruptCallback(BHAB_Handle handle, BHAB_DevId eDevId, BHAB_IntCallbackFunc fCallBack, void *pParm1, int parm2);
BERR_Code BHAB_45308_P_UnInstallInterruptCallback(BHAB_Handle handle, BHAB_DevId eDevId);
BERR_Code BHAB_45308_P_SendHabCommand(BHAB_Handle h, uint8_t *write_buf, uint16_t write_len, uint8_t *read_buf, uint16_t read_len, bool bCheckForAck, bool bInsertTermination, uint16_t command_len);
BERR_Code BHAB_45308_P_GetInterruptEventHandle(BHAB_Handle handle, BKNI_EventHandle *hEvent);
BERR_Code BHAB_45308_P_WriteBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_ReadBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_ReadRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_WriteRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_45308_P_ResetAp(BHAB_Handle h);
BERR_Code BHAB_45308_P_GetVersionInfo(BHAB_Handle handle, BFEC_SystemVersionInfo *pVersionInfo);
BERR_Code BHAB_45308_P_GetAvsData(BHAB_Handle handle, BHAB_AvsData *pData);
BERR_Code BHAB_45308_P_Reset(BHAB_Handle handle);
BERR_Code BHAB_45308_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);
BERR_Code BHAB_45308_P_ProcessInterruptEvent(BHAB_Handle handle);

#endif /* _BHAB_45308_PRIV_H_ */
