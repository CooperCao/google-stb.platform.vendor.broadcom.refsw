/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
#include "bhab_3461_priv.h"
#include "bchp_hsi.h"
#include "../../b0/bchp_leap_hab_mem.h"
#include "../../b0/bchp_tm.h"
#include "../../b0/bchp_leap_l2.h"

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
#define BHAB_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))
#else
#define BHAB_P_SWAP32(a) a
#endif

#define MAX_HAB_RETRIES 3

BDBG_MODULE(bhab_3461_priv);

/* local private functions */
static uint8_t BHAB_3461_P_CRC8Block(uint8_t crc, uint8_t *pDataBlock, uint32_t dataLength);

/******************************************************************************
 BHAB_3461_Open()
******************************************************************************/
BERR_Code BHAB_3461_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_Handle hDev;
    BHAB_3461_P_Handle *h3461Dev = NULL;
    unsigned i;

    BDBG_ASSERT(pDefSettings->interruptEnableFunc);

    hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));

    if(!hDev)
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    else
    {
        h3461Dev = (BHAB_3461_P_Handle *)BKNI_Malloc(sizeof(BHAB_3461_P_Handle));
        if(h3461Dev)
        {
            BKNI_Memset( h3461Dev, 0x00, sizeof(BHAB_3461_P_Handle));
            hDev->pImpl = (void*)h3461Dev;

            if(pDefSettings->isSpi) {
               h3461Dev->hSpiRegister = (BREG_SPI_Handle)pReg;
            }
            else {
               h3461Dev->hI2cRegister = (BREG_I2C_Handle)pReg;
            }

            BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));
            hDev->channelCapabilities = NULL;

            /* create events */
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3461Dev->hInterruptEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3461Dev->hApiEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3461Dev->hInitDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3461Dev->hHabDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h3461Dev->hHabReady)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h3461Dev->hMutex)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h3461Dev->hRegAccessMutex)));

            for(i=0; i<BHAB_DevId_eMax; i++){
                h3461Dev->InterruptCallbackInfo[i].func = NULL;
                h3461Dev->InterruptCallbackInfo[i].pParm1 = NULL;
                h3461Dev->InterruptCallbackInfo[i].parm2 = (int)NULL;
            }

            BKNI_Memset( &h3461Dev->nmiSettings, 0x00, sizeof(BHAB_NmiSettings));
            BKNI_Memset( &h3461Dev->wdtSettings, 0x00, sizeof(BHAB_WatchDogTimerSettings));

            h3461Dev->isSpi = pDefSettings->isSpi;
            h3461Dev->tnrApplication = BHAB_TunerApplication_eLast; /* set to invalid option */
            h3461Dev->rfInputMode = BHAB_RfInputMode_eLast; /* set to invalid option */

            retCode = BHAB_3461_P_DisableInterrupts(hDev);
        }
        else
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
done:
    if(retCode == BERR_SUCCESS)
        *handle = hDev;
    else
    {
        if(h3461Dev)
        {
            BKNI_Free(h3461Dev);
            h3461Dev = NULL;
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
 BHAB_3461_Close()
******************************************************************************/
BERR_Code BHAB_3461_Close(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint32_t sb, buf;

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    /* If we didn't load the AP when starting, don't reset it when closing */
    if (p3461->loadAP)
    {
        /* reset the AP */
        sb = 0x01;
        BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &sb));

        /* reset all interrupt status */
        buf = 0xFFFFFFFF;
        BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    }

    retCode = BHAB_3461_P_DisableInterrupts(handle);
done:
    if (p3461->hInterruptEvent) {BKNI_DestroyEvent(p3461->hInterruptEvent);}
    if (p3461->hApiEvent){BKNI_DestroyEvent(p3461->hApiEvent);}
    if (p3461->hInitDoneEvent){BKNI_DestroyEvent(p3461->hInitDoneEvent);}
    if (p3461->hHabDoneEvent){BKNI_DestroyEvent(p3461->hHabDoneEvent);}
    if (p3461->hHabReady){BKNI_DestroyEvent(p3461->hHabReady);}
    if (p3461->hMutex){BKNI_DestroyMutex(p3461->hMutex);}
    if (p3461->hRegAccessMutex){BKNI_DestroyMutex(p3461->hRegAccessMutex);}

    BKNI_Free((void*)p3461);
    if(handle->channelCapabilities != NULL)
        BKNI_Free((void*)handle->channelCapabilities);
    BKNI_Free((void*)handle);

    return retCode;
}



/******************************************************************************
 BHAB_3461_P_DumpRegisters()
******************************************************************************/
BERR_Code BHAB_3461_P_DumpRegisters(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t i, buf=0;

    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT7, &buf));
    BDBG_WRN(("BCHP_TM_SFT7 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT6, &buf));
    BDBG_WRN(("BCHP_TM_SFT6 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT5, &buf));
    BDBG_WRN(("BCHP_TM_SFT5 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT4, &buf));
    BDBG_WRN(("BCHP_TM_SFT4 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT3, &buf));
    BDBG_WRN(("BCHP_TM_SFT3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT2, &buf));
    BDBG_WRN(("BCHP_TM_SFT2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT1, &buf));
    BDBG_WRN(("BCHP_TM_SFT1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_SFT0, &buf));
    BDBG_WRN(("BCHP_TM_SFT0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_MISC0, &buf));
    BDBG_WRN(("BCHP_TM_MISC0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_MISC1, &buf));
    BDBG_WRN(("BCHP_TM_MISC1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_MISC2, &buf));
    BDBG_WRN(("BCHP_TM_MISC2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_MISC3, &buf));
    BDBG_WRN(("BCHP_TM_MISC3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, 0x78000, &buf));
    BDBG_WRN(("HAB first word %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, 0x78004, &buf));
    BDBG_WRN(("HAB second word %x", buf));
    for(i=0; i < 20; i++) {
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE0 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE1, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE1 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE2, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE2 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE3, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE3 %x", buf));
    BKNI_Sleep(250);
    }
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));

done:
    return retCode;
}

/******************************************************************************
 BHAB_3461_P_ErrorRecovery()
******************************************************************************/
BERR_Code BHAB_3461_P_ErrorRecovery(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t val=0;

    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    /* Clear the "HAB Done" bit before initiating "HAB Reset" */
    val = BHAB_HAB_DONE;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &val));
    val = BHAB_HAB_RESET;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_L2_CPU_SET, &val));

    if (BHAB_3461_P_WaitForEvent(handle, p3461->hHabReady, 1500) == BERR_TIMEOUT)
    {
        BDBG_ERR(("########################## Unable to reset HAB\n"));
        retCode = BHAB_ERR_HAB_TIMEOUT;
    }

done:
    return retCode;
}

/******************************************************************************
 BHAB_3461_P_InitAp()
******************************************************************************/
BERR_Code BHAB_3461_InitAp(
    BHAB_Handle handle,       /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM3461 microcode image */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint32_t n, instlen, datalen, instaddr, dataaddr;
    const uint8_t *pImage;
    uint8_t retries=0, count=0;

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    /* We are loading AP code, it is appropriate to reset the AP when closing */
    p3461->loadAP = true;

    /* Disable host interrupt */
    BHAB_CHK_RETCODE(BHAB_3461_P_EnableHostInterrupt(handle, false));

    /* reset the AP before downloading the microcode */
    BHAB_CHK_RETCODE(BHAB_3461_Reset(handle));

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
                retCode = BHAB_3461_WriteMemory(handle, instaddr, &pImage[19], instlen);
                if (retCode == BERR_SUCCESS)
                    break;
                else
                    BDBG_WRN(("BHAB_3461_WriteMemory() retCode 0x%X", retCode));
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
        BHAB_CHK_RETCODE(BHAB_3461_P_EnableInitDoneInterrupt(handle));

        /* start running the AP */
        if ((retCode = BHAB_3461_P_RunAp(handle)) == BERR_SUCCESS)
        {
            for (count = 0; count < BHAB_INIT_RETRIES; count++)
            {
                /* wait for init done interrupt */
                if ((retCode = BHAB_3461_P_WaitForEvent(handle, p3461->hInitDoneEvent, 250)) == BERR_SUCCESS)
                    break;
                /* Disable host interrupt */
                if ((retCode = BHAB_3461_P_EnableHostInterrupt(handle, false)) != BERR_SUCCESS)
                    BDBG_WRN(("Failed disabling Host Interrupt"));
            }

            if (count < BHAB_INIT_RETRIES)
                retCode = BERR_SUCCESS;
            else
            {
                /* reset the AP before exiting */
                BHAB_CHK_RETCODE(BHAB_3461_Reset(handle));
                BKNI_Sleep(10);
                BDBG_ERR(("AP initialization timeout. Starting over %d times", count));
                BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
            }
        }
    }

done:
    if(retCode != BERR_SUCCESS)
        BHAB_3461_P_EnableHostInterrupt(handle, true);
    return retCode;
}


/******************************************************************************
 BHAB_3461_GetApStatus()
******************************************************************************/
BERR_Code BHAB_3461_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t sb;

    BDBG_ASSERT(handle);

    *pStatus = 0;
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb));
    *pStatus = sb;

done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_GetApVersion()
******************************************************************************/
BERR_Code BHAB_3461_GetApVersion(
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

    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);
    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_FAMILY_ID, pFamilyId));

    buf[0] = ((BHAB_GETVERSION >> 2) & 0xFF);
    buf[1] = (((BHAB_GETVERSION & 0x3) << 6) | ((PAYLOAD_LEN >> 4) & 0x3F));
    buf[2] = ((PAYLOAD_LEN & 0xF) << 4);
    buf[3] = CORE_ID;
    buf[4] = 0;    /* CRC */

    BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf, 5, p3461->inBuf, 21, false, true, 21));
    *pChipId = ((p3461->inBuf[4] << 8) | p3461->inBuf[5]);
    *pChipVer = ((p3461->inBuf[6] << 8) | p3461->inBuf[7]);
    *pMajApVer = p3461->inBuf[8];
    *pMinApVer = p3461->inBuf[9];

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
BERR_Code BHAB_3461_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[21] = HAB_MSG_HDR(BHAB_GETVERSION, 0, CORE_TYPE, CORE_ID);
    uint8_t sysVersion[73] = HAB_MSG_HDR(BHAB_GET_SYS_VERSION, 0, CORE_TYPE, CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_TM_FAMILY_ID, &pVersionInfo->familyId));

    BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf, 5, p3461->inBuf, 21, false, true, 21));
    pVersionInfo->chipId = ((p3461->inBuf[4] << 8) | p3461->inBuf[5]);
    pVersionInfo->chipVersion = ((p3461->inBuf[6] << 8) | p3461->inBuf[7]);
    pVersionInfo->firmware.majorVersion = p3461->inBuf[8];
    pVersionInfo->firmware.minorVersion = p3461->inBuf[9];

    if((pVersionInfo->firmware.majorVersion >= 6) || ((pVersionInfo->firmware.majorVersion >= 5) & ( pVersionInfo->firmware.minorVersion>= 3)))
    {
        BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, sysVersion, 5, p3461->inBuf, 73, false, true, 73));
        pVersionInfo->familyId = (p3461->inBuf[4] << 24) | (p3461->inBuf[5] << 16) | (p3461->inBuf[6] << 8) | p3461->inBuf[7];
        pVersionInfo->chipId = (p3461->inBuf[8] << 24) | (p3461->inBuf[9] << 16) | (p3461->inBuf[10] << 8) | p3461->inBuf[11];
        pVersionInfo->chipVersion = (p3461->inBuf[12] << 24) | (p3461->inBuf[13] << 16) | (p3461->inBuf[14] << 8) | p3461->inBuf[15];
        pVersionInfo->firmware.majorVersion = ((p3461->inBuf[16] << 8) | p3461->inBuf[17]);
        pVersionInfo->firmware.minorVersion = ((p3461->inBuf[18] << 8) | p3461->inBuf[19]);
        pVersionInfo->firmware.buildType = ((p3461->inBuf[20] << 8) | p3461->inBuf[21]);
        pVersionInfo->firmware.buildId = ((p3461->inBuf[22] << 8) | p3461->inBuf[23]);
        pVersionInfo->bondoutOptions[0] = 0;
        pVersionInfo->bondoutOptions[1] = 0;
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
 BHAB_3461_ReadMemory()
******************************************************************************/
BERR_Code BHAB_3461_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint8_t sb, data, readbuf[8], writebuf[8];
    uint32_t sb1;
    uint16_t bytes_left, offset;
    uint8_t i=0;

    BDBG_ASSERT(handle);
    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);

    if (((uint32_t)addr + (uint32_t)n) > (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    sb1 = BHAB_P_SWAP32(addr);
    if(p3461->isSpi){
        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 6));

        writebuf[0] = (handle->settings.chipAddr << 1);
        for (offset = 0; offset < n/4 * 4; offset+=4) {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister, writebuf, readbuf, 6));
            *buf++ = readbuf[2];
            *buf++ = readbuf[3];
            *buf++ = readbuf[4];
            *buf++ = readbuf[5];

            for(i=0; i < 5; i++){
                writebuf[1] = CSR_STATUS;
                BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister, writebuf, readbuf, 3));
                if (readbuf[2] == BHAB_CPU_RUNNIG)
                    break;
            }
            if(i==5)
                BDBG_WRN(("Read transaction not finished"));
        }
        /* Read the bytes left */
        bytes_left = n%4;
        if(bytes_left)
        {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister, writebuf, readbuf, 2+bytes_left));
            for(i=0; i<bytes_left; i++) {
                *buf++ = readbuf[2+i];
            }
        }

        /* set it back to the default write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 3));
    }
    else {
        /* enable read mode and speculative read*/
        data = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* poll the busy bit to make sure the transaction is completed */
        sb=0;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
            if (sb == BHAB_CPU_RUNNIG)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished"));

        for (offset = 0; offset < n; offset+=4) {
            BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf+offset,
                n - offset < 4 ? n - offset : 4));
            for(i=0; i < 5; i++){
                BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
                if(sb == BHAB_CPU_RUNNIG)
                    break;
            }
            if(i==5)
                BDBG_WRN(("Read transaction not finished"));
        }

        data =0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_WriteMemory()
******************************************************************************/
BERR_Code BHAB_3461_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint8_t pad, i=0, k=0, bits_per_transfer=0, spi_write_size=0;
    uint16_t bytes_left;
    uint32_t sb, sb1, curr_addr, j=0;
    uint8_t readbuf[8], writebuf[32];

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    sb1 = BHAB_P_SWAP32(addr);

    if ((addr + n) >= (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE +128))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if(p3461->isSpi)
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 3));

        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 6));
        writebuf[1] = CSR_RBUS_DATA0;

        if(1){

            BREG_SPI_SetContinueAfterCommand(p3461->hSpiRegister , true);
            BHAB_CHK_RETCODE(BREG_SPI_WriteAll(p3461->hSpiRegister, writebuf, 2));

            BHAB_CHK_RETCODE(BREG_SPI_WriteAll(p3461->hSpiRegister, buf, n));
            BREG_SPI_SetContinueAfterCommand(p3461->hSpiRegister , false);

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
                BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 2+bytes_left));
            }
        }
        else{
            BREG_SPI_GetBitsPerTransfer(p3461->hSpiRegister, &bits_per_transfer);
            if(bits_per_transfer==8)
                spi_write_size = 14;
            else if(bits_per_transfer==16)
                spi_write_size = 30;
            else {
                BDBG_WRN(("Unsupported bits per transfer for this chip. bits_per_transfer = %d", bits_per_transfer));
                retCode = BERR_NOT_SUPPORTED; goto done;
            }

            /* The current max write_size is 14 bytes in 8 bit transfer mode and 30 bytes in 16 bit mode.  */
            for (j = 0; j < n; j+=spi_write_size) {
                for (i = 0; i < spi_write_size; i++) {
                    if(i+j == n)
                        break;
                    writebuf[2+i] = (*((uint8_t *)buf+j+i));
                }
                switch (j%4)
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

            BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, i+2));
        }

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        bytes_left = (n)%4;

            if(bytes_left)
            {
                writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
                writebuf[1] = CSR_RBUS_DATA0+(4-bytes_left);
                for(k=0; k<bytes_left; k++)
                {
                    writebuf[2+k] = 0;
                }
                BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 2+bytes_left));
            }
        }

        /* check for host transfer error */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* set CSR pointer to point to DATA0 and provide the data to be downloaded */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, n));

        /* check for host transfer error */
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, (uint8_t *)&sb, 1));
            if ((sb & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        bytes_left = n%4;
        if(bytes_left)
        {
            pad = 0;
            curr_addr = CSR_RBUS_DATA0 + bytes_left;
            for(i=0; i<(4-bytes_left); i++)
            {
                BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, curr_addr++, &pad, 1));
            }
        }

        /* check for host transfer error */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, (uint8_t *)&sb, 1));
            if ((sb & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_ReadMbox()
******************************************************************************/
BERR_Code BHAB_3461_ReadMbox(
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
 BHAB_3461_WriteMbox()
******************************************************************************/
BERR_Code BHAB_3461_WriteMbox(
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
 BHAB_3461_ReadRegister()
******************************************************************************/
BERR_Code BHAB_3461_ReadRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint8_t data, sb, i=0;
    uint32_t sb1, buf, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    sb1 = BHAB_P_SWAP32(reg);

    if(p3461->isSpi)
    {
        readbuf[2] = 0;

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister, writebuf, 3));

        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |=  ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister,  writebuf, readbuf, 3));
             if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished"));

        /* read data*/
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_RBUS_DATA0;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister, writebuf, readbuf, 6));

        value = ((uint32_t)(readbuf[2] << 0) | (uint32_t)(readbuf[3] << 8) | (uint32_t)(readbuf[4] << 16) | (uint32_t)(readbuf[5] << 24));
        *val = BHAB_P_SWAP32(value);

        /* set READ_RBUS to the reset value for write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p3461->hSpiRegister,  writebuf, 3));
    }
    else
    {
        data=0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, (uint8_t *)&data, 1));

        /* read CSR_CONFIG value*/
        BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
        data &= ~CSR_CONFIG_READ_RBUS_MASK;
        data |= ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));

        /* set READ_RBUS to the reset value for read mode */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            retCode = BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1);
            if ((sb & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished\n"));

        /* read the data */
        BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&buf, 4));
        *val = (uint32_t)BHAB_P_SWAP32(buf);

        /* set READ_RBUS to the reset value for write mode */
        data = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_3461_WriteRegister()
******************************************************************************/
BERR_Code BHAB_3461_WriteRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    uint8_t sb, buf[4], i=0;
    uint32_t sb1, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);
    BSTD_UNUSED(reg);

    sb1 = BHAB_P_SWAP32(reg);
    if(p3461->isSpi)
    {
        /* enable write mode */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read (p3461->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |= (CSR_CONFIG_READ_RBUS_WRITE << CSR_CONFIG_READ_RBUS_SHIFT);
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister,  writebuf, 6));

        /* write to RBUS*/
        writebuf[1] = CSR_RBUS_DATA0;
        value = BHAB_P_SWAP32(*(uint32_t *)val);
        writebuf[2] = (value & 0xff) >> 0;
        writebuf[3] = (value & 0xff00) >> 8;
        writebuf[4] = (value & 0xff0000) >> 16;
        writebuf[5] = (value & 0xff000000) >> 24;

        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p3461->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else {
        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));
        sb1 = BHAB_P_SWAP32(*(uint32_t *)val);
        /* write to RBUS*/
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&sb1, 4));

        BHAB_CHK_RETCODE(BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            retCode = BREG_I2C_P_Read3461(p3461->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1);
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
 BHAB_3461_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_3461_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
    BHAB_3461_P_Handle *p3461;
    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);

    BKNI_SetEvent_isr(p3461->hApiEvent);
    BKNI_SetEvent_isr(p3461->hInterruptEvent);
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_3461_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_3461_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(handle);

    BHAB_P_ACQUIRE_MUTEX(handle);
    BHAB_3461_P_EnableHostInterrupt(handle, false);
    BHAB_CHK_RETCODE(BHAB_3461_P_DecodeInterrupt(handle));
    BHAB_3461_P_EnableHostInterrupt(handle, true);

done:
    if(retCode != BERR_SUCCESS)
        BHAB_3461_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_3461_EnableLockInterrupt()
******************************************************************************/
BERR_Code BHAB_3461_EnableLockInterrupt(
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
 BHAB_3461_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_3461_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_3461_P_Handle *p3461;

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p3461->InterruptCallbackInfo[eDevId];

    BKNI_EnterCriticalSection();
    callback->pParm1 = pParm1;
    callback->parm2 = parm2;
    callback->func = fCallBack;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_3461_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_3461_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_3461_P_Handle *p3461;

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p3461->InterruptCallbackInfo[eDevId];


    BKNI_EnterCriticalSection();
    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->parm2 = (int)NULL;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_3461_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_3461_SendHabCommand(
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
        BHAB_P_RELEASE_MUTEX(handle);
        return (BERR_TRACE(BERR_INVALID_PARAMETER));
    }

    BHAB_3461_P_EnableHostInterrupt(handle, false);

    *(write_buf+write_len-1) = BHAB_3461_P_CRC8Block(crc, write_buf, write_len-1);
    for(retries=0; retries<MAX_HAB_RETRIES; retries++) {
        if(retries)
            BDBG_WRN(("Resending HAB command"));
        /* write the command to the HAB */
        BHAB_CHK_RETCODE(BHAB_3461_P_WriteHab(handle, 0, write_buf, write_len));
        if(bInsertTermination){
            sb=0x1;
            BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_CTRL_HAB_REQUEST, &sb));
        }
        /* wait for the AP to service the HAB, and then read any return data */
        retCode = BHAB_3461_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80);

        if(retCode == BHAB_ERR_HAB_TIMEOUT) {
                BDBG_WRN(("HAB Timeout detected, resetting HAB..."));
                BHAB_3461_P_ErrorRecovery(handle);
                continue;
        }
        else
            goto read_hab;

read_hab:
        crc = 0;
        datalength = (((*(read_buf+1) & 0x3F) << 4) | (*(read_buf+2) >> 4) ) + 4;

        if((read_len != 0) && (*(read_buf+datalength) != BHAB_3461_P_CRC8Block(crc, read_buf, datalength))) {
            BDBG_WRN(("Invalid CRC"));
            retCode = BHAB_ERR_INVALID_CRC;
            continue;
        }

        retCode = BHAB_3461_P_CheckHab(handle);
        if(retCode == BERR_SUCCESS)
            break;
    }

    if((retries==MAX_HAB_RETRIES) && (retCode == BHAB_ERR_HAB_TIMEOUT)){
        BDBG_ERR(("HAB timeout\n"));
        BDBG_WRN(("Dumping 3461 Registers\n"));
        BHAB_3461_P_DumpRegisters(handle);
#ifdef BHAB_DEBUG
        BDBG_ASSERT(false);
#endif
    }
done:
    if(retCode != BERR_SUCCESS)
        BHAB_3461_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);

    return retCode;
}

/******************************************************************************
 BHAB_3461_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_3461_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    BDBG_ASSERT(handle);

    *hEvent = ((BHAB_3461_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}


/****************************Private Functions*********************************/

static uint8_t BHAB_3461_P_CRC8Byte(
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

static uint8_t BHAB_3461_P_CRC8Block(
    uint8_t crc,
    uint8_t *pDataBlock,
    uint32_t dataLength
    )
{
    uint32_t i;

    for ( i = 0; i < dataLength; i++ )
    {
        crc = BHAB_3461_P_CRC8Byte(crc, *pDataBlock++ );
    }

    return crc;
}


/******************************************************************************
 BREG_I2C_P_Read3461()
******************************************************************************/
BERR_Code BREG_I2C_P_Read3461(
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
 BHAB_3461_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_3461_P_EnableHostInterrupt(
    BHAB_Handle handle, /* [in] HAB handle */
    bool bEnable        /* [in] true=enables the L1 interrupt on the host processor */
)
{
    BDBG_ASSERT(handle);

    BKNI_EnterCriticalSection();
    handle->settings.interruptEnableFunc(bEnable, handle->settings.interruptEnableFuncParam);
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_3461_P_DisableInterrupts()
******************************************************************************/
BERR_Code BHAB_3461_P_DisableInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_3461_P_EnableHabDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_3461_P_EnableHabDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_3461_P_EnableInitDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_3461_P_EnableInitDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BERR_3461_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_3461_P_WaitForEvent(
    BHAB_Handle handle,             /* [in] BHAB PI Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    int timeoutMsec            /* [in] timeout in milliseconds */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);

    while (1)
    {
        BHAB_3461_P_EnableHostInterrupt(handle, true);
        retCode = BKNI_WaitForEvent(p3461->hApiEvent, timeoutMsec);
        if ( retCode != BERR_SUCCESS ) { break; }
        BHAB_3461_P_EnableHostInterrupt(handle, false);
        BHAB_3461_P_DecodeInterrupt(handle);

        retCode = BKNI_WaitForEvent(hEvent, 0);
        if (retCode == BERR_SUCCESS || retCode == BERR_OS_ERROR) { break; }
    }

    BHAB_3461_P_EnableHostInterrupt(handle, true);

    return retCode;
}


/******************************************************************************
 BHAB_3461_P_RunAp()
******************************************************************************/
BERR_Code BHAB_3461_P_RunAp(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_3461_P_EnableHostInterrupt(handle, true);
#ifdef BHAB_LEAP_UART_ENABLE
    buf = 0x80000001;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_TM_SFT1, &buf));
#else
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_TM_SFT1, &buf));
#endif
    /* start running the AP */
    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x0F;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_Reset()
******************************************************************************/
BERR_Code BHAB_3461_Reset(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;
    uint8_t buf1;
    BHAB_3461_P_Handle *p3461;
    uint8_t writebuf[8];

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);
    /* clear LEAP_HOST_IRQ and put LEAP in reset */
    buf=0xFFFFFFFF;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));

    if(p3461->isSpi){
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = HIF_SFT_RST;
        writebuf[2] = 0x1;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister,  writebuf, 3));
        writebuf[2] = 0x0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p3461->hSpiRegister,  writebuf, 3));
    }
    else {
        buf1 = 0xFD;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));
        buf1 = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p3461->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));
    }

    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x0F;
    buf |= 0x0F;
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

done:
    return retCode;
}

/******************************************************************************
 BHAB_3461_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_3461_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint16_t n)
{

    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_3461_ReadMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_3461_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_3461_WriteMemory(handle, (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + addr), buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_3461_P_ServiceHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    if (BHAB_3461_P_WaitForEvent(handle, p3461->hHabDoneEvent, 10100) == BERR_TIMEOUT)
    {
        retCode = BHAB_ERR_HAB_TIMEOUT;
        goto done;
    }

    if (read_len > 0)
    {
        BHAB_CHK_RETCODE(BHAB_3461_P_ReadHab(handle, 0, read_buf, read_len));
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
 BHAB_3461_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_3461_P_DecodeInterrupt(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_3461_P_Handle *p3461;
    BHAB_P_CallbackInfo *callback;
    uint32_t buf;

    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_3461_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    if(buf == 0)
        return retCode;

    /*HAB DONE INTERRUPT*/
    if (buf & BHAB_HAB_DONE)
    {
        BDBG_MSG(("HAB_DONE"));
        BKNI_SetEvent(p3461->hHabDoneEvent);
    }

    if (buf & BHAB_HAB_READY)
        BKNI_SetEvent_isr(p3461->hHabReady);

    /*INIT DONE INTERRUPT*/
    if (buf & BHAB_AP_INIT_DONE)
    {
        BDBG_MSG(("AP INIT DONE"));
        BKNI_SetEvent(p3461->hInitDoneEvent);
    }

    if((buf & BHAB_ADS_NO_SIGNAL) == BHAB_ADS_OUT_OF_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS out of Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eADS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("ADS callback not installed. "));
        }
    }

    if((buf & BHAB_ADS_NO_SIGNAL) == BHAB_ADS_IN_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS In Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eADS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("ADS callback not installed. "));
        }
    }

    if((buf & BHAB_ADS_NO_SIGNAL) == BHAB_ADS_NO_SIGNAL)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS No Signal"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eADS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("ADS callback not installed. "));
        }
    }

    if (buf & BHAB_ADS_STATUS_RDY)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS Status Ready"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eADS0];
            callback->parm2 = (int) BHAB_Interrupt_eQamAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("ADS callback not installed. "));
        }
    }

    if((buf & BHAB_T2_NO_SIGNAL) == BHAB_T2_OUT_OF_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTC20].func){
            BDBG_WRN(("T2 out of Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTC20];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("T2 callback not installed. "));
        }
    }

    if((buf & BHAB_T2_NO_SIGNAL) == BHAB_T2_IN_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTC20].func){
            BDBG_WRN(("T2 In Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTC20];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("T2 callback not installed. "));
        }
    }

    if((buf & BHAB_T2_NO_SIGNAL) == BHAB_T2_NO_SIGNAL)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTC20].func){
            BDBG_WRN(("T2 No Signal"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTC20];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("T2 callback not installed. "));
        }
    }

    if (buf & BHAB_T2_STATUS_RDY)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTC20].func){
            BDBG_MSG(("T2 Status Ready"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTC20];
            callback->parm2 = (int) BHAB_Interrupt_eTc2AsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("T2 callback not installed. "));
        }
    }

    if((buf & BHAB_THD_NO_SIGNAL) == BHAB_THD_OUT_OF_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0].func){
            BDBG_WRN(("THD out of Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf & BHAB_THD_NO_SIGNAL) == BHAB_THD_IN_LOCK)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0].func){
            BDBG_WRN(("THD In Lock"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf & BHAB_THD_NO_SIGNAL) == BHAB_THD_NO_SIGNAL)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0].func){
            BDBG_WRN(("THD No Signal"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }


    if (buf & BHAB_THD_STATUS_RDY)
    {
        if(p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0].func){
            BDBG_MSG(("THD Status Ready"));
            callback = &p3461->InterruptCallbackInfo[BHAB_DevId_eTHD0];
            callback->parm2 = (int) BHAB_Interrupt_eThdAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    /* clear LEAP_HOST_IRQ */
    BHAB_CHK_RETCODE(BHAB_3461_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_3461_P_DecodeError(
    BHAB_Handle handle,           /* [in] BHAB PI Handle */
    BHAB_ApStatus *pApStatus /* [in] AP status returned by BHAB_GetApStatus */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t sb;
    BSTD_UNUSED(handle);

    if (*pApStatus & 0xF)
    {
        sb = *pApStatus & 0xF;

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
 BHAB_3461_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_3461_P_CheckHab(
    BHAB_Handle handle    /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_ApStatus status = 0;

    BHAB_CHK_RETCODE(BHAB_3461_GetApStatus(handle, &status));

    if (((status & 0xF) == 0x1) || ((status & 0xF) == 0))
        retCode = BERR_SUCCESS;
    else
    {
        BDBG_WRN(("BHAB_3461_P_DecodeError called"));
        retCode = BHAB_3461_P_DecodeError(handle, &status);
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_3461_GetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_3461_GetWatchDogTimer(
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
 BHAB_3461_SetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_3461_SetWatchDogTimer(
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
 BHAB_3461_GetNmiConfig()
******************************************************************************/
BERR_Code BHAB_3461_GetNmiConfig(
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
 BHAB_3461_SetNmiConfig()
******************************************************************************/
BERR_Code BHAB_3461_SetNmiConfig(
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
 BHAB_3461_GetConfigSettings()
****************************************************************************/
BERR_Code BHAB_3461_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BHAB_READ_DAISY, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[5] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_READ, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf, 5, p3461->inBuf, 0, false, true, 5));
    settings->daisyChain = p3461->inBuf[4] & 0x3;

    BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf1, 5, p3461->inBuf, 0, false, true, 5));
    settings->enableLoopThrough = p3461->inBuf[4] & 0x4;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 5, p3461->inBuf, 17, false, true, 17));
    settings->tnrApplication = ((p3461->inBuf[4] & 0xC0) >> 6) - 1;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 5, p3461->inBuf, 9, false, true, 9));
    settings->rfInputMode = p3461->inBuf[4];

done:
    return retCode;
}

/***************************************************************************
 BHAB_3461_SetConfigSettings()
****************************************************************************/
BERR_Code BHAB_3461_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_WRITE_DAISY, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[9] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_WRITE, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_WRITE, 0xC, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    if(settings->tnrApplication != p3461->tnrApplication) {
        tnrApplication[4] |= ((settings->tnrApplication + 1) << 6);
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 17, p3461->inBuf, 0, false, true, 17));
        p3461->tnrApplication = settings->tnrApplication;
    }

   if(settings->rfInputMode != p3461->rfInputMode) {
       rfInputMode[4] = settings->rfInputMode;
       BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 9, p3461->inBuf, 0, false, true, 9));
   }

    if(settings->daisyChain){
        buf[4] = (((uint8_t)settings->daisyChain & 0x3) - 1) | 0x4;
    }

    BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf, 9, p3461->inBuf, 0, false, true, 9));

    p3461->daisyChain = settings->daisyChain;

    if(settings->enableLoopThrough)
    {
        buf1[4] = 0x4;
        BHAB_CHK_RETCODE(BHAB_3461_SendHabCommand(handle, buf1, 9, p3461->inBuf, 0, false, true, 9));
    }

done:
    return retCode;
}

/***************************************************************************
 BHAB_3461_GetInternalGain()
****************************************************************************/
BERR_Code BHAB_3461_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings     /* [out] internal gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[17] = HAB_MSG_HDR(BHAB_INTERNAL_GAIN_READ, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    hab[4] = (inputParams->frequency >> 24);
    hab[5] = (inputParams->frequency >> 16);
    hab[6] = (inputParams->frequency >> 8);
    hab[7] = inputParams->frequency;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 9, p3461->inBuf, 17, false, true, 17 ));
    internalGainSettings->externalGainBypassed = (p3461->inBuf[4] >> 7);
    internalGainSettings->internalGainLoopThrough = (int16_t)((p3461->inBuf[8] << 8) | p3461->inBuf[9])*100/256;
    internalGainSettings->internalGainDaisy = (int16_t)((p3461->inBuf[10] << 8) | p3461->inBuf[11])*100/256;
    internalGainSettings->frequency = (p3461->inBuf[12] << 24) | (p3461->inBuf[13] << 16) | (p3461->inBuf[14] << 8) | p3461->inBuf[15];

done:
    return retCode;
}

BERR_Code BHAB_3461_GetExternalGain(
    BHAB_Handle handle,                               /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p3461->inBuf, 9, false, true, 9 ));
    externalGainSettings->externalGainTotal = (int16_t)((p3461->inBuf[4] << 8) | p3461->inBuf[5])*100/256;
    externalGainSettings->externalGainBypassable = (int16_t)((p3461->inBuf[6] << 8) | p3461->inBuf[7])*100/256;

done:
    return retCode;
}

BERR_Code BHAB_3461_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings   /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    int16_t externalGainTotal, externalGainBypassable;
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    externalGainTotal = externalGainSettings->externalGainTotal*256/100;
    externalGainBypassable = externalGainSettings->externalGainBypassable*256/100;

    buf[4] = (externalGainTotal >> 8);
    buf[5] = externalGainTotal;
    buf[6] = (externalGainBypassable >> 8);
    buf[7] = externalGainBypassable;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 9, p3461->inBuf, 0, false, true, 9 ));

done:
    return retCode;
}

/***************************************************************************
 BHAB_3461_GetAvsData()
****************************************************************************/
BERR_Code BHAB_3461_GetAvsData(
    BHAB_Handle handle,         /* [in] Device handle */
    BHAB_AvsData *pAvsData      /* [out] pointer to AVS Data */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[49] = HAB_MSG_HDR(BHAB_AVS_DATA, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p3461->inBuf, 49, false, true, 49 ));
    pAvsData->enabled = (p3461->inBuf[7] & 0x4) >> 2;
    pAvsData->voltage = (p3461->inBuf[8] << 24) | (p3461->inBuf[9] << 16) | (p3461->inBuf[10] << 8) | p3461->inBuf[11];
    pAvsData->temperature = (int32_t)((p3461->inBuf[12] << 24) | (p3461->inBuf[13] << 16) | (p3461->inBuf[14] << 8) | p3461->inBuf[15])*100;
done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_GetTunerChannels()
****************************************************************************/
BERR_Code BHAB_3461_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels)   /* [out] Returns total tuner channels */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BDBG_ASSERT(handle);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p3461->inBuf, 77, false, true, 77));

        numDescriptors =  p3461->inBuf[22];
        handle->totalTunerChannels = p3461->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p3461->inBuf[31+i*16];
            channelNumber = p3461->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p3461->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p3461->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p3461->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p3461->inBuf[35+i*16] & 0x80) >> 7;
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
 BHAB_3461_GetCapabilities()
****************************************************************************/
BERR_Code BHAB_3461_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities    /* [out] Returns chip capabilities */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_3461_P_Handle *p3461;
    BDBG_ASSERT(handle);

    p3461 = (BHAB_3461_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p3461);

    BDBG_ASSERT(handle);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p3461->inBuf, 77, false, true, 77));

        numDescriptors =  p3461->inBuf[22];
        handle->totalTunerChannels = p3461->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p3461->inBuf[31+i*16];
            channelNumber = p3461->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p3461->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p3461->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p3461->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p3461->inBuf[35+i*16] & 0x80) >> 7;
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
        pCapabilities->channelCapabilities[i].demodCoreType.dvbt = handle->channelCapabilities[i].demodCoreType.dvbt;
        pCapabilities->channelCapabilities[i].demodCoreType.dvbt2 = handle->channelCapabilities[i].demodCoreType.dvbt2;
        pCapabilities->channelCapabilities[i].demodCoreType.dvbc2 = handle->channelCapabilities[i].demodCoreType.dvbc2;
        pCapabilities->channelCapabilities[i].demodCoreType.isdbt = handle->channelCapabilities[i].demodCoreType.isdbt;
    }

done:
    return retCode;
}
