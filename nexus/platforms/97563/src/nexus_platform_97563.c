/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/
#include "nexus_platform_priv.h"
#include "bchp_common.h"

BDBG_MODULE(nexus_platform_97563);

#if NEXUS_PLATFORM_7563_DGL
#define NEXUS_PLATFORM_STRING " HDMI Dongle"
#elif NEXUS_PLATFORM_7563_ULC
#define NEXUS_PLATFORM_STRING " Ultra Low Cost"
#elif NEXUS_PLATFORM_7563_USFF2L
#define NEXUS_PLATFORM_STRING " RSFF 2 Layer"
#else
#define NEXUS_PLATFORM_STRING ""
#endif

#define MB (1024 * 1024)

static void nexus_p_modifyDefaultMemoryConfigurationSettings(NEXUS_MemoryConfigurationSettings *pSettings)
{
    unsigned int i,j;

    /* Only one video per display */
    for (i=0; i<NEXUS_NUM_DISPLAYS; ++i) {
        for (j=0; j<NEXUS_MAX_VIDEO_WINDOWS; ++j) {
            pSettings->display[i].window[j].used = ((i<2) && (j==0)) ? true: false;
        }
    }

#if NEXUS_HAS_VIDEO_DECODER && defined(NEXUS_PLATFORM_975635) /* 7563 doesn't do HEVC, 75635 does */
    pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH265] = true;
#endif

}

void NEXUS_Platform_P_SetSpecificOps(struct NEXUS_PlatformSpecificOps *pOps)
{
    pOps->modifyDefaultMemoryConfigurationSettings = nexus_p_modifyDefaultMemoryConfigurationSettings;
}

void NEXUS_Platform_P_GetPlatformHeapSettings(NEXUS_PlatformSettings *pSettings, unsigned boxMode)
{
    BSTD_UNUSED(boxMode);

    /* bmem=192M@64M for 256MB platforms or bmem=100M@28M for 128MB platforms which both use only one heap. */
    /* bmem=192M@64M bmem=192M@512M for the boards with 512 MB or more of memory. */

    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memcIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].subIndex = 0;
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].size = -1;   /* Use all the available memory in the region*/
    pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].memoryType = NEXUS_MemoryType_eFull;

    if (NEXUS_GetEnv("secure_heap") != NULL) { /* CABAC & CDB. */
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].subIndex = 0;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].size = 42 * 1024 * 1024;
        pSettings->heap[NEXUS_VIDEO_SECURE_HEAP].memoryType = NEXUS_MemoryType_eSecure;
    }

    if (g_platformMemory.memoryLayout.memc[0].size > 256*MB) {

        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].size = 0; /* Calculated */
        pSettings->heap[NEXUS_MEMC0_PICTURE_BUFFER_HEAP].memoryType = NEXUS_MemoryType_eDeviceOnly; /* Unmapped */

        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memcIndex = 0;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].subIndex = 1;
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].size = -1; /* Use all available space in resgion 1 */
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].memoryType = NEXUS_MemoryType_eApplication; /* Cached only */
        pSettings->heap[NEXUS_MEMC0_GRAPHICS_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    }
    else { /* If we don't have a separate graphics heap use the main heap. */
        pSettings->heap[NEXUS_MEMC0_MAIN_HEAP].heapType |= NEXUS_HEAP_TYPE_GRAPHICS;
    }

    pSettings->i2c[0].clock.type = NEXUS_GpioType_eSpecial;
    pSettings->i2c[0].clock.gpio = 0;
    pSettings->i2c[0].data.type = NEXUS_GpioType_eSpecial;
    pSettings->i2c[0].data.gpio = 1;
}

NEXUS_Error NEXUS_Platform_P_InitBoard(void)
{
    return NEXUS_SUCCESS;
}

void NEXUS_Platform_P_UninitBoard(void)
{
}

#if NEXUS_PLATFORM_7563_USFF2L

#define EXT_RFM_I2C_ADDR (0x65)
#define EXT_RFM_DATA_LENGTH 15 /* 4 RO + 11 R/W */
#define EXT_RFM_WRITE_ADDR 4

extern NEXUS_Error NEXUS_Platform_P_InitExternalRfm(const NEXUS_PlatformConfiguration *pConfig)
{
    uint8_t rfmConfig[EXT_RFM_DATA_LENGTH];
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned int i = 0;

    /* Make sure that platform initialised the correct I2C bus. */
    if ( pConfig->i2c[NEXUS_I2C_CHANNEL_EXT_RFM]) {

        rc = NEXUS_I2c_ReadNoAddr(pConfig->i2c[NEXUS_I2C_CHANNEL_EXT_RFM],
                                  EXT_RFM_I2C_ADDR, rfmConfig, EXT_RFM_DATA_LENGTH);

        if (rc != NEXUS_SUCCESS)
            BDBG_WRN(("Failed to read from RT500 RFM device - I2C bus %d - addr 0x%x",
                      NEXUS_I2C_CHANNEL_EXT_RFM, EXT_RFM_I2C_ADDR));

        /* Munge data as reading and writing appears to be byte swapped and bit reversed. */
        for (i = 0; i < EXT_RFM_DATA_LENGTH; ++i) {
            uint8_t tmp = 0x00;

            if (rfmConfig[i] & 0x01) tmp |= 0x80;
            if (rfmConfig[i] & 0x02) tmp |= 0x40;
            if (rfmConfig[i] & 0x04) tmp |= 0x20;
            if (rfmConfig[i] & 0x08) tmp |= 0x10;

            if (rfmConfig[i] & 0x10) tmp |= 0x08;
            if (rfmConfig[i] & 0x20) tmp |= 0x04;
            if (rfmConfig[i] & 0x40) tmp |= 0x02;
            if (rfmConfig[i] & 0x80) tmp |= 0x01;

            rfmConfig[i] = tmp;
        }

        /* Configure RT500 for RF loopthrough & disable modulator. */
        rfmConfig[EXT_RFM_WRITE_ADDR] = 0xFE;
        rfmConfig[EXT_RFM_WRITE_ADDR+1] = 0x9B;
        rfmConfig[EXT_RFM_WRITE_ADDR+9] |= 0x06;

        /*
         * If you want to enable the modulator the values below turn on the modulator
         * and default to 471.25MHz output frequency and PAL video format.
         * 0x96 0x3e 0x0f 0x0f 0x00 0xa8 0x88 0x34 0x29 0x4b 0xe8 0x70 0x05 0x2f 0x08
         */

        rc = NEXUS_I2c_Write(pConfig->i2c[NEXUS_I2C_CHANNEL_EXT_RFM],
                             EXT_RFM_I2C_ADDR, EXT_RFM_WRITE_ADDR,
                             &rfmConfig[EXT_RFM_WRITE_ADDR],
                             EXT_RFM_DATA_LENGTH-EXT_RFM_WRITE_ADDR);

        if (rc != NEXUS_SUCCESS)
            BDBG_WRN(("Failed to write RT500 RFM configuration - I2C bus %d - addr 0x%x",
                      NEXUS_I2C_CHANNEL_EXT_RFM, EXT_RFM_I2C_ADDR));
    }

    return NEXUS_SUCCESS;
}
#endif
