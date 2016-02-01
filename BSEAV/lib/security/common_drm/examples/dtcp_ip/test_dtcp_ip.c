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
#include "drm_dtcp_ip.h"
#include "drm_metadata.h"
#include "drm_common.h"

#include "bstd.h"
#include "bdbg.h" 
#include "bkni.h"


int main(int argc, char* argv[])
{
	DrmRC rc = Drm_Success;
	uint32_t i = 0;
	NEXUS_PlatformSettings platformSettings;
	drm_dtcp_ip_data_t dtcpIpData;

	/* init Nexus */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

	/* initialize PMC */
    rc = DRM_DtcpIp_Initialize(argv[1] /*DRM bin*/);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error calling DRM_DtcpIp_Initialize...\n");
    	return 0;
    }

    /********************************************************************
     * ROOT DIGEST
    ********************************************************************/
    rc = DRM_DtcpIp_GetData(&dtcpIpData);
    if(rc != Drm_Success)
    {
    	printf("\n\t###   MAIN - Error fetching data...\n");
    	return 0;
    }

	printf("\n\tMAIN - ACoefficientCurve :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.ACoefficientCurve); i++)
	{
		printf("0x%02x ", dtcpIpData.ACoefficientCurve[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - BCoefficientCurve :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.BCoefficientCurve); i++)
	{
		printf("0x%02x ", dtcpIpData.BCoefficientCurve[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}


	printf("\n\tMAIN - BaselineFullCert :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.BaselineFullCert); i++)
	{
		printf("0x%02x ", dtcpIpData.BaselineFullCert[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - DtlaPublicKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.DtlaPublicKey); i++)
	{
		printf("0x%02x ", dtcpIpData.DtlaPublicKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - Ksv :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.Ksv); i++)
	{
		printf("0x%02x ", dtcpIpData.Ksv[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - OrderBasePoint :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.OrderBasePoint); i++)
	{
		printf("0x%02x ", dtcpIpData.OrderBasePoint[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - PrimeGF :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.PrimeGF); i++)
	{
		printf("0x%02x ", dtcpIpData.PrimeGF[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - PrivateKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.PrivateKey); i++)
	{
		printf("0x%02x ", dtcpIpData.PrivateKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCert :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCert); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCert[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCopyNoMoreSinkKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCopyNoMoreSinkKey); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCopyNoMoreSinkKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCopyNoMoreSourceKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCopyNoMoreSourceKey); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCopyNoMoreSourceKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCopyOnceSinkKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCopyOnceSinkKey); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCopyOnceSinkKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCopyOnceSourceKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCopyOnceSourceKey); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCopyOnceSourceKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - YBasePoint :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.YBasePoint); i++)
	{
		printf("0x%02x ", dtcpIpData.YBasePoint[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}

	printf("\n\tMAIN - RestrictedCopyNoMoreSourceKey :\n\t");
	for(i = 0; i < sizeof(dtcpIpData.RestrictedCopyNoMoreSourceKey); i++)
	{
		printf("0x%02x ", dtcpIpData.RestrictedCopyNoMoreSourceKey[i]);
		if((i+1)%16 == 0)printf("\n\t");
	}


	printf("\n\tMAIN - Done\n");
    return 0;
}


