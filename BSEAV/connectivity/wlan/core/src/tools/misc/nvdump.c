/*
 * NVRAM dump utility
 *
 * Copyright (C) 2012 Broadcom Corporation
 *
 * $Id: nvdump.c 355634 2012-09-07 16:52:58Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <typedefs.h>
#include <bcmendian.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <sbsdram.h>

#include "lzma_src/C/LzmaLib.h"
#include "lzma_src/C/LzmaDec.h"


/* LZMA need to be able to allocate memory,
 * so set it up to use the OSL memory routines,
 * only the linux debug osl uses the osh on malloc and the osh and size on
 * free, and the debug code checks if they are valid, so pass NULL as the osh
 * to tell the OSL that we don't have a valid osh
 */
static void *SzAlloc(void *p, size_t size) { p = p; return malloc(size); }
static void SzFree(void *p, void *address) { p = p; free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };
#define	NVAR_TUPLES	300

/* once upon a time this used lex */
static void usage(void);
static void error(char *msg);


void dump_nvram(unsigned char *input_nvram)
{
	unsigned char nvram[NVRAM_SPACE] = { 0 };
	unsigned char *cp;
	unsigned int dstlen;
	unsigned int srclen = MAX_NVRAM_SPACE-LZMA_PROPS_SIZE-NVRAM_HEADER_SIZE;
	CLzmaDec state;
	SRes res;
	ELzmaStatus status;
	struct nvram_header *header;

	header = (struct nvram_header *)input_nvram;
	dstlen = header->len;
	cp = input_nvram + sizeof(struct nvram_header);
	if ((cp[0] == 0x5d) && (cp[1] == 0) && (cp[2] == 0)) {

		LzmaDec_Construct(&state);
		res = LzmaDec_Allocate(&state, (const Byte *)cp, LZMA_PROPS_SIZE, &g_Alloc);
		if (res != SZ_OK) {
			printf("Error Initializing LZMA Library\n");
			return;
		}
		LzmaDec_Init(&state);
		res = LzmaDec_DecodeToBuf(&state,
		                          (unsigned char *)nvram, &dstlen,
		                          (const Byte *)&cp[LZMA_PROPS_SIZE], &srclen,
		                          LZMA_FINISH_ANY,
		                          &status);

			LzmaDec_Free(&state, &g_Alloc);
			if (res != SZ_OK) {
				printf("Error Decompressing eNVRAM\n");
				return;
			}
	} else {
		memcpy(nvram, cp, sizeof(nvram));
	}

	cp = nvram;
	while (cp && *cp) {
		printf("%s\n", cp);
		cp = (unsigned char*)strchr((char *)cp, 0);
		if (cp)
			cp++;
	}
}

static void
usage(void)
{
	fprintf(stderr, "usage: nvdump [file]\n");
}

static void
error(char *msg)
{
	if (errno)
		fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	else
		fprintf(stderr, "%s\n", msg);
	usage();
	exit(errno ? errno : 1);
}

int main(int argc, char **argv)
{
	FILE *in = NULL;
	int ret = 0;
	unsigned char input_nvram[NVRAM_SPACE] = { 0 };


	/* Skip program name */
	--argc;
	++argv;

	if (argc == 0) {
		usage();
		ret = -1;
		goto out;
	}

	/* Parse command line arguments */
	for (; *argv; argv++) {
		if (strcmp(*argv, "-h") == 0 ||
		    strcmp(*argv, "--help") == 0) {
			usage();
		} else if (!(in = fopen((*argv), "r")))
			error(*argv);
	}

	fseek(in, 0x400, SEEK_SET);
	fread(input_nvram, sizeof(input_nvram), 1,  in);
	if (*(unsigned long *)input_nvram == NVRAM_MAGIC) {
		dump_nvram(input_nvram);
		goto out;
	}

	fseek(in, 0x1000, SEEK_SET);
	fread(input_nvram, sizeof(input_nvram), 1,  in);
	if (*(unsigned long *)input_nvram == NVRAM_MAGIC) {
		dump_nvram(input_nvram);
		goto out;
	}

	printf("Cound not find a valid envram\n");

out:
	if (in)
		fclose(in);

	return ret;
}
