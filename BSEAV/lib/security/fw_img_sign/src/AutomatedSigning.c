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


RSA *PublicKey = NULL;
RSA *PrivateKey = NULL;

#define FILENAME_LEN 300


char privateKey[]="-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEowIBAAKCAQEAr/aFD6bhDNMvgxnITmoPFyjBZiCov+qi9UMdhRrBnUnKhkS9\n"\
"FPZTKZ0LYDhYt3DcH4iD9vrHvCnpr4DDi1C9HMgJiNfLM7SWkyccjZK4FlfuwbhT\n"\
"nekdwAVO93mnhZbPx8Nl9BLFMA12mzpUAXk/45m3Bcq2Cw6ZVqA24t+ABTjS9de7\n"\
"u1JiVhUbFqPjD2KvVc7cB99L1VK9UZHX3fBORDoHhT03N9auWnmrGgt+nLxqVvxo\n"\
"IA85azkqNw2y/o8UYj/ZjYV971gitSuqETsjw7YdnIbQHbl5pudsTErJlen8I8Db\n"\
"/8sD+wuZ2rg1Kfi8zbm6PKUuHrugo+kzSYUHDwIDAQABAoIBAHsfkrGGAlj3+HnS\n"\
"c48ytQZudQzh81H+ezIEWUaGl01YkcZrmKZvSXeHtKsKIkGRTeUen8AUb+fgp+wu\n"\
"vCTiRME70zgWTtAKH4YtytPhAsBXnO+d65xFvpRa4otVa3uVMCRGPDORYrqh4Qjy\n"\
"wk8ZqTO3nhIGamTIPVEY3+7Vln2PwTrNHMako87AD0uM05R/FwFptfe7brwKaywx\n"\
"qIJ2sdIKjV5fKLs8C+5uuzjI32ONiGzj2C38IWrScD+YRW6hycbSjPx2tSw1J/I/\n"\
"GpQDJuD6h5sQ6hpSkGfF+MCDE2Z4WJSufB5+AmuKxDQWobQMlkCTcMjEzRif7ECb\n"\
"8rug+UECgYEA1sN/inH7vXxjPWoXRPnH2teIV1LaF8hHH57ZPYBvWQjtbjTkYib9\n"\
"2AurfawsR7MwkQmfxH8AJS0Orpd3tSU0ANrZCeDtD4RqSHRKoK2AwXHrqFd4Bw/R\n"\
"wBv9BH03uCTbOnFMLjAVztOCcC2EHiVrMyLfQhpMB9+NPalfKeOaGW8CgYEA0b/P\n"\
"gAH38bs7P0a70UN79hHTDUsd4Cz+WzCFDLIVisYB3kgIF0onQptn3hqBwXVLlReL\n"\
"W/0ra2dfKN9rcljBIQYvgXk0OHlJIHJKROYtcnJinQf1jRt6b7Bchu9CTBPyhQrT\n"\
"2J5fuk/8buPecucytpV34Qj9Koa4xKMgTQca3GECgYBtBnYa3F6GBT2EEfWaKIuS\n"\
"x1QJsJ/S7vNcVBHIrQjZ1DRkfClswoqfQN1gQSWBiLAhERewcyCpvsPzUiWNKkPK\n"\
"I0+HOk5eUER8X7z68NmqWqDdpvnh69/5fLvnqYG908gPVTDtSa/ofWt8cz7c5vYS\n"\
"0aMxNZRfcteK9A7Bfy/dtQKBgF3P7H7UYpE7rMQgXuC5zjfAwSSw9CvP8/PVP6zt\n"\
"+bSX2z4P+Y5xHB4uY8ZzFJXyYFvOrAX4tfLTyTv+sY/zIm4i1hySmUio9owMkis+\n"\
"yBToFDMn3CvAnoJV3wx69qwQP+hBb37zVt196OmwAU5jGcuQDo4X9yOOHhXU4B2T\n"\
"j/ABAoGBAMn2DLEWRl+49qztJ8w7iOmVVESb2EL1Jk04So14SWdCJgke6lUaK5cS\n"\
"bqUkQ24UVinA6h1zzlYb7SxqAciokzasQZhshr/tWod0f057keLU/Qrehldh90HX\n"\
"9wB7/fhX77pvyjBwmOizHXWAnTMQuaaFhEeZ3SY3g/OrV1UAEFWu\n"\
"-----END RSA PRIVATE KEY-----\n";

RSA * ReadKeyFromPemFile(char * filename, int public)
{
    FILE *fp = fopen(filename,"rb");

    if(fp == NULL)
    {
        printf("Unable to open file %s \n",filename);
        return NULL;
    }
    RSA *rsa = NULL;

    if(public)
    {
        rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL);
    }
    else
    {
        rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    }

    fclose(fp);

    return rsa;
}

RSA * createRSA(unsigned char * key,int Pub)
{
    RSA *rsa = NULL;
    BIO *keybio;
    keybio = BIO_new_mem_buf(key, -1);
    if (keybio==NULL)
    {
        printf( "Failed to create key BIO");
        return 0;
    }

    if(Pub)
    {
        rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    }
    else
    {
        rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    }
    if(rsa == NULL)
    {
        printf( "Failed to create RSA");
    }

    return rsa;
}


//int public_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
//{
//    RSA * rsa = createRSA(key,1);
//    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,RSA_PKCS1_PADDING);
//    return result;
//}
//
//int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
//{
//    RSA * rsa = createRSA(key,0);
//    int  result = RSA_private_decrypt(data_len,enc_data,decrypted,rsa,RSA_PKCS1_PADDING);
//    return result;
//}


int private_encrypt(unsigned char * data,int data_len,unsigned char * key, unsigned char *encrypted)
{
    int result = 0;
    RSA * rsa = createRSA(key,0);
    result = RSA_private_encrypt(data_len,data,encrypted,rsa,RSA_PKCS1_PADDING);
    return result;
}

int RSA_Sign(unsigned char *Data, int DataSize, RSA *Key, unsigned char *Signature)
{
    int SignatureSize = -1;
    int result = 0;
    result = RSA_sign(NID_sha256, Data, DataSize, Signature, &SignatureSize, Key);
    return SignatureSize;
}

//int public_decrypt(unsigned char * enc_data,int data_len,unsigned char * key, unsigned char *decrypted)
//{
//    RSA * rsa = createRSA(key,1);
//    int  result = RSA_public_decrypt(data_len,enc_data,decrypted,rsa,RSA_PKCS1_PADDING);
//    return result;
//}

void printLastError(char *msg)
{
    char * err = malloc(130);;
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    printf("%s ERROR: %s\n",msg, err);
    free(err);
}

void Sha256Digest (unsigned char *Data, int DataSize, unsigned char *Digest)
{
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, Data, DataSize);
        SHA256_Final(Digest, &ctx);
}

int SignData(unsigned char *Data, int DataSize, unsigned char *Signature)
{
        u_int8_t ShaDigest[SHA256_DIGEST_LENGTH];
    Sha256Digest(Data, DataSize, ShaDigest);
    if (NULL == PrivateKey)
    {
            printf("ERROR: Private Key Not Loaded.\n");
        return -1;

    }
        return RSA_Sign(ShaDigest, SHA256_DIGEST_LENGTH, PrivateKey, Signature);
}

int SignDataFile(unsigned char *FileName, unsigned char *Signature)
{
    u_int8_t ShaDigest[SHA256_DIGEST_LENGTH];
    u_int8_t *Data = NULL;
    int DataSize = 0;
    int i = 0;
    u_int8_t Temp;

    FILE *fp = fopen(FileName,"rb");

    if (NULL == PrivateKey)
    {
            printf("ERROR: Private Key Not Loaded.\n");
        return -1;

    }

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",FileName);
        return -1;
    }
    else
    {
        while (1 == fread(&Temp, 1, 1, fp))
        {
            DataSize++;
        }
        rewind(fp);
        //Allocate enough memory for binary Data. Plus 10 bytes as extra buffer.
        Data = malloc(DataSize + 10);
        if (!Data)
        {
            printf("ERROR Allocating %d Bytes for Binary File.\n", DataSize+10);
            exit (1);
        }
        //printf("Allocating %d Bytes for Binary File.\n", DataSize);
        i = 0;
        while (1 == fread(&Temp, 1, 1, fp))
        {
            Data[i++] = Temp;
        }
    }
    fclose(fp);
    printf("INFO: %d Bytes Read from file %s.\n", DataSize, FileName);
    Sha256Digest(Data, DataSize, ShaDigest);
    free(Data);
    return RSA_Sign(ShaDigest, SHA256_DIGEST_LENGTH, PrivateKey, Signature);
}

int WriteCSignatureAndParameter(unsigned char *DataFileName, unsigned char *OutFileName, unsigned int RegionId, unsigned int Append)
{

    FILE *fp = fopen(DataFileName,"rb");
    KeyWithParams_u KeyWithParams;
    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",DataFileName);
        return -1;
    }
    else
    {
        fread(&KeyWithParams.Words, sizeof(KeyWithParams), 1, fp);
    }
    fclose(fp);
    Generate_C_file((unsigned char *)&KeyWithParams.Words, sizeof(KeyWithParams), OutFileName, RegionId, Append);
    return 1;
}

int WriteAudioCSignature(unsigned char *DataFileName, unsigned char *OutFileName)
{

    FILE *fp = fopen(DataFileName,"rb");
    Sig_u Signature;

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",DataFileName);
        return -1;
    }
    else
    {
        fread(&Signature.Words, sizeof(Sig_u), 1, fp);
    }
    fclose(fp);
    Generate_Audio_C_file((unsigned char *)&Signature.Words, sizeof(Sig_u), OutFileName);

        return 1;
}

int WriteSidCSignature(unsigned char *DataFileName, unsigned char *OutFileName)
{

    FILE *fp = fopen(DataFileName,"rb");
    Sig_u Signature;

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",DataFileName);
        return -1;
    }
    else
    {
        fread(&Signature.Words, sizeof(Sig_u), 1, fp);
    }
    fclose(fp);
    Generate_SID_C_file((unsigned char *)&Signature.Words, sizeof(Sig_u), OutFileName);

        return 1;
}

int WriteRaveCSignature(unsigned char *DataFileName, unsigned char *OutFileName)
{

    FILE *fp = fopen(DataFileName,"rb");
    Sig_u Signature;

    if(fp == NULL)
    {
        printf("ERROR: Unable to open file %s \n",DataFileName);
        return -1;
    }
    else
    {
        fread(&Signature.Words, sizeof(Sig_u), 1, fp);
    }
    fclose(fp);
    Generate_RAVE_C_file((unsigned char *)&Signature.Words, sizeof(Sig_u), OutFileName);

        return 1;
}

void SwapRsaKey(Key_u *KeyIn, Key_u *KeyOut)
{
    int i;
    for (i = 0; i < RSA_KEY_SIZE_WORDS; i++)
    {
        KeyOut->Words[i] = KeyIn->Words[RSA_KEY_SIZE_WORDS - 1 - i];
    }
}

void SwapBytesInWord(unsigned int *pData, unsigned int WordCount)
{
    int i;
    for (i = 0; i < WordCount; i++)
    {
        pData[i] = SWAP_BYTES_32(pData[i]);
    }
}

void PrintSigningParameters()
{
    printf("=================================\n");
    printf("INFO: SigningRights  = 0x%02x.\n", SigningParameters.Param.SigningRights);
    printf("INFO: Reserved0      = 0x%02x.\n", SigningParameters.Param.Reserved0);
    printf("INFO: PublicExponent = 0x%02x.\n", SigningParameters.Param.PublicExponent);
    printf("INFO: Reserved1      = 0x%02x.\n", SigningParameters.Param.Reserved1);
    printf("INFO: MarketID       = 0x%08x.\n", SigningParameters.Param.MarketID);
    printf("INFO: MarketIDMask   = 0x%08x.\n", SigningParameters.Param.MarketIDMask);
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    printf("INFO: MarketIDSelect = 0x%02x.\n", SigningParameters.Param.MarketIDSelect);
    printf("INFO: EpochSelect    = 0x%02x.\n", SigningParameters.Param.EpochSelect);
#else
    printf("INFO: Reserved2      = 0x%02x.\n", SigningParameters.Param.Reserved2);
    printf("INFO: Reserved3      = 0x%02x.\n", SigningParameters.Param.Reserved3);
#endif
    printf("INFO: EpochMask      = 0x%02x.\n", SigningParameters.Param.EpochMask);
    printf("INFO: Epoch          = 0x%02x.\n", SigningParameters.Param.Epoch);
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    printf("INFO: SigVersion     = 0x%02x.\n", SigningParameters.Param.SigVersion);
    printf("INFO: SigType        = 0x%02x.\n", SigningParameters.Param.SigType);
    printf("INFO: Reserved3      = 0x%02x.\n", SigningParameters.Param.Reserved3);
    printf("INFO: Reserved4      = 0x%02x.\n", SigningParameters.Param.Reserved4);
#endif
    printf("=================================\n");
}

void WriteKeyToFile(char *FileName)
{
  Key_u PubKeyBin;
  //int i;
  unsigned char * BytePtr;
  BytePtr = (unsigned char *)PubKeyBin.Words;

  BN_bn2bin(PublicKey->n, PubKeyBin.Key.Data);

  Generate_BIN_file(BytePtr, sizeof(PubKeyBin), FileName, 0);

  //for (i = 0; i < sizeof(KeyWithParams_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n");

}

void WriteKeyAndParamsToFile(char *FileName)
{
  KeyWithParams_u KeyWithParams;
  Key_u PubKeyBin;
  //int i;
  unsigned char * BytePtr;
  BytePtr = (unsigned char *)KeyWithParams.Words;

  BN_bn2bin(PublicKey->n, PubKeyBin.Key.Data);
  SwapRsaKey(&PubKeyBin, &KeyWithParams.KeyWithParams.Key);
  memcpy(&KeyWithParams.KeyWithParams.Param, SigningParameters.Words, sizeof (Param_u));

  //Swap 32 and 16 bit fields in the Params structure.
  //Only MarketID and MarketIDMask are 32-bit fields. There are no 16 bit fields.
  KeyWithParams.KeyWithParams.Param.MarketID = LE_TO_BE_32(KeyWithParams.KeyWithParams.Param.MarketID);
  KeyWithParams.KeyWithParams.Param.MarketIDMask = LE_TO_BE_32(KeyWithParams.KeyWithParams.Param.MarketIDMask);

  Generate_BIN_file(BytePtr, sizeof(KeyWithParams_u), FileName, 0);

  //for (i = 0; i < sizeof(KeyWithParams_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n");

}

void WriteBinAndParamsToFile(FwParam_u FwParams, char * DataFileName, char *FileName, unsigned int LittleEndianInput)
{
  int i = 0;
  unsigned char * BytePtr;
  BytePtr = (unsigned char *)FwParams.Words;
  u_int8_t *Data = NULL;
  unsigned int *pData32 = (unsigned int *)Data;
  int DataSize = 0;
  u_int8_t Temp;

  //Swap 32 and 16 bit fields in the Params structure.
  //Only MarketID and MarketIDMask are 32-bit fields. There are no 16 bit fields.
  FwParams.Param.MarketID = LE_TO_BE_32(FwParams.Param.MarketID);
  FwParams.Param.MarketIDMask = LE_TO_BE_32(FwParams.Param.MarketIDMask);

  FILE *fp = fopen(DataFileName,"rb");

  if(fp == NULL)
  {
      printf("ERROR: Unable to open file %s \n",DataFileName);
      exit(1);
  }
  else
  {
    while (1 == fread(&Temp, 1, 1, fp))
    {
        DataSize++;
    }
    rewind(fp);
    //Allocate enough memory for binary Data + Parameters. Plus 10 bytes as extra buffer.
    Data = malloc(DataSize + sizeof(Param_u) + 10);
    if (!Data)
    {
        printf("ERROR Allocating %d Bytes for Binary File.\n", (int)(DataSize + sizeof(Param_u) + 10));
        exit (1);
    }
    //printf("Allocating %d Bytes for Binary File.\n", DataSize);
    i = 0;
    while (1 == fread(&Temp, 1, 1, fp))
    {
        Data[i++] = Temp;
    }
  }
  fclose(fp);
  printf("INFO: %d Bytes Read from file %s.\n", DataSize, DataFileName);
  if (1 == LittleEndianInput)
  {

    //Skip RAVE since its already in Big Endian.
    if (eRave != FwParams.Param.CpuType)
    {
      //convert little endian binary data to big endian on every word boundary.
      for (i = 0; i < (DataSize/4); i++)
      {
        pData32[i] = SWAP_BYTES_32(pData32[i]);
      }
    }
  }
  //copy parameters to data buffer
  memcpy(&Data[DataSize], FwParams.Words, sizeof (Param_u));

  //save data buffer into output binary file
  Generate_BIN_file(Data, DataSize+sizeof(Param_u), FileName, 0);

  free(Data);
  //for (i = 0; i < sizeof(KeyWithParams_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n");

}

void WriteKeyAndParamsAndSignatureToFile(char *DataFileName, char *SignatureFileName, char *OutFileName, unsigned int LittleEndianOutput)
{
  KeyWithParams_u KeyWithParams;
  Key_u KeySignature;
  Key_u KeySignatureSwapped;
  int i;
  unsigned int size = 0;
  unsigned char * BytePtr;

  //memcpy(&KeyWithParams.KeyWithParams.Param, SigningParameters.Words, sizeof (Param_u));

  FILE *pDataFileName = fopen(DataFileName,"rb");

  if (NULL == pDataFileName)
  {
    printf("ERROR: Cannot open file: %s.\n", DataFileName);
    return;
  }

  FILE *pSignatureFileName = fopen(SignatureFileName,"rb");
  if (NULL == pSignatureFileName)
  {
    printf("ERROR: Cannot open file: %s.\n", SignatureFileName);
    fclose(pDataFileName);
    return;
  }

  FILE *pOutFileName = fopen(OutFileName,"wb");
  if (NULL == pOutFileName)
  {
    printf("ERROR: Cannot open file: %s.\n", OutFileName);
    fclose(pDataFileName);
    fclose(pSignatureFileName);
    return;
  }

  size = fread((unsigned char *)KeyWithParams.Words, sizeof(KeyWithParams_u), 1, pDataFileName);
  if (1 == size)
  {
    printf("INFO: Read %d bytes from %s.\n", (int)sizeof(KeyWithParams_u), DataFileName);

  }

  size = fread((unsigned char *)KeySignature.Words, sizeof(Key_u), 1, pSignatureFileName);
  if (1 == size)
  {
    printf("INFO: Read %d bytes from %s.\n", (int)sizeof(Key_u), SignatureFileName);

  }
  SwapRsaKey(&KeySignature, &KeySignatureSwapped);

  //BytePtr = (unsigned char *)KeyWithParams.Words;
  //for (i = 0; i < sizeof(KeyWithParams_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n\n");

  //BytePtr = (unsigned char *)KeySignature.Words;
  //for (i = 0; i < sizeof(Key_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n\n");

  //BytePtr = (unsigned char *)KeySignatureSwapped.Words;
  //for (i = 0; i < sizeof(Key_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n\n");

  //Convert the Words from Big Endian to Little Endian Format.
  if(LittleEndianOutput)
  {
    SwapBytesInWord(KeyWithParams.Words, sizeof(KeyWithParams_u)/4);
    SwapBytesInWord(KeySignatureSwapped.Words, RSA_KEY_SIZE_WORDS);
  }

  fwrite((unsigned char *)KeyWithParams.Words, sizeof(KeyWithParams_u), 1, pOutFileName);
  fwrite((unsigned char *)KeySignatureSwapped.Words, sizeof(Key_u), 1, pOutFileName);


  fclose(pDataFileName);
  fclose(pSignatureFileName);
  fclose(pOutFileName);


  //Generate_BIN_file(BytePtr, sizeof(KeyWithParams_u), OutFileName);

  //for (i = 0; i < sizeof(KeyWithParams_u) ; i++)
  //{
  //    if (i%16 == 0)
  //        printf("\n");
  //    printf("%02x", BytePtr[i]);
  //}
  //printf("\n");

}

void SwapSignature(char *SignatureFileName, char *OutFileName, unsigned int Append)
{
  Key_u KeySignature;
  Key_u KeySignatureSwapped;
  int i;
  unsigned int size = 0;
  unsigned char * BytePtr;

  FILE *pSignatureFileName = fopen(SignatureFileName,"rb");
  if (NULL == pSignatureFileName)
  {
    printf("ERROR: Cannot open file: %s.\n", SignatureFileName);
    return;
  }

  FILE *pOutFileName = NULL;

  if (1 == Append)
  {
    pOutFileName = fopen(OutFileName, "ab");
  }
  else
  {
    pOutFileName = fopen(OutFileName, "wb");
  }

  if (NULL == pOutFileName)
  {
    printf("ERROR: Cannot open file: %s.\n", OutFileName);
    fclose(pSignatureFileName);
    return;
  }

  size = fread((unsigned char *)KeySignature.Words, sizeof(Key_u), 1, pSignatureFileName);
  SwapRsaKey(&KeySignature, &KeySignatureSwapped);

  SwapBytesInWord(KeySignatureSwapped.Words, RSA_KEY_SIZE_WORDS);

  fwrite((unsigned char *)KeySignatureSwapped.Words, sizeof(Key_u), 1, pOutFileName);

  fclose(pSignatureFileName);
  fclose(pOutFileName);

}

void HandleCommand_write_key(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char FileName[FILENAME_LEN];
char *pFileName = NULL;
unsigned int Value;
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&FileName);
                    pFileName = FileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pFileName)
    {
        printf("INFO: Writing Public key to %s.\n", pFileName);
        WriteKeyToFile(pFileName);
    }

}
void HandleCommand_add_param_key(char *CmdParameters, FILE* pBinFileList)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char FileName[FILENAME_LEN];
char *pFileName = NULL;
unsigned int Value;
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-right"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.SigningRights = Value;
            }
            else if (0 == strcmp(SubToken, "-exp"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.PublicExponent = Value;
            }
            else if (0 == strcmp(SubToken, "-mid"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.MarketID = Value;

            }
            else if (0 == strcmp(SubToken, "-mid_mask"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.MarketIDMask = Value;
            }
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            else if (0 == strcmp(SubToken, "-mid_sel"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.MarketIDSelect = Value;
            }
            else if (0 == strcmp(SubToken, "-epo_sel"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.EpochSelect = Value;
            }
#endif
            else if (0 == strcmp(SubToken, "-epo_mask"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.EpochMask = Value;

            }
            else if (0 == strcmp(SubToken, "-epo"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.Epoch = Value;
            }
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            else if (0 == strcmp(SubToken, "-sig_ver"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.SigVersion = Value;

            }
            else if (0 == strcmp(SubToken, "-sig_type"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                SigningParameters.Param.SigType = Value;
            }
#endif
            else if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&FileName);
                    pFileName = FileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    PrintSigningParameters();

    if (NULL != pFileName)
    {
        printf("INFO: Storing Public key with parameters to %s.\n", pFileName);
        if (NULL != pBinFileList)
            fprintf(pBinFileList, "%s\n", pFileName);
        WriteKeyAndParamsToFile(pFileName);

    }

}

void HandleCommand_add_param_bin(char *CmdParameters, FILE *pBinFileList)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char FileName[FILENAME_LEN];
char *pFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;
FwParam_u FwParams = {0};
CpuType_e CpuType;
unsigned int LittleEndianInput = 1;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-cpu_type"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if(0 == strcmp(SubToken, "host"))
                {
                    FwParams.Param.CpuType = eHost;
                }
                else if (0 == strcmp(SubToken, "raaga"))
                {
                    FwParams.Param.CpuType = eRaaga;
                }
                else if (0 == strcmp(SubToken, "avd"))
                {
                    FwParams.Param.CpuType = eAvd;
                }
                else if (0 == strcmp(SubToken, "rave"))
                {
                    FwParams.Param.CpuType = eRave;
                }
                else if (0 == strcmp(SubToken, "hvd"))
                {
                    FwParams.Param.CpuType = eHvd;
                }
                else if (0 == strcmp(SubToken, "vice"))
                {
                    FwParams.Param.CpuType = eVice;
                }
                else if (0 == strcmp(SubToken, "sid"))
                {
                    FwParams.Param.CpuType = eSid;
                }
                else if (0 == strcmp(SubToken, "sage"))
                {
                    FwParams.Param.CpuType = eSAGE;
                }
                else if (0 == strcmp(SubToken, "avs"))
                {
                    FwParams.Param.CpuType = eAvs;
                }
                else
                {
                    printf ("ERROR: Unknown CPU Type: [%s]. Parameter Ignored...\n", SubToken);
                }
            }
            else if (0 == strcmp(SubToken, "-fw_epo"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.Reserved1 = Value;

            }
            else if (0 == strcmp(SubToken, "-mid"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.MarketID = Value;

            }
            else if (0 == strcmp(SubToken, "-mid_mask"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.MarketIDMask = Value;
            }
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            else if (0 == strcmp(SubToken, "-mid_sel"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.MarketIDSelect = Value;
            }
            else if (0 == strcmp(SubToken, "-epo_sel"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.EpochSelect = Value;
            }
#endif
            else if (0 == strcmp(SubToken, "-epo_mask"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.EpochMask = Value;

            }
            else if (0 == strcmp(SubToken, "-epo"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.Epoch = Value;
            }
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
            else if (0 == strcmp(SubToken, "-sig_ver"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.SigVersion = Value;

            }
            else if (0 == strcmp(SubToken, "-sig_type"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                FwParams.Param.SigType = Value;
            }
#endif
            else if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&FileName);
                    pFileName = FileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in_endian"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    if(0 == strcmp(SubToken, "be"))
                    {
                        LittleEndianInput = 0;
                    }
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    //PrintSigningParameters();

    if (NULL != pFileName)
    {
        printf("INFO: Storing binary file with parameters to %s.\n", pFileName);
        if (NULL != pBinFileList)
            fprintf(pBinFileList, "%s\n", pFileName);
        WriteBinAndParamsToFile(FwParams, pDataFileName, pFileName, LittleEndianInput);

    }

}



void HandleCommand_add_signature(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
char SignatureFileName[FILENAME_LEN];
char *pSignatureFileName = NULL;
unsigned int LittleEndianOutput = 0;
unsigned int Value;
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in_key"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in_signature"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&SignatureFileName);
                    pSignatureFileName = SignatureFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-out_endian"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    if(0 == strcmp(SubToken, "le"))
                    {
                        LittleEndianOutput = 1;
                    }
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }


    if (NULL != pDataFileName)
    {
        printf("INFO: Output File %s created by adding signature from %s to key from %s.\n", pOutFileName, pSignatureFileName, pDataFileName);
        WriteKeyAndParamsAndSignatureToFile(pDataFileName, pSignatureFileName, pOutFileName, LittleEndianOutput);

    }

}

void HandleCommand_swap_signature(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char SignatureFileName[FILENAME_LEN];
char *pSignatureFileName = NULL;
unsigned int Append = 0;
unsigned int Value;
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&SignatureFileName);
                    pSignatureFileName = SignatureFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-append"))
            {
                Append = 1;
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }


    if (NULL != pSignatureFileName)
    {
        SwapSignature(pSignatureFileName, pOutFileName, Append);

    }

}

void HandleCommand_sign(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int LittleEndianOutput = 0;
unsigned int KeepOpenSSLOrder = 0;
unsigned int Append = 0;
unsigned int Value;
int ParamCount = 0;
eOutputType OutType;
Key_u Signature;
Key_u SignatureSwapped;
int SignatureSize = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));
    memset(((unsigned char *) &SignatureSwapped), 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-out_type"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    if(0 == strcmp(SubToken, "c_source"))
                    {
                        OutType = eCSource;
                    }
                    else if(0 == strcmp(SubToken, "binary"))
                    {
                        OutType = eBinary;
                    }
                }
            }
            else if (0 == strcmp(SubToken, "-out_endian"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    if(0 == strcmp(SubToken, "le"))
                    {
                        LittleEndianOutput = 1;
                    }
                }
            }
            else if (0 == strcmp(SubToken, "-append"))
            {
                Append = 1;
            }
            else if (0 == strcmp(SubToken, "-keep_openssl_order"))
            {
                KeepOpenSSLOrder = 1;
            }
                else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    SignatureSize = SignDataFile(pDataFileName, Signature.Key.Data);
    if(SignatureSize == -1)
    {
        printLastError("RSA Sign Operation Failed.");
        exit(0);
    }

    for (i = 0; i < 256; i++)
    {
        if (i%16 == 0)
            printf("\n");
        printf("%02x", Signature.Key.Data[i]);
    }


        if (KeepOpenSSLOrder == 0)
        {
        SwapRsaKey(&Signature, &SignatureSwapped);
    }
        else
        {
            memcpy(&SignatureSwapped, &Signature, sizeof (Signature));
        }

    //Convert the Words from Big Endian to Little Endian Format.
    if(LittleEndianOutput)
    {
      SwapBytesInWord(SignatureSwapped.Words, RSA_KEY_SIZE_WORDS);
    }


    if (NULL != pOutFileName)
    {
        if (eCSource == OutType)
        {
            printf("INFO: Signature File %s created by signing %s.\n", pOutFileName, pDataFileName);
            //Generate_BIN_file(SignatureSwapped.Key.Data, SignatureSize, pOutFileName);
            Generate_AVD_C_file(SignatureSwapped.Key.Data, pOutFileName);
        }
        else if (eBinary == OutType)
        {
            printf("INFO: Signature File %s created by signing %s.\n", pOutFileName, pDataFileName);
            Generate_BIN_file(SignatureSwapped.Key.Data, SignatureSize, pOutFileName, Append);
        }
    }

}

void HandleCommand_post_process(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;
Key_u Signature;
int SignatureSize = 0;
unsigned int RegionId = 0xFFFF;
unsigned int Append = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-region"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%x", &Value);
                RegionId = Value;

            }
            else if (0 == strcmp(SubToken, "-append"))
            {
                Append = 1;
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        WriteCSignatureAndParameter(pDataFileName, pOutFileName, RegionId, Append);
    }

}

void HandleCommand_post_process_audio(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;
Key_u Signature;
int SignatureSize = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        WriteAudioCSignature(pDataFileName, pOutFileName);
    }

}

void HandleCommand_post_process_sid(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;
Key_u Signature;
int SignatureSize = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        WriteSidCSignature(pDataFileName, pOutFileName);
    }

}

void HandleCommand_post_process_rave(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;
Key_u Signature;
int SignatureSize = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        WriteRaveCSignature(pDataFileName, pOutFileName);
    }

}


void HandleCommand_post_process_avd(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;


    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        Generate_AVD_C_file(pDataFileName, pOutFileName);
    }

}

void HandleCommand_post_process_vce(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
unsigned int Value;
int ParamCount = 0;


    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        Generate_VICE_C_file(pDataFileName, pOutFileName);
    }

}


void HandleCommand_c_array(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char OutFileName[FILENAME_LEN];
char *pOutFileName = NULL;
char DataFileName[FILENAME_LEN];
char *pDataFileName = NULL;
char VariableName[FILENAME_LEN];
unsigned int Value;
int ParamCount = 0;
Key_u Signature;
int SignatureSize = 0;
int i;

    memset(((unsigned char *) &Signature)       , 0, sizeof(Key_u));

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-out"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&OutFileName);
                    pOutFileName = OutFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&DataFileName);
                    pDataFileName = DataFileName;
                }
            }
            else if (0 == strcmp(SubToken, "-var"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                if (NULL != SubToken)
                {
                    sscanf(SubToken, "%s", (char *)&VariableName);
                }
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != pOutFileName)
    {
        printf("INFO: C Signature File %s created by reading %s\n", pOutFileName, pDataFileName);
        Generate_C_Array(pDataFileName, pOutFileName, VariableName);
    }

}



void HandleCommand_load_prv_key(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char FileName[FILENAME_LEN];
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%s", FileName);
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != PrivateKey)
    {
        RSA_free(PrivateKey);
        PrivateKey = NULL;
    }

    PrivateKey = ReadKeyFromPemFile(FileName, 0);
    if (NULL == PrivateKey)
    {
        printf("ERROR: Private Key load from file %s FAILED.\n", FileName);

    }
    else
    {
        printf("INFO: Private Key loaded from file %s.\n", FileName);
    }
}

void HandleCommand_load_pub_key(char *CmdParameters)
{
char *Token;
char *SubToken;
char *SavePtr2;
char *SavePtr3;
char FileName[FILENAME_LEN];
int ParamCount = 0;

    while (1)
    {
        Token = strtok_r(CmdParameters, " ", &SavePtr2);
        CmdParameters = NULL;
        if (NULL == Token)
        {
            //printf("Reached end of command. Found %d Parameters.\n",ParamCount);
            break;
        }
        //printf("Parameter[%d] = %s\n",ParamCount, Token);
        int SubParamCount = 0;
        while (1)
        {
            SubToken = strtok_r(Token, "=", &SavePtr3);
            Token = NULL;

            if (NULL == SubToken)
            {
                //printf("Reached end of parameter. Found %d SubParameters.\n\n",SubParamCount);
                break;
            }
            //printf("SubParameter[%d] for Parameter[%d] = %s\n",SubParamCount, ParamCount, SubToken);
            SubParamCount++;
            if (0 == strcmp(SubToken, "-in"))
            {
                SubToken = strtok_r(NULL, "=", &SavePtr3);
                sscanf(SubToken, "%s", FileName);
            }
            else
            {
                printf ("WARNING: Unknown Command Parameter: [%s]. Parameter Ignored...\n", SubToken);
            }

        }
        ParamCount++;
    }

    if (NULL != PublicKey)
    {
        RSA_free(PublicKey);
        PublicKey = NULL;
    }

    PublicKey = ReadKeyFromPemFile(FileName, 1);
    if (NULL == PublicKey)
    {
        printf("ERROR: Public Key load from file %s FAILED.\n", FileName);

    }
    else
    {
        printf("INFO: Public Key loaded from file %s.\n", FileName);
    }
}


#define COMMAND_LEN 1024

int main(int argc, char* argv[])
{
    FILE * pControlFile;
    FILE * pBinFileList = NULL;
    char scratch[COMMAND_LEN];
    char command[50];
    char parameters[10][FILENAME_LEN];
    char *Token;
    char *SubToken;
    char *SavePtr1;
    char *SavePtr2;
    char *SavePtr3;

    unsigned char DS2Key[DS2_SIZE];
    unsigned char AESKey[DS2_SIZE];
    unsigned int marketId, rightClass;
    int SignatureSize = 0;
    int i;
    char FileName[FILENAME_LEN];

    strcpy(FileName, "data_out/RsaSigned.bin");


        u_int8_t Signature[RSA_KEY_SIZE_BYTES];
    memset(((unsigned char *) Signature), 0, RSA_KEY_SIZE_BYTES);


        printf("ZEUS Version = %x.\n",BHSM_ZEUS_VERSION);

    if (argc < 2)
    {
        printf("Usage AutomatedSigning.exe commands.in\n");
        exit(1);
    }

    pControlFile = fopen(argv[1], "r");

    if (argc == 3)
    {
        pBinFileList = fopen(argv[2], "w");
    }

    while(fgets(scratch, COMMAND_LEN, pControlFile) != NULL)
    {

        scratch[strcspn(scratch, "\n\r")] = '\0';   //replace newline with null.
        printf("INFO: Command Line = %s\n",scratch);
        Token = strtok_r(scratch, " ", &SavePtr1);
        int ParamCount = 0;
        //If line starts with a comment marker of '#', skip that line.
        if ('#' == Token[0])
        {
            printf ("INFO: Skipping Command [%s]...\n\n", Token);
            continue;

        }
        //Decode the command token.
        if (0 == strcmp(Token, "write_key"))
        {
            HandleCommand_write_key(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        if (0 == strcmp(Token, "add_param_key"))
        {
            HandleCommand_add_param_key(SavePtr1, pBinFileList);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "add_param_bin"))
        {
            HandleCommand_add_param_bin(SavePtr1, pBinFileList);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "load_prv_key"))
        {
            HandleCommand_load_prv_key(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "load_pub_key"))
        {
            HandleCommand_load_pub_key(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "add_signature"))
        {
            HandleCommand_add_signature(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "sign"))
        {
            HandleCommand_sign(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process_audio"))
        {
            HandleCommand_post_process_audio(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process_sid"))
        {
            HandleCommand_post_process_sid(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process_avd"))
        {
            HandleCommand_post_process_avd(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process_vce"))
        {
            HandleCommand_post_process_vce(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process_rave"))
        {
            HandleCommand_post_process_rave(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "post_process"))
        {
            HandleCommand_post_process(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "swap_signature"))
        {
            HandleCommand_swap_signature(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else if (0 == strcmp(Token, "c_array"))
        {
            HandleCommand_c_array(SavePtr1);
            printf ("INFO: Processed Command [%s]\n\n", Token);
        }
        else
        {
            printf ("WARNING: Unknown Command: [%s]...\n\n", Token);
        }
        //printf("Parameter[0] = %s\n", parameters[0]);
        //sscanf(scratch,"%s", parameters[0]);
        //printf("Parameter[0] = %s\n",parameters[0]);


    }
    if (pBinFileList)
        fclose(pBinFileList);

    fclose(pControlFile);

//  SignatureSize = SignDataFile("test_key/brcm_test_rsa_fwkey2_pub.bin", Signature);
//
//  if(SignatureSize == -1)
//  {
//      printLastError("RSA Sign Operation Failed.");
//      exit(0);
//  }
//  Generate_BIN_file(Signature, SignatureSize, FileName);
//
//
//  Key_u PubKeyBin;
//  Key_u PubKeyBinSwapped;
//  BN_bn2bin(PublicKey->n, PubKeyBin.Key.Data);
//
//
//  printf("Read Public Key from file.\n");
//  for (i = 0; i < 256; i++)
//  {
//      if (i%16 == 0)
//          printf("\n");
//      printf("%02x", PubKeyBin.Key.Data[i]);
//  }
//  printf("\n");
//  SwapRsaKey(&PubKeyBin, &PubKeyBinSwapped);
//
//  for (i = 0; i < 256; i++)
//  {
//      if (i%16 == 0)
//          printf("\n");
//      printf("%02x", PubKeyBinSwapped.Key.Data[i]);
//  }
//  printf("\n");


    return 0;
}
