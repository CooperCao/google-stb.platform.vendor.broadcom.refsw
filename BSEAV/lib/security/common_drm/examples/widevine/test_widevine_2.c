/***************************************************************************
 *    (c)2010-2012 Broadcom Corporation
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
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_widevine.h"
#include "drm_metadata.h"


#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

static void test_decrypt(uint8_t *enc, uint8_t *dec, uint8_t* iv, DrmSecurityKeyType keyslot);
static void test_load_cw(uint8_t* common_key, uint8_t* ecm, DrmSecurityKeyType keyslot );
static void test_load_cw_fixed(uint8_t* fixed_key, uint8_t* ecm,  DrmSecurityKeyType keyslot);
static void test_get_keyID(void);
static void test_get_deviceID(void);

/*define the following to test in place decryption*/
#define IN_PLACE_DECRYPTION

/* Initialize the key used to encrypt the refence buffers with bdvd_crypto functions*/
/* to a know value. For this test to work, the drm bin file must use the same key.*/
#define TEST_ENCRYPTION_DECRYPTION
#ifdef TEST_ENCRYPTION_DECRYPTION
uint8_t key[16] = {0x00, 0xbf, 0x3e, 0x0a, 0x31, 0x32, 0x9b, 0x03, 0xa7, 0x71, 0x39, 0x20, 0x34, 0xd2, 0xf4, 0xce};
uint8_t enc_iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
uint8_t enc_ck[16] = {0x87, 0xc2, 0xf6, 0x19, 0xeb, 0x24, 0x04, 0x5e,
		              0xbb, 0x9a, 0x0c, 0x0e, 0x5c, 0xbc, 0x64, 0xeb};
uint8_t enc_ecm[32] = {0x42, 0xb2, 0x3a, 0xc5, 0xe7, 0x08, 0xa0, 0x11,
		               0x38, 0xab, 0x35, 0x34, 0xd5, 0x05, 0xce, 0x17,
		               0xa6, 0xdd, 0x03, 0x3e, 0xfd, 0xe1, 0xe2, 0xf8,
		               0xc8, 0xd0, 0x43, 0x9b, 0x0c, 0x96, 0xaf, 0xd2};
uint8_t enc_fixed[16] ={0x7e, 0xca, 0xd3, 0x2e, 0xec, 0x06, 0x5f, 0xf4, 0xeb, 0xa6, 0xc6, 0x32, 0x11, 0x5e, 0x0c, 0x0c};
uint8_t sw_key[16] = {0x0, 0xbf,0x3e,0xa,0x31,0x32,0x9b,0x3, 0xa7,0x71,0x39,0x20,0x34,0xd2,0xf4,0xce};

/* encrypted buffer*/
uint8_t enc_buf[50] =  {0x73,0x6c,0x37,0x15,0xbf,0xd2,0x77,0x97,
                        0x57,0x46,0x84,0xc4,0x4f,0x8e,0x5c,0xbc,
                        0x11,0x86,0xa9,0x5c,0x73,0x73,0xb0,0xa5,
                        0x60,0xc6,0x65,0xf9,0x3c,0x6c,0xbc,0x1c,
                        0x81,0x11,0x5e,0xf4,0x8e,0x76,0xf5,0xf6,
                        0x06,0xd9,0x1d,0x6b,0x9f,0xb3,0xda,0x87,0x55,0x55};

                        /*73 6C 37 15 BF D2 77 97
                        57 46 84 C4 4F 8E 5C BC
                        11 86 A9 5C 73 73 B0 A5
                        60 C6 65 F9 3C 6C BC 1C
                        81 11 5E F4 8E 76 F5 F6
                        06 D9 1D 6B 9F B3 DA 87*/
uint32_t test_buf_len = 50;
#endif

static 	uint8_t device_id[DRM_WIDEVINE_DEVICE_ID_SIZE] = {0x00};
static  uint8_t key_id[DRM_WIDEVINE_ENC_KEYID_SIZE] = {0x00};


int main(int argc, char* argv[])
{
    uint8_t *dec = NULL;
	uint8_t *ck = NULL;
	uint8_t *fixed = NULL;
	uint8_t *ecm = NULL;
	uint8_t *iv = NULL;
	uint8_t *k = NULL;
    uint8_t *wv_enc_buf = NULL;
    uint8_t *wv_dec_buf = NULL;
   DRM_Widevine_Init_t inputWvInitStruct;

   NEXUS_PlatformSettings platformSettings;
   NEXUS_Platform_GetDefaultSettings(&platformSettings); 
   platformSettings.openFrontend = false;
   NEXUS_Platform_Init(&platformSettings);

   BSTD_UNUSED(argc);

#ifdef TEST_ENCRYPTION_DECRYPTION
    dec = BKNI_Malloc(test_buf_len);
    ck  = BKNI_Malloc(16);
    fixed = BKNI_Malloc(16);
    ecm = BKNI_Malloc(32);
    iv  = BKNI_Malloc(16);
    k   = BKNI_Malloc(16);
    
    memset(dec, 0, test_buf_len);
    memcpy(ck, enc_ck, 16);
    memcpy(fixed, enc_fixed, 16);
    memcpy(ecm, enc_ecm, 32);
    memcpy(iv, enc_iv, 16);
    memcpy(k, key, 16);
#endif

    inputWvInitStruct.mode = DRM_WidevineDecryptMode_eVod;
   	inputWvInitStruct.key_file = argv[1];

	if(DRM_Widevine_Initialize(inputWvInitStruct) != Drm_Success)
    {
        printf("\n\tMAIN - failed to Initialize Widevine DRM module **************\n");
        return -1;
    }

    test_get_keyID();
    test_get_deviceID();


#ifdef TEST_ENCRYPTION_DECRYPTION

    NEXUS_Memory_Allocate(sizeof(enc_buf), NULL,(void **) &wv_enc_buf);
    NEXUS_Memory_Allocate(sizeof(enc_buf), NULL,(void **) &wv_dec_buf);

    memcpy(wv_enc_buf, enc_buf, sizeof(enc_buf));
    memset(wv_dec_buf, 0, sizeof(enc_buf));

    printf("\n\nload cw to odd key slot ...\n");
    test_load_cw(ck, ecm,DrmSecurityKeyType_eOdd  );
    printf("\ndecrypting using odd key slot ...(the expected result should be all '55')\n");
    test_decrypt(wv_enc_buf, wv_dec_buf, iv, DrmSecurityKeyType_eOdd );
    printf("\n");

    printf("load cw to even key slot...\n");
    test_load_cw(ck, ecm,DrmSecurityKeyType_eEven  );
    printf("\ndecrypting using even key slot ...(the expected result should be all '55')\n");
    memcpy(wv_enc_buf, enc_buf, sizeof(enc_buf));
    memset(wv_dec_buf, 0, sizeof(enc_buf));
    test_decrypt(wv_enc_buf, wv_dec_buf, iv, DrmSecurityKeyType_eEven );

    printf("\n\nload fixed key to odd key slot...\n");
    test_load_cw_fixed(fixed, ecm, DrmSecurityKeyType_eOdd );
    printf("\ndecrypting using odd key slot ...(the expected result should be all '55')\n");
    memcpy(wv_enc_buf, enc_buf, sizeof(enc_buf));
    memset(wv_dec_buf, 0, sizeof(enc_buf));
    test_decrypt(wv_enc_buf, wv_dec_buf, iv, DrmSecurityKeyType_eOdd);

    printf("\n\nload fixed key to even key slot...\n");
    test_load_cw_fixed(fixed, ecm, DrmSecurityKeyType_eEven );
    printf("\ndecrypting using odd key slot ...(the expected result should be all '55')\n");
    memcpy(wv_enc_buf, enc_buf, sizeof(enc_buf));
    memset(wv_dec_buf, 0, sizeof(enc_buf));
    test_decrypt(wv_enc_buf, wv_dec_buf, iv, DrmSecurityKeyType_eEven);

    printf("\tMAIN - finished testing\n");

    DRM_Widevine_Finalize();

    if( dec != NULL) BKNI_Free(dec);
    if( iv != NULL) BKNI_Free(iv);
    if( ecm != NULL) BKNI_Free(ecm);
    if( ck != NULL) BKNI_Free(ck);
    if( k != NULL) BKNI_Free(k);
#endif

    return 0;
}


#ifdef TEST_ENCRYPTION_DECRYPTION
void test_decrypt(uint8_t *enc, uint8_t *buf, uint8_t* iv, DrmSecurityKeyType keyslot )
{
    DrmRC rc;
    int ret = 1;
    uint32_t i = 0;
    
    BSTD_UNUSED(buf);

    printf("Encrypted value: \n\t");
    for(i = 0; i < test_buf_len; i++)
    {
    	printf("0x%02x ",enc[i]);
    	if((i+1)%16 == 0)printf("\n\t");
    }
    printf("\n");

    rc = DRM_Widevine_Decrypt(enc, test_buf_len-2,
#ifdef IN_PLACE_DECRYPTION
							NULL, iv,
							DrmDestinationType_eInPlace,
#else
							buf, iv,
							DrmDestinationType_eExternal,
#endif
							keyslot);
    if(rc != Drm_Success) 
	{
        printf("test_encryption - drm_wv_encrypt failed %d\n", rc);
    }

    for(i = 0; i < test_buf_len; i++)
    {
#ifdef IN_PLACE_DECRYPTION
	if(enc[i] != 0x55){
		  ret = 0;
	}
#else
	if(buf[i] != 0x55){
		  ret = 0;
	}
#endif
    }
    
    printf("Decrypted value: \n\t");
    for(i = 0; i < test_buf_len; i++)
    {
#ifdef IN_PLACE_DECRYPTION
    	printf("0x%02x ", enc[i]);
#else
    	printf("0x%02x ", buf[i]);
#endif
    	if((i+1)%16 == 0)printf("\n\t");
    }
    printf("\n");

    if (ret == 1){
    	printf("Success!\n");
    }
}




void test_load_cw(uint8_t* common_key, uint8_t* ecm,  DrmSecurityKeyType keyslot)
{
    DrmRC rc;
	uint8_t id[4] = {0};

	rc = DRM_Widevine_LoadCw(common_key, ecm, id, keyslot);
    if(rc != Drm_Success) 
	{
        printf("test_load_cw - drm_wv_load_cw failed, rc %d\n", rc);
    }
	else
	{
		printf("CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n", id[0], id[1], id[2], id[3]);
	}
}

void test_load_cw_fixed(uint8_t* fixed_key, uint8_t* ecm,  DrmSecurityKeyType keyslot)
{
    DrmRC rc;
    uint8_t id[4] = {0};

	rc = DRM_Widevine_LoadCwFixed(fixed_key, ecm, id, keyslot);
    	if(rc != Drm_Success) 
	{
        printf("test_load_cw - drm_wv_load_cw failed, rc %d\n", rc);
    	}
	else
	{
		printf("test_load_cw - CW ID: 0x%02x 0x%02x 0x%02x 0x%02x\n", id[0], id[1], id[2], id[3]);
	}
}
#endif

void test_get_keyID()
{
    DrmRC rc;
     int i;

	printf("\n\t***************** test_get_keyID - Entered function\n");

	    rc = DRM_Widevine_GetKeyId(key_id);
	    if(rc != Drm_Success) 
		{
		printf("test_get_keyID - Call to 'DRM_Widevine_GetKeyId' failed\n");
		return;
	    }
		else
		{
		printf("\tKey ID: ");
			for(i = 0; i < DRM_WIDEVINE_KEYID_SIZE; i++)
			{
				printf("%02x", key_id[i]);
			}
			printf("\n");
		}
	return;
}



void test_get_deviceID()
{
    DrmRC rc;
	int i;
	
	printf("\n\t***************** test_get_deviceID - Entered function\n");
	
	rc = DRM_Widevine_GetDeviceId(device_id);
	    if(rc != Drm_Success) 
		{
		printf("test_get_deviceID - Call to 'DRM_Widevine_GetDeviceId' failed\n");
		return;
	}
	else
	{
		printf("\tDevice ID: ");
			for(i = 0; i < DRM_WIDEVINE_DEVICE_ID_SIZE; i++)
			{
			printf("%c", device_id[i]);
			}
			printf("\n");
		}

    return;
}




