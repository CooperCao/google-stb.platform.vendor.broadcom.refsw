/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "ata_helper.h"
char *testMsg[] = {"blocking pipe", "non blocking pipe", "child", "parent"};

void child_thread(int *blockingPipeFd, int *nonBlockingPipeFd) {
    int len = 0;
    char recvMsgChild[40];
    close(nonBlockingPipeFd[1]); //close write end of non blocking pipe
    close(blockingPipeFd[0]); //close read end of blocking pipe

/* read entire buffer at once */
    do {
        if (read(nonBlockingPipeFd[0], &recvMsgChild, sizeof(recvMsgChild)) < 0) {
            ATA_LogErr("reading nonBlockingPipeFd: %s ", strerror(errno));
        }
    } while(strcmp(testMsg[3], recvMsgChild));

/* write to blocking pipe */
    if (write(blockingPipeFd[1], testMsg[2], strlen(testMsg[2])+1) < 0) {
        ATA_LogErr(" writing blockingPipeFd: %s ", strerror(errno));
    }

/* try again and again untill read EOF or some data*/
    while ((len = read(nonBlockingPipeFd[0], &recvMsgChild, sizeof(recvMsgChild))) < 0);
    if (len == 0)
        printf("[MSG] end of file\n");
    else
        printf("[MSG] child received msg: %s\n", recvMsgChild);

    /* write to blocking pipe */
    if (write(blockingPipeFd[1], testMsg[0], strlen(testMsg[0])+1) < 0) {
        ATA_LogErr(" writing blockingPipeFd: %s ", strerror(errno));
    }
    sleep(5); // sleep before you exit

    _exit(EXIT_SUCCESS);
}

void parent_thread(int *blockingPipeFd, int *nonBlockingPipeFd) {
    int len = 0;
    char recvMsgParent[40];
    close(nonBlockingPipeFd[0]); // close read end of non blocking pipe
    close(blockingPipeFd[1]); // close write end of blocking pipe

    if (write(nonBlockingPipeFd[1], testMsg[3], strlen(testMsg[3])+1) < 0) {
        ATA_LogErr(" writing nonBlockingPipeFd: %s ", strerror(errno));
    }

    if (read(blockingPipeFd[0], &recvMsgParent, sizeof(recvMsgParent)) < 0) {
        ATA_LogErr(" reading blockingPipeFd: %s ", strerror(errno));
    }

    if (strcmp(testMsg[2], recvMsgParent)) printf("[ERROR] Received wrong msg\n");

    if (write(nonBlockingPipeFd[1], testMsg[1], strlen(testMsg[1])+1) < 0) {
        ATA_LogErr(" writing nonBlockingPipeFd: %s ", strerror(errno));
    }

    if (read(blockingPipeFd[0], &recvMsgParent, sizeof(recvMsgParent)) < 0) {
        ATA_LogErr(" reading blockingPipeFd : %s ", strerror(errno));
    }
    printf("[MSG] parent received msg: %s\n", recvMsgParent);

    wait(NULL);                /* Wait for child */
    /* try to read pipe with no writers */
    len = read(blockingPipeFd[0], &recvMsgParent, sizeof(recvMsgParent));
    if (0 == len)
        printf("[MSG] blocking_pipefd with no writers : end of file\n");
    else if (len < 0)
        printf("[MSG] blocking_pipefd with no writers: %s ", strerror(errno));
    else
        printf("[MSG] parent received msg: %s\n", recvMsgParent);

    /* try to write to a pipe with no readers */
    len = write(nonBlockingPipeFd[1], testMsg[1], strlen(testMsg[1])+1);

    if (len < 0)
        printf("[MSG] nonblocking_pipefd with no readers: %s ", strerror(errno));
    else
        ATA_LogErr(" nonblocking_pipefd with no readers : unexpected return");

    /* close all open ends and delete pipe*/
    close(nonBlockingPipeFd[1]);
    close(blockingPipeFd[0]);

    /* try to read and write with pipe which doesn't exist */
    if(write(nonBlockingPipeFd[1], testMsg[1], strlen(testMsg[1])+1) >= 0)
        ATA_LogErr(" nonblocking_pipefd which dosn't exist : unexpected return");

    if(read(blockingPipeFd[0], &recvMsgParent, sizeof(recvMsgParent)) >= 0)
        ATA_LogErr(" blocking_pipefd which dosn't exist : unexpected return");
}

void pipe_test() {
    int blockingPipeFd[2];
    int nonBlockingPipeFd[2];

/* create blocking pipe with close on exec */
    if (pipe2(blockingPipeFd, O_CLOEXEC) == -1) {
        ATA_LogErr(" pipe_test : blockingPipeFd: %s ", strerror(errno));
        goto FAIL;
    }

/* create non blocking pipe with close on exec */
    if (pipe2(nonBlockingPipeFd, O_CLOEXEC | O_NONBLOCK) == -1) {
        ATA_LogErr(" pipe_test : nonblocking_pipefd: %s ", strerror(errno));
        goto FAIL;
    }

/* create child task */
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        ATA_LogErr(" pipe_test : fork: %s ", strerror(errno));
        goto FAIL;
    }

    if (pid == 0) {    /* Child task */
        child_thread(blockingPipeFd, nonBlockingPipeFd);
    } else {            /* Parent task */
        parent_thread(blockingPipeFd, nonBlockingPipeFd);
    }
    printf("Pipe test success\n");
    return;

FAIL:
    return;
}
