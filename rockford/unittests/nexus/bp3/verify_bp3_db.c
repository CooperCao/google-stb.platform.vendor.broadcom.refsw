/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_platform.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct {
    const char* fname;
    NEXUS_LicensedFeature support[NEXUS_LicensedFeature_eMax];
} testcases[] =
{
    { "BP3_ALL_328D4E10B809FABA_00000000.bin",    {1, 1, 1, 1}},
    { "BP3_DVHDR_328D4E10B809FABA_00000000.bin",  {1, 1, 0, 0}},
    { "BP3_NONE_328D4E10B809FABA_00000000.bin",   {1, 0, 0, 0}},
    { "BP3_TCHHDR_328D4E10B809FABA_00000000.bin", {1, 0, 1, 0}},
    { "BP3_TCHITM_328D4E10B809FABA_00000000.bin", {1, 0, 0, 1}}
};

#define NUM_TESTCASES (sizeof(testcases) / sizeof(*testcases))
#define BIN_TARGET "./bp3.bin"

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    bool single = false, supported;
    unsigned i, t = 0;
    char cmd[256], cmd_rm[256];
    sprintf(cmd_rm, "rm %s", BIN_TARGET);

    /* if there is an arg, only run that one test case. otherwise, run all test cases */
    if (argc > 1) {
        t = atoi(argv[1]);
        single = true;
        if (t >= NUM_TESTCASES) {
            fprintf(stderr, "Testcase %u does not exist\n", t);
            return 1;
        }
    }

    for (; t<NUM_TESTCASES; t++) {
        sprintf(cmd, "ln -s %s %s", testcases[t].fname, BIN_TARGET);
        if (system(cmd)!=0) {
            fprintf(stderr, "Error creating symlink %s -> %s\n", BIN_TARGET, testcases[t].fname);
            return 1;
        }
        fprintf(stderr, "Test %s\n", testcases[t].fname);

        NEXUS_Platform_GetDefaultSettings(&platformSettings);
        platformSettings.openFrontend = false;
        NEXUS_Platform_Init(&platformSettings);

        for (i=0; i<NEXUS_LicensedFeature_eMax; i++) {
            NEXUS_Platform_IsLicensedFeatureSupported(i, &supported);
            if (testcases[t].support[i] != supported) {
                fprintf(stderr, "Testcase %u, feature %u mismatch: expected %u, actual %u. Asserting..", t, i, testcases[t].support[i], supported);
                system(cmd_rm);
                BDBG_ASSERT(0);
            }
        }

        NEXUS_Platform_Uninit();
        system(cmd_rm);
        if (single) { break; }
    }

    fprintf(stderr, "Test completed\n");

    return 0;
}
