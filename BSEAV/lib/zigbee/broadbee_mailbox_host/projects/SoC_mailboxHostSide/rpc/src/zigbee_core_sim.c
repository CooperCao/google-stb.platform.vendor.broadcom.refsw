/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "bbMailAPI.h"
#include "zigbee_api.h"
#include "zigbee_common.h"
#include "zigbee_rpc_frame.h"
#include "zigbee_ioctl.h"
#include "zigbee_rpc_server_priv.h"
#include "zigbee_socket.h"

extern socket_cb_t socket_cb[];

#ifdef TEST
unsigned int g_server_tx_buffer[MAX_MSG_SIZE_IN_WORDS];
unsigned int g_server_rx_buffer[MAX_MSG_SIZE_IN_WORDS];
#endif

static void my_RF4CE_ZRC_CheckValidationInd_callback(RF4CE_GDP_CheckValidationIndDescr_t *req, RF4CE_GDP_CheckValidationRespConfParams_t *conf)
{
    printf("ZIGBEE_CORE_SIM:  my_RF4CE_ZRC_CheckValidationInd_callback invoked!  req=%p, status=0x%x\n", req, conf->status);
}

#ifdef TEST
    void ServerLoopbackInd_callback(unsigned int *tx_buffer, unsigned int *rx_buffer, unsigned int num_of_words)
    {
        int i;
        bool test_passed = true;

        printf("ZIGBEE_CORE_SIM:  in ServerLoopbackInd_callback:  tx_buffer=%p, rx_buffer=%p, num_of_words=%d\n", tx_buffer, rx_buffer, num_of_words);
        for (i=0; i<num_of_words; i++) {
            if (tx_buffer[i] != rx_buffer[i]) {
                printf("miscompare at %0d:  tx=0x%08x rx=0x%08x\n", i, tx_buffer[i], rx_buffer[i]);
                test_passed = false;
            }
        }
        printf("test %s\n", test_passed ? "passed" : "failed");
    }
#endif

int zigbee_core_sm(void)
{
    int i;
    int done;
    static int j=0;
    int socket;

    done=0;
    for (i=0; i<MAX_SOCKETS; i++, j++) {
        if (socket_cb[j%MAX_SOCKETS].state) {
            socket = j%MAX_SOCKETS;

            /* something to do? */
            switch (socket_cb[socket].state) {

            case SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_PairInd:
                {
                    RF4CE_PairingReferenceIndParams_t indication;
                    printf("ZIGBEE_CORE_SIM:  in process_RF4CE_ZRC_PairInd\n");
                    indication.pairingRef = 0x34;
                    RF4CE_ZRC_PairInd(&indication, socket);
                    j++;  // pick up where left off next time
                    return 0;
                }
                break;

            case SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_CheckValidationInd:
                {
                    RF4CE_GDP_CheckValidationIndDescr_t indication;
                    printf("ZIGBEE_CORE_SIM:  in process_RF4CE_ZRC_CheckValidationInd\n");
                    indication.params.pairingRef = 0x89;
                    indication.callback = my_RF4CE_ZRC_CheckValidationInd_callback;
                    RF4CE_ZRC_CheckValidationInd(&indication, socket);
                    j++;  // pick up where left off next time
                    return 0;
                }
                break;

            case SOCKET_NEED_TO_SEND_RPC_S2C_RF4CE_ZRC_ControlCommandInd:
                {
                    RF4CE_ZRC_ControlCommandIndParams_t indication;
                    printf("ZIGBEE_CORE_SIM:  in process_RF4CE_ZRC_ControlCommandInd\n");
                    indication.pairingRef = 0x56;
                    indication.payload = 0x89abcdef;
                    RF4CE_ZRC_ControlCommandInd(&indication, socket);
                    j++;  // pick up where left off next time
                    return 0;
                }
                break;

#ifdef TEST
            case SOCKET_NEED_TO_SEND_RPC_S2C_ServerLoopbackInd:
                {
                    int i;
                    memset((char *)g_server_rx_buffer, 0, sizeof(g_server_rx_buffer));
                    memset((char *)g_server_tx_buffer, 0, sizeof(g_server_tx_buffer));
#define NUM_OF_WORDS_TO_TEST 16
                    for (i=0; i<NUM_OF_WORDS_TO_TEST; i++) {
                        g_server_tx_buffer[i] = i;
                    }

                    ServerLoopbackInd(socket, g_server_tx_buffer, g_server_rx_buffer, NUM_OF_WORDS_TO_TEST, ServerLoopbackInd_callback);
                    j++;  // pick up where left off next time
                    return 0;
                }
                break;
#endif

            default:
                break;
            }
        }
    }
}

static void *zigbee_core_simulator(void *pParam)
{
    while (1) {
        zigbee_core_sm();
        sleep(1);
    }
}

void Zigbee_CoreSimulatorInit(void)
{
    int rc;
    pthread_t thread;
    pthread_attr_t threadAttr;
    struct sched_param schedParam;

    /* Create thread for the simulating the zigbee core */
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_FIFO);
    pthread_attr_getschedparam(&threadAttr, &schedParam);
    schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_attr_setschedparam(&threadAttr, &schedParam);
    pthread_attr_setstacksize(&threadAttr, 8*1024);
    rc = pthread_create(&thread,
                        &threadAttr,
                        zigbee_core_simulator,
                        (void *)g_zigbeeFd);
    if ( rc ) {
        printf("ZIGBEE_CORE_SIM:  Unable to create thread");
        while (1);
    }

    pthread_join(thread, NULL);
}