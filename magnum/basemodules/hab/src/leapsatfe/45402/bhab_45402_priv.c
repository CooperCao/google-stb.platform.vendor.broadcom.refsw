/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhab_45402_priv.h"
#include "bchp_45402_cpu_mem.h"
#include "bchp_45402_cpu_ctrl.h"
#include "bchp_45402_cpu_ctrl_misc.h"
#include "bchp_45402_cpu_misc.h"
#include "bchp_45402_leap_host_l2_0.h"
#include "bchp_45402_leap_host_l2_1.h"
#include "bchp_45402_csr.h"
#include "bchp_45402_cpu_hab_mem.h"
#include "bchp_45402_leap_shared_mem.h"
#include "bchp_45402_leap_host_l1.h"
#include "bchp_45402_tm.h"

#if 0 /* define BHAB_NO_IMG to disable IMG in the HAB module */
#define BHAB_NO_IMG
#endif

#ifndef BHAB_NO_IMG
#include "bhab_satfe_img.h"
#endif

BDBG_MODULE(bhab_45402_priv);

#define BHAB_LEAP_INIT_TIMEOUT 8000
#define BHAB_CMD_TIMEOUT       800

/* #define BHAB_DEBUG */
/* #define BHAB_EXT_RAM_DEBUG */

#define BHAB_MAX_BBSI_RETRIES  10

/* local private functions */
static BERR_Code BHAB_45402_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
static BERR_Code BHAB_45402_P_RunAp(BHAB_Handle h);
static BERR_Code BHAB_45402_P_ServiceHab(BHAB_Handle h, uint32_t *read_buf, uint16_t read_len);
static BERR_Code BHAB_45402_P_DisableLeapInterrupts(BHAB_Handle h);
static BERR_Code BHAB_45402_P_DecodeInterrupt(BHAB_Handle h);
static bool BHAB_45402_P_IsLeapRunning(BHAB_Handle h);
static BERR_Code BHAB_45402_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec);
static BERR_Code BHAB_45402_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
static BERR_Code BHAB_45402_P_WaitForBbsiDone(BHAB_Handle h);
static BERR_Code BHAB_45402_P_DispatchCallback(BHAB_45402_P_CallbackInfo *pCallbackInfo, uint32_t status0, uint32_t status1, uint32_t status2, uint32_t status3);
static void BHAB_45402_P_DumpError(BHAB_Handle h);
#ifdef BHAB_VERIFY_DOWNLOAD
static bool BHAB_45402_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len);
#endif


/******************************************************************************
 BHAB_45402_P_Open()
******************************************************************************/
BERR_Code BHAB_45402_P_Open(
   BHAB_Handle *handle,     /* [out] BHAB handle */
   void        *pReg,       /* [in] pointer to BREG handle */
   const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_Handle hDev;
   BHAB_45402_P_Handle *h45402Dev;

   BDBG_ASSERT(pDefSettings);
   BDBG_ASSERT(pDefSettings->interruptEnableFunc);

   hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
   BDBG_ASSERT(hDev);
   h45402Dev = (BHAB_45402_P_Handle *)BKNI_Malloc(sizeof(BHAB_45402_P_Handle));
   BDBG_ASSERT(h45402Dev);
   BKNI_Memset(h45402Dev, 0x00, sizeof(BHAB_45402_P_Handle));
   hDev->pImpl = (void*)h45402Dev;

   if (pDefSettings->isSpi)
      h45402Dev->hSpiRegister = (BREG_SPI_Handle)pReg;
   else
      h45402Dev->hI2cRegister = (BREG_I2C_Handle)pReg;

   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));

   /* create events */
   retCode = BKNI_CreateEvent(&(h45402Dev->hInterruptEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45402Dev->hApiEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45402Dev->hInitDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h45402Dev->hHabDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   *handle = hDev;
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_Close()
******************************************************************************/
BERR_Code BHAB_45402_P_Close(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_45402_P_Handle *p45402;

   BDBG_ASSERT(handle);
   p45402 = (BHAB_45402_P_Handle *)(handle->pImpl);

   /* disable host interrupts */
   retCode = BHAB_45402_P_DisableLeapInterrupts(handle);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BHAB_45402_P_DisableLeapInterrupts() error 0x%X", retCode));
   }

   /* reset the LEAP */
   retCode = BHAB_45402_P_Reset(handle);

   /* clean up */
   BKNI_DestroyEvent(p45402->hInitDoneEvent);
   BKNI_DestroyEvent(p45402->hHabDoneEvent);
   BKNI_DestroyEvent(p45402->hInterruptEvent);
   BKNI_DestroyEvent(p45402->hApiEvent);
   BKNI_Free((void*)p45402);
   BKNI_Free((void*)handle);
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_InitAp()
******************************************************************************/
BERR_Code BHAB_45402_P_InitAp(
    BHAB_Handle h,            /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM45402 microcode image */
)
{
   BHAB_45402_P_Handle *pImpl = (BHAB_45402_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t n, init_done_mask, retries, val32;
   uint32_t fwAddr = 0;
   const uint8_t *pImage;
#ifdef BHAB_NO_IMG
   const uint8_t *pDataBuf;
#else
   unsigned chunk, num_chunks, segment, num_segments;
   unsigned fw_size;
   unsigned chunk_size = MAX_SATFE_IMG_CHUNK_SIZE;
   unsigned header_size = 0;
   BIMG_Interface *pImgInterface = (BIMG_Interface*)h->settings.pImgInterface;
   void **pImgContext = h->settings.pImgContext;
   void *pImg;

   BSTD_UNUSED(pHexImage);

   if ((pImgInterface == NULL) || (pImgContext == NULL))
   {
      BDBG_ERR(("HAB IMG interface and/or context not initialized"));
      return BERR_NOT_INITIALIZED;
   }
#endif

   /* disable L1 host interrupt */
   BHAB_CHK_RETCODE(BHAB_45402_P_EnableHostInterrupt(h, false));

   /* reset the AP before downloading the firmware */
   BHAB_CHK_RETCODE(BHAB_45402_P_Reset(h));

   /* disable and clear all leap interrupts */
   BHAB_CHK_RETCODE(BHAB_45402_P_DisableLeapInterrupts(h));

   /* set the reset vector to the start of itcm */
   val32 = 0x80000;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_CPU_MISC_CORECTRL_RESET_VECTOR, &val32));

   /* enable the host L1 interrupt sources (LEAP_L2_0 and LEAP_L2_1) */
   val32 = BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR_LEAP_L2_0_INTR_MASK;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR, &val32));
   val32 = BCHP_LEAP_HOST_L1_INTR_W3_MASK_CLEAR_LEAP_L2_1_INTR_MASK;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W3_MASK_CLEAR, &val32));

   /* enable init_done interrupt */
   init_done_mask = BHAB_45402_HIRQ0_INIT_DONE;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_MASK_CLEAR0, &init_done_mask));
   BHAB_45402_P_WaitForEvent(h, pImpl->hInitDoneEvent, 0);

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
         retCode = BHAB_45402_P_WriteMemory(h, fwAddr, (uint8_t*)pImage, n);
         if (retCode == BERR_SUCCESS)
         {
#ifdef BHAB_VERIFY_DOWNLOAD
            if (BHAB_45402_VerifyMemory(h, fwAddr, pDataBuf, n))
               break;
            BDBG_ERR(("data read back does not match"));
#else
            break;
#endif
         }
         else
         {
            BDBG_ERR(("BHAB_45402_P_WriteMemory() error 0x%X", retCode));
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
   /* use IMG */
   BHAB_CHK_RETCODE(pImgInterface->open((void*)pImgContext, &pImg, 0));
   /* read header */
   BHAB_CHK_RETCODE(pImgInterface->next(pImg, 0, (const void **)&pImage, MAX_SATFE_IMG_QUERY_SIZE));

   header_size = (unsigned)pImage[0];
   num_segments = (unsigned)pImage[1];

   for (segment = 0; segment < num_segments; segment++)
   {
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
          BDBG_ERR(("BHAB_45402_P_InitAp: img next[%d of %d]...", chunk, num_chunks));
          if (chunk == num_chunks-1) {
              n = fw_size % chunk_size;
          } else
              n = chunk_size;
          BHAB_CHK_RETCODE(pImgInterface->next(pImg, chunk, (const void **)&pImage, (uint16_t)n));

         for (retries = 0; retries < 3; retries++)
         {
            BDBG_ERR(("writing %d bytes starting from 0x%04X", n, fwAddr + chunk*chunk_size));
            retCode = BHAB_45402_P_WriteMemory(h, fwAddr + chunk*chunk_size, (uint8_t*)pImage, n);
            if (retCode == BERR_SUCCESS)
            {
#ifdef BHAB_VERIFY_DOWNLOAD
               if (BHAB_45402_VerifyMemory(h, fwAddr + chunk*chunk_size, pImage, n))
                  break;
               BDBG_ERR(("data read back does not match"));
#else
               break;
#endif
            }
            else
            {
               BDBG_ERR(("BHAB_45402_P_WriteMemory() error 0x%X", retCode));
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

   /* start running the AP */
   BHAB_CHK_RETCODE(BHAB_45402_P_RunAp(h));
   BDBG_MSG(("LEAP is running"));

#ifdef BHAB_DONT_USE_INTERRUPT
   /* poll for AP init done */
   for (n = 0; n < BHAB_LEAP_INIT_TIMEOUT; n++)
   {
      BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS0, &val32);
      if (val32 & BHAB_45402_HIRQ0_INIT_DONE)
         break;
      BKNI_Sleep(1);
   }
   if (val32 & BHAB_45402_HIRQ0_INIT_DONE)
   {
      val32 = BHAB_45402_HIRQ0_INIT_DONE;
      BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR0, &val32);
   }
   else
   {
      BDBG_ERR(("AP initialization timeout"));
      BHAB_45402_P_DumpError(h);
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
   }
#else
   /* wait for init done interrupt */
   if (BHAB_45402_P_WaitForEvent(h, pImpl->hInitDoneEvent, BHAB_LEAP_INIT_TIMEOUT) != BERR_SUCCESS)
   {
      BHAB_CHK_RETCODE(BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS0, &val32));
      if ((val32 & BHAB_45402_HIRQ0_INIT_DONE) == 0)
      {
         BDBG_ERR(("AP initialization timeout"));
         BHAB_45402_P_DumpError(h);
         BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
      }
      else
      {
         BDBG_ERR(("firmware initialized but host didnt get init_done irq!"));
      }
   }
#endif

   /* disable the init_done interrupt */
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_MASK_SET0, &init_done_mask));

   done:
   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("LEAP initialized successfully"));
   }
   else
   {
      BDBG_ERR(("BHAB_45402_P_InitAp() error 0x%X", retCode));
   }
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_WriteRegister()
******************************************************************************/
BERR_Code BHAB_45402_P_WriteRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode;
   bool bHsi = false;
   uint8_t sb;

   if (reg <= 0x7e)
      bHsi = true;
   else if ((reg < 0x140000) || (reg > 0x780003f) || (reg & 0x3))
   {
      BDBG_ERR(("BHAB_45402_P_WriteRegister(): bad address (0x%X)", reg));
      return BERR_INVALID_PARAMETER;
   }

   if (bHsi)
   {
      sb = (uint8_t)(*val & 0xFF);
      retCode = BHAB_45402_P_WriteBbsi(h, (uint8_t)reg, &sb, 1);
   }
   else
      retCode = BHAB_45402_P_WriteRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_45402_P_ReadRegister()
******************************************************************************/
BERR_Code BHAB_45402_P_ReadRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode;
   bool bHsi = false;
   uint8_t sb;

   if (reg < 0x7e)
      bHsi = true;
   else if ((reg < 0x140000) || (reg > 0x780003f) || (reg & 0x3))
   {
      BDBG_ERR(("BHAB_45402_P_ReadRegister(): bad address (0x%X)", reg));
      return BERR_INVALID_PARAMETER;
   }

   if (bHsi)
   {
      retCode = BHAB_45402_P_ReadBbsi(h, (uint8_t)reg, &sb, 1);
      if (retCode == BERR_SUCCESS)
         *val = (uint32_t)sb;
   }
   else
      retCode = BHAB_45402_P_ReadRbus(h, reg, val, 1);

   return retCode;
}


/******************************************************************************
 BHAB_45402_P_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_45402_P_GetVersionInfo(BHAB_Handle handle, BFEC_SystemVersionInfo *p)
{
   BERR_Code retCode;
   uint32_t buf[11];

   BKNI_Memset(buf, 0, 11*sizeof(uint32_t));
   buf[0] = BHAB_45402_InitHeader(0x04, 0, 0, 0);
   BHAB_CHK_RETCODE(BHAB_45402_P_SendCommand(handle, buf, 11));
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
 BHAB_45402_P_GetApStatus()
******************************************************************************/
BERR_Code BHAB_45402_P_GetApStatus(
   BHAB_Handle h,          /* [in] HAB device handle */
   BHAB_ApStatus *pStatus  /* [out] AP status */
)
{
   uint32_t val32, hab_request;

   *pStatus = 0;

   if (BHAB_45402_P_IsLeapRunning(h))
   {
      /* LEAP is running */
      BHAB_45402_P_ReadRegister(h, BCHP_CPU_CTRL_GP5, &val32);
      if (val32 & 0x80000000)
      {
         /* AP has initialized */
         *pStatus |= BHAB_APSTATUS_INIT_DONE;

         BHAB_45402_P_ReadRegister(h, BCHP_CPU_CTRL_MISC_HAB_REQ_STAT, &hab_request);
         if ((hab_request & 1) == 0)
            *pStatus |= BHAB_APSTATUS_HAB_READY;
      }
   }
   else
      *pStatus |= BHAB_APSTATUS_RESET;

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45402_P_WriteMemory()
******************************************************************************/
BERR_Code BHAB_45402_P_WriteMemory(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t addr = start_addr, nWords, leftover;
   bool bIsRunning;
   uint8_t i2c_buf[6];
   BREG_SPI_Data spiData[2];

   /* only allow access to ITCM, DTCM, HAB, or SHARED_MEM */
   if ((n == 0) || (start_addr & 0x3) ||
       (start_addr < BCHP_CPU_MEM_ITCM_WORDi_ARRAY_BASE) ||
       ((start_addr >= (BCHP_CPU_HAB_MEM_WORDi_ARRAY_BASE + (BCHP_CPU_HAB_MEM_WORDi_ARRAY_END<<2))) && (start_addr < BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_BASE)) ||
       (start_addr > (BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_BASE+(BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_END<<2))))
      return BERR_INVALID_PARAMETER;

   bIsRunning = BHAB_45402_P_IsLeapRunning(h);
   if (bIsRunning && (addr < BCHP_CPU_MEM_DTCM_WORDi_ARRAY_BASE))
      return BHAB_ERR_MEMAV; /* cannot write to ITCM while running */

   leftover = n % 4;
   nWords = (n >> 2);

   /* wait for !busy or error */
   BHAB_CHK_RETCODE(BHAB_45402_P_WaitForBbsiDone(h));

   i2c_buf[0] = 0x00; /* BCHP_CSR_CONFIG: 32-bit writes */
   i2c_buf[1] = 0; /* bits [39:32] of rbus address */
   i2c_buf[2] = (uint8_t)((start_addr >> 24) & 0xFF);
   i2c_buf[3] = (uint8_t)((start_addr >> 16) & 0xFF);
   i2c_buf[4] = (uint8_t)((start_addr >> 8) & 0xFF);
   i2c_buf[5] = (uint8_t)(start_addr & 0xFF);
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 6));

   if (h->settings.isSpi == false)
   {
      /* for i2c, we can write entire buf in one transaction */
      BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, (uint8_t *)buf, nWords<<2));
   }
   else
   {
      i2c_buf[0] = BHAB_45402_SPI_WRITE_COMMAND;
      i2c_buf[1] = BCHP_CSR_RBUS_DATA0;

      spiData[0].data = (void *)i2c_buf;
      spiData[0].length = 2;
      spiData[1].data = (void *)buf;
      spiData[1].length = nWords<<2;
      BHAB_CHK_RETCODE(BREG_SPI_Multiple_Write(((BHAB_45402_P_Handle*)h->pImpl)->hSpiRegister, spiData, 2));
   }

   /* wait for !busy or error */
   BHAB_CHK_RETCODE(BHAB_45402_P_WaitForBbsiDone(h));

   if (leftover)
   {
      /* pad with zeros */
      i2c_buf[0] = buf[(nWords<<2)];
      i2c_buf[1] = (leftover > 1) ? buf[(nWords<<2)+1] : 0;
      i2c_buf[2] = (leftover > 2) ? buf[(nWords<<2)+2] : 0;
      i2c_buf[3] = 0;
      BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_ReadMemory()
******************************************************************************/
BERR_Code BHAB_45402_P_ReadMemory(BHAB_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, j, buf_idx, addr, end_addr, nReads, x, val32;
   bool bCsrAddrSet = false;
   uint8_t i2c_buf[6];

   /* only allow access to ITCM, DTCM, HAB, or SHARED_MEM */
   if ((n == 0) || (start_addr < BCHP_CPU_MEM_ITCM_WORDi_ARRAY_BASE) ||
       ((start_addr >= (BCHP_CPU_HAB_MEM_WORDi_ARRAY_BASE + (BCHP_CPU_HAB_MEM_WORDi_ARRAY_END<<2))) && (start_addr < BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_BASE)) ||
       (start_addr > (BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_BASE+(BCHP_LEAP_SHARED_MEM_SHARED_WORDi_ARRAY_END<<2))))
      return BERR_INVALID_PARAMETER;

   addr = start_addr & ~0x03; /* actual starting address is 32-bit aligned */
   end_addr = start_addr + n;
   if (end_addr & 0x03)
      end_addr = (end_addr & ~0x03) + 4;
   nReads = (end_addr - addr) >> 2; /* total number of 32-bit reads */

   /* wait for !busy or error */
   BHAB_CHK_RETCODE(BHAB_45402_P_WaitForBbsiDone(h));

   for (i = buf_idx = 0; i < nReads; i++)
   {
      if (i == 0)
      {
         x = start_addr & 0x03;
         if (x)
         {
            /* starting address is not 32-bit aligned */
            BHAB_CHK_RETCODE(BHAB_45402_P_ReadRbus(h, addr, &val32, 1));
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
         i2c_buf[0] = (i == (nReads-1)) ? 0x05 : 0x03; /* BCHP_CSR_CONFIG: 32-bit reads */
         i2c_buf[1] = 0; /* bits [39:32] of rbus address */
         i2c_buf[2] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[4] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[5] = (uint8_t)(addr & 0xFF);
         BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 6));
         bCsrAddrSet = true;
      }

      BHAB_CHK_RETCODE(BHAB_45402_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      for (j = 0; (j < 4) && (buf_idx < n); j++)
         buf[buf_idx++] = i2c_buf[j];
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_45402_P_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
   BHAB_45402_P_Handle *p45402;
   BHAB_45402_P_CallbackInfo *pCbInfo;

   BDBG_ASSERT(handle);
   p45402 = (BHAB_45402_P_Handle *)(handle->pImpl);

   switch (eDevId)
   {
      case BHAB_DevId_eSAT:
         pCbInfo = &(p45402->cbSat);
         break;

      case BHAB_DevId_eDSQ:
         pCbInfo = &(p45402->cbDsq);
         break;

      case BHAB_DevId_eFSK:
         pCbInfo = &(p45402->cbFsk);
         break;

      case BHAB_DevId_eWFE:
         pCbInfo = &(p45402->cbWfe);
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
 BHAB_45402_P_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_45402_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
   BHAB_45402_P_Handle *p45402;
   BHAB_45402_P_CallbackInfo *pCbInfo;

   BDBG_ASSERT(handle);
   p45402 = (BHAB_45402_P_Handle *)(handle->pImpl);

   switch (eDevId)
   {
      case BHAB_DevId_eSAT:
         pCbInfo = &(p45402->cbSat);
         break;

      case BHAB_DevId_eDSQ:
         pCbInfo = &(p45402->cbDsq);
         break;

      case BHAB_DevId_eFSK:
         pCbInfo = &(p45402->cbFsk);
         break;

      case BHAB_DevId_eWFE:
         pCbInfo = &(p45402->cbWfe);
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
 BHAB_45402_P_SendCommand()
******************************************************************************/
BERR_Code BHAB_45402_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_45402_P_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}


/******************************************************************************
 BHAB_45402_P_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_45402_P_SendHabCommand(
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
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_ApStatus status;
   uint32_t *pWriteBuf = (uint32_t*)write_buf;
   uint32_t *pReadBuf = (uint32_t*)read_buf;
   uint32_t checksum, val;
   uint16_t i;

   BSTD_UNUSED(command_len);
   BSTD_UNUSED(bCheckForAck);
   BSTD_UNUSED(bFixedLength);

   if ((write_len >= 256) || (read_len >= 256) || (write_len < 2) || (read_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BHAB_CHK_RETCODE(BHAB_45402_P_GetApStatus(handle, &status));
   if (status & BHAB_APSTATUS_RESET)
      return BERR_TRACE(BHAB_ERR_AP_FAIL);
   if ((status & BHAB_APSTATUS_INIT_DONE) == 0)
      return BERR_TRACE(BHAB_ERR_AP_NOT_INIT);
   if ((status & BHAB_APSTATUS_HAB_READY) == 0)
      return BERR_TRACE(BHAB_ERR_HAB_RESOURCE_BUSY);

   /* compute the checksum */
   checksum = 0;
   for (i = 0; i < (write_len-1); i++)
      checksum += pWriteBuf[i];
   pWriteBuf[write_len-1] = checksum;

#ifdef BHAB_DEBUG
   BDBG_WRN(("send HAB command:"));
   for (i = 0; i < write_len; i++)
   {
      BDBG_WRN(("   HAB[%d]=0x%08X", i, pWriteBuf[i]));
   }
#endif

   /* write command to the hab */
   BHAB_45402_P_WriteRbus(handle, BCHP_CPU_HAB_MEM_WORDi_ARRAY_BASE, pWriteBuf, write_len);

   /* wait for the AP to service the HAB, and then read any return data */
   BHAB_CHK_RETCODE(BHAB_45402_P_ServiceHab(handle, pReadBuf, read_len));

#ifdef BHAB_DEBUG
   BDBG_WRN(("rcvd HAB response:"));
   for (i = 0; i < read_len; i++)
   {
      BDBG_WRN(("   HAB[%d]=0x%08X", i, pReadBuf[i]));
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
   else if ((pReadBuf[0] & BHAB_45402_ACK) == 0)
   {
      BDBG_ERR(("No HAB ack"));
      BHAB_45402_P_DumpError(handle);
      retCode = BHAB_45402_P_ReadRegister(handle, BCHP_CPU_CTRL_GP2, &val);
      if (retCode == BERR_SUCCESS)
      {
         if (val == 0)
            retCode = BHAB_ERR_HAB_NO_ACK;
         else
            retCode = val;
      }
   }

   done:
   return retCode;
}


/****************************Private Functions*********************************/


/******************************************************************************
 BHAB_45402_P_Reset()
******************************************************************************/
BERR_Code BHAB_45402_P_Reset(BHAB_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val32;

   /* reset the maestro */
   BHAB_CHK_RETCODE(BHAB_45402_P_ReadRegister(h, BCHP_CPU_MISC_CORECTRL_CORE_ENABLE, &val32));
   if (val32 & 1)
   {
      val32 = 0;
      BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_CPU_MISC_CORECTRL_CORE_ENABLE, &val32));
   }

   /* reset the cores */
   val32 = 0xFFFF;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_TM_SFT_RST0, &val32));
   val32 = 0x00030331;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_TM_SFT_RST1, &val32));
   val32 = 0;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_TM_SFT_RST0, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_TM_SFT_RST1, &val32));

   if (BHAB_45402_P_IsLeapRunning(h))
      return BERR_TRACE(BHAB_ERR_AP_FAIL);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_RunAp()
******************************************************************************/
static BERR_Code BHAB_45402_P_RunAp(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t val32;

   val32 = 1;
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_CPU_MISC_CORECTRL_CORE_ENABLE, &val32));

   if (BHAB_45402_P_IsLeapRunning(h))
      retCode = BERR_SUCCESS;
   else
   {
      retCode = BHAB_ERR_AP_FAIL;
      BERR_TRACE(retCode);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_ServiceHab()
******************************************************************************/
static BERR_Code BHAB_45402_P_ServiceHab(
   BHAB_Handle h,      /* [in] BHAB handle */
   uint32_t *read_buf,  /* [out] holds the data read from the HAB */
   uint16_t read_len   /* [in] number of words to read from the HAB (including the checksum) */
)
{
   BHAB_45402_P_Handle *pImpl = (BHAB_45402_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val32;
#ifdef BHAB_DONT_USE_INTERRUPT
   uint32_t i;
#endif

   /* clear the HAB_DONE interrupt */
   mask = BHAB_45402_HIRQ0_HAB_DONE;
   BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR0, &mask);
   BHAB_45402_P_WaitForEvent(h, pImpl->hHabDoneEvent, 0);

#ifndef BHAB_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_MASK_CLEAR0, &mask);
#endif

   /* send the command */
   val32 =  BCHP_CPU_CTRL_MISC_HAB_REQ_SET_HAB_REQ_SET_MASK;
   BHAB_45402_P_WriteRegister(h, BCHP_CPU_CTRL_MISC_HAB_REQ_SET, &val32);

#if 0 /* explicitly setting HAB_REQ_STAT L2 irq may not be needed */
   /* assert the HAB interrupt on the LEAP */
   val32 = BCHP_CPU_L2_CPU_SET_HAB_REQ_STAT_MASK;
   BHAB_45402_P_WriteRegister(h, BCHP_LEAP_L2_CPU_SET, &val32);
#endif

#ifdef BHAB_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < BHAB_CMD_TIMEOUT; i++)
   {
      val32 = 0;
      BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS0, &val32);
      if (val32 & BHAB_45402_HIRQ0_HAB_DONE)
         break;
      BKNI_Sleep(1);
   }
   if ((val32 & BHAB_45402_HIRQ0_HAB_DONE) == 0)
      retCode = BERR_TIMEOUT;
   else
   {
      /* clear the HAB_DONE interrupt status */
      BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR0, &mask);
   }
#else
   /* wait for HAB done interrupt */
   retCode = BHAB_45402_P_WaitForEvent(h, pImpl->hHabDoneEvent, BHAB_CMD_TIMEOUT);
#endif

   if (retCode != BERR_SUCCESS)
   {
      /* check if HAB is done */
      BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS0, &val32);
      if ((val32 & BHAB_45402_HIRQ0_HAB_DONE) == 0)
      {
         BDBG_ERR(("HAB timeout"));
         BHAB_45402_P_DumpError(h);
         retCode = BHAB_45402_P_ReadRegister(h, BCHP_CPU_CTRL_GP2, &val32);
         if (retCode == BERR_SUCCESS)
         {
            if (val32 == 0)
            {
               BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
            }
            else
               retCode = val32;
         }
         goto done;
      }
      else
      {
         BDBG_WRN(("missed hab done irq"));
      }
   }

   /* get the HAB contents */
   retCode = BHAB_45402_P_ReadRbus(h, BCHP_CPU_HAB_MEM_WORDi_ARRAY_BASE, read_buf, read_len);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_45402_P_HandleInterrupt_isr(
    BHAB_Handle h /* [in] BHAB handle */
)
{
   BHAB_45402_P_Handle *pImpl;

   BDBG_ASSERT(h);
   pImpl = (BHAB_45402_P_Handle *)(h->pImpl);
   h->settings.interruptEnableFunc(false, h->settings.interruptEnableFuncParam);
   BKNI_SetEvent(pImpl->hApiEvent);
   BKNI_SetEvent(pImpl->hInterruptEvent);
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45402_P_DispatchCallback()
******************************************************************************/
static BERR_Code BHAB_45402_P_DispatchCallback(BHAB_45402_P_CallbackInfo *pCallbackInfo, uint32_t status0, uint32_t status1, uint32_t status2, uint32_t status3)
{
   BERR_Code retCode = BERR_SUCCESS;

   pCallbackInfo->callbackParams.status0 = status0;
   pCallbackInfo->callbackParams.status1 = status1;
   pCallbackInfo->callbackParams.status2 = status2;
   pCallbackInfo->callbackParams.status3 = status3;

   if (pCallbackInfo->func)
   {
      retCode = pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
   }
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_DecodeInterrupt()
******************************************************************************/
static BERR_Code BHAB_45402_P_DecodeInterrupt(BHAB_Handle h)
{
   BHAB_45402_P_Handle *pImpl = (BHAB_45402_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, status1, status2, mask0, mask1, mask2, fstatus0, fstatus1, fstatus2, fstatus3;
   uint32_t mask_set0 = 0;

   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS0, &status0);
   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_MASK_STATUS0, &mask0);
   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_STATUS1, &status1);
   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_0_MASK_STATUS1, &mask1);
   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_1_STATUS0, &status2);
   BHAB_45402_P_ReadRegister(h, BCHP_LEAP_HOST_L2_1_MASK_STATUS0, &mask2);

   fstatus0 = (status0 & ~mask0);
   fstatus1 = (status1 & ~mask1);
   fstatus2 = (status2 & ~mask2);
   fstatus3 = 0; /* currently not used */
   if ((fstatus0 == 0) && (fstatus1 == 0) && (fstatus2 == 0))
      goto done;

#if 0 /* for debug only */
   BDBG_ERR(("status0,mask0,fstatus0=%08X,%08X,%08X", status0, mask0, fstatus0));
   BDBG_ERR(("status1,mask1,fstatus1=%08X,%08X,%08X", status1, mask1, fstatus1));
   BDBG_ERR(("status2,mask2,fstatus2=%08X,%08X,%08X", status2, mask2, fstatus2));
#endif

   if (fstatus0)
   {
      if (fstatus0 & BHAB_45402_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BHAB_45402_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BHAB_45402_HIRQ0_HAB_DONE)
      {
         BKNI_SetEvent(pImpl->hHabDoneEvent);
         mask_set0 |= BHAB_45402_HIRQ0_HAB_DONE;
      }

      if (fstatus0 & BHAB_45402_HIRQ0_FSK_MASK)
      {
         retCode = BHAB_45402_P_DispatchCallback(&(pImpl->cbFsk), fstatus0, fstatus1, fstatus2, fstatus3);
      }

      if (fstatus0 & BHAB_45402_HIRQ0_DSQ_MASK)
      {
         if (fstatus0 & BHAB_45402_HIRQ0_DSQ0_TX)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ0_TX;
         if (fstatus0 & BHAB_45402_HIRQ0_DSQ0_RX)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ0_RX;
         if (fstatus0 & BHAB_45402_HIRQ0_DSQ0_VSENSE)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ0_VSENSE;

         if (fstatus0 & BHAB_45402_HIRQ0_DSQ1_TX)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ1_TX;
         if (fstatus0 & BHAB_45402_HIRQ0_DSQ1_RX)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ1_RX;
         if (fstatus0 & BHAB_45402_HIRQ0_DSQ1_VSENSE)
            mask_set0 |= BHAB_45402_HIRQ0_DSQ1_VSENSE;

         retCode = BHAB_45402_P_DispatchCallback(&(pImpl->cbDsq), fstatus0, fstatus1, fstatus2, fstatus3);
      }

      if (fstatus0 & BHAB_45402_HIRQ0_WFE_MASK)
      {
         if (fstatus0 & BHAB_45402_HIRQ0_WFE0_READY)
            mask_set0 |= BHAB_45402_HIRQ0_WFE0_READY;

         if (fstatus0 & BHAB_45402_HIRQ0_WFE1_READY)
            mask_set0 |= BHAB_45402_HIRQ0_WFE1_READY;

         retCode = BHAB_45402_P_DispatchCallback(&(pImpl->cbWfe), fstatus0, fstatus1, fstatus2, fstatus3);
      }

      if (fstatus0 & BHAB_45402_HIRQ0_SAT_MASK)
      {
         if (fstatus0 & BHAB_45402_HIRQ0_SAT_INIT_DONE)
            mask_set0 |= BHAB_45402_HIRQ0_SAT_INIT_DONE;

         if (fstatus0 & BHAB_45402_HIRQ0_SA_DONE)
            mask_set0 |= BHAB_45402_HIRQ0_SA_DONE;
      }
   }

   if (fstatus1 || fstatus2 || (fstatus0 & BHAB_45402_HIRQ0_SAT_MASK))
   {
      retCode = BHAB_45402_P_DispatchCallback(&(pImpl->cbSat), fstatus0, fstatus1, fstatus2, fstatus3);
   }


   /* clear the interrupt status */
   if (fstatus0)
      BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR0, &fstatus0);
   if (fstatus1)
      BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR1, &fstatus1);
   if (fstatus2)
      BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_1_CLEAR0, &fstatus2);

   if (mask_set0)
   {
      /* disable specified interrupts */
      BHAB_45402_P_WriteRegister(h,  BCHP_LEAP_HOST_L2_0_MASK_SET0, &mask_set0);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_IsLeapRunning()
******************************************************************************/
static bool BHAB_45402_P_IsLeapRunning(
   BHAB_Handle h  /* [in] BHAB handle */
)
{
   uint32_t val;
   bool bRunning = false;

   if (BHAB_45402_P_ReadRegister(h,  BCHP_CPU_MISC_CORECTRL_CORE_ENABLE, &val) == BERR_SUCCESS)
   {
      if (val & 1)
      {
         if (BHAB_45402_P_ReadRegister(h,  BCHP_CPU_MISC_CORECTRL_CORE_IDLE, &val) == BERR_SUCCESS)
         {
            if ((val & 1) == 0)
               bRunning = true;
         }
      }
   }

   return bRunning;
}


/******************************************************************************
 BHAB_45402_P_DisableLeapInterrupts()
******************************************************************************/
static BERR_Code BHAB_45402_P_DisableLeapInterrupts(
   BHAB_Handle h   /* [in] BHAB handle */
)
{
   uint32_t val32 = 0xFFFFFFFF;
   BERR_Code retCode;

   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W0_MASK_SET, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W1_MASK_SET, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W2_MASK_SET, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W3_MASK_SET, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_MASK_SET0, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_MASK_SET1, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR0, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_0_CLEAR1, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_1_MASK_SET0, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_1_MASK_SET1, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_1_CLEAR0, &val32));
   BHAB_CHK_RETCODE(BHAB_45402_P_WriteRegister(h, BCHP_LEAP_HOST_L2_1_CLEAR1, &val32));

   done:
   return retCode;
}


#ifdef BHAB_VERIFY_DOWNLOAD
/******************************************************************************
 BHAB_45402_P_CompareMemory()
******************************************************************************/
int BHAB_45402_P_CompareMemory(const void *p1, const void *p2, size_t n)
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
         BDBG_WRN(("BHAB_45402_P_CompareMemory error: i=0x%X, expected=0x%02X, read=0x%02X", i, data1, data2));
         nerrors++;
      }
   }

   return nerrors;
#endif
}


/******************************************************************************
 BHAB_45402_VerifyMemory() - Read the IRAM
******************************************************************************/
static bool BHAB_45402_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len)
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

   BHAB_45402_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, len);

   if (BHAB_45402_P_CompareMemory(pHexImage, pVerifyBuf, len) == 0)
      b = true;
   else
      b = false;
   BKNI_Free(pVerifyBuf);
   return b;
}
#endif


/******************************************************************************
 BERR_Code BHAB_45402_P_WaitForEvent()
******************************************************************************/
static BERR_Code BHAB_45402_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec)
{
   BHAB_45402_P_Handle *pImpl = (BHAB_45402_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BHAB_45402_P_EnableHostInterrupt(h, true);
   while (1)
   {
      retCode = BKNI_WaitForEvent(pImpl->hApiEvent, timeoutMsec);
      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR))
         break;
      else if (retCode == BERR_SUCCESS)
      {
         BHAB_45402_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BHAB_45402_P_EnableHostInterrupt(h, true);
   }

   BHAB_45402_P_EnableHostInterrupt(h, true);

   return retCode;
}


/******************************************************************************
 BHAB_45402_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_45402_P_GetInterruptEventHandle(
   BHAB_Handle handle,            /* [in] BHAB handle */
   BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
   *hEvent = ((BHAB_45402_P_Handle *)(handle->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45402_P_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BHAB_45402_P_ProcessInterruptEvent(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ASSERT(handle);

   BHAB_CHK_RETCODE(BHAB_45402_P_DecodeInterrupt(handle));
   BHAB_45402_P_EnableHostInterrupt(handle, true);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_EnableHostInterrupt()
******************************************************************************/
static BERR_Code BHAB_45402_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable)
{
   BKNI_EnterCriticalSection();
   h->settings.interruptEnableFunc(bEnable, h->settings.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_45402_P_WriteBbsi()
******************************************************************************/
BERR_Code BHAB_45402_P_WriteBbsi(
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
      if (n < 14)
      {
         /* perform a short spi write without malloc */
         spiBuf[0] = BHAB_45402_SPI_WRITE_COMMAND;
         spiBuf[1] = addr;
         for (i = 0; i < n; i++)
            spiBuf[i+2] = buf[i];

         retCode = BREG_SPI_Write(((BHAB_45402_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)spiBuf, n+2);
      }
      else
      {
         /* new spi write supports larger transactions */
         uint8_t *pBuf = (uint8_t *)BKNI_Malloc(n+2);
         if (pBuf == NULL)
         {
            retCode = BERR_OUT_OF_SYSTEM_MEMORY;
            goto done;
         }
         pBuf[0] = BHAB_45402_SPI_WRITE_COMMAND;
         pBuf[1] = addr;
         BKNI_Memcpy(&pBuf[2], buf, n);
         retCode = BREG_SPI_Write(((BHAB_45402_P_Handle*)h->pImpl)->hSpiRegister, (const uint8_t*)pBuf, n+2);
         BKNI_Free(pBuf);
      }
   }
   else
      retCode = BREG_I2C_Write(((BHAB_45402_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_ReadBbsi()
******************************************************************************/
BERR_Code BHAB_45402_P_ReadBbsi(
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
         BDBG_ERR(("BHAB_45402_P_ReadBbsi(): length (%d) too large!", n));
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         goto done;
      }

      spiWriteBuf[0] = BHAB_45402_SPI_READ_COMMAND;
      spiWriteBuf[1] = addr;
      for (i = 0; i < n; i++)
         spiWriteBuf[i+2] = 0;

      retCode = BREG_SPI_Read(((BHAB_45402_P_Handle*)h->pImpl)->hSpiRegister, spiWriteBuf, spiReadBuf, n+2);
      if (retCode == BERR_SUCCESS)
      {
         for (i = 0; i < n; i++)
            buf[i] = spiReadBuf[i+2];
      }
   }
   else
      retCode = BREG_I2C_Read(((BHAB_45402_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);

   done:
   if (retCode)
   {
      BERR_TRACE(retCode);
   }
   return retCode;
}


/******************************************************************************
BHAB_45402_P_ReadRbus()
******************************************************************************/
BERR_Code BHAB_45402_P_ReadRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [in] data to write */
    uint32_t    n     /* [in] number of 32-bit words to read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   uint8_t i2c_buf[6];

   if ((n == 0) || (addr & 0x3) || (buf == NULL))
      return BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      /* set up the starting address to read */
      if (i == 0)
      {
         i2c_buf[0] = (n > 1) ? 0x03 : 0x05; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = 0; /* bits[39:32] of rbus address */
         i2c_buf[2] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[4] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[5] = (uint8_t)(addr & 0xFF);
         BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, BCHP_CSR_CONFIG, i2c_buf, 6));
      }

      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_45402_P_WaitForBbsiDone(h));

      BHAB_CHK_RETCODE(BHAB_45402_P_ReadBbsi(h, BCHP_CSR_RBUS_DATA0, i2c_buf, 4));
      buf[i] = (uint32_t)((i2c_buf[0] << 24) | (i2c_buf[1] << 16) | (i2c_buf[2] << 8) | i2c_buf[3]);
   }

   done:
   return retCode;
}


/******************************************************************************
BHAB_45402_P_WriteRbus()
******************************************************************************/
BERR_Code BHAB_45402_P_WriteRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [out] buffer that holds the data */
    uint32_t    n     /* [in] number of 32-bit words to read */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i, data_idx;
   uint8_t i2c_buf[10];

   if ((n == 0) || (addr & 0x3) || (buf == NULL))
      return BERR_INVALID_PARAMETER;

   for (i = 0; i < n; i++)
   {
      /* wait for !busy or error */
      BHAB_CHK_RETCODE(BHAB_45402_P_WaitForBbsiDone(h));

      if (i == 0)
      {
         i2c_buf[0] = 0; /* BCHP_CSR_CONFIG */
         i2c_buf[1] = 0; /* bits[39:32] of rbus address */
         i2c_buf[2] = (uint8_t)((addr >> 24) & 0xFF);
         i2c_buf[3] = (uint8_t)((addr >> 16) & 0xFF);
         i2c_buf[4] = (uint8_t)((addr >> 8) & 0xFF);
         i2c_buf[5] = (uint8_t)(addr & 0xFF);
         data_idx = 6;
      }
      else
         data_idx = 0;

      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 24) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 16) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)((buf[i] >> 8) & 0xFF);
      i2c_buf[data_idx++] = (uint8_t)(buf[i] & 0xFF);

      BHAB_CHK_RETCODE(BHAB_45402_P_WriteBbsi(h, (uint8_t)(i ? BCHP_CSR_RBUS_DATA0 : BCHP_CSR_CONFIG), i2c_buf, data_idx));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_WaitForBbsiDone()
******************************************************************************/
static BERR_Code BHAB_45402_P_WaitForBbsiDone(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t retry;
   uint8_t status;

   for (retry = 0; retry < BHAB_MAX_BBSI_RETRIES; retry++)
   {
      BHAB_CHK_RETCODE(BHAB_45402_P_ReadBbsi(h, BCHP_CSR_STATUS, &status, 1));
      if (((status & BCHP_CSR_STATUS_BUSY_MASK) == 0) || (status & 0x0F))
         break;
   }

   if (status & 0x0F)
   {
      BDBG_ERR(("Error: CSR_STATUS=0x%X", status));
      retCode = BHAB_ERR_HOST_XFER;
   }
   else if ((retry >= BHAB_MAX_BBSI_RETRIES) || (status & BCHP_CSR_STATUS_BUSY_MASK))
      retCode = BERR_TIMEOUT;

   done:
   return retCode;
}


/******************************************************************************
 BHAB_45402_P_DumpError()
******************************************************************************/
static void BHAB_45402_P_DumpError(BHAB_Handle h)
{
   uint32_t i, j, reg, val1, val2, val3;

   for (i = 0; i < 7; i++)
   {
      j = i * 3;
      reg = BCHP_CPU_CTRL_GP0 + (j << 2);
      BHAB_45402_P_ReadRegister(h, reg, &val1);
      BHAB_45402_P_ReadRegister(h, reg+4, &val2);
      BHAB_45402_P_ReadRegister(h, reg+8, &val3);
      BDBG_ERR(("GP%02d=0x%08X   GP%02d=0x%08X   GP%02d=0x%08X",
                j, val1, j+1, val2, j+2, val3));
   }
}
