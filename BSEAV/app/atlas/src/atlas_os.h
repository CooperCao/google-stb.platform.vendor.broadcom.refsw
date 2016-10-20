/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef ATLASOS_H__
#define ATLASOS_H__

#include "b_os_lib.h"
#include "mstring.h"
#include "mlist.h"

#ifdef __cplusplus
extern "C" {
#endif

extern B_ThreadHandle    gSchedulerThread;
extern B_MutexHandle     gLock;
extern B_SchedulerHandle gScheduler;

/*
 *  This mutex class is based on the b_os mutex type but implements it such
 *  that the mutex is locked when this object is constructed, and unlocked
 *  when this object is freed.  It is a convenience class used to lock/unlock
 *  a given mutex based on the scope of a local variable - 'break' or 'return'
 *  will still trigger the mutex unlock based on scope rules.
 *
 *  For example:
 *  void test(int * pInt)
 *  {
 *      CScopedMutex(_myMutex);
 *
 *      if (NULL == pInt)
 *          return;
 *
 * pInt = 1;
 *  }
 *
 *  _myMutex will be locked at the start of test() and unlocked automatically
 *  when the function returns (regardless of whether pInt points to a valid
 *  int or is NULL).
 */
class CScopedMutex
{
public:
    CScopedMutex(B_MutexHandle mutex);
    ~CScopedMutex(void);
protected:
    B_MutexHandle _mutex;
};

/*
 * This class replaces getifaddrs api which is not avialble for legacy toolchains
 * A couple of macros for printing IP addresses that are in network byte order:
 */
#define INET_ADDR_PRINTF_ARG(f)  *((char *)&(f)), *(((char *)&(f))+1), *(((char *)&(f))+2), *(((char *)&(f))+3)
#define INET_ADDR_PRINTF_FMT  "%u.%u.%u.%u"

class if_interface
{
public:
    if_interface();
    if_interface(
            const char *  name,
            unsigned long s_addr
            );
    ~if_interface(void);
    MString       if_name;
    unsigned long s_addr;
};
/* returns list of interfcases with non zero ip address, loopback interface is ignored as well  */
MList <if_interface> * get_ifaddrs(void);
void                   free_ifaddrs(MList <if_interface> * pIfInterfaceList);

#ifdef __cplusplus
}
#endif

#endif /* ATLASOS_H__ */