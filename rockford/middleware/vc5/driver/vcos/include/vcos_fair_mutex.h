/*
 * Copyright (c) 2010-2011 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

/*=============================================================================
VideoCore OS Abstraction Layer - fair mutex public header file
=============================================================================*/

#ifndef VCOS_FAIR_MUTEX_H
#define VCOS_FAIR_MUTEX_H

#include "vcos_types.h"
#include "vcos_platform.h"

#if VCOS_HAVE_FAIR_MUTEX

/**
 * \file vcos_fair_mutex.h
 *
 * Fair mutex API; regular vcos mutexes (vcos_mutex.h) may not be fair. As with
 * regular vcos mutexes, fair mutexes are *not* reentrant.
 */

VCOS_EXTERN_C_BEGIN

static inline VCOS_STATUS_T vcos_fair_mutex_create(VCOS_FAIR_MUTEX_T *m, const char *name);
static inline void vcos_fair_mutex_delete(VCOS_FAIR_MUTEX_T *m);
static inline void vcos_fair_mutex_lock(VCOS_FAIR_MUTEX_T *m);
static inline void vcos_fair_mutex_unlock(VCOS_FAIR_MUTEX_T *m);

VCOS_EXTERN_C_END

#ifdef __cplusplus

namespace vcos
{

class fair_mutex
{
   VCOS_FAIR_MUTEX_T m_mutex;

   /* Non-copyable */
   fair_mutex(const fair_mutex &);
   fair_mutex &operator=(const fair_mutex &);

public:

   fair_mutex(const char *name) { throw_if_error(vcos_fair_mutex_create(&m_mutex, name)); }
   fair_mutex() : fair_mutex("vcos::fair_mutex") {}
   ~fair_mutex() { vcos_fair_mutex_delete(&m_mutex); }

   void lock() { vcos_fair_mutex_lock(&m_mutex); }
   void unlock() { vcos_fair_mutex_unlock(&m_mutex); }
};

}

#endif

#endif

#endif
