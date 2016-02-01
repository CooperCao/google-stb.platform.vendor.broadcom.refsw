/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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

#include "nexus_platform.h"

#include "nexus_security.h"
#include "nexus_keyladder.h"
#include "nexus_dma.h"
#ifdef B_HAS_DIVX_DRM
#include "nexus_divxdrm.h"
#endif

#include <stdio.h>
#include <string.h>

BDBG_MODULE(regcode);


#define HARDWARE_SECRET_LEN  NEXUS_DIVXDRM_HARDWARE_SECRET_LENGTH
#define FRAGMENT1_LEN        NEXUS_DIVXDRM_MEMORY_FRAGMENT1_LENGTH
#define FRAGMENT2_LEN        NEXUS_DIVXDRM_MEMORY_FRAGMENT2_LENGTH
#define FRAGMENT3_LEN        NEXUS_DIVXDRM_MEMORY_FRAGMENT3_LENGTH

NEXUS_Error Generate_HardwareKey(uint8_t *hardware_secret);


int main(int argc, char * argv[]) 
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_DivxDrmCreateSettings drmCreateSettings; 
    NEXUS_DivxDrmHandle divxDrm;
    NEXUS_DivxDrmCodeString codeString;
    uint8_t hardware_secret[HARDWARE_SECRET_LEN];
    NEXUS_DivxDrmState drmState;
    NEXUS_Error rc=NEXUS_SUCCESS;
    FILE *fp=NULL;
    
    if(argc<2){
	printf("Please specify an option.\n"
	       "Usage : nexus regcode option\n"
	       "\toption : register   (get registration code\n"
	       "\t       : deactivate (deactivate device)\n\n");
	
	return 0;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    
    rc = Generate_HardwareKey(hardware_secret);
    if(rc) {
	goto err_key;
    }
    
    if(!strcmp(argv[1], "key")) {
	unsigned i = 0;
        printf("Hardware key: \n");
        for(i = 0; i < HARDWARE_SECRET_LEN; i++)
        {
            printf("%02x", hardware_secret[i]);
        }
        printf("\n");
	return 0;
    }
    
    NEXUS_DivxDrm_GetDefaultCreateSettings(&drmCreateSettings);      
    BKNI_Memcpy(drmCreateSettings.drmHardwareSecret, hardware_secret, HARDWARE_SECRET_LEN);
    divxDrm = NEXUS_DivxDrm_Create(&drmCreateSettings);
    if(!divxDrm) {
	goto err_open;
    }

    BKNI_Memset(&drmState, 0, sizeof(drmState));
    /* Read DRM Memory from file */
    fp = fopen("playervis.db", "rb");
    if(fp) {	
	fread(drmState.fragment1, 1, FRAGMENT1_LEN, fp);
	fclose(fp);
	drmState.fragment1Length = FRAGMENT1_LEN;
    } 

    fp = fopen("ccoredm.dll", "rb");
    if(fp) {	
	fread(drmState.fragment2, 1, FRAGMENT2_LEN, fp);
	fclose(fp);
	drmState.fragment2Length = FRAGMENT2_LEN;
    } 

    fp = fopen("mskdmpw2.dll", "rb");
    if(fp) {		
	fread(drmState.fragment3, 1, FRAGMENT3_LEN, fp);
	fclose(fp);
	drmState.fragment3Length = FRAGMENT3_LEN;
    } 
		
    NEXUS_DivxDrm_SetDrmState(divxDrm, &drmState);


    if(!strcmp(argv[1], "register")) {
	NEXUS_DivxDrm_GetRegistrationCodeString(divxDrm, &codeString);
	
	BDBG_WRN(("**********************"));	
	BDBG_WRN(("REGISTRATION CODE : %s", codeString.codeString));
	BDBG_WRN(("**********************\n\n"));
	BDBG_WRN(("DOWNLOAD THE ACTIVATION VIDEO  USING THE REGISTRATION CODE")); 
	
    } else {
	NEXUS_DivxDrm_Deactivate(divxDrm, &codeString);
	
	BDBG_WRN(("**********************"));
	BDBG_WRN(("DEACTIVATION CODE : %s", codeString.codeString));
	BDBG_WRN(("**********************"));
	BDBG_WRN(( "DEVICE IS DEACTIVATED\n\n"));	
    }
    

    NEXUS_DivxDrm_GetDrmState(divxDrm, &drmState);

    fp = fopen("playervis.db", "wb");
    fwrite(drmState.fragment1, 1, FRAGMENT1_LEN, fp);
    fclose(fp);

    fp = fopen("ccoredm.dll", "wb");    
    fwrite(drmState.fragment2, 1, FRAGMENT2_LEN, fp);
    fclose(fp);

    fp = fopen("mskdmpw2.dll", "wb");    
    fwrite(drmState.fragment3, 1, FRAGMENT3_LEN, fp);
    fclose(fp);

    NEXUS_DivxDrm_Destroy(divxDrm);

err_open:
err_key:
    NEXUS_Platform_Uninit();

    return 0;
}
