/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
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
#include "bhab_31xx_priv.h"

BDBG_MODULE(bhab_31xx_priv);

/******************************************************************************
 BHAB_31xx_Open()
******************************************************************************/
BERR_Code BHAB_31xx_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
    BERR_Code retCode;
    BHAB_Handle hDev;
    BHAB_31xx_P_Handle *h31xxDev;
    unsigned i;

    BDBG_ASSERT(pDefSettings->interruptEnableFunc);

    hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
    BDBG_ASSERT(hDev);
    h31xxDev = (BHAB_31xx_P_Handle *)BKNI_Malloc(sizeof(BHAB_31xx_P_Handle));
    BDBG_ASSERT(h31xxDev);
    BKNI_Memset( h31xxDev, 0x00, sizeof(BHAB_31xx_P_Handle));
    hDev->pImpl = (void*)h31xxDev;
    
    h31xxDev->hRegister = (BREG_I2C_Handle)pReg;
    BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));
    

    /* create events */
    BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h31xxDev->hInterruptEvent)));
    BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h31xxDev->hApiEvent)));
    BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h31xxDev->hInitDoneEvent)));
    BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h31xxDev->hHabDoneEvent)));
    BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h31xxDev->hMutex)));

    h31xxDev->last_page_16_15 = 0xFF;
    h31xxDev->last_page_14_7 = 0xFF;    

    for(i=0; i<BHAB_DevId_eMax; i++){
        h31xxDev->InterruptCallbackInfo[i].func = NULL;
        h31xxDev->InterruptCallbackInfo[i].pParm1 = NULL;
        h31xxDev->InterruptCallbackInfo[i].parm2 = (int)NULL;
    }

    BKNI_Memset( &h31xxDev->nmiSettings, 0x00, sizeof(BHAB_NmiSettings));
    BKNI_Memset( &h31xxDev->wdtSettings, 0x00, sizeof(BHAB_WatchDogTimerSettings));

    retCode = BHAB_31xx_P_DisableInterrupts(hDev);
    h31xxDev->slaveChipAddr = pDefSettings->slaveChipAddr;

done:
    *handle = hDev;

    return retCode;
}


/******************************************************************************
 BHAB_31xx_Close()
******************************************************************************/
BERR_Code BHAB_31xx_Close(BHAB_Handle handle)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t buf[4], sb, i;
    
    BDBG_ASSERT(handle);

#if (BCHP_CHIP != 7125) /* This reset prevents us from restarting without reloading AP */
    buf[0]= 0x00;   /* SH_IND_GLOBAL_RESET1 */
    buf[1]= 0x0f;   /* AP_WDT_RST=1 UPG_WDT_RST=1 UPG_RST=1 PERIPH_RST=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
  
    buf[0]= 0x01;   /* SH_IND_GLOBAL_RESET0 */
    buf[1]= 0xff;   /* OTP_RST=1 TM_RST=1 reserved=1 DS_RST=1 AP_RST=1 reserved=1 reserved=1 CG_RST=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
#endif

    buf[0]= 0x02;   /* SH_IND_OSC_CTRL1 */
    buf[1]= 0x00;   /* IAMP_ADJ=150uA ICBUF_ADG=150uA IBUFD_ADJ=200uA LIMIT_AMP=inactive XTAL_ADJ_CM 1.45V */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x03;   /* SH_IND_OSC_CTRL0 ' disable diffosc clocks */
    buf[1]= 0x7d;   /* BYP_DBLR=0 DISABLE_CML6=1 DISABLE_CML5=1 DISABLE_CML4=1 DISABLE_CML3=1 DISABLE_CML2=1 ENABLE_FMON=0 PWRDN_DBLR=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x05;   /* SH_IND_POWER_UP ' power down d2cdiff_54, d2cdiff_108 and tuner */
    buf[1]= 0x00;   /* PWRUP_TUNER=0 PWRUP_108=0 PWRUP_54=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
    
    buf[0]= 0x07;   /* SH_IND_CLK_ENABLE1 ' disable internal register clocks */
    buf[1]= 0x00;   /* EN_OTP_9=0 EN_UPG_27=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x08;   /* SH_IND_CLK_ENABLE0 ' disable internal register clocks */
    buf[1]= 0x00;   /* EN_UPG_54=0 EN_PLL_54=0 EN_DS_54=0 EN_AP_54=0 EN_AP_108=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    /* reset the AP */
    sb = 0x1;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));

    /* reset all interrupt status */
    for (i = 0; i < 4; i++)
        buf[i] = 0xFF;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), buf, 4));
    
    /* clear MSGx registers */
    buf[0] = buf[1] = 0x00;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_MSG1), buf, 2));

    retCode = BHAB_31xx_P_DisableInterrupts(handle);
done:
    if (p31xx->hInterruptEvent) {BKNI_DestroyEvent(p31xx->hInterruptEvent);}
    if (p31xx->hApiEvent){BKNI_DestroyEvent(p31xx->hApiEvent);}
    if (p31xx->hInitDoneEvent){BKNI_DestroyEvent(p31xx->hInitDoneEvent);}
    if (p31xx->hHabDoneEvent){BKNI_DestroyEvent(p31xx->hHabDoneEvent);}
    if (p31xx->hMutex){BKNI_DestroyMutex(p31xx->hMutex);}

    BKNI_Free((void*)p31xx);
    BKNI_Free((void*)handle);

    return retCode;
}

/******************************************************************************
 BHAB_31xx_P_ClearAllInterrupts()
******************************************************************************/ 
BERR_Code BHAB_31xx_P_ClearAllInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{   
    BERR_Code err;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    const uint8_t val[4] = {0xFF, 0xFF, 0xFF, 0xFF};
   
     /* reset all interrupt status */
    err = BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), val, 4);
    return err;   
}

/******************************************************************************
 BHAB_31xx_P_InitAp()
******************************************************************************/
BERR_Code BHAB_31xx_InitAp(
    BHAB_Handle handle,       /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM31xx microcode image */
)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint16_t n, addr;
    const uint8_t *pImage;
    uint8_t sb, retries, count=0;

    /* Disable host interrupt */    
    BHAB_CHK_RETCODE(BHAB_31xx_P_EnableHostInterrupt(handle, false));

    /* We are loading AP code, it is appropriate to reset the AP when closing */
    if (pHexImage)
    {
        p31xx->loadAP = true;
    
        /* reset the AP before downloading the microcode */
        BHAB_CHK_RETCODE(BHAB_31xx_Reset(handle));
    
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
                BHAB_CHK_RETCODE(BHAB_31xx_WriteMemory(handle, addr, pImage, n));            
                
                /* check for host transfer error */
                BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));            
                if ((sb & DEVICE(STAT1_H_ER)) == 0)
                    break;
                
                BDBG_WRN(("host transfer error\n"));
                BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));           
            }
    
            if (retries == 3)
                return(BERR_OS_ERROR);
    
            pImage += n;
        }
    
     start_over:               
        /* enable init done interrupt */
        BHAB_CHK_RETCODE(BHAB_31xx_P_EnableInitDoneInterrupt(handle));
    
        /* clear H_STAT1 */
        sb = 0xFF;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));   
    
        /* start running the AP */
        if ((retCode = BHAB_31xx_P_RunAp(handle)) != BERR_SUCCESS)
            goto done;
    
        /* wait for init done interrupt */
        if (BHAB_31xx_P_WaitForEvent(handle, p31xx->hInitDoneEvent, 150) != BERR_SUCCESS)
        {
            /* Disable host interrupt */    
            BHAB_CHK_RETCODE(BHAB_31xx_P_EnableHostInterrupt(handle, false));
        
            /* reset the AP before downloading the microcode */
            BHAB_CHK_RETCODE(BHAB_31xx_Reset(handle));
            BKNI_Sleep(20);
            BDBG_WRN(("AP initialization timeout. Starting over %d times", count)); 
            if(count++ < BHAB_INIT_RETRIES)
                goto start_over;        
            BDBG_MSG(("AP initialization timeout.")); 
            BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);            
        }
    }
    else
    {
        BDBG_MSG(("BHAB_31xx_P_ClearAllInterrupts()"));
        BHAB_CHK_RETCODE(BHAB_31xx_P_ClearAllInterrupts(handle));
        BDBG_MSG(("BHAB_31xx_Reset()"));
        BHAB_CHK_RETCODE(BHAB_31xx_Reset(handle));
        BKNI_Sleep(20);
    }
 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_GetApStatus()
******************************************************************************/
BERR_Code BHAB_31xx_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t buf[4];

    *pStatus = 0;
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), buf, 4));
    *pStatus = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
   
 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_GetApVersion()
******************************************************************************/
BERR_Code BHAB_31xx_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t       *pFamilyId, /* [out] Chip Family id */
    uint32_t       *pChipId,   /* [out] chip id */
    uint16_t       *pChipVer,  /* [out] chip revision number */
    uint8_t        *pMajApVer,    /* [out] AP microcode major version */
    uint8_t        *pMinApVer     /* [out] AP microcode minor version */
)
{
    BERR_Code retCode;
    uint8_t buf[8];

    buf[0] = 0x09;
    BHAB_CHK_RETCODE(BHAB_31xx_SendHabCommand(handle, buf, 1, buf, 8, true, true, 8));

    *pFamilyId = ((buf[1] << 8) | buf[2]);
    *pChipId = *pFamilyId;
    *pChipVer = ((buf[3] << 8) | buf[4]);
    *pMajApVer = buf[5];
    *pMinApVer = 0x0;

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_ReadRegister()
******************************************************************************/
BERR_Code BHAB_31xx_ReadRegister(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    reg,   /* [in] address of register to read */
    uint32_t    *val   /* [in] contains data that was read */
)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t sb;

    if ((reg >= 0x80) && (reg <= 0xFF))
    {
        /* accessing host register space */
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, reg, &sb, 1));      
        *val = (uint32_t)sb;
    }
    else
    {
        /* accessing internal registers through io_mbox */
        BHAB_CHK_RETCODE(BHAB_31xx_ReadMbox(handle, (uint16_t)reg, val));
    }

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_WriteRegister()
******************************************************************************/
BERR_Code BHAB_31xx_WriteRegister(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    reg,   /* [in] address of register to read */
    uint32_t    *val   /* [in] contains data that was read */
)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t sb;

    if ((reg >= 0x80) && (reg <= 0xFF))
    {
        /* accessing host register space */
        sb = (uint8_t)(*val);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, reg, &sb, 1));      
    }
    else
    {
        /* accessing internal registers through io_mbox */
        BHAB_CHK_RETCODE(BHAB_31xx_WriteMbox(handle, (uint16_t)reg, val));
    }

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_ReadMemory()
******************************************************************************/
BERR_Code BHAB_31xx_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{   
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    if (((uint32_t)addr + (uint32_t)n) > BHAB_MEM_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_31xx_P_SetApWindow(handle, BHAB_WINDOW_IRAM + addr));
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, addr & 0x7F, buf, n));   

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_WriteMemory()
******************************************************************************/
BERR_Code BHAB_31xx_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{   
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint32_t  curr_addr, nbytes, bytes_left;

    if ((addr + n) > BHAB_IRAM_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    curr_addr = addr;
    bytes_left = n;
    while (bytes_left > 0)
    {
        BHAB_CHK_RETCODE(BHAB_31xx_P_SetApWindow(handle, BHAB_WINDOW_IRAM + curr_addr));
        nbytes = 128 - (curr_addr % 128);
        if (nbytes > bytes_left)
            nbytes = bytes_left;
        bytes_left -= nbytes;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, curr_addr & 0x7F, buf, nbytes));
        curr_addr += nbytes;
        buf += nbytes;
    }

 done:
    return retCode;
}

/******************************************************************************
 BHAB_31xx_ReadMbox()
******************************************************************************/
BERR_Code BHAB_31xx_ReadMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{   
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t sb, i, buf[4];

    /* update bits 15:8 of mbox address if changed */
    sb = reg >> 8;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_A_15_8), &sb, 1));   
    sb = reg >> 24;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_A_31_24), &sb, 1));   
    sb = reg >> 16;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_A_23_16), &sb, 1));   
    

    /* read from mbox */
    sb = reg & 0xFC;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_CMD), &sb, 1));

    /* check for mbox transfer complete */
    for (i = 0; i < 3; i++)
    {
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_STATUS), &sb, 1));       
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
                BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_D_31_24), buf, 4));
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
 BHAB_31xx_WriteMbox()
******************************************************************************/
BERR_Code BHAB_31xx_WriteMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{   
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t buf[8], sb, i;

    /* write addr, data, and cmd in one i2c transaction */
    buf[0] = reg >> 8;
    buf[1] = *val >> 24;
    buf[2] = *val >> 16;
    buf[3] = *val >> 8;
    buf[4] = *val & 0xFF; 
    buf[5] = (reg & 0xFC) | 0x01;
    buf[6] = reg >> 24;
    buf[7] = reg >> 16; 

    for (i = 0; i < 3; i++)
    {
        /* check for mbox transfer complete */
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_STATUS), &sb, 1));

        if ((sb & 0x80) == 0) {
            if (sb & 0x40) {
                BDBG_ERR(("Before Transfer -  i = %d", i));
                BERR_TRACE(retCode = BHAB_ERR_IOMB_XFER);
                goto done;
            }
            break;          
        }
        else{
            BDBG_ERR(("IO_MBOX operation in progress -  i = %d", i));
        }
    }

    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_A_15_8), buf, 8));

    for (i = 0; i < 3; i++)
    {
        /* check for mbox transfer complete */
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_IO_MBOX_STATUS), &sb, 1));

        if ((sb & 0x80) == 0) {
            if (sb & 0x40) {
                BDBG_ERR(("After Transfer -  i = %d", i));
                BERR_TRACE(retCode = BHAB_ERR_IOMB_XFER);
                goto done;
            }
            break;          
        }
        else{
            BDBG_ERR(("IO_MBOX operation in progress -  i = %d", i));
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
 BHAB_31xx_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_31xx_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_ASSERT(handle);
    handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);
    
    BKNI_SetEvent(p31xx->hApiEvent);   
    BKNI_SetEvent(p31xx->hInterruptEvent);
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_31xx_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_31xx_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
    BERR_Code retCode;
    BDBG_ASSERT(handle);

    BHAB_31xx_P_EnableHostInterrupt(handle, false);
    BHAB_CHK_RETCODE(BHAB_31xx_P_DecodeInterrupt(handle));
    BHAB_31xx_P_EnableHostInterrupt(handle, true);
 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_EnableLockInterrupt()
******************************************************************************/ 
BERR_Code BHAB_31xx_EnableLockInterrupt(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    bool bEnable   /* [in] true = enable Lock  interrupt */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t sb[2], tb;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    
    if( eDevId == BHAB_DevId_eADS0) {
        BHAB_31xx_P_EnableHostInterrupt(handle, false);
            sb[0] = 0;
            sb[1] = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE3), &sb[1], 1));

        if(bEnable) {
            sb[0] = DEVICE(STAT2_LOCK_MASK);
            sb[1] |= (DEVICE(STAT3_GAIN_MASK) | DEVICE(STAT3_QAM_STATUS_READY));
        }
        else {
            sb[0] = 0;
            /*sb[1] &= ~DEVICE(STAT3_GAIN_MASK); ;*/
            
        }
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), sb, 2));         
        BHAB_31xx_P_EnableHostInterrupt(handle, true);
    }
    else if ( eDevId == BHAB_DevId_eAOB0) {
        
        BHAB_31xx_P_EnableHostInterrupt(handle, false);
        tb = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE3), &tb, 1));
        
        if(bEnable) {
            tb |= (DEVICE(STAT3_LOCK_MASK) | DEVICE(STAT3_OOB_STATUS_READY));
        }
        else {
            tb = 0;            
        }

        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE3), &tb, 1));         
        BHAB_31xx_P_EnableHostInterrupt(handle, true);
    }
    
 done:
    return retCode;   
}
    

/******************************************************************************
 BHAB_31xx_InstallInterruptCallback()
******************************************************************************/ 
BERR_Code BHAB_31xx_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1, 
    int parm2
)
{
    BHAB_P_CallbackInfo *callback;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    BDBG_ASSERT(handle);
    
    /*TODO : setup bitmask to enable/disable interrupt callabcks*/

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p31xx->InterruptCallbackInfo[eDevId];

    callback->func = fCallBack;
    callback->pParm1 = pParm1;
    callback->parm2 = parm2;

    return BERR_TRACE(BERR_SUCCESS);
}


/******************************************************************************
 BHAB_31xx_UnInstallInterruptCallback()
******************************************************************************/ 
BERR_Code BHAB_31xx_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
    BHAB_P_CallbackInfo *callback;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    BDBG_ASSERT(handle);
    
    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p31xx->InterruptCallbackInfo[eDevId];

    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->parm2 = (int)NULL;

    return BERR_TRACE(BERR_SUCCESS);
}   
    

/******************************************************************************
 BHAB_31xx_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_31xx_SendHabCommand(
    BHAB_Handle handle, /* [in] BHAB PI Handle */
    uint8_t *write_buf, /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of bytes in the HAB command */ 
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    bool bInsertTermination, /* [in] true = insert termination byte 0x00 in write buffer at read_len position */
    uint16_t command_len
)
{
    BERR_Code retCode;
    uint8_t term_buf;

    BHAB_P_ACQUIRE_MUTEX(handle);
   
    if ((write_len > 127) || (read_len > 127) || (write_len == 0))
        return (BERR_TRACE(BERR_INVALID_PARAMETER));

    BHAB_CHK_RETCODE(BHAB_31xx_P_CheckHab(handle));
  
    /* write the command to the HAB */
    BHAB_CHK_RETCODE(BHAB_31xx_P_WriteHab(handle, 0, write_buf, write_len));

    if(bInsertTermination){
        term_buf = 0x00;
        BHAB_CHK_RETCODE(BHAB_31xx_P_WriteHab(handle, command_len, &term_buf, 1));
    }

    /* wait for the AP to service the HAB, and then read any return data */
    BHAB_CHK_RETCODE(BHAB_31xx_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80));
 
 done:
    BHAB_P_RELEASE_MUTEX(handle);

    return retCode;
}

/******************************************************************************
 BHAB_31xx_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_31xx_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    *hEvent = ((BHAB_31xx_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}



/****************************Private Functions*********************************/




/******************************************************************************
 BHAB_31xx_P_EnableHostInterrupt() 
******************************************************************************/
BERR_Code BHAB_31xx_P_EnableHostInterrupt(
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
 BHAB_31xx_P_DisableInterrupts()
******************************************************************************/ 
BERR_Code BHAB_31xx_P_DisableInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{   
    BERR_Code err;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    const uint8_t val[4] = {0, 0, 0, 0};
   
    /* clear IEx registers */
    err = BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), val, 4);
    return err;   
}


/******************************************************************************
 BHAB_31xx_P_EnableInitDoneInterrupt()
******************************************************************************/ 
BERR_Code BHAB_31xx_P_EnableHabDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{  
    BERR_Code retCode;  
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t sb;
   
    sb = DEVICE(STAT1_HAB_DONE);
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb, 1));
    sb = DEVICE(STAT1_HAB_DONE);
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), &sb, 1));
   
 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_EnableInitDoneInterrupt()
******************************************************************************/ 
BERR_Code BHAB_31xx_P_EnableInitDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{  
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t sb;

    sb = DEVICE(STAT2_INIT_DONE);
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT2), &sb, 1));   
    sb = DEVICE(STAT2_INIT_DONE);
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &sb, 1));
   
 done:
    return retCode;
}

/******************************************************************************
 BERR_31xx_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_31xx_P_WaitForEvent(
    BHAB_Handle handle,             /* [in] BHAB PI Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    int timeoutMsec            /* [in] timeout in milliseconds */
)
{   
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    
    while (1)
    {   
        BHAB_31xx_P_EnableHostInterrupt(handle, true);

        retCode = BKNI_WaitForEvent(p31xx->hApiEvent, timeoutMsec);
        if ( retCode != BERR_SUCCESS ) { break; }

        BHAB_31xx_P_EnableHostInterrupt(handle, false);
        BHAB_31xx_P_DecodeInterrupt(handle);

        retCode = BKNI_WaitForEvent(hEvent, 0);
        if (retCode == BERR_SUCCESS || retCode == BERR_OS_ERROR) { break; }
    }

    BHAB_31xx_P_EnableHostInterrupt(handle, true);

    return retCode;

}


/******************************************************************************
 BHAB_31xx_P_RunAp()
******************************************************************************/
BERR_Code BHAB_31xx_P_RunAp(BHAB_Handle handle)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t   sb, sb2;

    /* check if the AP is currently running */
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));
    if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RUN))
    {
        BHAB_31xx_P_EnableHostInterrupt(handle, true);
        
        /* start running the AP */
        sb2 = DEVICE(AP_RUN);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb2, 1));       
        
        /* verify that the AP is running */
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));     
        if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RUN))
        {
            BDBG_ERR(("unable to run the AP\n"));
            BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
            goto done;
        }
        
        /* clear AP_change state bit */
        sb2 = DEVICE(STAT1_AP_OP_CHG);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), &sb2, 1));   
    }
    else
    {
        BDBG_WRN(("BHAB_P_RunAp(): AP already running\n"));
    }

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_Reset()
******************************************************************************/
BERR_Code BHAB_31xx_Reset(BHAB_Handle handle)
{   
    
    uint8_t i, sb, buf[4];
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint32_t val=0;
    if (p31xx->loadAP)
    {
        buf[0]= 0x00;   /* SH_IND_GLOBAL_RESET1 */
        buf[1]= 0xff;   /* reserved=0xf AP_WDT_RST=1 UPG_WDT_RST=1 UPG_RST=1 PERIPH_RST=1 */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
        
        buf[0]= 0x01;   /* SH_IND_GLOBAL_RESET0 */
        buf[1]= 0xff;   /* OTP_RST=1 TM_RST=1 reserved=1 DS_RST=1 AP_RST=1 reserved=1 reserved=1 CG_RST=1 */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
    }

    buf[0]= 0x02;   /* SH_IND_OSC_CTRL1 */
    buf[1]= 0xa4;   /* IAMP_ADJ=250uA ICBUF_ADG=250uA IBUFD_ADJ=200uA LIMIT_AMP=inactive XTAL_ADJ_CM 1.45V */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x03;   /* SH_IND_OSC_CTRL0 ' enable diffosc clocks */
    buf[1]= 0x40;   /* BYP_DBLR=0 DISABLE_CML6=1 DISABLE_CML5=0 DISABLE_CML4=0 DISABLE_CML3=0 DISABLE_CML2=0 ENABLE_FMON=0 PWRDN_DBLR=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x06;   /* SH_IND_CLK_CNTRL */
    buf[1]= 0x00;   /* AP_CLK_SRC=DIFFOSC_108 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
    
    buf[0]= 0x05;   /* SH_IND_POWER_UP ' power up d2cdiff_54, d2cdiff_108 and tuner */
    buf[1]= 0x07;   /* PWRUP_TUNER=1 PWRUP_108=1 PWRUP_54=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x09;   /* SH_IND_D2CDIFF_CNTRL NO_AC_108=0 NO_AC_54=0*/
    buf[1]= 0x00;   /* NO_AC_108=0 NO_AC_54=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
    
    buf[0]= 0x07;   /* SH_IND_CLK_ENABLE1 ' enable internal register clocks */
    buf[1]= 0xff;   /* reserved=0x3f EN_OTP_9=1 EN_UPG_27=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x08;   /* SH_IND_CLK_ENABLE0 ' enable internal register clocks */
    buf[1]= 0xff;   /* EN_UPG_54=1 EN_PLL_54=1 reserved=0x3 EN_DS_54=1 reserved=0x1 EN_AP_54=1 EN_AP_108=1 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x00;   /* SH_IND_GLOBAL_RESET1 */
    buf[1]= 0x00;   /* AP_WDT_RST=0 UPG_WDT_RST=0 UPG_RST=0 PERIPH_RST=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 

    buf[0]= 0x01;   /* SH_IND_GLOBAL_RESET0 */
    buf[1]= 0x00;   /* OTP_RST=0 TM_RST=0 DS_RST=0 AP_RST=0 CG_RST=0 */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_LOCAL_ADR), buf, 2)); 
    
    if (p31xx->loadAP)
    {
        /* initialize JDEC
           sb = DEVICE(JDEC_RAM); */
        sb = 0x6;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_JDEC), &sb, 1));   
        
        /* reset the AP 
           sb = DEVICE(AP_RESET);*/
        sb = 0x1;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));
        
        /* Hab done command */
        sb = 0x0B;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_AP_CMD), &sb, 1));
    
        /* reset all interrupt status */
        for (i = 0; i < 4; i++)
            buf[i] = 0xFF;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT1), buf, 4));
    
        /* clear MSGx registers */
        buf[0] = buf[1] = 0x00;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_MSG1), buf, 2));
        
        /* verify that AP is reset */
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));   
        if ((sb & DEVICE(AP_MASK)) != DEVICE(AP_RESET))
        {
            BDBG_ERR(("unable to reset the AP\n"));
            BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
        }
    
        val = 0x4e0102;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_DIV1), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_DIV2), &val);
        val = 0x1;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CLK_US), &val);
        val = 0x2703;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CLK_OB), &val);
        val = 0xc03;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CLK_DS), &val);
        val = 0x1207;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CLK_DS_AFE), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_BYP), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_RST), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_MISC), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CTRL_1), &val);
        val = 0x57188058;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_CTRL_0), &val);
        val = 0x2e4;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_PHASE_1), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_PLL_PHASE_0), &val);
        val = 0x4000;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_TOP_BIAS_CTRL), &val);
        val = 0x0;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_ANALOG_CTRL_1), &val); 
        val = 0x131;
        BHAB_31xx_WriteMbox(handle, DEVICE(US_ANALOG_CTRL_0), &val);
    
        val = 0x40000001;
        BHAB_31xx_WriteMbox(handle, DEVICE(TM_TOP_GPIO_MUX_CTRL0), &val);
        val = 0x000;
        BHAB_31xx_WriteMbox(handle, DEVICE(TM_TOP_GPIO_MUX_CTRL1), &val);
        val = 0x5;
        BHAB_31xx_WriteMbox(handle, DEVICE(TIMER_WDCTRL), &val);
        val = 0x3ffffff;
        BHAB_31xx_WriteMbox(handle, DEVICE(TIMER_WDCHIPRST_CNT), &val);
        val = 0x10;
        BHAB_31xx_WriteMbox(handle, DEVICE(TIMER_TIMER_IE0), &val);
    
        val = 0x0;
        BHAB_31xx_ReadMbox(handle, DEVICE(DS_TUNER_ANACTL_WDATA_13_SPARE0), &val);
        val |= (p31xx->slaveChipAddr<< 21);
        val |= 0X20000000;
        BHAB_31xx_WriteMbox(handle, DEVICE(DS_TUNER_ANACTL_WDATA_13_SPARE0), &val);
        
        val = 0xfffffffe;
        BHAB_31xx_WriteMbox(handle, DEVICE(TIMER_WDTIMEOUT), &val);
        BHAB_31xx_ReadMbox(handle, DEVICE(TIMER_WDTIMEOUT), &val);
    }

done:
    p31xx->last_page_16_15 = 0xFF;
    p31xx->last_page_14_7 = 0xFF;

    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_SetApWindow()
******************************************************************************/
BERR_Code BHAB_31xx_P_SetApWindow(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t window   /* [in] base address of the 128-byte window */
)
{   
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t   haddr_16_15 = (window >> 15) & 0x03;
    uint8_t   haddr_14_7 = (window >> 7) & 0xFF;
    uint8_t   buf[2];

    if (p31xx->last_page_16_15 != haddr_16_15)
    {
        buf[0] = haddr_16_15;
        buf[1] = haddr_14_7;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_ADR_16_15), buf, 2));
        p31xx->last_page_16_15 = haddr_16_15;
        p31xx->last_page_14_7 = haddr_14_7;
    }
    else if (p31xx->last_page_14_7 != haddr_14_7)
    {
        BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_SFR_H_ADR_14_7), &haddr_14_7, 1));     
        p31xx->last_page_14_7 = haddr_14_7;
    }

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_31xx_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
   
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    if ((addr & BHAB_HAB_SIZE) || (n & BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_31xx_P_SetApWindow(handle, BHAB_WINDOW_HAB));
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, addr, buf, n));

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_31xx_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    if ((addr & BHAB_HAB_SIZE) || (n & BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_31xx_P_SetApWindow(handle, BHAB_WINDOW_HAB));
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, addr, buf, n));

 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_31xx_P_ServiceHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */ 
    uint8_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{   
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t   sb, ie2, buf[2];
       
    if (BERR_SUCCESS == BKNI_WaitForEvent(p31xx->hHabDoneEvent, 0))
    {
        BDBG_WRN(("Discarded a HabDoneEvent"));   
    }
   
    BHAB_31xx_P_EnableHostInterrupt(handle, false);
   
    /* save ie2 */
    if((retCode = BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &ie2, 1)) != BERR_SUCCESS )
        return retCode;
   
    /* clear ie2 */
    buf[0] = buf[1] = 0;
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), buf, 2));

    /* enable the hab done interrupt mask */    
    BHAB_CHK_RETCODE(BHAB_31xx_P_EnableHabDoneInterrupt(handle));
    
    /* send the command */
    sb = DEVICE(AP_HABR);
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_CTL1), &sb, 1));

    /* wait for HAB done interrupt */  
    if (BHAB_31xx_P_WaitForEvent(handle, p31xx->hHabDoneEvent, 800) == BERR_TIMEOUT)
    {
        BDBG_ERR(("HAB timeout\n"));   
        sb = 0;
        /*BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), &sb, 1); */
        BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
        goto done;
    }

    if (read_len > 0)
    {
        BHAB_CHK_RETCODE(BHAB_31xx_P_ReadHab(handle, 0, read_buf, read_len));
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
    retCode = BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE2), &ie2, 1);
    if(retCode) BERR_TRACE(retCode);

    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_31xx_P_DecodeInterrupt(BHAB_Handle handle)
{  
    BERR_Code retCode;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    uint8_t   h_ie[3], h_fstat[3], new_stat2, new_stat3;
    
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_FSTAT1), h_fstat, 3));
    BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), h_ie, 3));

    new_stat2 = 0;
    new_stat3 = 0;
    if (!h_fstat[0] && !h_fstat[1] && !h_fstat[2])
    {
        return BERR_SUCCESS;
    }
   
    /*HAB DONE INTERRUPT*/
    if (h_fstat[0] & DEVICE(STAT1_HAB_DONE))
    {
        h_ie[0] = 0;
        BKNI_SetEvent(p31xx->hHabDoneEvent);            
    }
   
    /*INIT DONE INTERRUPT*/
    if (h_fstat[1] & DEVICE(STAT2_INIT_DONE))
    {
        h_ie[1] &= ~DEVICE(STAT2_INIT_DONE);
        new_stat2 |= DEVICE(STAT2_INIT_DONE);
        BKNI_SetEvent(p31xx->hInitDoneEvent);
    }

    
    /* Down stream module interrupts */
    if ((h_fstat[1] & DEVICE(STAT2_LOCK_MASK)) ||(h_fstat[2] & ((DEVICE(STAT3_GAIN_MASK)) | (DEVICE(STAT3_QAM_STATUS_READY)))))
    {   
        BHAB_P_CallbackInfo *callback;
        callback = &p31xx->InterruptCallbackInfo[BHAB_DevId_eADS0];
        
        if(callback && callback->func){
            if (h_fstat[2] & DEVICE(STAT3_GET_GAIN))
            {
                new_stat3 |= DEVICE(STAT3_GET_GAIN);
                h_ie[2] &= ~DEVICE(STAT3_GET_GAIN);
                callback->parm2 = (int) BHAB_Interrupt_eUpdateGain;
                BDBG_MSG(("update gain"));
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            if (h_fstat[2] & DEVICE(STAT3_QAM_STATUS_READY))
            {
                new_stat3 |= DEVICE(STAT3_QAM_STATUS_READY);
                callback->parm2 = (int) BHAB_Interrupt_eQamAsyncStatusReady;
                BDBG_MSG(("Qam Async Status Ready."));
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
        
            if((h_fstat[1] & DEVICE(STAT2_LOCK_MASK)) || (h_fstat[2] & DEVICE(STAT3_NO_SIGNAL))){
                callback->parm2 = (int) BHAB_Interrupt_eLockChange;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();

                if(h_fstat[1] & DEVICE(STAT2_LOCK_MASK)){
                    if (h_fstat[1] & DEVICE(STAT2_IN_LOCK))
                    {
                       BDBG_MSG(("locked"));
                       new_stat2 |= DEVICE(STAT2_IN_LOCK);
                    }
                    if(h_fstat[1] & DEVICE(STAT2_OUT_OF_LOCK))
                    {
                       BDBG_MSG(("not locked"));
                       new_stat2 |= DEVICE(STAT2_OUT_OF_LOCK);
                    }
                    h_ie[2] |= DEVICE(STAT3_NO_SIGNAL);
                    h_ie[2] |= DEVICE(STAT3_GET_GAIN);
                }
                else if (h_fstat[2] & DEVICE(STAT3_NO_SIGNAL))
                {
                   BDBG_MSG(("no signal"));
                   new_stat3 |= DEVICE(STAT3_NO_SIGNAL);
                   h_ie[2] &= ~DEVICE(STAT3_NO_SIGNAL);
                }
            }
        }
        
    }

    /* Out of band module interrupts */
    if ((h_fstat[2] & DEVICE(STAT3_LOCK_MASK)) || (h_fstat[2] & DEVICE(STAT3_OOB_STATUS_READY)))
    {   
        BHAB_P_CallbackInfo *callback;
        callback = &p31xx->InterruptCallbackInfo[BHAB_DevId_eAOB0];

        if (callback && callback->func && (h_fstat[2] & DEVICE(STAT3_OOB_STATUS_READY)))
        {
            new_stat3 |= DEVICE(STAT3_OOB_STATUS_READY);
            callback->parm2 = (int) BHAB_Interrupt_eOobAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        
        if (callback && callback->func && (h_fstat[2] & DEVICE(STAT3_LOCK_MASK)))
        {

            new_stat3 |= DEVICE(STAT3_LOCK_MASK);
            h_ie[2] &= ~DEVICE(STAT3_LOCK_MASK);

            callback->parm2 = (int) BHAB_Interrupt_eLockChange; 

            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();

            if (h_fstat[2] & DEVICE(STAT3_IN_LOCK))
            {
                BDBG_MSG(("locked"));
                new_stat3 &= ~DEVICE(STAT3_OUT_OF_LOCK);
                h_ie[2] |= DEVICE(STAT3_OUT_OF_LOCK);

            }
            else
            {
                new_stat3 &= ~DEVICE(STAT3_IN_LOCK);
                h_ie[2] |= DEVICE(STAT3_IN_LOCK);
                BDBG_MSG(("not locked"));  
            }
        }
    }    

    /* clear the interrupt status */
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_IE1), h_ie, 3)); 
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT2), &new_stat2, 1));
    BHAB_CHK_RETCODE(BREG_I2C_Write(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_STAT3), &new_stat3, 1));
    
 done:
    return retCode;
}


/******************************************************************************
 BHAB_31xx_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_31xx_P_DecodeError(
    BHAB_Handle handle,           /* [in] BHAB PI Handle */
    BHAB_ApStatus *pApStatus /* [in] AP status returned by BHAB_GetApStatus */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
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
        BHAB_CHK_RETCODE(BREG_I2C_Read(p31xx->hRegister, handle->settings.chipAddr, DEVICE(SH_AP_SFR_H_MSG1), &sb, 1));     
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
 BHAB_31xx_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_31xx_P_CheckHab(
    BHAB_Handle handle    /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_ApStatus status = 0;

    BHAB_CHK_RETCODE(BHAB_31xx_GetApStatus(handle, &status));
    if ((status & BHAB_APSTATUS_HAB_MASK) == BHAB_APSTATUS_HAB_READY) {
        retCode = BERR_SUCCESS;
    }
    else
    {
        BERR_TRACE(retCode = BHAB_31xx_P_DecodeError(handle, &status));
    }
    
 done:
    return retCode;
}

/******************************************************************************
 BHAB_31xx_GetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_31xx_GetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_WatchDogTimerSettings *wdtSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    
    *wdtSettings = p31xx->wdtSettings;
    
     return retCode;
}

/******************************************************************************
 BHAB_31xx_SetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_31xx_SetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_WatchDogTimerSettings *wdtSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[11];
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    
    BKNI_Memset(buf, 0x00, sizeof(buf));
    
    buf[0] = 0x1B;

    if(wdtSettings->clearWatchDog.select < 0xf)
        buf[1] = wdtSettings->clearWatchDog.select;
    else
        BDBG_ERR(("unknown gpio selection.\n"));
    
    buf[1] |= (wdtSettings->clearWatchDog.enable << 4);
    buf[1] |= (wdtSettings->clearWatchDog.polarity << 5);
    buf[2] |= (wdtSettings->start << 2);
    buf[2] |= (wdtSettings->nmiMode << 1);
    buf[2] |= wdtSettings->oneShot;
    buf[3] = wdtSettings->timeout >> 24;
    buf[4] = wdtSettings->timeout >> 16;
    buf[5] = wdtSettings->timeout >> 8;
    buf[6] = wdtSettings->timeout;

    /* NMI width is hardcoded. It can be made into a parameter if needed. */
    buf[7] = 0x0;
    buf[8] = 0x52;
    buf[9] = 0x65;
    buf[10] = 0xc0;

    BHAB_CHK_RETCODE(BHAB_31xx_SendHabCommand(handle, buf, 11, buf, 1, true, true, 11));
    
    p31xx->wdtSettings = *wdtSettings;
    
 done:
    return retCode;
}

/******************************************************************************
 BHAB_31xx_GetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_31xx_GetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_NmiSettings *nmiSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    
    *nmiSettings = p31xx->nmiSettings;
    
     return retCode;
}

/******************************************************************************
 BHAB_31xx_SetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_31xx_SetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_NmiSettings *nmiSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[6];
    BHAB_31xx_P_Handle *p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);

    BKNI_Memset(buf, 0x00, sizeof(buf));
    
    buf[0] = 0x1a;

    if(nmiSettings->nmi.select < 0xf)
        buf[1] = nmiSettings->nmi.select;
    else
        BDBG_ERR(("unknown gpio selection.\n"));

    if(nmiSettings->powerDownDetect.select < 0xf)
        buf[2] = nmiSettings->powerDownDetect.select;
    else
        BDBG_ERR(("unknown gpio selection.\n"));
    
    if(nmiSettings->spare1.select < 0xf)
        buf[3] = nmiSettings->spare1.select;
    else
        BDBG_ERR(("unknown gpio selection.\n"));
    
    if(nmiSettings->spare2.select < 0xf)
        buf[4] = nmiSettings->spare2.select;
    else
        BDBG_ERR(("unknown gpio selection.\n"));
    
    buf[1] |= (nmiSettings->nmi.enable << 4);
    buf[2] |= (nmiSettings->powerDownDetect.enable << 4);
    buf[3] |= (nmiSettings->spare1.enable << 4);
    buf[4] |= (nmiSettings->spare2.enable << 4);
    
    buf[3] |= (nmiSettings->powerDownDetect.polarity << 5);
    buf[4] |= (nmiSettings->spare1.polarity << 5);
    buf[5] |= (nmiSettings->spare2.polarity << 5);

    BHAB_CHK_RETCODE(BHAB_31xx_SendHabCommand(handle, buf, 5, buf, 1, true, true, 5));      

    p31xx->nmiSettings = *nmiSettings;
    
 done:
    return retCode;
}


/***************************************************************************
 BHAB_31xx_GetConfigSettings()
****************************************************************************/
BERR_Code BHAB_31xx_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx;
    BDBG_ASSERT(handle);

    p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p31xx);

    settings->daisyChain  = p31xx->isDaisyChain;

    return retCode;
}

/***************************************************************************
 BHAB_31xx_SetConfigSettings()
****************************************************************************/
BERR_Code BHAB_31xx_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_31xx_P_Handle *p31xx;
    uint8_t   hab[2];
    BDBG_ASSERT(handle);

    p31xx = (BHAB_31xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p31xx);

    if(settings->daisyChain)
        hab[0] = 0x22;
    else
        hab[0] = 0x21;
    hab[1] = 0x02;

    BHAB_CHK_RETCODE(BHAB_31xx_SendHabCommand(handle, hab, 2, hab, 1, true, true, 2));

    if(settings->daisyChain)
        p31xx->isDaisyChain = true;
    else
        p31xx->isDaisyChain = false;

done:
    return retCode;
}

