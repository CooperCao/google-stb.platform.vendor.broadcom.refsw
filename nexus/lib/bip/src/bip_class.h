/******************************************************************************
 * (c) 2007-2015 Broadcom Corporation
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

#ifndef BIP_CLASS_H
#define BIP_CLASS_H

#include "bip_priv.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup bip_class
 *
 * BIP_Class Interface Definition.
 *
 * This class is responsible creating, maintaining, and destroying class-specific
 * constructs, such as class mutexes and class lists (lists of object belonging
 * to each class).
 *
**/


/**
Summary:
The BIP_Class structure exists once for each BIP class.
**/
typedef struct BIP_Class
{
    const char         *pClassName;
    B_MutexHandle       hClassMutex;
    uint32_t            refCount;
    BLST_Q_HEAD(classBaseList, BIP_ClassInstance)  listHead;
} BIP_Class;

/**
Summary:
The BIP_ClassInstance structure exists is embedded inside of the
per-instance data for each BIP object.
**/
typedef struct BIP_ClassInstance
{
    BLST_Q_ENTRY(BIP_ClassInstance)     listNext;
    void                               *pThisInstance;
} BIP_ClassInstance;


/**
Summary:
Define and allocate static per-class data.

Description:
This macro allocates a global per-class structure named according to the
specified BIP class.

This macro should be placed in the main C source file for the specified
BIP class, before any function definitions.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.
**/
#define BIP_CLASS_DEFINE(className)     BIP_Class BIP_Class_For_##className


/**
Summary:
Declare external reference to static per-class data.

Description:
This macro makes an external declaration to a BIP class' global per-class
structure for specified BIP class.

Since each BIP class' global per-class structure is defined in the class'
"main" source file, this macro must be used in any other source file that
needs to reference the global per-class structure.

This macro should be placed before any functions, and before any other
BIP_CLASS macros.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.
**/
#define BIP_CLASS_DECLARE(className)    extern BIP_Class  BIP_Class_For_##className


/**
Summary:
Define per-object data required by BIP_CLASS macros.

Description:
This macro adds some variables to the per-object data structure.
These variables are only to be used by the BIP_CLASS macros.

This macro must be placed within the class' per-object data structure.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for
                        the class' per-object data structure.
**/
#define BIP_CLASS_DEFINE_INSTANCE_VARS(className)          BIP_ClassInstance  bipClassInstance


/**
Summary:
Perform one-time BIP_Class initialization for a class.

Description:
This macro initializes any data structures that are used by the BIP_Class macros
for the specified class.

This macro is intended to be invoked by BIP_Init().

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for
                        the class' per-object data structure.
**/


BIP_Status BIP_Class_P_Init(BIP_Class *pDesc, const char *pClassName,
                            const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_INIT(className, pBipStatus)   \
        BIP_Class_P_Init(&BIP_Class_For_##className, #className, __FILE__, __LINE__);


/**
Summary:
Perform one-time BIP_Class un-initialization for a class.

Description:
This macro un-initializes (destroys/frees) any data structures that are used by
BIP_Class for the specified class.

This macro is intended to be invoked by BIP_Uninit().

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for
                        the class' per-object data structure.
**/

BIP_Status BIP_Class_P_Uninit(BIP_Class *pDesc, const char *pClassName,
                              const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_UNINIT(className)     \
        BIP_Class_P_Uninit(&BIP_Class_For_##className, #className, __FILE__, __LINE__);


/**
Summary:
Add an instance to a BIP class.

Description:
This macro adds a new object to a class' list of existing objects.  If
necessary.

This macro should be placed in the BIP class' "Create" function.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to the newly allocated object data
                        structure.

    pBipStatus -        An optional  pointer to a BIP_Status variable that will be
                        set to indicate the completion status of the macro.  The
                        caller can pass NULL if not interested in the completion status.
**/

BIP_Status BIP_Class_P_AddInstance(BIP_Class *pDesc, const char *pClassName, void *pObject, BIP_ClassInstance *pClassInstance,
                                   const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_ADD_INSTANCE(className, pObject)   \
        BIP_Class_P_AddInstance(&BIP_Class_For_##className, #className, pObject, &(pObject)->bipClassInstance, __FILE__, __LINE__);



/**
Summary:
Remove an instance from a BIP class.

Description:
This macro removes an existing object from a class' list of existing
objects.  If the object does not belong to the class, it quietly returns
to the caller.

This macro should be place in the BIP class' "Destroy" function.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to the existing object data structure
                        to be removed from the class.
**/
BIP_Status BIP_Class_P_RemoveInstance(BIP_Class *pDesc, const char *pClassName, void *pObject,
                                      const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_REMOVE_INSTANCE(className, pObject)   \
        BIP_Class_P_RemoveInstance(&BIP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


/**
Summary:
Validate an object pointer and lock the per-class mutex for it's BIP class.

Description:
If pObject is found in the BIP class's list of object, the per-class mutex will be
locked and the completion status will be BIP_SUCCESS.

If pObject is not found, nothing will be locked, and the completion status will be
BIP_ERR_INVALID_HANDLE.

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.

    pObject -           A pointer to an existing object data structure (or maybe not).

    pBipStatus -        A pointer to a BIP_Status variable that will be set to indicate
                        the completion status of the macro.
**/

BIP_Status BIP_Class_P_LockAndCheckInstance(BIP_Class *pDesc, const char *pClassName, void *pObject,
                                            const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_LOCK_AND_CHECK_INSTANCE(className, pObject)   \
        BIP_Class_P_LockAndCheckInstance(&BIP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


/**
Summary:
Unlock the per-class mutex for a BIP class.

Description:

Arguments:
    className -         The name of the BIP class (e.g., BIP_Socket)
                        It must match a struct tag for the class'
                        per-object data structure.

    pBipStatus -        A pointer to a BIP_Status variable that will be set to indicate
                        the completion status of the macro.  The caller can pass NULL
                        if not interested in the completion status.
**/
BIP_Status BIP_Class_P_Unlock(BIP_Class *pDesc, const char *pClassName, void *pObject,
                              const char *pFilename, unsigned lineNumber);

#define BIP_CLASS_UNLOCK(className, pObject)    \
        BIP_Class_P_Unlock(&BIP_Class_For_##className, #className, pObject, __FILE__, __LINE__);


#ifdef __cplusplus
}
#endif

#endif /* BIP_CLASS_H */
