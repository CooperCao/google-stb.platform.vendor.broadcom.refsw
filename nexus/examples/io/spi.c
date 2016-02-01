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
#if NEXUS_HAS_SPI
#include "bstd.h"
#include "bkni.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_spi.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    NEXUS_Error rc;
    NEXUS_SpiSettings spiSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_SpiHandle keypadspi;
    uint8_t wData[2];
    uint8_t chipAddrBuf[2];
    uint8_t rData[2];

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    NEXUS_Spi_GetDefaultSettings(&spiSettings);
#if 0
    /* These settings need to be vendor specific. Please populate spiSettings accordingly */    
    spiSettings.clockActiveLow = true;
    spiSettings.baud = 333000;      /* 333KHz */
    spiSettings.interruptMode = false;
    spiSettings.lastByteContinueEnable = true;  
    spiSettings.txLeadingCapFalling = true; 
    spiSettings.useUserDtlAndDsclk = true;  
    spiSettings.dtl = 100;  
#endif

    keypadspi = NEXUS_Spi_Open(0, &spiSettings);
    
    /* First Write 2 bytes into SPI bus. 
       The values here and the number of bytes need to be vendor specific  
    */
    wData[0] = 0xaa;
    wData[1] = 0xbb;
    rc = NEXUS_Spi_Write(keypadspi, &wData[0], 2);
    BDBG_ASSERT(!rc);

    /* Read 2 bytes from the SPI bus. The data read will be in rData buffer. 
       chipAddrBuf contains Dummy Values to follow the SPI Read Protocol
     */
    chipAddrBuf[0] = 0x00;
    chipAddrBuf[1] = 0xff;  

    rc = NEXUS_Spi_Read(keypadspi, &chipAddrBuf[0], &rData[0], 2);
    BDBG_ASSERT(!rc);
    
    NEXUS_Spi_Close(keypadspi);

    NEXUS_Platform_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
