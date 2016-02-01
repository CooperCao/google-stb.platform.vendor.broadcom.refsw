/***************************************************************************
 * bafl_elf.h
 *
 * Standard (common) definitions for ELF structures/fields.
 *
 * All of this material comes out of the ELF 1.1 Specification.
 * Metaware ARC-specific definitions are below.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   See module
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef _BAFL_ELF_H__
#define _BAFL_ELF_H__

typedef uint32_t   Bafl_Elf32_Addr;
typedef uint16_t   Bafl_Elf32_Half;
typedef uint32_t   Bafl_Elf32_Off;
typedef int32_t    Bafl_Elf32_Sword;
typedef uint32_t   Bafl_Elf32_Word;

#define ET_NIDENT       16

typedef struct
{
      unsigned char   e_ident[ET_NIDENT];
      Bafl_Elf32_Half e_type;
      Bafl_Elf32_Half e_machine;
      Bafl_Elf32_Word e_version;
      Bafl_Elf32_Addr e_entry;
      Bafl_Elf32_Off  e_phoff;
      Bafl_Elf32_Off  e_shoff;
      Bafl_Elf32_Word e_flags;
      Bafl_Elf32_Half e_ehsize;
      Bafl_Elf32_Half e_phentsize;
      Bafl_Elf32_Half e_phnum;
      Bafl_Elf32_Half e_shentsize;
      Bafl_Elf32_Half e_shnum;
      Bafl_Elf32_Half e_shstrndx;
} BAFL_Elf32_Ehdr;

        /* e_type definitions */
#define BAFL_ET_NONE         0       /* No file type         */
#define BAFL_ET_REL          1       /* Relocatable file     */
#define BAFL_ET_EXEC         2       /* Executable file      */
#define BAFL_ET_DYN          3       /* Shared object file   */
#define BAFL_ET_CORE         4       /* Core file            */
#define BAFL_ET_LOPROC       0xFF00  /* Processor-specific   */
#define BAFL_ET_HIPROC       0xFFFF  /* Processor-specific   */

        /* e_machine definitions */
#define BAFL_EM_NONE         0       /* No machine           */
#define BAFL_EM_M32          1       /* AT&T WE 32100        */
#define BAFL_EM_SPARC        2       /* SPARC                */
#define BAFL_EM_386          3       /* Intel 80386          */
#define BAFL_EM_68K          4       /* Motorola 68000       */
#define BAFL_EM_88K          5       /* Motorola 88000       */
#define BAFL_EM_860          7       /* Intel 80860          */
#define BAFL_EM_MIPS         8       /* MIPS RS3000          */
#define BAFL_EM_ARC          45      /* Argonaut Risc Core   */

        /* e_version definitions */
#define BAFL_EV_NONE         0       /* Invalid version      */
#define BAFL_EV_CURRENT      1       /* Current version      */

        /* e_ident[] fields */
#define BAFL_EI_MAG0         0       /* File identification  */
#define BAFL_EI_MAG1         1       /* File identification  */
#define BAFL_EI_MAG2         2       /* File identification  */
#define BAFL_EI_MAG3         3       /* File identification  */
#define BAFL_EI_CLASS        4       /* File class           */
#define BAFL_EI_DATA         5       /* Data encoding        */
#define BAFL_EI_VERSION      6       /* File version         */
#define BAFL_EI_PAD          7       /* Start of padding     */

#define BAFL_ELFMAG0         0x7F
#define BAFL_ELFMAG1         'E'
#define BAFL_ELFMAG2         'L'
#define BAFL_ELFMAG3         'F'

#define BAFL_ELFCLASSNONE    0       /* Invalid class        */
#define BAFL_ELFCLASS32      1       /* 32-bit objects       */
#define BAFL_ELFCLASS64      2       /* 64-bit objects       */

#define BAFL_ELFDATANONE     0       /* Invalid data encoding        */
#define BAFL_ELFDATA2LSB     1       /* Little-endian encoding       */
#define BAFL_ELFDATA2MSB     2       /* Big-endian encoding          */

#define BAFL_SHN_UNDEF     0
#define BAFL_SHN_LORESERVE 0xff00
#define BAFL_SHN_LOPROC    0xff00
#define BAFL_SHN_HIPROC    0xff1f
#define BAFL_SHN_ABS       0xfff1
#define BAFL_SHN_COMMON    0xfff2
#define BAFL_SHN_HIRESERVE 0xffff

typedef struct
{      
      Bafl_Elf32_Word  sh_name;      /* Section name (string tbl index)      */
      Bafl_Elf32_Word  sh_type;      /* Section type                         */
      Bafl_Elf32_Word  sh_flags;     /* Section flags                        */
      Bafl_Elf32_Addr  sh_addr;      /* Section virtual addr at execution    */
      Bafl_Elf32_Off   sh_offset;    /* Section file offset                  */
      Bafl_Elf32_Word  sh_size;      /* Section size in bytes                */
      Bafl_Elf32_Word  sh_link;      /* Link to another section              */
      Bafl_Elf32_Word  sh_info;      /* Additional section information       */
      Bafl_Elf32_Word  sh_addralign; /* Section alignment                    */
      Bafl_Elf32_Word  sh_entsize;   /* Entry size if section holds table    */
} BAFL_Elf32_Shdr;

#define BAFL_SHT_NULL        0
#define BAFL_SHT_PROGBITS    1
#define BAFL_SHT_SYMTAB      2
#define BAFL_SHT_STRTAB      3
#define BAFL_SHT_RELA        4
#define BAFL_SHT_HASH        5
#define BAFL_SHT_DYNAMIC     6
#define BAFL_SHT_NOTE        7
#define BAFL_SHT_NOBITS      8
#define BAFL_SHT_REL         9
#define BAFL_SHT_SHLIB       10
#define BAFL_SHT_DYNSYM      11
#define BAFL_SHT_LOPROC      0x70000000
#define BAFL_SHT_HIPROC      0x7FFFFFFF
#define BAFL_SHT_LOUSER      0x80000000
#define BAFL_SHT_HIUSER      0xFFFFFFFF

#define BAFL_SHF_WRITE       0x1
#define BAFL_SHF_ALLOC       0x2
#define BAFL_SHF_EXECINSTR   0x4
#define BAFL_SHF_MASKPROC    0xF0000000


typedef struct
{ 
      Bafl_Elf32_Word st_name;
      Bafl_Elf32_Addr st_value;
      Bafl_Elf32_Word st_size;
      unsigned char   st_info;
      unsigned char   st_other;
      Bafl_Elf32_Half st_shndx;
} Bafl_Elf32_Sym;

#define BAFL_ELF32_ST_BIND(i)   ((i) >> 4)
#define BAFL_ELF32_ST_TYPE(i)   ((i) & 0x0F)
#define BAFL_ELF32_ST_INFO(b,t) (((b) << 4) + ((t) & 0x0F))

#define BAFL_STB_LOCAL       0
#define BAFL_STB_GLOBAL      1
#define BAFL_STB_WEAK        2
#define BAFL_STB_LOPROC      13
#define BAFL_STB_HIPROC      15

#define BAFL_STT_NOTYPE      0
#define BAFL_STT_OBJECT      1
#define BAFL_STT_FUNC        2
#define BAFL_STT_SECTION     3
#define BAFL_STT_FILE        4
#define BAFL_STT_LOPROC      13
#define BAFL_STT_HIPROC      15

typedef struct
{ 
      Bafl_Elf32_Addr r_offset;
      Bafl_Elf32_Word r_info;
} Bafl_Elf32_Rel;

typedef struct
{
      Bafl_Elf32_Addr  r_offset;
      Bafl_Elf32_Word  r_info;
      Bafl_Elf32_Sword r_addend;
} Bafl_Elf32_Rela;

#define BAFL_ELF32_R_SYM(i)          ((i) >> 8)
#define BAFL_ELF32_R_TYPE(i)         ((unsigned char)(i))
#define BAFL_ELF32_R_INFO(s,t)       (((s) << 8) + (unsigned char)(t))


typedef struct
{ 
      Bafl_Elf32_Word p_type;
      Bafl_Elf32_Off  p_offset;
      Bafl_Elf32_Addr p_vaddr;
      Bafl_Elf32_Addr p_paddr;
      Bafl_Elf32_Word p_filesz;
      Bafl_Elf32_Word p_memsz;
      Bafl_Elf32_Word p_flags;
      Bafl_Elf32_Word p_align;
} Bafl_Elf32_Phdr;

#define BAFL_PT_NULL         0
#define BAFL_PT_LOAD         1
#define BAFL_PT_DYNAMIC      2
#define BAFL_PT_INTERP       3
#define BAFL_PT_NOTE         4
#define BAFL_PT_SHLIB        5
#define BAFL_PT_PHDR         6
#define BAFL_PT_LOPROC       0x70000000
#define BAFL_PT_HIPROC       0x7FFFFFFF

/*----------------------------------------------------------------------*
 *                                                                      *
 * The following is the elf_arc.h file from Metaware.                   *
 *                                                                      *
 *----------------------------------------------------------------------*/


/*********************************************************************
 * THE SOFTWARE CONTAINED IN THIS FILE IS LICENSED TO BROADCOMS 
 * CUSTOMERS FOR USE SOLELY IN CONNECTION WITH ARC processors
 * within BROADCOM'S PRODUCTS.
 * (C) Copyright 2005; ARC International (ARC);  Santa Cruz, CA 95060
 * This program is the unpublished property and trade secret of ARC.  It
 * is to be  utilized  solely  under  license and it is to be maintained 
 * on a confidential basis. The security  and  protection  of  the program
 * is paramount to maintenance of the trade secret status.  It is to be 
 * protected from disclosure to unauthorized parties, both within the 
 * Licensee company and outside, in a manner not less stringent than that
 * utilized for Licensee's own proprietary internal information.  
 *********************************************************************/

/**********************************************************************
* Relocation types for the ARC ELF object files
*   A = addend used to compute the relocation
*   S = The value of the symbol being relocated
*   P = The place (addr/section offset) of the storage unit being relocated
*       (computed using r_offset)
*
* Relocation Fields (note this is big-endian notation...)
*   +--------------------------------+
*   |31          bits31-0           0| word32 data in separate word
*   +--------------------------------+
*
*   +------------------------+
*   |23      bits23-0       0| bits24 data in separate 3-byte chunk
*   +------------------------+
*
*   +----------------+
*   |15  bits15-0   0| bits16 data in separate half-word
*   +----------------+
*
*   +-----------+
*   |7 bits7-0 0| bits8 data in separate byte
*   +-----------+
*
*   +--------------------------------+ 
*   |31     |23     bits25-2        0| targ26 data as part of an instruction
*   +--------------------------------+
*
*   +--------------------------------+
*   |31  |26    bits21-2     7|     0| disp22 data as part of an instruction
*   +--------------------------------+
*
*   +--------------------------------------------+
*   |31  |26 bits10-1  17| |15 bits20-11 6|     0| disp21h data in instruction
*   +--------------------------------------------+ (half-word aligned)
*
*   +--------------------------------------------+
*   |31  |26 bits10-2  18| |15 bits20-11 6|     0| disp21w data in instruction
*   +--------------------------------------------+ (long-word aligned)
*
*   +------------------------------------------------------+
*   |31  |26 bits10-1  17| |15 bits20-11 6|  |3 bits24-21 0| disp25h data
*   +------------------------------------------------------+ (halfword aligned)
*
*   +------------------------------------------------------+
*   |31  |26 bits10-2  18| |15 bits20-11 6|  |3 bits24-21 0| disp25w data
*   +------------------------------------------------------+ (longword aligned)
*
*   +--------------------------------+
*   |31                  9| bits8-0 0| disp9 as part of an instruction
*   +--------------------------------+
*
*   +--------------------------------+
*   |31 |23 bits7-0 16|15 bit8|14   0| disp9ls as part of an instruction
*   +--------------------------------+
*
*   +-------------------+
*   |15     9| bits8-0 0| disp9s as part of an instruction
*   +-------------------+
*
*   +----------------==-+
*   |15   11| bits10-0 0| disp13s as part of an instruction (longword aligned)
*   +----------------==-+
*
*********************************************************************/

/*      Relo Type Name        Value    Field    Calculation */
#define R_ARC_NONE            0x0   /* none     None */
#define R_ARC_8               0x1   /* bits8    S + A */
#define R_ARC_16              0x2   /* bits16   S + A */
#define R_ARC_24              0x3   /* bits24   S + A */
#define R_ARC_32              0x4   /* word32   S + A */
#define R_ARC_B26             0x5   /* targ26   (S + A) >> 2    
                                     *(convert to longword displacement) 
                                     */
#define R_ARC_B22_PCREL       0x6   /* disp22   (S + A - P) >> 2
				     *    (convert to longword displacement)
                                     */
#define R_ARC_H30             0x7   /* word32   (S + A) >> 2 */
#define R_ARC_N8              0x8   /* bits8    S - A */
#define R_ARC_N16             0x9   /* bits16   S - A */
#define R_ARC_N24             0xA   /* bits24   S - A */
#define R_ARC_N32             0xB   /* word32   S - A */
#define R_ARC_SDA             0xC   /* disp9    S + A */
#define R_ARC_SECTOFF         0xD   /* word32   (S - <start of section>) + A */

/*************************************************************************
 * following new relocations defined for the ARCompact ISA
 *************************************************************************/

/* for conditional branch. Example: bne   printf */
#define R_ARC_S21H_PCREL  0xE   /* disp21h  (S + A - P) >> 1 
                                 * (convert to halfword displacement) 
                                 */

/* for conditional branch and link. Example: blne  printf */
#define R_ARC_S21W_PCREL  0xF   /* disp21w  (S + A - P) >> 2
                                 * (convert to longword displacement) 
                                 */

/* For unconditional branch.  Example: b  printf */
#define R_ARC_S25H_PCREL  0x10   /* disp25h  (S + A - P) >> 1 
                                  * (convert to halfword displacement)
                                  */

/* For unconditional branch and link.  Example: bl  printf */
#define R_ARC_S25W_PCREL  0x11   /* disp25w  (S + A - P) >> 2
                                  * (convert to longword displacement)
                                  */

/* for 32-bit Small Data Area fixups.  Example: add   r0, gp, var@sda */
#define R_ARC_SDA32       0x12   /* word32   (S + A) - _SDA_BASE_ */

/* for small data fixups on loads and stores.  Examples:
 *     ldb   r0,  [gp, var@sda]   ; R_ARC_SDA_LDST
 *     stw   r0,  [gp, var@sda]   ; R_ARC_SDA_LDST1
 *     ld    r0,  [gp, var@sda]   ; R_ARC_SDA_LDST2
 */
#define R_ARC_SDA_LDST   0x13   /* disp9ls (S + A - _SDA_BASE_)  (s9 range) */
#define R_ARC_SDA_LDST1  0x14   /* disp9ls (S+A-_SDA_BASE_) >> 1 (s10 range) */
#define R_ARC_SDA_LDST2  0x15   /* disp9ls (S+A-_SDA_BASE_) >> 2 (s11 range) */

/* for 16-bit load gp-relative instruction. Example:
 *     ldb_s  r0, [gp, var@sda]   ; R_ARC_SDA16_LD
 *     ldw_s  r0, [gp, var@sda]   ; R_ARC_SDA16_LD1
 *     ld_s   r0, [gp, var@sda]   ; R_ARC_SDA16_LD2
 */
#define R_ARC_SDA16_LD   0x16   /* disp9s  (S + A - _SDA_BASE)   (s9 range) */
#define R_ARC_SDA16_LD1  0x17   /* disp9s  (S+A-_SDA_BASE_) >> 1 (s10 range) */
#define R_ARC_SDA16_LD2  0x18   /* disp9s  (S+A-_SDA_BASE_) >> 2 (s11 range) */

/* for 16-bit branch-and-link.  Example: bl_s  printf */
#define R_ARC_S13_PCREL      0x19   /* disp13s  (S + A - P) >> 2 */

/* for 32-bit alignment of the fixup value.  Examples:
 *     mov   r0,  var@l
 *     ld    r0, [pcl, lab - .@l]
 */
#define R_ARC_W              0x1a   /* word32   (S + A) & ~3   (word-align) */

/*
 * The following relocations are to support middle-endian storage, whereby
 * a 32-bit word is stored in two halfwords, with bits 31-16 stored first
 * in memory and bits 15-0 stored adjacently.  The individual half-words are
 * stored in the native endian of the machine.  This is how all instructions
 * and LIMMs are stored in the ARCompact architecture.
 */
#define R_ARC_32_ME          0x1b   /* Like ARC_32, but stored in ME format */
#define R_ARC_N32_ME         0x1c   /* Like N32, but stored in ME format */
#define R_ARC_SECTOFF_ME     0x1d   /* Like SECTOFF, but stored in ME format */
#define R_ARC_SDA32_ME       0x1e   /* Like SDA32, but stored in ME format */
#define R_ARC_W_ME           0x1f   /* Like W, but stored in ME format */
#define R_ARC_H30_ME         0x20   /* Like H30, but stored in ME format */

/*************************************************************************/

/*
 * for ARC4 ld/st instructions, allows a section-relative offset in the
 * range 0-255 (the positive portion of the shimm range).  The base
 * register must be loaded with the base address of the section. Example:
 * ld   r0, [r20, var@sectoff_u8] 
 */

#define R_ARC_SECTOFF_U8     0x21   / disp9 (S + A - <start of section>) */

/* ARC4. range -256 to 255.  The base register must be loaded with base of
 * section + 256. Example:
 *   ld   r0, [r20, var@sectoff_s9] 
 */
#define R_ARC_SECTOFF_S9  0x22   /* disp9 (S + A - <start of section> - 256) */

/*
 * Same semantics as R_ARC_SECTOFF_U8 above, but for ARCompact ISA
 * Note, however that the assembler encodes these loads and stores with
 * address scaling (.as) turned on so that the range for half-words is
 * 0-510 and the range for full 32-bit word accesses is 0-1020
 * The range for byte accesses remains 0-255
 *    ldb  r0, [r20, var@sectoff_u8]   ; R_AC_SECTOFF_U8
 *    stw  r0, [r20, var@sectoff_u8]   ; R_AC_SECTOFF_U8_1
 *    ld   r0, [r20, var@sectoff_u8]   ; R_AC_SECTOFF_U8_2
 */
#define R_AC_SECTOFF_U8   0x23   /* disp9ls (S + A - <start of section>) */
#define R_AC_SECTOFF_U8_1 0x24   /* disp9ls (S +A - <start of section>) >> 1 */
#define R_AC_SECTOFF_U8_2 0x25   /* disp9ls (S +A - <start of section>) >> 2 */

/*
 * Same semantics as R_ARC_SECTOFF_S9 above, but for ARCompact ISA
 * Note, however that the assembler encodes these loads and stores with
 * address scaling (.as) turned on so that the range for half-words is
 * -256-510 and the range for full 32-bit word accesses is -256-1020
 * The range for byte accesses remains -256-255
 *    ldb  r0, [r20, var@sectoff_s9]   ; R_AC_SECTOFF_S9
 *    stw  r0, [r20, var@sectoff_s9]   ; R_AC_SECTOFF_S9_1
 *    ld   r0, [r20, var@sectoff_s9]   ; R_AC_SECTOFF_S9_2
 */
#define R_AC_SECTOFF_S9   0x26   /* disp9ls (S + A - <start of section>) */
#define R_AC_SECTOFF_S9_1 0x27   /* disp9ls (S +A - <start of section>) >> 1 */
#define R_AC_SECTOFF_S9_2 0x28   /* disp9ls (S +A - <start of section>) >> 2 */

/* X/Y memory relocations */
#define R_ARC_SECTOFF_ME_1   0x29   /* word32  ((S - <start of section>) + A)
                                     * >> 1 [ME format]
                                     */

#define R_ARC_SECTOFF_ME_2   0x2a   /* word32  ((S - <start of section>) + A)
                                     * >> 2 [ME format]
                                     */

#define R_ARC_SECTOFF_1      0x2b   /* word32  ((S - <start of section>) + A)
                                     * >> 1
                                     */

#define R_ARC_SECTOFF_2      0x2c   /* word32  ((S - <start of section>) + A)
                                     * >> 2
                                     */

#endif
