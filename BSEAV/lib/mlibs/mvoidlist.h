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

#ifndef MVOIDLIST_H
#define MVOIDLIST_H

#include <stdlib.h>

class MVoidList;

struct MVoidListNode {
    MVoidListNode *prev;
    MVoidListNode *next;
    void *data;
    MVoidListNode(void *d) {data=d;prev=next=NULL;}
};

/**
Summary:
Provides an iterator to search an MVoidList without modifying the state of the MVoidList.

Description:
If you remove an element that is currently pointed to my MVoidList, you are in an
undefined state. You must call first(), last() or at() to reset state.
**/
class MVoidListItr {
friend class MVoidList;
public:
    MVoidListItr(const MVoidList *list);

    void *first();
    void *next();
    void *prev();
    void *last();
    void *at(int i);
    int index(void *data);
    void *current() const {return _current?_current->data:NULL;}

protected:
    const MVoidList *_list;
    const MVoidListNode *_current;
};

/**
* List of void*. Note that autodelete doesn't do anything
* because it's type-agnostic. But it's there because of
* MList<T>.
*/
class MVoidList {
friend class MVoidListItr;
public:
    MVoidList(bool autodelete = false);
    virtual ~MVoidList() {clear();}

    void setAutoDelete(bool ad) {_autoDelete = ad;}
    void add(void *data);
    void insert(int index, void *data);
    void *remove(void *data);
    void *remove(int i);
    void *remove();
    void clear();

    /**
    Get a node at a certain index without changing the current index.
    **/
    void *get(int i) const;
    int total() const {return _total;}

    /**
    The following functions use an internal MVoidListItr and
    will relocated it.
    **/
    void *first() {return _itr.first();}
    void *next() {return _itr.next();}
    void *prev() {return _itr.prev();}
    void *last() {return _itr.last();}
    void *at(int i) {return _itr.at(i);}
    void *current() const {return _itr.current();}
    int index(void *data) {return _itr.index(data);}

    /**
    * Sort in ascending order (currently using bubble sort).
    * If compar returns -1, d1 < d2.
    * If compar returns 1, d1 > d2.
    * If compar returns 0, d1 == d2.
    **/
    void sort(int (*compar)(void *d1, void *d2));

protected:
    MVoidListNode *_first;
    MVoidListNode *_last;
    MVoidListItr _itr;
    bool _autoDelete;
    int _total;

    virtual void deleteData(void *n) {n=n;}  /* "n=n" to avoid compiler warning */
};

#endif //MVOIDLIST_H
