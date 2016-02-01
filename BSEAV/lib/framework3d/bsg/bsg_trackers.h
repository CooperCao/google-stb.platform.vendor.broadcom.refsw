/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.  
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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

