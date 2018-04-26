/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ssdd.h"
#include "nexus_platform.h"
#include "nexus_platform_init.h"
#include "nexus_security.h"
#include "nxclient.h"

/* This define when set to 1 provides the ability to force enabling the RPMB
   initialisation even though the platform may not have RPMB or the version
   of BFW does not support a harware RPMB/Replay Key-ladder.  In a real
   production system, this define should always be set to 0 to allow auto-
   detection of whether the platform supports RPMB or not.
 */
#define FORCE_HAS_RPMB        0

static bool g_ssd_init;
static NxClient_AllocResults g_nxc_allocResults;
static int standbyId = 0;

static BERR_Code SSDD_Join_Nexus(void)
{
    int rc = 0;

    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings nxAllocSettings;

    SSDD_Debug_printf(("Bringing up nxclient\n"));
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "SSDD_TA");
    joinSettings.ignoreStandbyRequest = true;

    rc = NxClient_Join(&joinSettings);
    if (rc)
        return rc;
    standbyId = NxClient_RegisterAcknowledgeStandby();
    NxClient_GetDefaultAllocSettings(&nxAllocSettings);
    rc = NxClient_Alloc(&nxAllocSettings, &g_nxc_allocResults);
    if (rc) {
        NxClient_Uninit();
        return rc;
    }

    SSDD_Debug_printf(("nxclient has joined successfully\n"));

    return rc;
}

static void SSDD_Leave_Nexus(void)
{
    NxClient_UnregisterAcknowledgeStandby(standbyId);
    NxClient_Free(&g_nxc_allocResults);
    NxClient_Uninit();
    return;
}

static bool SSDD_IsRpmbAvailable(char *rpmb_path)
{
    bool hasRpmb = FORCE_HAS_RPMB;
    NEXUS_SecurityCapabilities securityCaps;
    struct stat buffer;

    NEXUS_GetSecurityCapabilities( &securityCaps );

    SSDD_Debug_printf("BFW Version=%u.%u.%u\n",
#if (NEXUS_SECURITY_API_VERSION==1)
            NEXUS_BFW_VERSION_MAJOR(securityCaps.version.firmware),
            NEXUS_BFW_VERSION_MINOR(securityCaps.version.firmware),
            NEXUS_BFW_VERSION_SUBMINOR(securityCaps.version.firmware));
#else
            securityCaps.version.bfw.major,
            securityCaps.version.bfw.minor,
            securityCaps.version.bfw.subminor);
#endif


    /* Zeus4:BFW version must be 4.2.3 or greater to support the RPMB/Replay key-ladder */
    /* Zeus5:RPMB/Replay key-ladder is supported */
#if (BHSM_ZEUS_VER_MAJOR < 5)
#if (NEXUS_SECURITY_API_VERSION==1)
    if (securityCaps.version.firmware >= NEXUS_BFW_VERSION_CALC(4,2,3)) {
#else
    if (NEXUS_BFW_VERSION_CALC(securityCaps.version.bfw.major, securityCaps.version.bfw.minor, securityCaps.version.bfw.subminor) >=
        NEXUS_BFW_VERSION_CALC(4,2,3))
#endif
#endif
    {
        SSDD_Debug_printf("RPMB path=\"%s\"\n", rpmb_path);

        /* Now check that RPMB path exists */
        if (stat(rpmb_path, &buffer) == 0) {
            hasRpmb = true;
        }
    }
    SSDD_Debug_printf("RPMB %s available\n", hasRpmb ? "is" : "isn't");

    return hasRpmb;
}

static BERR_Code SSDD_Init(ssdd_Settings *ssdd_settings)
{
    BERR_Code rc = BERR_SUCCESS;

    rc = SSDD_Join_Nexus();
    if (rc)
        return rc;

    /* Check RPMB availability */
    ssdd_settings->hasRpmb = SSDD_IsRpmbAvailable(ssdd_settings->rpmb_path);

    rc = SSDTl_Init(ssdd_settings);
    if (rc != BERR_SUCCESS) {
        SSDD_Debug_printf("Error initializing SSD TA\n");
    }

    g_ssd_init = true;
    return rc;
}

static void SSDD_Uninit(void)
{
    if (g_ssd_init) {
        SSDTl_Uninit();
        SSDD_Leave_Nexus();
        g_ssd_init = false;
    }
}

static void print_usage(char *progname, ssdd_Settings *ssdd_settings)
{
    printf("Usage: %s [Options]\n", progname);
    printf("Options:\n", progname);
    printf("    -h:      print this help message\n");
    printf("    -v:      specify path for VFS device [%s]\n", ssdd_settings->vfs_path);
    printf("    -r:      specify path for RPMB device [%s]\n", ssdd_settings->rpmb_path);
    printf("Development options (requires debug SSD TA):\n");
    printf("    -f:      format RPMB device on initialisation\n");
    printf("    -F:      format VFS device on initialisation\n");
}

int main(int argc, char *argv[])
{
    BERR_Code rc = BERR_SUCCESS;
    int c;
    char rpmb_path[NAME_MAX] = "";
    char vfs_path[NAME_MAX] = "";
    ssdd_Settings ssdd_settings;

    /* Get defailt settings */
    SSDTl_Get_Default_Settings(&ssdd_settings);

    /* Update according to command line parameters */
    while ((c = getopt(argc, argv, "v:r:fFh")) != -1) {
        switch(c) {
        case 'r':
            strncpy(rpmb_path, optarg, NAME_MAX);
            rpmb_path[NAME_MAX-1] = '\0';
            ssdd_settings.rpmb_path = rpmb_path;
            break;
        case 'v':
            strncpy(vfs_path, optarg, NAME_MAX);
            vfs_path[NAME_MAX-1] = '\0';
            ssdd_settings.vfs_path = vfs_path;
            break;
        case 'f':
            ssdd_settings.formatRpmb = true;
            break;
        case 'F':
            ssdd_settings.formatVfs = true;
            break;
        case 'h': /* fall-through */
        default:
            print_usage(argv[0], &ssdd_settings);
            return -1;
        }
    }

    /* Just check first whether we can take the lock */
    if (SSDD_Singleton_Lock_Is_Locked()) {
        printf("Error: SSDD lock currently held.\n");
        return -1;
    }

    SSDD_Daemonize();

    rc = SSDD_Singleton_Lock();
    if (rc) {
        SSDD_Debug_printf("Error: Failed to take SSDD lock\n");
        goto errorExit;
    }

    rc = SSDD_Debug_Open_Logfile();
    if (rc) {
        goto errorExit;
    }

    rc = SSDD_Init(&ssdd_settings);
    if (rc != BERR_SUCCESS) {
        goto errorExit;
    }

    SSDTl_Wait_For_Operations();

    SSDD_Debug_printf("Exiting SSDD test application\n");

errorExit:
    SSDD_Uninit();
    SSDD_Debug_Close_Logfile();
    SSDD_Singleton_Unlock();

    return rc;
}
