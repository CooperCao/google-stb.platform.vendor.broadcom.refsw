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

BDBG_MODULE(nexus_spi_os);

void NEXUS_Spi_OsShared_GetCapabilities(NEXUS_SpiOsSharedCapabilities *pCapabilities)
{
    pCapabilities->osManaged = false;
}

NEXUS_SpiOsSharedDeviceHandle NEXUS_Spi_OsShared_Open(unsigned index, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    return NULL;
}

void NEXUS_Spi_OsShared_Close(NEXUS_SpiOsSharedDeviceHandle spiDevice)
{
    BSTD_UNUSED(spiDevice);
}

void NEXUS_Spi_OsShared_GetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Spi_OsShared_SetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings)
{
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
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
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(count);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Spi_OsShared_Write(void *context, const uint8_t *pWriteData, size_t length)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(length);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Spi_OsShared_Read(void *context, const uint8_t *pWriteData, uint8_t *pReadData, size_t length)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(pWriteData);
    BSTD_UNUSED(pReadData);
    BSTD_UNUSED(length);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
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
    BSTD_UNUSED(spiDevice);
    BSTD_UNUSED(pSpiReg);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Spi_OsShared_DestroySpiRegHandle(BREG_SPI_Handle hSpiReg)
{
    BSTD_UNUSED(hSpiReg);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
