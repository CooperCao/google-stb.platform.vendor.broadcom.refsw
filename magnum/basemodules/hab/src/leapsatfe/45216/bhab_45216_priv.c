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
#include "bhab_45216_priv.h"
#include "bchp_45216_leap_ctrl.h"
#include "bchp_45216_leap_hab_mem.h"
#include "bchp_45216_leap_host_l1.h"
#include "bchp_45216_leap_host_l2.h"
#include "bchp_45216_leap_l2.h"
#include "bchp_45216_leap_rom_patch.h"
#include "bchp_45216_csr.h"

#if 0 /* define BHAB_NO_IMG to disable IMG in the HAB module */
#define BHAB_NO_IMG
#endif

#ifndef BHAB_NO_IMG
#include "bhab_satfe_img.h"
#endif

BDBG_MODULE(bhab_45216_priv);

#define BHAB_LEAP_INIT_TIMEOUT 8000
#define BHAB_CMD_TIMEOUT       800
#define BHAB_SPI_READ_COMMAND  0x40
#define BHAB_SPI_WRITE_COMMAND 0x41

/* #define BHAB_DEBUG */
/* #define BHAB_JTAG_DEBUG */
/* #define BHAB_EXT_RAM_DEBUG */

#define BHAB_MAX_BBSI_RETRIES  10


/* local private functions */
BERR_Code BHAB_45216_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
BERR_Code BHAB_45216_P_RunAp(BHAB_Handle h);
BERR_Code BHAB_45216_P_ServiceHab(BHAB_Handle h, uint32_t *read_buf, uint16_t read_len);
BERR_Code BHAB_45216_P_CheckHab(BHAB_Handle h);
BERR_Code BHAB_45216_P_DecodeHabError(BHAB_Handle h);
BERR_Code BHAB_45216_P_DisableLeapInterrupts(BHAB_Handle h);
BERR_Code BHAB_45216_P_DecodeInterrupt(BHAB_Handle h);
bool BHAB_45216_P_IsLeapRunning(BHAB_Handle h);
BERR_Code BHAB_45216_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec);
BERR_Code BHAB_45216_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
BERR_Code BHAB_45216_P_WriteBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_45216_P_ReadBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_45216_P_ReadRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_45216_P_WriteRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_45216_P_WaitForBbsiDone(BHAB_Handle h);
BERR_Code BHAB_45216_P_DispatchCallback(BHAB_45216_P_CallbackInfo *pCallbackInfo, uint32_t status0, uint32_t status1);
void BHAB_45216_P_DumpGpRegisters(BHAB_Handle h, uint32_t startReg, uint32_t endReg);
#ifdef BHAB_VERIFY_DOWNLOAD
bool BHAB_45216_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len);
#endif


/******************************************************************************
 BHAB_45216_P_Open()
******************************************************************************/
BERR_Code BHAB_45216_P_Open(
   BHAB_Handle *handle,     /* [out] BHAB handle */
   void        *pReg,       /* [in] pointer to BREG handle */
   const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_Handle hDev;
   BHAB_45216_P_Handle *h45216Dev;

   BDBG_ASSERT(pDefSettings);
   BDBG_ASSERT(pDefSettings->interruptEnableFunc);

   hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
   BDBG_ASSERT(hDev);
   h45216Dev = (BHAB_45216_P_Handle *)BKNI_Malloc(sizeof(BHAB_45216_P_Handle));
   BDBG_ASSERT(h45216Dev);
   BKNI_Memset(h45216Dev, 0x00, sizeof(BHAB_45216_P_Handle));
   hDev->pImpl = (void*)h45216Dev;

   if (pDefSettings->isSpi)
      h45216Dev->hSpiRegister = (BREG_SPI_Handle)pReg;
   else
      h45216Dev->hI2cRegister = (BREG_I2C_Handle)pReg;

   BKNI_Memset((void*)&(h45216Dev->cbSat), 0, sizeof(BHAB_45216_P_CallbackInfo));
   BKNI_Memset((void*)&(h45216Dev->cbDsq), 0, sizeof(BHAB_45216_P_CallbackInfo));
   BKNI_Memset((void*)&(h45216Dev->cbFsk), 0, sizeof(BHAB_45216_P_CallbackInfo));
   BKNI_Memset((void*)&(h45216Dev->cbWfe), 0, sizeof(BHAB_45216_P_CallbackInfo));
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));

   /* create events */
   retCode = BKNI_CreateEvent(&(h45216Dev->hInterruptEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45216Dev->hApiEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45216Dev->hInitDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45216Dev->hHabDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45216Dev->hFlashDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   h45216Dev->habBaseAddr = BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE;

   *handle = hDev;
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_Close()
******************************************************************************/
BERR_Code BHAB_45216_P_Close(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_45216_P_Handle *p45216 = (BHAB_45216_P_Handle *)(handle->pImpl);

   BDBG_ASSERT(handle);

   /* disable host interrupts */
   retCode = BHAB_45216_P_DisableLeapInterrupts(handle);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BHAB_45216_P_DisableLeapInterrupts() error 0x%X", retCode));
   }

   /* reset the LEAP */
   retCode = BHAB_45216_P_Reset(handle);

   /* clean up */
   BKNI_DestroyEvent(p45216->hFlashDoneEvent);
   BKNI_DestroyEvent(p45216->hInitDoneEvent);
   BKNI_DestroyEvent(p45216->hHabDoneEvent);
   BKNI_DestroyEvent(p45216->hInterruptEvent);
   BKNI_DestroyEvent(p45216->hApiEvent);
   BKNI_Free((void*)p45216);
   BKNI_Free((void*)handle);
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_InitAp()
******************************************************************************/
BERR_Code BHAB_45216_P_InitAp(
    BHAB_Handle h,            /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM45216 microcode image */
)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t n, init_done_mask, retries, val32, strap, bootmode;
   uint32_t fwAddr = 0;
   const uint8_t *pImage, *pDataBuf;
#ifndef BHAB_NO_IMG
   unsigned chunk, num_chunks, segment, num_segments;
   unsigned fw_size;
   unsigned chunk_size = MAX_SATFE_IMG_CHUNK_SIZE;
   unsigned header_size = 0;
   BIMG_Interface *pImgInterface = (BIMG_Interface*)h->settings.pImgInterface;
   void **pImgContext = h->settings.pImgContext;
   void *pImg;

   BSTD_UNUSED(pHexImage);
   BSTD_UNUSED(pDataBuf);

   if ((pImgInterface == NULL) || (pImgContext == NULL))
   {
      BDBG_ERR(("HAB IMG interface and/or context not initialized"));
      return BERR_NOT_INITIALIZED;
   }

#endif

   /* disable host interrupts */
   BHAB_CHK_RETCODE(BHAB_45216_P_EnableHostInterrupt(h, false));
   BHAB_CHK_RETCODE(BHAB_45216_P_DisableLeapInterrupts(h));

   /* determine boot mode */
   BHAB_CHK_RETCODE(BHAB_45216_P_ReadRbus(h, BCHP_LEAP_CTRL_CPU_STRAPS, &strap, 1));
   bootmode = strap & BCHP_LEAP_CTRL_CPU_STRAPS_BOOT_MODE_MASK;

   /* clear init_done interrupt status */
   init_done_mask = BHAB_45216_HIRQ0_INIT_DONE;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRbus(h, BCHP_LEAP_HOST_L2_CLEAR0, &init_done_mask, 1));
   BHAB_45216_P_WaitForEvent(h, pImpl->hInitDoneEvent, 0);

   /* reset the AP before downloading the firmware */
   BHAB_CHK_RETCODE(BHAB_45216_P_Reset(h));
   if (bootmode == 0)
   {
      /* boot mode 0 */
      if (BHAB_45216_P_IsLeapRunning(h))
         return BERR_TRACE(BHAB_ERR_AP_FAIL);

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

         fwAddr = (uint32_t)(*pImage++ << 24);
         fwAddr |= (uint32_t)(*pImage++ << 16);
         fwAddr |= (uint32_t)(*pImage++ << 8);
         fwAddr |= (uint32_t)(*pImage++);

         pDataBuf = pImage;

         for (retries = 0; retries < 3; retries++)
         {
            BDBG_MSG(("writing %d bytes starting from 0x%04X", n, fwAddr));
            retCode = BHAB_45216_P_WriteMemory(h, fwAddr, (uint8_t*)pImage, n);
            if (retCode == BERR_SUCCESS)
            {
#ifdef BHAB_VERIFY_DOWNLOAD
               if (BHAB_45216_VerifyMemory(h, fwAddr, pDataBuf, n))
                  break;
               BDBG_ERR(("data read back does not match\n"));
#else
               break;
#endif
            }
            else
            {
               BDBG_ERR(("BHAB_45216_P_WriteMemory() error 0x%X", retCode));
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
      BHAB_CHK_RETCODE(pImgInterface->open((void*)pImgContext, &pImg, 0));
      /* read header */
      BHAB_CHK_RETCODE(pImgInterface->next(pImg, 0, (const void **)&pImage, MAX_SATFE_IMG_QUERY_SIZE));

      header_size = (unsigned)pImage[0];
      num_segments = (unsigned)pImage[1];

      for (segment = 0; segment < num_segments; segment++) {

          /* read header */
          BHAB_CHK_RETCODE(pImgInterface->next(pImg, segment, (const void **)&pImage, header_size));

          n = (uint32_t)(*pImage++ << 16);
          n |= (uint32_t)(*pImage++ << 8);
          n |= (uint32_t)(*pImage++);

          fwAddr = (uint32_t)(*pImage++ << 24);
          fwAddr |= (uint32_t)(*pImage++ << 16);
          fwAddr |= (uint32_t)(*pImage++ << 8);
          fwAddr |= (uint32_t)(*pImage++);

          fw_size = n;
          num_chunks = fw_size / chunk_size;
          if (fw_size % chunk_size)
              num_chunks++;

          for (chunk = 0; chunk < num_chunks; chunk++)
          {
              BDBG_MSG(("BHAB_45216_P_InitAp: img next[%d of %d]...", chunk, num_chunks));
              if (chunk == num_chunks-1) {
                  n = fw_size % chunk_size;
              } else
                  n = chunk_size;
              BDBG_MSG(("requesting %d bytes", n));
              BHAB_CHK_RETCODE(pImgInterface->next(pImg, chunk, (const void **)&pImage, n));

             for (retries = 0; retries < 3; retries++)
             {
                BDBG_MSG(("writing %d bytes starting from 0x%04X", n, fwAddr + chunk*chunk_size));
                retCode = BHAB_45216_P_WriteMemory(h, fwAddr + chunk*chunk_size, (uint8_t*)pImage, n);
                if (retCode == BERR_SUCCESS)
                {
#ifdef BHAB_VERIFY_DOWNLOAD
                   if (BHAB_45216_VerifyMemory(h, fwAddr + chunk*chunk_size, pDataBuf, n))
                      break;
                   BDBG_ERR(("data read back does not match\n"));
#else
                   break;
#endif
                }
                else
                {
                   BDBG_ERR(("BHAB_45216_P_WriteMemory() error 0x%X", retCode));
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

      pImgInterface->close(pImg);
#endif

#ifdef BHAB_VERIFY_DOWNLOAD
      BDBG_MSG(("firmware loaded successfully in RAM"));
#endif
   }
   else if ((bootmode == 1) || (bootmode == 2))
   {
      /* autoboot from external flash */
      BDBG_MSG(("LEAP Boot Mode %d", bootmode));
   }
   else
   {
      BDBG_ERR(("unsupported LEAP boot mode (%d)", bootmode));
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
      goto done;
   }

#ifndef BHAB_DONT_USE_INTERRUPT
   /* enable init done interrupt */
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRbus(h, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &init_done_mask, 1));
#endif

   if (BHAB_45216_P_IsLeapRunning(h) == false)
   {
      /* start running the AP */
      BHAB_CHK_RETCODE(BHAB_45216_P_RunAp(h));
      BDBG_MSG(("LEAP is running"));
   }

#ifdef BHAB_DONT_USE_INTERRUPT
   /* poll for AP init done */
   for (n = 0; n < BHAB_LEAP_INIT_TIMEOUT; n++)
   {
      BHAB_45216_P_ReadRbus(h, BCHP_LEAP_HOST_L2_STATUS0, &val32, 1);
      if (val32 & BHAB_45216_HIRQ0_INIT_DONE)
         break;
      BKNI_Sleep(1);
   }
   if ((val32 & BHAB_45216_HIRQ0_INIT_DONE) == 0)
   {
      BDBG_ERR(("AP initialization timeout\n"));
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
   }
#else
   /* wait for init done interrupt */
   if (BHAB_45216_P_WaitForEvent(h, pImpl->hInitDoneEvent, BHAB_LEAP_INIT_TIMEOUT) != BERR_SUCCESS)
   {
      BHAB_CHK_RETCODE(BHAB_45216_P_ReadRbus(h, BCHP_LEAP_HOST_L2_STATUS0, &val32, 1));
      if ((val32 & BHAB_45216_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout\n"));
         BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
         goto done;
      }
      else
      {
         BDBG_ERR(("firmware initialized but host didnt get init_done irq!"));
      }
   }

   /* disable the init_done interrupt */
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRbus(h, BCHP_LEAP_HOST_L2_MASK_SET0, &init_done_mask, 1));
#endif

   done:
   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("LEAP initialized successfully"));
   }
   else
   {
      BDBG_ERR(("BHAB_45216_P_InitAp() error 0x%X", retCode));
   }
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_WriteRegister()
******************************************************************************/
BERR_Code BHAB_45216_P_WriteRegister(
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
   if ((reg > 0x7d008ef) || ((reg < 0xc8000) && !bHsi))
   {
      BDBG_ERR(("BHAB_45216_P_WriteRegister(): bad address (0x%X)", reg));
      return BERR_INVALID_PARAMETER;
   }

   if (bHsi)
   {
      sb = (uint8_t)(*val & 0xFF);
      retCode = BHAB_45216_P_WriteBbsi(h, (uint8_t)reg, &sb, 1);
   }
   else
      retCode = BHAB_45216_P_WriteRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_45216_P_ReadRegister()
******************************************************************************/
BERR_Code BHAB_45216_P_ReadRegister(
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
   if ((reg > 0x7d008ef) || ((reg < 0xc8000) && !bHsi))
   {
      BDBG_ERR(("BHAB_45216_P_ReadRegister(): bad address (0x%X)", reg));
      return BERR_INVALID_PARAMETER;
   }

   if (bHsi)
   {
      retCode = BHAB_45216_P_ReadBbsi(h, (uint8_t)reg, &sb, 1);
      if (retCode == BERR_SUCCESS)
         *val = (uint32_t)sb;
   }
   else
      retCode = BHAB_45216_P_ReadRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_45216_P_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_45216_P_GetVersionInfo(BHAB_Handle handle, BFEC_SystemVersionInfo *p)
{
   BERR_Code retCode;
   uint32_t buf[11];

   BKNI_Memset(buf, 0, 11*sizeof(uint32_t));
   buf[0] = BHAB_45216_InitHeader(0x04, 0, 0, 0);
   BHAB_CHK_RETCODE(BHAB_45216_P_SendCommand(handle, buf, 11));
   p->familyId = buf[1];
   p->chipId = buf[2];
   p->chipVersion = buf[3];
   p->bondoutOptions[0] = buf[4];
   p->bondoutOptions[1] = buf[5];
   p->firmware.majorVersion = buf[6];
   p->firmware.minorVersion = buf[7];
   p->firmware.buildType = buf[8];
   p->firmware.buildId = buf[9];

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_GetApStatus()
******************************************************************************/
BERR_Code BHAB_45216_P_GetApStatus(
   BHAB_Handle h,          /* [in] HAB device handle */
   BHAB_ApStatus *pStatus  /* [out] AP status */
)
{
   uint32_t spare3, hab_request;

   *pStatus = 0;

   if (BHAB_45216_P_IsLeapRunning(h))
   {
      /* LEAP is running */
      BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
      if (spare3 & BHAB_45216_SPARE3_INIT_DONE)
      {
         /* AP has initialized */
         *pStatus |= BHAB_APSTATUS_INIT_DONE;

         BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request);
         if ((hab_request & 1) == 0)
            *pStatus |= BHAB_APSTATUS_HAB_READY;
      }
   }
   else
      *pStatus |= BHAB_APSTATUS_RESET;

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_WriteMemory()
******************************************************************************/
BERR_Code BHAB_45216_P_WriteMemory(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, addr = start_addr, nWords, nWordsLeft, leftover, len, hab[128];
   bool bIsRunning;
   uint8_t i2c_buf[5], *pBuf = (uint8_t *)buf;

   /* make sure memory address is 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03) || (start_addr < 0x40000UL) ||
       ((start_addr >= 0xC9000UL) && (start_addr < 0x80000000UL)) ||
       ((start_addr >= 0xB0000) && (start_addr < 0xc8000)) ||
       (start_addr >= 0x80028000))
      return BERR_INVALID_PARAMETER;

   bIsRunning = BHAB_45216_P_IsLeapRunning(h);
   if ((bIsRunning && ((addr >= 0x40000) && (addr < 0x80000))))
      return BHAB_ERR_MEMAV; /* cannot write to instruction memory while running */

   leftover = n % 4;
   nWords = (n >> 2);

   if (bIsRunning && ((addr < BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE) || (addr & 0x80000000)))
   {
      /* make sure addr and length is 4-byte aligned */
      if (n & 0x03)
         return BERR_INVALID_PARAMETER;

      /* send HAB command to write to memory */
      nWordsLeft = nWords;
      addr = start_addr;
      while (nWordsLeft > 0)
      {
         if (nWordsLeft > 120)
            len = 120;
         else
            len = nWordsLeft;
         BKNI_Memset(hab, 0, 128*sizeof(uint32_t));
         hab[0] = BHAB_45216_InitHeader(0x01, 0, BHAB_45216_WRITE, 0);
         hab[1] = addr;
         hab[2] = len;
         for (i = 0; i < len; i++)
         {
            hab[3+i] = (uint32_t)(*pBuf++ << 24);
            hab[3+i] |= (uint32_t)(*pBuf++ << 16);
            hab[3+i] |= (uint32_t)(*pBuf++ << 8);
            hab[3+i] |= (uint32_t)*pBuf++;
         }
         BHAB_CHK_RETCODE(BHAB_45216_P_SendCommand(h, hab, 4+len));

         nWordsLeft -= len;
         addr += (len<<2);
      }
   }
   else
   {
      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));

      i2c_buf[0] = 0x00; /* BCHP_CSR_CONFIG: 32-bit writes */
      i2c_buf[1] = (uint8_t)((start_addr >> 24) & 0xFF);
      i2c_buf[2] = (uint8_t)((start_addr >> 16) & 0xFF);
      i2c_buf[3] = (uint8_t)((start_addr >> 8) & 0xFF);
      i2c_buf[4] = (uint8_t)(start_addr & 0xFF);
      BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));

      for (i = 0; i < nWords; i++)
      {
         /* wait for !busy or error */
         BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));

         if (bIsRunning)
         {
            i2c_buf[0] = buf[i<<2];
            i2c_buf[1] = buf[(i<<2)+1];
            i2c_buf[2] = buf[(i<<2)+2];
            i2c_buf[3] = buf[(i<<2)+3];
            BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
         }
         else
         {
            if (h->settings.isSpi == false)
            {
               /* for i2c, we can write entire buf in one transaction if CPU not running */
               BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, (uint8_t *)buf, nWords<<2));
            }
            else
            {
               BREG_SPI_SetContinueAfterCommand(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, true); /* assert and hold the chip select until the completion of BREG_SPI_WriteAlls */
               i2c_buf[0] = BHAB_SPI_WRITE_COMMAND;
               i2c_buf[1] = BCHP_CSR_RBUS_DATA0;
               BHAB_CHK_RETCODE(BREG_SPI_WriteAll(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, i2c_buf, 2)); /* Write chip address and sub address as part of first write of two bytes*/
               BHAB_CHK_RETCODE(BREG_SPI_WriteAll(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, buf, nWords<<2));
               BREG_SPI_SetContinueAfterCommand(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, false);
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
         BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_ReadMemory()
******************************************************************************/
BERR_Code BHAB_45216_P_ReadMemory(BHAB_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, j, buf_idx, addr, end_addr, nReads, x, idx, val32, hab[128];
   int32_t nReadsLeft;
   bool bIsRunning, bCsrAddrSet = false;
   uint8_t i2c_buf[5];

   if ((n == 0) || ((start_addr >= 0xB0000) && (start_addr < 0xc8000)) ||
       ((start_addr >= 0xC9000UL) && (start_addr < 0x80000000UL)) ||
       ((start_addr >= 0x80028000UL) && (start_addr < 0xC0000000UL)) ||
       (start_addr >= 0xC0080000))
      return BERR_INVALID_PARAMETER;

   addr = start_addr & ~0x03; /* actual starting address is 32-bit aligned */
   end_addr = start_addr + n;
   if (end_addr & 0x03)
      end_addr = (end_addr & ~0x03) + 4;
   nReads = (end_addr - addr) >> 2; /* total number of 32-bit reads */

   bIsRunning = BHAB_45216_P_IsLeapRunning(h);
   if (bIsRunning && ((addr < BCHP_LEAP_HAB_MEM_WORDi_ARRAY_BASE) || (addr & 0x80000000)))
   {
      /* make sure addr and length is 4-byte aligned */
      if ((start_addr & 0x03) || (n & 0x03))
         return BERR_INVALID_PARAMETER;

      /* send HAB command to read memory */
      nReadsLeft = nReads;
      idx = 0;
      while (nReadsLeft > 0)
      {
         if (nReadsLeft > 120)
            x = 120;
         else
            x = nReadsLeft;

         BKNI_Memset(hab, 0, 128*sizeof(uint32_t));
         hab[0] = BHAB_45216_InitHeader(0x01, 0, BHAB_45216_READ, 0);
         hab[1] = addr;
         hab[2] = x;
         BHAB_CHK_RETCODE(BHAB_45216_P_SendCommand(h, hab, 4+x));

         for (i = 0; i < x; i++)
         {
            buf[idx++] = (uint8_t)(hab[3+i] & 0xFF);
            buf[idx++] = (uint8_t)((hab[3+i] >> 8) & 0xFF);
            buf[idx++] = (uint8_t)((hab[3+i] >> 16) & 0xFF);
            buf[idx++] = (uint8_t)((hab[3+i] >> 24) & 0xFF);
         }

         nReadsLeft -= x;
         addr += (4*x);
      }
   }
   else
   {
      for (i = buf_idx = 0; i < nReads; i++)
      {
         /* wait for !busy or error */
         BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));

         if (i == 0)
         {
            x = start_addr & 0x03;
            if (x)
            {
               /* starting address is not 32-bit aligned */
               BHAB_CHK_RETCODE(BHAB_45216_P_ReadRbus(h, addr, &val32, 1));
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
            BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
            bCsrAddrSet = true;
         }

         BHAB_CHK_RETCODE(BHAB_45216_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
         for (j = 0; (j < 4) && (buf_idx < n); j++)
            buf[buf_idx++] = i2c_buf[j];
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_45216_P_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
   BHAB_45216_P_Handle *p45216;
   BHAB_45216_P_CallbackInfo *pCbInfo;

   BDBG_ASSERT(handle);
   p45216 = (BHAB_45216_P_Handle *)(handle->pImpl);

   switch (eDevId)
   {
      case BHAB_DevId_eSAT:
         pCbInfo = &(p45216->cbSat);
         break;

      case BHAB_DevId_eDSQ:
         pCbInfo = &(p45216->cbDsq);
         break;

      case BHAB_DevId_eFSK:
         pCbInfo = &(p45216->cbFsk);
         break;

      case BHAB_DevId_eWFE:
         pCbInfo = &(p45216->cbWfe);
         break;

      default:
         BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   pCbInfo->func = fCallBack;
   pCbInfo->callbackParams.pParm1 = pParm1;
   pCbInfo->callbackParams.parm2 = parm2;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_45216_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
   BHAB_45216_P_Handle *p45216;
   BHAB_45216_P_CallbackInfo *pCbInfo;

   BDBG_ASSERT(handle);
   p45216 = (BHAB_45216_P_Handle *)(handle->pImpl);

   switch (eDevId)
   {
      case BHAB_DevId_eSAT:
         pCbInfo = &(p45216->cbSat);
         break;

      case BHAB_DevId_eDSQ:
         pCbInfo = &(p45216->cbDsq);
         break;

      case BHAB_DevId_eFSK:
         pCbInfo = &(p45216->cbFsk);
         break;

      case BHAB_DevId_eWFE:
         pCbInfo = &(p45216->cbWfe);
         break;

      default:
         BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   pCbInfo->func = NULL;
   pCbInfo->callbackParams.pParm1 = NULL;
   pCbInfo->callbackParams.parm2 = 0;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_SendCommand()
******************************************************************************/
BERR_Code BHAB_45216_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_45216_P_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}


/******************************************************************************
 BHAB_45216_P_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_45216_P_SendHabCommand(
    BHAB_Handle handle,  /* [in] BHAB PI Handle */
    uint8_t *write_buf,  /* [in] specifies the HAB command to send */
    uint16_t write_len,  /* [in] number of 32-bit words in the HAB command */
    uint8_t *read_buf,   /* [out] holds the data read from the HAB */
    uint16_t read_len,   /* [in] number of 32-bit words to read from the HAB */
    bool bCheckForAck,   /* [in] not used */
    bool bFixedLength,   /* [in] not used */
    uint16_t command_len /* [in] not used */
)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(handle->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t *pWriteBuf = (uint32_t*)write_buf;
   uint32_t *pReadBuf = (uint32_t*)read_buf;
   uint32_t checksum;
   uint16_t i;

   BSTD_UNUSED(command_len);
   BSTD_UNUSED(bCheckForAck);
   BSTD_UNUSED(bFixedLength);

   if ((write_len >= 128) || (read_len >= 128) || (write_len < 2) || (read_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BHAB_CHK_RETCODE(BHAB_45216_P_CheckHab(handle));

   /* compute the checksum */
   checksum = 0;
   for (i = 0; i < (write_len-1); i++)
      checksum += pWriteBuf[i];
   pWriteBuf[write_len-1] = checksum;

   for (i = 0; i < write_len; i++)
   {
#ifdef BHAB_DEBUG
      if (i == 0)
      {
         BDBG_WRN(("send HAB command:"));
      }
      BDBG_WRN(("   HAB[%d]=0x%08X", i, pWriteBuf[i]));
#endif
      BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(handle, pImpl->habBaseAddr + (i<<2), &pWriteBuf[i]));
   }

   /* wait for the AP to service the HAB, and then read any return data */
   BHAB_CHK_RETCODE(BHAB_45216_P_ServiceHab(handle, pReadBuf, read_len));

#ifdef BHAB_DEBUG
   BDBG_WRN(("rcvd HAB response:"));
   for (i = 0; i < (read_len*4); i++)
   {
      BDBG_WRN(("   HAB[%d]=0x%02X", i, readBuf[i]));
   }
#endif

   /* verify checksum */
   checksum = 0;
   for (i = 0; i < (read_len - 1); i++)
      checksum += pReadBuf[i];
   if (checksum != pReadBuf[read_len-1])
   {
      BERR_TRACE(retCode = BHAB_ERR_HAB_CHECKSUM);
   }
   else if ((pReadBuf[0] & BHAB_45216_ACK) == 0)
   {
      retCode = BHAB_45216_P_DecodeHabError(handle);
      if (retCode == BERR_SUCCESS)
      {
         /* couldn't find a reason for the no-ack */
         BERR_TRACE(retCode = BHAB_ERR_HAB_NO_ACK);
      }
   }

   done:
   return retCode;
}


/****************************Private Functions*********************************/


/******************************************************************************
 BHAB_45216_P_Reset()
******************************************************************************/
BERR_Code BHAB_45216_P_Reset(BHAB_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t ctrl, val32, i;

   /* reset LEAP/CPU */
   BHAB_CHK_RETCODE(BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl));
   ctrl |= (BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK | BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK | BCHP_LEAP_CTRL_CTRL_UART_RST_MASK);
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_START_ARC_MASK; /* clear start_arc */
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl));

   /* initialize some LEAP registers */
   val32 = 0x101;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_HAB_CNTR_CLR, &val32));
   val32 = 0;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE0, &val32));
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE1, &val32));
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE2, &val32));
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &val32));
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_HAB_CNTR_CLR, &val32));
   for (i = 0; i < 64; i++)
   {
      BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_GP0 + (i<<2), &val32));
   }

   /* set up the L1 interrupt for the LEAP */
   val32 = BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR_LEAP_INTR_MASK;
   BHAB_CHK_RETCODE(BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR, &val32));
#if 0
   if (BHAB_45216_P_IsLeapRunning(h))
      return BERR_TRACE(BHAB_ERR_AP_FAIL);
#endif

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_RunAp()
******************************************************************************/
BERR_Code BHAB_45216_P_RunAp(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t ctrl;

   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl);
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK;
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK;
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_UART_RST_MASK;
#ifndef BHAB_JTAG_DEBUG
   ctrl |= BCHP_LEAP_CTRL_CTRL_START_ARC_MASK;
#endif
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl);

#ifdef BHAB_JTAG_DEBUG
   BDBG_ERR(("connect to JTAG..."));
   while(1);
#endif

   if (BHAB_45216_P_IsLeapRunning(h))
      return BERR_SUCCESS;
   else
   {
      retCode = BHAB_ERR_AP_FAIL;
      BERR_TRACE(retCode);
      return retCode;
   }
}


/******************************************************************************
 BHAB_45216_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_45216_P_ServiceHab(
   BHAB_Handle h,      /* [in] BHAB handle */
   uint32_t *read_buf,  /* [out] holds the data read from the HAB */
   uint16_t read_len   /* [in] number of words to read from the HAB (including the checksum) */
)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val32, i;

   /* clear the HAB_DONE interrupt */
   mask = BHAB_45216_HIRQ0_HAB_DONE;
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &mask);
   BHAB_45216_P_WaitForEvent(h, pImpl->hHabDoneEvent, 0);

#ifndef BHAB_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &mask);
#endif

   /* send the command */
   val32 = BCHP_LEAP_CTRL_HAB_REQUEST_HAB_REQUEST_MASK;
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &val32);

   /* assert the HAB interrupt on the LEAP */
   val32 = BCHP_LEAP_L2_CPU_SET_HAB_INTR_MASK;
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_L2_CPU_SET, &val32);

#ifdef BHAB_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < BHAB_CMD_TIMEOUT; i++)
   {
      val32 = 0;
      BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if (val32 & BHAB_45216_HIRQ0_HAB_DONE)
         break;
      BKNI_Sleep(1);
   }
   if ((val32 & BHAB_45216_HIRQ0_HAB_DONE) == 0)
      retCode = BERR_TIMEOUT;
   else
   {
      /* clear the HAB_DONE interrupt status */
      BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &mask);
   }
#else
   /* wait for HAB done interrupt */
   retCode = BHAB_45216_P_WaitForEvent(h, pImpl->hHabDoneEvent, BHAB_CMD_TIMEOUT);
#endif

   if (retCode == BERR_TIMEOUT)
   {
      /* check if HAB is done */
      BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if ((val32 & BHAB_45216_HIRQ0_HAB_DONE) == 0)
      {
         uint32_t spare3, status0, mask0;
         BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
         BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
         BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS0, &mask0);
         BDBG_ERR(("HAB timeout: last_cmd_rcvd=0x%X, last_cmd_svcd=0x%X, status0=0x%X, mask0=0x%X",
                   (spare3 >> 16) & 0xFF, (spare3 >> 24) & 0xFF, status0, mask0));

         BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
         goto done;
      }
      else
      {
         BDBG_WRN(("missed hab done irq"));
      }
   }

   /* get the HAB contents */
   for (i = 0; (retCode == BERR_SUCCESS) && (i < read_len); i++)
   {
      retCode = BHAB_45216_P_ReadRegister(h, pImpl->habBaseAddr + (i<<2), &read_buf[i]);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_45216_P_HandleInterrupt_isr(
    BHAB_Handle h /* [in] BHAB handle */
)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(h->pImpl);
   BDBG_ASSERT(h);

   h->settings.interruptEnableFunc(false, h->settings.interruptEnableFuncParam);
   BKNI_SetEvent(pImpl->hApiEvent);
   BKNI_SetEvent(pImpl->hInterruptEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_DispatchCallback()
******************************************************************************/
BERR_Code BHAB_45216_P_DispatchCallback(BHAB_45216_P_CallbackInfo *pCallbackInfo, uint32_t status0, uint32_t status1)
{
   BERR_Code retCode = BERR_SUCCESS;

   pCallbackInfo->callbackParams.status0 = status0;
   pCallbackInfo->callbackParams.status1 = status1;

   if (pCallbackInfo->func)
   {
      retCode = pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
   }
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_45216_P_DecodeInterrupt(BHAB_Handle h)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, status1, mask0, mask1, fstatus0, fstatus1;
   uint32_t mask_set0 = 0, mask_set1 = 0;

   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS1, &status1);
   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS0, &mask0);
   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS1, &mask1);

   fstatus0 = (status0 & ~mask0);
   fstatus1 = (status1 & ~mask1);
   if ((fstatus0 == 0) && (fstatus1 == 0))
      goto done;

#if 0
   BDBG_MSG(("status0,mask0,fstatus0=%08X,%08X,%08X", status0, mask0, fstatus0));
   BDBG_MSG(("status1,mask1,fstatus1=%08X,%08X,%08X", status1, mask1, fstatus1));
#endif

   if (fstatus0)
   {
      if (fstatus0 & BHAB_45216_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BHAB_45216_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BHAB_45216_HIRQ0_HAB_DONE)
      {
         BKNI_SetEvent(pImpl->hHabDoneEvent);
         mask_set0 |= BHAB_45216_HIRQ0_HAB_DONE;
      }

      if (fstatus0 & BHAB_45216_HIRQ0_FLASH_DONE)
      {
         BKNI_SetEvent(pImpl->hFlashDoneEvent);
         mask_set0 |= BHAB_45216_HIRQ0_FLASH_DONE;
      }

      if (fstatus0 & BHAB_45216_HIRQ0_SAT_INIT_DONE)
         mask_set0 |= BHAB_45216_HIRQ0_SAT_INIT_DONE;

      if (fstatus0 & BHAB_45216_HIRQ0_FSK_MASK)
      {
         retCode = BHAB_45216_P_DispatchCallback(&(pImpl->cbFsk), fstatus0, fstatus1);
      }

      if (fstatus0 & BHAB_45216_HIRQ0_DSQ_MASK)
      {
         if (fstatus0 & BHAB_45216_HIRQ0_DSQ0_TX)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ0_TX;
         if (fstatus0 & BHAB_45216_HIRQ0_DSQ0_RX)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ0_RX;
         if (fstatus0 & BHAB_45216_HIRQ0_DSQ0_VSENSE)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ0_VSENSE;

         if (fstatus0 & BHAB_45216_HIRQ0_DSQ1_TX)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ1_TX;
         if (fstatus0 & BHAB_45216_HIRQ0_DSQ1_RX)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ1_RX;
         if (fstatus0 & BHAB_45216_HIRQ0_DSQ1_VSENSE)
            mask_set0 |= BHAB_45216_HIRQ0_DSQ1_VSENSE;

         retCode = BHAB_45216_P_DispatchCallback(&(pImpl->cbDsq), fstatus0, fstatus1);
      }

      if (fstatus0 & BHAB_45216_HIRQ0_WFE_MASK)
      {
         if (fstatus0 & BHAB_45216_HIRQ0_WFE0_READY)
            mask_set0 |= BHAB_45216_HIRQ0_WFE0_READY;

         if (fstatus0 & BHAB_45216_HIRQ0_WFE1_READY)
            mask_set0 |= BHAB_45216_HIRQ0_WFE1_READY;

         if (fstatus0 & BHAB_45216_HIRQ0_WFE_SA_DONE)
            mask_set0 |= BHAB_45216_HIRQ0_WFE_SA_DONE;

         retCode = BHAB_45216_P_DispatchCallback(&(pImpl->cbWfe), fstatus0, fstatus1);
      }
   }

   if (fstatus1 || (fstatus0 & BHAB_45216_HIRQ0_SAT_MASK))
   {
      retCode = BHAB_45216_P_DispatchCallback(&(pImpl->cbSat), fstatus0, fstatus1);
   }

   /* clear the interrupt status */
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &fstatus0);
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR1, &fstatus1);

   if (mask_set0)
   {
      /* disable specified interrupts */
      BHAB_45216_P_WriteRegister(h,  BCHP_LEAP_HOST_L2_MASK_SET0, &mask_set0);
   }
   if (mask_set1)
   {
      /* disable specified interrupts */
      BHAB_45216_P_WriteRegister(h,  BCHP_LEAP_HOST_L2_MASK_SET1, &mask_set1);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_IsLeapRunning()
******************************************************************************/
bool BHAB_45216_P_IsLeapRunning(
   BHAB_Handle h  /* [in] BHAB handle */
)
{
   uint8_t csr_status;
   bool bRunning = false;

   if (BHAB_45216_P_ReadBbsi(h,  BCHP_CSR_STATUS, &csr_status, 1) == BERR_SUCCESS)
      bRunning = (csr_status & BCHP_CSR_STATUS_CPU_RUNNING_MASK) ? true : false;

   return bRunning;
}


/******************************************************************************
 BHAB_45216_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_45216_P_CheckHab(
   BHAB_Handle h    /* [in] BHAB handle */
)
{
   BERR_Code retCode = BHAB_ERR_HAB_NOT_AVAIL;
   uint32_t spare3, hab_request;

   if (BHAB_45216_P_IsLeapRunning(h))
   {
      /* LEAP is running */
      BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
      if (spare3 & BHAB_45216_SPARE3_INIT_DONE)
      {
         /* LEAP has initialized */
         BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request);
         if ((hab_request & 1) == 0)
            retCode = BERR_SUCCESS; /* HAB is available */
      }
   }

   if (retCode != BERR_SUCCESS)
   {
      BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_DecodeHabError() - called when the command was not ack'd
******************************************************************************/
BERR_Code BHAB_45216_P_DecodeHabError(BHAB_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, spare3, gp26, fw_err = 0;

   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
   BHAB_45216_P_ReadRegister(h, BCHP_LEAP_CTRL_GP26, &gp26);

   BDBG_ERR(("status0=0x%08X, spare3=0x%08X, gp26=0x%08X", status0, spare3, gp26));

   if (status0 & BHAB_45216_HIRQ0_FW_ERROR)
   {
      fw_err = spare3 & BHAB_45216_SPARE3_FW_ERROR_CODE_MASK;
      switch (fw_err)
      {
         case 0:
            goto get_hab_error_code;

         case 1: /* LEAP unhandled exception */
         case 9: /* LEAP instruction error */
         case 8: /* MPU error */
            retCode = BHAB_ERR_AP_IRQ;
            break;

         case 2: /* SAT error */
            BDBG_ERR(("SAT (LEAP) error:"));
            BHAB_45216_P_DumpGpRegisters(h, 0, 16);
            retCode = BHAB_ERR_CMD_PROCESSING_ERR;
            break;

         case 5: /* WFE error */
            BDBG_ERR(("WFE (LEAP) error:"));
            BHAB_45216_P_DumpGpRegisters(h, 17, 19);
            retCode = BHAB_ERR_CMD_PROCESSING_ERR;
            break;

         case 11: /* DSQ error */
            BDBG_ERR(("DSQ (LEAP) error:"));
            BHAB_45216_P_DumpGpRegisters(h, 20, 22);
            retCode = BHAB_ERR_CMD_PROCESSING_ERR;
            break;

         case 12: /* FSK error */
            BDBG_ERR(("FSK (LEAP) error:"));
            BHAB_45216_P_DumpGpRegisters(h, 23, 24);
            retCode = BHAB_ERR_CMD_PROCESSING_ERR;
            break;

         case 10:
            BDBG_ERR(("LEAP task closed unexpectedly:"));
            BHAB_45216_P_DumpGpRegisters(h, 25, 27);
            retCode = BHAB_ERR_AP_UNKNOWN;
            break;

         default:
            retCode = BHAB_ERR_AP_UNKNOWN;
            break;
      }
   }
   else if (gp26 != 0)
   {
      /* decode HAB error code */
      get_hab_error_code:
      retCode = gp26;
   }
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_DisableLeapInterrupts()
******************************************************************************/
BERR_Code BHAB_45216_P_DisableLeapInterrupts(
   BHAB_Handle h   /* [in] BHAB handle */
)
{
   uint32_t mask = 0xFFFFFFFF;

   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_SET0, &mask);
   BHAB_45216_P_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_SET1, &mask);

   return BERR_SUCCESS;
}


#ifdef BHAB_VERIFY_DOWNLOAD
/******************************************************************************
 BHAB_45216_P_CompareMemory()
******************************************************************************/
int BHAB_45216_P_CompareMemory(const void *p1, const void *p2, size_t n)
{
#if 0
   return BKNI_Memcmp(p1, p2, n);
#else
   size_t i;
   int nerrors = 0;
   uint8_t *pBuf1 = (uint8_t*)p1, *pBuf2 = (uint8_t*)p2, data1, data2;

   for (i = 0; (nerrors < 256) && (i < n); i++)
   {
      data1 = *pBuf1++;
      data2 = *pBuf2++;
      if (data1 != data2)
      {
         BDBG_WRN(("BHAB_45216_P_CompareMemory error: i=0x%X, expected=0x%02X, read=0x%02X", i, data1, data2));
         nerrors++;
      }
   }

   return nerrors;
#endif
}


/******************************************************************************
 BHAB_45216_VerifyMemory() - Read the IRAM
******************************************************************************/
bool BHAB_45216_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len)
{
   static uint8_t *pVerifyBuf = NULL;
   bool b;

   BDBG_MSG(("verifying memory (addr=0x%X, size=0x%X)", addr, len));

   pVerifyBuf = (uint8_t*)BKNI_Malloc(len);
   if (pVerifyBuf == NULL)
   {
      BDBG_ERR(("unable to allocate memory to verify code"));
      BDBG_ASSERT(0);
   }

   BHAB_45216_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, len);

   if (BHAB_45216_P_CompareMemory(pHexImage, pVerifyBuf, len) == 0)
      b = true;
   else
      b = false;
   BKNI_Free(pVerifyBuf);
   return b;
}
#endif


/******************************************************************************
 BERR_Code BHAB_45216_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_45216_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec)
{
   BHAB_45216_P_Handle *pImpl = (BHAB_45216_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BHAB_45216_P_EnableHostInterrupt(h, true);
   while (1)
   {
      retCode = BKNI_WaitForEvent(pImpl->hApiEvent, timeoutMsec);
      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR))
      {
         break;
      }
      else if (retCode == BERR_SUCCESS)
      {
         BHAB_45216_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BHAB_45216_P_EnableHostInterrupt(h, true);
   }

   BHAB_45216_P_EnableHostInterrupt(h, true);

   return retCode;
}


/******************************************************************************
 BHAB_45216_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_45216_P_GetInterruptEventHandle(
   BHAB_Handle handle,            /* [in] BHAB handle */
   BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
   *hEvent = ((BHAB_45216_P_Handle *)(handle->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BHAB_45216_P_ProcessInterruptEvent(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ASSERT(handle);

   BHAB_CHK_RETCODE(BHAB_45216_P_DecodeInterrupt(handle));
   BHAB_45216_P_EnableHostInterrupt(handle, true);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_45216_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable)
{
   BKNI_EnterCriticalSection();
   h->settings.interruptEnableFunc(bEnable, h->settings.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45216_P_WriteBbsi()
******************************************************************************/
BERR_Code BHAB_45216_P_WriteBbsi(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint8_t     addr, /* [in] address */
   uint8_t     *buf, /* [in] data to write */
   uint32_t    n     /* [in] number of bytes to write */
)
{
   BERR_Code retCode;
   uint32_t i;
   uint8_t spiBuf[16];

   if (h->settings.isSpi)
   {
      if (n < 16)
      {
         spiBuf[0] = BHAB_SPI_WRITE_COMMAND;
         spiBuf[1] = addr;
         for (i = 0; i < n; i++)
            spiBuf[i+2] = buf[i];

         retCode = BREG_SPI_Write(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)spiBuf, n+2);
      }
      else
      {
#if 1
         uint8_t *pBuf = (uint8_t *)BKNI_Malloc(n+2);
         if (pBuf == NULL)
         {
            retCode = BERR_OUT_OF_SYSTEM_MEMORY;
            goto done;
         }
         pBuf[0] = BHAB_SPI_WRITE_COMMAND;
         pBuf[1] = addr;
         BKNI_Memcpy(&pBuf[2], buf, n);
         retCode = BREG_SPI_Write(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)pBuf, n+2);
         BKNI_Free(pBuf);
#else
         BDBG_ERR(("BHAB_45216_P_WriteBbsi(): length (%d) too large!", n));
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
#endif
      }
   }
   else
      retCode = BREG_I2C_Write(((BHAB_45216_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_ReadBbsi()
******************************************************************************/
BERR_Code BHAB_45216_P_ReadBbsi(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint8_t     addr, /* [in] address */
    uint8_t     *buf, /* [out] buffer that holds the data */
    uint32_t     n     /* [in] number of bytes to read */
)
{
   BERR_Code retCode;
   uint32_t i;
   uint8_t spiWriteBuf[8], spiReadBuf[8];

   if (h->settings.isSpi)
   {
      if (n > 4)
      {
         BDBG_ERR(("BHAB_45216_P_ReadBbsi(): length (%d) too large!", n));
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         goto done;
      }

      spiWriteBuf[0] = BHAB_SPI_READ_COMMAND;
      spiWriteBuf[1] = addr;
      for (i = 0; i < n; i++)
         spiWriteBuf[i+2] = 0;

      retCode = BREG_SPI_Read(((BHAB_45216_P_Handle*)h->pImpl)->hSpiRegister, spiWriteBuf, spiReadBuf, n+2);
      if (retCode == BERR_SUCCESS)
      {
         for (i = 0; i < n; i++)
            buf[i] = spiReadBuf[i+2];
      }
   }
   else
      retCode = BREG_I2C_Read(((BHAB_45216_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   if (retCode)
   {
      BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
BHAB_45216_P_ReadRbus()
******************************************************************************/
BERR_Code BHAB_45216_P_ReadRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [in] data to write */
    uint32_t    n     /* [in] number of 32-bit words to read */
)
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
         BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 5));
      }

      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));

      BHAB_CHK_RETCODE(BHAB_45216_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      buf[i] = (uint32_t)((i2c_buf[0] << 24) | (i2c_buf[1] << 16) | (i2c_buf[2] << 8) | i2c_buf[3]);
   }

   done:
   return retCode;
}


/******************************************************************************
BHAB_45216_P_WriteRbus()
******************************************************************************/
BERR_Code BHAB_45216_P_WriteRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [out] buffer that holds the data */
    uint32_t    n     /* [in] number of 32-bit words to read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, data_idx;
   uint8_t i2c_buf[9];

   if (n == 0)
      retCode = BERR_INVALID_PARAMETER;

   /* wait for !busy or error */
   BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));

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

      BHAB_CHK_RETCODE(BHAB_45216_P_WriteBbsi(h, (uint8_t)(i ? BCHP_CSR_RBUS_DATA0 : BCHP_CSR_CONFIG), i2c_buf, data_idx));

      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_45216_P_WaitForBbsiDone(h));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_WaitForBbsiDone()
******************************************************************************/
BERR_Code BHAB_45216_P_WaitForBbsiDone(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t retry;
   uint8_t status;

   for (retry = 0; retry < BHAB_MAX_BBSI_RETRIES; retry++)
   {
      BHAB_CHK_RETCODE(BHAB_45216_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
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
 BHAB_45216_P_GetAvsData()
******************************************************************************/
BERR_Code BHAB_45216_P_GetAvsData(BHAB_Handle handle, BHAB_AvsData *pData)
{
   BERR_Code retCode;
   uint32_t buf[5];

   BKNI_Memset(buf, 0, 5*sizeof(uint32_t));
   buf[0] = BHAB_45216_InitHeader(0x07, 0, 0, 0);
   BHAB_CHK_RETCODE(BHAB_45216_P_SendCommand(handle, buf, 5));
   pData->enabled = (buf[1] & 1) ? true : false;
   pData->temperature = (int)(buf[2]);
   pData->voltage = buf[3];

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45216_P_DumpGpRegisters()
******************************************************************************/
void BHAB_45216_P_DumpGpRegisters(BHAB_Handle h, uint32_t startReg, uint32_t endReg)
{
   uint32_t i, reg, val;

   for (i = startReg; i <= endReg; i++)
   {
      reg = BCHP_LEAP_CTRL_GP0 + (i << 2);
      BHAB_45216_P_ReadRegister(h, reg, &val);
      BDBG_ERR(("GP%d=0x%08", i, val));
   }
}
