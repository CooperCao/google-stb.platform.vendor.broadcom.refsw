//**************************************************************************
//
//    Copyright 2000 Broadcom Corporation
//    All Rights Reserved
//    No portions of this material may be reproduced in any form without the
//    written permission of:
//             Broadcom Corporation
//             16251 Laguna Canyon Road
//             Irvine, California  92618
//    All information contained in this document is Broadcom Corporation
//    company private, proprietary, and trade secret.
//
//
//
//**************************************************************************
//    Filename: MibObjList.cpp
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
//    and thereby save some code space.
//      
//**************************************************************************
//    Revision History:
//
//**************************************************************************


#include "MibObjs.h"
#include "MibObjList.h"


BcmMibObjectList::BcmMibObjectList ()
{
}

BcmMibObjectList::~BcmMibObjectList ()
{
}

BcmMibObjectList::const_iterator BcmMibObjectList::begin() const
{
  return fList.begin();
}

BcmMibObjectList::iterator BcmMibObjectList::begin()
{
  return fList.begin();
}

void BcmMibObjectList::clear()
{
  fList.clear();
}

BcmMibObjectList::const_iterator BcmMibObjectList::end() const
{
  return fList.end();
}

BcmMibObjectList::iterator BcmMibObjectList::end()
{
  return fList.end();
}

bool BcmMibObjectList::empty() const
{
  return fList.empty();
}

BcmMibObjectList::iterator BcmMibObjectList::erase (iterator it)
{
  return fList.erase (it);
}

BcmMibObjectList::iterator BcmMibObjectList::erase (iterator first, iterator last)
{
  return fList.erase (first, last);
}

BcmMibObjectList::iterator BcmMibObjectList::insert (iterator it, const BcmMibObjectPtr &x)
{
  return fList.insert (it, x);
}

void BcmMibObjectList::insert (iterator it, size_type n, const BcmMibObjectPtr &x)
{
  fList.insert (it, n, x);
}

void BcmMibObjectList::insert (iterator it, const_iterator first, const_iterator last)
{
  fList.insert (it, first, last);
}

void BcmMibObjectList::push_back (const BcmMibObjectPtr &x)
{
  fList.push_back (x);
}

BcmMibObjectList::size_type BcmMibObjectList::size() const
{
  return fList.size();
}
