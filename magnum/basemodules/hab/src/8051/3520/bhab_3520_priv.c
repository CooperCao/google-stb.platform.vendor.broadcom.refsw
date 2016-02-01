/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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

#include "bhab_3520_priv.h"

BDBG_MODULE(BHAB);

/******************************************************************************
 BHAB_3520_Open()
******************************************************************************/
BERR_Code BHAB_3520_Open(
	BHAB_Handle *handle,     /* [out] BHAB handle */
	void        *pReg,       /* [in] pointer ot i2c or spi handle */
	const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
	BERR_Code retCode;
	BHAB_Handle hDev;
	BHAB_3520_P_Handle *h3520Dev;
	unsigned i;

	BDBG_ASSERT(pDefSettings->interruptEnableFunc);

	hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
	BDBG_ASSERT(hDev);
	h3520Dev = (BHAB_3520_P_Handle *)BKNI_Malloc(sizeof(BHAB_3520_P_Handle));
	BDBG_ASSERT(h3520Dev);
	hDev->pImpl = (void*)h3520Dev;
	
	h3520Dev->hRegister = (BREG_I2C_Handle)pReg;
	BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));

	/* create events */
	BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hInterruptEvent)));
	BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hApiEvent)));
	BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hLockChangeEvent)));
	BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hInitDoneEvent)));
	BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3520Dev->hHabDoneEvent)));
	BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h3520Dev->hMutex)));

	h3520Dev->last_page_16_15 = 0xFF;
	h3520Dev->last_page_14_7 = 0xFF;
	h3520Dev->last_mbox_15_8 = 0xFF;	

	for(i=0; i<BHAB_DevId_eMax; i++){
		h3520Dev->InterruptCallbackInfo[i].func = NULL;
		h3520Dev->InterruptCallbackInfo[i].pParm1 = NULL;
		h3520Dev->InterruptCallbackInfo[i].parm2 = (int)NULL;
	}

	retCode = BHAB_3520_P_DisableInterrupts(hDev);
	*handle = hDev;

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_Close()
******************************************************************************/
BERR_Code BHAB_3520_Close(BHAB_Handle handle)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	BDBG_ASSERT(handle);
   
	retCode = BHAB_3520_P_DisableInterrupts(handle);
	BKNI_DestroyEvent(p3520->hInterruptEvent);
	BKNI_DestroyEvent(p3520->hApiEvent);
	BKNI_DestroyEvent(p3520->hLockChangeEvent);
	BKNI_DestroyEvent(p3520->hInitDoneEvent);
	BKNI_DestroyEvent(p3520->hHabDoneEvent);
	BKNI_DestroyMutex(p3520->hMutex);

	BKNI_Free((void*)3520);
	BKNI_Free((void*)handle);

	return retCode;
}


/******************************************************************************
 BHAB_3520_P_InitAp()
******************************************************************************/
BERR_Code BHAB_3520_InitAp(
	BHAB_Handle handle,       /* [in] BHAB handle */
	const uint8_t *pHexImage  /* [in] pointer to BCM3520 microcode image */
)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint16_t n, addr;
	const uint8_t *pImage;
	uint8_t sb, retries;

	/* disable interrupts */	
	BHAB_CHK_RETCODE(BHAB_3520_P_DisableInterrupts(handle));
	BHAB_CHK_RETCODE(BHAB_3520_P_EnableHostInterrupt(handle, false));

	/* reset the AP before downloading the microcode */
	BHAB_CHK_RETCODE(BHAB_3520_P_ResetAp(handle));

	/* download to RAM */
	pImage = pHexImage;
	while (1)
	{
		n = (*pImage++ << 8);
		n |= *pImage++;
		
		if (n == 0)
			break;
		
		addr = (*pImage++ << 8);
		addr |= *pImage++;
		
		for (retries = 0; retries < 3; retries++)
		{
			BHAB_CHK_RETCODE(BHAB_3520_WriteMemory(handle, addr, (uint8_t *)pImage, n));
			pImage += n;
			
			/* check for host transfer error */
			BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));			
			if ((sb & DEVICE(STAT1_H_ER)) == 0)
				break;
			
			BDBG_WRN(("host transfer error\n"));
			BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));			
		}
	}
	
	/* enable init done interrupt */
	BHAB_CHK_RETCODE(BHAB_3520_P_EnableInitDoneInterrupt(handle));
	
	/* clear H_STAT1 */
	sb = 0xFF;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));	
	
	/* start running the AP */
	if ((retCode = BHAB_3520_P_RunAp(handle)) != BERR_SUCCESS)
		goto done;
	
	/* wait for init done interrupt */
	if (BHAB_3520_P_WaitForEvent(handle, p3520->hInitDoneEvent, 10) != BERR_SUCCESS)
	{
		BDBG_ERR(("AP initialization timeout\n")); 
		BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);            
	}
	


 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_GetApStatus()
******************************************************************************/
BERR_Code BHAB_3520_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t buf[3];

	*pStatus = 0;
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), buf, 3));
	*pStatus = buf[0] | (buf[1] << 8) | (buf[2] << 16);
   
 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_GetApVersion()
******************************************************************************/
BERR_Code BHAB_3520_GetApVersion(
	BHAB_Handle handle,          /* [in] BHAB handle */
	uint16_t    *pChipId,   /* [out] BHAB chip ID */
	uint16_t    *pChipVer,  /* [out] chip revision number */
	uint8_t     *pApVer,    /* [out] AP microcode version */
	uint8_t     *pScrVer,   /* [out] acquisition script version */
	uint8_t     *pCfgVer    /* [out] host configuration version */
)
{
	BERR_Code retCode;
	uint8_t buf[16];

	buf[0] = 0x09;
	buf[14] = 0x00;
	BHAB_CHK_RETCODE(BHAB_3520_SendHabCommand(handle, buf, 15, buf, 14, true, false));

	*pChipId = ((buf[1] << 8) | buf[2]);
	*pChipVer = ((buf[3] << 8) | buf[4]);
	*pApVer = buf[5];
	*pScrVer = buf[6];
	*pCfgVer = buf[7];

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_ReadRegister()
******************************************************************************/
BERR_Code BHAB_3520_ReadRegister(
	BHAB_Handle handle,     /* [in] BHAB handle */
	uint32_t    reg,   /* [in] address of register to read */
	uint32_t    *val   /* [in] contains data that was read */
)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;

	if ((reg >= 0x80) && (reg <= 0xFF))
	{
		/* accessing host register space */
		BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, reg, &sb, 1));      
		*val = (uint32_t)sb;
	}
	else
	{
		/* accessing internal registers through io_mbox */
		BHAB_CHK_RETCODE(BHAB_3520_ReadMbox(handle, (uint16_t)reg, val));
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_WriteRegister()
******************************************************************************/
BERR_Code BHAB_3520_WriteRegister(
	BHAB_Handle handle,     /* [in] BHAB handle */
	uint32_t    reg,   /* [in] address of register to read */
	uint32_t    *val   /* [in] contains data that was read */
)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;

	if ((reg >= 0x80) && (reg <= 0xFF))
	{
		/* accessing host register space */
		sb = (uint8_t)(*val);
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, reg, &sb, 1));      
	}
	else
	{
		/* accessing internal registers through io_mbox */
		BHAB_CHK_RETCODE(BHAB_3520_WriteMbox(handle, (uint16_t)reg, val));
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_ReadMemory()
******************************************************************************/
BERR_Code BHAB_3520_ReadMemory(BHAB_Handle handle, uint16_t addr, uint8_t *buf, uint16_t n)
{   
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	if (((uint32_t)addr + (uint32_t)n) > BHAB_MEM_SIZE)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	BHAB_CHK_RETCODE(BHAB_3520_P_SetApWindow(handle, BHAB_WINDOW_IRAM + addr));
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, addr & 0x7F, buf, n));   

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_WriteMemory()
******************************************************************************/
BERR_Code BHAB_3520_WriteMemory(BHAB_Handle handle, uint16_t addr, uint8_t *buf, uint16_t n)
{   
	BERR_Code retCode = BERR_SUCCESS;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint16_t  curr_addr, nbytes, bytes_left;

	if ((addr >= BHAB_IRAM_SIZE) || (n >= BHAB_IRAM_SIZE))
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	if ((addr + n) > BHAB_IRAM_SIZE)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	curr_addr = addr;
	bytes_left = n;
	while (bytes_left > 0)
	{
		BHAB_CHK_RETCODE(BHAB_3520_P_SetApWindow(handle, BHAB_WINDOW_IRAM + curr_addr));
		nbytes = 128 - (curr_addr % 128);
		if (nbytes > bytes_left)
			nbytes = bytes_left;
		bytes_left -= nbytes;
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, curr_addr & 0x7F, buf, nbytes));
		curr_addr += nbytes;
		buf += nbytes;
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_ReadMbox()
******************************************************************************/
BERR_Code BHAB_3520_ReadMbox(
	BHAB_Handle handle,    /* [in] BHAB PI Handle */
	uint16_t    reg,  /* [in] RBUS register address */
	uint32_t    *val  /* [out] value read from register */
)
{   
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb, i, buf[4];

	/* update bits 15:8 of mbox address if changed */
	sb = reg >> 8;
	if (sb != p3520->last_mbox_15_8)
	{
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_A_15_8), &sb, 1));	
		p3520->last_mbox_15_8 = sb;
	}

	/* read from mbox */
	sb = reg & 0xFC;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_CMD), &sb, 1));

	/* check for mbox transfer complete */
	for (i = 0; i < 3; i++)
	{
		BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_STATUS), &sb, 1));       
		if ((sb & 0x80) == 0)
		{
			if (sb & 0x40)
			{
				BERR_TRACE(retCode = BHAB_ERR_IOMB_XFER);
				goto done;
			}
			else
			{
				/* transfer completed - now get the data */
				BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_D_31_24), buf, 4));
				*val = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
			}
			break;
		}
	}

	if (i >= 3)
	{
		/* this should not happen */
		BERR_TRACE(retCode = BHAB_ERR_IOMB_BUSY);
		BDBG_ERR(("IO_MBOX busy\n"));
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_WriteMbox()
******************************************************************************/
BERR_Code BHAB_3520_WriteMbox(
	BHAB_Handle handle,    /* [in] BHAB PI Handle */
	uint16_t    reg,  /* [in] RBUS register address */
	uint32_t    *val  /* [in] value to write */
)
{	
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t buf[6], sb, i;

	/* write addr, data, and cmd in one i2c transaction */
	buf[0] = reg >> 8;
	buf[1] = *val >> 24;
	buf[2] = *val >> 16;
	buf[3] = *val >> 8;
	buf[4] = *val & 0xFF; 
	buf[5] = (reg & 0xFC) | 0x01;
	i = (buf[0] != p3520->last_mbox_15_8) ? 0 : 1;
	p3520->last_mbox_15_8 = buf[0];
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, i ? DEVICE(SH_SFR_IO_MBOX_D_31_24) : DEVICE(SH_SFR_IO_MBOX_A_15_8), &buf[i], 6-i));

	for (i = 0; i < 3; i++)
	{
		/* check for mbox transfer complete */
		BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_STATUS), &sb, 1));
		
		if ((sb & 0x80) == 0)
		{
			if (sb & 0x40)
			{
				BERR_TRACE(retCode = BHAB_ERR_IOMB_XFER);
			}
			goto done;
		}
	}

	if (sb & 0x80)
	{
		/* this should not happen */
		BERR_TRACE(retCode = BHAB_ERR_IOMB_BUSY);
		BDBG_ERR(("IO_MBOX busy\n"));
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_3520_HandleInterrupt_isr(
	BHAB_Handle handle /* [in] BHAB handle */
)
{
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	BDBG_ASSERT(handle);
   
	handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);
	BKNI_SetEvent(p3520->hApiEvent);   
	BKNI_SetEvent(p3520->hInterruptEvent);  

	return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_3520_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_3520_ProcessInterruptEvent(
	BHAB_Handle handle  /* [in] BHAB handle */
)
{
	BERR_Code retCode;
   
	BDBG_ASSERT(handle);
	BHAB_CHK_RETCODE(BHAB_3520_P_DecodeInterrupt(handle));
	BHAB_3520_P_EnableHostInterrupt(handle, true);
   
 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BHAB_3520_EnableLockInterrupt(
	BHAB_Handle handle,  /* [in] BHAB handle */
	BHAB_DevId eDevId,    /* [in] Device ID */
	bool bEnable   /* [in] true = enable Lock  interrupt */
)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;

	BSTD_UNUSED(eDevId);

	BHAB_3520_P_EnableHostInterrupt(handle, false);
	sb = bEnable ? DEVICE(STAT2_LOCK_MASK) : 0;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT2), &sb, 1)); 				
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &sb, 1));  		
	BHAB_3520_P_EnableHostInterrupt(handle, true);

 done:
	return retCode;   
}
	

/******************************************************************************
 BHAB_3520_InstallInterruptCallback()
******************************************************************************/ 
BERR_Code BHAB_3520_InstallInterruptCallback(
	BHAB_Handle handle,  /* [in] BHAB handle */
	BHAB_DevId eDevId,    /* [in] Device ID */
	BHAB_InterruptType eInterruptType, /* [in] Id for Interrupt to install the callback*/
	BHAB_IntCallbackFunc fCallBack,
	void * pParm1, 
	int parm2
)
{
	BHAB_P_CallbackInfo *callback;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	BDBG_ASSERT(handle);
	
	if (eInterruptType >= BHAB_Interrupt_eMax) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
	/*TODO : setup bitmask to enable/disable interrupt callabcks*/

	if (eDevId >= BHAB_DevId_eMax) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	callback = &p3520->InterruptCallbackInfo[eDevId];

	callback->func = fCallBack;
	callback->pParm1 = pParm1;
	callback->parm2 = parm2;

	return BERR_TRACE(BERR_SUCCESS);
}


/******************************************************************************
 BHAB_3520_UnInstallInterruptCallback()
******************************************************************************/ 
BERR_Code BHAB_3520_UnInstallInterruptCallback(
	BHAB_Handle handle,  /* [in] BHAB handle */
	BHAB_DevId eDevId,    /* [in] Device ID */
	BHAB_InterruptType eInterruptType /* [in] Id for Interrupt to install the callback*/
)
{
	BHAB_P_CallbackInfo *callback;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	BDBG_ASSERT(handle);
	
	if (eInterruptType >= BHAB_Interrupt_eMax) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	if (eDevId >= BHAB_DevId_eMax) {
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	callback = &p3520->InterruptCallbackInfo[eDevId];

	callback->func = NULL;
	callback->pParm1 = NULL;
	callback->parm2 = (int)NULL;

	return BERR_TRACE(BERR_SUCCESS);
}	
	

/******************************************************************************
 BHAB_3520_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_3520_SendHabCommand(
	BHAB_Handle handle, /* [in] BHAB PI Handle */
	uint8_t *write_buf, /* [in] specifies the HAB command to send */
	uint16_t write_len,  /* [in] number of bytes in the HAB command */ 
	uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
	uint16_t read_len,   /* [in] number of bytes to read from the HAB */
	bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
	bool bInsertTermination /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
)
{
	BERR_Code retCode;

	BHAB_P_ACQUIRE_MUTEX(handle);
   
	if ((write_len > 127) || (read_len > 127) || (write_len == 0))
		return (BERR_TRACE(BERR_INVALID_PARAMETER));

	BHAB_CHK_RETCODE(BHAB_3520_P_CheckHab(handle));
  
	/* write the command to the HAB */
	BHAB_CHK_RETCODE(BHAB_3520_P_WriteHab(handle, 0, write_buf, write_len));

	if(bInsertTermination){
		BDBG_ASSERT(read_len<write_len);
		write_buf[read_len] = 0x00;
		BHAB_CHK_RETCODE(BHAB_3520_P_WriteHab(handle, read_len, &write_buf[read_len], 1));
	}


	/* wait for the AP to service the HAB, and then read any return data */
	BHAB_CHK_RETCODE(BHAB_3520_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80));
 
 done:
	BHAB_P_RELEASE_MUTEX(handle);

	return retCode;
}


/******************************************************************************
 BHAB_3520_GetLockStateChangeEventHandle()
******************************************************************************/
BERR_Code BHAB_3520_GetLockStateChangeEventHandle(
	BHAB_Handle handle,            /* [in] BHAB handle */
	BKNI_EventHandle *hEvent       /* [out] lock event handle */
)
{
	*hEvent = ((BHAB_3520_P_Handle *)(handle->pImpl))->hLockChangeEvent;
	return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_3520_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_3520_GetInterruptEventHandle(
	BHAB_Handle handle,            /* [in] BHAB handle */
	BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
	*hEvent = ((BHAB_3520_P_Handle *)(handle->pImpl))->hInterruptEvent;
	return BERR_SUCCESS;
}



/****************************Private Functions*********************************/




/******************************************************************************
 BHAB_3520_P_EnableHostInterrupt() 
******************************************************************************/
BERR_Code BHAB_3520_P_EnableHostInterrupt(
	BHAB_Handle handle, /* [in] HAB handle */
	bool bEnable        /* [in] true=enables the L1 interrupt on the host processor */
)
{
	BKNI_EnterCriticalSection();
	handle->settings.interruptEnableFunc(bEnable, handle->settings.interruptEnableFuncParam);
	BKNI_LeaveCriticalSection();   

	return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_3520_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BHAB_3520_P_DisableInterrupts(
	BHAB_Handle handle   /* [in] BHAB Handle */
)
{   
	BERR_Code err;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	const uint8_t val[4] = {0, 0, 0, 0};
   
	/* clear IEx registers */
	err = BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), val, 4);
	return err;   
}


/******************************************************************************
 BHAB_3520_P_EnableInitDoneInterrupt()
******************************************************************************/ 
BERR_Code BHAB_3520_P_EnableHabDoneInterrupt(
	BHAB_Handle handle /* [in] BHAB PI Handle */
)
{  
	BERR_Code retCode;	
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;
   
	sb = DEVICE(STAT1_HAB_DONE);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));
	sb = DEVICE(STAT1_HAB_DONE);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), &sb, 1));
   
 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_EnableInitDoneInterrupt()
******************************************************************************/ 
BERR_Code BHAB_3520_P_EnableInitDoneInterrupt(
	BHAB_Handle handle /* [in] BHAB PI Handle */
)
{  
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;
   
	sb = DEVICE(STAT2_INIT_DONE);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT2), &sb, 1));	
	sb = DEVICE(STAT2_INIT_DONE);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &sb, 1));
   
 done:
	return retCode;
}


/******************************************************************************
 BERR_3520_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_3520_P_WaitForEvent(
	BHAB_Handle handle,             /* [in] BHAB PI Handle */
	BKNI_EventHandle hEvent,   /* [in] event to wait on */
	int timeoutMsec            /* [in] timeout in milliseconds */
)
{	
	BERR_Code retCode = BERR_SUCCESS;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	while (1)
	{   
		BHAB_3520_P_EnableHostInterrupt(handle, true);
		if ((retCode = BKNI_WaitForEvent(p3520->hApiEvent, timeoutMsec)) == BERR_TIMEOUT)
			break;
		
		BHAB_3520_P_DecodeInterrupt(handle);
		if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
			break;
	}

	BHAB_3520_P_EnableHostInterrupt(handle, true);
   
	return retCode;

}


/******************************************************************************
 BHAB_3520_P_RunAp()
******************************************************************************/
BERR_Code BHAB_3520_P_RunAp(BHAB_Handle handle)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t   sb, sb2;

	/* check if the AP is currently running */
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));

	if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RUN))
	{
		/* start running the AP */
		sb2 = DEVICE(AP_RUN);
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb2, 1));		

		/* verify that the AP is running */
		BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));		
		if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RUN))
		{
			BDBG_ERR(("unable to run the AP\n"));
			BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
			goto done;
		}

		/* clear AP_change state bit */
		sb2 = DEVICE(STAT1_AP_OP_CHG);
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb2, 1));	
	}
	else
	{
		BDBG_WRN(("BHAB_P_RunAp(): AP already running\n"));
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_ResetAp()
******************************************************************************/
BERR_Code BHAB_3520_P_ResetAp(BHAB_Handle handle)
{   
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t i, sb, buf[4];

	/* initialize JDEC */
	sb = DEVICE(JDEC_RAM);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_JDEC), &sb, 1));   

	/* reset the AP */
	sb = DEVICE(AP_RESET);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));

	/* reset AP status */
	sb = 0x0B;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_AP_CMD), &sb, 1));

	/* reset all interrupt status */
	for (i = 0; i < 4; i++)
		buf[i] = 0xFF;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), buf, 4));

	/* clear MSGx registers */
	buf[0] = buf[1] = 0x00;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_MSG1), buf, 2));

	/* verify that AP is reset */
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));   
	if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RESET))
	{
		BDBG_ERR(("unable to reset the AP\n"));
		BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
	}

 done:
	p3520->last_page_16_15 = 0xFF;
	p3520->last_page_14_7 = 0xFF;

	return retCode;
}


/******************************************************************************
 BHAB_3520_P_SetApWindow()
******************************************************************************/
BERR_Code BHAB_3520_P_SetApWindow(
	BHAB_Handle handle,    /* [in] BHAB PI Handle */
	uint32_t window   /* [in] base address of the 128-byte window */
)
{   
	BERR_Code retCode = BERR_SUCCESS;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t   haddr_16_15 = (window >> 15) & 0x03;
	uint8_t   haddr_14_7 = (window >> 7) & 0xFF;
	uint8_t   buf[2];

	if (p3520->last_page_16_15 != haddr_16_15)
	{
		buf[0] = haddr_16_15;
		buf[1] = haddr_14_7;
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_ADR_16_15), buf, 2));
		p3520->last_page_16_15 = haddr_16_15;
		p3520->last_page_14_7 = haddr_14_7;
	}
	else if (p3520->last_page_14_7 != haddr_14_7)
	{
		BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_ADR_14_7), &haddr_14_7, 1));     
		p3520->last_page_14_7 = haddr_14_7;
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_3520_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
   
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	if ((addr & BHAB_HAB_SIZE) || (n & BHAB_HAB_SIZE))
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	if ((addr + n) > BHAB_HAB_SIZE)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	BHAB_CHK_RETCODE(BHAB_3520_P_SetApWindow(handle, BHAB_WINDOW_HAB));
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, addr, buf, n));

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_3520_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);

	if ((addr & BHAB_HAB_SIZE) || (n & BHAB_HAB_SIZE))
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	if ((addr + n) > BHAB_HAB_SIZE)
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	BHAB_CHK_RETCODE(BHAB_3520_P_SetApWindow(handle, BHAB_WINDOW_HAB));
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, addr, buf, n));

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_3520_P_ServiceHab(
	BHAB_Handle handle,   /* [in] BHAB PI Handle */
	uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
	uint8_t read_len,   /* [in] number of bytes to read from the HAB */
	bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
	uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{   
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t   sb, ie2, buf[2];
       
	BHAB_3520_P_EnableHostInterrupt(handle, false);
   
	/* save ie2 */
	if((retCode = BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &ie2, 1)) != BERR_SUCCESS )
		return retCode;
   
	/* clear ie2 */
	buf[0] = buf[1] = 0;
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), buf, 2));

	BKNI_WaitForEvent(p3520->hHabDoneEvent, 0);      

   
	/* send the command */
	sb = DEVICE(AP_HABR);
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));
	   

	/* enable the hab done interrupt mask */	
	BHAB_CHK_RETCODE(BHAB_3520_P_EnableHabDoneInterrupt(handle));
	
	/* wait for HAB done interrupt */  
	if (BHAB_3520_P_WaitForEvent(handle, p3520->hHabDoneEvent, 100) == BERR_TIMEOUT)
	{
		BDBG_ERR(("HAB timeout\n"));   
		sb = 0;
		BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), &sb, 1);	
		BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
		goto done;
	}


	if (read_len > 0)
	{
		BHAB_CHK_RETCODE(BHAB_3520_P_ReadHab(handle, 0, read_buf, read_len));
		if (bCheckForAck)
		{
			if (ack_byte != read_buf[0])
			{
				BDBG_ERR(("HAB command not serviced!\n"));
				BERR_TRACE(retCode = BHAB_ERR_HAB_NO_ACK);
			}
		}
	}
	
 done:
	BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &ie2, 1);

	return retCode;
}


/******************************************************************************
 BHAB_3520_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_3520_P_DecodeInterrupt(BHAB_Handle handle)
{  
	BERR_Code retCode;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t   h_ie[2], h_fstat[2], new_stat2;
	
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_FSTAT1), h_fstat, 2));
	BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), h_ie, 2));
	
	new_stat2 = 0;
	
	if (!h_fstat[0] && !h_fstat[1])
	{
		return BERR_SUCCESS;
	}
   
	/*HAB DONE INTERRUPT*/
	if (h_fstat[0] & DEVICE(STAT1_HAB_DONE))
	{
		h_ie[0] = 0;
		BKNI_SetEvent(p3520->hHabDoneEvent);				
	}
   
	/*INIT DONE INTERRUPT*/
	if (h_fstat[1] & DEVICE(STAT2_INIT_DONE))
	{
		h_ie[1] &= ~DEVICE(STAT2_INIT_DONE);		
		BKNI_SetEvent(p3520->hInitDoneEvent);
	}
      
	/*LOCK INTERRUPT*/
	if (h_fstat[1] & DEVICE(STAT2_LOCK_MASK))
	{
		new_stat2 |= DEVICE(STAT2_LOCK_MASK);
		h_ie[1] &= ~DEVICE(STAT2_LOCK_MASK);
		if(p3520->InterruptCallbackInfo[BHAB_DevId_eVSB0].func){
			p3520->InterruptCallbackInfo[BHAB_DevId_eVSB0].func(
				p3520->InterruptCallbackInfo[BHAB_DevId_eVSB0].pParm1,
				p3520->InterruptCallbackInfo[BHAB_DevId_eVSB0].parm2);
		}
		BKNI_SetEvent(p3520->hLockChangeEvent);
		if (h_fstat[1] & DEVICE(STAT2_IN_LOCK))
		{
			BDBG_MSG(("locked"));
			h_ie[1] |= DEVICE(STAT2_OUT_OF_LOCK);
		}
		else
		{
			h_ie[1] |= DEVICE(STAT2_IN_LOCK);  
			BDBG_MSG(("not locked"));			
		}
	}
         
	/* clear the interrupt status */
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), h_ie, 2));   
	BHAB_CHK_RETCODE(BREG_I2C_Write(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT2), &new_stat2, 1));
    
 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_3520_P_DecodeError(
	BHAB_Handle handle,           /* [in] BHAB PI Handle */
	BHAB_ApStatus *pApStatus /* [in] AP status returned by BHAB_GetApStatus */
)
{
	BERR_Code retCode = BERR_SUCCESS;
	BHAB_3520_P_Handle *p3520 = (BHAB_3520_P_Handle *)(handle->pImpl);
	uint8_t sb;

	if (*pApStatus & BHAB_APSTATUS_HAB_ERR)
		retCode = BHAB_ERR_HABAV;
	else if (*pApStatus & BHAB_APSTATUS_MEM_ERR)
		retCode = BHAB_ERR_MEMAV;
	else if (*pApStatus & BHAB_APSTATUS_H_ERR)
		retCode = BHAB_ERR_HOST_XFER;
	else if (*pApStatus & BHAB_APSTATUS_IOMB_ERR)
		retCode = BHAB_ERR_IOMB_XFER;
	else if (*pApStatus & BHAB_APSTATUS_HABCMD_ERR)
		retCode = BHAB_ERR_HAB_ERR;
	else if (*pApStatus & BHAB_APSTATUS_AP_ERR)
	{
		BHAB_CHK_RETCODE(BREG_I2C_Read(p3520->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_MSG1), &sb, 1));		
		switch (sb)
		{
		case 1:
		  retCode = BHAB_ERR_AP_BSC;
		  break;

		case 2:
		  retCode = BHAB_ERR_AP_STACK;
		  break;

		case 3:
		  retCode = BHAB_ERR_AP_WD;
		  break;

		case 4:
		  retCode = BHAB_ERR_AP_ISB;
		  break;

		case 5:
		  retCode = BHAB_ERR_AP_SCR;
		  break;

		case 6:
		  retCode = BHAB_ERR_AP_IRQ;
		  break;

		case 7:
		  retCode = BHAB_ERR_AP_COPY;
		  break;

		case 8:
		  retCode = BHAB_ERR_AP_EEPROM;
		  break;

		case 9:
		  retCode = BHAB_ERR_AP_HABAV;
		  break;

		default:
		  BDBG_ERR(("unknown MSG1 (=0x%02X)\n", sb));
		  retCode = BHAB_ERR_AP_UNKNOWN;
		  break;
		}
	}

 done:
	return retCode;
}


/******************************************************************************
 BHAB_3520_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_3520_P_CheckHab(
	BHAB_Handle handle    /* [in] BHAB Handle */
)
{
	BERR_Code retCode;
	BHAB_ApStatus status;

	BHAB_CHK_RETCODE(BHAB_3520_GetApStatus(handle, &status));
	if ((status & BHAB_APSTATUS_HAB_MASK) == BHAB_APSTATUS_HAB_READY)
		retCode = BERR_SUCCESS;
	else
	{
		BDBG_ERR(("AP status = 0x%08X\n", (uint32_t)status));
		BERR_TRACE(retCode = BHAB_3520_P_DecodeError(handle, &status));
	}
	
 done:
	return retCode;
}
