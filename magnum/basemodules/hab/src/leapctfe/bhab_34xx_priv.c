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

#include "bhab_34xx_priv.h"
#include "bchp_hsi.h"
#if (BCHP_CHIP == 3472)
#include "../../b0/bchp_leap_hab_mem.h"
#include "../../b0/bchp_tm.h"
#include "../../b0/bchp_leap_l2.h"
#else
#include "../../a0/bchp_leap_hab_mem.h"
#include "../../a0/bchp_tm.h"
#include "../../a0/bchp_leap_l2.h"
#endif
#if BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE
#define BHAB_P_SWAP32(a) (((a&0xFF)<<24)|((a&0xFF00)<<8)|((a&0xFF0000)>>8)|((a&0xFF000000)>>24))
#else
#define BHAB_P_SWAP32(a) a
#endif

#define SPI_WRITE_SIZE 30

#define MAX_HAB_RETRIES 3

BDBG_MODULE(bhab_34xx_priv);

/* local private functions */
static uint8_t BHAB_34xx_P_CRC8Block(uint8_t crc, uint8_t *pDataBlock, uint32_t dataLength);

/******************************************************************************
 BHAB_34xx_Open()
******************************************************************************/
BERR_Code BHAB_34xx_Open(
    BHAB_Handle *handle,     /* [out] BHAB handle */
    void        *pReg,       /* [in] pointer ot i2c or spi handle */
    const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_Handle hDev;
    BHAB_34xx_P_Handle *h34xxDev = NULL;
    unsigned i;

    BDBG_ASSERT(pDefSettings->interruptEnableFunc);

    hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));

    if(!hDev)
        retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    else
    {
        h34xxDev = (BHAB_34xx_P_Handle *)BKNI_Malloc(sizeof(BHAB_34xx_P_Handle));
        if(h34xxDev)
        {
            BKNI_Memset( h34xxDev, 0x00, sizeof(BHAB_34xx_P_Handle));
            hDev->pImpl = (void*)h34xxDev;

            if(pDefSettings->isSpi) {
               h34xxDev->hSpiRegister = (BREG_SPI_Handle)pReg;
            }
            else {
               h34xxDev->hI2cRegister = (BREG_I2C_Handle)pReg;
            }

            BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));
            hDev->channelCapabilities = NULL;

            /* create events */
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h34xxDev->hInterruptEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h34xxDev->hApiEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h34xxDev->hInitDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h34xxDev->hHabDoneEvent)));
            BHAB_CHK_RETCODE(BKNI_CreateEvent(&(h34xxDev->hHabReady)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h34xxDev->hMutex)));
            BHAB_CHK_RETCODE(BKNI_CreateMutex(&(h34xxDev->hRegAccessMutex)));

            for(i=0; i<BHAB_DevId_eMax; i++){
                h34xxDev->InterruptCallbackInfo[i].func = NULL;
                h34xxDev->InterruptCallbackInfo[i].pParm1 = NULL;
                h34xxDev->InterruptCallbackInfo[i].parm2 = 0;
            }

            BKNI_Memset( &h34xxDev->nmiSettings, 0x00, sizeof(BHAB_NmiSettings));
            BKNI_Memset( &h34xxDev->wdtSettings, 0x00, sizeof(BHAB_WatchDogTimerSettings));

            h34xxDev->isSpi = pDefSettings->isSpi;
            h34xxDev->tnrApplication = BHAB_TunerApplication_eLast; /* set to invalid option */
            h34xxDev->rfInputMode = BHAB_RfInputMode_eLast; /* set to invalid option */

            retCode = BHAB_34xx_P_DisableInterrupts(hDev);
        }
        else
            retCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
done:
    if(retCode == BERR_SUCCESS)
        *handle = hDev;
    else
    {
        if(h34xxDev)
        {
            BKNI_Free(h34xxDev);
            h34xxDev = NULL;
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
 BHAB_34xx_Close()
******************************************************************************/
BERR_Code BHAB_34xx_Close(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint32_t buf;

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    /* If we didn't load the AP when starting, don't reset it when closing */
    if (p34xx->loadAP)
    {
        /* reset all interrupt status */
        buf = 0xFFFFFFFF;
        BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    }

    retCode = BHAB_34xx_P_DisableInterrupts(handle);
done:
    if (p34xx->hInterruptEvent) {BKNI_DestroyEvent(p34xx->hInterruptEvent);}
    if (p34xx->hApiEvent){BKNI_DestroyEvent(p34xx->hApiEvent);}
    if (p34xx->hInitDoneEvent){BKNI_DestroyEvent(p34xx->hInitDoneEvent);}
    if (p34xx->hHabDoneEvent){BKNI_DestroyEvent(p34xx->hHabDoneEvent);}
    if (p34xx->hHabReady){BKNI_DestroyEvent(p34xx->hHabReady);}
    if (p34xx->hMutex){BKNI_DestroyMutex(p34xx->hMutex);}
    if (p34xx->hRegAccessMutex){BKNI_DestroyMutex(p34xx->hRegAccessMutex);}

    BKNI_Free((void*)p34xx);
    if(handle->channelCapabilities != NULL)
        BKNI_Free((void*)handle->channelCapabilities);
    BKNI_Free((void*)handle);

    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_ErrorRecovery()
******************************************************************************/
BERR_Code BHAB_34xx_P_ErrorRecovery(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t val=0;

    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    /* Clear the "HAB Done" bit before initiating "HAB Reset" */
    val = BHAB_HAB_DONE;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &val));
    val = BHAB_HAB_RESET;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_L2_CPU_SET, &val));

    if (BHAB_34xx_P_WaitForEvent(handle, p34xx->hHabReady, 1500) == BERR_TIMEOUT)
    {
        BDBG_ERR(("########################## Unable to reset HAB"));
        retCode = BHAB_ERR_HAB_TIMEOUT;
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_DumpRegisters()
******************************************************************************/
BERR_Code BHAB_34xx_P_DumpRegisters(
    BHAB_Handle handle
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t i, buf=0;

    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT7, &buf));
    BDBG_WRN(("BCHP_TM_SFT7 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT6, &buf));
    BDBG_WRN(("BCHP_TM_SFT6 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT5, &buf));
    BDBG_WRN(("BCHP_TM_SFT5 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT4, &buf));
    BDBG_WRN(("BCHP_TM_SFT4 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT3, &buf));
    BDBG_WRN(("BCHP_TM_SFT3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT2, &buf));
    BDBG_WRN(("BCHP_TM_SFT2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT1, &buf));
    BDBG_WRN(("BCHP_TM_SFT1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_SFT0, &buf));
    BDBG_WRN(("BCHP_TM_SFT0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_MISC0, &buf));
    BDBG_WRN(("BCHP_TM_MISC0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_MISC1, &buf));
    BDBG_WRN(("BCHP_TM_MISC1 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_MISC2, &buf));
    BDBG_WRN(("BCHP_TM_MISC2 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_MISC3, &buf));
    BDBG_WRN(("BCHP_TM_MISC3 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, 0x78000, &buf));
    BDBG_WRN(("HAB first word %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, 0x78004, &buf));
    BDBG_WRN(("HAB second word %x", buf));
    for(i=0; i < 1; i++) {
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE0 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE1, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE1 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE2, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE2 %x", buf));
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE3, &buf));
    BDBG_WRN(("BCHP_LEAP_CTRL_SW_SPARE3 %x", buf));
    BKNI_Sleep(250);
    }
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_CLEAR0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_STATUS0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_SET0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_SET0 %x", buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &buf));
    BDBG_WRN(("BCHP_LEAP_HOST_IRQ_MASK_CLEAR0 %x", buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_InitAp()
******************************************************************************/
BERR_Code BHAB_34xx_InitAp(
    BHAB_Handle handle,       /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM34xx microcode image */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint32_t n, instlen=0, datalen, instaddr = 0x0, dataaddr;
    const uint8_t *pImage = NULL;
    uint8_t retries=0, count=0;

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    /* We are loading AP code, it is appropriate to reset the AP when closing */
    p34xx->loadAP = true;

    /* Disable host interrupt */
    BHAB_CHK_RETCODE(BHAB_34xx_P_EnableHostInterrupt(handle, false));
    /* reset the AP before downloading the microcode */
    BHAB_CHK_RETCODE(BHAB_34xx_Reset(handle));

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
                retCode = BHAB_34xx_WriteMemory(handle, instaddr, &pImage[19], instlen);
                if (retCode == BERR_SUCCESS)
                    break;
                else
                    BDBG_WRN(("BHAB_34xx_WriteMemory() retCode 0x%X", retCode));
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
        BHAB_CHK_RETCODE(BHAB_34xx_P_EnableInitDoneInterrupt(handle));

        /* start running the AP */
        if ((retCode = BHAB_34xx_P_RunAp(handle)) == BERR_SUCCESS)
        {
            for (count = 0; count < BHAB_INIT_RETRIES; count++)
            {
                /* wait for init done interrupt */
                if ((retCode = BHAB_34xx_P_WaitForEvent(handle, p34xx->hInitDoneEvent, 150)) == BERR_SUCCESS)
                    break;
                /* Disable host interrupt */
                if ((retCode = BHAB_34xx_P_EnableHostInterrupt(handle, false)) != BERR_SUCCESS)
                    BDBG_WRN(("Failed disabling Host Interrupt"));
            }

            if (count < BHAB_INIT_RETRIES)
                retCode = BERR_SUCCESS;
            else
            {
                /* reset the AP before exiting */
                BHAB_CHK_RETCODE(BHAB_34xx_Reset(handle));
                BKNI_Sleep(10);
                BDBG_ERR(("AP initialization timeout. Starting over %d times", count));
                BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
            }
        }
    }

done:
    if(retCode != BERR_SUCCESS)
        BHAB_34xx_P_EnableHostInterrupt(handle, true);
    return retCode;
}


/******************************************************************************
 BHAB_34xx_GetApStatus()
******************************************************************************/
BERR_Code BHAB_34xx_GetApStatus(
   BHAB_Handle handle,      /* [in] HAB device handle */
   BHAB_ApStatus *pStatus   /* [out] AP status */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t sb;

    BDBG_ASSERT(handle);

    *pStatus = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb));
    *pStatus = sb;

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_GetApVersion()
******************************************************************************/
BERR_Code BHAB_34xx_GetApVersion(
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
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_FAMILY_ID, pFamilyId));

    buf[0] = ((BHAB_GETVERSION >> 2) & 0xFF);
    buf[1] = (((BHAB_GETVERSION & 0x3) << 6) | ((PAYLOAD_LEN >> 4) & 0x3F));
    buf[2] = ((PAYLOAD_LEN & 0xF) << 4);
    buf[3] = CORE_ID;
    buf[4] = 0;    /* CRC */

    BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf, 5, p34xx->inBuf, 21, false, true, 21));
    *pChipId = ((p34xx->inBuf[4] << 8) | p34xx->inBuf[5]);
    *pChipVer = ((p34xx->inBuf[6] << 8) | p34xx->inBuf[7]);
    *pMajApVer = p34xx->inBuf[8];
    *pMinApVer = p34xx->inBuf[9];

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
 BHAB_34xx_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_34xx_GetVersionInfo(
    BHAB_Handle             handle,         /* [in]  BHAB handle */
    BFEC_SystemVersionInfo  *pVersionInfo   /* [out]  Returns FW version information */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[21] = HAB_MSG_HDR(BHAB_GETVERSION, 0, CORE_TYPE, CORE_ID);
    uint8_t sysVersion[73] = HAB_MSG_HDR(BHAB_GET_SYS_VERSION, 0, CORE_TYPE, CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_TM_FAMILY_ID, &pVersionInfo->familyId));

    BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf, 5, p34xx->inBuf, 21, false, true, 21));
    pVersionInfo->chipId = ((p34xx->inBuf[4] << 8) | p34xx->inBuf[5]);
    pVersionInfo->chipVersion = ((p34xx->inBuf[6] << 8) | p34xx->inBuf[7]);
    pVersionInfo->firmware.majorVersion = p34xx->inBuf[8];
    pVersionInfo->firmware.minorVersion = p34xx->inBuf[9];

    if((pVersionInfo->firmware.majorVersion >= 6) || ((pVersionInfo->firmware.majorVersion >= 5) & ( pVersionInfo->firmware.minorVersion>= 3)))
    {
        BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, sysVersion, 5, p34xx->inBuf, 73, false, true, 73));
        pVersionInfo->familyId = (p34xx->inBuf[4] << 24) | (p34xx->inBuf[5] << 16) | (p34xx->inBuf[6] << 8) | p34xx->inBuf[7];
        pVersionInfo->chipId = (p34xx->inBuf[8] << 24) | (p34xx->inBuf[9] << 16) | (p34xx->inBuf[10] << 8) | p34xx->inBuf[11];
        pVersionInfo->chipVersion = (p34xx->inBuf[12] << 24) | (p34xx->inBuf[13] << 16) | (p34xx->inBuf[14] << 8) | p34xx->inBuf[15];
        pVersionInfo->firmware.majorVersion = ((p34xx->inBuf[16] << 8) | p34xx->inBuf[17]);
        pVersionInfo->firmware.minorVersion = ((p34xx->inBuf[18] << 8) | p34xx->inBuf[19]);
        pVersionInfo->firmware.buildType = ((p34xx->inBuf[20] << 8) | p34xx->inBuf[21]);
        pVersionInfo->firmware.buildId = ((p34xx->inBuf[22] << 8) | p34xx->inBuf[23]);
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
 BHAB_34xx_ReadMemory()
******************************************************************************/
BERR_Code BHAB_34xx_ReadMemory(BHAB_Handle handle, uint32_t addr, uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint8_t sb, data, readbuf[8], writebuf[8];
    uint32_t sb1;
    uint16_t bytes_left, offset;
    uint8_t i=0;

    BDBG_ASSERT(handle);
    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);

    if (((uint32_t)addr + (uint32_t)n) > (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + BHAB_HAB_SIZE))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    sb1 = BHAB_P_SWAP32(addr);

    if(p34xx->isSpi){
        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);

        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 6));

        writebuf[0] = (handle->settings.chipAddr << 1);
        for (offset = 0; offset < n/4 * 4; offset+=4) {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister, writebuf, readbuf, 6));
            *buf++ = readbuf[2];
            *buf++ = readbuf[3];
            *buf++ = readbuf[4];
            *buf++ = readbuf[5];

            for(i=0; i < 5; i++){
                writebuf[1] = CSR_STATUS;
                BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister, writebuf, readbuf, 3));
                if (readbuf[2] == BHAB_CPU_RUNNIG)
                    break;
            }
            if((i==5) &&(readbuf[2] & 0x1))
                BDBG_WRN(("Read transaction not finished"));
        }
        /* Read the bytes left */
        bytes_left = n%4;
        if(bytes_left)
        {
            writebuf[1] = CSR_RBUS_DATA0;
            BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister, writebuf, readbuf, 2+bytes_left));
            for(i=0; i<bytes_left; i++) {
                *buf++ = readbuf[2+i];
            }
        }

        /* set it back to the default write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 3));
    }
    else {
        /* enable read mode and speculative read*/
        data = CSR_CONFIG_READ_RBUS_READ | (CSR_CONFIG_SPECULATIVE_READ_EN_BITS << CSR_CONFIG_SPECULATIVE_READ_EN_SHIFT);
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        /* set CSR pointer to point to ADDR0 and set RBUS address to read */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        for (offset = 0; offset < n; offset+=4) {
            BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf+offset,
                n - offset < 4 ? n - offset : 4));
            for(i=0; i < 5; i++){
                BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
                if(sb == BHAB_CPU_RUNNIG)
                    break;
            }
            if(i==5)
                BDBG_WRN(("Write transaction not finished"));
        }

        data =0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_WriteMemory()
******************************************************************************/
BERR_Code BHAB_34xx_WriteMemory(BHAB_Handle handle, uint32_t addr, const uint8_t *buf, uint32_t n)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint8_t pad, i=0, k=0;
    uint16_t bytes_left=0, orig_bytes_left=0;
    uint32_t sb1, curr_addr, j=0;
    uint8_t readbuf[8], writebuf[16];
    BREG_SPI_Data spiData[2];

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    sb1 = BHAB_P_SWAP32(addr);

    if ((addr + n) >= (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE +128))
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    if(p34xx->isSpi)
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 3));

        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 6));

        writebuf[1] = CSR_RBUS_DATA0;

        spiData[0].data = (void *)writebuf;
        spiData[0].length = 2;
        spiData[1].data = (void *)buf;
        spiData[1].length = n;
        BHAB_CHK_RETCODE(BREG_SPI_Multiple_Write(p34xx->hSpiRegister, spiData, 2));

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        orig_bytes_left = (n%4);

        if(orig_bytes_left)
        {
            bytes_left = 4-orig_bytes_left;
            writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;

            switch (n%4)
            {
                case 1:
                    writebuf[1] = CSR_RBUS_DATA1;
                    break;
                case 2:
                    writebuf[1] = CSR_RBUS_DATA2;
                    break;
                case 3:
                    writebuf[1] = CSR_RBUS_DATA3;
                    break;
                default:
                    writebuf[1] = CSR_RBUS_DATA0;
                  break;
            }

            for(k=0; k<bytes_left; k++)
            {
                writebuf[2+k] = 0;
            }
            BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 2+bytes_left));
        }

        /* check for host transfer error */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & CSR_STATUS_ERROR_BITS) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else
    {
        /* set CSR pointer to point to ADDR0 and set RBUS address to zero */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* set CSR pointer to point to DATA0 and provide the data to be downloaded */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, n));

        /* pad with zeros at the end, so that an RBUS write can be triggered for the last word */
        bytes_left = n%4;
        if(bytes_left)
        {
            pad = 0;
            curr_addr = CSR_RBUS_DATA0 + bytes_left;
            for(i=0; i<(4-bytes_left); i++)
            {
                BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, curr_addr++, &pad, 1));
            }
        }
    }
done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_ReadMbox()
******************************************************************************/
BERR_Code BHAB_34xx_ReadMbox(
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
 BHAB_34xx_WriteMbox()
******************************************************************************/
BERR_Code BHAB_34xx_WriteMbox(
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
 BHAB_34xx_ReadRegister()
******************************************************************************/
BERR_Code BHAB_34xx_ReadRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [out] value read from register */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint8_t data, sb, i=0;
    uint32_t sb1, buf, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    sb1 = BHAB_P_SWAP32(reg);

    if(p34xx->isSpi)
    {
        readbuf[2] = 0;

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = 0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister, writebuf, 3));

        /* enable read mode and speculative read */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |=  ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister,  writebuf, readbuf, 3));
             if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Read transaction not finished"));

        /* read data*/
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_RBUS_DATA0;
        BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister, writebuf, readbuf, 6));

        value = ((uint32_t)(readbuf[2] << 0) | (uint32_t)(readbuf[3] << 8) | (uint32_t)(readbuf[4] << 16) | (uint32_t)(readbuf[5] << 24));
        *val = BHAB_P_SWAP32(value);

        /* set READ_RBUS to the reset value for write mode */
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = CSR_CONFIG;
        writebuf[2] = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_SPI_Write(p34xx->hSpiRegister,  writebuf, 3));
    }
    else
    {
        data=0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, (uint8_t *)&data, 1));

        /* read CSR_CONFIG value*/
        BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
        data &= ~CSR_CONFIG_READ_RBUS_MASK;
        data |= ((CSR_CONFIG_READ_RBUS_READ << CSR_CONFIG_READ_RBUS_SHIFT) | (1 << CSR_CONFIG_NO_RBUS_ADDR_INC_SHIFT));

        /* set READ_RBUS to the reset value for read mode */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));

        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
            if ((sb & CSR_STATUS_BUSY_MASK) == 0)
                break;
        }
        if(i==5) {
            BDBG_WRN(("Read transaction not finished"));
            retCode = BERR_TIMEOUT;
            goto done;
        }
        else if (sb & BHAB_CSR_STATUS_ERROR_MASK)
        {
            BDBG_ERR(("CSR_STATUS error 0x%X", sb));
            retCode = BHAB_ERR_HOST_XFER;
            goto done;
        }
        /* read the data */
        BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&buf, 4));
        *val = (uint32_t)BHAB_P_SWAP32(buf);

        /* set READ_RBUS to the reset value for write mode */
        data = CSR_CONFIG_READ_RBUS_WRITE;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_CONFIG, &data, 1));
    }
done:
    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_34xx_WriteRegister()
******************************************************************************/
BERR_Code BHAB_34xx_WriteRegister(
    BHAB_Handle handle,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    uint8_t sb, buf[4], i=0;
    uint32_t sb1, value=0;
    uint8_t readbuf[8], writebuf[8];

    BDBG_ASSERT(handle);
    BHAB_P_ACQUIRE_REG_ACCESS_MUTEX(handle);
    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);
    BSTD_UNUSED(reg);

    sb1 = BHAB_P_SWAP32(reg);
    if(p34xx->isSpi)
    {
        /* enable write mode */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_CONFIG;
        BHAB_CHK_RETCODE(BREG_SPI_Read (p34xx->hSpiRegister, writebuf, readbuf, 3));

        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        readbuf[2] &= ~CSR_CONFIG_READ_RBUS_MASK;
        readbuf[2] |= (CSR_CONFIG_READ_RBUS_WRITE << CSR_CONFIG_READ_RBUS_SHIFT);
        writebuf[2] = readbuf[2];
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 3));

        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        writebuf[1] = CSR_RBUS_ADDR0;
        writebuf[2] = sb1;
        writebuf[3] = (sb1 >> 8);
        writebuf[4] = (sb1 >> 16);
        writebuf[5] = (sb1 >> 24);
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 6));

        /* write to RBUS*/
        writebuf[1] = CSR_RBUS_DATA0;
        value = BHAB_P_SWAP32(*(uint32_t *)val);
        writebuf[2] = (value & 0xff) >> 0;
        writebuf[3] = (value & 0xff00) >> 8;
        writebuf[4] = (value & 0xff0000) >> 16;
        writebuf[5] = (value & 0xff000000) >> 24;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 6));

        /* poll the busy bit to make sure the transaction is completed */
        writebuf[0] = (handle->settings.chipAddr << 1);
        writebuf[1] = CSR_STATUS;
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_SPI_Read(p34xx->hSpiRegister,  writebuf, readbuf, 3));
            if ((readbuf[2] & (1 << CSR_STATUS_BUSY_SHIFT)) == 0)
                break;
        }
        if(i==5)
            BDBG_WRN(("Write transaction not finished"));
    }
    else {
        /* set CSR pointer to point to ADDR0 and set RBUS address to write */
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_ADDR0, (uint8_t *)&sb1, 4));
        sb1 = BHAB_P_SWAP32(*(uint32_t *)val);
        /* write to RBUS*/
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, (uint8_t *)&sb1, 4));

        BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_RBUS_DATA0, buf, 4));

        /* poll the busy bit to make sure the transaction is completed */
        for(i=0; i < 5; i++){
            BHAB_CHK_RETCODE(BREG_I2C_P_Read34xx(p34xx->hI2cRegister, handle->settings.chipAddr, CSR_STATUS, &sb, 1));
            if ((sb & CSR_STATUS_BUSY_MASK) == 0)
                break;
        }
        if(i==5) {
            BDBG_WRN(("Write transaction not finished"));
            retCode = BERR_TIMEOUT;
            goto done;
        }
        else if (sb & BHAB_CSR_STATUS_ERROR_MASK)
        {
            BDBG_ERR(("CSR_STATUS error 0x%X", sb));
            retCode = BHAB_ERR_HOST_XFER;
        }
    }

done:
    BHAB_P_RELEASE_REG_ACCESS_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_34xx_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_34xx_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
    BHAB_34xx_P_Handle *p34xx;
    BKNI_ASSERT_ISR_CONTEXT();

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);

    BKNI_SetEvent_isr(p34xx->hApiEvent);
    BKNI_SetEvent_isr(p34xx->hInterruptEvent);
    return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_34xx_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_34xx_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BDBG_ASSERT(handle);

    BHAB_P_ACQUIRE_MUTEX(handle);
    BHAB_34xx_P_EnableHostInterrupt(handle, false);
    BHAB_CHK_RETCODE(BHAB_34xx_P_DecodeInterrupt(handle));
    BHAB_34xx_P_EnableHostInterrupt(handle, true);

done:
    if(retCode != BERR_SUCCESS)
        BHAB_34xx_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);
    return retCode;
}


/******************************************************************************
 BHAB_34xx_EnableLockInterrupt()
******************************************************************************/
BERR_Code BHAB_34xx_EnableLockInterrupt(
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
 BHAB_34xx_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_34xx_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_34xx_P_Handle *p34xx;

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p34xx->InterruptCallbackInfo[eDevId];

    BKNI_EnterCriticalSection();
    callback->pParm1 = pParm1;
    callback->parm2 = parm2;
    callback->func = fCallBack;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_34xx_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_34xx_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_P_CallbackInfo *callback;
    BHAB_34xx_P_Handle *p34xx;

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if (eDevId >= BHAB_DevId_eMax) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    callback = &p34xx->InterruptCallbackInfo[eDevId];


    BKNI_EnterCriticalSection();
    callback->func = NULL;
    callback->pParm1 = NULL;
    callback->parm2 = 0;
    BKNI_LeaveCriticalSection();

    return retCode;
}


/******************************************************************************
 BHAB_34xx_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_34xx_SendHabCommand(
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

    BHAB_34xx_P_EnableHostInterrupt(handle, false);

    *(write_buf+write_len-1) = BHAB_34xx_P_CRC8Block(crc, write_buf, write_len-1);
    for(retries=0; retries<MAX_HAB_RETRIES; retries++) {
        if(retries)
            BDBG_WRN(("Resending HAB command"));
        /* write the command to the HAB */
        BHAB_CHK_RETCODE(BHAB_34xx_P_WriteHab(handle, 0, write_buf, write_len));
        if(bInsertTermination){
            sb=0x1;
            BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_CTRL_HAB_REQUEST, &sb));
        }
        /* wait for the AP to service the HAB, and then read any return data */
        retCode = BHAB_34xx_P_ServiceHab(handle, read_buf, read_len, bCheckForAck, write_buf[0] | 0x80);

        if(retCode == BHAB_ERR_HAB_TIMEOUT) {
                BDBG_WRN(("HAB Timeout detected, resetting HAB..."));
                BHAB_34xx_P_ErrorRecovery(handle);
                continue;
        }
        else
            goto read_hab;

read_hab:
        crc = 0;
        datalength = (((*(read_buf+1) & 0x3F) << 4) | (*(read_buf+2) >> 4) ) + 4;

        if((read_len != 0) && (*(read_buf+datalength) != BHAB_34xx_P_CRC8Block(crc, read_buf, datalength))) {
            BDBG_WRN(("Invalid CRC"));
            retCode = BHAB_ERR_INVALID_CRC;
            continue;
        }

        retCode = BHAB_34xx_P_CheckHab(handle);
        if(retCode == BERR_SUCCESS)
            break;
    }

    if((retries==MAX_HAB_RETRIES) && (retCode == BHAB_ERR_HAB_TIMEOUT)){
        BDBG_ERR(("HAB timeout"));
        BDBG_WRN(("Dumping 34xx Registers"));
        BHAB_34xx_P_DumpRegisters(handle);
#ifdef BHAB_DEBUG
        BDBG_ASSERT(false);
#endif
    }
done:
    if(retCode != BERR_SUCCESS)
        BHAB_34xx_P_EnableHostInterrupt(handle, true);
    BHAB_P_RELEASE_MUTEX(handle);

    return retCode;
}

/******************************************************************************
 BHAB_34xx_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_34xx_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    BDBG_ASSERT(handle);

    *hEvent = ((BHAB_34xx_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}


/****************************Private Functions*********************************/

static uint8_t BHAB_34xx_P_CRC8Byte(
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

static uint8_t BHAB_34xx_P_CRC8Block(
    uint8_t crc,
    uint8_t *pDataBlock,
    uint32_t dataLength
    )
{
    uint32_t i;

    for ( i = 0; i < dataLength; i++ )
    {
        crc = BHAB_34xx_P_CRC8Byte(crc, *pDataBlock++ );
    }

    return crc;
}


/******************************************************************************
 BREG_I2C_P_Read34xx()
******************************************************************************/
BERR_Code BREG_I2C_P_Read34xx(
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
 BHAB_34xx_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_34xx_P_EnableHostInterrupt(
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
 BHAB_34xx_P_DisableInterrupts()
******************************************************************************/
BERR_Code BHAB_34xx_P_DisableInterrupts(
    BHAB_Handle handle   /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_EnableHabDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_34xx_P_EnableHabDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_EnableInitDoneInterrupt()
******************************************************************************/
BERR_Code BHAB_34xx_P_EnableInitDoneInterrupt(
    BHAB_Handle handle /* [in] BHAB PI Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSTD_UNUSED(handle);

    return retCode;
}


/******************************************************************************
 BERR_34xx_Code BHAB_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_34xx_P_WaitForEvent(
    BHAB_Handle handle,             /* [in] BHAB PI Handle */
    BKNI_EventHandle hEvent,   /* [in] event to wait on */
    int timeoutMsec            /* [in] timeout in milliseconds */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    while (1)
    {
        BHAB_34xx_P_EnableHostInterrupt(handle, true);
        retCode = BKNI_WaitForEvent(p34xx->hApiEvent, timeoutMsec);
        if ( retCode != BERR_SUCCESS ) { break; }
        BHAB_34xx_P_EnableHostInterrupt(handle, false);
        BHAB_34xx_P_DecodeInterrupt(handle);

        retCode = BKNI_WaitForEvent(hEvent, 0);
        if (retCode == BERR_SUCCESS || retCode == BERR_OS_ERROR) { break; }
    }

    BHAB_34xx_P_EnableHostInterrupt(handle, true);

    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_RunAp()
******************************************************************************/
BERR_Code BHAB_34xx_P_RunAp(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;

    BDBG_ASSERT(handle);

    BHAB_34xx_P_EnableHostInterrupt(handle, true);
#ifdef BHAB_LEAP_UART_ENABLE
    buf = 0x80000001;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_CTRL_GP38, &buf));
#else
    buf = 0;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_CTRL_GP38, &buf));
#endif
    /* start running the AP */
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x02;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_Reset()
******************************************************************************/
BERR_Code BHAB_34xx_Reset(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint32_t buf;
    uint8_t buf1;
    BHAB_34xx_P_Handle *p34xx;
    uint8_t writebuf[8];

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);
    /* clear LEAP_HOST_IRQ and put LEAP in reset */
    buf=0xFFFFFFFF;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));

    if(p34xx->isSpi){
        writebuf[0] = (handle->settings.chipAddr << 1) | 0x1;
        writebuf[1] = HIF_SFT_RST;
        writebuf[2] = 0xFD;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 3));

        writebuf[2] = 0x0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 3));

        /* 3472 LEAP memories to be powered down by default, make sure to power them on before downloading the FW */
#if (BHAB_CHIP == 3472)
        writebuf[1] = HIF_MEM_CTRL;
        writebuf[2] = 0x2;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 3));
        BKNI_Sleep(100);
        writebuf[2] = 0x0;
        BHAB_CHK_RETCODE(BREG_SPI_Write (p34xx->hSpiRegister,  writebuf, 3));
#endif
    }
    else {
        buf1 = 0xFD;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));
        buf1 = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, HIF_SFT_RST, (uint8_t *)&buf1, 1));

        /* 3472 LEAP memories to be powered down by default, make sure to power them on before downloading the FW */
#if (BHAB_CHIP == 3472)
        buf1 = 0x02;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, HIF_MEM_CTRL, (uint8_t *)&buf1, 1));
        BKNI_Sleep(1);
        buf1 = 0;
        BHAB_CHK_RETCODE(BREG_I2C_Write(p34xx->hI2cRegister, handle->settings.chipAddr, HIF_MEM_CTRL, (uint8_t *)&buf1, 1));
#endif
    }

    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
    buf &= ~0x02;
    buf |= 0x02;
    BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_CTRL_CTRL, &buf));
done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_34xx_P_ReadHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint16_t n)
{

    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_34xx_ReadMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_WriteHab()
******************************************************************************/
BERR_Code BHAB_34xx_P_WriteHab(BHAB_Handle handle, uint8_t addr, uint8_t *buf, uint8_t n)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ASSERT(handle);

    if ((addr + n) > BHAB_HAB_SIZE)
        return BERR_TRACE(BERR_INVALID_PARAMETER);

    BHAB_CHK_RETCODE(BHAB_34xx_WriteMemory(handle, (BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + addr), buf, n));

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_34xx_P_ServiceHab(
    BHAB_Handle handle,   /* [in] BHAB PI Handle */
    uint8_t *read_buf,  /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,  /* [in] true = determine if the AP has serviced the command */
    uint8_t ack_byte    /* [in] value of the ack byte to expect */
)
{
    BERR_Code retCode = BERR_SUCCESS;

    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if (BHAB_34xx_P_WaitForEvent(handle, p34xx->hHabDoneEvent, 10100) == BERR_TIMEOUT)
    {
        retCode = BHAB_ERR_HAB_TIMEOUT;
        goto done;
    }

    if (read_len > 0)
    {
        BHAB_CHK_RETCODE(BHAB_34xx_P_ReadHab(handle, 0, read_buf, read_len));
        if (bCheckForAck)
        {
            if (ack_byte != read_buf[0])
            {
                BDBG_ERR(("HAB command not serviced!"));
                BERR_TRACE(retCode = BHAB_ERR_HAB_NO_ACK);
            }
        }
    }

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_34xx_P_DecodeInterrupt(BHAB_Handle handle)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_34xx_P_Handle *p34xx;
    BHAB_P_CallbackInfo *callback;
    uint32_t   buf, buf1, sb, sb1;

    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS0, &buf));
    BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_HOST_IRQ_STATUS1, &buf1));

    if((buf == 0) &&  (buf1 == 0))
        return retCode;

    /*HAB DONE INTERRUPT*/
    if (buf & BHAB_HAB_DONE)
    {
        BDBG_MSG(("HAB_DONE"));
        BKNI_SetEvent(p34xx->hHabDoneEvent);
    }

    if (buf & BHAB_HAB_READY)
        BKNI_SetEvent_isr(p34xx->hHabReady);

    /*INIT DONE INTERRUPT*/
    if (buf & BHAB_AP_INIT_DONE)
    {
        BDBG_MSG(("AP INIT DONE"));
        BKNI_SetEvent(p34xx->hInitDoneEvent);
    }

    if((buf & BHAB_ADS_NO_SIGNAL) == BHAB_ADS_OUT_OF_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS Unlocked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0];
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
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_WRN(("ADS Locked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0];
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
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_MSG(("ADS No Signal"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0];
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
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0].func){
            BDBG_MSG(("ADS Status Ready"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eADS0];
            callback->parm2 = (int) BHAB_Interrupt_eQamAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("ADS callback not installed. "));
        }
    }

    if((buf1 & BHAB_DVBC2_NO_SIGNAL) == BHAB_DVBC2_OUT_OF_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("DVBC2 Unlocked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBC2 callback not installed. "));
        }
    }

    if((buf1 & BHAB_DVBC2_NO_SIGNAL) == BHAB_DVBC2_IN_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("DVBC2 Locked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBC2 callback not installed. "));
        }
    }

    if((buf1 & BHAB_DVBC2_NO_SIGNAL) == BHAB_DVBC2_NO_SIGNAL)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_MSG(("DVBC2 No Signal"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBC2 callback not installed. "));
        }
    }

    if (buf & BHAB_DVBC2_STATUS_RDY)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_MSG(("DVBC2 Status Ready"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eOdsAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBC2 callback not installed. "));
        }
    }

    if((buf & BHAB_DVBT2_NO_SIGNAL) == BHAB_DVBT2_OUT_OF_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("DVBT2 Unlocked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBT2 callback not installed. "));
        }
    }

    if((buf & BHAB_DVBT2_NO_SIGNAL) == BHAB_DVBT2_IN_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("DVBT2 Locked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBT2 callback not installed. "));
        }
    }

    if((buf & BHAB_DVBT2_NO_SIGNAL) == BHAB_DVBT2_NO_SIGNAL)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_MSG(("DVBT2 No Signal"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBT2 callback not installed. "));
        }
    }

    if (buf & BHAB_DVBT2_STATUS_RDY)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_MSG(("DVBT2 Status Ready"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eOdsAsyncStatusReady;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("DVBT2 callback not installed. "));
        }
    }

    if((buf & BHAB_THD_CHN0_NO_SIGNAL) == BHAB_THD_CHN0_OUT_OF_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("THD channel 0 Unlocked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf & BHAB_THD_CHN0_NO_SIGNAL) == BHAB_THD_CHN0_IN_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("THD channel 0 Locked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf & BHAB_THD_CHN0_NO_SIGNAL) == BHAB_THD_CHN0_NO_SIGNAL)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_MSG(("THD channel 0 No Signal"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf1 & BHAB_THD_CHN1_NO_SIGNAL) == BHAB_THD_CHN1_OUT_OF_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1].func){
            BDBG_WRN(("THD channel 1 Unlocked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf1 & BHAB_THD_CHN1_NO_SIGNAL) == BHAB_THD_CHN1_IN_LOCK)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1].func){
            BDBG_WRN(("THD channel 1 Locked"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1];
            callback->parm2 = (int) BHAB_Interrupt_eLockChange;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("THD callback not installed. "));
        }
    }

    if((buf1 & BHAB_THD_CHN1_NO_SIGNAL) == BHAB_THD_CHN1_NO_SIGNAL)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1].func){
            BDBG_MSG(("THD channel 1 No Signal"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1];
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
        BHAB_CHK_RETCODE(BHAB_34xx_ReadRegister(handle, BCHP_LEAP_CTRL_SW_SPARE0, &sb1));

        if (sb1 & BHAB_THD_CHN0_STATUS_RDY)
        {
            if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
                BDBG_MSG(("THD channel 0 Status Ready"));
                callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
                callback->parm2 = (int) BHAB_Interrupt_eOdsAsyncStatusReady;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            else {
                BDBG_MSG(("THD callback not installed. "));
            }
        }

        if (sb1 & BHAB_THD_CHN1_STATUS_RDY)
        {
            if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1].func){
                BDBG_MSG(("THD channel 1 Status Ready"));
                callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0+1];
                callback->parm2 = (int) BHAB_Interrupt_eOdsAsyncStatusReady;
                BKNI_EnterCriticalSection();
                callback->func(callback->pParm1, callback->parm2);
                BKNI_LeaveCriticalSection();
            }
            else {
                BDBG_MSG(("THD callback not installed. "));
            }
        }
    }

#if (BHAB_CHIP == 3472)
    if(buf1 & BHAB_EWS)
    {
        if(p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0].func){
            BDBG_WRN(("Emergency Warning"));
            callback = &p34xx->InterruptCallbackInfo[BHAB_DevId_eODS0];
            callback->parm2 = (int) BHAB_Interrupt_eEmergencyWarningSystem;
            BKNI_EnterCriticalSection();
            callback->func(callback->pParm1, callback->parm2);
            BKNI_LeaveCriticalSection();
        }
        else {
            BDBG_MSG(("EWS callback not installed. "));
        }
    }
#endif
    /* clear LEAP_HOST_IRQ and LEAP_CTRL_SPARE */
    sb=0xFFFFFFFF;
    if(buf != 0)
        BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR0, &buf));
    if(buf1 != 0)
        BHAB_CHK_RETCODE(BHAB_34xx_WriteRegister(handle, BCHP_LEAP_HOST_IRQ_CLEAR1, &buf1));

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_DecodeError()
******************************************************************************/
BERR_Code BHAB_34xx_P_DecodeError(
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
                retCode = BHAB_ERR_AP_UNKNOWN;
                break;
        }
    }

    return retCode;
}


/******************************************************************************
 BHAB_34xx_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_34xx_P_CheckHab(
    BHAB_Handle handle    /* [in] BHAB Handle */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BHAB_ApStatus status = 0;

    BHAB_CHK_RETCODE(BHAB_34xx_GetApStatus(handle, &status));

    if (((status & 0xF) == 0x1) || ((status & 0xF) == 0))
        retCode = BERR_SUCCESS;
    else
        retCode = BHAB_34xx_P_DecodeError(handle, &status);

done:
    return retCode;
}


/******************************************************************************
 BHAB_34xx_GetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_34xx_GetWatchDogTimer(
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
 BHAB_34xx_SetWatchDogTimer()
******************************************************************************/
BERR_Code BHAB_34xx_SetWatchDogTimer(
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
 BHAB_34xx_GetNmiConfig()
******************************************************************************/
BERR_Code BHAB_34xx_GetNmiConfig(
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
 BHAB_34xx_SetNmiConfig()
******************************************************************************/
BERR_Code BHAB_34xx_SetNmiConfig(
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
 BHAB_34xx_GetConfigSettings()
****************************************************************************/
BERR_Code BHAB_34xx_GetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    BHAB_ConfigSettings *settings)     /* [out] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[5] = HAB_MSG_HDR(BHAB_READ_DAISY, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[5] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_READ, 0, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_READ, 0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf, 5, p34xx->inBuf, 0, false, true, 5));
    settings->daisyChain = p34xx->inBuf[4] & 0x3;

    BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf1, 5, p34xx->inBuf, 0, false, true, 5));
    settings->enableLoopThrough = p34xx->inBuf[4] & 0x4;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 5, p34xx->inBuf, 17, false, true, 17));
    settings->tnrApplication = ((p34xx->inBuf[4] & 0xC0) >> 6) - 1;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 5, p34xx->inBuf, 9, false, true, 9));
    settings->rfInputMode = p34xx->inBuf[4];

done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_SetConfigSettings()
****************************************************************************/
BERR_Code BHAB_34xx_SetConfigSettings(
    BHAB_Handle handle,           /* [in] Device handle */
    const BHAB_ConfigSettings *settings)     /* [in] HAB config settings. */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_WRITE_DAISY, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t buf1[9] = HAB_MSG_HDR(BHAB_LOOP_THROUGH_WRITE, 0x4, BHAB_CORE_TYPE, BHAB_CORE_ID);
    uint8_t rfInputMode[9] = HAB_MSG_HDR(BHAB_RF_INPUT_MODE_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    uint8_t tnrApplication[17] = HAB_MSG_HDR(BHAB_TNR_APPLICATION_WRITE, 0xC, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if(settings->tnrApplication != p34xx->tnrApplication) {
        tnrApplication[4] |= ((settings->tnrApplication + 1) << 6);
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, tnrApplication, 17, p34xx->inBuf, 0, false, true, 17));
        p34xx->tnrApplication = settings->tnrApplication;
    }

   if(settings->rfInputMode != p34xx->rfInputMode) {
       rfInputMode[4] = settings->rfInputMode;
       BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, rfInputMode, 9, p34xx->inBuf, 0, false, true, 9));
   }

    if(settings->daisyChain){
        buf[4] = (((uint8_t)settings->daisyChain & 0x3) - 1) | 0x4;
    }

    BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf, 9, p34xx->inBuf, 0, false, true, 9));

    p34xx->daisyChain = settings->daisyChain;

    if(settings->enableLoopThrough)
    {
        buf1[4] = 0x4;
        BHAB_CHK_RETCODE(BHAB_34xx_SendHabCommand(handle, buf1, 9, p34xx->inBuf, 0, false, true, 9));
    }

done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_GetInternalGain()
****************************************************************************/
BERR_Code BHAB_34xx_GetInternalGain(
    BHAB_Handle handle,                                 /* [in] Device handle */
    const BHAB_InternalGainInputParams *inputParams,  /* [in] frequency */
    BHAB_InternalGainSettings *internalGainSettings     /* [out] internal gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t hab[17] = HAB_MSG_HDR(BHAB_INTERNAL_GAIN_READ, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    hab[4] = (inputParams->frequency >> 24);
    hab[5] = (inputParams->frequency >> 16);
    hab[6] = (inputParams->frequency >> 8);
    hab[7] = inputParams->frequency;

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, hab, 9, p34xx->inBuf, 17, false, true, 17 ));
    internalGainSettings->externalGainBypassed = (p34xx->inBuf[4] >> 7);
    internalGainSettings->internalGainLoopThrough = (int16_t)((p34xx->inBuf[8] << 8) | p34xx->inBuf[9])*100/256;
    internalGainSettings->internalGainDaisy = (int16_t)((p34xx->inBuf[10] << 8) | p34xx->inBuf[11])*100/256;
    internalGainSettings->frequency = (p34xx->inBuf[12] << 24) | (p34xx->inBuf[13] << 16) | (p34xx->inBuf[14] << 8) | p34xx->inBuf[15];

done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_GetExternalGain()
****************************************************************************/
BERR_Code BHAB_34xx_GetExternalGain(
    BHAB_Handle handle,                               /* [in] Device handle */
    BHAB_ExternalGainSettings *externalGainSettings /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_READ, 0x0, BTNR_CORE_TYPE, BTNR_CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p34xx->inBuf, 9, false, true, 9 ));
    externalGainSettings->externalGainTotal = (int16_t)((p34xx->inBuf[4] << 8) | p34xx->inBuf[5])*100/256;
    externalGainSettings->externalGainBypassable = (int16_t)((p34xx->inBuf[6] << 8) | p34xx->inBuf[7])*100/256;

done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_SetExternalGain()
****************************************************************************/
BERR_Code BHAB_34xx_SetExternalGain(
    BHAB_Handle handle,                                       /* [in] Device handle */
    const BHAB_ExternalGainSettings *externalGainSettings   /* [in] external gain settings. */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[9] = HAB_MSG_HDR(BHAB_EXTERNAL_GAIN_WRITE, 0x4, BTNR_CORE_TYPE, BTNR_CORE_ID);
    int16_t externalGainTotal, externalGainBypassable;
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    externalGainTotal = externalGainSettings->externalGainTotal*256/100;
    externalGainBypassable = externalGainSettings->externalGainBypassable*256/100;

    buf[4] = (externalGainTotal >> 8);
    buf[5] = externalGainTotal;
    buf[6] = (externalGainBypassable >> 8);
    buf[7] = externalGainBypassable;
    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 9, p34xx->inBuf, 0, false, true, 9 ));

done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_GetAvsData()
****************************************************************************/
BERR_Code BHAB_34xx_GetAvsData(
    BHAB_Handle handle,         /* [in] Device handle */
    BHAB_AvsData *pAvsData      /* [out] pointer to AVS Data */
)
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[49] = HAB_MSG_HDR(BHAB_AVS_DATA, 0, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 5, p34xx->inBuf, 49, false, true, 49 ));
    pAvsData->enabled = (p34xx->inBuf[7] & 0x4) >> 2;
    pAvsData->voltage = (p34xx->inBuf[8] << 24) | (p34xx->inBuf[9] << 16) | (p34xx->inBuf[10] << 8) | p34xx->inBuf[11];
    pAvsData->temperature = (int32_t)((p34xx->inBuf[12] << 24) | (p34xx->inBuf[13] << 16) | (p34xx->inBuf[14] << 8) | p34xx->inBuf[15])*100;
done:
    return retCode;
}

/***************************************************************************
 BHAB_34xx_GetTunerChannels()
****************************************************************************/
BERR_Code BHAB_34xx_GetTunerChannels(
    BHAB_Handle handle,     /* [in] Device handle */
    uint8_t *numChannels)   /* [out] Returns total tuner channels */
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p34xx->inBuf, 77, false, true, 77));

        numDescriptors =  p34xx->inBuf[22];
        handle->totalTunerChannels = p34xx->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p34xx->inBuf[31+i*16];
            channelNumber = p34xx->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p34xx->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p34xx->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p34xx->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p34xx->inBuf[35+i*16] & 0x80) >> 7;
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
 BHAB_34xx_GetCapabilities()
****************************************************************************/
BERR_Code BHAB_34xx_GetCapabilities(
    BHAB_Handle handle,                 /* [in] Device handle */
    BHAB_Capabilities *pCapabilities    /* [out] Returns chip capabilities */
    )
{
    BERR_Code retCode = BERR_SUCCESS;
    uint8_t buf[77] = HAB_MSG_HDR(BHAB_GET_CAPABILITIES, 0xc, BHAB_GLOBAL_CORE_TYPE, BHAB_CORE_ID);
    uint8_t i=0, j=0, numDescriptors=0, cores=0, channelNumber=0, offset=0;
    BHAB_34xx_P_Handle *p34xx;
    BDBG_ASSERT(handle);

    p34xx = (BHAB_34xx_P_Handle *)(handle->pImpl);
    BDBG_ASSERT(p34xx);

    if(handle->channelCapabilities == NULL) {
        buf[11] = 1;
        buf[15] = 2;
        BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 17, p34xx->inBuf, 77, false, true, 77));

        numDescriptors =  p34xx->inBuf[22];
        handle->totalTunerChannels = p34xx->inBuf[23];
        if(handle->channelCapabilities == NULL)
            handle->channelCapabilities = (BHAB_ChannelCapability *) BKNI_Malloc( handle->totalTunerChannels*sizeof(BHAB_ChannelCapability) );
        BKNI_Memset( handle->channelCapabilities, 0x00, sizeof(BHAB_ChannelCapability)*handle->totalTunerChannels);

        for (i=0; i<numDescriptors; i++)
        {
            cores = p34xx->inBuf[31+i*16];
            channelNumber = p34xx->inBuf[30+i*16];
            for (j=0; j<cores; j++)
            {
                handle->channelCapabilities[offset].tunerChannelNumber = channelNumber;
                handle->channelCapabilities[offset].demodCoreType.dvbt = (p34xx->inBuf[35+i*16] & 0x10) >> 4;
                handle->channelCapabilities[offset].demodCoreType.isdbt = (p34xx->inBuf[35+i*16] & 0x20) >> 5;
                handle->channelCapabilities[offset].demodCoreType.dvbt2 = (p34xx->inBuf[35+i*16] & 0x40) >> 6;
                handle->channelCapabilities[offset].demodCoreType.dvbc2 = (p34xx->inBuf[35+i*16] & 0x80) >> 7;
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
