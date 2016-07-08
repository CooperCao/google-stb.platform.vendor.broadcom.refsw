/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

// RSA_Storage_Generation.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include "GenerateFiles.h"

#define DS_SIZE 32
#define FILENAME_LEN 300

#if 0
static void Swap(unsigned char * pBuffer)
{
    unsigned int i;
    for (i = 0; i<(DS_SIZE / 2); i += 2)
    {
        unsigned char temp;

        temp = pBuffer[i];
        pBuffer[i] = pBuffer[DS_SIZE - 2 - i];
        pBuffer[DS_SIZE - 2 - i] = temp;

        temp = pBuffer[i + 1];
        pBuffer[i + 1] = pBuffer[DS_SIZE - 1 - i];
        pBuffer[DS_SIZE - 1 - i] = temp;

    }
}


void Generate_BBS_script (unsigned char *ds,
                                 unsigned int ds_id,
                                 unsigned char * crc_ds,
                                 char * file_name,
                                 unsigned int chipID)
{
    FILE * pOut = fopen (file_name, "w");

    fprintf(pOut, BBS_HEADER_1, chipID);
    fprintf(pOut, BBS_HEADER_2);
    fprintf(pOut, BBS_HEADER_3, ds_id);

    Swap (ds);
    fprintf(pOut, "OtpDsProg = Array( _\n&h00000010&, _ \n&h00000066&, _ \n&habcdef00&, _ \n&he055aa1F&, _ \n&h789a002C&, _ \n&h%08x&, _ \n", ds_id);

    for (int i=0; i<(DS_SIZE/4); i++ )
    {
        fprintf(pOut, "&h%02x", ds[i*4]);
        fprintf(pOut, "%02x", ds[i*4+1]);
        fprintf(pOut, "%02x", ds[i*4+2]);
        fprintf(pOut, "%02x&, _\n", ds[i*4+3]);
    }
    fprintf(pOut, "&h00010112&, _\n");
    fprintf(pOut, "&h%02x%02x%02x%02x& )\n\n", crc_ds[0], crc_ds[1], crc_ds[2], crc_ds[3]);

    fprintf(pOut, BBS_HEADER_FOOTER_0);
    fprintf(pOut, BBS_HEADER_FOOTER_1);
    fprintf(pOut, BBS_HEADER_FOOTER_2);
    fprintf(pOut, BBS_HEADER_FOOTER_3);
    fprintf(pOut, BBS_HEADER_FOOTER_4);


    fclose (pOut);
}
#endif

void Generate_C_file(unsigned char *data,
    unsigned int size,
    char * file_name,
    unsigned int RegionId,
    unsigned int Append)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;

/*
this causes problems because file path may include dots before file extension, e.g.
/projects/stbdevrefsw/jtna/refsw/git0//nexus/../obj.97445/BSEAV/lib/security/fw_img_sign/arm-linux/nexus_security_regver_signatures.h
*/
#if 0
    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".h");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".h");
    }
#else
    strcpy(FileName, file_name);
#endif

    if (1 == Append)
    {
        pOut = fopen(FileName, "a");
    }
    else
    {
        pOut = fopen(FileName, "w");
    }

    if (0 == Append)
    {
        fprintf(pOut, COPYRIGHT_NOTICE);
    }
    fprintf(pOut, "#define SIGNATURE_REGION_0x%02X\n", RegionId);
    fprintf(pOut, "const uint8_t signatureRegion_0x%02X [] = {  0x%02X,", RegionId, data[0]);
    for (i = 1; i < size - 1; i++)
    {
        if ( (i%16) ==0)
            fprintf(pOut, "\\\n                                          ");
        fprintf(pOut, " 0x%02X,", data[i]);
    }

    fprintf(pOut, " 0x%02X };\n\n\n", data[size-1]);

    fclose(pOut);
}

void Generate_C_Array(unsigned char *data_file_name,
    char * file_name,
    char * var_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;
    unsigned char Byte;

    FILE *fp = fopen(data_file_name,"rb");

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",data_file_name);
        exit(1);
    }
    else
    {
        pOut = fopen(file_name, "w");

        printf("Creating C file %s.\n", file_name);

        fprintf(pOut, COPYRIGHT_NOTICE);
        fprintf(pOut, "static unsigned char %s[] =\n{", var_name);

        while(fread(&Byte, 1, 1, fp))
        {
            if ( (i%16) ==0)
                fprintf(pOut, "\n   ");
            fprintf(pOut, " 0x%02x,", Byte);
            i++;
        }
        fprintf(pOut, "\n};\n");

    }

    fclose(fp);
    fclose(pOut);
}


void Generate_AVD_C_file(unsigned char *data_file_name,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;
    unsigned int AvdCount = 0;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".c");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".c");
    }


    FILE *fp = fopen(data_file_name,"rb");
    Sig_u Signature;

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",data_file_name);
        exit(1);
    }
    else
    {
        pOut = fopen(FileName, "w");

        printf("Creating C header file %s.\n", FileName);

        fprintf(pOut, COPYRIGHT_NOTICE);
        fprintf(pOut, AVD_SIGNATURE_HEADER_1);

        while(fread(&Signature.Words, sizeof(Sig_u), 1, fp))
        {
            if(AvdCount%3 == 0)
            {
                fprintf(pOut, AVD_SIGNATURE_HEADER_2, AvdCount/3);
                fprintf(pOut, "\t{\n\t\t{\n");
            }

            for (i = 0; i < sizeof(Sig_u); i++)
            {
                if ( (i%16) ==0)
                    fprintf(pOut, "\n\t\t\t");
                fprintf(pOut, " 0x%02x,", Signature.Signature.Data[i]);
            }
            AvdCount++;

            if (AvdCount%3 != 0)
            {
                fprintf(pOut, "\n\t\t},\n\t\t{\n");
                if (AvdCount == 5 || AvdCount == 8 || AvdCount == 11 || AvdCount == 14)
                {
                    for (i = 0; i < sizeof(Sig_u); i++)
                    {
                        if ( (i%16) ==0)
                            fprintf(pOut, "\n\t\t\t");
                        fprintf(pOut, " 0x00,");
                    }
                    AvdCount++;
                }
            }
            if(AvdCount%3 == 0)
            {
                fprintf(pOut, "\n\t\t}\n\t},\n");
                fprintf(pOut,"#endif\n\n");
            }

        }
        fprintf(pOut, "\n};\n#endif");

    }

    fclose(fp);
    fclose(pOut);
}

void Generate_VICE_C_file(unsigned char *data_file_name,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;
    unsigned int VceCount = 0;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".c");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".c");
    }


    FILE *fp = fopen(data_file_name,"rb");
    Sig_u Signature;

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",data_file_name);
        exit(1);
    }
    else
    {
        pOut = fopen(FileName, "w");

        printf("Creating C header file %s.\n", FileName);

        fprintf(pOut, COPYRIGHT_NOTICE);
        fprintf(pOut, VCE_SIGNATURE_HEADER_1);

        while(fread(&Signature.Words, sizeof(Sig_u), 1, fp))
        {
            if(VceCount%2 == 0)
            {
                fprintf(pOut, VCE_SIGNATURE_HEADER_2, VceCount/2);
                fprintf(pOut, "\t{\n\t\t{\n");
            }

            for (i = 0; i < sizeof(Sig_u); i++)
            {
                if ( (i%16) ==0)
                    fprintf(pOut, "\n\t\t\t");
                fprintf(pOut, " 0x%02x,", Signature.Signature.Data[i]);
            }
            VceCount++;
            if(VceCount%2 == 0)
            {
                fprintf(pOut, "\n\t\t}\n\t},\n");
                fprintf(pOut,"#endif\n\n");
            }
            else
            {
                fprintf(pOut, "\n\t\t},\n\t\t{\n");
            }

        }
        fprintf(pOut, "\n};\n#endif");

    }

    fclose(fp);
    fclose(pOut);
}


void Generate_Audio_C_file(unsigned char *data,
    unsigned int size,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".c");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".c");
    }

    pOut = fopen(FileName, "w");

    printf("Creating C source file %s.\n", FileName);
    fprintf(pOut, COPYRIGHT_NOTICE);
    fprintf(pOut, AUDIO_SIGNATURE_HEADER_1);

    fprintf(pOut, "{\n");

    for (i = 0; i < size; i++)
    {
        if ( (i%16) ==0)
            fprintf(pOut, "\n\t");
        fprintf(pOut, " 0x%02x,", data[i]);
    }

    fprintf(pOut, "\n};\n");

    fclose(pOut);
}

void Generate_SID_C_file(unsigned char *data,
    unsigned int size,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".c");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".c");
    }

    pOut = fopen(FileName, "w");

    printf("Creating C source file %s.\n", FileName);
    fprintf(pOut, COPYRIGHT_NOTICE);
    fprintf(pOut, SID_SIGNATURE_HEADER_1);

    fprintf(pOut, "{\n");

    for (i = 0; i < size; i++)
    {
        if ( (i%16) ==0)
            fprintf(pOut, "\n\t");
        fprintf(pOut, " 0x%02x,", data[i]);
    }

    fprintf(pOut, "\n};\n");

    fclose(pOut);
}

void Generate_RAVE_C_file(unsigned char *data,
    unsigned int size,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".c");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".c");
    }

    pOut = fopen(FileName, "w");

    printf("Creating C source file %s.\n", FileName);
    fprintf(pOut, COPYRIGHT_NOTICE);
    fprintf(pOut, RAVE_SIGNATURE_HEADER_1);

    fprintf(pOut, "{\n");

    for (i = 0; i < size; i++)
    {
        if ( (i%16) ==0)
            fprintf(pOut, "\n\t");
        fprintf(pOut, " 0x%02x,", data[i]);
    }

    fprintf(pOut, "\n};\n");

    fclose(pOut);
}


void Generate_HEX_file(unsigned char *data,
    unsigned int size,
    char * file_name)
{
    char FileName[FILENAME_LEN] = {0};
    char * Token;
    FILE * pOut;
    unsigned int i;

    Token = strtok(file_name, ".");
    if (NULL != Token)
    {
        strcpy(FileName, Token);
        strcat(FileName, ".hex");

    }
    else
    {
        strcpy(FileName, file_name);
        strcat(FileName, ".hex");
    }

    pOut = fopen(FileName, "w");
    printf("Creating hex file %s.\n", FileName);
    for (i = 0; i < size; i++)
    {
        fprintf(pOut, "%02x", data[i]);
    }

    fclose(pOut);
}

void Generate_BIN_file(unsigned char *data,
    unsigned int size,
    char * file_name,
    unsigned int Append)
{
    FILE * pOut;
    unsigned int i;

    if (1 == Append)
    {
        pOut = fopen(file_name, "ab");
    }
    else
    {
        pOut = fopen(file_name, "wb");
    }
    printf("Creating binary file %s.\n", file_name);
    fwrite(data, 1, size, pOut);
    fclose(pOut);
}
