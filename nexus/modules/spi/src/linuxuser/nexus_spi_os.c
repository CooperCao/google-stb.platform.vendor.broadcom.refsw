/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_spi_module.h"

#if NEXUS_SHARED_SPI
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "spi_driver.h"
#include "breg_spi_priv.h"

typedef struct NEXUS_SpiOsSharedDevice {
    int fd;
    NEXUS_SpiOsSharedDeviceSettings settings;
} NEXUS_SpiOsSharedDevice;
#endif

BDBG_MODULE(nexus_spi_os);

void NEXUS_Spi_OsShared_GetCapabilities(NEXUS_SpiOsSharedCapabilities *pCapabilities)
{
#if NEXUS_SHARED_SPI
    int i;

    for (i=0; i<UPG_MSPI_MAX_CS; i++) {
        char dev[16];
        struct stat st;

        BKNI_Snprintf(dev, sizeof(dev), "/dev/spi_drv%u", i);
        if (!stat(dev, &st)) {
            break;
        }
    }
    pCapabilities->osManaged = i==UPG_MSPI_MAX_CS?false:true;
#else
    pCapabilities->osManaged = false;
#endif
}

NEXUS_SpiOsSharedDeviceHandle NEXUS_Spi_OsShared_Open(unsigned index, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
#if NEXUS_SHARED_SPI
    NEXUS_SpiOsSharedDeviceHandle spiDevice = NULL;
    char dev[16];
    struct spidrv_settings settings;

    spiDevice = BKNI_Malloc(sizeof(NEXUS_SpiOsSharedDevice));
    if (!spiDevice) {
        BDBG_ERR(("Failed to allocate memory"));
        goto err_alloc;
    }
    BKNI_Memset(spiDevice, 0, sizeof(NEXUS_SpiOsSharedDevice));

    BKNI_Snprintf(dev, sizeof(dev), "/dev/spi_drv%u", index);
    spiDevice->fd = open(dev, O_RDWR);
    if (spiDevice->fd < 0) {
        BDBG_ERR(("Unable to open spi device %s", dev));
        goto err_fd;
    }

    if (ioctl(spiDevice->fd, BRCM_IOCTL_SPI_GET, &settings)) {
        BDBG_ERR(("Failed to get Spi settings"));
        goto err_ioctl;
    }

    if (pSettings->baud)
        settings.speed = pSettings->baud;
    if (pSettings->bitsPerTransfer)
        settings.bits_per_word = pSettings->bitsPerTransfer;
    if (pSettings->clockActiveLow)
        settings.clock_active_low = pSettings->clockActiveLow;
    if (pSettings->txLeadingCapFalling)
        settings.tx_leading_cap_falling = pSettings->txLeadingCapFalling;

    if(ioctl(spiDevice->fd, BRCM_IOCTL_SPI_SET, &settings)) {
        BDBG_ERR(("Failed to set Spi settings"));
        goto err_ioctl;
    }

    return spiDevice;

err_ioctl:
    if (spiDevice->fd) {
        close(spiDevice->fd);
    }
err_fd:
    if (spiDevice) {
        BKNI_Free(spiDevice);
        spiDevice = NULL;
    }
err_alloc:
    return spiDevice;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    return NULL;
#endif
}

void NEXUS_Spi_OsShared_Close(NEXUS_SpiOsSharedDeviceHandle spiDevice)
{
#if NEXUS_SHARED_SPI
    if (spiDevice->fd) {
        close(spiDevice->fd);
    }
    if (spiDevice) {
        BKNI_Free(spiDevice);
        spiDevice = NULL;
    }
#else
    BSTD_UNUSED(spiDevice);
#endif
}

void NEXUS_Spi_OsShared_GetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
#if NEXUS_SHARED_SPI
    struct spidrv_settings settings;

    if (ioctl(spiDevice->fd, BRCM_IOCTL_SPI_GET, &settings)) {
        BDBG_ERR(("Failed to get Spi settings"));
        goto err;
    }

    pSettings->baud = settings.speed;
    pSettings->bitsPerTransfer = settings.bits_per_word;
    pSettings->clockActiveLow = settings.clock_active_low;
    pSettings->txLeadingCapFalling = settings.tx_leading_cap_falling;
err:
    return;
#else
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSettings);
#endif
}

NEXUS_Error NEXUS_Spi_OsShared_SetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
#if NEXUS_SHARED_SPI
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct spidrv_settings settings;

    settings.speed = pSettings->baud;
    settings.bits_per_word = pSettings->bitsPerTransfer;
    settings.clock_active_low = pSettings->clockActiveLow;
    settings.tx_leading_cap_falling = pSettings->txLeadingCapFalling;

    if(ioctl(spiDevice->fd, BRCM_IOCTL_SPI_SET, &settings)) {
        BDBG_ERR(("Failed to set Spi settings"));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
    }

    return rc;
#else
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_Spi_OsShared_GetBitsPerTransfer(void *context, uint8_t *pBitsPerTransfer)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(pBitsPerTransfer);
    BDBG_WRN(("NEXUS_Spi_OsShared_GetBitsPerTransfer Not Implemented"));
    return;
}

NEXUS_Error NEXUS_Spi_OsShared_Multiple_Write(void *context, const BREG_SPI_Data *pWriteData, size_t count)
{
#if NEXUS_SHARED_SPI
    NEXUS_SpiOsSharedDeviceHandle spiDevice = (NEXUS_SpiOsSharedDeviceHandle) context;
    struct spidrv_message *msg;
    struct spidrv_message_multiple msgs;
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t i;

    BDBG_MSG(("NEXUS_Spi_OsShared_Multiple_Write : pWriteData %p, count %u", (void*)pWriteData, (unsigned)count));

    msg = BKNI_Malloc(count*sizeof(struct spidrv_message));

    for(i=0; i < count; i++) {
        msg[i].tx_buf = pWriteData[i].data;
        msg[i].rx_buf = NULL;
        msg[i].len = pWriteData[i].length;
    }

    msgs.msg = msg;
    msgs.count = count;

    if(ioctl(spiDevice->fd, BRCM_IOCTL_SPI_MSG_MULT, &msgs)) {
        BDBG_ERR(("Failed to write multiple Spi data\n"));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
    }

    BKNI_Free(msg);

    return rc;
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(count);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Spi_OsShared_Write(void *context, const uint8_t *pWriteData, size_t length)
{
#if NEXUS_SHARED_SPI
    NEXUS_SpiOsSharedDeviceHandle spiDevice = (NEXUS_SpiOsSharedDeviceHandle) context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct spidrv_message msg;

    BDBG_MSG(("NEXUS_Spi_OsShared_Write : pWriteData %p, length %u", (void*)pWriteData, (unsigned)length));

    msg.tx_buf = pWriteData;
    msg.rx_buf = NULL;
    msg.len = length;

    if(ioctl(spiDevice->fd, BRCM_IOCTL_SPI_MSG, &msg)) {
        BDBG_ERR(("Failed to write Spi data\n"));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
    }

    return rc;
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(length);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Spi_OsShared_Read(void *context, const uint8_t *pWriteData, uint8_t *pReadData, size_t length)
{
#if NEXUS_SHARED_SPI
    NEXUS_SpiOsSharedDeviceHandle spiDevice = (NEXUS_SpiOsSharedDeviceHandle) context;
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct spidrv_message msg;

    BDBG_MSG(("NEXUS_Spi_OsShared_Read : pWriteData %p, pReadData %p, length %u", (void*)pWriteData, (void*)pReadData, (unsigned)length));

    msg.tx_buf = pWriteData;
    msg.rx_buf = pReadData;
    msg.len = length;

    if(ioctl(spiDevice->fd, BRCM_IOCTL_SPI_MSG, &msg)) {
        BDBG_ERR(("Failed to read Spi data\n"));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
    }

    return rc;
#else
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(pReadData);
    BSTD_UNUSED(length);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Spi_OsShared_ReadAll(void *context, const uint8_t *pWriteData, size_t writeLength, uint8_t *pReadData, size_t readLength)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(writeLength);
    BSTD_UNUSED(pReadData);
    BSTD_UNUSED(readLength);

    BDBG_WRN(("NEXUS_Spi_OsShared_ReadAll Not Implemented"));
    return NEXUS_NOT_AVAILABLE;
}

NEXUS_Error NEXUS_Spi_OsShared_CreateSpiRegHandle(NEXUS_SpiOsSharedDeviceHandle spiDevice, BREG_SPI_Handle *pSpiReg)
{
#if NEXUS_SHARED_SPI
    NEXUS_Error rc = NEXUS_SUCCESS;

    *pSpiReg = (BREG_SPI_Handle)BKNI_Malloc( sizeof(BREG_SPI_Impl));
    if (*pSpiReg == NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    (*pSpiReg)->context                                  = (void *)spiDevice;
    (*pSpiReg)->BREG_SPI_Get_Bits_Per_Transfer_Func      = NEXUS_Spi_OsShared_GetBitsPerTransfer;
    (*pSpiReg)->BREG_SPI_Multiple_Write_Func             = NEXUS_Spi_OsShared_Multiple_Write;
    (*pSpiReg)->BREG_SPI_Write_All_Func                  = NEXUS_Spi_OsShared_Write;
    (*pSpiReg)->BREG_SPI_Write_Func                      = NEXUS_Spi_OsShared_Write;
    (*pSpiReg)->BREG_SPI_Read_Func                       = NEXUS_Spi_OsShared_Read;
    (*pSpiReg)->BREG_SPI_Read_All_Func                   = NEXUS_Spi_OsShared_ReadAll;

done:
    return rc;
#else
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSpiReg);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Spi_OsShared_DestroySpiRegHandle(BREG_SPI_Handle hSpiReg)
{
#if NEXUS_SHARED_SPI
    BDBG_ASSERT(hSpiReg);
    BKNI_Free(hSpiReg);

    return NEXUS_SUCCESS;
#else
    BSTD_UNUSED(hSpiReg);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}
