/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "pmc_adobe.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

static uint8_t compareBuffer[10*1024] = {0x00};


int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	uint32_t i = 0;
	uint8_t *pDmaBuf = NULL;
	uint32_t dataSize = 0;
	size_t bytes_read = 0;
	FILE *fptr = NULL;
	bool compare_match = false;
	NEXUS_PlatformSettings platformSettings;

	if(argc > 3 )
	{
    	printf("\n\t###   MAIN - Test  usage error.Example:\n");
    	printf("\t###     ./nexus test_pmc_adobe pmc.bin <certificate or password file>\n");
    	rc = Drm_Err;
    	goto ErrorExit;
	}

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	/* initialize PMC */
    rc = DRM_Pmc_AdobeInit(argv[1] /*PMC bin*/);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error calling DRM_Pmc_AdobeInit...\n");
    	rc = Drm_Err;
    	goto ErrorExit;
    }

    if(argv[2] != NULL)
    {
		fptr = fopen(argv[2], "rb");
		if(fptr == NULL)
		{
			printf("\n\t###   MAIN - Error calling fopen(%s)   (%s)...\n", argv[2], strerror(errno));
			rc = Drm_Err;
			goto ErrorExit;
		}

		bytes_read = fread(compareBuffer, 1, sizeof(compareBuffer), fptr);
		if(bytes_read == 0)
		{
			printf("\n\t###   MAIN - Error reading into buffer. bytes_read = '%u'   (%s)...\n", bytes_read, strerror(errno));
			rc = Drm_Err;
			goto ErrorExit;
		}
		printf("\n\tMAIN - bytes_read = '%u' from '%s'...\n", bytes_read, argv[2]);
    }

    /********************************************************************
     * ROOT DIGEST
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eRootDigest, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of Root Digest...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eRootDigest, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching Root Digest...\n");
    	goto ErrorExit;
    }

	printf("\n\tMAIN - RootDigest :\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES RootDigest ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * TRANSPORT CERTIFICATE
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eTransportCert, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of Transport Certificate...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eTransportCert, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching Transport Certificate...\n");
    	goto ErrorExit;

    }

	printf("\n\tMAIN - TransportCert:\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ============================= File '%s' MATCHES Transport Certificate =========================\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * DEVICE CREDENTIAL
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eDeviceCred, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of Device Credential...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eDeviceCred, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching Device Credential...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - DeviceCred:\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}

	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Device Credential ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * 	DEVICE CREDENTIAL PASSWORD
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eDeviceCredPwd, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of Device Credential Password...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eDeviceCredPwd, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching Device Credential Password...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - DeviceCredPwd:\n\t");
	for(i = 0; i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Device Credential PWD ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN 0
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom0, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom0...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom0, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom0...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom0:\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}

	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 0 ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN PASSWORD 0
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom0Pwd, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom0 Password...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom0Pwd, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom0 Password...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom0Pwd :\n\t");
	for(i = 0; i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 0 PWD ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN 1
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom1, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom1...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom1, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom1...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom1:\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}

	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 1 ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN PASSWORD 1
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom1Pwd, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom1 Password...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom1Pwd, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom1 Password...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom1Pwd :\n\t");
	for(i = 0; i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 1 PWD ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN 2
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom2, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom2...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom2, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom2...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom2:\n\t");
	for(i = 0; i < 16; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("......\n\t.......");
	for(i = (dataSize-16); i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}

	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 2 ==================================\n\n", argv[2]);
		compare_match = true;
	}

	NEXUS_Memory_Free(pDmaBuf);
	pDmaBuf = NULL;

    /********************************************************************
     * SHARED DOMAIN PASSWORD 2
    ********************************************************************/
    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom2Pwd, NULL, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error determining length of SharedDom2 Password...\n");
        goto ErrorExit;
    }

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_Pmc_AdobeGetData(PmcAdobeDataType_eSharedDom2Pwd, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
        printf("\n\t###   MAIN - Error fetching SharedDom2 Password...\n");
        goto ErrorExit;
    }

	printf("\n\tMAIN - SharedDom2Pwd :\n\t");
	for(i = 0; i < dataSize; i++)
	{
		printf("0x%02x ", pDmaBuf[i]);
	}
	printf("\n\n");

	if(BKNI_Memcmp(compareBuffer, pDmaBuf, dataSize) == 0)
	{
		printf("\n\tMAIN - ====================== File '%s' MATCHES Shared Domain 2 PWD ==================================\n\n", argv[2]);
		compare_match = true;
	}

        NEXUS_Memory_Free(pDmaBuf);
        pDmaBuf = NULL;

ErrorExit:
	if(compare_match == false && argv[2] != NULL)
	{
		printf("\n\t###MAIN - Error: file '%s' does not match any field\n", argv[2]);
	}

	if(pDmaBuf != NULL){
		NEXUS_Memory_Free(pDmaBuf);
		pDmaBuf = NULL;
	}

	if(fptr != NULL) {
		fclose(fptr);
		fptr = NULL;
	}

	DRM_Pmc_AdobeUnInit();

	NEXUS_Platform_Uninit();
	printf("\n\tMAIN - Done cleanup\n");
    return rc;
}
