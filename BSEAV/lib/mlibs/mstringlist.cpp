/******************************************************************************
 * (c) 2004-2014 Broadcom Corporation
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
 *
 *****************************************************************************/

#include "mstringlist.h"
#include "mstring.h"
#include <stdio.h>

struct MStringList::Node {
    Node *prev;
    Node *next;
    MString data;
    Node(const char *d,int len) {data.strncpy(d,len);prev=next=NULL;}
};

void MStringList::add(const char *data, int len) {
    Node *node = new Node(data, len);
    if (_last) {
        _last->next = node;
        node->prev = _last;
        _last = node;
    }
    else {
        _first = _last = node;
    }
    _current = node;
    _total++;
}

void MStringList::insert(int index, const char *data, int len) {
    at(index);
    if (_current) {
        Node *node = new Node(data, len);
        if (_current->prev) {
            _current->prev->next = node;
            node->prev = _current->prev;
        }
        else {
            _first = node;
        }
        _current->prev = node;
        node->next = _current;
        _current = node;
        _total++;
    }
    else {
        add(data);
    }
}

void MStringList::change(int index, const char *data, int len) {
    at(index);
    if (_current) {
        _current->data.strncpy(data, len);
    }
}

bool MStringList::remove(const char *data) {
    if (index(data) != -1)
        return remove();
    else
        return false;
}

bool MStringList::remove(int i) {
    if (at(i))
        return remove();
    else
        return false;
}

bool MStringList::remove() {
    if (_current) {
        Node *temp = _current->next;
        if (!temp)
            temp = _current->prev;

        if (_current->prev) {
            _current->prev->next = _current->next;
        }
        else {
            _first = _current->next;
        }
        if (_current->next) {
            _current->next->prev = _current->prev;
        }
        else {
            _last = _current->prev;
        }
        delete _current;
        _total--;

        _current = temp;
        return true;
    }
    return false;
}

int MStringList::index(const char *data) {
    int i = 0;
    _current = _first;
    while (_current) {
        if (_current->data == data) {
            return i;
        }
        _current = _current->next;
        i++;
    }
    return -1;
}

MString &MStringList::at(int i) {
    _current = _first;
    while (i-- && _current)
        _current = _current->next;
    return current();
}

void MStringList::clear() {
    first();
    while (_current) {
        remove();
    }
}

MString &MStringList::first() {
    _current = _first;
    return current();
}

MString &MStringList::next() {
    if (_current)
        _current = _current->next;
    return current();
}

MString &MStringList::prev() {
    if (_current)
        _current = _current->prev;
    return current();
}

MString &MStringList::last() {
    _current = _last;
    return current();
}

MString &MStringList::current() {
    static MString stub;
    return _current?_current->data:stub;
}

const char *MStringList::current() const {
    return _current?(const char *)_current->data:NULL;
}

int MStringList::split(const char *str, const char *delim, MStringList &list) {
    const char *next;
    int count = 0;
    while (str && *str) {
        next = strpbrk(str, delim);
        count++;
        if (next)
            list.add(str, next-str);
        else {
            list.add(str);
            break;
        }
        str = strspn(next, delim) + next;
    }
    return count;
}

int MStringList::split(const char *str, char delim, MStringList &list) {
    const char *next;
    int count = 0;
    if (str && *str)
    while (1) {
        next = strchr(str, delim);
        count++;
        if (next)
            list.add(str, next-str);
        else {
            list.add(str);
            break;
        }
        str = next+1;
    }
    return count;
}

MString MStringList::join(char delim) {
    MString result;
    const char *s = first();
    while (s) {
        result += s;
        s = next();
        if (s)
            result += delim;
    }
    return result;
}

bool readFile(const char *filename, MStringList &list) {
    char buf[256];

    FILE *f = fopen(filename, "r");
    if (!f)
        return false;

    while (!feof(f)) {
        if (!fgets(buf, 256, f))
            break;
        int len = strlen(buf);
        if (buf[len-1] == '\n')
            buf[--len] = 0;
        if (buf[len-1] == '\r')
            buf[--len] = 0;
        list.add(buf);
    }
    fclose(f);
    return true;
}

bool writeFile(const char *filename, MStringList &list) {
    FILE *f = fopen(filename, "w+");
    if (!f)
        return false;

    for (const char *s = list.first(); list.current(); s = list.next()) {
        fputs(s, f);
        fputc('\n', f);
    }
    fclose(f);
    return true;
}

void MStringList::add(MStringList &list)
{
    for (const char *s = list.first(); s; s = list.next())
        add(s);
}
