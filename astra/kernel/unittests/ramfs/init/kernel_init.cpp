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


#include <stdint.h>

#include "arm/arm.h"
#include "arm/gic.h"
#include "fs/ramfs.h"
#include "config.h"

#include "kernel.h"

#include "system.h"

extern "C" void tzKernelSecondary();

#define assert(cond) if (!(cond)) { ATA_LogErr("%s:%d - Assertion failed", __PRETTY_FUNCTION__, __LINE__); while (true) { asm volatile("wfi":::);} }

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

    ATA_LogMsg("Success: file creation tests passed\n");
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

    ATA_LogMsg("Success: page write test passed\n");

    for (int i=4096; i<4196; i++)
        pageBuffer[i] = 0xae;
    fp->write(pageBuffer, 4196, 0);
    for (int i=0; i<4196; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 4196, 0);
    for (int i=0; i<4196; i++)
        assert(pageBuffer[i] == 0xae);

    ATA_LogMsg("Success: page + partial write test passed\n");

    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0xcd;
    fp->write(pageBuffer, 8192, 0);
    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 8192, 0);
    for (int i=0; i<4096; i++)
        assert(pageBuffer[i] == 0xcd);

    ATA_LogMsg("Success: 2-page write test passed\n");

    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0xcd;
    fp->write(pageBuffer, 8704, 0);
    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 8704, 0);
    for (int i=0; i<8704; i++)
        assert(pageBuffer[i] == 0xcd);

    ATA_LogMsg("Success: 2-page + partial write test passed\n");

    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0xef;
    fp->write(pageBuffer, 12288, 0);
    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 12288, 0);
    for (int i=0; i<12288; i++)
        assert(pageBuffer[i] == 0xef);

    ATA_LogMsg("Success: 3-page write test passed\n");

    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0xbf;
    fp->write(pageBuffer, 12799, 0);
    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0;

    fp->read(pageBuffer, 12799, 0);
    for (int i=0; i<12799; i++)
        assert(pageBuffer[i] == 0xbf);

    ATA_LogMsg("Success: 3-page + partial write test passed\n");

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

    ATA_LogMsg("Success: scatter write: span segment and page boundaries - passed\n");

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

    ATA_LogMsg("Success: scatter write: at segment and page boundaries - passed\n");

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

    ATA_LogMsg("Success: scatter write: at segment and page boundaries - passed\n");

    RamFS::File::destroy(fp);

    ATA_LogMsg("Success: scatter write tests passed\n");
}

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

    ATA_LogMsg("Success: directory resolution tests passed\n");

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

    root->resolvePath("/init/", &dir, &file);
    rc = dir->removeFile("kernel_init.cpp");
    assert(rc == 0);
     rc = dir->removeFile("system.cpp");
    assert(rc == 0);

    rc = root->removeDir("init");
    assert(rc == 0);

    ((RamFS::Directory *)root)->print();
    ATA_LogMsg("Success: file and directory deletion tests passed\n");
}

void tzKernelInit(const void *devTree) {

    printf("%s: Ramfs unit tests\n", __FUNCTION__);

    if (arm::smpCpuNum() != 0) {
        tzKernelSecondary();
        return;
    }

    System::init(devTree);

    root = System::root();
    ((RamFS::Directory *)root)->print();

    ramfsTests();

    fileCreationTests();
    fileWriteTests();
    fileScatterWriteTests();

    ATA_LogSuccess("All done\n");
}

void tzKernelSecondary() {

    System::initSecondaryCpu();

    ARCH_SPECIFIC_DISABLE_INTERRUPTS;

    while (true) {}
}

void kernelHalt(const char *reason) {
    ATA_LogErr("%s\n", reason);
    while (true) {}
}

extern "C" void __cxa_pure_virtual() {
    ATA_LogErr("Pure virtual function called !\n");
    kernelHalt("Pure virtual function call");
}
