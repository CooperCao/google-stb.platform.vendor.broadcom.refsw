/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_hdcp_rx.h"
#include "drm_metadata.h"
#include "drm_common.h"
#include "drm_common_swcrypto.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"


int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	uint32_t i = 0;
	NEXUS_PlatformSettings platformSettings;
	DrmHdcpRxSettings_t hdcpRxSettings;
	drm_hdcp_rx_data_t hdcpRxData;
	DrmCommon_RsaSwParam_t 	rsaSwIO;
	uint8_t *ptrToStruct = (uint8_t*)&hdcpRxData;
	DrmCommon_RsaKey_t rsaKey;
    uint8_t ctext[9] = {0xbc, 0x65, 0x67, 0x47, 0xfa, 0x9e, 0xaf, 0xb3, 0xf0 };
	const char algo[]="RSA-SHA1";
	uint8_t signedData[128] =  {0x00};
	uint8_t sha1[20] =  {0x00};
	unsigned long outDataLen = 0;

	 uint8_t pub_exp[3] ={0x01, 0x00, 0x01};

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /* get default settings */
    DRM_HdcpRx_GetDefaultParamSettings(&hdcpRxSettings);

    /* overwrite if command line argv[1] is not null */
    hdcpRxSettings.drm_bin_file_path = argv[1];

	/* initialize */
    rc = DRM_HdcpRx_Initialize(&hdcpRxSettings);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error calling DRM_HdcpRx_Initialize...\n");
    	return 0;
    }

    rc = DRM_HdcpRx_GetData(&hdcpRxData);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    sleep(1);
	printf("\n\tMAIN - structure :\n\n\t");
	for(i = 0; i < sizeof(drm_hdcp_rx_data_t); i++)
	{
		printf("%02x ", ptrToStruct[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}


	/* Perform RSA Sign/Verify operation */
	BKNI_Memset((uint8_t*)&rsaSwIO, 0x00, sizeof(DrmCommon_RsaKey_t));
	rsaKey.e.pData = hdcpRxData.device_pub_exponent;
	rsaKey.e.len= 3;

	rsaKey.n.pData = hdcpRxData.device_pub_modulus;
	rsaKey.n.len = 128;

	rsaKey.d.len = 0;

	rsaKey.p.pData = hdcpRxData.p;
	rsaKey.p.len = 64;
	rsaKey.q.pData = hdcpRxData.q;
	rsaKey.q.len = 64;
	rsaKey.dmp1.pData = hdcpRxData.d_mod_p1;
	rsaKey.dmp1.len = 64;
	rsaKey.dmq1.pData = hdcpRxData.d_mod_q1;
	rsaKey.dmq1.len = 64;
	rsaKey.iqmp.pData = hdcpRxData.q1_mod_p;
	rsaKey.iqmp.len = 64;

	rsaSwIO.psAlgorithmId = (unsigned char *)algo;
	rsaSwIO.csAlgorithmId = sizeof(algo);

	printf("\n\tMAIN - set key parameters for signing\n");
    rsaSwIO.bRSAop		= drmRsasign_crt;
    rsaSwIO.key 		= &rsaKey;
    rsaSwIO.pbDataOut	= signedData;
    rsaSwIO.cbDataOut	= &outDataLen;

    DRM_Common_SwSha1(ctext, sha1, 9);
    rsaSwIO.pbDataIn 	= sha1;
    rsaSwIO.cbDataIn	= 20;

    printf("\n\tMAIN - Calling 'DRM_Common_SwRsa'\n");
    DRM_Common_SwRsa(&rsaSwIO);

    sleep(1);
	printf("\n\tMAIN - signature :\n\n\t");
	for(i = 0; i < outDataLen; i++)
	{
		printf("%02x ", signedData[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - set key parameters for verifying\n");
	BKNI_Memset((uint8_t*)&rsaSwIO, 0x00, sizeof(DrmCommon_RsaKey_t));
	BKNI_Memset((uint8_t*)&rsaKey, 0x00, sizeof(DrmCommon_RsaKey_t));

	rsaKey.n.pData = hdcpRxData.device_pub_modulus;
	rsaKey.n.len = 128;
	rsaKey.e.pData = hdcpRxData.device_pub_exponent;
	rsaKey.e.len= 3;
	rsaKey.p.len = 0;
	rsaKey.q.len = 0;
	rsaKey.dmp1.len = 0;
	rsaKey.dmq1.len = 0;
	rsaKey.iqmp.len = 0;

    rsaSwIO.bRSAop		= drmRsaverify;
    rsaSwIO.key 		= &rsaKey;

    outDataLen = 128;
    rsaSwIO.pbDataIn 	= sha1;
    rsaSwIO.cbDataIn	= 20;
    rsaSwIO.pbDataOut	= signedData;
    rsaSwIO.cbDataOut	= &outDataLen;


    rsaSwIO.psAlgorithmId = (unsigned char *)algo;
    rsaSwIO.csAlgorithmId = sizeof(algo);


    DRM_Common_SwRsa(&rsaSwIO);


    sleep(1);

	printf("\n\tMAIN - Done\n");

    /* Clean up test environment */
    DRM_HdcpRx_Finalize();
    NEXUS_Platform_Uninit();

    return 0;
}
