/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/

#include "bhab_4538_priv.h"
#include "bchp_4538_leap_ctrl.h"
#include "bchp_4538_leap_hab_mem.h"
#include "bchp_4538_leap_host_irq.h"
#include "bchp_4538_leap_l2.h"
#include "bchp_4538_hif.h"
#include "bchp_4538_csr.h"

#if 0 /* define BHAB_NO_IMG to disable IMG in the HAB module */
#define BHAB_NO_IMG
#endif

#ifndef BHAB_NO_IMG
#include "bhab_satfe_img.h"
#endif

BDBG_MODULE(bhab_4538_priv);

#define BHAB_MAX_BBSI_RETRIES 10


/* local private functions */
BERR_Code BHAB_4538_P_WaitForBbsiDone(BHAB_Handle h);
BERR_Code BHAB_4538_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
BERR_Code BHAB_4538_P_RunAp(BHAB_Handle h);
BERR_Code BHAB_4538_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec);
BERR_Code BHAB_4538_P_ServiceHab(BHAB_Handle h, uint8_t *read_buf, uint16_t read_len, uint8_t ack_byte, bool bFixedLength);
BERR_Code BHAB_4538_P_DecodeInterrupt(BHAB_Handle h);
BERR_Code BHAB_4538_P_CheckHab(BHAB_Handle h);
BERR_Code BHAB_4538_P_ReadHab(BHAB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n);
BERR_Code BHAB_4538_P_DecodeHabError(BHAB_Handle h, BERR_Code *habError);
BERR_Code BHAB_4538_P_DisableFrontendInterrupts(BHAB_Handle h);


/******************************************************************************
 BHAB_4538_P_Open()
******************************************************************************/
BERR_Code BHAB_4538_P_Open(
   BHAB_Handle *handle,     /* [out] BHAB handle */
   void        *pReg,       /* [in] pointer to i2c or spi handle */
   const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_Handle hDev;
   BHAB_4538_P_Handle *h4538Dev;

   BDBG_ASSERT(pDefSettings->interruptEnableFunc);

   hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
   BDBG_ASSERT(hDev);
   h4538Dev = (BHAB_4538_P_Handle *)BKNI_Malloc(sizeof(BHAB_4538_P_Handle));
   BDBG_ASSERT(h4538Dev);
   BKNI_Memset( h4538Dev, 0x00, sizeof(BHAB_4538_P_Handle));
   hDev->pImpl = (void*)h4538Dev;

   if (pDefSettings->isSpi)
      h4538Dev->hSpiRegister = (BREG_SPI_Handle)pReg;
   else
      h4538Dev->hI2cRegister = (BREG_I2C_Handle)pReg;

   h4538Dev->cbAst.func = NULL;
   h4538Dev->cbAst.callbackParams.pParm1 = NULL;
   h4538Dev->cbAst.callbackParams.parm2 = (int)NULL;

   h4538Dev->cbWfe.func = NULL;
   h4538Dev->cbWfe.callbackParams.pParm1 = NULL;
   h4538Dev->cbWfe.callbackParams.parm2 = (int)NULL;

   h4538Dev->cbFsk.func = NULL;
   h4538Dev->cbFsk.callbackParams.pParm1 = NULL;
   h4538Dev->cbFsk.callbackParams.parm2 = (int)NULL;

   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));

   /* create events */
   retCode = BKNI_CreateEvent(&(h4538Dev->hInterruptEvent));
   if (retCode != BERR_SUCCESS)
      goto done_0;
   retCode = BKNI_CreateEvent(&(h4538Dev->hApiEvent));
   if (retCode != BERR_SUCCESS)
      goto done_1;
   retCode = BKNI_CreateEvent(&(h4538Dev->hInitDoneEvent));
   if (retCode != BERR_SUCCESS)
      goto done_2;
   retCode = BKNI_CreateEvent(&(h4538Dev->hHabDoneEvent));
   if (retCode != BERR_SUCCESS)
   {
      BKNI_DestroyEvent(h4538Dev->hInitDoneEvent);

      done_2:
      BKNI_DestroyEvent(h4538Dev->hApiEvent);

      done_1:
      BKNI_DestroyEvent(h4538Dev->hInterruptEvent);

      done_0:
      BKNI_Free((void*)h4538Dev);
      BKNI_Free((void*)hDev);

      hDev = NULL;
   }

   *handle = hDev;
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_Close()
******************************************************************************/
BERR_Code BHAB_4538_P_Close(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_4538_P_Handle *p4538 = (BHAB_4538_P_Handle *)(handle->pImpl);

   BDBG_ASSERT(handle);

   /* disable host interrupts */
   retCode = BHAB_4538_P_DisableFrontendInterrupts(handle);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BHAB_4538_P_DisableFrontendInterrupts() error 0x%X", retCode));
   }

   /* reset the LEAP */
   retCode = BHAB_4538_P_Reset(handle);

   /* clean up */
   BKNI_DestroyEvent(p4538->hInterruptEvent);
   BKNI_DestroyEvent(p4538->hApiEvent);
   BKNI_DestroyEvent(p4538->hInitDoneEvent);
   BKNI_DestroyEvent(p4538->hHabDoneEvent);
   BKNI_Free((void*)p4538);
   BKNI_Free((void*)handle);
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_InitAp()
******************************************************************************/
BERR_Code BHAB_4538_P_InitAp(
    BHAB_Handle h,            /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM4538 microcode image */
)
{
#ifndef BHAB_DONT_USE_INTERRUPT
   BHAB_4538_P_Handle *pImpl = (BHAB_4538_P_Handle *)(h->pImpl);
#endif
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t n, addr, init_done_mask, retries, val32, strap;
   const uint8_t *pImage;
#ifdef BHAB_VERIFY_DOWNLOAD
   static uint8_t *pVerifyBuf = NULL;
#endif
#ifndef BHAB_NO_IMG
   unsigned chunk, num_chunks, segment, num_segments;
   unsigned fw_size;
   unsigned header_size = 0;
   unsigned chunk_size = MAX_SATFE_IMG_CHUNK_SIZE;
   BIMG_Interface *pImgInterface = (BIMG_Interface*)h->settings.pImgInterface;
   void **pImgContext = h->settings.pImgContext;
   void *pImg;

   BSTD_UNUSED(pHexImage);

   if ((pImgInterface == NULL) || (pImgContext == NULL))
   {
      BDBG_ERR(("HAB IMG interface and/or context not initialized"));
      return BERR_NOT_INITIALIZED;
   }

   BDBG_MSG(("BHAB_4538_P_InitAp: interface: %p, context: %p", (void*)pImgInterface, (void*)pImgContext));
#endif

   /* disable host interrupts */
   BHAB_CHK_RETCODE(BHAB_4538_P_EnableHostInterrupt(h, false));

   /* disable interrupts */
   BHAB_CHK_RETCODE(BHAB_4538_P_DisableFrontendInterrupts(h));

   /* reset the AP before downloading the firmware */
   BHAB_CHK_RETCODE(BHAB_4538_P_Reset(h));

   /* determine boot mode */
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_CPU_STRAPS, &strap, 1));
   if (strap == 0)
   {
      /* boot mode 0 */
      /* download to RAM */
#ifdef BHAB_NO_IMG
      pImage = pHexImage;
      while (1)
      {
         n = (uint32_t)(*pImage++ << 16);
         n |= (uint32_t)(*pImage++ << 8);
         n |= (uint32_t)(*pImage++);

         if (n == 0)
            break;

         addr = (uint32_t)(*pImage++ << 16);
         addr |= (uint32_t)(*pImage++ << 8);
         addr |= (uint32_t)(*pImage++);

#ifdef BHAB_VERIFY_DOWNLOAD
         if (pVerifyBuf)
            BKNI_Free((void*)pVerifyBuf);
         pVerifyBuf = (uint8_t*)BKNI_Malloc(n);
         if (pVerifyBuf == NULL)
         {
            BDBG_ERR(("unable to allocate memory to verify code"));
         }
#endif

         for (retries = 0; retries < 3; retries++)
         {
            BDBG_MSG(("writing %d bytes starting from 0x%04X", n, addr));
            retCode = BHAB_4538_P_WriteMemory(h, addr, (uint8_t*)pImage, n);
            if (retCode == BERR_SUCCESS)
            {
#ifdef BHAB_VERIFY_DOWNLOAD
               if (pVerifyBuf)
               {
                  BDBG_MSG(("verifying %d bytes from 0x%04X", n, addr));
                  BHAB_CHK_RETCODE(BHAB_4538_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, n));
                  if (BKNI_Memcmp(pImage, pVerifyBuf, n) == 0)
                     break;
                  BDBG_WRN(("data read back does not match\n"));
               }
#else
               break;
#endif
            }
            else
            {
               BDBG_ERR(("BHAB_4538_P_WriteMemory() error 0x%X", retCode));
            }
         }
         if (retries >= 3)
         {
            BDBG_ERR(("unable to initialize RAM"));
            BERR_TRACE(retCode = BHAB_ERR_HOST_XFER);
            goto done;
         }

         pImage += n;
      }

#else
      BDBG_MSG(("BHAB_4538_P_InitAp: opening img interface..."));
      BHAB_CHK_RETCODE(pImgInterface->open((void*)pImgContext, &pImg, 0));

      /* read header */
      BHAB_CHK_RETCODE(pImgInterface->next(pImg, 0, (const void **)&pImage, MAX_SATFE_IMG_QUERY_SIZE));

      header_size = (unsigned)pImage[0];
      num_segments = (unsigned)pImage[1];

      for (segment = 0; segment < num_segments; segment++) {

          /* read segment header */
          BHAB_CHK_RETCODE(pImgInterface->next(pImg, segment, (const void **)&pImage, header_size));

          n = (uint32_t)(*pImage++ << 16);
          n |= (uint32_t)(*pImage++ << 8);
          n |= (uint32_t)(*pImage++);

          addr = (uint32_t)(*pImage++ << 16);
          addr |= (uint32_t)(*pImage++ << 8);
          addr |= (uint32_t)(*pImage++);

          fw_size = n;
          num_chunks = fw_size / chunk_size;
          if (fw_size % chunk_size)
              num_chunks++;

          for (chunk = 0; chunk < num_chunks; chunk++)
          {
              BDBG_MSG(("BHAB_4538_P_InitAp: img next[%d of %d]...", chunk, num_chunks));
              if (chunk == num_chunks-1) {
                  n = fw_size % chunk_size;
              } else
                  n = chunk_size;
              BDBG_MSG(("requesting %d bytes", n));
              BHAB_CHK_RETCODE(pImgInterface->next(pImg, chunk, (const void **)&pImage, n));

#ifdef BHAB_VERIFY_DOWNLOAD
             if (pVerifyBuf)
                BKNI_Free((void*)pVerifyBuf);
             pVerifyBuf = (uint8_t*)BKNI_Malloc(n);
             if (pVerifyBuf == NULL)
             {
                BDBG_ERR(("unable to allocate memory to verify code"));
             }
#endif

             for (retries = 0; retries < 3; retries++)
             {
                BDBG_MSG(("writing %d bytes starting from 0x%04X", n, addr + chunk*chunk_size));
                retCode = BHAB_4538_P_WriteMemory(h, addr + chunk*chunk_size, (uint8_t*)pImage, n);
                if (retCode == BERR_SUCCESS)
                {
#ifdef BHAB_VERIFY_DOWNLOAD
                   if (pVerifyBuf)
                   {
                      BDBG_MSG(("verifying %d bytes from 0x%04X", n, addr));
                      BHAB_CHK_RETCODE(BHAB_4538_P_ReadMemory(h, addr + chunk*chunk_size, (uint8_t *)pVerifyBuf, n));
                      if (BKNI_Memcmp(pImage, pVerifyBuf, n) == 0)
                         break;
                      BDBG_WRN(("data read back does not match\n"));
                   }
#else
                   break;
#endif
                }
                else
                {
                   BDBG_ERR(("BHAB_4538_P_WriteMemory() error 0x%X", retCode));
                }
             }
             if (retries >= 3)
             {
                BDBG_ERR(("unable to initialize RAM"));
                BERR_TRACE(retCode = BHAB_ERR_HOST_XFER);
                goto done;
             }
          }
      }

      BDBG_MSG(("BHAB_4538_P_InitAp: img close..."));
      pImgInterface->close(pImg);
#endif

#ifdef BHAB_VERIFY_DOWNLOAD
      BDBG_MSG(("firmware loaded successfully in RAM"));
#endif

      /* clear init_done interrupt status */
      init_done_mask = BHAB_4538_HIRQ0_INIT_DONE;
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &init_done_mask, 1));

#ifndef BHAB_DONT_USE_INTERRUPT
      /* enable init done interrupt */
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &init_done_mask, 1));
      BKNI_WaitForEvent(pImpl->hInitDoneEvent, 0);
#endif

      /* start running the AP */
      BHAB_CHK_RETCODE(BHAB_4538_P_RunAp(h));
      BDBG_MSG(("LEAP is running"));

#ifdef BHAB_DONT_USE_INTERRUPT
      /* poll for AP init done */
      for (n = 0; n < 5000; n++)
      {
         BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
         if (val32 & BHAB_4538_HIRQ0_INIT_DONE)
            break;
         BKNI_Sleep(1);
      }
      if ((val32 & BHAB_4538_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
         goto done;
      }
#else
      /* wait for init done interrupt */
      if (BHAB_4538_P_WaitForEvent(h, pImpl->hInitDoneEvent, 5000) != BERR_SUCCESS)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
         goto done;
      }
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET0, &init_done_mask, 1));
#endif
   }
   else if (strap == 1)
   {
      /* boot mode 1 */
      BDBG_MSG(("Boot Mode 1"));
      for (n = 0; n < 1000; n++)
      {
         BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
         if (val32 & BHAB_4538_HIRQ0_INIT_DONE)
            break;
         BKNI_Sleep(1);
      }
      if ((val32 & BHAB_4538_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
         goto done;
      }
   }
   else
   {
      BDBG_ERR(("Unsupported boot mode (%d)", strap));
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
      goto done;
   }

   done:
#ifdef BHAB_VERIFY_DOWNLOAD
   if (pVerifyBuf)
      BKNI_Free((void*)pVerifyBuf);
#endif
   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("LEAP initialized successfully"));
   }
   else
   {
      BDBG_ERR(("BHAB_4538_P_InitAp() error 0x%X", retCode));
   }
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_GetApVersion()
******************************************************************************/
BERR_Code BHAB_4538_P_GetApVersion(
    BHAB_Handle handle,     /* [in] BHAB handle */
    uint32_t    *pFamilyId, /* [out] Chip Family id */
    uint32_t    *pChipId,   /* [out] BHAB chip ID */
    uint16_t    *pChipVer,  /* [out] chip revision number */
    uint8_t     *pMajApVer, /* [out] AP microcode major version */
    uint8_t     *pMinApVer  /* [out] AP microcode minor version */
)
{
   BERR_Code retCode;
   BFEC_SystemVersionInfo versionInfo;

   retCode = BHAB_4538_P_GetVersionInfo(handle, &versionInfo);
   if (retCode == BERR_SUCCESS)
   {
      *pFamilyId = (uint32_t)(versionInfo.familyId);
      *pChipId = (uint32_t)(versionInfo.chipId);
      *pChipVer = (uint16_t)(versionInfo.chipVersion);
      *pMajApVer = (uint8_t)(versionInfo.firmware.majorVersion);
      *pMinApVer = (uint8_t)(versionInfo.firmware.minorVersion);
   }
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_4538_P_GetVersionInfo(BHAB_Handle handle, BFEC_SystemVersionInfo *p)
{
   BERR_Code retCode;
   uint8_t buf[40];

   buf[0] = 0x03;
   BHAB_CHK_RETCODE(BHAB_SendHabCommand(handle, buf, 38, buf, 38, true, true, 38));

   p->familyId = (uint32_t)((buf[1] << 24) | (buf[2] << 16) | (buf[3] << 8) | buf[4]);
   p->chipId = (uint32_t)((buf[5] << 24) | (buf[6] << 16) | (buf[7] << 8) | buf[8]);
   p->chipVersion = (uint32_t)((buf[9] << 24) | (buf[10] << 16) | (buf[11] << 8) | buf[12]);
   p->firmware.majorVersion = (uint32_t)((buf[13] << 24) | (buf[14] << 16) | (buf[15] << 8) | buf[16]);
   p->firmware.minorVersion = (uint32_t)((buf[17] << 24) | (buf[18] << 16) | (buf[19] << 8) | buf[20]);
   p->firmware.buildType = (uint32_t)((buf[21] << 24) | (buf[22] << 16) | (buf[23] << 8) | buf[24]);
   p->firmware.buildId = (uint32_t)((buf[25] << 24) | (buf[26] << 16) | (buf[27] << 8) | buf[28]);
   p->bondoutOptions[0] = (uint32_t)((buf[29] << 24) | (buf[30] << 16) | (buf[31] << 8) | buf[32]);
   p->bondoutOptions[1] = (uint32_t)((buf[33] << 24) | (buf[34] << 16) | (buf[35] << 8) | buf[36]);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_GetApStatus()
******************************************************************************/
BERR_Code BHAB_4538_P_GetApStatus(
   BHAB_Handle h,          /* [in] HAB device handle */
   BHAB_ApStatus *pStatus  /* [out] AP status */
)
{
   BERR_Code retCode;
   uint32_t spare0, hab_request;
   uint8_t csr_status;

   *pStatus = 0;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1));
   if ((csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) == 0)
      *pStatus |= BHAB_APSTATUS_RESET;
   else
   {
      /* AP is running */
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0, 1));
      if (spare0 & BHAB_4538_SPARE0_INIT_DONE)
      {
         /* AP has initialized */
         *pStatus |= BHAB_APSTATUS_INIT_DONE;

         BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request, 1));
         if ((hab_request & 1) == 0)
            *pStatus |= BHAB_APSTATUS_HAB_READY;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_WriteMemory()
******************************************************************************/
BERR_Code BHAB_4538_P_WriteMemory(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, addr = start_addr, nWords, nWordsLeft, leftover, len;
   bool bIsRunning;
   uint8_t i2c_buf[5], csr_status, hab[128], length, x, *pBuf = (uint8_t *)buf;
   BREG_SPI_Data spiData[2];

   /* make sure memory address is 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03))
      retCode = BERR_INVALID_PARAMETER;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1));
   bIsRunning = (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) ? true : false;
   if ((bIsRunning && (addr < 0x30000)) || ((addr >= 0x60000) && (addr < 0x69000)))
      return BHAB_ERR_MEMAV; /* cannot write to ROM or instruction memory while running */

   leftover = n % 4;
   nWords = (n >> 2);

   if (bIsRunning && (addr < BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE))
   {
      /* make sure addr and length is 4-byte aligned */
      if ((start_addr & 0x03) || (n & 0x03))
         return BERR_INVALID_PARAMETER;

      nWordsLeft = nWords;
      while (nWordsLeft > 0)
      {
         if (nWordsLeft > 30)
            length = 30;
         else
            length = (uint8_t)nWordsLeft;
         hab[0] = 0x02;
         hab[1] = (uint8_t)((addr >> 24) & 0xFF);
         hab[2] = (uint8_t)((addr >> 16) & 0xFF);
         hab[3] = (uint8_t)((addr >> 8) & 0xFF);
         hab[4] = (uint8_t)(addr & 0xFF);
         hab[5] = (uint8_t)length;
         for (i = 0; i < length; i++)
         {
            hab[6+(i*4)] = *pBuf++;
            hab[7+(i*4)] = *pBuf++;
            hab[8+(i*4)] = *pBuf++;
            hab[9+(i*4)] = *pBuf++;
         }
         x = 7 + (length * 4);
         BHAB_CHK_RETCODE(BHAB_SendHabCommand(h, hab, x, hab, x, true, true, x));
         nWordsLeft -= length;
         addr += (4*length);
      }
   }
   else
   {
      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));

      i2c_buf[0] = 0x00; /* BCHP_CSR_CONFIG: 32-bit writes */
      i2c_buf[1] = (uint8_t)((start_addr >> 24) & 0xFF);
      i2c_buf[2] = (uint8_t)((start_addr >> 16) & 0xFF);
      i2c_buf[3] = (uint8_t)((start_addr >> 8) & 0xFF);
      i2c_buf[4] = (uint8_t)(start_addr & 0xFF);
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));

      for (i = 0; i < nWords; i++)
      {
         /* wait for !busy or error */
         BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));

         if (bIsRunning)
         {
            i2c_buf[0] = buf[i<<2];
            i2c_buf[1] = buf[(i<<2)+1];
            i2c_buf[2] = buf[(i<<2)+2];
            i2c_buf[3] = buf[(i<<2)+3];
            BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
         }
         else
         {
            if (nWords > 0)
            {
               if (h->settings.isSpi == false)
               {
                  /* for i2c, we can write entire buf in one transaction if CPU not running */
                  BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, (uint8_t *)buf, nWords<<2));
               }
               else
               {
                  BSTD_UNUSED(len);
                  i2c_buf[0] = 0x41;
                  i2c_buf[1] = BCHP_CSR_RBUS_DATA0;

                  spiData[0].data = (void *)i2c_buf;
                  spiData[0].length = 2;
                  spiData[1].data = (void *)buf;
                  spiData[1].length = nWords<<2;
                  BHAB_CHK_RETCODE(BREG_SPI_Multiple_Write(((BHAB_4538_P_Handle*)h->pImpl)->hSpiRegister, spiData, 2));
               }
            }
            break;
         }
      }

      if (leftover)
      {
         /* pad with zeros */
         i2c_buf[0] = buf[(nWords<<2)];
         i2c_buf[1] = (leftover > 1) ? buf[(nWords<<2)+1] : 0;
         i2c_buf[2] = (leftover > 2) ? buf[(nWords<<2)+2] : 0;
         i2c_buf[3] = 0;
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ReadMemory()
******************************************************************************/
BERR_Code BHAB_4538_P_ReadMemory(BHAB_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode;
   uint32_t i, j, buf_idx, addr, end_addr, nReads, x, val32, nReadsLeft;
   bool bIsRunning, bCsrAddrSet = false;
   uint8_t i2c_buf[5], csr_status = 0, hab[128];

   if (n == 0)
      return BERR_INVALID_PARAMETER;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1));
   bIsRunning = (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) ? true : false;

   addr = start_addr & ~0x03; /* actual starting address is 32-bit aligned */
   end_addr = start_addr + n;
   if (end_addr & 0x03)
      end_addr = (end_addr & ~0x03) + 4;
   nReads = (end_addr - addr) >> 2; /* total number of 32-bit reads */

   if (bIsRunning && ((addr < BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE) || (addr >= 0x0FC00000)))
   {
      /* make sure addr and length is 4-byte aligned */
      if ((start_addr & 0x03) || (n & 0x03))
         return BERR_INVALID_PARAMETER;

      /* send HAB command to read memory */
      nReadsLeft = nReads;
      i = 0;
      while (nReadsLeft > 0)
      {
         if (nReadsLeft > 30)
            x = 30;
         else
            x = nReadsLeft;
         hab[0] = 0x01;
         hab[1] = (uint8_t)((addr >> 24) & 0xFF);
         hab[2] = (uint8_t)((addr >> 16) & 0xFF);
         hab[3] = (uint8_t)((addr >> 8) & 0xFF);
         hab[4] = (uint8_t)(addr & 0xFF);
         hab[5] = (uint8_t)x;
         j = 7 + (x*4);
         BHAB_CHK_RETCODE(BHAB_SendHabCommand(h, hab, (uint8_t)j, hab, (uint8_t)j, true, true, (uint8_t)j));
         BKNI_Memcpy(&buf[i], &hab[6], x*4);
         nReadsLeft -= x;
         addr += (4*x);
         i += (4*x);
      }
   }
   else
   {
      for (i = buf_idx = 0; i < nReads; i++)
      {
         /* wait for !busy or error */
         BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));

         if (i == 0)
         {
            x = start_addr & 0x03;
            if (x)
            {
               /* starting address is not 32-bit aligned */
               BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, addr, &val32, 1));
               if (x == 3)
                  buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
               else if (x == 2)
               {
                  buf[buf_idx++] = (uint8_t)((val32 >> 8) & 0xFF);
                  buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
               }
               else /* x==1 */
               {
                  buf[buf_idx++] = (uint8_t)((val32 >> 16) & 0xFF);
                  buf[buf_idx++] = (uint8_t)((val32 >> 8) & 0xFF);
                  buf[buf_idx++] = (uint8_t)(val32 & 0xFF);
               }
               addr += 4;
               continue;
            }
         }

         if (!bCsrAddrSet)
         {
            i2c_buf[0] = (i == (nReads-1)) ? 0x01 : 0x03; /* BCHP_CSR_CONFIG: 32-bit reads */
            i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
            i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
            i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
            i2c_buf[4] = (uint8_t)(addr & 0xFF);
            BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
            bCsrAddrSet = true;
         }

         BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
         for (j = 0; (j < 4) && (buf_idx < n); j++)
            buf[buf_idx++] = i2c_buf[j];
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_WriteRegister()
******************************************************************************/
BERR_Code BHAB_4538_P_WriteRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode;
   bool bHsi = false;
   uint8_t sb;

   if (reg < 0x100)
      bHsi = true;
   if ((reg > 0xEE30F) || ((reg < 0x79000) && !bHsi))
      return BERR_INVALID_PARAMETER;

   if (bHsi)
   {
      sb = (uint8_t)(*val & 0xFF);
      retCode = BHAB_4538_P_WriteBbsi(h, (uint8_t)reg, &sb, 1);
   }
   else
      retCode = BHAB_4538_P_WriteRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ReadRegister()
******************************************************************************/
BERR_Code BHAB_4538_P_ReadRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode;
   bool bHsi = false;
   uint8_t sb;

   if (reg < 0x100)
      bHsi = true;
   if ((reg > 0xEE30F) || ((reg < 0x79000) && !bHsi))
   {
      BDBG_ERR(("BHAB_4538_P_ReadRegister(): bad address (0x%X)", reg));
      return BERR_INVALID_PARAMETER;
   }

   if (bHsi)
   {
      retCode = BHAB_4538_P_ReadBbsi(h, (uint8_t)reg, &sb, 1);
      if (retCode == BERR_SUCCESS)
         *val = (uint32_t)sb;
   }
   else
      retCode = BHAB_4538_P_ReadRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_4538_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_4538_P_HandleInterrupt_isr(
    BHAB_Handle handle /* [in] BHAB handle */
)
{
   BHAB_4538_P_Handle *p4538;

   BDBG_ASSERT(handle);
   p4538 = (BHAB_4538_P_Handle *)(handle->pImpl);
   handle->settings.interruptEnableFunc(false, handle->settings.interruptEnableFuncParam);

   BKNI_SetEvent(p4538->hApiEvent);
   BKNI_SetEvent(p4538->hInterruptEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4538_P_ProcessInterruptEvent()
******************************************************************************/
BERR_Code BHAB_4538_P_ProcessInterruptEvent(
    BHAB_Handle handle  /* [in] BHAB handle */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ASSERT(handle);

   BHAB_CHK_RETCODE(BHAB_4538_P_DecodeInterrupt(handle));
   BHAB_4538_P_EnableHostInterrupt(handle, true);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_4538_P_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
   BHAB_4538_P_Handle *p4538;

   BDBG_ASSERT(handle);
   p4538 = (BHAB_4538_P_Handle *)(handle->pImpl);

   if (eDevId == BHAB_DevId_eAST)
   {
      p4538->cbAst.func = fCallBack;
      p4538->cbAst.callbackParams.pParm1 = pParm1;
      p4538->cbAst.callbackParams.parm2 = parm2;
   }
   else if (eDevId == BHAB_DevId_eWFE)
   {
      p4538->cbWfe.func = fCallBack;
      p4538->cbWfe.callbackParams.pParm1 = pParm1;
      p4538->cbWfe.callbackParams.parm2 = parm2;
   }
   else if (eDevId == BHAB_DevId_eFSK)
   {
      p4538->cbFsk.func = fCallBack;
      p4538->cbFsk.callbackParams.pParm1 = pParm1;
      p4538->cbFsk.callbackParams.parm2 = parm2;
   }
   else
   {
      BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4538_P_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_4538_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
   BHAB_4538_P_Handle *p4538 = (BHAB_4538_P_Handle *)(handle->pImpl);

   BDBG_ASSERT(handle);
   p4538 = (BHAB_4538_P_Handle *)(handle->pImpl);

   if (eDevId == BHAB_DevId_eAST)
   {
      p4538->cbAst.func = NULL;
      p4538->cbAst.callbackParams.pParm1 = NULL;
      p4538->cbAst.callbackParams.parm2 = 0;
   }
   else if (eDevId == BHAB_DevId_eWFE)
   {
      p4538->cbWfe.func = NULL;
      p4538->cbWfe.callbackParams.pParm1 = NULL;
      p4538->cbWfe.callbackParams.parm2 = 0;
   }
   else if (eDevId == BHAB_DevId_eFSK)
   {
      p4538->cbFsk.func = NULL;
      p4538->cbFsk.callbackParams.pParm1 = NULL;
      p4538->cbFsk.callbackParams.parm2 = 0;
   }
   else
   {
      BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4538_P_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_4538_P_SendHabCommand(
    BHAB_Handle handle,  /* [in] BHAB PI Handle */
    uint8_t *write_buf,  /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of bytes in the HAB command */
    uint8_t *read_buf,   /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of bytes to read from the HAB */
    bool bCheckForAck,   /* [in] true = determine if the AP has serviced the command */
    bool bFixedLength,   /* [in] true = fixed length HAB command */
    uint16_t command_len
)
{
   BERR_Code retCode = BERR_SUCCESS;
   bool bOK = true;
   uint16_t i;
   uint8_t hab_buf[512], cs = 0;
#if 0
   uint8_t retry, hab_buf_check[512];
#endif

   BSTD_UNUSED(command_len);
   BSTD_UNUSED(bCheckForAck);

   if ((write_len >= 512) || (read_len >= 512) || (write_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BHAB_CHK_RETCODE(BHAB_4538_P_CheckHab(handle));

   BKNI_Memcpy(hab_buf, write_buf, write_len);

   /* compute checksum */
   for (i = 0, cs = 0; i < (write_len - 1); i++)
      cs += hab_buf[i];
   hab_buf[write_len-1] = cs;

   /* write the command to the HAB */
   hab_buf[write_len] = 0; /* make sure there is a zero byte after the last byte of the command */

#if 0
   for (i = 0; i < (write_len+1); i++)
   {
      BDBG_MSG(("HAB[%d]=0x%02X", i, hab_buf[i]));
   }
#endif

#if 0
   for (retry = 0; retry < 20; retry++)
   {
      bOK = true;
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, hab_buf, (uint32_t)(write_len+1)));

      BHAB_CHK_RETCODE(BHAB_4538_P_ReadMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, hab_buf_check, (uint32_t)(write_len+1)));
      for (i = 0; bOK && (i < (write_len+1)); i++)
      {
         if (hab_buf_check[i] != hab_buf[i])
         {
            BDBG_WRN(("HAB writing error in address 0x%X: read=0x%02X, expected=0x%02X", i, hab_buf_check[i], hab_buf[i]));
            bOK = false;
         }
      }

      if (bOK)
         break;
   }
#else
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteMemory(handle, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE, hab_buf, (uint32_t)(write_len+1)));
#endif

   if (bOK)
   {
      /* wait for the AP to service the HAB, and then read any return data */
      BHAB_CHK_RETCODE(BHAB_4538_P_ServiceHab(handle, read_buf, write_len, (uint8_t)(hab_buf[0] | 0x80), bFixedLength));
   }
   else
   {
      BDBG_ERR(("Unable to write to HAB"));
      retCode = BHAB_ERR_HAB_ERR;
   }

   done:
   return retCode;
}

/******************************************************************************
 BHAB_4538_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_4538_P_GetInterruptEventHandle(
    BHAB_Handle handle,            /* [in] BHAB handle */
    BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
    *hEvent = ((BHAB_4538_P_Handle *)(handle->pImpl))->hInterruptEvent;
    return BERR_SUCCESS;
}


/****************************Private Functions*********************************/

/******************************************************************************
 BHAB_4538_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_4538_P_EnableHostInterrupt(
   BHAB_Handle h, /* [in] BHAB handle */
   bool bEnable   /* [in] true=enables the L1 interrupt on the host processor */
)
{
   BKNI_EnterCriticalSection();
   h->settings.interruptEnableFunc(bEnable, h->settings.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4538_P_Reset()
******************************************************************************/
BERR_Code BHAB_4538_P_Reset(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t ctrl, val32, strap;
   uint8_t status, buf;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_CPU_STRAPS, &strap, 1));

   /* reset LEAP/CPU */
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));
   ctrl |= 0x00C00003;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));
   ctrl &= ~0x00C00000;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));

   /* initialize some LEAP registers */
   val32 = 0;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &val32, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE1, &val32, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE2, &val32, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_SW_SPARE3, &val32, 1));

   if ((strap & 0x03) == 0)
   {
      /* verify that AP is reset */
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if (status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
      {
         BDBG_ERR(("unable to reset the AP"));
         BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
      }
      else
      {
         buf = 0x0B;
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST0, &buf, 1));
         buf = 0x3F;
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST1, &buf, 1));
         buf = 0xFF;
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST2, &buf, 1));
         buf = 0x00;
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST0, &buf, 1));
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST1, &buf, 1));
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_HIF_SFT_RST2, &buf, 1));
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_WriteBbsi()
******************************************************************************/
BERR_Code BHAB_4538_P_WriteBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode;
   uint32_t i;
   uint8_t spiBuf[16], *pBuf;

   if (h->settings.isSpi)
   {
      if (n < 14)
      {
         /* perform a short spi write without malloc */
         spiBuf[0] = 0x41;
         spiBuf[1] = addr;
         for (i = 0; i < n; i++)
            spiBuf[i+2] = buf[i];

         retCode = BREG_SPI_Write(((BHAB_4538_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)spiBuf, n+2);
      }
      else
      {
         pBuf = (uint8_t *)BKNI_Malloc(n+2);
         if (pBuf == NULL)
         {
            retCode = BERR_OUT_OF_SYSTEM_MEMORY;
            goto done;
         }
         pBuf[0] = 0x41;
         pBuf[1] = addr;
         BKNI_Memcpy(&pBuf[2], buf, n);
         retCode = BREG_SPI_Write(((BHAB_4538_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)pBuf, n+2);
         BKNI_Free(pBuf);
      }
   }
   else
      retCode = BREG_I2C_Write(((BHAB_4538_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ReadBbsi()
******************************************************************************/
BERR_Code BHAB_4538_P_ReadBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode;
   uint32_t i;
   uint8_t spiWriteBuf[8], spiReadBuf[8];

   if (h->settings.isSpi)
   {
      if (n > 4)
      {
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         goto done;
      }

      spiWriteBuf[0] = 0x40;
      spiWriteBuf[1] = addr;
      for (i = 0; i < n; i++)
         spiWriteBuf[i+2] = 0;

      retCode = BREG_SPI_Read(((BHAB_4538_P_Handle*)h->pImpl)->hSpiRegister, spiWriteBuf, spiReadBuf, n+2);
      if (retCode == BERR_SUCCESS)
      {
         for (i = 0; i < n; i++)
            buf[i] = spiReadBuf[i+2];
      }
   }
   else
      retCode = BREG_I2C_Read(((BHAB_4538_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ReadRbus()
******************************************************************************/
BERR_Code BHAB_4538_P_ReadRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   uint8_t i2c_buf[5];

   if (n == 0)
      return BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      /* set up the starting address to read */
      if (i == 0)
      {
         i2c_buf[0] = (n > 1) ? 0x03 : 0x05; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[4] = (uint8_t)(addr & 0xFF);
         BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
      }

      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));

      BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      buf[i] = (uint32_t)((i2c_buf[0] << 24) | (i2c_buf[1] << 16) | (i2c_buf[2] << 8) | i2c_buf[3]);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_WriteRbus()
******************************************************************************/
BERR_Code BHAB_4538_P_WriteRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, data_idx;
   uint8_t i2c_buf[9];

   if (n == 0)
      retCode = BERR_INVALID_PARAMETER;

   /* wait for !busy or error */
   BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));

   for (i = 0; i < n; i++)
   {
      if (i == 0)
      {
         i2c_buf[0] = 0; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[2] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[4] = (uint8_t)(addr & 0xFF);
         data_idx = 5;
      }
      else
         data_idx = 0;

      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 24) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 16) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 8) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)(buf[i] & 0xFF);

      BHAB_CHK_RETCODE(BHAB_4538_P_WriteBbsi(h, (uint8_t)(i ? BCHP_CSR_RBUS_DATA0 : BCHP_CSR_CONFIG), i2c_buf, data_idx));

      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_4538_P_WaitForBbsiDone(h));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_WaitForBbsiDone()
******************************************************************************/
BERR_Code BHAB_4538_P_WaitForBbsiDone(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t retry;
   uint8_t status;

   for (retry = 0; retry < BHAB_MAX_BBSI_RETRIES; retry++)
   {
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if (((status & BCHP_CSR_STATUS_BUSY_MASK) == 0) || (status & 0x0F))
         break;
   }

   if (retry >= BHAB_MAX_BBSI_RETRIES)
      retCode = BERR_TIMEOUT;
   else if (status & 0x0F)
   {
      BDBG_ERR(("Error: CSR_STATUS=0x%X", status));
      retCode = BHAB_ERR_HOST_XFER;
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_RunAp() - called only in boot mode 0
******************************************************************************/
BERR_Code BHAB_4538_P_RunAp(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t ctrl;
   uint8_t status;

   /* check if the AP is currently running */



   BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
   if (status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
   {
      BDBG_WRN(("BHAB_4538_P_RunAp(): AP already running\n"));
   }
   else
   {
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK;
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK;
      ctrl &= ~BCHP_LEAP_CTRL_CTRL_UART_RST_MASK;
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_CTRL, &ctrl, 1));

      /* verify CPU is running */
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if ((status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) == 0)
      {
         BDBG_ERR(("unable to run the AP\n"));
         BERR_TRACE(retCode = BHAB_ERR_AP_FAIL);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_4538_P_ServiceHab(
   BHAB_Handle h,   /* [in] BHAB handle */
   uint8_t *read_buf,  /* [out] holds the data read from the HAB */
   uint16_t read_len,   /* [in] number of bytes to read from the HAB (including the checksum byte) */
   uint8_t ack_byte,   /* [in] value of the ack byte to expect */
   bool    bFixedLength /* [in] true = fixed length command */
)
{
   BHAB_4538_P_Handle *pImpl = (BHAB_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode, habErr;
   uint32_t mask, val32;
#ifdef BHAB_DONT_USE_INTERRUPT
   uint32_t  i;
#endif
   uint16_t  idx;
   uint8_t   cs, retry;

   /* clear the HAB_DONE interrupt */
   mask = BHAB_4538_HIRQ0_HAB_DONE;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask, 1));
   BKNI_WaitForEvent(pImpl->hHabDoneEvent, 0);

#ifndef BHAB_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_CLEAR0, &mask, 1));
#endif

   /* send the command */
   val32 = BCHP_LEAP_CTRL_HAB_REQUEST_HAB_REQUEST_MASK;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &val32, 1));

   /* assert the HAB interrupt on the LEAP */
   val32 = BCHP_LEAP_L2_CPU_SET_HAB_INTR_MASK;
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_L2_CPU_SET, &val32, 1));

#ifdef BHAB_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 5000; i++)
   {
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &val32, 1));
      if (val32 & BHAB_4538_HIRQ0_HAB_DONE)
         break;
   }
   if ((val32 & BHAB_4538_HIRQ0_HAB_DONE) == 0)
      retCode = BERR_TIMEOUT;
   else
   {
      /* clear the HAB_DONE interrupt status */
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &mask, 1));
   }
#else
   /* wait for HAB done interrupt */
   retCode = BHAB_4538_P_WaitForEvent(h, pImpl->hHabDoneEvent, 500);
#endif

   if (retCode == BERR_TIMEOUT)
   {
      uint32_t spare3, status0, mask0;
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3, 1));
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &mask0, 1));
      BDBG_ERR(("HAB timeout: spare3=0x%08X, status0=0x%08X, mask0=0x%08X", spare3, status0, mask0));
      BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
      goto done;
   }

   for (retry = 0; retry < 3; retry++) /* retry in case of i2c errors */
   {
      /* get the HAB contents */
      retCode = BHAB_4538_P_ReadHab(h, 0, read_buf, read_len);
      if (retCode == BERR_SUCCESS)
      {
         /* determine if the command was serviced */
         if (ack_byte == read_buf[0])
            break;
      }
      BDBG_WRN(("BHAB_4538_P_ServiceHab(): BHAB_4538_P_ReadHab() error 0x%X", retCode));
   }

   if (retCode)
      goto done;

   if (ack_byte != read_buf[0])
   {
      BERR_TRACE(BHAB_ERR_HAB_NO_ACK);
      retCode = BHAB_4538_P_DecodeHabError(h, &habErr);
      if (retCode == BERR_SUCCESS)
      {
         BDBG_ERR(("HAB command not serviced: err=0x%08X, ack_byte=0x%02X, expected=0x%02X\n", habErr, read_buf[0], ack_byte));
         retCode = habErr;
      }
      goto done;
   }

   if (bFixedLength)
   {
      /* validate the checksum */
      for (idx = cs = 0; idx < (read_len - 1); idx++)
      {
         cs += read_buf[idx];
#if 0
         BDBG_MSG(("ReadHab[%d] = 0x%02X", idx, read_buf[idx]));
#endif
      }
      if (cs != read_buf[read_len - 1])
      {
         BDBG_ERR(("Bad HAB checksum!\n"));
         BERR_TRACE(retCode = BHAB_ERR_HAB_CHECKSUM);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BHAB_4538_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_4538_P_WaitForEvent(
   BHAB_Handle h,             /* [in] BHAB handle */
   BKNI_EventHandle hEvent,   /* [in] event to wait on */
   int timeoutMsec            /* [in] timeout in milliseconds */
)
{
   BHAB_4538_P_Handle *pImpl = (BHAB_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BHAB_4538_P_EnableHostInterrupt(h, true);
   while (1)
   {
      retCode = BKNI_WaitForEvent(pImpl->hApiEvent, timeoutMsec);

      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR)) {
         break;
      }
      else if (retCode == BERR_SUCCESS)
      {
         BHAB_4538_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BHAB_4538_P_EnableHostInterrupt(h, true);
   }

   BHAB_4538_P_EnableHostInterrupt(h, true);
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_4538_P_DecodeInterrupt(BHAB_Handle h)
{
   BHAB_4538_P_Handle *pImpl = (BHAB_4538_P_Handle *)(h->pImpl);
   BERR_Code retCode;
   BHAB_4538_P_CallbackInfo *pCallbackInfo;
   uint32_t status0, status1, mask0, mask1, fstatus0, fstatus1, val32;
   uint32_t mask_set0 = 0, mask_set1 = 0;
   uint8_t channel;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS1, &status1, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_MASK_STATUS0, &mask0, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_MASK_STATUS1, &mask1, 1));

   fstatus0 = (status0 & ~mask0);
   fstatus1 = (status1 & ~mask1);
   if ((fstatus0 == 0) && (fstatus1 == 0))
      return BERR_SUCCESS;

#if 0
   BDBG_MSG(("status0, mask0 = 0x%08X, 0x%08X", status0, mask0));
   BDBG_MSG(("status1, mask1 = 0x%08X, 0x%08X", status1, mask1));
#endif

   if (fstatus0)
   {
      if (fstatus0 & BHAB_4538_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BHAB_4538_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BHAB_4538_HIRQ0_HAB_DONE)
      {
         BKNI_SetEvent(pImpl->hHabDoneEvent);
         mask_set0 |= BHAB_4538_HIRQ0_HAB_DONE;
      }

      if (fstatus0 & BHAB_4538_HIRQ0_DISEQC_DONE)
         mask_set0 |= BHAB_4538_HIRQ0_DISEQC_DONE;

      if (fstatus0 & BHAB_4538_HIRQ0_FLASH_DONE)
         mask_set0 |= BHAB_4538_HIRQ0_FLASH_DONE;

      if (fstatus0 & BHAB_4538_HIRQ0_WFE0_READY)
         mask_set0 |= BHAB_4538_HIRQ0_WFE0_READY;

      if (fstatus0 & BHAB_4538_HIRQ0_WFE1_READY)
         mask_set0 |= BHAB_4538_HIRQ0_WFE1_READY;

      if (fstatus0 & BHAB_4538_HIRQ0_WFE2_READY)
         mask_set0 |= BHAB_4538_HIRQ0_WFE2_READY;

      if (fstatus0 & BHAB_4538_HIRQ0_WFE3_READY)
         mask_set0 |= BHAB_4538_HIRQ0_WFE3_READY;
   }

   if (fstatus1)
   {
      for (channel = 0; channel < 8; channel++)
      {
         val32 = (fstatus1 >> (channel*4)) & 0x0F;
         if (val32 & BHAB_4538_HIRQ1_CHAN0_PEAK_SCAN_DONE)
         {
            mask_set1 |= (BHAB_4538_HIRQ1_CHAN0_PEAK_SCAN_DONE << (channel*4));
         }
      }
   }

   if ((fstatus0 & BHAB_4538_HIRQ0_AST_MASK) || (fstatus1 & BHAB_4538_HIRQ1_AST_MASK))
   {
      pCallbackInfo = &(pImpl->cbAst);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call ast callback */
         BKNI_EnterCriticalSection();
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
         BKNI_LeaveCriticalSection();
      }
   }

   if ((fstatus0 & BHAB_4538_HIRQ0_WFE_MASK) || (fstatus1 & BHAB_4538_HIRQ1_WFE_MASK))
   {
      pCallbackInfo = &(pImpl->cbWfe);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call wfe callback */
         BKNI_EnterCriticalSection();
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
         BKNI_LeaveCriticalSection();
      }
   }

   if ((fstatus0 & BHAB_4538_HIRQ0_FSK_MASK) || (fstatus1 & BHAB_4538_HIRQ1_FSK_MASK))
   {
      pCallbackInfo = &(pImpl->cbFsk);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call fsk callback */
         BKNI_EnterCriticalSection();
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
         BKNI_LeaveCriticalSection();
      }
   }

   /* clear the interrupt status */
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &fstatus0, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR1, &fstatus1, 1));

   if (mask_set0)
   {
      /* disable specified interrupts */
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask_set0, 1));
   }
   if (mask_set1)
   {
      /* disable specified interrupts */
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h,  BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask_set1, 1));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_4538_P_CheckHab(
   BHAB_Handle h    /* [in] BHAB handle */
)
{
   BERR_Code retCode = BHAB_ERR_HAB_NOT_AVAIL;
   uint32_t spare0, hab_request;
   uint8_t csr_status;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1));
   if (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK)
   {
      /* LEAP is running */
      BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0, 1));
      if (spare0 & BHAB_4538_SPARE0_INIT_DONE)
      {
         /* AP has initialized */
         BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request, 1));
         if ((hab_request & 1) == 0)
            retCode = BERR_SUCCESS; /* HAB is available */
      }
   }

   done:
   if (retCode != BERR_SUCCESS)
   {
      BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_ReadHab()
******************************************************************************/
BERR_Code BHAB_4538_P_ReadHab(BHAB_Handle h, uint16_t addr, uint8_t *buf, uint16_t n)
{
   if ((addr > 512) || (n > 512))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   if ((addr + n) > 512)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   return BHAB_4538_P_ReadMemory(h, BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE + (uint32_t)addr, buf, (uint32_t)n);
}


/******************************************************************************
 BHAB_4538_P_DecodeHabError() - called when command not ack'd, decoded error returned as habError
******************************************************************************/
BERR_Code BHAB_4538_P_DecodeHabError(BHAB_Handle h, BERR_Code *habError)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, spare3, leap_status, clear0 = 0, hab_error, fw_error;

   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_HOST_IRQ_STATUS0, &status0, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_ReadRbus(h, BCHP_LEAP_L2_CPU_STATUS, &leap_status, 1));

   BDBG_ERR(("status0=0x%08X, spare3=0x%08X, leap_l2_status=0x%08X", status0, spare3, leap_status));

   if (status0 & BHAB_4538_HIRQ0_HAB_ERROR)
   {
      clear0 |= BHAB_4538_HIRQ0_HAB_ERROR;

      /* decode HAB error code */
      hab_error = (spare3 & BHAB_4538_SPARE3_HAB_ERROR_CODE_MASK) >> BHAB_4538_SPARE3_HAB_ERROR_CODE_SHIFT;
      switch (hab_error)
      {
         case 0: /* HAB_SUCCESS */
            *habError = BERR_SUCCESS;
            break;
         case 1: /* invalid HAB command ID */
            *habError = BHAB_ERR_INVALID_OPCODE;
            break;
         case 2: /* checksum error */
            *habError = BHAB_ERR_HAB_CHECKSUM;
            break;
         case 3:  /* command not implemented */
            *habError = BHAB_ERR_HAB_NOT_SUPP;
            break;
         case 4: /* HAB_ERROR_PARAMETER */
            *habError = BHAB_ERR_HAB_BAD_PARAM;
            break;
         case 5: /* command not supported */
            *habError = BHAB_ERR_HAB_NOT_SUPP;
            break;
         case 6: /* HAB_ERROR_UNABLE_TO_SVC */
            *habError = BHAB_ERR_CMD_PROCESSING_ERR;
            break;
         case 7:  /* HAB_ERROR_BUSY */
            *habError = BHAB_ERR_HAB_RESOURCE_BUSY;
            break;
         default:
            *habError = BHAB_ERR_HAB_FAIL; /* should not get here */
            break;
      }
   }
   else if (status0 & BHAB_4538_HIRQ0_FW_ERROR)
   {
      clear0 |= BHAB_4538_HIRQ0_FW_ERROR;

      /* decode FW error code */
      fw_error = (spare3 & BHAB_4538_SPARE3_FW_ERROR_CODE_MASK) >> BHAB_4538_SPARE3_FW_ERROR_CODE_SHIFT;
      BDBG_ERR(("FW error 0x%X", fw_error));
      *habError = BHAB_ERR_AP_UNKNOWN; /* do this for now */
   }

   if (clear0)
   {
      BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_CLEAR0, &clear0, 1));
   }

   done:
   return retCode;
}



/******************************************************************************
 BHAB_4538_P_DisableFrontendInterrupts()
******************************************************************************/
BERR_Code BHAB_4538_P_DisableFrontendInterrupts(
   BHAB_Handle h   /* [in] BHAB handle */
)
{
   BERR_Code retCode;
   uint32_t mask = 0xFFFFFFFF;

   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET0, &mask, 1));
   BHAB_CHK_RETCODE(BHAB_4538_P_WriteRbus(h, BCHP_LEAP_HOST_IRQ_MASK_SET1, &mask, 1));

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4538_P_GetAvsData()
******************************************************************************/
BERR_Code BHAB_4538_P_GetAvsData(
   BHAB_Handle h,       /* [in] BHAB handle */
   BHAB_AvsData *pData  /* [out] returned AVS data */
)
{
   BERR_Code retCode;
   uint8_t buf[12];

   buf[0] = 0x29;
   BHAB_CHK_RETCODE(BHAB_SendHabCommand(h, buf, 11, buf, 11, true, true, 11));

   pData->enabled = (buf[1] & 0x01) ? true : false;
   pData->temperature = (uint32_t)((buf[2] << 24) | (buf[3] << 16) | (buf[4] << 8) | buf[5]);
   pData->voltage = (uint32_t)((buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9]);

   done:
   return retCode;
}
