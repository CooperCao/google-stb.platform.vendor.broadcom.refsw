/***************************************************************************
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bsu-api.h"
extern struct bsu_api *xapi;
extern void dbg_print(char *);

size_t strlen(const char *str);
char toupper(char c);
int xtoi(const char *dest);
uint64_t xtoq(const char *dest);
int strncmp(const char *dest, const char *src, size_t cnt);

char *strcpy(char *dest, const char *src)
{
    char *ptr = dest;

    while (*src)
        *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

char *strncpy(char *dest, const char *src, size_t cnt)
{
    char *ptr = dest;

    while (*src && (cnt > 0)) {
        *ptr++ = *src++;
        cnt--;
    }
    if (cnt > 0)
        *ptr = '\0';

    return dest;
}

size_t lib_xstrncpy(char *dest, const char *src, size_t cnt)
{
    char *ptr = dest;
    size_t copied = 0;

    while (*src && (cnt > 1)) {
        *ptr++ = *src++;
        cnt--;
        copied++;
    }
    *ptr = '\0';

    return copied;
}

size_t strlen(const char *str)
{
    size_t cnt = 0;

    while (*str) {
        str++;
        cnt++;
    }

    return cnt;
}

int strcmp(const char *dest, const char *src)
{
    while (*src && *dest) {
        if (*dest < *src)
            return -1;
        if (*dest > *src)
            return 1;
        dest++;
        src++;
    }

    if (*dest && !*src)
        return 1;
    if (!*dest && *src)
        return -1;
    return 0;
}

int strncmp(const char *dest, const char *src, size_t cnt)
{
    size_t i = 0;
    if (!cnt) /* SWBOLT-161 */
        return 0;
    while ((*src && *dest) && (i < (cnt - 1))) {
        if (*dest < *src)
            return -1;
        if (*dest > *src)
            return 1;
        dest++;
        src++;
        i++;
    }

    if (*dest == *src)
        return 0;
    else if (*dest > *src)
        return 1;
    else
        return -1;
}

int strcmpi(const char *dest, const char *src)
{
    char dc, sc;

    while (*src && *dest) {
        dc = toupper(*dest);
        sc = toupper(*src);
        if (dc < sc)
            return -1;
        if (dc > sc)
            return 1;
        dest++;
        src++;
    }

    if (*dest && !*src)
        return 1;
    if (!*dest && *src)
        return -1;
    return 0;
}

char *strchr(const char *dest, int c)
{
    while (*dest) {
        if (*dest == c)
            return (char *)dest;
        dest++;
    }
    return NULL;
}

char *strstr(const char *dest, const char *find)
{
    char c, sc;
    size_t len;
    char *s = (char *)dest;

    c = *find++;
    if (c != 0) {
        len = strlen(find);
        do {
            do {
                sc = *s++;
                if (sc == 0)
                    return NULL;
            } while (sc != c);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return s;
}

char *strnchr(const char *dest, int c, size_t cnt)
{
    while (*dest && (cnt > 0)) {
        if (*dest == c)
            return (char *)dest;
        dest++;
        cnt--;
    }
    return NULL;
}

char *strrchr(const char *dest, int c)
{
    char *ret = NULL;

    while (*dest) {
        if (*dest == c)
            ret = (char *)dest;
        dest++;
    }

    return ret;
}

int memcmp(const void *dest, const void *src, size_t cnt)
{
    const unsigned char *d;
    const unsigned char *s;

    d = (const unsigned char *)dest;
    s = (const unsigned char *)src;

    while (cnt) {
        if (*d < *s)
            return -1;
        if (*d > *s)
            return 1;
        d++;
        s++;
        cnt--;
    }

    return 0;
}

void *memcpy(void *dest, const void *src, size_t cnt)
{
    unsigned char *d;
    const unsigned char *s;

    d = (unsigned char *)dest;
    s = (const unsigned char *)src;

    while (cnt) {
        *d++ = *s++;
        cnt--;
    }

    return dest;
}

void *memmove(void *dest, const void *src, size_t n)
{
    void *tmp;
        tmp = xapi->xfn_malloc(n);
    if (!tmp)
        return NULL;
    memcpy(tmp, src, n);
    memcpy(dest, tmp, n);
    xapi->xfn_free(tmp);
    return dest;
}

void *memchr(const void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n) {
        if (*p == c)
            return p;
        p++; n--;
    }
    return NULL;
}


void *memset(void *dest, int c, size_t cnt)
{
    unsigned char *d;

    d = dest;

    while (cnt) {
        *d++ = (unsigned char)c;
        cnt--;
    }

    return d;
}

char toupper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
        c -= 32;
    return c;
}

void strupr(char *str)
{
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

char *strcat(char *dest, const char *src)
{
    char *ptr = dest;

    while (*ptr)
        ptr++;
    while (*src)
        *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

#define isspace(x) (((x) == ' ') || ((x) == '\t'))

char *lib_gettoken(char **ptr)
{
    char *p = *ptr;
    char *ret;

    /* skip white space */

    while (*p && isspace(*p))
        p++;
    ret = p;

    /* check for end of string */

    if (!*p) {
        *ptr = p;
        return NULL;
    }

    /* skip non-whitespace */

    while (*p) {
        if (isspace(*p))
            break;

        /* do quoted strings */

        if (*p == '"') {
            p++;
            ret = p;
            while (*p && (*p != '"'))
                p++;
            if (*p == '"')
                *p = '\0';
        }

        p++;

    }

    if (*p)
        *p++ = '\0';

    *ptr = p;

    return ret;
}

#undef atoi
int atoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x'))
        return xtoi(dest + 2);

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9'))
            digit = *dest - '0';
        else
            break;

        x *= 10;
        x += digit;
        dest++;
    }

    return x;
}

uint64_t atoq(const char *s)
{
    uint64_t x = 0;
    int digit;

    if ((*s == '0') && (*(s + 1) == 'x'))
        return xtoq(s);

    while (*s) {
        if ((*s >= '0') && (*s <= '9'))
            digit = *s - '0';
        else
            break;

        x *= 10;
        x += digit;
        s++;
    }

    return x;
}

uint64_t xtoq(const char *dest)
{
    uint64_t x = 0;
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

    return x;
}

int xtoi(const char *dest)
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
