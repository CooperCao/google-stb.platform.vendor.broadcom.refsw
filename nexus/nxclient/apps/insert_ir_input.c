/***************************************************************************
 *     (c)2014 Broadcom Corporation
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
 **************************************************************************/
#if NEXUS_INSERT_IR_INPUT
#include "nxclient.h"
#include "nexus_ir_input.h"
#include "nexus_insert_ir_input.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

BDBG_MODULE(insert_ir_input);

static void print_usage(void)
{
    printf(
    "Usage: insert_ir_input OPTIONS\n"
    "  --help or -h       print this info here; this app listens for socket msgs (IR keycodes)\n"
    "                     from client and send to IR device as IR events \n\n"
    "  -index index       where index is the unique index identifier for the IR input device referenced\n"
    "                     during NEXUS_IrOpen call; if not specified, default is NEXUS_ANY_ID which references\n"
    "                     first available IrInput device\n"
    "  -code CODE         send a single test code\n"
    );
}

static NEXUS_Error send_keycode(unsigned index, unsigned keycode)
{
    NEXUS_IrInputEvent event[1];  /* could be array of events since API supports it */

    /* this app here does not pack an array of events to the IR driver; this is just for example purpose
       to show you can use an array */
    BKNI_Memset(event, 0, sizeof(event));
    event[0].code = keycode;

    /* send to Nexus IR driver */
    BDBG_MSG(("Calling NEXUS_IrInput_InsertEvents with index=0x%x, keycode=0x%08lx...\n", index, keycode));
    return NEXUS_IrInput_InsertEvents(index, (const NEXUS_IrInputEvent *)&event[0], 1);
}

int main(int argc, const char **argv)
{
    /* setup socket intf for receiving IR commands */
    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[16];
    unsigned int index=NEXUS_ANY_ID;  /* the index to the IR input instance; default is NEXUS_ANY_ID when not specified on command line */
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned code = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-index") && curarg+1 < argc) {
            index = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-code") && curarg+1 < argc) {
            code = strtoul(argv[++curarg], NULL, 0);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc)
    {
        BDBG_ERR(("NxClient_Join failed - exiting."));
        return -1;
    }

    if (code) {
        send_keycode(index, code);
        goto done;
    }

    /* Create socket */
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        BDBG_ERR(("Could not create socket - exiting."));
        return -1;
    }

    BDBG_MSG(("Socket created"));

    /* Prepare the sockaddr_in structure */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );

    /* Bind */
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        BDBG_ERR(("bind failed - exiting."));
        return -1;
    }

    BDBG_MSG(("bind successful!"));

    BDBG_MSG(("using index 0x%x for IR device reference...\n", index));

    while (1)
    {
        /* Listen for incoming */
        BDBG_MSG(("Waiting for incoming connections..."));
        listen(socket_desc , 5);

        c = sizeof(struct sockaddr_in);

        /* Accept connection from an incoming client */
        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

        if (client_sock < 0)
        {
            BDBG_ERR(("accept failed - exiting."));
            break;
        }

        BDBG_MSG(("Connection accepted"));

        /* Receive a message from client */
        while( (read_size = recv(client_sock , client_message , 16, 0)) > 0 )
        {
            char *p;
            unsigned int keycode;

            client_message[10] = '\0';  /* null out end of string; we expect hex string of 0x00000000 for IR key codes */
            keycode  = strtoul(client_message, &p, 16);

            send_keycode(index, keycode);

            /* Echo back the message to client */
            write(client_sock , client_message , strlen(client_message));
        }

        if(read_size == 0)
        {
            BDBG_MSG(("Client disconnected"));
            fflush(stdout);
        }
        else if(read_size == -1)
        {
            BDBG_ERR(("recv failed"));
        }
    }

    /* Close socket */
    close(socket_desc);

done:
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("insert_ir_input requires building with NEXUS_IR_INPUT_EXTENSION_INC=$NEXUS_TOP/extensions/insert_ir_input/insert_ir_input.inc\n");
    return 0;
}
#endif
