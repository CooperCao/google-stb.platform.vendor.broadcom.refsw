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

/*
 * mq_test.c
 *
 *  Created on: May 13, 2015
 *      Author: gambhire
 */

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

static char testText[] = "Twinkle Twinkle little star";
static char recvText[80];

void mq_test() {

    struct mq_attr attr;
    memset(&attr, 0, sizeof(struct mq_attr));
    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 1024;

    mqd_t mq = mq_open("/mqueue/test", O_RDONLY|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO, &attr);
    if (mq <= 0) {
        perror("mq creation failed");
    }

    pid_t child = fork();
    if (child == 0) {
        // Child is the producer
        mqd_t mqc = mq_open("/mqueue/test", O_WRONLY);
        if (mqc <= 0) {
            perror("could not open message queue in child process\n");
        }

        for (int i=0; i<10; i++) {
            int rc = mq_send(mqc, testText, strlen(testText)+1, 10);
            if (rc <= 0) {
                perror("mq_send failed: ");
            }
        }

        int rc = mq_close(mqc);
        if (rc != 0) {
            perror("mq_close child failed: ");
        }


        exit(0);
    }
    else {
        // Parent is the consumer
        unsigned int prio = 0;

        for (int i=0; i<10; i++) {
            int rc = mq_receive(mq, recvText, 80, &prio);
            if (rc <= 0) {
                perror("mq_recv failed: ");
                return;
            }

            if (strcmp(recvText, testText)) {
                printf("Message queue test failed\n");
                exit(1);
            }
        }

        int status;
        wait(&status);
    }

    int rc = mq_close(mq);
    if (rc != 0) {
        perror("mq_close failed: ");
    }

    rc = mq_unlink("/mqueue/test");
    if (rc != 0) {
        perror("mq_unlink failed: ");
    }

    printf("Message queue test success\n");
}
