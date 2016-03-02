/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_spi_module.h"
#include "bspi.h"
#include "breg_spi.h"
#if (BCHP_CHIP==7408 || BCHP_CHIP==7468)
#include "bchp_hif_mspi.h"
#else
#include "bchp_mspi.h"
#endif
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_spi);

#ifndef BCHP_MSPI_SPCR0_MSB_CPOL_MASK
#define BCHP_MSPI_SPCR0_MSB_CPOL_MASK BCHP_HIF_MSPI_SPCR0_MSB_CPOL_MASK
#define BCHP_MSPI_SPCR0_MSB_CPHA_MASK BCHP_HIF_MSPI_SPCR0_MSB_CPHA_MASK
#endif

NEXUS_ModuleHandle g_NEXUS_spiModule;
struct {
    NEXUS_SpiModuleSettings settings;
    BSPI_Handle spi;
} g_NEXUS_spi;

/****************************************
* Module functions
***************/

void NEXUS_SpiModule_GetDefaultSettings(NEXUS_SpiModuleSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_GetDefaultCommonModuleSettings(&pSettings->common);
}

NEXUS_ModuleHandle NEXUS_SpiModule_Init(const NEXUS_SpiModuleSettings *pSettings)
{
    NEXUS_ModuleSettings moduleSettings;
    BSPI_Settings spiSettings;
    BERR_Code rc;

    BDBG_ASSERT(!g_NEXUS_spiModule);

    if (pSettings) {
        g_NEXUS_spi.settings = *pSettings;
    }
    else {
        NEXUS_SpiModule_GetDefaultSettings(&g_NEXUS_spi.settings);
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_AdjustModulePriority(moduleSettings.priority, &g_NEXUS_spi.settings.common);
    g_NEXUS_spiModule = NEXUS_Module_Create("spi", &moduleSettings);

    BSPI_GetDefaultSettings(&spiSettings, g_pCoreHandles->chp);
    rc = BSPI_Open(&g_NEXUS_spi.spi, g_pCoreHandles->chp, g_pCoreHandles->reg, g_pCoreHandles->bint, &spiSettings);
    if (rc) {rc=BERR_TRACE(NEXUS_UNKNOWN); return NULL;}

    return g_NEXUS_spiModule;
}

void NEXUS_SpiModule_Uninit(void)
{
    BSPI_Close(g_NEXUS_spi.spi);
    NEXUS_Module_Destroy(g_NEXUS_spiModule);
    g_NEXUS_spiModule = NULL;
}

/****************************************
* API functions
***************/

struct NEXUS_Spi {
    NEXUS_OBJECT(NEXUS_Spi);
    BSPI_ChannelHandle spiChannel;
    BREG_SPI_Handle spiReg;
    NEXUS_SpiSettings settings;
};

void NEXUS_Spi_GetDefaultSettings(NEXUS_SpiSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->txLeadingCapFalling = true;
    pSettings->clockActiveLow = true;
    pSettings->bitsPerTransfer = 8;
}

NEXUS_SpiHandle NEXUS_Spi_Open(unsigned index, const NEXUS_SpiSettings *pSettings)
{
    BSPI_ChannelSettings channelSettings;
    NEXUS_SpiHandle spi;
    BERR_Code rc;
    unsigned totalChannels;
    NEXUS_SpiSettings defaultSettings;
    BSPI_ChannelInfo    channelInfo;

    if (!pSettings) {
        NEXUS_Spi_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    channelInfo.channelNo = index & 0x3; /* There can be as many as three channels supported per interface */

    BSPI_GetTotalUpgSpiChannels(g_NEXUS_spi.spi, &totalChannels);
    BSPI_GetTotalChannels(g_NEXUS_spi.spi, &totalChannels);
    if (channelInfo.channelNo >= totalChannels) {
        BDBG_ERR(("invalid Spi[%d]", index));
        return NULL;
    }

    spi = BKNI_Malloc(sizeof(*spi));
    if (!spi) {
        rc=BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Spi, spi);

    channelInfo.spiCore = BSPI_SpiCore_Upg;
    BSPI_GetChannelDefaultSettings_Ext(g_NEXUS_spi.spi, channelInfo, &channelSettings);

    if ( pSettings->baud ) {
        channelSettings.baud                   = pSettings->baud;
    }
    if ( pSettings->bitsPerTransfer ) {
        channelSettings.bitsPerTxfr            = pSettings->bitsPerTransfer;
    }
    if ( pSettings->lastByteContinueEnable ) {
        channelSettings.lastByteContinueEnable = pSettings->lastByteContinueEnable;
    }
    if ( pSettings->useUserDtlAndDsclk ) {
        channelSettings.useUserDtlAndDsclk     = pSettings->useUserDtlAndDsclk;
    }
    if ( pSettings->ebiChipSelect) {
        channelSettings.ebiCs = pSettings->ebiChipSelect;
    }

    if (pSettings->clockActiveLow)
        channelSettings.clkConfig |= BCHP_MSPI_SPCR0_MSB_CPOL_MASK; /* if 1, then 0 is active */
    else
        channelSettings.clkConfig &= ~BCHP_MSPI_SPCR0_MSB_CPOL_MASK; /* if 0, then 1 is active */
    if (pSettings->txLeadingCapFalling)
        channelSettings.clkConfig |= BCHP_MSPI_SPCR0_MSB_CPHA_MASK;
    else
        channelSettings.clkConfig &= ~BCHP_MSPI_SPCR0_MSB_CPHA_MASK;

    channelSettings.intMode = pSettings->interruptMode;
    spi->settings = *pSettings;

    rc = BSPI_OpenChannel(g_NEXUS_spi.spi, &spi->spiChannel, channelInfo.channelNo, &channelSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    rc = BSPI_CreateSpiRegHandle(spi->spiChannel, &spi->spiReg);
    if (rc) {rc=BERR_TRACE(rc); goto error;}

    rc = NEXUS_Spi_SetSettings(spi, pSettings);
    if (rc) {rc=BERR_TRACE(rc); goto error;}


    return spi;
error:
    NEXUS_Spi_Close(spi);
    return NULL;
}

static void NEXUS_Spi_P_Finalizer(NEXUS_SpiHandle spi)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Spi, spi);

    if (spi->spiReg) {
        BSPI_CloseSpiRegHandle(spi->spiReg);
    }
    if (spi->spiChannel) {
        BSPI_CloseChannel(spi->spiChannel);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Spi, spi);
    BKNI_Free(spi);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Spi, NEXUS_Spi_Close);

void NEXUS_Spi_GetSettings(NEXUS_SpiHandle spi, NEXUS_SpiSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(spi, NEXUS_Spi);
    *pSettings = spi->settings;
}

NEXUS_Error NEXUS_Spi_SetSettings(NEXUS_SpiHandle spi, const NEXUS_SpiSettings *pSettings)
{
    uint8_t clkConfig;
    BERR_Code rc;
    BSPI_Delay delay;

    BDBG_OBJECT_ASSERT(spi, NEXUS_Spi);

    /* if open-time only settings are changed, fail */
    if (pSettings->baud != spi->settings.baud ||
        pSettings->useUserDtlAndDsclk != spi->settings.useUserDtlAndDsclk ||
        pSettings->interruptMode != spi->settings.interruptMode)
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    BSPI_GetClkConfig(spi->spiChannel, &clkConfig);
    if (pSettings->clockActiveLow)
        clkConfig |= BCHP_MSPI_SPCR0_MSB_CPOL_MASK; /* if 1, then 0 is active */
    else
        clkConfig &= ~BCHP_MSPI_SPCR0_MSB_CPOL_MASK; /* if 0, then 1 is active */
    if (pSettings->txLeadingCapFalling)
        clkConfig |= BCHP_MSPI_SPCR0_MSB_CPHA_MASK;
    else
        clkConfig &= ~BCHP_MSPI_SPCR0_MSB_CPHA_MASK;

    rc = BSPI_SetClkConfig(spi->spiChannel, clkConfig);
    if (rc) return BERR_TRACE(rc);

    if(pSettings->useUserDtlAndDsclk){
        if((pSettings->dtl) || pSettings->rdsclk){
            rc = BSPI_SetDTLConfig(spi->spiChannel, pSettings->dtl);
            if (rc) return BERR_TRACE(rc);
            rc = BSPI_SetRDSCLKConfig(spi->spiChannel, pSettings->rdsclk);
            if (rc) return BERR_TRACE(rc);
        }
        else{
            delay.postTransfer = pSettings->delayAfterTransfer;
            delay.chipSelectToClock = pSettings->delayChipSelectToClock;
            rc = BSPI_SetDelay(spi->spiChannel, &delay);
            if (rc) return BERR_TRACE(rc);
        }
    }

    rc = BSPI_SetBitsPerTransfer(spi->spiChannel, pSettings->bitsPerTransfer);
    if (rc) return BERR_TRACE(rc);

    if (pSettings->lastByteContinueEnable != spi->settings.lastByteContinueEnable)
        BSPI_SetLastByteContinueEnable(spi->spiChannel, pSettings->lastByteContinueEnable);

    spi->settings = *pSettings;
    return 0;
}

NEXUS_Error NEXUS_Spi_Write(NEXUS_SpiHandle spi, const uint8_t *pWriteData,
    size_t length)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(spi, NEXUS_Spi);
    rc = BREG_SPI_WriteAll(spi->spiReg, pWriteData, length);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

NEXUS_Error NEXUS_Spi_Read(NEXUS_SpiHandle spi, const uint8_t *pWriteData,
    uint8_t *pReadData, size_t length)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(spi, NEXUS_Spi);
    rc = BREG_SPI_Read(spi->spiReg, pWriteData, pReadData, length);
    if (rc) return BERR_TRACE(rc);
    return 0;
}


NEXUS_Error NEXUS_Spi_ReadAll(NEXUS_SpiHandle spi, const uint8_t *pWriteData,
    size_t writeLength, uint8_t *pReadData, size_t readLength, size_t *pActualReadLength)
{
    BERR_Code rc;
    BDBG_OBJECT_ASSERT(spi, NEXUS_Spi);

    rc = BREG_SPI_ReadAll(spi->spiReg, pWriteData, writeLength, pReadData, readLength);
    if (rc) return BERR_TRACE(rc);
    /* This needs to be tied to a timeout and pass back the correct read length. */
    *pActualReadLength = readLength;
    return 0;
}

BREG_SPI_Handle NEXUS_Spi_GetRegHandle(
    NEXUS_SpiHandle spiHandle
    )
{
    BDBG_OBJECT_ASSERT(spiHandle, NEXUS_Spi);

    return spiHandle->spiReg;
}

