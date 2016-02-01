/***************************************************************************
 *     (c)2015 Broadcom Corporation
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
#ifndef MHASH_H
#define MHASH_H

#include "mstring.h"
#include "mlist.h"

/* !!!!!!!!!! C A U T I O N !!!!!!!!!! */
/*
   ONLY DYNAMICALLY ALLOCATED DATA MAY BE ADDED TO MHASH USING THE DEFAULT
   CONSTUCTOR ARGUMENT.  THAT DATA WILL THEN BELONG TO THE MHASH
   OBJECT, AND WILL BE AUTOMATICALLY DELETED WHEN THE MHASH IS DELETED.  DO NOT
   DELETE THE DYNAMICALLY ALLOCATED DATA ELSEWHERE OR ***MEMORY CORRUPTION*** WILL
   RESULT!

   CARE MUST ALSO BE TAKEN NOT TO ADD THE SAME DYNAMICALLY ALLOCATED DATA TO
   MULTIPLE MHASHS OR MAUTOLISTS (OR THEIR SUBCLASSES).

   IF YOU NEED MHASH TO STORE STATICALLY ALLOCATED DATA OR YOU WISH TO
   FREE DYNAMICALLY ALLOCATED DATA FROM OUTSIDE THE MHASH
   OBJECT, YOU MUST CALL THE MHASH CONSTRUCTOR WITH THE ARGUMENT, AUTODELETE
   SET TO FALSE!  OR, YOU COULD USE THE REMOVE METHOD TO RETURN RESPONSIBILITY
   FOR THE DYNAMICALLY ALLOCATED DATA TO THE CALLING CODE.  CALLING THE REMOVE()
   METHOD WILL REMOVE THEY DATA FROM THE MHASH, AND WILL RELINQUISH RESPONSIBILITY
   FOR THE MEMORY MANAGEMENT AS WELL.  IN THIS CASE, CALLING CODE WILL NEED TO
   FREE THE DATA WHEN IT IS NO LONGER NEEDED.  THE REMOVE METHOD OPERATES IN
   THIS MANNER REGARDLESS OF THE STATE OF THE AUTODELETE FLAG.
*/
template <class T>
class MHash {
public:
    MHash(bool autodelete = true) { _autodelete = autodelete; };
    virtual ~MHash() { clear(); };

    /**
    * Add value to the hash. If the name is already used, the value
    * is replaced.
    */
    void add(const char *name, T *value);
    T *get(const char *name);
    const char *getName(int index);
    T *getData(int index);
    T *remove(const char *name);
    void clear();
    void print();

    int total() {return _list.total();}

protected:
    struct Node {
        MString name;
        T *data;
        bool _autodelete;
        Node(const char *n, T *d, bool ad) {name=n;data=d;_autodelete=ad;}
        ~Node() {if (_autodelete) delete data;}
    };
    MList<Node> _list;
    bool _autodelete;
};

template <class T>
inline void MHash<T>::add(const char *name, T *value) {
    if (get(name)) {
        Node *n = (Node*)_list.current();
        if (n->_autodelete && n->data)
            delete n->data;
        n->data = value;
    }
    else
    {
        _list.add(new Node(name, value, _autodelete));
    }
}

template <class T>
inline T *MHash<T>::get(const char *name) {
    for (Node *n = (Node*)_list.first(); n; n = (Node*)_list.next())
    {
        if (n->name == name)
            return n->data;
    }

    return NULL;
}

template <class T>
inline const char *MHash<T>::getName(int index) {
    Node * n = _list.at(index);

    if (n)
    {
        return n->name.s();
    }

    return NULL;
}

template <class T>
inline T *MHash<T>::getData(int index) {
    Node * n = _list.at(index);

    if (n)
    {
        return n->data;
    }

    return NULL;
}

template <class T>
inline T *MHash<T>::remove(const char *name) {
    T *data = get(name);
    if (data) {
        Node *n = (Node*)_list.remove();
        n->data = NULL;
        delete n;
    }
    return data;
}

template <class T>
inline void MHash<T>::clear() {
    while (Node * n = (Node *)_list.first())
    {
        T * data = remove(n->name.s());
        if (_autodelete && data)
        {
            delete data;
        }
    }
}

template <class T>
inline void MHash<T>::print() {
    for (Node *n = (Node*)_list.first(); n; n = (Node*)_list.next())
        printf("%s\n", n->name.s());
}

#endif
