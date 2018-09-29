/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "libastra_api.h"
#include "nexus_test_msg.h"
#include "nexus_tapp.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "nexus_memory.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

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

static int nexus_tapp_init(void);
static void nexus_tapp_exit(void);

static int nexus_tapp_info(void);
static int nexus_tapp_client(void);
static int nexus_tapp_file(void);
static int nexus_tapp_uapp_tapp(char *fileName);
static int nexus_tapp_peer_tapp(void);
static int nexus_tapp_msg_hello(void);
static int nexus_tapp_mem_alloc(void);

static void *nexus_tapp_msg_thread(void *arg);
static void nexus_tapp_msg_proc(void);

static void nexus_tapp_callback(
    astra_event event,
    void *pEventData,
    void *pPrivData);

/*
 * Variable Declarations
 */

static char nexus_tapp_name[16]="nexus_tapp";

static struct nexus_tapp nexus_tapp;
static struct nexus_tapp *ptapp = &nexus_tapp;
static  bool sNexusPlatformInitialized = false;
/*
 * Nexus Test App Functions
 */
/*
 * Nexus initalization
*/

static int init_nexus(void)
{
    int result = 0;

#ifdef NXCLIENT_SUPPORT

    NxClient_JoinSettings joinSettings;

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "nexus_tapp");
    joinSettings.ignoreStandbyRequest = true;
    result = NxClient_Join(&joinSettings);
    if (result) return -1;
    sNexusPlatformInitialized = true;
    return result;

#else

    NEXUS_PlatformSettings platformSettings;
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    sNexusPlatformInitialized = true;

return result;

#endif /* NXCLIENT_SUPPORT */

}

static void uninit_nexus(void)
{

#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else

    NEXUS_Platform_Uninit();

#endif  /* NXCLIENT_SUPPORT */

}

static int nexus_tapp_load_ta(char *src, char *dst)
{
    FILE *fp;
    int ta_bin_sz, ta_bin_sz_read, wbytes, err = 0;
    void *drmbuf;
    NEXUS_Addr drmbuf_offset;
    char dst_file[100];

    fp = fopen(src, "rb");
    if(!fp) {
        printf("Error opening the TA file\n");
        err = -EFAULT;
        goto ErrorExit;
    }
    fseek(fp, 0L, SEEK_END);
    ta_bin_sz = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    if(!ta_bin_sz) {
        err = -ENOMEM;
        goto ErrorExit;
    }

    NEXUS_Memory_Allocate(ta_bin_sz, NULL, &drmbuf);
    if (!drmbuf) {
        err = -ENOMEM;
        goto ErrorExit;
    }
    drmbuf_offset = NEXUS_AddrToOffset(drmbuf);
    if (!drmbuf_offset) {
        err = -ENOMEM;
        goto ErrorExit;
    }
    ta_bin_sz_read = fread(drmbuf, 1, ta_bin_sz, fp);
    if(ta_bin_sz_read != ta_bin_sz) {
        err = -EFAULT;
        goto ErrorExit;
    }
    snprintf ( dst_file, 100, "%s", dst);
    printf("\n\n TA files =%s\n",dst_file);
    /* open file to write */
    ptapp->hTAFile = astra_file_open(
        ptapp->hClient,
        dst_file,
        O_CREAT|O_EXCL);
    if (ptapp->hTAFile) {
        astra_file_close(ptapp->hTAFile);
        ptapp->hTAFile = astra_file_open(
            ptapp->hClient,
            dst_file,
            O_WRONLY|O_CREAT);
        if (!ptapp->hTAFile) {
            err = -EFAULT;
            goto ErrorExit;
        }
    } else {
        printf("\nFile already exists!\n");
        err = 0;
        goto ErrorExit;
    }
    wbytes = astra_file_write(
        ptapp->hTAFile,
        (astra_paddr_t)drmbuf_offset,
        ta_bin_sz_read);
        if (wbytes != ta_bin_sz_read) {
            err = -EFAULT;
            goto ErrorExit;
        }
        printf("\nFile write success !\n");
        astra_file_close(ptapp->hTAFile);
        NEXUS_Memory_Free(drmbuf);
        fclose(fp);
        return 0;
ErrorExit:
    if (fp)
        fclose(fp);
    if (ptapp->hTAFile)
        astra_file_close(ptapp->hTAFile);
    if(drmbuf)
        NEXUS_Memory_Free(drmbuf);
    return err;
}


int main(int argc, char *argv[])
{
    int err = 0;

    if (argc != 2) {
        printf("usage:\n");
        printf("nexus_tapp <TA Binary> \n");
        goto ERR_EXIT;
    }

    err = nexus_tapp_init();
    if (err) goto ERR_EXIT;

    printf("Astra testing starts...\n");

    /* info test */
    err = nexus_tapp_info();
    if (err) goto ERR_EXIT;

    /* client test */
    err = nexus_tapp_client();
    if (err) goto ERR_EXIT;

    /* file test */
    err = nexus_tapp_file();
    if (err) goto ERR_EXIT;

    /* uapp test - tapp */
    err = nexus_tapp_uapp_tapp(argv[1]);
    if (err) goto ERR_EXIT;

    /* peer test - tapp */
    err = nexus_tapp_peer_tapp();
    if (err) goto ERR_EXIT;

    /* msg test - hello */
    err = nexus_tapp_msg_hello();
    if (err) goto ERR_EXIT;

    /* mem test - alloc */
    err = nexus_tapp_mem_alloc();
    if (err) goto ERR_EXIT;

    printf("Astra testing done!\n");

 ERR_EXIT:
    nexus_tapp_exit();
    return err;
}

static int nexus_tapp_init(void)
{
    int err = 0;

    /* init test app control block */
    memset(&nexus_tapp, 0, sizeof(nexus_tapp));

    /* create msg thread */
    err = pthread_create(
        &ptapp->msgThread,
        NULL,
        &nexus_tapp_msg_thread,
        NULL);

    if (err) {
        printf("failed to create msg thread\n");
        return err;
    }

    /* init msg semaphore */
    sem_init(&ptapp->msgSem, 0, 0);

    /* init msg lock */
    pthread_mutex_init(&ptapp->msgLock, NULL);

    init_nexus();

    printf("Astra test app initialized\n");
    return 0;
}

static void nexus_tapp_exit(void)
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
    uninit_nexus();

    printf("Astra test app uninitialized\n");
}

/*
 * Astra Test Functions
 */

static int nexus_tapp_info(void)
{
    struct astra_version version;
    struct astra_config config;
    struct astra_status status;

    /* get astra version */
    astra_version_get(&version);

    printf("Astra version: %d.%d.%d\n",
         version.major,
         version.minor,
         version.build);

    /* get astra config */
    astra_config_get(&config);

    printf("Astra shared  mem size: 0x%lx\n", (size_t)config.smemSize);
    printf("Astra private mem size: 0x%lx\n", (size_t)config.pmemSize);

    /* get astra status */
    astra_status_get(&status);

    printf("Astra is %s\n", (status.up) ? "up" : "down");

    return (status.up) ? 0 : -ENOLINK;
}

static int nexus_tapp_client(void)
{
    int rep;

    /* open/close test client repeatedly */
    for (rep = 0; rep < CLIENT_TEST_REP; rep++) {

        printf("client testing %d...\n", rep);

        /* open test client */
        ptapp->hClient = astra_client_open(
            nexus_tapp_name,
            nexus_tapp_callback,
            0);  /* private data not used */

        if (!ptapp->hClient) {
            printf("failed to open client\n");
            return -EFAULT;
        }

        printf("client handle %p\n", (void *)ptapp->hClient);

        /* leave last test client */
        if (rep == CLIENT_TEST_REP - 1) {
            printf("client testing %d leave open\n", rep);
            break;
        }

        /* close test client */
        astra_client_close(ptapp->hClient);

        /* clear test client handle */
        ptapp->hClient = 0;

        printf("client testing %d done\n", rep);
    }

    printf("client testing done!\n");
    return 0;
}

static int nexus_tapp_file(void)
{
    int rep;
    int err = 0;
    char path[32];
    uint8_t *vaddr;
    astra_paddr_t paddr;
    uint32_t wchecksum, rchecksum;
    uint32_t wbytes, rbytes;
    uint32_t  i;

    /* open/close/write test file test_file.[n] */
    for (rep = 0; rep < FILE_TEST_REP; rep++) {
        printf("file testing %d...\n", rep);

        /* prepare file path */
        sprintf(path, "test_file.%d", rep);

        /* open file to write */
        ptapp->hFile = astra_file_open(
            ptapp->hClient,
            path,
            O_CREAT | O_WRONLY);

        if (!ptapp->hFile) {
            printf("failed to open file to write\n");
            err = -EFAULT;
            goto error;
        }

        printf("file write handle %p\n", (void *)ptapp->hFile);

        /* alloc file buffer */
        NEXUS_Memory_Allocate(FILE_BUFF_SIZE, NULL, (void*)&vaddr);
        if (!vaddr) {
            err = -ENOMEM;
            goto error;
        }
        paddr = (astra_paddr_t)NEXUS_AddrToOffset(vaddr);
        if (!paddr) {
            err = -ENOMEM;
            goto error;
        }
        /* fill in file buffer */
        /* fill in sequential data */
        for (i = 0; i < FILE_BUFF_SIZE; i++) {
            *(vaddr + i) = i & 0xFF;
        }
        /* calculate simple checksum */
        wchecksum = 0;
        for (i = 0; i < FILE_BUFF_SIZE; i += 4) {
            wchecksum += *(uint32_t *)(vaddr + i);
        }

        printf("file write: paddr 0x%x size 0x%x checksum 0x%x\n",
             (unsigned int)paddr, FILE_BUFF_SIZE, wchecksum);

        /* write to file */
        wbytes = astra_file_write(
            ptapp->hFile,
            paddr,
            FILE_BUFF_SIZE);

        if (wbytes != FILE_BUFF_SIZE) {
            printf("failed to write to file %d\n", wbytes);
            err = -EIO;
            goto error;
        }

        /* close file */
        astra_file_close(ptapp->hFile);

        /* open file to read */
        ptapp->hFile = astra_file_open(
            ptapp->hClient,
            path,
            O_RDONLY);

        if (!ptapp->hFile) {
            printf("failed to open file to read\n");
            err = -EFAULT;
            goto error;
        }

        printf("file read handle %p\n", (void *)ptapp->hFile);

        /* clear file buffer */
        memset(vaddr, 0, FILE_BUFF_SIZE);

        /* read from file */
        rbytes = astra_file_read(
            ptapp->hFile,
            paddr,
            FILE_BUFF_SIZE);

        if (rbytes != FILE_BUFF_SIZE) {
            printf("failed to read from file\n");
            err = -EIO;
            goto error;
        }

        /* calculate simple checksum */
        rchecksum = 0;
        for (i = 0; i < FILE_BUFF_SIZE; i += 4) {
            rchecksum += *(uint32_t *)(vaddr + i);
        }

        printf("file read: paddr 0x%x size 0x%x checksum 0x%x\n",
             (unsigned int)paddr, FILE_BUFF_SIZE, rchecksum);

        /* free file buffer - TBD */
        if(vaddr)
            NEXUS_Memory_Free(vaddr);

        /* close file */
        astra_file_close(ptapp->hFile);

        /* clear file handle */
        ptapp->hFile = 0;

        if (rchecksum != wchecksum) {
            printf("mismatched file checksums\n");
            err = -EIO;
            goto error;
        }
        printf("file testing %d done\n", rep);
    }

    printf("file testing done!\n");
    return 0;

error:
    if(vaddr)
        NEXUS_Memory_Free(vaddr);
    return err;
}

static int nexus_tapp_uapp_tapp(char * fileName)
{
    int rep;
    int err=0;
    err = nexus_tapp_load_ta(fileName,"/astra_tapp.elf");
    if (err) {
        printf("\n Error loading TA = %d\n", err);
        return -EFAULT;
    }
    /* open/close test uapp tapp repeatedly */
    for (rep = 0; rep < UAPP_TAPP_TEST_REP; rep++) {

        printf("uapp tapp testing %d...\n", rep);

        /* open test uapp tapp */
        ptapp->hUapp = astra_uapp_open(
            ptapp->hClient,
            "astra_tapp",
            "/astra_tapp.elf");

        if (!ptapp->hUapp) {
            printf("failed to open uapp tapp\n");
            return -EFAULT;
        }

        printf("uapp tapp handle %p\n",(void *)ptapp->hUapp);

        /* leave last test uapp tapp */
        if (rep == UAPP_TAPP_TEST_REP - 1) {
            printf("uapp tapp testing %d leave open\n", rep);
            break;
        }

        /* close test uapp tapp */
        astra_uapp_close(ptapp->hUapp);

        /* clear test client handle */
        ptapp->hUapp = 0;

        printf("uapp tapp testing %d done\n", rep);
    }

    printf("uapp tapp testing done!\n");
    return 0;
}

static int nexus_tapp_peer_tapp(void)
{
    int rep;

    /* open/close test peer repeatedly */
    for (rep = 0; rep < PEER_TAPP_TEST_REP; rep++) {

        printf("peer testing %d...\n", rep);

        /* open test peer */
        ptapp->hPeer = astra_peer_open(
            ptapp->hUapp,
            "astra_tapp");

        if (!ptapp->hPeer) {
            printf("failed to open peer");
            return -EFAULT;
        }

        printf("peer handle %p\n",(void *)ptapp->hPeer);

        /* leave last test peer */
        if (rep == PEER_TAPP_TEST_REP - 1) {
            printf("peer testing %d leave open\n", rep);
            break;
        }

        /* close test peer */
        astra_peer_close(ptapp->hPeer);

        /* clear test client handle */
        ptapp->hPeer = 0;

        printf("peer testing %d done\n", rep);
    }

    printf("peer testing done!\n");
    return 0;
}

static int nexus_tapp_msg_hello(void)
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

        printf("msg hello testing %d...\n", rep);

        pHdr->ucType = ASTRA_TEST_MSG_HELLO;
        pHdr->ucSeq  = 0;

        strcpy(pCmd->greet, "Hello from Linux astra test app.");

        err = astra_msg_send(
            ptapp->hPeer,
            (void *)&msg,
            sizeof(msg));

        if (err) {
            printf("failed to send hello msg\n");
            return err;
        }

        pthread_mutex_lock(&ptapp->msgLock);
        ptapp->msgCnt++;
        pthread_mutex_unlock(&ptapp->msgLock);

        printf("msg hello testing %d done\n", rep);
    }

    /* switch to TZOS */
    astra_call_smc(ptapp->hClient, ASTRA_SMC_CODE_SWITCH);

    /* wait for rpy or timeout */
    timeout = 10;
    while (ptapp->msgCnt > 0 && timeout--)
        nanosleep(&waittime, NULL);

    if (timeout == -1) {
        printf("timedout waiting for rpy\n");
        return -ETIMEDOUT;
    }

    printf("msg hello testing done!\n");
    return 0;
}

static int nexus_tapp_mem_alloc(void)
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

        printf("mem alloc testing %d...\n", rep);

        buffsize = MEM_ALLOC_BUFF_SIZE << rep;

        /* allocate buffer from shared memory */
        vaddr = astra_mem_alloc(
            ptapp->hClient,
            buffsize);

        if (!vaddr) {
            printf("failed to alloc shared memory buffer\n");
            return -ENOMEM;
        }

        /* convert virtual address to offset */
        offset = astra_vaddr2offset(
            ptapp->hClient,
            vaddr);

        if (offset == 0) {
            printf("failed to convert virtual address to offset\n");
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

        printf("mem alloc local: offset 0x%x size 0x%x checksum 0x%x\n",
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
                printf("failed to send mem alloc msg\n");
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
                printf("timedout waiting for rpy\n");
                return -ETIMEDOUT;
            }
        }

        /* free shared memory buffer */
        astra_mem_free(
            ptapp->hClient,
            vaddr);

        /* compare checksum */
        if (ptapp->checksum != checksum) {
            printf("mismatched checksum from peer\n");
            return -EFAULT;
        }

        printf("mem alloc testing %d done\n", rep);
    }

    printf("mem alloc testing done!\n");
    return 0;
}

static void *nexus_tapp_msg_thread(void *arg)
{
    UNUSED(arg);
    while (1) {
        sem_wait(&ptapp->msgSem);

        /* process incoming msg */
        nexus_tapp_msg_proc();
    }

    return NULL;
}

static void nexus_tapp_msg_proc()
{
    static uint8_t msg[1024];
    uint32_t msgLen = 1024 - 16;
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
                    printf("invalid hello rpy received\n");
                    return;
                }

                printf("hello rpy: %s\n", pRpy->reply);
            }
            break;

        case ASTRA_TEST_MSG_MEM_ALLOC:
            {
                struct astra_test_msg_mem_alloc_rpy *pRpy =
                    (struct astra_test_msg_mem_alloc_rpy *)ASTRA_TEST_MSG_PAYLOAD(pHdr);

                if (msgLen != sizeof(*pHdr) + sizeof(*pRpy)) {
                    printf("invalid mem alloc rpy received\n");
                    return;
                }

                printf("mem alloc rpy: checksum 0x%x\n", pRpy->checksum);

                /* remember checksum */
                ptapp->checksum = pRpy->checksum;
            }
            break;

        default:
            printf("unknown peer msg %d\n", pHdr->ucType);
            return;
        }
    }
}

static void nexus_tapp_callback(
    astra_event event,
    void *pEventData,
    void *pPrivData)
{
    UNUSED(pEventData);
    UNUSED(pPrivData);
    switch (event) {
    case ASTRA_EVENT_MSG_RECEIVED:
        printf("client event callback, msg received\n");
        sem_post(&ptapp->msgSem);
        break;

    case ASTRA_EVENT_UAPP_EXIT:
        printf("client event callback, userapp exit\n");
        break;

    default:
        printf("client event callback, unknown event %d\n", event);
    }
}
