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

#ifndef PROCESS_ELF_H_
#define PROCESS_ELF_H_

/*
 * ELF data structures and values
 */

typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Word;
typedef signed int Elf32_Sword;
typedef unsigned int Elf32_Off;
typedef unsigned int Elf32_Addr;
typedef unsigned char Elf_Char;

/*
 * File Header
 */

#define EI_NIDENT   16

typedef struct {
    Elf_Char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

/* e_indent */
#define EI_MAG0     0   /* File identification byte 0 index */
#define EI_MAG1     1   /* File identification byte 1 index */
#define EI_MAG2     2   /* File identification byte 2 index */
#define EI_MAG3     3   /* File identification byte 3 index */
#define EI_CLASS    4   /* File class */
#define  ELFCLASSNONE    0  /* Invalid class */
#define  ELFCLASS32  1  /* 32-bit objects */
#define  ELFCLASS64  2  /* 64-bit objects */
#define EI_DATA     5   /* Data encoding */
#define  ELFDATANONE     0  /* Invalid data encoding */
#define  ELFDATA2LSB     1  /* 2's complement, little endian */
#define  ELFDATA2MSB     2  /* 2's complement, big endian */
#define EI_VERSION  6   /* File version */
#define EI_PAD      7   /* Start of padding bytes */

#define ELFMAG0     0x7F    /* Magic number byte 0 */
#define ELFMAG1     'E' /* Magic number byte 1 */
#define ELFMAG2     'L' /* Magic number byte 2 */
#define ELFMAG3     'F' /* Magic number byte 3 */

/* e_type */
#define ET_NONE     0   /* No file type */
#define ET_REL      1   /* Relocatable file */
#define ET_EXEC     2   /* Executable file */
#define ET_DYN      3   /* Shared object file */
#define ET_CORE     4   /* Core file */
#define ET_LOPROC   0xFF00  /* Processor-specific */
#define ET_HIPROC   0xFFFF  /* Processor-specific */

/* e_machine */
#define EM_NONE     0   /* No machine */
#define EM_M32      1   /* AT&T WE 32100 */
#define EM_SPARC    2   /* SUN SPARC */
#define EM_386      3   /* Intel 80386 */
#define EM_68K      4   /* Motorola m68k family */
#define EM_88K      5   /* Motorola m88k family */
#define EM_860      7   /* Intel 80860 */
#define EM_MIPS     8   /* MIPS R3000  */
#define EM_ARM      40  /* Advanced RISC Machines ARM  */

/* e_version */
#define EV_NONE     0   /* Invalid ELF version */
#define EV_CURRENT  1   /* Current version */

/*
 * Program Header
 */
typedef struct {
    Elf32_Word p_type;  /* Identifies program segment type */
    Elf32_Off p_offset; /* Segment file offset */
    Elf32_Addr p_vaddr; /* Segment virtual address */
    Elf32_Addr p_paddr; /* Segment physical address */
    Elf32_Word p_filesz;    /* Segment size in file */
    Elf32_Word p_memsz; /* Segment size in memory */
    Elf32_Word p_flags; /* Segment flags */
    Elf32_Word p_align; /* Segment alignment, file & memory */
} Elf32_Phdr;

/* p_type */
#define PT_NULL     0   /* Program header table entry unused */
#define PT_LOAD     1   /* Loadable program segment */
#define PT_DYNAMIC  2   /* Dynamic linking information */
#define PT_INTERP   3   /* Program interpreter */
#define PT_NOTE     4   /* Auxiliary information */
#define PT_SHLIB    5   /* Reserved, unspecified semantics */
#define PT_PHDR     6   /* Entry for header table itself */
#define PT_LOPROC   0x70000000  /* Processor-specific */
#define PT_HIPROC   0x7FFFFFFF  /* Processor-specific */

/* p_flags */
#define PF_X        (1 << 0)    /* Segment is executable */
#define PF_W        (1 << 1)    /* Segment is writable */
#define PF_R        (1 << 2)    /* Segment is readable */
#define PF_MASKPROC 0xF0000000  /* Processor-specific reserved bits */

/*
 * Section Header
 */
typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

/* sh_type */
#define SHT_NULL    0   /* Section header table entry unused */
#define SHT_PROGBITS    1   /* Program specific (private) data */
#define SHT_SYMTAB  2   /* Link editing symbol table */
#define SHT_STRTAB  3   /* A string table */
#define SHT_RELA    4   /* Relocation entries with addends */
#define SHT_HASH    5   /* A symbol hash table */
#define SHT_DYNAMIC 6   /* Information for dynamic linking */
#define SHT_NOTE    7   /* Information that marks file */
#define SHT_NOBITS  8   /* Section occupies no space in file */
#define SHT_REL     9   /* Relocation entries, no addends */
#define SHT_SHLIB   10  /* Reserved, unspecified semantics */
#define SHT_DYNSYM  11  /* Dynamic linking symbol table */
#define SHT_LOPROC  0x70000000  /* Processor-specific semantics, lo */
#define SHT_HIPROC  0x7FFFFFFF  /* Processor-specific semantics, hi */
#define SHT_LOUSER  0x80000000  /* Application-specific semantics */
#define SHT_HIUSER  0x8FFFFFFF  /* Application-specific semantics */

/* sh_flags */
#define SHF_WRITE   (1 << 0)    /* Writable data during execution */
#define SHF_ALLOC   (1 << 1)    /* Occupies memory during execution */
#define SHF_EXECINSTR   (1 << 2)    /* Executable machine instructions */
#define SHF_MASKPROC    0xF0000000  /* Processor-specific semantics */

/*
 * Symbol Table
 */
typedef struct {
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    Elf_Char st_info;
    Elf_Char st_other;
    Elf32_Half st_shndx;
} Elf32_Sym;

#define ELF_ST_BIND(val)        (((unsigned int)(val)) >> 4)
#define ELF_ST_TYPE(val)        ((val) & 0xF)
#define ELF_ST_INFO(bind,type)      (((bind) << 4) | ((type) & 0xF))

/* symbol binding */
#define STB_LOCAL   0   /* Symbol not visible outside obj */
#define STB_GLOBAL  1   /* Symbol visible outside obj */
#define STB_WEAK    2   /* Like globals, lower precedence */
#define STB_LOPROC  13  /* Application-specific semantics */
#define STB_HIPROC  15  /* Application-specific semantics */

/* symbol type */
#define STT_NOTYPE  0   /* Symbol type is unspecified */
#define STT_OBJECT  1   /* Symbol is a data object */
#define STT_FUNC    2   /* Symbol is a code object */
#define STT_SECTION 3   /* Symbol associated with a section */
#define STT_FILE    4   /* Symbol gives a file name */
#define STT_LOPROC  13  /* Application-specific semantics */
#define STT_HIPROC  15  /* Application-specific semantics */

/* special values st_shndx */
#define SHN_UNDEF   0   /* Undefined section reference */
#define SHN_LORESERV    0xFF00  /* Begin range of reserved indices */
#define SHN_LOPROC  0xFF00  /* Begin range of appl-specific */
#define SHN_HIPROC  0xFF1F  /* End range of appl-specific */
#define SHN_ABS     0xFFF1  /* Associated symbol is absolute */
#define SHN_COMMON  0xFFF2  /* Associated symbol is in common */
#define SHN_HIRESERVE   0xFFFF  /* End range of reserved indices */

/* Auxiliary vector table types */
#define AT_NULL     0
#define AT_IGNORE   1
#define AT_EXECFD   2
#define AT_PHDR     3
#define AT_PHENT    4
#define AT_PHNUM    5
#define AT_PAGESZ   6
#define AT_BASE     7
#define AT_FLAGS    8
#define AT_ENTRY    9
#define AT_NOTELF   10
#define AT_UID      11
#define AT_EUID     12
#define AT_GID      13
#define AT_EGID     14
#define AT_CLKTCK   17
#define AT_PLATFORM 15
#define AT_HWCAP    16
#define AT_FPUCW    18
#define AT_DCACHEBSIZE  19
#define AT_ICACHEBSIZE  20
#define AT_UCACHEBSIZE  21
#define AT_IGNOREPPC    22
#define AT_SECURE   23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM   25
#define AT_HWCAP2   26
#define AT_EXECFN   31


#endif /* PROCESS_ELF_H_ */
