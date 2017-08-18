/*
 * NVRAM initialization utility
 *
 * NVRAM text file format:
 *
 * Blank lines and sh style comments ('#') are ignored. Variable
 * assignment is of the form "name=value". A newline ('\n') signifies
 * the end of variable assignment and is replaced with a single NUL
 * ('\0'). Variable values may be referred to by delimiting the
 * variable name with "${ ... }". Variable values are retrieved from
 * the enviroment or from previous assignments. If a variable value is
 * undefined, "" is assumed.
 *
 * Options:
 *
 * This utility parses the NVRAM text file named on the command line
 * and translates it into binary NVRAM data. If no NVRAM file is
 * specified, no values are assigned and only a NVRAM header will be
 * produced. If "-s <serno>" is specified, the variables ${serno}
 * ${maclo} and ${maclo12} will be made available for use in the NVRAM
 * text file. The format of ${maclo} is "xx:xx", a hex representation
 * of <serno>, ${maclo12} is 12 bits only represented as "x:xx".
 *
 * If "-o <output>" is specified, the utility will write the binary
 * NVRAM data to <output> instead of the default stdout. If "-i
 * <input>" is specified, <input> will be copied to <output> until the
 * byte offset specified by "-b <offset>" is reached. If <offset> is
 * greater than the length of <input>, a warning will be issued and no
 * NVRAM data will be copied to <output>. Otherwise, <count> bytes of
 * binary NVRAM data will be copied to <output> in place of <count>
 * bytes of <input>. The rest of <input> (starting <count> bytes
 * later) will also be copied to <output>, thus "embedding" the NVRAM
 * in the output file.
 *
 * Copyright (C) 2001 Broadcom Corporation
 *
 * $Id$
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


#define	NVAR_TUPLES	2400

/* once upon a time this used lex */
static int yylex(void);
static char *yyfn;
static FILE *yyin;
static char yytext[NVRAM_SPACE];
static int yylineno = 0;
static void eval(char *str);

static char *get(char *name);
static void set(char *name, char *value, int overwrite);
static struct nvram_tuple vars[NVAR_TUPLES] = { { 0, 0, 0 } };

static void hdrinit(void);
static char nvram[MAX_NVRAM_SPACE] = { 0 };			/* to contain binary nvram data */
static char *end = &nvram[NVRAM_HEADER_SIZE];
struct nvram_header *hdr = (struct nvram_header *)nvram;
static char output_nvram[MAX_NVRAM_SPACE] = { 0 };

static int verbose = 0;
static int compress = 0;
static void usage(void);
static void error(char *msg);

/* writes into global buffer nvram[], at offset pointed to by global pointer 'end' */
static void
eval(char *str)
{
	char *c = str;
	char *name, *value;
	char buf[4096] = "";

	/* parse ${name} anywhere in the string */
	for (;;) {
		if (!(name = strstr(c, "${"))) {
			strcat(buf, c);
			break;
		}
		*name = '\0';
		strcat(buf, c);
		if (!(c = strchr(&name[2], '}'))) {
			fprintf(stderr, "warning: missing close-brace ('}') in %s line %d\n",
			        yyfn, yylineno);
			return;
		}
		*c++ = '\0';
		if ((value = get(&name[2])))
			strcat(buf, value);
		else
			fprintf(stderr, "warning: \"%s\" undefined in %s line %d\n",
			        &name[2], yyfn, yylineno);
	}

	/* parse name=value */
	if (!(c = strchr(buf, '='))) {
		fprintf(stderr, "warning: syntax error in %s line %d\n", yyfn, yylineno);
		return;
	}
	*c++ = '\0';
	name = buf;
	value = c;
	end += sprintf(end, "%s=%s", name, value) + 1;
	set(name, value, 1);
}

/* text nvram data is parsed into binary nvram data and stored in global buffer 'nvram[]' */
static int
yylex(void)
{
	char *c;
	int i, len;

	while (fgets(yytext, sizeof(yytext), yyin)) {
		yylineno++;

		len = strlen(yytext);
		/* chop all white spaces from end of the line */
		for (i = 0, c = (yytext + len - 1); i < len && isspace(*c); i++, c--)
			*c = '\0';

		/* eat blank lines and comments */
		if (!*yytext || *yytext == '#')
			continue;
		/* evaluate variables */
		eval(yytext);
	}

	return 0;
}

static char *
get(char *name)
{
	struct nvram_tuple *t;

	for (t = vars; t < vars + NVAR_TUPLES && t->name; t++) {
		if (strcmp(name, t->name) == 0)
			return t->value;
	}
	return NULL;
}

static void
set(char *name, char *value, int overwrite)
{
	struct nvram_tuple *t;

	for (t = vars; t < vars + NVAR_TUPLES && t->name; t++) {
		if (strcmp(name, t->name) == 0) {
			if (overwrite) {
				free(t->name);
				free(t->value);
				break;
			} else
				return;
		}
	}

	if (t == (vars + NVAR_TUPLES)) {
		printf("Exceeding max tuples: %d, Try increasing\n", NVAR_TUPLES);
		return;
	}

	t->name = strdup(name);
	t->value = strdup(value);
}

static void
hdrinit()
{
	char *names[] = { "sdram_init", "sdram_config", "sdram_refresh", "sdram_ncdl", NULL };
	char **name, *strval;
	unsigned int values[4] = { 0 }, *value;

	for (name = names, value = values; *name; name++, value++)
		if ((strval = get(*name)) != NULL)
			*value = strtoul(strval, NULL, 0);

	memset(hdr, 0, sizeof(struct nvram_header));
	hdr->magic = NVRAM_MAGIC;

	hdr->crc_ver_init =
	        (((NVRAM_VERSION) << 8) & 0xff00) | ((values[0] << 16) & 0xffff0000);

	hdr->config_refresh = (values[1] & 0xffff) | ((values[2] << 16) & 0xffff0000);
	hdr->config_ncdl = values[3];
}

static void
usage(void)
{
	fprintf(stderr, "usage: nvserial [options] [file]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "	-h, --help            This message\n");
	fprintf(stderr, "	-v, --verbose         Dump more information to stderr\n");
	fprintf(stderr, "	-r, --ramnvram        Create RAM nvram image\n");
	fprintf(stderr, "	-i, --input  <input>  Input stream (default none)\n");
	fprintf(stderr, "	-o, --output <output> Output stream (default stdout)\n");
	fprintf(stderr, "	-b, --offset <offset> Offset within output to embed NVRAM "
	        "(default 0x400)\n");
	fprintf(stderr, "	-c, --count  <count>  Bytes of NVRAM to write (default 0x2000)\n");
	fprintf(stderr, "	-a, --standalone      Create standalone nvram image from text "
	        "file\n");
	fprintf(stderr, "	-z, --compress        Compress NVRAM area "
	        "file\n");
	fprintf(stderr, "	-n, --no-hdr-warn     Do not warn on missing hdr values\n");
	fprintf(stderr, "	-s, --serno  <serno>  Sets the variables \"serno\" \"maclo\" "
	        "and \"maclo12\" (optional)\n");
	fprintf(stderr, "	-l, --lengthappend    Appends length to serialized NVRAM file\n");
	fprintf(stderr, "	-p, --placeinram  <mem_size>  Input RAM size. Outputs RAM location"
	        " where serialized NVRAM should be written.\n");
	fprintf(stderr, "	-V, --version         Build date and time\n");
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
	FILE *in = NULL, *out = stdout;
	char buf[] = "xx:xx";
	unsigned short maclo = 0;
	unsigned int
	long pos = 0, offset = 0;
	int c = '\0';
	int i, count = 0, ret = 0;
	int swap = 0;
	uint32 tmp;
	int standalone = 0, ram_nvram = 0;
	struct nvram_header dummy_hdr;
	int len_app = 0;
	int mem_size = 0;
	int nvram_size;

	/* No NVRAM data file by default */
	yyin = NULL;

	/* Skip program name */
	--argc;
	++argv;

	if (argc == 0) {
		usage();
		ret = -1;
		goto out;
	}

	if ((argc == 1) && !strcmp(*argv, "-V")) { 
		printf ("%s %s \n", __DATE__, __TIME__); 
		goto out; 
	} 

	/* Parse command line arguments */
	for (; *argv; argv++) {
		if (strcmp(*argv, "-h") == 0 ||
		    strcmp(*argv, "--help") == 0) {
			usage();
			goto out;
		} else if (strcmp(*argv, "-i") == 0 ||
		           strcmp(*argv, "--input") == 0) {
			if (!*++argv)
				error("-i: missing argument");
			if (!(in = fopen(*argv, "rb")))
				error(*argv);
		} else if (strcmp(*argv, "-o") == 0 ||
		           strcmp(*argv, "--output") == 0) {
			if (!*++argv)
				error("-o: missing argument");
			if (!(out = fopen(*argv, "wb")))
				error(*argv);
		} else if (strcmp(*argv, "-b") == 0 ||
		           strcmp(*argv, "--offset") == 0) {
			if (!*++argv)
				error("-b: missing argument");
			offset = strtoul(*argv, NULL, 0);
		} else if (strcmp(*argv, "-c") == 0 ||
		           strcmp(*argv, "--count") == 0) {
			if (!*++argv)
				error("-c: missing argument");
			count = (int)strtoul(*argv, NULL, 0);
			if (count > NVRAM_SPACE) {
				fprintf(stderr, "error: NVRAM length greater than %d\n",
				        NVRAM_SPACE);
				ret = -2;
				goto out;
			}
		} else if (strcmp(*argv, "-s") == 0 ||
		           strcmp(*argv, "--serno") == 0) {
			if (!*++argv)
				error("-s: missing argument");
			set("serno", *argv, 1);
			maclo = (uint16)strtoul(*argv, NULL, 0);
			sprintf(buf, "%02x:%02x", (maclo >> 8) & 0xff, maclo & 0xff);
			set("maclo", buf, 1);
			sprintf(buf, "%1x:%02x", (maclo >> 8) & 0xf, maclo & 0xff);
			set("maclo12", buf, 1);
		} else if (strcmp(*argv, "-v") == 0 ||
		           strcmp(*argv, "--verbose") == 0) {
			verbose = 1;
		} else if (strcmp(*argv, "-z") == 0 ||
		           strcmp(*argv, "--compress") == 0) {
			compress = 1;
		} else if (strcmp(*argv, "-r") == 0 ||
		           strcmp(*argv, "--ramnvram") == 0) {
			ram_nvram = 1;
		} else if (strcmp(*argv, "-a") == 0 ||
		           strcmp(*argv, "--standalone") == 0) {
			standalone = 1;
		} else if (strcmp(*argv, "-p") == 0 ||
		           strcmp(*argv, "--placeinram") == 0) {
			if (!*++argv)
				error("-l: missing RAM size");
			mem_size = (int)strtoul(*argv, NULL, 0);
			printf("Memory size = %d bytes\n", mem_size);
		} else if (strcmp(*argv, "-l") == 0 ||
		           strcmp(*argv, "--lengthappend") == 0) {
			len_app = 1;
		} else if (!(yyin = fopen((yyfn = *argv), "r")))
			error(*argv);
	}

	if (standalone) {
		end = nvram;
		hdr = &dummy_hdr;
	}

	/* Parse NVRAM data file */
	if (yyin)
		yylex(); /* global buffer nvram[] now contains binary nvram data */

	/* Initialize NVRAM header */
	hdrinit();
	/* (end - nvram) + 1 to ensure the double null byte is written */
	hdr->len = (uint32)ROUNDUP((end - nvram) + 1, 4);
	hdr->crc_ver_init |= hndcrc8((unsigned char *)(nvram + 9),
	                             hdr->len - 9, CRC8_INIT_VALUE) & 0xff;

	/* write image including nvram header */
	if (ram_nvram) {
		uint32 *src, *dst;
		uint32 i;

		src = dst = (uint32 *) hdr;
		for (i = 0; i < hdr->len && i < NVRAM_SPACE; i += 4)
			*dst++ = htol32(*src++);

		if (out)
			fwrite(hdr, hdr->len, 1, out);
		return 0;
	}

	/* Find NVRAM space */
	if (in && !offset && !count && !standalone) {
		while (fread(&tmp, 4, 1, in) == 1) {
			if (tmp == ~NVRAM_MAGIC) {
				swap = 0;
				count += 4;
			} else if (tmp == BCMSWAP32(~NVRAM_MAGIC)) {
				swap = 1;
				count += 4;
			} else {
				if (count == 0)
					offset += 4;
				else if (count < sizeof(struct nvram_header))
					offset = count = 0;
				else {
					/*
					 * Check if this cfe image supports
					 * compressed envram, if not turn
					 * off the feature
					 */
					if (compress &&
					    (tmp != NVRAM_LZMA_MAGIC) &&
					    (tmp != BCMSWAP32(NVRAM_LZMA_MAGIC))) {
						fprintf(stderr,
						        "CFE Does not support compression\n");
						compress = 0;
					}
					break;
				}
			}
		}
		if (count < sizeof(struct nvram_header))
			offset = count = 0;
		rewind(in);
	}

	/* Use defaults */
	if (standalone)
		count = hdr->len;
	else if (!offset && !count) {
		offset = 0x400;
		count = 0x2000 - 4;
	} else {
		/*
		 * We retain last 4 bytes for marker ~NVRAM_MAGIC
		 * which helps us identify the size of envram region
		 * where ever required.
		 */
		count -= 4;
	}

	nvram_size = hdr->len;
	memset(output_nvram, 0, sizeof(output_nvram));

	if (compress) {
		unsigned int propsize;
		unsigned int destlen;
		unsigned char *cp;
		int res;

		memcpy(output_nvram, nvram, NVRAM_HEADER_SIZE);

		propsize = LZMA_PROPS_SIZE;

		destlen = sizeof(output_nvram)-NVRAM_HEADER_SIZE;
		cp = (unsigned char *)output_nvram+NVRAM_HEADER_SIZE;

		res = LzmaCompress(&cp[LZMA_PROPS_SIZE], &destlen,
		                   (unsigned char *)&nvram[NVRAM_HEADER_SIZE],
		                   nvram_size-NVRAM_HEADER_SIZE,
		                   &cp[0], &propsize,
		                   -1, 1<<16, -1, -1, -1, -1, -1);
		if (res != SZ_OK) {
			fprintf(stderr, "NVRAM compression error\n");
			ret = -1;
		goto out;

		}
		nvram_size = destlen+NVRAM_HEADER_SIZE+propsize;


	} else {
		memcpy(output_nvram, nvram, sizeof(nvram));
	}


	if (nvram_size > (uint32)count) {
		fprintf(stderr, "NVRAM contents is %d byte, does not fit in output file (%d)\n",
		        nvram_size, count);
		ret = -1;
		goto out;
	}

	if (len_app) {
		uint32 lenword = ((~(hdr->len/4) << 16) | (hdr->len/4));
		*(uint32 *)(output_nvram + count) = lenword;
		count += 4;
	}

	if (mem_size) {
		printf("NVRAM byte count %d\n", count);
		printf("Place NVRAM at 0x%x\n", mem_size - count);
	}

	for (;;) {
		if ((pos = ftell(out)) == offset) {
			/* Skip over NVRAM data in input stream */
			if (in) {
				for (i = 0; i < count; i++)
					if ((c = getc(in)) == EOF)
						break;
			}
			/* Embed NVRAM data in output stream */
			for (i = 0; i < count; i += 4) {
				tmp = *((uint32 *) &output_nvram[i]);
				if (swap)
					tmp = BCMSWAP32(tmp);
				fwrite(&tmp, 4, 1, out);
			}
		} else {
			if ((in && (c = getc(in)) == EOF) ||
			    (!in && (pos > offset)))
				break;
			putc(c, out);
		}
		if (ferror(out)) {
			fprintf(stderr, "Error writing output file\n");
			ret = -1;
			goto out;
		}
	}

	if (pos < offset)
		fprintf(stderr, "warning: input length %ld less than offset length %ld\n",
		        pos, offset);

	if (verbose) {
		fprintf(stderr, "offset: 0x%lx\n", offset);
		if (len_app == 1)
			fprintf(stderr, "count: 0x%x\n", count - 4);
		else
			fprintf(stderr, "count: 0x%x\n", count);
		fprintf(stderr, "magic: 0x%08x\n", hdr->magic);
		fprintf(stderr, "len: 0x%08x (0x%08x)\n", hdr->len, nvram_size);
		fprintf(stderr, "crc_ver_init: 0x%08x\n", hdr->crc_ver_init);
		fprintf(stderr, "config_refresh: 0x%08x\n", hdr->config_refresh);
		for (end = &nvram[NVRAM_HEADER_SIZE]; *end; end += strlen(end) + 1)
			fprintf(stderr, "%s\n", end);
	}

out:
	if (in)
		fclose(in);
	if (out)
		fclose(out);
	return ret;
}
