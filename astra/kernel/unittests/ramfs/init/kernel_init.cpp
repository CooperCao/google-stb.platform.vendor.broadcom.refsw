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


#include <fs/ramfs.h>
#include <hwtimer.h>
#include <stdint.h>
#include <waitqueue.h>

#include "arm/arm.h"
#include "arm/gic.h"
#include "config.h"

#include "lib_printf.h"
#include "libfdt.h"
#include "parse_utils.h"

#include "kernel.h"
#include "tzmemory.h"
#include "pgtable.h"
#include "objalloc.h"
#include "interrupt.h"
#include "tztask.h"
#include "scheduler.h"

static uint8_t tzDevTree[MAX_DT_SIZE_BYTES];

#define assert(cond) if (!(cond)) { err_msg("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) { asm volatile("wfi":::);} }

static const int NumTestFiles = 32;
static IFile *files[NumTestFiles];
static IDirectory *root;

static char *numToStr(int num) {
    static char str[80];

    int pos = 0;
    do {
        int digit = num%10;
        str[pos] = '0' + digit;

        pos++;
        num /= 10;
    } while (num > 0);

    str[pos] = 0;
    int i=pos-1, j=0;
    while (i > j) {
        char t = str[i];
        str[i] = str[j];
        str[j] = t;
        i--;
        j++;
    }

    return str;
}

void fileCreationTests() {

    static const char *testStr = "Mary had a little lamb.";

    IDirectory *dir;
    IFile *file;
    int rc = root->resolvePath("process", &dir, &file);
    assert(rc == 0);
    assert(file == nullptr);
    assert(dir != nullptr);

    for (int i=0; i<NumTestFiles; i++) {
        files[i] = RamFS::File::create(dir->owner(), dir->group());
        assert(files[i] != nullptr);

        char fname[80];
        strcpy(fname, "testFile");
        strcat(fname, numToStr(i));

        rc = dir->addFile(fname, files[i]);
        assert(rc == 0);

        files[i]->write(testStr, strlen(testStr), 0LL);
    }

    static char rdData[64];
    for (int i=0; i<NumTestFiles; i++) {
        memset(rdData, 0, 64);
        size_t nb = files[i]->read(rdData, 64, 0);

        assert(nb == strlen(testStr));
        assert(!strcmp(rdData, testStr));
    }

    // ((RamFS::Directory *)dir)->print();

    for (int i=0; i<NumTestFiles; i++) {
        char fname[80];
        strcpy(fname, "testFile");
        strcat(fname, numToStr(i));

        dir->removeFile(fname);
        RamFS::File::destroy(files[i]);
    }

    success_msg("file creation tests passed\n");
}

void fileWriteTests() {
    static uint8_t pageBuffer[16*1024];
    for (int i=0; i<4096; i++)
        pageBuffer[i] = 0xae;

    IFile *fp = RamFS::File::create(0, 0);
    assert(fp != nullptr);

    fp->write(pageBuffer, 4096, 0);
    for (int i=0; i<4096; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 4096, 0);
    for (int i=0; i<4096; i++)
        assert(pageBuffer[i] == 0xae);

    success_msg("page write test passed\n");

    for (int i=4096; i<4196; i++)
        pageBuffer[i] = 0xae;
    fp->write(pageBuffer, 4196, 0);
    for (int i=0; i<4196; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 4196, 0);
    for (int i=0; i<4196; i++)
        assert(pageBuffer[i] == 0xae);

    success_msg("page + partial write test passed\n");

    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0xcd;
    fp->write(pageBuffer, 8192, 0);
    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 8192, 0);
    for (int i=0; i<4096; i++)
        assert(pageBuffer[i] == 0xcd);

    success_msg("2-page write test passed\n");

    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0xcd;
    fp->write(pageBuffer, 8704, 0);
    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 8704, 0);
    for (int i=0; i<8704; i++)
        assert(pageBuffer[i] == 0xcd);

    success_msg("2-page + partial write test passed\n");

    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0xef;
    fp->write(pageBuffer, 12288, 0);
    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 12288, 0);
    for (int i=0; i<12288; i++)
        assert(pageBuffer[i] == 0xef);

    success_msg("3-page write test passed\n");

    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0xbf;
    fp->write(pageBuffer, 12799, 0);
    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 12799, 0);
    for (int i=0; i<12799; i++)
        assert(pageBuffer[i] == 0xbf);

    success_msg("3-page + partial write test passed\n");

    RamFS::File::destroy(fp);
}

void fileScatterWriteTests() {

    IFile *fp = RamFS::File::create(0, 0);
    assert(fp != nullptr);

    static uint8_t buffer[4096];
    for (int i=0; i<4096; i++)
        buffer[i] = 0x12;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024)
        fp->write(buffer, 4096, offset-2000);

    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        fp->read(buffer, 4096, offset-2000);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x12)

        for (int i=0; i<4096; i++)
            buffer[i] = 0;
    }

    success_msg("scatter write: span segment and page boundaries - passed\n");

    for (int i=0; i<4096; i++)
        buffer[i] = 0x34;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024)
        fp->write(buffer, 4096, offset);

    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        fp->read(buffer, 4096, offset);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x34)

            for (int i=0; i<4096; i++)
                buffer[i] = 0;
    }

    success_msg("scatter write: at segment and page boundaries - passed\n");

    for (int i=0; i<4096; i++)
        buffer[i] = 0x56;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024)
        fp->write(buffer, 4096, offset-4096);

    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        fp->read(buffer, 4096, offset-4096);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x56)

            for (int i=0; i<4096; i++)
                buffer[i] = 0;
    }

    success_msg("scatter write: at segment and page boundaries - passed\n");

    RamFS::File::destroy(fp);

    success_msg("scatter write tests passed\n");
}

extern "C" unsigned long _initramfs_start, _initramfs_end;

void ramfsTests() {

    IDirectory *dir;
    IFile *file;
    int rc = root->resolvePath("process", &dir, &file);
    assert(rc == 0);
    assert(file == nullptr);
    assert(dir != nullptr);

    rc = root->resolvePath("/unittests/timer", &dir, &file);
    assert(rc == 0);
    assert(file == nullptr);
    assert(dir != nullptr);

    rc = root->resolvePath("unittests/timer/", &dir, &file);
    assert(rc == 0);
    assert(file == nullptr);
    assert(dir != nullptr);

    rc = root->resolvePath("/unittests/timer//init/", &dir, &file);
    assert(rc == 0);
    assert(file == nullptr);
    assert(dir != nullptr);

    rc = root->resolvePath("/unittests/timer//init/kernel_init.cpp", &dir, &file);
    assert(rc == 0);
    assert(file != nullptr);
    assert(dir == nullptr);

    rc = root->resolvePath("/unittests/timer//init/kernel_init.cpp/", &dir, &file);
    assert(rc < 0);
    assert(file == nullptr);
    assert(dir == nullptr);

    rc = root->resolvePath("/this/is/an///invalid/path", &dir, &file);
    assert(rc < 0);
    assert(file == nullptr);
    assert(dir == nullptr);

    success_msg("directory resolution tests passed\n");

    root->resolvePath("/unittests/timer//init/", &dir, &file);
    rc = dir->removeFile("kernel_init.cpp");
    assert(rc == 0);

    rc = dir->removeFile("system.cpp");
    assert(rc == 0);

    rc = root->resolvePath("/unittests/timer//init/kernel_init.cpp", &dir, &file);
    assert(rc < 0);
    assert(dir == nullptr);
    assert(file == nullptr);

    root->resolvePath("/unittests/timer", &dir, &file);
    rc = dir->removeDir("init");
    assert(rc == 0);

    root->resolvePath("/unittests/memory", &dir, &file);
    rc = dir->removeDir("init");
    assert(rc < 0);

    root->resolvePath("/boot/", &dir, &file);
    rc = dir->removeFile("cpuid.cpp");
    assert(rc == 0);
    rc = dir->removeFile("monitor_init.cpp");
    assert(rc == 0);
    rc = dir->removeFile("data_abort.cpp");
    assert(rc == 0);

    rc = root->removeDir("boot");
    assert(rc == 0);

    ((RamFS::Directory *)root)->print();
    success_msg("file and directory deletion tests passed\n");
}

void tzKernelInit(const void *devTree) {

    printf("%s: RAMFS unit tests\n", __FUNCTION__);

    // The bootstrap code has mapped device tree memory one-to-one VA and PA.
    int rc = fdt_check_header(devTree);
    if (rc) {
        err_msg("Corrupted device tree at %p.\n", devTree);
        return;
    }

    int dtSize = fdt_totalsize(devTree);
    if (dtSize > MAX_DT_SIZE_BYTES) {
        err_msg("Device tree is too large (size %d) . Increase MAX_DT_SIZE_BYTES\n", dtSize);
        return;
    }

    rc = fdt_move(devTree, tzDevTree, dtSize);
    if (rc) {
        err_msg("Device tree could not be relocated to tzDevTree: %s\n", fdt_strerror(rc));
        return;
    }

    TzMem::init(tzDevTree);
    PageTable::init(devTree);
    GIC::init(tzDevTree);
    Interrupt::init();
    TzTask::init();
    TzTimers::init();
    Scheduler::init();
    RamFS::init();

    /* Initialize the secure config register and setup
     * access permissions for non-secure world */
    register uint32_t scr, nsacr;
    asm volatile("mrc p15, 0, %[rt], c1, c1, 0" : [rt] "=r" (scr) : :);
    scr |= (1 << 8) | (1 << 5) |(1 << 2);//(SCR_HYP_CALL_ENABLE | SCR_NS_ALLOW_DATA_ABORT_MASK | SCR_FIQ_IN_MON_MODE);
    asm volatile("mcr p15, 0, %[rt], c1, c1, 0" : : [rt] "r" (scr) :);

    asm volatile("mrc p15, 0, %[rt], c1, c1, 2" : [rt] "=r" (nsacr) : :);
    nsacr |= ( (1 << 19) | ( 1<<11 ) |  ( 1<<10 ) |  ( 1<<18 )); // RFR, SMP CP10 and CP11.
    asm volatile("mcr p15, 0, %[rt], c1, c1, 2" : : [rt] "r" (nsacr) :);

    asm volatile ("cpsie aif" : : :);

    unsigned long *fsStart = &_initramfs_start;
    unsigned long *fsEnd = &_initramfs_end;

    printf("%s: ramfs start %p end %p\n", __FUNCTION__, fsStart, fsEnd);

    root = RamFS::load(fsStart);
    ((RamFS::Directory *)root)->print();

    success_msg("RamFS created\n");

    ramfsTests();

    fileCreationTests();
    fileWriteTests();
    fileScatterWriteTests();

    success_msg("All done\n");
}

void kernelHalt(const char *reason) {

    UNUSED(reason);
    while (true) {
        asm volatile("wfi":::);
    }
}

extern "C" void __cxa_pure_virtual() {
    err_msg("Pure virtual function called !\n");
    kernelHalt("Pure virtual function call");
}
