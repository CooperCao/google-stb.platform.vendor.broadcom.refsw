/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#ifndef ATLAS_ACTION_H__
#define ATLAS_ACTION_H__

#include "notification.h"

/* Actions are created in response to asynchronous commands (i.e. from a thread).
 * They are stored temporarily until the Atlas main loop has a chance to service them.
 * Data associated with the action is allocated when the action is created, and freed
 * after the associated action has completed.  CAction base class allows us to reference
 * all non-data events using the base class, while the template derived class
 * ensures that different _data types are properly deleted when no longer needed.
 * CLua uses actions to assist in synchronizing Lua calls originating from the Lua thread
 * with the bwin main loop.  CWifi also uses it to synchronize threaded calls
 * to the WPA Supplicant library with the bwin main loop.
 */
class CAction
{
public:
    CAction() :
        _id(eNotify_Invalid),
        _waitTimeout(1000),
        _bTimedOut(false),
        _numReturnVals(1) {}

    CAction(
            eNotification id,
            eNotification waitId = eNotify_Invalid,
            int           waitTimeout = 1000 /*B_WAIT_FOREVER*/
            ) :
        _id(id),
        _waitTimeout(waitTimeout),
        _bTimedOut(false),
        _numReturnVals(1)
    {
        addWaitNotification(waitId);
    }

    virtual ~CAction() {}

    virtual void * getDataPtr(void) { return(NULL); }

    eNotification getId(void)                         { return(_id); }
    int           getWaitTimeout(void)                { return(_waitTimeout); }
    void          setTimedOut(bool bTimedOut)         { _bTimedOut = bTimedOut; }
    bool          isTimedOut(void)                    { return(_bTimedOut); }
    int           getNumReturnVals(void)              { return(_numReturnVals); }
    void          setNumReturnVals(int numReturnVals) { _numReturnVals = numReturnVals; }
    eNotification getWaitId(int index = 0)
    {
        eNotification * pNotification = _waitIdList[index];

        return((NULL != pNotification) ? *pNotification : eNotify_Invalid);
    }

    void addWaitNotification(eNotification notification)
    {
        eNotification * pNotify = new eNotification;

        BDBG_ASSERT(NULL != pNotify);

        *pNotify = notification;
        _waitIdList.add(pNotify);
    }

    eNotification getWaitNotification(int index = 0)
    {
        eNotification * pNotify = NULL;

        if (_waitIdList.total() > index)
        {
            pNotify = _waitIdList[index];
        }

        return((pNotify != NULL) ? *pNotify : eNotify_Invalid);
    }

    void clear(void)
    {
        _id = eNotify_Invalid;
        _waitIdList.clear();
        _waitTimeout   = 1000;
        _bTimedOut     = false;
        _numReturnVals = 1;
    }

    void operator =(CAction &other)
    {
        eNotification otherNotification = eNotify_Invalid;

        if (this == &other)
        {
            return;
        }

        _id            = other._id;
        _waitTimeout   = other._waitTimeout;
        _bTimedOut     = other._bTimedOut;
        _numReturnVals = other._numReturnVals;

        /* copy waitIdList but make sure it is empty first */
        _waitIdList.clear();
        for (int i = 0; eNotify_Invalid != (otherNotification = other.getWaitId(i)); i++)
        {
            eNotification * pNew = new eNotification(otherNotification);
            BDBG_ASSERT(NULL != pNew);

            _waitIdList.add(pNew);
        }
    } /* = */

protected:
    eNotification            _id;
    MAutoList<eNotification> _waitIdList;
    int                      _waitTimeout;
    bool                     _bTimedOut;
    int                      _numReturnVals;
};

template<class T>
class CDataAction : public CAction
{
public:
    CDataAction(
            eNotification id,
            T *           data,
            eNotification waitId = eNotify_Invalid,
            int           waitTimeout = 1000 /* B_WAIT_FOREVER */
            ) :
        CAction(id, waitId, waitTimeout),
        _data(data) {}

    virtual ~CDataAction(void) { DEL(_data); }

    virtual void * getDataPtr(void) { return((void *)_data); }

protected:
    T * _data;
};

#endif /* ATLAS_ACTION_H__ */