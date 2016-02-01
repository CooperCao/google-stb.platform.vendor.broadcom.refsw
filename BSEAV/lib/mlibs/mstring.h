/***************************************************************************
 *     (c)2012-2015 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#ifndef MSTRING_H
#define MSTRING_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Some macros for printing out 64-bit integers with C89 (because it doesn't have the %lld type of format. */

/* Example:
 *      \* C89 also doesn't have "LL" (64-bit) constants, so  do this to initialize
 *       * u64var to 9223372036854775807 (largest positive signed 64-bit integer): *\
 *      uint64_t    u64var =  9*MLIBS_P_1E18U +_223372036*MLIBS_P_1E9 + 854775807) ;
 *      printf( "u64var: "MLIBS_U64_FMT"\n", MLIBS_U64_ARG(u64var));
 *
 * Will print: "u64var: 9223372036854775807"
 **/
#define MLIBS_NEG( num )    ((num) > 0 ? -(num) : (num))          /* Returns negative absolute value of num. */
#define MLIBS_P_1E9         (1000000000)                          /* 1x10**9  */
#define MLIBS_P_1E18U       ((uint64_t)1000000000 * 1000000000)   /* Unsigned 1*10**18 */
#define MLIBS_P_1E18S       ((int64_t)1000000000 * 1000000000)    /* Signed 1*10**18 */

/* Unsigned versions: */
#define   MLIBS_U64_FMT  "%.*u%.*u%0*u"
#define   MLIBS_U64_ARG(num)                                                                                                                 \
            (num>=MLIBS_P_1E18U ? 1 : 0),                           (num>=MLIBS_P_1E18U ? (unsigned int)(num/MLIBS_P_1E18U) : 0),                \
            (num>=MLIBS_P_1E18U ? 9 : (num>=MLIBS_P_1E9 ? 1 : 0)) , (num>=MLIBS_P_1E9   ? (unsigned int)((num%MLIBS_P_1E18U)/MLIBS_P_1E9) : 0),  \
            (num>=MLIBS_P_1E9   ? 9 : 1) ,                          (num>=MLIBS_P_1E9   ? (unsigned int)(num%MLIBS_P_1E9) : (unsigned int)(num))

/* Signed versions: */
#define   MLIBS_I64_FMT  "%.*s%.*u%.*u%0*u"
#define   MLIBS_I64_ARG(num)                                                                                                                                                               \
            (num<0 ? 1 : 0),                                                               (num<0 ? "-" : ""),                                                                             \
            (MLIBS_NEG(num)<=-MLIBS_P_1E18S ? 1 : 0),                                      (MLIBS_NEG(num)<=-MLIBS_P_1E18S ? -(unsigned int)(MLIBS_NEG(num)/MLIBS_P_1E18S) : 0),               \
            (MLIBS_NEG(num)<=-MLIBS_P_1E18S ? 9 : (MLIBS_NEG(num)<=-MLIBS_P_1E9 ? 1 : 0)), (MLIBS_NEG(num)<=-MLIBS_P_1E9   ? -(unsigned int)((MLIBS_NEG(num)%MLIBS_P_1E18S)/MLIBS_P_1E9) : 0), \
            (MLIBS_NEG(num)<=-MLIBS_P_1E9   ? 9 : 1),                                      (MLIBS_NEG(num)<=-MLIBS_P_1E9   ? -(unsigned int)(MLIBS_NEG(num)%MLIBS_P_1E9) : -(unsigned int)(MLIBS_NEG(num)))

#ifdef __vxworks
    #include <vxWorks.h>
    #ifdef __cplusplus
        extern "C" {
    #endif
    int     snprintf (char *, size_t, const char *, ...);
    extern int  strcasecmp (const char *, const char *);
    extern int  strncasecmp (const char *, const char *, size_t);
    #ifdef __cplusplus
        }
    #endif
#endif // __vxworks

#ifdef _MSC_VER // MSVC
    #ifndef _WIN32_WCE
        #define strcasecmp  stricmp
        #define strncasecmp strnicmp
        #define snprintf    _snprintf
    #else
        #define strcasecmp  _stricmp
        #define strncasecmp _strnicmp
        #define snprintf    _snprintf
    #endif
#endif

class MRegExp;

class MString {
public:
    MString();
    MString(const char *str, int len = -1);
    MString(const MString &str);
    MString(int32_t i);
    MString(int64_t i64);
    MString(short s);
    MString(float f);
    MString(double d);
    MString(uint32_t u);
    MString(uint64_t ul);
    MString(unsigned short us);
    virtual ~MString();

    const char *s() const {return _s;}
    // NOTE: until I implement a better memory management scheme, copy()
    // is the same as data().
    const char *copy() const {return _s;}

    bool isNull() const {return _len == 0;}
    bool isEmpty() const {return isNull();}
    void clear();

    /**
    * Set the memory to be at least the specified
    * number of bytes. Returns the number of bytes
    * currently allocated after operation is done.
    */
    int allocate(int mem);

    int length() const {return _len;}
    void setLength(int length, char padchar = '\0');
    void truncate(int length) {setLength(length);}

    // editing methods
    void insert(int index, char ch);
    bool removeChar(int index);

    /**
    * Converts string to integer. Obeys hex (0x) and octal (0) prefixes.
    */
    int toInt() const;
    long toLong() const;
    short toShort() const;
    char toChar() const;
    float toFloat() const;
    double toDouble() const;
    int64_t toLongLong() const;

    void strncpy(const char *str, int len = -1);
    void strncat(const char *str, int len = -1);
    int strncmp(const char *str, int len = -1) const;
    int strncasecmp(const char *str, int len = -1) const;
    int replace(char oldch, char newch);

    int find(const char *str, int index = 0, bool caseSensitive = true) const;
    int find(char ch, int index = 0, bool caseSensitive = true) const;
    int findRev(const char *str, int index = -1, bool caseSensitive = true) const;
    int findRev(char ch, int index = -1, bool caseSensitive = true) const;
    int find(const MRegExp &reg) const;

    /**
    * I have a separate mid(int index) method because I can return
    * a const char * instead of copying data.
    */
    const char *mid(int index) const;
    MString mid(int index, int length) const;
    MString left(int len) const;
    const char *right(int len) const;

    MString stripWhiteSpace() const;
    MString stripChars(const char *charsToRemove) const;
    MString lower() const;
    MString upper() const;

    // operators
    MString &operator +=(const char *str) {strncat(str);return *this;}
    MString &operator +=(char ch) {insert(-1,ch);return *this;}
    MString &operator +=(const MString &str) {strncat(str, str.length());return *this;}
    MString &operator =(const char *str) {strncpy(str);return *this;}
    MString &operator =(const MString &str) {strncpy(str, str.length());return *this;}
    char operator [](int index);
    operator const char *() const {return _s;}
    //operator bool() const {return !isNull();}

protected:
    char *_s;
    int _len;
    int _mem;
};

inline bool operator == (const char *str1, const MString &str2) {
    return !str2.strncmp(str1);
}
inline bool operator == (const MString &str1, const char *str2) {
    return !str1.strncmp(str2);
}
inline bool operator == (const MString &str1, const MString &str2) {
    return !str2.strncmp(str1, str1.length()+1);
}
inline bool operator != (const MString &str1, const char *str2) {
    return str1.strncmp(str2) ? true:false;
}
inline bool operator != (const char *str1, const MString &str2) {
    return str2.strncmp(str1) ? true:false;
}
inline MString operator + (const char *str1, const MString &str2) {
    return MString(str1) += str2;
}
inline MString operator + (const MString &str1, const char *str2) {
    return MString(str1) += str2;
}
inline MString operator + (const MString &str1, const MString &str2) {
    return MString(str1) += str2;
}

char *strskip(const char *str, const char *chars);

#endif
