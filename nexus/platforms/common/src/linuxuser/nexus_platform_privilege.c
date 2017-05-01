/***************************************************************************
*  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
* API Description:
*   API name: Platform linuxuser
*    linuxuser OS routines
*
***************************************************************************/

#include "nexus_platform.h"
#include "nexus_platform_local_priv.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* this file runs in linuxuser and linuxuser.proxy, but not in linuxkernel */

BDBG_MODULE(nexus_platform_privilege);

NEXUS_Error NEXUS_Platform_P_DropPrivilege(const NEXUS_PlatformSettings *pSettings)
{
    int rc;
    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;

    if (!pSettings->permissions.userId && !pSettings->permissions.groupId) {
        /* nothing to drop */
        return 0;
    }

    rc = setresgid(pSettings->permissions.groupId, pSettings->permissions.groupId, pSettings->permissions.groupId);
    if (rc) {BERR_TRACE(rc); goto error;}

    rc = getresgid(&rgid, &egid, &sgid);
    if (rc) {BERR_TRACE(rc); goto error;}
    if (rgid != pSettings->permissions.groupId ||
        egid != pSettings->permissions.groupId ||
        sgid != pSettings->permissions.groupId) {BERR_TRACE(NEXUS_UNKNOWN); goto error;}

    rc = setresuid(pSettings->permissions.userId, pSettings->permissions.userId, pSettings->permissions.userId);
    if (rc) {BERR_TRACE(rc); goto error;}

    rc = getresuid(&ruid, &euid, &suid);
    if (rc) {BERR_TRACE(rc); goto error;}
    if (ruid != pSettings->permissions.userId ||
        euid != pSettings->permissions.userId ||
        suid != pSettings->permissions.userId) {BERR_TRACE(NEXUS_UNKNOWN); goto error;}

    rc = setuid(0);
    if (!rc) {BDBG_ERR(("able to get root privilege again")); goto error;}

    return 0;

error:
    BDBG_ERR(("unable to drop privilege. terminating immediately."));
    BKNI_Fail();
    return -1;
}


#if !NEXUS_PLATFORM_P_READ_BOX_MODE
#ifndef NEXUS_Platform_P_ReadBoxMode
unsigned NEXUS_Platform_P_ReadBoxMode(void)
{
    unsigned boxMode = 0;
    const char *override;
    FILE *pFile = NULL;
    override = NEXUS_GetEnv("B_REFSW_BOXMODE");
    if (override) {
        boxMode = atoi(override);
    }
    if (!boxMode) {
        pFile = fopen("/proc/device-tree/bolt/rts", "r");
    }
    if (pFile) {
        char buf[64];
        if (fgets(buf, sizeof(buf), pFile)) {
            /* example: 20140402215718_7445_box1, but skip over everything (which may change) and get to _box# */
            char *p = strstr(buf, "_box");
            if (p) {
                boxMode = atoi(&p[4]);
            }
        }
        fclose(pFile);
    }
    return boxMode;
}
#endif
#endif

unsigned NEXUS_Platform_P_ReadBoardId(void)
{
    unsigned id = 0;
    FILE *f = fopen("/proc/device-tree/bolt/board-id", "r");
    if (f) {
        if (fread(&id, sizeof(id), 1, f) != 1) {
            id = 0;
        }
        fclose(f);
    }
    return id;
}

unsigned NEXUS_Platform_P_ReadPMapId(void)
{
    unsigned pMapId = 0;
    const char *override;
    FILE *pFile = NULL;

    override = NEXUS_GetEnv("B_REFSW_PMAP_ID");
    if (override) {
        pMapId = atoi(override);
    }
    if (!pMapId) {
        pFile = fopen("/proc/device-tree/bolt/pmap", "rb");
    }
    if (pFile) {
        uint8_t buf[4];
        if (fread(buf, sizeof(buf), 1, pFile) != 1) {
            pMapId = 0;
        } else {
            pMapId = (unsigned) buf[3];
        }
        fclose(pFile);
    }
    return pMapId;
}
