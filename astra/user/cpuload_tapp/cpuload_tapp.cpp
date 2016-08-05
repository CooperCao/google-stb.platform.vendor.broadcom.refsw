/***************************************************************************
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
 ***************************************************************************/

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdarg.h>

#include "cpuload_tapp.h"

// Static non-const data from TzIoc class
tzioc_client_handle CpuLoadTApp::hClient;
int CpuLoadTApp::msgQ;
uint8_t CpuLoadTApp::clientId;
pthread_t edf_thread;

volatile bool quitThread;
static pthread_mutex_t mutex;


volatile bool appQuit;

#define PRINTF_BUF_SIZE 2048

int printf(const char *templat,...){

    va_list marker;
    int count;
    char buffer[PRINTF_BUF_SIZE];

    va_start(marker, templat);
    count = vsprintf(buffer, templat, marker);
    va_end(marker);

    write(1,&buffer,count);
    return count;

}

static void *thread_edf(void *arg)
{
    int s;
    int policy;
    struct sched_param param;

    printf("thread_start started. pid %d\n", getpid());

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    param.sched_priority = 15;
    policy = SCHED_DEADLINE;
    //policy = SCHED_OTHER;

    s = pthread_setschedparam(pthread_self(), policy, &param);
    printf("\t pthread_setschedparam %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    bool shouldExit = false;
    do{


        pthread_mutex_lock(&mutex);
        shouldExit = quitThread;
        pthread_mutex_unlock(&mutex);



        //printf("EDF Thread !!!\n");

    }while(shouldExit == false);


    printf("Exit Thread !!!! \n");
    return NULL;
}


void CpuLoadTApp::init()
{
    int s = 1;

    // Open TZIOC client
    hClient = tzioc_client_open(
        "cpuload_tapp",
        &msgQ,
        &clientId);

    if (!hClient) {
        LOGE("failed to open TZIOC client");
        throw(-EIO);
    }
    appQuit = false;

    quitThread = false;
    pthread_mutex_init(&mutex, NULL);

    s = pthread_create(&edf_thread, NULL, &thread_edf, NULL);
    if (s != 0)
        printf("Error pthread_create");

    LOGI("CpuLoad App initialized!");
}

void CpuLoadTApp::deinit()
{

    int s;
    LOGI("CpuLoad App de-initialize!");

    pthread_mutex_lock(&mutex);
    quitThread = true;
    pthread_mutex_unlock(&mutex);


    s = pthread_join(edf_thread, NULL);
    if (s != 0)
        printf("Error pthread_join");

    // Close TZIOC client
    if (hClient) {
        tzioc_client_close(hClient);
        hClient = NULL;
    }

    LOGI("CpuLoad App deinitialized!!!!!");
}

void CpuLoadTApp::run()
{
    LOGI("CpuLoad App running!");

    while (appQuit == false) {
        static uint8_t msg[TZIOC_MSG_SIZE_MAX];
        struct tzioc_msg_hdr *pHdr = (struct tzioc_msg_hdr *)msg;

        // Receive incoming msg
        int err = tzioc_msg_receive(
            hClient,
            pHdr,
            TZIOC_MSG_SIZE_MAX,
            -1);
        LOGI("tzioc_msg_receive !!");

        if (err) {
            if (err != -EINTR) {
                LOGE("error receiving TZIOC msg, err %d", err);
            }
            continue;
        }

        // Process incoming msg
        msgProc(pHdr);
    }

    LOGI("CpuLoad App finished!");
}

void CpuLoadTApp::msgProc(
    struct tzioc_msg_hdr *pHdr)
{
    LOGE("Msg Recieved %d \n",pHdr->ucType);
    switch (pHdr->ucType) {
    case CPU_LOAD_MSG_HELLO:
        helloMsg(pHdr);
        break;
    case CPU_LOAD_MSG_STOP:
        stopMsg(pHdr);
        appQuit = true;
        break;
    default:
        LOGW("unknown cpuload_tapp msg %d", pHdr->ucType);
    }
}

void CpuLoadTApp::stopMsg(
    struct tzioc_msg_hdr *pHdr)
{
    struct cpu_load_msg_stop *pCmd =
        (struct cpu_load_msg_stop*)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid hello cmd received");
        return;
    }

    LOGD("Stop rec %d", pCmd->value);

    // Caution: reused the cmd buffer for rpy.
    struct cpu_load_msg_stop *pRpy =
        (struct cpu_load_msg_stop *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->value = 1;

    tzioc_msg_send(hClient, pHdr);
}

void CpuLoadTApp::helloMsg(
    struct tzioc_msg_hdr *pHdr)
{
    struct cpu_load_msg_hello_cmd *pCmd =
        (struct cpu_load_msg_hello_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid hello cmd received");
        return;
    }

    LOGD("hello cmd: %s", pCmd->greet);

    // Caution: reused the cmd buffer for rpy.
    struct cpu_load_msg_hello_rpy *pRpy =
        (struct cpu_load_msg_hello_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    strcpy(pRpy->reply, "Reply from CPULoad TA.");

    tzioc_msg_send(hClient, pHdr);
}

int main(int argc, char **argv)
{

    printf("main started. pid %d\n", getpid());

    int s;
    int policy;
    struct sched_param param;
    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    param.sched_priority = 10;
    policy = SCHED_DEADLINE;
    //policy = SCHED_OTHER;

    s = pthread_setschedparam(pthread_self(), policy, &param);
    printf("\t pthread_setschedparam %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);


    try {

        CpuLoadTApp::init();
        CpuLoadTApp::run();
        CpuLoadTApp::deinit();
    }
    catch (int exception) {
        LOGE("fatal error %d", exception);
        LOGI("CpuLoad App terminated abnormally");
        exit(exception);
    }
    return 0;
}
