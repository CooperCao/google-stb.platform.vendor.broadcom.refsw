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

/*
 * fs_tests.c
 *
 *  Created on: Feb 24, 2015
 *      Author: gambhire
 */

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>

#define assert(cond) if (!(cond)) { printf("%s:%d - Assertion failed\n", __PRETTY_FUNCTION__, __LINE__); while (true) { } }

static const int NumTestFiles = 8;

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

static void fileCreationTests() {

    static const char *testStr = "Mary had a little lamb.";
    int fds[NumTestFiles];

    for (int i=0; i<NumTestFiles; i++) {
        char fname[80];
        strcpy(fname, "/unittests/fs_userspace/testFile");
        strcat(fname, numToStr(i));

        fds[i] = open(fname, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if (fds[i] <= 0) {
            perror("file creation failure: ");
            continue;
        }

        int nw = write(fds[i], testStr, strlen(testStr)+1);
        assert(nw == strlen(testStr)+1);

    }

    printf("Files created and written\n");

    for (int i=0; i<NumTestFiles; i++) {
        static char rdBuf[80];
        if (fds[i] <= 0)
            continue;

        off_t rc = lseek(fds[i], 0, SEEK_SET);
        assert(rc == 0);

        int nr = read(fds[i], rdBuf, 80);
        printf("FD %d: read %s\n", fds[i], rdBuf);

        assert(nr == strlen(testStr)+1);
        assert(!strcmp(rdBuf, testStr));


    }

    for (int i=0; i<NumTestFiles; i++) {
        if (fds[i] <= 0)
            continue;
        close(fds[i]);
    }

    printf("file creation tests passed\n");

      /* Test the file open/create & append on the same file name before deleting it */
        for (int i=0; i<NumTestFiles; i++) {
        char fname[80];
        strcpy(fname, "/unittests/fs_userspace/testFile");
        strcat(fname, numToStr(i));

        fds[i] = open(fname, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if (fds[i] <= 0) {
            perror("file creation failure: ");
            continue;
        }

        int nw = write(fds[i], testStr, strlen(testStr)+1);
        assert(nw == strlen(testStr)+1);

    }

    printf("Files re-opened and written\n");

    for (int i=0; i<NumTestFiles; i++) {
        static char rdBuf[80];
        if (fds[i] <= 0)
            continue;

        off_t rc = lseek(fds[i], 0, SEEK_SET);
        assert(rc == 0);

        int nr = read(fds[i], rdBuf, 80);
        printf("FD %d: read %s\n", fds[i], rdBuf);

        assert(nr == strlen(testStr)+1);
        assert(!strcmp(rdBuf, testStr));


    }

    for (int i=0; i<NumTestFiles; i++) {
        if (fds[i] <= 0)
            continue;

        close(fds[i]);

        char fname[80];
        strcpy(fname, "/unittests/fs_userspace/testFile");
        strcat(fname, numToStr(i));

        int rc = unlink(fname);
        assert(rc == 0);
    }

    printf("file re-open tests passed\n");

}

static void fileScatterWriteTests() {

    int fd = open("/unittests/fs_userspace/scatter_tests", O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
    assert(fd > 0);

    static uint8_t buffer[4096];
    for (int i=0; i<4096; i++)
        buffer[i] = 0x12;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset - 2000;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        write(fd, buffer, 4096);
    }


    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset - 2000;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        read(fd, buffer, 4096);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x12)

        for (int i=0; i<4096; i++)
            buffer[i] = 0;
    }

    printf("scatter write: span segment and page boundaries - passed\n");

    for (int i=0; i<4096; i++)
        buffer[i] = 0x34;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        write(fd, buffer, 4096);
    }

    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        read(fd, buffer, 4096);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x34)

            for (int i=0; i<4096; i++)
                buffer[i] = 0;
    }

    printf("scatter write: at segment and page boundaries - passed\n");

    for (int i=0; i<4096; i++)
        buffer[i] = 0x56;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset-4096;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        write(fd, buffer, 4096);
    }

    for (int i=0; i<4096; i++)
        buffer[i] = 0;

    for (int offset=4096; offset<8*1024*1024; offset+=1024*1024) {
        off_t pos = offset-4096;
        off_t rc = lseek(fd, pos, SEEK_SET);
        assert(rc == pos);

        read(fd, buffer, 4096);
        for (int i=0; i<4096; i++)
            assert(buffer[i] == 0x56)

        for (int i=0; i<4096; i++)
            buffer[i] = 0;
    }

    close(fd);
    printf("scatter write: at segment and page boundaries - passed\n");

    printf("scatter write tests passed\n");
}


static void fileWriteTests() {
    static uint8_t pageBuffer[16*1024];
    for (int i=0; i<4096; i++)
        pageBuffer[i] = 0xae;

    int fd = open("/unittests/fs_userspace/write_tests", O_CREAT|O_TRUNC|O_RDWR, S_IRUSR|S_IWUSR);
    assert(fd > 0);

    int rc = write(fd, pageBuffer, 4096);
    assert(rc == 4096);
    for (int i=0; i<4096; i++)
        pageBuffer[i] = 0;

    off_t rco = lseek(fd, 0, SEEK_SET);
    assert(rco == 0);

    rc = read(fd, pageBuffer, 4096);
    assert(rc == 4096);
    for (int i=0; i<4096; i++)
        assert(pageBuffer[i] == 0xae);

    printf("page write test passed\n");

    for (int i=4096; i<4196; i++)
        pageBuffer[i] = 0xae;
    rc = write(fd, pageBuffer, 4196);
    assert(rc == 4196);
    for (int i=0; i<4196; i++)
        pageBuffer[i] = 0;

    off_t pos = -4196;
    lseek(fd, pos, SEEK_CUR);
    rc = read(fd, pageBuffer, 4196);
    assert(rc == 4196);
    for (int i=0; i<4196; i++)
        assert(pageBuffer[i] == 0xae);

    printf("page + partial write test passed\n");

    lseek(fd, 0, SEEK_SET);
    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0xcd;
    rc = write(fd, pageBuffer, 8192);
    assert(rc == 8192);
    for (int i=0; i<8192; i++)
        pageBuffer[i] = 0;

    pos = -8192;
    rco = lseek(fd, pos, SEEK_CUR);
    assert(rco == 0);
    rc = read(fd, pageBuffer, 8192);
    assert(rc == 8192);
    for (int i=0; i<4096; i++)
        assert(pageBuffer[i] == 0xcd);

    printf("2-page write test passed\n");

    lseek(fd, 0, SEEK_SET);
    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0xcd;
    write(fd, pageBuffer, 8704);
    for (int i=0; i<8704; i++)
        pageBuffer[i] = 0;

    lseek(fd, 0, SEEK_SET);
    read(fd, pageBuffer, 8704);
    for (int i=0; i<8704; i++)
        assert(pageBuffer[i] == 0xcd);

    printf("2-page + partial write test passed\n");

    lseek(fd, 0, SEEK_END);
    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0xef;
    write(fd, pageBuffer, 12288);
    for (int i=0; i<12288; i++)
        pageBuffer[i] = 0;

    lseek(fd, -12288, SEEK_END);
    read(fd, pageBuffer, 12288);
    for (int i=0; i<12288; i++)
        assert(pageBuffer[i] == 0xef);

    printf("3-page write test passed\n");

    lseek(fd, -12799, SEEK_END);
    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0xbf;
    write(fd, pageBuffer, 12799);
    for (int i=0; i<12799; i++)
        pageBuffer[i] = 0;

    lseek(fd, -12799, SEEK_END);
    read(fd, pageBuffer, 12799);
    for (int i=0; i<12799; i++)
        assert(pageBuffer[i] == 0xbf);

    printf("3-page + partial write test passed\n");

    close(fd);
}

static void ramfsTests() {
    int fd = open("/process", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/timer", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/timer/", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/timer//init", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/timer//init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/this/is/an///invalid/path", O_RDONLY);
    assert(fd < 0); perror("Invalid path test 2: ");

    printf("directory resolution tests passed\n");
}

static void linkTests() {
    int rc = link("/unittests/musl_userspace", "/unittests/libc_userspace");
    assert(rc == 0);

    int fd = open("/unittests/libc_userspace/init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    rc = unlink("/unittests/libc_userspace/init/system.cpp");
    assert(rc == 0);

    fd = open("/unittests/musl_userspace/init/system.cpp", O_RDONLY);
    assert(fd < 0);

    rc = unlink("/unittests/libc_userspace");
    assert(rc == 0);

    fd = open("/unittests/libc_userspace/init/kernel_init.cpp", O_RDONLY);
    assert(fd < 0);

    fd = open("/unittests/musl_userspace/init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    printf("Link tests passed\n");
}

static void cwdTests() {
    int rc = chdir("/unittests/fs_userspace/init");
    assert(rc == 0);

    int fd = open("system.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("creationtest", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    assert(fd > 0);
    const char *testStr = "This is a newly created file using a relative path";
    int nw = write(fd, testStr, strlen(testStr)+1);
    assert(nw == (strlen(testStr)+1));
    close(fd);

    rc = chdir("/this/is/not/a/valid/path");
    assert(rc != 0); perror("bad path chdir test: ");

    printf("cwdTests passed\n");
}

static void renameTests() {
    // 1. Rename file
    int rc = rename("/unittests/memory/init/kernel_init.cpp", "/process/kernel_init.cpp");
    assert(rc == 0);

    int fd = open("/process/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/memory/init/kernel_init.cpp", O_RDONLY);
    assert(fd <= 0); perror("renameTests - attempted open on old name: ");

    rc = rename("/process/kernel_init.cpp","/unittests/memory/init/kernel_init.cpp");
    assert(rc == 0);

    fd = open("/unittests/memory/init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    // 2. Rename dir
    rc = rename("/unittests/memory/init", "/process/init");
    assert(rc == 0);

    fd = open("/process/init/kernel_init.cpp", O_RDONLY);
    assert(fd >= 0);
    close(fd);

    fd = open("/unittests/memory/init/kernel_init.cpp", O_RDONLY);
    assert(fd <= 0); perror("renameTests - attempted open on old name: ");

    rc = rename("/process/init", "/unittests/memory/init");
    assert(rc == 0);

    fd = open("/unittests/memory/init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    // 3. Rename a file to an existing directory with elements. This should fail.
    rc = rename("/unittests/memory/init/kernel_init.cpp", "/process");
    assert(rc != 0); printf("renameTests - attempted to rename file to non-empty dir: errno %d\n", errno);

    // 4. Rename a file to an existing file. This should succeed.
    rc = rename("/unittests/memory/init/kernel_init.cpp", "/process/elfimage.cpp");
    assert(rc == 0);

    printf("opening renamed file\n");
    fd = open("/process/elfimage.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    printf("opening old name\n");
    fd = open("/unittests/memory/init/kernel.cpp", O_RDONLY);
    assert(fd <= 0);
    close(fd);

    //5. Rename a directory to an existing file. This should succeed.
    rc = rename("/unittests/timer", "/process/elfimage.cpp");
    assert(rc == 0);

    fd = open("/process/elfimage.cpp/init/system.cpp", O_RDONLY);
    assert(fd > 0);
    close(fd);

    fd = open("/unittests/timer/init/system.cpp", O_RDONLY);
    assert(fd <= 0);
    close(fd);

    printf("rename tests passed\n");
}

static void dirCreationTests() {

    int rc = mkdir("/dirTest", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    assert(rc == 0);

    rc = mkdir("/dirTest/innerDir", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    assert(rc == 0);

    rc = mkdir("/dirTest/leafDir", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    assert(rc == 0);

    int fd = open("/dirTest/innerDir/testFile", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);
    assert(fd > 0);
    const char *testStr = "this is a test";
    write(fd, testStr, strlen(testStr)+1);
    struct stat statbuf;
    rc = fstat(fd, &statbuf);
    assert(rc == 0);
    close(fd);

    rc = rmdir("/dirTest/leafDir");
    assert(rc == 0);

    rc = rmdir("/dirTest/innerDir");
    assert(rc != 0);

    printf("dir creation and deletion tests passed\n");
}

static void statTests() {

    struct stat statBuf;

    int rc = stat("/unittests/memory/init/kernel_init.cpp", &statBuf);
    assert(rc == 0);
    printf("stat:\n");
    printf("\tst_dev: %lld\n", statBuf.st_dev);
    printf("\tst_mode: 0x%x\n", statBuf.st_mode);
    printf("\tst_nlink: %d\n", statBuf.st_nlink);
    printf("\tst_uid: %d\n", statBuf.st_uid);
    printf("\tst_gid: %d\n", statBuf.st_gid);
    printf("\tst_rdev: %lld\n", statBuf.st_rdev);
    printf("\tst_size: %lld\n", statBuf.st_size);
    printf("\tst_blksize: %d\n", statBuf.st_blksize);
    printf("\tst_blocks: %lld\n", statBuf.st_blocks);
    printf("\tst_atim %d: %d\n", statBuf.st_atim.tv_sec, statBuf.st_atim.tv_nsec);
    printf("\tst_mtim %d: %d\n", statBuf.st_mtim.tv_sec, statBuf.st_mtim.tv_nsec);
    printf("\tst_ctim %d: %d\n", statBuf.st_ctim.tv_sec, statBuf.st_ctim.tv_nsec);
    printf("\tst_ino %lld\n", statBuf.st_ino);

    int fd = open("/unittests/memory/init/kernel_init.cpp", O_RDONLY);
    assert(fd > 0);

    size_t len = (size_t)statBuf.st_size;
    printf("Attempt mmap: size %lld len %d \n", statBuf.st_size, len);
    void *mappedAddr = mmap(NULL, len, PROT_READ, MAP_SHARED, fd, 0);
    assert(mappedAddr != NULL);

#if 0
    printf("mapped file to %p\n", mappedAddr);
    uint8_t *curr = (uint8_t *)mappedAddr;
    for (int i=0; i<len; i++)
        printf("%c", curr[i]);
    printf("\n");
#endif

    rc = stat("/invalid/file", &statBuf);
    assert(rc != 0);


}

int main(int argc, char **argv) {
    int i = 0;
    i++;

    printf("hello from fs_tests: stackAddr %p i=%d \n", &i, i);

    statTests();
    dirCreationTests();
    linkTests();
    ramfsTests();
    fileCreationTests();
    fileWriteTests();
    fileScatterWriteTests();
    cwdTests();
    renameTests();

    return 0;
}
