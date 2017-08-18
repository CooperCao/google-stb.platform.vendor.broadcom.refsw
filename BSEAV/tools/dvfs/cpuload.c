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

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sched.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>

struct context {
    unsigned load;
    unsigned cpu;
    unsigned timeout;
    unsigned ramp;
} g_context;

static void print_usage(void)
{
    printf("\n");
    printf("Usage: cpuload OPTIONS\n");
    printf("\n");
    printf("OPTIONS:\n");
    printf("  -h or --help   Print help.\n");
    printf("  -l load        CPU load percentage. Default is 100\n");
    printf("  -n cpu         Number of CPUs to load. Default is 1\n");
    printf("  -t timeout     Duration of test in seconds. Default is 60\n");
    printf("  -r 0|1         Set to 1 to graudally ramp CPU Load to max specified by -l option. Default 0\n");
}

int parse_cmdline_args(int argc, char **argv)
{
    int curarg = 0;

    while (++curarg < argc) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-l") && curarg+1 < argc) {
            unsigned load = atoi(argv[++curarg]);
            if (load > 0 && load <=100)
                g_context.load = load;
        }
        else if (!strcmp(argv[curarg], "-n") && curarg+1 < argc) {
            unsigned num_cpu = atoi(argv[++curarg]);
            if (num_cpu > 0 && num_cpu < g_context.cpu)
                g_context.cpu = num_cpu;
        }
        else if (!strcmp(argv[curarg], "-t") && curarg+1 < argc) {
            g_context.timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-r") && curarg+1 < argc) {
            g_context.ramp = atoi(argv[++curarg]);
        }
    }
}

int run_on_cpu(unsigned cpu)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu, &cpu_set);
    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpu_set);
}

int load_cpu()
{
    unsigned load = g_context.load;
    unsigned timeout = g_context.timeout;
    unsigned ramp = g_context.ramp;
    unsigned step, elapsed_time=0;
    struct timeval begin, start, end;

    if(ramp) {
        step = timeout/((load)/5);
        load = 0;
        elapsed_time += step;
    }

    gettimeofday(&start, NULL);
    begin = start;

    while(1) {
        unsigned diff;
        gettimeofday(&end, NULL);

        if(ramp) {
            if(end.tv_sec-begin.tv_sec > elapsed_time) {
               if(load < 100) {
                   load += 5;
                   elapsed_time += step;
                }
            }
        }

        diff = (end.tv_usec>=start.tv_usec)?end.tv_usec-start.tv_usec:1000000-start.tv_usec+end.tv_usec;
        if(end.tv_sec-begin.tv_sec > timeout)
            break;

        if(load < 100 && diff>=load*100) {
            usleep((100-load)*100);
            gettimeofday(&start, NULL);
        }
        /* printf("Elapsed time %d\n", (end.tv_usec>start.tv_usec)?end.tv_usec-start.tv_usec:1000000-start.tv_usec+end.tv_usec); */
    }

    return 0;
}

int main(int argc, char *argv[])
{
    unsigned i;
    int status, pid;
    cpu_set_t allcpus;

    CPU_ZERO(&allcpus);
    sched_getaffinity(0, sizeof(cpu_set_t), &allcpus);

    g_context.load = 100;
    g_context.cpu = CPU_COUNT(&allcpus);
    g_context.timeout = 60;
    g_context.ramp = 0;

    parse_cmdline_args(argc, argv);

    if (g_context.timeout == -1)
        g_context.ramp = 0;

    for (i=0; i<g_context.cpu; i++) {
        switch (pid = fork ()) {
            case 0:            /* child */
                run_on_cpu(i);
                exit (load_cpu ());
            case -1:           /* error */
                err (stderr, "fork failed: %s\n", strerror (errno));
                break;
            default:           /* parent */
                break;
        }
    }

    for (i=0; i<g_context.cpu; i++) {
        pid = wait (&status);
    }

    return 0;
}
