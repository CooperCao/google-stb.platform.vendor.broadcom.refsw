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

#ifndef TZIOC_TAPP_H
#define TZIOC_TAPP_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <time.h>

using namespace std;

#include "libtzioc_api.h"
#include "astra_test_msg.h"

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef LOGI
#define LOGD(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGW(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGE(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#define LOGI(format, ...) printf("%s: " format "\n", __FUNCTION__, ## __VA_ARGS__)
#endif

/* bit-mask of profiling levels */
#define PROFILING_LEVEL_0 (0x1 << 0)
#define PROFILING_LEVEL_1 (0x1 << 1)
#define PROFILING_LEVEL_2 (0x1 << 2)
#define PROFILING_LEVEL_3 (0x1 << 3)

/* bit-map of profiling levels */
#define ENABLE_PROFILING     0x00

/* ARM PMU based profiling */
#define ENABLE_ARM_PM_PROFILING 1

/* user app daemon class */
class AstraTapp {
public:
    /* defines and types */

public:
    /* constructor / destructor / copy op */
    AstraTapp() = delete;
    ~AstraTapp() = delete;
    AstraTapp& operator = (const AstraTapp&) = delete;

public:
    /* static methods */
    static void init();
    static void deinit();
    static void run();

public:
    /* public data */

private:
    /* private methods */
    static void msgProc(
        struct tzioc_msg_hdr *pHdr);

    static void helloProc(
        struct tzioc_msg_hdr *pHdr);

    static void memAllocProc(
        struct tzioc_msg_hdr *pHdr);

    static void mapPaddrProc(
        struct tzioc_msg_hdr *pHdr);

    static void mapPaddrsProc(
        struct tzioc_msg_hdr *pHdr);

#if ENABLE_PROFILING
#if ENABLE_ARM_PM_PROFILING
    static inline void profilingInit() {
        /* Note: divider is enabled */
        asm volatile("mcr p15, 0, %[val], c9, c12, 0" : : [val] "r" (0x1f) :);
        asm volatile("mcr p15, 0, %[val], c9, c12, 1" : : [val] "r" (0x800000ff) :);
        asm volatile("mcr p15, 0, %[val], c9, c12, 3" : : [val] "r" (0x800000ff) :);
    }

    static inline void profilingStart() {
        register uint32_t pmccntr;
        asm volatile("mrc p15, 0, %[val], c9, c13, 0" : [val] "=r" (pmccntr) : :);
        pmccntr0 = pmccntr;
    }

    static inline void profilingStop() {
        register uint32_t pmccntr;
        asm volatile("mrc p15, 0, %[val], c9, c13, 0" : [val] "=r" (pmccntr) : :);
        pmccntr1 = pmccntr;
    }

    static inline void profilingPrint() {
        /* Note: divider is enabled */
        printf("time elapsed %d cycle", (pmccntr1 - pmccntr0) * 64);
    }
#else
    static inline void profilingInit() {
    }

    static inline void profilingStart() {
        clock_gettime(CLOCK_REALTIME, &tv0);
    }

    static inline void profilingStop() {
        clock_gettime(CLOCK_REALTIME, &tv1);
    }

    static inline void profilingPrint() {
        printf("time elapsed %ld ns",
               (tv1.tv_sec >= tv0.tv_sec) ?
               (tv1.tv_nsec - tv0.tv_nsec) :
               ((tv1.tv_sec - tv0.tv_sec) * 1000000000 + tv1.tv_nsec - tv0.tv_nsec));
    }
#endif
#endif

private:
    /* private data */
    static tzioc_client_handle hClient;
    static int msgQ;
    static uint8_t clientId;

#if ENABLE_PROFILING
#if ENABLE_ARM_PM_PROFILING
    static uint32_t pmccntr0, pmccntr1;
#else
    static struct timespec tv0, tv1;
#endif
#endif
};

#endif /* TZIOC_TAPP_H */
