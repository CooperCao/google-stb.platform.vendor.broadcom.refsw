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

#ifndef FS_RAMFS_H_
#define FS_RAMFS_H_

#include "system.h"
#include "utils/vector.h"
#include "fs/fs.h"
#include "lib_string.h"
#include "crypto/csprng.h"

namespace RamFS {

    static const unsigned int MaxFileSize = 0xffffffff;
    static const unsigned int MaxFileNameLength = 256;

    static const int NumL1BlockEntries = 4;
    static const int L1RangeSize = 1024*1024*1024;

    static const int NumL2BlockEntries = 512;
    static const int L2RangeSize = 2*1024*1024;

    static const int NumL3BlockEntries = 512;
    static const int L3RangeSize = 4096;

    void init();
    IDirectory *load(TzMem::VirtAddr memAddr);

    class File : public IFile {
    public:
        static void init();

        static IFile *create(uint16_t uid, uint16_t gid, uint32_t perms = DEFAULT_PERMS_UMASK);

        static void destroy(IFile *);

        static void open(File *);
        static void close(File *);

    public:
        uint32_t permissions() const { return perms; }
        int changePermissions(uint32_t newPerms);

        uint16_t owner() const { return uid; }
        int changeOwner(uint16_t newOwner);

        uint16_t group() const { return gid; }
        int changeGroup(uint16_t newGroup);

        uint64_t size() const { return fileSize; }
        long numBlocks() const;
        short numLinks() const { return linkCount; }

        void erase();

        virtual size_t read(const void *data, const size_t numBytes, const uint64_t offset);
        virtual ssize_t readv(const struct iovec *iov, int iovcnt, const uint64_t offset);
        virtual size_t write(const void *data, const size_t numBytes, const uint64_t offset);
        virtual ssize_t writev(const struct iovec *iov, int iovcnt, const uint64_t offset);

        void addWatcher(short pollEvent, short *pollResult, EventQueue *);

        void dup() { addRef(); }
        IFile *clone();

        bool isExecutable() { return checkPermissions(PERMS_EXECUTE_BIT); }

        void accessTimes(AccessTimes *times);

        int mmap(void *addr, void **mappedAddr, size_t length, int prot, int flags, uint64_t offset);

        void freeze();

    protected:
        File(uint16_t uid, uint16_t gid, uint32_t perms = DEFAULT_PERMS_UMASK);

        ~File();

        void *operator new(size_t sz);
        void operator delete(void *);

    private:
        inline int l1Index(uint64_t pos) const {
            return (pos >> 30) & 0x3;
        }

        inline int l2Index(uint64_t pos) const {
            return (pos >> 21) & 0x1ff;
        }

        inline int l3Index(uint64_t pos) const {
            return (pos >> 12) & 0x1ff;
        }

        inline int pageIndex(uint64_t pos) const {
            return pos & 0xfff;
        }

        size_t fetchFromBlockTable(uint8_t *data, uint64_t startAddr, uint64_t stopAddr);
        size_t writeToBlockTable(uint8_t *data, uint64_t startAddr, uint64_t stopAddr);

        uint8_t *offsetToPage(uint64_t offset);

        TzMem::VirtAddr rfsAllocPage();
        void rfsFreePage(TzMem::VirtAddr );

        bool checkPermissions(int permsBit);

    private:

        TzMem::VirtAddr l1BlockTable[NumL1BlockEntries];

    protected:
        uint64_t fileSize;
        uint32_t perms;
        uint16_t uid;
        uint16_t gid;

        int linkCount;
        int numRefs;

        AccessTimes times;
        spinlock_t lock;

    protected:
        int addRef();
        int release();

        File(const File& ) = delete;
        File& operator = (const File& ) = delete;

    };

    class Directory : public IDirectory {
    public:
        static void init();

        static IDirectory *create(uint16_t uid, uint16_t gid, IDirectory *parent, uint32_t perms = DEFAULT_DIR_UMASK);
        static void destroy(IDirectory *);

        static void open(Directory *);
        static void close(Directory *);

    public:
        uint32_t permissions() const { return perms; }
        int changePermissions(uint32_t newPerms);

        uint16_t owner() const { return uid; }
        int changeOwner(uint16_t newOwner);

        uint16_t group() const { return gid; }
        int changeGroup(uint16_t newGroup);

        void dup() { addRef(); }
        IDirectory *clone();

        int addFile(const char *fileName, IFile *file);
        int removeFile(const char *fileName);

        int addDir(const char *dirName, IDirectory *dir);
        int removeDir(const char *dirName);

        int readDir(const void *data, const size_t size);
        void openDir();
        void closeDir();

        int mount(const char *dirName, IDirectory *dir);
        int unmount(const char *dirName);

        IFile *file(const char *fileName);
        IDirectory *dir(const char *dirName);

        int resolvePath(const char *path, IDirectory **dir, IFile **file, IDirectory **parentDir=nullptr);

        bool isEmpty();

        short numLinks() const { return linkCount; }

        void accessTimes(AccessTimes *times);

        void print(int level=0);

    public:
        enum EntryType {
            FileEntry,
            DirEntry
        };

        struct Entry {
            char name[MaxFileNameLength];
            uint32_t nameHash;
            EntryType type;
            union {
                RamFS::File *file;
                RamFS::Directory *dir;
            };
            Entry *leftChild;
            Entry *rightChild;

            Entry *prevEntry;
            Entry *nextEntry;
        };

        struct dirent
        {
            ino_t d_ino;
            off_t d_off;
            unsigned short d_reclen;
            unsigned char d_type;
            char d_name[256];
        };
        uint32_t perms;
        uint16_t uid;
        uint16_t gid;
        Entry *firstEntry; // Binary search tree of sentries ordered by nameHash.
        Entry *shadowed; // Linked list of entries.
        Entry *readEntry;
        int linkCount;
        int numEntries;
        int numRefs;

        AccessTimes times;
        spinlock_t lock;

    private:
        Directory(uint16_t uid, uint16_t gid, IDirectory *parent = nullptr, uint32_t permissions = DEFAULT_DIR_UMASK);
        ~Directory();

        void *operator new(size_t sz);
        void operator delete(void *);

        bool checkPermissions(int permsBit);

        bool addEntry(Entry *);
        Entry *removeEntry(const char *entryName);
        Entry *fetchEntry(const char *entryName);

        void shadowListAdd(Entry *entry);
        Entry *shadowListRemove(const char *entryName);

        int addRef();
        int release();
    };


    class RandNumGen : public File {
    public:
        static IFile *create();

        virtual size_t read(const void *data, const size_t numBytes, const uint64_t offset) {
            UNUSED(offset);
            return CryptoPRNG::instance()->read((uint8_t *)data, numBytes);
        }

        virtual ssize_t readv(const struct iovec *iov, int iovcnt, const uint64_t offset) {
            UNUSED(offset);
            int nr = 0;
            CryptoPRNG *cprng = CryptoPRNG::instance();
            for (int i=0; i<iovcnt; i++)
                nr += cprng->read((uint8_t *)iov[i].iov_base, iov[i].iov_len);

            return nr;
        }

        virtual size_t write(const void *data, const size_t numBytes, const uint64_t offset) {
            UNUSED(data);
            UNUSED(numBytes);
            UNUSED(offset);
            return -EACCES;
        }

    private:
        RandNumGen() : File(System::UID, System::GID, MAKE_PERMS(PERMS_READ_ONLY, PERMS_READ_ONLY, PERMS_READ_ONLY)){}
        ~RandNumGen() {}

    private:
        void *operator new(size_t sz, void *where);
    };
}


#endif /* RAMFS_RAMFS_H_ */
