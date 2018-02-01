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
#include "drm_hdcp_tx.h"
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
	DrmHdcpTxSettings_t hdcpTxSettings;
	drm_hdcp_tx_data_t hdcpTxData;
	DrmCommon_RsaSwParam_t 	rsaSwIO;
	uint8_t *ptrToStruct = (uint8_t*)&hdcpTxData;
	DrmCommon_RsaKey_t rsaKey;
	uint8_t ctext[32] = {	0x66, 0x28, 0x19, 0x4e, 0x12, 0x07, 0x3d, 0xb0, 0x3b, 0xa9, 0x4c, 0xda, 0x9e, 0xf9, 0x53, 0x23,
	   					    0x97, 0xd5, 0x0d, 0xba, 0x79, 0xb9, 0x87, 0x00, 0x4a, 0xfe, 0xfe, 0x34, 0x4a, 0xfe, 0xfe, 0x34 };
	const char* testValue1 = "Everyone gets Friday off.";
	uint8_t signedData[128] =  {0x00};
	unsigned long outDataLen = 0;

	uint8_t n1[]	= {0x0A, 0x66, 0x79, 0x1D, 0xC6, 0x98, 0x81, 0x68, 0xDE, 0x7A, 0xB7, 0x74,
                       0x19, 0xBB, 0x7F, 0xB0, 0xC0, 0x01, 0xC6, 0x27, 0x10, 0x27, 0x00, 0x75,
                       0x14, 0x29, 0x42, 0xE1, 0x9A, 0x8D, 0x8C, 0x51, 0xD0, 0x53, 0xB3, 0xE3,
                       0x78, 0x2A, 0x1D, 0xE5, 0xDC, 0x5A, 0xF4, 0xEB, 0xE9, 0x94, 0x68, 0x17,
                       0x01, 0x14, 0xA1, 0xDF, 0xE6, 0x7C, 0xDC, 0x9A, 0x9A, 0xF5, 0x5D, 0x65, 0x56, 0x20, 0xBB, 0xAB};

	uint8_t d1[]	= {0x01, 0x23, 0xC5, 0xB6, 0x1B, 0xA3, 0x6E, 0xDB, 0x1D, 0x36, 0x79, 0x90, 0x41,
                       0x99, 0xA8, 0x9E, 0xA8, 0x0C, 0x09, 0xB9, 0x12, 0x2E, 0x14, 0x00, 0xC0, 0x9A,
                       0xDC, 0xF7, 0x78, 0x46, 0x76, 0xD0, 0x1D, 0x23, 0x35, 0x6A, 0x7D, 0x44, 0xD6,
                       0xBD, 0x8B, 0xD5, 0x0E, 0x94, 0xBF, 0xC7, 0x23, 0xFA, 0x87, 0xD8, 0x86, 0x2B,
                       0x75, 0x17, 0x76, 0x91, 0xC1, 0x1D, 0x75, 0x76, 0x92, 0xDF, 0x88, 0x81};

	uint8_t p1[]	= {0x33, 0xD4, 0x84, 0x45, 0xC8, 0x59, 0xE5, 0x23, 0x40, 0xDE, 0x70, 0x4B, 0xCD,
                       0xDA, 0x06, 0x5F, 0xBB, 0x40, 0x58, 0xD7, 0x40, 0xBD, 0x1D, 0x67, 0xD2, 0x9E,
                       0x9C, 0x14, 0x6C, 0x11, 0xCF, 0x61};

	uint8_t q1[]	= {0x33, 0x5E, 0x84, 0x08, 0x86, 0x6B, 0x0F, 0xD3, 0x8D, 0xC7, 0x00,
                       0x2D, 0x3F, 0x97, 0x2C, 0x67, 0x38, 0x9A, 0x65, 0xD5, 0xD8, 0x30,
                       0x65, 0x66, 0xD5, 0xC4, 0xF2, 0xA5, 0xAA, 0x52, 0x62, 0x8B};

	uint8_t dp11[]	= {0x04, 0x5E, 0xC9, 0x00, 0x71, 0x52, 0x53, 0x25, 0xD3, 0xD4, 0x6D, 0xB7,
                       0x96, 0x95, 0xE9, 0xAF, 0xAC, 0xC4, 0x52, 0x39, 0x64, 0x36, 0x0E, 0x02,
                       0xB1, 0x19, 0xBA, 0xA3, 0x66, 0x31, 0x62, 0x41};

	uint8_t dq11[]	= {0x15, 0xEB, 0x32, 0x73, 0x60, 0xC7, 0xB6, 0x0D, 0x12, 0xE5,
                       0xE2, 0xD1, 0x6B, 0xDC, 0xD9, 0x79, 0x81, 0xD1, 0x7F, 0xBA,
                       0x6B, 0x70, 0xDB, 0x13, 0xB2, 0x0B, 0x43, 0x6E, 0x24, 0xEA, 0xDA, 0x59};

	uint8_t pq1[]	= {0x2C, 0xA6, 0x36, 0x6D,
                       0x72, 0x78, 0x1D, 0xFA,
                       0x24, 0xD3, 0x4A, 0x9A,
                       0x24, 0xCB, 0xC2, 0xAE,
                       0x92, 0x7A, 0x99, 0x58,
                       0xAF, 0x42, 0x65, 0x63,
                       0xFF, 0x63, 0xFB, 0x11,
                       0x65, 0x8A, 0x46, 0x1D};

	 uint8_t pub_exp[4]={0x01,0x00,0x01,0x00};

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /* get default settings */
    DRM_HdcpTx_GetDefaultParamSettings(&hdcpTxSettings);

    /* overwrite if command line argv[1] is not null */
    hdcpTxSettings.drm_bin_file_path = argv[1];

	/* initialize */
    rc = DRM_HdcpTx_Initialize(&hdcpTxSettings);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error calling DRM_HdcpTx_Initialize...\n");
    	return 0;
    }

    rc = DRM_HdcpTx_GetData(&hdcpTxData);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    sleep(1);
	printf("\n\tMAIN - structure :\n\n\t");
	for(i = 0; i < sizeof(drm_hdcp_tx_data_t); i++)
	{
		printf("%02x ", ptrToStruct[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}


	/* Perform RSA Sign/Verify operation */
	rsaKey.e.pData = pub_exp;
	rsaKey.e.len=4;

	rsaKey.n.pData = n1;
	rsaKey.n.len = sizeof(n1);

	rsaKey.d.pData = d1;
	rsaKey.d.len = sizeof(d1);

	rsaKey.p.pData = p1;
	rsaKey.p.len = sizeof(p1);
	rsaKey.q.pData = q1;
	rsaKey.q.len = sizeof(q1);
	rsaKey.dmp1.pData = dp11;
	rsaKey.dmp1.len = sizeof(dp11);
	rsaKey.dmq1.pData = dq11;
	rsaKey.dmq1.len = sizeof(dq11);
	rsaKey.iqmp.pData = pq1;
	rsaKey.iqmp.len = sizeof(pq1);

	printf("\n\tMAIN - set key parameters\n");
    rsaSwIO.bRSAop		= drmRsasign;
    rsaSwIO.key 		= &rsaKey;
    rsaSwIO.pbDataIn 	= ctext;
    rsaSwIO.cbDataIn	= 32;
    rsaSwIO.pbDataOut	= signedData;
    rsaSwIO.cbDataOut	= &outDataLen;

    DRM_Common_SwRsa(&rsaSwIO);


    sleep(1);
	printf("\n\tMAIN - signature :\n\n\t");
	for(i = 0; i < outDataLen; i++)
	{
		printf("%02x ", signedData[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - Done\n");

    /* Clean up test environment */
    DRM_HdcpTx_Finalize();
    NEXUS_Platform_Uninit();

    return 0;
}
