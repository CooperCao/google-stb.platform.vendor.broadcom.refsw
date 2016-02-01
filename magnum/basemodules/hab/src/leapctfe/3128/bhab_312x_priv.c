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
#include "bhab_312x_priv.h"
#include "bhab_3128.h"
#include "bchp_hsi.h"
#include "../../c0/bchp_leap_hab_mem.h"
#include "../../c0/bchp_tm.h"
#include "../../c0/bchp_leap_l2.h"

#ifndef BHAB_312X_VER
#error "BHAB_312X_VER needs to be defined"
#endif

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
#define BHAB_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))
#else
#define BHAB_P_SWAP32(a) a
#endif

/* define BHAB_DEBUG */
#define MAX_HAB_RETRIES 3

BDBG_MODULE(bhab_312x_priv);

static uint8_t BHAB_312x_P_CRC8Block(uint8_t crc, uint8_t *pDataBlock, uint32_t dataLength);

/******************************************************************************
 BHAB_312x_Open()
******************************************************************************/
BERR_Code BHAB_312x_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_Handle hDev;
    BHAB_312x_P_Handle *h312xDev = NULL;
    unsigned i;

    hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));

    if(!hDev)
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    else
    {
        h312xDev = (BHAB_312x_P_Handle *)BKNI_Malloc(sizeof(BHAB_312x_P_Handle));
        if(h312xDev)
        {
            BKNI_Memset( h312xDev, 0x00, sizeof(BHAB_312x_P_Handle));
            hDev->pImpl = (void*)h312xDev;

            if(pDefSettings->isSpi) {
               h312xDev->hSpiRegister = (BREG_SPI_Handle)pReg;
            }
            else {
               h312xDev->hI2cRegister = (BREG_I2C_Handle)pReg;
            }

            BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));
            hDev->channelCapabilities = NULL;

            /* create events */
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hInterruptEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hApiEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hInitDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hHabDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hHabReady)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h312xDev->hTnrStatusReady)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h312xDev->hMutex)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h312xDev->hRegAccessMutex)));

            for(i=0; i<BHAB_DevId_eMax; i++){
                h312xDev->InterruptCallbackInfo[i].func = NULL;
                h312xDev->InterruptCallbackInfo[i].pParm1 = NULL;
                h312xDev->InterruptCallbackInfo[i].parm2 = (int)NULL;
            }

            BKNI_Memset( &h312xDev->nmiSettings, 0x00, sizeof(BHAB_NmiSettings));
            BKNI_Memset( &h312xDev->wdtSettings, 0x00, sizeof(BHAB_WatchDogTimerSettings));

            h312xDev->isSpi = pDefSettings->isSpi;
            h312xDev->isMtsif = pDefSettings->isMtsif;

            BHAB_3128_GetDefaultRecalibrateSettings(&h312xDev->recalibrateSettings);

            retCode = BHAB_312x_P_DisableInterrupts(hDev);
        }
        else
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
done:
    if(retCode == BERR_SUCCESS)
        *handle = hDev;
    else
    {
        if(h312xDev)
        {
            BKNI_Free(h312xDev);
            h312xDev = NULL;
        }
        if(hDev)
        {
            BKNI_Free(hDev);
            hDev = NULL;
        }
        *handle = NULL;
    }

    return retCode;
}


/******************************************************************************
 BHAB_312x_Close()
******************************************************************************/
BERR_Code BHAB_312x_Close(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint32_t sb, buf;

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    /* If we didn't load the AP when starting, don't reset it when closing */
    if (p312x->loadAP)
    {
        /* reset all interrupt status */
        buf = 0xFFFFFFFF;
        BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
        BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR1, &buf));
        /* reset the AP */
        sb = 0x01;
        retCode = BHAB_312x_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &sb);

        if(retCode)
            BDBG_WRN(("Ap not in reset."));
    }

    retCode = BHAB_312x_P_DisableInterrupts(handle);
done:
    if (p312x->hInterruptEvent) {BKNI_DestroyEvent(p312x->hInterruptEvent);}
    if (p312x->hApiEvent){BKNI_DestroyEvent(p312x->hApiEvent);}
    if (p312x->hInitDoneEvent){BKNI_DestroyEvent(p312x->hInitDoneEvent);}
    if (p312x->hHabDoneEvent){BKNI_DestroyEvent(p312x->hHabDoneEvent);}
    if (p312x->hHabReady){BKNI_DestroyEvent(p312x->hHabReady);}
    if (p312x->hTnrStatusReady){BKNI_DestroyEvent(p312x->hTnrStatusReady);}
    if (p312x->hMutex){BKNI_DestroyMutex(p312x->hMutex);}
    if (p312x->hRegAccessMutex){BKNI_DestroyMutex(p312x->hRegAccessMutex);}

    BKNI_Free((void*)p312x);
    if(handle->channelCapabilities != NULL)
        BKNI_Free((void*)handle->channelCapabilities);
    BKNI_Free((void*)handle);

    return retCode;
}

/******************************************************************************
 BHAB_312x_P_DumpRegisters()
******************************************************************************/
BERR_Code BHAB_312x_P_DumpRegisters(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t i, buf=0;

    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS1, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT7, &buf));
    BDBG_WRN(("BCHP_TM_SFT7 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT6, &buf));
    BDBG_WRN(("BCHP_TM_SFT6 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT5, &buf));
    BDBG_WRN(("BCHP_TM_SFT5 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT4, &buf));
    BDBG_WRN(("BCHP_TM_SFT4 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT3, &buf));
    BDBG_WRN(("BCHP_TM_SFT3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT2, &buf));
    BDBG_WRN(("BCHP_TM_SFT2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT1, &buf));
    BDBG_WRN(("BCHP_TM_SFT1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_SFT0, &buf));
    BDBG_WRN(("BCHP_TM_SFT0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_MISC0, &buf));
    BDBG_WRN(("BCHP_TM_MISC0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_MISC1, &buf));
    BDBG_WRN(("BCHP_TM_MISC1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_MISC2, &buf));
    BDBG_WRN(("BCHP_TM_MISC2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_MISC3, &buf));
    BDBG_WRN(("BCHP_TM_MISC3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, 0x78000, &buf));
    BDBG_WRN(("HAB first word %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, 0x78004, &buf));
    BDBG_WRN(("HAB second word %x", buf));
#ifdef BHAB_DEBUG
    for(i=0; i < 20; i++) {
#else
    for(i=0; i < 1; i++) {
#endif
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE0 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE1, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE1 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE2, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE2 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE3, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE3 %x", buf));
    BKNI_Sleep(250);
    }

    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS1, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));

done:
    return retCode;
}

/******************************************************************************
 BHAB_312x_P_ErrorRecovery()
******************************************************************************/
BERR_Code BHAB_312x_P_ErrorRecovery(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t val=0;

    BHAB_312x_P_Handle *p312x;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    /* Clear the "HAB Done" bit before initiating "HAB Reset" */
    val = BHAB_HAB_DONE;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &val));
    val = BHAB_HAB_RESET;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_L2_CPU_SET, &val));
    if (BHAB_312x_P_WaitForEvent(handle, p312x->hHabReady, 1500) == BERR_TIMEOUT)
    {
        BDBG_ERR(("########################## Unable to reset HAB\n"));
        retCode = BHAB_ERR_HAB_TIMEOUT;
    }

done:
    return retCode;
}

/******************************************************************************
 BHAB_312x_P_InitAp()
******************************************************************************/
BERR_Code BHAB_312x_InitAp(
    BHAB_Handle handle,       /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM312x microcode image */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint32_t n, instlen, datalen, instaddr, dataaddr;
    const uint8_t *pImage;
    uint8_t retries=0, count=0, buf=0, writebuf[3];

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    /* We are loading AP code, it is appropriate to reset the AP when closing */
    p312x->loadAP = true;

    /* Disable host interrupt */
    BHAB_CHK_RETCODE(BHAB_312x_P_EnableHostInterrupt(handle, false));

    if(p312x->isSpi){
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = HIF_OSC_STRAP_OVRD_XCORE_BIAS;
        writebuf[2] = 0x1C;
        BKNI_Sleep(5);
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 3));
        writebuf[2] = 0x14;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 3));
    }
    else {
        buf = 0x1C;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, HIF_OSC_STRAP_OVRD_XCORE_BIAS, (uint8_t *)&buf, 1));
        BKNI_Sleep(5);
        buf = 0x14;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, HIF_OSC_STRAP_OVRD_XCORE_BIAS, (uint8_t *)&buf, 1));
    }
    BKNI_Sleep(5);

    /* reset the AP before downloading the microcode */
    BHAB_CHK_RETCODE(BHAB_312x_Reset(handle));

    if (pHexImage)
    {
        /* download to RAM */
        pImage = pHexImage;

        instlen = (pImage[10] << 16) | (pImage[11] << 8) | pImage[12];
        datalen = (pImage[16] << 16) | (pImage[17] << 8) | pImage[18];
        n = instlen + datalen;
        if (n != 0)
        {
            instaddr = (pImage[7] << 16) | (pImage[8] << 8) | pImage[9];
            dataaddr = (pImage[13] << 16) | (pImage[14] << 8) | pImage[15];

            for (retries = 0; retries < 3; retries++)
            {
                retCode = BHAB_312x_WriteMemory(handle, instaddr, &pImage[19], instlen);
                if (retCode == BERR_SUCCESS)
                    break;
                else
                    BDBG_WRN(("BHAB_312x_WriteMemory() retCode 0x%X", retCode));
            }
        }

    }

    if (retries == 3)
    {
        BDBG_ERR(("unable to download FW"));
        BERR_TRACE(retCode = BHAB_ERR_HOST_XFER);
    }
    else
    {
        /* enable init done interrupt */
        BHAB_CHK_RETCODE(BHAB_312x_P_EnableInitDoneInterrupt(handle));

        /* start running the AP */
        if ((retCode = BHAB_312x_P_RunAp(handle)) == BERR_SUCCESS)
        {
            for (count = 0; count < BHAB_INIT_RETRIES; count++)
            {
                /* wait for init done interrupt */
                if ((retCode = BHAB_312x_P_WaitForEvent(handle, p312x->hInitDoneEvent, 150)) == BERR_SUCCESS)
                    break;
                /* Disable host interrupt */
                if ((retCode = BHAB_312x_P_EnableHostInterrupt(handle, false)) != BERR_SUCCESS)
                    BDBG_WRN(("Failed disabling Host Interrupt"));
            }

            if (count < BHAB_INIT_RETRIES)
                retCode = BERR_SUCCESS;
            else
            {
                /* reset the AP before exiting */
                BHAB_CHK_RETCODE(BHAB_312x_Reset(handle));
                BKNI_Sleep(10);
                BDBG_ERR(("AP initialization timeout. Starting over %d times", count));
                BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
            }
        }
    }

done:

    if(retCode != BERR_SUCCESS)
        BHAB_312x_P_EnableHostInterrupt(handle, true);
    return retCode;
}


/******************************************************************************
 BHAB_312x_GetApStatus()
******************************************************************************/
BERR_Code BHAB_312x_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t sb;

    BDBG_ASSERT(handle);

    *pStatus = 0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb));
    *pStatus = sb;

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_GetApVersion()
******************************************************************************/
BERR_Code BHAB_312x_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    *pFamilyId, /* [out] Chip Family id */
    uint32_t    *pChipId,   /* [out] BHAB chip ID */
    uint16_t    *pChipVer,  /* [out] chip revision number */
    uint8_t     *pMajApVer, /* [out] AP microcode major version */
    uint8_t     *pMinApVer  /* [out] AP microcode minor version */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[21];
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_FAMILY_ID, pFamilyId));

    buf[0] = ((BHAB_GETVERSION >> 2) & 0xFF);
    buf[1] = (((BHAB_GETVERSION & 0x3) << 6) | ((PAYLOAD_LEN >> 4) & 0x3F));
    buf[2] = ((PAYLOAD_LEN & 0xF) << 4);
    buf[3] = BHAB_CORE_ID;
    buf[4] = 0;    /* CRC */
    BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf, 5, p312x->inBuf, 21, false, true, 21));
    *pChipId = ((p312x->inBuf[4] << 8) | p312x->inBuf[5]);
    *pChipVer = ((p312x->inBuf[6] << 8) | p312x->inBuf[7]);
    *pMajApVer = p312x->inBuf[8];
    *pMinApVer = p312x->inBuf[9];

done:
    if(retCode != BERR_SUCCESS)
    {
        *pChipId = 0;
        *pChipVer = 0;
        *pMajApVer = 0;
        *pMinApVer = 0;
    }

    return retCode;
}


/******************************************************************************
 BHAB_3461_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_312x_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[21] = HAB_MSG_HDR(BHAB_GETVERSION, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t sysVersion[73] = HAB_MSG_HDR(BHAB_GET_SYS_VERSION, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_FAMILY_ID, &pVersionInfo->familyId));

    BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf, 5, p312x->inBuf, 21, false, true, 21));
    pVersionInfo->chipId = ((p312x->inBuf[4] << 8) | p312x->inBuf[5]);
    pVersionInfo->chipVersion = ((p312x->inBuf[6] << 8) | p312x->inBuf[7]);
    pVersionInfo->firmware.majorVersion = p312x->inBuf[8];
    pVersionInfo->firmware.minorVersion = p312x->inBuf[9];

    if((pVersionInfo->firmware.majorVersion >= 6) || ((pVersionInfo->firmware.majorVersion >= 5) & ( pVersionInfo->firmware.minorVersion>= 7)))
    {
        BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, sysVersion, 5, p312x->inBuf, 73, false, true, 73));
        pVersionInfo->familyId = (p312x->inBuf[4] << 24) | (p312x->inBuf[5] << 16) | (p312x->inBuf[6] << 8) | p312x->inBuf[7];
        pVersionInfo->chipId = (p312x->inBuf[8] << 24) | (p312x->inBuf[9] << 16) | (p312x->inBuf[10] << 8) | p312x->inBuf[11];
        pVersionInfo->chipVersion = (p312x->inBuf[12] << 24) | (p312x->inBuf[13] << 16) | (p312x->inBuf[14] << 8) | p312x->inBuf[15];
        pVersionInfo->firmware.majorVersion = ((p312x->inBuf[16] << 8) | p312x->inBuf[17]);
        pVersionInfo->firmware.minorVersion = ((p312x->inBuf[18] << 8) | p312x->inBuf[19]);
        pVersionInfo->firmware.buildType = ((p312x->inBuf[20] << 8) | p312x->inBuf[21]);
        pVersionInfo->firmware.buildId = ((p312x->inBuf[22] << 8) | p312x->inBuf[23]);
        pVersionInfo->bondoutOptions[0] = (p312x->inBuf[40] << 24) | (p312x->inBuf[41] << 16) | (p312x->inBuf[42] << 8) | p312x->inBuf[43];
        pVersionInfo->bondoutOptions[1] = (p312x->inBuf[44] << 24) | (p312x->inBuf[45] << 16) | (p312x->inBuf[46] << 8) | p312x->inBuf[47];
    }
    else
    {
        pVersionInfo->firmware.buildType = 0;
        pVersionInfo->firmware.buildId = 0;
        pVersionInfo->bondoutOptions[0] = 0;
        pVersionInfo->bondoutOptions[1] = 0;
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_ReadMemory()
******************************************************************************/
BERR_Code BHAB_312x_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint8_t sb, data, readbuf[8], writebuf[8];
    uint32_t sb1;
    uint16_t bytes_left, offset;
    uint8_t i=0;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);

    if (((uint32_t)addr + (uint32_t)n) > (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    sb1 = BHAB_P_SWAP32(addr);
    if(p312x->isSpi){
        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 6));

        writebuf[0] = (handle->settings.chipAddr << 1);
        for (offset = 0; offset < n/4 * 4; offset+=4) {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister, writebuf, readbuf, 6));
            *buf++ = readbuf[2];
            *buf++ = readbuf[3];
            *buf++ = readbuf[4];
            *buf++ = readbuf[5];

            for(i=0; i < 5; i++){
                writebuf[1] = CSR_STATUS;
                BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister, writebuf, readbuf, 3));
                if (readbuf[2] == BHAB_CPU_RUNNIG)
                    break;
            }
            if(i==5)
                BDBG_WRN(("Write transaction not finished"));
        }

        /* Read the bytes left */
        bytes_left = n%4;
        if(bytes_left)
        {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister, writebuf, readbuf, 2+bytes_left));
            for(i=0; i<bytes_left; i++) {
                *buf++ = readbuf[2+i];
            }
        }

        /* set it back to the default write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 3));
    }
    else {
        /* enable read mode and speculative read*/
        data = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        for (offset = 0; offset < n; offset+=4) {
            BHAB_CHK_RETCODE(BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf+offset,
                n - offset < 4 ? n - offset : 4));
            for(i=0; i < 5; i++){
                BHAB_CHK_RETCODE(BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
                if(sb == BHAB_CPU_RUNNIG)
                    break;
            }
            if(i==5)
                BDBG_WRN(("Write transaction not finished"));
        }

        data =0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_WriteMemory()
******************************************************************************/
BERR_Code BHAB_312x_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint8_t pad, i=0, k=0;
    uint16_t bytes_left;
    uint32_t sb1, curr_addr;
    uint8_t readbuf[8], writebuf[32];

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    sb1 = BHAB_P_SWAP32(addr);

    if ((addr + n) >= (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE +128))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if(p312x->isSpi)
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 3));

        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 6));

        writebuf[1] = CSR_RBUS_DATA0;

        BREG_SPI_SetContinueAfterCommand(p312x->hSpiRegister , true);
        BHAB_CHK_RETCODE(BREG_SPI_WriteAll(p312x->hSpiRegister, writebuf, 2));

        BHAB_CHK_RETCODE(BREG_SPI_WriteAll(p312x->hSpiRegister, buf, n));
        BREG_SPI_SetContinueAfterCommand(p312x->hSpiRegister , false);

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        bytes_left = (n%4);

        if(bytes_left)
        {
            bytes_left = 4-bytes_left;
            writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;

            switch (n%4)
            {
                case 0:
                default:
                    writebuf[1] = CSR_RBUS_DATA0;
                  break;
                case 1:
                    writebuf[1] = CSR_RBUS_DATA1;
                    break;
                case 2:
                    writebuf[1] = CSR_RBUS_DATA2;
                    break;
                case 3:
                    writebuf[1] = CSR_RBUS_DATA3;
                    break;
            }

            for(k=0; k<bytes_left; k++)
            {
                writebuf[2+k] = 0;
            }
            BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 2+bytes_left));
        }

        /* check for host transfer error */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* set CSR pointer to point to DATA0 and provide the data to be downloaded */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, n));

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        bytes_left = n%4;
        if(bytes_left)
        {
            pad = 0;
            curr_addr = CSR_RBUS_DATA0 + bytes_left;
            for(i=0; i<(4-bytes_left); i++)
            {
                BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, curr_addr++, &pad, 1));
            }
        }
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_ReadMbox()
******************************************************************************/
BERR_Code BHAB_312x_ReadMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(reg);
    BSTD_UNUSED(val);

    return retCode;
}


/******************************************************************************
 BHAB_312x_WriteMbox()
******************************************************************************/
BERR_Code BHAB_312x_WriteMbox(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(reg);
    BSTD_UNUSED(val);

    return retCode;
}


/******************************************************************************
 BHAB_312x_ReadRegister()
******************************************************************************/
BERR_Code BHAB_312x_ReadRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint8_t data, sb, i=0;
    uint32_t sb1, buf, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    sb1 = BHAB_P_SWAP32(reg);

    if(p312x->isSpi)
    {
        readbuf[2] = 0;

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister, writebuf, 3));

        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |=  ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister,  writebuf, readbuf, 3));
             if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished"));

        /* read data*/
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_RBUS_DATA0;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister, writebuf, readbuf, 6));

        value = ((uint32_t)(readbuf[2] << 0) | (uint32_t)(readbuf[3] << 8) | (uint32_t)(readbuf[4] << 16) | (uint32_t)(readbuf[5] << 24));
        *val = BHAB_P_SWAP32(value);

        /* set READ_RBUS to the reset value for write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p312x->hSpiRegister,  writebuf, 3));
    }
    else
    {
        /* set READ_RBUS for read mode */
        data = ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            retCode = BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1);
            if ((sb & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished\n"));

        /* read the data */
        BHAB_CHK_RETCODE(BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&buf, 4));
        *val = (uint32_t)BHAB_P_SWAP32(buf);

        /* set READ_RBUS to the reset value for write mode */
        data = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_312x_WriteRegister()
******************************************************************************/
BERR_Code BHAB_312x_WriteRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    uint8_t sb, buf[4], i=0;
    uint32_t sb1, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);
    BSTD_UNUSED(reg);
    sb1 = BHAB_P_SWAP32(reg);
    if(p312x->isSpi)
    {
        /* enable write mode */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read (p312x->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |= (CSR_CONFIG_READ_RBUS_WRITE << CSR_CONFIG_READ_RBUS_SHIFT);
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 6));

        /* write to RBUS*/
        writebuf[1] = CSR_RBUS_DATA0;
        value = BHAB_P_SWAP32(*(uint32_t *)val);
        writebuf[2] = (value & 0xff) >> 0;
        writebuf[3] = (value & 0xff00) >> 8;
        writebuf[4] = (value & 0xff0000) >> 16;
        writebuf[5] = (value & 0xff000000) >> 24;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p312x->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else {
        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        sb1 = BHAB_P_SWAP32(*(uint32_t *)val);
        /* write to RBUS*/
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&sb1, 4));

        BHAB_CHK_RETCODE(BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            retCode = BREG_I2C_P_Read312x(p312x->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1);
            if ((sb & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished\n"));

    }

done:
    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_312x_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_312x_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
    BHAB_312x_P_Handle *p312x;
    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if(handle->settings.interruptEnableFunc)
        handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);

    BKNI_SetEvent_isr(p312x->hApiEvent);
    BKNI_SetEvent_isr(p312x->hInterruptEvent);
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_312x_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_312x_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(handle);

    BHAB_P_ACQUIRE_MUTEX(handle);
    BHAB_312x_P_EnableHostInterrupt(handle, false);
    BHAB_CHK_RETCODE(BHAB_312x_P_DecodeInterrupt(handle));
    BHAB_312x_P_EnableHostInterrupt(handle, true);

done:
    if(retCode != BERR_SUCCESS)
        BHAB_312x_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_312x_EnableLockInterrupt()
******************************************************************************/
BERR_Code BHAB_312x_EnableLockInterrupt(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    bool bEnable   /* [in] true = enable Lock  interrupt */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(eDevId);
    BSTD_UNUSED(bEnable);

    return retCode;
}


/******************************************************************************
 BHAB_312x_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_312x_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p312x->InterruptCallbackInfo[eDevId];

    BKNI_EnterCriticalSection();
    callback->pParm1 = pParm1;
    callback->parm2 = parm2;
    callback->func = fCallBack;
    BKNI_LeaveCriticalSection();

    switch (eDevId) {
    case BHAB_DevId_eGlobal:
        switch( parm2 )
        {
            case BHAB_Interrupt_eCppmPowerLevelChange:
                p312x->powerLevelChangeCallback = fCallBack;
                break;
            case BHAB_Interrupt_eCalibrationComplete:
                p312x->calibrationCompleteCallback = fCallBack;
                break;
            default:
                break;
        }
        break;
    default:
        switch(parm2)
        {
            case BHAB_Callback_eLockChange:
                callback->callbackType |= BHAB_LOCK_CHANGE_MASK;
                break;
            case BHAB_Callback_eUpdateGain:
                callback->callbackType |= BHAB_UPDATE_GAIN_MASK;
                break;
            case BHAB_Callback_eNoSignal:
                callback->callbackType |= BHAB_NO_SIGNAL_MASK;
                break;
            case BHAB_Callback_eAsyncStatusReady:
                callback->callbackType |= BHAB_STATUS_RDY_MASK;
                break;
            case BHAB_Callback_eSpectrumDataReady:
                callback->callbackType |= BHAB_SPECTRUM_DATA_MASK;
                break;
            default:
                break;
        }
        break;
    }

    return retCode;
}


/******************************************************************************
 BHAB_312x_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_312x_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p312x->InterruptCallbackInfo[eDevId];


    BKNI_EnterCriticalSection();
    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->parm2 = (int)NULL;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_312x_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_312x_SendHabCommand(
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
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t sb, datalength = 0;
    uint8_t crc = 0, retries=0;
    BSTD_UNUSED(command_len);

    BDBG_ASSERT(handle);

    BHAB_P_ACQUIRE_MUTEX(handle);

    if ((write_len > BHAB_MEM_SIZE) || (read_len > BHAB_MEM_SIZE) || (write_len == 0)){
        retCode = BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    BHAB_312x_P_EnableHostInterrupt(handle, false);

    *(write_buf+write_len-1) = BHAB_312x_P_CRC8Block(crc, write_buf, write_len-1);
    for(retries=0; retries<MAX_HAB_RETRIES; retries++) {
        if(retries)
            BDBG_WRN(("Resending HAB command"));
        /* write the command to the HAB */
        BHAB_CHK_RETCODE(BHAB_312x_P_WriteHab(handle, 0, write_buf, write_len));
        if(bInsertTermination){
            sb=0x1;
            BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_CTRL_HAB_REQUEST, &sb));
        }
        /* wait for the AP to service the HAB, and then read any return data */
        retCode = BHAB_312x_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80);

        if(retCode == BHAB_ERR_HAB_TIMEOUT) {
                BDBG_WRN(("HAB Timeout detected, resetting HAB..."));
                BHAB_312x_P_ErrorRecovery(handle);
                continue;
        }
        else
            goto read_hab;

read_hab:
        crc = 0;
        datalength = (((*(read_buf+1) & 0x3F) << 4) | (*(read_buf+2) >> 4) ) + 4;

        if((read_len != 0) && (*(read_buf+datalength) != BHAB_312x_P_CRC8Block(crc, read_buf, datalength))) {
            BDBG_WRN(("Invalid CRC"));
            retCode = BHAB_ERR_INVALID_CRC;
            continue;
        }

        retCode = BHAB_312x_P_CheckHab(handle);
        if(retCode == BERR_SUCCESS)
            break;
    }

    if((retries==MAX_HAB_RETRIES) && (retCode == BHAB_ERR_HAB_TIMEOUT)){
        BDBG_ERR(("HAB timeout\n"));
        BDBG_WRN(("Dumping 312x Registers\n"));
        BHAB_312x_P_DumpRegisters(handle);
#ifdef BHAB_DEBUG
        BDBG_ASSERT(false);
#endif
    }
done:
    if(retCode != BERR_SUCCESS)
        BHAB_312x_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);

    return retCode;
}

/******************************************************************************
 BHAB_312x_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_312x_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    BDBG_ASSERT(handle);

    *hEvent = ((BHAB_312x_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}


/****************************Private Functions*********************************/

static uint8_t BHAB_312x_P_CRC8Byte(
    uint8_t inCRC,
    uint8_t inData
    )
{
    int i;
    unsigned short data;

    data = inCRC ^ inData;
    data <<= 8;

    for ( i = 0; i < 8; i++ )
    {
        if (( data & 0x8000 ) != 0 )
        {
            data = data ^ (0x1070U << 3);
        }
        data = data << 1;
    }

    return (uint8_t)(data >> 8 );
}

static uint8_t BHAB_312x_P_CRC8Block(
    uint8_t crc,
    uint8_t *pDataBlock,
    uint32_t dataLength
    )
{
    uint32_t i;

    for ( i = 0; i < dataLength; i++ )
    {
        crc = BHAB_312x_P_CRC8Byte(crc, *pDataBlock++ );
    }

    return crc;
}

/******************************************************************************
 BREG_I2C_P_Read312x()
******************************************************************************/
BERR_Code BREG_I2C_P_Read312x(
    BREG_I2C_Handle i2cHandle,    /* [in] BREG_I2C Handle */
    uint16_t chipAddr,
    uint8_t subAddr,
    uint8_t *pData,
    size_t length
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(i2cHandle);

    BHAB_CHK_RETCODE(BREG_I2C_Read(i2cHandle, chipAddr, subAddr, pData, length));

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_312x_P_EnableHostInterrupt(
    BHAB_Handle handle, /* [in] HAB handle */
    bool bEnable        /* [in] true=enables the L1 interrupt on the host processor */
)
{
    BDBG_ASSERT(handle);

    BKNI_EnterCriticalSection();
    if(handle->settings.interruptEnableFunc)
        handle->settings.interruptEnableFunc(bEnable, handle->settings.interruptEnableFuncParam);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_312x_P_DisableInterrupts()
******************************************************************************/
BERR_Code BHAB_312x_P_DisableInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_312x_P_EnableHabDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_312x_P_EnableHabDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_312x_P_EnableInitDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_312x_P_EnableInitDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BERR_312x_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_312x_P_WaitForEvent(
    BHAB_Handle handle,        /* [in] BHAB PI Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    uint32_t timeoutMsec       /* [in] timeout in milliseconds */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    /* Temporary fix till B0 312x chip as that provides maskable Host IRQ register. */
    while(1) {
        BHAB_312x_P_EnableHostInterrupt(handle, true);
        retCode = BKNI_WaitForEvent(p312x->hApiEvent, timeoutMsec);
        if ( retCode != BERR_SUCCESS ) { break; }
        BHAB_312x_P_EnableHostInterrupt(handle, false);
        BHAB_CHK_RETCODE(BHAB_312x_P_DecodeInterrupt(handle));
        retCode = BKNI_WaitForEvent(hEvent, 0);
        if (retCode == BERR_SUCCESS ) { break; }
    }

    BHAB_312x_P_EnableHostInterrupt(handle, true);

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_P_RunAp()
******************************************************************************/
BERR_Code BHAB_312x_P_RunAp(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_312x_P_EnableHostInterrupt(handle, true);
#ifdef BHAB_LEAP_UART_ENABLE
    buf = 0x80000001;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_TM_SFT1, &buf));
#else
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_TM_SFT1, &buf));
#endif
    /* start running the AP */
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x0F;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_Reset()
******************************************************************************/
BERR_Code BHAB_312x_Reset(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;
    uint8_t buf1;
    uint8_t writebuf[8];
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);
    /* clear LEAP_HOST_IRQ and put LEAP in reset */
    buf=0xFFFFFFFF;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR1, &buf));

    if(p312x->isSpi){
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = HIF_SFT_RST;
        writebuf[2] = 0x1;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 3));
        writebuf[2] = 0x0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p312x->hSpiRegister,  writebuf, 3));
    }
    else {
        buf1 = 0xFD;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));
        buf1 = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p312x->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));
    }
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x0F;
    buf |= 0x0F;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

    buf =0;
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_TM_MTSIF_CTRL, &buf));
    if(p312x->isMtsif)
       buf &= ~0x01;
    else
       buf |= 0x1;
    BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_TM_MTSIF_CTRL, &buf));
done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_312x_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint16_t n)
{

    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_312x_ReadMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_312x_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_312x_WriteMemory(handle, (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + addr), buf, n));

done:
    return retCode;
}

/******************************************************************************
 BHAB_312x_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_312x_P_ServiceHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BHAB_312x_P_Handle *p312x;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if (BHAB_312x_P_WaitForEvent(handle, p312x->hHabDoneEvent, 10100) == BERR_TIMEOUT)
    {
        retCode = BHAB_ERR_HAB_TIMEOUT;
        goto done;
    }

    if (read_len > 0)
    {
        BHAB_CHK_RETCODE(BHAB_312x_P_ReadHab(handle, 0, read_buf, read_len));
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
    return retCode;
}

/******************************************************************************
 BHAB_312x_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_312x_P_DecodeInterrupt(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    BHAB_P_CallbackInfo *callback;
    uint32_t   buf, buf1, sb;
    unsigned i;

    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS1, &buf1));

    if((buf == 0) &&  (buf1 == 0))
        return retCode;

    /*HAB DONE INTERRUPT*/
    if (buf & BHAB_HAB_DONE)
    {
        BDBG_MSG(("HAB_DONE"));
        BKNI_SetEvent(p312x->hHabDoneEvent);
    }

    if (buf & BHAB_HAB_READY)
        BKNI_SetEvent(p312x->hHabReady);

    /*INIT DONE INTERRUPT*/
    if (buf & BHAB_AP_INIT_DONE)
    {
        BDBG_MSG(("AP INIT DONE"));
        BKNI_SetEvent(p312x->hInitDoneEvent);
    }

    if (buf & BHAB_TUNER_STATUS_RDY)
    {
        BDBG_MSG(("LNA Status Ready"));
        BKNI_SetEvent(p312x->hTnrStatusReady);
    }

    /* ADS interrupts */
    for(i = 0; i < 8; i++)
    {
        if((buf & (BHAB_ADS_CHN0_NO_SIGNAL << 2*i)) == (uint32_t)(BHAB_ADS_CHN0_IN_LOCK << 2*i))
        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].callbackType & 0x1)){
                BDBG_MSG(("Channel %d Locked", i));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eLockChange;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            else {
                BDBG_WRN(("Channel %d LockChange callback not installed. ", i));
            }
        }

        if((buf & (BHAB_ADS_CHN0_NO_SIGNAL << 2*i)) == (uint32_t)(BHAB_ADS_CHN0_OUT_OF_LOCK << 2*i))
        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].callbackType & 0x1)){
                BDBG_MSG(("Channel %d Unlocked", i));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eLockChange;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            else {
                BDBG_WRN(("Channel %d LockChange callback not installed. ", i));
            }
        }

        if((buf & (BHAB_ADS_CHN0_NO_SIGNAL << 2*i)) == (uint32_t)(BHAB_ADS_CHN0_NO_SIGNAL << 2*i))

        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].callbackType & 0x4)){
                BDBG_MSG(("Channel %d found No Signal", i));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eNoSignal;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            else {
                BDBG_WRN(("Channel %d No Signal callback not installed. ", i));
            }
        }

    }

    if (buf & BHAB_ADS_STATUS_RDY)
    {
        BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb));

        for(i = 0; i < 8; i++)
        {
            if (sb & (BHAB_ADS_CHN0_STATUS_RDY << i))
            {
                if((p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i].callbackType & 0x8)){
                    BDBG_MSG(("Channel %d Status Rdy", i));
                    callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eADS0 + i];
                    callback->parm2 = (int) BHAB_Interrupt_eQamAsyncStatusReady;
                    BKNI_EnterCriticalSection();
                    callback->func(callback->pParm1, callback->parm2);
                    BKNI_LeaveCriticalSection();
                }
                else
                    BDBG_WRN(("Channel %d Status Ready callback not installed. ", i));
            }
        }
    }

    /* Out of band interrupts */
    if((buf & BHAB_OOB_NO_SIGNAL) == BHAB_OOB_OUT_OF_LOCK)
    {
        if(p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0].func){
                BDBG_MSG(("OOB out of Lock"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0];
                callback->parm2 = (int) BHAB_Interrupt_eLockChange;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_WRN(("OOB LockChange callback not installed. "));
        }
    }

    if((buf & BHAB_OOB_NO_SIGNAL) == BHAB_OOB_IN_LOCK)
    {
        if(p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0].func){
            BDBG_MSG(("OOB In Lock"));
            callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_WRN(("OOB LockChange callback not installed. "));
        }
    }

    if((buf & BHAB_OOB_NO_SIGNAL) == BHAB_OOB_NO_SIGNAL)
    {
        if(p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0].func){
            BDBG_MSG(("OOB No Signal"));
            callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0];
            callback->parm2 = (int) BHAB_Interrupt_eNoSignal;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_WRN(("OOB No Signal callback not installed. "));
        }
    }

    if (buf & BHAB_OOB_STATUS_RDY)
    {
        if(p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0].func){
            BDBG_MSG(("OOB Status Ready"));
            callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eAOB0];
            callback->parm2 = (int) BHAB_Interrupt_eOobAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_WRN(("OOB callback not installed. "));
        }
    }

    if ((buf1 & BHAB_TNR_SPECTRUM_DATA_RDY) == BHAB_TNR_SPECTRUM_DATA_RDY)
    {
        for(i = 0; i < 8; i++)
        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].callbackType & 0x1)){
                BDBG_MSG(("Spectrum Data Rdy"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eSpectrumAnalyzerDataReady;
                callback->func(callback->pParm1, callback->parm2);
            }
        }
    }

    if ((buf1 & BHAB_CPPM_PWR_LVL_CHANGE) == BHAB_CPPM_PWR_LVL_CHANGE)
    {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eGlobal].func)){
                BDBG_MSG(("CPPM Power Level Change Detected"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eGlobal];
                callback->parm2 = (int) BHAB_Interrupt_eCppmPowerLevelChange;
                BKNI_EnterCriticalSection();
                if(p312x->powerLevelChangeCallback)
                    p312x->powerLevelChangeCallback(callback->pParm1, callback->parm2);
                else
                    BDBG_WRN(("CPPM Power Level Change callback not installed. "));
                BKNI_LeaveCriticalSection();
            }
    }

    if ((buf1 & BHAB_CALIBRATION_COMPLETE) == BHAB_CALIBRATION_COMPLETE)
    {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eGlobal].func)){
                BDBG_MSG(("Calibration Complete"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eGlobal];
                callback->parm2 = (int) BHAB_Interrupt_eCalibrationComplete;
                BKNI_EnterCriticalSection();
                if(p312x->calibrationCompleteCallback)
                    p312x->calibrationCompleteCallback(callback->pParm1, callback->parm2);
                else
                    BDBG_WRN(("Calibration Complete callback not installed. "));
                BKNI_LeaveCriticalSection();
            }
    }

    if ((buf1 & BHAB_IFDAC_ACQUIRE_COMPLETE) == BHAB_IFDAC_ACQUIRE_COMPLETE)
    {
        for(i = 0; i < 8; i++)
        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].callbackType & 0x2)){
                BDBG_MSG(("IFDAC Acquire Complete"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eIfDacAcquireComplete;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
        }
    }

    if ((buf1 & BHAB_IFDAC_STATUS_DATA_RDY) == BHAB_IFDAC_STATUS_DATA_RDY)
    {
        for(i = 0; i < 8; i++)
        {
            if((p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].func) && (p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i].callbackType & 0x4)){
                BDBG_MSG(("IFDAC Status Ready"));
                callback = &p312x->InterruptCallbackInfo[BHAB_DevId_eTNR0 + i];
                callback->parm2 = (int) BHAB_Interrupt_eIfDacStatusReady;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
        }
    }

        /* clear LEAP_HOST_IRQ and LEAP_CTRL_SPARE */
    if(buf != 0)
        BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    if(buf1 != 0)
        BHAB_CHK_RETCODE(BHAB_312x_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR1, &buf1));
done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_312x_P_DecodeError(
    BHAB_Handle handle,           /* [in] BHAB PI Handle */
    BHAB_ApStatus *pApStatus /* [in] AP status returned by BHAB_GetApStatus */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t sb;
    BSTD_UNUSED(handle);

    if (*pApStatus & 0xF)
    {
        sb = *pApStatus & 0XF;

        switch (sb)
        {
            case 1:
              break;

            case 2:
                BDBG_ERR(("Invalid Opcode"));
                retCode = BHAB_ERR_INVALID_OPCODE;
                break;

            case 3:
                BDBG_ERR(("HAB CRC/length error"));
                retCode = BHAB_ERR_INVALID_CRC;
                break;

            case 5:
                BDBG_ERR(("LEAP HAB command processing timeout error"));
                retCode = BHAB_ERR_HAB_TIMEOUT;
                break;

            case 6:
                BDBG_ERR(("LEAP HAB command processing error"));
                retCode = BHAB_ERR_CMD_PROCESSING_ERR;
                break;

            default:
                BDBG_ERR(("Unknown HAB error"));
                retCode = BHAB_ERR_AP_UNKNOWN;
                break;
        }
    }

    return retCode;
}


/******************************************************************************
 BHAB_312x_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_312x_P_CheckHab(
    BHAB_Handle handle    /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_ApStatus status = 0;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_CHK_RETCODE(BHAB_312x_GetApStatus(handle, &status));
    BHAB_CHK_RETCODE(BHAB_312x_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_MSG(("BHAB_312x_P_CheckHab status = 0x%x", status ));
    BDBG_MSG(("BHAB_312x_P_CheckHab IRQ register =  0x%x", buf ));

    if ((status & 0xF) == 0x1) {
        retCode = BERR_SUCCESS;
    }
    else
    {
        BDBG_MSG(("BHAB_312x_P_DecodeError called."));
        retCode = BHAB_312x_P_DecodeError(handle, &status);
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_312x_GetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_312x_GetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_WatchDogTimerSettings *wdtSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(wdtSettings);

     return retCode;
}


/******************************************************************************
 BHAB_312x_SetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_312x_SetWatchDogTimer(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_WatchDogTimerSettings *wdtSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(wdtSettings);

    return retCode;
}


/******************************************************************************
 BHAB_312x_GetNmiConfig()
******************************************************************************/
BERR_Code BHAB_312x_GetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    BHAB_NmiSettings *nmiSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(nmiSettings);

    return retCode;
}


/******************************************************************************
 BHAB_312x_SetNmiConfig()
******************************************************************************/
BERR_Code BHAB_312x_SetNmiConfig(
    BHAB_Handle handle,    /* [in] BHAB Handle */
    const BHAB_NmiSettings *nmiSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);
    BSTD_UNUSED(nmiSettings);

    return retCode;
}

/***************************************************************************
 BHAB_312x_GetConfigSettings()
****************************************************************************/
BERR_Code BHAB_312x_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_READ, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[5] = HAB_MSG_HDR(BHAB_READ_DAISY, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf1, 5, p312x->inBuf, 0, false, true, 5));
    settings->daisyChain = p312x->inBuf[4] & 0x3;

    BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf, 5, p312x->inBuf, 9, false, true, 9));
    settings->enableLoopThrough = p312x->inBuf[4] & 0x1;

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_SetConfigSettings()
****************************************************************************/
BERR_Code BHAB_312x_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_WRITE, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[9] = HAB_MSG_HDR(BHAB_WRITE_DAISY, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if(settings->daisyChain){
        buf1[4] = (((uint8_t)settings->daisyChain & 0x3) - 1) | 0x4;
    }

    BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf1, 9, p312x->inBuf, 0, false, true, 9));

    p312x->daisyChain = settings->daisyChain;

    if(settings->enableLoopThrough)
    {
        buf[4] = 0x1;
        BHAB_CHK_RETCODE(BHAB_312x_SendHabCommand(handle, buf, 9, p312x->inBuf, 0, false, true, 9));
    }

done:
    return retCode;
}

/******************************************************************************
 BHAB_312x_ReadSlave()
******************************************************************************/
BERR_Code BHAB_312x_ReadSlave(
    BHAB_Handle handle,     /* [in] Device channel handle */
    uint8_t chipAddr,       /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,       /* [in] sub addr of the register to read from the slave device */
    uint32_t *data,         /* [out] ptr to the data that we will read from the slave device */
    uint8_t dataLen)        /* [in] how many bytes are we going to read? one to four, For 3412 LNA, this should be in multiples of 4-bytes*/
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[13] = HAB_MSG_HDR(BLNA_READ_REGISTER, 0x4, CORE_TYPE_GLOBAL, BLNA_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BDBG_ENTER(BHAB_312x_ReadSlave);
    if(dataLen != 4)
    {
        BDBG_ERR(("dataLen can only be 4 for 3128"));
        BERR_TRACE(retCode = BERR_NOT_SUPPORTED);
    }
    else
    {
        hab[4] = chipAddr;
        hab[5] = subAddr;
        hab[7] = dataLen;

        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 9, p312x->inBuf, 13, false, true, 13));
            *data = (p312x->inBuf[11] << 24) | (p312x->inBuf[10] << 16) | (p312x->inBuf[9] << 8) | p312x->inBuf[8];
    }

done:
    BDBG_LEAVE(BHAB_312x_ReadSlave);
    return retCode;
}

/******************************************************************************
 BHAB_312x_WriteSlave()
******************************************************************************/
BERR_Code BHAB_312x_WriteSlave(
    BHAB_Handle handle,     /* [in] Device channel handle */
    uint8_t chipAddr,       /* [in] chip addr of the i2c slave device */
    uint32_t subAddr,       /* [in] sub addr of the register to read from the slave device */
    uint32_t *data,         /* [in] ptr to the data that we will write to the slave device */
    uint8_t dataLen)        /* [in] how many bytes are we going to write? one to four. For 3412 LNA, this should be in multiples of 4-bytes*/
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[13] = HAB_MSG_HDR(BLNA_WRITE_REGISTER, 0x8, CORE_TYPE_GLOBAL, BLNA_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BDBG_ENTER(BHAB_312x_WriteSlave);
    if(dataLen != 4)
    {
        BDBG_ERR(("dataLen can only be 4 for 3128"));
        BERR_TRACE(retCode = BERR_NOT_SUPPORTED);
    }
    else
    {
        hab[4] = chipAddr;
        hab[5] = subAddr;
        hab[7] = dataLen;
        hab[8] = *data & 0xFF;
        hab[9] = (*data >> 8) & 0xFF;
        hab[10] = (*data >> 16) & 0xFF;
        hab[11] = (*data >> 24) & 0xFF;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 13, p312x->inBuf, 0, false, true, 13));
    }

done:
    BDBG_LEAVE(BLNA_3412_WriteRegister);
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetInternalGain()
****************************************************************************/
BERR_Code BHAB_312x_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings     /* [out] internal gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[17] = HAB_MSG_HDR(BHAB_INTERNAL_GAIN_READ, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    hab[4] = (inputParams->frequency >> 24);
    hab[5] = (inputParams->frequency >> 16);
    hab[6] = (inputParams->frequency >> 8);
    hab[7] = inputParams->frequency;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 9, p312x->inBuf, 17, false, true, 17 ));
    internalGainSettings->externalGainBypassed = (p312x->inBuf[4] >> 7);
    internalGainSettings->internalGainLoopThrough = (int16_t)((p312x->inBuf[8] << 8) | p312x->inBuf[9])*100/256;
    internalGainSettings->internalGainDaisy = (int16_t)((p312x->inBuf[10] << 8) | p312x->inBuf[11])*100/256;
    internalGainSettings->frequency = (p312x->inBuf[12] << 24) | (p312x->inBuf[13] << 16) | (p312x->inBuf[14] << 8) | p312x->inBuf[15];

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetExternalGain()
****************************************************************************/
BERR_Code BHAB_312x_GetExternalGain(
    BHAB_Handle handle,                               /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p312x->inBuf, 9, false, true, 9 ));
    externalGainSettings->externalGainTotal = (int16_t)((p312x->inBuf[4] << 8) | p312x->inBuf[5])*100/256;
    externalGainSettings->externalGainBypassable = (int16_t)((p312x->inBuf[6] << 8) | p312x->inBuf[7])*100/256;

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_SetExternalGain()
****************************************************************************/
BERR_Code BHAB_312x_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings   /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    int16_t externalGainTotal, externalGainBypassable;
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    externalGainTotal = externalGainSettings->externalGainTotal*256/100;
    externalGainBypassable = externalGainSettings->externalGainBypassable*256/100;

    buf[4] = (externalGainTotal >> 8);
    buf[5] = externalGainTotal;
    buf[6] = (externalGainBypassable >> 8);
    buf[7] = externalGainBypassable;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 9, p312x->inBuf, 0, false, true, 9 ));

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetAvsData()
****************************************************************************/
BERR_Code BHAB_312x_GetAvsData(
    BHAB_Handle handle,         /* [in] Device handle */
    BHAB_AvsData *pAvsData      /* [out] pointer to AVS Data */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[49] = HAB_MSG_HDR(BHAB_AVS_DATA, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p312x->inBuf, 49, false, true, 49 ));
    pAvsData->enabled = (p312x->inBuf[7] & 0x4) >> 2;
    pAvsData->voltage = (p312x->inBuf[8] << 24) | (p312x->inBuf[9] << 16) | (p312x->inBuf[10] << 8) | p312x->inBuf[11];
    pAvsData->temperature = (int32_t)((p312x->inBuf[12] << 24) | (p312x->inBuf[13] << 16) | (p312x->inBuf[14] << 8) | p312x->inBuf[15])*100;
done:
    return retCode;
}
/***************************************************************************
 BHAB_312x_GetTunerChannels()
****************************************************************************/
BERR_Code BHAB_312x_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels)   /* [out] Returns total tuner channels */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, CORE_TYPE_GLOBAL, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p312x->inBuf, 77, false, true, 77));

        numDescriptors =  p312x->inBuf[22];
        handle->totalTunerChannels = p312x->inBuf[23];

        handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        if(!handle->channelCapabilities){
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p312x->inBuf[31+i*16];
            channelNumber = p312x->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.ads = (p312x->inBuf[35+i*16] & 0x2) >> 1;
                handle->channelCapabilities[offset].demodCoreType.aob = (p312x->inBuf[35+i*16] & 0x4) >> 2;
                handle->channelCapabilities[offset].demodCoreType.ifdac = p312x->inBuf[35+i*16] & 0x1;
                channelNumber++;
                offset++;
            }
        }
    }

    *numChannels = handle->totalTunerChannels;

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetCapabilities()
****************************************************************************/
BERR_Code BHAB_312x_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities)   /* [out] Returns chip capabilities */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, CORE_TYPE_GLOBAL, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_312x_P_Handle *p312x;

    BDBG_ASSERT(handle);
    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p312x->inBuf, 77, false, true, 77));

        numDescriptors =  p312x->inBuf[22];
        handle->totalTunerChannels = p312x->inBuf[23];

        handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        if(!handle->channelCapabilities){
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); goto done;
        }
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p312x->inBuf[31+i*16];
            channelNumber = p312x->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.ads = (p312x->inBuf[35+i*16] & 0x2) >> 1;
                handle->channelCapabilities[offset].demodCoreType.aob = (p312x->inBuf[35+i*16] & 0x4) >> 2;
                handle->channelCapabilities[offset].demodCoreType.ifdac = p312x->inBuf[35+i*16] & 0x1;
                channelNumber++;
                offset++;
            }
        }
    }

    pCapabilities->totalTunerChannels = handle->totalTunerChannels;
    BKNI_Memset( pCapabilities->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

    for (i=0; i<pCapabilities->totalTunerChannels; i++)
    {
        pCapabilities->channelCapabilities[i].tunerChannelNumber = handle->channelCapabilities[i].tunerChannelNumber;
        pCapabilities->channelCapabilities[i].demodCoreType.ads = handle->channelCapabilities[i].demodCoreType.ads;
        pCapabilities->channelCapabilities[i].demodCoreType.aob = handle->channelCapabilities[i].demodCoreType.aob;
        pCapabilities->channelCapabilities[i].demodCoreType.ifdac = handle->channelCapabilities[i].demodCoreType.ifdac;
    }

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetRecalibrateSettings()
****************************************************************************/
BERR_Code BHAB_312x_GetRecalibrateSettings(
    BHAB_Handle handle,
    BHAB_RecalibrateSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_312x_P_Handle *p312x;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    *pSettings = p312x->recalibrateSettings;

    return retCode;
}

/***************************************************************************
 BHAB_312x_SetRecalibrateSettings()
****************************************************************************/
BERR_Code BHAB_312x_SetRecalibrateSettings (
    BHAB_Handle handle,
    const BHAB_RecalibrateSettings *pSettings
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[49] = HAB_MSG_HDR(BHAB_SET_CPPM_SETTINGS, 0x2c, BTNR_CORE_TYPE, BHAB_CORE_ID);
    BHAB_312x_P_Handle *p312x;
    int32_t threshold=0;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);

    buf[0] = (0x27  >> 2) & 0xFF;
    buf[1] = ((0x27 & 0x3) << 6) | ((0x2C >> 4) & 0x3F);
    buf[2] = ((0x2C & 0xF) << 4) | 0xE;
    buf[3] = BHAB_CORE_ID;
    if(pSettings->cppm.enabled)
        buf[4] = 0x86;
    else
        buf[4] = 0x85;

    buf[8] = (int32_t)pSettings->cppm.thresholdHysteresis*256/10 >> 8;
    buf[9] = (int32_t)pSettings->cppm.thresholdHysteresis*256/10;

    threshold = pSettings->cppm.threshold*256/10 - 12480;
    buf[10] = (int32_t)threshold >> 8;
    buf[11] = (int32_t)threshold;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 49, p312x->inBuf, 0, false, true, 49));
    p312x->recalibrateSettings = *pSettings;

done:
    return retCode;
}

/***************************************************************************
 BHAB_312x_GetLnaStatus()
****************************************************************************/
BERR_Code BHAB_312x_GetLnaStatus(
    BHAB_Handle handle,
    BHAB_LnaStatus *pStatus
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[121] = HAB_MSG_HDR(BHAB_GET_TUNER_STATUS, 0xc, BTNR_CORE_TYPE, BHAB_CORE_ID);
#if 0
    uint8_t buf1[17] = HAB_MSG_HDR(BHAB_REQUEST_TUNER_STATUS, 0xc, BTNR_CORE_TYPE, BHAB_CORE_ID);
#endif
    BHAB_312x_P_Handle *p312x;
    BDBG_ASSERT(handle);

    p312x = (BHAB_312x_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p312x);
#if 0
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf1, 17, p312x->inBuf, 0, false, true, 17));

    if (BHAB_312x_P_WaitForEvent(handle, p312x->hTnrStatusReady, 1000) == BERR_TIMEOUT)
    {
        BDBG_WRN(("LNA Status Ready Timeout"));
        retCode = BHAB_ERR_HAB_TIMEOUT;
        goto done;
    }
#endif
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p312x->inBuf, 121, false, true, 121));
    pStatus->externalFixedGainLnaState = p312x->inBuf[0xc] >> 7;

done:
    return retCode;
}
