/*
 * File MKCORE utility - Generate an ELF corefile
 *
 * Copyright (C) 2012 Broadcom Corporation
 *
 * $Id: mkcore.c 241182 2011-02-17 21:50:03Z $
 */

#include <sys/types.h>

#include <elf.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>

#include "hnd_armtrap.h"
#include "hnd_debug.h"

#define READ_SIZE 4096
#define NHDR_NAME "REG"

#define BUFSIZE	512

static char *progname;

const uint32 dump_info_ptr_ptr[] = {DUMP_INFO_PTR_PTR_LIST};

static void
usage(void)
{
	fprintf(stderr, "Usage: %s [-rom <roml.bin>] [-d <dumpfile>] [-raw] [-noinfo]\n"
		        "          [-rambase <hex number>] [-rombase <hex number>] <elf file>\n"
		"\tif <roml.bin> is missing then only RAM is produced\n"
		"\tif <dumpfile> is producing using \"dlcmd d\" and defaults to stdin\n"
		"\tif -raw disables the prescan of the input for the start of the dump data\n"
		"\tif -noinfo skips the debug info and uses the filesize as the ramsize\n"
		"\tif -rambase sets the default ram base addr (usefule with -noinfo)\n"
		"\tif -rombase sets the default rom base addr (usefule with -noinfo)\n",
		progname);
	exit(-1);
}

int
main(int argc, char *argv[])
{
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;
	Elf32_Nhdr nhdr;

	prstatus_t status;

	hnd_debug_t debugInfo;

	unsigned char c;

	int i;

	int raw = 0;
	int noinfo = 0;

	char *filename = NULL;

	FILE *dump = stdin;
	FILE *rom = NULL;
	FILE *core = NULL;

	uint32 ram_base = 0xffffffff;
	uint32 rom_base = 0xffffffff;

	/* Process args */
	progname = argv[0];

	argv++;
	while (*argv) {
		if (strcmp(*argv, "-help") == 0) {
			usage();

		} else if (strcmp(*argv, "-rom") == 0) {
			argv++;
			rom = fopen(*argv, "r");
			if (rom == NULL) {
				fprintf(stderr, "%s: Cannot open rom file - %s\n", progname, *argv);
				usage();
			}

		} else if (strcmp(*argv, "-d") == 0) {
			argv++;
			dump = fopen(*argv, "r");
			if (dump == NULL) {
				fprintf(stderr, "%s: Cannot open dump file - %s\n",
					progname, *argv);
				usage();
			}

		} else if (strcmp(*argv, "-raw") == 0) {
			/* Raw mode assume input starts with coredump
			 * and not marker of 20 "X"
			*/
			raw = 1;

		} else if (strcmp(*argv, "-noinfo") == 0) {
			noinfo = 1;

		} else if (strcmp(*argv, "-rambase") == 0) {
			argv++;
			ram_base = strtoul(*argv, 0, 16);

		} else if (strcmp(*argv, "-rombase") == 0) {
			argv++;
			rom_base = strtoul(*argv, 0, 16);

		} else {
			filename = *argv;
			core = fopen(filename, "wb");
			if (core == NULL) {
				fprintf(stderr, "%s: Cannot open core file - %s\n",
					progname, *argv);
				usage();
				break;
			}
		}
		argv++;
	}

	if (filename == NULL) {
		fprintf(stderr, "%s: Core file name required\n\n", progname);
		usage();
	}

	if (raw == 0) {
		char outbuf[1024];
		int totCount = 0;
		/* The dlcmd dump command puts 20 Xs in the output to mark the
		 * start of the actual dump area.  We print out anything up to
		 * that since this could be run as a filter.
		 */
		int scanCount = 0;
		while (1) {
			if ((totCount + 20) >= sizeof(outbuf)) {
				fprintf(stderr, "Failed to find start of dump"
					" in first %i characters - resetting"
					" to RAW mode\n", sizeof(outbuf));
				raw = 1;
				break;
			}
			c = fgetc(dump);
			if (c == 'X') {
				scanCount++;
			} else {
				/* Dump out the accumulated Xs and the non-X char */
				for (i = 0; i < scanCount; i++) {
					outbuf[totCount++] = 'X';
				}
				outbuf[totCount++] = c;
				scanCount = 0;
			}

			if (scanCount == 20) {
				fwrite(outbuf, sizeof(char), totCount, stdout);
				break;
			}
		}
	}

	if (raw == 0) {
		/* Found 20 X - now get the debug area info size */
		int debugSize = 0;
		for (i = 0; i < 8; i++) {
			debugSize = (debugSize << 4) + (fgetc(dump) & 0x0f);
		}

		if (debugSize != sizeof(hnd_debug_t)) {
			fprintf(stderr, "%s: Debug size mismatch\n", progname);
			exit(-1);
		}

		/* Now read the actual debugInfo */
		fread(&debugInfo, sizeof(char), sizeof(debugInfo), dump);

	} else {
		/* Raw mode - have to find the debug area by hand */
		hnd_debug_ptr_t debugPtr;
		int32 dumpsize;

		fseek(dump, 0, SEEK_END);
		dumpsize = ftell(dump);
		fseek(dump, 0, SEEK_SET);
		for (i = 0; ; i++) {
			if (dump_info_ptr_ptr[i] == DUMP_INFO_PTR_PTR_END) {
				if (noinfo == 0) {
					fprintf(stderr, "%s: Debug area pointer "
					        "not found - assuming noinfo\n", progname);
				}
				bzero(&debugInfo, sizeof(debugInfo));
				noinfo = 1;
				break;
			}
			fseek(dump, dump_info_ptr_ptr[i], SEEK_SET);
			fread(&debugPtr, sizeof(char), sizeof(debugPtr), dump);
			if (debugPtr.magic == HND_DEBUG_PTR_PTR_MAGIC) {
				int32 seekoffset[2];
				int  i;

				/* for new firmware there is a value in: "debugPtr.ram_base_addr" */
				/* for old firmware there is no value there */
				seekoffset[0] = debugPtr.hnd_debug_addr - debugPtr.ram_base_addr;
				seekoffset[1] = debugPtr.hnd_debug_addr;
				for (i = 0; i < 2; i++) {
					if ((seekoffset[i] >= 0) && (seekoffset[i] < dumpsize)) {
						fseek(dump, seekoffset[i], SEEK_SET);
						fread(&debugInfo, sizeof(char), sizeof(debugInfo),
							dump);
						if ((debugInfo.magic == HND_DEBUG_MAGIC) &&
							(debugInfo.version == HND_DEBUG_VERSION))
						  break;
					}
				}
				break;
			}
		}
	}

	/* Sanity check the area */
	if ((noinfo == 0) && ((debugInfo.magic != HND_DEBUG_MAGIC) ||
	                      (debugInfo.version != HND_DEBUG_VERSION))) {
		fprintf(stderr, "%s: Error: Invalid debug info area\n", progname);
		noinfo = 1;
	}

	uint32 ram_addr = debugInfo.ram_base;
	uint32 ram_size = debugInfo.ram_size;

	uint32 rom_addr = debugInfo.rom_base;
	uint32 rom_size = debugInfo.rom_size;

	if (noinfo) {
		/* No size - use the dump file size */
		fseek(dump, 0, SEEK_END);
		ram_size = ftell(dump);

		if (rom) {
			fseek(rom, 0, SEEK_END);
			rom_size = ftell(rom);
			fseek(rom, 0, SEEK_SET);
		}

		if (ram_base != 0xffffffff) {
			ram_addr = ram_base;
		}

		if (rom_base != 0xffffffff) {
			rom_addr = rom_base;
		}
	}

	printf("Rom base address: 0x%8.8x size: 0x%8.8x\n", rom_addr, rom_size);
	printf("RAM base address: 0x%8.8x size: 0x%8.8x\n", ram_addr, ram_size);

	if (raw == 0) {
		/* 8-bytes RAM size + NULL terminator. */
		char ram_size_str[8 + 1];

		/* Read the prstatus area */
		fread(&status, sizeof(char), sizeof(status), dump);

		/* Parse the ramsize for sanity */
		int fRamSize;
		fread(ram_size_str, sizeof(char), 8, dump);
		ram_size_str[8] = '\0';
		fRamSize = strtoul(ram_size_str, NULL, 16);

		if (fRamSize != ram_size) {
			fprintf(stderr, "%s: Error - Ram size in file does not match debugInfo\n",
				progname);
			exit(-1);
		}
	} else {
		trap_t armtrap;
		uint32 *reg;
		uint32 trap_ptr = (debugInfo.trap_ptr - debugInfo.ram_base);

		fseek(dump, trap_ptr, SEEK_SET);
		fread(&armtrap, sizeof(char), sizeof(trap_t), dump);
		rewind(dump);

		/* Populate the prstatus */
		status.si_signo = armtrap.type;
		reg = &armtrap.r0;
		for (i = 0; i < 15; i++, reg++) {
			status.uregs[i] = *reg;
		}
		status.uregs[15] = armtrap.epc;
	}

	int num_phdr = (rom == NULL) ? 2 : 3;

	/* Set up ehdr */
	bzero(&ehdr, sizeof(ehdr));

	ehdr.e_ident[EI_MAG0] = ELFMAG0;
	ehdr.e_ident[EI_MAG1] = ELFMAG1;
	ehdr.e_ident[EI_MAG2] = ELFMAG2;
	ehdr.e_ident[EI_MAG3] = ELFMAG3;

	ehdr.e_ident[EI_CLASS] = ELFCLASS32;
	ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr.e_ident[EI_VERSION] = EV_CURRENT;
	ehdr.e_ident[EI_OSABI] = ELFOSABI_LINUX;

	ehdr.e_type = ET_CORE;
	ehdr.e_machine = EM_ARM;
	ehdr.e_version = EV_CURRENT;
	ehdr.e_entry = 0;
	ehdr.e_phoff = sizeof(ehdr);
	ehdr.e_shoff = 0;
	ehdr.e_flags = 0;
	ehdr.e_ehsize = sizeof(ehdr);
	ehdr.e_phentsize = sizeof(phdr);
	ehdr.e_phnum = num_phdr;
	ehdr.e_shentsize = 0;
	ehdr.e_shnum = 0;
	ehdr.e_shstrndx = 0;

	/* Write the ehdr header to the file */
	if (fwrite(&ehdr, sizeof(unsigned char), sizeof(ehdr), core) != sizeof(ehdr)) {
		fprintf(stderr, "%s: error writing to file %s\n", progname, filename);
	}

	int offset = sizeof(ehdr) + num_phdr * sizeof(phdr);

	/* Init the notes program header */
	bzero(&phdr, sizeof(phdr));
	phdr.p_type = PT_NOTE;
	phdr.p_offset = offset;
	phdr.p_vaddr = 0;
	phdr.p_paddr = 0;
	phdr.p_filesz = sizeof(nhdr) + sizeof(NHDR_NAME) + sizeof(status);
	phdr.p_memsz = 0;
	phdr.p_align = 2;

	/* Write the notes header to the file */
	if (fwrite(&phdr, sizeof(unsigned char), sizeof(phdr), core) != sizeof(phdr)) {
		fprintf(stderr, "%s: error writing to file %s\n", progname, filename);
	}

	offset += phdr.p_filesz;

	/* Init the RAM load program header */
	bzero(&phdr, sizeof(phdr));
	phdr.p_type = PT_LOAD;
	phdr.p_offset = offset;
	phdr.p_vaddr = ram_addr;
	phdr.p_paddr = ram_addr;
	phdr.p_filesz = ram_size;
	phdr.p_memsz = ram_size;
	phdr.p_align = 2;

	/* Write the header to the file */
	if (fwrite(&phdr, sizeof(unsigned char), sizeof(phdr), core) != sizeof(phdr)) {
		fprintf(stderr, "%s: error writing to file %s\n", progname, filename);
	}

	offset += phdr.p_filesz;

	if (rom != NULL) {
		/* Init the ROM load program header */
		bzero(&phdr, sizeof(phdr));
		phdr.p_type = PT_LOAD;
		phdr.p_offset = offset;
		phdr.p_vaddr = rom_addr;
		phdr.p_paddr = rom_addr;
		phdr.p_filesz = rom_size;
		phdr.p_memsz = rom_size;
		phdr.p_align = 2;

		/* Write the header to the file */
		if (fwrite(&phdr, sizeof(unsigned char), sizeof(phdr), core) != sizeof(phdr)) {
			fprintf(stderr, "error writing to file %s\n", filename);
		}

		offset += phdr.p_filesz;
	}

	/* Write the notes data section */
	nhdr.n_namesz = sizeof(NHDR_NAME);
	nhdr.n_descsz = sizeof(status);
	nhdr.n_type = NT_PRSTATUS;

	if ((fwrite(&nhdr, sizeof(unsigned char), sizeof(nhdr), core) != sizeof(nhdr)) ||
	    (fwrite(NHDR_NAME, sizeof(unsigned char), sizeof(NHDR_NAME), core) !=
	     sizeof(NHDR_NAME)) ||
	    (fwrite(&status, sizeof(unsigned char), sizeof(status), core) !=
	     sizeof(status))) {
		fprintf(stderr, "%s: error writing to file %s\n", progname, filename);
	}

	unsigned char *buf = malloc(BUFSIZE);
	if (buf == NULL) {
		fprintf(stderr, "%s: malloc failure: %s\n", progname, strerror(errno));
		exit(-1);
	}

	/* Read and write RAM in BUFSIZE sized chunks */
	int totlen = ram_size;
	while (totlen > 0) {
		int len = totlen > BUFSIZE  ? BUFSIZE  : totlen;

		if (fread(buf, sizeof(char), len, dump) != len) {
			fprintf(stderr, "%s: Error reading ram at 0x%x\n", progname, ram_addr);
			exit(-1);
		}

		if (fwrite(buf, sizeof(unsigned char), len, core) != len) {
			fprintf(stderr, "%s: error writing to file %s\n", progname, filename);
			exit(-1);
		}

		totlen -= len;
		ram_addr += len;
	}


	if (rom != NULL) {
		/* Read and write ROM in BUFSIZE sized chunks */
		totlen = rom_size;
		while (totlen > 0) {
			int len = totlen > BUFSIZE ? BUFSIZE : totlen;

			if (fread(buf, sizeof(char), len, rom) != len) {
				fprintf(stderr, "%s: Error reading rom at 0x%x\n",
					progname, rom_addr);
				exit(-1);
			}

			if (fwrite(buf, sizeof(unsigned char), len, core) != len) {
				fprintf(stderr, "%s: error writing to file %s\n",
					progname, filename);
				exit(-1);
			}

			totlen -= len;
			rom_addr += len;
		}

		fclose(rom);
	}

	fclose(dump);
	fclose(core);

	return (0);
}
