/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_TRACKERS_H__
#define __BSG_TRACKERS_H__

#include "bsg_common.h"

#include <stdlib.h>
#include <stdio.h>

// @cond

namespace bsg
{

//! Frees up a malloced pointer when it is deleted
class Free
{
public:
   Free(void *ptr) : m_ptr(ptr) {}
   ~Free()                      { free(m_ptr); }

private:
   // NO COPY
   Free(const Free &rhs);
   Free &operator=(const Free &rhs);

   void  *m_ptr;
};

//! Closes a stdio file when it is deleted
class FClose
{
public:
   FClose(FILE *fp) : m_fp(fp) {}
   ~FClose()                   { fclose(m_fp); }

private:
   // NO COPY
   FClose(const FClose &rhs);
   FClose &operator=(const FClose &rhs);

   FILE  *m_fp;
};

//! Deletes on destruction a newed pointer -- very restrictive version of auto_ptr
template <class T>
class Auto
{
public:
   Auto() : m_ptr(0)           {}
   Auto(T *ptr) : m_ptr(ptr)   {}
   ~Auto()                     { delete m_ptr; }

   void operator=(T *ptr)      { if (m_ptr) delete m_ptr; m_ptr = ptr; }

   T *operator->()             { return m_ptr; }
   const T *operator->() const { return m_ptr; }

   //T *operator*()              { return m_ptr; }
   //const T *operator*() const  { return m_ptr; }

   operator bool() const       { return m_ptr != 0; }

private:
   // NO COPY
   Auto(const Auto &rhs);
   Auto &operator=(const Auto &rhs);

   // NO COMPARE use boolean cast to test for null
   bool operator==(const Auto &rhs);

   T  *m_ptr;
};

}

// @endcond

#endif
