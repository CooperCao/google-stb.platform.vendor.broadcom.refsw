/***************************************************************************
 *     Copyright (c) 2003-2008, Broadcom Corporation
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

#ifndef BVSB_PRIV_H__
#define BVSB_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bvsb.h"

typedef enum BVSB_AccessType
{
   BVSB_AccessType_eWrite,
   BVSB_AccessType_eRead
} BVSB_AccessType;


#define BVSB_CHK_RETCODE(func) \
   { if ((retCode = BERR_TRACE(func)) != BERR_SUCCESS) goto done; }

#if 0
#define BVSB_WriteRegister(hVSB, offset, val) \
   BVSB_P_AccessRegister(hVSB, BVSB_AccessType_eWrite, offset, val)

#define BVSB_ReadRegister(hVSB, offset, val) \
   BVSB_P_AccessRegister(hVSB, BVSB_AccessType_eRead, offset, val)
#else
#define BVSB_WriteRegister(hVSB, offset, val) \
   BREG_I2C_Write(hVSB->hRegister, hVSB->settings.chipAddr, offset, val, 1)
   
#define BVSB_ReadRegister(hVSB, offset, val) \
   BREG_I2C_Read(hVSB->hRegister, hVSB->settings.chipAddr, offset, val, 1)   
#endif

typedef struct BVSB_P_Handle
{
   BREG_I2C_Handle     hRegister;
   BVSB_Settings       settings;
   BKNI_EventHandle    hInterruptEvent;
   BKNI_EventHandle    hApiEvent;
   BKNI_EventHandle    hLockStateChangeEvent;
   BKNI_EventHandle    hHabEvent;
   BKNI_EventHandle    hInitEvent;
   bool                bLocked;
   BVSB_AcqParams      acqParams;
   uint32_t            corrErrCount;
   uint32_t            ucorrBlockCount;
} BVSB_P_Handle;


/* I2C access functions */
#if 0
BERR_Code BVSB_P_AccessRegister(BVSB_Handle hVSB, BVSB_AccessType accessType, uint8_t offset, uint8_t *val);
#endif

/* AP control functions */
BERR_Code BVSB_P_ResetAp(BVSB_Handle hVSB);
BERR_Code BVSB_P_RunAp(BVSB_Handle hVSB);
BERR_Code BVSB_P_IdleAp(BVSB_Handle hVSB);

/* HAB access functions */
BERR_Code BVSB_P_SendHabCommand(BVSB_Handle hVSB, uint8_t write_len, uint8_t *write_buf, uint8_t read_len, uint8_t *read_buf);

/* interrupt functions */
BERR_Code BVSB_P_DisableInitDoneInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_EnableInitDoneInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_DisableHabInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_EnableHabInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_DisableLossLockInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_EnableLossLockInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_DisableLockInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_EnableLockInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_DisableAllInterrupts(BVSB_Handle hVSB);
BERR_Code BVSB_P_WaitForApiEvent(BVSB_Handle hVSB, BKNI_EventHandle hEvent, int timeoutMsec);
BERR_Code BVSB_P_DecodeInterrupt(BVSB_Handle hVSB);
BERR_Code BVSB_P_EnableHostInterrupt(BVSB_Handle hVSB, bool bEnable);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVSB_PRIV_H__ */
/* End of File */

