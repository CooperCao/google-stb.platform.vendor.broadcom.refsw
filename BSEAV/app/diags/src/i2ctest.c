/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 ***************************************************************************/
#include "test.h"

#ifdef DIAGS_I2C_TEST

#include <stdio.h>
#include <string.h>
#include "bstd.h"
#include "nexus_platform.h"
#include "prompt.h"

#define MAX_I2C_TEST_SIZE   512

/***********************************************************************
 *                       Global Variables
 ***********************************************************************/

/***********************************************************************
 *                      External References
 ***********************************************************************/
#if __cplusplus
extern "C" {
#endif


#if __cplusplus
}
#endif

/***********************************************************************
 *                      Function Prototypes
 ***********************************************************************/

#if __cplusplus
extern "C" {
#endif

void bcmI2cTest (void);

#if __cplusplus
}
#endif

 /***********************************************************************
 *
 *  bcmI2cClockRate()
 * 
 *  Get clock rate.
 *
 ***********************************************************************/
NEXUS_I2cClockRate bcmI2cClockRate(void)
{
    NEXUS_I2cClockRate val;
    while (1) {
        printf("Enter one of the following values:  %d, %d, %d, %d, %d, %d, %d, %d\n", 
            NEXUS_I2cClockRate_e47Khz,
            NEXUS_I2cClockRate_e50Khz,
            NEXUS_I2cClockRate_e93Khz,
            NEXUS_I2cClockRate_e100Khz,
            NEXUS_I2cClockRate_e187Khz,
            NEXUS_I2cClockRate_e200Khz,
            NEXUS_I2cClockRate_e375Khz,
            NEXUS_I2cClockRate_e400Khz);
        scanf("%d", &val);
        if ((val != NEXUS_I2cClockRate_e47Khz) &&
            (val != NEXUS_I2cClockRate_e50Khz) &&
            (val != NEXUS_I2cClockRate_e93Khz) &&
            (val != NEXUS_I2cClockRate_e100Khz) &&
            (val != NEXUS_I2cClockRate_e187Khz) &&
            (val != NEXUS_I2cClockRate_e200Khz) &&
            (val != NEXUS_I2cClockRate_e375Khz) &&
            (val != NEXUS_I2cClockRate_e400Khz))
            printf("invalid choice\n");
        else
            break;
    }
    return val;
}

 /***********************************************************************
 *
 *  bcmI2cTest()
 * 
 *  I2C Test
 *
 ***********************************************************************/
void bcmI2cTest (void)
{
    char                str[20];
    uint32_t            i, val, chan = 0;
    static uint16_t     chipAddr = 0;
    static bool         ack=1;
    uint8_t             segment, data[MAX_I2C_TEST_SIZE];
    uint32_t            subAddr=0;
    int32_t             subAddrBytes;
    size_t              length;
    BERR_Code           error = BERR_SUCCESS;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_I2cHandle hI2cHandle;
    NEXUS_I2cSettings settings;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    hI2cHandle = platformConfig.i2c[chan];

    while (1)
    {
        NEXUS_I2c_GetSettings(hI2cHandle, &settings);

        printf("=================\n");
        printf("  I2C Test Menu  \n");
        printf("=================\n");
        printf("    0) Exit\n");
        printf("    1) Select I2C channel, current channel is %d\n", chan);
        printf("    2) Change default chip address, current chip address is %02x\n", chipAddr);
        printf("    3) Change ignore ack setting, current setting is: %s\n", ack?"do not ignore ack":"ignore ack");
#ifdef SUPPORT_4_BYTE_XFER_MODE
        printf("    4) Toggle 4 byte xfer.  Current setting:  %s\n", settings.fourByteXferMode?"4 byte xfer mode":"normal xfer mode");
#endif
        printf("    5) Toggle hw/sw i2c.  Current setting:  %s\n", settings.softI2c?"soft i2c":"hard i2c");
        printf("    6) Change clock speed.  Current setting:  %d Hz\n", settings.clockRate);
        printf("    7) I2C read\n");
        printf("    8) I2C write\n");
        printf("    9) I2C write/read\n");
        printf("    a) EDDC read\n");
        printf("    b) EDDC write\n");
        printf("    c) I2C write/read/compare 64 bytes\n");

        switch(PromptChar())
        {
            case '0':
                return;

            case '1':
                printf("\n");
                while (1)
                {
                    printf("Enter valid I2C channel number from 0 to %d:  ", NEXUS_MAX_I2C_CHANNELS-1);
                    fflush(stdout);
                    chan = NoPrompt();
                    printf("%d", chan);
                    if (chan > NEXUS_MAX_I2C_CHANNELS-1 || !platformConfig.i2c[chan])
                        printf(" Invalid I2C channel\n");
                    else
                        break;
                }
                printf("\n");
                hI2cHandle = platformConfig.i2c[chan];
                break;

            case '2':
                printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                scanf("%x", &val);
                chipAddr = (uint16_t)val;
                break;      

            case '3':
                printf("\n");
                while (1)
                {
                    printf("Enter 0=ignore ack or 1=check for ack:  ");
                    fflush(stdout);
                    ack = NoPrompt();
                    printf("%d", ack);
                    if ((ack!=0) && (ack!=1))
                        printf(" Invalid selection\n");
                    else
                        break;
                }
                printf("\n");
                break;

            case '4':
                    #ifdef SUPPORT_4_BYTE_XFER_MODE
                    {
                        /* Toggle 4 byte xfer mode */
                        bool prevSoftI2c;
                        bool prevFourByteXferMode;
                        NEXUS_I2cClockRate prevClockRate;
                        NEXUS_I2c_GetSettings(hI2cHandle, &settings);
                        prevSoftI2c = settings.softI2c;
                        prevClockRate = settings.clockRate;
                        prevFourByteXferMode = settings.fourByteXferMode;
                        NEXUS_I2c_Close(hI2cHandle);
                        NEXUS_I2c_GetDefaultSettings(&settings);
                        settings.softI2c = prevSoftI2c;
                        settings.clockRate = prevClockRate;
                        settings.fourByteXferMode = !prevFourByteXferMode;
                        hI2cHandle = platformConfig.i2c[chan] = NEXUS_I2c_Open(chan, &settings);
                    }
                    #else
                        printf("4 byte transfer mode not supported on this chip\n");
                    #endif
                break;
                
            case '5':
                {
                    /* Toggle hw/sw i2c */
                    bool prevSoftI2c;
                    #ifdef SUPPORT_4_BYTE_XFER_MODE
                        bool prevFourByteXferMode;
                    #endif
                    NEXUS_I2cClockRate prevClockRate;
                    NEXUS_I2c_GetSettings(hI2cHandle, &settings);
                    prevSoftI2c = settings.softI2c;
                    prevClockRate = settings.clockRate;
                    #ifdef SUPPORT_4_BYTE_XFER_MODE
                        prevFourByteXferMode = settings.fourByteXferMode;
                    #endif
                    NEXUS_I2c_Close(hI2cHandle);
                    NEXUS_I2c_GetDefaultSettings(&settings);
                    settings.softI2c = !prevSoftI2c;
                    settings.clockRate = prevClockRate;
                    #ifdef SUPPORT_4_BYTE_XFER_MODE
                        settings.fourByteXferMode = prevFourByteXferMode;
                    #endif
                    hI2cHandle = platformConfig.i2c[chan] = NEXUS_I2c_Open(chan, &settings);
                }
                break;
                
            case '6':
                NEXUS_I2c_GetSettings(hI2cHandle, &settings);
                settings.clockRate = bcmI2cClockRate();
                NEXUS_I2c_SetSettings(hI2cHandle, &settings);
                break;
                
            case '7':
                if (!chipAddr) {
                    printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                    scanf("%x", &val);
                    chipAddr = (uint16_t)val;
                }
                else {
                    printf("\nChip address = %02x\n", chipAddr);
                }

                while (1) {
                    printf("Sub Address Bytes? (0-3):  ");
                    fflush(stdout);
                    subAddrBytes = NoPrompt();
                    printf("%d", subAddrBytes);
                    if ((0<=subAddrBytes) && (subAddrBytes<=3))
                        break;
                    else
                        printf(" invalid number of sub address bytes\n");
                }

                if ((subAddrBytes == 1) || (subAddrBytes == 2) || (subAddrBytes == 3)) {
                    printf("\nEnter the sub address (in hex): ");
                    scanf("%x", &val);
                    subAddr = (uint32_t)val;
                }

                printf("\nEnter the number of bytes to read (maximum %d bytes): ", MAX_I2C_TEST_SIZE);
                scanf("%d", &val);
                length = (size_t)val;
                printf("\n");

                switch (subAddrBytes) {
                    case 0:
                        if (ack)
                            error = NEXUS_I2c_ReadNoAddr (hI2cHandle, chipAddr, &data[0], length);
                        else
                            error = NEXUS_I2c_ReadNoAddrNoAck (hI2cHandle, chipAddr, &data[0], length);
                        break;

                    case 1:
                        if (ack)
                            error = NEXUS_I2c_Read (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        else
                            error = NEXUS_I2c_ReadNoAck (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    case 2:
                        error = NEXUS_I2c_ReadA16 (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    case 3:
                        error = NEXUS_I2c_ReadA24 (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    default:
                        error = BERR_INVALID_PARAMETER;
                        break;
                }

                if (error != BERR_SUCCESS) {
                    printf("I2C read fails!\n");
                }
                else {
                    printf("\nRead Data: \n");
                    printf("%s    Value\n", subAddrBytes ? "Subaddr" : "Offset");
                    for (i=0; i<length; i++)
                        printf("  %02x        %02x\n", subAddrBytes ? (subAddr+i) : i, data[i]);
                    printf("\n");
                }
                printf("\n");
                break;

            case '8':
                if (!chipAddr) {
                    printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                    scanf("%x", &val);
                    chipAddr = (uint16_t)val;
                }
                else {
                    printf("\nChip address = %02x\n", chipAddr);
                }

                while (1) {
                    printf("Sub Address Bytes? (0-3):  ");
                    fflush(stdout);
                    subAddrBytes = NoPrompt();
                    printf("%d", subAddrBytes);
                    if ((0<=subAddrBytes) && (subAddrBytes<=3))
                        break;
                    else
                        printf(" invalid number of sub address bytes\n");
                }

                if ((subAddrBytes == 1) || (subAddrBytes == 2) || (subAddrBytes == 3)) {
                    printf("\nEnter the sub address (in hex): ");
                    subAddr = NoPromptHex();
                }

                printf("\nEnter the number of bytes to write (maximum %d bytes): ", MAX_I2C_TEST_SIZE);
                length = NoPrompt();

                for (i=0; i<length; i++) {
                    printf("\nEnter value to write (in hex): ");
                    scanf("%x", &val);
                    data[i] = (uint8_t)val;
                }
                printf("\n");

                switch (subAddrBytes) {
                    case 0:
                        if (ack)
                            error = NEXUS_I2c_WriteNoAddr (hI2cHandle, chipAddr, &data[0], length);
                        else
                            error = NEXUS_I2c_WriteNoAddrNoAck (hI2cHandle, chipAddr, &data[0], length);
                        break;

                    case 1:
                        if (ack)
                            error = NEXUS_I2c_Write (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        else
                            error = NEXUS_I2c_WriteNoAck (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    case 2:
                        error = NEXUS_I2c_WriteA16 (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    case 3:
                        error = NEXUS_I2c_WriteA24 (hI2cHandle, chipAddr, subAddr, &data[0], length);
                        break;

                    default:
                        error = BERR_INVALID_PARAMETER;
                        break;
                }

                if (error != BERR_SUCCESS) {
                    printf("I2C write fails!\n");
                }
                printf("\n");
                break;

            case '9':
                {
                    uint8_t wbuf[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
                    uint8_t rbuf[16]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
                    
                    if (!chipAddr)
                    {
                        printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                        scanf("%x", &val);
                        chipAddr = (uint16_t)val;
                    }
                    else
                    {
                        printf("\nChip address = %02x\n", chipAddr);
                    }
                    
                    while (1)
                    {
                        printf("Sub Address Bytes? (0-3):  ");
                        fflush(stdout);
                        subAddrBytes = NoPrompt();
                        printf("%d", subAddrBytes);
                        if ((0<=subAddrBytes) && (subAddrBytes<=3))
                            break;
                        else
                            printf(" invalid number of sub address bytes\n");
                    }

                    if ((subAddrBytes == 1) || (subAddrBytes == 2) || (subAddrBytes == 3))
                    {
                        printf("\nEnter the sub address (in hex): ");
                        scanf("%x", &val);
                        subAddr = (uint32_t)val;
                    }

                    switch (subAddrBytes)
                    {
                        case 0:
                            #ifdef SUPPORT_WRITE_READ_NO_ADDR
                                error = NEXUS_I2c_WriteReadNoAddr (hI2cHandle, chipAddr, &wbuf[0], sizeof(wbuf), &rbuf[0], sizeof(rbuf));
                            #else
                                printf("NEXUS_I2c_WriteReadNoAddr not supported\n");
                            #endif
                            break;
                    
                        case 1:
                            error = NEXUS_I2c_WriteRead (hI2cHandle, chipAddr, subAddr, &wbuf[0], sizeof(wbuf), &rbuf[0], sizeof(rbuf));
                            break;
                    }

                    if (error != BERR_SUCCESS)
                    {
                        printf("I2C write/read fails!\n");
                    }
                    else
                    {
                        for (i=0; i<16; i++)
                        {
                            printf("rbuf[%d]:  0x%x\n", i, rbuf[i]);
                        }
                    }
                    printf("\n");
                }
                break;
            
            case 'a':
                chan = 3;                               /* HDMI uses I2C channel 4 */
                hI2cHandle = platformConfig.i2c[chan];
                if (!chipAddr)
                {
                    printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                    scanf("%x", &val);
                    chipAddr = (uint16_t)val;
                }
                else
                {
                    printf("\nChip address = %02x\n", chipAddr);
                }

                printf("\nEnter the segment (in hex): ");
                scanf("%x", &val);
                segment = (uint8_t)val;

                printf("\nEnter the sub address to read (in hex): ");
                scanf("%x", &val);
                subAddr = (uint8_t)val;

                printf("\nEnter the number of bytes to read (maximum %d bytes): ", MAX_I2C_TEST_SIZE);
                scanf(str, "%d", &val);
                length = (size_t)val;
                printf("\n");

                error = NEXUS_I2c_ReadEDDC (hI2cHandle, chipAddr, segment, subAddr, &data[0], length);

                if (error != BERR_SUCCESS)
                {
                    printf("EDDC read fails!\n");
                }
                else
                {
                    printf("\nRead Data: \n");
                    printf("Offset    Value\n");
                    for (i=0; i<length; i++)
                        printf("  %02x        %02x\n", subAddr+i, data[i]);
                }
                printf("\n");
                break;

            case 'b':
                chan = 3;                               /* HDMI uses I2C channel 4 */
                hI2cHandle = platformConfig.i2c[chan];
                if (!chipAddr)
                {
                    printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                    scanf("%x", &val);
                    chipAddr = (uint16_t)val;
                }
                else
                {
                    printf("\nChip address = %02x\n", chipAddr);
                }

                printf("\nEnter the segment (in hex): ");
                scanf("%x", &val);
                segment = (uint8_t)val;

                printf("\nEnter the sub address (in hex): ");
                scanf("%x", &val);
                subAddr = (uint8_t)val;

                printf("\nEnter the number of bytes to write (maximum %d bytes): ", MAX_I2C_TEST_SIZE);
                scanf("%d", &val);
                length = (size_t)val;
                printf("\n");

                for (i=0; i<length; i++)
                {
                    printf("\nEnter value to write (in hex): ");
                    scanf("%x", &val);
                    data[i] = (uint8_t)val;
                }
                printf("\n");

                error = NEXUS_I2c_WriteEDDC (hI2cHandle, chipAddr, segment, subAddr, &data[0], length);

                if (error != BERR_SUCCESS)
                {
                    printf("EDDC write fails!\n");
                }
                printf("\n");
                break;

            case 'c':
                {
#if 1
                    uint8_t wbuf[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
                                    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
                                    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                                    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f};
                    uint8_t rbuf[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#else
                    uint8_t wbuf[]={0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                                    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
                                    
                    uint8_t rbuf[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
#endif                                      
                    
                    if (!chipAddr)
                    {
                        printf("\nEnter the 7-bit chip address (unshifted, in hex): ");
                        scanf("%x", &val);
                        chipAddr = (uint16_t)val;
                    }
                    else
                    {
                        printf("\nChip address = %02x\n", chipAddr);
                    }
                    
                    while (1)
                    {
                        printf("Sub Address Bytes? (0-3):  ");
                        fflush(stdout);
                        subAddrBytes = NoPrompt();
                        printf("%d", subAddrBytes);
                        if ((0<=subAddrBytes) && (subAddrBytes<=3))
                            break;
                        else
                            printf(" invalid number of sub address bytes\n");
                    }

                    if ((subAddrBytes == 1) || (subAddrBytes == 2) || (subAddrBytes == 3))
                    {
                        printf("\nEnter the sub address (in hex): ");
                        scanf("%x", &val);
                        subAddr = (uint32_t)val;
                    }

                    switch (subAddrBytes)
                    {
                        case 0:
                            error = NEXUS_I2c_WriteNoAddr (hI2cHandle, chipAddr, &wbuf[0], sizeof(wbuf));
                            break;
                    
                        case 1:
                            error = NEXUS_I2c_Write (hI2cHandle, chipAddr, subAddr, &wbuf[0], sizeof(wbuf));
                            break;
                            
                        case 2:
                            error = NEXUS_I2c_WriteA16 (hI2cHandle, chipAddr, subAddr, &wbuf[0], sizeof(wbuf));
                            break;

                        case 3:
                            error = NEXUS_I2c_WriteA24 (hI2cHandle, chipAddr, subAddr, &wbuf[0], sizeof(wbuf));
                            break;
                    }

                    if (error != BERR_SUCCESS)
                    {
                        printf("I2C write fails!\n");
                    }
                    printf("\n");

                    switch (subAddrBytes)
                    {
                        case 0:
                            error = NEXUS_I2c_ReadNoAddr (hI2cHandle, chipAddr, &rbuf[0], sizeof(rbuf));
                            break;
                    
                        case 1:
                            error = NEXUS_I2c_Read (hI2cHandle, chipAddr, subAddr, &rbuf[0], sizeof(rbuf));
                            break;
                            
                        case 2:
                            error = NEXUS_I2c_ReadA16 (hI2cHandle, chipAddr, subAddr, &rbuf[0], sizeof(rbuf));
                            break;

                        case 3:
                            error = NEXUS_I2c_ReadA24 (hI2cHandle, chipAddr, subAddr, &rbuf[0], sizeof(rbuf));
                            break;
                    }

                    if (error != BERR_SUCCESS)
                    {
                        printf("I2C read fails!\n");
                    }
                    printf("\n");
                    
                    {
                        unsigned int i;
                        for (i=0; i<sizeof(rbuf); i++)
                        {
                            if (wbuf[i] != rbuf[i]) printf("miscompare at i=%d, wbuf=0x%x, rbuf=0x%x\n", i, wbuf[i], rbuf[i]);
                        }
                    }
                }
                break;

            default:
                printf("Please enter a valid choice\n");
                break;
        }
    }
}

#else

#include <stdio.h>

void bcmI2cTest (void)
{
    printf("Not enabled\n");
}

#endif /* DIAGS_I2C_TEST */
