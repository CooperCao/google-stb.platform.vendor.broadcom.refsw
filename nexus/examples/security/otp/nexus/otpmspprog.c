/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

/* Nexus example app: OTP/MSP Access */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "nexus_platform.h"

#if NEXUS_SECURITY_API_VERSION == 2
int main(int argc, char  *argv[])
{
    printf ("otpmspprog unsupported\n");
    return 0;
}
#else

#include "nexus_otpmsp.h"
#include "nexus_read_otp_id.h"
#ifdef NEXUS_HAS_PROGRAM_OTP_DATASECTION
#include "crc/bhsm_crc.h"
#endif

#define BHSM_MSP_OUTPUT_DATA_LEN        4



void programMSPField(
    NEXUS_OtpCmdMsp mspField,
    unsigned long   bits,
    unsigned long   mspVal,
    unsigned long   mspMask
)
{

    BERR_Code             errCode = BERR_SUCCESS;
    NEXUS_ReadMspParms    readMspParms;
    NEXUS_ReadMspIO        readMspIO;
    NEXUS_ProgramMspIO    progMspIO;
    unsigned int         ulVal;
    unsigned int        index;


    printf("*****MSP Field: %d*******\n", mspField);
    readMspParms.readMspEnum    = mspField;
    errCode = NEXUS_Security_ReadMSP(&readMspParms, &readMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ReadMSP() errCode: %x\n", errCode);
        return;
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


    printf("MSP Field:  %d  Field value before programming : %x\n", mspField, ulVal );

    progMspIO.progMode       = 0x010112;
    progMspIO.progMspEnum    = mspField;
    progMspIO.dataBitLen     = bits;
    for (index = 0; index < 4; index++)
    {
        progMspIO.dataBitMask[index] = (unsigned char)(mspMask >> (8 * (3 - index)) & 0xFF);
        progMspIO.mspData[index]     = (unsigned char)(mspVal  >> (8 * (3 - index)) & 0xFF);
    }
    errCode = NEXUS_Security_ProgramMSP(&progMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ProgramMSP() errCode: %x\n", errCode );
        return;
    }

    readMspParms.readMspEnum    = mspField;
    errCode = NEXUS_Security_ReadMSP(&readMspParms, &readMspIO);
    if (errCode != 0)
    {
        printf("NEXUS_Security_ReadMSP() errCode: %x\n", errCode);
        return;
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


    printf("MSP Field:  %d  Field value after programming : %x\n", mspField, ulVal );

    return;
}




#ifdef NEXUS_HAS_PROGRAM_OTP_DATASECTION
void Program_DataSection(
    unsigned int    dataSectNum,
    FILE            *dataFile
)
{
    NEXUS_Error             errCode = BERR_SUCCESS;
    unsigned int            i;
    NEXUS_ProgramDataSectIO progDataSectIO;
    unsigned char           progData[NEXUS_OTP_DATASECTION_LEN];
    unsigned char           dataCRC[NEXUS_OTP_DATASECTION_CRC_LEN];
    int                     size;

    if ((NEXUS_OtpDataSection)dataSectNum >= NEXUS_OtpDataSection_eMax)
    {
        fprintf(stderr,"\nInvalid OTP Data Section number to program\n ");
        return;
    }

    progDataSectIO.progDsEnum = (NEXUS_OtpDataSection)dataSectNum;
    progDataSectIO.mode       = NEXUS_OTP_DATASECTIONPROG_MODE;

    /* Read the data to be programmed from dataFile*/
    size = fread(progData, 1, NEXUS_OTP_DATASECTION_LEN, dataFile);
    if (size < NEXUS_OTP_DATASECTION_LEN)
    {
        printf( "The data file does not have sufficient bytes (%d) for the data section.\n", NEXUS_OTP_DATASECTION_LEN );
        return;
    }

    for (i = 0; i < NEXUS_OTP_DATASECTION_LEN; i++)
    {
        progDataSectIO.dataSectData[i] = progData[i];
    }

    /* Calculate CRC  */
    BHSM_CalculateCrc(progData, dataCRC);

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

int main(int argc, char  *argv[])
{

    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    unsigned int                opCode;
    unsigned int                MSPEnum;
    unsigned int                MSPBits;
    unsigned long               MSPVal;
    unsigned long               MSPMask;
    unsigned int                dataSectNum;
    FILE                        *dataFile;


    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init( &platformSettings );
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (argc < 2)
    {
        printf( "Please provide the following parameters\n");
        printf( "%s  0 <MSP Field to program>    <Bits in Field> <MSP Field Value in Hex> <Field Mask in Hex>\n", argv[0]);
        printf( "%s  1 <Data Section to program> <Data File> \n", argv[0]);
           return 0;
    }

    opCode = atoi(argv[1]);
    switch (opCode)
    {
        case  0 :                    /* Program MSP Field */

            if (argc < 6)
            {
                printf( "Please provide the following parameters\n");
                printf( "%s  0 <MSP Field to program>     <Bits in Field> <MSP Field Value in Hex> <Field Mask in Hex>\n", argv[0]);
                printf( "e.g.  %s 0 43 8 0x2B 0xFF\n", argv[0]);
                return 0;
            }
            MSPEnum  = atoi(argv[2]);
            MSPBits  = atoi(argv[3]);
            MSPVal   = strtoul(argv[4], NULL, 16);
            MSPMask  = strtoul(argv[5], NULL, 16);

            programMSPField((NEXUS_OtpCmdMsp)MSPEnum, MSPBits, MSPVal, MSPMask);

            break;

        case  1 :                    /* Program Data Section */
            if (argc < 4)
            {
                printf( "Please provide the following parameters\n");
                printf( "%s  1 <Data Section to program> <Data File> \n", argv[0]);
                printf( "e.g.  %s 1 2 datain.bin\n", argv[0]);
                return 0;
            }
            dataSectNum = atoi(argv[2]);
            dataFile    = fopen(argv[3],"rb");
            if ( ( dataFile == 0 ) )
            {
                printf( "Error: can not open data input file %s for Data Section programming\n", argv[3] );
                return 0;
            }
            break;

        default :
            printf("Invalid Op Code!\n");
            return 0;

    }

    return 0;
}
#endif
