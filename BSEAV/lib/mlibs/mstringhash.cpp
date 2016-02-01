/******************************************************************************
 * (c) 2001-2014 Broadcom Corporation
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

#include "mstringhash.h"
#include "mstringlist.h"

static MString stub;

MStringHash::MStringHash(bool caseSensitive) {
    _caseSensitive = caseSensitive;
}

MStringHash::MStringHash(const MStringHash &hash) {
    _caseSensitive = hash._caseSensitive;

    /* copy the hash list */
    MListItr<Node> itr(&hash._list);
    for (;itr.current();itr.next()) {
        add(itr.current()->name, itr.current()->data);
    }
}

MStringHash::Node *MStringHash::find(const char *name, int len) const {
    MListItr<Node> itr(&_list);
    for (Node *n = itr.first(); n; n = itr.next()) {
        if (_caseSensitive && !n->name.strncmp(name,len) ||
            !_caseSensitive && !n->name.strncasecmp(name, len))
            return n;
    }
    return NULL;
}

void MStringHash::add(const char *name, int namelen, const char *value, int len) {
    Node *node = find(name, namelen);
    if (node)
        node->data.strncpy(value,len);
    else
        _list.add(new Node(name, namelen, value, len));
}

void MStringHash::add(const char *name, const char *value, int len) {
    add(name, -1, value, len);
}

MString &MStringHash::get(const char *name) const {
    Node *node = find(name);
    if (node)
        return node->data;
    else
        return stub;
}

#ifdef I_PREFER_SLOW_CODE
MString &MStringHash::name(int index) {
    Node *n = _list[index];
    if (n)
        return n->name;
    else
        return stub;
}

MString &MStringHash::value(int index) {
    Node *n = _list[index];
    if (n)
        return n->data;
    else
        return stub;
}
#endif

MString &MStringHash::name() const {
    Node *n = _list.current();
    if (n)
        return n->name;
    else
        return stub;
}

MString &MStringHash::value() const {
    Node *n = _list.current();
    if (n)
        return n->data;
    else
        return stub;
}

bool MStringHash::remove(const char *name) {
    if (find(name)) {
        _list.remove();
        return true;
    }
    else
        return false;
}

#include <ctype.h> /*for isspace*/

bool MStringHash::read(const char *filename, bool recursive) {
    MStringList list;
    if (!readFile(filename, list))
        return false;
    int line=1;
    for (const char *s = list.first(); s; s = list.next(), line++) {
        bool added = false;
        // '#' begins line comments (ignore everything after the #)
        char *x = (char *)strchr(s, '#');
        if (x) *x = 0;

        // remove leading and trailing whitespace from the input
        while (*s && isspace(*s)) s++;
        if (*s) { x = (char*)&s[strlen(s)-1]; while (x != s && isspace(*x)) x--; if (*x) x[1] = 0; }
        // is there anything left after striping the whitespace?  Ignore blank lines
        if (!*s) continue;

        if (recursive) {
            if (strncmp(s, "include ", 8)==0 && strlen(s)>8) {
                char temp[64], *a;
                strncpy(temp, s, 63);
                a = &temp[8];
                /* remove leading whitespace */
                while (*a && isspace(*a)) a++;
                /* remove surrounding " " or < > */
                if ((a[0] == '"' && a[strlen(a)-1] == '"') || (a[0] == '<' && a[strlen(a)-1] == '>')) {
                    a++;
                    a[strlen(a)-1] = 0;
                }
                read(a, false); /* allow only one level of recursion to prevent circular includes */
                added = true;
            }
        }

        // split the name/value pair and clean them both up
        x = (char *)strchr(s, '='); /*marks the spot*/
        if (x && x != s) {
            char *b = x, *a = x;
            // remove trailing whitespace from key and leading whitespace from data
            // note: the '=' tests below are important to skip over the '=' (starting point)
            while (*a && (*a == '=' || isspace(*a))) a++;
            while (b != s && (*b == '=' || isspace(*b))) b--;
            *(b+1) = 0;
            // If the data is quoted (either kind), remove the quotes
            if ((a[0] == '"'  && a[strlen(a)-1] == '"') ||
                (a[0] == '\'' && a[strlen(a)-1] == '\'')) { a++; a[strlen(a)-1] = 0; }
            // If there is anything left (after striping off whitespace and quotes) add it.
            if (*s && *a) {
                //printf("Adding s:'%s'  a:'%s'\n", s,a);
                add(s, a);
                added = true;
            }
        }
        if (!added) printf("Configuration file '%s' line %d is malformed (format is: A=B or A=\"B\")\n", filename, line);
    }
    return true;
}

void MStringHash::print() const {
    MListItr<Node> itr(&_list);
    for (;itr.current();itr.next()) {
        printf("%s=%s\n", (const char *)itr.current()->name, (const char *)itr.current()->data);
    }
}
