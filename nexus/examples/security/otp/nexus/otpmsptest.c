/******************************************************************************
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
 ******************************************************************************/

/* Nexus example app: OTP/MSP Access */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_message.h"
#include "nexus_memory.h"
#include "bstd.h"
#include "bkni.h"

#if NEXUS_SECURITY_API_VERSION == 2
int main(void)
{
    printf ("otpmsptest unsupported\n");
}

#else
#include "nexus_otpmsp.h"
#include "nexus_read_otp_id.h"
#ifdef NEXUS_HAS_PROGRAM_OTP_DATASECTION
 #include "crc/bhsm_crc.h"
#endif

BDBG_MODULE(OTPMSPTEST);

#define BHSM_MSP_OUTPUT_DATA_LEN        4



void Test_OTP_Read(void)
{
    NEXUS_Error        errCode = BERR_SUCCESS;
    unsigned int    OTPCode;
    unsigned int    keyType;
    unsigned int    i, j;
    NEXUS_ReadOtpIO readOtpIO;

    fprintf(stderr, "\nEnter OTP word to read: ");
    scanf (" %u", &OTPCode);

    fprintf(stderr, "\nEnter key type to read (0 for KeyA or not applicable): ");
    scanf (" %u", &keyType);
    errCode = NEXUS_Security_ReadOTP((NEXUS_OtpCmdReadRegister)OTPCode, (NEXUS_OtpKeyType)keyType, &readOtpIO);
    if (!errCode)
    {
        if ((NEXUS_OtpCmdReadRegister)OTPCode == NEXUS_OtpCmdReadRegister_eKeyID ||
            (NEXUS_OtpCmdReadRegister)OTPCode == NEXUS_OtpCmdReadRegister_eKeyHash )
        {
            j = 0;
        }
        else
        {
            j = 4;
        }
        fprintf(stderr, "\nOTP word for code %x and key type %x is ", OTPCode, keyType);
        for (i = 0; i < readOtpIO.otpKeyIdSize; i++)
        {
            fprintf(stderr, " %02X ", readOtpIO.otpKeyIdBuf[i + j]);
        }
        fprintf(stderr, "\n");
    }
    else
    {
        fprintf(stderr, "\nTest_OTP_Read failed. Error code: %x\n", errCode);
    }

    return;
}


unsigned int  readMSPBit(NEXUS_OtpCmdMsp bit)
{
    BERR_Code             errCode = BERR_SUCCESS;
    NEXUS_ReadMspParms    readMspParms;
    NEXUS_ReadMspIO        readMspIO;
    unsigned int         ulVal = 0;
    unsigned int        index = 0;

    printf("*****MSP Bit: %d*******\n", bit);
    readMspParms.readMspEnum    = bit;
    errCode = NEXUS_Security_ReadMSP(&readMspParms, &readMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ReadMSP() errCode: %x\n", errCode );
        return (0);
    }

    printf("****     MSP Data ");
    for (index = 0; index < readMspIO.mspDataSize; index++)
    {
        printf(" %x", readMspIO.mspDataBuf[index]);
    }
    printf("\n");
    printf("****Lock MSP Data ");
    for (index = 0; index < readMspIO.mspDataSize; index++)
    {
        printf(" %x", readMspIO.lockMspDataBuf[index]);
        ulVal = (ulVal << 8) | (readMspIO.mspDataBuf[index] & readMspIO.lockMspDataBuf[index]);
    }
    printf("\n");


    printf("MSP bit:  %d  bit value: %d\n", bit, ulVal );
    return (ulVal);
}



void programMSPBit(NEXUS_OtpCmdMsp bit)
{

    BERR_Code             errCode = BERR_SUCCESS;
    NEXUS_ReadMspParms    readMspParms;
    NEXUS_ReadMspIO        readMspIO;
    NEXUS_ProgramMspIO    progMspIO;
    unsigned int         ulVal;


    printf("*****MSP Bit: %d*******\n", bit);
    readMspParms.readMspEnum    = bit;
    errCode = NEXUS_Security_ReadMSP(&readMspParms, &readMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ReadMSP() errCode: %x\n", errCode );
        return;
    }

    ulVal = readMspIO.mspDataBuf[3];
    printf("**** %x  %x   %x   %x\n", readMspIO.mspDataBuf[0], readMspIO.mspDataBuf[1],
                                      readMspIO.mspDataBuf[2], readMspIO.mspDataBuf[3]);

    if(ulVal)
    {
        printf("Already programmed - bit value: %d\n", ulVal );
        return;
    }

#if 1
    progMspIO.progMode       = 0x010112;
    progMspIO.progMspEnum    = bit;
    progMspIO.dataBitLen     = 1;
    progMspIO.dataBitMask[0] = 0x0;
    progMspIO.dataBitMask[1] = 0x0;
    progMspIO.dataBitMask[2] = 0x0;
    progMspIO.dataBitMask[3] = 0x1;
    progMspIO.mspData[0]     = 0x0;
    progMspIO.mspData[1]     = 0x0;
    progMspIO.mspData[2]     = 0x0;
    progMspIO.mspData[3]     = 0x1;
    errCode = NEXUS_Security_ProgramMSP(&progMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ProgramMSP() errCode: %x\n", errCode );
        return;
    }
#endif

    readMspParms.readMspEnum    = bit;
    errCode = NEXUS_Security_ReadMSP(&readMspParms, &readMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ReadMSP() errCode: %x\n", errCode);
        return;
    }

    ulVal = readMspIO.mspDataBuf[3];
    printf("**** %x  %x   %x   %x\n", readMspIO.mspDataBuf[0], readMspIO.mspDataBuf[1],
                                      readMspIO.mspDataBuf[2], readMspIO.mspDataBuf[3]);

    if(ulVal)
    {
        printf("Programmed...Passed - bit value: %d\n", ulVal );
    }
    else
    {
        printf("Programmed...Failed - bit value: %d\n", ulVal );

    }

    return;
}




void Test_MSP_Program(void)

{
    unsigned int     MSPCode;

    fprintf(stderr, "\nEnter MSP Field to program: ");
    scanf (" %u", &MSPCode);
    if ((NEXUS_OtpCmdMsp)MSPCode >= NEXUS_OtpCmdMsp_eMax)
    {
        printf("\nInvalid MSP Field code!\n");
        return;
    }

    programMSPBit((NEXUS_OtpCmdMsp)MSPCode);

    return;
}



void Test_MSP_Read(void)

{
    unsigned int     MSPCode;

    fprintf(stderr, "\nEnter MSP Field to read: ");
    scanf (" %u", &MSPCode);
    if ((NEXUS_OtpCmdMsp)MSPCode >= NEXUS_OtpCmdMsp_eMax)
    {
        printf("\nInvalid MSP Field code!\n");
        return;
    }

    readMSPBit((NEXUS_OtpCmdMsp)MSPCode);

    return;
}


#ifdef NEXUS_HAS_PROGRAM_OTP_DATASECTION
static void otpCalculateCrc(
    unsigned char *data_section,
    unsigned char *crc
)
{
    /* The algorithm is propertary, please call that in the library. */
}


void Test_Program_DataSection(void)

{
    NEXUS_Error                errCode = BERR_SUCCESS;
    unsigned int            dataSectNum;
    unsigned int            i;
    NEXUS_ProgramDataSectIO    progDataSectIO;
    unsigned char progData[]    = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                                   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                                   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                                   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};
    unsigned char dataCRC[NEXUS_OTP_DATASECTION_CRC_LEN];

    fprintf(stderr, "\nEnter OTP Data Section to program: ");
    scanf (" %u", &dataSectNum);
    if ((NEXUS_OtpDataSection)dataSectNum >= NEXUS_OtpDataSection_eMax)
    {
        fprintf(stderr,"\nInvalid OTP Data Section number to program\n ");
        return;
    }

    progDataSectIO.progDsEnum = (NEXUS_OtpDataSection)dataSectNum;
    progDataSectIO.mode       = NEXUS_OTP_DATASECTIONPROG_MODE;

    /* fill up the data to be programmed */
    for (i = 0; i < NEXUS_OTP_DATASECTION_LEN; i++)
    {
        progDataSectIO.dataSectData[i] = progData[i];
    }

    /* Calculate CRC  */
    otpCalculateCrc(progData, dataCRC);

    /* load CRC */
    for (i = 0; i < NEXUS_OTP_DATASECTION_CRC_LEN; i++)
    {
        progDataSectIO.crc[i] = dataCRC[i];
    }

    errCode = NEXUS_Security_ProgramDataSect(&progDataSectIO);
    if (!errCode)
    {
        fprintf(stderr, "\nThe programming of OTP Data Section  %x is successful!\n", dataSectNum);
    }
    else
    {
        fprintf(stderr, "\nProgramming of data section  %x failed. Error code: %x\n",dataSectNum, errCode);
    }

    return;
}
#endif



void Test_Read_DataSection(void)

{
    NEXUS_Error                errCode = BERR_SUCCESS;
    unsigned int            dataSectNum;
    unsigned int            i;
    NEXUS_ReadDataSectIO    readDataSectIO;


    fprintf(stderr, "\nEnter OTP Data Section to read: ");
    scanf (" %u", &dataSectNum);
    if ((NEXUS_OtpDataSection)dataSectNum >= NEXUS_OtpDataSection_eMax)
    {
        fprintf(stderr,"\nInvalid OTP Data Section number to read\n ");
        return;
    }

    errCode = NEXUS_Security_ReadDataSect((NEXUS_OtpDataSection)dataSectNum, &readDataSectIO);
    if (!errCode)
    {
        fprintf(stderr, "\nThe content of OTP Data Section  %x is :\n", dataSectNum);
        for (i = 0; i < readDataSectIO.dataSectSize; i++)
        {
            fprintf(stderr, " %02X ", readDataSectIO.dataSectBuf[i]);
        }
        fprintf(stderr, "Size %d.\n\n\n", readDataSectIO.dataSectSize);
        fprintf(stderr, "The value is %s.\n\n\n", readDataSectIO.isShaOfData ? "Sha1 of the Data Section" : "Value of the Data Section");
    }
    else
    {
        fprintf(stderr, "\nTest_Read_DataSection failed. Error code: %x\n", errCode);
    }

    return;
}


void Test_Read_OTP_ID(void)

{
    NEXUS_Error                errCode = BERR_SUCCESS;
    unsigned int            OTP_ID_Num;
    unsigned int            i;
    NEXUS_OtpIdOutput        OTP_ID_out;


    while (1)
    {
        fprintf(stderr, "\nEnter OTP ID to read: \n");
        fprintf(stderr, " 0) OTP ID A\n");
        fprintf(stderr, " 1) OTP ID B\n");
        fprintf(stderr, " 2) OTP ID C\n");
        fprintf(stderr, " 3) OTP ID D\n");
        fprintf(stderr, " 4) OTP ID E\n");
        fprintf(stderr, " 5) OTP ID F\n");
        fprintf(stderr, " 6) OTP ID G\n");
        fprintf(stderr, " 7) OTP ID H\n");
        fprintf(stderr, " 9) Exit\n");
        scanf (" %u", &OTP_ID_Num);
        if (OTP_ID_Num == 9)
        {
            return;
        }

        if( (NEXUS_OtpIdType)OTP_ID_Num >= NEXUS_OtpIdType_eMax )
        {
            fprintf(stderr, "\nInvalid OTP ID requested [%d] \n", OTP_ID_Num );
            break;
        }

        errCode = NEXUS_Security_ReadOtpId((NEXUS_OtpIdType)OTP_ID_Num, &OTP_ID_out);
        if (!errCode)
        {
            fprintf(stderr, "\nThe OTP ID requested is :  ");
            for (i = 0; i < OTP_ID_out.size; i++)
            {
                fprintf(stderr, " %02X ", OTP_ID_out.otpId[i]);
            }
            fprintf(stderr, "\n");
        }
        else
        {
            fprintf(stderr, "\nTest_Read_Otp_Id() failed. Error code: %x\n", errCode);
        }
    }

    return;
}


void Test_EnableDRAMScrambling(void)
{

    NEXUS_OtpCmdMsp opCode = NEXUS_OtpCmdMsp_eForceDramScrambler;

    programMSPBit(opCode);

    return;
}


int main(void)
{

    unsigned int                  choice = 0;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;

    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );

    NEXUS_Platform_GetConfiguration( &platformConfig );

    while(1)
    {
        fprintf(stderr,"\n\n");
        fprintf(stderr,    "\nSelect OTP/MSP test to Test:\n");
          fprintf(stderr,    "0) Exit\n");
        fprintf(stderr,    "1) Read OTP\n");
        fprintf(stderr,    "2) Program MSP\n");
        fprintf(stderr,    "3) Read MSP\n");
        fprintf(stderr,    "4) Program Data Section\n");
        fprintf(stderr,    "5) Read Data Section\n");
        fprintf(stderr, "6) Read OTP ID Test\n");
        fprintf(stderr, "7) Enable DRAM Scrambling\n");
        fprintf(stderr,"Choice: ");
        scanf(" %u", &choice);
        fprintf(stderr,"\n\n");

        switch(choice)
        {
            case 0:
                goto BHSM_P_DONE_LABEL;
                break;
            case 1:
                Test_OTP_Read();
                break;
            case 2:
                Test_MSP_Program();
                break;
            case 3:
                Test_MSP_Read();
                break;
            case 4:
                #ifdef NEXUS_HAS_PROGRAM_OTP_DATASECTION
                Test_Program_DataSection();
                #else
                fprintf(stderr,"NEXUS_HAS_PROGRAM_OTP_DATASECTION not enabled\n");
                #endif
                break;
            case 5:
                Test_Read_DataSection();
                break;
            case 6:
                Test_Read_OTP_ID();
                break;
            case 9:
                Test_EnableDRAMScrambling();
                break;
            default:
                printf("Unknown choice...\n");
                break;
        }
    }

    BHSM_P_DONE_LABEL:

    return 0;
}
#endif
