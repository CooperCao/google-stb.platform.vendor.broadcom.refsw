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

#ifndef MSTRINGHASH_H
#define MSTRINGHASH_H

#include "mstring.h"
#include "mlist.h"

class MStringHash {
public:
    /**
    Summary:
    Create a new hash
    **/
    MStringHash(bool caseSensitive = true);

    /**
    Summary:
    Create a new hash and copy the contents of another
    **/
    MStringHash(const MStringHash &hash);

    /**
    Summary:
    Add a value to the hash.
    Description:
    If the name is already used, the value is replaced.
    */
    void add(const char *name, const char *value, int valuelen = -1);

    /**
    Summary:
    Add a value to the hash, but used both namelen and optional valuelen.
    Description:
    This allows you to add a name/value pair by clipping a portion of a string, not
    taking the entire string.
    **/
    void add(const char *name, int namelen, const char *value, int valuelen = -1);

    /**
    Summary:
    Remove a value from the hash.
    **/
    bool remove(const char *name);

    /**
    Summary:
    Remove all values from the hash.
    **/
    void clear() {_list.clear();}


    /**
    Summary:
    Find a value in the hash.
    **/
    MString &get(const char *name) const;

    /**
    Summary:
    Position the hash to its first element.

    Example code:
        for (hash.first();hash.current();hash.next()) {
            printf("%s=%s\n", hash.name().s(), hash.value().s());
        }
    */
    bool first() {return _list.first() ? true:false;}

    /**
    Summary:
    Go to the next element in the hash.
    **/
    bool next() {return _list.next() ? true:false;}

    /**
    Summary:
    Check if the hash is currently on an element.
    **/
    bool current() const {return _list.current() ? true:false;}

    /**
    Summary:
    Return the name of the current element.
    **/
    MString &name() const;

    /**
    Summary:
    Return the value of the current element.
    **/
    MString &value() const;

#ifdef I_PREFER_SLOW_CODE
    MString &name(int index);
    MString &value(int index);
    int total() const {return _list.total();}
#endif

    /**
    Summary:
    Read name=value from a file. Doesn't clear the hash first.
    Returns true on success.
    */
    bool read(const char *filename, bool recursive = false);


    /**
    Summary:
    Print hash contents to stdout for debug
    */
    void print() const;

protected:
    struct Node {
        MString name,data;
        Node(const char *n, int nlen, const char *d, int len = -1) {
            name.strncpy(n,nlen);
            data.strncpy(d,len);
        }
    };
    Node *find(const char *name, int len = -1) const;
    MAutoList<Node> _list;
    bool _caseSensitive;
};

#endif // MSTRINGHASH_H
