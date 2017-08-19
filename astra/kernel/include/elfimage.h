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

#ifndef PROCESS_ELFIMAGE_H_
#define PROCESS_ELFIMAGE_H_

#include "fs/fs.h"
#include "pgtable.h"
#include "tzmemory.h"
#include "utils/vector.h"

#include "elf.h"

#define DEFAULT_STACK_ADDR 0x70000000
#define DEFAULT_HEAP_ADDR 0x40000000
#define LINKER_LOAD_ADDR 0x80000
#define MAX_PATH 4096
#define MUSL_LINKER_NAME "/lib/libc.so"

class ElfImage {
public:
    static void init();
	~ElfImage();

	unsigned long entry() { return (unsigned long) entryAddr; }
	unsigned int phOffset() { return phOff;}
	unsigned long stackBase() { return (unsigned long) stackAddr; }

	TzMem::VirtAddr stackTopPageVA() { return stackTopPage; }
	void unmapStackFromKernel();

	TzMem::VirtAddr paramsPageVA() { return paramsStart; }
	TzMem::VirtAddr paramsPageUser() { return paramsStartUser; }
	void unmapParamsFromKernel();

	void *operator new (size_t sz);
	void operator delete (void *task);

	int numProgramHeaders() { return programHeaders.numElements(); }
	void programHeader(int idx, Elf_Phdr *hdr);

	TzMem::VirtAddr dataSegmentBrk() const { return (TzMem::VirtAddr)DEFAULT_HEAP_ADDR; }
	int addMmapSection(TzMem::VirtAddr va, int numPages, int accessPerms, bool noExec, bool shared, uint16_t tid);
	void removeMmapSection(TzMem::VirtAddr va);

    void allocAndCopyFrom(const PageTable *srcPt, TzMem::VirtAddr srcVa, TzMem::VirtAddr dstVa, const int numPages);
    static ElfImage * loadElf(int pid, IFile *exeFile, IDirectory *exeDir);
    static ElfImage * loadElf(int pid, const ElfImage* parent);
    const char *ldNamePtr(void) { return loaderName32bit; };

public:
	static const int NumParamsPages=4;
	static const int ParamsBlockSize = NumParamsPages*PAGE_SIZE_4K_BYTES;

	bool LinkFlag = false;

	int status;
	PageTable userPageTable;

	class ElfSection {
	public:
		TzMem::VirtAddr vaddr;
		int numPages;
		uint32_t refCount;

		void *operator new(size_t sz);

		void operator delete(void *es);

	public:
		~ElfSection() {}
	};

private:
	TzMem::VirtAddr entryAddr;
	TzMem::VirtAddr stackAddr;
	TzMem::VirtAddr stackTopPage;
	TzMem::VirtAddr paramsStart;
	TzMem::VirtAddr paramsStartUser;
	int pid;
	unsigned int phOff;

	tzutils::Vector<ElfSection *> sections;
	TzMem::VirtAddr dataSegmentEnd;

	tzutils::Vector<Elf_Phdr> programHeaders;
    const static char loaderName32bit[MAX_PATH];

private:
    ElfImage(int pid,  IFile *file);
    ElfImage(int pid,  IFile *file, const char *name);
    ElfImage(int pid, const ElfImage* parent);

	void parseImage(int procId, IFile *file, const char *name);
	bool validateElfHeader(const Elf_Ehdr& ehdr);
	bool parseProgramHeader(const Elf_Ehdr& ehdr, IFile *file, const char *name);
	bool parseLinkerPath(const Elf_Phdr& phdr, IFile *file);
	bool staticLinkedElf(const Elf_Phdr& phdr, IFile *file, const char *name);

	bool createStack(const Elf_Ehdr& ehdr,  IFile *file);
	bool locateStackAddr(const Elf_Ehdr& ehdr,  IFile *file);

	bool createParamsPages();

	TzMem::VirtAddr allocAndMap(TzMem::VirtAddr userVA, const int numPages, int perms);
	void kernelUnmap(TzMem::VirtAddr virtAddr, int numPages);

public:
	static const int Valid = 0;
	static const int NoElfHeader = -1;
	static const int BadElfMagicNumber = -2;
	static const int UnsupportedBitArch = -3;
	static const int UnsupportedEndianNess = -4;
	static const int NotAnExecutable = -5;
	static const int UnsupportedCpuArch = -6;
	static const int UnsupportedElfVersion = -7;
	static const int UnsupportedProgramHeader = -8;
	static const int IncompleteProgramHeader = -9;
	static const int OutOfMemory = -10;
	static const int OutOfAddressSpace = -11;
	static const int IncompleteFile = -12;

};



#endif /* PROCESS_ELFIMAGE_H_ */
