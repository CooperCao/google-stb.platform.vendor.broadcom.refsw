/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 * Module Description:
 *
 **************************************************************************/
#include "mstring.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#if defined(__unix__)
    #include "mregexp.h"
#endif

MString::MString() {
    _s = NULL;
    _len = _mem = 0;
}

MString::MString(const char *str, int len) {
    _s = NULL;
    _mem = 0;
    strncpy(str, len);
}

MString::MString(const MString &str) {
    _s = NULL;
    _mem = 0;
    strncpy(str, str.length());
}

MString::MString(int32_t i) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), "%d", i);
    strncpy(buf);
}

MString::MString(int64_t i64) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), MLIBS_I64_FMT, MLIBS_I64_ARG(i64));
    strncpy(buf);
}

MString::MString(uint32_t u) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), "%u", u);
    strncpy(buf);
}

MString::MString(uint64_t u64) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), MLIBS_U64_FMT, MLIBS_U64_ARG(u64));
    strncpy(buf);
}

MString::MString(short s) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), "%hd", s);
    strncpy(buf);
}

MString::MString(unsigned short s) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), "%hu", s);
    strncpy(buf);
}

MString::MString(float f) {
    _s = NULL;
    _mem = 0;
    char buf[40];
    snprintf(buf, sizeof(buf), "%f", f);
    strncpy(buf);
}

MString::MString(double d) {
    _s = NULL;
    _mem = 0;
    char buf[80];
    snprintf(buf, sizeof(buf), "%e", d);
    strncpy(buf);
}

MString::~MString() {
    clear();
}

void MString::clear() {
    if (_s) delete[] _s;
    _s = NULL;
    _mem = _len = 0;
}

int MString::allocate(int mem) {
    if (_mem < mem) {
        _mem = mem;
        char *temp = new char[_mem];
        if (_s) {
            ::strncpy(temp, _s, _len);
            delete[] _s;
        }
        _s = temp;
    }
    return _mem;
}

void MString::strncpy(const char *str, int len) {
    if (!str) {
        clear();
        return;
    }

    if (len == -1)
        len = strlen(str);
    if (_mem < len+1) {
        if (_s) delete[] _s;
        _mem = len+1;
        _s = new char[_mem];
    }
    if(_s==NULL)
        return;
    _len = len;
    ::strncpy(_s, str, len);
    _s[_len] = 0;
}

void MString::strncat(const char *str, int len) {
    if (!str)
        return;

    if (len == -1)
        len = strlen(str);
    allocate(_len+len+1);
    ::strncpy(_s+_len, str, len);
    _len += len;
    _s[_len] = 0;
}

int MString::strncasecmp(const char *str, int len) const {
    if (!_s) {
        if (str == NULL || !*str)
            return 0;
        else
            return -1;
    }
    else {
        return ::strncasecmp(_s, str?str:"", (len!=-1)?len:(length()+1));
    }
}

int MString::strncmp(const char *str, int len) const {
    if (!_s) {
        if (str == NULL || !*str)
            return 0;
        else
            return -1;
    }
    else {
        return ::strncmp(_s, str?str:"", (len!=-1)?len:(length()+1));
    }
}

long MString::toLong() const {
    if (_s) {
        if (!strncmp("0x", 2)) {
            return strtoul(_s, NULL, 16);
        }
        else if (*_s == '0') {
            return strtol(_s, NULL, 8);
        }
        else {
            return strtol(_s, NULL, 10);
        }
    }
    else
        return 0;
}

int64_t MString::toLongLong() const {
    if (_s) {
        if (!strncmp("0x", 2)) {
            return strtoull(_s, NULL, 16);
        }
        else if (*_s == '0') {
            return strtoll(_s, NULL, 8);
        }
        else {
            return strtoll(_s, NULL, 10);
        }
    }
    else
        return 0;
}

short MString::toShort() const {
    return (short)toLong();
}

char MString::toChar() const {
    return (char)toLong();
}

int MString::toInt() const {
    if (sizeof(int) == sizeof(short))
        return (int)toShort();
    else
        return (int)toLong();
}

float MString::toFloat() const {
    if (_s)
        return (float)atof(_s);
    else
        return 0.0;
}

double MString::toDouble() const {
    if (_s)
        return atof(_s);
    else
        return 0.0;
}

void MString::setLength(int length, char padchar) {
    if (length < _len && length >= 0) {
        _len = length;
        _s[_len] = 0;
    }
    else if (length > _len) {
        int l = length-_len;
        char *temp = new char[l];
        for (int i=0;i<l;i++)
            temp[i] = padchar;
        strncat(temp, l);
        delete[] temp;
    }
}

void MString::insert(int index, char ch) {
    // normalize index
    if (index < 0)
        index = _len;

    // make sure we have memory for one more char
    if (_mem <= _len+1) {
        strncat(" ", 1);
        _len--;
    }

    // insert in the middle
    if (index < _len) {
        for (int i=_len;i>index;i--)
            _s[i] = _s[i-1];
        _s[index] = ch;
    }
    // append
    else {
        _s[_len] = ch;
    }
    _s[++_len] = 0;
}

bool MString::removeChar(int index) {
    if (index < 0) index = 0;
    // don't remove from end by default (is this right????)
    if (index >= _len) return false;
    // shift everything forward, including the null
    for (int i=index;i<_len;i++)
        _s[i] = _s[i+1];
    --_len;
    return true;
}

//#define HAS_MAIN
#ifdef HAS_MAIN
int main() {
    MString str;
    str = "this is a test";
    printf("%d: *%s*\n", str.length(), (const char *)str);
    str += " add more";
    printf("%d: *%s*\n", str.length(), (const char *)str);
    str = "replace";
    printf("%d: *%s*\n", str.length(), (const char *)str);
    str = MString("123456789", 5);
    printf("%d: *%s*\n", str.length(), (const char *)str);

    printf("%d %d %d\n",
        MString("0x10").toInt(),
        MString("010").toInt(),
        MString("10").toInt()
        );
    return 0;
}
#endif

char MString::operator [](int index) {
    if (_s && index >=0 && index < length())
        return _s[index];
    else
        return 0;
}

#if defined(__unix__)
int MString::find(const MRegExp &reg) const {
    return reg.match(*this);
}
#endif

int MString::findRev(const char *str, int index, bool caseSensitive) const {
    if (index == -1)
        index = length()-1;
    if (isNull() || index > length())
        return -1;
    int i = 0, lastfind = -1;
    while (i < index && (i = find(str, i, caseSensitive)) != -1) {
        lastfind = i;
        i++;
    }
    return lastfind;
}

int MString::find(const char *str, int index, bool caseSensitive) const {
    if (isNull() || index >= length())
        return -1;
    char *s;
    if (caseSensitive)
        s = strstr(&_s[index], str);
    else
        s = strcasestr(&_s[index], str);
    if (s)
        return s - _s;
    else
        return -1;
}

int MString::findRev(char ch, int index, bool caseSensitive) const {
    if (index == -1)
        index = length()-1;
    if (isNull() || length() <= index)
        return -1;
    if (!caseSensitive)
        ch = toupper(ch);
    while (index>=0) {
        if ((caseSensitive && _s[index] == ch) ||
            (!caseSensitive && toupper(_s[index]) == ch))
            return index;
        index--;
    }
    return -1;

}

int MString::find(char ch, int index, bool caseSensitive) const {
    if (isNull() || length() <= index)
        return -1;
    if (!caseSensitive)
        ch = toupper(ch);
    char *s = _s + index;
    while (*s) {
        if ((caseSensitive && *s == ch) ||
            (!caseSensitive && toupper(*s) == ch))
            return s-_s;
        s++;
    }
    return -1;
}

const char *MString::right(int len) const {
    if (length() >= len)
        return _s + length() - len;
    else
        return _s;
}

MString MString::left(int len) const {
    if (length() > len)
        return MString(_s, len);
    else
        return *this;
}

const char *MString::mid(int index) const {
    if (length() > index)
        return &_s[index];
    else
        return NULL;
}

MString MString::mid(int index, int len) const {
    if (length() > index) {
        return MString(&_s[index], len);
    }
    else
        return MString();
}

MString MString::lower() const {
    MString value = *this;
    char *s = _s;
    if (s)
    while (*s) {
        *s = tolower(*s);
        s++;
    }
    return value;
}

MString MString::upper() const {
    MString value = *this;
    char *s = _s;
    if (s)
    while (*s) {
        *s = toupper(*s);
        s++;
    }
    return value;
}

int MString::replace(char oldch, char newch) {
    int count = 0;
    if (_s) {
        char *s = _s;
        while (*s) {
            if (*s == oldch) {
                *s = newch;
                count++;
            }
            s++;
        }
    }
    return count;
}

MString MString::stripWhiteSpace() const {
    return stripChars(" \r\t\n");
}

MString MString::stripChars(const char *charsToRemove) const {
    MString value;
    if (!isNull()) {
        value.allocate(length()+1);
        char *source = _s;
        char *next;
        while ((next = strpbrk(source, charsToRemove))) {
            value.strncat(source, next-source);
            source += strspn(source, charsToRemove);
        }
        value.strncat(source, next-source);
    }
    return value;
}

char *strskip(const char *str, const char *chars) {
    return (char *)str + strspn(str, chars);
}

#if defined(__vxworks)  // These CRTL functions are missing in VxWorks

extern "C" void strupr(char* s)
{
    while(*s) {
        *s=toupper(*s);
        s++;
    }
}

extern "C" void strlwr(char* s)
{
    while(*s) {
        *s=tolower(*s);
        s++;
    }
}

extern "C" int snprintf(char* s, size_t buflen, const char* fmt, ...)
{
    va_list argptr;
    int     len;

    va_start(argptr,fmt);
    len=vsprintf(s,fmt,argptr);
    if(len>buflen) // this obviously could be handled better
        strcpy(s,"(buffer overflow)");
    va_end(argptr);
    return(len);
}

#if 0
// defined in SetTop API and assumed to never run in the absence of that
extern "C" int strcasecmp(const char* s1, const char* s2)
{
    char cmp1[512];
    char cmp2[512];

    sprintf(cmp1,"%.*s",sizeof(cmp1)-1,s1);
    sprintf(cmp2,"%.*s",sizeof(cmp2)-1,s2);

    strlwr(cmp1);
    strlwr(cmp2);

    return(strcmp(cmp1,cmp2));
}

// defined in SetTop API and assumed to never run in the absence of that
extern "C" int strncasecmp(const char* s1, const char* s2, size_t len)
{
    char cmp1[512];
    char cmp2[512];

    sprintf(cmp1,"%.*s",sizeof(cmp1)-1,s1);
    sprintf(cmp2,"%.*s",sizeof(cmp2)-1,s2);

    strlwr(cmp1);
    strlwr(cmp2);

    return(strncmp(cmp1,cmp2,len));
}
#endif

#endif
