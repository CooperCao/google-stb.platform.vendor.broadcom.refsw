/***************************************************************************
 *    (c)2012 Broadcom Corporation
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
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_custom_private.h"
#include "drm_metadata.h"
#include "drm_common.h"

int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	uint32_t i = 0;
	uint8_t *pDmaBuf = NULL;
	uint32_t dataSize = 0;
	NEXUS_PlatformSettings platformSettings;

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	/* initialize PMC */
    rc = DRM_CustomPrivate_Init(argv[1] /*DRM bin*/);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error calling DRM_CustomPrivate_Init...\n");
    	return 0;
    }

    /********************************************************************
     * DrmCustomPrivateDataType_eSharedCert_Id1
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id1, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id1, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedCert_Id1 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eSharedCert_Id2
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id2, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id2, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedCert_Id2 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eSharedCert_Id3
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id3, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedCert_Id3, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedCert_Id3 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eSharedKey_Id1
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id1, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id1, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedKey_Id1 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eSharedKey_Id2
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id2, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id2, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedKey_Id2 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);


    /********************************************************************
     * DrmCustomPrivateDataType_eSharedKey_Id3
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id3, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eSharedKey_Id3, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eSharedKey_Id3 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);




    /********************************************************************
     * DrmCustomPrivateDataType_eUniqueKey_Id1
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id1, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id1, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueKey_Id1 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eUniqueKey_Id2
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id2, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id2, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueKey_Id2 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eUniqueKey_Id3
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id3, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueKey_Id3, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueKey_Id3 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);


    /********************************************************************
     * 	DrmCustomPrivateDataType_eUniqueCert_Id1
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id1, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id1, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueCert_Id1 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);


    /********************************************************************
     * 	DrmCustomPrivateDataType_eUniqueCert_Id2
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id2, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id2, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

    printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueCert_Id2 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

    /********************************************************************
     * DrmCustomPrivateDataType_eUniqueCert_Id3
    ********************************************************************/
    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id3, NULL, &dataSize);

    /* Allocate a continuous physical address buffer for DMA */
    NEXUS_Memory_Allocate(dataSize, NULL,(void **) &pDmaBuf);

    rc = DRM_CustomPrivate_GetData(DrmCustomPrivateDataType_eUniqueCert_Id3, pDmaBuf, &dataSize);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

	printf("\n\tMAIN - DrmCustomPrivateDataType_eUniqueCert_Id3 (datasize = '%u'):\n\t", dataSize);
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

	NEXUS_Memory_Free(pDmaBuf);

	/* Close down module */

	printf("\tMAIN - Calling DRM_CustomPrivate_UnInit\n");
	DRM_CustomPrivate_UnInit();

	printf("\n\tMAIN - Done\n");
    return 0;
}


