/*
 * win32-specific utility code so <windows.h> won't conflict with <ntddk.h> .
 * 
 * $Id$
 */

#define	DBG	1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <typedefs.h>
#include <utils.h>


const char *progname = "";

/* For error checking */
int failcall = 0;
int curcall = 0;
int doingerrors = FALSE;
int randfail = 0;

void
utils_init()
{
}

void
assfail(char *exp, char *file, int line)
{
	fprintf(stderr, "assertion \"%s\" failed: file \"%s\", line %d\n",
		exp, file, line);
#ifdef EXT_CBALL
	{
		/* Ugly but very useful */
		extern uint armulator_last_pc;
		fprintf(stderr, "Armulator PC was 0x%x\n", armulator_last_pc);
	}
#endif

	abort();
}

void
pattern(uint8 *buf, uint len)
{
	uint i;

	for (i = 0; i < len; i++)
		buf[i] = i & 0x7f;
}

void *
mallocdeadbeef(size_t len)
{
	void *p;

	p = malloc(len);
	if (p)
		deadbeef(p, len);

	return (p);
}

#if !defined(BCMDRIVER) || !defined(BCMDBG) /* BCMDRIVER grabs deadbeef and phrex 
					     * from ./src/shared/bcmutils.c if BCMDBG is defined
					     */
void
deadbeef(void *buf, uint len)
{
	uint *w;

	/* skip 0-3bytes to get sizeof(int) aligned */
	while ((uintptr)buf & (sizeof(int)-1)) {
		*(uint8 *)buf = 0xde;
		buf = (uint8 *)buf + 1;
		len--;
	}

	w = (uint*) buf;
	while (len >= sizeof (int)) {
		*w++ = 0xdeadbeef;
		len -= sizeof (int);
	}
}

/* pretty hex print a contiguous buffer */
void
prhex(const char *msg, uchar *buf, uint nbytes)
{
	char line[128], *p;
	uint i;

	if (msg && (msg[0] != '\0'))
		printf("%s:\n", msg);

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			p += sprintf(p, "  %04d: ", i);	/* line prefix */
		}
		p += sprintf(p, "%02x ", buf[i]);
		if (i % 16 == 15) {
			printf("%s\n", line);		/* flush line */
			p = line;
		}
	}

	/* flush last partial line */
	if (p != line)
		printf("%s\n", line);
}
#endif

int
prmac(char *buf, uint8* addr)
{
	int n;
	
	n = sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", 
		    addr[0], addr[1], addr[2], 
		    addr[3], addr[4], addr[5]);
	return n;
}

int
reterror (void)
{
	static int initialized = 0;

	if (!initialized) {
		srand ((unsigned) time (NULL));
		initialized = 1;
	}

	if (doingerrors && (curcall++ == failcall)) {
		printf ("Failing return call %d\n", failcall);
		return 1;
	} else if (randfail) {
		if ( ((rand () * 100) / 0x7fff) < randfail)
			return 1;
	}
	return 0;
}


void
syserr(char *s)
{
	perror(s);
	exit(1);
}

#ifndef TARGETENV_freebsd
void
err(char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	vprintf(fmt, ap);
	vprintf("\n", NULL);
	exit(1);
}
#endif

void
xmalloc_set_program_name(const char *s)
{
	progname = s;
}

void *
xmalloc (unsigned long size)
{
	void *newmem;

	if (size == 0)
		size = 1;
	newmem = malloc (size);
	ASSERT (newmem);
	return (newmem);
}


void *
xrealloc (void *oldmem, unsigned long size)
{
	void *newmem;

	if (size == 0)
		size = 1;
	if (!oldmem)
		newmem = malloc (size);
	else
		newmem = realloc (oldmem, size);
	ASSERT (newmem);
	return (newmem);
}

char *
concat (const char *first, ...)
{
	int length;
	char *newstr;
	char *end;
	const char *arg;
	va_list args;

	/* First compute the size of the result and get sufficient memory. */

	va_start (args, first);

	if (first == NULL)
		length = 0;
	else {
		length = strlen (first);
		while ((arg = va_arg (args, const char *)) != NULL) {
			length += strlen (arg);
		}
	}
	newstr = (char *) xmalloc (length + 1);
	va_end (args);

	/* Now copy the individual pieces to the result string. */

	if (newstr != NULL) {
		va_start (args, first);
		end = newstr;
		if (first != NULL) {
			arg = first;
			while (*arg) {
				*end++ = *arg++;
			}
			while ((arg = va_arg (args, const char *)) != NULL) {
				while (*arg) {
					*end++ = *arg++;
				}
			}
		}
		*end = '\000';
		va_end (args);
	}

	return (newstr);
}

int
access(const char *file, int acc)
{
	FILE	*f;
	char	*mode;
	int	rc;

	if (acc & W_OK)
		mode = "r+";
	else
		mode = "r";

	f = fopen(file, mode);

	if (f == NULL)
		rc = -1;
	else {
		fclose (f);
		rc = 0;
	}

	return (rc);
}

#if !defined(TARGETENV_freebsd) && !defined(_RTE_SIM_) && !defined(linux) && \
	!defined(__APPLE__)
void
bcopy(const void *src, void *dst, unsigned int len)
{
	memcpy(dst, src, len);
}

int
bcmp(const void *b1, const void *b2, unsigned int len)
{
	return (memcmp(b1, b2, len));
}

void
bzero(void *b, unsigned int len)
{
	memset(b, '\0', len);
}
#endif

/*
 * compute the contiguous byte field size
 * from the total number of words, first byte enable,
 * and last byte enable.
 */
int
fieldsize(uint size, uint fe, uint le)
{
	static int beconv[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	int nbytes;

	ASSERT(beconv[fe] != -1);
	ASSERT(beconv[le] != -1);

	nbytes = size * sizeof (int);
	nbytes -= beconv[fe];

	if (size > 1)
		nbytes -= beconv[le];

	return (nbytes);
}

/*
 * compute the byte mask from a given byte enable
 */
int
fieldmask(uint be)
{
	uint mask = 0;
	uint byte = 0;

	ASSERT(be < 16);

	for (byte = 0; byte < 4; byte++) {
		if ((be & (1<<byte)) == 0) {
			mask |= 0xff << (byte*8);
		}
	}

	return mask;
}

/*
 * compute the byte offset from a given byte enable
 */
int
fieldoff(uint be)
{
	static int beoff[] = {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 0};
	
	ASSERT(be < 16);

	return beoff[be];
}

int
atomac(char* mac_string, uint8* mac)
{
	int len, num = 0;
	uint8 mac_buf[6];
	uint8* m = mac_buf;
	char* s = mac_string;
	
	/* skip leading space */
	while (isspace((int)*s)) s++;
		
	len = strlen(mac_string);
	if (len == 0)
		return -1;

	if (strchr(s, ':')) {
		/* mac addr with ':' delimiters */
		char* e;
		while (len > 0) {
			if (num == 6) return -1;
			
			e = strchr(s, ':');
			if (e == NULL) e = s+len;
			/* verify that we have 1 or 2 characters */
			if (e-s != 1 && e-s != 2)
				return -1;
			/* verify that the digits are hex */
			if (!isxdigit((int)s[0]) || (e-s == 2 && !isxdigit((int)s[1])))
				return -1;

			*m++ = (uint8)strtoul(s, NULL, 16);
			num++;
			if (*e == ':') e++;
			len -= e-s;
			s = e;
		}
	} else {
		/* mac addr as a string of hex digits */
		char buf[3];
		while (len > 0) {
			if (num == 6) return -1;
			
			if (len < 2) return -1;
			/* verify that we have 2 hex digits */
			if (!isxdigit((int)s[0]) || !isxdigit((int)s[1]))
				return -1;
			
			buf[0] = s[0];
			buf[1] = s[1];
			buf[2] = '\0';
			
			*m++ = (uint8)strtoul(buf, NULL, 16);
			num++;
			len -= 2;
			s = s+2;
		}
	}
	
	if (num == 0) 
		return -1;
	
	/* copy the bytes we parsed to the end of the mac addr */
	bcopy(mac_buf, mac + (6-num), num);

	return 0;
}
