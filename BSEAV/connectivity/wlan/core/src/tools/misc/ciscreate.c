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
 * produced. If "-s <serno>" is specified, the variables ${serno} and
 * ${maclo} will be made available for use in the NVRAM text file. The
 * format of ${maclo} is "xx:xx", a hex representation of <serno>.
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

#include <bcmendian.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <sbsdram.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <bcmsrom_fmt.h>

#include <bcmsrom_tbl.h>

/* once upon a time this used lex */
static int yylex(void);
static char *yyfn;
static FILE *yyin = NULL;
static char yytext[NVRAM_SPACE];
static int yylineno = 0;
static char tuples[NVRAM_SPACE];
static void eval(char *str);

static char *get(char *name);
static void set(char *name, char *value, int overwrite);
static struct nvram_tuple vars[256] = { { 0, 0, 0 } };

static char * cooked[256] = { 0 };
static int  cooked_cnt = 0;

static char nvram[NVRAM_SPACE] = { 0 };
static char *end = &nvram[NVRAM_HEADER_SIZE];
struct nvram_header *hdr = (struct nvram_header *)nvram;

static int verbose = 0;
static void usage(void);
static void error(char *msg);

enum
{
	USB,
	SDIO,
	UNSUPPORTED
};
static int bus = USB;
static char * tuple_buf = tuples;

int
wl_ether_atoe(const char *a, struct ether_addr *n)
{
	char *c = (char *) a;
	int i = 0;

	memset(n, 0, ETHER_ADDR_LEN);
	for (;;) {
		n->octet[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

static char *
get(char *name)
{
	struct nvram_tuple *t;

	for (t = vars; t->name && t < vars + sizeof(vars); t++) {
		if (strcmp(name, t->name) == 0)
			return t->value;
	}
	return NULL;
}

static void
set(char *name, char *value, int overwrite)
{
	struct nvram_tuple *t;

	for (t = vars; t->name && t < vars + sizeof(vars); t++) {
		if (strcmp(name, t->name) == 0) {
			if (overwrite) {
				free(t->name);
				free(t->value);
				break;
			} else
				return;
		}
	}
	t->name = strdup(name);
	t->value = strdup(value);
}

static void
eval(char *str)
{
	char *c = str;
	char *name, *value;
	char buf[4096] = "";
	char tbuf[16];
	int base;
	unsigned int tmp;

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

	if ((bus == SDIO) && (strcmp(buf, "hwhdr") == 0)) {
		memset(tbuf, 0, 16);
		if (strncmp(value, "0x", 2))
			base = 10;
		else {
			base = 16;
			value += 2;
		}
		while (*value) {
			strncpy(tbuf, value, 8);
			value += 8;
			tmp = (unsigned int) strtoul(tbuf, NULL, base);
			tuple_buf[3] = (unsigned char) (tmp & 0xff);
			tuple_buf[2] = (unsigned char) ((tmp>>8) & 0xff);
			tuple_buf[1] = (unsigned char) ((tmp>>16) & 0xff);
			tuple_buf[0] = (unsigned char) ((tmp>>24) & 0xff);
			tuple_buf += 4;
		}
		tuple_buf = tuples + 8;
	}
	else {
		cooked[cooked_cnt++] = end;
		end += sprintf(end, "%s=%s", name, value) + 1;
	}
}

static int
yylex(void)
{
	char *c;

	while (fgets(yytext, sizeof(yytext), yyin)) {
		yylineno++;
		/* chomp newline */
		if ((c = strchr(yytext, '\n')))
			*c = '\0';
		/* eat blank lines and comments */
		if (!*yytext || *yytext == '#')
			continue;
		/* evaluate variables */
		eval(yytext);
	}

	return 0;
}

/* VX wants prototypes even for static functions. */
static char* find_pattern(char **argv, const char *pattern, uint *val);
static int newtuple(char *b, int *cnt, uint8 tag, const cis_tuple_t *srv);
static int parsecis(char *b, char **argv);

/* Find an entry in argv[][] in this form
 *	pattern=(0x)1234
 * fill the value into val, return the pointer to this entry if found, NULL if not found.
 */
static char*
find_pattern(char **argv, const char *pattern, uint *val)
{
	char *ret = NULL, *name = NULL, **pargv = argv;

	/* clear val first */
	if (val)	*val = 0;

	while ((name = *pargv++)) {
		if ((ret = strstr(name, pattern))) {
			char *p = ret, *q = NULL;

			/* Extracting the content */
			p += strlen(pattern);

			/* var name could have same prefix */
			if (*p++ != '=') {
				ret = NULL;
				continue;
			}
			if (!val)
			return (ret+strlen(pattern)+1);

			*val = strtoul(p, &q, 0);
			if (p == q) {
				printf("Bad value: %s\n", ret);
				return NULL;
			}

			break;
		}
	}
	return ret;
}
static int
newtuple(char *b, int *cnt, uint8 tag, const cis_tuple_t *srv)
{
	memset(b, 0, srv->len + 2);

	b[0] = tag;
	b[1] = (char)srv->len;
	b[2] = (char)srv->tag;

	if (cnt)
		*cnt += 3;
	return 0;
}

static int
parsecis(char *b, char **argv)
{
	const cis_tuple_t *srv = cis_hnbuvars;
	char	*cpar = NULL, *p = NULL;
	char	*par = NULL;
	char	delimit[2] = " \0";
	int	cnt = 0;

	/* Walk through all the tuples, create append buffer */
	while (srv->tag != 0xFF) {
		uint val = 0;

		/* Special cases (Raw Data / macaddr / ccode / fem) */
		if (srv->tag == OTP_RAW) {
			if ((p = find_pattern(argv, "RAW", NULL))) {
				for (;;) {
					b[cnt++] = (unsigned char) strtoul(p, &p, 16);
					if (!*p++)
						break;
				}
			}
		} else if (srv->tag == OTP_RAW1) {
			if ((p = find_pattern(argv, "RAW1", NULL))) {
				for (;;) {
					b[cnt++] = (unsigned char) strtoul(p, &p, 16);
					if (!*p++)
						break;
				}
			}
		} else if (srv->tag == OTP_MANFID) {
			bool found = FALSE;
			uint manfid = 0, prodid = 0;

			if ((p = find_pattern(argv, "manfid", &manfid)))
				found = TRUE;

			if ((p = find_pattern(argv, "prodid", &prodid)))
				found = TRUE;

			if (found) {
				b[cnt++] = CISTPL_MANFID;
				b[cnt++] = srv->len;
				b[cnt++] = (uint8)(manfid & 0xff);
				b[cnt++] = (uint8)((manfid >> 8) & 0xff);
				b[cnt++] = (uint8)(prodid & 0xff);
				b[cnt++] = (uint8)((prodid >> 8) & 0xff);
			}
		} else if (srv->tag == HNBU_MACADDR) {
			if ((p = find_pattern(argv, "macaddr", &val))) {
				newtuple(&b[cnt], &cnt, CISTPL_BRCM_HNBU, srv);
				p += (strlen("macaddr") + 1);	/* macaddr= */
				if (!wl_ether_atoe(p, (struct ether_addr*)&b[cnt]))
					printf("Argument does not look like a MAC "
					"address: %s\n", p);
				cnt += sizeof(struct ether_addr);
			}
		} else if (srv->tag == HNBU_CCODE) {
			bool found = FALSE;
			char tmp[3] = "\0\0\0";

			if ((p = find_pattern(argv, "ccode", NULL))) {
				found = TRUE;
				tmp[0] = *p++;
				tmp[1] = *p++;
			}
			if ((p = find_pattern(argv, "cctl", &val))) {
				found = TRUE;
				p += (strlen("cctl") + 1);	/* cctl= */
				tmp[2] = (uint8)val;
			}
			if (found) {
				newtuple(&b[cnt], &cnt, CISTPL_BRCM_HNBU, srv);
				memcpy(&b[cnt], tmp, 3);
				cnt += 3;	/* contents filled already */
			}
		} else if (srv->tag == HNBU_FEM) {
			bool	found = FALSE;
			uint16	tmp2g = 0, tmp5g = 0;

			if ((p = find_pattern(argv, "antswctl2g", &val))) {
				found = TRUE;
				tmp2g |= ((val << SROM8_FEM_ANTSWLUT_SHIFT) &
					SROM8_FEM_ANTSWLUT_MASK);
			}
			if ((p = find_pattern(argv, "triso2g", &val))) {
				found = TRUE;
				tmp2g |= ((val << SROM8_FEM_TR_ISO_SHIFT) &
					SROM8_FEM_TR_ISO_MASK);
			}
			if ((p = find_pattern(argv, "pdetrange2g", &val))) {
				found = TRUE;
				tmp2g |= ((val << SROM8_FEM_PDET_RANGE_SHIFT) &
					SROM8_FEM_PDET_RANGE_MASK);
			}
			if ((p = find_pattern(argv, "extpagain2g", &val))) {
				found = TRUE;
				tmp2g |= ((val << SROM8_FEM_EXTPA_GAIN_SHIFT) &
					SROM8_FEM_EXTPA_GAIN_MASK);
			}
			if ((p = find_pattern(argv, "tssipos2g", &val))) {
				found = TRUE;
				tmp2g |= ((val << SROM8_FEM_TSSIPOS_SHIFT) &
					SROM8_FEM_TSSIPOS_MASK);
			}
			if ((p = find_pattern(argv, "antswctl5g", &val))) {
				found = TRUE;
				tmp5g |= ((val << SROM8_FEM_ANTSWLUT_SHIFT) &
					SROM8_FEM_ANTSWLUT_MASK);
			}
			if ((p = find_pattern(argv, "triso5g", &val))) {
				found = TRUE;
				tmp5g |= ((val << SROM8_FEM_TR_ISO_SHIFT) &
					SROM8_FEM_TR_ISO_MASK);
			}
			if ((p = find_pattern(argv, "pdetrange5g", &val))) {
				found = TRUE;
				tmp5g |= ((val << SROM8_FEM_PDET_RANGE_SHIFT) &
					SROM8_FEM_PDET_RANGE_MASK);
			}
			if ((p = find_pattern(argv, "extpagain5g", &val))) {
				found = TRUE;
				tmp5g |= ((val << SROM8_FEM_EXTPA_GAIN_SHIFT) &
					SROM8_FEM_EXTPA_GAIN_MASK);
			}
			if ((p = find_pattern(argv, "tssipos5g", &val))) {
				found = TRUE;
				tmp5g |= ((val << SROM8_FEM_TSSIPOS_SHIFT) &
					SROM8_FEM_TSSIPOS_MASK);
			}

			if (found) {
				newtuple(&b[cnt], &cnt, CISTPL_BRCM_HNBU, srv);
				b[cnt++] = (uint8)(tmp2g & 0xff);
				b[cnt++] = (uint8)((tmp2g >> 8) & 0xff);
				b[cnt++] = (uint8)(tmp5g & 0xff);
				b[cnt++] = (uint8)((tmp5g >> 8) & 0xff);
			}
		} else {	/* All other tuples */
			int	found = FALSE, varlen = 0;
			char	*cur = &b[cnt];
			uint	newtp = TRUE;

			/* example srv->params contents: "1aa2g 1aa5g" */
			par = malloc(strlen(srv->params)+1);
			if (!par)
				return BCME_NOMEM;

			/* Walk through each parameters in one tuple */
			strcpy(par, srv->params);

			cpar = strtok (par, delimit);	/* current param */
			while (cpar) {
				int array_sz = 1;
				val = 0;

				/* Fill the CIS tuple to b but don't commit cnt yet */
				if (newtp) {
					newtuple(cur, NULL, CISTPL_BRCM_HNBU, srv);
					cur += 3;
					newtp = FALSE;
				}

				/* the first byte of each parameter indicates its length */
				varlen = (*cpar++) - '0';

				/* parse array size if any */
				if (*cpar == '*') {
					array_sz = 0;
					while (((*++cpar) >= '0') && (*cpar <= '9'))
						array_sz = (array_sz * 10) + *cpar - '0';
				}

				/* Find the parameter in the input argument list */
				if ((p = find_pattern(argv, cpar, &val)))
					found = TRUE;

				while (found && array_sz--) {
					*cur++ = (uint8)(val & 0xff);
					if (varlen >= 2)
						*cur++ = (uint8)((val >> 8) & 0xff);
					if (varlen >= 4) {
						*cur++ = (uint8)((val >> 16) & 0xff);
						*cur++ = (uint8)((val >> 24) & 0xff);
					}
					/* skip the "," if more array elements */
					if (p && array_sz) {
						char *q = NULL;

						p = strstr (p, ",");	/* current param */
						if (p) {
							p++;
							val = strtoul(p, &q, strncmp(p, "0x", 2) ?
								10 : 16);
						} else {
							printf("Input array size error!");
							free(par);
							return BCME_BADARG;
						}
					}
				}

				/* move to the next parameter string */
				cpar = strtok(NULL, delimit);
			}
			free(par);
			/* commit the tuple if its valid */
			if (found)
			{
				cnt += (cur - &b[cnt]);
			}
		}

		srv++;
	}

	return cnt;
}

int cisCreate(char * i_file_name, char* o_file_name, int i_bus, int count, char * serno)
{
	FILE *in = NULL, *out = NULL;
	char buf[] = "xx:xx";
	unsigned short maclo = 0;
	int ret = -1;
	int i;

	printf("enter cisCreate\n");
	printf("i_file_name %s\n", i_file_name);
	printf("o_file_name %s\n", o_file_name);
	printf("i_bus %d\n", i_bus);
	printf("count %d\n", count);
	printf("serno %s\n", serno);

	bus = i_bus;
	maclo = (uint16)strtoul(serno, NULL, 0);

	sprintf(buf, "%02x:%02x", (maclo >> 8) & 0xff, maclo & 0xff);
	set("serno", serno, 1);
	set("maclo", buf, 1);

	if (!(yyin = fopen(i_file_name, "r")))
	{
		printf("failed to open input file: %s\n", i_file_name);
		goto out;
	}
	if (!(out = fopen(o_file_name, "wb")))
	{
		printf("failed to open output file: %s\n", o_file_name);
		goto out;
	}

	end = nvram;

	printf("enter cisCreate 0\n");
	/* Parse NVRAM data file */
	yylex();

	if ((i_bus == SDIO) && (tuples == tuple_buf))
	{
		printf("Missing hardware header for SDIO cis format!\n");
		printf("Please add hwhdr=0xxxxxxxxx to input file\n");
		goto out;
	}
	cooked[cooked_cnt] = 0;
	memset(tuple_buf, 0, NVRAM_SPACE);
	if (!parsecis(tuple_buf, cooked)) {
		printf("Nothing to write!\n");
		goto out;
		}
	/* add end flag of the cis content */
	tuples[count-1] = (char)0xff;

	fwrite(tuples, 1, count, out);

	printf("Output: %s\n", o_file_name);

	printf("Bus: (%s)", (i_bus == USB) ? "USB" : "SDIO");

	printf("\nMaximum length: %d bytes", count);
	for (i = 0; i < (int)count; i++) {
		if ((i % 8) == 0)
			printf("\nByte %3d: ", i);
		printf("0x%02x ", (uint8)tuples[i]);
	}
	printf("\n");

	ret = 0;
out:
	if (in)
		fclose(in);
	if (out)
		fclose(out);
	return ret;
}

static void
usage(void)
{
	fprintf(stderr, "usage: nvserial [options] [file]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, "	-h, --help            This message\n");
	fprintf(stderr, "	-v, --verbose         Dump more information to stderr\n");
	fprintf(stderr, "	-d, --device <device> device interface (USB |SDIO)\n");
	fprintf(stderr, "	-i, --input  <input>  Input stream (default none)\n");
	fprintf(stderr, "	-o, --output <output> Output stream (default stdout)\n");
	fprintf(stderr, "	-c, --count  <count>  Bytes of NVRAM to write (default 0x2000)\n");
	fprintf(stderr, "	-a, --standalone      Create standalone nvram image from text "
	        "file\n");
	fprintf(stderr, "	-n, --no-hdr-warn     Do not warn on missing hdr values\n");
	fprintf(stderr, "	-s, --serno  <serno>  Sets the variables \"serno\" and "
	        "\"maclo\" (optional)\n");
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
	char i_file_name[128] = "nvram.txt";
	char o_file_name[128] = "otp.bin";
	char default_serno[32] = "100";
	int  bus_type = USB;
	char * i_serno = &default_serno[0];

	int count = 0, ret = 0;

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
			goto out;
		} else if (strcmp(*argv, "-d") == 0 ||
		           strcmp(*argv, "--device") == 0) {
			if (!*++argv)
				error("-i: missing argument");
			if ((strcmp(*argv, "sdio") == 0) || (strcmp(*argv, "SDIO") == 0))
				bus_type = SDIO;
		} else if (strcmp(*argv, "-i") == 0 ||
		           strcmp(*argv, "--input") == 0) {
			if (!*++argv)
				error("-i: missing argument");
			strcpy(i_file_name, *argv);
		} else if (strcmp(*argv, "-o") == 0 ||
		           strcmp(*argv, "--output") == 0) {
			if (!*++argv)
				error("-o: missing argument");
			strcpy(o_file_name, *argv);
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
			i_serno = *argv;
		} else if (strcmp(*argv, "-v") == 0 ||
		           strcmp(*argv, "--verbose") == 0) {
			verbose = 1;
		}
	}

	cisCreate(i_file_name, o_file_name, bus_type, count, i_serno);

	if (verbose) {
		fprintf(stderr, "count: 0x%x\n", count);
		fprintf(stderr, "magic: 0x%08x\n", hdr->magic);
		fprintf(stderr, "len: 0x%08x\n", hdr->len);
		fprintf(stderr, "crc_ver_init: 0x%08x\n", hdr->crc_ver_init);
		fprintf(stderr, "config_refresh: 0x%08x\n", hdr->config_refresh);
		for (end = &nvram[NVRAM_HEADER_SIZE]; *end; end += strlen(end) + 1)
			fprintf(stderr, "%s\n", end);
	}

out:
	return ret;
}
