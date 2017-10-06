/*
 * fwtag - read/write tag information at the end of a firmware bin file.
 *
 * Copyright (C) 2010 Broadcom Corporation
 *
 * $Id$
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <typedefs.h>
#include <bcmsdpcm.h>
#include <bcmutils.h>
#include <limits.h>

#define BUFSIZE		(1024*1024)
static unsigned char buffer[BUFSIZE];

#define USE_EPI_VERSION_STR
#ifdef USE_EPI_VERSION_STR
#define EPIVER_STR_SIZE	128
static char epiver_str[EPIVER_STR_SIZE];

#define	VERTSIZE		uint8
#define	SCANF_VERSTR_FMT	"%c.%c.%c.%c"
#else
#define	VERTSIZE		uint32
#define	SCANF_VERSTR_FMT	"%d.%d.%d.%d"
#endif /* USE_EPI_VERSION_STR */

#define CRCSIZE		4
#define TIMESIZE	4
#define VERSIONSIZE	(sizeof(VERTSIZE) * 4)

#define MAXVARIANTSIZE	19
#define MAXTAGSIZE		(TIMESIZE +  MAXVARIANTSIZE + VERSIONSIZE + 1)
				/* 28 (TIMESIZE + MAXVARIANTSIZE + VERSIONSIZE + SIZESIZE */

#define NEWMAXVARIANTSIZE	35
#define NEWMAXTAGSIZE		(TIMESIZE +  NEWMAXVARIANTSIZE + VERSIONSIZE + 1)
				/* 44 (TIMESIZE + NEWMAXVARIANTSIZE + VERSIONSIZE + SIZESIZE */

#define MAXVARIANTS	(maxvariants - 1)
#define MAXBRANDS	(maxbrands - 1)

#define TAGMAGIC	0x832bd01
#define TAGMAGICSIZE	4
#define TAGVERSIONSIZE	2

#define ATAGLENSIZE	2	/* Ascii Tag Length Size */

#define INIFILE_DEFAULT	"/projects/hnd/tools/linux/bin/fwtag.ini"

#define MAXLINE		512

#define FALSE		0
#define TRUE		1


/*
 * Backwards compatibility notes
 *
 * For Reading:
 *
 * Reading firmware files or coredumps and figuring out which tag format was used:
 *
 * There are 2 version numbers we use to determine the correct offsets.
 *
 * For processing firmware files, we use Tag version that was writen to the ASCII
 * tag header when the file was originally tagged.
 *
 * TAGVERSION == 1 then old style 32 byte tag
 * TAGVERSION == 2 then new style 48 byte tag
 * TAGVERSION == 3 Same format and size as V2 but write new CRC in last 4 bytes of
 *                 encoded variant. Backwards compatible, fw will just copy this in so
 *                 it will appear in dumps.
 *
 * For processing coredumps, we use the version number in the sdpcm_shared_t
 * struct retrived from coredumps/socram uploads.
 *
 * SDPCM_SHARED_VERSION == 2 then old style 32 byte tag
 * SDPCM_SHARED_VERSION >= 3 then new style 48 byte tag
 *
 * For writing:
 *
 * We default to new style tags....
 * Unless the "-W" option is used in which case we read the SDPCM_SHARED_VERSION
 * from the include directory to determine which type to write.
 *
 */

#define OLDTAGVERSION	1
#define NEWTAGVERSION	2
#define V3TAGVERSION	3

#define V3MAXVARIANTSIZE	(NEWMAXVARIANTSIZE-4) /* steal last 4 bytes for new crc */

#define V0_SDPCM_SHARED_VERSION 1
#define OLD_SDPCM_SHARED_VERSION 2
#define NEW_SDPCM_SHARED_VERSION 3
#define V3_SDPCM_SHARED_VERSION 4

void test1(int passes);
void test2(int passes);
void test3(int passes);

char *Variants[256];
char *Brands[256];

int maxbrands;
int maxvariants;

char buf[0x8000];

unsigned long flags;

#define dflag	0x0001
#define rflag	0x0002
#define wflag	0x0004
#define Wflag	0x0008
#define Tflag	0x0010
#define vflag	0x0020
#define Rflag	0x0040
#define Dflag	0x0080
#define tflag	0x0100

void dumpBrands()
{
	int i = 1;

	printf("\nBrands:\n");

	while (Brands[i])
		printf("    %s\n", Brands[i++]);

	printf("Total %d\n", i - 2);

	i = 1;
	printf("\n\nVariants:\n");

	while (Variants[i])
		printf("    %s\n", Variants[i++]);

	printf("Total %d\n\n", i - 2);
}

/*
 * Encode (compress) a Ascii firmware brand & variant string into a series of bytes.
 *
 * Example:
 *
 * "4329b1/sdio-ag-cdc-full11n-reclaim-roml-idsup-nocis-p2p-memredux16-pno-aoe-...
 * ...pktfilter-minioctl-29agbf-overlay"
 *
 * 117 bytes, becomes: "19 5a 6 14 21 51 54 24 40 44 30 48 7 46 35 2", 16 bytes
 *
 */

/* Terminology:
 *
 * 4329b1/sdio-ag-cdc-full11n-reclaim-roml-wme
 * ^^^^^^ ^^^^ ^^ ^^^ ^^^^^^^ ^^^^^^^ ^^^^ ^^^
 * brand  variants
 *
 * The entire string is the tag
 */

int KeywordToCode(char **Keywords, char *Keyword)
{
	int i = 0;

	while  (Keywords[i]) {
		if (!strcmp(Keywords[i], Keyword))
			return i;
		i++;
	}

	return -1;
}

char *CodeToKeyword(char **Keywords, int code)
{
	return Keywords[code];
}

char VariantToCode(char *Variant)
{
	char Code = KeywordToCode(Variants, Variant);

	if (Code < 0) {
		Code = 0;
	}

	return Code;
}

char BrandToCode(char *Brand)
{
	char Code = KeywordToCode(Brands, Brand);

	if (Code < 0) {
		Code = 0;
	}

	return Code;
}

char TagToCodes(char *Tag, char *Codes, int MaxLength)
{
	char *cp = strchr(Tag, '/');
	char *VarantStart;
	int CodeLength;
	int done = 0;

	if (MaxLength <= 0)
		return 0;

	if (!cp) {
		printf("error: tag does not contain a /\n");
		printf("Tag = %s\n", Tag);
		exit(1);
	}

	*cp++ = 0;

	*Codes++ = BrandToCode(Tag);

	for (CodeLength = 1; CodeLength < MaxLength; CodeLength++) {
		if (done)
			break;

		if (!(*cp))
			break;

		VarantStart = cp;

		cp = strchr(cp, '-');

		if (!cp)
			done++;
		else
			*cp = 0;

		*Codes++ = VariantToCode(VarantStart);

		cp++;

	}

	*Codes = 0;
	return CodeLength;
}


/*
 * Assumes file is seeked to first byte of CRC32
 */

int FileToTag(FILE *file, char *cp, bool new)
{
		unsigned char Tag[NEWMAXTAGSIZE + CRCSIZE + 1];
		char DateBuf[128];
		int CodeLen;
		int i;
		int j;
		int tmp;
		int CrcOffset;
		int DateOffset;
		int VersionOffset;
		time_t t;
		struct tm *ts;
		int maxTagSize = MAXTAGSIZE;
		int maxVariantSize = MAXVARIANTSIZE;
		VERTSIZE ver1, ver2, ver3, ver4;

		if (new) {
			maxTagSize = NEWMAXTAGSIZE;
			maxVariantSize = NEWMAXVARIANTSIZE;
		}

		fread(&Tag, 1, (maxTagSize + CRCSIZE), file);

		if (ferror(file)) {
			perror("FileToTag");
			exit(-3);
		}

		CodeLen = Tag[maxTagSize + CRCSIZE - 1];

		if (flags & Dflag)
			printf("\tCodeLen %d, maxVariantSize %d\n", CodeLen, maxVariantSize);

		if (CodeLen > maxVariantSize)
			goto BadTag2;

		CrcOffset = 0;
		VersionOffset = CrcOffset + CRCSIZE;
		DateOffset = VersionOffset + VERSIONSIZE;
		i = DateOffset + TIMESIZE;

		tmp = CodeLen;

		/* make pass thru and validate codes */

		if (Tag[i++] > MAXBRANDS)
			goto BadTag2;


		if (flags & Dflag) {
			printf("\tFixed CRC (%d): %x\n", 12 + maxVariantSize - 4,  *((unsigned int *) &Tag[12 + maxVariantSize - 4]));

			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			Tag[0x0], Tag[0x1], Tag[0x2], Tag[0x3], Tag[0x4], Tag[0x5], Tag[0x6], Tag[0x7], Tag[0x8], Tag[0x9], Tag[0xa], Tag[0xb], Tag[0xc], Tag[0xd], Tag[0xe], Tag[0xf]);
			printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
			Tag[0x10], Tag[0x11], Tag[0x12], Tag[0x13], Tag[0x14], Tag[0x15], Tag[0x16], Tag[0x17], Tag[0x18], Tag[0x19], Tag[0x1a], Tag[0x1b], Tag[0x1c], Tag[0x1d], Tag[0x1e], Tag[0x1f]);
		}

		/* make pass thru and print tag */

		i = 12;

		tmp = sprintf(cp, "%s/", Brands[Tag[i++]]);

		if (tmp < 0)
			return 0;

		cp += tmp;
		j = CodeLen;

		while (--j) {
			tmp = sprintf(cp, "%s", Variants[Tag[i++]]);

			if (tmp < 0)
				return 0;

			cp += tmp;

			if (j > 1) {
				*cp++ = '-';
			}
		}

		if (CodeLen == maxVariantSize) {
			*cp++ = '-';
			*cp++ = '.';
			*cp++ = '.';
			*cp++ = '.';
		}

		tmp = *((unsigned long *) &Tag[DateOffset]);

		t = (time_t) tmp;
		ts = localtime(&t);

		strftime(DateBuf, sizeof(DateBuf), "%a %Y-%m-%d %H:%M:%S %Z", ts);

		memcpy(&ver1, &Tag[VersionOffset], sizeof(ver1));
		VersionOffset += sizeof(ver1);
		memcpy(&ver2, &Tag[VersionOffset], sizeof(ver2));
		VersionOffset += sizeof(ver2);
		memcpy(&ver3, &Tag[VersionOffset], sizeof(ver3));
		VersionOffset += sizeof(ver3);
		memcpy(&ver4, &Tag[VersionOffset], sizeof(ver4));

#ifdef USE_EPI_VERSION_STR
		tmp = sprintf(cp, " Version: %s CRC: %x Date: %s", epiver_str,
			~*((unsigned int *) &Tag[CrcOffset]), DateBuf);
#else
		tmp = sprintf(cp, " Version: %d.%d.%d.%d CRC: %x Date: %s", ver1, ver2, ver3, ver4,
			~*((unsigned int *) &Tag[CrcOffset]), DateBuf);
#endif

		if (tmp < 0)
			return 0;

		return 1;
BadTag2:
		return 0;
}

void usage()
{

	/* Plz forgive the formating of the printf's below */

	/* It was done to placate the "cstyle" command */
	/* (nothing quite like subjecting others to your bad taste in formating */
	/* via a external command) */

	printf("\nusage:\n\n");
	printf("\nThis utility is used to write a tag with the build information %s",
		"to a binary firmware file\n");
	printf("This utility can also be used to retrieve that tag from a firmware %s",
		"file or from a socram download (crashdump)\n");
	printf("\nA socram download a dump of the dongle ram created via the  %s",
		"\"dhd download\" command\n\n");
	printf("\nNOTE: Includes support for version beyond 255\n");
	printf("\nSyntax:\n\n");
	printf(" fwtag -W                                         - derive tag and version from path (obsolete)\n");
	printf("                                                    and write to end of rtecdc.bin\n");
	printf(" fwtag -w <firmware file> -t <tag> -v <version>   - write tag to end of file\n");
	printf(" fwtag -r <firmware file>                         - read and display tag from a %s",
		"firmware file (rtecdc.bin format)\n");
	printf(" fwtag -R <core file>                             - read and display tag from socram %s",
		"file (dhd download format)\n");
	printf(" fwtag -f <firmware file> -p <src path>           - derive tag and version from path\n");
	printf("\n\n");
	printf("Extras/debugging:\n");
	printf(" -D                                               - display debugging info\n");
	printf(" -V                                               - display brands and variants\n");
	printf(" fwtag -d <tag>                                   - encode and display tag on stdout\n");
	printf("\n\n");
	printf("Example:\n");
	printf(" fwtag -w rtecdc.bin -t 4329b1/sdio-ag-cdc-roml -v 4.221.47.3 \n\n");
}

char* stripWhite(char* s)
{
	char* p = s + strlen(s);

	while (p > s && isspace(*--p))
		*p = '\0';

	while (*s && isspace(*s))
		s++;

	return s;
}

char *dupString(char *s)
{
	char *cp;
	int len;

	if (!*s)
		return NULL;

	len = strlen(s);

	if (!len)
		return NULL;

	cp = malloc(len + 1);
	strncpy(cp, s, len);

	cp[len] = 0;

	return cp;
}

void parseINIFile(char *fname_ini)
{
	FILE* file;
	char line[MAXLINE];
	char *cp;
	int inBrandSection = FALSE;
	int inVariantSection = FALSE;
	int lineno = 0;

	maxbrands = 0;
	maxvariants = 0;

	file = fopen(fname_ini, "r");

	if (!file) {
		perror(fname_ini);
		exit(1);
	}

	while (fgets(line, sizeof(line), file) != NULL) {

		lineno++;
		cp = stripWhite(line);
		if (*cp && *cp != ';') {
			if (!inBrandSection && !inVariantSection && *cp != '[') {
				printf("error parsing %s file at line %d: unknown token while %s",
				       fname_ini, lineno,
				       "looking for start of chip or variant section\n");
				exit(1);
			}

			if (!strncmp(cp, "[brands]", strlen(cp))) {
				inBrandSection = TRUE;
				inVariantSection = FALSE;
			} else if (!strncmp(cp, "[variants]", strlen(cp))) {
				inBrandSection = FALSE;
				inVariantSection = TRUE;
			} else if (*cp == '[') {
				printf("error parsing %s file at line %d: unknown section %s\n",
				       fname_ini, lineno, cp);
				exit(1);
			} else if (inBrandSection) {
				Brands[maxbrands] = dupString(cp);
				maxbrands++;
			} else {
				Variants[maxvariants] = dupString(cp);
				maxvariants++;
			}
		}
	}

	Brands[maxbrands] = NULL;
	Variants[maxvariants] = NULL;

	fclose(file);

	if (flags & Dflag)
		printf("read %d lines, %d brands found, %d variants found\n",
			lineno, maxbrands, maxvariants);
}

char *Dates[] = { "Jan ", "Feb ", "Mar ", "Apr ", "May ", "Jun ", "Jul ", "Aug ", "Sep ", "Oct ", "Nov ", "Dec ", 0 };

void
wipedates(unsigned char *cp, int size) {
        char **dp;
        char *np;
        char *ep;

        for (dp = Dates; *dp; dp++) {
		if (flags & Dflag) printf("checking: %s\n", *dp);
		
                np = (void *)memmem(cp, size, *dp, strlen(*dp));
                if (np) {
                        if (flags & Dflag) printf("Found: %s\n", np);
                        ep = np + strlen(np) + 1;
                        if (flags & Dflag) printf("Found: %s\n", ep);
                        ep += strlen(np);
                        while (np < ep) {
                                *np++ = 0;
                        }
                }
        }
}


#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]



/*******************************************************************************
 * crc16
 *
 * Computes a crc16 over the input data using the polynomial:
 *
 *       x^16 + x^12 +x^5 + 1
 *
 * The caller provides the initial value (either CRC16_INIT_VALUE
 * or the previous returned value) to allow for processing of
 * discontiguous blocks of data.  When generating the CRC the
 * caller is responsible for complementing the final return value
 * and inserting it into the byte stream.  When checking, a final
 * return value of CRC16_GOOD_VALUE indicates a valid CRC.
 *
 * Reference: Dallas Semiconductor Application Note 27
 *   Williams, Ross N., "A Painless Guide to CRC Error Detection Algorithms",
 *     ver 3, Aug 1993, ross@guest.adelaide.edu.au, Rocksoft Pty Ltd.,
 *     ftp://ftp.rocksoft.com/clients/rocksoft/papers/crc_v3.txt
 *
 * ****************************************************************************
 */

static const uint16 crc16_table[256] = {
    0x0000, 0x1189, 0x2312, 0x329B, 0x4624, 0x57AD, 0x6536, 0x74BF,
    0x8C48, 0x9DC1, 0xAF5A, 0xBED3, 0xCA6C, 0xDBE5, 0xE97E, 0xF8F7,
    0x1081, 0x0108, 0x3393, 0x221A, 0x56A5, 0x472C, 0x75B7, 0x643E,
    0x9CC9, 0x8D40, 0xBFDB, 0xAE52, 0xDAED, 0xCB64, 0xF9FF, 0xE876,
    0x2102, 0x308B, 0x0210, 0x1399, 0x6726, 0x76AF, 0x4434, 0x55BD,
    0xAD4A, 0xBCC3, 0x8E58, 0x9FD1, 0xEB6E, 0xFAE7, 0xC87C, 0xD9F5,
    0x3183, 0x200A, 0x1291, 0x0318, 0x77A7, 0x662E, 0x54B5, 0x453C,
    0xBDCB, 0xAC42, 0x9ED9, 0x8F50, 0xFBEF, 0xEA66, 0xD8FD, 0xC974,
    0x4204, 0x538D, 0x6116, 0x709F, 0x0420, 0x15A9, 0x2732, 0x36BB,
    0xCE4C, 0xDFC5, 0xED5E, 0xFCD7, 0x8868, 0x99E1, 0xAB7A, 0xBAF3,
    0x5285, 0x430C, 0x7197, 0x601E, 0x14A1, 0x0528, 0x37B3, 0x263A,
    0xDECD, 0xCF44, 0xFDDF, 0xEC56, 0x98E9, 0x8960, 0xBBFB, 0xAA72,
    0x6306, 0x728F, 0x4014, 0x519D, 0x2522, 0x34AB, 0x0630, 0x17B9,
    0xEF4E, 0xFEC7, 0xCC5C, 0xDDD5, 0xA96A, 0xB8E3, 0x8A78, 0x9BF1,
    0x7387, 0x620E, 0x5095, 0x411C, 0x35A3, 0x242A, 0x16B1, 0x0738,
    0xFFCF, 0xEE46, 0xDCDD, 0xCD54, 0xB9EB, 0xA862, 0x9AF9, 0x8B70,
    0x8408, 0x9581, 0xA71A, 0xB693, 0xC22C, 0xD3A5, 0xE13E, 0xF0B7,
    0x0840, 0x19C9, 0x2B52, 0x3ADB, 0x4E64, 0x5FED, 0x6D76, 0x7CFF,
    0x9489, 0x8500, 0xB79B, 0xA612, 0xD2AD, 0xC324, 0xF1BF, 0xE036,
    0x18C1, 0x0948, 0x3BD3, 0x2A5A, 0x5EE5, 0x4F6C, 0x7DF7, 0x6C7E,
    0xA50A, 0xB483, 0x8618, 0x9791, 0xE32E, 0xF2A7, 0xC03C, 0xD1B5,
    0x2942, 0x38CB, 0x0A50, 0x1BD9, 0x6F66, 0x7EEF, 0x4C74, 0x5DFD,
    0xB58B, 0xA402, 0x9699, 0x8710, 0xF3AF, 0xE226, 0xD0BD, 0xC134,
    0x39C3, 0x284A, 0x1AD1, 0x0B58, 0x7FE7, 0x6E6E, 0x5CF5, 0x4D7C,
    0xC60C, 0xD785, 0xE51E, 0xF497, 0x8028, 0x91A1, 0xA33A, 0xB2B3,
    0x4A44, 0x5BCD, 0x6956, 0x78DF, 0x0C60, 0x1DE9, 0x2F72, 0x3EFB,
    0xD68D, 0xC704, 0xF59F, 0xE416, 0x90A9, 0x8120, 0xB3BB, 0xA232,
    0x5AC5, 0x4B4C, 0x79D7, 0x685E, 0x1CE1, 0x0D68, 0x3FF3, 0x2E7A,
    0xE70E, 0xF687, 0xC41C, 0xD595, 0xA12A, 0xB0A3, 0x8238, 0x93B1,
    0x6B46, 0x7ACF, 0x4854, 0x59DD, 0x2D62, 0x3CEB, 0x0E70, 0x1FF9,
    0xF78F, 0xE606, 0xD49D, 0xC514, 0xB1AB, 0xA022, 0x92B9, 0x8330,
    0x7BC7, 0x6A4E, 0x58D5, 0x495C, 0x3DE3, 0x2C6A, 0x1EF1, 0x0F78
};

uint16
hndcrc16(
    uint8 *pdata,  /* pointer to array of data to process */
    uint nbytes, /* number of input data bytes to process */
    uint16 crc     /* either CRC16_INIT_VALUE or previous return value */
)
{
	while (nbytes-- > 0)
		CRC_INNER_LOOP(16, crc, *pdata++);
	return crc;
}

static const uint32 crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};


/*
 * crc input is CRC32_INIT_VALUE for a fresh start, or previous return value if
 * accumulating over multiple pieces.
 */
uint32
hndcrc32(uint8 *pdata, uint nbytes, uint32 crc)
{
	uint8 *pend;
#ifdef __mips__
	uint8 tmp[4];
	ulong *tptr = (ulong *)tmp;

	if (nbytes > 3) {
		/* in case the beginning of the buffer isn't aligned */
		pend = (uint8 *)((uint)(pdata + 3) & ~0x3);
		nbytes -= (pend - pdata);
		while (pdata < pend)
			CRC_INNER_LOOP(32, crc, *pdata++);
	}

	if (nbytes > 3) {
		/* handle bulk of data as 32-bit words */
		pend = pdata + (nbytes & ~0x3);
		while (pdata < pend) {
			*tptr = *(ulong *)pdata;
			pdata += sizeof(ulong *);
			CRC_INNER_LOOP(32, crc, tmp[0]);
			CRC_INNER_LOOP(32, crc, tmp[1]);
			CRC_INNER_LOOP(32, crc, tmp[2]);
			CRC_INNER_LOOP(32, crc, tmp[3]);
		}
	}

	/* 1-3 bytes at end of buffer */
	pend = pdata + (nbytes & 0x03);
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#else
	pend = pdata + nbytes;
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#endif /* __mips__ */

	return crc;
}


unsigned int
crc(const char *path)
{
	unsigned int crcval = CRC32_INIT_VALUE;
	unsigned int count;
	size_t len = 0;
	FILE *file;


	file = fopen(path, "rb");
	if (file == NULL) {
		perror(path);
		exit(-2);
	}

	for (count = 0; !feof(file); count += len) {
		len = fread(buffer, sizeof(unsigned char), BUFSIZE, file);
		if (len != BUFSIZE && ferror(file)) {
			perror(path);
			exit(-2);
		}

		wipedates(buffer, BUFSIZE);

		if (0) {
			unsigned short offset = *(unsigned short *) &buffer[len - 2];
			unsigned int magic;

			if (flags & Dflag) printf("offset %d\n", offset);

			if (offset < 256) {
				magic = *((unsigned int *) &buffer[len - (offset + 8)]);
				//printf("magic %x, tagmagic %x\n", magic, (unsigned int) TAGMAGIC);

				if (magic == ((unsigned int)TAGMAGIC)) {
					unsigned short version = *((unsigned short *) &buffer[len - (offset + 4)]);

					if (flags & Dflag) printf("magic %x, version %d\n", magic, version);
					if (version == 1)
						len -= offset + 28 + 12;
					else
						len -= offset + 44 + 12;

					if (flags & Dflag) printf("%x\n", *((unsigned int *) &buffer[len]));

				} else
					if (flags & Dflag) printf("no tag\n");
			}

		}

		crcval = hndcrc32(buffer, len - 4, crcval);
	}

	if (fclose(file)) {
		perror(path);
		exit(-2);
	}

	if (flags & Dflag) printf("CRC32 over %d bytes: 0x%08x\n", count, crcval);

	if (flags & Dflag) {
		file = fopen("tmp.bin", "wb");
		fwrite(buffer, len -4, 1, file);
		fclose(file);
	}

	return crcval;
}



int findTagMagic(unsigned char *buf, int len) {
	int i;
	int offset = 0;

	/* first locate TAGMAGIC */
	for (i = 0; i < len-4; i++) {
		if (buf[i] == 0x01 && buf[i+1] == 0xbd && buf[i+2] == 0x32 && buf[i+3] == 0x8) {
			offset = i;
			break;
		}
			
	}

	if (!offset)
		return -1;

	/* Skim over ascii tag till a null */
	/* The byte before the null should contain the length of the tag string */
	for (i = offset + 6; i < len; i++) {
		if (!buf[i]) {
			if (buf[i-1] == i - offset - 7);
				/* Found end of tag, return the offset from the end of buffer */
				return len - i;
		}
	}

	return -1;
}

int main(int ac, char**av)
{
	int c;
	int i;
	int fwTagVersion;
	int sharedVersion = NEW_SDPCM_SHARED_VERSION; /* default to new style tags */
	short asciiTagVersion;
	int tmp;
	short tmpShort;
	int CodeLen;
	char Codes[64];
	time_t t;
	FILE *file;
	char *fileName = NULL;
	char *tag = NULL;
	char *versionString = NULL;
	char *iniFileName = INIFILE_DEFAULT;
	char *srcPathName = "../../../../../../../src";
	char *ucodePathName = NULL;
	VERTSIZE v[4];
	char buf2[2048];
	char tagOverride[2048];
	char incFileName[1024];
	char uCodeVerFileName[1024];
	char uCodeVersionString[128];
	int d11ucode_p2p_bommajor, d11ucode_p2p_bomminor;

	if (ac == 1) {
		usage();
		exit(1);
	}

	memset(v, 0, sizeof(v)); /* clear version string */
	while ((c = getopt(ac, av, "r:R:w:WdTDt:v:Vi:f:p:u:")) != -1)
		switch (c) {
			case 'r':
				flags |= rflag;
				fileName = optarg;
				break;
			case 'R':
				flags |= Rflag;
				fileName = optarg;
				break;
			case 'w':
				flags |= wflag;
				fileName = optarg;
				break;
			case 'W':
				flags |= Wflag;
				fileName = "rtecdc.bin";
				break;
			case 'd':
				flags |= dflag;
				break;
			case 'T':
				flags |= Tflag;
				break;
			case 't':
				flags |= tflag;
				tag = optarg;
				break;
			case 'v':
				flags |= vflag;
				versionString = optarg;
				break;
			case 'D':
				flags |= Dflag;
				printf("D flag\n");
				break;
			case 'V':
				dumpBrands();
				exit(0);
			case 'i':
				iniFileName = optarg;
				break;
			case 'f':
				flags |= Wflag;
				fileName = optarg;
				break;
			case 'p':
				srcPathName = optarg;
				break;
			case 'u':
				ucodePathName = optarg;
				break;
			default:
				usage();
				exit(1);
		}

	parseINIFile(iniFileName);
	tagOverride[0] = 0;

	if (flags & Tflag) {
		int seed = 1;
		int test = 1;
		int passes = 10;

		if (ac >= 3)
			seed = atoi(av[1]);

		if (ac >= 4)
			test = atoi(av[2]);

		if (ac == 5)
			passes = atoi(av[3]);

		printf("\nTest #%d, passes %d, seed %d\n\n", test, passes, seed);

		srand(seed);

		if (test == 1)
			test1(passes);

		if (test == 2)
			test2(passes);

		if (test == 3)
			test3(passes);

		exit(0);
	}

	if (flags & Wflag) {
		/*
		 * derive the tag from the file path and version from the epivers.h file.
		 */

		char *cp = buf;
		char *lcp = NULL;
		char *llcp = NULL;
#ifdef USE_EPI_VERSION_STR
		char *cpbk = NULL;
#endif

		snprintf(incFileName, sizeof(incFileName), "%s/include/epivers.h", srcPathName);

		file = fopen(incFileName, "rb");

		if (file == NULL) {
			/* can't locate include/epivers.h. */
			/* Make a note of it and exit quietly  */

			char *errormsg = "can't locate include/epivers.h";

			exit(0);
		}


		fread(buf, 1, sizeof(buf), file);

		if (ferror(file)) {
			perror("include/epivers.h");
			exit(-3);
		}

		cp = strstr(buf, "EPI_VERSION");

		if (cp) {
			int ver1 = 0, ver2 = 0, ver3 = 0, ver4 = 0;

			cp += sizeof("EPI_VERSION") + 1;
			sscanf(cp, "%d, %d, %d, %d", &ver1, &ver2, &ver3, &ver4);
			v[0] = ver1;
			v[1] = ver2;
			v[2] = ver3;
			v[3] = ver4;
		}

#ifdef USE_EPI_VERSION_STR
		cp = strstr(buf, "EPI_VERSION_STR");
		/* find the last EPI_VERSION_STR, the default one, in epivers.h */
		while (cp) {
			cpbk = cp;
			cp = strstr(cpbk + 1, "EPI_VERSION_STR");
		}
		cp = cpbk;

		if (cp) {
			char *vstr;
			uint copy_len;

			/* find EPI_VERSION_STR between "" and copy to epiver_str */
			vstr = strchr(cp, '"') + 1;
			copy_len = MIN(strchr(vstr, '"') - vstr, sizeof(epiver_str) - 1);
			strncpy(epiver_str, vstr, copy_len);
			epiver_str[copy_len + 1] = 0;
		}
#endif

		fclose(file);

		if (ucodePathName) {
			snprintf(uCodeVerFileName, sizeof(uCodeVerFileName),
					"%s/d11ucode_p2p.c", ucodePathName);
		} else {
			snprintf(uCodeVerFileName, sizeof(uCodeVerFileName),
					"%s/wl/sys/d11ucode_p2p.c", srcPathName);
		}

		file = fopen(uCodeVerFileName, "rb");

		if (file == NULL) {
			/* can't locate /wl/sys/d11ucode_p2p.c. */
			/* Make a note of it and exit quietly  */

			char *errormsg = "can't locate wl/sys/d11ucode_p2p.c";

			exit(0);
		}

		fread(buf, 1, sizeof(buf), file);

		if (ferror(file)) {
			perror("wl/sys/d11ucode_p2p.c");
			exit(-3);
		}

		cp = strstr(buf, "d11ucode_p2p_bommajor");

		if (cp) {
			cp += sizeof("d11ucode_p2p_bommajor") + 1;
			sscanf(cp, "%d", &d11ucode_p2p_bommajor);
		}

		cp = strstr(buf, "d11ucode_p2p_bomminor");

		if (cp) {
			cp += sizeof("d11ucode_p2p_bomminor") + 1;
			sscanf(cp, "%d", &d11ucode_p2p_bomminor);
		}

		fclose(file);

		tmp = sprintf(uCodeVersionString, " Ucode Ver: %d.%d",
			d11ucode_p2p_bommajor, d11ucode_p2p_bomminor);

		if (tmp < 0)
			exit(-3);

		printf("uCodeVersionString %s\n", uCodeVersionString);

		snprintf(incFileName, sizeof(incFileName), "%s/include/bcmsdpcm.h", srcPathName);

		file = fopen(incFileName, "rb");

		if (file == NULL) {
			perror("include/bcmsdpcm.h");
			printf("make sure your cwd is the same as rtecdc.bin\n");
			exit(-3);
		}


		fread(buf, 1, sizeof(buf), file);

		if (ferror(file)) {
			perror("include/bcmsdpcm.h");
			exit(-3);
		}

		cp = strstr(buf, "SDPCM_SHARED_VERSION");

		if (cp) {
			cp += sizeof("SDPCM_SHARED_VERSION");
			while (*cp == ' ' || *cp == '\t')
				cp++;
			i = sscanf(cp, "%x", &sharedVersion);

			if (i != 1) {
				printf("Warning: unable to read SDPCM_SHARED_VERSION from env\n");
				printf("Warning: defaulting to old style tags\n");
			}

			if (flags & Dflag) printf("\tsharedVersion %x\n", sharedVersion);
		}

		fclose(file);

		getcwd(buf, sizeof(buf));

		for (cp = buf;;) {
			cp = strchr(cp + 1, '/');

			if (!cp)
				break;

			llcp = lcp;
			lcp = cp;
		}

		tag = llcp + 1;
		strncpy(tagOverride, tag, sizeof(tagOverride));

		flags |= wflag | tflag;
	}

	if (flags & dflag) {
		CodeLen = TagToCodes(av[1], Codes, sizeof(Codes));

		for (i = 0; i < CodeLen; i++)
			printf("%x ", Codes[i]);

		printf("\n");

		exit(0);
	}

	if (flags & wflag) {
		unsigned int newCrc;
		int maxTagSize = MAXTAGSIZE;
		int maxVariantSize = MAXVARIANTSIZE;
		int tagVersion = OLDTAGVERSION;

		if (!(flags & tflag)) {
			printf("error: you must supply a tag with -t\n");
			usage();
			exit(1);
		}

		if (sharedVersion >= NEW_SDPCM_SHARED_VERSION) {
			maxTagSize = NEWMAXTAGSIZE;
			if (sharedVersion == NEW_SDPCM_SHARED_VERSION) {
				/* old V2 without the new crc */
				maxVariantSize = NEWMAXVARIANTSIZE;
				tagVersion = NEWTAGVERSION;
			} else {
				maxVariantSize = V3MAXVARIANTSIZE;
				tagVersion = V3TAGVERSION;
			}
		}

		file = fopen(fileName, "rb+");

		if (file == NULL) {
			perror(fileName);
			exit(-3);
		}

		fseek(file, 0, SEEK_END);

		if (file == NULL) {
			perror(fileName);
			exit(-3);
		}

		CodeLen = TagToCodes(tag, Codes, maxVariantSize);


		newCrc = crc(fileName);
		printf("FWID 01-%x\n", newCrc);


		if (sharedVersion == V3_SDPCM_SHARED_VERSION || sharedVersion == V0_SDPCM_SHARED_VERSION ||
			sharedVersion == NEW_SDPCM_SHARED_VERSION) {
			if (flags & Dflag) {
				printf("*** %d\n", CodeLen);
				printf("sharedVersion %d\n", sharedVersion);
				printf("Fixed CRC %x\n", newCrc);

				printf("Codes[0]...\n");
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", 
						Codes[0x00], Codes[0x01], Codes[0x02], Codes[0x03], Codes[0x04], Codes[0x05],
						Codes[0x06], Codes[0x07], Codes[0x08], Codes[0x09], Codes[0x0a], Codes[0x0b],
						Codes[0x0c], Codes[0x0d], Codes[0x0e], Codes[0x0f]);
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						Codes[0x10], Codes[0x11], Codes[0x12], Codes[0x13], Codes[0x14], Codes[0x15],
						Codes[0x16], Codes[0x17], Codes[0x18], Codes[0x19], Codes[0x1a], Codes[0x1b],
						Codes[0x1c], Codes[0x1d], Codes[0x1e], Codes[0x1f]);
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n\n",
						Codes[0x20], Codes[0x21], Codes[0x22], Codes[0x23], Codes[0x24], Codes[0x25],
						Codes[0x26], Codes[0x27], Codes[0x28], Codes[0x29], Codes[0x2a], Codes[0x2b],
						Codes[0x2c], Codes[0x2d], Codes[0x2e], Codes[0x2f]);
			}


			*((unsigned int *) &Codes[maxVariantSize - 4]) = newCrc;

			if (flags & Dflag) {
				printf("Codes[0]...\n");
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						Codes[0x00], Codes[0x01], Codes[0x02], Codes[0x03], Codes[0x04], Codes[0x05],
						Codes[0x06], Codes[0x07], Codes[0x08], Codes[0x09], Codes[0x0a], Codes[0x0b],
						Codes[0x0c], Codes[0x0d], Codes[0x0e], Codes[0x0f]);
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						Codes[0x10], Codes[0x11], Codes[0x12], Codes[0x13], Codes[0x14], Codes[0x15],
						Codes[0x16], Codes[0x17], Codes[0x18], Codes[0x19], Codes[0x1a], Codes[0x1b],
						Codes[0x1c], Codes[0x1d], Codes[0x1e], Codes[0x1f]);
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
						Codes[0x20], Codes[0x21], Codes[0x22], Codes[0x23], Codes[0x24], Codes[0x25],
						Codes[0x26], Codes[0x27], Codes[0x28], Codes[0x29], Codes[0x2a], Codes[0x2b],
						Codes[0x2c], Codes[0x2d], Codes[0x2e], Codes[0x2f]);
			}
		}

		/* offset CrcOffset + CRCSIZE - write version number */

		if (!(flags & Wflag) && !(flags & vflag)) {
			printf("Warning: writing null version number\n");

			fwrite(&v, 1, sizeof(v), file);
			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

		} else {
			if (!(flags & Wflag))
				sscanf(versionString, SCANF_VERSTR_FMT, &v[0], &v[1], &v[2], &v[3]);

			for (i = 0; i < ARRAYSIZE(v); i++) {
				fwrite(&v[i], 1, sizeof(v[i]), file);
				if (ferror(file)) {
					perror(fileName);
					exit(-3);
				}
			}
		}


		/* offset VersionOffset + VERSIONSIZE - write time */

		t = time(NULL);
		tmp = (unsigned long) t;

		fwrite(&tmp, 1, sizeof(tmp), file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		/* offset DateOffset + TIMESIZE - write encoded brand + variants */
		if (CodeLen > maxVariantSize)
			CodeLen = maxVariantSize;

		fwrite(Codes, 1, maxVariantSize, file);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		/* offset 30 (old tag) or 46 (new tag) - write length of encoded brand + variants */
		fwrite(&CodeLen, 1, 1, file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		/* backup and decode the newly written tag to ascii so we can write that out */
		fseek(file, -(maxTagSize + CRCSIZE), SEEK_END);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (!FileToTag(file, buf, tagVersion >= NEWTAGVERSION)) {
			printf("Error writing ASCII Tag to file\n");
			exit(-4);
		}


		tmp = TAGMAGIC;
		fwrite(&tmp, 1, sizeof(tmp), file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		tmpShort = tagVersion;
		fwrite(&tmpShort, 1, sizeof(tmpShort), file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (tagOverride[0]) {
			char *cp = strstr(buf, " Version");
			tag = tagOverride;
			if (cp)
				strcat(tag, cp);
			sprintf(buf, " FWID: 01-%x\n", newCrc);
			strcat(tag, uCodeVersionString);
			strcat(tag, buf);
		} else {
			tag = buf;
		}

		tmpShort = strlen(tag) + 1;
		fwrite(tag, 1, tmpShort, file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		fwrite(&tmpShort, 1, sizeof(tmpShort), file);
		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}


		if (fclose(file)) {
			perror(fileName);
			exit(-3);
		}

	}

	/*
	* fwtag -R <core file>                   - read and display tag from core file
	*/

	if (flags & Rflag) {
		int sizeof_sdpcm_shared_t = sizeof(sdpcm_shared_t);

		file = fopen(fileName, "rb");

		if (file == NULL) {
			perror(fileName);
			exit(-3);
		}

		fseek(file, -sizeof(tmp), SEEK_END);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		fread(&tmp, 1, sizeof(tmp), file);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (flags & Dflag) printf("sdpcm_shared_t location %x\n", tmp);


		fseek(file, tmp, SEEK_SET);

		fread(&fwTagVersion, 1, sizeof(fwTagVersion), file);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (flags & Dflag)
			printf("\nsdpcm_shared_t version %x\n",
			fwTagVersion & SDPCM_SHARED_VERSION_MASK);

		if ((fwTagVersion & SDPCM_SHARED_VERSION_MASK) != OLD_SDPCM_SHARED_VERSION &&
		    (fwTagVersion & SDPCM_SHARED_VERSION_MASK) != NEW_SDPCM_SHARED_VERSION)
			goto BadTag;


		/* V3_SDPCM_SHARED_VERSION does not contain a encoded tag */
		/* the brpt_addr field contains the new crc which can be decoded with fwfind */

		fseek(file, tmp + sizeof_sdpcm_shared_t  - MAXTAGSIZE - CRCSIZE, SEEK_SET);

		if (flags & Dflag)
			 printf("sdpcm_shared_t location + tag start offset %x\n",
			tmp + sizeof_sdpcm_shared_t  - MAXTAGSIZE - CRCSIZE);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}
		fseek(file, tmp + 28, SEEK_SET);

		if (flags & Dflag) {
			unsigned char buf[128];
			int i;

			fread(&buf, 1, sizeof(buf), file);

			for (i = 0; i < 0x40; i += 8)
				printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					buf[i], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7],
					buf[i+8], buf[i+9], buf[i+10], buf[i+11], buf[i+12], buf[i+13], buf[i+14], buf[i+15]);

			fseek(file, tmp + 28, SEEK_SET);
		}

		if (!FileToTag(file, buf,
		(fwTagVersion & SDPCM_SHARED_VERSION_MASK) == NEW_SDPCM_SHARED_VERSION))
			goto BadTag;

		printf("%s: %s\n", fileName, buf);
	}

	if (flags & rflag) {
		int trxOffset = 0;

		file = fopen(fileName, "rb");

		if (file == NULL) {
			perror(fileName);
			exit(-3);
		}

		if (strstr(fileName, ".trx")) {
			unsigned char buf[4096];

			fseek(file, -sizeof(buf), SEEK_END);

			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			fread(&buf, 1, sizeof(buf), file);

			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			if ((trxOffset = findTagMagic(buf, sizeof(buf))) < 0)
				goto BadTag;

			trxOffset--;

		}

		fseek(file, -(sizeof(tmpShort) + trxOffset), SEEK_END);


		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		fread(&tmpShort, 1, sizeof(tmpShort), file);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (flags & Dflag) printf("trxOffset %x, atag len %x\n", trxOffset, tmpShort);


		fseek(file, -(TAGVERSIONSIZE + tmpShort + ATAGLENSIZE + trxOffset), SEEK_END);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}


		fread(&asciiTagVersion, 1, sizeof(asciiTagVersion), file);

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (flags & Dflag) printf("Ascii tag version %d\n", asciiTagVersion);

		if (asciiTagVersion != OLDTAGVERSION
				&& asciiTagVersion != NEWTAGVERSION
				&& asciiTagVersion != V3TAGVERSION)
			goto BadTag;

		if (asciiTagVersion >= NEWTAGVERSION) {
			fseek(file, -(NEWMAXTAGSIZE + CRCSIZE + TAGMAGICSIZE + TAGVERSIONSIZE +
				tmpShort + ATAGLENSIZE + trxOffset), SEEK_END);
		} else {
			fseek(file, -(MAXTAGSIZE + CRCSIZE + TAGMAGICSIZE + TAGVERSIONSIZE +
				tmpShort + ATAGLENSIZE + trxOffset), SEEK_END);
		}

		if (ferror(file)) {
			perror(fileName);
			exit(-3);
		}

		if (!FileToTag(file, buf, asciiTagVersion >= NEWTAGVERSION))
			goto BadTag;

		if (flags & Dflag) {
			fseek(file, -(tmpShort + ATAGLENSIZE + trxOffset), SEEK_END);
			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			fread(&buf2, 1, tmpShort, file);
			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			printf("%s:\n", fileName);
			printf("\tEncoded Tag: %s\n\tASCII Tag:   %s\n", buf, buf2);

			fseek(file, -(TAGMAGICSIZE + TAGVERSIONSIZE + tmpShort + ATAGLENSIZE + trxOffset),
				SEEK_END);
			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			fread(&tmp, 1, sizeof(tmp), file);

			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}
			fread(&tmpShort, 1, sizeof(tmpShort), file);

			if (ferror(file)) {
				perror(fileName);
				exit(-3);
			}

			printf("\tTag Version: %d\n\tTag Magic:   %x\n\n\n", tmpShort, tmp);

		} else
			printf("%s: %s\n", fileName, buf);
	}

	exit(0);
BadTag:
	printf("Warning: file does not contain a tag\n");
	exit(1);
}


void test()
{
	int i;
	int CodeLen;
	char Codes[32];
	char Tag[] = "4329b1/sdio-ag-cdc-full11n-reclaim-roml-29agbf-overlay";

	printf("TagToCodes returns: %d\n", CodeLen = TagToCodes(Tag, Codes, sizeof(Codes)));

	for (i = 0; i < CodeLen; i++)
		printf("%x ", Codes[i]);

	printf("\n");
}

char *getRandomTag(char *cp)
{
	int i;
	int v;

	*cp = 0;
	strcat(cp, Brands[ (rand() % (MAXBRANDS-1)) + 1]);
	strcat(cp, "/");

	v = (rand() % (MAXVARIANTSIZE - 1)) + 1;

	for (i = 0; i < v; i++) {
		strcat(cp, Variants[ (rand() % (MAXVARIANTS-1)) + 1]);

		if (i < v-1)
			strcat(cp, "-");
	}

	return cp;
}

char *getRandomVersion(char *cp)
{
	sprintf(cp, "%d.%d.%d.%d", rand() % 256, rand() % 256, rand() % 256, rand() % 256);
	return cp;
}

void test1(int passes)
{
	int i;
	char buf[2048];
	char sys[2048];

	for (i = 0; i < passes; i++) {
		sprintf(sys, "./fwtag -d %s\n", getRandomTag(buf));
		printf("%s", sys);
		system(sys);
		printf("\n");
	}
}


void test2(int passes)
{
	int i;
	char buf[2048];
	char vbuf[2048];
	char sys[2048];

	for (i = 0; i < passes; i++) {
		sprintf(sys, "./fwtag -w foo %s %s\n", getRandomTag(buf), getRandomVersion(vbuf));
		printf("%s", sys);
		system(sys);
		printf("\n");
	}
}

void test3(int passes)
{
	int i;
	int size;
	char buf[2048];
	char vbuf[2048];
	char sys[2048];
	char output[2048];
	char sys2[] = "./fwtag -r foo > foo2\n";
	char output2[2048];
	char *cp = buf;

	FILE *file;

	for (i = 0; i < passes; i++) {
		sprintf(sys, "./fwtag -w foo -t %s -v %s > foo3\n",
			getRandomTag(buf), getRandomVersion(vbuf));
		system(sys);
		system(sys2);

		sprintf(sys, "%s %s\n", buf, vbuf);
		sprintf(output2, "%s Version: %s ", buf, vbuf);

		file = fopen("foo2", "rb");

		if (file == NULL) {
			perror("foo2");
			exit(-3);
		}

		size = fread(output, 1, sizeof(output), file);

		if (ferror(file)) {
			perror("foo2");
			exit(-3);
		}
		output[size] = 0;

		cp = strstr(output, "CRC:");
		if (cp)
			*cp = 0;


		if (strcmp(output2, &output[5]) || strstr(&output[5], "unknown"))
			printf("failed on:\n%s%s\n", output2, &output[5]);

		fclose(file);
	}
}
