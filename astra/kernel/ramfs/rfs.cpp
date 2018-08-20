/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "system.h"
#include "arm/spinlock.h"
#include "fs/ramfs.h"

#include "lib_printf.h"

struct CpioBinHdr {
    uint16_t magic;
    uint16_t dev;
    uint16_t ino;
    uint16_t mode;
    uint16_t uid;
    uint16_t gid;
    uint16_t nlinks;
    uint16_t rdev;
    uint16_t modTime[2];
    uint16_t nameLength;
    uint16_t fileSize[2];
};
#define CPIO_BIN_MAGIC 0x71C7

static void printCpioHdr(CpioBinHdr *curr) {
    printf("-----------------------\n");
    printf("Magic: 0x%x\n", curr->magic);
    printf("nameLength: %d\n", curr->nameLength);
    printf("file size: %d\n", ((unsigned int)curr->fileSize[0] << 16)|(curr->fileSize[1]));

    char *namePtr = (char *)curr + sizeof(CpioBinHdr);
    printf("file path: %s\n", namePtr);
}

static inline uint8_t *nextDirInPath(uint8_t *path) {
    char *startPos = (char *)path;
    while ((*path != '/') && (*path != 0))
        path++;

    if (*path == 0)
        return nullptr;

    *path = 0;
    return (uint8_t *)startPos;
}

struct MountPoint {
    TzMem::VirtAddr va;
    IDirectory *mount;
};

static SpinLock mountLock;
static tzutils::Vector<MountPoint> activeMounts;

void RamFS::init() {
    RamFS::File::init();
    RamFS::Directory::init();
    activeMounts.init();
    spinLockInit(&mountLock);
}

IDirectory *RamFS::load(TzMem::VirtAddr vaStart, TzMem::VirtAddr vaEnd, bool initRamFS) {
    SpinLocker locker(&mountLock);
    int err=0;
    for (int i=0; i<activeMounts.numElements(); i++) {
        if (activeMounts[i].va == vaStart)
            return activeMounts[i].mount;
    }

    IDirectory *root = RamFS::Directory::create(RamFS::UID, RamFS::GID, nullptr);

    uint8_t *currFile = (uint8_t *)vaStart;

    while (true) {
        CpioBinHdr *hdr = (CpioBinHdr *)currFile;
        size_t bytesWritten;
        if (hdr->magic != CPIO_BIN_MAGIC) {
            err_msg("Malformed FS archive\n");
            printCpioHdr(hdr);

            RamFS::Directory::destroy(root);
            root = nullptr;
            break;
        }

        uint8_t *filePath = currFile + sizeof(CpioBinHdr);
        int namePadding = (hdr->nameLength & 1);
        int nameLength = strlen((char *)filePath);

        if (!strcmp((char *)filePath, "TRAILER!!!"))
            break;

        uint8_t *fileStart = filePath + nameLength + namePadding + 1;
        uint32_t fileSize = ((uint32_t)hdr->fileSize[0] << 16)|hdr->fileSize[1];

        uint8_t *currPos = filePath;
        IDirectory *currDir = root;

        while (nextDirInPath(currPos) != nullptr) {
            char *dirName = (char *)currPos;
            int dirNameSize = strlen(dirName);

            if ((dirNameSize == 0) || (!strcmp(dirName, ".")) || (!strcmp(dirName, ".."))) {
                currPos += dirNameSize+1;
                continue;
            }

            IDirectory *nextDir = currDir->dir(dirName);
            if (nextDir == nullptr) {
                uint16_t mode = hdr->mode;
                int othersBits = mode & 0x7;
                int groupBits = (mode >> 3) & 0x7;
                int ownerBits = (mode >> 6) & 0x7;
                int perms = MAKE_PERMS(ownerBits, groupBits, othersBits);
                perms = MAKE_PERMS(PERMS_READ_WRITE_EXECUTE, PERMS_READ_WRITE_EXECUTE, PERMS_READ_WRITE_EXECUTE);
                //printf("dir %s (%s): mode %x perms %x \n",dirName,filePath, mode,perms);
                nextDir = RamFS::Directory::create(RamFS::UID, RamFS::GID, currDir, perms);
                err = currDir->addDir(dirName, nextDir);
                if(err != 0) {
                    printf("Unable to add dir err =%d\n",err);
                    return nullptr;
                }
            }

            currDir = nextDir;
            currPos += dirNameSize+1;
        }

        uint16_t mode = hdr->mode;
        int othersBits= mode & 0x7;
        int groupBits = (mode >> 3) & 0x7;
        int ownerBits = (mode >> 6) & 0x7;
        int perms = MAKE_PERMS(ownerBits, groupBits, othersBits);
        perms = MAKE_PERMS(PERMS_READ_ONLY_EXECUTE, PERMS_READ_ONLY_EXECUTE, PERMS_READ_ONLY_EXECUTE);
        //printf("file %s : mode %x perms %x \n",currPos, mode,perms);

        IFile *file = RamFS::File::create(RamFS::UID, RamFS::GID, perms); // fileStart, fileSize, perms);
        err = currDir->addFile((char *)currPos, file);
        if(err != 0) {
            printf("Unable to add file err =%d\n",err);
            return nullptr;
        }
        bytesWritten = file->write(fileStart, fileSize, 0);
        if(bytesWritten!=fileSize) {
            RamFS::Directory::destroy(root);
            return nullptr;
        }

        uint8_t *lastFile = currFile;

        currFile += sizeof(CpioBinHdr) + nameLength + namePadding + fileSize + 1;
        if ((uintptr_t)currFile & 0x1)
            currFile++;

        if (initRamFS)
            TzMem::freeInitRamFS(lastFile, currFile);
    }

    if (initRamFS)
        TzMem::freeInitRamFS(currFile, vaEnd);

    if (root) {
        MountPoint mp;
        mp.va = vaStart;
        mp.mount = root;
        activeMounts.pushBack(mp);
    }

    return root;
}
