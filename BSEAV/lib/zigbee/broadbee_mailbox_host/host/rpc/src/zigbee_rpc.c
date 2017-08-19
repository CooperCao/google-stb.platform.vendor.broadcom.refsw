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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
//#include "zigbee_types.h"
#include "bbMailAPI.h"
#include "zigbee_rpc.h"
#include "zigbee_api.h"
#include "zigbee_common.h"
#include "zigbee_rpc_frame.h"
#include "zigbee_socket.h"

/*******************************************************************************
   This function may block

   socket is used for the server socket process to know which client to send to.
*******************************************************************************/
void Zigbee_Rpc_Send(unsigned int *pData, int size_in_words, uint32_t code, void (*sendBuffer)(unsigned int *, int, int), int socket)
{
    int i;
    static unsigned int msg_seq_nbr=0;
    unsigned int buffer[RPC_MAX_FRAME_SIZE_IN_WORDS];
    memset((char *)buffer, 0, sizeof(buffer));

    RPC_FRAME_SET_MSG_ID(&buffer[RPC_FRAME_HEADER_OFFSET], code);
    RPC_FRAME_SET_MSG_SEQ_NUM(&buffer[RPC_FRAME_HEADER_OFFSET], msg_seq_nbr);

    assert(size_in_words+RPC_FRAME_HEADER_SIZE_IN_WORDS <= MAX_MSG_SIZE_IN_WORDS);
    buffer[RPC_FRAME_LENGTH_OFFSET] = size_in_words+RPC_FRAME_HEADER_SIZE_IN_WORDS;

    /* Copy the data */
    for (i=0; i<size_in_words; i++) {
        buffer[i+RPC_FRAME_HEADER_SIZE_IN_WORDS] = pData[i];
    }

    (*sendBuffer)(buffer, size_in_words+RPC_FRAME_HEADER_SIZE_IN_WORDS, socket);
    msg_seq_nbr++;
}

int Zigbee_Rpc_Receive(unsigned int *message_buffer, int *socket, int (*recvBuffer)(unsigned int *, int *, int))
{
    unsigned int socket_data;
    int message_index=0;
    int message_length;
    int remaining_words=0;
    int ret;

    while (1) {
        if ((ret = (*recvBuffer)(&socket_data, socket, 1)) == -1) {
            return (ret);
        }
        if (ret==0) return -1;
        assert(ret==4); /* Assert that we are reading 4 bytes */
        message_buffer[message_index]=socket_data;

        if (message_index == RPC_FRAME_HEADER_OFFSET) {
            message_index++;
        } else if (message_index == RPC_FRAME_LENGTH_OFFSET) {
            message_length = message_buffer[message_index];
            remaining_words = message_length - (RPC_FRAME_LENGTH_OFFSET + 1);
            message_index++;
        }
        else {
            if (remaining_words) {
                remaining_words--;

                if (!remaining_words) {
                    /* process message */
                    #ifdef DEBUG
                    {
                        int i;
                        printf("ZIGBEE_RPC:  total words in this message=%d\n", message_index+1);
                        for (i=0; i<message_index+1; i++) {
                            printf("%d:  0x%08x\n", i, message_buffer[i]);
                        }
                    }
                    #endif
                    return 0;
                } else {
                    message_index++;
                }
            }
            else {
                printf("remaining_words equal to zero\n");
                assert(0);
            }
        }
    }
}

/* eof zigbee_rpc.c */