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

#ifndef __GENERATEFILES_H__
#define __GENERATEFILES_H__

void Generate_BBS_script (unsigned char *ds,
								 unsigned int ds_id,
								 unsigned char * crc_ds,
								 char * file_name,
								 unsigned int chipID);
void Generate_C_file(unsigned char *data,
	unsigned int size,
	char * file_name,
	unsigned int RegionId,
	unsigned int Append);

void Generate_HEX_file(unsigned char *data,
	unsigned int size,
	char * file_name);

void Generate_BIN_file(unsigned char *data,
	unsigned int size,
	char * file_name,
	unsigned int Append);


#define COPYRIGHT_NOTICE "/******************************************************************************\n\
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.\n\
 *\n\
 *  This program is the proprietary software of Broadcom and/or its licensors,\n\
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and\n\
 *  conditions of a separate, written license agreement executed between you and Broadcom\n\
 *  (an \"Authorized License\").  Except as set forth in an Authorized License, Broadcom grants\n\
 *  no license (express or implied), right to use, or waiver of any kind with respect to the\n\
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all\n\
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU\n\
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY\n\
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.\n\
 *\n\
 *  Except as expressly set forth in the Authorized License,\n\
 *\n\
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade\n\
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,\n\
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.\n\
 *\n\
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED \"AS IS\"\n\
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR\n\
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO\n\
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES\n\
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,\n\
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION\n\
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF\n\
 *  USE OR PERFORMANCE OF THE SOFTWARE.\n\
 *\n\
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS\n\
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR\n\
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR\n\
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF\n\
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT\n\
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE\n\
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF\n\
 *  ANY LIMITED REMEDY.\n\
 ******************************************************************************/\n\n\n\n\n"

#define AVD_SIGNATURE_HEADER_1 "#if (NEXUS_NUM_XVD_DEVICES)\n\n\n\n\
static unsigned char gAvdFirmwareSignatures[NEXUS_NUM_XVD_DEVICES][NEXUS_VIDEO_DECODER_SECURITY_MAX_ARCHS_PER_AVD][256] =\n\
{\n"

#define AVD_SIGNATURE_HEADER_2 "#if (NEXUS_NUM_XVD_DEVICES > %d)\n"

#define VCE_SIGNATURE_HEADER_1 "#if (NEXUS_NUM_VCE_DEVICES)\n\n\n\n\
static unsigned char gVceFirmwareSignatures[NEXUS_NUM_VCE_DEVICES][NEXUS_VIDEO_ENCODER_SECURITY_MAX_ARCHS_PER_VCE][256] =\n\
{\n"

#define VCE_SIGNATURE_HEADER_2 "#if (NEXUS_NUM_VCE_DEVICES > %d)\n"

#define AUDIO_SIGNATURE_HEADER_1 "unsigned char gRapFirmwareSignature[256] =\n"

#define SID_SIGNATURE_HEADER_1 "unsigned char gSidFirmwareSignature[256] =\n"


#define RAVE_SIGNATURE_HEADER_1 "unsigned char gRaveFirmwareSignature[256] =\n"


#define BLOB_SIZE 44
#define LICENSE_TOKEN_SIZE 28
#define PADDING_SIZE 7
#define DS2_SIZE 16

#define LE_TO_BE_32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#define SWAP_BYTES_32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
#define RSA_KEY_SIZE_BITS 2048
#define RSA_KEY_SIZE_BYTES (RSA_KEY_SIZE_BITS/8)
#define RSA_KEY_SIZE_WORDS (RSA_KEY_SIZE_BYTES/4)



#define BHSM_ZEUS_VERSION_CALC(major,minor) (((major)<<16)|(minor))
#define BHSM_ZEUS_VERSION     BHSM_ZEUS_VERSION_CALC(BHSM_ZEUS_VER_MAJOR,BHSM_ZEUS_VER_MINOR)


extern RSA *PublicKey;
extern RSA *PrivateKey;

struct KeyStruct
{
  unsigned char Data[RSA_KEY_SIZE_BYTES];
};

typedef union KeyUnion
{
  struct KeyStruct Key;
  unsigned int Words[RSA_KEY_SIZE_WORDS];
} Key_u;

struct SigStruct
{
  unsigned char Data[RSA_KEY_SIZE_BYTES];
};

typedef union SigUnion
{
  struct SigStruct Signature;
  unsigned int Words[RSA_KEY_SIZE_WORDS];
} Sig_u;

struct FwParamStruct
{
  unsigned char Reserved0;
  unsigned char Reserved1;
  unsigned char CpuType;
  unsigned char NoReloc;
  unsigned int  MarketID;
  unsigned int  MarketIDMask;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
  unsigned char MarketIDSelect;
  unsigned char EpochSelect;
#else
  unsigned char Reserved2;
  unsigned char Reserved3;
#endif
  unsigned char EpochMask;
  unsigned char Epoch;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
  unsigned char SigVersion;
  unsigned char SigType;
  unsigned char Reserved3;
  unsigned char Reserved4;
#endif
};

typedef union FwParamUnion
{
  struct FwParamStruct Param;
  unsigned int Words[sizeof(struct FwParamStruct)/4];
} FwParam_u;

struct ParamStruct
{
  unsigned char SigningRights;
  unsigned char Reserved0;
  unsigned char PublicExponent;
  unsigned char Reserved1;
  unsigned int  MarketID;
  unsigned int  MarketIDMask;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
  unsigned char MarketIDSelect;
  unsigned char EpochSelect;
#else
  unsigned char Reserved2;
  unsigned char Reserved3;
#endif
  unsigned char EpochMask;
  unsigned char Epoch;
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
  unsigned char SigVersion;
  unsigned char SigType;
  unsigned char Reserved3;
  unsigned char Reserved4;
#endif
};

typedef union ParamUnion
{
  struct ParamStruct Param;
  unsigned int Words[sizeof(struct ParamStruct)/4];
} Param_u;

struct KeyWithParmsStruct
{
  Key_u  Key;
  struct ParamStruct Param;
};

typedef union KeyWithParamsUnion
{
  struct KeyWithParmsStruct KeyWithParams;
  unsigned int Words[sizeof(struct KeyWithParmsStruct)/4];
} KeyWithParams_u;

typedef enum OutputType
{
  eBinary = 1,
  eCSource = 2,
  eCHeader = 3,
  eNumOutputType
} eOutputType;

typedef enum CpuType_e
{
     eHost = 0,
     eRaaga = 1,
     eAvd = 2,
     eRave = 3,
     eHvd = 4,
     eVice = 5,
     eSid = 6,
     eSAGE = 7,
     eAvs = 8,
} CpuType_e;


Param_u SigningParameters;

void Generate_Audio_C_file(unsigned char *data, unsigned int size, char * file_name);
void Generate_SID_C_file(unsigned char *data, unsigned int size, char * file_name);
void Generate_RAVE_C_file(unsigned char *data, unsigned int size, char * file_name);
void Generate_AVD_C_file(unsigned char *data_file_name, char * file_name);
void Generate_VICE_C_file(unsigned char *data_file_name, char * file_name);
void WriteBinAndParamsToFile(FwParam_u FwParams, char * DataFileName, char *FileName, unsigned int LittleEndianInput);
void Generate_C_Array(unsigned char *data_file_name, char * file_name, char * var_name);

#endif
