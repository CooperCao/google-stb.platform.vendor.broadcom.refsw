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

#include "mvoidlist.h"

MVoidList::MVoidList(bool autodelete) : _itr(this) {
    setAutoDelete(autodelete);
    _first = _last = NULL;
    _itr.first(); // must reset after _first is initialized
    _total = 0;
}

void MVoidList::add(void *data) {
    MVoidListNode *node = new MVoidListNode(data);
    if (_last) {
        _last->next = node;
        node->prev = _last;
        _last = node;
    }
    else {
        _first = _last = node;
    }
    _itr._current = node;
    _total++;
}

void MVoidList::insert(int index, void *data) {
    at(index);
    MVoidListNode *current = (MVoidListNode *)_itr._current;
    if (current) {
        MVoidListNode *node = new MVoidListNode(data);
        if (current->prev) {
            current->prev->next = node;
            node->prev = current->prev;
        }
        else {
            _first = node;
        }
        current->prev = node;
        node->next = current;
        _itr._current = node;
        _total++;
    }
    else {
        add(data);
    }
}

void *MVoidList::remove(void *data) {
    if (index(data) != -1)
        return remove();
    else
        return NULL;
}

void *MVoidList::remove(int i) {
    if (at(i))
        return remove();
    else
        return NULL;
}

void *MVoidList::remove() {
    MVoidListNode *current = (MVoidListNode *)_itr._current;
    if (current) {
        MVoidListNode *temp = current->next;
//      if (!temp)
//          temp = _current->prev;

        if (current->prev) {
            current->prev->next = current->next;
        }
        else {
            _first = current->next;
        }
        if (current->next) {
            current->next->prev = current->prev;
        }
        else {
            _last = current->prev;
        }
        //NOTE: I don't delete the data here
        void *data = current->data;
        delete current;
        _total--;

        _itr._current = temp;
        return data;
    }
    return NULL;
}

void MVoidList::clear() {
    while (_first) {
        if (_autoDelete && _first->data)
            deleteData(_first->data);
        remove(0);
    }
    _itr._current = NULL;
}

void MVoidList::sort(int (*compar)(void *d1, void *d2)) {
    MVoidListNode *last = NULL;
    while (last != _first) {
        MVoidListNode *d;
        for (d = _first; d && d->next != last; d = d->next)
            if ((*compar)(d->data, d->next->data) > 0) {
                void *temp = d->data;
                d->data = d->next->data;
                d->next->data = temp;
            }
        last = d;
    }
}

void *MVoidList::get(int i) const
{
    if (i < 0) {
        return NULL;
    }
    else {
        MVoidListNode *n = _first;
        while (i-- && n)
            n = n->next;
        return n?n->data:NULL;
    }
}

////////////////////////////////////////////////

MVoidListItr::MVoidListItr(const MVoidList *list)
{
    _list = list;
    _current = _list->_first;
}

void *MVoidListItr::first()
{
    _current = _list->_first;
    return current();
}

void *MVoidListItr::next()
{
    if (_current)
        _current = _current->next;
    return current();
}

void *MVoidListItr::prev()
{
    if (_current)
        _current = _current->prev;
    return current();
}

void *MVoidListItr::last()
{
    _current = _list->_last;
    return current();
}

void *MVoidListItr::at(int i)
{
    if (i < 0) {
        _current = NULL;
    }
    else {
        _current = _list->_first;
        while (i-- && _current)
            _current = _current->next;
    }
    return current();
}

int MVoidListItr::index(void *data) {
    int i = 0;
    first();
    while (_current) {
        if (_current->data == data) {
            return i;
        }
        _current = _current->next;
        i++;
    }
    return -1;
}
