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
#include <fcntl.h>
#include <time.h>

#include "libastra_api.h"
#include "astra_test_msg.h"
#include "astra_tapp.h"

/*
 * Compiler Switches
 */


/*
 * Constant Definitions
 */

#define CLIENT_TEST_REP         16
#define FILE_TEST_REP           2
#define UAPP_HELLO_TEST_REP     2
#define UAPP_TAPP_TEST_REP      2
#define PEER_TAPP_TEST_REP      2
#define MSG_HELLO_TEST_REP      3
#define MEM_ALLOC_TEST_REP      4

#define FILE_BUFF_SIZE          (8192)
#define MEM_ALLOC_BUFF_SIZE     (1024)

/*
 * Function Declarations
 */

static int astra_tapp_init(void);
static void astra_tapp_exit(void);

static int astra_tapp_info(void);
static int astra_tapp_client(void);
static int astra_tapp_file(void);
static int astra_tapp_uapp_hello(void);
static int astra_tapp_uapp_tapp(void);
static int astra_tapp_peer_tapp(void);
static int astra_tapp_msg_hello(void);
static int astra_tapp_mem_alloc(void);

static void *astra_tapp_msg_thread(void *arg);
static void astra_tapp_msg_proc();

static void astra_tapp_callback(
    astra_event event,
    void *pEventData,
    void *pPrivData);

/*
 * Variable Declarations
 */

static char astra_tapp_name[16]="astra_tapp";

static struct astra_tapp astra_tapp;
static struct astra_tapp *ptapp = &astra_tapp;

/*
 * Astra Test App Functions
 */

int main(void)
{
    int err = 0;

    err = astra_tapp_init();
    if (err) goto ERR_EXIT;

    LOGI("Astra testing starts...\n");

    /* info test */
    err = astra_tapp_info();
    if (err) goto ERR_EXIT;

    /* client test */
    err = astra_tapp_client();
    if (err) goto ERR_EXIT;

    /* file test */
    err = astra_tapp_file();
    if (err) goto ERR_EXIT;

    /* uapp test - hello */
    err = astra_tapp_uapp_hello();
    if (err) goto ERR_EXIT;

    /* uapp test - tapp */
    err = astra_tapp_uapp_tapp();
    if (err) goto ERR_EXIT;

    /* peer test - tapp */
    err = astra_tapp_peer_tapp();
    if (err) goto ERR_EXIT;

    /* msg test - hello */
    err = astra_tapp_msg_hello();
    if (err) goto ERR_EXIT;

    /* mem test - alloc */
    err = astra_tapp_mem_alloc();
    if (err) goto ERR_EXIT;

    LOGI("Astra testing done!\n");

 ERR_EXIT:
    astra_tapp_exit();
    return err;
}

static int astra_tapp_init(void)
{
    int err = 0;

    /* init test app control block */
    memset(&astra_tapp, 0, sizeof(astra_tapp));

    /* create msg thread */
    err = pthread_create(
        &ptapp->msgThread,
        NULL,
        &astra_tapp_msg_thread,
        NULL);

    if (err) {
        LOGE("failed to create msg thread");
        return err;
    }

    /* init msg semaphore */
    sem_init(&ptapp->msgSem, 0, 0);

    /* init msg lock */
    pthread_mutex_init(&ptapp->msgLock, NULL);

    LOGI("Astra test app initialized\n");
    return 0;
}

static void astra_tapp_exit(void)
{
    if (ptapp->hPeer) {
        /* close test peer */
        astra_peer_close(ptapp->hPeer);

        /* clear test peer handle */
        ptapp->hPeer = 0;
    }

    if (ptapp->hUapp) {
        /* close test uapp */
        astra_uapp_close(ptapp->hUapp);

        /* clear test uapp handle */
        ptapp->hUapp = 0;
    }

    if (ptapp->hClient) {
        /* close test client */
        astra_client_close(ptapp->hClient);

        /* clear test client handle */
        ptapp->hClient = 0;
    }

    LOGI("Astra test app uninitialized\n");
}

/*
 * Astra Test Functions
 */

static int astra_tapp_info(void)
{
    struct astra_version version;
    struct astra_config config;
    struct astra_status status;

    /* get astra version */
    astra_version_get(&version);

    LOGI("Astra version: %d.%d.%d",
         version.major,
         version.minor,
         version.build);

    /* get astra config */
    astra_config_get(&config);

    LOGI("Astra shared  mem size: 0x%zx", (size_t)config.smemSize);
    LOGI("Astra private mem size: 0x%zx", (size_t)config.pmemSize);

    /* get astra status */
    astra_status_get(&status);

    LOGI("Astra is %s\n", (status.up) ? "up" : "down");

    return (status.up) ? 0 : -ENOLINK;
}

static int astra_tapp_client(void)
{
    int rep;

    /* open/close test client repeatedly */
    for (rep = 0; rep < CLIENT_TEST_REP; rep++) {

        LOGI("client testing %d...", rep);

        /* open test client */
        ptapp->hClient = astra_client_open(
            astra_tapp_name,
            astra_tapp_callback,
            0);  /* private data not used */

        if (!ptapp->hClient) {
            LOGE("failed to open client");
            return -EFAULT;
        }

        LOGD("client handle %p", ptapp->hClient);

        /* leave last test client */
        if (rep == CLIENT_TEST_REP - 1) {
            LOGD("client testing %d leave open\n", rep);
            break;
        }

        /* close test client */
        astra_client_close(ptapp->hClient);

        /* clear test client handle */
        ptapp->hClient = 0;

        LOGI("client testing %d done\n", rep);
    }

    LOGI("client testing done!\n");
    return 0;
}

static int astra_tapp_file(void)
{
    int rep;

    /* open/close/write test file test_file.[n] */
    for (rep = 0; rep < FILE_TEST_REP; rep++) {
        char path[32];
#if 0
        uint8_t *vaddr;
        astra_paddr_t paddr;
        uint32_t wchecksum, rchecksum;
        size_t wbytes, rbytes;
        size_t i;
#endif
        LOGI("file testing %d...", rep);

        /* prepare file path */
        sprintf(path, "test_file.%d", rep);

        /* open file to write */
        ptapp->hFile = astra_file_open(
            ptapp->hClient,
            path,
            O_CREAT | O_WRONLY);

        if (!ptapp->hFile) {
            LOGE("failed to open file to write");
            return -EFAULT;
        }

        LOGD("file write handle %p", ptapp->hFile);
#if 0
        /* alloc file buffer - TBD */
        vaddr = NULL;
        paddr = 0;

        /* fill in file buffer */
        if (rep == 0) {
            /* fill in sequential data */
            for (i = 0; i < FILE_BUFF_SIZE; i++) {
                *(vaddr + i) = i & 0xFF;
            }
        }
        else {
            /* fill in random data */
            get_random_bytes(vaddr, FILE_BUFF_SIZE);
        }

        /* calculate simple checksum */
        wchecksum = 0;
        for (i = 0; i < FILE_BUFF_SIZE; i += 4) {
            wchecksum += *(uint32_t *)(vaddr + i);
        }

        LOGD("file write: paddr 0x%x size 0x%x checksum 0x%x",
             (unsigned int)__pa(vaddr), FILE_BUFF_SIZE, wchecksum);

        /* write to file */
        wbytes = astra_file_write(
            ptapp->hFile,
            paddr,
            FILE_BUFF_SIZE);

        if (wbytes != FILE_BUFF_SIZE) {
            LOGE("failed to write to file %d", wbytes);
            return -EIO;
        }
#endif
        /* close file */
        astra_file_close(ptapp->hFile);

        /* open file to read */
        ptapp->hFile = astra_file_open(
            ptapp->hClient,
            path,
            O_RDONLY);

        if (!ptapp->hFile) {
            LOGE("failed to open file to read");
            return -EFAULT;
        }

        LOGD("file read handle %p", ptapp->hFile);
#if 0
        /* clear file buffer */
        memset(vaddr, 0, FILE_BUFF_SIZE);

        /* read from file */
        rbytes = astra_file_read(
            ptapp->hFile,
            paddr,
            FILE_BUFF_SIZE);

        if (rbytes != FILE_BUFF_SIZE) {
            LOGE("failed to read from file");
            return -EIO;
        }

        /* calculate simple checksum */
        rchecksum = 0;
        for (i = 0; i < FILE_BUFF_SIZE; i += 4) {
            rchecksum += *(uint32_t *)(vaddr + i);
        }

        LOGD("file read: paddr 0x%x size 0x%x checksum 0x%x",
             (unsigned int)__pa(vaddr), FILE_BUFF_SIZE, rchecksum);

        /* free file buffer - TBD */
#endif
        /* close file */
        astra_file_close(ptapp->hFile);

        /* clear file handle */
        ptapp->hFile = 0;
#if 0
        if (rchecksum != wchecksum) {
            LOGE("mismatched file checksums");
            return -EIO;
        }
#endif
        LOGI("file testing %d done\n", rep);
    }

    LOGI("file testing done!\n");
    return 0;
}

static int astra_tapp_uapp_hello(void)
{
    int rep;

    /* open/close test uapp hello repeatedly */
    for (rep = 0; rep < UAPP_HELLO_TEST_REP; rep++) {

        LOGI("uapp hello testing %d...", rep);

        /* open test uapp hello */
        ptapp->hUapp = astra_uapp_open(
            ptapp->hClient,
            "hello",
            "hello.elf");

        if (!ptapp->hUapp) {
            LOGE("failed to open uapp hello");
            return -EFAULT;
        }

        LOGD("uapp hello handle %p", ptapp->hUapp);

        /* close test uapp hello */
        astra_uapp_close(ptapp->hUapp);

        /* clear test client handle */
        ptapp->hUapp = 0;

        LOGI("uapp hello testing %d done\n", rep);
    }

    LOGI("uapp hello testing done!\n");
    return 0;
}

static int astra_tapp_uapp_tapp(void)
{
    int rep;

    /* open/close test uapp tapp repeatedly */
    for (rep = 0; rep < UAPP_TAPP_TEST_REP; rep++) {

        LOGI("uapp tapp testing %d...", rep);

        /* open test uapp tapp */
        ptapp->hUapp = astra_uapp_open(
            ptapp->hClient,
            "astra_tapp",
            "astra_tapp.elf");

        if (!ptapp->hUapp) {
            LOGE("failed to open uapp tapp");
            return -EFAULT;
        }

        LOGD("uapp tapp handle %p", ptapp->hUapp);

        /* leave last test uapp tapp */
        if (rep == UAPP_TAPP_TEST_REP - 1) {
            LOGD("uapp tapp testing %d leave open\n", rep);
            break;
        }

        /* close test uapp tapp */
        astra_uapp_close(ptapp->hUapp);

        /* clear test client handle */
        ptapp->hUapp = 0;

        LOGI("uapp tapp testing %d done\n", rep);
    }

    LOGI("uapp tapp testing done!\n");
    return 0;
}

static int astra_tapp_peer_tapp(void)
{
    int rep;

    /* open/close test peer repeatedly */
    for (rep = 0; rep < PEER_TAPP_TEST_REP; rep++) {

        LOGI("peer testing %d...", rep);

        /* open test peer */
        ptapp->hPeer = astra_peer_open(
            ptapp->hUapp,
            "astra_tapp");

        if (!ptapp->hPeer) {
            LOGE("failed to open peer");
            return -EFAULT;
        }

        LOGD("peer handle %p", ptapp->hPeer);

        /* leave last test peer */
        if (rep == PEER_TAPP_TEST_REP - 1) {
            LOGD("peer testing %d leave open\n", rep);
            break;
        }

        /* close test peer */
        astra_peer_close(ptapp->hPeer);

        /* clear test client handle */
        ptapp->hPeer = 0;

        LOGI("peer testing %d done\n", rep);
    }

    LOGI("peer testing done!\n");
    return 0;
}

static int astra_tapp_msg_hello(void)
{
    struct timespec waittime = {1, 0};
    int timeout;
    int rep;
    int err = 0;

    /* send hello cmd to peer */
    for (rep = 0; rep < MSG_HELLO_TEST_REP; rep++) {
        static uint8_t msg[sizeof(struct astra_test_msg_hdr) +
                           sizeof(struct astra_test_msg_hello_cmd)];
        struct astra_test_msg_hdr *pHdr =
            (struct astra_test_msg_hdr *)msg;
        struct astra_test_msg_hello_cmd *pCmd =
            (struct astra_test_msg_hello_cmd *)ASTRA_TEST_MSG_PAYLOAD(pHdr);

        LOGI("msg hello testing %d...", rep);

        pHdr->ucType = ASTRA_TEST_MSG_HELLO;
        pHdr->ucSeq  = 0;

        strcpy(pCmd->greet, "Hello from Linux astra test app.");

        err = astra_msg_send(
            ptapp->hPeer,
            (void *)&msg,
            sizeof(msg));

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
    astra_call_smc(ptapp->hClient, ASTRA_SMC_CODE_SWITCH);

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

static int astra_tapp_mem_alloc(void)
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
        vaddr = astra_mem_alloc(
            ptapp->hClient,
            buffsize);

        if (!vaddr) {
            LOGE("failed to alloc shared memory buffer");
            return -ENOMEM;
        }

        /* convert virtual address to offset */
        offset = astra_vaddr2offset(
            ptapp->hClient,
            vaddr);

        if (offset == 0) {
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
            static uint8_t msg[sizeof(struct astra_test_msg_hdr) +
                               sizeof(struct astra_test_msg_mem_alloc_cmd)];
            struct astra_test_msg_hdr *pHdr =
                (struct astra_test_msg_hdr *)msg;
            struct astra_test_msg_mem_alloc_cmd *pCmd =
                (struct astra_test_msg_mem_alloc_cmd *)ASTRA_TEST_MSG_PAYLOAD(pHdr);

            pHdr->ucType = ASTRA_TEST_MSG_MEM_ALLOC;
            pHdr->ucSeq  = 0;

            pCmd->offset = offset;
            pCmd->size = buffsize;

            err = astra_msg_send(
                ptapp->hPeer,
                (void *)&msg,
                sizeof(msg));

            if (err) {
                LOGE("failed to send mem alloc msg");
                return err;
            }

            pthread_mutex_lock(&ptapp->msgLock);
            ptapp->msgCnt++;
            pthread_mutex_unlock(&ptapp->msgLock);

            /* switch to TZOS */
            astra_call_smc(ptapp->hClient, ASTRA_SMC_CODE_SWITCH);

            /* wait for rpy or timeout */
            timeout = 10;
            while (ptapp->msgCnt > 0 && timeout--)
                nanosleep(&waittime, NULL);

            if (timeout == -1) {
                LOGE("timedout waiting for rpy");
                return -ETIMEDOUT;
            }
        }

        /* free shared memory buffer */
        astra_mem_free(
            ptapp->hClient,
            vaddr);

        /* compare checksum */
        if (ptapp->checksum != checksum) {
            LOGE("mismatched checksum from peer");
            return -EFAULT;
        }

        LOGI("mem alloc testing %d done\n", rep);
    }

    LOGI("mem alloc testing done!\n");
    return 0;
}

static void *astra_tapp_msg_thread(void *arg)
{
    while (1) {
        sem_wait(&ptapp->msgSem);

        /* process incoming msg */
        astra_tapp_msg_proc();
    }

    return NULL;
}

static void astra_tapp_msg_proc()
{
    static uint8_t msg[1024];
    size_t msgLen = 1024;
    astra_peer_handle hPeer;
    int err = 0;

    while (1) {
        struct astra_test_msg_hdr *pHdr =
            (struct astra_test_msg_hdr *)msg;

        err = astra_msg_receive(
            ptapp->hClient,
            &hPeer,
            &msg,
            &msgLen,
            ASTRA_WAIT_NONE);

        if (err) {
            if (err != -ENOMSG) continue;
            return;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt--;
        pthread_mutex_unlock(&ptapp->msgLock);

        switch (pHdr->ucType) {
        case ASTRA_TEST_MSG_HELLO:
            {
                struct astra_test_msg_hello_rpy *pRpy =
                    (struct astra_test_msg_hello_rpy *)ASTRA_TEST_MSG_PAYLOAD(pHdr);

                if (msgLen != sizeof(*pHdr) + sizeof(*pRpy)) {
                    LOGE("invalid hello rpy received");
                    return;
                }

                LOGD("hello rpy: %s", pRpy->reply);
            }
            break;

        case ASTRA_TEST_MSG_MEM_ALLOC:
            {
                struct astra_test_msg_mem_alloc_rpy *pRpy =
                    (struct astra_test_msg_mem_alloc_rpy *)ASTRA_TEST_MSG_PAYLOAD(pHdr);

                if (msgLen != sizeof(*pHdr) + sizeof(*pRpy)) {
                    LOGE("invalid mem alloc rpy received");
                    return;
                }

                LOGD("mem alloc rpy: checksum 0x%x", pRpy->checksum);

                /* remember checksum */
                ptapp->checksum = pRpy->checksum;
            }
            break;

        default:
            LOGE("unknown peer msg %d", pHdr->ucType);
            return;
        }
    }
}

static void astra_tapp_callback(
    astra_event event,
    void *pEventData,
    void *pPrivData)
{
    switch (event) {
    case ASTRA_EVENT_MSG_RECEIVED:
        LOGD("client event callback, msg received");
        sem_post(&ptapp->msgSem);
        break;

    case ASTRA_EVENT_UAPP_EXIT:
        LOGD("client event callback, userapp exit");
        break;

    default:
        LOGE("client event callback, unknown event %d", event);
    }
}
