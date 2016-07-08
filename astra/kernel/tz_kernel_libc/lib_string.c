/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 ***************************************************************************/

#include "lib_types.h"
#define _LIB_NO_MACROS_
#include "lib_string.h"

char *lib_strcpy(char *dest, const char *src)
{
    char *ptr = dest;

    while (*src)
        *ptr++ = *src++;
    *ptr = '\0';

    return dest;
}

char *lib_strncpy(char *dest, const char *src, size_t cnt)
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

size_t lib_strlen(const char *str)
{
    size_t cnt = 0;

    while (*str) {
        str++;
        cnt++;
    }

    return cnt;
}

#ifndef LIBSTRING_ASM
int lib_strcmp(const char *dest, const char *src)
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
#endif

int lib_strncmp(const char *dest, const char *src, size_t cnt)
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

int lib_strcmpi(const char *dest, const char *src)
{
    char dc, sc;

    while (*src && *dest) {
        dc = lib_toupper(*dest);
        sc = lib_toupper(*src);
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

char *lib_strchr(const char *dest, int c)
{
    while (*dest) {
        if (*dest == c)
            return (char *)dest;
        dest++;
    }
    return NULL;
}

char *lib_strstr(const char *dest, const char *find)
{
    char c, sc;
    size_t len;
    char *s = (char *)dest;

    if ((c = *find++) != 0) {
        len = lib_strlen(find);
        do {
            do {
                if ((sc = *s++) == 0)
                    return NULL;
            } while (sc != c);
        } while (lib_strncmp(s, find, len) != 0);
        s--;
    }
    return s;
}

char *lib_strnchr(const char *dest, int c, size_t cnt)
{
    while (*dest && (cnt > 0)) {
        if (*dest == c)
            return (char *)dest;
        dest++;
        cnt--;
    }
    return NULL;
}

char *lib_strrchr(const char *dest, int c)
{
    char *ret = NULL;

    while (*dest) {
        if (*dest == c)
            ret = (char *)dest;
        dest++;
    }

    return ret;
}

int lib_memcmp(const void *dest, const void *src, size_t cnt)
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

#ifndef LIBSTRING_ASM_MEMCPY
void *lib_memcpy(void *dest, const void *src, size_t cnt)
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
#endif


#if 0
/* No comparisons, braindead.
*/
void *lib_memmove(void *dest, const void *src, size_t n)
{
    void *tmp;
    tmp = KMALLOC(n, 1);
    if (!tmp)
        return NULL;
    lib_memcpy(tmp, src, n);
    lib_memcpy(dest, tmp,n);
    KFREE(tmp);
    return dest;
}
#endif

void *lib_memmove(void *dest, const void *src, size_t n)
{
    char *dest_begin = dest;
        char *dest_end = (char *)dest + n;
        const char *src_begin = src;
        const char *src_end = (char *)src + n;

        const char *sptr = src_end;
        char *dptr = dest_end;

        if ((dest_end < src_begin) || (src_end < dest_begin))
            return lib_memcpy(dest, src, n);

        if (src_begin < dest_end)
            return lib_memcpy(dest, src, n);

        while (sptr >= src_begin)
            *dptr-- = *sptr--;

        return dest;
}

void *lib_memchr(const void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n)
    {
        if (*p == c)
            return p;
        p++; n--;
    }
    return NULL;
}



#ifndef LIBSTRING_ASM
void *lib_memset(void *dest, int c, size_t cnt)
{
    unsigned char *d;

    d = dest;

    while (cnt) {
        *d++ = (unsigned char)c;
        cnt--;
    }

    return d;
}
#endif

char lib_toupper(char c)
{
    if ((c >= 'a') && (c <= 'z'))
        c -= 32;
    return c;
}

void lib_strupr(char *str)
{
    while (*str) {
        *str = lib_toupper(*str);
        str++;
    }
}

char *lib_strcat(char *dest, const char *src)
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

    if (*p) {
        *p++ = '\0';
    }
    *ptr = p;

    return ret;
}

int lib_atoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x')) {
        return lib_xtoi(dest + 2);
    }

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9')) {
            digit = *dest - '0';
        } else {
            break;
        }
        x *= 10;
        x += digit;
        dest++;
    }

    return x;
}

uint64_t lib_xtoq(const char *dest)
{
    uint64_t x = 0;
    unsigned int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x'))
        dest += 2;

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9')) {
            digit = *dest - '0';
        } else if ((*dest >= 'A') && (*dest <= 'F')) {
            digit = 10 + *dest - 'A';
        } else if ((*dest >= 'a') && (*dest <= 'f')) {
            digit = 10 + *dest - 'a';
        } else {
            break;
        }
        x *= 16;
        x += digit;
        dest++;
    }

    return x;
}

int lib_xtoi(const char *dest)
{
    int x = 0;
    int digit;

    if ((*dest == '0') && (*(dest + 1) == 'x'))
        dest += 2;

    while (*dest) {
        if ((*dest >= '0') && (*dest <= '9')) {
            digit = *dest - '0';
        } else if ((*dest >= 'A') && (*dest <= 'F')) {
            digit = 10 + *dest - 'A';
        } else if ((*dest >= 'a') && (*dest <= 'f')) {
            digit = 10 + *dest - 'a';
        } else {
            break;
        }
        x *= 16;
        x += digit;
        dest++;
    }

    return x;
}

/**********************************************************************
 *  lib_crc32(buf, len)
 *
 *  Yes, this is an Ethernet CRC. I'm lazy.
 *
 *  Input parameters:
 *         buf - buffer to CRC
 *         len - length of data
 *
 *  Return value:
 *         CRC-32
 **********************************************************************/
#define CRC32_POLY  0xEDB88320UL /* CRC-32 Poly */
unsigned int lib_crc32(const unsigned char *buf, unsigned int len)
{
    unsigned int idx, bit, data, crc = 0xFFFFFFFFUL;

    for (idx = 0; idx < len; idx++) {
        for (data = *buf++, bit = 0; bit < 8; bit++, data >>= 1) {
            crc =
                (crc >> 1) ^ (((crc ^ data) & 1) ? CRC32_POLY : 0);
        }
    }

    return crc;
}
