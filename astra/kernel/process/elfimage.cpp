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
#include "elf.h"
#include "elfimage.h"
#include "arm/arm.h"
#include "config.h"
#include "objalloc.h"
#include "lib_string.h"
#include "pgtable.h"

static ObjCacheAllocator<ElfImage> allocator;

static ObjCacheAllocator<ElfImage::ElfSection> elfSectionAllocator;

void ElfImage::init() {
    allocator.init();
    elfSectionAllocator.init();
}

void *ElfImage::ElfSection::operator new(size_t sz) {
    UNUSED(sz);
    void *rv = elfSectionAllocator.alloc();
    return rv;
}

void ElfImage::ElfSection::operator delete(void *section) {
    elfSectionAllocator.free((ElfSection *)section);
}

ElfImage::ElfImage(int procId,  IFile *file) : userPageTable(*PageTable::kernelPageTable(), procId), pid(procId) {

    Elf32_Ehdr ehdr;
    size_t nr = file->read(&ehdr, sizeof(Elf32_Ehdr), 0);
    if (nr < sizeof(Elf32_Ehdr)) {
        printf("Could not read elf header: nr %d \n", nr);
        status = NoElfHeader;
        return;
    }

    dataSegmentEnd = nullptr;

    sections.init();
    programHeaders.init();

    if (!validateElfHeader(ehdr))
        return;

    if (!parseProgramHeader(ehdr, file))
        return;

    if (!createStack(ehdr, file))
        return;

    if (!createParamsPages())
        return;

    entryAddr = (TzMem::VirtAddr)ehdr.e_entry;
    status = Valid;

}

ElfImage::ElfImage(int procId, const ElfImage* parentImg) : userPageTable(parentImg->userPageTable, procId, true), pid(procId) {
    sections.init();
    programHeaders.init();

    sections.copyFrom(parentImg->sections);
    int numSections = sections.numElements();
    for (int i=0; i<numSections; i++)
        atomic_incr(&sections[i]->refCount);

    programHeaders.copyFrom(parentImg->programHeaders);

    stackAddr = parentImg->stackAddr;
    stackTopPage = parentImg->stackTopPage;

    paramsStart = parentImg->paramsStart;
    paramsStartUser = parentImg->paramsStartUser;
    dataSegmentEnd = parentImg->dataSegmentEnd;

    entryAddr = parentImg->entryAddr;
    status = Valid;
}

ElfImage::~ElfImage() {
    PageTable *kernelPageTable = PageTable::kernelPageTable();
    const int numSections = sections.numElements();
    for (int i=0; i<numSections; i++) {
        if (sections[i]->refCount > 1) {
            atomic_decr(&sections[i]->refCount);
            continue;
        }

        const int numPages = sections[i]->numPages;
        TzMem::VirtAddr va = sections[i]->vaddr;
        for (int j=0; j<numPages; j++) {
            TzMem::PhysAddr pa = userPageTable.lookUp(va);
            TzMem::freePage(pa);

            userPageTable.unmapPage(va);

            pa = kernelPageTable->lookUp(va);
            if (pa != nullptr)
                kernelPageTable->unmapPage(va);

            va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
        }

        delete sections[i];
    }
}

void ElfImage::unmapStackFromKernel() {
    // Remove the user-space stack's mapping in kernel space.
    TzMem::VirtAddr stack = ((unsigned char *)stackTopPage - USER_SPACE_STACK_SIZE);
    kernelUnmap(stack, USER_SPACE_STACK_SIZE/PAGE_SIZE_4K_BYTES);
}

void *ElfImage::operator new(size_t sz) {
    UNUSED(sz);
    return allocator.alloc();
}

void ElfImage::operator delete(void *image) {
    allocator.free((ElfImage *)image);
}

bool ElfImage::validateElfHeader(const Elf32_Ehdr& ehdr) {
    // Validate the ELF header signature
    if ((ehdr.e_ident[EI_MAG0] != ELFMAG0) || (ehdr.e_ident[EI_MAG1] != ELFMAG1) ||
            (ehdr.e_ident[EI_MAG2] != ELFMAG2) || (ehdr.e_ident[EI_MAG3] != ELFMAG3)) {
        status = BadElfMagicNumber;
        err_msg("%s: bad magic 0x%x %c %c %c\n", __FUNCTION__, ehdr.e_ident[EI_MAG0], ehdr.e_ident[EI_MAG1],  ehdr.e_ident[EI_MAG2],  ehdr.e_ident[EI_MAG3]);
        return false;
    }

    //Validate the bit-ness
    if (ehdr.e_ident[EI_CLASS] != ELFCLASS32) {
        status = UnsupportedBitArch;
        err_msg("%s: bad arch %d. expected %d\n", __FUNCTION__, ehdr.e_ident[EI_CLASS], ELFCLASS32);
        return false;
    }

    // Validate the endian-ness
    if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
        status = UnsupportedEndianNess;
        err_msg("%s: bad endianness %d. expected %d\n", __FUNCTION__, ehdr.e_ident[EI_DATA], ELFDATA2LSB);
        return false;
    }

    // We can only run fully linked executables
    if (ehdr.e_type != ET_EXEC) {
        status = NotAnExecutable;
        err_msg("%s: not executable %d. expected %d \n", __FUNCTION__, ehdr.e_type, ET_EXEC);
        return false;
    }

    // We only run on ARM CPUs.
    if (ehdr.e_machine != EM_ARM) {
        status = UnsupportedCpuArch;
        err_msg("%s: not an ARM executable %d. expected %d \n", __FUNCTION__, ehdr.e_machine, EM_ARM);
        return false;
    }

    // We only support version 1.
    if (ehdr.e_version != EV_CURRENT) {
        status = UnsupportedElfVersion;
        err_msg("%s: bad elf version %d. expected %d \n", __FUNCTION__, ehdr.e_version, EV_CURRENT);
        return false;
    }

    //printf("%s: success\n", __FUNCTION__);
    return true;
}

bool ElfImage::parseProgramHeader(const Elf32_Ehdr& ehdr,  IFile *file) {

    int numEntries = ehdr.e_phnum;
    int entrySize = ehdr.e_phentsize;

    if (entrySize != sizeof(Elf32_Phdr)) {
        status = UnsupportedProgramHeader;
        err_msg("%s: bad program header size %d. expected %d \n", __FUNCTION__, entrySize, sizeof(Elf32_Phdr));
        return false;
    }

    uint64_t progHdrPos = ehdr.e_phoff;

    for (int i=0; i<numEntries; i++) {
        Elf32_Phdr phdr;
        int nr = file->read(&phdr, entrySize, progHdrPos);
        if (nr != entrySize) {
            status = IncompleteProgramHeader;
            err_msg("%s: incomplete program header i %d nr %d. \n", __FUNCTION__,i, nr);
            return false;
        }

        programHeaders.pushBack(phdr);

        if (phdr.p_type == PT_LOAD) {

            uint64_t segmentPos = phdr.p_offset;
            int segmentSize = phdr.p_memsz;
            int fileImgSize = phdr.p_filesz;
            int segmentFlags = phdr.p_flags;
            int alignment = phdr.p_align;

            int segmentNumUnits = (segmentSize/alignment);
            if ((segmentNumUnits * alignment) < segmentSize)
                segmentNumUnits++;

            int segmentNumPages = (segmentNumUnits * alignment)/PAGE_SIZE_4K_BYTES;
            TzMem::VirtAddr userVA = (TzMem::VirtAddr) phdr.p_vaddr;
            if (PAGE_START_4K(userVA) != userVA)
                segmentNumPages++;

            //printf("----> Mapping userVA %p numPages %d (%p)\n", userVA, segmentNumPages,
            //          (char *)userVA+segmentNumPages*4096);

            TzMem::VirtAddr va = allocAndMap(userVA, segmentNumPages, segmentFlags);
            if (va == nullptr) {
                status = OutOfMemory;
                return false;
            }

            int pageOffset = (unsigned long)userVA % PAGE_SIZE_4K_BYTES;
            uint8_t *readAt = (uint8_t *)va + pageOffset;
            nr = file->read(readAt, fileImgSize, segmentPos);
            if (nr < fileImgSize) {
                status = IncompleteFile;
                return false;
            }

            TzMem::cacheFlush(va, segmentSize);
            kernelUnmap(va, segmentNumPages);


            ElfSection *section = new ElfSection;
            section->vaddr = userVA;
            section->numPages = segmentNumPages;
            section->refCount = 1;
            sections.pushBack(section);

            uint8_t *segEnd = (uint8_t *)userVA + (segmentNumPages * PAGE_SIZE_4K_BYTES);
            segEnd = (uint8_t *)PAGE_START_4K(segEnd) + PAGE_SIZE_4K_BYTES;
            if (segEnd > dataSegmentEnd)
                dataSegmentEnd = segEnd;
        }

        progHdrPos += entrySize;
    }

    //printf("%s: success\n", __FUNCTION__);
    return true;
}

void ElfImage::programHeader(int idx, Elf32_Phdr *hdr) {
    if ((idx < 0) || (idx >= programHeaders.numElements())) {
        err_msg("%s: Bad header index %d.\n", __FUNCTION__, idx);
        return;
    }

    memcpy(hdr, &programHeaders[idx], sizeof(Elf32_Phdr));
}

TzMem::VirtAddr ElfImage::allocAndMap(TzMem::VirtAddr userVa, const int numPages, int perms) {
    PageTable *kernelPageTable = PageTable::kernelPageTable();

    TzMem::VirtAddr rv = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, numPages*PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (rv == nullptr) {
        status = OutOfAddressSpace;
        return rv;
    }

    uint8_t *currVa = (uint8_t *)rv;
    uint8_t *currUserVa = (uint8_t *)userVa;

    for (int i=0; i<numPages; i++) {
        TzMem::PhysAddr pa = TzMem::allocPage(pid);
        if (pa == nullptr) {
            kernelPageTable->unmapPageRange(rv, currVa);
            err_msg("%s: alloc failure !\n", __PRETTY_FUNCTION__);
            status = OutOfMemory;
            return nullptr;
        }

        TzMem::VirtAddr mapAddr = (TzMem::VirtAddr)((unsigned long)currUserVa & PAGE_MASK);

        kernelPageTable->mapPage(currVa, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL);
        memset(currVa, 0, PAGE_SIZE_4K_BYTES);

        int userPerms = (perms & PF_W) ? MEMORY_ACCESS_RW_USER : MEMORY_ACCESS_RO_USER;
        bool execNever = !(perms & PF_X);
        userPageTable.mapPage(mapAddr, pa, MAIR_MEMORY, userPerms, execNever);


        currVa += PAGE_SIZE_4K_BYTES;
        currUserVa += PAGE_SIZE_4K_BYTES;
    }

    return rv;
}

void ElfImage::kernelUnmap(TzMem::VirtAddr va, int numPages) {
    PageTable *kernelPageTable = PageTable::kernelPageTable();
    uint8_t *rangeStart = (uint8_t *)va;
    kernelPageTable->unmapPageRange(rangeStart, rangeStart+((numPages - 1) * PAGE_SIZE_4K_BYTES));
}

bool ElfImage::createStack(const Elf32_Ehdr& ehdr,  IFile *file) {

    if (!locateStackAddr(ehdr, file)) {
        status = IncompleteFile;
        return false;
    }

    ElfSection *section = new ElfSection;
    section->vaddr = ((unsigned char *)stackAddr - USER_SPACE_STACK_SIZE);
    section->numPages = USER_SPACE_STACK_SIZE/PAGE_SIZE_4K_BYTES;
    section->refCount = 1;

    TzMem::VirtAddr va = allocAndMap(section->vaddr, section->numPages, (PF_R|PF_W));
    if (va == nullptr)
        return false;

    sections.pushBack(section);
    stackTopPage = ((unsigned char *)va + USER_SPACE_STACK_SIZE);

    //printf("%s: stack va %p topPage %p\n", __PRETTY_FUNCTION__, va, stackTopPage);
    return true;
}

bool ElfImage::createParamsPages() {
    TzMem::VirtAddr va = userPageTable.reserveAddrRange((void *)DEFAULT_STACK_ADDR, ParamsBlockSize, PageTable::ScanForward);
    if (va == nullptr) {
        err_msg("Ran out of user virtual address space\n");
        return false;
    }

    ElfSection *section = new ElfSection;
    section->vaddr = va;
    section->numPages = NumParamsPages;
    section->refCount = 1;

    TzMem::VirtAddr kva = allocAndMap(section->vaddr, section->numPages, (PF_R|PF_W));
    if (kva == nullptr)
        return false;

    sections.pushBack(section);
    paramsStart = kva;
    paramsStartUser = va;

    return true;
}

int ElfImage::addMmapSection(TzMem::VirtAddr va, int numPages, int accessPerms, bool noExec, bool shared, uint16_t tid) {
    PageTable *kernelPageTable = PageTable::kernelPageTable();
    uint8_t *curr = (uint8_t *)va;
    uint8_t *stop = curr + (numPages * PAGE_SIZE_4K_BYTES);

    //printf("%s: va %p numPages %d\n", __FUNCTION__, va, numPages);

    TzMem::VirtAddr kva = kernelPageTable->reserveAddrRange((void *)KERNEL_HEAP_START, PAGE_SIZE_4K_BYTES, PageTable::ScanForward);
    if (kva == nullptr)
        return -ENOMEM;

    while (curr < stop) {
        TzMem::PhysAddr pa = TzMem::allocPage(tid);
        if (pa == nullptr)
            break;

        userPageTable.mapPage(curr, pa, MAIR_MEMORY, accessPerms, noExec, shared);

        kernelPageTable->mapPage(kva, pa, MAIR_MEMORY, MEMORY_ACCESS_RW_KERNEL, true, false);
        memset(kva, 0, PAGE_SIZE_4K_BYTES);
        kernelPageTable->unmapPage(kva);

        curr += PAGE_SIZE_4K_BYTES;
    }

    if (curr < stop) {
        // Undo all the mappings and fail
        uint8_t *page= (uint8_t *)va;
        while (va != curr) {
            TzMem::PhysAddr pa = userPageTable.lookUp(page);

            userPageTable.unmapPage(page);
            printf("mmap fail va %p pa %p\n", page, pa);
            TzMem::freePage(pa);

            page += PAGE_SIZE_4K_BYTES;
        }

        return -ENOMEM;
    }

    ElfSection *section = new ElfSection;
    section->vaddr = va;
    section->numPages = numPages;
    section->refCount = 1;

    sections.pushBack(section);

    return 0;
}

void ElfImage::removeMmapSection(TzMem::VirtAddr va) {

    int numSections = sections.numElements();
    for (int i=0; i<numSections; i++) {
        if (sections[i]->vaddr != va)
            continue;

        if (sections[i]->refCount > 1) {
            atomic_decr(&sections[i]->refCount);
            return;
        }

        const int numPages = sections[i]->numPages;
        TzMem::VirtAddr va = sections[i]->vaddr;
        for (int j=0; j<numPages; j++) {
            TzMem::PhysAddr pa = userPageTable.lookUp(va);
            TzMem::freePage(pa);

            userPageTable.unmapPage(va);

            va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
        }

        delete sections[i];
        sections[i] = sections[numSections-1];
        sections.popBack(nullptr);
    }
}

bool ElfImage::locateStackAddr(const Elf32_Ehdr& ehdr,  IFile *file) {
    UNUSED(ehdr);
    UNUSED(file);

    stackAddr = (TzMem::VirtAddr)DEFAULT_STACK_ADDR;
    return true;

}
