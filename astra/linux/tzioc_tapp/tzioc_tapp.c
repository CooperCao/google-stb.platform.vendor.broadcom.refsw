/***************************************************************************
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
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libtzioc_api.h"
#include "uappd_msg.h"
#include "tzioc_test_msg.h"
#include "tzioc_tapp.h"

/*
 * Compiler Switches
 */

/*
 * Constant Definitions
 */

#define CLIENT_TEST_REP         16
#define PEER_HELLO_TEST_REP     2
#define PEER_TAPP_TEST_REP      1
#define MSG_ECHO_TEST_REP       16
#define MSG_HELLO_TEST_REP      3
#define MEM_ALLOC_TEST_REP      4

#define MEM_ALLOC_BUFF_SIZE     (1024)

/*
 * Function Declarations
 */

static int tzioc_tapp_init(void);
static void tzioc_tapp_exit(void);

static int tzioc_tapp_client(void);
static int tzioc_tapp_peer_hello(void);
static int tzioc_tapp_peer_tapp(void);
static int tzioc_tapp_msg_echo(void);
static int tzioc_tapp_msg_hello(void);
static int tzioc_tapp_mem_alloc(void);

static void *tzioc_tapp_msg_thread(void *arg);
static int tzioc_tapp_msg_proc(tzioc_msg_hdr *pHdr);

/*
 * Variable Declarations
 */

static char tzioc_tapp_name[16]="tzioc_tapp";

static struct tzioc_tapp tzioc_tapp;
static struct tzioc_tapp *ptapp = &tzioc_tapp;

/*
 * TZIOC Test App Functions
 */

int main(void)
{
    int err = 0;

    err = tzioc_tapp_init();
    if (err) goto ERR_EXIT;

    LOGI("TZIOC testing starts...\n");

    /* client test */
    err = tzioc_tapp_client();
    if (err) goto ERR_EXIT;

    /* create msg thread */
    err = pthread_create(
        &ptapp->msgThread,
        NULL,
        &tzioc_tapp_msg_thread,
        NULL);

    if (err) {
        LOGE("failed to create msg thread");
        goto ERR_EXIT;
    }

    /* msg test - echo */
    err = tzioc_tapp_msg_echo();
    if (err) goto ERR_EXIT;

    /* peer test - hello */
    err = tzioc_tapp_peer_hello();
    if (err) goto ERR_EXIT;

    /* peer test - test app */
    err = tzioc_tapp_peer_tapp();
    if (err) goto ERR_EXIT;

    /* msg test - hello */
    err = tzioc_tapp_msg_hello();
    if (err) goto ERR_EXIT;

    /* mem test - alloc*/
    err = tzioc_tapp_mem_alloc();
    if (err) goto ERR_EXIT;

    LOGI("TZIOC testing done!\n");

 ERR_EXIT:
    tzioc_tapp_exit();
    return err;
}

static int tzioc_tapp_init(void)
{
    /* init test app control block */
    memset(&tzioc_tapp, 0, sizeof(tzioc_tapp));

    /* init msg lock */
    pthread_mutex_init(&ptapp->msgLock, NULL);

    LOGI("TZIOC test app initialized\n");
    return 0;
}

static void tzioc_tapp_exit(void)
{
    if (ptapp->peerId) {
        /* stop peer tzioc_tapp */
        tzioc_peer_stop(
            ptapp->hClient,
            "tzioc_tapp");

        /* clear id for peer tzioc_tapp */
        ptapp->peerId = 0;
    }

    if (ptapp->hClient) {
        /* close test client */
        tzioc_client_close(ptapp->hClient);

        /* clear test client handle */
        ptapp->hClient = 0;
    }

    LOGI("TZIOC test app uninitialized\n");
}

/*
 * TZIOC Test Functions
 */

static int tzioc_tapp_client(void)
{
    int rep;

    /* open/close test client repeatedly */
    for (rep = 0; rep < CLIENT_TEST_REP; rep++) {

        LOGI("client testing %d...", rep);

        /* open test client */
        ptapp->hClient = tzioc_client_open(
            tzioc_tapp_name,
            &ptapp->msgQ,
            &ptapp->clientId);

        if (!ptapp->hClient) {
            LOGE("failed to open client");
            return -EFAULT;
        }

        LOGI("client handle 0x%x, client id %d",
             ptapp->hClient,
             ptapp->clientId);

        /* leave last test client */
        if (rep == CLIENT_TEST_REP - 1) break;

        /* close test client */
        tzioc_client_close(ptapp->hClient);

        /* clear test client handle */
        ptapp->hClient = 0;

        LOGI("client testing %d done\n", rep);
    }

    LOGI("client testing done!\n");
    return 0;
}

static int tzioc_tapp_peer_hello(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    for (rep = 0; rep < PEER_HELLO_TEST_REP; rep++) {

        LOGI("peer hello app testing %d...", rep);

        /* start peer hello */
        err = tzioc_peer_start(
            ptapp->hClient,
            "hello",
            "hello.elf",
            false);

        if (err) {
            LOGE("failed to start peer");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* wait for rpy */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        /* stop peer hello */
        err = tzioc_peer_stop(
            ptapp->hClient,
            "hello");

        if (err) {
            LOGE("failed to stop peer");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* wait for rpy */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* wait for exit nfy for peer hello */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        LOGI("peer hello app testing %d done\n", rep);
    }

    LOGI("peer hello app testing done!\n");
    return 0;
}

static int tzioc_tapp_peer_tapp(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    for (rep = 0; rep < PEER_TAPP_TEST_REP; rep++) {

        LOGI("peer test app testing %d...", rep);

        /* start peer tzioc_tapp */
        err = tzioc_peer_start(
            ptapp->hClient,
            "tzioc_tapp",
            "tzioc_tapp.elf",
            false);

        if (err) {
            LOGE("failed to start peer");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* wait for rpy */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        /* get id for peer tzioc_tapp */
        err = tzioc_peer_getid(
            ptapp->hClient,
            "tzioc_tapp");

        if (err) {
            LOGE("failed to get peer id");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* wait for rpy */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        /* leave last peer active */
        if (rep == PEER_TAPP_TEST_REP - 1) break;

        /* stop peer tzioc_tapp */
        err = tzioc_peer_stop(
            ptapp->hClient,
            "tzioc_tapp");

        if (err) {
            LOGE("failed to stop peer");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        /* clear id for peer tzioc_tapp */
        ptapp->peerId = 0;

        /* wait for rpy */
        timeout = 10;
        while (ptapp->msgCnt > 0 && timeout--)
            nanosleep(&waittime, NULL);

        if (timeout == -1) {
            LOGE("timedout waiting for rpy");
            return -ETIMEDOUT;
        }

        LOGI("peer test app testing %d done\n", rep);
    }

    LOGI("peer test app testing done!\n");
    return 0;
}

static int tzioc_tapp_msg_echo(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    /* send echo msg repeatedly */
    for (rep = 0; rep < MSG_ECHO_TEST_REP; rep++) {
        static uint8_t msg[sizeof(struct tzioc_msg_hdr) +
                           sizeof(struct tzioc_test_msg_echo)];
        struct tzioc_msg_hdr *pHdr =
            (struct tzioc_msg_hdr *)msg;
        struct tzioc_test_msg_echo *pEcho =
            (struct tzioc_test_msg_echo *)TZIOC_MSG_PAYLOAD(pHdr);

        LOGI("msg echo testing %d...", rep);

        /* send echo msg to echo client */
        pHdr->ucType = TZIOC_TEST_MSG_ECHO;
        pHdr->ucOrig = ptapp->clientId;
        pHdr->ucDest = TZIOC_CLIENT_ID_ECHO;
        pHdr->ucSeq  = 0;
        pHdr->ulLen  = sizeof(*pEcho);

        pEcho->value = rep;

        err = tzioc_msg_send(
            ptapp->hClient,
            pHdr);

        if (err) {
            LOGE("failed to send echo msg");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        LOGI("msg echo testing %d done\n", rep);
    }

    /* switch to TZOS */
    tzioc_call_smc(ptapp->hClient, 0x83000007);

    /* wait for echo msgs or timeout */
    timeout = 10;
    while (ptapp->msgCnt > 0 && timeout--)
        nanosleep(&waittime, NULL);

    if (timeout == -1) {
        LOGE("timedout waiting for rpy");
        return -ETIMEDOUT;
    }

    LOGI("msg echo testing done!\n");
    return 0;
}

static int tzioc_tapp_msg_hello(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    /* send hello cmd to peer */
    for (rep = 0; rep < MSG_HELLO_TEST_REP; rep++) {
        static uint8_t msg[sizeof(struct tzioc_msg_hdr) +
                           sizeof(struct tzioc_test_msg_hello_cmd)];
        struct tzioc_msg_hdr *pHdr =
            (struct tzioc_msg_hdr *)msg;
        struct tzioc_test_msg_hello_cmd *pCmd =
            (struct tzioc_test_msg_hello_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

        LOGI("msg hello testing %d...", rep);

        pHdr->ucType = TZIOC_TEST_MSG_HELLO;
        pHdr->ucOrig = ptapp->clientId;
        pHdr->ucDest = ptapp->peerId;
        pHdr->ucSeq  = 0;
        pHdr->ulLen  = sizeof(*pCmd);

        strcpy(pCmd->greet, "Hello from Linux TZIOC test app.");

        err = tzioc_msg_send(
            ptapp->hClient,
            pHdr);

        if (err) {
            LOGE("failed to send hello msg");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        LOGI("msg hello testing %d done\n", rep);
    }

    /* switch to TZOS */
    tzioc_call_smc(ptapp->hClient, 0x83000007);

    /* wait for rpy or timeout */
    timeout = 10;
    while (ptapp->msgCnt > 0 && timeout--)
        nanosleep(&waittime, NULL);

    if (timeout == -1) {
        LOGE("timedout waiting for rpy");
        return -ETIMEDOUT;
    }

    LOGI("msg hello testing done!\n");
    return 0;
}

static int tzioc_tapp_mem_alloc(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    for (rep = 0; rep < MEM_ALLOC_TEST_REP; rep++) {
        uint8_t *vaddr;
        uint32_t offset;
        uint32_t buffsize;
        uint32_t checksum;
        size_t i;

        LOGI("mem alloc testing %d...", rep);

        buffsize = MEM_ALLOC_BUFF_SIZE << rep;

        /* allocate buffer from shared memory */
        vaddr = tzioc_mem_alloc(
            ptapp->hClient,
            buffsize);

        if (!vaddr) {
            LOGE("failed to alloc shared memory buffer");
            return -ENOMEM;
        }

        /* convert virtual address to offset */
        offset = tzioc_vaddr2offset(
            ptapp->hClient,
            (uint32_t)vaddr);

        if (offset == (uint32_t)-1) {
            LOGE("failed to convert virtual address to offset");
            return -EFAULT;
        }

        /* fill in random data */
        for (i = 0; i < buffsize; i += 4) {
            *(uint32_t *)(vaddr + i) = random();
        }

        /* calculate simple checksum */
        checksum = 0;
        for (i = 0; i < buffsize; i += 4) {
            checksum += *(uint32_t *)(vaddr + i);
        }

        LOGD("mem alloc local: offset 0x%x size 0x%x checksum 0x%x",
             offset, buffsize, checksum);

        /* send mem alloc cmd to peer */
        {
            static uint8_t msg[sizeof(struct tzioc_msg_hdr) +
                               sizeof(struct tzioc_test_msg_mem_alloc_cmd)];
            struct tzioc_msg_hdr *pHdr =
                (struct tzioc_msg_hdr *)msg;
            struct tzioc_test_msg_mem_alloc_cmd *pCmd =
                (struct tzioc_test_msg_mem_alloc_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

            pHdr->ucType = TZIOC_TEST_MSG_MEM_ALLOC;
            pHdr->ucOrig = ptapp->clientId;
            pHdr->ucDest = ptapp->peerId;
            pHdr->ucSeq  = 0;
            pHdr->ulLen  = sizeof(*pCmd);

            pCmd->offset = offset;
            pCmd->size = buffsize;

            err = tzioc_msg_send(
                ptapp->hClient,
                pHdr);

            if (err) {
                LOGE("failed to send mem alloc msg");
                return err;
            }

            pthread_mutex_lock(&ptapp->msgLock);
            ptapp->msgCnt++;
            pthread_mutex_unlock(&ptapp->msgLock);

            /* switch to TZOS */
            tzioc_call_smc(ptapp->hClient, 0x83000007);

            /* wait for rpy or timeout */
            timeout = 10;
            while (ptapp->msgCnt > 0 && timeout--)
                nanosleep(&waittime, NULL);

            if (timeout == -1) {
                LOGE("timedout waiting for rpy");
                return -ETIMEDOUT;
            }
        }

        /* compare checksum */
        if (ptapp->checksum != checksum) {
            LOGE("mismatched checksum from peer");
            return -EFAULT;
        }

        /* free shared memory buffer */
        tzioc_mem_free(
            ptapp->hClient,
            vaddr);

        LOGI("mem alloc testing %d done\n", rep);
    }

    LOGI("mem alloc testing done!\n");
    return 0;
}

static void *tzioc_tapp_msg_thread(void *arg)
{
    while (1) {
        static uint8_t msg[TZIOC_MSG_SIZE_MAX];
        struct tzioc_msg_hdr *pHdr = (struct tzioc_msg_hdr *)msg;

        /* receive incoming msg */
        int err = tzioc_msg_receive(
            ptapp->hClient,
            pHdr,
            TZIOC_MSG_SIZE_MAX,
            -1);

        if (err) {
            if (err != -EINTR) {
                LOGE("error receiving TZIOC msg, err %d", err);
            }
            continue;
        }

        /* process incoming msg */
        tzioc_tapp_msg_proc(pHdr);
    }

    return NULL;
}

static int tzioc_tapp_msg_proc(tzioc_msg_hdr *pHdr)
{
    pthread_mutex_lock(&ptapp->msgLock);
    ptapp->msgCnt--;
    pthread_mutex_unlock(&ptapp->msgLock);

    if (pHdr->ucOrig == TZIOC_CLIENT_ID_UAPPD) {
        switch (pHdr->ucType) {
        /* user app start rpy */
        case UAPPD_MSG_UAPP_START:
            {
                struct uappd_msg_uapp_start_rpy *pRpy =
                    (struct uappd_msg_uapp_start_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pRpy)) {
                    LOGE("invalid uapp start rpy received");
                    return -EINVAL;
                }

                LOGD("uapp start rpy: name %s, retVal %d",
                     pRpy->name, pRpy->retVal);
            }
            break;

        /* user app stop rpy */
        case UAPPD_MSG_UAPP_STOP:
            {
                struct uappd_msg_uapp_stop_rpy *pRpy =
                    (struct uappd_msg_uapp_stop_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pRpy)) {
                    LOGE("invalid uapp stop rpy received");
                    return -EINVAL;
                }

                LOGD("uapp stop rpy: name %s, retVal %d",
                     pRpy->name, pRpy->retVal);
            }
            break;

        /* user app getid rpy */
        case UAPPD_MSG_UAPP_GETID:
            {
                struct uappd_msg_uapp_getid_rpy *pRpy =
                    (struct uappd_msg_uapp_getid_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pRpy)) {
                    LOGE("invalid uapp getid rpy received");
                    return -EINVAL;
                }

                LOGD("uapp getid rpy: name %s, retVal %d, id %d",
                     pRpy->name, pRpy->retVal, pRpy->id);

                /* remember peer id */
                ptapp->peerId = pRpy->id;
            }
            break;

        /* user app exit nfy */
        case UAPPD_MSG_UAPP_EXIT:
            {
                struct uappd_msg_uapp_exit_nfy *pNfy =
                    (struct uappd_msg_uapp_exit_nfy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pNfy)) {
                    LOGE("invalid uapp exit nfy received");
                    return -EINVAL;
                }

                LOGD("uapp exit nfy: name %s",
                     pNfy->name);
            }
            break;

        default:
            LOGE("unknown uappd msg %d", pHdr->ucType);
            return -ENOENT;
        }
    }
    else if (pHdr->ucOrig == TZIOC_CLIENT_ID_ECHO) {
        switch (pHdr->ucType) {
        /* test echo msg */
        case TZIOC_TEST_MSG_ECHO:
            {
                struct tzioc_test_msg_echo *pEcho =
                    (struct tzioc_test_msg_echo *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pEcho)) {
                    LOGE("invalid echo msg received");
                    return -EINVAL;
                }

                LOGD("echo msg: value %d", pEcho->value);
            }
            break;

        default:
            LOGE("unknown echo msg %d", pHdr->ucType);
            return -ENOENT;
        }
    }
    else if (pHdr->ucOrig != 0 && pHdr->ucOrig == ptapp->peerId) {
        switch (pHdr->ucType) {
        case TZIOC_TEST_MSG_HELLO:
            {
                struct tzioc_test_msg_hello_rpy *pRpy =
                    (struct tzioc_test_msg_hello_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pRpy)) {
                    LOGE("invalid hello rpy received");
                    return -EINVAL;
                }

                LOGD("hello rpy: %s", pRpy->reply);
            }
            break;

        case TZIOC_TEST_MSG_MEM_ALLOC:
            {
                struct tzioc_test_msg_mem_alloc_rpy *pRpy =
                    (struct tzioc_test_msg_mem_alloc_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

                if (pHdr->ulLen  != sizeof(*pRpy)) {
                    LOGE("invalid mem alloc rpy received");
                    return -EINVAL;
                }

                LOGD("mem alloc rpy: checksum 0x%x", pRpy->checksum);

                /* remember checksum */
                ptapp->checksum = pRpy->checksum;
            }
            break;
        }
    }
    else {
        LOGE("unknown msg origin %d", pHdr->ucOrig);
        return -ENOENT;
    }

    return 0;
}
