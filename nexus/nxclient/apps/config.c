/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#if NEXUS_HAS_SIMPLE_DECODER
#include "nexus_platform_client.h"
#include "nexus_platform_server.h"
#include "nxclient.h"
#include "nxclient_config.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_base_mmap.h"
#include "nxapps_cmdline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(nxconfig);

#define MAX_OBJECTS 128

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
    "nxclient - configure clients\n"
    "\n"
    "If no params are specified, it will print all clients.\n"
    "\n"
    "  -c <CLIENT NAME or ID>          if client not specified, operates on top client. \n"
    "                                if multiple clients with same name, operates on first.\n"
    "  -pid <CLIENT PID>               select client by pid\n"
    " \n"
    );
    printf(
    "# individual commands\n"
    "  -full               position client to full screen\n"
    "  -focus              receive user input and unfocus all other clients\n"
    "  -refresh            refresh all connect requests (for example, grab decoders back)\n"
    "  -list {full|pid}    list format\n"
    );
    printf(
    "  -mem                print estimate of device memory used by this client\n"
    );
    nxapps_cmdline_print_usage(cmdline);
}

static void print_mem(NEXUS_ClientHandle client)
{
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
    size_t num;
    unsigned total = 0;
    NEXUS_MemoryBlockHandle blocks[MAX_OBJECTS];
    unsigned total_blocks = 0;
    int rc;
    unsigned i;
    NEXUS_InterfaceName interfaceName;

    printf("device memory:\n");
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_Surface");
    rc = NEXUS_Platform_GetClientObjects(client, &interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    for (i=0;i<num;i++) {
        NEXUS_SurfaceMemoryProperties prop;
        NEXUS_SurfaceStatus status;
        unsigned sz;
        NEXUS_Surface_GetMemoryProperties(objects[i].object, &prop);
        NEXUS_Surface_GetStatus(objects[i].object, &status);
        sz = status.height * status.pitch + status.numPaletteEntries * sizeof(uint32_t);
        total += sz;
        /* must cache memory blocks to avoid double counting */
        if (prop.pixelMemory && total_blocks < MAX_OBJECTS) {
            blocks[total_blocks++] = prop.pixelMemory;
        }
        if (prop.paletteMemory && total_blocks < MAX_OBJECTS) {
            blocks[total_blocks++] = prop.paletteMemory;
        }
        printf("  surface %p: %d bytes\n", objects[i].object, sz);
    }
    strcpy(interfaceName.name, "NEXUS_MemoryBlock");
    rc = NEXUS_Platform_GetClientObjects(client, &interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    for (i=0;i<num;i++) {
        NEXUS_MemoryBlockProperties prop;
        unsigned j;

        /* don't double count memory blocks */
        for (j=0;j<total_blocks;j++) {
            if (blocks[j] == objects[i].object) break;
        }
        if (j<total_blocks) continue;

        NEXUS_MemoryBlock_GetProperties(objects[i].object, &prop);
        total += prop.size;
        printf(" block %p: %ld bytes\n", objects[i].object, (unsigned long)prop.size);
    }
    printf("  total: %d bytes\n", total);
}

int main(int argc, const char **argv)  {
    NxClient_JoinSettings joinSettings;
    int curarg = 1;
    int rc;
    NEXUS_ClientHandle client;
    int clientIndex = -1;
    unsigned i;
    bool focus = false;
    bool refresh = false;
    NEXUS_InterfaceName interfaceName;
    struct {
        NEXUS_ClientHandle handle;
        unsigned pid;
        NxClient_JoinSettings joinSettings;
    } clients[MAX_OBJECTS];
    enum {
        list_full,
        list_pid
    } list_format = list_full;
    bool mem = false;
    struct nxapps_cmdline cmdline;
    int n;

    memset(clients, 0, sizeof(clients));
    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        curarg++;
    }
    curarg = 1;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    joinSettings.mode = NEXUS_ClientMode_eVerified;
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    {
        NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
        size_t num;
        unsigned total = 0;

        NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
        strcpy(interfaceName.name, "NEXUS_Client");
        rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
        BDBG_ASSERT(!rc);

        for (i=0;i<num;i++) {
            NEXUS_ClientStatus status;
            NxClient_JoinSettings joinSettings;
            unsigned j;

            rc = NEXUS_Platform_GetClientStatus(objects[i].object, &status);
            if (rc) continue;

            NxClient_Config_GetJoinSettings(objects[i].object, &joinSettings);
            if (status.pid == (unsigned)getpid()) continue; /* skip thyself */

            /* insert into clients[], sorted by pid */
            for (j=0;j<total;j++) {
                if (status.pid < clients[j].pid) {
                    /* bubble */
                    unsigned k;
                    for (k=total;k>j;k--) clients[k] = clients[k-1];
                    break;
                }
            }
            clients[j].handle = objects[i].object;
            clients[j].pid = status.pid;
            clients[j].joinSettings = joinSettings;
            total++;
        }
    }

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "-pid") && argc>curarg+1) {
            unsigned pid = atoi(argv[++curarg]);
            for (i=0;clients[i].handle;i++) {
                if ((unsigned)clients[i].pid == pid) {
                    clientIndex = i;
                    /* TEST */
                    BDBG_ASSERT(NxClient_Config_LookupClient(pid) == clients[i].handle);
                    break;
                }
            }
        }
        else if (!strcmp(argv[curarg], "-c") && argc>curarg+1) {
            unsigned long id = atol(argv[++curarg]);
            if (id) {
                id--;
                if (id < MAX_OBJECTS && clients[id].handle) {
                    /* by index */
                    clientIndex = id;
                }
                else {
                    /* by handle */
                    for (i=0;clients[i].handle;i++) {
                        if ((unsigned long)clients[i].handle == id) {
                            clientIndex = i;
                            break;
                        }
                    }
                }
            }
            else {
                /* by name */
                for (i=0;clients[i].handle;i++) {
                    if (strstr(clients[i].joinSettings.name, argv[curarg])) {
                        clientIndex = i;
                        break;
                    }
                }
            }
            if (clientIndex == -1) {
                BDBG_WRN(("client not found"));
            }
        }
        else if (!strcmp(argv[curarg], "-full")) {
            const char *argv[] = {"-rect","0,0,1920,1080"};
            nxapps_cmdline_parse(0, 2, argv, &cmdline);
        }
        else if (!strcmp(argv[curarg], "-focus")) {
            focus = true;
        }
        else if (!strcmp(argv[curarg], "-refresh")) {
            refresh = true;
        }
        else if (!strcmp(argv[curarg], "-list") && argc>curarg+1) {
            curarg++;
            if (!strcmp(argv[curarg], "full")) {
                list_format = list_full;
            }
            else if (!strcmp(argv[curarg], "pid")) {
                list_format = list_pid;
            }
        }
        else if (!strcmp(argv[curarg], "-mem")) {
            mem = true;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else {
            print_usage(&cmdline);
            return -1;
        }
        curarg++;
    }

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition) ||
        nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings) ||
        focus ||
        refresh)
    {
        if (clientIndex == -1) {
            printf("unknown client\n");
            goto done;
        }
        client = clients[clientIndex].handle;

        if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
            NEXUS_PlatformObjectInstance surfaceClients[MAX_OBJECTS];
            size_t num;

            strcpy(interfaceName.name, "NEXUS_SurfaceClient");
            rc = NEXUS_Platform_GetClientObjects(client, &interfaceName, surfaceClients, MAX_OBJECTS, &num);
            BDBG_ASSERT(!rc);
            if (num) {
                for (i=0;i<num;i++) {
                    NEXUS_SurfaceComposition comp;
                    NxClient_Config_GetSurfaceClientComposition(client, surfaceClients[i].object, &comp);
                    if (!comp.position.width && !comp.position.height) continue; /* skip child video windows */
                    nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
                    NxClient_Config_SetSurfaceClientComposition(client, surfaceClients[i].object, &comp);
                    break;
                }
            }
            else {
                BDBG_WRN(("no surface clients"));
            }
        }
        if (refresh) {
            NxClient_ConnectList list;
            rc = NxClient_Config_GetConnectList(client, &list);
            BDBG_ASSERT(!rc);
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                if (!list.connectId[i]) break;
                rc = NxClient_Config_RefreshConnect(client, list.connectId[i]);
                if (rc) BERR_TRACE(rc);
            }
        }
        if (focus) {
            NEXUS_PlatformObjectInstance inputClients[MAX_OBJECTS];
            size_t num;

            strcpy(interfaceName.name, "NEXUS_InputClient");
            for (i=0;i<MAX_OBJECTS;i++) {
                if (!clients[i].handle) break;
                rc = NEXUS_Platform_GetClientObjects(clients[i].handle, &interfaceName, inputClients, MAX_OBJECTS, &num);
                BDBG_ASSERT(!rc);
                if (num) {
                    unsigned j;
                    /* if client acquires more than one, the best we can do is focus/unfocus all */
                    for (j=0;j<num;j++) {
                        NxClient_Config_SetInputClientServerFilter(clients[i].handle, inputClients[j].object, (i==(unsigned)clientIndex)?0xFFFFFFFF/*all*/: 0x0/*none*/);
                    }
                }
                else {
                    BDBG_WRN(("no input clients for client %d", i+1));
                }
            }
        }

        if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings)) {
            NEXUS_PlatformObjectInstance decoders[MAX_OBJECTS];
            size_t num;
            strcpy(interfaceName.name, "NEXUS_SimpleVideoDecoder");
            rc = NEXUS_Platform_GetClientObjects(client, &interfaceName, decoders, MAX_OBJECTS, &num);
            BDBG_ASSERT(!rc);
            if (num) {
                unsigned j;
                for (j=0;j<num;j++) {
                    NEXUS_SimpleVideoDecoderPictureQualitySettings settings;
                    NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(decoders[j].object, &settings);
                    nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(&cmdline, &settings);
                    NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(decoders[j].object, &settings);
                }
            }
            else {
                BDBG_WRN(("no SimpleVideoDecoders for client %d", clientIndex+1));
            }
        }
    }
    else {
        if (clientIndex != -1) {
            NEXUS_PlatformObjectInstance surfaceClients[MAX_OBJECTS];
            size_t num;

            NxClient_ConnectList list;

            client = clients[clientIndex].handle;

            strcpy(interfaceName.name, "NEXUS_SurfaceClient");
            rc = NEXUS_Platform_GetClientObjects(client, &interfaceName, surfaceClients, MAX_OBJECTS, &num);
            BDBG_ASSERT(!rc);

            printf("client %d: %s\n", clientIndex+1, clients[clientIndex].joinSettings.name);
            printf("pid=%d session=%d\n", clients[clientIndex].pid, clients[clientIndex].joinSettings.session);
            for (i=0;i<num;i++) {
                NEXUS_SurfaceComposition comp;
                NxClient_Config_GetSurfaceClientComposition(client, surfaceClients[i].object, &comp);
                printf("SurfaceClient %p: rect=%d,%d,%d,%d zorder=%d\n", surfaceClients[i].object,
                    comp.position.x, comp.position.y, comp.position.width, comp.position.height,
                    comp.zorder);
            }

            rc = NxClient_Config_GetConnectList(client, &list);
            BDBG_ASSERT(!rc);
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                NxClient_ConnectSettings settings;
                unsigned j;
                if (!list.connectId[i]) break;
                NxClient_Config_GetConnectSettings(client, list.connectId[i], &settings);
                printf("Connect %#x:\n", list.connectId[i]);
                for (j=0;j<NXCLIENT_MAX_IDS;j++) {
                    if (!settings.simpleVideoDecoder[j].id) break;
                    printf("  SimpleVideoDecoder[%d]: %d, surfaceClientId %d, windowId %d, %dx%d\n",
                        j,
                        settings.simpleVideoDecoder[j].id, settings.simpleVideoDecoder[j].surfaceClientId,
                        settings.simpleVideoDecoder[j].windowId,
                        settings.simpleVideoDecoder[j].decoderCapabilities.maxWidth,
                        settings.simpleVideoDecoder[j].decoderCapabilities.maxHeight);
                }
                if (settings.simpleAudioDecoder.id) {
                    printf("  SimpleAudioDecoder: %d\n",
                        settings.simpleAudioDecoder.id);
                }
                for (j=0;j<NXCLIENT_MAX_IDS;j++) {
                    if (!settings.simpleAudioPlayback[j].id) break;
                    printf("  SimpleAudioPlayback[%d]: %d\n",
                        j,
                        settings.simpleAudioPlayback[j].id);
                }
                for (j=0;j<NXCLIENT_MAX_IDS;j++) {
                    if (!settings.simpleEncoder[j].id) break;
                    printf("  simpleEncoder[%d]: %d\n",
                        j,
                        settings.simpleEncoder[j].id);
                }
            }

            if (mem) {
                print_mem(client);
            }
        }
        else {
            if (list_format == list_pid) {
                for (i=0;clients[i].handle;i++) {
                    printf("%d\n", clients[i].pid);
                }
            }
            else {
                printf("#: pid   ses name\n");
                printf("---------------------\n");
                for (i=0;clients[i].handle;i++) {
                    printf("%d: %5d %3d %s\n", i+1, clients[i].pid, clients[i].joinSettings.session, clients[i].joinSettings.name);
                    if (mem) {
                        print_mem(clients[i].handle);
                        printf("\n");
                    }
                }
            }
        }
    }

done:
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs simple_decoder)!\n");
    return 0;
}
#endif
