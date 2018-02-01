/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "bip_priv.h"

BDBG_MODULE( bip_class );

BIP_Status BIP_Class_P_Init(BIP_Class *pDesc, const char *pClassName, const char *pFilename, unsigned lineNumber)
{
    BIP_Status                  rc = BIP_SUCCESS;

    /* Lock the B_Os mutex to protect the refCount and class mutex creation and  */
    /* destruction.                                                            */
    B_Os_Lock();

    if (pDesc->hClassMutex)
    {
        B_Os_Unlock();

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s already initialized!"
                   BIP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(BIP_ERR_INVALID_API_SEQUENCE);
    }

    pDesc->pClassName = pClassName;
    pDesc->refCount = 0;
    pDesc->hClassMutex = B_Mutex_Create(NULL);
    if (pDesc->hClassMutex == NULL)
    {
        B_Os_Unlock();
        BIP_LOGERR(("B_Mutex_Create() failed."), BIP_ERR_B_OS_LIB);
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(BIP_ERR_B_OS_LIB);
    }

    B_Os_Unlock();

    BDBG_MSG(( BIP_MSG_PRE_FMT "Created class mutex for %s"
               BIP_MSG_PRE_ARG, pClassName));

    return(rc);
}

BIP_Status BIP_Class_P_Uninit(BIP_Class *pDesc, const char *pClassName, const char *pFilename, unsigned lineNumber)
{
    BIP_Status                  rc = BIP_SUCCESS;

    /* Lock the B_Os mutex to protect the refCount and class mutex creation and  */
    /* destruction.                                                            */
    B_Os_Lock();

    if (pDesc->refCount > 0)
    {
        B_Os_Unlock();

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s still has %u objects!"
                   BIP_MSG_PRE_ARG, pClassName, pDesc->refCount ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(BIP_ERR_INVALID_API_SEQUENCE);
    }

    if (pDesc->hClassMutex == NULL)
    {
        B_Os_Unlock();

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s is already uninitialized!"
                   BIP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(BIP_ERR_INVALID_API_SEQUENCE);
    }

    B_Mutex_Destroy(pDesc->hClassMutex);
    pDesc->hClassMutex = NULL;
    BDBG_MSG(( BIP_MSG_PRE_FMT "Destroyed class mutex for %s"
               BIP_MSG_PRE_ARG, pClassName));
    B_Os_Unlock();

    return(rc);
}

BIP_Status BIP_Class_P_AddInstance(BIP_Class *pDesc, const char *pClassName, void *pObject, BIP_ClassInstance *pClassInstance, const char *pFilename, unsigned lineNumber)
{
    BIP_Status                  rc = BIP_SUCCESS;

    /* Lock the class mutex to protect the object list during insertion. */
    if (pDesc->hClassMutex == NULL)
    {
        rc = BIP_ERR_INVALID_API_SEQUENCE;

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   BIP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    B_Mutex_Lock(pDesc->hClassMutex);
    pClassInstance->pThisInstance = pObject;
    BLST_Q_INSERT_TAIL(&pDesc->listHead, pClassInstance, listNext);

    BDBG_MSG(( BIP_MSG_PRE_FMT "Class %s (count=%u) added instance: %p"
               BIP_MSG_PRE_ARG, pClassName, pDesc->refCount, (void *)(pObject)));

    B_Mutex_Unlock(pDesc->hClassMutex);

    return(rc);
}

BIP_Status BIP_Class_P_RemoveInstance(BIP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    BIP_Status                  rc = BIP_SUCCESS;
    struct BIP_ClassInstance    *pCheck;

    /* Lock the class mutex to protect the object list during navigation */
    /* and removal.                                                      */
    if (pDesc->hClassMutex == NULL)
    {
        rc = BIP_ERR_INVALID_API_SEQUENCE;

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   BIP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    B_Mutex_Lock(pDesc->hClassMutex);

    for ( pCheck=BLST_Q_FIRST(&pDesc->listHead);
          pCheck ;
          pCheck = BLST_Q_NEXT((pCheck), listNext))
    {
        if (pCheck->pThisInstance == pObject)
        {
            BLST_Q_REMOVE(&pDesc->listHead, pCheck, listNext);
            break;
        }
    }
    B_Mutex_Unlock(pDesc->hClassMutex);

    if (pCheck == NULL) /* If pObject was not in list. */
    {
        rc = BIP_ERR_INVALID_HANDLE;

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s: Tried to remove non-existent instance: %p"
                   BIP_MSG_PRE_ARG, pClassName, (void *)(pObject)));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
    }

    return(rc);
}


BIP_Status BIP_Class_P_LockAndCheckInstance(BIP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    BIP_Status            rc = BIP_SUCCESS;
    BIP_ClassInstance    *pCheck;

    BDBG_MSG((BIP_MSG_PRE_FMT "ClassName=%s Requested by %s:%u\n" BIP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    /* Lock the class mutex to protect the object list during navigation. */
    if (pDesc->hClassMutex == NULL)
    {
        rc = BIP_ERR_INVALID_API_SEQUENCE;
        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   BIP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    /* Lock the class mutex to protect the list while we navigate. */
    B_Mutex_Lock(pDesc->hClassMutex);

    for ( pCheck=BLST_Q_FIRST(&pDesc->listHead) ;
          pCheck ;
          pCheck = BLST_Q_NEXT((pCheck),listNext))
    {
        if (pCheck->pThisInstance == pObject) { break; }
    }

    if (pCheck == NULL) /* If pObject is not in list. */
    {
        B_Mutex_Unlock(pDesc->hClassMutex);

        rc = BIP_ERR_INVALID_HANDLE;

        BDBG_ERR(( BIP_MSG_PRE_FMT "Class %s: Attempted use of stale handle: %p"
                   BIP_MSG_PRE_ARG, pClassName, (void*)(pObject)));
        BDBG_ERR(( BIP_MSG_PRE_FMT "...at %s:%u"
                   BIP_MSG_PRE_ARG, pFilename, lineNumber ));
    }

    /* If successful, return to caller with the class lock held.
     * Caller will call BIP_CLASS_UNLOCK() to release it.  */
    /* coverity[missing_unlock] */
    BDBG_MSG((BIP_MSG_PRE_FMT "ClassName=%s Locked by %s:%u\n" BIP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    return(rc);
}

BIP_Status BIP_Class_P_Unlock(BIP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    BIP_Status      rc = BIP_SUCCESS;

    BSTD_UNUSED(pObject);
    BSTD_UNUSED(pClassName);
    BSTD_UNUSED(pFilename);
    BSTD_UNUSED(lineNumber);

    B_MUTEX_ASSERT_LOCKED(pDesc->hClassMutex);

    BDBG_MSG((BIP_MSG_PRE_FMT "ClassName=%s Unlocked by %s:%u\n" BIP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    B_Mutex_Unlock(pDesc->hClassMutex);

    return(rc);
}
