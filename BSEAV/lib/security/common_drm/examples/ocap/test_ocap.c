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
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "nexus_platform.h"
#include "nexus_memory.h"
#include "drm_ocap.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"

int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	drm_ocap_data_t ocap_data;
	uint32_t i = 0;
	uint32_t size = 0;
	uint8_t data[3072] = {0};
	NEXUS_PlatformSettings platformSettings;

	BSTD_UNUSED(argc);
	BSTD_UNUSED(argv);

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	/* initialize OCAP DRM */
    rc = DRM_Ocap_Initialize(NULL);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_Initialize() failed\n", __FUNCTION__);
		return 1;
	}

	/* get OCAP data */

	/* DRM_OCAP_DATA_CA */
	printf("Retrieving OCAP CA data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_CA, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("CA size: %u\n", size);
	printf("CA :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_ROOT */
	printf("Retrieving OCAP ROOT data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_ROOT, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("ROOT size: %u\n", size);
	printf("ROOT :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_CERT */
	printf("Retrieving OCAP CERT data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_CERT, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("CERT size: %u\n", size);
	printf("CERT :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_KEY */
	printf("Retrieving OCAP KEY data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_KEY, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("KEY size: %u\n", size);
	printf("KEY :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_BASE */
	printf("Retrieving OCAP BASE data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_BASE, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("BASE size: %u\n", size);
	printf("BASE :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_PRIME */
	printf("Retrieving OCAP PRIME data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_PRIME, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("PRIME size: %u\n", size);
	printf("PRIME :\n");
	for(i = 0; i < size; i++)
	{
		printf("0x%02x ", data[i]);
	}
	printf("\n\n");

	/* DRM_OCAP_DATA_FILENAME_KEY */
	printf("Retrieving OCAP FILENAME_KEY data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_FILENAME_KEY, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("FILENAME KEY size: %u\n", size);
	printf("FILENAME KEY: %s\n\n", data);

	/* DRM_OCAP_DATA_FILENAME_CERT */
	printf("Retrieving OCAP FILENAME_CERT data\n");
	rc = DRM_Ocap_GetDataBlock(data , DRM_OCAP_DATA_FILENAME_CERT, &size);
	if(rc != Drm_Success)
	{
		printf("%s - DRM_Ocap_GetDataBlock() failed\n", __FUNCTION__);
		return 1;
	}
	printf("FILENAME CERT size: %u\n", size);
	printf("FILENAME CERT: %s\n\n", data);

	/*  Finalize */
	DRM_Ocap_Finalize();

	printf("Done\n");
    return 0;
}
