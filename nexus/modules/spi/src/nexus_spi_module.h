/***************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_SPI_MODULE_H__
#define NEXUS_SPI_MODULE_H__

#include "nexus_base.h"
#include "nexus_spi_thunks.h"
#include "nexus_spi.h"
#include "nexus_spi_private.h"
#include "nexus_spi_init.h"
#include "priv/nexus_spi_priv.h"
#include "breg_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME spi
#define NEXUS_MODULE_SELF g_NEXUS_spiModule

/* global handle. there is no global data. */
extern NEXUS_ModuleHandle g_NEXUS_spiModule;

typedef struct NEXUS_SpiOsSharedDevice * NEXUS_SpiOsSharedDeviceHandle;

typedef struct NEXUS_SpiOsSharedCapabilities
{
    bool osManaged;
} NEXUS_SpiOsSharedCapabilities;

typedef struct NEXUS_SpiOsSharedDeviceSettings {
    uint32_t      baud;
    uint8_t       bitsPerTransfer;
    bool          clockActiveLow;
    bool          txLeadingCapFalling;
} NEXUS_SpiOsSharedDeviceSettings;

void NEXUS_Spi_OsShared_GetCapabilities(NEXUS_SpiOsSharedCapabilities *pCapabilities);
NEXUS_SpiOsSharedDeviceHandle NEXUS_Spi_OsShared_Open(unsigned index, NEXUS_SpiOsSharedDeviceSettings *pSettings);
void NEXUS_Spi_OsShared_Close(NEXUS_SpiOsSharedDeviceHandle spiDevice);
void NEXUS_Spi_OsShared_GetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings);
NEXUS_Error NEXUS_Spi_OsShared_SetSettings(NEXUS_SpiOsSharedDeviceHandle spiDevice, NEXUS_SpiOsSharedDeviceSettings *pSettings);
NEXUS_Error NEXUS_Spi_OsShared_CreateSpiRegHandle(NEXUS_SpiOsSharedDeviceHandle spiDevice, BREG_SPI_Handle *pSpiReg);
NEXUS_Error NEXUS_Spi_OsShared_DestroySpiRegHandle(BREG_SPI_Handle hSpiReg);


#ifdef __cplusplus
}
#endif

#endif
