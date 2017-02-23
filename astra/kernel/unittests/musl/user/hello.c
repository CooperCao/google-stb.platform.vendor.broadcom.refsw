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
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define assert(cond) if (!(cond)) { printf("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (1) {} }

#define NumProcs  4
static char pages[NumProcs * 4096];

void test_fork() {

    char *test = malloc(32);
    printf("Malloc(32) returned %p\n", test);
    for (int i=0; i<32; i++)
        test[i] = 'a';


    char *mmapTest = (char *)mmap(NULL, 2*1024*1024, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    assert(mmapTest != (char *)MAP_FAILED);
    for (int i=0; i<10; i++)
        mmapTest[i*100] = 'b';

    /*
     * Fork N processes and wait.
     */
    for (int i=0; i<NumProcs; i++) {
        pid_t rc = fork();
        if (rc == 0) {
            pid_t pid = getpid();
            printf("Child process %d\n", pid);

#if 0
            /* Test Segfault */
            if (pid == 5) {
                // cause a segfault
                int *badAddr = (int *)0x4;
                *badAddr = 1;
            }
#endif

            char *myPage = pages + (i*4096);
            memset(myPage, (char)pid, 4096);

            struct timespec now;
            //char strTime[80];
            //memset(strTime, 0, 80);
            clock_gettime(CLOCK_REALTIME, &now);
            //ctime_r(&now.tv_sec, strTime);

            printf("[Child %d] Time now %d:%d. Sleep for %d seconds\n", pid, (int)now.tv_sec, (int)now.tv_nsec, pid*2);

            struct timespec sleepFor;
            sleepFor.tv_sec = pid*2;
            sleepFor.tv_nsec = 0;
            nanosleep(&sleepFor, NULL);

            // Verify that malloced memory from parent is accessible
            // in child process.
            for (int i=0; i<32; i++)
                assert(test[i] == 'a');

            assert(mmapTest[i*100] == 'b');

            mmapTest[i*100 + 1] = '0'+i;
            exit(i);
        }
        else
            printf("forked process pid %d\n", rc);
    }
    for (int i=0; i<NumProcs; i++) {
        int status = 0;
        pid_t pid = wait(&status);
        printf("Child %d exited with rv %d\n", pid, WEXITSTATUS(status));

        assert(mmapTest[i*100+1] == ('0'+i));
    }

    int status;
    pid_t pid = wait(&status);
    if (pid > 0) {
        printf("ERROR: wait returned success on no children !\n");
    }
    else
        printf("SUCCESS: wait returned %d errno %d\n", pid, errno);


    /*
     * Fork N processes and waitpid for each one of them
     */
    pid_t childPids[NumProcs];
    for (int i=0; i<NumProcs; i++) {
        pid_t rc = fork();
        if (rc == 0) {
            pid_t pid = getpid();
            struct timespec now;
            struct timespec sleepFor;
            //char strTime[80];

            //printf("Child process %d\n", pid);

            char *myPage = pages + (i*4096);
            memset(myPage, (char)pid, 4096);

            //memset(strTime, 0, 80);
            clock_gettime(CLOCK_REALTIME, &now);
            //ctime_r(&now.tv_sec, strTime);

            printf("[Child %d] Time now %d:%d. Sleep for %d seconds\n", pid, (int)now.tv_sec, (int)now.tv_nsec, pid*2);
            sleepFor.tv_sec = pid*2;
            sleepFor.tv_nsec = 0;
            nanosleep(&sleepFor, NULL);

            exit(i);
        }
        else {
            childPids[i] = rc;
        }
    }
    for (int i=0; i<NumProcs; i++) {
        int status = 0;
        pid_t rc = waitpid(childPids[i], &status, 0);
        if (rc != childPids[i]) {
            printf("waitpid for child %d pid %d failed\n", i, childPids[i]);
        }
        else {
            if (WIFEXITED(status))
                printf("child %d exited with return code %d\n", i, WEXITSTATUS(status));
            else
                printf("child %d exited in an unexpected fashion.\n", i);

            assert(WIFEXITED(status) && (WEXITSTATUS(status) == i));
        }
    }

    printf("Fork test success\n");
}

void test_reparent() {
    pid_t rc = fork();
    if (rc == 0) {
        /*
         * Create N children and exit before all of them.
         */
        for (int i=0; i<NumProcs; i++) {
            pid_t child = fork();
            if (child == 0) {
                sleep(5);
                exit(i);
            }
        }
        exit(10);
    }
    else {
        /*
         * The child created N subsequent children and exited before all of them.
         * This process should now have N+1 children.
         */
        int nc = 0;
        while (1) {
            int status;
            pid_t child = wait(&status);
            if (child == -1)
                break;

            nc++;
            printf("[Reparent test] Child %d pid %d exited with return code %d\n", nc, child, WEXITSTATUS(status));
        }

        assert(nc == NumProcs+1);
    }

    printf("Reparent test success\n");
}

void test_exec() {

    pid_t rc = fork();
    if (rc == 0) {
        char *argv[] = {"hello2.elf", "mary", "had a", "little lamb", NULL};
        printf("In child process. Now calling execv\n");
        execv("hello2.elf", argv);
    }
    else {
        int status;
        wait(&status);
        printf("[EXEC test] child exited with return code %d\n", WEXITSTATUS(status));
    }

    printf("Exec test success\n");
}

extern void mq_test();

int main(int argc, char **argv) {

    struct timespec resolution;
    struct timespec now;
    int rc;

    printf("Hello world\n");

    mq_test();

    resolution.tv_sec = 0;
    resolution.tv_nsec = 0;
    rc = clock_getres(CLOCK_REALTIME, &resolution);
    if (rc < 0) {
        perror("clock_getres failed: ");
        return -1;
    }
    printf("CLOCK_REALTIME resolution: %d %d\n", (int)resolution.tv_sec, (int)resolution.tv_nsec);

    clock_gettime(CLOCK_REALTIME, &now);
    printf("time now: %d:%d\n", (int)now.tv_sec, (int)now.tv_nsec);

    printf("sleep for 5 seconds\n");
    sleep(5);

    clock_gettime(CLOCK_REALTIME, &now);
    printf("time now: %d:%d\n", (int)now.tv_sec, (int)now.tv_nsec);

    test_fork();
    test_reparent();
    test_exec();

    printf("All tests completed successfully.\n");
    return 0;
}
