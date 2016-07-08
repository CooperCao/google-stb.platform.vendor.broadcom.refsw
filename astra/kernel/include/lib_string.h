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

#ifndef __LIB_STRING_H__
#define __LIB_STRING_H__

#include "lib_types.h"

#ifdef __cplusplus
extern "C" {
#endif

char *lib_strcpy(char *dest, const char *src);
char *lib_strncpy(char *dest, const char *src, size_t cnt);
size_t lib_xstrncpy(char *dest, const char *src, size_t cnt);
size_t lib_strlen(const char *str);

int lib_strcmp(const char *dest, const char *src);
int lib_strcmpi(const char *dest, const char *src);
char *lib_strchr(const char *dest, int c);
char *lib_strrchr(const char *dest, int c);
int lib_memcmp(const void *dest, const void *src, size_t cnt);
void *lib_memcpy(void *dest, const void *src, size_t cnt);
void *lib_memmove(void *dest, const void *src, size_t n);
void *lib_memchr(const void *s, int c, size_t n);
void *lib_memset(void *dest, int c, size_t cnt);
char *lib_strdup(const char *str);
void lib_strupr(char *s);
char lib_toupper(char c);
char *lib_strcat(char *dest, const char *src);
char *lib_gettoken(char **str);
char *lib_strnchr(const char *dest, int c, size_t cnt);
int lib_parseipaddr(const char *ipaddr, uint8_t * dest);
int lib_atoi(const char *dest);
int lib_lookup(const cons_t * list, char *str);
int lib_setoptions(const cons_t * list, char *str, unsigned int *flags);
int lib_xtoi(const char *dest);
uint64_t lib_xtoq(const char *dest);
char *lib_strstr(const char *dest, const char *find);
int lib_strncmp(const char *dest, const char *src, size_t cnt);

#ifndef _LIB_NO_MACROS_
#define strcpy(d,s) lib_strcpy(d,s)
#define strncpy(d,s,c) lib_strncpy(d,s,c)
#define xstrncpy(d,s,c) lib_xstrncpy(d,s,c)
#define strlen(s) lib_strlen(s)
#define strchr(s,c) lib_strchr(s,c)
#define strrchr(s,c) lib_strrchr(s,c)
#define strdup(s) lib_strdup(s)
#define strcmp(d,s) lib_strcmp(d,s)
#define strcmpi(d,s) lib_strcmpi(d,s)
#define memcmp(d,s,c) lib_memcmp(d,s,c)
#define memset(d,s,c) lib_memset(d,s,c)
#define memcpy(d,s,c) lib_memcpy(d,s,c)
#define memmove(d,s,c) lib_memmove(d,s,c)
#define memchr(s,c,n) lib_memchr(s,c,n)
#define bcopy(s,d,c) lib_memcpy(d,s,c)
#define bzero(d,c) lib_memset(d,0,c)
#define strupr(s) lib_strupr(s)
#define toupper(c) lib_toupper(c)
#define strcat(d,s) lib_strcat(d,s)
#define gettoken(s) lib_gettoken(s)
#define strnchr(d,ch,cnt) lib_strnchr(d,ch,cnt)
#define atoi(d) lib_atoi(d)
#define xtoi(d) lib_xtoi(d)
#define xtoq(d) lib_xtoq(d)
#define parseipaddr(i,d) lib_parseipaddr(i,d)
#define lookup(x,y) lib_lookup(x,y)
#define setoptions(x,y,z) lib_setoptions(x,y,z)
#define strstr(d,s) lib_strstr(d,s)
#define strncmp(d,s,c) lib_strncmp(d,s,c)

#endif

void
qsort(void *bot, size_t nmemb, size_t size,
      int (*compar) (const void *, const void *));

unsigned int lib_crc32(const unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif


#endif /* __LIB_STRING_H__ */
