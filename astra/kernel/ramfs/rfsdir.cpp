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
#include "system.h"
#include "fs/ramfs.h"
#include "objalloc.h"
#include "tztask.h"
#include "lib_string.h"

static ObjCacheAllocator<RamFS::Directory> dirAllocator;
static ObjCacheAllocator<RamFS::Directory::Entry> entryAllocator;

/* FNV-1a hash */
static unsigned long hash(const char *str, int maxLen=RamFS::MaxFileNameLength) {
    unsigned long h = 2166136261;
    for (int i=0; i<maxLen; i++) {
        if (str[i] == 0)
            break;

        h = (h * 16777619) ^ (unsigned long)str[i];
    }

    return h;
}

void RamFS::Directory::init() {
    dirAllocator.init();
    entryAllocator.init();
}

void *RamFS::Directory::operator new(size_t sz) {
    UNUSED(sz);
    return dirAllocator.alloc();
}

void RamFS::Directory::operator delete(void *f){
    RamFS::Directory *fp = (RamFS::Directory *)f;
    dirAllocator.free(fp);
}

IDirectory *RamFS::Directory::create(uint16_t uid, uint16_t gid, IDirectory *parent, uint32_t perms) {
    Directory *rv = new Directory(uid, gid, parent, perms);
    atomic_incr(&rv->linkCount);
    return rv;
}

bool RamFS::Directory::addEntry(Entry *entry) {
    entry->leftChild = entry->rightChild = nullptr;

    if (firstEntry == nullptr) {
        firstEntry = entry;
        return true;
    }

    Entry *curr = firstEntry;
    while (true) {
        if (entry->nameHash < curr->nameHash) {
            if (curr->leftChild == nullptr) {
                curr->leftChild = entry;
                return true;
            }
            else {
                curr = curr->leftChild;
                continue;
            }
        }

        if ((curr->nameHash == entry->nameHash) && (!strncmp(curr->name, entry->name, MaxFileNameLength))) {
            return false;
        }

        if (curr->rightChild == nullptr) {
            curr->rightChild = entry;
            return true;
        }
        else {
            curr = curr->rightChild;
            continue;
        }
    }
}

RamFS::Directory::Entry * RamFS::Directory::fetchEntry(const char *entryName) {
    Entry *curr = firstEntry;
    uint32_t nameHash = hash(entryName);
    while (curr != nullptr) {
        if (nameHash < curr->nameHash) {
            if (curr->leftChild == nullptr)
                return nullptr;

            curr = curr->leftChild;
            continue;
        }

        if ((nameHash == curr->nameHash) && (!strncmp(curr->name, entryName, MaxFileNameLength)))
            return curr;

        if (curr->rightChild == nullptr)
            return nullptr;

        curr = curr->rightChild;
    }

    return nullptr;
}

RamFS::Directory::Entry *RamFS::Directory::removeEntry(const char *entryName) {
    Entry *curr = firstEntry;
    Entry **parentPtr = nullptr;
    uint32_t nameHash = hash(entryName);
    while (curr != nullptr) {
        if (nameHash < curr->nameHash) {
            if (curr->leftChild == nullptr)
                return nullptr;

            parentPtr = &curr->leftChild;
            curr = curr->leftChild;
            continue;
        }

        if (nameHash == curr->nameHash) {
            if (!strcmp(curr->name, entryName)) {
                // We have found a matching node. Delete it by replacing
                // it with its in-order traversal successor.

                //If curr has no right child, its parent is the in-order successor.
                if (curr->rightChild == nullptr) {
                    (parentPtr == nullptr) ? firstEntry = curr->leftChild : *parentPtr = curr->leftChild;
                    return curr;
                }

                // If curr has a right child, the leftmost element in the subtree rooted
                // at the right child's left is the in-order successor.
                if (curr->rightChild->leftChild == nullptr) {
                    // There is no subtree rooted at the left. The right child is
                    // the in-order successor.
                    curr->rightChild->leftChild = curr->leftChild;
                    (parentPtr == nullptr) ? firstEntry = curr->rightChild : *parentPtr = curr->rightChild;
                    return curr;
                }

                Entry *leftParent = curr->rightChild;
                Entry *leftMost = curr->rightChild->leftChild;
                while (leftMost->leftChild != nullptr) {
                    leftParent = leftMost;
                    leftMost = leftMost->leftChild;
                }

                leftParent->leftChild = leftMost->rightChild;

                leftMost->leftChild = curr->leftChild;
                leftMost->rightChild = curr->rightChild;
                (parentPtr == nullptr) ? firstEntry = leftMost : *parentPtr = leftMost;

                return curr;
            }
        }

        if (curr->rightChild == nullptr)
            return nullptr;

        parentPtr = &curr->rightChild;
        curr = curr->rightChild;
    }

    return nullptr;
}

void RamFS::Directory::shadowListAdd(RamFS::Directory::Entry *entry) {
    entry->leftChild = nullptr;
    entry->rightChild = nullptr;

    if (shadowed == nullptr) {
        shadowed = entry;
        return;
    }

    Entry *curr = shadowed;
    while (curr->leftChild != nullptr)
        curr = curr->leftChild;

    curr->leftChild = entry;
}

RamFS::Directory::Entry *RamFS::Directory::shadowListRemove(const char *entryName) {
    if (shadowed == nullptr)
        return nullptr;

    Entry *curr = shadowed;
    Entry **prev = &shadowed;

    uint32_t nameHash = hash(entryName);
    while ( (curr != nullptr) && (curr->nameHash != nameHash) && (strncmp(entryName, curr->name, MaxFileNameLength))) {
        prev = &curr;
        curr = curr->leftChild;
    }

    if (curr == nullptr)
        return nullptr;

    *prev = curr->leftChild;
    return curr;
}

RamFS::Directory::Directory(uint16_t uid, uint16_t gid, IDirectory *parent, uint32_t perms) {
    firstEntry = nullptr;
    shadowed = nullptr;
    readEntry = nullptr;

    this->perms = perms;
    this->uid = uid;
    this->gid = gid;

    linkCount = 0;
    numRefs = 0;
    numEntries = 0;

    spinlock_init("ramfs.dir.lock", &lock);

    // Every directory starts its life with two entries: "." and ".."
    // These are created when the directory is created and these entries
    // cannot be removed.
    Entry *dotEntry = entryAllocator.alloc();
    if (dotEntry == nullptr)
        return;

    strcpy(dotEntry->name, ".");
    dotEntry->nameHash = hash(dotEntry->name);
    dotEntry->type = DirEntry;
    dotEntry->dir = this;
    dotEntry->leftChild = nullptr;
    dotEntry->rightChild = nullptr;
    dotEntry->prevEntry = nullptr;
    dotEntry->nextEntry = nullptr;

    firstEntry = dotEntry;

    Entry *parentDirEntry = entryAllocator.alloc();
    strcpy(parentDirEntry->name, "..");
    parentDirEntry->nameHash = hash(parentDirEntry->name);
    parentDirEntry->type = DirEntry;
    parentDirEntry->dir = (RamFS::Directory *)parent;
    parentDirEntry->leftChild = nullptr;
    parentDirEntry->rightChild = nullptr;
    parentDirEntry->prevEntry = nullptr;
    parentDirEntry->nextEntry = nullptr;

    memset(&times.lastReadAt, 0, sizeof(struct timespec));
    TzClock::RealTime::time(&times.lastModifiedAt);
    TzClock::RealTime::time(&times.lastStatusChangeAt);

    addEntry(parentDirEntry);
}

RamFS::Directory::~Directory() {
    while (firstEntry != nullptr) {
        if ((strcmp(firstEntry->name, ".")) && (strcmp(firstEntry->name, "..")))
            (firstEntry->type == FileEntry) ? RamFS::File::destroy(firstEntry->file) : RamFS::Directory::destroy(firstEntry->dir);

        Entry *entry = removeEntry(firstEntry->name);
        entryAllocator.free(entry);
    }
}

void RamFS::Directory::destroy(IDirectory *dp) {
    RamFS::Directory *dir = (RamFS::Directory *)dp;

    atomic_decr(&dir->linkCount);
    if ((dir->linkCount == 0) && (dir->numRefs == 0))
        delete dir;
}

IDirectory *RamFS::Directory::clone() {
    atomic_incr(&linkCount);

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    return this;
}

void RamFS::Directory::open(RamFS::Directory *dir) {
    dir->addRef();
    dir->openDir();
}

void RamFS::Directory::close(RamFS::Directory *dir) {

    dir->closeDir();
    dir->release();
    if ((dir->linkCount == 0) && (dir->numRefs == 0))
        delete dir;
}

bool RamFS::Directory::checkPermissions(int permsBit) {

    TzTask *currTask = TzTask::current();
    if (currTask == nullptr)
        return true;

    uint16_t owner = currTask->owner();
    uint16_t group = currTask->group();

    //printf("%s: perms 0x%lx owner %d group %d uid %d gid %d\n", __PRETTY_FUNCTION__, perms, owner, group, uid, gid);

    if ((permsBit != PERMS_READ_BIT) && (permsBit != PERMS_WRITE_BIT) && (permsBit != PERMS_EXECUTE_BIT)) {
        err_msg("%s: Bad permsBit 0x%x\n", __PRETTY_FUNCTION__, permsBit);
        return false;
    }

    // Root has all permissions
    if (owner == System::UID)
        return true;

    if (owner == uid) {
        if (!(OWNER_ACCESS_PERMS(perms) & permsBit))
            return false;
        else
            return true;
    }

    if (group == gid) {
        if (!(GROUP_ACCESS_PERMS(perms) & permsBit))
            return false;
        else
            return true;
    }

    if (!(OTHERS_ACCESS_PERMS(perms) & permsBit))
            return false;

    return true;
}

int RamFS::Directory::changePermissions(uint32_t newPerms) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    perms = newPerms;
    return 0;
}

int RamFS::Directory::changeOwner(uint16_t newOwner) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);


    uid = newOwner;
    return 0;
}

int RamFS::Directory::changeGroup(uint16_t newGroup) {
    TzTask *currTask = TzTask::current();
    uint16_t owner =  (currTask == nullptr) ? 0 : currTask->owner();

    if ((owner != uid) && (owner != 0))
        return -EACCES;

    TzClock::RealTime::time(&times.lastStatusChangeAt);

    gid = newGroup;
    return 0;
}

int RamFS::Directory::addFile(const char *fileName, IFile *file) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *entry = entryAllocator.alloc();
    if (entry == nullptr)
        return -ENOMEM;

    strncpy(entry->name, fileName, MaxFileNameLength);
    entry->nameHash = hash(fileName);
    entry->type = FileEntry;
    entry->file = (RamFS::File *)file;
    entry->leftChild = nullptr;
    entry->rightChild = nullptr;
    entry->nextEntry = nullptr;
    entry->prevEntry = nullptr;

    if (!addEntry(entry)) {
        entryAllocator.free(entry);
        return -EEXIST;
    }

    TzClock::RealTime::time(&times.lastModifiedAt);

    numEntries++;
    return 0;
}

int RamFS::Directory::removeFile(const char *fileName) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *entry = removeEntry(fileName);
    if (entry == nullptr)
        return -ENOENT;

    if (entry->type != FileEntry) {
        addEntry(entry);
        return -EISDIR;
    }

    TzClock::RealTime::time(&times.lastModifiedAt);

    entryAllocator.free(entry);
    numEntries--;
    return 0;
}

int RamFS::Directory::removeDir(const char *dirName) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *entry = removeEntry(dirName);
    if (entry == nullptr)
        return -ENOENT;

    if (entry->type == FileEntry) {
        addEntry(entry);
        return -ENOTDIR;
    }

    if ((entry->dir->linkCount == 1) && (!entry->dir->isEmpty())) {
        addEntry(entry);
        return -ENOTEMPTY;
    }

    TzClock::RealTime::time(&times.lastModifiedAt);

    entryAllocator.free(entry);
    numEntries--;
    return 0;
}

int RamFS::Directory::addDir(const char *dirName, IDirectory *dir) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *entry = entryAllocator.alloc();
    if (entry == nullptr)
        return -ENOMEM;

    strncpy(entry->name, dirName, MaxFileNameLength);
    entry->nameHash = hash(dirName);
    entry->type = DirEntry;
    entry->dir = (RamFS::Directory *)dir;
    entry->leftChild = nullptr;
    entry->rightChild = nullptr;
    entry->nextEntry = nullptr;
    entry->prevEntry = nullptr;

    if (!addEntry(entry)) {
        entryAllocator.free(entry);
        return -EEXIST;
    }

    TzClock::RealTime::time(&times.lastModifiedAt);

    numEntries++;
    return 0;
}
void resetLabel(RamFS::Directory::Entry* readEntry)
{

    if(readEntry == nullptr)
            return;

        //printf("readEntry %s (%p <-> %p)\n",readEntry->name, readEntry->leftChild, readEntry->rightChild);
        readEntry->prevEntry= readEntry->leftChild;
        readEntry->nextEntry= readEntry->rightChild;

    if((readEntry->leftChild == nullptr) && (readEntry->rightChild == nullptr)){
            return;
    }
    else{
            resetLabel(readEntry->leftChild);
            resetLabel(readEntry->rightChild);
    }

}



void labelDir(RamFS::Directory::Entry* readEntry)
{

    if(readEntry == nullptr)
            return;

    while(readEntry){
        //printf("readEntry %s (%p %p)\n",readEntry->name, readEntry->prevEntry, readEntry->nextEntry);
        if((readEntry->prevEntry != nullptr)){

            RamFS::Directory::Entry* temp = readEntry->prevEntry;
            while(temp->nextEntry){
                temp=temp->nextEntry;
            }
            temp->nextEntry = readEntry->nextEntry;


            readEntry->nextEntry = readEntry->prevEntry;
            readEntry->prevEntry = nullptr;
        }else{
            readEntry = readEntry->nextEntry;
        }
    }

}

void RamFS::Directory::closeDir(){

    readEntry = nullptr;

}
void RamFS::Directory::openDir(){

    if(readEntry == nullptr){
        readEntry = firstEntry;
        resetLabel(readEntry);
        labelDir(readEntry);
    }

}

int RamFS::Directory::readDir(const void *data, const size_t size){
    SpinLocker locker(&lock);
    uint32_t count = 0;
    struct dirent de;
    size_t fillsize = 0;
    if (!checkPermissions(PERMS_READ_BIT))
        return -EACCES;


    //printf("Start from Entry %s %p\n", readEntry->name, readEntry->nextEntry);
    if(readEntry == nullptr)
        return 0;

    while(fillsize < size){
        de.d_ino = (ino_t) (count++);
        de.d_off = count++;
        de.d_type = readEntry->type;
        strncpy(de.d_name, readEntry->name, MaxFileNameLength);
        de.d_reclen = strlen(de.d_name) + sizeof(de.d_ino) + sizeof(de.d_off) + sizeof(de.d_reclen) + sizeof(de.d_type);
        de.d_reclen += (4 - de.d_reclen%4);

        //printf("readDir: Entry size %d Used (%d/%d) - %d %d %d %d %d \n",de.d_reclen,fillsize,size,strlen(de.d_name), sizeof(de.d_ino), sizeof(de.d_off), sizeof(de.d_reclen), sizeof(de.d_type));

        if(de.d_reclen < (size - fillsize)){
            memcpy((void *)((size_t)data+fillsize),&de,de.d_reclen);
            fillsize = fillsize + de.d_reclen;

            //printf("readDir: Entry %s (Next Entry %p) \n",readEntry->name, readEntry->nextEntry);

            readEntry = readEntry->nextEntry;
            if(readEntry == nullptr){ /* Last entry */
                break;
            }
            else{                   /* More entry */
                continue;
            }
        }else                   /* Ran out of buffer. Leave readEntry in the same place to continue next time */
            break;
    }

    TzClock::RealTime::time(&times.lastModifiedAt);

    return fillsize;
}


int RamFS::Directory::mount(const char *dirName, IDirectory *dir) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *entry = entryAllocator.alloc();
    if (entry == nullptr)
        return -ENOMEM;

    strncpy(entry->name, dirName, MaxFileNameLength);
    entry->nameHash = hash(dirName);
    entry->type = DirEntry;
    entry->dir = (RamFS::Directory *)dir;
    entry->leftChild = nullptr;
    entry->rightChild = nullptr;

    Entry *shadow = removeEntry(dirName);
    if (shadow == nullptr) {
        entryAllocator.free(entry);
        return -ENOENT;
    }

    shadowListAdd(shadow);
    return 0;

}

int RamFS::Directory::unmount(const char *dirName) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_WRITE_BIT))
        return -EACCES;

    Entry *shadow = shadowListRemove(dirName);
    if (shadow == nullptr)
        return -ENOENT;

    Entry *unmounted = removeEntry(dirName);
    UNUSED(unmounted);

    addEntry(shadow);
    return 0;
}

IFile *RamFS::Directory::file(const char *fileName) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_READ_BIT))
        return nullptr;

    Entry *entry = fetchEntry(fileName);
    if (entry == nullptr)
        return nullptr;

    if (entry->type == FileEntry)
        return entry->file;

    TzClock::RealTime::time(&times.lastReadAt);

    return nullptr;
}

IDirectory *RamFS::Directory::dir(const char *dirName) {
    SpinLocker locker(&lock);

    if (!checkPermissions(PERMS_READ_BIT))
        return nullptr;

    Entry *entry = fetchEntry(dirName);
    if (entry == nullptr)
        return nullptr;

    if (entry->type == DirEntry)
        return entry->dir;

    TzClock::RealTime::time(&times.lastReadAt);

    return nullptr;
}

bool RamFS::Directory::isEmpty() {
    return (numEntries == 0);
}

int RamFS::Directory::resolvePath(const char *path, IDirectory **dir, IFile **file, IDirectory **parentDir) {

    //printf("To resolve: %s\n", path);

    char componentName[MaxFileNameLength+1];
    *dir = nullptr;
    *file = nullptr;

    if (parentDir != nullptr)
        *parentDir = nullptr;

    int pathLength = strlen(path);
    if (pathLength > MaxFileNameLength*1024)
        return -ENAMETOOLONG;

    IDirectory *currDir = this;

    const char *curr = path;
    const char *end = curr + pathLength;
    const char *next = (*curr == '/') ? curr+1 : curr;

    while (next < end) {

        int i = 0;
        while ((next < end) && (*next != '/')) {
            componentName[i] = *next;
            next++;
            i++;
        }
        componentName[i] = 0;


        if (strlen(componentName) == 0) {
            curr = next+1;
            next = curr;
            continue;
        }

        if (*next != '/') {
            // We have reached the end of the path. Check if it ends with a valid file
            *file = currDir->file(componentName);
            if (parentDir != nullptr)
                *parentDir = currDir;
            if (*file == nullptr) {
                // Path does not end in a file. Check if it ends with a valid directory
                *dir = currDir->dir(componentName);
                if (*dir == nullptr)
                    return -ENOENT;

                return 0;
            }

            return 0;
        }

        currDir = currDir->dir(componentName);
        //printf("resolving %s currDir %p\n", componentName, currDir);
        if (currDir == nullptr)
            return -ENOENT;

        curr = next+1;
        next = curr;
    }

    *dir = currDir;
    return 0;
}

int RamFS::Directory::addRef() {
    atomic_incr(&numRefs);
    return numRefs;
}

int RamFS::Directory::release() {
    atomic_decr(&numRefs);
    return numRefs;
}

void RamFS::Directory::accessTimes(AccessTimes *tm) {
    memcpy(tm, &times, sizeof(struct AccessTimes));
}

#define DO_DIR_PRINTF 1

void RamFS::Directory::print(int level) {
#ifdef DO_DIR_PRINTF
    SpinLocker locker(&lock);

    Entry *entryStack[1024];
    int top = -1;
    entryStack[++top] = firstEntry;

    while (top >= 0) {
        Entry *entry = entryStack[top--];
        for (int i=0; i<level; i++) printf("\t");

        printf("%s\n", entry->name);
        if ((entry->type == DirEntry) && (strcmp(entry->name, ".")) && (strcmp(entry->name, "..")))
            entry->dir->print(level+1);

        if (entry->rightChild != nullptr)
            entryStack[++top] = entry->rightChild;

        if (entry->leftChild != nullptr)
            entryStack[++top] = entry->leftChild;
    }
#endif
}
