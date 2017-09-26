/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

/* Simple RF4CE app */
#include <getopt.h>
#include "rf4ce_common.h"

void BroadBee_Mail_UartRecvInd(Mail_UartRecvIndDescr_t *indication)
{
    uint8_t input[256] = {0};
    Mail_UartRecvRespParams_t resp;
    Mail_UartRecvIndParams_t *param = &indication->params;
    SYS_CopyFromPayload(input, &param->payload, 0, param->payloadSize);
    printf(input);
    fflush(stdout);
    resp.status = 0;
    indication->callback(indication, &resp);
}

void BroadBee_Mail_UartSendReq(uint8_t *dataSending, int length)
{
    uint8_t statusMailUartSent = 0;

    void BroadBee_Mail_UartSendReq_Callback(Mail_UartSendReqDescr_t *request, Mail_UartSendConfParams_t *conf)
    {
        // free payload
        SYS_FreePayload(&request->params.payload);
        statusMailUartSent = 1;
    }

    Mail_UartSendReqDescr_t req = {0};
    req.params.payloadSize = length;
    SYS_MemAlloc(&req.params.payload, length);
    SYS_CopyToPayload(&req.params.payload, 0, dataSending, length);
    req.callback = BroadBee_Mail_UartSendReq_Callback;

    Mail_UartSendReq(&req);
    while(!statusMailUartSent);
}

#define EXIT_STRING "exit"
int main(int argc, char *argv[])
{
    struct zigbeeCallback zcb;
    uint8_t input[256];
    uint16_t length;
    RF4CE_StartReqDescr_t request;
    static struct termios oldt, newt;
    int c, option_index = 0, running_dut = 1;
#ifdef BYPASS_RPC
    extern int zigbee_init(int argc, char *argv[]);
    zigbee_init(argc, argv);
#endif
    struct option long_options[] =
    {
        {"cmd",     no_argument,       0, 'm'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv,"m:c:",
                   long_options, &option_index )) != -1) {
        switch (c) {
             case 'm' :
             case 'c' : running_dut = 0;
                 break;
             default:
                 break;
        }
    }

    /* Register the callback functions you are interested in.  Ones that are not filled out, won't be called back. */
    /* Calling Zigbee_GetDefaultSettings will initialize the callback structure */
    Zigbee_GetDefaultSettings(&zcb);
    zcb.RF4CE_PairInd = NULL;
    zcb.RF4CE_ZRC2_CheckValidationInd = NULL;
    zcb.RF4CE_ZRC2_ControlCommandInd = NULL;
    zcb.RF4CE_ZRC1_ControlCommandInd = NULL;
    zcb.RF4CE_ZRC1_VendorSpecificInd = NULL;
    zcb.Mail_UartRecvInd = BroadBee_Mail_UartRecvInd;

    Zigbee_Open(&zcb, argv[1]);

    if(running_dut)
        printf("running as dut\n");
    else
        printf("running as cmd\n");
    while(1)
    {
        if(running_dut){
            sleep(0);
            continue;
        }
        else{
            char *line = NULL;
            size_t size;
            if (getline(&line, &size, stdin) == -1) {
                continue;
            } else {
                length = strlen(line);
                if(length > sizeof(input) - 2)
                    continue;
                if(!strncmp(line, EXIT_STRING, strlen(EXIT_STRING)))
                    break;
                length -= 1;
                memset(input, 0, sizeof(input));
                memcpy(input, line, length);
                input[length++] = '\r';
                BroadBee_Mail_UartSendReq(input, length);
            }
        }
    }
#ifdef BYPASS_RPC
    extern void zigbee_deinit(void);
    zigbee_deinit();
#endif
    printf("\nCompleted 'phy_test' application successfully.\n");
    Zigbee_Close();

    return 0;
}

/* eof rf4ce_phy_test.c */
