/******************************************************************************
 *    (c)2011-2013 Broadcom Corporation
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
#include "nxclient.h"
#include "getstatus_autogen.c"

unsigned num_status_functions = sizeof(status_functions) / sizeof(status_func_entry);

void dump_all_handles(void) {
    unsigned i;
    NEXUS_Error rc;

    for (i=0; i < num_status_functions; i++) {
        rc = (*status_functions[i].func)(status_functions[i].module_name);
        if (rc) {
            printf("Error %d calling '%s'\n", rc, status_functions[i].function_name);
        }
    }
}

static void print_usage(void)
{
    unsigned i;
    printf("Usage:\n");
    printf("Enter either a function name or the associated index to print the getstatus for all handles of that type.\n");
    printf("Enter 'all' to getstatus for all available handles.\n");
    printf("Functions:\n");
    for (i=0; i < num_status_functions; i++) {
        printf("  %d: %s\n", i+1, status_functions[i].function_name);
    }
}

int main(int argc, char **argv) {
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    bool interactive = false;
    unsigned i;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) { printf("Failed to join\n"); return -1; }

    if (argc > 1) {
        unsigned j;
        char *s;
        for (i=1; i < (unsigned)argc; i++) {
            s = argv[i];
            if (!strcmp(s, "-h") || !strcmp(s, "--help")) {
                print_usage();
                return 0;
            }
            else if (!strcmp(s, "-i")) {
                interactive = true;
            } 
            else if (!strcmp(s, "all")) {
                dump_all_handles();
            } 
            else {
                for (j=0; j < num_status_functions; j++) {
                    if (!strcmp(s,status_functions[j].function_name) || !strcmp(s,status_functions[j].module_name)) {
                        rc = (*status_functions[j].func)(status_functions[j].module_name);
                        if (rc) {
                            printf("Error %d calling '%s'\n", rc, status_functions[j].function_name);
                        }
                        break;
                    }
                }
                if (j == num_status_functions) {
                    unsigned index = atoi(s);
                    if (index) {
                        index--;
                        rc = (*status_functions[index].func)(status_functions[index].module_name);
                        if (rc) {
                            printf("Error %d calling '%s'\n", rc, status_functions[index].function_name);
                        }
                    }
                }
            }
        }
    } else {
        interactive = true;
    }

    while (interactive) {
        char buffer[512];
        char *buf;
        unsigned i;
        printf("Status function or index ('?' for help): "); fflush(stdout);
        fgets(buffer, 512, stdin);
        buffer[strlen(buffer)-1] = 0; /* chomp \n */
        buf = strtok(buffer, ";");
        if (!buf) continue;
        do {
            int index = -1;
            i=atoi(buf);
            if (!strcmp(buf, "q") || !strcmp(buf, "quit"))
                goto done;
            if (!strcmp(buf, "h") || !strcmp(buf, "help") || !strcmp(buf, "?")) {
                print_usage();
            } else if (!strcmp(buf, "all") || !strcmp(buf, "a")) {
                dump_all_handles();
            } else if (i > 0 && i <= num_status_functions) {
                index = i-1;
            } else {
                for (i=0; i < num_status_functions; i++) {
                    if (!strcmp(buf,status_functions[i].function_name) || !strcmp(buf,status_functions[i].module_name))
                        index = i;
                }
            }
            if (index >= 0) {
                rc = (*status_functions[index].func)(status_functions[index].module_name);
                if (rc) {
                    printf("Error %d calling '%s'\n", rc, status_functions[index].function_name);
                }
            }
            buf = strtok(NULL, ";");
        } while(buf);
    }
done:
    NxClient_Uninit();

    return 0;
}
