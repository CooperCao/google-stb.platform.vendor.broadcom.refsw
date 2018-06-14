/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
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
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "b_asp_class.h"
#include "b_asp_priv.h"
#include "nexus_types.h"

BDBG_MODULE( b_asp_class );

/* Called only once via the B_Asp_Init(). */
NEXUS_Error B_ASP_Class_P_Init(B_ASP_Class *pDesc, const char *pClassName, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (pDesc->hClassMutex)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s already initialized!"
                   B_ASP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(NEXUS_NOT_SUPPORTED);
    }

    pDesc->pClassName = pClassName;
    pDesc->refCount = 0;

    if (BKNI_CreateMutex(&pDesc->hClassMutex) != 0)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "BKNI_CreateMutex() failed at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "Created class mutex for %s"
               B_ASP_MSG_PRE_ARG, pClassName));

    return(rc);
}

NEXUS_Error B_ASP_Class_P_Uninit(B_ASP_Class *pDesc, const char *pClassName, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error                  rc = NEXUS_SUCCESS;

    if (pDesc->refCount > 0)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s still has %u objects!"
                   B_ASP_MSG_PRE_ARG, pClassName, pDesc->refCount ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(NEXUS_INVALID_PARAMETER);
    }

    if (pDesc->hClassMutex == NULL)
    {
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s is already uninitialized!"
                   B_ASP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));

        return(NEXUS_INVALID_PARAMETER);
    }

    BKNI_DestroyMutex(pDesc->hClassMutex);
    pDesc->hClassMutex = NULL;
    BDBG_MSG(( B_ASP_MSG_PRE_FMT "Destroyed class mutex for %s"
               B_ASP_MSG_PRE_ARG, pClassName));
    return(rc);
}

NEXUS_Error B_ASP_Class_P_AddInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject, B_ASP_ClassInstance *pClassInstance, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error                  rc = NEXUS_SUCCESS;

    /* Lock the class mutex to protect the object list during insertion. */
    if (pDesc->hClassMutex == NULL)
    {
        rc = NEXUS_INVALID_PARAMETER;

        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   B_ASP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    BKNI_AcquireMutex(pDesc->hClassMutex);
    pClassInstance->pThisInstance = pObject;
    BLST_Q_INSERT_TAIL(&pDesc->listHead, pClassInstance, listNext);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "Class %s (count=%u) added instance: %p"
               B_ASP_MSG_PRE_ARG, pClassName, pDesc->refCount, (void *)(pObject)));

    BKNI_ReleaseMutex(pDesc->hClassMutex);

    return(rc);
}

NEXUS_Error B_ASP_Class_P_RemoveInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error                  rc = NEXUS_SUCCESS;
    struct B_ASP_ClassInstance    *pCheck;

    /* Lock the class mutex to protect the object list during navigation */
    /* and removal.                                                      */
    if (pDesc->hClassMutex == NULL)
    {
        rc = NEXUS_INVALID_PARAMETER;

        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   B_ASP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    BKNI_AcquireMutex(pDesc->hClassMutex);

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
    BKNI_ReleaseMutex(pDesc->hClassMutex);

    if (pCheck == NULL) /* If pObject was not in list. */
    {
        rc = NEXUS_INVALID_PARAMETER;

        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s: Tried to remove non-existent instance: %p"
                   B_ASP_MSG_PRE_ARG, pClassName, (void *)(pObject)));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
    }

    return(rc);
}


NEXUS_Error B_ASP_Class_P_LockAndCheckInstance(B_ASP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error            rc = NEXUS_SUCCESS;
    B_ASP_ClassInstance    *pCheck;

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "ClassName=%s Requested by %s:%u\n" B_ASP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    /* Lock the class mutex to protect the object list during navigation. */
    if (pDesc->hClassMutex == NULL)
    {
        rc = NEXUS_INVALID_PARAMETER;
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s is NOT initialized!"
                   B_ASP_MSG_PRE_ARG, pClassName ));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
        return(rc);
    }

    /* Lock the class mutex to protect the list while we navigate. */
    BKNI_AcquireMutex(pDesc->hClassMutex);

    for ( pCheck=BLST_Q_FIRST(&pDesc->listHead) ;
          pCheck ;
          pCheck = BLST_Q_NEXT((pCheck),listNext))
    {
        if (pCheck->pThisInstance == pObject) { break; }
    }

    if (pCheck == NULL) /* If pObject is not in list. */
    {
        BKNI_ReleaseMutex(pDesc->hClassMutex);

        rc = NEXUS_INVALID_PARAMETER;

        BDBG_ERR(( B_ASP_MSG_PRE_FMT "Class %s: Attempted use of stale handle: %p"
                   B_ASP_MSG_PRE_ARG, pClassName, (void*)(pObject)));
        BDBG_ERR(( B_ASP_MSG_PRE_FMT "...at %s:%u"
                   B_ASP_MSG_PRE_ARG, pFilename, lineNumber ));
        return (rc);
    }

    /* If successful, return to caller with the class lock held.
     * Caller will call BIP_CLASS_UNLOCK() to release it.  */
    /* coverity[missing_unlock] */
    BDBG_MSG(( B_ASP_MSG_PRE_FMT "ClassName=%s Locked by %s:%u\n" B_ASP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    return(rc);
}

NEXUS_Error B_ASP_Class_P_Unlock(B_ASP_Class *pDesc, const char *pClassName, void *pObject, const char *pFilename, unsigned lineNumber)
{
    NEXUS_Error      rc = NEXUS_SUCCESS;

    BSTD_UNUSED(pObject);
    BSTD_UNUSED(pClassName);
    BSTD_UNUSED(pFilename);
    BSTD_UNUSED(lineNumber);

    BDBG_MSG(( B_ASP_MSG_PRE_FMT "ClassName=%s Unlocked by %s:%u\n" B_ASP_MSG_PRE_ARG, pClassName, pFilename, lineNumber ));
    BKNI_ReleaseMutex(pDesc->hClassMutex);

    return(rc);
}
