/******************************************************************************
 *    (c)2011 Broadcom Corporation
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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "nexus_platform.h"
#include "nexus_memory.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"
#include "drm_common.h"
#include "drm_common_swcrypto_types.h"
#include "drm_common_swcrypto.h"
#include "drm_key_region.h"
#include "drm_common_priv.h"

static void test_sha1(void);
static void test_partial_sha1(void);
static void test_sha256(void);

static void test_ecdsa_verify(void);
static void test_ecdsa_sign(void);
static void test_ecdsa_multiply(void);

static void test_aes_ecb(void);
static void test_aes_cbc(void);

static void test_cmac(void);

static void test_des_cbc(void);
static void test_des_ecb(void);

static void test_dsa(void);

static void test_md5(void);

static void test_rc4(void);

static void test_rng(void);

static void test_rsa(void);

/* Global error variable counter */
static uint32_t error_count = 0;


/******************************************************************************/
int main(int argc, char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    DrmCommonInit_t commonDrmInit;

    BKNI_Memset((uint8_t*)&commonDrmInit, 0x00, sizeof(DrmCommonInit_t));

    BSTD_UNUSED(argv);
    BSTD_UNUSED(argc);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    printf("\n\tMain - Starting test app ^^^^^^^^^^^^^^^^^^^^^^^\n");

    DRM_Common_BasicInitialize(&commonDrmInit);

    printf("\tMain - Opened BCRYPT ^^^^^^^^^^^^^^^^^^^^^^^\n");

    /* SHA-1 */
    test_sha1();
    test_partial_sha1();

    /* SHA-256 */
    test_sha256();

    /* ECDSA */
    test_ecdsa_verify();
    test_ecdsa_sign();
    test_ecdsa_multiply();

    /* AES */
    test_aes_ecb();
    test_aes_cbc();

    /* CMAC */
    test_cmac();

    /* DES-xxx */
    test_des_ecb();
    test_des_cbc();

    /* DSA */
    test_dsa();

    /* MD5 */
    test_md5();

    /* RC4 */
    test_rc4();

    /* RNG */
    test_rng();

    /* RSA */
    test_rsa();


    printf("\n\n\t%s - Number of test cases that failed = '%u'\n", __FUNCTION__, error_count);
    printf("\t%s - Number of test cases that failed = '%u'\n", __FUNCTION__, error_count);
    printf("\t%s - Number of test cases that failed = '%u'\n", __FUNCTION__, error_count);
    printf("\t%s - Number of test cases that failed = '%u'\n", __FUNCTION__, error_count);
    printf("\t%s - Exiting\n", __FUNCTION__);
    return 0;
}

void test_sha1()
{
    int i = 0;
    uint8_t pShaBuf[32] = {0};

    uint8_t pBuf[32] = {0xa8, 0xea, 0xc9, 0x81, 0x9b, 0xf0, 0xec, 0xcc,
    					0xa9, 0x04, 0xc4, 0x7f, 0xb1, 0x8c, 0x22, 0xa4,
    					0x1a, 0xaf, 0x6c, 0x05, 0xe8, 0x9e, 0xdb, 0xcf,
    					0x76, 0x5b, 0x21, 0xbd, 0x20, 0xf6, 0x19, 0x38};

    uint8_t pCompareSha1Hash[32] = {0x2e, 0x43, 0x7b, 0x40, 0x28, 0x8f, 0x1d, 0x89,
    							0x7e, 0xdd, 0xc8, 0x61, 0x32, 0x19, 0x9e, 0xce,
    							0x3b, 0x9e, 0xa0, 0xcc, 0x00, 0x00, 0x00, 0x00,
    							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	printf("\n\t%s - Entered SHA-1 test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);
	DRM_Common_SwSha1(pBuf, pShaBuf, 32);

	printf("\tMain - Calculated SHA-1 :\n\t\t");
	for(i = 0; i < 20; i++)
	{
	   printf("0x%02x ", pShaBuf[i]);
	}

	if(memcmp(pCompareSha1Hash, pShaBuf, 20) == 0)
	{
	   printf("\n\tMain - SHA-1 Test PASSED!!\n\n");
	}
	else
	{
	   printf("\n\tMain - SHA-1 Test FAILED!!\n\n");
	   error_count++;
	}
	return;
}


void test_partial_sha1()
{
    int i = 0;
    int curr_data_size = 0;
    uint8_t pShaBuf[32] = {0};
    DrmCommon_PartialSha1Type partialSha1Type;
    uint32_t context = 0;

    uint8_t *pCurrData = NULL;
    uint8_t pBuf[32] = {0xa8, 0xea, 0xc9, 0x81, 0x9b, 0xf0, 0xec, 0xcc,
    					0xa9, 0x04, 0xc4, 0x7f, 0xb1, 0x8c, 0x22, 0xa4,
    					0x1a, 0xaf, 0x6c, 0x05, 0xe8, 0x9e, 0xdb, 0xcf,
    					0x76, 0x5b, 0x21, 0xbd, 0x20, 0xf6, 0x19, 0x38};

    uint8_t pCompareSha1Hash[32] = {0x2e, 0x43, 0x7b, 0x40, 0x28, 0x8f, 0x1d, 0x89,
    							0x7e, 0xdd, 0xc8, 0x61, 0x32, 0x19, 0x9e, 0xce,
    							0x3b, 0x9e, 0xa0, 0xcc, 0x00, 0x00, 0x00, 0x00,
    							0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	printf("\n\t%s - Entered Partial SHA-1 test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	for(i=0; i<3; i++)
	{
		switch(i)
		{
			case 0:
				printf("\t%s - Partial SHA-1 Init\n", __FUNCTION__);
				pCurrData = pBuf;
				curr_data_size = 8;
				partialSha1Type = DrmCommon_PartialSha1Type_Init;
			break;

			case 1:
				printf("\t%s - Partial SHA-1 Update\n", __FUNCTION__);
				pCurrData = &pBuf[8];
				curr_data_size = 16;
				partialSha1Type = DrmCommon_PartialSha1Type_Update;
			break;

			case 2:
				printf("\t%s - Partial SHA-1 Finalize\n", __FUNCTION__);
				pCurrData = &pBuf[24];
				curr_data_size = 8;
				partialSha1Type = DrmCommon_PartialSha1Type_Finalize;
			break;

		}
		DRM_Common_SwPartialSha1(pBuf, pShaBuf, 32, partialSha1Type, &context);
	}

	printf("\t%s - Calculated Full SHA-1 :\n\t\t", __FUNCTION__);
	for(i = 0; i < 20; i++)
	{
	   printf("0x%02x ", pShaBuf[i]);
	}

	if(memcmp(pCompareSha1Hash, pShaBuf, 20) == 0)
	{
	   printf("\n\tMain - SHA-1 Test PASSED!!\n\n");
	}
	else
	{
	   printf("\n\tMain - SHA-1 Test FAILED!!\n\n");
	   error_count++;
	}
	return;
}


void test_sha256()
{
    int i = 0;
    uint8_t pShaBuf[32] = {0};

    uint8_t pBuf[32] = {0xa8, 0xea, 0xc9, 0x81, 0x9b, 0xf0, 0xec, 0xcc,
    					0xa9, 0x04, 0xc4, 0x7f, 0xb1, 0x8c, 0x22, 0xa4,
    					0x1a, 0xaf, 0x6c, 0x05, 0xe8, 0x9e, 0xdb, 0xcf,
    					0x76, 0x5b, 0x21, 0xbd, 0x20, 0xf6, 0x19, 0x38};

    uint8_t pCompareSha256Hash[32] ={0xf2, 0xe1, 0xe, 0x3b, 0x67, 0x2a, 0x51, 0x9e,
    								0x47, 0x33, 0xc9, 0xf2, 0x96, 0x43, 0xa8, 0xc2,
    								0xe, 0x43, 0xd6, 0xd9, 0xfc, 0x7b, 0x2f, 0xdf,
    								0xc7, 0x32, 0x6e, 0x7f, 0x94, 0x97, 0x29, 0xef};

    printf("\n\t%s - Entered SHA-1 test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);
	DRM_Common_SwSha256(pBuf, pShaBuf, 32);

	printf("\t%s - Calculated SHA-256 :\n\t\t", __FUNCTION__);
	for(i = 0; i < 32; i++)
	{
	   printf("0x%02x ", pShaBuf[i]);
	   if(((i+1)%16)==0)printf("\n\t\t");
	}

	if(memcmp(pCompareSha256Hash, pShaBuf, 32) == 0)
	{
	   printf("\n\t%s - SHA-256 Test PASSED!!\n\n", __FUNCTION__);
	}
	else
	{
	   printf("\n\t%s - SHA-256 Test FAILED!!\n\n", __FUNCTION__);
	   error_count++;
	}

	return;
}


void test_ecdsa_verify()
{
	DrmCommon_ECDSASw_Verify_t 	inoutp_ecdsaSwIO;
	unsigned char hash[] = {
		0xA9, 0x99, 0x3E, 0x36,
		0x47, 0x06, 0x81, 0x6A,
		0xBA, 0x3E, 0x25, 0x71,
		0x78, 0x50, 0xC2, 0x6C,
		0x9C, 0xD0, 0xD8, 0x9D
	};

	printf("\n\t%s - Entered ECDSA Verify test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);
	/* ECC params for AACS */
	inoutp_ecdsaSwIO.eccParm.P		= "900812823637587646514106462588455890498729007071";
	inoutp_ecdsaSwIO.eccParm.A		= "-3";
	inoutp_ecdsaSwIO.eccParm.B		= "366394034647231750324370400222002566844354703832";
	inoutp_ecdsaSwIO.eccParm.G_x	= "264865613959729647018113670854605162895977008838";
	inoutp_ecdsaSwIO.eccParm.G_y	= "51841075954883162510413392745168936296187808697";
	inoutp_ecdsaSwIO.eccParm.N		= "900812823637587646514106555566573588779770753047";

	/* signature */
	inoutp_ecdsaSwIO.sig.r			= "596533521920081240188195482970617559279336554242";
	inoutp_ecdsaSwIO.sig.s			= "798725022652965436783142054526225441223252669106";
	inoutp_ecdsaSwIO.sig.format	= DrmCommon_ECCSig_Format_Decimal;

	/* public key */
	inoutp_ecdsaSwIO.publicKey.x	= "746560871256702314793488376307783559983516734106";
	inoutp_ecdsaSwIO.publicKey.y	= "243999996927820557454500389846375685464403175020";

	inoutp_ecdsaSwIO.hash			= hash;
	inoutp_ecdsaSwIO.hashLen		= 20;

	DRM_Common_SwEcdsaVerify(&inoutp_ecdsaSwIO);

	if(inoutp_ecdsaSwIO.sigIsValid != 0)
	{
		printf("\ttest_ecdsa - PASSED!!!\n\n");
	}
	else
	{
		printf("\ttest_ecdsa - FAILED!!!\n\n");
		error_count++;
	}

	return;
}


void test_ecdsa_sign()
{
	DrmCommon_ECDSASw_Sign_t 	ecdsaSwIO;

	unsigned char hash[] = {
		0xA9, 0x99, 0x3E, 0x36,
		0x47, 0x06, 0x81, 0x6A,
		0xBA, 0x3E, 0x25, 0x71,
		0x78, 0x50, 0xC2, 0x6C,
		0x9C, 0xD0, 0xD8, 0x9D
	};

	printf("\n\t%s - Entered ECDSA Signature test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	/* ECC params for AACS */
	ecdsaSwIO.eccParm.P		= "900812823637587646514106462588455890498729007071";
	ecdsaSwIO.eccParm.A		= "-3";
	ecdsaSwIO.eccParm.B		= "366394034647231750324370400222002566844354703832";
	ecdsaSwIO.eccParm.G_x	= "264865613959729647018113670854605162895977008838";
	ecdsaSwIO.eccParm.G_y	= "51841075954883162510413392745168936296187808697";
	ecdsaSwIO.eccParm.N		= "900812823637587646514106555566573588779770753047";

	/* signature */
	ecdsaSwIO.sig.r			= NULL;
	ecdsaSwIO.sig.s			= NULL;
	ecdsaSwIO.sig.format	= DrmCommon_ECCSig_Format_Decimal;

	/* public key */
	ecdsaSwIO.privateKey	= "900812823637587646514106462588455811558729007071";

	/* randomly selected integer k */
	ecdsaSwIO.k				= "702232148019446860144825009548118511996283736794";

	ecdsaSwIO.hash			= hash;
	ecdsaSwIO.hashLen		= 20;

	if(DRM_Common_SwEcdsaSign(&ecdsaSwIO) == Drm_Success)
	{
		printf("\t%s - ECDSA signature (r,s)\n", __FUNCTION__);
		printf("\t%s - ecdsaSwIO.r = %s\n", __FUNCTION__, ecdsaSwIO.sig.r);
		printf("\t%s - ecdsaSwIO.s = %s\n", __FUNCTION__, ecdsaSwIO.sig.s);
	}
	else{
		printf("\t%s - ECDSA signature FAILED\n", __FUNCTION__);
	}
	return;
}



void test_ecdsa_multiply()
{
	DrmCommon_ECDSASw_Multiply_t 	ecdsaSwIO;

	char *exp_x = "1176954224688105769566774212902092897866168635793";
	char *exp_y = "1130322298812061698910820170565981471918861336822";

	printf("\n\t%s - Entered ECDSA Multiplication test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	/* ECC params for SECP160R1 */
	ecdsaSwIO.eccParm.P		= "1461501637330902918203684832716283019653785059327";
	ecdsaSwIO.eccParm.A		= "1461501637330902918203684832716283019653785059324";
	ecdsaSwIO.eccParm.B		= "163235791306168110546604919403271579530548345413";
	ecdsaSwIO.eccParm.G_x	= "425826231723888350446541592701409065913635568770";
	ecdsaSwIO.eccParm.G_y	= "203520114162904107873991457957346892027982641970";
	ecdsaSwIO.eccParm.N		= "1461501637330902918203687197606826779884643492439";

	/* scalar value */
	ecdsaSwIO.scalar		= "702232148019446860144825009548118511996283736794";

	/* input point */
	ecdsaSwIO.inPoint.x		= "425826231723888350446541592701409065913635568770";
	ecdsaSwIO.inPoint.y		= "203520114162904107873991457957346892027982641970";

	if (DRM_Common_SwEcdsaMultiply(&ecdsaSwIO) != Drm_Success)
	{
		printf("\t%s - Error calling Ecdsa multiply\n", __FUNCTION__);
		goto ErrorExit;
	}
	else
	{
		printf("\t%s - ECDSA scalar multiplcation result:\n", __FUNCTION__);
		printf("\t%s - x = %s\n", __FUNCTION__, ecdsaSwIO.outPoint.x);
		printf("\t%s - y = %s\n", __FUNCTION__, ecdsaSwIO.outPoint.y);

		if (strcmp (ecdsaSwIO.outPoint.x, exp_x) != 0)
		{
			printf("\t%s - Multiplication error x\n", __FUNCTION__);
			error_count++;
			goto ErrorExit;
		}
		if (strcmp (ecdsaSwIO.outPoint.y, exp_y) != 0)
		{
			printf("\t%s - Multiplication error y\n", __FUNCTION__);
			error_count++;
			goto ErrorExit;
		}
		printf("\t%s - Multiplication PASSED!!\n", __FUNCTION__);
	}

ErrorExit:
	return;
}


void test_aes_ecb()
{
    uint8_t pSrcBuf[16] = {0x78, 0x00, 0x46, 0x65, 0x89, 0x45, 0x98, 0xf1,
    					0x42, 0x9f, 0x67, 0x59, 0x21, 0x7e, 0x0d, 0x05};
    uint8_t compareBuf[16] = {0x47, 0x01, 0x00, 0x31, 0x14, 0x00, 0x00, 0x00,
    						  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t pDestBuf[16] = {0};
    const uint8_t KeyForAes[] = {0x54,0x32,0x87,0x4f, 0xee,0xbe,0xef,0x0d,
        						0x12,0x34,0x56,0x78, 0x9a,0xbc,0xde,0xf0};
    int i = 0;
	DrmCommon_AesEcbSw_t SwAesCtrl;

	BKNI_Memset((unsigned char*)pDestBuf, 0x0, 16);
	printf("\n\t%s - Entered AES-ECB test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	SwAesCtrl.pIn = pSrcBuf;
	SwAesCtrl.pkey = (unsigned char*)KeyForAes;
	SwAesCtrl.len = 16;
	SwAesCtrl.bEncFlag = false;
	SwAesCtrl.pOut = pDestBuf;
	SwAesCtrl.keybits = 128;

	if(DRM_Common_SwAesEcb(&SwAesCtrl) != Drm_Success)
	{
		printf("\t%s - bcrypt error\n", __FUNCTION__);
	}


   printf("\t%s - Decrypted buffer (do you see MPEG header?) :\n\t\t", __FUNCTION__);
   for(i = 0; i < 16; i++)
   {
	   printf("0x%02x ", pDestBuf[i]);
   }
   printf("\n");

   if(memcmp(compareBuf, pDestBuf, 16) == 0)
   {
	   printf("\t%s - TEST PASSED\n\n", __FUNCTION__);
   }
   else{
	   printf("\t%s - TEST FAILED\n\n", __FUNCTION__);
	   error_count++;
   }

   return;
}


void test_aes_cbc()
{
    uint8_t KeyForAes[] = {0x1b,0x2a,0x8e,0x26, 0x06,0x55,0x03,0x43,
    							 0xef,0x5e,0x42,0x87, 0xbe,0xfa,0x76,0xe0};
    uint8_t KeyForIV[]=  	{0x12,0x34,0x56,0x78, 0x23,0x45,0x67,0x89,
    							 0x34,0x56,0x78,0x9a, 0x45,0x67,0x89,0xab};
    uint8_t pSrcBuf[16] = {0x64, 0xf6, 0x7d, 0xc6, 0x64, 0x98, 0x4c, 0x26,
    						0xa8, 0x88, 0x69, 0x41, 0x6c, 0x23, 0x9e, 0x9d};
	uint8_t compareBuf[16] = {0x45, 0x02, 0x6c, 0xa7, 0x46, 0xc9, 0xbb, 0xb9,
    						0x3b, 0x24, 0xed, 0xa3, 0x38, 0x70, 0xf0, 0xd2};
    uint8_t pDestBuf[16] = {0};
    int i = 0;
	DrmCommon_AesCbcSw_t SwAesCtrl;

	printf("\n\t%s - Entered AES-CBC test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	BKNI_Memset((unsigned char*)pDestBuf, 0x0, 16);

	SwAesCtrl.pIn = pSrcBuf;
	SwAesCtrl.piv = KeyForIV;
	SwAesCtrl.pkey = (unsigned char*)KeyForAes;
	SwAesCtrl.len = 16;
	SwAesCtrl.bEncFlag = false;
	SwAesCtrl.pOut = pDestBuf;
	SwAesCtrl.keybits = 128;

	if(DRM_Common_SwAesCbc(&SwAesCtrl) != Drm_Success)
	{
		printf("\t%s - bcrypt error\n", __FUNCTION__);
	}


   printf("\t%s - Decrypted buffer :\n\t\t", __FUNCTION__);
   for(i = 0; i < 16; i++)
   {
	   printf("0x%02x ", pDestBuf[i]);
   }
   printf("\n");

   if(memcmp(compareBuf, pDestBuf, 16) == 0)
   {
	   printf("\t%s - TEST PASSED\n\n", __FUNCTION__);
   }
   else{
	   printf("\t%s - TEST FAILED\n\n", __FUNCTION__);
	   error_count++;
   }

   return;
}



/********************************************************************************/
/*							CMAC												*/
/********************************************************************************/
void test_cmac()
{
	DrmCommon_CmacSw_t	cmacParam;
	int i = 0;
	unsigned char Exp_CMAC_16[16] = {
		0x07, 0x0a, 0x16, 0xb4, 0x6b, 0x4d, 0x41, 0x44,
		0xf7, 0x9b, 0xdd, 0x9d, 0xd0, 0x4a, 0x28, 0x7c
	};
	unsigned char cmac_val[16];

	unsigned char key[16] = {
	    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
	    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};
	static unsigned char M[64] = {				/* input data */
	    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
	    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
	    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
	    0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
	    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
	    0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
	    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
	    0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
	};

	printf("\n\t%s - Entered CMAC test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	/* Perform CMAC, Mlen = 128 bits */
	cmacParam.pBuffer = M;
	cmacParam.length = 16;
	cmacParam.pKey = key;
	cmacParam.pMac = cmac_val;
    DRM_Common_SwCmac(&cmacParam);

    printf("\t%s - CMAC value:\n\t\t", __FUNCTION__);
    for(i = 0; i < 16; i++)
    {
 	   printf("0x%02x ", cmac_val[i]);
    }
    printf("\n");

	if(memcmp(cmac_val, Exp_CMAC_16, 16) == 0)
	{
	   printf("\t%s - TEST PASSED\n\n", __FUNCTION__);
	}
	else{
	   printf("\t%s - TEST FAILED\n\n", __FUNCTION__);
	   error_count++;
	}

	return;
}


void test_des_ecb()
{
	DrmCommon_DesEcbSw_t	ctrl;
	uint8_t encBuf [16] = {0xac, 0x17, 0xd1, 0x7a, 0xba, 0x0c, 0x57, 0x16,
						 0x5c, 0x8d, 0x84, 0x6b, 0xfc, 0x2e, 0xc7, 0x14};
	uint8_t decBuf [16] = {0};
	uint8_t compareBuf [16] = {0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
			                   0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b};
	uint8_t ecb_key[]={0x54,0x62,0xcb,0xa8, 0x72,0x46,0x43,0xe4};
	printf("\n\t%s - Entered DES-ECB test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);
	ctrl.pIn = encBuf;
	ctrl.pkey = ecb_key;
	ctrl.len = 16;
	ctrl.bEncFlag = false;
	ctrl.pOut = decBuf;

	DRM_Common_SwDesEcb(&ctrl);

	if(memcmp(decBuf, compareBuf, 16) == 0)
	{
	   printf("\t%s - TEST PASSED\n\n", __FUNCTION__);
	}
	else{
	   printf("\t%s - TEST FAILED\n\n", __FUNCTION__);
	   error_count++;
	}
	return;
}

void test_des_cbc()
{
	DrmCommon_DesCbcSw_t	ctrl;
	unsigned char cbc_key [8]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
	unsigned char cbc_iv  [8]={0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
	unsigned char cbc_data[40]={
		0x37,0x36,0x35,0x34,0x33,0x32,0x31,0x20,
		0x4E,0x6F,0x77,0x20,0x69,0x73,0x20,0x74,
		0x68,0x65,0x20,0x74,0x69,0x6D,0x65,0x20,
		0x66,0x6F,0x72,0x20,0x00,0x31,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	unsigned char cbc_ok[32]={
		0xcc,0xd1,0x73,0xff,0xab,0x20,0x39,0xf4,
		0xac,0xd8,0xae,0xfd,0xdf,0xd8,0xa1,0xeb,
		0x46,0x8e,0x91,0x15,0x78,0x88,0xba,0x68,
		0x1d,0x26,0x93,0x97,0xf7,0xfe,0x62,0xb4};

	unsigned char cbc_out[40];
	printf("\n\t%s - Entered DES-CBC test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);


	BKNI_Memset (cbc_out, 40, 0);

	ctrl.pIn = cbc_data;
	ctrl.piv = cbc_iv;  /* IV */
	ctrl.pkey = cbc_key;
	ctrl.len = strlen((char*)cbc_data)+1;	/* length of data. */
	ctrl.bEncFlag = true;
	ctrl.pOut = cbc_out;	/* output. */

	DRM_Common_SwDesCbc(&ctrl);

	if(memcmp(cbc_out, cbc_ok, 32) == 0)
	{
	   printf("\t%s - TEST PASSED\n\n", __FUNCTION__);
	}
	else{
	   printf("\t%s - TEST FAILED\n\n", __FUNCTION__);
	   error_count++;
	}

	return;
}




void test_dsa()
{
	DrmCommon_DsaSw_t		dsaSwIO;
	DrmCommon_DsaKey_t		key;

	uint32_t i = 0;

static uint8_t p[]={
	0x8d,0xf2,0xa4,0x94,0x49,0x22,0x76,0xaa,
	0x3d,0x25,0x75,0x9b,0xb0,0x68,0x69,0xcb,
	0xea,0xc0,0xd8,0x3a,0xfb,0x8d,0x0c,0xf7,
	0xcb,0xb8,0x32,0x4f,0x0d,0x78,0x82,0xe5,
	0xd0,0x76,0x2f,0xc5,0xb7,0x21,0x0e,0xaf,
	0xc2,0xe9,0xad,0xac,0x32,0xab,0x7a,0xac,
	0x49,0x69,0x3d,0xfb,0xf8,0x37,0x24,0xc2,
	0xec,0x07,0x36,0xee,0x31,0xc8,0x02,0x91,
	};

static uint8_t q[]={
	0xc7,0x73,0x21,0x8c,0x73,0x7e,0xc8,0xee,
	0x99,0x3b,0x4f,0x2d,0xed,0x30,0xf4,0x8e,
	0xda,0xce,0x91,0x5f,
	};

static uint8_t g[]={
	0x62,0x6d,0x02,0x78,0x39,0xea,0x0a,0x13,
	0x41,0x31,0x63,0xa5,0x5b,0x4c,0xb5,0x00,
	0x29,0x9d,0x55,0x22,0x95,0x6c,0xef,0xcb,
	0x3b,0xff,0x10,0xf3,0x99,0xce,0x2c,0x2e,
	0x71,0xcb,0x9d,0xe5,0xfa,0x24,0xba,0xbf,
	0x58,0xe5,0xb7,0x95,0x21,0x92,0x5c,0x9c,
	0xc4,0x2e,0x9f,0x6f,0x46,0x4b,0x08,0x8c,
	0xc5,0x72,0xaf,0x53,0xe6,0xd7,0x88,0x02,
	};


static uint8_t priv_key[]={
     	0xc5 ,0x37 ,0x73 ,0xf3 ,0x4d ,0xd0 ,0x48 ,0x81,
     	0x51 ,0x87 ,0x1d ,0x62 ,0x72 ,0xc7 ,0x90 ,0x3c,
     	0x58 ,0x02 ,0x45 ,0x09 };

/*
static uint8_t pub_key[]={
    	0x38 ,0x14 ,0xe8 ,0xd1 ,0xf4 ,0xd1 ,0x5d ,0x4b ,
    	0xdd ,0x22 ,0xb8 ,0x06 ,0x61 ,0x16 ,0x91 ,0x17 ,
    	0x4d ,0xca ,0x3a ,0x45 ,0x6b ,0xce ,0xe1 ,0xea ,
    	0xdd ,0x51 ,0x0b ,0xab ,0xf1 ,0xd2 ,0x4d ,0x8c ,
    	0x06 ,0x6e ,0x42 ,0x4d ,0x31 ,0x0a ,0x32 ,0x1b ,
    	0xef ,0x6e ,0x53 ,0x4b ,0xdb ,0x39 ,0xdf ,0x27 ,
    	0x76 ,0x0e ,0xe6 ,0x4f ,0x82 ,0x93 ,0x1b ,0x8a ,
    	0x1b ,0x22 ,0x04 ,0x75 ,0xf8 ,0x64 ,0xb9 ,0xa4 };
*/

static uint8_t k[]={
	0x23, 0x11, 0x96, 0x06, 0x15, 0x60, 0xa6, 0x53,
	0x17, 0xf2, 0x91, 0x7e, 0x16, 0xbb, 0xde, 0x3e,
	0x85, 0xfe, 0x44, 0x03};

static const uint8_t str1[]="12345678901234567890";

static uint8_t signature[]= {
	0x30, 0x2c, 0x02, 0x14, 0x1d, 0x39, 0x4b, 0x6f,
	0x99, 0xfa, 0xa9, 0xfa, 0xb1, 0xbd, 0xcc, 0xc7,
	0xcf, 0xf8, 0xe6, 0xdf, 0xac, 0x7f, 0x9c, 0xb8,
	0x02, 0x14, 0x36, 0xac, 0x7b, 0x29, 0x3a, 0xa2,
	0x16, 0xa6, 0x29, 0x98, 0xee, 0xab, 0x4e, 0xb9,
	0xb0, 0x41, 0xb4, 0x5b, 0xf0, 0x07} ;

	/* Setup the key to use	*/
	key.p.pData = p;
	key.p.len = sizeof (p);
	key.q.pData = q;
	key.q.len = sizeof (q);
	key.g.pData = g;
	key.g.len = sizeof (g);
	key.k.pData = k;
	key.k.len = sizeof (k);

	/* This is for DSA Sign */
	key.privkey.pData = priv_key;
	key.privkey.len = sizeof (priv_key);

	dsaSwIO.bDSASign		= true;
	dsaSwIO.key 		= &key;
	dsaSwIO.pbDataIn 	= (uint8_t *)str1;
	dsaSwIO.cbDataIn	= strlen((char*)str1)  ;
	printf("\n\t%s - Entered DSA test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	DRM_Common_SwDsa(&dsaSwIO);

	printf("\t%s - Print signature is \n\t\t", __FUNCTION__) ;
	for (i=0; i< dsaSwIO.sigoutlen ; i++)
	{
		printf("%02x ", dsaSwIO.sigout[i]) ;
		if( ((i+1)%16)==0)printf("\n\t\t");
	}
	printf("\n") ;

	if (memcmp(dsaSwIO.sigout, signature, dsaSwIO.sigoutlen) != 0)
	{
		printf("\t%s - TEST FAILED Signature data is wrong!\n", __FUNCTION__);
		error_count++;
	}
	else{
		printf("\t%s - TEST PASSED (correct signature data generated).\n", __FUNCTION__);
	}

	return;
}



void test_md5()
{
	DrmCommon_Md5Sw_t	md5Param;
	uint8_t md[16];

	char *input1 = "abc";

	uint8_t output1[] = {0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0,
								0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 };

	md5Param.pbInData = (uint8_t *)input1;
	md5Param.cbInData = strlen (input1);
	md5Param.pbOutData = md;

	printf("\n\t%s - Entered MD5 test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	DRM_Common_SwMd5(&md5Param);

	if(memcmp(output1, md, 16) == 0)
	{
		printf("\t%s - TEST PASSED.\n", __FUNCTION__);
	}
	else{
		printf("\t%s - TEST FAILED\n", __FUNCTION__);
		error_count++;
	}

	return;
}


void test_rc4()
{
	DrmCommon_Rc4Sw_t	rc4Param;
	DrmCommon_Rc4Key_t	key;
	int i = 0;
	static uint8_t key0[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef  };
	static uint8_t input0[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };
	static uint8_t output0[] = {0x75, 0xb7, 0x87, 0x80, 0x99, 0xe0, 0xc5, 0x96 };
	uint8_t	outData[8];

	DRM_Common_SwRc4SetKey (&key, 8, key0);

	rc4Param.key = &key;
	rc4Param.cbLen = 8;
	rc4Param.pbDataIn = input0;
	rc4Param.pbDataOut = outData;

	printf("\n\t%s - Entered RC4 test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	DRM_Common_SwRc4SetKey (&key, 8, key0);

	DRM_Common_SwRc4(&rc4Param);

	printf("\n\t%sOutput:\n\t", __FUNCTION__);
	for(i=0; i<8; i++) printf("0x%02x ", outData[i]);

	if(memcmp(outData, output0, 8) == 0)
	{
		printf("\n\t%s - TEST PASSED\n\n", __FUNCTION__);
	}
	else{
		printf("\n\t%s - TEST FAILED\n\n", __FUNCTION__);
		error_count++;
	}

	return;
}

void test_rng()
{
	DrmCommon_RngSw_t	rngParam;
	uint8_t buf[16];
	int i = 0;

	rngParam.pucBuf = buf;
	rngParam.ulLen = 16;

	printf("\n\t%s - Entered RNG test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	DRM_Common_SwRng(&rngParam);

    printf("\t%s - Random bytes:\n\t\t", __FUNCTION__);
    for(i = 0; i < 16; i++)
    {
 	   printf("0x%02x ", buf[i]);
    }
    printf("\n\n");

	return;
}


void test_rsa()
{
	DrmCommon_RsaSwParam_t		rsaSwIO;
	DrmCommon_RsaKey_t				key;
	unsigned long				outDataLen, i;
	uint8_t ctext[128] =  {0x00};
	uint8_t ptext[128] =  {0x00};
	uint8_t ptext1_1[] = {	0x66, 0x28, 0x19, 0x4e, 0x12, 0x07, 0x3d, 0xb0,
	   									0x3b, 0xa9, 0x4c, 0xda, 0x9e, 0xf9, 0x53, 0x23,
	   									0x97, 0xd5, 0x0d, 0xba, 0x79, 0xb9, 0x87, 0x00,
	   									0x4a, 0xfe, 0xfe, 0x34 };
    uint8_t n[] = {
	0xa8, 0xb3, 0xb2, 0x84, 0xaf, 0x8e, 0xb5, 0x0b, 0x38, 0x70, 0x34, 0xa8, 0x60, 0xf1, 0x46, 0xc4,
	0x91, 0x9f, 0x31, 0x87, 0x63, 0xcd, 0x6c, 0x55, 0x98, 0xc8, 0xae, 0x48, 0x11, 0xa1, 0xe0, 0xab,
	0xc4, 0xc7, 0xe0, 0xb0, 0x82, 0xd6, 0x93, 0xa5, 0xe7, 0xfc, 0xed, 0x67, 0x5c, 0xf4, 0x66, 0x85,
	0x12, 0x77, 0x2c, 0x0c, 0xbc, 0x64, 0xa7, 0x42, 0xc6, 0xc6, 0x30, 0xf5, 0x33, 0xc8, 0xcc, 0x72,
	0xf6, 0x2a, 0xe8, 0x33, 0xc4, 0x0b, 0xf2, 0x58, 0x42, 0xe9, 0x84, 0xbb, 0x78, 0xbd, 0xbf, 0x97,
	0xc0, 0x10, 0x7d, 0x55, 0xbd, 0xb6, 0x62, 0xf5, 0xc4, 0xe0, 0xfa, 0xb9, 0x84, 0x5c, 0xb5, 0x14,
	0x8e, 0xf7, 0x39, 0x2d, 0xd3, 0xaa, 0xff, 0x93, 0xae, 0x1e, 0x6b, 0x66, 0x7b, 0xb3, 0xd4, 0x24,
	0x76, 0x16, 0xd4, 0xf5, 0xba, 0x10, 0xd4, 0xcf, 0xd2, 0x26, 0xde, 0x88, 0xd3, 0x9f, 0x16, 0xfb };

    uint8_t e[] = {0x01, 0x00, 0x01};

    uint8_t d[] = {
	0x53, 0x33, 0x9c, 0xfd, 0xb7, 0x9f, 0xc8, 0x46, 0x6a, 0x65, 0x5c, 0x73, 0x16, 0xac, 0xa8, 0x5c,
	0x55, 0xfd, 0x8f, 0x6d, 0xd8, 0x98, 0xfd, 0xaf, 0x11, 0x95, 0x17, 0xef, 0x4f, 0x52, 0xe8, 0xfd,
	0x8e, 0x25, 0x8d, 0xf9, 0x3f, 0xee, 0x18, 0x0f, 0xa0, 0xe4, 0xab, 0x29, 0x69, 0x3c, 0xd8, 0x3b,
	0x15, 0x2a, 0x55, 0x3d, 0x4a, 0xc4, 0xd1, 0x81, 0x2b, 0x8b, 0x9f, 0xa5, 0xaf, 0x0e, 0x7f, 0x55,
	0xfe, 0x73, 0x04, 0xdf, 0x41, 0x57, 0x09, 0x26, 0xf3, 0x31, 0x1f, 0x15, 0xc4, 0xd6, 0x5a, 0x73,
	0x2c, 0x48, 0x31, 0x16, 0xee, 0x3d, 0x3d, 0x2d, 0x0a, 0xf3, 0x54, 0x9a, 0xd9, 0xbf, 0x7c, 0xbf,
	0xb7, 0x8a, 0xd8, 0x84, 0xf8, 0x4d, 0x5b, 0xeb, 0x04, 0x72, 0x4d, 0xc7, 0x36, 0x9b, 0x31, 0xde,
	0xf3, 0x7d, 0x0c, 0xf5, 0x39, 0xe9, 0xcf, 0xcd, 0xd3, 0xde, 0x65, 0x37, 0x29, 0xea, 0xd5, 0xd1 };

    printf("\n\t%s - Entered RSA test ^^^^^^^^^^^^^^^^^^^^^^^\n", __FUNCTION__);

	/* Setup the key to use */
	key.n.pData = n;
	key.n.len = sizeof (n);
	key.e.pData = e;
	key.e.len = sizeof (e);

	rsaSwIO.bRSAop		= drmRsaenc;
	rsaSwIO.key 		= &key;
	rsaSwIO.pbDataIn 	= ptext1_1;
	rsaSwIO.cbDataIn	= sizeof (ptext1_1);
	rsaSwIO.pbDataOut	= ctext;
	rsaSwIO.cbDataOut	= &outDataLen;

	/* Initialize output buffer to 0 */
	memset(rsaSwIO.pbDataOut, 0x00, 128);

	DRM_Common_SwRsa(&rsaSwIO);

	printf("\t%s - Size of encrypted data in bytes = %d\n", __FUNCTION__, (int)(*rsaSwIO.cbDataOut));

	if (*rsaSwIO.cbDataOut != 128)
	{
		printf("\t%s - Length '%lu' != expected length of '128'\n", __FUNCTION__, (*rsaSwIO.cbDataOut));
		goto ErrorExit;
	}

	printf("\t%s - Input Data (encrypted) =\n\t\t", __FUNCTION__);
	for (i=0; i<16; i++){
		printf("0x%02x ", rsaSwIO.pbDataOut[i]);
	}
	printf("\n");

	/* Now try to decrypt it */
	key.d.pData = d;
	key.d.len = sizeof (d);

	rsaSwIO.bRSAop		= drmRsadec;
	rsaSwIO.key 		= &key;
	rsaSwIO.pbDataIn 	= ctext;
	rsaSwIO.cbDataIn	= outDataLen;
	rsaSwIO.pbDataOut	= ptext;
	rsaSwIO.cbDataOut	= &outDataLen;

	/* Initialize output buffer to 0 */
	for (i=0; i<sizeof(ptext1_1); i++)
		rsaSwIO.pbDataOut[i] = 0;

	DRM_Common_SwRsa(&rsaSwIO);

	printf("\t%s - Decrypted data length = %d\n", __FUNCTION__, (int)outDataLen);
	if (outDataLen != sizeof (ptext1_1))
	{
		printf("\t%s - Decrypted size is wrong!! expected %d\n", __FUNCTION__, sizeof (ptext1_1));
	}

	printf("\t%s - Output Data (decrypted) =\n\t\t", __FUNCTION__);
	for (i=0; i<16; i++)	{
		printf("0x%02x ", rsaSwIO.pbDataOut[i]);
	}
	printf("\n");

	if (memcmp(ptext, ptext1_1, outDataLen) != 0)
	{
		printf("\t%s - TEST FAILED Decryption data is wrong!\n", __FUNCTION__);
		error_count++;
	}
	else
	{
		printf("\t%s - TEST PASSED (ptext matches output)\n", __FUNCTION__);
	}

ErrorExit:
	return;
}

