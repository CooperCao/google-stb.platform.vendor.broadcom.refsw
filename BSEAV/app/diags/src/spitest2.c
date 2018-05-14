/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nexus_spi.h"
#include <stdio.h>
#include <stdlib.h>

#if NEXUS_HAS_SPI && NEXUS_NUM_SPI_CHANNELS

#define READ_SIZE 6
#define WRITE_SIZE 6
#define ADDR 12

/*

Notes:

   This was tested on a 97445 DBS board with 7445D0.
   Two steps:
   1) nexus frontend had to be initialized for some reason, otherwise the spi test won't work.  Suspect there is some
      gpio's that has to be done.
   2) Make sure to initialize nexus without the frontend in diags.c.  Otherwise, the spi channels would be opened twice.

*/
void bcmSpiTest (void)
{
    NEXUS_Error rc;
    NEXUS_SpiSettings spiSettings;
    uint8_t wData[WRITE_SIZE];
    uint8_t chipAddrBuf[2];
    uint8_t rData[READ_SIZE];
    NEXUS_SpiHandle spi[NEXUS_NUM_SPI_CHANNELS];
    int i,usableSpi=0xff;

    NEXUS_Spi_GetDefaultSettings(&spiSettings);
    spiSettings.clockActiveLow = false;
    spiSettings.baud = 6750000;

    for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++) {
        spi[i] = NEXUS_Spi_Open(i, &spiSettings);
        printf(" spi[i=%d]=0x%8x \n" , i, spi[i] );
        if ( spi[i] != NULL && usableSpi == 0xff ) usableSpi=i;
    }

    chipAddrBuf[0] = 0x40;
    chipAddrBuf[1] = ADDR;

    printf("usableSpi=%d\n" , usableSpi );
    if ( usableSpi != 0xff /* spi[usableSpi] != NULL */ ) {
        rc = NEXUS_Spi_Read(spi[usableSpi], &chipAddrBuf[0], &rData[0], READ_SIZE);
        BDBG_ASSERT(!rc);

    /*
    for (i=0; i<READ_SIZE; i++) {
        printf("%d:  0x%02x\n", i, rData[i]);
    }
    */

        wData[0] = 0x41;
        wData[1] = ADDR;
        wData[2] = 0x01;
        wData[3] = 0x23;
        wData[4] = 0x45;
        wData[5] = 0x67;

        rc = NEXUS_Spi_Write(spi[usableSpi], &wData[0], WRITE_SIZE);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Spi_Read(spi[usableSpi], &chipAddrBuf[0], &rData[0], READ_SIZE);
        BDBG_ASSERT(!rc);

        for (i=0; i<READ_SIZE; i++) {
            printf("%d:  0x%02x\n", i, rData[i]);
        }

        for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++) {
            if ( spi[i] ) NEXUS_Spi_Close(spi[i]);
        }
    }
    else {
        printf("Failed to open spi device !!!\n" );
        printf("\n Please re-run diags with \"nexus diags -notuner\" to see if it becomes available \n" );
        printf("\n\tPress a key to continue \n");
        getchar();
        for (i=0; i<NEXUS_NUM_SPI_CHANNELS; i++) {
            if ( spi[i] ) NEXUS_Spi_Close(spi[i]);
        }
    }
    return;
}
#else
#include <stdio.h>
void bcmSpiTest (void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
