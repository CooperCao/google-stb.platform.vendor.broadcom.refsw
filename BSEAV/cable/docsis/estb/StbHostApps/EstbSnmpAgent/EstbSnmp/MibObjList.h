/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
//
//
//**************************************************************************
//    Filename: MibObjList.h
//    Author:   Kevin O'Neal
//    Creation Date: 18-Sept-2001
//
//**************************************************************************
//    Description:
//
//      Cablemodem V2 code
//      
//    This object is simply a wrapper for an STL list with the same
//    programatic interface.  The sole purpose of this is to avoid STL
//    inline expansions of things like push_front, pop_back, erase, etc.
//    and thereby save considerable code space.
//
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************

#ifndef MIBOBJLIST_H
#define MIBOBJLIST_H

#if ((__GNUC__ >= 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <list>
#include <memory>
#else
#include <list.h>
#include <alloc.h>
#endif


// Forward reference:
class BcmMibObject;

class BcmMibObjectList
{
  public: 
    BcmMibObjectList ();
   ~BcmMibObjectList ();
   
    typedef list <BcmMibObject *>::iterator iterator;
    typedef list <BcmMibObject *>::const_iterator const_iterator;
    typedef long int size_type;
    typedef BcmMibObject * BcmMibObjectPtr;
   
    const_iterator begin() const;
    iterator begin();
    void clear();
    const_iterator end() const;
    iterator end();
    bool empty() const;
    iterator erase (iterator it);
    iterator erase (iterator first, iterator last);
    iterator insert (iterator it, const BcmMibObjectPtr &x);
    void insert (iterator it, size_type n, const BcmMibObjectPtr &x);
    void insert (iterator it, const_iterator first, const_iterator last);
    void push_back (const BcmMibObjectPtr &x);
    size_type size() const;
  
  private:
#if defined(__linux__)
    // How do I get malloc_alloc for linux?  For now, just use the standard
    // allocator...
    list <BcmMibObject *> fList;
#else
    #if defined(TARGETOS_eCos)
        // The new Gnu STL doesn't define malloc_alloc any more, so we have
        // to define it ourselves.  This will most likely need to change if
        // we start using a different STL library.
        typedef __malloc_alloc_template<0>  malloc_alloc;
    #endif
    // If we replace the standard STL list allocator with the 'malloc_alloc'
    // allocator, we can reduce heap fragmentation when a large list (like a
    // table with lots of entries) is populated and then depopulated.  The 
    // reason is that the standard allocator never gets rid of allocated
    // 'nodes' once they are created, so even though all the table entries
    // go away, still the list nodes remain.  This fragments memory like
    // crazy!  However, the tradeoff is that the malloc_alloc allocator is
    // (slightly) more heap-hungry in general and also slower.  Since the heap
    // penalty is fairly small, and speed in this case is not of particular
    // interest, we will use the malloc_alloc.
    list <BcmMibObject *, malloc_alloc> fList;
#endif
};

#endif
