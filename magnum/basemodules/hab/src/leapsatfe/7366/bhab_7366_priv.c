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
*
* Module Description:
*
*****************************************************************************/
#include "bhab_7366_priv.h"
#include "bchp_leap_ctrl.h"
#include "bchp_leap_hab_mem.h"
#include "bchp_leap_host_l1.h"
#include "bchp_leap_host_l2.h"
#include "bchp_leap_l2.h"
#include "bchp_leap_rom_patch.h"
#if BHAB_CHIP==4548
#include "bchp_4548_sun_top_ctrl.h"
#include "bchp_4548_sun_gisb_arb.h"
#include "bchp_4548_gio.h"
#endif

#include "bmem.h"

#if 0 /* define BHAB_NO_IMG to disable IMG in the HAB module */
#define BHAB_NO_IMG
#endif

#ifndef BHAB_NO_IMG
#include "bhab_satfe_img.h"
#endif

BDBG_MODULE(bhab_7366_priv);
/* #define BHAB_DEBUG */
/* #define BHAB_JTAG_DEBUG */
/* #define BHAB_EXT_RAM_DEBUG */

#if BHAB_CHIP==4548
#define CORE_CSR_SELECT_MASK   0x80
#define CORE_SELECT            0x80
#define CSR_SELECT             0x00

#define EXTN_ADDR_REG0_MASK    0x40
#define EXTN_ADDR_REG0         0x40

#define EXTN_ADDR_MASK         0x80
#define EXTN_ADDR              0x80

#define EXTN_ADDR_STOP         0x00

#define CORE_ADDR_5_0(x)       ( x & 0x3F )
#define CORE_ADDR_12_6(x)      ((x >> 6 ) & 0x7F )
#define CORE_ADDR_19_13(x)     ((x >> 13) & 0x7F )
#define CORE_ADDR_26_20(x)     ((x >> 20) & 0x7F )
#define CORE_ADDR_33_27(x)     ((x >> 27) & 0x7F )

#define BHAB_MAX_BBSI_RETRIES  10

/* Specifies the max number of status bytes before data is ready */
#define BHAB_MAX_BBSI_STATUS_BYTES 2

#define I2C_READ_STATUS_READY      0x01
#define I2C_READ_STATUS_ERROR      0x02

#define BCHP_CSR_REVISION          0x00
#define BCHP_CSR_CHIP_ID_ADR       0x01
#define BCHP_CSR_STATUS            0x02
#define BCHP_CSR_CONFIG_0          0x03
#define BCHP_CSR_RBUS_ADDR0        0x05

#define CSR_CONFIG0_SNGL_ADDR      0x08
#define CSR_CONFIG0_ACCESS_32BIT   0x40
#define CSR_CONFIG0_LITTLE_ENDIAN  0x80
#define CSR_CONFIG0_SINGL_XFR      0x10

#define CSR_STATUS_TXFR_PENDING    0x80
#define CSR_STATUS_TXFR_ERR        0x08
#define CSR_STATUS_TXFR_TIMEOUT    0x04
#define CSR_STATUS_TXFR_W_OVERUN   0x02
#endif


/* local private functions */
BERR_Code BHAB_7366_P_RunAp(BHAB_Handle h);
BERR_Code BHAB_7366_P_ServiceHab(BHAB_Handle h, uint32_t *read_buf, uint16_t read_len);
BERR_Code BHAB_7366_P_CheckHab(BHAB_Handle h);
BERR_Code BHAB_7366_P_DecodeHabError(BHAB_Handle h);
BERR_Code BHAB_7366_P_DisableLeapInterrupts(BHAB_Handle h);
bool BHAB_7366_P_IsLeapRunning(BHAB_Handle h);
BERR_Code BHAB_7366_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec);
BERR_Code BHAB_7366_P_DecodeInterrupt_isr(BHAB_Handle h);
#if BHAB_CHIP==4548
BERR_Code BHAB_4548_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable);
BERR_Code BHAB_4548_P_UnlockSerialInterface(BHAB_Handle h);
BERR_Code BHAB_4548_P_WriteBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_4548_P_ReadBbsi(BHAB_Handle h, uint8_t addr, uint8_t *buf, uint32_t n);
BERR_Code BHAB_4548_P_ReadRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_4548_P_WriteRbus(BHAB_Handle h, uint32_t addr, uint32_t *buf, uint32_t n);
BERR_Code BHAB_4548_P_Reset(BHAB_Handle h);
BERR_Code BHAB_4548_P_ClearHirq(BHAB_Handle h);
BERR_Code BHAB_4548_P_DownloadFirmware(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n);
BERR_Code BHAB_4548_P_DecodeInterrupt(BHAB_Handle h);
#endif
#ifdef BHAB_VERIFY_DOWNLOAD
bool BHAB_7366_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len);
#endif


/******************************************************************************
 BHAB_7366_P_Open()
******************************************************************************/
BERR_Code BHAB_7366_P_Open(
   BHAB_Handle *handle,     /* [out] BHAB handle */
   void        *pReg,       /* [in] pointer to BREG handle */
   const BHAB_Settings *pDefSettings /* [in] Default Settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_Handle hDev;
   BHAB_7366_P_Handle *h7366Dev;

   hDev = (BHAB_Handle)BKNI_Malloc(sizeof(BHAB_P_Handle));
   BDBG_ASSERT(hDev);
   h7366Dev = (BHAB_7366_P_Handle *)BKNI_Malloc(sizeof(BHAB_7366_P_Handle));
   BDBG_ASSERT(h7366Dev);
   BKNI_Memset( h7366Dev, 0x00, sizeof(BHAB_7366_P_Handle));
   hDev->pImpl = (void*)h7366Dev;

#if BHAB_CHIP==4548
   if (pDefSettings->isSpi)
   {
      h7366Dev->hSpiRegister = (BREG_SPI_Handle)pReg;
      h7366Dev->hI2cRegister = NULL;
   }
   else
   {
      h7366Dev->hI2cRegister = (BREG_I2C_Handle)pReg;
      h7366Dev->hSpiRegister = NULL;
   }
#else
   h7366Dev->hReg = (BREG_Handle)pReg;
#endif
   BKNI_Memset((void*)&(h7366Dev->cbSat), 0, sizeof(BHAB_7366_P_CallbackInfo));
   BKNI_Memset((void*)&(h7366Dev->cbDsq), 0, sizeof(BHAB_7366_P_CallbackInfo));
   BKNI_Memset((void*)&(h7366Dev->cbFsk), 0, sizeof(BHAB_7366_P_CallbackInfo));
   BKNI_Memset((void*)&(h7366Dev->cbWfe), 0, sizeof(BHAB_7366_P_CallbackInfo));
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BHAB_Settings));

   h7366Dev->settings.bUseInternalMemory = true;

   /* create events */
#if BHAB_CHIP==4548
   retCode = BKNI_CreateEvent(&(h7366Dev->hInterruptEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h7366Dev->hApiEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
#endif
   retCode = BKNI_CreateEvent(&(h7366Dev->hInitDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   retCode = BKNI_CreateEvent(&(h7366Dev->hHabDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);

#if BHAB_CHIP==4548
   retCode = BHAB_4548_P_UnlockSerialInterface(hDev);
   BDBG_ASSERT(retCode == BERR_SUCCESS);
#endif

   *handle = hDev;
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_Close()
******************************************************************************/
BERR_Code BHAB_7366_P_Close(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_7366_P_Handle *p7366 = (BHAB_7366_P_Handle *)(handle->pImpl);

   BDBG_ASSERT(handle);

   /* disable host interrupts */
   retCode = BHAB_7366_P_DisableLeapInterrupts(handle);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BHAB_7366_P_DisableLeapInterrupts() error 0x%X", retCode));
   }

   /* reset the LEAP */
   retCode = BHAB_7366_P_Reset(handle);

   /* clean up */
   BKNI_DestroyEvent(p7366->hInitDoneEvent);
   BKNI_DestroyEvent(p7366->hHabDoneEvent);
#if BHAB_CHIP==4548
   BKNI_DestroyEvent(p7366->hInterruptEvent);
   BKNI_DestroyEvent(p7366->hApiEvent);
#endif
   BKNI_Free((void*)p7366);
   BKNI_Free((void*)handle);
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_InitAp()
******************************************************************************/
BERR_Code BHAB_7366_P_InitAp(
    BHAB_Handle h,            /* [in] BHAB handle */
    const uint8_t *pHexImage  /* [in] pointer to BCM7366 microcode image */
)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   BFEC_SystemVersionInfo versionInfo;
   uint32_t n, init_done_mask, retries, baseAddr, val32;
   uint32_t fwSize = 0, fwAddr = 0;
   const uint8_t *pImage, *pDataBuf = NULL;
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
   BSTD_UNUSED(fwSize);

   if ((pImgInterface == NULL) || (pImgContext == NULL))
   {
      BDBG_ERR(("HAB IMG interface and/or context not initialized"));
      return BERR_NOT_INITIALIZED;
   }
#endif

   if (pImpl->settings.bUseInternalMemory)
      baseAddr = (uint32_t)0x10000000; /*-0xF0000000*/
   else
      baseAddr = (uint32_t)(pImpl->settings.pRam); /* base address of the LEAP RAM in DDR */

#if BHAB_CHIP==4548
   BHAB_CHK_RETCODE(BHAB_4548_P_Reset(h));
#endif

   /* disable interrupts */
   BHAB_CHK_RETCODE(BHAB_7366_P_DisableLeapInterrupts(h));
#if BHAB_CHIP==4548
   BHAB_CHK_RETCODE(BHAB_4548_P_EnableHostInterrupt(h, false));
#endif

   /* reset the AP before downloading the firmware */
   BHAB_CHK_RETCODE(BHAB_7366_P_Reset(h));

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
      fwAddr += baseAddr;

      pDataBuf = pImage;
      fwSize = n;

      for (retries = 0; retries < 3; retries++)
      {
         BDBG_MSG(("writing %d bytes starting from 0x%04X", fwSize, fwAddr));
#if BHAB_CHIP==4548
         retCode = BHAB_4548_P_DownloadFirmware(h, fwAddr, (uint8_t*)pImage, fwSize);
#else
         retCode = BHAB_7366_P_WriteMemory(h, fwAddr, (uint8_t*)pImage, fwSize);
#endif
         if (retCode == BERR_SUCCESS)
         {
#ifdef BHAB_VERIFY_DOWNLOAD
            if (BHAB_7366_VerifyMemory(h, fwAddr, pDataBuf, fwSize))
               break;

            BDBG_ERR(("data read back does not match\n"));
#else
            break;
#endif
         }
         else
         {
            BDBG_ERR(("BHAB_7366_P_WriteMemory() error 0x%X", retCode));
         }
      }
      if (retries >= 3)
      {
         BDBG_ERR(("unable to initialize RAM"));
         BERR_TRACE(retCode = BHAB_ERR_HOST_XFER);
         goto done;
      }

      pImage += fwSize;
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
       fwAddr += baseAddr;

       fw_size = n;
       num_chunks = fw_size / chunk_size;
       if (fw_size % chunk_size)
           num_chunks++;

       for (chunk = 0; chunk < num_chunks; chunk++)
       {
           BDBG_MSG(("BHAB_7366_P_InitAp: img next[%d of %d]...", chunk, num_chunks));
           if (chunk == num_chunks-1) {
               n = fw_size % chunk_size;
           } else
               n = chunk_size;
           BDBG_MSG(("requesting %d bytes", n));
           BHAB_CHK_RETCODE(pImgInterface->next(pImg, chunk, (const void **)&pImage, n));

          for (retries = 0; retries < 3; retries++)
          {
             BDBG_MSG(("writing %d bytes starting from 0x%04X", n, fwAddr + chunk*chunk_size));
             retCode = BHAB_7366_P_WriteMemory(h, fwAddr + chunk*chunk_size, (uint8_t*)pImage, n);
             if (retCode == BERR_SUCCESS)
             {
#ifdef BHAB_VERIFY_DOWNLOAD
                if (BHAB_7366_VerifyMemory(h, fwAddr + chunk*chunk_size, pImage, n))
                   break;
                BDBG_ERR(("data read back does not match\n"));
#else
                break;
#endif
             }
             else
             {
                BDBG_ERR(("BHAB_7366_P_WriteMemory() error 0x%X", retCode));
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

   /* clear init_done interrupt status */
   init_done_mask = BHAB_7366_HIRQ0_INIT_DONE;
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &init_done_mask);

#ifndef BHAB_7366_7366_DONT_USE_INTERRUPT
   /* enable init done interrupt */
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &init_done_mask);
   BHAB_7366_P_WaitForEvent(h, pImpl->hInitDoneEvent, 0);
#endif

   /* start running the AP */
   BHAB_CHK_RETCODE(BHAB_7366_P_RunAp(h));
   BDBG_MSG(("LEAP is running"));

#ifdef BHAB_7366_7366_DONT_USE_INTERRUPT
   /* poll for AP init done */
   for (n = 0; n < 1000; n++)
   {
      BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if (val32 & BHAB_7366_HIRQ0_INIT_DONE)
         break;
      BKNI_Sleep(1);
   }
   if ((val32 & BHAB_7366_HIRQ0_INIT_DONE) == 0)
   {
      BDBG_ERR(("AP initialization timeout\n"));
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
      goto done;
   }
#else
   /* wait for init done interrupt */
   if (BHAB_7366_P_WaitForEvent(h, pImpl->hInitDoneEvent, 5000) != BERR_SUCCESS)
   {
      BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if (val32 & BHAB_7366_HIRQ0_INIT_DONE)
      {
         BDBG_ERR(("firmware initialized but host didnt get init_done irq!"));
         goto got_init_done;
      }
      BDBG_ERR(("AP initialization timeout\n"));
      BERR_TRACE(retCode = BHAB_ERR_AP_NOT_INIT);
      goto done;
   }
   got_init_done:
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_SET0, &init_done_mask);
   BDBG_MSG(("LEAP firmware initialized"));
#endif

   /* display version info */
   if (BHAB_7366_P_GetVersionInfo(h, &versionInfo) == BERR_SUCCESS)
   {
      BDBG_WRN(("LEAP firmware version %d.%d.%d", versionInfo.firmware.majorVersion, versionInfo.firmware.minorVersion, versionInfo.firmware.buildId));
   }

   done:
#ifdef BHAB_EXT_RAM_DEBUG /* for debug only */
   BKNI_Sleep(5000);

   /* stop the LEAP */
   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &n);
   n |= BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK;
   n &= ~BCHP_LEAP_CTRL_CTRL_START_ARC_MASK;
   BHAB_WriteRegister(h, BCHP_LEAP_CTRL_CTRL, &n);

   if (BHAB_7366_VerifyMemory(h, fwAddr, pDataBuf, fwSize) == false)
   {
      BDBG_ERR(("LEAP Instruction Memory is corrupted."));
   }
   else
   {
      BDBG_MSG(("LEAP Instruction Memory is OK."));
   }
   while (1);
#endif

   if (retCode == BERR_SUCCESS)
   {
      BDBG_MSG(("LEAP initialized successfully"));
   }
   else
   {
      BDBG_ERR(("BHAB_7366_P_InitAp() error 0x%X", retCode));
   }
   return retCode;
}


#if BHAB_CHIP!=4548
/******************************************************************************
 BHAB_7366_P_WriteRegister_isrsafe()
******************************************************************************/
BERR_Code BHAB_7366_P_WriteRegister_isrsafe(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BHAB_7366_P_Handle *p = (BHAB_7366_P_Handle*)h->pImpl;
   BREG_Write32(p->hReg, reg & ~0xF0000000, *val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_ReadRegister_isrsafe()
******************************************************************************/
BERR_Code BHAB_7366_P_ReadRegister_isrsafe(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BHAB_7366_P_Handle *p = (BHAB_7366_P_Handle*)h->pImpl;
   *val = BREG_Read32(p->hReg, reg & ~0xF0000000);
   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BHAB_7366_P_GetVersionInfo()
******************************************************************************/
BERR_Code BHAB_7366_P_GetVersionInfo(BHAB_Handle handle, BFEC_SystemVersionInfo *p)
{
   BERR_Code retCode;
   uint32_t buf[11];

   BKNI_Memset(buf, 0, 11*sizeof(uint32_t));
   buf[0] = BHAB_7366_InitHeader(0x04, 0, 0, 0);
   BHAB_CHK_RETCODE(BHAB_7366_P_SendCommand(handle, buf, 11));
   p->familyId = buf[1];
#if BHAB_CHIP==4548
   p->chipId = 0x4548;
#else
   p->chipId = buf[2];
#endif
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
 BHAB_7366_P_GetApStatus()
******************************************************************************/
BERR_Code BHAB_7366_P_GetApStatus(
   BHAB_Handle h,          /* [in] HAB device handle */
   BHAB_ApStatus *pStatus  /* [out] AP status */
)
{
   uint32_t spare0, hab_request, val;

   *pStatus = 0;

   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &val);
   if ((val & (BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK | BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK)) == 0)
   {
      /* LEAP is running */
      BHAB_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0);
      if (spare0 & BHAB_7366_SPARE0_INIT_DONE)
      {
         /* AP has initialized */
         *pStatus |= BHAB_APSTATUS_INIT_DONE;

         BHAB_ReadRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request);
         if ((hab_request & 1) == 0)
            *pStatus |= BHAB_APSTATUS_HAB_READY;
      }
   }
   else
      *pStatus |= BHAB_APSTATUS_RESET;

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_WriteMemory()
******************************************************************************/
BERR_Code BHAB_7366_P_WriteMemory(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   uint32_t addr, i, data;
   bool bIsRunning;

   BSTD_UNUSED(h);

   /* make sure memory addresses are 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   /* make sure length is multiple of 4 bytes */
   if (n & 0x3)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   bIsRunning = BHAB_7366_P_IsLeapRunning(h);

   for (addr = start_addr, i = 0; i < n; i += 4)
   {
      data = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
      buf += 4;

      if (pImpl->settings.bUseInternalMemory)
      {
         if (bIsRunning)
         {
            /* for now, don't allow memory write if leap is running */
            return BERR_NOT_SUPPORTED;
         }
         else
         {
            BHAB_WriteRegister(h, addr, &data);
         }
      }
      else
      {
         *((volatile uint32_t *)addr) = data;
      }
      addr += 4;
   }

   if (!pImpl->settings.bUseInternalMemory) {
       BMEM_Heap_FlushCache(pImpl->settings.heap,(void *)start_addr,n);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_ReadMemory()
******************************************************************************/
BERR_Code BHAB_7366_P_ReadMemory(BHAB_Handle h, uint32_t start_addr, uint8_t *buf, uint32_t n)
{
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   uint32_t addr, end_addr, i, j, data, nReadsLeft, x, hab[127];
   uint8_t *pData;
   bool bIsRunning;

   /* make sure memory addresses are 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   /* make sure length is multiple of 4 bytes */
   if (n & 0x3)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   bIsRunning = BHAB_7366_P_IsLeapRunning(h);

   if (!pImpl->settings.bUseInternalMemory) {
       BMEM_Heap_FlushCache(pImpl->settings.heap,(void *)start_addr,n);
   }

   /* calculate end_addr and nReadsLeft */
   end_addr = start_addr + n;
   if (end_addr & 0x03)
      end_addr = (end_addr & ~0x03) + 4;
   nReadsLeft = (end_addr - start_addr) >> 2; /* total number of 32-bit reads */

   pData = (uint8_t *)buf;
   for (addr = start_addr, i = 0; addr < end_addr;)
   {
      if (pImpl->settings.bUseInternalMemory)
      {
         if (bIsRunning)
         {
            /* send HAB command to read memory */
            hab_read:
            if (nReadsLeft > 123)
               x = 123;
            else
               x = nReadsLeft;

            hab[0] = BHAB_7366_InitHeader(0x01, 0, BHAB_7366_READ, 0);
            hab[1] = addr;
            hab[2] = x;
            BHAB_CHK_RETCODE(BHAB_7366_P_SendCommand(h, hab, x+4));

            for (j = 0; j < x; j++)
            {
               *pData++ = (uint8_t)(hab[3+j] & 0xFF);
               *pData++ = (uint8_t)(hab[3+j] >> 8);
               *pData++ = (uint8_t)(hab[3+j] >> 16);
               *pData++ = (uint8_t)(hab[3+j] >> 24);
            }
            nReadsLeft -= x;
            addr += 4*x;
            i += x;
         }
         else
         {
            BHAB_ReadRegister(h, addr, &data);
            *pData++ = (data & 0xFF);
            *pData++ = ((data >> 8) & 0xFF);
            *pData++ = ((data >> 16) & 0xFF);
            *pData++ = ((data >> 24) & 0xFF);
            nReadsLeft--;
            addr += 4;
         }
      }
      else
      {
         if (bIsRunning && (addr < 0x4007E000))
            goto hab_read;
         else
         {
            *pData++ = (uint8_t)(*((volatile uint32_t *)addr) & 0xFF);
            *pData++ = (uint8_t)(*((volatile uint32_t *)addr) >> 8);
            *pData++ = (uint8_t)(*((volatile uint32_t *)addr) >> 16);
            *pData++ = (uint8_t)(*((volatile uint32_t *)addr) >> 24);
            nReadsLeft--;
            addr += 4;
         }
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_InstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_7366_P_InstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId,    /* [in] Device ID */
    BHAB_IntCallbackFunc fCallBack,
    void * pParm1,
    int parm2
)
{
   BHAB_7366_P_Handle *p7366;

   BDBG_ASSERT(handle);
   p7366 = (BHAB_7366_P_Handle *)(handle->pImpl);

   if (eDevId == BHAB_DevId_eSAT)
   {
      p7366->cbSat.func = fCallBack;
      p7366->cbSat.callbackParams.pParm1 = pParm1;
      p7366->cbSat.callbackParams.parm2 = parm2;
   }
   else if (eDevId == BHAB_DevId_eDSQ)
   {
      p7366->cbDsq.func = fCallBack;
      p7366->cbDsq.callbackParams.pParm1 = pParm1;
      p7366->cbDsq.callbackParams.parm2 = parm2;
   }
   else if (eDevId == BHAB_DevId_eFSK)
   {
      p7366->cbFsk.func = fCallBack;
      p7366->cbFsk.callbackParams.pParm1 = pParm1;
      p7366->cbFsk.callbackParams.parm2 = parm2;
   }
   else if (eDevId == BHAB_DevId_eWFE)
   {
      p7366->cbWfe.func = fCallBack;
      p7366->cbWfe.callbackParams.pParm1 = pParm1;
      p7366->cbWfe.callbackParams.parm2 = parm2;
   }
   else
   {
      BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_UnInstallInterruptCallback()
******************************************************************************/
BERR_Code BHAB_7366_P_UnInstallInterruptCallback(
    BHAB_Handle handle,  /* [in] BHAB handle */
    BHAB_DevId eDevId    /* [in] Device ID */
)
{
   BHAB_7366_P_Handle *p7366 = (BHAB_7366_P_Handle *)(handle->pImpl);

   BDBG_ASSERT(handle);
   p7366 = (BHAB_7366_P_Handle *)(handle->pImpl);

   if (eDevId == BHAB_DevId_eSAT)
   {
      p7366->cbSat.func = NULL;
      p7366->cbSat.callbackParams.pParm1 = NULL;
      p7366->cbSat.callbackParams.parm2 = 0;
   }
   else if (eDevId == BHAB_DevId_eDSQ)
   {
      p7366->cbDsq.func = NULL;
      p7366->cbDsq.callbackParams.pParm1 = NULL;
      p7366->cbDsq.callbackParams.parm2 = 0;
   }
   else if (eDevId == BHAB_DevId_eFSK)
   {
      p7366->cbFsk.func = NULL;
      p7366->cbFsk.callbackParams.pParm1 = NULL;
      p7366->cbFsk.callbackParams.parm2 = 0;
   }
   else if (eDevId == BHAB_DevId_eWFE)
   {
      p7366->cbWfe.func = NULL;
      p7366->cbWfe.callbackParams.pParm1 = NULL;
      p7366->cbWfe.callbackParams.parm2 = 0;
   }
   else
   {
      BDBG_ERR(("invalid BHAB_DevId (%u)", eDevId));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_7366_P_SendCommand()
******************************************************************************/
BERR_Code BHAB_7366_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n)
{
   return BHAB_7366_P_SendHabCommand(h, (uint8_t*)pBuf, (uint16_t)n, (uint8_t*)pBuf, (uint16_t)n, true, true, 0);
}


/******************************************************************************
 BHAB_7366_P_SendHabCommand()
******************************************************************************/
BERR_Code BHAB_7366_P_SendHabCommand(
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
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(handle->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t *pWriteBuf = (uint32_t*)write_buf;
   uint32_t *pReadBuf = (uint32_t*)read_buf;
   uint32_t checksum;
   bool bOK = true;
   uint16_t i;

   BSTD_UNUSED(command_len);
   BSTD_UNUSED(bCheckForAck);
   BSTD_UNUSED(bFixedLength);

   if ((write_len >= 128) || (read_len >= 128) || (write_len < 2))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BHAB_CHK_RETCODE(BHAB_7366_P_CheckHab(handle));

   /* compute the checksum */
   checksum = 0;
   for (i = 0; i < (write_len-1); i++)
      checksum += pWriteBuf[i];
   pWriteBuf[write_len-1] = checksum;

   /* write the command bytes to the HAB */
   for (i = 0; i < write_len; i++)
   {
#ifdef BHAB_DEBUG
      if (i == 0)
      {
         BDBG_WRN(("send HAB command:"));
      }
      BDBG_WRN(("   HAB[%d]=0x%08X", i, pWriteBuf[i]));
#endif
      BHAB_CHK_RETCODE(BHAB_WriteRegister(handle, pImpl->habBaseAddr + (i<<2), &pWriteBuf[i]));
   }

   if (bOK)
   {
      /* wait for the AP to service the HAB, and then read any return data */
      BHAB_CHK_RETCODE(BHAB_7366_P_ServiceHab(handle, pReadBuf, read_len));
   }
   else
   {
      BDBG_ERR(("Unable to write to HAB"));
      retCode = BHAB_ERR_HAB_ERR;
   }

#ifdef BHAB_DEBUG
   BDBG_WRN(("rcvd HAB response:"));
   for (i = 0; i < read_len; i++)
   {
      BDBG_WRN(("   HAB[%d]=0x%08X", i, pReadBuf[i]));
   }
#endif

   /* get back the array of 32-bit words */
   checksum = 0;
   for (i = 0; i < read_len; i++)
   {
      if (i < (read_len-1))
         checksum += pReadBuf[i];
   }

   if (checksum != pReadBuf[read_len-1])
   {
      BDBG_ERR(("bad checksum: cmd=0x%08X, read=0x%X, expected=0x%X", pReadBuf[0], pReadBuf[read_len-1], checksum));
      BERR_TRACE(retCode = BHAB_ERR_HAB_CHECKSUM);
   }
   else if ((pReadBuf[0] & BHAB_7366_ACK) == 0)
   {
      retCode = BHAB_7366_P_DecodeHabError(handle);
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
 BHAB_7366_P_Reset()
******************************************************************************/
BERR_Code BHAB_7366_P_Reset(BHAB_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t ctrl, val32;
#if BHAB_CHIP==4548
   uint32_t val;
#endif

   /* save addr_trans reg */
   BHAB_CHK_RETCODE(BHAB_ReadRegister(h, BCHP_LEAP_CTRL_ADDR_TRANS, &val32));

   /* reset LEAP/CPU */
   BHAB_CHK_RETCODE(BHAB_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl));
   ctrl |= (BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK | BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK | BCHP_LEAP_CTRL_CTRL_UART_RST_MASK);
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_START_ARC_MASK; /* clear start_arc */
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl));

   /* restore addr_trans reg */
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_ADDR_TRANS, &val32));

   /* initialize some LEAP registers */
   val32 = 0x101;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_HAB_CNTR_CLR, &val32));

   val32 = 0;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE0, &val32));
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE1, &val32));
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE2, &val32));
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &val32));
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_CTRL_HAB_CNTR_CLR, &val32));

#if BHAB_CHIP==4548
   /* set up pin mux for LEAP UART */
   /* LEAP uart is on GPIO_00/GPIO_01 (i.e. GPIO_117(RX)/GPIO_118(TX) on 7366) */
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, &val));
   val &= ~BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_MASK;
   val |= (BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_LEAP << BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0_port_0_cpu_sel_SHIFT);
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_UART_ROUTER_SEL_0, &val));

   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, &val));
   val &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_onoff_gpio_117_MASK;
   val |= (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_onoff_gpio_117_TP_IN_07 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16_onoff_gpio_117_SHIFT);
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_16, &val));

   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, &val));
   val &= ~BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_onoff_gpio_118_MASK;
   val |= (BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_onoff_gpio_118_ALT_TP_OUT_07 << BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17_onoff_gpio_118_SHIFT);
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_PIN_MUX_CTRL_17, &val));

   /* configure test port for leap uart */
   BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, &val);
   val &= ~BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_MASK;
   val |= BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL_encoded_tp_enable_SYS;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_TEST_PORT_CTRL, &val));
#endif /* BHAB_CHIP==4548 */

   /* set up the L1 interrupt for the LEAP */
   val32 = BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR_LEAP_INTR_MASK;
   BHAB_CHK_RETCODE(BHAB_WriteRegister(h, BCHP_LEAP_HOST_L1_INTR_W0_MASK_CLEAR, &val32));

   if (BHAB_7366_P_IsLeapRunning(h))
      return BERR_TRACE(BHAB_ERR_AP_FAIL);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_RunAp()
******************************************************************************/
BERR_Code BHAB_7366_P_RunAp(BHAB_Handle h)
{
   uint32_t ctrl;

#if (BCHP_VER==BCHP_VER_A0)
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   uint32_t val32;

   /* 7366-A0 workaround */
   if (pImpl->settings.bUseInternalMemory)
   {
      val32 = 0x00000004;
      BHAB_WriteRegister(h, BCHP_LEAP_ROM_PATCH_VALUE0, &val32);
      val32 = 0x0000021C | BCHP_LEAP_ROM_PATCH_CONFIG0_ENABLE_MASK;
      BHAB_WriteRegister(h, BCHP_LEAP_ROM_PATCH_CONFIG0, &val32);
   }
#endif

   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl);
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK;
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK;
   ctrl &= ~BCHP_LEAP_CTRL_CTRL_UART_RST_MASK;
#ifndef BHAB_JTAG_DEBUG
   ctrl |= BCHP_LEAP_CTRL_CTRL_START_ARC_MASK;
#endif
   BHAB_WriteRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl);

#ifdef BHAB_JTAG_DEBUG
   BDBG_MSG(("connect to JTAG..."));
   while(1);
#endif

   if (BHAB_7366_P_IsLeapRunning(h))
      return BERR_SUCCESS;
   else
      return BERR_TRACE(BHAB_ERR_AP_FAIL);
}


/******************************************************************************
 BHAB_7366_P_ServiceHab()
******************************************************************************/
BERR_Code BHAB_7366_P_ServiceHab(
   BHAB_Handle h,      /* [in] BHAB handle */
   uint32_t *read_buf,  /* [out] holds the data read from the HAB */
   uint16_t read_len   /* [in] number of words to read from the HAB (including the checksum) */
)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t mask, val32, i, addr;

   /* clear the HAB_DONE interrupt */
   mask = BHAB_7366_HIRQ0_HAB_DONE;
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &mask);

#ifndef BHAB_7366_DONT_USE_INTERRUPT
   /* enable HAB_DONE interrupt */
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_CLEAR0, &mask);
#endif

   BKNI_ResetEvent(pImpl->hHabDoneEvent);

   /* send the command */
   val32 = BCHP_LEAP_CTRL_HAB_REQUEST_HAB_REQUEST_MASK;
   BHAB_WriteRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &val32);

   /* assert the HAB interrupt on the LEAP */
#ifdef BCHP_LEAP_L2_CPU_SET_HAB_INTR_MASK
   val32 = BCHP_LEAP_L2_CPU_SET_HAB_INTR_MASK;
#else
   val32 = BCHP_LEAP_L2_CPU_SET_HAB_READY_MASK;
#endif
   BHAB_WriteRegister(h, BCHP_LEAP_L2_CPU_SET, &val32);

#ifdef BHAB_7366_DONT_USE_INTERRUPT
   /* wait for HAB to be serviced (polling) */
   for (i = 0; i < 800; i++)
   {
      BKNI_Sleep(1);
      BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if (val32 & BHAB_7366_HIRQ0_HAB_DONE)
         break;
   }
   if ((val32 & BHAB_7366_HIRQ0_HAB_DONE) == 0)
      retCode = BERR_TIMEOUT;
   else
   {
      /* clear the HAB_DONE interrupt status */
      BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &mask);
   }
#else
   /* wait for HAB done interrupt */
   retCode = BHAB_7366_P_WaitForEvent(h, pImpl->hHabDoneEvent, 800);
#endif

   if (retCode == BERR_TIMEOUT)
   {
      /* check if HAB is done */
      BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &val32);
      if ((val32 & BHAB_7366_HIRQ0_HAB_DONE) == 0)
      {
         uint32_t spare3, status0, mask0;
         BHAB_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
         BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
         BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS0, &mask0);
         BDBG_ERR(("HAB timeout: spare3=0x%08X, status0=0x%08X, mask0=0x%08X", spare3, status0, mask0));

         retCode = BHAB_7366_P_DecodeHabError(h);
         if (retCode == BERR_SUCCESS)
         {
            /* couldn't find a cause for the timeout */
            BERR_TRACE(retCode = BHAB_ERR_HAB_TIMEOUT);
         }
         goto done;
      }
      else
      {
         BDBG_WRN(("missed hab done irq"));
         retCode = BERR_SUCCESS;
      }
   }

   /* get the HAB contents */
   for (i = 0; (retCode == BERR_SUCCESS) && (i < read_len); i++)
   {
      addr = pImpl->habBaseAddr + (i<<2);
      retCode = BHAB_ReadRegister(h, addr, &read_buf[i]);
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_HandleInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_7366_P_HandleInterrupt_isr(
    BHAB_Handle h /* [in] BHAB handle */
)
{
#if (BHAB_CHIP==4548)
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
#endif
   BDBG_ASSERT(h);

#if (BHAB_CHIP==4548)
   h->settings.interruptEnableFunc(false, h->settings.interruptEnableFuncParam);
   BKNI_SetEvent(pImpl->hApiEvent);
   BKNI_SetEvent(pImpl->hInterruptEvent);
   return BERR_SUCCESS;
#else
   return BHAB_7366_P_DecodeInterrupt_isr(h);
#endif
}


#if BHAB_CHIP!=4548
/******************************************************************************
 BHAB_7366_P_DecodeInterrupt_isr()
******************************************************************************/
BERR_Code BHAB_7366_P_DecodeInterrupt_isr(BHAB_Handle h)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_7366_P_CallbackInfo *pCallbackInfo;
   uint32_t status0, status1, mask0, mask1, fstatus0, fstatus1, channel, val32;
   uint32_t mask_set0 = 0, mask_set1 = 0;

   BHAB_7366_P_ReadRegister_isrsafe(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
   BHAB_7366_P_ReadRegister_isrsafe(h, BCHP_LEAP_HOST_L2_STATUS1, &status1);
   BHAB_7366_P_ReadRegister_isrsafe(h, BCHP_LEAP_HOST_L2_MASK_STATUS0, &mask0);
   BHAB_7366_P_ReadRegister_isrsafe(h, BCHP_LEAP_HOST_L2_MASK_STATUS1, &mask1);

   fstatus0 = (status0 & ~mask0);
   fstatus1 = (status1 & ~mask1);
   if ((fstatus0 == 0) && (fstatus1 == 0))
      goto done;

#if 0
   BDBG_ERR(("status0,mask0,fstatus0=%08X,%08X,%08X", status0, mask0, fstatus0));
   BDBG_ERR(("status1,mask1,fstatus1=%08X,%08X,%08X", status1, mask1, fstatus1));
#endif

   if (fstatus0)
   {
      if (fstatus0 & BHAB_7366_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BHAB_7366_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BHAB_7366_HIRQ0_HAB_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_HAB_DONE;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE0_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE0_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE1_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE1_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE2_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE2_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE3_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE3_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE_SA_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_WFE_SA_DONE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_RX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_RX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_RX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_RX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_FLASH_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_FLASH_DONE;

      if (fstatus0 & BHAB_7366_HIRQ0_SAT_INIT_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_SAT_INIT_DONE;
   }

   if (fstatus1)
   {
      for (channel = 0; channel < 8; channel++)
      {
         val32 = (fstatus1 >> (channel*4)) & 0x0F;
         if (val32 & BHAB_7366_HIRQ1_CHAN0_READY)
         {
            mask_set1 |= (BHAB_7366_HIRQ1_CHAN0_READY << (channel*4));
         }
      }
   }

   if (fstatus1 || (fstatus0 & BHAB_7366_HIRQ0_SAT_MASK))
   {
      pCallbackInfo = &(pImpl->cbSat);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call sat pi callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
      else
      {
         BDBG_ERR(("BHAB: SAT PI interrupt callback not installed!"));
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_DSQ_MASK)
   {
      pCallbackInfo = &(pImpl->cbDsq);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call dsq callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_FSK_MASK)
   {
      pCallbackInfo = &(pImpl->cbFsk);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call dsq callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_WFE_MASK)
   {
      pCallbackInfo = &(pImpl->cbWfe);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call wfe callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   /* clear the interrupt status */
   BHAB_7366_P_WriteRegister_isrsafe(h, BCHP_LEAP_HOST_L2_CLEAR0, &fstatus0);
   BHAB_7366_P_WriteRegister_isrsafe(h, BCHP_LEAP_HOST_L2_CLEAR1, &fstatus1);

   if (mask_set0)
   {
      /* disable specified interrupts */
      BHAB_7366_P_WriteRegister_isrsafe(h,  BCHP_LEAP_HOST_L2_MASK_SET0, &mask_set0);

      if (mask_set0 & BHAB_7366_HIRQ0_HAB_DONE)
         BKNI_SetEvent(pImpl->hHabDoneEvent);
   }
   if (mask_set1)
   {
      /* disable specified interrupts */
      BHAB_7366_P_WriteRegister_isrsafe(h,  BCHP_LEAP_HOST_L2_MASK_SET1, &mask_set1);
   }

   done:
   return retCode;
}
#endif


/******************************************************************************
 BHAB_7366_P_IsLeapRunning()
******************************************************************************/
bool BHAB_7366_P_IsLeapRunning(
   BHAB_Handle h  /* [in] BHAB handle */
)
{
   uint32_t ctrl;

   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_CTRL, &ctrl);
   ctrl &= (BCHP_LEAP_CTRL_CTRL_CPU_RST_MASK | BCHP_LEAP_CTRL_CTRL_LEAP_RST_MASK | BCHP_LEAP_CTRL_CTRL_START_ARC_MASK);
   if (ctrl == BCHP_LEAP_CTRL_CTRL_START_ARC_MASK)
      return true;
   else
      return false;
}


/******************************************************************************
 BHAB_7366_P_CheckHab()
******************************************************************************/
BERR_Code BHAB_7366_P_CheckHab(
   BHAB_Handle h    /* [in] BHAB handle */
)
{
   BERR_Code retCode = BHAB_ERR_HAB_NOT_AVAIL;
   uint32_t spare0, hab_request;

   if (BHAB_7366_P_IsLeapRunning(h))
   {
      /* LEAP is running */
      BHAB_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE0, &spare0);
      if (spare0 & BHAB_7366_SPARE0_INIT_DONE)
      {
         /* LEAP has initialized */
         BHAB_ReadRegister(h, BCHP_LEAP_CTRL_HAB_REQUEST, &hab_request);
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
 BHAB_7366_P_DecodeHabError() - called when the command was not ack'd
******************************************************************************/
BERR_Code BHAB_7366_P_DecodeHabError(BHAB_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t status0, spare2, spare3, leap_status, clear0 = 0, hab_error, fw_error;

   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE2, &spare2);
   BHAB_ReadRegister(h, BCHP_LEAP_CTRL_SW_SPARE3, &spare3);
   BHAB_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
   BHAB_ReadRegister(h, BCHP_LEAP_L2_CPU_STATUS, &leap_status);

   BDBG_ERR(("status0=0x%08X, spare2=0x%08X, spare3=0x%08X, leap_l2_status=0x%08X", status0, spare2, spare3, leap_status));

   if (status0 & BHAB_7366_HIRQ0_HAB_ERROR)
   {
      clear0 |= BHAB_7366_HIRQ0_HAB_ERROR;

      /* decode HAB error code */
      hab_error = (spare3 & BHAB_7366_SPARE3_HAB_ERROR_CODE_MASK) >> BHAB_7366_SPARE3_HAB_ERROR_CODE_SHIFT;
      switch (hab_error)
      {
         case 0: /* HAB_SUCCESS */
            retCode = BERR_SUCCESS;
            break;
         case 1: /* invalid HAB command ID */
            retCode = BHAB_ERR_INVALID_OPCODE;
            break;
         case 2: /* checksum error */
            retCode = BHAB_ERR_HAB_CHECKSUM;
            break;
         case 4: /* HAB_ERROR_PARAMETER */
            retCode = BHAB_ERR_HAB_BAD_PARAM;
            break;
         case 3:  /* command not implemented */
         case 5: /* command not supported */
            retCode = BHAB_ERR_HAB_NOT_SUPP;
            break;
         case 6: /* HAB_ERROR_UNABLE_TO_SVC */
            if (spare2 != 0)
               retCode = spare2;
            else
               retCode = BHAB_ERR_CMD_PROCESSING_ERR;
            break;
         case 7:
            retCode = BHAB_ERR_HAB_RESOURCE_BUSY;
            break;
         default:
            if (spare2 != 0)
               retCode = spare2;
            else
               retCode = BHAB_ERR_HAB_FAIL;
            break;
      }
   }
   else if (status0 & BHAB_7366_HIRQ0_FW_ERROR)
   {
      clear0 |= BHAB_7366_HIRQ0_FW_ERROR;

      /* decode FW error code */
      fw_error = (spare3 & BHAB_7366_SPARE3_FW_ERROR_CODE_MASK) >> BHAB_7366_SPARE3_FW_ERROR_CODE_SHIFT;
      BDBG_ERR(("LEAP FW error 0x%X", fw_error));
      retCode = BHAB_ERR_AP_UNKNOWN; /* do this for now */
   }

   if (clear0)
      BHAB_WriteRegister(h,  BCHP_LEAP_HOST_L2_CLEAR0, &clear0);

   return retCode;
}


/******************************************************************************
 BHAB_7366_P_DisableLeapInterrupts()
******************************************************************************/
BERR_Code BHAB_7366_P_DisableLeapInterrupts(
   BHAB_Handle h   /* [in] BHAB handle */
)
{
   uint32_t mask = 0xFFFFFFFF;

   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_SET0, &mask);
   BHAB_WriteRegister(h, BCHP_LEAP_HOST_L2_MASK_SET1, &mask);

   return BERR_SUCCESS;
}


#ifdef BHAB_VERIFY_DOWNLOAD
/******************************************************************************
 BHAB_7366_P_CompareMemory()
******************************************************************************/
int BHAB_7366_P_CompareMemory(const void *p1, const void *p2, size_t n)
{
   size_t i;
   int nerrors = 0;
   uint8_t *pBuf1 = (uint8_t*)p1, *pBuf2 = (uint8_t*)p2, data1, data2;

   for (i = 0; (nerrors < 256) && (i < n); i++)
   {
      data1 = *pBuf1++;
      data2 = *pBuf2++;
      if (data1 != data2)
      {
         BDBG_WRN(("BHAB_7366_P_CompareMemory error: i=0x%X, expected=0x%02X, read=0x%02X", i, data1, data2));
         nerrors++;
      }
   }

   return nerrors;
}


/******************************************************************************
 BHAB_7366_VerifyMemory() - Read the IRAM
******************************************************************************/
bool BHAB_7366_VerifyMemory(BHAB_Handle h, uint32_t addr, const uint8_t *pHexImage, uint32_t len)
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

   BHAB_7366_P_ReadMemory(h, addr, (uint8_t *)pVerifyBuf, len);

   if (BHAB_7366_P_CompareMemory(pHexImage, pVerifyBuf, len) == 0)
      b = true;
   else
      b = false;
   BKNI_Free(pVerifyBuf);
   return b;
}
#endif


/******************************************************************************
 BERR_Code BHAB_7366_P_WaitForEvent()
******************************************************************************/
BERR_Code BHAB_7366_P_WaitForEvent(BHAB_Handle h, BKNI_EventHandle hEvent, int timeoutMsec)
{
#if BHAB_CHIP==4548
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;

   BHAB_4548_P_EnableHostInterrupt(h, true);
   while (1)
   {
      retCode = BKNI_WaitForEvent(pImpl->hApiEvent, timeoutMsec);
      if ((retCode == BERR_TIMEOUT) || (retCode == BERR_OS_ERROR))
      {
         break;
      }
      else if (retCode == BERR_SUCCESS)
      {
         BHAB_4548_P_DecodeInterrupt(h);
         if ((retCode = BKNI_WaitForEvent(hEvent, 0)) == BERR_SUCCESS)
            break;
      }
      BHAB_4548_P_EnableHostInterrupt(h, true);
   }

   BHAB_4548_P_EnableHostInterrupt(h, true);

   return retCode;
#else
   BSTD_UNUSED(h);
   return BKNI_WaitForEvent(hEvent, timeoutMsec);
#endif
}


#if BHAB_CHIP==4548
/******************************************************************************
 BHAB_4548_P_ClearHirq()
******************************************************************************/
BERR_Code BHAB_4548_P_ClearHirq(BHAB_Handle h)
{
   uint32_t val = 1;
   return BHAB_4548_P_WriteRegister(h, BCHP_GIO_DATA_LO, &val);
}


/******************************************************************************
 BHAB_4548_P_Reset()
******************************************************************************/
BERR_Code BHAB_4548_P_Reset(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t val;

   /* assert sw_init signals */
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, &val));
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_bac_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_stb_chan_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_wb_stat_top0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_aif_mdac_cal_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_afec0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_sds_top0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_fsk_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_SET_leap_sw_init_MASK;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_SW_INIT_1_SET, &val));

   /* release sw_init signals */
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, &val));
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_bac_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_stb_chan_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_wb_stat_top0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_aif_mdac_cal_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_afec0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top3_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top2_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top1_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_sds_top0_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_fsk_top_sw_init_MASK;
   val |= BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR_leap_sw_init_MASK;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_TOP_CTRL_SW_INIT_1_CLEAR, &val));

   /* SUN_GISB_ARB_REQ_MASK[22]=0 */
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_SUN_GISB_ARB_REQ_MASK, &val));
   val &= ~(1<<22);
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_SUN_GISB_ARB_REQ_MASK, &val));

   /* set up the Host IRQ pin on 4548 */
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_GIO_ODEN_LO, &val));
   val &= ~1;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_GIO_ODEN_LO, &val));
   BHAB_CHK_RETCODE(BHAB_4548_P_ReadRegister(h, BCHP_GIO_IODIR_LO, &val));
   val &= ~1;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_GIO_IODIR_LO, &val));
   BHAB_4548_P_ClearHirq(h);

   done:
   return retCode;
}


/******************************************************************************
 BHAB_7366_P_GetInterruptEventHandle()
******************************************************************************/
BERR_Code BHAB_7366_P_GetInterruptEventHandle(
   BHAB_Handle handle,            /* [in] BHAB handle */
   BKNI_EventHandle *hEvent       /* [out] interrupt event handle */
)
{
   *hEvent = ((BHAB_7366_P_Handle *)(handle->pImpl))->hInterruptEvent;
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4548_P_ProcessInterruptEvent()
 ******************************************************************************/
BERR_Code BHAB_4548_P_ProcessInterruptEvent(BHAB_Handle handle)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ASSERT(handle);

   BHAB_CHK_RETCODE(BHAB_4548_P_DecodeInterrupt(handle));
   BHAB_4548_P_EnableHostInterrupt(handle, true);

   done:
   return retCode;
}


/******************************************************************************
 BERR_Code BHAB_4548_P_UnlockSerialInterface()
******************************************************************************/
BERR_Code BHAB_4548_P_UnlockSerialInterface(BHAB_Handle h)
{
   BERR_Code retCode;
   uint32_t unlock, csr_config, i;

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      for (i = 0; i < 20; i++)
      {
         unlock = 0xFD;
         retCode = BHAB_4548_P_WriteRegister(h,BCHP_CSR_CHIP_ID_ADR,&unlock);
         if (retCode == BERR_SUCCESS)
         {
            unlock = 0;
            retCode = BHAB_4548_P_ReadRegister(h,BCHP_CSR_CHIP_ID_ADR,&unlock);
            if ((retCode == BERR_SUCCESS) && (unlock == 0xFD))
               break;
            else if (retCode == BERR_SUCCESS)
               retCode = BERR_NOT_INITIALIZED;
         }
      }
      if (retCode != BERR_SUCCESS)
         goto done;

      csr_config = (CSR_CONFIG0_ACCESS_32BIT | CSR_CONFIG0_SINGL_XFR | CSR_CONFIG0_SNGL_ADDR);
      BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h,BCHP_CSR_CONFIG_0,&csr_config));
   }

   done:
   return retCode;
}


/******************************************************************************
 BHAB_4548_P_DownloadFirmware()
******************************************************************************/
BERR_Code BHAB_4548_P_DownloadFirmware(BHAB_Handle h, uint32_t start_addr, const uint8_t *buf, uint32_t n)
{
   BERR_Code retCode;
   uint32_t csr_config, phy_address, i;
   uint8_t *pBuf, csr_status = 0;

   /* make sure memory addresses are 32-bit aligned */
   if ((n == 0) || (start_addr & 0x03))
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   /* make sure length is multiple of 4 bytes */
   if (n & 0x3)
      return BERR_TRACE(BERR_INVALID_PARAMETER);

   phy_address = (0xF0000000 | start_addr);
   pBuf = (uint8_t*)BKNI_Malloc(n+5);

   csr_config = CSR_CONFIG0_ACCESS_32BIT;
   BHAB_CHK_RETCODE(BHAB_4548_P_WriteRegister(h, BCHP_CSR_CONFIG_0, &csr_config));

   pBuf[0] = (uint8_t)(CORE_ADDR_5_0(phy_address)  | CORE_SELECT | EXTN_ADDR_REG0);
   pBuf[1] = (uint8_t)(CORE_ADDR_12_6(phy_address) | EXTN_ADDR);
   pBuf[2] = (uint8_t)(CORE_ADDR_19_13(phy_address)| EXTN_ADDR);
   pBuf[3] = (uint8_t)(CORE_ADDR_26_20(phy_address)| EXTN_ADDR);
   pBuf[4] = (uint8_t)(CORE_ADDR_33_27(phy_address)| EXTN_ADDR_STOP);

   for (i = 0; i < n; i += 4)
   {
      pBuf[5+i] = buf[i+3];
      pBuf[6+i] = buf[i+2];
      pBuf[7+i] = buf[i+1];
      pBuf[8+i] = buf[i];
   }

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
      goto done;
   }
   else
   {
      retCode = BREG_I2C_WriteNoAddr(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr,  pBuf, n+5);
      if(retCode != BERR_SUCCESS)
      {
         BDBG_ERR(("BREG_I2C_WriteNoAddr Error"));
         goto done;
      }
   }

   retCode = BHAB_4548_P_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("BHAB_4548_P_ReadBbsi(CSR_STATUS) error 0x%X", csr_status));
   }
   else if (csr_status & (CSR_STATUS_TXFR_W_OVERUN | CSR_STATUS_TXFR_TIMEOUT | CSR_STATUS_TXFR_ERR | CSR_STATUS_TXFR_PENDING))
   {
      BDBG_ERR(("ERROR: CSR_STATUS=0x%X", csr_status));
   }

   done:
   BKNI_Free(pBuf);
   csr_config = (CSR_CONFIG0_ACCESS_32BIT | CSR_CONFIG0_SINGL_XFR | CSR_CONFIG0_SNGL_ADDR);
   BHAB_4548_P_WriteRegister(h, BCHP_CSR_CONFIG_0, &csr_config);
   return retCode;
}


/******************************************************************************
 BHAB_4548_P_EnableHostInterrupt()
******************************************************************************/
BERR_Code BHAB_4548_P_EnableHostInterrupt(BHAB_Handle h, bool bEnable)
{
   BKNI_EnterCriticalSection();
   h->settings.interruptEnableFunc(bEnable, h->settings.interruptEnableFuncParam);
   BKNI_LeaveCriticalSection();
   return BERR_SUCCESS;
}


/******************************************************************************
 BHAB_4548_P_WriteBbsi()
******************************************************************************/
BERR_Code BHAB_4548_P_WriteBbsi(
   BHAB_Handle h,    /* [in] BHAB PI Handle */
   uint8_t     addr, /* [in] address */
   uint8_t     *buf, /* [in] data to write */
   uint32_t    n     /* [in] number of bytes to write */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
       retCode = BREG_I2C_Write(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);
   }

   return retCode;
}


/******************************************************************************
 BHAB_4548_P_ReadBbsi()
******************************************************************************/
BERR_Code BHAB_4548_P_ReadBbsi(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint8_t     addr, /* [in] address */
    uint8_t     *buf, /* [out] buffer that holds the data */
    uint32_t     n     /* [in] number of bytes to read */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      retCode = BREG_I2C_Read(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, addr, buf, n);
   }

   return retCode;
}


/******************************************************************************
BHAB_4548_P_ReadRbus()
******************************************************************************/
BERR_Code BHAB_4548_P_ReadRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [in] data to write */
    uint32_t    n     /* [in] number of 32-bit words to read */
    )
{
   uint8_t wBuf[8], rBuf[16], csr_status=0;
   uint32_t i, i2c_index, phy_address;
   BERR_Code retCode = BERR_SUCCESS;

   if (n == 0)
      return BERR_INVALID_PARAMETER;

   phy_address = (0xF0000000 | addr);

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      for (i = 0; i < n; i++)
      {
         /* Set up the Core access address */
         if (i == 0)
         {
            wBuf[0] = (uint8_t)(CORE_ADDR_5_0(phy_address)  | CORE_SELECT | EXTN_ADDR_REG0);
            wBuf[1] = (uint8_t)(CORE_ADDR_12_6(phy_address) | EXTN_ADDR);
            wBuf[2] = (uint8_t)(CORE_ADDR_19_13(phy_address)| EXTN_ADDR);
            wBuf[3] = (uint8_t)(CORE_ADDR_26_20(phy_address)| EXTN_ADDR);
            wBuf[4] = (uint8_t)(CORE_ADDR_33_27(phy_address)| EXTN_ADDR_STOP);
            retCode = BREG_I2C_WriteNoAddr(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr,  wBuf, 5);
            if(retCode != BERR_SUCCESS)
            {
               BDBG_ERR(("BREG_I2C_WriteNoAddr Error 0x%X",retCode));
            }
         }

         retCode = BREG_I2C_ReadNoAddr(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr, rBuf,(4 + BHAB_MAX_BBSI_STATUS_BYTES));
         if (retCode != BERR_SUCCESS)
         {
            BDBG_ERR(("BREG_I2C_ReadNoAddr Error 0x%X",retCode));
         }

         i2c_index = 0;
         while(i2c_index < (4 + BHAB_MAX_BBSI_STATUS_BYTES))
         {
            if (rBuf[i2c_index] & I2C_READ_STATUS_ERROR)
            {
               BHAB_4548_P_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1);
               BDBG_ERR(("I2C_READ_STATUS_ERROR(Addr 0x%X) : Status 0x%X",addr,csr_status));
               retCode = BHAB_ERR_HOST_XFER;
               goto done;
            }

            if (i2c_index == BHAB_MAX_BBSI_STATUS_BYTES)
            {
               BDBG_ERR(("I2C_READ_STATUS_READY not set"));
               retCode = BHAB_ERR_HAB_TIMEOUT;
               goto done;
            }

            if (rBuf[i2c_index] & I2C_READ_STATUS_READY)
            {
               i2c_index++;
               break;
            }

            i2c_index++;
         }

         buf[i] = (uint32_t)((rBuf[i2c_index] << 24) | (rBuf[i2c_index + 1] << 16) | (rBuf[i2c_index + 2] << 8) | rBuf[i2c_index + 3] << 0);
      }
   }

   done:
   return retCode;
}


/******************************************************************************
BHAB_4548_P_WriteRbus()
******************************************************************************/
BERR_Code BHAB_4548_P_WriteRbus(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    addr, /* [in] address */
    uint32_t    *buf, /* [out] buffer that holds the data */
    uint32_t    n     /* [in] number of 32-bit words to read */
    )
{
   uint8_t wBuf[16], csr_status =0;
   uint32_t i, data_idx, phy_address;
   BERR_Code retCode = BERR_SUCCESS;

   phy_address = (0xF0000000 | addr);

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      for (i = 0; i < n; i++)
      {
         /* set up the starting address to write */
         if (i == 0)
         {
            wBuf[0] = (uint8_t)(CORE_ADDR_5_0(phy_address)  | CORE_SELECT | EXTN_ADDR_REG0);
            wBuf[1] = (uint8_t)(CORE_ADDR_12_6(phy_address) | EXTN_ADDR);
            wBuf[2] = (uint8_t)(CORE_ADDR_19_13(phy_address)| EXTN_ADDR);
            wBuf[3] = (uint8_t)(CORE_ADDR_26_20(phy_address)| EXTN_ADDR);
            wBuf[4] = (uint8_t)(CORE_ADDR_33_27(phy_address)| EXTN_ADDR_STOP);
            data_idx = 5;
         }
         else
            data_idx = 0;

         wBuf[data_idx++] = (uint8_t)((buf[i] >> 24)& 0xFF);
         wBuf[data_idx++] = (uint8_t)((buf[i] >> 16)& 0xFF);
         wBuf[data_idx++] = (uint8_t)((buf[i] >> 8) & 0xFF);
         wBuf[data_idx++] = (uint8_t)(buf[i] & 0xFF);

         retCode = BREG_I2C_WriteNoAddr(((BHAB_7366_P_Handle*)h->pImpl)->hI2cRegister, h->settings.chipAddr,  wBuf, data_idx);
         if(retCode != BERR_SUCCESS)
         {
            BDBG_ERR(("BREG_I2C_WriteNoAddr Error"));
         }

         BHAB_4548_P_ReadBbsi(h, BCHP_CSR_STATUS, &csr_status, 1);
         if (csr_status & (CSR_STATUS_TXFR_W_OVERUN | CSR_STATUS_TXFR_TIMEOUT | CSR_STATUS_TXFR_ERR | CSR_STATUS_TXFR_PENDING))
         {
            BDBG_ERR(("BHAB_7366_P_WriteRbus Error(Addr 0x%X) : Status 0x%X",addr,csr_status));
         }
      }
   }

   return retCode;
}


/******************************************************************************
 BHAB_4548_P_DecodeInterrupt()
******************************************************************************/
BERR_Code BHAB_4548_P_DecodeInterrupt(BHAB_Handle h)
{
   BHAB_7366_P_Handle *pImpl = (BHAB_7366_P_Handle *)(h->pImpl);
   BERR_Code retCode = BERR_SUCCESS;
   BHAB_7366_P_CallbackInfo *pCallbackInfo;
   uint32_t status0, status1, mask0, mask1, fstatus0, fstatus1, channel, val32;
   uint32_t mask_set0 = 0, mask_set1 = 0;

   BHAB_4548_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS0, &status0);
   BHAB_4548_P_ReadRegister(h, BCHP_LEAP_HOST_L2_STATUS1, &status1);
   BHAB_4548_P_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS0, &mask0);
   BHAB_4548_P_ReadRegister(h, BCHP_LEAP_HOST_L2_MASK_STATUS1, &mask1);

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
      if (fstatus0 & BHAB_7366_HIRQ0_INIT_DONE)
      {
         BKNI_SetEvent(pImpl->hInitDoneEvent);
         mask_set0 |= BHAB_7366_HIRQ0_INIT_DONE;
      }

      if (fstatus0 & BHAB_7366_HIRQ0_HAB_DONE)
      {
         BKNI_SetEvent(pImpl->hHabDoneEvent);
         mask_set0 |= BHAB_7366_HIRQ0_HAB_DONE;
      }

      if (fstatus0 & BHAB_7366_HIRQ0_WFE0_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE0_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE1_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE1_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE2_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE2_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE3_READY)
         mask_set0 |= BHAB_7366_HIRQ0_WFE3_READY;

      if (fstatus0 & BHAB_7366_HIRQ0_WFE_SA_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_WFE_SA_DONE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_RX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_RX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC0_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC0_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_RX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_RX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC1_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC1_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC2_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC2_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_TX)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_TX;
      if (fstatus0 & BHAB_7366_HIRQ0_DSEC3_VSENSE)
         mask_set0 |= BHAB_7366_HIRQ0_DSEC3_VSENSE;

      if (fstatus0 & BHAB_7366_HIRQ0_FLASH_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_FLASH_DONE;

      if (fstatus0 & BHAB_7366_HIRQ0_SAT_INIT_DONE)
         mask_set0 |= BHAB_7366_HIRQ0_SAT_INIT_DONE;
   }

   if (fstatus1)
   {
      for (channel = 0; channel < 8; channel++)
      {
         val32 = (fstatus1 >> (channel*4)) & 0x0F;
         if (val32 & BHAB_7366_HIRQ1_CHAN0_READY)
         {
            mask_set1 |= (BHAB_7366_HIRQ1_CHAN0_READY << (channel*4));
         }
      }
   }

   if (fstatus1 || (fstatus0 & BHAB_7366_HIRQ0_SAT_MASK))
   {
      pCallbackInfo = &(pImpl->cbSat);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call sat pi callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
      else
      {
         BDBG_ERR(("BHAB: SAT PI interrupt callback not installed!"));
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_DSQ_MASK)
   {
      pCallbackInfo = &(pImpl->cbDsq);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call dsq callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_FSK_MASK)
   {
      pCallbackInfo = &(pImpl->cbFsk);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call dsq callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   if (fstatus0 & BHAB_7366_HIRQ0_WFE_MASK)
   {
      pCallbackInfo = &(pImpl->cbWfe);
      pCallbackInfo->callbackParams.status0 = fstatus0;
      pCallbackInfo->callbackParams.status1 = fstatus1;

      if (pCallbackInfo->func)
      {
         /* call wfe callback */
         pCallbackInfo->func(&(pCallbackInfo->callbackParams), 0);
      }
   }

   /* clear the interrupt status */
   BHAB_4548_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR0, &fstatus0);
   BHAB_4548_P_WriteRegister(h, BCHP_LEAP_HOST_L2_CLEAR1, &fstatus1);

   if (mask_set0)
   {
      /* disable specified interrupts */
      BHAB_4548_P_WriteRegister(h,  BCHP_LEAP_HOST_L2_MASK_SET0, &mask_set0);
   }
   if (mask_set1)
   {
      /* disable specified interrupts */
      BHAB_4548_P_WriteRegister(h,  BCHP_LEAP_HOST_L2_MASK_SET1, &mask_set1);
   }

   done:
   BHAB_4548_P_ClearHirq(h);
   return retCode;
}


/******************************************************************************
 BHAB_4548_P_WriteRegister()
******************************************************************************/
BERR_Code BHAB_4548_P_WriteRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   bool bHsi = false;
   uint8_t sb;

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      if (reg < 0x100)
         bHsi = true;

      /* TBD : INVALID REGISTER ACCESS BOUNDS TO BE HANDLED */

      if (bHsi)
      {
         sb = (uint8_t)(*val & 0xFF);
         retCode = BHAB_4548_P_WriteBbsi(h, (uint8_t)reg, &sb, 1);
      }
      else
         retCode = BHAB_4548_P_WriteRbus(h, reg, val, 1);
   }

   return retCode;
}


/******************************************************************************
 BHAB_4548_P_ReadRegister()
******************************************************************************/
BERR_Code BHAB_4548_P_ReadRegister(
    BHAB_Handle h,    /* [in] BHAB PI Handle */
    uint32_t    reg,  /* [in] RBUS register address */
    uint32_t    *val  /* [in] value to write */
)
{
   BERR_Code retCode = BERR_SUCCESS;

   bool bHsi = false;
   uint8_t sb;

   if (h->settings.isSpi)
   {
      /* TBD... */
      retCode = BERR_NOT_SUPPORTED;
   }
   else
   {
      if (reg < 0x100)
         bHsi = true;

      /* TBD : INVALID REGISTER ACCESS BOUNDS TO BE HANDLED */

      if (bHsi)
      {
         retCode = BHAB_4548_P_ReadBbsi(h, (uint8_t)reg, &sb, 1);
         if (retCode == BERR_SUCCESS)
            *val = (uint32_t)sb;
      }
      else
         retCode = BHAB_4548_P_ReadRbus(h, reg, val, 1);
   }

   return retCode;
}

#endif /*BHAB_CHIP==4548*/
