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

#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <pthread.h>

#define assert(cond) if (!(cond)) { printf("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (1) {} }

typedef unsigned int * uintptr_t;

struct pthread {
    struct pthread *self;
    void **dtv, *unused1, *unused2;
    uintptr_t sysinfo;
    uintptr_t canary;
    pid_t tid, pid;
    int tsd_used, errno_val;
    volatile int cancel, canceldisable, cancelasync;
    int detached;
    unsigned char *map_base;
    size_t map_size;
    void *stack;
    size_t stack_size;
    void *start_arg;
    void *(*start)(void *);
    void *result;
    struct __ptcb *cancelbuf;
    void **tsd;
    pthread_attr_t attr;
    volatile int dead;
    struct {
        volatile void *volatile head;
        long off;
        volatile void *volatile pending;
    } robust_list;
    int unblock_cancel;
    int timer_id;
    locale_t locale;
    int killlock[2];
    int exitlock[2];
    int startlock[2];
    unsigned long sigmask[_NSIG/8/sizeof(long)];
    void *stdio_locks;
};

/*
 * A thread that goes to sleep for long durations
 */
void *periodicWaker(void *arg) {

    int i = 0;
    while (i < 10) {
        printf("%d %d: wokeup\n", getpid(), i);

        sleep(1);
        i++;
    }

    return (void *)i;
}


/*
 * The classic single producer single consumer problem with
 * pthread mutexes and condition variables.
 */
static int fifo[256];
static int head = -1;
static int tail = -1;
static pthread_mutex_t mutex;
static pthread_cond_t cvProducer, cvConsumer;

static const int ProduceNumElements = 512; //1024*1024;
static int numElements() {
    if (head < tail)
        return (tail - head);

    if (tail == -1)
        return 0;

    return (256 - tail) + head + 1;
}

static void *producer(void *arg) {
    printf("started producer, pid %d\n", getpid());
    int numProduced = 0;
    while (numProduced < ProduceNumElements) {
        pthread_mutex_lock(&mutex);

        if ((tail == head) && (tail != -1)){
            printf("producer blocked\n");
            pthread_cond_wait(&cvProducer, &mutex);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        if (tail == -1) {
            tail = 0;
            head = 0;
        }

        fifo[tail] = numProduced++;
        tail = (tail + 1)%256;

        int ne = numElements();
        //printf("%s: produced %d ne %d head %d tail %d\n", __FUNCTION__, numProduced, ne, head, tail);

        pthread_mutex_unlock(&mutex);

        if (ne > 16) {
            //printf("%s: signaling consumer\n", __FUNCTION__);
            pthread_cond_signal(&cvConsumer);
            //printf("consumer signaled\n");
        }
    }

}

static void *consumer(void *arg) {
    printf("Consumer started. pid %d\n", getpid());
    int numConsumed = 0;
    while (numConsumed < ProduceNumElements) {
        pthread_mutex_lock(&mutex);

        if (head == -1) {
            printf("consumer blocked\n");
            pthread_cond_wait(&cvConsumer, &mutex);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        fifo[head] = -1;
        head = (head + 1)%256;

        if (head == tail) {
            head = -1;
            tail = -1;
        }

        numConsumed++;
        int ne = numElements();

        //printf("%s: consumed %d ne %d head %d tail %d\n", __FUNCTION__, numConsumed, ne, head, tail);

        pthread_mutex_unlock(&mutex);

        if (ne < 8) {
            //printf("%s: signaling producer\n", __FUNCTION__);
            pthread_cond_signal(&cvProducer);
        }
    }
}

static void runProducerConsumerTest() {
    pthread_t prod;
    pthread_t cons;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cvProducer, NULL);
    pthread_cond_init(&cvConsumer, NULL);

    int rc = pthread_create(&prod, NULL, producer, 0);
    assert(rc == 0);

    rc = pthread_create(&cons, NULL, consumer, 0);
    assert(rc == 0);

    void *rv;
    rc = pthread_join(prod, &rv);
    assert(rc == 0);
    printf("producer joined\n");

    rc = pthread_join(cons, &rv);
    assert(rc == 0);
    printf("consumer joined\n");
}

/*
 * A compute heavy thread
 */
static void *lockerDoors(void *arg) {
    int numDoors = (int)arg;
    unsigned char *doors = malloc((numDoors+1) * sizeof(unsigned char));
    assert(doors != NULL);

    for (int i=1; i<=numDoors; i++)
        doors[i] = 0;

    for (int i=1; i<=numDoors; i++) {
        for (int j=i; j<=numDoors; j+=i) {
            doors[j] = doors[j] ^ 1;
        }
    }

    int numClosed = 0;
    for (int i=0; i<numDoors; i++)
        if (doors[i] == 0)
            numClosed++;

    printf("%s: NumDoors %d closed %d open %d\n", __FUNCTION__, numDoors, numClosed, (numDoors-numClosed));

    free(doors);
    return (void *)numClosed;
}

static void runLockerDoorTest() {
    pthread_t workers[4];
    for (int i=0; i<4; i++) {
        int arg = 4096*i;
        int rc = pthread_create(&workers[i], NULL, lockerDoors, (void *)arg);
        assert(rc == 0);
    }

    for (int i=0; i<4; i++) {
        void *result;
        int rc = pthread_join(workers[i], &result);
        assert(rc == 0);

        printf("Thread %d returned %d\n", i, (int)result);
    }
}

static void runSleeperTest() {
    pthread_t sleeper;

    printf("pid %d: spawn thread %d\n", getpid(), sizeof(struct pthread));

    if (pthread_create(&sleeper, NULL, periodicWaker, 0)) {
        fprintf(stderr, "Error creating thread\n");
        return;
    }

    void *rc;
    if (pthread_join(sleeper, &rc)) {
        fprintf(stderr, "Error joining thread\n");
        return;
    }

    printf("Thread joined - rc %d. Now exiting\n", (int)rc);

    return;
}

// extern void playEuclidGame();


static void *
thread_idle(void *arg)
{

    int s;
    int policy;
    struct sched_param param;
    printf("thread_start started. pid %d\n", getpid());

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);
    s = pthread_setschedprio(pthread_self(), 15);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    sleep(20);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    param.sched_priority = 25;
    policy = SCHED_OTHER;
    s = pthread_setschedparam(pthread_self(), policy, &param);
    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);
    sleep(20);

    return NULL;
}

static void *
thread_edf1(void *arg)
{
    int s;
    int policy;
    struct sched_param param;

    printf("thread_start started. pid %d\n", getpid());

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    param.sched_priority = 10;
    policy = SCHED_DEADLINE;

    s = pthread_setschedparam(pthread_self(), policy, &param);
    printf("\t pthread_setschedparam %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_setschedprio(pthread_self(), 15);
    printf("\t pthread_setschedprio %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    return NULL;
}


static void *
thread_edf2(void *arg)
{
    int s;
    int policy;
    struct sched_param param;

    printf("thread_start started. pid %d\n", getpid());

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    param.sched_priority = 10;
    policy = SCHED_DEADLINE;

    s = pthread_setschedparam(pthread_self(), policy, &param);
    printf("\t pthread_setschedparam %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_setschedprio(pthread_self(), 10);
    printf("\t pthread_setschedprio %d\n",s);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    s = pthread_getschedparam(pthread_self(), &policy, &param);
    printf("\t pthread_getschedparam %d - %d %d \n",s, policy,param.sched_priority);

    return NULL;
}
static void runPriorityTest()
{

    int s;
    pthread_t cfs_thread_idle;
    pthread_t edf_thread_1;
    pthread_t edf_thread_2;

    pthread_attr_t *attrp = NULL;

    s = pthread_create(&cfs_thread_idle, NULL, &thread_idle, NULL);
    if (s != 0)
        printf("Error pthread_create");

    sleep(5);

    s = pthread_create(&edf_thread_1, NULL, &thread_edf1, NULL);
    if (s != 0)
        printf("Error pthread_create");

    s = pthread_create(&edf_thread_2, NULL, &thread_edf2, NULL);
    if (s != 0)
        printf("Error pthread_create");

    s = pthread_join(edf_thread_2, NULL);
    if (s != 0)
        printf("Error pthread_join");

    s = pthread_join(edf_thread_1, NULL);
    if (s != 0)
        printf("Error pthread_join");

    s = pthread_join(cfs_thread_idle, NULL);
    if (s != 0)
        printf("Error pthread_join");

    return 0;
}

int main(int argc, char **argv) {
    // playEuclidGame();
    runLockerDoorTest();
    runProducerConsumerTest();
    runSleeperTest();
    runPriorityTest();
}
