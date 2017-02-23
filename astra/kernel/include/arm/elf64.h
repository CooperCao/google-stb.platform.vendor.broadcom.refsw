/******************************************************************************
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
 *****************************************************************************/
#ifndef PROCESS_ELF_64_H_
#define PROCESS_ELF_64_H_

/*
 * ELF 64 data structures and values
 */

typedef unsigned short Elf64_Half;
typedef unsigned int Elf64_Word;
typedef signed int Elf64_Sword;
typedef unsigned long Elf64_Off;
typedef unsigned long Elf64_Addr;
typedef unsigned long Elf64_Xword;
typedef unsigned char Elf64_Char;

/*
 * File Header
 */

#define EI_NIDENT	16

typedef struct
{
	Elf64_Char  e_ident[EI_NIDENT];     /* ELF identification */
    Elf64_Half  e_type;                 /* Object file type */
    Elf64_Half  e_machine;              /* Machine type */
    Elf64_Word  e_version;              /* Object file version */
    Elf64_Addr  e_entry;                /* Entry point address */
    Elf64_Off   e_phoff;                /* Program header offset */
    Elf64_Off   e_shoff;                /* Section header offset */
    Elf64_Word  e_flags;                /* Processor-specific flags */
    Elf64_Half  e_ehsize;               /* ELF header size */
    Elf64_Half  e_phentsize;            /* Size of program header entry */
    Elf64_Half  e_phnum;                /* Number of program header entries */
    Elf64_Half  e_shentsize;            /* Size of section header entry */
    Elf64_Half  e_shnum;                /* Number of section header entries */
    Elf64_Half  e_shstrndx;             /* Section name string table index */
} Elf64_Ehdr;

typedef Elf64_Ehdr Elf_Ehdr;

/* e_indent */
#define EI_MAG0		    0	/* File identification byte 0 index */
#define EI_MAG1		    1	/* File identification byte 1 index */
#define EI_MAG2		    2	/* File identification byte 2 index */
#define EI_MAG3		    3	/* File identification byte 3 index */
#define EI_CLASS	    4	/* File class */
#define  ELFCLASSNONE	0   /* Invalid class */
#define  ELFCLASS32	    1	/* 32-bit objects */
#define  ELFCLASS64	    2	/* 64-bit objects */
#define EI_DATA		    5	/* Data encoding */
#define  ELFDATANONE	0	/* Invalid data encoding */
#define  ELFDATA2LSB	1	/* 2's complement, little endian */
#define  ELFDATA2MSB	2	/* 2's complement, big endian */
#define EI_VERSION	    6	/* File version */
#define EI_OSABI		7	/* OS/ABI identification */
#define EI_ABIVERSION   8   /* ABI version */
#define EI_PAD          9   /* Start of padding bytes */

/* Magic Word */
#define ELFMAG0     0x7F	/* Magic number byte 0 */
#define ELFMAG1		'E'     /* Magic number byte 1 */
#define ELFMAG2		'L'	    /* Magic number byte 2 */
#define ELFMAG3		'F'	    /* Magic number byte 3 */

/* e_type */
#define ET_NONE		0	    /* No file type */
#define ET_REL		1	    /* Relocatable file */
#define ET_EXEC		2	    /* Executable file */
#define ET_DYN		3	    /* Shared object file */
#define ET_CORE		4	    /* Core file */
#define ET_LOOS     0xFE00  /* Environment-specific use */
#define ET_HIOS     0xFEFF  /* Environment-specific use */
#define ET_LOPROC	0xFF00	/* Processor-specific */
#define ET_HIPROC	0xFFFF	/* Processor-specific */

/* e_machine */
#define EM_NONE		0	/* No machine */
#define EM_M32		1	/* AT&T WE 32100 */
#define EM_SPARC	2	/* SUN SPARC */
#define EM_386		3	/* Intel 80386 */
#define EM_68K		4	/* Motorola m68k family */
#define EM_88K		5	/* Motorola m88k family */
#define EM_860		7	/* Intel 80860 */
#define EM_MIPS		8	/* MIPS R3000  */
#define EM_ARM		40	/* Advanced RISC Machines ARM  */
#define EM_AARCH64  183 /* ARM 64 bit Architecture */

/* e_version */
#define EV_NONE		0	/* Invalid ELF version */
#define EV_CURRENT	1	/* Current version */

/* Program Header */
typedef struct
{
    Elf64_Word  p_type;     /* Type of segment */
    Elf64_Word  p_flags;    /* Segment attributes */
    Elf64_Off   p_offset;   /* Offset in file */
    Elf64_Addr  p_vaddr;    /* Virtual address in memory */
    Elf64_Addr  p_paddr;    /* Reserved */
    Elf64_Xword p_filesz;   /* Size of segment in file */
    Elf64_Xword p_memsz;    /* Size of segment in memory */
    Elf64_Xword p_align;    /* Alignment of segment */
} Elf64_Phdr;

typedef Elf64_Phdr Elf_Phdr;


/* p_type */
#define	PT_NULL		0	        /* Program header table entry unused */
#define PT_LOAD		1	        /* Loadable program segment */
#define PT_DYNAMIC	2	        /* Dynamic linking information */
#define PT_INTERP	3	        /* Program interpreter */
#define PT_NOTE		4	        /* Auxiliary information */
#define PT_SHLIB	5	        /* Reserved, unspecified semantics */
#define PT_PHDR		6	        /* Entry for header table itself */
#define PT_LOOS     0x60000000  /* Environment-specific use */
#define PT_HIOS     0x6FFFFFFF  /* Environment-specific use */
#define PT_LOPROC	0x70000000	/* Processor-specific */
#define PT_HIPROC	0x7FFFFFFF	/* Processor-specific */

/* p_flags */
#define PF_X		(1 << 0)	/* Segment is executable */
#define PF_W		(1 << 1)	/* Segment is writable */
#define PF_R		(1 << 2)	/* Segment is readable */
#define PF_MASKOS   0x00FF0000  /* Environment-specific reserved bits */
#define PF_MASKPROC	0xF0000000	/* Processor-specific reserved bits */

/* Section Header */
typedef struct
{
    Elf64_Word  sh_name;        /* Section name */
    Elf64_Word  sh_type;        /* Section type */
    Elf64_Xword sh_flags;       /* Section attributes */
    Elf64_Addr  sh_addr;        /* Virtual address in memory */
    Elf64_Off   sh_offset;      /* Offset in file */
    Elf64_Xword sh_size;        /* Size of section */
    Elf64_Word  sh_link;        /* Link to other section */
    Elf64_Word  sh_info;        /* Miscellaneous information */
    Elf64_Xword sh_addralign;   /* Address alignment boundary */
    Elf64_Xword sh_entsize;     /* Size of entries, if section has table */
} Elf64_Shdr;

/* sh_type */
#define SHT_NULL	0	/* Section header table entry unused */
#define SHT_PROGBITS	1	/* Program specific (private) data */
#define SHT_SYMTAB	2	/* Link editing symbol table */
#define SHT_STRTAB	3	/* A string table */
#define SHT_RELA	4	/* Relocation entries with addends */
#define SHT_HASH	5	/* A symbol hash table */
#define SHT_DYNAMIC	6	/* Information for dynamic linking */
#define SHT_NOTE	7	/* Information that marks file */
#define SHT_NOBITS	8	/* Section occupies no space in file */
#define SHT_REL		9	/* Relocation entries, no addends */
#define SHT_SHLIB	10	/* Reserved, unspecified semantics */
#define SHT_DYNSYM	11	/* Dynamic linking symbol table */
#define SHT_LOOS    0x60000000  /* Environment-specific use */
#define SHT_HIOS    0x6FFFFFFF  /* Environment-specific use */
#define SHT_LOPROC	0x70000000	/* Processor-specific semantics, lo */
#define SHT_HIPROC	0x7FFFFFFF	/* Processor-specific semantics, hi */
#define SHT_LOUSER	0x80000000	/* Application-specific semantics */
#define SHT_HIUSER	0x8FFFFFFF	/* Application-specific semantics */

/* sh_flags */
#define SHF_WRITE	(1 << 0)	/* Writable data during execution */
#define SHF_ALLOC	(1 << 1)	/* Occupies memory during execution */
#define SHF_EXECINSTR	(1 << 2)	/* Executable machine instructions */
#define SHF_MASKOS      0x0F000000  /* Environment-specific use */
#define SHF_MASKPROC	0xF0000000	/* Processor-specific semantics */

/* Symbol Table */
typedef struct
{
    Elf64_Word      st_name; /* Symbol name */
    unsigned char   st_info; /* Type and Binding attributes */
    unsigned char   st_other; /* Reserved */
    Elf64_Half      st_shndx; /* Section table index */
    Elf64_Addr      st_value; /* Symbol value */
    Elf64_Xword     st_size; /* Size of object (e.g., common) */
} Elf64_Sym;


#define ELF_ST_BIND(val)		(((unsigned int)(val)) >> 4)
#define ELF_ST_TYPE(val)		((val) & 0xF)
#define ELF_ST_INFO(bind,type)		(((bind) << 4) | ((type) & 0xF))

/* symbol binding */
#define STB_LOCAL	0	/* Symbol not visible outside obj */
#define STB_GLOBAL	1	/* Symbol visible outside obj */
#define STB_WEAK	2	/* Like globals, lower precedence */
#define STB_LOOS    10  /* Environment-specific use */
#define STB_HIOS    12  /* Environment-specific use */
#define STB_LOPROC	13	/* Application-specific semantics */
#define STB_HIPROC	15	/* Application-specific semantics */

/* symbol type */
#define STT_NOTYPE	0	/* Symbol type is unspecified */
#define STT_OBJECT	1	/* Symbol is a data object */
#define STT_FUNC	2	/* Symbol is a code object */
#define STT_SECTION	3	/* Symbol associated with a section */
#define STT_FILE	4	/* Symbol gives a file name */
#define STT_LOOS    10  /* Environment-specific use */
#define STT_HIOS    12  /* Environment-specific use */
#define STT_LOPROC	13	/* Application-specific semantics */
#define STT_HIPROC	15	/* Application-specific semantics */

/* special values st_shndx */
#define SHN_UNDEF	0	/* Undefined section reference */
#define SHN_LORESERV	0xFF00	/* Begin range of reserved indices */
#define SHN_LOPROC	0xFF00	/* Begin range of appl-specific */
#define SHN_HIPROC	0xFF1F	/* End range of appl-specific */
#define SHN_LOOS    0xFF20  /* Environment-specific use */
#define SHN_HIOS    0xFF3F  /* Environment-specific use */
#define SHN_ABS		0xFFF1	/* Associated symbol is absolute */
#define SHN_COMMON	0xFFF2	/* Associated symbol is in common */
#define SHN_HIRESERVE	0xFFFF	/* End range of reserved indices */

/* Auxiliary vector table types */
#define AT_NULL		0
#define AT_IGNORE	1
#define AT_EXECFD	2
#define AT_PHDR		3
#define AT_PHENT	4
#define AT_PHNUM	5
#define AT_PAGESZ	6
#define AT_BASE		7
#define AT_FLAGS	8
#define AT_ENTRY	9
#define AT_NOTELF	10
#define AT_UID		11
#define AT_EUID		12
#define AT_GID		13
#define AT_EGID		14
#define AT_CLKTCK	17
#define AT_PLATFORM	15
#define AT_HWCAP	16
#define AT_FPUCW	18
#define AT_DCACHEBSIZE	19
#define AT_ICACHEBSIZE	20
#define AT_UCACHEBSIZE	21
#define AT_IGNOREPPC	22
#define	AT_SECURE	23
#define AT_BASE_PLATFORM 24
#define AT_RANDOM	25
#define AT_HWCAP2	26
#define AT_EXECFN	31


#endif /* PROCESS_ELF_64_H_ */
