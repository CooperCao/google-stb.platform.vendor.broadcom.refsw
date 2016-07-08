/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bhab_7563_priv.h"
#include "bchp_leap_l2.h"

#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
#define BHAB_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))
#else
#define BHAB_P_SWAP32(a) a
#endif

/* define BHAB_DEBUG */
#define MAX_HAB_RETRIES 3

BDBG_MODULE(bhab_7563_priv);

static uint8_t BHAB_7563_P_CRC8Block(uint8_t crc, uint8_t *pDataBlock, uint32_t dataLength);

/******************************************************************************
 BHAB_7563_Open()
******************************************************************************/
BERR_Code BHAB_7563_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot register handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_Handle hDev;
    BHAB_7563_P_Handle *h7563Dev = NULL;
    unsigned i;

    BDBG_ASSERT(pDefSettings->interruptEnableFunc);

    hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));

    if(!hDev)
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    else
    {
        h7563Dev = (BHAB_7563_P_Handle *)BKNI_Malloc(sizeof(BHAB_7563_P_Handle));
        if(h7563Dev)
        {
            BKNI_Memset( h7563Dev, 0x00, sizeof(BHAB_7563_P_Handle));
            hDev->pImpl = (void*)h7563Dev;

            h7563Dev->hRegHandle = (BREG_Handle)pReg;

            BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));
            hDev->channelCapabilities = NULL;

            /* create events */
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h7563Dev->hInterruptEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h7563Dev->hApiEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h7563Dev->hInitDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h7563Dev->hHabReady)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h7563Dev->hHabDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h7563Dev->hMutex)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h7563Dev->hRegAccessMutex)));

            for(i=0; i<BHAB_DevId_eMax; i++){
                h7563Dev->InterruptCallbackInfo[i].func = NULL;
                h7563Dev->InterruptCallbackInfo[i].pParm1 = NULL;
                h7563Dev->InterruptCallbackInfo[i].parm2 = 0;
            }

            BKNI_Memset( &h7563Dev->nmiSettings, 0x00, sizeof(BHAB_NmiSettings));
            BKNI_Memset( &h7563Dev->wdtSettings, 0x00, sizeof(BHAB_WatchDogTimerSettings));

            h7563Dev->isSpi = pDefSettings->isSpi;
            h7563Dev->tnrApplication = BHAB_TunerApplication_eLast; /* set to invalid option */
            h7563Dev->rfInputMode = BHAB_RfInputMode_eLast; /* set to invalid option */

            retCode = BHAB_7563_P_DisableInterrupts(hDev);
        }
        else
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
done:
    if(retCode == BERR_SUCCESS)
        *handle = hDev;
    else
    {
        if(h7563Dev)
        {
            BKNI_Free(h7563Dev);
            h7563Dev = NULL;
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
 BHAB_7563_Close()
******************************************************************************/
BERR_Code BHAB_7563_Close(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    uint32_t sb;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    /* If we didn't load the AP when starting, don't reset it when closing */
    if (p7563->loadAP)
    {
        /* reset the LEAP */
        retCode = BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &sb);
        sb |= 0x03;
        retCode = BHAB_7563_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &sb);

        if(retCode)
            BDBG_WRN(("Ap not in reset."));
    }

    retCode = BHAB_7563_P_DisableInterrupts(handle);

    if (p7563->hInterruptEvent) {BKNI_DestroyEvent(p7563->hInterruptEvent);}
    if (p7563->hApiEvent){BKNI_DestroyEvent(p7563->hApiEvent);}
    if (p7563->hInitDoneEvent){BKNI_DestroyEvent(p7563->hInitDoneEvent);}
    if (p7563->hHabReady){BKNI_DestroyEvent(p7563->hHabReady);}
    if (p7563->hHabDoneEvent){BKNI_DestroyEvent(p7563->hHabDoneEvent);}
    if (p7563->hMutex){BKNI_DestroyMutex(p7563->hMutex);}
    if (p7563->hRegAccessMutex){BKNI_DestroyMutex(p7563->hRegAccessMutex);}

    BKNI_Free((void*)p7563);
    if(handle->channelCapabilities != NULL)
        BKNI_Free((void*)handle->channelCapabilities);
    BKNI_Free((void*)handle);

    return retCode;
}


/******************************************************************************
 BHAB_7563_P_DownloadFirmware()
******************************************************************************/
BERR_Code BHAB_7563_P_DownloadFirmware(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    uint16_t bytes_left;
    uint32_t i=0, val = 0;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if ((addr + n) >= (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = 0; i < n/4; i++) {
        val = (*(buf+4*i) << 24) | (*(buf+4*i+1) << 16) | (*(buf+4*i+2) << 8) | *(buf+4*i+3);
        val = BHAB_P_SWAP32(val);
        BREG_Write32(p7563->hRegHandle, addr, val);
        addr += 4;
    }

    bytes_left = n%4;
    if(bytes_left)
    {
        val = 0;
        for (i = 0; i < bytes_left; i++) {
            val |= *(buf+n/4*4+i) << 8*i;
        }
        val = BHAB_P_SWAP32(val);
        BREG_Write32(p7563->hRegHandle, addr, val);
        addr += 4;
    }
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_ErrorRecovery()
******************************************************************************/
BERR_Code BHAB_7563_P_ErrorRecovery(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t val=0;

    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    val = BHAB_HAB_RESET;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_L2_CPU_SET, &val));
    if (BHAB_7563_P_WaitForEvent(handle, p7563->hHabReady, 1500) == BERR_TIMEOUT)
    {
        BDBG_ERR(("########################## Unable to reset HAB\n"));
        retCode = BHAB_ERR_HAB_TIMEOUT;
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_DumpRegisters()
******************************************************************************/
BERR_Code BHAB_7563_P_DumpRegisters(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t i, buf=0;

    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, &buf));
    BDBG_WRN(("HAB first word %x", buf));
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_HAB_MEM_WORD_TWO, &buf));
    BDBG_WRN(("HAB second word %x", buf));
#ifdef BHAB_DEBUG
    for(i=0; i < 20; i++) {
#else
    for(i=0; i < 1; i++) {
#endif
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE0 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE1, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE1 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE2, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE2 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE3, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE3 %x", buf));
    BKNI_Sleep(250);
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_InitAp()
******************************************************************************/
BERR_Code BHAB_7563_InitAp(
    BHAB_Handle handle,       /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM7563 microcode image */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    uint32_t n, instaddr;
    const uint8_t *pImage;
    uint8_t retries=0, count=0;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    /* We are loading AP code, it is appropriate to reset the AP when closing */
    p7563->loadAP = true;
    /* Disable host interrupt */
    BHAB_CHK_RETCODE(BHAB_7563_P_EnableHostInterrupt(handle, false));
    /* reset the AP before downloading the microcode */
    BHAB_CHK_RETCODE(BHAB_7563_Reset(handle));

    if (pHexImage)
    {
        /* download to RAM */
        pImage = pHexImage;
        if(pImage[59]) {
            n = (pImage[72] << 24) | (pImage[73] << 16) | (pImage[74] << 8) | pImage[75];
            if (n != 0)
            {
                instaddr = (pImage[64] << 24) | (pImage[65] << 16) | (pImage[66] << 8) | pImage[67];
                instaddr = BCHP_LEAP_PROG0_MEM_WORDi_ARRAY_BASE; /* TODO AT */
                for (retries = 0; retries < 3; retries++)
                {
                    retCode = BHAB_7563_P_DownloadFirmware(handle, instaddr, &pImage[312], n);
                    if (retCode == BERR_SUCCESS)
                        break;
                    else
                        BDBG_WRN(("BHAB_7563_P_DownloadFirmware() retCode 0x%X", retCode));
                }
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
        /* start running the LEAP */
        if ((retCode = BHAB_7563_P_RunAp(handle)) == BERR_SUCCESS)
        {
            for (count = 0; count < BHAB_INIT_RETRIES; count++)
            {
                /* wait for init done interrupt */
                if ((retCode = BHAB_7563_P_WaitForEvent(handle, p7563->hInitDoneEvent, 150)) == BERR_SUCCESS)
                    break;
                /* Disable host interrupt */
                if ((retCode = BHAB_7563_P_EnableHostInterrupt(handle, false)) != BERR_SUCCESS)
                    BDBG_WRN(("Failed disabling Host Interrupt"));
            }

            if (count < BHAB_INIT_RETRIES)
                retCode = BERR_SUCCESS;
            else
            {
                /* reset the AP before exiting */
                BHAB_CHK_RETCODE(BHAB_7563_Reset(handle));
                BKNI_Sleep(10);
                BDBG_ERR(("AP initialization timeout."));
                BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
            }
        }
    }

done:
    if(retCode != BERR_SUCCESS)
        BHAB_7563_P_EnableHostInterrupt(handle, true);
    return retCode;
}


/******************************************************************************
 BHAB_7563_GetApStatus()
******************************************************************************/
BERR_Code BHAB_7563_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t sb;

    BDBG_ASSERT(handle);

    *pStatus = 0;
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb));
    *pStatus = sb;

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_GetApVersion()
******************************************************************************/
BERR_Code BHAB_7563_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    *pFamilyId, /* [out] Chip Family id */
    uint32_t    *pChipId,   /* [out] BHAB chip ID */
    uint16_t    *pChipVer,  /* [out] chip revision number */
    uint8_t     *pMajApVer, /* [out] AP microcode major version */
    uint8_t     *pMinApVer  /* [out] AP microcode minor version */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[73] = HAB_MSG_HDR(BHAB_GET_SYS_VERSION, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, buf, 5, p7563->inBuf, 73, false, true, 73));
    *pFamilyId = (p7563->inBuf[4] << 24) | (p7563->inBuf[5] << 16) | (p7563->inBuf[6] << 8) | p7563->inBuf[7];
    *pChipId = (p7563->inBuf[8] << 24) | (p7563->inBuf[9] << 16) | (p7563->inBuf[10] << 8) | p7563->inBuf[11];
    *pChipVer = (p7563->inBuf[14] << 8) | p7563->inBuf[15];
    *pMajApVer = ((p7563->inBuf[16] << 8) | p7563->inBuf[17]);
    *pMinApVer = ((p7563->inBuf[18] << 8) | p7563->inBuf[19]);

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
BERR_Code BHAB_7563_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t sysVersion[73] = HAB_MSG_HDR(BHAB_GET_SYS_VERSION, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, sysVersion, 5, p7563->inBuf, 73, false, true, 73));
    pVersionInfo->familyId = (p7563->inBuf[4] << 24) | (p7563->inBuf[5] << 16) | (p7563->inBuf[6] << 8) | p7563->inBuf[7];
    pVersionInfo->chipId = (p7563->inBuf[8] << 24) | (p7563->inBuf[9] << 16) | (p7563->inBuf[10] << 8) | p7563->inBuf[11];
    pVersionInfo->chipVersion = (p7563->inBuf[12] << 24) | (p7563->inBuf[13] << 16) | (p7563->inBuf[14] << 8) | p7563->inBuf[15];
    pVersionInfo->firmware.majorVersion = ((p7563->inBuf[16] << 8) | p7563->inBuf[17]);
    pVersionInfo->firmware.minorVersion = ((p7563->inBuf[18] << 8) | p7563->inBuf[19]);
    pVersionInfo->firmware.buildType = ((p7563->inBuf[20] << 8) | p7563->inBuf[21]);
    pVersionInfo->firmware.buildId = ((p7563->inBuf[22] << 8) | p7563->inBuf[23]);
    pVersionInfo->bondoutOptions[0] = (p7563->inBuf[40] << 24) | (p7563->inBuf[41] << 16) | (p7563->inBuf[42] << 8) | p7563->inBuf[43];
    pVersionInfo->bondoutOptions[1] = (p7563->inBuf[44] << 24) | (p7563->inBuf[45] << 16) | (p7563->inBuf[46] << 8) | p7563->inBuf[47];

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_ReadMemory()
******************************************************************************/
BERR_Code BHAB_7563_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    uint16_t bytes_left;
    uint8_t i=0, j=0;
    uint32_t val=0;

    BDBG_ASSERT(handle);
    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);

    if (((uint32_t)addr + (uint32_t)n) > (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = 0; i < n/4; i++) {
        val = BREG_Read32(p7563->hRegHandle, addr);
        val = BHAB_P_SWAP32(val);
        for (j = 0; j < 4; j++) {
            *(buf+4*i+j) = val >> 8*j;
        }
        addr += 4;
    }

    bytes_left = n%4;
    if(bytes_left)
    {
        val = BREG_Read32(p7563->hRegHandle, addr);
        val = BHAB_P_SWAP32(val);
        for (j = 0; j < bytes_left; j++) {
            *(buf+4*i+j) = val >> 8*j;
        }
        addr += 4;
    }

    return retCode;
}


 /******************************************************************************
 BHAB_7563_WriteMemory()
******************************************************************************/
BERR_Code BHAB_7563_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    uint16_t bytes_left;
    uint32_t i=0, val = 0;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if ((addr + n) >= (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    for (i = 0; i < n/4; i++) {
        val = (*(buf+4*i+3) << 24) | (*(buf+4*i+2) << 16) | (*(buf+4*i+1) << 8) | *(buf+4*i);
        val = BHAB_P_SWAP32(val);
        BREG_Write32(p7563->hRegHandle, addr, val);
        addr += 4;
    }

    BREG_Write32(p7563->hRegHandle, addr, val);

    bytes_left = n%4;
    if(bytes_left)
    {
        val = 0;
        for (i = 0; i < bytes_left; i++) {
            val |= *(buf+n/4*4+i) << 8*i;
        }
        val = BHAB_P_SWAP32(val);
        BREG_Write32(p7563->hRegHandle, addr, val);
        addr += 4;
    }

    return retCode;
}


/******************************************************************************
 BHAB_7563_ReadRegister()
******************************************************************************/
BERR_Code BHAB_7563_ReadRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    *val = BREG_Read32(p7563->hRegHandle, reg);

    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_7563_WriteRegister()
******************************************************************************/
BERR_Code BHAB_7563_WriteRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BREG_Write32(p7563->hRegHandle, reg, *val);

    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_7563_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_7563_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
    BHAB_7563_P_Handle *p7563;
    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);

    BKNI_SetEvent_isr(p7563->hApiEvent);
    BKNI_SetEvent_isr(p7563->hInterruptEvent);
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7563_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_7563_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(handle);

    BHAB_P_ACQUIRE_MUTEX(handle);
    BHAB_7563_P_EnableHostInterrupt(handle, false);
    BHAB_CHK_RETCODE(BHAB_7563_P_DecodeInterrupt(handle));
    BHAB_7563_P_EnableHostInterrupt(handle, true);

done:
    if(retCode != BERR_SUCCESS)
        BHAB_7563_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_7563_EnableLockInterrupt()
******************************************************************************/
BERR_Code BHAB_7563_EnableLockInterrupt(
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
 BHAB_7563_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_7563_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p7563->InterruptCallbackInfo[eDevId];

    BKNI_EnterCriticalSection();
    callback->pParm1 = pParm1;
    callback->parm2 = parm2;
    callback->func = fCallBack;
    BKNI_LeaveCriticalSection();

    switch(parm2)
    {
        case 0:
            callback->callbackType |= 0x1;
            break;
        case 1:
            callback->callbackType |= 0x2;
            break;
        case 2:
            callback->callbackType |= 0x4;
            break;
        case 3:
            callback->callbackType |= 0x8;
            break;
        case 5:
            callback->callbackType |= 0x10;
            break;
        default:
            retCode = BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    return retCode;
}


/******************************************************************************
 BHAB_7563_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_7563_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p7563->InterruptCallbackInfo[eDevId];


    BKNI_EnterCriticalSection();
    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->parm2 = 0;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_7563_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_7563_SendHabCommand(
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

    BHAB_7563_P_EnableHostInterrupt(handle, false);

    *(write_buf+write_len-1) = BHAB_7563_P_CRC8Block(crc, write_buf, write_len-1);
    for(retries=0; retries<MAX_HAB_RETRIES; retries++) {
        if(retries)
            BDBG_WRN(("Resending HAB command"));
        /* write the command to the HAB */
        BHAB_CHK_RETCODE(BHAB_7563_P_WriteHab(handle, 0, write_buf, write_len));
        if(bInsertTermination){
            sb=0x1;
            BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_CTRL_HAB_REQUEST, &sb));
        }
        /* wait for the AP to service the HAB, and then read any return data */
        retCode = BHAB_7563_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80);

        if(retCode == BHAB_ERR_HAB_TIMEOUT) {
                BDBG_WRN(("HAB Timeout detected, resetting HAB..."));
                BHAB_7563_P_ErrorRecovery(handle);
                continue;
        }
        else
            goto read_hab;

read_hab:
        crc = 0;
        datalength = (((*(read_buf+1) & 0x3F) << 4) | (*(read_buf+2) >> 4) ) + 4;

        if((read_len != 0) && (*(read_buf+datalength) != BHAB_7563_P_CRC8Block(crc, read_buf, datalength))) {
            BDBG_WRN(("Invalid CRC"));
            retCode = BHAB_ERR_INVALID_CRC;
            continue;
        }
        retCode = BHAB_7563_P_CheckHab(handle);
        if(retCode == BERR_SUCCESS)
            break;
    }

    if((retries==MAX_HAB_RETRIES) && (retCode == BHAB_ERR_HAB_TIMEOUT)){
        BDBG_ERR(("HAB timeout\n"));
        BDBG_WRN(("Dumping 7563 Registers\n"));
        BHAB_7563_P_DumpRegisters(handle);
#ifdef BHAB_DEBUG
        BDBG_ASSERT(false);
#endif
    }
done:
    if(retCode != BERR_SUCCESS)
        BHAB_7563_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);

    return retCode;
}

/******************************************************************************
 BHAB_7563_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_7563_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    BDBG_ASSERT(handle);

    *hEvent = ((BHAB_7563_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}


/****************************Private Functions*********************************/

static uint8_t BHAB_7563_P_CRC8Byte(
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

static uint8_t BHAB_7563_P_CRC8Block(
    uint8_t crc,
    uint8_t *pDataBlock,
    uint32_t dataLength
    )
{
    uint32_t i;

    for ( i = 0; i < dataLength; i++ )
    {
        crc = BHAB_7563_P_CRC8Byte(crc, *pDataBlock++ );
    }

    return crc;
}


/******************************************************************************
 BREG_I2C_P_Read7563()
******************************************************************************/
BERR_Code BREG_I2C_P_Read7563(
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
 BHAB_7563_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_7563_P_EnableHostInterrupt(
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
 BHAB_7563_P_DisableInterrupts()
******************************************************************************/
BERR_Code BHAB_7563_P_DisableInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BERR_7563_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_7563_P_WaitForEvent(
    BHAB_Handle handle,             /* [in] BHAB PI Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    int timeoutMsec            /* [in] timeout in milliseconds */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    /* Temporary fix till B0 7563 chip as that provides maskable Host IRQ register. */
    while(1) {
        BHAB_7563_P_EnableHostInterrupt(handle, true);
        retCode = BKNI_WaitForEvent(p7563->hApiEvent, timeoutMsec);
        if ( retCode != BERR_SUCCESS ) { break; }
        BHAB_7563_P_EnableHostInterrupt(handle, false);
        BHAB_CHK_RETCODE(BHAB_7563_P_DecodeInterrupt(handle));
        retCode = BKNI_WaitForEvent(hEvent, 0);
        if (retCode == BERR_SUCCESS ) { break; }
    }

    BHAB_7563_P_EnableHostInterrupt(handle, true);

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_RunAp()
******************************************************************************/
BERR_Code BHAB_7563_P_RunAp(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_7563_P_EnableHostInterrupt(handle, true);
#ifdef BHAB_LEAP_UART_ENABLE
    buf = 0x80000001;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_CTRL_GP38, &buf));
#else
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_CTRL_GP38, &buf));
#endif
    /* unmask HOST L1 and L2 interrupt bits */
    buf = 0x1;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L1_INTR_MASK_CLEAR, &buf));

    buf = 0xFF000000;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L2_CPU_MASK_CLEAR, &buf));

    /* start running the AP */
    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x0F;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_Reset()
******************************************************************************/
BERR_Code BHAB_7563_Reset(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;
    BHAB_7563_P_Handle *p7563;

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    /* Now clear the reset. */
    buf = 0x08;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, &buf));
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, &buf));

    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_SUN_GISB_ARB_REQ_MASK, &buf));
    buf &= ~0x100;
    BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_SUN_GISB_ARB_REQ_MASK, &buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_7563_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint16_t n)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_7563_ReadMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_7563_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_7563_WriteMemory(handle, (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + addr), buf, n));

done:
    return retCode;
}

/******************************************************************************
 BHAB_7563_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_7563_P_ServiceHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if (BHAB_7563_P_WaitForEvent(handle, p7563->hHabDoneEvent, 10100) == BERR_TIMEOUT)
    {
        retCode = BHAB_ERR_HAB_TIMEOUT;
        goto done;
    }

    if (read_len > 0)
    {
        BHAB_CHK_RETCODE(BHAB_7563_P_ReadHab(handle, 0, read_buf, read_len));
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
 BHAB_7563_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_7563_P_DecodeInterrupt(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_7563_P_Handle *p7563;
    BHAB_P_CallbackInfo *callback;
    uint32_t   buf, mbox_depth, mbox_data;
    uint16_t coreType, coreId;
    uint8_t lockStatus;
    unsigned i;
    char *core = "DVB-T";

    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_HOST_L2_CPU_STATUS, &buf));
    if(buf == 0)
        return retCode;

    buf &= 0xFF000000;

    /*INIT DONE INTERRUPT*/
    if(buf & BHAB_AP_INIT_DONE) {
        BDBG_MSG(("AP INIT DONE"));
        BKNI_SetEvent(p7563->hInitDoneEvent);
        if(buf != 0)
            BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L2_CPU_CLEAR, &buf));
    }
    else if(buf & BHAB_AP_ERROR) { /* AP ERROR */
        BDBG_WRN(("AP ERROR"));
        if(buf != 0)
            BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L2_CPU_CLEAR, &buf));
    }
    else if(buf & BHAB_AP_EVENT) {
        BDBG_MSG(("AP EVENT"));
        for(;;){
            BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_MBOX_FIFO_DEPTH, &mbox_depth));
            if(!mbox_depth)
                break;

            for(i=0; i<mbox_depth; i++) {
                BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_MBOX_FIFO_DATA, &mbox_data));

                if((mbox_data >> 24) == BHAB_EventId_eHabDone) {
                    BDBG_MSG(("HAB_DONE"));
                    BKNI_SetEvent_isr(p7563->hHabDoneEvent);
                    continue;
                }

                if((mbox_data >> 24) == BHAB_EventId_eHabReady)
                    BKNI_SetEvent(p7563->hHabReady);

                coreType = (mbox_data & BHAB_CORE_TYPE_MASK) >> 19;
                coreId = (mbox_data & BHAB_CORE_ID_MASK) >> 11;
                if((mbox_data >> 24) == BHAB_EventId_eLockChange) {
                    if(p7563->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
                        callback = &p7563->InterruptCallbackInfo[BHAB_DevId_eODS0];
                        callback->parm2 = (int) BHAB_Interrupt_eLockChange;
                        BKNI_EnterCriticalSection();
                        callback->func(callback->pParm1, callback->parm2);
                        BKNI_LeaveCriticalSection();
                    }
                    else
                        BDBG_MSG(("LockChange callback not installed. "));

                    switch(coreType) {
                        case BHAB_CoreType_eDvbt:
                            core = "DVB-T";
                            break;
                        case BHAB_CoreType_eDvbt2:
                            core = "DVB-T2";
                            break;
                        case BHAB_CoreType_eDvbc2:
                            core = "DVB-C2";
                            break;
                        default:
                            BDBG_MSG(("Invalid Core Type"));
                            break;
                        }

                    lockStatus = (mbox_data & BHAB_EVENT_DATA_MASK) >> 6;
                    switch(lockStatus) {
                        case BBHAB_LockStatus_eLocked:
                            BDBG_WRN(("%s Locked", core));
                            break;
                        case BBHAB_LockStatus_eUnlocked:
                            BDBG_WRN(("%s Unlocked", core));
                            break;
                        case BBHAB_LockStatus_eNoSignal:
                            BDBG_WRN(("%s No Signal", core));
                            break;
                        default:
                            BDBG_MSG(("Invalid LockChange IRQ"));
                            break;
                        }
                }

                if((mbox_data >> 24) == BHAB_EventId_eStatusReady) {
                    if(p7563->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
                        callback = &p7563->InterruptCallbackInfo[BHAB_DevId_eODS0];
                        callback->parm2 = (int) BHAB_Interrupt_eOdsAsyncStatusReady;
                        BKNI_EnterCriticalSection();
                        callback->func(callback->pParm1, callback->parm2);
                        BKNI_LeaveCriticalSection();
                    }
                    else
                        BDBG_MSG(("ODS callback not installed. "));
                    switch(coreType) {
                        case BHAB_CoreType_eDvbt:
                            BDBG_MSG(("DVB-T Status Ready"));
                            break;
                        case BHAB_CoreType_eDvbt2:
                            BDBG_MSG(("DVB-T2 Status Ready"));
                            break;
                        case BHAB_CoreType_eDvbc2:
                            BDBG_MSG(("DVB-C2 Status Ready"));
                            break;
                        default:
                            BDBG_WRN(("Invalid Core Type"));
                            break;
                    }
                }
            }
        }
        if(buf != 0)
            BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L2_CPU_CLEAR, &buf));
    }
    else if(buf) {
            BDBG_MSG(("Invalid IRQ 0x%x", (buf>>24)));
            BHAB_CHK_RETCODE(BHAB_7563_WriteRegister(handle, BCHP_LEAP_HOST_L2_CPU_CLEAR, &buf));
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_7563_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_7563_P_DecodeError(
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
 BHAB_7563_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_7563_P_CheckHab(
    BHAB_Handle handle    /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_ApStatus status = 0;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_CHK_RETCODE(BHAB_7563_GetApStatus(handle, &status));

    BHAB_CHK_RETCODE(BHAB_7563_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &buf));
    BDBG_MSG(("BHAB_7563_P_CheckHab status = 0x%x", status ));

    if ((status & 0xF) == 0x1) {
        retCode = BERR_SUCCESS;
    }
    else
    {
        BDBG_MSG(("BHAB_7563_P_DecodeError called"));
        retCode = BHAB_7563_P_DecodeError(handle, &status);
    }

done:
    return retCode;
}

/***************************************************************************
 BHAB_7563_GetConfigSettings()
****************************************************************************/
BERR_Code BHAB_7563_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BHAB_READ_DAISY, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[5] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_READ, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, buf, 5, p7563->inBuf, 0, false, true, 5));
    settings->daisyChain = p7563->inBuf[4] & 0x3;

    BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, buf1, 5, p7563->inBuf, 0, false, true, 5));
    settings->enableLoopThrough = p7563->inBuf[4] & 0x4;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 5, p7563->inBuf, 17, false, true, 17));
    settings->tnrApplication = ((p7563->inBuf[4] & 0xC) >> 6) - 1;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 5, p7563->inBuf, 9, false, true, 9));
    settings->rfInputMode = p7563->inBuf[4];

done:
    return retCode;
}

/***************************************************************************
 BHAB_7563_SetConfigSettings()
****************************************************************************/
BERR_Code BHAB_7563_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_WRITE_DAISY, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[9] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_WRITE, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_WRITE, 0xC, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    if(settings->tnrApplication != p7563->tnrApplication) {
        tnrApplication[4] |= ((settings->tnrApplication + 1) << 6);
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 17, p7563->inBuf, 0, false, true, 17));
        p7563->tnrApplication = settings->tnrApplication;
    }

   if(settings->rfInputMode != p7563->rfInputMode) {
       rfInputMode[4] = settings->rfInputMode;
       BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 9, p7563->inBuf, 0, false, true, 9));
   }

    if(settings->daisyChain){
        buf[4] = (((uint8_t)settings->daisyChain & 0x3) - 1) | 0x4;
    }

    BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, buf, 9, p7563->inBuf, 0, false, true, 9));

    p7563->daisyChain = settings->daisyChain;

    if(settings->enableLoopThrough)
    {
        buf1[4] = 0x4;
        BHAB_CHK_RETCODE(BHAB_7563_SendHabCommand(handle, buf1, 9, p7563->inBuf, 0, false, true, 9));
    }

done:
    return retCode;
}


/***************************************************************************
 BHAB_7563_GetInternalGain()
****************************************************************************/
BERR_Code BHAB_7563_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings     /* [out] internal gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[17] = HAB_MSG_HDR(BHAB_INTERNAL_GAIN_READ, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    hab[4] = (inputParams->frequency >> 24);
    hab[5] = (inputParams->frequency >> 16);
    hab[6] = (inputParams->frequency >> 8);
    hab[7] = inputParams->frequency;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 9, p7563->inBuf, 17, false, true, 17 ));
    internalGainSettings->externalGainBypassed = (p7563->inBuf[4] >> 7);
    internalGainSettings->internalGainLoopThrough = (int16_t)((p7563->inBuf[8] << 8) | p7563->inBuf[9])*100/256;
    internalGainSettings->internalGainDaisy = (int16_t)((p7563->inBuf[10] << 8) | p7563->inBuf[11])*100/256;
    internalGainSettings->frequency = (p7563->inBuf[12] << 24) | (p7563->inBuf[13] << 16) | (p7563->inBuf[14] << 8) | p7563->inBuf[15];

done:
    return retCode;
}

/***************************************************************************
 BHAB_7563_GetExternalGain()
****************************************************************************/
BERR_Code BHAB_7563_GetExternalGain(
    BHAB_Handle handle,                               /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p7563->inBuf, 9, false, true, 9 ));
    externalGainSettings->externalGainTotal = (int16_t)((p7563->inBuf[4] << 8) | p7563->inBuf[5])*100/256;
    externalGainSettings->externalGainBypassable = (int16_t)((p7563->inBuf[6] << 8) | p7563->inBuf[7])*100/256;

done:
    return retCode;
}

/***************************************************************************
 BHAB_7563_SetExternalGain()
****************************************************************************/
BERR_Code BHAB_7563_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings   /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    int16_t externalGainTotal, externalGainBypassable;
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    externalGainTotal = externalGainSettings->externalGainTotal*256/100;
    externalGainBypassable = externalGainSettings->externalGainBypassable*256/100;

    buf[4] = (externalGainTotal >> 8);
    buf[5] = externalGainTotal;
    buf[6] = (externalGainBypassable >> 8);
    buf[7] = externalGainBypassable;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 9, p7563->inBuf, 0, false, true, 9 ));

done:
    return retCode;
}

/***************************************************************************
 BHAB_7563_GetTunerChannels()
****************************************************************************/
BERR_Code BHAB_7563_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels)   /* [out] Returns total tuner channels */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BDBG_ASSERT(handle);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p7563->inBuf, 77, false, true, 77));

        numDescriptors =  p7563->inBuf[22];
        handle->totalTunerChannels = p7563->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p7563->inBuf[31+i*16];
            channelNumber = p7563->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p7563->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p7563->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p7563->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p7563->inBuf[35+i*16] & 0x80) >> 7;
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
 BHAB_7563_GetCapabilities()
****************************************************************************/
BERR_Code BHAB_7563_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities    /* [out] Returns chip capabilities */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, CORE_TYPE_GLOBAL, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_7563_P_Handle *p7563;
    BDBG_ASSERT(handle);

    p7563 = (BHAB_7563_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p7563);

    BDBG_ASSERT(handle);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p7563->inBuf, 77, false, true, 77));

        numDescriptors =  p7563->inBuf[22];
        handle->totalTunerChannels = p7563->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p7563->inBuf[31+i*16];
            channelNumber = p7563->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p7563->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p7563->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p7563->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p7563->inBuf[35+i*16] & 0x80) >> 7;
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
