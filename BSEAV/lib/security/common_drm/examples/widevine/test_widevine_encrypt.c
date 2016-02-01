/***************************************************************************
 *    (c)2010-2013 Broadcom Corporation
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
 *  This test app validates the new Widevine Key encryption API  
 * Revision History:
 * 
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>


#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_widevine.h"
#include "drm_metadata.h"


#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

static void test_get_deviceID(void);


/* Initialize the key used to encrypt the refence buffers with bdvd_crypto functions*/
/* to a know value. For this test to work, the drm bin file must use the same key.*/
/* #define TEST_ENCRYPTION_DECRYPTION */

static 	uint8_t device_id[DRM_WIDEVINE_DEVICE_ID_SIZE] = {0x00};



uint8_t enc_asset[16] = {0xf1, 0x24, 0x60, 0x4b, 0xb0, 0xfc, 0xe9, 0x0c, 0x00, 0x68, 0x5e, 0x85, 0x2b, 0x73, 0xbf, 0x57};
uint8_t iseg_iv[16] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t wrapped_ecm[16] = {0xD8, 0x1A, 0x06, 0xE9, 0x37, 0x31, 0xB3, 0x7E, 0x20, 0x10, 0xCA, 0xE7, 0xB7, 0x0E, 0x14, 0xD7};




int main(int argc, char* argv[])
{
   
   uint8_t *ecm = NULL;
   uint8_t *iv = NULL;
   int i = 0;
   DRM_Widevine_Init_t inputWvInitStruct;
   char  *file_sequence_enc_name;
   char  *file_sequence_dec_name;
   FILE  *file_sequence_enc=NULL;
   FILE  *file_sequence_dec=NULL;
   DrmRC rc;
   uint8_t*    decbuff;
   uint8_t*    encbuff;
   uint8_t*    outbuff;
   int readsz1=0;
   int readsz2=0;

   NEXUS_PlatformSettings platformSettings;
   NEXUS_Platform_GetDefaultSettings(&platformSettings); 
   platformSettings.openFrontend = false;
   NEXUS_Platform_Init(&platformSettings);

	ecm = BKNI_Malloc(16);
    iv  = BKNI_Malloc(16);

    inputWvInitStruct.mode = DRM_WidevineDecryptMode_eVod;
   	inputWvInitStruct.key_file = argv[1];

	if(DRM_Widevine_Initialize(inputWvInitStruct) != Drm_Success)
    {
        printf("\n\tMAIN - failed to Initialize Widevine DRM module **************\n");
        return 0;
    }

 
	file_sequence_dec_name = argv[2];
	file_sequence_enc_name = argv[3];


	file_sequence_enc=fopen(file_sequence_enc_name, "rb");
	printf("\n  Reading encrypted file file %s...",file_sequence_enc_name);
	if(file_sequence_enc==NULL)
	{
		printf("\n Error Reading Encrypted file %s...",file_sequence_enc_name);	
	    goto Exit;
	}

	file_sequence_dec=fopen(file_sequence_dec_name, "rb");
		printf("\n  Reading decrypted file file %s...",file_sequence_dec_name);
	if(file_sequence_dec==NULL)
	{
		printf("\n Error Reading Decrypted  file...");	
	    goto Exit;
	}

   test_get_deviceID();

	
   /* load the ECM into the KS 1 */

   rc = DRM_Widevine_LoadAssetKey(enc_asset, 16);

   if(rc != Drm_Success) 
	{
        printf("DRM_Widevine_LoadAssetKey failed, rc %d\n", rc);
	    goto Exit;
    }
	else
	{
		printf("DRM_Widevine_LoadAssetKey success\n");
	}

   /* generate the ECM and iv  and load it into KS2 */
   printf("Calling the DRM_Widevine_GenerateEcm \n");
    rc =  DRM_Widevine_GenerateEcm(ecm, iv);
	if(rc != Drm_Success) 
	{
        printf("DRM_Widevine_GenerateEcm failed, rc %d\n", rc);
    }
	else
	{
		printf("\n\nDRM_Widevine_GenerateEcm success\n");
        if ( BKNI_Memcmp(ecm,wrapped_ecm,16) !=0 )
		{
			printf("\n\n ====> Wrapped ECM does not match  ... :-( \n"); 
		}
		else
		{
			printf("\n\n ====> Wrapped ECM Matches ...  :-) \n");
		}
		

		printf("\n\nEncrypted ECM: \n\t");
		for(i = 0; i < 16; i++)
        {
    	printf("0x%02x ",ecm[i]);
    	if((i+1)%16 == 0)printf("\n\t");
        }
        printf("\n");

		printf("IV: \n\t");
		for(i = 0; i < 16; i++)
        {
    	printf("0x%02x ",iv[i]);
    	if((i+1)%16 == 0)printf("\n\t");
        }
        printf("\n");
	}

   /*encrypt the decrypted ts stream using the key in KS2 and provided IV */ 

		
    NEXUS_Memory_Allocate(20*1024, NULL,(void **) &decbuff);
    NEXUS_Memory_Allocate(20*1024, NULL,(void **) &encbuff);
	NEXUS_Memory_Allocate(20*1024, NULL,(void **) &outbuff);
	memset(decbuff, 0, 20*1024);
	memset(encbuff, 0, 20*1024);
	memset(outbuff , 0, 20*1024);
	readsz1=fread(decbuff,sizeof(uint8_t),20*1024,file_sequence_dec);
    if(readsz1==0)
	   goto Exit;
	printf("Decrypted Buffer read from file: ");
    for(i = 0; i < 16; i++)
    {
    	printf("0x%02x ", decbuff[i]);
    }
    printf("\n");
	readsz2=fread(encbuff,sizeof(uint8_t),20*1024,file_sequence_enc);
    if(readsz2==0)
	   goto Exit;
	printf("Encrypted Buffer read from file: ");
    for(i = 0; i < 16; i++)
    {
    	printf("0x%02x ", encbuff[i]);
    }
    printf("\n");

	rc = DRM_Widevine_Encrypt(decbuff,readsz1,iseg_iv,16,outbuff);

   /*Compare the encrypted result with the test data */ 
	
	printf("Encrypted Buffer after DMA operation: ");
    for(i = 0; i < 16; i++)
    {
    	printf("0x%02x ", outbuff[i]);
    }
    printf("\n");


    if (BKNI_Memcmp(encbuff,outbuff, readsz1) !=0 )
	{
		 printf("\n\n =====> Data does not match  ... :-(\n"); 

	}else
	{
		printf("\n\n  =====> Data Matches ...  :-) \n");
	}


Exit:
	if( decbuff != NULL) NEXUS_Memory_Free(decbuff);
	if( encbuff != NULL) NEXUS_Memory_Free(encbuff);
	if( outbuff != NULL) NEXUS_Memory_Free(outbuff);
	fclose(file_sequence_enc);
	fclose(file_sequence_dec);
    return 0;
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




