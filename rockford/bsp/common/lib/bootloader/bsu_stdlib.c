/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <stdio.h>
#include <string.h>
#ifdef UCOS
#include <os.h>
#endif
#include "bstd.h"
#include "bsu-api.h"
extern struct bsu_api *xapi;
extern void dbg_print(char *);
#ifdef UCOS
OS_SEM malloc_sem;
OS_SEM calloc_sem;
OS_SEM realloc_sem;
OS_SEM free_sem;
#endif
#define PRINTF_BUF_SIZE	2048

#if MIPS_BSU_HEAP_ADDR == 0
static int init=0;
#endif

char *strchr(const char *s, int c);
size_t strlen ( const char * str );
#if MIPS_BSU_HEAP_ADDR == 0
void * memset ( void * ptr, int value, size_t num );
#endif

void stdlib_init(void)
{
#ifdef UCOS
    OS_ERR err;
    /* initialize semaphores for stdlib supported functions */
    OSSemCreate(&malloc_sem, "Malloc Semaphore", 1, &err);
    OSSemCreate(&calloc_sem, "Calloc Semaphore", 1, &err);
    OSSemCreate(&realloc_sem, "Realloc Semaphore", 1, &err);
    OSSemCreate(&free_sem, "Free Semaphore", 1, &err);
#endif
}


int lib_xtoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x'))
        dest += 2;

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9'))
            digit = *dest - '0';
        else if ((*dest >= 'A') && (*dest <= 'F'))
            digit = 10 + *dest - 'A';
        else if ((*dest >= 'a') && (*dest <= 'f'))
            digit = 10 + *dest - 'a';
        else
            break;

        x *= 16;
        x += digit;
        dest++;
    }

    return x;
}

int __char_in(char c, const char *s)
{
    while (*s) {
        if (c == *s)
            return 1;
        s++;
    }
    return 0;
}

char *strtok(char *str, const char *delim)
{
    static char *s;
    char *p;

    if (str)
        s = str;
    p = s;

    while (*s) {
        if (__char_in(*s, delim)) {
            *s = '\0';
            s++;
            break;
        }
        s++;
    }

    if (*p)
        return p;

    return NULL;
}

size_t strspn(const char *s, const char *accept)
{
    size_t c = 0;

    while (*s) {
        if (NULL == strchr(accept, *s))
            break;
        c++; s++;
    }
    return c;
}


static long int __bsu_xtol(const char *dest, char **ep)
{
    long int x = 0;
    unsigned int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x'))
        dest += 2;

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9'))
            digit = *dest - '0';
        else if ((*dest >= 'A') && (*dest <= 'F'))
            digit = 10 + *dest - 'A';
        else if ((*dest >= 'a') && (*dest <= 'f'))
            digit = 10 + *dest - 'a';
        else
            break;

        x *= 16;
        x += digit;
        dest++;
    }

    if (ep)
        *ep = (char *)dest;

    return x;
}

long int bsu_xtol(const char *dest)
{
    return __bsu_xtol(dest, NULL);
}

#undef atol
long int atol(const char *s)
{
    long int x = 0;
    int neg  = 0, digit;

    if ((*s == '0') && (*(s + 1) == 'x'))
        return bsu_xtol(s);

    if (*s == '-') {
        neg = 1;
        s++;
    }

    while (*s) {
        if ((*s >= '0') && (*s <= '9'))
            digit = *s - '0';
        else
            break;

        x *= 10;
        x += digit;
        s++;
    }

    return neg ? -x : x;
}

void itoa(int n, char *buffer)
{
    int tmp = n;
    unsigned int digit = 1;

    /* count digits */
    while ((tmp = (tmp / 10)))
        digit++;

    if (n < 0) {
        buffer[0] = '-';
        n = -n;
        digit++;
    }

    /* work backwards from the end of the string */
    buffer[digit--] = '\0';

    do {
        buffer[digit--] = ((n) - (10 * (n / 10))) + '0';
        n = n / 10;
    } while (n);
}

#define bsu_isspace(x) (((x) == ' ') || ((x) == '\t'))


static int checkbase(char c, int base)
{
    char a = '0' + (unsigned)(base);

    return (c < '0') || (c >= a);
}

static long int bsu_strtol(const char *nptr, char **endptr, int base)
{
    long int x = 0;
    int neg  = 0, digit;
    int maybehex = ((base == 16) || (base == 0));

    while (1) {
        *endptr = (char *)nptr;

        if (*nptr == '\0')
            return 0;

        if (!bsu_isspace(*nptr))
            break;
        nptr++;
    }

    if (*nptr == '-') {
        neg = 1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    } else {
    }

    if (*nptr == '0') {
        nptr++;
        if (*nptr == 'x') {
            if (!maybehex) {
                *endptr = (char *)nptr;
                return 0;
            } else {
                nptr++;
                x = __bsu_xtol(nptr, endptr);
                return  neg ? -x : x;
            }
        }

        if (0 == base)
            base = 8;
    }

    if (16 == base) {
        x = __bsu_xtol(nptr, endptr);
        return  neg ? -x : x;
    }

    if (0 == base)
        base = 10;

    if ((base < 2) || (base > 36)) {
        *endptr = NULL;
        return 0;
    }

    while (*nptr) {
        if (!checkbase(*nptr, base))
            digit = *nptr - '0';
        else
            break;

        x *= base;
        x += digit;
        nptr++;
    }

    *endptr = (char *)nptr;

    return neg ? -x : x;
}


long int strtol(const char *nptr, char **endptr, int base)
{
    return bsu_strtol(nptr, endptr, base);
}


unsigned long int strtoul(const char *nptr, char **endptr, int base)
{
    return (unsigned long int)bsu_strtol(nptr, endptr, base);
}

char *strncat(char *dest, const char *src, size_t n)
{
    size_t i, len = strlen(dest);

    for (i = 0; i < n; i++) {
        if ('\0' == *src)
            break;

        dest[len + i] = *src;
        src++;
    }

    dest[len + i] = '\0';

    return dest;
}

#if MIPS_BSU_HEAP_ADDR == 0
/* thread-safe */
void *malloc(size_t size)
{
    void *p;
    OS_ERR err;
    CPU_TS ts;

    if (!init) {
        init=1;
        stdlib_init();
    }

    OSSemPend(&malloc_sem, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    p = xapi->xfn_malloc(size);
    OSSemPost(&malloc_sem, OS_OPT_POST_1, &err);
    return p;
}

/* thread-safe */
void free(void *ptr)
{
    OS_ERR err;
    CPU_TS ts;

    if (!init) {
        init=1;
        stdlib_init();
    }

    OSSemPend(&free_sem, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    xapi->xfn_free(ptr);
    OSSemPost(&free_sem, OS_OPT_POST_1, &err);
    return;
}

void *calloc(size_t nmemb, size_t size)
{
    size_t s = nmemb * size;
    void *p = NULL;
    OS_ERR err;
    CPU_TS ts;

    if (s) {
        OSSemPend(&calloc_sem, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
        p = xapi->xfn_malloc(s);
        OSSemPost(&calloc_sem, OS_OPT_POST_1, &err);
    }
    if (p)
        memset(p, '\0', s);

    return p;
}

/* thread-safe */
void* realloc (void* ptr, size_t size)
{
    OS_ERR err;
    CPU_TS ts;

    if (!init) {
        init=1;
        stdlib_init();
    }

    OSSemPend(&realloc_sem, 0, OS_OPT_PEND_BLOCKING, &ts, &err);
    BSTD_UNUSED(ptr);
    BSTD_UNUSED(size);
    dbg_print("realloc\n");
    OSSemPost(&realloc_sem, OS_OPT_POST_1, &err);
    return NULL;
}
#endif

#if USE_LOCAL_PRINTF==0
/* thread-safe? */
int printf(const char *format, ...)
{
    va_list marker;
    int count;
    char buffer[PRINTF_BUF_SIZE];

    va_start(marker, format);
    count = xapi->xfn_vsnprintf(buffer, PRINTF_BUF_SIZE, format, marker);
    va_end(marker);

    if (xapi->xfn_xprinthook)
        (*xapi->xfn_xprinthook) (buffer);

    return count;
}
#endif

int sprintf(char *buf, const char *format, ...)
{
    va_list marker;
    int count;

    va_start(marker, format);
    count = xapi->xfn_vsprintf(buf, format, marker);
    va_end(marker);

    return count;
}

int snprintf(char *buf, /*int*/ size_t len, const char *templat, ...)
{
    va_list marker;
    int count;

    va_start(marker, templat);
    count = xapi->xfn_vsnprintf(buf, len, templat, marker);
    va_end(marker);

    return count;
}

/* thread-safe? */
int fprintf(FILE *stream, const char *format, ...)
{
    va_list marker;
    int count;
    char buffer[PRINTF_BUF_SIZE];

    BSTD_UNUSED(stream);
    va_start(marker, format);
    count = xapi->xfn_vsnprintf(buffer, PRINTF_BUF_SIZE, format, marker);
    va_end(marker);

    if (xapi->xfn_xprinthook)
        (*xapi->xfn_xprinthook) (buffer);

    return count;
}

int fflush(FILE *stream)
{
    BSTD_UNUSED(stream);
    return 0;
}

int _filbuf(FILE *iop)
{
    BSTD_UNUSED(iop);
    return 0;
}

size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream)
{
    BSTD_UNUSED(ptr);
    BSTD_UNUSED(size);
    BSTD_UNUSED(nitems);
    BSTD_UNUSED(stream);
    return 0;
}

/* thread-safe? */
#if 0 /* mips_sde & mips_sde_stdlib */    //defined(MIPS_SDE) && !defined(MIPS_SDE_STDLIB)
int vprintf2(const char *templat, va_list marker)
{
    return xapi->xfn_vprintf(templat, marker);
}
#else
int vprintf(const char *templat, va_list marker)
{
    return xapi->xfn_vprintf(templat, marker);
}
#endif

/* thread-safe? */
int vsprintf(char *outbuf, const char *templat, va_list marker)
{
    return xapi->xfn_vsprintf(outbuf, templat, marker);
}

/* thread-safe? */
int vsnprintf(char *outbuf, size_t/*int*/ n, const char *templat, va_list marker)
{
    return xapi->xfn_vsnprintf(outbuf, n, templat, marker);
}

unsigned int sleep(unsigned int seconds)
{
    xapi->xfn_msleep(seconds*1000);
    return 0;
}

#undef getchar
int getchar(void)
{
    unsigned char c;

    while (xapi->xfn_console_read(&c, 1) != 1)
        xapi->xfn_poll_task();

    return (int)c;
}

char *gets(char *str)
{
    int len = 256;
    int v, i, l = len - 1; /* reserve space for '\0' char */

    if (l < 1)
        return NULL;

    for (i = 0; i < l; i++)	{
        v = xapi->xfn_console_readkey();
        switch (v) {
            case 0x0d: /* CR or LF */
            case 0x0a:
            case 0x04: /* ^D */
                str[i] = '\0';
                return str;
            default:
                str[i] = (char)v;
                break;
        }
    }

    str[l] = '\0';
    return str;
}
