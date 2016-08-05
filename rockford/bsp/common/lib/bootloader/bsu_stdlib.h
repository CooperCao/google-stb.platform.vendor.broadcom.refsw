/******************************************************************************
* Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/

//char *strchr(const char *s, int c);
//size_t strlen ( const char * str );

//#if MIPS_BSU_HEAP_ADDR == 0
//void * memset ( void * ptr, int value, size_t num );
//#endif

void stdlib_init(void);
int lib_xtoi(const char *dest);
int __char_in(char c, const char *s);
//char *strtok(char *str, const char *delim);
size_t strspn(const char *s, const char *accept);
//static long int __bsu_xtol(const char *dest, char **ep);
long int bsu_xtol(const char *dest);
#undef atol
long int atol(const char *s);
void itoa(int n, char *buffer);

//static int checkbase(char c, int base);
//static long int bsu_strtol(const char *nptr, char **endptr, int base);
long int strtol(const char *nptr, char **endptr, int base);
unsigned long int strtoul(const char *nptr, char **endptr, int base);
char *strncat(char *dest, const char *src, size_t n);

//#if MIPS_BSU_HEAP_ADDR == 0
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void* realloc (void* ptr, size_t size);
//#endif

int printf(const char *format, ...);
int sprintf(char *buf, const char *format, ...);
int snprintf(char *buf, /*int*/ size_t len, const char *templat, ...);
int fprintf(FILE *stream, const char *format, ...);
int fflush(FILE *stream);
int _filbuf(FILE *iop);
size_t fread(void *ptr, size_t size, size_t nitems, FILE *stream);

#if 0 /* mips_sde & mips_sde_stdlib */    //defined(MIPS_SDE) && !defined(MIPS_SDE_STDLIB)
int vprintf2(const char *templat, va_list marker);
#else
int vprintf(const char *templat, va_list marker);
#endif

int vsprintf(char *outbuf, const char *templat, va_list marker);
int vsnprintf(char *outbuf, size_t/*int*/ n, const char *templat, va_list marker);
unsigned int sleep(unsigned int seconds);
#undef getchar
int getchar(void);
char *gets(char *str);
