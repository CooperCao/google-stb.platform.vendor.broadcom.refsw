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
 * $brcm_Workfile: save_utility_rsa_host.c $
 * $brcm_Revision: 1 $
 * $brcm_Date: 6/14/14 12:41p $
 *
 * Module Description: SAGE DRM bin file Validation tool (host-side)

 ******************************************************************************/

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "bstd.h"
#include "bkni.h"

#include "bsagelib_types.h"
#include "bsagelib_tools.h"
#include "bsagelib_crypto_types.h"

#include "sage_srai.h"

#include "utility_ids.h"
#include "utility_platform.h"
#include "rsa_tl.h"


#include "nexus_platform.h"
#include "nexus_platform_init.h"

#include "drm_metadata_tl.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

BDBG_MODULE(sage_utility_rsa_tool);

/* defines */
#define MAX_ARG_STRING_LENGTH         (256)
#define SIZEOF_DRM_BINFILE_HEADER     (192)
#define SIZEOF_DYNAMIC_OFFSET_HEADER  (16)
#define DEFAULT_DRM_BINFILE_PATH      "./drm.bin"

#define GET_UINT32_FROM_BUF(pBuf) \
    (((uint32_t)(((uint8_t*)(pBuf))[0]) << 24) | \
     ((uint32_t)(((uint8_t*)(pBuf))[1]) << 16) | \
     ((uint32_t)(((uint8_t*)(pBuf))[2]) << 8)  | \
     ((uint8_t *)(pBuf))[3])


static RsaTl_Handle hRsaTl = NULL;

static int SAGE_app_join_nexus(void);
static void SAGE_app_leave_nexus(void);
static void print_help(void);



static int SAGE_app_join_nexus(void)
{
    int rc = 0;
    NEXUS_PlatformSettings platformSettings;

    BDBG_LOG(("\t*** Bringing up all Nexus modules for platform using default settings\n\n\n"));

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;

    if(NEXUS_Platform_Init(&platformSettings) == NEXUS_SUCCESS){
        BDBG_LOG(("\t*** Nexus has initialized successfully\n\n"));
        rc = 0;
    }
    else{
        BDBG_ERR(("\t### Failed to bring up Nexus\n\n"));
        rc = 1;
    }
    return rc;
}

static void SAGE_app_leave_nexus(void)
{
    NEXUS_Platform_Uninit();
}







int SAGE_Utility_Rsa_ProcessBinFile(void)
{
    int ii =0;
    int rc = 0;
    uint8_t *ptr = NULL;
    uint8_t srcData[64] = {0};
    uint8_t signature[384] = {0};


    for(ii = 0; ii < 8; ii++)
    {
        /* allocated memory for signature (max possbile size is 384) */
        BKNI_Memset(signature, 0x00, sizeof(signature));
        BKNI_Memset(srcData, 0xFF, sizeof(srcData));

        if(RsaTl_SignVerify(hRsaTl,
                            Rsa_CommandId_eSign,
                            srcData, sizeof(srcData),
                            ii, BSAGElib_Crypto_ShaVariant_eSha256,
                            signature, sizeof(signature)) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error sending Sign command to SAGE (key index = '%u')", __FUNCTION__, ii));
            rc = 1;
            goto handle_error;
        }

        ptr = signature;
        BDBG_LOG(("Computed signature = %02x %02x %02x %02x ...", ptr[0], ptr[1], ptr[2], ptr[3]));

        if(RsaTl_SignVerify(hRsaTl,
                            Rsa_CommandId_eVerify,
                            srcData, sizeof(srcData),
                            ii, BSAGElib_Crypto_ShaVariant_eSha256,
                            signature, sizeof(signature)) != BERR_SUCCESS)
        {
            BDBG_ERR(("%s - Error sending Verify command to SAGE (key index = '%u')", __FUNCTION__, ii));
            rc = 1;
            goto handle_error;
        }
    }

handle_error:

    return rc;
}



/* main file
 * This platform can run in a standalone application or
 * be a part of a larger one.
 */
int main(int argc, char* argv[])
{
    static RsaSettings rsaModuleSettings;

#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;
    NxClient_AllocResults allocResults;
    int rc = NEXUS_SUCCESS;
#else
    BERR_Code rc = BERR_SUCCESS;
#endif


    BDBG_LOG(("\n\n\n\n\n"));
    BDBG_LOG(("***************************************************************************************"));
    BDBG_LOG(("***************************************************************************************"));
    BDBG_LOG(("***\tBroadcom Limited RSA test utility (Copyright 2014)"));
    BDBG_LOG(("***************************************************************************************"));
    BDBG_LOG(("***************************************************************************************\n"));

    if(argc > 2)
    {
        BDBG_ERR(("\tInvalid number of command line arguments (%u)\n", argc));
        print_help();
        rc = -1;
        goto handle_error;
    }

#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "util_rsa_host");
    joinSettings.ignoreStandbyRequest = true;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
#else
    rc = SAGE_app_join_nexus();
    if(rc != 0){
        goto handle_error;
    }
#endif

    /* Initialize Rsa TL module */
    RsaTl_GetDefaultSettings(&rsaModuleSettings);

    if(argv[1] != NULL)
    {
        if(strlen(argv[1]) > sizeof(rsaModuleSettings.drm_binfile_path))
        {
            BDBG_ERR(("\tString length of argument '%s' is too large (%u bytes)\n", argv[1], strlen(argv[1])));
            rc = 1;
            goto handle_error;
        }
        memcpy(rsaModuleSettings.drm_binfile_path, argv[1], strlen(argv[1]));
    }
    else
    {
        memcpy(rsaModuleSettings.drm_binfile_path, DEFAULT_DRM_BINFILE_PATH, strlen(DEFAULT_DRM_BINFILE_PATH));
    }

    BDBG_LOG(("\t***  Input file to validate = '%s'", rsaModuleSettings.drm_binfile_path));

    rc = RsaTl_Init(&hRsaTl, &rsaModuleSettings);
    rc = access(rsaModuleSettings.drm_binfile_path, R_OK);
    if(rc != 0)
    {
        BDBG_ERR(("### '%s' doesn't exist or is not readable\n", rsaModuleSettings.drm_binfile_path));
        print_help();
        return rc;
    }
    BDBG_LOG(("\n\n\n"));


    rc = SAGE_Utility_Rsa_ProcessBinFile();
    if (rc != 0) {
        goto handle_error;
    }


handle_error:

    RsaTl_Uninit(hRsaTl);
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    /* Leave Nexus: Finalize platform ... */
    BDBG_LOG(("Closing Nexus driver..."));
    SAGE_app_leave_nexus();
#endif

    if (rc) {
        BDBG_ERR(("Failure in SAGE Utility RSA test\n"));
    }

    return rc;
}


static void print_help(void)
{
    BDBG_LOG(("\tOption 1)    ./nexus brcm_utility_rsa_tool <path to DRM bin file>\n"));
    BDBG_LOG(("\tOption 2)    ./nexus brcm_utility_rsa_tool\n"));
    BDBG_LOG(("\t\tIn the case of option 2) the DRM bin file should named 'drm.bin' be placed in the same directory as the executable\n\n"));
    return;
}
