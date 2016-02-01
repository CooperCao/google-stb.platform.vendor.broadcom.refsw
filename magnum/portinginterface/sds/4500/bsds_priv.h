/***************************************************************************
 *     Copyright (c) 2003, Broadcom Corporation
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

#ifndef BSDS_PRIV_H__
#define BSDS_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bsds.h"

typedef enum BSDS_AccessType
{
   BSDS_AccessType_eWrite,
   BSDS_AccessType_eRead
} BSDS_AccessType;


#define BSDS_CHK_RETCODE(func) \
   { if ((retCode = BERR_TRACE(func)) != BERR_SUCCESS) goto done; }

#if 0
#define BSDS_WriteRegister(hSDS, offset, val) \
   BSDS_P_AccessRegister(hSDS, BSDS_AccessType_eWrite, offset, val)

#define BSDS_ReadRegister(hSDS, offset, val) \
   BSDS_P_AccessRegister(hSDS, BSDS_AccessType_eRead, offset, val)
#else
#define BSDS_WriteRegister(hSDS, offset, val) \
   BREG_I2C_Write(hSDS->hRegister, hSDS->settings.chipAddr, offset, val, 1)
   
#define BSDS_ReadRegister(hSDS, offset, val) \
   BREG_I2C_Read(hSDS->hRegister, hSDS->settings.chipAddr, offset, val, 1)   
#endif

typedef struct BSDS_P_Handle
{
   BREG_I2C_Handle     hRegister;
   BSDS_Settings       settings;
   BKNI_EventHandle    hInterruptEvent;
   BKNI_EventHandle    hApiEvent;
   BKNI_EventHandle    hLockStateChangeEvent;
   BKNI_EventHandle    hHabEvent;
   BKNI_EventHandle    hDiseqcEvent;
   BKNI_EventHandle    hInitEvent;
   bool                bLocked;
   BSDS_AcqParams      acqParams;
   BSDS_ModulationType turboIdx9;
} BSDS_P_Handle;

/* I2C access functions */
#if 0
BERR_Code BSDS_P_AccessRegister(BSDS_Handle hSDS, BSDS_AccessType accessType, uint8_t offset, uint8_t *val);
#endif

/* AP control functions */
BERR_Code BSDS_P_ResetAp(BSDS_Handle hSDS);
BERR_Code BSDS_P_RunAp(BSDS_Handle hSDS);
BERR_Code BSDS_P_IdleAp(BSDS_Handle hSDS);

/* HAB access functions */
BERR_Code BSDS_P_SendHabCommand(BSDS_Handle hSDS, uint8_t write_len, uint8_t *write_buf, uint8_t read_len, uint8_t *read_buf);

/* interrupt functions */
BERR_Code BSDS_P_DisableInitDoneInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableInitDoneInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_DisableHabInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableHabInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_DisableDiseqcInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableDiseqcInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_DisableLossLockInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableLossLockInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_DisableLockInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableLockInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_DisableAllInterrupts(BSDS_Handle hSDS);
BERR_Code BSDS_P_WaitForApiEvent(BSDS_Handle hSDS, BKNI_EventHandle hEvent, int timeoutMsec);
BERR_Code BSDS_P_DecodeInterrupt(BSDS_Handle hSDS);
BERR_Code BSDS_P_EnableHostInterrupt(BSDS_Handle hSDS, bool bEnable);

/* acquisition functions */
BERR_Code BSDS_P_LoadTurboCodeRate9(BSDS_Handle hSDS, BSDS_ModulationType m);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSDS_PRIV_H__ */
/* End of File */
