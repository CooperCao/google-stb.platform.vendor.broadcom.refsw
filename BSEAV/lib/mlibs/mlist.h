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

#ifndef MLIST_H
#define MLIST_H

#include <stdlib.h> //define NULL
#include <stdio.h>
#include "mvoidlist.h"

template<class T>
class MListItr : MVoidListItr {
public:
    MListItr(const MVoidList *list) : MVoidListItr(list) {}

    T *first() {return (T*)MVoidListItr::first();}
    T *next() {return (T*)MVoidListItr::next();}
    T *prev() {return (T*)MVoidListItr::prev();}
    T *last() {return (T*)MVoidListItr::last();}
    T *at(int i) {return (T*)MVoidListItr::at(i);}
    int index(T *data) {return (int)MVoidListItr::index(data);}
    T *current() const {return (T*)MVoidListItr::current();}
};

template<class T>
class MList : public MVoidList {
public:
    /* CAUTION: see comment for MAutoList concerning autodelete=true */
    MList(bool autodelete = false) : MVoidList(autodelete) {}
    virtual ~MList() { clear(); }

    void add(T *data) {MVoidList::add(data);}
    void insert(int index, T*data) {MVoidList::insert(index, data);}
    T *remove(T *data) {return (T*)MVoidList::remove(data);}
    T *remove(int i) {return (T*)MVoidList::remove(i);}
    T *remove() {return (T*)MVoidList::remove();}
    int index(T *data) {return MVoidList::index(data);}
    T *get(int i) const {return (T*)MVoidList::get(i);}

    int total() const {return _total;}

    T *first() {return (T*)MVoidList::first();}
    T *next() {return (T*)MVoidList::next();}
    T *prev() {return (T*)MVoidList::prev();}
    T *last() {return (T*)MVoidList::last();}
    T *at(int i) {return (T*)MVoidList::at(i);}
    T *current() const {return (T*)MVoidList::current();}

    T *operator [](int i) const {return get(i);}

    void sort(int (*compar)(T *d1, T *d2)) {
        MVoidList::sort((int (*)(void *,void *))compar);
    }

protected:
    void deleteData(void *data)
    {
        if (data)
        {
            delete (T*)data;
            data = NULL;
        }
    }
};

/**
* MAutoList is a list which defaults to autodelete.
*/

/* !!!!!!!!!! C A U T I O N !!!!!!!!!! */
/*
   ONLY DYNAMICALLY ALLOCATED DATA MAY BE ADDED TO MAUTOLIST. THAT DATA WILL
   THEN BELONG TO THE MAUTOLIST OBJECT, AND WILL BE AUTOMATICALLY DELETED WHEN
   THE MAUTOLIST IS DELETED.  DO NOT DELETE THE DYNAMICALLY ALLOCATED DATA
   ELSEWHERE OR ***MEMORY CORRUPTION*** WILL RESULT!

   CARE MUST ALSO BE TAKEN NOT TO ADD THE SAME DYNAMICALLY ALLOCATED DATA TO
   MULTIPLE MAUTOLISTS (OR ANY SUBCLASSES SUCH AS MHASH).
*/
template <class T>
class MAutoList : public MList<T> {
public:
    MAutoList() : MList<T>(true) {}
};

//////////////////////////////////////////////////


#endif
