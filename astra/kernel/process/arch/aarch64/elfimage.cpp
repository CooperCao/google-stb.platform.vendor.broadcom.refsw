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
#include "elf.h"
#include "elfimage.h"
#include "arm/arm.h"
#include "config.h"
#include "objalloc.h"
#include "lib_string.h"
#include "pgtable.h"
#include "atomic.h"
#include "system.h"

const char ElfImage::loaderName32bit[MAX_PATH] = MUSL_LINKER_NAME;

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
	parseImage(procId, file, (const char *)NULL);
}

ElfImage::ElfImage(int procId,  IFile *file, const char *name) : userPageTable(*PageTable::kernelPageTable(), procId), pid(procId) {
	parseImage(procId, file, name);
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
			if(userPageTable.isAddrRangeMapped(va,PAGE_SIZE_4K_BYTES)){
				TzMem::PhysAddr pa = userPageTable.lookUp(va);
				TzMem::freePage(pa);

				userPageTable.unmapPage(va);

				pa = kernelPageTable->lookUp(va);
				if (pa != nullptr)
					kernelPageTable->unmapPage(va);

				va = (uint8_t *)va + PAGE_SIZE_4K_BYTES;
			}
		}

		delete sections[i];
	}
}

void ElfImage::parseImage(int procId,  IFile *file, const char *fileName){

	UNUSED(procId);
	Elf_Ehdr ehdr;
	size_t nr = file->read(&ehdr, sizeof(Elf_Ehdr), 0);
	if (nr < sizeof(Elf_Ehdr)) {
		printf("Could not read elf header: nr %zd \n", nr);
		status = NoElfHeader;
		return;
	}

	//printf("ELF parsing \n");
	dataSegmentEnd = nullptr;
	phOff=ehdr.e_phoff;

	//Load section headers
	sections.init();
	//Load Program Headers
	programHeaders.init();

	if (!validateElfHeader(ehdr))
		return;

	if (!parseProgramHeader(ehdr, file, fileName))
		return;

	if (!createStack(ehdr, file))
		return;

	if (!createParamsPages())
		return;

	if((fileName != NULL) && !strcmp(fileName, MUSL_LINKER_NAME)) {
		entryAddr = (TzMem::VirtAddr)(ehdr.e_entry + (uintptr_t)LINKER_LOAD_ADDR);
		LinkFlag = true;
	}
	else
		entryAddr = (TzMem::VirtAddr)(uintptr_t)ehdr.e_entry;

	status = Valid;
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

bool ElfImage::validateElfHeader(const Elf_Ehdr& ehdr) {
	// Validate the ELF header signature
	if ((ehdr.e_ident[EI_MAG0] != ELFMAG0) || (ehdr.e_ident[EI_MAG1] != ELFMAG1) ||
			(ehdr.e_ident[EI_MAG2] != ELFMAG2) || (ehdr.e_ident[EI_MAG3] != ELFMAG3)) {
		status = BadElfMagicNumber;
		err_msg("%s: bad magic 0x%x %c %c %c\n", __FUNCTION__, ehdr.e_ident[EI_MAG0], ehdr.e_ident[EI_MAG1],  ehdr.e_ident[EI_MAG2],  ehdr.e_ident[EI_MAG3]);
		return false;
	}

	//Validate the bit-ness
	if (ehdr.e_ident[EI_CLASS] != ELFCLASS64) {
		status = UnsupportedBitArch;
		err_msg("%s: bad arch %d. expected %d\n", __FUNCTION__, ehdr.e_ident[EI_CLASS], ELFCLASS64);
		return false;
	}

	// Validate the endian-ness
	if (ehdr.e_ident[EI_DATA] != ELFDATA2LSB) {
		status = UnsupportedEndianNess;
		err_msg("%s: bad endianness %d. expected %d\n", __FUNCTION__, ehdr.e_ident[EI_DATA], ELFDATA2LSB);
		return false;
	}

	// We can only run fully linked executables or load dynamic linker
	if ((ehdr.e_type != ET_EXEC) && (ehdr.e_type != ET_DYN)) {
		status = NotAnExecutable;
		err_msg("%s: not executable %d. expected %d \n", __FUNCTION__, ehdr.e_type, ET_EXEC);
		return false;
	}

	// We only run on ARM CPUs.
	if (ehdr.e_machine != EM_AARCH64) {
		status = UnsupportedCpuArch;
		err_msg("%s: not an ARM executable %d. expected %d \n", __FUNCTION__, ehdr.e_machine, EM_AARCH64);
		return false;
	}

	// We only support version 1.
	if (ehdr.e_version != EV_CURRENT) {
		status = UnsupportedElfVersion;
		err_msg("%s: bad elf version %d. expected %d \n", __FUNCTION__, ehdr.e_version, EV_CURRENT);
		return false;
	}

	return true;
}

bool ElfImage::parseLinkerPath(const Elf_Phdr& phdr, IFile *file) {

	/* This is the program interpreter used for shared libs
	 * Finding ld.so string
	 */
	uint64_t InterpSegmentPos = phdr.p_offset;
	int InterpSegmentSize = phdr.p_memsz;
	int InterpfileImgSize = phdr.p_filesz;
	int InterpSegmentFlags = phdr.p_flags;
	int InterpAlignment = phdr.p_align;

	int InterpSegmentNumUnits = (InterpSegmentSize/InterpAlignment);
	if ((InterpSegmentNumUnits * InterpAlignment) < InterpSegmentSize)
		InterpSegmentNumUnits++;

	/* Using Program memory space to launch Interpreter as well ?? */
	int InterpSegmentNumPages = (InterpSegmentNumUnits * InterpAlignment)/PAGE_SIZE_4K_BYTES;
    /* Cant be zero pages */
    if(InterpSegmentNumPages==0) InterpSegmentNumPages = 1;
	TzMem::VirtAddr userVA = (TzMem::VirtAddr) (uintptr_t)phdr.p_vaddr; // Program Virtual Address
	if (PAGE_START_4K(userVA) != userVA)
		InterpSegmentNumPages++;

	TzMem::VirtAddr va = allocAndMap(userVA, InterpSegmentNumPages, InterpSegmentFlags);
	if (va == nullptr) {
		status = OutOfMemory;
		return false;
	}

	int InterpPageOffset = (unsigned long)userVA % PAGE_SIZE_4K_BYTES;
	uint8_t *readAt = (uint8_t *)va + InterpPageOffset;
	int nr = file->read(readAt, InterpfileImgSize, InterpSegmentPos);
	if (nr < InterpfileImgSize) {
		status = IncompleteFile;
		return false;
	}

	//Make sure the interpreter path is NULL terminated
	if (readAt[InterpfileImgSize-1] != '\0') {
		err_msg("%s: Invalid Interpreter path. \n", __FUNCTION__);
		return false;
	}

	//strcpy(Ld_Path, (char *)readAt);
	//printf("%s ld.so path %s\n",__PRETTY_FUNCTION__,Ld_Path);

	return true;
}

bool ElfImage::staticLinkedElf(const Elf_Phdr& phdr, IFile *file, const char *fileName) {
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
		TzMem::VirtAddr userVA = (TzMem::VirtAddr) (uintptr_t)phdr.p_vaddr;
		//printf("Name of the fileName %s\n",fileName);

		/* Hack for Linker not to get loaded at 0x0 and other Load sections needs to be adjusted as well */
		if((fileName != NULL) && (!strcmp(fileName, MUSL_LINKER_NAME))){
			//printf("Linked Load section with 0x0\n");
			userVA = (void*)((unsigned long)userVA + (unsigned long)LINKER_LOAD_ADDR);
		}

		if (PAGE_START_4K(userVA) != userVA)
			segmentNumPages++;

		//printf("----> Mapping userVA %p numPages %d (%p)\n", userVA, segmentNumPages,
		//			(char *)userVA+segmentNumPages*4096);

		TzMem::VirtAddr va = allocAndMap(userVA, segmentNumPages, segmentFlags);
		if (va == nullptr)
			status = OutOfMemory;

		int pageOffset = (unsigned long)userVA % PAGE_SIZE_4K_BYTES;
		uint8_t *readAt = (uint8_t *)va + pageOffset;
		int nr = file->read(readAt, fileImgSize, segmentPos);
		if (nr < fileImgSize)
			status = IncompleteFile;

		TzMem::cacheFlush(va, segmentSize);
		kernelUnmap(va, segmentNumPages);

		ElfSection *section = new ElfSection;
		section->vaddr = (TzMem::VirtAddr)((unsigned long)userVA & PAGE_MASK);//userVA;
		section->numPages = segmentNumPages;
		section->refCount = 1;
		sections.pushBack(section);

		uint8_t *segEnd = (uint8_t *)userVA + (segmentNumPages * PAGE_SIZE_4K_BYTES);
		segEnd = (uint8_t *)PAGE_START_4K(segEnd) + PAGE_SIZE_4K_BYTES;
		if (segEnd > dataSegmentEnd)
			dataSegmentEnd = segEnd;
	}
	return true;
}

bool ElfImage::parseProgramHeader(const Elf_Ehdr& ehdr,  IFile *file, const char *fileName) {

	int numEntries = ehdr.e_phnum;
	int entrySize = ehdr.e_phentsize;

	if (entrySize != sizeof(Elf_Phdr)) {
		status = UnsupportedProgramHeader;
		err_msg("%s: bad program header size %d. expected %zd \n", __FUNCTION__, entrySize, sizeof(Elf_Phdr));
		return false;
	}

	uint64_t progHdrPos = ehdr.e_phoff;

	// If dynamically linked elf image. checking the Interp program type
	for (int i=0; i<numEntries; i++) {
		Elf_Phdr phdr;
		int nr = file->read(&phdr, entrySize, progHdrPos);
		if (nr != entrySize) {
			status = IncompleteProgramHeader;
			err_msg("%s: incomplete program header i %d nr %d. \n", __FUNCTION__,i, nr);
			return false;
		}
		// No need to save ProgramHeaders during finding the precense of INTERP section.
		//programHeaders.pushBack(phdr);

		if(phdr.p_type == PT_INTERP) {
			// Need to load dynamic linker
			LinkFlag = true;
			if(!parseLinkerPath(phdr, file))
				return false;
			else
				return true; // Linker path is copied to ELF LDPATH
		}
		progHdrPos += entrySize;
	}

	// If no INTERP section is found. Elf doesn't require the Linker
	if (LinkFlag == false) {

		// Not the linker ELF and just all other static ELFs
		//if(NULL == fileName){

		//printf("Static linking no INTERP header \n");
		progHdrPos = ehdr.e_phoff;

		for (int i=0; i<numEntries; i++) {
			Elf_Phdr phdr;
			int nr = file->read(&phdr, entrySize, progHdrPos);
			if (nr != entrySize) {
				status = IncompleteProgramHeader;
				err_msg("%s: incomplete program header i %d nr %d. \n", __FUNCTION__,i, nr);
				return false;
			}

			programHeaders.pushBack(phdr);
			if(!staticLinkedElf(phdr, file, fileName)){
				printf(" Error load sections static elf \n");
				return false;
			}
			progHdrPos += entrySize;
		}
	}

	return true;
}

void ElfImage::programHeader(int idx, Elf_Phdr *hdr) {
	if ((idx < 0) || (idx >= programHeaders.numElements())) {
		err_msg("%s: Bad header index %d.\n", __FUNCTION__, idx);
		return;
	}
	memcpy(hdr, &programHeaders[idx], sizeof(Elf_Phdr));
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

bool ElfImage::createStack(const Elf_Ehdr& ehdr,  IFile *file) {

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
			//printf("mmap fail va %p pa %p\n", page, pa);
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

bool ElfImage::locateStackAddr(const Elf_Ehdr& ehdr,  IFile *file) {
	UNUSED(ehdr);
	UNUSED(file);

	stackAddr = (TzMem::VirtAddr)DEFAULT_STACK_ADDR;
	return true;

}

ElfImage * ElfImage::loadElf(int procId, IFile *exeFile, IDirectory *exeDir) {

    UNUSED(exeDir);
    /*
     * Load the new image.
     */
    ElfImage *image = NULL;
    image = new ElfImage(procId, exeFile);
    if (image == nullptr){
        err_msg("Could not load elf image\n");
        goto ERR;
    }
    if (image->status != ElfImage::Valid) {
        err_msg("Could not load elf image. status %d\n", image->status);
        goto ERR;
    }

    /*
     * Check if the elf is dynamically linked ?
     */
    if(image->LinkFlag == true) {
        /* Now Delete the elf Image object */
        printf("\n Dynamically linked Elf image");
        delete image;
        image = (ElfImage *)NULL;

        /* Create new ElfImage to load linker */
        IFile *ldFile;
        IDirectory *ldDir, *ldparentDir;
        IDirectory *root = System::root();
        int rv = root->resolvePath(loaderName32bit,&ldDir,&ldFile,&ldparentDir);
        if( rv != 0) {
            printf("could not resolve LinkerName %s\n", loaderName32bit);
            goto ERR;
        }
        if (ldDir != nullptr) {
        // Implies files==nullptr and this is a directory
            printf("cannot execute a directory\n");
            goto ERR;
        }
        /* Load the linker elf file */
        image = new ElfImage(procId, ldFile,loaderName32bit);
        if(image == NULL) {
            err_msg("Couldn't load Linker elf Image\n");
            goto ERR;
        }
        if(image->status != ElfImage::Valid) {
            err_msg("Could not load Linker elf image. status %d\n", image->status);
            goto ERR;
        }
    }
    else {
        printf("\n Statically linked Elf image");
    }

    return image;

ERR:
    // Destroy the current elf image
    if (image != nullptr) {
        delete image;
    }
    return NULL;
}

ElfImage * ElfImage::loadElf(int procId, const ElfImage* parent) {
    /*
     * Load the parent image.
     */
    ElfImage *image = NULL;
    image = new ElfImage(procId, parent);
    if (image == nullptr){
        err_msg("Could not load elf image\n");
        goto ERR;
    }
    if (image->status != ElfImage::Valid) {
        err_msg("Could not load elf image. status %d\n", image->status);
        goto ERR;
    }

    return image;

ERR:
    // Destroy the current elf image
    if (image != nullptr) {
        delete image;
    }
    return NULL;
}
